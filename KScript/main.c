#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 400
#define H 300

HWND hInput;
HWND hOutput;
HWND hBtnRun;

// A tiny recursive descent parser for basic math expressions without CRT
int ParseExpr(const char** p);

int ParseFactor(const char** p) {
    while (**p == ' ') (*p)++;
    int val = 0;
    if (**p == '(') {
        (*p)++;
        val = ParseExpr(p);
        while (**p == ' ') (*p)++;
        if (**p == ')') (*p)++;
    } else {
        while (**p >= '0' && **p <= '9') {
            val = val * 10 + (**p - '0');
            (*p)++;
        }
    }
    while (**p == ' ') (*p)++;
    return val;
}

int ParseTerm(const char** p) {
    int val = ParseFactor(p);
    while (**p == '*' || **p == '/') {
        char op = **p;
        (*p)++;
        int nextVal = ParseFactor(p);
        if (op == '*') val *= nextVal;
        else if (nextVal != 0) val /= nextVal;
    }
    return val;
}

int ParseExpr(const char** p) {
    int val = ParseTerm(p);
    while (**p == '+' || **p == '-') {
        char op = **p;
        (*p)++;
        int nextVal = ParseTerm(p);
        if (op == '+') val += nextVal;
        else val -= nextVal;
    }
    return val;
}

void IntToStr(int val, char* buf) {
    if (val == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    char temp[32];
    int i = 0;
    int isNeg = val < 0;
    if (isNeg) val = -val;
    while (val > 0) {
        temp[i++] = (val % 10) + '0';
        val /= 10;
    }
    if (isNeg) temp[i++] = '-';
    int j = 0;
    while (i > 0) {
        buf[j++] = temp[--i];
    }
    buf[j] = '\0';
}

void RunScript() {
    char input[256];
    GetWindowTextA(hInput, input, sizeof(input));
    
    const char* p = input;
    int result = ParseExpr(&p);
    
    char outStr[256];
    char resStr[32];
    IntToStr(result, resStr);
    
    wsprintfA(outStr, "Evaluating:\r\n%s\r\n\r\nResult: %s", input, resStr);
    SetWindowTextA(hOutput, outStr);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
            
            hInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "10 + 2 * (5 - 3)",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                10, 10, W - 100, 24, hwnd, NULL, NULL, NULL);
            SendMessage(hInput, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnRun = CreateWindowEx(0, "BUTTON", "Run",
                WS_CHILD | WS_VISIBLE,
                W - 80, 10, 60, 24, hwnd, (HMENU)1, NULL, NULL);
            SendMessage(hBtnRun, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hOutput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                10, 44, W - 35, H - 95, hwnd, NULL, NULL, NULL);
            SendMessage(hOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                RunScript();
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            MoveWindow(hInput, 10, 10, nw - 100, 24, TRUE);
            MoveWindow(hBtnRun, nw - 80, 10, 60, 24, TRUE);
            MoveWindow(hOutput, 10, 44, nw - 20, nh - 55, TRUE);
            break;
        }
        case WM_DESTROY:
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
    wc.lpszClassName = "KScriptApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KScriptApp", "KScript REPL", WS_OVERLAPPEDWINDOW,
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
