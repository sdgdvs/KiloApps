#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>

#pragma comment (lib, "crypt32.lib")
#pragma comment (lib, "advapi32.lib")
#pragma comment (lib, "user32.lib")
#pragma comment (lib, "gdi32.lib")

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

HWND hInput, hOutput, hBitDisplay, hEditA, hEditB, hBtnEnc, hBtnDec, hBtnHash;

// Custom String Helpers (No CRT dependencies)
static size_t my_strlen(const char* s) {
    size_t len = 0;
    while (s && s[len]) len++;
    return len;
}

static int parse_hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static UINT64 parse_u64(const char* str, int base) {
    UINT64 res = 0;
    if (!str) return 0;
    
    // Skip 0x / 0b / 0o prefix if present
    if (base == 16 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) str += 2;
    if (base == 2 && str[0] == '0' && (str[1] == 'b' || str[1] == 'B')) str += 2;

    while (*str) {
        int digit = -1;
        char c = *str;
        if (base == 16) {
            digit = parse_hex_digit(c);
        } else if (base == 10) {
            if (c >= '0' && c <= '9') digit = c - '0';
        } else if (base == 2) {
            if (c == '0' || c == '1') digit = c - '0';
        } else if (base == 8) {
            if (c >= '0' && c <= '7') digit = c - '0';
        }
        if (digit < 0 || digit >= base) break;
        res = res * base + digit;
        str++;
    }
    return res;
}

static void u64_to_hex(UINT64 val, char* outBuf) {
    wsprintfA(outBuf, "%08X%08X", (DWORD)(val >> 32), (DWORD)(val & 0xFFFFFFFF));
}

static void u64_to_bin(UINT64 val, char* outBuf) {
    for (int i = 63; i >= 0; i--) {
        outBuf[63 - i] = ((val >> i) & 1) ? '1' : '0';
    }
    outBuf[64] = '\0';
}

static void u64_to_dec(UINT64 val, char* outBuf) {
    if (val == 0) {
        outBuf[0] = '0';
        outBuf[1] = '\0';
        return;
    }
    char temp[32];
    int pos = 0;
    while (val > 0) {
        temp[pos++] = '0' + (char)(val % 10);
        val /= 10;
    }
    int outPos = 0;
    for (int i = pos - 1; i >= 0; i--) {
        outBuf[outPos++] = temp[i];
    }
    outBuf[outPos] = '\0';
}

// Window Controls
HWND hInput, hOutput, hBitDisplay;
HWND hEditA, hEditB;

// Core Utility Operations
void DoB64Encode() {
    DWORD len = GetWindowTextLengthA(hInput);
    if (len == 0) return;
    char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    GetWindowTextA(hInput, buf, len + 1);
    DWORD outLen = 0;
    CryptBinaryToStringA((const BYTE*)buf, len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &outLen);
    char* outBuf = (char*)HeapAlloc(GetProcessHeap(), 0, outLen + 1);
    CryptBinaryToStringA((const BYTE*)buf, len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, outBuf, &outLen);
    SetWindowTextA(hOutput, outBuf);
    HeapFree(GetProcessHeap(), 0, buf);
    HeapFree(GetProcessHeap(), 0, outBuf);
}

void DoB64Decode() {
    DWORD len = GetWindowTextLengthA(hInput);
    if (len == 0) return;
    char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    GetWindowTextA(hInput, buf, len + 1);
    DWORD outLen = 0;
    if (CryptStringToBinaryA(buf, len, CRYPT_STRING_BASE64, NULL, &outLen, NULL, NULL)) {
        char* outBuf = (char*)HeapAlloc(GetProcessHeap(), 0, outLen + 1);
        if (CryptStringToBinaryA(buf, len, CRYPT_STRING_BASE64, (BYTE*)outBuf, &outLen, NULL, NULL)) {
            outBuf[outLen] = 0;
            SetWindowTextA(hOutput, outBuf);
        } else {
            SetWindowTextA(hOutput, "Error decoding Base64.");
        }
        HeapFree(GetProcessHeap(), 0, outBuf);
    } else {
        SetWindowTextA(hOutput, "Error decoding Base64.");
    }
    HeapFree(GetProcessHeap(), 0, buf);
}

