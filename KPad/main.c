#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <wincrypt.h>
#include <string.h>
#include <stdio.h>

#define W 800
#define H 600
#define MAX_TABS 10

typedef struct {
    HWND hEdit;
    char szPath[MAX_PATH];
    char szTitle[64];
    BOOL isModified;
} TABINFO;

TABINFO g_Tabs[MAX_TABS];
int g_ActiveTab = 0;
int g_NumTabs = 0;

HWND g_hMainWnd = NULL;
HWND g_hTabCtrl = NULL;
HWND g_hStatus = NULL;
HBRUSH g_bgBrush = NULL;
HFONT g_hFontGlobal = NULL;

UINT g_uFindMsg = 0;
HWND g_hFindDlg = NULL;
FINDREPLACEA g_frFind;
char g_szFindWhat[256] = {0};
char g_szReplaceWith[256] = {0};
BOOL g_bWordWrap = FALSE;

#define ID_FILE_NEW        9000
#define ID_FILE_OPEN       9001
#define ID_FILE_SAVE       9002
#define ID_FILE_SAVEAS     9003
#define ID_FILE_EXIT       9004
#define ID_EDIT_UNDO       9005
#define ID_EDIT_CUT        9006
#define ID_EDIT_COPY       9007
#define ID_EDIT_PASTE      9008
#define ID_EDIT_SELECTALL  9009
#define ID_EDIT_FIND       9010
#define ID_EDIT_REPLACE    9011
#define ID_EDIT_TIME_DATE  9012
#define ID_EDIT_UPPERCASE  9013
#define ID_EDIT_LOWERCASE  9014
#define ID_TAB_NEW         9015
#define ID_TAB_CLOSE       9016
#define ID_VIEW_STATS      9017
#define ID_VIEW_WRAP       9018
#define ID_HELP_SHORTCUTS  9020
#define ID_HELP_ABOUT      9021
#define ID_FILE_EXPORT_ENC 9022
#define ID_FILE_OPEN_ENC   9023
#define ID_EDIT_ENCRYPT    9024
#define ID_EDIT_DECRYPT    9025
#define ID_VIEW_DIAGNOSTICS 9026

