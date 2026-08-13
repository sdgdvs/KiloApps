#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#define W 1024
#define H 768

#define ID_TAB_CTRL        1001
#define ID_TXT_MAIN        1002
#define ID_BTN_CPU         1003
#define ID_BTN_RAM         1004
#define ID_BTN_DISK        1005
#define ID_BTN_ALL         1006
#define ID_BTN_EXP_TXT      1007
#define ID_BTN_EXP_JSON     1008
#define ID_BTN_EXP_HTML     1009
#define ID_BTN_HELP          1010
#define ID_BTN_SVC_REFRESH  1011
#define ID_BTN_SVC_FILTER   1012

HWND hTabCtrl = NULL;
HWND hOutput = NULL;
HWND hBtnCpu = NULL;
HWND hBtnRam = NULL;
HWND hBtnDisk = NULL;
HWND hBtnAll = NULL;
HWND hBtnExpTxt = NULL;
HWND hBtnExpJson = NULL;
HWND hBtnExpHtml = NULL;
HWND hBtnHelp = NULL;
HWND hBtnSvcRefresh = NULL;
HWND hBtnSvcFilter = NULL;

char g_LogBuffer[16384] = {0};
char g_CpuResult[128] = "Not Executed";
char g_RamResult[128] = "Not Executed";
char g_DiskResult[128] = "Not Executed";

int g_CurrentTab = 0;
int g_ServiceFilterMode = 0; // 0: All, 1: Running Only, 2: Stopped Only

void LogEvent(const char* level, const char* msg) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    char timeStr[64];
    wsprintfA(timeStr, "[%02d:%02d:%02d] [%s] %s\r\n", st.wHour, st.wMinute, st.wSecond, level, msg);
    
    lstrcatA(g_LogBuffer, timeStr);
}

ULONGLONG ftTo64(const FILETIME* ft) {
    return ((ULONGLONG)ft->dwHighDateTime << 32) | ft->dwLowDateTime;
}

int GetCpuUsagePercent() {
    static ULONGLONG prevIdle = 0, prevKernel = 0, prevUser = 0;
    FILETIME ftIdle, ftKernel, ftUser;
    if (!GetSystemTimes(&ftIdle, &ftKernel, &ftUser)) return 0;
    
    ULONGLONG idle = ftTo64(&ftIdle);
    ULONGLONG kernel = ftTo64(&ftKernel);
    ULONGLONG user = ftTo64(&ftUser);
    
    ULONGLONG diffIdle = idle - prevIdle;
    ULONGLONG diffKernel = kernel - prevKernel;
    ULONGLONG diffUser = user - prevUser;
    
    prevIdle = idle;
    prevKernel = kernel;
    prevUser = user;
    
    ULONGLONG totalSys = diffKernel + diffUser;
    if (totalSys == 0) return 0;
    
    ULONGLONG sysUsage = totalSys - diffIdle;
    int pct = (int)((sysUsage * 100) / totalSys);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    return pct;
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
    
    DWORD totalSvc = 0, runSvc = 0;
    SC_HANDLE hSCM = OpenSCManagerA(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (hSCM) {
        DWORD bytesNeeded = 0, servicesReturned = 0, resumeHandle = 0;
        EnumServicesStatusA(hSCM, SERVICE_WIN32 | SERVICE_DRIVER, SERVICE_STATE_ALL, NULL, 0, &bytesNeeded, &servicesReturned, &resumeHandle);
        if (bytesNeeded > 0) {
            BYTE* pBuf = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytesNeeded);
            if (pBuf) {
                resumeHandle = 0;
                if (EnumServicesStatusA(hSCM, SERVICE_WIN32 | SERVICE_DRIVER, SERVICE_STATE_ALL, (LPENUM_SERVICE_STATUSA)pBuf, bytesNeeded, &bytesNeeded, &servicesReturned, &resumeHandle)) {
                    totalSvc = servicesReturned;
                    LPENUM_SERVICE_STATUSA pServices = (LPENUM_SERVICE_STATUSA)pBuf;
                    for (DWORD i = 0; i < totalSvc; i++) {
                        if (pServices[i].ServiceStatus.dwCurrentState == SERVICE_RUNNING) runSvc++;
                    }
                }
                HeapFree(GetProcessHeap(), 0, pBuf);
            }
        }
        CloseServiceHandle(hSCM);
    }

    wsprintfA(buf,
        "=================================================================\r\n"
        "       KSYS NATIVE SYSTEM DIAGNOSTICS & HARDWARE REPORT          \r\n"
        "=================================================================\r\n"
        "-> Press 'H' for Help/Instructions \r\n\r\n"
        "System Uptime      : %u h %u m %u s\r\n"
        "Architecture       : %s\r\n"
        "Logical Processors : %u Cores\r\n"
        "Page Size          : %u bytes\r\n\r\n"
        "--- MEMORY SPECIFICATIONS ---\r\n"
        "Memory Load        : %u%%\r\n"
        "Total Physical RAM : %u MB\r\n"
        "Available Physical : %u MB\r\n"
        "Total Page File    : %u MB\r\n\r\n"
        "--- SYSTEM SERVICES SUMMARY ---\r\n"
        "Total Services     : %u Services & Drivers Monitored\r\n"
        "Running Services   : %u Active Services\r\n\r\n"
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
        totalSvc, runSvc,
        hasDisk ? (DWORD)(totalBytes.QuadPart >> 20) : 0,
        hasDisk ? (DWORD)(totalFree.QuadPart >> 20) : 0,
        sw, sh, bpp, hz,
        hasPower ? ((sps.ACLineStatus == 1) ? "Online (AC)" : "Offline (Battery)") : "Unknown",
        hasPower ? ((sps.BatteryLifePercent != 255) ? "Charged" : "N/A") : "N/A",
        g_CpuResult, g_RamResult, g_DiskResult
    );
}

