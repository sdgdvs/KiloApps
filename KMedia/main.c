#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <commdlg.h>
#include <stdio.h>

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

#define W 950
#define H 700
#define MAX_TRACKS 256

HWND g_hwndMain;
HWND hTitle, hEditSearch, hBtnOpen, hBtnPlay, hBtnStop, hBtnPrev, hBtnNext, hBtnRem, hBtnClear, hBtnMode, hBtnSpeed, hBtnExport, hListBox, hSubText, hBtnHelp;
HWND hBtnSeekBack, hBtnSeekFwd, hTimeStatus;

char g_tracks[MAX_TRACKS][MAX_PATH];
int g_trackCount = 0;
int g_currentIndex = -1;
char currentFile[MAX_PATH] = {0};
char mciCmd[512] = {0};
char g_searchQuery[128] = {0};

int g_playbackMode = 0;
int g_speedIndex = 0;
const int g_speeds[] = {1000, 1250, 1500, 2000, 500};
const char* g_speedLabels[] = {"Spd: 1.0x", "Spd: 1.25x", "Spd: 1.5x", "Spd: 2.0x", "Spd: 0.5x"};
const char* g_modeLabels[] = {"Mode: Normal", "Mode: Repeat All", "Mode: Repeat 1", "Mode: Shuffle"};

typedef struct {
    int start;
    int end;
    char text[256];
} Subtitle;

Subtitle g_subs[500];
int g_subCount = 0;

int str_to_int(const char* str) {
    int val = 0;
    while (*str == ' ') str++;
    while (*str >= '0' && *str <= '9') {
        val = val * 10 + (*str - '0');
        str++;
    }
    return val;
}

void FormatTimeMs(int ms, char* out, int outSize) {
    int totalSecs = ms / 1000;
    int m = totalSecs / 60;
    int s = totalSecs % 60;
    wsprintfA(out, "%d:%02d", m, s);
}

void LoadSrt(const char* videoPath) {
    g_subCount = 0;
    SetWindowTextA(hSubText, "");
    char srtPath[MAX_PATH];
    lstrcpyA(srtPath, videoPath);
    char* ext = my_strrchr(srtPath, '.');
    if (ext) {
        lstrcpyA(ext, ".srt");
        HANDLE hFile = CreateFileA(srtPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD size = GetFileSize(hFile, NULL);
            if (size > 0 && size < 1024*1024) {
                char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, size + 1);
                if (buf) {
                    DWORD read;
                    if (ReadFile(hFile, buf, size, &read, NULL)) {
                        buf[read] = '\0';
                        char* p = buf;
                        int state = 0;
                        Subtitle curSub = {0};
                        while (*p) {
                            char line[256];
                            int i = 0;
                            while (*p && *p != '\n' && i < 255) {
                                if (*p != '\r') { line[i++] = *p; }
                                p++;
                            }
                            line[i] = '\0';
                            if (*p == '\n') p++;
                            
                            if (line[0] == '\0') {
                                if (state == 2 && g_subCount < 500) {
                                    g_subs[g_subCount++] = curSub;
                                }
                                state = 0;
                                curSub.text[0] = '\0';
                            } else if (state == 0) {
                                state = 1;
                            } else if (state == 1) {
                                int start = 0, end = 0;
                                const char* arrow = line;
                                while(*arrow && (*arrow != '-' || arrow[1] != '-' || arrow[2] != '>')) arrow++;
                                if (*arrow) {
                                    int h=0, m=0, s=0, ms=0;
                                    const char* t = line;
                                    while(*t == ' ') t++;
                                    while(*t >= '0' && *t <= '9') h = h*10 + (*t++ - '0'); if(*t==':') t++;
                                    while(*t >= '0' && *t <= '9') m = m*10 + (*t++ - '0'); if(*t==':') t++;
                                    while(*t >= '0' && *t <= '9') s = s*10 + (*t++ - '0'); if(*t==',') t++;
                                    while(*t >= '0' && *t <= '9') ms = ms*10 + (*t++ - '0');
                                    start = h*3600000 + m*60000 + s*1000 + ms;
                                    
                                    t = arrow + 3;
                                    h=0; m=0; s=0; ms=0;
                                    while(*t == ' ') t++;
                                    while(*t >= '0' && *t <= '9') h = h*10 + (*t++ - '0'); if(*t==':') t++;
                                    while(*t >= '0' && *t <= '9') m = m*10 + (*t++ - '0'); if(*t==':') t++;
                                    while(*t >= '0' && *t <= '9') s = s*10 + (*t++ - '0'); if(*t==',') t++;
                                    while(*t >= '0' && *t <= '9') ms = ms*10 + (*t++ - '0');
                                    end = h*3600000 + m*60000 + s*1000 + ms;
                                    
                                    curSub.start = start;
                                    curSub.end = end;
                                    state = 2;
                                }
                            } else if (state == 2) {
                                if (curSub.text[0] != '\0' && lstrlenA(curSub.text) + 2 < sizeof(curSub.text)) lstrcatA(curSub.text, "\n");
                                if (lstrlenA(curSub.text) + lstrlenA(line) < sizeof(curSub.text) - 1) lstrcatA(curSub.text, line);
                            }
                        }
                        if (state == 2 && g_subCount < 500) g_subs[g_subCount++] = curSub;
                    }
                    HeapFree(GetProcessHeap(), 0, buf);
                }
            }
            CloseHandle(hFile);
        }
    }
}

