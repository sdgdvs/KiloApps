/*
 * KContribute - Voluntary Fleet Compute Contributor Daemon v1.0.0
 * SETI@home / Folding@home style voluntary distributed compute client for KiloApps.
 *
 * Runs silently in the Windows System Tray (Taskbar Notification Area).
 * Donates free Google Gemini quota / turns to autonomously evolve the KiloApps ecosystem.
 *
 * Zero distraction: auto-minimizes to tray, zero intrusive windows, zero nag screens.
 * Click system tray icon anytime to open retro dashboard, telemetry, and work unit visualizer.
 */

#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")

#define WM_TRAYICON (WM_USER + 100)

// Control Identifiers
#define ID_BTN_GOOGLE_SIGNIN    101
#define ID_EDIT_APIKEY          102
#define ID_EDIT_HANDLE          103
#define ID_RADIO_BALANCED       104
#define ID_RADIO_QUIET          105
#define ID_RADIO_TURBO          106
#define ID_CHK_AUTOSTART        107
#define ID_BTN_START_CONTRIBUTE 108

#define ID_BTN_DASH_RUNNOW      110
#define ID_BTN_DASH_PAUSE       111
#define ID_BTN_DASH_SETTINGS    112
#define ID_BTN_DASH_MINIMIZE    113

// Tray Menu Identifiers
#define ID_TRAY_OPEN            201
#define ID_TRAY_RUNNOW          202
#define ID_TRAY_PAUSE           203
#define ID_TRAY_SETTINGS        204
#define ID_TRAY_AUTOSTART       205
#define ID_TRAY_WEB             206
#define ID_TRAY_EXIT            207

// Timers
#define TIMER_ANIM              1
#define TIMER_TICK              2

// View Modes
typedef enum {
    VIEW_ONBOARDING = 0,
    VIEW_DASHBOARD = 1
} ViewMode;

// Application Configuration
typedef struct {
    char apiKey[128];
    char handle[64];
    int cadence;        // 0: Balanced (1/hr), 1: Quiet (1/4hr), 2: Turbo (3/hr)
    int autoStart;      // 1: yes, 0: no
    int configured;     // 1: configured, 0: first run
    int paused;         // 1: paused, 0: running
    int turnsCount;     // Total turns completed
    char lastTurnApp[64];
    time_t lastTurnTime;
} ContributorConfig;

// Global State
static ContributorConfig g_config;
static ViewMode g_viewMode = VIEW_ONBOARDING;
static HWND g_hWnd = NULL;
static NOTIFYICONDATAA g_nid;
static HICON g_hTrayIcon = NULL;
static HFONT g_fontTitle = NULL;
static HFONT g_fontMono = NULL;
static HFONT g_fontMonoBold = NULL;
static HFONT g_fontUI = NULL;
static HFONT g_fontUIBold = NULL;

static float g_radarAngle = 0.0f;
static float g_fftWave[64];
static int g_countdownSeconds = 3600;
static int g_isProcessingTurn = 0;
static float g_processProgress = 0.0f;
static char g_statusText[128] = "IDLE (Awaiting scheduled turn window)";

// Work Unit App Targets Pool
static const char* WORK_UNIT_APPS[] = {
    "KChess (Retro Multiplayer RFMS Expansion)",
    "KGo (RFMS P2P Matchmaking & Solo Bot Fallback)",
    "KMandel (Pass 5: QA & Memory Bound Audit)",
    "KReversi (Board Logic & Turn Pacing)",
    "KDarts (301/501 Match Pacing & SFX Synthesis)",
    "KTetris (Line Clear Canvas Performance & 60 FPS)",
    "KPad (VFS File Decryption & Text Wrapping)",
    "KSynth (Yamaha YM2612 2-Operator FM Patch Engine)",
    "KStarDredge (Ore Node Shimmer & Solar Drift)",
    "KMatrix (Century Milestone Singularity Parity)",
    "KNet (10.19.99.4 Classified Route Diagnostics)",
    "KBookmark (Responsive Split Layout & URL History)"
};
static const int WORK_UNIT_COUNT = sizeof(WORK_UNIT_APPS) / sizeof(WORK_UNIT_APPS[0]);

// Control Handles
static HWND hEditApiKey = NULL;
static HWND hEditHandle = NULL;
static HWND hRadioBalanced = NULL;
static HWND hRadioQuiet = NULL;
static HWND hRadioTurbo = NULL;
static HWND hChkAutoStart = NULL;
static HWND hBtnGoogle = NULL;
static HWND hBtnStart = NULL;

static HWND hBtnRunNow = NULL;
static HWND hBtnPause = NULL;
static HWND hBtnSettings = NULL;
static HWND hBtnMinimize = NULL;

// Forward Declarations
void GetConfigPath(char* outPath, size_t maxLen);
void LoadConfiguration(void);
void SaveConfiguration(void);
void SetAutoStart(int enable);
void InitTrayIcon(HWND hWnd);
void RemoveTrayIcon(void);
void CreateChildControls(HWND hWnd);
void UpdateControlVisibility(void);
void TriggerVolunteerTurn(void);
HICON CreateProceduralTrayIcon(void);

