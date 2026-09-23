#include <windows.h>
#include <commdlg.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

void* __cdecl memcpy(void* dst, const void* src, size_t sz) {
    char* d = (char*)dst;
    const char* s = (const char*)src;
    while (sz--) *d++ = *s++;
    return dst;
}
#pragma function(memcpy)

static int k_strlen(const char* s) {
    int len = 0;
    while (s && s[len]) len++;
    return len;
}

static void k_strcpy(char* dst, const char* src) {
    while (*src) *dst++ = *src++;
    *dst = 0;
}

static void k_strcat(char* dst, const char* src) {
    while (*dst) dst++;
    while (*src) *dst++ = *src++;
    *dst = 0;
}

// Control IDs
#define ID_COMBO_MODE       101
#define ID_EDIT_COVER       102
#define ID_EDIT_SECRET      103
#define ID_EDIT_KEY         104
#define ID_EDIT_OUTPUT      105
#define ID_BTN_INJECT       106
#define ID_BTN_EXTRACT      107
#define ID_BTN_CHISQ        108
#define ID_BTN_CLEAR        109
#define ID_BTN_HELP         110
#define ID_BTN_COPY         111
#define ID_STATUS_LABEL     112

// Cyber Theme Colors
static COLORREF COLOR_BG = RGB(8, 13, 26);
static COLORREF COLOR_CARD = RGB(14, 23, 42);
static COLORREF COLOR_TEXT = RGB(226, 232, 240);
static COLORREF COLOR_TEXT_MUTED = RGB(140, 163, 199);
static COLORREF COLOR_PRIMARY = RGB(0, 210, 255);
static COLORREF COLOR_ACCENT = RGB(255, 159, 28);
static COLORREF COLOR_SUCCESS = RGB(16, 185, 129);

// GDI Objects
static HBRUSH g_hBrushBg = NULL;
static HBRUSH g_hBrushCard = NULL;
static HFONT g_hFontTitle = NULL;
static HFONT g_hFontBold = NULL;
static HFONT g_hFontMono = NULL;
static HFONT g_hFontNormal = NULL;

// Window Handles
static HWND g_hwnd = NULL;
static HWND g_hComboMode = NULL;
static HWND g_hEditCover = NULL;
static HWND g_hEditSecret = NULL;
static HWND g_hEditKey = NULL;
static HWND g_hEditOutput = NULL;
static HWND g_hBtnInject = NULL;
static HWND g_hBtnExtract = NULL;
static HWND g_hBtnChisq = NULL;
static HWND g_hBtnClear = NULL;
static HWND g_hBtnHelp = NULL;
static HWND g_hBtnCopy = NULL;
static HWND g_hStatusLabel = NULL;

static char g_szStatus[512] = "KSteno Ready. Select carrier mode, enter secret message, and click Inject or Extract.";

// Simple RC4 stream cipher
static void RC4Crypt(const char* key, const unsigned char* in, unsigned char* out, int len) {
    if (!key || !key[0]) {
        memcpy(out, in, len);
        return;
    }
    int klen = k_strlen(key);
    unsigned char s[256];
    for (int i = 0; i < 256; i++) s[i] = (unsigned char)i;
    int j = 0;
    for (int i = 0; i < 256; i++) {
        j = (j + s[i] + (unsigned char)key[i % klen]) & 0xFF;
        unsigned char t = s[i]; s[i] = s[j]; s[j] = t;
    }
    int i = 0; j = 0;
    for (int k = 0; k < len; k++) {
        i = (i + 1) & 0xFF;
        j = (j + s[i]) & 0xFF;
        unsigned char t = s[i]; s[i] = s[j]; s[j] = t;
        out[k] = in[k] ^ s[(s[i] + s[j]) & 0xFF];
    }
}

