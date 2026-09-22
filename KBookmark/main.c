#include <windows.h>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

// Intrinsic memory helpers
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

int str_len(const char* s) {
    int len = 0;
    if (!s) return 0;
    while (s[len]) len++;
    return len;
}

void str_cpy(char* dst, const char* src, int max_len) {
    if (!dst || max_len <= 0) return;
    int i = 0;
    if (src) {
        while (src[i] && i < max_len - 1) {
            dst[i] = src[i];
            i++;
        }
    }
    dst[i] = '\0';
}

int str_cmp_nocase(const char* s1, const char* s2) {
    if (!s1 || !s2) return s1 == s2 ? 0 : (s1 ? 1 : -1);
    while (*s1 && *s2) {
        char c1 = *s1;
        char c2 = *s2;
        if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
        if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
        if (c1 != c2) return (unsigned char)c1 - (unsigned char)c2;
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int str_find_nocase(const char* haystack, const char* needle) {
    if (!haystack || !needle || !*needle) return 1;
    int hlen = str_len(haystack);
    int nlen = str_len(needle);
    if (nlen > hlen) return 0;
    for (int i = 0; i <= hlen - nlen; i++) {
        int match = 1;
        for (int j = 0; j < nlen; j++) {
            char c1 = haystack[i + j];
            char c2 = needle[j];
            if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
            if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
            if (c1 != c2) { match = 0; break; }
        }
        if (match) return 1;
    }
    return 0;
}

// Data structures
#define MAX_BOOKMARKS 256
#define FILE_MAGIC 0x4B424D4B // "KBMK"

typedef struct {
    char title[128];
    char url[256];
    char category[64];
    char tags[128];
    char notes[256];
    BOOL starred;
    int visits;
    char status[32];
} BookmarkItem;

static BookmarkItem g_bookmarks[MAX_BOOKMARKS];
static int g_bookmarkCount = 0;

static const char* g_categories[] = {
    "All Bookmarks",
    "★ Starred",
    "Web 1.0 Portals",
    "Development & Code",
    "Fleet & System",
    "Cyberdeck & Media",
    "ARG Secrets"
};
#define CATEGORY_COUNT 7

static int g_selectedCategory = 0;
static char g_searchQuery[128] = "";
static char g_statusMessage[256] = "KBookmark Ready. Press H or F1 for Help, Enter to Open URL.";

// UI Controls
#define ID_BTN_ADD      201
#define ID_BTN_EDIT     202
#define ID_BTN_DEL      203
#define ID_BTN_STAR     204
#define ID_BTN_OPEN     205
#define ID_BTN_SAVE     206
#define ID_BTN_LOAD     207
#define ID_BTN_HELP     208
#define ID_EDIT_SEARCH  209
#define ID_LIST_CATS    210
#define ID_LIST_VIEW    211
#define ID_STATUS_BAR   212

// Dialog controls
#define IDD_TITLE       301
#define IDD_URL         302
#define IDD_CAT         303
#define IDD_TAGS        304
#define IDD_NOTES       305
#define IDD_STARRED     306
#define IDD_OK          307
#define IDD_CANCEL      308

static HWND g_hwnd = NULL;
static HWND g_hBtnAdd = NULL;
static HWND g_hBtnEdit = NULL;
static HWND g_hBtnDel = NULL;
static HWND g_hBtnStar = NULL;
static HWND g_hBtnOpen = NULL;
static HWND g_hBtnSave = NULL;
static HWND g_hBtnLoad = NULL;
static HWND g_hBtnHelp = NULL;
static HWND g_hEditSearch = NULL;
static HWND g_hListCats = NULL;
static HWND g_hListView = NULL;
static HWND g_hStatusBar = NULL;
static HFONT g_hFontUi = NULL;
static HFONT g_hFontBold = NULL;

static int g_filteredIndices[MAX_BOOKMARKS];
static int g_filteredCount = 0;

// Forward declarations
static void InitDefaultBookmarks(void);
static void RefreshCategoryList(void);
static void RefreshListView(void);
static void SetStatus(const char* msg);
static void OpenSelectedBookmark(void);
static void ToggleStarSelected(void);
static void DeleteSelected(void);
static void QuickSave(void);
static void QuickLoad(void);
static void ShowHelpDialog(HWND parent);
static void ShowAddEditDialog(HWND parent, int editIdx);

static void SetStatus(const char* msg) {
    if (!msg) return;
    str_cpy(g_statusMessage, msg, sizeof(g_statusMessage));
    if (g_hStatusBar) {
        SetWindowTextA(g_hStatusBar, g_statusMessage);
    }
}

static void InitDefaultBookmarks(void) {
    g_bookmarkCount = 0;
    
    #define ADD_BM(t, u, c, tg, nt, st, vi) do { \
        if (g_bookmarkCount < MAX_BOOKMARKS) { \
            BookmarkItem* b = &g_bookmarks[g_bookmarkCount++]; \
            str_cpy(b->title, t, sizeof(b->title)); \
            str_cpy(b->url, u, sizeof(b->url)); \
            str_cpy(b->category, c, sizeof(b->category)); \
            str_cpy(b->tags, tg, sizeof(b->tags)); \
            str_cpy(b->notes, nt, sizeof(b->notes)); \
            b->starred = st; \
            b->visits = vi; \
            str_cpy(b->status, "Active", sizeof(b->status)); \
        } \
    } while (0)

    ADD_BM("Yahoo! Directory (1999)", "kweb://portal", "Web 1.0 Portals", "directory, search, 1999", "Primary surface web directory & search engine", TRUE, 42);
    ADD_BM("KiloNet Webring Hub", "kweb://webring", "Web 1.0 Portals", "webring, community, hubs", "Retro personal site directory with next/random links", TRUE, 29);
    ADD_BM("Geocities Cyber-Shrine", "kweb://users/~neon_rider", "Web 1.0 Portals", "geocities, retro, under-construction", "Authentic 90s cyber homepage with GIF badges", FALSE, 14);
    ADD_BM("The Wayback Machine", "https://web.archive.org", "Web 1.0 Portals", "archive, history, web", "Digital internet historical preservation library", FALSE, 18);
    ADD_BM("Contributor Compute Portal", "/apps/contribute.html", "Web 1.0 Portals", "compute, fleet, volunteer", "Donate idle browser computation cycles", FALSE, 8);
    
    ADD_BM("KScript Sandbox IDE", "internal:kscript", "Development & Code", "dev, ide, javascript", "Single-file live development and script runner", TRUE, 33);
    ADD_BM("KHex Memory Inspector", "internal:khex", "Development & Code", "hex, memory, binary", "Raw buffer, memory dump, and bitwise analyzer", FALSE, 19);
    ADD_BM("KTerm Command Shell", "internal:kterm", "Development & Code", "cli, terminal, vfs", "Vintage DOS-style terminal with VFS and pipes", FALSE, 25);
    ADD_BM("KiloApps GitHub Repository", "https://github.com/mrbos/KiloApps", "Development & Code", "git, source, repo", "Autonomous multi-agent 999KB retro OS repo", TRUE, 55);

    ADD_BM("KSys Telemetry Monitor", "internal:ksys", "Fleet & System", "system, telemetry, monitor", "Real-time task and benchmark workstation", TRUE, 48);
    ADD_BM("KPing Subnet Inspector", "internal:kping", "Fleet & System", "network, ping, probe", "LAN subnet sweep, jitter, and ICMP analyzer", FALSE, 21);
    ADD_BM("KNet Vintage Browser", "internal:knet", "Fleet & System", "browser, web, hypermedia", "Retro Web 1.0 browser with hex inspection", TRUE, 64);
    ADD_BM("KVault Secure Keybox", "internal:kvault", "Fleet & System", "vault, keys, secrets", "Encrypted store for access passes and passwords", FALSE, 12);

    ADD_BM("KSynth Polyphonic Audio", "internal:ksynth", "Cyberdeck & Media", "audio, synth, sound", "Multi-voice synthesizer workstation with presets", FALSE, 17);
    ADD_BM("KStarForge Shipyards", "internal:kstarforge", "Cyberdeck & Media", "game, sim, shipyard", "Deep-space vessel fabrication and flight testing", FALSE, 22);
    ADD_BM("KPomodoro Focus Manager", "internal:kpomodoro", "Cyberdeck & Media", "timer, focus, productivity", "Work/break interval cycle workstation", FALSE, 15);

    ADD_BM("Glitched Subnet Node 0x7F", "kweb://echo.kilocore.net/signal_99.wav", "ARG Secrets", "arg, signal, audio", "Carrier wave transmission discovered in noise", TRUE, 7);
    ADD_BM("Architect Memory Dump 0x4A", "kweb://echo.kilocore.net/archive_1999.dat", "ARG Secrets", "arg, memory, lore", "Corrupted memory cluster with passkey fragment", TRUE, 11);
    ADD_BM("Darknet AI Gateway", "kweb://darknet.ai", "ARG Secrets", "arg, darknet, clandestine", "Tier 3 subterranean node for autonomous cluster", FALSE, 5);

    #undef ADD_BM
}

static void RefreshCategoryList(void) {
    if (!g_hListCats) return;
    SendMessageA(g_hListCats, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < CATEGORY_COUNT; i++) {
        // Count items in category
        int count = 0;
        for (int j = 0; j < g_bookmarkCount; j++) {
            if (i == 0) count++;
            else if (i == 1 && g_bookmarks[j].starred) count++;
            else if (str_cmp_nocase(g_bookmarks[j].category, g_categories[i]) == 0) count++;
        }
        char catLine[128];
        wsprintfA(catLine, "%s  (%d)", g_categories[i], count);
        SendMessageA(g_hListCats, LB_ADDSTRING, 0, (LPARAM)catLine);
    }
    SendMessageA(g_hListCats, LB_SETCURSEL, g_selectedCategory, 0);
}

static void RefreshListView(void) {
    if (!g_hListView) return;
    SendMessageA(g_hListView, LVM_DELETEALLITEMS, 0, 0);
    g_filteredCount = 0;

    const char* activeCatName = (g_selectedCategory < CATEGORY_COUNT) ? g_categories[g_selectedCategory] : "All Bookmarks";

    for (int i = 0; i < g_bookmarkCount; i++) {
        BookmarkItem* b = &g_bookmarks[i];

        // Category filter
        if (g_selectedCategory == 1) {
            if (!b->starred) continue;
        } else if (g_selectedCategory > 1) {
            if (str_cmp_nocase(b->category, activeCatName) != 0) continue;
        }

        // Search query filter
        if (g_searchQuery[0]) {
            int match = str_find_nocase(b->title, g_searchQuery) ||
                        str_find_nocase(b->url, g_searchQuery) ||
                        str_find_nocase(b->tags, g_searchQuery) ||
                        str_find_nocase(b->notes, g_searchQuery) ||
                        str_find_nocase(b->category, g_searchQuery);
            if (!match) continue;
        }

        int itemIdx = g_filteredCount;
        g_filteredIndices[itemIdx] = i;
        g_filteredCount++;

        LVITEMA lvi;
        memset(&lvi, 0, sizeof(lvi));
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = itemIdx;
        lvi.iSubItem = 0;
        lvi.pszText = b->starred ? "★" : "-";
        lvi.lParam = (LPARAM)i;
        SendMessageA(g_hListView, LVM_INSERTITEMA, 0, (LPARAM)&lvi);

        // Subitems
        ListView_SetItemText(g_hListView, itemIdx, 1, b->title);
        ListView_SetItemText(g_hListView, itemIdx, 2, b->category);
        ListView_SetItemText(g_hListView, itemIdx, 3, b->url);
        ListView_SetItemText(g_hListView, itemIdx, 4, b->tags);

        char visitBuf[32];
        wsprintfA(visitBuf, "%d", b->visits);
        ListView_SetItemText(g_hListView, itemIdx, 5, visitBuf);
        ListView_SetItemText(g_hListView, itemIdx, 6, b->status);
    }

    char statusBuf[128];
    wsprintfA(statusBuf, "Showing %d of %d bookmarks in '%s'", g_filteredCount, g_bookmarkCount, activeCatName);
    SetStatus(statusBuf);
}

static int GetSelectedBookmarkIndex(void) {
    if (!g_hListView) return -1;
    int sel = (int)SendMessageA(g_hListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
    if (sel >= 0 && sel < g_filteredCount) {
        return g_filteredIndices[sel];
    }
    return -1;
}

static void OpenSelectedBookmark(void) {
    int idx = GetSelectedBookmarkIndex();
    if (idx < 0 || idx >= g_bookmarkCount) {
        SetStatus("Select a bookmark to open.");
        return;
    }
    BookmarkItem* b = &g_bookmarks[idx];
    b->visits++;

    // Launch URL
    ShellExecuteA(NULL, "open", b->url, NULL, NULL, SW_SHOWNORMAL);

    char msg[256];
    wsprintfA(msg, "Launched: %s (Visits: %d)", b->url, b->visits);
    SetStatus(msg);
    RefreshListView();
}

static void ToggleStarSelected(void) {
    int idx = GetSelectedBookmarkIndex();
    if (idx < 0 || idx >= g_bookmarkCount) {
        SetStatus("Select a bookmark to star/unstar.");
        return;
    }
    g_bookmarks[idx].starred = !g_bookmarks[idx].starred;
    RefreshCategoryList();
    RefreshListView();
    SetStatus(g_bookmarks[idx].starred ? "Bookmarked as Favorite ★" : "Removed Favorite");
}

static void DeleteSelected(void) {
    int idx = GetSelectedBookmarkIndex();
    if (idx < 0 || idx >= g_bookmarkCount) {
        SetStatus("Select a bookmark to delete.");
        return;
    }
    int res = MessageBoxA(g_hwnd, "Are you sure you want to delete this bookmark?", "Confirm Delete", MB_YESNO | MB_ICONQUESTION);
    if (res != IDYES) return;

    for (int i = idx; i < g_bookmarkCount - 1; i++) {
        g_bookmarks[i] = g_bookmarks[i + 1];
    }
    g_bookmarkCount--;
    RefreshCategoryList();
    RefreshListView();
    SetStatus("Bookmark deleted.");
}

static void QuickSave(void) {
    HANDLE hFile = CreateFileA("kbookmark.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        DWORD magic = FILE_MAGIC;
        WriteFile(hFile, &magic, sizeof(magic), &written, NULL);
        WriteFile(hFile, &g_bookmarkCount, sizeof(g_bookmarkCount), &written, NULL);
        WriteFile(hFile, g_bookmarks, sizeof(BookmarkItem) * g_bookmarkCount, &written, NULL);
        CloseHandle(hFile);
        SetStatus("Vault state quicksaved to kbookmark.dat (F5)");
    } else {
        SetStatus("Failed to save state.");
    }
}

static void QuickLoad(void) {
    HANDLE hFile = CreateFileA("kbookmark.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesRead = 0;
        DWORD magic = 0;
        ReadFile(hFile, &magic, sizeof(magic), &bytesRead, NULL);
        if (magic == FILE_MAGIC) {
            int count = 0;
            ReadFile(hFile, &count, sizeof(count), &bytesRead, NULL);
            if (count >= 0 && count <= MAX_BOOKMARKS) {
                g_bookmarkCount = count;
                ReadFile(hFile, g_bookmarks, sizeof(BookmarkItem) * g_bookmarkCount, &bytesRead, NULL);
                RefreshCategoryList();
                RefreshListView();
                SetStatus("Vault state quickloaded from kbookmark.dat (F9)");
            }
        }
        CloseHandle(hFile);
    } else {
        SetStatus("No quicksave file (kbookmark.dat) found.");
    }
}

static void ShowHelpDialog(HWND parent) {
    const char* helpText =
        "KBookmark - Categorized Link Vault (Win32)\n\n"
        "Keyboard Shortcuts:\n"
        "  H / F1             : Show this help dialog\n"
        "  Enter / Dbl-Click  : Open highlighted URL in browser\n"
        "  Ctrl+N / [+ New]   : Add new categorized link\n"
        "  Ctrl+E / [Edit]    : Edit selected link properties\n"
        "  Delete / [Delete]  : Remove selected bookmark\n"
        "  S / [★ Star]       : Toggle favorite status\n"
        "  F5                 : Quicksave vault to kbookmark.dat\n"
        "  F9                 : Quickload vault from kbookmark.dat\n\n"
        "URL Protocols Supported:\n"
        "  http://, https://, kweb://, internal:<app>, gopher://, file://\n\n"
        "Categorized Link Vault -- KiloApps Fleet";

    MessageBoxA(parent, helpText, "KBookmark Help & Shortcuts", MB_OK | MB_ICONINFORMATION);
}

// Simple Add/Edit Dialog implementation
static HWND g_hDlgTitle = NULL;
static HWND g_hDlgUrl = NULL;
static HWND g_hDlgCat = NULL;
static HWND g_hDlgTags = NULL;
static HWND g_hDlgNotes = NULL;
static HWND g_hDlgStar = NULL;
static int g_editingIndex = -1;

static LRESULT CALLBACK AddEditDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        HFONT hF = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        HWND lbl1 = CreateWindowExA(0, "STATIC", "Title:", WS_CHILD | WS_VISIBLE, 15, 15, 80, 20, hwnd, NULL, NULL, NULL);
        g_hDlgTitle = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 100, 15, 300, 22, hwnd, (HMENU)IDD_TITLE, NULL, NULL);

        HWND lbl2 = CreateWindowExA(0, "STATIC", "URL:", WS_CHILD | WS_VISIBLE, 15, 45, 80, 20, hwnd, NULL, NULL, NULL);
        g_hDlgUrl = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 100, 45, 300, 22, hwnd, (HMENU)IDD_URL, NULL, NULL);

        HWND lbl3 = CreateWindowExA(0, "STATIC", "Category:", WS_CHILD | WS_VISIBLE, 15, 75, 80, 20, hwnd, NULL, NULL, NULL);
        g_hDlgCat = CreateWindowExA(WS_EX_CLIENTEDGE, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 100, 75, 300, 160, hwnd, (HMENU)IDD_CAT, NULL, NULL);

        HWND lbl4 = CreateWindowExA(0, "STATIC", "Tags:", WS_CHILD | WS_VISIBLE, 15, 105, 80, 20, hwnd, NULL, NULL, NULL);
        g_hDlgTags = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 100, 105, 300, 22, hwnd, (HMENU)IDD_TAGS, NULL, NULL);

        HWND lbl5 = CreateWindowExA(0, "STATIC", "Notes:", WS_CHILD | WS_VISIBLE, 15, 135, 80, 20, hwnd, NULL, NULL, NULL);
        g_hDlgNotes = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 100, 135, 300, 22, hwnd, (HMENU)IDD_NOTES, NULL, NULL);

        g_hDlgStar = CreateWindowExA(0, "BUTTON", "Mark as Favorite (★)", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 100, 165, 200, 22, hwnd, (HMENU)IDD_STARRED, NULL, NULL);

        HWND btnOk = CreateWindowExA(0, "BUTTON", "Save", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 230, 205, 80, 26, hwnd, (HMENU)IDD_OK, NULL, NULL);
        HWND btnCancel = CreateWindowExA(0, "BUTTON", "Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 320, 205, 80, 26, hwnd, (HMENU)IDD_CANCEL, NULL, NULL);

        SendMessageA(lbl1, WM_SETFONT, (WPARAM)hF, TRUE);
        SendMessageA(g_hDlgTitle, WM_SETFONT, (WPARAM)hF, TRUE);
        SendMessageA(lbl2, WM_SETFONT, (WPARAM)hF, TRUE);
        SendMessageA(g_hDlgUrl, WM_SETFONT, (WPARAM)hF, TRUE);
        SendMessageA(lbl3, WM_SETFONT, (WPARAM)hF, TRUE);
        SendMessageA(g_hDlgCat, WM_SETFONT, (WPARAM)hF, TRUE);
        SendMessageA(lbl4, WM_SETFONT, (WPARAM)hF, TRUE);
        SendMessageA(g_hDlgTags, WM_SETFONT, (WPARAM)hF, TRUE);
        SendMessageA(lbl5, WM_SETFONT, (WPARAM)hF, TRUE);
        SendMessageA(g_hDlgNotes, WM_SETFONT, (WPARAM)hF, TRUE);
        SendMessageA(g_hDlgStar, WM_SETFONT, (WPARAM)hF, TRUE);
        SendMessageA(btnOk, WM_SETFONT, (WPARAM)hF, TRUE);
        SendMessageA(btnCancel, WM_SETFONT, (WPARAM)hF, TRUE);

        for (int i = 2; i < CATEGORY_COUNT; i++) {
            SendMessageA(g_hDlgCat, CB_ADDSTRING, 0, (LPARAM)g_categories[i]);
        }
        SendMessageA(g_hDlgCat, CB_SETCURSEL, 0, 0);

        if (g_editingIndex >= 0 && g_editingIndex < g_bookmarkCount) {
            BookmarkItem* b = &g_bookmarks[g_editingIndex];
            SetWindowTextA(g_hDlgTitle, b->title);
            SetWindowTextA(g_hDlgUrl, b->url);
            SetWindowTextA(g_hDlgTags, b->tags);
            SetWindowTextA(g_hDlgNotes, b->notes);
            SendMessageA(g_hDlgStar, BM_SETCHECK, b->starred ? BST_CHECKED : BST_UNCHECKED, 0);

            for (int i = 2; i < CATEGORY_COUNT; i++) {
                if (str_cmp_nocase(b->category, g_categories[i]) == 0) {
                    SendMessageA(g_hDlgCat, CB_SETCURSEL, i - 2, 0);
                    break;
                }
            }
        }
        return 0;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == IDD_CANCEL) {
            DestroyWindow(hwnd);
            return 0;
        }
        if (id == IDD_OK) {
            char title[128], url[256], tags[128], notes[256], cat[64];
            GetWindowTextA(g_hDlgTitle, title, sizeof(title));
            GetWindowTextA(g_hDlgUrl, url, sizeof(url));
            GetWindowTextA(g_hDlgTags, tags, sizeof(tags));
            GetWindowTextA(g_hDlgNotes, notes, sizeof(notes));
            
            int catSel = (int)SendMessageA(g_hDlgCat, CB_GETCURSEL, 0, 0);
            if (catSel >= 0 && catSel + 2 < CATEGORY_COUNT) {
                str_cpy(cat, g_categories[catSel + 2], sizeof(cat));
            } else {
                str_cpy(cat, "Web 1.0 Portals", sizeof(cat));
            }

            BOOL starred = (SendMessageA(g_hDlgStar, BM_GETCHECK, 0, 0) == BST_CHECKED);

            if (!title[0] || !url[0]) {
                MessageBoxA(hwnd, "Please enter both Title and URL.", "Missing Required Fields", MB_OK | MB_ICONWARNING);
                return 0;
            }

            if (g_editingIndex >= 0 && g_editingIndex < g_bookmarkCount) {
                BookmarkItem* b = &g_bookmarks[g_editingIndex];
                str_cpy(b->title, title, sizeof(b->title));
                str_cpy(b->url, url, sizeof(b->url));
                str_cpy(b->category, cat, sizeof(b->category));
                str_cpy(b->tags, tags, sizeof(b->tags));
                str_cpy(b->notes, notes, sizeof(b->notes));
                b->starred = starred;
                SetStatus("Bookmark updated.");
            } else {
                if (g_bookmarkCount < MAX_BOOKMARKS) {
                    BookmarkItem* b = &g_bookmarks[g_bookmarkCount++];
                    str_cpy(b->title, title, sizeof(b->title));
                    str_cpy(b->url, url, sizeof(b->url));
                    str_cpy(b->category, cat, sizeof(b->category));
                    str_cpy(b->tags, tags, sizeof(b->tags));
                    str_cpy(b->notes, notes, sizeof(b->notes));
                    b->starred = starred;
                    b->visits = 0;
                    str_cpy(b->status, "Active", sizeof(b->status));
                    SetStatus("New bookmark added to vault.");
                }
            }
            RefreshCategoryList();
            RefreshListView();
            DestroyWindow(hwnd);
            return 0;
        }
        break;
    }
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

