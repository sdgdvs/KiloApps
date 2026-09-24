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

static void k_itoa(int val, char* buf) {
    char tmp[16];
    int i = 0;
    int isNeg = 0;
    if (val == 0) {
        buf[0] = '0';
        buf[1] = 0;
        return;
    }
    if (val < 0) {
        isNeg = 1;
        val = -val;
    }
    while (val > 0) {
        tmp[i++] = (char)('0' + (val % 10));
        val /= 10;
    }
    int j = 0;
    if (isNeg) buf[j++] = '-';
    while (i > 0) {
        buf[j++] = tmp[--i];
    }
    buf[j] = 0;
}

// Control IDs
#define ID_LIST_CLIPS       101
#define ID_EDIT_PREVIEW     102
#define ID_BTN_CAPTURE      103
#define ID_BTN_COPY         104
#define ID_BTN_PIN          105
#define ID_BTN_DELETE       106
#define ID_BTN_CLEAR        107
#define ID_BTN_UPPER        108
#define ID_BTN_LOWER        109
#define ID_BTN_ROT13        110
#define ID_BTN_TRIM         111
#define ID_BTN_SNIPPET      112
#define ID_BTN_HELP         113
#define ID_BTN_SAVE         114
#define ID_BTN_LOAD         115
#define ID_STATUS_BAR       116

// Maximum items
#define MAX_CLIPS 64
#define MAX_CLIP_TEXT 2048

typedef struct {
    int id;
    char title[64];
    char category[32];
    char content[MAX_CLIP_TEXT];
    char timestamp[32];
    int isPinned;
    int charCount;
    int lineCount;
} ClipItem;

static ClipItem g_clips[MAX_CLIPS];
static int g_clipCount = 0;
static int g_selectedClip = 0;
static int g_nextClipId = 1;

// Colors
static COLORREF COLOR_BG = RGB(11, 15, 25);
static COLORREF COLOR_CARD = RGB(21, 30, 50);
static COLORREF COLOR_TEXT = RGB(241, 245, 249);
static COLORREF COLOR_MUTED = RGB(148, 163, 184);
static COLORREF COLOR_ACCENT = RGB(6, 182, 212); // Cyan #06b6d4
static COLORREF COLOR_GOLD = RGB(234, 179, 8);   // Gold #eab308
static COLORREF COLOR_SUCCESS = RGB(16, 185, 129);

// GDI Objects
static HBRUSH g_hBrushBg = NULL;
static HBRUSH g_hBrushCard = NULL;
static HBRUSH g_hBrushList = NULL;
static HFONT g_hFontTitle = NULL;
static HFONT g_hFontNormal = NULL;
static HFONT g_hFontMono = NULL;

// Window Handles
static HWND g_hwnd = NULL;
static HWND g_hListClips = NULL;
static HWND g_hEditPreview = NULL;
static HWND g_hStatusBar = NULL;
static HWND g_hBtnCapture = NULL;
static HWND g_hBtnCopy = NULL;
static HWND g_hBtnPin = NULL;
static HWND g_hBtnDelete = NULL;
static HWND g_hBtnClear = NULL;
static HWND g_hBtnUpper = NULL;
static HWND g_hBtnLower = NULL;
static HWND g_hBtnRot13 = NULL;
static HWND g_hBtnTrim = NULL;
static HWND g_hBtnSnippet = NULL;
static HWND g_hBtnHelp = NULL;
static HWND g_hBtnSave = NULL;
static HWND g_hBtnLoad = NULL;

