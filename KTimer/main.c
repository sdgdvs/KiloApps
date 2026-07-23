#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)


#define ID_BTN_SW_TAB    1000
#define ID_BTN_TM_TAB    1001
#define ID_BTN_START     1002
#define ID_BTN_LAP       1003
#define ID_BTN_RESET     1004
#define ID_LIST_LAPS     1005
#define ID_EDIT_TM_INPUT 1006

// Global handles
HWND hMainWnd = NULL;
HWND hTabSW, hTabTM;
HWND hDisplay, hTmInput;
HWND hBtnStart, hBtnLap, hBtnReset;
HWND hListLaps;

HFONT hFontDisplay = NULL;
HFONT hFontBtn = NULL;
HBRUSH hBgBrush = NULL;
HBRUSH hControlBrush = NULL;

// App Modes
typedef enum { MODE_STOPWATCH = 0, MODE_TIMER = 1 } AppMode;
AppMode g_mode = MODE_STOPWATCH;

// Stopwatch State
DWORD g_swStartTime = 0;
DWORD g_swElapsed = 0;
int g_swIsRunning = 0;
int g_swLapCount = 0;
char g_swTimeBuf[32] = "00:00:00.000";

// Timer State
DWORD g_tmTargetTime = 0;
DWORD g_tmRemainingMs = 300000; // default 5 mins
DWORD g_tmTotalMs = 300000;
int g_tmIsRunning = 0;
char g_tmTimeBuf[32] = "00:05:00";

// Helper for formatting
static void FormatMsToStopwatch(DWORD totalMs, char* outBuf, size_t bufSize) {
    DWORD ms = totalMs % 1000;
    DWORD s = (totalMs / 1000) % 60;
    DWORD m = (totalMs / 60000) % 60;
    DWORD h = (totalMs / 3600000);
    wsprintfA(outBuf, "%02u:%02u:%02u.%03u", h, m, s, ms);
}

static void FormatMsToTimer(DWORD totalMs, char* outBuf, size_t bufSize) {
    DWORD s = (totalMs / 1000) % 60;
    DWORD m = (totalMs / 60000) % 60;
    DWORD h = (totalMs / 3600000);
    wsprintfA(outBuf, "%02u:%02u:%02u", h, m, s);
}

static int SimpleStrToInt(const char* p) {
    int val = 0;
    while (*p >= '0' && *p <= '9') {
        val = val * 10 + (*p - '0');
        p++;
    }
    return val;
}

static DWORD ParseTimerInput(const char* str) {
    int h = 0, m = 0, s = 0;
    int colons = 0;
    const char* p = str;
    const char* part1 = str;
    const char* part2 = NULL;
    const char* part3 = NULL;

    while (*p) {
        if (*p == ':') {
            colons++;
            if (colons == 1) part2 = p + 1;
            else if (colons == 2) part3 = p + 1;
        }
        p++;
    }

    if (colons == 2) {
        h = SimpleStrToInt(part1);
        m = SimpleStrToInt(part2);
        s = SimpleStrToInt(part3);
    } else if (colons == 1) {
        m = SimpleStrToInt(part1);
        s = SimpleStrToInt(part2);
    } else {
        s = SimpleStrToInt(part1);
    }

    if (h < 0) h = 0;
    if (m < 0) m = 0;
    if (s < 0) s = 0;

    DWORD totalSec = (DWORD)h * 3600 + (DWORD)m * 60 + (DWORD)s;
    if (totalSec > 359999) totalSec = 359999;
    return totalSec * 1000;
}

static void UpdateStopwatchDisplay() {
    DWORD current = g_swElapsed;
    if (g_swIsRunning) {
        current += (GetTickCount() - g_swStartTime);
    }
    FormatMsToStopwatch(current, g_swTimeBuf, sizeof(g_swTimeBuf));
    if (g_mode == MODE_STOPWATCH) {
        SetWindowTextA(hDisplay, g_swTimeBuf);
    }
}

static void PlayAlarmSound() {
    MessageBeep(MB_ICONEXCLAMATION);
    Beep(800, 250);
    Beep(1000, 250);
    Beep(1200, 300);
}