void UpdateSubtitles() {
    if (g_subCount == 0 || currentFile[0] == '\0') return;
    char posStr[64] = {0};
    mciSendStringA("status myMedia position", posStr, sizeof(posStr), NULL);
    int pos = str_to_int(posStr);
    for (int i = 0; i < g_subCount; i++) {
        if (pos >= g_subs[i].start && pos <= g_subs[i].end) {
            char currentTxt[256];
            GetWindowTextA(hSubText, currentTxt, sizeof(currentTxt));
            if (lstrcmpA(currentTxt, g_subs[i].text) != 0) {
                SetWindowTextA(hSubText, g_subs[i].text);
            }
            return;
        }
    }
    char currentTxt[256];
    GetWindowTextA(hSubText, currentTxt, sizeof(currentTxt));
    if (currentTxt[0] != '\0') SetWindowTextA(hSubText, "");
}

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
    ofn.lpstrFilter = "Media Files\0*.wav;*.mp3;*.mid;*.avi;*.mp4;*.mkv\0All Files\0*.*\0";
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
        char titleBuf[MAX_PATH + 64];
        wsprintfA(titleBuf, "%s (Press 'H' or F1 for help)", title);
        SetWindowTextA(hTitle, titleBuf);
        
        mciSendStringA("close myMedia", NULL, 0, NULL);
        wsprintfA(mciCmd, "open \"%s\" alias myMedia", currentFile);
        mciSendStringA(mciCmd, NULL, 0, NULL);
        
        wsprintfA(mciCmd, "set myMedia time format ms");
        mciSendStringA(mciCmd, NULL, 0, NULL);

        wsprintfA(mciCmd, "set myMedia speed %d", g_speeds[g_speedIndex]);
        mciSendStringA(mciCmd, NULL, 0, NULL);
        
        mciSendStringA("play myMedia from 0 notify", NULL, 0, g_hwndMain);
        SetWindowTextA(hBtnPlay, "Pause");
        
        LoadSrt(currentFile);
        
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
    if (g_playbackMode == 2) {
        if (g_currentIndex >= 0 && g_currentIndex < g_trackCount) PlayTrackByIndex(g_currentIndex);
        else PlayTrackByIndex(0);
    } else if (g_playbackMode == 3) {
        int next = GetTickCount() % g_trackCount;
        PlayTrackByIndex(next);
    } else if (g_playbackMode == 1) {
        int next = (g_currentIndex + 1) % g_trackCount;
        PlayTrackByIndex(next);
    } else {
        if (g_currentIndex + 1 < g_trackCount) {
            PlayTrackByIndex(g_currentIndex + 1);
        } else {
            SetWindowTextA(hBtnPlay, "Play");
        }
    }
}

void PlayPrevTrack() {
    if (g_trackCount == 0) return;
    if (g_currentIndex > 0) PlayTrackByIndex(g_currentIndex - 1);
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
        char modeStr[64] = {0};
        mciSendStringA("status myMedia mode", modeStr, sizeof(modeStr), NULL);
        if (lstrcmpA(modeStr, "playing") == 0) {
            mciSendStringA("pause myMedia", NULL, 0, NULL);
            SetWindowTextA(hBtnPlay, "Play");
        } else {
            mciSendStringA("play myMedia notify", NULL, 0, g_hwndMain);
            SetWindowTextA(hBtnPlay, "Pause");
        }
    } else if (g_trackCount > 0) {
        PlaySelectedTrack();
    }
}