void UpdateStatusBar() {
    if (!g_hStatus || g_NumTabs == 0) return;
    HWND hEdit = g_Tabs[g_ActiveTab].hEdit;
    if (!hEdit) return;

    DWORD start, end;
    SendMessageA(hEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
    LRESULT lineIdx = SendMessageA(hEdit, EM_LINEFROMCHAR, start, 0);
    LRESULT lineStartChar = SendMessageA(hEdit, EM_LINEINDEX, lineIdx, 0);
    int colIdx = (int)(start - lineStartChar + 1);

    int totalLines = (int)SendMessageA(hEdit, EM_GETLINECOUNT, 0, 0);
    int totalChars = GetWindowTextLengthA(hEdit);
    int selChars = (int)(end - start);

    char stat1[128], stat2[128], stat3[128];
    const char* docName = g_Tabs[g_ActiveTab].szPath[0] ? g_Tabs[g_ActiveTab].szPath : g_Tabs[g_ActiveTab].szTitle;
    wsprintfA(stat1, "%s%s", docName, g_Tabs[g_ActiveTab].isModified ? " *" : "");

    if (selChars > 0) {
        wsprintfA(stat2, "Ln %d, Col %d (Sel %d)", (int)lineIdx + 1, colIdx, selChars);
    } else {
        wsprintfA(stat2, "Ln %d, Col %d", (int)lineIdx + 1, colIdx);
    }
    wsprintfA(stat3, "Lines: %d | Chars: %d", totalLines, totalChars);

    SendMessageA(g_hStatus, SB_SETTEXTA, 0, (LPARAM)stat1);
    SendMessageA(g_hStatus, SB_SETTEXTA, 1, (LPARAM)stat2);
    SendMessageA(g_hStatus, SB_SETTEXTA, 2, (LPARAM)stat3);
    SendMessageA(g_hStatus, SB_SETTEXTA, 3, (LPARAM)"[F1] Help / Guide");

    if (g_hMainWnd) {
        char winTitle[160];
        wsprintfA(winTitle, "KPad Pro - [%s%s] - Press F1 for Help", g_Tabs[g_ActiveTab].szTitle, g_Tabs[g_ActiveTab].isModified ? " *" : "");
        SetWindowTextA(g_hMainWnd, winTitle);
    }
}

void ResizeControls(int width, int height) {
    if (!g_hTabCtrl) return;
    RECT rcStatus = {0};
    if (g_hStatus) {
        SendMessage(g_hStatus, WM_SIZE, 0, 0);
        GetWindowRect(g_hStatus, &rcStatus);
    }
    int statusHeight = rcStatus.bottom - rcStatus.top;
    if (statusHeight <= 0) statusHeight = 22;

    MoveWindow(g_hTabCtrl, 0, 0, width, height - statusHeight, TRUE);

    RECT rcTab;
    GetClientRect(g_hTabCtrl, &rcTab);
    SendMessage(g_hTabCtrl, TCM_ADJUSTRECT, FALSE, (LPARAM)&rcTab);

    for (int i = 0; i < g_NumTabs; i++) {
        if (g_Tabs[i].hEdit) {
            MoveWindow(g_Tabs[i].hEdit, rcTab.left, rcTab.top, rcTab.right - rcTab.left, rcTab.bottom - rcTab.top, TRUE);
        }
    }
}

void SwitchTab(int index) {
    if (index < 0 || index >= g_NumTabs) return;
    for (int i = 0; i < g_NumTabs; i++) {
        if (g_Tabs[i].hEdit) {
            ShowWindow(g_Tabs[i].hEdit, (i == index) ? SW_SHOW : SW_HIDE);
        }
    }
    g_ActiveTab = index;
    SendMessage(g_hTabCtrl, TCM_SETCURSEL, index, 0);
    SetFocus(g_Tabs[index].hEdit);
    UpdateStatusBar();
}

void UpdateTabTitle(int index) {
    if (index < 0 || index >= g_NumTabs) return;
    char displayTitle[80];
    wsprintfA(displayTitle, "%s%s", g_Tabs[index].szTitle, g_Tabs[index].isModified ? " *" : "");
    
    TCITEM tie;
    tie.mask = TCIF_TEXT;
    tie.pszText = displayTitle;
    SendMessage(g_hTabCtrl, TCM_SETITEM, index, (LPARAM)&tie);
}

void AddTab(const char* name, const char* path) {
    if (g_NumTabs >= MAX_TABS) {
        MessageBoxA(g_hMainWnd, "Maximum tab limit reached (10 tabs).", "KPad Pro", MB_OK | MB_ICONWARNING);
        return;
    }

    int idx = g_NumTabs;
    g_Tabs[idx].szPath[0] = 0;
    g_Tabs[idx].isModified = FALSE;

    if (name && name[0]) {
        lstrcpynA(g_Tabs[idx].szTitle, name, sizeof(g_Tabs[idx].szTitle));
    } else {
        wsprintfA(g_Tabs[idx].szTitle, "Untitled %d", idx + 1);
    }
    if (path) {
        lstrcpynA(g_Tabs[idx].szPath, path, sizeof(g_Tabs[idx].szPath));
    }

    TCITEM tie;
    tie.mask = TCIF_TEXT;
    tie.pszText = g_Tabs[idx].szTitle;
    SendMessage(g_hTabCtrl, TCM_INSERTITEM, idx, (LPARAM)&tie);

    RECT rcTab;
    GetClientRect(g_hTabCtrl, &rcTab);
    SendMessage(g_hTabCtrl, TCM_ADJUSTRECT, FALSE, (LPARAM)&rcTab);

    DWORD style = WS_CHILD | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | ES_AUTOHSCROLL;
    if (g_bWordWrap) style &= ~WS_HSCROLL;

    g_Tabs[idx].hEdit = CreateWindowExA(0, "EDIT", "", style,
        rcTab.left, rcTab.top, rcTab.right - rcTab.left, rcTab.bottom - rcTab.top,
        g_hMainWnd, (HMENU)(UINT_PTR)(1000 + idx), GetModuleHandle(NULL), NULL);

    if (g_hFontGlobal) {
        SendMessage(g_Tabs[idx].hEdit, WM_SETFONT, (WPARAM)g_hFontGlobal, TRUE);
    }
    SendMessage(g_Tabs[idx].hEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(8, 8));

    g_NumTabs++;
    SwitchTab(idx);
}

void CloseTab(int index) {
    if (g_NumTabs <= 1) {
        // Clear remaining tab content
        SetWindowTextA(g_Tabs[0].hEdit, "");
        g_Tabs[0].szPath[0] = 0;
        lstrcpyA(g_Tabs[0].szTitle, "Untitled 1");
        g_Tabs[0].isModified = FALSE;
        UpdateTabTitle(0);
        UpdateStatusBar();
        return;
    }

    if (g_Tabs[index].isModified) {
        char msg[128];
        wsprintfA(msg, "Save changes to %s?", g_Tabs[index].szTitle);
        int res = MessageBoxA(g_hMainWnd, msg, "KPad Pro", MB_YESNOCANCEL | MB_ICONQUESTION);
        if (res == IDCANCEL) return;
    }

    DestroyWindow(g_Tabs[index].hEdit);
    SendMessage(g_hTabCtrl, TCM_DELETEITEM, index, 0);

    for (int i = index; i < g_NumTabs - 1; i++) {
        g_Tabs[i] = g_Tabs[i + 1];
    }
    g_NumTabs--;

    if (g_ActiveTab >= g_NumTabs) g_ActiveTab = g_NumTabs - 1;
    SwitchTab(g_ActiveTab);
}

void OpenFileNative() {
    OPENFILENAMEA ofn;
    char szFile[260] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hMainWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0Code Files (*.c;*.cpp;*.h;*.html;*.js;*.py;*.json;*.md)\0*.c;*.cpp;*.h;*.html;*.js;*.py;*.json;*.md\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD fileSize = GetFileSize(hFile, NULL);
            char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileSize + 1);
            if (buf) {
                DWORD bytesRead;
                ReadFile(hFile, buf, fileSize, &bytesRead, NULL);
                buf[fileSize] = 0;

                // Extract filename
                char title[64] = {0};
                int start = 0;
                for (int i = 0; szFile[i]; i++) {
                    if (szFile[i] == '\\' || szFile[i] == '/') start = i + 1;
                }
                lstrcpynA(title, szFile + start, 60);

                // If current tab is empty and unmodified, load into current tab
                int lenCurrent = GetWindowTextLengthA(g_Tabs[g_ActiveTab].hEdit);
                if (lenCurrent == 0 && !g_Tabs[g_ActiveTab].isModified && g_Tabs[g_ActiveTab].szPath[0] == 0) {
                    SetWindowTextA(g_Tabs[g_ActiveTab].hEdit, buf);
                    lstrcpyA(g_Tabs[g_ActiveTab].szTitle, title);
                    lstrcpyA(g_Tabs[g_ActiveTab].szPath, szFile);
                    g_Tabs[g_ActiveTab].isModified = FALSE;
                    UpdateTabTitle(g_ActiveTab);
                    UpdateStatusBar();
                } else {
                    AddTab(title, szFile);
                    SetWindowTextA(g_Tabs[g_ActiveTab].hEdit, buf);
                    g_Tabs[g_ActiveTab].isModified = FALSE;
                    UpdateTabTitle(g_ActiveTab);
                    UpdateStatusBar();
                }

                HeapFree(GetProcessHeap(), 0, buf);
            }
            CloseHandle(hFile);
        }
    }
}

void SaveFileNative(BOOL saveAs) {
    if (g_NumTabs == 0) return;
    char szFile[260] = {0};
    if (!saveAs && g_Tabs[g_ActiveTab].szPath[0] != 0) {
        lstrcpyA(szFile, g_Tabs[g_ActiveTab].szPath);
    } else {
        OPENFILENAMEA ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = g_hMainWnd;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrDefExt = "txt";
        ofn.Flags = OFN_OVERWRITEPROMPT;
        if (!GetSaveFileNameA(&ofn)) return;
    }

    HANDLE hFile = CreateFileA(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        int len = GetWindowTextLengthA(g_Tabs[g_ActiveTab].hEdit);
        char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len + 1);
        if (buf) {
            GetWindowTextA(g_Tabs[g_ActiveTab].hEdit, buf, len + 1);
            DWORD bytesWritten;
            WriteFile(hFile, buf, lstrlenA(buf), &bytesWritten, NULL);
            HeapFree(GetProcessHeap(), 0, buf);

            char title[64] = {0};
            int start = 0;
            for (int i = 0; szFile[i]; i++) {
                if (szFile[i] == '\\' || szFile[i] == '/') start = i + 1;
            }
            lstrcpynA(title, szFile + start, 60);

            lstrcpyA(g_Tabs[g_ActiveTab].szPath, szFile);
            lstrcpyA(g_Tabs[g_ActiveTab].szTitle, title);
            g_Tabs[g_ActiveTab].isModified = FALSE;
            UpdateTabTitle(g_ActiveTab);
            UpdateStatusBar();
        }
        CloseHandle(hFile);
    }
}

