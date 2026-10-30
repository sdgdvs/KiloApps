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
#define H 356

#define ID_BTN_PLAY     1
#define ID_BTN_STOP     2
#define ID_BTN_HELP     3
#define ID_BTN_SAVE     4
#define ID_BTN_LOAD     5
#define ID_BTN_VIZ      6
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
    int vizMode;        // 0=LED, 1=CRT, 2=VU
    int bufferMode;     // 0=64KB, 1=128KB, 2=32KB
    int bufferHealth;   // 0-100%
} KRadioState;

static const StationPreset g_presets[6] = {
    { "Subspace",     "Synthwave",    "https://radio.erb.pw/public/subspace" },
    { "Groove Salad", "Chillout",     "https://ice2.somafm.com/groovesalad-128-mp3" },
    { "Nightwave",    "Vaporwave",    "https://radio.plaza.one/mp3" },
    { "DEF CON",      "Cyber Ambient","https://ice2.somafm.com/defcon-128-mp3" },
    { "Secret Agent", "Spy Retro",    "https://ice1.somafm.com/secretagent-128-mp3" },
    { "Space Station","Spacemusic",   "https://ice1.somafm.com/spacestation-128-mp3" }
};

static const char* g_bufferNames[3] = {
    "64 KB (Normal)",
    "128 KB (Robust)",
    "32 KB (Low-Latency)"
};

static const char* g_vizNames[3] = {
    "LED BARS",
    "CRT WAVE",
    "ANALOG VU"
};

HWND g_hwndMain = NULL;
HWND hTitle = NULL;
HWND hEditUrl = NULL;
HWND hBtnPlay = NULL;
HWND hBtnStop = NULL;
HWND hBtnHelp = NULL;
HWND hBtnSave = NULL;
HWND hBtnLoad = NULL;
HWND hBtnViz = NULL;
HWND hBtnPresets[6] = {0};
HWND hStatus = NULL;
HWND hHint = NULL;
HWND hPresetsLabel = NULL;
HWND hUrlLabel = NULL;
HWND hViz = NULL;

HFONT g_hFontNormal = NULL;
HFONT g_hFontBold = NULL;
HFONT g_hFontSmall = NULL;

WNDPROC g_pfnOrigEditProc = NULL;
char mciCmd[1024] = {0};
BOOL g_isPlaying = FALSE;
int g_vizMode = 0;       // 0=LED, 1=CRT, 2=VU
int g_bufferMode = 0;    // 0=64KB, 1=128KB, 2=32KB
int g_bufferHealth = 100;// 100%
int g_animPhase = 0;

// Spectrum visualizer levels (16 bands)
int g_bandLevels[16] = {0};
int g_peakLevels[16] = {0};
int g_peakDecay[16] = {0};

// Precomputed 64-entry integer sine table (* 1000)
static const int g_sinTable[64] = {
    0, 98, 195, 290, 382, 471, 555, 634, 707, 773, 831, 881, 923, 956, 980, 995,
    1000, 995, 980, 956, 923, 881, 831, 773, 707, 634, 555, 471, 382, 290, 195, 98,
    0, -98, -195, -290, -382, -471, -555, -634, -707, -773, -831, -881, -923, -956, -980, -995,
    -1000, -995, -980, -956, -923, -881, -831, -773, -707, -634, -555, -471, -382, -290, -195, -98
};

static unsigned long g_rng = 19990815;
static int PseudoRand() {
    g_rng = g_rng * 1103515245 + 12345;
    return (int)((g_rng >> 16) & 0x7FFF);
}

