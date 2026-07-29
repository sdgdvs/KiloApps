#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#define W 560
#define H 480
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
HBRUSH hEditBgBrush;

const char* headers[] = {"ID", "Name", "Department", "Role"};

#define MAX_RECORDS 200
#define FILE_NAME "kdb_data.dat"

typedef struct {
    char id[16];
    char name[64];
    char dept[64];
    char role[64];
} Record;

Record data[MAX_RECORDS] = {
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

const char* FindChar(const char* str, char c) {
    if (!str) return NULL;
    while (*str) {
        if (*str == c) return str;
        str++;
    }
    return NULL;
}

void SaveDataToFile() {
    HANDLE hFile = CreateFileA(FILE_NAME, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hFile, &data_count, sizeof(int), &written, NULL);
        WriteFile(hFile, data, sizeof(Record) * data_count, &written, NULL);
        CloseHandle(hFile);
    }
}

void LoadDataFromFile() {
    HANDLE hFile = CreateFileA(FILE_NAME, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD readBytes = 0;
        int count = 0;
        if (ReadFile(hFile, &count, sizeof(int), &readBytes, NULL) && readBytes == sizeof(int)) {
            if (count >= 0 && count <= MAX_RECORDS) {
                if (ReadFile(hFile, data, sizeof(Record) * count, &readBytes, NULL) && readBytes == (DWORD)(sizeof(Record) * count)) {
                    data_count = count;
                }
            }
        }
        CloseHandle(hFile);
    }
}

// Bounded Query Matcher supporting dept:val, role:val, id:val, name:val or general text
int MatchQuery(const Record* rec, const char* query) {
    if (!query || !*query) return 1;

    char buf[128];
    lstrcpynA(buf, query, sizeof(buf));
    
    // Check for column query e.g. "dept:engineering"
    char* colon = (char*)FindChar(buf, ':');
    if (colon) {
        *colon = '\0';
        char* field = buf;
        char* val = colon + 1;
        while (*field == ' ') field++;
        while (*val == ' ') val++;

        if (lstrcmpiA(field, "dept") == 0 || lstrcmpiA(field, "department") == 0) {
            return StrContainsI(rec->dept, val);
        } else if (lstrcmpiA(field, "role") == 0) {
            return StrContainsI(rec->role, val);
        } else if (lstrcmpiA(field, "id") == 0) {
            return StrContainsI(rec->id, val);
        } else if (lstrcmpiA(field, "name") == 0) {
            return StrContainsI(rec->name, val);
        }
    }

    return StrContainsI(rec->id, query) ||
           StrContainsI(rec->name, query) ||
           StrContainsI(rec->dept, query) ||
           StrContainsI(rec->role, query);
}

void PopulateListView(const char* filter) {
    SendMessage(hListView, LVM_DELETEALLITEMS, 0, 0);
    LVITEMA lvi;
    
    int index = 0;
    for (int i = 0; i < data_count; i++) {
        if (MatchQuery(&data[i], filter)) {
            lvi.mask = LVIF_TEXT | LVIF_PARAM;
            lvi.iItem = index;
            lvi.iSubItem = 0;
            lvi.pszText = data[i].id;
            lvi.lParam = i;
            SendMessage(hListView, LVM_INSERTITEMA, 0, (LPARAM)&lvi);
            
            lvi.mask = LVIF_TEXT;
            
            lvi.iSubItem = 1;
            lvi.pszText = data[i].name;
            SendMessage(hListView, LVM_SETITEMTEXTA, index, (LPARAM)&lvi);

            lvi.iSubItem = 2;
            lvi.pszText = data[i].dept;
            SendMessage(hListView, LVM_SETITEMTEXTA, index, (LPARAM)&lvi);

            lvi.iSubItem = 3;
            lvi.pszText = data[i].role;
            SendMessage(hListView, LVM_SETITEMTEXTA, index, (LPARAM)&lvi);

            index++;
        }
    }
}

