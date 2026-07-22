#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>

#define W 440
#define H 320

HWND hEdit, hList, hBtnNew, hBtnDel, hStatus, hSearch, hBtnPin, hBtnExport;
HBRUSH bgBrush, sidebarBrush;
HFONT hFont;

#define ID_FILE_OPEN 9001
#define ID_FILE_SAVE 9002
#define ID_FILE_EXIT 9003
#define ID_FILE_EXPORT 9004
#define ID_BTN_NEW 9005
#define ID_BTN_DEL 9006
#define ID_LIST 9007
#define ID_SEARCH 9008
#define ID_BTN_PIN 9009
#define ID_BTN_EXPORT 9010
#define ID_STATUS 9011

char notes[100][1024] = {0};
int pinned[100] = {0};
int displayToReal[100] = {0};
int displayCount = 0;
int numNotes = 0;
int activeNote = -1;

int StrStrI(const char* haystack, const char* needle) {
    if (!needle || !*needle) return 1;
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
                if (l > 0) {
                    if (p[0] == '1' || p[0] == '0') {
                        pinned[numNotes] = (p[0] == '1') ? 1 : 0;
                        p++; l--;
                    } else {
                        pinned[numNotes] = 0;
                    }
                    if (l > 1023) l = 1023;
                    for(int i=0; i<l; i++) notes[numNotes][i] = p[i];
                    notes[numNotes][l] = 0;
                    numNotes++;
                }
                p = next + 1;
            }
            HeapFree(GetProcessHeap(), 0, buf);
        }
        CloseHandle(hFile);
    }
    if (numNotes == 0) {
        numNotes = 1;
        pinned[0] = 0;
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
            char pinFlag = pinned[i] ? '1' : '0';
            WriteFile(hFile, &pinFlag, 1, &bw, NULL);
            int l=0; while(notes[i][l]) l++;
            WriteFile(hFile, notes[i], l, &bw, NULL);
            char delim = '\1';
            WriteFile(hFile, &delim, 1, &bw, NULL);
        }
        CloseHandle(hFile);
    }
}

void ExportNote() {
    if (activeNote < 0) return;
    if (activeNote >= 0) GetWindowTextA(hEdit, notes[activeNote], 1024);
    
    OPENFILENAMEA ofn;
    char szFile[260] = "note.txt";
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hEdit;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    char filename[260] = "knote_export.txt";
    if (GetSaveFileNameA(&ofn)) {
        lstrcpyA(filename, ofn.lpstrFile);
    }
    HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bw;
        int len = 0;
        while (notes[activeNote][len]) len++;
        WriteFile(hFile, notes[activeNote], len, &bw, NULL);
        CloseHandle(hFile);
    }
}

void UpdateStats() {
    int len = GetWindowTextLengthA(hEdit);
    char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, len + 1);
    if (buf) {
        GetWindowTextA(hEdit, buf, len + 1);
        int words = 0, chars = len;
        int inWord = 0;
        for (int i = 0; i < len; i++) {
            if (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\r' || buf[i] == '\t') {
                inWord = 0;
            } else if (!inWord) {
                inWord = 1;
                words++;
            }
        }
        HeapFree(GetProcessHeap(), 0, buf);
        char stat[64];
        wsprintfA(stat, "  Words: %d | Chars: %d", words, chars);
        SetWindowTextA(hStatus, stat);
    }
}

