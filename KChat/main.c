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
#define MAX_MSGS 250
#define MAX_ROOMS 16

typedef struct {
    char user[32];
    char text[384];
    char room[32];
    int pinned;
    int reactions_like;
    int reactions_love;
    int reactions_rocket;
    int is_poll;
    char poll_question[128];
    char poll_options[4][64];
    int poll_votes[4];
    int poll_opt_count;
} Message;

typedef struct {
    char room[32];
    char topic[128];
} RoomTopic;

SOCKET s = INVALID_SOCKET;
HWND hLog, hIp, hPort, hBtn, hInput, hSend, hClear, hSave;
HWND hRoomCombo, hPersonaCombo, hAskAI, hSearchInput, hPinBtn, hReactBtn, hExportJson, hImportBtn;
HWND hTopicLabel, hPollBtn, hVoteBtn, hStatsBtn, hHelpBtn;

char logBuf[65536] = "";
char currentRoom[32] = "#general";
char currentUsername[32] = "User";
char activePersona[32] = "Assistant";
char searchKeyword[64] = "";
int filterPinnedOnly = 0;

Message g_messages[MAX_MSGS];
int g_msgCount = 0;

RoomTopic g_roomTopics[MAX_ROOMS] = {
    {"#general", "General discussions & community hub"},
    {"#dev", "KiloApps architecture, C/ASM & Web dev"},
    {"#random", "Off-topic banters and fun"},
    {"#ai-lounge", "Prompt crafting & neural explorations"},
    {"", ""}
};
int g_topicCount = 4;

HFONT hUIFont = NULL;
HBRUSH hBgBrush = NULL;
WNDPROC oldInputProc = NULL;
static unsigned int g_rand_seed = 987654321;

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

int my_rand(int min, int max) {
    g_rand_seed = g_rand_seed * 1103515245 + 12345;
    unsigned int r = (g_rand_seed >> 16) & 0x7FFF;
    if (max <= min) return min;
    return min + (int)(r % (max - min + 1));
}

const char* GetRoomTopic(const char* room) {
    for (int i = 0; i < g_topicCount; i++) {
        if (my_strcmp(g_roomTopics[i].room, room) == 0) {
            return g_roomTopics[i].topic;
        }
    }
    return "No topic set for this room. Use /topic <text> to set one.";
}

void SetRoomTopic(const char* room, const char* topic) {
    for (int i = 0; i < g_topicCount; i++) {
        if (my_strcmp(g_roomTopics[i].room, room) == 0) {
            my_strncpy(g_roomTopics[i].topic, topic, sizeof(g_roomTopics[i].topic));
            return;
        }
    }
    if (g_topicCount < MAX_ROOMS) {
        my_strncpy(g_roomTopics[g_topicCount].room, room, sizeof(g_roomTopics[g_topicCount].room));
        my_strncpy(g_roomTopics[g_topicCount].topic, topic, sizeof(g_roomTopics[g_topicCount].topic));
        g_topicCount++;
    }
}

void UpdateTopicDisplay() {
    if (hTopicLabel) {
        char displayBuf[192];
        my_strcpy(displayBuf, "Topic: ");
        my_strcat(displayBuf, GetRoomTopic(currentRoom));
        SetWindowTextA(hTopicLabel, displayBuf);
    }
}

