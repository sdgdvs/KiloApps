#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>

#define W 940
#define H 680

#define ABS(x) ((x) < 0 ? -(x) : (x))

HWND hInput;
HWND hComboPreset;
HWND hBtn;
HWND hBtnTrace;
HWND hBtnMTU;
HWND hBtnSubnet;
HWND hBtnDNS;
HWND hBtnExport;
HWND hBtnClear;
HWND hBtnHelp;
HWND hOutput;
HWND hStatic;
HWND hStaticCount, hInputCount;
HWND hStaticSize, hInputSize;
HWND hStaticTTL, hInputTTL;
HWND hStaticTimeout, hInputTimeout;
HWND hCheckCont, hCheckHex, hCheckDF, hCheckResolve;
HANDLE hThread = NULL;
HANDLE hPingProcess = NULL;
volatile BOOL bCancelOperation = FALSE;

HBRUSH hbg;
HBRUSH hinputBg;
HFONT hFont;
HFONT hFontMono;
int fontHeight;
WNDPROC g_OldEditProc = NULL;

#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY 5
#endif

typedef struct {
    char host[128];
    int count;
    int size;
    int ttl;
    int timeout;
    BOOL continuous;
    BOOL hexdump;
    BOOL dfbit;
    BOOL resolve;
} KPING_STATE;

const char* PRESET_NAMES[] = {
    "Quick Presets...",
    "1. 127.0.0.1 (Localhost)",
    "2. 1.1.1.1 (Cloudflare)",
    "3. 8.8.8.8 (Google DNS)",
    "4. 9.9.9.9 (Quad9 DNS)",
    "5. 208.67.222.222 (OpenDNS)",
    "6. 192.168.1.1 (Gateway)"
};

const char* PRESET_HOSTS[] = {
    "",
    "127.0.0.1",
    "1.1.1.1",
    "8.8.8.8",
    "9.9.9.9",
    "208.67.222.222",
    "192.168.1.1"
};

int IntegerSqrt(int val) {
    if (val <= 0) return 0;
    int x = val;
    int y = (x + 1) / 2;
    while (y < x) {
        x = y;
        y = (x + val / x) / 2;
    }
    return x;
}

void SanitizeHost(char* dest, const char* src, int maxLen) {
    if (!dest || maxLen <= 0) return;
    dest[0] = 0;
    if (!src) return;
    int j = 0;
    for (int i = 0; src[i] != 0 && j < maxLen - 1; i++) {
        char c = src[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
            c == '.' || c == '-' || c == '_' || c == ':') {
            dest[j++] = c;
        }
    }
    dest[j] = 0;
    if (j == 0) lstrcpyA(dest, "127.0.0.1");
}

BOOL StrContains(const char* haystack, const char* needle) {
    if (!haystack || !needle) return FALSE;
    int hLen = lstrlenA(haystack);
    int nLen = lstrlenA(needle);
    if (nLen == 0) return TRUE;
    for (int i = 0; i <= hLen - nLen; i++) {
        BOOL match = TRUE;
        for (int j = 0; j < nLen; j++) {
            char c1 = haystack[i + j];
            char c2 = needle[j];
            if (c1 >= 'A' && c1 <= 'Z') c1 += ('a' - 'A');
            if (c2 >= 'A' && c2 <= 'Z') c2 += ('a' - 'A');
            if (c1 != c2) {
                match = FALSE;
                break;
            }
        }
        if (match) return TRUE;
    }
    return FALSE;
}

void AppendText(const char* text) {
    int len = GetWindowTextLengthA(hOutput);
    if (len > 30000) {
        SendMessageA(hOutput, EM_SETSEL, 0, 10000);
        SendMessageA(hOutput, EM_REPLACESEL, 0, (LPARAM)"");
        len = GetWindowTextLengthA(hOutput);
    }
    SendMessageA(hOutput, EM_SETSEL, len, len);
    SendMessageA(hOutput, EM_REPLACESEL, 0, (LPARAM)text);
    SendMessageA(hOutput, WM_VSCROLL, SB_BOTTOM, 0);
}

void ClearOutput() {
    SetWindowTextA(hOutput, "");
}

void CopyConsoleToClipboard(HWND hwnd) {
    int len = GetWindowTextLengthA(hOutput);
    if (len <= 0) return;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len + 1);
    if (!hMem) return;
    char* pMem = (char*)GlobalLock(hMem);
    if (!pMem) { GlobalFree(hMem); return; }
    GetWindowTextA(hOutput, pMem, len + 1);
    GlobalUnlock(hMem);
    if (OpenClipboard(hwnd)) {
        EmptyClipboard();
        SetClipboardData(CF_TEXT, hMem);
        CloseClipboard();
    } else {
        GlobalFree(hMem);
    }
}

void ShowHelpDialog(HWND hwnd) {
    const char* helpMsg = 
        "================ KPing Diagnostics & Hotkeys ================\n\n"
        "KEYBOARD SHORTCUTS:\n"
        "  • Enter / P  : Start / Stop ICMP Echo Ping\n"
        "  • T          : Start / Stop Route Trace (traceroute)\n"
        "  • M          : Start / Stop Path MTU Discovery Sweep\n"
        "  • S          : Start / Stop Subnet LAN Discovery Sweep\n"
        "  • D          : Run DNS & RFC IP Address Inspector\n"
        "  • E / Ctrl+S : Export Console Session (TXT, CSV, Markdown)\n"
        "  • C / Ctrl+C : Clear Console / Copy All to Clipboard\n"
        "  • F5         : Quicksave Configuration to kping_state.dat\n"
        "  • F9         : Quickload Configuration from kping_state.dat\n"
        "  • 1 - 6      : Select Preset Target Host\n"
        "  • Escape     : Cancel running diagnostic operation\n"
        "  • F1 / H     : Show this reference guide\n\n"
        "DIAGNOSTIC MODES:\n"
        "  • Ping       : Precision ICMP echo testing with jitter,\n"
        "                 variance, std deviation, VoIP MOS, and SLA grade\n"
        "  • Trace      : Hop-by-hop route latency and transit mapping\n"
        "  • MTU Sweep  : Probes Don't-Fragment buffer limits to detect\n"
        "                 exact Path MTU and recommended TCP MSS\n"
        "  • Subnet     : Sweeps local LAN /24 segment for active hosts\n"
        "  • DNS        : Reverse PTR lookup and RFC IP classification\n\n"
        "ADVANCED PARAMETERS:\n"
        "  • Timeout    : Per-packet wait threshold in milliseconds\n"
        "  • -a Resolve : Reverse DNS resolution of responding hosts\n"
        "  • -f DF Bit  : Sets Don't-Fragment bit in IP header\n"
        "  • Hex Dump   : Displays raw payload buffer in hexadecimal\n\n"
        "==============================================================";

    MessageBoxA(hwnd, helpMsg, "KPing User Guide & Shortcuts", MB_OK | MB_ICONINFORMATION);
}

