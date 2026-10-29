#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <mmsystem.h>

#pragma function(memset, memcpy)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* p = (char*)dest;
    while (count--) *p++ = (char)c;
    return dest;
}

void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}

#define APP_TITLE "KSettings - System Settings & Control Panel [F1: Help | F5: Save | F9: Load]"
#define CLASS_NAME "KSettingsClass"
#define SAVE_FILE "ksettings_save.dat"
#define TUTORIAL_FILE "ksettings_tutorial.dat"
#define SAVE_SIG 0x5445534B /* 'KSET' */

#define ID_TAB_CTRL       100
#define ID_BTN_SAVE       101
#define ID_BTN_LOAD       102
#define ID_BTN_HELP       103
#define ID_BTN_RESET      104
#define ID_STATUS         105

// Personalization IDs
#define ID_THEME_BASE     200 // 200..205
#define ID_CHK_ANIM       210
#define ID_CHK_CONTRAST   211

// Audio IDs
#define ID_CHK_SOUND      300
#define ID_BTN_VOL_DOWN   301
#define ID_BTN_VOL_UP     302
#define ID_BTN_CHIME_1    310
#define ID_BTN_CHIME_2    311
#define ID_BTN_CHIME_3    312
#define ID_BTN_CHIME_4    313

// Display Mode IDs
#define ID_DISP_BASE      400 // 400..404

// Taskbar IDs
#define ID_TASKBAR_LIST   500
#define ID_BTN_PIN_TOGGLE 501

// System Specs IDs
#define ID_SPEC_TEXT      600

typedef struct {
    DWORD signature;
    int activeTab;
    int themeIndex;     // 0..5
    COLORREF accentColor;
    int soundEnabled;   // 0 or 1
    int soundVolume;    // 0..100
    int displayMode;    // 0: Standard, 1: CRT Scanlines, 2: Amber Mono, 3: Green Phosphor, 4: High Contrast
    int animations;     // 0 or 1
    int highContrast;   // 0 or 1
    DWORD pinnedMask;   // bitmask of pinned apps
    DWORD timestamp;
} SettingsState;

static SettingsState g_state;
static HWND g_hMain = NULL;
static HWND g_hTab = NULL;
static HWND g_hStatus = NULL;

// Theme Definitions
typedef struct {
    const char* name;
    COLORREF color;
} ThemePreset;

static const ThemePreset g_themes[6] = {
    { "Default Blue", RGB(33, 150, 243) },
    { "Classic Teal", RGB(0, 128, 128) },
    { "Matrix Phosphor", RGB(0, 230, 118) },
    { "Amber CRT", RGB(255, 145, 0) },
    { "Cyber Neon", RGB(224, 64, 251) },
    { "Midnight Ice", RGB(0, 229, 255) }
};

static const char* g_displayNames[5] = {
    "Standard 32-bit GUI",
    "Retro CRT Scanlines",
    "Amber CRT Monochrome",
    "Phosphor Green Terminal",
    "High Contrast Mono"
};

static const char* g_appList[8] = {
    "KPad (Notepad & Editor)",
    "KCalc (Scientific Calculator)",
    "KNet (Hypertext Browser)",
    "KTetris (Falling Blocks)",
    "KSnake (Arcade Duel)",
    "KMail (Virtual Messaging)",
    "KTodo (Task Tracker)",
    "KClock (World Clock & Alarms)"
};

// Child Controls
static HWND g_hwndsPersonal[12];
static int g_countPersonal = 0;

static HWND g_hwndsAudio[12];
static int g_countAudio = 0;

static HWND g_hwndsDisplay[10];
static int g_countDisplay = 0;

static HWND g_hwndsTaskbar[6];
static int g_countTaskbar = 0;

static HWND g_hwndsSpecs[6];
static int g_countSpecs = 0;

static HFONT g_hFontNormal = NULL;
static HFONT g_hFontBold = NULL;
static HFONT g_hFontMono = NULL;

static HBRUSH g_hBrushBg = NULL;
static HBRUSH g_hBrushCard = NULL;
static HBRUSH g_hBrushAccent = NULL;
static COLORREF g_colorText = RGB(249, 250, 251);

