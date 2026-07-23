#include <windows.h>

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

// Game State Definition
typedef struct {
    char shipName[32];
    char sector[16];
    int hull, maxHull;
    int fuel, maxFuel;
    int energy, maxEnergy;
    int shields, maxShields;
    int credits;
    int ore;
    int crew;
    int scraps;
} StarshipState;

static StarshipState g_State = {
    "SS ANTIGRAVITY", "ALPHA-01",
    100, 100,
    80, 100,
    100, 100,
    75, 75,
    1200, 25, 12, 40
};

static char g_LogLines[5][128] = {
    "[SYSTEM] Subsystems initialized. Command deck online.",
    "[LOG] Starship commissioned. Awaiting captain's commands.",
    "", "", ""
};
static int g_LogCount = 2;

void AddLog(const char* msg) {
    if (g_LogCount < 5) {
        int i;
        for (i = 0; msg[i] != '\0' && i < 127; i++) {
            g_LogLines[g_LogCount][i] = msg[i];
        }
        g_LogLines[g_LogCount][i] = '\0';
        g_LogCount++;
    } else {
        int i;
        for (i = 0; i < 4; i++) {
            int j;
            for (j = 0; g_LogLines[i+1][j] != '\0'; j++) {
                g_LogLines[i][j] = g_LogLines[i+1][j];
            }
            g_LogLines[i][j] = '\0';
        }
        int k;
        for (k = 0; msg[k] != '\0' && k < 127; k++) {
            g_LogLines[4][k] = msg[k];
        }
        g_LogLines[4][k] = '\0';
    }
}

// Simple Pseudo-RNG
static unsigned int g_RngSeed = 98765;
static int xrand() {
    g_RngSeed = g_RngSeed * 1103515245 + 12345;
    return (int)((g_RngSeed / 65536) % 32768);
}

// Controls & Handles
static HWND hBtnScan, hBtnWarp, hBtnMine, hBtnDock, hBtnRepair;
static HBRUSH hBgBrush = NULL;
static HBRUSH hPanelBrush = NULL;
static HFONT hTitleFont = NULL;
static HFONT hMonoFont = NULL;

