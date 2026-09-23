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

static int k_atoi(const char* s) {
    int v = 0;
    while (*s >= '0' && *s <= '9') {
        v = v * 10 + (*s - '0');
        s++;
    }
    return v;
}

// Control IDs
#define ID_EDIT_INPUT       101
#define ID_EDIT_OUTPUT      102
#define ID_COMBO_CIPHER     103
#define ID_EDIT_KEY         104
#define ID_BTN_ENCRYPT      105
#define ID_BTN_DECRYPT      106
#define ID_BTN_SWAP         107
#define ID_BTN_CLEAR        108
#define ID_BTN_HELP         109
#define ID_BTN_COPY         110

// Colors
static COLORREF COLOR_BG = RGB(9, 13, 22);
static COLORREF COLOR_CARD = RGB(15, 23, 42);
static COLORREF COLOR_TEXT = RGB(248, 250, 252);
static COLORREF COLOR_TEXT_MUTED = RGB(148, 163, 184);
static COLORREF COLOR_PRIMARY = RGB(56, 189, 248);
static COLORREF COLOR_ACCENT = RGB(245, 158, 11);

// GDI Objects
static HBRUSH g_hBrushBg = NULL;
static HBRUSH g_hBrushCard = NULL;
static HFONT g_hFontTitle = NULL;
static HFONT g_hFontBold = NULL;
static HFONT g_hFontMono = NULL;
static HFONT g_hFontNormal = NULL;

// Window Handles
static HWND g_hwnd = NULL;
static HWND g_hEditInput = NULL;
static HWND g_hEditOutput = NULL;
static HWND g_hComboCipher = NULL;
static HWND g_hEditKey = NULL;
static HWND g_hBtnEncrypt = NULL;
static HWND g_hBtnDecrypt = NULL;
static HWND g_hBtnSwap = NULL;
static HWND g_hBtnClear = NULL;
static HWND g_hBtnHelp = NULL;
static HWND g_hBtnCopy = NULL;
static HWND g_hStatusLabel = NULL;

static char g_szStatus[256] = "KCipher Ready. Select cipher, enter key/shift, and click Encrypt/Decrypt.";

// Cipher Logic
static void DoCaesar(const char* in, char* out, int shift, int decrypt) {
    if (decrypt) shift = (26 - (shift % 26)) % 26;
    shift = ((shift % 26) + 26) % 26;

    int i = 0;
    while (in[i]) {
        char c = in[i];
        if (c >= 'A' && c <= 'Z') {
            out[i] = (char)(((c - 'A' + shift) % 26) + 'A');
        } else if (c >= 'a' && c <= 'z') {
            out[i] = (char)(((c - 'a' + shift) % 26) + 'a');
        } else {
            out[i] = c;
        }
        i++;
    }
    out[i] = 0;
}

static void DoVigenere(const char* in, char* out, const char* key, int decrypt) {
    int keyLen = k_strlen(key);
    if (keyLen == 0) {
        k_strcpy(out, in);
        return;
    }

    int kIdx = 0;
    int i = 0;
    while (in[i]) {
        char c = in[i];
        char kChar = key[kIdx % keyLen];
        int shift = 0;
        if (kChar >= 'A' && kChar <= 'Z') shift = kChar - 'A';
        else if (kChar >= 'a' && kChar <= 'z') shift = kChar - 'a';

        if (decrypt) shift = (26 - shift) % 26;

        if (c >= 'A' && c <= 'Z') {
            out[i] = (char)(((c - 'A' + shift) % 26) + 'A');
            kIdx++;
        } else if (c >= 'a' && c <= 'z') {
            out[i] = (char)(((c - 'a' + shift) % 26) + 'a');
            kIdx++;
        } else {
            out[i] = c;
        }
        i++;
    }
    out[i] = 0;
}

static void DoAtbash(const char* in, char* out) {
    int i = 0;
    while (in[i]) {
        char c = in[i];
        if (c >= 'A' && c <= 'Z') {
            out[i] = (char)('Z' - (c - 'A'));
        } else if (c >= 'a' && c <= 'z') {
            out[i] = (char)('z' - (c - 'a'));
        } else {
            out[i] = c;
        }
        i++;
    }
    out[i] = 0;
}