void DoFindReplace(BOOL isReplace) {
    if (!g_hFindDlg) {
        ZeroMemory(&g_frFind, sizeof(g_frFind));
        g_frFind.lStructSize = sizeof(g_frFind);
        g_frFind.hwndOwner = g_hMainWnd;
        g_frFind.lpstrFindWhat = g_szFindWhat;
        g_frFind.wFindWhatLen = sizeof(g_szFindWhat);
        g_frFind.lpstrReplaceWith = g_szReplaceWith;
        g_frFind.wReplaceWithLen = sizeof(g_szReplaceWith);
        g_frFind.Flags = FR_DOWN;
        if (isReplace) {
            g_hFindDlg = ReplaceTextA(&g_frFind);
        } else {
            g_hFindDlg = FindTextA(&g_frFind);
        }
    }
}

void TransformCase(BOOL uppercase) {
    HWND hEdit = g_Tabs[g_ActiveTab].hEdit;
    DWORD start, end;
    SendMessageA(hEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
    if (start == end) return;

    int len = GetWindowTextLengthA(hEdit);
    char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len + 1);
    if (buf) {
        GetWindowTextA(hEdit, buf, len + 1);
        for (DWORD i = start; i < end && i < (DWORD)len; i++) {
            if (uppercase) {
                if (buf[i] >= 'a' && buf[i] <= 'z') buf[i] -= 32;
            } else {
                if (buf[i] >= 'A' && buf[i] <= 'Z') buf[i] += 32;
            }
        }
        SetWindowTextA(hEdit, buf);
        SendMessageA(hEdit, EM_SETSEL, start, end);
        HeapFree(GetProcessHeap(), 0, buf);
    }
}

void ShowStatsDialog() {
    HWND hEdit = g_Tabs[g_ActiveTab].hEdit;
    int len = GetWindowTextLengthA(hEdit);
    char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len + 1);
    int words = 0;
    int lines = (int)SendMessageA(hEdit, EM_GETLINECOUNT, 0, 0);

    if (buf) {
        GetWindowTextA(hEdit, buf, len + 1);
        BOOL inWord = FALSE;
        for (int i = 0; buf[i]; i++) {
            if (buf[i] > 32) {
                if (!inWord) { inWord = TRUE; words++; }
            } else {
                inWord = FALSE;
            }
        }
        HeapFree(GetProcessHeap(), 0, buf);
    }

    char msg[256];
    wsprintfA(msg, "Document Statistics:\n\nLines: %d\nWords: %d\nCharacters: %d\nFile: %s",
        lines, words, len, g_Tabs[g_ActiveTab].szPath[0] ? g_Tabs[g_ActiveTab].szPath : "Unsaved");
    MessageBoxA(g_hMainWnd, msg, "Document Stats - KPad Pro", MB_OK | MB_ICONINFORMATION);
}

// --- Document Security & Cryptography Suite ---
static const char g_b64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void Base64Encode(const unsigned char* in, int len, char* out) {
    int i = 0, j = 0;
    while (i < len) {
        unsigned int a = in[i++];
        unsigned int b = (i < len) ? in[i++] : 0;
        unsigned int c = (i < len) ? in[i++] : 0;

        unsigned int triple = (a << 16) | (b << 8) | c;

        out[j++] = g_b64Table[(triple >> 18) & 0x3F];
        out[j++] = g_b64Table[(triple >> 12) & 0x3F];
        out[j++] = (i > len + 1) ? '=' : g_b64Table[(triple >> 6) & 0x3F];
        out[j++] = (i > len) ? '=' : g_b64Table[triple & 0x3F];
    }
    out[j] = '\0';
}

int Base64Decode(const char* in, unsigned char* out) {
    int in_len = lstrlenA(in);
    int i = 0, j = 0;
    while (i < in_len) {
        if (in[i] == '\r' || in[i] == '\n' || in[i] == ' ' || in[i] == '\t') { i++; continue; }
        if (in[i] == '=') break;

        int b[4] = {0};
        int count = 0;
        for (int k = 0; k < 4 && i < in_len; k++) {
            while (i < in_len && (in[i] == '\r' || in[i] == '\n' || in[i] == ' ' || in[i] == '\t')) i++;
            if (i >= in_len || in[i] == '=') break;
            char c = in[i++];
            if (c >= 'A' && c <= 'Z') b[k] = c - 'A';
            else if (c >= 'a' && c <= 'z') b[k] = c - 'a' + 26;
            else if (c >= '0' && c <= '9') b[k] = c - '0' + 52;
            else if (c == '+') b[k] = 62;
            else if (c == '/') b[k] = 63;
            count++;
        }
        if (count >= 2) out[j++] = (unsigned char)((b[0] << 2) | (b[1] >> 4));
        if (count >= 3) out[j++] = (unsigned char)(((b[1] & 0x0F) << 4) | (b[2] >> 2));
        if (count >= 4) out[j++] = (unsigned char)(((b[2] & 0x03) << 6) | b[3]);
    }
    return j;
}

unsigned int CalculateCRC32(const unsigned char* data, int len) {
    unsigned int crc = 0xFFFFFFFF;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(int)(crc & 1)));
        }
    }
    return ~crc;
}

void RC4Transform(const unsigned char* key, int keyLen, unsigned char* data, int dataLen) {
    if (keyLen <= 0 || dataLen <= 0) return;
    unsigned char s[256];
    for (int i = 0; i < 256; i++) s[i] = (unsigned char)i;
    int j = 0;
    for (int i = 0; i < 256; i++) {
        j = (j + s[i] + key[i % keyLen]) & 255;
        unsigned char tmp = s[i]; s[i] = s[j]; s[j] = tmp;
    }
    int i = 0; j = 0;
    for (int k = 0; k < dataLen; k++) {
        i = (i + 1) & 255;
        j = (j + s[i]) & 255;
        unsigned char tmp = s[i]; s[i] = s[j]; s[j] = tmp;
        data[k] ^= s[(s[i] + s[j]) & 255];
    }
}

char g_szPasswordPromptResult[128] = {0};
BOOL g_bPasswordPromptOK = FALSE;

