#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "advapi32.lib")

#define WM_SOCKET (WM_USER + 1)
#define MAX_CLIENTS 64
#define SAVE_MAGIC 0x5348434B // "KCHS"

#pragma optimize("", off)
void my_memset(void* dest, int c, size_t count) {
    volatile char* bytes = (volatile char*)dest;
    while (count--) {
        *bytes++ = (char)c;
    }
}
void my_memcpy(void* dest, const void* src, size_t count) {
    volatile char* d = (volatile char*)dest;
    const volatile char* s = (const volatile char*)src;
    while (count--) *d++ = *s++;
}
#pragma optimize("", on)

void my_strcpy(char* d, const char* s) { while (*s) *d++ = *s++; *d = 0; }
int my_strlen(const char* s) { int l = 0; while (s[l]) l++; return l; }
int my_strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) { a++; b++; }
    return *(unsigned char*)a - *(unsigned char*)b;
}
int my_strncmp(const char* a, const char* b, int n) {
    while (n && *a && (*a == *b)) { a++; b++; n--; }
    if (n == 0) return 0;
    return *(unsigned char*)a - *(unsigned char*)b;
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
void my_itoa(int val, char* buf) {
    char t[16]; int i = 0, j = 0;
    if (val == 0) { buf[0] = '0'; buf[1] = 0; return; }
    while (val > 0) { t[i++] = (val % 10) + '0'; val /= 10; }
    while (i > 0) buf[j++] = t[--i];
    buf[j] = 0;
}
int my_atoi(const char* s) {
    int v = 0;
    while (*s >= '0' && *s <= '9') { v = v * 10 + (*s - '0'); s++; }
    return v;
}

// SHA1 Implementation for RFC 6455 WebSocket Handshake
#define SHA1_ROTL(bits,word) (((word) << (bits)) | ((word) >> (32-(bits))))
typedef struct { unsigned int state[5]; unsigned int count[2]; unsigned char buffer[64]; } SHA1_CTX;
void SHA1Transform(unsigned int state[5], const unsigned char buffer[64]) {
    unsigned int a, b, c, d, e, t;
    unsigned int W[80];
    for (int i = 0; i < 16; i++) {
        W[i] = ((unsigned int)buffer[i * 4] << 24) | ((unsigned int)buffer[i * 4 + 1] << 16) | ((unsigned int)buffer[i * 4 + 2] << 8) | ((unsigned int)buffer[i * 4 + 3]);
    }
    for (int i = 16; i < 80; i++) W[i] = SHA1_ROTL(1, W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]);
    a = state[0]; b = state[1]; c = state[2]; d = state[3]; e = state[4];
    for (int i = 0; i < 80; i++) {
        if (i < 20) t = (b & c) | ((~b) & d) + 0x5A827999;
        else if (i < 40) t = (b ^ c ^ d) + 0x6ED9EBA1;
        else if (i < 60) t = (b & c) | (b & d) | (c & d) + 0x8F1BBCDC;
        else t = (b ^ c ^ d) + 0xCA62C1D6;
        t += SHA1_ROTL(5, a) + e + W[i];
        e = d; d = c; c = SHA1_ROTL(30, b); b = a; a = t;
    }
    state[0] += a; state[1] += b; state[2] += c; state[3] += d; state[4] += e;
}
void SHA1Init(SHA1_CTX* context) {
    context->count[0] = context->count[1] = 0;
    context->state[0] = 0x67452301; context->state[1] = 0xEFCDAB89; context->state[2] = 0x98BADCFE; context->state[3] = 0x10325476; context->state[4] = 0xC3D2E1F0;
}
void SHA1Update(SHA1_CTX* context, const unsigned char* data, unsigned int len) {
    unsigned int i, j;
    j = (context->count[0] >> 3) & 63;
    if ((context->count[0] += len << 3) < (len << 3)) context->count[1]++;
    context->count[1] += (len >> 29);
    i = 64 - j;
    if (len >= i) {
        my_memcpy(&context->buffer[j], data, i);
        SHA1Transform(context->state, context->buffer);
        for (j = i; j + 63 < len; j += 64) SHA1Transform(context->state, &data[j]);
        i = j; j = 0;
    } else i = 0;
    my_memcpy(&context->buffer[j], &data[i], len - i);
}
void SHA1Final(unsigned char digest[20], SHA1_CTX* context) {
    unsigned int i;
    unsigned char finalcount[8];
    for (i = 0; i < 8; i++) finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255);
    unsigned char c = 0200;
    SHA1Update(context, &c, 1);
    while ((context->count[0] & 504) != 448) { c = 0000; SHA1Update(context, &c, 1); }
    SHA1Update(context, finalcount, 8);
    for (i = 0; i < 20; i++) digest[i] = (unsigned char)((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
}

// Base64
const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#pragma optimize("", off)
void base64_encode(const unsigned char *in, int in_len, char *out) {
    int i = 0, j = 0, k = 0;
    unsigned char arr_3[3], arr_4[4];
    while (in_len--) {
        arr_3[i++] = *(in++);
        if (i == 3) {
            arr_4[0] = (arr_3[0] & 0xfc) >> 2;
            arr_4[1] = ((arr_3[0] & 0x03) << 4) + ((arr_3[1] & 0xf0) >> 4);
            arr_4[2] = ((arr_3[1] & 0x0f) << 2) + ((arr_3[2] & 0xc0) >> 6);
            arr_4[3] = arr_3[2] & 0x3f;
            for(i = 0; (i <4) ; i++) out[k++] = base64_chars[arr_4[i]];
            i = 0;
        }
    }
    if (i) {
        if (i <= 0) arr_3[0] = 0;
        if (i <= 1) arr_3[1] = 0;
        if (i <= 2) arr_3[2] = 0;
        arr_4[0] = (arr_3[0] & 0xfc) >> 2;
        arr_4[1] = ((arr_3[0] & 0x03) << 4) + ((arr_3[1] & 0xf0) >> 4);
        arr_4[2] = ((arr_3[1] & 0x0f) << 2) + ((arr_3[2] & 0xc0) >> 6);
        arr_4[3] = arr_3[2] & 0x3f;
        for (j = 0; (j < i + 1); j++) out[k++] = base64_chars[arr_4[j]];
        while((i++ < 3)) out[k++] = '=';
    }
    out[k] = '\0';
}
#pragma optimize("", on)

struct Client {
    SOCKET s;
    char room[32];
    char nick[32];
    int is_ws;
    int handshaked;
    char buf[4096];
    int buf_len;
};

// Persistence State Struct
typedef struct {
    unsigned int magic;
    int port;
    int maxClients;
    char motd[256];
    int peakClients;
    int packetsIn;
    int packetsOut;
} KChatSaveData;

struct Client clients[MAX_CLIENTS];
SOCKET listenSocket = INVALID_SOCKET;
HWND hLog, hPort, hBtn, hMotd, hBroadcast, hBtnBroadcast, hBtnSave, hBtnLoad, hBtnClear, hBtnHelp, hTelemetry;
HFONT hUIFont = NULL;
char logBuf[16384] = "";
char motd[256] = "Welcome to KChat!";

// Telemetry Counters
int g_activeClients = 0;
int g_peakClients = 0;
int g_packetsIn = 0;
int g_packetsOut = 0;

void UpdateTelemetryUI() {
    if (!hTelemetry) return;
    char buf[160];
    my_memset(buf, 0, sizeof(buf));
    my_strcpy(buf, "Clients: ");
    char num[16];
    my_itoa(g_activeClients, num);
    my_strcat(buf, num);
    my_strcat(buf, "/64 (Peak: ");
    my_itoa(g_peakClients, num);
    my_strcat(buf, num);
    my_strcat(buf, ") | Pkts: ");
    my_itoa(g_packetsIn, num);
    my_strcat(buf, num);
    my_strcat(buf, "/");
    my_itoa(g_packetsOut, num);
    my_strcat(buf, num);
    SetWindowTextA(hTelemetry, buf);
}

void Log(const char* msg) {
    int len = my_strlen(logBuf);
    int mlen = my_strlen(msg);
    if (mlen > 4000) mlen = 4000;
    if (len + mlen + 5 > (int)sizeof(logBuf)) {
        my_strcpy(logBuf, logBuf + (len / 2));
        len = my_strlen(logBuf);
    }
    my_strncpy(logBuf + len, msg, mlen + 1);
    len = my_strlen(logBuf);
    my_strcpy(logBuf + len, "\r\n");
    SetWindowTextA(hLog, logBuf);
    SendMessageA(hLog, EM_LINESCROLL, 0, 9999);
}

void SendRaw(SOCKET s, const char* msg, int is_ws) {
    if (is_ws) {
        int len = my_strlen(msg);
        unsigned char head[10];
        head[0] = 0x81; // FIN + Text
        int hlen = 2;
        if (len <= 125) {
            head[1] = len;
        } else if (len <= 65535) {
            head[1] = 126;
            head[2] = (len >> 8) & 255;
            head[3] = len & 255;
            hlen = 4;
        }
        send(s, (const char*)head, hlen, 0);
        send(s, msg, len, 0);
    } else {
        send(s, msg, my_strlen(msg), 0);
        send(s, "\n", 1, 0);
    }
    g_packetsOut++;
    UpdateTelemetryUI();
}

void Broadcast(const char* room, const char* msg) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].s != INVALID_SOCKET && my_strcmp(clients[i].room, room) == 0 && (clients[i].handshaked || !clients[i].is_ws)) {
            SendRaw(clients[i].s, msg, clients[i].is_ws);
        }
    }
}

