#include <windows.h>
#include <stdio.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

HWND hClockDisplay, hWorldDisplay, hTzCalcDisplay, hDisplay, hTimerDisplay;
HWND hBtnStart, hBtnStop, hBtnReset, hBtnLap;
HWND hEditTimerMins, hBtnTimerStart, hBtnTimerReset;
HWND hListBox;
HWND hEditAlarmHour, hEditAlarmMin, hBtnAddAlarm, hBtnDelAlarm, hBtnToggleAlarm, hAlarmList;
HWND hStatusDisplay, hBtnSilenceAlarm, hBtnSnoozeAlarm, hBtnWorldCity, hBtnTzCalc, hBtnExport, hBtnImport;

#define MAX_ALARMS 20
#define MAX_LAPS 50

typedef struct {
    int hour;
    int min;
    int daysMask; // Bitmask: bit 0 = Sun, bit 1 = Mon, ..., bit 6 = Sat (127 = Daily)
    int active;
    int triggered;
    char label[32];
} Alarm;

Alarm alarms[MAX_ALARMS];
int numAlarms = 0;

DWORD startTime = 0;
DWORD elapsed = 0;
int isRunning = 0;

int lapCount = 0;
DWORD lapSplits[MAX_LAPS];
DWORD lapTotals[MAX_LAPS];
DWORD lastLapTotal = 0;

int tmRunning = 0;
DWORD tmStartTime = 0;
DWORD tmDuration = 0;
DWORD tmRemaining = 0;
int alarmRinging = 0;
int ringingAlarmIndex = -1;

HFONT hFont, hFontMono, hFontSmall;
HBRUSH hBkBrush;

// String caching to eliminate GDI flicker & wasteful SetWindowText calls
char lastClockBuf[32] = {0};
char lastWorldBuf[64] = {0};
char lastTzCalcBuf[64] = {0};
char lastSwBuf[32] = {0};
char lastTmBuf[32] = {0};

typedef struct {
    const char* name;
    const char* code;
    int offsetHours;
    int offsetMins;
} WorldCity;

WorldCity cities[] = {
    {"New York", "EDT", -4, 0},
    {"London", "BST", 1, 0},
    {"Paris", "CEST", 2, 0},
    {"Tokyo", "JST", 9, 0},
    {"Sydney", "AEST", 10, 0},
    {"New Delhi", "IST", 5, 30},
    {"Dubai", "GST", 4, 0},
    {"Los Angeles", "PDT", -7, 0},
    {"UTC", "UTC", 0, 0}
};
#define NUM_CITIES 9
int currentCityIndex = 0;
int tzSrcCityIndex = 0;
int tzTgtCityIndex = 3; // Tokyo

void FormatTime(DWORD ms, char* buf) {
    int centis = (ms % 1000) / 10;
    int seconds = (ms / 1000) % 60;
    int minutes = (ms / 60000);
    wsprintfA(buf, "%02d:%02d.%02d", minutes, seconds, centis);
}

void FormatTimer(DWORD ms, char* buf) {
    int seconds = (ms / 1000) % 60;
    int minutes = (ms / 60000);
    wsprintfA(buf, "%02d:%02d", minutes, seconds);
}

int StrEquals(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

void StrCopy(char* dest, const char* src) {
    while ((*dest++ = *src++));
}

void FormatDaysMask(int mask, char* buf) {
    if (mask == 127) {
        wsprintfA(buf, "Daily");
    } else if (mask == 62) {
        wsprintfA(buf, "Mon-Fri");
    } else if (mask == 65) {
        wsprintfA(buf, "Sat-Sun");
    } else {
        wsprintfA(buf, "Custom");
    }
}

void RebuildLapList() {
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    if (lapCount == 0) return;

    DWORD minSplit = 0xFFFFFFFF;
    DWORD maxSplit = 0;
    int fastestIdx = -1;
    int slowestIdx = -1;

    if (lapCount >= 2) {
        for (int i = 0; i < lapCount; i++) {
            if (lapSplits[i] < minSplit) { minSplit = lapSplits[i]; fastestIdx = i; }
            if (lapSplits[i] > maxSplit) { maxSplit = lapSplits[i]; slowestIdx = i; }
        }
    }

    for (int i = lapCount - 1; i >= 0; i--) {
        char splitBuf[32], totalBuf[32], lapStr[128];
        FormatTime(lapSplits[i], splitBuf);
        FormatTime(lapTotals[i], totalBuf);

        const char* badge = "";
        if (i == fastestIdx && lapCount >= 2) badge = " [FAST]";
        else if (i == slowestIdx && lapCount >= 2) badge = " [SLOW]";

        wsprintfA(lapStr, "Lap %d: +%s (Total: %s)%s", i + 1, splitBuf, totalBuf, badge);
        SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)lapStr);
    }
}

