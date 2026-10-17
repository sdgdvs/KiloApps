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
#define ID_BTN_CAMEL        117
#define ID_BTN_SNAKE        118
#define ID_BTN_DEDUP        119
#define ID_BTN_ANALYTICS    120
#define ID_BTN_EXPORT_TXT   121
#define ID_BTN_IMPORT_TXT   122

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
static HWND g_hBtnCamel = NULL;
static HWND g_hBtnSnake = NULL;
static HWND g_hBtnDedup = NULL;
static HWND g_hBtnAnalytics = NULL;
static HWND g_hBtnExport = NULL;
static HWND g_hBtnImport = NULL;

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

static void SyncEditBufferToClip(void) {
    if (g_selectedClip < 0 || g_selectedClip >= g_clipCount || !g_hEditPreview) return;
    char buf[MAX_CLIP_TEXT];
    GetWindowTextA(g_hEditPreview, buf, MAX_CLIP_TEXT);
    k_strcpy(g_clips[g_selectedClip].content, buf);
    AnalyzeClip(&g_clips[g_selectedClip]);
}

static void CopySelectedToClipboard(void) {
    SyncEditBufferToClip();
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
    if (g_hStatusBar) {
        SetWindowTextA(g_hStatusBar, "Copied selected clip to clipboard! [*]");
    }
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
    SyncEditBufferToClip();
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
    SyncEditBufferToClip();
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
    SyncEditBufferToClip();
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
    SyncEditBufferToClip();
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

static void TransformSelectedCamel(void) {
    SyncEditBufferToClip();
    if (g_selectedClip < 0 || g_selectedClip >= g_clipCount) return;
    ClipItem* c = &g_clips[g_selectedClip];
    char temp[MAX_CLIP_TEXT];
    int r = 0, w = 0;
    int capitalizeNext = 0;
    int firstWord = 1;

    while (c->content[r] && (c->content[r] == ' ' || c->content[r] == '_' || c->content[r] == '-' || c->content[r] == '\t')) {
        r++;
    }

    while (c->content[r] && w < MAX_CLIP_TEXT - 1) {
        char ch = c->content[r++];
        if (ch == ' ' || ch == '_' || ch == '-' || ch == '\t' || ch == '\r' || ch == '\n') {
            capitalizeNext = 1;
        } else {
            if (capitalizeNext) {
                if (ch >= 'a' && ch <= 'z') ch -= 32;
                capitalizeNext = 0;
            } else if (firstWord) {
                if (ch >= 'A' && ch <= 'Z') ch += 32;
            }
            firstWord = 0;
            temp[w++] = ch;
        }
    }
    temp[w] = 0;
    k_strcpy(c->content, temp);
    AnalyzeClip(c);
    UpdateUI();
}

static void TransformSelectedSnake(void) {
    SyncEditBufferToClip();
    if (g_selectedClip < 0 || g_selectedClip >= g_clipCount) return;
    ClipItem* c = &g_clips[g_selectedClip];
    char temp[MAX_CLIP_TEXT];
    int r = 0, w = 0;
    int lastWasSep = 1;

    while (c->content[r] && w < MAX_CLIP_TEXT - 2) {
        char ch = c->content[r++];
        if (ch == ' ' || ch == '-' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '_') {
            if (!lastWasSep && w > 0) {
                temp[w++] = '_';
                lastWasSep = 1;
            }
        } else {
            if (ch >= 'A' && ch <= 'Z') {
                if (!lastWasSep && w > 0 && temp[w - 1] != '_') {
                    temp[w++] = '_';
                }
                temp[w++] = (char)(ch + 32);
            } else {
                temp[w++] = ch;
            }
            lastWasSep = 0;
        }
    }
    if (w > 0 && temp[w - 1] == '_') w--;
    temp[w] = 0;
    k_strcpy(c->content, temp);
    AnalyzeClip(c);
    UpdateUI();
}

static void TransformSelectedDedup(void) {
    SyncEditBufferToClip();
    if (g_selectedClip < 0 || g_selectedClip >= g_clipCount) return;
    ClipItem* c = &g_clips[g_selectedClip];
    char temp[MAX_CLIP_TEXT];
    char lineBuf[64][128];
    int numLines = 0;
    int r = 0;

    while (c->content[r] && numLines < 64) {
        int col = 0;
        while (c->content[r] && c->content[r] != '\n' && c->content[r] != '\r' && col < 127) {
            lineBuf[numLines][col++] = c->content[r++];
        }
        lineBuf[numLines][col] = 0;
        if (c->content[r] == '\r') r++;
        if (c->content[r] == '\n') r++;

        int isDup = 0;
        for (int i = 0; i < numLines; i++) {
            if (k_strcmp(lineBuf[i], lineBuf[numLines]) == 0) {
                isDup = 1;
                break;
            }
        }
        if (!isDup) {
            numLines++;
        }
    }

    int w = 0;
    for (int i = 0; i < numLines; i++) {
        int l = k_strlen(lineBuf[i]);
        if (w + l + 3 >= MAX_CLIP_TEXT) break;
        k_strcpy(&temp[w], lineBuf[i]);
        w += l;
        if (i < numLines - 1) {
            temp[w++] = '\r';
            temp[w++] = '\n';
        }
    }
    temp[w] = 0;
    k_strcpy(c->content, temp);
    AnalyzeClip(c);
    UpdateUI();
}

static unsigned int CalcCRC32(const char* data, int len) {
    unsigned int crc = 0xFFFFFFFF;
    for (int i = 0; i < len; i++) {
        unsigned char b = (unsigned char)data[i];
        crc ^= b;
        for (int j = 0; j < 8; j++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
            else crc = crc >> 1;
        }
    }
    return ~crc;
}

static void ShowBufferAnalytics(void) {
    SyncEditBufferToClip();
    if (g_selectedClip < 0 || g_selectedClip >= g_clipCount) {
        MessageBoxA(g_hwnd, "No clip selected for diagnostic analytics.", "KClip Analytics", MB_OK | MB_ICONINFORMATION);
        return;
    }
    ClipItem* c = &g_clips[g_selectedClip];
    int len = k_strlen(c->content);
    int words = 0;
    int inWord = 0;
    int upperCount = 0, lowerCount = 0, digitCount = 0, spaceCount = 0;

    for (int i = 0; i < len; i++) {
        char ch = c->content[i];
        if (ch >= 'A' && ch <= 'Z') upperCount++;
        else if (ch >= 'a' && ch <= 'z') lowerCount++;
        else if (ch >= '0' && ch <= '9') digitCount++;
        else if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') spaceCount++;

        if (ch > 32) {
            if (!inWord) { words++; inWord = 1; }
        } else {
            inWord = 0;
        }
    }

    unsigned int crc = CalcCRC32(c->content, len);

    char msgBuf[512];
    wsprintfA(msgBuf,
        "CLIP BUFFER TELEMETRY & DIAGNOSTICS\n"
        "====================================\n"
        "Title: %s\n"
        "Category: %s | Pinned: %s\n"
        "Timestamp: %s\n\n"
        "METRICS:\n"
        "- Characters : %d\n"
        "- Words      : %d\n"
        "- Lines      : %d\n"
        "- Uppercase  : %d\n"
        "- Lowercase  : %d\n"
        "- Digits     : %d\n"
        "- Whitespace : %d\n\n"
        "INTEGRITY:\n"
        "- CRC-32 Checksum : 0x%08X\n"
        "- Buffer Capacity : %d / %d bytes",
        c->title,
        c->category,
        c->isPinned ? "YES" : "NO",
        c->timestamp,
        len, words, c->lineCount,
        upperCount, lowerCount, digitCount, spaceCount,
        crc,
        len, MAX_CLIP_TEXT
    );
    MessageBoxA(g_hwnd, msgBuf, "KClip Diagnostics & Analytics", MB_OK | MB_ICONINFORMATION);
}

static void ExportArchiveTxt(void) {
    SyncEditBufferToClip();
    char szFile[MAX_PATH] = "kclip_archive.txt";
    OPENFILENAMEA ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hwnd;
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = "txt";

    if (GetSaveFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written;
            const char* header = "# KClip Sovereign Workstation Text Archive\r\n# Version: 1.0.0 (Win32 Sovereign Build)\r\n\r\n";
            WriteFile(hFile, header, k_strlen(header), &written, NULL);
            for (int i = 0; i < g_clipCount; i++) {
                char banner[128];
                wsprintfA(banner, "=== CLIP %d [%s] %s ===\r\n", i + 1, g_clips[i].category, g_clips[i].isPinned ? "[PINNED]" : "");
                WriteFile(hFile, banner, k_strlen(banner), &written, NULL);
                WriteFile(hFile, g_clips[i].content, k_strlen(g_clips[i].content), &written, NULL);
                WriteFile(hFile, "\r\n\r\n", 4, &written, NULL);
            }
            CloseHandle(hFile);
            MessageBoxA(g_hwnd, "Archive exported successfully to text file!", "KClip Export", MB_OK | MB_ICONINFORMATION);
        }
    }
}