void GetServicesAndTelemetryText(char* buf, int maxLen) {
    int cpuPct = GetCpuUsagePercent();
    
    MEMORYSTATUSEX mem = {0};
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);
    
    DWORD ticks = GetTickCount();
    DWORD hours = ticks / 3600000;
    DWORD mins = (ticks / 60000) % 60;
    DWORD secs = (ticks / 1000) % 60;
    
    wsprintfA(buf,
        "=================================================================\r\n"
        "       REAL-TIME SYSTEM TELEMETRY & SERVICES MANAGER            \r\n"
        "=================================================================\r\n"
        "-> Press 'H' for Help/Instructions \r\n\r\n"
        "--- REAL-TIME TELEMETRY ---\r\n"
        "Live CPU Usage      : %d%%\r\n"
        "Memory Load         : %u%%\r\n"
        "Avail Physical RAM  : %u MB / %u MB\r\n"
        "Page File Avail     : %u MB\r\n"
        "System Uptime       : %u h %u m %u s\r\n\r\n"
        "--- WINDOWS SYSTEM SERVICES & DRIVERS STATUS ---\r\n"
        "Filter Mode         : %s\r\n",
        cpuPct,
        mem.dwMemoryLoad,
        (DWORD)(mem.ullAvailPhys >> 20),
        (DWORD)(mem.ullTotalPhys >> 20),
        (DWORD)(mem.ullAvailPageFile >> 20),
        hours, mins, secs,
        (g_ServiceFilterMode == 1) ? "RUNNING ONLY" : (g_ServiceFilterMode == 2) ? "STOPPED ONLY" : "ALL SERVICES"
    );

    SC_HANDLE hSCM = OpenSCManagerA(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (!hSCM) {
        lstrcatA(buf, "Error: Unable to open Windows Service Control Manager.\r\n");
        return;
    }

    DWORD bytesNeeded = 0, servicesReturned = 0, resumeHandle = 0;
    EnumServicesStatusA(hSCM, SERVICE_WIN32 | SERVICE_DRIVER, SERVICE_STATE_ALL, NULL, 0, &bytesNeeded, &servicesReturned, &resumeHandle);

    if (bytesNeeded > 0) {
        BYTE* pBuf = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytesNeeded);
        if (pBuf) {
            resumeHandle = 0;
            if (EnumServicesStatusA(hSCM, SERVICE_WIN32 | SERVICE_DRIVER, SERVICE_STATE_ALL, (LPENUM_SERVICE_STATUSA)pBuf, bytesNeeded, &bytesNeeded, &servicesReturned, &resumeHandle)) {
                LPENUM_SERVICE_STATUSA pServices = (LPENUM_SERVICE_STATUSA)pBuf;
                DWORD totalCount = servicesReturned;
                DWORD runningCount = 0;
                DWORD stoppedCount = 0;
                DWORD driverCount = 0;

                for (DWORD i = 0; i < totalCount; i++) {
                    if (pServices[i].ServiceStatus.dwCurrentState == SERVICE_RUNNING) runningCount++;
                    else if (pServices[i].ServiceStatus.dwCurrentState == SERVICE_STOPPED) stoppedCount++;
                    if (pServices[i].ServiceStatus.dwServiceType & SERVICE_DRIVER) driverCount++;
                }

                char headerBuf[256];
                wsprintfA(headerBuf,
                    "Total Services      : %u | Running: %u | Stopped: %u | Drivers: %u\r\n\r\n"
                    "%-24s %-12s %-12s %-32s\r\n"
                    "--------------------------------------------------------------------------------\r\n",
                    totalCount, runningCount, stoppedCount, driverCount,
                    "SERVICE NAME", "TYPE", "STATUS", "DISPLAY NAME"
                );
                lstrcatA(buf, headerBuf);

                DWORD shown = 0;
                for (DWORD i = 0; i < totalCount && shown < 80; i++) {
                    DWORD state = pServices[i].ServiceStatus.dwCurrentState;
                    DWORD type = pServices[i].ServiceStatus.dwServiceType;

                    if (g_ServiceFilterMode == 1 && state != SERVICE_RUNNING) continue;
                    if (g_ServiceFilterMode == 2 && state != SERVICE_STOPPED) continue;

                    const char* stStr = (state == SERVICE_RUNNING) ? "RUNNING" :
                                        (state == SERVICE_STOPPED) ? "STOPPED" :
                                        (state == SERVICE_PAUSED) ? "PAUSED" : "PENDING";

                    const char* typeStr = (type & SERVICE_DRIVER) ? "DRIVER" : "WIN32";

                    char svcName[25];
                    lstrcpynA(svcName, pServices[i].lpServiceName ? pServices[i].lpServiceName : "N/A", sizeof(svcName));

                    char dispName[33];
                    lstrcpynA(dispName, pServices[i].lpDisplayName ? pServices[i].lpDisplayName : "N/A", sizeof(dispName));

                    char line[256];
                    wsprintfA(line, "%-24s %-12s %-12s %-32s\r\n", svcName, typeStr, stStr, dispName);
                    lstrcatA(buf, line);
                    shown++;
                }
                if (shown < totalCount && g_ServiceFilterMode == 0) {
                    char tailBuf[64];
                    wsprintfA(tailBuf, "... and %u more services.\r\n", totalCount - shown);
                    lstrcatA(buf, tailBuf);
                }
            }
            HeapFree(GetProcessHeap(), 0, pBuf);
        }
    }
    CloseServiceHandle(hSCM);
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
    BOOL isSvcTab   = (g_CurrentTab == 2);
    BOOL isExpTab   = (g_CurrentTab == 4);

    ShowWindow(hBtnCpu, isBenchTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnRam, isBenchTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnDisk, isBenchTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnAll, isBenchTab ? SW_SHOW : SW_HIDE);

    ShowWindow(hBtnSvcRefresh, isSvcTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnSvcFilter, isSvcTab ? SW_SHOW : SW_HIDE);

    ShowWindow(hBtnExpTxt, isExpTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnExpJson, isExpTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnExpHtml, isExpTab ? SW_SHOW : SW_HIDE);

    if (g_CurrentTab == 0) { // Hardware Inspector
        GetSystemAuditText(contentBuf, sizeof(contentBuf));
        SetWindowTextA(hOutput, contentBuf);
    } else if (g_CurrentTab == 1) { // Benchmarks
        wsprintfA(contentBuf,
            "--- DIAGNOSTIC BENCHMARK SUITE ---\r\n\r\n"
            "-> Press 'H' for Help/Instructions \r\n\r\n"
            "CPU Benchmark Test  : %s\r\n"
            "RAM Throughput Test : %s\r\n"
            "Disk I/O Speed Test : %s\r\n\r\n"
            "Click buttons below to execute diagnostic performance benchmarks.\r\n",
            g_CpuResult, g_RamResult, g_DiskResult
        );
        SetWindowTextA(hOutput, contentBuf);
    } else if (g_CurrentTab == 2) { // Services & Telemetry
        GetServicesAndTelemetryText(contentBuf, sizeof(contentBuf));
        SetWindowTextA(hOutput, contentBuf);
    } else if (g_CurrentTab == 3) { // Event History Logs
        SetWindowTextA(hOutput, g_LogBuffer);
    } else if (g_CurrentTab == 4) { // Report Export
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
            hFont = CreateFontA(-15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Consolas");
            if (!hFont) hFont = CreateFontA(-15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Courier New");
            
            InitCommonControls();

            hTabCtrl = CreateWindowEx(0, WC_TABCONTROL, "", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                5, 5, W - 25, 30, hwnd, (HMENU)ID_TAB_CTRL, NULL, NULL);

            TCITEM tie;
            tie.mask = TCIF_TEXT;
            tie.pszText = "Hardware Inspector";
            TabCtrl_InsertItem(hTabCtrl, 0, &tie);
            tie.pszText = "Diagnostic Benchmarks";
            TabCtrl_InsertItem(hTabCtrl, 1, &tie);
            tie.pszText = "Services & Telemetry";
            TabCtrl_InsertItem(hTabCtrl, 2, &tie);
            tie.pszText = "Event Logs";
            TabCtrl_InsertItem(hTabCtrl, 3, &tie);
            tie.pszText = "Report Export";
            TabCtrl_InsertItem(hTabCtrl, 4, &tie);

            hOutput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY, 
                5, 40, W - 25, H - 125, hwnd, (HMENU)ID_TXT_MAIN, NULL, NULL);

            // Benchmark Buttons
            hBtnCpu = CreateWindow("BUTTON", "Test CPU", WS_CHILD | BS_PUSHBUTTON, 10, H - 75, 100, 25, hwnd, (HMENU)ID_BTN_CPU, NULL, NULL);
            hBtnRam = CreateWindow("BUTTON", "Test RAM", WS_CHILD | BS_PUSHBUTTON, 120, H - 75, 100, 25, hwnd, (HMENU)ID_BTN_RAM, NULL, NULL);
            hBtnDisk = CreateWindow("BUTTON", "Test Disk", WS_CHILD | BS_PUSHBUTTON, 230, H - 75, 100, 25, hwnd, (HMENU)ID_BTN_DISK, NULL, NULL);
            hBtnAll = CreateWindow("BUTTON", "Run All Tests", WS_CHILD | BS_PUSHBUTTON, 340, H - 75, 110, 25, hwnd, (HMENU)ID_BTN_ALL, NULL, NULL);

            // Service Buttons
            hBtnSvcRefresh = CreateWindow("BUTTON", "Refresh Services", WS_CHILD | BS_PUSHBUTTON, 10, H - 75, 140, 25, hwnd, (HMENU)ID_BTN_SVC_REFRESH, NULL, NULL);
            hBtnSvcFilter  = CreateWindow("BUTTON", "Filter: All Services", WS_CHILD | BS_PUSHBUTTON, 160, H - 75, 160, 25, hwnd, (HMENU)ID_BTN_SVC_FILTER, NULL, NULL);

            // Export Buttons
            hBtnExpTxt = CreateWindow("BUTTON", "Export TXT", WS_CHILD | BS_PUSHBUTTON, 10, H - 75, 110, 25, hwnd, (HMENU)ID_BTN_EXP_TXT, NULL, NULL);
            hBtnExpJson = CreateWindow("BUTTON", "Export JSON", WS_CHILD | BS_PUSHBUTTON, 130, H - 75, 110, 25, hwnd, (HMENU)ID_BTN_EXP_JSON, NULL, NULL);
            hBtnExpHtml = CreateWindow("BUTTON", "Export HTML", WS_CHILD | BS_PUSHBUTTON, 250, H - 75, 110, 25, hwnd, (HMENU)ID_BTN_EXP_HTML, NULL, NULL);

            hBtnHelp = CreateWindow("BUTTON", "Help (H)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, W - 110, H - 75, 90, 25, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);

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
            } else if (id == ID_BTN_SVC_REFRESH) {
                LogEvent("INFO", "Refreshed Win32 Services & Telemetry status");
                UpdateView();
            } else if (id == ID_BTN_SVC_FILTER) {
                g_ServiceFilterMode = (g_ServiceFilterMode + 1) % 3;
                if (g_ServiceFilterMode == 0) SetWindowTextA(hBtnSvcFilter, "Filter: All Services");
                else if (g_ServiceFilterMode == 1) SetWindowTextA(hBtnSvcFilter, "Filter: Running Only");
                else if (g_ServiceFilterMode == 2) SetWindowTextA(hBtnSvcFilter, "Filter: Stopped Only");
                LogEvent("INFO", "Toggled Service Manager filter mode");
                UpdateView();
            } else if (id == ID_BTN_EXP_TXT) {
                ExportReport(0);
            } else if (id == ID_BTN_EXP_JSON) {
                ExportReport(1);
            } else if (id == ID_BTN_EXP_HTML) {
                ExportReport(2);
            } else if (id == ID_BTN_HELP) {
                MessageBoxA(hwnd, "KSys Help Instructions:\n\n1. Hardware Inspector: View live system details.\n2. Diagnostic Benchmarks: Run CPU, RAM, and Disk I/O tests.\n3. Services & Telemetry: Inspect real-time CPU load & Win32 Services.\n4. Event Logs: Check background activity.\n5. Report Export: Generate and download summaries.\n\nUse the tabs to navigate.", "KSys Help", MB_OK | MB_ICONINFORMATION);
            }
            break;
        }
        case WM_TIMER: {
            if (wParam == 1 && (g_CurrentTab == 0 || g_CurrentTab == 2)) {
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

            if (hBtnSvcRefresh) MoveWindow(hBtnSvcRefresh, 10, nh - 38, 140, 25, TRUE);
            if (hBtnSvcFilter) MoveWindow(hBtnSvcFilter, 160, nh - 38, 160, 25, TRUE);

            if (hBtnExpTxt) MoveWindow(hBtnExpTxt, 10, nh - 38, 110, 25, TRUE);
            if (hBtnExpJson) MoveWindow(hBtnExpJson, 130, nh - 38, 110, 25, TRUE);
            if (hBtnExpHtml) MoveWindow(hBtnExpHtml, 250, nh - 38, 110, 25, TRUE);
            if (hBtnHelp) MoveWindow(hBtnHelp, nw - 110, nh - 38, 100, 25, TRUE);
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
    HMODULE hUser32 = LoadLibraryA("user32.dll");
    if (hUser32) {
        FARPROC setDpi = GetProcAddress(hUser32, "SetProcessDPIAware");
        if (setDpi) ((BOOL(WINAPI*)())setDpi)();
        FreeLibrary(hUser32);
    }
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KSysApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    RECT rect = { 0, 0, W, H };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, FALSE);
    int realW = rect.right - rect.left;
    int realH = rect.bottom - rect.top;

    HWND hwnd = CreateWindowEx(0, "KSysApp", "KSys Workstation Diagnostics - Press 'H' for Help", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, realW, realH, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN && msg.wParam == 'H') {
            MessageBoxA(hwnd, "KSys Help Instructions:\n\n1. Hardware Inspector: View live system details.\n2. Diagnostic Benchmarks: Run CPU, RAM, and Disk I/O tests.\n3. Services & Telemetry: Inspect real-time CPU load & Win32 Services.\n4. Event Logs: Check background activity.\n5. Report Export: Generate and download summaries.\n\nUse the tabs to navigate.", "KSys Help", MB_OK | MB_ICONINFORMATION);
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
