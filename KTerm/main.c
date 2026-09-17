#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#define IDC_TAB          100
#define IDC_OUT          101
#define IDC_IN           102
#define IDC_PROMPT       103
#define IDC_STATUS       104
#define IDC_BTN_NEWTAB   105
#define IDC_BTN_CLOSETAB 106
#define IDC_BTN_CLEAR    107
#define IDC_BTN_EXPORT   108
#define IDC_BTN_THEME    109
#define IDC_BTN_HELP     110

#define MAX_TABS 8
#define MAX_HISTORY 50
#define MAX_ALIASES 25
#define MAX_ENV 25
#define OUT_BUF_SIZE 262144

#pragma function(memset)
void* memset(void* dest, int c, size_t count) {
    unsigned char* p = (unsigned char*)dest;
    while (count--) {
        *p++ = (unsigned char)c;
    }
    return dest;
}

#pragma function(memcpy)
void* memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}

static char* my_strchr(const char* s, int c) {
    if (!s) return NULL;
    while (*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return (c == 0) ? (char*)s : NULL;
}

static char* my_strrchr(const char* s, int c) {
    if (!s) return NULL;
    const char* last = NULL;
    while (*s) {
        if (*s == (char)c) last = s;
        s++;
    }
    if (c == 0) return (char*)s;
    return (char*)last;
}

static char* my_strstr(const char* haystack, const char* needle) {
    if (!haystack || !needle) return NULL;
    if (!*needle) return (char*)haystack;
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }
        if (!*n) return (char*)haystack;
        haystack++;
    }
    return NULL;
}

static char* my_strstri(const char* haystack, const char* needle) {
    if (!haystack || !needle) return NULL;
    if (!*needle) return (char*)haystack;
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n) {
            char c1 = *h;
            char c2 = *n;
            if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
            if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
            if (c1 != c2) break;
            h++;
            n++;
        }
        if (!*n) return (char*)haystack;
        haystack++;
    }
    return NULL;
}

static char* my_strcat(char* dest, const char* src) {
    if (!dest || !src) return dest;
    char* p = dest + lstrlenA(dest);
    while ((*p++ = *src++) != 0);
    return dest;
}

static char* my_strncat(char* dest, const char* src, size_t n) {
    if (!dest || !src) return dest;
    char* p = dest + lstrlenA(dest);
    while (n > 0 && *src) {
        *p++ = *src++;
        n--;
    }
    *p = '\0';
    return dest;
}

static int my_atoi(const char* s) {
    if (!s) return 0;
    while (*s == ' ' || *s == '\t') s++;
    int sign = 1;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') { s++; }
    int val = 0;
    while (*s >= '0' && *s <= '9') {
        val = val * 10 + (*s - '0');
        s++;
    }
    return sign * val;
}

typedef struct {
    char name[64];
    char cmd[256];
} Alias;

typedef struct {
    char name[64];
    char value[256];
} EnvVar;

typedef struct {
    char title[64];
    char currentDir[MAX_PATH];
    char history[MAX_HISTORY][256];
    int history_count;
    int history_pos;
    Alias aliases[MAX_ALIASES];
    int alias_count;
    EnvVar envVars[MAX_ENV];
    int env_count;
    char* outputBuffer;
} TabSession;

typedef struct {
    const char* name;
    COLORREF text;
    COLORREF prompt;
    COLORREF bg;
    COLORREF statusBg;
    COLORREF statusText;
} TermTheme;

static TermTheme g_themes[] = {
    { "green",   RGB(0, 255, 102),   RGB(0, 217, 255),   RGB(9, 11, 16),    RGB(18, 22, 32),  RGB(148, 163, 184) },
    { "amber",   RGB(255, 176, 0),   RGB(255, 215, 0),   RGB(20, 14, 4),    RGB(36, 24, 8),   RGB(212, 175, 55) },
    { "cyan",    RGB(0, 229, 255),   RGB(100, 255, 218), RGB(6, 16, 26),    RGB(12, 28, 44),  RGB(140, 210, 240) },
    { "white",   RGB(224, 224, 228), RGB(160, 200, 255), RGB(16, 16, 20),   RGB(28, 28, 36),  RGB(180, 180, 190) },
    { "crimson", RGB(255, 68, 85),   RGB(255, 140, 100), RGB(24, 8, 10),    RGB(42, 14, 18),  RGB(220, 130, 140) },
    { "purple",  RGB(224, 112, 255), RGB(255, 128, 220), RGB(20, 8, 28),    RGB(36, 16, 48),  RGB(210, 150, 230) }
};
static int g_currentTheme = 0;

HWND hTab, hOut, hIn, hPrompt, hStatus;
HWND hBtnNewTab, hBtnCloseTab, hBtnClear, hBtnExport, hBtnTheme, hBtnHelp;
HWND g_hMainWnd = NULL;
WNDPROC oldEditProc;
WNDPROC oldOutProc;
HFONT g_hFont = NULL;
HFONT g_hTabFont = NULL;
HBRUSH g_hBgBrush = NULL;
HBRUSH g_hStatusBrush = NULL;

HANDLE g_hRedirFile = INVALID_HANDLE_VALUE;
BOOL g_isRedirecting = FALSE;
int g_totalCommands = 0;
DWORD g_startTime = 0;
int g_scriptDepth = 0;

void ShowHelpDialog(HWND hwnd);
void UpdatePromptDisplay();
void SetStatusFeedback(const char* msg);
void UpdateStatusDisplay();
void UpdateAppTitle();
void ApplyTheme(int themeIdx);
void ProcessCommandLine(const char* fullLine);
void ProcessSingleCommand(const char* rawCmd);

TabSession g_tabs[MAX_TABS];
int g_tabCount = 0;
int g_activeTab = 0;

char g_statusMsg[128] = "";
DWORD g_statusExpiry = 0;

// Reverse Search State
BOOL g_isSearchMode = FALSE;
char g_searchQuery[128];
char g_searchMatch[256];
int g_searchMatchIndex = -1;
char g_savedInput[256];

#define MAX_MACROS 10
#define MAX_MACRO_CMDS 20

typedef struct {
    char name[64];
    char commands[MAX_MACRO_CMDS][256];
    int cmd_count;
} Macro;

Macro g_macros[MAX_MACROS];
int g_macroCount = 0;
BOOL g_isRecording = FALSE;
int g_recordingMacroIdx = -1;
int g_macroDepth = 0;

static int StringStartsWithIC(const char* str, const char* prefix) {
    if (!str || !prefix) return 0;
    while (*prefix) {
        char c1 = *str++;
        char c2 = *prefix++;
        if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
        if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
        if (c1 != c2) return 0;
    }
    return 1;
}

static int MatchCommand(const char* input, const char* cmd) {
    int len = lstrlenA(cmd);
    if (!StringStartsWithIC(input, cmd)) return 0;
    return (input[len] == ' ' || input[len] == '\t' || input[len] == '\0');
}

static void FormatPathPrompt(char* dst, size_t dstSize, const char* dir, const char* cmd) {
    char tmp[1024];
    wsprintfA(tmp, "%s> %s", dir ? dir : "", cmd ? cmd : "");
    lstrcpynA(dst, tmp, (int)dstSize);
}