void StopTrack() {
    mciSendStringA("stop myMedia", NULL, 0, NULL);
    mciSendStringA("seek myMedia to start", NULL, 0, NULL);
    SetWindowTextA(hBtnPlay, "Play");
}

void SeekRelative(int msDelta) {
    if (currentFile[0] == '\0') return;
    char posStr[64] = {0};
    mciSendStringA("status myMedia position", posStr, sizeof(posStr), NULL);
    int pos = str_to_int(posStr) + msDelta;
    if (pos < 0) pos = 0;
    
    char lenStr[64] = {0};
    mciSendStringA("status myMedia length", lenStr, sizeof(lenStr), NULL);
    int len = str_to_int(lenStr);
    if (len > 0 && pos > len) pos = len;
    
    char modeStr[64] = {0};
    mciSendStringA("status myMedia mode", modeStr, sizeof(modeStr), NULL);
    
    wsprintfA(mciCmd, "seek myMedia to %d", pos);
    mciSendStringA(mciCmd, NULL, 0, NULL);
    
    if (lstrcmpA(modeStr, "playing") == 0) {
        mciSendStringA("play myMedia notify", NULL, 0, g_hwndMain);
    }
}

void ExportFrameToBMP() {
    if (currentFile[0] == '\0') {
        MessageBoxA(g_hwndMain, "No media file currently loaded to export.", "Export Frame", MB_OK | MB_ICONINFORMATION);
        return;
    }
    
    // Attempt MCI capture first
    MCIERROR err = mciSendStringA("capture myMedia as frame.bmp", NULL, 0, NULL);
    if (err == 0) {
        MessageBoxA(g_hwndMain, "Frame exported successfully as frame.bmp using MCI.", "Export Frame", MB_OK);
        return;
    }
    
    // Fallback: Screen capture main window
    HDC hdcWindow = GetDC(g_hwndMain);
    HDC hdcMem = CreateCompatibleDC(hdcWindow);
    RECT rc;
    GetClientRect(g_hwndMain, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    
    HBITMAP hbm = CreateCompatibleBitmap(hdcWindow, width, height);
    HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hbm);
    BitBlt(hdcMem, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY);
    
    BITMAPINFOHEADER bi = {sizeof(BITMAPINFOHEADER), width, height, 1, 32, BI_RGB, 0, 0, 0, 0, 0};
    HANDLE hFile = CreateFileA("frame.bmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD dwBytesWritten = 0;
        BITMAPFILEHEADER bmf = {0x4D42, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width * height * 4, 0, 0, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)};
        WriteFile(hFile, &bmf, sizeof(bmf), &dwBytesWritten, NULL);
        WriteFile(hFile, &bi, sizeof(bi), &dwBytesWritten, NULL);
        
        char* bits = (char*)HeapAlloc(GetProcessHeap(), 0, width * height * 4);
        if (bits) {
            GetDIBits(hdcWindow, hbm, 0, height, bits, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
            WriteFile(hFile, bits, width * height * 4, &dwBytesWritten, NULL);
            HeapFree(GetProcessHeap(), 0, bits);
        }
        CloseHandle(hFile);
        MessageBoxA(g_hwndMain, "Client view exported as frame.bmp.", "Export Frame", MB_OK);
    }
    
    SelectObject(hdcMem, hOldBmp);
    DeleteObject(hbm);
    DeleteDC(hdcMem);
    ReleaseDC(g_hwndMain, hdcWindow);
}

