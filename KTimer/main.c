#include <windows.h>
#include <commdlg.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

// Control Identifiers
#define ID_BTN_SW_TAB      1000
#define ID_BTN_TM_TAB      1001
#define ID_BTN_MT_TAB      1002
#define ID_BTN_POMO_TAB    1003

#define ID_BTN_START       1010
#define ID_BTN_LAP         1011
#define ID_BTN_RESET       1012
#define ID_BTN_EXPORT_CSV  1013
#define ID_BTN_EXPORT_TXT  1014
#define ID_LIST_LAPS       1015

#define ID_EDIT_TM_INPUT   1020
#define ID_PRESET_1M       1021
#define ID_PRESET_3M       1022
#define ID_PRESET_5M       1023
#define ID_PRESET_10M      1024
#define ID_PRESET_15M      1025
#define ID_PRESET_25M      1026
#define ID_PRESET_30M      1027
#define ID_PRESET_60M      1028

#define ID_EDIT_MT_NAME    1030
#define ID_EDIT_MT_TIME    1031
#define ID_BTN_MT_ADD      1032
#define ID_LIST_MT         1033
#define ID_BTN_MT_STARTALL 1034
#define ID_BTN_MT_PAUSEALL 1035
#define ID_BTN_MT_DEL      1036

#define ID_BTN_POMO_START  1040
#define ID_BTN_POMO_SKIP   1041
#define ID_BTN_POMO_RESET  1042

// App Modes
typedef enum {
    MODE_STOPWATCH = 0,
    MODE_TIMER = 1,
    MODE_MULTI = 2,
    MODE_POMODORO = 3
} AppMode;

AppMode g_mode = MODE_STOPWATCH;

// Lap Structure
typedef struct {
    int id;
    DWORD totalMs;
    DWORD splitMs;
} LapInfo;

#define MAX_LAPS 200
LapInfo g_laps[MAX_LAPS];
int g_lapCount = 0;

// Multi-Timer Structure
typedef struct {
    char name[32];
    DWORD totalMs;
    DWORD remainingMs;
    DWORD lastTick;
    int isRunning;
} MultiTimer;

#define MAX_MULTI_TIMERS 10
MultiTimer g_multiTimers[MAX_MULTI_TIMERS];
int g_multiTimerCount = 0;

// Pomodoro State
typedef enum { POMO_WORK = 0, POMO_SHORT_BREAK = 1, POMO_LONG_BREAK = 2 } PomoState;
PomoState g_pomoState = POMO_WORK;
int g_pomoCycleCount = 1;
DWORD g_pomoRemainingMs = 25 * 60 * 1000;
DWORD g_pomoTotalMs = 25 * 60 * 1000;
DWORD g_pomoTargetTime = 0;
int g_pomoIsRunning = 0;
int g_pomoCompletedSessions = 0;
DWORD g_pomoTotalFocusMins = 0;

// Stopwatch State
DWORD g_swStartTime = 0;
DWORD g_swElapsed = 0;
int g_swIsRunning = 0;
char g_swTimeBuf[32] = "00:00:00.000";

// Timer State
DWORD g_tmTargetTime = 0;
DWORD g_tmRemainingMs = 300000; // default 5 mins
DWORD g_tmTotalMs = 300000;
int g_tmIsRunning = 0;
char g_tmTimeBuf[32] = "00:05:00";

// Global Window Handles
HWND hMainWnd = NULL;
HWND hTabSW, hTabTM, hTabMT, hTabPOMO;
HWND hDisplay, hTmInput, hStaticStats;
HWND hBtnStart, hBtnLap, hBtnReset, hBtnExportCsv, hBtnExportTxt;
HWND hListLaps;

// Presets Handles
HWND hPresets[8];

// Multi-Timer Handles
HWND hEditMtName, hEditMtTime, hBtnMtAdd, hListMt, hBtnMtStartAll, hBtnMtPauseAll, hBtnMtDel;

// Pomodoro Handles
HWND hBtnPomoStart, hBtnPomoSkip, hBtnPomoReset;

HFONT hFontDisplay = NULL;
HFONT hFontBtn = NULL;
HFONT hFontSmall = NULL;
HBRUSH hBgBrush = NULL;
HBRUSH hControlBrush = NULL;
HBRUSH hProgressBarBrush = NULL;

// Helper String Formatting
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