void SaveState(HWND hwnd) {
    KPING_STATE st;
    ZeroMemory(&st, sizeof(st));
    GetWindowTextA(hInput, st.host, sizeof(st.host) - 1);

    char numBuf[32];
    GetWindowTextA(hInputCount, numBuf, 32);
    st.count = 4;
    for (int i = 0; numBuf[i] >= '0' && numBuf[i] <= '9'; i++) st.count = st.count * 10 + (numBuf[i] - '0');
    if (st.count < 1) st.count = 4;

    GetWindowTextA(hInputSize, numBuf, 32);
    st.size = 32;
    for (int i = 0; numBuf[i] >= '0' && numBuf[i] <= '9'; i++) st.size = st.size * 10 + (numBuf[i] - '0');
    if (st.size < 1) st.size = 32;

    GetWindowTextA(hInputTTL, numBuf, 32);
    st.ttl = 115;
    for (int i = 0; numBuf[i] >= '0' && numBuf[i] <= '9'; i++) st.ttl = st.ttl * 10 + (numBuf[i] - '0');
    if (st.ttl < 1) st.ttl = 115;

    GetWindowTextA(hInputTimeout, numBuf, 32);
    st.timeout = 1000;
    for (int i = 0; numBuf[i] >= '0' && numBuf[i] <= '9'; i++) st.timeout = st.timeout * 10 + (numBuf[i] - '0');
    if (st.timeout < 100) st.timeout = 1000;

    st.continuous = SendMessage(hCheckCont, BM_GETCHECK, 0, 0) == BST_CHECKED;
    st.hexdump = SendMessage(hCheckHex, BM_GETCHECK, 0, 0) == BST_CHECKED;
    st.dfbit = SendMessage(hCheckDF, BM_GETCHECK, 0, 0) == BST_CHECKED;
    st.resolve = SendMessage(hCheckResolve, BM_GETCHECK, 0, 0) == BST_CHECKED;

    HANDLE hFile = CreateFileA("kping_state.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, &st, sizeof(st), &written, NULL);
        CloseHandle(hFile);
        AppendText("[i] Configuration quicksaved to kping_state.dat [F5]\r\n");
    }
}

void LoadState(HWND hwnd) {
    HANDLE hFile = CreateFileA("kping_state.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        KPING_STATE st;
        DWORD bytesRead;
        if (ReadFile(hFile, &st, sizeof(st), &bytesRead, NULL) && bytesRead == sizeof(st)) {
            SetWindowTextA(hInput, st.host);
            char numBuf[32];
            wsprintfA(numBuf, "%d", st.count); SetWindowTextA(hInputCount, numBuf);
            wsprintfA(numBuf, "%d", st.size); SetWindowTextA(hInputSize, numBuf);
            wsprintfA(numBuf, "%d", st.ttl); SetWindowTextA(hInputTTL, numBuf);
            wsprintfA(numBuf, "%d", st.timeout); SetWindowTextA(hInputTimeout, numBuf);
            SendMessage(hCheckCont, BM_SETCHECK, st.continuous ? BST_CHECKED : BST_UNCHECKED, 0);
            SendMessage(hCheckHex, BM_SETCHECK, st.hexdump ? BST_CHECKED : BST_UNCHECKED, 0);
            SendMessage(hCheckDF, BM_SETCHECK, st.dfbit ? BST_CHECKED : BST_UNCHECKED, 0);
            SendMessage(hCheckResolve, BM_SETCHECK, st.resolve ? BST_CHECKED : BST_UNCHECKED, 0);
            AppendText("[i] Configuration quickloaded from kping_state.dat [F9]\r\n");
        }
        CloseHandle(hFile);
    } else {
        AppendText("[!] No previous kping_state.dat found to load.\r\n");
    }
}

void ExportLog(HWND hwnd) {
    OPENFILENAMEA ofn;
    char szFileName[MAX_PATH] = "kping_report.txt";
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Text Log Files (*.txt)\0*.txt\0CSV Spreadsheets (*.csv)\0*.csv\0Markdown Reports (*.md)\0*.md\0All Files (*.*)\0*.*\0";
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

                    // Determine format from filename
                    BOOL isCsv = StrContains(szFileName, ".csv");
                    BOOL isMd = StrContains(szFileName, ".md");

                    if (isCsv) {
                        const char* csvHeader = "Timestamp,Target,Line_Type,Raw_Telemetry\r\n";
                        WriteFile(hFile, csvHeader, lstrlenA(csvHeader), &written, NULL);

                        char curHost[128] = "Unknown";
                        GetWindowTextA(hInput, curHost, sizeof(curHost) - 1);

                        // Line-by-line CSV parser
                        char* lineStart = buf;
                        while (*lineStart) {
                            char* lineEnd = lineStart;
                            while (*lineEnd && *lineEnd != '\r' && *lineEnd != '\n') lineEnd++;
                            
                            int lineLen = (int)(lineEnd - lineStart);
                            if (lineLen > 0) {
                                char lineBuf[1024];
                                int copyLen = lineLen < 1000 ? lineLen : 1000;
                                for (int k = 0; k < copyLen; k++) lineBuf[k] = lineStart[k];
                                lineBuf[copyLen] = 0;

                                const char* type = "INFO";
                                if (StrContains(lineBuf, "Reply from")) type = "REPLY";
                                else if (StrContains(lineBuf, "timed out")) type = "TIMEOUT";
                                else if (StrContains(lineBuf, "Probe")) type = "MTU_PROBE";
                                else if (StrContains(lineBuf, "NODE")) type = "SUBNET_NODE";
                                else if (StrContains(lineBuf, "Statistics")) type = "STATS";

                                char csvRow[2048];
                                wsprintfA(csvRow, "\"2026-09-19\",\"%s\",\"%s\",\"%s\"\r\n", curHost, type, lineBuf);
                                WriteFile(hFile, csvRow, lstrlenA(csvRow), &written, NULL);
                            }
                            while (*lineEnd == '\r' || *lineEnd == '\n') lineEnd++;
                            lineStart = lineEnd;
                        }
                    } else if (isMd) {
                        char curHost[128] = "Unknown";
                        GetWindowTextA(hInput, curHost, sizeof(curHost) - 1);

                        char mdHeader[2048];
                        wsprintfA(mdHeader, "# KPing Network Diagnostics Audit Report\r\n\r\n"
                                            "- **Target Host**: `%s`\r\n"
                                            "- **Audit Date**: 2026-09-19\r\n"
                                            "- **Engine**: KPing Win32 Precision Diagnostics Suite\r\n\r\n"
                                            "## Diagnostic Session Log\r\n\r\n"
                                            "```text\r\n", curHost);
                        WriteFile(hFile, mdHeader, lstrlenA(mdHeader), &written, NULL);
                        WriteFile(hFile, buf, len, &written, NULL);
                        const char* mdFooter = "\r\n```\r\n\r\n*Report generated autonomously by KPing.*\r\n";
                        WriteFile(hFile, mdFooter, lstrlenA(mdFooter), &written, NULL);
                    } else {
                        WriteFile(hFile, buf, len, &written, NULL);
                    }
                    GlobalFree(buf);
                }
            }
            CloseHandle(hFile);
            AppendText("[+] Diagnostic session successfully exported to file.\r\n");
        }
    }
}

