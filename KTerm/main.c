#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#define IDC_TAB 100
#define IDC_OUT 101
#define IDC_IN  102

#define MAX_TABS 8
#define MAX_HISTORY 25
#define MAX_ALIASES 20
#define MAX_ENV 20
#define OUT_BUF_SIZE 262144

#pragma function(memset)
void* memset(void* dest, int c, size_t count) {
    unsigned char* p = (unsigned char*)dest;
    while (count--) {
        *p++ = (unsigned char)c;
    }
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

HWND hTab, hOut, hIn;
WNDPROC oldEditProc;

TabSession g_tabs[MAX_TABS];
int g_tabCount = 0;
int g_activeTab = 0;

// Reverse Search State
BOOL g_isSearchMode = FALSE;
char g_searchQuery[128];
char g_searchMatch[256];
int g_searchMatchIndex = -1;
char g_savedInput[256];

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

static void FormatPathPrompt(char* dst, size_t dstSize, const char* dir, const char* cmd) {
    char tmp[1024];
    wsprintfA(tmp, "%s> %s", dir ? dir : "", cmd ? cmd : "");
    lstrcpynA(dst, tmp, (int)dstSize);
}

void AppendOutput(const char* text) {
    if (!text) return;
    int len = GetWindowTextLengthA(hOut);
    SendMessageA(hOut, EM_SETSEL, len, len);
    SendMessageA(hOut, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessageA(hOut, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
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
    tab->alias_count = 4;

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
    if (g_tabCount >= MAX_TABS - 1) return;
    
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
    wsprintfA(banner, "KiloOS Terminal v1.2 [%s]\r\nType 'help' for a list of commands.", nameBuf);
    
    if (g_tabCount == 1) {
        AppendOutput(banner);
    } else {
        lstrcpynA(g_tabs[tabIndex].outputBuffer, banner, OUT_BUF_SIZE);
        TabCtrl_SetCurSel(hTab, tabIndex);
        SaveActiveTabOutput();
        g_activeTab = tabIndex;
        LoadTabOutput(g_activeTab);
    }
}

void SwitchTab(int newIdx) {
    if (newIdx < 0 || newIdx >= g_tabCount || newIdx == g_activeTab) return;
    SaveActiveTabOutput();
    g_activeTab = newIdx;
    LoadTabOutput(g_activeTab);
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
        ZeroMemory(result, sizeof(result));
        char* pos = temp;
        char* found = NULL;
        while ((found = my_strstr(pos, target1)) != NULL || (found = my_strstr(pos, target2)) != NULL) {
            size_t prefixLen = found - pos;
            size_t matchLen = (found[0] == '%') ? lstrlenA(target1) : lstrlenA(target2);
            
            my_strncat(result, pos, prefixLen);
            my_strcat(result, tab->envVars[i].value);
            pos = found + matchLen;
        }
        my_strcat(result, pos);
        lstrcpynA(temp, result, sizeof(temp));
    }
    lstrcpynA(outBuf, temp, (int)outSize);
}

void ProcessCommand(const char* rawCmd) {
    if (!rawCmd) return;
    while (*rawCmd == ' ' || *rawCmd == '\t') rawCmd++;
    if (*rawCmd == '\0') return;

    TabSession* tab = &g_tabs[g_activeTab];

    char cmdWithAlias[512];
    ExpandAlias(rawCmd, cmdWithAlias, sizeof(cmdWithAlias));

    char cmd[512];
    ExpandEnvVars(cmdWithAlias, cmd, sizeof(cmd));

    char fullCmd[512];
    FormatPathPrompt(fullCmd, sizeof(fullCmd), tab->currentDir, rawCmd);
    AppendOutput(fullCmd);

    if (lstrcmpiA(cmd, "help") == 0) {
        AppendOutput("KTerm Commands:");
        AppendOutput("  help       - Show available commands");
        AppendOutput("  ver        - Show OS version");
        AppendOutput("  clear/cls  - Clear screen");
        AppendOutput("  dir/ls     - List files");
        AppendOutput("  cd         - Change directory");
        AppendOutput("  echo       - Print text (supports %VAR% & $VAR)");
        AppendOutput("  mkdir      - Create directory");
        AppendOutput("  type/cat   - Read file");
        AppendOutput("  date       - Show date");
        AppendOutput("  time       - Show time");
        AppendOutput("  whoami     - Show current user");
        AppendOutput("  alias      - Custom aliases (alias name=cmd, unalias)");
        AppendOutput("  env/export - Environment variables (export VAR=val, unset)");
        AppendOutput("  export-log - Export output log to file");
        AppendOutput("  newtab     - Open new terminal tab session");
    } else if (lstrcmpiA(cmd, "ver") == 0) {
        AppendOutput("KiloOS Native v1.2 (Multi-Tab & Deep Utilities)");
    } else if (lstrcmpiA(cmd, "date") == 0) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        char buf[64];
        wsprintfA(buf, "%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
        AppendOutput(buf);
    } else if (lstrcmpiA(cmd, "time") == 0) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        char buf[64];
        wsprintfA(buf, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
        AppendOutput(buf);
    } else if (lstrcmpiA(cmd, "whoami") == 0) {
        for (int i = 0; i < tab->env_count; i++) {
            if (lstrcmpiA(tab->envVars[i].name, "USER") == 0) {
                AppendOutput(tab->envVars[i].value);
                return;
            }
        }
        AppendOutput("kilo_user");
    } else if (lstrcmpiA(cmd, "clear") == 0 || lstrcmpiA(cmd, "cls") == 0) {
        SetWindowTextA(hOut, "");
    } else if (StringStartsWithIC(cmd, "newtab")) {
        const char* title = cmd + 6;
        while (*title == ' ' || *title == '\t') title++;
        AddNewTab(title);
    } else if (StringStartsWithIC(cmd, "export-log")) {
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
    } else if (StringStartsWithIC(cmd, "alias ")) {
        const char* expr = cmd + 6;
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
    } else if (StringStartsWithIC(cmd, "unalias ")) {
        const char* target = cmd + 8;
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
    } else if (StringStartsWithIC(cmd, "export ")) {
        const char* expr = cmd + 7;
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
    } else if (StringStartsWithIC(cmd, "unset ")) {
        const char* target = cmd + 6;
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
    } else if (lstrcmpiA(cmd, "dir") == 0 || lstrcmpiA(cmd, "ls") == 0) {
        WIN32_FIND_DATAA fd;
        char search[MAX_PATH + 16];
        char tmpSearch[1024];
        wsprintfA(tmpSearch, "%s\\*", tab->currentDir);
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
    } else if (StringStartsWithIC(cmd, "cd")) {
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
                } else {
                    AppendOutput("Directory not found.");
                }
            }
        } else {
            AppendOutput("Bad command or file name.");
        }
    } else if (StringStartsWithIC(cmd, "echo")) {
        const char* text = cmd + 4;
        if (*text == ' ' || *text == '\t' || *text == '\0') {
            while (*text == ' ' || *text == '\t') text++;
            AppendOutput(text);
        } else {
            AppendOutput("Bad command or file name.");
        }
    } else if (StringStartsWithIC(cmd, "mkdir")) {
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
    } else if (StringStartsWithIC(cmd, "type") || StringStartsWithIC(cmd, "cat")) {
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
                        if (fileSize > 4096) fileSize = 4096;
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
    } else {
        AppendOutput("Bad command or file name.");
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
        "echo", "mkdir", "date", "time", "whoami", "alias", "unalias", 
        "env", "export", "unset", "export-log", "newtab", NULL
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
        if (my_strstr(tab->history[i], query) != NULL) {
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

    if (msg == WM_KEYDOWN) {
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
            if (wParam == VK_ESCAPE) {
                g_isSearchMode = FALSE;
                SetWindowTextA(hIn, g_savedInput);
                return 0;
            }
            if (wParam == VK_RETURN) {
                g_isSearchMode = FALSE;
                if (g_searchMatch[0]) {
                    ProcessCommand(g_searchMatch);
                    if (tab->history_count < MAX_HISTORY) {
                        lstrcpynA(tab->history[tab->history_count++], g_searchMatch, sizeof(tab->history[0]));
                    }
                    tab->history_pos = tab->history_count;
                }
                SetWindowTextA(hIn, "");
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
            ProcessCommand(buf);
            
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            INITCOMMONCONTROLSEX icex;
            icex.dwSize = sizeof(icex);
            icex.dwICC = ICC_TAB_CLASSES;
            InitCommonControlsEx(&icex);

            hTab = CreateWindowExA(0, WC_TABCONTROLA, "", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                                  0, 0, 0, 0, hwnd, (HMENU)IDC_TAB, GetModuleHandle(NULL), NULL);

            hOut = CreateWindowExA(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                                  0, 0, 0, 0, hwnd, (HMENU)IDC_OUT, GetModuleHandle(NULL), NULL);

            hIn = CreateWindowExA(0, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                 0, 0, 0, 0, hwnd, (HMENU)IDC_IN, GetModuleHandle(NULL), NULL);
            
            SendMessageA(hOut, EM_SETLIMITTEXT, OUT_BUF_SIZE, 0);

            HFONT hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
            SendMessageA(hTab, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
            SendMessageA(hOut, WM_SETFONT, (WPARAM)hFont, 0);
            SendMessageA(hIn, WM_SETFONT, (WPARAM)hFont, 0);
            
            oldEditProc = (WNDPROC)SetWindowLongPtrA(hIn, GWLP_WNDPROC, (LONG_PTR)EditProc);
            
            AddNewTab("Tab 1");
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
            int tabH = 26;
            int inH = 24;
            int outH = h - tabH - inH;
            if (outH < 0) outH = 0;

            MoveWindow(hTab, 0, 0, w, tabH, TRUE);
            MoveWindow(hOut, 0, tabH, w, outH, TRUE);
            MoveWindow(hIn, 0, tabH + outH, w, inH, TRUE);
            break;
        }
        case WM_SETFOCUS:
            SetFocus(hIn);
            break;
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(0, 255, 0));
            SetBkColor(hdc, RGB(0, 0, 0));
            return (LRESULT)GetStockObject(BLACK_BRUSH);
        }
        case WM_DESTROY:
            for (int i = 0; i < g_tabCount; i++) {
                if (g_tabs[i].outputBuffer) {
                    HeapFree(GetProcessHeap(), 0, g_tabs[i].outputBuffer);
                }
            }
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KTermApp";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCE(1));
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(0, "KTermApp", "KTerm", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 440, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
