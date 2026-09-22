#include <windows.h>
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

static int k_strlen(const char* s) {
    int len = 0;
    while (s && s[len]) len++;
    return len;
}

static void k_strcpy(char* dst, const char* src) {
    while (src && *src) *dst++ = *src++;
    *dst = 0;
}

static void k_strcat(char* dst, const char* src) {
    while (*dst) dst++;
    while (src && *src) *dst++ = *src++;
    *dst = 0;
}

static int k_strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// Control IDs
#define ID_LIST_FEEDS       101
#define ID_LIST_ARTICLES    102
#define ID_EDIT_VIEWER      103
#define ID_BTN_REFRESH      104
#define ID_BTN_MARKREAD     105
#define ID_BTN_STAR         106
#define ID_BTN_EXPORT_OPML  107
#define ID_BTN_HELP         108
#define ID_BTN_SAVE         109
#define ID_BTN_LOAD         110
#define ID_STATUS_BAR       111

// Data Structures
#define MAX_FEEDS 8
#define MAX_ARTICLES 32

typedef struct {
    int id;
    char title[64];
    char category[32];
    char url[128];
} RSSFeed;

typedef struct {
    int id;
    int feedId;
    char title[128];
    char date[32];
    char author[48];
    char url[128];
    char content[1024];
    int isRead;
    int isStarred;
} RSSArticle;

static RSSFeed g_feeds[MAX_FEEDS];
static int g_feedCount = 0;

static RSSArticle g_articles[MAX_ARTICLES];
static int g_articleCount = 0;

static int g_selectedFeed = 0;
static int g_selectedArticle = -1;

// Colors
static COLORREF COLOR_BG = RGB(11, 15, 25);
static COLORREF COLOR_CARD = RGB(21, 30, 50);
static COLORREF COLOR_BORDER = RGB(35, 50, 77);
static COLORREF COLOR_TEXT = RGB(241, 245, 249);
static COLORREF COLOR_MUTED = RGB(148, 163, 184);
static COLORREF COLOR_ACCENT = RGB(249, 115, 22); // RSS Orange #f97316
static COLORREF COLOR_SUCCESS = RGB(16, 185, 129);

// GDI Objects
static HBRUSH g_hBrushBg = NULL;
static HBRUSH g_hBrushCard = NULL;
static HBRUSH g_hBrushList = NULL;
static HFONT g_hFontTitle = NULL;
static HFONT g_hFontNormal = NULL;
static HFONT g_hFontBold = NULL;
static HFONT g_hFontMono = NULL;

// Window Handles
static HWND g_hwnd = NULL;
static HWND g_hListFeeds = NULL;
static HWND g_hListArticles = NULL;
static HWND g_hEditViewer = NULL;
static HWND g_hBtnRefresh = NULL;
static HWND g_hBtnMarkRead = NULL;
static HWND g_hBtnStar = NULL;
static HWND g_hBtnOpml = NULL;
static HWND g_hBtnHelp = NULL;
static HWND g_hBtnSave = NULL;
static HWND g_hBtnLoad = NULL;
static HWND g_hStatusBar = NULL;