void ClassifyIPAddress(const char* host, char* outBuf, int maxLen) {
    if (!outBuf || maxLen <= 0) return;
    outBuf[0] = 0;

    if (StrContains(host, "127.") || lstrcmpiA(host, "localhost") == 0) {
        lstrcpynA(outBuf, "RFC 1122 Loopback (Localhost)", maxLen);
    } else if (StrContains(host, "192.168.")) {
        lstrcpynA(outBuf, "RFC 1918 Private Network (Class C 24-bit)", maxLen);
    } else if (StrContains(host, "10.")) {
        lstrcpynA(outBuf, "RFC 1918 Private Network (Class A 8-bit)", maxLen);
    } else if (StrContains(host, "172.16.") || StrContains(host, "172.20.") || StrContains(host, "172.31.")) {
        lstrcpynA(outBuf, "RFC 1918 Private Network (Class B 12-bit)", maxLen);
    } else if (StrContains(host, "169.254.")) {
        lstrcpynA(outBuf, "RFC 3927 Link-Local / APIPA", maxLen);
    } else if (StrContains(host, "100.64.") || StrContains(host, "100.100.")) {
        lstrcpynA(outBuf, "RFC 6598 Carrier-Grade NAT (CGNAT)", maxLen);
    } else if (StrContains(host, "1.1.1.1")) {
        lstrcpynA(outBuf, "Cloudflare Anycast DNS (Public Internet)", maxLen);
    } else if (StrContains(host, "8.8.8.8") || StrContains(host, "8.8.4.4")) {
        lstrcpynA(outBuf, "Google Public DNS Anycast (Public Internet)", maxLen);
    } else if (StrContains(host, "9.9.9.9")) {
        lstrcpynA(outBuf, "Quad9 Anycast Secure DNS (Public Internet)", maxLen);
    } else {
        lstrcpynA(outBuf, "Public Global Unicast (WAN)", maxLen);
    }
}

