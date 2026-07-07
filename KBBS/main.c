#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "gdi32.lib")

#define WM_SOCKET (WM_USER + 1)

#define TERM_COLS 80
#define TERM_ROWS 25

#define STATE_NORMAL 0
#define STATE_ESC    1
#define STATE_CSI    2

#define TELNET_IAC   0xFF
#define TELNET_WILL  0xFB
#define TELNET_WONT  0xFC
#define TELNET_DO    0xFD
#define TELNET_DONT  0xFE
#define TELNET_SB    0xFA
#define TELNET_SE    0xF0
#define TELNET_NAWS  31
#define TELNET_TTYPE 24

#define TEL_STATE_NORMAL 0
#define TEL_STATE_IAC    1
#define TEL_STATE_WILL   2
#define TEL_STATE_WONT   3
#define TEL_STATE_DO     4
#define TEL_STATE_DONT   5
#define TEL_STATE_SB     6
#define TEL_STATE_SB_DATA 7

#pragma optimize("", off)
void my_memset(void* dest, int c, size_t count) {
    volatile char* bytes = (volatile char*)dest;
    while (count--) {
        *bytes++ = (char)c;
    }
}
#pragma optimize("", on)

void my_strcpy(char* d, const char* s) { while (*s) *d++ = *s++; *d = 0; }
int my_strlen(const char* s) { int l = 0; while (s[l]) l++; return l; }
int my_atoi(const char* str) {
    int v = 0;
    while (*str >= '0' && *str <= '9') { v = v * 10 + (*str - '0'); str++; }
    return v;
}

void my_itoa(int val, char* buf) {
    char tmp[16];
    int i = 0;
    if (val == 0) { buf[0] = '0'; buf[1] = 0; return; }
    while (val > 0) { tmp[i++] = '0' + (val % 10); val /= 10; }
    int j = 0;
    while (i > 0) { buf[j++] = tmp[--i]; }
    buf[j] = 0;
}

int dpiScale(int x) {
    static int dpi = 0;
    if (!dpi) {
        HDC hdc = GetDC(NULL);
        if (hdc) {
            dpi = GetDeviceCaps(hdc, LOGPIXELSX);
            ReleaseDC(NULL, hdc);
        }
        if (dpi == 0) dpi = 96;
    }
    return (x * dpi) / 96;
}

/* 16-color ANSI palette (COLORREF is BGR) */
COLORREF ansiColors[16] = {
    0x000000, 0x000080, 0x008000, 0x008080,  /* black, dark red, dark green, dark yellow */
    0x800000, 0x800080, 0x808000, 0xC0C0C0,  /* dark blue, dark magenta, dark cyan, light gray */
    0x808080, 0x0000FF, 0x00FF00, 0x00FFFF,  /* dark gray, bright red, bright green, bright yellow */
    0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF   /* bright blue, bright magenta, bright cyan, white */
};

struct Cell { char ch; unsigned char fg; unsigned char bg; };
#define MAX_LINES 1000
struct Cell screen[MAX_LINES][TERM_COLS];
int activeTop = 0;
int scrollOffset = 0;

int curX = 0, curY = 0;
int savedX = 0, savedY = 0;
unsigned char curFg = 7;  /* default: light gray */
unsigned char curBg = 0;  /* default: black */
int boldFlag = 0;
int blinkState = 1;

int ansiState = STATE_NORMAL;
char ansiParams[64];
int ansiParamLen = 0;

int telState = TEL_STATE_NORMAL;
unsigned char telSBOption = 0;
unsigned char telSBData[64];
int telSBLen = 0;

SOCKET sock = INVALID_SOCKET;
HWND hMain, hHost, hPort, hBtn, hCombo, hStatus;
HFONT hTermFont;

