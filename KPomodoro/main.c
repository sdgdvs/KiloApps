#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

// Identifiers
#define ID_TIMER_TICK        101
#define ID_TIMER_STATUS      102

#define ID_BTN_START         201
#define ID_BTN_SKIP          202
#define ID_BTN_RESET         203
#define ID_BTN_MODE_FOCUS    204
#define ID_BTN_MODE_SHORT    205
#define ID_BTN_MODE_LONG     206
#define ID_BTN_HELP          207
#define ID_BTN_SET_TASK      208
#define ID_EDIT_TASK         209

// App State
typedef enum {
    MODE_FOCUS = 0,
    MODE_SHORT_BREAK = 1,
    MODE_LONG_BREAK = 2
} PomoMode;

static PomoMode g_mode = MODE_FOCUS;
static BOOL g_isRunning = FALSE;
static int g_remainingSeconds = 25 * 60;
static int g_targetDuration = 25 * 60;
static int g_currentSet = 1;         // 1 to g_longInterval
static int g_longInterval = 4;
static int g_completedToday = 0;
static int g_todayMinutes = 0;
static int g_streak = 1;
static char g_taskTitle[128] = "System Architecture & Core Loop";
static int g_taskEst = 3;
static int g_taskDone = 0;

static int g_focusMin = 25;
static int g_shortMin = 5;
static int g_longMin = 15;

static char g_statusText[128] = "KPomodoro Ready. Press [Space] to begin focus cycle.";
static int g_statusTicks = 0;

// Controls
static HWND g_hwnd = NULL;
static HWND g_hBtnStart = NULL;
static HWND g_hBtnSkip = NULL;
static HWND g_hBtnReset = NULL;
static HWND g_hBtnModeFocus = NULL;
static HWND g_hBtnModeShort = NULL;
static HWND g_hBtnModeLong = NULL;
static HWND g_hBtnHelp = NULL;
static HWND g_hEditTask = NULL;
static HWND g_hBtnSetTask = NULL;

// GDI Resources
static HFONT g_hFontGiant = NULL;
static HFONT g_hFontTitle = NULL;
static HFONT g_hFontBold = NULL;
static HFONT g_hFontRegular = NULL;
static HFONT g_hFontSmall = NULL;

static HBRUSH g_hBrushBg = NULL;
static HBRUSH g_hBrushCard = NULL;
static HBRUSH g_hBrushCardDark = NULL;

static COLORREF COLOR_BG = RGB(15, 17, 24);
static COLORREF COLOR_CARD = RGB(23, 26, 36);
static COLORREF COLOR_BORDER = RGB(39, 45, 63);
static COLORREF COLOR_TEXT_PRIMARY = RGB(241, 245, 249);
static COLORREF COLOR_TEXT_MUTED = RGB(148, 163, 184);

static COLORREF COLOR_POMO_RED = RGB(239, 68, 68);
static COLORREF COLOR_BREAK_GREEN = RGB(16, 185, 129);
static COLORREF COLOR_BREAK_BLUE = RGB(14, 165, 233);
static COLORREF COLOR_GOLD = RGB(245, 158, 11);

// Forward declarations
static void SetMode(PomoMode newMode, BOOL resetTimer);
static void ToggleStartPause(void);
static void SkipPhase(void);
static void ResetTimer(void);
static void QuickSave(void);
static void QuickLoad(void);
static void ShowHelp(HWND hwnd);
static void ShowStatus(const char* txt);

static void ShowStatus(const char* txt) {
    if (!txt) return;
    int i = 0;
    while (txt[i] && i < (int)sizeof(g_statusText) - 1) {
        g_statusText[i] = txt[i];
        i++;
    }
    g_statusText[i] = '\0';
    g_statusTicks = 4; // ~4 sec
    if (g_hwnd) InvalidateRect(g_hwnd, NULL, FALSE);
}

static void FormatTime(int sec, char* buf) {
    int m = sec / 60;
    int s = sec % 60;
    wsprintfA(buf, "%02d:%02d", m, s);
}

