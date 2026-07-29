#include <windows.h>
#include <stdio.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

HWND hClockDisplay, hWorldDisplay, hDisplay, hTimerDisplay;
HWND hBtnStart, hBtnStop, hBtnReset, hBtnLap;
HWND hEditTimerMins, hBtnTimerStart, hBtnTimerReset;
HWND hListBox;
HWND hEditAlarmHour, hEditAlarmMin, hBtnAddAlarm, hBtnDelAlarm, hBtnToggleAlarm, hAlarmList;
HWND hStatusDisplay, hBtnSilenceAlarm, hBtnWorldCity;

#define MAX_ALARMS 20
typedef struct {
    int hour;
    int min;
    int active;
    int triggered;
} Alarm;
Alarm alarms[MAX_ALARMS];
int numAlarms = 0;

DWORD startTime = 0;
DWORD elapsed = 0;
int isRunning = 0;
int lapCount = 0;

int tmRunning = 0;
DWORD tmStartTime = 0;
DWORD tmDuration = 0;
DWORD tmRemaining = 0;
int alarmRinging = 0;

HFONT hFont, hFontMono, hFontSmall;
HBRUSH hBkBrush;

// String caching to eliminate GDI flicker & wasteful SetWindowText calls
char lastClockBuf[32] = {0};
char lastWorldBuf[64] = {0};
char lastSwBuf[32] = {0};
char lastTmBuf[32] = {0};

typedef struct {
    const char* name;
    int offsetHours;
    int offsetMins;
} WorldCity;

