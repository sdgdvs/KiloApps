#include <windows.h>
#include <tlhelp32.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

HWND hListBox = NULL;
HWND hSearchBox = NULL;
HWND hBtnRefresh = NULL;
HWND hBtnTree = NULL;
HWND hBtnEndTask = NULL;
HWND hBtnPriority = NULL;
HWND hBtnAffinity = NULL;
HWND hBtnInspect = NULL;
HWND hBtnExportCSV = NULL;
HWND hBtnExportJSON = NULL;
HWND hBtnExportMD = NULL;
HWND hBtnSave = NULL;
HWND hBtnLoad = NULL;
HWND hBtnHelp = NULL;
HWND hStatusText = NULL;
HFONT g_hFont = NULL;
WNDPROC g_OldEditProc = NULL;
WNDPROC g_OldListProc = NULL;
static WNDPROC g_OldInspectEditProc = NULL;

static BOOL g_isTreeView = FALSE;
static BOOL g_isSnapshotMode = FALSE;
static char g_toastText[128] = "Ready. Press [F1] for Help | [T] Tree | [A] Affinity | [I] Inspect | [P] Priority";
static int g_toastTimer = 2; // ~5 seconds (2 x 2.5s timer ticks)

void ShowHelpDialog(HWND hwnd);

void ShowNativeToast(const char* txt) {
    if (!txt) return;
    int i = 0;
    while (txt[i] && i < (int)sizeof(g_toastText) - 1) {
        g_toastText[i] = txt[i];
        i++;
    }
    g_toastText[i] = 0;
    g_toastTimer = 2;
    if (hStatusText) {
        SetWindowTextA(hStatusText, g_toastText);
    }
}

void my_utoa(DWORD num, char* str) {
    int i = 0;
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
    while (num != 0) {
        DWORD rem = num % 10;
        str[i++] = (char)(rem + '0');
        num = num / 10;
    }
    str[i] = '\0';
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char t = str[start];
        str[start] = str[end];
        str[end] = t;
        start++;
        end--;
    }
}

int my_strlen(const char* s) {
    if (!s) return 0;
    int len = 0;
    while (s[len]) len++;
    return len;
}

void my_strcpy(char* dest, const char* src) {
    if (!dest || !src) return;
    while (*src) *dest++ = *src++;
    *dest = 0;
}

void my_strcat(char* dest, const char* src) {
    if (!dest || !src) return;
    while (*dest) dest++;
    while (*src) *dest++ = *src++;
    *dest = 0;
}

void my_strncat(char* dest, const char* src, int maxDest) {
    if (!dest || !src || maxDest <= 0) return;
    int d = 0;
    while (dest[d] && d < maxDest - 1) d++;
    while (*src && d < maxDest - 1) {
        dest[d++] = *src++;
    }
    dest[d] = '\0';
}

void my_itoa(int num, char* str) {
    if (num < 0) {
        *str++ = '-';
        if (num == -2147483648) {
            my_strcpy(str, "2147483648");
            return;
        }
        num = -num;
    }
    my_utoa((DWORD)num, str);
}

int my_stristr(const char* haystack, const char* needle) {
    if (!needle || !*needle) return 1;
    if (!haystack) return 0;
    for (int i = 0; haystack[i]; i++) {
        int j = 0;
        while (haystack[i + j] && needle[j]) {
            char c1 = haystack[i + j];
            char c2 = needle[j];
            if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
            if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
            if (c1 != c2) break;
            j++;
        }
        if (!needle[j]) return 1;
    }
    return 0;
}

void my_hex8(DWORD num, char* str) {
    const char* hex = "0123456789ABCDEF";
    str[0] = '0';
    str[1] = 'x';
    for (int i = 7; i >= 0; i--) {
        str[2 + (7 - i)] = hex[(num >> (i * 4)) & 0xF];
    }
    str[10] = '\0';
}

void my_escape_json(const char* src, char* dest, int maxLen) {
    if (!src || !dest || maxLen <= 1) return;
    int d = 0;
    for (int s = 0; src[s] && d < maxLen - 2; s++) {
        char c = src[s];
        if (c == '"' || c == '\\') {
            if (d < maxLen - 3) {
                dest[d++] = '\\';
                dest[d++] = c;
            }
        } else if (c == '\r') {
            if (d < maxLen - 3) { dest[d++] = '\\'; dest[d++] = 'r'; }
        } else if (c == '\n') {
            if (d < maxLen - 3) { dest[d++] = '\\'; dest[d++] = 'n'; }
        } else if (c == '\t') {
            if (d < maxLen - 3) { dest[d++] = '\\'; dest[d++] = 't'; }
        } else {
            dest[d++] = c;
        }
    }
    dest[d] = '\0';
}

void my_escape_csv(const char* src, char* dest, int maxLen) {
    if (!src || !dest || maxLen <= 1) return;
    int d = 0;
    for (int s = 0; src[s] && d < maxLen - 2; s++) {
        char c = src[s];
        if (c == '"') {
            if (d < maxLen - 3) {
                dest[d++] = '"';
                dest[d++] = '"';
            }
        } else {
            dest[d++] = c;
        }
    }
    dest[d] = '\0';
}

void LayoutControls(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    if (width < 320 || height < 200) return;

    MoveWindow(hSearchBox, 10, 10, width - 20, 25, TRUE);

    int listHeight = height - 126;
    if (listHeight < 60) listHeight = 60;
    MoveWindow(hListBox, 10, 42, width - 20, listHeight, TRUE);

    int row1Y = height - 80;
    int row2Y = height - 52;
    int curX = 10;

    // Row 1: Core Process Navigation & Actions
    MoveWindow(hBtnRefresh, curX, row1Y, 85, 24, TRUE); curX += 90;
    MoveWindow(hBtnTree, curX, row1Y, 70, 24, TRUE); curX += 75;
    MoveWindow(hBtnInspect, curX, row1Y, 75, 24, TRUE); curX += 80;
    MoveWindow(hBtnPriority, curX, row1Y, 75, 24, TRUE); curX += 80;
    MoveWindow(hBtnAffinity, curX, row1Y, 75, 24, TRUE); curX += 80;

    int endTaskX = width - 115;
    if (endTaskX < curX + 5) endTaskX = curX + 5;
    MoveWindow(hBtnEndTask, endTaskX, row1Y, 105, 24, TRUE);

    // Row 2: Diagnostic Exports, State Persistence & Reference
    curX = 10;
    MoveWindow(hBtnExportCSV, curX, row2Y, 60, 24, TRUE); curX += 65;
    MoveWindow(hBtnExportJSON, curX, row2Y, 60, 24, TRUE); curX += 65;
    MoveWindow(hBtnExportMD, curX, row2Y, 70, 24, TRUE); curX += 75;
    MoveWindow(hBtnSave, curX, row2Y, 75, 24, TRUE); curX += 80;
    MoveWindow(hBtnLoad, curX, row2Y, 75, 24, TRUE); curX += 80;
    MoveWindow(hBtnHelp, curX, row2Y, 70, 24, TRUE);

    MoveWindow(hStatusText, 10, height - 24, width - 20, 20, TRUE);
}

typedef struct {
    DWORD pid;
    DWORD ppid;
    DWORD threads;
    LONG pri;
    char szExeFile[MAX_PATH];
    BOOL displayed;
} PROC_ENTRY;

#define KTASK_SAVE_MAGIC 0x4B54534B  // "KTSK"
#define KTASK_SAVE_VERSION 1
#define MAX_SAVED_PROCS 128

typedef struct {
    DWORD magic;
    DWORD version;
    DWORD timestamp;
    BOOL isTreeView;
    char searchFilter[128];
    DWORD selectedPid;
    DWORD procCount;
    PROC_ENTRY procs[MAX_SAVED_PROCS];
    DWORD checksum;
} KTaskSaveData;

static KTaskSaveData g_saveBuffer;

DWORD CalculateKTaskChecksum(const KTaskSaveData* data) {
    const unsigned char* p = (const unsigned char*)data;
    size_t sz = sizeof(KTaskSaveData) - sizeof(DWORD);
    DWORD sum = 0x5A5AA5A5;
    for (size_t i = 0; i < sz; i++) {
        sum = ((sum << 5) + sum) + p[i];
    }
    return sum;
}

