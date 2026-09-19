#include <windows.h>

#pragma function(memset)
void* memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) {
        *bytes++ = (char)c;
    }
    return dest;
}

#pragma function(memcpy)
void* memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) {
        *d++ = *s++;
    }
    return dest;
}

/* Pseudo-random number generator */
static unsigned int g_seed = 98765;
static int MyRand() {
    g_seed = (214013 * g_seed + 2531011);
    return (g_seed >> 16) & 0x7FFF;
}

/* Sound Thread Engine */
static DWORD WINAPI SoundThread(LPVOID lpParam) {
    int type = (int)(intptr_t)lpParam;
    if (type == 1) { // Click / select
        Beep(520, 20);
    } else if (type == 2) { // Place block
        Beep(780, 25);
    } else if (type == 3) { // Remove block
        Beep(320, 25);
    } else if (type == 4) { // Laser / Autocannon fire
        Beep(980, 30);
    } else if (type == 5) { // Plasma / Heavy weapon
        Beep(440, 50);
        Beep(220, 60);
    } else if (type == 6) { // Mining beam
        Beep(650, 40);
    } else if (type == 7) { // Quicksave
        Beep(700, 50);
        Beep(900, 70);
    } else if (type == 8) { // Quickload
        Beep(900, 50);
        Beep(700, 70);
    } else if (type == 9) { // Contract complete fanfare
        Beep(523, 80);
        Beep(659, 80);
        Beep(784, 100);
        Beep(1046, 200);
    } else if (type == 10) { // Warning / Overheat
        Beep(920, 60);
        Sleep(30);
        Beep(750, 60);
    }
    return 0;
}

static void PlaySfx(int type) {
    CreateThread(NULL, 0, SoundThread, (LPVOID)(intptr_t)type, 0, NULL);
}

/* Game States */
#define STATE_SPLASH    0
#define STATE_BLUEPRINT 1
#define STATE_DRYDOCK   2
#define STATE_FLIGHT    3
#define STATE_CONTRACTS 4
#define STATE_HELP      5
#define STATE_TUTORIAL  6

#define GRID_SZ 12

/* Module Types */
enum ModuleType {
    MOD_EMPTY = 0,
    MOD_HULL_BASIC = 1,
    MOD_HULL_ARMOR = 2,
    MOD_BRIDGE = 3,
    MOD_REACTOR_FISSION = 4,
    MOD_REACTOR_FUSION = 5,
    MOD_THRUSTER_CHEM = 6,
    MOD_THRUSTER_PLASMA = 7,
    MOD_RADIATOR = 8,
    MOD_WEAPON_CANNON = 9,
    MOD_WEAPON_PLASMA = 10,
    MOD_WEAPON_RAILGUN = 11,
    MOD_SHIELD = 12,
    MOD_CARGO = 13,
    MOD_MINING_BEAM = 14
};

typedef struct {
    const char* name;
    COLORREF color;
    int hp;
    int mass;
    int powerGen;
    int powerDraw;
    int heatGen;
    int heatDissip;
    int thrust;
    int dps;
    int costCredits;
    int costTitanium;
} ModuleDef;

static const ModuleDef g_moduleDefs[] = {
    { "Empty",          RGB(15, 20, 35),     0,   0,   0,   0,  0,  0,   0,   0,   0,   0 },
    { "Titanium Hull",  RGB(70, 90, 120),  120,  10,   0,   0,  0,  0,   0,   0,  50,  15 },
    { "Armored Plate",  RGB(110, 130, 160), 280,  25,   0,   0,  0,  0,   0,   0, 120,  35 },
    { "Command Bridge", RGB(0, 220, 255),  150,  12,   5,   5,  0,  0,   0,   0, 250,  20 },
    { "Fission Core",   RGB(255, 180, 0),  100,  30,  50,   0, 15,  0,   0,   0, 300,  40 },
    { "Fusion Tokamak", RGB(255, 90, 0),   180,  60, 140,   0, 35,  0,   0,   0, 750,  90 },
    { "Chem Thruster",  RGB(0, 180, 255),   80,  15,   0,  15,  8,  0,  90,   0, 150,  25 },
    { "Plasma Drive",   RGB(180, 50, 255), 140,  35,   0,  55, 22,  0, 260,   0, 500,  70 },
    { "Radiator Fin",   RGB(50, 200, 150),  60,  10,   0,   0,  0, 35,   0,   0, 100,  20 },
    { "Autocannon",     RGB(220, 70, 70),   90,  15,   0,  15,  5,  0,   0,  30, 200,  30 },
    { "Plasma Lance",   RGB(255, 120, 50), 120,  25,   0,  45, 18,  0,   0,  80, 450,  60 },
    { "Railgun",        RGB(240, 240, 60), 150,  40,   0,  80, 30,  0,   0, 160, 800, 110 },
    { "Deflector Gen",  RGB(100, 150, 255),110,  20,   0,  35, 10,  0,   0,   0, 350,  45 },
    { "Cargo Pod",      RGB(140, 120, 70), 100,  15,   0,   5,  0,  0,   0,   0, 100,  25 },
    { "Mining Laser",   RGB(0, 240, 180),  100,  20,   0,  25, 12,  0,   0,  20, 250,  35 }
};

#define MOD_COUNT (sizeof(g_moduleDefs) / sizeof(g_moduleDefs[0]))

/* Yard Resources & Ship Stats */
typedef struct {
    int credits;
    int titanium;
    int ferrite;
    int deuterium;
    int reputation;
} YardEconomy;

typedef struct {
    int totalHp;
    int maxShields;
    int currentShields;
    int mass;
    int powerGen;
    int powerDraw;
    int heatGen;
    int heatDissip;
    int totalThrust;
    int dps;
    int cargoCap;
    int twr10; /* TWR * 10 */
} ShipStats;

/* Flight Simulation Entities */
typedef struct {
    float x, y;
    float vx, vy;
    float angle;
    float heat;
    int currentHp;
    int currentShield;
} PlayerFlight;

#define MAX_ASTEROIDS 16
typedef struct {
    float x, y;
    float vx, vy;
    float radius;
    int hp;
    int mineralType;
    int active;
} Asteroid;

