#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#pragma function(memset, memcpy)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* p = (char*)dest;
    while (count--) *p++ = (char)c;
    return dest;
}

void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}

void UpdateView(void);

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
#define ID_BTN_INSP_REFRESH 1013
#define ID_BTN_INSP_BENCH   1014
#define ID_BTN_LOG_CLEAR    1015
#define ID_BTN_LOG_EXPORT   1016
#define ID_STATUS_BAR       1017
#define ID_BTN_EXP_CSV      1018
#define ID_BTN_EXP_MD       1019

HWND hTabCtrl = NULL;
HWND hOutput = NULL;
HWND hBtnCpu = NULL;
HWND hBtnRam = NULL;
HWND hBtnDisk = NULL;
HWND hBtnAll = NULL;
HWND hBtnExpTxt = NULL;
HWND hBtnExpJson = NULL;
HWND hBtnExpHtml = NULL;
HWND hBtnExpCsv = NULL;
HWND hBtnExpMd = NULL;
HWND hBtnHelp = NULL;
HWND hBtnSvcRefresh = NULL;
HWND hBtnSvcFilter = NULL;
HWND hBtnInspRefresh = NULL;
HWND hBtnInspBench = NULL;
HWND hBtnLogClear = NULL;
HWND hBtnLogExport = NULL;
HWND hStatusBar = NULL;

char g_LogBuffer[16384] = {0};
char g_CpuResult[128] = "Not Executed";
char g_RamResult[128] = "Not Executed";
char g_DiskResult[128] = "Not Executed";
char g_ToastMsg[256] = "Welcome to KSys! Tabs: [1-5] | Run: [R] | Help: [F1] / [H]";
DWORD g_ToastExpire = 0;

int g_CurrentTab = 0;
int g_ServiceFilterMode = 0; // 0: All, 1: Running Only, 2: Stopped Only