BOOL HasSeenTutorial(void) {
    HANDLE hFile = CreateFileA("ktask_tutorial.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        return TRUE;
    }
    return FALSE;
}

void MarkTutorialSeen(void) {
    HANDLE hFile = CreateFileA("ktask_tutorial.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        const char* mark = "TUTORIAL_SEEN_V1\r\n";
        DWORD written = 0;
        WriteFile(hFile, mark, my_strlen(mark), &written, NULL);
        CloseHandle(hFile);
    }
}

BOOL HasSavedState(const char* filename) {
    HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return FALSE;
    DWORD size = GetFileSize(hFile, NULL);
    CloseHandle(hFile);
    return (size == sizeof(KTaskSaveData));
}

BOOL SaveStateToFile(const char* filename) {
    memset(&g_saveBuffer, 0, sizeof(g_saveBuffer));
    g_saveBuffer.magic = KTASK_SAVE_MAGIC;
    g_saveBuffer.version = KTASK_SAVE_VERSION;
    g_saveBuffer.timestamp = GetTickCount();
    g_saveBuffer.isTreeView = g_isTreeView;
    if (hSearchBox) {
        GetWindowTextA(hSearchBox, g_saveBuffer.searchFilter, sizeof(g_saveBuffer.searchFilter) - 1);
    }
    int currentSel = SendMessageA(hListBox, LB_GETCURSEL, 0, 0);
    if (currentSel != LB_ERR) {
        g_saveBuffer.selectedPid = (DWORD)SendMessageA(hListBox, LB_GETITEMDATA, currentSel, 0);
    }

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        DWORD count = 0;
        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (count < MAX_SAVED_PROCS) {
                    g_saveBuffer.procs[count].pid = pe32.th32ProcessID;
                    g_saveBuffer.procs[count].ppid = pe32.th32ParentProcessID;
                    g_saveBuffer.procs[count].threads = pe32.cntThreads;
                    g_saveBuffer.procs[count].pri = pe32.pcPriClassBase;
                    my_strcpy(g_saveBuffer.procs[count].szExeFile, pe32.szExeFile);
                    count++;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
        g_saveBuffer.procCount = count;
    }

    g_saveBuffer.checksum = CalculateKTaskChecksum(&g_saveBuffer);

    HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return FALSE;

    DWORD written = 0;
    BOOL ok = WriteFile(hFile, &g_saveBuffer, sizeof(g_saveBuffer), &written, NULL);
    CloseHandle(hFile);

    if (ok && written == sizeof(g_saveBuffer)) {
        MarkTutorialSeen();
        return TRUE;
    }
    return FALSE;
}

BOOL LoadStateFromFile(const char* filename) {
    HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return FALSE;

    DWORD size = GetFileSize(hFile, NULL);
    if (size != sizeof(KTaskSaveData)) {
        CloseHandle(hFile);
        return FALSE;
    }

    DWORD read = 0;
    BOOL ok = ReadFile(hFile, &g_saveBuffer, sizeof(g_saveBuffer), &read, NULL);
    CloseHandle(hFile);

    if (!ok || read != sizeof(g_saveBuffer)) return FALSE;
    if (g_saveBuffer.magic != KTASK_SAVE_MAGIC || g_saveBuffer.version != KTASK_SAVE_VERSION) return FALSE;
    if (g_saveBuffer.checksum != CalculateKTaskChecksum(&g_saveBuffer)) return FALSE;

    g_isTreeView = g_saveBuffer.isTreeView;
    if (hSearchBox) {
        SetWindowTextA(hSearchBox, g_saveBuffer.searchFilter);
    }

    SendMessageA(hListBox, WM_SETREDRAW, FALSE, 0);
    SendMessageA(hListBox, LB_RESETCONTENT, 0, 0);

    for (DWORD i = 0; i < g_saveBuffer.procCount; i++) {
        char buf[512] = {0};
        char pidStr[16] = {0};
        char thrStr[16] = {0};
        char priStr[16] = {0};

        my_utoa(g_saveBuffer.procs[i].pid, pidStr);
        my_utoa(g_saveBuffer.procs[i].threads, thrStr);
        my_itoa(g_saveBuffer.procs[i].pri, priStr);

        my_strcpy(buf, "[PID: ");
        my_strcat(buf, pidStr);
        my_strcat(buf, "] ");
        my_strcat(buf, g_saveBuffer.procs[i].szExeFile);
        my_strcat(buf, " (Threads: ");
        my_strcat(buf, thrStr);
        my_strcat(buf, ", BasePri: ");
        my_strcat(buf, priStr);
        my_strcat(buf, ")");

        int index = SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)buf);
        SendMessageA(hListBox, LB_SETITEMDATA, index, (LPARAM)g_saveBuffer.procs[i].pid);
    }

    if (g_saveBuffer.selectedPid != 0 && g_saveBuffer.procCount > 0) {
        int count = SendMessageA(hListBox, LB_GETCOUNT, 0, 0);
        for (int i = 0; i < count; i++) {
            DWORD pid = (DWORD)SendMessageA(hListBox, LB_GETITEMDATA, i, 0);
            if (pid == g_saveBuffer.selectedPid) {
                SendMessageA(hListBox, LB_SETCURSEL, i, 0);
                break;
            }
        }
    } else if (g_saveBuffer.procCount > 0) {
        SendMessageA(hListBox, LB_SETCURSEL, 0, 0);
    }

    SendMessageA(hListBox, WM_SETREDRAW, TRUE, 0);
    RedrawWindow(hListBox, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);

    if (hBtnTree) {
        SetWindowTextA(hBtnTree, g_isTreeView ? "List [T]" : "Tree [T]");
    }

    g_isSnapshotMode = TRUE;
    MarkTutorialSeen();

    char toast[128] = {0};
    char numStr[16] = {0};
    my_utoa(g_saveBuffer.procCount, numStr);
    my_strcpy(toast, "★ Restored snapshot (");
    my_strcat(toast, numStr);
    my_strcat(toast, " tasks) from ktask.dat [F9] (Auto-refresh paused. Press R to resume)");
    ShowNativeToast(toast);

    return TRUE;
}

