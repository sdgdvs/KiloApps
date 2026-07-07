#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 400
#define H 300

HWND hListBox;
HWND hBtnExtract;
HWND hBtnAdd;

const char* dummyFiles[] = {
    "readme.txt",
    "data.bin",
    "image.bmp",
    "config.ini"
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            hListBox = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "",
                WS_CHILD | WS_VISIBLE | LBS_STANDARD,
                10, 10, W - 35, H - 90, hwnd, NULL, NULL, NULL);
            SendMessage(hListBox, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            for (int i = 0; i < 4; i++) {
                SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)dummyFiles[i]);
            }
            
            hBtnExtract = CreateWindowEx(0, "BUTTON", "Extract All",
                WS_CHILD | WS_VISIBLE,
                10, H - 70, 100, 24, hwnd, (HMENU)1, NULL, NULL);
            SendMessage(hBtnExtract, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnAdd = CreateWindowEx(0, "BUTTON", "Add File...",
                WS_CHILD | WS_VISIBLE,
                120, H - 70, 100, 24, hwnd, (HMENU)2, NULL, NULL);
            SendMessage(hBtnAdd, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                MessageBoxA(hwnd, "Extracted 4 files successfully to current directory.", "KZip", MB_OK | MB_ICONINFORMATION);
            } else if (LOWORD(wParam) == 2) {
                MessageBoxA(hwnd, "Added newfile.txt to archive.", "KZip", MB_OK | MB_ICONINFORMATION);
                SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)"newfile.txt");
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            MoveWindow(hListBox, 10, 10, nw - 20, nh - 50, TRUE);
            MoveWindow(hBtnExtract, 10, nh - 35, 100, 24, TRUE);
            MoveWindow(hBtnAdd, 120, nh - 35, 100, 24, TRUE);
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
    wc.lpszClassName = "KZipApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KZipApp", "KZip", WS_OVERLAPPEDWINDOW,
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