static void PlayAlarmSound() {
    MessageBeep(MB_ICONASTERISK);
    Beep(800, 200);
    Beep(1000, 200);
    Beep(1200, 300);
}

static void PlayChimeSound() {
    MessageBeep(MB_OK);
    Beep(523, 150);
    Beep(659, 150);
    Beep(784, 250);
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

static void UpdatePomodoroDisplay() {
    if (g_pomoIsRunning) {
        DWORD now = GetTickCount();
        if (now >= g_pomoTargetTime) {
            g_pomoRemainingMs = 0;
            g_pomoIsRunning = 0;
            SetWindowTextA(hBtnPomoStart, "Start");
            PlayChimeSound();

            if (g_pomoState == POMO_WORK) {
                g_pomoCompletedSessions++;
                g_pomoTotalFocusMins += (g_pomoTotalMs / 60000);
                if (g_pomoCycleCount >= 4) {
                    g_pomoState = POMO_LONG_BREAK;
                    g_pomoCycleCount = 1;
                    g_pomoTotalMs = 15 * 60 * 1000;
                } else {
                    g_pomoState = POMO_SHORT_BREAK;
                    g_pomoCycleCount++;
                    g_pomoTotalMs = 5 * 60 * 1000;
                }
            } else {
                g_pomoState = POMO_WORK;
                g_pomoTotalMs = 25 * 60 * 1000;
            }
            g_pomoRemainingMs = g_pomoTotalMs;
        } else {
            g_pomoRemainingMs = g_pomoTargetTime - now;
        }
    }

    char pomoBuf[64];
    FormatMsToTimer(g_pomoRemainingMs, pomoBuf, sizeof(pomoBuf));

    if (g_mode == MODE_POMODORO) {
        SetWindowTextA(hDisplay, pomoBuf + 3); // MM:SS

        const char* stateName = "WORK SESSION";
        if (g_pomoState == POMO_SHORT_BREAK) stateName = "SHORT BREAK";
        if (g_pomoState == POMO_LONG_BREAK) stateName = "LONG BREAK";

        char statsBuf[128];
        wsprintfA(statsBuf, "%s (Cycle %d/4)\nDone: %d | Focus: %u mins", stateName, g_pomoCycleCount, g_pomoCompletedSessions, g_pomoTotalFocusMins);
        SetWindowTextA(hStaticStats, statsBuf);
    }
}

static void UpdateMultiTimers() {
    DWORD now = GetTickCount();
    int needRefresh = 0;

    for (int i = 0; i < g_multiTimerCount; i++) {
        if (g_multiTimers[i].isRunning) {
            DWORD delta = now - g_multiTimers[i].lastTick;
            g_multiTimers[i].lastTick = now;
            if (delta >= g_multiTimers[i].remainingMs) {
                g_multiTimers[i].remainingMs = 0;
                g_multiTimers[i].isRunning = 0;
                PlayAlarmSound();
            } else {
                g_multiTimers[i].remainingMs -= delta;
            }
            needRefresh = 1;
        }
    }

    if (needRefresh && g_mode == MODE_MULTI) {
        SendMessageA(hListMt, LB_RESETCONTENT, 0, 0);
        for (int i = 0; i < g_multiTimerCount; i++) {
            char itemBuf[128];
            char timeBuf[32];
            FormatMsToTimer(g_multiTimers[i].remainingMs, timeBuf, sizeof(timeBuf));
            const char* status = g_multiTimers[i].remainingMs == 0 ? "[DONE]" : (g_multiTimers[i].isRunning ? "[RUNNING]" : "[PAUSED]");
            wsprintfA(itemBuf, "%s - %s %s", g_multiTimers[i].name, timeBuf, status);
            SendMessageA(hListMt, LB_ADDSTRING, 0, (LPARAM)itemBuf);
        }
    }
}

static void ExportLapsToFile(const char* ext) {
    if (g_lapCount == 0) {
        MessageBoxA(hMainWnd, "No laps recorded to export.", "KTimer Export", MB_OK | MB_ICONINFORMATION);
        return;
    }

    OPENFILENAMEA ofn = {0};
    char filename[MAX_PATH] = "ktimer_laps";
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hMainWnd;
    if (lstrcmpA(ext, "csv") == 0) {
        ofn.lpstrFilter = "CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0";
        lstrcatA(filename, ".csv");
    } else {
        ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
        lstrcatA(filename, ".txt");
    }
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename);
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            char buffer[2048];
            DWORD written = 0;
            if (lstrcmpA(ext, "csv") == 0) {
                lstrcpyA(buffer, "Lap,Split Time,Total Time\r\n");
                WriteFile(hFile, buffer, lstrlenA(buffer), &written, NULL);
                for (int i = 0; i < g_lapCount; i++) {
                    char sBuf[32], tBuf[32];
                    FormatMsToStopwatch(g_laps[i].splitMs, sBuf, sizeof(sBuf));
                    FormatMsToStopwatch(g_laps[i].totalMs, tBuf, sizeof(tBuf));
                    wsprintfA(buffer, "%d,%s,%s\r\n", g_laps[i].id, sBuf, tBuf);
                    WriteFile(hFile, buffer, lstrlenA(buffer), &written, NULL);
                }
            } else {
                lstrcpyA(buffer, "=== KTimer Lap Report ===\r\n\r\n");
                WriteFile(hFile, buffer, lstrlenA(buffer), &written, NULL);
                for (int i = 0; i < g_lapCount; i++) {
                    char sBuf[32], tBuf[32];
                    FormatMsToStopwatch(g_laps[i].splitMs, sBuf, sizeof(sBuf));
                    FormatMsToStopwatch(g_laps[i].totalMs, tBuf, sizeof(tBuf));
                    wsprintfA(buffer, "Lap %d | Split: %s | Total: %s\r\n", g_laps[i].id, sBuf, tBuf);
                    WriteFile(hFile, buffer, lstrlenA(buffer), &written, NULL);
                }
            }
            CloseHandle(hFile);
            MessageBoxA(hMainWnd, "Laps exported successfully!", "KTimer Export", MB_OK | MB_ICONINFORMATION);
        }
    }
}