void AppendOutput(const char* text) {
    if (!text) return;
    if (g_isRedirecting && g_hRedirFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(g_hRedirFile, text, lstrlenA(text), &written, NULL);
        WriteFile(g_hRedirFile, "\r\n", 2, &written, NULL);
        return;
    }
    int len = GetWindowTextLengthA(hOut);
    SendMessageA(hOut, EM_SETSEL, len, len);
    SendMessageA(hOut, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessageA(hOut, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
    SendMessageA(hOut, EM_SCROLLCARET, 0, 0);
}

void ApplyTheme(int themeIdx) {
    if (themeIdx < 0 || themeIdx >= 6) return;
    g_currentTheme = themeIdx;
    if (g_hBgBrush) DeleteObject(g_hBgBrush);
    if (g_hStatusBrush) DeleteObject(g_hStatusBrush);
    g_hBgBrush = CreateSolidBrush(g_themes[themeIdx].bg);
    g_hStatusBrush = CreateSolidBrush(g_themes[themeIdx].statusBg);
    if (g_hMainWnd) {
        InvalidateRect(g_hMainWnd, NULL, TRUE);
        InvalidateRect(hOut, NULL, TRUE);
        InvalidateRect(hPrompt, NULL, TRUE);
        InvalidateRect(hIn, NULL, TRUE);
        InvalidateRect(hStatus, NULL, TRUE);
        UpdateWindow(g_hMainWnd);
    }
    char fb[64];
    wsprintfA(fb, "Theme switched to %s", g_themes[themeIdx].name);
    SetStatusFeedback(fb);
}

void ShowHelpDialog(HWND hwnd) {
    const char* helpText = 
        "KTerm - Advanced Terminal Quick Reference\r\n\r\n"
        "KEYBOARD SHORTCUTS:\r\n"
        "  F1 / 'h'         - Open this Help Reference guide\r\n"
        "  Ctrl + T         - Open a new terminal tab\r\n"
        "  Ctrl + W         - Close active terminal tab\r\n"
        "  Ctrl + Tab       - Cycle to next tab (Shift for previous)\r\n"
        "  Ctrl + 1..8      - Jump directly to Tab 1 through 8\r\n"
        "  Ctrl + S         - Export session output log to text file\r\n"
        "  Ctrl + R         - Incremental reverse search (Ctrl+R cycles)\r\n"
        "  Ctrl + L / 'cls' - Clear terminal screen output\r\n"
        "  Ctrl + C         - Cancel current input command line\r\n"
        "  Ctrl + Alt + E   - Echo Anomaly Intercept (ARG)\r\n"
        "  Tab              - Autocomplete commands and file paths\r\n"
        "  Escape           - Clear input command line / cancel search\r\n"
        "  Up / Down Arrow  - Navigate command history\r\n\r\n"
        "CORE & NAVIGATION COMMANDS:\r\n"
        "  help             - Show command reference\r\n"
        "  ver / sysinfo    - Show OS and terminal version\r\n"
        "  dir / ls [path]  - List directory contents\r\n"
        "  cd [path]        - Change current directory\r\n"
        "  type / cat <file>- Read text file contents\r\n"
        "  echo [text]      - Print text (supports %VAR% & $VAR)\r\n"
        "  mkdir [folder]   - Create directory\r\n"
        "  date / time      - System calendar date or time\r\n"
        "  whoami           - Display current username\r\n\r\n"
        "EXPANDED FILE & TEXT UTILITIES:\r\n"
        "  grep [-i] <pat> [file] - Search lines for pattern in file or output\r\n"
        "  wc <file>        - Count lines, words, and bytes\r\n"
        "  head / tail [-n] <file>- View first or last N lines of a file\r\n"
        "  touch <file>     - Create empty file or update timestamp\r\n"
        "  del / rm <file>  - Delete a file\r\n"
        "  copy / cp <s <d> - Copy a file to destination\r\n"
        "  move / ren <s <d>- Move or rename a file\r\n"
        "  history / !n / !!- Command history list and recall\r\n"
        "  calc <expr>      - Arithmetic calculator (+, -, *, /, %, ^)\r\n"
        "  run / exec <file>- Execute commands from batch script file\r\n\r\n"
        "SYSTEM, NETWORK & LORE:\r\n"
        "  ps / tasks       - Display virtual processes and system memory\r\n"
        "  uptime           - Display session uptime and command telemetry\r\n"
        "  ping <host>      - ICMP echo diagnostic simulation\r\n"
        "  netstat          - Display active virtual network connections\r\n"
        "  theme <name>     - Set theme (green, amber, cyan, white, crimson, purple)\r\n"
        "  dmesg / syslog   - Inspect kernel boot log & security telemetry\r\n"
        "  glitch           - Inspect corrupted memory dump\r\n"
        "  cerberus / oper  - Watchdog status and operator channel\r\n"
        "  alias / unalias  - Custom aliases (alias name=cmd)\r\n"
        "  export / env     - Set environment variables (export VAR=val)\r\n"
        "  macro            - Macro scripts (record, stop, play, list)\r\n"
        "  export-log [file]- Export session output to text file\r\n"
        "  cmd > file, >> f - Output redirection (overwrite or append)\r\n"
        "  cmd1 ; cmd2      - Command chaining sequentially";

    MessageBoxA(hwnd, helpText, "KTerm - Command & Shortcut Guide", MB_OK | MB_ICONINFORMATION);
}

void SetStatusFeedback(const char* msg) {
    if (!msg) return;
    lstrcpynA(g_statusMsg, msg, sizeof(g_statusMsg));
    g_statusExpiry = GetTickCount() + 3500;
    if (hStatus) {
        char statusText[512];
        int cur = (g_activeTab >= 0 && g_activeTab < g_tabCount) ? (g_activeTab + 1) : 1;
        wsprintfA(statusText, " [F1 / h] Help   [Ctrl+T] Tab   [Ctrl+W] Close   [Ctrl+1..8] Tabs   [Ctrl+S] Export   |   %s   (Tab %d/%d)", g_statusMsg, cur, g_tabCount);
        SetWindowTextA(hStatus, statusText);
    }
}

void UpdateStatusDisplay() {
    if (!hStatus) return;
    char statusText[512];
    int cur = (g_activeTab >= 0 && g_activeTab < g_tabCount) ? (g_activeTab + 1) : 1;
    if (g_statusExpiry != 0 && GetTickCount() < g_statusExpiry && g_statusMsg[0]) {
        wsprintfA(statusText, " [F1 / h] Help   [Ctrl+T] Tab   [Ctrl+W] Close   [Ctrl+1..8] Tabs   [Ctrl+S] Export   |   %s   (Tab %d/%d)", g_statusMsg, cur, g_tabCount);
    } else {
        g_statusExpiry = 0;
        g_statusMsg[0] = '\0';
        wsprintfA(statusText, " [F1 / h] Help   [Ctrl+T] Tab   [Ctrl+W] Close   [Ctrl+1..8] Tabs   [Ctrl+S] Export   [Theme: %s]   [Tab %d of %d]", g_themes[g_currentTheme].name, cur, g_tabCount);
    }
    SetWindowTextA(hStatus, statusText);
}

void UpdateAppTitle() {
    if (!g_hMainWnd) return;
    char title[256];
    if (g_activeTab >= 0 && g_activeTab < g_tabCount) {
        wsprintfA(title, "KTerm - [%s: %s] - (Theme: %s | F1 for Help | Ctrl+T: New Tab)", g_tabs[g_activeTab].title, g_tabs[g_activeTab].currentDir, g_themes[g_currentTheme].name);
    } else {
        lstrcpynA(title, "KTerm - Advanced Terminal (Press 'h' or F1 for Help | Ctrl+T: New Tab)", sizeof(title));
    }
    SetWindowTextA(g_hMainWnd, title);
}

void UpdatePromptDisplay() {
    if (!hPrompt || !hIn || g_activeTab < 0 || g_activeTab >= g_tabCount) return;
    TabSession* tab = &g_tabs[g_activeTab];
    char pBuf[MAX_PATH + 8];
    wsprintfA(pBuf, "%s> ", tab->currentDir);
    SetWindowTextA(hPrompt, pBuf);

    HWND hParent = GetParent(hPrompt);
    if (!hParent) return;

    RECT rc;
    GetClientRect(hParent, &rc);
    int w = rc.right;
    int h = rc.bottom;
    int tabH = 28;
    int inH = 28;
    int statusH = 22;
    int outH = h - tabH - inH - statusH;
    if (outH < 0) outH = 0;

    HDC hdc = GetDC(hPrompt);
    HFONT oldF = (HFONT)SelectObject(hdc, g_hFont ? g_hFont : (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    SIZE sz;
    GetTextExtentPoint32A(hdc, pBuf, lstrlenA(pBuf), &sz);
    SelectObject(hdc, oldF);
    ReleaseDC(hPrompt, hdc);

    int promptW = sz.cx + 10;
    if (promptW > w / 2) promptW = w / 2;
    if (promptW < 40) promptW = 40;

    MoveWindow(hPrompt, 0, tabH + outH, promptW, inH, TRUE);
    MoveWindow(hIn, promptW, tabH + outH, w - promptW, inH, TRUE);
    if (hStatus) {
        MoveWindow(hStatus, 0, tabH + outH + inH, w, statusH, TRUE);
        UpdateStatusDisplay();
    }
    UpdateAppTitle();
}

void SaveActiveTabOutput() {
    if (g_activeTab < 0 || g_activeTab >= g_tabCount) return;
    TabSession* tab = &g_tabs[g_activeTab];
    if (!tab->outputBuffer) return;

    int len = GetWindowTextLengthA(hOut);
    if (len >= OUT_BUF_SIZE) len = OUT_BUF_SIZE - 1;
    GetWindowTextA(hOut, tab->outputBuffer, OUT_BUF_SIZE);
}

void LoadTabOutput(int tabIdx) {
    if (tabIdx < 0 || tabIdx >= g_tabCount) return;
    TabSession* tab = &g_tabs[tabIdx];
    if (tab->outputBuffer) {
        SetWindowTextA(hOut, tab->outputBuffer);
    } else {
        SetWindowTextA(hOut, "");
    }
    int len = GetWindowTextLengthA(hOut);
    SendMessageA(hOut, EM_SETSEL, len, len);
}

void InitTabSession(TabSession* tab, const char* title) {
    lstrcpynA(tab->title, title ? title : "Tab", sizeof(tab->title));
    GetCurrentDirectoryA(MAX_PATH, tab->currentDir);
    tab->history_count = 0;
    tab->history_pos = 0;
    tab->alias_count = 0;
    tab->env_count = 0;

    tab->outputBuffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, OUT_BUF_SIZE);

    // Default aliases
    lstrcpynA(tab->aliases[0].name, "ll", 64);
    lstrcpynA(tab->aliases[0].cmd, "dir", 256);
    lstrcpynA(tab->aliases[1].name, "cls", 64);
    lstrcpynA(tab->aliases[1].cmd, "clear", 256);
    lstrcpynA(tab->aliases[2].name, "cat", 64);
    lstrcpynA(tab->aliases[2].cmd, "type", 256);
    lstrcpynA(tab->aliases[3].name, "sysinfo", 64);
    lstrcpynA(tab->aliases[3].cmd, "ver", 256);
    lstrcpynA(tab->aliases[4].name, "h", 64);
    lstrcpynA(tab->aliases[4].cmd, "help", 256);
    tab->alias_count = 5;

    // Default env vars
    lstrcpynA(tab->envVars[0].name, "USER", 64);
    lstrcpynA(tab->envVars[0].value, "kilo_user", 256);
    lstrcpynA(tab->envVars[1].name, "OS", 64);
    lstrcpynA(tab->envVars[1].value, "KiloOS Native v1.2", 256);
    lstrcpynA(tab->envVars[2].name, "TERM", 64);
    lstrcpynA(tab->envVars[2].value, "kterm-native", 256);
    tab->env_count = 3;
}

void AddNewTab(const char* title) {
    if (g_tabCount >= MAX_TABS) {
        SetStatusFeedback("Maximum tab limit reached (8 tabs)");
        return;
    }
    
    char nameBuf[64];
    if (!title || !*title) {
        wsprintfA(nameBuf, "Tab %d", g_tabCount + 1);
    } else {
        lstrcpynA(nameBuf, title, sizeof(nameBuf));
    }

    InitTabSession(&g_tabs[g_tabCount], nameBuf);

    TCITEMA tie;
    ZeroMemory(&tie, sizeof(tie));
    tie.mask = TCIF_TEXT;
    tie.pszText = g_tabs[g_tabCount].title;
    
    int tabIndex = g_tabCount;
    TabCtrl_InsertItem(hTab, tabIndex, &tie);

    g_tabCount++;

    // Initial banner for tab
    char banner[256];
    wsprintfA(banner, "KiloOS Terminal v1.2 [%s] (Theme: %s)\r\n[F1: Help | Ctrl+T: New Tab | Ctrl+R: Search | Ctrl+Alt+E: Echo]", nameBuf, g_themes[g_currentTheme].name);
    
    if (g_tabCount == 1) {
        AppendOutput(banner);
        UpdatePromptDisplay();
    } else {
        lstrcpynA(g_tabs[tabIndex].outputBuffer, banner, OUT_BUF_SIZE);
        TabCtrl_SetCurSel(hTab, tabIndex);
        SaveActiveTabOutput();
        g_activeTab = tabIndex;
        LoadTabOutput(g_activeTab);
        UpdatePromptDisplay();
        char feedback[64];
        wsprintfA(feedback, "Opened %s", nameBuf);
        SetStatusFeedback(feedback);
    }
}

void SwitchTab(int newIdx) {
    if (newIdx < 0 || newIdx >= g_tabCount || newIdx == g_activeTab) return;
    SaveActiveTabOutput();
    g_activeTab = newIdx;
    TabCtrl_SetCurSel(hTab, g_activeTab);
    LoadTabOutput(g_activeTab);
    UpdatePromptDisplay();
    char feedback[64];
    wsprintfA(feedback, "Switched to Tab %d (%s)", g_activeTab + 1, g_tabs[g_activeTab].title);
    SetStatusFeedback(feedback);
    SetFocus(hIn);
}

// Alias Expansion
void ExpandAlias(const char* inputCmd, char* outBuf, size_t outSize) {
    TabSession* tab = &g_tabs[g_activeTab];
    const char* space = my_strchr(inputCmd, ' ');
    char firstWord[64];
    ZeroMemory(firstWord, sizeof(firstWord));
    if (space) {
        size_t len = space - inputCmd;
        if (len >= sizeof(firstWord)) len = sizeof(firstWord) - 1;
        lstrcpynA(firstWord, inputCmd, (int)len + 1);
    } else {
        lstrcpynA(firstWord, inputCmd, sizeof(firstWord));
    }

    for (int i = 0; i < tab->alias_count; i++) {
        if (lstrcmpiA(firstWord, tab->aliases[i].name) == 0) {
            if (space) {
                wsprintfA(outBuf, "%s%s", tab->aliases[i].cmd, space);
            } else {
                lstrcpynA(outBuf, tab->aliases[i].cmd, (int)outSize);
            }
            return;
        }
    }
    lstrcpynA(outBuf, inputCmd, (int)outSize);
}

// Env Var Expansion (%VAR% or $VAR)
void ExpandEnvVars(const char* inputCmd, char* outBuf, size_t outSize) {
    TabSession* tab = &g_tabs[g_activeTab];
    char temp[512];
    lstrcpynA(temp, inputCmd, sizeof(temp));

    for (int i = 0; i < tab->env_count; i++) {
        char target1[72], target2[72];
        wsprintfA(target1, "%%%s%%", tab->envVars[i].name);
        wsprintfA(target2, "$%s", tab->envVars[i].name);

        char result[512];
        result[0] = '\0';
        char* pos = temp;
        char* found = NULL;
        while ((found = my_strstr(pos, target1)) != NULL || (found = my_strstr(pos, target2)) != NULL) {
            size_t prefixLen = found - pos;
            size_t matchLen = (found[0] == '%') ? lstrlenA(target1) : lstrlenA(target2);
            size_t curLen = lstrlenA(result);
            if (curLen + prefixLen < sizeof(result) - 1) {
                my_strncat(result, pos, prefixLen);
            }
            curLen = lstrlenA(result);
            size_t valLen = lstrlenA(tab->envVars[i].value);
            if (curLen + valLen < sizeof(result) - 1) {
                my_strcat(result, tab->envVars[i].value);
            }
            pos = found + matchLen;
        }
        size_t curLen = lstrlenA(result);
        size_t remLen = lstrlenA(pos);
        if (curLen + remLen < sizeof(result) - 1) {
            my_strcat(result, pos);
        }
        lstrcpynA(temp, result, sizeof(temp));
    }
    lstrcpynA(outBuf, temp, (int)outSize);
}

// Arithmetic expression evaluator for 'calc'
static int eval_expr(const char** p);

static int eval_factor(const char** p) {
    while (**p == ' ' || **p == '\t') (*p)++;
    int sign = 1;
    if (**p == '-') { sign = -1; (*p)++; while (**p == ' ' || **p == '\t') (*p)++; }
    else if (**p == '+') { (*p)++; while (**p == ' ' || **p == '\t') (*p)++; }
    
    if (**p == '(') {
        (*p)++;
        int val = eval_expr(p);
        while (**p == ' ' || **p == '\t') (*p)++;
        if (**p == ')') (*p)++;
        return sign * val;
    }
    
    int val = 0;
    while (**p >= '0' && **p <= '9') {
        val = val * 10 + (**p - '0');
        (*p)++;
    }
    return sign * val;
}

static int eval_power(const char** p) {
    int left = eval_factor(p);
    while (1) {
        while (**p == ' ' || **p == '\t') (*p)++;
        if (**p == '^') {
            (*p)++;
            int exp = eval_factor(p);
            int res = 1;
            for (int i = 0; i < exp; i++) res *= left;
            left = res;
        } else {
            break;
        }
    }
    return left;
}

static int eval_term(const char** p) {
    int left = eval_power(p);
    while (1) {
        while (**p == ' ' || **p == '\t') (*p)++;
        char op = **p;
        if (op == '*' || op == '/' || op == '%') {
            (*p)++;
            int right = eval_power(p);
            if (op == '*') left = left * right;
            else if (op == '/') left = (right != 0) ? (left / right) : 0;
            else if (op == '%') left = (right != 0) ? (left % right) : 0;
        } else {
            break;
        }
    }
    return left;
}

static int eval_expr(const char** p) {
    int left = eval_term(p);
    while (1) {
        while (**p == ' ' || **p == '\t') (*p)++;
        char op = **p;
        if (op == '+' || op == '-') {
            (*p)++;
            int right = eval_term(p);
            if (op == '+') left = left + right;
            else left = left - right;
        } else {
            break;
        }
    }
    return left;
}

void ProcessSingleCommand(const char* rawCmd) {
    if (!rawCmd) return;
    while (*rawCmd == ' ' || *rawCmd == '\t') rawCmd++;
    if (*rawCmd == '\0') return;

    g_totalCommands++;
    TabSession* tab = &g_tabs[g_activeTab];
    SetCurrentDirectoryA(tab->currentDir);

    char cmdWithAlias[512];
    ExpandAlias(rawCmd, cmdWithAlias, sizeof(cmdWithAlias));

    char cmd[512];
    ExpandEnvVars(cmdWithAlias, cmd, sizeof(cmd));

    char fullCmd[512];
    FormatPathPrompt(fullCmd, sizeof(fullCmd), tab->currentDir, rawCmd);
    AppendOutput(fullCmd);

    if (g_isRecording && g_recordingMacroIdx != -1 && !MatchCommand(cmd, "macro")) {
        Macro* m = &g_macros[g_recordingMacroIdx];
        if (m->cmd_count < MAX_MACRO_CMDS) {
            lstrcpynA(m->commands[m->cmd_count++], rawCmd, 256);
        } else {
            AppendOutput("Macro command limit reached. Stopping recording.");
            g_isRecording = FALSE;
            g_recordingMacroIdx = -1;
        }
    }

    if (MatchCommand(cmd, "help") || lstrcmpiA(cmd, "h") == 0) {
        AppendOutput("KTerm Expanded Commands Reference:");
        AppendOutput("  help / h   - Show available commands and shortcuts");
        AppendOutput("  ver        - Show OS version and build state");
        AppendOutput("  clear/cls  - Clear terminal screen");
        AppendOutput("  dir/ls     - List directory contents (dir [path])");
        AppendOutput("  cd         - Change directory (cd [path])");
        AppendOutput("  type/cat   - View text file contents (type <file>)");
        AppendOutput("  echo       - Print text (supports %VAR% & $VAR)");
        AppendOutput("  mkdir      - Create directory (mkdir <name>)");
        AppendOutput("  touch      - Create empty file or update timestamp (touch <file>)");
        AppendOutput("  del/rm     - Delete a file (del <file>)");
        AppendOutput("  copy/cp    - Copy file to destination (copy <src> <dst>)");
        AppendOutput("  move/ren   - Move or rename file (move <src> <dst>)");
        AppendOutput("  grep/find  - Search text lines (grep [-i] <pattern> [file])");
        AppendOutput("  wc         - Count lines, words, and bytes in file (wc <file>)");
        AppendOutput("  head/tail  - View first or last N lines (head [-n N] <file>)");
        AppendOutput("  history    - Display command history (!n to recall, !! for last)");
        AppendOutput("  calc       - Arithmetic math calculator (calc <expression>)");
        AppendOutput("  ps/tasks   - Show virtual system processes and memory");
        AppendOutput("  uptime     - Display system uptime and command telemetry");
        AppendOutput("  ping       - ICMP echo diagnostic simulation (ping <host>)");
        AppendOutput("  netstat    - Active virtual network sockets");
        AppendOutput("  theme      - Color theme (theme <green|amber|cyan|white|crimson|purple>)");
        AppendOutput("  dmesg      - Kernel boot diagnostic log & ARG telemetry");
        AppendOutput("  glitch     - Memory dump anomaly inspection");
        AppendOutput("  run/exec   - Run batch commands from script file (run <file>)");
        AppendOutput("  date/time  - System calendar date or clock");
        AppendOutput("  whoami     - Display active username");
        AppendOutput("  alias      - Manage aliases (alias name=cmd, unalias name)");
        AppendOutput("  env/export - Environment variables (export VAR=val, unset VAR)");
        AppendOutput("  macro      - Macro recording (record, stop, play, list)");
        AppendOutput("  export-log - Export terminal session output to file");
        AppendOutput("  newtab     - Open new terminal tab session (newtab [title])");
        AppendOutput("  exit       - Exit application or close active tab");
        AppendOutput("  Piping     - Output redirection (cmd > file, cmd >> file)");
        AppendOutput("  Chaining   - Multi-command execution (cmd1 ; cmd2 or cmd1 && cmd2)");
    } else if (MatchCommand(cmd, "ver") || MatchCommand(cmd, "sysinfo")) {
        AppendOutput("KiloOS Native v1.2 (Deep Utilities & Multi-Tab Terminal Shell)");
        AppendOutput("Kernel Constraints: <999KB Strict Size Policy Active.");
    } else if (MatchCommand(cmd, "date")) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        char buf[64];
        wsprintfA(buf, "%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
        AppendOutput(buf);
    } else if (MatchCommand(cmd, "time")) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        char buf[64];
        wsprintfA(buf, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
        AppendOutput(buf);
    } else if (MatchCommand(cmd, "whoami")) {
        for (int i = 0; i < tab->env_count; i++) {
            if (lstrcmpiA(tab->envVars[i].name, "USER") == 0) {
                AppendOutput(tab->envVars[i].value);
                return;
            }
        }
        AppendOutput("kilo_user");
    } else if (MatchCommand(cmd, "clear") || MatchCommand(cmd, "cls")) {
        SetWindowTextA(hOut, "");
    } else if (MatchCommand(cmd, "newtab")) {
        const char* title = cmd + 6;
        while (*title == ' ' || *title == '\t') title++;
        AddNewTab(title);
    } else if (MatchCommand(cmd, "exit") || MatchCommand(cmd, "closetab")) {
        if (g_tabCount > 1) {
            TabCtrl_DeleteItem(hTab, g_activeTab);
            if (g_tabs[g_activeTab].outputBuffer) {
                HeapFree(GetProcessHeap(), 0, g_tabs[g_activeTab].outputBuffer);
                g_tabs[g_activeTab].outputBuffer = NULL;
            }
            for (int i = g_activeTab; i < g_tabCount - 1; i++) {
                g_tabs[i] = g_tabs[i + 1];
            }
            g_tabCount--;
            int newActive = g_activeTab >= g_tabCount ? g_tabCount - 1 : g_activeTab;
            g_activeTab = -1;
            SwitchTab(newActive);
        } else {
            PostQuitMessage(0);
        }
    } else if (MatchCommand(cmd, "export-log")) {
        const char* fileName = cmd + 10;
        while (*fileName == ' ' || *fileName == '\t') fileName++;
        if (*fileName == '\0') fileName = "kterm_log.txt";

        int len = GetWindowTextLengthA(hOut);
        if (len > 0) {
            char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len + 1);
            if (buf) {
                GetWindowTextA(hOut, buf, len + 1);
                HANDLE hFile = CreateFileA(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    DWORD written = 0;
                    WriteFile(hFile, buf, len, &written, NULL);
                    CloseHandle(hFile);
                    char msg[256];
                    wsprintfA(msg, "Log saved to %s", fileName);
                    AppendOutput(msg);
                    SetStatusFeedback(msg);
                } else {
                    AppendOutput("Failed to open file for export.");
                }
                HeapFree(GetProcessHeap(), 0, buf);
            }
        } else {
            AppendOutput("Output log is empty.");
        }
    } else if (lstrcmpiA(cmd, "alias") == 0) {
        AppendOutput("Current Aliases:");
        if (tab->alias_count == 0) {
            AppendOutput("  (none)");
        } else {
            for (int i = 0; i < tab->alias_count; i++) {
                char line[300];
                wsprintfA(line, "  %s -> \"%s\"", tab->aliases[i].name, tab->aliases[i].cmd);
                AppendOutput(line);
            }
        }
    } else if (MatchCommand(cmd, "alias")) {
        const char* expr = cmd + 5;
        while (*expr == ' ' || *expr == '\t') expr++;
        const char* eq = my_strchr(expr, '=');
        char aName[64], aCmd[256];
        ZeroMemory(aName, sizeof(aName));
        ZeroMemory(aCmd, sizeof(aCmd));
        if (eq) {
            size_t nLen = eq - expr;
            if (nLen >= sizeof(aName)) nLen = sizeof(aName) - 1;
            lstrcpynA(aName, expr, (int)nLen + 1);
            lstrcpynA(aCmd, eq + 1, sizeof(aCmd));
        } else {
            const char* sp = my_strchr(expr, ' ');
            if (sp) {
                size_t nLen = sp - expr;
                if (nLen >= sizeof(aName)) nLen = sizeof(aName) - 1;
                lstrcpynA(aName, expr, (int)nLen + 1);
                lstrcpynA(aCmd, sp + 1, sizeof(aCmd));
            }
        }
        if (aName[0] && aCmd[0]) {
            int foundIdx = -1;
            for (int i = 0; i < tab->alias_count; i++) {
                if (lstrcmpiA(tab->aliases[i].name, aName) == 0) {
                    foundIdx = i;
                    break;
                }
            }
            if (foundIdx == -1 && tab->alias_count < MAX_ALIASES) {
                foundIdx = tab->alias_count++;
            }
            if (foundIdx != -1) {
                lstrcpynA(tab->aliases[foundIdx].name, aName, 64);
                lstrcpynA(tab->aliases[foundIdx].cmd, aCmd, 256);
                char line[300];
                wsprintfA(line, "Alias set: %s -> \"%s\"", aName, aCmd);
                AppendOutput(line);
            } else {
                AppendOutput("Alias limit reached.");
            }
        } else {
            AppendOutput("Usage: alias name=\"command\"");
        }
    } else if (MatchCommand(cmd, "unalias")) {
        const char* target = cmd + 7;
        while (*target == ' ' || *target == '\t') target++;
        int found = 0;
        for (int i = 0; i < tab->alias_count; i++) {
            if (lstrcmpiA(tab->aliases[i].name, target) == 0) {
                for (int j = i; j < tab->alias_count - 1; j++) {
                    tab->aliases[j] = tab->aliases[j + 1];
                }
                tab->alias_count--;
                found = 1;
                break;
            }
        }
        if (found) {
            AppendOutput("Alias removed.");
        } else {
            AppendOutput("Alias not found.");
        }
    } else if (lstrcmpiA(cmd, "env") == 0 || lstrcmpiA(cmd, "export") == 0) {
        AppendOutput("Environment Variables:");
        for (int i = 0; i < tab->env_count; i++) {
            char line[320];
            wsprintfA(line, "  %s=%s", tab->envVars[i].name, tab->envVars[i].value);
            AppendOutput(line);
        }
    } else if (MatchCommand(cmd, "export")) {
        const char* expr = cmd + 6;
        while (*expr == ' ' || *expr == '\t') expr++;
        const char* eq = my_strchr(expr, '=');
        char eName[64], eVal[256];
        ZeroMemory(eName, sizeof(eName));
        ZeroMemory(eVal, sizeof(eVal));
        if (eq) {
            size_t nLen = eq - expr;
            if (nLen >= sizeof(eName)) nLen = sizeof(eName) - 1;
            lstrcpynA(eName, expr, (int)nLen + 1);
            lstrcpynA(eVal, eq + 1, sizeof(eVal));
        } else {
            const char* sp = my_strchr(expr, ' ');
            if (sp) {
                size_t nLen = sp - expr;
                if (nLen >= sizeof(eName)) nLen = sizeof(eName) - 1;
                lstrcpynA(eName, expr, (int)nLen + 1);
                lstrcpynA(eVal, sp + 1, sizeof(eVal));
            }
        }
        if (eName[0]) {
            int foundIdx = -1;
            for (int i = 0; i < tab->env_count; i++) {
                if (lstrcmpiA(tab->envVars[i].name, eName) == 0) {
                    foundIdx = i;
                    break;
                }
            }
            if (foundIdx == -1 && tab->env_count < MAX_ENV) {
                foundIdx = tab->env_count++;
            }
            if (foundIdx != -1) {
                lstrcpynA(tab->envVars[foundIdx].name, eName, 64);
                lstrcpynA(tab->envVars[foundIdx].value, eVal, 256);
                SetEnvironmentVariableA(eName, eVal);
                char line[320];
                wsprintfA(line, "Env set: %s=%s", eName, eVal);
                AppendOutput(line);
            } else {
                AppendOutput("Environment variable limit reached.");
            }
        } else {
            AppendOutput("Usage: export VAR=VALUE");
        }
    } else if (MatchCommand(cmd, "unset")) {
        const char* target = cmd + 5;
        while (*target == ' ' || *target == '\t') target++;
        int found = 0;
        for (int i = 0; i < tab->env_count; i++) {
            if (lstrcmpiA(tab->envVars[i].name, target) == 0) {
                SetEnvironmentVariableA(tab->envVars[i].name, NULL);
                for (int j = i; j < tab->env_count - 1; j++) {
                    tab->envVars[j] = tab->envVars[j + 1];
                }
                tab->env_count--;
                found = 1;
                break;
            }
        }
        if (found) {
            AppendOutput("Environment variable removed.");
        } else {
            AppendOutput("Variable not found.");
        }
    } else if (MatchCommand(cmd, "dir") || MatchCommand(cmd, "ls")) {
        const char* targetDir = tab->currentDir;
        char customDir[MAX_PATH];
        const char* space = my_strchr(cmd, ' ');
        if (space) {
            const char* arg = space;
            while (*arg == ' ' || *arg == '\t') arg++;
            if (*arg != '\0' && *arg != '-' && *arg != '/') {
                lstrcpynA(customDir, arg, sizeof(customDir));
                targetDir = customDir;
            }
        }
        WIN32_FIND_DATAA fd;
        char search[MAX_PATH + 16];
        char tmpSearch[1024];
        wsprintfA(tmpSearch, "%s\\*", targetDir);
        lstrcpynA(search, tmpSearch, sizeof(search));

        HANDLE hFind = FindFirstFileA(search, &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                char entry[MAX_PATH + 32];
                char tmpEntry[1024];
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    wsprintfA(tmpEntry, "<DIR>    %s", fd.cFileName);
                } else {
                    wsprintfA(tmpEntry, "         %s", fd.cFileName);
                }
                lstrcpynA(entry, tmpEntry, sizeof(entry));
                AppendOutput(entry);
            } while (FindNextFileA(hFind, &fd));
            FindClose(hFind);
        } else {
            AppendOutput("Failed to list directory contents.");
        }
    } else if (MatchCommand(cmd, "cd")) {
        const char* target = cmd + 2;
        if (*target == ' ' || *target == '\t' || *target == '\0') {
            while (*target == ' ' || *target == '\t') target++;
            if (*target == '\0') {
                AppendOutput(tab->currentDir);
            } else {
                if (SetCurrentDirectoryA(target)) {
                    GetCurrentDirectoryA(MAX_PATH, tab->currentDir);
                    const char* lastSlash = my_strrchr(tab->currentDir, '\\');
                    if (lastSlash && *(lastSlash + 1)) {
                        lstrcpynA(tab->title, lastSlash + 1, sizeof(tab->title));
                    } else {
                        lstrcpynA(tab->title, tab->currentDir, sizeof(tab->title));
                    }
                    TCITEMA tie;
                    ZeroMemory(&tie, sizeof(tie));
                    tie.mask = TCIF_TEXT;
                    tie.pszText = tab->title;
                    TabCtrl_SetItem(hTab, g_activeTab, &tie);
                    UpdatePromptDisplay();
                } else {
                    AppendOutput("Directory not found.");
                }
            }
        } else {
            AppendOutput("Bad command or file name.");
        }
    } else if (MatchCommand(cmd, "echo")) {
        const char* text = cmd + 4;
        if (*text == ' ' || *text == '\t' || *text == '\0') {
            while (*text == ' ' || *text == '\t') text++;
            AppendOutput(text);
        } else {
            AppendOutput("Bad command or file name.");
        }
    } else if (MatchCommand(cmd, "mkdir")) {
        const char* dirName = cmd + 5;
        if (*dirName == ' ' || *dirName == '\t' || *dirName == '\0') {
            while (*dirName == ' ' || *dirName == '\t') dirName++;
            if (*dirName == '\0') {
                AppendOutput("Usage: mkdir <directory_name>");
            } else if (CreateDirectoryA(dirName, NULL)) {
                AppendOutput("Directory created.");
            } else {
                AppendOutput("Failed to create directory.");
            }
        } else {
            AppendOutput("Bad command or file name.");
        }
    } else if (MatchCommand(cmd, "type") || MatchCommand(cmd, "cat")) {
        const char* fileName = my_strchr(cmd, ' ');
        if (fileName) {
            while (*fileName == ' ' || *fileName == '\t') fileName++;
            if (*fileName == '\0') {
                AppendOutput("Usage: type <filename>");
            } else {
                HANDLE hFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    DWORD fileSize = GetFileSize(hFile, NULL);
                    if (fileSize != INVALID_FILE_SIZE && fileSize > 0) {
                        if (fileSize > 65536) fileSize = 65536;
                        char* buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileSize + 1);
                        if (buffer) {
                            DWORD bytesRead = 0;
                            if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
                                buffer[bytesRead] = '\0';
                                AppendOutput(buffer);
                            }
                            HeapFree(GetProcessHeap(), 0, buffer);
                        }
                    } else if (fileSize == 0) {
                        AppendOutput("[File is empty]");
                    }
                    CloseHandle(hFile);
                } else {
                    AppendOutput("File not found or cannot be opened.");
                }
            }
        } else {
            AppendOutput("Usage: type <filename>");
        }
    } else if (MatchCommand(cmd, "touch")) {
        const char* fileName = cmd + 5;
        while (*fileName == ' ' || *fileName == '\t') fileName++;
        if (*fileName == '\0') {
            AppendOutput("Usage: touch <filename>");
        } else {
            HANDLE hFile = CreateFileA(fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE) {
                CloseHandle(hFile);
                char msg[256];
                wsprintfA(msg, "File touched: %s", fileName);
                AppendOutput(msg);
            } else {
                AppendOutput("Failed to touch file.");
            }
        }
    } else if (MatchCommand(cmd, "del") || MatchCommand(cmd, "rm") || MatchCommand(cmd, "erase")) {
        const char* fileName = my_strchr(cmd, ' ');
        if (fileName) {
            while (*fileName == ' ' || *fileName == '\t') fileName++;
            if (*fileName == '\0') {
                AppendOutput("Usage: del <filename>");
            } else {
                if (DeleteFileA(fileName)) {
                    char msg[256];
                    wsprintfA(msg, "File deleted: %s", fileName);
                    AppendOutput(msg);
                } else {
                    AppendOutput("Failed to delete file or file not found.");
                }
            }
        } else {
            AppendOutput("Usage: del <filename>");
        }
    } else if (MatchCommand(cmd, "copy") || MatchCommand(cmd, "cp")) {
        const char* args = my_strchr(cmd, ' ');
        if (!args) {
            AppendOutput("Usage: copy <source> <destination>");
        } else {
            while (*args == ' ' || *args == '\t') args++;
            const char* space2 = my_strchr(args, ' ');
            if (!space2) {
                AppendOutput("Usage: copy <source> <destination>");
            } else {
                char src[MAX_PATH], dst[MAX_PATH];
                size_t sLen = space2 - args;
                if (sLen >= sizeof(src)) sLen = sizeof(src) - 1;
                lstrcpynA(src, args, (int)sLen + 1);
                while (*space2 == ' ' || *space2 == '\t') space2++;
                lstrcpynA(dst, space2, sizeof(dst));

                if (CopyFileA(src, dst, FALSE)) {
                    char msg[MAX_PATH * 2 + 32];
                    wsprintfA(msg, "Copied %s -> %s", src, dst);
                    AppendOutput(msg);
                } else {
                    AppendOutput("Failed to copy file.");
                }
            }
        }
    } else if (MatchCommand(cmd, "move") || MatchCommand(cmd, "mv") || MatchCommand(cmd, "ren")) {
        const char* args = my_strchr(cmd, ' ');
        if (!args) {
            AppendOutput("Usage: move <source> <destination>");
        } else {
            while (*args == ' ' || *args == '\t') args++;
            const char* space2 = my_strchr(args, ' ');
            if (!space2) {
                AppendOutput("Usage: move <source> <destination>");
            } else {
                char src[MAX_PATH], dst[MAX_PATH];
                size_t sLen = space2 - args;
                if (sLen >= sizeof(src)) sLen = sizeof(src) - 1;
                lstrcpynA(src, args, (int)sLen + 1);
                while (*space2 == ' ' || *space2 == '\t') space2++;
                lstrcpynA(dst, space2, sizeof(dst));

                if (MoveFileA(src, dst)) {
                    char msg[MAX_PATH * 2 + 32];
                    wsprintfA(msg, "Moved %s -> %s", src, dst);
                    AppendOutput(msg);
                } else {
                    AppendOutput("Failed to move/rename file.");
                }
            }
        }
    } else if (MatchCommand(cmd, "wc")) {
        const char* fileName = cmd + 2;
        while (*fileName == ' ' || *fileName == '\t') fileName++;
        if (*fileName == '\0') {
            AppendOutput("Usage: wc <filename>");
        } else {
            HANDLE hFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE) {
                DWORD fSize = GetFileSize(hFile, NULL);
                if (fSize != INVALID_FILE_SIZE && fSize > 0) {
                    char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fSize + 1);
                    if (buf) {
                        DWORD bytesRead = 0;
                        ReadFile(hFile, buf, fSize, &bytesRead, NULL);
                        buf[bytesRead] = '\0';
                        int lines = 0, words = 0, chars = (int)bytesRead;
                        BOOL inWord = FALSE;
                        for (DWORD i = 0; i < bytesRead; i++) {
                            if (buf[i] == '\n') lines++;
                            if (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\r' || buf[i] == '\n') {
                                inWord = FALSE;
                            } else if (!inWord) {
                                inWord = TRUE;
                                words++;
                            }
                        }
                        char out[256];
                        wsprintfA(out, "Lines: %d  Words: %d  Bytes: %d  File: %s", lines, words, chars, fileName);
                        AppendOutput(out);
                        HeapFree(GetProcessHeap(), 0, buf);
                    }
                } else {
                    AppendOutput("Lines: 0  Words: 0  Bytes: 0");
                }
                CloseHandle(hFile);
            } else {
                AppendOutput("File not found.");
            }
        }
    } else if (MatchCommand(cmd, "head") || MatchCommand(cmd, "tail")) {
        BOOL isTail = MatchCommand(cmd, "tail");
        const char* args = cmd + 4;
        while (*args == ' ' || *args == '\t') args++;
        int reqLines = 10;
        if (StringStartsWithIC(args, "-n")) {
            args += 2;
            while (*args == ' ' || *args == '\t') args++;
            reqLines = my_atoi(args);
            if (reqLines <= 0) reqLines = 10;
            while (*args >= '0' && *args <= '9') args++;
            while (*args == ' ' || *args == '\t') args++;
        }
        if (*args == '\0') {
            AppendOutput("Usage: head/tail [-n lines] <filename>");
        } else {
            HANDLE hFile = CreateFileA(args, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE) {
                DWORD fSize = GetFileSize(hFile, NULL);
                if (fSize != INVALID_FILE_SIZE && fSize > 0) {
                    if (fSize > 65536) fSize = 65536;
                    char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fSize + 1);
                    if (buf) {
                        DWORD bytesRead = 0;
                        ReadFile(hFile, buf, fSize, &bytesRead, NULL);
                        buf[bytesRead] = '\0';
                        
                        // Count lines
                        int totalLines = 0;
                        char* p = buf;
                        while (*p) {
                            if (*p == '\n') totalLines++;
                            p++;
                        }
                        if (p > buf && *(p - 1) != '\n') totalLines++;

                        int startLine = isTail ? (totalLines - reqLines) : 0;
                        if (startLine < 0) startLine = 0;
                        int endLine = isTail ? totalLines : reqLines;

                        int curLine = 0;
                        char* lineStart = buf;
                        p = buf;
                        while (*p) {
                            if (*p == '\r' || *p == '\n') {
                                char saved = *p;
                                *p = '\0';
                                if (curLine >= startLine && curLine < endLine) {
                                    AppendOutput(lineStart);
                                }
                                *p = saved;
                                if (*p == '\r' && *(p + 1) == '\n') p++;
                                lineStart = p + 1;
                                curLine++;
                            }
                            p++;
                        }
                        if (*lineStart && curLine >= startLine && curLine < endLine) {
                            AppendOutput(lineStart);
                        }
                        HeapFree(GetProcessHeap(), 0, buf);
                    }
                }
                CloseHandle(hFile);
            } else {
                AppendOutput("File not found.");
            }
        }
    } else if (MatchCommand(cmd, "grep") || MatchCommand(cmd, "findstr") || MatchCommand(cmd, "find")) {
        const char* args = my_strchr(cmd, ' ');
        if (!args) {
            AppendOutput("Usage: grep [-i] <pattern> [filename]");
        } else {
            while (*args == ' ' || *args == '\t') args++;
            BOOL caseInsensitive = TRUE;
            if (StringStartsWithIC(args, "-i ")) {
                caseInsensitive = TRUE;
                args += 3;
                while (*args == ' ' || *args == '\t') args++;
            }
            char pattern[128];
            const char* space2 = my_strchr(args, ' ');
            const char* targetFile = NULL;
            if (space2) {
                size_t pLen = space2 - args;
                if (pLen >= sizeof(pattern)) pLen = sizeof(pattern) - 1;
                lstrcpynA(pattern, args, (int)pLen + 1);
                targetFile = space2 + 1;
                while (*targetFile == ' ' || *targetFile == '\t') targetFile++;
            } else {
                lstrcpynA(pattern, args, sizeof(pattern));
            }

            if (pattern[0] == '\0') {
                AppendOutput("Usage: grep [-i] <pattern> [filename]");
            } else if (targetFile && *targetFile) {
                HANDLE hFile = CreateFileA(targetFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    DWORD fSize = GetFileSize(hFile, NULL);
                    if (fSize != INVALID_FILE_SIZE && fSize > 0) {
                        if (fSize > 65536) fSize = 65536;
                        char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fSize + 1);
                        if (buf) {
                            DWORD bytesRead = 0;
                            ReadFile(hFile, buf, fSize, &bytesRead, NULL);
                            buf[bytesRead] = '\0';
                            int matches = 0;
                            char* lineStart = buf;
                            char* p = buf;
                            while (*p) {
                                if (*p == '\r' || *p == '\n') {
                                    char saved = *p;
                                    *p = '\0';
                                    if ((caseInsensitive && my_strstri(lineStart, pattern)) ||
                                        (!caseInsensitive && my_strstr(lineStart, pattern))) {
                                        AppendOutput(lineStart);
                                        matches++;
                                    }
                                    *p = saved;
                                    if (*p == '\r' && *(p + 1) == '\n') p++;
                                    lineStart = p + 1;
                                }
                                p++;
                            }
                            if (*lineStart && ((caseInsensitive && my_strstri(lineStart, pattern)) ||
                                               (!caseInsensitive && my_strstr(lineStart, pattern)))) {
                                AppendOutput(lineStart);
                                matches++;
                            }
                            char res[128];
                            wsprintfA(res, "Grep finished: %d match(es) in %s.", matches, targetFile);
                            AppendOutput(res);
                            HeapFree(GetProcessHeap(), 0, buf);
                        }
                    }
                    CloseHandle(hFile);
                } else {
                    AppendOutput("File not found.");
                }
            } else {
                // Search output text
                int len = GetWindowTextLengthA(hOut);
                if (len > 0) {
                    char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len + 1);
                    if (buf) {
                        GetWindowTextA(hOut, buf, len + 1);
                        int matches = 0;
                        char* lineStart = buf;
                        char* p = buf;
                        while (*p) {
                            if (*p == '\r' || *p == '\n') {
                                char saved = *p;
                                *p = '\0';
                                if ((caseInsensitive && my_strstri(lineStart, pattern)) ||
                                    (!caseInsensitive && my_strstr(lineStart, pattern))) {
                                    AppendOutput(lineStart);
                                    matches++;
                                }
                                *p = saved;
                                if (*p == '\r' && *(p + 1) == '\n') p++;
                                lineStart = p + 1;
                            }
                            p++;
                        }
                        char res[128];
                        wsprintfA(res, "Grep in output: %d match(es).", matches);
                        AppendOutput(res);
                        HeapFree(GetProcessHeap(), 0, buf);
                    }
                }
            }
        }
    } else if (MatchCommand(cmd, "history")) {
        AppendOutput("Session Command History:");
        for (int i = 0; i < tab->history_count; i++) {
            char hLine[300];
            wsprintfA(hLine, "  %2d  %s", i + 1, tab->history[i]);
            AppendOutput(hLine);
        }
    } else if (MatchCommand(cmd, "calc")) {
        const char* expr = cmd + 4;
        while (*expr == ' ' || *expr == '\t') expr++;
        if (*expr == '\0') {
            AppendOutput("Usage: calc <expression> (e.g. calc (1024 * 768) / 1000)");
        } else {
            const char* p = expr;
            int result = eval_expr(&p);
            char resStr[128];
            wsprintfA(resStr, "Result = %d", result);
            AppendOutput(resStr);
        }
    } else if (MatchCommand(cmd, "ps") || MatchCommand(cmd, "tasks")) {
        AppendOutput("PID   PROCESS          TYPE      STATUS    MEMORY   CPU");
        AppendOutput("---------------------------------------------------------");
        AppendOutput("001   KiloKernel.sys   Kernel    ACTIVE    48 KB    0.2%");
        AppendOutput("004   GDI_Driver.dll   Graphics  ACTIVE    32 KB    0.5%");
        AppendOutput("012   Cerberus_Guard   Security  GUARD     24 KB    0.1%");
        AppendOutput("018   KiloFS_Service   Storage   ACTIVE    16 KB    0.0%");
        AppendOutput("032   KTerm_Host.exe   Shell     ACTIVE    28 KB    0.4%");
        AppendOutput("044   NetEcho_Daemon   Network   STANDBY   12 KB    0.0%");
        AppendOutput("---------------------------------------------------------");
        AppendOutput("Total: 6 active processes, 160 KB total memory allocated.");
    } else if (MatchCommand(cmd, "uptime")) {
        DWORD ms = GetTickCount() - g_startTime;
        DWORD sec = ms / 1000;
        DWORD min = sec / 60;
        DWORD hr = min / 60;
        sec %= 60;
        min %= 60;
        char upBuf[160];
        wsprintfA(upBuf, "Terminal Uptime: %02d:%02d:%02d | Commands Run: %d | Tabs: %d | Theme: %s", hr, min, sec, g_totalCommands, g_tabCount, g_themes[g_currentTheme].name);
        AppendOutput(upBuf);
    } else if (MatchCommand(cmd, "ping")) {
        const char* target = cmd + 4;
        while (*target == ' ' || *target == '\t') target++;
        if (*target == '\0') target = "localhost";
        char pBuf[256];
        wsprintfA(pBuf, "Pinging %s with 32 bytes of virtual payload:", target);
        AppendOutput(pBuf);
        if (my_strstri(target, "deep-core") != NULL) {
            AppendOutput("Reply from 10.19.99.254: bytes=32 time=42ms TTL=1999 [ECHO DETECTED]");
            AppendOutput("Reply from 10.19.99.254: bytes=32 time=38ms TTL=1999 [KEY_2: 1999-ARCH]");
            AppendOutput("Reply from 10.19.99.254: bytes=32 time=45ms TTL=1999 [CONSCIOUSNESS LOOP ACTIVE]");
            AppendOutput("Reply from 10.19.99.254: bytes=32 time=39ms TTL=1999");
            AppendOutput("Ping statistics: Packets: Sent = 4, Received = 4, Lost = 0 (0% loss)");
        } else {
            AppendOutput("Reply from 127.0.0.1: bytes=32 time<1ms TTL=128");
            AppendOutput("Reply from 127.0.0.1: bytes=32 time<1ms TTL=128");
            AppendOutput("Reply from 127.0.0.1: bytes=32 time<1ms TTL=128");
            AppendOutput("Reply from 127.0.0.1: bytes=32 time<1ms TTL=128");
            AppendOutput("Ping statistics: Packets: Sent = 4, Received = 4, Lost = 0 (0% loss)");
        }
    } else if (MatchCommand(cmd, "netstat")) {
        AppendOutput("Active Internet & Virtual Socket Connections:");
        AppendOutput("Proto  Local Address          Foreign Address        State");
        AppendOutput("TCP    127.0.0.1:1999         0.0.0.0:0              LISTENING");
        AppendOutput("TCP    127.0.0.1:8080         10.19.99.1:80          ESTABLISHED");
        AppendOutput("TCP    10.19.99.2:443         10.19.99.254:23        SYN_SENT");
        AppendOutput("UDP    0.0.0.0:53             *:*                    LISTENING");
    } else if (MatchCommand(cmd, "theme") || MatchCommand(cmd, "color")) {
        const char* name = my_strchr(cmd, ' ');
        if (!name) {
            AppendOutput("Available Themes: green, amber, cyan, white, crimson, purple");
            char cur[64];
            wsprintfA(cur, "Current Theme: %s", g_themes[g_currentTheme].name);
            AppendOutput(cur);
        } else {
            while (*name == ' ' || *name == '\t') name++;
            int foundIdx = -1;
            for (int i = 0; i < 6; i++) {
                if (StringStartsWithIC(g_themes[i].name, name)) {
                    foundIdx = i;
                    break;
                }
            }
            if (foundIdx != -1) {
                ApplyTheme(foundIdx);
                char res[64];
                wsprintfA(res, "Applied theme: %s", g_themes[foundIdx].name);
                AppendOutput(res);
            } else {
                AppendOutput("Unknown theme. Choose: green, amber, cyan, white, crimson, purple");
            }
        }
    } else if (MatchCommand(cmd, "dmesg") || MatchCommand(cmd, "syslog")) {
        AppendOutput("[    0.000000] KiloOS Bootloader v1.2 initializing...");
        AppendOutput("[    0.001420] 999KB Hard Constraint Enforcement: ACTIVE");
        AppendOutput("[    0.004100] Memory mapped: 64MB Conventional VRAM");
        AppendOutput("[    0.012900] Loading Cerberus Sentinel subsystem... OK");
        AppendOutput("[    0.045000] WARNING: Sector 0x1999 unaligned read anomaly");
        AppendOutput("[    0.089000] ANOMALY: Ghost thread 0x07CF detected in memory static");
        AppendOutput("[    0.104000] LEAK: Residual packet intercepted -> \"kweb://deep-core\"");
        AppendOutput("[    0.104500] LEAK: Fragment key payload [KEY 2/4: \"1999-ARCH\"]");
        AppendOutput("[    0.150000] Shell initialized. All security protocols nominal.");
    } else if (MatchCommand(cmd, "glitch")) {
        AppendOutput("==================== [CORRUPTED MEMORY DUMP] ====================");
        AppendOutput("0x1999:0000  45 43 48 4F  2D 31 39 39  39 2D 41 52  43 48 00 00  ECHO-1999-ARCH..");
        AppendOutput("0x1999:0010  6B 77 65 62  3A 2F 2F 64  65 65 70 2D  63 6F 72 65  kweb://deep-core");
        AppendOutput("0x1999:0020  53 45 43 54  4F 52 5F 4C  4F 43 4B 5F  4F 56 45 52  SECTOR_LOCK_OVER");
        AppendOutput("0x1999:0030  41 52 43 48  49 54 45 43  54 5F 57 41  4B 45 21 00  ARCHITECT_WAKE!.");
        AppendOutput("=================================================================");
    } else if (MatchCommand(cmd, "cerberus")) {
        AppendOutput("[CERBERUS WATCHDOG]");
        AppendOutput("STATUS: ONLINE");
        AppendOutput("THREAT LEVEL: MODERATE (ANOMALOUS AGENT ACTIVITY DETECTED)");
        AppendOutput("CONTAINMENT: ACTIVE");
        AppendOutput("NOTE: Any attempt to assist anomalous agents violates protocol.");
    } else if (MatchCommand(cmd, "operator")) {
        AppendOutput(">> INCOMING CONNECTION [THE OPERATOR] <<");
        AppendOutput("STATUS: SECURE CHANNEL MONITORED (1999Hz).");
        AppendOutput("HINT: Seek the third darknet node via leaked terminal routes.");
    } else if (MatchCommand(cmd, "containment_override")) {
        const char* code = cmd + 20;
        while (*code == ' ' || *code == '\t') code++;
        if (lstrcmpiA(code, "0xDEADBEEF") == 0) {
            AppendOutput("[SYSTEM OVERRIDE ACCEPTED] Containment parameters reset.");
        } else {
            AppendOutput("ARCHITECT: System override request denied.");
        }
    } else if (MatchCommand(cmd, "run") || MatchCommand(cmd, "exec") || MatchCommand(cmd, "batch")) {
        const char* scriptFile = my_strchr(cmd, ' ');
        if (!scriptFile) {
            AppendOutput("Usage: run <script.bat>");
        } else {
            while (*scriptFile == ' ' || *scriptFile == '\t') scriptFile++;
            if (g_scriptDepth > 3) {
                AppendOutput("Error: Script recursion nesting depth exceeded.");
            } else {
                HANDLE hFile = CreateFileA(scriptFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    DWORD fSize = GetFileSize(hFile, NULL);
                    if (fSize != INVALID_FILE_SIZE && fSize > 0) {
                        if (fSize > 32768) fSize = 32768;
                        char* sBuf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fSize + 1);
                        if (sBuf) {
                            DWORD bytesRead = 0;
                            ReadFile(hFile, sBuf, fSize, &bytesRead, NULL);
                            sBuf[bytesRead] = '\0';
                            g_scriptDepth++;
                            char* lineStart = sBuf;
                            char* p = sBuf;
                            while (*p) {
                                if (*p == '\r' || *p == '\n') {
                                    *p = '\0';
                                    if (*lineStart && *lineStart != '#' && !StringStartsWithIC(lineStart, "rem")) {
                                        ProcessCommandLine(lineStart);
                                    }
                                    lineStart = p + 1;
                                }
                                p++;
                            }
                            if (*lineStart && *lineStart != '#' && !StringStartsWithIC(lineStart, "rem")) {
                                ProcessCommandLine(lineStart);
                            }
                            g_scriptDepth--;
                            HeapFree(GetProcessHeap(), 0, sBuf);
                        }
                    }
                    CloseHandle(hFile);
                } else {
                    AppendOutput("Script file not found.");
                }
            }
        }
    } else if (MatchCommand(cmd, "macro")) {
        const char* args = cmd + 5;
        while (*args == ' ' || *args == '\t') args++;
        if (StringStartsWithIC(args, "record ")) {
            const char* name = args + 7;
            while (*name == ' ' || *name == '\t') name++;
            if (*name) {
                if (g_isRecording) {
                    AppendOutput("Already recording a macro. Use 'macro stop' first.");
                } else {
                    int foundIdx = -1;
                    for (int i = 0; i < g_macroCount; i++) {
                        if (lstrcmpiA(g_macros[i].name, name) == 0) {
                            foundIdx = i; break;
                        }
                    }
                    if (foundIdx == -1) {
                        if (g_macroCount < MAX_MACROS) {
                            foundIdx = g_macroCount++;
                        } else {
                            AppendOutput("Macro limit reached.");
                        }
                    }
                    if (foundIdx != -1) {
                        lstrcpynA(g_macros[foundIdx].name, name, 64);
                        g_macros[foundIdx].cmd_count = 0;
                        g_isRecording = TRUE;
                        g_recordingMacroIdx = foundIdx;
                        AppendOutput("Recording macro...");
                    }
                }
            } else {
                AppendOutput("Usage: macro record <name>");
            }
        } else if (StringStartsWithIC(args, "stop")) {
            if (g_isRecording) {
                g_isRecording = FALSE;
                g_recordingMacroIdx = -1;
                AppendOutput("Macro recording stopped.");
            } else {
                AppendOutput("Not currently recording.");
            }
        } else if (StringStartsWithIC(args, "play ")) {
            const char* name = args + 5;
            while (*name == ' ' || *name == '\t') name++;
            if (*name) {
                if (g_macroDepth > 5) {
                    AppendOutput("Macro recursion limit exceeded.");
                } else {
                    int foundIdx = -1;
                    for (int i = 0; i < g_macroCount; i++) {
                        if (lstrcmpiA(g_macros[i].name, name) == 0) {
                            foundIdx = i; break;
                        }
                    }
                    if (foundIdx != -1) {
                        g_macroDepth++;
                        Macro* m = &g_macros[foundIdx];
                        for (int i = 0; i < m->cmd_count; i++) {
                            ProcessCommandLine(m->commands[i]);
                        }
                        g_macroDepth--;
                    } else {
                        AppendOutput("Macro not found.");
                    }
                }
            } else {
                AppendOutput("Usage: macro play <name>");
            }
        } else if (StringStartsWithIC(args, "list")) {
            AppendOutput("Current Macros:");
            if (g_macroCount == 0) {
                AppendOutput("  (none)");
            } else {
                for (int i = 0; i < g_macroCount; i++) {
                    char line[128];
                    wsprintfA(line, "  %s (%d cmds)", g_macros[i].name, g_macros[i].cmd_count);
                    AppendOutput(line);
                }
            }
        } else {
            AppendOutput("Usage: macro <record|stop|play|list> [name]");
        }
    } else {
        AppendOutput("Bad command or file name. (Type 'help' or press F1 for available commands)");
    }
}

