#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>

#define W 960
#define H 600

HWND hInput, hOutput, hMemory, hRegexFind, hRegexRep;
HWND hBtnRun, hBtnLoad, hBtnSave, hBtnStep, hBtnRec, hBtnPlay, hBtnRep, hBtnHelp;

int vars[26] = {0};
int nodeCount = 0;
int dpi = 96;
#define S(x) MulDiv(x, dpi, 96)
const char* debugPtr = NULL;
char debugInput[4096];
char outStr[4096];
char memStr[4096];

int isRecording = 0;
char macroBuf[4096];
int macroLen = 0;
WNDPROC oldEditProc;

int ParseExpr(const char** p);

void SkipWhitespace(const char** p) {
    while (**p == ' ' || **p == '\t' || **p == '\r' || **p == '\n' || **p == ';') {
        (*p)++;
    }
}

int ParseFactor(const char** p) {
    nodeCount++;
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
    nodeCount++;
    int val = ParseFactor(p);
    while (**p == '*' || **p == '/' || **p == '%') {
        char op = **p;
        (*p)++;
        int nextVal = ParseFactor(p);
        if (op == '*') val *= nextVal;
        else if (op == '/') { if (nextVal != 0) { if (val == (int)0x80000000 && nextVal == -1) val = (int)0x80000000; else val /= nextVal; } }
        else if (op == '%') { if (nextVal != 0) { if (val == (int)0x80000000 && nextVal == -1) val = 0; else val %= nextVal; } }
    }
    return val;
}

int ParseExpr(const char** p) {
    nodeCount++;
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
    wsprintfA(buf, "%d", val);
}
int StrLen(const char* s) { int c = 0; while (*s++) c++; return c; }

void UpdateMemoryUI() {
    memStr[0] = '\0';
    wsprintfA(memStr, "AST Nodes: %d\r\n\r\n", nodeCount);
    for (int i=0; i<26; i++) {
        if (vars[i] != 0) {
            char t[32];
            wsprintfA(t, "%c = %d\r\n", 'A'+i, vars[i]);
            lstrcatA(memStr, t);
        }
    }
    SetWindowTextA(hMemory, memStr);
    SetWindowTextA(hOutput, outStr);
}

int StepScript(int* lastVal) {
    if (!debugPtr) {
        GetWindowTextA(hInput, debugInput, sizeof(debugInput));
        debugPtr = debugInput;
        for (int i = 0; i < 26; i++) vars[i] = 0;
        outStr[0] = '\0';
        nodeCount = 0;
    }
    
    SkipWhitespace(&debugPtr);
    if (!*debugPtr) {
        char resStr[32];
        IntToStr(*lastVal, resStr);
        int l = StrLen(outStr);
        if (l < sizeof(outStr) - 64) wsprintfA(outStr + l, "\r\nReturn: %s", resStr);
        UpdateMemoryUI();
        debugPtr = NULL; // Finished
        return 0; // done
    }
    
    const char* q = debugPtr;
    if (q[0] == 'p' && q[1] == 'r' && q[2] == 'i' && q[3] == 'n' && q[4] == 't') {
        char after = q[5];
        if (after == ' ' || after == '\t' || after == '(' || after == '\0' || after == '\r' || after == '\n' || after == ';') {
            debugPtr += 5;
            int val = ParseExpr(&debugPtr);
            char vstr[32];
            IntToStr(val, vstr);
            int l = StrLen(outStr);
            if (l < sizeof(outStr) - 64) wsprintfA(outStr + l, "Print: %s\r\n", vstr);
            *lastVal = val;
            UpdateMemoryUI();
            return 1;
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
            debugPtr = q + 1;
        }
    }
    
    int val = ParseExpr(&debugPtr);
    if (isAssign) {
        vars[varIdx] = val;
        char vname = varIdx + 'A';
        char vstr[32];
        IntToStr(val, vstr);
        int l = StrLen(outStr);
        if (l < sizeof(outStr) - 64) wsprintfA(outStr + l, "%c = %s\r\n", vname, vstr);
    }
    *lastVal = val;
    UpdateMemoryUI();
    return 1;
}

void RunScript() {
    debugPtr = NULL;
    int lastVal = 0;
    while (StepScript(&lastVal)) {}
}

