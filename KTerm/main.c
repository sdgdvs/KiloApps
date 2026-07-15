#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>

#define IDC_OUT 101
#define IDC_IN 102

HWND hOut, hIn;
WNDPROC oldEditProc;
char currentDir[MAX_PATH];

#define MAX_HISTORY 20
char history[MAX_HISTORY][256];
int history_count = 0;
int history_pos = 0;

void AppendOutput(const char* text) {
    int len = GetWindowTextLength(hOut);
    SendMessage(hOut, EM_SETSEL, len, len);
    SendMessage(hOut, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessage(hOut, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
}

void ProcessCommand(const char* cmd) {
    char fullCmd[1024];
    wsprintfA(fullCmd, "%s> %s", currentDir, cmd);
    AppendOutput(fullCmd);

    if (lstrcmpiA(cmd, "help") == 0) {
        AppendOutput("KTerm Commands:");
        AppendOutput("  help   - Show this message");
        AppendOutput("  ver    - Show OS version");
        AppendOutput("  clear  - Clear screen");
        AppendOutput("  dir    - List files");
        AppendOutput("  cd     - Change directory");
        AppendOutput("  echo   - Print text");
        AppendOutput("  mkdir  - Create directory");
        AppendOutput("  type   - Read file");
        AppendOutput("  date   - Show date");
        AppendOutput("  time   - Show time");
        AppendOutput("  whoami - Current user");
    } else if (lstrcmpiA(cmd, "ver") == 0) {
        AppendOutput("KiloOS Native v1.0");
    } else if (lstrcmpiA(cmd, "date") == 0) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        char buf[64];
        wsprintfA(buf, "%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
        AppendOutput(buf);
    } else if (lstrcmpiA(cmd, "time") == 0) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        char buf[64];
        wsprintfA(buf, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
        AppendOutput(buf);
    } else if (lstrcmpiA(cmd, "whoami") == 0) {
        AppendOutput("kilo_user");
    } else if (lstrcmpiA(cmd, "clear") == 0 || lstrcmpiA(cmd, "cls") == 0) {
        SetWindowTextA(hOut, "");
    } else if (lstrcmpiA(cmd, "dir") == 0) {
        WIN32_FIND_DATAA fd;
        char search[MAX_PATH + 10];
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
    } else if (cmd[0] == 'e' && cmd[1] == 'c' && cmd[2] == 'h' && cmd[3] == 'o' && cmd[4] == ' ') {
        AppendOutput(cmd + 5);
    } else if (cmd[0] == 'm' && cmd[1] == 'k' && cmd[2] == 'd' && cmd[3] == 'i' && cmd[4] == 'r' && cmd[5] == ' ') {
        if (CreateDirectoryA(cmd + 6, NULL)) {
            AppendOutput("Directory created.");
        } else {
            AppendOutput("Failed to create directory.");
        }
    } else if (cmd[0] == 't' && cmd[1] == 'y' && cmd[2] == 'p' && cmd[3] == 'e' && cmd[4] == ' ') {
        HANDLE hFile = CreateFileA(cmd + 5, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD fileSize = GetFileSize(hFile, NULL);
            if (fileSize != INVALID_FILE_SIZE && fileSize > 0) {
                if (fileSize > 4096) fileSize = 4096;
                char* buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileSize + 1);
                if (buffer) {
                    DWORD bytesRead;
                    if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
                        AppendOutput(buffer);
                    }
                    HeapFree(GetProcessHeap(), 0, buffer);
                }
            }
            CloseHandle(hFile);
        } else {
            AppendOutput("File not found or cannot be opened.");
        }
    } else if (lstrlenA(cmd) > 0) {
        AppendOutput("Bad command or file name.");
    }
}

LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN) {
        if (wParam == VK_RETURN) {
            char buf[256];
            GetWindowTextA(hIn, buf, sizeof(buf));
            ProcessCommand(buf);
            
            if (buf[0]) {
                if (history_count < MAX_HISTORY) {
                    lstrcpyA(history[history_count++], buf);
                } else {
                    for (int i = 0; i < MAX_HISTORY - 1; i++) lstrcpyA(history[i], history[i+1]);
                    lstrcpyA(history[MAX_HISTORY - 1], buf);
                }
            }
            history_pos = history_count;
            
            SetWindowTextA(hIn, "");
            return 0; // Prevent beep
        } else if (wParam == VK_UP) {
            if (history_pos > 0) {
                history_pos--;
                SetWindowTextA(hIn, history[history_pos]);
                SendMessage(hIn, EM_SETSEL, 256, 256);
            }
            return 0;
        } else if (wParam == VK_DOWN) {
            if (history_pos < history_count - 1) {
                history_pos++;
                SetWindowTextA(hIn, history[history_pos]);
                SendMessage(hIn, EM_SETSEL, 256, 256);
            } else if (history_pos == history_count - 1) {
                history_pos++;
                SetWindowTextA(hIn, "");
            }
            return 0;
        }
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