void RefreshList() {
    char query[64] = {0};
    if (hSearch) GetWindowTextA(hSearch, query, 64);
    
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    displayCount = 0;
    
    // Add pinned matching notes first, then unpinned matching notes
    for (int pPass = 1; pPass >= 0; pPass--) {
        for (int i = 0; i < numNotes; i++) {
            if ((pinned[i] ? 1 : 0) != pPass) continue;
            if (query[0] != 0 && !StrStrI(notes[i], query)) continue;
            
            displayToReal[displayCount] = i;
            
            char title[40] = {0};
            int j = 0;
            int offset = 0;
            if (pinned[i]) {
                title[0] = '['; title[1] = 'P'; title[2] = ']'; title[3] = ' ';
                offset = 4;
            }
            while (notes[i][j] && notes[i][j] != '\r' && notes[i][j] != '\n' && j < 25) {
                title[offset + j] = notes[i][j];
                j++;
            }
            if (j == 0) {
                title[offset] = 'E'; title[offset+1] = 'm'; title[offset+2] = 'p'; title[offset+3] = 't'; title[offset+4] = 'y';
            }
            
            SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)title);
            if (i == activeNote) {
                SendMessage(hList, LB_SETCURSEL, displayCount, 0);
            }
            displayCount++;
        }
    }
    
    if (activeNote >= 0) {
        SetWindowTextA(hEdit, notes[activeNote]);
        UpdateStats();
        if (hBtnPin) {
            SetWindowTextA(hBtnPin, pinned[activeNote] ? "Unpin" : "Pin");
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HMENU hMenu = CreateMenu();
            HMENU hFileMenu = CreatePopupMenu();
            AppendMenuA(hFileMenu, MF_STRING, ID_FILE_OPEN, "Open");
            AppendMenuA(hFileMenu, MF_STRING, ID_FILE_SAVE, "Save");
            AppendMenuA(hFileMenu, MF_STRING, ID_FILE_EXPORT, "Export TXT...");
            AppendMenuA(hFileMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hFileMenu, MF_STRING, ID_FILE_EXIT, "Exit");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, "File");
            SetMenu(hwnd, hMenu);

            bgBrush = CreateSolidBrush(RGB(255, 255, 150));
            sidebarBrush = CreateSolidBrush(RGB(224, 224, 160));
            
            hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Comic Sans MS");
            if (!hFont) hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            hBtnNew = CreateWindow("BUTTON", "New", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 60, 26, hwnd, (HMENU)ID_BTN_NEW, NULL, NULL);
            hBtnDel = CreateWindow("BUTTON", "Del", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 60, 0, 60, 26, hwnd, (HMENU)ID_BTN_DEL, NULL, NULL);
            
            hSearch = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 26, 120, 22, hwnd, (HMENU)ID_SEARCH, NULL, NULL);
            SendMessageA(hSearch, EM_SETCUEBANNER, FALSE, (LPARAM)L"Search...");

            hList = CreateWindowEx(0, "LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 0, 48, 120, H-48, hwnd, (HMENU)ID_LIST, NULL, NULL);
            
            hBtnPin = CreateWindow("BUTTON", "Pin", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 120, 0, 60, 26, hwnd, (HMENU)ID_BTN_PIN, NULL, NULL);
            hBtnExport = CreateWindow("BUTTON", "Export TXT", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 180, 0, 75, 26, hwnd, (HMENU)ID_BTN_EXPORT, NULL, NULL);

            hEdit = CreateWindowEx(0, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL,
                120, 26, W-120, H-46, hwnd, NULL, NULL, NULL);
                
            hStatus = CreateWindowEx(0, "STATIC", "  Words: 0 | Chars: 0",
                WS_CHILD | WS_VISIBLE,
                120, H-20, W-120, 20, hwnd, (HMENU)ID_STATUS, NULL, NULL);
                
            SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            HFONT hSys = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            SendMessage(hList, WM_SETFONT, (WPARAM)hSys, TRUE);
            SendMessage(hBtnNew, WM_SETFONT, (WPARAM)hSys, TRUE);
            SendMessage(hBtnDel, WM_SETFONT, (WPARAM)hSys, TRUE);
            SendMessage(hBtnPin, WM_SETFONT, (WPARAM)hSys, TRUE);
            SendMessage(hBtnExport, WM_SETFONT, (WPARAM)hSys, TRUE);
            SendMessage(hSearch, WM_SETFONT, (WPARAM)hSys, TRUE);
            SendMessage(hStatus, WM_SETFONT, (WPARAM)hSys, TRUE);
            
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
            } else if (LOWORD(wParam) == ID_FILE_EXPORT || LOWORD(wParam) == ID_BTN_EXPORT) {
                ExportNote();
            } else if (LOWORD(wParam) == ID_FILE_EXIT) {
                PostMessageA(hwnd, WM_CLOSE, 0, 0);
            } else if (LOWORD(wParam) == ID_BTN_NEW) {
                if (numNotes < 100) {
                    if (activeNote >= 0) GetWindowTextA(hEdit, notes[activeNote], 1024);
                    notes[numNotes][0] = 0;
                    pinned[numNotes] = 0;
                    activeNote = numNotes;
                    numNotes++;
                    RefreshList();
                }
            } else if (LOWORD(wParam) == ID_BTN_DEL) {
                if (numNotes > 1) {
                    for(int i=activeNote; i<numNotes-1; i++) {
                        for(int j=0; j<1024; j++) notes[i][j] = notes[i+1][j];
                        pinned[i] = pinned[i+1];
                    }
                    numNotes--;
                    if (activeNote >= numNotes) activeNote = numNotes - 1;
                    RefreshList();
                } else {
                    notes[0][0] = 0;
                    pinned[0] = 0;
                    RefreshList();
                }
            } else if (LOWORD(wParam) == ID_BTN_PIN) {
                if (activeNote >= 0) {
                    pinned[activeNote] = !pinned[activeNote];
                    RefreshList();
                }
            } else if (LOWORD(wParam) == ID_LIST && HIWORD(wParam) == LBN_SELCHANGE) {
                if (activeNote >= 0) GetWindowTextA(hEdit, notes[activeNote], 1024);
                int sel = SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR && sel < displayCount) {
                    activeNote = displayToReal[sel];
                    SetWindowTextA(hEdit, notes[activeNote]);
                    UpdateStats();
                    if (hBtnPin) {
                        SetWindowTextA(hBtnPin, pinned[activeNote] ? "Unpin" : "Pin");
                    }
                }
            }
            else if ((HWND)lParam == hSearch && HIWORD(wParam) == EN_CHANGE) {
                RefreshList();
            }
            else if ((HWND)lParam == hEdit && HIWORD(wParam) == EN_CHANGE) {
                UpdateStats();
            }
            break;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(255, 255, 150));
            return (LRESULT)bgBrush;
        }
        case WM_CTLCOLORSTATIC: {
            if ((HWND)lParam == hStatus) {
                HDC hdc = (HDC)wParam;
                SetBkColor(hdc, RGB(224, 224, 160));
                return (LRESULT)sidebarBrush;
            }
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        case WM_CTLCOLORLISTBOX: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(224, 224, 160));
            return (LRESULT)sidebarBrush;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            int sideW = 120;
            int topH = 26;

            MoveWindow(hBtnNew, 0, 0, 60, topH, TRUE);
            MoveWindow(hBtnDel, 60, 0, 60, topH, TRUE);
            MoveWindow(hSearch, 0, topH, sideW, 22, TRUE);
            MoveWindow(hList, 0, topH + 22, sideW, nh - (topH + 22), TRUE);

            MoveWindow(hBtnPin, sideW, 0, 60, topH, TRUE);
            MoveWindow(hBtnExport, sideW + 60, 0, 75, topH, TRUE);
            MoveWindow(hEdit, sideW, topH, nw - sideW, nh - topH - 20, TRUE);
            MoveWindow(hStatus, sideW, nh - 20, nw - sideW, 20, TRUE);
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

