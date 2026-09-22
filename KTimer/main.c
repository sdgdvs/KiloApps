#include <windows.h>
#include <commdlg.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

// Control Identifiers
#define ID_BTN_SW_TAB      1000
#define ID_BTN_TM_TAB      1001
#define ID_BTN_MT_TAB      1002
#define ID_BTN_POMO_TAB    1003
#define ID_BTN_INT_TAB     1004
#define ID_BTN_HELP        1005

#define ID_BTN_START       1010
#define ID_BTN_LAP         1011
#define ID_BTN_RESET       1012
#define ID_BTN_EXPORT_CSV  1013
#define ID_BTN_EXPORT_TXT  1014
#define ID_LIST_LAPS       1015
#define ID_BTN_COPY_LAPS   1016

#define ID_EDIT_TM_INPUT   1020
#define ID_PRESET_1M       1021
#define ID_PRESET_3M       1022
#define ID_PRESET_5M       1023
#define ID_PRESET_10M      1024
#define ID_PRESET_15M      1025
#define ID_PRESET_25M      1026
#define ID_PRESET_30M      1027
#define ID_PRESET_60M      1028
#define ID_BTN_SUB_1M      1060
#define ID_BTN_ADD_1M      1061
#define ID_BTN_ADD_5M      1062

#define ID_EDIT_MT_NAME    1030
#define ID_EDIT_MT_TIME    1031
#define ID_BTN_MT_ADD      1032
#define ID_LIST_MT         1033
#define ID_BTN_MT_STARTALL 1034
#define ID_BTN_MT_PAUSEALL 1035
#define ID_BTN_MT_DEL      1036
#define ID_BTN_MT_PRESET1  1065
#define ID_BTN_MT_PRESET2  1066
#define ID_BTN_MT_PRESET3  1067

#define ID_BTN_POMO_START  1040
#define ID_BTN_POMO_SKIP   1041
#define ID_BTN_POMO_RESET  1042

#define ID_EDIT_INT_WORK    1050
#define ID_EDIT_INT_REST    1051
#define ID_EDIT_INT_SETS    1052
#define ID_EDIT_INT_PREP    1053
#define ID_BTN_INT_START    1054
#define ID_BTN_INT_SKIP     1055
#define ID_BTN_INT_RESET    1056
#define ID_PRESET_TABATA    1057
#define ID_PRESET_HIIT30    1058
#define ID_PRESET_BOXING    1059
#define ID_STATIC_STATUS    1070
#define ID_BTN_SAVE         1071
#define ID_BTN_LOAD         1072

// App Modes
typedef enum {
    MODE_STOPWATCH = 0,
    MODE_TIMER = 1,
    MODE_MULTI = 2,
    MODE_POMODORO = 3,
    MODE_INTERVAL = 4
} AppMode;

AppMode g_mode = MODE_STOPWATCH;

// Interval State
typedef enum { INT_PHASE_PREP = 0, INT_PHASE_WORK = 1, INT_PHASE_REST = 2, INT_PHASE_DONE = 3 } IntervalPhase;
IntervalPhase g_intPhase = INT_PHASE_PREP;
int g_intCurrentSet = 1;
int g_intTotalSets = 8;
DWORD g_intWorkMs = 20 * 1000;
DWORD g_intRestMs = 10 * 1000;
DWORD g_intPrepMs = 5 * 1000;
DWORD g_intRemainingMs = 5 * 1000;
DWORD g_intTotalPhaseMs = 5 * 1000;
DWORD g_intTargetTime = 0;
int g_intIsRunning = 0;

// Lap Structure
typedef struct {
    int id;
    DWORD totalMs;
    DWORD splitMs;
} LapInfo;

#define MAX_LAPS 200
LapInfo g_laps[MAX_LAPS];
int g_lapCount = 0;

// Multi-Timer Structure
typedef struct {
    char name[32];
    DWORD totalMs;
    DWORD remainingMs;
    DWORD lastTick;
    int isRunning;
} MultiTimer;

#define MAX_MULTI_TIMERS 10
MultiTimer g_multiTimers[MAX_MULTI_TIMERS];
int g_multiTimerCount = 0;

// Pomodoro State
typedef enum { POMO_WORK = 0, POMO_SHORT_BREAK = 1, POMO_LONG_BREAK = 2 } PomoState;
PomoState g_pomoState = POMO_WORK;
int g_pomoCycleCount = 1;
DWORD g_pomoRemainingMs = 25 * 60 * 1000;
DWORD g_pomoTotalMs = 25 * 60 * 1000;
DWORD g_pomoTargetTime = 0;
int g_pomoIsRunning = 0;
int g_pomoCompletedSessions = 0;
DWORD g_pomoTotalFocusMins = 0;

// Stopwatch State
DWORD g_swStartTime = 0;
DWORD g_swElapsed = 0;
int g_swIsRunning = 0;
char g_swTimeBuf[32] = "00:00:00.000";

// Timer State
DWORD g_tmTargetTime = 0;
DWORD g_tmRemainingMs = 300000; // default 5 mins
DWORD g_tmTotalMs = 300000;
int g_tmIsRunning = 0;
char g_tmTimeBuf[32] = "00:05:00";

// Global Window Handles

static int g_dpiScale = 100;
#define S(x) ((x) * g_dpiScale / 100)

HWND hMainWnd = NULL;
HWND hTabSW, hTabTM, hTabMT, hTabPOMO, hTabINT, hBtnHelp, hBtnSave, hBtnLoad;
HWND hDisplay, hTmInput, hStaticStats, hStaticIntStats, hStaticIntLabels;
HWND hBtnStart, hBtnLap, hBtnReset, hBtnExportCsv, hBtnExportTxt, hBtnCopyLaps;
HWND hListLaps;

// Presets Handles
HWND hPresets[8];
HWND hBtnSub1m, hBtnAdd1m, hBtnAdd5m;

// Multi-Timer Handles
HWND hEditMtName, hEditMtTime, hBtnMtAdd, hListMt, hBtnMtStartAll, hBtnMtPauseAll, hBtnMtDel;
HWND hBtnMtPreset1, hBtnMtPreset2, hBtnMtPreset3;

// Pomodoro Handles
HWND hBtnPomoStart, hBtnPomoSkip, hBtnPomoReset;

// Interval Handles
HWND hEditIntWork, hEditIntRest, hEditIntSets, hEditIntPrep;
HWND hBtnIntStart, hBtnIntSkip, hBtnIntReset;
HWND hBtnPresetTabata, hBtnPresetHiit, hBtnPresetBoxing;

HWND hStatusLabel = NULL;
HWND hHelpLabel = NULL;

char g_statusMsg[128] = "⏱️ Ready - Space: Start/Pause, 1-5: Tabs, F1: Help";
int g_statusTimer = 160;

static void ShowNativeStatus(const char* msg) {
    lstrcpynA(g_statusMsg, msg, sizeof(g_statusMsg));
    g_statusTimer = 140; // ~3.5 seconds
    if (hStatusLabel) SetWindowTextA(hStatusLabel, g_statusMsg);
}

HFONT hFontDisplay = NULL;
HFONT hFontBtn = NULL;
HFONT hFontSmall = NULL;
HBRUSH hBgBrush = NULL;
HBRUSH hControlBrush = NULL;
HBRUSH hProgressBarBrush = NULL;

// Helper String Formatting
static void FormatMsToStopwatch(DWORD totalMs, char* outBuf, size_t bufSize) {
    DWORD ms = totalMs % 1000;
    DWORD s = (totalMs / 1000) % 60;
    DWORD m = (totalMs / 60000) % 60;
    DWORD h = (totalMs / 3600000);
    wsprintfA(outBuf, "%02u:%02u:%02u.%03u", h, m, s, ms);
}

static void FormatMsToTimer(DWORD totalMs, char* outBuf, size_t bufSize) {
    DWORD s = (totalMs / 1000) % 60;
    DWORD m = (totalMs / 60000) % 60;
    DWORD h = (totalMs / 3600000);
    wsprintfA(outBuf, "%02u:%02u:%02u", h, m, s);
}

static int SimpleStrToInt(const char* p) {
    if (!p) return 0;
    while (*p == ' ' || *p == '\t') p++;
    int val = 0;
    while (*p >= '0' && *p <= '9') {
        val = val * 10 + (*p - '0');
        p++;
    }
    return val;
}

static DWORD ParseTimerInput(const char* str) {
    if (!str) return 0;
    while (*str == ' ' || *str == '\t') str++;
    int h = 0, m = 0, s = 0;
    int colons = 0;
    const char* p = str;
    const char* part1 = str;
    const char* part2 = NULL;
    const char* part3 = NULL;

    while (*p) {
        if (*p == ':') {
            colons++;
            if (colons == 1) part2 = p + 1;
            else if (colons == 2) part3 = p + 1;
        }
        p++;
    }

    if (colons == 2) {
        h = SimpleStrToInt(part1);
        m = SimpleStrToInt(part2);
        s = SimpleStrToInt(part3);
    } else if (colons == 1) {
        m = SimpleStrToInt(part1);
        s = SimpleStrToInt(part2);
    } else {
        int hasM = 0;
        p = str;
        while (*p) {
            if (*p == 'm' || *p == 'M') hasM = 1;
            p++;
        }
        if (hasM) {
            m = SimpleStrToInt(part1);
        } else {
            s = SimpleStrToInt(part1);
        }
    }

    if (h < 0) h = 0;
    if (m < 0) m = 0;
    if (s < 0) s = 0;

    DWORD totalSec = (DWORD)h * 3600 + (DWORD)m * 60 + (DWORD)s;
    if (totalSec > 359999) totalSec = 359999;
    return totalSec * 1000;
}

static void PlayAlarmSound() {
    MessageBeep(MB_ICONASTERISK);
    Beep(800, 200);
    Beep(1000, 200);
    Beep(1200, 300);
}

static void PlayChimeSound() {
    MessageBeep(MB_OK);
    Beep(523, 150);
    Beep(659, 150);
    Beep(784, 250);
}

static void PlayWorkStartSound() {
    MessageBeep(MB_ICONASTERISK);
    Beep(880, 150);
    Beep(1174, 250);
}

static void PlayRestStartSound() {
    MessageBeep(MB_OK);
    Beep(587, 180);
    Beep(440, 250);
}

static void UpdateStopwatchDisplay() {
    DWORD current = g_swElapsed;
    if (g_swIsRunning) {
        current += (GetTickCount() - g_swStartTime);
    }
    FormatMsToStopwatch(current, g_swTimeBuf, sizeof(g_swTimeBuf));
    if (g_mode == MODE_STOPWATCH) {
        SetWindowTextA(hDisplay, g_swTimeBuf);
    }
}