void BroadcastAll(const char* msg) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].s != INVALID_SOCKET && (clients[i].handshaked || !clients[i].is_ws)) {
            SendRaw(clients[i].s, msg, clients[i].is_ws);
        }
    }
}

void HandleMessage(int cidx, char* msg) {
    for (int i=0; msg[i]; i++) if (msg[i] == '\r' || msg[i] == '\n') msg[i] = 0;
    if (!msg[0]) return;

    if (my_strlen(msg) > 400) {
        msg[400] = '\0';
    }
    
    if (my_strncmp(msg, "/join ", 6) == 0) {
        my_strncpy(clients[cidx].room, msg + 6, 31);
        char out[128]; my_memset(out, 0, sizeof(out));
        my_strcpy(out, "System: Joined ");
        my_strncpy(out + 15, msg + 6, 100);
        SendRaw(clients[cidx].s, out, clients[cidx].is_ws);
    } else if (my_strncmp(msg, "/nick ", 6) == 0) {
        my_strncpy(clients[cidx].nick, msg + 6, 31);
        char out[128]; my_memset(out, 0, sizeof(out));
        my_strcpy(out, "System: Nickname changed to ");
        my_strncpy(out + my_strlen(out), msg + 6, 90);
        SendRaw(clients[cidx].s, out, clients[cidx].is_ws);
    } else if (my_strcmp(msg, "/list") == 0) {
        SendRaw(clients[cidx].s, "System: Active Rooms:", clients[cidx].is_ws);
        for (int i=0; i<MAX_CLIENTS; i++) {
            if (clients[i].s != INVALID_SOCKET) {
                SendRaw(clients[cidx].s, clients[i].room, clients[cidx].is_ws);
            }
        }
    } else if (my_strcmp(msg, "/names") == 0) {
        SendRaw(clients[cidx].s, "System: Users in room:", clients[cidx].is_ws);
        for (int i=0; i<MAX_CLIENTS; i++) {
            if (clients[i].s != INVALID_SOCKET && my_strcmp(clients[i].room, clients[cidx].room) == 0) {
                SendRaw(clients[cidx].s, clients[i].nick, clients[cidx].is_ws);
            }
        }
    } else if (my_strncmp(msg, "/me ", 4) == 0) {
        char out[512]; my_memset(out, 0, sizeof(out));
        my_strcpy(out, "* ");
        my_strncpy(out + my_strlen(out), clients[cidx].nick, 31);
        my_strcpy(out + my_strlen(out), " ");
        my_strncpy(out + my_strlen(out), msg + 4, 400);
        Broadcast(clients[cidx].room, out);
        Log(out);
    } else {
        char out[512]; my_memset(out, 0, sizeof(out));
        my_strcpy(out, "[");
        my_strncpy(out + 1, clients[cidx].nick, 31);
        my_strcpy(out + my_strlen(out), "] ");
        my_strncpy(out + my_strlen(out), msg, 400);
        Broadcast(clients[cidx].room, out);
        Log(out);
    }
}