static void AnalyzeClip(ClipItem* clip) {
    int chars = 0;
    int lines = 1;
    const char* p = clip->content;
    while (*p) {
        chars++;
        if (*p == '\n') lines++;
        p++;
    }
    clip->charCount = chars;
    clip->lineCount = lines;

    // Detect category
    if (clip->content[0] == '{' || clip->content[0] == '[') {
        k_strcpy(clip->category, "JSON");
    } else if (clip->content[0] == '#' && (chars == 4 || chars == 7 || chars == 9)) {
        k_strcpy(clip->category, "Color");
    } else if (clip->content[0] == 'h' && clip->content[1] == 't' && clip->content[2] == 't' && clip->content[3] == 'p') {
        k_strcpy(clip->category, "URL");
    } else if (clip->content[0] == 'k' && clip->content[1] == 'w' && clip->content[2] == 'e' && clip->content[3] == 'b') {
        k_strcpy(clip->category, "KNet URL");
    } else if (chars > 16 && (clip->content[0] == '0' && clip->content[1] == 'x')) {
        k_strcpy(clip->category, "Hex/Mem");
    } else {
        // Check for code tokens
        int hasCode = 0;
        for (int i = 0; i < chars - 3; i++) {
            if (clip->content[i] == ';' && clip->content[i+1] == '\n') hasCode = 1;
            if (clip->content[i] == '=' && clip->content[i+1] == '>') hasCode = 1;
            if (clip->content[i] == '<' && clip->content[i+1] == '/') hasCode = 1;
        }
        if (hasCode) k_strcpy(clip->category, "Code");
        else k_strcpy(clip->category, "Text");
    }

    // Build title snippet
    int titleIdx = 0;
    for (int i = 0; i < chars && titleIdx < 45; i++) {
        char c = clip->content[i];
        if (c == '\r' || c == '\n' || c == '\t') c = ' ';
        clip->title[titleIdx++] = c;
    }
    if (chars > 45) {
        clip->title[titleIdx++] = '.';
        clip->title[titleIdx++] = '.';
        clip->title[titleIdx++] = '.';
    }
    clip->title[titleIdx] = 0;
}

static void AddClip(const char* content, int isPinned, const char* timestamp) {
    if (!content || !content[0]) return;
    if (g_clipCount >= MAX_CLIPS) {
        // Remove oldest unpinned clip
        int removeIdx = -1;
        for (int i = g_clipCount - 1; i >= 0; i--) {
            if (!g_clips[i].isPinned) {
                removeIdx = i;
                break;
            }
        }
        if (removeIdx >= 0) {
            for (int i = removeIdx; i < g_clipCount - 1; i++) {
                g_clips[i] = g_clips[i + 1];
            }
            g_clipCount--;
        } else {
            return; // All are pinned
        }
    }

    // Shift down to insert at top
    for (int i = g_clipCount; i > 0; i--) {
        g_clips[i] = g_clips[i - 1];
    }

    ClipItem* c = &g_clips[0];
    c->id = g_nextClipId++;
    int len = k_strlen(content);
    if (len >= MAX_CLIP_TEXT) len = MAX_CLIP_TEXT - 1;
    for (int i = 0; i < len; i++) c->content[i] = content[i];
    c->content[len] = 0;
    c->isPinned = isPinned;
    k_strcpy(c->timestamp, timestamp ? timestamp : "Just now");

    AnalyzeClip(c);
    g_clipCount++;
}

static void InitDefaultClips(void) {
    g_clipCount = 0;
    g_nextClipId = 1;

    // Default retro snippets & Project Echo clue
    AddClip(
        "/* KClip Win32 Sovereign Workspace v1.0 */\n"
        "#define CLIB_MAX 64\n"
        "void FlushClipboardHistory(void);\n",
        1, "1999-09-22 00:00"
    );

    AddClip(
        "kweb://portal/search?q=kiloos+hypermedia",
        1, "1999-09-22 01:15"
    );

    AddClip(
        "{\"system\":\"KiloOS\",\"version\":\"0.4.2\",\"node\":\"0x99\",\"status\":\"AUTHENTIC\"}",
        0, "1999-09-22 02:30"
    );

    AddClip(
        "#06b6d4",
        0, "1999-09-22 03:00"
    );

    AddClip(
        "// [PROJECT ECHO TRANSMISSION - NODE 0x99]\n"
        "// \"Carrier locked at 432 Hz. Node synchronization pending.\"\n"
        "// Memory Offset: 0x10199904\n",
        1, "1999-09-22 04:00"
    );
}

