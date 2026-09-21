#include <windows.h>
#include <wincrypt.h>
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

// Control IDs
#define ID_EDIT_INPUT       101
#define ID_BTN_COMPUTE      102
#define ID_BTN_BROWSE       103
#define ID_EDIT_FILE        104
#define ID_EDIT_CRC32       105
#define ID_EDIT_ADLER32     106
#define ID_EDIT_FNV1A       107
#define ID_EDIT_MD5         108
#define ID_EDIT_SHA1        109
#define ID_EDIT_SHA256      110
#define ID_EDIT_EXPECTED    111
#define ID_BTN_VERIFY       112
#define ID_BTN_HELP         113
#define ID_BTN_SAVE         114
#define ID_BTN_LOAD         115
#define ID_BTN_CLEAR        116

// Colors
static COLORREF COLOR_BG = RGB(11, 15, 25);
static COLORREF COLOR_CARD = RGB(21, 30, 50);
static COLORREF COLOR_TEXT = RGB(241, 245, 249);
static COLORREF COLOR_TEXT_MUTED = RGB(148, 163, 184);
static COLORREF COLOR_PRIMARY = RGB(56, 189, 248);
static COLORREF COLOR_ACCENT = RGB(245, 158, 11);
static COLORREF COLOR_SUCCESS = RGB(16, 185, 129);
static COLORREF COLOR_DANGER = RGB(239, 68, 68);

// GDI Brushes & Fonts
static HBRUSH g_hBrushBg = NULL;
static HBRUSH g_hBrushCard = NULL;
static HBRUSH g_hBrushInput = NULL;
static HFONT g_hFontTitle = NULL;
static HFONT g_hFontBold = NULL;
static HFONT g_hFontMono = NULL;
static HFONT g_hFontNormal = NULL;

// Window handles
static HWND g_hwnd = NULL;
static HWND g_hEditInput = NULL;
static HWND g_hEditFile = NULL;
static HWND g_hBtnBrowse = NULL;
static HWND g_hBtnCompute = NULL;
static HWND g_hEditCRC32 = NULL;
static HWND g_hEditAdler32 = NULL;
static HWND g_hEditFNV1a = NULL;
static HWND g_hEditMD5 = NULL;
static HWND g_hEditSHA1 = NULL;
static HWND g_hEditSHA256 = NULL;
static HWND g_hEditExpected = NULL;
static HWND g_hBtnVerify = NULL;
static HWND g_hBtnHelp = NULL;
static HWND g_hBtnSave = NULL;
static HWND g_hBtnLoad = NULL;
static HWND g_hBtnClear = NULL;
static HWND g_hStatusLabel = NULL;

static char g_szStatus[256] = "KHash Ready. Enter payload, browse file, or press [F1] for Help.";
static char g_szCrc32[16] = "00000000";
static char g_szAdler32[16] = "00000001";
static char g_szFnv1a[16] = "811C9DC5";
static char g_szMd5[64] = "D41D8CD98F00B204E9800998ECF8427E";
static char g_szSha1[64] = "DA39A3EE5E6B4B0D3255BFEF95601890AFD80709";
static char g_szSha256[128] = "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855";

// Checksum Tables & Engines
static DWORD g_crcTable[256];
static int g_crcInit = 0;

static void InitCrcTable(void) {
    if (g_crcInit) return;
    for (DWORD i = 0; i < 256; i++) {
        DWORD c = i;
        for (int k = 0; k < 8; k++) {
            c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
        }
        g_crcTable[i] = c;
    }
    g_crcInit = 1;
}

