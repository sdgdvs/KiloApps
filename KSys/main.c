#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#define W 540
#define H 420

#define ID_TAB_CTRL   1001
#define ID_TXT_MAIN   1002
#define ID_BTN_CPU    1003
#define ID_BTN_RAM    1004
#define ID_BTN_DISK   1005
#define ID_BTN_ALL    1006
#define ID_BTN_EXP_TXT 1007
#define ID_BTN_EXP_JSON 1008
#define ID_BTN_EXP_HTML 1009

HWND hTabCtrl = NULL;
HWND hOutput = NULL;
HWND hBtnCpu = NULL;
HWND hBtnRam = NULL;
HWND hBtnDisk = NULL;
HWND hBtnAll = NULL;
HWND hBtnExpTxt = NULL;
HWND hBtnExpJson = NULL;
HWND hBtnExpHtml = NULL;

char g_LogBuffer[16384] = {0};
char g_CpuResult[128] = "Not Executed";
char g_RamResult[128] = "Not Executed";
char g_DiskResult[128] = "Not Executed";

int g_CurrentTab = 0;

void LogEvent(const char* level, const char* msg) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    char timeStr[64];
    wsprintfA(timeStr, "[%02d:%02d:%02d] [%s] %s\r\n", st.wHour, st.wMinute, st.wSecond, level, msg);
    
    lstrcatA(g_LogBuffer, timeStr);
}

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

void GetSystemAuditText(char* buf, int maxLen) {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    
    MEMORYSTATUSEX mem = {0};
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);
    
    ULARGE_INTEGER freeBytesCaller, totalBytes, totalFree;
    BOOL hasDisk = GetDiskFreeSpaceExA("C:\\", &freeBytesCaller, &totalBytes, &totalFree);
    
    DWORD ticks = GetTickCount();
    DWORD hours = ticks / 3600000;
    DWORD mins = (ticks / 60000) % 60;
    DWORD secs = (ticks / 1000) % 60;
    
    HDC hdc = GetDC(NULL);
    int sw = hdc ? GetDeviceCaps(hdc, HORZRES) : 0;
    int sh = hdc ? GetDeviceCaps(hdc, VERTRES) : 0;
    int bpp = hdc ? GetDeviceCaps(hdc, BITSPIXEL) : 0;
    int hz = hdc ? GetDeviceCaps(hdc, VREFRESH) : 0;
    if (hdc) ReleaseDC(NULL, hdc);
    
    SYSTEM_POWER_STATUS sps;
    BOOL hasPower = GetSystemPowerStatus(&sps);
    
    wsprintfA(buf,
        "=================================================================\r\n"
        "       KSYS NATIVE SYSTEM DIAGNOSTICS & HARDWARE REPORT          \r\n"
        "=================================================================\r\n"
        "System Uptime      : %u h %u m %u s\r\n"
        "Architecture       : %s\r\n"
        "Logical Processors : %u Cores\r\n"
        "Page Size          : %u bytes\r\n\r\n"
        "--- MEMORY SPECIFICATIONS ---\r\n"
        "Memory Load        : %u%%\r\n"
        "Total Physical RAM : %u MB\r\n"
        "Available Physical : %u MB\r\n"
        "Total Page File    : %u MB\r\n\r\n"
        "--- DISK STORAGE (C:\\) ---\r\n"
        "Total Capacity     : %u MB\r\n"
        "Free Capacity      : %u MB\r\n\r\n"
        "--- DISPLAY & GRAPHICS ---\r\n"
        "Resolution         : %dx%d (%d-bit, %d Hz)\r\n\r\n"
        "--- POWER & BATTERY ---\r\n"
        "AC Power Line      : %s\r\n"
        "Battery Level      : %s\r\n\r\n"
        "--- DIAGNOSTIC BENCHMARK RESULTS ---\r\n"
        "CPU Multi-thread   : %s\r\n"
        "RAM Throughput     : %s\r\n"
        "Disk I/O Throughput: %s\r\n"
        "=================================================================\r\n",
        hours, mins, secs,
        (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) ? "x64 (AMD64)" :
        (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) ? "x86 (Intel)" : "ARM/Other",
        si.dwNumberOfProcessors,
        si.dwPageSize,
        mem.dwMemoryLoad,
        (DWORD)(mem.ullTotalPhys >> 20),
        (DWORD)(mem.ullAvailPhys >> 20),
        (DWORD)(mem.ullTotalPageFile >> 20),
        hasDisk ? (DWORD)(totalBytes.QuadPart >> 20) : 0,
        hasDisk ? (DWORD)(totalFree.QuadPart >> 20) : 0,
        sw, sh, bpp, hz,
        hasPower ? ((sps.ACLineStatus == 1) ? "Online (AC)" : "Offline (Battery)") : "Unknown",
        hasPower ? ((sps.BatteryLifePercent != 255) ? "Charged" : "N/A") : "N/A",
        g_CpuResult, g_RamResult, g_DiskResult
    );
}

