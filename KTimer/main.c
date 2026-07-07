#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 300
#define H 200

HWND hDisplay, hInput, hBtnStart, hBtnReset;
BOOL running = FALSE;
BOOL countdown = FALSE;
DWORD startTime = 0;
DWORD totalMs = 0;
DWORD targetMs = 0;

void formatTime(DWORD ms, char* buf) {
    DWORD totalSec = ms / 1000;
    DWORD deciseconds = (ms % 1000) / 10;
    DWORD sec = totalSec % 60;
    DWORD min = totalSec / 60;
    wsprintfA(buf, "%02u:%02u.%02u", min, sec, deciseconds);
}

void parseInput() {
    char buf[64];
    GetWindowTextA(hInput, buf, 64);
    DWORD min = 0, sec = 0;
    
    char* p = buf;
    while (*p && *p != ':') {
        if (*p >= '0' && *p <= '9') min = min * 10 + (*p - '0');
        p++;
    }
    if (*p == ':') {
        p++;
        while (*p) {
            if (*p >= '0' && *p <= '9') sec = sec * 10 + (*p - '0');
            p++;
        }
    } else {
        // Only seconds provided
        sec = min;
        min = 0;
    }
    targetMs = (min * 60 + sec) * 1000;
}

void UpdateDisplay(HWND hwnd) {
    DWORD current = GetTickCount();
    DWORD elapsed = totalMs;
    if (running) {
        elapsed += (current - startTime);
    }
    
    char buf[64];
    if (countdown) {
        if (elapsed >= targetMs) {
            formatTime(0, buf);
            if (running) {
                running = FALSE;
                SetWindowTextA(hBtnStart, "Start");
                KillTimer(hwnd, 1);
                MessageBeep(MB_ICONASTERISK);
            }
        } else {
            formatTime(targetMs - elapsed, buf);
        }
    } else {
        formatTime(elapsed, buf);
    }
    
    SetWindowTextA(hDisplay, buf);
}

BOOL CALLBACK SetFontProc(HWND child, LPARAM hFont) {
    SendMessage(child, WM_SETFONT, hFont, TRUE);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            HFONT hBigFont = CreateFontA(48, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
            
            hDisplay = CreateWindowEx(0, "STATIC", "00:00.00", WS_CHILD | WS_VISIBLE | SS_CENTER, 10, 10, W - 35, 60, hwnd, NULL, NULL, NULL);
            SendMessage(hDisplay, WM_SETFONT, (WPARAM)hBigFont, TRUE);
            
            CreateWindowEx(0, "STATIC", "Target (mm:ss):", WS_CHILD | WS_VISIBLE, 10, 80, 100, 20, hwnd, NULL, NULL, NULL);
            hInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 110, 78, 100, 22, hwnd, NULL, NULL, NULL);
            
            hBtnStart = CreateWindowEx(0, "BUTTON", "Start", WS_CHILD | WS_VISIBLE, 10, 120, 80, 30, hwnd, (HMENU)1, NULL, NULL);
            hBtnReset = CreateWindowEx(0, "BUTTON", "Reset", WS_CHILD | WS_VISIBLE, 100, 120, 80, 30, hwnd, (HMENU)2, NULL, NULL);
            
            EnumChildWindows(hwnd, SetFontProc, (LPARAM)hFont);
            SendMessage(hDisplay, WM_SETFONT, (WPARAM)hBigFont, TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) { // Start/Stop
                if (running) {
                    running = FALSE;
                    totalMs += (GetTickCount() - startTime);
                    SetWindowTextA(hBtnStart, "Start");
                    KillTimer(hwnd, 1);
                } else {
                    if (totalMs == 0) {
                        parseInput();
                        countdown = (targetMs > 0);
                    }
                    running = TRUE;
                    startTime = GetTickCount();
                    SetWindowTextA(hBtnStart, "Stop");
                    SetTimer(hwnd, 1, 30, NULL);
                }
                UpdateDisplay(hwnd);
            } else if (LOWORD(wParam) == 2) { // Reset
                running = FALSE;
                totalMs = 0;
                SetWindowTextA(hBtnStart, "Start");
                KillTimer(hwnd, 1);
                
                parseInput();
                countdown = (targetMs > 0);
                UpdateDisplay(hwnd);
            }
            break;
        }
        case WM_TIMER: {
            if (wParam == 1) {
                UpdateDisplay(hwnd);
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) *bytes++ = (char)c;
    return dest;
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KTimerApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KTimerApp", "KTimer", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, W, H, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
