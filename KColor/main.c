#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define WIN_W 560
#define WIN_H 450

// Control IDs
#define IDC_SCROLL_R 101
#define IDC_SCROLL_G 102
#define IDC_SCROLL_B 103
#define IDC_SCROLL_H 104
#define IDC_SCROLL_S 105
#define IDC_SCROLL_L 106

#define IDC_HEX_EDIT     107
#define IDC_BTN_COPY_HEX 108
#define IDC_BTN_COPY_RGB 109
#define IDC_BTN_COPY_HSL 110
#define IDC_BTN_RANDOM   111
#define IDC_BTN_INVERT   112

// Swatch rect count
#define NUM_SWATCHES 10

typedef struct {
    COLORREF color;
    const char* name;
    RECT rect;
} Swatch;

// Global State
static int g_r = 100, g_g = 150, g_b = 200;
static int g_h = 210, g_s = 50, g_l = 59;
static unsigned int g_randSeed = 123456789;

static BOOL g_bUpdatingScrolls = FALSE;
static BOOL g_bUpdatingHexEdit = FALSE;

// Handles
static HWND g_hScrollR, g_hScrollG, g_hScrollB;
static HWND g_hScrollH, g_hScrollS, g_hScrollL;
static HWND g_hValR, g_hValG, g_hValB;
static HWND g_hValH, g_hValS, g_hValL;
static HWND g_hEditHex;
static HWND g_hBtnCopyHex, g_hBtnCopyRgb, g_hBtnCopyHsl;
static HWND g_hBtnRandom, g_hBtnInvert;
static HWND g_hStatus;

static HFONT g_hFontNormal = NULL;
static HFONT g_hFontBold = NULL;
static HFONT g_hFontMono = NULL;
static HBRUSH g_hBgBrush = NULL;
static WNDPROC g_OldEditProc = NULL;

static Swatch g_swatches[NUM_SWATCHES] = {
    { RGB(239, 68, 68),  "Red",     {0} },
    { RGB(245, 158, 11), "Orange",  {0} },
    { RGB(234, 179, 8),  "Yellow",  {0} },
    { RGB(16, 185, 129), "Green",   {0} },
    { RGB(6, 182, 212),  "Cyan",    {0} },
    { RGB(59, 130, 246), "Blue",    {0} },
    { RGB(139, 92, 246), "Purple",  {0} },
    { RGB(236, 72, 153), "Pink",    {0} },
    { RGB(15, 23, 42),   "Dark",    {0} },
    { RGB(255, 255, 255),"White",   {0} }
};

// CRT replacement definitions for no-CRT link
int _fltused = 1;

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) *bytes++ = (char)c;
    return dest;
}

#pragma function(memcpy)
void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}

static unsigned int fast_rand() {
    g_randSeed = (g_randSeed * 1103515245 + 12345) & 0x7fffffff;
    return g_randSeed;
}

// --- MATH & CONVERSION BOUNDS ---