void LogEvent(const char* level, const char* msg) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    char timeStr[128];
    wsprintfA(timeStr, "[%02d:%02d:%02d] [%s] %s\r\n", st.wHour, st.wMinute, st.wSecond, level, msg);
    
    int curLen = lstrlenA(g_LogBuffer);
    int addLen = lstrlenA(timeStr);
    if (curLen + addLen >= (int)sizeof(g_LogBuffer) - 1) {
        int pruneLen = (int)sizeof(g_LogBuffer) / 2;
        char* pNextLine = g_LogBuffer + pruneLen;
        while (*pNextLine && *pNextLine != '\n') pNextLine++;
        if (*pNextLine == '\n') pNextLine++;
        int remainLen = lstrlenA(pNextLine);
        for (int i = 0; i <= remainLen; i++) {
            g_LogBuffer[i] = pNextLine[i];
        }
    }
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

    char computerName[64] = "Unknown";
    DWORD cnSize = sizeof(computerName);
    GetComputerNameA(computerName, &cnSize);

    char userName[64] = "Unknown";
    DWORD unSize = sizeof(userName);
    GetUserNameA(userName, &unSize);

    char winDir[MAX_PATH] = "Unknown";
    GetWindowsDirectoryA(winDir, sizeof(winDir));

    char sysDir[MAX_PATH] = "Unknown";
    GetSystemDirectoryA(sysDir, sizeof(sysDir));

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

    // Health Index calculation
    int healthScore = 100;
    if (mem.dwMemoryLoad > 85) healthScore -= 25;
    else if (mem.dwMemoryLoad > 70) healthScore -= 10;

    ULARGE_INTEGER cFreeBytesCaller, cTotalBytes, cTotalFree;
    BOOL hasDiskC = GetDiskFreeSpaceExA("C:\\", &cFreeBytesCaller, &cTotalBytes, &cTotalFree);
    if (hasDiskC) {
        DWORD freeMB = (DWORD)(cTotalFree.QuadPart >> 20);
        if (freeMB < 5120) healthScore -= 20;
        else if (freeMB < 15360) healthScore -= 10;
    }
    if (healthScore < 0) healthScore = 0;

    const char* healthRating = (healthScore >= 90) ? "EXCELLENT - Optimal Operation" :
                               (healthScore >= 75) ? "GOOD - Standard Workload Parameters" :
                               (healthScore >= 50) ? "FAIR - High Memory or Storage Load" : "CRITICAL - Resource Exhaustion Risk";

    char chunk[512];
    buf[0] = '\0';
    lstrcatA(buf, "=================================================================\r\n"
                  "       KSYS NATIVE SYSTEM DIAGNOSTICS & HARDWARE REPORT          \r\n"
                  "=================================================================\r\n"
                  "-> Press 'H' or F1 for Help/Instructions \r\n\r\n");

    wsprintfA(chunk, "--- SYSTEM HEALTH & HOST IDENTITY ---\r\n"
                     "Overall Health Index : %u%% [%s]\r\n"
                     "Host Computer Name   : %s\r\n"
                     "Active User Profile  : %s\r\n"
                     "System Uptime        : %u h %u m %u s\r\n\r\n",
              healthScore, healthRating, computerName, userName, hours, mins, secs);
    lstrcatA(buf, chunk);

    wsprintfA(chunk, "--- PROCESSOR & TOPOLOGY ---\r\n"
                     "Architecture         : %s\r\n"
                     "Logical Processors   : %u Cores\r\n"
                     "Page Size            : %u bytes\r\n"
                     "Alloc Granularity    : %u bytes\r\n"
                     "Min App Address      : 0x%p\r\n"
                     "Max App Address      : 0x%p\r\n\r\n",
              (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) ? "x64 (AMD64)" :
              (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) ? "x86 (Intel)" : "ARM/Other",
              si.dwNumberOfProcessors, si.dwPageSize, si.dwAllocationGranularity,
              si.lpMinimumApplicationAddress, si.lpMaximumApplicationAddress);
    lstrcatA(buf, chunk);

    wsprintfA(chunk, "--- MEMORY SPECIFICATIONS ---\r\n"
                     "Memory Load          : %u%%\r\n"
                     "Total Physical RAM   : %u MB\r\n"
                     "Available Physical   : %u MB\r\n"
                     "Total Page File      : %u MB\r\n"
                     "Available Page File  : %u MB\r\n\r\n",
              mem.dwMemoryLoad, (DWORD)(mem.ullTotalPhys >> 20), (DWORD)(mem.ullAvailPhys >> 20),
              (DWORD)(mem.ullTotalPageFile >> 20), (DWORD)(mem.ullAvailPageFile >> 20));
    lstrcatA(buf, chunk);

    wsprintfA(chunk, "--- SYSTEM SERVICES SUMMARY ---\r\n"
                     "Total Services       : %u Services & Drivers Monitored\r\n"
                     "Running Services     : %u Active Services\r\n\r\n",
              totalSvc, runSvc);
    lstrcatA(buf, chunk);

    lstrcatA(buf, "--- MOUNTED DRIVES & STORAGE VOLUMES ---\r\n");
    char driveStrings[512] = {0};
    DWORD drvLen = GetLogicalDriveStringsA(sizeof(driveStrings) - 1, driveStrings);
    if (drvLen > 0 && drvLen < sizeof(driveStrings)) {
        char* pDrive = driveStrings;
        while (*pDrive && lstrlenA(buf) < maxLen - 1024) {
            UINT driveType = GetDriveTypeA(pDrive);
            const char* typeStr = "Unknown";
            if (driveType == DRIVE_FIXED) typeStr = "Fixed (HDD/SSD)";
            else if (driveType == DRIVE_REMOVABLE) typeStr = "Removable (USB)";
            else if (driveType == DRIVE_CDROM) typeStr = "CD-ROM/Optical";
            else if (driveType == DRIVE_REMOTE) typeStr = "Network Share";
            else if (driveType == DRIVE_RAMDISK) typeStr = "RAM Disk";

            char volName[64] = {0};
            char fsName[32] = {0};
            GetVolumeInformationA(pDrive, volName, sizeof(volName), NULL, NULL, NULL, fsName, sizeof(fsName));

            ULARGE_INTEGER dCaller, dTotal, dFree;
            if (GetDiskFreeSpaceExA(pDrive, &dCaller, &dTotal, &dFree)) {
                wsprintfA(chunk, "  %-4s [%-12s] %-16s FS: %-6s Total: %6u MB | Free: %6u MB\r\n",
                          pDrive, volName[0] ? volName : "Local Disk", typeStr, fsName[0] ? fsName : "NTFS",
                          (DWORD)(dTotal.QuadPart >> 20), (DWORD)(dFree.QuadPart >> 20));
            } else {
                wsprintfA(chunk, "  %-4s %-16s (Media Unmounted/Not Ready)\r\n", pDrive, typeStr);
            }
            lstrcatA(buf, chunk);
            pDrive += lstrlenA(pDrive) + 1;
        }
    }
    lstrcatA(buf, "\r\n");

    wsprintfA(chunk, "--- OS DIRECTORIES ---\r\n"
                     "Windows Directory    : %s\r\n"
                     "System Directory     : %s\r\n\r\n",
              winDir, sysDir);
    lstrcatA(buf, chunk);

    wsprintfA(chunk, "--- DISPLAY & GRAPHICS ---\r\n"
                     "Resolution           : %dx%d (%d-bit, %d Hz)\r\n\r\n",
              sw, sh, bpp, hz);
    lstrcatA(buf, chunk);

    wsprintfA(chunk, "--- POWER & BATTERY ---\r\n"
                     "AC Power Line        : %s\r\n"
                     "Battery Level        : %s\r\n\r\n",
              hasPower ? ((sps.ACLineStatus == 1) ? "Online (AC)" : "Offline (Battery)") : "Unknown",
              hasPower ? ((sps.BatteryLifePercent != 255) ? "Charged" : "N/A") : "N/A");
    lstrcatA(buf, chunk);

    wsprintfA(chunk, "--- DIAGNOSTIC BENCHMARK RESULTS ---\r\n"
                     "CPU Multi-thread     : %s\r\n"
                     "RAM Throughput       : %s\r\n"
                     "Disk I/O Throughput  : %s\r\n"
                     "=================================================================\r\n",
              g_CpuResult, g_RamResult, g_DiskResult);
    lstrcatA(buf, chunk);
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
        "-> Press 'H' or F1 for Help/Instructions \r\n\r\n"
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
        (g_ServiceFilterMode == 1) ? "RUNNING ONLY" :
        (g_ServiceFilterMode == 2) ? "STOPPED ONLY" :
        (g_ServiceFilterMode == 3) ? "WIN32 SERVICES ONLY" :
        (g_ServiceFilterMode == 4) ? "DRIVERS ONLY" : "ALL SERVICES"
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
                    if (g_ServiceFilterMode == 3 && (type & SERVICE_DRIVER)) continue;
                    if (g_ServiceFilterMode == 4 && !(type & SERVICE_DRIVER)) continue;

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

