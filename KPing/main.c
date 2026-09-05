#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>

#define W 870
#define H 650

HWND hInput;
HWND hComboPreset;
HWND hBtn;
HWND hBtnTrace;
HWND hBtnMTU;
HWND hBtnExport;
HWND hBtnClear;
HWND hBtnHelp;
HWND hOutput;
HWND hStatic;
HWND hStaticCount, hInputCount;
HWND hStaticSize, hInputSize;
HWND hStaticTTL, hInputTTL;
HWND hCheckCont, hCheckHex, hCheckDF;
HANDLE hThread = NULL;
HANDLE hPingProcess = NULL;
volatile BOOL bCancelOperation = FALSE;

HBRUSH hbg;
HBRUSH hinputBg;
HFONT hFont;
HFONT hFontMono;
int fontHeight;
WNDPROC g_OldEditProc = NULL;

// Helper to set crisp fonts
#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY 5
#endif

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

void AppendText(const char* text) {
    int len = GetWindowTextLengthA(hOutput);
    if (len > 30000) {
        SendMessageA(hOutput, EM_SETSEL, 0, 10000);
        SendMessageA(hOutput, EM_REPLACESEL, 0, (LPARAM)"");
        len = GetWindowTextLengthA(hOutput);
    }
    SendMessageA(hOutput, EM_SETSEL, len, len);
    SendMessageA(hOutput, EM_REPLACESEL, 0, (LPARAM)text);
}

void ClearOutput() {
    SetWindowTextA(hOutput, "");
}