static DWORD ComputeCRC32(const BYTE* data, DWORD len) {
    InitCrcTable();
    DWORD crc = 0xFFFFFFFF;
    for (DWORD i = 0; i < len; i++) {
        crc = g_crcTable[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

static DWORD ComputeAdler32(const BYTE* data, DWORD len) {
    const DWORD MOD_ADLER = 65521;
    DWORD a = 1, b = 0;
    for (DWORD i = 0; i < len; i++) {
        a = (a + data[i]) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }
    return (b << 16) | a;
}

static DWORD ComputeFNV1a32(const BYTE* data, DWORD len) {
    DWORD hash = 0x811C9DC5;
    for (DWORD i = 0; i < len; i++) {
        hash ^= data[i];
        hash *= 0x01000193;
    }
    return hash;
}

static void BinToHex(const BYTE* data, DWORD len, char* outHex, int uppercase) {
    static const char hexCharsLower[] = "0123456789abcdef";
    static const char hexCharsUpper[] = "0123456789ABCDEF";
    const char* lut = uppercase ? hexCharsUpper : hexCharsLower;
    for (DWORD i = 0; i < len; i++) {
        outHex[i * 2] = lut[(data[i] >> 4) & 0x0F];
        outHex[i * 2 + 1] = lut[data[i] & 0x0F];
    }
    outHex[len * 2] = '\0';
}

static BOOL ComputeCryptoApiHash(ALG_ID algId, const BYTE* data, DWORD len, char* outHex) {
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    BOOL bSuccess = FALSE;

    if (CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        if (CryptCreateHash(hProv, algId, 0, 0, &hHash)) {
            if (CryptHashData(hHash, data, len, 0)) {
                DWORD hashLen = 64;
                BYTE hashVal[64];
                if (CryptGetHashParam(hHash, HP_HASHVAL, hashVal, &hashLen, 0)) {
                    BinToHex(hashVal, hashLen, outHex, 1);
                    bSuccess = TRUE;
                }
            }
            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProv, 0);
    }
    return bSuccess;
}

// Compute all checksums on in-memory buffer
static void ComputeAllChecksums(const BYTE* buf, DWORD len) {
    DWORD crc = ComputeCRC32(buf, len);
    wsprintfA(g_szCrc32, "%08X", crc);
    SetWindowTextA(g_hEditCRC32, g_szCrc32);

    DWORD adler = ComputeAdler32(buf, len);
    wsprintfA(g_szAdler32, "%08X", adler);
    SetWindowTextA(g_hEditAdler32, g_szAdler32);

    DWORD fnv = ComputeFNV1a32(buf, len);
    wsprintfA(g_szFnv1a, "%08X", fnv);
    SetWindowTextA(g_hEditFNV1a, g_szFnv1a);

    // CryptoAPI Hashes
    if (!ComputeCryptoApiHash(CALG_MD5, buf, len, g_szMd5)) {
        wsprintfA(g_szMd5, "UNAVAILABLE");
    }
    SetWindowTextA(g_hEditMD5, g_szMd5);

    if (!ComputeCryptoApiHash(CALG_SHA1, buf, len, g_szSha1)) {
        wsprintfA(g_szSha1, "UNAVAILABLE");
    }
    SetWindowTextA(g_hEditSHA1, g_szSha1);

    if (!ComputeCryptoApiHash(CALG_SHA_256, buf, len, g_szSha256)) {
        wsprintfA(g_szSha256, "UNAVAILABLE");
    }
    SetWindowTextA(g_hEditSHA256, g_szSha256);

    wsprintfA(g_szStatus, "Computed 6 algorithmic checksums on %d bytes.", len);
    SetWindowTextA(g_hStatusLabel, g_szStatus);
}

static void HashFromTextPayload(void) {
    int len = GetWindowTextLengthA(g_hEditInput);
    char* buf = (char*)GlobalAlloc(GPTR, len + 1);
    if (buf) {
        GetWindowTextA(g_hEditInput, buf, len + 1);
        ComputeAllChecksums((const BYTE*)buf, (DWORD)len);
        GlobalFree(buf);
    }
}

static void HashFromFile(const char* szPath) {
    HANDLE hFile = CreateFileA(szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        SetWindowTextA(g_hStatusLabel, "Error: Could not open specified file.");
        return;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE || fileSize > 64 * 1024 * 1024) { // limit single buffer to 64MB for safety
        CloseHandle(hFile);
        SetWindowTextA(g_hStatusLabel, "File size exceeds 64MB limit for in-memory buffer.");
        return;
    }

    BYTE* buf = (BYTE*)GlobalAlloc(GPTR, fileSize);
    if (!buf) {
        CloseHandle(hFile);
        SetWindowTextA(g_hStatusLabel, "Memory allocation error.");
        return;
    }

    DWORD bytesRead = 0;
    if (ReadFile(hFile, buf, fileSize, &bytesRead, NULL)) {
        ComputeAllChecksums(buf, bytesRead);
    } else {
        SetWindowTextA(g_hStatusLabel, "Error reading file data.");
    }

    GlobalFree(buf);
    CloseHandle(hFile);
}

static void BrowseForFile(HWND hwnd) {
    char szFileName[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        SetWindowTextA(g_hEditFile, szFileName);
        HashFromFile(szFileName);
    }
}

static void VerifyIntegrity(void) {
    char szExp[256] = "";
    GetWindowTextA(g_hEditExpected, szExp, sizeof(szExp));

    // Clean whitespace
    int j = 0;
    char cleanExp[256];
    for (int i = 0; szExp[i]; i++) {
        if (szExp[i] != ' ' && szExp[i] != '\t' && szExp[i] != '\r' && szExp[i] != '\n') {
            char c = szExp[i];
            if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
            cleanExp[j++] = c;
        }
    }
    cleanExp[j] = '\0';

    if (j == 0) {
        SetWindowTextA(g_hStatusLabel, "Please enter an expected checksum to verify against.");
        return;
    }

    // Match against our calculated hashes
    BOOL bMatch = FALSE;
    const char* matchedAlgo = "";

    if (lstrcmpiA(cleanExp, g_szCrc32) == 0) { bMatch = TRUE; matchedAlgo = "CRC32"; }
    else if (lstrcmpiA(cleanExp, g_szAdler32) == 0) { bMatch = TRUE; matchedAlgo = "Adler-32"; }
    else if (lstrcmpiA(cleanExp, g_szFnv1a) == 0) { bMatch = TRUE; matchedAlgo = "FNV-1a"; }
    else if (lstrcmpiA(cleanExp, g_szMd5) == 0) { bMatch = TRUE; matchedAlgo = "MD5"; }
    else if (lstrcmpiA(cleanExp, g_szSha1) == 0) { bMatch = TRUE; matchedAlgo = "SHA-1"; }
    else if (lstrcmpiA(cleanExp, g_szSha256) == 0) { bMatch = TRUE; matchedAlgo = "SHA-256"; }

    if (bMatch) {
        char msg[256];
        wsprintfA(msg, "INTEGRITY MATCH! Perfect match on %s (%d-bit).", matchedAlgo, j * 4);
        SetWindowTextA(g_hStatusLabel, msg);
        MessageBoxA(g_hwnd, msg, "KHash Verification Passed", MB_OK | MB_ICONINFORMATION);
    } else {
        SetWindowTextA(g_hStatusLabel, "INTEGRITY MISMATCH! Expected hash does not match any calculated digest.");
        MessageBoxA(g_hwnd, "Calculated checksums do NOT match expected value!\nCheck input data or algorithm type.", "KHash Mismatch Alert", MB_OK | MB_ICONWARNING);
    }
}

static void QuickSaveState(void) {
    HANDLE hFile = CreateFileA("khash.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        char buf[1024] = "";
        GetWindowTextA(g_hEditInput, buf, sizeof(buf));
        WriteFile(hFile, buf, sizeof(buf), &written, NULL);
        GetWindowTextA(g_hEditExpected, buf, sizeof(buf));
        WriteFile(hFile, buf, sizeof(buf), &written, NULL);
        CloseHandle(hFile);
        SetWindowTextA(g_hStatusLabel, "Workstation state quicksaved to khash.dat (F5).");
    }
}

static void QuickLoadState(void) {
    HANDLE hFile = CreateFileA("khash.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD read = 0;
        char buf[1024] = "";
        if (ReadFile(hFile, buf, sizeof(buf), &read, NULL)) {
            SetWindowTextA(g_hEditInput, buf);
        }
        if (ReadFile(hFile, buf, sizeof(buf), &read, NULL)) {
            SetWindowTextA(g_hEditExpected, buf);
        }
        CloseHandle(hFile);
        HashFromTextPayload();
        SetWindowTextA(g_hStatusLabel, "Workstation state quickloaded from khash.dat (F9).");
    } else {
        SetWindowTextA(g_hStatusLabel, "No saved state (khash.dat) found.");
    }
}

static void ShowHelp(HWND hwnd) {
    const char* helpText =
        "KHASH - MULTI-ALGORITHM CHECKSUM WORKSTATION\r\n"
        "============================================\r\n\r\n"
        "CORE FEATURES:\r\n"
        "  - Text Payload Hashing: Real-time calculation on arbitrary text strings.\r\n"
        "  - File Checksumming: Inspect and verify any file via Open Dialog.\r\n"
        "  - Multi-Engine: CRC32, Adler-32, FNV-1a, MD5, SHA-1, SHA-256.\r\n"
        "  - Integrity Verification: Compare against vendor or expected checksum.\r\n\r\n"
        "KEYBOARD SHORTCUTS:\r\n"
        "  [F1]        Open this Help & Reference manual\r\n"
        "  [F5]        Quicksave active state to khash.dat\r\n"
        "  [F9]        Quickload saved state from khash.dat\r\n"
        "  [Enter]     Compute hashes / Verify expected checksum\r\n"
        "  [Esc]       Clear input fields\r\n\r\n"
        "KiloApps Fleet Sovereign Cryptographic Tool // Retro OS Series";
    MessageBoxA(hwnd, helpText, "KHash Help & Quick Reference", MB_OK | MB_ICONINFORMATION);
}

// Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_hBrushBg = CreateSolidBrush(COLOR_BG);
            g_hBrushCard = CreateSolidBrush(COLOR_CARD);
            g_hBrushInput = CreateSolidBrush(RGB(15, 23, 42));

            g_hFontTitle = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 0, 0, DEFAULT_QUALITY, 0, "Segoe UI");
            g_hFontBold = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 0, 0, DEFAULT_QUALITY, 0, "Segoe UI");
            g_hFontNormal = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, 0, 0, DEFAULT_QUALITY, 0, "Segoe UI");
            g_hFontMono = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, 0, 0, DEFAULT_QUALITY, 0, "Consolas");

            // Title Banner
            CreateWindowExA(0, "STATIC", "KHash - Multi-Algorithm Checksum Workstation", WS_CHILD | WS_VISIBLE,
                16, 12, 450, 24, hwnd, NULL, NULL, NULL);

            // Text Payload Section
            CreateWindowExA(0, "STATIC", "Text Payload Input:", WS_CHILD | WS_VISIBLE, 16, 44, 200, 18, hwnd, NULL, NULL, NULL);
            g_hEditInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "KiloOS Sovereign Checksum Engine",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
                16, 64, 520, 54, hwnd, (HMENU)ID_EDIT_INPUT, NULL, NULL);

            g_hBtnCompute = CreateWindowExA(0, "BUTTON", "⚡ Hash Text", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                546, 64, 110, 25, hwnd, (HMENU)ID_BTN_COMPUTE, NULL, NULL);
            g_hBtnClear = CreateWindowExA(0, "BUTTON", "🧹 Clear", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                546, 93, 110, 25, hwnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);

            // File Checksum Section
            CreateWindowExA(0, "STATIC", "File Inspection & Hashing:", WS_CHILD | WS_VISIBLE, 16, 126, 200, 18, hwnd, NULL, NULL, NULL);
            g_hEditFile = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_READONLY,
                16, 146, 520, 24, hwnd, (HMENU)ID_EDIT_FILE, NULL, NULL);
            g_hBtnBrowse = CreateWindowExA(0, "BUTTON", "📁 Browse File...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                546, 145, 110, 26, hwnd, (HMENU)ID_BTN_BROWSE, NULL, NULL);

            // Algorithm Output Fields
            int y = 184;
            int h = 24;
            int gap = 28;

            CreateWindowExA(0, "STATIC", "CRC32 (IEEE):", WS_CHILD | WS_VISIBLE, 16, y, 100, 20, hwnd, NULL, NULL, NULL);
            g_hEditCRC32 = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", g_szCrc32, WS_CHILD | WS_VISIBLE | ES_READONLY, 120, y, 160, h, hwnd, (HMENU)ID_EDIT_CRC32, NULL, NULL);

            CreateWindowExA(0, "STATIC", "Adler-32:", WS_CHILD | WS_VISIBLE, 300, y, 80, 20, hwnd, NULL, NULL, NULL);
            g_hEditAdler32 = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", g_szAdler32, WS_CHILD | WS_VISIBLE | ES_READONLY, 380, y, 160, h, hwnd, (HMENU)ID_EDIT_ADLER32, NULL, NULL);

            CreateWindowExA(0, "STATIC", "FNV-1a 32:", WS_CHILD | WS_VISIBLE, 555, y, 75, 20, hwnd, NULL, NULL, NULL);
            g_hEditFNV1a = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", g_szFnv1a, WS_CHILD | WS_VISIBLE | ES_READONLY, 635, y, 140, h, hwnd, (HMENU)ID_EDIT_FNV1A, NULL, NULL);

            y += gap;
            CreateWindowExA(0, "STATIC", "MD5 (128-bit):", WS_CHILD | WS_VISIBLE, 16, y, 100, 20, hwnd, NULL, NULL, NULL);
            g_hEditMD5 = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", g_szMd5, WS_CHILD | WS_VISIBLE | ES_READONLY, 120, y, 420, h, hwnd, (HMENU)ID_EDIT_MD5, NULL, NULL);

            y += gap;
            CreateWindowExA(0, "STATIC", "SHA-1 (160-bit):", WS_CHILD | WS_VISIBLE, 16, y, 100, 20, hwnd, NULL, NULL, NULL);
            g_hEditSHA1 = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", g_szSha1, WS_CHILD | WS_VISIBLE | ES_READONLY, 120, y, 520, h, hwnd, (HMENU)ID_EDIT_SHA1, NULL, NULL);

            y += gap;
            CreateWindowExA(0, "STATIC", "SHA-256 (256-bit):", WS_CHILD | WS_VISIBLE, 16, y, 100, 20, hwnd, NULL, NULL, NULL);
            g_hEditSHA256 = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", g_szSha256, WS_CHILD | WS_VISIBLE | ES_READONLY, 120, y, 655, h, hwnd, (HMENU)ID_EDIT_SHA256, NULL, NULL);

            // Verification Section
            y += gap + 8;
            CreateWindowExA(0, "STATIC", "Expected Hash (Compare / Verify):", WS_CHILD | WS_VISIBLE, 16, y, 220, 20, hwnd, NULL, NULL, NULL);
            y += 20;
            g_hEditExpected = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE, 16, y, 650, 24, hwnd, (HMENU)ID_EDIT_EXPECTED, NULL, NULL);
            g_hBtnVerify = CreateWindowExA(0, "BUTTON", "⚖️ Verify", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 676, y - 1, 100, 26, hwnd, (HMENU)ID_BTN_VERIFY, NULL, NULL);

            // Quick Toolbar Buttons
            y += 36;
            g_hBtnSave = CreateWindowExA(0, "BUTTON", "💾 Save [F5]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 16, y, 100, 26, hwnd, (HMENU)ID_BTN_SAVE, NULL, NULL);
            g_hBtnLoad = CreateWindowExA(0, "BUTTON", "📂 Load [F9]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 124, y, 100, 26, hwnd, (HMENU)ID_BTN_LOAD, NULL, NULL);
            g_hBtnHelp = CreateWindowExA(0, "BUTTON", "❓ Help [F1]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 232, y, 100, 26, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);

            // Status Bar Label
            y += 36;
            g_hStatusLabel = CreateWindowExA(0, "STATIC", g_szStatus, WS_CHILD | WS_VISIBLE, 16, y, 760, 20, hwnd, NULL, NULL, NULL);

            // Apply Monospace Font to Hashes and Inputs
            SendMessageA(g_hEditInput, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);
            SendMessageA(g_hEditFile, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);
            SendMessageA(g_hEditCRC32, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);
            SendMessageA(g_hEditAdler32, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);
            SendMessageA(g_hEditFNV1a, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);
            SendMessageA(g_hEditMD5, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);
            SendMessageA(g_hEditSHA1, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);
            SendMessageA(g_hEditSHA256, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);
            SendMessageA(g_hEditExpected, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

            // Compute initial hashes
            HashFromTextPayload();
            break;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case ID_BTN_COMPUTE:
                    HashFromTextPayload();
                    break;
                case ID_BTN_BROWSE:
                    BrowseForFile(hwnd);
                    break;
                case ID_BTN_CLEAR:
                    SetWindowTextA(g_hEditInput, "");
                    SetWindowTextA(g_hEditExpected, "");
                    HashFromTextPayload();
                    break;
                case ID_BTN_VERIFY:
                    VerifyIntegrity();
                    break;
                case ID_BTN_SAVE:
                    QuickSaveState();
                    break;
                case ID_BTN_LOAD:
                    QuickLoadState();
                    break;
                case ID_BTN_HELP:
                    ShowHelp(hwnd);
                    break;
            }
            break;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, COLOR_TEXT);
            SetBkColor(hdc, COLOR_BG);
            return (LRESULT)g_hBrushBg;
        }

        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, COLOR_PRIMARY);
            SetBkColor(hdc, RGB(15, 23, 42));
            return (LRESULT)g_hBrushInput;
        }

        case WM_DESTROY:
            if (g_hBrushBg) DeleteObject(g_hBrushBg);
            if (g_hBrushCard) DeleteObject(g_hBrushCard);
            if (g_hBrushInput) DeleteObject(g_hBrushInput);
            if (g_hFontTitle) DeleteObject(g_hFontTitle);
            if (g_hFontBold) DeleteObject(g_hFontBold);
            if (g_hFontNormal) DeleteObject(g_hFontNormal);
            if (g_hFontMono) DeleteObject(g_hFontMono);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Entry Point
void __cdecl MainEntry(void) {
    HINSTANCE hInstance = GetModuleHandleA(NULL);

    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KHashWndClass";
    wc.hbrBackground = CreateSolidBrush(COLOR_BG);
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(1));

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,
        "KHashWndClass",
        "KHash - Multi-Algorithm Checksum Workstation",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 820, 480,
        NULL, NULL, hInstance, NULL
    );

    g_hwnd = hwnd;
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_F1) {
                ShowHelp(hwnd);
                continue;
            }
            if (msg.wParam == VK_F5) {
                QuickSaveState();
                continue;
            }
            if (msg.wParam == VK_F9) {
                QuickLoadState();
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    ExitProcess(0);
}
