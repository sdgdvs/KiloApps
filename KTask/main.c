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
HWND hBtnEndTask = NULL;
HWND hBtnPriority = NULL;
HWND hBtnInspect = NULL;
HWND hBtnExportCSV = NULL;
HWND hBtnExportJSON = NULL;
HWND hBtnHelp = NULL;
HWND hStatusText = NULL;
HFONT g_hFont = NULL;
WNDPROC g_OldEditProc = NULL;
WNDPROC g_OldListProc = NULL;

void ShowHelpDialog(HWND hwnd);

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
    MoveWindow(hListBox, 10, 42, width - 20, height - 105, TRUE);
    
    int btnY = height - 58;
    int curX = 10;
    MoveWindow(hBtnRefresh, curX, btnY, 85, 24, TRUE); curX += 90;
    MoveWindow(hBtnPriority, curX, btnY, 85, 24, TRUE); curX += 90;
    MoveWindow(hBtnInspect, curX, btnY, 80, 24, TRUE); curX += 85;
    MoveWindow(hBtnExportCSV, curX, btnY, 45, 24, TRUE); curX += 50;
    MoveWindow(hBtnExportJSON, curX, btnY, 45, 24, TRUE); curX += 50;
    MoveWindow(hBtnHelp, curX, btnY, 75, 24, TRUE); curX += 80;

    int endTaskX = width - 115;
    if (endTaskX < curX + 5) endTaskX = curX + 5;
    MoveWindow(hBtnEndTask, endTaskX, btnY, 105, 24, TRUE);

    MoveWindow(hStatusText, 10, height - 28, width - 20, 20, TRUE);
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

    if (Process32First(hSnapshot, &pe32)) {
        do {
            totalTasks++;
            totalThreads += pe32.cntThreads;
            if (filter[0] && !my_stristr(pe32.szExeFile, filter)) continue;

            char buf[512] = {0};
            char pidStr[16] = {0};
            char thrStr[16] = {0};
            char priStr[16] = {0};

            my_utoa(pe32.th32ProcessID, pidStr);
            my_utoa(pe32.cntThreads, thrStr);
            my_itoa(pe32.pcPriClassBase, priStr);

            my_strcpy(buf, "[PID: ");
            my_strcat(buf, pidStr);
            my_strcat(buf, "] ");
            my_strcat(buf, pe32.szExeFile);
            my_strcat(buf, " (Threads: ");
            my_strcat(buf, thrStr);
            my_strcat(buf, ", BasePri: ");
            my_strcat(buf, priStr);
            my_strcat(buf, ")");

            int index = SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)buf);
            SendMessageA(hListBox, LB_SETITEMDATA, index, (LPARAM)pe32.th32ProcessID);
            shownTasks++;
        } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);

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

    if (hStatusText) {
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

        my_strcpy(statusBuf, "Procs: ");
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
    my_strcat(report, "           KTASK DEEP PROCESS & MEMORY INSPECTOR REPORT           \r\n");
    my_strcat(report, "===================================================================\r\n");
    my_strcat(report, "Process Info: ");
    my_strcat(report, itemText);
    my_strcat(report, "\r\n\r\n");

    // 1. Thread List
    my_strcat(report, "-------------------------------------------------------------------\r\n");
    my_strcat(report, " 1. ACTIVE THREADS SNAPSHOT (Toolhelp32)\r\n");
    my_strcat(report, "-------------------------------------------------------------------\r\n");

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
                    my_strcat(report, lineBuf);
                }
            } while (Thread32Next(hSnap, &te32));
        }
        CloseHandle(hSnap);
    }
    if (threadIdx == 0) {
        my_strcat(report, "  (No accessible thread details found or process restricted)\r\n");
    }

    // 2. Loaded Modules
    my_strcat(report, "\r\n-------------------------------------------------------------------\r\n");
    my_strcat(report, " 2. LOADED MODULES & LIBRARIES (Toolhelp32)\r\n");
    my_strcat(report, "-------------------------------------------------------------------\r\n");

    HANDLE hModSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    int modCount = 0;
    if (hModSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 me32;
        me32.dwSize = sizeof(MODULEENTRY32);
        if (Module32First(hModSnap, &me32)) {
            do {
                modCount++;
                if (modCount > 40) {
                    my_strcat(report, "  ... [Truncated remaining loaded modules] ...\r\n");
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
                my_strcat(report, lineBuf);
            } while (Module32Next(hModSnap, &me32));
        }
        CloseHandle(hModSnap);
    }
    if (modCount == 0) {
        my_strcat(report, "  (Module query denied or unavailable for system process)\r\n");
    }

    // 3. Virtual Memory Map
    my_strcat(report, "\r\n-------------------------------------------------------------------\r\n");
    my_strcat(report, " 3. VIRTUAL MEMORY REGION MAP (VirtualQueryEx)\r\n");
    my_strcat(report, "-------------------------------------------------------------------\r\n");

    HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    int regCount = 0;
    if (hProc) {
        MEMORY_BASIC_INFORMATION mbi;
        unsigned char* pAddr = NULL;
        while (VirtualQueryEx(hProc, pAddr, &mbi, sizeof(mbi)) == sizeof(mbi)) {
            regCount++;
            if (regCount > 50) {
                my_strcat(report, "  ... [Truncated remaining memory regions] ...\r\n");
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

            my_strcat(report, lineBuf);

            unsigned char* nextAddr = (unsigned char*)mbi.BaseAddress + mbi.RegionSize;
            if (nextAddr <= pAddr) break;
            pAddr = nextAddr;
        }
        CloseHandle(hProc);
    }
    if (regCount == 0) {
        my_strcat(report, "  (Virtual Memory query restricted for this system process)\r\n");
    }

    my_strcat(report, "===================================================================\r\n");

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
        my_strcpy(msgBuf, "Process priority class updated to ");
        my_strcat(msgBuf, priName);
        my_strcat(msgBuf, ".");
        MessageBoxA(hwnd, msgBuf, "KTask Success", MB_OK | MB_ICONINFORMATION);
        RefreshList();
    } else {
        MessageBoxA(hwnd, "Failed to update priority class.", "KTask Error", MB_OK | MB_ICONERROR);
    }
    CloseHandle(hProc);
}