void ProcessData(int cidx) {
    struct Client* c = &clients[cidx];
    if (!c->is_ws && !c->handshaked) {
        if (my_strncmp(c->buf, "GET ", 4) == 0) {
            c->is_ws = 1;
            c->handshaked = 0;
        } else {
            c->handshaked = 1; // Pure TCP
        }
    }

    if (c->is_ws && !c->handshaked) {
        // WebSocket Handshake
        char* end = 0;
        for (int i=0; i < c->buf_len - 3; i++) {
            if (c->buf[i]=='\r' && c->buf[i+1]=='\n' && c->buf[i+2]=='\r' && c->buf[i+3]=='\n') {
                end = &c->buf[i]; break;
            }
        }
        if (!end) return;
        
        char* key_start = 0;
        for (int i=0; i < c->buf_len - 20; i++) {
            if (my_strncmp(&c->buf[i], "Sec-WebSocket-Key: ", 19) == 0) {
                key_start = &c->buf[i+19]; break;
            }
        }
        
        if (key_start) {
            char key[256]; my_memset(key, 0, sizeof(key));
            int ki = 0;
            while (*key_start && *key_start != '\r' && *key_start != '\n' && ki < 200) key[ki++] = *key_start++;
            my_strcpy(key + ki, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
            
            SHA1_CTX ctx;
            unsigned char hash[20];
            SHA1Init(&ctx);
            SHA1Update(&ctx, (unsigned char*)key, my_strlen(key));
            SHA1Final(hash, &ctx);
            
            char b64[128]; my_memset(b64, 0, sizeof(b64));
            base64_encode(hash, 20, b64);
            
            char resp[512]; my_memset(resp, 0, sizeof(resp));
            my_strcpy(resp, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ");
            my_strcpy(resp + my_strlen(resp), b64);
            my_strcpy(resp + my_strlen(resp), "\r\n\r\n");
            
            send(c->s, resp, my_strlen(resp), 0);
            c->handshaked = 1;
            c->buf_len = 0;
            
            if (motd[0]) {
                char m[512]; my_memset(m, 0, sizeof(m));
                my_strcpy(m, "System MOTD: ");
                my_strncpy(m + my_strlen(m), motd, 450);
                SendRaw(c->s, m, 1);
            }
        }
        return;
    }

    if (c->is_ws && c->handshaked) {
        while (c->buf_len >= 2) {
            unsigned char b1 = c->buf[0];
            unsigned char b2 = c->buf[1];
            int opcode = b1 & 0x0F;
            int masked = (b2 & 0x80) != 0;
            int payload_len = b2 & 0x7F;
            int header_len = 2;
            
            if (payload_len == 126) { header_len += 2; }
            else if (payload_len == 127) { header_len += 8; }
            if (masked) header_len += 4;
            
            if (c->buf_len < header_len) break;
            
            if (payload_len == 126) payload_len = ((unsigned char)c->buf[2] << 8) | (unsigned char)c->buf[3];
            
            if (c->buf_len < header_len + payload_len) break;
            
            if (opcode == 8) {
                c->buf_len = 0;
                return; 
            }
            
            if (opcode == 1) { // Text frame
                static char payload[4096]; my_memset(payload, 0, sizeof(payload));
                unsigned char mask[4] = {0};
                if (masked) {
                    mask[0] = c->buf[header_len - 4];
                    mask[1] = c->buf[header_len - 3];
                    mask[2] = c->buf[header_len - 2];
                    mask[3] = c->buf[header_len - 1];
                }
                
                int len = payload_len > 4000 ? 4000 : payload_len;
                for (int i=0; i<len; i++) {
                    payload[i] = c->buf[header_len + i] ^ (masked ? mask[i % 4] : 0);
                }
                payload[len] = '\0';
                HandleMessage(cidx, payload);
            }
            
            int total = header_len + payload_len;
            c->buf_len -= total;
            if (c->buf_len > 0) my_memcpy(c->buf, c->buf + total, c->buf_len);
        }
    } else if (!c->is_ws && c->handshaked) {
        // Raw TCP
        static char msg[4096]; my_memset(msg, 0, sizeof(msg));
        int len = c->buf_len > 4000 ? 4000 : c->buf_len;
        my_memcpy(msg, c->buf, len);
        msg[len] = '\0';
        HandleMessage(cidx, msg);
        c->buf_len = 0;
    }
}

// QuickSave State to binary file
void SaveState() {
    KChatSaveData data;
    my_memset(&data, 0, sizeof(data));
    data.magic = SAVE_MAGIC;

    char portStr[16]; my_memset(portStr, 0, sizeof(portStr));
    GetWindowTextA(hPort, portStr, 15);
    data.port = my_atoi(portStr);
    if (data.port <= 0) data.port = 6667;

    data.maxClients = MAX_CLIENTS;
    GetWindowTextA(hMotd, data.motd, 255);
    data.peakClients = g_peakClients;
    data.packetsIn = g_packetsIn;
    data.packetsOut = g_packetsOut;

    HANDLE hFile = CreateFileA("kchatserver.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hFile, &data, sizeof(data), &written, NULL);
        CloseHandle(hFile);
        Log("[SAVE] Server state & telemetry saved to kchatserver.dat (F5).");
    } else {
        Log("[SAVE] Error: Failed to write kchatserver.dat.");
    }
}

// QuickLoad State from binary file
void LoadState() {
    HANDLE hFile = CreateFileA("kchatserver.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        KChatSaveData data;
        my_memset(&data, 0, sizeof(data));
        DWORD readBytes = 0;
        ReadFile(hFile, &data, sizeof(data), &readBytes, NULL);
        CloseHandle(hFile);

        if (readBytes >= sizeof(data) && data.magic == SAVE_MAGIC) {
            char portStr[16];
            my_itoa(data.port > 0 ? data.port : 6667, portStr);
            SetWindowTextA(hPort, portStr);
            SetWindowTextA(hMotd, data.motd);
            g_peakClients = data.peakClients;
            g_packetsIn = data.packetsIn;
            g_packetsOut = data.packetsOut;
            UpdateTelemetryUI();
            Log("[LOAD] Server state & telemetry restored from kchatserver.dat (F9).");
        } else {
            Log("[LOAD] Error: Invalid or corrupted kchatserver.dat format.");
        }
    } else {
        Log("[LOAD] Error: kchatserver.dat not found.");
    }
}

// Stop Server Daemon
void StopServer() {
    if (listenSocket == INVALID_SOCKET) return;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].s != INVALID_SOCKET) {
            closesocket(clients[i].s);
            clients[i].s = INVALID_SOCKET;
        }
    }
    closesocket(listenSocket);
    listenSocket = INVALID_SOCKET;
    g_activeClients = 0;
    UpdateTelemetryUI();

    SetWindowTextA(hBtn, "Start");
    EnableWindow(hPort, TRUE);
    EnableWindow(hMotd, TRUE);
    Log("[STOP] Server daemon shutdown. Closed all active socket descriptors.");
}