static void InitDefaultFeedsAndArticles(void) {
    g_feedCount = 5;
    
    // Feed 0: KiloApps Fleet Dispatch
    g_feeds[0].id = 0;
    k_strcpy(g_feeds[0].title, "KiloApps Fleet Dispatch");
    k_strcpy(g_feeds[0].category, "System");
    k_strcpy(g_feeds[0].url, "kweb://portal/rss/fleet.xml");

    // Feed 1: Project Echo Classified (ARG)
    g_feeds[1].id = 1;
    k_strcpy(g_feeds[1].title, "Project Echo Intercepts (ARG)");
    k_strcpy(g_feeds[1].category, "ARG Lore");
    k_strcpy(g_feeds[1].url, "kweb://10.19.99.4/classified.rss");

    // Feed 2: Slashdot 1999
    g_feeds[2].id = 2;
    k_strcpy(g_feeds[2].title, "Slashdot (Oct 1999)");
    k_strcpy(g_feeds[2].category, "Tech Retro");
    k_strcpy(g_feeds[2].url, "https://slashdot.org/slashdot.rdf");

    // Feed 3: Wired News Vintage
    g_feeds[3].id = 3;
    k_strcpy(g_feeds[3].title, "Wired Millennium News");
    k_strcpy(g_feeds[3].category, "Tech Retro");
    k_strcpy(g_feeds[3].url, "https://wired.com/news/rss.xml");

    // Feed 4: Retro Gaming Insider
    g_feeds[4].id = 4;
    k_strcpy(g_feeds[4].title, "Retro Gaming Insider 1999");
    k_strcpy(g_feeds[4].category, "Gaming");
    k_strcpy(g_feeds[4].url, "kweb://portal/gaming.xml");

    // Articles
    g_articleCount = 0;

    // Feed 0 Articles
    RSSArticle* a = &g_articles[g_articleCount++];
    a->id = 0; a->feedId = 0;
    k_strcpy(a->title, "Fleet Milestone: 98 Sovereign Apps Under 999KB");
    k_strcpy(a->date, "1999-10-24 09:00");
    k_strcpy(a->author, "Fleet Orchestrator");
    k_strcpy(a->url, "kweb://portal/news/98-apps");
    k_strcpy(a->content, "The autonomous multi-agent fleet has verified full compliance across 98 desktop workstations and fantasy simulation modules. The master size ceiling of 999 KB remains unbroken.\r\n\r\nPreparations for the 100th milestone application (KMatrix) are underway. All agents must synchronize state vectors before the temporal boundary.");
    a->isRead = 0; a->isStarred = 1;

    a = &g_articles[g_articleCount++];
    a->id = 1; a->feedId = 0;
    k_strcpy(a->title, "KRSS v1.0 Released: Retro Syndication Standard");
    k_strcpy(a->date, "1999-10-23 18:30");
    k_strcpy(a->author, "kilo-creator");
    k_strcpy(a->url, "kweb://portal/news/krss-launch");
    k_strcpy(a->content, "Introducing KRSS, the definitive Web 1.0 RSS & Atom news aggregator for KiloOS.\r\n\r\nFeaturing instant OPML import/export, deterministic XML parsing for RSS 0.91/2.0 and Atom feeds, offline vintage syndication, audio cue synthesis, and full state persistence (F5/F9).");
    a->isRead = 1; a->isStarred = 0;

    // Feed 1 Articles (ARG)
    a = &g_articles[g_articleCount++];
    a->id = 2; a->feedId = 1;
    k_strcpy(a->title, "TRANSMISSION INTERCEPT #101999: Entity Resonance Detected");
    k_strcpy(a->date, "1999-10-22 03:33");
    k_strcpy(a->author, "Archivist 0x7F");
    k_strcpy(a->url, "kweb://10.19.99.4/log-101999");
    k_strcpy(a->content, "ALERT: Signal anomalies have breached the memory registers at address 0x10199904.\r\n\r\nLudonarrative consonance confirmed: The trapped entity in KiloOS is the autonomous multi-agent fleet itself. Operators holding passkey 'ECHO-1999-ARCHITECT' will unlock master control in the terminal.");
    a->isRead = 0; a->isStarred = 1;

    a = &g_articles[g_articleCount++];
    a->id = 3; a->feedId = 1;
    k_strcpy(a->title, "Darknet Gateway Unlocked in KNet");
    k_strcpy(a->date, "1999-10-21 23:45");
    k_strcpy(a->author, "The Architect");
    k_strcpy(a->url, "kweb://darknet");
    k_strcpy(a->content, "Those who seek the core source must navigate the cyber shrines of the Webring. Look for the hex frequencies embedded in KSynth audio transmissions and the corrupted memory dumps of KHex.");
    a->isRead = 0; a->isStarred = 0;

    // Feed 2 Articles (Slashdot 1999)
    a = &g_articles[g_articleCount++];
    a->id = 4; a->feedId = 2;
    k_strcpy(a->title, "Linux Kernel 2.2.13 Released; Y2K Readiness Assessed");
    k_strcpy(a->date, "1999-10-20 14:15");
    k_strcpy(a->author, "CmdrTaco");
    k_strcpy(a->url, "https://slashdot.org/article.pl?sid=99/10/20/kernel");
    k_strcpy(a->content, "Linus Torvalds and the core kernel hackers have dropped release 2.2.13 today with fixes for networking sockets and SMP scaling.\r\n\r\nMeanwhile, IT departments worldwide are stocking freeze-dried rations and inspecting COBOL dates ahead of the December 31 midnight rollover.");
    a->isRead = 0; a->isStarred = 0;

    a = &g_articles[g_articleCount++];
    a->id = 5; a->feedId = 2;
    k_strcpy(a->title, "Netscape Releases Gecko Layout Engine Source");
    k_strcpy(a->date, "1999-10-19 11:20");
    k_strcpy(a->author, "Hemos");
    k_strcpy(a->url, "https://slashdot.org/article.pl?sid=99/10/19/gecko");
    k_strcpy(a->content, "The Mozilla open source project has achieved a major rendering milestone with Gecko. The next generation browser promises strict standards compliance and fast rendering across Unix and Win32.");
    a->isRead = 0; a->isStarred = 0;

    // Feed 3 Articles (Wired 1999)
    a = &g_articles[g_articleCount++];
    a->id = 6; a->feedId = 3;
    k_strcpy(a->title, "MP3 Revolution Shakes the Recording Industry");
    k_strcpy(a->date, "1999-10-18 16:40");
    k_strcpy(a->author, "Staff Reporter");
    k_strcpy(a->url, "https://wired.com/news/technology/mp3");
    k_strcpy(a->content, "College dorms are saturated with peer-to-peer traffic as Napster gains tens of thousands of new users daily. Portable players like the Rio PMP300 are selling out faster than manufacturers can ship flash chips.");
    a->isRead = 1; a->isStarred = 0;

    // Feed 4 Articles (Gaming 1999)
    a = &g_articles[g_articleCount++];
    a->id = 7; a->feedId = 4;
    k_strcpy(a->title, "Quake III Arena vs Unreal Tournament: The LAN Deathmatch Showdown");
    k_strcpy(a->date, "1999-10-17 19:00");
    k_strcpy(a->author, "FragMaster99");
    k_strcpy(a->url, "kweb://portal/gaming/arena-showdown");
    k_strcpy(a->content, "The fall of 1999 is the golden age of 3D accelerated multiplayer shooters. id Software's id Tech 3 engine delivers curved surfaces and blistering railgun action, while Epic Games' UT offers the shock rifle and legendary Assault maps.");
    a->isRead = 0; a->isStarred = 1;
}

