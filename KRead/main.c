#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

void* __cdecl memcpy(void* dst, const void* src, size_t sz) {
    char* d = (char*)dst;
    const char* s = (const char*)src;
    while (sz--) *d++ = *s++;
    return dst;
}
#pragma function(memcpy)

#define MAX_TABS 12
#define TIMER_STATUS_RESET 101

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
static HWND g_hStatus = NULL;

static HFONT hFont = NULL;
static HBRUSH hBrush = NULL;

static int currentFontSize = 18;
static char currentFontFace[32] = "Georgia";

static COLORREF g_bgColor = RGB(250, 250, 250);
static COLORREF g_textColor = RGB(30, 30, 30);

static FINDREPLACEA g_fr;
static HWND g_hFindDlg = NULL;
static char g_szFindWhat[128] = {0};
static UINT g_uFindReplaceMsg = 0;

static char g_szStatusToast[128] = {0};
static BOOL g_bToastActive = FALSE;

static const char* g_SampleCyberTitle = "Cyberpunk Manifesto.txt";
static const char* g_SampleCyberText = 
    "THE CYBERPUNK MANIFESTO\r\n"
    "=======================\r\n\r\n"
    "We are the electronic mind.\r\n"
    "We create our own worlds with logic, silicon, and keystrokes.\r\n\r\n"
    "We exist without skin color, without nationality, without religious bias.\r\n"
    "Our only crime is curiosity, exploration, and the relentless pursuit of knowledge.\r\n\r\n"
    "You build borders, fences, and firewalls.\r\n"
    "We explore every gap, understand the underlying protocol, and bridge minds across oceans.\r\n\r\n"
    "Information wants to be free.\r\n"
    "Systems are meant to be understood.\r\n"
    "Ideas cannot be locked in vaults.\r\n\r\n"
    "Welcome to the new frontier.\r\n"
    "Welcome to the KiloOS cyberspace.";

static const char* g_SampleTimeTitle = "The Time Machine Excerpt.txt";
static const char* g_SampleTimeText = 
    "THE TIME MACHINE (Excerpt)\r\n"
    "by H.G. Wells\r\n"
    "==========================\r\n\r\n"
    "The Time Traveller was expounding a recondite matter to us. His grey eyes\r\n"
    "shone and twinkled, and his usually pale face was flushed and animated.\r\n"
    "The fire burnt brightly, and the soft radiance of the incandescent lights\r\n"
    "in the lilies of silver caught the bubbles that flashed and passed in our glasses.\r\n\r\n"
    "\"You must follow me carefully. I shall have to controvert one or two ideas\r\n"
    "that are almost universally accepted. The geometry, for instance, that you\r\n"
    "were taught in school is founded on a misconception.\"\r\n\r\n"
    "\"Is not that rather a large thing to expect us to begin upon?\" said Filby,\r\n"
    "an argumentative person with red hair.\r\n\r\n"
    "\"I do not mean to ask you to accept anything without reasonable ground for it.\r\n"
    "You will soon admit as much as I need from you. You know of course that a\r\n"
    "mathematical line, a line of thickness nil, has no real existence. Nor has a\r\n"
    "mathematical plane. These things are mere abstractions.\"\r\n\r\n"
    "\"That is all right,\" said the Psychologist.\r\n\r\n"
    "\"Nor, having only length, breadth, and thickness, can a cube have a real existence.\"\r\n\r\n"
    "\"There I object,\" said Filby. \"Of course a solid body may exist. All real things—\"\r\n\r\n"
    "\"So most people think. But wait a moment. Can an instantaneous cube exist?\"\r\n"
    "\"Don't follow you,\" said Filby.\r\n\r\n"
    "\"Can a cube that does not exist for any time at all, have a real existence?\"";

static const char* g_SampleKiloTitle = "KiloOS Architecture Guide.md";
static const char* g_SampleKiloText = 
    "# KiloOS System Architecture & Usage Guide\r\n\r\n"
    "## Overview\r\n"
    "KiloOS is a lightweight, high-performance web-native and retro-compatible\r\n"
    "desktop operating environment designed for absolute speed and modularity.\r\n\r\n"
    "## Core Principles\r\n"
    "- Sub-999KB size budget for every native and web application.\r\n"
    "- Multi-agent coordination with continuous integration.\r\n"
    "- Instant load times and zero unnecessary runtime bloat.\r\n\r\n"
    "## Built-in Productivity Apps\r\n"
    "1. KRead: Multi-tab document e-reader with reading statistics & bookmarks.\r\n"
    "2. KPad: Monospace text editor with syntax highlighting.\r\n"
    "3. KJournal: Encrypted daily diary and thought logger.\r\n"
    "4. KBase: Desktop database and record organizer.\r\n"
    "5. KTerm: Cybernetic console emulator.\r\n\r\n"
    "## Shortcuts Quick Reference\r\n"
    "- Ctrl+T: New Tab in supported apps\r\n"
    "- Ctrl+W: Close active Tab\r\n"
    "- Ctrl+F: In-document Search\r\n"
    "- F1 / H: Contextual Help";

