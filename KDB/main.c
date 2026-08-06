#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>

#define W 1024
#define H 768
#define IDC_SEARCH 101
#define IDC_ADD_ID 102
#define IDC_ADD_NAME 103
#define IDC_ADD_DEPT 104
#define IDC_ADD_ROLE 105
#define IDC_ADD_BTN 106
#define IDC_DEL_BTN 107
#define IDC_PWD 108
#define IDC_EXPORT_CSV 109
#define IDC_IMPORT_CSV 110
#define IDC_EXPORT_JSON 111
#define IDC_IMPORT_JSON 112
#define IDC_RELOAD_BTN 113

HWND hListView;
HWND hSearch;
HWND hAddId, hAddName, hAddDept, hAddRole, hAddBtn, hDelBtn;
HWND hPwd, hExpCSV, hImpCSV, hExpJSON, hImpJSON, hReload;
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

#pragma function(memcpy)
void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}

int atoi(const char* str) {
    int res = 0;
    while (*str >= '0' && *str <= '9') {
        res = res * 10 + (*str - '0');
        str++;
    }
    return res;
}

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

const char* FindStr(const char* haystack, const char* needle) {
    if (!*needle) return haystack;
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && *h == *n) { h++; n++; }
        if (!*n) return haystack;
        haystack++;
    }
    return NULL;
}

void CryptData(char* buffer, int len, const char* key) {
    if (!key || !*key) return;
    int klen = lstrlenA(key);
    for (int i = 0; i < len; i++) {
        buffer[i] ^= key[i % klen];
    }
}

void SaveDataToFile() {
    char key[64] = {0};
    if (hPwd) GetWindowTextA(hPwd, key, sizeof(key));
    HANDLE hFile = CreateFileA(FILE_NAME, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        int magic = key[0] ? 0x4B444245 : 0x4B444231;
        WriteFile(hFile, &magic, sizeof(int), &written, NULL);
        
        int bufferSize = sizeof(int) + sizeof(Record) * data_count;
        char* buffer = (char*)GlobalAlloc(GMEM_FIXED, bufferSize);
        if (buffer) {
            memcpy(buffer, &data_count, sizeof(int));
            memcpy(buffer + sizeof(int), data, sizeof(Record) * data_count);
            
            if (key[0]) CryptData(buffer, bufferSize, key);
            
            WriteFile(hFile, buffer, bufferSize, &written, NULL);
            GlobalFree(buffer);
        }
        CloseHandle(hFile);
    }
}

void LoadDataFromFile() {
    char key[64] = {0};
    if (hPwd) GetWindowTextA(hPwd, key, sizeof(key));
    HANDLE hFile = CreateFileA(FILE_NAME, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD readBytes = 0;
        int magic = 0;
        if (ReadFile(hFile, &magic, sizeof(int), &readBytes, NULL) && readBytes == sizeof(int)) {
            int encrypted = (magic == 0x4B444245);
            if (encrypted && !key[0]) {
                CloseHandle(hFile);
                // Can't load without key.
                data_count = 0;
                return;
            }
            
            DWORD size = GetFileSize(hFile, NULL) - sizeof(int);
            if (size > 0 && size < 1000000) {
                char* buffer = (char*)GlobalAlloc(GMEM_FIXED, size);
                if (buffer) {
                    if (ReadFile(hFile, buffer, size, &readBytes, NULL) && readBytes == size) {
                        if (encrypted) CryptData(buffer, size, key);
                        int count = 0;
                        memcpy(&count, buffer, sizeof(int));
                        if (count >= 0 && count <= MAX_RECORDS) {
                            memcpy(data, buffer + sizeof(int), count * sizeof(Record));
                            data_count = count;
                        } else {
                            data_count = 0; // Failed decryption
                        }
                    }
                    GlobalFree(buffer);
                }
            }
        }
        CloseHandle(hFile);
    } else {
        data_count = 5;
    }
}

