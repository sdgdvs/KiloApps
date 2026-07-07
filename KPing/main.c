#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 400
#define H 300

HWND hInput;
HWND hBtn;
HWND hOutput;
HANDLE hThread = NULL;

DWORD WINAPI PingThread(LPVOID param) {
    char host[256];
    GetWindowTextA(hInput, host, 256);
    
    char cmd[512];
    wsprintfA(cmd, "ping.exe %s", host);
    
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
        
        char buf[512];
        DWORD bytesRead;
        while (ReadFile(hRead, buf, sizeof(buf) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buf[bytesRead] = 0;
            // Append to edit control
            int len = GetWindowTextLengthA(hOutput);
            SendMessageA(hOutput, EM_SETSEL, len, len);
            
            // Replace \n with \r\n
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
    } else {
        CloseHandle(hWrite);
    }
    CloseHandle(hRead);
    EnableWindow(hBtn, TRUE);
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
            HFONT hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            HFONT hFontMono = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
            if (!hFontMono) hFontMono = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Courier New");
            
            CreateWindowEx(0, "STATIC", "Host:", WS_CHILD | WS_VISIBLE, 10, 12, 40, 20, hwnd, NULL, NULL, NULL);
            hInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "127.0.0.1", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 10, W - 140, 22, hwnd, NULL, NULL, NULL);
            hBtn = CreateWindowEx(0, "BUTTON", "Ping", WS_CHILD | WS_VISIBLE, W - 80, 10, 60, 22, hwnd, (HMENU)1, NULL, NULL);
            
            hOutput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY, 10, 40, W - 35, H - 90, hwnd, NULL, NULL, NULL);
            
            EnumChildWindows(hwnd, SetFontProc, (LPARAM)hFont);
            SendMessage(hOutput, WM_SETFONT, (WPARAM)hFontMono, TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                if (!hThread) {
                    SetWindowTextA(hOutput, "");
                    EnableWindow(hBtn, FALSE);
                    hThread = CreateThread(NULL, 0, PingThread, NULL, 0, NULL);
                }
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            MoveWindow(hInput, 50, 10, nw - 120, 22, TRUE);
            MoveWindow(hBtn, nw - 65, 10, 55, 22, TRUE);
            MoveWindow(hOutput, 10, 40, nw - 20, nh - 50, TRUE);
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
    wc.lpszClassName = "KPingApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KPingApp", "KPing", WS_OVERLAPPEDWINDOW,
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