#define MAX_PROJECTILES 32
typedef struct {
    float x, y;
    float vx, vy;
    int damage;
    int isEnemy;
    int life;
    int active;
} Projectile;

#define MAX_DRONES 6
typedef struct {
    float x, y;
    float vx, vy;
    float angle;
    int hp;
    int active;
    int shootTimer;
} EnemyDrone;

/* Faction Contracts */
typedef struct {
    const char* title;
    const char* client;
    int reqHp;
    int reqThrust;
    int reqDps;
    int reqCargo;
    int rewardCredits;
    int rewardRep;
    int completed;
} Contract;

static Contract g_contracts[] = {
    { "Terran System Patrol",    "Terran Star Navy",    400, 180,  60,   0, 1200, 25, 0 },
    { "Asteroid Ore Surveyor",   "Solaris Mining Corp", 200,  90,   0, 100,  850, 15, 0 },
    { "Heavy Strike Corvette",   "Orion Coalition",     650, 350, 140,   0, 2400, 40, 0 },
    { "Deep-Space Hauler Mk II", "Free Traders Guild",  300, 150,  20, 200, 1600, 30, 0 }
};

#define CONTRACT_COUNT (sizeof(g_contracts) / sizeof(g_contracts[0]))

/* Master Game State */
typedef struct {
    int state;
    int selectedModule;
    int symmetryMode;
    int grid[GRID_SZ][GRID_SZ];
    YardEconomy economy;
    ShipStats stats;
    PlayerFlight flight;
    Asteroid asteroids[MAX_ASTEROIDS];
    Projectile projectiles[MAX_PROJECTILES];
    EnemyDrone drones[MAX_DRONES];
    int tutorialStep;
    int tutorialSeen;
    int drydockProgress; /* 0 - 100 */
    char toast[64];
    int toastTimer;
} GameContext;

static GameContext g_game;
static HWND g_hwnd = NULL;

/* Forward Declarations */
static void RecalculateStats();
static void ResetShipGrid();
static void InitFlightSim();
static void StepFlightSim();
static void SetToast(const char* msg);

static void SetToast(const char* msg) {
    int i = 0;
    while (msg[i] && i < 63) {
        g_game.toast[i] = msg[i];
        i++;
    }
    g_game.toast[i] = '\0';
    g_game.toastTimer = 50;
}

static void ResetShipGrid() {
    for (int y = 0; y < GRID_SZ; y++) {
        for (int x = 0; x < GRID_SZ; x++) {
            g_game.grid[y][x] = MOD_EMPTY;
        }
    }
    /* Default Vanguard Corvette layout */
    int mid = GRID_SZ / 2;
    g_game.grid[2][mid - 1] = MOD_BRIDGE;
    g_game.grid[2][mid] = MOD_BRIDGE;
    g_game.grid[3][mid - 1] = MOD_HULL_ARMOR;
    g_game.grid[3][mid] = MOD_HULL_ARMOR;
    g_game.grid[4][mid - 2] = MOD_WEAPON_CANNON;
    g_game.grid[4][mid - 1] = MOD_HULL_BASIC;
    g_game.grid[4][mid] = MOD_HULL_BASIC;
    g_game.grid[4][mid + 1] = MOD_WEAPON_CANNON;
    g_game.grid[5][mid - 2] = MOD_RADIATOR;
    g_game.grid[5][mid - 1] = MOD_REACTOR_FISSION;
    g_game.grid[5][mid] = MOD_REACTOR_FISSION;
    g_game.grid[5][mid + 1] = MOD_RADIATOR;
    g_game.grid[6][mid - 1] = MOD_SHIELD;
    g_game.grid[6][mid] = MOD_CARGO;
    g_game.grid[7][mid - 2] = MOD_THRUSTER_CHEM;
    g_game.grid[7][mid - 1] = MOD_THRUSTER_PLASMA;
    g_game.grid[7][mid] = MOD_THRUSTER_PLASMA;
    g_game.grid[7][mid + 1] = MOD_THRUSTER_CHEM;
    RecalculateStats();
}

static void RecalculateStats() {
    ShipStats* s = &g_game.stats;
    s->totalHp = 0;
    s->maxShields = 0;
    s->mass = 20; /* base hull frame */
    s->powerGen = 0;
    s->powerDraw = 0;
    s->heatGen = 0;
    s->heatDissip = 10; /* passive radiant */
    s->totalThrust = 0;
    s->dps = 0;
    s->cargoCap = 0;

    for (int y = 0; y < GRID_SZ; y++) {
        for (int x = 0; x < GRID_SZ; x++) {
            int m = g_game.grid[y][x];
            if (m > 0 && m < (int)MOD_COUNT) {
                const ModuleDef* def = &g_moduleDefs[m];
                s->totalHp += def->hp;
                s->mass += def->mass;
                s->powerGen += def->powerGen;
                s->powerDraw += def->powerDraw;
                s->heatGen += def->heatGen;
                s->heatDissip += def->heatDissip;
                s->totalThrust += def->thrust;
                s->dps += def->dps;
                if (m == MOD_SHIELD) s->maxShields += 180;
                if (m == MOD_CARGO) s->cargoCap += 80;
            }
        }
    }

    if (s->mass > 0) {
        s->twr10 = (s->totalThrust * 10) / s->mass;
    } else {
        s->twr10 = 0;
    }
}

static void InitGame() {
    g_game.state = STATE_SPLASH;
    g_game.selectedModule = MOD_HULL_BASIC;
    g_game.symmetryMode = 1;
    g_game.economy.credits = 3500;
    g_game.economy.titanium = 240;
    g_game.economy.ferrite = 120;
    g_game.economy.deuterium = 80;
    g_game.economy.reputation = 10;
    g_game.tutorialStep = 0;
    g_game.tutorialSeen = 0;
    g_game.drydockProgress = 0;
    g_game.toastTimer = 0;

    ResetShipGrid();
}

