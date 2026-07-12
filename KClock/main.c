#include <windows.h>
#include <stdio.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

HWND hClockDisplay, hDisplay, hTimerDisplay;
HWND hBtnStart, hBtnStop, hBtnReset, hBtnLap;
HWND hEditTimerMins, hBtnTimerStart, hBtnTimerReset;
HWND hListBox;
DWORD startTime = 0;
DWORD elapsed = 0;
int isRunning = 0;
int lapCount = 0;

int tmRunning = 0;
DWORD tmStartTime = 0;
DWORD tmDuration = 0;
DWORD tmRemaining = 0;

HFONT hFont, hFontMono, hFontSmall;

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

void UpdateDisplays() {
    // Clock
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buf[32];
    wsprintfA(buf, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
    SetWindowTextA(hClockDisplay, buf);

    // Stopwatch
    DWORD currentMs = elapsed;
    if (isRunning) {
        currentMs += (GetTickCount() - startTime);
    }
    char swBuf[16];
    FormatTime(currentMs, swBuf);
    SetWindowTextA(hDisplay, swBuf);

    // Timer
    DWORD tmCur = tmRemaining;
    if (tmRunning) {
        DWORD pass = GetTickCount() - tmStartTime;
        if (pass >= tmDuration) {
            tmRunning = 0;
            tmRemaining = 0;
            tmCur = 0;
            SetWindowTextA(hBtnTimerStart, "Start");
            MessageBeep(MB_ICONEXCLAMATION);
            MessageBoxA(NULL, "Timer Finished!", "KClock Timer", MB_OK);
        } else {
            tmCur = tmDuration - pass;
        }
    }
    char tmBuf[16];
    FormatTimer(tmCur, tmBuf);
    SetWindowTextA(hTimerDisplay, tmBuf);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hFont = CreateFontA(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Consolas");
            hFontMono = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Consolas");
            hFontSmall = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");

            CreateWindowA("STATIC", "Local Time:", WS_CHILD | WS_VISIBLE, 10, 5, 100, 15, hwnd, NULL, NULL, NULL);
            hClockDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "00:00:00", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 10, 20, 220, 35, hwnd, NULL, NULL, NULL);
            SendMessageA(hClockDisplay, WM_SETFONT, (WPARAM)hFont, TRUE);

            CreateWindowA("STATIC", "Stopwatch:", WS_CHILD | WS_VISIBLE, 10, 60, 100, 15, hwnd, NULL, NULL, NULL);
            hDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "00:00.00", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 10, 75, 220, 35, hwnd, NULL, NULL, NULL);
            SendMessageA(hDisplay, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnStart = CreateWindowA("BUTTON", "Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 115, 50, 25, hwnd, (HMENU)1, NULL, NULL);
            hBtnStop = CreateWindowA("BUTTON", "Stop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 65, 115, 50, 25, hwnd, (HMENU)2, NULL, NULL);
            hBtnLap = CreateWindowA("BUTTON", "Lap", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 120, 115, 50, 25, hwnd, (HMENU)4, NULL, NULL);
            hBtnReset = CreateWindowA("BUTTON", "Reset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 175, 115, 50, 25, hwnd, (HMENU)3, NULL, NULL);
            
            hListBox = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 10, 145, 220, 80, hwnd, NULL, NULL, NULL);
            SendMessageA(hListBox, WM_SETFONT, (WPARAM)hFontMono, TRUE);

            CreateWindowA("STATIC", "Timer:", WS_CHILD | WS_VISIBLE, 10, 235, 50, 15, hwnd, NULL, NULL, NULL);
            hEditTimerMins = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "5", WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_CENTER, 60, 232, 30, 20, hwnd, (HMENU)10, NULL, NULL);
            CreateWindowA("STATIC", "min", WS_CHILD | WS_VISIBLE, 95, 235, 30, 15, hwnd, NULL, NULL, NULL);

            hBtnTimerStart = CreateWindowA("BUTTON", "Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 130, 230, 50, 25, hwnd, (HMENU)5, NULL, NULL);
            hBtnTimerReset = CreateWindowA("BUTTON", "Reset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 185, 230, 45, 25, hwnd, (HMENU)6, NULL, NULL);

            hTimerDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "00:00", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 10, 260, 220, 35, hwnd, NULL, NULL, NULL);
            SendMessageA(hTimerDisplay, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            SetTimer(hwnd, 1, 15, NULL); // Roughly 60fps update
            break;
        }
        case WM_TIMER: {
            UpdateDisplays();
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
                    UpdateDisplays();
                }
            } else if (id == 3) { // SW Reset
                isRunning = 0;
                elapsed = 0;
                lapCount = 0;
                SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
                UpdateDisplays();
            } else if (id == 4) { // SW Lap
                if (isRunning) {
                    lapCount++;
                    DWORD currentMs = elapsed + (GetTickCount() - startTime);
                    char timeBuf[16];
                    FormatTime(currentMs, timeBuf);
                    char lapStr[64];
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
                        if(mins <= 0) mins = 5;
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
                UpdateDisplays();
            }
            break;
        }
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            if (hFont) DeleteObject(hFont);
            if (hFontMono) DeleteObject(hFontMono);
            if (hFontSmall) DeleteObject(hFontSmall);
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
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1); // standard dialog background

    RegisterClassA(&wc);
    
    // Client area size: 240x305
    RECT rc = {0, 0, 240, 305};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, FALSE);
    
    HWND hwnd = CreateWindowExA(0, "KClockClass", "KClock", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