struct BBS { const char* name; const char* host; int port; };
struct BBS bbsList[] = {
    {"20 For Beers", "20forbeers.com", 1337},
    {"Mutiny BBS", "mutinybbs.com", 23},
    {"Borderline BBS", "borderlinebbs.dyndns.org", 9023},
    {"Dreamline BBS", "din.asciiattic.com", 23},
    {"Bottomless Abyss", "bbs.bottomlessabyss.net", 2323},
    {"Dark Realms", "bbs.darkrealms.ca", 23},
    {"Throwback BBS", "bbs.throwbackbbs.com", 23},
    {"Vertrauen", "vert.synchro.net", 23},
    {"Electronic Chicken", "bbs.electronicchicken.com", 23},
    {"Capitol City Online", "capitolcityonline.net", 23},
    {"Agency BBS", "bbs.agency", 23},
    {"The Titanic BBS", "ttb.rgbbs.info", 23},
    {"Al's Geek Lab", "bbs.alsgeeklab.com", 2323},
    {"Retrocampus BBS", "bbs.retrocampus.com", 23},
    {"Air & Wave BBS", "bbs.airandwave.net", 2323},
    {"BBS NZ", "bbs.nz", 23},
    {"Blocktronics HQ", "blocktronics.org", 23},
    {"The Keep", "bbs.thekeep.net", 23},
    {"13th Leader", "13leader.net", 8023},
    {"8-Bit Boyz", "bbs.8bitboyz.com", 23},
    {"Particles BBS", "particlesbbs.dyndns.org", 23},
    {"300 Baud", "300baud.dynu.net", 2525},
    {"Fozztexx", "bbs.fozztexx.com", 23},
    {"Xibalba", "xibalba.l33t.codes", 44510},
    {"Black Flag", "blackflag.acid.org", 23},
    {"Clutch BBS", "clutchbbs.com", 23},
    {"End of the Line", "bbs.endofthelinebbs.com", 23},
    {"Absinthe", "bbs.absinthe.ws", 23},
    {"Broken Bubble", "bbs.thebrokenbubble.com", 23},
    {"20 For Beers", "20forbeers.com", 1337}
};
#define BBS_COUNT (sizeof(bbsList) / sizeof(bbsList[0]))

void ClearScreen(void) {
    int r, c;
    for (r = 0; r < MAX_LINES; r++) {
        for (c = 0; c < TERM_COLS; c++) {
            screen[r][c].ch = ' ';
            screen[r][c].fg = 7;
            screen[r][c].bg = 0;
        }
    }
    activeTop = 0;
    scrollOffset = 0;
    curX = 0;
    curY = 0;
}

void ScrollUp(void) {
    int r, c;
    if (activeTop + TERM_ROWS < MAX_LINES) {
        activeTop++;
    } else {
        int shift = 100;
        for (r = 0; r < MAX_LINES - shift; r++) {
            for (c = 0; c < TERM_COLS; c++) {
                screen[r][c] = screen[r + shift][c];
            }
        }
        activeTop -= shift;
        for (r = MAX_LINES - shift; r < MAX_LINES; r++) {
            for (c = 0; c < TERM_COLS; c++) {
                screen[r][c].ch = ' ';
                screen[r][c].fg = 7;
                screen[r][c].bg = 0;
            }
        }
        activeTop++;
    }
    for (c = 0; c < TERM_COLS; c++) {
        screen[activeTop + TERM_ROWS - 1][c].ch = ' ';
        screen[activeTop + TERM_ROWS - 1][c].fg = 7;
        screen[activeTop + TERM_ROWS - 1][c].bg = 0;
    }
    if (scrollOffset > 0) {
        scrollOffset++;
    }
}

void AdvanceLine(void) {
    curY++;
    if (curY >= TERM_ROWS) {
        ScrollUp();
        curY = TERM_ROWS - 1;
    }
}

void PutChar(char ch) {
    if (curX >= TERM_COLS) {
        curX = 0;
        AdvanceLine();
    }
    screen[activeTop + curY][curX].ch = ch;
    screen[activeTop + curY][curX].fg = boldFlag ? (curFg | 8) : curFg;
    screen[activeTop + curY][curX].bg = curBg;
    curX++;
}

/* Parse semicolon-separated ANSI params */
int ParseParams(const char* buf, int len, int* params, int maxParams) {
    int count = 0;
    int val = 0;
    int hasVal = 0;
    int i;
    for (i = 0; i < len && count < maxParams; i++) {
        if (buf[i] >= '0' && buf[i] <= '9') {
            val = val * 10 + (buf[i] - '0');
            hasVal = 1;
        } else if (buf[i] == ';') {
            params[count++] = hasVal ? val : 0;
            val = 0;
            hasVal = 0;
        }
    }
    if (hasVal || count == 0) {
        params[count++] = val;
    }
    return count;
}