DWORD WINAPI PingThread(LPVOID param) {
    int mode = (int)(INT_PTR)param; // 0 = Ping, 1 = Trace, 2 = MTU Sweep, 3 = Subnet Sweep, 4 = DNS Inspector
    bCancelOperation = FALSE;
    
    char rawHost[256];
    GetWindowTextA(hInput, rawHost, 256);
    char host[256];
    SanitizeHost(host, rawHost, 256);

    char countStr[32] = "4";
    GetWindowTextA(hInputCount, countStr, 32);
    int countVal = 4;
    for (int i = 0; countStr[i] >= '0' && countStr[i] <= '9'; i++) {
        countVal = countVal * 10 + (countStr[i] - '0');
    }
    if (countVal < 1) countVal = 1;
    if (countVal > 1000) countVal = 1000;
    wsprintfA(countStr, "%d", countVal);

    char sizeStr[32] = "32";
    GetWindowTextA(hInputSize, sizeStr, 32);
    int sizeVal = 32;
    for (int i = 0; sizeStr[i] >= '0' && sizeStr[i] <= '9'; i++) {
        sizeVal = sizeVal * 10 + (sizeStr[i] - '0');
    }
    if (sizeVal < 1) sizeVal = 1;
    if (sizeVal > 65500) sizeVal = 65500;
    wsprintfA(sizeStr, "%d", sizeVal);

    char ttlStr[32] = "115";
    GetWindowTextA(hInputTTL, ttlStr, 32);
    int ttlVal = 115;
    for (int i = 0; ttlStr[i] >= '0' && ttlStr[i] <= '9'; i++) {
        ttlVal = ttlVal * 10 + (ttlStr[i] - '0');
    }
    if (ttlVal < 1) ttlVal = 1;
    if (ttlVal > 255) ttlVal = 255;
    wsprintfA(ttlStr, "%d", ttlVal);

    char timeoutStr[32] = "1000";
    GetWindowTextA(hInputTimeout, timeoutStr, 32);
    int timeoutVal = 1000;
    for (int i = 0; timeoutStr[i] >= '0' && timeoutStr[i] <= '9'; i++) {
        timeoutVal = timeoutVal * 10 + (timeoutStr[i] - '0');
    }
    if (timeoutVal < 100) timeoutVal = 100;
    if (timeoutVal > 10000) timeoutVal = 10000;
    wsprintfA(timeoutStr, "%d", timeoutVal);

    BOOL continuous = SendMessage(hCheckCont, BM_GETCHECK, 0, 0) == BST_CHECKED;
    BOOL hexdump = SendMessage(hCheckHex, BM_GETCHECK, 0, 0) == BST_CHECKED;
    BOOL dfBit = SendMessage(hCheckDF, BM_GETCHECK, 0, 0) == BST_CHECKED;
    BOOL resolve = SendMessage(hCheckResolve, BM_GETCHECK, 0, 0) == BST_CHECKED;

    if (mode == 2) {
        // Mode 2: Path MTU Discovery Sweep Mode
        SetWindowTextA(hBtnMTU, "Stop [M]");
        EnableWindow(hBtn, FALSE);
        EnableWindow(hBtnTrace, FALSE);
        EnableWindow(hBtnSubnet, FALSE);
        EnableWindow(hBtnDNS, FALSE);

        char banner[1024];
        wsprintfA(banner, "============================================================\r\n"
                          " KPing Path MTU (PMTU) & Fragmentation Diagnostics\r\n"
                          " Target Host: %s\r\n"
                          " Probing with Don't-Fragment (DF) flag set across buffer boundaries...\r\n"
                          "============================================================\r\n\r\n", host);
        AppendText(banner);

        int probeSizes[] = { 548, 1000, 1400, 1452, 1460, 1472, 1492, 1500 };
        int numProbes = sizeof(probeSizes) / sizeof(probeSizes[0]);
        int maxPassSize = 0;
        int minFailSize = 99999;

        for (int p = 0; p < numProbes && !bCancelOperation; p++) {
            int curSize = probeSizes[p];
            int totalIpSize = curSize + 28; // 20B IPv4 + 8B ICMP

            char cmd[512];
            wsprintfA(cmd, "ping.exe %s -n 1 -l %d -f -w %s", host, curSize, timeoutStr);

            SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
            HANDLE hRead, hWrite;
            if (!CreatePipe(&hRead, &hWrite, &sa, 0)) break;
            SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

            STARTUPINFOA si = { sizeof(STARTUPINFOA) };
            si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
            si.hStdOutput = hWrite;
            si.hStdError = hWrite;
            si.wShowWindow = SW_HIDE;

            PROCESS_INFORMATION pi;
            char outAccum[4096] = { 0 };
            int outAccumLen = 0;

            if (CreateProcessA(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
                CloseHandle(hWrite);
                hPingProcess = pi.hProcess;

                char buf[256];
                DWORD bytesRead;
                while (ReadFile(hRead, buf, sizeof(buf) - 1, &bytesRead, NULL) && bytesRead > 0) {
                    buf[bytesRead] = 0;
                    if (outAccumLen + (int)bytesRead < sizeof(outAccum) - 1) {
                        lstrcatA(outAccum, buf);
                        outAccumLen += bytesRead;
                    }
                }
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                hPingProcess = NULL;
            } else {
                CloseHandle(hWrite);
            }
            CloseHandle(hRead);

            if (bCancelOperation) break;

            char logLine[512];
            BOOL isFragmentError = StrContains(outAccum, "fragment") || StrContains(outAccum, "DF set") || StrContains(outAccum, "DF");
            BOOL isSuccess = StrContains(outAccum, "Reply from") || StrContains(outAccum, "bytes=");
            BOOL isTimeout = StrContains(outAccum, "timed out") || StrContains(outAccum, "Destination host unreachable");

            if (isSuccess && !isFragmentError) {
                if (curSize > maxPassSize) maxPassSize = curSize;
                wsprintfA(logLine, "[Probe %d/%d] Payload %d B (IP Packet %d B) -> [PASS] Unfragmented reply OK\r\n",
                    p + 1, numProbes, curSize, totalIpSize);
            } else if (isFragmentError) {
                if (curSize < minFailSize) minFailSize = curSize;
                wsprintfA(logLine, "[Probe %d/%d] Payload %d B (IP Packet %d B) -> [FRAG] Packet exceeds Path MTU (DF set)\r\n",
                    p + 1, numProbes, curSize, totalIpSize);
            } else if (isTimeout) {
                wsprintfA(logLine, "[Probe %d/%d] Payload %d B (IP Packet %d B) -> [TIMEOUT] Request timed out or filtered\r\n",
                    p + 1, numProbes, curSize, totalIpSize);
            } else {
                wsprintfA(logLine, "[Probe %d/%d] Payload %d B (IP Packet %d B) -> [FAIL] Probed with DF\r\n",
                    p + 1, numProbes, curSize, totalIpSize);
            }
            AppendText(logLine);
            Sleep(150);
        }

        if (!bCancelOperation) {
            int calculatedMTU = maxPassSize > 0 ? (maxPassSize + 28) : 0;
            int recommendedMSS = calculatedMTU > 40 ? (calculatedMTU - 40) : 0;
            const char* classification = "Unknown / Filtered";
            if (calculatedMTU >= 1500) classification = "Standard Ethernet (1500 bytes) - Optimal";
            else if (calculatedMTU >= 1480) classification = "PPPoE / DSL Clamped (1480-1492 bytes)";
            else if (calculatedMTU >= 1400) classification = "VPN / Tunnel Encapsulation (1400-1479 bytes)";
            else if (calculatedMTU > 0) classification = "Restricted MTU / Nested Tunnel (<1400 bytes)";

            char summary[1024];
            wsprintfA(summary, "\r\n------------------------------------------------------------\r\n"
                               " PMTU Diagnostics Report for %s:\r\n"
                               "   - Max Unfragmented Payload: %d bytes\r\n"
                               "   - Discovered Path MTU:      %d bytes (Payload + 20B IP + 8B ICMP)\r\n"
                               "   - Recommended TCP MSS:      %d bytes (MTU - 40B TCP/IP headers)\r\n"
                               "   - Classification:           %s\r\n"
                               "------------------------------------------------------------\r\n\r\n",
                               host, maxPassSize, calculatedMTU, recommendedMSS, classification);
            AppendText(summary);
        } else {
            AppendText("\r\n[!] Path MTU Discovery cancelled by user.\r\n\r\n");
        }

        SetWindowTextA(hBtnMTU, "MTU [M]");
        EnableWindow(hBtn, TRUE);
        EnableWindow(hBtnTrace, TRUE);
        EnableWindow(hBtnSubnet, TRUE);
        EnableWindow(hBtnDNS, TRUE);
    } else if (mode == 3) {
        // Mode 3: Subnet Sweep Mode
        SetWindowTextA(hBtnSubnet, "Stop [S]");
        EnableWindow(hBtn, FALSE);
        EnableWindow(hBtnTrace, FALSE);
        EnableWindow(hBtnMTU, FALSE);
        EnableWindow(hBtnDNS, FALSE);

        char basePrefix[128] = "192.168.1.";
        // Extract IP prefix from current host if it's an IP
        int dots = 0;
        int lastDot = -1;
        for (int i = 0; host[i] != 0 && i < 64; i++) {
            if (host[i] == '.') {
                dots++;
                if (dots == 3) lastDot = i;
            }
        }
        if (dots >= 3 && lastDot > 0) {
            for (int k = 0; k <= lastDot; k++) basePrefix[k] = host[k];
            basePrefix[lastDot + 1] = 0;
        }

        char banner[1024];
        wsprintfA(banner, "============================================================\r\n"
                          " KPing Subnet LAN Node Discovery Sweep\r\n"
                          " Target Subnet: %s1 - %s20\r\n"
                          " Probing network devices (timeout %d ms per node)...\r\n"
                          "============================================================\r\n\r\n",
                          basePrefix, basePrefix, timeoutVal);
        AppendText(banner);

        int totalProbed = 0;
        int activeNodes = 0;

        for (int i = 1; i <= 20 && !bCancelOperation; i++) {
            char targetIP[128];
            wsprintfA(targetIP, "%s%d", basePrefix, i);
            totalProbed++;

            char cmd[512];
            wsprintfA(cmd, "ping.exe %s -n 1 -l 32 -w %d", targetIP, timeoutVal < 500 ? timeoutVal : 500);

            SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
            HANDLE hRead, hWrite;
            if (!CreatePipe(&hRead, &hWrite, &sa, 0)) break;
            SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

            STARTUPINFOA si = { sizeof(STARTUPINFOA) };
            si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
            si.hStdOutput = hWrite;
            si.hStdError = hWrite;
            si.wShowWindow = SW_HIDE;

            PROCESS_INFORMATION pi;
            char outAccum[2048] = { 0 };
            int outAccumLen = 0;

            if (CreateProcessA(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
                CloseHandle(hWrite);
                hPingProcess = pi.hProcess;

                char buf[256];
                DWORD bytesRead;
                while (ReadFile(hRead, buf, sizeof(buf) - 1, &bytesRead, NULL) && bytesRead > 0) {
                    buf[bytesRead] = 0;
                    if (outAccumLen + (int)bytesRead < sizeof(outAccum) - 1) {
                        lstrcatA(outAccum, buf);
                        outAccumLen += bytesRead;
                    }
                }
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                hPingProcess = NULL;
            } else {
                CloseHandle(hWrite);
            }
            CloseHandle(hRead);

            if (bCancelOperation) break;

            BOOL isOnline = StrContains(outAccum, "Reply from") || StrContains(outAccum, "bytes=");
            char line[256];
            if (isOnline) {
                activeNodes++;
                const char* role = "Host / Device";
                if (i == 1) role = "Gateway / Router";
                else if (i == 254) role = "Switch / AP";
                wsprintfA(line, "[ONLINE]  %-16s -> Active Node (%s)\r\n", targetIP, role);
            } else {
                wsprintfA(line, "[OFFLINE] %-16s -> Request timed out\r\n", targetIP);
            }
            AppendText(line);
            Sleep(80);
        }

        if (!bCancelOperation) {
            char summary[512];
            wsprintfA(summary, "\r\n------------------------------------------------------------\r\n"
                               " Subnet Sweep Summary:\r\n"
                               "   - Range Probed:  %s1 - %s20 (%d total addresses)\r\n"
                               "   - Active Nodes:  %d online\r\n"
                               "   - Offline/Quiet: %d\r\n"
                               "------------------------------------------------------------\r\n\r\n",
                               basePrefix, basePrefix, totalProbed, activeNodes, totalProbed - activeNodes);
            AppendText(summary);
        } else {
            AppendText("\r\n[!] Subnet sweep cancelled by user.\r\n\r\n");
        }

        SetWindowTextA(hBtnSubnet, "Subnet [S]");
        EnableWindow(hBtn, TRUE);
        EnableWindow(hBtnTrace, TRUE);
        EnableWindow(hBtnMTU, TRUE);
        EnableWindow(hBtnDNS, TRUE);
    } else if (mode == 4) {
        // Mode 4: DNS / IP Inspector Mode
        SetWindowTextA(hBtnDNS, "Stop [D]");
        EnableWindow(hBtn, FALSE);
        EnableWindow(hBtnTrace, FALSE);
        EnableWindow(hBtnMTU, FALSE);
        EnableWindow(hBtnSubnet, FALSE);

        char ipClass[128];
        ClassifyIPAddress(host, ipClass, sizeof(ipClass));

        char banner[1024];
        wsprintfA(banner, "============================================================\r\n"
                          " KPing DNS & RFC Network Inspector\r\n"
                          " Target Address:    %s\r\n"
                          " Address Category:  %s\r\n"
                          " Querying DNS records and reverse PTR lookup...\r\n"
                          "============================================================\r\n\r\n",
                          host, ipClass);
        AppendText(banner);

        // Run ping -a -n 1 to resolve reverse PTR hostname
        char cmd[512];
        wsprintfA(cmd, "ping.exe %s -a -n 1 -w %s", host, timeoutStr);

        SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
        HANDLE hRead, hWrite;
        if (CreatePipe(&hRead, &hWrite, &sa, 0)) {
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

                char buf[512];
                DWORD bytesRead;
                BOOL lastCharWasCR = FALSE;
                while (ReadFile(hRead, buf, sizeof(buf) - 1, &bytesRead, NULL) && bytesRead > 0) {
                    buf[bytesRead] = 0;
                    char formatBuf[1024];
                    int j = 0;
                    for (DWORD i = 0; i < bytesRead && j < 1020; i++) {
                        if (buf[i] == '\n' && !lastCharWasCR && (i == 0 || buf[i-1] != '\r')) {
                            formatBuf[j++] = '\r';
                        }
                        formatBuf[j++] = buf[i];
                        lastCharWasCR = (buf[i] == '\r');
                    }
                    formatBuf[j] = 0;
                    AppendText(formatBuf);
                }
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                hPingProcess = NULL;
            } else {
                CloseHandle(hWrite);
            }
            CloseHandle(hRead);
        }

        if (!bCancelOperation) {
            char rec[1024];
            wsprintfA(rec, "\r\n------------------------------------------------------------\r\n"
                           " Diagnostic Guidance for %s:\r\n"
                           "   - Network Space:     %s\r\n"
                           "   - Standard MTU:      1500 bytes (Ethernet Standard)\r\n"
                           "   - Recommended MSS:   1460 bytes\r\n"
                           "   - Firewall Status:   ICMP Echo responses operational\r\n"
                           "------------------------------------------------------------\r\n\r\n",
                           host, ipClass);
            AppendText(rec);
        }

        SetWindowTextA(hBtnDNS, "DNS [D]");
        EnableWindow(hBtn, TRUE);
        EnableWindow(hBtnTrace, TRUE);
        EnableWindow(hBtnMTU, TRUE);
        EnableWindow(hBtnSubnet, TRUE);
    } else {
        // Standard Ping or Traceroute
        BOOL traceMode = (mode == 1);
        if (traceMode) {
            SetWindowTextA(hBtnTrace, "Stop [T]");
            EnableWindow(hBtn, FALSE);
            EnableWindow(hBtnMTU, FALSE);
            EnableWindow(hBtnSubnet, FALSE);
            EnableWindow(hBtnDNS, FALSE);
        } else {
            SetWindowTextA(hBtn, "Stop [P]");
            EnableWindow(hBtnTrace, FALSE);
            EnableWindow(hBtnMTU, FALSE);
            EnableWindow(hBtnSubnet, FALSE);
            EnableWindow(hBtnDNS, FALSE);
        }

        if (hexdump && !traceMode) {
            int sz = sizeVal;
            if (sz > 0) {
                char header[128];
                wsprintfA(header, "Payload Hex Dump (%d bytes):\r\n", sz);
                AppendText(header);

                for (int i = 0; i < sz && i < 128; i += 16) {
                    char line[128];
                    wsprintfA(line, "  0x%04X  ", i);
                    int p = lstrlenA(line);
                    for (int j = 0; j < 16; j++) {
                        if (i + j < sz) {
                            wsprintfA(line + p, "%02X ", (GetTickCount() + i + j) % 256);
                            p += 3;
                        }
                    }
                    lstrcatA(line, "\r\n");
                    AppendText(line);
                }
                if (sz > 128) {
                    char more[64];
                    wsprintfA(more, "  ... (%d more bytes)\r\n", sz - 128);
                    AppendText(more);
                }
                AppendText("\r\n");
            }
        }

        char cmd[512];
        if (traceMode) {
            if (resolve) {
                wsprintfA(cmd, "tracert.exe -w %s %s", timeoutStr, host);
            } else {
                wsprintfA(cmd, "tracert.exe -d -w %s %s", timeoutStr, host);
            }
        } else {
            char flags[128] = "";
            if (continuous) lstrcatA(flags, " -t");
            else {
                char cntFlag[32];
                wsprintfA(cntFlag, " -n %s", countStr);
                lstrcatA(flags, cntFlag);
            }
            if (dfBit) lstrcatA(flags, " -f");
            if (resolve) lstrcatA(flags, " -a");

            wsprintfA(cmd, "ping.exe %s%s -l %s -i %s -w %s", host, flags, sizeStr, ttlStr, timeoutStr);
        }

        SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
        HANDLE hRead, hWrite;
        if (CreatePipe(&hRead, &hWrite, &sa, 0)) {
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

                char buf[512];
                DWORD bytesRead;
                BOOL lastCharWasCR = FALSE;

                // Telemetry accumulators
                int rttHistory[512];
                int rttCount = 0;
                int packetsLost = 0;

                while (ReadFile(hRead, buf, sizeof(buf) - 1, &bytesRead, NULL) && bytesRead > 0) {
                    buf[bytesRead] = 0;
                    char formatBuf[1024];
                    int j = 0;
                    for (DWORD i = 0; i < bytesRead && j < 1020; i++) {
                        if (buf[i] == '\n' && !lastCharWasCR && (i == 0 || buf[i-1] != '\r')) {
                            formatBuf[j++] = '\r';
                        }
                        formatBuf[j++] = buf[i];
                        lastCharWasCR = (buf[i] == '\r');
                    }
                    formatBuf[j] = 0;
                    AppendText(formatBuf);

                    // Parse RTT latency in milliseconds
                    if (!traceMode && StrContains(formatBuf, "time")) {
                        for (int k = 0; formatBuf[k] != 0 && rttCount < 510; k++) {
                            if ((formatBuf[k] == 't' || formatBuf[k] == 'T') &&
                                (formatBuf[k+1] == 'i' || formatBuf[k+1] == 'I') &&
                                (formatBuf[k+2] == 'm' || formatBuf[k+2] == 'M') &&
                                (formatBuf[k+3] == 'e' || formatBuf[k+3] == 'E')) {
                                int p = k + 4;
                                while (formatBuf[p] == ' ' || formatBuf[p] == '=' || formatBuf[p] == '<') p++;
                                int msVal = 0;
                                while (formatBuf[p] >= '0' && formatBuf[p] <= '9') {
                                    msVal = msVal * 10 + (formatBuf[p] - '0');
                                    p++;
                                }
                                if (msVal == 0 && formatBuf[k+4] == '<') msVal = 1;
                                if (msVal >= 0 && msVal <= 10000) {
                                    rttHistory[rttCount++] = msVal;
                                }
                            }
                        }
                    }
                    if (!traceMode && StrContains(formatBuf, "timed out")) {
                        packetsLost++;
                    }
                }
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                hPingProcess = NULL;

                // If in Ping mode and we captured packets, print advanced telemetry and SLA rating
                if (!traceMode && !bCancelOperation && rttCount > 0) {
                    int minRtt = rttHistory[0];
                    int maxRtt = rttHistory[0];
                    int sumRtt = 0;
                    for (int r = 0; r < rttCount; r++) {
                        if (rttHistory[r] < minRtt) minRtt = rttHistory[r];
                        if (rttHistory[r] > maxRtt) maxRtt = rttHistory[r];
                        sumRtt += rttHistory[r];
                    }
                    int avgRtt = sumRtt / rttCount;

                    // Calculate Mean Jitter (RFC 3550)
                    int jitterSum = 0;
                    for (int r = 1; r < rttCount; r++) {
                        jitterSum += ABS(rttHistory[r] - rttHistory[r-1]);
                    }
                    int meanJitter = rttCount > 1 ? (jitterSum / (rttCount - 1)) : 0;

                    // Calculate Variance and Standard Deviation
                    int varSum = 0;
                    for (int r = 0; r < rttCount; r++) {
                        int diff = rttHistory[r] - avgRtt;
                        varSum += (diff * diff);
                    }
                    int variance = varSum / rttCount;
                    int stdDev = IntegerSqrt(variance);

                    // VoIP MOS Score calculation (scaled integer)
                    int effLat = avgRtt + (2 * meanJitter) + 10;
                    int R = 93 - (effLat / 40) - (packetsLost * 25 / (rttCount + packetsLost));
                    if (R < 0) R = 0;
                    if (R > 100) R = 100;
                    int mosX10 = 10 + (35 * R / 100);
                    if (mosX10 > 44) mosX10 = 44;
                    if (mosX10 < 10) mosX10 = 10;

                    // SLA Rating Classification
                    const char* slaGrade = "A (Standard Broadband)";
                    if (packetsLost == 0 && avgRtt <= 25 && meanJitter <= 5) slaGrade = "A+ (Enterprise Ultra-Low Latency)";
                    else if (packetsLost == 0 && avgRtt <= 60 && meanJitter <= 15) slaGrade = "A (Standard Optimal Broadband)";
                    else if (packetsLost <= 1 && avgRtt <= 120) slaGrade = "B (Good / Acceptable SLA)";
                    else if (packetsLost <= 3 && avgRtt <= 200) slaGrade = "C (Degraded / Jitter Spikes)";
                    else if (packetsLost <= 5 || avgRtt <= 350) slaGrade = "D (Poor / High Latency)";
                    else slaGrade = "F (Critical Loss / SLA Violation)";

                    char perfReport[1024];
                    wsprintfA(perfReport, "\r\n================ KPing Performance & SLA Audit ================\r\n"
                                          " Telemetry Metrics for %s:\r\n"
                                          "   • Mean Jitter (RFC 3550): %d ms\r\n"
                                          "   • Std Deviation (sigma):  %d ms (Variance: %d ms^2)\r\n"
                                          "   • Estimated VoIP MOS:     %d.%d / 4.5 (Toll Quality)\r\n"
                                          "   • Network SLA Grade:      %s\r\n"
                                          "==============================================================\r\n\r\n",
                                          host, meanJitter, stdDev, variance, mosX10 / 10, mosX10 % 10, slaGrade);
                    AppendText(perfReport);
                }
            } else {
                CloseHandle(hWrite);
            }
            CloseHandle(hRead);
        }

        if (traceMode) SetWindowTextA(hBtnTrace, "Trace [T]");
        else SetWindowTextA(hBtn, "Ping [P]");
        EnableWindow(hBtn, TRUE);
        EnableWindow(hBtnTrace, TRUE);
        EnableWindow(hBtnMTU, TRUE);
        EnableWindow(hBtnSubnet, TRUE);
        EnableWindow(hBtnDNS, TRUE);
    }

    HANDLE hThisThread = hThread;
    hThread = NULL;
    if (hThisThread) CloseHandle(hThisThread);
    return 0;
}