BOOL SaveReportFile(const char* filename, const char* content) {
    HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return FALSE;
    DWORD written = 0;
    BOOL ok = WriteFile(hFile, content, lstrlenA(content), &written, NULL);
    CloseHandle(hFile);
    if (ok) {
        char logMsg[256];
        wsprintfA(logMsg, "Exported report to file: %s", filename);
        LogEvent("INFO", logMsg);
    }
    return ok;
}

void ShowNativeToast(HWND hwnd, const char* msg) {
    lstrcpynA(g_ToastMsg, msg, sizeof(g_ToastMsg));
    g_ToastExpire = GetTickCount() + 4000;
    if (hStatusBar) {
        SetWindowTextA(hStatusBar, g_ToastMsg);
    }
}

void ClearLogs(HWND hwnd) {
    g_LogBuffer[0] = '\0';
    LogEvent("INFO", "Event history logs cleared by user");
    ShowNativeToast(hwnd, "Event history logs cleared.");
    UpdateView();
}

void ExportLogs(HWND hwnd) {
    if (SaveReportFile("ksys_event_log.txt", g_LogBuffer[0] ? g_LogBuffer : "[No event logs recorded]\r\n")) {
        ShowNativeToast(hwnd, "SUCCESS: Exported event log to ksys_event_log.txt");
    } else {
        ShowNativeToast(hwnd, "ERROR: Failed to write ksys_event_log.txt");
    }
}

void GenerateCsvReport(char* buf, int maxLen) {
    buf[0] = '\0';
    lstrcatA(buf, "Section,Metric,Value\r\n");
    
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    MEMORYSTATUSEX mem = {0};
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);

    char line[256];
    wsprintfA(line, "System,Architecture,%s\r\n", (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) ? "x64" : "x86");
    lstrcatA(buf, line);
    wsprintfA(line, "System,LogicalCores,%u\r\n", si.dwNumberOfProcessors);
    lstrcatA(buf, line);
    wsprintfA(line, "Memory,MemoryLoadPercent,%u%%\r\n", mem.dwMemoryLoad);
    lstrcatA(buf, line);
    wsprintfA(line, "Memory,TotalPhysicalRAM_MB,%u\r\n", (DWORD)(mem.ullTotalPhys >> 20));
    lstrcatA(buf, line);
    wsprintfA(line, "Memory,AvailPhysicalRAM_MB,%u\r\n", (DWORD)(mem.ullAvailPhys >> 20));
    lstrcatA(buf, line);
    wsprintfA(line, "Benchmarks,CPUStress,\"%s\"\r\n", g_CpuResult);
    lstrcatA(buf, line);
    wsprintfA(line, "Benchmarks,RAMSpeed,\"%s\"\r\n", g_RamResult);
    lstrcatA(buf, line);
    wsprintfA(line, "Benchmarks,DiskIO,\"%s\"\r\n", g_DiskResult);
    lstrcatA(buf, line);

    lstrcatA(buf, "\r\nServiceName,DisplayName,Type,Status\r\n");
    SC_HANDLE hSCM = OpenSCManagerA(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (hSCM) {
        DWORD bytesNeeded = 0, servicesReturned = 0, resumeHandle = 0;
        EnumServicesStatusA(hSCM, SERVICE_WIN32 | SERVICE_DRIVER, SERVICE_STATE_ALL, NULL, 0, &bytesNeeded, &servicesReturned, &resumeHandle);
        if (bytesNeeded > 0) {
            BYTE* pBuf = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytesNeeded);
            if (pBuf) {
                resumeHandle = 0;
                if (EnumServicesStatusA(hSCM, SERVICE_WIN32 | SERVICE_DRIVER, SERVICE_STATE_ALL, (LPENUM_SERVICE_STATUSA)pBuf, bytesNeeded, &bytesNeeded, &servicesReturned, &resumeHandle)) {
                    LPENUM_SERVICE_STATUSA pServices = (LPENUM_SERVICE_STATUSA)pBuf;
                    for (DWORD i = 0; i < servicesReturned && lstrlenA(buf) < maxLen - 256; i++) {
                        const char* st = (pServices[i].ServiceStatus.dwCurrentState == SERVICE_RUNNING) ? "RUNNING" : "STOPPED";
                        const char* tp = (pServices[i].ServiceStatus.dwServiceType & SERVICE_DRIVER) ? "DRIVER" : "WIN32";
                        wsprintfA(line, "\"%s\",\"%s\",%s,%s\r\n",
                            pServices[i].lpServiceName ? pServices[i].lpServiceName : "",
                            pServices[i].lpDisplayName ? pServices[i].lpDisplayName : "",
                            tp, st);
                        lstrcatA(buf, line);
                    }
                }
                HeapFree(GetProcessHeap(), 0, pBuf);
            }
        }
        CloseServiceHandle(hSCM);
    }
}