static void UpdateTimerDisplay() {
    if (g_tmIsRunning) {
        DWORD now = GetTickCount();
        if (now >= g_tmTargetTime) {
            g_tmRemainingMs = 0;
            g_tmIsRunning = 0;
            SetWindowTextA(hBtnStart, "Start");
            ShowWindow(hTmInput, SW_SHOW);
            ShowWindow(hDisplay, SW_HIDE);
            PlayAlarmSound();
        } else {
            g_tmRemainingMs = g_tmTargetTime - now;
        }
    }

    FormatMsToTimer(g_tmRemainingMs, g_tmTimeBuf, sizeof(g_tmTimeBuf));
    if (g_mode == MODE_TIMER && g_tmIsRunning) {
        static char s_lastTmBuf[32] = {0};
        if (lstrcmpA(s_lastTmBuf, g_tmTimeBuf) != 0) {
            lstrcpyA(s_lastTmBuf, g_tmTimeBuf);
            SetWindowTextA(hDisplay, g_tmTimeBuf);
        }
    }
}

static void UpdatePomodoroDisplay() {
    if (g_pomoIsRunning) {
        DWORD now = GetTickCount();
        if (now >= g_pomoTargetTime) {
            g_pomoRemainingMs = 0;
            g_pomoIsRunning = 0;
            SetWindowTextA(hBtnPomoStart, "Start");
            PlayChimeSound();

            if (g_pomoState == POMO_WORK) {
                g_pomoCompletedSessions++;
                g_pomoTotalFocusMins += (g_pomoTotalMs / 60000);
                if (g_pomoCycleCount >= 4) {
                    g_pomoState = POMO_LONG_BREAK;
                    g_pomoCycleCount = 1;
                    g_pomoTotalMs = 15 * 60 * 1000;
                } else {
                    g_pomoState = POMO_SHORT_BREAK;
                    g_pomoCycleCount++;
                    g_pomoTotalMs = 5 * 60 * 1000;
                }
            } else {
                g_pomoState = POMO_WORK;
                g_pomoTotalMs = 25 * 60 * 1000;
            }
            g_pomoRemainingMs = g_pomoTotalMs;
        } else {
            g_pomoRemainingMs = g_pomoTargetTime - now;
        }
    }

    char pomoBuf[64];
    FormatMsToTimer(g_pomoRemainingMs, pomoBuf, sizeof(pomoBuf));

    if (g_mode == MODE_POMODORO) {
        static char s_lastPomoDisp[32] = {0};
        static char s_lastPomoStats[128] = {0};

        if (lstrcmpA(s_lastPomoDisp, pomoBuf + 3) != 0) {
            lstrcpyA(s_lastPomoDisp, pomoBuf + 3);
            SetWindowTextA(hDisplay, pomoBuf + 3); // MM:SS
        }

        const char* stateName = "WORK SESSION";
        if (g_pomoState == POMO_SHORT_BREAK) stateName = "SHORT BREAK";
        if (g_pomoState == POMO_LONG_BREAK) stateName = "LONG BREAK";

        char statsBuf[128];
        wsprintfA(statsBuf, "%s (Cycle %d/4)\nDone: %d | Focus: %u mins", stateName, g_pomoCycleCount, g_pomoCompletedSessions, g_pomoTotalFocusMins);
        if (lstrcmpA(s_lastPomoStats, statsBuf) != 0) {
            lstrcpyA(s_lastPomoStats, statsBuf);
            SetWindowTextA(hStaticStats, statsBuf);
        }
    }
}

static void UpdateIntervalDisplay() {
    if (g_intIsRunning) {
        DWORD now = GetTickCount();
        if (now >= g_intTargetTime) {
            g_intRemainingMs = 0;

            if (g_intPhase == INT_PHASE_PREP) {
                g_intPhase = INT_PHASE_WORK;
                g_intCurrentSet = 1;
                g_intTotalPhaseMs = g_intWorkMs;
                g_intRemainingMs = g_intWorkMs;
                g_intTargetTime = now + g_intWorkMs;
                PlayWorkStartSound();
            } else if (g_intPhase == INT_PHASE_WORK) {
                if (g_intCurrentSet < g_intTotalSets) {
                    g_intPhase = INT_PHASE_REST;
                    g_intTotalPhaseMs = g_intRestMs;
                    g_intRemainingMs = g_intRestMs;
                    g_intTargetTime = now + g_intRestMs;
                    PlayRestStartSound();
                } else {
                    g_intPhase = INT_PHASE_DONE;
                    g_intIsRunning = 0;
                    SetWindowTextA(hBtnIntStart, "Start");
                    PlayChimeSound();
                }
            } else if (g_intPhase == INT_PHASE_REST) {
                g_intCurrentSet++;
                g_intPhase = INT_PHASE_WORK;
                g_intTotalPhaseMs = g_intWorkMs;
                g_intRemainingMs = g_intWorkMs;
                g_intTargetTime = now + g_intWorkMs;
                PlayWorkStartSound();
            }
        } else {
            g_intRemainingMs = g_intTargetTime - now;
        }
    }

    char timeBuf[32];
    FormatMsToTimer(g_intRemainingMs, timeBuf, sizeof(timeBuf));

    if (g_mode == MODE_INTERVAL) {
        static char s_lastIntDisp[32] = {0};
        static char s_lastIntStats[128] = {0};

        if (lstrcmpA(s_lastIntDisp, timeBuf + 3) != 0) {
            lstrcpyA(s_lastIntDisp, timeBuf + 3);
            SetWindowTextA(hDisplay, timeBuf + 3); // MM:SS
        }

        const char* phaseStr = "PREPARE";
        if (g_intPhase == INT_PHASE_WORK) phaseStr = "WORK!";
        else if (g_intPhase == INT_PHASE_REST) phaseStr = "REST";
        else if (g_intPhase == INT_PHASE_DONE) phaseStr = "FINISHED!";

        char statsBuf[128];
        wsprintfA(statsBuf, "%s (Set %d/%d)\nWork: %ds | Rest: %ds", phaseStr, g_intCurrentSet, g_intTotalSets, (int)(g_intWorkMs/1000), (int)(g_intRestMs/1000));
        if (lstrcmpA(s_lastIntStats, statsBuf) != 0) {
            lstrcpyA(s_lastIntStats, statsBuf);
            SetWindowTextA(hStaticIntStats, statsBuf);
        }
    }
}

static void UpdateMultiTimers() {
    DWORD now = GetTickCount();
    int needRefresh = 0;
    static DWORD s_lastMtListTick = 0;

    for (int i = 0; i < g_multiTimerCount; i++) {
        if (g_multiTimers[i].isRunning) {
            DWORD delta = now - g_multiTimers[i].lastTick;
            g_multiTimers[i].lastTick = now;
            if (delta >= g_multiTimers[i].remainingMs) {
                g_multiTimers[i].remainingMs = 0;
                g_multiTimers[i].isRunning = 0;
                PlayAlarmSound();
            } else {
                g_multiTimers[i].remainingMs -= delta;
            }
            needRefresh = 1;
        }
    }

    if (needRefresh && g_mode == MODE_MULTI && (now - s_lastMtListTick >= 250)) {
        s_lastMtListTick = now;
        int curSel = (int)SendMessageA(hListMt, LB_GETCURSEL, 0, 0);
        int topIdx = (int)SendMessageA(hListMt, LB_GETTOPINDEX, 0, 0);

        SendMessageA(hListMt, WM_SETREDRAW, FALSE, 0);
        SendMessageA(hListMt, LB_RESETCONTENT, 0, 0);
        for (int i = 0; i < g_multiTimerCount; i++) {
            char itemBuf[128];
            char timeBuf[32];
            FormatMsToTimer(g_multiTimers[i].remainingMs, timeBuf, sizeof(timeBuf));
            const char* status = g_multiTimers[i].remainingMs == 0 ? "[DONE]" : (g_multiTimers[i].isRunning ? "[RUNNING]" : "[PAUSED]");
            wsprintfA(itemBuf, "%s - %s %s", g_multiTimers[i].name, timeBuf, status);
            SendMessageA(hListMt, LB_ADDSTRING, 0, (LPARAM)itemBuf);
        }
        if (curSel >= 0 && curSel < g_multiTimerCount) {
            SendMessageA(hListMt, LB_SETCURSEL, curSel, 0);
        }
        if (topIdx >= 0) {
            SendMessageA(hListMt, LB_SETTOPINDEX, topIdx, 0);
        }
        SendMessageA(hListMt, WM_SETREDRAW, TRUE, 0);
        InvalidateRect(hListMt, NULL, FALSE);
    }
}

static void CopyLapsToClipboard() {
    if (g_lapCount == 0) {
        ShowNativeStatus("No laps recorded to copy.");
        return;
    }
    DWORD bufSize = 128 + g_lapCount * 80;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bufSize);
    if (!hMem) return;
    char* p = (char*)GlobalLock(hMem);
    if (!p) { GlobalFree(hMem); return; }
    char* cur = p;
    wsprintfA(cur, "=== KTimer Lap Report (%d laps) ===\r\n\r\n", g_lapCount);
    cur += lstrlenA(cur);
    for (int i = 0; i < g_lapCount; i++) {
        char sBuf[32], tBuf[32];
        FormatMsToStopwatch(g_laps[i].splitMs, sBuf, sizeof(sBuf));
        FormatMsToStopwatch(g_laps[i].totalMs, tBuf, sizeof(tBuf));
        wsprintfA(cur, "Lap %d | Split: %s | Total: %s\r\n", g_laps[i].id, sBuf, tBuf);
        cur += lstrlenA(cur);
    }
    GlobalUnlock(hMem);
    if (OpenClipboard(hMainWnd)) {
        EmptyClipboard();
        SetClipboardData(CF_TEXT, hMem);
        CloseClipboard();
        ShowNativeStatus("📋 Laps copied to clipboard!");
    } else {
        GlobalFree(hMem);
    }
}