static void UpdateStatusBar(void) {
    if (!g_hStatusBar) return;
    char text[256];
    char numBuf[16];

    k_strcpy(text, "Clips: ");
    k_itoa(g_clipCount, numBuf);
    k_strcat(text, numBuf);
    k_strcat(text, " | Pinned: ");

    int pinned = 0;
    for (int i = 0; i < g_clipCount; i++) {
        if (g_clips[i].isPinned) pinned++;
    }
    k_itoa(pinned, numBuf);
    k_strcat(text, numBuf);

    if (g_selectedClip >= 0 && g_selectedClip < g_clipCount) {
        ClipItem* c = &g_clips[g_selectedClip];
        k_strcat(text, " | Active: [");
        k_strcat(text, c->category);
        k_strcat(text, "] ");
        k_itoa(c->charCount, numBuf);
        k_strcat(text, numBuf);
        k_strcat(text, " chars, ");
        k_itoa(c->lineCount, numBuf);
        k_strcat(text, numBuf);
        k_strcat(text, " lines");
    } else {
        k_strcat(text, " | Ready (F5: QuickSave, F9: QuickLoad, F1: Help)");
    }

    SetWindowTextA(g_hStatusBar, text);
}

static void UpdateUI(void) {
    if (!g_hListClips || !g_hEditPreview) return;

    SendMessageA(g_hListClips, LB_RESETCONTENT, 0, 0);

    for (int i = 0; i < g_clipCount; i++) {
        char itemStr[128];
        itemStr[0] = 0;
        if (g_clips[i].isPinned) {
            k_strcat(itemStr, "[*PIN] ");
        } else {
            k_strcat(itemStr, "       ");
        }
        k_strcat(itemStr, "[");
        k_strcat(itemStr, g_clips[i].category);
        k_strcat(itemStr, "] ");
        k_strcat(itemStr, g_clips[i].title);

        SendMessageA(g_hListClips, LB_ADDSTRING, 0, (LPARAM)itemStr);
    }

    if (g_selectedClip >= g_clipCount) g_selectedClip = g_clipCount - 1;
    if (g_selectedClip < 0 && g_clipCount > 0) g_selectedClip = 0;

    if (g_selectedClip >= 0 && g_selectedClip < g_clipCount) {
        SendMessageA(g_hListClips, LB_SETCURSEL, (WPARAM)g_selectedClip, 0);
        SetWindowTextA(g_hEditPreview, g_clips[g_selectedClip].content);
        if (g_hBtnPin) {
            SetWindowTextA(g_hBtnPin, g_clips[g_selectedClip].isPinned ? "Unpin [*]" : "Pin Clip");
        }
    } else {
        SetWindowTextA(g_hEditPreview, "");
        if (g_hBtnPin) SetWindowTextA(g_hBtnPin, "Pin Clip");
    }

    UpdateStatusBar();
}

static void CaptureSystemClipboard(void) {
    if (!OpenClipboard(g_hwnd)) {
        MessageBoxA(g_hwnd, "Unable to access Windows clipboard.", "KClip", MB_OK | MB_ICONWARNING);
        return;
    }
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData) {
        char* pszText = (char*)GlobalLock(hData);
        if (pszText && pszText[0]) {
            AddClip(pszText, 0, "Captured");
            g_selectedClip = 0;
        }
        GlobalUnlock(hData);
    } else {
        MessageBoxA(g_hwnd, "Clipboard is empty or does not contain text.", "KClip", MB_OK | MB_ICONINFORMATION);
    }
    CloseClipboard();
    UpdateUI();
}

static void CopySelectedToClipboard(void) {
    if (g_selectedClip < 0 || g_selectedClip >= g_clipCount) return;
    ClipItem* c = &g_clips[g_selectedClip];
    int len = k_strlen(c->content);
    if (len <= 0) return;

    if (!OpenClipboard(g_hwnd)) return;
    EmptyClipboard();

    HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, len + 1);
    if (hGlob) {
        char* pDst = (char*)GlobalLock(hGlob);
        if (pDst) {
            k_strcpy(pDst, c->content);
            GlobalUnlock(hGlob);
            SetClipboardData(CF_TEXT, hGlob);
        }
    }
    CloseClipboard();
    MessageBoxA(g_hwnd, "Copied selected clip to clipboard!", "KClip Sovereign", MB_OK | MB_ICONINFORMATION);
}