LRESULT CALLBACK PasswordDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            CreateWindowExA(0, "STATIC", "Enter Password / Passphrase Key:", WS_CHILD | WS_VISIBLE, 15, 12, 260, 18, hwnd, NULL, GetModuleHandle(NULL), NULL);
            HWND hEditPwd = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_PASSWORD | ES_AUTOHSCROLL, 15, 34, 250, 24, hwnd, (HMENU)101, GetModuleHandle(NULL), NULL);
            CreateWindowExA(0, "BUTTON", "OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 85, 70, 80, 26, hwnd, (HMENU)IDOK, GetModuleHandle(NULL), NULL);
            CreateWindowExA(0, "BUTTON", "Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 175, 70, 80, 26, hwnd, (HMENU)IDCANCEL, GetModuleHandle(NULL), NULL);
            SetFocus(hEditPwd);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == IDOK) {
                GetDlgItemTextA(hwnd, 101, g_szPasswordPromptResult, sizeof(g_szPasswordPromptResult));
                g_bPasswordPromptOK = TRUE;
                DestroyWindow(hwnd);
            } else if (LOWORD(wParam) == IDCANCEL) {
                g_bPasswordPromptOK = FALSE;
                DestroyWindow(hwnd);
            }
            break;
        }
        case WM_CLOSE:
            g_bPasswordPromptOK = FALSE;
            DestroyWindow(hwnd);
            break;
        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

BOOL PromptPassword(HWND hWndParent, const char* title, char* outPassword, int maxLen) {
    static BOOL s_registered = FALSE;
    if (!s_registered) {
        WNDCLASSA wc = {0};
        wc.lpfnWndProc = PasswordDlgProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = "KPadPwdDlgClass";
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        RegisterClassA(&wc);
        s_registered = TRUE;
    }

    g_bPasswordPromptOK = FALSE;
    g_szPasswordPromptResult[0] = 0;

    RECT rcParent;
    GetWindowRect(hWndParent, &rcParent);
    int dlgW = 295, dlgH = 145;
    int dlgX = rcParent.left + (rcParent.right - rcParent.left - dlgW) / 2;
    int dlgY = rcParent.top + (rcParent.bottom - rcParent.top - dlgH) / 2;

    HWND hDlg = CreateWindowExA(WS_EX_DLGMODALFRAME | WS_EX_TOPMOST, "KPadPwdDlgClass", title,
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, dlgX, dlgY, dlgW, dlgH, hWndParent, NULL, GetModuleHandle(NULL), NULL);

    if (!hDlg) return FALSE;

    EnableWindow(hWndParent, FALSE);
    MSG msg;
    while (IsWindow(hDlg) && GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_RETURN) {
                SendMessageA(hDlg, WM_COMMAND, IDOK, 0);
                continue;
            } else if (msg.wParam == VK_ESCAPE) {
                SendMessageA(hDlg, WM_COMMAND, IDCANCEL, 0);
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    EnableWindow(hWndParent, TRUE);
    SetForegroundWindow(hWndParent);
    if (g_NumTabs > 0 && g_Tabs[g_ActiveTab].hEdit) {
        SetFocus(g_Tabs[g_ActiveTab].hEdit);
    }

    if (g_bPasswordPromptOK && g_szPasswordPromptResult[0]) {
        lstrcpynA(outPassword, g_szPasswordPromptResult, maxLen);
        return TRUE;
    }
    return FALSE;
}

void ShowDetailedDiagnostics() {
    if (g_NumTabs == 0) return;
    HWND hEdit = g_Tabs[g_ActiveTab].hEdit;
    int len = GetWindowTextLengthA(hEdit);
    char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len + 1);
    int words = 0, lines = (int)SendMessageA(hEdit, EM_GETLINECOUNT, 0, 0);
    int charsNoSpace = 0;
    unsigned int crc = 0;

    if (buf) {
        GetWindowTextA(hEdit, buf, len + 1);
        BOOL inWord = FALSE;
        for (int i = 0; buf[i]; i++) {
            if (buf[i] > 32) {
                charsNoSpace++;
                if (!inWord) { inWord = TRUE; words++; }
            } else {
                inWord = FALSE;
            }
        }
        crc = CalculateCRC32((const unsigned char*)buf, len);
        HeapFree(GetProcessHeap(), 0, buf);
    }

    char msg[512];
    wsprintfA(msg, "Document Diagnostics & Integrity:\n\n"
                   "• Lines: %d\n"
                   "• Words: %d\n"
                   "• Characters (total): %d\n"
                   "• Characters (excluding spaces): %d\n"
                   "• Buffer Size: %d bytes\n"
                   "• CRC32 Checksum: 0x%08X\n"
                   "• File: %s",
        lines, words, len, charsNoSpace, len, crc,
        g_Tabs[g_ActiveTab].szPath[0] ? g_Tabs[g_ActiveTab].szPath : "Untitled (Unsaved)");
    MessageBoxA(g_hMainWnd, msg, "Document Diagnostics - KPad Pro", MB_OK | MB_ICONINFORMATION);
}