void DoSHA256() {
    DWORD len = GetWindowTextLengthA(hInput);
    if (len == 0) return;
    char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    GetWindowTextA(hInput, buf, len + 1);
    
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    if (CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        if (CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
            if (CryptHashData(hHash, (BYTE*)buf, len, 0)) {
                BYTE hash[32];
                DWORD hashLen = 32;
                if (CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
                    char hex[65];
                    for (int i = 0; i < 32; i++) {
                        wsprintfA(&hex[i*2], "%02x", hash[i]);
                    }
                    SetWindowTextA(hOutput, hex);
                }
            }
            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProv, 0);
    }
    HeapFree(GetProcessHeap(), 0, buf);
}

void DoUrlEncode() {
    DWORD len = GetWindowTextLengthA(hInput);
    if (len == 0) return;
    char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    GetWindowTextA(hInput, buf, len + 1);
    char* outBuf = (char*)HeapAlloc(GetProcessHeap(), 0, len * 3 + 1);
    char* p = outBuf;
    for (DWORD i = 0; i < len; i++) {
        unsigned char c = buf[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            *p++ = c;
        } else {
            p += wsprintfA(p, "%%%02X", c);
        }
    }
    *p = 0;
    SetWindowTextA(hOutput, outBuf);
    HeapFree(GetProcessHeap(), 0, buf);
    HeapFree(GetProcessHeap(), 0, outBuf);
}

void DoUrlDecode() {
    DWORD len = GetWindowTextLengthA(hInput);
    if (len == 0) return;
    char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    GetWindowTextA(hInput, buf, len + 1);
    char* outBuf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    char* p = outBuf;
    for (DWORD i = 0; i < len; i++) {
        if (buf[i] == '%' && i + 2 < len) {
            char hex[3] = { buf[i+1], buf[i+2], 0 };
            *p++ = (char)parse_hex_digit(hex[0]) * 16 + parse_hex_digit(hex[1]);
            i += 2;
        } else if (buf[i] == '+') {
            *p++ = ' ';
        } else {
            *p++ = buf[i];
        }
    }
    *p = 0;
    SetWindowTextA(hOutput, outBuf);
    HeapFree(GetProcessHeap(), 0, buf);
    HeapFree(GetProcessHeap(), 0, outBuf);
}

void DoHexEncode() {
    DWORD len = GetWindowTextLengthA(hInput);
    if (len == 0) return;
    char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    GetWindowTextA(hInput, buf, len + 1);
    char* outBuf = (char*)HeapAlloc(GetProcessHeap(), 0, len * 3 + 1);
    char* p = outBuf;
    for (DWORD i = 0; i < len; i++) {
        if (i > 0) p += wsprintfA(p, " %02X", (unsigned char)buf[i]);
        else p += wsprintfA(p, "%02X", (unsigned char)buf[i]);
    }
    *p = 0;
    SetWindowTextA(hOutput, outBuf);
    HeapFree(GetProcessHeap(), 0, buf);
    HeapFree(GetProcessHeap(), 0, outBuf);
}

void DoHexDecode() {
    DWORD len = GetWindowTextLengthA(hInput);
    if (len == 0) return;
    char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    GetWindowTextA(hInput, buf, len + 1);
    char* outBuf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    DWORD outLen = 0;
    for (DWORD i = 0; i < len; i++) {
        char c = buf[i];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') continue;
        if (i + 1 < len) {
            int d1 = parse_hex_digit(buf[i]);
            int d2 = parse_hex_digit(buf[i+1]);
            if (d1 >= 0 && d2 >= 0) {
                outBuf[outLen++] = (char)(d1 * 16 + d2);
                i++;
            }
        }
    }
    outBuf[outLen] = 0;
    SetWindowTextA(hOutput, outBuf);
    HeapFree(GetProcessHeap(), 0, buf);
    HeapFree(GetProcessHeap(), 0, outBuf);
}

// Convert Base live
void DoConvertBases() {
    char buf[128];
    GetWindowTextA(hInput, buf, 128);
    UINT64 val = parse_u64(buf, 10);
    if (val == 0 && buf[0] != '0') {
        val = parse_u64(buf, 16);
    }
    
    char hex[32], dec[32], bin[68], formatted[256];
    u64_to_hex(val, hex);
    u64_to_dec(val, dec);
    u64_to_bin(val, bin);

    wsprintfA(formatted, "DEC: %s\r\nHEX: 0x%s\r\nBIN: %s", dec, hex, bin);
    SetWindowTextA(hOutput, formatted);
    SetWindowTextA(hBitDisplay, bin);
}