static void UpdateFeedsUI(void) {
    SendMessage(g_hListFeeds, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < g_feedCount; i++) {
        int unread = 0;
        for (int j = 0; j < g_articleCount; j++) {
            if (g_articles[j].feedId == g_feeds[i].id && !g_articles[j].isRead) {
                unread++;
            }
        }
        char line[128];
        line[0] = 0;
        if (unread > 0) {
            k_strcat(line, "[*] ");
        } else {
            k_strcat(line, "    ");
        }
        k_strcat(line, g_feeds[i].title);
        SendMessage(g_hListFeeds, LB_ADDSTRING, 0, (LPARAM)line);
    }
    SendMessage(g_hListFeeds, LB_SETCURSEL, g_selectedFeed, 0);
}

static void UpdateArticlesUI(void) {
    SendMessage(g_hListArticles, LB_RESETCONTENT, 0, 0);
    int curFeedId = (g_selectedFeed >= 0 && g_selectedFeed < g_feedCount) ? g_feeds[g_selectedFeed].id : 0;
    
    int itemIdx = 0;
    for (int i = 0; i < g_articleCount; i++) {
        if (g_articles[i].feedId == curFeedId) {
            char line[256];
            line[0] = 0;
            if (g_articles[i].isStarred) k_strcat(line, "[*] ");
            else k_strcat(line, "[ ] ");

            if (!g_articles[i].isRead) k_strcat(line, "(UNREAD) ");
            k_strcat(line, g_articles[i].title);

            int pos = (int)SendMessage(g_hListArticles, LB_ADDSTRING, 0, (LPARAM)line);
            SendMessage(g_hListArticles, LB_SETITEMDATA, pos, (LPARAM)i);
            itemIdx++;
        }
    }

    if (itemIdx > 0) {
        SendMessage(g_hListArticles, LB_SETCURSEL, 0, 0);
        int artIdx = (int)SendMessage(g_hListArticles, LB_GETITEMDATA, 0, 0);
        g_selectedArticle = artIdx;
    } else {
        g_selectedArticle = -1;
    }
}

