#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <commdlg.h>

#ifndef EM_SETCUEBANNER
#define EM_SETCUEBANNER 0x1501
#endif

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) {
        *bytes++ = (char)c;
    }
    return dest;
}

char* my_strrchr(const char* str, int ch) {
    const char* last = NULL;
    while (*str) {
        if (*str == (char)ch) {
            last = str;
        }
        str++;
    }
    return (char*)last;
}

#define W 340
#define H 430
#define MAX_TRACKS 256

HWND g_hwndMain;
HWND hTitle;
HWND hEditSearch;
HWND hBtnOpen;
HWND hBtnPlay;
HWND hBtnStop;
HWND hBtnPrev;
HWND hBtnNext;
HWND hBtnRem;
HWND hBtnClear;
HWND hBtnMode;
HWND hBtnSpeed;
HWND hListBox;

char g_tracks[MAX_TRACKS][MAX_PATH];
int g_trackCount = 0;
int g_currentIndex = -1;
char currentFile[MAX_PATH] = {0};
char mciCmd[512] = {0};
char g_searchQuery[128] = {0};

int g_playbackMode = 0; // 0: Normal, 1: Repeat All, 2: Repeat 1, 3: Shuffle
int g_speedIndex = 0;   // 0: 1.0x, 1: 1.25x, 2: 1.5x, 3: 2.0x, 4: 0.5x
const int g_speeds[] = {1000, 1250, 1500, 2000, 500};
const char* g_speedLabels[] = {"Spd: 1.0x", "Spd: 1.25x", "Spd: 1.5x", "Spd: 2.0x", "Spd: 0.5x"};
const char* g_modeLabels[] = {"Mode: Normal", "Mode: Repeat All", "Mode: Repeat 1", "Mode: Shuffle"};

int ContainsCaseInsensitive(const char* haystack, const char* needle) {
    if (!needle || !*needle) return 1;
    if (!haystack) return 0;
    for (; *haystack; haystack++) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n) {
            char ch1 = *h;
            char ch2 = *n;
            if (ch1 >= 'A' && ch1 <= 'Z') ch1 += 32;
            if (ch2 >= 'A' && ch2 <= 'Z') ch2 += 32;
            if (ch1 != ch2) break;
            h++;
            n++;
        }
        if (!*n) return 1;
    }
    return 0;
}

void RefilterPlaylist() {
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < g_trackCount; i++) {
        char* fname = my_strrchr(g_tracks[i], '\\');
        fname = fname ? fname + 1 : g_tracks[i];
        
        if (g_searchQuery[0] == '\0' || ContainsCaseInsensitive(fname, g_searchQuery) || ContainsCaseInsensitive(g_tracks[i], g_searchQuery)) {
            int pos = SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)fname);
            SendMessage(hListBox, LB_SETITEMDATA, pos, (LPARAM)i);
            if (i == g_currentIndex) {
                SendMessage(hListBox, LB_SETCURSEL, pos, 0);
            }
        }
    }
}

void AddTrack(const char* path) {
    if (g_trackCount < MAX_TRACKS) {
        lstrcpyA(g_tracks[g_trackCount], path);
        g_trackCount++;
    }
}

void OpenFileDlg(HWND hwnd) {
    OPENFILENAMEA ofn = {0};
    char szFile[MAX_PATH * 16] = {0};
    
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Media Files\0*.wav;*.mp3;*.mid\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
    
    if (GetOpenFileNameA(&ofn) == TRUE) {
        char dir[MAX_PATH];
        char* p = ofn.lpstrFile;
        lstrcpyA(dir, p);
        p += lstrlenA(p) + 1;
        if (*p == '\0') {
            AddTrack(dir);
        } else {
            while (*p) {
                char fullPath[MAX_PATH];
                wsprintfA(fullPath, "%s\\%s", dir, p);
                AddTrack(fullPath);
                p += lstrlenA(p) + 1;
            }
        }
        RefilterPlaylist();
    }
}

