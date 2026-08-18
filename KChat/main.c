#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <commdlg.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comdlg32.lib")

#define WM_SOCKET (WM_USER + 1)
#define MAX_MSGS 150

typedef struct {
    char user[32];
    char text[384];
    char room[32];
    int pinned;
    int reactions_like;
    int reactions_love;
    int reactions_rocket;
} Message;

SOCKET s = INVALID_SOCKET;
HWND hLog, hIp, hPort, hBtn, hInput, hSend, hClear, hSave;
HWND hRoomCombo, hPersonaCombo, hAskAI, hSearchInput, hPinBtn, hReactBtn, hExportJson, hImportBtn;

char logBuf[32768] = "";
char currentRoom[32] = "#general";
char activePersona[32] = "Assistant";
char searchKeyword[64] = "";
int filterPinnedOnly = 0;

Message g_messages[MAX_MSGS];
int g_msgCount = 0;

HFONT hUIFont = NULL;
WNDPROC oldInputProc = NULL;

#pragma optimize("", off)
void* my_memset(void* dest, int c, size_t count) {
    volatile char* bytes = (volatile char*)dest;
    while (count--) {
        *bytes++ = (char)c;
    }
    return dest;
}

void* memset(void* dest, int c, size_t count) {
    return my_memset(dest, c, count);
}
#pragma optimize("", on)

void my_strcpy(char* d, const char* s) { while (*s) *d++ = *s++; *d = 0; }
int my_strlen(const char* s) { int l = 0; while (s[l]) l++; return l; }
int my_strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}
void my_strncpy(char* d, const char* s, int n) {
    int i = 0;
    while (i < n - 1 && s[i]) { d[i] = s[i]; i++; }
    d[i] = 0;
}
void my_strcat(char* d, const char* s) {
    while (*d) d++;
    while (*s) *d++ = *s++;
    *d = 0;
}
int my_atoi(const char* str) {
    int v = 0;
    while (*str >= '0' && *str <= '9') { v = v * 10 + (*str - '0'); str++; }
    return v;
}

char to_lower(char c) {
    if (c >= 'A' && c <= 'Z') return c + 32;
    return c;
}

int my_stristr(const char* haystack, const char* needle) {
    if (!needle || !*needle) return 1;
    for (int i = 0; haystack[i]; i++) {
        int j = 0;
        while (haystack[i + j] && needle[j] && to_lower(haystack[i + j]) == to_lower(needle[j])) {
            j++;
        }
        if (!needle[j]) return 1;
    }
    return 0;
}

void RebuildLogView() {
    logBuf[0] = '\0';
    
    // Header for Pinned Message if any
    int pinnedCount = 0;
    for (int i = 0; i < g_msgCount; i++) {
        if (my_strcmp(g_messages[i].room, currentRoom) == 0 && g_messages[i].pinned) {
            if (pinnedCount == 0) {
                my_strcat(logBuf, "--- 📌 PINNED IN ");
                my_strcat(logBuf, currentRoom);
                my_strcat(logBuf, " ---\r\n");
            }
            my_strcat(logBuf, "  [");
            my_strcat(logBuf, g_messages[i].user);
            my_strcat(logBuf, "]: ");
            my_strcat(logBuf, g_messages[i].text);
            my_strcat(logBuf, "\r\n");
            pinnedCount++;
        }
    }
    if (pinnedCount > 0) {
        my_strcat(logBuf, "-------------------------------\r\n\r\n");
    }

    // Body messages
    for (int i = 0; i < g_msgCount; i++) {
        Message* m = &g_messages[i];
        if (my_strcmp(m->room, currentRoom) != 0) continue;
        if (filterPinnedOnly && !m->pinned) continue;
        if (searchKeyword[0] != '\0') {
            if (!my_stristr(m->user, searchKeyword) && !my_stristr(m->text, searchKeyword)) {
                continue;
            }
        }

        my_strcat(logBuf, "[");
        my_strcat(logBuf, m->room);
        my_strcat(logBuf, "] <");
        my_strcat(logBuf, m->user);
        my_strcat(logBuf, ">");
        if (m->pinned) my_strcat(logBuf, " 📌");
        my_strcat(logBuf, " ");
        my_strcat(logBuf, m->text);

        if (m->reactions_like > 0 || m->reactions_love > 0 || m->reactions_rocket > 0) {
            my_strcat(logBuf, " (Reactions:");
            if (m->reactions_like > 0) my_strcat(logBuf, " 👍");
            if (m->reactions_love > 0) my_strcat(logBuf, " ❤️");
            if (m->reactions_rocket > 0) my_strcat(logBuf, " 🚀");
            my_strcat(logBuf, ")");
        }
        my_strcat(logBuf, "\r\n");
    }

    SetWindowTextA(hLog, logBuf);
    SendMessageA(hLog, EM_LINESCROLL, 0, 9999);
}