LRESULT CALLBACK InputEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_CHAR && isRecording) {
        if (macroLen < sizeof(macroBuf) - 1) {
            macroBuf[macroLen++] = (char)wp;
        }
    }
    if (msg == WM_SYSKEYDOWN && wp == 'H') {
        SendMessage(GetParent(hwnd), WM_COMMAND, 8, 0);
        return 0;
    }
    if (msg == WM_KEYDOWN && wp == VK_F1) {
        SendMessage(GetParent(hwnd), WM_COMMAND, 8, 0);
        return 0;
    }
    return CallWindowProc(oldEditProc, hwnd, msg, wp, lp);
}

void SimpleRegexReplace() {
    char f[256], r[256], inBuf[4096], outBuf[4096];
    GetWindowTextA(hRegexFind, f, sizeof(f));
    GetWindowTextA(hRegexRep, r, sizeof(r));
    GetWindowTextA(hInput, inBuf, sizeof(inBuf));
    if (f[0] == '\0') return;
    
    outBuf[0] = '\0';
    char* src = inBuf;
    char* dst = outBuf;
    int fLen = StrLen(f);
    int rLen = StrLen(r);
    
    while (*src) {
        int match = 1;
        for (int i=0; i<fLen; i++) {
            if (src[i] == '\0' || src[i] != f[i]) { match = 0; break; }
        }
        if (match) {
            for (int i=0; i<rLen; i++) {
                if (dst - outBuf < sizeof(outBuf) - 1) {
                    *dst++ = r[i];
                }
            }
            src += fLen;
        } else {
            if (dst - outBuf < sizeof(outBuf) - 1) {
                *dst++ = *src;
            }
            src++;
        }
    }
    *dst = '\0';
    SetWindowTextA(hInput, outBuf);
}