void AutoScaleListViewColumns(int totalWidth) {
    if (totalWidth < 100) return;
    int colId = 70;
    int remaining = totalWidth - colId - 25;
    if (remaining < 150) remaining = 150;

    int colName = (remaining * 40) / 100;
    int colDept = (remaining * 30) / 100;
    int colRole = (remaining * 30) / 100;

    ListView_SetColumnWidth(hListView, 0, colId);
    ListView_SetColumnWidth(hListView, 1, colName);
    ListView_SetColumnWidth(hListView, 2, colDept);
    ListView_SetColumnWidth(hListView, 3, colRole);
}

void InitListView(HWND hwnd) {
    LoadDataFromFile();

    hSearch = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        10, 10, W - 35, 25, hwnd, (HMENU)IDC_SEARCH, GetModuleHandle(NULL), NULL);

    hListView = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        10, 45, W - 35, H - 100, hwnd, NULL, GetModuleHandle(NULL), NULL);

    hAddId = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0, hwnd, (HMENU)IDC_ADD_ID, GetModuleHandle(NULL), NULL);
    hAddName = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0, hwnd, (HMENU)IDC_ADD_NAME, GetModuleHandle(NULL), NULL);
    hAddDept = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0, hwnd, (HMENU)IDC_ADD_DEPT, GetModuleHandle(NULL), NULL);
    hAddRole = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0, hwnd, (HMENU)IDC_ADD_ROLE, GetModuleHandle(NULL), NULL);
    hAddBtn = CreateWindowEx(0, "BUTTON", "Add", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, (HMENU)IDC_ADD_BTN, GetModuleHandle(NULL), NULL);
    hDelBtn = CreateWindowEx(0, "BUTTON", "Del", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, (HMENU)IDC_DEL_BTN, GetModuleHandle(NULL), NULL);
        
    SendMessage(hListView, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    
    SendMessage(hListView, LVM_SETTEXTCOLOR, 0, (LPARAM)RGB(240, 240, 240));
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

    // Set placeholder cue banners
    SendMessageA(hSearch, EM_SETCUEBANNER, FALSE, (LPARAM)"Search / Query (e.g. dept:engineering)...");
    SendMessageA(hAddId, EM_SETCUEBANNER, FALSE, (LPARAM)"ID");
    SendMessageA(hAddName, EM_SETCUEBANNER, FALSE, (LPARAM)"Name");
    SendMessageA(hAddDept, EM_SETCUEBANNER, FALSE, (LPARAM)"Department");
    SendMessageA(hAddRole, EM_SETCUEBANNER, FALSE, (LPARAM)"Role");

    LVCOLUMNA lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.fmt = LVCFMT_LEFT;
    
    int widths[] = {70, 160, 130, 130};
    for (int i = 0; i < 4; i++) {
        lvc.iSubItem = i;
        lvc.cx = widths[i];
        lvc.pszText = (char*)headers[i];
        SendMessage(hListView, LVM_INSERTCOLUMNA, i, (LPARAM)&lvc);
    }
    
    PopulateListView("");
    AutoScaleListViewColumns(W - 35);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            InitCommonControls();
            hBgBrush = CreateSolidBrush(RGB(26, 32, 38));
            hEditBgBrush = CreateSolidBrush(RGB(35, 40, 45));
            InitListView(hwnd);
            break;
        }
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(240, 240, 240));
            SetBkColor(hdc, RGB(35, 40, 45));
            return (LRESULT)hEditBgBrush;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == IDC_SEARCH && HIWORD(wParam) == EN_CHANGE) {
                char buf[128];
                GetWindowTextA(hSearch, buf, sizeof(buf));
                PopulateListView(buf);
            } else if (LOWORD(wParam) == IDC_ADD_BTN) {
                if (data_count >= MAX_RECORDS) {
                    MessageBoxA(hwnd, "Database capacity limit reached (200 records).", "Database Full", MB_OK | MB_ICONWARNING);
                    break;
                }
                char newId[16] = {0};
                char newName[64] = {0};
                char newDept[64] = {0};
                char newRole[64] = {0};

                GetWindowTextA(hAddId, newId, sizeof(newId) - 1);
                GetWindowTextA(hAddName, newName, sizeof(newName) - 1);
                GetWindowTextA(hAddDept, newDept, sizeof(newDept) - 1);
                GetWindowTextA(hAddRole, newRole, sizeof(newRole) - 1);

                if (!*newId || !*newName || !*newDept || !*newRole) {
                    MessageBoxA(hwnd, "All fields (ID, Name, Dept, Role) are required.", "Missing Information", MB_OK | MB_ICONINFORMATION);
                    break;
                }

                // Check duplicate ID
                int dup = 0;
                for (int i = 0; i < data_count; i++) {
                    if (lstrcmpiA(data[i].id, newId) == 0) {
                        dup = 1;
                        break;
                    }
                }
                if (dup) {
                    MessageBoxA(hwnd, "An employee record with this ID already exists.", "Duplicate ID", MB_OK | MB_ICONWARNING);
                    break;
                }

                lstrcpynA(data[data_count].id, newId, sizeof(data[data_count].id));
                lstrcpynA(data[data_count].name, newName, sizeof(data[data_count].name));
                lstrcpynA(data[data_count].dept, newDept, sizeof(data[data_count].dept));
                lstrcpynA(data[data_count].role, newRole, sizeof(data[data_count].role));
                data_count++;

                SaveDataToFile();

                SetWindowTextA(hAddId, "");
                SetWindowTextA(hAddName, "");
                SetWindowTextA(hAddDept, "");
                SetWindowTextA(hAddRole, "");

                char buf[128];
                GetWindowTextA(hSearch, buf, sizeof(buf));
                PopulateListView(buf);

            } else if (LOWORD(wParam) == IDC_DEL_BTN) {
                int sel = SendMessage(hListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
                if (sel != -1) {
                    LVITEMA lvi;
                    lvi.mask = LVIF_PARAM;
                    lvi.iItem = sel;
                    lvi.iSubItem = 0;
                    if (SendMessage(hListView, LVM_GETITEMA, 0, (LPARAM)&lvi)) {
                        int idx = (int)lvi.lParam;
                        if (idx >= 0 && idx < data_count) {
                            for (int i = idx; i < data_count - 1; i++) {
                                data[i] = data[i + 1];
                            }
                            data_count--;
                            SaveDataToFile();
                            char buf[128];
                            GetWindowTextA(hSearch, buf, sizeof(buf));
                            PopulateListView(buf);
                        }
                    }
                } else {
                    MessageBoxA(hwnd, "Please select an employee record to delete.", "No Selection", MB_OK | MB_ICONINFORMATION);
                }
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            if (nw < 100 || nh < 100) break;

            MoveWindow(hSearch, 10, 10, nw - 20, 25, TRUE);
            MoveWindow(hListView, 10, 45, nw - 20, nh - 90, TRUE);
            
            int by = nh - 35;
            int avail = nw - 120;
            int ew = avail / 4;
            if (ew < 40) ew = 40;

            MoveWindow(hAddId, 10, by, ew, 25, TRUE);
            MoveWindow(hAddName, 10 + ew + 5, by, ew, 25, TRUE);
            MoveWindow(hAddDept, 10 + ew*2 + 10, by, ew, 25, TRUE);
            MoveWindow(hAddRole, 10 + ew*3 + 15, by, ew, 25, TRUE);
            MoveWindow(hAddBtn, 10 + ew*4 + 20, by, 40, 25, TRUE);
            MoveWindow(hDelBtn, 10 + ew*4 + 65, by, 40, 25, TRUE);

            AutoScaleListViewColumns(nw - 20);
            break;
        }
        case WM_DESTROY:
            if (hFont) DeleteObject(hFont);
            if (hBgBrush) DeleteObject(hBgBrush);
            if (hEditBgBrush) DeleteObject(hEditBgBrush);
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
