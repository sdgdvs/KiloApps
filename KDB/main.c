#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#define W 500
#define H 450
#define IDC_SEARCH 101
#define IDC_ADD_ID 102
#define IDC_ADD_NAME 103
#define IDC_ADD_DEPT 104
#define IDC_ADD_ROLE 105
#define IDC_ADD_BTN 106
#define IDC_DEL_BTN 107

HWND hListView;
HWND hSearch;
HWND hAddId, hAddName, hAddDept, hAddRole, hAddBtn, hDelBtn;
HFONT hFont;
HBRUSH hBgBrush;

const char* headers[] = {"ID", "Name", "Department", "Role"};

#define MAX_RECORDS 100
char data[MAX_RECORDS][4][64] = {
    {"101", "Alice Smith", "Engineering", "Developer"},
    {"102", "Bob Johnson", "Marketing", "Designer"},
    {"103", "Charlie Davis", "Sales", "Executive"},
    {"104", "Diana Prince", "Engineering", "Lead"},
    {"105", "Evan Wright", "HR", "Manager"}
};
int data_count = 5;

char ToLower(char c) {
    if (c >= 'A' && c <= 'Z') return c + 32;
    return c;
}

int StrContainsI(const char* haystack, const char* needle) {
    if (!needle || !*needle) return 1;
    if (!haystack || !*haystack) return 0;
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && ToLower(*h) == ToLower(*n)) {
            h++;
            n++;
        }
        if (!*n) return 1;
        haystack++;
    }
    return 0;
}

void PopulateListView(const char* filter) {
    SendMessage(hListView, LVM_DELETEALLITEMS, 0, 0);
    LVITEMA lvi;
    
    int index = 0;
    for (int i = 0; i < data_count; i++) {
        int match = 0;
        for (int j = 0; j < 4; j++) {
            if (StrContainsI(data[i][j], filter)) {
                match = 1;
                break;
            }
        }
        
        if (match) {
            lvi.mask = LVIF_TEXT | LVIF_PARAM;
            lvi.iItem = index;
            lvi.iSubItem = 0;
            lvi.pszText = data[i][0];
            lvi.lParam = i;
            SendMessage(hListView, LVM_INSERTITEMA, 0, (LPARAM)&lvi);
            
            lvi.mask = LVIF_TEXT;
            for (int j = 1; j < 4; j++) {
                lvi.iSubItem = j;
                lvi.pszText = data[i][j];
                SendMessage(hListView, LVM_SETITEMTEXTA, index, (LPARAM)&lvi);
            }
            index++;
        }
    }
}

