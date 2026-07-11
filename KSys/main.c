#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#define W 400
#define H 300

HWND hOutput;

void formatInt(DWORD v, char* s) {
    char tmp[32];
    int i = 0;
    if (v == 0) { s[0] = '0'; s[1] = 0; return; }
    while (v > 0) {
        tmp[i++] = (char)((v % 10) + '0');
        v /= 10;
    }
    int j = 0;
    while (i > 0) {
        s[j++] = tmp[--i];
    }
    s[j] = 0;
}

void strcatA(char* dest, const char* src) {
    while (*dest) dest++;
    while (*src) *dest++ = *src++;
    *dest = 0;
}

void UpdateInfo() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    
    MEMORYSTATUSEX mem = {0};
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);
    
    char buf[2048] = {0};
    char numBuf[64];
    
    strcatA(buf, "--- System Information ---\r\n\r\n");
    
    strcatA(buf, "Architecture: ");
    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) strcatA(buf, "x64 (AMD64)\r\n");
    else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM) strcatA(buf, "ARM\r\n");
    else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) strcatA(buf, "x86 (Intel)\r\n");
    else strcatA(buf, "Unknown\r\n");
    
    strcatA(buf, "Processor Cores: ");
    formatInt(si.dwNumberOfProcessors, numBuf);
    strcatA(buf, numBuf);
    strcatA(buf, "\r\n");
    
    strcatA(buf, "Page Size: ");
    formatInt(si.dwPageSize, numBuf);
    strcatA(buf, numBuf);
    strcatA(buf, " bytes\r\n\r\n");
    
    strcatA(buf, "--- Memory Information ---\r\n\r\n");
    
    strcatA(buf, "Memory Load: ");
    formatInt(mem.dwMemoryLoad, numBuf);
    strcatA(buf, numBuf);
    strcatA(buf, "%\r\n");
    
    strcatA(buf, "Total Physical: ");
    formatInt((DWORD)(mem.ullTotalPhys >> 20), numBuf);
    strcatA(buf, numBuf);
    strcatA(buf, " MB\r\n");
    
    strcatA(buf, "Available Physical: ");
    formatInt((DWORD)(mem.ullAvailPhys >> 20), numBuf);
    strcatA(buf, numBuf);
    strcatA(buf, " MB\r\n");
    
    strcatA(buf, "Total Page File: ");
    formatInt((DWORD)(mem.ullTotalPageFile >> 20), numBuf);
    strcatA(buf, numBuf);
    strcatA(buf, " MB\r\n\r\n");
    
    ULARGE_INTEGER freeBytesAvailableToCaller;
    ULARGE_INTEGER totalNumberOfBytes;
    ULARGE_INTEGER totalNumberOfFreeBytes;
    if (GetDiskFreeSpaceExA("C:\\", &freeBytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        strcatA(buf, "--- Disk Information (C:\\) ---\r\n\r\n");
        strcatA(buf, "Total Space: ");
        formatInt((DWORD)(totalNumberOfBytes.QuadPart >> 20), numBuf);
        strcatA(buf, numBuf);
        strcatA(buf, " MB\r\n");
        
        strcatA(buf, "Free Space: ");
        formatInt((DWORD)(totalNumberOfFreeBytes.QuadPart >> 20), numBuf);
        strcatA(buf, numBuf);
        strcatA(buf, " MB\r\n\r\n");
    }
    
    strcatA(buf, "--- System Uptime ---\r\n\r\n");
    DWORD ticks = GetTickCount();
    DWORD hours = ticks / 3600000;
    DWORD mins = (ticks / 60000) % 60;
    DWORD secs = (ticks / 1000) % 60;
    strcatA(buf, "Uptime: ");
    formatInt(hours, numBuf); strcatA(buf, numBuf); strcatA(buf, "h ");
    formatInt(mins, numBuf); strcatA(buf, numBuf); strcatA(buf, "m ");
    formatInt(secs, numBuf); strcatA(buf, numBuf); strcatA(buf, "s\r\n");
    
    SetWindowTextA(hOutput, buf);
}

BOOL CALLBACK SetFontProc(HWND child, LPARAM hFont) {
    SendMessage(child, WM_SETFONT, hFont, TRUE);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
            if (!hFont) hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Courier New");
            
            hOutput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY, 
                10, 10, W - 35, H - 55, hwnd, NULL, NULL, NULL);
            
            EnumChildWindows(hwnd, SetFontProc, (LPARAM)hFont);
            
            UpdateInfo();
            SetTimer(hwnd, 1, 1000, NULL);
            break;
        }
        case WM_TIMER: {
            if (wParam == 1) {
                UpdateInfo();
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            MoveWindow(hOutput, 10, 10, nw - 20, nh - 20, TRUE);
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
    wc.lpszClassName = "KSysApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KSysApp", "KSys", WS_OVERLAPPEDWINDOW,
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