void ApplyDisplayBrushes(void) {
    if (g_hBrushBg) DeleteObject(g_hBrushBg);
    if (g_hBrushCard) DeleteObject(g_hBrushCard);
    if (g_hBrushAccent) DeleteObject(g_hBrushAccent);

    g_hBrushAccent = CreateSolidBrush(g_state.accentColor);

    if (g_state.displayMode == 1) { // CRT Scanlines mode
        g_hBrushBg = CreateSolidBrush(RGB(15, 23, 42));
        g_hBrushCard = CreateSolidBrush(RGB(30, 41, 59));
        g_colorText = RGB(226, 232, 240);
    } else if (g_state.displayMode == 2) { // Amber Monochrome
        g_hBrushBg = CreateSolidBrush(RGB(20, 10, 0));
        g_hBrushCard = CreateSolidBrush(RGB(38, 20, 0));
        g_colorText = RGB(255, 179, 0);
    } else if (g_state.displayMode == 3) { // Phosphor Green
        g_hBrushBg = CreateSolidBrush(RGB(0, 20, 8));
        g_hBrushCard = CreateSolidBrush(RGB(0, 36, 15));
        g_colorText = RGB(0, 255, 102);
    } else if (g_state.displayMode == 4) { // High Contrast
        g_hBrushBg = CreateSolidBrush(RGB(0, 0, 0));
        g_hBrushCard = CreateSolidBrush(RGB(10, 10, 10));
        g_colorText = RGB(255, 255, 255);
    } else { // Standard Modern 32-bit GUI
        g_hBrushBg = CreateSolidBrush(RGB(17, 24, 39));
        g_hBrushCard = CreateSolidBrush(RGB(31, 41, 55));
        g_colorText = RGB(249, 250, 251);
    }
}

void SetStatus(const char* text) {
    if (g_hStatus) {
        SetWindowTextA(g_hStatus, text);
    }
}

void PlayChime(int type) {
    if (!g_state.soundEnabled) return;
    switch (type) {
        case 0: // Startup arpeggio
            Beep(523, 80);
            Beep(659, 80);
            Beep(784, 80);
            Beep(1046, 120);
            break;
        case 1: // Notification Ping
            Beep(1046, 150);
            break;
        case 2: // Alert Beep
            Beep(440, 180);
            break;
        case 3: // Success Chord
            Beep(659, 90);
            Beep(784, 90);
            Beep(1046, 160);
            break;
    }
}

void RefreshVisibility(void) {
    int tab = g_state.activeTab;
    int i;
    for (i = 0; i < g_countPersonal; i++) ShowWindow(g_hwndsPersonal[i], tab == 0 ? SW_SHOW : SW_HIDE);
    for (i = 0; i < g_countAudio; i++) ShowWindow(g_hwndsAudio[i], tab == 1 ? SW_SHOW : SW_HIDE);
    for (i = 0; i < g_countDisplay; i++) ShowWindow(g_hwndsDisplay[i], tab == 2 ? SW_SHOW : SW_HIDE);
    for (i = 0; i < g_countTaskbar; i++) ShowWindow(g_hwndsTaskbar[i], tab == 3 ? SW_SHOW : SW_HIDE);
    for (i = 0; i < g_countSpecs; i++) ShowWindow(g_hwndsSpecs[i], tab == 4 ? SW_SHOW : SW_HIDE);

    ApplyDisplayBrushes();
    InvalidateRect(g_hMain, NULL, TRUE);
}

void UpdateTaskbarList(void) {
    HWND hList = NULL;
    int i;
    for (i = 0; i < g_countTaskbar; i++) {
        char cls[32] = {0};
        GetClassNameA(g_hwndsTaskbar[i], cls, sizeof(cls)-1);
        if (lstrcmpiA(cls, "LISTBOX") == 0) {
            hList = g_hwndsTaskbar[i];
            break;
        }
    }
    if (!hList) return;

    int sel = (int)SendMessageA(hList, LB_GETCURSEL, 0, 0);
    SendMessageA(hList, LB_RESETCONTENT, 0, 0);
    for (i = 0; i < 8; i++) {
        char itemBuf[80];
        int pinned = (g_state.pinnedMask & (1 << i)) != 0;
        wsprintfA(itemBuf, "[%s] %s", pinned ? "PINNED" : "   -   ", g_appList[i]);
        SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)itemBuf);
    }
    if (sel >= 0 && sel < 8) SendMessageA(hList, LB_SETCURSEL, (WPARAM)sel, 0);
    else SendMessageA(hList, LB_SETCURSEL, 0, 0);
}

