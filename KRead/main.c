#include <windows.h>
#include <commdlg.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

HWND hEdit;

void OpenFileAndLoad(HWND hwnd) {
    OPENFILENAMEA ofn;
    char szFile[260] = {0};

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Text Files\0*.txt\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(ofn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD dwFileSize = GetFileSize(hFile, NULL);
            if (dwFileSize != INVALID_FILE_SIZE) {
                char* pszFileText = (char*)VirtualAlloc(NULL, dwFileSize + 1, MEM_COMMIT, PAGE_READWRITE);
                if (pszFileText) {
                    DWORD dwRead;
                    if (ReadFile(hFile, pszFileText, dwFileSize, &dwRead, NULL)) {
                        pszFileText[dwFileSize] = 0;
                        SetWindowTextA(hEdit, pszFileText);
                    }
                    VirtualFree(pszFileText, 0, MEM_RELEASE);
                }
            }
            CloseHandle(hFile);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HMENU hMenu = CreateMenu();
            HMENU hSubMenu = CreatePopupMenu();
            AppendMenuA(hSubMenu, MF_STRING, 1001, "Open File...");
            AppendMenuA(hSubMenu, MF_STRING, 1002, "Exit");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, "File");
            SetMenu(hwnd, hMenu);

            hEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Use File -> Open to read a text file.", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            
            HFONT hFont = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN, "Georgia");
            SendMessageA(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }
        case WM_SIZE: {
            MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1001) OpenFileAndLoad(hwnd);
            if (LOWORD(wParam) == 1002) PostQuitMessage(0);
            break;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(250, 250, 250));
            SetTextColor(hdc, RGB(30, 30, 30));
            return (LRESULT)CreateSolidBrush(RGB(250, 250, 250));
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KReadClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KReadClass", "KRead", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 600, 500, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
