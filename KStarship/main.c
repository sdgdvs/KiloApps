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

// Game State Definitions
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
    int mapMode;       // 0: Graphical GDI, 1: ASCII Matrix
    int audioMuted;    // 0: Sound ON, 1: Muted
    int inInitModal;   // 1: Commission Modal Active
    int inStationModal;// 1: Outpost Services Active
    int selectedClass; // 0: Corvette, 1: Frigate, 2: Interceptor, 3: Cruiser
} StarshipState;

static StarshipState g_State = {
    "SS ANTIGRAVITY", "CORVETTE", "ALPHA-01",
    100, 100,
    100, 100,
    100, 100,
    75, 75,
    1000, 0, 10, 25,
    3, 3,
    {{0}}, 0,
    0, 0, 1, 0, 0
};

typedef struct {
    char msg[128];
    int type; // 0: sys (blue), 1: warn (amber), 2: alert (red), 3: good (green)
} LogEntry;

static LogEntry g_Logs[16];
static int g_LogCount = 0;

// Starfield background data
typedef struct {
    int x, y;
    int size;
    int alpha;
    int speed;
} StarDot;

static StarDot g_Stars[75];
static unsigned int g_RngSeed = 12345;

static int xrand() {
    g_RngSeed = g_RngSeed * 1103515245 + 12345;
    return (int)((g_RngSeed / 65536) % 32768);
}

void AddLog(const char* msg, int type) {
    if (g_LogCount < 16) {
        int i;
        for (i = 0; msg[i] != '\0' && i < 127; i++) g_Logs[g_LogCount].msg[i] = msg[i];
        g_Logs[g_LogCount].msg[i] = '\0';
        g_Logs[g_LogCount].type = type;
        g_LogCount++;
    } else {
        int i;
        for (i = 0; i < 15; i++) {
            g_Logs[i] = g_Logs[i + 1];
        }
        int k;
        for (k = 0; msg[k] != '\0' && k < 127; k++) g_Logs[15].msg[k] = msg[k];
        g_Logs[15].msg[k] = '\0';
        g_Logs[15].type = type;
    }
}