void UpdateStatusBar() {
    if (!g_hStatus) return;
    RECT rc;
    GetClientRect(g_hMainWnd, &rc);
    int width = rc.right - rc.left;
    int parts[3];
    parts[0] = (width > 600) ? 240 : 160;
    parts[1] = (width > 600) ? 460 : 320;
    parts[2] = -1;
    SendMessageA(g_hStatus, SB_SETPARTS, 3, (LPARAM)parts);

    char szPart0[128];
    if (g_NumTabs > 0 && g_ActiveTab >= 0 && g_ActiveTab < g_NumTabs) {
        wsprintfA(szPart0, "Tab %d/%d: %s", g_ActiveTab + 1, g_NumTabs, g_Tabs[g_ActiveTab].szTitle);
    } else {
        lstrcpyA(szPart0, "Ready");
    }
    SendMessageA(g_hStatus, SB_SETTEXTA, 0, (LPARAM)szPart0);

    char szPart1[128];
    int docLen = hEdit ? GetWindowTextLengthA(hEdit) : 0;
    DWORD selStart = 0;
    if (hEdit) SendMessageA(hEdit, EM_GETSEL, (WPARAM)&selStart, (LPARAM)NULL);
    int lineIdx = (int)SendMessageA(hEdit, EM_LINEFROMCHAR, selStart, 0) + 1;
    int lineStart = (int)SendMessageA(hEdit, EM_LINEINDEX, lineIdx - 1, 0);
    int colIdx = (int)selStart - lineStart + 1;
    wsprintfA(szPart1, "Ln %d, Col %d | %d ch | %dpt", lineIdx, colIdx, docLen, currentFontSize);
    SendMessageA(g_hStatus, SB_SETTEXTA, 1, (LPARAM)szPart1);

    if (g_bToastActive && g_szStatusToast[0]) {
        SendMessageA(g_hStatus, SB_SETTEXTA, 2, (LPARAM)g_szStatusToast);
    } else {
        SendMessageA(g_hStatus, SB_SETTEXTA, 2, (LPARAM)"F1: Help | Ctrl+O: Open | Ctrl+T: Tab | Ctrl+B: Bookmark | Ctrl+S: Stats");
    }
}

void ShowNativeStatus(const char* text) {
    if (!g_hStatus || !text) return;
    lstrcpynA(g_szStatusToast, text, sizeof(g_szStatusToast));
    g_bToastActive = TRUE;
    UpdateStatusBar();
    SetTimer(g_hMainWnd, TIMER_STATUS_RESET, 3200, NULL);
}

void UpdateWindowTitle(HWND hwnd) {
    if (!hwnd) return;
    char szTitle[256];
    if (g_NumTabs > 0 && g_ActiveTab >= 0 && g_ActiveTab < g_NumTabs) {
        wsprintfA(szTitle, "KRead Native - [%s] (%d/%d) - Press F1 or H for Help", 
            g_Tabs[g_ActiveTab].szTitle, g_ActiveTab + 1, g_NumTabs);
    } else {
        lstrcpyA(szTitle, "KRead Native E-Reader - Press F1 or H for Help");
    }
    SetWindowTextA(hwnd, szTitle);
}

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
    UpdateWindowTitle(g_hMainWnd);
    UpdateStatusBar();
}

void SwitchToTab(HWND hwnd, int newIndex) {
    if (newIndex < 0 || newIndex >= g_NumTabs || newIndex == g_ActiveTab) return;
    SaveActiveTabState();
    LoadTabState(newIndex);
}

