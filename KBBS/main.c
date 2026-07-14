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
#define STATE_MUSIC  3

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
    0x000000, 0x0000AA, 0x00AA00, 0x0055AA,  /* black, red, green, brown */
    0xAA0000, 0xAA00AA, 0xAAAA00, 0xAAAAAA,  /* blue, magenta, cyan, light gray */
    0x555555, 0x5555FF, 0x55FF55, 0x55FFFF,  /* dark gray, bright red, bright green, bright yellow */
    0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF   /* bright blue, bright magenta, bright cyan, white */
};

const WCHAR cp437[256] = {
    0x0020, 0x263A, 0x263B, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022, 0x25D8, 0x25CB, 0x25D9, 0x2642, 0x2640, 0x266A, 0x266B, 0x263C,
    0x25BA, 0x25C4, 0x2195, 0x203C, 0x00B6, 0x00A7, 0x25AC, 0x21A8, 0x2191, 0x2193, 0x2192, 0x2190, 0x221F, 0x2194, 0x25B2, 0x25BC,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x2302,
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7, 0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
    0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9, 0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x20A7, 0x0192,
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA, 0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556, 0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F, 0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B, 0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
    0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x03BC, 0x03C4, 0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
    0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248, 0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0
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
char ansiMusicBuf[1024];
int ansiMusicLen = 0;

int telState = TEL_STATE_NORMAL;
unsigned char telSBOption = 0;
unsigned char telSBData[64];
int telSBLen = 0;

SOCKET sock = INVALID_SOCKET;
HWND hMain, hHost, hPort, hBtn, hCombo, hEcho, hStatus;
HWND hBtnMacros;
HFONT hTermFont;
int bytesRx = 0;
int bytesTx = 0;

struct MusicArgs {
    char data[1024];
};

int GetFreq(const char* noteStr, int octave) {
    int baseFreqs[] = {
        26163, 27718, 29366, 31113, 32963, 34923,
        36999, 39200, 41530, 44000, 46616, 49388
    };
    int base = 0;
    char n = noteStr[0];
    if (n == 'c' || n == 'C') base = 0;
    else if (n == 'd' || n == 'D') base = 2;
    else if (n == 'e' || n == 'E') base = 4;
    else if (n == 'f' || n == 'F') base = 5;
    else if (n == 'g' || n == 'G') base = 7;
    else if (n == 'a' || n == 'A') base = 9;
    else if (n == 'b' || n == 'B') base = 11;

    if (noteStr[1] == '+' || noteStr[1] == '#') base++;
    else if (noteStr[1] == '-') base--;

    if (base < 0) { base += 12; octave--; }
    else if (base > 11) { base -= 12; octave++; }
    
    int f = baseFreqs[base];
    while (octave > 4) { f *= 2; octave--; }
    while (octave < 4) { f /= 2; octave++; }
    return f / 100;
}

DWORD WINAPI MusicThread(LPVOID lpParam) {
    struct MusicArgs* args = (struct MusicArgs*)lpParam;
    char* str = args->data;
    int octave = 4, tempo = 120, length = 4;
    int p = 0;
    while (str[p]) {
        char c = str[p++];
        if (c >= 'a' && c <= 'z') c -= 32;
        if (c == 'O') {
            int val = 0;
            while (str[p] >= '0' && str[p] <= '9') val = val * 10 + (str[p++] - '0');
            if (val > 0) octave = val;
        } else if (c == 'T') {
            int val = 0;
            while (str[p] >= '0' && str[p] <= '9') val = val * 10 + (str[p++] - '0');
            if (val > 0) tempo = val;
        } else if (c == 'L') {
            int val = 0;
            while (str[p] >= '0' && str[p] <= '9') val = val * 10 + (str[p++] - '0');
            if (val > 0) length = val;
        } else if ((c >= 'A' && c <= 'G') || c == 'P') {
            char note[3] = {c, 0, 0};
            if (str[p] == '+' || str[p] == '#' || str[p] == '-') note[1] = str[p++];
            int curLen = length;
            if (str[p] >= '0' && str[p] <= '9') {
                curLen = 0;
                while (str[p] >= '0' && str[p] <= '9') curLen = curLen * 10 + (str[p++] - '0');
            }
            int dots = 0;
            while (str[p] == '.') { dots++; p++; }
            int durMs = (60000 / tempo) * 4 / (curLen ? curLen : 4);
            int i;
            for (i=0; i<dots; i++) durMs += durMs / 2;
            if (c != 'P') {
                int freq = GetFreq(note, octave);
                Beep(freq, durMs);
            } else {
                Sleep(durMs);
            }
        } else if (c == 'M') {
            if (str[p]) p++;
        } else if (c == '<') {
            if (octave > 0) octave--;
        } else if (c == '>') {
            if (octave < 6) octave++;
        }
    }
    GlobalFree(args);
    return 0;
}

void PlayANSI(const char* str) {
    struct MusicArgs* args = (struct MusicArgs*)GlobalAlloc(GPTR, sizeof(struct MusicArgs));
    if (args) {
        my_strcpy(args->data, str);
        CreateThread(NULL, 0, MusicThread, args, 0, NULL);
    }
}

void PlayChime(int type) {
    if (type == 1) { /* connect */
        CreateThread(NULL, 0, MusicThread, NULL, 0, NULL); // Hack: avoid using this just call Beep in new thread? No.
    }
}
// wait let's write a proper PlayChimeThread
DWORD WINAPI ChimeThread(LPVOID lpParam) {
    int type = (int)(INT_PTR)lpParam;
    if (type == 1) { Beep(523, 100); Beep(659, 100); Beep(784, 200); }
    else if (type == 2) { Beep(784, 100); Beep(1046, 200); }
    return 0;
}
void PlayChimeAsync(int type) {
    CreateThread(NULL, 0, ChimeThread, (LPVOID)(INT_PTR)type, 0, NULL);
}

/* Transfer state */
#define XMODEM_SOH 0x01
#define XMODEM_EOT 0x04
#define XMODEM_ACK 0x06
#define XMODEM_NAK 0x15
#define XMODEM_CAN 0x18
#define XMODEM_SUB 0x1A

int transferActive = 0; /* 1=XmodemDL, 2=XmodemUL */
int transferState = 0;
HANDLE hTransferFile = INVALID_HANDLE_VALUE;
int transferBlockNum = 1;
int transferByteCount = 0;
unsigned char transferBuf[132];
int transferBytesTotal = 0;
char transferStatusMsg[128] = "";

HWND hBtnXmDl, hBtnXmUl;

void EndTransfer(const char* msg) {
    if (hTransferFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hTransferFile);
        hTransferFile = INVALID_HANDLE_VALUE;
    }
    transferActive = 0;
    SetStatusText(msg);
    InvalidateRect(hMain, NULL, FALSE);
    
    // Check if Complete
    int i = 0, isComp = 0;
    while(msg[i]) { if (msg[i] == 'C' && msg[i+1] == 'o' && msg[i+2] == 'm') isComp = 1; i++; }
    if (isComp) PlayChimeAsync(2);
}