void ProcessSGR(int* params, int count) {
    int i;
    for (i = 0; i < count; i++) {
        int p = params[i];
        if (p == 0) { curFg = 7; curBg = 0; boldFlag = 0; }
        else if (p == 1) { boldFlag = 1; }
        else if (p == 5) { curFg |= 0x80; } /* Blink on */
        else if (p == 25) { curFg &= ~0x80; } /* Blink off */
        else if (p >= 30 && p <= 37) { curFg = (curFg & 0x80) | (unsigned char)(p - 30); }
        else if (p >= 40 && p <= 47) { curBg = (unsigned char)(p - 40); }
        else if (p >= 90 && p <= 97) { curFg = (curFg & 0x80) | (unsigned char)(p - 90 + 8); }
        else if (p >= 100 && p <= 107) { curBg = (unsigned char)(p - 100 + 8); }
        else if (p == 39) { curFg = (curFg & 0x80) | 7; } /* default fg */
        else if (p == 49) { curBg = 0; } /* default bg */
    }
}

void ProcessCSI(char finalChar) {
    int params[16];
    int count = ParseParams(ansiParams, ansiParamLen, params, 16);
    int r, c;

    switch (finalChar) {
        case 'm': /* SGR */
            ProcessSGR(params, count);
            break;
        case 'H': /* CUP - cursor position */
        case 'f':
            curY = (count >= 1 && params[0] > 0) ? params[0] - 1 : 0;
            curX = (count >= 2 && params[1] > 0) ? params[1] - 1 : 0;
            if (curY >= TERM_ROWS) curY = TERM_ROWS - 1;
            if (curX >= TERM_COLS) curX = TERM_COLS - 1;
            break;
        case 'A': /* Cursor Up */
            r = (count >= 1 && params[0] > 0) ? params[0] : 1;
            curY -= r;
            if (curY < 0) curY = 0;
            break;
        case 'B': /* Cursor Down */
            r = (count >= 1 && params[0] > 0) ? params[0] : 1;
            curY += r;
            if (curY >= TERM_ROWS) curY = TERM_ROWS - 1;
            break;
        case 'C': /* Cursor Right */
            c = (count >= 1 && params[0] > 0) ? params[0] : 1;
            curX += c;
            if (curX >= TERM_COLS) curX = TERM_COLS - 1;
            break;
        case 'D': /* Cursor Left */
            c = (count >= 1 && params[0] > 0) ? params[0] : 1;
            curX -= c;
            if (curX < 0) curX = 0;
            break;
        case 'J': /* Erase Display */
            if (params[0] == 2 || params[0] == 3) {
                for (r = 0; r < TERM_ROWS; r++) {
                    for (c = 0; c < TERM_COLS; c++) {
                        screen[activeTop + r][c].ch = ' ';
                        screen[activeTop + r][c].fg = curFg;
                        screen[activeTop + r][c].bg = curBg;
                    }
                }
                curX = 0;
                curY = 0;
            } else if (params[0] == 0) {
                /* clear from cursor to end */
                for (c = curX; c < TERM_COLS; c++) {
                    screen[activeTop + curY][c].ch = ' ';
                    screen[activeTop + curY][c].fg = curFg;
                    screen[activeTop + curY][c].bg = curBg;
                }
                for (r = curY + 1; r < TERM_ROWS; r++) {
                    for (c = 0; c < TERM_COLS; c++) {
                        screen[activeTop + r][c].ch = ' ';
                        screen[activeTop + r][c].fg = curFg;
                        screen[activeTop + r][c].bg = curBg;
                    }
                }
            } else if (params[0] == 1) {
                /* clear from start to cursor */
                for (r = 0; r < curY; r++) {
                    for (c = 0; c < TERM_COLS; c++) {
                        screen[activeTop + r][c].ch = ' ';
                        screen[activeTop + r][c].fg = curFg;
                        screen[activeTop + r][c].bg = curBg;
                    }
                }
                for (c = 0; c <= curX; c++) {
                    screen[activeTop + curY][c].ch = ' ';
                    screen[activeTop + curY][c].fg = curFg;
                    screen[activeTop + curY][c].bg = curBg;
                }
            }
            break;
        case 'K': /* Erase Line */
            if (params[0] == 0) {
                /* clear from cursor to end of line */
                for (c = curX; c < TERM_COLS; c++) {
                    screen[activeTop + curY][c].ch = ' ';
                    screen[activeTop + curY][c].fg = curFg;
                    screen[activeTop + curY][c].bg = curBg;
                }
            } else if (params[0] == 1) {
                /* clear from start to cursor */
                for (c = 0; c <= curX; c++) {
                    screen[activeTop + curY][c].ch = ' ';
                    screen[activeTop + curY][c].fg = curFg;
                    screen[activeTop + curY][c].bg = curBg;
                }
            } else if (params[0] == 2) {
                /* clear entire line */
                for (c = 0; c < TERM_COLS; c++) {
                    screen[activeTop + curY][c].ch = ' ';
                    screen[activeTop + curY][c].fg = curFg;
                    screen[activeTop + curY][c].bg = curBg;
                }
            }
            break;
        case 's': /* Save cursor */
            savedX = curX;
            savedY = curY;
            break;
        case 'u': /* Restore cursor */
            curX = savedX;
            curY = savedY;
            break;
    }
}