// Bounded Query Matcher supporting multiple terms
int MatchQuery(const Record* rec, const char* query) {
    if (!query || !*query) return 1;
    char buf[256];
    lstrcpynA(buf, query, sizeof(buf));
    
    char* p = buf;
    while (*p) {
        while (*p == ' ') p++;
        if (!*p) break;
        char* end = p;
        while (*end && *end != ' ') end++;
        if (*end) {
            *end = '\0';
            end++;
        }
        
        char* term = p;
        p = end; // next
        
        int match = 0;
        char* colon = (char*)FindChar(term, ':');
        if (colon) {
            *colon = '\0';
            char* field = term;
            char* val = colon + 1;
            if (lstrcmpiA(field, "dept") == 0 || lstrcmpiA(field, "department") == 0) {
                match = StrContainsI(rec->dept, val);
            } else if (lstrcmpiA(field, "role") == 0) {
                match = StrContainsI(rec->role, val);
            } else if (lstrcmpiA(field, "id") == 0) {
                match = StrContainsI(rec->id, val);
            } else if (lstrcmpiA(field, "name") == 0) {
                match = StrContainsI(rec->name, val);
            }
        } else if (term[0] == 'i' && term[1] == 'd' && term[2] == '>') {
            int num = atoi(term + 3);
            match = (atoi(rec->id) > num);
        } else if (term[0] == 'i' && term[1] == 'd' && term[2] == '<') {
            int num = atoi(term + 3);
            match = (atoi(rec->id) < num);
        } else {
            match = StrContainsI(rec->id, term) ||
                    StrContainsI(rec->name, term) ||
                    StrContainsI(rec->dept, term) ||
                    StrContainsI(rec->role, term);
        }
        if (!match) return 0;
    }
    return 1;
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

int PromptFile(HWND hwnd, char* outPath, int isSave, const char* filter, const char* ext) {
    OPENFILENAMEA ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = outPath;
    outPath[0] = '\0';
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = ext;
    ofn.Flags = OFN_PATHMUSTEXIST | (isSave ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);
    if (isSave) return GetSaveFileNameA(&ofn);
    return GetOpenFileNameA(&ofn);
}

void ExportCSV(HWND hwnd) {
    char path[MAX_PATH];
    if (PromptFile(hwnd, path, 1, "CSV Files\0*.csv\0All Files\0*.*\0", "csv")) {
        HANDLE hFile = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD w;
            const char* header = "ID,Name,Department,Role\r\n";
            WriteFile(hFile, header, lstrlenA(header), &w, NULL);
            char buf[512];
            for (int i = 0; i < data_count; i++) {
                wsprintfA(buf, "\"%s\",\"%s\",\"%s\",\"%s\"\r\n", data[i].id, data[i].name, data[i].dept, data[i].role);
                WriteFile(hFile, buf, lstrlenA(buf), &w, NULL);
            }
            CloseHandle(hFile);
            MessageBoxA(hwnd, "Exported successfully", "Success", MB_OK);
        }
    }
}

void ImportCSV(HWND hwnd) {
    char path[MAX_PATH];
    if (PromptFile(hwnd, path, 0, "CSV Files\0*.csv\0All Files\0*.*\0", "csv")) {
        HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD size = GetFileSize(hFile, NULL);
            char* buf = (char*)GlobalAlloc(GMEM_FIXED, size + 1);
            if (buf) {
                DWORD r;
                ReadFile(hFile, buf, size, &r, NULL);
                buf[size] = 0;
                
                char* p = buf;
                int line = 0;
                while (*p && data_count < MAX_RECORDS) {
                    char* next = p;
                    while (*next && *next != '\n') next++;
                    if (*next) { *next = 0; next++; }
                    if (line > 0 && lstrlenA(p) > 5) {
                        char* id = p;
                        char* name = (char*)FindChar(id, ','); if (name) { *name = 0; name++; }
                        char* dept = name ? (char*)FindChar(name, ',') : NULL; if (dept) { *dept = 0; dept++; }
                        char* role = dept ? (char*)FindChar(dept, ',') : NULL; if (role) { *role = 0; role++; }
                        if (id && name && dept && role) {
                            if (id[0] == '"') { id++; id[lstrlenA(id)-1] = 0; }
                            if (name[0] == '"') { name++; name[lstrlenA(name)-1] = 0; }
                            if (dept[0] == '"') { dept++; dept[lstrlenA(dept)-1] = 0; }
                            if (role[0] == '"') { role++; role[lstrlenA(role)-1] = 0; }
                            char* ret = (char*)FindChar(role, '\r'); if (ret) *ret = 0;
                            
                            int dup = 0;
                            for (int i = 0; i < data_count; i++) { if (lstrcmpiA(data[i].id, id) == 0) dup = 1; }
                            if (!dup) {
                                lstrcpynA(data[data_count].id, id, 16);
                                lstrcpynA(data[data_count].name, name, 64);
                                lstrcpynA(data[data_count].dept, dept, 64);
                                lstrcpynA(data[data_count].role, role, 64);
                                data_count++;
                            }
                        }
                    }
                    p = next;
                    line++;
                }
                GlobalFree(buf);
            }
            CloseHandle(hFile);
            SaveDataToFile();
            char msg[128]; wsprintfA(msg, "Imported CSV. Total: %d", data_count);
            MessageBoxA(hwnd, msg, "Success", MB_OK);
        }
    }
}