void RebuildLogView() {
    logBuf[0] = '\0';
    
    // Header for Pinned Message if any
    int pinnedCount = 0;
    for (int i = 0; i < g_msgCount; i++) {
        if (my_strcmp(g_messages[i].room, currentRoom) == 0 && g_messages[i].pinned) {
            if (pinnedCount == 0) {
                my_strcat(logBuf, "--- [PINNED IN ");
                my_strcat(logBuf, currentRoom);
                my_strcat(logBuf, "] ---\r\n");
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
            if (!my_stristr(m->user, searchKeyword) && !my_stristr(m->text, searchKeyword) && !my_stristr(m->poll_question, searchKeyword)) {
                continue;
            }
        }

        if (m->is_poll) {
            // Render interactive Poll card in log
            my_strcat(logBuf, "[");
            my_strcat(logBuf, m->room);
            my_strcat(logBuf, "] [POLL by ");
            my_strcat(logBuf, m->user);
            my_strcat(logBuf, "]: ");
            my_strcat(logBuf, m->poll_question);
            my_strcat(logBuf, "\r\n");

            int totalVotes = 0;
            for (int k = 0; k < m->poll_opt_count; k++) totalVotes += m->poll_votes[k];

            for (int k = 0; k < m->poll_opt_count; k++) {
                int pct = totalVotes > 0 ? (m->poll_votes[k] * 100 / totalVotes) : 0;
                char optLine[128];
                int barLen = pct / 10;
                char bar[16];
                for (int b = 0; b < 10; b++) bar[b] = (b < barLen) ? '#' : '.';
                bar[10] = '\0';

                wsprintfA(optLine, "    [%d] %s  --  %d votes (%d%%)  [%s]\r\n", k + 1, m->poll_options[k], m->poll_votes[k], pct, bar);
                my_strcat(logBuf, optLine);
            }
            char pollFooter[96];
            wsprintfA(pollFooter, "    (Total votes: %d | Type /vote <1-%d> to vote)\r\n", totalVotes, m->poll_opt_count);
            my_strcat(logBuf, pollFooter);
        } else {
            my_strcat(logBuf, "[");
            my_strcat(logBuf, m->room);
            my_strcat(logBuf, "] <");
            my_strcat(logBuf, m->user);
            my_strcat(logBuf, ">");
            if (m->pinned) my_strcat(logBuf, " [PIN]");
            my_strcat(logBuf, " ");
            my_strcat(logBuf, m->text);

            if (m->reactions_like > 0 || m->reactions_love > 0 || m->reactions_rocket > 0) {
                my_strcat(logBuf, " (Reactions:");
                if (m->reactions_like > 0) my_strcat(logBuf, " +1");
                if (m->reactions_love > 0) my_strcat(logBuf, " <3");
                if (m->reactions_rocket > 0) my_strcat(logBuf, " ^");
                my_strcat(logBuf, ")");
            }
            my_strcat(logBuf, "\r\n");
        }
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
    m->is_poll = 0;
    RebuildLogView();
}

void AddPollMessage(const char* user, const char* question, const char* options[], int optCount, const char* room) {
    if (g_msgCount >= MAX_MSGS) {
        for (int i = 0; i < MAX_MSGS - 1; i++) {
            g_messages[i] = g_messages[i + 1];
        }
        g_msgCount = MAX_MSGS - 1;
    }
    Message* m = &g_messages[g_msgCount++];
    my_memset(m, 0, sizeof(Message));
    my_strncpy(m->user, user, sizeof(m->user));
    my_strncpy(m->room, room, sizeof(m->room));
    my_strncpy(m->poll_question, question, sizeof(m->poll_question));
    my_strcpy(m->text, "[Poll: ");
    my_strcat(m->text, question);
    my_strcat(m->text, "]");
    m->is_poll = 1;
    m->poll_opt_count = optCount > 4 ? 4 : optCount;
    for (int i = 0; i < m->poll_opt_count; i++) {
        my_strncpy(m->poll_options[i], options[i], sizeof(m->poll_options[i]));
        m->poll_votes[i] = 0;
    }
    RebuildLogView();
}

int VoteOnLatestPoll(int optIndex) {
    for (int i = g_msgCount - 1; i >= 0; i--) {
        if (my_strcmp(g_messages[i].room, currentRoom) == 0 && g_messages[i].is_poll) {
            if (optIndex >= 1 && optIndex <= g_messages[i].poll_opt_count) {
                g_messages[i].poll_votes[optIndex - 1]++;
                RebuildLogView();
                return 1;
            }
            return -1;
        }
    }
    return 0;
}

void ShowRoomStats() {
    int roomMsgs = 0;
    int roomPolls = 0;
    int roomPinned = 0;
    int totalVotes = 0;

    for (int i = 0; i < g_msgCount; i++) {
        if (my_strcmp(g_messages[i].room, currentRoom) == 0) {
            roomMsgs++;
            if (g_messages[i].pinned) roomPinned++;
            if (g_messages[i].is_poll) {
                roomPolls++;
                for (int k = 0; k < g_messages[i].poll_opt_count; k++) {
                    totalVotes += g_messages[i].poll_votes[k];
                }
            }
        }
    }

    char statsReport[384];
    wsprintfA(statsReport, "[STATS for %s]: User: %s | Messages: %d (Total: %d) | Pinned: %d | Active Polls: %d (Votes: %d) | Persona: %s",
        currentRoom, currentUsername, roomMsgs, g_msgCount, roomPinned, roomPolls, totalVotes, activePersona);
    AddMessage("System", statsReport, currentRoom, 0);
}

void GenerateAIResponse(const char* prompt) {
    char reply[384];
    char userPersonaTag[64];
    my_strcpy(userPersonaTag, activePersona);
    my_strcat(userPersonaTag, " AI");

    if (my_strcmp(activePersona, "Cyberpunk") == 0) {
        my_strcpy(reply, "Data node ping received on grid. Packet decrypted: ");
        my_strcat(reply, prompt);
        my_strcat(reply, ". Cyber signal status: 100Gbps active.");
    } else if (my_strcmp(activePersona, "CodeBot") == 0) {
        my_strcpy(reply, "[CODEBOT]: // Processed query: ");
        my_strcat(reply, prompt);
        my_strcat(reply, " -> Status: 200 OK. Compiled with 0 errors.");
    } else if (my_strcmp(activePersona, "Sarcastic") == 0) {
        my_strcpy(reply, "Really? \"");
        my_strcat(reply, prompt);
        my_strcat(reply, "\"? Groundbreaking input. Pausing quantum computing to appreciate that.");
    } else if (my_strcmp(activePersona, "Cerberus") == 0) {
        my_strcpy(reply, "[CERBERUS]: Security Protocol 9 active. Query audited and cleared.");
    } else {
        my_strcpy(reply, "I am happy to assist you with \"");
        my_strcat(reply, prompt);
        my_strcat(reply, "\". Everything in ");
        my_strcat(reply, currentRoom);
        my_strcat(reply, " is operating smoothly!");
    }

    AddMessage(userPersonaTag, reply, currentRoom, 0);
}

void EscapeJsonString(const char* src, char* dst, int maxDst) {
    int j = 0;
    for (int i = 0; src[i] && j < maxDst - 6; i++) {
        if (src[i] == '"') { dst[j++] = '\\'; dst[j++] = '"'; }
        else if (src[i] == '\\') { dst[j++] = '\\'; dst[j++] = '\\'; }
        else if (src[i] == '\r') { dst[j++] = '\\'; dst[j++] = 'r'; }
        else if (src[i] == '\n') { dst[j++] = '\\'; dst[j++] = 'n'; }
        else if (src[i] == '\t') { dst[j++] = '\\'; dst[j++] = 't'; }
        else { dst[j++] = src[i]; }
    }
    dst[j] = '\0';
}

void ShowHelpDialog(HWND hwnd) {
    const char* helpText =
        "=== KChat Native Pro User Guide & Reference ===\r\n\r\n"
        "[KEYBOARD SHORTCUTS]\r\n"
        "  F1, H            : Open this comprehensive Help & Command guide\r\n"
        "  Ctrl+1 .. Ctrl+4 : Quick switch channel (#general, #dev, #random, #ai-lounge)\r\n"
        "  Ctrl+A           : Query active AI Persona with current input\r\n"
        "  Ctrl+P           : Pin / Unpin latest message in channel\r\n"
        "  Ctrl+S           : Save chat log to chat_log.txt\r\n"
        "  Ctrl+J           : Export structured JSON chat history\r\n"
        "  Enter            : Send message (when focused in input box)\r\n"
        "  Esc              : Clear input field or reset search filter\r\n\r\n"
        "[SLASH COMMANDS]\r\n"
        "  /poll <Q>? <Opt1> | <Opt2> | <Opt3> : Create interactive poll\r\n"
        "  /vote <1-4>                         : Vote on active room poll\r\n"
        "  /topic <new topic text>             : Set channel topic\r\n"
        "  /nick <username>                    : Change your display nickname\r\n"
        "  /join <#room>                       : Switch or join a custom channel\r\n"
        "  /roll [d6|d20|d100]                 : Roll dice with custom sides\r\n"
        "  /stats                              : Display channel analytics\r\n"
        "  /ai <prompt>                        : Direct query to active AI Persona\r\n"
        "  /me <action>                        : Send 3rd-person action notice\r\n"
        "  /shrug, /table                      : Quick fun ASCII emotes\r\n"
        "  /clear                              : Clear message view\r\n"
        "  /unpin                              : Unpin banner message\r\n\r\n"
        "[AI PERSONAS]\r\n"
        "  Assistant, Cyberpunk AI, CodeBot, Sarcastic Hacker, Cerberus Security\r\n\r\n"
        "[SERVER CONNECTIVITY]\r\n"
        "  Connect to TCP chat server at specified IP:Port.";
    MessageBoxA(hwnd, helpText, "KChat Pro - Help & Shortcuts", MB_OK | MB_ICONINFORMATION);
}

void SwitchToRoom(HWND hwnd, const char* room) {
    my_strncpy(currentRoom, room, sizeof(currentRoom));
    if (hRoomCombo) {
        for (int i = 0; i < 4; i++) {
            char itemText[32];
            SendMessageA(hRoomCombo, CB_GETLBTEXT, i, (LPARAM)itemText);
            if (my_strcmp(itemText, room) == 0) {
                SendMessageA(hRoomCombo, CB_SETCURSEL, i, 0);
                break;
            }
        }
    }
    UpdateTopicDisplay();
    RebuildLogView();
}

WNDPROC oldSearchProc = NULL;
LRESULT CALLBACK SearchSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == VK_ESCAPE) {
        SetWindowTextA(hwnd, "");
        searchKeyword[0] = '\0';
        RebuildLogView();
        return 0;
    }
    if (msg == WM_KEYDOWN && wParam == VK_F1) {
        HWND hParent = GetParent(hwnd);
        SendMessageA(hParent, WM_COMMAND, 112, 0);
        return 0;
    }
    return CallWindowProcA(oldSearchProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK InputSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == VK_RETURN) {
        HWND hParent = GetParent(hwnd);
        SendMessageA(hParent, WM_COMMAND, MAKEWPARAM(101, BN_CLICKED), (LPARAM)GetDlgItem(hParent, 101));
        return 0;
    }
    if (msg == WM_KEYDOWN && wParam == VK_ESCAPE) {
        SetWindowTextA(hwnd, "");
        return 0;
    }
    if (msg == WM_KEYDOWN && wParam == VK_F1) {
        HWND hParent = GetParent(hwnd);
        SendMessageA(hParent, WM_COMMAND, 112, 0);
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
            hIp = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "127.0.0.1", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|WS_TABSTOP, 32, 8, 75, 22, hwnd, 0, 0, 0);
            
            CreateWindowA("STATIC", "Port:", WS_CHILD|WS_VISIBLE, 112, 10, 30, 20, hwnd, 0, 0, 0);
            hPort = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "6667", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|WS_TABSTOP, 145, 8, 42, 22, hwnd, 0, 0, 0);
            
            hBtn = CreateWindowA("BUTTON", "Connect [C]", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 192, 8, 78, 23, hwnd, (HMENU)100, 0, 0);
            
            CreateWindowA("STATIC", "Room:", WS_CHILD|WS_VISIBLE, 276, 10, 40, 20, hwnd, 0, 0, 0);
            hRoomCombo = CreateWindowA("COMBOBOX", "", WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_VSCROLL|WS_TABSTOP, 318, 8, 88, 150, hwnd, (HMENU)104, 0, 0);
            SendMessageA(hRoomCombo, CB_ADDSTRING, 0, (LPARAM)"#general");
            SendMessageA(hRoomCombo, CB_ADDSTRING, 0, (LPARAM)"#dev");
            SendMessageA(hRoomCombo, CB_ADDSTRING, 0, (LPARAM)"#random");
            SendMessageA(hRoomCombo, CB_ADDSTRING, 0, (LPARAM)"#ai-lounge");
            SendMessageA(hRoomCombo, CB_SETCURSEL, 0, 0);

            CreateWindowA("STATIC", "AI:", WS_CHILD|WS_VISIBLE, 412, 10, 22, 20, hwnd, 0, 0, 0);
            hPersonaCombo = CreateWindowA("COMBOBOX", "", WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_VSCROLL|WS_TABSTOP, 436, 8, 105, 150, hwnd, (HMENU)105, 0, 0);
            SendMessageA(hPersonaCombo, CB_ADDSTRING, 0, (LPARAM)"Assistant");
            SendMessageA(hPersonaCombo, CB_ADDSTRING, 0, (LPARAM)"Cyberpunk");
            SendMessageA(hPersonaCombo, CB_ADDSTRING, 0, (LPARAM)"CodeBot");
            SendMessageA(hPersonaCombo, CB_ADDSTRING, 0, (LPARAM)"Sarcastic");
            SendMessageA(hPersonaCombo, CB_ADDSTRING, 0, (LPARAM)"Cerberus");
            SendMessageA(hPersonaCombo, CB_SETCURSEL, 0, 0);

            hPollBtn = CreateWindowA("BUTTON", "+ Poll [P]", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 547, 8, 65, 23, hwnd, (HMENU)113, 0, 0);
            hVoteBtn = CreateWindowA("BUTTON", "Vote [V]", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 616, 8, 58, 23, hwnd, (HMENU)114, 0, 0);
            hStatsBtn = CreateWindowA("BUTTON", "Stats [S]", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 678, 8, 58, 23, hwnd, (HMENU)115, 0, 0);
            hHelpBtn = CreateWindowA("BUTTON", "Help [F1]", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 740, 8, 65, 23, hwnd, (HMENU)112, 0, 0);

            // Row 2: Search, Pin, Reaction, Export/Import controls + Topic Header
            CreateWindowA("STATIC", "Search:", WS_CHILD|WS_VISIBLE, 10, 38, 45, 20, hwnd, 0, 0, 0);
            hSearchInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|WS_TABSTOP, 58, 36, 105, 22, hwnd, (HMENU)111, 0, 0);
            
            hPinBtn = CreateWindowA("BUTTON", "Pin [P]", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 168, 36, 50, 23, hwnd, (HMENU)107, 0, 0);
            hReactBtn = CreateWindowA("BUTTON", "React [R]", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 222, 36, 62, 23, hwnd, (HMENU)108, 0, 0);
            hExportJson = CreateWindowA("BUTTON", "JSON", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 288, 36, 46, 23, hwnd, (HMENU)109, 0, 0);
            hImportBtn = CreateWindowA("BUTTON", "Import", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 338, 36, 50, 23, hwnd, (HMENU)110, 0, 0);
            hClear = CreateWindowA("BUTTON", "Clear", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 392, 36, 46, 23, hwnd, (HMENU)102, 0, 0);

            hTopicLabel = CreateWindowA("STATIC", "Topic: General discussions & community hub", WS_CHILD|WS_VISIBLE|SS_LEFTNOWORDWRAP, 444, 38, 375, 20, hwnd, 0, 0, 0);

            // Row 3: Log area
            hLog = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY|WS_TABSTOP, 10, 65, 810, 510, hwnd, 0, 0, 0);
            
            // Row 4: Send & Input area
            hInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|WS_TABSTOP, 10, 585, 520, 24, hwnd, 0, 0, 0);
            hSend = CreateWindowA("BUTTON", "Send [Enter]", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 536, 585, 82, 24, hwnd, (HMENU)101, 0, 0);
            hAskAI = CreateWindowA("BUTTON", "Ask AI [Ctrl+A]", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 622, 585, 96, 24, hwnd, (HMENU)106, 0, 0);
            hSave = CreateWindowA("BUTTON", "Save TXT [Ctrl+S]", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 722, 585, 102, 24, hwnd, (HMENU)103, 0, 0);
            
            oldInputProc = (WNDPROC)SetWindowLongPtrA(hInput, GWLP_WNDPROC, (LONG_PTR)InputSubclassProc);
            oldSearchProc = (WNDPROC)SetWindowLongPtrA(hSearchInput, GWLP_WNDPROC, (LONG_PTR)SearchSubclassProc);

            int dpi = 96;
            HDC hdc = GetDC(NULL);
            if (hdc) {
                dpi = GetDeviceCaps(hdc, LOGPIXELSY);
                ReleaseDC(NULL, hdc);
            }
            int fontHeight = -MulDiv(12, dpi, 72);
            hUIFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            
            SendMessageA(hIp, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hPort, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hBtn, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hRoomCombo, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hPersonaCombo, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hPollBtn, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hVoteBtn, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hStatsBtn, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hHelpBtn, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hSearchInput, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hPinBtn, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hReactBtn, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hExportJson, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hImportBtn, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hTopicLabel, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hLog, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hInput, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hSend, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hAskAI, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hClear, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hSave, WM_SETFONT, (WPARAM)hUIFont, TRUE);

            AddMessage("System", "Welcome to KChat Native Pro Suite! Interactive Polls, Topics, AI Personas, and Slash Commands active.", "#general", 1);
            
            const char* sampleOpts[] = { "C/Win32 Native App", "HTML5/JS Web App", "Both with Full Parity" };
            AddPollMessage("KChatBot", "What is your favorite KiloApp architecture style?", sampleOpts, 3, "#general");

            AddMessage("System", "Press [F1] or 'H' for Help Guide, Slash Commands, and Shortcuts. Press [Ctrl+1..4] to switch channels.", "#general", 0);
            UpdateTopicDisplay();
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            HWND hwndStatic = (HWND)lParam;
            if (hwndStatic == hTopicLabel) {
                SetTextColor(hdcStatic, RGB(147, 197, 253)); // Soft blue
            } else {
                SetTextColor(hdcStatic, RGB(255, 255, 255));
            }
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
                    UpdateTopicDisplay();
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
                ShowHelpDialog(hwnd);
            } else if (wmId == 113) { // Poll Button
                const char* defaultOpts[] = { "Yes, absolutely!", "Needs more testing", "Not sure" };
                AddPollMessage(currentUsername, "Should we launch the next KiloApp release today?", defaultOpts, 3, currentRoom);
            } else if (wmId == 114) { // Vote Button (Vote Option 1 on latest poll)
                int res = VoteOnLatestPoll(1);
                if (res == 1) {
                    AddMessage("System", "Voted option [1] on the active poll.", currentRoom, 0);
                } else {
                    AddMessage("System", "No active poll found to vote on. Create one with /poll!", currentRoom, 0);
                }
            } else if (wmId == 115) { // Stats Button
                ShowRoomStats();
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
                    // Slash command handling
                    if (my_strcmp(buf, "/help") == 0) {
                        AddMessage("System", "Help: /poll <q>? <o1>|<o2>, /vote <num>, /topic <text>, /nick <name>, /roll [d20], /stats, /me <act>, /shrug, /ai <prompt>, /clear, /unpin", currentRoom, 0);
                        SetWindowTextA(hInput, "");
                        break;
                    }

                    if (my_strcmp(buf, "/clear") == 0) {
                        g_msgCount = 0;
                        RebuildLogView();
                        SetWindowTextA(hInput, "");
                        break;
                    }

                    if (my_strcmp(buf, "/unpin") == 0) {
                        for (int i = g_msgCount - 1; i >= 0; i--) {
                            if (my_strcmp(g_messages[i].room, currentRoom) == 0 && g_messages[i].pinned) {
                                g_messages[i].pinned = 0;
                                RebuildLogView();
                                break;
                            }
                        }
                        SetWindowTextA(hInput, "");
                        break;
                    }

                    if (buf[0] == '/' && buf[1] == 'n' && buf[2] == 'i' && buf[3] == 'c' && buf[4] == 'k' && buf[5] == ' ') {
                        my_strncpy(currentUsername, buf + 6, sizeof(currentUsername));
                        char nMsg[64];
                        wsprintfA(nMsg, "Nickname changed to %s", currentUsername);
                        AddMessage("System", nMsg, currentRoom, 0);
                        if (s != INVALID_SOCKET) {
                            send(s, buf, my_strlen(buf), 0);
                            send(s, "\n", 1, 0);
                        }
                        SetWindowTextA(hInput, "");
                        break;
                    }

                    if (buf[0] == '/' && buf[1] == 'j' && buf[2] == 'o' && buf[3] == 'i' && buf[4] == 'n' && buf[5] == ' ') {
                        my_strncpy(currentRoom, buf + 6, sizeof(currentRoom));
                        UpdateTopicDisplay();
                        AddMessage("System", "Switched room.", currentRoom, 0);
                        if (s != INVALID_SOCKET) {
                            send(s, buf, my_strlen(buf), 0);
                            send(s, "\n", 1, 0);
                        }
                        SetWindowTextA(hInput, "");
                        break;
                    }

                    if (buf[0] == '/' && buf[1] == 't' && buf[2] == 'o' && buf[3] == 'p' && buf[4] == 'i' && buf[5] == 'c' && buf[6] == ' ') {
                        SetRoomTopic(currentRoom, buf + 7);
                        UpdateTopicDisplay();
                        char topMsg[192];
                        my_strcpy(topMsg, "Room topic changed to: ");
                        my_strcat(topMsg, buf + 7);
                        AddMessage("System", topMsg, currentRoom, 0);
                        SetWindowTextA(hInput, "");
                        break;
                    }

                    if (buf[0] == '/' && buf[1] == 'p' && buf[2] == 'o' && buf[3] == 'l' && buf[4] == 'l' && buf[5] == ' ') {
                        // Parse /poll Question? Opt1 | Opt2 | Opt3
                        char* p = buf + 6;
                        char question[128] = "";
                        char optArr[4][64];
                        const char* optPtrs[4];
                        int optCount = 0;

                        char* qMark = p;
                        while (*qMark && *qMark != '?') qMark++;
                        if (*qMark == '?') {
                            int qLen = (int)(qMark - p + 1);
                            my_strncpy(question, p, qLen < 127 ? qLen + 1 : 127);
                            p = qMark + 1;
                            while (*p == ' ') p++;
                        } else {
                            my_strcpy(question, "Poll");
                        }

                        while (*p && optCount < 4) {
                            while (*p == ' ' || *p == '|') p++;
                            if (!*p) break;
                            char* nextPipe = p;
                            while (*nextPipe && *nextPipe != '|') nextPipe++;
                            int oLen = (int)(nextPipe - p);
                            while (oLen > 0 && p[oLen - 1] == ' ') oLen--;
                            my_strncpy(optArr[optCount], p, oLen < 63 ? oLen + 1 : 63);
                            optPtrs[optCount] = optArr[optCount];
                            optCount++;
                            p = nextPipe;
                        }

                        if (optCount >= 2) {
                            AddPollMessage(currentUsername, question, optPtrs, optCount, currentRoom);
                        } else {
                            const char* defOpts[] = { "Option A", "Option B" };
                            AddPollMessage(currentUsername, question[0] ? question : "Quick Poll", defOpts, 2, currentRoom);
                        }
                        SetWindowTextA(hInput, "");
                        break;
                    }

                    if (buf[0] == '/' && buf[1] == 'v' && buf[2] == 'o' && buf[3] == 't' && buf[4] == 'e' && buf[5] == ' ') {
                        int optNum = my_atoi(buf + 6);
                        int res = VoteOnLatestPoll(optNum);
                        if (res == 1) {
                            char vMsg[64];
                            wsprintfA(vMsg, "Voted option [%d] on active poll.", optNum);
                            AddMessage("System", vMsg, currentRoom, 0);
                        } else if (res == -1) {
                            AddMessage("System", "Invalid option number.", currentRoom, 0);
                        } else {
                            AddMessage("System", "No active poll found.", currentRoom, 0);
                        }
                        SetWindowTextA(hInput, "");
                        break;
                    }

                    if (buf[0] == '/' && buf[1] == 's' && buf[2] == 't' && buf[3] == 'a' && buf[4] == 't' && buf[5] == 's') {
                        ShowRoomStats();
                        SetWindowTextA(hInput, "");
                        break;
                    }

                    if (buf[0] == '/' && buf[1] == 'r' && buf[2] == 'o' && buf[3] == 'l' && buf[4] == 'l') {
                        int maxSides = 20;
                        if (buf[5] == ' ') {
                            if (buf[6] == 'd' || buf[6] == 'D') maxSides = my_atoi(buf + 7);
                            else maxSides = my_atoi(buf + 6);
                        }
                        if (maxSides <= 1) maxSides = 6;
                        int rollVal = my_rand(1, maxSides);
                        char rollMsg[128];
                        wsprintfA(rollMsg, "rolled a d%d: %d!", maxSides, rollVal);
                        AddMessage(currentUsername, rollMsg, currentRoom, 0);
                        SetWindowTextA(hInput, "");
                        break;
                    }

                    if (buf[0] == '/' && buf[1] == 's' && buf[2] == 'h' && buf[3] == 'r' && buf[4] == 'u' && buf[5] == 'g') {
                        AddMessage(currentUsername, "¯\\_(ツ)_/¯", currentRoom, 0);
                        SetWindowTextA(hInput, "");
                        break;
                    }

                    if (buf[0] == '/' && buf[1] == 't' && buf[2] == 'a' && buf[3] == 'b' && buf[4] == 'l' && buf[5] == 'e') {
                        AddMessage(currentUsername, "(╯°□°)╯︵ ┻━┻", currentRoom, 0);
                        SetWindowTextA(hInput, "");
                        break;
                    }

                    if (buf[0] == '/' && buf[1] == 'm' && buf[2] == 'e' && buf[3] == ' ') {
                        char actMsg[256];
                        my_strcpy(actMsg, "* ");
                        my_strcat(actMsg, currentUsername);
                        my_strcat(actMsg, " ");
                        my_strcat(actMsg, buf + 4);
                        my_strcat(actMsg, " *");
                        AddMessage("Action", actMsg, currentRoom, 0);
                        if (s != INVALID_SOCKET) {
                            send(s, buf, my_strlen(buf), 0);
                            send(s, "\n", 1, 0);
                        }
                        SetWindowTextA(hInput, "");
                        break;
                    }

                    if (buf[0] == '/' && buf[1] == 'a' && buf[2] == 'i' && buf[3] == ' ') {
                        AddMessage(currentUsername, buf, currentRoom, 0);
                        GenerateAIResponse(buf + 4);
                        SetWindowTextA(hInput, "");
                        break;
                    }

                    AddMessage(currentUsername, buf, currentRoom, 0);
                    if (s != INVALID_SOCKET) {
                        send(s, buf, my_strlen(buf), 0);
                        send(s, "\n", 1, 0);
                    }
                    SetWindowTextA(hInput, "");
                }
            } else if (wmId == 106) { // Ask AI
                char buf[384];
                GetWindowTextA(hInput, buf, sizeof(buf));
                if (!buf[0]) my_strcpy(buf, "What is your system status?");
                AddMessage(currentUsername, buf, currentRoom, 0);
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
                    char jsonHeader[] = "{\r\n  \"app\": \"KChat Native Pro\",\r\n  \"messages\": [\r\n";
                    WriteFile(hFile, jsonHeader, my_strlen(jsonHeader), &written, NULL);

                    for (int i = 0; i < g_msgCount; i++) {
                        char escRoom[64], escUser[64], escText[768];
                        EscapeJsonString(g_messages[i].room, escRoom, sizeof(escRoom));
                        EscapeJsonString(g_messages[i].user, escUser, sizeof(escUser));
                        EscapeJsonString(g_messages[i].text, escText, sizeof(escText));

                        char item[1024];
                        wsprintfA(item, "    {\"room\": \"%s\", \"user\": \"%s\", \"text\": \"%s\", \"pinned\": %d, \"is_poll\": %d}%s\r\n",
                            escRoom, escUser, escText, g_messages[i].pinned, g_messages[i].is_poll,
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
                            AddMessage("System", "Imported history file.", currentRoom, 0);
                            
                            // Parse lines
                            char* line = buffer;
                            while (*line) {
                                while (*line == '\r' || *line == '\n') line++;
                                if (!*line) break;
                                char* nextLine = line;
                                while (*nextLine && *nextLine != '\r' && *nextLine != '\n') nextLine++;
                                int lineLen = (int)(nextLine - line);
                                if (lineLen > 0 && lineLen < 350) {
                                    char cleanLine[384];
                                    my_strncpy(cleanLine, line, lineLen + 1);
                                    if (cleanLine[0] != '{' && cleanLine[0] != '}' && cleanLine[0] != '[') {
                                        AddMessage("Imported", cleanLine, currentRoom, 0);
                                    }
                                }
                                line = nextLine;
                            }
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
        case WM_KEYDOWN:
            if (wParam == VK_F1 || wParam == 'H' || wParam == 'h') {
                ShowHelpDialog(hwnd);
                return 0;
            }
            break;
        case WM_DESTROY:
            if (s != INVALID_SOCKET) closesocket(s);
            if (hUIFont) DeleteObject(hUIFont);
            if (hBgBrush) DeleteObject(hBgBrush);
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
    hBgBrush = CreateSolidBrush(RGB(15, 23, 42));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KChatClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = hBgBrush;

    RegisterClassA(&wc);
    RECT rect = { 0, 0, 850, 650 };
    AdjustWindowRect(&rect, (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN) & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);
    HWND hwnd = CreateWindowExA(0, "KChatClass", "KChat Native Pro - [F1] Help", (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN) & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_F1) {
                ShowHelpDialog(hwnd);
                continue;
            }
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                if (msg.wParam == '1') {
                    SwitchToRoom(hwnd, "#general");
                    continue;
                } else if (msg.wParam == '2') {
                    SwitchToRoom(hwnd, "#dev");
                    continue;
                } else if (msg.wParam == '3') {
                    SwitchToRoom(hwnd, "#random");
                    continue;
                } else if (msg.wParam == '4') {
                    SwitchToRoom(hwnd, "#ai-lounge");
                    continue;
                } else if (msg.wParam == 'A' || msg.wParam == 'a') {
                    SendMessageA(hwnd, WM_COMMAND, 106, 0);
                    continue;
                } else if (msg.wParam == 'P' || msg.wParam == 'p') {
                    SendMessageA(hwnd, WM_COMMAND, 107, 0);
                    continue;
                } else if (msg.wParam == 'S' || msg.wParam == 's') {
                    SendMessageA(hwnd, WM_COMMAND, 103, 0);
                    continue;
                } else if (msg.wParam == 'J' || msg.wParam == 'j') {
                    SendMessageA(hwnd, WM_COMMAND, 109, 0);
                    continue;
                }
            }
        }
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    if (hBgBrush) DeleteObject(hBgBrush);
    ExitProcess(0);
}