void UpdateDisplays(HWND hwnd) {
    // 1. Local Clock
    SYSTEMTIME st;
    GetLocalTime(&st);
    char clockBuf[32];
    wsprintfA(clockBuf, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
    if (!StrEquals(clockBuf, lastClockBuf)) {
        StrCopy(lastClockBuf, clockBuf);
        SetWindowTextA(hClockDisplay, clockBuf);
    }

    // 2. World Clock with Offset Math
    SYSTEMTIME stUtc;
    GetSystemTime(&stUtc);
    WorldCity city = cities[currentCityIndex];
    int h = (int)stUtc.wHour + city.offsetHours;
    int m = (int)stUtc.wMinute + city.offsetMins;
    int dayDiff = 0;

    if (m >= 60) { m -= 60; h++; }
    else if (m < 0) { m += 60; h--; }

    if (h >= 24) { h -= 24; dayDiff = 1; }
    else if (h < 0) { h += 24; dayDiff = -1; }

    char worldBuf[64];
    const char* diffStr = (dayDiff > 0) ? "(+1d)" : ((dayDiff < 0) ? "(-1d)" : "(Today)");
    wsprintfA(worldBuf, "%s (%s): %02d:%02d %s", city.name, city.code, h, m, diffStr);
    if (!StrEquals(worldBuf, lastWorldBuf)) {
        StrCopy(lastWorldBuf, worldBuf);
        SetWindowTextA(hWorldDisplay, worldBuf);
    }

    // 3. Timezone Difference Calculator Display
    WorldCity srcC = cities[tzSrcCityIndex];
    WorldCity tgtC = cities[tzTgtCityIndex];
    int srcMins = srcC.offsetHours * 60 + srcC.offsetMins;
    int tgtMins = tgtC.offsetHours * 60 + tgtC.offsetMins;
    int diffMins = tgtMins - srcMins;
    int diffH = diffMins / 60;
    int diffM = diffMins % 60;
    if (diffM < 0) diffM = -diffM;

    char tzCalcBuf[64];
    wsprintfA(tzCalcBuf, "%s -> %s: %s%dh%dm", srcC.code, tgtC.code, (diffMins >= 0 ? "+" : "-"), diffH < 0 ? -diffH : diffH, diffM);
    if (!StrEquals(tzCalcBuf, lastTzCalcBuf)) {
        StrCopy(lastTzCalcBuf, tzCalcBuf);
        SetWindowTextA(hTzCalcDisplay, tzCalcBuf);
    }

    // 4. Repeating Alarms Check (Bitmask day check)
    int currentDayBit = (1 << st.wDayOfWeek);
    for (int i = 0; i < numAlarms; i++) {
        if (alarms[i].active && (alarms[i].daysMask & currentDayBit) && alarms[i].hour == st.wHour && alarms[i].min == st.wMinute) {
            if (!alarms[i].triggered) {
                alarms[i].triggered = 1;
                alarmRinging = 1;
                ringingAlarmIndex = i;
                MessageBeep(MB_ICONASTERISK);
                SetWindowTextA(hStatusDisplay, "⏰ ALARM RINGING!");
                ShowWindow(hBtnSilenceAlarm, SW_SHOW);
                ShowWindow(hBtnSnoozeAlarm, SW_SHOW);
            }
        } else if (alarms[i].hour != st.wHour || alarms[i].min != st.wMinute) {
            alarms[i].triggered = 0;
        }
    }

    // 5. Stopwatch
    DWORD currentMs = elapsed;
    if (isRunning) {
        currentMs += (GetTickCount() - startTime);
    }
    char swBuf[32];
    FormatTime(currentMs, swBuf);
    if (!StrEquals(swBuf, lastSwBuf)) {
        StrCopy(lastSwBuf, swBuf);
        SetWindowTextA(hDisplay, swBuf);
    }

    // 6. Timer
    DWORD tmCur = tmRemaining;
    if (tmRunning) {
        DWORD pass = GetTickCount() - tmStartTime;
        if (pass >= tmDuration) {
            tmRunning = 0;
            tmRemaining = 0;
            tmCur = 0;
            SetWindowTextA(hBtnTimerStart, "Start");
            MessageBeep(MB_ICONEXCLAMATION);
            SetWindowTextA(hStatusDisplay, "⏲️ TIMER FINISHED!");
            ShowWindow(hBtnSilenceAlarm, SW_SHOW);
        } else {
            tmCur = tmDuration - pass;
        }
    }
    char tmBuf[32];
    FormatTimer(tmCur, tmBuf);
    if (!StrEquals(tmBuf, lastTmBuf)) {
        StrCopy(lastTmBuf, tmBuf);
        SetWindowTextA(hTimerDisplay, tmBuf);
    }
}

void SaveConfig() {
    HANDLE hFile = CreateFileA("kclock_config.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[256];
        DWORD bytesWritten;
        wsprintfA(buf, "NUM_ALARMS=%d\n", numAlarms);
        WriteFile(hFile, buf, (DWORD)lstrlenA(buf), &bytesWritten, NULL);

        for (int i = 0; i < numAlarms; i++) {
            wsprintfA(buf, "ALARM=%02d:%02d,%d,%d\n", alarms[i].hour, alarms[i].min, alarms[i].active, alarms[i].daysMask);
            WriteFile(hFile, buf, (DWORD)lstrlenA(buf), &bytesWritten, NULL);
        }
        CloseHandle(hFile);
        MessageBoxA(NULL, "Config exported to kclock_config.txt", "KClock Backup", MB_OK | MB_ICONINFORMATION);
    }
}

void LoadConfig() {
    HANDLE hFile = CreateFileA("kclock_config.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buffer[1024] = {0};
        DWORD bytesRead;
        if (ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            SendMessage(hAlarmList, LB_RESETCONTENT, 0, 0);
            numAlarms = 0;

            char* line = buffer;
            while (*line) {
                if (line[0] == 'A' && line[1] == 'L' && line[2] == 'A' && line[3] == 'R' && line[4] == 'M' && line[5] == '=') {
                    int h = 0, m = 0, active = 1, days = 127;
                    char* ptr = line + 6;
                    h = (ptr[0] - '0') * 10 + (ptr[1] - '0');
                    m = (ptr[3] - '0') * 10 + (ptr[4] - '0');
                    if (numAlarms < MAX_ALARMS) {
                        alarms[numAlarms].hour = h;
                        alarms[numAlarms].min = m;
                        alarms[numAlarms].active = active;
                        alarms[numAlarms].daysMask = days;
                        alarms[numAlarms].triggered = 0;
                        numAlarms++;

                        char dayStr[16], listBuf[48];
                        FormatDaysMask(days, dayStr);
                        wsprintfA(listBuf, "[ON-%s] %02d:%02d", dayStr, h, m);
                        SendMessageA(hAlarmList, LB_ADDSTRING, 0, (LPARAM)listBuf);
                    }
                }
                while (*line && *line != '\n') line++;
                if (*line == '\n') line++;
            }
            MessageBoxA(NULL, "Config imported successfully!", "KClock Backup", MB_OK | MB_ICONINFORMATION);
        }
        CloseHandle(hFile);
    } else {
        MessageBoxA(NULL, "No saved kclock_config.txt found.", "KClock Backup", MB_OK | MB_ICONWARNING);
    }
}

BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam) {
    SendMessage(hwnd, WM_SETFONT, (WPARAM)lParam, TRUE);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HDC hdc = GetDC(hwnd);
            int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(hwnd, hdc);
            
            hFont = CreateFontA(-MulDiv(24, dpi, 72), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Consolas");
            hFontMono = CreateFontA(-MulDiv(12, dpi, 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Consolas");
            hFontSmall = CreateFontA(-MulDiv(14, dpi, 72), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            hBkBrush = CreateSolidBrush(RGB(240, 243, 246));

            // Local Time
            CreateWindowA("STATIC", "Local Time:", WS_CHILD | WS_VISIBLE, 10, 5, 280, 15, hwnd, NULL, NULL, NULL);
            hClockDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "00:00:00", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 10, 20, 300, 32, hwnd, NULL, NULL, NULL);

            // World Clock
            CreateWindowA("STATIC", "World Clock:", WS_CHILD | WS_VISIBLE, 10, 56, 140, 15, hwnd, NULL, NULL, NULL);
            hBtnWorldCity = CreateWindowA("BUTTON", "Cycle City 🌍", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 155, 54, 155, 20, hwnd, (HMENU)9, NULL, NULL);
            hWorldDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "Loading...", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 10, 75, 300, 26, hwnd, NULL, NULL, NULL);

            // Timezone Calculator
            CreateWindowA("STATIC", "TZ Calculator:", WS_CHILD | WS_VISIBLE, 10, 105, 140, 15, hwnd, NULL, NULL, NULL);
            hBtnTzCalc = CreateWindowA("BUTTON", "Swap Cities ⇄", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 155, 103, 155, 20, hwnd, (HMENU)13, NULL, NULL);
            hTzCalcDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "Calculating...", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 10, 123, 300, 24, hwnd, NULL, NULL, NULL);

            // Stopwatch
            CreateWindowA("STATIC", "Stopwatch:", WS_CHILD | WS_VISIBLE, 10, 151, 280, 15, hwnd, NULL, NULL, NULL);
            hDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "00:00.00", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 10, 166, 300, 32, hwnd, NULL, NULL, NULL);

            hBtnStart = CreateWindowA("BUTTON", "Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 202, 65, 23, hwnd, (HMENU)1, NULL, NULL);
            hBtnStop = CreateWindowA("BUTTON", "Stop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 80, 202, 65, 23, hwnd, (HMENU)2, NULL, NULL);
            hBtnLap = CreateWindowA("BUTTON", "Lap", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 150, 202, 65, 23, hwnd, (HMENU)4, NULL, NULL);
            hBtnReset = CreateWindowA("BUTTON", "Reset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 220, 202, 70, 23, hwnd, (HMENU)3, NULL, NULL);
            
            hListBox = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 10, 229, 300, 55, hwnd, NULL, NULL, NULL);

            // Timer
            CreateWindowA("STATIC", "Timer:", WS_CHILD | WS_VISIBLE, 10, 288, 45, 15, hwnd, NULL, NULL, NULL);
            hEditTimerMins = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "5", WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_CENTER, 55, 286, 35, 20, hwnd, (HMENU)10, NULL, NULL);
            CreateWindowA("STATIC", "min", WS_CHILD | WS_VISIBLE, 95, 288, 30, 15, hwnd, NULL, NULL, NULL);

            hBtnTimerStart = CreateWindowA("BUTTON", "Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 130, 284, 70, 23, hwnd, (HMENU)5, NULL, NULL);
            hBtnTimerReset = CreateWindowA("BUTTON", "Reset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 205, 284, 70, 23, hwnd, (HMENU)6, NULL, NULL);

            hTimerDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "00:00", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 10, 310, 300, 32, hwnd, NULL, NULL, NULL);
            
            // Alarms
            CreateWindowA("STATIC", "Alarms (HH:MM):", WS_CHILD | WS_VISIBLE, 10, 346, 280, 15, hwnd, NULL, NULL, NULL);
            hEditAlarmHour = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "08", WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_CENTER, 10, 362, 30, 20, hwnd, (HMENU)20, NULL, NULL);
            CreateWindowA("STATIC", ":", WS_CHILD | WS_VISIBLE, 42, 364, 10, 15, hwnd, NULL, NULL, NULL);
            hEditAlarmMin = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "00", WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_CENTER, 52, 362, 30, 20, hwnd, (HMENU)21, NULL, NULL);
            hBtnAddAlarm = CreateWindowA("BUTTON", "Add", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 90, 360, 55, 23, hwnd, (HMENU)7, NULL, NULL);
            hBtnDelAlarm = CreateWindowA("BUTTON", "Del", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 150, 360, 55, 23, hwnd, (HMENU)8, NULL, NULL);
            hBtnToggleAlarm = CreateWindowA("BUTTON", "Toggle", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 210, 360, 70, 23, hwnd, (HMENU)12, NULL, NULL);
            
            hAlarmList = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 10, 387, 300, 55, hwnd, NULL, NULL, NULL);

            // Export / Import Config & Status Bar
            hBtnExport = CreateWindowA("BUTTON", "Export", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 446, 60, 23, hwnd, (HMENU)14, NULL, NULL);
            hBtnImport = CreateWindowA("BUTTON", "Import", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 75, 446, 60, 23, hwnd, (HMENU)15, NULL, NULL);
            hStatusDisplay = CreateWindowA("STATIC", "Ready (H or F1: Help)", WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, 140, 446, 170, 23, hwnd, NULL, NULL, NULL);

            hBtnSilenceAlarm = CreateWindowA("BUTTON", "Dismiss 🔔", WS_CHILD | BS_PUSHBUTTON, 10, 472, 140, 25, hwnd, (HMENU)11, NULL, NULL);
            hBtnSnoozeAlarm = CreateWindowA("BUTTON", "Snooze 5m 💤", WS_CHILD | BS_PUSHBUTTON, 160, 472, 140, 25, hwnd, (HMENU)16, NULL, NULL);

            EnumChildWindows(hwnd, EnumChildProc, (LPARAM)hFontSmall);

            SendMessageA(hClockDisplay, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hDisplay, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hTimerDisplay, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hListBox, WM_SETFONT, (WPARAM)hFontMono, TRUE);
            SendMessageA(hAlarmList, WM_SETFONT, (WPARAM)hFontMono, TRUE);

            SetTimer(hwnd, 1, 15, NULL);
            break;
        }
        case WM_ERASEBKGND:
            return 1;

        case WM_TIMER: {
            UpdateDisplays(hwnd);
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == 1) { // SW Start
                if (!isRunning) {
                    startTime = GetTickCount();
                    isRunning = 1;
                }
            } else if (id == 2) { // SW Stop
                if (isRunning) {
                    elapsed += (GetTickCount() - startTime);
                    isRunning = 0;
                    UpdateDisplays(hwnd);
                }
            } else if (id == 3) { // SW Reset
                isRunning = 0;
                elapsed = 0;
                lapCount = 0;
                lastLapTotal = 0;
                SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
                UpdateDisplays(hwnd);
            } else if (id == 4) { // SW Lap
                if (isRunning && lapCount < MAX_LAPS) {
                    DWORD currentMs = elapsed + (GetTickCount() - startTime);
                    DWORD split = currentMs - lastLapTotal;
                    lastLapTotal = currentMs;

                    lapSplits[lapCount] = split;
                    lapTotals[lapCount] = currentMs;
                    lapCount++;
                    RebuildLapList();
                }
            } else if (id == 5) { // Timer Start/Pause
                if (tmRunning) {
                    tmRemaining = tmDuration - (GetTickCount() - tmStartTime);
                    tmRunning = 0;
                    SetWindowTextA(hBtnTimerStart, "Resume");
                } else {
                    if (tmRemaining == 0) {
                        int mins = GetDlgItemInt(hwnd, 10, NULL, FALSE);
                        if (mins <= 0) mins = 5;
                        if (mins > 999) mins = 999;
                        tmDuration = mins * 60000;
                        tmRemaining = tmDuration;
                    } else {
                        tmDuration = tmRemaining;
                    }
                    if (tmDuration > 0) {
                        tmStartTime = GetTickCount();
                        tmRunning = 1;
                        SetWindowTextA(hBtnTimerStart, "Pause");
                    }
                }
            } else if (id == 6) { // Timer Reset
                tmRunning = 0;
                tmRemaining = 0;
                SetWindowTextA(hBtnTimerStart, "Start");
                UpdateDisplays(hwnd);
            } else if (id == 7) { // Add Alarm
                if (numAlarms < MAX_ALARMS) {
                    int h = GetDlgItemInt(hwnd, 20, NULL, FALSE);
                    int m = GetDlgItemInt(hwnd, 21, NULL, FALSE);
                    if (h >= 0 && h < 24 && m >= 0 && m < 60) {
                        alarms[numAlarms].hour = h;
                        alarms[numAlarms].min = m;
                        alarms[numAlarms].daysMask = 127; // Daily by default
                        alarms[numAlarms].active = 1;
                        alarms[numAlarms].triggered = 0;
                        numAlarms++;

                        char buf[48];
                        wsprintfA(buf, "[ON-Daily] %02d:%02d", h, m);
                        SendMessageA(hAlarmList, LB_ADDSTRING, 0, (LPARAM)buf);
                    } else {
                        MessageBoxA(hwnd, "Invalid time! Hour (0-23), Min (0-59)", "Error", MB_OK | MB_ICONWARNING);
                    }
                }
            } else if (id == 8) { // Del Alarm
                int sel = SendMessage(hAlarmList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR && sel < numAlarms) {
                    for (int i = sel; i < numAlarms - 1; i++) {
                        alarms[i] = alarms[i+1];
                    }
                    numAlarms--;
                    SendMessageA(hAlarmList, LB_DELETESTRING, sel, 0);
                }
            } else if (id == 9) { // Cycle World City
                currentCityIndex = (currentCityIndex + 1) % NUM_CITIES;
                UpdateDisplays(hwnd);
            } else if (id == 11) { // Dismiss Alarm
                alarmRinging = 0;
                SetWindowTextA(hStatusDisplay, "Ready (H or F1: Help)");
                ShowWindow(hBtnSilenceAlarm, SW_HIDE);
                ShowWindow(hBtnSnoozeAlarm, SW_HIDE);
            } else if (id == 12) { // Toggle Alarm Enable/Disable
                int sel = SendMessage(hAlarmList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR && sel < numAlarms) {
                    alarms[sel].active = !alarms[sel].active;
                    alarms[sel].triggered = 0;
                    char dayStr[16], buf[48];
                    FormatDaysMask(alarms[sel].daysMask, dayStr);
                    wsprintfA(buf, "[%s-%s] %02d:%02d", alarms[sel].active ? "ON" : "OFF", dayStr, alarms[sel].hour, alarms[sel].min);
                    SendMessageA(hAlarmList, LB_DELETESTRING, sel, 0);
                    SendMessageA(hAlarmList, LB_INSERTSTRING, sel, (LPARAM)buf);
                }
            } else if (id == 13) { // Swap TZ Calc Cities
                int tmp = tzSrcCityIndex;
                tzSrcCityIndex = tzTgtCityIndex;
                tzTgtCityIndex = tmp;
                UpdateDisplays(hwnd);
            } else if (id == 14) { // Export Config
                SaveConfig();
            } else if (id == 15) { // Import Config
                LoadConfig();
            } else if (id == 16) { // Snooze Alarm 5 Minutes
                if (alarmRinging && numAlarms < MAX_ALARMS) {
                    SYSTEMTIME st;
                    GetLocalTime(&st);
                    int snoozeM = st.wMinute + 5;
                    int snoozeH = st.wHour;
                    if (snoozeM >= 60) { snoozeM -= 60; snoozeH = (snoozeH + 1) % 24; }

                    alarms[numAlarms].hour = snoozeH;
                    alarms[numAlarms].min = snoozeM;
                    alarms[numAlarms].daysMask = (1 << st.wDayOfWeek);
                    alarms[numAlarms].active = 1;
                    alarms[numAlarms].triggered = 0;
                    numAlarms++;

                    char buf[48];
                    wsprintfA(buf, "[SNOOZE] %02d:%02d", snoozeH, snoozeM);
                    SendMessageA(hAlarmList, LB_ADDSTRING, 0, (LPARAM)buf);

                    alarmRinging = 0;
                    SetWindowTextA(hStatusDisplay, "Snoozed 5m");
                    ShowWindow(hBtnSilenceAlarm, SW_HIDE);
                    ShowWindow(hBtnSnoozeAlarm, SW_HIDE);
                }
            }
            break;
        }
        case WM_KEYDOWN:
            if (wParam == 'H' || wParam == 'h' || wParam == VK_F1) {
                MessageBoxA(hwnd, "KClock Help:\n\n- Tabs: Click buttons to navigate.\n- Export/Import: Save your settings to kclock_config.txt.\n- Stopwatch: Press Start/Lap/Stop.\n- World Clock: Cycle through cities.\n\nKeyboard Shortcuts:\n- Press 'H' or 'F1' to view this help.", "KClock Help", MB_OK | MB_ICONINFORMATION);
            }
            break;
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            if (hFont) DeleteObject(hFont);
            if (hFontMono) DeleteObject(hFontMono);
            if (hFontSmall) DeleteObject(hFontSmall);
            if (hBkBrush) DeleteObject(hBkBrush);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    SetProcessDPIAware();
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KClockClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    RegisterClassA(&wc);
    
    // Client area size: 340x580
    RECT rc = {0, 0, 340, 580};
    AdjustWindowRect(&rc, (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX) | WS_CLIPCHILDREN, FALSE);
    
    HWND hwnd = CreateWindowExA(WS_EX_COMPOSITED, "KClockClass", "KClock (Press H or F1 for Help)", (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX) | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