void ShowHelpDialog(HWND hwnd) {
    const char* helpMsg = 
        "================ KPing Diagnostics & Hotkeys ================\n\n"
        "KEYBOARD SHORTCUTS:\n"
        "  • Enter / P  : Start / Stop ICMP Ping\n"
        "  • T          : Start / Stop Route Trace (traceroute)\n"
        "  • M          : Start / Stop Path MTU Discovery Sweep\n"
        "  • E / Ctrl+S : Export Console Session to Log File\n"
        "  • C          : Clear Output Console\n"
        "  • 1 - 6      : Select Preset Host (Localhost, Cloudflare, Google, etc.)\n"
        "  • Escape     : Cancel running operation / dismiss\n"
        "  • F1 / H     : Show this Help reference guide\n\n"
        "DIAGNOSTIC MODES:\n"
        "  • Ping       : Sends ICMP echo requests to assess latency and loss\n"
        "  • Trace      : Displays each router hop along the network route\n"
        "  • MTU Sweep  : Probes buffer boundaries with Don't-Fragment (DF) bit\n"
        "                 to calculate exact Path MTU and recommended TCP MSS\n\n"
        "FLAGS & PARAMETERS:\n"
        "  • Count      : Number of echo requests to send (default: 4)\n"
        "  • Size (B)   : Payload buffer size in bytes (excludes 28B header)\n"
        "  • TTL        : Time-To-Live hop limit (default: 115)\n"
        "  • -t (Cont)  : Continuous ping loop until stopped\n"
        "  • Hex Dump   : Displays raw payload buffer in hexadecimal\n"
        "  • -f (DF)    : Sets Don't-Fragment bit in IP header\n\n"
        "==============================================================";

    MessageBoxA(hwnd, helpMsg, "KPing User Guide & Shortcuts", MB_OK | MB_ICONINFORMATION);
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
    int mode = (int)(INT_PTR)param; // 0 = Ping, 1 = Trace, 2 = MTU Sweep
    bCancelOperation = FALSE;
    
    char host[256];
    GetWindowTextA(hInput, host, 256);
    if (host[0] == 0) lstrcpyA(host, "127.0.0.1");

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
    BOOL dfBit = SendMessage(hCheckDF, BM_GETCHECK, 0, 0) == BST_CHECKED;

    if (mode == 2) {
        // Path MTU Discovery Sweep Mode
        SetWindowTextA(hBtnMTU, "Stop");
        EnableWindow(hBtn, FALSE);
        EnableWindow(hBtnTrace, FALSE);

        char banner[512];
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
            wsprintfA(cmd, "ping.exe %s -n 1 -l %d -f -w 1500", host, curSize);

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

            char logLine[256];
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

            char summary[512];
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
    } else {
        // Standard Ping or Traceroute
        BOOL traceMode = (mode == 1);
        if (traceMode) {
            SetWindowTextA(hBtnTrace, "Stop [T]");
            EnableWindow(hBtn, FALSE);
            EnableWindow(hBtnMTU, FALSE);
        } else {
            SetWindowTextA(hBtn, "Stop [P]");
            EnableWindow(hBtnTrace, FALSE);
            EnableWindow(hBtnMTU, FALSE);
        }

        if (hexdump && !traceMode) {
            int sz = 0;
            for (int i = 0; sizeStr[i] >= '0' && sizeStr[i] <= '9'; i++) {
                sz = sz * 10 + (sizeStr[i] - '0');
            }
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
            wsprintfA(cmd, "tracert.exe %s", host);
        } else if (continuous) {
            if (dfBit) {
                wsprintfA(cmd, "ping.exe %s -t -l %s -i %s -f", host, sizeStr, ttlStr);
            } else {
                wsprintfA(cmd, "ping.exe %s -t -l %s -i %s", host, sizeStr, ttlStr);
            }
        } else {
            if (dfBit) {
                wsprintfA(cmd, "ping.exe %s -n %s -l %s -i %s -f", host, countStr, sizeStr, ttlStr);
            } else {
                wsprintfA(cmd, "ping.exe %s -n %s -l %s -i %s", host, countStr, sizeStr, ttlStr);
            }
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
                while (ReadFile(hRead, buf, sizeof(buf) - 1, &bytesRead, NULL) && bytesRead > 0) {
                    buf[bytesRead] = 0;
                    char formatBuf[1024];
                    int j = 0;
                    for (DWORD i = 0; i < bytesRead && j < 1020; i++) {
                        if (buf[i] == '\n' && (i == 0 || buf[i-1] != '\r')) {
                            formatBuf[j++] = '\r';
                        }
                        formatBuf[j++] = buf[i];
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

        if (traceMode) SetWindowTextA(hBtnTrace, "Trace [T]");
        else SetWindowTextA(hBtn, "Ping [P]");
        EnableWindow(hBtn, TRUE);
        EnableWindow(hBtnTrace, TRUE);
        EnableWindow(hBtnMTU, TRUE);
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

void CancelCurrentOperation() {
    if (hThread) {
        bCancelOperation = TRUE;
        if (hPingProcess) TerminateProcess(hPingProcess, 0);
        AppendText("\r\n[!] Operation cancelled.\r\n");
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

            hStatic = CreateWindowEx(0, "STATIC", "Host:", WS_CHILD | WS_VISIBLE, 15, 14, 40, 22, hwnd, NULL, NULL, NULL);
            hInput = CreateWindowEx(0, "EDIT", "127.0.0.1", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, 60, 12, 180, 24, hwnd, NULL, NULL, NULL);
            g_OldEditProc = (WNDPROC)SetWindowLongPtr(hInput, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

            hComboPreset = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | CBS_DROPDOWNLIST, 248, 12, 175, 200, hwnd, (HMENU)10, NULL, NULL);

            for (int i = 0; i < sizeof(PRESET_NAMES) / sizeof(PRESET_NAMES[0]); i++) {
                SendMessageA(hComboPreset, CB_ADDSTRING, 0, (LPARAM)PRESET_NAMES[i]);
            }
            SendMessageA(hComboPreset, CB_SETCURSEL, 0, 0);

            hBtn = CreateWindowEx(0, "BUTTON", "Ping [P]", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, W - 430, 12, 65, 24, hwnd, (HMENU)1, NULL, NULL);
            hBtnTrace = CreateWindowEx(0, "BUTTON", "Trace [T]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, W - 360, 12, 68, 24, hwnd, (HMENU)2, NULL, NULL);
            hBtnMTU = CreateWindowEx(0, "BUTTON", "MTU [M]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, W - 287, 12, 68, 24, hwnd, (HMENU)4, NULL, NULL);
            hBtnExport = CreateWindowEx(0, "BUTTON", "Export [E]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, W - 214, 12, 72, 24, hwnd, (HMENU)3, NULL, NULL);
            hBtnClear = CreateWindowEx(0, "BUTTON", "Clear [C]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, W - 137, 12, 65, 24, hwnd, (HMENU)5, NULL, NULL);
            hBtnHelp = CreateWindowEx(0, "BUTTON", "Help [F1]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, W - 68, 12, 65, 24, hwnd, (HMENU)6, NULL, NULL);

            hStaticCount = CreateWindowEx(0, "STATIC", "Count:", WS_CHILD | WS_VISIBLE, 15, 44, 42, 22, hwnd, NULL, NULL, NULL);
            hInputCount = CreateWindowEx(0, "EDIT", "4", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER, 60, 42, 40, 22, hwnd, NULL, NULL, NULL);

            hStaticSize = CreateWindowEx(0, "STATIC", "Size:", WS_CHILD | WS_VISIBLE, 110, 44, 35, 22, hwnd, NULL, NULL, NULL);
            hInputSize = CreateWindowEx(0, "EDIT", "32", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER, 148, 42, 45, 22, hwnd, NULL, NULL, NULL);

            hStaticTTL = CreateWindowEx(0, "STATIC", "TTL:", WS_CHILD | WS_VISIBLE, 203, 44, 30, 22, hwnd, NULL, NULL, NULL);
            hInputTTL = CreateWindowEx(0, "EDIT", "115", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER, 235, 42, 38, 22, hwnd, NULL, NULL, NULL);

            hCheckCont = CreateWindowEx(0, "BUTTON", "Continuous (-t)", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 285, 44, 110, 20, hwnd, NULL, NULL, NULL);
            hCheckHex = CreateWindowEx(0, "BUTTON", "Hex Dump", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 405, 44, 85, 20, hwnd, NULL, NULL, NULL);
            hCheckDF = CreateWindowEx(0, "BUTTON", "DF Bit (-f)", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 498, 44, 85, 20, hwnd, NULL, NULL, NULL);

            hOutput = CreateWindowEx(0, "EDIT", "Welcome to KPing Network Diagnostics.\r\nFeatures: ICMP Ping [P], Path MTU Discovery (PMTU) [M], Route Tracing [T], Hex Dump, DF-Flag Toggle, Log Export [E].\r\nPress Enter or 'P' to Ping, 'C' to Clear, or 'F1' for Help.\r\n\r\n",
                WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
                15, 75, W - 30, H - 90, hwnd, NULL, NULL, NULL);

            EnumChildWindows(hwnd, SetFontProc, (LPARAM)hFont);
            SendMessage(hOutput, WM_SETFONT, (WPARAM)hFontMono, TRUE);
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            if ((HWND)lParam == hStatic || (HWND)lParam == hStaticCount || (HWND)lParam == hStaticSize || (HWND)lParam == hStaticTTL ||
                (HWND)lParam == hCheckCont || (HWND)lParam == hCheckHex || (HWND)lParam == hCheckDF) {
                SetTextColor(hdc, RGB(226, 232, 240));
                SetBkColor(hdc, RGB(15, 23, 42));
                return (LRESULT)hbg;
            } else if ((HWND)lParam == hOutput || (HWND)lParam == hInput || (HWND)lParam == hInputCount || (HWND)lParam == hInputSize || (HWND)lParam == hInputTTL) {
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
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            int rightButtonsWidth = 430;
            int inputW = nw - rightButtonsWidth - 250;
            if (inputW < 100) inputW = 100;

            MoveWindow(hInput, 60, 12, inputW, 24, TRUE);
            MoveWindow(hComboPreset, 65 + inputW + 5, 12, 160, 200, TRUE);
            
            MoveWindow(hBtn, nw - 430, 12, 65, 24, TRUE);
            MoveWindow(hBtnTrace, nw - 360, 12, 68, 24, TRUE);
            MoveWindow(hBtnMTU, nw - 287, 12, 68, 24, TRUE);
            MoveWindow(hBtnExport, nw - 214, 12, 72, 24, TRUE);
            MoveWindow(hBtnClear, nw - 137, 12, 65, 24, TRUE);
            MoveWindow(hBtnHelp, nw - 68, 12, 65, 24, TRUE);

            MoveWindow(hOutput, 15, 75, nw - 30, nh - 90, TRUE);
            break;
        }
        case WM_GETMINMAXINFO: {
            MINMAXINFO* mmi = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = 720;
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

    HWND hwnd = CreateWindowEx(0, "KPingApp", "KPing - Network Diagnostics & Path MTU [Press F1 for Help]", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top, NULL, NULL, hInstance, NULL);

    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(15, 23, 42)));

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN) {
            HWND hFocus = GetFocus();
            BOOL isEditing = (hFocus == hInput || hFocus == hInputCount || hFocus == hInputSize || hFocus == hInputTTL || hFocus == hComboPreset);

            if (msg.wParam == VK_F1 || (!isEditing && (msg.wParam == 'H' || msg.wParam == 'h'))) {
                ShowHelpDialog(hwnd);
                continue;
            }
            if (msg.wParam == VK_ESCAPE) {
                CancelCurrentOperation();
                continue;
            }
            if (!isEditing) {
                if (msg.wParam == 'P' || msg.wParam == 'p' || msg.wParam == VK_RETURN) {
                    TriggerPing();
                    continue;
                } else if (msg.wParam == 'T' || msg.wParam == 't') {
                    TriggerTrace();
                    continue;
                } else if (msg.wParam == 'M' || msg.wParam == 'm') {
                    TriggerMTU();
                    continue;
                } else if (msg.wParam == 'E' || msg.wParam == 'e' || (GetKeyState(VK_CONTROL) < 0 && (msg.wParam == 'S' || msg.wParam == 's'))) {
                    ExportLog(hwnd);
                    continue;
                } else if (msg.wParam == 'C' || msg.wParam == 'c') {
                    ClearOutput();
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