void ProcessTransferByte(unsigned char ch) {
    if (transferActive == 1) { /* Xmodem DL */
        if (transferState == 0) { /* Wait for SOH, EOT, or CAN */
            if (ch == XMODEM_SOH) {
                transferState = 1;
                transferByteCount = 0;
                transferBuf[transferByteCount++] = ch;
            } else if (ch == XMODEM_EOT) {
                unsigned char ack = XMODEM_ACK;
                send(sock, (char*)&ack, 1, 0);
                EndTransfer("Download Complete");
            } else if (ch == XMODEM_CAN) {
                EndTransfer("Download Cancelled by Host");
            }
        } else if (transferState == 1) { /* Reading block */
            transferBuf[transferByteCount++] = ch;
            if (transferByteCount == 132) {
                int blk = transferBuf[1];
                int inv = transferBuf[2];
                if (blk == (255 - inv) && blk == (transferBlockNum & 0xFF)) {
                    unsigned char csum = 0;
                    for (int i = 3; i < 131; i++) csum += transferBuf[i];
                    if (csum == transferBuf[131]) {
                        DWORD written;
                        WriteFile(hTransferFile, transferBuf + 3, 128, &written, NULL);
                        transferBytesTotal += 128;
                        wsprintfA(transferStatusMsg, "Downloading... %d bytes", transferBytesTotal);
                        transferBlockNum++;
                        unsigned char ack = XMODEM_ACK;
                        send(sock, (char*)&ack, 1, 0);
                        transferState = 0;
                        InvalidateRect(hMain, NULL, FALSE);
                        return;
                    }
                }
                /* Error or bad csum */
                unsigned char nak = XMODEM_NAK;
                send(sock, (char*)&nak, 1, 0);
                transferState = 0;
            }
        }
    } else if (transferActive == 2) { /* Xmodem UL */
        if (transferState == 0) { /* Wait for NAK to start */
            if (ch == XMODEM_NAK || ch == 'C') {
                transferState = 1; /* Sending */
                goto send_block;
            } else if (ch == XMODEM_CAN) {
                EndTransfer("Upload Cancelled");
            }
        } else if (transferState == 1) { /* Wait for ACK/NAK */
            if (ch == XMODEM_ACK) {
                transferBlockNum++;
send_block:
                {
                    unsigned char buf[132];
                    buf[0] = XMODEM_SOH;
                    buf[1] = transferBlockNum & 0xFF;
                    buf[2] = 255 - buf[1];
                    DWORD readBytes = 0;
                    ReadFile(hTransferFile, buf + 3, 128, &readBytes, NULL);
                    if (readBytes == 0) { /* EOF */
                        unsigned char eot = XMODEM_EOT;
                        send(sock, (char*)&eot, 1, 0);
                        transferState = 2; /* Wait for EOT ACK */
                        return;
                    }
                    for (DWORD i = readBytes; i < 128; i++) buf[3 + i] = XMODEM_SUB;
                    unsigned char csum = 0;
                    for (int i = 3; i < 131; i++) csum += buf[i];
                    buf[131] = csum;
                    /* Send over telnet (escape 0xFF) */
                    for (int i = 0; i < 132; i++) {
                        send(sock, (char*)&buf[i], 1, 0);
                        if (buf[i] == 0xFF) send(sock, (char*)&buf[i], 1, 0);
                    }
                    transferBytesTotal += readBytes;
                    wsprintfA(transferStatusMsg, "Uploading... %d bytes", transferBytesTotal);
                    InvalidateRect(hMain, NULL, FALSE);
                }
            } else if (ch == XMODEM_NAK) {
                /* Resend */
                SetFilePointer(hTransferFile, -128, NULL, FILE_CURRENT);
                goto send_block;
            } else if (ch == XMODEM_CAN) {
                EndTransfer("Upload Cancelled");
            }
        } else if (transferState == 2) {
            if (ch == XMODEM_ACK) {
                EndTransfer("Upload Complete");
            }
        }
    }
}

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

