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
    char shipClass[16];
    char sector[16];
    int hull, maxHull;
    int fuel, maxFuel;
    int energy, maxEnergy;
    int shields, maxShields;
    int credits;
    int ore;
    int crew;
    int scraps;
    int shipX, shipY;
    int gridMap[8][8]; // 0: empty, 1: star, 2: station, 3: asteroid, 4: anomaly, 5: pirate
    int moveCount;
} StarshipState;

static StarshipState g_State = {
    "SS ANTIGRAVITY", "CORVETTE", "ALPHA-01",
    100, 100,
    100, 100,
    100, 100,
    75, 75,
    1000, 0, 10, 25,
    3, 3,
    {{0}}, 0
};

static char g_LogLines[5][128] = {
    "[SYSTEM] Subsystems online. Sector command deck ready.",
    "[LOG] Commissioned CORVETTE [SS ANTIGRAVITY].",
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

void GenerateSectorGrid() {
    int x, y;
    for (y = 0; y < 8; y++) {
        for (x = 0; x < 8; x++) {
            if (x == g_State.shipX && y == g_State.shipY) {
                g_State.gridMap[y][x] = 0;
                continue;
            }
            int r = xrand() % 100;
            if (r < 6) g_State.gridMap[y][x] = 2;       // Station
            else if (r < 16) g_State.gridMap[y][x] = 1; // Star
            else if (r < 32) g_State.gridMap[y][x] = 3; // Asteroid
            else if (r < 42) g_State.gridMap[y][x] = 4; // Anomaly
            else if (r < 50) g_State.gridMap[y][x] = 5; // Pirate
            else g_State.gridMap[y][x] = 0;            // Empty
        }
    }
}

// Controls & Handles
static HWND hBtnScan, hBtnWarp, hBtnMine, hBtnDock, hBtnRepair, hBtnShield;
static HWND hBtnN, hBtnS, hBtnE, hBtnW, hBtnNW, hBtnNE, hBtnSW, hBtnSE;
static HBRUSH hBgBrush = NULL;
static HBRUSH hPanelBrush = NULL;
static HFONT hTitleFont = NULL;
static HFONT hMonoFont = NULL;

void MoveShip(HWND hwnd, int dx, int dy) {
    if (g_State.hull <= 0) {
        AddLog("[ALERT] SHIP DESTROYED! Cannot navigate.");
        Beep(200, 300);
        return;
    }
    if (g_State.fuel < 1) {
        AddLog("[WARN] HYPER FUEL EXHAUSTED! Starship drifting.");
        Beep(250, 200);
        return;
    }

    int newX = g_State.shipX + dx;
    int newY = g_State.shipY + dy;

    if (newX < 0 || newX >= 8 || newY < 0 || newY >= 8) {
        AddLog("[SYS] Sector boundary reached. Use WARP JUMP.");
        return;
    }

    g_State.shipX = newX;
    g_State.shipY = newY;
    g_State.fuel = (g_State.fuel > 0) ? g_State.fuel - 1 : 0;
    g_State.energy = (g_State.energy > 0) ? g_State.energy - 1 : 0;

    Beep(600, 30);

    int cell = g_State.gridMap[g_State.shipY][g_State.shipX];
    if (cell == 1) { // Star
        int gain = (g_State.maxEnergy - g_State.energy < 25) ? (g_State.maxEnergy - g_State.energy) : 25;
        g_State.energy += gain;
        AddLog("[SOLAR] Harvesting energy from star radiation.");
        Beep(800, 80);
    } else if (cell == 2) { // Station
        AddLog("[OUTPOST] Docking perimeter reached. Outpost online.");
        Beep(1000, 100);
    } else if (cell == 3) { // Asteroid
        AddLog("[ASTEROID] Entered dense asteroid belt.");
        if ((xrand() % 100) < 25) {
            int dmg = (xrand() % 8) + 4;
            if (g_State.shields >= dmg) {
                g_State.shields -= dmg;
                AddLog("[WARN] Asteroid impact absorbed by shields.");
            } else {
                int rem = dmg - g_State.shields;
                g_State.shields = 0;
                g_State.hull = (g_State.hull > rem) ? g_State.hull - rem : 0;
                AddLog("[ALERT] Asteroid impact breached hull!");
                Beep(300, 150);
            }
        }
    } else if (cell == 4) { // Anomaly
        int ev = xrand() % 3;
        if (ev == 0) {
            g_State.scraps += 10;
            AddLog("[ANOMALY] Salvaged probe scrap (+10 Scrap).");
        } else if (ev == 1) {
            g_State.energy = g_State.maxEnergy;
            AddLog("[ANOMALY] Energy surge recharged reactor!");
        } else {
            g_State.shields = (g_State.shields > 15) ? g_State.shields - 15 : 0;
            AddLog("[ANOMALY] Spatial distortion drained shields.");
        }
        Beep(1200, 100);
    } else if (cell == 5) { // Pirate
        AddLog("[ALERT] PIRATE AMBUSH! Hostile raider opening fire.");
        Beep(250, 250);
        int dmg = (xrand() % 14) + 8;
        if (g_State.shields >= dmg) {
            g_State.shields -= dmg;
            AddLog("[WARN] Pirate energy bolt absorbed by shields.");
        } else {
            int rem = dmg - g_State.shields;
            g_State.shields = 0;
            g_State.hull = (g_State.hull > rem) ? g_State.hull - rem : 0;
            AddLog("[ALERT] Deflectors down! Hull took combat damage.");
        }
    }

    InvalidateRect(hwnd, NULL, TRUE);
}

void ExecuteScan(HWND hwnd) {
    if (g_State.energy < 10) {
        AddLog("[WARN] Insufficient energy for sector scan.");
        Beep(300, 150);
    } else {
        g_State.energy -= 10;
        int st = 0, ast = 0, anom = 0, pir = 0, x, y;
        for (y = 0; y < 8; y++) {
            for (x = 0; x < 8; x++) {
                int c = g_State.gridMap[y][x];
                if (c == 2) st++;
                else if (c == 3) ast++;
                else if (c == 4) anom++;
                else if (c == 5) pir++;
            }
        }
        char buf[128];
        wsprintfA(buf, "[SCAN] Outposts:%d Asteroids:%d Anom:%d Hostiles:%d", st, ast, anom, pir);
        AddLog(buf);
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
        const char* sectors[] = {"BETA-02", "GAMMA-09", "NEBULA-X", "SOLARIS-IV", "ORION-7", "OMEGA-99"};
        int idx = xrand() % 6;
        int i;
        for (i = 0; sectors[idx][i] != '\0'; i++) {
            g_State.sector[i] = sectors[idx][i];
        }
        g_State.sector[i] = '\0';

        g_State.shipX = xrand() % 8;
        g_State.shipY = xrand() % 8;
        GenerateSectorGrid();

        AddLog("[WARP JUMP] Disengaged warp! Entered new sector.");
        Beep(1800, 200);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void ExecuteMine(HWND hwnd) {
    int cell = g_State.gridMap[g_State.shipY][g_State.shipX];
    if (cell != 3) {
        AddLog("[WARN] No asteroid targets in immediate cell.");
        return;
    }
    if (g_State.energy < 15) {
        AddLog("[WARN] Insufficient reactor energy for mining lasers.");
        Beep(300, 150);
    } else {
        g_State.energy -= 15;
        int mined = (xrand() % 8) + 4;
        g_State.ore += mined;
        g_State.scraps += 2;
        AddLog("[MINING] Extracted mineral ore and tech scraps.");
        Beep(900, 100);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void ExecuteDock(HWND hwnd) {
    int cell = g_State.gridMap[g_State.shipY][g_State.shipX];
    if (cell != 2) {
        AddLog("[WARN] Outpost docking unavailable outside station cell.");
        return;
    }
    g_State.fuel = (g_State.fuel + 30 > g_State.maxFuel) ? g_State.maxFuel : g_State.fuel + 30;
    g_State.energy = g_State.maxEnergy;
    if (g_State.ore > 0) {
        int earned = g_State.ore * 25;
        g_State.credits += earned;
        g_State.ore = 0;
        AddLog("[STATION] Docked! Sold ore cargo for credits.");
    } else {
        AddLog("[STATION] Docked! Recharged energy & refueled.");
    }
    Beep(1000, 120);
    InvalidateRect(hwnd, NULL, TRUE);
}

void ExecuteShieldBoost(HWND hwnd) {
    if (g_State.energy < 15) {
        AddLog("[WARN] Insufficient energy to charge shields.");
        Beep(300, 150);
    } else if (g_State.shields >= g_State.maxShields) {
        AddLog("[SYS] Shield matrix already at 100%.");
    } else {
        g_State.energy -= 15;
        g_State.shields = (g_State.shields + 25 > g_State.maxShields) ? g_State.maxShields : g_State.shields + 25;
        AddLog("[DEFLECTORS] Boosted shield matrix (+25 Shield).");
        Beep(1100, 100);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void ExecuteRepair(HWND hwnd) {
    if (g_State.hull >= g_State.maxHull) {
        AddLog("[SYS] Hull integrity already at 100%.");
    } else if (g_State.credits >= 50) {
        g_State.credits -= 50;
        g_State.hull = (g_State.hull + 25 > g_State.maxHull) ? g_State.maxHull : g_State.hull + 25;
        AddLog("[ENGINEERING] Hull plating repaired (+25 HP).");
        Beep(1400, 120);
    } else if (g_State.scraps >= 10) {
        g_State.scraps -= 10;
        g_State.hull = (g_State.hull + 20 > g_State.maxHull) ? g_State.maxHull : g_State.hull + 20;
        AddLog("[ENGINEERING] Used 10 Scrap to patch hull (+20 HP).");
        Beep(1200, 120);
    } else {
        AddLog("[WARN] Insufficient credits or scrap for repairs.");
        Beep(300, 150);
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
            hTitleFont = CreateFontA(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            hMonoFont = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

            GenerateSectorGrid();

            // Right Panel Action Buttons
            hBtnScan   = CreateWindowA("BUTTON", "📡 SCAN SECTOR", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 585, 48, 180, 28, hwnd, (HMENU)101, NULL, NULL);
            hBtnWarp   = CreateWindowA("BUTTON", "🌌 WARP JUMP",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 585, 80, 180, 28, hwnd, (HMENU)102, NULL, NULL);
            hBtnMine   = CreateWindowA("BUTTON", "⛏️ MINE ORE",    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 585, 112, 180, 28, hwnd, (HMENU)103, NULL, NULL);
            hBtnDock   = CreateWindowA("BUTTON", "⚓ DOCK STATION", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 585, 144, 180, 28, hwnd, (HMENU)104, NULL, NULL);
            hBtnShield = CreateWindowA("BUTTON", "🛡️ BOOST DEFLECT",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 585, 176, 180, 28, hwnd, (HMENU)105, NULL, NULL);
            hBtnRepair = CreateWindowA("BUTTON", "🛠️ REPAIR HULL",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 585, 208, 180, 28, hwnd, (HMENU)106, NULL, NULL);

            // D-Pad Navigation Buttons
            hBtnNW = CreateWindowA("BUTTON", "NW", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 585, 245, 55, 25, hwnd, (HMENU)201, NULL, NULL);
            hBtnN  = CreateWindowA("BUTTON", "N ▲", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 645, 245, 55, 25, hwnd, (HMENU)202, NULL, NULL);
            hBtnNE = CreateWindowA("BUTTON", "NE", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 705, 245, 55, 25, hwnd, (HMENU)203, NULL, NULL);

            hBtnW  = CreateWindowA("BUTTON", "◄ W", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 585, 275, 55, 25, hwnd, (HMENU)204, NULL, NULL);
            CreateWindowA("BUTTON", "●", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 645, 275, 55, 25, hwnd, (HMENU)205, NULL, NULL);
            hBtnE  = CreateWindowA("BUTTON", "E ►", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 705, 275, 55, 25, hwnd, (HMENU)206, NULL, NULL);

            hBtnSW = CreateWindowA("BUTTON", "SW", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 585, 305, 55, 25, hwnd, (HMENU)207, NULL, NULL);
            hBtnS  = CreateWindowA("BUTTON", "S ▼", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 645, 305, 55, 25, hwnd, (HMENU)208, NULL, NULL);
            hBtnSE = CreateWindowA("BUTTON", "SE", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 705, 305, 55, 25, hwnd, (HMENU)209, NULL, NULL);

            break;
        }

        case WM_KEYDOWN: {
            switch (wParam) {
                case VK_UP:    MoveShip(hwnd, 0, -1); break;
                case VK_DOWN:  MoveShip(hwnd, 0, 1);  break;
                case VK_LEFT:  MoveShip(hwnd, -1, 0); break;
                case VK_RIGHT: MoveShip(hwnd, 1, 0);  break;
                case 'W':      MoveShip(hwnd, 0, -1); break;
                case 'S':      MoveShip(hwnd, 0, 1);  break;
                case 'A':      MoveShip(hwnd, -1, 0); break;
                case 'D':      MoveShip(hwnd, 1, 0);  break;
                case VK_SPACE: ExecuteScan(hwnd);     break;
            }
            break;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            switch (id) {
                case 101: ExecuteScan(hwnd); break;
                case 102: ExecuteWarp(hwnd); break;
                case 103: ExecuteMine(hwnd); break;
                case 104: ExecuteDock(hwnd); break;
                case 105: ExecuteShieldBoost(hwnd); break;
                case 106: ExecuteRepair(hwnd); break;

                case 201: MoveShip(hwnd, -1, -1); break;
                case 202: MoveShip(hwnd, 0, -1);  break;
                case 203: MoveShip(hwnd, 1, -1);  break;
                case 204: MoveShip(hwnd, -1, 0);  break;
                case 205: InvalidateRect(hwnd, NULL, TRUE); break;
                case 206: MoveShip(hwnd, 1, 0);   break;
                case 207: MoveShip(hwnd, -1, 1);  break;
                case 208: MoveShip(hwnd, 0, 1);   break;
                case 209: MoveShip(hwnd, 1, 1);   break;
            }
            SetFocus(hwnd);
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Title
            SelectObject(hdc, hTitleFont);
            SetTextColor(hdc, RGB(0, 240, 255));
            SetBkMode(hdc, TRANSPARENT);
            TextOutA(hdc, 20, 12, "🚀 KSTARSHIP SECTOR COMMAND v2.0", 32);

            char sectorBuf[64];
            char colChar = (char)('A' + g_State.shipX);
            wsprintfA(sectorBuf, "SECTOR: %s [%c%d]", g_State.sector, colChar, g_State.shipY + 1);
            SetTextColor(hdc, RGB(255, 170, 0));
            TextOutA(hdc, 560, 12, sectorBuf, (int)lstrlenA(sectorBuf));

            SelectObject(hdc, hMonoFont);

            // Left Panel: Ship Vitals
            RECT leftPanel = {20, 48, 240, 340};
            FillRect(hdc, &leftPanel, hPanelBrush);
            FrameRect(hdc, &leftPanel, (HBRUSH)GetStockObject(WHITE_BRUSH));

            SetTextColor(hdc, RGB(0, 240, 255));
            TextOutA(hdc, 30, 56, "[ SHIP VITALS ]", 15);
            SetTextColor(hdc, RGB(200, 225, 255));
            TextOutA(hdc, 30, 76, g_State.shipName, (int)lstrlenA(g_State.shipName));

            // Status Bars
            SetTextColor(hdc, RGB(160, 190, 220));
            TextOutA(hdc, 30, 98, "HULL INTEGRITY:", 15);
            DrawBar(hdc, 30, 114, 190, 11, g_State.hull, g_State.maxHull, RGB(57, 255, 20));

            TextOutA(hdc, 30, 132, "HYPER FUEL:", 11);
            DrawBar(hdc, 30, 148, 190, 11, g_State.fuel, g_State.maxFuel, RGB(255, 170, 0));

            TextOutA(hdc, 30, 166, "REACTOR ENERGY:", 15);
            DrawBar(hdc, 30, 182, 190, 11, g_State.energy, g_State.maxEnergy, RGB(0, 240, 255));

            TextOutA(hdc, 30, 200, "SHIELD MATRIX:", 14);
            DrawBar(hdc, 30, 216, 190, 11, g_State.shields, g_State.maxShields, RGB(59, 130, 246));

            // Manifest
            SetTextColor(hdc, RGB(0, 240, 255));
            TextOutA(hdc, 30, 238, "[ MANIFEST ]", 12);
            SetTextColor(hdc, RGB(200, 225, 255));

            char statBuf[64];
            wsprintfA(statBuf, "Credits : %d", g_State.credits);
            TextOutA(hdc, 30, 256, statBuf, (int)lstrlenA(statBuf));
            wsprintfA(statBuf, "Rare Ore: %d tons", g_State.ore);
            TextOutA(hdc, 30, 272, statBuf, (int)lstrlenA(statBuf));
            wsprintfA(statBuf, "Crew    : %d officers", g_State.crew);
            TextOutA(hdc, 30, 288, statBuf, (int)lstrlenA(statBuf));
            wsprintfA(statBuf, "Scraps  : %d units", g_State.scraps);
            TextOutA(hdc, 30, 304, statBuf, (int)lstrlenA(statBuf));

            // Center Viewport: ASCII Star Map Grid
            RECT centerPanel = {255, 48, 565, 340};
            HBRUSH hMapBg = CreateSolidBrush(RGB(3, 6, 13));
            FillRect(hdc, &centerPanel, hMapBg);
            DeleteObject(hMapBg);
            FrameRect(hdc, &centerPanel, (HBRUSH)GetStockObject(WHITE_BRUSH));

            // Draw Column Headers A..H
            SetTextColor(hdc, RGB(0, 139, 155));
            int gx;
            for (gx = 0; gx < 8; gx++) {
                char cHeader[2] = {(char)('A' + gx), '\0'};
                TextOutA(hdc, 290 + (gx * 33), 54, cHeader, 1);
            }

            // Draw 8x8 ASCII Grid Cells
            int gy;
            for (gy = 0; gy < 8; gy++) {
                // Row Headers 1..8
                char rHeader[2] = {(char)('1' + gy), '\0'};
                SetTextColor(hdc, RGB(0, 139, 155));
                TextOutA(hdc, 268, 72 + (gy * 32), rHeader, 1);

                for (gx = 0; gx < 8; gx++) {
                    int cellX = 282 + (gx * 33);
                    int cellY = 68 + (gy * 32);

                    RECT cRect = {cellX, cellY, cellX + 31, cellY + 30};
                    HPEN hCellPen = CreatePen(PS_SOLID, 1, RGB(15, 35, 60));
                    HPEN hOldP = (HPEN)SelectObject(hdc, hCellPen);
                    HBRUSH hOldB = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    Rectangle(hdc, cRect.left, cRect.top, cRect.right, cRect.bottom);
                    SelectObject(hdc, hOldP);
                    SelectObject(hdc, hOldB);
                    DeleteObject(hCellPen);

                    if (gx == g_State.shipX && gy == g_State.shipY) {
                        // Player Starship (Cyan ▲)
                        SetTextColor(hdc, RGB(0, 240, 255));
                        TextOutA(hdc, cellX + 11, cellY + 7, "@", 1);
                    } else {
                        int type = g_State.gridMap[gy][gx];
                        if (type == 1) { // Star
                            SetTextColor(hdc, RGB(255, 238, 85));
                            TextOutA(hdc, cellX + 11, cellY + 7, "*", 1);
                        } else if (type == 2) { // Station
                            SetTextColor(hdc, RGB(59, 130, 246));
                            TextOutA(hdc, cellX + 11, cellY + 7, "O", 1);
                        } else if (type == 3) { // Asteroid
                            SetTextColor(hdc, RGB(255, 170, 0));
                            TextOutA(hdc, cellX + 11, cellY + 7, "#", 1);
                        } else if (type == 4) { // Anomaly
                            SetTextColor(hdc, RGB(176, 38, 255));
                            TextOutA(hdc, cellX + 11, cellY + 7, "?", 1);
                        } else if (type == 5) { // Pirate
                            SetTextColor(hdc, RGB(255, 51, 102));
                            TextOutA(hdc, cellX + 11, cellY + 7, "X", 1);
                        } else { // Empty
                            SetTextColor(hdc, RGB(25, 50, 85));
                            TextOutA(hdc, cellX + 11, cellY + 7, ".", 1);
                        }
                    }
                }
            }

            // Bottom Panel: Log Console
            RECT logPanel = {20, 350, 765, 470};
            FillRect(hdc, &logPanel, hPanelBrush);
            FrameRect(hdc, &logPanel, (HBRUSH)GetStockObject(WHITE_BRUSH));

            SetTextColor(hdc, RGB(0, 240, 255));
            TextOutA(hdc, 30, 356, "--- CAPTAIN'S TACTICAL FEED & TERMINAL LOG ---", 46);

            SetTextColor(hdc, RGB(180, 210, 240));
            int i;
            for (i = 0; i < g_LogCount; i++) {
                TextOutA(hdc, 30, 376 + (i * 17), g_LogLines[i], (int)lstrlenA(g_LogLines[i]));
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
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 515,
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
