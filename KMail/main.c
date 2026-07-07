#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 500
#define H 400

HWND hList;
HWND hEdit;
HWND hTitle;

const char* subjects[] = {
    "Welcome to KiloOS",
    "Meeting at 3PM",
    "URGENT: Server down",
    "Newsletter #42"
};

const char* senders[] = {
    "sysadmin@kilo.os",
    "boss@kilo.os",
    "alerts@kilo.os",
    "news@kilo.os"
};

const char* bodies[] = {
    "Hello User,\r\n\r\nWelcome to KiloOS, your new minimalist operating environment. "
    "We have installed several micro-apps for your convenience.\r\n\r\nEnjoy your stay!\r\n- SysAdmin",
    
    "Don't forget our meeting at 3PM in the conference room. We need to discuss the Q3 roadmap.\r\n\r\nRegards,\r\nBoss",
    
    "The primary database server is unresponsive. Please investigate immediately. This is not a drill.",
    
    "Here is your weekly digest of what's new in the world of retro computing..."
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            hList = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", NULL,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
                10, 10, W / 3 - 20, H - 50, hwnd, (HMENU)1, NULL, NULL);
            SendMessage(hList, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hTitle = CreateWindowEx(0, "STATIC", "Select an email to read...",
                WS_CHILD | WS_VISIBLE,
                W / 3, 10, W * 2 / 3 - 20, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                W / 3, 40, W * 2 / 3 - 20, H - 80, hwnd, NULL, NULL, NULL);
            SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            for (int i = 0; i < 4; i++) {
                SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)subjects[i]);
            }
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1 && HIWORD(wParam) == LBN_SELCHANGE) {
                int idx = SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (idx >= 0 && idx < 4) {
                    char titleStr[128];
                    wsprintfA(titleStr, "From: %s", senders[idx]);
                    SetWindowTextA(hTitle, titleStr);
                    SetWindowTextA(hEdit, bodies[idx]);
                }
            }
            break;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            MoveWindow(hList, 10, 10, nw / 3 - 20, nh - 20, TRUE);
            MoveWindow(hTitle, nw / 3, 10, nw * 2 / 3 - 10, 20, TRUE);
            MoveWindow(hEdit, nw / 3, 40, nw * 2 / 3 - 10, nh - 50, TRUE);
            break;
        }
        case WM_CTLCOLORSTATIC: {
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }
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
    wc.lpszClassName = "KMailApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KMailApp", "KMail", WS_OVERLAPPEDWINDOW,
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