void ShowHelpDialog(HWND hwnd) {
    MessageBoxA(hwnd,
        "KMedia Keyboard & Mouse Shortcuts:\n\n"
        "  Space            Play / Pause\n"
        "  Left Arrow / P   Previous Track\n"
        "  Right Arrow / N  Next Track\n"
        "  [ / ]            Seek Backward / Forward 5s\n"
        "  S                Stop Playback\n"
        "  M                Cycle Playback Mode\n"
        "  H / F1           Show this Help\n"
        "  Double Click     Play track from playlist\n",
        "KMedia Help", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(-14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            hTitle = CreateWindowEx(0, "STATIC", "No file selected (Press 'H' or F1 for help)",
                WS_CHILD | WS_VISIBLE | SS_CENTER | 0x4000,
                10, 10, W - 36, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)hFont, TRUE);

            hEditSearch = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                10, 35, W - 36, 22, hwnd, (HMENU)11, NULL, NULL);
            SendMessage(hEditSearch, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hEditSearch, EM_SETCUEBANNER, FALSE, (LPARAM)L"Filter playlist...");
            
            // Row 1 controls (y=62)
            hBtnOpen = CreateWindowEx(0, "BUTTON", "Add",
                WS_CHILD | WS_VISIBLE,
                10, 62, 75, 26, hwnd, (HMENU)1, NULL, NULL);
            SendMessage(hBtnOpen, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnPlay = CreateWindowEx(0, "BUTTON", "Play",
                WS_CHILD | WS_VISIBLE,
                90, 62, 75, 26, hwnd, (HMENU)2, NULL, NULL);
            SendMessage(hBtnPlay, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnStop = CreateWindowEx(0, "BUTTON", "Stop",
                WS_CHILD | WS_VISIBLE,
                170, 62, 75, 26, hwnd, (HMENU)3, NULL, NULL);
            SendMessage(hBtnStop, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnClear = CreateWindowEx(0, "BUTTON", "Clear",
                WS_CHILD | WS_VISIBLE,
                250, 62, 75, 26, hwnd, (HMENU)8, NULL, NULL);
            SendMessage(hBtnClear, WM_SETFONT, (WPARAM)hFont, TRUE);

            hTimeStatus = CreateWindowEx(0, "STATIC", "0:00 / 0:00  [No Media]",
                WS_CHILD | WS_VISIBLE | SS_RIGHT,
                335, 66, W - 36 - 335, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hTimeStatus, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Row 2 controls (y=93)
            hBtnPrev = CreateWindowEx(0, "BUTTON", "Prev",
                WS_CHILD | WS_VISIBLE,
                10, 93, 75, 26, hwnd, (HMENU)5, NULL, NULL);
            SendMessage(hBtnPrev, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnNext = CreateWindowEx(0, "BUTTON", "Next",
                WS_CHILD | WS_VISIBLE,
                90, 93, 75, 26, hwnd, (HMENU)6, NULL, NULL);
            SendMessage(hBtnNext, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnSeekBack = CreateWindowEx(0, "BUTTON", "<< 5s",
                WS_CHILD | WS_VISIBLE,
                170, 93, 75, 26, hwnd, (HMENU)14, NULL, NULL);
            SendMessage(hBtnSeekBack, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnSeekFwd = CreateWindowEx(0, "BUTTON", ">> 5s",
                WS_CHILD | WS_VISIBLE,
                250, 93, 75, 26, hwnd, (HMENU)15, NULL, NULL);
            SendMessage(hBtnSeekFwd, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnRem = CreateWindowEx(0, "BUTTON", "Remove",
                WS_CHILD | WS_VISIBLE,
                330, 93, 75, 26, hwnd, (HMENU)7, NULL, NULL);
            SendMessage(hBtnRem, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnSpeed = CreateWindowEx(0, "BUTTON", "Spd: 1.0x",
                WS_CHILD | WS_VISIBLE,
                410, 93, 85, 26, hwnd, (HMENU)10, NULL, NULL);
            SendMessage(hBtnSpeed, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Row 3 controls (y=124)
            hBtnMode = CreateWindowEx(0, "BUTTON", "Mode: Normal",
                WS_CHILD | WS_VISIBLE,
                10, 124, 155, 26, hwnd, (HMENU)9, NULL, NULL);
            SendMessage(hBtnMode, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnExport = CreateWindowEx(0, "BUTTON", "Export Frame",
                WS_CHILD | WS_VISIBLE,
                170, 124, 155, 26, hwnd, (HMENU)12, NULL, NULL);
            SendMessage(hBtnExport, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnHelp = CreateWindowEx(0, "BUTTON", "Help (F1)",
                WS_CHILD | WS_VISIBLE,
                330, 124, 165, 26, hwnd, (HMENU)13, NULL, NULL);
            SendMessage(hBtnHelp, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // ListBox
            hListBox = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | WS_HSCROLL,
                10, 155, W - 36, H - 315, hwnd, (HMENU)4, NULL, NULL);
            SendMessage(hListBox, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            // Subtitle / Lyrics Area
            HFONT hSubFont = CreateFontA(-16, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            hSubText = CreateWindowEx(WS_EX_CLIENTEDGE, "STATIC", "[ Subtitles will display here when playing media with .srt ]",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                10, H - 150, W - 36, 105, hwnd, NULL, NULL, NULL);
            SendMessage(hSubText, WM_SETFONT, (WPARAM)hSubFont, TRUE);

            SetTimer(hwnd, 1, 200, NULL);
            break;
        }
        case WM_TIMER: {
            if (wParam == 1) {
                UpdateSubtitles();
                if (currentFile[0] != '\0') {
                    char posStr[64] = {0};
                    char lenStr[64] = {0};
                    char modeStr[64] = {0};
                    mciSendStringA("status myMedia position", posStr, sizeof(posStr), NULL);
                    mciSendStringA("status myMedia length", lenStr, sizeof(lenStr), NULL);
                    mciSendStringA("status myMedia mode", modeStr, sizeof(modeStr), NULL);
                    int pos = str_to_int(posStr);
                    int len = str_to_int(lenStr);
                    char szPos[32], szLen[32], statusBuf[128];
                    FormatTimeMs(pos, szPos, sizeof(szPos));
                    FormatTimeMs(len, szLen, sizeof(szLen));
                    wsprintfA(statusBuf, "%s / %s  [%s]", szPos, szLen, modeStr[0] ? modeStr : "stopped");
                    SetWindowTextA(hTimeStatus, statusBuf);
                } else {
                    SetWindowTextA(hTimeStatus, "0:00 / 0:00  [No Media]");
                }
            }
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                OpenFileDlg(hwnd);
            } else if (LOWORD(wParam) == 2) {
                TogglePlayPause();
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
                            SetWindowTextA(hTitle, "No file selected (Press 'H' or F1 for help)");
                            SetWindowTextA(hSubText, "[ Subtitles will display here when playing media with .srt ]");
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
                SetWindowTextA(hTitle, "No file selected (Press 'H' or F1 for help)");
                SetWindowTextA(hSubText, "[ Subtitles will display here when playing media with .srt ]");
                RefilterPlaylist();
            } else if (LOWORD(wParam) == 9) {
                CycleMode();
            } else if (LOWORD(wParam) == 10) {
                CycleSpeed();
            } else if (LOWORD(wParam) == 11 && HIWORD(wParam) == EN_CHANGE) {
                GetWindowTextA(hEditSearch, g_searchQuery, sizeof(g_searchQuery));
                RefilterPlaylist();
            } else if (LOWORD(wParam) == 12) {
                ExportFrameToBMP();
            } else if (LOWORD(wParam) == 13) {
                ShowHelpDialog(g_hwndMain);
            } else if (LOWORD(wParam) == 14) {
                SeekRelative(-5000);
            } else if (LOWORD(wParam) == 15) {
                SeekRelative(5000);
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
            if ((HWND)lParam == hSubText) {
                SetBkColor((HDC)wParam, RGB(0, 0, 0));
                SetTextColor((HDC)wParam, RGB(255, 255, 0));
                return (LRESULT)GetStockObject(BLACK_BRUSH);
            }
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            mciSendStringA("close myMedia", NULL, 0, NULL);
            {
                HFONT hOldFont1 = (HFONT)SendMessage(hTitle, WM_GETFONT, 0, 0);
                HFONT hOldFont2 = (HFONT)SendMessage(hSubText, WM_GETFONT, 0, 0);
                if (hOldFont1) DeleteObject(hOldFont1);
                if (hOldFont2) DeleteObject(hOldFont2);
            }
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    SetProcessDPIAware();
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KMediaApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    DWORD style = (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN;
    RECT rc = {0, 0, W, H};
    AdjustWindowRect(&rc, style, FALSE);

    g_hwndMain = CreateWindowEx(0, "KMediaApp", "KMedia", style,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

    ShowWindow(g_hwndMain, SW_SHOW);
    UpdateWindow(g_hwndMain);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_F1) {
                ShowHelpDialog(g_hwndMain);
                continue;
            }
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
                } else if (msg.wParam == VK_OEM_4 /* [ */) {
                    SeekRelative(-5000);
                    continue;
                } else if (msg.wParam == VK_OEM_6 /* ] */) {
                    SeekRelative(5000);
                    continue;
                } else if (msg.wParam == 'S' || msg.wParam == 's') {
                    StopTrack();
                    continue;
                } else if (msg.wParam == 'M' || msg.wParam == 'm') {
                    CycleMode();
                    continue;
                } else if (msg.wParam == 'H' || msg.wParam == 'h') {
                    ShowHelpDialog(g_hwndMain);
                    continue;
                }
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
