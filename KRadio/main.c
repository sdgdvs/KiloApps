#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) {
        *bytes++ = (char)c;
    }
    return dest;
}

#define W 480
#define H 360

#define ID_BTN_PLAY     1
#define ID_BTN_STOP     2
#define ID_BTN_HELP     3
#define ID_BTN_SAVE     4
#define ID_BTN_LOAD     5
#define ID_PRESET_1     101
#define ID_PRESET_2     102
#define ID_PRESET_3     103
#define ID_PRESET_4     104
#define ID_PRESET_5     105
#define ID_PRESET_6     106

typedef struct {
    const char* name;
    const char* genre;
    const char* url;
} StationPreset;

typedef struct {
    DWORD magic;        // 0x4441524B ('KRAD')
    int activePreset;   // 0-5, or -1 for custom
    BOOL isPlaying;     // TRUE or FALSE
    char url[512];      // URL string
} KRadioState;

static const StationPreset g_presets[6] = {
    { "Subspace",     "Synthwave",    "https://radio.erb.pw/public/subspace" },
    { "Groove Salad", "Chillout",     "https://ice2.somafm.com/groovesalad-128-mp3" },
    { "Nightwave",    "Vaporwave",    "https://radio.plaza.one/mp3" },
    { "DEF CON",      "Cyber Ambient","https://ice2.somafm.com/defcon-128-mp3" },
    { "Secret Agent", "Spy Retro",    "https://ice1.somafm.com/secretagent-128-mp3" },
    { "Space Station","Spacemusic",   "https://ice1.somafm.com/spacestation-128-mp3" }
};

HWND g_hwndMain = NULL;
HWND hTitle = NULL;
HWND hEditUrl = NULL;
HWND hBtnPlay = NULL;
HWND hBtnStop = NULL;
HWND hBtnHelp = NULL;
HWND hBtnSave = NULL;
HWND hBtnLoad = NULL;
HWND hBtnPresets[6] = {0};
HWND hStatus = NULL;
HWND hHint = NULL;
HWND hPresetsLabel = NULL;
HWND hUrlLabel = NULL;

WNDPROC g_pfnOrigEditProc = NULL;
char mciCmd[1024] = {0};
BOOL g_isPlaying = FALSE;

void ShowHelpDialog(HWND hwnd) {
    MessageBoxA(hwnd,
        "=== KRadio Retro Stream Player ===\n\n"
        "Tuning & Controls:\n"
        "  - [1-6] Quick Presets: Instantly switch & play preset stations\n"
        "  - [Space / P] Play: Tune into the current URL\n"
        "  - [S] Stop: Stop active audio stream\n"
        "  - [Enter]: Tune to stream when typing in URL field\n"
        "  - [F5] Quicksave: Save active station & playback state\n"
        "  - [F9] Quickload: Restore saved station & playback state\n"
        "  - [F1 / H]: Show this Help reference\n\n"
        "Station Presets:\n"
        "  [1] Subspace (Synthwave / Retrowave)\n"
        "  [2] Groove Salad (Ambient Chillout)\n"
        "  [3] Nightwave Plaza (Vaporwave)\n"
        "  [4] DEF CON Radio (Cyber Ambient)\n"
        "  [5] Secret Agent (Spy / Retro Lounge)\n"
        "  [6] Space Station (Spacemusic Ambient)\n\n"
        "Supports direct MP3 and AAC streaming links.",
        "KRadio Help & Keyboard Shortcuts",
        MB_OK | MB_ICONINFORMATION);
}

void StopStream() {
    mciSendStringA("stop myStream", NULL, 0, NULL);
    mciSendStringA("close myStream", NULL, 0, NULL);
    g_isPlaying = FALSE;
    if (hStatus) SetWindowTextA(hStatus, "Status: Stopped");
}

void PlayStream(HWND hwnd) {
    char url[512] = {0};
    GetWindowTextA(hEditUrl, url, sizeof(url));
    if (url[0] == '\0') {
        if (hStatus) SetWindowTextA(hStatus, "Status: Please enter a stream URL");
        return;
    }

    if (hStatus) SetWindowTextA(hStatus, "Status: Connecting to stream...");
    
    // Stop any existing stream
    mciSendStringA("close myStream", NULL, 0, NULL);
    
    wsprintfA(mciCmd, "open \"%s\" alias myStream", url);
    MCIERROR err = mciSendStringA(mciCmd, NULL, 0, NULL);
    if (err == 0) {
        mciSendStringA("play myStream", NULL, 0, NULL);
        g_isPlaying = TRUE;
        if (hStatus) SetWindowTextA(hStatus, "Status: Playing live stream");
    } else {
        g_isPlaying = FALSE;
        if (hStatus) SetWindowTextA(hStatus, "Status: Error opening stream (Server offline / Format unsupported)");
    }
}

