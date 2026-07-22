#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#define W 500
#define H 400

HWND hInput;
HWND hOutput;
HWND hBtnRun;
HWND hBtnLoad;
HWND hBtnSave;

int vars[26] = {0};

int ParseExpr(const char** p);

void SkipWhitespace(const char** p) {
    while (**p == ' ' || **p == '\t' || **p == '\r' || **p == '\n' || **p == ';') {
        (*p)++;
    }
}

int ParseFactor(const char** p) {
    SkipWhitespace(p);
    int val = 0;
    if (**p == '(') {
        (*p)++;
        val = ParseExpr(p);
        SkipWhitespace(p);
        if (**p == ')') (*p)++;
    } else if ((**p >= 'a' && **p <= 'z') || (**p >= 'A' && **p <= 'Z')) {
        char v = **p;
        int idx = (v >= 'a') ? v - 'a' : v - 'A';
        (*p)++;
        val = vars[idx];
    } else {
        if (**p >= '0' && **p <= '9') {
            while (**p >= '0' && **p <= '9') {
                val = val * 10 + (**p - '0');
                (*p)++;
            }
        } else if (**p != '\0') {
            (*p)++;
        }
    }
    SkipWhitespace(p);
    return val;
}

int ParseTerm(const char** p) {
    int val = ParseFactor(p);
    while (**p == '*' || **p == '/' || **p == '%') {
        char op = **p;
        (*p)++;
        int nextVal = ParseFactor(p);
        if (op == '*') val *= nextVal;
        else if (op == '/') { if (nextVal != 0) val /= nextVal; }
        else if (op == '%') { if (nextVal != 0) val %= nextVal; }
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

int StrLen(const char* s) {
    int c = 0;
    while (*s++) c++;
    return c;
}

void RunScript() {
    char input[4096];
    GetWindowTextA(hInput, input, sizeof(input));
    for (int i = 0; i < 26; i++) vars[i] = 0;
    
    char outStr[4096];
    outStr[0] = '\0';
    
    const char* p = input;
    int lastVal = 0;
    while (*p) {
        SkipWhitespace(&p);
        if (!*p) break;
        
        const char* q = p;
        if (q[0] == 'p' && q[1] == 'r' && q[2] == 'i' && q[3] == 'n' && q[4] == 't') {
            char after = q[5];
            if (after == ' ' || after == '\t' || after == '(' || after == '\0' || after == '\r' || after == '\n' || after == ';') {
                p += 5;
                int val = ParseExpr(&p);
                char vstr[32];
                IntToStr(val, vstr);
                int l = StrLen(outStr);
                if (l < sizeof(outStr) - 64) {
                    wsprintfA(outStr + l, "Print: %s\r\n", vstr);
                }
                lastVal = val;
                continue;
            }
        }
        
        int isAssign = 0;
        int varIdx = -1;
        if ((*q >= 'a' && *q <= 'z') || (*q >= 'A' && *q <= 'Z')) {
            char v = *q;
            q++;
            while (*q == ' ' || *q == '\t') q++;
            if (*q == '=') {
                isAssign = 1;
                varIdx = (v >= 'a') ? v - 'a' : v - 'A';
                p = q + 1;
            }
        }
        
        int val = ParseExpr(&p);
        if (isAssign) {
            vars[varIdx] = val;
            char vname = varIdx + 'A';
            char vstr[32];
            IntToStr(val, vstr);
            int l = StrLen(outStr);
            if (l < sizeof(outStr) - 64) {
                wsprintfA(outStr + l, "%c = %s\r\n", vname, vstr);
            }
        }
        lastVal = val;
    }
    
    char resStr[32];
    IntToStr(lastVal, resStr);
    int l = StrLen(outStr);
    if (l < sizeof(outStr) - 64) {
        wsprintfA(outStr + l, "\r\nReturn: %s", resStr);
    }
    
    SetWindowTextA(hOutput, outStr);
}

HBRUSH hbrBg;
HFONT hFont;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hbrBg = CreateSolidBrush(RGB(30, 30, 30));
            hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
            
            hInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "a = 10\r\nb = 20\r\nprint a * b + 5\r\nprint a % 3",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
                10, 44, (W - 30) / 2, H - 95, hwnd, NULL, NULL, NULL);
            SendMessage(hInput, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnRun = CreateWindowEx(0, "BUTTON", "Run Script",
                WS_CHILD | WS_VISIBLE,
                10, 10, 100, 24, hwnd, (HMENU)1, NULL, NULL);
            SendMessage(hBtnRun, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnLoad = CreateWindowEx(0, "BUTTON", "Load",
                WS_CHILD | WS_VISIBLE,
                120, 10, 80, 24, hwnd, (HMENU)2, NULL, NULL);
            SendMessage(hBtnLoad, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnSave = CreateWindowEx(0, "BUTTON", "Save",
                WS_CHILD | WS_VISIBLE,
                210, 10, 80, 24, hwnd, (HMENU)3, NULL, NULL);
            SendMessage(hBtnSave, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hOutput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                10 + (W - 30) / 2 + 10, 44, (W - 30) / 2, H - 95, hwnd, NULL, NULL, NULL);
            SendMessage(hOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                RunScript();
            } else if (LOWORD(wParam) == 2) {
                char szFile[260] = {0};
                OPENFILENAMEA ofn = {0};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = "KScript Files\0*.ksc\0All Files\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                if (GetOpenFileNameA(&ofn)) {
                    HANDLE hFile = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hFile != INVALID_HANDLE_VALUE) {
                        DWORD dwSize = GetFileSize(hFile, NULL);
                        if (dwSize > 0) {
                            char* buf = (char*)VirtualAlloc(NULL, dwSize + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                            DWORD dwRead;
                            if (ReadFile(hFile, buf, dwSize, &dwRead, NULL)) {
                                buf[dwRead] = '\0';
                                SetWindowTextA(hInput, buf);
                            }
                            VirtualFree(buf, 0, MEM_RELEASE);
                        } else {
                            SetWindowTextA(hInput, "");
                        }
                        CloseHandle(hFile);
                    }
                }
            } else if (LOWORD(wParam) == 3) {
                char szFile[260] = {0};
                OPENFILENAMEA ofn = {0};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = "KScript Files\0*.ksc\0All Files\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrDefExt = "ksc";
                ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
                if (GetSaveFileNameA(&ofn)) {
                    HANDLE hFile = CreateFileA(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hFile != INVALID_HANDLE_VALUE) {
                        int len = GetWindowTextLengthA(hInput);
                        if (len > 0) {
                            char* buf = (char*)VirtualAlloc(NULL, len + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                            GetWindowTextA(hInput, buf, len + 1);
                            DWORD dwWritten;
                            WriteFile(hFile, buf, len, &dwWritten, NULL);
                            VirtualFree(buf, 0, MEM_RELEASE);
                        } else {
                            DWORD dwWritten;
                            WriteFile(hFile, "", 0, &dwWritten, NULL);
                        }
                        CloseHandle(hFile);
                    }
                }
            }
            break;
        }
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(220, 220, 220));
            return (LRESULT)hbrBg;
        }
        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, hbrBg);
            return 1;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            int halfW = (nw - 30) / 2;
            MoveWindow(hBtnRun, 10, 10, 100, 24, TRUE);
            MoveWindow(hInput, 10, 44, halfW, nh - 55, TRUE);
            MoveWindow(hOutput, 20 + halfW, 44, halfW, nh - 55, TRUE);
            break;
        }
        case WM_DESTROY:
            DeleteObject(hFont);
            DeleteObject(hbrBg);
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
    wc.hbrBackground = NULL;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KScriptApp", "KScript", WS_OVERLAPPEDWINDOW,
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