void PerformExportCSV(HWND hwnd) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        MessageBoxA(hwnd, "Failed to create process snapshot.", "Export Failed", MB_OK | MB_ICONERROR);
        return;
    }

    HANDLE hFile = CreateFileA("ktask_export.csv", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        CloseHandle(hSnapshot);
        MessageBoxA(hwnd, "Failed to create ktask_export.csv file.", "Export Failed", MB_OK | MB_ICONERROR);
        return;
    }

    const char* header = "PID,Process Name,Threads,BasePriority\r\n";
    DWORD written = 0;
    WriteFile(hFile, header, my_strlen(header), &written, NULL);

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &pe32)) {
        do {
            char lineBuf[512] = {0};
            char pidStr[16] = {0};
            char thrStr[16] = {0};
            char priStr[16] = {0};
            char escapedExe[256] = {0};

            my_utoa(pe32.th32ProcessID, pidStr);
            my_utoa(pe32.cntThreads, thrStr);
            my_itoa(pe32.pcPriClassBase, priStr);
            my_escape_csv(pe32.szExeFile, escapedExe, sizeof(escapedExe));

            my_strcpy(lineBuf, pidStr);
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

    MessageBoxA(hwnd, "Process list snapshot exported to 'ktask_export.csv'!", "Export Complete", MB_OK | MB_ICONINFORMATION);
}

void PerformExportJSON(HWND hwnd) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        MessageBoxA(hwnd, "Failed to create process snapshot.", "Export Failed", MB_OK | MB_ICONERROR);
        return;
    }

    HANDLE hFile = CreateFileA("ktask_export.json", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        CloseHandle(hSnapshot);
        MessageBoxA(hwnd, "Failed to create ktask_export.json file.", "Export Failed", MB_OK | MB_ICONERROR);
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
            char thrStr[16] = {0};
            char priStr[16] = {0};
            char escapedExe[256] = {0};

            my_utoa(pe32.th32ProcessID, pidStr);
            my_utoa(pe32.cntThreads, thrStr);
            my_itoa(pe32.pcPriClassBase, priStr);
            my_escape_json(pe32.szExeFile, escapedExe, sizeof(escapedExe));

            my_strcpy(lineBuf, "  {\"pid\": ");
            my_strcat(lineBuf, pidStr);
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

    MessageBoxA(hwnd, "Process list snapshot exported to 'ktask_export.json'!", "Export Complete", MB_OK | MB_ICONINFORMATION);
}