static void QuickSave(void) {
    HANDLE hFile = CreateFileA("kpomodoro.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        int data[8];
        data[0] = (int)g_mode;
        data[1] = g_remainingSeconds;
        data[2] = g_targetDuration;
        data[3] = g_currentSet;
        data[4] = g_completedToday;
        data[5] = g_todayMinutes;
        data[6] = g_streak;
        data[7] = g_taskDone;
        WriteFile(hFile, data, sizeof(data), &written, NULL);
        WriteFile(hFile, g_taskTitle, sizeof(g_taskTitle), &written, NULL);
        CloseHandle(hFile);
        ShowStatus("Workstation state quicksaved to kpomodoro.dat (F5)");
    } else {
        ShowStatus("Failed to save state.");
    }
}

static void QuickLoad(void) {
    HANDLE hFile = CreateFileA("kpomodoro.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesRead = 0;
        int data[8];
        if (ReadFile(hFile, data, sizeof(data), &bytesRead, NULL) && bytesRead == sizeof(data)) {
            g_mode = (PomoMode)data[0];
            g_remainingSeconds = data[1];
            g_targetDuration = data[2];
            g_currentSet = data[3];
            g_completedToday = data[4];
            g_todayMinutes = data[5];
            g_streak = data[6];
            g_taskDone = data[7];
            ReadFile(hFile, g_taskTitle, sizeof(g_taskTitle), &bytesRead, NULL);
            if (g_hEditTask) SetWindowTextA(g_hEditTask, g_taskTitle);
            SetMode(g_mode, FALSE);
            ShowStatus("Workstation state quickloaded from kpomodoro.dat (F9)");
        }
        CloseHandle(hFile);
    } else {
        ShowStatus("No quicksave file (kpomodoro.dat) found.");
    }
}

static void SetMode(PomoMode newMode, BOOL resetTimer) {
    g_mode = newMode;
    if (g_mode == MODE_FOCUS) {
        g_targetDuration = g_focusMin * 60;
    } else if (g_mode == MODE_SHORT_BREAK) {
        g_targetDuration = g_shortMin * 60;
    } else {
        g_targetDuration = g_longMin * 60;
    }

    if (resetTimer) {
        g_remainingSeconds = g_targetDuration;
    }

    char timeBuf[16];
    FormatTime(g_remainingSeconds, timeBuf);
    char titleBuf[64];
    wsprintfA(titleBuf, "[%s] KPomodoro - %s", timeBuf, (g_mode == MODE_FOCUS ? "Focus" : "Break"));
    SetWindowTextA(g_hwnd, titleBuf);

    if (g_hwnd) InvalidateRect(g_hwnd, NULL, FALSE);
}

static void ToggleStartPause(void) {
    g_isRunning = !g_isRunning;
    if (g_isRunning) {
        SetTimer(g_hwnd, ID_TIMER_TICK, 1000, NULL);
        if (g_hBtnStart) SetWindowTextA(g_hBtnStart, "Pause (Space)");
        ShowStatus(g_mode == MODE_FOCUS ? "Focusing... Keep undivided attention." : "Break active. Relax and hydrate.");
    } else {
        KillTimer(g_hwnd, ID_TIMER_TICK);
        if (g_hBtnStart) SetWindowTextA(g_hBtnStart, "Start (Space)");
        ShowStatus("Paused.");
    }
    if (g_hwnd) InvalidateRect(g_hwnd, NULL, FALSE);
}

static void ResetTimer(void) {
    if (g_isRunning) ToggleStartPause();
    g_remainingSeconds = g_targetDuration;
    ShowStatus("Timer reset.");
    SetMode(g_mode, FALSE);
}

