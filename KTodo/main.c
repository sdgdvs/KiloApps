#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

int my_strlen(const char* s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

HWND hInput, hAddBtn, hList, hPriority;

int my_strcpy(char* dest, const char* src) {
    int i = 0;
    while (src[i]) { dest[i] = src[i]; i++; }
    dest[i] = '\0';
    return i;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 10, 10, 140, 24, hwnd, NULL, NULL, NULL);
            hPriority = CreateWindowExA(WS_EX_CLIENTEDGE, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 155, 10, 60, 100, hwnd, NULL, NULL, NULL);
            SendMessageA(hPriority, 0x0143, 0, (LPARAM)"Low"); // CB_ADDSTRING
            SendMessageA(hPriority, 0x0143, 0, (LPARAM)"Med");
            SendMessageA(hPriority, 0x0143, 0, (LPARAM)"High");
            SendMessageA(hPriority, 0x014E, 1, 0); // CB_SETCURSEL (Med)
            hAddBtn = CreateWindowA("BUTTON", "Add", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 220, 10, 50, 24, hwnd, (HMENU)1001, NULL, NULL);
            hList = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL, 10, 45, 260, 200, hwnd, NULL, NULL, NULL);
            
            HFONT hFont = CreateFontA(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Tahoma");
            SendMessageA(hInput, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hPriority, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hAddBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hList, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1001) {
                char buf[256];
                GetWindowTextA(hInput, buf, 200);
                if (my_strlen(buf) > 0) {
                    char prio[16];
                    GetWindowTextA(hPriority, prio, 15);
                    char finalBuf[280];
                    finalBuf[0] = '[';
                    my_strcpy(finalBuf + 1, prio);
                    my_strcpy(finalBuf + 1 + my_strlen(prio), "] ");
                    my_strcpy(finalBuf + 1 + my_strlen(prio) + 2, buf);
                    SendMessageA(hList, LB_INSERTSTRING, 0, (LPARAM)finalBuf);
                    SetWindowTextA(hInput, "");
                }
            } else if (LOWORD(wParam) == 0 && HIWORD(wParam) == LBN_DBLCLK) {
                // Double click deletes task
                int sel = SendMessageA(hList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    SendMessageA(hList, LB_DELETESTRING, sel, 0);
                }
            }
            break;
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
    wc.lpszClassName = "KTodoClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KTodoClass", "KTodo", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