static void ImportArchiveTxt(void) {
    char szFile[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hwnd;
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(szFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD fileSize = GetFileSize(hFile, NULL);
            if (fileSize > 0 && fileSize < 65536) {
                char* buf = (char*)GlobalAlloc(GPTR, fileSize + 1);
                if (buf) {
                    DWORD readBytes = 0;
                    ReadFile(hFile, buf, fileSize, &readBytes, NULL);
                    buf[readBytes] = 0;

                    int imported = 0;
                    for (DWORD i = 0; i < readBytes && imported < 16; i++) {
                        if (buf[i] == '=' && buf[i+1] == '=' && buf[i+2] == '=') {
                            while (i < readBytes && buf[i] != '\n') i++;
                            DWORD start = i + 1;
                            DWORD nextSep = start;
                            while (nextSep < readBytes) {
                                if (buf[nextSep] == '=' && buf[nextSep+1] == '=' && buf[nextSep+2] == '=') break;
                                nextSep++;
                            }
                            int clipLen = (int)(nextSep - start);
                            if (clipLen > 0 && clipLen < MAX_CLIP_TEXT) {
                                char clipData[MAX_CLIP_TEXT];
                                memcpy(clipData, &buf[start], clipLen);
                                clipData[clipLen] = 0;
                                while (clipLen > 0 && (clipData[clipLen-1] == '\r' || clipData[clipLen-1] == '\n' || clipData[clipLen-1] == ' ')) {
                                    clipData[--clipLen] = 0;
                                }
                                if (clipLen > 0) {
                                    AddClip(clipData, 0, "Imported");
                                    imported++;
                                }
                            }
                            i = nextSep - 1;
                        }
                    }
                    GlobalFree(buf);
                    CloseHandle(hFile);
                    if (imported > 0) {
                        g_selectedClip = 0;
                        UpdateUI();
                        char resMsg[64];
                        wsprintfA(resMsg, "Imported %d clips successfully!", imported);
                        MessageBoxA(g_hwnd, resMsg, "KClip Import", MB_OK | MB_ICONINFORMATION);
                        return;
                    }
                }
            }
            CloseHandle(hFile);
        }
        MessageBoxA(g_hwnd, "No valid '=== CLIP ===' delimited blocks found.", "KClip Import", MB_OK | MB_ICONWARNING);
    }
}