WorldCity cities[] = {
    {"Tokyo (JST)", 9, 0},
    {"London (BST/GMT)", 1, 0},
    {"New York (EDT)", -4, 0},
    {"Sydney (AEST)", 10, 0},
    {"New Delhi (IST)", 5, 30},
    {"UTC", 0, 0}
};
#define NUM_CITIES 6
int currentCityIndex = 0;

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

    // 2. World Clock with Safe Bounds Math
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
    wsprintfA(worldBuf, "%s: %02d:%02d %s", city.name, h, m, diffStr);
    if (!StrEquals(worldBuf, lastWorldBuf)) {
        StrCopy(lastWorldBuf, worldBuf);
        SetWindowTextA(hWorldDisplay, worldBuf);
    }

    // 3. Check Alarms Safely without Blocking Main Thread
    for (int i = 0; i < numAlarms; i++) {
        if (alarms[i].active && alarms[i].hour == st.wHour && alarms[i].min == st.wMinute) {
            if (!alarms[i].triggered) {
                alarms[i].triggered = 1;
                alarmRinging = 1;
                MessageBeep(MB_ICONASTERISK);
                SetWindowTextA(hStatusDisplay, "⏰ ALARM RINGING!");
                ShowWindow(hBtnSilenceAlarm, SW_SHOW);
            }
        } else if (alarms[i].hour != st.wHour || alarms[i].min != st.wMinute) {
            alarms[i].triggered = 0;
        }
    }

    // 4. Stopwatch
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

    // 5. Timer
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hFont = CreateFontA(26, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Consolas");
            hFontMono = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Consolas");
            hFontSmall = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
            hBkBrush = CreateSolidBrush(RGB(240, 243, 246));

            // Local Time
            CreateWindowA("STATIC", "Local Time:", WS_CHILD | WS_VISIBLE, 10, 5, 240, 15, hwnd, NULL, NULL, NULL);
            hClockDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "00:00:00", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 10, 20, 240, 35, hwnd, NULL, NULL, NULL);
            SendMessageA(hClockDisplay, WM_SETFONT, (WPARAM)hFont, TRUE);

            // World Clock
            CreateWindowA("STATIC", "World Clock:", WS_CHILD | WS_VISIBLE, 10, 60, 140, 15, hwnd, NULL, NULL, NULL);
            hBtnWorldCity = CreateWindowA("BUTTON", "Cycle City 🌍", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 155, 57, 95, 20, hwnd, (HMENU)9, NULL, NULL);
            hWorldDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "Loading...", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 10, 80, 240, 28, hwnd, NULL, NULL, NULL);
            SendMessageA(hWorldDisplay, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

            // Stopwatch
            CreateWindowA("STATIC", "Stopwatch:", WS_CHILD | WS_VISIBLE, 10, 115, 240, 15, hwnd, NULL, NULL, NULL);
            hDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "00:00.00", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 10, 130, 240, 35, hwnd, NULL, NULL, NULL);
            SendMessageA(hDisplay, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnStart = CreateWindowA("BUTTON", "Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 170, 55, 25, hwnd, (HMENU)1, NULL, NULL);
            hBtnStop = CreateWindowA("BUTTON", "Stop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 70, 170, 55, 25, hwnd, (HMENU)2, NULL, NULL);
            hBtnLap = CreateWindowA("BUTTON", "Lap", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 130, 170, 55, 25, hwnd, (HMENU)4, NULL, NULL);
            hBtnReset = CreateWindowA("BUTTON", "Reset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 190, 170, 60, 25, hwnd, (HMENU)3, NULL, NULL);
            
            hListBox = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 10, 200, 240, 60, hwnd, NULL, NULL, NULL);
            SendMessageA(hListBox, WM_SETFONT, (WPARAM)hFontMono, TRUE);

            // Timer
            CreateWindowA("STATIC", "Timer:", WS_CHILD | WS_VISIBLE, 10, 268, 45, 15, hwnd, NULL, NULL, NULL);
            hEditTimerMins = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "5", WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_CENTER, 55, 265, 35, 20, hwnd, (HMENU)10, NULL, NULL);
            CreateWindowA("STATIC", "min", WS_CHILD | WS_VISIBLE, 95, 268, 30, 15, hwnd, NULL, NULL, NULL);

            hBtnTimerStart = CreateWindowA("BUTTON", "Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 130, 263, 55, 25, hwnd, (HMENU)5, NULL, NULL);
            hBtnTimerReset = CreateWindowA("BUTTON", "Reset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 190, 263, 60, 25, hwnd, (HMENU)6, NULL, NULL);

            hTimerDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "00:00", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 10, 292, 240, 35, hwnd, NULL, NULL, NULL);
            SendMessageA(hTimerDisplay, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Alarms
            CreateWindowA("STATIC", "Alarms (HH:MM):", WS_CHILD | WS_VISIBLE, 10, 333, 240, 15, hwnd, NULL, NULL, NULL);
            hEditAlarmHour = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "08", WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_CENTER, 10, 350, 30, 20, hwnd, (HMENU)20, NULL, NULL);
            CreateWindowA("STATIC", ":", WS_CHILD | WS_VISIBLE, 42, 352, 10, 15, hwnd, NULL, NULL, NULL);
            hEditAlarmMin = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "00", WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_CENTER, 52, 350, 30, 20, hwnd, (HMENU)21, NULL, NULL);
            hBtnAddAlarm = CreateWindowA("BUTTON", "Add", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 90, 348, 45, 25, hwnd, (HMENU)7, NULL, NULL);
            hBtnDelAlarm = CreateWindowA("BUTTON", "Del", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 140, 348, 45, 25, hwnd, (HMENU)8, NULL, NULL);
            hBtnToggleAlarm = CreateWindowA("BUTTON", "Toggle", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 190, 348, 60, 25, hwnd, (HMENU)12, NULL, NULL);
            
            hAlarmList = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 10, 376, 240, 60, hwnd, NULL, NULL, NULL);
            SendMessageA(hAlarmList, WM_SETFONT, (WPARAM)hFontMono, TRUE);

            // Status Bar & Non-Blocking Dismiss Button
            hStatusDisplay = CreateWindowA("STATIC", "Ready", WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, 10, 442, 140, 25, hwnd, NULL, NULL, NULL);
            SendMessageA(hStatusDisplay, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            hBtnSilenceAlarm = CreateWindowA("BUTTON", "Dismiss 🔔", WS_CHILD | BS_PUSHBUTTON, 155, 442, 95, 25, hwnd, (HMENU)11, NULL, NULL);

            SetTimer(hwnd, 1, 15, NULL); // 15ms timer for stopwatch precision, text cached to avoid GDI flicker
            break;
        }
        case WM_ERASEBKGND:
            return 1; // Prevent full window background erase flicker

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
                SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
                UpdateDisplays(hwnd);
            } else if (id == 4) { // SW Lap
                if (isRunning) {
                    lapCount++;
                    DWORD currentMs = elapsed + (GetTickCount() - startTime);
                    char timeBuf[32];
                    FormatTime(currentMs, timeBuf);
                    char lapStr[128];
                    wsprintfA(lapStr, "Lap %d:\t%s", lapCount, timeBuf);
                    SendMessageA(hListBox, LB_INSERTSTRING, 0, (LPARAM)lapStr);
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
                        alarms[numAlarms].active = 1;
                        alarms[numAlarms].triggered = 0;
                        numAlarms++;
                        char buf[32];
                        wsprintfA(buf, "[ON]  %02d:%02d", h, m);
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
            } else if (id == 11) { // Silence / Dismiss Alarm
                alarmRinging = 0;
                SetWindowTextA(hStatusDisplay, "Ready");
                ShowWindow(hBtnSilenceAlarm, SW_HIDE);
            } else if (id == 12) { // Toggle Alarm Enable/Disable
                int sel = SendMessage(hAlarmList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR && sel < numAlarms) {
                    alarms[sel].active = !alarms[sel].active;
                    alarms[sel].triggered = 0;
                    char buf[32];
                    wsprintfA(buf, "[%s]  %02d:%02d", alarms[sel].active ? "ON" : "OFF", alarms[sel].hour, alarms[sel].min);
                    SendMessageA(hAlarmList, LB_DELETESTRING, sel, 0);
                    SendMessageA(hAlarmList, LB_INSERTSTRING, sel, (LPARAM)buf);
                }
            }
            break;
        }
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
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KClockClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    RegisterClassA(&wc);
    
    // Client area size: 260x475
    RECT rc = {0, 0, 260, 475};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, FALSE);
    
    HWND hwnd = CreateWindowExA(WS_EX_COMPOSITED, "KClockClass", "KClock", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