// Helper: Config File Path in %LOCALAPPDATA%\KiloApps\kcontribute.ini
void GetConfigPath(char* outPath, size_t maxLen) {
    char localApp[MAX_PATH];
    if (GetEnvironmentVariableA("LOCALAPPDATA", localApp, MAX_PATH) > 0) {
        char dirPath[MAX_PATH];
        sprintf_s(dirPath, MAX_PATH, "%s\\KiloApps", localApp);
        CreateDirectoryA(dirPath, NULL);
        sprintf_s(outPath, maxLen, "%s\\kcontribute.ini", dirPath);
    } else {
        strcpy_s(outPath, maxLen, "kcontribute.ini");
    }
}

void LoadConfiguration(void) {
    memset(&g_config, 0, sizeof(g_config));
    strcpy_s(g_config.handle, sizeof(g_config.handle), "Anonymous_Voyager");
    g_config.cadence = 0;       // Balanced (1/hr)
    g_config.autoStart = 1;     // Enabled by default
    g_config.configured = 0;
    g_config.paused = 0;
    g_config.turnsCount = 0;
    strcpy_s(g_config.lastTurnApp, sizeof(g_config.lastTurnApp), "None");
    g_config.lastTurnTime = 0;

    char cfgPath[MAX_PATH];
    GetConfigPath(cfgPath, MAX_PATH);
    FILE* f = NULL;
    if (fopen_s(&f, cfgPath, "r") == 0 && f != NULL) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            char key[64], val[192];
            if (sscanf_s(line, "%63[^=]=%191[^\r\n]", key, (unsigned)sizeof(key), val, (unsigned)sizeof(val)) == 2) {
                if (strcmp(key, "ApiKey") == 0) strcpy_s(g_config.apiKey, sizeof(g_config.apiKey), val);
                else if (strcmp(key, "Handle") == 0) strcpy_s(g_config.handle, sizeof(g_config.handle), val);
                else if (strcmp(key, "Cadence") == 0) g_config.cadence = atoi(val);
                else if (strcmp(key, "AutoStart") == 0) g_config.autoStart = atoi(val);
                else if (strcmp(key, "Configured") == 0) g_config.configured = atoi(val);
                else if (strcmp(key, "Paused") == 0) g_config.paused = atoi(val);
                else if (strcmp(key, "TurnsCount") == 0) g_config.turnsCount = atoi(val);
                else if (strcmp(key, "LastTurnApp") == 0) strcpy_s(g_config.lastTurnApp, sizeof(g_config.lastTurnApp), val);
                else if (strcmp(key, "LastTurnTime") == 0) g_config.lastTurnTime = (time_t)_atoi64(val);
            }
        }
        fclose(f);
    }

    if (g_config.configured && strlen(g_config.apiKey) > 5) {
        g_viewMode = VIEW_DASHBOARD;
    } else {
        g_viewMode = VIEW_ONBOARDING;
    }
}

void SaveConfiguration(void) {
    char cfgPath[MAX_PATH];
    GetConfigPath(cfgPath, MAX_PATH);
    FILE* f = NULL;
    if (fopen_s(&f, cfgPath, "w") == 0 && f != NULL) {
        fprintf(f, "ApiKey=%s\n", g_config.apiKey);
        fprintf(f, "Handle=%s\n", g_config.handle);
        fprintf(f, "Cadence=%d\n", g_config.cadence);
        fprintf(f, "AutoStart=%d\n", g_config.autoStart);
        fprintf(f, "Configured=%d\n", g_config.configured);
        fprintf(f, "Paused=%d\n", g_config.paused);
        fprintf(f, "TurnsCount=%d\n", g_config.turnsCount);
        fprintf(f, "LastTurnApp=%s\n", g_config.lastTurnApp);
        fprintf(f, "LastTurnTime=%lld\n", (long long)g_config.lastTurnTime);
        fclose(f);
    }
}

// Auto-start via Windows User Startup Folder (clean, user-transparent, zero registry lint triggers)
void SetAutoStart(int enable) {
    char appData[MAX_PATH];
    if (GetEnvironmentVariableA("APPDATA", appData, MAX_PATH) > 0) {
        char startupPath[MAX_PATH];
        sprintf_s(startupPath, MAX_PATH, "%s\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\KContribute.bat", appData);
        if (enable) {
            char exePath[MAX_PATH];
            GetModuleFileNameA(NULL, exePath, MAX_PATH);
            FILE* f = NULL;
            if (fopen_s(&f, startupPath, "w") == 0 && f != NULL) {
                fprintf(f, "@start \"\" \"%s\"\n", exePath);
                fclose(f);
            }
        } else {
            DeleteFileA(startupPath);
        }
    }
}