void PlayTrackByIndex(int masterIdx) {
    if (masterIdx >= 0 && masterIdx < g_trackCount) {
        g_currentIndex = masterIdx;
        lstrcpyA(currentFile, g_tracks[masterIdx]);
        
        char* title = my_strrchr(currentFile, '\\');
        title = title ? title + 1 : currentFile;
        SetWindowTextA(hTitle, title);
        
        mciSendStringA("close myMedia", NULL, 0, NULL);
        wsprintfA(mciCmd, "open \"%s\" alias myMedia", currentFile);
        mciSendStringA(mciCmd, NULL, 0, NULL);
        
        wsprintfA(mciCmd, "set myMedia speed %d", g_speeds[g_speedIndex]);
        mciSendStringA(mciCmd, NULL, 0, NULL);
        
        mciSendStringA("play myMedia from 0 notify", NULL, 0, g_hwndMain);
        
        int count = SendMessage(hListBox, LB_GETCOUNT, 0, 0);
        for (int i = 0; i < count; i++) {
            int data = SendMessage(hListBox, LB_GETITEMDATA, i, 0);
            if (data == masterIdx) {
                SendMessage(hListBox, LB_SETCURSEL, i, 0);
                break;
            }
        }
    }
}

void PlaySelectedTrack() {
    int sel = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
    if (sel != LB_ERR) {
        int masterIdx = SendMessage(hListBox, LB_GETITEMDATA, sel, 0);
        PlayTrackByIndex(masterIdx);
    } else if (g_trackCount > 0) {
        PlayTrackByIndex(0);
    }
}

void PlayNextTrackAuto() {
    if (g_trackCount == 0) return;
    
    if (g_playbackMode == 2) { // Repeat 1
        if (g_currentIndex >= 0 && g_currentIndex < g_trackCount) {
            PlayTrackByIndex(g_currentIndex);
        } else {
            PlayTrackByIndex(0);
        }
    } else if (g_playbackMode == 3) { // Shuffle
        int next = GetTickCount() % g_trackCount;
        PlayTrackByIndex(next);
    } else if (g_playbackMode == 1) { // Repeat All
        int next = (g_currentIndex + 1) % g_trackCount;
        PlayTrackByIndex(next);
    } else { // Normal
        if (g_currentIndex + 1 < g_trackCount) {
            PlayTrackByIndex(g_currentIndex + 1);
        }
    }
}

void PlayPrevTrack() {
    if (g_trackCount == 0) return;
    if (g_currentIndex > 0) {
        PlayTrackByIndex(g_currentIndex - 1);
    }
}

void CycleMode() {
    g_playbackMode = (g_playbackMode + 1) % 4;
    SetWindowTextA(hBtnMode, g_modeLabels[g_playbackMode]);
}

void CycleSpeed() {
    g_speedIndex = (g_speedIndex + 1) % 5;
    SetWindowTextA(hBtnSpeed, g_speedLabels[g_speedIndex]);
    if (currentFile[0] != '\0') {
        wsprintfA(mciCmd, "set myMedia speed %d", g_speeds[g_speedIndex]);
        mciSendStringA(mciCmd, NULL, 0, NULL);
    }
}

void TogglePlayPause() {
    if (currentFile[0] != '\0') {
        mciSendStringA("play myMedia notify", NULL, 0, g_hwndMain);
    } else if (g_trackCount > 0) {
        PlaySelectedTrack();
    }
}