void RefreshList() {
    DWORD selectedPid = 0;
    int currentSel = SendMessageA(hListBox, LB_GETCURSEL, 0, 0);
    if (currentSel != LB_ERR) {
        selectedPid = (DWORD)SendMessageA(hListBox, LB_GETITEMDATA, currentSel, 0);
    }

    SendMessageA(hListBox, WM_SETREDRAW, FALSE, 0);
    SendMessageA(hListBox, LB_RESETCONTENT, 0, 0);

    char filter[256] = {0};
    if (hSearchBox) GetWindowTextA(hSearchBox, filter, sizeof(filter) - 1);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)"[Error: Unable to snapshot processes]");
        SendMessageA(hListBox, WM_SETREDRAW, TRUE, 0);
        InvalidateRect(hListBox, NULL, TRUE);
        if (hStatusText) SetWindowTextA(hStatusText, "Error: Unable to snapshot processes");
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    int totalTasks = 0;
    int shownTasks = 0;
    DWORD totalThreads = 0;

    // Buffer for tree/hierarchy layout
    const int MAX_PROCS = 1024;
    PROC_ENTRY* procList = (PROC_ENTRY*)VirtualAlloc(NULL, sizeof(PROC_ENTRY) * MAX_PROCS, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (Process32First(hSnapshot, &pe32)) {
        do {
            totalTasks++;
            totalThreads += pe32.cntThreads;

            char pidStr[16] = {0};
            my_utoa(pe32.th32ProcessID, pidStr);

            // Advanced filter matching: supports pid:, pri:, thr>, or substring
            BOOL matches = TRUE;
            if (filter[0]) {
                if (filter[0] == 'p' && filter[1] == 'i' && filter[2] == 'd' && filter[3] == ':') {
                    matches = my_stristr(pidStr, filter + 4);
                } else if (filter[0] == 'p' && filter[1] == 'r' && filter[2] == 'i' && filter[3] == ':') {
                    char priBuf[16] = {0};
                    my_itoa(pe32.pcPriClassBase, priBuf);
                    matches = my_stristr(priBuf, filter + 4);
                } else {
                    matches = (my_stristr(pe32.szExeFile, filter) || my_stristr(pidStr, filter));
                }
            }
            if (!matches) continue;

            if (procList && shownTasks < MAX_PROCS) {
                procList[shownTasks].pid = pe32.th32ProcessID;
                procList[shownTasks].ppid = pe32.th32ParentProcessID;
                procList[shownTasks].threads = pe32.cntThreads;
                procList[shownTasks].pri = pe32.pcPriClassBase;
                my_strcpy(procList[shownTasks].szExeFile, pe32.szExeFile);
                procList[shownTasks].displayed = FALSE;
            }
            shownTasks++;
        } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);

    if (g_isTreeView && procList && shownTasks > 0) {
        // Render Tree hierarchy
        int count = shownTasks < MAX_PROCS ? shownTasks : MAX_PROCS;

        // Phase 1: Render root nodes (where ppid is 0 or ppid not in current list)
        for (int i = 0; i < count; i++) {
            BOOL hasParentInList = FALSE;
            if (procList[i].ppid != 0) {
                for (int p = 0; p < count; p++) {
                    if (procList[p].pid == procList[i].ppid && p != i) {
                        hasParentInList = TRUE;
                        break;
                    }
                }
            }

            if (!hasParentInList && !procList[i].displayed) {
                procList[i].displayed = TRUE;

                char buf[512] = {0};
                char pidStr[16] = {0};
                char thrStr[16] = {0};
                char priStr[16] = {0};
                my_utoa(procList[i].pid, pidStr);
                my_utoa(procList[i].threads, thrStr);
                my_itoa(procList[i].pri, priStr);

                my_strcpy(buf, "[PID: ");
                my_strcat(buf, pidStr);
                my_strcat(buf, "] ");
                my_strcat(buf, procList[i].szExeFile);
                my_strcat(buf, " (Threads: ");
                my_strcat(buf, thrStr);
                my_strcat(buf, ", Pri: ");
                my_strcat(buf, priStr);
                my_strcat(buf, ")");

                int index = SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)buf);
                SendMessageA(hListBox, LB_SETITEMDATA, index, (LPARAM)procList[i].pid);

                // Phase 2: Render direct children of this root
                for (int c = 0; c < count; c++) {
                    if (procList[c].ppid == procList[i].pid && !procList[c].displayed) {
                        procList[c].displayed = TRUE;
                        char cbuf[512] = {0};
                        char cpidStr[16] = {0};
                        char cthrStr[16] = {0};
                        my_utoa(procList[c].pid, cpidStr);
                        my_utoa(procList[c].threads, cthrStr);

                        my_strcpy(cbuf, "  |-- [PID: ");
                        my_strcat(cbuf, cpidStr);
                        my_strcat(cbuf, "] ");
                        my_strcat(cbuf, procList[c].szExeFile);
                        my_strcat(cbuf, " (Thr: ");
                        my_strcat(cbuf, cthrStr);
                        my_strcat(cbuf, ")");

                        int cindex = SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)cbuf);
                        SendMessageA(hListBox, LB_SETITEMDATA, cindex, (LPARAM)procList[c].pid);
                    }
                }
            }
        }

        // Phase 3: Display any remaining unlinked processes
        for (int i = 0; i < count; i++) {
            if (!procList[i].displayed) {
                procList[i].displayed = TRUE;
                char buf[512] = {0};
                char pidStr[16] = {0};
                char thrStr[16] = {0};
                my_utoa(procList[i].pid, pidStr);
                my_utoa(procList[i].threads, thrStr);

                my_strcpy(buf, "[PID: ");
                my_strcat(buf, pidStr);
                my_strcat(buf, "] ");
                my_strcat(buf, procList[i].szExeFile);
                my_strcat(buf, " (Threads: ");
                my_strcat(buf, thrStr);
                my_strcat(buf, ")");

                int index = SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)buf);
                SendMessageA(hListBox, LB_SETITEMDATA, index, (LPARAM)procList[i].pid);
            }
        }
    } else if (procList && shownTasks > 0) {
        // Render Flat list
        int count = shownTasks < MAX_PROCS ? shownTasks : MAX_PROCS;
        for (int i = 0; i < count; i++) {
            char buf[512] = {0};
            char pidStr[16] = {0};
            char thrStr[16] = {0};
            char priStr[16] = {0};

            my_utoa(procList[i].pid, pidStr);
            my_utoa(procList[i].threads, thrStr);
            my_itoa(procList[i].pri, priStr);

            my_strcpy(buf, "[PID: ");
            my_strcat(buf, pidStr);
            my_strcat(buf, "] ");
            my_strcat(buf, procList[i].szExeFile);
            my_strcat(buf, " (Threads: ");
            my_strcat(buf, thrStr);
            my_strcat(buf, ", BasePri: ");
            my_strcat(buf, priStr);
            my_strcat(buf, ")");

            int index = SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)buf);
            SendMessageA(hListBox, LB_SETITEMDATA, index, (LPARAM)procList[i].pid);
        }
    }

    if (procList) {
        VirtualFree(procList, 0, MEM_RELEASE);
    }

    if (selectedPid != 0 && shownTasks > 0) {
        int count = SendMessageA(hListBox, LB_GETCOUNT, 0, 0);
        for (int i = 0; i < count; i++) {
            DWORD pid = (DWORD)SendMessageA(hListBox, LB_GETITEMDATA, i, 0);
            if (pid == selectedPid) {
                SendMessageA(hListBox, LB_SETCURSEL, i, 0);
                break;
            }
        }
    }

    SendMessageA(hListBox, WM_SETREDRAW, TRUE, 0);
    RedrawWindow(hListBox, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);

    if (hBtnTree) {
        SetWindowTextA(hBtnTree, g_isTreeView ? "List [T]" : "Tree [T]");
    }

    if (hStatusText) {
        if (g_toastTimer > 0) {
            SetWindowTextA(hStatusText, g_toastText);
        } else {
            MEMORYSTATUSEX memStatus;
            memStatus.dwLength = sizeof(memStatus);
            GlobalMemoryStatusEx(&memStatus);

            char statusBuf[256] = {0};
            char shownStr[16] = {0};
            char totalStr[16] = {0};
            char threadsStr[16] = {0};
            char memLoadStr[16] = {0};

            my_utoa((DWORD)shownTasks, shownStr);
            my_utoa((DWORD)totalTasks, totalStr);
            my_utoa(totalThreads, threadsStr);
            my_utoa(memStatus.dwMemoryLoad, memLoadStr);

            my_strcpy(statusBuf, g_isTreeView ? "Tree | Procs: " : "Flat | Procs: ");
            my_strcat(statusBuf, shownStr);
            my_strcat(statusBuf, "/");
            my_strcat(statusBuf, totalStr);
            my_strcat(statusBuf, " | Threads: ");
            my_strcat(statusBuf, threadsStr);
            my_strcat(statusBuf, " | RAM Load: ");
            my_strcat(statusBuf, memLoadStr);
            my_strcat(statusBuf, "%");
            SetWindowTextA(hStatusText, statusBuf);
        }
    }
}

