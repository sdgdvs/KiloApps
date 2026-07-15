#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

int my_strlen(const char* s) {
    int len = 0;
    while (*s++) len++;
    return len;
}

void my_strcpy(char* d, const char* s) {
    while (*s) *d++ = *s++;
    *d = 0;
}

char* my_strstr(const char* s1, const char* s2) {
    if (!*s2) return (char*)s1;
    for (; *s1; s1++) {
        const char* p1 = s1;
        const char* p2 = s2;
        while (*p1 && *p2 && *p1 == *p2) { p1++; p2++; }
        if (!*p2) return (char*)s1;
    }
    return NULL;
}

typedef struct {
    char name[64];
    char phone[64];
    char email[128];
    char category[64];
} Contact;

Contact contacts[100];
int contact_count = 0;

HWND hList, hEdit, hBtnNew, hBtnDel, hBtnSave;
HFONT hFont;

void LoadDemoData() {
    contact_count = 2;
    my_strcpy(contacts[0].name, "Alice Smith");
    my_strcpy(contacts[0].phone, "555-1234");
    my_strcpy(contacts[0].email, "alice@example.com");
    my_strcpy(contacts[0].category, "Work");

    my_strcpy(contacts[1].name, "Bob Jones");
    my_strcpy(contacts[1].phone, "555-5678");
    my_strcpy(contacts[1].email, "bob@example.com");
    my_strcpy(contacts[1].category, "Personal");
}

void RefreshList() {
    SendMessageA(hList, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < contact_count; i++) {
        SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)contacts[i].name);
    }
}

void extract_field(char* text, const char* prefix, char* out, int out_len) {
    char* p = my_strstr(text, prefix);
    if (p) {
        p += my_strlen(prefix);
        int i = 0;
        while (*p && *p != '\r' && *p != '\n' && i < out_len - 1) {
            out[i++] = *p++;
        }
        out[i] = 0;
    } else {
        out[0] = 0;
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            LoadDemoData();
            hList = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 10, 10, 150, 200, hwnd, (HMENU)1001, NULL, NULL);
            hBtnNew = CreateWindowExA(0, "BUTTON", "New", WS_CHILD | WS_VISIBLE, 10, 220, 70, 30, hwnd, (HMENU)1002, NULL, NULL);
            hBtnDel = CreateWindowExA(0, "BUTTON", "Del", WS_CHILD | WS_VISIBLE, 90, 220, 70, 30, hwnd, (HMENU)1003, NULL, NULL);

            hEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL, 170, 10, 200, 200, hwnd, NULL, NULL, NULL);
            hBtnSave = CreateWindowExA(0, "BUTTON", "Save Details", WS_CHILD | WS_VISIBLE, 170, 220, 200, 30, hwnd, (HMENU)1004, NULL, NULL);
            
            hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Tahoma");
            SendMessageA(hList, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnNew, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnDel, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnSave, WM_SETFONT, (WPARAM)hFont, TRUE);

            RefreshList();
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1001 && HIWORD(wParam) == LBN_SELCHANGE) {
                int idx = SendMessageA(hList, LB_GETCURSEL, 0, 0);
                if (idx >= 0 && idx < contact_count) {
                    char buf[512];
                    wsprintfA(buf, "Name: %s\r\nPhone: %s\r\nEmail: %s\r\nCategory: %s", 
                        contacts[idx].name, contacts[idx].phone, contacts[idx].email, contacts[idx].category);
                    SetWindowTextA(hEdit, buf);
                }
            }
            else if (LOWORD(wParam) == 1002) { // New
                if (contact_count < 100) {
                    my_strcpy(contacts[contact_count].name, "New Contact");
                    my_strcpy(contacts[contact_count].phone, "");
                    my_strcpy(contacts[contact_count].email, "");
                    my_strcpy(contacts[contact_count].category, "Other");
                    contact_count++;
                    RefreshList();
                    SendMessageA(hList, LB_SETCURSEL, contact_count - 1, 0);
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(1001, LBN_SELCHANGE), (LPARAM)hList);
                }
            }
            else if (LOWORD(wParam) == 1003) { // Del
                int idx = SendMessageA(hList, LB_GETCURSEL, 0, 0);
                if (idx >= 0 && idx < contact_count) {
                    for (int i = idx; i < contact_count - 1; i++) {
                        contacts[i] = contacts[i+1];
                    }
                    contact_count--;
                    RefreshList();
                    SetWindowTextA(hEdit, "");
                }
            }
            else if (LOWORD(wParam) == 1004) { // Save Details
                int idx = SendMessageA(hList, LB_GETCURSEL, 0, 0);
                if (idx >= 0 && idx < contact_count) {
                    char buf[1024];
                    GetWindowTextA(hEdit, buf, sizeof(buf));
                    extract_field(buf, "Name: ", contacts[idx].name, sizeof(contacts[idx].name));
                    extract_field(buf, "Phone: ", contacts[idx].phone, sizeof(contacts[idx].phone));
                    extract_field(buf, "Email: ", contacts[idx].email, sizeof(contacts[idx].email));
                    extract_field(buf, "Category: ", contacts[idx].category, sizeof(contacts[idx].category));
                    RefreshList();
                    SendMessageA(hList, LB_SETCURSEL, idx, 0);
                }
            }
            break;
        }
        case WM_DESTROY:
            if (hFont) DeleteObject(hFont);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KContactsClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KContactsClass", "KContacts", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 410, 300, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