static int clamp_int(int val, int minVal, int maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

static float clamp_float(float val, float minVal, float maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

static void RgbToHsl(int r, int g, int b, int *outH, int *outS, int *outL) {
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;
    float max = (rf > gf) ? ((rf > bf) ? rf : bf) : ((gf > bf) ? gf : bf);
    float min = (rf < gf) ? ((rf < bf) ? rf : bf) : ((gf < bf) ? gf : bf);
    float h = 0.0f, s = 0.0f;
    float l = (max + min) / 2.0f;

    if (max != min) {
        float d = max - min;
        s = (l > 0.5f) ? (d / (2.0f - max - min)) : (d / (max + min));
        if (max == rf) {
            h = (gf - bf) / d + (gf < bf ? 6.0f : 0.0f);
        } else if (max == gf) {
            h = (bf - rf) / d + 2.0f;
        } else {
            h = (rf - gf) / d + 4.0f;
        }
        h /= 6.0f;
    }
    *outH = clamp_int((int)(h * 360.0f + 0.5f), 0, 360);
    *outS = clamp_int((int)(s * 100.0f + 0.5f), 0, 100);
    *outL = clamp_int((int)(l * 100.0f + 0.5f), 0, 100);
}

static float Hue2Rgb(float p, float q, float t) {
    if (t < 0.0f) t += 1.0f;
    if (t > 1.0f) t -= 1.0f;
    if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
    if (t < 1.0f / 2.0f) return q;
    if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
    return p;
}

static void HslToRgb(int h, int s, int l, int *outR, int *outG, int *outB) {
    float hf = ((h % 360 + 360) % 360) / 360.0f;
    float sf = clamp_float(s / 100.0f, 0.0f, 1.0f);
    float lf = clamp_float(l / 100.0f, 0.0f, 1.0f);
    float r, g, b;

    if (sf == 0.0f) {
        r = g = b = lf;
    } else {
        float q = (lf < 0.5f) ? (lf * (1.0f + sf)) : (lf + sf - lf * sf);
        float p = 2.0f * lf - q;
        r = Hue2Rgb(p, q, hf + 1.0f / 3.0f);
        g = Hue2Rgb(p, q, hf);
        b = Hue2Rgb(p, q, hf - 1.0f / 3.0f);
    }
    *outR = clamp_int((int)(r * 255.0f + 0.5f), 0, 255);
    *outG = clamp_int((int)(g * 255.0f + 0.5f), 0, 255);
    *outB = clamp_int((int)(b * 255.0f + 0.5f), 0, 255);
}

static int HexCharVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static BOOL ParseHexColor(const char* str, int* outR, int* outG, int* outB) {
    if (!str) return FALSE;
    while (*str == ' ' || *str == '\t' || *str == '#') str++;
    
    int len = 0;
    while (str[len] && str[len] != ' ' && str[len] != '\r' && str[len] != '\n') len++;

    if (len == 3 || len == 4) {
        int r1 = HexCharVal(str[0]);
        int g1 = HexCharVal(str[1]);
        int b1 = HexCharVal(str[2]);
        if (r1 < 0 || g1 < 0 || b1 < 0) return FALSE;
        *outR = r1 * 17;
        *outG = g1 * 17;
        *outB = b1 * 17;
        return TRUE;
    } else if (len >= 6) {
        int r1 = HexCharVal(str[0]), r2 = HexCharVal(str[1]);
        int g1 = HexCharVal(str[2]), g2 = HexCharVal(str[3]);
        int b1 = HexCharVal(str[4]), b2 = HexCharVal(str[5]);
        if (r1 < 0 || r2 < 0 || g1 < 0 || g2 < 0 || b1 < 0 || b2 < 0) return FALSE;
        *outR = r1 * 16 + r2;
        *outG = g1 * 16 + g2;
        *outB = b1 * 16 + b2;
        return TRUE;
    }
    return FALSE;
}

// --- CLIPBOARD SAFETY ---

static void CopyTextToClipboard(HWND hwndOwner, const char* text) {
    if (!text || !*text) return;
    if (OpenClipboard(hwndOwner)) {
        EmptyClipboard();
        size_t len = lstrlenA(text) + 1;
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
        if (hMem) {
            char* pMem = (char*)GlobalLock(hMem);
            if (pMem) {
                lstrcpyA(pMem, text);
                GlobalUnlock(hMem);
                if (!SetClipboardData(CF_TEXT, hMem)) {
                    GlobalFree(hMem);
                }
            } else {
                GlobalFree(hMem);
            }
        }
        CloseClipboard();
        if (g_hStatus) {
            char msg[64];
            wsprintfA(msg, "Copied '%s' to clipboard!", text);
            SetWindowTextA(g_hStatus, msg);
        }
    }
}

// --- EDIT CONTROL SUBCLASSING ---

static LRESULT CALLBACK HexEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_CHAR) {
        if (wParam < 32 || wParam == 127) {
            return CallWindowProcA(g_OldEditProc, hwnd, msg, wParam, lParam);
        }
        char c = (char)wParam;
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || c == '#') {
            return CallWindowProcA(g_OldEditProc, hwnd, msg, wParam, lParam);
        }
        MessageBeep(MB_OK);
        return 0;
    }
    if (msg == WM_KEYDOWN && wParam == VK_RETURN) {
        HWND hParent = GetParent(hwnd);
        if (hParent) {
            SendMessageA(hParent, WM_COMMAND, MAKEWPARAM(IDC_HEX_EDIT, EN_CHANGE), (LPARAM)hwnd);
        }
        return 0;
    }
    if (msg == WM_KEYDOWN && wParam == VK_ESCAPE) {
        HWND hParent = GetParent(hwnd);
        if (hParent) {
            SetFocus(hParent);
        }
        return 0;
    }
    return CallWindowProcA(g_OldEditProc, hwnd, msg, wParam, lParam);
}