// CPU Stress Worker
DWORD WINAPI CpuWorkerProc(LPVOID lpParam) {
    DWORD iterations = 15000000;
    double val = 1.0001;
    for (DWORD i = 0; i < iterations; i++) {
        val = val * 1.0000001 + 0.000001;
    }
    return 0;
}

void RunCpuBenchmark() {
    LogEvent("BENCH", "Running CPU Multi-Thread stress benchmark...");
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    DWORD numThreads = si.dwNumberOfProcessors;
    if (numThreads > 16) numThreads = 16;
    if (numThreads < 1) numThreads = 1;

    HANDLE hThreads[16];
    DWORD start = GetTickCount();

    for (DWORD i = 0; i < numThreads; i++) {
        hThreads[i] = CreateThread(NULL, 0, CpuWorkerProc, NULL, 0, NULL);
    }

    WaitForMultipleObjects(numThreads, hThreads, TRUE, 10000);

    for (DWORD i = 0; i < numThreads; i++) {
        CloseHandle(hThreads[i]);
    }

    DWORD elapsed = GetTickCount() - start;
    if (elapsed == 0) elapsed = 1;

    DWORD score = (numThreads * 15000) / elapsed;
    wsprintfA(g_CpuResult, "%u Ops/sec (%u ms across %u cores)", score, elapsed, numThreads);

    LogEvent("BENCH", g_CpuResult);
}

void RunRamBenchmark() {
    LogEvent("BENCH", "Running RAM Read/Write Throughput benchmark...");
    SIZE_T bufSize = 32 * 1024 * 1024; // 32MB buffer
    BYTE* ptr = (BYTE*)VirtualAlloc(NULL, bufSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!ptr) {
        wsprintfA(g_RamResult, "Memory Allocation Failed");
        return;
    }

    DWORD startWrite = GetTickCount();
    for (SIZE_T i = 0; i < bufSize; i += 4) {
        *(DWORD*)(ptr + i) = (DWORD)i;
    }
    DWORD writeTime = GetTickCount() - startWrite;
    if (writeTime == 0) writeTime = 1;

    DWORD startRead = GetTickCount();
    volatile DWORD sum = 0;
    for (SIZE_T i = 0; i < bufSize; i += 4) {
        sum += *(DWORD*)(ptr + i);
    }
    DWORD readTime = GetTickCount() - startRead;
    if (readTime == 0) readTime = 1;

    VirtualFree(ptr, 0, MEM_RELEASE);

    DWORD writeMBs = (32 * 1000) / writeTime;
    DWORD readMBs = (32 * 1000) / readTime;

    wsprintfA(g_RamResult, "Write: %u MB/s | Read: %u MB/s", writeMBs, readMBs);
    LogEvent("BENCH", g_RamResult);
}

void RunDiskBenchmark() {
    LogEvent("BENCH", "Running Disk I/O Throughput benchmark...");
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    char tempFile[MAX_PATH];
    wsprintfA(tempFile, "%sksys_bench.tmp", tempPath);

    HANDLE hFile = CreateFileA(tempFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        wsprintfA(g_DiskResult, "Disk Access Failed");
        return;
    }

    SIZE_T chunkSize = 8 * 1024 * 1024; // 8MB
    BYTE* buf = (BYTE*)VirtualAlloc(NULL, chunkSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!buf) {
        CloseHandle(hFile);
        DeleteFileA(tempFile);
        return;
    }

    DWORD written = 0;
    DWORD startWrite = GetTickCount();
    WriteFile(hFile, buf, (DWORD)chunkSize, &written, NULL);
    FlushFileBuffers(hFile);
    DWORD writeTime = GetTickCount() - startWrite;
    if (writeTime == 0) writeTime = 1;

    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    DWORD readBytes = 0;
    DWORD startRead = GetTickCount();
    ReadFile(hFile, buf, (DWORD)chunkSize, &readBytes, NULL);
    DWORD readTime = GetTickCount() - startRead;
    if (readTime == 0) readTime = 1;

    CloseHandle(hFile);
    DeleteFileA(tempFile);
    VirtualFree(buf, 0, MEM_RELEASE);

    DWORD writeMBs = (8 * 1000) / writeTime;
    DWORD readMBs = (8 * 1000) / readTime;

    wsprintfA(g_DiskResult, "Write: %u MB/s | Read: %u MB/s", writeMBs, readMBs);
    LogEvent("BENCH", g_DiskResult);
}

