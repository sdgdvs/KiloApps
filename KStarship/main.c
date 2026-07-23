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

// Game State Definitions (Phase 5 - Biomes & Planet Scanning)
typedef struct {
    int type;      // 0: Terran, 1: Volcanic, 2: Gas Giant, 3: Ancient Ruins
    int scanned;   // 0: no, 1: yes
    int minerals;
    int gas;
    int artifacts;
    char name[32];
} PlanetCellData;

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
    int gas;       // Plasma Gas Resource
    int artifacts; // Ancient Artifacts
    int crew;
    int scraps;
    int shipX, shipY;
    int gridMap[8][8]; // 0: empty, 1: star, 2: station, 3: asteroid, 4: anomaly, 5: pirate, 6: planet
    PlanetCellData planetData[8][8];
    int currentBiome;  // 0: Terran, 1: Volcanic, 2: Nebula, 3: Asteroid Belt
    int moveCount;
    int mapMode;       // 0: Graphical GDI, 1: ASCII Matrix
    int audioMuted;    // 0: Sound ON, 1: Muted
    int inInitModal;   // 1: Commission Modal Active
    int inStationModal;// 1: Outpost Services Active
    int inPlanetModal; // 1: Planet Telemetry Active
    int inEncounterModal; // 1: Random Encounter Active
    int encounterType; // 0: Pirate, 1: Derelict, 2: Anomaly, 3: Trader, 4: Distress
    int selectedClass; // 0: Corvette, 1: Frigate, 2: Interceptor, 3: Cruiser
    // Phase 7 Tactical Combat State
    int inCombatModal;
    char enemyName[32];
    int enemyHull, enemyMaxHull;
    int enemyShields, enemyMaxShields;
    int enemyWeaponPower;
    int enemyWeaponsDamaged;
    int enemyEvasion;
    int enemyEnginesDamaged;
    int playerEvading;
    int targetSubsystem; // 0: Hull, 1: Shields, 2: Weapons, 3: Engines
    char combatFeed[4][128];
    int combatFeedCount;
    // Phase 8 Upgrade Bay State
    int inUpgradeModal;
    int upgradeHullLvl;
    int upgradeCannonsLvl;
    int upgradeShieldsLvl;
    int upgradeWarpLvl;
    int upgradeSensorsLvl;
} StarshipState;

static StarshipState g_State = {
    "SS ANTIGRAVITY", "CORVETTE", "ALPHA-01",
    100, 100,
    100, 100,
    100, 100,
    75, 75,
    1000, 0, 0, 0, 10, 25,
    3, 3,
    {{0}}, {{{0}}}, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0,
    0, "CYBERNETIC PIRATE RAIDER",
    90, 90, 50, 50, 16, 0, 15, 0, 0, 0,
    {{0}}, 0,
    0, 0, 0, 0, 0, 0
};

static const char* g_BiomeNames[] = {"TERRAN SYSTEM", "VOLCANIC SYSTEM", "NEBULA SYSTEM", "ASTEROID BELT"};

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
        case 1: Beep(450, 40); break;   // Move
        case 2: Beep(1200, 100); break;  // Scan
        case 3: Beep(250, 150); break;   // Mine
        case 4: Beep(1600, 200); break;  // Warp
        case 5: Beep(900, 120); break;   // Dock
        case 6: Beep(220, 220); break;   // Alert
        case 7: Beep(850, 100); break;   // Good
        case 8: Beep(1400, 80); Beep(1800, 120); break; // Planet Scan
    }
}

