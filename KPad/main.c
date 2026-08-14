#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <string.h>
#include <stdio.h>

#define W 1000
#define H 700
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
    if (g_Tabs[g_ActiveTab].szPath[0] != 0) {
        wsprintfA(stat1, "%s%s", g_Tabs[g_ActiveTab].szPath, g_Tabs[g_ActiveTab].isModified ? " *" : "");
    } else {
        wsprintfA(stat1, "%s%s", g_Tabs[g_ActiveTab].szTitle, g_Tabs[g_ActiveTab].isModified ? " *" : "");
    }

    if (selChars > 0) {
        wsprintfA(stat2, "Ln %d, Col %d (Sel %d)", (int)lineIdx + 1, colIdx, selChars);
    } else {
        wsprintfA(stat2, "Ln %d, Col %d", (int)lineIdx + 1, colIdx);
    }
    wsprintfA(stat3, "Lines: %d | Chars: %d", totalLines, totalChars);

    SendMessageA(g_hStatus, SB_SETTEXTA, 0, (LPARAM)stat1);
    SendMessageA(g_hStatus, SB_SETTEXTA, 1, (LPARAM)stat2);
    SendMessageA(g_hStatus, SB_SETTEXTA, 2, (LPARAM)stat3);
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
            int parts[3] = { 280, 460, -1 };
            SendMessage(g_hStatus, SB_SETPARTS, 3, (LPARAM)parts);

            // Create Tab Control
            g_hTabCtrl = CreateWindowExA(0, WC_TABCONTROLA, "", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
                                       0, 0, W, H, hwnd, NULL, GetModuleHandle(NULL), NULL);

            g_hFontGlobal = CreateFontA(-18, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Consolas");

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
                    MessageBoxA(hwnd, "KPad Pro Keyboard Shortcuts:\n\nCtrl+N: New Document\nCtrl+O: Open Local File\nCtrl+S: Save\nCtrl+T: New Tab\nCtrl+W: Close Tab\nCtrl+F: Find\nCtrl+H: Replace\nAlt+Z: Toggle Word Wrap\nF5: Insert Time/Date\nF1: Show this help", "Keyboard Shortcuts", MB_OK | MB_ICONINFORMATION);
                    break;
                case ID_HELP_ABOUT:
                    MessageBoxA(hwnd, "KPad Pro\nAdvanced Text & Code Editor for KiloOS", "About", MB_OK | MB_ICONINFORMATION);
                    break;
            }
            break;
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

    HWND hwnd = CreateWindowEx(0, "KPadApp", "KPad Pro - Press F1 for Help", WS_OVERLAPPEDWINDOW,
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