LRESULT CALLBACK InspectEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == VK_ESCAPE) {
        DestroyWindow(GetParent(hwnd));
        return 0;
    }
    return CallWindowProcA(g_OldInspectEditProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK InspectWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HWND hEdit = CreateWindowExA(0, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                0, 0, 100, 100, hwnd, (HMENU)101, NULL, NULL);

            NONCLIENTMETRICSA ncm;
            ncm.cbSize = sizeof(NONCLIENTMETRICSA);
            SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSA), &ncm, 0);
            HFONT hFont = CreateFontA(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            if (hFont) {
                SendMessageA(hEdit, WM_SETFONT, (WPARAM)hFont, FALSE);
                SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)hFont);
            }

            g_OldInspectEditProc = (WNDPROC)SetWindowLongPtrA(hEdit, GWLP_WNDPROC, (LONG_PTR)InspectEditProc);
            SendMessageA(hEdit, EM_SETLIMITTEXT, 0, 0);

            CREATESTRUCTA* cs = (CREATESTRUCTA*)lParam;
            if (cs && cs->lpCreateParams) {
                SetWindowTextA(hEdit, (const char*)cs->lpCreateParams);
            }
            break;
        }
        case WM_SIZE: {
            HWND hEdit = GetDlgItem(hwnd, 101);
            if (hEdit) {
                MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
            }
            break;
        }
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY: {
            HFONT hFont = (HFONT)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
            if (hFont) DeleteObject(hFont);
            break;
        }
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void PerformInspectProcess(HWND parentHwnd) {
    int sel = SendMessageA(hListBox, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR) {
        MessageBoxA(parentHwnd, "Please select a process from the list first.", "KTask Notice", MB_OK | MB_ICONINFORMATION);
        return;
    }

    DWORD pid = (DWORD)SendMessageA(hListBox, LB_GETITEMDATA, sel, 0);
    if (pid == 0) return;

    char itemText[256] = {0};
    SendMessageA(hListBox, LB_GETTEXT, sel, (LPARAM)itemText);

    char* report = (char*)VirtualAlloc(NULL, 65536, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!report) {
        MessageBoxA(parentHwnd, "Out of memory.", "KTask Error", MB_OK | MB_ICONERROR);
        return;
    }

    my_strcpy(report, "===================================================================\r\n");
    my_strncat(report, "           KTASK DEEP PROCESS & MEMORY INSPECTOR REPORT           \r\n", 65535);
    my_strncat(report, "===================================================================\r\n", 65535);
    my_strncat(report, "Process Info: ", 65535);
    my_strncat(report, itemText, 65535);
    my_strncat(report, "\r\n\r\n", 65535);

    // 1. Thread List
    my_strncat(report, "-------------------------------------------------------------------\r\n", 65535);
    my_strncat(report, " 1. ACTIVE THREADS SNAPSHOT (Toolhelp32)\r\n", 65535);
    my_strncat(report, "-------------------------------------------------------------------\r\n", 65535);

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    int threadIdx = 0;
    if (hSnap != INVALID_HANDLE_VALUE) {
        THREADENTRY32 te32;
        te32.dwSize = sizeof(THREADENTRY32);
        if (Thread32First(hSnap, &te32)) {
            do {
                if (te32.th32OwnerProcessID == pid) {
                    threadIdx++;
                    char lineBuf[128] = {0};
                    char idxStr[16] = {0};
                    char tidStr[16] = {0};
                    char basePriStr[16] = {0};
                    char deltaPriStr[16] = {0};

                    my_utoa((DWORD)threadIdx, idxStr);
                    my_utoa(te32.th32ThreadID, tidStr);
                    my_itoa(te32.tpBasePri, basePriStr);
                    my_itoa(te32.tpDeltaPri, deltaPriStr);

                    my_strcpy(lineBuf, "  [#");
                    my_strcat(lineBuf, idxStr);
                    my_strcat(lineBuf, "] Thread ID: ");
                    my_strcat(lineBuf, tidStr);
                    my_strcat(lineBuf, "\t| Base Pri: ");
                    my_strcat(lineBuf, basePriStr);
                    my_strcat(lineBuf, "\t| Delta: ");
                    my_strcat(lineBuf, deltaPriStr);
                    my_strcat(lineBuf, "\r\n");
                    my_strncat(report, lineBuf, 65535);
                }
            } while (Thread32Next(hSnap, &te32));
        }
        CloseHandle(hSnap);
    }
    if (threadIdx == 0) {
        my_strncat(report, "  (No accessible thread details found or process restricted)\r\n", 65535);
    }

    // 2. Loaded Modules
    my_strncat(report, "\r\n-------------------------------------------------------------------\r\n", 65535);
    my_strncat(report, " 2. LOADED MODULES & LIBRARIES (Toolhelp32)\r\n", 65535);
    my_strncat(report, "-------------------------------------------------------------------\r\n", 65535);

    HANDLE hModSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    int modCount = 0;
    if (hModSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 me32;
        me32.dwSize = sizeof(MODULEENTRY32);
        if (Module32First(hModSnap, &me32)) {
            do {
                modCount++;
                if (modCount > 40) {
                    my_strncat(report, "  ... [Truncated remaining loaded modules] ...\r\n", 65535);
                    break;
                }
                char lineBuf[512] = {0};
                char baseStr[16] = {0};
                char sizeStr[16] = {0};

                my_hex8((DWORD)(ULONG_PTR)me32.modBaseAddr, baseStr);
                my_utoa(me32.modBaseSize / 1024, sizeStr);

                my_strcpy(lineBuf, "  * Module: ");
                my_strcat(lineBuf, me32.szModule);
                my_strcat(lineBuf, " \t| Base: ");
                my_strcat(lineBuf, baseStr);
                my_strcat(lineBuf, " | Size: ");
                my_strcat(lineBuf, sizeStr);
                my_strcat(lineBuf, " KB\r\n    Path: ");
                my_strcat(lineBuf, me32.szExePath);
                my_strcat(lineBuf, "\r\n");
                my_strncat(report, lineBuf, 65535);
            } while (Module32Next(hModSnap, &me32));
        }
        CloseHandle(hModSnap);
    }
    if (modCount == 0) {
        my_strncat(report, "  (Module query denied or unavailable for system process)\r\n", 65535);
    }

    // 3. Virtual Memory Map
    my_strncat(report, "\r\n-------------------------------------------------------------------\r\n", 65535);
    my_strncat(report, " 3. VIRTUAL MEMORY REGION MAP (VirtualQueryEx)\r\n", 65535);
    my_strncat(report, "-------------------------------------------------------------------\r\n", 65535);

    HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    int regCount = 0;
    if (hProc) {
        MEMORY_BASIC_INFORMATION mbi;
        unsigned char* pAddr = NULL;
        while (VirtualQueryEx(hProc, pAddr, &mbi, sizeof(mbi)) == sizeof(mbi)) {
            regCount++;
            if (regCount > 40) {
                my_strncat(report, "  ... [Truncated remaining memory regions] ...\r\n", 65535);
                break;
            }

            char lineBuf[256] = {0};
            char baseStr[16] = {0};
            char sizeStr[16] = {0};
            const char* stateStr = "UNKNOWN";
            const char* protStr = "NONE";

            my_hex8((DWORD)(ULONG_PTR)mbi.BaseAddress, baseStr);
            my_utoa((DWORD)(mbi.RegionSize / 1024), sizeStr);

            if (mbi.State == MEM_COMMIT) stateStr = "COMMIT";
            else if (mbi.State == MEM_RESERVE) stateStr = "RESERVE";
            else if (mbi.State == MEM_FREE) stateStr = "FREE";

            if (mbi.Protect & PAGE_READWRITE) protStr = "READWRITE";
            else if (mbi.Protect & PAGE_READONLY) protStr = "READONLY";
            else if (mbi.Protect & PAGE_EXECUTE_READ) protStr = "EXEC_READ";
            else if (mbi.Protect & PAGE_EXECUTE_READWRITE) protStr = "EXEC_READWRITE";
            else if (mbi.Protect & PAGE_NOACCESS) protStr = "NOACCESS";
            else if (mbi.Protect & PAGE_EXECUTE) protStr = "EXECUTE";

            my_strcpy(lineBuf, "  Addr: ");
            my_strcat(lineBuf, baseStr);
            my_strcat(lineBuf, " | Size: ");
            my_strcat(lineBuf, sizeStr);
            my_strcat(lineBuf, " KB\t| State: ");
            my_strcat(lineBuf, stateStr);
            my_strcat(lineBuf, "\t| Protect: ");
            my_strcat(lineBuf, protStr);
            my_strcat(lineBuf, "\r\n");

            my_strncat(report, lineBuf, 65535);

            unsigned char* nextAddr = (unsigned char*)mbi.BaseAddress + mbi.RegionSize;
            if (nextAddr <= pAddr) break;
            pAddr = nextAddr;
        }

        // 4. CPU Affinity & Execution Times
        my_strncat(report, "\r\n-------------------------------------------------------------------\r\n", 65535);
        my_strncat(report, " 4. CPU AFFINITY & PROCESS EXECUTION TIMES\r\n", 65535);
        my_strncat(report, "-------------------------------------------------------------------\r\n", 65535);

        DWORD_PTR procAff = 0, sysAff = 0;
        if (GetProcessAffinityMask(hProc, &procAff, &sysAff)) {
            char affBuf[128] = {0};
            char pAffStr[16] = {0};
            char sAffStr[16] = {0};
            my_hex8((DWORD)procAff, pAffStr);
            my_hex8((DWORD)sysAff, sAffStr);

            my_strcpy(affBuf, "  Process CPU Mask: ");
            my_strcat(affBuf, pAffStr);
            my_strcat(affBuf, " | System Mask: ");
            my_strcat(affBuf, sAffStr);
            my_strcat(affBuf, "\r\n");
            my_strncat(report, affBuf, 65535);
        }

        FILETIME ftCreate, ftExit, ftKernel, ftUser;
        if (GetProcessTimes(hProc, &ftCreate, &ftExit, &ftKernel, &ftUser)) {
            ULARGE_INTEGER kTime, uTime;
            kTime.LowPart = ftKernel.dwLowDateTime;
            kTime.HighPart = ftKernel.dwHighDateTime;
            uTime.LowPart = ftUser.dwLowDateTime;
            uTime.HighPart = ftUser.dwHighDateTime;

            DWORD kSec = (DWORD)(kTime.QuadPart / 10000000);
            DWORD uSec = (DWORD)(uTime.QuadPart / 10000000);

            char timeBuf[128] = {0};
            char kStr[16] = {0};
            char uStr[16] = {0};
            my_utoa(kSec, kStr);
            my_utoa(uSec, uStr);

            my_strcpy(timeBuf, "  Kernel Time: ");
            my_strcat(timeBuf, kStr);
            my_strcat(timeBuf, "s | User Time: ");
            my_strcat(timeBuf, uStr);
            my_strcat(timeBuf, "s\r\n");
            my_strncat(report, timeBuf, 65535);
        }

        CloseHandle(hProc);
    }
    if (regCount == 0) {
        my_strncat(report, "  (Virtual Memory query restricted for this system process)\r\n", 65535);
    }

    my_strncat(report, "===================================================================\r\n", 65535);

    static BOOL regDone = FALSE;
    HINSTANCE hInst = GetModuleHandleA(NULL);
    if (!regDone) {
        WNDCLASSA wc = {0};
        wc.lpfnWndProc = InspectWndProc;
        wc.hInstance = hInst;
        wc.lpszClassName = "KTaskInspectClass";
        wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClassA(&wc);
        regDone = TRUE;
    }

    char titleBuf[128] = {0};
    char pidStr[16] = {0};
    my_utoa(pid, pidStr);
    my_strcpy(titleBuf, "KTask Inspector - PID: ");
    my_strcat(titleBuf, pidStr);

    HWND hInspectWnd = CreateWindowExA(WS_EX_TOPMOST, "KTaskInspectClass", titleBuf,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 680, 540, parentHwnd, NULL, hInst, (LPVOID)report);

    ShowWindow(hInspectWnd, SW_SHOW);
    UpdateWindow(hInspectWnd);

    VirtualFree(report, 0, MEM_RELEASE);
}