static void UpdateTimerDisplay() {
    if (g_tmIsRunning) {
        DWORD now = GetTickCount();
        if (now >= g_tmTargetTime) {
            g_tmRemainingMs = 0;
            g_tmIsRunning = 0;
            SetWindowTextA(hBtnStart, "Start");
            ShowWindow(hTmInput, SW_SHOW);
            ShowWindow(hDisplay, SW_HIDE);
            PlayAlarmSound();
        } else {
            g_tmRemainingMs = g_tmTargetTime - now;
        }
    }

    FormatMsToTimer(g_tmRemainingMs, g_tmTimeBuf, sizeof(g_tmTimeBuf));
    if (g_mode == MODE_TIMER && g_tmIsRunning) {
        SetWindowTextA(hDisplay, g_tmTimeBuf);
    }
}

static void SwitchMode(AppMode newMode) {
    g_mode = newMode;
    if (g_mode == MODE_STOPWATCH) {
        SetWindowTextA(hTabSW, "[ Stopwatch ]");
        SetWindowTextA(hTabTM, "  Timer  ");
        ShowWindow(hTmInput, SW_HIDE);
        ShowWindow(hDisplay, SW_SHOW);
        ShowWindow(hBtnLap, SW_SHOW);
        ShowWindow(hListLaps, SW_SHOW);

        SetWindowTextA(hBtnStart, g_swIsRunning ? "Stop" : "Start");
        UpdateStopwatchDisplay();
    } else {
        SetWindowTextA(hTabSW, "  Stopwatch  ");
        SetWindowTextA(hTabTM, "[ Timer ]");
        ShowWindow(hBtnLap, SW_HIDE);
        ShowWindow(hListLaps, SW_HIDE);

        if (g_tmIsRunning) {
            ShowWindow(hTmInput, SW_HIDE);
            ShowWindow(hDisplay, SW_SHOW);
            SetWindowTextA(hBtnStart, "Pause");
            UpdateTimerDisplay();
        } else {
            ShowWindow(hTmInput, SW_SHOW);
            ShowWindow(hDisplay, SW_HIDE);
            SetWindowTextA(hBtnStart, "Start");
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hMainWnd = hwnd;

            hBgBrush = CreateSolidBrush(RGB(20, 20, 20));
            hControlBrush = CreateSolidBrush(RGB(30, 30, 30));

            hFontDisplay = CreateFontA(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Consolas");
            hFontBtn = CreateFontA(15, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            hTabSW = CreateWindowA("BUTTON", "[ Stopwatch ]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 20, 10, 125, 30, hwnd, (HMENU)ID_BTN_SW_TAB, NULL, NULL);
            hTabTM = CreateWindowA("BUTTON", "  Timer  ", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 155, 10, 125, 30, hwnd, (HMENU)ID_BTN_TM_TAB, NULL, NULL);

            hDisplay = CreateWindowExA(0, "STATIC", "00:00:00.000", WS_CHILD | WS_VISIBLE | SS_CENTER, 20, 55, 260, 45, hwnd, NULL, NULL, NULL);
            hTmInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "00:05:00", WS_CHILD | ES_CENTER | ES_AUTOHSCROLL | WS_TABSTOP, 20, 55, 260, 45, hwnd, (HMENU)ID_EDIT_TM_INPUT, NULL, NULL);

            hBtnStart = CreateWindowA("BUTTON", "Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 20, 110, 75, 32, hwnd, (HMENU)ID_BTN_START, NULL, NULL);
            hBtnLap = CreateWindowA("BUTTON", "Lap", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 112, 110, 75, 32, hwnd, (HMENU)ID_BTN_LAP, NULL, NULL);
            hBtnReset = CreateWindowA("BUTTON", "Reset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 205, 110, 75, 32, hwnd, (HMENU)ID_BTN_RESET, NULL, NULL);

            hListLaps = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | WS_TABSTOP, 20, 152, 260, 140, hwnd, (HMENU)ID_LIST_LAPS, NULL, NULL);

            SendMessageA(hTabSW, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hTabTM, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hDisplay, WM_SETFONT, (WPARAM)hFontDisplay, TRUE);
            SendMessageA(hTmInput, WM_SETFONT, (WPARAM)hFontDisplay, TRUE);
            SendMessageA(hBtnStart, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnLap, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnReset, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hListLaps, WM_SETFONT, (WPARAM)hFontBtn, TRUE);

            SetTimer(hwnd, 1, 20, NULL);
            SwitchMode(MODE_STOPWATCH);
            break;
        }
        case WM_COMMAND: {
            WORD id = LOWORD(wParam);
            if (id == ID_BTN_SW_TAB) {
                SwitchMode(MODE_STOPWATCH);
            } else if (id == ID_BTN_TM_TAB) {
                SwitchMode(MODE_TIMER);
            } else if (id == ID_BTN_START) {
                if (g_mode == MODE_STOPWATCH) {
                    if (g_swIsRunning) {
                        g_swIsRunning = 0;
                        g_swElapsed += GetTickCount() - g_swStartTime;
                        SetWindowTextA(hBtnStart, "Start");
                    } else {
                        g_swIsRunning = 1;
                        g_swStartTime = GetTickCount();
                        SetWindowTextA(hBtnStart, "Stop");
                    }
                } else {
                    if (g_tmIsRunning) {
                        g_tmIsRunning = 0;
                        SetWindowTextA(hBtnStart, "Resume");
                    } else {
                        if (IsWindowVisible(hTmInput)) {
                            char inputStr[64] = {0};
                            GetWindowTextA(hTmInput, inputStr, sizeof(inputStr) - 1);
                            DWORD parsedMs = ParseTimerInput(inputStr);
                            if (parsedMs == 0) parsedMs = 300000;
                            g_tmTotalMs = parsedMs;
                            g_tmRemainingMs = parsedMs;
                            ShowWindow(hTmInput, SW_HIDE);
                            ShowWindow(hDisplay, SW_SHOW);
                        }
                        if (g_tmRemainingMs > 0) {
                            g_tmIsRunning = 1;
                            g_tmTargetTime = GetTickCount() + g_tmRemainingMs;
                            SetWindowTextA(hBtnStart, "Pause");
                        }
                    }
                }
            } else if (id == ID_BTN_LAP) {
                if (g_mode == MODE_STOPWATCH && g_swIsRunning) {
                    g_swLapCount++;
                    char lapBuf[128];
                    wsprintfA(lapBuf, "Lap %d: %s", g_swLapCount, g_swTimeBuf);
                    SendMessageA(hListLaps, LB_ADDSTRING, 0, (LPARAM)lapBuf);
                    int count = (int)SendMessageA(hListLaps, LB_GETCOUNT, 0, 0);
                    if (count > 0) {
                        SendMessageA(hListLaps, LB_SETTOPINDEX, count - 1, 0);
                    }
                }
            } else if (id == ID_BTN_RESET) {
                if (g_mode == MODE_STOPWATCH) {
                    g_swIsRunning = 0;
                    g_swElapsed = 0;
                    g_swLapCount = 0;
                    SetWindowTextA(hBtnStart, "Start");
                    SendMessageA(hListLaps, LB_RESETCONTENT, 0, 0);
                    UpdateStopwatchDisplay();
                } else {
                    g_tmIsRunning = 0;
                    g_tmRemainingMs = 300000;
                    g_tmTotalMs = 300000;
                    SetWindowTextA(hBtnStart, "Start");
                    SetWindowTextA(hTmInput, "00:05:00");
                    ShowWindow(hTmInput, SW_SHOW);
                    ShowWindow(hDisplay, SW_HIDE);
                }
            }
            break;
        }
        case WM_TIMER: {
            if (g_swIsRunning) {
                UpdateStopwatchDisplay();
            }
            if (g_tmIsRunning) {
                UpdateTimerDisplay();
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(20, 20, 20));
            SetTextColor(hdc, RGB(90, 139, 212));
            return (LRESULT)hBgBrush;
        }
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(30, 30, 30));
            SetTextColor(hdc, RGB(220, 220, 220));
            return (LRESULT)hControlBrush;
        }
        case WM_DESTROY: {
            KillTimer(hwnd, 1);
            if (hFontDisplay) DeleteObject(hFontDisplay);
            if (hFontBtn) DeleteObject(hFontBtn);
            if (hBgBrush) DeleteObject(hBgBrush);
            if (hControlBrush) DeleteObject(hControlBrush);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KTimerClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(20, 20, 20));

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KTimerClass", "KTimer", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 320, 340, NULL, NULL, wc.hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    if (wc.hbrBackground) DeleteObject(wc.hbrBackground);
    ExitProcess(0);
}
