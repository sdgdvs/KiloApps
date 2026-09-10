#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <stdio.h>

#define W 960
#define H 600

HWND hMainWnd = NULL;
HWND hInput, hOutput, hMemory, hRegexFind, hRegexRep;
HWND hBtnRun, hBtnLoad, hBtnSave, hBtnStep, hBtnRec, hBtnPlay, hBtnRep, hBtnHelp, hBtnClear;

int vars[26] = {0};
int varAssigned[26] = {0};
int nodeCount = 0;
int dpi = 96;
#define S(x) MulDiv(x, dpi, 96)
const char* debugPtr = NULL;
char debugInput[16384];
char outStr[16384];
char memStr[16384];

int isRecording = 0;
char macroBuf[8192];
int macroLen = 0;
WNDPROC oldEditProc;
WNDPROC oldFindProc;
WNDPROC oldRepProc;

int ParseExpr(const char** p);
void UpdateWindowTitle();

void SkipWhitespace(const char** p) {
    while (1) {
        if (**p == ' ' || **p == '\t' || **p == '\r' || **p == '\n' || **p == ';') {
            (*p)++;
        } else if (**p == '/' && *(*p + 1) == '/') {
            while (**p != '\0' && **p != '\r' && **p != '\n') (*p)++;
        } else {
            break;
        }
    }
}