static void TogglePinSelected(void) {
    if (g_selectedClip < 0 || g_selectedClip >= g_clipCount) return;
    g_clips[g_selectedClip].isPinned = !g_clips[g_selectedClip].isPinned;
    UpdateUI();
}

static void DeleteSelected(void) {
    if (g_selectedClip < 0 || g_selectedClip >= g_clipCount) return;
    for (int i = g_selectedClip; i < g_clipCount - 1; i++) {
        g_clips[i] = g_clips[i + 1];
    }
    g_clipCount--;
    if (g_selectedClip >= g_clipCount) g_selectedClip = g_clipCount - 1;
    UpdateUI();
}

static void ClearHistory(void) {
    int writeIdx = 0;
    for (int i = 0; i < g_clipCount; i++) {
        if (g_clips[i].isPinned) {
            g_clips[writeIdx++] = g_clips[i];
        }
    }
    g_clipCount = writeIdx;
    g_selectedClip = 0;
    UpdateUI();
}

static void TransformSelectedUpper(void) {
    if (g_selectedClip < 0 || g_selectedClip >= g_clipCount) return;
    ClipItem* c = &g_clips[g_selectedClip];
    for (int i = 0; c->content[i]; i++) {
        if (c->content[i] >= 'a' && c->content[i] <= 'z') {
            c->content[i] -= 32;
        }
    }
    AnalyzeClip(c);
    UpdateUI();
}

static void TransformSelectedLower(void) {
    if (g_selectedClip < 0 || g_selectedClip >= g_clipCount) return;
    ClipItem* c = &g_clips[g_selectedClip];
    for (int i = 0; c->content[i]; i++) {
        if (c->content[i] >= 'A' && c->content[i] <= 'Z') {
            c->content[i] += 32;
        }
    }
    AnalyzeClip(c);
    UpdateUI();
}

static void TransformSelectedRot13(void) {
    if (g_selectedClip < 0 || g_selectedClip >= g_clipCount) return;
    ClipItem* c = &g_clips[g_selectedClip];
    for (int i = 0; c->content[i]; i++) {
        char ch = c->content[i];
        if (ch >= 'a' && ch <= 'z') {
            c->content[i] = (char)('a' + (ch - 'a' + 13) % 26);
        } else if (ch >= 'A' && ch <= 'Z') {
            c->content[i] = (char)('A' + (ch - 'A' + 13) % 26);
        }
    }
    AnalyzeClip(c);
    UpdateUI();
}

static void TransformSelectedTrim(void) {
    if (g_selectedClip < 0 || g_selectedClip >= g_clipCount) return;
    ClipItem* c = &g_clips[g_selectedClip];
    int start = 0;
    while (c->content[start] == ' ' || c->content[start] == '\t' || c->content[start] == '\r' || c->content[start] == '\n') {
        start++;
    }
    int end = k_strlen(c->content) - 1;
    while (end >= start && (c->content[end] == ' ' || c->content[end] == '\t' || c->content[end] == '\r' || c->content[end] == '\n')) {
        end--;
    }
    int len = 0;
    for (int i = start; i <= end; i++) {
        c->content[len++] = c->content[i];
    }
    c->content[len] = 0;
    AnalyzeClip(c);
    UpdateUI();
}

static void InsertSnippet(void) {
    AddClip(
        "/* Win32 Macro Template */\n"
        "LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);\n",
        0, "Template"
    );
    g_selectedClip = 0;
    UpdateUI();
}

