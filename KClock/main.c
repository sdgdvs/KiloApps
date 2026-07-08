#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

HWND hDisplay;
HWND hBtnStart, hBtnStop, hBtnReset, hBtnLap;
HWND hListBox;
DWORD startTime = 0;
DWORD elapsed = 0;
int isRunning = 0;
int lapCount = 0;

void FormatTime(DWORD ms, char* buf) {
    int centis = (ms % 1000) / 10;
    int seconds = (ms / 1000) % 60;
    int minutes = (ms / 60000);
    
    buf[0] = '0' + (minutes / 10);
    buf[1] = '0' + (minutes % 10);
    buf[2] = ':';
    buf[3] = '0' + (seconds / 10);
    buf[4] = '0' + (seconds % 10);
    buf[5] = '.';
    buf[6] = '0' + (centis / 10);
    buf[7] = '0' + (centis % 10);
    buf[8] = '\0';
}

void UpdateDisplay() {
    DWORD currentMs = elapsed;
    if (isRunning) {
        currentMs += (GetTickCount() - startTime);
    }
    char buf[16];
    FormatTime(currentMs, buf);
    SetWindowTextA(hDisplay, buf);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "00:00.00", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 10, 10, 160, 40, hwnd, NULL, NULL, NULL);
            HFONT hFont = CreateFontA(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Consolas");
            SendMessageA(hDisplay, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnStart = CreateWindowA("BUTTON", "Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 60, 50, 25, hwnd, (HMENU)1, NULL, NULL);
            hBtnStop = CreateWindowA("BUTTON", "Stop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 65, 60, 50, 25, hwnd, (HMENU)2, NULL, NULL);
            hBtnLap = CreateWindowA("BUTTON", "Lap", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 120, 60, 50, 25, hwnd, (HMENU)4, NULL, NULL);
            hBtnReset = CreateWindowA("BUTTON", "Reset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 175, 60, 50, 25, hwnd, (HMENU)3, NULL, NULL);
            
            hListBox = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 10, 95, 215, 140, hwnd, NULL, NULL, NULL);
            HFONT hFontMono = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Consolas");
            SendMessageA(hListBox, WM_SETFONT, (WPARAM)hFontMono, TRUE);
            
            SetTimer(hwnd, 1, 15, NULL); // Roughly 60fps update
            break;
        }
        case WM_TIMER: {
            if (isRunning) UpdateDisplay();
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == 1) { // Start
                if (!isRunning) {
                    startTime = GetTickCount();
                    isRunning = 1;
                }
            } else if (id == 2) { // Stop
                if (isRunning) {
                    elapsed += (GetTickCount() - startTime);
                    isRunning = 0;
                    UpdateDisplay();
                }
            } else if (id == 3) { // Reset
                isRunning = 0;
                elapsed = 0;
                lapCount = 0;
                SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
                UpdateDisplay();
            } else if (id == 4) { // Lap
                if (isRunning) {
                    lapCount++;
                    DWORD currentMs = elapsed + (GetTickCount() - startTime);
                    char timeBuf[16];
                    FormatTime(currentMs, timeBuf);
                    
                    char lapStr[64];
                    wsprintfA(lapStr, "Lap %d:\t%s", lapCount, timeBuf);
                    SendMessageA(hListBox, LB_INSERTSTRING, 0, (LPARAM)lapStr);
                }
            }
            break;
        }
        case WM_DESTROY:
            KillTimer(hwnd, 1);
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
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);

    RegisterClassA(&wc);
    
    // Client area size: 235x245
    RECT rc = {0, 0, 235, 245};
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