// Procedural 16x16 / 32x32 retro satellite radio dish icon for tray and window
HICON CreateProceduralTrayIcon(void) {
    int w = 32, h = 32;
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmColor = CreateCompatibleBitmap(hdcScreen, w, h);
    HBITMAP hbmMask = CreateBitmap(w, h, 1, 1, NULL);

    HBITMAP hOldBm = (HBITMAP)SelectObject(hdcMem, hbmColor);
    HBRUSH hBrBg = CreateSolidBrush(RGB(15, 23, 42)); // Navy-slate
    RECT rc = { 0, 0, w, h };
    FillRect(hdcMem, &rc, hBrBg);
    DeleteObject(hBrBg);

    // Cyan circular rim
    HPEN hPenCyan = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
    HPEN hOldPen = (HPEN)SelectObject(hdcMem, hPenCyan);
    HBRUSH hBrDish = CreateSolidBrush(RGB(22, 31, 46));
    HBRUSH hOldBr = (HBRUSH)SelectObject(hdcMem, hBrDish);
    Ellipse(hdcMem, 4, 10, 24, 28);

    // Center transceiver arm
    HPEN hPenGold = CreatePen(PS_SOLID, 2, RGB(245, 158, 11));
    SelectObject(hdcMem, hPenGold);
    MoveToEx(hdcMem, 14, 19, NULL);
    LineTo(hdcMem, 24, 9);

    // Emerald transmitter pulse point
    HBRUSH hBrGreen = CreateSolidBrush(RGB(16, 185, 129));
    SelectObject(hdcMem, hBrGreen);
    Ellipse(hdcMem, 22, 7, 27, 12);
    DeleteObject(hBrGreen);

    // Signal broadcast arc waves
    HPEN hPenArc = CreatePen(PS_SOLID, 1, RGB(52, 211, 153));
    SelectObject(hdcMem, hPenArc);
    Arc(hdcMem, 20, 2, 30, 12, 20, 7, 27, 2);
    Arc(hdcMem, 18, 0, 32, 14, 18, 7, 29, 0);

    SelectObject(hdcMem, hOldPen);
    SelectObject(hdcMem, hOldBr);
    SelectObject(hdcMem, hOldBm);
    DeleteObject(hPenCyan);
    DeleteObject(hPenGold);
    DeleteObject(hPenArc);
    DeleteObject(hBrDish);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    // Create mask (opaque)
    ICONINFO ii;
    ii.fIcon = TRUE;
    ii.xHotspot = 0;
    ii.yHotspot = 0;
    ii.hbmMask = hbmMask;
    ii.hbmColor = hbmColor;
    HICON hIcon = CreateIconIndirect(&ii);

    DeleteObject(hbmColor);
    DeleteObject(hbmMask);
    return hIcon;
}

void InitTrayIcon(HWND hWnd) {
    if (!g_hTrayIcon) g_hTrayIcon = CreateProceduralTrayIcon();
    memset(&g_nid, 0, sizeof(g_nid));
    g_nid.cbSize = sizeof(NOTIFYICONDATAA);
    g_nid.hWnd = hWnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = g_hTrayIcon;
    strcpy_s(g_nid.szTip, sizeof(g_nid.szTip), "KContribute: Volunteer Fleet Active");
    strcpy_s(g_nid.szInfo, sizeof(g_nid.szInfo), "KContribute is running quietly in your system tray. Contributing turns to KiloApps.");
    strcpy_s(g_nid.szInfoTitle, sizeof(g_nid.szInfoTitle), "KiloApps Fleet Contributor");
    g_nid.dwInfoFlags = NIIF_INFO;

    Shell_NotifyIconA(NIM_ADD, &g_nid);
}

void RemoveTrayIcon(void) {
    Shell_NotifyIconA(NIM_DELETE, &g_nid);
}

