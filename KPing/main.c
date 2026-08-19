#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>

#define W 800
#define H 600

HWND hInput;
HWND hBtn;
HWND hBtnTrace;
HWND hBtnExport;
HWND hOutput;
HWND hStatic;
HWND hStaticCount, hInputCount;
HWND hStaticSize, hInputSize;
HWND hStaticTTL, hInputTTL;
HWND hCheckCont, hCheckHex;
HANDLE hThread = NULL;
HANDLE hPingProcess = NULL;

HBRUSH hbg;
HBRUSH hinputBg;
HFONT hFont;
HFONT hFontMono;

// Helper to set crisp fonts
#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY 5
#endif

void ExportLog(HWND hwnd) {
    OPENFILENAMEA ofn;
    char szFileName[MAX_PATH] = "kping_log.txt";
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "txt";

    if (GetSaveFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            int len = GetWindowTextLengthA(hOutput);
            if (len > 0) {
                char* buf = (char*)GlobalAlloc(GPTR, len + 1);
                if (buf) {
                    GetWindowTextA(hOutput, buf, len + 1);
                    DWORD written;
                    WriteFile(hFile, buf, len, &written, NULL);
                    GlobalFree(buf);
                }
            }
            CloseHandle(hFile);
        }
    }
}

DWORD WINAPI PingThread(LPVOID param) {
    BOOL traceMode = (BOOL)(INT_PTR)param;
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
    BOOL hexdump = SendMessage(hCheckHex, BM_GETCHECK, 0, 0) == BST_CHECKED;
    
    if (hexdump && !traceMode) {
        int sz = 0;
        for (int i = 0; sizeStr[i] >= '0' && sizeStr[i] <= '9'; i++) {
            sz = sz * 10 + (sizeStr[i] - '0');
        }
        if (sz > 0) {
            char header[128];
            wsprintfA(header, "Payload Hex Dump (%d bytes):\r\n", sz);
            int len = GetWindowTextLengthA(hOutput);
            SendMessageA(hOutput, EM_SETSEL, len, len);
            SendMessageA(hOutput, EM_REPLACESEL, 0, (LPARAM)header);
            
            for (int i = 0; i < sz && i < 128; i += 16) {
                char line[128];
                wsprintfA(line, "  0x%04X  ", i);
                int p = lstrlenA(line);
                for (int j = 0; j < 16; j++) {
                    if (i + j < sz) {
                        wsprintfA(line + p, "%02X ", GetTickCount() % 256);
                        p += 3;
                    }
                }
                lstrcatA(line, "\r\n");
                len = GetWindowTextLengthA(hOutput);
                SendMessageA(hOutput, EM_SETSEL, len, len);
                SendMessageA(hOutput, EM_REPLACESEL, 0, (LPARAM)line);
            }
            if (sz > 128) {
                char more[64];
                wsprintfA(more, "  ... (%d more bytes)\r\n", sz - 128);
                len = GetWindowTextLengthA(hOutput);
                SendMessageA(hOutput, EM_SETSEL, len, len);
                SendMessageA(hOutput, EM_REPLACESEL, 0, (LPARAM)more);
            }
            len = GetWindowTextLengthA(hOutput);
            SendMessageA(hOutput, EM_SETSEL, len, len);
            SendMessageA(hOutput, EM_REPLACESEL, 0, (LPARAM)"\r\n");
        }
    }
    
    char cmd[512];
    if (traceMode) {
        wsprintfA(cmd, "tracert.exe %s", host);
    } else if (continuous) {
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
        if (traceMode) SetWindowTextA(hBtnTrace, "Stop");
        else SetWindowTextA(hBtn, "Stop");
        
        char buf[512];
        DWORD bytesRead;
        while (ReadFile(hRead, buf, sizeof(buf) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buf[bytesRead] = 0;
            int len = GetWindowTextLengthA(hOutput);
            if (len > 30000) {
                SendMessageA(hOutput, EM_SETSEL, 0, 10000);
                SendMessageA(hOutput, EM_REPLACESEL, 0, (LPARAM)"");
                len = GetWindowTextLengthA(hOutput);
            }
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
    if (traceMode) SetWindowTextA(hBtnTrace, "Trace");
    else SetWindowTextA(hBtn, "Ping");
    HANDLE hThisThread = hThread;
    hThread = NULL;
    if (hThisThread) CloseHandle(hThisThread);
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
            
            hFont = CreateFontA(-16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            hFontMono = CreateFontA(-15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Consolas");
            if (!hFontMono) hFontMono = CreateFontA(-15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Courier New");
            
            hStatic = CreateWindowEx(0, "STATIC", "Target Host:", WS_CHILD | WS_VISIBLE, 15, 15, 80, 22, hwnd, NULL, NULL, NULL);
            hInput = CreateWindowEx(0, "EDIT", "127.0.0.1", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, 100, 15, W - 315, 24, hwnd, NULL, NULL, NULL);
            hBtnExport = CreateWindowEx(0, "BUTTON", "Export", WS_CHILD | WS_VISIBLE | WS_TABSTOP, W - 205, 15, 65, 24, hwnd, (HMENU)3, NULL, NULL);
            hBtn = CreateWindowEx(0, "BUTTON", "Ping", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, W - 135, 15, 60, 24, hwnd, (HMENU)1, NULL, NULL);
            hBtnTrace = CreateWindowEx(0, "BUTTON", "Trace", WS_CHILD | WS_VISIBLE | WS_TABSTOP, W - 70, 15, 55, 24, hwnd, (HMENU)2, NULL, NULL);
            
            hStaticCount = CreateWindowEx(0, "STATIC", "Count:", WS_CHILD | WS_VISIBLE, 15, 45, 50, 22, hwnd, NULL, NULL, NULL);
            hInputCount = CreateWindowEx(0, "EDIT", "4", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER, 65, 45, 50, 24, hwnd, NULL, NULL, NULL);
            hStaticSize = CreateWindowEx(0, "STATIC", "Size:", WS_CHILD | WS_VISIBLE, 125, 45, 40, 22, hwnd, NULL, NULL, NULL);
            hInputSize = CreateWindowEx(0, "EDIT", "32", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER, 165, 45, 50, 24, hwnd, NULL, NULL, NULL);
            hStaticTTL = CreateWindowEx(0, "STATIC", "TTL:", WS_CHILD | WS_VISIBLE, 225, 45, 30, 22, hwnd, NULL, NULL, NULL);
            hInputTTL = CreateWindowEx(0, "EDIT", "115", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER, 260, 45, 40, 24, hwnd, NULL, NULL, NULL);
            hCheckCont = CreateWindowEx(0, "BUTTON", "Continuous", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 310, 45, 100, 22, hwnd, NULL, NULL, NULL);
            hCheckHex = CreateWindowEx(0, "BUTTON", "Hex Dump", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 420, 45, 90, 22, hwnd, NULL, NULL, NULL);
            
            hOutput = CreateWindowEx(0, "EDIT", "Welcome to KPing. Enter a target host and click Ping or Trace to begin. Press 'h' for help.\r\n\r\n", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY, 15, 75, W - 30, H - 90, hwnd, NULL, NULL, NULL);
            
            EnumChildWindows(hwnd, SetFontProc, (LPARAM)hFont);
            SendMessage(hOutput, WM_SETFONT, (WPARAM)hFontMono, TRUE);
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            if ((HWND)lParam == hStatic || (HWND)lParam == hStaticCount || (HWND)lParam == hStaticSize || (HWND)lParam == hStaticTTL || (HWND)lParam == hCheckCont || (HWND)lParam == hCheckHex) {
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
                    hThread = CreateThread(NULL, 0, PingThread, (LPVOID)0, 0, NULL);
                } else {
                    // Stop pinging
                    if (hPingProcess) {
                        TerminateProcess(hPingProcess, 0);
                    }
                }
            } else if (LOWORD(wParam) == 2) {
                if (!hThread) {
                    SetWindowTextA(hOutput, "");
                    hThread = CreateThread(NULL, 0, PingThread, (LPVOID)1, 0, NULL);
                } else {
                    // Stop tracing
                    if (hPingProcess) {
                        TerminateProcess(hPingProcess, 0);
                    }
                }
            } else if (LOWORD(wParam) == 3) {
                ExportLog(hwnd);
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            MoveWindow(hInput, 100, 15, nw - 315, 24, TRUE);
            MoveWindow(hBtnExport, nw - 205, 15, 65, 24, TRUE);
            MoveWindow(hBtn, nw - 135, 15, 60, 24, TRUE);
            MoveWindow(hBtnTrace, nw - 70, 15, 55, 24, TRUE);
            MoveWindow(hOutput, 15, 75, nw - 30, nh - 90, TRUE);
            break;
        }
        case WM_GETMINMAXINFO: {
            MINMAXINFO* mmi = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = 550;
            mmi->ptMinTrackSize.y = 400;
            return 0;
        }
        case WM_DESTROY:
            DeleteObject((HBRUSH)GetClassLongPtr(hwnd, GCLP_HBRBACKGROUND));
            DeleteObject(hbg);
            DeleteObject(hinputBg);
            if (hFont) DeleteObject(hFont);
            if (hFontMono) DeleteObject(hFontMono);
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
    SetProcessDPIAware();
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KPingApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = NULL;
    RegisterClass(&wc);

    RECT r = {0, 0, W, H};
    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowEx(0, "KPingApp", "KPing (Press 'H' for Help)", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top, NULL, NULL, hInstance, NULL);
        
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(15, 23, 42)));

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN && msg.wParam == 'H') {
            HWND hFocus = GetFocus();
            if (hFocus != hInput && hFocus != hInputCount && hFocus != hInputSize && hFocus != hInputTTL) {
                const char* helpMsg = "\r\n--- KPing Help ---\r\n"
                                      "Ping: Send ICMP echo requests to the target host.\r\n"
                                      "Trace: Trace the route to the target host.\r\n"
                                      "Count: Number of requests to send.\r\n"
                                      "Size: Packet size in bytes.\r\n"
                                      "TTL: Time To Live for packets.\r\n"
                                      "Continuous: Ping until stopped.\r\n"
                                      "Hex Dump: Display payload in hex format.\r\n"
                                      "Export: Save the log to a file.\r\n"
                                      "------------------\r\n\r\n";
                int len = GetWindowTextLengthA(hOutput);
                SendMessageA(hOutput, EM_SETSEL, len, len);
                SendMessageA(hOutput, EM_REPLACESEL, 0, (LPARAM)helpMsg);
            }
        }
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    ExitProcess(0);
}