static int g_snippetCounter = 0;
static void InsertSnippet(void) {
    static const char* s_templates[] = {
        "/* Win32 Macro Template */\n"
        "LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);\n",

        "<!-- Retro 1999 HTML Skeleton -->\n"
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head><title>KiloOS Workspace</title></head>\n"
        "<body bgcolor=\"#0b0f19\" text=\"#f1f5f9\">\n"
        "  <h1>KClip Hypermedia</h1>\n"
        "</body>\n"
        "</html>\n",

        "/* SQL Data Definition */\n"
        "CREATE TABLE kclip_registry (\n"
        "  id INT PRIMARY KEY,\n"
        "  hash VARCHAR(32),\n"
        "  payload TEXT,\n"
        "  pinned TINYINT DEFAULT 0\n"
        ");\n",

        "// REST JSON Specification\n"
        "{\n"
        "  \"protocol\": \"KNet/1.0\",\n"
        "  \"status\": 200,\n"
        "  \"node\": \"0x99\",\n"
        "  \"verified\": true\n"
        "}\n",

        "/* Project Echo Telemetry Frame */\n"
        "// Carrier Frequency: 432 Hz\n"
        "// Sector: 0x10199904 (Node B)\n"
        "// Status: SYNCHRONIZED\n"
    };
    int numTemplates = 5;
    AddClip(s_templates[g_snippetCounter % numTemplates], 0, "Template");
    g_snippetCounter++;
    g_selectedClip = 0;
    UpdateUI();
}