void PerformEndTask(HWND hwnd) {
    int sel = SendMessageA(hListBox, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR) {
        MessageBoxA(hwnd, "Please select a process from the list first.", "KTask Notice", MB_OK | MB_ICONINFORMATION);
        return;
    }

    DWORD pid = (DWORD)SendMessageA(hListBox, LB_GETITEMDATA, sel, 0);
    if (pid == 0) return;

    if (pid == 0 || pid == 4) {
        MessageBoxA(hwnd, "System and Idle processes cannot be terminated.", "KTask Warning", MB_OK | MB_ICONWARNING);
        return;
    }

    char confirmBuf[256] = {0};
    char pidStr[16] = {0};
    my_utoa(pid, pidStr);
    my_strcpy(confirmBuf, "Are you sure you want to terminate process with PID ");
    my_strcat(confirmBuf, pidStr);
    my_strcat(confirmBuf, "? Unsaved data may be lost.");

    if (MessageBoxA(hwnd, confirmBuf, "Confirm End Task", MB_YESNO | MB_ICONWARNING) != IDYES) {
        return;
    }

    HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProc) {
        if (TerminateProcess(hProc, 1)) {
            CloseHandle(hProc);
            RefreshList();
            ShowNativeToast("Process successfully terminated.");
        } else {
            DWORD err = GetLastError();
            CloseHandle(hProc);
            char errBuf[128] = {0};
            char errStr[16] = {0};
            my_utoa(err, errStr);
            my_strcpy(errBuf, "Failed to terminate process (Error Code: ");
            my_strcat(errBuf, errStr);
            my_strcat(errBuf, ").");
            MessageBoxA(hwnd, errBuf, "KTask Error", MB_OK | MB_ICONERROR);
        }
    } else {
        DWORD err = GetLastError();
        char errBuf[128] = {0};
        char errStr[16] = {0};
        my_utoa(err, errStr);
        my_strcpy(errBuf, "Access Denied or Process Exited (Error Code: ");
        my_strcat(errBuf, errStr);
        my_strcat(errBuf, ").");
        MessageBoxA(hwnd, errBuf, "KTask Error", MB_OK | MB_ICONERROR);
    }
}

void PerformSetPriority(HWND hwnd) {
    int sel = SendMessageA(hListBox, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR) {
        MessageBoxA(hwnd, "Please select a process from the list first.", "KTask Notice", MB_OK | MB_ICONINFORMATION);
        return;
    }

    DWORD pid = (DWORD)SendMessageA(hListBox, LB_GETITEMDATA, sel, 0);
    if (pid == 0 || pid == 4) {
        MessageBoxA(hwnd, "Cannot change priority for System or Idle processes.", "KTask Warning", MB_OK | MB_ICONWARNING);
        return;
    }

    HANDLE hProc = OpenProcess(PROCESS_SET_INFORMATION | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!hProc) {
        MessageBoxA(hwnd, "Access Denied: Unable to modify process priority.", "KTask Error", MB_OK | MB_ICONERROR);
        return;
    }

    DWORD curPri = GetPriorityClass(hProc);
    DWORD newPri = NORMAL_PRIORITY_CLASS;
    const char* priName = "Normal";

    if (curPri == NORMAL_PRIORITY_CLASS) {
        newPri = HIGH_PRIORITY_CLASS;
        priName = "High";
    } else if (curPri == HIGH_PRIORITY_CLASS) {
        newPri = BELOW_NORMAL_PRIORITY_CLASS;
        priName = "Below Normal";
    } else {
        newPri = NORMAL_PRIORITY_CLASS;
        priName = "Normal";
    }

    if (SetPriorityClass(hProc, newPri)) {
        char msgBuf[128] = {0};
        my_strcpy(msgBuf, "PID priority updated to ");
        my_strcat(msgBuf, priName);
        my_strcat(msgBuf, ".");
        ShowNativeToast(msgBuf);
        RefreshList();
    } else {
        ShowNativeToast("Failed to update priority class.");
    }
    CloseHandle(hProc);
}

void PerformSetAffinity(HWND hwnd) {
    int sel = SendMessageA(hListBox, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR) {
        MessageBoxA(hwnd, "Please select a process from the list first.", "KTask Notice", MB_OK | MB_ICONINFORMATION);
        return;
    }

    DWORD pid = (DWORD)SendMessageA(hListBox, LB_GETITEMDATA, sel, 0);
    if (pid == 0 || pid == 4) {
        MessageBoxA(hwnd, "Cannot modify CPU affinity for System or Idle processes.", "KTask Warning", MB_OK | MB_ICONWARNING);
        return;
    }

    HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION, FALSE, pid);
    if (!hProc) {
        MessageBoxA(hwnd, "Access Denied: Unable to modify process CPU affinity.", "KTask Error", MB_OK | MB_ICONERROR);
        return;
    }

    DWORD_PTR procMask = 0;
    DWORD_PTR sysMask = 0;
    if (GetProcessAffinityMask(hProc, &procMask, &sysMask)) {
        DWORD_PTR nextMask = sysMask;
        const char* desc = "All Available Cores";

        if (procMask == sysMask) {
            nextMask = 1; // Core 0 only
            desc = "Core 0 Only (0x1)";
        } else if (procMask == 1 && (sysMask >= 3)) {
            nextMask = 3; // Cores 0 & 1
            desc = "Cores 0 & 1 (0x3)";
        } else if (procMask == 3 && (sysMask >= 15)) {
            nextMask = 15; // Cores 0-3
            desc = "Cores 0-3 (0xF)";
        } else {
            nextMask = sysMask;
            desc = "All Cores (Reset)";
        }

        if (SetProcessAffinityMask(hProc, nextMask)) {
            char msg[128] = {0};
            my_strcpy(msg, "Affinity for PID updated: ");
            my_strcat(msg, desc);
            ShowNativeToast(msg);
        } else {
            ShowNativeToast("Failed to set process affinity.");
        }
    } else {
        ShowNativeToast("Failed to query process affinity.");
    }
    CloseHandle(hProc);
}