static void ShowHelpDialog(void) {
    MessageBoxA(
        g_hwnd,
        "KClip - Retro Clipboard & Snippet Workstation v1.0.0\n"
        "====================================================\n\n"
        "FEATURES:\n"
        "- Clipboard Stack: Capture & manage multi-item clips\n"
        "- Pin Protection: Retain essential clips indefinitely\n"
        "- Text Transforms: UPPER, lower, ROT13, Trim\n"
        "- Keyboard Shortcuts:\n"
        "  * F1: Show this Help / Quick Guide\n"
        "  * F5: QuickSave current state (kclip.dat)\n"
        "  * F9: QuickLoad saved state\n"
        "  * Arrow Keys: Navigate clip history\n\n"
        "(C) 1999 KiloApps Autonomous Fleet - App #99 Milestone",
        "KClip Help & Reference",
        MB_OK | MB_ICONINFORMATION
    );
}

static void SaveState(void) {
    HANDLE hFile = CreateFileA(
        "kclip.dat",
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten;
        DWORD magic = 0x4B434C50; // "KCLP"
        WriteFile(hFile, &magic, sizeof(magic), &bytesWritten, NULL);
        WriteFile(hFile, &g_clipCount, sizeof(g_clipCount), &bytesWritten, NULL);
        WriteFile(hFile, &g_selectedClip, sizeof(g_selectedClip), &bytesWritten, NULL);
        WriteFile(hFile, g_clips, sizeof(ClipItem) * g_clipCount, &bytesWritten, NULL);
        CloseHandle(hFile);
        MessageBoxA(g_hwnd, "State quicksaved successfully to kclip.dat [F5]!", "KClip QuickSave", MB_OK | MB_ICONINFORMATION);
    }
}

