#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#define W 500
#define H 400

HWND hListView;

const char* headers[] = {"ID", "Name", "Department", "Role"};
const char* data[][4] = {
    {"101", "Alice Smith", "Engineering", "Developer"},
    {"102", "Bob Johnson", "Marketing", "Designer"},
    {"103", "Charlie Davis", "Sales", "Executive"},
    {"104", "Diana Prince", "Engineering", "Lead"},
    {"105", "Evan Wright", "HR", "Manager"}
};

void InitListView(HWND hwnd) {
    hListView = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EDITLABELS,
        10, 10, W - 35, H - 60, hwnd, NULL, GetModuleHandle(NULL), NULL);
        
    SendMessage(hListView, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    
    HFONT hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
    SendMessage(hListView, WM_SETFONT, (WPARAM)hFont, TRUE);

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
    
    LVITEMA lvi;
    lvi.mask = LVIF_TEXT;
    
    for (int i = 0; i < 5; i++) {
        lvi.iItem = i;
        lvi.iSubItem = 0;
        lvi.pszText = (char*)data[i][0];
        SendMessage(hListView, LVM_INSERTITEMA, 0, (LPARAM)&lvi);
        
        for (int j = 1; j < 4; j++) {
            lvi.iSubItem = j;
            lvi.pszText = (char*)data[i][j];
            SendMessage(hListView, LVM_SETITEMTEXTA, i, (LPARAM)&lvi);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            InitCommonControls();
            InitListView(hwnd);
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            MoveWindow(hListView, 10, 10, nw - 20, nh - 20, TRUE);
            break;
        }
        case WM_DESTROY:
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
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
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