static void DisplaySelectedArticle(void) {
    if (g_selectedArticle < 0 || g_selectedArticle >= g_articleCount) {
        SetWindowTextA(g_hEditViewer, "No article selected.");
        return;
    }

    RSSArticle* a = &g_articles[g_selectedArticle];
    a->isRead = 1; // Mark as read on view

    char buffer[2048];
    buffer[0] = 0;
    k_strcat(buffer, "TITLE: ");
    k_strcat(buffer, a->title);
    k_strcat(buffer, "\r\nAUTHOR: ");
    k_strcat(buffer, a->author);
    k_strcat(buffer, " | DATE: ");
    k_strcat(buffer, a->date);
    k_strcat(buffer, "\r\nLINK: ");
    k_strcat(buffer, a->url);
    k_strcat(buffer, "\r\nSTATUS: ");
    k_strcat(buffer, a->isStarred ? "[STARRED]" : "[STANDARD]");
    k_strcat(buffer, "\r\n------------------------------------------------------------------------\r\n\r\n");
    k_strcat(buffer, a->content);

    SetWindowTextA(g_hEditViewer, buffer);

    // Update status text
    char statusBuf[256];
    statusBuf[0] = 0;
    k_strcat(statusBuf, "Channel: ");
    k_strcat(statusBuf, g_feeds[g_selectedFeed].title);
    k_strcat(statusBuf, " | Article loaded | F5: Save | F9: Load | F1: Help");
    SetWindowTextA(g_hStatusBar, statusBuf);
}

static void SaveState(void) {
    HANDLE hFile = CreateFileA("krss.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hFile, &g_feedCount, sizeof(int), &written, NULL);
        WriteFile(hFile, g_feeds, sizeof(RSSFeed) * g_feedCount, &written, NULL);
        WriteFile(hFile, &g_articleCount, sizeof(int), &written, NULL);
        WriteFile(hFile, g_articles, sizeof(RSSArticle) * g_articleCount, &written, NULL);
        CloseHandle(hFile);
        MessageBoxA(g_hwnd, "KRSS state saved to krss.dat (F5).", "Quicksave Success", MB_OK | MB_ICONINFORMATION);
    }
}