static void ShowHelpDialog(void) {
    MessageBoxA(
        g_hwnd,
        "KClip - Retro Clipboard & Snippet Workstation v1.0.0\n"
        "====================================================\n\n"
        "FEATURES:\n"
        "- Multi-item Clipboard Stack & Filtered Clips\n"
        "- Pin Protection: Retain essential clips indefinitely\n"
        "- Case Transforms: UPPER, lower, camelCase, snake_case\n"
        "- Line Tools: Trim whitespace, ROT13, Deduplicate lines\n"
        "- Template Library: C Win32, HTML5, SQL, JSON, Telemetry\n"
        "- Buffer Telemetry: CRC-32 checksums & character analytics\n"
        "- Interoperability: Export/Import Delimited Plain Text Archives\n\n"
        "KEYBOARD SHORTCUTS:\n"
        "  * F1 / H : Show this Help & Reference Guide\n"
        "  * F2     : Buffer Telemetry & CRC-32 Analytics\n"
        "  * F5     : Sovereign QuickSave State (kclip.dat)\n"
        "  * F9     : Sovereign QuickLoad State (kclip.dat)\n"
        "  * F6     : Export Library to Plain Text Archive (.txt)\n"
        "  * F7     : Import Clips from Delimited Archive (.txt)\n"
        "  * C      : Copy selected clip to Windows clipboard\n"
        "  * P      : Toggle Pin status on selected clip\n"
        "  * Del    : Delete selected clip\n"
        "  * 1 - 9  : Jump to clips 1 through 9 in stack\n"
        "  * Esc    : Close Application\n\n"
        "(C) 1999 KiloOS Systems Suite - App #99 Milestone",
        "KClip Help & Reference",
        MB_OK | MB_ICONINFORMATION
    );
}

static void SaveState(void) {
    SyncEditBufferToClip();
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
        DWORD bytesRead = 0;
        DWORD magic = 0;
        ReadFile(hFile, &magic, sizeof(magic), &bytesRead, NULL);
        if (magic == 0x4B434C50) {
            int clipCount = 0;
            int selectedClip = 0;
            ReadFile(hFile, &clipCount, sizeof(clipCount), &bytesRead, NULL);
            ReadFile(hFile, &selectedClip, sizeof(selectedClip), &bytesRead, NULL);
            if (clipCount >= 0 && clipCount <= MAX_CLIPS) {
                g_clipCount = clipCount;
                ReadFile(hFile, g_clips, sizeof(ClipItem) * g_clipCount, &bytesRead, NULL);
                if (selectedClip >= 0 && selectedClip < g_clipCount) {
                    g_selectedClip = selectedClip;
                } else {
                    g_selectedClip = (g_clipCount > 0) ? 0 : -1;
                }
                CloseHandle(hFile);
                UpdateUI();
                MessageBoxA(g_hwnd, "State quickloaded successfully from kclip.dat [F9]!", "KClip QuickLoad", MB_OK | MB_ICONINFORMATION);
                return;
            }
        }
        CloseHandle(hFile);
    }
    MessageBoxA(g_hwnd, "No valid kclip.dat save file found. Press F5 to save first.", "KClip QuickLoad", MB_OK | MB_ICONWARNING);
}