// Show Operations Manual
void ShowHelp(HWND hwnd) {
    MessageBoxA(hwnd,
        "KChat Server 1.0 (WinSock2 Multi-Client Host)\n\n"
        "Features & Protocol Parity:\n"
        "- Dual-Stack Server: WebSocket RFC 6455 & Raw TCP WinSock\n"
        "- Broadcast Console: Transmit system announcements globally\n"
        "- Telemetry: Real-time socket, peak, and packet metrics\n"
        "- F5 / F9 State Persistence: Saves to kchatserver.dat\n\n"
        "Keyboard Shortcuts:\n"
        "  S       - Start / Stop daemon\n"
        "  Enter   - Transmit Broadcast (when in broadcast field)\n"
        "  F5      - QuickSave state to kchatserver.dat\n"
        "  F9      - QuickLoad state from kchatserver.dat\n"
        "  F1      - Display this Operations Manual",
        "KChat Server Operations Manual",
        MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            WSADATA wsa;
            WSAStartup(MAKEWORD(2,2), &wsa);

            for (int i=0; i<MAX_CLIENTS; i++) {
                clients[i].s = INVALID_SOCKET;
                clients[i].buf_len = 0;
            }

            // Row 1: Port, MOTD, Start/Stop Button, Help
            CreateWindowA("STATIC", "Port:", WS_CHILD|WS_VISIBLE, 10, 10, 36, 20, hwnd, 0, 0, 0);
            hPort = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "6667", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|WS_TABSTOP, 48, 8, 54, 22, hwnd, 0, 0, 0);
            
            CreateWindowA("STATIC", "MOTD:", WS_CHILD|WS_VISIBLE, 110, 10, 42, 20, hwnd, 0, 0, 0);
            hMotd = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Welcome to KChat!", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|WS_TABSTOP, 155, 8, 160, 22, hwnd, 0, 0, 0);
            
            hBtn = CreateWindowA("BUTTON", "Start", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 322, 7, 58, 24, hwnd, (HMENU)100, 0, 0);
            hBtnHelp = CreateWindowA("BUTTON", "Help F1", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 386, 7, 68, 24, hwnd, (HMENU)104, 0, 0);

            // Row 2: Broadcast Console
            CreateWindowA("STATIC", "Broadcast:", WS_CHILD|WS_VISIBLE, 10, 38, 64, 20, hwnd, 0, 0, 0);
            hBroadcast = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|WS_TABSTOP, 76, 36, 280, 22, hwnd, 0, 0, 0);
            hBtnBroadcast = CreateWindowA("BUTTON", "Send", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 362, 35, 92, 24, hwnd, (HMENU)101, 0, 0);

            // Row 3: Persistence Buttons & Live Telemetry
            hBtnSave = CreateWindowA("BUTTON", "Save F5", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 10, 64, 62, 22, hwnd, (HMENU)102, 0, 0);
            hBtnLoad = CreateWindowA("BUTTON", "Load F9", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 76, 64, 62, 22, hwnd, (HMENU)103, 0, 0);
            hBtnClear = CreateWindowA("BUTTON", "Clear", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP, 142, 64, 52, 22, hwnd, (HMENU)105, 0, 0);
            
            hTelemetry = CreateWindowA("STATIC", "Clients: 0/64 (Peak: 0) | Pkts: 0/0", WS_CHILD|WS_VISIBLE|SS_LEFT, 204, 68, 255, 18, hwnd, 0, 0, 0);

            // Row 4: Console Log Output
            hLog = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY|WS_TABSTOP, 10, 92, 452, 290, hwnd, 0, 0, 0);
            
            int dpi = 96;
            HDC hdc = GetDC(NULL);
            if (hdc) {
                dpi = GetDeviceCaps(hdc, LOGPIXELSY);
                ReleaseDC(NULL, hdc);
            }
            int fontHeight = -MulDiv(9, dpi, 72);
            hUIFont = CreateFontA(fontHeight, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            
            SendMessageA(hPort, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hMotd, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hBtn, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hBtnHelp, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hBroadcast, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hBtnBroadcast, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hBtnSave, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hBtnLoad, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hBtnClear, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hTelemetry, WM_SETFONT, (WPARAM)hUIFont, TRUE);
            SendMessageA(hLog, WM_SETFONT, (WPARAM)hUIFont, TRUE);

            Log("[BOOT] KChat Server 1.0 (WinSock2 Multi-Client Host)");
            Log("[HINT] Press S to start, F5/F9 for quicksave/load, F1 for manual.");
            UpdateTelemetryUI();
            break;
        }
        case WM_COMMAND: {
            int cmdId = LOWORD(wParam);
            if (cmdId == 100) {
                // Toggle Start / Stop
                if (listenSocket != INVALID_SOCKET) {
                    StopServer();
                    break;
                }
                char portStr[16];
                GetWindowTextA(hPort, portStr, 16);
                int port = my_atoi(portStr);
                if (port <= 0) port = 6667;
                
                GetWindowTextA(hMotd, motd, 255);
                
                listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (listenSocket == INVALID_SOCKET) {
                    Log("[ERROR] Socket creation failed.");
                    break;
                }

                struct sockaddr_in addr;
                addr.sin_family = AF_INET;
                addr.sin_addr.s_addr = INADDR_ANY;
                addr.sin_port = htons(port);
                
                if (bind(listenSocket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
                    Log("[ERROR] Bind failed on port.");
                    closesocket(listenSocket);
                    listenSocket = INVALID_SOCKET;
                    break;
                }
                listen(listenSocket, SOMAXCONN);
                WSAAsyncSelect(listenSocket, hwnd, WM_SOCKET, FD_ACCEPT);
                
                char m[80]; my_memset(m, 0, sizeof(m));
                my_strcpy(m, "[READY] KChat daemon listening on port ");
                my_strncpy(m + my_strlen(m), portStr, 16);
                my_strcat(m, " (Dual TCP/WS)");
                Log(m);

                SetWindowTextA(hBtn, "Stop");
                EnableWindow(hPort, FALSE);
                EnableWindow(hMotd, FALSE);
                UpdateTelemetryUI();
            } else if (cmdId == 101) {
                // Send Broadcast
                char bmsg[384];
                my_memset(bmsg, 0, sizeof(bmsg));
                GetWindowTextA(hBroadcast, bmsg, 380);
                if (bmsg[0]) {
                    char out[512];
                    my_memset(out, 0, sizeof(out));
                    my_strcpy(out, "[BROADCAST] <ADMIN>: ");
                    my_strncpy(out + my_strlen(out), bmsg, 350);
                    BroadcastAll(out);
                    Log(out);
                    SetWindowTextA(hBroadcast, "");
                }
            } else if (cmdId == 102) {
                // Save F5
                SaveState();
            } else if (cmdId == 103) {
                // Load F9
                LoadState();
            } else if (cmdId == 104) {
                // Help F1
                ShowHelp(hwnd);
            } else if (cmdId == 105) {
                // Clear log
                logBuf[0] = '\0';
                SetWindowTextA(hLog, "");
                Log("[SYS] Console log buffer cleared.");
            }
            break;
        }
        case WM_SOCKET: {
            SOCKET s = (SOCKET)wParam;
            if (WSAGETSELECTERROR(lParam)) {
                if (s == listenSocket) {
                    Log("[ERROR] Listen socket error.");
                    StopServer();
                } else {
                    for (int i=0; i<MAX_CLIENTS; i++) {
                        if (clients[i].s == s) {
                            clients[i].s = INVALID_SOCKET;
                            closesocket(s);
                            if (g_activeClients > 0) g_activeClients--;
                            UpdateTelemetryUI();
                            Log("[DISCONNECT] Client socket disconnected due to network error.");
                            break;
                        }
                    }
                }
                break;
            }
            if (WSAGETSELECTEVENT(lParam) == FD_ACCEPT) {
                SOCKET client = accept(s, NULL, NULL);
                if (client != INVALID_SOCKET) {
                    int found = 0;
                    for (int i=0; i<MAX_CLIENTS; i++) {
                        if (clients[i].s == INVALID_SOCKET) {
                            clients[i].s = client;
                            my_strcpy(clients[i].room, "#general");
                            char nick[32]; my_memset(nick, 0, sizeof(nick));
                            my_strcpy(nick, "Guest");
                            char idx[16]; my_itoa(i, idx);
                            my_strcpy(nick+5, idx);
                            my_strcpy(clients[i].nick, nick);
                            clients[i].is_ws = 0;
                            clients[i].handshaked = 0;
                            clients[i].buf_len = 0;
                            
                            WSAAsyncSelect(client, hwnd, WM_SOCKET, FD_READ | FD_CLOSE);
                            g_activeClients++;
                            if (g_activeClients > g_peakClients) g_peakClients = g_activeClients;
                            g_packetsIn++;
                            UpdateTelemetryUI();

                            char m[96]; my_memset(m, 0, sizeof(m));
                            my_strcpy(m, "[CONNECT] Client slot #");
                            my_strcat(m, idx);
                            my_strcat(m, " connected -> #general");
                            Log(m);
                            
                            if (motd[0] && !clients[i].is_ws) {
                                char mm[512]; my_memset(mm, 0, sizeof(mm));
                                my_strcpy(mm, "System MOTD: ");
                                my_strncpy(mm + my_strlen(mm), motd, 450);
                                SendRaw(client, mm, 0);
                            }
                            found = 1;
                            break;
                        }
                    }
                    if (!found) closesocket(client);
                }
            } else if (WSAGETSELECTEVENT(lParam) == FD_READ) {
                for (int i=0; i<MAX_CLIENTS; i++) {
                    if (clients[i].s == s) {
                        int rem = 4096 - clients[i].buf_len;
                        if (rem > 0) {
                            int ret = recv(s, clients[i].buf + clients[i].buf_len, rem, 0);
                            if (ret > 0) {
                                clients[i].buf_len += ret;
                                g_packetsIn++;
                                UpdateTelemetryUI();
                                ProcessData(i);
                            }
                        }
                        break;
                    }
                }
            } else if (WSAGETSELECTEVENT(lParam) == FD_CLOSE) {
                for (int i=0; i<MAX_CLIENTS; i++) {
                    if (clients[i].s == s) {
                        char out[128]; my_memset(out, 0, sizeof(out));
                        my_strcpy(out, "System: ");
                        my_strncpy(out + 8, clients[i].nick, 31);
                        my_strcat(out, " disconnected.");
                        Broadcast(clients[i].room, out);

                        clients[i].s = INVALID_SOCKET;
                        closesocket(s);
                        if (g_activeClients > 0) g_activeClients--;
                        UpdateTelemetryUI();
                        Log("[DISCONNECT] Client socket closed cleanly.");
                        break;
                    }
                }
            }
            break;
        }
        case WM_DESTROY:
            StopServer();
            if (hUIFont) DeleteObject(hUIFont);
            WSACleanup();
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc; my_memset(&wc, 0, sizeof(wc));
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        typedef BOOL (WINAPI *SetProcessDPIAwareFunc)(void);
        SetProcessDPIAwareFunc setDpiAware = (SetProcessDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (setDpiAware) setDpiAware();
    }
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KChatServerClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KChatServerClass", "KChat Server (WinSock2 Multi-Client Host)",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 488, 436, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_F1) {
                SendMessageA(hwnd, WM_COMMAND, 104, 0);
                continue;
            } else if (msg.wParam == VK_F5) {
                SendMessageA(hwnd, WM_COMMAND, 102, 0);
                continue;
            } else if (msg.wParam == VK_F9) {
                SendMessageA(hwnd, WM_COMMAND, 103, 0);
                continue;
            } else if (msg.wParam == VK_RETURN && GetFocus() == hBroadcast) {
                SendMessageA(hwnd, WM_COMMAND, 101, 0);
                continue;
            }
        }
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    ExitProcess(0);
}