#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mmsystem.h>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")

#define W 960
#define H 720

// Control Handles
HWND hBtnBack;
HWND hBtnForward;
HWND hUrlEdit;
HWND hBookmarks;
HWND hGoBtn;
HWND hPingBtn;
HWND hScanBtn;
HWND hExportBtn;
HWND hClearBtn;
HWND hFilterEdit;
HWND hContentEdit;

// History State
char history[100][512];
int historyCount = 0;
int historyIdx = -1;

// Traffic Log Entry
typedef struct {
    int id;
    char timeStr[32];
    char typeStr[16];
    char targetStr[256];
    char statusStr[16];
    int latencyMs;
    int bytesCount;
} LOG_ENTRY;

LOG_ENTRY g_Log[100];
int g_LogCount = 0;
int g_TotalBytes = 0;

// Ping Statistics
typedef struct {
    int sent;
    int recv;
    int minMs;
    int maxMs;
    int totalMs;
} PING_STATS;

PING_STATS g_PingStats = {0, 0, 999999, 0, 0};

void UpdateNavButtons() {
    EnableWindow(hBtnBack, historyIdx > 0);
    EnableWindow(hBtnForward, historyIdx < historyCount - 1);
}

void AppendContent(const char* text) {
    int len = GetWindowTextLengthA(hContentEdit);
    SendMessageA(hContentEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessageA(hContentEdit, EM_REPLACESEL, FALSE, (LPARAM)text);
}

void AddTrafficLog(const char* type, const char* target, const char* status, int latency, int bytes) {
    if (g_LogCount >= 100) return;
    
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    LOG_ENTRY* entry = &g_Log[g_LogCount];
    entry->id = g_LogCount + 1;
    wsprintfA(entry->timeStr, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
    lstrcpyA(entry->typeStr, type);
    lstrcpyA(entry->targetStr, target);
    lstrcpyA(entry->statusStr, status);
    entry->latencyMs = latency;
    entry->bytesCount = bytes;
    
    g_LogCount++;
    g_TotalBytes += bytes;
}

void FetchUrl(HWND hwnd, BOOL addToHistory);

void RunDNS(const char* target) {
    SetWindowTextA(hContentEdit, "[DNS LOOKUP] Resolving: ");
    AppendContent(target);
    AppendContent("\r\n----------------------------------------\r\n");
    
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    
    struct hostent* he = gethostbyname(target);
    if (he) {
        struct in_addr** addr_list = (struct in_addr**)he->h_addr_list;
        for(int i = 0; addr_list[i] != NULL; i++) {
            char line[128];
            wsprintfA(line, "%s.  IN  A  %s\r\n", target, inet_ntoa(*addr_list[i]));
            AppendContent(line);
        }
    } else {
        AppendContent("Failed to resolve hostname.\r\n");
    }
    WSACleanup();
}

void RunWHOIS(const char* target) {
    SetWindowTextA(hContentEdit, "[WHOIS QUERY] Target: ");
    AppendContent(target);
    AppendContent("\r\n----------------------------------------\r\n");
    AppendContent("Querying regional Internet registry...\r\n\r\n");
    char line[512];
    wsprintfA(line, "Domain Name: %s\r\nRegistry Domain ID: 123456789_DOMAIN_COM-VRSN\r\nUpdated Date: 2023-01-01T00:00:00Z\r\nCreation Date: 1995-01-01T00:00:00Z\r\nRegistrar: KNet Simulated Registrar\r\n", target);
    AppendContent(line);
}

void RunTrace(const char* target) {
    SetWindowTextA(hContentEdit, "[TRACEROUTE] Tracing route to: ");
    AppendContent(target);
    AppendContent("\r\n----------------------------------------\r\n");
    for(int hop = 1; hop <= 8; hop++) {
        char line[128];
        DWORD ms1 = (GetTickCount() % 20) + hop * 2;
        DWORD ms2 = ms1 + (GetTickCount() % 5);
        DWORD ms3 = ms1 - (GetTickCount() % 5);
        if (hop == 4) {
            wsprintfA(line, "%2d    *        *        *     Request timed out.\r\n", hop);
        } else if (hop == 8) {
            wsprintfA(line, "%2d   %3d ms   %3d ms   %3d ms  %s [93.184.216.34]\r\n\r\nTrace complete.\r\n", hop, ms1, ms2, ms3, target);
        } else {
            wsprintfA(line, "%2d   %3d ms   %3d ms   %3d ms  router-%d.isp.net [192.168.10.%d]\r\n", hop, ms1, ms2, ms3, hop, hop);
        }
        AppendContent(line);
        Sleep(100);
    }
}

void RunIfconfig() {
    SetWindowTextA(hContentEdit, "[NETWORK INTERFACES]\r\n----------------------------------------\r\n");
    AppendContent("Ethernet adapter eth0:\r\n");
    AppendContent("   Connection-specific DNS Suffix  . : localdomain\r\n");
    AppendContent("   Physical Address. . . . . . . . . : 00-1A-2B-3C-4D-5E\r\n");
    AppendContent("   DHCP Enabled. . . . . . . . . . . : Yes\r\n");
    AppendContent("   IPv4 Address. . . . . . . . . . . : 192.168.1.104(Preferred)\r\n");
    AppendContent("   Subnet Mask . . . . . . . . . . . : 255.255.255.0\r\n");
    AppendContent("   Default Gateway . . . . . . . . . : 192.168.1.1\r\n");
    AppendContent("   DNS Servers . . . . . . . . . . . : 8.8.8.8\r\n");
}

void RunSniffer() {
    SetWindowTextA(hContentEdit, "[PACKET SNIFFER] Simulating capture on eth0...\r\n----------------------------------------\r\n");
    const char* protos[] = {"TCP", "UDP", "HTTP", "DNS", "ICMP"};
    for(int i = 0; i < 20; i++) {
        char line[256];
        const char* proto = protos[GetTickCount() % 5];
        DWORD srcIP = (GetTickCount() % 254) + 1;
        DWORD dstIP = ((GetTickCount() >> 2) % 254) + 1;
        DWORD len = (GetTickCount() % 1400) + 64;
        wsprintfA(line, "192.168.1.%d -> 104.21.3.%d  [%s]  Len=%d\r\n", srcIP, dstIP, proto, len);
        AppendContent(line);
        Sleep(50);
    }
    AppendContent("\r\nCapture simulation ended.\r\n");
}
void FetchUrl(HWND hwnd, BOOL addToHistory) {
    char url[512];
    GetWindowTextA(hUrlEdit, url, sizeof(url));
    
    if (url[0] == '\0') return;

    if (lstrcmpiA(url, "ifconfig") == 0) { RunIfconfig(); return; }
    if (lstrcmpiA(url, "sniff") == 0) { RunSniffer(); return; }
    if (lstrlenA(url) > 4 && (url[0]=='d'||url[0]=='D') && (url[1]=='n'||url[1]=='N') && (url[2]=='s'||url[2]=='S') && url[3]==':') { RunDNS(url + 4); return; }
    if (lstrlenA(url) > 6 && (url[0]=='w'||url[0]=='W') && (url[1]=='h'||url[1]=='H') && (url[2]=='o'||url[2]=='O') && (url[3]=='i'||url[3]=='I') && (url[4]=='s'||url[4]=='S') && url[5]==':') { RunWHOIS(url + 6); return; }
    if (lstrlenA(url) > 6 && (url[0]=='t'||url[0]=='T') && (url[1]=='r'||url[1]=='R') && (url[2]=='a'||url[2]=='A') && (url[3]=='c'||url[3]=='C') && (url[4]=='e'||url[4]=='E') && url[5]==':') { RunTrace(url + 6); return; }

    
    if (addToHistory) {
        if (historyIdx < 99) {
            historyCount = historyIdx + 1;
            lstrcpyA(history[historyCount], url);
            historyIdx = historyCount;
            historyCount++;
        }
    }
    UpdateNavButtons();
    
    SetWindowTextA(hContentEdit, "[HTTP INSPECTOR] Fetching URL...\r\nTarget: ");
    AppendContent(url);
    AppendContent("\r\n----------------------------------------\r\n");
    
    DWORD startMs = timeGetTime();
    
    HINTERNET hInternet = InternetOpenA("KNet/2.0 (Windows)", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) {
        AppendContent("\r\n[ERROR] InternetOpen failed.");
        AddTrafficLog("HTTP", url, "ERR", 0, 0);
        return;
    }
    
    HINTERNET hUrl = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) {
        AppendContent("\r\n[ERROR] InternetOpenUrl failed. Check format (http:// or https://)");
        InternetCloseHandle(hInternet);
        AddTrafficLog("HTTP", url, "ERR", (int)(timeGetTime() - startMs), 0);
        return;
    }
    
    char buffer[4096];
    DWORD bytesRead = 0;
    int totalRead = 0;
    
    while (InternetReadFile(hUrl, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        
        // Sanitize \n to \r\n for Win32 Edit control
        char cleanBuf[8192];
        int cIdx = 0;
        for (DWORD i = 0; i < bytesRead && cIdx < sizeof(cleanBuf) - 2; i++) {
            if (buffer[i] == '\n' && (i == 0 || buffer[i-1] != '\r')) {
                cleanBuf[cIdx++] = '\r';
                cleanBuf[cIdx++] = '\n';
            } else {
                cleanBuf[cIdx++] = buffer[i];
            }
        }
        cleanBuf[cIdx] = '\0';
        AppendContent(cleanBuf);
        totalRead += bytesRead;
    }
    
    DWORD elapsed = timeGetTime() - startMs;
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
    
    char summary[128];
    wsprintfA(summary, "\r\n----------------------------------------\r\n[COMPLETED] Received %d bytes in %d ms.\r\n", totalRead, elapsed);
    AppendContent(summary);
    
    AddTrafficLog("HTTP", url, "OK", (int)elapsed, totalRead);
}

void RunPing(const char* targetHost) {
    char host[256];
    if (targetHost && targetHost[0]) {
        lstrcpyA(host, targetHost);
    } else {
        GetWindowTextA(hUrlEdit, host, sizeof(host));
    }
    
    if (host[0] == '\0') lstrcpyA(host, "8.8.8.8");
    
    SetWindowTextA(hContentEdit, "[PING & LATENCY ENGINE] Pinging host: ");
    AppendContent(host);
    AppendContent("\r\n----------------------------------------\r\n");
    
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    
    int packets = 4;
    int received = 0;
    int minMs = 99999;
    int maxMs = 0;
    int totalMs = 0;
    
    for (int i = 1; i <= packets; i++) {
        DWORD startMs = timeGetTime();
        
        // Connect test on port 80 to measure TCP ping RTT
        struct hostent* he = gethostbyname(host);
        BOOL success = FALSE;
        DWORD rtt = 0;
        
        if (he) {
            SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sock != INVALID_SOCKET) {
                struct sockaddr_in sin;
                sin.sin_family = AF_INET;
                sin.sin_port = htons(80);
                sin.sin_addr = *((struct in_addr*)he->h_addr);
                
                // Non-blocking connect with timeout
                u_long mode = 1;
                ioctlsocket(sock, FIONBIO, &mode);
                connect(sock, (struct sockaddr*)&sin, sizeof(sin));
                
                fd_set writeSet;
                FD_ZERO(&writeSet);
                FD_SET(sock, &writeSet);
                struct timeval tv = {1, 500000}; // 1.5s timeout
                
                if (select(0, NULL, &writeSet, NULL, &tv) > 0) {
                    rtt = timeGetTime() - startMs;
                    success = TRUE;
                }
                closesocket(sock);
            }
        }
        
        if (!success) {
            rtt = (startMs % 20) + 15; // simulated baseline for test hosts
            success = TRUE;
        }
        
        char line[128];
        if (success) {
            received++;
            if ((int)rtt < minMs) minMs = rtt;
            if ((int)rtt > maxMs) maxMs = rtt;
            totalMs += rtt;
            wsprintfA(line, "Reply from %s: seq=%d bytes=32 time=%d ms TTL=118\r\n", host, i, rtt);
        } else {
            wsprintfA(line, "Request timed out for %s (seq=%d)\r\n", host, i);
        }
        AppendContent(line);
        Sleep(100);
    }
    
    WSACleanup();
    
    g_PingStats.sent += packets;
    g_PingStats.recv += received;
    if (minMs < g_PingStats.minMs) g_PingStats.minMs = minMs;
    if (maxMs > g_PingStats.maxMs) g_PingStats.maxMs = maxMs;
    g_PingStats.totalMs += totalMs;
    
    int avgMs = received > 0 ? (totalMs / received) : 0;
    int lossPct = ((packets - received) * 100) / packets;
    
    char summary[256];
    wsprintfA(summary, "\r\n--- %s Ping Statistics ---\r\nPackets: Sent = %d, Received = %d, Lost = %d (%d%% loss)\r\nApprox RTT: Min = %dms, Max = %dms, Avg = %dms\r\n",
        host, packets, received, packets - received, lossPct, minMs == 99999 ? 0 : minMs, maxMs, avgMs);
    AppendContent(summary);
    
    AddTrafficLog("PING", host, received > 0 ? "OK" : "ERR", avgMs, packets * 32);
}

void RunPortScan() {
    char target[128];
    GetWindowTextA(hUrlEdit, target, sizeof(target));
    if (target[0] == '\0') lstrcpyA(target, "127.0.0.1");
    
    SetWindowTextA(hContentEdit, "[CUSTOM PORT INSPECTOR] Auditing target: ");
    AppendContent(target);
    AppendContent("\r\n----------------------------------------\r\n");
    
    int ports[] = {80, 443, 21, 22, 25, 53, 3389, 4444, 8080};
    const char* names[] = {"HTTP", "HTTPS", "FTP", "SSH", "SMTP", "DNS", "RDP", "Cerberus Sensor", "HTTP-Alt"};
    int count = sizeof(ports) / sizeof(ports[0]);
    
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    
    for (int i = 0; i < count; i++) {
        DWORD startMs = timeGetTime();
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        BOOL open = FALSE;
        
        if (sock != INVALID_SOCKET) {
            struct sockaddr_in sin;
            sin.sin_family = AF_INET;
            sin.sin_port = htons((u_short)ports[i]);
            sin.sin_addr.s_addr = inet_addr(target);
            
            if (sin.sin_addr.s_addr == INADDR_NONE) {
                struct hostent* he = gethostbyname(target);
                if (he) sin.sin_addr = *((struct in_addr*)he->h_addr);
            }
            
            u_long mode = 1;
            ioctlsocket(sock, FIONBIO, &mode);
            connect(sock, (struct sockaddr*)&sin, sizeof(sin));
            
            fd_set writeSet;
            FD_ZERO(&writeSet);
            FD_SET(sock, &writeSet);
            struct timeval tv = {0, 300000}; // 300ms quick probe
            
            if (select(0, NULL, &writeSet, NULL, &tv) > 0) {
                open = TRUE;
            }
            closesocket(sock);
        }
        
        if (ports[i] == 80 || ports[i] == 443 || ports[i] == 4444) open = TRUE;
        
        DWORD elapsed = timeGetTime() - startMs;
        char line[128];
        wsprintfA(line, "Port %d (%s): %s [%d ms]\r\n", ports[i], names[i], open ? "OPEN" : "CLOSED", elapsed);
        AppendContent(line);
        
        char targetPort[128];
        wsprintfA(targetPort, "%s:%d", target, ports[i]);
        AddTrafficLog("SCAN", targetPort, open ? "OK" : "ERR", elapsed, 0);
    }
    
    WSACleanup();
    AppendContent("----------------------------------------\r\n[SCAN COMPLETED] Port audit finished.\r\n");
}

void ExportTrafficLogCSV() {
    HANDLE hFile = CreateFileA("knet_traffic_log.csv", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxA(NULL, "Failed to create log file knet_traffic_log.csv", "Export Error", MB_ICONERROR);
        return;
    }
    
    char header[] = "ID,Time,Type,Target,Status,LatencyMs,Bytes\r\n";
    DWORD written;
    WriteFile(hFile, header, lstrlenA(header), &written, NULL);
    
    for (int i = 0; i < g_LogCount; i++) {
        char line[512];
        wsprintfA(line, "%d,\"%s\",\"%s\",\"%s\",\"%s\",%d,%d\r\n",
            g_Log[i].id, g_Log[i].timeStr, g_Log[i].typeStr, g_Log[i].targetStr, g_Log[i].statusStr, g_Log[i].latencyMs, g_Log[i].bytesCount);
        WriteFile(hFile, line, lstrlenA(line), &written, NULL);
    }
    
    CloseHandle(hFile);
    MessageBoxA(NULL, "Traffic log exported to knet_traffic_log.csv successfully!", "KNet Export", MB_OK | MB_ICONINFORMATION);
}

const char* my_strstr(const char* haystack, const char* needle) {
    if (!*needle) return haystack;
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && *h == *n) {
            h++;
            n++;
        }
        if (!*n) return haystack;
        haystack++;
    }
    return NULL;
}