void AddMessage(const char* user, const char* text, const char* room, int is_pinned) {
    if (g_msgCount >= MAX_MSGS) {
        for (int i = 0; i < MAX_MSGS - 1; i++) {
            g_messages[i] = g_messages[i + 1];
        }
        g_msgCount = MAX_MSGS - 1;
    }
    Message* m = &g_messages[g_msgCount++];
    my_memset(m, 0, sizeof(Message));
    my_strncpy(m->user, user, sizeof(m->user));
    my_strncpy(m->text, text, sizeof(m->text));
    my_strncpy(m->room, room, sizeof(m->room));
    m->pinned = is_pinned;
    RebuildLogView();
}

void GenerateAIResponse(const char* prompt) {
    char reply[384];
    char userPersonaTag[64];
    my_strcpy(userPersonaTag, activePersona);
    my_strcat(userPersonaTag, " AI");

    if (my_strcmp(activePersona, "Cyberpunk") == 0) {
        my_strcpy(reply, "⚡ Data node ping received on grid. Encrypted packet decrypted: ");
        my_strcat(reply, prompt);
        my_strcat(reply, ". Cyber signal status: 100Gbps active.");
    } else if (my_strcmp(activePersona, "CodeBot") == 0) {
        my_strcpy(reply, "💻 [CODEBOT]: // Processed query: ");
        my_strcat(reply, prompt);
        my_strcat(reply, " -> Status: 200 OK. Compiled with 0 errors.");
    } else if (my_strcmp(activePersona, "Sarcastic") == 0) {
        my_strcpy(reply, "😈 Really? \"");
        my_strcat(reply, prompt);
        my_strcat(reply, "\"? Groundbreaking input. Pausing quantum computing to appreciate that.");
    } else if (my_strcmp(activePersona, "Cerberus") == 0) {
        my_strcpy(reply, "🛡️ [CERBERUS]: Security Protocol 9 active. Query audited and cleared.");
    } else {
        my_strcpy(reply, "🤖 I am happy to assist you with \"");
        my_strcat(reply, prompt);
        my_strcat(reply, "\". Everything in ");
        my_strcat(reply, currentRoom);
        my_strcat(reply, " is operating smoothly!");
    }

    AddMessage(userPersonaTag, reply, currentRoom, 0);
}

