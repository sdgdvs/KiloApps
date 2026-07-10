#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "gdi32.lib")

#define WM_SOCKET (WM_USER + 1)

#pragma optimize("", off)
void my_memset(void* dest, int c, size_t count) {
    volatile char* bytes = (volatile char*)dest;
    while (count--) {
        *bytes++ = (char)c;
    }
}
#pragma optimize("", on)

SOCKET s = INVALID_SOCKET;
HWND hLog, hIp, hPort, hBtn, hInput, hSend;
char logBuf[16384] = "";

void my_strcpy(char* d, const char* s) { while (*s) *d++ = *s++; *d = 0; }
int my_strlen(const char* s) { int l = 0; while (s[l]) l++; return l; }
int my_atoi(const char* str) {
    int v = 0;
    while (*str >= '0' && *str <= '9') { v = v * 10 + (*str - '0'); str++; }
    return v;
}

void Log(const char* msg) {
    int len = my_strlen(logBuf);
    int mlen = my_strlen(msg);
    if (len + mlen + 5 > sizeof(logBuf)) {
        int cut = len / 2;
        while (cut < len && logBuf[cut] != '\n') cut++;
        if (cut < len) cut++;
        my_strcpy(logBuf, logBuf + cut);
        len = my_strlen(logBuf);
    }
    my_strcpy(logBuf + len, msg);
    my_strcpy(logBuf + len + mlen, "\r\n");
    SetWindowTextA(hLog, logBuf);
    SendMessageA(hLog, EM_LINESCROLL, 0, 9999);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            CreateWindowA("STATIC", "IP:", WS_CHILD|WS_VISIBLE, 10, 10, 20, 20, hwnd, 0, 0, 0);
            hIp = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "127.0.0.1", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 35, 10, 100, 24, hwnd, 0, 0, 0);
            
            CreateWindowA("STATIC", "Port:", WS_CHILD|WS_VISIBLE, 145, 10, 30, 20, hwnd, 0, 0, 0);
            hPort = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "6667", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 180, 10, 50, 24, hwnd, 0, 0, 0);
            
            hBtn = CreateWindowA("BUTTON", "Connect", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, 240, 10, 80, 24, hwnd, (HMENU)100, 0, 0);
            
            hLog = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY, 10, 40, 360, 220, hwnd, 0, 0, 0);
            
            hInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 10, 270, 290, 24, hwnd, 0, 0, 0);
            hSend = CreateWindowA("BUTTON", "Send", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|WS_DISABLED, 310, 270, 60, 24, hwnd, (HMENU)101, 0, 0);
            
            HFONT hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            SendMessageA(hIp, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hPort, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hLog, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hInput, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hSend, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            Log("Welcome to KChat! Connect to a server to begin.");
            Log("Commands: /nick <name>, /join <#room>");
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(255, 255, 255));
            SetBkMode(hdcStatic, TRANSPARENT);
            return (LRESULT)GetStockObject(NULL_BRUSH);
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 100) { // Connect
                if (s != INVALID_SOCKET) {
                    closesocket(s);
                    s = INVALID_SOCKET;
                    SetWindowTextA(hBtn, "Connect");
                    EnableWindow(hSend, FALSE);
                    Log("Disconnected.");
                    break;
                }
                
                char ip[64], portStr[16];
                GetWindowTextA(hIp, ip, 64);
                GetWindowTextA(hPort, portStr, 16);
                int port = my_atoi(portStr);
                
                WSADATA wsa;
                WSAStartup(MAKEWORD(2,2), &wsa);
                
                s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                
                // Get host by name dynamically
                struct hostent *he = gethostbyname(ip);
                if (!he) {
                    Log("Invalid IP/Host.");
                    closesocket(s);
                    s = INVALID_SOCKET;
                    break;
                }
                
                struct sockaddr_in addr;
                addr.sin_family = AF_INET;
                addr.sin_addr = *((struct in_addr *)he->h_addr);
                addr.sin_port = htons(port);
                
                WSAAsyncSelect(s, hwnd, WM_SOCKET, FD_CONNECT | FD_READ | FD_CLOSE);
                connect(s, (struct sockaddr*)&addr, sizeof(addr));
                Log("Connecting...");
                EnableWindow(hBtn, FALSE);
            } else if (LOWORD(wParam) == 101) { // Send
                if (s == INVALID_SOCKET) break;
                char buf[512];
                GetWindowTextA(hInput, buf, 512);
                if (buf[0]) {
                    send(s, buf, my_strlen(buf), 0);
                    SetWindowTextA(hInput, "");
                }
            }
            break;
        }
        case WM_SOCKET: {
            if (WSAGETSELECTERROR(lParam)) {
                Log("Socket error/disconnected.");
                if (s != INVALID_SOCKET) closesocket(s);
                s = INVALID_SOCKET;
                EnableWindow(hBtn, TRUE);
                EnableWindow(hSend, FALSE);
                break;
            }
            if (WSAGETSELECTEVENT(lParam) == FD_CONNECT) {
                Log("Connected!");
                EnableWindow(hBtn, TRUE);
                SetWindowTextA(hBtn, "Reconnect");
                EnableWindow(hSend, TRUE);
            } else if (WSAGETSELECTEVENT(lParam) == FD_READ) {
                char buf[513];
                int ret = recv(s, buf, 512, 0);
                if (ret > 0) {
                    buf[ret] = 0;
                    // strip trailing newlines for cleaner log
                    while(ret > 0 && (buf[ret-1] == '\r' || buf[ret-1] == '\n')) {
                        buf[ret-1] = 0;
                        ret--;
                    }
                    Log(buf);
                }
            } else if (WSAGETSELECTEVENT(lParam) == FD_CLOSE) {
                Log("Disconnected by server.");
                closesocket(s);
                s = INVALID_SOCKET;
                EnableWindow(hBtn, TRUE);
                EnableWindow(hSend, FALSE);
            }
            break;
        }
        case WM_DESTROY:
            if (s != INVALID_SOCKET) closesocket(s);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc;
    my_memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KChatClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(15, 23, 42));

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KChatClass", "KChat", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 400, 350, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