static void InitFlightSim() {
    PlayerFlight* p = &g_game.flight;
    p->x = 420.0f;
    p->y = 350.0f;
    p->vx = 0.0f;
    p->vy = 0.0f;
    p->angle = -1.5707f; /* Face North */
    p->heat = 0.0f;
    p->currentHp = g_game.stats.totalHp > 0 ? g_game.stats.totalHp : 100;
    p->currentShield = g_game.stats.maxShields;

    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        Asteroid* a = &g_game.asteroids[i];
        a->active = 1;
        a->x = (float)(80 + (MyRand() % 680));
        a->y = (float)(80 + (MyRand() % 460));
        a->vx = ((MyRand() % 20) - 10) * 0.05f;
        a->vy = ((MyRand() % 20) - 10) * 0.05f;
        a->radius = (float)(14 + (MyRand() % 16));
        a->hp = (int)a->radius * 3;
        a->mineralType = (MyRand() % 3);
    }

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        g_game.projectiles[i].active = 0;
    }

    for (int i = 0; i < MAX_DRONES; i++) {
        EnemyDrone* d = &g_game.drones[i];
        d->active = (i < 3) ? 1 : 0;
        d->x = (float)(100 + (MyRand() % 640));
        d->y = (float)(100 + (MyRand() % 200));
        d->vx = 0.0f;
        d->vy = 0.0f;
        d->angle = 0.0f;
        d->hp = 80;
        d->shootTimer = 30 + MyRand() % 60;
    }
}

static void StepFlightSim() {
    PlayerFlight* p = &g_game.flight;

    p->x += p->vx;
    p->y += p->vy;
    p->vx *= 0.985f;
    p->vy *= 0.985f;

    /* Cooling */
    float coolRate = (float)g_game.stats.heatDissip * 0.02f;
    if (coolRate < 0.1f) coolRate = 0.1f;
    p->heat -= coolRate;
    if (p->heat < 0.0f) p->heat = 0.0f;

    /* Shield Recharge if power positive */
    if (g_game.stats.powerGen >= g_game.stats.powerDraw && p->currentShield < g_game.stats.maxShields) {
        p->currentShield++;
    }

    /* Screen boundaries */
    if (p->x < 30.0f) { p->x = 30.0f; p->vx = -p->vx * 0.5f; }
    if (p->x > 810.0f) { p->x = 810.0f; p->vx = -p->vx * 0.5f; }
    if (p->y < 60.0f) { p->y = 60.0f; p->vy = -p->vy * 0.5f; }
    if (p->y > 540.0f) { p->y = 540.0f; p->vy = -p->vy * 0.5f; }

    /* Asteroids */
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        Asteroid* a = &g_game.asteroids[i];
        if (!a->active) continue;
        a->x += a->vx;
        a->y += a->vy;
        if (a->x < 20.0f || a->x > 820.0f) a->vx = -a->vx;
        if (a->y < 50.0f || a->y > 550.0f) a->vy = -a->vy;
    }

    /* Projectiles */
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        Projectile* pr = &g_game.projectiles[i];
        if (!pr->active) continue;
        pr->x += pr->vx;
        pr->y += pr->vy;
        pr->life--;
        if (pr->life <= 0 || pr->x < 0 || pr->x > 850 || pr->y < 0 || pr->y > 600) {
            pr->active = 0;
            continue;
        }

        if (!pr->isEnemy) {
            /* Check hit asteroids */
            for (int j = 0; j < MAX_ASTEROIDS; j++) {
                Asteroid* a = &g_game.asteroids[j];
                if (!a->active) continue;
                float dx = pr->x - a->x;
                float dy = pr->y - a->y;
                if (dx * dx + dy * dy < a->radius * a->radius) {
                    pr->active = 0;
                    a->hp -= pr->damage;
                    if (a->hp <= 0) {
                        a->active = 0;
                        if (a->mineralType == 0) g_game.economy.titanium += 15;
                        else if (a->mineralType == 1) g_game.economy.ferrite += 10;
                        else g_game.economy.deuterium += 8;
                        SetToast("ASTEROID HARVESTED! +ORE");
                        PlaySfx(6);
                    }
                    break;
                }
            }

            /* Check hit enemy drones */
            for (int k = 0; k < MAX_DRONES; k++) {
                EnemyDrone* d = &g_game.drones[k];
                if (!d->active) continue;
                float dx = pr->x - d->x;
                float dy = pr->y - d->y;
                if (dx * dx + dy * dy < 400.0f) {
                    pr->active = 0;
                    d->hp -= pr->damage;
                    if (d->hp <= 0) {
                        d->active = 0;
                        g_game.economy.credits += 120;
                        g_game.economy.reputation += 2;
                        SetToast("SYNDICATE RAIDER NEUTRALIZED! +120$C");
                        PlaySfx(9);
                    }
                    break;
                }
            }
        } else {
            /* Check hit player */
            float dx = pr->x - p->x;
            float dy = pr->y - p->y;
            if (dx * dx + dy * dy < 400.0f) {
                pr->active = 0;
                if (p->currentShield > 0) {
                    p->currentShield -= pr->damage;
                    if (p->currentShield < 0) {
                        p->currentHp += p->currentShield;
                        p->currentShield = 0;
                    }
                } else {
                    p->currentHp -= pr->damage;
                }
                PlaySfx(10);
                if (p->currentHp <= 0) {
                    p->currentHp = 0;
                    SetToast("HULL BREACH! EMERGENCY ORBITAL EVAC");
                    g_game.state = STATE_DRYDOCK;
                }
            }
        }
    }

    /* Drones AI */
    for (int k = 0; k < MAX_DRONES; k++) {
        EnemyDrone* d = &g_game.drones[k];
        if (!d->active) continue;

        float dx = p->x - d->x;
        float dy = p->y - d->y;
        float distSq = dx * dx + dy * dy;

        if (distSq > 100.0f) {
            d->vx += (dx > 0 ? 0.08f : -0.08f);
            d->vy += (dy > 0 ? 0.08f : -0.08f);
        }
        d->x += d->vx;
        d->y += d->vy;
        d->vx *= 0.96f;
        d->vy *= 0.96f;

        d->shootTimer--;
        if (d->shootTimer <= 0 && distSq < 90000.0f) {
            d->shootTimer = 45 + MyRand() % 40;
            for (int i = 0; i < MAX_PROJECTILES; i++) {
                if (!g_game.projectiles[i].active) {
                    Projectile* pr = &g_game.projectiles[i];
                    pr->active = 1;
                    pr->isEnemy = 1;
                    pr->x = d->x;
                    pr->y = d->y;
                    float mag = 5.0f;
                    pr->vx = (dx > 0 ? 1.0f : -1.0f) * mag;
                    pr->vy = (dy > 0 ? 1.0f : -1.0f) * mag;
                    pr->damage = 15;
                    pr->life = 70;
                    break;
                }
            }
        }
    }
}

