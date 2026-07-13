#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 400
#define H 300

HWND hEdit, hList, hBtnNew, hBtnDel;
HBRUSH bgBrush, sidebarBrush;
HFONT hFont;

#define ID_FILE_OPEN 9001
#define ID_FILE_SAVE 9002
#define ID_FILE_EXIT 9003
#define ID_BTN_NEW 9004
#define ID_BTN_DEL 9005
#define ID_LIST 9006

char notes[100][1024] = {0};
int numNotes = 0;
int activeNote = -1;

void LoadNotes() {
    HANDLE hFile = CreateFileA("knote_data.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD fileSize = GetFileSize(hFile, NULL);
        char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileSize + 1);
        if (buf) {
            DWORD bytesRead;
            ReadFile(hFile, buf, fileSize, &bytesRead, NULL);
            buf[fileSize] = 0;
            
            numNotes = 0;
            char* p = buf;
            while(p < buf + fileSize && numNotes < 100) {
                char* next = p;
                while(*next && *next != '\1') next++;
                *next = 0;
                int l = next - p;
                if (l > 1023) l = 1023;
                for(int i=0; i<l; i++) notes[numNotes][i] = p[i];
                notes[numNotes][l] = 0;
                numNotes++;
                p = next + 1;
            }
            HeapFree(GetProcessHeap(), 0, buf);
        }
        CloseHandle(hFile);
    }
    if (numNotes == 0) {
        numNotes = 1;
        const char* def = "Buy milk\r\nFix that bug\r\nCall mom";
        for(int i=0; def[i]; i++) notes[0][i] = def[i];
    }
}

void SaveNotes() {
    if (activeNote >= 0) {
        GetWindowTextA(hEdit, notes[activeNote], 1024);
    }
    HANDLE hFile = CreateFileA("knote_data.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bw;
        for(int i=0; i<numNotes; i++) {
            int l=0; while(notes[i][l]) l++;
            WriteFile(hFile, notes[i], l, &bw, NULL);
            char delim = '\1';
            WriteFile(hFile, &delim, 1, &bw, NULL);
        }
        CloseHandle(hFile);
    }
}

void RefreshList() {
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    for(int i=0; i<numNotes; i++) {
        char title[32] = {0};
        int j=0;
        while(notes[i][j] && notes[i][j] != '\r' && notes[i][j] != '\n' && j < 30) {
            title[j] = notes[i][j];
            j++;
        }
        if (j==0) { title[0] = 'E'; title[1] = 'm'; title[2] = 'p'; title[3] = 't'; title[4] = 'y'; }
        SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)title);
    }
    if (activeNote >= 0) {
        SendMessage(hList, LB_SETCURSEL, activeNote, 0);
        SetWindowTextA(hEdit, notes[activeNote]);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HMENU hMenu = CreateMenu();
            HMENU hFileMenu = CreatePopupMenu();
            AppendMenuA(hFileMenu, MF_STRING, ID_FILE_OPEN, "Open");
            AppendMenuA(hFileMenu, MF_STRING, ID_FILE_SAVE, "Save");
            AppendMenuA(hFileMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hFileMenu, MF_STRING, ID_FILE_EXIT, "Exit");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, "File");
            SetMenu(hwnd, hMenu);

            bgBrush = CreateSolidBrush(RGB(255, 255, 150));
            sidebarBrush = CreateSolidBrush(RGB(224, 224, 160));
            
            hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Comic Sans MS");
            if (!hFont) hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            hList = CreateWindowEx(0, "LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 0, 30, 100, H-30, hwnd, (HMENU)ID_LIST, NULL, NULL);
            hBtnNew = CreateWindow("BUTTON", "New", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 50, 30, hwnd, (HMENU)ID_BTN_NEW, NULL, NULL);
            hBtnDel = CreateWindow("BUTTON", "Del", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 50, 0, 50, 30, hwnd, (HMENU)ID_BTN_DEL, NULL, NULL);
            
            hEdit = CreateWindowEx(0, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL,
                100, 0, W-100, H, hwnd, NULL, NULL, NULL);
                
            SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            HFONT hSys = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            SendMessage(hList, WM_SETFONT, (WPARAM)hSys, TRUE);
            SendMessage(hBtnNew, WM_SETFONT, (WPARAM)hSys, TRUE);
            SendMessage(hBtnDel, WM_SETFONT, (WPARAM)hSys, TRUE);
            
            LoadNotes();
            activeNote = 0;
            RefreshList();
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_FILE_OPEN) {
                LoadNotes();
                activeNote = 0;
                RefreshList();
            } else if (LOWORD(wParam) == ID_FILE_SAVE) {
                SaveNotes();
            } else if (LOWORD(wParam) == ID_FILE_EXIT) {
                PostMessageA(hwnd, WM_CLOSE, 0, 0);
            } else if (LOWORD(wParam) == ID_BTN_NEW) {
                if (numNotes < 100) {
                    if (activeNote >= 0) GetWindowTextA(hEdit, notes[activeNote], 1024);
                    notes[numNotes][0] = 0;
                    activeNote = numNotes;
                    numNotes++;
                    RefreshList();
                }
            } else if (LOWORD(wParam) == ID_BTN_DEL) {
                if (numNotes > 1) {
                    for(int i=activeNote; i<numNotes-1; i++) {
                        for(int j=0; j<1024; j++) notes[i][j] = notes[i+1][j];
                    }
                    numNotes--;
                    if (activeNote >= numNotes) activeNote = numNotes - 1;
                    RefreshList();
                } else {
                    notes[0][0] = 0;
                    RefreshList();
                }
            } else if (LOWORD(wParam) == ID_LIST && HIWORD(wParam) == LBN_SELCHANGE) {
                if (activeNote >= 0) GetWindowTextA(hEdit, notes[activeNote], 1024);
                int sel = SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    activeNote = sel;
                    SetWindowTextA(hEdit, notes[activeNote]);
                }
            }
            break;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(255, 255, 150));
            return (LRESULT)bgBrush;
        }
        case WM_CTLCOLORLISTBOX: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(224, 224, 160));
            return (LRESULT)sidebarBrush;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            MoveWindow(hBtnNew, 0, 0, 50, 30, TRUE);
            MoveWindow(hBtnDel, 50, 0, 50, 30, TRUE);
            MoveWindow(hList, 0, 30, 100, nh-30, TRUE);
            MoveWindow(hEdit, 100, 0, nw-100, nh, TRUE);
            break;
        }
        case WM_DESTROY:
            SaveNotes();
            DeleteObject(bgBrush);
            DeleteObject(sidebarBrush);
            if (hFont) DeleteObject(hFont);
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
    wc.lpszClassName = "KNoteApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 150));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, "KNoteApp", "KNote", WS_OVERLAPPEDWINDOW,
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