static void ExportLapsToFile(const char* ext) {
    if (g_lapCount == 0) {
        ShowNativeStatus("No laps recorded to export.");
        return;
    }

    OPENFILENAMEA ofn = {0};
    char filename[MAX_PATH] = "ktimer_laps";
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hMainWnd;
    if (lstrcmpA(ext, "csv") == 0) {
        ofn.lpstrFilter = "CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0";
        lstrcatA(filename, ".csv");
    } else {
        ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
        lstrcatA(filename, ".txt");
    }
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename);
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            char buffer[2048];
            DWORD written = 0;
            if (lstrcmpA(ext, "csv") == 0) {
                lstrcpyA(buffer, "Lap,Split Time,Total Time\r\n");
                WriteFile(hFile, buffer, lstrlenA(buffer), &written, NULL);
                for (int i = 0; i < g_lapCount; i++) {
                    char sBuf[32], tBuf[32];
                    FormatMsToStopwatch(g_laps[i].splitMs, sBuf, sizeof(sBuf));
                    FormatMsToStopwatch(g_laps[i].totalMs, tBuf, sizeof(tBuf));
                    wsprintfA(buffer, "%d,%s,%s\r\n", g_laps[i].id, sBuf, tBuf);
                    WriteFile(hFile, buffer, lstrlenA(buffer), &written, NULL);
                }
            } else {
                lstrcpyA(buffer, "=== KTimer Lap Report ===\r\n\r\n");
                WriteFile(hFile, buffer, lstrlenA(buffer), &written, NULL);
                for (int i = 0; i < g_lapCount; i++) {
                    char sBuf[32], tBuf[32];
                    FormatMsToStopwatch(g_laps[i].splitMs, sBuf, sizeof(sBuf));
                    FormatMsToStopwatch(g_laps[i].totalMs, tBuf, sizeof(tBuf));
                    wsprintfA(buffer, "Lap %d | Split: %s | Total: %s\r\n", g_laps[i].id, sBuf, tBuf);
                    WriteFile(hFile, buffer, lstrlenA(buffer), &written, NULL);
                }
            }
            CloseHandle(hFile);
            ShowNativeStatus("📁 Laps exported successfully!");
        }
    }
}

static void ShowHelpDialog(HWND hwnd) {
    const char* helpText = 
        "KTimer - High-Precision Multi-Mode Timer & Stopwatch\n\n"
        "KEYBOARD SHORTCUTS:\n"
        "  [Space]     - Start / Pause active timer\n"
        "  [1] - [5]   - Switch Mode Tabs (1:SW, 2:Timer, 3:Multi, 4:Pomo, 5:HIIT)\n"
        "  [L]         - Record Lap split (Stopwatch mode)\n"
        "  [C]         - Copy laps to clipboard (Stopwatch mode)\n"
        "  [+] / [-]   - Quick adjust timer duration ±1m (Timer mode)\n"
        "  [S]         - Skip Phase (Pomo / HIIT) | Start All (Multi)\n"
        "  [P]         - Pause All (Multi-Timer)\n"
        "  [Del]       - Delete selected timer (Multi-Timer)\n"
        "  [R]         - Reset active mode timer\n"
        "  [Enter]     - Start Countdown / Add Multi-Timer\n"
        "  [F5]        - Quicksave full timers & workspace snapshot\n"
        "  [F9]        - Quickload saved snapshot from ktimer.dat\n"
        "  [F1] or [H] - Open this Help dialog\n\n"
        "MODES & FEATURES:\n"
        "  1. Stopwatch : Precision split tracking with 1-click clipboard copy & CSV/TXT export\n"
        "  2. Timer     : Countdown with 1m-60m presets and ±1m / +5m quick adjustments\n"
        "  3. Multi     : Run multiple concurrent labeled timers with quick presets\n"
        "  4. Pomodoro  : 25m work / 5m short break / 15m long break\n"
        "  5. HIIT      : Interval circuit with Tabata, HIIT, Boxing presets\n";
    MessageBoxA(hwnd, helpText, "KTimer - Help & User Guide", MB_OK | MB_ICONINFORMATION);
}

static void SwitchMode(AppMode newMode) {
    g_mode = newMode;

    // Reset button states
    SetWindowTextA(hTabSW, g_mode == MODE_STOPWATCH ? "[ SW [1] ]" : "SW [1]");
    SetWindowTextA(hTabTM, g_mode == MODE_TIMER ? "[ Timer [2] ]" : "Timer [2]");
    SetWindowTextA(hTabMT, g_mode == MODE_MULTI ? "[ Multi [3] ]" : "Multi [3]");
    SetWindowTextA(hTabPOMO, g_mode == MODE_POMODORO ? "[ Pomo [4] ]" : "Pomo [4]");
    SetWindowTextA(hTabINT, g_mode == MODE_INTERVAL ? "[ HIIT [5] ]" : "HIIT [5]");

    // Hide all mode-specific controls first
    ShowWindow(hDisplay, SW_SHOW);
    ShowWindow(hTmInput, SW_HIDE);
    ShowWindow(hBtnStart, SW_HIDE);
    ShowWindow(hBtnLap, SW_HIDE);
    ShowWindow(hBtnReset, SW_HIDE);
    ShowWindow(hBtnCopyLaps, SW_HIDE);
    ShowWindow(hBtnExportCsv, SW_HIDE);
    ShowWindow(hBtnExportTxt, SW_HIDE);
    ShowWindow(hListLaps, SW_HIDE);

    for (int i = 0; i < 8; i++) ShowWindow(hPresets[i], SW_HIDE);
    ShowWindow(hBtnSub1m, SW_HIDE);
    ShowWindow(hBtnAdd1m, SW_HIDE);
    ShowWindow(hBtnAdd5m, SW_HIDE);

    ShowWindow(hEditMtName, SW_HIDE);
    ShowWindow(hEditMtTime, SW_HIDE);
    ShowWindow(hBtnMtAdd, SW_HIDE);
    ShowWindow(hBtnMtPreset1, SW_HIDE);
    ShowWindow(hBtnMtPreset2, SW_HIDE);
    ShowWindow(hBtnMtPreset3, SW_HIDE);
    ShowWindow(hListMt, SW_HIDE);
    ShowWindow(hBtnMtStartAll, SW_HIDE);
    ShowWindow(hBtnMtPauseAll, SW_HIDE);
    ShowWindow(hBtnMtDel, SW_HIDE);

    ShowWindow(hBtnPomoStart, SW_HIDE);
    ShowWindow(hBtnPomoSkip, SW_HIDE);
    ShowWindow(hBtnPomoReset, SW_HIDE);
    ShowWindow(hStaticStats, SW_HIDE);

    ShowWindow(hBtnIntStart, SW_HIDE);
    ShowWindow(hBtnIntSkip, SW_HIDE);
    ShowWindow(hBtnIntReset, SW_HIDE);
    ShowWindow(hStaticIntStats, SW_HIDE);
    ShowWindow(hStaticIntLabels, SW_HIDE);
    ShowWindow(hEditIntWork, SW_HIDE);
    ShowWindow(hEditIntRest, SW_HIDE);
    ShowWindow(hEditIntSets, SW_HIDE);
    ShowWindow(hEditIntPrep, SW_HIDE);
    ShowWindow(hBtnPresetTabata, SW_HIDE);
    ShowWindow(hBtnPresetHiit, SW_HIDE);
    ShowWindow(hBtnPresetBoxing, SW_HIDE);

    if (g_mode == MODE_STOPWATCH) {
        ShowWindow(hBtnStart, SW_SHOW);
        ShowWindow(hBtnLap, SW_SHOW);
        ShowWindow(hBtnReset, SW_SHOW);
        ShowWindow(hBtnCopyLaps, SW_SHOW);
        ShowWindow(hBtnExportCsv, SW_SHOW);
        ShowWindow(hBtnExportTxt, SW_SHOW);
        ShowWindow(hListLaps, SW_SHOW);

        SetWindowTextA(hBtnStart, g_swIsRunning ? "Stop" : "Start");
        SetWindowTextA(hMainWnd, "KTimer - Stopwatch [Space: Start, L: Lap, C: Copy, F1: Help]");
        ShowNativeStatus("Stopwatch - Space: Start/Stop, L: Lap, C: Copy, F1: Help");
        UpdateStopwatchDisplay();
    } else if (g_mode == MODE_TIMER) {
        ShowWindow(hBtnStart, SW_SHOW);
        ShowWindow(hBtnReset, SW_SHOW);
        ShowWindow(hBtnSub1m, SW_SHOW);
        ShowWindow(hBtnAdd1m, SW_SHOW);
        ShowWindow(hBtnAdd5m, SW_SHOW);
        for (int i = 0; i < 8; i++) ShowWindow(hPresets[i], SW_SHOW);

        if (g_tmIsRunning) {
            ShowWindow(hTmInput, SW_HIDE);
            ShowWindow(hDisplay, SW_SHOW);
            SetWindowTextA(hBtnStart, "Pause");
            UpdateTimerDisplay();
        } else {
            ShowWindow(hTmInput, SW_SHOW);
            ShowWindow(hDisplay, SW_HIDE);
            SetWindowTextA(hBtnStart, "Start");
        }
        SetWindowTextA(hMainWnd, "KTimer - Timer [Space: Start, +/-: Adj, R: Reset, F1: Help]");
        ShowNativeStatus("Timer - Space: Start/Pause, +/-: Nudge 1m, R: Reset, F1: Help");
    } else if (g_mode == MODE_MULTI) {
        ShowWindow(hEditMtName, SW_SHOW);
        ShowWindow(hEditMtTime, SW_SHOW);
        ShowWindow(hBtnMtAdd, SW_SHOW);
        ShowWindow(hBtnMtPreset1, SW_SHOW);
        ShowWindow(hBtnMtPreset2, SW_SHOW);
        ShowWindow(hBtnMtPreset3, SW_SHOW);
        ShowWindow(hListMt, SW_SHOW);
        ShowWindow(hBtnMtStartAll, SW_SHOW);
        ShowWindow(hBtnMtPauseAll, SW_SHOW);
        ShowWindow(hBtnMtDel, SW_SHOW);
        ShowWindow(hDisplay, SW_HIDE);

        // Refresh Multi Listbox
        SendMessageA(hListMt, LB_RESETCONTENT, 0, 0);
        for (int i = 0; i < g_multiTimerCount; i++) {
            char itemBuf[128], timeBuf[32];
            FormatMsToTimer(g_multiTimers[i].remainingMs, timeBuf, sizeof(timeBuf));
            const char* status = g_multiTimers[i].remainingMs == 0 ? "[DONE]" : (g_multiTimers[i].isRunning ? "[RUNNING]" : "[PAUSED]");
            wsprintfA(itemBuf, "%s - %s %s", g_multiTimers[i].name, timeBuf, status);
            SendMessageA(hListMt, LB_ADDSTRING, 0, (LPARAM)itemBuf);
        }
        SetWindowTextA(hMainWnd, "KTimer - Multi-Timer [Enter: Add, S: Start, P: Pause, F1: Help]");
        ShowNativeStatus("Multi - Enter: Add, S: Start All, P: Pause All, Del: Remove");
    } else if (g_mode == MODE_POMODORO) {
        ShowWindow(hBtnPomoStart, SW_SHOW);
        ShowWindow(hBtnPomoSkip, SW_SHOW);
        ShowWindow(hBtnPomoReset, SW_SHOW);
        ShowWindow(hStaticStats, SW_SHOW);

        SetWindowTextA(hBtnPomoStart, g_pomoIsRunning ? "Pause" : "Start");
        SetWindowTextA(hMainWnd, "KTimer - Pomodoro [Space: Start, S: Skip, R: Reset, F1: Help]");
        ShowNativeStatus("Pomodoro - Space: Start/Pause, S: Skip phase, R: Reset, F1: Help");
        UpdatePomodoroDisplay();
    } else if (g_mode == MODE_INTERVAL) {
        ShowWindow(hBtnIntStart, SW_SHOW);
        ShowWindow(hBtnIntSkip, SW_SHOW);
        ShowWindow(hBtnIntReset, SW_SHOW);
        ShowWindow(hStaticIntStats, SW_SHOW);
        ShowWindow(hStaticIntLabels, SW_SHOW);
        ShowWindow(hEditIntWork, SW_SHOW);
        ShowWindow(hEditIntRest, SW_SHOW);
        ShowWindow(hEditIntSets, SW_SHOW);
        ShowWindow(hEditIntPrep, SW_SHOW);
        ShowWindow(hBtnPresetTabata, SW_SHOW);
        ShowWindow(hBtnPresetHiit, SW_SHOW);
        ShowWindow(hBtnPresetBoxing, SW_SHOW);

        SetWindowTextA(hBtnIntStart, g_intIsRunning ? "Pause" : "Start");
        SetWindowTextA(hMainWnd, "KTimer - HIIT [Space: Start, S: Skip, R: Reset, F1: Help]");
        ShowNativeStatus("HIIT - Space: Start/Pause, S: Skip phase, R: Reset, F1: Help");
        UpdateIntervalDisplay();
    }
}