void ExportJSON(HWND hwnd) {
    char path[MAX_PATH];
    if (PromptFile(hwnd, path, 1, "JSON Files\0*.json\0All Files\0*.*\0", "json")) {
        HANDLE hFile = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD w;
            WriteFile(hFile, "[\r\n", 3, &w, NULL);
            char buf[512];
            for (int i = 0; i < data_count; i++) {
                wsprintfA(buf, "  {\"id\":\"%s\", \"name\":\"%s\", \"dept\":\"%s\", \"role\":\"%s\"}%s\r\n", 
                    data[i].id, data[i].name, data[i].dept, data[i].role, (i == data_count - 1) ? "" : ",");
                WriteFile(hFile, buf, lstrlenA(buf), &w, NULL);
            }
            WriteFile(hFile, "]\r\n", 3, &w, NULL);
            CloseHandle(hFile);
            MessageBoxA(hwnd, "Exported successfully", "Success", MB_OK);
        }
    }
}

void GetJsonString(const char* objStart, const char* objEnd, const char* key, char* out, int outLen) {
    char search[32]; wsprintfA(search, "\"%s\"", key);
    const char* p = FindStr(objStart, search);
    if (p && p < objEnd) {
        p += lstrlenA(search);
        while (p < objEnd && (*p == ' ' || *p == ':')) p++;
        if (p < objEnd && *p == '"') {
            p++;
            int i = 0;
            while (p < objEnd && *p != '"' && i < outLen - 1) {
                out[i++] = *p++;
            }
            out[i] = 0;
        }
    }
}

void ImportJSON(HWND hwnd) {
    char path[MAX_PATH];
    if (PromptFile(hwnd, path, 0, "JSON Files\0*.json\0All Files\0*.*\0", "json")) {
        HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD size = GetFileSize(hFile, NULL);
            char* buf = (char*)GlobalAlloc(GMEM_FIXED, size + 1);
            if (buf) {
                DWORD r; ReadFile(hFile, buf, size, &r, NULL); buf[size] = 0;
                const char* p = buf;
                while (*p && data_count < MAX_RECORDS) {
                    p = FindChar(p, '{'); if (!p) break;
                    const char* end = FindChar(p, '}'); if (!end) break;
                    char id[64]={0}, name[64]={0}, dept[64]={0}, role[64]={0};
                    GetJsonString(p, end, "id", id, 16); GetJsonString(p, end, "name", name, 64);
                    GetJsonString(p, end, "dept", dept, 64); GetJsonString(p, end, "role", role, 64);
                    if (id[0] && name[0]) {
                        int dup = 0; for (int i = 0; i < data_count; i++) if (lstrcmpiA(data[i].id, id) == 0) dup = 1;
                        if (!dup) {
                            lstrcpynA(data[data_count].id, id, 16); lstrcpynA(data[data_count].name, name, 64);
                            lstrcpynA(data[data_count].dept, dept, 64); lstrcpynA(data[data_count].role, role, 64);
                            data_count++;
                        }
                    }
                    p = end + 1;
                }
                GlobalFree(buf);
            }
            CloseHandle(hFile);
            SaveDataToFile();
            char msg[128]; wsprintfA(msg, "Imported JSON. Total: %d", data_count);
            MessageBoxA(hwnd, msg, "Success", MB_OK);
        }
    }
}