void PerformExportCSV(HWND hwnd) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        ShowNativeToast("Failed to create process snapshot.");
        return;
    }

    HANDLE hFile = CreateFileA("ktask_export.csv", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        CloseHandle(hSnapshot);
        ShowNativeToast("Failed to create ktask_export.csv file.");
        return;
    }

    const char* header = "PID,PPID,Process Name,Threads,BasePriority\r\n";
    DWORD written = 0;
    WriteFile(hFile, header, my_strlen(header), &written, NULL);

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &pe32)) {
        do {
            char lineBuf[512] = {0};
            char pidStr[16] = {0};
            char ppidStr[16] = {0};
            char thrStr[16] = {0};
            char priStr[16] = {0};
            char escapedExe[256] = {0};

            my_utoa(pe32.th32ProcessID, pidStr);
            my_utoa(pe32.th32ParentProcessID, ppidStr);
            my_utoa(pe32.cntThreads, thrStr);
            my_itoa(pe32.pcPriClassBase, priStr);
            my_escape_csv(pe32.szExeFile, escapedExe, sizeof(escapedExe));

            my_strcpy(lineBuf, pidStr);
            my_strcat(lineBuf, ",");
            my_strcat(lineBuf, ppidStr);
            my_strcat(lineBuf, ",\"");
            my_strcat(lineBuf, escapedExe);
            my_strcat(lineBuf, "\",");
            my_strcat(lineBuf, thrStr);
            my_strcat(lineBuf, ",");
            my_strcat(lineBuf, priStr);
            my_strcat(lineBuf, "\r\n");

            WriteFile(hFile, lineBuf, my_strlen(lineBuf), &written, NULL);
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hFile);
    CloseHandle(hSnapshot);

    ShowNativeToast("Snapshot exported to 'ktask_export.csv' successfully!");
}

void PerformExportJSON(HWND hwnd) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        ShowNativeToast("Failed to create process snapshot.");
        return;
    }

    HANDLE hFile = CreateFileA("ktask_export.json", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        CloseHandle(hSnapshot);
        ShowNativeToast("Failed to create ktask_export.json file.");
        return;
    }

    const char* startJson = "[\r\n";
    DWORD written = 0;
    WriteFile(hFile, startJson, my_strlen(startJson), &written, NULL);

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    BOOL isFirst = TRUE;

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (!isFirst) {
                const char* comma = ",\r\n";
                WriteFile(hFile, comma, my_strlen(comma), &written, NULL);
            }
            isFirst = FALSE;

            char lineBuf[512] = {0};
            char pidStr[16] = {0};
            char ppidStr[16] = {0};
            char thrStr[16] = {0};
            char priStr[16] = {0};
            char escapedExe[256] = {0};

            my_utoa(pe32.th32ProcessID, pidStr);
            my_utoa(pe32.th32ParentProcessID, ppidStr);
            my_utoa(pe32.cntThreads, thrStr);
            my_itoa(pe32.pcPriClassBase, priStr);
            my_escape_json(pe32.szExeFile, escapedExe, sizeof(escapedExe));

            my_strcpy(lineBuf, "  {\"pid\": ");
            my_strcat(lineBuf, pidStr);
            my_strcat(lineBuf, ", \"ppid\": ");
            my_strcat(lineBuf, ppidStr);
            my_strcat(lineBuf, ", \"name\": \"");
            my_strcat(lineBuf, escapedExe);
            my_strcat(lineBuf, "\", \"threads\": ");
            my_strcat(lineBuf, thrStr);
            my_strcat(lineBuf, ", \"basePriority\": ");
            my_strcat(lineBuf, priStr);
            my_strcat(lineBuf, "}");

            WriteFile(hFile, lineBuf, my_strlen(lineBuf), &written, NULL);
        } while (Process32Next(hSnapshot, &pe32));
    }

    const char* endJson = "\r\n]\r\n";
    WriteFile(hFile, endJson, my_strlen(endJson), &written, NULL);

    CloseHandle(hFile);
    CloseHandle(hSnapshot);

    ShowNativeToast("Snapshot exported to 'ktask_export.json' successfully!");
}

void PerformExportMD(HWND hwnd) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        ShowNativeToast("Failed to create process snapshot.");
        return;
    }

    HANDLE hFile = CreateFileA("ktask_audit_report.md", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        CloseHandle(hSnapshot);
        ShowNativeToast("Failed to create ktask_audit_report.md file.");
        return;
    }

    DWORD written = 0;
    const char* title = "# KTask Process & System Diagnostic Audit Report\r\n\r\n";
    WriteFile(hFile, title, my_strlen(title), &written, NULL);

    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    GlobalMemoryStatusEx(&memStatus);

    char sysInfo[512] = {0};
    char memLoadStr[16] = {0};
    char totalMemMB[16] = {0};
    char availMemMB[16] = {0};
    my_utoa(memStatus.dwMemoryLoad, memLoadStr);
    my_utoa((DWORD)(memStatus.ullTotalPhys / (1024 * 1024)), totalMemMB);
    my_utoa((DWORD)(memStatus.ullAvailPhys / (1024 * 1024)), availMemMB);

    my_strcpy(sysInfo, "## System Resource Summary\r\n");
    my_strcat(sysInfo, "- **Memory Load**: ");
    my_strcat(sysInfo, memLoadStr);
    my_strcat(sysInfo, "%\r\n- **Physical RAM**: ");
    my_strcat(sysInfo, totalMemMB);
    my_strcat(sysInfo, " MB Total (");
    my_strcat(sysInfo, availMemMB);
    my_strcat(sysInfo, " MB Available)\r\n\r\n## Active Process Inventory\r\n\r\n");
    my_strcat(sysInfo, "| PID | PPID | Process Name | Threads | Base Priority |\r\n");
    my_strcat(sysInfo, "|---|---|---|---|---|\r\n");
    WriteFile(hFile, sysInfo, my_strlen(sysInfo), &written, NULL);

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &pe32)) {
        do {
            char row[512] = {0};
            char pidStr[16] = {0};
            char ppidStr[16] = {0};
            char thrStr[16] = {0};
            char priStr[16] = {0};

            my_utoa(pe32.th32ProcessID, pidStr);
            my_utoa(pe32.th32ParentProcessID, ppidStr);
            my_utoa(pe32.cntThreads, thrStr);
            my_itoa(pe32.pcPriClassBase, priStr);

            my_strcpy(row, "| `");
            my_strcat(row, pidStr);
            my_strcat(row, "` | `");
            my_strcat(row, ppidStr);
            my_strcat(row, "` | **");
            my_strcat(row, pe32.szExeFile);
            my_strcat(row, "** | ");
            my_strcat(row, thrStr);
            my_strcat(row, " | ");
            my_strcat(row, priStr);
            my_strcat(row, " |\r\n");

            WriteFile(hFile, row, my_strlen(row), &written, NULL);
        } while (Process32Next(hSnapshot, &pe32));
    }

    const char* footer = "\r\n---\r\n*Generated autonomously by KTask Process Monitor diagnostic engine.*\r\n";
    WriteFile(hFile, footer, my_strlen(footer), &written, NULL);

    CloseHandle(hFile);
    CloseHandle(hSnapshot);

    ShowNativeToast("Report saved to 'ktask_audit_report.md' successfully!");
}

