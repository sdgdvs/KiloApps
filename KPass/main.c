#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

HWND hDisplay, hBtnGen, hUpper, hLower, hNum, hSym, hLen;
HFONT hFont, hBtnFont;
HBRUSH hBgBrush;

int my_atoi(const char* str) {
    int res = 0;
    while(*str) { res = res * 10 + (*str - '0'); str++; }
    return res;
}
int my_strlen(const char* s) { int l=0; while(*s++) l++; return l; }
void my_strcpy(char* d, const char* s) { while(*s) *d++ = *s++; *d = 0; }

void GeneratePassword() {
    char pool[200] = {0};
    int pLen = 0;
    if (SendMessage(hUpper, BM_GETCHECK, 0, 0)) { my_strcpy(pool + pLen, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"); pLen += 26; }
    if (SendMessage(hLower, BM_GETCHECK, 0, 0)) { my_strcpy(pool + pLen, "abcdefghijklmnopqrstuvwxyz"); pLen += 26; }
    if (SendMessage(hNum, BM_GETCHECK, 0, 0)) { my_strcpy(pool + pLen, "0123456789"); pLen += 10; }
    if (SendMessage(hSym, BM_GETCHECK, 0, 0)) { my_strcpy(pool + pLen, "!@#$%^&*()_+"); pLen += 12; }
    if (pLen == 0) { my_strcpy(pool, "abcdefghijklmnopqrstuvwxyz"); pLen = 26; SendMessage(hLower, BM_SETCHECK, BST_CHECKED, 0); }
    
    char lenStr[10];
    GetWindowTextA(hLen, lenStr, 10);
    int len = my_atoi(lenStr);
    if (len < 8) len = 8;
    if (len > 64) len = 64;
    
    char pwd[65] = {0};
    for(int i = 0; i < len; i++) {
        pwd[i] = pool[GetTickCount() % (pLen + i) % pLen];
        Sleep(1);
    }
    SetWindowTextA(hDisplay, pwd);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Click Generate...", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, 20, 20, 240, 30, hwnd, NULL, NULL, NULL);
            hUpper = CreateWindowA("BUTTON", "Uppercase", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 20, 60, 100, 20, hwnd, NULL, NULL, NULL);
            hLower = CreateWindowA("BUTTON", "Lowercase", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 130, 60, 100, 20, hwnd, NULL, NULL, NULL);
            hNum = CreateWindowA("BUTTON", "Numbers", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 20, 80, 100, 20, hwnd, NULL, NULL, NULL);
            hSym = CreateWindowA("BUTTON", "Symbols", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 130, 80, 100, 20, hwnd, NULL, NULL, NULL);
            CreateWindowA("STATIC", "Length:", WS_CHILD | WS_VISIBLE, 20, 110, 60, 20, hwnd, NULL, NULL, NULL);
            hLen = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "16", WS_CHILD | WS_VISIBLE | ES_NUMBER, 80, 110, 40, 20, hwnd, NULL, NULL, NULL);
            
            SendMessage(hUpper, BM_SETCHECK, BST_CHECKED, 0);
            SendMessage(hLower, BM_SETCHECK, BST_CHECKED, 0);
            SendMessage(hNum, BM_SETCHECK, BST_CHECKED, 0);
            SendMessage(hSym, BM_SETCHECK, BST_CHECKED, 0);
            
            hBtnGen = CreateWindowA("BUTTON", "Generate", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 90, 140, 100, 30, hwnd, (HMENU)1001, NULL, NULL);
            
            hBgBrush = CreateSolidBrush(RGB(20, 20, 20));
            hFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Consolas");
            SendMessageA(hDisplay, WM_SETFONT, (WPARAM)hFont, TRUE);
            hBtnFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Tahoma");
            SendMessageA(hBtnGen, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1001) {
                GeneratePassword();
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(20, 20, 20));
            SetTextColor(hdc, RGB(231, 76, 60)); // Red-ish for KPass
            return (LRESULT)hBgBrush;
        }
        case WM_DESTROY:
            if (hBgBrush) DeleteObject(hBgBrush);
            if (hFont) DeleteObject(hFont);
            if (hBtnFont) DeleteObject(hBtnFont);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KPassClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KPassClass", "KPass", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 220, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
