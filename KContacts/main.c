#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

HWND hList, hEdit;
HFONT hFont;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hList = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 10, 10, 150, 240, hwnd, (HMENU)1001, NULL, NULL);
            hEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Details...", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL, 170, 10, 200, 240, hwnd, NULL, NULL, NULL);
            
            SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)"Alice Smith");
            SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)"Bob Jones");
            
            hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Tahoma");
            SendMessageA(hList, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1001 && HIWORD(wParam) == LBN_SELCHANGE) {
                int idx = SendMessageA(hList, LB_GETCURSEL, 0, 0);
                if (idx == 0) SetWindowTextA(hEdit, "Name: Alice Smith\r\nPhone: 555-1234\r\nEmail: alice@example.com");
                else if (idx == 1) SetWindowTextA(hEdit, "Name: Bob Jones\r\nPhone: 555-5678\r\nEmail: bob@example.com");
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
    HWND hwnd = CreateWindowExA(0, "KContactsClass", "KContacts", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