HBRUSH hbrBg;
HFONT hFont;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_GETMINMAXINFO: {
            MINMAXINFO* mmi = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = 960;
            mmi->ptMinTrackSize.y = 400;
            return 0;
        }
        case WM_CREATE: {
            hbrBg = CreateSolidBrush(RGB(30, 30, 30));
            int fontHeight = -MulDiv(12, dpi, 72);
            hFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 5 /* CLEARTYPE_QUALITY */, DEFAULT_PITCH, "Consolas");
            
            // Toolbar
            hBtnRec  = CreateWindowEx(0, "BUTTON", "Rec Macro", WS_CHILD | WS_VISIBLE, S(10), S(10), S(100), S(24), hwnd, (HMENU)4, NULL, NULL);
            hBtnPlay = CreateWindowEx(0, "BUTTON", "Play Macro", WS_CHILD | WS_VISIBLE, S(120), S(10), S(100), S(24), hwnd, (HMENU)5, NULL, NULL);
            hBtnStep = CreateWindowEx(0, "BUTTON", "Step", WS_CHILD | WS_VISIBLE, S(230), S(10), S(80), S(24), hwnd, (HMENU)6, NULL, NULL);
            hBtnRun  = CreateWindowEx(0, "BUTTON", "Run", WS_CHILD | WS_VISIBLE, S(320), S(10), S(80), S(24), hwnd, (HMENU)1, NULL, NULL);
            hBtnLoad = CreateWindowEx(0, "BUTTON", "Load", WS_CHILD | WS_VISIBLE, S(410), S(10), S(80), S(24), hwnd, (HMENU)2, NULL, NULL);
            hBtnSave = CreateWindowEx(0, "BUTTON", "Save", WS_CHILD | WS_VISIBLE, S(500), S(10), S(80), S(24), hwnd, (HMENU)3, NULL, NULL);
            hBtnHelp = CreateWindowEx(0, "BUTTON", "Help", WS_CHILD | WS_VISIBLE, S(590), S(10), S(80), S(24), hwnd, (HMENU)8, NULL, NULL);
            
            hRegexFind = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Find...", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, S(680), S(10), S(80), S(24), hwnd, NULL, NULL, NULL);
            hRegexRep  = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Rep...", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, S(770), S(10), S(80), S(24), hwnd, NULL, NULL, NULL);
            hBtnRep    = CreateWindowEx(0, "BUTTON", "Replace", WS_CHILD | WS_VISIBLE, S(860), S(10), S(70), S(24), hwnd, (HMENU)7, NULL, NULL);
            
            HWND hwnds[] = {hBtnRec, hBtnPlay, hBtnStep, hBtnRun, hBtnLoad, hBtnSave, hRegexFind, hRegexRep, hBtnRep, hBtnHelp};
            for(int i=0; i<10; i++) SendMessage(hwnds[i], WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Panels
            hInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "// Welcome to KScript!\r\n// Press F1, Alt+H, or click Help for instructions\r\na = 10\r\nb = 20\r\nprint a * b + 5\r\nprint a % 3",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
                S(10), S(44), S(250), S(H) - S(95), hwnd, NULL, NULL, NULL);
            SendMessage(hInput, WM_SETFONT, (WPARAM)hFont, TRUE);
            oldEditProc = (WNDPROC)SetWindowLongPtr(hInput, GWLP_WNDPROC, (LONG_PTR)InputEditProc);
            
            hOutput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                S(270), S(44), S(250), S(H) - S(95), hwnd, NULL, NULL, NULL);
            SendMessage(hOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hMemory = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Memory Inspector",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                S(530), S(44), S(240), S(H) - S(95), hwnd, NULL, NULL, NULL);
            SendMessage(hMemory, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            break;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            if (wmId == 1) RunScript();
            else if (wmId == 6) { int lv = 0; StepScript(&lv); }
            else if (wmId == 4) {
                isRecording = !isRecording;
                SetWindowTextA(hBtnRec, isRecording ? "Stop Rec" : "Rec Macro");
                if (isRecording) macroLen = 0;
            }
            else if (wmId == 5) {
                if (isRecording) {
                    isRecording = 0;
                    SetWindowTextA(hBtnRec, "Rec Macro");
                }
                SetFocus(hInput);
                for(int i=0; i<macroLen; i++) {
                    SendMessage(hInput, WM_CHAR, macroBuf[i], 0);
                    Sleep(20);
                }
            }
            else if (wmId == 7) SimpleRegexReplace();
            else if (wmId == 8) {
                MessageBoxA(hwnd, "Welcome to KScript!\n\nWrite equations, assign variables (a-z), and use 'print' to output values.\n\nExample:\na = 10\nprint a * 2\n\nControls:\n- Rec Macro: Record keystrokes\n- Play Macro: Replay them\n- Step: Run line-by-line\n- Run: Run everything\n- Find/Replace: Regex support", "KScript Help", MB_OK | MB_ICONINFORMATION);
            }
            else if (wmId == 2) {
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
                        }
                        CloseHandle(hFile);
                    }
                }
            }
            else if (wmId == 3) {
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
                        }
                        CloseHandle(hFile);
                    }
                }
            }
            break;
        }
        case WM_SYSKEYDOWN: {
            if (wParam == 'H') {
                SendMessage(hwnd, WM_COMMAND, 8, 0);
                return 0;
            }
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        case WM_KEYDOWN: {
            if (wParam == VK_F1) {
                SendMessage(hwnd, WM_COMMAND, 8, 0);
                return 0;
            }
            break;
        }
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            HWND hCtl = (HWND)lParam;
            SetBkMode(hdc, TRANSPARENT);
            if (hCtl == hMemory) SetTextColor(hdc, RGB(100, 200, 255));
            else SetTextColor(hdc, RGB(220, 220, 220));
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
            int panelW = (nw - S(40)) / 3;
            MoveWindow(hInput, S(10), S(44), panelW, nh - S(55), TRUE);
            MoveWindow(hOutput, S(20) + panelW, S(44), panelW, nh - S(55), TRUE);
            MoveWindow(hMemory, S(30) + panelW * 2, S(44), panelW, nh - S(55), TRUE);
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

#pragma function(memcpy)
void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}

void MainEntry() {
    SetProcessDPIAware();
    HDC hdc = GetDC(NULL);
    dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(NULL, hdc);
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KScriptApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = NULL;
    RegisterClass(&wc);

    DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    RECT rect = {0, 0, S(W), S(H)};
    AdjustWindowRect(&rect, style, FALSE);
    HWND hwnd = CreateWindowEx(0, "KScriptApp", "KScript - Press F1 for Help", style,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