void UpdateSpecsReadout(void) {
    HWND hSpecs = NULL;
    int i;
    for (i = 0; i < g_countSpecs; i++) {
        if (GetDlgCtrlID(g_hwndsSpecs[i]) == ID_SPEC_TEXT) {
            hSpecs = g_hwndsSpecs[i];
            break;
        }
    }
    if (!hSpecs) return;

    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);

    int scrW = GetSystemMetrics(SM_CXSCREEN);
    int scrH = GetSystemMetrics(SM_CYSCREEN);

    char buf[1024];
    wsprintfA(buf,
        "System Architecture: KiloOS Retro 1999 (v1.0.4 - Win32 Parity)\r\n"
        "Build Tag: 1999.12.31-RTM-GOLD\r\n"
        "Execution Core: Pure Win32 Native C Subsystem (MSVC /O1 /Os)\r\n"
        "Memory Pool: %lu MB Total Virtual RAM (%lu%% in use)\r\n"
        "Display Mode: %s\r\n"
        "Screen Resolution: %d x %d (32-bit True Color)\r\n"
        "Subnet Gateway: 10.19.99.1 [Carrier OK, 14.4 kHz Subcarrier]\r\n"
        "Boundary Constraint: < 999 KB Per-Application Verified\r\n"
        "VFS Persistence: State synchronized with /.sys_settings_*\r\n",
        (DWORD)(mem.ullTotalPhys / (1024 * 1024)),
        mem.dwMemoryLoad,
        g_displayNames[g_state.displayMode],
        scrW, scrH
    );
    SetWindowTextA(hSpecs, buf);
}

void QuicksaveState(void) {
    g_state.signature = SAVE_SIG;
    g_state.timestamp = GetTickCount();

    HANDLE hFile = CreateFileA(SAVE_FILE, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hFile, &g_state, sizeof(g_state), &written, NULL);
        CloseHandle(hFile);
        SetStatus("💾 Quicksaved settings state [F5]");
        PlayChime(3);
    } else {
        SetStatus("⚠️ Failed to write quicksave file!");
        MessageBeep(MB_ICONWARNING);
    }
}

void QuickloadState(void) {
    HANDLE hFile = CreateFileA(SAVE_FILE, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        SettingsState loaded;
        DWORD read = 0;
        BOOL ok = ReadFile(hFile, &loaded, sizeof(loaded), &read, NULL);
        CloseHandle(hFile);
        if (ok && read == sizeof(loaded) && loaded.signature == SAVE_SIG) {
            g_state = loaded;
            TabCtrl_SetCurSel(g_hTab, g_state.activeTab);
            RefreshVisibility();
            UpdateTaskbarList();
            UpdateSpecsReadout();
            SetStatus("📂 Quickloaded settings state [F9]");
            PlayChime(0);
            return;
        }
    }
    SetStatus("⚠️ No quicksaved state found (F5 to save)");
    MessageBeep(MB_ICONWARNING);
}

void ResetDefaults(void) {
    g_state.signature = SAVE_SIG;
    g_state.activeTab = 0;
    g_state.themeIndex = 0;
    g_state.accentColor = g_themes[0].color;
    g_state.soundEnabled = 1;
    g_state.soundVolume = 80;
    g_state.displayMode = 0;
    g_state.animations = 1;
    g_state.highContrast = 0;
    g_state.pinnedMask = 0x07; // Apps 0, 1, 2 pinned
    g_state.timestamp = GetTickCount();

    TabCtrl_SetCurSel(g_hTab, 0);
    RefreshVisibility();
    UpdateTaskbarList();
    UpdateSpecsReadout();
    SetStatus("🔄 Reset to factory defaults");
    PlayChime(1);
}

void ShowHelp(HWND hwnd) {
    MessageBoxA(hwnd,
        "KSettings - Control Panel & System Configuration\r\n\r\n"
        "Features & Options:\r\n"
        "• Personalization: 6 Retro Desktop Themes & live UI accent color\r\n"
        "• Sound & Audio: Master volume, mute toggle, and synthesizer chimes\r\n"
        "• Display Modes: 5 rendering profiles (Standard, CRT, Amber, Green, High Contrast)\r\n"
        "• Taskbar Shortcuts: Manage pinned applications\r\n"
        "• System Specs: Live CPU, memory pool, display resolution metrics\r\n\r\n"
        "Keyboard Shortcuts:\r\n"
        "• [F1] or [H]: Open this Help Guide\r\n"
        "• [F5]: Quicksave Full State Snapshot\r\n"
        "• [F9]: Quickload Full State Snapshot\r\n"
        "• [1] - [5]: Switch Navigation Tabs\r\n"
        "• [Esc]: Dismiss active dialog\r\n",
        "KSettings Feature & Shortcut Guide",
        MB_OK | MB_ICONINFORMATION
    );
}