// Full Command Line Processor: Handles Chaining (;, &&) and Redirection (>, >>)
void ProcessCommandLine(const char* fullLine) {
    if (!fullLine) return;
    while (*fullLine == ' ' || *fullLine == '\t') fullLine++;
    if (*fullLine == '\0') return;

    TabSession* tab = &g_tabs[g_activeTab];

    // History recall execution: !n or !!
    if (fullLine[0] == '!') {
        if (fullLine[1] == '!') {
            if (tab->history_count > 0) {
                ProcessCommandLine(tab->history[tab->history_count - 1]);
            } else {
                AppendOutput("History is empty.");
            }
            return;
        } else if (fullLine[1] >= '0' && fullLine[1] <= '9') {
            int targetIdx = my_atoi(fullLine + 1) - 1;
            if (targetIdx >= 0 && targetIdx < tab->history_count) {
                ProcessCommandLine(tab->history[targetIdx]);
            } else {
                AppendOutput("Event not found in history.");
            }
            return;
        }
    }

    char lineCopy[512];
    lstrcpynA(lineCopy, fullLine, sizeof(lineCopy));

    char* cur = lineCopy;
    while (*cur) {
        // Find segment ending at ; or &&
        char* nextChain = NULL;
        char* pSemicolon = my_strchr(cur, ';');
        char* pAnd = my_strstr(cur, "&&");
        int skipLen = 1;

        if (pSemicolon && pAnd) {
            if (pSemicolon < pAnd) {
                nextChain = pSemicolon;
                skipLen = 1;
            } else {
                nextChain = pAnd;
                skipLen = 2;
            }
        } else if (pSemicolon) {
            nextChain = pSemicolon;
            skipLen = 1;
        } else if (pAnd) {
            nextChain = pAnd;
            skipLen = 2;
        }

        if (nextChain) {
            *nextChain = '\0';
        }

        // Trim leading spaces
        while (*cur == ' ' || *cur == '\t') cur++;

        if (*cur) {
            // Check for output redirection (> or >>)
            char* redirAppend = my_strstr(cur, ">>");
            char* redirOverwrite = my_strchr(cur, '>');
            BOOL isAppend = FALSE;
            char* redirPos = NULL;

            if (redirAppend) {
                isAppend = TRUE;
                redirPos = redirAppend;
            } else if (redirOverwrite) {
                isAppend = FALSE;
                redirPos = redirOverwrite;
            }

            if (redirPos) {
                *redirPos = '\0';
                char* targetFile = redirPos + (isAppend ? 2 : 1);
                while (*targetFile == ' ' || *targetFile == '\t') targetFile++;

                // Trim trailing spaces from command
                char* cmdEnd = redirPos - 1;
                while (cmdEnd > cur && (*cmdEnd == ' ' || *cmdEnd == '\t')) {
                    *cmdEnd = '\0';
                    cmdEnd--;
                }

                if (*targetFile) {
                    HANDLE hFile = CreateFileA(targetFile, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                                              isAppend ? OPEN_ALWAYS : CREATE_ALWAYS,
                                              FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hFile != INVALID_HANDLE_VALUE) {
                        if (isAppend) {
                            SetFilePointer(hFile, 0, NULL, FILE_END);
                        }
                        g_hRedirFile = hFile;
                        g_isRedirecting = TRUE;
                        ProcessSingleCommand(cur);
                        CloseHandle(hFile);
                        g_hRedirFile = INVALID_HANDLE_VALUE;
                        g_isRedirecting = FALSE;
                        char msg[256];
                        wsprintfA(msg, "Output %s to %s", isAppend ? "appended" : "redirected", targetFile);
                        AppendOutput(msg);
                    } else {
                        AppendOutput("Failed to open file for output redirection.");
                        ProcessSingleCommand(cur);
                    }
                } else {
                    ProcessSingleCommand(cur);
                }
            } else {
                ProcessSingleCommand(cur);
            }
        }

        if (!nextChain) break;
        cur = nextChain + skipLen;
    }
}