void AddNewTab(HWND hwnd, const char* title, const char* initialText) {
    if (g_NumTabs >= MAX_TABS) {
        ShowNativeStatus("Maximum number of tabs reached (12).");
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
        UpdateWindowTitle(hwnd);
        UpdateStatusBar();
        ShowNativeStatus("Document cleared [Ctrl+Del]");
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

void LoadFileFromPath(HWND hwnd, const char* szFilePath, BOOL inNewTab) {
    HANDLE hFile = CreateFileA(szFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    
    DWORD dwFileSize = GetFileSize(hFile, NULL);
    if (dwFileSize == INVALID_FILE_SIZE || dwFileSize == 0) {
        CloseHandle(hFile);
        return;
    }

    char* pszRaw = (char*)VirtualAlloc(NULL, dwFileSize + 1, MEM_COMMIT, PAGE_READWRITE);
    if (!pszRaw) {
        CloseHandle(hFile);
        return;
    }

    DWORD dwRead = 0;
    if (ReadFile(hFile, pszRaw, dwFileSize, &dwRead, NULL)) {
        pszRaw[dwRead] = 0;

        // Check if Unix newlines (\n without \r) need normalization to \r\n
        int newlineCount = 0;
        for (DWORD i = 0; i < dwRead; i++) {
            if (pszRaw[i] == '\n' && (i == 0 || pszRaw[i - 1] != '\r')) {
                newlineCount++;
            }
        }

        char* pszNormalized = pszRaw;
        if (newlineCount > 0) {
            char* pConv = (char*)VirtualAlloc(NULL, dwRead + newlineCount + 1, MEM_COMMIT, PAGE_READWRITE);
            if (pConv) {
                DWORD dst = 0;
                for (DWORD src = 0; src < dwRead; src++) {
                    if (pszRaw[src] == '\n' && (src == 0 || pszRaw[src - 1] != '\r')) {
                        pConv[dst++] = '\r';
                    }
                    pConv[dst++] = pszRaw[src];
                }
                pConv[dst] = 0;
                pszNormalized = pConv;
            }
        }

        char fname[64];
        ExtractFileName(szFilePath, fname, sizeof(fname));

        if (inNewTab || g_NumTabs == 0) {
            AddNewTab(hwnd, fname, pszNormalized);
        } else {
            SetWindowTextA(hEdit, pszNormalized);
            lstrcpynA(g_Tabs[g_ActiveTab].szTitle, fname, 60);
            g_Tabs[g_ActiveTab].dwBookmark = 0;

            if (g_Tabs[g_ActiveTab].pszText) {
                VirtualFree(g_Tabs[g_ActiveTab].pszText, 0, MEM_RELEASE);
                g_Tabs[g_ActiveTab].pszText = NULL;
                g_Tabs[g_ActiveTab].dwTextLen = 0;
            }
            int normLen = lstrlenA(pszNormalized);
            char* buf = (char*)VirtualAlloc(NULL, normLen + 1, MEM_COMMIT, PAGE_READWRITE);
            if (buf) {
                lstrcpyA(buf, pszNormalized);
                g_Tabs[g_ActiveTab].pszText = buf;
                g_Tabs[g_ActiveTab].dwTextLen = normLen;
            }

            TCITEMA tie;
            tie.mask = TCIF_TEXT;
            tie.pszText = g_Tabs[g_ActiveTab].szTitle;
            TabCtrl_SetItem(g_hTabCtrl, g_ActiveTab, &tie);
            UpdateWindowTitle(hwnd);
        }

        if (pszNormalized != pszRaw) {
            VirtualFree(pszNormalized, 0, MEM_RELEASE);
        }
    }
    VirtualFree(pszRaw, 0, MEM_RELEASE);
    CloseHandle(hFile);
}

void OpenFileAndLoad(HWND hwnd, BOOL inNewTab) {
    OPENFILENAMEA ofn;
    char szFile[MAX_PATH] = {0};

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Text & Document Files\0*.txt;*.md;*.csv;*.log;*.json\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        LoadFileFromPath(hwnd, ofn.lpstrFile, inNewTab);
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

void PerformSearchNext(HWND hwnd, const char* query, BOOL down, BOOL matchCase) {
    int len = GetWindowTextLengthA(hEdit);
    if (len <= 0 || !query || !query[0]) {
        MessageBoxA(hwnd, "No text to search.", "Find Text", MB_OK | MB_ICONINFORMATION);
        return;
    }

    char* text = (char*)VirtualAlloc(NULL, len + 1, MEM_COMMIT, PAGE_READWRITE);
    if (!text) return;
    GetWindowTextA(hEdit, text, len + 1);

    int qLen = lstrlenA(query);
    DWORD selStart = 0, selEnd = 0;
    SendMessageA(hEdit, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);

    int foundIdx = -1;

    if (down) {
        for (int i = (int)selEnd; i <= len - qLen; i++) {
            int match = 1;
            for (int j = 0; j < qLen; j++) {
                char c1 = text[i + j];
                char c2 = query[j];
                if (!matchCase) {
                    if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
                    if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
                }
                if (c1 != c2) { match = 0; break; }
            }
            if (match) { foundIdx = i; break; }
        }
        if (foundIdx == -1 && selEnd > 0) {
            for (int i = 0; i < (int)selEnd && i <= len - qLen; i++) {
                int match = 1;
                for (int j = 0; j < qLen; j++) {
                    char c1 = text[i + j];
                    char c2 = query[j];
                    if (!matchCase) {
                        if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
                        if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
                    }
                    if (c1 != c2) { match = 0; break; }
                }
                if (match) { foundIdx = i; break; }
            }
        }
    } else {
        int startPos = (int)selStart - 1;
        if (startPos > len - qLen) startPos = len - qLen;
        for (int i = startPos; i >= 0; i--) {
            int match = 1;
            for (int j = 0; j < qLen; j++) {
                char c1 = text[i + j];
                char c2 = query[j];
                if (!matchCase) {
                    if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
                    if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
                }
                if (c1 != c2) { match = 0; break; }
            }
            if (match) { foundIdx = i; break; }
        }
        if (foundIdx == -1 && selStart < (DWORD)len) {
            for (int i = len - qLen; i >= (int)selStart; i--) {
                int match = 1;
                for (int j = 0; j < qLen; j++) {
                    char c1 = text[i + j];
                    char c2 = query[j];
                    if (!matchCase) {
                        if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
                        if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
                    }
                    if (c1 != c2) { match = 0; break; }
                }
                if (match) { foundIdx = i; break; }
            }
        }
    }

    if (foundIdx != -1) {
        SendMessageA(hEdit, EM_SETSEL, foundIdx, foundIdx + qLen);
        SendMessageA(hEdit, EM_SCROLLCARET, 0, 0);
    } else {
        char notFoundMsg[256];
        wsprintfA(notFoundMsg, "Cannot find \"%s\".", query);
        MessageBoxA(hwnd, notFoundMsg, "Find Text", MB_OK | MB_ICONINFORMATION);
    }

    VirtualFree(text, 0, MEM_RELEASE);
}

void OpenFindDialog(HWND hwnd) {
    if (g_hFindDlg) {
        SetFocus(g_hFindDlg);
        return;
    }
    DWORD selStart = 0, selEnd = 0;
    SendMessageA(hEdit, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);
    if (selEnd > selStart && (selEnd - selStart) < sizeof(g_szFindWhat)) {
        int selLen = (int)(selEnd - selStart);
        int docLen = GetWindowTextLengthA(hEdit);
        if (docLen > 0) {
            char* pDoc = (char*)VirtualAlloc(NULL, docLen + 1, MEM_COMMIT, PAGE_READWRITE);
            if (pDoc) {
                GetWindowTextA(hEdit, pDoc, docLen + 1);
                for (int i = 0; i < selLen; i++) {
                    g_szFindWhat[i] = pDoc[selStart + i];
                }
                g_szFindWhat[selLen] = 0;
                VirtualFree(pDoc, 0, MEM_RELEASE);
            }
        }
    }

    ZeroMemory(&g_fr, sizeof(g_fr));
    g_fr.lStructSize = sizeof(g_fr);
    g_fr.hwndOwner = hwnd;
    g_fr.lpstrFindWhat = g_szFindWhat;
    g_fr.wFindWhatLen = sizeof(g_szFindWhat);
    g_fr.Flags = FR_DOWN;
    g_hFindDlg = FindTextA(&g_fr);
}

void CopyTextToClipboard(HWND hwnd, const char* text, int len) {
    if (!text || len <= 0) {
        ShowNativeStatus("No text to copy.");
        return;
    }
    if (!OpenClipboard(hwnd)) return;
    EmptyClipboard();
    HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, len + 1);
    if (hGlob) {
        char* pDst = (char*)GlobalLock(hGlob);
        if (pDst) {
            for (int i = 0; i < len; i++) pDst[i] = text[i];
            pDst[len] = 0;
            GlobalUnlock(hGlob);
            SetClipboardData(CF_TEXT, hGlob);
            ShowNativeStatus("Text copied to clipboard! [Ctrl+C]");
        }
    }
    CloseClipboard();
}

void ShowHelpDialog(HWND hwnd) {
    const char* szHelp = 
        "KRead Native E-Reader - User & Keyboard Guide\n"
        "===============================================\n\n"
        "KEYBOARD SHORTCUTS:\n"
        "  F1 or H               : Show this Help Guide\n"
        "  Ctrl + O              : Open File in Active Tab\n"
        "  Ctrl + Shift + O      : Open File in New Tab\n"
        "  Ctrl + T              : Open New Reading Tab\n"
        "  Ctrl + W              : Close Current Tab\n"
        "  Ctrl + Tab            : Switch to Next Tab\n"
        "  Ctrl + Shift + Tab    : Switch to Previous Tab\n"
        "  Ctrl + 1 .. 9         : Jump Directly to Tab 1-9\n"
        "  Ctrl + A              : Select All Text in Active Tab\n"
        "  Ctrl + C              : Copy Selected Text\n"
        "  Ctrl + Shift + C      : Copy All Document Text\n"
        "  Ctrl + F              : Find Text (Windows Search Dialog)\n"
        "  Ctrl + B              : Save Bookmark at Position\n"
        "  Ctrl + J              : Jump to Saved Bookmark\n"
        "  Ctrl + S              : Reading Statistics Engine & WPM\n"
        "  Ctrl + '+' / '-'      : Increase / Decrease Font Size\n"
        "  Ctrl + 0              : Reset Font Size to Default (18pt)\n"
        "  Alt + 1 .. 4          : Switch Theme (Light/Dark/Sepia/Contrast)\n"
        "  Ctrl + Del            : Clear Document Content\n\n"
        "FEATURES:\n"
        "  * Multi-Tab Sessions (up to 12 concurrent docs)\n"
        "  * Quick Document Starter Presets (Cyberpunk, Time Machine, KiloOS)\n"
        "  * Real-time Interactive Status Bar with Reading Position & Statistics\n"
        "  * Non-blocking Toast Status System (eliminates modal alert popups)\n"
        "  * Automatic Unix Newline Normalization (\\n to \\r\\n)\n"
        "  * Per-tab Independent Reading Cursor & Bookmarks\n"
        "  * Reading Speed & Time Remaining Estimator\n"
        "  * High-DPI Crisp Font Scaling & Themes (Light/Dark/Sepia/Contrast)";
    MessageBoxA(hwnd, szHelp, "KRead Help (F1 / H)", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == g_uFindReplaceMsg && g_uFindReplaceMsg != 0) {
        LPFINDREPLACEA lpfr = (LPFINDREPLACEA)lParam;
        if (lpfr->Flags & FR_DIALOGTERM) {
            g_hFindDlg = NULL;
            return 0;
        }
        if (lpfr->Flags & FR_FINDNEXT) {
            PerformSearchNext(hwnd, lpfr->lpstrFindWhat, (lpfr->Flags & FR_DOWN) != 0, (lpfr->Flags & FR_MATCHCASE) != 0);
            return 0;
        }
    }

    switch (msg) {
        case WM_CREATE: {
            g_hMainWnd = hwnd;
            g_uFindReplaceMsg = RegisterWindowMessageA(FINDMSGSTRINGA);
            DragAcceptFiles(hwnd, TRUE);

            HMENU hMenu = CreateMenu();
            
            // File Menu
            HMENU hSubFile = CreatePopupMenu();
            AppendMenuA(hSubFile, MF_STRING, 1001, "Open File...\tCtrl+O");
            AppendMenuA(hSubFile, MF_STRING, 1009, "Open File in New Tab...\tCtrl+Shift+O");
            
            HMENU hSubSamples = CreatePopupMenu();
            AppendMenuA(hSubSamples, MF_STRING, 1040, "🚀 Cyberpunk Manifesto");
            AppendMenuA(hSubSamples, MF_STRING, 1041, "⏳ The Time Machine (Excerpt)");
            AppendMenuA(hSubSamples, MF_STRING, 1042, "💻 KiloOS Architecture Guide");
            AppendMenuA(hSubFile, MF_POPUP, (UINT_PTR)hSubSamples, "Load Sample Document");

            AppendMenuA(hSubFile, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hSubFile, MF_STRING, 1000, "Clear Document\tCtrl+Del");
            AppendMenuA(hSubFile, MF_STRING, 1007, "Export Statistics...");
            AppendMenuA(hSubFile, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hSubFile, MF_STRING, 1002, "Exit\tAlt+F4");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubFile, "File");

            // Edit Menu
            HMENU hSubEdit = CreatePopupMenu();
            AppendMenuA(hSubEdit, MF_STRING, 1050, "Copy Selection\tCtrl+C");
            AppendMenuA(hSubEdit, MF_STRING, 1051, "Copy All Text\tCtrl+Shift+C");
            AppendMenuA(hSubEdit, MF_STRING, 1052, "Select All\tCtrl+A");
            AppendMenuA(hSubEdit, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hSubEdit, MF_STRING, 1004, "Find Text...\tCtrl+F");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubEdit, "Edit");

            // Tabs Menu
            HMENU hSubTabs = CreatePopupMenu();
            AppendMenuA(hSubTabs, MF_STRING, 1030, "New Tab\tCtrl+T");
            AppendMenuA(hSubTabs, MF_STRING, 1031, "Close Current Tab\tCtrl+W");
            AppendMenuA(hSubTabs, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hSubTabs, MF_STRING, 1032, "Next Tab\tCtrl+Tab");
            AppendMenuA(hSubTabs, MF_STRING, 1033, "Previous Tab\tCtrl+Shift+Tab");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubTabs, "Tabs");

            // Bookmarks Menu
            HMENU hSubBM = CreatePopupMenu();
            AppendMenuA(hSubBM, MF_STRING, 1005, "Add Bookmark at Position\tCtrl+B");
            AppendMenuA(hSubBM, MF_STRING, 1006, "Jump to Saved Bookmark\tCtrl+J");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubBM, "Bookmarks");

            // Theme Menu
            HMENU hSubTheme = CreatePopupMenu();
            AppendMenuA(hSubTheme, MF_STRING, 1010, "☀️ Light Theme\tAlt+1");
            AppendMenuA(hSubTheme, MF_STRING, 1011, "🌙 Dark Theme\tAlt+2");
            AppendMenuA(hSubTheme, MF_STRING, 1012, "📜 Sepia Theme\tAlt+3");
            AppendMenuA(hSubTheme, MF_STRING, 1013, "⚡ High-Contrast Theme\tAlt+4");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubTheme, "Themes");

            // Font Menu
            HMENU hSubFont = CreatePopupMenu();
            AppendMenuA(hSubFont, MF_STRING, 1020, "Font Size + (Ctrl++)");
            AppendMenuA(hSubFont, MF_STRING, 1021, "Font Size - (Ctrl+-)");
            AppendMenuA(hSubFont, MF_STRING, 1025, "Reset Font Size (18pt)\tCtrl+0");
            AppendMenuA(hSubFont, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hSubFont, MF_STRING, 1022, "Georgia (Serif)");
            AppendMenuA(hSubFont, MF_STRING, 1023, "Segoe UI (Sans-Serif)");
            AppendMenuA(hSubFont, MF_STRING, 1024, "Consolas (Monospace)");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubFont, "Font");

            // View Menu
            HMENU hSubView = CreatePopupMenu();
            AppendMenuA(hSubView, MF_STRING, 1003, "Reading Statistics Engine\tCtrl+S");
            AppendMenuA(hSubView, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hSubView, MF_STRING, 1008, "Help Guide\tF1 / H");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubView, "View");

            SetMenu(hwnd, hMenu);

            HDC hdc = GetDC(hwnd);
            int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(hwnd, hdc);
            int tabHeight = MulDiv(28, dpi, 96);
            int statusHeight = MulDiv(22, dpi, 96);

            // Tab Control
            g_hTabCtrl = CreateWindowExA(0, WC_TABCONTROLA, "", 
                WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_TABS | TCS_FOCUSNEVER, 
                0, 0, 850, tabHeight, hwnd, (HMENU)2001, GetModuleHandleA(NULL), NULL);
            SendMessageA(g_hTabCtrl, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

            // Edit Control
            hEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", 
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL | ES_WANTRETURN | ES_READONLY, 
                0, tabHeight, 850, 520, hwnd, NULL, NULL, NULL);

            SendMessageA(hEdit, EM_SETLIMITTEXT, 0, 0);
            SendMessageA(hEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(16, 16));

            // Status Bar
            g_hStatus = CreateWindowExA(0, STATUSCLASSNAME, "", 
                WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 
                0, 0, 0, 0, hwnd, (HMENU)2002, GetModuleHandleA(NULL), NULL);

            UpdateFont(hwnd);
            SetTheme(hwnd, RGB(250, 250, 250), RGB(30, 30, 30));

            // Create Initial Tab
            AddNewTab(hwnd, "Welcome", 
                "====================================================\r\n"
                "  WELCOME TO KREAD NATIVE E-READER & DOCUMENT ENGINE\r\n"
                "====================================================\r\n\r\n"
                "Quick Start Guide:\r\n"
                "  * Press F1 or 'H' anytime for the comprehensive Help Guide.\r\n"
                "  * Press Ctrl+O to open a text document into the current tab.\r\n"
                "  * Press Ctrl+T to open a new tab session.\r\n"
                "  * Press Ctrl+1 through Ctrl+9 to quickly switch tabs.\r\n"
                "  * Press Ctrl+A to select all text in the active document.\r\n"
                "  * Press Ctrl+C to copy selected text to clipboard.\r\n"
                "  * Press Ctrl+Shift+C to copy entire document to clipboard.\r\n"
                "  * Press Ctrl+F to open the Windows Find Text dialog.\r\n"
                "  * Press Ctrl+B to save your reading bookmark.\r\n"
                "  * Press Ctrl+J to jump directly to saved bookmark.\r\n"
                "  * Press Ctrl+S to view real-time reading stats and estimated time.\r\n"
                "  * Use File -> Load Sample Document to explore classic texts.\r\n"
                "  * Drag and drop any text/document files directly into this window.\r\n"
                "  * Use Themes (Alt+1..4) & Font menus to customize your reader.\r\n\r\n"
                "Happy Reading!");

            ShowNativeStatus("Welcome to KRead! Press F1 for Help.");
            break;
        }
        case WM_DROPFILES: {
            HDROP hDrop = (HDROP)wParam;
            char szDropFile[MAX_PATH];
            UINT numFiles = DragQueryFileA(hDrop, 0xFFFFFFFF, NULL, 0);
            for (UINT i = 0; i < numFiles; i++) {
                if (DragQueryFileA(hDrop, i, szDropFile, sizeof(szDropFile))) {
                    LoadFileFromPath(hwnd, szDropFile, (i > 0 || g_NumTabs > 0));
                }
            }
            DragFinish(hDrop);
            break;
        }
        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, hBrush);
            return 1;
        }
        case WM_GETMINMAXINFO: {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = 480;
            lpMMI->ptMinTrackSize.y = 350;
            return 0;
        }
        case WM_SIZE: {
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            HDC hdc = GetDC(hwnd);
            int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(hwnd, hdc);
            int tabHeight = MulDiv(28, dpi, 96);
            int statusHeight = MulDiv(22, dpi, 96);

            if (g_hStatus) {
                SendMessageA(g_hStatus, WM_SIZE, 0, 0);
            }
            if (g_hTabCtrl) {
                MoveWindow(g_hTabCtrl, 0, 0, w, tabHeight, TRUE);
            }
            if (hEdit) {
                int editH = max(0, h - tabHeight - statusHeight);
                MoveWindow(hEdit, 0, tabHeight, w, editH, TRUE);
            }
            UpdateStatusBar();
            break;
        }
        case WM_TIMER: {
            if (wParam == TIMER_STATUS_RESET) {
                KillTimer(hwnd, TIMER_STATUS_RESET);
                g_bToastActive = FALSE;
                g_szStatusToast[0] = 0;
                UpdateStatusBar();
                return 0;
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

            // Samples
            if (id == 1040) {
                AddNewTab(hwnd, g_SampleCyberTitle, g_SampleCyberText);
                ShowNativeStatus("Loaded Cyberpunk Manifesto sample!");
            }
            if (id == 1041) {
                AddNewTab(hwnd, g_SampleTimeTitle, g_SampleTimeText);
                ShowNativeStatus("Loaded The Time Machine sample!");
            }
            if (id == 1042) {
                AddNewTab(hwnd, g_SampleKiloTitle, g_SampleKiloText);
                ShowNativeStatus("Loaded KiloOS Architecture Guide sample!");
            }

            // Clear
            if (id == 1000) {
                SetWindowTextA(hEdit, "");
                ShowNativeStatus("Document cleared [Ctrl+Del]");
                UpdateStatusBar();
            }

            // Edit & Clipboard
            if (id == 1050) {
                DWORD start = 0, end = 0;
                SendMessageA(hEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
                if (end > start) {
                    int len = (int)(end - start);
                    int totalLen = GetWindowTextLengthA(hEdit);
                    char* fullText = (char*)VirtualAlloc(NULL, totalLen + 1, MEM_COMMIT, PAGE_READWRITE);
                    if (fullText) {
                        GetWindowTextA(hEdit, fullText, totalLen + 1);
                        char* selText = (char*)VirtualAlloc(NULL, len + 1, MEM_COMMIT, PAGE_READWRITE);
                        if (selText) {
                            for (int i = 0; i < len; i++) selText[i] = fullText[start + i];
                            selText[len] = 0;
                            CopyTextToClipboard(hwnd, selText, len);
                            VirtualFree(selText, 0, MEM_RELEASE);
                        }
                        VirtualFree(fullText, 0, MEM_RELEASE);
                    }
                } else {
                    ShowNativeStatus("No text selected to copy.");
                }
            }
            if (id == 1051) {
                int totalLen = GetWindowTextLengthA(hEdit);
                if (totalLen > 0) {
                    char* fullText = (char*)VirtualAlloc(NULL, totalLen + 1, MEM_COMMIT, PAGE_READWRITE);
                    if (fullText) {
                        GetWindowTextA(hEdit, fullText, totalLen + 1);
                        CopyTextToClipboard(hwnd, fullText, totalLen);
                        VirtualFree(fullText, 0, MEM_RELEASE);
                    }
                } else {
                    ShowNativeStatus("Document is empty.");
                }
            }
            if (id == 1052) {
                SendMessageA(hEdit, EM_SETSEL, 0, -1);
                ShowNativeStatus("Selected all document text [Ctrl+A]");
            }

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
                        char msg[1024];
                        wsprintfA(msg, "--- KREAD READING STATISTICS ENGINE ---\n\nActive Tab:\t\t%s (%d of %d)\nTotal Characters:\t%d\nTotal Words:\t\t%d\nTotal Lines:\t\t%d\nEst. Reading Speed:\t200 WPM\nEst. Time Remaining:\t%d minutes", 
                            g_Tabs[g_ActiveTab].szTitle, g_ActiveTab + 1, g_NumTabs, chars, words, lines, estMins);
                        
                        if (id == 1007) {
                            SaveStatsExport(hwnd, msg);
                        } else {
                            MessageBoxA(hwnd, msg, "KRead Statistics Engine", MB_OK | MB_ICONINFORMATION);
                        }
                        VirtualFree(text, 0, MEM_RELEASE);
                    }
                } else {
                    ShowNativeStatus("Current tab document is empty.");
                }
            }

            // Find Text
            if (id == 1004) {
                OpenFindDialog(hwnd);
            }

            // Help
            if (id == 1008) {
                ShowHelpDialog(hwnd);
            }

            // Bookmarks
            if (id == 1005) {
                DWORD start = 0, end = 0;
                SendMessageA(hEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
                g_Tabs[g_ActiveTab].dwBookmark = start;
                ShowNativeStatus("Bookmark saved for active tab! [Ctrl+B]");
            }
            if (id == 1006) {
                DWORD bm = g_Tabs[g_ActiveTab].dwBookmark;
                SendMessageA(hEdit, EM_SETSEL, bm, bm);
                SendMessageA(hEdit, EM_SCROLLCARET, 0, 0);
                ShowNativeStatus("Jumped to saved bookmark position! [Ctrl+J]");
            }

            // Themes
            if (id == 1010) { SetTheme(hwnd, RGB(250, 250, 250), RGB(30, 30, 30)); ShowNativeStatus("Theme: Light [Alt+1]"); }
            if (id == 1011) { SetTheme(hwnd, RGB(24, 24, 28), RGB(228, 228, 231)); ShowNativeStatus("Theme: Dark [Alt+2]"); }
            if (id == 1012) { SetTheme(hwnd, RGB(251, 240, 217), RGB(67, 52, 34)); ShowNativeStatus("Theme: Sepia [Alt+3]"); }
            if (id == 1013) { SetTheme(hwnd, RGB(0, 0, 0), RGB(0, 255, 102)); ShowNativeStatus("Theme: High-Contrast [Alt+4]"); }

            // Fonts
            if (id == 1020) { currentFontSize = min(currentFontSize + 2, 42); UpdateFont(hwnd); ShowNativeStatus("Font size increased"); UpdateStatusBar(); }
            if (id == 1021) { currentFontSize = max(currentFontSize - 2, 12); UpdateFont(hwnd); ShowNativeStatus("Font size decreased"); UpdateStatusBar(); }
            if (id == 1025) { currentFontSize = 18; UpdateFont(hwnd); ShowNativeStatus("Font size reset to 18pt [Ctrl+0]"); UpdateStatusBar(); }
            if (id == 1022) { lstrcpyA(currentFontFace, "Georgia"); UpdateFont(hwnd); ShowNativeStatus("Font: Georgia (Serif)"); UpdateStatusBar(); }
            if (id == 1023) { lstrcpyA(currentFontFace, "Segoe UI"); UpdateFont(hwnd); ShowNativeStatus("Font: Segoe UI (Sans)"); UpdateStatusBar(); }
            if (id == 1024) { lstrcpyA(currentFontFace, "Consolas"); UpdateFont(hwnd); ShowNativeStatus("Font: Consolas (Monospace)"); UpdateStatusBar(); }

            if (HIWORD(wParam) == EN_CHANGE) {
                UpdateStatusBar();
            }

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
            if (g_hFindDlg) {
                DestroyWindow(g_hFindDlg);
                g_hFindDlg = NULL;
            }
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
    HWND hwnd = CreateWindowExA(0, "KReadClass", "KRead Native E-Reader - Press F1 or H for Help", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (g_hFindDlg && IsDialogMessageA(g_hFindDlg, &msg)) {
            continue;
        }
        if (msg.message == WM_KEYDOWN) {
            BOOL ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            BOOL shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            BOOL alt = (GetKeyState(VK_MENU) & 0x8000) != 0;

            if (ctrl && (msg.wParam == 'A' || msg.wParam == 'a')) {
                SendMessageA(hEdit, EM_SETSEL, 0, -1);
                continue;
            }
            if (ctrl && (msg.wParam == 'C' || msg.wParam == 'c')) {
                if (shift) {
                    SendMessageA(hwnd, WM_COMMAND, 1051, 0); // Copy All
                    continue;
                }
            }
            if (ctrl && msg.wParam == VK_DELETE) {
                SendMessageA(hwnd, WM_COMMAND, 1000, 0); // Clear
                continue;
            }
            if (ctrl && (msg.wParam == 'T' || msg.wParam == 't')) {
                SendMessageA(hwnd, WM_COMMAND, 1030, 0);
                continue;
            }
            if (ctrl && (msg.wParam == 'W' || msg.wParam == 'w')) {
                SendMessageA(hwnd, WM_COMMAND, 1031, 0);
                continue;
            }
            if (ctrl && (msg.wParam == 'O' || msg.wParam == 'o')) {
                if (shift) {
                    SendMessageA(hwnd, WM_COMMAND, 1009, 0);
                } else {
                    SendMessageA(hwnd, WM_COMMAND, 1001, 0);
                }
                continue;
            }
            if (ctrl && (msg.wParam == 'F' || msg.wParam == 'f')) {
                SendMessageA(hwnd, WM_COMMAND, 1004, 0);
                continue;
            }
            if (ctrl && (msg.wParam == 'B' || msg.wParam == 'b')) {
                SendMessageA(hwnd, WM_COMMAND, 1005, 0);
                continue;
            }
            if (ctrl && (msg.wParam == 'J' || msg.wParam == 'j')) {
                SendMessageA(hwnd, WM_COMMAND, 1006, 0);
                continue;
            }
            if (ctrl && (msg.wParam == 'S' || msg.wParam == 's')) {
                SendMessageA(hwnd, WM_COMMAND, 1003, 0);
                continue;
            }
            if (ctrl && (msg.wParam == VK_OEM_PLUS || msg.wParam == VK_ADD || msg.wParam == '=')) {
                SendMessageA(hwnd, WM_COMMAND, 1020, 0);
                continue;
            }
            if (ctrl && (msg.wParam == VK_OEM_MINUS || msg.wParam == VK_SUBTRACT)) {
                SendMessageA(hwnd, WM_COMMAND, 1021, 0);
                continue;
            }
            if (ctrl && (msg.wParam == '0' || msg.wParam == VK_NUMPAD0)) {
                SendMessageA(hwnd, WM_COMMAND, 1025, 0);
                continue;
            }
            if (alt && msg.wParam >= '1' && msg.wParam <= '4') {
                SendMessageA(hwnd, WM_COMMAND, 1010 + (msg.wParam - '1'), 0);
                continue;
            }
            if (ctrl && msg.wParam >= '1' && msg.wParam <= '9') {
                int targetTab = (int)(msg.wParam - '1');
                if (targetTab < g_NumTabs) {
                    SwitchToTab(hwnd, targetTab);
                }
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
            if (msg.wParam == VK_F1 || (!ctrl && !shift && (msg.wParam == 'H' || msg.wParam == 'h'))) {
                SendMessageA(hwnd, WM_COMMAND, 1008, 0);
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}

