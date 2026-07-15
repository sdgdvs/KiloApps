#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>

#pragma comment (lib, "crypt32.lib")
#pragma comment (lib, "advapi32.lib")
#pragma comment (lib, "user32.lib")

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) *bytes++ = (char)c;
    return dest;
}

HWND hInput, hOutput;
HWND hBtnEnc, hBtnDec, hBtnHash;

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
            SetWindowTextA(hOutput, "Error decoding.");
        }
        HeapFree(GetProcessHeap(), 0, outBuf);
    } else {
        SetWindowTextA(hOutput, "Error decoding.");
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
            *p++ = (char)strtol(hex, NULL, 16);
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
            char hex[3] = { buf[i], buf[i+1], 0 };
            outBuf[outLen++] = (char)strtol(hex, NULL, 16);
            i++;
        }
    }
    outBuf[outLen] = 0;
    SetWindowTextA(hOutput, outBuf);
    HeapFree(GetProcessHeap(), 0, buf);
    HeapFree(GetProcessHeap(), 0, outBuf);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
            
            hInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN,
                10, 10, 360, 100, hwnd, NULL, NULL, NULL);
            SendMessageA(hInput, WM_SETFONT, (WPARAM)hFont, 0);
            
            hBtnEnc = CreateWindowA("BUTTON", "B64 Enc", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 120, 80, 25, hwnd, (HMENU)1, NULL, NULL);
            SendMessageA(hBtnEnc, WM_SETFONT, (WPARAM)hFont, 0);
            
            hBtnDec = CreateWindowA("BUTTON", "B64 Dec", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 100, 120, 80, 25, hwnd, (HMENU)2, NULL, NULL);
            SendMessageA(hBtnDec, WM_SETFONT, (WPARAM)hFont, 0);
            
            HWND hBtnUrlEnc = CreateWindowA("BUTTON", "URL Enc", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 190, 120, 80, 25, hwnd, (HMENU)4, NULL, NULL);
            SendMessageA(hBtnUrlEnc, WM_SETFONT, (WPARAM)hFont, 0);

            HWND hBtnUrlDec = CreateWindowA("BUTTON", "URL Dec", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 280, 120, 80, 25, hwnd, (HMENU)5, NULL, NULL);
            SendMessageA(hBtnUrlDec, WM_SETFONT, (WPARAM)hFont, 0);

            HWND hBtnHexEnc = CreateWindowA("BUTTON", "Hex Enc", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 155, 80, 25, hwnd, (HMENU)6, NULL, NULL);
            SendMessageA(hBtnHexEnc, WM_SETFONT, (WPARAM)hFont, 0);

            HWND hBtnHexDec = CreateWindowA("BUTTON", "Hex Dec", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 100, 155, 80, 25, hwnd, (HMENU)7, NULL, NULL);
            SendMessageA(hBtnHexDec, WM_SETFONT, (WPARAM)hFont, 0);
            
            hBtnHash = CreateWindowA("BUTTON", "SHA-256", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 190, 155, 80, 25, hwnd, (HMENU)3, NULL, NULL);
            SendMessageA(hBtnHash, WM_SETFONT, (WPARAM)hFont, 0);
            
            hOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                10, 190, 360, 100, hwnd, NULL, NULL, NULL);
            SendMessageA(hOutput, WM_SETFONT, (WPARAM)hFont, 0);
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
    
    RECT rc = {0, 0, 380, 300};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, FALSE);
    
    HWND hwnd = CreateWindowExA(0, "KBaseApp", "KBase", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
        
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