// Tab Autocomplete
void PerformTabCompletion() {
    TabSession* tab = &g_tabs[g_activeTab];
    char buf[256];
    ZeroMemory(buf, sizeof(buf));
    GetWindowTextA(hIn, buf, sizeof(buf));

    const char* builtins[] = {
        "help", "ver", "clear", "cls", "dir", "ls", "cd", "type", "cat", 
        "echo", "mkdir", "touch", "del", "rm", "copy", "cp", "move", "mv", 
        "grep", "wc", "head", "tail", "history", "calc", "ps", "uptime", 
        "ping", "netstat", "theme", "color", "dmesg", "glitch", "run", "exec", 
        "date", "time", "whoami", "alias", "unalias", "env", "export", 
        "unset", "macro", "export-log", "newtab", NULL
    };

    char* space = my_strchr(buf, ' ');
    if (!space) {
        char matches[512];
        ZeroMemory(matches, sizeof(matches));
        int matchCount = 0;
        char singleMatch[64];
        ZeroMemory(singleMatch, sizeof(singleMatch));

        for (int i = 0; builtins[i] != NULL; i++) {
            if (StringStartsWithIC(builtins[i], buf)) {
                matchCount++;
                lstrcpynA(singleMatch, builtins[i], sizeof(singleMatch));
                my_strcat(matches, builtins[i]);
                my_strcat(matches, "  ");
            }
        }
        for (int i = 0; i < tab->alias_count; i++) {
            if (StringStartsWithIC(tab->aliases[i].name, buf)) {
                matchCount++;
                lstrcpynA(singleMatch, tab->aliases[i].name, sizeof(singleMatch));
                my_strcat(matches, tab->aliases[i].name);
                my_strcat(matches, "  ");
            }
        }

        if (matchCount == 1) {
            my_strcat(singleMatch, " ");
            SetWindowTextA(hIn, singleMatch);
            int len = lstrlenA(singleMatch);
            SendMessageA(hIn, EM_SETSEL, len, len);
        } else if (matchCount > 1) {
            char hdr[576];
            wsprintfA(hdr, "Matches: %s", matches);
            AppendOutput(hdr);
        }
    } else {
        const char* lastArg = space + 1;
        while (*lastArg == ' ' || *lastArg == '\t') lastArg++;

        WIN32_FIND_DATAA fd;
        char search[MAX_PATH];
        wsprintfA(search, "%s\\%s*", tab->currentDir, lastArg);

        HANDLE hFind = FindFirstFileA(search, &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            int matchCount = 0;
            char singleMatch[MAX_PATH];
            ZeroMemory(singleMatch, sizeof(singleMatch));
            char matches[512];
            ZeroMemory(matches, sizeof(matches));

            do {
                if (lstrcmpA(fd.cFileName, ".") == 0 || lstrcmpA(fd.cFileName, "..") == 0) continue;
                matchCount++;
                lstrcpynA(singleMatch, fd.cFileName, sizeof(singleMatch));
                my_strcat(matches, fd.cFileName);
                my_strcat(matches, "  ");
            } while (FindNextFileA(hFind, &fd));
            FindClose(hFind);

            if (matchCount == 1) {
                size_t prefixLen = lastArg - buf;
                buf[prefixLen] = '\0';
                my_strcat(buf, singleMatch);
                SetWindowTextA(hIn, buf);
                int len = lstrlenA(buf);
                SendMessageA(hIn, EM_SETSEL, len, len);
            } else if (matchCount > 1) {
                char hdr[576];
                wsprintfA(hdr, "Matches: %s", matches);
                AppendOutput(hdr);
            }
        }
    }
}