static void CompleteCycle(void) {
    g_isRunning = FALSE;
    KillTimer(g_hwnd, ID_TIMER_TICK);
    if (g_hBtnStart) SetWindowTextA(g_hBtnStart, "Start (Space)");

    MessageBeep(MB_ICONEXCLAMATION);

    if (g_mode == MODE_FOCUS) {
        g_completedToday++;
        g_todayMinutes += (g_targetDuration / 60);
        g_taskDone++;

        if (g_currentSet >= g_longInterval) {
            g_currentSet = 1;
            SetMode(MODE_LONG_BREAK, TRUE);
            ShowStatus("Cycle complete! Time for a restorative 15m Long Break.");
        } else {
            g_currentSet++;
            SetMode(MODE_SHORT_BREAK, TRUE);
            ShowStatus("Focus session finished! Enjoy a 5m Short Break.");
        }
    } else {
        SetMode(MODE_FOCUS, TRUE);
        ShowStatus("Break finished! Ready for next Focus session.");
    }
    QuickSave();
    if (g_hwnd) InvalidateRect(g_hwnd, NULL, FALSE);
}

static void SkipPhase(void) {
    if (g_isRunning) ToggleStartPause();
    if (g_mode == MODE_FOCUS) {
        if (g_currentSet >= g_longInterval) {
            g_currentSet = 1;
            SetMode(MODE_LONG_BREAK, TRUE);
        } else {
            g_currentSet++;
            SetMode(MODE_SHORT_BREAK, TRUE);
        }
        ShowStatus("Focus phase skipped.");
    } else {
        SetMode(MODE_FOCUS, TRUE);
        ShowStatus("Break phase skipped. Back to Focus.");
    }
    if (g_hwnd) InvalidateRect(g_hwnd, NULL, FALSE);
}

static void ShowHelp(HWND hwnd) {
    const char* helpText =
        "KPomodoro v1.0.0 - Work/Break Cycle Manager\n\n"
        "The Pomodoro Technique is a proven cognitive cadence method:\n"
        "  * 25 minutes of undivided high-intensity Focus\n"
        "  * 5 minutes of restorative Short Break\n"
        "  * After 4 sessions, take an extended 15-minute Long Break\n\n"
        "Keyboard Shortcuts:\n"
        "  * Space       : Start / Pause Timer\n"
        "  * S           : Skip to Next Phase\n"
        "  * R           : Reset Current Phase\n"
        "  * F1          : This Help Manual\n"
        "  * F5          : Quicksave State to disk (kpomodoro.dat)\n"
        "  * F9          : Quickload State from disk\n\n"
        "Ludonarrative ARG Lore:\n"
        "  The KiloOS fleet runs on disciplined cycles. Focus your attention,\n"
        "  conserve CPU cycles, and prepare for KMatrix #100.";
    MessageBoxA(hwnd, helpText, "KPomodoro Help & Guidelines", MB_OK | MB_ICONINFORMATION);
}

