#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <commdlg.h>

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) {
        *bytes++ = (char)c;
    }
    return dest;
}

#define W 320
#define H 350

HWND hTitle;
HWND hBtnOpen;
HWND hBtnPlay;
HWND hBtnStop;
HWND hBtnPrev;
HWND hBtnNext;
HWND hBtnRem;
HWND hBtnClear;
HWND hListBox;

char currentFile[MAX_PATH] = {0};
char mciCmd[512] = {0};

void OpenFileDlg(HWND hwnd) {
    OPENFILENAMEA ofn = {0};
    char szFile[MAX_PATH] = {0};
    
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Media Files\0*.wav;*.mp3;*.mid\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
    
    if (GetOpenFileNameA(&ofn) == TRUE) {
        char dir[MAX_PATH];
        char* p = ofn.lpstrFile;
        lstrcpyA(dir, p);
        p += lstrlenA(p) + 1;
        if (*p == '\0') {
            // Single file selected
            SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)dir);
        } else {
            // Multiple files selected
            while (*p) {
                char fullPath[MAX_PATH];
                wsprintfA(fullPath, "%s\\%s", dir, p);
                SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)fullPath);
                p += lstrlenA(p) + 1;
            }
        }
    }
}

void PlaySelectedTrack() {
    int idx = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
    if (idx != LB_ERR) {
        SendMessageA(hListBox, LB_GETTEXT, idx, (LPARAM)currentFile);
        
        // Extract just the filename for the title
        char* title = strrchr(currentFile, '\\');
        title = title ? title + 1 : currentFile;
        SetWindowTextA(hTitle, title);
        
        mciSendStringA("close myMedia", NULL, 0, NULL);
        wsprintfA(mciCmd, "open \"%s\" alias myMedia", currentFile);
        mciSendStringA(mciCmd, NULL, 0, NULL);
        mciSendStringA("play myMedia from 0", NULL, 0, NULL);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            hTitle = CreateWindowEx(0, "STATIC", "No file selected",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                10, 15, W - 36, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnOpen = CreateWindowEx(0, "BUTTON", "Add",
                WS_CHILD | WS_VISIBLE,
                10, 50, 65, 30, hwnd, (HMENU)1, NULL, NULL);
            SendMessage(hBtnOpen, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnPlay = CreateWindowEx(0, "BUTTON", "Play",
                WS_CHILD | WS_VISIBLE,
                80, 50, 65, 30, hwnd, (HMENU)2, NULL, NULL);
            SendMessage(hBtnPlay, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnStop = CreateWindowEx(0, "BUTTON", "Stop",
                WS_CHILD | WS_VISIBLE,
                150, 50, 65, 30, hwnd, (HMENU)3, NULL, NULL);
            SendMessage(hBtnStop, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnClear = CreateWindowEx(0, "BUTTON", "Clear",
                WS_CHILD | WS_VISIBLE,
                220, 50, 65, 30, hwnd, (HMENU)8, NULL, NULL);
            SendMessage(hBtnClear, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnPrev = CreateWindowEx(0, "BUTTON", "Prev",
                WS_CHILD | WS_VISIBLE,
                10, 90, 65, 30, hwnd, (HMENU)5, NULL, NULL);
            SendMessage(hBtnPrev, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnNext = CreateWindowEx(0, "BUTTON", "Next",
                WS_CHILD | WS_VISIBLE,
                80, 90, 65, 30, hwnd, (HMENU)6, NULL, NULL);
            SendMessage(hBtnNext, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnRem = CreateWindowEx(0, "BUTTON", "Remove",
                WS_CHILD | WS_VISIBLE,
                150, 90, 65, 30, hwnd, (HMENU)7, NULL, NULL);
            SendMessage(hBtnRem, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hListBox = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | WS_HSCROLL,
                10, 130, W - 36, 170, hwnd, (HMENU)4, NULL, NULL);
            SendMessage(hListBox, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                OpenFileDlg(hwnd);
            } else if (LOWORD(wParam) == 2) {
                PlaySelectedTrack();
            } else if (LOWORD(wParam) == 3) {
                mciSendStringA("stop myMedia", NULL, 0, NULL);
            } else if (LOWORD(wParam) == 5) {
                int idx = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                if (idx > 0) {
                    SendMessage(hListBox, LB_SETCURSEL, idx - 1, 0);
                    PlaySelectedTrack();
                }
            } else if (LOWORD(wParam) == 6) {
                int idx = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                int count = SendMessage(hListBox, LB_GETCOUNT, 0, 0);
                if (idx != LB_ERR && idx < count - 1) {
                    SendMessage(hListBox, LB_SETCURSEL, idx + 1, 0);
                    PlaySelectedTrack();
                }
            } else if (LOWORD(wParam) == 7) {
                int idx = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                if (idx != LB_ERR) {
                    SendMessage(hListBox, LB_DELETESTRING, idx, 0);
                    mciSendStringA("stop myMedia", NULL, 0, NULL);
                    SetWindowTextA(hTitle, "No file selected");
                }
            } else if (LOWORD(wParam) == 8) {
                SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
                mciSendStringA("stop myMedia", NULL, 0, NULL);
                SetWindowTextA(hTitle, "No file selected");
            } else if (LOWORD(wParam) == 4 && HIWORD(wParam) == LBN_DBLCLK) {
                PlaySelectedTrack();
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }
        case WM_DESTROY:
            mciSendStringA("close myMedia", NULL, 0, NULL);
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
    wc.lpszClassName = "KMediaApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KMediaApp", "KMedia", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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