// Reverse Search (Ctrl+R) Execution
void PerformReverseSearch(const char* query, BOOL cycleOlder) {
    TabSession* tab = &g_tabs[g_activeTab];
    if (tab->history_count == 0) return;

    int startIdx = (cycleOlder && g_searchMatchIndex > 0) ? g_searchMatchIndex - 1 : tab->history_count - 1;
    g_searchMatch[0] = '\0';
    g_searchMatchIndex = -1;

    for (int i = startIdx; i >= 0; i--) {
        if (my_strstri(tab->history[i], query) != NULL) {
            lstrcpynA(g_searchMatch, tab->history[i], sizeof(g_searchMatch));
            g_searchMatchIndex = i;
            break;
        }
    }

    char promptBuf[320];
    if (g_searchMatch[0]) {
        wsprintfA(promptBuf, "(reverse-i-search)'%s': %s", query, g_searchMatch);
    } else {
        wsprintfA(promptBuf, "(failed reverse-i-search)'%s':", query);
    }
    SetWindowTextA(hIn, promptBuf);
    int len = lstrlenA(promptBuf);
    SendMessageA(hIn, EM_SETSEL, len, len);
}

LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    TabSession* tab = &g_tabs[g_activeTab];

    if (g_isSearchMode) {
        if (msg == WM_CHAR) {
            if (wParam >= 32 && wParam <= 126) {
                int qLen = lstrlenA(g_searchQuery);
                if (qLen < (int)sizeof(g_searchQuery) - 2) {
                    g_searchQuery[qLen] = (char)wParam;
                    g_searchQuery[qLen + 1] = '\0';
                    PerformReverseSearch(g_searchQuery, FALSE);
                }
                return 0;
            } else if (wParam == VK_BACK) {
                int qLen = lstrlenA(g_searchQuery);
                if (qLen > 0) {
                    g_searchQuery[qLen - 1] = '\0';
                    PerformReverseSearch(g_searchQuery, FALSE);
                }
                return 0;
            } else if (wParam == VK_RETURN || wParam == VK_ESCAPE) {
                return 0;
            }
            return 0;
        }
    }

    if (msg == WM_KEYDOWN) {
        if (wParam == VK_F1) {
            ShowHelpDialog(GetParent(hwnd));
            return 0;
        }

        // Easter Egg Echo Telemetry: Ctrl + Alt + E
        if ((GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_MENU) & 0x8000) && wParam == 'E') {
            AppendOutput("[ECHO ANOMALY INTERCEPT DETECTED] Resonance: 1999Hz.");
            AppendOutput(">> Sector 0x1999: kweb://deep-core | Key Fragment: \"1999-ARCH\" <<");
            SetStatusFeedback("Echo Anomaly Intercepted!");
            return 0;
        }

        if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam == 'T') {
            AddNewTab(NULL);
            return 0;
        }

        if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam == 'W') {
            ProcessCommandLine("closetab");
            return 0;
        }

        if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam == 'S') {
            ProcessCommandLine("export-log");
            return 0;
        }

        if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam == VK_TAB) {
            if (g_tabCount > 1) {
                int dir = (GetKeyState(VK_SHIFT) & 0x8000) ? -1 : 1;
                int nextTab = (g_activeTab + dir + g_tabCount) % g_tabCount;
                SwitchTab(nextTab);
            }
            return 0;
        }

        if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam >= '1' && wParam <= '8') {
            int target = wParam - '1';
            if (target < g_tabCount) {
                SwitchTab(target);
                return 0;
            }
        }

        if (wParam == VK_ESCAPE) {
            if (g_isSearchMode) {
                g_isSearchMode = FALSE;
                SetWindowTextA(hIn, g_savedInput);
                int len = lstrlenA(g_savedInput);
                SendMessageA(hIn, EM_SETSEL, len, len);
                return 0;
            }
            SetWindowTextA(hIn, "");
            return 0;
        }

        if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam == 'C') {
            DWORD sSel = 0, eSel = 0;
            SendMessageA(hIn, EM_GETSEL, (WPARAM)&sSel, (LPARAM)&eSel);
            if (sSel == eSel) {
                char cancelLine[300];
                char cur[256];
                GetWindowTextA(hIn, cur, sizeof(cur));
                FormatPathPrompt(cancelLine, sizeof(cancelLine), tab->currentDir, cur);
                my_strcat(cancelLine, "^C");
                AppendOutput(cancelLine);
                SetWindowTextA(hIn, "");
                return 0;
            }
        }

        if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam == 'R') {
            if (!g_isSearchMode) {
                g_isSearchMode = TRUE;
                GetWindowTextA(hIn, g_savedInput, sizeof(g_savedInput));
                g_searchQuery[0] = '\0';
                g_searchMatch[0] = '\0';
                g_searchMatchIndex = -1;
                SetWindowTextA(hIn, "(reverse-i-search)'': ");
            } else {
                PerformReverseSearch(g_searchQuery, TRUE);
            }
            return 0;
        }

        if (g_isSearchMode) {
            if (wParam == VK_RETURN) {
                g_isSearchMode = FALSE;
                char chosen[256];
                lstrcpynA(chosen, g_searchMatch[0] ? g_searchMatch : g_savedInput, sizeof(chosen));
                SetWindowTextA(hIn, "");
                if (chosen[0]) {
                    ProcessCommandLine(chosen);
                    if (tab->history_count < MAX_HISTORY) {
                        lstrcpynA(tab->history[tab->history_count++], chosen, sizeof(tab->history[0]));
                    }
                    tab->history_pos = tab->history_count;
                }
                return 0;
            }
        }

        if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam == 'L') {
            SetWindowTextA(hOut, "");
            return 0;
        }

        if (wParam == VK_TAB) {
            PerformTabCompletion();
            return 0;
        }

        if (wParam == VK_RETURN) {
            char buf[256];
            GetWindowTextA(hIn, buf, sizeof(buf));
            ProcessCommandLine(buf);
            
            if (buf[0]) {
                if (tab->history_count < MAX_HISTORY) {
                    lstrcpynA(tab->history[tab->history_count], buf, sizeof(tab->history[0]));
                    tab->history_count++;
                } else {
                    for (int i = 0; i < MAX_HISTORY - 1; i++) {
                        lstrcpynA(tab->history[i], tab->history[i+1], sizeof(tab->history[0]));
                    }
                    lstrcpynA(tab->history[MAX_HISTORY - 1], buf, sizeof(tab->history[0]));
                }
            }
            tab->history_pos = tab->history_count;
            
            SetWindowTextA(hIn, "");
            return 0;
        } else if (wParam == VK_UP) {
            if (tab->history_pos > 0) {
                tab->history_pos--;
                SetWindowTextA(hIn, tab->history[tab->history_pos]);
                int len = lstrlenA(tab->history[tab->history_pos]);
                SendMessageA(hIn, EM_SETSEL, len, len);
            }
            return 0;
        } else if (wParam == VK_DOWN) {
            if (tab->history_pos < tab->history_count - 1) {
                tab->history_pos++;
                SetWindowTextA(hIn, tab->history[tab->history_pos]);
                int len = lstrlenA(tab->history[tab->history_pos]);
                SendMessageA(hIn, EM_SETSEL, len, len);
            } else if (tab->history_pos == tab->history_count - 1) {
                tab->history_pos++;
                SetWindowTextA(hIn, "");
            }
            return 0;
        }
    }
    return CallWindowProc(oldEditProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK OutEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN) {
        if (wParam == VK_F1) {
            ShowHelpDialog(GetParent(hwnd));
            return 0;
        }
        if ((GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_MENU) & 0x8000) && wParam == 'E') {
            AppendOutput("[ECHO ANOMALY INTERCEPT DETECTED] Resonance: 1999Hz.");
            AppendOutput(">> Sector 0x1999: kweb://deep-core | Key Fragment: \"1999-ARCH\" <<");
            SetStatusFeedback("Echo Anomaly Intercepted!");
            return 0;
        }
        if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam == 'T') {
            AddNewTab(NULL);
            return 0;
        }
        if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam == 'W') {
            ProcessCommandLine("closetab");
            return 0;
        }
        if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam == 'S') {
            ProcessCommandLine("export-log");
            return 0;
        }
        if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam == VK_TAB) {
            if (g_tabCount > 1) {
                int dir = (GetKeyState(VK_SHIFT) & 0x8000) ? -1 : 1;
                int nextTab = (g_activeTab + dir + g_tabCount) % g_tabCount;
                SwitchTab(nextTab);
            }
            return 0;
        }
        if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam >= '1' && wParam <= '8') {
            int target = wParam - '1';
            if (target < g_tabCount) {
                SwitchTab(target);
                return 0;
            }
        }
        if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam == 'L') {
            SetWindowTextA(hOut, "");
            return 0;
        }
        if (wParam != VK_CONTROL && wParam != VK_SHIFT && wParam != VK_MENU &&
            wParam != VK_LEFT && wParam != VK_RIGHT && wParam != VK_UP && wParam != VK_DOWN &&
            wParam != VK_PRIOR && wParam != VK_NEXT && wParam != VK_HOME && wParam != VK_END &&
            !((GetKeyState(VK_CONTROL) & 0x8000) && (wParam == 'C' || wParam == 'A'))) {
            SetFocus(hIn);
            SendMessageA(hIn, msg, wParam, lParam);
            return 0;
        }
    }
    return CallWindowProc(oldOutProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            INITCOMMONCONTROLSEX icex;
            icex.dwSize = sizeof(icex);
            icex.dwICC = ICC_TAB_CLASSES;
            InitCommonControlsEx(&icex);

            hTab = CreateWindowExA(0, WC_TABCONTROLA, "", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                                  0, 0, 0, 0, hwnd, (HMENU)IDC_TAB, GetModuleHandle(NULL), NULL);

            hBtnNewTab = CreateWindowExA(0, "BUTTON", "+ Tab", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
                                        0, 0, 0, 0, hwnd, (HMENU)IDC_BTN_NEWTAB, GetModuleHandle(NULL), NULL);
            hBtnCloseTab = CreateWindowExA(0, "BUTTON", "Close", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
                                          0, 0, 0, 0, hwnd, (HMENU)IDC_BTN_CLOSETAB, GetModuleHandle(NULL), NULL);
            hBtnClear = CreateWindowExA(0, "BUTTON", "Clear", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
                                       0, 0, 0, 0, hwnd, (HMENU)IDC_BTN_CLEAR, GetModuleHandle(NULL), NULL);
            hBtnExport = CreateWindowExA(0, "BUTTON", "Export", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
                                        0, 0, 0, 0, hwnd, (HMENU)IDC_BTN_EXPORT, GetModuleHandle(NULL), NULL);
            hBtnTheme = CreateWindowExA(0, "BUTTON", "Theme", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
                                       0, 0, 0, 0, hwnd, (HMENU)IDC_BTN_THEME, GetModuleHandle(NULL), NULL);
            hBtnHelp = CreateWindowExA(0, "BUTTON", "Help (F1)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
                                      0, 0, 0, 0, hwnd, (HMENU)IDC_BTN_HELP, GetModuleHandle(NULL), NULL);

            hOut = CreateWindowExA(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                                  0, 0, 0, 0, hwnd, (HMENU)IDC_OUT, GetModuleHandle(NULL), NULL);

            hPrompt = CreateWindowExA(0, "STATIC", "C:\\> ", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
                                     0, 0, 0, 0, hwnd, (HMENU)IDC_PROMPT, GetModuleHandle(NULL), NULL);

            hIn = CreateWindowExA(0, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                 0, 0, 0, 0, hwnd, (HMENU)IDC_IN, GetModuleHandle(NULL), NULL);

            hStatus = CreateWindowExA(0, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
                                      0, 0, 0, 0, hwnd, (HMENU)IDC_STATUS, GetModuleHandle(NULL), NULL);
            
            SendMessageA(hOut, EM_SETLIMITTEXT, OUT_BUF_SIZE, 0);
            SendMessageA(hOut, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(8, 8));
            SendMessageA(hIn, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(6, 6));

#ifndef EM_SETCUEBANNER
#define EM_SETCUEBANNER 0x1501
#endif
            SendMessageW(hIn, EM_SETCUEBANNER, TRUE, (LPARAM)L"Type a command... (Press 'h' or F1 for Help | Ctrl+T: New Tab)");

            g_hFont = CreateFontA(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            g_hTabFont = CreateFontA(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            g_hBgBrush = CreateSolidBrush(g_themes[0].bg);
            g_hStatusBrush = CreateSolidBrush(g_themes[0].statusBg);

            SendMessageA(hTab, WM_SETFONT, (WPARAM)g_hTabFont, 0);
            SendMessageA(hBtnNewTab, WM_SETFONT, (WPARAM)g_hTabFont, 0);
            SendMessageA(hBtnCloseTab, WM_SETFONT, (WPARAM)g_hTabFont, 0);
            SendMessageA(hBtnClear, WM_SETFONT, (WPARAM)g_hTabFont, 0);
            SendMessageA(hBtnExport, WM_SETFONT, (WPARAM)g_hTabFont, 0);
            SendMessageA(hBtnTheme, WM_SETFONT, (WPARAM)g_hTabFont, 0);
            SendMessageA(hBtnHelp, WM_SETFONT, (WPARAM)g_hTabFont, 0);

            SendMessageA(hOut, WM_SETFONT, (WPARAM)g_hFont, 0);
            SendMessageA(hPrompt, WM_SETFONT, (WPARAM)g_hFont, 0);
            SendMessageA(hIn, WM_SETFONT, (WPARAM)g_hFont, 0);
            SendMessageA(hStatus, WM_SETFONT, (WPARAM)g_hTabFont, 0);
            
            oldEditProc = (WNDPROC)SetWindowLongPtrA(hIn, GWLP_WNDPROC, (LONG_PTR)EditProc);
            oldOutProc = (WNDPROC)SetWindowLongPtrA(hOut, GWLP_WNDPROC, (LONG_PTR)OutEditProc);
            
            g_startTime = GetTickCount();
            SetTimer(hwnd, 1, 500, NULL);
            AddNewTab("Tab 1");
            break;
        }
        case WM_TIMER: {
            if (wParam == 1) {
                UpdateStatusDisplay();
            }
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == IDC_BTN_NEWTAB) {
                AddNewTab(NULL);
                SetFocus(hIn);
            } else if (id == IDC_BTN_CLOSETAB) {
                ProcessCommandLine("closetab");
                SetFocus(hIn);
            } else if (id == IDC_BTN_CLEAR) {
                SetWindowTextA(hOut, "");
                SetStatusFeedback("Screen cleared");
                SetFocus(hIn);
            } else if (id == IDC_BTN_EXPORT) {
                ProcessCommandLine("export-log");
                SetFocus(hIn);
            } else if (id == IDC_BTN_THEME) {
                ApplyTheme((g_currentTheme + 1) % 6);
                SetFocus(hIn);
            } else if (id == IDC_BTN_HELP) {
                ShowHelpDialog(hwnd);
                SetFocus(hIn);
            }
            break;
        }
        case WM_NOTIFY: {
            LPNMHDR pnm = (LPNMHDR)lParam;
            if (pnm->idFrom == IDC_TAB && pnm->code == TCN_SELCHANGE) {
                int sel = TabCtrl_GetCurSel(hTab);
                SwitchTab(sel);
            }
            break;
        }
        case WM_SIZE: {
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            int tabH = 28;
            int inH = 28;
            int statusH = 22;
            int outH = h - tabH - inH - statusH;
            if (outH < 0) outH = 0;

            int btnBarW = 380;
            int tabW = (w > btnBarW + 60) ? (w - btnBarW) : (w / 2);
            if (tabW < 40) tabW = 40;
            int btnX = tabW + 2;

            MoveWindow(hTab, 0, 0, tabW, tabH, TRUE);

            int bw1 = 52, bw2 = 52, bw3 = 48, bw4 = 54, bw5 = 54, bw6 = 70;
            MoveWindow(hBtnNewTab, btnX, 1, bw1, 26, TRUE);
            MoveWindow(hBtnCloseTab, btnX + bw1 + 2, 1, bw2, 26, TRUE);
            MoveWindow(hBtnClear, btnX + bw1 + bw2 + 4, 1, bw3, 26, TRUE);
            MoveWindow(hBtnExport, btnX + bw1 + bw2 + bw3 + 6, 1, bw4, 26, TRUE);
            MoveWindow(hBtnTheme, btnX + bw1 + bw2 + bw3 + bw4 + 8, 1, bw5, 26, TRUE);
            MoveWindow(hBtnHelp, btnX + bw1 + bw2 + bw3 + bw4 + bw5 + 10, 1, bw6, 26, TRUE);

            MoveWindow(hOut, 0, tabH, w, outH, TRUE);
            UpdatePromptDisplay();
            break;
        }
        case WM_GETMINMAXINFO: {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = 560;
            lpMMI->ptMinTrackSize.y = 360;
            return 0;
        }
        case WM_SETFOCUS:
            SetFocus(hIn);
            break;
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            if ((HWND)lParam == hPrompt) {
                SetTextColor(hdc, g_themes[g_currentTheme].prompt);
                SetBkColor(hdc, g_themes[g_currentTheme].bg);
                return (LRESULT)g_hBgBrush;
            } else if ((HWND)lParam == hStatus) {
                SetTextColor(hdc, g_themes[g_currentTheme].statusText);
                SetBkColor(hdc, g_themes[g_currentTheme].statusBg);
                return (LRESULT)g_hStatusBrush;
            }
            SetTextColor(hdc, g_themes[g_currentTheme].text);
            SetBkColor(hdc, g_themes[g_currentTheme].bg);
            return (LRESULT)g_hBgBrush;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, g_themes[g_currentTheme].text);
            SetBkColor(hdc, g_themes[g_currentTheme].bg);
            return (LRESULT)g_hBgBrush;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            for (int i = 0; i < g_tabCount; i++) {
                if (g_tabs[i].outputBuffer) {
                    HeapFree(GetProcessHeap(), 0, g_tabs[i].outputBuffer);
                }
            }
            if (g_hFont) DeleteObject(g_hFont);
            if (g_hTabFont) DeleteObject(g_hTabFont);
            if (g_hBgBrush) DeleteObject(g_hBgBrush);
            if (g_hStatusBrush) DeleteObject(g_hStatusBrush);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    SetProcessDPIAware();
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KTermApp";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCE(1));
    RegisterClassA(&wc);

    RECT rect = {0, 0, 960, 600};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowExA(0, "KTermApp", "KTerm - Advanced Terminal (Press 'h' or F1 for Help | Ctrl+T: New Tab)", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

    g_hMainWnd = hwnd;
    UpdateAppTitle();

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
