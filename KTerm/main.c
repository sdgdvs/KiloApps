#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define IDC_OUT 101
#define IDC_IN 102

HWND hOut, hIn;
WNDPROC oldEditProc;
char currentDir[MAX_PATH];

#define MAX_HISTORY 20
char history[MAX_HISTORY][256];
int history_count = 0;
int history_pos = 0;

static int StringStartsWithIC(const char* str, const char* prefix) {
    if (!str || !prefix) return 0;
    while (*prefix) {
        char c1 = *str++;
        char c2 = *prefix++;
        if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
        if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
        if (c1 != c2) return 0;
    }
    return 1;
}

static void FormatPathPrompt(char* dst, size_t dstSize, const char* dir, const char* cmd) {
    char tmp[1024];
    wsprintfA(tmp, "%s> %s", dir ? dir : "", cmd ? cmd : "");
    lstrcpynA(dst, tmp, (int)dstSize);
}

void AppendOutput(const char* text) {
    if (!text) return;
    int len = GetWindowTextLengthA(hOut);
    SendMessageA(hOut, EM_SETSEL, len, len);
    SendMessageA(hOut, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessageA(hOut, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
}

void ProcessCommand(const char* cmd) {
    if (!cmd) return;

    // Skip leading whitespace
    while (*cmd == ' ' || *cmd == '\t') cmd++;

    if (*cmd == '\0') return;

    char fullCmd[512];
    FormatPathPrompt(fullCmd, sizeof(fullCmd), currentDir, cmd);
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
    } else if (lstrcmpiA(cmd, "dir") == 0 || lstrcmpiA(cmd, "ls") == 0) {
        WIN32_FIND_DATAA fd;
        char search[MAX_PATH + 16];
        char tmpSearch[1024];
        wsprintfA(tmpSearch, "%s\\*", currentDir);
        lstrcpynA(search, tmpSearch, sizeof(search));

        HANDLE hFind = FindFirstFileA(search, &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                char entry[MAX_PATH + 32];
                char tmpEntry[1024];
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    wsprintfA(tmpEntry, "<DIR>    %s", fd.cFileName);
                } else {
                    wsprintfA(tmpEntry, "         %s", fd.cFileName);
                }
                lstrcpynA(entry, tmpEntry, sizeof(entry));
                AppendOutput(entry);
            } while (FindNextFileA(hFind, &fd));
            FindClose(hFind);
        } else {
            AppendOutput("Failed to list directory contents.");
        }
    } else if (StringStartsWithIC(cmd, "cd")) {
        const char* target = cmd + 2;
        if (*target == ' ' || *target == '\t' || *target == '\0') {
            while (*target == ' ' || *target == '\t') target++;
            if (*target == '\0') {
                AppendOutput(currentDir);
            } else {
                if (SetCurrentDirectoryA(target)) {
                    GetCurrentDirectoryA(MAX_PATH, currentDir);
                } else {
                    AppendOutput("Directory not found.");
                }
            }
        } else {
            AppendOutput("Bad command or file name.");
        }
    } else if (StringStartsWithIC(cmd, "echo")) {
        const char* text = cmd + 4;
        if (*text == ' ' || *text == '\t' || *text == '\0') {
            while (*text == ' ' || *text == '\t') text++;
            AppendOutput(text);
        } else {
            AppendOutput("Bad command or file name.");
        }
    } else if (StringStartsWithIC(cmd, "mkdir")) {
        const char* dirName = cmd + 5;
        if (*dirName == ' ' || *dirName == '\t' || *dirName == '\0') {
            while (*dirName == ' ' || *dirName == '\t') dirName++;
            if (*dirName == '\0') {
                AppendOutput("Usage: mkdir <directory_name>");
            } else if (CreateDirectoryA(dirName, NULL)) {
                AppendOutput("Directory created.");
            } else {
                AppendOutput("Failed to create directory.");
            }
        } else {
            AppendOutput("Bad command or file name.");
        }
    } else if (StringStartsWithIC(cmd, "type")) {
        const char* fileName = cmd + 4;
        if (*fileName == ' ' || *fileName == '\t' || *fileName == '\0') {
            while (*fileName == ' ' || *fileName == '\t') fileName++;
            if (*fileName == '\0') {
                AppendOutput("Usage: type <filename>");
            } else {
                HANDLE hFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    DWORD fileSize = GetFileSize(hFile, NULL);
                    if (fileSize != INVALID_FILE_SIZE && fileSize > 0) {
                        if (fileSize > 4096) fileSize = 4096;
                        char* buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileSize + 1);
                        if (buffer) {
                            DWORD bytesRead = 0;
                            if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
                                buffer[bytesRead] = '\0';
                                AppendOutput(buffer);
                            }
                            HeapFree(GetProcessHeap(), 0, buffer);
                        }
                    } else if (fileSize == 0) {
                        AppendOutput("[File is empty]");
                    }
                    CloseHandle(hFile);
                } else {
                    AppendOutput("File not found or cannot be opened.");
                }
            }
        } else {
            AppendOutput("Bad command or file name.");
        }
    } else {
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
                    lstrcpynA(history[history_count], buf, sizeof(history[0]));
                    history_count++;
                } else {
                    for (int i = 0; i < MAX_HISTORY - 1; i++) {
                        lstrcpynA(history[i], history[i+1], sizeof(history[0]));
                    }
                    lstrcpynA(history[MAX_HISTORY - 1], buf, sizeof(history[0]));
                }
            }
            history_pos = history_count;
            
            SetWindowTextA(hIn, "");
            return 0; // Prevent default beep
        } else if (wParam == VK_UP) {
            if (history_pos > 0) {
                history_pos--;
                SetWindowTextA(hIn, history[history_pos]);
                int len = lstrlenA(history[history_pos]);
                SendMessageA(hIn, EM_SETSEL, len, len);
            }
            return 0;
        } else if (wParam == VK_DOWN) {
            if (history_pos < history_count - 1) {
                history_pos++;
                SetWindowTextA(hIn, history[history_pos]);
                int len = lstrlenA(history[history_pos]);
                SendMessageA(hIn, EM_SETSEL, len, len);
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
            hOut = CreateWindowExA(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                                  0, 0, 0, 0, hwnd, (HMENU)IDC_OUT, GetModuleHandle(NULL), NULL);
            hIn = CreateWindowExA(0, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                 0, 0, 0, 0, hwnd, (HMENU)IDC_IN, GetModuleHandle(NULL), NULL);
            
            // Set text limit for output control so terminal can log up to 1MB of output without clipping
            SendMessageA(hOut, EM_SETLIMITTEXT, 1048576, 0);

            HFONT hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
            SendMessageA(hOut, WM_SETFONT, (WPARAM)hFont, 0);
            SendMessageA(hIn, WM_SETFONT, (WPARAM)hFont, 0);
            
            oldEditProc = (WNDPROC)SetWindowLongPtrA(hIn, GWLP_WNDPROC, (LONG_PTR)EditProc);
            
            GetCurrentDirectoryA(MAX_PATH, currentDir);
            AppendOutput("KiloOS Terminal\r\nType 'help' for a list of commands.");
            break;
        }
        case WM_SIZE: {
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            int inH = 24;
            int outH = h - inH;
            if (outH < 0) outH = 0;
            MoveWindow(hOut, 0, 0, w, outH, TRUE);
            MoveWindow(hIn, 0, outH, w, inH, TRUE);
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
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KTermApp";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCE(1));
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(0, "KTermApp", "KTerm", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