static void LoadState(void) {
    HANDLE hFile = CreateFileA(
        "kclip.dat",
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesRead;
        DWORD magic = 0;
        ReadFile(hFile, &magic, sizeof(magic), &bytesRead, NULL);
        if (magic == 0x4B434C50) {
            ReadFile(hFile, &g_clipCount, sizeof(g_clipCount), &bytesRead, NULL);
            ReadFile(hFile, &g_selectedClip, sizeof(g_selectedClip), &bytesRead, NULL);
            ReadFile(hFile, g_clips, sizeof(ClipItem) * g_clipCount, &bytesRead, NULL);
            CloseHandle(hFile);
            UpdateUI();
            MessageBoxA(g_hwnd, "State quickloaded successfully [F9]!", "KClip QuickLoad", MB_OK | MB_ICONINFORMATION);
            return;
        }
        CloseHandle(hFile);
    }
    MessageBoxA(g_hwnd, "No valid kclip.dat save file found.", "KClip QuickLoad", MB_OK | MB_ICONWARNING);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;

            g_hBrushBg = CreateSolidBrush(COLOR_BG);
            g_hBrushCard = CreateSolidBrush(COLOR_CARD);
            g_hBrushList = CreateSolidBrush(RGB(18, 26, 43));

            g_hFontTitle = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            g_hFontNormal = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            g_hFontMono = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

            // Top Command Bar
            g_hBtnCapture = CreateWindowA("BUTTON", "Capture Clipboard", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 12, 10, 130, 28, hwnd, (HMENU)ID_BTN_CAPTURE, hInst, NULL);
            g_hBtnCopy    = CreateWindowA("BUTTON", "Copy to Clip",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 148, 10, 96, 28, hwnd, (HMENU)ID_BTN_COPY, hInst, NULL);
            g_hBtnPin     = CreateWindowA("BUTTON", "Pin Clip",       WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 250, 10, 80, 28, hwnd, (HMENU)ID_BTN_PIN, hInst, NULL);
            g_hBtnDelete  = CreateWindowA("BUTTON", "Delete",         WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 336, 10, 70, 28, hwnd, (HMENU)ID_BTN_DELETE, hInst, NULL);
            g_hBtnClear   = CreateWindowA("BUTTON", "Clear Unpinned", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 412, 10, 105, 28, hwnd, (HMENU)ID_BTN_CLEAR, hInst, NULL);

            g_hBtnSave    = CreateWindowA("BUTTON", "Save [F5]",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 535, 10, 75, 28, hwnd, (HMENU)ID_BTN_SAVE, hInst, NULL);
            g_hBtnLoad    = CreateWindowA("BUTTON", "Load [F9]",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 616, 10, 75, 28, hwnd, (HMENU)ID_BTN_LOAD, hInst, NULL);
            g_hBtnHelp    = CreateWindowA("BUTTON", "Help [F1]",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 697, 10, 75, 28, hwnd, (HMENU)ID_BTN_HELP, hInst, NULL);

            // Left Pane: Clips ListBox
            g_hListClips = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 12, 48, 320, 500, hwnd, (HMENU)ID_LIST_CLIPS, hInst, NULL);
            SendMessageA(g_hListClips, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            // Right Pane: Multiline Text Viewer / Editor
            g_hEditPreview = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, 342, 48, 560, 460, hwnd, (HMENU)ID_EDIT_PREVIEW, hInst, NULL);
            SendMessageA(g_hEditPreview, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

            // Transform action buttons under preview
            g_hBtnUpper   = CreateWindowA("BUTTON", "UPPERCASE", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 342, 516, 95, 28, hwnd, (HMENU)ID_BTN_UPPER, hInst, NULL);
            g_hBtnLower   = CreateWindowA("BUTTON", "lowercase", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 443, 516, 95, 28, hwnd, (HMENU)ID_BTN_LOWER, hInst, NULL);
            g_hBtnRot13   = CreateWindowA("BUTTON", "ROT13",     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 544, 516, 75, 28, hwnd, (HMENU)ID_BTN_ROT13, hInst, NULL);
            g_hBtnTrim    = CreateWindowA("BUTTON", "Trim",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 625, 516, 65, 28, hwnd, (HMENU)ID_BTN_TRIM, hInst, NULL);
            g_hBtnSnippet = CreateWindowA("BUTTON", "+ Template",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 696, 516, 95, 28, hwnd, (HMENU)ID_BTN_SNIPPET, hInst, NULL);

            // Status Bar at Bottom
            g_hStatusBar = CreateWindowA("STATIC", "Ready", WS_CHILD | WS_VISIBLE | SS_LEFT, 12, 558, 890, 22, hwnd, (HMENU)ID_STATUS_BAR, hInst, NULL);
            SendMessageA(g_hStatusBar, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            InitDefaultClips();
            UpdateUI();
            return 0;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (wmId == ID_LIST_CLIPS && wmEvent == LBN_SELCHANGE) {
                int sel = (int)SendMessageA(g_hListClips, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR && sel >= 0 && sel < g_clipCount) {
                    g_selectedClip = sel;
                    SetWindowTextA(g_hEditPreview, g_clips[sel].content);
                    if (g_hBtnPin) {
                        SetWindowTextA(g_hBtnPin, g_clips[sel].isPinned ? "Unpin [*]" : "Pin Clip");
                    }
                    UpdateStatusBar();
                }
            } else if (wmId == ID_BTN_CAPTURE) {
                CaptureSystemClipboard();
            } else if (wmId == ID_BTN_COPY) {
                CopySelectedToClipboard();
            } else if (wmId == ID_BTN_PIN) {
                TogglePinSelected();
            } else if (wmId == ID_BTN_DELETE) {
                DeleteSelected();
            } else if (wmId == ID_BTN_CLEAR) {
                ClearHistory();
            } else if (wmId == ID_BTN_UPPER) {
                TransformSelectedUpper();
            } else if (wmId == ID_BTN_LOWER) {
                TransformSelectedLower();
            } else if (wmId == ID_BTN_ROT13) {
                TransformSelectedRot13();
            } else if (wmId == ID_BTN_TRIM) {
                TransformSelectedTrim();
            } else if (wmId == ID_BTN_SNIPPET) {
                InsertSnippet();
            } else if (wmId == ID_BTN_HELP) {
                ShowHelpDialog();
            } else if (wmId == ID_BTN_SAVE) {
                SaveState();
            } else if (wmId == ID_BTN_LOAD) {
                LoadState();
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
    wc.lpszClassName = "KClipMainWindowClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(1));
    RegisterClassA(&wc);

    g_hwnd = CreateWindowExA(
        0,
        "KClipMainWindowClass",
        "KClip - Clipboard History & Snippet Tool v1.0.0",
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
