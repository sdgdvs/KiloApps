#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

#define MAX_TABS 12

typedef struct {
    char szTitle[64];
    char* pszText;
    DWORD dwTextLen;
    DWORD dwBookmark;
} KREAD_TAB;

static KREAD_TAB g_Tabs[MAX_TABS];
static int g_NumTabs = 0;
static int g_ActiveTab = 0;

static HWND g_hMainWnd = NULL;
static HWND g_hTabCtrl = NULL;
static HWND hEdit = NULL;

static HFONT hFont = NULL;
static HBRUSH hBrush = NULL;

static int currentFontSize = 18;
static char currentFontFace[32] = "Georgia";

static COLORREF g_bgColor = RGB(250, 250, 250);
static COLORREF g_textColor = RGB(30, 30, 30);

static char g_lastSearchQuery[128] = {0};

void UpdateFont(HWND hwnd) {
    if (hFont) DeleteObject(hFont);
    HDC hdc = GetDC(hwnd);
    int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(hwnd, hdc);
    int fontHeight = -MulDiv(currentFontSize, dpi, 72);
    hFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_ROMAN, currentFontFace);
    if (hEdit) {
        SendMessageA(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
}

void SetTheme(HWND hwnd, COLORREF bg, COLORREF text) {
    g_bgColor = bg;
    g_textColor = text;
    if (hBrush) DeleteObject(hBrush);
    hBrush = CreateSolidBrush(g_bgColor);
    if (hEdit) {
        InvalidateRect(hEdit, NULL, TRUE);
        UpdateWindow(hEdit);
    }
}

void SaveActiveTabState() {
    if (g_ActiveTab < 0 || g_ActiveTab >= g_NumTabs || !hEdit) return;
    
    // Save bookmark / cursor
    DWORD start = 0, end = 0;
    SendMessageA(hEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
    g_Tabs[g_ActiveTab].dwBookmark = start;

    // Save text
    int len = GetWindowTextLengthA(hEdit);
    if (g_Tabs[g_ActiveTab].pszText) {
        VirtualFree(g_Tabs[g_ActiveTab].pszText, 0, MEM_RELEASE);
        g_Tabs[g_ActiveTab].pszText = NULL;
        g_Tabs[g_ActiveTab].dwTextLen = 0;
    }
    if (len > 0) {
        char* buf = (char*)VirtualAlloc(NULL, len + 1, MEM_COMMIT, PAGE_READWRITE);
        if (buf) {
            GetWindowTextA(hEdit, buf, len + 1);
            g_Tabs[g_ActiveTab].pszText = buf;
            g_Tabs[g_ActiveTab].dwTextLen = len;
        }
    }
}

void LoadTabState(int index) {
    if (index < 0 || index >= g_NumTabs || !hEdit) return;
    g_ActiveTab = index;
    
    if (g_Tabs[index].pszText) {
        SetWindowTextA(hEdit, g_Tabs[index].pszText);
    } else {
        SetWindowTextA(hEdit, "");
    }
    
    DWORD bm = g_Tabs[index].dwBookmark;
    SendMessageA(hEdit, EM_SETSEL, bm, bm);
    SendMessageA(hEdit, EM_SCROLLCARET, 0, 0);
    
    if (g_hTabCtrl) {
        TabCtrl_SetCurSel(g_hTabCtrl, index);
    }
}

void SwitchToTab(HWND hwnd, int newIndex) {
    if (newIndex < 0 || newIndex >= g_NumTabs || newIndex == g_ActiveTab) return;
    SaveActiveTabState();
    LoadTabState(newIndex);
}

void AddNewTab(HWND hwnd, const char* title, const char* initialText) {
    if (g_NumTabs >= MAX_TABS) {
        MessageBoxA(hwnd, "Maximum number of tabs reached (12).", "KRead Tabs", MB_OK | MB_ICONWARNING);
        return;
    }

    SaveActiveTabState();

    int newIdx = g_NumTabs;
    g_NumTabs++;

    if (title && title[0]) {
        lstrcpynA(g_Tabs[newIdx].szTitle, title, 60);
    } else {
        wsprintfA(g_Tabs[newIdx].szTitle, "Document %d", newIdx + 1);
    }

    g_Tabs[newIdx].dwBookmark = 0;
    g_Tabs[newIdx].pszText = NULL;
    g_Tabs[newIdx].dwTextLen = 0;

    if (initialText) {
        int len = lstrlenA(initialText);
        if (len > 0) {
            char* buf = (char*)VirtualAlloc(NULL, len + 1, MEM_COMMIT, PAGE_READWRITE);
            if (buf) {
                lstrcpyA(buf, initialText);
                g_Tabs[newIdx].pszText = buf;
                g_Tabs[newIdx].dwTextLen = len;
            }
        }
    }

    TCITEMA tie;
    tie.mask = TCIF_TEXT;
    tie.pszText = g_Tabs[newIdx].szTitle;
    TabCtrl_InsertItem(g_hTabCtrl, newIdx, &tie);

    LoadTabState(newIdx);
}

void CloseCurrentTab(HWND hwnd) {
    if (g_NumTabs <= 1) {
        // Just reset current tab
        if (g_Tabs[0].pszText) {
            VirtualFree(g_Tabs[0].pszText, 0, MEM_RELEASE);
            g_Tabs[0].pszText = NULL;
            g_Tabs[0].dwTextLen = 0;
        }
        g_Tabs[0].dwBookmark = 0;
        lstrcpyA(g_Tabs[0].szTitle, "Untitled");
        
        TCITEMA tie;
        tie.mask = TCIF_TEXT;
        tie.pszText = g_Tabs[0].szTitle;
        TabCtrl_SetItem(g_hTabCtrl, 0, &tie);
        
        SetWindowTextA(hEdit, "");
        return;
    }

    int closingIdx = g_ActiveTab;
    if (g_Tabs[closingIdx].pszText) {
        VirtualFree(g_Tabs[closingIdx].pszText, 0, MEM_RELEASE);
        g_Tabs[closingIdx].pszText = NULL;
    }

    TabCtrl_DeleteItem(g_hTabCtrl, closingIdx);

    for (int i = closingIdx; i < g_NumTabs - 1; i++) {
        g_Tabs[i] = g_Tabs[i + 1];
    }
    g_NumTabs--;

    int nextIdx = closingIdx;
    if (nextIdx >= g_NumTabs) nextIdx = g_NumTabs - 1;

    LoadTabState(nextIdx);
}

void ExtractFileName(const char* fullPath, char* dest, int maxLen) {
    const char* p = fullPath;
    const char* lastSlash = fullPath;
    while (*p) {
        if (*p == '\\' || *p == '/') lastSlash = p + 1;
        p++;
    }
    lstrcpynA(dest, lastSlash, maxLen);
}

void OpenFileAndLoad(HWND hwnd, BOOL inNewTab) {
    OPENFILENAMEA ofn;
    char szFile[260] = {0};

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Text & Document Files\0*.txt;*.md;*.csv;*.log;*.json\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(ofn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD dwFileSize = GetFileSize(hFile, NULL);
            if (dwFileSize != INVALID_FILE_SIZE) {
                char* pszFileText = (char*)VirtualAlloc(NULL, dwFileSize + 1, MEM_COMMIT, PAGE_READWRITE);
                if (pszFileText) {
                    DWORD dwRead = 0;
                    if (ReadFile(hFile, pszFileText, dwFileSize, &dwRead, NULL)) {
                        pszFileText[dwRead] = 0;
                        
                        char fname[64];
                        ExtractFileName(ofn.lpstrFile, fname, sizeof(fname));

                        if (inNewTab || g_NumTabs == 0) {
                            AddNewTab(hwnd, fname, pszFileText);
                        } else {
                            SetWindowTextA(hEdit, pszFileText);
                            lstrcpynA(g_Tabs[g_ActiveTab].szTitle, fname, 60);
                            g_Tabs[g_ActiveTab].dwBookmark = 0;
                            
                            TCITEMA tie;
                            tie.mask = TCIF_TEXT;
                            tie.pszText = g_Tabs[g_ActiveTab].szTitle;
                            TabCtrl_SetItem(g_hTabCtrl, g_ActiveTab, &tie);
                        }
                    }
                    VirtualFree(pszFileText, 0, MEM_RELEASE);
                }
            }
            CloseHandle(hFile);
        }
    }
}

void SaveStatsExport(HWND hwnd, const char* statsText) {
    OPENFILENAMEA ofn;
    char szFile[260] = "KRead_Stats.txt";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD dwWritten;
            DWORD dwLen = lstrlenA(statsText);
            WriteFile(hFile, statsText, dwLen, &dwWritten, NULL);
            CloseHandle(hFile);
            MessageBoxA(hwnd, "Statistics exported successfully!", "KRead Export", MB_OK | MB_ICONINFORMATION);
        }
    }
}