// Bitwise operations (AND, OR, XOR, NOT, SHL, SHR, ROL, ROR)
void DoBitwiseOp(int op) {
    char bufA[64], bufB[64];
    GetWindowTextA(hEditA, bufA, 64);
    GetWindowTextA(hEditB, bufB, 64);

    UINT64 valA = parse_u64(bufA, 16);
    if (valA == 0 && bufA[0] != '0') valA = parse_u64(bufA, 10);

    UINT64 valB = parse_u64(bufB, 16);
    if (valB == 0 && bufB[0] != '0') valB = parse_u64(bufB, 10);

    UINT64 res = 0;
    const char* opName = "";

    switch (op) {
        case 10: res = valA & valB; opName = "AND"; break;
        case 11: res = valA | valB; opName = "OR"; break;
        case 12: res = valA ^ valB; opName = "XOR"; break;
        case 13: res = ~valA; opName = "NOT(A)"; break;
        case 14: res = valA << (valB & 63); opName = "SHL"; break;
        case 15: res = valA >> (valB & 63); opName = "SHR"; break;
        case 16: {
            UINT64 cnt = valB & 63;
            res = (valA << cnt) | (valA >> (64 - cnt));
            opName = "ROL";
            break;
        }
        case 17: {
            UINT64 cnt = valB & 63;
            res = (valA >> cnt) | (valA << (64 - cnt));
            opName = "ROR";
            break;
        }
    }

    char hex[32], dec[32], bin[68], outStr[256];
    u64_to_hex(res, hex);
    u64_to_dec(res, dec);
    u64_to_bin(res, bin);

    wsprintfA(outStr, "[Bitwise %s Result]\r\nHEX: 0x%s\r\nDEC: %s\r\nBIN: %s", opName, hex, dec, bin);
    SetWindowTextA(hOutput, outStr);
    SetWindowTextA(hBitDisplay, bin);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 5 /* CLEARTYPE_QUALITY */, DEFAULT_PITCH, "Consolas");
            
            CreateWindowA("STATIC", "Input Buffer / Number:", WS_CHILD | WS_VISIBLE, 10, 10, 200, 18, hwnd, NULL, NULL, NULL);
            HWND hBtnHelp = CreateWindowA("BUTTON", "Help (F1)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 510, 7, 100, 20, hwnd, (HMENU)99, NULL, NULL);
            SendMessageA(hBtnHelp, WM_SETFONT, (WPARAM)hFont, 0);

            hInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "42", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN,
                10, 30, 600, 50, hwnd, NULL, NULL, NULL);
            SendMessageA(hInput, WM_SETFONT, (WPARAM)hFont, 0);

            // Operands A & B for bitwise calculations
            CreateWindowA("STATIC", "Operand A (Hex/Dec):", WS_CHILD | WS_VISIBLE, 10, 88, 160, 18, hwnd, NULL, NULL, NULL);
            hEditA = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "0x0F0F0F0F", WS_CHILD | WS_VISIBLE, 170, 85, 120, 22, hwnd, NULL, NULL, NULL);
            SendMessageA(hEditA, WM_SETFONT, (WPARAM)hFont, 0);

            CreateWindowA("STATIC", "Operand B / Shift:", WS_CHILD | WS_VISIBLE, 300, 88, 140, 18, hwnd, NULL, NULL, NULL);
            hEditB = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "0x00FF00FF", WS_CHILD | WS_VISIBLE, 445, 85, 120, 22, hwnd, NULL, NULL, NULL);
            SendMessageA(hEditB, WM_SETFONT, (WPARAM)hFont, 0);

            // Action Buttons - Base & Text Encoding
            HWND hBtnConv = CreateWindowA("BUTTON", "Convert Base", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 115, 100, 24, hwnd, (HMENU)100, NULL, NULL);
            SendMessageA(hBtnConv, WM_SETFONT, (WPARAM)hFont, 0);

            hBtnEnc = CreateWindowA("BUTTON", "B64 Enc", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 115, 115, 75, 24, hwnd, (HMENU)1, NULL, NULL);
            SendMessageA(hBtnEnc, WM_SETFONT, (WPARAM)hFont, 0);
            
            hBtnDec = CreateWindowA("BUTTON", "B64 Dec", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 195, 115, 75, 24, hwnd, (HMENU)2, NULL, NULL);
            SendMessageA(hBtnDec, WM_SETFONT, (WPARAM)hFont, 0);
            
            HWND hBtnUrlEnc = CreateWindowA("BUTTON", "URL Enc", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 275, 115, 75, 24, hwnd, (HMENU)4, NULL, NULL);
            SendMessageA(hBtnUrlEnc, WM_SETFONT, (WPARAM)hFont, 0);

            HWND hBtnUrlDec = CreateWindowA("BUTTON", "URL Dec", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 355, 115, 75, 24, hwnd, (HMENU)5, NULL, NULL);
            SendMessageA(hBtnUrlDec, WM_SETFONT, (WPARAM)hFont, 0);

            hBtnHash = CreateWindowA("BUTTON", "SHA-256", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 435, 115, 80, 24, hwnd, (HMENU)3, NULL, NULL);
            SendMessageA(hBtnHash, WM_SETFONT, (WPARAM)hFont, 0);

            // Bitwise Operator Buttons
            HWND hBtnAnd = CreateWindowA("BUTTON", "AND", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 145, 60, 24, hwnd, (HMENU)10, NULL, NULL);
            SendMessageA(hBtnAnd, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnOr  = CreateWindowA("BUTTON", "OR", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 75, 145, 60, 24, hwnd, (HMENU)11, NULL, NULL);
            SendMessageA(hBtnOr, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnXor = CreateWindowA("BUTTON", "XOR", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 140, 145, 60, 24, hwnd, (HMENU)12, NULL, NULL);
            SendMessageA(hBtnXor, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnNot = CreateWindowA("BUTTON", "NOT(A)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 205, 145, 65, 24, hwnd, (HMENU)13, NULL, NULL);
            SendMessageA(hBtnNot, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnShl = CreateWindowA("BUTTON", "SHL", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 275, 145, 60, 24, hwnd, (HMENU)14, NULL, NULL);
            SendMessageA(hBtnShl, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnShr = CreateWindowA("BUTTON", "SHR", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 340, 145, 60, 24, hwnd, (HMENU)15, NULL, NULL);
            SendMessageA(hBtnShr, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnRol = CreateWindowA("BUTTON", "ROL", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 405, 145, 60, 24, hwnd, (HMENU)16, NULL, NULL);
            SendMessageA(hBtnRol, WM_SETFONT, (WPARAM)hFont, 0);
            HWND hBtnRor = CreateWindowA("BUTTON", "ROR", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 470, 145, 60, 24, hwnd, (HMENU)17, NULL, NULL);
            SendMessageA(hBtnRor, WM_SETFONT, (WPARAM)hFont, 0);

            // 64-Bit Binary Stream Display
            CreateWindowA("STATIC", "64-Bit Binary Stream:", WS_CHILD | WS_VISIBLE, 10, 178, 200, 18, hwnd, NULL, NULL, NULL);
            hBitDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "0000000000000000000000000000000000000000000000000000000000000000",
                WS_CHILD | WS_VISIBLE | ES_READONLY, 10, 196, 600, 24, hwnd, NULL, NULL, NULL);
            SendMessageA(hBitDisplay, WM_SETFONT, (WPARAM)hFont, 0);

            // Output Display Area
            CreateWindowA("STATIC", "Output Result:", WS_CHILD | WS_VISIBLE, 10, 226, 200, 18, hwnd, NULL, NULL, NULL);
            hOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                10, 245, 600, 150, hwnd, NULL, NULL, NULL);
            SendMessageA(hOutput, WM_SETFONT, (WPARAM)hFont, 0);

            // Trigger default conversion
            DoConvertBases();
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == 1) DoB64Encode();
            else if (id == 2) DoB64Decode();
            else if (id == 3) DoSHA256();
            else if (id == 4) DoUrlEncode();
            else if (id == 5) DoUrlDecode();
            else if (id == 6) DoHexEncode();
            else if (id == 7) DoHexDecode();
            else if (id == 99) MessageBoxA(hwnd, "KBase Help:\n\n- Convert Base: Type number and click Convert.\n- Bitwise: Enter Operand A & B as Hex/Dec.\n- Strings: Encode/Decode/Hash text.\n- F1: Show this help dialog.", "KBase Help", MB_OK | MB_ICONINFORMATION);
            else if (id == 100) DoConvertBases();
            else if (id >= 10 && id <= 17) DoBitwiseOp(id);
            break;
        }
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(0, 255, 0));
            SetBkColor(hdc, RGB(0, 0, 0));
            return (LRESULT)GetStockObject(BLACK_BRUSH);
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void __stdcall MainEntry() {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KBaseApp";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    
    RegisterClassA(&wc);
    
    RECT rc = {0, 0, 640, 480};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, FALSE);
    
    HWND hwnd = CreateWindowExA(0, "KBaseApp", "KBase - Universal Base & Bitwise Utility", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
        
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_F1) {
            SendMessageA(hwnd, WM_COMMAND, 99, 0);
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