void ShowHelpDialog(HWND hwnd) {
    MessageBoxA(hwnd,
        "KTask Process Monitor & Diagnostic Suite\r\n\r\n"
        "Keyboard Shortcuts:\r\n"
        "  F1 or H   : View this Help dialog\r\n"
        "  F5        : Quicksave process snapshot to ktask.dat\r\n"
        "  F9        : Quickload process snapshot from ktask.dat\r\n"
        "  R         : Refresh active process list (live)\r\n"
        "  T         : Toggle Process Tree / Flat List view\r\n"
        "  Del       : Terminate selected process\r\n"
        "  Enter / I : Deep Inspect selected process\r\n"
        "  P         : Cycle priority (Normal -> High -> Below Normal)\r\n"
        "  A         : Cycle CPU Core Affinity mask\r\n"
        "  C         : Export process list to CSV (ktask_export.csv)\r\n"
        "  J         : Export process list to JSON (ktask_export.json)\r\n"
        "  M         : Export System Diagnostic Audit Report (ktask_audit_report.md)\r\n"
        "  Esc       : Clear search filter / dismiss\r\n\r\n"
        "Advanced Filter Syntax:\r\n"
        "  Type 'pid:123' to match PID\r\n"
        "  Type 'pri:8' to match Priority\r\n"
        "  Type any text to search executable name\r\n\r\n"
        "Double-click any process to open Deep Inspector.",
        "KTask Help & Shortcuts", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN) {
        if (wParam == VK_F5) {
            if (SaveStateToFile("ktask.dat")) {
                ShowNativeToast("★ Diagnostics snapshot quicksaved to ktask.dat [F5]");
            } else {
                ShowNativeToast("⚠ Failed to quicksave snapshot.");
            }
            return 0;
        } else if (wParam == VK_F9) {
            if (!LoadStateFromFile("ktask.dat")) {
                ShowNativeToast("⚠ No quicksave snapshot found (ktask.dat).");
            }
            return 0;
        } else if (wParam == VK_ESCAPE) {
            SetWindowTextA(hwnd, "");
            g_isSnapshotMode = FALSE;
            RefreshList();
            return 0;
        } else if (wParam == VK_RETURN) {
            g_isSnapshotMode = FALSE;
            RefreshList();
            if (hListBox) SetFocus(hListBox);
            return 0;
        } else if (wParam == VK_F1) {
            ShowHelpDialog(GetParent(hwnd));
            return 0;
        }
    }
    return CallWindowProcA(g_OldEditProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK ListSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN) {
        if (wParam == VK_F5) {
            if (SaveStateToFile("ktask.dat")) {
                ShowNativeToast("★ Diagnostics snapshot quicksaved to ktask.dat [F5]");
            } else {
                ShowNativeToast("⚠ Failed to quicksave snapshot.");
            }
            return 0;
        } else if (wParam == VK_F9) {
            if (!LoadStateFromFile("ktask.dat")) {
                ShowNativeToast("⚠ No quicksave snapshot found (ktask.dat).");
            }
            return 0;
        } else if (wParam == 'R' || wParam == 'r') {
            g_isSnapshotMode = FALSE;
            RefreshList();
            ShowNativeToast("Live process list refreshed [R].");
            return 0;
        } else if (wParam == 'T' || wParam == 't') {
            g_isTreeView = !g_isTreeView;
            RefreshList();
            ShowNativeToast(g_isTreeView ? "View: Process Tree Hierarchy" : "View: Flat Process List");
            return 0;
        } else if (wParam == VK_DELETE) {
            PerformEndTask(GetParent(hwnd));
            return 0;
        } else if (wParam == 'I' || wParam == 'i' || wParam == VK_RETURN) {
            PerformInspectProcess(GetParent(hwnd));
            return 0;
        } else if (wParam == 'P' || wParam == 'p') {
            PerformSetPriority(GetParent(hwnd));
            return 0;
        } else if (wParam == 'A' || wParam == 'a') {
            PerformSetAffinity(GetParent(hwnd));
            return 0;
        } else if (wParam == 'C' || wParam == 'c') {
            PerformExportCSV(GetParent(hwnd));
            return 0;
        } else if (wParam == 'J' || wParam == 'j') {
            PerformExportJSON(GetParent(hwnd));
            return 0;
        } else if (wParam == 'M' || wParam == 'm') {
            PerformExportMD(GetParent(hwnd));
            return 0;
        } else if (wParam == VK_F1 || wParam == 'H' || wParam == 'h') {
            ShowHelpDialog(GetParent(hwnd));
            return 0;
        }
    }
    return CallWindowProcA(g_OldListProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_GETMINMAXINFO: {
            MINMAXINFO* mmi = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = 480;
            mmi->ptMinTrackSize.y = 300;
            return 0;
        }
        case WM_CREATE: {
            hSearchBox = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 10, 10, 360, 25, hwnd, (HMENU)3, NULL, NULL);
#ifndef EM_SETCUEBANNER
#define EM_SETCUEBANNER 0x1501
#endif
            SendMessageW(hSearchBox, EM_SETCUEBANNER, 0, (LPARAM)L"Filter Name or pid:123, pri:8... (Enter: focus list | F1: Help)");
            hListBox = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT, 10, 45, 360, 165, hwnd, (HMENU)4, NULL, NULL);
            hStatusText = CreateWindowA("STATIC", "Processes: 0", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 215, 360, 20, hwnd, (HMENU)5, NULL, NULL);

            g_hFont = CreateFontA(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            if (!g_hFont) {
                NONCLIENTMETRICSA ncm;
                ncm.cbSize = sizeof(NONCLIENTMETRICSA);
                SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSA), &ncm, 0);
                g_hFont = CreateFontIndirectA(&ncm.lfMessageFont);
            }
            if (g_hFont) {
                SendMessageA(hSearchBox, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hListBox, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hStatusText, WM_SETFONT, (WPARAM)g_hFont, FALSE);
            }

            hBtnRefresh = CreateWindowA("BUTTON", "Refresh [R]", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 10, 240, 85, 25, hwnd, (HMENU)1, NULL, NULL);
            hBtnTree = CreateWindowA("BUTTON", "Tree [T]", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 100, 240, 70, 25, hwnd, (HMENU)11, NULL, NULL);
            hBtnInspect = CreateWindowA("BUTTON", "Inspect [I]", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 175, 240, 75, 25, hwnd, (HMENU)9, NULL, NULL);
            hBtnPriority = CreateWindowA("BUTTON", "Priority [P]", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 255, 240, 75, 25, hwnd, (HMENU)6, NULL, NULL);
            hBtnAffinity = CreateWindowA("BUTTON", "Affinity [A]", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 335, 240, 75, 25, hwnd, (HMENU)12, NULL, NULL);
            hBtnEndTask = CreateWindowA("BUTTON", "End Task [Del]", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 415, 240, 105, 25, hwnd, (HMENU)2, NULL, NULL);

            hBtnExportCSV = CreateWindowA("BUTTON", "CSV [C]", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 10, 270, 60, 25, hwnd, (HMENU)7, NULL, NULL);
            hBtnExportJSON = CreateWindowA("BUTTON", "JSON [J]", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 75, 270, 60, 25, hwnd, (HMENU)8, NULL, NULL);
            hBtnExportMD = CreateWindowA("BUTTON", "Report [M]", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 140, 270, 70, 25, hwnd, (HMENU)13, NULL, NULL);
            hBtnSave = CreateWindowA("BUTTON", "Save [F5]", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 215, 270, 75, 25, hwnd, (HMENU)14, NULL, NULL);
            hBtnLoad = CreateWindowA("BUTTON", "Load [F9]", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 295, 270, 75, 25, hwnd, (HMENU)15, NULL, NULL);
            hBtnHelp = CreateWindowA("BUTTON", "Help [F1]", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 375, 270, 70, 25, hwnd, (HMENU)10, NULL, NULL);
            
            if (g_hFont) {
                SendMessageA(hBtnRefresh, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hBtnTree, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hBtnInspect, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hBtnPriority, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hBtnAffinity, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hBtnEndTask, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hBtnExportCSV, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hBtnExportJSON, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hBtnExportMD, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hBtnSave, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hBtnLoad, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hBtnHelp, WM_SETFONT, (WPARAM)g_hFont, FALSE);
            }

            g_OldEditProc = (WNDPROC)SetWindowLongPtrA(hSearchBox, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);
            g_OldListProc = (WNDPROC)SetWindowLongPtrA(hListBox, GWLP_WNDPROC, (LONG_PTR)ListSubclassProc);
            
            RefreshList();
            SetTimer(hwnd, 1, 2500, NULL);
            break;
        }
        case WM_SIZE:
            LayoutControls(hwnd);
            break;

        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, (HBRUSH)(COLOR_BTNFACE + 1));
            return 1;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetBkMode(hdcStatic, TRANSPARENT);
            SetTextColor(hdcStatic, RGB(30, 30, 30));
            return (INT_PTR)GetSysColorBrush(COLOR_BTNFACE);
        }

        case WM_TIMER:
            if (wParam == 1) {
                if (g_toastTimer > 0) {
                    g_toastTimer--;
                    if (g_toastTimer == 0 && !g_isSnapshotMode) {
                        RefreshList();
                    }
                } else if (GetFocus() != hSearchBox && !g_isSnapshotMode) {
                    RefreshList();
                }
            }
            break;

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);
            if (id == 3 && code == EN_CHANGE) {
                g_isSnapshotMode = FALSE;
                RefreshList();
            } else if (id == 1) {
                g_isSnapshotMode = FALSE;
                RefreshList();
                ShowNativeToast("Process list refreshed.");
            } else if (id == 2) {
                PerformEndTask(hwnd);
            } else if (id == 6) {
                PerformSetPriority(hwnd);
            } else if (id == 7) {
                PerformExportCSV(hwnd);
            } else if (id == 8) {
                PerformExportJSON(hwnd);
            } else if (id == 9) {
                PerformInspectProcess(hwnd);
            } else if (id == 10) {
                ShowHelpDialog(hwnd);
            } else if (id == 11) {
                g_isTreeView = !g_isTreeView;
                RefreshList();
                ShowNativeToast(g_isTreeView ? "View: Process Tree Hierarchy" : "View: Flat Process List");
            } else if (id == 12) {
                PerformSetAffinity(hwnd);
            } else if (id == 13) {
                PerformExportMD(hwnd);
            } else if (id == 14) {
                if (SaveStateToFile("ktask.dat")) {
                    ShowNativeToast("★ Diagnostics snapshot quicksaved to ktask.dat [F5]");
                } else {
                    ShowNativeToast("⚠ Failed to quicksave snapshot.");
                }
            } else if (id == 15) {
                if (!LoadStateFromFile("ktask.dat")) {
                    ShowNativeToast("⚠ No quicksave snapshot found (ktask.dat).");
                }
            } else if (id == 4 && code == LBN_DBLCLK) {
                PerformInspectProcess(hwnd);
            }
            break;
        }

        case WM_KEYDOWN:
            if (wParam == VK_F5) {
                if (SaveStateToFile("ktask.dat")) {
                    ShowNativeToast("★ Diagnostics snapshot quicksaved to ktask.dat [F5]");
                } else {
                    ShowNativeToast("⚠ Failed to quicksave snapshot.");
                }
                return 0;
            } else if (wParam == VK_F9) {
                if (!LoadStateFromFile("ktask.dat")) {
                    ShowNativeToast("⚠ No quicksave snapshot found (ktask.dat).");
                }
                return 0;
            } else if (wParam == 'R' || wParam == 'r') {
                g_isSnapshotMode = FALSE;
                RefreshList();
                ShowNativeToast("Live process list refreshed [R].");
                return 0;
            } else if (wParam == 'T' || wParam == 't') {
                g_isTreeView = !g_isTreeView;
                RefreshList();
                ShowNativeToast(g_isTreeView ? "View: Process Tree Hierarchy" : "View: Flat Process List");
                return 0;
            } else if (wParam == VK_DELETE) {
                PerformEndTask(hwnd);
                return 0;
            } else if (wParam == 'I' || wParam == 'i' || wParam == VK_RETURN) {
                PerformInspectProcess(hwnd);
                return 0;
            } else if (wParam == 'P' || wParam == 'p') {
                PerformSetPriority(hwnd);
                return 0;
            } else if (wParam == 'A' || wParam == 'a') {
                PerformSetAffinity(hwnd);
                return 0;
            } else if (wParam == 'C' || wParam == 'c') {
                PerformExportCSV(hwnd);
                return 0;
            } else if (wParam == 'J' || wParam == 'j') {
                PerformExportJSON(hwnd);
                return 0;
            } else if (wParam == 'M' || wParam == 'm') {
                PerformExportMD(hwnd);
                return 0;
            } else if (wParam == VK_F1 || wParam == 'H' || wParam == 'h') {
                ShowHelpDialog(hwnd);
                return 0;
            }
            break;

        case WM_DESTROY:
            SaveStateToFile("ktask.dat");
            KillTimer(hwnd, 1);
            if (g_hFont) {
                DeleteObject(g_hFont);
                g_hFont = NULL;
            }
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
    wc.lpszClassName = "KTaskClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    RegisterClassA(&wc);
    
    RECT rc = {0, 0, 800, 600};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    
    HWND hwnd = CreateWindowExA(0, "KTaskClass", "KTask Process Monitor (F1: Help | T: Tree | I: Inspect | P: Priority | A: Affinity | M: Report)", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    if (HasSavedState("ktask.dat")) {
        if (LoadStateFromFile("ktask.dat")) {
            ShowNativeToast("★ Restored saved state from ktask.dat [F9]");
        }
    } else if (!HasSeenTutorial()) {
        ShowHelpDialog(hwnd);
        MarkTutorialSeen();
        ShowNativeToast("Welcome to KTask! Process monitor initialized");
    }

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            HWND focusWnd = GetFocus();
            if (focusWnd == hSearchBox) {
                if (msg.wParam == VK_F1) { ShowHelpDialog(hwnd); continue; }
                if (msg.wParam == VK_F5) {
                    if (SaveStateToFile("ktask.dat")) {
                        ShowNativeToast("★ Diagnostics snapshot quicksaved to ktask.dat [F5]");
                    } else {
                        ShowNativeToast("⚠ Failed to quicksave snapshot.");
                    }
                    continue;
                }
                if (msg.wParam == VK_F9) {
                    if (!LoadStateFromFile("ktask.dat")) {
                        ShowNativeToast("⚠ No quicksave snapshot found (ktask.dat).");
                    }
                    continue;
                }
                if (msg.wParam == VK_ESCAPE) { SetWindowTextA(hSearchBox, ""); g_isSnapshotMode = FALSE; RefreshList(); continue; }
                if (msg.wParam == VK_RETURN) { g_isSnapshotMode = FALSE; RefreshList(); if (hListBox) SetFocus(hListBox); continue; }
            } else {
                if (msg.wParam == VK_F1 || msg.wParam == 'H' || msg.wParam == 'h') { ShowHelpDialog(hwnd); continue; }
                if (msg.wParam == VK_F5) {
                    if (SaveStateToFile("ktask.dat")) {
                        ShowNativeToast("★ Diagnostics snapshot quicksaved to ktask.dat [F5]");
                    } else {
                        ShowNativeToast("⚠ Failed to quicksave snapshot.");
                    }
                    continue;
                }
                if (msg.wParam == VK_F9) {
                    if (!LoadStateFromFile("ktask.dat")) {
                        ShowNativeToast("⚠ No quicksave snapshot found (ktask.dat).");
                    }
                    continue;
                }
                if (msg.wParam == 'R' || msg.wParam == 'r') { g_isSnapshotMode = FALSE; RefreshList(); ShowNativeToast("Process list refreshed."); continue; }
                if (msg.wParam == 'T' || msg.wParam == 't') {
                    g_isTreeView = !g_isTreeView;
                    RefreshList();
                    ShowNativeToast(g_isTreeView ? "View: Process Tree Hierarchy" : "View: Flat Process List");
                    continue;
                }
                if (msg.wParam == VK_DELETE) { PerformEndTask(hwnd); continue; }
                if (msg.wParam == 'I' || msg.wParam == 'i') { PerformInspectProcess(hwnd); continue; }
                if (msg.wParam == 'P' || msg.wParam == 'p') { PerformSetPriority(hwnd); continue; }
                if (msg.wParam == 'A' || msg.wParam == 'a') { PerformSetAffinity(hwnd); continue; }
                if (msg.wParam == 'C' || msg.wParam == 'c') { PerformExportCSV(hwnd); continue; }
                if (msg.wParam == 'J' || msg.wParam == 'j') { PerformExportJSON(hwnd); continue; }
                if (msg.wParam == 'M' || msg.wParam == 'm') { PerformExportMD(hwnd); continue; }
                if (msg.wParam == VK_ESCAPE) { SetWindowTextA(hSearchBox, ""); g_isSnapshotMode = FALSE; RefreshList(); continue; }
            }
        }
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    ExitProcess(0);
}