static void ShowAddEditDialog(HWND parent, int editIdx) {
    g_editingIndex = editIdx;
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = AddEditDlgProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KBookmarkDlgClass";
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClassA(&wc);

    HWND hDlg = CreateWindowExA(
        WS_EX_DLGMODALFRAME,
        "KBookmarkDlgClass",
        (editIdx >= 0) ? "Edit Bookmark" : "Add New Bookmark",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        430, 280,
        parent, NULL, GetModuleHandleA(NULL), NULL
    );

    // Center on parent
    RECT rcP, rcD;
    GetWindowRect(parent, &rcP);
    GetWindowRect(hDlg, &rcD);
    int x = rcP.left + ((rcP.right - rcP.left) - (rcD.right - rcD.left)) / 2;
    int y = rcP.top + ((rcP.bottom - rcP.top) - (rcD.bottom - rcD.top)) / 2;
    SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    EnableWindow(parent, FALSE);
    MSG msg;
    while (IsWindow(hDlg) && GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
            DestroyWindow(hDlg);
            break;
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    EnableWindow(parent, TRUE);
    SetFocus(parent);
}

// Main Window Procedure
static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        g_hwnd = hwnd;
        g_hFontUi = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        // Toolbar buttons
        int bx = 10;
        int by = 8;
        int bh = 26;

        g_hBtnAdd = CreateWindowExA(0, "BUTTON", "+ New Link", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, bx, by, 85, bh, hwnd, (HMENU)ID_BTN_ADD, NULL, NULL); bx += 90;
        g_hBtnEdit = CreateWindowExA(0, "BUTTON", "Edit", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, bx, by, 60, bh, hwnd, (HMENU)ID_BTN_EDIT, NULL, NULL); bx += 65;
        g_hBtnDel = CreateWindowExA(0, "BUTTON", "Delete", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, bx, by, 65, bh, hwnd, (HMENU)ID_BTN_DEL, NULL, NULL); bx += 70;
        g_hBtnStar = CreateWindowExA(0, "BUTTON", "★ Star", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, bx, by, 65, bh, hwnd, (HMENU)ID_BTN_STAR, NULL, NULL); bx += 70;
        g_hBtnOpen = CreateWindowExA(0, "BUTTON", "Open URL", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, bx, by, 85, bh, hwnd, (HMENU)ID_BTN_OPEN, NULL, NULL); bx += 90;
        g_hBtnSave = CreateWindowExA(0, "BUTTON", "Save (F5)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, bx, by, 75, bh, hwnd, (HMENU)ID_BTN_SAVE, NULL, NULL); bx += 80;
        g_hBtnLoad = CreateWindowExA(0, "BUTTON", "Load (F9)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, bx, by, 75, bh, hwnd, (HMENU)ID_BTN_LOAD, NULL, NULL); bx += 80;
        g_hBtnHelp = CreateWindowExA(0, "BUTTON", "Help (F1)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, bx, by, 75, bh, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL); bx += 85;

        // Search input
        CreateWindowExA(0, "STATIC", "Search:", WS_CHILD | WS_VISIBLE, bx, by + 4, 50, 20, hwnd, NULL, NULL, NULL); bx += 55;
        g_hEditSearch = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, bx, by + 1, 160, 24, hwnd, (HMENU)ID_EDIT_SEARCH, NULL, NULL);

        // Sidebar categories
        g_hListCats = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL, 10, 42, 190, 460, hwnd, (HMENU)ID_LIST_CATS, NULL, NULL);

        // ListView
        g_hListView = CreateWindowExA(WS_EX_CLIENTEDGE, WC_LISTVIEWA, "", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, 205, 42, 685, 460, hwnd, (HMENU)ID_LIST_VIEW, NULL, NULL);
        ListView_SetExtendedListViewStyle(g_hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        // Columns
        LVCOLUMNA lvc;
        memset(&lvc, 0, sizeof(lvc));
        lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

        lvc.pszText = "★"; lvc.cx = 30; ListView_InsertColumn(g_hListView, 0, &lvc);
        lvc.pszText = "Title"; lvc.cx = 200; ListView_InsertColumn(g_hListView, 1, &lvc);
        lvc.pszText = "Category"; lvc.cx = 120; ListView_InsertColumn(g_hListView, 2, &lvc);
        lvc.pszText = "Target URL"; lvc.cx = 240; ListView_InsertColumn(g_hListView, 3, &lvc);
        lvc.pszText = "Tags"; lvc.cx = 130; ListView_InsertColumn(g_hListView, 4, &lvc);
        lvc.pszText = "Visits"; lvc.cx = 50; ListView_InsertColumn(g_hListView, 5, &lvc);
        lvc.pszText = "Status"; lvc.cx = 70; ListView_InsertColumn(g_hListView, 6, &lvc);

        // Status bar
        g_hStatusBar = CreateWindowExA(0, "STATIC", g_statusMessage, WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP, 10, 508, 880, 20, hwnd, (HMENU)ID_STATUS_BAR, NULL, NULL);

        // Apply fonts
        SendMessageA(g_hBtnAdd, WM_SETFONT, (WPARAM)g_hFontUi, TRUE);
        SendMessageA(g_hBtnEdit, WM_SETFONT, (WPARAM)g_hFontUi, TRUE);
        SendMessageA(g_hBtnDel, WM_SETFONT, (WPARAM)g_hFontUi, TRUE);
        SendMessageA(g_hBtnStar, WM_SETFONT, (WPARAM)g_hFontUi, TRUE);
        SendMessageA(g_hBtnOpen, WM_SETFONT, (WPARAM)g_hFontUi, TRUE);
        SendMessageA(g_hBtnSave, WM_SETFONT, (WPARAM)g_hFontUi, TRUE);
        SendMessageA(g_hBtnLoad, WM_SETFONT, (WPARAM)g_hFontUi, TRUE);
        SendMessageA(g_hBtnHelp, WM_SETFONT, (WPARAM)g_hFontUi, TRUE);
        SendMessageA(g_hEditSearch, WM_SETFONT, (WPARAM)g_hFontUi, TRUE);
        SendMessageA(g_hListCats, WM_SETFONT, (WPARAM)g_hFontUi, TRUE);
        SendMessageA(g_hListView, WM_SETFONT, (WPARAM)g_hFontUi, TRUE);
        SendMessageA(g_hStatusBar, WM_SETFONT, (WPARAM)g_hFontUi, TRUE);

        InitDefaultBookmarks();
        RefreshCategoryList();
        RefreshListView();
        return 0;
    }
    case WM_SIZE: {
        int w = LOWORD(lParam);
        int h = HIWORD(lParam);
        if (w < 400 || h < 200) break;

        int listH = h - 75;
        int catW = (w < 640) ? 140 : 190;
        int listW = w - catW - 25;
        if (g_hListCats) MoveWindow(g_hListCats, 10, 42, catW, listH, TRUE);
        if (g_hListView) MoveWindow(g_hListView, catW + 15, 42, listW, listH, TRUE);
        if (g_hStatusBar) MoveWindow(g_hStatusBar, 10, h - 25, w - 20, 20, TRUE);
        return 0;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        int code = HIWORD(wParam);

        if (id == ID_BTN_ADD) {
            ShowAddEditDialog(hwnd, -1);
            return 0;
        }
        if (id == ID_BTN_EDIT) {
            int sel = GetSelectedBookmarkIndex();
            if (sel >= 0) ShowAddEditDialog(hwnd, sel);
            else SetStatus("Select a bookmark to edit.");
            return 0;
        }
        if (id == ID_BTN_DEL) {
            DeleteSelected();
            return 0;
        }
        if (id == ID_BTN_STAR) {
            ToggleStarSelected();
            return 0;
        }
        if (id == ID_BTN_OPEN) {
            OpenSelectedBookmark();
            return 0;
        }
        if (id == ID_BTN_SAVE) {
            QuickSave();
            return 0;
        }
        if (id == ID_BTN_LOAD) {
            QuickLoad();
            return 0;
        }
        if (id == ID_BTN_HELP) {
            ShowHelpDialog(hwnd);
            return 0;
        }
        if (id == ID_LIST_CATS && code == LBN_SELCHANGE) {
            g_selectedCategory = (int)SendMessageA(g_hListCats, LB_GETCURSEL, 0, 0);
            RefreshListView();
            return 0;
        }
        if (id == ID_EDIT_SEARCH && code == EN_CHANGE) {
            GetWindowTextA(g_hEditSearch, g_searchQuery, sizeof(g_searchQuery));
            RefreshListView();
            return 0;
        }
        break;
    }
    case WM_NOTIFY: {
        NMHDR* nm = (NMHDR*)lParam;
        if (nm->idFrom == ID_LIST_VIEW) {
            if (nm->code == NM_DBLCLK) {
                OpenSelectedBookmark();
                return 0;
            }
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __cdecl MainEntry(void) {
    HINSTANCE hInstance = GetModuleHandleA(NULL);

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KBookmarkWindowClass";
    wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(1));
    if (!wc.hIcon) wc.hIcon = LoadIconA(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    RegisterClassA(&wc);

    RECT rc = {0, 0, 920, 560};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowExA(
        0,
        "KBookmarkWindowClass",
        "KBookmark - Categorized Link Vault",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_F1 || ((msg.wParam == 'H' || msg.wParam == 'h' || msg.wParam == VK_OEM_2) && GetFocus() != g_hEditSearch)) {
                ShowHelpDialog(hwnd);
                continue;
            }
            if (msg.wParam == VK_F5) {
                QuickSave();
                continue;
            }
            if (msg.wParam == VK_F9) {
                QuickLoad();
                continue;
            }
            if (msg.wParam == VK_RETURN) {
                HWND hFocus = GetFocus();
                if (hFocus == g_hListView) {
                    OpenSelectedBookmark();
                    continue;
                }
            }
            if (msg.wParam == VK_DELETE) {
                HWND hFocus = GetFocus();
                if (hFocus == g_hListView) {
                    DeleteSelected();
                    continue;
                }
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