void InitListView(HWND hwnd) {
    hPwd = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD, 10, 10, 100, 25, hwnd, (HMENU)IDC_PWD, GetModuleHandle(NULL), NULL);
    hReload = CreateWindowEx(0, "BUTTON", "Load", WS_CHILD | WS_VISIBLE, 115, 10, 45, 25, hwnd, (HMENU)IDC_RELOAD_BTN, GetModuleHandle(NULL), NULL);
    hSearch = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 165, 10, 180, 25, hwnd, (HMENU)IDC_SEARCH, GetModuleHandle(NULL), NULL);
    
    hExpCSV = CreateWindowEx(0, "BUTTON", "ExCSV", WS_CHILD | WS_VISIBLE, 350, 10, 50, 25, hwnd, (HMENU)IDC_EXPORT_CSV, GetModuleHandle(NULL), NULL);
    hImpCSV = CreateWindowEx(0, "BUTTON", "ImCSV", WS_CHILD | WS_VISIBLE, 405, 10, 50, 25, hwnd, (HMENU)IDC_IMPORT_CSV, GetModuleHandle(NULL), NULL);
    hExpJSON = CreateWindowEx(0, "BUTTON", "ExJSON", WS_CHILD | WS_VISIBLE, 460, 10, 55, 25, hwnd, (HMENU)IDC_EXPORT_JSON, GetModuleHandle(NULL), NULL);
    hImpJSON = CreateWindowEx(0, "BUTTON", "ImJSON", WS_CHILD | WS_VISIBLE, 520, 10, 55, 25, hwnd, (HMENU)IDC_IMPORT_JSON, GetModuleHandle(NULL), NULL);
    
    LoadDataFromFile();

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
    
    hFont = CreateFontA(18, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
    SendMessage(hListView, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hSearch, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hPwd, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hReload, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hExpCSV, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hImpCSV, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hExpJSON, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hImpJSON, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    SendMessage(hAddId, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hAddName, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hAddDept, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hAddRole, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hAddBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hDelBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

    // Set placeholder cue banners
    SendMessageA(hPwd, EM_SETCUEBANNER, FALSE, (LPARAM)"Encryption Key");
    SendMessageA(hSearch, EM_SETCUEBANNER, FALSE, (LPARAM)"Search / Query (Press H for Help)...");
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
            } else if (LOWORD(wParam) == IDC_RELOAD_BTN) {
                LoadDataFromFile();
                char buf[128]; GetWindowTextA(hSearch, buf, sizeof(buf));
                PopulateListView(buf);
            } else if (LOWORD(wParam) == IDC_EXPORT_CSV) {
                ExportCSV(hwnd);
            } else if (LOWORD(wParam) == IDC_IMPORT_CSV) {
                ImportCSV(hwnd);
                char buf[128]; GetWindowTextA(hSearch, buf, sizeof(buf));
                PopulateListView(buf);
            } else if (LOWORD(wParam) == IDC_EXPORT_JSON) {
                ExportJSON(hwnd);
            } else if (LOWORD(wParam) == IDC_IMPORT_JSON) {
                ImportJSON(hwnd);
                char buf[128]; GetWindowTextA(hSearch, buf, sizeof(buf));
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

            MoveWindow(hPwd, 10, 10, 110, 25, TRUE);
            MoveWindow(hReload, 125, 10, 50, 25, TRUE);
            
            int sh = nw - 470;
            if (sh < 50) sh = 50;
            MoveWindow(hSearch, 185, 10, sh, 25, TRUE);
            
            int rx = 185 + sh + 10;
            MoveWindow(hExpCSV, rx, 10, 60, 25, TRUE);
            MoveWindow(hImpCSV, rx + 65, 10, 60, 25, TRUE);
            MoveWindow(hExpJSON, rx + 130, 10, 65, 25, TRUE);
            MoveWindow(hImpJSON, rx + 200, 10, 65, 25, TRUE);

            MoveWindow(hListView, 10, 45, nw - 20, nh - 90, TRUE);
            
            int by = nh - 35;
            int avail = nw - 140;
            int ew = avail / 4;
            if (ew < 40) ew = 40;

            MoveWindow(hAddId, 10, by, ew, 25, TRUE);
            MoveWindow(hAddName, 10 + ew + 5, by, ew, 25, TRUE);
            MoveWindow(hAddDept, 10 + ew*2 + 10, by, ew, 25, TRUE);
            MoveWindow(hAddRole, 10 + ew*3 + 15, by, ew, 25, TRUE);
            MoveWindow(hAddBtn, 10 + ew*4 + 20, by, 50, 25, TRUE);
            MoveWindow(hDelBtn, 10 + ew*4 + 75, by, 50, 25, TRUE);

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

    HWND hwnd = CreateWindowEx(0, "KDBApp", "KDB - Employee Database (Press H for Help)", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, W, H, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN && msg.wParam == 'H') {
            char cls[64] = {0};
            GetClassNameA(msg.hwnd, cls, sizeof(cls));
            if (lstrcmpiA(cls, "EDIT") != 0) {
                MessageBoxA(hwnd, "KDB Help\n\n- Search supports tags (e.g. 'dept:engineering' or 'role:lead')\n- Conditions (e.g. 'id>102' or 'id<105')\n- Data is auto-saved locally.\n- Set a password to encrypt/decrypt database payloads.\n- Max table capacity is 200 records.\n- Export and import via CSV or JSON.", "KDB Help", MB_OK | MB_ICONINFORMATION);
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