static void DoRailFence(const char* in, char* out, int rails, int decrypt) {
    if (rails < 2) rails = 2;
    if (rails > 10) rails = 10;
    int len = k_strlen(in);
    if (len <= rails) {
        k_strcpy(out, in);
        return;
    }

    if (!decrypt) {
        int pos = 0;
        for (int r = 0; r < rails; r++) {
            int step = 2 * (rails - 1);
            for (int i = r; i < len; i += step) {
                out[pos++] = in[i];
                if (r > 0 && r < rails - 1) {
                    int mid = i + step - 2 * r;
                    if (mid < len) out[pos++] = in[mid];
                }
            }
        }
        out[pos] = 0;
    } else {
        // Decrypt
        char matrix[10][4096];
        memset(matrix, 0, sizeof(matrix));

        int rail = 0;
        int dir = 1;
        for (int i = 0; i < len; i++) {
            matrix[rail][i] = '*';
            rail += dir;
            if (rail == 0 || rail == rails - 1) dir = -dir;
        }

        int idx = 0;
        for (int r = 0; r < rails; r++) {
            for (int c = 0; c < len; c++) {
                if (matrix[r][c] == '*' && idx < len) {
                    matrix[r][c] = in[idx++];
                }
            }
        }

        rail = 0;
        dir = 1;
        for (int i = 0; i < len; i++) {
            out[i] = matrix[rail][i];
            rail += dir;
            if (rail == 0 || rail == rails - 1) dir = -dir;
        }
        out[len] = 0;
    }
}

// RC4 implementation
static void DoRC4(const char* in, char* out, const char* key, int decrypt) {
    int keyLen = k_strlen(key);
    if (keyLen == 0) key = "DEFAULT_KEY";
    keyLen = k_strlen(key);

    unsigned char s[256];
    for (int i = 0; i < 256; i++) s[i] = (unsigned char)i;

    int j = 0;
    for (int i = 0; i < 256; i++) {
        j = (j + s[i] + (unsigned char)key[i % keyLen]) & 0xFF;
        unsigned char tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
    }

    int i = 0;
    j = 0;

    if (!decrypt) {
        // Encrypt to hex
        int inLen = k_strlen(in);
        int outPos = 0;
        for (int k = 0; k < inLen; k++) {
            i = (i + 1) & 0xFF;
            j = (j + s[i]) & 0xFF;
            unsigned char tmp = s[i];
            s[i] = s[j];
            s[j] = tmp;
            unsigned char kByte = s[(s[i] + s[j]) & 0xFF];
            unsigned char c = ((unsigned char)in[k]) ^ kByte;

            const char hexChars[] = "0123456789ABCDEF";
            out[outPos++] = hexChars[(c >> 4) & 0x0F];
            out[outPos++] = hexChars[c & 0x0F];
        }
        out[outPos] = 0;
    } else {
        // Decrypt from hex
        int inLen = k_strlen(in);
        int outPos = 0;
        for (int k = 0; k < inLen; k += 2) {
            char h1 = in[k];
            char h2 = (k + 1 < inLen) ? in[k + 1] : '0';
            int v1 = (h1 >= '0' && h1 <= '9') ? (h1 - '0') : (h1 >= 'A' && h1 <= 'F' ? h1 - 'A' + 10 : (h1 >= 'a' && h1 <= 'f' ? h1 - 'a' + 10 : 0));
            int v2 = (h2 >= '0' && h2 <= '9') ? (h2 - '0') : (h2 >= 'A' && h2 <= 'F' ? h2 - 'A' + 10 : (h2 >= 'a' && h2 <= 'f' ? h2 - 'a' + 10 : 0));
            unsigned char encByte = (unsigned char)((v1 << 4) | v2);

            i = (i + 1) & 0xFF;
            j = (j + s[i]) & 0xFF;
            unsigned char tmp = s[i];
            s[i] = s[j];
            s[j] = tmp;
            unsigned char kByte = s[(s[i] + s[j]) & 0xFF];
            out[outPos++] = (char)(encByte ^ kByte);
        }
        out[outPos] = 0;
    }
}