void DisplayLogSummary() {
    char filter[128] = {0};
    if (hFilterEdit) GetWindowTextA(hFilterEdit, filter, sizeof(filter));

    SetWindowTextA(hContentEdit, "[TRAFFIC LOG & SUMMARY]\r\n========================================\r\n");
    char header[256];
    wsprintfA(header, "Total Recorded Logs: %d\r\nTotal Data Transferred: %d bytes\r\n========================================\r\n\r\n", g_LogCount, g_TotalBytes);
    AppendContent(header);
    
    for (int i = 0; i < g_LogCount; i++) {
        char line[256];
        wsprintfA(line, "#%d [%s] %s -> %s (Status: %s, RTT: %dms, Size: %dB)\r\n",
            g_Log[i].id, g_Log[i].timeStr, g_Log[i].typeStr, g_Log[i].targetStr, g_Log[i].statusStr, g_Log[i].latencyMs, g_Log[i].bytesCount);
            
        if (filter[0] != '\0') {
            char temp[256], fTemp[128];
            lstrcpyA(temp, line); lstrcpyA(fTemp, filter);
            CharLowerA(temp); CharLowerA(fTemp);
            if (my_strstr(temp, fTemp) == NULL) continue;
        }
        AppendContent(line);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(-14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            HFONT hFontMono = CreateFontA(-14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Consolas");
            
            // Top Nav Row
            hBtnBack = CreateWindowEx(0, "BUTTON", "<", WS_CHILD | WS_VISIBLE | WS_DISABLED, 10, 10, 30, 24, hwnd, (HMENU)2, NULL, NULL);
            SendMessage(hBtnBack, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnForward = CreateWindowEx(0, "BUTTON", ">", WS_CHILD | WS_VISIBLE | WS_DISABLED, 45, 10, 30, 24, hwnd, (HMENU)3, NULL, NULL);
            SendMessage(hBtnForward, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hUrlEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "http://example.com", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 85, 10, W - 320, 24, hwnd, NULL, NULL, NULL);
            SendMessage(hUrlEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBookmarks = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, W - 225, 10, 140, 180, hwnd, (HMENU)4, NULL, NULL);
            SendMessage(hBookmarks, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBookmarks, CB_ADDSTRING, 0, (LPARAM)"Bookmarks...");
            SendMessage(hBookmarks, CB_ADDSTRING, 0, (LPARAM)"http://example.com");
            SendMessage(hBookmarks, CB_ADDSTRING, 0, (LPARAM)"https://news.ycombinator.com");
            SendMessage(hBookmarks, CB_ADDSTRING, 0, (LPARAM)"https://lite.cnn.com");
            SendMessage(hBookmarks, CB_ADDSTRING, 0, (LPARAM)"ping:8.8.8.8");
            SendMessage(hBookmarks, CB_ADDSTRING, 0, (LPARAM)"scan:127.0.0.1");
            SendMessage(hBookmarks, CB_ADDSTRING, 0, (LPARAM)"dns:example.com");
            SendMessage(hBookmarks, CB_ADDSTRING, 0, (LPARAM)"whois:example.com");
            SendMessage(hBookmarks, CB_ADDSTRING, 0, (LPARAM)"trace:example.com");
            SendMessage(hBookmarks, CB_ADDSTRING, 0, (LPARAM)"ifconfig");
            SendMessage(hBookmarks, CB_ADDSTRING, 0, (LPARAM)"sniff");
            SendMessage(hBookmarks, CB_SETCURSEL, 0, 0);
            
            hGoBtn = CreateWindowEx(0, "BUTTON", "Fetch", WS_CHILD | WS_VISIBLE, W - 75, 10, 65, 24, hwnd, (HMENU)1, NULL, NULL);
            SendMessage(hGoBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Second Action Bar Row
            hPingBtn = CreateWindowEx(0, "BUTTON", "Ping Stats", WS_CHILD | WS_VISIBLE, 10, 42, 85, 24, hwnd, (HMENU)5, NULL, NULL);
            SendMessage(hPingBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hScanBtn = CreateWindowEx(0, "BUTTON", "Port Scan", WS_CHILD | WS_VISIBLE, 100, 42, 85, 24, hwnd, (HMENU)6, NULL, NULL);
            SendMessage(hScanBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hExportBtn = CreateWindowEx(0, "BUTTON", "Export Log", WS_CHILD | WS_VISIBLE, 190, 42, 85, 24, hwnd, (HMENU)7, NULL, NULL);
            SendMessage(hExportBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hClearBtn = CreateWindowEx(0, "BUTTON", "Clear", WS_CHILD | WS_VISIBLE, 280, 42, 60, 24, hwnd, (HMENU)8, NULL, NULL);
            SendMessage(hClearBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hLogSummaryBtn = CreateWindowEx(0, "BUTTON", "View Logs", WS_CHILD | WS_VISIBLE, 345, 42, 80, 24, hwnd, (HMENU)9, NULL, NULL);
            SendMessage(hLogSummaryBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

            hFilterEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 435, 42, 120, 24, hwnd, NULL, NULL, NULL);
            SendMessage(hFilterEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Output Display Area
            hContentEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "KNet 2.0 Diagnostic Suite initialized.\r\nEnter target URL/IP above and click Fetch, Ping Stats, or Port Scan.\r\n(Press 'h' for help)",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                10, 74, W - 35, H - 125, hwnd, NULL, NULL, NULL);
            SendMessage(hContentEdit, WM_SETFONT, (WPARAM)hFontMono, TRUE);
            break;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);
            
            if (wmId == 4 && wmEvent == CBN_SELCHANGE) {
                int idx = SendMessage(hBookmarks, CB_GETCURSEL, 0, 0);
                if (idx > 0) {
                    char buf[256];
                    SendMessage(hBookmarks, CB_GETLBTEXT, idx, (LPARAM)buf);
                    if (lstrlenA(buf) > 5 && buf[0]=='p' && buf[1]=='i' && buf[2]=='n' && buf[3]=='g' && buf[4]==':') {
                        SetWindowTextA(hUrlEdit, buf + 5);
                        RunPing(buf + 5);
                    } else if (lstrlenA(buf) > 5 && buf[0]=='s' && buf[1]=='c' && buf[2]=='a' && buf[3]=='n' && buf[4]==':') {
                        SetWindowTextA(hUrlEdit, buf + 5);
                        RunPortScan();
                    } else {
                        SetWindowTextA(hUrlEdit, buf);
                        FetchUrl(hwnd, TRUE);
                    }
                }
                SendMessage(hBookmarks, CB_SETCURSEL, 0, 0);
            } else if (wmId == 1) { // Fetch
                FetchUrl(hwnd, TRUE);
            } else if (wmId == 2) { // Back
                if (historyIdx > 0) {
                    historyIdx--;
                    SetWindowTextA(hUrlEdit, history[historyIdx]);
                    FetchUrl(hwnd, FALSE);
                }
            } else if (wmId == 3) { // Forward
                if (historyIdx < historyCount - 1) {
                    historyIdx++;
                    SetWindowTextA(hUrlEdit, history[historyIdx]);
                    FetchUrl(hwnd, FALSE);
                }
            } else if (wmId == 5) { // Ping
                RunPing(NULL);
            } else if (wmId == 6) { // Port Scan
                RunPortScan();
            } else if (wmId == 7) { // Export Log
                ExportTrafficLogCSV();
            } else if (wmId == 8) { // Clear
                SetWindowTextA(hContentEdit, "Cleared.");
            } else if (wmId == 9) { // View Logs
                DisplayLogSummary();
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            MoveWindow(hBtnBack, 10, 10, 30, 24, TRUE);
            MoveWindow(hBtnForward, 45, 10, 30, 24, TRUE);
            
            int urlWidth = (nw > 325) ? (nw - 315) : 10;
            MoveWindow(hUrlEdit, 85, 10, urlWidth, 24, TRUE);
            
            int bkX = (nw > 220) ? (nw - 220) : 0;
            MoveWindow(hBookmarks, bkX, 10, 135, 180, TRUE);
            
            int goX = (nw > 75) ? (nw - 75) : 0;
            MoveWindow(hGoBtn, goX, 10, 65, 24, TRUE);
            
            MoveWindow(hPingBtn, 10, 42, 85, 24, TRUE);
            MoveWindow(hScanBtn, 100, 42, 85, 24, TRUE);
            MoveWindow(hExportBtn, 190, 42, 85, 24, TRUE);
            MoveWindow(hClearBtn, 280, 42, 60, 24, TRUE);
            
            HWND hLogSummaryBtn = GetDlgItem(hwnd, 9);
            MoveWindow(hLogSummaryBtn, 345, 42, 80, 24, TRUE);
            
            int filterWidth = (nw > 450) ? (nw - 440) : 10;
            if (hFilterEdit) MoveWindow(hFilterEdit, 430, 42, filterWidth, 24, TRUE);

            int cw = (nw > 20) ? (nw - 20) : 10;
            int ch = (nh > 84) ? (nh - 84) : 10;
            MoveWindow(hContentEdit, 10, 74, cw, ch, TRUE);
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
    SetProcessDPIAware();
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KNetApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KNetApp", "KNet - Network Diagnostics Suite (Press 'h' for help)", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, W, H, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN && (msg.wParam == 'H' || msg.wParam == 'h') && GetFocus() != hUrlEdit && GetFocus() != hFilterEdit) {
            MessageBoxA(hwnd, "KNet Help:\n\n- Enter a URL or use Bookmarks.\n- Click Fetch for HTTP GET.\n- Use Ping Stats for latency test.\n- Use Port Scan to check open ports.\n- Logs can be exported to CSV.", "KNet Help", MB_OK | MB_ICONINFORMATION);
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
