#include <windows.h>
#include <tlhelp32.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

HWND hListBox;
HWND hSearchBox;
HWND hBtnRefresh, hBtnEndTask;
HWND hStatusText;

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

void my_itoa(int num, char* str) {
    if (num < 0) {
        *str++ = '-';
        num = -num;
    }
    my_utoa((DWORD)num, str);
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

void LayoutControls(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    if (width < 200 || height < 150) return;

    MoveWindow(hSearchBox, 10, 10, width - 20, 25, TRUE);
    MoveWindow(hListBox, 10, 42, width - 20, height - 78, TRUE);
    MoveWindow(hBtnRefresh, 10, height - 30, 95, 24, TRUE);
    MoveWindow(hBtnEndTask, width - 105, height - 30, 95, 24, TRUE);
}

void RefreshList() {
    DWORD selectedPid = 0;
    int currentSel = SendMessageA(hListBox, LB_GETCURSEL, 0, 0);
    if (currentSel != LB_ERR) {
        selectedPid = (DWORD)SendMessageA(hListBox, LB_GETITEMDATA, currentSel, 0);
    }

    SendMessageA(hListBox, LB_RESETCONTENT, 0, 0);
    char filter[256] = {0};
    if (hSearchBox) GetWindowTextA(hSearchBox, filter, sizeof(filter) - 1);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)"[Error: Unable to snapshot processes]");
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    int totalTasks = 0;

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (filter[0] && !my_stristr(pe32.szExeFile, filter)) continue;

            char buf[MAX_PATH + 128] = {0};
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
            totalTasks++;
        } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);

    // Restore selection if process is still running
    if (selectedPid != 0 && totalTasks > 0) {
        int count = SendMessageA(hListBox, LB_GETCOUNT, 0, 0);
        for (int i = 0; i < count; i++) {
            DWORD pid = (DWORD)SendMessageA(hListBox, LB_GETITEMDATA, i, 0);
            if (pid == selectedPid) {
                SendMessageA(hListBox, LB_SETCURSEL, i, 0);
                break;
            }
        }
    }
}

void PerformEndTask(HWND hwnd) {
    int sel = SendMessageA(hListBox, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR) {
        MessageBoxA(hwnd, "Please select a process from the list first.", "KTask Notice", MB_OK | MB_ICONINFORMATION);
        return;
    }

    DWORD pid = SendMessageA(hListBox, LB_GETITEMDATA, sel, 0);
    if (pid == 0) return;

    if (pid == GetCurrentProcessId()) {
        if (MessageBoxA(hwnd, "Terminating KTask will exit this application. Continue?", "Confirm End Task", MB_YESNO | MB_ICONWARNING) != IDYES) {
            return;
        }
    }

    HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProc) {
        if (TerminateProcess(hProc, 1)) {
            CloseHandle(hProc);
            RefreshList();
        } else {
            CloseHandle(hProc);
            MessageBoxA(hwnd, "Failed to terminate process.", "KTask Error", MB_OK | MB_ICONERROR);
        }
    } else {
        MessageBoxA(hwnd, "Access Denied or Process Exited.", "KTask Error", MB_OK | MB_ICONERROR);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hSearchBox = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 10, 10, 360, 25, hwnd, (HMENU)3, NULL, NULL);
            hListBox = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT, 10, 45, 360, 165, hwnd, (HMENU)4, NULL, NULL);
            
            HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            SendMessageA(hSearchBox, WM_SETFONT, (WPARAM)hFont, FALSE);
            SendMessageA(hListBox, WM_SETFONT, (WPARAM)hFont, FALSE);

            hBtnRefresh = CreateWindowA("BUTTON", "Refresh", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 220, 95, 25, hwnd, (HMENU)1, NULL, NULL);
            hBtnEndTask = CreateWindowA("BUTTON", "End Task", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 275, 220, 95, 25, hwnd, (HMENU)2, NULL, NULL);
            
            SendMessageA(hBtnRefresh, WM_SETFONT, (WPARAM)hFont, FALSE);
            SendMessageA(hBtnEndTask, WM_SETFONT, (WPARAM)hFont, FALSE);
            
            RefreshList();
            SetTimer(hwnd, 1, 2500, NULL); // Auto refresh timer
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
            } else if (id == 1) { // Refresh button
                RefreshList();
            } else if (id == 2) { // End Task button
                PerformEndTask(hwnd);
            } else if (id == 4 && code == LBN_DBLCLK) { // Double click item in list
                PerformEndTask(hwnd);
            }
            break;
        }

        case WM_KEYDOWN:
            if (wParam == VK_F5) {
                RefreshList();
                return 0;
            } else if (wParam == VK_DELETE) {
                PerformEndTask(hwnd);
                return 0;
            }
            break;

        case WM_DESTROY:
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KTaskClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);

    RegisterClassA(&wc);
    
    RECT rc = {0, 0, 420, 320};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    
    HWND hwnd = CreateWindowExA(0, "KTaskClass", "KTask Process Monitor", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    ExitProcess(0);
}