void EncryptBufferAction() {
    if (g_NumTabs == 0) return;
    HWND hEdit = g_Tabs[g_ActiveTab].hEdit;
    int len = GetWindowTextLengthA(hEdit);
    if (len <= 0) {
        MessageBoxA(g_hMainWnd, "Document buffer is empty.", "KPad Pro Security", MB_OK | MB_ICONWARNING);
        return;
    }

    char password[128] = {0};
    if (!PromptPassword(g_hMainWnd, "Encrypt Document", password, sizeof(password))) {
        return;
    }

    char* plain = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len + 1);
    if (!plain) return;
    GetWindowTextA(hEdit, plain, len + 1);

    unsigned char salt[8] = {0};
    HCRYPTPROV hProv = 0;
    if (CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        CryptGenRandom(hProv, sizeof(salt), salt);
        CryptReleaseContext(hProv, 0);
    } else {
        DWORD tc = GetTickCount();
        for (int i = 0; i < 8; i++) salt[i] = (unsigned char)((tc >> (i * 4)) ^ (i * 37));
    }

    unsigned int crc = CalculateCRC32((const unsigned char*)plain, len);
    int payloadLen = len + 4;
    unsigned char* payload = (unsigned char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, payloadLen);
    if (!payload) {
        HeapFree(GetProcessHeap(), 0, plain);
        return;
    }
    payload[0] = (unsigned char)(crc & 0xFF);
    payload[1] = (unsigned char)((crc >> 8) & 0xFF);
    payload[2] = (unsigned char)((crc >> 16) & 0xFF);
    payload[3] = (unsigned char)((crc >> 24) & 0xFF);
    for (int i = 0; i < len; i++) payload[4 + i] = (unsigned char)plain[i];

    int passLen = lstrlenA(password);
    int keyLen = passLen + 8;
    unsigned char* key = (unsigned char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, keyLen);
    for (int i = 0; i < passLen; i++) key[i] = (unsigned char)password[i];
    for (int i = 0; i < 8; i++) key[passLen + i] = salt[i];

    RC4Transform(key, keyLen, payload, payloadLen);

    char saltB64[32] = {0};
    Base64Encode(salt, 8, saltB64);

    int cipherB64Len = ((payloadLen + 2) / 3) * 4 + 8;
    char* cipherB64 = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cipherB64Len);
    Base64Encode(payload, payloadLen, cipherB64);

    const char* header1 = "-----BEGIN KPAD ENCRYPTED DOCUMENT-----\r\nVersion: 1.0\r\nCipher: RC4-CRC32\r\nSalt: ";
    const char* header2 = "\r\n\r\n";
    const char* footer = "\r\n-----END KPAD ENCRYPTED DOCUMENT-----\r\n";

    int totalOutLen = lstrlenA(header1) + lstrlenA(saltB64) + lstrlenA(header2) + lstrlenA(cipherB64) + lstrlenA(footer) + 8;
    char* armored = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, totalOutLen);
    if (armored) {
        lstrcpyA(armored, header1);
        lstrcatA(armored, saltB64);
        lstrcatA(armored, header2);
        lstrcatA(armored, cipherB64);
        lstrcatA(armored, footer);

        SetWindowTextA(hEdit, armored);
        g_Tabs[g_ActiveTab].isModified = TRUE;
        UpdateTabTitle(g_ActiveTab);
        UpdateStatusBar();
        MessageBoxA(g_hMainWnd, "Document successfully encrypted with password protection!", "KPad Pro Security", MB_OK | MB_ICONINFORMATION);

        HeapFree(GetProcessHeap(), 0, armored);
    }

    HeapFree(GetProcessHeap(), 0, plain);
    HeapFree(GetProcessHeap(), 0, payload);
    HeapFree(GetProcessHeap(), 0, key);
    HeapFree(GetProcessHeap(), 0, cipherB64);
}

void DecryptBufferAction() {
    if (g_NumTabs == 0) return;
    HWND hEdit = g_Tabs[g_ActiveTab].hEdit;
    int len = GetWindowTextLengthA(hEdit);
    if (len <= 0) return;

    char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len + 1);
    if (!buf) return;
    GetWindowTextA(hEdit, buf, len + 1);

    const char* startTag = "-----BEGIN KPAD ENCRYPTED DOCUMENT-----";
    const char* endTag = "-----END KPAD ENCRYPTED DOCUMENT-----";
    char* pStart = strstr(buf, startTag);
    char* pEnd = strstr(buf, endTag);
    if (!pStart || !pEnd) {
        MessageBoxA(g_hMainWnd, "Buffer does not contain a valid KPad encrypted container.", "KPad Pro Security", MB_OK | MB_ICONWARNING);
        HeapFree(GetProcessHeap(), 0, buf);
        return;
    }

    char* pSalt = strstr(pStart, "Salt: ");
    if (!pSalt) {
        MessageBoxA(g_hMainWnd, "Corrupted container: Salt header missing.", "KPad Pro Security", MB_OK | MB_ICONERROR);
        HeapFree(GetProcessHeap(), 0, buf);
        return;
    }
    pSalt += 6;
    char saltB64[32] = {0};
    int sIdx = 0;
    while (*pSalt && *pSalt != '\r' && *pSalt != '\n' && sIdx < 30) {
        saltB64[sIdx++] = *pSalt++;
    }
    saltB64[sIdx] = 0;

    char* pPayload = strstr(pStart, "\r\n\r\n");
    if (!pPayload) pPayload = strstr(pStart, "\n\n");
    if (!pPayload || pPayload > pEnd) {
        MessageBoxA(g_hMainWnd, "Corrupted container: Payload header boundary missing.", "KPad Pro Security", MB_OK | MB_ICONERROR);
        HeapFree(GetProcessHeap(), 0, buf);
        return;
    }
    while (*pPayload == '\r' || *pPayload == '\n') pPayload++;

    int payloadB64Len = (int)(pEnd - pPayload);
    char* payloadB64 = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, payloadB64Len + 1);
    for (int i = 0; i < payloadB64Len; i++) payloadB64[i] = pPayload[i];
    payloadB64[payloadB64Len] = 0;

    char password[128] = {0};
    if (!PromptPassword(g_hMainWnd, "Decrypt Document", password, sizeof(password))) {
        HeapFree(GetProcessHeap(), 0, buf);
        HeapFree(GetProcessHeap(), 0, payloadB64);
        return;
    }

    unsigned char salt[16] = {0};
    Base64Decode(saltB64, salt);

    int maxDecLen = payloadB64Len + 16;
    unsigned char* decryptedData = (unsigned char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, maxDecLen);
    int decBytes = Base64Decode(payloadB64, decryptedData);

    int passLen = lstrlenA(password);
    int keyLen = passLen + 8;
    unsigned char* key = (unsigned char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, keyLen);
    for (int i = 0; i < passLen; i++) key[i] = (unsigned char)password[i];
    for (int i = 0; i < 8; i++) key[passLen + i] = salt[i];

    RC4Transform(key, keyLen, decryptedData, decBytes);

    if (decBytes < 4) {
        MessageBoxA(g_hMainWnd, "Decryption failed: Payload corrupted.", "KPad Pro Security", MB_OK | MB_ICONERROR);
    } else {
        unsigned int storedCRC = (unsigned int)(decryptedData[0]) | ((unsigned int)(decryptedData[1]) << 8) | ((unsigned int)(decryptedData[2]) << 16) | ((unsigned int)(decryptedData[3]) << 24);
        int plainLen = decBytes - 4;
        char* plainText = (char*)(decryptedData + 4);
        unsigned int calcCRC = CalculateCRC32((const unsigned char*)plainText, plainLen);

        if (storedCRC == calcCRC) {
            plainText[plainLen] = 0;
            SetWindowTextA(hEdit, plainText);
            g_Tabs[g_ActiveTab].isModified = TRUE;
            UpdateTabTitle(g_ActiveTab);
            UpdateStatusBar();
            MessageBoxA(g_hMainWnd, "Document successfully unlocked and decrypted!", "KPad Pro Security", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBoxA(g_hMainWnd, "Decryption failed: Incorrect password key or corrupted file.", "KPad Pro Security", MB_OK | MB_ICONERROR);
        }
    }

    HeapFree(GetProcessHeap(), 0, buf);
    HeapFree(GetProcessHeap(), 0, payloadB64);
    HeapFree(GetProcessHeap(), 0, decryptedData);
    HeapFree(GetProcessHeap(), 0, key);
}