void InitListView(HWND hwnd) {
    hSearch = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        10, 10, W - 35, 25, hwnd, (HMENU)IDC_SEARCH, GetModuleHandle(NULL), NULL);

    hListView = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EDITLABELS,
        10, 45, W - 35, H - 95, hwnd, NULL, GetModuleHandle(NULL), NULL);

    hAddId = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0, hwnd, (HMENU)IDC_ADD_ID, GetModuleHandle(NULL), NULL);
    hAddName = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0, hwnd, (HMENU)IDC_ADD_NAME, GetModuleHandle(NULL), NULL);
    hAddDept = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0, hwnd, (HMENU)IDC_ADD_DEPT, GetModuleHandle(NULL), NULL);
    hAddRole = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0, hwnd, (HMENU)IDC_ADD_ROLE, GetModuleHandle(NULL), NULL);
    hAddBtn = CreateWindowEx(0, "BUTTON", "Add", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, (HMENU)IDC_ADD_BTN, GetModuleHandle(NULL), NULL);
    hDelBtn = CreateWindowEx(0, "BUTTON", "Del", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, (HMENU)IDC_DEL_BTN, GetModuleHandle(NULL), NULL);
        
    SendMessage(hListView, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    
    SendMessage(hListView, LVM_SETTEXTCOLOR, 0, (LPARAM)RGB(255, 255, 255));
    SendMessage(hListView, LVM_SETTEXTBKCOLOR, 0, (LPARAM)RGB(35, 40, 45));
    SendMessage(hListView, LVM_SETBKCOLOR, 0, (LPARAM)RGB(26, 32, 38));
    
    hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
    SendMessage(hListView, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hSearch, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hAddId, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hAddName, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hAddDept, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hAddRole, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hAddBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hDelBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

    LVCOLUMNA lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.fmt = LVCFMT_LEFT;
    
    int widths[] = {50, 150, 120, 120};
    for (int i = 0; i < 4; i++) {
        lvc.iSubItem = i;
        lvc.cx = widths[i];
        lvc.pszText = (char*)headers[i];
        SendMessage(hListView, LVM_INSERTCOLUMNA, i, (LPARAM)&lvc);
    }
    
    PopulateListView("");
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            InitCommonControls();
            InitListView(hwnd);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == IDC_SEARCH && HIWORD(wParam) == EN_CHANGE) {
                char buf[256];
                GetWindowTextA(hSearch, buf, sizeof(buf));
                PopulateListView(buf);
            } else if (LOWORD(wParam) == IDC_ADD_BTN) {
                if (data_count < MAX_RECORDS) {
                    GetWindowTextA(hAddId, data[data_count][0], 64);
                    GetWindowTextA(hAddName, data[data_count][1], 64);
                    GetWindowTextA(hAddDept, data[data_count][2], 64);
                    GetWindowTextA(hAddRole, data[data_count][3], 64);
                    data_count++;
                    SetWindowTextA(hAddId, "");
                    SetWindowTextA(hAddName, "");
                    SetWindowTextA(hAddDept, "");
                    SetWindowTextA(hAddRole, "");
                    char buf[256];
                    GetWindowTextA(hSearch, buf, sizeof(buf));
                    PopulateListView(buf);
                }
            } else if (LOWORD(wParam) == IDC_DEL_BTN) {
                int sel = SendMessage(hListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
                if (sel != -1) {
                    LVITEMA lvi;
                    lvi.mask = LVIF_PARAM;
                    lvi.iItem = sel;
                    lvi.iSubItem = 0;
                    SendMessage(hListView, LVM_GETITEMA, 0, (LPARAM)&lvi);
                    int idx = (int)lvi.lParam;
                    if (idx >= 0 && idx < data_count) {
                        for (int i = idx; i < data_count - 1; i++) {
                            for (int j = 0; j < 4; j++) {
                                lstrcpyA(data[i][j], data[i+1][j]);
                            }
                        }
                        data_count--;
                        char buf[256];
                        GetWindowTextA(hSearch, buf, sizeof(buf));
                        PopulateListView(buf);
                    }
                }
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            MoveWindow(hSearch, 10, 10, nw - 20, 25, TRUE);
            MoveWindow(hListView, 10, 45, nw - 20, nh - 90, TRUE);
            
            int by = nh - 35;
            int ew = (nw - 130) / 4;
            if (ew < 10) ew = 10;
            MoveWindow(hAddId, 10, by, ew, 25, TRUE);
            MoveWindow(hAddName, 10 + ew + 5, by, ew, 25, TRUE);
            MoveWindow(hAddDept, 10 + ew*2 + 10, by, ew, 25, TRUE);
            MoveWindow(hAddRole, 10 + ew*3 + 15, by, ew, 25, TRUE);
            MoveWindow(hAddBtn, 10 + ew*4 + 20, by, 40, 25, TRUE);
            MoveWindow(hDelBtn, 10 + ew*4 + 65, by, 40, 25, TRUE);
            break;
        }
        case WM_DESTROY:
            if (hFont) DeleteObject(hFont);
            if (hBgBrush) DeleteObject(hBgBrush);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) *bytes++ = (char)c;
    return dest;
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KDBApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    hBgBrush = CreateSolidBrush(RGB(26, 32, 38));
    wc.hbrBackground = hBgBrush;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KDBApp", "KDB - Employee Database", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, W, H, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