void PerformSearch(HWND hwnd) {
    int len = GetWindowTextLengthA(hEdit);
    if (len <= 0) {
        MessageBoxA(hwnd, "Document is empty.", "Find Text", MB_OK | MB_ICONINFORMATION);
        return;
    }

    char* text = (char*)VirtualAlloc(NULL, len + 1, MEM_COMMIT, PAGE_READWRITE);
    if (!text) return;
    GetWindowTextA(hEdit, text, len + 1);

    DWORD selStart = 0, selEnd = 0;
    SendMessageA(hEdit, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);

    int qLen = lstrlenA(g_lastSearchQuery);
    if (qLen == 0) {
        VirtualFree(text, 0, MEM_RELEASE);
        return;
    }

    int foundIdx = -1;
    for (int i = (int)selEnd; i <= len - qLen; i++) {
        int match = 1;
        for (int j = 0; j < qLen; j++) {
            char c1 = text[i + j];
            char c2 = g_lastSearchQuery[j];
            if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
            if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
            if (c1 != c2) { match = 0; break; }
        }
        if (match) { foundIdx = i; break; }
    }

    // Wrap around if not found from cursor
    if (foundIdx == -1 && selEnd > 0) {
        for (int i = 0; i < (int)selEnd && i <= len - qLen; i++) {
            int match = 1;
            for (int j = 0; j < qLen; j++) {
                char c1 = text[i + j];
                char c2 = g_lastSearchQuery[j];
                if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
                if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
                if (c1 != c2) { match = 0; break; }
            }
            if (match) { foundIdx = i; break; }
        }
    }

    if (foundIdx != -1) {
        SendMessageA(hEdit, EM_SETSEL, foundIdx, foundIdx + qLen);
        SendMessageA(hEdit, EM_SCROLLCARET, 0, 0);
    } else {
        MessageBoxA(hwnd, "Text not found.", "Find Text", MB_OK | MB_ICONINFORMATION);
    }

    VirtualFree(text, 0, MEM_RELEASE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_hMainWnd = hwnd;
            HMENU hMenu = CreateMenu();
            
            // File Menu
            HMENU hSubFile = CreatePopupMenu();
            AppendMenuA(hSubFile, MF_STRING, 1001, "Open File...");
            AppendMenuA(hSubFile, MF_STRING, 1009, "Open File in New Tab...\tCtrl+O");
            AppendMenuA(hSubFile, MF_STRING, 1007, "Export Statistics...");
            AppendMenuA(hSubFile, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hSubFile, MF_STRING, 1002, "Exit");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubFile, "File");

            // Tabs Menu
            HMENU hSubTabs = CreatePopupMenu();
            AppendMenuA(hSubTabs, MF_STRING, 1030, "New Tab\tCtrl+T");
            AppendMenuA(hSubTabs, MF_STRING, 1031, "Close Current Tab\tCtrl+W");
            AppendMenuA(hSubTabs, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hSubTabs, MF_STRING, 1032, "Next Tab\tCtrl+Tab");
            AppendMenuA(hSubTabs, MF_STRING, 1033, "Previous Tab\tCtrl+Shift+Tab");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubTabs, "Tabs");

            // View Menu
            HMENU hSubView = CreatePopupMenu();
            AppendMenuA(hSubView, MF_STRING, 1003, "Reading Statistics Engine");
            AppendMenuA(hSubView, MF_STRING, 1004, "Find Text...");
            AppendMenuA(hSubView, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hSubView, MF_STRING, 1008, "Help\tF1 / H");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubView, "View");

            // Bookmarks Menu
            HMENU hSubBM = CreatePopupMenu();
            AppendMenuA(hSubBM, MF_STRING, 1005, "Add Bookmark at Position");
            AppendMenuA(hSubBM, MF_STRING, 1006, "Jump to Saved Bookmark");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubBM, "Bookmarks");

            // Theme Menu
            HMENU hSubTheme = CreatePopupMenu();
            AppendMenuA(hSubTheme, MF_STRING, 1010, "Light Theme");
            AppendMenuA(hSubTheme, MF_STRING, 1011, "Dark Theme");
            AppendMenuA(hSubTheme, MF_STRING, 1012, "Sepia Theme");
            AppendMenuA(hSubTheme, MF_STRING, 1013, "High-Contrast Theme");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubTheme, "Themes");

            // Font Menu
            HMENU hSubFont = CreatePopupMenu();
            AppendMenuA(hSubFont, MF_STRING, 1020, "Font Size +");
            AppendMenuA(hSubFont, MF_STRING, 1021, "Font Size -");
            AppendMenuA(hSubFont, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hSubFont, MF_STRING, 1022, "Georgia (Serif)");
            AppendMenuA(hSubFont, MF_STRING, 1023, "Segoe UI (Sans-Serif)");
            AppendMenuA(hSubFont, MF_STRING, 1024, "Consolas (Monospace)");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubFont, "Font");

            SetMenu(hwnd, hMenu);

            // Tab Control
            g_hTabCtrl = CreateWindowExA(0, WC_TABCONTROLA, "", 
                WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_TABS | TCS_FOCUSNEVER, 
                0, 0, 800, 28, hwnd, (HMENU)2001, GetModuleHandleA(NULL), NULL);

            // Edit Control
            hEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", 
                WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL | ES_WANTRETURN | ES_READONLY, 
                0, 28, 800, 500, hwnd, NULL, NULL, NULL);

            SendMessageA(hEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(16, 16));

            UpdateFont(hwnd);
            SetTheme(hwnd, RGB(250, 250, 250), RGB(30, 30, 30));

            // Create Initial Tab
            AddNewTab(hwnd, "Welcome", "Welcome to KRead Native E-Reader.\r\n\r\nFeatures:\r\n- Multi-Tab Reading Sessions (Ctrl+T for New Tab, Ctrl+W to Close)\r\n- File -> Open to load documents into tabs\r\n- Bookmarks & Reading Statistics Engine\r\n- Themes and Font Customization\r\n\r\nPress F1 or 'H' for Help.");
            break;
        }
        case WM_SIZE: {
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            int tabHeight = 28;
            if (g_hTabCtrl) {
                MoveWindow(g_hTabCtrl, 0, 0, w, tabHeight, TRUE);
            }
            if (hEdit) {
                MoveWindow(hEdit, 0, tabHeight, w, h - tabHeight, TRUE);
            }
            break;
        }
        case WM_NOTIFY: {
            LPNMHDR pnm = (LPNMHDR)lParam;
            if (pnm->hwndFrom == g_hTabCtrl && pnm->code == TCN_SELCHANGE) {
                int sel = TabCtrl_GetCurSel(g_hTabCtrl);
                if (sel != g_ActiveTab) {
                    SwitchToTab(hwnd, sel);
                }
            }
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == 1001) OpenFileAndLoad(hwnd, FALSE);
            if (id == 1009) OpenFileAndLoad(hwnd, TRUE);
            if (id == 1002) PostQuitMessage(0);

            // Tab Commands
            if (id == 1030) AddNewTab(hwnd, "New Tab", "");
            if (id == 1031) CloseCurrentTab(hwnd);
            if (id == 1032 && g_NumTabs > 1) SwitchToTab(hwnd, (g_ActiveTab + 1) % g_NumTabs);
            if (id == 1033 && g_NumTabs > 1) SwitchToTab(hwnd, (g_ActiveTab - 1 + g_NumTabs) % g_NumTabs);
            
            // Reading Statistics
            if (id == 1003 || id == 1007) {
                int len = GetWindowTextLengthA(hEdit);
                if (len > 0) {
                    char* text = (char*)VirtualAlloc(NULL, len + 1, MEM_COMMIT, PAGE_READWRITE);
                    if (text) {
                        GetWindowTextA(hEdit, text, len + 1);
                        int lines = 1, words = 0, chars = len;
                        int inWord = 0;
                        for (int i = 0; i < len; i++) {
                            if (text[i] == '\n') lines++;
                            if (text[i] == ' ' || text[i] == '\n' || text[i] == '\r' || text[i] == '\t') {
                                inWord = 0;
                            } else {
                                if (!inWord) {
                                    words++;
                                    inWord = 1;
                                }
                            }
                        }
                        int estMins = (words + 199) / 200; // 200 WPM
                        char msg[512];
                        wsprintfA(msg, "--- KREAD STATISTICS ENGINE ---\n\nActive Tab:\t\t%s (%d of %d)\nTotal Characters:\t%d\nTotal Words:\t\t%d\nTotal Lines:\t\t%d\nEst. Reading Speed:\t200 WPM\nEst. Time Remaining:\t%d minutes", 
                            g_Tabs[g_ActiveTab].szTitle, g_ActiveTab + 1, g_NumTabs, chars, words, lines, estMins);
                        
                        if (id == 1007) {
                            SaveStatsExport(hwnd, msg);
                        } else {
                            MessageBoxA(hwnd, msg, "KRead Statistics Engine", MB_OK | MB_ICONINFORMATION);
                        }
                        VirtualFree(text, 0, MEM_RELEASE);
                    }
                } else {
                    MessageBoxA(hwnd, "Current tab document is empty.", "KRead Statistics Engine", MB_OK | MB_ICONINFORMATION);
                }
            }

            // Find Text
            if (id == 1004) {
                PerformSearch(hwnd);
            }

            // Help
            if (id == 1008) {
                MessageBoxA(hwnd, "KRead Help:\n\n- Tabs -> New Tab (Ctrl+T): Open a new document tab\n- Tabs -> Close Tab (Ctrl+W): Close the current tab\n- File -> Open / Open in New Tab: Load text documents\n- View -> Statistics: Check reading progress & WPM\n- Bookmarks: Save & jump to position per tab\n- Themes/Fonts: Customize appearance", "KRead Help", MB_OK | MB_ICONINFORMATION);
            }

            // Bookmarks
            if (id == 1005) {
                DWORD start = 0, end = 0;
                SendMessageA(hEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
                g_Tabs[g_ActiveTab].dwBookmark = start;
                MessageBoxA(hwnd, "Bookmark saved for current tab at text position!", "KRead Bookmarks", MB_OK | MB_ICONINFORMATION);
            }
            if (id == 1006) {
                DWORD bm = g_Tabs[g_ActiveTab].dwBookmark;
                SendMessageA(hEdit, EM_SETSEL, bm, bm);
                SendMessageA(hEdit, EM_SCROLLCARET, 0, 0);
            }

            // Themes
            if (id == 1010) SetTheme(hwnd, RGB(250, 250, 250), RGB(30, 30, 30));     // Light
            if (id == 1011) SetTheme(hwnd, RGB(24, 24, 28), RGB(228, 228, 231));      // Dark
            if (id == 1012) SetTheme(hwnd, RGB(251, 240, 217), RGB(67, 52, 34));      // Sepia
            if (id == 1013) SetTheme(hwnd, RGB(0, 0, 0), RGB(0, 255, 102));          // Contrast

            // Fonts
            if (id == 1020) { currentFontSize = min(currentFontSize + 2, 42); UpdateFont(hwnd); }
            if (id == 1021) { currentFontSize = max(currentFontSize - 2, 12); UpdateFont(hwnd); }
            if (id == 1022) { lstrcpyA(currentFontFace, "Georgia"); UpdateFont(hwnd); }
            if (id == 1023) { lstrcpyA(currentFontFace, "Segoe UI"); UpdateFont(hwnd); }
            if (id == 1024) { lstrcpyA(currentFontFace, "Consolas"); UpdateFont(hwnd); }

            break;
        }
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC: {
            if ((HWND)lParam == hEdit) {
                HDC hdc = (HDC)wParam;
                SetBkColor(hdc, g_bgColor);
                SetTextColor(hdc, g_textColor);
                return (LRESULT)hBrush;
            }
            break;
        }
        case WM_DESTROY:
            SaveActiveTabState();
            for (int i = 0; i < g_NumTabs; i++) {
                if (g_Tabs[i].pszText) {
                    VirtualFree(g_Tabs[i].pszText, 0, MEM_RELEASE);
                    g_Tabs[i].pszText = NULL;
                }
            }
            if (hFont) DeleteObject(hFont);
            if (hBrush) DeleteObject(hBrush);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    InitCommonControls();
    SetProcessDPIAware();
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KReadClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClassA(&wc);
    RECT rc = { 0, 0, 850, 620 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, TRUE);
    HWND hwnd = CreateWindowExA(0, "KReadClass", "KRead Native E-Reader - Multi-Tab Edition (F1 for Help)", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            BOOL ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            BOOL shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;

            if (ctrl && (msg.wParam == 'T' || msg.wParam == 't')) {
                SendMessageA(hwnd, WM_COMMAND, 1030, 0);
                continue;
            }
            if (ctrl && (msg.wParam == 'W' || msg.wParam == 'w')) {
                SendMessageA(hwnd, WM_COMMAND, 1031, 0);
                continue;
            }
            if (ctrl && (msg.wParam == 'O' || msg.wParam == 'o')) {
                SendMessageA(hwnd, WM_COMMAND, 1009, 0);
                continue;
            }
            if (ctrl && msg.wParam == VK_TAB) {
                if (shift) {
                    SendMessageA(hwnd, WM_COMMAND, 1033, 0);
                } else {
                    SendMessageA(hwnd, WM_COMMAND, 1032, 0);
                }
                continue;
            }
            if (msg.wParam == VK_F1 || msg.wParam == 'H') {
                if (!ctrl && !shift) {
                    SendMessageA(hwnd, WM_COMMAND, 1008, 0);
                    continue;
                }
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
