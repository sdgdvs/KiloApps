#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

int my_strlen(const char* s) {
    if (!s) return 0;
    int len = 0;
    while (s[len]) len++;
    return len;
}

HWND hInput, hAddBtn, hPriority, hDeleteBtn, hClearBtn, hList;
HFONT hFont;

WNDPROC g_OldEditProc = NULL;
WNDPROC g_OldListProc = NULL;

#define ID_INPUT     1000
#define ID_ADDBTN    1001
#define ID_PRIORITY  1002
#define ID_DELETEBTN 1003
#define ID_CLEARBTN  1004
#define ID_LIST      1005

LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == VK_RETURN) {
        HWND hParent = GetParent(hwnd);
        if (hParent) {
            SendMessageA(hParent, WM_COMMAND, MAKEWPARAM(ID_ADDBTN, BN_CLICKED), (LPARAM)hAddBtn);
        }
        return 0;
    }
    return CallWindowProcA(g_OldEditProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK ListSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == VK_DELETE) {
        HWND hParent = GetParent(hwnd);
        if (hParent) {
            SendMessageA(hParent, WM_COMMAND, MAKEWPARAM(ID_DELETEBTN, BN_CLICKED), (LPARAM)hDeleteBtn);
        }
        return 0;
    }
    return CallWindowProcA(g_OldListProc, hwnd, msg, wParam, lParam);
}

void DoAddTask(HWND hwnd) {
    char buf[220];
    buf[0] = '\0';
    GetWindowTextA(hInput, buf, sizeof(buf) - 1);
    
    // Trim leading whitespace
    char* p = buf;
    while (*p == ' ' || *p == '\t') p++;
    
    if (my_strlen(p) > 0) {
        char prio[16];
        prio[0] = '\0';
        GetWindowTextA(hPriority, prio, sizeof(prio) - 1);
        if (my_strlen(prio) == 0) {
            wsprintfA(prio, "Med");
        }
        
        char finalBuf[256];
        wsprintfA(finalBuf, "[%s] %s", prio, p);
        
        SendMessageA(hList, LB_INSERTSTRING, 0, (LPARAM)finalBuf);
        SetWindowTextA(hInput, "");
        SetFocus(hInput);
    }
}

void DoDeleteSelectedTask() {
    int sel = SendMessageA(hList, LB_GETCURSEL, 0, 0);
    if (sel != LB_ERR) {
        SendMessageA(hList, LB_DELETESTRING, sel, 0);
        int count = SendMessageA(hList, LB_GETCOUNT, 0, 0);
        if (count > 0) {
            if (sel >= count) sel = count - 1;
            SendMessageA(hList, LB_SETCURSEL, sel, 0);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 10, 10, 130, 24, hwnd, (HMENU)ID_INPUT, NULL, NULL);
            hPriority = CreateWindowExA(WS_EX_CLIENTEDGE, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 145, 10, 60, 120, hwnd, (HMENU)ID_PRIORITY, NULL, NULL);
            
            SendMessageA(hPriority, 0x0143, 0, (LPARAM)"Low");  // CB_ADDSTRING
            SendMessageA(hPriority, 0x0143, 0, (LPARAM)"Med");
            SendMessageA(hPriority, 0x0143, 0, (LPARAM)"High");
            SendMessageA(hPriority, 0x014E, 1, 0);             // CB_SETCURSEL (Med)
            
            hAddBtn = CreateWindowA("BUTTON", "Add", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 210, 10, 45, 24, hwnd, (HMENU)ID_ADDBTN, NULL, NULL);
            hDeleteBtn = CreateWindowA("BUTTON", "Del", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 260, 10, 40, 24, hwnd, (HMENU)ID_DELETEBTN, NULL, NULL);
            
            hList = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL, 10, 42, 290, 190, hwnd, (HMENU)ID_LIST, NULL, NULL);
            hClearBtn = CreateWindowA("BUTTON", "Clear All", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 240, 75, 24, hwnd, (HMENU)ID_CLEARBTN, NULL, NULL);
            
            hFont = CreateFontA(15, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            if (!hFont) {
                hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            }
            
            SendMessageA(hInput, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hPriority, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hAddBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hDeleteBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hList, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hClearBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Subclass Edit & ListBox controls
            g_OldEditProc = (WNDPROC)SetWindowLongPtrA(hInput, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);
            g_OldListProc = (WNDPROC)SetWindowLongPtrA(hList, GWLP_WNDPROC, (LONG_PTR)ListSubclassProc);
            break;
        }
        case WM_GETMINMAXINFO: {
            LPMINMAXINFO mmi = (LPMINMAXINFO)lParam;
            mmi->ptMinTrackSize.x = 320;
            mmi->ptMinTrackSize.y = 260;
            break;
        }
        case WM_SIZE: {
            int cx = LOWORD(lParam);
            int cy = HIWORD(lParam);
            if (cx > 50 && cy > 50) {
                int inputWidth = cx - 210;
                if (inputWidth < 60) inputWidth = 60;
                
                MoveWindow(hInput, 10, 10, inputWidth, 24, TRUE);
                MoveWindow(hPriority, 15 + inputWidth, 10, 60, 120, TRUE);
                MoveWindow(hAddBtn, 80 + inputWidth, 10, 50, 24, TRUE);
                MoveWindow(hDeleteBtn, 135 + inputWidth, 10, 65, 24, TRUE);
                
                int listHeight = cy - 78;
                if (listHeight < 50) listHeight = 50;
                MoveWindow(hList, 10, 42, cx - 20, listHeight, TRUE);
                MoveWindow(hClearBtn, 10, cy - 30, 80, 24, TRUE);
            }
            break;
        }
        case WM_COMMAND: {
            WORD id = LOWORD(wParam);
            WORD code = HIWORD(wParam);
            
            if (id == ID_ADDBTN) {
                DoAddTask(hwnd);
            } else if (id == ID_DELETEBTN) {
                DoDeleteSelectedTask();
            } else if (id == ID_CLEARBTN) {
                SendMessageA(hList, LB_RESETCONTENT, 0, 0);
            } else if (id == ID_LIST && code == LBN_DBLCLK) {
                DoDeleteSelectedTask();
            }
            break;
        }
        case WM_DESTROY:
            if (hFont && hFont != GetStockObject(DEFAULT_GUI_FONT)) {
                DeleteObject(hFont);
                hFont = NULL;
            }
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
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KTodoClass", "KTodo", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 360, 360, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}