void ExportEncryptedFile() {
    if (g_NumTabs == 0) return;
    HWND hEdit = g_Tabs[g_ActiveTab].hEdit;
    int len = GetWindowTextLengthA(hEdit);
    if (len <= 0) {
        MessageBoxA(g_hMainWnd, "Document buffer is empty.", "KPad Pro", MB_OK | MB_ICONWARNING);
        return;
    }

    char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len + 1);
    if (!buf) return;
    GetWindowTextA(hEdit, buf, len + 1);

    if (!strstr(buf, "-----BEGIN KPAD ENCRYPTED DOCUMENT-----")) {
        HeapFree(GetProcessHeap(), 0, buf);
        EncryptBufferAction();
    } else {
        HeapFree(GetProcessHeap(), 0, buf);
    }
    SaveFileNative(TRUE);
}

void OpenEncryptedFile() {
    OpenFileNative();
    if (g_NumTabs > 0) {
        HWND hEdit = g_Tabs[g_ActiveTab].hEdit;
        int len = GetWindowTextLengthA(hEdit);
        if (len > 0) {
            char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len + 1);
            if (buf) {
                GetWindowTextA(hEdit, buf, len + 1);
                if (strstr(buf, "-----BEGIN KPAD ENCRYPTED DOCUMENT-----")) {
                    if (MessageBoxA(g_hMainWnd, "This document is encrypted. Would you like to decrypt it now?", "KPad Pro Security", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                        DecryptBufferAction();
                    }
                }
                HeapFree(GetProcessHeap(), 0, buf);
            }
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == g_uFindMsg && g_uFindMsg != 0) {
        LPFINDREPLACEA lpfr = (LPFINDREPLACEA)lParam;
        if (lpfr->Flags & FR_DIALOGTERM) {
            g_hFindDlg = NULL;
            return 0;
        }

        HWND hEdit = g_Tabs[g_ActiveTab].hEdit;
        int len = GetWindowTextLengthA(hEdit);
        char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
        if (buf) {
            GetWindowTextA(hEdit, buf, len + 1);
            DWORD start, end;
            SendMessageA(hEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);

            char* findWhat = lpfr->lpstrFindWhat;
            int findLen = lstrlenA(findWhat);

            if ((lpfr->Flags & FR_FINDNEXT) || (lpfr->Flags & FR_REPLACE) || (lpfr->Flags & FR_REPLACEALL)) {
                BOOL matchCase = (lpfr->Flags & FR_MATCHCASE) ? TRUE : FALSE;
                int matchIdx = -1;

                for (int i = (lpfr->Flags & FR_FINDNEXT) ? (int)end : (int)start; i <= len - findLen; i++) {
                    BOOL match = TRUE;
                    for (int j = 0; j < findLen; j++) {
                        char c1 = buf[i + j];
                        char c2 = findWhat[j];
                        if (!matchCase) {
                            if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
                            if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
                        }
                        if (c1 != c2) { match = FALSE; break; }
                    }
                    if (match) { matchIdx = i; break; }
                }

                if (matchIdx == -1 && (lpfr->Flags & FR_FINDNEXT)) {
                    // Wrap around
                    for (int i = 0; i < (int)end && i <= len - findLen; i++) {
                        BOOL match = TRUE;
                        for (int j = 0; j < findLen; j++) {
                            char c1 = buf[i + j];
                            char c2 = findWhat[j];
                            if (!matchCase) {
                                if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
                                if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
                            }
                            if (c1 != c2) { match = FALSE; break; }
                        }
                        if (match) { matchIdx = i; break; }
                    }
                }

                if (matchIdx != -1) {
                    if (lpfr->Flags & FR_REPLACE) {
                        SendMessageA(hEdit, EM_SETSEL, matchIdx, matchIdx + findLen);
                        SendMessageA(hEdit, EM_REPLACESEL, TRUE, (LPARAM)lpfr->lpstrReplaceWith);
                    } else if (lpfr->Flags & FR_FINDNEXT) {
                        SendMessageA(hEdit, EM_SETSEL, matchIdx, matchIdx + findLen);
                        SendMessageA(hEdit, EM_SCROLLCARET, 0, 0);
                    }
                } else {
                    MessageBoxA(hwnd, "Text not found", "Find", MB_OK | MB_ICONINFORMATION);
                }
            }
            HeapFree(GetProcessHeap(), 0, buf);
        }
        return 0;
    }

    switch (msg) {
        case WM_CREATE: {
            g_hMainWnd = hwnd;
            InitCommonControls();

            HMENU hMenu = CreateMenu();
            HMENU hFileMenu = CreatePopupMenu();
            AppendMenuA(hFileMenu, MF_STRING, ID_FILE_NEW, "New Document\tCtrl+N");
            AppendMenuA(hFileMenu, MF_STRING, ID_FILE_OPEN, "Open...\tCtrl+O");
            AppendMenuA(hFileMenu, MF_STRING, ID_FILE_SAVE, "Save\tCtrl+S");
            AppendMenuA(hFileMenu, MF_STRING, ID_FILE_SAVEAS, "Save As...");
            AppendMenuA(hFileMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hFileMenu, MF_STRING, ID_FILE_EXPORT_ENC, "Export Encrypted (.enc)...");
            AppendMenuA(hFileMenu, MF_STRING, ID_FILE_OPEN_ENC, "Open Encrypted (.enc)...");
            AppendMenuA(hFileMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hFileMenu, MF_STRING, ID_FILE_EXIT, "Exit");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, "File");

            HMENU hEditMenu = CreatePopupMenu();
            AppendMenuA(hEditMenu, MF_STRING, ID_EDIT_UNDO, "Undo\tCtrl+Z");
            AppendMenuA(hEditMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hEditMenu, MF_STRING, ID_EDIT_CUT, "Cut\tCtrl+X");
            AppendMenuA(hEditMenu, MF_STRING, ID_EDIT_COPY, "Copy\tCtrl+C");
            AppendMenuA(hEditMenu, MF_STRING, ID_EDIT_PASTE, "Paste\tCtrl+V");
            AppendMenuA(hEditMenu, MF_STRING, ID_EDIT_SELECTALL, "Select All\tCtrl+A");
            AppendMenuA(hEditMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hEditMenu, MF_STRING, ID_EDIT_FIND, "Find...\tCtrl+F");
            AppendMenuA(hEditMenu, MF_STRING, ID_EDIT_REPLACE, "Replace...\tCtrl+H");
            AppendMenuA(hEditMenu, MF_STRING, ID_EDIT_ENCRYPT, "Encrypt Buffer (Password Lock)...\tCtrl+E");
            AppendMenuA(hEditMenu, MF_STRING, ID_EDIT_DECRYPT, "Decrypt Buffer (Unlock)...\tCtrl+D");
            AppendMenuA(hEditMenu, MF_STRING, ID_EDIT_TIME_DATE, "Time/Date\tF5");
            AppendMenuA(hEditMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hEditMenu, MF_STRING, ID_EDIT_UPPERCASE, "Convert UPPERCASE");
            AppendMenuA(hEditMenu, MF_STRING, ID_EDIT_LOWERCASE, "Convert lowercase");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hEditMenu, "Edit");

            HMENU hTabMenu = CreatePopupMenu();
            AppendMenuA(hTabMenu, MF_STRING, ID_TAB_NEW, "New Tab\tCtrl+T");
            AppendMenuA(hTabMenu, MF_STRING, ID_TAB_CLOSE, "Close Tab\tCtrl+W");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hTabMenu, "Tabs");

            HMENU hViewMenu = CreatePopupMenu();
            AppendMenuA(hViewMenu, MF_STRING, ID_VIEW_STATS, "Document Stats...");
            AppendMenuA(hViewMenu, MF_STRING, ID_VIEW_DIAGNOSTICS, "Detailed Diagnostics & Integrity...");
            AppendMenuA(hViewMenu, MF_STRING, ID_VIEW_WRAP, "Toggle Word Wrap");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hViewMenu, "View");

            HMENU hHelpMenu = CreatePopupMenu();
            AppendMenuA(hHelpMenu, MF_STRING, ID_HELP_SHORTCUTS, "Keyboard Shortcuts\tF1");
            AppendMenuA(hHelpMenu, MF_STRING, ID_HELP_ABOUT, "About");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, "Help");

            SetMenu(hwnd, hMenu);

            g_uFindMsg = RegisterWindowMessageA(FINDMSGSTRINGA);
            g_bgBrush = CreateSolidBrush(RGB(15, 23, 42)); // Dark theme

            // Create Status Window
            g_hStatus = CreateStatusWindowA(WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, "", hwnd, 100);
            int parts[4] = { 250, 410, 580, -1 };
            SendMessage(g_hStatus, SB_SETPARTS, 4, (LPARAM)parts);

            // Create Tab Control
            g_hTabCtrl = CreateWindowExA(0, WC_TABCONTROLA, "", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
                                       0, 0, W, H, hwnd, NULL, GetModuleHandle(NULL), NULL);

            HDC hdc = GetDC(NULL);
            int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(NULL, hdc);
            int fontHeight = -MulDiv(12, dpi, 72);
            g_hFontGlobal = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Consolas");

            AddTab("Welcome", NULL);
            SetWindowTextA(g_Tabs[0].hEdit, "Welcome to KPad Pro!\r\n\r\nPress F1 for Help to view keyboard shortcuts.\r\n");
            g_Tabs[0].isModified = FALSE;
            break;
        }

        case WM_COMMAND: {
            WORD id = LOWORD(wParam);
            WORD code = HIWORD(wParam);

            if (code == EN_CHANGE) {
                for (int i = 0; i < g_NumTabs; i++) {
                    if ((HWND)lParam == g_Tabs[i].hEdit) {
                        if (!g_Tabs[i].isModified) {
                            g_Tabs[i].isModified = TRUE;
                            UpdateTabTitle(i);
                        }
                        UpdateStatusBar();
                        break;
                    }
                }
                return 0;
            }

            switch (id) {
                case ID_FILE_NEW:
                case ID_TAB_NEW:
                    AddTab(NULL, NULL);
                    break;
                case ID_FILE_OPEN:
                    OpenFileNative();
                    break;
                case ID_FILE_SAVE:
                    SaveFileNative(FALSE);
                    break;
                case ID_FILE_SAVEAS:
                    SaveFileNative(TRUE);
                    break;
                case ID_FILE_EXPORT_ENC:
                    ExportEncryptedFile();
                    break;
                case ID_FILE_OPEN_ENC:
                    OpenEncryptedFile();
                    break;
                case ID_FILE_EXIT:
                    PostMessageA(hwnd, WM_CLOSE, 0, 0);
                    break;
                case ID_TAB_CLOSE:
                    CloseTab(g_ActiveTab);
                    break;
                case ID_EDIT_UNDO:
                    SendMessageA(g_Tabs[g_ActiveTab].hEdit, WM_UNDO, 0, 0);
                    break;
                case ID_EDIT_CUT:
                    SendMessageA(g_Tabs[g_ActiveTab].hEdit, WM_CUT, 0, 0);
                    break;
                case ID_EDIT_COPY:
                    SendMessageA(g_Tabs[g_ActiveTab].hEdit, WM_COPY, 0, 0);
                    break;
                case ID_EDIT_PASTE:
                    SendMessageA(g_Tabs[g_ActiveTab].hEdit, WM_PASTE, 0, 0);
                    break;
                case ID_EDIT_SELECTALL:
                    SendMessageA(g_Tabs[g_ActiveTab].hEdit, EM_SETSEL, 0, -1);
                    break;
                case ID_EDIT_FIND:
                    DoFindReplace(FALSE);
                    break;
                case ID_EDIT_REPLACE:
                    DoFindReplace(TRUE);
                    break;
                case ID_EDIT_ENCRYPT:
                    EncryptBufferAction();
                    break;
                case ID_EDIT_DECRYPT:
                    DecryptBufferAction();
                    break;
                case ID_EDIT_TIME_DATE: {
                    SYSTEMTIME st;
                    GetLocalTime(&st);
                    char dt[64];
                    wsprintfA(dt, "%04d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
                    SendMessageA(g_Tabs[g_ActiveTab].hEdit, EM_REPLACESEL, TRUE, (LPARAM)dt);
                    break;
                }
                case ID_EDIT_UPPERCASE:
                    TransformCase(TRUE);
                    break;
                case ID_EDIT_LOWERCASE:
                    TransformCase(FALSE);
                    break;
                case ID_VIEW_STATS:
                    ShowStatsDialog();
                    break;
                case ID_VIEW_DIAGNOSTICS:
                    ShowDetailedDiagnostics();
                    break;
                case ID_VIEW_WRAP:
                    g_bWordWrap = !g_bWordWrap;
                    for (int i = 0; i < g_NumTabs; i++) {
                        DWORD style = GetWindowLongA(g_Tabs[i].hEdit, GWL_STYLE);
                        if (g_bWordWrap) style &= ~WS_HSCROLL;
                        else style |= WS_HSCROLL;
                        SetWindowLongA(g_Tabs[i].hEdit, GWL_STYLE, style);
                    }
                    break;
                case ID_HELP_SHORTCUTS:
                    MessageBoxA(hwnd,
                        "KPad Pro - Keyboard Shortcuts Guide\n\n"
                        "DOCUMENT & TABS:\n"
                        "  • Ctrl+N / Ctrl+T  : New Document / Tab\n"
                        "  • Ctrl+O           : Open File\n"
                        "  • Ctrl+S           : Save Document\n"
                        "  • Ctrl+W           : Close Active Tab\n"
                        "  • Ctrl+1 .. 9      : Switch to Tab 1-9\n"
                        "  • Ctrl+Tab         : Next Tab Cycle\n\n"
                        "EDITING & SEARCH:\n"
                        "  • Ctrl+F / Ctrl+H  : Find / Replace\n"
                        "  • Ctrl+A           : Select All\n"
                        "  • Alt+Z            : Toggle Word Wrap\n"
                        "  • F5               : Insert Date & Time\n\n"
                        "SECURITY & TOOLS:\n"
                        "  • Ctrl+E           : Encrypt Document\n"
                        "  • Ctrl+D           : Decrypt Document\n"
                        "  • F1               : Show this Help Guide",
                        "KPad Pro User Guide [F1]", MB_OK | MB_ICONINFORMATION);
                    break;
                case ID_HELP_ABOUT:
                    MessageBoxA(hwnd, "KPad Pro v1.4\nAdvanced Text & Code Editor with AES & RC4 Encryption Suite\nPart of KiloOS Suite", "About KPad Pro", MB_OK | MB_ICONINFORMATION);
                    break;
            }
            break;
        }

        case WM_GETMINMAXINFO: {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = 480;
            lpMMI->ptMinTrackSize.y = 360;
            return 0;
        }

        case WM_NOTIFY: {
            LPNMHDR pnm = (LPNMHDR)lParam;
            if (pnm->hwndFrom == g_hTabCtrl && pnm->code == TCN_SELCHANGE) {
                int iPage = SendMessage(g_hTabCtrl, TCM_GETCURSEL, 0, 0);
                SwitchTab(iPage);
            }
            break;
        }

        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(15, 23, 42));      // Dark background
            SetTextColor(hdc, RGB(248, 250, 252)); // Light text
            return (LRESULT)g_bgBrush;
        }

        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            ResizeControls(nw, nh);
            break;
        }

        case WM_DESTROY:
            if (g_bgBrush) DeleteObject(g_bgBrush);
            if (g_hFontGlobal) DeleteObject(g_hFontGlobal);
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