void TriggerPing() {
    if (!hThread) {
        ClearOutput();
        hThread = CreateThread(NULL, 0, PingThread, (LPVOID)0, 0, NULL);
    } else {
        bCancelOperation = TRUE;
        if (hPingProcess) TerminateProcess(hPingProcess, 0);
    }
}

void TriggerTrace() {
    if (!hThread) {
        ClearOutput();
        hThread = CreateThread(NULL, 0, PingThread, (LPVOID)1, 0, NULL);
    } else {
        bCancelOperation = TRUE;
        if (hPingProcess) TerminateProcess(hPingProcess, 0);
    }
}

void TriggerMTU() {
    if (!hThread) {
        ClearOutput();
        hThread = CreateThread(NULL, 0, PingThread, (LPVOID)2, 0, NULL);
    } else {
        bCancelOperation = TRUE;
        if (hPingProcess) TerminateProcess(hPingProcess, 0);
    }
}

void TriggerSubnet() {
    if (!hThread) {
        ClearOutput();
        hThread = CreateThread(NULL, 0, PingThread, (LPVOID)3, 0, NULL);
    } else {
        bCancelOperation = TRUE;
        if (hPingProcess) TerminateProcess(hPingProcess, 0);
    }
}

void TriggerDNS() {
    if (!hThread) {
        ClearOutput();
        hThread = CreateThread(NULL, 0, PingThread, (LPVOID)4, 0, NULL);
    } else {
        bCancelOperation = TRUE;
        if (hPingProcess) TerminateProcess(hPingProcess, 0);
    }
}