int ParseFactor(const char** p) {
    nodeCount++;
    SkipWhitespace(p);
    int val = 0;
    if (**p == '+') {
        (*p)++;
        return ParseFactor(p);
    } else if (**p == '-') {
        (*p)++;
        return -ParseFactor(p);
    } else if (**p == '(') {
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
        } else if (**p != '\0' && **p != ';' && **p != '\n' && **p != '\r') {
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
    int active = 0;
    for (int i = 0; i < 26; i++) {
        if (varAssigned[i]) {
            char t[32];
            wsprintfA(t, "%c = %d\r\n", 'A' + i, vars[i]);
            lstrcatA(memStr, t);
            active++;
        }
    }
    if (active == 0) {
        lstrcatA(memStr, "(no non-zero variables)");
    }
    SetWindowTextA(hMemory, memStr);
    SetWindowTextA(hOutput, outStr);
}

int StepScript(int* lastVal) {
    if (!debugPtr) {
        GetWindowTextA(hInput, debugInput, sizeof(debugInput));
        debugPtr = debugInput;
        for (int i = 0; i < 26; i++) { vars[i] = 0; varAssigned[i] = 0; }
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
        varAssigned[varIdx] = 1;
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

void SimpleRegexReplace() {
    char f[256], r[256];
    char inBuf[16384], outBuf[16384];
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
        for (int i = 0; i < fLen; i++) {
            if (src[i] == '\0' || src[i] != f[i]) { match = 0; break; }
        }
        if (match) {
            for (int i = 0; i < rLen; i++) {
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

void ShowHelpDialog(HWND hwnd) {
    const char* helpText = 
        "=========================================\n"
        "           KSCRIPT USER GUIDE            \n"
        "=========================================\n\n"
        "[SYNTAX & EXPRESSIONS]\n"
        " - Variables: Single letters 'a' through 'z' (e.g. a = 10)\n"
        " - Arithmetic: +, -, *, /, % and nested parentheses ()\n"
        " - Unary Operators: Negative (-5) and Positive (+10)\n"
        " - Console Output: print <expression>\n"
        " - Comments: Lines starting with // are ignored\n\n"
        "[TOOLBAR CONTROLS & SHORTCUTS]\n"
        " - [F5] or [Ctrl+Enter] : Run All Script\n"
        " - [F10] or [Alt+S]     : Step Line-by-Line\n"
        " - [Ctrl+S]             : Save Script File (.ksc)\n"
        " - [Ctrl+O]             : Load Script File (.ksc)\n"
        " - [Ctrl+K]             : Clear Console & State\n"
        " - [Alt+M] / [Ctrl+M]   : Toggle Macro Recording\n"
        " - [Alt+P] / [Ctrl+P]   : Play Recorded Macro\n"
        " - [Enter in Find/Rep]  : Execute Replace\n"
        " - [F1] or [Alt+H]      : Open this Help Guide\n"
        " - [Drag & Drop]        : Drop .ksc file into window\n\n"
        "[MACRO ENGINE]\n"
        " Click 'Rec Macro' (or Alt+M) to record editor keystrokes.\n"
        " Click 'Play Macro' (or Alt+P) to replay automated input.";

    MessageBoxA(hwnd, helpText, "KScript - Help & Keyboard Shortcuts", MB_OK | MB_ICONINFORMATION);
}

void UpdateWindowTitle() {
    if (isRecording) {
        SetWindowTextA(hMainWnd, "KScript [● RECORDING MACRO] - Press Alt+M to Stop");
    } else {
        SetWindowTextA(hMainWnd, "KScript - [F5] Run | [F10] Step | [F1] Help");
    }
}

LRESULT CALLBACK InputEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_CHAR && isRecording) {
        if (macroLen < sizeof(macroBuf) - 1) {
            macroBuf[macroLen++] = (char)wp;
        }
    }
    if (msg == WM_KEYDOWN) {
        if (wp == VK_F5 || (GetKeyState(VK_CONTROL) < 0 && wp == VK_RETURN)) {
            SendMessage(GetParent(hwnd), WM_COMMAND, 1, 0);
            return 0;
        }
        if (wp == VK_F10 || (GetKeyState(VK_MENU) < 0 && (wp == 'S' || wp == 's'))) {
            SendMessage(GetParent(hwnd), WM_COMMAND, 6, 0);
            return 0;
        }
        if (GetKeyState(VK_CONTROL) < 0 && (wp == 'S' || wp == 's')) {
            SendMessage(GetParent(hwnd), WM_COMMAND, 3, 0);
            return 0;
        }
        if (GetKeyState(VK_CONTROL) < 0 && (wp == 'O' || wp == 'o')) {
            SendMessage(GetParent(hwnd), WM_COMMAND, 2, 0);
            return 0;
        }
        if (GetKeyState(VK_CONTROL) < 0 && (wp == 'K' || wp == 'k')) {
            SendMessage(GetParent(hwnd), WM_COMMAND, 9, 0);
            return 0;
        }
        if ((GetKeyState(VK_MENU) < 0 || GetKeyState(VK_CONTROL) < 0) && (wp == 'M' || wp == 'm')) {
            SendMessage(GetParent(hwnd), WM_COMMAND, 4, 0);
            return 0;
        }
        if ((GetKeyState(VK_MENU) < 0 || GetKeyState(VK_CONTROL) < 0) && (wp == 'P' || wp == 'p')) {
            SendMessage(GetParent(hwnd), WM_COMMAND, 5, 0);
            return 0;
        }
        if (wp == VK_F1) {
            SendMessage(GetParent(hwnd), WM_COMMAND, 8, 0);
            return 0;
        }
    }
    if (msg == WM_SYSKEYDOWN && (wp == 'H' || wp == 'h')) {
        SendMessage(GetParent(hwnd), WM_COMMAND, 8, 0);
        return 0;
    }
    return CallWindowProc(oldEditProc, hwnd, msg, wp, lp);
}

LRESULT CALLBACK FindEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_KEYDOWN && wp == VK_RETURN) {
        SendMessage(GetParent(hwnd), WM_COMMAND, 7, 0);
        return 0;
    }
    if (msg == WM_KEYDOWN && wp == VK_F1) {
        SendMessage(GetParent(hwnd), WM_COMMAND, 8, 0);
        return 0;
    }
    return CallWindowProc(oldFindProc, hwnd, msg, wp, lp);
}

LRESULT CALLBACK RepEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_KEYDOWN && wp == VK_RETURN) {
        SendMessage(GetParent(hwnd), WM_COMMAND, 7, 0);
        return 0;
    }
    if (msg == WM_KEYDOWN && wp == VK_F1) {
        SendMessage(GetParent(hwnd), WM_COMMAND, 8, 0);
        return 0;
    }
    return CallWindowProc(oldRepProc, hwnd, msg, wp, lp);
}

