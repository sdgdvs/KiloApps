#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) {
        *bytes++ = (char)c;
    }
    return dest;
}

#define W 350
#define H 170

HWND hTitle;
HWND hEditUrl;
HWND hBtnPlay;
HWND hBtnStop;

char mciCmd[1024] = {0};

void PlayStream(HWND hwnd) {
    char url[512] = {0};
    GetWindowTextA(hEditUrl, url, sizeof(url));
    if (url[0] == '\0') return;

    SetWindowTextA(hTitle, "Status: Connecting...");
    mciSendStringA("close myStream", NULL, 0, NULL);
    
    wsprintfA(mciCmd, "open \"%s\" alias myStream", url);
    MCIERROR err = mciSendStringA(mciCmd, NULL, 0, NULL);
    if (err == 0) {
        mciSendStringA("play myStream", NULL, 0, NULL);
        SetWindowTextA(hTitle, "Status: Playing");
    } else {
        SetWindowTextA(hTitle, "Status: Error opening stream (Format unsupported?)");
    }
}

void StopStream() {
    mciSendStringA("stop myStream", NULL, 0, NULL);
    mciSendStringA("close myStream", NULL, 0, NULL);
    SetWindowTextA(hTitle, "Status: Stopped");
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            hEditUrl = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "https://radio.erb.pw/public/subspace",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                10, 20, W - 36, 25, hwnd, NULL, NULL, NULL);
            SendMessage(hEditUrl, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnPlay = CreateWindowEx(0, "BUTTON", "Play",
                WS_CHILD | WS_VISIBLE,
                10, 60, 80, 30, hwnd, (HMENU)1, NULL, NULL);
            SendMessage(hBtnPlay, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnStop = CreateWindowEx(0, "BUTTON", "Stop",
                WS_CHILD | WS_VISIBLE,
                100, 60, 80, 30, hwnd, (HMENU)2, NULL, NULL);
            SendMessage(hBtnStop, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hTitle = CreateWindowEx(0, "STATIC", "Status: Ready",
                WS_CHILD | WS_VISIBLE,
                10, 100, W - 36, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Auto play on startup
            PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(1, 0), 0);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                PlayStream(hwnd);
            } else if (LOWORD(wParam) == 2) {
                StopStream();
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }
        case WM_DESTROY:
            mciSendStringA("close myStream", NULL, 0, NULL);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KRadioApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KRadioApp", "KRadio", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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
