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

// SHA1
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
    unsigned int i, j;
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

struct Client clients[MAX_CLIENTS];
SOCKET listenSocket = INVALID_SOCKET;
HWND hLog, hPort, hBtn, hMotd;
char logBuf[16384] = "";
char motd[256] = "Welcome to KChat!";

void Log(const char* msg) {
    int len = my_strlen(logBuf);
    int mlen = my_strlen(msg);
    if (len + mlen + 5 > sizeof(logBuf)) {
        my_strcpy(logBuf, logBuf + (len / 2));
        len = my_strlen(logBuf);
    }
    my_strcpy(logBuf + len, msg);
    my_strcpy(logBuf + len + mlen, "\r\n");
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
}

void Broadcast(const char* room, const char* msg) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].s != INVALID_SOCKET && my_strcmp(clients[i].room, room) == 0 && (clients[i].handshaked || !clients[i].is_ws)) {
            SendRaw(clients[i].s, msg, clients[i].is_ws);
        }
    }
}

void HandleMessage(int cidx, char* msg) {
    // Strip newlines
    for (int i=0; msg[i]; i++) if (msg[i] == '\r' || msg[i] == '\n') msg[i] = 0;
    if (!msg[0]) return;
    
    if (my_strncmp(msg, "/join ", 6) == 0) {
        my_strcpy(clients[cidx].room, msg + 6);
        char out[128]; my_memset(out, 0, sizeof(out));
        my_strcpy(out, "System: Joined ");
        my_strcpy(out + 15, msg + 6);
        SendRaw(clients[cidx].s, out, clients[cidx].is_ws);
    } else if (my_strncmp(msg, "/nick ", 6) == 0) {
        my_strcpy(clients[cidx].nick, msg + 6);
        char out[128]; my_memset(out, 0, sizeof(out));
        my_strcpy(out, "System: Nickname changed to ");
        my_strcpy(out + my_strlen(out), msg + 6);
        SendRaw(clients[cidx].s, out, clients[cidx].is_ws);
    } else if (my_strcmp(msg, "/list") == 0) {
        SendRaw(clients[cidx].s, "System: Active Rooms:", clients[cidx].is_ws);
        for (int i=0; i<MAX_CLIENTS; i++) {
            if (clients[i].s != INVALID_SOCKET) {
                // very simple duplicate avoidance would be nice, but skipping for minimal size
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
        my_strcpy(out + my_strlen(out), clients[cidx].nick);
        my_strcpy(out + my_strlen(out), " ");
        my_strcpy(out + my_strlen(out), msg + 4);
        Broadcast(clients[cidx].room, out);
        Log(out);
    } else {
        char out[512]; my_memset(out, 0, sizeof(out));
        my_strcpy(out, "[");
        my_strcpy(out + 1, clients[cidx].nick);
        my_strcpy(out + my_strlen(out), "] ");
        my_strcpy(out + my_strlen(out), msg);
        Broadcast(clients[cidx].room, out);
        Log(out);
    }
}

void ProcessData(int cidx) {
    struct Client* c = &clients[cidx];
    if (!c->is_ws && !c->handshaked) {
        // First message heuristic
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
        if (!end) return; // Wait for full headers
        
        char* key_start = 0;
        for (int i=0; i < c->buf_len - 20; i++) {
            if (my_strncmp(&c->buf[i], "Sec-WebSocket-Key: ", 19) == 0) {
                key_start = &c->buf[i+19]; break;
            }
        }
        
        if (key_start) {
            char key[256]; my_memset(key, 0, sizeof(key));
            int ki = 0;
            while (*key_start && *key_start != '\r' && *key_start != '\n') key[ki++] = *key_start++;
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
            c->buf_len = 0; // clear buffer
            
            if (motd[0]) {
                char m[512]; my_memset(m, 0, sizeof(m));
                my_strcpy(m, "System MOTD: ");
                my_strcpy(m + my_strlen(m), motd);
                SendRaw(c->s, m, 1);
            }
        }
        return;
    }

    if (c->is_ws && c->handshaked) {
        // Read WebSocket frames
        while (c->buf_len >= 2) {
            unsigned char b1 = c->buf[0];
            unsigned char b2 = c->buf[1];
            int fin = (b1 & 0x80) != 0;
            int opcode = b1 & 0x0F;
            int masked = (b2 & 0x80) != 0;
            int payload_len = b2 & 0x7F;
            int header_len = 2;
            
            if (payload_len == 126) { header_len += 2; }
            else if (payload_len == 127) { header_len += 8; }
            if (masked) header_len += 4;
            
            if (c->buf_len < header_len) break; // Need more data
            
            if (payload_len == 126) payload_len = ((unsigned char)c->buf[2] << 8) | (unsigned char)c->buf[3];
            
            if (c->buf_len < header_len + payload_len) break; // Need payload
            
            if (opcode == 8) {
                // Close
                c->buf_len = 0; // force close loop
                return; 
            }
            
            if (opcode == 1) { // Text
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
                HandleMessage(cidx, payload);
            }
            
            // Advance buffer
            int total = header_len + payload_len;
            c->buf_len -= total;
            if (c->buf_len > 0) my_memcpy(c->buf, c->buf + total, c->buf_len);
        }
    } else if (!c->is_ws && c->handshaked) {
        // Raw TCP
        static char msg[4096]; my_memset(msg, 0, sizeof(msg));
        int len = c->buf_len > 4000 ? 4000 : c->buf_len;
        my_memcpy(msg, c->buf, len);
        HandleMessage(cidx, msg);
        c->buf_len = 0;
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            for (int i=0; i<MAX_CLIENTS; i++) {
                clients[i].s = INVALID_SOCKET;
                clients[i].buf_len = 0;
            }
            
            CreateWindowA("STATIC", "Port:", WS_CHILD|WS_VISIBLE, 10, 10, 40, 20, hwnd, 0, 0, 0);
            hPort = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "6667", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 50, 10, 60, 24, hwnd, 0, 0, 0);
            
            CreateWindowA("STATIC", "MOTD:", WS_CHILD|WS_VISIBLE, 120, 10, 40, 20, hwnd, 0, 0, 0);
            hMotd = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Welcome to KChat!", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 165, 10, 140, 24, hwnd, 0, 0, 0);
            
            hBtn = CreateWindowA("BUTTON", "Start", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, 315, 10, 60, 24, hwnd, (HMENU)100, 0, 0);
            
            hLog = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY, 10, 40, 365, 260, hwnd, 0, 0, 0);
            
            HFONT hFont = CreateFontA(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Tahoma");
            SendMessageA(hPort, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hMotd, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hLog, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 100) {
                if (listenSocket != INVALID_SOCKET) {
                    Log("Already running.");
                    break;
                }
                char portStr[16];
                GetWindowTextA(hPort, portStr, 16);
                int port = my_atoi(portStr);
                
                GetWindowTextA(hMotd, motd, 255);
                
                WSADATA wsa;
                WSAStartup(MAKEWORD(2,2), &wsa);
                
                listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                struct sockaddr_in addr;
                addr.sin_family = AF_INET;
                addr.sin_addr.s_addr = INADDR_ANY;
                addr.sin_port = htons(port);
                
                if (bind(listenSocket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
                    Log("Bind failed.");
                    closesocket(listenSocket);
                    listenSocket = INVALID_SOCKET;
                    break;
                }
                listen(listenSocket, SOMAXCONN);
                WSAAsyncSelect(listenSocket, hwnd, WM_SOCKET, FD_ACCEPT);
                
                char m[64]; my_memset(m, 0, sizeof(m));
                my_strcpy(m, "Server started on port ");
                my_strcpy(m + my_strlen(m), portStr);
                Log(m);
                EnableWindow(hBtn, FALSE);
                EnableWindow(hPort, FALSE);
                EnableWindow(hMotd, FALSE);
            }
            break;
        }
        case WM_SOCKET: {
            if (WSAGETSELECTERROR(lParam)) {
                // Ignore disconnect errors silently
            }
            SOCKET s = (SOCKET)wParam;
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
                            Log("Client connected.");
                            
                            if (motd[0] && !clients[i].is_ws) {
                                // Raw TCP can send immediately, WS must wait for handshake
                                char m[512]; my_memset(m, 0, sizeof(m));
                                my_strcpy(m, "System MOTD: ");
                                my_strcpy(m + my_strlen(m), motd);
                                SendRaw(client, m, 0);
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
                                ProcessData(i);
                            }
                        }
                        break;
                    }
                }
            } else if (WSAGETSELECTEVENT(lParam) == FD_CLOSE) {
                for (int i=0; i<MAX_CLIENTS; i++) {
                    if (clients[i].s == s) {
                        clients[i].s = INVALID_SOCKET;
                        closesocket(s);
                        Log("Client disconnected.");
                        break;
                    }
                }
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc; my_memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KChatServerClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KChatServerClass", "KChat Server (WS/TCP)", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 400, 350, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