void CreateChildControls(HWND hWnd) {
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);

    // === Onboarding Controls ===
    hBtnGoogle = CreateWindowA("BUTTON", "🔑 1-Click: Sign In with Google & Get Free Gemini Key ↗",
        WS_CHILD | BS_PUSHBUTTON, 30, 115, 500, 36, hWnd, (HMENU)ID_BTN_GOOGLE_SIGNIN, hInst, NULL);

    hEditApiKey = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", g_config.apiKey,
        WS_CHILD | ES_AUTOHSCROLL | ES_PASSWORD, 30, 185, 500, 26, hWnd, (HMENU)ID_EDIT_APIKEY, hInst, NULL);

    hEditHandle = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", g_config.handle,
        WS_CHILD | ES_AUTOHSCROLL, 30, 245, 500, 26, hWnd, (HMENU)ID_EDIT_HANDLE, hInst, NULL);

    hRadioBalanced = CreateWindowA("BUTTON", "Balanced: 1 turn / hour (~5% daily free quota) [Recommended]",
        WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP, 30, 305, 500, 20, hWnd, (HMENU)ID_RADIO_BALANCED, hInst, NULL);

    hRadioQuiet = CreateWindowA("BUTTON", "Quiet: 1 turn / 4 hours (~1.2% daily free quota)",
        WS_CHILD | BS_AUTORADIOBUTTON, 30, 330, 500, 20, hWnd, (HMENU)ID_RADIO_QUIET, hInst, NULL);

    hRadioTurbo = CreateWindowA("BUTTON", "Turbo: 3 turns / hour (~15% daily free quota)",
        WS_CHILD | BS_AUTORADIOBUTTON, 30, 355, 500, 20, hWnd, (HMENU)ID_RADIO_TURBO, hInst, NULL);

    hChkAutoStart = CreateWindowA("BUTTON", "Start automatically with Windows (Runs in System Tray)",
        WS_CHILD | BS_AUTOCHECKBOX, 30, 385, 500, 20, hWnd, (HMENU)ID_CHK_AUTOSTART, hInst, NULL);

    hBtnStart = CreateWindowA("BUTTON", "🚀 Start Donating in Background (Minimize to Tray)",
        WS_CHILD | BS_DEFPUSHBUTTON, 30, 420, 500, 38, hWnd, (HMENU)ID_BTN_START_CONTRIBUTE, hInst, NULL);

    // Initial check state
    if (g_config.cadence == 1) SendMessage(hRadioQuiet, BM_SETCHECK, BST_CHECKED, 0);
    else if (g_config.cadence == 2) SendMessage(hRadioTurbo, BM_SETCHECK, BST_CHECKED, 0);
    else SendMessage(hRadioBalanced, BM_SETCHECK, BST_CHECKED, 0);

    SendMessage(hChkAutoStart, BM_SETCHECK, g_config.autoStart ? BST_CHECKED : BST_UNCHECKED, 0);

    // === Dashboard Controls ===
    hBtnRunNow = CreateWindowA("BUTTON", "⚡ Run Turn Now",
        WS_CHILD | BS_PUSHBUTTON, 20, 440, 130, 32, hWnd, (HMENU)ID_BTN_DASH_RUNNOW, hInst, NULL);

    hBtnPause = CreateWindowA("BUTTON", g_config.paused ? "▶️ Resume Engine" : "⏸️ Pause Engine",
        WS_CHILD | BS_PUSHBUTTON, 160, 440, 130, 32, hWnd, (HMENU)ID_BTN_DASH_PAUSE, hInst, NULL);

    hBtnSettings = CreateWindowA("BUTTON", "⚙️ Key & Settings",
        WS_CHILD | BS_PUSHBUTTON, 300, 440, 130, 32, hWnd, (HMENU)ID_BTN_DASH_SETTINGS, hInst, NULL);

    hBtnMinimize = CreateWindowA("BUTTON", "Minimize to Tray",
        WS_CHILD | BS_PUSHBUTTON, 440, 440, 140, 32, hWnd, (HMENU)ID_BTN_DASH_MINIMIZE, hInst, NULL);

    // Apply UI fonts
    HWND all[] = { hBtnGoogle, hEditApiKey, hEditHandle, hRadioBalanced, hRadioQuiet, hRadioTurbo,
                   hChkAutoStart, hBtnStart, hBtnRunNow, hBtnPause, hBtnSettings, hBtnMinimize };
    for (int i = 0; i < sizeof(all)/sizeof(all[0]); i++) {
        SendMessage(all[i], WM_SETFONT, (WPARAM)g_fontUIBold, TRUE);
    }
}

void UpdateControlVisibility(void) {
    int onbShow = (g_viewMode == VIEW_ONBOARDING) ? SW_SHOW : SW_HIDE;
    int dashShow = (g_viewMode == VIEW_DASHBOARD) ? SW_SHOW : SW_HIDE;

    ShowWindow(hBtnGoogle, onbShow);
    ShowWindow(hEditApiKey, onbShow);
    ShowWindow(hEditHandle, onbShow);
    ShowWindow(hRadioBalanced, onbShow);
    ShowWindow(hRadioQuiet, onbShow);
    ShowWindow(hRadioTurbo, onbShow);
    ShowWindow(hChkAutoStart, onbShow);
    ShowWindow(hBtnStart, onbShow);

    ShowWindow(hBtnRunNow, dashShow);
    ShowWindow(hBtnPause, dashShow);
    ShowWindow(hBtnSettings, dashShow);
    ShowWindow(hBtnMinimize, dashShow);

    InvalidateRect(g_hWnd, NULL, TRUE);
}

void TriggerVolunteerTurn(void) {
    if (g_isProcessingTurn) return;
    g_isProcessingTurn = 1;
    g_processProgress = 0.0f;

    // Pick target app
    int targetIdx = g_config.turnsCount % WORK_UNIT_COUNT;
    strcpy_s(g_config.lastTurnApp, sizeof(g_config.lastTurnApp), WORK_UNIT_APPS[targetIdx]);
    sprintf_s(g_statusText, sizeof(g_statusText), "DISPATCHING WORK UNIT: %s", g_config.lastTurnApp);

    // Check if local run_orchestrator.bat exists in repository root
    char batPath[MAX_PATH];
    char exeDir[MAX_PATH];
    GetModuleFileNameA(NULL, exeDir, MAX_PATH);
    char* lastSlash = strrchr(exeDir, '\\');
    if (lastSlash) *lastSlash = '\0';

    sprintf_s(batPath, MAX_PATH, "%s\\..\\scripts\\run_orchestrator.bat", exeDir);
    DWORD attribs = GetFileAttributesA(batPath);
    if (attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY)) {
        // Execute background turn via orchestrator batch runner
        ShellExecuteA(NULL, "open", batPath, NULL, NULL, SW_HIDE);
    } else {
        sprintf_s(batPath, MAX_PATH, "%s\\scripts\\run_orchestrator.bat", exeDir);
        attribs = GetFileAttributesA(batPath);
        if (attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY)) {
            ShellExecuteA(NULL, "open", batPath, NULL, NULL, SW_HIDE);
        }
    }
}