void StopTrack() {
    mciSendStringA("stop myMedia", NULL, 0, NULL);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            hTitle = CreateWindowEx(0, "STATIC", "No file selected",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                10, 10, W - 36, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)hFont, TRUE);

            hEditSearch = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                10, 35, W - 36, 22, hwnd, (HMENU)11, NULL, NULL);
            SendMessage(hEditSearch, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hEditSearch, EM_SETCUEBANNER, FALSE, (LPARAM)L"Filter playlist...");
            
            hBtnOpen = CreateWindowEx(0, "BUTTON", "Add",
                WS_CHILD | WS_VISIBLE,
                10, 62, 70, 26, hwnd, (HMENU)1, NULL, NULL);
            SendMessage(hBtnOpen, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnPlay = CreateWindowEx(0, "BUTTON", "Play",
                WS_CHILD | WS_VISIBLE,
                85, 62, 70, 26, hwnd, (HMENU)2, NULL, NULL);
            SendMessage(hBtnPlay, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnStop = CreateWindowEx(0, "BUTTON", "Stop",
                WS_CHILD | WS_VISIBLE,
                160, 62, 70, 26, hwnd, (HMENU)3, NULL, NULL);
            SendMessage(hBtnStop, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnClear = CreateWindowEx(0, "BUTTON", "Clear",
                WS_CHILD | WS_VISIBLE,
                235, 62, 70, 26, hwnd, (HMENU)8, NULL, NULL);
            SendMessage(hBtnClear, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnPrev = CreateWindowEx(0, "BUTTON", "Prev",
                WS_CHILD | WS_VISIBLE,
                10, 93, 70, 26, hwnd, (HMENU)5, NULL, NULL);
            SendMessage(hBtnPrev, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnNext = CreateWindowEx(0, "BUTTON", "Next",
                WS_CHILD | WS_VISIBLE,
                85, 93, 70, 26, hwnd, (HMENU)6, NULL, NULL);
            SendMessage(hBtnNext, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnRem = CreateWindowEx(0, "BUTTON", "Remove",
                WS_CHILD | WS_VISIBLE,
                160, 93, 70, 26, hwnd, (HMENU)7, NULL, NULL);
            SendMessage(hBtnRem, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnSpeed = CreateWindowEx(0, "BUTTON", "Spd: 1.0x",
                WS_CHILD | WS_VISIBLE,
                235, 93, 70, 26, hwnd, (HMENU)10, NULL, NULL);
            SendMessage(hBtnSpeed, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnMode = CreateWindowEx(0, "BUTTON", "Mode: Normal",
                WS_CHILD | WS_VISIBLE,
                10, 124, W - 36, 26, hwnd, (HMENU)9, NULL, NULL);
            SendMessage(hBtnMode, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hListBox = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | WS_HSCROLL,
                10, 155, W - 36, 220, hwnd, (HMENU)4, NULL, NULL);
            SendMessage(hListBox, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                OpenFileDlg(hwnd);
            } else if (LOWORD(wParam) == 2) {
                PlaySelectedTrack();
            } else if (LOWORD(wParam) == 3) {
                StopTrack();
            } else if (LOWORD(wParam) == 5) {
                PlayPrevTrack();
            } else if (LOWORD(wParam) == 6) {
                PlayNextTrackAuto();
            } else if (LOWORD(wParam) == 7) {
                int sel = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    int masterIdx = SendMessage(hListBox, LB_GETITEMDATA, sel, 0);
                    if (masterIdx >= 0 && masterIdx < g_trackCount) {
                        for (int i = masterIdx; i < g_trackCount - 1; i++) {
                            lstrcpyA(g_tracks[i], g_tracks[i+1]);
                        }
                        g_trackCount--;
                        if (g_currentIndex == masterIdx) {
                            StopTrack();
                            mciSendStringA("close myMedia", NULL, 0, NULL);
                            currentFile[0] = '\0';
                            SetWindowTextA(hTitle, "No file selected");
                            g_currentIndex = -1;
                        } else if (g_currentIndex > masterIdx) {
                            g_currentIndex--;
                        }
                        RefilterPlaylist();
                    }
                }
            } else if (LOWORD(wParam) == 8) {
                g_trackCount = 0;
                g_currentIndex = -1;
                currentFile[0] = '\0';
                StopTrack();
                mciSendStringA("close myMedia", NULL, 0, NULL);
                SetWindowTextA(hTitle, "No file selected");
                RefilterPlaylist();
            } else if (LOWORD(wParam) == 9) {
                CycleMode();
            } else if (LOWORD(wParam) == 10) {
                CycleSpeed();
            } else if (LOWORD(wParam) == 11 && HIWORD(wParam) == EN_CHANGE) {
                GetWindowTextA(hEditSearch, g_searchQuery, sizeof(g_searchQuery));
                RefilterPlaylist();
            } else if (LOWORD(wParam) == 4 && HIWORD(wParam) == LBN_DBLCLK) {
                PlaySelectedTrack();
            }
            break;
        }
        case MM_MCINOTIFY: {
            if (wParam == MCI_NOTIFY_SUCCESSFUL) {
                PlayNextTrackAuto();
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }
        case WM_DESTROY:
            mciSendStringA("close myMedia", NULL, 0, NULL);
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
    wc.lpszClassName = "KMediaApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    g_hwndMain = CreateWindowEx(0, "KMediaApp", "KMedia", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, W, H, NULL, NULL, hInstance, NULL);

    ShowWindow(g_hwndMain, SW_SHOW);
    UpdateWindow(g_hwndMain);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN) {
            HWND hFocus = GetFocus();
            if (hFocus != hEditSearch) {
                if (msg.wParam == VK_SPACE) {
                    TogglePlayPause();
                    continue;
                } else if (msg.wParam == 'P' || msg.wParam == 'p' || msg.wParam == VK_LEFT) {
                    PlayPrevTrack();
                    continue;
                } else if (msg.wParam == 'N' || msg.wParam == 'n' || msg.wParam == VK_RIGHT) {
                    PlayNextTrackAuto();
                    continue;
                } else if (msg.wParam == 'S' || msg.wParam == 's') {
                    StopTrack();
                    continue;
                } else if (msg.wParam == 'M' || msg.wParam == 'm') {
                    CycleMode();
                    continue;
                }
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