static void SwitchMode(AppMode newMode) {
    g_mode = newMode;

    // Reset button states
    SetWindowTextA(hTabSW, g_mode == MODE_STOPWATCH ? "[ Stopwatch ]" : "Stopwatch");
    SetWindowTextA(hTabTM, g_mode == MODE_TIMER ? "[ Timer ]" : "Timer");
    SetWindowTextA(hTabMT, g_mode == MODE_MULTI ? "[ Multi ]" : "Multi");
    SetWindowTextA(hTabPOMO, g_mode == MODE_POMODORO ? "[ Pomodoro ]" : "Pomodoro");

    // Hide all mode-specific controls first
    ShowWindow(hDisplay, SW_SHOW);
    ShowWindow(hTmInput, SW_HIDE);
    ShowWindow(hBtnStart, SW_HIDE);
    ShowWindow(hBtnLap, SW_HIDE);
    ShowWindow(hBtnReset, SW_HIDE);
    ShowWindow(hBtnExportCsv, SW_HIDE);
    ShowWindow(hBtnExportTxt, SW_HIDE);
    ShowWindow(hListLaps, SW_HIDE);

    for (int i = 0; i < 8; i++) ShowWindow(hPresets[i], SW_HIDE);

    ShowWindow(hEditMtName, SW_HIDE);
    ShowWindow(hEditMtTime, SW_HIDE);
    ShowWindow(hBtnMtAdd, SW_HIDE);
    ShowWindow(hListMt, SW_HIDE);
    ShowWindow(hBtnMtStartAll, SW_HIDE);
    ShowWindow(hBtnMtPauseAll, SW_HIDE);
    ShowWindow(hBtnMtDel, SW_HIDE);

    ShowWindow(hBtnPomoStart, SW_HIDE);
    ShowWindow(hBtnPomoSkip, SW_HIDE);
    ShowWindow(hBtnPomoReset, SW_HIDE);
    ShowWindow(hStaticStats, SW_HIDE);

    if (g_mode == MODE_STOPWATCH) {
        ShowWindow(hBtnStart, SW_SHOW);
        ShowWindow(hBtnLap, SW_SHOW);
        ShowWindow(hBtnReset, SW_SHOW);
        ShowWindow(hBtnExportCsv, SW_SHOW);
        ShowWindow(hBtnExportTxt, SW_SHOW);
        ShowWindow(hListLaps, SW_SHOW);

        SetWindowTextA(hBtnStart, g_swIsRunning ? "Stop" : "Start");
        UpdateStopwatchDisplay();
    } else if (g_mode == MODE_TIMER) {
        ShowWindow(hBtnStart, SW_SHOW);
        ShowWindow(hBtnReset, SW_SHOW);
        for (int i = 0; i < 8; i++) ShowWindow(hPresets[i], SW_SHOW);

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
    } else if (g_mode == MODE_MULTI) {
        ShowWindow(hEditMtName, SW_SHOW);
        ShowWindow(hEditMtTime, SW_SHOW);
        ShowWindow(hBtnMtAdd, SW_SHOW);
        ShowWindow(hListMt, SW_SHOW);
        ShowWindow(hBtnMtStartAll, SW_SHOW);
        ShowWindow(hBtnMtPauseAll, SW_SHOW);
        ShowWindow(hBtnMtDel, SW_SHOW);
        ShowWindow(hDisplay, SW_HIDE);

        // Refresh Multi Listbox
        SendMessageA(hListMt, LB_RESETCONTENT, 0, 0);
        for (int i = 0; i < g_multiTimerCount; i++) {
            char itemBuf[128], timeBuf[32];
            FormatMsToTimer(g_multiTimers[i].remainingMs, timeBuf, sizeof(timeBuf));
            const char* status = g_multiTimers[i].remainingMs == 0 ? "[DONE]" : (g_multiTimers[i].isRunning ? "[RUNNING]" : "[PAUSED]");
            wsprintfA(itemBuf, "%s - %s %s", g_multiTimers[i].name, timeBuf, status);
            SendMessageA(hListMt, LB_ADDSTRING, 0, (LPARAM)itemBuf);
        }
    } else if (g_mode == MODE_POMODORO) {
        ShowWindow(hBtnPomoStart, SW_SHOW);
        ShowWindow(hBtnPomoSkip, SW_SHOW);
        ShowWindow(hBtnPomoReset, SW_SHOW);
        ShowWindow(hStaticStats, SW_SHOW);

        SetWindowTextA(hBtnPomoStart, g_pomoIsRunning ? "Pause" : "Start");
        UpdatePomodoroDisplay();
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hMainWnd = hwnd;

            hBgBrush = CreateSolidBrush(RGB(18, 19, 24));
            hControlBrush = CreateSolidBrush(RGB(26, 28, 35));
            hProgressBarBrush = CreateSolidBrush(RGB(90, 139, 212));

            hFontDisplay = CreateFontA(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Consolas");
            hFontBtn = CreateFontA(14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            hFontSmall = CreateFontA(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            // Top Bar Tabs
            hTabSW = CreateWindowA("BUTTON", "[ Stopwatch ]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 10, 92, 30, hwnd, (HMENU)ID_BTN_SW_TAB, NULL, NULL);
            hTabTM = CreateWindowA("BUTTON", "Timer", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 106, 10, 92, 30, hwnd, (HMENU)ID_BTN_TM_TAB, NULL, NULL);
            hTabMT = CreateWindowA("BUTTON", "Multi", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 202, 10, 92, 30, hwnd, (HMENU)ID_BTN_MT_TAB, NULL, NULL);
            hTabPOMO = CreateWindowA("BUTTON", "Pomodoro", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 298, 10, 92, 30, hwnd, (HMENU)ID_BTN_POMO_TAB, NULL, NULL);

            // Display & Input Controls
            hDisplay = CreateWindowExA(0, "STATIC", "00:00:00.000", WS_CHILD | WS_VISIBLE | SS_CENTER, 10, 50, 380, 40, hwnd, NULL, NULL, NULL);
            hTmInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "00:05:00", WS_CHILD | ES_CENTER | ES_AUTOHSCROLL, 10, 50, 380, 40, hwnd, (HMENU)ID_EDIT_TM_INPUT, NULL, NULL);

            // Stopwatch Controls
            hBtnStart = CreateWindowA("BUTTON", "Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 98, 70, 30, hwnd, (HMENU)ID_BTN_START, NULL, NULL);
            hBtnLap = CreateWindowA("BUTTON", "Lap", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 85, 98, 70, 30, hwnd, (HMENU)ID_BTN_LAP, NULL, NULL);
            hBtnReset = CreateWindowA("BUTTON", "Reset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 160, 98, 70, 30, hwnd, (HMENU)ID_BTN_RESET, NULL, NULL);
            hBtnExportCsv = CreateWindowA("BUTTON", "CSV", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 240, 98, 70, 30, hwnd, (HMENU)ID_BTN_EXPORT_CSV, NULL, NULL);
            hBtnExportTxt = CreateWindowA("BUTTON", "TXT", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 315, 98, 75, 30, hwnd, (HMENU)ID_BTN_EXPORT_TXT, NULL, NULL);

            hListLaps = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT, 10, 136, 380, 240, hwnd, (HMENU)ID_LIST_LAPS, NULL, NULL);

            // Timer Presets Buttons
            const char* presetLabels[8] = {"1m", "3m", "5m", "10m", "15m", "25m", "30m", "60m"};
            for (int i = 0; i < 8; i++) {
                hPresets[i] = CreateWindowA("BUTTON", presetLabels[i], WS_CHILD | BS_PUSHBUTTON, 10 + (i % 4) * 95, 136 + (i / 4) * 36, 88, 30, hwnd, (HMENU)(INT_PTR)(ID_PRESET_1M + i), NULL, NULL);
                SendMessageA(hPresets[i], WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            }

            // Multi-Timer Controls
            hEditMtName = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Tea", WS_CHILD | ES_AUTOHSCROLL, 10, 50, 170, 28, hwnd, (HMENU)ID_EDIT_MT_NAME, NULL, NULL);
            hEditMtTime = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "03:00", WS_CHILD | ES_CENTER | ES_AUTOHSCROLL, 185, 50, 110, 28, hwnd, (HMENU)ID_EDIT_MT_TIME, NULL, NULL);
            hBtnMtAdd = CreateWindowA("BUTTON", "+ Add", WS_CHILD | BS_PUSHBUTTON, 300, 50, 90, 28, hwnd, (HMENU)ID_BTN_MT_ADD, NULL, NULL);
            hListMt = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VSCROLL | LBS_NOINTEGRALHEIGHT, 10, 86, 380, 240, hwnd, (HMENU)ID_LIST_MT, NULL, NULL);
            hBtnMtStartAll = CreateWindowA("BUTTON", "Start All", WS_CHILD | BS_PUSHBUTTON, 10, 334, 115, 30, hwnd, (HMENU)ID_BTN_MT_STARTALL, NULL, NULL);
            hBtnMtPauseAll = CreateWindowA("BUTTON", "Pause All", WS_CHILD | BS_PUSHBUTTON, 135, 334, 115, 30, hwnd, (HMENU)ID_BTN_MT_PAUSEALL, NULL, NULL);
            hBtnMtDel = CreateWindowA("BUTTON", "Delete", WS_CHILD | BS_PUSHBUTTON, 260, 334, 130, 30, hwnd, (HMENU)ID_BTN_MT_DEL, NULL, NULL);

            // Pomodoro Controls
            hBtnPomoStart = CreateWindowA("BUTTON", "Start", WS_CHILD | BS_PUSHBUTTON, 10, 98, 115, 32, hwnd, (HMENU)ID_BTN_POMO_START, NULL, NULL);
            hBtnPomoSkip = CreateWindowA("BUTTON", "Skip Phase", WS_CHILD | BS_PUSHBUTTON, 135, 98, 125, 32, hwnd, (HMENU)ID_BTN_POMO_SKIP, NULL, NULL);
            hBtnPomoReset = CreateWindowA("BUTTON", "Reset", WS_CHILD | BS_PUSHBUTTON, 270, 98, 120, 32, hwnd, (HMENU)ID_BTN_POMO_RESET, NULL, NULL);
            hStaticStats = CreateWindowExA(0, "STATIC", "WORK SESSION\nDone: 0 | Focus: 0 mins", WS_CHILD | SS_CENTER, 10, 145, 380, 60, hwnd, NULL, NULL, NULL);

            // Font Application
            SendMessageA(hTabSW, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hTabTM, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hTabMT, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hTabPOMO, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hDisplay, WM_SETFONT, (WPARAM)hFontDisplay, TRUE);
            SendMessageA(hTmInput, WM_SETFONT, (WPARAM)hFontDisplay, TRUE);
            SendMessageA(hBtnStart, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnLap, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnReset, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnExportCsv, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnExportTxt, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hListLaps, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hEditMtName, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hEditMtTime, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnMtAdd, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hListMt, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnMtStartAll, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnMtPauseAll, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnMtDel, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnPomoStart, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnPomoSkip, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnPomoReset, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hStaticStats, WM_SETFONT, (WPARAM)hFontBtn, TRUE);

            SetTimer(hwnd, 1, 25, NULL);
            SwitchMode(MODE_STOPWATCH);
            break;
        }
        case WM_COMMAND: {
            WORD id = LOWORD(wParam);
            if (id == ID_BTN_SW_TAB) SwitchMode(MODE_STOPWATCH);
            else if (id == ID_BTN_TM_TAB) SwitchMode(MODE_TIMER);
            else if (id == ID_BTN_MT_TAB) SwitchMode(MODE_MULTI);
            else if (id == ID_BTN_POMO_TAB) SwitchMode(MODE_POMODORO);

            // Stopwatch Handlers
            else if (id == ID_BTN_START && g_mode == MODE_STOPWATCH) {
                if (g_swIsRunning) {
                    g_swIsRunning = 0;
                    g_swElapsed += GetTickCount() - g_swStartTime;
                    SetWindowTextA(hBtnStart, "Start");
                } else {
                    g_swIsRunning = 1;
                    g_swStartTime = GetTickCount();
                    SetWindowTextA(hBtnStart, "Stop");
                }
            } else if (id == ID_BTN_LAP && g_mode == MODE_STOPWATCH) {
                DWORD currentMs = g_swElapsed + (g_swIsRunning ? (GetTickCount() - g_swStartTime) : 0);
                if (currentMs > 0 && g_lapCount < MAX_LAPS) {
                    DWORD prevTotal = g_lapCount > 0 ? g_laps[g_lapCount - 1].totalMs : 0;
                    DWORD splitMs = currentMs - prevTotal;

                    g_lapCount++;
                    g_laps[g_lapCount - 1].id = g_lapCount;
                    g_laps[g_lapCount - 1].totalMs = currentMs;
                    g_laps[g_lapCount - 1].splitMs = splitMs;

                    char sBuf[32], tBuf[32], lapBuf[128];
                    FormatMsToStopwatch(splitMs, sBuf, sizeof(sBuf));
                    FormatMsToStopwatch(currentMs, tBuf, sizeof(tBuf));
                    wsprintfA(lapBuf, "Lap %d | Split: %s | Total: %s", g_lapCount, sBuf, tBuf);
                    SendMessageA(hListLaps, LB_ADDSTRING, 0, (LPARAM)lapBuf);
                    SendMessageA(hListLaps, LB_SETTOPINDEX, g_lapCount - 1, 0);
                }
            } else if (id == ID_BTN_RESET && g_mode == MODE_STOPWATCH) {
                g_swIsRunning = 0;
                g_swElapsed = 0;
                g_lapCount = 0;
                SetWindowTextA(hBtnStart, "Start");
                SendMessageA(hListLaps, LB_RESETCONTENT, 0, 0);
                UpdateStopwatchDisplay();
            } else if (id == ID_BTN_EXPORT_CSV) {
                ExportLapsToFile("csv");
            } else if (id == ID_BTN_EXPORT_TXT) {
                ExportLapsToFile("txt");
            }

            // Single Timer Handlers
            else if (id == ID_BTN_START && g_mode == MODE_TIMER) {
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
            } else if (id == ID_BTN_RESET && g_mode == MODE_TIMER) {
                g_tmIsRunning = 0;
                g_tmRemainingMs = 300000;
                g_tmTotalMs = 300000;
                SetWindowTextA(hBtnStart, "Start");
                SetWindowTextA(hTmInput, "00:05:00");
                ShowWindow(hTmInput, SW_SHOW);
                ShowWindow(hDisplay, SW_HIDE);
            } else if (id >= ID_PRESET_1M && id <= ID_PRESET_60M) {
                const char* presetTimes[8] = {"00:01:00", "00:03:00", "00:05:00", "00:10:00", "00:15:00", "00:25:00", "00:30:00", "01:00:00"};
                int idx = id - ID_PRESET_1M;
                SetWindowTextA(hTmInput, presetTimes[idx]);
            }

            // Multi-Timer Handlers
            else if (id == ID_BTN_MT_ADD) {
                if (g_multiTimerCount < MAX_MULTI_TIMERS) {
                    char nameBuf[32] = {0};
                    char timeBuf[32] = {0};
                    GetWindowTextA(hEditMtName, nameBuf, sizeof(nameBuf) - 1);
                    GetWindowTextA(hEditMtTime, timeBuf, sizeof(timeBuf) - 1);
                    DWORD ms = ParseTimerInput(timeBuf);
                    if (ms > 0) {
                        lstrcpyA(g_multiTimers[g_multiTimerCount].name, nameBuf[0] ? nameBuf : "Timer");
                        g_multiTimers[g_multiTimerCount].totalMs = ms;
                        g_multiTimers[g_multiTimerCount].remainingMs = ms;
                        g_multiTimers[g_multiTimerCount].lastTick = GetTickCount();
                        g_multiTimers[g_multiTimerCount].isRunning = 1;
                        g_multiTimerCount++;
                        SwitchMode(MODE_MULTI);
                    }
                }
            } else if (id == ID_BTN_MT_STARTALL) {
                DWORD now = GetTickCount();
                for (int i = 0; i < g_multiTimerCount; i++) {
                    if (g_multiTimers[i].remainingMs > 0) {
                        g_multiTimers[i].isRunning = 1;
                        g_multiTimers[i].lastTick = now;
                    }
                }
            } else if (id == ID_BTN_MT_PAUSEALL) {
                for (int i = 0; i < g_multiTimerCount; i++) {
                    g_multiTimers[i].isRunning = 0;
                }
            } else if (id == ID_BTN_MT_DEL) {
                int sel = (int)SendMessageA(hListMt, LB_GETCURSEL, 0, 0);
                if (sel >= 0 && sel < g_multiTimerCount) {
                    for (int i = sel; i < g_multiTimerCount - 1; i++) {
                        g_multiTimers[i] = g_multiTimers[i + 1];
                    }
                    g_multiTimerCount--;
                    SwitchMode(MODE_MULTI);
                }
            }

            // Pomodoro Handlers
            else if (id == ID_BTN_POMO_START) {
                if (g_pomoIsRunning) {
                    g_pomoIsRunning = 0;
                    SetWindowTextA(hBtnPomoStart, "Resume");
                } else {
                    if (g_pomoRemainingMs > 0) {
                        g_pomoIsRunning = 1;
                        g_pomoTargetTime = GetTickCount() + g_pomoRemainingMs;
                        SetWindowTextA(hBtnPomoStart, "Pause");
                    }
                }
            } else if (id == ID_BTN_POMO_SKIP) {
                g_pomoIsRunning = 0;
                SetWindowTextA(hBtnPomoStart, "Start");
                if (g_pomoState == POMO_WORK) {
                    g_pomoState = POMO_SHORT_BREAK;
                    g_pomoTotalMs = 5 * 60 * 1000;
                } else {
                    g_pomoState = POMO_WORK;
                    g_pomoTotalMs = 25 * 60 * 1000;
                }
                g_pomoRemainingMs = g_pomoTotalMs;
                UpdatePomodoroDisplay();
            } else if (id == ID_BTN_POMO_RESET) {
                g_pomoIsRunning = 0;
                g_pomoState = POMO_WORK;
                g_pomoCycleCount = 1;
                g_pomoTotalMs = 25 * 60 * 1000;
                g_pomoRemainingMs = g_pomoTotalMs;
                SetWindowTextA(hBtnPomoStart, "Start");
                UpdatePomodoroDisplay();
            }

            break;
        }
        case WM_TIMER: {
            if (g_swIsRunning) UpdateStopwatchDisplay();
            if (g_tmIsRunning) UpdateTimerDisplay();
            if (g_pomoIsRunning) UpdatePomodoroDisplay();
            UpdateMultiTimers();
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(18, 19, 24));
            SetTextColor(hdc, RGB(90, 139, 212));
            return (LRESULT)hBgBrush;
        }
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(26, 28, 35));
            SetTextColor(hdc, RGB(240, 242, 245));
            return (LRESULT)hControlBrush;
        }
        case WM_DESTROY: {
            KillTimer(hwnd, 1);
            if (hFontDisplay) DeleteObject(hFontDisplay);
            if (hFontBtn) DeleteObject(hFontBtn);
            if (hFontSmall) DeleteObject(hFontSmall);
            if (hBgBrush) DeleteObject(hBgBrush);
            if (hControlBrush) DeleteObject(hControlBrush);
            if (hProgressBarBrush) DeleteObject(hProgressBarBrush);
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
    wc.hbrBackground = CreateSolidBrush(RGB(18, 19, 24));

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KTimerClass", "KTimer", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 416, 420, NULL, NULL, wc.hInstance, NULL);

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