void CheckFirstRunTutorial(HWND hwnd) {
    HANDLE hTut = CreateFileA(TUTORIAL_FILE, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hTut != INVALID_HANDLE_VALUE) {
        CloseHandle(hTut);
        return;
    }

    MessageBoxA(hwnd,
        "Welcome to KSettings!\r\n\r\n"
        "KSettings provides central control panel diagnostics and preferences for KiloOS:\r\n"
        "1. Personalization: Desktop accent styling and retro presets.\r\n"
        "2. Sound & Audio: FM synthesis earcon chimes and volume.\r\n"
        "3. Display Modes: Standard 32-bit GUI, Retro CRT, Amber & Green Phosphor.\r\n"
        "4. Taskbar Shortcuts: Configure pinned desktop applications.\r\n"
        "5. System Specs: Live hardware diagnostics and memory pool monitoring.\r\n\r\n"
        "Shortcuts: [F1] Help | [F5] Save | [F9] Load | [1-5] Tabs | [Esc] Dismiss",
        "Welcome to KSettings (1999 Control Panel)",
        MB_OK | MB_ICONINFORMATION
    );

    HANDLE hCreate = CreateFileA(TUTORIAL_FILE, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hCreate != INVALID_HANDLE_VALUE) {
        const char* msg = "KSettings Tutorial Seen\n";
        DWORD written = 0;
        WriteFile(hCreate, msg, (DWORD)lstrlenA(msg), &written, NULL);
        CloseHandle(hCreate);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            INITCOMMONCONTROLSEX icex;
            icex.dwSize = sizeof(icex);
            icex.dwICC = ICC_TAB_CLASSES | ICC_BAR_CLASSES;
            InitCommonControlsEx(&icex);

            // Fonts
            g_hFontNormal = CreateFontA(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            g_hFontBold = CreateFontA(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            g_hFontMono = CreateFontA(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");

            // Top Toolbar Buttons
            CreateWindowA("BUTTON", "💾 Save [F5]", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 12, 8, 85, 26, hwnd, (HMENU)ID_BTN_SAVE, NULL, NULL);
            CreateWindowA("BUTTON", "📂 Load [F9]", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 102, 8, 85, 26, hwnd, (HMENU)ID_BTN_LOAD, NULL, NULL);
            CreateWindowA("BUTTON", "🔄 Defaults", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 192, 8, 80, 26, hwnd, (HMENU)ID_BTN_RESET, NULL, NULL);
            CreateWindowA("BUTTON", "❓ Help [F1]", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 277, 8, 80, 26, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);

            // Tab Control
            g_hTab = CreateWindowA(WC_TABCONTROLA, "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_TABS, 12, 42, 656, 425, hwnd, (HMENU)ID_TAB_CTRL, NULL, NULL);
            SendMessageA(g_hTab, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            TCITEMA tie;
            tie.mask = TCIF_TEXT;
            tie.pszText = "🎨 Personalization [1]";
            TabCtrl_InsertItem(g_hTab, 0, &tie);
            tie.pszText = "🔊 Audio & Sound [2]";
            TabCtrl_InsertItem(g_hTab, 1, &tie);
            tie.pszText = "📺 Display Modes [3]";
            TabCtrl_InsertItem(g_hTab, 2, &tie);
            tie.pszText = "📌 Taskbar Shortcuts [4]";
            TabCtrl_InsertItem(g_hTab, 3, &tie);
            tie.pszText = "ℹ️ System Specs [5]";
            TabCtrl_InsertItem(g_hTab, 4, &tie);

            // 1. Personalization Controls (Parent = hwnd)
            g_hwndsPersonal[g_countPersonal++] = CreateWindowA("STATIC", "Desktop Wallpaper & Accent Themes:", WS_CHILD | SS_LEFT, 30, 85, 300, 18, hwnd, NULL, NULL, NULL);
            int i;
            for (i = 0; i < 6; i++) {
                int col = i % 3;
                int row = i / 3;
                g_hwndsPersonal[g_countPersonal++] = CreateWindowA("BUTTON", g_themes[i].name, WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 30 + col * 195, 110 + row * 34, 185, 28, hwnd, (HMENU)(INT_PTR)(ID_THEME_BASE + i), NULL, NULL);
            }
            g_hwndsPersonal[g_countPersonal++] = CreateWindowA("BUTTON", "Enable Dynamic UI Animations (Window & Desktop transitions)", WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX, 30, 195, 450, 24, hwnd, (HMENU)ID_CHK_ANIM, NULL, NULL);
            g_hwndsPersonal[g_countPersonal++] = CreateWindowA("BUTTON", "High Contrast Accent Boundaries (Enhanced legibility)", WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX, 30, 225, 450, 24, hwnd, (HMENU)ID_CHK_CONTRAST, NULL, NULL);
            g_hwndsPersonal[g_countPersonal++] = CreateWindowA("STATIC", "Live Accent Preview Box (Click any theme above to apply)", WS_CHILD | SS_CENTERIMAGE | SS_CENTER, 30, 265, 620, 180, hwnd, NULL, NULL, NULL);

            // 2. Audio & Sound Controls
            g_hwndsAudio[g_countAudio++] = CreateWindowA("BUTTON", "Enable Procedural FM Synthesizer Earcons & Bells", WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX, 30, 85, 400, 24, hwnd, (HMENU)ID_CHK_SOUND, NULL, NULL);
            g_hwndsAudio[g_countAudio++] = CreateWindowA("STATIC", "Master Audio Output Volume: 80%", WS_CHILD | SS_LEFT, 30, 125, 260, 20, hwnd, NULL, NULL, NULL);
            g_hwndsAudio[g_countAudio++] = CreateWindowA("BUTTON", "Vol -", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 300, 120, 60, 26, hwnd, (HMENU)ID_BTN_VOL_DOWN, NULL, NULL);
            g_hwndsAudio[g_countAudio++] = CreateWindowA("BUTTON", "Vol +", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 370, 120, 60, 26, hwnd, (HMENU)ID_BTN_VOL_UP, NULL, NULL);
            g_hwndsAudio[g_countAudio++] = CreateWindowA("STATIC", "Synthesizer Chime Diagnostics (Zero external WAV files):", WS_CHILD | SS_LEFT, 30, 175, 400, 20, hwnd, NULL, NULL, NULL);
            g_hwndsAudio[g_countAudio++] = CreateWindowA("BUTTON", "▶ Startup Chime (1999)", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 30, 205, 200, 32, hwnd, (HMENU)ID_BTN_CHIME_1, NULL, NULL);
            g_hwndsAudio[g_countAudio++] = CreateWindowA("BUTTON", "🔔 Notification Ping", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 240, 205, 190, 32, hwnd, (HMENU)ID_BTN_CHIME_2, NULL, NULL);
            g_hwndsAudio[g_countAudio++] = CreateWindowA("BUTTON", "⚠️ Alert Beep", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 440, 205, 190, 32, hwnd, (HMENU)ID_BTN_CHIME_3, NULL, NULL);
            g_hwndsAudio[g_countAudio++] = CreateWindowA("BUTTON", "✨ Victory Fanfare", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 30, 250, 200, 32, hwnd, (HMENU)ID_BTN_CHIME_4, NULL, NULL);

            // 3. Display Modes Controls
            g_hwndsDisplay[g_countDisplay++] = CreateWindowA("STATIC", "Select Monitor Output Rasterization & Screen Filter Profile:", WS_CHILD | SS_LEFT, 30, 85, 500, 20, hwnd, NULL, NULL, NULL);
            for (i = 0; i < 5; i++) {
                g_hwndsDisplay[g_countDisplay++] = CreateWindowA("BUTTON", g_displayNames[i], WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 30, 115 + i * 40, 320, 32, hwnd, (HMENU)(INT_PTR)(ID_DISP_BASE + i), NULL, NULL);
            }
            g_hwndsDisplay[g_countDisplay++] = CreateWindowA("STATIC", "Profile details:\r\n• Standard 32-bit: Clean high-res desktop rendering\r\n• Retro CRT: Phosphor raster simulation & micro-flicker\r\n• Amber Mono: Warm 1980s monochrome monitor\r\n• Green Phosphor: Terminal green mainframe screen\r\n• High Contrast: Stark black & white accessibility mode", WS_CHILD | SS_LEFT, 370, 115, 275, 190, hwnd, NULL, NULL, NULL);

            // 4. Taskbar Shortcuts Controls
            g_hwndsTaskbar[g_countTaskbar++] = CreateWindowA("STATIC", "Manage Shortcuts Pinned to the KiloOS Taskbar:", WS_CHILD | SS_LEFT, 30, 85, 400, 20, hwnd, NULL, NULL, NULL);
            g_hwndsTaskbar[g_countTaskbar++] = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_TABSTOP | WS_VSCROLL | LBS_NOTIFY, 30, 110, 480, 280, hwnd, (HMENU)ID_TASKBAR_LIST, NULL, NULL);
            g_hwndsTaskbar[g_countTaskbar++] = CreateWindowA("BUTTON", "📌 Toggle Pin / Unpin", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 520, 110, 130, 32, hwnd, (HMENU)ID_BTN_PIN_TOGGLE, NULL, NULL);

            // 5. System Specs Controls
            g_hwndsSpecs[g_countSpecs++] = CreateWindowA("STATIC", "Live KiloOS Environment Architecture & Telemetry:", WS_CHILD | SS_LEFT, 30, 85, 450, 20, hwnd, NULL, NULL, NULL);
            g_hwndsSpecs[g_countSpecs++] = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL, 30, 110, 620, 330, hwnd, (HMENU)ID_SPEC_TEXT, NULL, NULL);

            // Apply fonts
            for (i = 0; i < g_countPersonal; i++) SendMessageA(g_hwndsPersonal[i], WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            for (i = 0; i < g_countAudio; i++) SendMessageA(g_hwndsAudio[i], WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            for (i = 0; i < g_countDisplay; i++) SendMessageA(g_hwndsDisplay[i], WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            for (i = 0; i < g_countTaskbar; i++) SendMessageA(g_hwndsTaskbar[i], WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            for (i = 0; i < g_countSpecs; i++) SendMessageA(g_hwndsSpecs[i], WM_SETFONT, (WPARAM)(i == 1 ? g_hFontMono : g_hFontNormal), TRUE);

            // Status Bar
            g_hStatus = CreateWindowA("STATIC", "Ready (H/F1: Help, F5: Save, F9: Load, 1-5: Tabs)", WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_LEFT, 12, 475, 656, 22, hwnd, (HMENU)ID_STATUS, NULL, NULL);
            SendMessageA(g_hStatus, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            // Initialize State
            ResetDefaults();
            break;
        }

        case WM_NOTIFY: {
            NMHDR* pnm = (NMHDR*)lParam;
            if (pnm && pnm->idFrom == ID_TAB_CTRL && pnm->code == TCN_SELCHANGE) {
                g_state.activeTab = TabCtrl_GetCurSel(g_hTab);
                RefreshVisibility();
                UpdateSpecsReadout();
            }
            break;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == ID_BTN_SAVE) {
                QuicksaveState();
            } else if (id == ID_BTN_LOAD) {
                QuickloadState();
            } else if (id == ID_BTN_RESET) {
                ResetDefaults();
            } else if (id == ID_BTN_HELP) {
                ShowHelp(hwnd);
            } else if (id >= ID_THEME_BASE && id < ID_THEME_BASE + 6) {
                int idx = id - ID_THEME_BASE;
                g_state.themeIndex = idx;
                g_state.accentColor = g_themes[idx].color;
                ApplyDisplayBrushes();
                InvalidateRect(hwnd, NULL, TRUE);
                char statusMsg[64];
                wsprintfA(statusMsg, "Theme applied: %s", g_themes[idx].name);
                SetStatus(statusMsg);
                PlayChime(1);
            } else if (id == ID_CHK_ANIM) {
                g_state.animations = !g_state.animations;
                SendMessageA((HWND)lParam, BM_SETCHECK, g_state.animations ? BST_CHECKED : BST_UNCHECKED, 0);
            } else if (id == ID_CHK_CONTRAST) {
                g_state.highContrast = !g_state.highContrast;
                SendMessageA((HWND)lParam, BM_SETCHECK, g_state.highContrast ? BST_CHECKED : BST_UNCHECKED, 0);
            } else if (id == ID_CHK_SOUND) {
                g_state.soundEnabled = !g_state.soundEnabled;
                SendMessageA((HWND)lParam, BM_SETCHECK, g_state.soundEnabled ? BST_CHECKED : BST_UNCHECKED, 0);
                SetStatus(g_state.soundEnabled ? "🔊 System sounds enabled" : "🔇 System sounds muted");
            } else if (id == ID_BTN_VOL_DOWN) {
                if (g_state.soundVolume > 10) g_state.soundVolume -= 10;
                char volMsg[64];
                wsprintfA(volMsg, "Master Output Volume: %d%%", g_state.soundVolume);
                SetWindowTextA(g_hwndsAudio[1], volMsg);
                PlayChime(1);
            } else if (id == ID_BTN_VOL_UP) {
                if (g_state.soundVolume < 100) g_state.soundVolume += 10;
                char volMsg[64];
                wsprintfA(volMsg, "Master Output Volume: %d%%", g_state.soundVolume);
                SetWindowTextA(g_hwndsAudio[1], volMsg);
                PlayChime(1);
            } else if (id >= ID_BTN_CHIME_1 && id <= ID_BTN_CHIME_4) {
                PlayChime(id - ID_BTN_CHIME_1);
            } else if (id >= ID_DISP_BASE && id < ID_DISP_BASE + 5) {
                g_state.displayMode = id - ID_DISP_BASE;
                ApplyDisplayBrushes();
                UpdateSpecsReadout();
                InvalidateRect(hwnd, NULL, TRUE);
                char dispMsg[80];
                wsprintfA(dispMsg, "Display mode switched: %s", g_displayNames[g_state.displayMode]);
                SetStatus(dispMsg);
                PlayChime(3);
            } else if (id == ID_BTN_PIN_TOGGLE) {
                HWND hList = g_hwndsTaskbar[1];
                int sel = (int)SendMessageA(hList, LB_GETCURSEL, 0, 0);
                if (sel >= 0 && sel < 8) {
                    g_state.pinnedMask ^= (1 << sel);
                    UpdateTaskbarList();
                    PlayChime(1);
                }
            }
            break;
        }

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORDLG: {
            HDC hdc = (HDC)wParam;
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, g_colorText);
            return (LRESULT)g_hBrushBg;
        }

        case WM_KEYDOWN: {
            if (wParam == VK_F5) {
                QuicksaveState();
                return 0;
            } else if (wParam == VK_F9) {
                QuickloadState();
                return 0;
            } else if (wParam == VK_F1 || wParam == 'H' || wParam == 'h') {
                ShowHelp(hwnd);
                return 0;
            } else if (wParam >= '1' && wParam <= '5') {
                g_state.activeTab = (int)(wParam - '1');
                TabCtrl_SetCurSel(g_hTab, g_state.activeTab);
                RefreshVisibility();
                UpdateSpecsReadout();
                return 0;
            } else if (wParam == VK_ESCAPE) {
                SetStatus("KSettings Ready");
                return 0;
            }
            break;
        }

        case WM_DESTROY: {
            if (g_hFontNormal) DeleteObject(g_hFontNormal);
            if (g_hFontBold) DeleteObject(g_hFontBold);
            if (g_hFontMono) DeleteObject(g_hFontMono);
            if (g_hBrushBg) DeleteObject(g_hBrushBg);
            if (g_hBrushCard) DeleteObject(g_hBrushCard);
            if (g_hBrushAccent) DeleteObject(g_hBrushAccent);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry(void) {
    SetProcessDPIAware();
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hIcon = LoadIconA(wc.hInstance, MAKEINTRESOURCEA(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    RegisterClassA(&wc);

    RECT rc = {0, 0, 680, 510};
    AdjustWindowRect(&rc, (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX) | WS_CLIPCHILDREN, FALSE);

    g_hMain = CreateWindowExA(
        WS_EX_COMPOSITED,
        CLASS_NAME,
        APP_TITLE,
        (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX) | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        NULL, NULL, wc.hInstance, NULL
    );

    ShowWindow(g_hMain, SW_SHOW);
    UpdateWindow(g_hMain);

    CheckFirstRunTutorial(g_hMain);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_F5) {
                QuicksaveState();
                continue;
            } else if (msg.wParam == VK_F9) {
                QuickloadState();
                continue;
            } else if (msg.wParam == VK_F1) {
                ShowHelp(g_hMain);
                continue;
            } else if (msg.wParam >= '1' && msg.wParam <= '5') {
                g_state.activeTab = (int)(msg.wParam - '1');
                TabCtrl_SetCurSel(g_hTab, g_state.activeTab);
                RefreshVisibility();
                UpdateSpecsReadout();
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