void SelectPreset(int index) {
    if (index < 0 || index >= 6) return;
    SetWindowTextA(hEditUrl, g_presets[index].url);
    PlayStream(g_hwndMain);
}

void QuicksaveState(HWND hwnd) {
    KRadioState state;
    memset(&state, 0, sizeof(state));
    state.magic = 0x4441524B;
    GetWindowTextA(hEditUrl, state.url, sizeof(state.url));
    state.activePreset = -1;
    for (int i = 0; i < 6; i++) {
        const char* s1 = state.url;
        const char* s2 = g_presets[i].url;
        int match = 1;
        while (*s1 || *s2) {
            if (*s1 != *s2) { match = 0; break; }
            s1++; s2++;
        }
        if (match) {
            state.activePreset = i;
            break;
        }
    }
    state.isPlaying = g_isPlaying;

    HANDLE hFile = CreateFileA("kradio_save.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hFile, &state, sizeof(state), &written, NULL);
        CloseHandle(hFile);

        HANDLE hTut = CreateFileA("kradio_tutorial.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hTut != INVALID_HANDLE_VALUE) {
            char d = '1';
            WriteFile(hTut, &d, 1, &written, NULL);
            CloseHandle(hTut);
        }

        if (hStatus) SetWindowTextA(hStatus, "Status: Quicksave saved [F5]");
    } else {
        if (hStatus) SetWindowTextA(hStatus, "Status: Error saving state");
    }
}

void QuickloadState(HWND hwnd) {
    HANDLE hFile = CreateFileA("kradio_save.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        if (hStatus) SetWindowTextA(hStatus, "Status: No quicksave found [F5 to save]");
        return;
    }

    KRadioState state;
    memset(&state, 0, sizeof(state));
    DWORD read = 0;
    BOOL ok = ReadFile(hFile, &state, sizeof(state), &read, NULL);
    CloseHandle(hFile);

    if (ok && read == sizeof(state) && state.magic == 0x4441524B) {
        SetWindowTextA(hEditUrl, state.url);
        HANDLE hTut = CreateFileA("kradio_tutorial.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hTut != INVALID_HANDLE_VALUE) {
            char d = '1';
            DWORD written = 0;
            WriteFile(hTut, &d, 1, &written, NULL);
            CloseHandle(hTut);
        }

        if (state.isPlaying) {
            PlayStream(hwnd);
        } else {
            StopStream();
        }
        if (hStatus) SetWindowTextA(hStatus, "Status: Quickload restored [F9]");
    } else {
        if (hStatus) SetWindowTextA(hStatus, "Status: Corrupt quicksave file");
    }
}

LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN) {
        if (wParam == VK_RETURN) {
            PlayStream(GetParent(hwnd));
            return 0;
        }
        if (wParam == VK_ESCAPE) {
            SetFocus(GetParent(hwnd));
            return 0;
        }
        if (wParam == VK_F1) {
            ShowHelpDialog(GetParent(hwnd));
            return 0;
        }
        if (wParam == VK_F5) {
            QuicksaveState(GetParent(hwnd));
            return 0;
        }
        if (wParam == VK_F9) {
            QuickloadState(GetParent(hwnd));
            return 0;
        }
    }
    return CallWindowProcA(g_pfnOrigEditProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_hwndMain = hwnd;
            int dpi = 96;
            HMODULE hUser32 = GetModuleHandleA("user32.dll");
            if (hUser32) {
                typedef UINT(WINAPI *GETDPIFORWINDOW)(HWND);
                GETDPIFORWINDOW pGetDpiForWindow = (GETDPIFORWINDOW)GetProcAddress(hUser32, "GetDpiForWindow");
                if (pGetDpiForWindow) dpi = pGetDpiForWindow(hwnd);
            }
            int fontHeightNormal = -MulDiv(11, dpi, 72);
            int fontHeightBold = -MulDiv(13, dpi, 72);
            int fontHeightSmall = -MulDiv(9, dpi, 72);

            HFONT hFontNormal = CreateFontA(fontHeightNormal, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            HFONT hFontBold = CreateFontA(fontHeightBold, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            HFONT hFontSmall = CreateFontA(fontHeightSmall, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            // Title Header
            hTitle = CreateWindowEx(0, "STATIC", "KRadio - Retro Stream Player",
                WS_CHILD | WS_VISIBLE,
                16, 14, 280, 24, hwnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)hFontBold, TRUE);

            // Help button
            hBtnHelp = CreateWindowEx(0, "BUTTON", "Help [F1]",
                WS_CHILD | WS_VISIBLE,
                W - 110, 12, 90, 28, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);
            SendMessage(hBtnHelp, WM_SETFONT, (WPARAM)hFontNormal, TRUE);

            // URL Label
            hUrlLabel = CreateWindowEx(0, "STATIC", "Stream URL or Direct MP3/AAC Address:",
                WS_CHILD | WS_VISIBLE,
                16, 48, W - 32, 18, hwnd, NULL, NULL, NULL);
            SendMessage(hUrlLabel, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

            // URL Edit Box
            hEditUrl = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "https://radio.erb.pw/public/subspace",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                16, 70, W - 32, 26, hwnd, NULL, NULL, NULL);
            SendMessage(hEditUrl, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            g_pfnOrigEditProc = (WNDPROC)SetWindowLongPtrA(hEditUrl, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

            // Presets Label
            hPresetsLabel = CreateWindowEx(0, "STATIC", "Station Presets (Hotkeys 1 - 6):",
                WS_CHILD | WS_VISIBLE,
                16, 108, W - 32, 18, hwnd, NULL, NULL, NULL);
            SendMessage(hPresetsLabel, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

            // Preset Buttons (Row 1: 3 buttons, Row 2: 3 buttons)
            hBtnPresets[0] = CreateWindowEx(0, "BUTTON", "[1] Subspace",
                WS_CHILD | WS_VISIBLE,
                16, 130, 140, 28, hwnd, (HMENU)ID_PRESET_1, NULL, NULL);
            SendMessage(hBtnPresets[0], WM_SETFONT, (WPARAM)hFontNormal, TRUE);

            hBtnPresets[1] = CreateWindowEx(0, "BUTTON", "[2] Groove Salad",
                WS_CHILD | WS_VISIBLE,
                164, 130, 140, 28, hwnd, (HMENU)ID_PRESET_2, NULL, NULL);
            SendMessage(hBtnPresets[1], WM_SETFONT, (WPARAM)hFontNormal, TRUE);

            hBtnPresets[2] = CreateWindowEx(0, "BUTTON", "[3] Nightwave",
                WS_CHILD | WS_VISIBLE,
                312, 130, 140, 28, hwnd, (HMENU)ID_PRESET_3, NULL, NULL);
            SendMessage(hBtnPresets[2], WM_SETFONT, (WPARAM)hFontNormal, TRUE);

            hBtnPresets[3] = CreateWindowEx(0, "BUTTON", "[4] DEF CON",
                WS_CHILD | WS_VISIBLE,
                16, 164, 140, 28, hwnd, (HMENU)ID_PRESET_4, NULL, NULL);
            SendMessage(hBtnPresets[3], WM_SETFONT, (WPARAM)hFontNormal, TRUE);

            hBtnPresets[4] = CreateWindowEx(0, "BUTTON", "[5] Secret Agent",
                WS_CHILD | WS_VISIBLE,
                164, 164, 140, 28, hwnd, (HMENU)ID_PRESET_5, NULL, NULL);
            SendMessage(hBtnPresets[4], WM_SETFONT, (WPARAM)hFontNormal, TRUE);

            hBtnPresets[5] = CreateWindowEx(0, "BUTTON", "[6] Space Station",
                WS_CHILD | WS_VISIBLE,
                312, 164, 140, 28, hwnd, (HMENU)ID_PRESET_6, NULL, NULL);
            SendMessage(hBtnPresets[5], WM_SETFONT, (WPARAM)hFontNormal, TRUE);

            // Playback Action Buttons
            hBtnPlay = CreateWindowEx(0, "BUTTON", "Play [Space]",
                WS_CHILD | WS_VISIBLE,
                16, 206, 108, 36, hwnd, (HMENU)ID_BTN_PLAY, NULL, NULL);
            SendMessage(hBtnPlay, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            
            hBtnStop = CreateWindowEx(0, "BUTTON", "Stop [S]",
                WS_CHILD | WS_VISIBLE,
                130, 206, 82, 36, hwnd, (HMENU)ID_BTN_STOP, NULL, NULL);
            SendMessage(hBtnStop, WM_SETFONT, (WPARAM)hFontNormal, TRUE);

            hBtnSave = CreateWindowEx(0, "BUTTON", "Save [F5]",
                WS_CHILD | WS_VISIBLE,
                218, 206, 110, 36, hwnd, (HMENU)ID_BTN_SAVE, NULL, NULL);
            SendMessage(hBtnSave, WM_SETFONT, (WPARAM)hFontNormal, TRUE);

            hBtnLoad = CreateWindowEx(0, "BUTTON", "Load [F9]",
                WS_CHILD | WS_VISIBLE,
                334, 206, 114, 36, hwnd, (HMENU)ID_BTN_LOAD, NULL, NULL);
            SendMessage(hBtnLoad, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            
            // Status Display
            hStatus = CreateWindowEx(0, "STATIC", "Status: Ready",
                WS_CHILD | WS_VISIBLE,
                16, 254, W - 32, 22, hwnd, NULL, NULL, NULL);
            SendMessage(hStatus, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            
            // Shortcut Hint
            hHint = CreateWindowEx(0, "STATIC", "Hotkeys: [1-6] Presets | [Space] Play | [S] Stop | [F5] Save | [F9] Load | [F1] Help",
                WS_CHILD | WS_VISIBLE,
                16, 282, W - 32, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hHint, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            
            // Auto play on startup
            PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_PLAY, 0), 0);
            break;
        }
        case WM_KEYDOWN: {
            if (wParam == VK_F1 || wParam == 'H' || wParam == 'h') {
                ShowHelpDialog(hwnd);
            } else if (wParam == VK_F5) {
                QuicksaveState(hwnd);
            } else if (wParam == VK_F9) {
                QuickloadState(hwnd);
            } else if (wParam >= '1' && wParam <= '6') {
                SelectPreset((int)(wParam - '1'));
            } else if (wParam == VK_SPACE || wParam == 'P' || wParam == 'p') {
                if (g_isPlaying) StopStream();
                else PlayStream(hwnd);
            } else if (wParam == 'S' || wParam == 's') {
                StopStream();
            }
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == ID_BTN_PLAY) {
                PlayStream(hwnd);
            } else if (id == ID_BTN_STOP) {
                StopStream();
            } else if (id == ID_BTN_HELP) {
                ShowHelpDialog(hwnd);
            } else if (id == ID_BTN_SAVE) {
                QuicksaveState(hwnd);
            } else if (id == ID_BTN_LOAD) {
                QuickloadState(hwnd);
            } else if (id >= ID_PRESET_1 && id <= ID_PRESET_6) {
                SelectPreset(id - ID_PRESET_1);
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }
        case WM_DESTROY:
            mciSendStringA("close myStream", NULL, 0, NULL);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        typedef BOOL(WINAPI *SETPROCESSDPIAWARE)(void);
        SETPROCESSDPIAWARE pSetProcessDPIAware = (SETPROCESSDPIAWARE)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (pSetProcessDPIAware) pSetProcessDPIAware();
    }

    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KRadioApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    DWORD style = (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN;
    RECT rect = { 0, 0, W, H };
    AdjustWindowRect(&rect, style, FALSE);

    HWND hwnd = CreateWindowEx(0, "KRadioApp", "KRadio - Retro Stream Player [F1: Help]", style,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // First-run tutorial check (only fires on fresh sessions, never interrupts restored states)
    DWORD tutAttr = GetFileAttributesA("kradio_tutorial.dat");
    DWORD saveAttr = GetFileAttributesA("kradio_save.dat");
    if (tutAttr == INVALID_FILE_ATTRIBUTES && saveAttr == INVALID_FILE_ATTRIBUTES) {
        ShowHelpDialog(hwnd);
        HANDLE hTut = CreateFileA("kradio_tutorial.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hTut != INVALID_HANDLE_VALUE) {
            char d = '1';
            DWORD written = 0;
            WriteFile(hTut, &d, 1, &written, NULL);
            CloseHandle(hTut);
        }
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        // Intercept global hotkeys before standard edit control dispatching
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_F1) {
                ShowHelpDialog(hwnd);
                continue;
            }
            if (msg.wParam == VK_F5) {
                QuicksaveState(hwnd);
                continue;
            }
            if (msg.wParam == VK_F9) {
                QuickloadState(hwnd);
                continue;
            }
            // If focus is NOT the edit control, process general accelerators
            if (GetFocus() != hEditUrl) {
                if (msg.wParam >= '1' && msg.wParam <= '6') {
                    SelectPreset((int)(msg.wParam - '1'));
                    continue;
                }
                if (msg.wParam == VK_SPACE || msg.wParam == 'P' || msg.wParam == 'p') {
                    if (g_isPlaying) StopStream();
                    else PlayStream(hwnd);
                    continue;
                }
                if (msg.wParam == 'S' || msg.wParam == 's') {
                    StopStream();
                    continue;
                }
                if (msg.wParam == 'H' || msg.wParam == 'h') {
                    ShowHelpDialog(hwnd);
                    continue;
                }
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}