char* __cdecl strstr(const char* haystack, const char* needle) {
    if (!haystack || !needle) return NULL;
    if (!*needle) return (char*)haystack;
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && *h == *n) {
            h++;
            n++;
        }
        if (!*n) return (char*)haystack;
        haystack++;
    }
    return NULL;
}

void MainEntry() {
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        typedef BOOL (WINAPI *SetProcessDPIAwareFunc)();
        SetProcessDPIAwareFunc setDpiAware = (SetProcessDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (setDpiAware) setDpiAware();
    }
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KPadApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = CreateSolidBrush(RGB(15, 23, 42));
    RegisterClass(&wc);

    RECT rc = { 0, 0, W, H };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);

    HWND hwnd = CreateWindowEx(0, "KPadApp", "KPad Pro - Press F1 for Help", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (g_hFindDlg && IsDialogMessage(g_hFindDlg, &msg)) {
            continue;
        }
        if (msg.message == WM_KEYDOWN) {
            BOOL ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            if (msg.wParam == VK_F5) {
                SendMessage(hwnd, WM_COMMAND, ID_EDIT_TIME_DATE, 0);
                continue;
            }
            if (msg.wParam == VK_F1) {
                SendMessage(hwnd, WM_COMMAND, ID_HELP_SHORTCUTS, 0);
                continue;
            }
            if (ctrl && msg.wParam >= '1' && msg.wParam <= '9') {
                int target = (int)(msg.wParam - '1');
                if (target < g_NumTabs) {
                    SwitchTab(target);
                }
                continue;
            }
            if (ctrl && msg.wParam == VK_TAB) {
                if (g_NumTabs > 1) {
                    BOOL shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                    int nextTab = shift ? (g_ActiveTab - 1 + g_NumTabs) % g_NumTabs : (g_ActiveTab + 1) % g_NumTabs;
                    SwitchTab(nextTab);
                }
                continue;
            }
            if (ctrl && msg.wParam == 'A') {
                if (g_NumTabs > 0 && g_Tabs[g_ActiveTab].hEdit) {
                    SendMessage(g_Tabs[g_ActiveTab].hEdit, EM_SETSEL, 0, -1);
                }
                continue;
            }
            if (ctrl && msg.wParam == 'N') {
                SendMessage(hwnd, WM_COMMAND, ID_FILE_NEW, 0);
                continue;
            }
            if (ctrl && msg.wParam == 'O') {
                SendMessage(hwnd, WM_COMMAND, ID_FILE_OPEN, 0);
                continue;
            }
            if (ctrl && msg.wParam == 'S') {
                SendMessage(hwnd, WM_COMMAND, ID_FILE_SAVE, 0);
                continue;
            }
            if (ctrl && msg.wParam == 'F') {
                SendMessage(hwnd, WM_COMMAND, ID_EDIT_FIND, 0);
                continue;
            }
            if (ctrl && msg.wParam == 'H') {
                SendMessage(hwnd, WM_COMMAND, ID_EDIT_REPLACE, 0);
                continue;
            }
            if (ctrl && msg.wParam == 'E') {
                SendMessage(hwnd, WM_COMMAND, ID_EDIT_ENCRYPT, 0);
                continue;
            }
            if (ctrl && msg.wParam == 'D') {
                SendMessage(hwnd, WM_COMMAND, ID_EDIT_DECRYPT, 0);
                continue;
            }
            if (ctrl && msg.wParam == 'T') {
                SendMessage(hwnd, WM_COMMAND, ID_TAB_NEW, 0);
                continue;
            }
            if (ctrl && msg.wParam == 'W') {
                SendMessage(hwnd, WM_COMMAND, ID_TAB_CLOSE, 0);
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