static void CheckFirstRunTutorial(HWND hwnd) {
    DWORD attribState = GetFileAttributesA("kclip.dat");
    if (attribState != INVALID_FILE_ATTRIBUTES) return;

    DWORD attribTut = GetFileAttributesA("kclip_tutorial.dat");
    if (attribTut != INVALID_FILE_ATTRIBUTES) return;

    MessageBoxA(hwnd,
        "Welcome to KClip - Retro Clipboard & Snippet Workstation v1.0.0!\n\n"
        "Features:\n"
        "- Left Pane: Multi-item Clipboard Stack & Filtered Clips\n"
        "- Right Pane: Live Text Inspector, Hex Viewer & Transforms\n"
        "- Pin Protection: Retain essential clips indefinitely\n\n"
        "Keyboard Shortcuts:\n"
        "- F1: Show Help & Keybindings Guide\n"
        "- F5: QuickSave State (kclip.dat)\n"
        "- F9: QuickLoad State (kclip.dat)\n"
        "- Esc: Exit Application\n\n"
        "Press OK to enter the workstation.",
        "KClip - Quick-Start Onboarding",
        MB_OK | MB_ICONINFORMATION);

    HANDLE hTut = CreateFileA("kclip_tutorial.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hTut != INVALID_HANDLE_VALUE) {
        char val = 1;
        DWORD written = 0;
        WriteFile(hTut, &val, 1, &written, NULL);
        CloseHandle(hTut);
    }
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

            // Top Command Bar (Y=10)
            g_hBtnCapture   = CreateWindowA("BUTTON", "Capture",        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 12, 10, 80, 26, hwnd, (HMENU)ID_BTN_CAPTURE, hInst, NULL);
            g_hBtnCopy      = CreateWindowA("BUTTON", "Copy [C]",       WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 96, 10, 75, 26, hwnd, (HMENU)ID_BTN_COPY, hInst, NULL);
            g_hBtnPin       = CreateWindowA("BUTTON", "Pin Clip",       WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 175, 10, 75, 26, hwnd, (HMENU)ID_BTN_PIN, hInst, NULL);
            g_hBtnDelete    = CreateWindowA("BUTTON", "Delete",         WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 254, 10, 60, 26, hwnd, (HMENU)ID_BTN_DELETE, hInst, NULL);
            g_hBtnClear     = CreateWindowA("BUTTON", "Clear Free",     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 318, 10, 75, 26, hwnd, (HMENU)ID_BTN_CLEAR, hInst, NULL);
            g_hBtnExport    = CreateWindowA("BUTTON", "Export [F6]",    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 397, 10, 85, 26, hwnd, (HMENU)ID_BTN_EXPORT_TXT, hInst, NULL);
            g_hBtnImport    = CreateWindowA("BUTTON", "Import [F7]",    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 486, 10, 85, 26, hwnd, (HMENU)ID_BTN_IMPORT_TXT, hInst, NULL);
            g_hBtnSave      = CreateWindowA("BUTTON", "Save [F5]",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 575, 10, 75, 26, hwnd, (HMENU)ID_BTN_SAVE, hInst, NULL);
            g_hBtnLoad      = CreateWindowA("BUTTON", "Load [F9]",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 654, 10, 75, 26, hwnd, (HMENU)ID_BTN_LOAD, hInst, NULL);
            g_hBtnAnalytics = CreateWindowA("BUTTON", "Stats [F2]",     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 733, 10, 80, 26, hwnd, (HMENU)ID_BTN_ANALYTICS, hInst, NULL);
            g_hBtnHelp      = CreateWindowA("BUTTON", "Help [F1]",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 817, 10, 80, 26, hwnd, (HMENU)ID_BTN_HELP, hInst, NULL);

            // Left Pane: Clips ListBox
            g_hListClips = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 12, 44, 315, 500, hwnd, (HMENU)ID_LIST_CLIPS, hInst, NULL);
            SendMessageA(g_hListClips, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            // Right Pane: Multiline Text Viewer / Editor
            g_hEditPreview = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, 335, 44, 575, 460, hwnd, (HMENU)ID_EDIT_PREVIEW, hInst, NULL);
            SendMessageA(g_hEditPreview, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

            // Transform action buttons under preview (Y=514)
            g_hBtnUpper   = CreateWindowA("BUTTON", "UPPER",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 335, 514, 60, 26, hwnd, (HMENU)ID_BTN_UPPER, hInst, NULL);
            g_hBtnLower   = CreateWindowA("BUTTON", "lower",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 399, 514, 60, 26, hwnd, (HMENU)ID_BTN_LOWER, hInst, NULL);
            g_hBtnRot13   = CreateWindowA("BUTTON", "ROT13",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 463, 514, 55, 26, hwnd, (HMENU)ID_BTN_ROT13, hInst, NULL);
            g_hBtnTrim    = CreateWindowA("BUTTON", "Trim",       WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 522, 514, 50, 26, hwnd, (HMENU)ID_BTN_TRIM, hInst, NULL);
            g_hBtnCamel   = CreateWindowA("BUTTON", "camelCase",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 576, 514, 75, 26, hwnd, (HMENU)ID_BTN_CAMEL, hInst, NULL);
            g_hBtnSnake   = CreateWindowA("BUTTON", "snake_case", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 655, 514, 75, 26, hwnd, (HMENU)ID_BTN_SNAKE, hInst, NULL);
            g_hBtnDedup   = CreateWindowA("BUTTON", "Dedup",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 734, 514, 55, 26, hwnd, (HMENU)ID_BTN_DEDUP, hInst, NULL);
            g_hBtnSnippet = CreateWindowA("BUTTON", "+ Template", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 793, 514, 85, 26, hwnd, (HMENU)ID_BTN_SNIPPET, hInst, NULL);

            // Set font on all buttons
            HWND allBtns[] = {
                g_hBtnCapture, g_hBtnCopy, g_hBtnPin, g_hBtnDelete, g_hBtnClear,
                g_hBtnExport, g_hBtnImport, g_hBtnSave, g_hBtnLoad, g_hBtnAnalytics, g_hBtnHelp,
                g_hBtnUpper, g_hBtnLower, g_hBtnRot13, g_hBtnTrim, g_hBtnCamel, g_hBtnSnake, g_hBtnDedup, g_hBtnSnippet
            };
            for (int b = 0; b < 19; b++) {
                if (allBtns[b]) SendMessageA(allBtns[b], WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            }

            // Status Bar at Bottom
            g_hStatusBar = CreateWindowA("STATIC", "Ready", WS_CHILD | WS_VISIBLE | SS_LEFT, 12, 550, 890, 22, hwnd, (HMENU)ID_STATUS_BAR, hInst, NULL);
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
            } else if (wmId == ID_BTN_CAMEL) {
                TransformSelectedCamel();
            } else if (wmId == ID_BTN_SNAKE) {
                TransformSelectedSnake();
            } else if (wmId == ID_BTN_DEDUP) {
                TransformSelectedDedup();
            } else if (wmId == ID_BTN_ANALYTICS) {
                ShowBufferAnalytics();
            } else if (wmId == ID_BTN_EXPORT_TXT) {
                ExportArchiveTxt();
            } else if (wmId == ID_BTN_IMPORT_TXT) {
                ImportArchiveTxt();
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
            } else if (wParam == VK_F2) {
                ShowBufferAnalytics();
                return 0;
            } else if (wParam == VK_F5) {
                SaveState();
                return 0;
            } else if (wParam == VK_F6) {
                ExportArchiveTxt();
                return 0;
            } else if (wParam == VK_F7) {
                ImportArchiveTxt();
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

    CheckFirstRunTutorial(g_hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            HWND hFocus = GetFocus();
            if (hFocus != g_hEditPreview) {
                if (msg.wParam == 'H' || msg.wParam == 'h') {
                    ShowHelpDialog();
                    continue;
                } else if (msg.wParam == 'C' || msg.wParam == 'c') {
                    CopySelectedToClipboard();
                    continue;
                } else if (msg.wParam == 'P' || msg.wParam == 'p') {
                    TogglePinSelected();
                    continue;
                } else if (msg.wParam == VK_DELETE) {
                    DeleteSelected();
                    continue;
                } else if (msg.wParam >= '1' && msg.wParam <= '9') {
                    int idx = (int)(msg.wParam - '1');
                    if (idx < g_clipCount) {
                        g_selectedClip = idx;
                        UpdateUI();
                    }
                    continue;
                }
            }
            if (msg.wParam == VK_F1) {
                ShowHelpDialog();
                continue;
            } else if (msg.wParam == VK_F2) {
                ShowBufferAnalytics();
                continue;
            } else if (msg.wParam == VK_F5) {
                SaveState();
                continue;
            } else if (msg.wParam == VK_F6) {
                ExportArchiveTxt();
                continue;
            } else if (msg.wParam == VK_F7) {
                ImportArchiveTxt();
                continue;
            } else if (msg.wParam == VK_F9) {
                LoadState();
                continue;
            } else if (msg.wParam == VK_ESCAPE) {
                PostMessage(g_hwnd, WM_CLOSE, 0, 0);
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    ExitProcess(0);
}