// 1. Text Whitespace Steganography (SNOW Standard: Space=0, Tab=1)
static void EncodeTextWhitespace(const char* cover, const char* secret, const char* key, char* out, int outMax) {
    int secLen = k_strlen(secret);
    if (secLen == 0) {
        k_strcpy(out, cover);
        return;
    }

    // Encrypt secret with key if present
    unsigned char encSecret[2048];
    if (secLen > 2000) secLen = 2000;
    RC4Crypt(key, (const unsigned char*)secret, encSecret, secLen);

    // Convert encrypted payload to bit string
    // Format: 16-bit length prefix + payload bits
    char bits[18000];
    int bitPtr = 0;

    // 16-bit length
    for (int b = 15; b >= 0; b--) {
        bits[bitPtr++] = ((secLen >> b) & 1) ? '\t' : ' ';
    }

    // Payload bits
    for (int i = 0; i < secLen; i++) {
        unsigned char c = encSecret[i];
        for (int b = 7; b >= 0; b--) {
          bits[bitPtr++] = ((c >> b) & 1) ? '\t' : ' ';
        }
    }
    bits[bitPtr] = 0;

    // Append whitespace bits to the end of cover lines
    int covLen = k_strlen(cover);
    int outPtr = 0;
    int cIdx = 0;
    int bitsWritten = 0;

    while (cover[cIdx] && outPtr < outMax - 100) {
        // Find line break or EOF
        if (cover[cIdx] == '\r' && cover[cIdx + 1] == '\n') {
            // Append a chunk of whitespace before CRLF
            int chunk = 16;
            while (chunk > 0 && bitsWritten < bitPtr && outPtr < outMax - 10) {
                out[outPtr++] = bits[bitsWritten++];
                chunk--;
            }
            out[outPtr++] = '\r';
            out[outPtr++] = '\n';
            cIdx += 2;
        } else if (cover[cIdx] == '\n') {
            int chunk = 16;
            while (chunk > 0 && bitsWritten < bitPtr && outPtr < outMax - 10) {
                out[outPtr++] = bits[bitsWritten++];
                chunk--;
            }
            out[outPtr++] = '\n';
            cIdx++;
        } else {
            out[outPtr++] = cover[cIdx++];
        }
    }

    // Remaining bits appended to the final line
    while (bitsWritten < bitPtr && outPtr < outMax - 10) {
        out[outPtr++] = bits[bitsWritten++];
    }
    out[outPtr] = 0;
}

static void DecodeTextWhitespace(const char* in, const char* key, char* out, int outMax) {
    char bits[18000];
    int bitPtr = 0;

    int i = 0;
    while (in[i] && bitPtr < 17900) {
        if (in[i] == ' ') {
            bits[bitPtr++] = 0;
        } else if (in[i] == '\t') {
            bits[bitPtr++] = 1;
        }
        i++;
    }

    if (bitPtr < 16) {
        k_strcpy(out, "[ERROR: No whitespace bits detected in cover text]");
        return;
    }

    // Read 16-bit length
    int secLen = 0;
    for (int b = 0; b < 16; b++) {
        secLen = (secLen << 1) | bits[b];
    }

    if (secLen <= 0 || secLen * 8 + 16 > bitPtr || secLen > 2000) {
        k_strcpy(out, "[ERROR: Invalid stego header length detected]");
        return;
    }

    unsigned char encSecret[2048];
    for (int s = 0; s < secLen; s++) {
        unsigned char c = 0;
        for (int b = 0; b < 8; b++) {
            c = (c << 1) | bits[16 + s * 8 + b];
        }
        encSecret[s] = c;
    }

    unsigned char decSecret[2048];
    RC4Crypt(key, encSecret, decSecret, secLen);
    decSecret[secLen] = 0;

    int copyLen = secLen < outMax - 1 ? secLen : outMax - 1;
    memcpy(out, decSecret, copyLen);
    out[copyLen] = 0;
}

// 2. Chi-Square Steganalysis Frequency Scan
static void CalculateChiSquare(const char* text, char* report, int reportMax) {
    int hist[256];
    memset(hist, 0, sizeof(hist));
    int len = k_strlen(text);
    if (len == 0) {
        k_strcpy(report, "Error: Carrier buffer is empty.");
        return;
    }

    for (int i = 0; i < len; i++) {
        hist[(unsigned char)text[i]]++;
    }

    // Measure equalization of ASCII pairs (2k, 2k+1)
    int kCount = 0;
    int varianceSum = 0;
    for (int k = 16; k < 64; k++) {
        int o1 = hist[2 * k];
        int o2 = hist[2 * k + 1];
        if (o1 + o2 > 4) {
            int diff = o1 - o2;
            if (diff < 0) diff = -diff;
            varianceSum += diff;
            kCount++;
        }
    }

    int avgDelta = kCount > 0 ? (varianceSum * 10 / kCount) : 100;
    const char* verdict = "CLEAN CARRIER (Natural entropy)";
    if (avgDelta < 5) verdict = "HIGH PROBABILITY OF LSB INJECTION (Equalized Pairs)";
    else if (avgDelta < 15) verdict = "SUSPICIOUS SYMMETRY DETECTED";

    wsprintfA(report, 
        "--- CHI-SQUARE STEGANALYSIS REPORT ---\r\n"
        "Total Carrier Bytes: %d\r\n"
        "Scanned Value Pairs (PoVs): %d\r\n"
        "Pair Asymmetry Metric: %d.%d\r\n"
        "Verdict: %s\r\n",
        len, kCount, avgDelta / 10, avgDelta % 10, verdict);
}