/* Save / Load */
static void SaveGame() {
    HANDLE hFile = CreateFileA("kstarforge.sav", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten;
        WriteFile(hFile, &g_game, sizeof(GameContext), &bytesWritten, NULL);
        CloseHandle(hFile);
        SetToast("QUICKSAVE COMPLETE [F5]");
        PlaySfx(7);
    }
}

static void LoadGame() {
    HANDLE hFile = CreateFileA("kstarforge.sav", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesRead;
        ReadFile(hFile, &g_game, sizeof(GameContext), &bytesRead, NULL);
        CloseHandle(hFile);
        RecalculateStats();
        SetToast("QUICKSAVE LOADED [F9]");
        PlaySfx(8);
    } else {
        SetToast("NO SAVE FILE FOUND");
    }
}

/* Rendering */
static void DrawGame(HDC hdc, RECT* rc) {
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBM = CreateCompatibleBitmap(hdc, rc->right, rc->bottom);
    HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

    /* Background Void */
    HBRUSH bgBrush = CreateSolidBrush(RGB(8, 12, 22));
    FillRect(memDC, rc, bgBrush);
    DeleteObject(bgBrush);

    /* Header Bar */
    RECT hdrRc = { 0, 0, rc->right, 42 };
    HBRUSH hdrBrush = CreateSolidBrush(RGB(14, 22, 38));
    FillRect(memDC, &hdrRc, hdrBrush);
    DeleteObject(hdrBrush);

    HPEN linePen = CreatePen(PS_SOLID, 1, RGB(35, 55, 90));
    HPEN oldPen = (HPEN)SelectObject(memDC, linePen);
    MoveToEx(memDC, 0, 42, NULL);
    LineTo(memDC, rc->right, 42);

    SetBkMode(memDC, TRANSPARENT);
    SetTextColor(memDC, RGB(0, 220, 255));
    TextOutA(memDC, 15, 12, "STARFORGE IX // SHIPYARD SIM v1.0.0", 35);

    char ecoBuf[128];
    wsprintfA(ecoBuf, "CREDITS: %d$C | TITANIUM: %d | FERRITE: %d | REP: %d",
        g_game.economy.credits, g_game.economy.titanium, g_game.economy.ferrite, g_game.economy.reputation);
    SetTextColor(memDC, RGB(220, 220, 240));
    TextOutA(memDC, 340, 12, ecoBuf, lstrlenA(ecoBuf));

    /* Nav Buttons */
    int btnX = 15;
    const char* tabs[] = { "[1] TITLE", "[2] BLUEPRINT", "[3] DRYDOCK", "[4] FLIGHT TEST", "[5] CONTRACTS", "[6] MANUAL [F1]" };
    for (int t = 0; t < 6; t++) {
        RECT tabRc = { btnX, 48, btnX + 115, 72 };
        int active = 0;
        if (t == 0 && g_game.state == STATE_SPLASH) active = 1;
        if (t == 1 && g_game.state == STATE_BLUEPRINT) active = 1;
        if (t == 2 && g_game.state == STATE_DRYDOCK) active = 1;
        if (t == 3 && g_game.state == STATE_FLIGHT) active = 1;
        if (t == 4 && g_game.state == STATE_CONTRACTS) active = 1;
        if (t == 5 && g_game.state == STATE_HELP) active = 1;

        HBRUSH tBrush = CreateSolidBrush(active ? RGB(30, 80, 140) : RGB(18, 28, 48));
        FillRect(memDC, &tabRc, tBrush);
        DeleteObject(tBrush);

        SetTextColor(memDC, active ? RGB(255, 255, 255) : RGB(140, 160, 190));
        DrawTextA(memDC, tabs[t], -1, &tabRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        btnX += 122;
    }

    if (g_game.state == STATE_SPLASH) {
        /* Title Screen */
        SetTextColor(memDC, RGB(0, 240, 255));
        TextOutA(memDC, 260, 160, "K S T A R F O R G E", 19);
        SetTextColor(memDC, RGB(140, 180, 220));
        TextOutA(memDC, 235, 190, "DEEP-SPACE SHIPYARD ENGINEERING SIM", 35);
        SetTextColor(memDC, RGB(180, 200, 220));
        TextOutA(memDC, 240, 250, "[ENTER / CLICK] Start New Yard Career", 37);
        TextOutA(memDC, 240, 285, "[C] Continue Saved Simulation", 29);
        TextOutA(memDC, 240, 320, "[F] Shakedown Orbital Flight Range", 34);
        TextOutA(memDC, 240, 355, "[H] Engineering Operations Manual", 33);
        TextOutA(memDC, 240, 390, "[F5] Quicksave | [F9] Quickload", 31);
        SetTextColor(memDC, RGB(100, 130, 160));
        TextOutA(memDC, 210, 480, "LUDONARRATIVE FLEET DIRECTIVE // ORBITAL MATRIX READY", 53);
    }
    else if (g_game.state == STATE_BLUEPRINT) {
        /* Blueprint Editor */
        int gx0 = 20;
        int gy0 = 85;
        int cSize = 34;

        /* Grid */
        for (int y = 0; y < GRID_SZ; y++) {
            for (int x = 0; x < GRID_SZ; x++) {
                int bx = gx0 + x * cSize;
                int by = gy0 + y * cSize;
                RECT bRc = { bx, by, bx + cSize - 1, by + cSize - 1 };
                int m = g_game.grid[y][x];

                HBRUSH bBrush = CreateSolidBrush(g_moduleDefs[m].color);
                FillRect(memDC, &bRc, bBrush);
                DeleteObject(bBrush);

                if (m == MOD_EMPTY) {
                    /* Grid lines */
                    SetPixel(memDC, bx, by, RGB(30, 45, 75));
                } else {
                    /* Letter indicator */
                    char ch = g_moduleDefs[m].name[0];
                    SetTextColor(memDC, RGB(255, 255, 255));
                    TextOutA(memDC, bx + 12, by + 9, &ch, 1);
                }
            }
        }

        /* Palette / Module Selector */
        int px = 450;
        int py = 85;
        SetTextColor(memDC, RGB(0, 220, 255));
        TextOutA(memDC, px, py, "ENGINEERING MODULE PALETTE (CLICK TO SELECT)", 44);

        for (int m = 1; m < (int)MOD_COUNT; m++) {
            int mx = px + ((m - 1) % 2) * 190;
            int my = py + 25 + ((m - 1) / 2) * 38;
            RECT pRc = { mx, my, mx + 180, my + 32 };
            int isSel = (g_game.selectedModule == m);

            HBRUSH pBrush = CreateSolidBrush(isSel ? RGB(40, 100, 160) : RGB(20, 30, 50));
            FillRect(memDC, &pRc, pBrush);
            DeleteObject(pBrush);

            RECT iconRc = { mx + 4, my + 4, mx + 28, my + 28 };
            HBRUSH iBrush = CreateSolidBrush(g_moduleDefs[m].color);
            FillRect(memDC, &iconRc, iBrush);
            DeleteObject(iBrush);

            SetTextColor(memDC, isSel ? RGB(255, 255, 255) : RGB(180, 200, 220));
            TextOutA(memDC, mx + 34, my + 8, g_moduleDefs[m].name, lstrlenA(g_moduleDefs[m].name));
        }

        /* Telemetry Panel */
        int ty = 380;
        RECT telRc = { px, ty, rc->right - 20, rc->bottom - 20 };
        HBRUSH telBrush = CreateSolidBrush(RGB(14, 20, 35));
        FillRect(memDC, &telRc, telBrush);
        DeleteObject(telBrush);

        SetTextColor(memDC, RGB(0, 240, 220));
        TextOutA(memDC, px + 15, ty + 10, "VESSEL ENGINEERING TELEMETRY", 28);

        char tBuf[128];
        wsprintfA(tBuf, "HULL INTEGRITY: %d HP | SHIELD: %d", g_game.stats.totalHp, g_game.stats.maxShields);
        SetTextColor(memDC, RGB(220, 220, 220));
        TextOutA(memDC, px + 15, ty + 35, tBuf, lstrlenA(tBuf));

        wsprintfA(tBuf, "TOTAL MASS: %d TONS | THRUST: %d kN | TWR: %d.%d",
            g_game.stats.mass, g_game.stats.totalThrust, g_game.stats.twr10 / 10, g_game.stats.twr10 % 10);
        TextOutA(memDC, px + 15, ty + 55, tBuf, lstrlenA(tBuf));

        int pNet = g_game.stats.powerGen - g_game.stats.powerDraw;
        wsprintfA(tBuf, "POWER GRID: +%d MW / -%d MW (NET: %s%d MW)",
            g_game.stats.powerGen, g_game.stats.powerDraw, pNet >= 0 ? "+" : "", pNet);
        SetTextColor(memDC, pNet >= 0 ? RGB(50, 240, 100) : RGB(255, 70, 70));
        TextOutA(memDC, px + 15, ty + 75, tBuf, lstrlenA(tBuf));

        int hNet = g_game.stats.heatGen - g_game.stats.heatDissip;
        wsprintfA(tBuf, "HEAT BUDGET: %d GEN / %d DISSIPATION (%s%d)",
            g_game.stats.heatGen, g_game.stats.heatDissip, hNet <= 0 ? "STABLE " : "OVERHEAT +", hNet);
        SetTextColor(memDC, hNet <= 0 ? RGB(50, 240, 100) : RGB(255, 120, 0));
        TextOutA(memDC, px + 15, ty + 95, tBuf, lstrlenA(tBuf));

        wsprintfA(tBuf, "FIREPOWER: %d DPS | CARGO CAPACITY: %d TONS", g_game.stats.dps, g_game.stats.cargoCap);
        SetTextColor(memDC, RGB(220, 220, 220));
        TextOutA(memDC, px + 15, ty + 115, tBuf, lstrlenA(tBuf));

        SetTextColor(memDC, RGB(120, 160, 200));
        TextOutA(memDC, px + 15, ty + 140, "[S] Symmetry: ON | [R] Reset | [SPACE] To Drydock", 49);
    }
    else if (g_game.state == STATE_DRYDOCK) {
        /* Drydock Assembly */
        SetTextColor(memDC, RGB(0, 220, 255));
        TextOutA(memDC, 60, 120, "ORBITAL DRYDOCK GANTRY // ASSEMBLY MATRIX", 41);

        /* Scaffolding GDI drawing */
        SelectObject(memDC, linePen);
        MoveToEx(memDC, 100, 180, NULL); LineTo(memDC, 740, 180);
        MoveToEx(memDC, 100, 380, NULL); LineTo(memDC, 740, 380);
        for (int x = 120; x <= 720; x += 60) {
            MoveToEx(memDC, x, 180, NULL); LineTo(memDC, x, 380);
        }

        /* Assembly progress bar */
        RECT pBar = { 150, 420, 690, 450 };
        HBRUSH pbBg = CreateSolidBrush(RGB(20, 30, 50));
        FillRect(memDC, &pBar, pbBg);
        DeleteObject(pbBg);

        int filledW = (540 * g_game.drydockProgress) / 100;
        RECT pFill = { 150, 420, 150 + filledW, 450 };
        HBRUSH pfBrush = CreateSolidBrush(RGB(0, 200, 255));
        FillRect(memDC, &pFill, pfBrush);
        DeleteObject(pfBrush);

        char pBuf[64];
        wsprintfA(pBuf, "CONSTRUCTION PROGRESS: %d%%", g_game.drydockProgress);
        SetTextColor(memDC, RGB(240, 240, 255));
        TextOutA(memDC, 330, 428, pBuf, lstrlenA(pBuf));

        SetTextColor(memDC, RGB(200, 220, 255));
        TextOutA(memDC, 250, 480, "[SPACE] Complete Fabrication & Launch to Proving Grounds", 56);
        TextOutA(memDC, 310, 510, "[ESC] Return to Blueprint Bay", 29);
    }
    else if (g_game.state == STATE_FLIGHT) {
        /* Shakedown Flight Sim */
        PlayerFlight* p = &g_game.flight;

        /* Starfield background */
        for (int i = 0; i < 40; i++) {
            int sx = (i * 73 + (int)p->x / 4) % (rc->right - 20) + 10;
            int sy = (i * 107 + (int)p->y / 4) % (rc->bottom - 80) + 70;
            SetPixel(memDC, sx, sy, RGB(180, 200, 255));
        }

        /* Asteroids */
        for (int i = 0; i < MAX_ASTEROIDS; i++) {
            Asteroid* a = &g_game.asteroids[i];
            if (!a->active) continue;
            HBRUSH astBrush = CreateSolidBrush(a->mineralType == 0 ? RGB(100, 110, 130) : (a->mineralType == 1 ? RGB(140, 90, 60) : RGB(60, 120, 160)));
            Ellipse(memDC, (int)(a->x - a->radius), (int)(a->y - a->radius), (int)(a->x + a->radius), (int)(a->y + a->radius));
            DeleteObject(astBrush);
        }

        /* Projectiles */
        for (int i = 0; i < MAX_PROJECTILES; i++) {
            Projectile* pr = &g_game.projectiles[i];
            if (!pr->active) continue;
            HPEN prPen = CreatePen(PS_SOLID, 2, pr->isEnemy ? RGB(255, 60, 60) : RGB(80, 240, 255));
            HPEN oP = (HPEN)SelectObject(memDC, prPen);
            MoveToEx(memDC, (int)pr->x, (int)pr->y, NULL);
            LineTo(memDC, (int)(pr->x - pr->vx * 1.5f), (int)(pr->y - pr->vy * 1.5f));
            SelectObject(memDC, oP);
            DeleteObject(prPen);
        }

        /* Drones */
        for (int k = 0; k < MAX_DRONES; k++) {
            EnemyDrone* d = &g_game.drones[k];
            if (!d->active) continue;
            HBRUSH drBrush = CreateSolidBrush(RGB(220, 40, 40));
            HBRUSH oB = (HBRUSH)SelectObject(memDC, drBrush);
            Rectangle(memDC, (int)d->x - 8, (int)d->y - 8, (int)d->x + 8, (int)d->y + 8);
            SelectObject(memDC, oB);
            DeleteObject(drBrush);
        }

        /* Player Ship Vector */
        HBRUSH pBrush = CreateSolidBrush(RGB(0, 200, 255));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, pBrush);
        POINT pts[3];
        float sa = 14.0f;
        pts[0].x = (int)(p->x + sa * 1.5f * -p->vy * 0.15f); /* Simple direction representation */
        pts[0].y = (int)(p->y - sa * 1.5f);
        pts[1].x = (int)(p->x - sa);
        pts[1].y = (int)(p->y + sa);
        pts[2].x = (int)(p->x + sa);
        pts[2].y = (int)(p->y + sa);
        Polygon(memDC, pts, 3);
        SelectObject(memDC, oB);
        DeleteObject(pBrush);

        /* HUD Readout */
        char hBuf[128];
        wsprintfA(hBuf, "HULL: %d HP | SHIELD: %d | HEAT: %d%%", p->currentHp, p->currentShield, (int)p->heat);
        SetTextColor(memDC, RGB(0, 240, 255));
        TextOutA(memDC, 20, 80, hBuf, lstrlenA(hBuf));

        SetTextColor(memDC, RGB(180, 200, 220));
        TextOutA(memDC, 20, rc->bottom - 30, "[W/S] Thrust/Retro | [A/D] Steering | [SPACE] Fire | [E] Mining Beam | [ESC] Dock", 81);
    }
    else if (g_game.state == STATE_CONTRACTS) {
        /* Faction Contracts */
        SetTextColor(memDC, RGB(0, 240, 255));
        TextOutA(memDC, 40, 95, "FACTION FLEET COMMISSION BOARD", 30);

        int cy = 130;
        for (int i = 0; i < (int)CONTRACT_COUNT; i++) {
            Contract* c = &g_contracts[i];
            RECT cRc = { 40, cy, rc->right - 40, cy + 70 };
            HBRUSH cBrush = CreateSolidBrush(c->completed ? RGB(20, 60, 35) : RGB(16, 25, 45));
            FillRect(memDC, &cRc, cBrush);
            DeleteObject(cBrush);

            SetTextColor(memDC, RGB(255, 255, 255));
            TextOutA(memDC, 55, cy + 10, c->title, lstrlenA(c->title));

            char clBuf[128];
            wsprintfA(clBuf, "Client: %s | Reward: %d$C, +%d Rep", c->client, c->rewardCredits, c->rewardRep);
            SetTextColor(memDC, RGB(180, 200, 220));
            TextOutA(memDC, 55, cy + 30, clBuf, lstrlenA(clBuf));

            char reqBuf[128];
            int meets = (g_game.stats.totalHp >= c->reqHp &&
                         g_game.stats.totalThrust >= c->reqThrust &&
                         g_game.stats.dps >= c->reqDps &&
                         g_game.stats.cargoCap >= c->reqCargo);
            wsprintfA(reqBuf, "Reqs: HP >= %d, Thrust >= %d, DPS >= %d, Cargo >= %d (%s)",
                c->reqHp, c->reqThrust, c->reqDps, c->reqCargo, meets ? "COMPLIANT!" : "NON-COMPLIANT");
            SetTextColor(memDC, meets ? RGB(50, 255, 120) : RGB(255, 100, 100));
            TextOutA(memDC, 55, cy + 48, reqBuf, lstrlenA(reqBuf));

            if (!c->completed && meets) {
                RECT bDeliver = { rc->right - 180, cy + 18, rc->right - 60, cy + 52 };
                HBRUSH dBrush = CreateSolidBrush(RGB(40, 140, 70));
                FillRect(memDC, &bDeliver, dBrush);
                DeleteObject(dBrush);
                SetTextColor(memDC, RGB(255, 255, 255));
                TextOutA(memDC, rc->right - 165, cy + 26, "[DELIVER SHIP]", 14);
            } else if (c->completed) {
                SetTextColor(memDC, RGB(80, 255, 140));
                TextOutA(memDC, rc->right - 165, cy + 26, "FULFILLED", 9);
            }

            cy += 85;
        }
    }
    else if (g_game.state == STATE_HELP) {
        /* Manual */
        SetTextColor(memDC, RGB(0, 240, 255));
        TextOutA(memDC, 40, 95, "STARFORGE IX OPERATIONS & ENGINEERING MANUAL", 44);

        const char* manualLines[] = {
            "1. BLUEPRINT BAY: Click modules from palette, then click grid to install.",
            "   Right-click to erase. Press [S] to toggle horizontal symmetry placement.",
            "2. POWER & HEAT: Every system draws power. Maintain positive Net Power (MW)",
            "   and install sufficient Radiator Fins to dissipate thermal waste.",
            "3. THRUST & TWR: Install Chemical or Plasma Drives to achieve high TWR.",
            "4. DRYDOCK: Gantry fabricates your design layer by layer.",
            "5. SHAKEDOWN PROVING RANGE: Pilot your ship in live space! Mine ore and fight pirates.",
            "6. CONTRACTS: Fulfill faction commissions to earn credits and fleet reputation.",
            "7. SHORTCUTS: [F1/H] Manual, [F5] Quicksave, [F9] Quickload, [1-6] Tabs, [S] Symmetry, [ESC] Back."
        };
        int my = 135;
        SetTextColor(memDC, RGB(200, 220, 240));
        for (int i = 0; i < 9; i++) {
            TextOutA(memDC, 50, my, manualLines[i], lstrlenA(manualLines[i]));
            my += 28;
        }
    }

    /* Persistent Usability Status Bar */
    RECT statRc = { 0, rc->bottom - 22, rc->right, rc->bottom };
    HBRUSH statBrush = CreateSolidBrush(RGB(12, 18, 30));
    FillRect(memDC, &statRc, statBrush);
    DeleteObject(statBrush);
    SetTextColor(memDC, RGB(100, 140, 180));
    TextOutA(memDC, 12, rc->bottom - 18, "[F1/H] Manual  [1-6] Tabs  [S] Symmetry  [R] Reset  [F5] Save  [F9] Load  [LMB] Place  [RMB] Erase", 94);

    /* Onscreen Toast */
    if (g_game.toastTimer > 0) {
        g_game.toastTimer--;
        RECT tRc = { rc->right / 2 - 180, rc->bottom - 55, rc->right / 2 + 180, rc->bottom - 25 };
        HBRUSH tBrush = CreateSolidBrush(RGB(0, 180, 220));
        FillRect(memDC, &tRc, tBrush);
        DeleteObject(tBrush);
        SetTextColor(memDC, RGB(0, 15, 30));
        DrawTextA(memDC, g_game.toast, -1, &tRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    /* BitBlt to primary DC */
    BitBlt(hdc, 0, 0, rc->right, rc->bottom, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldPen);
    SelectObject(memDC, oldBM);
    DeleteObject(linePen);
    DeleteObject(memBM);
    DeleteDC(memDC);
}

/* Window Procedure */
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            InitGame();
            SetTimer(hwnd, 1, 33, NULL); /* ~30 FPS */
            return 0;
        }
        case WM_TIMER: {
            if (g_game.state == STATE_DRYDOCK) {
                if (g_game.drydockProgress < 100) {
                    g_game.drydockProgress += 2;
                }
            } else if (g_game.state == STATE_FLIGHT) {
                StepFlightSim();
            }
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case WM_KEYDOWN: {
            if (wParam == VK_F5) {
                SaveGame();
            } else if (wParam == VK_F9) {
                LoadGame();
            } else if (wParam == '1') {
                g_game.state = STATE_SPLASH;
                PlaySfx(1);
            } else if (wParam == '2') {
                g_game.state = STATE_BLUEPRINT;
                PlaySfx(1);
            } else if (wParam == '3') {
                g_game.state = STATE_DRYDOCK;
                PlaySfx(1);
            } else if (wParam == '4') {
                g_game.state = STATE_FLIGHT;
                InitFlightSim();
                PlaySfx(1);
            } else if (wParam == '5') {
                g_game.state = STATE_CONTRACTS;
                PlaySfx(1);
            } else if (wParam == '6' || wParam == 'H' || wParam == VK_F1) {
                g_game.state = STATE_HELP;
                PlaySfx(1);
            } else if (wParam == VK_ESCAPE) {
                if (g_game.state != STATE_SPLASH) {
                    g_game.state = STATE_BLUEPRINT;
                    PlaySfx(1);
                }
            } else if (wParam == 'S' && g_game.state == STATE_BLUEPRINT) {
                g_game.symmetryMode = !g_game.symmetryMode;
                SetToast(g_game.symmetryMode ? "SYMMETRY: ON" : "SYMMETRY: OFF");
                PlaySfx(1);
            } else if (wParam == 'R' && g_game.state == STATE_BLUEPRINT) {
                ResetShipGrid();
                SetToast("SHIP BLUEPRINT RESET");
                PlaySfx(3);
            } else if (wParam == VK_SPACE) {
                if (g_game.state == STATE_SPLASH) {
                    g_game.state = STATE_BLUEPRINT;
                    PlaySfx(1);
                } else if (g_game.state == STATE_DRYDOCK) {
                    g_game.state = STATE_FLIGHT;
                    InitFlightSim();
                    PlaySfx(1);
                } else if (g_game.state == STATE_FLIGHT) {
                    /* Fire primaries */
                    PlayerFlight* p = &g_game.flight;
                    for (int i = 0; i < MAX_PROJECTILES; i++) {
                        if (!g_game.projectiles[i].active) {
                            Projectile* pr = &g_game.projectiles[i];
                            pr->active = 1;
                            pr->isEnemy = 0;
                            pr->x = p->x;
                            pr->y = p->y - 15.0f;
                            pr->vx = 0.0f;
                            pr->vy = -8.0f;
                            pr->damage = g_game.stats.dps > 0 ? (g_game.stats.dps / 2 + 10) : 25;
                            pr->life = 60;
                            p->heat += 3.0f;
                            PlaySfx(4);
                            break;
                        }
                    }
                }
            } else if (g_game.state == STATE_FLIGHT) {
                PlayerFlight* p = &g_game.flight;
                float thr = (float)g_game.stats.twr10 * 0.06f;
                if (thr < 0.4f) thr = 0.4f;

                if (wParam == 'W' || wParam == VK_UP) {
                    p->vy -= thr;
                } else if (wParam == 'S' || wParam == VK_DOWN) {
                    p->vy += thr * 0.6f;
                } else if (wParam == 'A' || wParam == VK_LEFT) {
                    p->vx -= thr * 0.7f;
                } else if (wParam == 'D' || wParam == VK_RIGHT) {
                    p->vx += thr * 0.7f;
                } else if (wParam == 'E') {
                    /* Mining beam */
                    p->heat += 5.0f;
                    PlaySfx(6);
                    for (int j = 0; j < MAX_ASTEROIDS; j++) {
                        Asteroid* a = &g_game.asteroids[j];
                        if (!a->active) continue;
                        float dx = p->x - a->x;
                        float dy = p->y - a->y;
                        if (dx * dx + dy * dy < 25000.0f) {
                            a->hp -= 20;
                            if (a->hp <= 0) {
                                a->active = 0;
                                g_game.economy.titanium += 20;
                                SetToast("MINING BEAM EXTRACTED ORE!");
                            }
                            break;
                        }
                    }
                }
            }
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);

            /* Tab Navigation */
            if (my >= 48 && my <= 72) {
                int t = (mx - 15) / 122;
                if (t >= 0 && t < 6) {
                    if (t == 0) g_game.state = STATE_SPLASH;
                    if (t == 1) g_game.state = STATE_BLUEPRINT;
                    if (t == 2) g_game.state = STATE_DRYDOCK;
                    if (t == 3) { g_game.state = STATE_FLIGHT; InitFlightSim(); }
                    if (t == 4) g_game.state = STATE_CONTRACTS;
                    if (t == 5) g_game.state = STATE_HELP;
                    PlaySfx(1);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            }

            if (g_game.state == STATE_SPLASH) {
                if (my >= 240 && my <= 270) {
                    g_game.state = STATE_BLUEPRINT;
                    PlaySfx(1);
                } else if (my >= 275 && my <= 305) {
                    LoadGame();
                    g_game.state = STATE_BLUEPRINT;
                } else if (my >= 310 && my <= 340) {
                    g_game.state = STATE_FLIGHT;
                    InitFlightSim();
                    PlaySfx(1);
                } else if (my >= 345 && my <= 375) {
                    g_game.state = STATE_HELP;
                    PlaySfx(1);
                }
            }
            else if (g_game.state == STATE_BLUEPRINT) {
                /* Click grid */
                int gx0 = 20;
                int gy0 = 85;
                int cSize = 34;
                if (mx >= gx0 && mx < gx0 + GRID_SZ * cSize && my >= gy0 && my < gy0 + GRID_SZ * cSize) {
                    int gx = (mx - gx0) / cSize;
                    int gy = (my - gy0) / cSize;
                    g_game.grid[gy][gx] = g_game.selectedModule;
                    if (g_game.symmetryMode) {
                        int symX = GRID_SZ - 1 - gx;
                        g_game.grid[gy][symX] = g_game.selectedModule;
                    }
                    RecalculateStats();
                    PlaySfx(2);
                }

                /* Click palette */
                int px = 450;
                int py = 85;
                for (int m = 1; m < (int)MOD_COUNT; m++) {
                    int bpx = px + ((m - 1) % 2) * 190;
                    int bpy = py + 25 + ((m - 1) / 2) * 38;
                    if (mx >= bpx && mx < bpx + 180 && my >= bpy && my < bpy + 32) {
                        g_game.selectedModule = m;
                        PlaySfx(1);
                        break;
                    }
                }
            }
            else if (g_game.state == STATE_CONTRACTS) {
                int cy = 130;
                RECT rc;
                GetClientRect(hwnd, &rc);
                for (int i = 0; i < (int)CONTRACT_COUNT; i++) {
                    Contract* c = &g_contracts[i];
                    int meets = (g_game.stats.totalHp >= c->reqHp &&
                                 g_game.stats.totalThrust >= c->reqThrust &&
                                 g_game.stats.dps >= c->reqDps &&
                                 g_game.stats.cargoCap >= c->reqCargo);
                    if (!c->completed && meets) {
                        int bx1 = rc.right - 180;
                        int bx2 = rc.right - 60;
                        if (mx >= bx1 && mx <= bx2 && my >= cy + 18 && my <= cy + 52) {
                            c->completed = 1;
                            g_game.economy.credits += c->rewardCredits;
                            g_game.economy.reputation += c->rewardRep;
                            SetToast("CONTRACT FULFILLED! REWARDS DELIVERED");
                            PlaySfx(9);
                            break;
                        }
                    }
                    cy += 85;
                }
            }

            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case WM_RBUTTONDOWN: {
            /* Right click to erase block in blueprint mode */
            if (g_game.state == STATE_BLUEPRINT) {
                int mx = LOWORD(lParam);
                int my = HIWORD(lParam);
                int gx0 = 20;
                int gy0 = 85;
                int cSize = 34;
                if (mx >= gx0 && mx < gx0 + GRID_SZ * cSize && my >= gy0 && my < gy0 + GRID_SZ * cSize) {
                    int gx = (mx - gx0) / cSize;
                    int gy = (my - gy0) / cSize;
                    g_game.grid[gy][gx] = MOD_EMPTY;
                    if (g_game.symmetryMode) {
                        int symX = GRID_SZ - 1 - gx;
                        g_game.grid[gy][symX] = MOD_EMPTY;
                    }
                    RecalculateStats();
                    PlaySfx(3);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);
            DrawGame(hdc, &rc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY: {
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandleA(NULL);
    const char CLASS_NAME[] = "KStarForgeWindowClass";

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);
    wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(1));
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    RegisterClassA(&wc);

    RECT wr = { 0, 0, 920, 620 };
    AdjustWindowRect(&wr, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME, "KStarForge - Orbital Shipyard Engineering Sim v1.0.0",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd) {
        g_hwnd = hwnd;
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        MSG msg = {0};
        while (GetMessageA(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    ExitProcess(0);
}