HBRUSH hbrBg;
HFONT hFont;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_GETMINMAXINFO: {
            MINMAXINFO* mmi = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = S(720);
            mmi->ptMinTrackSize.y = S(380);
            return 0;
        }
        case WM_CREATE: {
            hMainWnd = hwnd;
            hbrBg = CreateSolidBrush(RGB(20, 24, 38));
            int fontHeight = -MulDiv(11, dpi, 72);
            hFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 5 /* CLEARTYPE_QUALITY */, DEFAULT_PITCH, "Consolas");
            
            // Toolbar Buttons with clear keyboard shortcut hints
            hBtnRec   = CreateWindowEx(0, "BUTTON", "Rec [Alt+M]", WS_CHILD | WS_VISIBLE, S(8), S(8), S(90), S(26), hwnd, (HMENU)4, NULL, NULL);
            hBtnPlay  = CreateWindowEx(0, "BUTTON", "Play [Alt+P]", WS_CHILD | WS_VISIBLE, S(102), S(8), S(90), S(26), hwnd, (HMENU)5, NULL, NULL);
            hBtnStep  = CreateWindowEx(0, "BUTTON", "Step [F10]", WS_CHILD | WS_VISIBLE, S(196), S(8), S(80), S(26), hwnd, (HMENU)6, NULL, NULL);
            hBtnRun   = CreateWindowEx(0, "BUTTON", "Run [F5]", WS_CHILD | WS_VISIBLE, S(280), S(8), S(75), S(26), hwnd, (HMENU)1, NULL, NULL);
            hBtnLoad  = CreateWindowEx(0, "BUTTON", "Load [Ctrl+O]", WS_CHILD | WS_VISIBLE, S(359), S(8), S(90), S(26), hwnd, (HMENU)2, NULL, NULL);
            hBtnSave  = CreateWindowEx(0, "BUTTON", "Save [Ctrl+S]", WS_CHILD | WS_VISIBLE, S(453), S(8), S(90), S(26), hwnd, (HMENU)3, NULL, NULL);
            hBtnClear = CreateWindowEx(0, "BUTTON", "Clear [Ctrl+K]", WS_CHILD | WS_VISIBLE, S(547), S(8), S(90), S(26), hwnd, (HMENU)9, NULL, NULL);
            hBtnHelp  = CreateWindowEx(0, "BUTTON", "Help [F1]", WS_CHILD | WS_VISIBLE, S(641), S(8), S(75), S(26), hwnd, (HMENU)8, NULL, NULL);
            
            hRegexFind = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, S(722), S(9), S(75), S(24), hwnd, NULL, NULL, NULL);
            hRegexRep  = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, S(801), S(9), S(75), S(24), hwnd, NULL, NULL, NULL);
            hBtnRep    = CreateWindowEx(0, "BUTTON", "Rep [Enter]", WS_CHILD | WS_VISIBLE, S(880), S(8), S(72), S(26), hwnd, (HMENU)7, NULL, NULL);
            
            HWND hwnds[] = {hBtnRec, hBtnPlay, hBtnStep, hBtnRun, hBtnLoad, hBtnSave, hBtnClear, hRegexFind, hRegexRep, hBtnRep, hBtnHelp};
            for (int i = 0; i < 11; i++) SendMessage(hwnds[i], WM_SETFONT, (WPARAM)hFont, TRUE);
            
            oldFindProc = (WNDPROC)SetWindowLongPtr(hRegexFind, GWLP_WNDPROC, (LONG_PTR)FindEditProc);
            oldRepProc  = (WNDPROC)SetWindowLongPtr(hRegexRep, GWLP_WNDPROC, (LONG_PTR)RepEditProc);
            
            // Enable Drag-and-Drop file loading
            DragAcceptFiles(hwnd, TRUE);
            
            // Panels
            hInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "// Welcome to KScript!\r\n// Press F1, Alt+H, or click Help for instructions\r\na = 10\r\nb = 20\r\nprint a * b + 5\r\nprint a % 3",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
                S(8), S(42), S(290), S(H) - S(90), hwnd, NULL, NULL, NULL);
            SendMessage(hInput, WM_SETFONT, (WPARAM)hFont, TRUE);
            oldEditProc = (WNDPROC)SetWindowLongPtr(hInput, GWLP_WNDPROC, (LONG_PTR)InputEditProc);
            
            hOutput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                S(306), S(42), S(290), S(H) - S(90), hwnd, NULL, NULL, NULL);
            SendMessage(hOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hMemory = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Memory Inspector",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                S(604), S(42), S(240), S(H) - S(90), hwnd, NULL, NULL, NULL);
            SendMessage(hMemory, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Run script on startup
            RunScript();
            break;
        }
        case WM_DROPFILES: {
            HDROP hDrop = (HDROP)wParam;
            char szFile[MAX_PATH] = {0};
            if (DragQueryFileA(hDrop, 0, szFile, sizeof(szFile))) {
                HANDLE hFile = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    DWORD dwSize = GetFileSize(hFile, NULL);
                    if (dwSize > 0 && dwSize < 1024 * 1024) {
                        char* buf = (char*)VirtualAlloc(NULL, dwSize + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                        if (buf) {
                            DWORD dwRead;
                            if (ReadFile(hFile, buf, dwSize, &dwRead, NULL)) {
                                buf[dwRead] = '\0';
                                SetWindowTextA(hInput, buf);
                                RunScript();
                            }
                            VirtualFree(buf, 0, MEM_RELEASE);
                        }
                    }
                    CloseHandle(hFile);
                }
            }
            DragFinish(hDrop);
            return 0;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            if (wmId == 1) {
                RunScript();
            }
            else if (wmId == 6) {
                int lv = 0;
                StepScript(&lv);
            }
            else if (wmId == 4) {
                isRecording = !isRecording;
                SetWindowTextA(hBtnRec, isRecording ? "Stop [Alt+M]" : "Rec [Alt+M]");
                if (isRecording) macroLen = 0;
                UpdateWindowTitle();
            }
            else if (wmId == 5) {
                if (isRecording) {
                    isRecording = 0;
                    SetWindowTextA(hBtnRec, "Rec [Alt+M]");
                    UpdateWindowTitle();
                }
                SetFocus(hInput);
                for (int i = 0; i < macroLen; i++) {
                    SendMessage(hInput, WM_CHAR, (WPARAM)(unsigned char)macroBuf[i], 0);
                }
            }
            else if (wmId == 7) {
                SimpleRegexReplace();
            }
            else if (wmId == 8) {
                ShowHelpDialog(hwnd);
            }
            else if (wmId == 9) {
                debugPtr = NULL;
                for (int i = 0; i < 26; i++) { vars[i] = 0; varAssigned[i] = 0; }
                outStr[0] = '\0';
                nodeCount = 0;
                UpdateMemoryUI();
            }
            else if (wmId == 2) {
                char szFile[MAX_PATH] = {0};
                OPENFILENAMEA ofn = {0};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = "KScript Files (*.ksc)\0*.ksc\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                if (GetOpenFileNameA(&ofn)) {
                    HANDLE hFile = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hFile != INVALID_HANDLE_VALUE) {
                        DWORD dwSize = GetFileSize(hFile, NULL);
                        if (dwSize > 0 && dwSize < 1024 * 1024) {
                            char* buf = (char*)VirtualAlloc(NULL, dwSize + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                            if (buf) {
                                DWORD dwRead;
                                if (ReadFile(hFile, buf, dwSize, &dwRead, NULL)) {
                                    buf[dwRead] = '\0';
                                    SetWindowTextA(hInput, buf);
                                    RunScript();
                                }
                                VirtualFree(buf, 0, MEM_RELEASE);
                            }
                        }
                        CloseHandle(hFile);
                    }
                }
            }
            else if (wmId == 3) {
                char szFile[MAX_PATH] = "script.ksc";
                OPENFILENAMEA ofn = {0};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = "KScript Files (*.ksc)\0*.ksc\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrDefExt = "ksc";
                ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
                if (GetSaveFileNameA(&ofn)) {
                    HANDLE hFile = CreateFileA(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hFile != INVALID_HANDLE_VALUE) {
                        int len = GetWindowTextLengthA(hInput);
                        if (len > 0) {
                            char* buf = (char*)VirtualAlloc(NULL, len + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                            if (buf) {
                                GetWindowTextA(hInput, buf, len + 1);
                                DWORD dwWritten;
                                WriteFile(hFile, buf, len, &dwWritten, NULL);
                                VirtualFree(buf, 0, MEM_RELEASE);
                            }
                        }
                        CloseHandle(hFile);
                    }
                }
            }
            break;
        }
        case WM_SYSKEYDOWN: {
            if (wParam == 'H' || wParam == 'h') {
                ShowHelpDialog(hwnd);
                return 0;
            }
            if (wParam == 'M' || wParam == 'm') {
                SendMessage(hwnd, WM_COMMAND, 4, 0);
                return 0;
            }
            if (wParam == 'P' || wParam == 'p') {
                SendMessage(hwnd, WM_COMMAND, 5, 0);
                return 0;
            }
            if (wParam == 'S' || wParam == 's') {
                SendMessage(hwnd, WM_COMMAND, 6, 0);
                return 0;
            }
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        case WM_KEYDOWN: {
            if (wParam == VK_F1) {
                ShowHelpDialog(hwnd);
                return 0;
            }
            if (wParam == VK_F5) {
                RunScript();
                return 0;
            }
            if (wParam == VK_F10) {
                int lv = 0;
                StepScript(&lv);
                return 0;
            }
            if (GetKeyState(VK_CONTROL) < 0 && (wParam == 'K' || wParam == 'k')) {
                SendMessage(hwnd, WM_COMMAND, 9, 0);
                return 0;
            }
            break;
        }
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            HWND hCtl = (HWND)lParam;
            SetBkMode(hdc, OPAQUE);
            SetBkColor(hdc, RGB(20, 24, 38));
            if (hCtl == hMemory) SetTextColor(hdc, RGB(56, 189, 248));
            else if (hCtl == hOutput) SetTextColor(hdc, RGB(196, 181, 253));
            else SetTextColor(hdc, RGB(241, 245, 249));
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
            int topY = S(40);
            int bottomH = nh - topY - S(10);
            if (bottomH < S(50)) bottomH = S(50);
            int panelGap = S(8);
            int panelW = (nw - S(16) - panelGap * 2) / 3;
            if (panelW < S(100)) panelW = S(100);

            MoveWindow(hInput, S(8), topY, panelW, bottomH, TRUE);
            MoveWindow(hOutput, S(8) + panelW + panelGap, topY, panelW, bottomH, TRUE);
            MoveWindow(hMemory, S(8) + (panelW + panelGap) * 2, topY, nw - S(8) - (S(8) + (panelW + panelGap) * 2), bottomH, TRUE);

            int repW = S(75);
            int repInputW = S(75);
            int rx3 = nw - S(8) - repW;
            int rx2 = rx3 - S(6) - repInputW;
            int rx1 = rx2 - S(6) - repInputW;
            if (rx1 > S(640)) {
                MoveWindow(hRegexFind, rx1, S(9), repInputW, S(24), TRUE);
                MoveWindow(hRegexRep, rx2, S(9), repInputW, S(24), TRUE);
                MoveWindow(hBtnRep, rx3, S(8), repW, S(26), TRUE);
            }
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
    HWND hwnd = CreateWindowEx(0, "KScriptApp", "KScript - [F5] Run | [F10] Step | [F1] Help", style,
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