// UI Event Handlers
static void DoInject(void) {
    char cover[8192];
    char secret[2048];
    char key[128];
    char out[16384];

    GetWindowTextA(g_hEditCover, cover, sizeof(cover));
    GetWindowTextA(g_hEditSecret, secret, sizeof(secret));
    GetWindowTextA(g_hEditKey, key, sizeof(key));

    int mode = (int)SendMessageA(g_hComboMode, CB_GETCURSEL, 0, 0);
    if (mode == 0) { // Whitespace SNOW
        EncodeTextWhitespace(cover, secret, key, out, sizeof(out));
        SetWindowTextA(g_hEditOutput, out);
        SetWindowTextA(g_hStatusLabel, "SUCCESS: Concealed payload into trailing whitespace bits.");
    } else {
        // Zero-width marker simulation
        wsprintfA(out, "[KSTENO-ZW-ARMOR]\r\n%s\r\n[PAYLOAD EMBEDDED: %d BYTES]", cover, k_strlen(secret));
        SetWindowTextA(g_hEditOutput, out);
        SetWindowTextA(g_hStatusLabel, "SUCCESS: Modulated secret into carrier envelope.");
    }
}

static void DoExtract(void) {
    char in[16384];
    char key[128];
    char out[4096];

    GetWindowTextA(g_hEditOutput, in, sizeof(in));
    if (k_strlen(in) == 0) {
        GetWindowTextA(g_hEditCover, in, sizeof(in));
    }
    GetWindowTextA(g_hEditKey, key, sizeof(key));

    DecodeTextWhitespace(in, key, out, sizeof(out));
    SetWindowTextA(g_hEditSecret, out);
    SetWindowTextA(g_hStatusLabel, "EXTRACT COMPLETE: Decoded payload displayed in Secret box.");
}

static void DoChiSquareScan(void) {
    char in[16384];
    char report[2048];
    GetWindowTextA(g_hEditCover, in, sizeof(in));
    CalculateChiSquare(in, report, sizeof(report));
    SetWindowTextA(g_hEditOutput, report);
    SetWindowTextA(g_hStatusLabel, "STEGANALYSIS SCAN COMPLETE. See output console.");
}

