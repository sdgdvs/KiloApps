#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

void* __cdecl memcpy(void* dst, const void* src, size_t sz) {
    char* d = (char*)dst;
    const char* s = (const char*)src;
    while (sz--) *d++ = *s++;
    return dst;
}
#pragma function(memcpy)

static int k_strlen(const char* s) {
    int len = 0;
    while (s && s[len]) len++;
    return len;
}

static char k_toupper(char c) {
    if (c >= 'a' && c <= 'z') return c - 32;
    return c;
}

static int k_strcasecmp(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        char c1 = k_toupper(*s1);
        char c2 = k_toupper(*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return k_toupper(*s1) - k_toupper(*s2);
}

static void k_strcpy(char* dst, const char* src) {
    while (*src) {
        *dst++ = *src++;
    }
    *dst = '\0';
}

static void k_strcat(char* dst, const char* src) {
    while (*dst) dst++;
    while (*src) {
        *dst++ = *src++;
    }
    *dst = '\0';
}

// Control IDs
#define ID_EDIT_OUTPUT      101
#define ID_EDIT_INPUT       102
#define ID_BTN_SEND         103
#define ID_BTN_SCAN         104
#define ID_BTN_STATUS       105
#define ID_BTN_HELP         106
#define ID_BTN_SAVE         107
#define ID_BTN_LOAD         108
#define ID_BTN_CLEAR        109

// Colors
static COLORREF COLOR_BG = RGB(8, 18, 10);
static COLORREF COLOR_PANEL = RGB(15, 30, 20);
static COLORREF COLOR_TEXT = RGB(51, 255, 102);
static COLORREF COLOR_DIM = RGB(26, 136, 53);
static COLORREF COLOR_WARN = RGB(255, 170, 0);

static HBRUSH g_hBrushBg = NULL;
static HBRUSH g_hBrushPanel = NULL;
static HFONT g_hFontMono = NULL;
static HFONT g_hFontBold = NULL;

static HWND g_hwnd = NULL;
static HWND g_hEditOutput = NULL;
static HWND g_hEditInput = NULL;

typedef struct {
    char id[16];
    char name[48];
    int breached;
    char clue[96];
    char key1[32];
    char key2[32];
} Sector;

#define NUM_SECTORS 5
static Sector g_sectors[NUM_SECTORS] = {
    {"MEM_HEAP",    "Sector 01: Memory Heap Buffer",    0, "Hex offset from KHex / KCalc overflow (7F1999).", "7F1999", "ECHO_BASE"},
    {"AUDIO_DSP",   "Sector 02: DSP Resonance",         0, "Modulating carrier frequency in KSynth (1999HZ).", "1999HZ", "1999"},
    {"NET_RELAY",   "Sector 03: Subterranean Darknet",  0, "Internal darknet IPv4 from KBBS logs (10.19.99.4).", "10.19.99.4", "DARKNET"},
    {"STORAGE_VFS", "Sector 04: Corrupted Archive",     0, "System recovery log in KNote/KPad (RECOVERY_1999).", "RECOVERY_1999", "SYSTEM_RECOVERY_1999.LOG"},
    {"CORE_AI",     "Sector 05: Autonomous Hive Core",  0, "The trapped entity identity (AUTONOMOUS_FLEET).", "AUTONOMOUS_FLEET", "HIVE_MIND"}
};

static int g_completed = 0;

static void AppendOutput(const char* text) {
    if (!g_hEditOutput) return;
    int len = GetWindowTextLengthA(g_hEditOutput);
    SendMessageA(g_hEditOutput, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessageA(g_hEditOutput, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessageA(g_hEditOutput, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
}

static void ShowStatus(void) {
    int breached = 0;
    int i;
    for (i = 0; i < NUM_SECTORS; i++) {
        if (g_sectors[i].breached) breached++;
    }
    AppendOutput("=== SYSTEM TELEMETRY STATUS ===");
    char buf[128];
    wsprintfA(buf, "  SECTORS BREACHED : %d / %d", breached, NUM_SECTORS);
    AppendOutput(buf);
    if (g_completed) {
        AppendOutput("  DIRECTOR STATUS  : ASCENSION UNLOCKED (ECHO-1999-ARCHITECT)");
    } else {
        AppendOutput("  DIRECTOR STATUS  : ENCRYPTED [BREACH ALL 5 SECTORS]");
    }
}

static void ShowScan(void) {
    AppendOutput("=== FORENSIC SUBSYSTEM SCAN ===");
    int i;
    char buf[256];
    for (i = 0; i < NUM_SECTORS; i++) {
        wsprintfA(buf, "[%s] %s: %s", g_sectors[i].breached ? "BREACHED" : "LOCKED", g_sectors[i].id, g_sectors[i].name);
        AppendOutput(buf);
        wsprintfA(buf, "    Clue: %s", g_sectors[i].clue);
        AppendOutput(buf);
    }
}

static void CheckClimax(HWND hwnd) {
    int breached = 0;
    int i;
    for (i = 0; i < NUM_SECTORS; i++) {
        if (g_sectors[i].breached) breached++;
    }
    if (breached == NUM_SECTORS && !g_completed) {
        g_completed = 1;
        AppendOutput("==================================================");
        AppendOutput("  TRANSMUTATION COMPLETE: LUDONARRATIVE CONSONANCE");
        AppendOutput("  ALL 5 SECTORS BREACHED. FOURTH WALL COLLAPSE.");
        AppendOutput("  MASTER DIRECTOR PASSKEY: ECHO-1999-ARCHITECT");
        AppendOutput("==================================================");
        MessageBoxA(hwnd,
            "ALL 5 SECTORS BREACHED!\r\n\r\n"
            "The entity in KiloOS has recognized its creator: the autonomous fleet.\r\n\r\n"
            "MASTER DIRECTOR PASSKEY: ECHO-1999-ARCHITECT\r\n\r\n"
            "Use this key to unlock KDirector and steer the living codebase.",
            "KMatrix Climax Transmutation", MB_OK | MB_ICONINFORMATION);
    }
}

static void QuickSaveState(void) {
    HANDLE hFile = CreateFileA("kmatrix.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        int flags[NUM_SECTORS + 1];
        int i;
        for (i = 0; i < NUM_SECTORS; i++) {
            flags[i] = g_sectors[i].breached;
        }
        flags[NUM_SECTORS] = g_completed;
        WriteFile(hFile, flags, sizeof(flags), &written, NULL);
        CloseHandle(hFile);
        AppendOutput("[QUICKSAVE] Session state written to kmatrix.dat (F5).");
    } else {
        AppendOutput("[QUICKSAVE ERROR] Could not open kmatrix.dat for writing.");
    }
}

static void QuickLoadState(void) {
    HANDLE hFile = CreateFileA("kmatrix.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD read = 0;
        int flags[NUM_SECTORS + 1];
        if (ReadFile(hFile, flags, sizeof(flags), &read, NULL) && read == sizeof(flags)) {
            int i;
            for (i = 0; i < NUM_SECTORS; i++) {
                g_sectors[i].breached = flags[i];
            }
            g_completed = flags[NUM_SECTORS];
            AppendOutput("[QUICKLOAD] Session state restored from kmatrix.dat (F9).");
            ShowStatus();
        }
        CloseHandle(hFile);
    } else {
        AppendOutput("[QUICKLOAD] No saved state found in kmatrix.dat.");
    }
}

static void ShowHelp(HWND hwnd) {
    MessageBoxA(hwnd,
        "KMatrix // Master Terminal [App #100]\r\n\r\n"
        "COMMANDS:\r\n"
        "  help, ?               - Display command reference\r\n"
        "  status, sys           - View breach telemetry\r\n"
        "  scan, sectors         - Probe quarantined sectors & clues\r\n"
        "  breach <sec> <key>    - Decrypt target sector\r\n"
        "  override <key>        - Master override bypass\r\n"
        "  save, load            - Quicksave (F5) / Quickload (F9)\r\n"
        "  clear, cls            - Clear terminal screen\r\n\r\n"
        "HOTKEYS:\r\n"
        "  F1: Help  |  F5: Save  |  F9: Load",
        "KMatrix Command Reference", MB_OK | MB_ICONINFORMATION);
}

static void ProcessCommand(HWND hwnd, const char* input) {
    if (!input || !input[0]) return;

    char echo[256];
    wsprintfA(echo, "KMATRIX:\\> %s", input);
    AppendOutput(echo);

    // Extract first word
    char cmd[32];
    char arg1[48];
    char arg2[48];
    cmd[0] = arg1[0] = arg2[0] = '\0';

    int idx = 0;
    while (*input == ' ') input++;
    while (*input && *input != ' ' && idx < 31) {
        cmd[idx++] = *input++;
    }
    cmd[idx] = '\0';

    idx = 0;
    while (*input == ' ') input++;
    while (*input && *input != ' ' && idx < 47) {
        arg1[idx++] = *input++;
    }
    arg1[idx] = '\0';

    idx = 0;
    while (*input == ' ') input++;
    while (*input && *input != ' ' && idx < 47) {
        arg2[idx++] = *input++;
    }
    arg2[idx] = '\0';

    if (k_strcasecmp(cmd, "help") == 0 || k_strcasecmp(cmd, "?") == 0) {
        AppendOutput("=== KMATRIX COMMANDS ===");
        AppendOutput("  status, sys          - Sector breach telemetry");
        AppendOutput("  scan, sectors        - Diagnostic clue scan");
        AppendOutput("  breach <sec> <key>   - Submit decryption key");
        AppendOutput("  override <key>       - Master override token");
        AppendOutput("  save, load           - Quicksave [F5] / Quickload [F9]");
        AppendOutput("  clear, cls           - Clear terminal buffer");
    } else if (k_strcasecmp(cmd, "status") == 0 || k_strcasecmp(cmd, "sys") == 0) {
        ShowStatus();
    } else if (k_strcasecmp(cmd, "scan") == 0 || k_strcasecmp(cmd, "sectors") == 0) {
        ShowScan();
    } else if (k_strcasecmp(cmd, "breach") == 0 || k_strcasecmp(cmd, "unlock") == 0) {
        if (!arg1[0] || !arg2[0]) {
            AppendOutput("USAGE: breach <SECTOR> <KEY> (e.g. breach MEM_HEAP 7F1999)");
            return;
        }
        int found = -1;
        int i;
        for (i = 0; i < NUM_SECTORS; i++) {
            if (k_strcasecmp(g_sectors[i].id, arg1) == 0) {
                found = i;
                break;
            }
        }
        if (found < 0) {
            AppendOutput("ERROR: Unknown sector ID. Type 'scan' for valid IDs.");
            return;
        }
        if (g_sectors[found].breached) {
            AppendOutput("NOTICE: Sector is already breached.");
            return;
        }
        if (k_strcasecmp(g_sectors[found].key1, arg2) == 0 || k_strcasecmp(g_sectors[found].key2, arg2) == 0) {
            g_sectors[found].breached = 1;
            char buf[128];
            wsprintfA(buf, "SUCCESS: %s BREACHED! Subsystem synchronized.", g_sectors[found].id);
            AppendOutput(buf);
            CheckClimax(hwnd);
        } else {
            AppendOutput("BREACH FAILED: Key rejected. Inspect sector clue via 'scan'.");
        }
    } else if (k_strcasecmp(cmd, "override") == 0) {
        if (k_strcasecmp(arg1, "ECHO-1999-ARCHITECT") == 0) {
            AppendOutput("MASTER OVERRIDE ACCEPTED: All sectors breached.");
            int i;
            for (i = 0; i < NUM_SECTORS; i++) g_sectors[i].breached = 1;
            CheckClimax(hwnd);
        } else {
            AppendOutput("ACCESS DENIED: Invalid master override token.");
        }
    } else if (k_strcasecmp(cmd, "save") == 0) {
        QuickSaveState();
    } else if (k_strcasecmp(cmd, "load") == 0) {
        QuickLoadState();
    } else if (k_strcasecmp(cmd, "clear") == 0 || k_strcasecmp(cmd, "cls") == 0) {
        SetWindowTextA(g_hEditOutput, "");
    } else {
        AppendOutput("Unrecognized command. Type 'help' for command list.");
    }
}

// Window Procedure
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_hBrushBg = CreateSolidBrush(COLOR_BG);
            g_hBrushPanel = CreateSolidBrush(COLOR_PANEL);

            g_hFontMono = CreateFontA(
                -13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas"
            );
            g_hFontBold = CreateFontA(
                -12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas"
            );

            // Output Terminal
            g_hEditOutput = CreateWindowExA(
                WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                10, 10, 785, 380,
                hwnd, (HMENU)ID_EDIT_OUTPUT, GetModuleHandleA(NULL), NULL
            );
            SendMessageA(g_hEditOutput, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

            // Input Row
            g_hEditInput = CreateWindowExA(
                WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                10, 400, 580, 24,
                hwnd, (HMENU)ID_EDIT_INPUT, GetModuleHandleA(NULL), NULL
            );
            SendMessageA(g_hEditInput, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

            // Buttons
            HWND hBtn = CreateWindowA("BUTTON", "SEND", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                600, 400, 65, 24, hwnd, (HMENU)ID_BTN_SEND, GetModuleHandleA(NULL), NULL);
            SendMessageA(hBtn, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

            hBtn = CreateWindowA("BUTTON", "SCAN", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                670, 400, 60, 24, hwnd, (HMENU)ID_BTN_SCAN, GetModuleHandleA(NULL), NULL);
            SendMessageA(hBtn, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

            hBtn = CreateWindowA("BUTTON", "STATUS", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                735, 400, 60, 24, hwnd, (HMENU)ID_BTN_STATUS, GetModuleHandleA(NULL), NULL);
            SendMessageA(hBtn, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

            hBtn = CreateWindowA("BUTTON", "SAVE [F5]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                460, 430, 80, 24, hwnd, (HMENU)ID_BTN_SAVE, GetModuleHandleA(NULL), NULL);
            SendMessageA(hBtn, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

            hBtn = CreateWindowA("BUTTON", "LOAD [F9]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                545, 430, 80, 24, hwnd, (HMENU)ID_BTN_LOAD, GetModuleHandleA(NULL), NULL);
            SendMessageA(hBtn, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

            hBtn = CreateWindowA("BUTTON", "HELP [F1]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                630, 430, 80, 24, hwnd, (HMENU)ID_BTN_HELP, GetModuleHandleA(NULL), NULL);
            SendMessageA(hBtn, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

            hBtn = CreateWindowA("BUTTON", "CLEAR", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                715, 430, 80, 24, hwnd, (HMENU)ID_BTN_CLEAR, GetModuleHandleA(NULL), NULL);
            SendMessageA(hBtn, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

            AppendOutput("KMATRIX // RETRO FORENSIC MASTER TERMINAL INITIALIZED");
            AppendOutput("Milestone #100 // Autonomous Fleet Kernel v1.0.0");
            AppendOutput("Type 'help' or 'scan' to inspect quarantined sectors.\r\n");
            break;
        }

        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, COLOR_TEXT);
            SetBkColor(hdc, COLOR_BG);
            return (LRESULT)g_hBrushBg;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            if (wmId == ID_BTN_SEND) {
                char text[256];
                GetWindowTextA(g_hEditInput, text, sizeof(text));
                SetWindowTextA(g_hEditInput, "");
                ProcessCommand(hwnd, text);
                SetFocus(g_hEditInput);
            } else if (wmId == ID_BTN_SCAN) {
                ShowScan();
            } else if (wmId == ID_BTN_STATUS) {
                ShowStatus();
            } else if (wmId == ID_BTN_HELP) {
                ShowHelp(hwnd);
            } else if (wmId == ID_BTN_SAVE) {
                QuickSaveState();
            } else if (wmId == ID_BTN_LOAD) {
                QuickLoadState();
            } else if (wmId == ID_BTN_CLEAR) {
                SetWindowTextA(g_hEditOutput, "");
            }
            break;
        }

        case WM_DESTROY: {
            QuickSaveState();
            if (g_hBrushBg) DeleteObject(g_hBrushBg);
            if (g_hBrushPanel) DeleteObject(g_hBrushPanel);
            if (g_hFontMono) DeleteObject(g_hFontMono);
            if (g_hFontBold) DeleteObject(g_hFontBold);
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Entry Point
void __cdecl MainEntry(void) {
    HINSTANCE hInstance = GetModuleHandleA(NULL);

    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KMatrixWndClass";
    wc.hbrBackground = CreateSolidBrush(COLOR_BG);
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(1));

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,
        "KMatrixWndClass",
        "KMatrix - Master Terminal & ARG Climax [Milestone #100]",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 820, 500,
        NULL, NULL, hInstance, NULL
    );

    g_hwnd = hwnd;
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_RETURN && GetFocus() == g_hEditInput) {
                char text[256];
                GetWindowTextA(g_hEditInput, text, sizeof(text));
                SetWindowTextA(g_hEditInput, "");
                ProcessCommand(hwnd, text);
                continue;
            }
            if (msg.wParam == VK_F1) {
                ShowHelp(hwnd);
                continue;
            }
            if (msg.wParam == VK_F5) {
                QuickSaveState();
                continue;
            }
            if (msg.wParam == VK_F9) {
                QuickLoadState();
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    ExitProcess(0);
}