void GenerateMarkdownReport(char* buf, int maxLen) {
    static char auditText[16384];
    GetSystemAuditText(auditText, sizeof(auditText));

    buf[0] = '\0';
    lstrcatA(buf, "# KSys Native Workstation Diagnostics Report\r\n\r\n");
    lstrcatA(buf, "## Benchmark Telemetry\r\n\r\n");
    lstrcatA(buf, "| Diagnostic Benchmark | Result / Score |\r\n|---|---|\r\n");
    char line[256];
    wsprintfA(line, "| **CPU Multi-thread Stress** | `%s` |\r\n", g_CpuResult);
    lstrcatA(buf, line);
    wsprintfA(line, "| **RAM Throughput** | `%s` |\r\n", g_RamResult);
    lstrcatA(buf, line);
    wsprintfA(line, "| **Disk I/O Throughput** | `%s` |\r\n\r\n", g_DiskResult);
    lstrcatA(buf, line);

    lstrcatA(buf, "## System Hardware & Diagnostics Audit\r\n\r\n```text\r\n");
    lstrcatA(buf, auditText);
    lstrcatA(buf, "```\r\n");
}

void ExportReport(HWND hwndOwner, int type) {
    static char reportBuf[16384];
    GetSystemAuditText(reportBuf, sizeof(reportBuf));

    if (type == 0) { // TXT
        if (SaveReportFile("ksys_report.txt", reportBuf)) {
            ShowNativeToast(hwndOwner, "SUCCESS: Exported report to ksys_report.txt");
        } else {
            ShowNativeToast(hwndOwner, "ERROR: Failed to write ksys_report.txt");
        }
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
        if (SaveReportFile("ksys_report.json", jsonBuf)) {
            ShowNativeToast(hwndOwner, "SUCCESS: Exported report to ksys_report.json");
        } else {
            ShowNativeToast(hwndOwner, "ERROR: Failed to write ksys_report.json");
        }
    } else if (type == 2) { // HTML
        char htmlBuf[18432];
        wsprintfA(htmlBuf,
            "<!DOCTYPE html><html><head><title>KSys Report</title>"
            "<style>body{background:#0f172a;color:#38bdf8;font-family:monospace;padding:20px;}"
            "pre{background:#1e293b;padding:15px;border-radius:8px;color:#f8fafc;}</style></head>"
            "<body><h1>KSys Diagnostic Report</h1><pre>%s</pre></body></html>",
            reportBuf
        );
        if (SaveReportFile("ksys_report.html", htmlBuf)) {
            ShowNativeToast(hwndOwner, "SUCCESS: Exported report to ksys_report.html");
        } else {
            ShowNativeToast(hwndOwner, "ERROR: Failed to write ksys_report.html");
        }
    } else if (type == 3) { // CSV
        static char csvBuf[32768];
        GenerateCsvReport(csvBuf, sizeof(csvBuf));
        if (SaveReportFile("ksys_report.csv", csvBuf)) {
            ShowNativeToast(hwndOwner, "SUCCESS: Exported report to ksys_report.csv");
        } else {
            ShowNativeToast(hwndOwner, "ERROR: Failed to write ksys_report.csv");
        }
    } else if (type == 4) { // Markdown
        static char mdBuf[32768];
        GenerateMarkdownReport(mdBuf, sizeof(mdBuf));
        if (SaveReportFile("ksys_report.md", mdBuf)) {
            ShowNativeToast(hwndOwner, "SUCCESS: Exported report to ksys_report.md");
        } else {
            ShowNativeToast(hwndOwner, "ERROR: Failed to write ksys_report.md");
        }
    }
}