void ProcessByte(unsigned char ch) {
    switch (ansiState) {
        case STATE_NORMAL:
            if (ch == 0x1B) {
                ansiState = STATE_ESC;
            } else if (ch == '\r') {
                curX = 0;
            } else if (ch == '\n') {
                AdvanceLine();
            } else if (ch == '\b') {
                if (curX > 0) curX--;
            } else if (ch == '\t') {
                /* tab to next 8-col stop */
                int stop = (curX / 8 + 1) * 8;
                if (stop > TERM_COLS) stop = TERM_COLS;
                while (curX < stop) PutChar(' ');
            } else if (ch == '\a') {
                /* bell - ignore */
            } else if (ch >= 32) {
                PutChar((char)ch);
            }
            break;
        case STATE_ESC:
            if (ch == '[') {
                ansiState = STATE_CSI;
                ansiParamLen = 0;
            } else {
                ansiState = STATE_NORMAL;
            }
            break;
        case STATE_CSI:
            if ((ch >= '0' && ch <= '9') || ch == ';' || ch == '?') {
                if (ansiParamLen < (int)(sizeof(ansiParams) - 1)) {
                    ansiParams[ansiParamLen++] = (char)ch;
                }
            } else if (ch >= 0x40 && ch <= 0x7E) {
                ansiParams[ansiParamLen] = 0;
                ProcessCSI((char)ch);
                ansiState = STATE_NORMAL;
            } else {
                ansiState = STATE_NORMAL;
            }
            break;
    }
}

/* Telnet negotiation */
void SendTelnetResponse(unsigned char cmd, unsigned char option) {
    unsigned char buf[3];
    buf[0] = TELNET_IAC;
    buf[1] = cmd;
    buf[2] = option;
    send(sock, (char*)buf, 3, 0);
}

void SendNAWS(void) {
    unsigned char buf[9];
    buf[0] = TELNET_IAC;
    buf[1] = TELNET_SB;
    buf[2] = TELNET_NAWS;
    buf[3] = 0; buf[4] = TERM_COLS; /* width high, low */
    buf[5] = 0; buf[6] = TERM_ROWS; /* height high, low */
    buf[7] = TELNET_IAC;
    buf[8] = TELNET_SE;
    send(sock, (char*)buf, 9, 0);
}

void SendTermType(void) {
    /* IAC SB TERMINAL-TYPE IS "ANSI" IAC SE */
    unsigned char buf[10];
    buf[0] = TELNET_IAC;
    buf[1] = TELNET_SB;
    buf[2] = TELNET_TTYPE;
    buf[3] = 0; /* IS */
    buf[4] = 'A';
    buf[5] = 'N';
    buf[6] = 'S';
    buf[7] = 'I';
    buf[8] = TELNET_IAC;
    buf[9] = TELNET_SE;
    send(sock, (char*)buf, 10, 0);
}

