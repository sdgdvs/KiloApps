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


void my_itoa(int num, char* str) {
    int i = 0;
    if (num == 0) { str[i++] = '0'; str[i] = '\0'; return; }
    while (num != 0) {
        int rem = num % 10;
        str[i++] = rem + '0';
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
    int len = 0;
    while (s[len]) len++;
    return len;
}

void my_strcpy(char* dest, const char* src) {
    while (*src) *dest++ = *src++;
    *dest = 0;
}

void my_strcat(char* dest, const char* src) {
    while (*dest) dest++;
    while (*src) *dest++ = *src++;
    *dest = 0;
}

int my_stristr(const char* haystack, const char* needle) {
    if (!*needle) return 1;
    for (int i = 0; haystack[i]; i++) {
        int j = 0;
        while (haystack[i+j] && needle[j]) {
            char c1 = haystack[i+j];
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

void RefreshList() {
    SendMessageA(hListBox, LB_RESETCONTENT, 0, 0);
    char filter[256] = {0};
    if (hSearchBox) GetWindowTextA(hSearchBox, filter, sizeof(filter));

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (filter[0] && !my_stristr(pe32.szExeFile, filter)) continue;

                char buf[MAX_PATH + 32] = {0};
                char pidStr[16] = {0};
                my_itoa(pe32.th32ProcessID, pidStr);
                my_strcpy(buf, "[");
                my_strcat(buf, pidStr);
                my_strcat(buf, "] ");
                my_strcat(buf, pe32.szExeFile);
                
                int index = SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)buf);
                SendMessageA(hListBox, LB_SETITEMDATA, index, (LPARAM)pe32.th32ProcessID);
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hSearchBox = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 10, 10, 260, 25, hwnd, (HMENU)3, NULL, NULL);
            hListBox = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT, 10, 45, 260, 165, hwnd, NULL, NULL, NULL);
            HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            SendMessageA(hSearchBox, WM_SETFONT, (WPARAM)hFont, FALSE);
            SendMessageA(hListBox, WM_SETFONT, (WPARAM)hFont, FALSE);

            hBtnRefresh = CreateWindowA("BUTTON", "Refresh", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 220, 100, 25, hwnd, (HMENU)1, NULL, NULL);
            hBtnEndTask = CreateWindowA("BUTTON", "End Task", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 170, 220, 100, 25, hwnd, (HMENU)2, NULL, NULL);
            
            SendMessageA(hBtnRefresh, WM_SETFONT, (WPARAM)hFont, FALSE);
            SendMessageA(hBtnEndTask, WM_SETFONT, (WPARAM)hFont, FALSE);
            
            RefreshList();
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);
            if (id == 3 && code == EN_CHANGE) {
                RefreshList();
            } else if (id == 1) { // Refresh
                RefreshList();
            } else if (id == 2) { // End Task
                int sel = SendMessageA(hListBox, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    DWORD pid = SendMessageA(hListBox, LB_GETITEMDATA, sel, 0);
                    HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
                    if (hProc) {
                        TerminateProcess(hProc, 1);
                        CloseHandle(hProc);
                        RefreshList();
                    } else {
                        MessageBoxA(hwnd, "Access Denied.", "Error", MB_ICONERROR);
                    }
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

void __stdcall MainEntry() {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KTaskClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);

    RegisterClassA(&wc);
    
    RECT rc = {0, 0, 280, 255};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, FALSE);
    
    HWND hwnd = CreateWindowExA(0, "KTaskClass", "KTask", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