void PlayGameSound(int type) {
    if (g_State.audioMuted) return;
    switch (type) {
        case 1: Beep(450, 40); break;  // Move
        case 2: Beep(1200, 100); break; // Scan
        case 3: Beep(250, 150); break;  // Mine
        case 4: Beep(1600, 200); break; // Warp
        case 5: Beep(900, 120); break;  // Dock
        case 6: Beep(220, 220); break;  // Alert
        case 7: Beep(850, 100); break;  // Good
    }
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

void InitStars() {
    int i;
    for (i = 0; i < 75; i++) {
        g_Stars[i].x = xrand() % 348;
        g_Stars[i].y = xrand() % 343;
        g_Stars[i].size = (xrand() % 2) + 1;
        g_Stars[i].alpha = (xrand() % 200) + 55;
        g_Stars[i].speed = (xrand() % 3) + 1;
    }
}

// Window Controls & Handles
static HWND hBtnAudioToggle, hBtnModeToggle;
static HWND hBtnScan, hBtnWarp, hBtnMine, hBtnDock, hBtnShield, hBtnRepair;
static HWND hBtnNW, hBtnN, hBtnNE, hBtnW, hBtnCenter, hBtnE, hBtnSW, hBtnS, hBtnSE;

// Modal Controls
static HWND hBtnClass0, hBtnClass1, hBtnClass2, hBtnClass3, hBtnConfirmInit;
static HWND hBtnStRefuel, hBtnStRepair, hBtnStSellOre, hBtnStRecruit, hBtnStClose;

static HBRUSH hBgBrush = NULL;
static HBRUSH hPanelBrush = NULL;
static HFONT hTitleFont = NULL;
static HFONT hMonoFont = NULL;
static HFONT hBoldFont = NULL;

void UpdateControlVisibility(HWND hwnd) {
    BOOL inMain = (!g_State.inInitModal && !g_State.inStationModal);

    // Main Deck Action & Nav Buttons
    ShowWindow(hBtnScan, inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnWarp, inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnMine, inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnDock, inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnShield, inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnRepair, inMain ? SW_SHOW : SW_HIDE);

    ShowWindow(hBtnNW, inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnN, inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnNE, inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnW, inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnCenter, inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnE, inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnSW, inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnS, inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnSE, inMain ? SW_SHOW : SW_HIDE);

    // Init Modal Buttons
    ShowWindow(hBtnClass0, g_State.inInitModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnClass1, g_State.inInitModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnClass2, g_State.inInitModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnClass3, g_State.inInitModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnConfirmInit, g_State.inInitModal ? SW_SHOW : SW_HIDE);

    // Station Modal Buttons
    ShowWindow(hBtnStRefuel, g_State.inStationModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnStRepair, g_State.inStationModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnStSellOre, g_State.inStationModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnStRecruit, g_State.inStationModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnStClose, g_State.inStationModal ? SW_SHOW : SW_HIDE);

    InvalidateRect(hwnd, NULL, TRUE);
}

void ApplyShipClassStats() {
    if (g_State.selectedClass == 0) { // Corvette
        lstrcpyA(g_State.shipClass, "CORVETTE");
        g_State.maxHull = 100; g_State.hull = 100;
        g_State.maxFuel = 120; g_State.fuel = 120;
        g_State.maxEnergy = 100; g_State.energy = 100;
        g_State.maxShields = 75; g_State.shields = 75;
        g_State.credits = 1000; g_State.crew = 10; g_State.ore = 0; g_State.scraps = 25;
    } else if (g_State.selectedClass == 1) { // Frigate
        lstrcpyA(g_State.shipClass, "FRIGATE");
        g_State.maxHull = 150; g_State.hull = 150;
        g_State.maxFuel = 90; g_State.fuel = 90;
        g_State.maxEnergy = 90; g_State.energy = 90;
        g_State.maxShields = 50; g_State.shields = 50;
        g_State.credits = 800; g_State.crew = 16; g_State.ore = 20; g_State.scraps = 25;
    } else if (g_State.selectedClass == 2) { // Interceptor
        lstrcpyA(g_State.shipClass, "INTERCEPTOR");
        g_State.maxHull = 80; g_State.hull = 80;
        g_State.maxFuel = 100; g_State.fuel = 100;
        g_State.maxEnergy = 130; g_State.energy = 130;
        g_State.maxShields = 100; g_State.shields = 100;
        g_State.credits = 1200; g_State.crew = 8; g_State.ore = 0; g_State.scraps = 25;
    } else if (g_State.selectedClass == 3) { // Cruiser
        lstrcpyA(g_State.shipClass, "CRUISER");
        g_State.maxHull = 110; g_State.hull = 110;
        g_State.maxFuel = 140; g_State.fuel = 140;
        g_State.maxEnergy = 110; g_State.energy = 110;
        g_State.maxShields = 85; g_State.shields = 85;
        g_State.credits = 1500; g_State.crew = 20; g_State.ore = 0; g_State.scraps = 25;
    }
}

void MoveShip(HWND hwnd, int dx, int dy) {
    if (g_State.inInitModal || g_State.inStationModal) return;

    if (g_State.hull <= 0) {
        AddLog("[ALERT] SHIP DESTROYED! Subsystems unresponsive.", 2);
        PlayGameSound(6);
        return;
    }
    if (g_State.fuel < 1) {
        AddLog("[WARN] HYPER FUEL EXHAUSTED! Starship drifting in deep space.", 1);
        PlayGameSound(6);
        return;
    }

    int newX = g_State.shipX + dx;
    int newY = g_State.shipY + dy;

    if (newX < 0 || newX >= 8 || newY < 0 || newY >= 8) {
        AddLog("[WARN] Sector boundary reached. Engage Warp Jump to leave system.", 1);
        return;
    }

    g_State.shipX = newX;
    g_State.shipY = newY;
    g_State.fuel = (g_State.fuel > 0) ? g_State.fuel - 1 : 0;
    g_State.energy = (g_State.energy > 0) ? g_State.energy - 1 : 0;

    g_State.moveCount++;
    if (g_State.moveCount % 6 == 0 && g_State.crew > 0) {
        AddLog("[SYS] Life support consumed 1 unit supplies for crew.", 0);
    }

    PlayGameSound(1);

    int cell = g_State.gridMap[g_State.shipY][g_State.shipX];
    if (cell == 1) { // Star
        int gain = (g_State.maxEnergy - g_State.energy < 25) ? (g_State.maxEnergy - g_State.energy) : 25;
        g_State.energy += gain;
        AddLog("[SOLAR] Harvesting energy from star radiation (+25 Energy).", 3);
        PlayGameSound(7);
    } else if (cell == 2) { // Station
        AddLog("[OUTPOST] Docking perimeter reached. Station services online.", 3);
        PlayGameSound(5);
        g_State.inStationModal = 1;
        UpdateControlVisibility(hwnd);
    } else if (cell == 3) { // Asteroid
        AddLog("[ASTEROID] Entered dense asteroid field. Use Mine Asteroids.", 1);
        if ((xrand() % 100) < 25) {
            int dmg = (xrand() % 8) + 4;
            if (g_State.shields >= dmg) {
                g_State.shields -= dmg;
                AddLog("[WARN] Asteroid fragment impact! Shields absorbed damage.", 1);
            } else {
                int rem = dmg - g_State.shields;
                g_State.shields = 0;
                g_State.hull = (g_State.hull > rem) ? g_State.hull - rem : 0;
                AddLog("[ALERT] Asteroid impact breached shields! Hull damaged!", 2);
                PlayGameSound(6);
            }
        }
    } else if (cell == 4) { // Anomaly
        PlayGameSound(2);
        int ev = xrand() % 3;
        if (ev == 0) {
            g_State.scraps += 10;
            AddLog("[ANOMALY] Salvaged derelict probe! (+10 Tech Scrap)", 3);
        } else if (ev == 1) {
            g_State.energy = g_State.maxEnergy;
            AddLog("[ANOMALY] Cosmic ion surge! Reactor energy fully recharged!", 3);
        } else {
            g_State.shields = (g_State.shields > 15) ? g_State.shields - 15 : 0;
            AddLog("[ANOMALY] Spatial distortion flare! Shield matrix drained (-15).", 1);
        }
    } else if (cell == 5) { // Pirate
        PlayGameSound(6);
        AddLog("[ALERT] PIRATE RAIDER AMBUSH! Red Alert active!", 2);
        int dmg = (xrand() % 14) + 8;
        if (g_State.shields >= dmg) {
            g_State.shields -= dmg;
            AddLog("[WARN] Pirate disruptor fire hit shields.", 1);
        } else {
            int rem = dmg - g_State.shields;
            g_State.shields = 0;
            g_State.hull = (g_State.hull > rem) ? g_State.hull - rem : 0;
            AddLog("[ALERT] Shields collapsed! Hull took combat damage!", 2);
        }
    }

    InvalidateRect(hwnd, NULL, TRUE);
}

void ExecuteScan(HWND hwnd) {
    if (g_State.energy < 10) {
        AddLog("[WARN] Insufficient energy for sensor scan.", 1);
        PlayGameSound(6);
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
        wsprintfA(buf, "[SCAN COMPLETE] Outposts: %d | Asteroids: %d | Anomalies: %d | Hostiles: %d", st, ast, anom, pir);
        AddLog(buf, 0);
        PlayGameSound(2);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void ExecuteWarp(HWND hwnd) {
    if (g_State.fuel < 15) {
        AddLog("[ALERT] Insufficient Hyper Fuel for sector jump!", 2);
        PlayGameSound(6);
    } else {
        g_State.fuel -= 15;
        const char* sectors[] = {"ALPHA-01", "BETA-07", "GAMMA-09", "NEBULA-X", "SOLARIS-IV", "ORION-9", "OMEGA-99"};
        int idx = xrand() % 7;
        int i;
        for (i = 0; sectors[idx][i] != '\0'; i++) g_State.sector[i] = sectors[idx][i];
        g_State.sector[i] = '\0';

        g_State.shipX = xrand() % 8;
        g_State.shipY = xrand() % 8;
        GenerateSectorGrid();

        AddLog("[WARP JUMP] Disengaged hyperdrive. Arrived at sector.", 3);
        PlayGameSound(4);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void ExecuteMine(HWND hwnd) {
    int cell = g_State.gridMap[g_State.shipY][g_State.shipX];
    if (cell != 3) {
        AddLog("[WARN] No mining targets detected in immediate grid space.", 1);
        return;
    }
    if (g_State.energy < 15) {
        AddLog("[WARN] Insufficient reactor energy for mining lasers.", 1);
        PlayGameSound(6);
    } else {
        g_State.energy -= 15;
        int minedOre = (xrand() % 10) + 4;
        int minedScrap = (xrand() % 4) + 1;
        g_State.ore += minedOre;
        g_State.scraps += minedScrap;
        char buf[128];
        wsprintfA(buf, "[MINING LASER] Extracted %d tons ore & %d tech scrap!", minedOre, minedScrap);
        AddLog(buf, 3);
        PlayGameSound(3);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void ExecuteDock(HWND hwnd) {
    int cell = g_State.gridMap[g_State.shipY][g_State.shipX];
    if (cell != 2) {
        AddLog("[WARN] Outpost docking clearance unavailable outside station grid.", 1);
        return;
    }
    g_State.inStationModal = 1;
    UpdateControlVisibility(hwnd);
    PlayGameSound(5);
}

void ExecuteShieldBoost(HWND hwnd) {
    if (g_State.energy < 15) {
        AddLog("[WARN] Insufficient energy to charge shield matrix.", 1);
        PlayGameSound(6);
    } else if (g_State.shields >= g_State.maxShields) {
        AddLog("[SYS] Shield matrix already at maximum capacity.", 0);
    } else {
        g_State.energy -= 15;
        g_State.shields = (g_State.shields + 25 > g_State.maxShields) ? g_State.maxShields : g_State.shields + 25;
        AddLog("[DEFLECTORS] Diverted energy to shields (+25 Shields).", 3);
        PlayGameSound(7);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void ExecuteRepair(HWND hwnd) {
    if (g_State.hull >= g_State.maxHull) {
        AddLog("[SYS] Hull integrity already nominal (100%).", 0);
    } else if (g_State.credits >= 50) {
        g_State.credits -= 50;
        g_State.hull = (g_State.hull + 25 > g_State.maxHull) ? g_State.maxHull : g_State.hull + 25;
        AddLog("[REPAIR] Nanite welding repaired hull (+25 HP). Cost: 50 Cr.", 3);
        PlayGameSound(7);
    } else if (g_State.scraps >= 10) {
        g_State.scraps -= 10;
        g_State.hull = (g_State.hull + 20 > g_State.maxHull) ? g_State.maxHull : g_State.hull + 20;
        AddLog("[REPAIR] Used 10 Tech Scrap to patch hull (+20 HP).", 3);
        PlayGameSound(7);
    } else {
        AddLog("[WARN] Insufficient credits or scrap to conduct hull repairs.", 1);
        PlayGameSound(6);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

// Station Modal Actions
void StationRefuel(HWND hwnd) {
    if (g_State.credits < 40) {
        AddLog("[WARN] Insufficient credits for refueling.", 1);
        PlayGameSound(6);
        return;
    }
    g_State.credits -= 40;
    g_State.fuel = (g_State.fuel + 30 > g_State.maxFuel) ? g_State.maxFuel : g_State.fuel + 30;
    AddLog("[STATION] Refueled hyper tanks (+30 Fuel).", 3);
    PlayGameSound(7);
    InvalidateRect(hwnd, NULL, TRUE);
}

void StationRepair(HWND hwnd) {
    if (g_State.credits < 80) {
        AddLog("[WARN] Insufficient credits for full hull repair.", 1);
        PlayGameSound(6);
        return;
    }
    g_State.credits -= 80;
    g_State.hull = (g_State.hull + 40 > g_State.maxHull) ? g_State.maxHull : g_State.hull + 40;
    AddLog("[STATION] Drydock hull reconditioning (+40 HP).", 3);
    PlayGameSound(7);
    InvalidateRect(hwnd, NULL, TRUE);
}

void StationSellOre(HWND hwnd) {
    if (g_State.ore <= 0) {
        AddLog("[WARN] No ore in cargo bay to sell.", 1);
        return;
    }
    int earned = g_State.ore * 25;
    g_State.credits += earned;
    char buf[128];
    wsprintfA(buf, "[STATION] Sold %d tons of rare ore for +%d Cr!", g_State.ore, earned);
    AddLog(buf, 3);
    g_State.ore = 0;
    PlayGameSound(7);
    InvalidateRect(hwnd, NULL, TRUE);
}

void StationRecruitCrew(HWND hwnd) {
    if (g_State.credits < 100) {
        AddLog("[WARN] Insufficient credits to hire crew officers.", 1);
        PlayGameSound(6);
        return;
    }
    g_State.credits -= 100;
    g_State.crew += 3;
    AddLog("[STATION] Recruited 3 veteran crew officers.", 3);
    PlayGameSound(7);
    InvalidateRect(hwnd, NULL, TRUE);
}

// Drawing Utilities
void DrawBar(HDC hdc, int x, int y, int w, int h, int cur, int max, COLORREF col) {
    RECT bgRect = {x, y, x + w, y + h};
    HBRUSH hBg = CreateSolidBrush(RGB(8, 18, 32));
    FillRect(hdc, &bgRect, hBg);
    DeleteObject(hBg);

    if (max <= 0) max = 1;
    int fillW = (int)(((double)cur / max) * w);
    if (fillW > w) fillW = w;
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
            hTitleFont = CreateFontA(17, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            hMonoFont = CreateFontA(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            hBoldFont = CreateFontA(13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

            InitStars();
            GenerateSectorGrid();

            AddLog("[SYSTEM] Subsystems online. Sector command deck ready.", 0);
            AddLog("Commission vessel to launch sector exploration.", 3);

            // Header Toggles
            hBtnAudioToggle = CreateWindowA("BUTTON", "🔊 AUDIO: ON", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 395, 8, 110, 24, hwnd, (HMENU)100, NULL, NULL);
            hBtnModeToggle  = CreateWindowA("BUTTON", "🌌 GRAPHICAL", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 515, 8, 115, 24, hwnd, (HMENU)101, NULL, NULL);

            // Right Panel Action Buttons
            hBtnScan   = CreateWindowA("BUTTON", "📡 SCAN SECTOR", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 48, 195, 26, hwnd, (HMENU)102, NULL, NULL);
            hBtnWarp   = CreateWindowA("BUTTON", "🌌 HYPER WARP",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 78, 195, 26, hwnd, (HMENU)103, NULL, NULL);
            hBtnMine   = CreateWindowA("BUTTON", "⛏️ MINE ASTEROID", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 108, 195, 26, hwnd, (HMENU)104, NULL, NULL);
            hBtnDock   = CreateWindowA("BUTTON", "⚓ DOCK OUTPOST", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 138, 195, 26, hwnd, (HMENU)105, NULL, NULL);
            hBtnShield = CreateWindowA("BUTTON", "🛡️ BOOST DEFLECT",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 168, 195, 26, hwnd, (HMENU)106, NULL, NULL);
            hBtnRepair = CreateWindowA("BUTTON", "🛠️ REPAIR HULL",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 198, 195, 26, hwnd, (HMENU)107, NULL, NULL);

            // D-Pad Navigation Buttons
            hBtnNW     = CreateWindowA("BUTTON", "NW",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 240, 60, 26, hwnd, (HMENU)201, NULL, NULL);
            hBtnN      = CreateWindowA("BUTTON", "N ▲",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 697, 240, 60, 26, hwnd, (HMENU)202, NULL, NULL);
            hBtnNE     = CreateWindowA("BUTTON", "NE",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 765, 240, 60, 26, hwnd, (HMENU)203, NULL, NULL);

            hBtnW      = CreateWindowA("BUTTON", "◄ W",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 272, 60, 26, hwnd, (HMENU)204, NULL, NULL);
            hBtnCenter = CreateWindowA("BUTTON", "●",    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 697, 272, 60, 26, hwnd, (HMENU)205, NULL, NULL);
            hBtnE      = CreateWindowA("BUTTON", "E ►",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 765, 272, 60, 26, hwnd, (HMENU)206, NULL, NULL);

            hBtnSW     = CreateWindowA("BUTTON", "SW",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 304, 60, 26, hwnd, (HMENU)207, NULL, NULL);
            hBtnS      = CreateWindowA("BUTTON", "S ▼",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 697, 304, 60, 26, hwnd, (HMENU)208, NULL, NULL);
            hBtnSE     = CreateWindowA("BUTTON", "SE",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 765, 304, 60, 26, hwnd, (HMENU)209, NULL, NULL);

            // Init Modal Buttons
            hBtnClass0 = CreateWindowA("BUTTON", "[1] CORVETTE (Explorer)", WS_CHILD | BS_PUSHBUTTON, 230, 150, 380, 28, hwnd, (HMENU)301, NULL, NULL);
            hBtnClass1 = CreateWindowA("BUTTON", "[2] FRIGATE (Mining)",    WS_CHILD | BS_PUSHBUTTON, 230, 184, 380, 28, hwnd, (HMENU)302, NULL, NULL);
            hBtnClass2 = CreateWindowA("BUTTON", "[3] INTERCEPTOR (Tactical)",WS_CHILD | BS_PUSHBUTTON, 230, 218, 380, 28, hwnd, (HMENU)303, NULL, NULL);
            hBtnClass3 = CreateWindowA("BUTTON", "[4] CRUISER (Deep Space)",WS_CHILD | BS_PUSHBUTTON, 230, 252, 380, 28, hwnd, (HMENU)304, NULL, NULL);
            hBtnConfirmInit = CreateWindowA("BUTTON", "🚀 INITIALIZE COMMAND DECK", WS_CHILD | BS_PUSHBUTTON, 230, 300, 380, 34, hwnd, (HMENU)305, NULL, NULL);

            // Station Modal Buttons
            hBtnStRefuel  = CreateWindowA("BUTTON", "⛽ REFUEL HYPER TANK (+30 Fuel - 40 Cr)", WS_CHILD | BS_PUSHBUTTON, 230, 150, 380, 28, hwnd, (HMENU)401, NULL, NULL);
            hBtnStRepair  = CreateWindowA("BUTTON", "🛠️ HULL RECONDITIONING (+40 HP - 80 Cr)", WS_CHILD | BS_PUSHBUTTON, 230, 186, 380, 28, hwnd, (HMENU)402, NULL, NULL);
            hBtnStSellOre = CreateWindowA("BUTTON", "💰 SELL ALL MINED ORE (+25 Cr/Ton)",       WS_CHILD | BS_PUSHBUTTON, 230, 222, 380, 28, hwnd, (HMENU)403, NULL, NULL);
            hBtnStRecruit = CreateWindowA("BUTTON", "👨‍🚀 RECRUIT CREW OFFICERS (+3 - 100 Cr)",   WS_CHILD | BS_PUSHBUTTON, 230, 258, 380, 28, hwnd, (HMENU)404, NULL, NULL);
            hBtnStClose   = CreateWindowA("BUTTON", "✕ CLOSE / UNDOCK STATION",               WS_CHILD | BS_PUSHBUTTON, 230, 300, 380, 30, hwnd, (HMENU)405, NULL, NULL);

            UpdateControlVisibility(hwnd);
            SetTimer(hwnd, 1, 50, NULL);
            break;
        }

        case WM_TIMER: {
            int i;
            for (i = 0; i < 75; i++) {
                g_Stars[i].alpha += g_Stars[i].speed * 5;
                if (g_Stars[i].alpha > 255) {
                    g_Stars[i].alpha = 255;
                    g_Stars[i].speed = -g_Stars[i].speed;
                } else if (g_Stars[i].alpha < 40) {
                    g_Stars[i].alpha = 40;
                    g_Stars[i].speed = -g_Stars[i].speed;
                }
            }
            if (g_State.mapMode == 0 && !g_State.inInitModal && !g_State.inStationModal) {
                RECT rectMap = {260, 48, 615, 393};
                InvalidateRect(hwnd, &rectMap, FALSE);
            }
            break;
        }

        case WM_KEYDOWN: {
            if (g_State.inInitModal || g_State.inStationModal) break;

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
            if (id == 100) { // Audio Toggle
                g_State.audioMuted = !g_State.audioMuted;
                SetWindowTextA(hBtnAudioToggle, g_State.audioMuted ? "🔇 AUDIO: OFF" : "🔊 AUDIO: ON");
            } else if (id == 101) { // Mode Toggle
                g_State.mapMode = (g_State.mapMode == 0) ? 1 : 0;
                SetWindowTextA(hBtnModeToggle, (g_State.mapMode == 0) ? "🌌 GRAPHICAL" : "📟 ASCII");
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (id == 102) ExecuteScan(hwnd);
            else if (id == 103) ExecuteWarp(hwnd);
            else if (id == 104) ExecuteMine(hwnd);
            else if (id == 105) ExecuteDock(hwnd);
            else if (id == 106) ExecuteShieldBoost(hwnd);
            else if (id == 107) ExecuteRepair(hwnd);
            else if (id >= 201 && id <= 209) {
                int dx = 0, dy = 0;
                if (id == 201) { dx = -1; dy = -1; }
                else if (id == 202) { dx = 0; dy = -1; }
                else if (id == 203) { dx = 1; dy = -1; }
                else if (id == 204) { dx = -1; dy = 0; }
                else if (id == 205) { dx = 0; dy = 0; }
                else if (id == 206) { dx = 1; dy = 0; }
                else if (id == 207) { dx = -1; dy = 1; }
                else if (id == 208) { dx = 0; dy = 1; }
                else if (id == 209) { dx = 1; dy = 1; }
                MoveShip(hwnd, dx, dy);
            } else if (id >= 301 && id <= 304) {
                g_State.selectedClass = id - 301;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (id == 305) { // Confirm Ship Init
                ApplyShipClassStats();
                g_State.inInitModal = 0;
                UpdateControlVisibility(hwnd);
                char buf[128];
                wsprintfA(buf, "Commissioned %s [%s]. Command assumed.", g_State.shipClass, g_State.shipName);
                AddLog(buf, 3);
                PlayGameSound(7);
            } else if (id == 401) StationRefuel(hwnd);
            else if (id == 402) StationRepair(hwnd);
            else if (id == 403) StationSellOre(hwnd);
            else if (id == 404) StationRecruitCrew(hwnd);
            else if (id == 405) {
                g_State.inStationModal = 0;
                UpdateControlVisibility(hwnd);
                AddLog("[STATION] Undocked from orbital outpost.", 0);
            }
            SetFocus(hwnd);
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Header Bar
            SelectObject(hdc, hTitleFont);
            SetTextColor(hdc, RGB(0, 240, 255));
            SetBkMode(hdc, TRANSPARENT);
            TextOutA(hdc, 12, 10, "🚀 KSTARSHIP", 14);

            SelectObject(hdc, hMonoFont);
            SetTextColor(hdc, RGB(0, 139, 155));
            TextOutA(hdc, 160, 14, "SECTOR COMMAND V2.0", 19);

            char sectorBuf[64];
            char colChar = (char)('A' + g_State.shipX);
            wsprintfA(sectorBuf, "SECTOR: %s   GRID: %c%d", g_State.sector, colChar, g_State.shipY + 1);
            SetTextColor(hdc, RGB(255, 170, 0));
            TextOutA(hdc, 640, 14, sectorBuf, (int)lstrlenA(sectorBuf));

            // Left Panel: Ship Vitals & Manifest
            RECT leftPanel = {10, 48, 250, 393};
            FillRect(hdc, &leftPanel, hPanelBrush);
            FrameRect(hdc, &leftPanel, (HBRUSH)GetStockObject(WHITE_BRUSH));

            SetTextColor(hdc, RGB(0, 240, 255));
            TextOutA(hdc, 20, 56, "[ SHIP VITALS ]", 15);
            SetTextColor(hdc, RGB(255, 170, 0));
            TextOutA(hdc, 170, 56, g_State.shipClass, (int)lstrlenA(g_State.shipClass));

            SelectObject(hdc, hBoldFont);
            SetTextColor(hdc, RGB(0, 240, 255));
            TextOutA(hdc, 20, 76, g_State.shipName, (int)lstrlenA(g_State.shipName));
            SelectObject(hdc, hMonoFont);

            // Status Bars
            char valBuf[32];
            SetTextColor(hdc, RGB(180, 210, 240));

            wsprintfA(valBuf, "%d/%d", g_State.hull, g_State.maxHull);
            TextOutA(hdc, 20, 96, "HULL INTEGRITY:", 15);
            TextOutA(hdc, 190, 96, valBuf, (int)lstrlenA(valBuf));
            DrawBar(hdc, 20, 112, 210, 10, g_State.hull, g_State.maxHull, RGB(57, 255, 20));

            wsprintfA(valBuf, "%d/%d", g_State.fuel, g_State.maxFuel);
            TextOutA(hdc, 20, 128, "HYPER FUEL:", 11);
            TextOutA(hdc, 190, 128, valBuf, (int)lstrlenA(valBuf));
            DrawBar(hdc, 20, 144, 210, 10, g_State.fuel, g_State.maxFuel, RGB(255, 170, 0));

            wsprintfA(valBuf, "%d/%d", g_State.energy, g_State.maxEnergy);
            TextOutA(hdc, 20, 160, "REACTOR ENERGY:", 15);
            TextOutA(hdc, 190, 160, valBuf, (int)lstrlenA(valBuf));
            DrawBar(hdc, 20, 176, 210, 10, g_State.energy, g_State.maxEnergy, RGB(0, 240, 255));

            wsprintfA(valBuf, "%d/%d", g_State.shields, g_State.maxShields);
            TextOutA(hdc, 20, 192, "SHIELD MATRIX:", 14);
            TextOutA(hdc, 190, 192, valBuf, (int)lstrlenA(valBuf));
            DrawBar(hdc, 20, 208, 210, 10, g_State.shields, g_State.maxShields, RGB(59, 130, 246));

            // Manifest & Resources Grid
            SetTextColor(hdc, RGB(0, 240, 255));
            TextOutA(hdc, 20, 232, "[ MANIFEST & CARGO ]", 20);

            RECT resCard1 = {20, 252, 120, 312};
            RECT resCard2 = {130, 252, 230, 312};
            RECT resCard3 = {20, 320, 120, 380};
            RECT resCard4 = {130, 320, 230, 380};

            HBRUSH hResBg = CreateSolidBrush(RGB(15, 30, 55));
            FillRect(hdc, &resCard1, hResBg);
            FillRect(hdc, &resCard2, hResBg);
            FillRect(hdc, &resCard3, hResBg);
            FillRect(hdc, &resCard4, hResBg);
            DeleteObject(hResBg);

            SetTextColor(hdc, RGB(0, 240, 255));
            SelectObject(hdc, hBoldFont);

            wsprintfA(valBuf, "%d", g_State.credits);
            TextOutA(hdc, 30, 260, valBuf, (int)lstrlenA(valBuf));
            wsprintfA(valBuf, "%d", g_State.ore);
            TextOutA(hdc, 140, 260, valBuf, (int)lstrlenA(valBuf));
            wsprintfA(valBuf, "%d", g_State.crew);
            TextOutA(hdc, 30, 328, valBuf, (int)lstrlenA(valBuf));
            wsprintfA(valBuf, "%d", g_State.scraps);
            TextOutA(hdc, 140, 328, valBuf, (int)lstrlenA(valBuf));

            SelectObject(hdc, hMonoFont);
            SetTextColor(hdc, RGB(123, 155, 185));
            TextOutA(hdc, 30, 285, "CREDITS", 7);
            TextOutA(hdc, 140, 285, "ORE (TONS)", 10);
            TextOutA(hdc, 30, 353, "CREW", 4);
            TextOutA(hdc, 140, 353, "SCRAP", 5);

            // Center Viewport: Star Map Grid
            RECT centerPanel = {260, 48, 615, 393};
            HBRUSH hMapBg = CreateSolidBrush(RGB(3, 6, 13));
            FillRect(hdc, &centerPanel, hMapBg);
            DeleteObject(hMapBg);
            FrameRect(hdc, &centerPanel, (HBRUSH)GetStockObject(WHITE_BRUSH));

            // HUD Info
            SetTextColor(hdc, RGB(0, 240, 255));
            TextOutA(hdc, 270, 54, "SYSTEM: SOLARIS PRIME", 21);

            const char* objectNames[] = {"DEEP SPACE", "SOLAR STAR", "ORBITAL OUTPOST", "ASTEROID FIELD", "QUANTUM ANOMALY", "HOSTILE PIRATE"};
            int curCellType = g_State.gridMap[g_State.shipY][g_State.shipX];
            char objBuf[64];
            wsprintfA(objBuf, "OBJECT: %s", objectNames[curCellType]);
            SetTextColor(hdc, RGB(255, 170, 0));
            TextOutA(hdc, 450, 54, objBuf, (int)lstrlenA(objBuf));

            if (g_State.mapMode == 0) { // Graphical GDI Render
                // Background Stars
                int s;
                for (s = 0; s < 75; s++) {
                    int sx = 265 + g_Stars[s].x;
                    int sy = 72 + g_Stars[s].y;
                    if (sx >= 262 && sx <= 610 && sy >= 72 && sy <= 388) {
                        COLORREF starCol = RGB(g_Stars[s].alpha, g_Stars[s].alpha, (g_Stars[s].alpha + 40 > 255) ? 255 : g_Stars[s].alpha + 40);
                        SetPixel(hdc, sx, sy, starCol);
                        if (g_Stars[s].size > 1) {
                            SetPixel(hdc, sx + 1, sy, starCol);
                            SetPixel(hdc, sx, sy + 1, starCol);
                        }
                    }
                }

                // Grid Lines
                HPEN hGridPen = CreatePen(PS_SOLID, 1, RGB(15, 45, 75));
                HPEN hOldPen = (HPEN)SelectObject(hdc, hGridPen);
                int gx, gy;
                int startX = 285, startY = 85;
                int cellW = 38, cellH = 36;

                for (gy = 0; gy <= 8; gy++) {
                    MoveToEx(hdc, startX, startY + gy * cellH, NULL);
                    LineTo(hdc, startX + 8 * cellW, startY + gy * cellH);
                }
                for (gx = 0; gx <= 8; gx++) {
                    MoveToEx(hdc, startX + gx * cellW, startY, NULL);
                    LineTo(hdc, startX + gx * cellW, startY + 8 * cellH);
                }
                SelectObject(hdc, hOldPen);
                DeleteObject(hGridPen);

                // Column and Row Headers
                SetTextColor(hdc, RGB(0, 139, 155));
                for (gx = 0; gx < 8; gx++) {
                    char ch[2] = {(char)('A' + gx), '\0'};
                    TextOutA(hdc, startX + gx * cellW + cellW / 2 - 4, startY - 14, ch, 1);
                }
                for (gy = 0; gy < 8; gy++) {
                    char rh[2] = {(char)('1' + gy), '\0'};
                    TextOutA(hdc, startX - 14, startY + gy * cellH + cellH / 2 - 6, rh, 1);
                }

                // Render Objects & Ship in Grid
                for (gy = 0; gy < 8; gy++) {
                    for (gx = 0; gx < 8; gx++) {
                        int cx = startX + gx * cellW + cellW / 2;
                        int cy = startY + gy * cellH + cellH / 2;

                        if (gx == g_State.shipX && gy == g_State.shipY) {
                            // Cyan Vector Triangle Starship
                            POINT pts[4];
                            pts[0].x = cx;     pts[0].y = cy - 10;
                            pts[1].x = cx + 8; pts[1].y = cy + 8;
                            pts[2].x = cx;     pts[2].y = cy + 4;
                            pts[3].x = cx - 8; pts[3].y = cy + 8;

                            HBRUSH hShipB = CreateSolidBrush(RGB(0, 240, 255));
                            HPEN hShipP = CreatePen(PS_SOLID, 1, RGB(200, 255, 255));
                            HPEN hOldP = (HPEN)SelectObject(hdc, hShipP);
                            HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hShipB);

                            Polygon(hdc, pts, 4);

                            SelectObject(hdc, hOldP);
                            SelectObject(hdc, hOldB);
                            DeleteObject(hShipP);
                            DeleteObject(hShipB);
                        } else {
                            int type = g_State.gridMap[gy][gx];
                            if (type == 1) { // Solar Star
                                HBRUSH hSunB = CreateSolidBrush(RGB(255, 238, 85));
                                HPEN hSunP = CreatePen(PS_SOLID, 1, RGB(255, 200, 0));
                                HPEN hOldP = (HPEN)SelectObject(hdc, hSunP);
                                HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hSunB);
                                Ellipse(hdc, cx - 7, cy - 7, cx + 7, cy + 7);
                                SelectObject(hdc, hOldP);
                                SelectObject(hdc, hOldB);
                                DeleteObject(hSunP);
                                DeleteObject(hSunB);
                            } else if (type == 2) { // Station
                                HPEN hStP = CreatePen(PS_SOLID, 2, RGB(59, 130, 246));
                                HPEN hOldP = (HPEN)SelectObject(hdc, hStP);
                                HBRUSH hOldB = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
                                Ellipse(hdc, cx - 8, cy - 8, cx + 8, cy + 8);
                                MoveToEx(hdc, cx - 12, cy, NULL); LineTo(hdc, cx + 12, cy);
                                MoveToEx(hdc, cx, cy - 12, NULL); LineTo(hdc, cx, cy + 12);
                                SelectObject(hdc, hOldP);
                                SelectObject(hdc, hOldB);
                                DeleteObject(hStP);
                            } else if (type == 3) { // Asteroid Field
                                HBRUSH hAstB = CreateSolidBrush(RGB(255, 170, 0));
                                HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hAstB);
                                SelectObject(hdc, GetStockObject(NULL_PEN));
                                Ellipse(hdc, cx - 8, cy - 6, cx - 2, cy);
                                Ellipse(hdc, cx + 2, cy - 4, cx + 8, cy + 2);
                                Ellipse(hdc, cx - 4, cy + 2, cx + 2, cy + 8);
                                SelectObject(hdc, hOldB);
                                DeleteObject(hAstB);
                            } else if (type == 4) { // Anomaly (Diamond)
                                POINT dPts[4];
                                dPts[0].x = cx;     dPts[0].y = cy - 9;
                                dPts[1].x = cx + 8; dPts[1].y = cy;
                                dPts[2].x = cx;     dPts[2].y = cy + 9;
                                dPts[3].x = cx - 8; dPts[3].y = cy;

                                HBRUSH hAnomB = CreateSolidBrush(RGB(176, 38, 255));
                                HPEN hAnomP = CreatePen(PS_SOLID, 1, RGB(220, 150, 255));
                                HPEN hOldP = (HPEN)SelectObject(hdc, hAnomP);
                                HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hAnomB);
                                Polygon(hdc, dPts, 4);
                                SelectObject(hdc, hOldP);
                                SelectObject(hdc, hOldB);
                                DeleteObject(hAnomP);
                                DeleteObject(hAnomB);
                            } else if (type == 5) { // Pirate Raider
                                POINT pPts[3];
                                pPts[0].x = cx;     pPts[0].y = cy + 8;
                                pPts[1].x = cx + 7; pPts[1].y = cy - 7;
                                pPts[2].x = cx - 7; pPts[2].y = cy - 7;

                                HBRUSH hPirB = CreateSolidBrush(RGB(255, 51, 102));
                                HPEN hPirP = CreatePen(PS_SOLID, 1, RGB(255, 120, 150));
                                HPEN hOldP = (HPEN)SelectObject(hdc, hPirP);
                                HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hPirB);
                                Polygon(hdc, pPts, 3);
                                SelectObject(hdc, hOldP);
                                SelectObject(hdc, hOldB);
                                DeleteObject(hPirP);
                                DeleteObject(hPirB);
                            }
                        }
                    }
                }
            } else { // ASCII Matrix Render
                SetTextColor(hdc, RGB(0, 139, 155));
                int gx, gy;
                for (gx = 0; gx < 8; gx++) {
                    char cHeader[2] = {(char)('A' + gx), '\0'};
                    TextOutA(hdc, 305 + (gx * 35), 75, cHeader, 1);
                }

                for (gy = 0; gy < 8; gy++) {
                    char rHeader[2] = {(char)('1' + gy), '\0'};
                    SetTextColor(hdc, RGB(0, 139, 155));
                    TextOutA(hdc, 280, 95 + (gy * 35), rHeader, 1);

                    for (gx = 0; gx < 8; gx++) {
                        int cellX = 295 + (gx * 35);
                        int cellY = 90 + (gy * 35);

                        RECT cRect = {cellX, cellY, cellX + 32, cellY + 32};
                        HPEN hCellPen = CreatePen(PS_SOLID, 1, RGB(15, 35, 60));
                        HPEN hOldP = (HPEN)SelectObject(hdc, hCellPen);
                        HBRUSH hOldB = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
                        Rectangle(hdc, cRect.left, cRect.top, cRect.right, cRect.bottom);
                        SelectObject(hdc, hOldP);
                        SelectObject(hdc, hOldB);
                        DeleteObject(hCellPen);

                        if (gx == g_State.shipX && gy == g_State.shipY) {
                            SetTextColor(hdc, RGB(0, 240, 255));
                            TextOutA(hdc, cellX + 11, cellY + 8, "^", 1);
                        } else {
                            int type = g_State.gridMap[gy][gx];
                            if (type == 1) {
                                SetTextColor(hdc, RGB(255, 238, 85));
                                TextOutA(hdc, cellX + 11, cellY + 8, "*", 1);
                            } else if (type == 2) {
                                SetTextColor(hdc, RGB(59, 130, 246));
                                TextOutA(hdc, cellX + 11, cellY + 8, "O", 1);
                            } else if (type == 3) {
                                SetTextColor(hdc, RGB(255, 170, 0));
                                TextOutA(hdc, cellX + 11, cellY + 8, "#", 1);
                            } else if (type == 4) {
                                SetTextColor(hdc, RGB(176, 38, 255));
                                TextOutA(hdc, cellX + 11, cellY + 8, "?", 1);
                            } else if (type == 5) {
                                SetTextColor(hdc, RGB(255, 51, 102));
                                TextOutA(hdc, cellX + 11, cellY + 8, "X", 1);
                            } else {
                                SetTextColor(hdc, RGB(25, 50, 85));
                                TextOutA(hdc, cellX + 11, cellY + 8, ".", 1);
                            }
                        }
                    }
                }
            }

            // Right Panel Frame (Controls Header)
            RECT rightPanel = {620, 48, 830, 393};
            FillRect(hdc, &rightPanel, hPanelBrush);
            FrameRect(hdc, &rightPanel, (HBRUSH)GetStockObject(WHITE_BRUSH));

            SetTextColor(hdc, RGB(0, 240, 255));
            TextOutA(hdc, 630, 222, "[ NAVIGATION GRID ]", 19);

            // Bottom Panel: Log Console
            RECT logPanel = {10, 402, 830, 545};
            FillRect(hdc, &logPanel, hPanelBrush);
            FrameRect(hdc, &logPanel, (HBRUSH)GetStockObject(WHITE_BRUSH));

            SetTextColor(hdc, RGB(0, 240, 255));
            TextOutA(hdc, 20, 408, "--- CAPTAIN'S TACTICAL FEED & TERMINAL LOG ---", 46);

            int visibleLogs = (g_LogCount > 7) ? 7 : g_LogCount;
            int startIdx = (g_LogCount > 7) ? (g_LogCount - 7) : 0;
            int i;
            for (i = 0; i < visibleLogs; i++) {
                int logIdx = startIdx + i;
                if (g_Logs[logIdx].type == 0) SetTextColor(hdc, RGB(95, 135, 184));     // Sys / blue
                else if (g_Logs[logIdx].type == 1) SetTextColor(hdc, RGB(255, 170, 0)); // Warn / amber
                else if (g_Logs[logIdx].type == 2) SetTextColor(hdc, RGB(255, 51, 102));// Alert / red
                else if (g_Logs[logIdx].type == 3) SetTextColor(hdc, RGB(57, 255, 20)); // Good / green

                TextOutA(hdc, 20, 426 + (i * 16), g_Logs[logIdx].msg, (int)lstrlenA(g_Logs[logIdx].msg));
            }

            // Render Active Modals (Commissioning / Outpost)
            if (g_State.inInitModal) {
                RECT modalOverlay = {10, 48, 830, 393};
                HBRUSH hDimBrush = CreateSolidBrush(RGB(4, 10, 22));
                FillRect(hdc, &modalOverlay, hDimBrush);
                DeleteObject(hDimBrush);

                RECT modalCard = {210, 70, 630, 370};
                FillRect(hdc, &modalCard, hPanelBrush);
                FrameRect(hdc, &modalCard, (HBRUSH)GetStockObject(WHITE_BRUSH));

                SelectObject(hdc, hTitleFont);
                SetTextColor(hdc, RGB(0, 240, 255));
                TextOutA(hdc, 230, 85, "🚀 COMMISSION STARSHIP", 22);

                SelectObject(hdc, hMonoFont);
                SetTextColor(hdc, RGB(160, 190, 220));
                TextOutA(hdc, 230, 115, "Captain, select vessel blueprint to launch frontier sector exploration:", 71);

                // Highlight selected class
                int selY = 150 + (g_State.selectedClass * 34);
                RECT selRect = {228, selY - 2, 612, selY + 28};
                HPEN hSelPen = CreatePen(PS_SOLID, 2, RGB(0, 240, 255));
                HPEN hOldP = (HPEN)SelectObject(hdc, hSelPen);
                HBRUSH hOldB = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
                Rectangle(hdc, selRect.left, selRect.top, selRect.right, selRect.bottom);
                SelectObject(hdc, hOldP);
                SelectObject(hdc, hOldB);
                DeleteObject(hSelPen);
            } else if (g_State.inStationModal) {
                RECT modalOverlay = {10, 48, 830, 393};
                HBRUSH hDimBrush = CreateSolidBrush(RGB(4, 10, 22));
                FillRect(hdc, &modalOverlay, hDimBrush);
                DeleteObject(hDimBrush);

                RECT modalCard = {210, 70, 630, 370};
                FillRect(hdc, &modalCard, hPanelBrush);
                FrameRect(hdc, &modalCard, (HBRUSH)GetStockObject(WHITE_BRUSH));

                SelectObject(hdc, hTitleFont);
                SetTextColor(hdc, RGB(0, 240, 255));
                TextOutA(hdc, 230, 85, "⚓ ORBITAL OUTPOST SERVICES", 26);

                SelectObject(hdc, hMonoFont);
                SetTextColor(hdc, RGB(160, 190, 220));
                TextOutA(hdc, 230, 115, "Welcome Captain. Station refueling docks & outpost supplies active:", 68);
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
            KillTimer(hwnd, 1);
            if (hBgBrush) DeleteObject(hBgBrush);
            if (hPanelBrush) DeleteObject(hPanelBrush);
            if (hTitleFont) DeleteObject(hTitleFont);
            if (hMonoFont) DeleteObject(hMonoFont);
            if (hBoldFont) DeleteObject(hBoldFont);
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
                              CW_USEDEFAULT, CW_USEDEFAULT, 856, 595,
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