void ExecuteScan(HWND hwnd) {
    if (g_State.energy < 10) {
        AddLog("[WARN] Insufficient energy for sector scan.");
        Beep(300, 150);
    } else {
        g_State.energy -= 10;
        const char* events[] = {
            "[SCAN] Dense asteroid field detected with titanium ore.",
            "[SCAN] Derelict freighter signal located 200km port side.",
            "[SCAN] Planetary outpost emitting active trade beacon.",
            "[SCAN] Cosmic ion flare detected. Shields stable."
        };
        AddLog(events[xrand() % 4]);
        Beep(1200, 100);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void ExecuteWarp(HWND hwnd) {
    if (g_State.fuel < 15) {
        AddLog("[ALERT] Hyper fuel exhausted! Warp jump unavailable.");
        Beep(200, 300);
    } else {
        g_State.fuel -= 15;
        const char* sectors[] = {"BETA-02", "GAMMA-09", "NEBULA-X", "SOLARIS-IV", "ORION-7"};
        int idx = xrand() % 5;
        int i;
        for (i = 0; sectors[idx][i] != '\0'; i++) {
            g_State.sector[i] = sectors[idx][i];
        }
        g_State.sector[i] = '\0';
        AddLog("[WARP] Jump successful! Entered new star sector.");
        Beep(1800, 150);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void ExecuteMine(HWND hwnd) {
    if (g_State.energy < 15) {
        AddLog("[WARN] Insufficient reactor energy for mining lasers.");
        Beep(300, 150);
    } else {
        g_State.energy -= 15;
        int mined = (xrand() % 8) + 3;
        g_State.ore += mined;
        AddLog("[MINING] Extracted rare mineral ore from asteroids.");
        Beep(900, 100);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void ExecuteDock(HWND hwnd) {
    g_State.fuel = (g_State.fuel + 20 > g_State.maxFuel) ? g_State.maxFuel : g_State.fuel + 20;
    g_State.energy = g_State.maxEnergy;
    AddLog("[STATION] Docked at outpost. Recharged energy & refueled.");
    Beep(1000, 120);
    InvalidateRect(hwnd, NULL, TRUE);
}

void ExecuteRepair(HWND hwnd) {
    if (g_State.credits < 50) {
        AddLog("[WARN] Insufficient credits for hull repair.");
        Beep(300, 150);
    } else if (g_State.hull >= g_State.maxHull) {
        AddLog("[SYS] Hull integrity already at 100%.");
    } else {
        g_State.credits -= 50;
        g_State.hull = (g_State.hull + 25 > g_State.maxHull) ? g_State.maxHull : g_State.hull + 25;
        AddLog("[ENGINEERING] Hull plating repaired (+25 HP).");
        Beep(1400, 120);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void DrawBar(HDC hdc, int x, int y, int w, int h, int cur, int max, COLORREF col) {
    RECT bgRect = {x, y, x + w, y + h};
    HBRUSH hBg = CreateSolidBrush(RGB(10, 20, 35));
    FillRect(hdc, &bgRect, hBg);
    DeleteObject(hBg);

    int fillW = (int)(((double)cur / max) * w);
    if (fillW > 0) {
        RECT fillRect = {x + 1, y + 1, x + fillW - 1, y + h - 1};
        HBRUSH hFill = CreateSolidBrush(col);
        FillRect(hdc, &fillRect, hFill);
        DeleteObject(hFill);
    }

    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(30, 60, 95));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, x, y, x + w, y + h);
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPen);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hBgBrush = CreateSolidBrush(RGB(5, 8, 17));
            hPanelBrush = CreateSolidBrush(RGB(10, 20, 38));
            hTitleFont = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            hMonoFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

            hBtnScan   = CreateWindowA("BUTTON", "📡 SCAN SECTOR", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 580, 50, 180, 32, hwnd, (HMENU)101, NULL, NULL);
            hBtnWarp   = CreateWindowA("BUTTON", "🌌 WARP JUMP",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 580, 90, 180, 32, hwnd, (HMENU)102, NULL, NULL);
            hBtnMine   = CreateWindowA("BUTTON", "⛏️ MINE ORE",    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 580, 130, 180, 32, hwnd, (HMENU)103, NULL, NULL);
            hBtnDock   = CreateWindowA("BUTTON", "⚓ DOCK STATION", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 580, 170, 180, 32, hwnd, (HMENU)104, NULL, NULL);
            hBtnRepair = CreateWindowA("BUTTON", "🛠️ REPAIR HULL",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 580, 210, 180, 32, hwnd, (HMENU)105, NULL, NULL);
            break;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case 101: ExecuteScan(hwnd); break;
                case 102: ExecuteWarp(hwnd); break;
                case 103: ExecuteMine(hwnd); break;
                case 104: ExecuteDock(hwnd); break;
                case 105: ExecuteRepair(hwnd); break;
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Title
            SelectObject(hdc, hTitleFont);
            SetTextColor(hdc, RGB(0, 240, 255));
            SetBkMode(hdc, TRANSPARENT);
            TextOutA(hdc, 20, 12, "🚀 KSTARSHIP COMMAND DECK", 26);

            char sectorBuf[48];
            wsprintfA(sectorBuf, "SECTOR: %s", g_State.sector);
            SetTextColor(hdc, RGB(255, 170, 0));
            TextOutA(hdc, 580, 12, sectorBuf, (int)lstrlenA(sectorBuf));

            SelectObject(hdc, hMonoFont);

            // Left Panel: Ship Vitals
            RECT leftPanel = {20, 48, 250, 340};
            FillRect(hdc, &leftPanel, hPanelBrush);
            FrameRect(hdc, &leftPanel, (HBRUSH)GetStockObject(WHITE_BRUSH));

            SetTextColor(hdc, RGB(0, 240, 255));
            TextOutA(hdc, 30, 56, "[ SHIP VITALS ]", 15);
            SetTextColor(hdc, RGB(200, 225, 255));
            TextOutA(hdc, 30, 78, g_State.shipName, (int)lstrlenA(g_State.shipName));

            // Status Bars
            SetTextColor(hdc, RGB(160, 190, 220));
            TextOutA(hdc, 30, 105, "HULL INTEGRITY:", 15);
            DrawBar(hdc, 30, 122, 200, 12, g_State.hull, g_State.maxHull, RGB(57, 255, 20));

            TextOutA(hdc, 30, 142, "HYPER FUEL:", 11);
            DrawBar(hdc, 30, 159, 200, 12, g_State.fuel, g_State.maxFuel, RGB(255, 170, 0));

            TextOutA(hdc, 30, 179, "REACTOR ENERGY:", 15);
            DrawBar(hdc, 30, 196, 200, 12, g_State.energy, g_State.maxEnergy, RGB(0, 240, 255));

            TextOutA(hdc, 30, 216, "SHIELD MATRIX:", 14);
            DrawBar(hdc, 30, 233, 200, 12, g_State.shields, g_State.maxShields, RGB(59, 130, 246));

            // Manifest
            SetTextColor(hdc, RGB(0, 240, 255));
            TextOutA(hdc, 30, 258, "[ MANIFEST ]", 12);
            SetTextColor(hdc, RGB(200, 225, 255));

            char statBuf[64];
            wsprintfA(statBuf, "Credits : %d", g_State.credits);
            TextOutA(hdc, 30, 276, statBuf, (int)lstrlenA(statBuf));
            wsprintfA(statBuf, "Rare Ore: %d tons", g_State.ore);
            TextOutA(hdc, 30, 292, statBuf, (int)lstrlenA(statBuf));
            wsprintfA(statBuf, "Crew    : %d officers", g_State.crew);
            TextOutA(hdc, 30, 308, statBuf, (int)lstrlenA(statBuf));

            // Center Viewport: Star Map Grid
            RECT centerPanel = {265, 48, 565, 340};
            HBRUSH hMapBg = CreateSolidBrush(RGB(3, 6, 13));
            FillRect(hdc, &centerPanel, hMapBg);
            DeleteObject(hMapBg);
            FrameRect(hdc, &centerPanel, (HBRUSH)GetStockObject(WHITE_BRUSH));

            // Grid lines
            HPEN hGridPen = CreatePen(PS_DOT, 1, RGB(15, 35, 60));
            HPEN hOldP = (HPEN)SelectObject(hdc, hGridPen);
            int gx;
            for (gx = 285; gx < 565; gx += 35) {
                MoveToEx(hdc, gx, 48, NULL);
                LineTo(hdc, gx, 340);
            }
            int gy;
            for (gy = 68; gy < 340; gy += 35) {
                MoveToEx(hdc, 265, gy, NULL);
                LineTo(hdc, 565, gy);
            }
            SelectObject(hdc, hOldP);
            DeleteObject(hGridPen);

            // Draw Vessel Icon (Cyan Star/Cross)
            SetTextColor(hdc, RGB(0, 240, 255));
            TextOutA(hdc, 405, 185, "▲", 2);
            TextOutA(hdc, 385, 200, "[VESSEL]", 8);

            // Stars
            SetPixel(hdc, 290, 80, RGB(255, 255, 255));
            SetPixel(hdc, 340, 150, RGB(0, 240, 255));
            SetPixel(hdc, 480, 100, RGB(255, 200, 100));
            SetPixel(hdc, 520, 280, RGB(255, 255, 255));
            SetPixel(hdc, 310, 300, RGB(100, 200, 255));

            // Bottom Panel: Log Console
            RECT logPanel = {20, 355, 765, 475};
            FillRect(hdc, &logPanel, hPanelBrush);
            FrameRect(hdc, &logPanel, (HBRUSH)GetStockObject(WHITE_BRUSH));

            SetTextColor(hdc, RGB(0, 240, 255));
            TextOutA(hdc, 30, 362, "--- CAPTAIN'S TERMINAL LOG ---", 30);

            SetTextColor(hdc, RGB(180, 210, 240));
            int i;
            for (i = 0; i < g_LogCount; i++) {
                TextOutA(hdc, 30, 382 + (i * 17), g_LogLines[i], (int)lstrlenA(g_LogLines[i]));
            }

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, hBgBrush);
            return 1;
        }

        case WM_DESTROY:
            if (hBgBrush) DeleteObject(hBgBrush);
            if (hPanelBrush) DeleteObject(hPanelBrush);
            if (hTitleFont) DeleteObject(hTitleFont);
            if (hMonoFont) DeleteObject(hMonoFont);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = NULL;
    wc.lpszClassName = "KStarshipClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowA("KStarshipClass", "KStarship - Sci-Fi Space Exploration",
                              WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 520,
                              NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}

void MainEntry() {
    int ret = WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWDEFAULT);
    ExitProcess(ret);
}