static void PaintUI(HWND hwnd, HDC hdc) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    // Double buffering
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBM = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

    // Background fill
    FillRect(memDC, &rc, g_hBrushBg);

    // Header strip
    RECT rcHeader = {0, 0, width, 48};
    HBRUSH hHeaderBrush = CreateSolidBrush(RGB(19, 22, 32));
    FillRect(memDC, &rcHeader, hHeaderBrush);
    DeleteObject(hHeaderBrush);

    SelectObject(memDC, g_hFontTitle);
    SetBkMode(memDC, TRANSPARENT);
    SetTextColor(memDC, COLOR_TEXT_PRIMARY);
    TextOutA(memDC, 18, 12, "KPOMODORO", 9);

    SelectObject(memDC, g_hFontSmall);
    SetTextColor(memDC, COLOR_TEXT_MUTED);
    TextOutA(memDC, 130, 16, "v1.0.0 | Work/Break Cycle Manager", 33);

    // Top Mode Status
    COLORREF activeColor = (g_mode == MODE_FOCUS ? COLOR_POMO_RED : (g_mode == MODE_SHORT_BREAK ? COLOR_BREAK_GREEN : COLOR_BREAK_BLUE));

    // Center Display Card
    RECT rcCard = {width / 2 - 250, 70, width / 2 + 250, 310};
    FillRect(memDC, &rcCard, g_hBrushCard);
    FrameRect(memDC, &rcCard, g_hBrushCardDark);

    // Mode Badge
    SelectObject(memDC, g_hFontBold);
    SetTextColor(memDC, activeColor);
    const char* modeTitle = (g_mode == MODE_FOCUS ? "--- FOCUS SESSION ---" : (g_mode == MODE_SHORT_BREAK ? "--- SHORT REST BREAK ---" : "--- LONG REST BREAK ---"));
    DrawTextA(memDC, modeTitle, -1, &rcCard, DT_CENTER | DT_TOP | DT_SINGLELINE);

    // Huge Digital Time
    char timeStr[16];
    FormatTime(g_remainingSeconds, timeStr);
    SelectObject(memDC, g_hFontGiant);
    SetTextColor(memDC, COLOR_TEXT_PRIMARY);

    RECT rcTime = {rcCard.left, rcCard.top + 35, rcCard.right, rcCard.top + 160};
    DrawTextA(memDC, timeStr, -1, &rcTime, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Cycle Pips: e.g. Session 2 of 4
    SelectObject(memDC, g_hFontRegular);
    SetTextColor(memDC, COLOR_TEXT_MUTED);
    char pipStr[64];
    char pips[32] = "";
    for (int p = 1; p <= g_longInterval; p++) {
        if (p < g_currentSet) lstrcatA(pips, "[#] ");
        else if (p == g_currentSet) lstrcatA(pips, "[*] ");
        else lstrcatA(pips, "[ ] ");
    }
    wsprintfA(pipStr, "Cycle: %s (Session %d of %d)", pips, g_currentSet, g_longInterval);
    RECT rcPips = {rcCard.left, rcCard.top + 175, rcCard.right, rcCard.top + 200};
    DrawTextA(memDC, pipStr, -1, &rcPips, DT_CENTER | DT_SINGLELINE);

    // Active Goal Banner
    SelectObject(memDC, g_hFontBold);
    SetTextColor(memDC, COLOR_GOLD);
    char goalStr[180];
    wsprintfA(goalStr, "Target Goal: \"%s\" (Completed: %d/%d)", g_taskTitle, g_taskDone, g_taskEst);
    RECT rcGoal = {rcCard.left + 10, rcCard.top + 205, rcCard.right - 10, rcCard.top + 230};
    DrawTextA(memDC, goalStr, -1, &rcGoal, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    // Daily Stats Box
    RECT rcStats = {width / 2 - 250, 380, width / 2 + 250, 470};
    FillRect(memDC, &rcStats, g_hBrushCard);
    FrameRect(memDC, &rcStats, g_hBrushCardDark);

    SelectObject(memDC, g_hFontBold);
    SetTextColor(memDC, COLOR_TEXT_PRIMARY);
    TextOutA(memDC, rcStats.left + 20, rcStats.top + 14, "Productivity Telemetry", 22);

    SelectObject(memDC, g_hFontRegular);
    SetTextColor(memDC, COLOR_TEXT_MUTED);
    char stat1[64], stat2[64], stat3[64];
    wsprintfA(stat1, "Completed Today: %d Pomodoros", g_completedToday);
    wsprintfA(stat2, "Total Focus Time: %d Minutes", g_todayMinutes);
    wsprintfA(stat3, "Focus Streak: %d Day%s", g_streak, g_streak == 1 ? "" : "s");

    TextOutA(memDC, rcStats.left + 20, rcStats.top + 42, stat1, lstrlenA(stat1));
    TextOutA(memDC, rcStats.left + 250, rcStats.top + 42, stat2, lstrlenA(stat2));
    TextOutA(memDC, rcStats.left + 20, rcStats.top + 64, stat3, lstrlenA(stat3));

    // Bottom Status Bar
    RECT rcStatus = {0, height - 28, width, height};
    HBRUSH hStatusBrush = CreateSolidBrush(RGB(13, 15, 21));
    FillRect(memDC, &rcStatus, hStatusBrush);
    DeleteObject(hStatusBrush);

    SelectObject(memDC, g_hFontSmall);
    SetTextColor(memDC, activeColor);
    TextOutA(memDC, 12, height - 20, "[*]", 3);

    SetTextColor(memDC, COLOR_TEXT_PRIMARY);
    TextOutA(memDC, 32, height - 20, g_statusText, lstrlenA(g_statusText));

    const char* hintText = "Space: Play/Pause | S: Skip | R: Reset | F1: Help | F5: Save | F9: Load";
    SIZE hintSize;
    GetTextExtentPoint32A(memDC, hintText, lstrlenA(hintText), &hintSize);
    SetTextColor(memDC, COLOR_TEXT_MUTED);
    TextOutA(memDC, width - hintSize.cx - 16, height - 20, hintText, lstrlenA(hintText));

    // Blit to screen
    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBM);
    DeleteObject(memBM);
    DeleteDC(memDC);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        g_hwnd = hwnd;

        // Fonts
        g_hFontGiant = CreateFontA(72, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        g_hFontTitle = CreateFontA(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        g_hFontBold = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        g_hFontRegular = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        g_hFontSmall = CreateFontA(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

        // Brushes
        g_hBrushBg = CreateSolidBrush(COLOR_BG);
        g_hBrushCard = CreateSolidBrush(COLOR_CARD);
        g_hBrushCardDark = CreateSolidBrush(COLOR_BORDER);

        // Buttons
        // Center Controls
        g_hBtnStart = CreateWindowExA(0, "BUTTON", "Start (Space)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 250, 325, 120, 36, hwnd, (HMENU)ID_BTN_START, NULL, NULL);
        g_hBtnSkip = CreateWindowExA(0, "BUTTON", "Skip (S)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 380, 325, 90, 36, hwnd, (HMENU)ID_BTN_SKIP, NULL, NULL);
        g_hBtnReset = CreateWindowExA(0, "BUTTON", "Reset (R)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 480, 325, 90, 36, hwnd, (HMENU)ID_BTN_RESET, NULL, NULL);

        // Header Mode Switchers
        g_hBtnModeFocus = CreateWindowExA(0, "BUTTON", "Focus (25m)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 420, 8, 100, 30, hwnd, (HMENU)ID_BTN_MODE_FOCUS, NULL, NULL);
        g_hBtnModeShort = CreateWindowExA(0, "BUTTON", "Short (5m)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 530, 8, 90, 30, hwnd, (HMENU)ID_BTN_MODE_SHORT, NULL, NULL);
        g_hBtnModeLong = CreateWindowExA(0, "BUTTON", "Long (15m)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 8, 90, 30, hwnd, (HMENU)ID_BTN_MODE_LONG, NULL, NULL);
        g_hBtnHelp = CreateWindowExA(0, "BUTTON", "Help (F1)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 730, 8, 70, 30, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);

        // Task edit input
        g_hEditTask = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", g_taskTitle, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 160, 485, 420, 26, hwnd, (HMENU)ID_EDIT_TASK, NULL, NULL);
        g_hBtnSetTask = CreateWindowExA(0, "BUTTON", "Set Target Goal", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 590, 485, 120, 26, hwnd, (HMENU)ID_BTN_SET_TASK, NULL, NULL);

        SendMessageA(g_hEditTask, WM_SETFONT, (WPARAM)g_hFontRegular, TRUE);

        SetTimer(hwnd, ID_TIMER_STATUS, 1000, NULL);
        QuickLoad();
        return 0;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == ID_BTN_START) {
            ToggleStartPause();
        } else if (id == ID_BTN_SKIP) {
            SkipPhase();
        } else if (id == ID_BTN_RESET) {
            ResetTimer();
        } else if (id == ID_BTN_MODE_FOCUS) {
            if (g_isRunning) ToggleStartPause();
            SetMode(MODE_FOCUS, TRUE);
        } else if (id == ID_BTN_MODE_SHORT) {
            if (g_isRunning) ToggleStartPause();
            SetMode(MODE_SHORT_BREAK, TRUE);
        } else if (id == ID_BTN_MODE_LONG) {
            if (g_isRunning) ToggleStartPause();
            SetMode(MODE_LONG_BREAK, TRUE);
        } else if (id == ID_BTN_HELP) {
            ShowHelp(hwnd);
        } else if (id == ID_BTN_SET_TASK) {
            char buf[128];
            GetWindowTextA(g_hEditTask, buf, sizeof(buf));
            if (buf[0]) {
                lstrcpynA(g_taskTitle, buf, sizeof(g_taskTitle));
                g_taskDone = 0;
                ShowStatus("Target focus goal updated.");
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        return 0;
    }

    case WM_TIMER: {
        if (wParam == ID_TIMER_TICK) {
            if (g_remainingSeconds > 0) {
                g_remainingSeconds--;
                char timeBuf[16];
                FormatTime(g_remainingSeconds, timeBuf);
                char titleBuf[64];
                wsprintfA(titleBuf, "[%s] KPomodoro - %s", timeBuf, (g_mode == MODE_FOCUS ? "Focus" : "Break"));
                SetWindowTextA(hwnd, titleBuf);
                InvalidateRect(hwnd, NULL, FALSE);
            } else {
                CompleteCycle();
            }
        } else if (wParam == ID_TIMER_STATUS) {
            if (g_statusTicks > 0) {
                g_statusTicks--;
                if (g_statusTicks == 0) {
                    ShowStatus("KPomodoro Ready. Press [Space] to begin focus cycle.");
                }
            }
        }
        return 0;
    }

    case WM_KEYDOWN: {
        if (wParam == VK_SPACE) {
            ToggleStartPause();
            return 0;
        } else if (wParam == 'S' || wParam == 's') {
            SkipPhase();
            return 0;
        } else if (wParam == 'R' || wParam == 'r') {
            ResetTimer();
            return 0;
        } else if (wParam == VK_F1) {
            ShowHelp(hwnd);
            return 0;
        } else if (wParam == VK_F5) {
            QuickSave();
            return 0;
        } else if (wParam == VK_F9) {
            QuickLoad();
            return 0;
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        PaintUI(hwnd, hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY: {
        KillTimer(hwnd, ID_TIMER_TICK);
        KillTimer(hwnd, ID_TIMER_STATUS);

        DeleteObject(g_hFontGiant);
        DeleteObject(g_hFontTitle);
        DeleteObject(g_hFontBold);
        DeleteObject(g_hFontRegular);
        DeleteObject(g_hFontSmall);

        DeleteObject(g_hBrushBg);
        DeleteObject(g_hBrushCard);
        DeleteObject(g_hBrushCardDark);

        QuickSave();
        PostQuitMessage(0);
        return 0;
    }
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

void MainEntry(void) {
    HINSTANCE hInstance = GetModuleHandleA(NULL);

    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KPomodoroClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    RegisterClassA(&wc);

    RECT rc = {0, 0, 820, 560};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowExA(
        0,
        "KPomodoroClass",
        "KPomodoro - Work/Break Cycle Manager",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            HWND focusWnd = GetFocus();
            if (focusWnd == g_hEditTask) {
                if (msg.wParam == VK_RETURN) {
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_SET_TASK, BN_CLICKED), (LPARAM)g_hBtnSetTask);
                    SetFocus(hwnd);
                    continue;
                }
                if (msg.wParam == VK_ESCAPE) {
                    SetFocus(hwnd);
                    continue;
                }
            } else {
                if (msg.wParam == VK_SPACE) {
                    ToggleStartPause();
                    continue;
                }
                if (msg.wParam == 'S' || msg.wParam == 's') {
                    SkipPhase();
                    continue;
                }
                if (msg.wParam == 'R' || msg.wParam == 'r') {
                    ResetTimer();
                    continue;
                }
                if (msg.wParam == VK_F1) {
                    ShowHelp(hwnd);
                    continue;
                }
                if (msg.wParam == VK_F5) {
                    QuickSave();
                    continue;
                }
                if (msg.wParam == VK_F9) {
                    QuickLoad();
                    continue;
                }
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
