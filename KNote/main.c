#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 250
#define H 250

HWND hEdit;
HBRUSH bgBrush;

#define ID_FILE_OPEN 9001
#define ID_FILE_SAVE 9002
#define ID_FILE_EXIT 9003

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
            
            HFONT hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Comic Sans MS");
            if (!hFont) hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            hEdit = CreateWindowEx(0, "EDIT", "Buy milk\r\nFix that bug\r\nCall mom",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL,
                0, 0, W, H, hwnd, NULL, NULL, NULL);
                
            SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_FILE_OPEN) {
                HANDLE hFile = CreateFileA("knote_data.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    DWORD fileSize = GetFileSize(hFile, NULL);
                    char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileSize + 1);
                    if (buf) {
                        DWORD bytesRead;
                        ReadFile(hFile, buf, fileSize, &bytesRead, NULL);
                        buf[fileSize] = 0;
                        SetWindowTextA(hEdit, buf);
                        HeapFree(GetProcessHeap(), 0, buf);
                    }
                    CloseHandle(hFile);
                }
            } else if (LOWORD(wParam) == ID_FILE_SAVE) {
                int len = GetWindowTextLengthA(hEdit);
                char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len + 1);
                if (buf) {
                    GetWindowTextA(hEdit, buf, len + 1);
                    HANDLE hFile = CreateFileA("knote_data.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hFile != INVALID_HANDLE_VALUE) {
                        DWORD bytesWritten;
                        WriteFile(hFile, buf, len, &bytesWritten, NULL);
                        CloseHandle(hFile);
                    }
                    HeapFree(GetProcessHeap(), 0, buf);
                }
            } else if (LOWORD(wParam) == ID_FILE_EXIT) {
                PostMessageA(hwnd, WM_CLOSE, 0, 0);
            }
            break;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(255, 255, 150));
            return (LRESULT)bgBrush;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            MoveWindow(hEdit, 0, 0, nw, nh, TRUE);
            break;
        }
        case WM_DESTROY:
            DeleteObject(bgBrush);
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