LRESULT CALLBACK InputSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == VK_RETURN) {
        HWND hParent = GetParent(hwnd);
        SendMessageA(hParent, WM_COMMAND, MAKEWPARAM(101, BN_CLICKED), (LPARAM)GetDlgItem(hParent, 101));
        return 0;
    }
    return CallWindowProcA(oldInputProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            WSADATA wsa;
            WSAStartup(MAKEWORD(2,2), &wsa);

            // Row 1: Network & Room & Persona controls
            CreateWindowA("STATIC", "IP:", WS_CHILD|WS_VISIBLE, 10, 10, 20, 20, hwnd, 0, 0, 0);
            hIp = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "127.0.0.1", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 32, 8, 85, 22, hwnd, 0, 0, 0);
            
            CreateWindowA("STATIC", "Port:", WS_CHILD|WS_VISIBLE, 122, 10, 30, 20, hwnd, 0, 0, 0);
            hPort = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "6667", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 155, 8, 45, 22, hwnd, 0, 0, 0);
            
            hBtn = CreateWindowA("BUTTON", "Connect", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, 205, 8, 65, 23, hwnd, (HMENU)100, 0, 0);
            
            CreateWindowA("STATIC", "Room:", WS_CHILD|WS_VISIBLE, 275, 10, 40, 20, hwnd, 0, 0, 0);
            hRoomCombo = CreateWindowA("COMBOBOX", "", WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_VSCROLL, 315, 8, 95, 150, hwnd, (HMENU)104, 0, 0);
            SendMessageA(hRoomCombo, CB_ADDSTRING, 0, (LPARAM)"#general");
            SendMessageA(hRoomCombo, CB_ADDSTRING, 0, (LPARAM)"#dev");
            SendMessageA(hRoomCombo, CB_ADDSTRING, 0, (LPARAM)"#random");
            SendMessageA(hRoomCombo, CB_ADDSTRING, 0, (LPARAM)"#ai-lounge");
            SendMessageA(hRoomCombo, CB_SETCURSEL, 0, 0);

            CreateWindowA("STATIC", "AI:", WS_CHILD|WS_VISIBLE, 415, 10, 25, 20, hwnd, 0, 0, 0);
            hPersonaCombo = CreateWindowA("COMBOBOX", "", WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_VSCROLL, 440, 8, 145, 150, hwnd, (HMENU)105, 0, 0);
            SendMessageA(hPersonaCombo, CB_ADDSTRING, 0, (LPARAM)"Assistant");
            SendMessageA(hPersonaCombo, CB_ADDSTRING, 0, (LPARAM)"Cyberpunk");
            SendMessageA(hPersonaCombo, CB_ADDSTRING, 0, (LPARAM)"CodeBot");
            SendMessageA(hPersonaCombo, CB_ADDSTRING, 0, (LPARAM)"Sarcastic");
            SendMessageA(hPersonaCombo, CB_ADDSTRING, 0, (LPARAM)"Cerberus");
            SendMessageA(hPersonaCombo, CB_SETCURSEL, 0, 0);

            HWND hHelpBtn = CreateWindowA("BUTTON", "Help", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, 595, 8, 60, 23, hwnd, (HMENU)112, 0, 0);

            // Row 2: Search, Pin, Reaction, Export/Import controls
            CreateWindowA("STATIC", "Search:", WS_CHILD|WS_VISIBLE, 10, 38, 45, 20, hwnd, 0, 0, 0);
            hSearchInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 58, 36, 120, 22, hwnd, (HMENU)111, 0, 0);
            
            hPinBtn = CreateWindowA("BUTTON", "📌 Pin", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, 183, 36, 55, 23, hwnd, (HMENU)107, 0, 0);
            hReactBtn = CreateWindowA("BUTTON", "👍 React", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, 242, 36, 60, 23, hwnd, (HMENU)108, 0, 0);
            hExportJson = CreateWindowA("BUTTON", "JSON", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, 306, 36, 55, 23, hwnd, (HMENU)109, 0, 0);
            hImportBtn = CreateWindowA("BUTTON", "Import", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, 365, 36, 55, 23, hwnd, (HMENU)110, 0, 0);
            hClear = CreateWindowA("BUTTON", "Clear", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, 425, 36, 55, 23, hwnd, (HMENU)102, 0, 0);

            // Row 3: Log area
            hLog = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY, 10, 65, 810, 510, hwnd, 0, 0, 0);
            
            // Row 4: Send & Input area
            hInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 10, 585, 580, 24, hwnd, 0, 0, 0);
            hSend = CreateWindowA("BUTTON", "Send", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, 600, 585, 55, 24, hwnd, (HMENU)101, 0, 0);
            hAskAI = CreateWindowA("BUTTON", "Ask AI", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, 665, 585, 60, 24, hwnd, (HMENU)106, 0, 0);
            hSave = CreateWindowA("BUTTON", "Save TXT", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, 735, 585, 85, 24, hwnd, (HMENU)103, 0, 0);
            
            oldInputProc = (WNDPROC)SetWindowLongPtrA(hInput, GWLP_WNDPROC, (LONG_PTR)InputSubclassProc);

            hUIFont = CreateFontA(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            SendMessageA(hIp, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hPort, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hBtn, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hRoomCombo, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hPersonaCombo, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hSearchInput, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hPinBtn, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hReactBtn, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hExportJson, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hImportBtn, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hLog, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hInput, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hSend, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hAskAI, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hClear, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hSave, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hHelpBtn, WM_SETFONT, (WPARAM)hUIFont, TRUE);

            AddMessage("System", "Welcome to KChat Native Pro! Connect to server or use offline AI Personas.", "#general", 1);
            AddMessage("System", "Commands: /nick <name>, /join <#room>, /ai <prompt>. Click Help or type /help.", "#general", 0);
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(255, 255, 255));
            SetBkMode(hdcStatic, TRANSPARENT);
            return (LRESULT)GetStockObject(NULL_BRUSH);
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (wmId == 104 && wmEvent == CBN_SELCHANGE) { // Room Combobox selection
                int idx = (int)SendMessageA(hRoomCombo, CB_GETCURSEL, 0, 0);
                if (idx != CB_ERR) {
                    SendMessageA(hRoomCombo, CB_GETLBTEXT, idx, (LPARAM)currentRoom);
                    RebuildLogView();
                }
            } else if (wmId == 105 && wmEvent == CBN_SELCHANGE) { // Persona Combobox selection
                int idx = (int)SendMessageA(hPersonaCombo, CB_GETCURSEL, 0, 0);
                if (idx != CB_ERR) {
                    SendMessageA(hPersonaCombo, CB_GETLBTEXT, idx, (LPARAM)activePersona);
                }
            } else if (wmId == 111 && wmEvent == EN_CHANGE) { // Search Edit Box
                GetWindowTextA(hSearchInput, searchKeyword, sizeof(searchKeyword));
                RebuildLogView();
            } else if (wmId == 112) { // Help
                AddMessage("System", "Help: /nick <name>, /join <#room>, /ai <prompt>. Use UI buttons for Connect, Pin, Export.", currentRoom, 0);
            } else if (wmId == 100) { // Connect
                if (s != INVALID_SOCKET) {
                    closesocket(s);
                    s = INVALID_SOCKET;
                    SetWindowTextA(hBtn, "Connect");
                    AddMessage("System", "Disconnected.", currentRoom, 0);
                    break;
                }
                
                char ip[64], portStr[16];
                GetWindowTextA(hIp, ip, 64);
                GetWindowTextA(hPort, portStr, 16);
                int port = my_atoi(portStr);
                
                s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (s == INVALID_SOCKET) {
                    AddMessage("System", "Failed to create socket.", currentRoom, 0);
                    break;
                }

                struct hostent *he = gethostbyname(ip);
                if (!he) {
                    AddMessage("System", "Invalid IP/Host.", currentRoom, 0);
                    closesocket(s);
                    s = INVALID_SOCKET;
                    SetWindowTextA(hBtn, "Connect");
                    break;
                }
                
                struct sockaddr_in addr;
                addr.sin_family = AF_INET;
                addr.sin_addr = *((struct in_addr *)he->h_addr);
                addr.sin_port = htons(port);
                
                WSAAsyncSelect(s, hwnd, WM_SOCKET, FD_CONNECT | FD_READ | FD_CLOSE);
                if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
                    if (WSAGetLastError() != WSAEWOULDBLOCK) {
                        AddMessage("System", "Connection failed.", currentRoom, 0);
                        closesocket(s);
                        s = INVALID_SOCKET;
                        SetWindowTextA(hBtn, "Connect");
                        break;
                    }
                }
                AddMessage("System", "Connecting...", currentRoom, 0);
                EnableWindow(hBtn, FALSE);
            } else if (wmId == 101) { // Send
                char buf[384];
                GetWindowTextA(hInput, buf, sizeof(buf));
                if (buf[0]) {
                    if (buf[0] == '/' && buf[1] == 'j' && buf[2] == 'o' && buf[3] == 'i' && buf[4] == 'n' && buf[5] == ' ') {
                        my_strcpy(currentRoom, buf + 6);
                        AddMessage("System", "Switched room.", currentRoom, 0);
                        SetWindowTextA(hInput, "");
                        break;
                    }

                    if (buf[0] == '/' && buf[1] == 'h' && buf[2] == 'e' && buf[3] == 'l' && buf[4] == 'p') {
                        AddMessage("System", "Help: /nick <name>, /join <#room>, /ai <prompt>. Use UI buttons for Connect, Pin, Export.", currentRoom, 0);
                        SetWindowTextA(hInput, "");
                        break;
                    }

                    if (buf[0] == '/' && buf[1] == 'a' && buf[2] == 'i' && buf[3] == ' ') {
                        AddMessage("User", buf, currentRoom, 0);
                        GenerateAIResponse(buf + 4);
                        SetWindowTextA(hInput, "");
                        break;
                    }

                    AddMessage("User", buf, currentRoom, 0);
                    if (s != INVALID_SOCKET) {
                        send(s, buf, my_strlen(buf), 0);
                    }
                    SetWindowTextA(hInput, "");
                }
            } else if (wmId == 106) { // Ask AI
                char buf[384];
                GetWindowTextA(hInput, buf, sizeof(buf));
                if (!buf[0]) my_strcpy(buf, "What is your system status?");
                AddMessage("User", buf, currentRoom, 0);
                GenerateAIResponse(buf);
                SetWindowTextA(hInput, "");
            } else if (wmId == 107) { // Pin Last
                if (g_msgCount > 0) {
                    g_messages[g_msgCount - 1].pinned = !g_messages[g_msgCount - 1].pinned;
                    RebuildLogView();
                }
            } else if (wmId == 108) { // React 👍
                if (g_msgCount > 0) {
                    g_messages[g_msgCount - 1].reactions_like++;
                    RebuildLogView();
                }
            } else if (wmId == 109) { // Export JSON
                HANDLE hFile = CreateFileA("kchat_history.json", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    DWORD written;
                    char jsonHeader[] = "{\r\n  \"app\": \"KChat Native\",\r\n  \"messages\": [\r\n";
                    WriteFile(hFile, jsonHeader, my_strlen(jsonHeader), &written, NULL);

                    for (int i = 0; i < g_msgCount; i++) {
                        char item[512];
                        wsprintfA(item, "    {\"room\": \"%s\", \"user\": \"%s\", \"text\": \"%s\", \"pinned\": %d}%s\r\n",
                            g_messages[i].room, g_messages[i].user, g_messages[i].text, g_messages[i].pinned,
                            (i == g_msgCount - 1) ? "" : ",");
                        WriteFile(hFile, item, my_strlen(item), &written, NULL);
                    }

                    char jsonFooter[] = "  ]\r\n}\r\n";
                    WriteFile(hFile, jsonFooter, my_strlen(jsonFooter), &written, NULL);
                    CloseHandle(hFile);
                    AddMessage("System", "Exported history to kchat_history.json", currentRoom, 0);
                }
            } else if (wmId == 110) { // Import File
                OPENFILENAMEA ofn;
                char szFile[260];
                my_memset(szFile, 0, sizeof(szFile));
                my_memset(&ofn, 0, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = "JSON Files (*.json)\0*.json\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                if (GetOpenFileNameA(&ofn)) {
                    HANDLE hFile = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hFile != INVALID_HANDLE_VALUE) {
                        char buffer[4096];
                        DWORD readBytes;
                        if (ReadFile(hFile, buffer, sizeof(buffer) - 1, &readBytes, NULL)) {
                            buffer[readBytes] = 0;
                            AddMessage("Imported", "History loaded from file.", currentRoom, 0);
                            AddMessage("ImportedData", buffer, currentRoom, 0);
                        }
                        CloseHandle(hFile);
                    }
                }
            } else if (wmId == 102) { // Clear
                g_msgCount = 0;
                RebuildLogView();
            } else if (wmId == 103) { // Save TXT
                HANDLE hFile = CreateFileA("chat_log.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    DWORD written;
                    WriteFile(hFile, logBuf, my_strlen(logBuf), &written, NULL);
                    CloseHandle(hFile);
                    AddMessage("System", "Saved chat to chat_log.txt", currentRoom, 0);
                }
            }
            break;
        }
        case WM_SOCKET: {
            if (WSAGETSELECTERROR(lParam)) {
                AddMessage("System", "Socket error/disconnected.", currentRoom, 0);
                if (s != INVALID_SOCKET) closesocket(s);
                s = INVALID_SOCKET;
                EnableWindow(hBtn, TRUE);
                SetWindowTextA(hBtn, "Connect");
                break;
            }
            if (WSAGETSELECTEVENT(lParam) == FD_CONNECT) {
                AddMessage("System", "Connected!", currentRoom, 0);
                EnableWindow(hBtn, TRUE);
                SetWindowTextA(hBtn, "Disconnect");
            } else if (WSAGETSELECTEVENT(lParam) == FD_READ) {
                char buf[385];
                int ret = recv(s, buf, 384, 0);
                if (ret > 0) {
                    buf[ret] = 0;
                    while(ret > 0 && (buf[ret-1] == '\r' || buf[ret-1] == '\n')) {
                        buf[ret-1] = 0;
                        ret--;
                    }
                    AddMessage("Server", buf, currentRoom, 0);
                }
            } else if (WSAGETSELECTEVENT(lParam) == FD_CLOSE) {
                AddMessage("System", "Disconnected by server.", currentRoom, 0);
                closesocket(s);
                s = INVALID_SOCKET;
                EnableWindow(hBtn, TRUE);
                SetWindowTextA(hBtn, "Connect");
            }
            break;
        }
        case WM_DESTROY:
            if (s != INVALID_SOCKET) closesocket(s);
            if (hUIFont) DeleteObject(hUIFont);
            WSACleanup();
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc;
    my_memset(&wc, 0, sizeof(wc));
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        typedef BOOL (WINAPI *SetProcessDPIAwareFunc)(void);
        SetProcessDPIAwareFunc setDpiAware = (SetProcessDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (setDpiAware) setDpiAware();
    }
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KChatClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(15, 23, 42));

    RegisterClassA(&wc);
    RECT rect = { 0, 0, 850, 650 };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);
    HWND hwnd = CreateWindowExA(0, "KChatClass", "KChat Native Pro", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    DeleteObject(wc.hbrBackground);
    ExitProcess(0);
}
