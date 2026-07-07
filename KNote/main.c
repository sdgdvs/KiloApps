#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 250
#define H 250

HWND hEdit;
HBRUSH bgBrush;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            bgBrush = CreateSolidBrush(RGB(255, 255, 150));
            
            HFONT hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Comic Sans MS");
            if (!hFont) hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            hEdit = CreateWindowEx(0, "EDIT", "Buy milk\r\nFix that bug\r\nCall mom",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL,
                0, 0, W, H, hwnd, NULL, NULL, NULL);
                
            SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
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