// --- UI REFRESH logic ---

static void SyncControlsFromRgb(HWND hwnd, BOOL updateHexEdit) {
    g_bUpdatingScrolls = TRUE;

    // Calc HSL
    RgbToHsl(g_r, g_g, g_b, &g_h, &g_s, &g_l);

    // Scrollbars
    SetScrollPos(g_hScrollR, SB_CTL, g_r, TRUE);
    SetScrollPos(g_hScrollG, SB_CTL, g_g, TRUE);
    SetScrollPos(g_hScrollB, SB_CTL, g_b, TRUE);
    SetScrollPos(g_hScrollH, SB_CTL, g_h, TRUE);
    SetScrollPos(g_hScrollS, SB_CTL, g_s, TRUE);
    SetScrollPos(g_hScrollL, SB_CTL, g_l, TRUE);

    // Labels
    char buf[32];
    wsprintfA(buf, "%d", g_r); SetWindowTextA(g_hValR, buf);
    wsprintfA(buf, "%d", g_g); SetWindowTextA(g_hValG, buf);
    wsprintfA(buf, "%d", g_b); SetWindowTextA(g_hValB, buf);

    wsprintfA(buf, "%d°", g_h); SetWindowTextA(g_hValH, buf);
    wsprintfA(buf, "%d%%", g_s); SetWindowTextA(g_hValS, buf);
    wsprintfA(buf, "%d%%", g_l); SetWindowTextA(g_hValL, buf);

    // Hex Edit
    if (updateHexEdit) {
        g_bUpdatingHexEdit = TRUE;
        wsprintfA(buf, "#%02X%02X%02X", g_r, g_g, g_b);
        SetWindowTextA(g_hEditHex, buf);
        g_bUpdatingHexEdit = FALSE;
    }

    g_bUpdatingScrolls = FALSE;
    InvalidateRect(hwnd, NULL, FALSE);
}

