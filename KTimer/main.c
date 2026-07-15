#include <windows.h>
#include <stdio.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

int sprintf(char* buf, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = wvsprintfA(buf, fmt, args);
    va_end(args);
    return ret;
}

HWND hDisplay, hBtnStart, hBtnReset, hBtnLap, hListLaps;
DWORD startTime = 0;
DWORD elapsed = 0;
int isRunning = 0;
char timeBuf[32] = "00:00:00.000";
int lapCount = 0;


void UpdateDisplay() {
    DWORD current = elapsed;
    if (isRunning) {
        current += GetTickCount() - startTime;
    }
    
    DWORD ms = current % 1000;
    DWORD s = (current / 1000) % 60;
    DWORD m = (current / 60000) % 60;
    DWORD h = (current / 3600000);
    
    sprintf(timeBuf, "%02d:%02d:%02d.%03d", h, m, s, ms);
    SetWindowTextA(hDisplay, timeBuf);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hDisplay = CreateWindowExA(0, "STATIC", "00:00:00.000", WS_CHILD | WS_VISIBLE | SS_CENTER, 20, 20, 240, 50, hwnd, NULL, NULL, NULL);
            hBtnStart = CreateWindowA("BUTTON", "Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 80, 70, 30, hwnd, (HMENU)1001, NULL, NULL);
            hBtnLap = CreateWindowA("BUTTON", "Lap", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 105, 80, 70, 30, hwnd, (HMENU)1003, NULL, NULL);
            hBtnReset = CreateWindowA("BUTTON", "Reset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 190, 80, 70, 30, hwnd, (HMENU)1002, NULL, NULL);
            hListLaps = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT, 20, 120, 240, 170, hwnd, (HMENU)1004, NULL, NULL);
            
            HFONT hFont = CreateFontA(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Consolas");
            SendMessageA(hDisplay, WM_SETFONT, (WPARAM)hFont, TRUE);
            HFONT hBtnFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Tahoma");
            SendMessageA(hBtnStart, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessageA(hBtnLap, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessageA(hBtnReset, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessageA(hListLaps, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            
            SetTimer(hwnd, 1, 10, NULL);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1001) {
                if (isRunning) {
                    isRunning = 0;
                    elapsed += GetTickCount() - startTime;
                    SetWindowTextA(hBtnStart, "Start");
                } else {
                    isRunning = 1;
                    startTime = GetTickCount();
                    SetWindowTextA(hBtnStart, "Stop");
                }
            } else if (LOWORD(wParam) == 1002) {
                isRunning = 0;
                elapsed = 0;
                lapCount = 0;
                SetWindowTextA(hBtnStart, "Start");
                SendMessageA(hListLaps, LB_RESETCONTENT, 0, 0);
                UpdateDisplay();
            } else if (LOWORD(wParam) == 1003) {
                if (isRunning) {
                    lapCount++;
                    char lapBuf[64];
                    sprintf(lapBuf, "Lap %d: %s", lapCount, timeBuf);
                    SendMessageA(hListLaps, LB_ADDSTRING, 0, (LPARAM)lapBuf);
                    int count = SendMessageA(hListLaps, LB_GETCOUNT, 0, 0);
                    SendMessageA(hListLaps, LB_SETTOPINDEX, count - 1, 0);
                }
            }
            break;
        }
        case WM_TIMER: {
            if (isRunning) {
                UpdateDisplay();
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(20, 20, 20));
            SetTextColor(hdc, RGB(90, 139, 212));
            return (LRESULT)GetStockObject(BLACK_BRUSH);
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
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
    HWND hwnd = CreateWindowExA(0, "KTimerClass", "KTimer", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 350, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
