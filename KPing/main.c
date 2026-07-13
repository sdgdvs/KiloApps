#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 500
#define H 400

HWND hInput;
HWND hBtn;
HWND hOutput;
HWND hStatic;
HWND hStaticCount, hInputCount;
HWND hStaticSize, hInputSize;
HWND hStaticTTL, hInputTTL;
HWND hCheckCont;
HANDLE hThread = NULL;
HANDLE hPingProcess = NULL;

HBRUSH hbg;
HBRUSH hinputBg;

DWORD WINAPI PingThread(LPVOID param) {
    char host[256];
    GetWindowTextA(hInput, host, 256);
    
    char countStr[32] = "4";
    GetWindowTextA(hInputCount, countStr, 32);
    if (countStr[0] == 0) lstrcpyA(countStr, "4");
    
    char sizeStr[32] = "32";
    GetWindowTextA(hInputSize, sizeStr, 32);
    if (sizeStr[0] == 0) lstrcpyA(sizeStr, "32");
    
    char ttlStr[32] = "115";
    GetWindowTextA(hInputTTL, ttlStr, 32);
    if (ttlStr[0] == 0) lstrcpyA(ttlStr, "115");
    
    BOOL continuous = SendMessage(hCheckCont, BM_GETCHECK, 0, 0) == BST_CHECKED;
    
    char cmd[512];
    if (continuous) {
        wsprintfA(cmd, "ping.exe %s -t -l %s -i %s", host, sizeStr, ttlStr);
    } else {
        wsprintfA(cmd, "ping.exe %s -n %s -l %s -i %s", host, countStr, sizeStr, ttlStr);
    }
    
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    HANDLE hRead, hWrite;
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) return 0;
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);
    
    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;
    si.wShowWindow = SW_HIDE;
    
    PROCESS_INFORMATION pi;
    if (CreateProcessA(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(hWrite);
        hPingProcess = pi.hProcess;
        SetWindowTextA(hBtn, "Stop");
        
        char buf[512];
        DWORD bytesRead;
        while (ReadFile(hRead, buf, sizeof(buf) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buf[bytesRead] = 0;
            int len = GetWindowTextLengthA(hOutput);
            SendMessageA(hOutput, EM_SETSEL, len, len);
            
            char formatBuf[1024];
            int j = 0;
            for (DWORD i = 0; i < bytesRead; i++) {
                if (buf[i] == '\n' && (i == 0 || buf[i-1] != '\r')) {
                    formatBuf[j++] = '\r';
                }
                formatBuf[j++] = buf[i];
            }
            formatBuf[j] = 0;
            
            SendMessageA(hOutput, EM_REPLACESEL, 0, (LPARAM)formatBuf);
        }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        hPingProcess = NULL;
    } else {
        CloseHandle(hWrite);
    }
    CloseHandle(hRead);
    SetWindowTextA(hBtn, "Ping");
    hThread = NULL;
    return 0;
}

BOOL CALLBACK SetFontProc(HWND child, LPARAM hFont) {
    SendMessage(child, WM_SETFONT, hFont, TRUE);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hbg = CreateSolidBrush(RGB(15, 23, 42)); // #0f172a
            hinputBg = CreateSolidBrush(RGB(30, 41, 59)); // #1e293b
            
            HFONT hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            HFONT hFontMono = CreateFontA(15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
            if (!hFontMono) hFontMono = CreateFontA(15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Courier New");
            
            hStatic = CreateWindowEx(0, "STATIC", "Target Host:", WS_CHILD | WS_VISIBLE, 15, 15, 80, 22, hwnd, NULL, NULL, NULL);
            hInput = CreateWindowEx(0, "EDIT", "127.0.0.1", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 100, 15, W - 180, 24, hwnd, NULL, NULL, NULL);
            hBtn = CreateWindowEx(0, "BUTTON", "Ping", WS_CHILD | WS_VISIBLE, W - 75, 15, 60, 24, hwnd, (HMENU)1, NULL, NULL);
            
            hStaticCount = CreateWindowEx(0, "STATIC", "Count:", WS_CHILD | WS_VISIBLE, 15, 45, 50, 22, hwnd, NULL, NULL, NULL);
            hInputCount = CreateWindowEx(0, "EDIT", "4", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, 65, 45, 50, 24, hwnd, NULL, NULL, NULL);
            hStaticSize = CreateWindowEx(0, "STATIC", "Size:", WS_CHILD | WS_VISIBLE, 125, 45, 40, 22, hwnd, NULL, NULL, NULL);
            hInputSize = CreateWindowEx(0, "EDIT", "32", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, 165, 45, 50, 24, hwnd, NULL, NULL, NULL);
            hStaticTTL = CreateWindowEx(0, "STATIC", "TTL:", WS_CHILD | WS_VISIBLE, 225, 45, 30, 22, hwnd, NULL, NULL, NULL);
            hInputTTL = CreateWindowEx(0, "EDIT", "115", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, 260, 45, 40, 24, hwnd, NULL, NULL, NULL);
            hCheckCont = CreateWindowEx(0, "BUTTON", "Continuous", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 310, 45, 100, 22, hwnd, NULL, NULL, NULL);
            
            hOutput = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY, 15, 75, W - 30, H - 120, hwnd, NULL, NULL, NULL);
            
            EnumChildWindows(hwnd, SetFontProc, (LPARAM)hFont);
            SendMessage(hOutput, WM_SETFONT, (WPARAM)hFontMono, TRUE);
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            if ((HWND)lParam == hStatic || (HWND)lParam == hStaticCount || (HWND)lParam == hStaticSize || (HWND)lParam == hStaticTTL || (HWND)lParam == hCheckCont) {
                SetTextColor(hdc, RGB(226, 232, 240)); // #e2e8f0
                SetBkColor(hdc, RGB(15, 23, 42));
                return (LRESULT)hbg;
            } else if ((HWND)lParam == hOutput || (HWND)lParam == hInput || (HWND)lParam == hInputCount || (HWND)lParam == hInputSize || (HWND)lParam == hInputTTL) {
                SetTextColor(hdc, (HWND)lParam == hOutput ? RGB(163, 190, 140) : RGB(226, 232, 240)); // Green for output, white for input
                SetBkColor(hdc, RGB(30, 41, 59));
                return (LRESULT)hinputBg;
            }
            break;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(163, 190, 140));
            SetBkColor(hdc, RGB(30, 41, 59));
            return (LRESULT)hinputBg;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                if (!hThread) {
                    SetWindowTextA(hOutput, "");
                    hThread = CreateThread(NULL, 0, PingThread, NULL, 0, NULL);
                } else {
                    // Stop pinging
                    if (hPingProcess) {
                        TerminateProcess(hPingProcess, 0);
                    }
                }
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            MoveWindow(hInput, 100, 15, nw - 180, 24, TRUE);
            MoveWindow(hBtn, nw - 75, 15, 60, 24, TRUE);
            MoveWindow(hOutput, 15, 75, nw - 30, nh - 120, TRUE);
            break;
        }
        case WM_DESTROY:
            DeleteObject(hbg);
            DeleteObject(hinputBg);
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
    wc.lpszClassName = "KPingApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = NULL;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KPingApp", "KPing", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, W, H, NULL, NULL, hInstance, NULL);
        
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(15, 23, 42)));

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
