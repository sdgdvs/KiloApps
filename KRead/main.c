#include <windows.h>
#include <commdlg.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

// Global Theme and Font state
HFONT hFont = NULL;
HBRUSH hBrush = NULL;
HWND hEdit = NULL;

int currentFontSize = 18;
char currentFontFace[32] = "Georgia";

COLORREF g_bgColor = RGB(250, 250, 250);
COLORREF g_textColor = RGB(30, 30, 30);

DWORD g_bookmarkCharIdx = 0;
char g_lastSearchQuery[128] = {0};

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
    InvalidateRect(hEdit, NULL, TRUE);
    UpdateWindow(hEdit);
}

void OpenFileAndLoad(HWND hwnd) {
    OPENFILENAMEA ofn;
    char szFile[260] = {0};

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Text Files\0*.txt\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(ofn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD dwFileSize = GetFileSize(hFile, NULL);
            if (dwFileSize != INVALID_FILE_SIZE) {
                char* pszFileText = (char*)VirtualAlloc(NULL, dwFileSize + 1, MEM_COMMIT, PAGE_READWRITE);
                if (pszFileText) {
                    DWORD dwRead;
                    if (ReadFile(hFile, pszFileText, dwFileSize, &dwRead, NULL)) {
                        pszFileText[dwRead] = 0;
                        SetWindowTextA(hEdit, pszFileText);
                        g_bookmarkCharIdx = 0;
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
            HMENU hMenu = CreateMenu();
            
            // File Menu
            HMENU hSubFile = CreatePopupMenu();
            AppendMenuA(hSubFile, MF_STRING, 1001, "Open File...");
            AppendMenuA(hSubFile, MF_STRING, 1007, "Export Statistics...");
            AppendMenuA(hSubFile, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hSubFile, MF_STRING, 1002, "Exit");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubFile, "File");

            // View Menu
            HMENU hSubView = CreatePopupMenu();
            AppendMenuA(hSubView, MF_STRING, 1003, "Reading Statistics Engine");
            AppendMenuA(hSubView, MF_STRING, 1004, "Find Text...");
            AppendMenuA(hSubView, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hSubView, MF_STRING, 1008, "Help");
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

            hEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Welcome to KRead Native E-Reader.\r\nUse File -> Open to load a document.\r\nPress F1 or 'H' for Help.", 
                WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL | ES_WANTRETURN | ES_READONLY, 
                0, 0, 0, 0, hwnd, NULL, NULL, NULL);

            SendMessageA(hEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(16, 16));

            UpdateFont(hwnd);
            SetTheme(hwnd, RGB(250, 250, 250), RGB(30, 30, 30));
            break;
        }
        case WM_SIZE: {
            MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == 1001) OpenFileAndLoad(hwnd);
            if (id == 1002) PostQuitMessage(0);
            
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
                        wsprintfA(msg, "--- KREAD STATISTICS ENGINE ---\n\nTotal Characters:\t%d\nTotal Words:\t\t%d\nTotal Lines:\t\t%d\nEst. Reading Speed:\t200 WPM\nEst. Time Remaining:\t%d minutes", 
                            chars, words, lines, estMins);
                        
                        if (id == 1007) {
                            SaveStatsExport(hwnd, msg);
                        } else {
                            MessageBoxA(hwnd, msg, "KRead Statistics Engine", MB_OK | MB_ICONINFORMATION);
                        }
                        VirtualFree(text, 0, MEM_RELEASE);
                    }
                } else {
                    MessageBoxA(hwnd, "Document is empty.", "KRead Statistics Engine", MB_OK | MB_ICONINFORMATION);
                }
            }

            // Find Text
            if (id == 1004) {
                PerformSearch(hwnd);
            }

            // Help
            if (id == 1008) {
                MessageBoxA(hwnd, "KRead Help:\n\n- File -> Open: Load a text file\n- View -> Find Text: Search for text\n- View -> Statistics: Check reading progress\n- Bookmarks: Save your current position\n- Themes/Fonts: Customize the reader appearance", "KRead Help", MB_OK | MB_ICONINFORMATION);
            }

            // Bookmarks
            if (id == 1005) {
                DWORD start = 0, end = 0;
                SendMessageA(hEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
                g_bookmarkCharIdx = start;
                MessageBoxA(hwnd, "Bookmark saved at current text selection position!", "KRead Bookmarks", MB_OK | MB_ICONINFORMATION);
            }
            if (id == 1006) {
                SendMessageA(hEdit, EM_SETSEL, g_bookmarkCharIdx, g_bookmarkCharIdx);
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
            if (hFont) DeleteObject(hFont);
            if (hBrush) DeleteObject(hBrush);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    SetProcessDPIAware();
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KReadClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClassA(&wc);
    RECT rc = { 0, 0, 800, 600 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, TRUE);
    HWND hwnd = CreateWindowExA(0, "KReadClass", "KRead Native E-Reader - Press F1 or H for Help", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN && (msg.wParam == VK_F1 || msg.wParam == 'H')) {
            SendMessageA(hwnd, WM_COMMAND, 1008, 0);
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