void ProcessTelnetByte(unsigned char ch) {
    switch (telState) {
        case TEL_STATE_NORMAL:
            if (ch == TELNET_IAC) {
                telState = TEL_STATE_IAC;
            } else {
                ProcessByte(ch);
            }
            break;
        case TEL_STATE_IAC:
            if (ch == TELNET_WILL) { telState = TEL_STATE_WILL; }
            else if (ch == TELNET_WONT) { telState = TEL_STATE_WONT; }
            else if (ch == TELNET_DO) { telState = TEL_STATE_DO; }
            else if (ch == TELNET_DONT) { telState = TEL_STATE_DONT; }
            else if (ch == TELNET_SB) { telState = TEL_STATE_SB; }
            else if (ch == TELNET_IAC) { ProcessByte(ch); telState = TEL_STATE_NORMAL; }
            else { telState = TEL_STATE_NORMAL; }
            break;
        case TEL_STATE_WILL:
            /* Server offers option - refuse most */
            SendTelnetResponse(TELNET_DONT, ch);
            telState = TEL_STATE_NORMAL;
            break;
        case TEL_STATE_WONT:
            telState = TEL_STATE_NORMAL;
            break;
        case TEL_STATE_DO:
            if (ch == TELNET_TTYPE) {
                SendTelnetResponse(TELNET_WILL, ch);
            } else if (ch == TELNET_NAWS) {
                SendTelnetResponse(TELNET_WILL, ch);
                SendNAWS();
            } else {
                SendTelnetResponse(TELNET_WONT, ch);
            }
            telState = TEL_STATE_NORMAL;
            break;
        case TEL_STATE_DONT:
            SendTelnetResponse(TELNET_WONT, ch);
            telState = TEL_STATE_NORMAL;
            break;
        case TEL_STATE_SB:
            telSBOption = ch;
            telSBLen = 0;
            telState = TEL_STATE_SB_DATA;
            break;
        case TEL_STATE_SB_DATA:
            if (ch == TELNET_IAC) {
                /* Next byte should be SE */
                telState = TEL_STATE_NORMAL;
                /* Handle sub-negotiation */
                if (telSBOption == TELNET_TTYPE && telSBLen > 0 && telSBData[0] == 1) {
                    /* SEND request */
                    SendTermType();
                }
            } else {
                if (telSBLen < (int)sizeof(telSBData)) {
                    telSBData[telSBLen++] = ch;
                }
            }
            break;
    }
}

