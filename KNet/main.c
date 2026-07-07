#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>

#define W 600
#define H 500

HWND hUrlEdit;
HWND hGoBtn;
HWND hContentEdit;

void FetchUrl(HWND hwnd) {
    char* url = (char*)VirtualAlloc(NULL, 512, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    GetWindowTextA(hUrlEdit, url, 512);
    
    if (url[0] == '\0') {
        VirtualFree(url, 0, MEM_RELEASE);
        return;
    }
    
    SetWindowTextA(hContentEdit, "Fetching...");
    
    HINTERNET hInternet = InternetOpenA("KNet/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) {
        SetWindowTextA(hContentEdit, "InternetOpen failed.");
        VirtualFree(url, 0, MEM_RELEASE);
        return;
    }
    
    HINTERNET hUrl = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    VirtualFree(url, 0, MEM_RELEASE);
    
    if (!hUrl) {
        SetWindowTextA(hContentEdit, "InternetOpenUrl failed. Make sure to include http:// or https://");
        InternetCloseHandle(hInternet);
        return;
    }
    
    // Read response
    char* buffer = (char*)VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    DWORD bytesRead;
    
    // Pre-allocate a large buffer (very simple, bounded to ~64KB for this minimal app)
    int maxData = 65535;
    char* data = (char*)VirtualAlloc(NULL, maxData, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    int offset = 0;
    
    while (InternetReadFile(hUrl, buffer, 4095, &bytesRead) && bytesRead > 0) {
        if (offset + bytesRead < maxData - 1) {
            for (DWORD i = 0; i < bytesRead; i++) {
                if (buffer[i] != '\r') {
                    data[offset++] = buffer[i];
                }
            }
        }
    }
    data[offset] = '\0';
    VirtualFree(buffer, 0, MEM_RELEASE);
    
    // Convert newlines to \r\n for the Edit control
    int finalMax = 131072;
    char* finalData = (char*)VirtualAlloc(NULL, finalMax, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    int j = 0;
    for (int i = 0; i < offset && j < finalMax - 2; i++) {
        if (data[i] == '\n') {
            finalData[j++] = '\r';
            finalData[j++] = '\n';
        } else {
            finalData[j++] = data[i];
        }
    }
    finalData[j] = '\0';
    
    SetWindowTextA(hContentEdit, finalData);
    
    VirtualFree(finalData, 0, MEM_RELEASE);
    VirtualFree(data, 0, MEM_RELEASE);
    
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            HFONT hFontMono = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
            
            hUrlEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "http://example.com",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                10, 10, W - 100, 24, hwnd, NULL, NULL, NULL);
            SendMessage(hUrlEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hGoBtn = CreateWindowEx(0, "BUTTON", "Go",
                WS_CHILD | WS_VISIBLE,
                W - 80, 10, 60, 24, hwnd, (HMENU)1, NULL, NULL);
            SendMessage(hGoBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hContentEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                10, 44, W - 35, H - 95, hwnd, NULL, NULL, NULL);
            SendMessage(hContentEdit, WM_SETFONT, (WPARAM)hFontMono, TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                FetchUrl(hwnd);
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            MoveWindow(hUrlEdit, 10, 10, nw - 100, 24, TRUE);
            MoveWindow(hGoBtn, nw - 80, 10, 60, 24, TRUE);
            MoveWindow(hContentEdit, 10, 44, nw - 20, nh - 55, TRUE);
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
    wc.lpszClassName = "KNetApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KNetApp", "KNet", WS_OVERLAPPEDWINDOW,
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