#define IDD_DIALDIR 1000
#define IDC_LIST    1001
#define IDC_NAME    1002
#define IDC_HOST    1003
#define IDC_PORT    1004
#define IDC_TYPE    1005
#define IDC_SAVE    1006
#define IDC_NEW     1007
#define IDC_DELETE  1008

#define MAX_BBS 100
struct BBS_ENTRY {
    char name[64];
    char host[128];
    int port;
    char type[32];
};
struct BBS_ENTRY dynBbsList[MAX_BBS];
int numBbs = 0;
int selectedDirIdx = -1;

void LoadBBSList(void) {
    HANDLE hFile = CreateFileA("kbbs_dir.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    numBbs = 0;
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD readBytes;
        char buf[8192];
        if (ReadFile(hFile, buf, sizeof(buf)-1, &readBytes, NULL)) {
            buf[readBytes] = 0;
            char* line = buf;
            while (*line && numBbs < MAX_BBS) {
                char* next = line;
                while (*next && *next != '\n' && *next != '\r') next++;
                char term = *next;
                *next = 0;
                if (line[0]) {
                    char* p1 = line;
                    char* p2 = p1; while(*p2 && *p2 != '|') p2++; if (*p2) { *p2++ = 0; }
                    char* p3 = p2; while(*p3 && *p3 != '|') p3++; if (*p3) { *p3++ = 0; }
                    char* p4 = p3; while(*p4 && *p4 != '|') p4++; if (*p4) { *p4++ = 0; }
                    
                    if (*p1 && *p2 && *p3) {
                        my_strcpy(dynBbsList[numBbs].name, p1);
                        my_strcpy(dynBbsList[numBbs].host, p2);
                        dynBbsList[numBbs].port = my_atoi(p3);
                        my_strcpy(dynBbsList[numBbs].type, *p4 ? p4 : "Telnet");
                        numBbs++;
                    }
                }
                line = next;
                if (term == '\r' && *(line+1) == '\n') line += 2;
                else if (term != 0) line++;
            }
        }
        CloseHandle(hFile);
    } 
    if (numBbs == 0) {
        int i;
        for(i = 0; i < (int)BBS_COUNT && i < MAX_BBS; i++) {
            my_strcpy(dynBbsList[numBbs].name, bbsList[i].name);
            my_strcpy(dynBbsList[numBbs].host, bbsList[i].host);
            dynBbsList[numBbs].port = bbsList[i].port;
            my_strcpy(dynBbsList[numBbs].type, "Telnet");
            numBbs++;
        }
    }
}

void SaveBBSList(void) {
    HANDLE hFile = CreateFileA("kbbs_dir.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        int i;
        char buf[512];
        DWORD written;
        for (i = 0; i < numBbs; i++) {
            char portStr[16];
            my_itoa(dynBbsList[i].port, portStr);
            my_strcpy(buf, dynBbsList[i].name);
            my_strcpy(buf + my_strlen(buf), "|");
            my_strcpy(buf + my_strlen(buf), dynBbsList[i].host);
            my_strcpy(buf + my_strlen(buf), "|");
            my_strcpy(buf + my_strlen(buf), portStr);
            my_strcpy(buf + my_strlen(buf), "|");
            my_strcpy(buf + my_strlen(buf), dynBbsList[i].type);
            my_strcpy(buf + my_strlen(buf), "\r\n");
            WriteFile(hFile, buf, (DWORD)my_strlen(buf), &written, NULL);
        }
        CloseHandle(hFile);
    }
}