void SetStatusText(const char* text) {
    SetWindowTextA(hStatus, text);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            unsigned int i;
            SetTimer(hwnd, 1, 500, NULL);
            HFONT hFont = CreateFontA(dpiScale(14), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
                ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                DEFAULT_PITCH | FF_SWISS, "Tahoma");

            hTermFont = CreateFontA(dpiScale(16), dpiScale(8), 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,
                FIXED_PITCH | FF_MODERN, "Consolas");

            /* Top bar controls */
            CreateWindowA("STATIC", "Host:", WS_CHILD | WS_VISIBLE, dpiScale(5), dpiScale(7), dpiScale(30), dpiScale(20), hwnd, 0, 0, 0);
            hHost = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                dpiScale(38), dpiScale(4), dpiScale(200), dpiScale(22), hwnd, 0, 0, 0);

            CreateWindowA("STATIC", "Port:", WS_CHILD | WS_VISIBLE, dpiScale(245), dpiScale(7), dpiScale(28), dpiScale(20), hwnd, 0, 0, 0);
            hPort = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "23", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                dpiScale(276), dpiScale(4), dpiScale(60), dpiScale(22), hwnd, 0, 0, 0);

            hBtn = CreateWindowA("BUTTON", "Connect", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                dpiScale(342), dpiScale(4), dpiScale(80), dpiScale(22), hwnd, (HMENU)100, 0, 0);

            hCombo = CreateWindowA("COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                dpiScale(428), dpiScale(4), dpiScale(200), dpiScale(400), hwnd, (HMENU)101, 0, 0);

            /* Status bar */
            hStatus = CreateWindowA("STATIC", "Disconnected", WS_CHILD | WS_VISIBLE | SS_LEFT,
                dpiScale(5), dpiScale(30 + TERM_ROWS * 16 + 4), dpiScale(640), dpiScale(18), hwnd, 0, 0, 0);

            /* Set fonts */
            SendMessageA(hHost, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hPort, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hStatus, WM_SETFONT, (WPARAM)hFont, TRUE);

            /* Populate BBS list */
            for (i = 0; i < BBS_COUNT; i++) {
                SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)bbsList[i].name);
            }

            ClearScreen();
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 100) { /* Connect button */
                char host[128], portStr[16];
                int port;
                struct hostent *he;
                struct sockaddr_in addr;
                WSADATA wsa;

                if (sock != INVALID_SOCKET) {
                    closesocket(sock);
                    sock = INVALID_SOCKET;
                }

                GetWindowTextA(hHost, host, 128);
                GetWindowTextA(hPort, portStr, 16);
                port = my_atoi(portStr);

                if (host[0] == 0 || port == 0) {
                    SetStatusText("Enter host and port");
                    break;
                }

                WSAStartup(MAKEWORD(2, 2), &wsa);
                sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

                he = gethostbyname(host);
                if (!he) {
                    SetStatusText("DNS lookup failed");
                    break;
                }

                my_memset(&addr, 0, sizeof(addr));
                addr.sin_family = AF_INET;
                addr.sin_addr = *((struct in_addr *)he->h_addr);
                addr.sin_port = htons((unsigned short)port);

                WSAAsyncSelect(sock, hwnd, WM_SOCKET, FD_CONNECT | FD_READ | FD_CLOSE);
                connect(sock, (struct sockaddr*)&addr, sizeof(addr));

                SetStatusText("Connecting...");
                EnableWindow(hBtn, FALSE);
                SetFocus(hwnd);

                /* Reset terminal */
                ClearScreen();
                curFg = 7; curBg = 0; boldFlag = 0;
                ansiState = STATE_NORMAL;
                ansiParamLen = 0;
                telState = TEL_STATE_NORMAL;
                InvalidateRect(hwnd, NULL, FALSE);
                RECT cr;
                GetClientRect(hwnd, &cr);
                PostMessage(hwnd, WM_SIZE, 0, MAKELPARAM(cr.right, cr.bottom));
            }
            else if (LOWORD(wParam) == 101 && HIWORD(wParam) == CBN_SELCHANGE) {
                /* BBS dropdown selection changed */
                int sel = (int)SendMessageA(hCombo, CB_GETCURSEL, 0, 0);
                if (sel >= 0 && sel < (int)BBS_COUNT) {
                    char portBuf[16];
                    SetWindowTextA(hHost, bbsList[sel].host);
                    my_itoa(bbsList[sel].port, portBuf);
                    SetWindowTextA(hPort, portBuf);
                }
            }
            break;
        }
        case WM_SOCKET: {
            if (WSAGETSELECTERROR(lParam)) {
                SetStatusText("Connection error");
                if (sock != INVALID_SOCKET) closesocket(sock);
                sock = INVALID_SOCKET;
                EnableWindow(hBtn, TRUE);
                break;
            }
            if (WSAGETSELECTEVENT(lParam) == FD_CONNECT) {
                SetStatusText("Connected");
                EnableWindow(hBtn, TRUE);
                SetWindowTextA(hBtn, "Reconnect");
            } else if (WSAGETSELECTEVENT(lParam) == FD_READ) {
                unsigned char buf[2048];
                int ret = recv(sock, (char*)buf, sizeof(buf), 0);
                if (ret > 0) {
                    int i;
                    for (i = 0; i < ret; i++) {
                        ProcessTelnetByte(buf[i]);
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (WSAGETSELECTEVENT(lParam) == FD_CLOSE) {
                SetStatusText("Disconnected");
                closesocket(sock);
                sock = INVALID_SOCKET;
                EnableWindow(hBtn, TRUE);
                SetWindowTextA(hBtn, "Connect");
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC memDC;
            HBITMAP memBmp, oldBmp;
            HFONT oldFont;
            int termX = dpiScale(5), termY = dpiScale(30);
            int termW = TERM_COLS * dpiScale(8);
            int termH = TERM_ROWS * dpiScale(16);
            int r, c;

            memDC = CreateCompatibleDC(hdc);
            memBmp = CreateCompatibleBitmap(hdc, termW, termH);
            oldBmp = (HBITMAP)SelectObject(memDC, memBmp);
            oldFont = (HFONT)SelectObject(memDC, hTermFont);

            SetBkMode(memDC, OPAQUE);

            for (r = 0; r < TERM_ROWS; r++) {
                for (c = 0; c < TERM_COLS; c++) {
                    char ch = screen[activeTop + r][c].ch;
                    if ((screen[activeTop + r][c].fg & 0x80) && !blinkState) {
                        ch = ' ';
                    }
                    SetTextColor(memDC, ansiColors[screen[activeTop + r][c].fg & 0x0F]);
                    SetBkColor(memDC, ansiColors[screen[activeTop + r][c].bg & 0x0F]);
                    TextOutA(memDC, c * dpiScale(8), r * dpiScale(16), &ch, 1);
                }
            }

            /* Draw cursor block */
            if (scrollOffset == 0 && curX >= 0 && curX < TERM_COLS && curY >= 0 && curY < TERM_ROWS) {
                RECT cursorRect;
                cursorRect.left = curX * dpiScale(8);
                cursorRect.top = curY * dpiScale(16) + dpiScale(14);
                cursorRect.right = curX * dpiScale(8) + dpiScale(8);
                cursorRect.bottom = curY * dpiScale(16) + dpiScale(16);
                InvertRect(memDC, &cursorRect);
            }

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int drawW = clientRect.right - clientRect.left - termX * 2;
            int drawH = clientRect.bottom - clientRect.top - termY - dpiScale(24);
            if (drawW < termW) drawW = termW;
            if (drawH < termH) drawH = termH;
            SetStretchBltMode(hdc, COLORONCOLOR);
            StretchBlt(hdc, termX, termY, drawW, drawH, memDC, 0, 0, termW, termH, SRCCOPY);

            SelectObject(memDC, oldFont);
            SelectObject(memDC, oldBmp);
            DeleteObject(memBmp);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_TIMER: {
            if (wParam == 1) {
                blinkState = !blinkState;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        case WM_SIZE: {
            int cx = LOWORD(lParam);
            int cy = HIWORD(lParam);
            SetWindowPos(hStatus, NULL, dpiScale(5), cy - dpiScale(20), cx - dpiScale(10), dpiScale(18), SWP_NOZORDER);
            SCROLLINFO si;
            si.cbSize = sizeof(si);
            si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
            si.nMin = 0;
            si.nMax = activeTop + TERM_ROWS - 1;
            si.nPage = TERM_ROWS;
            si.nPos = activeTop - scrollOffset;
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case WM_MOUSEWHEEL: {
            int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (zDelta > 0) scrollOffset += 3;
            else scrollOffset -= 3;
            if (scrollOffset > activeTop) scrollOffset = activeTop;
            if (scrollOffset < 0) scrollOffset = 0;
            
            SCROLLINFO si;
            si.cbSize = sizeof(si);
            si.fMask = SIF_POS;
            si.nPos = activeTop - scrollOffset;
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case WM_VSCROLL: {
            int action = LOWORD(wParam);
            if (action == SB_LINEUP) scrollOffset++;
            else if (action == SB_LINEDOWN) scrollOffset--;
            else if (action == SB_PAGEUP) scrollOffset += TERM_ROWS;
            else if (action == SB_PAGEDOWN) scrollOffset -= TERM_ROWS;
            else if (action == SB_THUMBTRACK) {
                SCROLLINFO si;
                si.cbSize = sizeof(si);
                si.fMask = SIF_TRACKPOS;
                GetScrollInfo(hwnd, SB_VERT, &si);
                scrollOffset = activeTop - si.nTrackPos;
            }
            if (scrollOffset > activeTop) scrollOffset = activeTop;
            if (scrollOffset < 0) scrollOffset = 0;
            
            SCROLLINFO si;
            si.cbSize = sizeof(si);
            si.fMask = SIF_POS;
            si.nPos = activeTop - scrollOffset;
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            SetFocus(hwnd);
            return 0;
        }
        case WM_CHAR: {
            if (sock != INVALID_SOCKET && wParam >= 32 && wParam < 127) {
                char ch = (char)wParam;
                scrollOffset = 0;
                send(sock, &ch, 1, 0);
            }
            return 0;
        }
        case WM_KEYDOWN: {
            const char* seq = NULL;
            int seqLen = 0;
            char fkey[6];

            if (sock == INVALID_SOCKET) break;

            switch (wParam) {
                case VK_UP:     seq = "\x1B[A"; seqLen = 3; break;
                case VK_DOWN:   seq = "\x1B[B"; seqLen = 3; break;
                case VK_RIGHT:  seq = "\x1B[C"; seqLen = 3; break;
                case VK_LEFT:   seq = "\x1B[D"; seqLen = 3; break;
                case VK_ESCAPE: seq = "\x1B"; seqLen = 1; break;
                case VK_BACK:   seq = "\x08"; seqLen = 1; break;
                case VK_RETURN: seq = "\r\n"; seqLen = 2; break;
                case VK_TAB:    seq = "\t"; seqLen = 1; break;
                case VK_DELETE: seq = "\x1B[3~"; seqLen = 4; break;
                case VK_HOME:   seq = "\x1B[H"; seqLen = 3; break;
                case VK_END:    seq = "\x1B[F"; seqLen = 3; break;
                case VK_PRIOR:  seq = "\x1B[5~"; seqLen = 4; break;
                case VK_NEXT:   seq = "\x1B[6~"; seqLen = 4; break;
                case VK_F1:     seq = "\x1BOP"; seqLen = 3; break;
                case VK_F2:     seq = "\x1BOQ"; seqLen = 3; break;
                case VK_F3:     seq = "\x1BOR"; seqLen = 3; break;
                case VK_F4:     seq = "\x1BOS"; seqLen = 3; break;
                case VK_F5:     seq = "\x1B[15~"; seqLen = 5; break;
                case VK_F6:     seq = "\x1B[17~"; seqLen = 5; break;
                case VK_F7:     seq = "\x1B[18~"; seqLen = 5; break;
                case VK_F8:     seq = "\x1B[19~"; seqLen = 5; break;
                case VK_F9:     seq = "\x1B[20~"; seqLen = 5; break;
                case VK_F10:    seq = "\x1B[21~"; seqLen = 5; break;
                case VK_F11:    seq = "\x1B[23~"; seqLen = 5; break;
                case VK_F12:    seq = "\x1B[24~"; seqLen = 5; break;
            }
            if (seq && seqLen > 0) {
                send(sock, seq, seqLen, 0);
                return 0;
            }
            break;
        }
        case WM_DESTROY:
            if (sock != INVALID_SOCKET) closesocket(sock);
            if (hTermFont) DeleteObject(hTermFont);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc;
    MSG msg;
    int winW, winH;
    typedef BOOL (WINAPI *SetProcessDPIAwareFunc)(void);
    SetProcessDPIAwareFunc setDpi = (SetProcessDPIAwareFunc)GetProcAddress(GetModuleHandleA("user32.dll"), "SetProcessDPIAware");
    if (setDpi) setDpi();

    my_memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KBBSClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hIcon = LoadIconA(wc.hInstance, MAKEINTRESOURCEA(1));

    RegisterClassA(&wc);

    /* Terminal: 80*8=640 wide, 25*16=400 tall. Plus padding and bars. */
    winW = dpiScale(650);
    winH = dpiScale(550);

    hMain = CreateWindowExA(0, "KBBSClass", "KBBS",
        WS_OVERLAPPEDWINDOW | WS_VSCROLL,
        CW_USEDEFAULT, CW_USEDEFAULT, winW, winH,
        NULL, NULL, wc.hInstance, NULL);

    ShowWindow(hMain, SW_SHOW);
    RECT cr;
    GetClientRect(hMain, &cr);
    PostMessage(hMain, WM_SIZE, 0, MAKELPARAM(cr.right, cr.bottom));
    UpdateWindow(hMain);

    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
