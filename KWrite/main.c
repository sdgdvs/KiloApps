#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <commdlg.h>

#define IDC_TAB 101
#define IDC_RICHEDIT 102
#define IDM_FILENEWTAB 1000
#define IDM_FILESAVE 1001
#define IDM_FILEEXIT 1002

#define MAX_TABS 10

HWND hTab, hEdit;

char* docs[MAX_TABS];
int tabCount = 1;
int currentTab = 0;

#pragma function(memset)
void* __cdecl memset(void* dest, int c, unsigned int count) {
    char* bytes = (char*)dest;
    while (count--) {
        *bytes++ = (char)c;
    }
    return dest;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == IDM_FILENEWTAB) {
                if (tabCount < MAX_TABS) {
                    docs[tabCount] = (char*)GlobalAlloc(GPTR, 65536);
                    lstrcpyA(docs[tabCount], "");
                    
                    char tabName[32];
                    wsprintfA(tabName, "Document %d", tabCount + 1);
                    TCITEMA tie;
                    tie.mask = TCIF_TEXT;
                    tie.pszText = tabName;
                    SendMessage(hTab, TCM_INSERTITEMA, tabCount, (LPARAM)&tie);
                    
                    // Switch to new tab
                    GetWindowTextA(hEdit, docs[currentTab], 65536);
                    
                    currentTab = tabCount;
                    SendMessage(hTab, TCM_SETCURSEL, currentTab, 0);
                    SetWindowTextA(hEdit, docs[currentTab]);
                    
                    tabCount++;
                }
            } else if (LOWORD(wParam) == IDM_FILESAVE) {
                OPENFILENAMEA ofn = {0};
                char szFile[260] = {0};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

                if (GetSaveFileNameA(&ofn)) {
                    int len = GetWindowTextLengthA(hEdit);
                    char* buf = (char*)GlobalAlloc(GPTR, len + 1);
                    if (buf) {
                        GetWindowTextA(hEdit, buf, len + 1);
                        HANDLE hFile = CreateFileA(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                        if (hFile != INVALID_HANDLE_VALUE) {
                            DWORD written;
                            WriteFile(hFile, buf, len, &written, NULL);
                            CloseHandle(hFile);
                        }
                        GlobalFree(buf);
                    }
                }
            } else if (LOWORD(wParam) == IDM_FILEEXIT) {
                PostQuitMessage(0);
            }
            break;
        case WM_NOTIFY: {
            LPNMHDR lpnmhdr = (LPNMHDR)lParam;
            if (lpnmhdr->idFrom == IDC_TAB && lpnmhdr->code == TCN_SELCHANGE) {
                int selected = SendMessage(hTab, TCM_GETCURSEL, 0, 0);
                if (selected != currentTab) {
                    // Save old buffer
                    GetWindowTextA(hEdit, docs[currentTab], 65536);
                    // Switch to new
                    currentTab = selected;
                    SetWindowTextA(hEdit, docs[currentTab]);
                }
            }
            break;
        }
        case WM_CREATE: {
            LoadLibraryA("Riched20.dll");
            InitCommonControls();
            
            hTab = CreateWindowEx(0, WC_TABCONTROL, "", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                                  0, 0, 0, 0, hwnd, (HMENU)IDC_TAB, GetModuleHandle(NULL), NULL);
            
            docs[0] = (char*)GlobalAlloc(GPTR, 65536);
            lstrcpyA(docs[0], "# Welcome to KWrite\r\n\r\nThis is a richer text editor supporting multiple tabs.\r\nPress Ctrl+B for bold (if supported).");
            
            TCITEMA tie;
            tie.mask = TCIF_TEXT;
            tie.pszText = "Document 1";
            SendMessage(hTab, TCM_INSERTITEMA, 0, (LPARAM)&tie);
            
            hEdit = CreateWindowEx(0, RICHEDIT_CLASS, "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL,
                                   0, 0, 0, 0, hwnd, (HMENU)IDC_RICHEDIT, GetModuleHandle(NULL), NULL);
            
            SendMessage(hEdit, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
            SendMessage(hEdit, EM_SETTEXTMODE, TM_PLAINTEXT, 0); // start simple
            SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)docs[0]);
            break;
        }
        case WM_SIZE: {
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            MoveWindow(hTab, 0, 0, w, 25, TRUE);
            MoveWindow(hEdit, 0, 25, w, h - 25, TRUE);
            break;
        }
        case WM_SETFOCUS:
            SetFocus(hEdit);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KWriteApp";
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName = MAKEINTRESOURCE(101);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KWriteApp", "KWrite", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