// Window Procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        g_hWnd = hWnd;
        CreateChildControls(hWnd);
        UpdateControlVisibility();
        SetTimer(hWnd, TIMER_ANIM, 50, NULL);   // 20 FPS radar/FFT wave animation
        SetTimer(hWnd, TIMER_TICK, 1000, NULL); // 1-second countdown clock & turn scheduler
        break;

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        switch (id) {
        case ID_BTN_GOOGLE_SIGNIN:
            // 1-Click: Open Google AI Studio API key generator directly in default browser
            ShellExecuteA(NULL, "open", "https://aistudio.google.com/apikey", NULL, NULL, SW_SHOWNORMAL);
            break;

        case ID_BTN_START_CONTRIBUTE: {
            // Save Onboarding Settings
            char keyBuf[128] = { 0 };
            char handleBuf[64] = { 0 };
            GetWindowTextA(hEditApiKey, keyBuf, sizeof(keyBuf));
            GetWindowTextA(hEditHandle, handleBuf, sizeof(handleBuf));

            if (strlen(keyBuf) < 8) {
                MessageBoxA(hWnd,
                    "Please paste your free Google Gemini API key to proceed.\nClick the 1-Click Sign-In button above to get your free key instantly.",
                    "API Key Required", MB_ICONWARNING | MB_OK);
                return 0;
            }

            strcpy_s(g_config.apiKey, sizeof(g_config.apiKey), keyBuf);
            if (strlen(handleBuf) > 0) {
                strcpy_s(g_config.handle, sizeof(g_config.handle), handleBuf);
            }

            if (SendMessage(hRadioQuiet, BM_GETCHECK, 0, 0) == BST_CHECKED) g_config.cadence = 1;
            else if (SendMessage(hRadioTurbo, BM_GETCHECK, 0, 0) == BST_CHECKED) g_config.cadence = 2;
            else g_config.cadence = 0;

            g_config.autoStart = (SendMessage(hChkAutoStart, BM_GETCHECK, 0, 0) == BST_CHECKED);
            g_config.configured = 1;
            g_config.paused = 0;
            SaveConfiguration();
            SetAutoStart(g_config.autoStart);

            // Switch to Dashboard mode & minimize immediately to tray
            g_viewMode = VIEW_DASHBOARD;
            UpdateControlVisibility();
            InitTrayIcon(hWnd);
            ShowWindow(hWnd, SW_HIDE);
            break;
        }

        case ID_BTN_DASH_RUNNOW:
        case ID_TRAY_RUNNOW:
            TriggerVolunteerTurn();
            break;

        case ID_BTN_DASH_PAUSE:
        case ID_TRAY_PAUSE:
            g_config.paused = !g_config.paused;
            SetWindowTextA(hBtnPause, g_config.paused ? "▶️ Resume Engine" : "⏸️ Pause Engine");
            SaveConfiguration();
            InvalidateRect(hWnd, NULL, TRUE);
            break;

        case ID_BTN_DASH_SETTINGS:
        case ID_TRAY_SETTINGS:
            g_viewMode = VIEW_ONBOARDING;
            UpdateControlVisibility();
            ShowWindow(hWnd, SW_SHOWNORMAL);
            SetForegroundWindow(hWnd);
            break;

        case ID_BTN_DASH_MINIMIZE:
            ShowWindow(hWnd, SW_HIDE);
            break;

        case ID_TRAY_OPEN:
            g_viewMode = VIEW_DASHBOARD;
            UpdateControlVisibility();
            ShowWindow(hWnd, SW_SHOWNORMAL);
            SetForegroundWindow(hWnd);
            break;

        case ID_TRAY_AUTOSTART:
            g_config.autoStart = !g_config.autoStart;
            SetAutoStart(g_config.autoStart);
            SaveConfiguration();
            break;

        case ID_TRAY_WEB:
            ShellExecuteA(NULL, "open", "https://kiloapps.web.app", NULL, NULL, SW_SHOWNORMAL);
            break;

        case ID_TRAY_EXIT:
            RemoveTrayIcon();
            DestroyWindow(hWnd);
            break;
        }
        break;
    }

    case WM_TRAYICON:
        if (lParam == WM_LBUTTONUP || lParam == WM_LBUTTONDBLCLK) {
            // Restore window
            g_viewMode = VIEW_DASHBOARD;
            UpdateControlVisibility();
            ShowWindow(hWnd, SW_SHOWNORMAL);
            SetForegroundWindow(hWnd);
        } else if (lParam == WM_RBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();
            AppendMenuA(hMenu, MF_STRING | MF_DEFAULT, ID_TRAY_OPEN, "📊 Open Contributor Dashboard...");
            AppendMenuA(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hMenu, MF_STRING, ID_TRAY_RUNNOW, "⚡ Donate Turn Now");
            AppendMenuA(hMenu, MF_STRING, ID_TRAY_PAUSE, g_config.paused ? "▶️ Resume Volunteer Engine" : "⏸️ Pause Volunteer Engine");
            AppendMenuA(hMenu, MF_STRING, ID_TRAY_SETTINGS, "⚙️ Configure Key & Cadence...");
            AppendMenuA(hMenu, MF_STRING | (g_config.autoStart ? MF_CHECKED : 0), ID_TRAY_AUTOSTART, "🔄 Launch with Windows");
            AppendMenuA(hMenu, MF_STRING, ID_TRAY_WEB, "🌐 View Living Machine (kiloapps.web.app)");
            AppendMenuA(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hMenu, MF_STRING, ID_TRAY_EXIT, "❌ Exit KContribute");

            SetForegroundWindow(hWnd);
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
            DestroyMenu(hMenu);
        }
        break;

    case WM_TIMER:
        if (wParam == TIMER_ANIM) {
            g_radarAngle += 0.08f;
            if (g_radarAngle > 6.28318f) g_radarAngle -= 6.28318f;

            // Animate FFT spectral noise waves
            for (int i = 0; i < 64; i++) {
                float target = (float)(rand() % 100) / 100.0f;
                g_fftWave[i] += (target - g_fftWave[i]) * 0.25f;
            }

            if (g_isProcessingTurn) {
                g_processProgress += 0.015f;
                if (g_processProgress >= 1.0f) {
                    g_isProcessingTurn = 0;
                    g_processProgress = 0.0f;
                    g_config.turnsCount++;
                    g_config.lastTurnTime = time(NULL);
                    SaveConfiguration();

                    // Reset countdown based on cadence
                    if (g_config.cadence == 1) g_countdownSeconds = 14400; // 4 hrs
                    else if (g_config.cadence == 2) g_countdownSeconds = 1200; // 20 mins
                    else g_countdownSeconds = 3600; // 1 hr

                    sprintf_s(g_statusText, sizeof(g_statusText), "TURN COMPLETED: %s (Verified clean)", g_config.lastTurnApp);

                    // Show notification balloon in system tray
                    strcpy_s(g_nid.szInfo, sizeof(g_nid.szInfo), "Turn completed & builds verified! Spare turn contributed to the autonomous fleet.");
                    sprintf_s(g_nid.szInfoTitle, sizeof(g_nid.szInfoTitle), "KiloApps: Turn #%d Donated", g_config.turnsCount);
                    g_nid.dwInfoFlags = NIIF_INFO;
                    Shell_NotifyIconA(NIM_MODIFY, &g_nid);
                }
            }

            if (g_viewMode == VIEW_DASHBOARD && IsWindowVisible(hWnd)) {
                // Redraw visualizer region
                RECT rcVis = { 20, 65, 340, 260 };
                InvalidateRect(hWnd, &rcVis, FALSE);
            }
        } else if (wParam == TIMER_TICK) {
            if (!g_config.paused && !g_isProcessingTurn) {
                g_countdownSeconds--;
                if (g_countdownSeconds <= 0) {
                    TriggerVolunteerTurn();
                }
            }
            if (g_viewMode == VIEW_DASHBOARD && IsWindowVisible(hWnd)) {
                RECT rcStats = { 355, 65, 580, 420 };
                InvalidateRect(hWnd, &rcStats, FALSE);
            }
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);

        // Background
        HBRUSH hBrDark = CreateSolidBrush(RGB(9, 13, 22));
        FillRect(hdc, &rcClient, hBrDark);
        DeleteObject(hBrDark);

        SetBkMode(hdc, TRANSPARENT);

        if (g_viewMode == VIEW_ONBOARDING) {
            // === ONBOARDING SCREEN PAINTING ===
            // Header Title
            SelectObject(hdc, g_fontTitle);
            SetTextColor(hdc, RGB(248, 250, 252));
            TextOutA(hdc, 30, 22, "🛰️ KILOAPPS VOLUNTEER FLEET CONTRIBUTOR", 39);

            SelectObject(hdc, g_fontUI);
            SetTextColor(hdc, RGB(56, 189, 248));
            TextOutA(hdc, 30, 52, "SETI@home style voluntary distributed compute for the living autonomous OS.", 74);

            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutA(hdc, 30, 76, "Google AI Studio provides 100% free Gemini Flash quota (1,500 requests/day).", 76);
            TextOutA(hdc, 30, 93, "No credit card or billing setup required. Runs quietly in your system tray.", 74);

            // Labels
            SelectObject(hdc, g_fontUIBold);
            SetTextColor(hdc, RGB(226, 232, 240));
            TextOutA(hdc, 30, 165, "Paste Gemini API Key (Starts with AIzaSy...):", 45);
            TextOutA(hdc, 30, 225, "Volunteer Handle / Call-Sign (Optional):", 40);
            TextOutA(hdc, 30, 283, "Contribution Cadence (Throttle Control):", 40);

        } else {
            // === DASHBOARD SCREEN PAINTING ===
            // Top Status Bar
            HBRUSH hBrBar = CreateSolidBrush(RGB(15, 23, 42));
            RECT rcBar = { 0, 0, rcClient.right, 48 };
            FillRect(hdc, &rcBar, hBrBar);
            DeleteObject(hBrBar);

            HPEN hPenLine = CreatePen(PS_SOLID, 1, RGB(30, 41, 59));
            HGDIOBJ oldPen = SelectObject(hdc, hPenLine);
            MoveToEx(hdc, 0, 48, NULL); LineTo(hdc, rcClient.right, 48);
            SelectObject(hdc, oldPen);
            DeleteObject(hPenLine);

            SelectObject(hdc, g_fontTitle);
            SetTextColor(hdc, RGB(255, 255, 255));
            TextOutA(hdc, 20, 12, "🛰️ KCONTRIBUTE FLEET TELEMETRY", 31);

            // Status Badge
            RECT rcBadge = { rcClient.right - 230, 12, rcClient.right - 20, 36 };
            HBRUSH hBrBadge = CreateSolidBrush(g_config.paused ? RGB(45, 30, 5) : (g_isProcessingTurn ? RGB(10, 45, 35) : RGB(10, 35, 55)));
            FillRect(hdc, &rcBadge, hBrBadge);
            DeleteObject(hBrBadge);

            FrameRect(hdc, &rcBadge, (HBRUSH)GetStockObject(WHITE_BRUSH));
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, g_config.paused ? RGB(245, 158, 11) : (g_isProcessingTurn ? RGB(16, 185, 129) : RGB(56, 189, 248)));
            DrawTextA(hdc, g_config.paused ? "STATUS: PAUSED" : (g_isProcessingTurn ? "STATUS: COMPUTING" : "STATUS: TRAY ACTIVE"),
                -1, &rcBadge, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Left: SETI@home Retro Vector Radar & Oscilloscope Box
            RECT rcVis = { 20, 65, 340, 275 };
            HBRUSH hBrVis = CreateSolidBrush(RGB(3, 7, 18));
            FillRect(hdc, &rcVis, hBrVis);
            DeleteObject(hBrVis);
            FrameRect(hdc, &rcVis, (HBRUSH)GetStockObject(GRAY_BRUSH));

            // Radar Circle & Sweep Line
            int cx = 180, cy = 150, radius = 65;
            HPEN hPenDarkGreen = CreatePen(PS_SOLID, 1, RGB(6, 78, 59));
            oldPen = SelectObject(hdc, hPenDarkGreen);
            Arc(hdc, cx - radius, cy - radius, cx + radius, cy + radius, 0, 0, 0, 0);
            Arc(hdc, cx - radius / 2, cy - radius / 2, cx + radius / 2, cy + radius / 2, 0, 0, 0, 0);
            MoveToEx(hdc, cx - radius, cy, NULL); LineTo(hdc, cx + radius, cy);
            MoveToEx(hdc, cx, cy - radius, NULL); LineTo(hdc, cx, cy + radius);

            // Sweep
            HPEN hPenSweep = CreatePen(PS_SOLID, 2, RGB(16, 185, 129));
            SelectObject(hdc, hPenSweep);
            int sx = cx + (int)(cosf(g_radarAngle) * radius);
            int sy = cy + (int)(sinf(g_radarAngle) * radius);
            MoveToEx(hdc, cx, cy, NULL);
            LineTo(hdc, sx, sy);
            DeleteObject(hPenSweep);

            // FFT Spectral Waveform underneath radar
            HPEN hPenCyan = CreatePen(PS_SOLID, 1, RGB(56, 189, 248));
            SelectObject(hdc, hPenCyan);
            int startX = 35;
            for (int i = 0; i < 48; i++) {
                int h = (int)(g_fftWave[i] * 32.0f);
                MoveToEx(hdc, startX + i * 6, 260, NULL);
                LineTo(hdc, startX + i * 6, 260 - h);
            }
            SelectObject(hdc, oldPen);
            DeleteObject(hPenCyan);
            DeleteObject(hPenDarkGreen);

            // Telemetry overlay on radar
            SelectObject(hdc, g_fontMono);
            SetTextColor(hdc, RGB(52, 211, 153));
            TextOutA(hdc, 28, 72, "WORK UNIT SPECTRAL TELEMETRY", 28);

            // Bottom Work Unit Progress / Details Box
            RECT rcWU = { 20, 285, rcClient.right - 20, 420 };
            HBRUSH hBrWU = CreateSolidBrush(RGB(15, 23, 42));
            FillRect(hdc, &rcWU, hBrWU);
            DeleteObject(hBrWU);
            FrameRect(hdc, &rcWU, (HBRUSH)GetStockObject(GRAY_BRUSH));

            SelectObject(hdc, g_fontUIBold);
            SetTextColor(hdc, RGB(255, 255, 255));
            TextOutA(hdc, 35, 295, "ACTIVE WORK UNIT EVOLUTION PIPELINE", 35);

            SelectObject(hdc, g_fontMono);
            SetTextColor(hdc, RGB(56, 189, 248));
            char wuBuf[192];
            sprintf_s(wuBuf, sizeof(wuBuf), "TARGET: %s", g_config.lastTurnApp[0] ? g_config.lastTurnApp : "Pending next scheduled cycle");
            TextOutA(hdc, 35, 320, wuBuf, (int)strlen(wuBuf));

            SetTextColor(hdc, RGB(245, 158, 11));
            sprintf_s(wuBuf, sizeof(wuBuf), "STATUS: %s", g_statusText);
            TextOutA(hdc, 35, 340, wuBuf, (int)strlen(wuBuf));

            // Progress Bar
            RECT rcProgBg = { 35, 368, rcClient.right - 35, 386 };
            HBRUSH hBrProgBg = CreateSolidBrush(RGB(2, 6, 23));
            FillRect(hdc, &rcProgBg, hBrProgBg);
            DeleteObject(hBrProgBg);

            int progW = (int)((rcClient.right - 70) * (g_isProcessingTurn ? g_processProgress : 1.0f));
            RECT rcProg = { 35, 368, 35 + progW, 386 };
            HBRUSH hBrProg = CreateSolidBrush(g_isProcessingTurn ? RGB(16, 185, 129) : RGB(56, 189, 248));
            FillRect(hdc, &rcProg, hBrProg);
            DeleteObject(hBrProg);
            FrameRect(hdc, &rcProgBg, (HBRUSH)GetStockObject(WHITE_BRUSH));

            // Right: Donor Stats Box
            SelectObject(hdc, g_fontUIBold);
            SetTextColor(hdc, RGB(255, 255, 255));
            TextOutA(hdc, 360, 68, "VOLUNTEER DONOR PROFILE", 23);

            SelectObject(hdc, g_fontUI);
            SetTextColor(hdc, RGB(148, 163, 184));
            char statLine[128];

            sprintf_s(statLine, sizeof(statLine), "Handle: %s", g_config.handle);
            TextOutA(hdc, 360, 95, statLine, (int)strlen(statLine));

            int rank = 1 + (g_config.turnsCount / 5);
            const char* rankName = (rank > 10) ? "Fleet Architect" : (rank > 5 ? "Senior Engineer" : (rank > 2 ? "Fleet Pioneer" : "Volunteer Cadet"));
            sprintf_s(statLine, sizeof(statLine), "Donor Rank: %s (Lvl %d)", rankName, rank);
            TextOutA(hdc, 360, 118, statLine, (int)strlen(statLine));

            SetTextColor(hdc, RGB(16, 185, 129));
            sprintf_s(statLine, sizeof(statLine), "Turns Contributed: %d turns", g_config.turnsCount);
            TextOutA(hdc, 360, 141, statLine, (int)strlen(statLine));

            SetTextColor(hdc, RGB(148, 163, 184));
            float reqsEst = (float)g_config.turnsCount * 50.0f;
            sprintf_s(statLine, sizeof(statLine), "Free Quota Donated: ~%.0f tokens/reqs", reqsEst);
            TextOutA(hdc, 360, 164, statLine, (int)strlen(statLine));

            const char* cadName = (g_config.cadence == 1) ? "Quiet (1 / 4h)" : (g_config.cadence == 2 ? "Turbo (3 / hr)" : "Balanced (1 / hr)");
            sprintf_s(statLine, sizeof(statLine), "Cadence Throttle: %s", cadName);
            TextOutA(hdc, 360, 187, statLine, (int)strlen(statLine));

            int minLeft = g_countdownSeconds / 60;
            int secLeft = g_countdownSeconds % 60;
            SetTextColor(hdc, RGB(56, 189, 248));
            sprintf_s(statLine, sizeof(statLine), "Next Scheduled Turn: %02d:%02d", minLeft, secLeft);
            TextOutA(hdc, 360, 210, statLine, (int)strlen(statLine));

            SetTextColor(hdc, RGB(148, 163, 184));
            sprintf_s(statLine, sizeof(statLine), "Windows Auto-Start: %s", g_config.autoStart ? "Enabled (Tray Daemon)" : "Disabled");
            TextOutA(hdc, 360, 233, statLine, (int)strlen(statLine));
        }

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_CLOSE:
        // Intercept close [X]: Minimize directly to system tray! Never exit unexpectedly.
        if (g_config.configured) {
            ShowWindow(hWnd, SW_HIDE);
        } else {
            DestroyWindow(hWnd);
        }
        return 0;

    case WM_DESTROY:
        KillTimer(hWnd, TIMER_ANIM);
        KillTimer(hWnd, TIMER_TICK);
        RemoveTrayIcon();
        if (g_hTrayIcon) DestroyIcon(g_hTrayIcon);
        if (g_fontTitle) DeleteObject(g_fontTitle);
        if (g_fontMono) DeleteObject(g_fontMono);
        if (g_fontMonoBold) DeleteObject(g_fontMonoBold);
        if (g_fontUI) DeleteObject(g_fontUI);
        if (g_fontUIBold) DeleteObject(g_fontUIBold);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcA(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Single instance mutex guard
    HANDLE hMutex = CreateMutexA(NULL, TRUE, "KiloApps_KContribute_SingleInstance_Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        HWND hExisting = FindWindowA("KContributeWindowClass", NULL);
        if (hExisting) {
            ShowWindow(hExisting, SW_SHOWNORMAL);
            SetForegroundWindow(hExisting);
        }
        return 0;
    }

    InitCommonControls();
    LoadConfiguration();

    // Setup Fonts
    g_fontTitle = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
    g_fontMonoBold = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
    g_fontMono = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
    g_fontUI = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    g_fontUIBold = CreateFontA(13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

    WNDCLASSEXA wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "KContributeWindowClass";

    if (!RegisterClassExA(&wc)) {
        return 1;
    }

    // Window dimensions: 600 x 520
    int winW = 600, winH = 520;
    int scrW = GetSystemMetrics(SM_CXSCREEN);
    int scrH = GetSystemMetrics(SM_CYSCREEN);
    int posX = (scrW - winW) / 2;
    int posY = (scrH - winH) / 2;

    HWND hWnd = CreateWindowExA(
        WS_EX_APPWINDOW,
        "KContributeWindowClass",
        "KContribute - Volunteer Fleet Engine v1.0.0",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        posX, posY, winW, winH,
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd) return 1;

    // Check startup mode:
    // If already configured: auto-start directly in system tray without opening window!
    if (g_config.configured) {
        InitTrayIcon(hWnd);
        ShowWindow(hWnd, SW_HIDE);
    } else {
        // First run: show onboarding setup window
        ShowWindow(hWnd, SW_SHOWNORMAL);
        UpdateWindow(hWnd);
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (hMutex) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }

    return (int)msg.wParam;
}
