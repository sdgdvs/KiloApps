#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 300
#define H 200

HWND hHex, hDec, hBin, hOct;
BOOL updating = FALSE;
HBRUSH hBrushBg;
HFONT hFont;
HBRUSH hEditBrush;

unsigned int parseHex(const char* s) {
    unsigned int res = 0;
    while (*s) {
        res <<= 4;
        if (*s >= '0' && *s <= '9') res += *s - '0';
        else if (*s >= 'a' && *s <= 'f') res += *s - 'a' + 10;
        else if (*s >= 'A' && *s <= 'F') res += *s - 'A' + 10;
        s++;
    }
    return res;
}

unsigned int parseDec(const char* s) {
    unsigned int res = 0;
    while (*s) {
        res = res * 10 + (*s - '0');
        s++;
    }
    return res;
}

unsigned int parseBin(const char* s) {
    unsigned int res = 0;
    while (*s) {
        res <<= 1;
        if (*s == '1') res++;
        s++;
    }
    return res;
}

unsigned int parseOct(const char* s) {
    unsigned int res = 0;
    while (*s) {
        res <<= 3;
        if (*s >= '0' && *s <= '7') res += *s - '0';
        s++;
    }
    return res;
}

void fmtHex(unsigned int v, char* s) { wsprintfA(s, "%X", v); }
void fmtDec(unsigned int v, char* s) { wsprintfA(s, "%u", v); }
void fmtBin(unsigned int v, char* s) {
    if (v == 0) { s[0] = '0'; s[1] = 0; return; }
    char tmp[33];
    int i = 0;
    while (v > 0) {
        tmp[i++] = (v & 1) ? '1' : '0';
        v >>= 1;
    }
    int j = 0;
    while (i > 0) s[j++] = tmp[--i];
    s[j] = 0;
}

void fmtOct(unsigned int v, char* s) {
    if (v == 0) { s[0] = '0'; s[1] = 0; return; }
    char tmp[33];
    int i = 0;
    while (v > 0) {
        tmp[i++] = (v & 7) + '0';
        v >>= 3;
    }
    int j = 0;
    while (i > 0) s[j++] = tmp[--i];
    s[j] = 0;
}

void UpdateFields(HWND hSrc) {
    if (updating) return;
    updating = TRUE;
    
    char buf[64];
    GetWindowTextA(hSrc, buf, 64);
    
    unsigned int val = 0;
    if (hSrc == hHex) val = parseHex(buf);
    else if (hSrc == hDec) val = parseDec(buf);
    else if (hSrc == hBin) val = parseBin(buf);
    else if (hSrc == hOct) val = parseOct(buf);
    
    char hex[64], dec[64], bin[64], oct[64];
    fmtHex(val, hex);
    fmtDec(val, dec);
    fmtBin(val, bin);
    fmtOct(val, oct);
    
    if (hSrc != hHex) SetWindowTextA(hHex, hex);
    if (hSrc != hDec) SetWindowTextA(hDec, dec);
    if (hSrc != hBin) SetWindowTextA(hBin, bin);
    if (hSrc != hOct) SetWindowTextA(hOct, oct);
    
    updating = FALSE;
}

BOOL CALLBACK SetFontProc(HWND child, LPARAM hFont) {
    SendMessage(child, WM_SETFONT, hFont, TRUE);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            CreateWindowEx(0, "STATIC", "Hex:", WS_CHILD | WS_VISIBLE, 10, 15, 30, 20, hwnd, NULL, NULL, NULL);
            hHex = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 15, W - 80, 22, hwnd, (HMENU)1, NULL, NULL);
            
            CreateWindowEx(0, "STATIC", "Dec:", WS_CHILD | WS_VISIBLE, 10, 45, 30, 20, hwnd, NULL, NULL, NULL);
            hDec = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 45, W - 80, 22, hwnd, (HMENU)2, NULL, NULL);
            
            CreateWindowEx(0, "STATIC", "Bin:", WS_CHILD | WS_VISIBLE, 10, 75, 30, 20, hwnd, NULL, NULL, NULL);
            hBin = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 75, W - 80, 22, hwnd, (HMENU)3, NULL, NULL);
            
            CreateWindowEx(0, "STATIC", "Oct:", WS_CHILD | WS_VISIBLE, 10, 105, 30, 20, hwnd, NULL, NULL, NULL);
            hOct = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 105, W - 80, 22, hwnd, (HMENU)4, NULL, NULL);
            
            EnumChildWindows(hwnd, SetFontProc, (LPARAM)hFont);
            break;
        }
        case WM_COMMAND: {
            if (HIWORD(wParam) == EN_CHANGE) {
                HWND hSrc = (HWND)lParam;
                UpdateFields(hSrc);
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(148, 163, 184));
            SetBkColor(hdc, RGB(15, 23, 42));
            return (LRESULT)hBrushBg;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(248, 250, 252));
            SetBkColor(hdc, RGB(30, 41, 59));
            if (!hEditBrush) hEditBrush = CreateSolidBrush(RGB(30, 41, 59));
            return (LRESULT)hEditBrush;
        }
        case WM_DESTROY:
            if (hFont) DeleteObject(hFont);
            if (hEditBrush) DeleteObject(hEditBrush);
            if (hBrushBg) DeleteObject(hBrushBg);
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
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KHexApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    hBrushBg = CreateSolidBrush(RGB(15, 23, 42));
    wc.hbrBackground = hBrushBg;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KHexApp", "KHex", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, W, H, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