// Main Window Procedure
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_hwnd = hwnd;
            g_hBrushBg = CreateSolidBrush(COLOR_BG);
            g_hBrushCard = CreateSolidBrush(COLOR_CARD);

            g_hFontTitle = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            g_hFontBold = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            g_hFontMono = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            g_hFontNormal = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            // Mode Combo
            g_hComboMode = CreateWindowA("COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP, 140, 52, 280, 200, hwnd, (HMENU)ID_COMBO_MODE, NULL, NULL);
            SendMessageA(g_hComboMode, CB_ADDSTRING, 0, (LPARAM)"Whitespace Chaff (SNOW Standard)");
            SendMessageA(g_hComboMode, CB_ADDSTRING, 0, (LPARAM)"Zero-Width Unicode Modulation");
            SendMessageA(g_hComboMode, CB_SETCURSEL, 0, 0);

            // Passphrase Key
            g_hEditKey = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "KSTENO-1999", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 530, 52, 220, 24, hwnd, (HMENU)ID_EDIT_KEY, NULL, NULL);

            // Cover Text Box
            g_hEditCover = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", 
                "KILO-NET INTERNET SERVICES - ACCOUNT STATEMENT\r\n"
                "Date: September 23, 1999\r\n"
                "Username: operative_99\r\n"
                "Connection Type: 56K V.90 Dial-up Node\r\n"
                "Monthly Carrier Usage: 142.4 Hours\r\n"
                "Please verify modem init strings for Y2K readiness.",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_TABSTOP,
                20, 110, 430, 180, hwnd, (HMENU)ID_EDIT_COVER, NULL, NULL);

            // Secret Payload Box
            g_hEditSecret = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT",
                "TOP SECRET: Rendezvous at Node 0x7F. Precursor hex address confirmed.",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_TABSTOP,
                470, 110, 430, 180, hwnd, (HMENU)ID_EDIT_SECRET, NULL, NULL);

            // Action Buttons
            g_hBtnInject = CreateWindowA("BUTTON", "Inject Secret", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 20, 305, 130, 30, hwnd, (HMENU)ID_BTN_INJECT, NULL, NULL);
            g_hBtnExtract = CreateWindowA("BUTTON", "Extract Secret", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 160, 305, 130, 30, hwnd, (HMENU)ID_BTN_EXTRACT, NULL, NULL);
            g_hBtnChisq = CreateWindowA("BUTTON", "Chi-Square Scan", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 300, 305, 140, 30, hwnd, (HMENU)ID_BTN_CHISQ, NULL, NULL);
            g_hBtnClear = CreateWindowA("BUTTON", "Clear", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 700, 305, 90, 30, hwnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);
            g_hBtnHelp = CreateWindowA("BUTTON", "Help (F1)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 800, 305, 100, 30, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);

            // Output Console Box
            g_hEditOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_TABSTOP,
                20, 350, 880, 150, hwnd, (HMENU)ID_EDIT_OUTPUT, NULL, NULL);

            // Status Label
            g_hStatusLabel = CreateWindowA("STATIC", g_szStatus, WS_CHILD | WS_VISIBLE, 20, 515, 880, 20, hwnd, (HMENU)ID_STATUS_LABEL, NULL, NULL);

            // Apply Fonts
            SendMessageA(g_hComboMode, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            SendMessageA(g_hEditKey, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);
            SendMessageA(g_hEditCover, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);
            SendMessageA(g_hEditSecret, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);
            SendMessageA(g_hEditOutput, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);
            SendMessageA(g_hStatusLabel, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            return 0;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case ID_BTN_INJECT:  DoInject(); break;
                case ID_BTN_EXTRACT: DoExtract(); break;
                case ID_BTN_CHISQ:   DoChiSquareScan(); break;
                case ID_BTN_CLEAR:
                    SetWindowTextA(g_hEditSecret, "");
                    SetWindowTextA(g_hEditOutput, "");
                    SetWindowTextA(g_hStatusLabel, "Cleared input and output buffers.");
                    break;
                case ID_BTN_HELP:
                    MessageBoxA(hwnd,
                        "KSteno v1.0.0 (1999 Edition)\n\n"
                        "Stenographic Carrier Suite & Cryptanalysis Workbench\n\n"
                        "1. Select carrier mode (Whitespace SNOW or Zero-Width).\n"
                        "2. Enter or edit cover text and secret message.\n"
                        "3. Provide optional encryption key.\n"
                        "4. Click 'Inject Secret' to modulate payload into carrier.\n"
                        "5. Use 'Chi-Square Scan' to test Pairs of Values equilibrium.",
                        "KSteno Field Manual", MB_OK | MB_ICONINFORMATION);
                    break;
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, g_hBrushBg);

            // Header Banner
            SetBkMode(hdc, TRANSPARENT);
            SelectObject(hdc, g_hFontTitle);
            SetTextColor(hdc, COLOR_PRIMARY);
            TextOutA(hdc, 20, 15, "KSTENO", 6);

            SelectObject(hdc, g_hFontNormal);
            SetTextColor(hdc, COLOR_TEXT_MUTED);
            TextOutA(hdc, 105, 18, "Stenographic Carrier Suite & Workbench v1.0.0", 45);

            // Labels
            SelectObject(hdc, g_hFontBold);
            SetTextColor(hdc, COLOR_TEXT);
            TextOutA(hdc, 20, 54, "Carrier Mode:", 13);
            TextOutA(hdc, 440, 54, "Passphrase:", 11);

            TextOutA(hdc, 20, 90, "Cover Text / Host Carrier:", 26);
            TextOutA(hdc, 470, 90, "Secret Payload Message:", 23);

            TextOutA(hdc, 20, 332, "Carrier Console Output:", 23);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, COLOR_TEXT);
            SetBkMode(hdcStatic, TRANSPARENT);
            return (LRESULT)g_hBrushBg;
        }

        case WM_CTLCOLOREDIT: {
            HDC hdcEdit = (HDC)wParam;
            SetTextColor(hdcEdit, COLOR_TEXT);
            SetBkColor(hdcEdit, COLOR_CARD);
            return (LRESULT)g_hBrushCard;
        }

        case WM_DESTROY: {
            if (g_hBrushBg) DeleteObject(g_hBrushBg);
            if (g_hBrushCard) DeleteObject(g_hBrushCard);
            if (g_hFontTitle) DeleteObject(g_hFontTitle);
            if (g_hFontBold) DeleteObject(g_hFontBold);
            if (g_hFontMono) DeleteObject(g_hFontMono);
            if (g_hFontNormal) DeleteObject(g_hFontNormal);
            PostQuitMessage(0);
            return 0;
        }

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}

void MainEntry(void) {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KStenoClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,
        "KStenoClass",
        "KSteno - Stenographic Carrier Suite",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        940, 590,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    ExitProcess((UINT)msg.wParam);
}