void ShowHelpDialog(HWND hwnd) {
    MessageBoxA(hwnd,
        "KTask Process Monitor\r\n\r\n"
        "Keyboard Shortcuts:\r\n"
        "  F1 or H   : View this Help dialog\r\n"
        "  F5 or R   : Refresh active process list\r\n"
        "  Del       : Terminate selected process\r\n"
        "  Enter / I : Deep Inspect selected process\r\n"
        "  Esc       : Clear search filter / dismiss\r\n\r\n"
        "Toolbar Buttons:\r\n"
        "  - Refresh [F5]: Live snapshot of running processes\r\n"
        "  - Set Priority: Cycle priority (Normal -> High -> Below Normal)\r\n"
        "  - Inspect [I] : View Threads, Loaded DLLs, and Memory Map\r\n"
        "  - CSV / JSON  : Export process snapshot data\r\n"
        "  - Help [F1]   : Show shortcuts & documentation\r\n"
        "  - End Task [Del]: Safely terminate unresponsive process\r\n\r\n"
        "Double-click any process to open Deep Inspector.",
        "KTask Help & Shortcuts", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN) {
        if (wParam == VK_F5) {
            RefreshList();
            return 0;
        } else if (wParam == VK_ESCAPE) {
            SetWindowTextA(hwnd, "");
            RefreshList();
            return 0;
        } else if (wParam == VK_RETURN) {
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
        if (wParam == VK_F5 || wParam == 'R' || wParam == 'r') {
            RefreshList();
            return 0;
        } else if (wParam == VK_DELETE) {
            PerformEndTask(GetParent(hwnd));
            return 0;
        } else if (wParam == 'I' || wParam == 'i' || wParam == VK_RETURN) {
            PerformInspectProcess(GetParent(hwnd));
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
            mmi->ptMinTrackSize.x = 440;
            mmi->ptMinTrackSize.y = 300;
            return 0;
        }
        case WM_CREATE: {
            hSearchBox = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 10, 10, 360, 25, hwnd, (HMENU)3, NULL, NULL);
#ifndef EM_SETCUEBANNER
#define EM_SETCUEBANNER 0x1501
#endif
            SendMessageW(hSearchBox, EM_SETCUEBANNER, 0, (LPARAM)L"Filter by Name or PID... (Enter: focus list | F1: Help)");
            hListBox = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT, 10, 45, 360, 165, hwnd, (HMENU)4, NULL, NULL);
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

            hBtnRefresh = CreateWindowA("BUTTON", "Refresh [F5]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 240, 85, 25, hwnd, (HMENU)1, NULL, NULL);
            hBtnPriority = CreateWindowA("BUTTON", "Set Priority", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 100, 240, 85, 25, hwnd, (HMENU)6, NULL, NULL);
            hBtnInspect = CreateWindowA("BUTTON", "Inspect [I]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 190, 240, 80, 25, hwnd, (HMENU)9, NULL, NULL);
            hBtnExportCSV = CreateWindowA("BUTTON", "CSV", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 275, 240, 45, 25, hwnd, (HMENU)7, NULL, NULL);
            hBtnExportJSON = CreateWindowA("BUTTON", "JSON", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 325, 240, 45, 25, hwnd, (HMENU)8, NULL, NULL);
            hBtnHelp = CreateWindowA("BUTTON", "Help [F1]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 375, 240, 75, 25, hwnd, (HMENU)10, NULL, NULL);
            hBtnEndTask = CreateWindowA("BUTTON", "End Task [Del]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 455, 240, 105, 25, hwnd, (HMENU)2, NULL, NULL);
            
            if (g_hFont) {
                SendMessageA(hBtnRefresh, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hBtnPriority, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hBtnInspect, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hBtnExportCSV, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hBtnExportJSON, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hBtnHelp, WM_SETFONT, (WPARAM)g_hFont, FALSE);
                SendMessageA(hBtnEndTask, WM_SETFONT, (WPARAM)g_hFont, FALSE);
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

        case WM_TIMER:
            if (wParam == 1 && GetFocus() != hSearchBox) {
                RefreshList();
            }
            break;

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);
            if (id == 3 && code == EN_CHANGE) {
                RefreshList();
            } else if (id == 1) {
                RefreshList();
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
            } else if (id == 4 && code == LBN_DBLCLK) {
                PerformInspectProcess(hwnd);
            }
            break;
        }

        case WM_KEYDOWN:
            if (wParam == VK_F5 || wParam == 'R' || wParam == 'r') {
                RefreshList();
                return 0;
            } else if (wParam == VK_DELETE) {
                PerformEndTask(hwnd);
                return 0;
            } else if (wParam == 'I' || wParam == 'i' || wParam == VK_RETURN) {
                PerformInspectProcess(hwnd);
                return 0;
            } else if (wParam == VK_F1 || wParam == 'H' || wParam == 'h') {
                ShowHelpDialog(hwnd);
                return 0;
            }
            break;

        case WM_DESTROY:
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
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);
    
    RECT rc = {0, 0, 800, 600};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    
    HWND hwnd = CreateWindowExA(0, "KTaskClass", "KTask Process Monitor (F1/H: Help | F5/R: Refresh | Del: End Task | I/Enter: Inspect)", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
