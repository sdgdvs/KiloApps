#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>

#define IDC_OUT 101
#define IDC_IN 102

HWND hOut, hIn;
WNDPROC oldEditProc;
char currentDir[MAX_PATH];

void AppendOutput(const char* text) {
    int len = GetWindowTextLength(hOut);
    SendMessage(hOut, EM_SETSEL, len, len);
    SendMessage(hOut, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessage(hOut, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
}

void ProcessCommand(const char* cmd) {
    char fullCmd[512];
    wsprintfA(fullCmd, "%s> %s", currentDir, cmd);
    AppendOutput(fullCmd);

    if (lstrcmpiA(cmd, "help") == 0) {
        AppendOutput("KTerm Commands:");
        AppendOutput("  help  - Show this message");
        AppendOutput("  ver   - Show OS version");
        AppendOutput("  clear - Clear screen");
        AppendOutput("  dir   - List files");
        AppendOutput("  cd    - Change directory");
    } else if (lstrcmpiA(cmd, "ver") == 0) {
        AppendOutput("KiloOS Native v1.0");
    } else if (lstrcmpiA(cmd, "clear") == 0 || lstrcmpiA(cmd, "cls") == 0) {
        SetWindowTextA(hOut, "");
    } else if (lstrcmpiA(cmd, "dir") == 0) {
        WIN32_FIND_DATAA fd;
        char search[MAX_PATH];
        wsprintfA(search, "%s\\*", currentDir);
        HANDLE hFind = FindFirstFileA(search, &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                AppendOutput(fd.cFileName);
            } while (FindNextFileA(hFind, &fd));
            FindClose(hFind);
        }
    } else if (cmd[0] == 'c' && cmd[1] == 'd') {
        if (cmd[2] == ' ') {
            if (SetCurrentDirectoryA(cmd + 3)) {
                GetCurrentDirectoryA(MAX_PATH, currentDir);
            } else {
                AppendOutput("Directory not found.");
            }
        }
    } else if (lstrlenA(cmd) > 0) {
        AppendOutput("Bad command or file name.");
    }
}

LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == VK_RETURN) {
        char buf[256];
        GetWindowTextA(hIn, buf, sizeof(buf));
        ProcessCommand(buf);
        SetWindowTextA(hIn, "");
        return 0; // Prevent beep
    }
    return CallWindowProc(oldEditProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hOut = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                                  0, 0, 0, 0, hwnd, (HMENU)IDC_OUT, GetModuleHandle(NULL), NULL);
            hIn = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                 0, 0, 0, 0, hwnd, (HMENU)IDC_IN, GetModuleHandle(NULL), NULL);
            
            HFONT hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
            SendMessage(hOut, WM_SETFONT, (WPARAM)hFont, 0);
            SendMessage(hIn, WM_SETFONT, (WPARAM)hFont, 0);
            
            oldEditProc = (WNDPROC)SetWindowLongPtr(hIn, GWLP_WNDPROC, (LONG_PTR)EditProc);
            
            GetCurrentDirectoryA(MAX_PATH, currentDir);
            AppendOutput("KiloOS Terminal\r\nType 'help' for a list of commands.");
            break;
        }
        case WM_SIZE: {
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            MoveWindow(hOut, 0, 0, w, h - 24, TRUE);
            MoveWindow(hIn, 0, h - 24, w, 24, TRUE);
            break;
        }
        case WM_SETFOCUS:
            SetFocus(hIn);
            break;
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(0, 255, 0));
            SetBkColor(hdc, RGB(0, 0, 0));
            return (LRESULT)GetStockObject(BLACK_BRUSH);
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
    wc.lpszClassName = "KTermApp";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KTermApp", "KTerm", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