void UpdateView() {
    static char contentBuf[32768];

    // Show/hide buttons based on tab
    BOOL isInspTab  = (g_CurrentTab == 0);
    BOOL isBenchTab = (g_CurrentTab == 1);
    BOOL isSvcTab   = (g_CurrentTab == 2);
    BOOL isLogTab   = (g_CurrentTab == 3);
    BOOL isExpTab   = (g_CurrentTab == 4);

    ShowWindow(hBtnInspRefresh, isInspTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnInspBench,   isInspTab ? SW_SHOW : SW_HIDE);

    ShowWindow(hBtnCpu,  isBenchTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnRam,  isBenchTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnDisk, isBenchTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnAll,  isBenchTab ? SW_SHOW : SW_HIDE);

    ShowWindow(hBtnSvcRefresh, isSvcTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnSvcFilter,  isSvcTab ? SW_SHOW : SW_HIDE);

    ShowWindow(hBtnLogClear,  isLogTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnLogExport, isLogTab ? SW_SHOW : SW_HIDE);

    ShowWindow(hBtnExpTxt,  isExpTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnExpJson, isExpTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnExpHtml, isExpTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnExpCsv,  isExpTab ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnExpMd,   isExpTab ? SW_SHOW : SW_HIDE);

    if (g_CurrentTab == 0) { // Hardware Inspector
        GetSystemAuditText(contentBuf, sizeof(contentBuf));
        SetWindowTextA(hOutput, contentBuf);
    } else if (g_CurrentTab == 1) { // Benchmarks
        wsprintfA(contentBuf,
            "--- DIAGNOSTIC BENCHMARK SUITE ---\r\n\r\n"
            "-> Press 'H' or F1 for Help/Instructions \r\n\r\n"
            "CPU Benchmark Test  : %s\r\n"
            "RAM Throughput Test : %s\r\n"
            "Disk I/O Speed Test : %s\r\n\r\n"
            "Click buttons below or press [C], [M], [D], or [R] to execute diagnostic performance benchmarks.\r\n",
            g_CpuResult, g_RamResult, g_DiskResult
        );
        SetWindowTextA(hOutput, contentBuf);
    } else if (g_CurrentTab == 2) { // Services & Telemetry
        GetServicesAndTelemetryText(contentBuf, sizeof(contentBuf));
        SetWindowTextA(hOutput, contentBuf);
    } else if (g_CurrentTab == 3) { // Event History Logs
        SetWindowTextA(hOutput, g_LogBuffer[0] ? g_LogBuffer : "[No event logs recorded yet]\r\n");
    } else if (g_CurrentTab == 4) { // Report Export
        GetSystemAuditText(contentBuf, sizeof(contentBuf));
        SetWindowTextA(hOutput, contentBuf);
    }
}

BOOL CALLBACK SetFontProc(HWND child, LPARAM hFont) {
    SendMessage(child, WM_SETFONT, hFont, TRUE);
    return TRUE;
}