INT_PTR CALLBACK DialDirProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_INITDIALOG: {
            HWND hList = GetDlgItem(hdlg, IDC_LIST);
            HWND hType = GetDlgItem(hdlg, IDC_TYPE);
            int i;
            SendMessageA(hType, CB_ADDSTRING, 0, (LPARAM)"Telnet");
            SendMessageA(hType, CB_ADDSTRING, 0, (LPARAM)"WebSocket");
            
            LoadBBSList();
            for (i = 0; i < numBbs; i++) {
                SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)dynBbsList[i].name);
            }
            selectedDirIdx = 0;
            if (numBbs > 0) {
                SendMessageA(hList, LB_SETCURSEL, selectedDirIdx, 0);
                SetDlgItemTextA(hdlg, IDC_NAME, dynBbsList[0].name);
                SetDlgItemTextA(hdlg, IDC_HOST, dynBbsList[0].host);
                char pBuf[16]; my_itoa(dynBbsList[0].port, pBuf);
                SetDlgItemTextA(hdlg, IDC_PORT, pBuf);
                SendMessageA(hType, CB_SETCURSEL, dynBbsList[0].type[0]=='W' ? 1 : 0, 0);
            }
            return TRUE;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);
            if (id == IDC_LIST && code == LBN_SELCHANGE) {
                int sel = (int)SendMessageA(GetDlgItem(hdlg, IDC_LIST), LB_GETCURSEL, 0, 0);
                if (sel >= 0 && sel < numBbs) {
                    selectedDirIdx = sel;
                    SetDlgItemTextA(hdlg, IDC_NAME, dynBbsList[sel].name);
                    SetDlgItemTextA(hdlg, IDC_HOST, dynBbsList[sel].host);
                    char pBuf[16]; my_itoa(dynBbsList[sel].port, pBuf);
                    SetDlgItemTextA(hdlg, IDC_PORT, pBuf);
                    SendMessageA(GetDlgItem(hdlg, IDC_TYPE), CB_SETCURSEL, dynBbsList[sel].type[0]=='W' ? 1 : 0, 0);
                }
            } else if (id == IDC_NEW) {
                selectedDirIdx = -1;
                SendMessageA(GetDlgItem(hdlg, IDC_LIST), LB_SETCURSEL, (WPARAM)-1, 0);
                SetDlgItemTextA(hdlg, IDC_NAME, "New BBS");
                SetDlgItemTextA(hdlg, IDC_HOST, "");
                SetDlgItemTextA(hdlg, IDC_PORT, "23");
                SendMessageA(GetDlgItem(hdlg, IDC_TYPE), CB_SETCURSEL, 0, 0);
            } else if (id == IDC_SAVE) {
                char name[64], host[128], port[16];
                GetDlgItemTextA(hdlg, IDC_NAME, name, 64);
                GetDlgItemTextA(hdlg, IDC_HOST, host, 128);
                GetDlgItemTextA(hdlg, IDC_PORT, port, 16);
                int typeSel = (int)SendMessageA(GetDlgItem(hdlg, IDC_TYPE), CB_GETCURSEL, 0, 0);
                if (name[0] && host[0]) {
                    int target = selectedDirIdx;
                    if (target < 0) {
                        target = numBbs;
                        if (numBbs < MAX_BBS) numBbs++;
                        else target = MAX_BBS - 1;
                    }
                    my_strcpy(dynBbsList[target].name, name);
                    my_strcpy(dynBbsList[target].host, host);
                    dynBbsList[target].port = my_atoi(port);
                    my_strcpy(dynBbsList[target].type, typeSel == 1 ? "WebSocket" : "Telnet");
                    
                    HWND hList = GetDlgItem(hdlg, IDC_LIST);
                    SendMessageA(hList, LB_RESETCONTENT, 0, 0);
                    int i;
                    for (i = 0; i < numBbs; i++) SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)dynBbsList[i].name);
                    SendMessageA(hList, LB_SETCURSEL, target, 0);
                    selectedDirIdx = target;
                    SaveBBSList();
                }
            } else if (id == IDC_DELETE) {
                if (selectedDirIdx >= 0 && selectedDirIdx < numBbs) {
                    int i;
                    for (i = selectedDirIdx; i < numBbs - 1; i++) {
                        dynBbsList[i] = dynBbsList[i+1];
                    }
                    numBbs--;
                    selectedDirIdx = -1;
                    HWND hList = GetDlgItem(hdlg, IDC_LIST);
                    SendMessageA(hList, LB_RESETCONTENT, 0, 0);
                    int j;
                    for (j = 0; j < numBbs; j++) SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)dynBbsList[j].name);
                    SaveBBSList();
                }
            } else if (id == 1) { // Connect
                if (selectedDirIdx >= 0 && selectedDirIdx < numBbs) {
                    EndDialog(hdlg, selectedDirIdx);
                } else {
                    EndDialog(hdlg, -1);
                }
            } else if (id == 2) { // Cancel
                EndDialog(hdlg, -1);
            }
            return TRUE;
        }
    }
    return FALSE;
}

#define IDD_MACROS   2000
#define IDC_MACRO_LIST 2001
#define IDC_MACRO_KEY  2002
#define IDC_MACRO_STR  2003
#define IDC_MACRO_SAVE 2006
#define IDC_MACRO_NEW  2007
#define IDC_MACRO_DEL  2008

struct MACRO_ENTRY {
    char key;
    char str[1024];
};

struct MACRO_ENTRY dynMacros[MAX_BBS];
int numMacros = 0;
int selectedMacroIdx = -1;

void LoadMacros(void) {
    HANDLE hFile = CreateFileA("kbbs_macros.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    numMacros = 0;
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD readBytes;
        char buf[8192];
        if (ReadFile(hFile, buf, sizeof(buf)-1, &readBytes, NULL)) {
            buf[readBytes] = 0;
            char* line = buf;
            while (*line && numMacros < MAX_BBS) {
                char* next = line;
                while (*next && *next != '\n' && *next != '\r') next++;
                char term = *next;
                *next = 0;
                if (line[0]) {
                    char* p1 = line;
                    char* p2 = p1; while(*p2 && *p2 != '|') p2++; if (*p2) { *p2++ = 0; }
                    
                    if (*p1 && *p2) {
                        dynMacros[numMacros].key = p1[0];
                        my_strcpy(dynMacros[numMacros].str, p2);
                        numMacros++;
                    }
                }
                line = next;
                if (term == '\r' && *(line+1) == '\n') line += 2;
                else if (term != 0) line++;
            }
        }
        CloseHandle(hFile);
    }
}