// ==========================================
// Pass 5 State Persistence Architecture
// ==========================================
#define KTIMER_SAVE_MAGIC 0x4B544D52 // "KTMR"
#define KTIMER_SAVE_VERSION 1

#pragma pack(push, 1)
typedef struct {
    DWORD magic;
    DWORD version;
    int mode;
    // Stopwatch
    int swIsRunning;
    DWORD swElapsed;
    int lapCount;
    LapInfo laps[MAX_LAPS];
    // Timer
    int tmIsRunning;
    DWORD tmRemainingMs;
    DWORD tmTotalMs;
    char tmTimeBuf[32];
    // Multi-Timers
    int multiTimerCount;
    MultiTimer multiTimers[MAX_MULTI_TIMERS];
    // Pomodoro
    int pomoState;
    int pomoCycleCount;
    DWORD pomoRemainingMs;
    DWORD pomoTotalMs;
    int pomoIsRunning;
    int pomoCompletedSessions;
    DWORD pomoTotalFocusMins;
    // Interval
    int intPhase;
    int intCurrentSet;
    int intTotalSets;
    DWORD intWorkMs;
    DWORD intRestMs;
    DWORD intPrepMs;
    DWORD intRemainingMs;
    DWORD intTotalPhaseMs;
    int intIsRunning;
} KTimerSaveData;
#pragma pack(pop)

static int SaveStateToFile(const char* filename) {
    KTimerSaveData data;
    memset(&data, 0, sizeof(data));
    data.magic = KTIMER_SAVE_MAGIC;
    data.version = KTIMER_SAVE_VERSION;
    data.mode = (int)g_mode;

    // Stopwatch
    DWORD currentSwMs = g_swElapsed + (g_swIsRunning ? (GetTickCount() - g_swStartTime) : 0);
    data.swIsRunning = g_swIsRunning;
    data.swElapsed = currentSwMs;
    data.lapCount = (g_lapCount > MAX_LAPS) ? MAX_LAPS : g_lapCount;
    for (int i = 0; i < data.lapCount; i++) {
        data.laps[i] = g_laps[i];
    }

    // Timer
    data.tmIsRunning = g_tmIsRunning;
    data.tmRemainingMs = g_tmRemainingMs;
    data.tmTotalMs = g_tmTotalMs;
    lstrcpynA(data.tmTimeBuf, g_tmTimeBuf, sizeof(data.tmTimeBuf));

    // Multi-timers
    data.multiTimerCount = (g_multiTimerCount > MAX_MULTI_TIMERS) ? MAX_MULTI_TIMERS : g_multiTimerCount;
    for (int i = 0; i < data.multiTimerCount; i++) {
        data.multiTimers[i] = g_multiTimers[i];
    }

    // Pomodoro
    data.pomoState = (int)g_pomoState;
    data.pomoCycleCount = g_pomoCycleCount;
    data.pomoRemainingMs = g_pomoRemainingMs;
    data.pomoTotalMs = g_pomoTotalMs;
    data.pomoIsRunning = g_pomoIsRunning;
    data.pomoCompletedSessions = g_pomoCompletedSessions;
    data.pomoTotalFocusMins = g_pomoTotalFocusMins;

    // Interval
    data.intPhase = (int)g_intPhase;
    data.intCurrentSet = g_intCurrentSet;
    data.intTotalSets = g_intTotalSets;
    data.intWorkMs = g_intWorkMs;
    data.intRestMs = g_intRestMs;
    data.intPrepMs = g_intPrepMs;
    data.intRemainingMs = g_intRemainingMs;
    data.intTotalPhaseMs = g_intTotalPhaseMs;
    data.intIsRunning = g_intIsRunning;

    HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return 0;
    DWORD written = 0;
    BOOL res = WriteFile(hFile, &data, sizeof(data), &written, NULL);
    CloseHandle(hFile);
    return res && (written == sizeof(data));
}

static int LoadStateFromFile(const char* filename) {
    HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return 0;
    KTimerSaveData data;
    memset(&data, 0, sizeof(data));
    DWORD bytesRead = 0;
    BOOL res = ReadFile(hFile, &data, sizeof(data), &bytesRead, NULL);
    CloseHandle(hFile);

    if (!res || bytesRead != sizeof(data) || data.magic != KTIMER_SAVE_MAGIC || data.version != KTIMER_SAVE_VERSION) {
        return 0;
    }

    // Restore Stopwatch
    g_swIsRunning = data.swIsRunning;
    g_swElapsed = data.swElapsed;
    g_swStartTime = GetTickCount();
    g_lapCount = (data.lapCount > MAX_LAPS) ? MAX_LAPS : data.lapCount;
    if (hListLaps) SendMessageA(hListLaps, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < g_lapCount; i++) {
        g_laps[i] = data.laps[i];
        if (hListLaps) {
            char sBuf[32], tBuf[32], lapBuf[128];
            FormatMsToStopwatch(g_laps[i].splitMs, sBuf, sizeof(sBuf));
            FormatMsToStopwatch(g_laps[i].totalMs, tBuf, sizeof(tBuf));
            wsprintfA(lapBuf, "Lap %d | Split: %s | Total: %s", g_laps[i].id, sBuf, tBuf);
            SendMessageA(hListLaps, LB_ADDSTRING, 0, (LPARAM)lapBuf);
        }
    }
    if (hListLaps && g_lapCount > 0) SendMessageA(hListLaps, LB_SETTOPINDEX, g_lapCount - 1, 0);
    UpdateStopwatchDisplay();

    // Restore Timer
    g_tmIsRunning = data.tmIsRunning;
    g_tmRemainingMs = data.tmRemainingMs;
    g_tmTotalMs = data.tmTotalMs ? data.tmTotalMs : 300000;
    g_tmTargetTime = GetTickCount() + g_tmRemainingMs;
    lstrcpynA(g_tmTimeBuf, data.tmTimeBuf[0] ? data.tmTimeBuf : "00:05:00", sizeof(g_tmTimeBuf));
    if (hTmInput) SetWindowTextA(hTmInput, g_tmTimeBuf);
    UpdateTimerDisplay();

    // Restore Multi-Timers
    g_multiTimerCount = (data.multiTimerCount > MAX_MULTI_TIMERS) ? MAX_MULTI_TIMERS : data.multiTimerCount;
    DWORD now = GetTickCount();
    for (int i = 0; i < g_multiTimerCount; i++) {
        g_multiTimers[i] = data.multiTimers[i];
        g_multiTimers[i].lastTick = now;
    }
    if (hListMt) {
        SendMessageA(hListMt, LB_RESETCONTENT, 0, 0);
        for (int i = 0; i < g_multiTimerCount; i++) {
            char itemBuf[128], timeBuf[32];
            FormatMsToTimer(g_multiTimers[i].remainingMs, timeBuf, sizeof(timeBuf));
            const char* status = g_multiTimers[i].remainingMs == 0 ? "[DONE]" : (g_multiTimers[i].isRunning ? "[RUNNING]" : "[PAUSED]");
            wsprintfA(itemBuf, "%s - %s %s", g_multiTimers[i].name, timeBuf, status);
            SendMessageA(hListMt, LB_ADDSTRING, 0, (LPARAM)itemBuf);
        }
    }

    // Restore Pomodoro
    g_pomoState = (PomoState)data.pomoState;
    g_pomoCycleCount = data.pomoCycleCount;
    g_pomoRemainingMs = data.pomoRemainingMs;
    g_pomoTotalMs = data.pomoTotalMs ? data.pomoTotalMs : (25 * 60 * 1000);
    g_pomoIsRunning = data.pomoIsRunning;
    g_pomoTargetTime = GetTickCount() + g_pomoRemainingMs;
    g_pomoCompletedSessions = data.pomoCompletedSessions;
    g_pomoTotalFocusMins = data.pomoTotalFocusMins;
    UpdatePomodoroDisplay();

    // Restore Interval
    g_intPhase = (IntervalPhase)data.intPhase;
    g_intCurrentSet = data.intCurrentSet;
    g_intTotalSets = data.intTotalSets ? data.intTotalSets : 8;
    g_intWorkMs = data.intWorkMs ? data.intWorkMs : 20000;
    g_intRestMs = data.intRestMs ? data.intRestMs : 10000;
    g_intPrepMs = data.intPrepMs ? data.intPrepMs : 5000;
    g_intRemainingMs = data.intRemainingMs;
    g_intTotalPhaseMs = data.intTotalPhaseMs ? data.intTotalPhaseMs : g_intPrepMs;
    g_intIsRunning = data.intIsRunning;
    g_intTargetTime = GetTickCount() + g_intRemainingMs;

    char buf[16];
    if (hEditIntWork) { wsprintfA(buf, "%d", g_intWorkMs / 1000); SetWindowTextA(hEditIntWork, buf); }
    if (hEditIntRest) { wsprintfA(buf, "%d", g_intRestMs / 1000); SetWindowTextA(hEditIntRest, buf); }
    if (hEditIntSets) { wsprintfA(buf, "%d", g_intTotalSets); SetWindowTextA(hEditIntSets, buf); }
    if (hEditIntPrep) { wsprintfA(buf, "%d", g_intPrepMs / 1000); SetWindowTextA(hEditIntPrep, buf); }
    UpdateIntervalDisplay();

    // Switch to loaded mode
    AppMode m = (AppMode)data.mode;
    if (m < MODE_STOPWATCH || m > MODE_INTERVAL) m = MODE_STOPWATCH;
    SwitchMode(m);

    return 1;
}