void ShowHelpDialog(HWND hwnd) {
    MessageBoxA(hwnd,
        "==========================================================\n"
        "       KSYS WORKSTATION DIAGNOSTICS HELP GUIDE            \n"
        "==========================================================\n\n"
        "KEYBOARD SHORTCUTS:\n"
        "  [1]           - Component Hardware Inspector Tab\n"
        "  [2]           - Diagnostic Stress Benchmarks Tab\n"
        "  [3]           - Services & Telemetry Manager Tab\n"
        "  [4]           - Event History Logs Tab\n"
        "  [5]           - Detailed System Report Export Tab\n"
        "  [Left/Right]  - Switch between Navigation Tabs\n"
        "  [Tab]         - Cycle Focus through Controls / Buttons\n"
        "  [F1] or [H]   - Display this Help Guide\n"
        "  [Esc]         - Return to Hardware Inspector [1]\n"
        "  [R] or [F5]   - Run All Benchmarks / Refresh Active View\n"
        "  [C]           - Run CPU Benchmark (Benchmarks Tab)\n"
        "  [M]           - Run RAM Benchmark (Benchmarks Tab)\n"
        "  [D]           - Run Disk Benchmark (Benchmarks Tab)\n"
        "  [S]           - Cycle Service Filter (All/Running/Stopped/Win32/Drivers)\n"
        "  [L]           - Clear Event History Logs (Logs Tab)\n"
        "  [E]           - Export TXT Report or Event Log\n"
        "  [J]           - Export JSON Report (Export Tab)\n"
        "  [T]           - Export HTML Report (Export Tab)\n"
        "  [V]           - Export CSV Report (Export Tab)\n"
        "  [K]           - Export Markdown Report (Export Tab)\n\n"
        "FEATURES & EXPANSIONS:\n"
        "  - Multi-Drive : Live volume inspection across all logical disks.\n"
        "  - Topology    : Physical RAM, pagefiles, CPU cores & app bounds.\n"
        "  - Diagnostics : Automated System Health Index & heuristic rating.\n"
        "  - Benchmarks  : Multi-threaded CPU matrix, RAM throughput, Disk I/O.\n"
        "  - Services    : Live CPU usage, Win32 & Kernel Driver filtering.\n"
        "  - Export Hub  : Multi-format exports: TXT, JSON, HTML, CSV, and Markdown.",
        "KSys Diagnostics Help",
        MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HFONT hFont = NULL;
    switch (msg) {
        case WM_CREATE: {
            hFont = CreateFontA(-15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Consolas");
            if (!hFont) hFont = CreateFontA(-15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Courier New");
            
            InitCommonControls();

            hTabCtrl = CreateWindowEx(0, WC_TABCONTROL, "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS,
                5, 5, W - 25, 30, hwnd, (HMENU)ID_TAB_CTRL, NULL, NULL);

            TCITEM tie;
            tie.mask = TCIF_TEXT;
            tie.pszText = "[1] Hardware Inspector";
            TabCtrl_InsertItem(hTabCtrl, 0, &tie);
            tie.pszText = "[2] Diagnostic Benchmarks";
            TabCtrl_InsertItem(hTabCtrl, 1, &tie);
            tie.pszText = "[3] Services & Telemetry";
            TabCtrl_InsertItem(hTabCtrl, 2, &tie);
            tie.pszText = "[4] Event Logs";
            TabCtrl_InsertItem(hTabCtrl, 3, &tie);
            tie.pszText = "[5] Report Export";
            TabCtrl_InsertItem(hTabCtrl, 4, &tie);

            hOutput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | ES_MULTILINE | ES_READONLY, 
                5, 40, W - 25, H - 145, hwnd, (HMENU)ID_TXT_MAIN, NULL, NULL);

            // Inspector Buttons
            hBtnInspRefresh = CreateWindow("BUTTON", "Refresh [R]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 10, H - 75, 110, 25, hwnd, (HMENU)ID_BTN_INSP_REFRESH, NULL, NULL);
            hBtnInspBench   = CreateWindow("BUTTON", "Run Benchmarks [2]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 130, H - 75, 160, 25, hwnd, (HMENU)ID_BTN_INSP_BENCH, NULL, NULL);

            // Benchmark Buttons
            hBtnCpu  = CreateWindow("BUTTON", "Test CPU [C]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 10, H - 75, 105, 25, hwnd, (HMENU)ID_BTN_CPU, NULL, NULL);
            hBtnRam  = CreateWindow("BUTTON", "Test RAM [M]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 125, H - 75, 105, 25, hwnd, (HMENU)ID_BTN_RAM, NULL, NULL);
            hBtnDisk = CreateWindow("BUTTON", "Test Disk [D]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 240, H - 75, 105, 25, hwnd, (HMENU)ID_BTN_DISK, NULL, NULL);
            hBtnAll  = CreateWindow("BUTTON", "Run All [R]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 355, H - 75, 110, 25, hwnd, (HMENU)ID_BTN_ALL, NULL, NULL);

            // Service Buttons
            hBtnSvcRefresh = CreateWindow("BUTTON", "Refresh [R]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 10, H - 75, 120, 25, hwnd, (HMENU)ID_BTN_SVC_REFRESH, NULL, NULL);
            hBtnSvcFilter  = CreateWindow("BUTTON", "Filter: All Services [S]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 140, H - 75, 180, 25, hwnd, (HMENU)ID_BTN_SVC_FILTER, NULL, NULL);

            // Log Buttons
            hBtnLogClear  = CreateWindow("BUTTON", "Clear Logs [L]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 10, H - 75, 120, 25, hwnd, (HMENU)ID_BTN_LOG_CLEAR, NULL, NULL);
            hBtnLogExport = CreateWindow("BUTTON", "Export Log [E]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 140, H - 75, 120, 25, hwnd, (HMENU)ID_BTN_LOG_EXPORT, NULL, NULL);

            // Export Buttons
            hBtnExpTxt  = CreateWindow("BUTTON", "Export TXT [E]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 10, H - 75, 105, 25, hwnd, (HMENU)ID_BTN_EXP_TXT, NULL, NULL);
            hBtnExpJson = CreateWindow("BUTTON", "Export JSON [J]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 120, H - 75, 110, 25, hwnd, (HMENU)ID_BTN_EXP_JSON, NULL, NULL);
            hBtnExpHtml = CreateWindow("BUTTON", "Export HTML [T]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 235, H - 75, 110, 25, hwnd, (HMENU)ID_BTN_EXP_HTML, NULL, NULL);
            hBtnExpCsv  = CreateWindow("BUTTON", "Export CSV [V]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 350, H - 75, 105, 25, hwnd, (HMENU)ID_BTN_EXP_CSV, NULL, NULL);
            hBtnExpMd   = CreateWindow("BUTTON", "Export MD [K]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 460, H - 75, 105, 25, hwnd, (HMENU)ID_BTN_EXP_MD, NULL, NULL);

            hBtnHelp = CreateWindow("BUTTON", "Help [F1]", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, W - 115, H - 75, 95, 25, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);

            hStatusBar = CreateWindowEx(0, "STATIC", g_ToastMsg, WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, 10, H - 45, W - 25, 20, hwnd, (HMENU)ID_STATUS_BAR, NULL, NULL);

            EnumChildWindows(hwnd, SetFontProc, (LPARAM)hFont);

            LogEvent("INFO", "KSys Workstation Diagnostics initialized");
            ShowNativeToast(hwnd, "Welcome to KSys! Tabs: [1-5] | Run: [R] | Help: [F1] / [H]");
            UpdateView();
            SetTimer(hwnd, 1, 1000, NULL);
            break;
        }
        case WM_ERASEBKGND:
            return 1;
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
            if (id == ID_BTN_INSP_REFRESH) {
                ShowNativeToast(hwnd, "Hardware specifications refreshed.");
                UpdateView();
            } else if (id == ID_BTN_INSP_BENCH) {
                g_CurrentTab = 1;
                if (hTabCtrl) TabCtrl_SetCurSel(hTabCtrl, 1);
                ShowNativeToast(hwnd, "Switched to Diagnostic Benchmarks [2]");
                UpdateView();
            } else if (id == ID_BTN_CPU) {
                RunCpuBenchmark();
                ShowNativeToast(hwnd, "CPU Benchmark finished.");
                UpdateView();
            } else if (id == ID_BTN_RAM) {
                RunRamBenchmark();
                ShowNativeToast(hwnd, "RAM Benchmark finished.");
                UpdateView();
            } else if (id == ID_BTN_DISK) {
                RunDiskBenchmark();
                ShowNativeToast(hwnd, "Disk Benchmark finished.");
                UpdateView();
            } else if (id == ID_BTN_ALL) {
                RunCpuBenchmark();
                RunRamBenchmark();
                RunDiskBenchmark();
                ShowNativeToast(hwnd, "All diagnostic benchmarks complete.");
                UpdateView();
            } else if (id == ID_BTN_SVC_REFRESH) {
                LogEvent("INFO", "Refreshed Win32 Services & Telemetry status");
                ShowNativeToast(hwnd, "Refreshed Services & Telemetry.");
                UpdateView();
            } else if (id == ID_BTN_SVC_FILTER) {
                g_ServiceFilterMode = (g_ServiceFilterMode + 1) % 5;
                if (g_ServiceFilterMode == 0) {
                    SetWindowTextA(hBtnSvcFilter, "Filter: All Services [S]");
                    ShowNativeToast(hwnd, "Filter: Showing All Services");
                } else if (g_ServiceFilterMode == 1) {
                    SetWindowTextA(hBtnSvcFilter, "Filter: Running Only [S]");
                    ShowNativeToast(hwnd, "Filter: Showing Running Services Only");
                } else if (g_ServiceFilterMode == 2) {
                    SetWindowTextA(hBtnSvcFilter, "Filter: Stopped Only [S]");
                    ShowNativeToast(hwnd, "Filter: Showing Stopped Services Only");
                } else if (g_ServiceFilterMode == 3) {
                    SetWindowTextA(hBtnSvcFilter, "Filter: Win32 Only [S]");
                    ShowNativeToast(hwnd, "Filter: Showing Win32 Services Only");
                } else if (g_ServiceFilterMode == 4) {
                    SetWindowTextA(hBtnSvcFilter, "Filter: Drivers Only [S]");
                    ShowNativeToast(hwnd, "Filter: Showing Kernel Drivers Only");
                }
                LogEvent("INFO", "Toggled Service Manager filter mode");
                UpdateView();
            } else if (id == ID_BTN_LOG_CLEAR) {
                ClearLogs(hwnd);
            } else if (id == ID_BTN_LOG_EXPORT) {
                ExportLogs(hwnd);
            } else if (id == ID_BTN_EXP_TXT) {
                ExportReport(hwnd, 0);
            } else if (id == ID_BTN_EXP_JSON) {
                ExportReport(hwnd, 1);
            } else if (id == ID_BTN_EXP_HTML) {
                ExportReport(hwnd, 2);
            } else if (id == ID_BTN_EXP_CSV) {
                ExportReport(hwnd, 3);
            } else if (id == ID_BTN_EXP_MD) {
                ExportReport(hwnd, 4);
            } else if (id == ID_BTN_HELP) {
                ShowHelpDialog(hwnd);
            }
            break;
        }
        case WM_TIMER: {
            if (wParam == 1) {
                if (g_CurrentTab == 0 || g_CurrentTab == 2) {
                    UpdateView();
                }
                if (g_ToastExpire != 0 && GetTickCount() > g_ToastExpire) {
                    g_ToastExpire = 0;
                    lstrcpynA(g_ToastMsg, "KSys Workstation Ready | Keys: [1-5] Tabs, [R] Run/Refresh, [F1] Help", sizeof(g_ToastMsg));
                    if (hStatusBar) SetWindowTextA(hStatusBar, g_ToastMsg);
                }
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            if (hTabCtrl) MoveWindow(hTabCtrl, 5, 5, nw - 10, 30, TRUE);
            if (hOutput) MoveWindow(hOutput, 5, 38, nw - 10, nh - 110, TRUE);

            if (hBtnInspRefresh) MoveWindow(hBtnInspRefresh, 10, nh - 68, 110, 26, TRUE);
            if (hBtnInspBench)   MoveWindow(hBtnInspBench, 130, nh - 68, 160, 26, TRUE);

            if (hBtnCpu)  MoveWindow(hBtnCpu, 10, nh - 68, 105, 26, TRUE);
            if (hBtnRam)  MoveWindow(hBtnRam, 125, nh - 68, 105, 26, TRUE);
            if (hBtnDisk) MoveWindow(hBtnDisk, 240, nh - 68, 105, 26, TRUE);
            if (hBtnAll)  MoveWindow(hBtnAll, 355, nh - 68, 110, 26, TRUE);

            if (hBtnSvcRefresh) MoveWindow(hBtnSvcRefresh, 10, nh - 68, 120, 26, TRUE);
            if (hBtnSvcFilter)  MoveWindow(hBtnSvcFilter, 140, nh - 68, 180, 26, TRUE);

            if (hBtnLogClear)  MoveWindow(hBtnLogClear, 10, nh - 68, 120, 26, TRUE);
            if (hBtnLogExport) MoveWindow(hBtnLogExport, 140, nh - 68, 120, 26, TRUE);

            if (hBtnExpTxt)  MoveWindow(hBtnExpTxt, 10, nh - 68, 105, 26, TRUE);
            if (hBtnExpJson) MoveWindow(hBtnExpJson, 120, nh - 68, 110, 26, TRUE);
            if (hBtnExpHtml) MoveWindow(hBtnExpHtml, 235, nh - 68, 110, 26, TRUE);
            if (hBtnExpCsv)  MoveWindow(hBtnExpCsv, 350, nh - 68, 105, 26, TRUE);
            if (hBtnExpMd)   MoveWindow(hBtnExpMd, 460, nh - 68, 105, 26, TRUE);

            if (hBtnHelp) MoveWindow(hBtnHelp, nw - 115, nh - 68, 95, 26, TRUE);

            if (hStatusBar) MoveWindow(hStatusBar, 10, nh - 34, nw - 20, 22, TRUE);
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

    HWND hwnd = CreateWindowEx(0, "KSysApp", "KSys Workstation Diagnostics - Press [1-5] for Tabs, [F1] for Help", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, realW, realH, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN) {
            BOOL ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            BOOL alt = (GetKeyState(VK_MENU) & 0x8000) != 0;
            if (!ctrl && !alt) {
                if (msg.wParam >= '1' && msg.wParam <= '5') {
                    int newTab = (int)(msg.wParam - '1');
                    g_CurrentTab = newTab;
                    if (hTabCtrl) TabCtrl_SetCurSel(hTabCtrl, newTab);
                    UpdateView();
                    continue;
                } else if (msg.wParam == VK_LEFT) {
                    g_CurrentTab = (g_CurrentTab - 1 + 5) % 5;
                    if (hTabCtrl) TabCtrl_SetCurSel(hTabCtrl, g_CurrentTab);
                    UpdateView();
                    continue;
                } else if (msg.wParam == VK_RIGHT) {
                    g_CurrentTab = (g_CurrentTab + 1) % 5;
                    if (hTabCtrl) TabCtrl_SetCurSel(hTabCtrl, g_CurrentTab);
                    UpdateView();
                    continue;
                } else if (msg.wParam == VK_F1 || msg.wParam == 'H' || msg.wParam == 'h') {
                    ShowHelpDialog(hwnd);
                    continue;
                } else if (msg.wParam == VK_ESCAPE) {
                    if (g_CurrentTab != 0) {
                        g_CurrentTab = 0;
                        if (hTabCtrl) TabCtrl_SetCurSel(hTabCtrl, 0);
                        UpdateView();
                        ShowNativeToast(hwnd, "Switched to Hardware Inspector [1]");
                    }
                    continue;
                } else if (msg.wParam == VK_F5 || msg.wParam == 'R' || msg.wParam == 'r') {
                    if (g_CurrentTab == 1) {
                        RunCpuBenchmark();
                        RunRamBenchmark();
                        RunDiskBenchmark();
                        ShowNativeToast(hwnd, "All diagnostic benchmarks complete.");
                    } else if (g_CurrentTab == 2) {
                        LogEvent("INFO", "Refreshed Win32 Services & Telemetry status");
                        ShowNativeToast(hwnd, "Refreshed Services & Telemetry.");
                    } else if (g_CurrentTab == 0) {
                        ShowNativeToast(hwnd, "Hardware specifications refreshed.");
                    } else {
                        ShowNativeToast(hwnd, "Refreshed view.");
                    }
                    UpdateView();
                    continue;
                } else if ((msg.wParam == 'C' || msg.wParam == 'c') && g_CurrentTab == 1) {
                    RunCpuBenchmark();
                    ShowNativeToast(hwnd, "CPU Benchmark finished.");
                    UpdateView();
                    continue;
                } else if ((msg.wParam == 'M' || msg.wParam == 'm') && g_CurrentTab == 1) {
                    RunRamBenchmark();
                    ShowNativeToast(hwnd, "RAM Benchmark finished.");
                    UpdateView();
                    continue;
                } else if ((msg.wParam == 'D' || msg.wParam == 'd') && g_CurrentTab == 1) {
                    RunDiskBenchmark();
                    ShowNativeToast(hwnd, "Disk Benchmark finished.");
                    UpdateView();
                    continue;
                } else if ((msg.wParam == 'S' || msg.wParam == 's') && g_CurrentTab == 2) {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_SVC_FILTER, BN_CLICKED), 0);
                    continue;
                } else if ((msg.wParam == 'L' || msg.wParam == 'l') && g_CurrentTab == 3) {
                    ClearLogs(hwnd);
                    continue;
                } else if ((msg.wParam == 'E' || msg.wParam == 'e') && g_CurrentTab == 3) {
                    ExportLogs(hwnd);
                    continue;
                } else if ((msg.wParam == 'E' || msg.wParam == 'e') && (g_CurrentTab == 4 || g_CurrentTab == 0)) {
                    ExportReport(hwnd, 0);
                    continue;
                } else if ((msg.wParam == 'J' || msg.wParam == 'j') && g_CurrentTab == 4) {
                    ExportReport(hwnd, 1);
                    continue;
                } else if ((msg.wParam == 'T' || msg.wParam == 't') && g_CurrentTab == 4) {
                    ExportReport(hwnd, 2);
                    continue;
                } else if ((msg.wParam == 'V' || msg.wParam == 'v') && g_CurrentTab == 4) {
                    ExportReport(hwnd, 3);
                    continue;
                } else if ((msg.wParam == 'K' || msg.wParam == 'k') && g_CurrentTab == 4) {
                    ExportReport(hwnd, 4);
                    continue;
                }
            }
        }
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    ExitProcess(0);
}