void SaveMacros(void) {
    HANDLE hFile = CreateFileA("kbbs_macros.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        int i;
        char buf[2048];
        DWORD written;
        for (i = 0; i < numMacros; i++) {
            char kStr[2] = {dynMacros[i].key, 0};
            my_strcpy(buf, kStr);
            my_strcpy(buf + my_strlen(buf), "|");
            my_strcpy(buf + my_strlen(buf), dynMacros[i].str);
            my_strcpy(buf + my_strlen(buf), "\r\n");
            WriteFile(hFile, buf, (DWORD)my_strlen(buf), &written, NULL);
        }
        CloseHandle(hFile);
    }
}

void UpdateMacroListUI(HWND hList) {
    SendMessageA(hList, LB_RESETCONTENT, 0, 0);
    int i;
    for (i = 0; i < numMacros; i++) {
        char item[128];
        wsprintfA(item, "Alt + %c", dynMacros[i].key);
        SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)item);
    }
}

INT_PTR CALLBACK MacrosProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_INITDIALOG: {
            HWND hList = GetDlgItem(hdlg, IDC_MACRO_LIST);
            LoadMacros();
            UpdateMacroListUI(hList);
            selectedMacroIdx = 0;
            if (numMacros > 0) {
                SendMessageA(hList, LB_SETCURSEL, selectedMacroIdx, 0);
                char kStr[2] = {dynMacros[0].key, 0};
                SetDlgItemTextA(hdlg, IDC_MACRO_KEY, kStr);
                SetDlgItemTextA(hdlg, IDC_MACRO_STR, dynMacros[0].str);
            }
            return TRUE;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);
            if (id == IDC_MACRO_LIST && code == LBN_SELCHANGE) {
                int sel = (int)SendMessageA(GetDlgItem(hdlg, IDC_MACRO_LIST), LB_GETCURSEL, 0, 0);
                if (sel >= 0 && sel < numMacros) {
                    selectedMacroIdx = sel;
                    char kStr[2] = {dynMacros[sel].key, 0};
                    SetDlgItemTextA(hdlg, IDC_MACRO_KEY, kStr);
                    SetDlgItemTextA(hdlg, IDC_MACRO_STR, dynMacros[sel].str);
                }
            } else if (id == IDC_MACRO_NEW) {
                selectedMacroIdx = -1;
                SendMessageA(GetDlgItem(hdlg, IDC_MACRO_LIST), LB_SETCURSEL, (WPARAM)-1, 0);
                SetDlgItemTextA(hdlg, IDC_MACRO_KEY, "");
                SetDlgItemTextA(hdlg, IDC_MACRO_STR, "");
            } else if (id == IDC_MACRO_SAVE) {
                char keyBuf[16], strBuf[1024];
                GetDlgItemTextA(hdlg, IDC_MACRO_KEY, keyBuf, 16);
                GetDlgItemTextA(hdlg, IDC_MACRO_STR, strBuf, 1024);
                char k = keyBuf[0];
                if (k >= 'a' && k <= 'z') k -= 32;
                if ((k >= 'A' && k <= 'Z') || (k >= '0' && k <= '9')) {
                    int target = selectedMacroIdx;
                    if (target < 0) {
                        int i;
                        int exists = -1;
                        for(i=0; i<numMacros; i++) {
                            if (dynMacros[i].key == k) { exists = i; break; }
                        }
                        if (exists >= 0) target = exists;
                        else {
                            target = numMacros;
                            if (numMacros < MAX_BBS) numMacros++;
                            else target = MAX_BBS - 1;
                        }
                    }
                    dynMacros[target].key = k;
                    my_strcpy(dynMacros[target].str, strBuf);
                    
                    HWND hList = GetDlgItem(hdlg, IDC_MACRO_LIST);
                    UpdateMacroListUI(hList);
                    SendMessageA(hList, LB_SETCURSEL, target, 0);
                    selectedMacroIdx = target;
                    SaveMacros();
                }
            } else if (id == IDC_MACRO_DEL) {
                if (selectedMacroIdx >= 0 && selectedMacroIdx < numMacros) {
                    int i;
                    for (i = selectedMacroIdx; i < numMacros - 1; i++) {
                        dynMacros[i] = dynMacros[i+1];
                    }
                    numMacros--;
                    selectedMacroIdx = -1;
                    HWND hList = GetDlgItem(hdlg, IDC_MACRO_LIST);
                    UpdateMacroListUI(hList);
                    SaveMacros();
                }
            } else if (id == 2) {
                EndDialog(hdlg, -1);
            }
            return TRUE;
        }
    }
    return FALSE;
}

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
        if (scrollOffset > activeTop) scrollOffset = activeTop;
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
        case 'M': /* ANSI Music */
            ansiState = STATE_MUSIC;
            ansiMusicLen = 0;
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
                if (ansiState != STATE_MUSIC) ansiState = STATE_NORMAL;
            } else {
                ansiState = STATE_NORMAL;
            }
            break;
        case STATE_MUSIC:
            if (ch == 0x0E) { /* Ctrl+N */
                ansiMusicBuf[ansiMusicLen] = 0;
                PlayANSI(ansiMusicBuf);
                ansiState = STATE_NORMAL;
            } else if (ch == 0x1B) {
                ansiMusicBuf[ansiMusicLen] = 0;
                PlayANSI(ansiMusicBuf);
                ansiState = STATE_ESC;
            } else {
                if (ansiMusicLen < (int)(sizeof(ansiMusicBuf) - 1)) {
                    ansiMusicBuf[ansiMusicLen++] = (char)ch;
                }
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
                if (transferActive) ProcessTransferByte(ch);
                else ProcessByte(ch);
            }
            break;
        case TEL_STATE_IAC:
            if (ch == TELNET_WILL) { telState = TEL_STATE_WILL; }
            else if (ch == TELNET_WONT) { telState = TEL_STATE_WONT; }
            else if (ch == TELNET_DO) { telState = TEL_STATE_DO; }
            else if (ch == TELNET_DONT) { telState = TEL_STATE_DONT; }
            else if (ch == TELNET_SB) { telState = TEL_STATE_SB; }
            else if (ch == TELNET_IAC) { 
                if (transferActive) ProcessTransferByte(TELNET_IAC);
                else ProcessByte(TELNET_IAC); 
                telState = TEL_STATE_NORMAL; 
            }
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