static void ProcessCipher(int decrypt) {
    char inText[4096];
    char outText[4096];
    char keyText[128];
    memset(inText, 0, sizeof(inText));
    memset(outText, 0, sizeof(outText));
    memset(keyText, 0, sizeof(keyText));

    GetWindowTextA(decrypt ? g_hEditOutput : g_hEditInput, inText, sizeof(inText) - 1);
    GetWindowTextA(g_hEditKey, keyText, sizeof(keyText) - 1);

    int sel = (int)SendMessageA(g_hComboCipher, CB_GETCURSEL, 0, 0);

    switch (sel) {
        case 0: { // Caesar
            int shift = k_atoi(keyText);
            if (shift == 0) shift = 13;
            DoCaesar(inText, outText, shift, decrypt);
            break;
        }
        case 1: { // Vigenere
            if (keyText[0] == 0) k_strcpy(keyText, "KILO1999");
            DoVigenere(inText, outText, keyText, decrypt);
            break;
        }
        case 2: { // Rail Fence
            int rails = k_atoi(keyText);
            if (rails < 2) rails = 3;
            DoRailFence(inText, outText, rails, decrypt);
            break;
        }
        case 3: { // Atbash
            DoAtbash(inText, outText);
            break;
        }
        case 4: { // RC4
            if (keyText[0] == 0) k_strcpy(keyText, "ARCHITECT_1999");
            DoRC4(inText, outText, keyText, decrypt);
            break;
        }
        default:
            k_strcpy(outText, inText);
            break;
    }

    SetWindowTextA(decrypt ? g_hEditInput : g_hEditOutput, outText);

    // Update Status with metrics
    int inLen = k_strlen(inText);
    int outLen = k_strlen(outText);
    wsprintfA(g_szStatus, "Processed %d chars -> %d chars. Mode: %s.", inLen, outLen, decrypt ? "Decryption" : "Encryption");
    SetWindowTextA(g_hStatusLabel, g_szStatus);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_hwnd = hwnd;
            g_hBrushBg = CreateSolidBrush(COLOR_BG);
            g_hBrushCard = CreateSolidBrush(COLOR_CARD);

            g_hFontTitle = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            g_hFontBold = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            g_hFontNormal = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            g_hFontMono = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");

            // Controls
            g_hComboCipher = CreateWindowA("COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 120, 48, 220, 200, hwnd, (HMENU)ID_COMBO_CIPHER, NULL, NULL);
            SendMessageA(g_hComboCipher, CB_ADDSTRING, 0, (LPARAM)"Caesar / ROT-N (Key = Shift)");
            SendMessageA(g_hComboCipher, CB_ADDSTRING, 0, (LPARAM)"Vigenere (Key = Word)");
            SendMessageA(g_hComboCipher, CB_ADDSTRING, 0, (LPARAM)"Rail Fence (Key = Rails)");
            SendMessageA(g_hComboCipher, CB_ADDSTRING, 0, (LPARAM)"Atbash Mirror");
            SendMessageA(g_hComboCipher, CB_ADDSTRING, 0, (LPARAM)"RC4 Stream (Key = Passphrase)");
            SendMessageA(g_hComboCipher, CB_SETCURSEL, 0, 0);
            SendMessageA(g_hComboCipher, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            g_hEditKey = CreateWindowA("EDIT", "13", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 410, 48, 160, 24, hwnd, (HMENU)ID_EDIT_KEY, NULL, NULL);
            SendMessageA(g_hEditKey, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

            g_hBtnEncrypt = CreateWindowA("BUTTON", "Encrypt [Ctrl+E]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 590, 47, 120, 26, hwnd, (HMENU)ID_BTN_ENCRYPT, NULL, NULL);
            SendMessageA(g_hBtnEncrypt, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

            g_hBtnDecrypt = CreateWindowA("BUTTON", "Decrypt [Ctrl+D]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 720, 47, 120, 26, hwnd, (HMENU)ID_BTN_DECRYPT, NULL, NULL);
            SendMessageA(g_hBtnDecrypt, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

            // Text Areas
            g_hEditInput = CreateWindowA("EDIT", "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG.", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, 20, 110, 430, 360, hwnd, (HMENU)ID_EDIT_INPUT, NULL, NULL);
            SendMessageA(g_hEditInput, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

            g_hEditOutput = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, 470, 110, 430, 360, hwnd, (HMENU)ID_EDIT_OUTPUT, NULL, NULL);
            SendMessageA(g_hEditOutput, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

            // Action Buttons
            g_hBtnSwap = CreateWindowA("BUTTON", "Swap Panes", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 485, 100, 26, hwnd, (HMENU)ID_BTN_SWAP, NULL, NULL);
            SendMessageA(g_hBtnSwap, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            g_hBtnClear = CreateWindowA("BUTTON", "Clear All", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 130, 485, 90, 26, hwnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);
            SendMessageA(g_hBtnClear, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            g_hBtnCopy = CreateWindowA("BUTTON", "Copy Output", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 230, 485, 110, 26, hwnd, (HMENU)ID_BTN_COPY, NULL, NULL);
            SendMessageA(g_hBtnCopy, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            g_hBtnHelp = CreateWindowA("BUTTON", "Cipher Manual (F1)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 770, 485, 130, 26, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);
            SendMessageA(g_hBtnHelp, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            g_hStatusLabel = CreateWindowA("STATIC", g_szStatus, WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 525, 880, 20, hwnd, NULL, NULL, NULL);
            SendMessageA(g_hStatusLabel, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            ProcessCipher(0);
            return 0;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            if (wmId == ID_BTN_ENCRYPT) {
                ProcessCipher(0);
            } else if (wmId == ID_BTN_DECRYPT) {
                ProcessCipher(1);
            } else if (wmId == ID_BTN_SWAP) {
                char t1[4096], t2[4096];
                GetWindowTextA(g_hEditInput, t1, sizeof(t1));
                GetWindowTextA(g_hEditOutput, t2, sizeof(t2));
                SetWindowTextA(g_hEditInput, t2);
                SetWindowTextA(g_hEditOutput, t1);
            } else if (wmId == ID_BTN_CLEAR) {
                SetWindowTextA(g_hEditInput, "");
                SetWindowTextA(g_hEditOutput, "");
            } else if (wmId == ID_BTN_COPY) {
                char outText[4096];
                GetWindowTextA(g_hEditOutput, outText, sizeof(outText));
                int len = k_strlen(outText);
                if (len > 0 && OpenClipboard(hwnd)) {
                    EmptyClipboard();
                    HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, len + 1);
                    if (hGlob) {
                        char* p = (char*)GlobalLock(hGlob);
                        memcpy(p, outText, len + 1);
                        GlobalUnlock(hGlob);
                        SetClipboardData(CF_TEXT, hGlob);
                    }
                    CloseClipboard();
                    SetWindowTextA(g_hStatusLabel, "Output copied to Windows clipboard.");
                }
            } else if (wmId == ID_BTN_HELP) {
                MessageBoxA(hwnd,
                    "KCipher - Cryptographic Cipher Suite (v1.0.0 - 1999)\n\n"
                    "Supported Ciphers:\n"
                    "1. Caesar / ROT-N: Shift letter by integer N (1-25). ROT-13 uses 13.\n"
                    "2. Vigenere: Polyalphabetic key phrase shifting.\n"
                    "3. Rail Fence: Zig-zag transposition with N rails.\n"
                    "4. Atbash: Mirror substitution (A<->Z, B<->Y).\n"
                    "5. RC4: Stream cipher with passphrase (hex output).\n\n"
                    "Shortcuts:\n"
                    "F1: Manual | F5/F9: Session QuickSave/Load in KiloOS web edition.",
                    "KCipher Manual", MB_OK | MB_ICONINFORMATION);
            } else if (wmId == ID_COMBO_CIPHER && HIWORD(wParam) == CBN_SELCHANGE) {
                int sel = (int)SendMessageA(g_hComboCipher, CB_GETCURSEL, 0, 0);
                if (sel == 0) SetWindowTextA(g_hEditKey, "13");
                else if (sel == 1) SetWindowTextA(g_hEditKey, "KILO1999");
                else if (sel == 2) SetWindowTextA(g_hEditKey, "3");
                else if (sel == 3) SetWindowTextA(g_hEditKey, "(None)");
                else if (sel == 4) SetWindowTextA(g_hEditKey, "ARCHITECT_1999");
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, g_hBrushBg);

            SetBkMode(hdc, TRANSPARENT);

            // Title
            SelectObject(hdc, g_hFontTitle);
            SetTextColor(hdc, COLOR_PRIMARY);
            TextOutA(hdc, 20, 12, "KCipher - Cryptographic Cipher Suite", 36);

            SelectObject(hdc, g_hFontNormal);
            SetTextColor(hdc, COLOR_TEXT_MUTED);
            TextOutA(hdc, 400, 18, "v1.0.0 (1999 Edition)", 21);

            // Labels
            SelectObject(hdc, g_hFontBold);
            SetTextColor(hdc, COLOR_TEXT);
            TextOutA(hdc, 20, 51, "Cipher Engine:", 14);
            TextOutA(hdc, 360, 51, "Key / Shift:", 12);

            TextOutA(hdc, 20, 88, "Plaintext Input:", 16);
            TextOutA(hdc, 470, 88, "Ciphertext Output:", 18);

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
    wc.lpszClassName = "KCipherClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,
        "KCipherClass",
        "KCipher - Cryptographic Cipher Suite",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        940, 600,
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