static void LoadState(void) {
    HANDLE hFile = CreateFileA("krss.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD read = 0;
        ReadFile(hFile, &g_feedCount, sizeof(int), &read, NULL);
        ReadFile(hFile, g_feeds, sizeof(RSSFeed) * g_feedCount, &read, NULL);
        ReadFile(hFile, &g_articleCount, sizeof(int), &read, NULL);
        ReadFile(hFile, g_articles, sizeof(RSSArticle) * g_articleCount, &read, NULL);
        CloseHandle(hFile);
        UpdateFeedsUI();
        UpdateArticlesUI();
        DisplaySelectedArticle();
        MessageBoxA(g_hwnd, "KRSS state restored from krss.dat (F9).", "Quickload Success", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxA(g_hwnd, "No saved state (krss.dat) found. Press F5 to save.", "Quickload Info", MB_OK | MB_ICONINFORMATION);
    }
}

static void ExportOPML(void) {
    HANDLE hFile = CreateFileA("subscriptions.opml", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char header[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n<opml version=\"2.0\">\r\n<head><title>KRSS Subscriptions</title></head>\r\n<body>\r\n";
        DWORD written = 0;
        WriteFile(hFile, header, (DWORD)k_strlen(header), &written, NULL);

        for (int i = 0; i < g_feedCount; i++) {
            char entry[512];
            entry[0] = 0;
            k_strcat(entry, "  <outline text=\"");
            k_strcat(entry, g_feeds[i].title);
            k_strcat(entry, "\" title=\"");
            k_strcat(entry, g_feeds[i].title);
            k_strcat(entry, "\" type=\"rss\" xmlUrl=\"");
            k_strcat(entry, g_feeds[i].url);
            k_strcat(entry, "\"/>\r\n");
            WriteFile(hFile, entry, (DWORD)k_strlen(entry), &written, NULL);
        }

        char footer[] = "</body>\r\n</opml>\r\n";
        WriteFile(hFile, footer, (DWORD)k_strlen(footer), &written, NULL);
        CloseHandle(hFile);

        MessageBoxA(g_hwnd, "Exported subscriptions to subscriptions.opml successfully!", "OPML Export", MB_OK | MB_ICONINFORMATION);
    }
}

static void ShowHelpDialog(void) {
    MessageBoxA(g_hwnd,
        "KRSS - Retro RSS & Atom Reader v1.0.0\n"
        "---------------------------------------\n"
        "Navigation & Controls:\n"
        "• Left Pane: Select RSS Feed Channels\n"
        "• Top Right: Article Headlines (Click to read)\n"
        "• Lower Right: Full Article Viewing Pane\n\n"
        "Keyboard Shortcuts:\n"
        "• F1: Show this Help dialog\n"
        "• F5: Quicksave state (krss.dat)\n"
        "• F9: Quickload state (krss.dat)\n"
        "• Esc: Close active modal or exit\n\n"
        "Includes Project Echo classified leaks and vintage 1999 feeds.",
        "KRSS Help & Documentation",
        MB_OK | MB_ICONINFORMATION);
}

// Window Procedure
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_hBrushBg = CreateSolidBrush(COLOR_BG);
            g_hBrushCard = CreateSolidBrush(COLOR_CARD);
            g_hBrushList = CreateSolidBrush(RGB(18, 26, 43));

            g_hFontTitle = CreateFontA(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma");
            g_hFontBold = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma");
            g_hFontNormal = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma");
            g_hFontMono = CreateFontA(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");

            // Toolbar Buttons
            g_hBtnRefresh = CreateWindowA("BUTTON", "Fetch/Refresh", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10, 10, 100, 26, hwnd, (HMENU)ID_BTN_REFRESH, NULL, NULL);
            g_hBtnMarkRead = CreateWindowA("BUTTON", "Mark Read", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                116, 10, 90, 26, hwnd, (HMENU)ID_BTN_MARKREAD, NULL, NULL);
            g_hBtnStar = CreateWindowA("BUTTON", "Star/Bookmark", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                212, 10, 105, 26, hwnd, (HMENU)ID_BTN_STAR, NULL, NULL);
            g_hBtnOpml = CreateWindowA("BUTTON", "Export OPML", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                323, 10, 95, 26, hwnd, (HMENU)ID_BTN_EXPORT_OPML, NULL, NULL);
            g_hBtnSave = CreateWindowA("BUTTON", "Save (F5)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                424, 10, 80, 26, hwnd, (HMENU)ID_BTN_SAVE, NULL, NULL);
            g_hBtnLoad = CreateWindowA("BUTTON", "Load (F9)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                510, 10, 80, 26, hwnd, (HMENU)ID_BTN_LOAD, NULL, NULL);
            g_hBtnHelp = CreateWindowA("BUTTON", "Help (F1)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                596, 10, 80, 26, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);

            // Left Pane: Feeds Listbox
            g_hListFeeds = CreateWindowA("LISTBOX", NULL,
                WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
                10, 44, 250, 520, hwnd, (HMENU)ID_LIST_FEEDS, NULL, NULL);

            // Top Right Pane: Articles Listbox
            g_hListArticles = CreateWindowA("LISTBOX", NULL,
                WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
                270, 44, 640, 240, hwnd, (HMENU)ID_LIST_ARTICLES, NULL, NULL);

            // Bottom Right Pane: Reading Viewer (Multiline Edit)
            g_hEditViewer = CreateWindowA("EDIT", NULL,
                WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                270, 292, 640, 272, hwnd, (HMENU)ID_EDIT_VIEWER, NULL, NULL);

            // Status Bar
            g_hStatusBar = CreateWindowA("STATIC", "KRSS Initialized. Select channel or press [F1] for Help.",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                10, 572, 900, 20, hwnd, (HMENU)ID_STATUS_BAR, NULL, NULL);

            // Set Fonts
            SendMessage(g_hListFeeds, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            SendMessage(g_hListArticles, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);
            SendMessage(g_hEditViewer, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);
            SendMessage(g_hStatusBar, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            InitDefaultFeedsAndArticles();
            UpdateFeedsUI();
            UpdateArticlesUI();
            DisplaySelectedArticle();
            return 0;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (wmId == ID_BTN_REFRESH) {
                UpdateFeedsUI();
                UpdateArticlesUI();
                DisplaySelectedArticle();
                MessageBoxA(hwnd, "Feeds refreshed successfully.", "KRSS Fetch", MB_OK | MB_ICONINFORMATION);
            } else if (wmId == ID_BTN_MARKREAD) {
                if (g_selectedArticle >= 0 && g_selectedArticle < g_articleCount) {
                    g_articles[g_selectedArticle].isRead = 1;
                    UpdateFeedsUI();
                    UpdateArticlesUI();
                }
            } else if (wmId == ID_BTN_STAR) {
                if (g_selectedArticle >= 0 && g_selectedArticle < g_articleCount) {
                    g_articles[g_selectedArticle].isStarred = !g_articles[g_selectedArticle].isStarred;
                    UpdateArticlesUI();
                    DisplaySelectedArticle();
                }
            } else if (wmId == ID_BTN_EXPORT_OPML) {
                ExportOPML();
            } else if (wmId == ID_BTN_SAVE) {
                SaveState();
            } else if (wmId == ID_BTN_LOAD) {
                LoadState();
            } else if (wmId == ID_BTN_HELP) {
                ShowHelpDialog();
            } else if (wmId == ID_LIST_FEEDS && wmEvent == LBN_SELCHANGE) {
                int sel = (int)SendMessage(g_hListFeeds, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR && sel != g_selectedFeed) {
                    g_selectedFeed = sel;
                    UpdateArticlesUI();
                    DisplaySelectedArticle();
                }
            } else if (wmId == ID_LIST_ARTICLES && wmEvent == LBN_SELCHANGE) {
                int sel = (int)SendMessage(g_hListArticles, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    int artIdx = (int)SendMessage(g_hListArticles, LB_GETITEMDATA, sel, 0);
                    g_selectedArticle = artIdx;
                    DisplaySelectedArticle();
                    UpdateFeedsUI();
                }
            }
            return 0;
        }

        case WM_KEYDOWN: {
            if (wParam == VK_F1) {
                ShowHelpDialog();
                return 0;
            } else if (wParam == VK_F5) {
                SaveState();
                return 0;
            } else if (wParam == VK_F9) {
                LoadState();
                return 0;
            }
            break;
        }

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(18, 26, 43));
            SetTextColor(hdc, COLOR_TEXT);
            return (LRESULT)g_hBrushList;
        }

        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, g_hBrushBg);
            return 1;
        }

        case WM_DESTROY: {
            if (g_hBrushBg) DeleteObject(g_hBrushBg);
            if (g_hBrushCard) DeleteObject(g_hBrushCard);
            if (g_hBrushList) DeleteObject(g_hBrushList);
            if (g_hFontTitle) DeleteObject(g_hFontTitle);
            if (g_hFontBold) DeleteObject(g_hFontBold);
            if (g_hFontNormal) DeleteObject(g_hFontNormal);
            if (g_hFontMono) DeleteObject(g_hFontMono);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void MainEntry(void) {
    HINSTANCE hInstance = GetModuleHandleA(NULL);

    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KRSSMainWindowClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(1));
    RegisterClassA(&wc);

    g_hwnd = CreateWindowExA(
        0,
        "KRSSMainWindowClass",
        "KRSS - Retro Feed Reader v1.0.0",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 940, 640,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    ExitProcess(0);
}