const char* lastStatus = "Disconnected";
void UpdateStatus(void) {
    char buf[128];
    wsprintfA(buf, "%s | RX: %d B | TX: %d B", lastStatus, bytesRx, bytesTx);
    SetWindowTextA(hStatus, buf);
}

void SetStatusText(const char* text) {
    lastStatus = text;
    UpdateStatus();
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

            hCombo = CreateWindowA("BUTTON", "Directory", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                dpiScale(428), dpiScale(4), dpiScale(70), dpiScale(22), hwnd, (HMENU)101, 0, 0);

            hBtnMacros = CreateWindowA("BUTTON", "Macros", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                dpiScale(502), dpiScale(4), dpiScale(60), dpiScale(22), hwnd, (HMENU)109, 0, 0);

            hBtnXmDl = CreateWindowA("BUTTON", "DL (XMODEM)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                dpiScale(566), dpiScale(4), dpiScale(80), dpiScale(22), hwnd, (HMENU)103, 0, 0);

            hBtnXmUl = CreateWindowA("BUTTON", "UL (XMODEM)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                dpiScale(650), dpiScale(4), dpiScale(80), dpiScale(22), hwnd, (HMENU)104, 0, 0);
            
            hEcho = CreateWindowA("BUTTON", "Echo", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                dpiScale(734), dpiScale(4), dpiScale(52), dpiScale(22), hwnd, (HMENU)102, 0, 0);

            /* Status bar */
            hStatus = CreateWindowA("STATIC", "Disconnected | RX: 0 B | TX: 0 B", WS_CHILD | WS_VISIBLE | SS_LEFT,
                dpiScale(5), dpiScale(30 + TERM_ROWS * 16 + 4), dpiScale(640), dpiScale(18), hwnd, 0, 0, 0);

            /* Set fonts */
            SendMessageA(hHost, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hPort, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnMacros, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnXmDl, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnXmUl, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hEcho, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hStatus, WM_SETFONT, (WPARAM)hFont, TRUE);

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
                bytesRx = 0;
                bytesTx = 0;
                InvalidateRect(hwnd, NULL, FALSE);
                RECT cr;
                GetClientRect(hwnd, &cr);
                PostMessage(hwnd, WM_SIZE, 0, MAKELPARAM(cr.right, cr.bottom));
            }
            else if (LOWORD(wParam) == 101) {
                /* Directory button */
                int res = DialogBoxParamA(GetModuleHandleA(NULL), MAKEINTRESOURCEA(IDD_DIALDIR), hwnd, DialDirProc, 0);
                if (res >= 0 && res < numBbs) {
                    char portBuf[16];
                    SetWindowTextA(hHost, dynBbsList[res].host);
                    my_itoa(dynBbsList[res].port, portBuf);
                    SetWindowTextA(hPort, portBuf);
                    SendMessage(hwnd, WM_COMMAND, 100, 0);
                }
            } else if (LOWORD(wParam) == 109) {
                DialogBoxParamA(GetModuleHandleA(NULL), MAKEINTRESOURCEA(IDD_MACROS), hwnd, MacrosProc, 0);
            } else if (LOWORD(wParam) == 103) {
                /* XMODEM DL */
                if (sock != INVALID_SOCKET && !transferActive) {
                    OPENFILENAMEA ofn = {0};
                    char szFile[260] = {0};
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = szFile;
                    ofn.nMaxFile = sizeof(szFile);
                    ofn.lpstrFilter = "All Files\0*.*\0";
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
                    if (GetSaveFileNameA(&ofn)) {
                        hTransferFile = CreateFileA(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                        if (hTransferFile != INVALID_HANDLE_VALUE) {
                            transferActive = 1;
                            transferState = 0;
                            transferBlockNum = 1;
                            transferBytesTotal = 0;
                            wsprintfA(transferStatusMsg, "Waiting for host...");
                            unsigned char nak = XMODEM_NAK;
                            send(sock, (char*)&nak, 1, 0);
                            InvalidateRect(hwnd, NULL, FALSE);
                        }
                    }
                }
            } else if (LOWORD(wParam) == 104) {
                /* XMODEM UL */
                if (sock != INVALID_SOCKET && !transferActive) {
                    OPENFILENAMEA ofn = {0};
                    char szFile[260] = {0};
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = szFile;
                    ofn.nMaxFile = sizeof(szFile);
                    ofn.lpstrFilter = "All Files\0*.*\0";
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                    if (GetOpenFileNameA(&ofn)) {
                        hTransferFile = CreateFileA(ofn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                        if (hTransferFile != INVALID_HANDLE_VALUE) {
                            transferActive = 2;
                            transferState = 0;
                            transferBlockNum = 1;
                            transferBytesTotal = 0;
                            wsprintfA(transferStatusMsg, "Waiting for NAK...");
                            InvalidateRect(hwnd, NULL, FALSE);
                        }
                    }
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
                PlayChimeAsync(1);
            } else if (WSAGETSELECTEVENT(lParam) == FD_READ) {
                unsigned char buf[2048];
                int ret = recv(sock, (char*)buf, sizeof(buf), 0);
                if (ret > 0) {
                    int i;
                    bytesRx += ret;
                    UpdateStatus();
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
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int cx = clientRect.right - clientRect.left;
            int cy = clientRect.bottom - clientRect.top;

            HDC winDC = CreateCompatibleDC(hdc);
            HBITMAP winBmp = CreateCompatibleBitmap(hdc, cx, cy);
            HBITMAP oldWinBmp = (HBITMAP)SelectObject(winDC, winBmp);

            HBRUSH hBg = CreateSolidBrush(RGB(9, 9, 11));
            FillRect(winDC, &clientRect, hBg);
            DeleteObject(hBg);

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
            
            int viewTop = activeTop - scrollOffset;
            for (r = 0; r < TERM_ROWS; r++) {
                for (c = 0; c < TERM_COLS; c++) {
                    unsigned char raw = (unsigned char)screen[viewTop + r][c].ch;
                    WCHAR wch = cp437[raw];
                    if ((screen[viewTop + r][c].fg & 0x80) && !blinkState) {
                        wch = L' ';
                    }
                    COLORREF fgColor = ansiColors[screen[viewTop + r][c].fg & 0x0F];
                    COLORREF bgColor = ansiColors[screen[viewTop + r][c].bg & 0x0F];
                    
                    SetTextColor(memDC, fgColor);
                    SetBkColor(memDC, bgColor);
                    
                    int x = c * dpiScale(8);
                    int y = r * dpiScale(16);
                    int w = dpiScale(8);
                    int h = dpiScale(16);
                    
                    if (raw == 0xDB) { /* Full block */
                        RECT rBlock = { x, y, x + w, y + h };
                        HBRUSH hBr = CreateSolidBrush(fgColor);
                        FillRect(memDC, &rBlock, hBr);
                        DeleteObject(hBr);
                    } else if (raw == 0xDC) { /* Lower half block */
                        RECT rBlock = { x, y, x + w, y + h };
                        HBRUSH hBg = CreateSolidBrush(bgColor);
                        FillRect(memDC, &rBlock, hBg);
                        DeleteObject(hBg);
                        rBlock.top = y + h / 2;
                        HBRUSH hFg = CreateSolidBrush(fgColor);
                        FillRect(memDC, &rBlock, hFg);
                        DeleteObject(hFg);
                    } else if (raw == 0xDF) { /* Upper half block */
                        RECT rBlock = { x, y, x + w, y + h };
                        HBRUSH hBg = CreateSolidBrush(bgColor);
                        FillRect(memDC, &rBlock, hBg);
                        DeleteObject(hBg);
                        rBlock.bottom = y + h / 2;
                        HBRUSH hFg = CreateSolidBrush(fgColor);
                        FillRect(memDC, &rBlock, hFg);
                        DeleteObject(hFg);
                    } else if (raw == 0xDD) { /* Left half block */
                        RECT rBlock = { x, y, x + w, y + h };
                        HBRUSH hBg = CreateSolidBrush(bgColor);
                        FillRect(memDC, &rBlock, hBg);
                        DeleteObject(hBg);
                        rBlock.right = x + w / 2;
                        HBRUSH hFg = CreateSolidBrush(fgColor);
                        FillRect(memDC, &rBlock, hFg);
                        DeleteObject(hFg);
                    } else if (raw == 0xDE) { /* Right half block */
                        RECT rBlock = { x, y, x + w, y + h };
                        HBRUSH hBg = CreateSolidBrush(bgColor);
                        FillRect(memDC, &rBlock, hBg);
                        DeleteObject(hBg);
                        rBlock.left = x + w / 2;
                        HBRUSH hFg = CreateSolidBrush(fgColor);
                        FillRect(memDC, &rBlock, hFg);
                        DeleteObject(hFg);
                    } else if (raw >= 0xB0 && raw <= 0xB2) { /* Shades */
                        /* We simulate shades via alternating pixels, but standard TextOutW is often better and less complex */
                        TextOutW(memDC, x, y, &wch, 1);
                    } else {
                        if (raw != ' ' && raw != 0) {
                            TextOutW(memDC, x, y, &wch, 1);
                        } else {
                            RECT rBlock = { x, y, x + w, y + h };
                            HBRUSH hBg = CreateSolidBrush(bgColor);
                            FillRect(memDC, &rBlock, hBg);
                            DeleteObject(hBg);
                        }
                    }
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
            SetStretchBltMode(winDC, COLORONCOLOR);
            StretchBlt(winDC, termX, termY, drawW, drawH, memDC, 0, 0, termW, termH, SRCCOPY);

            /* Draw transfer overlay if active */
            if (transferActive) {
                HFONT uiFont = CreateFontA(dpiScale(14), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                    DEFAULT_PITCH | FF_SWISS, "Tahoma");
                HFONT oldUIFont = (HFONT)SelectObject(winDC, uiFont);
                
                RECT rOverlay = { clientRect.right/2 - dpiScale(120), clientRect.bottom/2 - dpiScale(40), clientRect.right/2 + dpiScale(120), clientRect.bottom/2 + dpiScale(40) };
                HBRUSH br = CreateSolidBrush(RGB(24, 24, 27));
                FillRect(winDC, &rOverlay, br);
                DeleteObject(br);
                
                HPEN pen = CreatePen(PS_SOLID, 1, RGB(14, 165, 233));
                HPEN oldPen = (HPEN)SelectObject(winDC, pen);
                MoveToEx(winDC, rOverlay.left, rOverlay.top, NULL); LineTo(winDC, rOverlay.right, rOverlay.top);
                LineTo(winDC, rOverlay.right, rOverlay.bottom); LineTo(winDC, rOverlay.left, rOverlay.bottom); LineTo(winDC, rOverlay.left, rOverlay.top);
                SelectObject(winDC, oldPen);
                DeleteObject(pen);

                SetTextColor(winDC, RGB(255, 255, 255));
                SetBkMode(winDC, TRANSPARENT);
                RECT rTitle = rOverlay; rTitle.top += dpiScale(15);
                DrawTextA(winDC, transferActive == 1 ? "XMODEM DOWNLOAD" : "XMODEM UPLOAD", -1, &rTitle, DT_CENTER | DT_TOP);
                
                RECT rMsg = rOverlay; rMsg.top += dpiScale(40);
                SetTextColor(winDC, RGB(16, 185, 129));
                DrawTextA(winDC, transferStatusMsg, -1, &rMsg, DT_CENTER | DT_TOP);
                
                SelectObject(winDC, oldUIFont);
                DeleteObject(uiFont);
            }

            SelectObject(memDC, oldFont);
            SelectObject(memDC, oldBmp);
            DeleteObject(memBmp);
            DeleteDC(memDC);

            BitBlt(hdc, 0, 0, cx, cy, winDC, 0, 0, SRCCOPY);

            SelectObject(winDC, oldWinBmp);
            DeleteObject(winBmp);
            DeleteDC(winDC);

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
                bytesTx += 1;
                UpdateStatus();
                if (SendMessageA(hEcho, BM_GETCHECK, 0, 0) == BST_CHECKED) {
                    ProcessTelnetByte((unsigned char)ch);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
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
                case VK_INSERT: seq = "\x1B[2~"; seqLen = 4; break;
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
                bytesTx += seqLen;
                UpdateStatus();
                return 0;
            }
            break;
        }
        case WM_SYSKEYDOWN: {
            if (sock != INVALID_SOCKET) {
                if ((wParam >= 'A' && wParam <= 'Z') || (wParam >= '0' && wParam <= '9')) {
                    char k = (char)wParam;
                    int i;
                    for (i = 0; i < numMacros; i++) {
                        if (dynMacros[i].key == k) {
                            char translated[1024];
                            int t = 0;
                            int j = 0;
                            while(dynMacros[i].str[j] && t < 1023) {
                                if (dynMacros[i].str[j] == '\\') {
                                    j++;
                                    if (dynMacros[i].str[j] == 0) break;
                                    if (dynMacros[i].str[j] == 'r') translated[t++] = '\r';
                                    else if (dynMacros[i].str[j] == 'n') translated[t++] = '\n';
                                    else if (dynMacros[i].str[j] == 't') translated[t++] = '\t';
                                    else if (dynMacros[i].str[j] == 'e') translated[t++] = '\x1B';
                                    else translated[t++] = dynMacros[i].str[j];
                                    j++;
                                } else {
                                    translated[t++] = dynMacros[i].str[j++];
                                }
                            }
                            if (t > 0) {
                                send(sock, translated, t, 0);
                                bytesTx += t;
                                UpdateStatus();
                            }
                            return 0; // Handled, stop processing
                        }
                    }
                }
            }
            break; // Let DefWindowProc handle it
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
    wc.hbrBackground = CreateSolidBrush(RGB(9, 9, 11));
    wc.hIcon = LoadIconA(wc.hInstance, MAKEINTRESOURCEA(1));

    RegisterClassA(&wc);

    /* Terminal: 80*8=640 wide, 25*16=400 tall. Plus padding and bars. */
    winW = dpiScale(820);
    winH = dpiScale(550);

    LoadMacros();

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