void SaveReportFile(const char* filename, const char* content) {
    HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hFile, content, lstrlenA(content), &written, NULL);
        CloseHandle(hFile);
        char logMsg[256];
        wsprintfA(logMsg, "Exported report to file: %s", filename);
        LogEvent("INFO", logMsg);
    }
}

void ExportReport(int type) {
    static char reportBuf[8192];
    GetSystemAuditText(reportBuf, sizeof(reportBuf));

    if (type == 0) { // TXT
        SaveReportFile("ksys_report.txt", reportBuf);
        MessageBoxA(NULL, "Report exported to ksys_report.txt", "Export Success", MB_OK | MB_ICONINFORMATION);
    } else if (type == 1) { // JSON
        char jsonBuf[12288];
        wsprintfA(jsonBuf,
            "{\n"
            "  \"app\": \"KSys Native Diagnostic Workstation\",\n"
            "  \"cpuResult\": \"%s\",\n"
            "  \"ramResult\": \"%s\",\n"
            "  \"diskResult\": \"%s\"\n"
            "}\n",
            g_CpuResult, g_RamResult, g_DiskResult
        );
        SaveReportFile("ksys_report.json", jsonBuf);
        MessageBoxA(NULL, "Report exported to ksys_report.json", "Export Success", MB_OK | MB_ICONINFORMATION);
    } else if (type == 2) { // HTML
        char htmlBuf[14336];
        wsprintfA(htmlBuf,
            "<!DOCTYPE html><html><head><title>KSys Report</title>"
            "<style>body{background:#0f172a;color:#38bdf8;font-family:monospace;padding:20px;}"
            "pre{background:#1e293b;padding:15px;border-radius:8px;color:#f8fafc;}</style></head>"
            "<body><h1>KSys Diagnostic Report</h1><pre>%s</pre></body></html>",
            reportBuf
        );
        SaveReportFile("ksys_report.html", htmlBuf);
        MessageBoxA(NULL, "Report exported to ksys_report.html", "Export Success", MB_OK | MB_ICONINFORMATION);
    }
}

void UpdateView() {
    static char contentBuf[16384];

    // Show/hide buttons based on tab
    BOOL isBenchTab = (g_CurrentTab == 1);
    BOOL isExpTab = (g_CurrentTab == 3);

    ShowWindow(hBtnCpu, isBenchTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnRam, isBenchTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnDisk, isBenchTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnAll, isBenchTab ? SW_SHOW : SW_HIDE);

    ShowWindow(hBtnExpTxt, isExpTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnExpJson, isExpTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnExpHtml, isExpTab ? SW_SHOW : SW_HIDE);

    if (g_CurrentTab == 0) { // Hardware Inspector
        GetSystemAuditText(contentBuf, sizeof(contentBuf));
        SetWindowTextA(hOutput, contentBuf);
    } else if (g_CurrentTab == 1) { // Benchmarks
        wsprintfA(contentBuf,
            "--- DIAGNOSTIC BENCHMARK SUITE ---\r\n\r\n"
            "CPU Benchmark Test  : %s\r\n"
            "RAM Throughput Test : %s\r\n"
            "Disk I/O Speed Test : %s\r\n\r\n"
            "Click buttons below to execute diagnostic performance benchmarks.\r\n",
            g_CpuResult, g_RamResult, g_DiskResult
        );
        SetWindowTextA(hOutput, contentBuf);
    } else if (g_CurrentTab == 2) { // Event History Logs
        SetWindowTextA(hOutput, g_LogBuffer);
    } else if (g_CurrentTab == 3) { // Report Export
        GetSystemAuditText(contentBuf, sizeof(contentBuf));
        SetWindowTextA(hOutput, contentBuf);
    }
}