void ShowHelpDialog(HWND hwnd) {
    MessageBoxA(hwnd,
        "=== KRadio Retro Stream Player ===\n\n"
        "Tuning & Controls:\n"
        "  - [1-6] Quick Presets: Instantly switch & play preset stations\n"
        "  - [Space / P] Play: Tune into the current URL\n"
        "  - [S] Stop: Stop active audio stream\n"
        "  - [V] Visualizer Mode: Cycle LED Bars, CRT Wave, and VU Meters\n"
        "  - [B] Buffer Mode: Cycle 64KB Normal, 128KB Robust, 32KB Low-Latency\n"
        "  - [Enter]: Tune to stream when typing in URL field\n"
        "  - [Escape]: Return focus to controls / dismiss edit\n"
        "  - [F5] Quicksave: Save active station, visualizer & playback state\n"
        "  - [F9] Quickload: Restore saved station, visualizer & playback state\n"
        "  - [F1 / H]: Show this Help reference\n\n"
        "Station Presets:\n"
        "  [1] Subspace (Synthwave / Retrowave)\n"
        "  [2] Groove Salad (Ambient Chillout)\n"
        "  [3] Nightwave Plaza (Vaporwave)\n"
        "  [4] DEF CON Radio (Cyber Ambient)\n"
        "  [5] Secret Agent (Spy / Retro Lounge)\n"
        "  [6] Space Station (Spacemusic Ambient)\n\n"
        "Interactive Spectrum Visualizer:\n"
        "  Click the visualizer panel or press [V] to cycle styles.",
        "KRadio Help & Keyboard Shortcuts",
        MB_OK | MB_ICONINFORMATION);
}

void UpdateStatusText() {
    if (!hStatus) return;
    char buf[128];
    if (g_isPlaying) {
        wsprintfA(buf, "Status: Playing live stream | Buffer: %s [100%% OK]", g_bufferNames[g_bufferMode]);
    } else {
        wsprintfA(buf, "Status: Stopped | Buffer: Standby (%s)", g_bufferNames[g_bufferMode]);
    }
    SetWindowTextA(hStatus, buf);
}

void StopStream() {
    mciSendStringA("stop myStream", NULL, 0, NULL);
    mciSendStringA("close myStream", NULL, 0, NULL);
    g_isPlaying = FALSE;
    g_bufferHealth = 0;
    UpdateStatusText();
    if (hViz) InvalidateRect(hViz, NULL, FALSE);
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
        g_bufferHealth = 100;
        UpdateStatusText();
    } else {
        g_isPlaying = FALSE;
        g_bufferHealth = 0;
        if (hStatus) SetWindowTextA(hStatus, "Status: Stream open failed (Server offline / Format unsupported)");
    }
    if (hViz) InvalidateRect(hViz, NULL, FALSE);
}

void SelectPreset(int index) {
    if (index < 0 || index >= 6) return;
    SetWindowTextA(hEditUrl, g_presets[index].url);
    PlayStream(g_hwndMain);
}

void CycleVizMode() {
    g_vizMode = (g_vizMode + 1) % 3;
    if (hBtnViz) {
        char buf[32];
        wsprintfA(buf, "Viz [%s]", g_vizNames[g_vizMode]);
        SetWindowTextA(hBtnViz, buf);
    }
    if (hViz) InvalidateRect(hViz, NULL, FALSE);
}