static int HasSavedState(const char* filename) {
    DWORD attr = GetFileAttributesA(filename);
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

static int HasSeenTutorial(void) {
    return HasSavedState("ktimer_tutorial.dat");
}

static void MarkTutorialSeen(void) {
    HANDLE h = CreateFileA("ktimer_tutorial.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        char buf[16] = "seen\r\n";
        DWORD written = 0;
        WriteFile(h, buf, 6, &written, NULL);
        CloseHandle(h);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hMainWnd = hwnd;

            hBgBrush = CreateSolidBrush(RGB(18, 19, 24));
            hControlBrush = CreateSolidBrush(RGB(26, 28, 35));
            hProgressBarBrush = CreateSolidBrush(RGB(90, 139, 212));

            hFontDisplay = CreateFontA(-S(32), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_MODERN, "Consolas");
            hFontBtn = CreateFontA(-S(14), 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            hFontSmall = CreateFontA(-S(12), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            // Top Bar Tabs
            hTabSW = CreateWindowA("BUTTON", "[ 1 ] SW", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, S(10), S(10), S(66), S(30), hwnd, (HMENU)ID_BTN_SW_TAB, NULL, NULL);
            hTabTM = CreateWindowA("BUTTON", "[ 2 ] Timer", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, S(78), S(10), S(68), S(30), hwnd, (HMENU)ID_BTN_TM_TAB, NULL, NULL);
            hTabMT = CreateWindowA("BUTTON", "[ 3 ] Multi", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, S(148), S(10), S(66), S(30), hwnd, (HMENU)ID_BTN_MT_TAB, NULL, NULL);
            hTabPOMO = CreateWindowA("BUTTON", "[ 4 ] Pomo", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, S(216), S(10), S(66), S(30), hwnd, (HMENU)ID_BTN_POMO_TAB, NULL, NULL);
            hTabINT = CreateWindowA("BUTTON", "[ 5 ] HIIT", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, S(284), S(10), S(66), S(30), hwnd, (HMENU)ID_BTN_INT_TAB, NULL, NULL);
            hBtnHelp = CreateWindowA("BUTTON", "Help [F1]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, S(352), S(10), S(78), S(30), hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);

            // Display & Input Controls
            hDisplay = CreateWindowExA(0, "STATIC", "00:00:00.000", WS_CHILD | WS_VISIBLE | SS_CENTER, S(10), S(48), S(420), S(40), hwnd, NULL, NULL, NULL);
            hTmInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "00:05:00", WS_CHILD | ES_CENTER | ES_AUTOHSCROLL | WS_TABSTOP, S(10), S(48), S(420), S(40), hwnd, (HMENU)ID_EDIT_TM_INPUT, NULL, NULL);

            // Stopwatch Controls
            hBtnStart = CreateWindowA("BUTTON", "Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, S(10), S(96), S(70), S(30), hwnd, (HMENU)ID_BTN_START, NULL, NULL);
            hBtnLap = CreateWindowA("BUTTON", "Lap [L]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, S(82), S(96), S(68), S(30), hwnd, (HMENU)ID_BTN_LAP, NULL, NULL);
            hBtnReset = CreateWindowA("BUTTON", "Reset [R]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, S(152), S(96), S(70), S(30), hwnd, (HMENU)ID_BTN_RESET, NULL, NULL);
            hBtnCopyLaps = CreateWindowA("BUTTON", "Copy [C]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, S(224), S(96), S(72), S(30), hwnd, (HMENU)ID_BTN_COPY_LAPS, NULL, NULL);
            hBtnExportCsv = CreateWindowA("BUTTON", "CSV", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, S(298), S(96), S(64), S(30), hwnd, (HMENU)ID_BTN_EXPORT_CSV, NULL, NULL);
            hBtnExportTxt = CreateWindowA("BUTTON", "TXT", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, S(364), S(96), S(66), S(30), hwnd, (HMENU)ID_BTN_EXPORT_TXT, NULL, NULL);

            hListLaps = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | WS_TABSTOP, S(10), S(134), S(420), S(360), hwnd, (HMENU)ID_LIST_LAPS, NULL, NULL);

            // Timer Presets & Time Nudge Buttons
            hBtnSub1m = CreateWindowA("BUTTON", "-1m [-]", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(215), S(96), S(65), S(30), hwnd, (HMENU)ID_BTN_SUB_1M, NULL, NULL);
            hBtnAdd1m = CreateWindowA("BUTTON", "+1m [+]", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(285), S(96), S(65), S(30), hwnd, (HMENU)ID_BTN_ADD_1M, NULL, NULL);
            hBtnAdd5m = CreateWindowA("BUTTON", "+5m", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(355), S(96), S(75), S(30), hwnd, (HMENU)ID_BTN_ADD_5M, NULL, NULL);

            const char* presetLabels[8] = {"1m", "3m", "5m", "10m", "15m", "25m", "30m", "60m"};
            for (int i = 0; i < 8; i++) {
                hPresets[i] = CreateWindowA("BUTTON", presetLabels[i], WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(10 + (i % 4) * 105), S(136 + (i / 4) * 36), S(95), S(30), hwnd, (HMENU)(INT_PTR)(ID_PRESET_1M + i), NULL, NULL);
                SendMessageA(hPresets[i], WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            }

            // Multi-Timer Controls
            hEditMtName = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Tea", WS_CHILD | ES_AUTOHSCROLL | WS_TABSTOP, S(10), S(48), S(180), S(28), hwnd, (HMENU)ID_EDIT_MT_NAME, NULL, NULL);
            hEditMtTime = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "03:00", WS_CHILD | ES_CENTER | ES_AUTOHSCROLL | WS_TABSTOP, S(195), S(48), S(90), S(28), hwnd, (HMENU)ID_EDIT_MT_TIME, NULL, NULL);
            hBtnMtAdd = CreateWindowA("BUTTON", "+ Add [Enter]", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(290), S(48), S(140), S(28), hwnd, (HMENU)ID_BTN_MT_ADD, NULL, NULL);

            hBtnMtPreset1 = CreateWindowA("BUTTON", "+ Tea 3m", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(10), S(80), S(135), S(26), hwnd, (HMENU)ID_BTN_MT_PRESET1, NULL, NULL);
            hBtnMtPreset2 = CreateWindowA("BUTTON", "+ Eggs 7m", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(150), S(80), S(135), S(26), hwnd, (HMENU)ID_BTN_MT_PRESET2, NULL, NULL);
            hBtnMtPreset3 = CreateWindowA("BUTTON", "+ Nap 20m", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(290), S(80), S(140), S(26), hwnd, (HMENU)ID_BTN_MT_PRESET3, NULL, NULL);

            hListMt = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | WS_TABSTOP, S(10), S(110), S(420), S(350), hwnd, (HMENU)ID_LIST_MT, NULL, NULL);
            hBtnMtStartAll = CreateWindowA("BUTTON", "Start All [S]", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(10), S(466), S(135), S(30), hwnd, (HMENU)ID_BTN_MT_STARTALL, NULL, NULL);
            hBtnMtPauseAll = CreateWindowA("BUTTON", "Pause All [P]", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(150), S(466), S(135), S(30), hwnd, (HMENU)ID_BTN_MT_PAUSEALL, NULL, NULL);
            hBtnMtDel = CreateWindowA("BUTTON", "Delete [Del]", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(290), S(466), S(140), S(30), hwnd, (HMENU)ID_BTN_MT_DEL, NULL, NULL);

            // Pomodoro Controls
            hBtnPomoStart = CreateWindowA("BUTTON", "Start", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(10), S(96), S(135), S(32), hwnd, (HMENU)ID_BTN_POMO_START, NULL, NULL);
            hBtnPomoSkip = CreateWindowA("BUTTON", "Skip Phase [S]", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(150), S(96), S(135), S(32), hwnd, (HMENU)ID_BTN_POMO_SKIP, NULL, NULL);
            hBtnPomoReset = CreateWindowA("BUTTON", "Reset [R]", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(290), S(96), S(140), S(32), hwnd, (HMENU)ID_BTN_POMO_RESET, NULL, NULL);
            hStaticStats = CreateWindowExA(0, "STATIC", "WORK SESSION\nDone: 0 | Focus: 0 mins", WS_CHILD | SS_CENTER, S(10), S(145), S(420), S(60), hwnd, NULL, NULL, NULL);

            // Interval / HIIT Controls
            hBtnIntStart = CreateWindowA("BUTTON", "Start", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(10), S(96), S(135), S(32), hwnd, (HMENU)ID_BTN_INT_START, NULL, NULL);
            hBtnIntSkip = CreateWindowA("BUTTON", "Skip Phase [S]", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(150), S(96), S(135), S(32), hwnd, (HMENU)ID_BTN_INT_SKIP, NULL, NULL);
            hBtnIntReset = CreateWindowA("BUTTON", "Reset [R]", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(290), S(96), S(140), S(32), hwnd, (HMENU)ID_BTN_INT_RESET, NULL, NULL);
            hStaticIntStats = CreateWindowExA(0, "STATIC", "PREPARE\nSet 1 of 8 | Work: 20s Rest: 10s", WS_CHILD | SS_CENTER, S(10), S(140), S(420), S(48), hwnd, NULL, NULL, NULL);

            hStaticIntLabels = CreateWindowExA(0, "STATIC", "Work(s)       Rest(s)       Sets          Prep(s)", WS_CHILD | SS_CENTER, S(10), S(195), S(420), S(18), hwnd, NULL, NULL, NULL);
            hEditIntWork = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "20", WS_CHILD | ES_CENTER | ES_NUMBER | WS_TABSTOP, S(10), S(215), S(95), S(26), hwnd, (HMENU)ID_EDIT_INT_WORK, NULL, NULL);
            hEditIntRest = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "10", WS_CHILD | ES_CENTER | ES_NUMBER | WS_TABSTOP, S(115), S(215), S(95), S(26), hwnd, (HMENU)ID_EDIT_INT_REST, NULL, NULL);
            hEditIntSets = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "8", WS_CHILD | ES_CENTER | ES_NUMBER | WS_TABSTOP, S(220), S(215), S(95), S(26), hwnd, (HMENU)ID_EDIT_INT_SETS, NULL, NULL);
            hEditIntPrep = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "5", WS_CHILD | ES_CENTER | ES_NUMBER | WS_TABSTOP, S(325), S(215), S(95), S(26), hwnd, (HMENU)ID_EDIT_INT_PREP, NULL, NULL);

            hBtnPresetTabata = CreateWindowA("BUTTON", "Tabata 20/10x8", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(10), S(250), S(130), S(28), hwnd, (HMENU)ID_PRESET_TABATA, NULL, NULL);
            hBtnPresetHiit   = CreateWindowA("BUTTON", "HIIT 30/15x10", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(150), S(250), S(130), S(28), hwnd, (HMENU)ID_PRESET_HIIT30, NULL, NULL);
            hBtnPresetBoxing = CreateWindowA("BUTTON", "Boxing 3m/1mx3", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, S(290), S(250), S(140), S(28), hwnd, (HMENU)ID_PRESET_BOXING, NULL, NULL);

            hStatusLabel = CreateWindowExA(0, "STATIC", g_statusMsg, WS_CHILD | WS_VISIBLE | SS_CENTER, S(10), S(500), S(420), S(18), hwnd, (HMENU)ID_STATIC_STATUS, NULL, NULL);
            hBtnSave = CreateWindowA("BUTTON", "Save [F5]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, S(10), S(520), S(75), S(22), hwnd, (HMENU)ID_BTN_SAVE, NULL, NULL);
            hBtnLoad = CreateWindowA("BUTTON", "Load [F9]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, S(88), S(520), S(75), S(22), hwnd, (HMENU)ID_BTN_LOAD, NULL, NULL);
            hHelpLabel = CreateWindowExA(0, "STATIC", "Press 'H' or F1 for Help | Space: Start/Pause", WS_CHILD | WS_VISIBLE | SS_CENTER, S(166), S(522), S(264), S(18), hwnd, NULL, NULL, NULL);

            // Font Application
            SendMessageA(hTabSW, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hTabTM, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hTabMT, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hTabPOMO, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hTabINT, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnHelp, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnSave, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hBtnLoad, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hDisplay, WM_SETFONT, (WPARAM)hFontDisplay, TRUE);
            SendMessageA(hTmInput, WM_SETFONT, (WPARAM)hFontDisplay, TRUE);
            SendMessageA(hBtnStart, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnLap, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnReset, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnCopyLaps, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnExportCsv, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnExportTxt, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hListLaps, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnSub1m, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnAdd1m, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnAdd5m, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hEditMtName, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hEditMtTime, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnMtAdd, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnMtPreset1, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hBtnMtPreset2, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hBtnMtPreset3, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hListMt, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnMtStartAll, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnMtPauseAll, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnMtDel, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnPomoStart, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnPomoSkip, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnPomoReset, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hStaticStats, WM_SETFONT, (WPARAM)hFontBtn, TRUE);

            SendMessageA(hBtnIntStart, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnIntSkip, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnIntReset, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hStaticIntStats, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hStaticIntLabels, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hEditIntWork, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hEditIntRest, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hEditIntSets, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hEditIntPrep, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessageA(hBtnPresetTabata, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hBtnPresetHiit, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hBtnPresetBoxing, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

            SendMessageA(hStatusLabel, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hHelpLabel, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

            SetTimer(hwnd, 1, 25, NULL);
            SwitchMode(MODE_STOPWATCH);
            break;
        }
        case WM_COMMAND: {
            WORD id = LOWORD(wParam);
            if (id == ID_BTN_SW_TAB) SwitchMode(MODE_STOPWATCH);
            else if (id == ID_BTN_TM_TAB) SwitchMode(MODE_TIMER);
            else if (id == ID_BTN_MT_TAB) SwitchMode(MODE_MULTI);
            else if (id == ID_BTN_POMO_TAB) SwitchMode(MODE_POMODORO);
            else if (id == ID_BTN_INT_TAB) SwitchMode(MODE_INTERVAL);
            else if (id == ID_BTN_HELP) ShowHelpDialog(hwnd);
            else if (id == ID_BTN_SAVE) {
                if (SaveStateToFile("ktimer.dat")) {
                    ShowNativeStatus("★ Timer state quicksaved to ktimer.dat [F5]");
                } else {
                    ShowNativeStatus("⚠ Failed to quicksave state.");
                }
            }
            else if (id == ID_BTN_LOAD) {
                if (LoadStateFromFile("ktimer.dat")) {
                    ShowNativeStatus("★ Restored saved state from ktimer.dat [F9]");
                } else {
                    ShowNativeStatus("⚠ No quicksave state found (ktimer.dat).");
                }
            }

            // Stopwatch Handlers
            else if (id == ID_BTN_START && g_mode == MODE_STOPWATCH) {
                if (g_swIsRunning) {
                    g_swIsRunning = 0;
                    g_swElapsed += GetTickCount() - g_swStartTime;
                    SetWindowTextA(hBtnStart, "Start");
                } else {
                    g_swIsRunning = 1;
                    g_swStartTime = GetTickCount();
                    SetWindowTextA(hBtnStart, "Stop");
                }
            } else if (id == ID_BTN_LAP && g_mode == MODE_STOPWATCH) {
                DWORD currentMs = g_swElapsed + (g_swIsRunning ? (GetTickCount() - g_swStartTime) : 0);
                if (currentMs > 0 && g_lapCount < MAX_LAPS) {
                    DWORD prevTotal = g_lapCount > 0 ? g_laps[g_lapCount - 1].totalMs : 0;
                    DWORD splitMs = currentMs - prevTotal;

                    g_lapCount++;
                    g_laps[g_lapCount - 1].id = g_lapCount;
                    g_laps[g_lapCount - 1].totalMs = currentMs;
                    g_laps[g_lapCount - 1].splitMs = splitMs;

                    char sBuf[32], tBuf[32], lapBuf[128];
                    FormatMsToStopwatch(splitMs, sBuf, sizeof(sBuf));
                    FormatMsToStopwatch(currentMs, tBuf, sizeof(tBuf));
                    wsprintfA(lapBuf, "Lap %d | Split: %s | Total: %s", g_lapCount, sBuf, tBuf);
                    SendMessageA(hListLaps, LB_ADDSTRING, 0, (LPARAM)lapBuf);
                    SendMessageA(hListLaps, LB_SETTOPINDEX, g_lapCount - 1, 0);
                }
            } else if (id == ID_BTN_RESET && g_mode == MODE_STOPWATCH) {
                g_swIsRunning = 0;
                g_swElapsed = 0;
                g_lapCount = 0;
                SetWindowTextA(hBtnStart, "Start");
                SendMessageA(hListLaps, LB_RESETCONTENT, 0, 0);
                UpdateStopwatchDisplay();
            } else if (id == ID_BTN_EXPORT_CSV) {
                ExportLapsToFile("csv");
            } else if (id == ID_BTN_EXPORT_TXT) {
                ExportLapsToFile("txt");
            } else if (id == ID_BTN_COPY_LAPS) {
                CopyLapsToClipboard();
            }

            // Single Timer Handlers
            else if (id == ID_BTN_START && g_mode == MODE_TIMER) {
                if (g_tmIsRunning) {
                    g_tmIsRunning = 0;
                    SetWindowTextA(hBtnStart, "Resume");
                } else {
                    if (IsWindowVisible(hTmInput)) {
                        char inputStr[64] = {0};
                        GetWindowTextA(hTmInput, inputStr, sizeof(inputStr) - 1);
                        DWORD parsedMs = ParseTimerInput(inputStr);
                        if (parsedMs == 0) parsedMs = 300000;
                        g_tmTotalMs = parsedMs;
                        g_tmRemainingMs = parsedMs;
                        ShowWindow(hTmInput, SW_HIDE);
                        ShowWindow(hDisplay, SW_SHOW);
                    }
                    if (g_tmRemainingMs > 0) {
                        g_tmIsRunning = 1;
                        g_tmTargetTime = GetTickCount() + g_tmRemainingMs;
                        SetWindowTextA(hBtnStart, "Pause");
                    }
                }
            } else if (id == ID_BTN_RESET && g_mode == MODE_TIMER) {
                g_tmIsRunning = 0;
                g_tmRemainingMs = 300000;
                g_tmTotalMs = 300000;
                SetWindowTextA(hBtnStart, "Start");
                SetWindowTextA(hTmInput, "00:05:00");
                ShowWindow(hTmInput, SW_SHOW);
                ShowWindow(hDisplay, SW_HIDE);
            } else if (id >= ID_PRESET_1M && id <= ID_PRESET_60M) {
                const char* presetTimes[8] = {"00:01:00", "00:03:00", "00:05:00", "00:10:00", "00:15:00", "00:25:00", "00:30:00", "01:00:00"};
                int idx = id - ID_PRESET_1M;
                SetWindowTextA(hTmInput, presetTimes[idx]);
            } else if (id == ID_BTN_SUB_1M) {
                if (g_tmIsRunning || (g_tmRemainingMs > 0 && !IsWindowVisible(hTmInput))) {
                    g_tmRemainingMs = (g_tmRemainingMs > 60000) ? (g_tmRemainingMs - 60000) : 1000;
                    if (g_tmIsRunning) g_tmTargetTime = GetTickCount() + g_tmRemainingMs;
                    UpdateTimerDisplay();
                    ShowNativeStatus("-1m adjustment applied");
                } else {
                    char inBuf[64] = {0};
                    GetWindowTextA(hTmInput, inBuf, sizeof(inBuf) - 1);
                    DWORD ms = ParseTimerInput(inBuf);
                    ms = (ms > 60000) ? (ms - 60000) : 60000;
                    FormatMsToTimer(ms, inBuf, sizeof(inBuf));
                    SetWindowTextA(hTmInput, inBuf);
                    ShowNativeStatus("Timer set to input duration");
                }
            } else if (id == ID_BTN_ADD_1M) {
                if (g_tmIsRunning || (g_tmRemainingMs > 0 && !IsWindowVisible(hTmInput))) {
                    g_tmRemainingMs += 60000;
                    if (g_tmRemainingMs > g_tmTotalMs) g_tmTotalMs = g_tmRemainingMs;
                    if (g_tmIsRunning) g_tmTargetTime = GetTickCount() + g_tmRemainingMs;
                    UpdateTimerDisplay();
                    ShowNativeStatus("+1m adjustment applied");
                } else {
                    char inBuf[64] = {0};
                    GetWindowTextA(hTmInput, inBuf, sizeof(inBuf) - 1);
                    DWORD ms = ParseTimerInput(inBuf) + 60000;
                    FormatMsToTimer(ms, inBuf, sizeof(inBuf));
                    SetWindowTextA(hTmInput, inBuf);
                    ShowNativeStatus("Timer set to input duration");
                }
            } else if (id == ID_BTN_ADD_5M) {
                if (g_tmIsRunning || (g_tmRemainingMs > 0 && !IsWindowVisible(hTmInput))) {
                    g_tmRemainingMs += 300000;
                    if (g_tmRemainingMs > g_tmTotalMs) g_tmTotalMs = g_tmRemainingMs;
                    if (g_tmIsRunning) g_tmTargetTime = GetTickCount() + g_tmRemainingMs;
                    UpdateTimerDisplay();
                    ShowNativeStatus("+5m adjustment applied");
                } else {
                    char inBuf[64] = {0};
                    GetWindowTextA(hTmInput, inBuf, sizeof(inBuf) - 1);
                    DWORD ms = ParseTimerInput(inBuf) + 300000;
                    FormatMsToTimer(ms, inBuf, sizeof(inBuf));
                    SetWindowTextA(hTmInput, inBuf);
                    ShowNativeStatus("Timer set to input duration");
                }
            }

            // Multi-Timer Handlers
            else if (id == ID_BTN_MT_ADD) {
                if (g_multiTimerCount < MAX_MULTI_TIMERS) {
                    char nameBuf[32] = {0};
                    char timeBuf[32] = {0};
                    GetWindowTextA(hEditMtName, nameBuf, sizeof(nameBuf) - 1);
                    GetWindowTextA(hEditMtTime, timeBuf, sizeof(timeBuf) - 1);
                    DWORD ms = ParseTimerInput(timeBuf);
                    if (ms > 0) {
                        lstrcpynA(g_multiTimers[g_multiTimerCount].name, nameBuf[0] ? nameBuf : "Timer", sizeof(g_multiTimers[g_multiTimerCount].name));
                        g_multiTimers[g_multiTimerCount].totalMs = ms;
                        g_multiTimers[g_multiTimerCount].remainingMs = ms;
                        g_multiTimers[g_multiTimerCount].lastTick = GetTickCount();
                        g_multiTimers[g_multiTimerCount].isRunning = 1;
                        g_multiTimerCount++;
                        SwitchMode(MODE_MULTI);
                    }
                }
            } else if (id == ID_BTN_MT_PRESET1) {
                if (g_multiTimerCount < MAX_MULTI_TIMERS) {
                    lstrcpyA(g_multiTimers[g_multiTimerCount].name, "Green Tea");
                    g_multiTimers[g_multiTimerCount].totalMs = 180000;
                    g_multiTimers[g_multiTimerCount].remainingMs = 180000;
                    g_multiTimers[g_multiTimerCount].lastTick = GetTickCount();
                    g_multiTimers[g_multiTimerCount].isRunning = 1;
                    g_multiTimerCount++;
                    SwitchMode(MODE_MULTI);
                    ShowNativeStatus("Started Green Tea 3m timer");
                }
            } else if (id == ID_BTN_MT_PRESET2) {
                if (g_multiTimerCount < MAX_MULTI_TIMERS) {
                    lstrcpyA(g_multiTimers[g_multiTimerCount].name, "Boiled Eggs");
                    g_multiTimers[g_multiTimerCount].totalMs = 420000;
                    g_multiTimers[g_multiTimerCount].remainingMs = 420000;
                    g_multiTimers[g_multiTimerCount].lastTick = GetTickCount();
                    g_multiTimers[g_multiTimerCount].isRunning = 1;
                    g_multiTimerCount++;
                    SwitchMode(MODE_MULTI);
                    ShowNativeStatus("Started Boiled Eggs 7m timer");
                }
            } else if (id == ID_BTN_MT_PRESET3) {
                if (g_multiTimerCount < MAX_MULTI_TIMERS) {
                    lstrcpyA(g_multiTimers[g_multiTimerCount].name, "Power Nap");
                    g_multiTimers[g_multiTimerCount].totalMs = 1200000;
                    g_multiTimers[g_multiTimerCount].remainingMs = 1200000;
                    g_multiTimers[g_multiTimerCount].lastTick = GetTickCount();
                    g_multiTimers[g_multiTimerCount].isRunning = 1;
                    g_multiTimerCount++;
                    SwitchMode(MODE_MULTI);
                    ShowNativeStatus("Started Power Nap 20m timer");
                }
            } else if (id == ID_BTN_MT_STARTALL) {
                DWORD now = GetTickCount();
                for (int i = 0; i < g_multiTimerCount; i++) {
                    if (g_multiTimers[i].remainingMs > 0) {
                        g_multiTimers[i].isRunning = 1;
                        g_multiTimers[i].lastTick = now;
                    }
                }
            } else if (id == ID_BTN_MT_PAUSEALL) {
                for (int i = 0; i < g_multiTimerCount; i++) {
                    g_multiTimers[i].isRunning = 0;
                }
            } else if (id == ID_BTN_MT_DEL) {
                int sel = (int)SendMessageA(hListMt, LB_GETCURSEL, 0, 0);
                if (sel >= 0 && sel < g_multiTimerCount) {
                    for (int i = sel; i < g_multiTimerCount - 1; i++) {
                        g_multiTimers[i] = g_multiTimers[i + 1];
                    }
                    g_multiTimerCount--;
                    SwitchMode(MODE_MULTI);
                }
            }

            // Pomodoro Handlers
            else if (id == ID_BTN_POMO_START) {
                if (g_pomoIsRunning) {
                    g_pomoIsRunning = 0;
                    SetWindowTextA(hBtnPomoStart, "Resume");
                } else {
                    if (g_pomoRemainingMs > 0) {
                        g_pomoIsRunning = 1;
                        g_pomoTargetTime = GetTickCount() + g_pomoRemainingMs;
                        SetWindowTextA(hBtnPomoStart, "Pause");
                    }
                }
            } else if (id == ID_BTN_POMO_SKIP) {
                g_pomoIsRunning = 0;
                SetWindowTextA(hBtnPomoStart, "Start");
                if (g_pomoState == POMO_WORK) {
                    g_pomoState = POMO_SHORT_BREAK;
                    g_pomoTotalMs = 5 * 60 * 1000;
                } else {
                    g_pomoState = POMO_WORK;
                    g_pomoTotalMs = 25 * 60 * 1000;
                }
                g_pomoRemainingMs = g_pomoTotalMs;
                UpdatePomodoroDisplay();
            } else if (id == ID_BTN_POMO_RESET) {
                g_pomoIsRunning = 0;
                g_pomoState = POMO_WORK;
                g_pomoCycleCount = 1;
                g_pomoTotalMs = 25 * 60 * 1000;
                g_pomoRemainingMs = g_pomoTotalMs;
                SetWindowTextA(hBtnPomoStart, "Start");
                UpdatePomodoroDisplay();
            }

            // Interval Handlers
            else if (id == ID_BTN_INT_START && g_mode == MODE_INTERVAL) {
                if (g_intIsRunning) {
                    g_intIsRunning = 0;
                    SetWindowTextA(hBtnIntStart, "Resume");
                } else {
                    char bufW[16], bufR[16], bufS[16], bufP[16];
                    GetWindowTextA(hEditIntWork, bufW, sizeof(bufW));
                    GetWindowTextA(hEditIntRest, bufR, sizeof(bufR));
                    GetWindowTextA(hEditIntSets, bufS, sizeof(bufS));
                    GetWindowTextA(hEditIntPrep, bufP, sizeof(bufP));
                    int w = SimpleStrToInt(bufW);
                    int r = SimpleStrToInt(bufR);
                    int s = SimpleStrToInt(bufS);
                    int p = SimpleStrToInt(bufP);
                    if (w <= 0) w = 20;
                    if (r < 0) r = 10;
                    if (s <= 0) s = 8;
                    if (p < 0) p = 5;

                    g_intWorkMs = w * 1000;
                    g_intRestMs = r * 1000;
                    g_intTotalSets = s;
                    g_intPrepMs = p * 1000;

                    if (g_intPhase == INT_PHASE_DONE) {
                        g_intPhase = INT_PHASE_PREP;
                        g_intCurrentSet = 1;
                        g_intRemainingMs = g_intPrepMs;
                        g_intTotalPhaseMs = g_intPrepMs;
                    }

                    if (g_intRemainingMs > 0) {
                        g_intIsRunning = 1;
                        g_intTargetTime = GetTickCount() + g_intRemainingMs;
                        SetWindowTextA(hBtnIntStart, "Pause");
                    }
                }
            } else if (id == ID_BTN_INT_SKIP && g_mode == MODE_INTERVAL) {
                if (g_intPhase == INT_PHASE_PREP) {
                    g_intPhase = INT_PHASE_WORK;
                    g_intCurrentSet = 1;
                    g_intTotalPhaseMs = g_intWorkMs;
                    g_intRemainingMs = g_intWorkMs;
                } else if (g_intPhase == INT_PHASE_WORK) {
                    if (g_intCurrentSet < g_intTotalSets) {
                        g_intPhase = INT_PHASE_REST;
                        g_intTotalPhaseMs = g_intRestMs;
                        g_intRemainingMs = g_intRestMs;
                    } else {
                        g_intPhase = INT_PHASE_DONE;
                        g_intIsRunning = 0;
                        SetWindowTextA(hBtnIntStart, "Start");
                    }
                } else if (g_intPhase == INT_PHASE_REST) {
                    g_intCurrentSet++;
                    g_intPhase = INT_PHASE_WORK;
                    g_intTotalPhaseMs = g_intWorkMs;
                    g_intRemainingMs = g_intWorkMs;
                }
                if (g_intIsRunning) {
                    g_intTargetTime = GetTickCount() + g_intRemainingMs;
                }
                UpdateIntervalDisplay();
            } else if (id == ID_BTN_INT_RESET && g_mode == MODE_INTERVAL) {
                g_intIsRunning = 0;
                char bufW[16], bufR[16], bufS[16], bufP[16];
                GetWindowTextA(hEditIntWork, bufW, sizeof(bufW));
                GetWindowTextA(hEditIntRest, bufR, sizeof(bufR));
                GetWindowTextA(hEditIntSets, bufS, sizeof(bufS));
                GetWindowTextA(hEditIntPrep, bufP, sizeof(bufP));
                int w = SimpleStrToInt(bufW); if (w <= 0) w = 20;
                int r = SimpleStrToInt(bufR); if (r < 0) r = 10;
                int s = SimpleStrToInt(bufS); if (s <= 0) s = 8;
                int p = SimpleStrToInt(bufP); if (p < 0) p = 5;
                g_intWorkMs = w * 1000;
                g_intRestMs = r * 1000;
                g_intTotalSets = s;
                g_intPrepMs = p * 1000;

                g_intPhase = INT_PHASE_PREP;
                g_intCurrentSet = 1;
                g_intRemainingMs = g_intPrepMs;
                g_intTotalPhaseMs = g_intPrepMs;
                SetWindowTextA(hBtnIntStart, "Start");
                UpdateIntervalDisplay();
            } else if (id == ID_PRESET_TABATA) {
                SetWindowTextA(hEditIntWork, "20");
                SetWindowTextA(hEditIntRest, "10");
                SetWindowTextA(hEditIntSets, "8");
                SetWindowTextA(hEditIntPrep, "5");
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_INT_RESET, 0), 0);
            } else if (id == ID_PRESET_HIIT30) {
                SetWindowTextA(hEditIntWork, "30");
                SetWindowTextA(hEditIntRest, "15");
                SetWindowTextA(hEditIntSets, "10");
                SetWindowTextA(hEditIntPrep, "5");
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_INT_RESET, 0), 0);
            } else if (id == ID_PRESET_BOXING) {
                SetWindowTextA(hEditIntWork, "180");
                SetWindowTextA(hEditIntRest, "60");
                SetWindowTextA(hEditIntSets, "3");
                SetWindowTextA(hEditIntPrep, "10");
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_INT_RESET, 0), 0);
            }

            break;
        }
        case WM_TIMER: {
            if (g_swIsRunning) UpdateStopwatchDisplay();
            if (g_tmIsRunning) UpdateTimerDisplay();
            if (g_pomoIsRunning) UpdatePomodoroDisplay();
            if (g_intIsRunning) UpdateIntervalDisplay();
            UpdateMultiTimers();

            if (g_statusTimer > 0) {
                g_statusTimer--;
                if (g_statusTimer == 0) {
                    if (g_mode == MODE_STOPWATCH) lstrcpyA(g_statusMsg, "Stopwatch - Space: Start, L: Lap, C: Copy, F1: Help");
                    else if (g_mode == MODE_TIMER) lstrcpyA(g_statusMsg, "Timer - Space: Start, +/-: Adj, R: Reset, F1: Help");
                    else if (g_mode == MODE_MULTI) lstrcpyA(g_statusMsg, "Multi - Enter: Add, S: Start All, P: Pause, Del: Remove");
                    else if (g_mode == MODE_POMODORO) lstrcpyA(g_statusMsg, "Pomodoro - Space: Start, S: Skip, R: Reset, F1: Help");
                    else if (g_mode == MODE_INTERVAL) lstrcpyA(g_statusMsg, "HIIT - Space: Start, S: Skip, R: Reset, F1: Help");
                    if (hStatusLabel) SetWindowTextA(hStatusLabel, g_statusMsg);
                }
            }
            break;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            HWND hCtrl = (HWND)lParam;
            SetBkColor(hdc, RGB(18, 19, 24));
            if (hCtrl == hStatusLabel) {
                SetTextColor(hdc, RGB(76, 217, 100)); // status feedback
            } else if (hCtrl == hHelpLabel || hCtrl == hStaticIntLabels) {
                SetTextColor(hdc, RGB(140, 147, 164)); // subtle muted labels
            } else {
                SetTextColor(hdc, RGB(90, 139, 212)); // accent blue
            }
            return (LRESULT)hBgBrush;
        }
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(26, 28, 35));
            SetTextColor(hdc, RGB(240, 242, 245));
            return (LRESULT)hControlBrush;
        }
        case WM_KEYDOWN: {
            if (wParam == VK_F5) {
                if (SaveStateToFile("ktimer.dat")) {
                    ShowNativeStatus("★ Timer state quicksaved to ktimer.dat [F5]");
                } else {
                    ShowNativeStatus("⚠ Failed to quicksave state.");
                }
                return 0;
            } else if (wParam == VK_F9) {
                if (LoadStateFromFile("ktimer.dat")) {
                    ShowNativeStatus("★ Restored saved state from ktimer.dat [F9]");
                } else {
                    ShowNativeStatus("⚠ No quicksave state found (ktimer.dat).");
                }
                return 0;
            }
            break;
        }
        case WM_DESTROY: {
            SaveStateToFile("ktimer.dat");
            KillTimer(hwnd, 1);
            if (hFontDisplay) DeleteObject(hFontDisplay);
            if (hFontBtn) DeleteObject(hFontBtn);
            if (hFontSmall) DeleteObject(hFontSmall);
            if (hBgBrush) DeleteObject(hBgBrush);
            if (hControlBrush) DeleteObject(hControlBrush);
            if (hProgressBarBrush) DeleteObject(hProgressBarBrush);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    SetProcessDPIAware();
    
    HDC hdc = GetDC(NULL);
    g_dpiScale = GetDeviceCaps(hdc, 88); // LOGPIXELSX is 88
    if (g_dpiScale < 96) g_dpiScale = 96;
    g_dpiScale = (g_dpiScale * 100) / 96;
    ReleaseDC(NULL, hdc);

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KTimerClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(18, 19, 24));

    RegisterClassA(&wc);
    
    RECT rc = {0, 0, S(460), S(580)};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);
    
    HWND hwnd = CreateWindowExA(0, "KTimerClass", "KTimer - Stopwatch [Space: Start, L: Lap, C: Copy, F1: Help]", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    if (HasSavedState("ktimer.dat")) {
        if (LoadStateFromFile("ktimer.dat")) {
            ShowNativeStatus("★ Restored saved state from ktimer.dat [F9]");
        }
    } else if (!HasSeenTutorial()) {
        ShowHelpDialog(hwnd);
        MarkTutorialSeen();
        ShowNativeStatus("Welcome to KTimer! Space: Start, F1: Help");
    }

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_F5) {
                if (SaveStateToFile("ktimer.dat")) {
                    ShowNativeStatus("★ Timer state quicksaved to ktimer.dat [F5]");
                } else {
                    ShowNativeStatus("⚠ Failed to quicksave state.");
                }
                continue;
            }
            if (msg.wParam == VK_F9) {
                if (LoadStateFromFile("ktimer.dat")) {
                    ShowNativeStatus("★ Restored saved state from ktimer.dat [F9]");
                } else {
                    ShowNativeStatus("⚠ No quicksave state found (ktimer.dat).");
                }
                continue;
            }

            HWND hFocus = GetFocus();
            char className[32] = {0};
            GetClassNameA(hFocus, className, sizeof(className));
            int isEdit = (lstrcmpiA(className, "EDIT") == 0);

            if (msg.wParam == VK_F1 || (!isEdit && (msg.wParam == 'H' || msg.wParam == 'h'))) {
                ShowHelpDialog(hwnd);
                continue;
            }

            if (isEdit && msg.wParam == VK_RETURN) {
                if (hFocus == hTmInput) {
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_START, 0), (LPARAM)hBtnStart);
                    continue;
                } else if (hFocus == hEditMtName || hFocus == hEditMtTime) {
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_MT_ADD, 0), (LPARAM)hBtnMtAdd);
                    continue;
                }
            }

            if (!isEdit) {
                int hasModifier = (GetKeyState(VK_CONTROL) < 0) || (GetKeyState(VK_MENU) < 0);
                if (hasModifier) {
                    if ((GetKeyState(VK_CONTROL) < 0) && (msg.wParam == 'C' || msg.wParam == 'c') && g_mode == MODE_STOPWATCH) {
                        SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_COPY_LAPS, 0), (LPARAM)hBtnCopyLaps);
                        continue;
                    }
                    if (!IsDialogMessage(hwnd, &msg)) {
                        TranslateMessage(&msg);
                        DispatchMessageA(&msg);
                    }
                    continue;
                }

                if (msg.wParam >= '1' && msg.wParam <= '5') {
                    if (msg.wParam == '1') SwitchMode(MODE_STOPWATCH);
                    else if (msg.wParam == '2') SwitchMode(MODE_TIMER);
                    else if (msg.wParam == '3') SwitchMode(MODE_MULTI);
                    else if (msg.wParam == '4') SwitchMode(MODE_POMODORO);
                    else if (msg.wParam == '5') SwitchMode(MODE_INTERVAL);
                    continue;
                }

                if (msg.wParam == 'L' || msg.wParam == 'l') {
                    if (g_mode == MODE_STOPWATCH) {
                        SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_LAP, 0), (LPARAM)hBtnLap);
                        continue;
                    }
                }

                if (msg.wParam == 'C' || msg.wParam == 'c') {
                    if (g_mode == MODE_STOPWATCH) {
                        SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_COPY_LAPS, 0), (LPARAM)hBtnCopyLaps);
                        continue;
                    }
                }

                if (msg.wParam == 'S' || msg.wParam == 's') {
                    if (g_mode == MODE_POMODORO) {
                        SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_POMO_SKIP, 0), (LPARAM)hBtnPomoSkip);
                        continue;
                    } else if (g_mode == MODE_INTERVAL) {
                        SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_INT_SKIP, 0), (LPARAM)hBtnIntSkip);
                        continue;
                    } else if (g_mode == MODE_MULTI) {
                        SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_MT_STARTALL, 0), (LPARAM)hBtnMtStartAll);
                        continue;
                    }
                }

                if (msg.wParam == 'P' || msg.wParam == 'p') {
                    if (g_mode == MODE_MULTI) {
                        SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_MT_PAUSEALL, 0), (LPARAM)hBtnMtPauseAll);
                        continue;
                    }
                }

                if (msg.wParam == VK_OEM_PLUS || msg.wParam == VK_ADD || msg.wParam == '=') {
                    if (g_mode == MODE_TIMER) {
                        SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_ADD_1M, 0), (LPARAM)hBtnAdd1m);
                        continue;
                    }
                }

                if (msg.wParam == VK_OEM_MINUS || msg.wParam == VK_SUBTRACT || msg.wParam == '-') {
                    if (g_mode == MODE_TIMER) {
                        SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_SUB_1M, 0), (LPARAM)hBtnSub1m);
                        continue;
                    }
                }

                if (msg.wParam == VK_DELETE) {
                    if (g_mode == MODE_MULTI) {
                        SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_MT_DEL, 0), (LPARAM)hBtnMtDel);
                        continue;
                    }
                }

                if (msg.wParam == 'R' || msg.wParam == 'r') {
                    if (g_mode == MODE_STOPWATCH || g_mode == MODE_TIMER) {
                        SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_RESET, 0), (LPARAM)hBtnReset);
                    } else if (g_mode == MODE_POMODORO) {
                        SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_POMO_RESET, 0), (LPARAM)hBtnPomoReset);
                    } else if (g_mode == MODE_INTERVAL) {
                        SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_INT_RESET, 0), (LPARAM)hBtnIntReset);
                    }
                    continue;
                }

                if (msg.wParam == VK_SPACE) {
                    if (lstrcmpiA(className, "BUTTON") != 0) {
                        if (g_mode == MODE_STOPWATCH || g_mode == MODE_TIMER) {
                            SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_START, 0), (LPARAM)hBtnStart);
                            continue;
                        } else if (g_mode == MODE_POMODORO) {
                            SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_POMO_START, 0), (LPARAM)hBtnPomoStart);
                            continue;
                        } else if (g_mode == MODE_INTERVAL) {
                            SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_INT_START, 0), (LPARAM)hBtnIntStart);
                            continue;
                        }
                    }
                }
            }
        }
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    if (wc.hbrBackground) DeleteObject(wc.hbrBackground);
    ExitProcess(0);
}