// --- WNDPROC ---

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_randSeed = GetTickCount();

            g_hFontNormal = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            g_hFontBold = CreateFontA(14, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            g_hFontMono = CreateFontA(16, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
            g_hBgBrush = CreateSolidBrush(RGB(240, 243, 246));

            // Section 1: RGB Sliders
            HWND hLbl;
            hLbl = CreateWindowExA(0, "STATIC", "RGB Controls", WS_CHILD | WS_VISIBLE, 10, 10, 100, 18, hwnd, NULL, NULL, NULL);
            SendMessageA(hLbl, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

            // R
            CreateWindowExA(0, "STATIC", "R:", WS_CHILD | WS_VISIBLE, 10, 35, 20, 20, hwnd, NULL, NULL, NULL);
            g_hScrollR = CreateWindowExA(0, "SCROLLBAR", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | SBS_HORZ, 30, 35, 140, 20, hwnd, (HMENU)IDC_SCROLL_R, NULL, NULL);
            SetScrollRange(g_hScrollR, SB_CTL, 0, 255, FALSE);
            g_hValR = CreateWindowExA(0, "STATIC", "100", WS_CHILD | WS_VISIBLE | SS_RIGHT, 175, 35, 30, 20, hwnd, NULL, NULL, NULL);

            // G
            CreateWindowExA(0, "STATIC", "G:", WS_CHILD | WS_VISIBLE, 10, 65, 20, 20, hwnd, NULL, NULL, NULL);
            g_hScrollG = CreateWindowExA(0, "SCROLLBAR", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | SBS_HORZ, 30, 65, 140, 20, hwnd, (HMENU)IDC_SCROLL_G, NULL, NULL);
            SetScrollRange(g_hScrollG, SB_CTL, 0, 255, FALSE);
            g_hValG = CreateWindowExA(0, "STATIC", "150", WS_CHILD | WS_VISIBLE | SS_RIGHT, 175, 65, 30, 20, hwnd, NULL, NULL, NULL);

            // B
            CreateWindowExA(0, "STATIC", "B:", WS_CHILD | WS_VISIBLE, 10, 95, 20, 20, hwnd, NULL, NULL, NULL);
            g_hScrollB = CreateWindowExA(0, "SCROLLBAR", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | SBS_HORZ, 30, 95, 140, 20, hwnd, (HMENU)IDC_SCROLL_B, NULL, NULL);
            SetScrollRange(g_hScrollB, SB_CTL, 0, 255, FALSE);
            g_hValB = CreateWindowExA(0, "STATIC", "200", WS_CHILD | WS_VISIBLE | SS_RIGHT, 175, 95, 30, 20, hwnd, NULL, NULL, NULL);

            // Section 2: HSL Sliders
            hLbl = CreateWindowExA(0, "STATIC", "HSL Controls", WS_CHILD | WS_VISIBLE, 10, 130, 100, 18, hwnd, NULL, NULL, NULL);
            SendMessageA(hLbl, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

            // H
            CreateWindowExA(0, "STATIC", "H:", WS_CHILD | WS_VISIBLE, 10, 155, 20, 20, hwnd, NULL, NULL, NULL);
            g_hScrollH = CreateWindowExA(0, "SCROLLBAR", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | SBS_HORZ, 30, 155, 140, 20, hwnd, (HMENU)IDC_SCROLL_H, NULL, NULL);
            SetScrollRange(g_hScrollH, SB_CTL, 0, 360, FALSE);
            g_hValH = CreateWindowExA(0, "STATIC", "210°", WS_CHILD | WS_VISIBLE | SS_RIGHT, 175, 155, 30, 20, hwnd, NULL, NULL, NULL);

            // S
            CreateWindowExA(0, "STATIC", "S:", WS_CHILD | WS_VISIBLE, 10, 185, 20, 20, hwnd, NULL, NULL, NULL);
            g_hScrollS = CreateWindowExA(0, "SCROLLBAR", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | SBS_HORZ, 30, 185, 140, 20, hwnd, (HMENU)IDC_SCROLL_S, NULL, NULL);
            SetScrollRange(g_hScrollS, SB_CTL, 0, 100, FALSE);
            g_hValS = CreateWindowExA(0, "STATIC", "50%", WS_CHILD | WS_VISIBLE | SS_RIGHT, 175, 185, 30, 20, hwnd, NULL, NULL, NULL);

            // L
            CreateWindowExA(0, "STATIC", "L:", WS_CHILD | WS_VISIBLE, 10, 215, 20, 20, hwnd, NULL, NULL, NULL);
            g_hScrollL = CreateWindowExA(0, "SCROLLBAR", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | SBS_HORZ, 30, 215, 140, 20, hwnd, (HMENU)IDC_SCROLL_L, NULL, NULL);
            SetScrollRange(g_hScrollL, SB_CTL, 0, 100, FALSE);
            g_hValL = CreateWindowExA(0, "STATIC", "59%", WS_CHILD | WS_VISIBLE | SS_RIGHT, 175, 215, 30, 20, hwnd, NULL, NULL, NULL);

            // Hex Input & Copy Buttons
            CreateWindowExA(0, "STATIC", "Hex Code:", WS_CHILD | WS_VISIBLE, 225, 130, 70, 20, hwnd, NULL, NULL, NULL);
            g_hEditHex = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "#6496C8", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 300, 127, 90, 24, hwnd, (HMENU)IDC_HEX_EDIT, NULL, NULL);
            SendMessageA(g_hEditHex, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

            // Subclass the Edit control
            g_OldEditProc = (WNDPROC)SetWindowLongPtrA(g_hEditHex, GWLP_WNDPROC, (LONG_PTR)HexEditSubclassProc);

            // Copy Buttons
            g_hBtnCopyHex = CreateWindowExA(0, "BUTTON", "Copy HEX", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 225, 160, 95, 26, hwnd, (HMENU)IDC_BTN_COPY_HEX, NULL, NULL);
            g_hBtnCopyRgb = CreateWindowExA(0, "BUTTON", "Copy RGB", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 325, 160, 95, 26, hwnd, (HMENU)IDC_BTN_COPY_RGB, NULL, NULL);
            g_hBtnCopyHsl = CreateWindowExA(0, "BUTTON", "Copy HSL", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 425, 160, 95, 26, hwnd, (HMENU)IDC_BTN_COPY_HSL, NULL, NULL);

            // Utility Buttons: Random & Invert
            g_hBtnRandom = CreateWindowExA(0, "BUTTON", "Random (R)", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 225, 192, 110, 26, hwnd, (HMENU)IDC_BTN_RANDOM, NULL, NULL);
            g_hBtnInvert = CreateWindowExA(0, "BUTTON", "Invert (I)", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 345, 192, 110, 26, hwnd, (HMENU)IDC_BTN_INVERT, NULL, NULL);

            // Status Bar
            g_hStatus = CreateWindowExA(0, "STATIC", "Ready (L-Click select swatch, R-Click save to swatch, R: Random, I: Invert)", WS_CHILD | WS_VISIBLE | SS_SUNKEN, 10, 380, 525, 22, hwnd, NULL, NULL, NULL);

            // Apply font to all controls
            EnumChildWindows(hwnd, (WNDENUMPROC)SendMessageA, (LPARAM)WM_SETFONT);

            // Calculate Swatch Rectangles (bottom area)
            int swX = 225, swY = 260, swW = 27, swH = 27, gap = 4;
            for (int i = 0; i < NUM_SWATCHES; i++) {
                int col = i % 10;
                g_swatches[i].rect.left = swX + col * (swW + gap);
                g_swatches[i].rect.top = swY;
                g_swatches[i].rect.right = g_swatches[i].rect.left + swW;
                g_swatches[i].rect.bottom = g_swatches[i].rect.top + swH;
            }

            SyncControlsFromRgb(hwnd, TRUE);
            break;
        }

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN: {
            HDC hdc = (HDC)wParam;
            SetBkMode(hdc, TRANSPARENT);
            return (LRESULT)g_hBgBrush;
        }

        case WM_HSCROLL: {
            if (g_bUpdatingScrolls) break;
            HWND hScroll = (HWND)lParam;
            int id = GetDlgCtrlID(hScroll);
            
            SCROLLINFO si = { sizeof(SCROLLINFO), SIF_ALL };
            GetScrollInfo(hScroll, SB_CTL, &si);

            int pos = si.nPos;
            int code = LOWORD(wParam);
            if (code == SB_LINELEFT) pos -= 1;
            else if (code == SB_LINERIGHT) pos += 1;
            else if (code == SB_PAGELEFT) pos -= 10;
            else if (code == SB_PAGERIGHT) pos += 10;
            else if (code == SB_THUMBPOSITION || code == SB_THUMBTRACK) pos = si.nTrackPos;

            if (id == IDC_SCROLL_R || id == IDC_SCROLL_G || id == IDC_SCROLL_B) {
                pos = clamp_int(pos, 0, 255);
                if (id == IDC_SCROLL_R) g_r = pos;
                else if (id == IDC_SCROLL_G) g_g = pos;
                else if (id == IDC_SCROLL_B) g_b = pos;
                SyncControlsFromRgb(hwnd, TRUE);
            } else if (id == IDC_SCROLL_H || id == IDC_SCROLL_S || id == IDC_SCROLL_L) {
                if (id == IDC_SCROLL_H) g_h = clamp_int(pos, 0, 360);
                else if (id == IDC_SCROLL_S) g_s = clamp_int(pos, 0, 100);
                else if (id == IDC_SCROLL_L) g_l = clamp_int(pos, 0, 100);
                
                HslToRgb(g_h, g_s, g_l, &g_r, &g_g, &g_b);
                SyncControlsFromRgb(hwnd, TRUE);
            }
            break;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);

            if (id == IDC_HEX_EDIT && code == EN_CHANGE && !g_bUpdatingHexEdit) {
                char buf[32];
                GetWindowTextA(g_hEditHex, buf, sizeof(buf));
                int newR, newG, newB;
                if (ParseHexColor(buf, &newR, &newG, &newB)) {
                    g_r = newR; g_g = newG; g_b = newB;
                    SyncControlsFromRgb(hwnd, FALSE);
                }
            } else if (id == IDC_BTN_COPY_HEX) {
                char hex[16];
                wsprintfA(hex, "#%02X%02X%02X", g_r, g_g, g_b);
                CopyTextToClipboard(hwnd, hex);
            } else if (id == IDC_BTN_COPY_RGB) {
                char rgb[32];
                wsprintfA(rgb, "rgb(%d, %d, %d)", g_r, g_g, g_b);
                CopyTextToClipboard(hwnd, rgb);
            } else if (id == IDC_BTN_COPY_HSL) {
                char hsl[32];
                wsprintfA(hsl, "hsl(%d, %d%%, %d%%)", g_h, g_s, g_l);
                CopyTextToClipboard(hwnd, hsl);
            } else if (id == IDC_BTN_RANDOM) {
                g_r = fast_rand() % 256;
                g_g = fast_rand() % 256;
                g_b = fast_rand() % 256;
                SyncControlsFromRgb(hwnd, TRUE);
                if (g_hStatus) SetWindowTextA(g_hStatus, "Generated Random Color");
            } else if (id == IDC_BTN_INVERT) {
                g_r = 255 - g_r;
                g_g = 255 - g_g;
                g_b = 255 - g_b;
                SyncControlsFromRgb(hwnd, TRUE);
                if (g_hStatus) SetWindowTextA(g_hStatus, "Inverted Active Color");
            }
            break;
        }

        case WM_KEYDOWN: {
            if (wParam == 'R' || wParam == 'r') {
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(IDC_BTN_RANDOM, 0), 0);
            } else if (wParam == 'I' || wParam == 'i') {
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(IDC_BTN_INVERT, 0), 0);
            } else if (wParam == 'C' || wParam == 'c') {
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(IDC_BTN_COPY_HEX, 0), 0);
            } else if (wParam >= '1' && wParam <= '9') {
                int idx = (int)(wParam - '1');
                if (idx < NUM_SWATCHES) {
                    COLORREF c = g_swatches[idx].color;
                    g_r = GetRValue(c);
                    g_g = GetGValue(c);
                    g_b = GetBValue(c);
                    SyncControlsFromRgb(hwnd, TRUE);
                    if (g_hStatus) {
                        char msg[64];
                        wsprintfA(msg, "Selected Palette Swatch: %s", g_swatches[idx].name);
                        SetWindowTextA(g_hStatus, msg);
                    }
                }
            } else if (wParam == '0') {
                COLORREF c = g_swatches[9].color;
                g_r = GetRValue(c);
                g_g = GetGValue(c);
                g_b = GetBValue(c);
                SyncControlsFromRgb(hwnd, TRUE);
                if (g_hStatus) {
                    char msg[64];
                    wsprintfA(msg, "Selected Palette Swatch: %s", g_swatches[9].name);
                    SetWindowTextA(g_hStatus, msg);
                }
            }
            break;
        }

        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            POINT pt = { x, y };

            for (int i = 0; i < NUM_SWATCHES; i++) {
                if (PtInRect(&g_swatches[i].rect, pt)) {
                    COLORREF c = g_swatches[i].color;
                    g_r = GetRValue(c);
                    g_g = GetGValue(c);
                    g_b = GetBValue(c);
                    SyncControlsFromRgb(hwnd, TRUE);
                    if (g_hStatus) {
                        char msg[64];
                        wsprintfA(msg, "Selected Palette Swatch: %s", g_swatches[i].name);
                        SetWindowTextA(g_hStatus, msg);
                    }
                    break;
                }
            }
            break;
        }

        case WM_RBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            POINT pt = { x, y };

            for (int i = 0; i < NUM_SWATCHES; i++) {
                if (PtInRect(&g_swatches[i].rect, pt)) {
                    g_swatches[i].color = RGB(g_r, g_g, g_b);
                    InvalidateRect(hwnd, &g_swatches[i].rect, FALSE);
                    if (g_hStatus) {
                        char msg[64];
                        wsprintfA(msg, "Saved #%02X%02X%02X into Swatch %d (%s)!", g_r, g_g, g_b, i + 1, g_swatches[i].name);
                        SetWindowTextA(g_hStatus, msg);
                    }
                    break;
                }
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);

            // Double Buffering to eliminate flicker
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBmp = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
            HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

            // Fill Background
            FillRect(memDC, &clientRect, g_hBgBrush);

            // Draw Main Color Preview Box (Right Side)
            RECT previewRect = { 225, 10, 520, 115 };
            HBRUSH colorBrush = CreateSolidBrush(RGB(g_r, g_g, g_b));
            FillRect(memDC, &previewRect, colorBrush);
            DeleteObject(colorBrush);

            // Border around preview
            FrameRect(memDC, &previewRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

            // Overlay Hex Code inside preview box
            char hexStr[16];
            wsprintfA(hexStr, "#%02X%02X%02X", g_r, g_g, g_b);
            
            // Choose black or white text based on luminance
            float lum = (g_r * 0.299f + g_g * 0.587f + g_b * 0.114f);
            COLORREF textColor = lum > 128.0f ? RGB(0,0,0) : RGB(255,255,255);

            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, textColor);
            HFONT oldFont = (HFONT)SelectObject(memDC, g_hFontMono);
            TextOutA(memDC, 240, 45, hexStr, lstrlenA(hexStr));

            // Draw Swatches Title
            SelectObject(memDC, g_hFontBold);
            SetTextColor(memDC, RGB(30, 41, 59));
            TextOutA(memDC, 225, 235, "Quick Palette Swatches (L-Click select, R-Click save):", 54);

            // Draw Swatch Rects
            for (int i = 0; i < NUM_SWATCHES; i++) {
                HBRUSH swBrush = CreateSolidBrush(g_swatches[i].color);
                FillRect(memDC, &g_swatches[i].rect, swBrush);
                DeleteObject(swBrush);
                FrameRect(memDC, &g_swatches[i].rect, (HBRUSH)GetStockObject(GRAY_BRUSH));
            }

            // BitBlt to screen
            BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, SRCCOPY);

            // Cleanup GDI Objects
            SelectObject(memDC, oldFont);
            SelectObject(memDC, oldBmp);
            DeleteObject(memBmp);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_DESTROY:
            if (g_hFontNormal) DeleteObject(g_hFontNormal);
            if (g_hFontBold) DeleteObject(g_hFontBold);
            if (g_hFontMono) DeleteObject(g_hFontMono);
            if (g_hBgBrush) DeleteObject(g_hBgBrush);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandleA(NULL);
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KColorAppClass";
    wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(0, "KColorAppClass", "KColor - Advanced Color Picker & Palette Studio",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, WIN_W, WIN_H, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        if (!IsDialogMessageA(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    ExitProcess(0);
}