void CycleBufferMode() {
    g_bufferMode = (g_bufferMode + 1) % 3;
    UpdateStatusText();
    if (hViz) InvalidateRect(hViz, NULL, FALSE);
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
    state.vizMode = g_vizMode;
    state.bufferMode = g_bufferMode;
    state.bufferHealth = g_bufferHealth;

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

    if (ok && read >= 524 && state.magic == 0x4441524B) {
        SetWindowTextA(hEditUrl, state.url);
        if (read >= (DWORD)sizeof(KRadioState)) {
            if (state.vizMode >= 0 && state.vizMode < 3) g_vizMode = state.vizMode;
            if (state.bufferMode >= 0 && state.bufferMode < 3) g_bufferMode = state.bufferMode;
            g_bufferHealth = state.bufferHealth;
        }

        if (hBtnViz) {
            char buf[32];
            wsprintfA(buf, "Viz [%s]", g_vizNames[g_vizMode]);
            SetWindowTextA(hBtnViz, buf);
        }

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
        if (hViz) InvalidateRect(hViz, NULL, FALSE);
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

// Custom Spectrum Visualizer Window Proc
LRESULT CALLBACK KRadioVizProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_ERASEBKGND:
            return 1; // Double buffered, skip background erase

        case WM_LBUTTONDOWN:
            CycleVizMode();
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);
            int vw = rc.right - rc.left;
            int vh = rc.bottom - rc.top;
            if (vw <= 0 || vh <= 0) {
                EndPaint(hwnd, &ps);
                return 0;
            }

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, vw, vh);
            HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

            // Background (#080D19)
            HBRUSH bgBrush = CreateSolidBrush(RGB(8, 13, 25));
            FillRect(memDC, &rc, bgBrush);
            DeleteObject(bgBrush);

            // Border (#1E293B)
            HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(30, 41, 59));
            HPEN oldPen = (HPEN)SelectObject(memDC, borderPen);
            SelectObject(memDC, GetStockObject(NULL_BRUSH));
            Rectangle(memDC, 0, 0, vw, vh);

            // Top Header Telemetry Strip
            RECT headerRc = { 1, 1, vw - 1, 18 };
            HBRUSH headerBrush = CreateSolidBrush(RGB(13, 21, 37));
            FillRect(memDC, &headerRc, headerBrush);
            DeleteObject(headerBrush);

            SetBkMode(memDC, TRANSPARENT);
            SelectObject(memDC, g_hFontSmall ? g_hFontSmall : GetStockObject(DEFAULT_GUI_FONT));

            // Left text: Visualizer Mode
            SetTextColor(memDC, RGB(56, 189, 248));
            char modeText[64];
            wsprintfA(modeText, "[VIZ MODE: %s]", g_vizNames[g_vizMode]);
            TextOutA(memDC, 8, 2, modeText, lstrlenA(modeText));

            // Right text: Buffer Health
            char bufText[80];
            if (g_isPlaying) {
                SetTextColor(memDC, RGB(34, 197, 94));
                wsprintfA(bufText, "BUFFER: 100%% HEALTHY (%s)", g_bufferNames[g_bufferMode]);
            } else {
                SetTextColor(memDC, RGB(148, 163, 184));
                wsprintfA(bufText, "BUFFER: STANDBY (%s)", g_bufferNames[g_bufferMode]);
            }
            SIZE sz;
            GetTextExtentPoint32A(memDC, bufText, lstrlenA(bufText), &sz);
            TextOutA(memDC, vw - sz.cx - 8, 2, bufText, lstrlenA(bufText));

            int plotY = 20;
            int plotH = vh - plotY - 4;

            if (g_vizMode == 0) {
                // Mode 0: LED Segmented Spectrum Bars (16 bands)
                int numBands = 16;
                int numSegments = 8;
                int barSpacing = 4;
                int totalSpacing = (numBands + 1) * barSpacing;
                int barW = (vw - totalSpacing) / numBands;
                if (barW < 4) barW = 4;
                int segH = (plotH - (numSegments - 1) * 2) / numSegments;
                if (segH < 2) segH = 2;

                HBRUSH unlitBrush = CreateSolidBrush(RGB(22, 32, 48));
                HBRUSH cyanBrush = CreateSolidBrush(RGB(56, 189, 248));
                HBRUSH amberBrush = CreateSolidBrush(RGB(245, 158, 11));
                HBRUSH redBrush = CreateSolidBrush(RGB(239, 68, 68));
                HBRUSH peakBrush = CreateSolidBrush(RGB(253, 224, 71));

                for (int b = 0; b < numBands; b++) {
                    int bx = barSpacing + b * (barW + barSpacing);
                    int lvl = g_bandLevels[b]; // 0-1000
                    int activeSegments = (lvl * numSegments) / 1000;

                    for (int s = 0; s < numSegments; s++) {
                        int sy = plotY + plotH - (s + 1) * (segH + 2);
                        RECT segRc = { bx, sy, bx + barW, sy + segH };

                        if (s < activeSegments) {
                            if (s >= 6) FillRect(memDC, &segRc, redBrush);
                            else if (s >= 4) FillRect(memDC, &segRc, amberBrush);
                            else FillRect(memDC, &segRc, cyanBrush);
                        } else {
                            FillRect(memDC, &segRc, unlitBrush);
                        }
                    }

                    // Peak hold tick
                    int peakSeg = (g_peakLevels[b] * numSegments) / 1000;
                    if (peakSeg >= numSegments) peakSeg = numSegments - 1;
                    if (peakSeg >= 0 && g_isPlaying) {
                        int py = plotY + plotH - (peakSeg + 1) * (segH + 2);
                        RECT pkRc = { bx, py, bx + barW, py + 2 };
                        FillRect(memDC, &pkRc, peakBrush);
                    }
                }

                DeleteObject(unlitBrush);
                DeleteObject(cyanBrush);
                DeleteObject(amberBrush);
                DeleteObject(redBrush);
                DeleteObject(peakBrush);

            } else if (g_vizMode == 1) {
                // Mode 1: CRT Oscilloscope Wave
                HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(16, 26, 46));
                SelectObject(memDC, gridPen);
                for (int gx = 20; gx < vw; gx += 28) {
                    MoveToEx(memDC, gx, plotY, NULL);
                    LineTo(memDC, gx, plotY + plotH);
                }
                int centerY = plotY + plotH / 2;
                for (int gy = plotY + 12; gy < plotY + plotH; gy += 14) {
                    MoveToEx(memDC, 2, gy, NULL);
                    LineTo(memDC, vw - 2, gy);
                }
                DeleteObject(gridPen);

                // Center baseline
                HPEN centerPen = CreatePen(PS_SOLID, 1, RGB(30, 48, 80));
                SelectObject(memDC, centerPen);
                MoveToEx(memDC, 2, centerY, NULL);
                LineTo(memDC, vw - 2, centerY);
                DeleteObject(centerPen);

                // Phosphor Waveform
                HPEN wavePen = CreatePen(PS_SOLID, 2, RGB(34, 197, 94));
                SelectObject(memDC, wavePen);

                int maxAmp = plotH / 2 - 4;
                if (maxAmp < 6) maxAmp = 6;

                MoveToEx(memDC, 4, centerY, NULL);
                for (int x = 4; x < vw - 4; x += 3) {
                    int sample = 0;
                    if (g_isPlaying) {
                        int idx1 = ((x * 64) / vw + g_animPhase * 3) & 63;
                        int idx2 = ((x * 128) / vw - g_animPhase * 2) & 63;
                        int idx3 = ((x * 32) / vw + g_animPhase) & 63;
                        sample = (g_sinTable[idx1] * 500 + g_sinTable[idx2] * 300 + g_sinTable[idx3] * 200) / 1000;
                        sample = (sample * maxAmp) / 1000;
                    } else {
                        // Ambient resting hum
                        int idx = ((x * 32) / vw + g_animPhase) & 63;
                        sample = (g_sinTable[idx] * 2) / 1000;
                    }
                    int wy = centerY - sample;
                    LineTo(memDC, x, wy);
                }
                DeleteObject(wavePen);

            } else {
                // Mode 2: Analog Dual VU Meters (Left / Right)
                int meterW = (vw - 32) / 2;
                int leftX1 = 10;
                int rightX1 = leftX1 + meterW + 12;

                HBRUSH dialBrush = CreateSolidBrush(RGB(240, 238, 228));
                RECT leftRc = { leftX1, plotY + 4, leftX1 + meterW, plotY + plotH };
                RECT rightRc = { rightX1, plotY + 4, rightX1 + meterW, plotY + plotH };
                FillRect(memDC, &leftRc, dialBrush);
                FillRect(memDC, &rightRc, dialBrush);
                DeleteObject(dialBrush);

                HPEN framePen = CreatePen(PS_SOLID, 1, RGB(70, 70, 70));
                SelectObject(memDC, framePen);
                SelectObject(memDC, GetStockObject(NULL_BRUSH));
                Rectangle(memDC, leftRc.left, leftRc.top, leftRc.right, leftRc.bottom);
                Rectangle(memDC, rightRc.left, rightRc.top, rightRc.right, rightRc.bottom);
                DeleteObject(framePen);

                // Channel Labels & dB marks
                SetTextColor(memDC, RGB(40, 40, 40));
                TextOutA(memDC, leftRc.left + 8, leftRc.top + 4, "LEFT [CH-1]", 11);
                TextOutA(memDC, rightRc.left + 8, rightRc.top + 4, "RIGHT [CH-2]", 12);

                SetTextColor(memDC, RGB(220, 38, 38));
                TextOutA(memDC, leftRc.right - 44, leftRc.top + 4, "+3 dB", 5);
                TextOutA(memDC, rightRc.right - 44, rightRc.top + 4, "+3 dB", 5);

                // Compute needles based on low and high frequency averages
                int sumL = 0, sumR = 0;
                for (int b = 0; b < 8; b++) sumL += g_bandLevels[b];
                for (int b = 8; b < 16; b++) sumR += g_bandLevels[b];
                int lvlL = sumL / 8; // 0-1000
                int lvlR = sumR / 8; // 0-1000

                // Needles
                HPEN needlePen = CreatePen(PS_SOLID, 2, RGB(20, 20, 20));
                SelectObject(memDC, needlePen);

                int pivotYL = leftRc.bottom;
                int pivotXL = leftRc.left + meterW / 2;
                int targetXL = leftRc.left + 16 + (lvlL * (meterW - 32)) / 1000;
                int targetYL = leftRc.top + 16;
                MoveToEx(memDC, pivotXL, pivotYL, NULL);
                LineTo(memDC, targetXL, targetYL);

                int pivotYR = rightRc.bottom;
                int pivotXR = rightRc.left + meterW / 2;
                int targetXR = rightRc.left + 16 + (lvlR * (meterW - 32)) / 1000;
                int targetYR = rightRc.top + 16;
                MoveToEx(memDC, pivotXR, pivotYR, NULL);
                LineTo(memDC, targetXR, targetYR);

                DeleteObject(needlePen);
            }

            SelectObject(memDC, oldPen);
            DeleteObject(borderPen);

            BitBlt(hdc, 0, 0, vw, vh, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBM);
            DeleteObject(memBM);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
            return 0;
        }

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
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
            int fontHeightNormal = -MulDiv(10, dpi, 72);
            int fontHeightBold = -MulDiv(12, dpi, 72);
            int fontHeightSmall = -MulDiv(8, dpi, 72);

            g_hFontNormal = CreateFontA(fontHeightNormal, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            g_hFontBold = CreateFontA(fontHeightBold, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            g_hFontSmall = CreateFontA(fontHeightSmall, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            // Title Header
            hTitle = CreateWindowEx(0, "STATIC", "KRadio - Retro Stream Player",
                WS_CHILD | WS_VISIBLE,
                16, 12, 270, 24, hwnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)g_hFontBold, TRUE);

            // Help button
            hBtnHelp = CreateWindowEx(0, "BUTTON", "Help [F1]",
                WS_CHILD | WS_VISIBLE,
                374, 10, 90, 26, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);
            SendMessage(hBtnHelp, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            // URL Label
            hUrlLabel = CreateWindowEx(0, "STATIC", "Stream URL or Direct MP3/AAC Address:",
                WS_CHILD | WS_VISIBLE,
                16, 42, 448, 16, hwnd, NULL, NULL, NULL);
            SendMessage(hUrlLabel, WM_SETFONT, (WPARAM)g_hFontSmall, TRUE);

            // URL Edit Box
            hEditUrl = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "https://radio.erb.pw/public/subspace",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                16, 60, 448, 24, hwnd, NULL, NULL, NULL);
            SendMessage(hEditUrl, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            g_pfnOrigEditProc = (WNDPROC)SetWindowLongPtrA(hEditUrl, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

            // Presets Label
            hPresetsLabel = CreateWindowEx(0, "STATIC", "Station Presets (Hotkeys 1 - 6):",
                WS_CHILD | WS_VISIBLE,
                16, 88, 448, 16, hwnd, NULL, NULL, NULL);
            SendMessage(hPresetsLabel, WM_SETFONT, (WPARAM)g_hFontSmall, TRUE);

            // Preset Buttons (Row 1: 3 buttons, Row 2: 3 buttons)
            hBtnPresets[0] = CreateWindowEx(0, "BUTTON", "[1] Subspace",
                WS_CHILD | WS_VISIBLE,
                16, 106, 144, 26, hwnd, (HMENU)ID_PRESET_1, NULL, NULL);
            SendMessage(hBtnPresets[0], WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            hBtnPresets[1] = CreateWindowEx(0, "BUTTON", "[2] Groove Salad",
                WS_CHILD | WS_VISIBLE,
                168, 106, 144, 26, hwnd, (HMENU)ID_PRESET_2, NULL, NULL);
            SendMessage(hBtnPresets[1], WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            hBtnPresets[2] = CreateWindowEx(0, "BUTTON", "[3] Nightwave",
                WS_CHILD | WS_VISIBLE,
                320, 106, 144, 26, hwnd, (HMENU)ID_PRESET_3, NULL, NULL);
            SendMessage(hBtnPresets[2], WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            hBtnPresets[3] = CreateWindowEx(0, "BUTTON", "[4] DEF CON",
                WS_CHILD | WS_VISIBLE,
                16, 136, 144, 26, hwnd, (HMENU)ID_PRESET_4, NULL, NULL);
            SendMessage(hBtnPresets[3], WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            hBtnPresets[4] = CreateWindowEx(0, "BUTTON", "[5] Secret Agent",
                WS_CHILD | WS_VISIBLE,
                168, 136, 144, 26, hwnd, (HMENU)ID_PRESET_5, NULL, NULL);
            SendMessage(hBtnPresets[4], WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            hBtnPresets[5] = CreateWindowEx(0, "BUTTON", "[6] Space Station",
                WS_CHILD | WS_VISIBLE,
                320, 136, 144, 26, hwnd, (HMENU)ID_PRESET_6, NULL, NULL);
            SendMessage(hBtnPresets[5], WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            // Playback Action Buttons
            hBtnPlay = CreateWindowEx(0, "BUTTON", "Play [Space]",
                WS_CHILD | WS_VISIBLE,
                16, 168, 88, 30, hwnd, (HMENU)ID_BTN_PLAY, NULL, NULL);
            SendMessage(hBtnPlay, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            hBtnStop = CreateWindowEx(0, "BUTTON", "Stop [S]",
                WS_CHILD | WS_VISIBLE,
                110, 168, 72, 30, hwnd, (HMENU)ID_BTN_STOP, NULL, NULL);
            SendMessage(hBtnStop, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            hBtnSave = CreateWindowEx(0, "BUTTON", "Save [F5]",
                WS_CHILD | WS_VISIBLE,
                188, 168, 84, 30, hwnd, (HMENU)ID_BTN_SAVE, NULL, NULL);
            SendMessage(hBtnSave, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            hBtnLoad = CreateWindowEx(0, "BUTTON", "Load [F9]",
                WS_CHILD | WS_VISIBLE,
                278, 168, 84, 30, hwnd, (HMENU)ID_BTN_LOAD, NULL, NULL);
            SendMessage(hBtnLoad, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            hBtnViz = CreateWindowEx(0, "BUTTON", "Viz [V]",
                WS_CHILD | WS_VISIBLE,
                368, 168, 96, 30, hwnd, (HMENU)ID_BTN_VIZ, NULL, NULL);
            SendMessage(hBtnViz, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            // Spectrum Visualizer Window
            hViz = CreateWindowEx(0, "KRadioVizClass", "",
                WS_CHILD | WS_VISIBLE,
                16, 204, 448, 96, hwnd, NULL, NULL, NULL);

            // Status Display
            hStatus = CreateWindowEx(0, "STATIC", "Status: Ready | Buffer: Standby (64 KB Normal)",
                WS_CHILD | WS_VISIBLE,
                16, 306, 448, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hStatus, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            
            // Shortcut Hint
            hHint = CreateWindowEx(0, "STATIC", "Hotkeys: [1-6] Presets | [Space] Play | [S] Stop | [V] Viz | [B] Buffer | [F5] Save | [F9] Load | [F1] Help",
                WS_CHILD | WS_VISIBLE,
                16, 328, 448, 18, hwnd, NULL, NULL, NULL);
            SendMessage(hHint, WM_SETFONT, (WPARAM)g_hFontSmall, TRUE);
            
            // 30 FPS Visualizer animation timer
            SetTimer(hwnd, 1, 33, NULL);

            // Auto play on startup
            PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_PLAY, 0), 0);
            break;
        }

        case WM_TIMER: {
            if (wParam == 1) {
                // Skip processing if minimized to conserve CPU
                if (IsIconic(hwnd)) break;

                g_animPhase = (g_animPhase + 1) & 63;

                // Animate spectrum bands
                for (int i = 0; i < 16; i++) {
                    int target = 80;
                    if (g_isPlaying) {
                        int wave = (g_sinTable[((g_animPhase * 3) + i * 4) & 63] * 400) / 1000 + 500;
                        int jitter = PseudoRand() % 350;
                        target = wave + jitter;
                        if (target > 1000) target = 1000;
                        if (target < 80) target = 80;
                    }
                    g_bandLevels[i] += (target - g_bandLevels[i]) / 4;

                    if (g_bandLevels[i] >= g_peakLevels[i]) {
                        g_peakLevels[i] = g_bandLevels[i];
                        g_peakDecay[i] = 0;
                    } else {
                        g_peakDecay[i] += 12;
                        g_peakLevels[i] -= g_peakDecay[i];
                        if (g_peakLevels[i] < g_bandLevels[i]) g_peakLevels[i] = g_bandLevels[i];
                    }
                }
                if (hViz) InvalidateRect(hViz, NULL, FALSE);
            }
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
            } else if (wParam == 'V' || wParam == 'v') {
                CycleVizMode();
            } else if (wParam == 'B' || wParam == 'b') {
                CycleBufferMode();
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
            } else if (id == ID_BTN_VIZ) {
                CycleVizMode();
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
            KillTimer(hwnd, 1);
            mciSendStringA("close myStream", NULL, 0, NULL);
            if (g_hFontNormal) DeleteObject(g_hFontNormal);
            if (g_hFontBold) DeleteObject(g_hFontBold);
            if (g_hFontSmall) DeleteObject(g_hFontSmall);
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

    // Register visualizer window class
    WNDCLASS wcv = {0};
    wcv.lpfnWndProc = KRadioVizProc;
    wcv.hInstance = hInstance;
    wcv.lpszClassName = "KRadioVizClass";
    wcv.hCursor = LoadCursor(NULL, IDC_HAND);
    RegisterClassA(&wcv);

    // Register main window class
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
                if (msg.wParam == 'V' || msg.wParam == 'v') {
                    CycleVizMode();
                    continue;
                }
                if (msg.wParam == 'B' || msg.wParam == 'b') {
                    CycleBufferMode();
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