void GenerateSectorGrid() {
    g_State.currentBiome = xrand() % 4;
    int x, y;
    for (y = 0; y < 8; y++) {
        for (x = 0; x < 8; x++) {
            g_State.planetData[y][x].scanned = 0;
            if (x == g_State.shipX && y == g_State.shipY) {
                g_State.gridMap[y][x] = 0;
                continue;
            }
            int r = xrand() % 100;
            int pStation = 6, pStar = 16, pAsteroid = 32, pAnomaly = 42, pPirate = 50, pPlanet = 65;

            if (g_State.currentBiome == 1) { // Volcanic
                pAsteroid = 38; pPlanet = 70;
            } else if (g_State.currentBiome == 2) { // Nebula
                pAnomaly = 48; pPlanet = 68;
            } else if (g_State.currentBiome == 3) { // Asteroid Belt
                pAsteroid = 42; pPlanet = 72;
            }

            if (r < pStation) g_State.gridMap[y][x] = 2;       // Station
            else if (r < pStar) g_State.gridMap[y][x] = 1;      // Star
            else if (r < pAsteroid) g_State.gridMap[y][x] = 3;  // Asteroid
            else if (r < pAnomaly) g_State.gridMap[y][x] = 4;   // Anomaly
            else if (r < pPirate) g_State.gridMap[y][x] = 5;    // Pirate
            else if (r < pPlanet) {
                g_State.gridMap[y][x] = 6;                      // Planet
                g_State.planetData[y][x].type = xrand() % 4;
                g_State.planetData[y][x].minerals = (xrand() % 20) + 10;
                g_State.planetData[y][x].gas = (xrand() % 18) + 6;
                g_State.planetData[y][x].artifacts = ((xrand() % 100) < 40) ? ((xrand() % 2) + 1) : 0;

                if (g_State.currentBiome == 1) g_State.planetData[y][x].minerals += 15;
                if (g_State.currentBiome == 2) g_State.planetData[y][x].gas += 15;
                if (g_State.currentBiome == 3) g_State.planetData[y][x].artifacts += 1;

                const char* pNames[] = {"Terran Prime", "Volcanic Core", "Gas Giant Alpha", "Ancient Relic Moon"};
                lstrcpyA(g_State.planetData[y][x].name, pNames[g_State.planetData[y][x].type]);
            } else {
                g_State.gridMap[y][x] = 0;                     // Empty
            }
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

void TriggerRandomEncounter(HWND hwnd, int type);
void ResolveEncounterChoice(HWND hwnd, int choiceIdx);
void StartTacticalCombat(HWND hwnd, int presetIdx);
void CombatPlayerFire(HWND hwnd);
void CombatPlayerShield(HWND hwnd);
void CombatPlayerEvade(HWND hwnd);
void CombatPlayerRepair(HWND hwnd);
void CombatPlayerFlee(HWND hwnd);

// Window Controls & Handles
static HWND hBtnAudioToggle, hBtnModeToggle;
static HWND hBtnScan, hBtnScanPlanet, hBtnWarp, hBtnMine, hBtnDock, hBtnShield, hBtnRepair, hBtnEncScan, hBtnCombat, hBtnUpgrade;
static HWND hBtnNW, hBtnN, hBtnNE, hBtnW, hBtnCenter, hBtnE, hBtnSW, hBtnS, hBtnSE;

// Modal Controls
static HWND hBtnClass0, hBtnClass1, hBtnClass2, hBtnClass3, hBtnConfirmInit;
static HWND hBtnStRefuel, hBtnStRepair, hBtnStSellOre, hBtnStSellGas, hBtnStSellArtifacts, hBtnStRecruit, hBtnStUpgrade, hBtnStClose;
static HWND hBtnPlanetClose;
static HWND hBtnEncChoice1, hBtnEncChoice2, hBtnEncChoice3, hBtnEncClose;
static HWND hBtnCombatFire, hBtnCombatShield, hBtnCombatEvade, hBtnCombatRepair, hBtnCombatFlee;
static HWND hBtnTgtHull, hBtnTgtShield, hBtnTgtWeap, hBtnTgtEng;
static HWND hBtnUpgHull, hBtnUpgCannons, hBtnUpgShields, hBtnUpgWarp, hBtnUpgSensors, hBtnUpgClose;

static HBRUSH hBgBrush = NULL;
static HBRUSH hPanelBrush = NULL;
static HFONT hTitleFont = NULL;
static HFONT hMonoFont = NULL;
static HFONT hBoldFont = NULL;

void UpdateControlVisibility(HWND hwnd) {
    BOOL inMain = (!g_State.inInitModal && !g_State.inStationModal && !g_State.inPlanetModal && !g_State.inEncounterModal && !g_State.inCombatModal && !g_State.inUpgradeModal);

    // Main Deck Action & Nav Buttons
    ShowWindow(hBtnScan,       inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnScanPlanet, inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnWarp,       inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnMine,       inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnDock,       inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnShield,     inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnRepair,     inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnEncScan,    inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnCombat,     inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnUpgrade,    inMain ? SW_SHOW : SW_HIDE);

    ShowWindow(hBtnNW,     inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnN,      inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnNE,     inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnW,      inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnCenter, inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnE,      inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnSW,     inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnS,      inMain ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnSE,     inMain ? SW_SHOW : SW_HIDE);

    // Init Modal Buttons
    ShowWindow(hBtnClass0,      g_State.inInitModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnClass1,      g_State.inInitModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnClass2,      g_State.inInitModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnClass3,      g_State.inInitModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnConfirmInit, g_State.inInitModal ? SW_SHOW : SW_HIDE);

    // Station Modal Buttons
    ShowWindow(hBtnStRefuel,        g_State.inStationModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnStRepair,        g_State.inStationModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnStSellOre,       g_State.inStationModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnStSellGas,       g_State.inStationModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnStSellArtifacts, g_State.inStationModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnStRecruit,       g_State.inStationModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnStUpgrade,       g_State.inStationModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnStClose,         g_State.inStationModal ? SW_SHOW : SW_HIDE);

    // Planet Telemetry Modal Buttons
    ShowWindow(hBtnPlanetClose, g_State.inPlanetModal ? SW_SHOW : SW_HIDE);

    // Encounter Modal Buttons
    ShowWindow(hBtnEncChoice1, g_State.inEncounterModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnEncChoice2, g_State.inEncounterModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnEncChoice3, g_State.inEncounterModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnEncClose,   g_State.inEncounterModal ? SW_SHOW : SW_HIDE);

    // Tactical Combat Modal Buttons (Phase 7)
    ShowWindow(hBtnTgtHull,       g_State.inCombatModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnTgtShield,     g_State.inCombatModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnTgtWeap,       g_State.inCombatModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnTgtEng,        g_State.inCombatModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnCombatFire,   g_State.inCombatModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnCombatShield, g_State.inCombatModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnCombatEvade,  g_State.inCombatModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnCombatRepair, g_State.inCombatModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnCombatFlee,   g_State.inCombatModal ? SW_SHOW : SW_HIDE);

    // Upgrade Modal Buttons (Phase 8)
    ShowWindow(hBtnUpgHull,    g_State.inUpgradeModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnUpgCannons, g_State.inUpgradeModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnUpgShields, g_State.inUpgradeModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnUpgWarp,    g_State.inUpgradeModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnUpgSensors, g_State.inUpgradeModal ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnUpgClose,   g_State.inUpgradeModal ? SW_SHOW : SW_HIDE);

    InvalidateRect(hwnd, NULL, TRUE);
}

void ApplyShipClassStats() {
    if (g_State.selectedClass == 0) { // Corvette
        lstrcpyA(g_State.shipClass, "CORVETTE");
        g_State.maxHull = 100; g_State.hull = 100;
        g_State.maxFuel = 120; g_State.fuel = 120;
        g_State.maxEnergy = 100; g_State.energy = 100;
        g_State.maxShields = 75; g_State.shields = 75;
        g_State.credits = 1000; g_State.crew = 10; g_State.ore = 0; g_State.gas = 0; g_State.artifacts = 0; g_State.scraps = 25;
    } else if (g_State.selectedClass == 1) { // Frigate
        lstrcpyA(g_State.shipClass, "FRIGATE");
        g_State.maxHull = 150; g_State.hull = 150;
        g_State.maxFuel = 90; g_State.fuel = 90;
        g_State.maxEnergy = 90; g_State.energy = 90;
        g_State.maxShields = 50; g_State.shields = 50;
        g_State.credits = 800; g_State.crew = 16; g_State.ore = 20; g_State.gas = 5; g_State.artifacts = 0; g_State.scraps = 25;
    } else if (g_State.selectedClass == 2) { // Interceptor
        lstrcpyA(g_State.shipClass, "INTERCEPTOR");
        g_State.maxHull = 80; g_State.hull = 80;
        g_State.maxFuel = 100; g_State.fuel = 100;
        g_State.maxEnergy = 130; g_State.energy = 130;
        g_State.maxShields = 100; g_State.shields = 100;
        g_State.credits = 1200; g_State.crew = 8; g_State.ore = 0; g_State.gas = 0; g_State.artifacts = 0; g_State.scraps = 25;
    } else if (g_State.selectedClass == 3) { // Cruiser
        lstrcpyA(g_State.shipClass, "CRUISER");
        g_State.maxHull = 110; g_State.hull = 110;
        g_State.maxFuel = 140; g_State.fuel = 140;
        g_State.maxEnergy = 110; g_State.energy = 110;
        g_State.maxShields = 85; g_State.shields = 85;
        g_State.credits = 1500; g_State.crew = 20; g_State.ore = 0; g_State.gas = 0; g_State.artifacts = 0; g_State.scraps = 25;
    }
    g_State.upgradeHullLvl = 0;
    g_State.upgradeCannonsLvl = 0;
    g_State.upgradeShieldsLvl = 0;
    g_State.upgradeWarpLvl = 0;
    g_State.upgradeSensorsLvl = 0;
}

// Phase 8: Upgrade Bay Functions
void UpdateUpgradeButtonLabels() {
    char buf[128];
    int c, o;

    c = 200 + g_State.upgradeHullLvl * 150;
    o = 15 + g_State.upgradeHullLvl * 10;
    wsprintfA(buf, "[1] HULL PLATING (Lvl %d) - %d Cr + %d Ore (+25 Max HP)", g_State.upgradeHullLvl, c, o);
    SetWindowTextA(hBtnUpgHull, buf);

    c = 250 + g_State.upgradeCannonsLvl * 180;
    o = 20 + g_State.upgradeCannonsLvl * 12;
    wsprintfA(buf, "[2] PLASMA CANNONS (Lvl %d) - %d Cr + %d Ore (+10 DMG)", g_State.upgradeCannonsLvl, c, o);
    SetWindowTextA(hBtnUpgCannons, buf);

    c = 220 + g_State.upgradeShieldsLvl * 160;
    o = 15 + g_State.upgradeShieldsLvl * 10;
    wsprintfA(buf, "[3] PARTICLE SHIELDS (Lvl %d) - %d Cr + %d Ore (+20 Max Shield)", g_State.upgradeShieldsLvl, c, o);
    SetWindowTextA(hBtnUpgShields, buf);

    c = 300 + g_State.upgradeWarpLvl * 200;
    o = 25 + g_State.upgradeWarpLvl * 15;
    wsprintfA(buf, "[4] WARP DRIVE RANGE (Lvl %d) - %d Cr + %d Ore (+30 Fuel / -2 Jump)", g_State.upgradeWarpLvl, c, o);
    SetWindowTextA(hBtnUpgWarp, buf);

    c = 180 + g_State.upgradeSensorsLvl * 140;
    o = 10 + g_State.upgradeSensorsLvl * 8;
    wsprintfA(buf, "[5] SENSOR ARRAYS (Lvl %d) - %d Cr + %d Ore (+25 E / -2 Scan)", g_State.upgradeSensorsLvl, c, o);
    SetWindowTextA(hBtnUpgSensors, buf);
}

void OpenUpgradeModal(HWND hwnd) {
    g_State.inUpgradeModal = 1;
    g_State.inStationModal = 0;
    UpdateUpgradeButtonLabels();
    UpdateControlVisibility(hwnd);
    PlayGameSound(5);
}

void BuyHullUpgrade(HWND hwnd) {
    int c = 200 + g_State.upgradeHullLvl * 150;
    int o = 15 + g_State.upgradeHullLvl * 10;
    if (g_State.credits < c || g_State.ore < o) {
        AddLog("[WARN] Insufficient Credits or Ore for Hull upgrade!", 1);
        PlayGameSound(6);
        return;
    }
    g_State.credits -= c;
    g_State.ore -= o;
    g_State.upgradeHullLvl++;
    g_State.maxHull += 25;
    g_State.hull += 25;
    char buf[128];
    wsprintfA(buf, "[UPGRADE BAY] Hull Plating upgraded to Lvl %d! (+25 Max HP)", g_State.upgradeHullLvl);
    AddLog(buf, 3);
    PlayGameSound(7);
    UpdateUpgradeButtonLabels();
    InvalidateRect(hwnd, NULL, TRUE);
}

void BuyCannonsUpgrade(HWND hwnd) {
    int c = 250 + g_State.upgradeCannonsLvl * 180;
    int o = 20 + g_State.upgradeCannonsLvl * 12;
    if (g_State.credits < c || g_State.ore < o) {
        AddLog("[WARN] Insufficient Credits or Ore for Plasma Cannons upgrade!", 1);
        PlayGameSound(6);
        return;
    }
    g_State.credits -= c;
    g_State.ore -= o;
    g_State.upgradeCannonsLvl++;
    char buf[128];
    wsprintfA(buf, "[UPGRADE BAY] Plasma Cannons upgraded to Lvl %d! (+10 Phaser DMG)", g_State.upgradeCannonsLvl);
    AddLog(buf, 3);
    PlayGameSound(7);
    UpdateUpgradeButtonLabels();
    InvalidateRect(hwnd, NULL, TRUE);
}

void BuyShieldsUpgrade(HWND hwnd) {
    int c = 220 + g_State.upgradeShieldsLvl * 160;
    int o = 15 + g_State.upgradeShieldsLvl * 10;
    if (g_State.credits < c || g_State.ore < o) {
        AddLog("[WARN] Insufficient Credits or Ore for Particle Shields upgrade!", 1);
        PlayGameSound(6);
        return;
    }
    g_State.credits -= c;
    g_State.ore -= o;
    g_State.upgradeShieldsLvl++;
    g_State.maxShields += 20;
    g_State.shields += 20;
    char buf[128];
    wsprintfA(buf, "[UPGRADE BAY] Particle Shields upgraded to Lvl %d! (+20 Max Shields)", g_State.upgradeShieldsLvl);
    AddLog(buf, 3);
    PlayGameSound(7);
    UpdateUpgradeButtonLabels();
    InvalidateRect(hwnd, NULL, TRUE);
}

void BuyWarpUpgrade(HWND hwnd) {
    int c = 300 + g_State.upgradeWarpLvl * 200;
    int o = 25 + g_State.upgradeWarpLvl * 15;
    if (g_State.credits < c || g_State.ore < o) {
        AddLog("[WARN] Insufficient Credits or Ore for Warp Drive upgrade!", 1);
        PlayGameSound(6);
        return;
    }
    g_State.credits -= c;
    g_State.ore -= o;
    g_State.upgradeWarpLvl++;
    g_State.maxFuel += 30;
    g_State.fuel += 30;
    char buf[128];
    wsprintfA(buf, "[UPGRADE BAY] Warp Drive Range upgraded to Lvl %d! (+30 Fuel / -2 Jump Fuel)", g_State.upgradeWarpLvl);
    AddLog(buf, 3);
    PlayGameSound(7);
    UpdateUpgradeButtonLabels();
    InvalidateRect(hwnd, NULL, TRUE);
}

void BuySensorsUpgrade(HWND hwnd) {
    int c = 180 + g_State.upgradeSensorsLvl * 140;
    int o = 10 + g_State.upgradeSensorsLvl * 8;
    if (g_State.credits < c || g_State.ore < o) {
        AddLog("[WARN] Insufficient Credits or Ore for Sensor Arrays upgrade!", 1);
        PlayGameSound(6);
        return;
    }
    g_State.credits -= c;
    g_State.ore -= o;
    g_State.upgradeSensorsLvl++;
    g_State.maxEnergy += 25;
    g_State.energy += 25;
    char buf[128];
    wsprintfA(buf, "[UPGRADE BAY] Sensor Arrays upgraded to Lvl %d! (+25 Energy / -2 Scan Energy)", g_State.upgradeSensorsLvl);
    AddLog(buf, 3);
    PlayGameSound(7);
    UpdateUpgradeButtonLabels();
    InvalidateRect(hwnd, NULL, TRUE);
}

void MoveShip(HWND hwnd, int dx, int dy) {
    if (g_State.inInitModal || g_State.inStationModal || g_State.inPlanetModal || g_State.inEncounterModal) return;

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

    // Biome Environmental Effects
    if (g_State.currentBiome == 1 && (xrand() % 100) < 15) { // Volcanic
        int flare = (xrand() % 4) + 2;
        if (g_State.shields >= flare) {
          g_State.shields -= flare;
          AddLog("[VOLCANIC] Geothermal solar flare scorched deflectors!", 1);
        } else {
          g_State.hull = (g_State.hull > flare) ? g_State.hull - flare : 0;
          AddLog("[VOLCANIC] Thermal radiation scorched hull structure!", 2);
        }
    } else if (g_State.currentBiome == 2 && (xrand() % 100) < 20) { // Nebula
        int energyGain = (xrand() % 6) + 4;
        g_State.energy = (g_State.energy + energyGain > g_State.maxEnergy) ? g_State.maxEnergy : g_State.energy + energyGain;
        AddLog("[NEBULA] Plasma cloud absorbed (+Reactor Energy).", 3);
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
        TriggerRandomEncounter(hwnd, 2);
    } else if (cell == 5) { // Pirate
        TriggerRandomEncounter(hwnd, 0);
    } else if (cell == 6) { // Planet
        char buf[128];
        wsprintfA(buf, "[ORBIT] Entered orbit around %s. Execute Planet Scan.", g_State.planetData[g_State.shipY][g_State.shipX].name);
        AddLog(buf, 0);
    } else if (cell == 0 && (xrand() % 100) < 20) {
        int r = xrand() % 3;
        int enc = (r == 0) ? 1 : ((r == 1) ? 3 : 4);
        TriggerRandomEncounter(hwnd, enc);
    }

    InvalidateRect(hwnd, NULL, TRUE);
}

void ExecuteScan(HWND hwnd) {
    int cost = (10 - g_State.upgradeSensorsLvl * 2 < 3) ? 3 : (10 - g_State.upgradeSensorsLvl * 2);
    if (g_State.energy < cost) {
        AddLog("[WARN] Insufficient energy for sensor scan.", 1);
        PlayGameSound(6);
    } else {
        g_State.energy -= cost;
        int st = 0, ast = 0, anom = 0, pir = 0, plan = 0, x, y;
        for (y = 0; y < 8; y++) {
            for (x = 0; x < 8; x++) {
                int c = g_State.gridMap[y][x];
                if (c == 2) st++;
                else if (c == 3) ast++;
                else if (c == 4) anom++;
                else if (c == 5) pir++;
                else if (c == 6) plan++;
            }
        }
        char buf[128];
        wsprintfA(buf, "[SCAN COMPLETE] Planets: %d | Outposts: %d | Asteroids: %d | Hostiles: %d", plan, st, ast, pir);
        AddLog(buf, 0);
        PlayGameSound(2);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void ExecutePlanetScan(HWND hwnd) {
    int cell = g_State.gridMap[g_State.shipY][g_State.shipX];
    if (cell != 6) {
        AddLog("[WARN] Target is not a planet cell. Move starship to a Planet location.", 1);
        PlayGameSound(6);
        return;
    }
    int cost = (12 - g_State.upgradeSensorsLvl * 2 < 4) ? 4 : (12 - g_State.upgradeSensorsLvl * 2);
    if (g_State.energy < cost) {
        AddLog("[WARN] Insufficient reactor energy for orbital planet scan.", 1);
        PlayGameSound(6);
        return;
    }

    g_State.energy -= cost;
    PlayGameSound(8);

    PlanetCellData* p = &g_State.planetData[g_State.shipY][g_State.shipX];
    if (!p->scanned) {
        p->scanned = 1;
        g_State.ore += p->minerals;
        g_State.gas += p->gas;
        g_State.artifacts += p->artifacts;

        char buf[160];
        wsprintfA(buf, "[ORBITAL SCAN] %s surveyed! Minerals +%d | Gas +%d | Artifacts +%d", p->name, p->minerals, p->gas, p->artifacts);
        AddLog(buf, 3);
    } else {
        char buf[160];
        wsprintfA(buf, "[ORBITAL SURVEY] %s already scanned. Minerals: %d | Gas: %d | Artifacts: %d", p->name, p->minerals, p->gas, p->artifacts);
        AddLog(buf, 0);
    }

    g_State.inPlanetModal = 1;
    UpdateControlVisibility(hwnd);
    InvalidateRect(hwnd, NULL, TRUE);
}

void ExecuteWarp(HWND hwnd) {
    int cost = (15 - g_State.upgradeWarpLvl * 2 < 5) ? 5 : (15 - g_State.upgradeWarpLvl * 2);
    if (g_State.fuel < cost) {
        AddLog("[ALERT] Insufficient Hyper Fuel for sector jump!", 2);
        PlayGameSound(6);
    } else {
        g_State.fuel -= cost;
        const char* sectors[] = {"ALPHA-01", "BETA-07", "GAMMA-09", "NEBULA-X", "SOLARIS-IV", "ORION-9", "OMEGA-99"};
        int idx = xrand() % 7;
        int i;
        for (i = 0; sectors[idx][i] != '\0'; i++) g_State.sector[i] = sectors[idx][i];
        g_State.sector[i] = '\0';

        g_State.shipX = xrand() % 8;
        g_State.shipY = xrand() % 8;
        GenerateSectorGrid();

        char buf[128];
        wsprintfA(buf, "[WARP JUMP] Disengaged hyperdrive. Arrived at sector [%s].", g_BiomeNames[g_State.currentBiome]);
        AddLog(buf, 3);
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
    wsprintfA(buf, "[STATION] Sold %d tons of minerals for +%d Cr!", g_State.ore, earned);
    AddLog(buf, 3);
    g_State.ore = 0;
    PlayGameSound(7);
    InvalidateRect(hwnd, NULL, TRUE);
}

void StationSellGas(HWND hwnd) {
    if (g_State.gas <= 0) {
        AddLog("[WARN] No gas reserves in cargo bay to sell.", 1);
        return;
    }
    int earned = g_State.gas * 20;
    g_State.credits += earned;
    char buf[128];
    wsprintfA(buf, "[STATION] Sold %d units plasma gas for +%d Cr!", g_State.gas, earned);
    AddLog(buf, 3);
    g_State.gas = 0;
    PlayGameSound(7);
    InvalidateRect(hwnd, NULL, TRUE);
}

void StationSellArtifacts(HWND hwnd) {
    if (g_State.artifacts <= 0) {
        AddLog("[WARN] No ancient artifacts in inventory to sell.", 1);
        return;
    }
    int earned = g_State.artifacts * 150;
    g_State.credits += earned;
    char buf[128];
    wsprintfA(buf, "[STATION] Sold %d ancient relics to museum for +%d Cr!", g_State.artifacts, earned);
    AddLog(buf, 3);
    g_State.artifacts = 0;
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

// Phase 6: Random Encounter Engine
void TriggerRandomEncounter(HWND hwnd, int type) {
    g_State.inEncounterModal = 1;
    g_State.encounterType = type;

    if (type == 0) { // PIRATE
        SetWindowTextA(hBtnEncChoice1, "[1] ENTER TACTICAL COMBAT (-12 Energy)");
        SetWindowTextA(hBtnEncChoice2, "[2] PAY TRIBUTE TOLL (-80 Credits)");
        SetWindowTextA(hBtnEncChoice3, "[3] EVASIVE MANEUVERS (-10 Fuel)");
    } else if (type == 1) { // DERELICT
        SetWindowTextA(hBtnEncChoice1, "[1] SEND SALVAGE CREW (+30 Scrap, +1 Relic)");
        SetWindowTextA(hBtnEncChoice2, "[2] SIPHON FUEL MATRIX (+30 Fuel, +20 Energy)");
        SetWindowTextA(hBtnEncChoice3, "[3] DOWNLOAD SHIP LOGS (+100 Credits)");
    } else if (type == 2) { // ANOMALY
        SetWindowTextA(hBtnEncChoice1, "[1] HARVEST ION CORE (100% Energy, +20 Shield)");
        SetWindowTextA(hBtnEncChoice2, "[2] STABILIZE GRAVITY CORE (-15 Energy)");
        SetWindowTextA(hBtnEncChoice3, "[3] OVERCHARGE DEFLECTORS (-10 Fuel)");
    } else if (type == 3) { // TRADER
        SetWindowTextA(hBtnEncChoice1, "[1] TRADE 20 ORE FOR ANCIENT RELIC");
        SetWindowTextA(hBtnEncChoice2, "[2] TRADE 10 GAS FOR 35 HYPER FUEL");
        SetWindowTextA(hBtnEncChoice3, "[3] BUY TECH SCRAP PACK (-100 Credits)");
    } else if (type == 4) { // DISTRESS
        SetWindowTextA(hBtnEncChoice1, "[1] ASSIST REPAIRS (-15 Scrap)");
        SetWindowTextA(hBtnEncChoice2, "[2] BEAM HYPER FUEL (-20 Fuel)");
        SetWindowTextA(hBtnEncChoice3, "[3] IGNORE DISTRESS SIGNAL");
    }

    PlayGameSound(6);
    UpdateControlVisibility(hwnd);
}

void ResolveEncounterChoice(HWND hwnd, int choiceIdx) {
    if (!g_State.inEncounterModal) return;
    int type = g_State.encounterType;
    int resolved = 0;

    if (type == 0) { // PIRATE
        if (choiceIdx == 0) {
            if (g_State.energy < 12) {
                AddLog("[WARN] Insufficient reactor energy for tactical combat!", 1);
                PlayGameSound(6);
                return;
            }
            StartTacticalCombat(hwnd, 0);
            return;
        } else if (choiceIdx == 1) {
            if (g_State.credits < 80) {
                AddLog("[ALERT] Insufficient credits! Pirates open fire!", 2);
                g_State.hull = (g_State.hull > 15) ? g_State.hull - 15 : 0;
                PlayGameSound(6);
            } else {
                g_State.credits -= 80;
                AddLog("[PIRATE TOLL] Paid 80 Credits tribute for safe passage.", 1);
                PlayGameSound(7);
            }
            resolved = 1;
        } else if (choiceIdx == 2) {
            if (g_State.fuel < 10) {
                AddLog("[WARN] Insufficient hyper fuel for evasive burn!", 1);
                PlayGameSound(6);
                return;
            }
            g_State.fuel -= 10;
            if ((xrand() % 100) < 70) {
                AddLog("[EVASION SUCCESS] Outmaneuvered pirate raider cleanly!", 3);
                PlayGameSound(4);
            } else {
                g_State.shields = (g_State.shields > 12) ? g_State.shields - 12 : 0;
                AddLog("[EVASION FAILURE] Took 12 shield damage while escaping!", 1);
                PlayGameSound(6);
            }
            resolved = 1;
        }
    } else if (type == 1) { // DERELICT
        if (choiceIdx == 0) {
            g_State.scraps += 30;
            g_State.artifacts += 1;
            if ((xrand() % 100) < 20 && g_State.crew > 1) {
                g_State.crew -= 1;
                AddLog("[SALVAGE HAZARD] Recovered +30 Scrap & +1 Relic, but lost 1 crew member!", 2);
                PlayGameSound(6);
            } else {
                AddLog("[SALVAGE SUCCESS] Recovered +30 Tech Scrap & +1 Ancient Relic!", 3);
                PlayGameSound(7);
            }
            resolved = 1;
        } else if (choiceIdx == 1) {
            g_State.fuel = (g_State.fuel + 30 > g_State.maxFuel) ? g_State.maxFuel : g_State.fuel + 30;
            g_State.energy = (g_State.energy + 20 > g_State.maxEnergy) ? g_State.maxEnergy : g_State.energy + 20;
            AddLog("[SIPHON] Siphoned +30 Hyper Fuel & +20 Energy from derelict reactor!", 3);
            PlayGameSound(7);
            resolved = 1;
        } else if (choiceIdx == 2) {
            g_State.credits += 100;
            AddLog("[DATA LOGS] Uploaded vessel telemetry database for +100 Credits bounty!", 3);
            PlayGameSound(7);
            resolved = 1;
        }
    } else if (type == 2) { // ANOMALY
        if (choiceIdx == 0) {
            g_State.energy = g_State.maxEnergy;
            g_State.shields = (g_State.shields + 20 > g_State.maxShields) ? g_State.maxShields : g_State.shields + 20;
            AddLog("[QUANTUM SURGE] Reactor fully recharged & +20 Deflector Boost!", 3);
            PlayGameSound(7);
            resolved = 1;
        } else if (choiceIdx == 1) {
            if (g_State.energy < 15) {
                AddLog("[WARN] Insufficient energy to stabilize gravity core!", 1);
                PlayGameSound(6);
                return;
            }
            g_State.energy -= 15;
            g_State.artifacts += 2;
            g_State.scraps += 20;
            AddLog("[GRAVITY STABILIZED] Extracted +2 Ancient Relics & +20 Scrap!", 3);
            PlayGameSound(7);
            resolved = 1;
        } else if (choiceIdx == 2) {
            if (g_State.fuel < 10) {
                AddLog("[WARN] Insufficient fuel for shield overcharge!", 1);
                PlayGameSound(6);
                return;
            }
            g_State.fuel -= 10;
            g_State.shields = (g_State.shields + 40 > g_State.maxShields) ? g_State.maxShields : g_State.shields + 40;
            AddLog("[DEFLECTOR SURGE] Shield matrix overcharged (+40 Shields)!", 3);
            PlayGameSound(7);
            resolved = 1;
        }
    } else if (type == 3) { // TRADER
        if (choiceIdx == 0) {
            if (g_State.ore < 20) {
                AddLog("[WARN] Insufficient minerals to trade for alien relic!", 1);
                PlayGameSound(6);
                return;
            }
            g_State.ore -= 20;
            g_State.artifacts += 1;
            AddLog("[ALIEN BARTER] Traded 20 Minerals for +1 Ancient Relic!", 3);
            PlayGameSound(7);
            resolved = 1;
        } else if (choiceIdx == 1) {
            if (g_State.gas < 10) {
                AddLog("[WARN] Insufficient plasma gas to trade for fuel!", 1);
                PlayGameSound(6);
                return;
            }
            g_State.gas -= 10;
            g_State.fuel = (g_State.fuel + 35 > g_State.maxFuel) ? g_State.maxFuel : g_State.fuel + 35;
            AddLog("[ALIEN BARTER] Traded 10 Plasma Gas for +35 Hyper Fuel!", 3);
            PlayGameSound(7);
            resolved = 1;
        } else if (choiceIdx == 2) {
            if (g_State.credits < 100) {
                AddLog("[WARN] Insufficient credits to buy tech scrap!", 1);
                PlayGameSound(6);
                return;
            }
            g_State.credits -= 100;
            g_State.scraps += 30;
            AddLog("[ALIEN COMMERCE] Purchased +30 Tech Scrap for 100 Credits!", 3);
            PlayGameSound(7);
            resolved = 1;
        }
    } else if (type == 4) { // DISTRESS
        if (choiceIdx == 0) {
            if (g_State.scraps < 15) {
                AddLog("[WARN] Insufficient tech scrap to assist repairs!", 1);
                PlayGameSound(6);
                return;
            }
            g_State.scraps -= 15;
            g_State.credits += 200;
            g_State.crew += 2;
            AddLog("[RESCUE SUCCESS] Repaired freighter! Awarded +200 Cr & +2 Crew!", 3);
            PlayGameSound(7);
            resolved = 1;
        } else if (choiceIdx == 1) {
            if (g_State.fuel < 20) {
                AddLog("[WARN] Insufficient hyper fuel to beam to stranded vessel!", 1);
                PlayGameSound(6);
                return;
            }
            g_State.fuel -= 20;
            g_State.credits += 150;
            g_State.gas += 15;
            AddLog("[RESCUE SUCCESS] Transferred fuel! Awarded +150 Cr & +15 Gas!", 3);
            PlayGameSound(7);
            resolved = 1;
        } else if (choiceIdx == 2) {
            AddLog("[DISTRESS IGNORED] Bypassed SOS beacon signal.", 0);
            resolved = 1;
        }
    }

    if (resolved) {
        g_State.inEncounterModal = 0;
        UpdateControlVisibility(hwnd);
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

// Phase 7: Turn-based Tactical Ship Combat Engine
void AddCombatFeed(const char* msg) {
    if (g_State.combatFeedCount < 4) {
        int i;
        for (i = 0; msg[i] != '\0' && i < 127; i++) g_State.combatFeed[g_State.combatFeedCount][i] = msg[i];
        g_State.combatFeed[g_State.combatFeedCount][i] = '\0';
        g_State.combatFeedCount++;
    } else {
        int i;
        for (i = 0; i < 3; i++) {
            int k;
            for (k = 0; g_State.combatFeed[i + 1][k] != '\0'; k++) g_State.combatFeed[i][k] = g_State.combatFeed[i + 1][k];
            g_State.combatFeed[i][k] = '\0';
        }
        int k;
        for (k = 0; msg[k] != '\0' && k < 127; k++) g_State.combatFeed[3][k] = msg[k];
        g_State.combatFeed[3][k] = '\0';
    }
}

void StartTacticalCombat(HWND hwnd, int presetIdx) {
    if (g_State.energy < 12) {
        AddLog("[WARN] Insufficient reactor energy to power tactical weapon targeting matrix!", 1);
        PlayGameSound(6);
        return;
    }
    g_State.energy -= 12;
    g_State.inCombatModal = 1;
    g_State.inEncounterModal = 0;
    g_State.targetSubsystem = 0;
    g_State.playerEvading = 0;
    g_State.combatFeedCount = 0;

    const char* names[] = {"CYBERNETIC PIRATE RAIDER", "VOID CRUISER DREADNOUGHT", "OBSIDIAN RAVAGER INTERCEPTOR", "IMPERIAL STAR DESTROYER"};
    int hulls[] = {85, 120, 65, 140};
    int shields[] = {45, 75, 55, 90};
    int powers[] = {15, 22, 18, 26};
    int evasions[] = {15, 8, 30, 5};

    int p = presetIdx % 4;
    int i;
    for (i = 0; names[p][i] != '\0'; i++) g_State.enemyName[i] = names[p][i];
    g_State.enemyName[i] = '\0';

    g_State.enemyHull = hulls[p]; g_State.enemyMaxHull = hulls[p];
    g_State.enemyShields = shields[p]; g_State.enemyMaxShields = shields[p];
    g_State.enemyWeaponPower = powers[p];
    g_State.enemyWeaponsDamaged = 0;
    g_State.enemyEvasion = evasions[p];
    g_State.enemyEnginesDamaged = 0;

    char buf[128];
    wsprintfA(buf, "TACTICAL LOCK: %s in visual space!", g_State.enemyName);
    AddCombatFeed(buf);
    AddCombatFeed("Targeting matrix engaged. Choose target subsystem & fire!");

    PlayGameSound(6);
    UpdateControlVisibility(hwnd);
}

void CombatVictory(HWND hwnd) {
    g_State.credits += 120;
    g_State.scraps += 25;
    g_State.fuel = (g_State.fuel + 20 > g_State.maxFuel) ? g_State.maxFuel : g_State.fuel + 20;
    g_State.artifacts += 1;

    char buf[160];
    wsprintfA(buf, "[COMBAT VICTORY] Destroyed %s! Claimed +120 Cr, +25 Scrap, +20 Fuel & +1 Relic!", g_State.enemyName);
    AddLog(buf, 3);
    PlayGameSound(7);

    if (g_State.gridMap[g_State.shipY][g_State.shipX] == 5) {
        g_State.gridMap[g_State.shipY][g_State.shipX] = 0;
    }

    g_State.inCombatModal = 0;
    UpdateControlVisibility(hwnd);
}

void CombatDefeat(HWND hwnd) {
    AddLog("[CRITICAL BREACH] Hull compromised! Emergency escape pod activated.", 2);
    g_State.hull = 25;
    g_State.shields = 0;
    PlayGameSound(6);

    g_State.inCombatModal = 0;
    UpdateControlVisibility(hwnd);
}

void CombatEnemyTurn(HWND hwnd) {
    if (!g_State.inCombatModal || g_State.enemyHull <= 0) return;

    if (g_State.enemyShields <= 0 && (xrand() % 100) < 25) {
        g_State.enemyShields = 25;
        char buf[128];
        wsprintfA(buf, "[ENEMY ACTION] %s recharged deflector shields (+25 HP)!", g_State.enemyName);
        AddCombatFeed(buf);
        InvalidateRect(hwnd, NULL, TRUE);
        return;
    }

    int hitChance = g_State.playerEvading ? 45 : 85;
    g_State.playerEvading = 0;

    if ((xrand() % 100) >= hitChance) {
        char buf[128];
        wsprintfA(buf, "[ENEMY ATTACK] %s fired weapons, but MISSED due to evasion!", g_State.enemyName);
        AddCombatFeed(buf);
        PlayGameSound(4);
    } else {
        int dmg = g_State.enemyWeaponPower + (xrand() % 8) - 3;
        if (dmg < 5) dmg = 5;

        if (g_State.shields > 0) {
            if (g_State.shields >= dmg) {
                g_State.shields -= dmg;
                char buf[128];
                wsprintfA(buf, "[ENEMY ATTACK] %s struck shields for %d DMG!", g_State.enemyName, dmg);
                AddCombatFeed(buf);
            } else {
                int rem = dmg - g_State.shields;
                g_State.shields = 0;
                g_State.hull = (g_State.hull > rem) ? g_State.hull - rem : 0;
                char buf[128];
                wsprintfA(buf, "[SHIELD BREACH] %s breached shields! Took %d HULL DMG!", g_State.enemyName, rem);
                AddCombatFeed(buf);
            }
        } else {
            g_State.hull = (g_State.hull > dmg) ? g_State.hull - dmg : 0;
            char buf[128];
            wsprintfA(buf, "[HULL BREACH] %s struck hull for %d DMG!", g_State.enemyName, dmg);
            AddCombatFeed(buf);
        }
        PlayGameSound(6);
    }

    if (g_State.hull <= 0) {
        CombatDefeat(hwnd);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void CombatPlayerFire(HWND hwnd) {
    if (!g_State.inCombatModal) return;
    if (g_State.energy < 12) {
        AddCombatFeed("[WARN] Need 12 Energy to fire phaser cannons!");
        PlayGameSound(6);
        return;
    }
    g_State.energy -= 12;

    int currentEnemyEvasion = g_State.enemyEnginesDamaged ? 0 : g_State.enemyEvasion;
    int hitRoll = xrand() % 100;

    if (hitRoll < currentEnemyEvasion) {
        char buf[128];
        wsprintfA(buf, "[PLAYER FIRE] Phasers missed! %s executed evasive roll!", g_State.enemyName);
        AddCombatFeed(buf);
        PlayGameSound(4);
    } else {
        int baseDmg = (xrand() % 12) + 18 + (g_State.upgradeCannonsLvl * 10);
        int sys = g_State.targetSubsystem;

        if (sys == 1) { // SHIELDS
            if (g_State.enemyShields > 0) {
                int shDmg = (int)(baseDmg * 1.5);
                g_State.enemyShields = (g_State.enemyShields > shDmg) ? g_State.enemyShields - shDmg : 0;
                char buf[128];
                wsprintfA(buf, "[TARGET: SHIELD CORE] Heavy ion beam struck shields for %d DMG!", shDmg);
                AddCombatFeed(buf);
            } else {
                g_State.enemyHull = (g_State.enemyHull > baseDmg) ? g_State.enemyHull - baseDmg : 0;
                char buf[128];
                wsprintfA(buf, "[TARGET: SHIELD CORE] Shields down! Phaser struck hull for %d DMG!", baseDmg);
                AddCombatFeed(buf);
            }
        } else if (sys == 2) { // WEAPONS
            int actualDmg = baseDmg;
            if (g_State.enemyShields > 0) {
                int shDmg = (int)(baseDmg * 0.7);
                g_State.enemyShields = (g_State.enemyShields > shDmg) ? g_State.enemyShields - shDmg : 0;
                actualDmg = (int)(baseDmg * 0.3);
            }
            g_State.enemyHull = (g_State.enemyHull > actualDmg) ? g_State.enemyHull - actualDmg : 0;
            g_State.enemyWeaponsDamaged = 1;
            g_State.enemyWeaponPower = (int)(g_State.enemyWeaponPower * 0.7);
            if (g_State.enemyWeaponPower < 6) g_State.enemyWeaponPower = 6;
            char buf[128];
            wsprintfA(buf, "[TARGET: WEAPONS] Disrupted weapon array! (%d Hull DMG, -30%% Pwr)", actualDmg);
            AddCombatFeed(buf);
        } else if (sys == 3) { // ENGINES
            int actualDmg = baseDmg;
            if (g_State.enemyShields > 0) {
                int shDmg = (int)(baseDmg * 0.7);
                g_State.enemyShields = (g_State.enemyShields > shDmg) ? g_State.enemyShields - shDmg : 0;
                actualDmg = (int)(baseDmg * 0.3);
            }
            g_State.enemyHull = (g_State.enemyHull > actualDmg) ? g_State.enemyHull - actualDmg : 0;
            g_State.enemyEnginesDamaged = 1;
            char buf[128];
            wsprintfA(buf, "[TARGET: THRUSTERS] Scorched thrusters! Evasion reduced to 0%%! (%d Hull DMG)", actualDmg);
            AddCombatFeed(buf);
        } else { // HULL
            if (g_State.enemyShields > 0) {
                int shDmg = (int)(baseDmg * 0.6);
                int hDmg = (int)(baseDmg * 0.5);
                g_State.enemyShields = (g_State.enemyShields > shDmg) ? g_State.enemyShields - shDmg : 0;
                g_State.enemyHull = (g_State.enemyHull > hDmg) ? g_State.enemyHull - hDmg : 0;
                char buf[128];
                wsprintfA(buf, "[TARGET: CORE HULL] Pierced deflectors! (%d Shield / %d Hull DMG)", shDmg, hDmg);
                AddCombatFeed(buf);
            } else {
                g_State.enemyHull = (g_State.enemyHull > baseDmg) ? g_State.enemyHull - baseDmg : 0;
                char buf[128];
                wsprintfA(buf, "[TARGET: CORE HULL] DIRECT HIT ON HULL! Dealt %d critical DMG!", baseDmg);
                AddCombatFeed(buf);
            }
        }
        PlayGameSound(7);
    }

    if (g_State.enemyHull <= 0) {
        CombatVictory(hwnd);
    } else {
        CombatEnemyTurn(hwnd);
    }
}

void CombatPlayerShield(HWND hwnd) {
    if (!g_State.inCombatModal) return;
    if (g_State.energy < 15) {
        AddCombatFeed("[WARN] Need 15 Energy for deflector overcharge!");
        PlayGameSound(6);
        return;
    }
    g_State.energy -= 15;
    g_State.shields = (g_State.shields + 30 > g_State.maxShields) ? g_State.maxShields : g_State.shields + 30;
    AddCombatFeed("[DEFLECTOR OVERCHARGE] Overcharged shields (+30 HP)!");
    PlayGameSound(7);

    CombatEnemyTurn(hwnd);
}

void CombatPlayerEvade(HWND hwnd) {
    if (!g_State.inCombatModal) return;
    if (g_State.energy < 10 || g_State.fuel < 5) {
        AddCombatFeed("[WARN] Need 10 Energy & 5 Fuel for evasive burn!");
        PlayGameSound(6);
        return;
    }
    g_State.energy -= 10;
    g_State.fuel -= 5;
    g_State.playerEvading = 1;
    AddCombatFeed("[EVASIVE BURN] Lateral thrusters active (+35% Evasion)!");
    PlayGameSound(4);

    CombatEnemyTurn(hwnd);
}

void CombatPlayerRepair(HWND hwnd) {
    if (!g_State.inCombatModal) return;
    if (g_State.scraps >= 10) {
        g_State.scraps -= 10;
        g_State.hull = (g_State.hull + 25 > g_State.maxHull) ? g_State.maxHull : g_State.hull + 25;
        AddCombatFeed("[EMERGENCY REPAIR] Used 10 Tech Scrap (+25 Hull HP)!");
        PlayGameSound(7);
    } else if (g_State.energy >= 15) {
        g_State.energy -= 15;
        g_State.hull = (g_State.hull + 20 > g_State.maxHull) ? g_State.maxHull : g_State.hull + 20;
        AddCombatFeed("[EMERGENCY REPAIR] Diverted energy to nanites (+20 Hull HP)!");
        PlayGameSound(7);
    } else {
        AddCombatFeed("[WARN] Insufficient Scrap / Energy for damage control!");
        PlayGameSound(6);
        return;
    }

    CombatEnemyTurn(hwnd);
}

void CombatPlayerFlee(HWND hwnd) {
    if (!g_State.inCombatModal) return;
    if (g_State.fuel < 15) {
        AddCombatFeed("[WARN] Need 15 Hyper Fuel to charge emergency warp flee!");
        PlayGameSound(6);
        return;
    }
    g_State.fuel -= 15;
    if ((xrand() % 100) < 65) {
        char buf[128];
        wsprintfA(buf, "[TACTICAL RETREAT] Outmaneuvered %s and engaged hyperdrive!", g_State.enemyName);
        AddLog(buf, 3);
        PlayGameSound(4);
        g_State.inCombatModal = 0;
        UpdateControlVisibility(hwnd);
    } else {
        AddCombatFeed("[WARP RETREAT FAILED] Tractor beam disrupted warp startup!");
        PlayGameSound(6);
        CombatEnemyTurn(hwnd);
    }
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
            hBgBrush = CreateSolidBrush(RGB(3, 6, 13));
            hPanelBrush = CreateSolidBrush(RGB(6, 14, 28));
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
            hBtnScan       = CreateWindowA("BUTTON", "📡 SCAN SECTOR", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 48, 195, 21, hwnd, (HMENU)102, NULL, NULL);
            hBtnScanPlanet = CreateWindowA("BUTTON", "🪐 PLANET SCAN", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 70, 195, 21, hwnd, (HMENU)108, NULL, NULL);
            hBtnWarp       = CreateWindowA("BUTTON", "🌌 HYPER WARP",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 92, 195, 21, hwnd, (HMENU)103, NULL, NULL);
            hBtnMine       = CreateWindowA("BUTTON", "⛏️ MINE ASTEROID", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 114, 195, 21, hwnd, (HMENU)104, NULL, NULL);
            hBtnDock       = CreateWindowA("BUTTON", "⚓ DOCK OUTPOST", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 136, 195, 21, hwnd, (HMENU)105, NULL, NULL);
            hBtnShield     = CreateWindowA("BUTTON", "🛡️ BOOST DEFLECT",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 158, 195, 21, hwnd, (HMENU)106, NULL, NULL);
            hBtnRepair     = CreateWindowA("BUTTON", "🛠️ REPAIR HULL",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 180, 195, 21, hwnd, (HMENU)107, NULL, NULL);
            hBtnEncScan    = CreateWindowA("BUTTON", "🎲 ENCOUNTER SCAN",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 202, 195, 21, hwnd, (HMENU)109, NULL, NULL);
            hBtnCombat     = CreateWindowA("BUTTON", "⚔️ TACTICAL COMBAT",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 224, 195, 21, hwnd, (HMENU)110, NULL, NULL);
            hBtnUpgrade    = CreateWindowA("BUTTON", "🔧 SHIP UPGRADE BAY",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 246, 195, 21, hwnd, (HMENU)111, NULL, NULL);

            // D-Pad Navigation Buttons
            hBtnNW     = CreateWindowA("BUTTON", "NW",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 272, 60, 21, hwnd, (HMENU)201, NULL, NULL);
            hBtnN      = CreateWindowA("BUTTON", "N ▲",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 697, 272, 60, 21, hwnd, (HMENU)202, NULL, NULL);
            hBtnNE     = CreateWindowA("BUTTON", "NE",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 765, 272, 60, 21, hwnd, (HMENU)203, NULL, NULL);

            hBtnW      = CreateWindowA("BUTTON", "◄ W",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 295, 60, 21, hwnd, (HMENU)204, NULL, NULL);
            hBtnCenter = CreateWindowA("BUTTON", "●",    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 697, 295, 60, 21, hwnd, (HMENU)205, NULL, NULL);
            hBtnE      = CreateWindowA("BUTTON", "E ►",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 765, 295, 60, 21, hwnd, (HMENU)206, NULL, NULL);

            hBtnSW     = CreateWindowA("BUTTON", "SW",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 318, 60, 21, hwnd, (HMENU)207, NULL, NULL);
            hBtnS      = CreateWindowA("BUTTON", "S ▼",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 697, 318, 60, 21, hwnd, (HMENU)208, NULL, NULL);
            hBtnSE     = CreateWindowA("BUTTON", "SE",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 765, 318, 60, 21, hwnd, (HMENU)209, NULL, NULL);

            // Init Modal Buttons
            hBtnClass0 = CreateWindowA("BUTTON", "[1] CORVETTE (Explorer)", WS_CHILD | BS_PUSHBUTTON, 230, 150, 380, 28, hwnd, (HMENU)301, NULL, NULL);
            hBtnClass1 = CreateWindowA("BUTTON", "[2] FRIGATE (Mining)",    WS_CHILD | BS_PUSHBUTTON, 230, 184, 380, 28, hwnd, (HMENU)302, NULL, NULL);
            hBtnClass2 = CreateWindowA("BUTTON", "[3] INTERCEPTOR (Tactical)",WS_CHILD | BS_PUSHBUTTON, 230, 218, 380, 28, hwnd, (HMENU)303, NULL, NULL);
            hBtnClass3 = CreateWindowA("BUTTON", "[4] CRUISER (Deep Space)",WS_CHILD | BS_PUSHBUTTON, 230, 252, 380, 28, hwnd, (HMENU)304, NULL, NULL);
            hBtnConfirmInit = CreateWindowA("BUTTON", "🚀 INITIALIZE COMMAND DECK", WS_CHILD | BS_PUSHBUTTON, 230, 300, 380, 34, hwnd, (HMENU)305, NULL, NULL);

            // Station Modal Buttons
            hBtnStRefuel        = CreateWindowA("BUTTON", "⛽ REFUEL HYPER TANK (+30 Fuel - 40 Cr)",  WS_CHILD | BS_PUSHBUTTON, 230, 130, 380, 25, hwnd, (HMENU)401, NULL, NULL);
            hBtnStRepair        = CreateWindowA("BUTTON", "🛠️ HULL RECONDITIONING (+40 HP - 80 Cr)", WS_CHILD | BS_PUSHBUTTON, 230, 158, 380, 25, hwnd, (HMENU)402, NULL, NULL);
            hBtnStSellOre       = CreateWindowA("BUTTON", "💰 SELL MINED ORE / MINERALS (+25 Cr/Ton)",WS_CHILD | BS_PUSHBUTTON, 230, 186, 380, 25, hwnd, (HMENU)403, NULL, NULL);
            hBtnStSellGas       = CreateWindowA("BUTTON", "💨 SELL PLASMA GAS RESERVES (+20 Cr/Unit)",WS_CHILD | BS_PUSHBUTTON, 230, 214, 380, 25, hwnd, (HMENU)406, NULL, NULL);
            hBtnStSellArtifacts = CreateWindowA("BUTTON", "🏛️ SELL ANCIENT ARTIFACTS (+150 Cr/Relic)", WS_CHILD | BS_PUSHBUTTON, 230, 242, 380, 25, hwnd, (HMENU)407, NULL, NULL);
            hBtnStRecruit       = CreateWindowA("BUTTON", "👨‍🚀 RECRUIT CREW OFFICERS (+3 - 100 Cr)",   WS_CHILD | BS_PUSHBUTTON, 230, 270, 380, 25, hwnd, (HMENU)404, NULL, NULL);
            hBtnStUpgrade       = CreateWindowA("BUTTON", "🔧 SHIP SYSTEMS UPGRADE BAY",             WS_CHILD | BS_PUSHBUTTON, 230, 298, 380, 25, hwnd, (HMENU)408, NULL, NULL);
            hBtnStClose         = CreateWindowA("BUTTON", "✕ CLOSE / UNDOCK STATION",               WS_CHILD | BS_PUSHBUTTON, 230, 328, 380, 26, hwnd, (HMENU)405, NULL, NULL);

            // Planet Telemetry Modal Button
            hBtnPlanetClose     = CreateWindowA("BUTTON", "✅ LOG SURVEY DATA & RESUME COMMAND",     WS_CHILD | BS_PUSHBUTTON, 230, 300, 380, 32, hwnd, (HMENU)501, NULL, NULL);

            // Encounter Modal Buttons
            hBtnEncChoice1 = CreateWindowA("BUTTON", "[1] CHOICE 1", WS_CHILD | BS_PUSHBUTTON, 230, 150, 380, 28, hwnd, (HMENU)601, NULL, NULL);
            hBtnEncChoice2 = CreateWindowA("BUTTON", "[2] CHOICE 2", WS_CHILD | BS_PUSHBUTTON, 230, 185, 380, 28, hwnd, (HMENU)602, NULL, NULL);
            hBtnEncChoice3 = CreateWindowA("BUTTON", "[3] CHOICE 3", WS_CHILD | BS_PUSHBUTTON, 230, 220, 380, 28, hwnd, (HMENU)603, NULL, NULL);
            hBtnEncClose   = CreateWindowA("BUTTON", "✕ DISENGAGE ENCOUNTER", WS_CHILD | BS_PUSHBUTTON, 230, 270, 380, 28, hwnd, (HMENU)604, NULL, NULL);

            // Tactical Combat Modal Buttons (Phase 7)
            hBtnTgtHull    = CreateWindowA("BUTTON", "[1] CORE HULL",    WS_CHILD | BS_PUSHBUTTON, 185, 218, 112, 22, hwnd, (HMENU)706, NULL, NULL);
            hBtnTgtShield  = CreateWindowA("BUTTON", "[2] SHIELD CORE",  WS_CHILD | BS_PUSHBUTTON, 301, 218, 112, 22, hwnd, (HMENU)707, NULL, NULL);
            hBtnTgtWeap    = CreateWindowA("BUTTON", "[3] WEAPONS",      WS_CHILD | BS_PUSHBUTTON, 417, 218, 112, 22, hwnd, (HMENU)708, NULL, NULL);
            hBtnTgtEng     = CreateWindowA("BUTTON", "[4] THRUSTERS",    WS_CHILD | BS_PUSHBUTTON, 533, 218, 112, 22, hwnd, (HMENU)709, NULL, NULL);

            hBtnCombatFire   = CreateWindowA("BUTTON", "⚔️ FIRE PHASERS (-12 E)",         WS_CHILD | BS_PUSHBUTTON, 185, 245, 226, 26, hwnd, (HMENU)701, NULL, NULL);
            hBtnCombatShield = CreateWindowA("BUTTON", "🛡️ OVERCHARGE SHIELDS (-15 E)",   WS_CHILD | BS_PUSHBUTTON, 417, 245, 226, 26, hwnd, (HMENU)702, NULL, NULL);
            hBtnCombatEvade  = CreateWindowA("BUTTON", "⚡ EVASIVE MANEUVER (-10 E, -5 F)",WS_CHILD | BS_PUSHBUTTON, 185, 275, 226, 26, hwnd, (HMENU)703, NULL, NULL);
            hBtnCombatRepair = CreateWindowA("BUTTON", "🛠️ DAMAGE CONTROL (-10 Scrap)",   WS_CHILD | BS_PUSHBUTTON, 417, 275, 226, 26, hwnd, (HMENU)704, NULL, NULL);
            hBtnCombatFlee   = CreateWindowA("BUTTON", "🚀 TACTICAL WARP FLEE (-15 Fuel)",WS_CHILD | BS_PUSHBUTTON, 185, 305, 458, 26, hwnd, (HMENU)705, NULL, NULL);

            // Upgrade Modal Buttons (Phase 8)
            hBtnUpgHull    = CreateWindowA("BUTTON", "[1] HULL PLATING",    WS_CHILD | BS_PUSHBUTTON, 210, 140, 420, 28, hwnd, (HMENU)801, NULL, NULL);
            hBtnUpgCannons = CreateWindowA("BUTTON", "[2] PLASMA CANNONS",  WS_CHILD | BS_PUSHBUTTON, 210, 174, 420, 28, hwnd, (HMENU)802, NULL, NULL);
            hBtnUpgShields = CreateWindowA("BUTTON", "[3] PARTICLE SHIELDS",WS_CHILD | BS_PUSHBUTTON, 210, 208, 420, 28, hwnd, (HMENU)803, NULL, NULL);
            hBtnUpgWarp    = CreateWindowA("BUTTON", "[4] WARP DRIVE RANGE",WS_CHILD | BS_PUSHBUTTON, 210, 242, 420, 28, hwnd, (HMENU)804, NULL, NULL);
            hBtnUpgSensors = CreateWindowA("BUTTON", "[5] SENSOR ARRAYS",   WS_CHILD | BS_PUSHBUTTON, 210, 276, 420, 28, hwnd, (HMENU)805, NULL, NULL);
            hBtnUpgClose   = CreateWindowA("BUTTON", "✕ EXIT UPGRADE BAY",  WS_CHILD | BS_PUSHBUTTON, 210, 316, 420, 28, hwnd, (HMENU)806, NULL, NULL);

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
            if (g_State.mapMode == 0 && !g_State.inInitModal && !g_State.inStationModal && !g_State.inPlanetModal && !g_State.inEncounterModal && !g_State.inCombatModal) {
                RECT rectMap = {260, 48, 615, 393};
                InvalidateRect(hwnd, &rectMap, FALSE);
            }
            break;
        }

        case WM_KEYDOWN: {
            if (g_State.inInitModal || g_State.inStationModal || g_State.inPlanetModal || g_State.inEncounterModal || g_State.inCombatModal) break;

            switch (wParam) {
                case VK_UP:    MoveShip(hwnd, 0, -1); break;
                case VK_DOWN:  MoveShip(hwnd, 0, 1);  break;
                case VK_LEFT:  MoveShip(hwnd, -1, 0); break;
                case VK_RIGHT: MoveShip(hwnd, 1, 0);  break;
                case 'W':      MoveShip(hwnd, 0, -1); break;
                case 'S':      MoveShip(hwnd, 0, 1);  break;
                case 'A':      MoveShip(hwnd, -1, 0); break;
                case 'D':      MoveShip(hwnd, 1, 0);  break;
                case 'P':      ExecutePlanetScan(hwnd); break;
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
            else if (id == 108) ExecutePlanetScan(hwnd);
            else if (id == 103) ExecuteWarp(hwnd);
            else if (id == 104) ExecuteMine(hwnd);
            else if (id == 105) ExecuteDock(hwnd);
            else if (id == 106) ExecuteShieldBoost(hwnd);
            else if (id == 107) ExecuteRepair(hwnd);
            else if (id == 109) {
                if (g_State.energy >= 8) {
                    g_State.energy -= 8;
                    TriggerRandomEncounter(hwnd, xrand() % 5);
                } else {
                    AddLog("[WARN] Insufficient energy for tactical encounter scan.", 1);
                    PlayGameSound(6);
                }
            } else if (id == 110) {
                StartTacticalCombat(hwnd, xrand() % 4);
            } else if (id == 111 || id == 408) {
                OpenUpgradeModal(hwnd);
            } else if (id == 801) BuyHullUpgrade(hwnd);
            else if (id == 802) BuyCannonsUpgrade(hwnd);
            else if (id == 803) BuyShieldsUpgrade(hwnd);
            else if (id == 804) BuyWarpUpgrade(hwnd);
            else if (id == 805) BuySensorsUpgrade(hwnd);
            else if (id == 806) {
                g_State.inUpgradeModal = 0;
                UpdateControlVisibility(hwnd);
                AddLog("[UPGRADE BAY] Exited Subsystem Upgrade Bay.", 0);
            } else if (id == 701) CombatPlayerFire(hwnd);
            else if (id == 702) CombatPlayerShield(hwnd);
            else if (id == 703) CombatPlayerEvade(hwnd);
            else if (id == 704) CombatPlayerRepair(hwnd);
            else if (id == 705) CombatPlayerFlee(hwnd);
            else if (id >= 706 && id <= 709) {
                g_State.targetSubsystem = id - 706;
                InvalidateRect(hwnd, NULL, TRUE);
            }
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
            else if (id == 406) StationSellGas(hwnd);
            else if (id == 407) StationSellArtifacts(hwnd);
            else if (id == 404) StationRecruitCrew(hwnd);
            else if (id == 405) {
                g_State.inStationModal = 0;
                UpdateControlVisibility(hwnd);
                AddLog("[STATION] Undocked from orbital outpost.", 0);
            } else if (id == 501) {
                g_State.inPlanetModal = 0;
                UpdateControlVisibility(hwnd);
                AddLog("[ORBIT] Planetary telemetry logged.", 0);
            } else if (id == 601) ResolveEncounterChoice(hwnd, 0);
            else if (id == 602) ResolveEncounterChoice(hwnd, 1);
            else if (id == 603) ResolveEncounterChoice(hwnd, 2);
            else if (id == 604) {
                g_State.inEncounterModal = 0;
                UpdateControlVisibility(hwnd);
                AddLog("[ENCOUNTER] Disengaged tactical event.", 0);
            }
            SetFocus(hwnd);
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            HBRUSH hBorderCyanB = CreateSolidBrush(RGB(0, 139, 155));
            HBRUSH hBorderAmberB = CreateSolidBrush(RGB(255, 170, 0));

            // Header Bar
            SelectObject(hdc, hTitleFont);
            SetTextColor(hdc, RGB(0, 240, 255));
            SetBkMode(hdc, TRANSPARENT);
            TextOutA(hdc, 12, 10, "🚀 KSTARSHIP", 14);

            SelectObject(hdc, hMonoFont);
            SetTextColor(hdc, RGB(255, 170, 0));
            TextOutA(hdc, 160, 14, "[CYBERNETIC SECTOR COMMAND v5.0]", 32);

            char sectorBuf[96];
            char colChar = (char)('A' + g_State.shipX);
            wsprintfA(sectorBuf, "SECTOR: %s [%s] GRID: %c%d", g_State.sector, g_BiomeNames[g_State.currentBiome], colChar, g_State.shipY + 1);
            SetTextColor(hdc, RGB(0, 240, 255));
            TextOutA(hdc, 630, 14, sectorBuf, (int)lstrlenA(sectorBuf));

            // Left Panel: Ship Vitals & Manifest
            RECT leftPanel = {10, 48, 250, 393};
            FillRect(hdc, &leftPanel, hPanelBrush);
            FrameRect(hdc, &leftPanel, hBorderCyanB);

            SetTextColor(hdc, RGB(0, 240, 255));
            TextOutA(hdc, 20, 56, "[ SHIP VITALS ]", 15);
            SetTextColor(hdc, RGB(255, 170, 0));
            TextOutA(hdc, 170, 56, g_State.shipClass, (int)lstrlenA(g_State.shipClass));

            SelectObject(hdc, hBoldFont);
            SetTextColor(hdc, RGB(0, 240, 255));
            TextOutA(hdc, 20, 74, g_State.shipName, (int)lstrlenA(g_State.shipName));
            SelectObject(hdc, hMonoFont);

            // Status Bars
            char valBuf[32];
            SetTextColor(hdc, RGB(180, 210, 240));

            wsprintfA(valBuf, "%d/%d", g_State.hull, g_State.maxHull);
            TextOutA(hdc, 20, 92, "HULL INTEGRITY:", 15);
            TextOutA(hdc, 190, 92, valBuf, (int)lstrlenA(valBuf));
            DrawBar(hdc, 20, 106, 210, 8, g_State.hull, g_State.maxHull, RGB(57, 255, 20));

            wsprintfA(valBuf, "%d/%d", g_State.fuel, g_State.maxFuel);
            TextOutA(hdc, 20, 118, "HYPER FUEL:", 11);
            TextOutA(hdc, 190, 118, valBuf, (int)lstrlenA(valBuf));
            DrawBar(hdc, 20, 132, 210, 8, g_State.fuel, g_State.maxFuel, RGB(255, 170, 0));

            wsprintfA(valBuf, "%d/%d", g_State.energy, g_State.maxEnergy);
            TextOutA(hdc, 20, 144, "REACTOR ENERGY:", 15);
            TextOutA(hdc, 190, 144, valBuf, (int)lstrlenA(valBuf));
            DrawBar(hdc, 20, 158, 210, 8, g_State.energy, g_State.maxEnergy, RGB(0, 240, 255));

            wsprintfA(valBuf, "%d/%d", g_State.shields, g_State.maxShields);
            TextOutA(hdc, 20, 170, "SHIELD MATRIX:", 14);
            TextOutA(hdc, 190, 170, valBuf, (int)lstrlenA(valBuf));
            DrawBar(hdc, 20, 184, 210, 8, g_State.shields, g_State.maxShields, RGB(59, 130, 246));

            // Manifest & Resources Grid (2x3 grid)
            SetTextColor(hdc, RGB(0, 240, 255));
            TextOutA(hdc, 20, 200, "[ MANIFEST & CARGO ]", 20);

            RECT resCard1 = {20, 218, 120, 268};
            RECT resCard2 = {130, 218, 230, 268};
            RECT resCard3 = {20, 274, 120, 324};
            RECT resCard4 = {130, 274, 230, 324};
            RECT resCard5 = {20, 330, 120, 380};
            RECT resCard6 = {130, 330, 230, 380};

            HBRUSH hResBg = CreateSolidBrush(RGB(10, 24, 48));
            FillRect(hdc, &resCard1, hResBg);
            FillRect(hdc, &resCard2, hResBg);
            FillRect(hdc, &resCard3, hResBg);
            FillRect(hdc, &resCard4, hResBg);
            FillRect(hdc, &resCard5, hResBg);
            FillRect(hdc, &resCard6, hResBg);
            DeleteObject(hResBg);

            SetTextColor(hdc, RGB(0, 240, 255));
            SelectObject(hdc, hBoldFont);

            wsprintfA(valBuf, "%d", g_State.credits);
            TextOutA(hdc, 30, 224, valBuf, (int)lstrlenA(valBuf));
            wsprintfA(valBuf, "%d", g_State.ore);
            TextOutA(hdc, 140, 224, valBuf, (int)lstrlenA(valBuf));

            wsprintfA(valBuf, "%d", g_State.gas);
            TextOutA(hdc, 30, 280, valBuf, (int)lstrlenA(valBuf));
            wsprintfA(valBuf, "%d", g_State.artifacts);
            TextOutA(hdc, 140, 280, valBuf, (int)lstrlenA(valBuf));

            wsprintfA(valBuf, "%d", g_State.crew);
            TextOutA(hdc, 30, 336, valBuf, (int)lstrlenA(valBuf));
            wsprintfA(valBuf, "%d", g_State.scraps);
            TextOutA(hdc, 140, 336, valBuf, (int)lstrlenA(valBuf));

            SelectObject(hdc, hMonoFont);
            SetTextColor(hdc, RGB(123, 155, 185));
            TextOutA(hdc, 30, 246, "CREDITS", 7);
            TextOutA(hdc, 140, 246, "MINERALS", 8);
            TextOutA(hdc, 30, 302, "GAS", 3);
            TextOutA(hdc, 140, 302, "ARTIFACTS", 9);
            TextOutA(hdc, 30, 358, "CREW", 4);
            TextOutA(hdc, 140, 358, "SCRAP", 5);

            // Center Viewport: Star Map Grid
            RECT centerPanel = {260, 48, 615, 393};
            HBRUSH hMapBg = CreateSolidBrush(RGB(2, 4, 8));
            FillRect(hdc, &centerPanel, hMapBg);
            DeleteObject(hMapBg);
            FrameRect(hdc, &centerPanel, hBorderCyanB);

            // HUD Info
            SetTextColor(hdc, RGB(0, 240, 255));
            TextOutA(hdc, 270, 54, "SYSTEM: SOLARIS PRIME", 21);

            char biomeHud[64];
            wsprintfA(biomeHud, "BIOME: %s", g_BiomeNames[g_State.currentBiome]);
            SetTextColor(hdc, RGB(57, 255, 20));
            TextOutA(hdc, 445, 54, biomeHud, (int)lstrlenA(biomeHud));

            const char* objectNames[] = {"DEEP SPACE", "SOLAR STAR", "ORBITAL OUTPOST", "ASTEROID FIELD", "QUANTUM ANOMALY", "HOSTILE PIRATE", "ORBITAL PLANET"};
            int curCellType = g_State.gridMap[g_State.shipY][g_State.shipX];
            char objBuf[64];
            if (curCellType == 6) {
                wsprintfA(objBuf, "OBJECT: PLANET [%s]", g_State.planetData[g_State.shipY][g_State.shipX].name);
            } else {
                wsprintfA(objBuf, "OBJECT: %s", objectNames[curCellType]);
            }
            SetTextColor(hdc, RGB(255, 170, 0));
            TextOutA(hdc, 270, 68, objBuf, (int)lstrlenA(objBuf));

            if (g_State.mapMode == 0) { // Graphical GDI Render
                // Background Stars tinted per biome
                int s;
                for (s = 0; s < 75; s++) {
                    int sx = 265 + g_Stars[s].x;
                    int sy = 72 + g_Stars[s].y;
                    if (sx >= 262 && sx <= 610 && sy >= 85 && sy <= 388) {
                        COLORREF starCol = RGB(g_Stars[s].alpha, g_Stars[s].alpha, (g_Stars[s].alpha + 40 > 255) ? 255 : g_Stars[s].alpha + 40);
                        if (g_State.currentBiome == 1) starCol = RGB((g_Stars[s].alpha + 60 > 255) ? 255 : g_Stars[s].alpha + 60, g_Stars[s].alpha / 2, g_Stars[s].alpha / 2);
                        else if (g_State.currentBiome == 2) starCol = RGB((g_Stars[s].alpha + 40 > 255) ? 255 : g_Stars[s].alpha + 40, g_Stars[s].alpha / 2, (g_Stars[s].alpha + 60 > 255) ? 255 : g_Stars[s].alpha + 60);
                        else if (g_State.currentBiome == 3) starCol = RGB((g_Stars[s].alpha + 50 > 255) ? 255 : g_Stars[s].alpha + 50, (g_Stars[s].alpha + 40 > 255) ? 255 : g_Stars[s].alpha + 40, g_Stars[s].alpha / 3);

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
                int startX = 285, startY = 98;
                int cellW = 38, cellH = 34;

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
                            } else if (type == 4) { // Anomaly
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
                            } else if (type == 6) { // Orbital Planet
                                HBRUSH hPlB = CreateSolidBrush(RGB(0, 255, 170));
                                HPEN hPlP = CreatePen(PS_SOLID, 1, RGB(180, 255, 230));
                                HPEN hOldP = (HPEN)SelectObject(hdc, hPlP);
                                HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hPlB);
                                Ellipse(hdc, cx - 7, cy - 7, cx + 7, cy + 7);

                                // Ring
                                HPEN hRingP = CreatePen(PS_SOLID, 1, RGB(0, 220, 150));
                                SelectObject(hdc, hRingP);
                                Arc(hdc, cx - 11, cy - 4, cx + 11, cy + 4, cx - 11, cy, cx + 11, cy);
                                DeleteObject(hRingP);

                                SelectObject(hdc, hOldP);
                                SelectObject(hdc, hOldB);
                                DeleteObject(hPlP);
                                DeleteObject(hPlB);
                            }
                        }
                    }
                }
            } else { // ASCII Matrix Render
                SetTextColor(hdc, RGB(0, 139, 155));
                int gx, gy;
                for (gx = 0; gx < 8; gx++) {
                    char cHeader[2] = {(char)('A' + gx), '\0'};
                    TextOutA(hdc, 305 + (gx * 35), 78, cHeader, 1);
                }

                for (gy = 0; gy < 8; gy++) {
                    char rHeader[2] = {(char)('1' + gy), '\0'};
                    SetTextColor(hdc, RGB(0, 139, 155));
                    TextOutA(hdc, 280, 98 + (gy * 34), rHeader, 1);

                    for (gx = 0; gx < 8; gx++) {
                        int cellX = 295 + (gx * 35);
                        int cellY = 94 + (gy * 34);

                        RECT cRect = {cellX, cellY, cellX + 32, cellY + 30};
                        HPEN hCellPen = CreatePen(PS_SOLID, 1, RGB(15, 35, 60));
                        HPEN hOldP = (HPEN)SelectObject(hdc, hCellPen);
                        HBRUSH hOldB = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
                        Rectangle(hdc, cRect.left, cRect.top, cRect.right, cRect.bottom);
                        SelectObject(hdc, hOldP);
                        SelectObject(hdc, hOldB);
                        DeleteObject(hCellPen);

                        if (gx == g_State.shipX && gy == g_State.shipY) {
                            SetTextColor(hdc, RGB(0, 240, 255));
                            TextOutA(hdc, cellX + 11, cellY + 7, "^", 1);
                        } else {
                            int type = g_State.gridMap[gy][gx];
                            if (type == 1) {
                                SetTextColor(hdc, RGB(255, 238, 85));
                                TextOutA(hdc, cellX + 11, cellY + 7, "*", 1);
                            } else if (type == 2) {
                                SetTextColor(hdc, RGB(59, 130, 246));
                                TextOutA(hdc, cellX + 11, cellY + 7, "O", 1);
                            } else if (type == 3) {
                                SetTextColor(hdc, RGB(255, 170, 0));
                                TextOutA(hdc, cellX + 11, cellY + 7, "#", 1);
                            } else if (type == 4) {
                                SetTextColor(hdc, RGB(176, 38, 255));
                                TextOutA(hdc, cellX + 11, cellY + 7, "?", 1);
                            } else if (type == 5) {
                                SetTextColor(hdc, RGB(255, 51, 102));
                                TextOutA(hdc, cellX + 11, cellY + 7, "X", 1);
                            } else if (type == 6) {
                                SetTextColor(hdc, RGB(0, 255, 170));
                                TextOutA(hdc, cellX + 11, cellY + 7, "P", 1);
                            } else {
                                SetTextColor(hdc, RGB(25, 50, 85));
                                TextOutA(hdc, cellX + 11, cellY + 7, ".", 1);
                            }
                        }
                    }
                }
            }

            // Right Panel Frame (Controls Header)
            RECT rightPanel = {620, 48, 830, 393};
            FillRect(hdc, &rightPanel, hPanelBrush);
            FrameRect(hdc, &rightPanel, hBorderCyanB);

            // Bottom Panel: Log Console
            RECT logPanel = {10, 402, 830, 545};
            FillRect(hdc, &logPanel, hPanelBrush);
            FrameRect(hdc, &logPanel, hBorderAmberB);

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

                char logLine[160];
                wsprintfA(logLine, "> %s", g_Logs[logIdx].msg);
                TextOutA(hdc, 20, 426 + (i * 16), logLine, (int)lstrlenA(logLine));
            }

            DeleteObject(hBorderCyanB);
            DeleteObject(hBorderAmberB);

            // Render Active Modals
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
                TextOutA(hdc, 230, 115, "Captain, select vessel blueprint to launch sector exploration:", 61);

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

                RECT modalCard = {210, 60, 630, 375};
                FillRect(hdc, &modalCard, hPanelBrush);
                FrameRect(hdc, &modalCard, (HBRUSH)GetStockObject(WHITE_BRUSH));

                SelectObject(hdc, hTitleFont);
                SetTextColor(hdc, RGB(0, 240, 255));
                TextOutA(hdc, 230, 75, "⚓ ORBITAL OUTPOST SERVICES", 26);

                SelectObject(hdc, hMonoFont);
                SetTextColor(hdc, RGB(160, 190, 220));
                TextOutA(hdc, 230, 105, "Welcome Captain. Station refueling docks & outpost supplies active:", 67);
            } else if (g_State.inPlanetModal) {
                RECT modalOverlay = {10, 48, 830, 393};
                HBRUSH hDimBrush = CreateSolidBrush(RGB(4, 10, 22));
                FillRect(hdc, &modalOverlay, hDimBrush);
                DeleteObject(hDimBrush);

                RECT modalCard = {210, 70, 630, 360};
                FillRect(hdc, &modalCard, hPanelBrush);
                FrameRect(hdc, &modalCard, (HBRUSH)GetStockObject(WHITE_BRUSH));

                SelectObject(hdc, hTitleFont);
                SetTextColor(hdc, RGB(0, 255, 170));
                TextOutA(hdc, 230, 85, "🪐 ORBITAL PLANET TELEMETRY", 27);

                SelectObject(hdc, hMonoFont);
                SetTextColor(hdc, RGB(160, 190, 220));
                char pDesc[128];
                PlanetCellData* p = &g_State.planetData[g_State.shipY][g_State.shipX];
                wsprintfA(pDesc, "Survey telemetry report for target world: %s", p->name);
                TextOutA(hdc, 230, 115, pDesc, (int)lstrlenA(pDesc));

                char yield1[64], yield2[64], yield3[64];
                wsprintfA(yield1, "MINERALS (ORE): +%d Tons", p->minerals);
                wsprintfA(yield2, "PLASMA GAS:     +%d Units", p->gas);
                wsprintfA(yield3, "ARTIFACTS:      +%d Relics", p->artifacts);

                SetTextColor(hdc, RGB(57, 255, 20));
                TextOutA(hdc, 250, 150, yield1, (int)lstrlenA(yield1));
                SetTextColor(hdc, RGB(0, 240, 255));
                TextOutA(hdc, 250, 180, yield2, (int)lstrlenA(yield2));
                SetTextColor(hdc, RGB(255, 170, 0));
                TextOutA(hdc, 250, 210, yield3, (int)lstrlenA(yield3));

                SetTextColor(hdc, RGB(180, 200, 220));
                TextOutA(hdc, 230, 250, "SURVEY STATUS: ANALYSIS COMPLETE & CARGO LOGGED", 47);
            } else if (g_State.inEncounterModal) {
                RECT modalOverlay = {10, 48, 830, 393};
                HBRUSH hDimBrush = CreateSolidBrush(RGB(4, 10, 22));
                FillRect(hdc, &modalOverlay, hDimBrush);
                DeleteObject(hDimBrush);

                RECT modalCard = {210, 65, 630, 370};
                FillRect(hdc, &modalCard, hPanelBrush);
                FrameRect(hdc, &modalCard, (HBRUSH)GetStockObject(WHITE_BRUSH));

                SelectObject(hdc, hTitleFont);
                COLORREF titleCol = RGB(255, 170, 0);
                const char* titleText = "🚨 RANDOM SPACE ENCOUNTER";
                const char* descText = "Deep space tactical event detected. Select command decision:";

                if (g_State.encounterType == 0) {
                    titleCol = RGB(255, 51, 102);
                    titleText = "🚨 HOSTILE PIRATE AMBUSH";
                    descText = "A heavily armed raider locks weapons and demands tribute!";
                } else if (g_State.encounterType == 1) {
                    titleCol = RGB(255, 170, 0);
                    titleText = "🚢 DERELICT SHIP ADRIFT";
                    descText = "An abandoned vessel is drifting with salvageable components.";
                } else if (g_State.encounterType == 2) {
                    titleCol = RGB(176, 38, 255);
                    titleText = "🌀 QUANTUM SPATIAL ANOMALY";
                    descText = "A localized tachyon rift distorts space-time around your sensors.";
                } else if (g_State.encounterType == 3) {
                    titleCol = RGB(0, 255, 170);
                    titleText = "👽 XENON ALIEN TRADER";
                    descText = "A benevolent alien merchant ship offers exotic barter.";
                } else if (g_State.encounterType == 4) {
                    titleCol = RGB(0, 240, 255);
                    titleText = "📻 SOS DISTRESS BEACON";
                    descText = "A stranded civilian freighter broadcasts an urgent SOS!";
                }

                SetTextColor(hdc, titleCol);
                TextOutA(hdc, 230, 80, titleText, (int)lstrlenA(titleText));

                SelectObject(hdc, hMonoFont);
                SetTextColor(hdc, RGB(160, 190, 220));
                TextOutA(hdc, 230, 110, descText, (int)lstrlenA(descText));
            } else if (g_State.inCombatModal) {
                RECT modalOverlay = {10, 48, 830, 393};
                HBRUSH hDimBrush = CreateSolidBrush(RGB(4, 10, 22));
                FillRect(hdc, &modalOverlay, hDimBrush);
                DeleteObject(hDimBrush);

                RECT modalCard = {170, 50, 670, 390};
                FillRect(hdc, &modalCard, hPanelBrush);
                HBRUSH hRedPen = CreateSolidBrush(RGB(255, 51, 102));
                FrameRect(hdc, &modalCard, hRedPen);
                DeleteObject(hRedPen);

                SelectObject(hdc, hTitleFont);
                SetTextColor(hdc, RGB(255, 51, 102));
                TextOutA(hdc, 185, 60, "⚔️ TURN-BASED TACTICAL SHIP COMBAT", 34);

                SelectObject(hdc, hMonoFont);
                // Player Vitals (Left Card)
                RECT playerCard = {185, 82, 410, 195};
                HBRUSH hCardBg = CreateSolidBrush(RGB(4, 12, 24));
                FillRect(hdc, &playerCard, hCardBg);
                DeleteObject(hCardBg);
                FrameRect(hdc, &playerCard, hBorderCyanB);

                SelectObject(hdc, hBoldFont);
                SetTextColor(hdc, RGB(0, 240, 255));
                TextOutA(hdc, 195, 88, g_State.shipName, (int)lstrlenA(g_State.shipName));

                SelectObject(hdc, hMonoFont);
                SetTextColor(hdc, RGB(180, 210, 240));
                char pVal[32];
                wsprintfA(pVal, "HULL: %d/%d", g_State.hull, g_State.maxHull);
                TextOutA(hdc, 195, 108, pVal, (int)lstrlenA(pVal));
                DrawBar(hdc, 195, 122, 200, 6, g_State.hull, g_State.maxHull, RGB(57, 255, 20));

                wsprintfA(pVal, "SHIELD: %d/%d", g_State.shields, g_State.maxShields);
                TextOutA(hdc, 195, 132, pVal, (int)lstrlenA(pVal));
                DrawBar(hdc, 195, 146, 200, 6, g_State.shields, g_State.maxShields, RGB(59, 130, 246));

                wsprintfA(pVal, "ENERGY: %d/%d", g_State.energy, g_State.maxEnergy);
                TextOutA(hdc, 195, 156, pVal, (int)lstrlenA(pVal));
                DrawBar(hdc, 195, 170, 200, 6, g_State.energy, g_State.maxEnergy, RGB(0, 240, 255));

                if (g_State.playerEvading) {
                    SetTextColor(hdc, RGB(0, 240, 255));
                    TextOutA(hdc, 195, 180, "STATUS: +35% EVASION ACTIVE", 27);
                } else {
                    SetTextColor(hdc, RGB(100, 140, 180));
                    TextOutA(hdc, 195, 180, "STATUS: STANCE ONLINE", 21);
                }

                // Enemy Vitals (Right Card)
                RECT enemyCard = {430, 82, 655, 195};
                HBRUSH hEnemyCardBg = CreateSolidBrush(RGB(4, 12, 24));
                FillRect(hdc, &enemyCard, hEnemyCardBg);
                DeleteObject(hEnemyCardBg);
                HBRUSH hRedBorder = CreateSolidBrush(RGB(255, 51, 102));
                FrameRect(hdc, &enemyCard, hRedBorder);
                DeleteObject(hRedBorder);

                SelectObject(hdc, hBoldFont);
                SetTextColor(hdc, RGB(255, 51, 102));
                TextOutA(hdc, 440, 88, g_State.enemyName, (int)lstrlenA(g_State.enemyName));

                SelectObject(hdc, hMonoFont);
                SetTextColor(hdc, RGB(180, 210, 240));
                wsprintfA(pVal, "HULL: %d/%d", (g_State.enemyHull > 0) ? g_State.enemyHull : 0, g_State.enemyMaxHull);
                TextOutA(hdc, 440, 108, pVal, (int)lstrlenA(pVal));
                DrawBar(hdc, 440, 122, 200, 6, g_State.enemyHull, g_State.enemyMaxHull, RGB(255, 51, 102));

                wsprintfA(pVal, "SHIELD: %d/%d", (g_State.enemyShields > 0) ? g_State.enemyShields : 0, g_State.enemyMaxShields);
                TextOutA(hdc, 440, 132, pVal, (int)lstrlenA(pVal));
                DrawBar(hdc, 440, 146, 200, 6, g_State.enemyShields, g_State.enemyMaxShields, RGB(59, 130, 246));

                if (g_State.enemyWeaponsDamaged) {
                    SetTextColor(hdc, RGB(255, 170, 0));
                    TextOutA(hdc, 440, 160, "WEAPONS: DAMAGED (-30% PWR)", 27);
                } else {
                    SetTextColor(hdc, RGB(57, 255, 20));
                    TextOutA(hdc, 440, 160, "WEAPONS: ONLINE (100%)", 22);
                }

                if (g_State.enemyEnginesDamaged) {
                    SetTextColor(hdc, RGB(255, 51, 102));
                    TextOutA(hdc, 440, 178, "ENGINES: THRUSTERS DISABLED", 27);
                } else {
                    SetTextColor(hdc, RGB(0, 255, 170));
                    wsprintfA(pVal, "ENGINES: ACTIVE (%d%% EVASION)", g_State.enemyEvasion);
                    TextOutA(hdc, 440, 178, pVal, (int)lstrlenA(pVal));
                }

                // Subsystem Target Header
                SetTextColor(hdc, RGB(0, 240, 255));
                TextOutA(hdc, 185, 200, "TARGET SUBSYSTEM:", 17);

                // Combat Feed Log Box (Bottom of Modal)
                RECT combatLogRect = {185, 336, 655, 385};
                HBRUSH hFeedBg = CreateSolidBrush(RGB(2, 6, 12));
                FillRect(hdc, &combatLogRect, hFeedBg);
                DeleteObject(hFeedBg);
                FrameRect(hdc, &combatLogRect, hBorderCyanB);

                int c;
                for (c = 0; c < g_State.combatFeedCount; c++) {
                    SetTextColor(hdc, RGB(0, 240, 255));
                    char feedLine[140];
                    wsprintfA(feedLine, "> %s", g_State.combatFeed[c]);
                    TextOutA(hdc, 192, 340 + (c * 11), feedLine, (int)lstrlenA(feedLine));
                }
            } else if (g_State.inUpgradeModal) {
                RECT modalOverlay = {10, 48, 830, 393};
                HBRUSH hDimBrush = CreateSolidBrush(RGB(4, 10, 22));
                FillRect(hdc, &modalOverlay, hDimBrush);
                DeleteObject(hDimBrush);

                RECT modalCard = {190, 55, 650, 375};
                FillRect(hdc, &modalCard, hPanelBrush);
                FrameRect(hdc, &modalCard, hBorderCyanB);

                SelectObject(hdc, hTitleFont);
                SetTextColor(hdc, RGB(0, 240, 255));
                TextOutA(hdc, 210, 68, "🔧 SHIP SYSTEMS UPGRADE BAY", 27);

                SelectObject(hdc, hMonoFont);
                SetTextColor(hdc, RGB(160, 190, 220));
                TextOutA(hdc, 210, 92, "Upgrade vessel technology using Credits & Mined Minerals (Ore):", 62);

                char resBuf[96];
                wsprintfA(resBuf, "CREDITS: %d | MINERAL ORE: %d", g_State.credits, g_State.ore);
                SetTextColor(hdc, RGB(57, 255, 20));
                TextOutA(hdc, 210, 112, resBuf, (int)lstrlenA(resBuf));
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