BOOL CALLBACK SetFontProc(HWND child, LPARAM hFont) {
    SendMessage(child, WM_SETFONT, hFont, TRUE);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HFONT hFont = NULL;
    switch (msg) {
        case WM_CREATE: {
            hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
            if (!hFont) hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Courier New");
            
            InitCommonControls();

            hTabCtrl = CreateWindowEx(0, WC_TABCONTROL, "", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                5, 5, W - 25, 30, hwnd, (HMENU)ID_TAB_CTRL, NULL, NULL);

            TCITEM tie;
            tie.mask = TCIF_TEXT;
            tie.pszText = "Hardware Inspector";
            TabCtrl_InsertItem(hTabCtrl, 0, &tie);
            tie.pszText = "Diagnostic Benchmarks";
            TabCtrl_InsertItem(hTabCtrl, 1, &tie);
            tie.pszText = "Event Logs";
            TabCtrl_InsertItem(hTabCtrl, 2, &tie);
            tie.pszText = "Report Export";
            TabCtrl_InsertItem(hTabCtrl, 3, &tie);

            hOutput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY, 
                5, 40, W - 25, H - 125, hwnd, (HMENU)ID_TXT_MAIN, NULL, NULL);

            // Benchmark Buttons
            hBtnCpu = CreateWindow("BUTTON", "Test CPU", WS_CHILD | BS_PUSHBUTTON, 10, H - 75, 100, 25, hwnd, (HMENU)ID_BTN_CPU, NULL, NULL);
            hBtnRam = CreateWindow("BUTTON", "Test RAM", WS_CHILD | BS_PUSHBUTTON, 120, H - 75, 100, 25, hwnd, (HMENU)ID_BTN_RAM, NULL, NULL);
            hBtnDisk = CreateWindow("BUTTON", "Test Disk", WS_CHILD | BS_PUSHBUTTON, 230, H - 75, 100, 25, hwnd, (HMENU)ID_BTN_DISK, NULL, NULL);
            hBtnAll = CreateWindow("BUTTON", "Run All Tests", WS_CHILD | BS_PUSHBUTTON, 340, H - 75, 110, 25, hwnd, (HMENU)ID_BTN_ALL, NULL, NULL);

            // Export Buttons
            hBtnExpTxt = CreateWindow("BUTTON", "Export TXT", WS_CHILD | BS_PUSHBUTTON, 10, H - 75, 110, 25, hwnd, (HMENU)ID_BTN_EXP_TXT, NULL, NULL);
            hBtnExpJson = CreateWindow("BUTTON", "Export JSON", WS_CHILD | BS_PUSHBUTTON, 130, H - 75, 110, 25, hwnd, (HMENU)ID_BTN_EXP_JSON, NULL, NULL);
            hBtnExpHtml = CreateWindow("BUTTON", "Export HTML", WS_CHILD | BS_PUSHBUTTON, 250, H - 75, 110, 25, hwnd, (HMENU)ID_BTN_EXP_HTML, NULL, NULL);

            EnumChildWindows(hwnd, SetFontProc, (LPARAM)hFont);

            LogEvent("INFO", "KSys Workstation Diagnostics initialized");
            UpdateView();
            SetTimer(hwnd, 1, 1000, NULL);
            break;
        }
        case WM_NOTIFY: {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            if (pnmh->idFrom == ID_TAB_CTRL && pnmh->code == TCN_SELCHANGE) {
                g_CurrentTab = TabCtrl_GetCurSel(hTabCtrl);
                UpdateView();
            }
            break;
        }
        case WM_COMMAND: {
            WORD id = LOWORD(wParam);
            if (id == ID_BTN_CPU) {
                RunCpuBenchmark();
                UpdateView();
            } else if (id == ID_BTN_RAM) {
                RunRamBenchmark();
                UpdateView();
            } else if (id == ID_BTN_DISK) {
                RunDiskBenchmark();
                UpdateView();
            } else if (id == ID_BTN_ALL) {
                RunCpuBenchmark();
                RunRamBenchmark();
                RunDiskBenchmark();
                UpdateView();
            } else if (id == ID_BTN_EXP_TXT) {
                ExportReport(0);
            } else if (id == ID_BTN_EXP_JSON) {
                ExportReport(1);
            } else if (id == ID_BTN_EXP_HTML) {
                ExportReport(2);
            }
            break;
        }
        case WM_TIMER: {
            if (wParam == 1 && g_CurrentTab == 0) {
                UpdateView();
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            if (hTabCtrl) MoveWindow(hTabCtrl, 5, 5, nw - 10, 30, TRUE);
            if (hOutput) MoveWindow(hOutput, 5, 40, nw - 10, nh - 85, TRUE);
            if (hBtnCpu) MoveWindow(hBtnCpu, 10, nh - 38, 90, 25, TRUE);
            if (hBtnRam) MoveWindow(hBtnRam, 105, nh - 38, 90, 25, TRUE);
            if (hBtnDisk) MoveWindow(hBtnDisk, 200, nh - 38, 90, 25, TRUE);
            if (hBtnAll) MoveWindow(hBtnAll, 295, nh - 38, 110, 25, TRUE);

            if (hBtnExpTxt) MoveWindow(hBtnExpTxt, 10, nh - 38, 110, 25, TRUE);
            if (hBtnExpJson) MoveWindow(hBtnExpJson, 130, nh - 38, 110, 25, TRUE);
            if (hBtnExpHtml) MoveWindow(hBtnExpHtml, 250, nh - 38, 110, 25, TRUE);
            break;
        }
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            if (hFont) DeleteObject(hFont);
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

    HWND hwnd = CreateWindowEx(0, "KSysApp", "KSys Workstation Diagnostics", WS_OVERLAPPEDWINDOW,
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