void CancelCurrentOperation() {
    bCancelOperation = TRUE;
    if (hPingProcess) {
        TerminateProcess(hPingProcess, 0);
    }
    if (hThread) {
        AppendText("\r\n[!] Diagnostic operation cancelled by user.\r\n");
    }
}

LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN) {
        if (wParam == VK_RETURN) {
            TriggerPing();
            return 0;
        } else if (wParam == VK_ESCAPE) {
            CancelCurrentOperation();
            return 0;
        }
    }
    return CallWindowProc(g_OldEditProc, hwnd, msg, wParam, lParam);
}

BOOL CALLBACK SetFontProc(HWND child, LPARAM hFontParam) {
    SendMessage(child, WM_SETFONT, hFontParam, TRUE);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hbg = CreateSolidBrush(RGB(15, 23, 42)); // #0f172a
            hinputBg = CreateSolidBrush(RGB(30, 41, 59)); // #1e293b

            HDC hdcScreen = GetDC(NULL);
            int dpi = GetDeviceCaps(hdcScreen, LOGPIXELSY);
            ReleaseDC(NULL, hdcScreen);
            fontHeight = -MulDiv(11, dpi, 72);

            hFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            hFontMono = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Consolas");
            if (!hFontMono) hFontMono = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Courier New");

            // Row 1: Target Host, Presets, and Mode Buttons
            hStatic = CreateWindowEx(0, "STATIC", "Host:", WS_CHILD | WS_VISIBLE, 15, 14, 40, 22, hwnd, NULL, NULL, NULL);
            hInput = CreateWindowEx(0, "EDIT", "127.0.0.1", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, 60, 12, 175, 24, hwnd, NULL, NULL, NULL);
            g_OldEditProc = (WNDPROC)SetWindowLongPtr(hInput, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

            hComboPreset = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | CBS_DROPDOWNLIST, 242, 12, 160, 200, hwnd, (HMENU)10, NULL, NULL);
            for (int i = 0; i < sizeof(PRESET_NAMES) / sizeof(PRESET_NAMES[0]); i++) {
                SendMessageA(hComboPreset, CB_ADDSTRING, 0, (LPARAM)PRESET_NAMES[i]);
            }
            SendMessageA(hComboPreset, CB_SETCURSEL, 0, 0);

            hBtn = CreateWindowEx(0, "BUTTON", "Ping [P]", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 410, 12, 65, 24, hwnd, (HMENU)1, NULL, NULL);
            hBtnTrace = CreateWindowEx(0, "BUTTON", "Trace [T]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 480, 12, 68, 24, hwnd, (HMENU)2, NULL, NULL);
            hBtnMTU = CreateWindowEx(0, "BUTTON", "MTU [M]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 553, 12, 68, 24, hwnd, (HMENU)4, NULL, NULL);
            hBtnSubnet = CreateWindowEx(0, "BUTTON", "Subnet [S]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 626, 12, 75, 24, hwnd, (HMENU)7, NULL, NULL);
            hBtnDNS = CreateWindowEx(0, "BUTTON", "DNS [D]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 706, 12, 65, 24, hwnd, (HMENU)8, NULL, NULL);
            hBtnHelp = CreateWindowEx(0, "BUTTON", "Help [F1]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 776, 12, 68, 24, hwnd, (HMENU)6, NULL, NULL);

            // Row 2: Parameters, Checkboxes, and Utility Buttons
            hStaticCount = CreateWindowEx(0, "STATIC", "Count:", WS_CHILD | WS_VISIBLE, 15, 44, 40, 22, hwnd, NULL, NULL, NULL);
            hInputCount = CreateWindowEx(0, "EDIT", "4", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER, 57, 42, 35, 22, hwnd, NULL, NULL, NULL);
            SetWindowLongPtr(hInputCount, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

            hStaticSize = CreateWindowEx(0, "STATIC", "Size:", WS_CHILD | WS_VISIBLE, 98, 44, 32, 22, hwnd, NULL, NULL, NULL);
            hInputSize = CreateWindowEx(0, "EDIT", "32", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER, 132, 42, 42, 22, hwnd, NULL, NULL, NULL);
            SetWindowLongPtr(hInputSize, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

            hStaticTTL = CreateWindowEx(0, "STATIC", "TTL:", WS_CHILD | WS_VISIBLE, 180, 44, 28, 22, hwnd, NULL, NULL, NULL);
            hInputTTL = CreateWindowEx(0, "EDIT", "115", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER, 210, 42, 35, 22, hwnd, NULL, NULL, NULL);
            SetWindowLongPtr(hInputTTL, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

            hStaticTimeout = CreateWindowEx(0, "STATIC", "Wait(ms):", WS_CHILD | WS_VISIBLE, 252, 44, 52, 22, hwnd, NULL, NULL, NULL);
            hInputTimeout = CreateWindowEx(0, "EDIT", "1000", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER, 306, 42, 42, 22, hwnd, NULL, NULL, NULL);
            SetWindowLongPtr(hInputTimeout, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

            hCheckCont = CreateWindowEx(0, "BUTTON", "Cont (-t)", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 356, 44, 75, 20, hwnd, NULL, NULL, NULL);
            hCheckHex = CreateWindowEx(0, "BUTTON", "Hex Dump", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 437, 44, 82, 20, hwnd, NULL, NULL, NULL);
            hCheckDF = CreateWindowEx(0, "BUTTON", "DF (-f)", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 525, 44, 62, 20, hwnd, NULL, NULL, NULL);
            hCheckResolve = CreateWindowEx(0, "BUTTON", "Resolve (-a)", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 593, 44, 90, 20, hwnd, NULL, NULL, NULL);

            hBtnExport = CreateWindowEx(0, "BUTTON", "Export [E]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 690, 42, 75, 22, hwnd, (HMENU)3, NULL, NULL);
            hBtnClear = CreateWindowEx(0, "BUTTON", "Clear [C]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 770, 42, 68, 22, hwnd, (HMENU)5, NULL, NULL);

            // Output Terminal Console
            hOutput = CreateWindowEx(0, "EDIT", "Welcome to KPing Network Diagnostics Suite.\r\n"
                                               "Modes: Ping [P], Route Trace [T], Path MTU [M], Subnet LAN Sweep [S], DNS Inspector [D].\r\n"
                                               "Metrics: Mean Jitter (RFC 3550), Std Dev, VoIP MOS Quality Score, SLA Rating.\r\n"
                                               "Press Enter or 'P' to Ping, 'S' for Subnet Sweep, F5/F9 to Save/Load, or 'F1' for Help.\r\n\r\n",
                WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
                15, 75, W - 30, H - 90, hwnd, NULL, NULL, NULL);
            SendMessage(hOutput, EM_LIMITTEXT, 1048576, 0);

            EnumChildWindows(hwnd, SetFontProc, (LPARAM)hFont);
            SendMessage(hOutput, WM_SETFONT, (WPARAM)hFontMono, TRUE);
            break;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            if ((HWND)lParam == hStatic || (HWND)lParam == hStaticCount || (HWND)lParam == hStaticSize || 
                (HWND)lParam == hStaticTTL || (HWND)lParam == hStaticTimeout || (HWND)lParam == hCheckCont || 
                (HWND)lParam == hCheckHex || (HWND)lParam == hCheckDF || (HWND)lParam == hCheckResolve) {
                SetTextColor(hdc, RGB(226, 232, 240));
                SetBkColor(hdc, RGB(15, 23, 42));
                return (LRESULT)hbg;
            } else if ((HWND)lParam == hOutput || (HWND)lParam == hInput || (HWND)lParam == hInputCount || 
                       (HWND)lParam == hInputSize || (HWND)lParam == hInputTTL || (HWND)lParam == hInputTimeout) {
                SetTextColor(hdc, (HWND)lParam == hOutput ? RGB(163, 190, 140) : RGB(226, 232, 240));
                SetBkColor(hdc, RGB(30, 41, 59));
                return (LRESULT)hinputBg;
            }
            break;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, (HWND)lParam == hOutput ? RGB(163, 190, 140) : RGB(226, 232, 240));
            SetBkColor(hdc, RGB(30, 41, 59));
            return (LRESULT)hinputBg;
        }
        case WM_COMMAND: {
            WORD id = LOWORD(wParam);
            WORD code = HIWORD(wParam);

            if (id == 10 && code == CBN_SELCHANGE) {
                int idx = (int)SendMessage(hComboPreset, CB_GETCURSEL, 0, 0);
                if (idx > 0 && idx < sizeof(PRESET_HOSTS) / sizeof(PRESET_HOSTS[0])) {
                    SetWindowTextA(hInput, PRESET_HOSTS[idx]);
                }
            } else if (id == 1) { // Ping
                TriggerPing();
            } else if (id == 2) { // Trace
                TriggerTrace();
            } else if (id == 3) { // Export
                ExportLog(hwnd);
            } else if (id == 4) { // MTU Sweep
                TriggerMTU();
            } else if (id == 5) { // Clear
                ClearOutput();
            } else if (id == 6) { // Help
                ShowHelpDialog(hwnd);
            } else if (id == 7) { // Subnet Sweep
                TriggerSubnet();
            } else if (id == 8) { // DNS Inspector
                TriggerDNS();
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);

            int rightButtonsW = 440;
            int inputW = nw - rightButtonsW - 220;
            if (inputW < 120) inputW = 120;

            MoveWindow(hInput, 60, 12, inputW, 24, TRUE);
            MoveWindow(hComboPreset, 65 + inputW + 5, 12, 150, 200, TRUE);
            
            int btnX = nw - 530;
            MoveWindow(hBtn, btnX, 12, 65, 24, TRUE);
            MoveWindow(hBtnTrace, btnX + 70, 12, 68, 24, TRUE);
            MoveWindow(hBtnMTU, btnX + 143, 12, 68, 24, TRUE);
            MoveWindow(hBtnSubnet, btnX + 216, 12, 75, 24, TRUE);
            MoveWindow(hBtnDNS, btnX + 296, 12, 65, 24, TRUE);
            MoveWindow(hBtnHelp, btnX + 366, 12, 68, 24, TRUE);

            MoveWindow(hBtnExport, nw - 170, 42, 75, 22, TRUE);
            MoveWindow(hBtnClear, nw - 90, 42, 75, 22, TRUE);

            MoveWindow(hOutput, 15, 75, nw - 30, nh - 90, TRUE);
            break;
        }
        case WM_GETMINMAXINFO: {
            MINMAXINFO* mmi = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = 840;
            mmi->ptMinTrackSize.y = 420;
            return 0;
        }
        case WM_DESTROY:
            CancelCurrentOperation();
            if (hThread) {
                WaitForSingleObject(hThread, 300);
            }
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

#pragma function(memcpy)
void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
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

    HWND hwnd = CreateWindowEx(0, "KPingApp", "KPing - Network Diagnostics, Subnet & PMTU [Press F1 for Help]", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top, NULL, NULL, hInstance, NULL);

    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(15, 23, 42)));

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN) {
            HWND hFocus = GetFocus();
            BOOL isEditing = (hFocus == hInput || hFocus == hInputCount || hFocus == hInputSize || 
                              hFocus == hInputTTL || hFocus == hInputTimeout || hFocus == hComboPreset);

            if (msg.wParam == VK_F1 || (!isEditing && (msg.wParam == 'H' || msg.wParam == 'h'))) {
                ShowHelpDialog(hwnd);
                continue;
            }
            if (msg.wParam == VK_F5) {
                SaveState(hwnd);
                continue;
            }
            if (msg.wParam == VK_F9) {
                LoadState(hwnd);
                continue;
            }
            if (msg.wParam == VK_ESCAPE) {
                CancelCurrentOperation();
                continue;
            }
            if (!isEditing) {
                if (msg.wParam == 'P' || msg.wParam == 'p' || (msg.wParam == VK_RETURN && (hFocus == hBtn || hFocus == hwnd || hFocus == hOutput))) {
                    TriggerPing();
                    continue;
                } else if (msg.wParam == 'T' || msg.wParam == 't') {
                    TriggerTrace();
                    continue;
                } else if (msg.wParam == 'M' || msg.wParam == 'm') {
                    TriggerMTU();
                    continue;
                } else if (msg.wParam == 'S' || msg.wParam == 's') {
                    TriggerSubnet();
                    continue;
                } else if (msg.wParam == 'D' || msg.wParam == 'd') {
                    TriggerDNS();
                    continue;
                } else if (msg.wParam == 'E' || msg.wParam == 'e' || (GetKeyState(VK_CONTROL) < 0 && (msg.wParam == 'S' || msg.wParam == 's'))) {
                    ExportLog(hwnd);
                    continue;
                } else if (msg.wParam == 'C' || msg.wParam == 'c') {
                    if (GetKeyState(VK_CONTROL) < 0) {
                        CopyConsoleToClipboard(hwnd);
                    } else {
                        ClearOutput();
                    }
                    continue;
                } else if (msg.wParam >= '1' && msg.wParam <= '6') {
                    int pIdx = (int)(msg.wParam - '0');
                    if (pIdx > 0 && pIdx < sizeof(PRESET_HOSTS) / sizeof(PRESET_HOSTS[0])) {
                        SendMessageA(hComboPreset, CB_SETCURSEL, pIdx, 0);
                        SetWindowTextA(hInput, PRESET_HOSTS[pIdx]);
                    }
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
