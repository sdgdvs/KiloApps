#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TIMER_ID 1
#define TIMER_INTERVAL 30 // ms (~33 FPS)

#define ID_BTN_LASER       101
#define ID_BTN_TRACTOR     102
#define ID_BTN_DAMPENER    103
#define ID_BTN_SCAN        104
#define ID_BTN_NAV         105
#define ID_BTN_THEME       106
#define ID_BTN_SCANLINES   107
#define ID_BTN_AUDIO       108
#define ID_BTN_HELP        109
#define ID_BTN_JETTISON    110
#define ID_BTN_SELL        111

#define SFX_NONE        0
#define SFX_COLLECT     1
#define SFX_FRACTURE    2
#define SFX_OVERHEAT    3
#define SFX_LASER_PULSE 4
#define SFX_BEEP        5
#define SFX_WARP        6

#define MAX_STARS 150
#define MAX_ASTEROIDS 24
#define MAX_ORE_CHUNKS 64
#define MAX_PARTICLES 128
#define MAX_FLOATING_TEXTS 16
#define MAX_LOG_ENTRIES 30

typedef struct {
    float x, y;
    float size;
    float brightness;
} Star;

typedef struct {
    float a, r;
} PolyVert;

typedef struct {
    char id[16];
    float x, y;
    float vx, vy;
    float radius;
    PolyVert verts[12];
    int numVerts;
    int oreType; // 0=Ferrum, 1=Silicates, 2=Platinum, 3=Void Quartz, 4=Dark Geode, 5=Derelict Scrap
    int richness;
    float hp, maxHp;
    float rot, rotSpeed;
    int active;
} Asteroid;

typedef struct {
    float x, y;
    float vx, vy;
    int oreType;
    int amount;
    float life;
    float rot, rotSpeed;
    int active;
} OreChunk;

typedef struct {
    float x, y;
    float vx, vy;
    COLORREF color;
    float life, decay;
    float size;
    int active;
} Particle;

typedef struct {
    char text[32];
    float x, y;
    float vy;
    COLORREF color;
    float life;
    int active;
} FloatingText;

typedef struct {
    char text[128];
    int type; // 0=system, 1=mining, 2=cargo, 3=warning, 4=critical, 5=success, 6=warp
} LogEntry;

// Ore Definitions
typedef struct {
    const char* name;
    COLORREF color;
    int value;
} OreInfo;

static const OreInfo ORE_DEFS[6] = {
    { "Ferrum Ore",     RGB(148, 163, 184), 15 },
    { "Silicates",      RGB(96, 165, 250),  25 },
    { "Platinum Vein",  RGB(226, 232, 240), 75 },
    { "Void Quartz",    RGB(192, 132, 252), 140 },
    { "Dark Geode",     RGB(244, 63, 94),   300 },
    { "Derelict Scrap", RGB(251, 191, 36),  50 }
};

// Sector Star Chart Definitions
typedef struct {
    const char* name;
    const char* type;
    const char* hazard;
    COLORREF hazardColor;
    int fuelCost;
    int density;
    const char* desc;
    int oreWeights[6]; // Ferrum, Silicates, Platinum, Void Quartz, Dark Geode, Derelict Scrap
    COLORREF nebulaColor;
    COLORREF starColor;
} SectorDef;

static const SectorDef SECTOR_DEFS[4] = {
    {
        "Belt Alpha-09",
        "Dense Metallic Asteroid Belt",
        "LOW (★☆☆☆)",
        RGB(16, 185, 129),
        10,
        22,
        "Inner dense asteroid belt rich in industrial ferrum, silicates, and standard veins.",
        { 50, 30, 12, 8, 0, 0 },
        RGB(8, 16, 40),
        RGB(224, 242, 254)
    },
    {
        "Kuiper Ring",
        "Frozen Outer Asteroid Ring",
        "MODERATE (★★☆☆)",
        RGB(245, 158, 11),
        18,
        26,
        "Sub-zero icy peripheral belt containing dense platinum veins and crystallized silicates.",
        { 15, 30, 45, 10, 0, 0 },
        RGB(4, 30, 40),
        RGB(207, 250, 254)
    },
    {
        "Derelict Graveyard",
        "Warzone Salvage Wreckage",
        "HIGH (★★★☆)",
        RGB(219, 39, 119),
        25,
        24,
        "Shattered remnants of orbital fleet skirmishes. High concentration of scrap & quartz.",
        { 0, 0, 15, 25, 10, 50 },
        RGB(35, 18, 5),
        RGB(254, 243, 199)
    },
    {
        "Plasma Nebula",
        "Ionized Stellar Storm",
        "EXTREME (★★★★)",
        RGB(239, 68, 68),
        35,
        28,
        "Energetic stellar nursery saturated with volatile plasma discharges and Dark Matter.",
        { 0, 0, 15, 35, 40, 10 },
        RGB(32, 6, 32),
        RGB(250, 232, 255)
    }
};

// CRT Vector Theme Definitions
typedef struct {
    const char* name;
    COLORREF bgSpace;
    COLORREF bgPanel;
    COLORREF bgHeader;
    COLORREF borderPanel;
    COLORREF borderGlow;
    COLORREF textPrimary;
    COLORREF textBright;
    COLORREF textDim;
    COLORREF vector;
    COLORREF vectorDim;
    COLORREF laserGlow;
    COLORREF laserCore;
    COLORREF radarBg;
    COLORREF radarGrid;
    COLORREF radarSweep;
    COLORREF radarTarget;
    COLORREF starCol;
} ThemePalette;

static const ThemePalette THEME_PALETTES[4] = {
    // 0: Cyan (Default Mk-IV)
    {
        "CRT CYAN",
        RGB(1, 4, 10),      // bgSpace
        RGB(11, 19, 41),    // bgPanel
        RGB(15, 28, 63),    // bgHeader
        RGB(30, 58, 138),   // borderPanel
        RGB(56, 189, 248),  // borderGlow
        RGB(56, 189, 248),  // textPrimary
        RGB(240, 249, 255), // textBright
        RGB(2, 132, 199),   // textDim
        RGB(0, 240, 255),   // vector
        RGB(56, 189, 248),  // vectorDim
        RGB(0, 240, 255),   // laserGlow
        RGB(255, 255, 255), // laserCore
        RGB(3, 8, 22),      // radarBg
        RGB(30, 58, 138),   // radarGrid
        RGB(0, 240, 255),   // radarSweep
        RGB(0, 240, 255),   // radarTarget
        RGB(224, 242, 254)  // starCol
    },
    // 1: Amber (P3 Phosphor CRT)
    {
        "CRT AMBER",
        RGB(13, 8, 1),      // bgSpace
        RGB(26, 16, 2),     // bgPanel
        RGB(38, 23, 3),     // bgHeader
        RGB(120, 53, 15),   // borderPanel
        RGB(245, 158, 11),  // borderGlow
        RGB(251, 191, 36),  // textPrimary
        RGB(254, 243, 199), // textBright
        RGB(180, 83, 9),    // textDim
        RGB(245, 158, 11),  // vector
        RGB(251, 191, 36),  // vectorDim
        RGB(245, 158, 11),  // laserGlow
        RGB(255, 251, 235), // laserCore
        RGB(21, 12, 2),     // radarBg
        RGB(120, 53, 15),   // radarGrid
        RGB(245, 158, 11),  // radarSweep
        RGB(254, 240, 138), // radarTarget
        RGB(254, 243, 199)  // starCol
    },
    // 2: Green (P1 Phosphor Matrix CRT)
    {
        "CRT GREEN",
        RGB(1, 13, 4),      // bgSpace
        RGB(3, 28, 9),      // bgPanel
        RGB(6, 43, 16),     // bgHeader
        RGB(21, 128, 61),   // borderPanel
        RGB(34, 197, 94),   // borderGlow
        RGB(74, 222, 128),  // textPrimary
        RGB(220, 252, 231), // textBright
        RGB(22, 163, 74),   // textDim
        RGB(34, 197, 94),   // vector
        RGB(74, 222, 128),  // vectorDim
        RGB(34, 197, 94),   // laserGlow
        RGB(240, 253, 244), // laserCore
        RGB(2, 20, 7),      // radarBg
        RGB(21, 128, 61),   // radarGrid
        RGB(34, 197, 94),   // radarSweep
        RGB(134, 239, 172), // radarTarget
        RGB(220, 252, 231)  // starCol
    },
    // 3: Solar Crimson Hazard CRT
    {
        "SOLAR CRT",
        RGB(15, 3, 3),      // bgSpace
        RGB(32, 6, 6),      // bgPanel
        RGB(53, 10, 10),    // bgHeader
        RGB(153, 27, 27),   // borderPanel
        RGB(239, 68, 68),   // borderGlow
        RGB(248, 113, 113), // textPrimary
        RGB(254, 226, 226), // textBright
        RGB(185, 28, 28),   // textDim
        RGB(239, 68, 68),   // vector
        RGB(248, 113, 113), // vectorDim
        RGB(239, 68, 68),   // laserGlow
        RGB(254, 242, 242), // laserCore
        RGB(26, 4, 4),      // radarBg
        RGB(153, 27, 27),   // radarGrid
        RGB(239, 68, 68),   // radarSweep
        RGB(252, 165, 165), // radarTarget
        RGB(254, 226, 226)  // starCol
    }
};

// Game State
typedef struct {
    int credits;
    int currentSectorIndex;
    int selectedSectorIndex;
    int showStarChart;
    int warpActive;
    float warpTimer;
    char sector[32];
    float hull, maxHull;
    float shield, maxShield;
    float fuel, maxFuel;
    float heat, maxHeat;
    float reactor;
    float o2;
    int cargoHold[6];
    int totalCargo;
    int maxCargo;
    int dampeners;
    int laserOverheated;
    int themeIndex;
    int scanlineMode; // 0=Off, 1=Normal, 2=CRT+
    
    // Ship Navigation
    float shipX, shipY;
    float shipVx, shipVy;
    float shipAngle; // radians
    int miningActive;
    int tractorActive;
    int thrusting;
    int reversing;
    int turningLeft;
    int turningRight;
    
    int selectedAstIndex;
    float radarAngle;
    int soundEnabled;
    int showHelp;
    
    Star stars[MAX_STARS];
    Asteroid asteroids[MAX_ASTEROIDS];
    OreChunk oreChunks[MAX_ORE_CHUNKS];
    Particle particles[MAX_PARTICLES];
    FloatingText texts[MAX_FLOATING_TEXTS];
    LogEntry logs[MAX_LOG_ENTRIES];
    int logCount;
} GameState;

static GameState g_state;
static HWND g_hwnd = NULL;
static HWND g_btnLaser, g_btnTractor, g_btnDampener, g_btnScan, g_btnNav, g_btnTheme, g_btnScanlines, g_btnAudio, g_btnHelp, g_btnJettison, g_btnSell;
static HFONT g_fontMono = NULL;
static HFONT g_fontMonoBold = NULL;
static HFONT g_fontSmall = NULL;
static HFONT g_fontHeader = NULL;

// Audio System
static volatile int g_currentSfx = SFX_NONE;
static HANDLE g_hSoundThread = NULL;

DWORD WINAPI SoundThreadProc(LPVOID lpParam) {
    while (1) {
        if (g_currentSfx != SFX_NONE && g_state.soundEnabled) {
            int sfx = g_currentSfx;
            g_currentSfx = SFX_NONE;
            if (sfx == SFX_COLLECT) {
                Beep(600, 40);
                Beep(880, 60);
            } else if (sfx == SFX_FRACTURE) {
                Beep(180, 80);
                Beep(110, 100);
            } else if (sfx == SFX_OVERHEAT) {
                Beep(320, 100);
                Beep(240, 120);
            } else if (sfx == SFX_BEEP) {
                Beep(700, 50);
            } else if (sfx == SFX_LASER_PULSE) {
                Beep(480, 30);
            } else if (sfx == SFX_WARP) {
                Beep(160, 60);
                Beep(420, 80);
                Beep(920, 110);
                Beep(240, 90);
            }
        }
        Sleep(20);
    }
    return 0;
}

void TriggerSound(int sfx) {
    if (g_state.soundEnabled) {
        g_currentSfx = sfx;
    }
}

// Forward Declarations
void AddLog(const char* text, int type);
void AddFloatingText(const char* text, float x, float y, COLORREF color);
void AddSparks(float x, float y, COLORREF color, int count);
void SpawnOreChunk(float x, float y, int oreType, int amount);
void SpawnAsteroid(int index, int oreType);
int PickOreForSector(int sectorIdx);
void InitSectorField(int sectorIdx);
void EngageWarpJump(int targetSectorIdx);
void InitGame(void);
void UpdateGame(float dt);
void RenderGame(HDC hdc, RECT* clientRect);
int CalculateCargoValue(void);
void UpdateCargoTotal(void);

void AddLog(const char* text, int type) {
    if (g_state.logCount < MAX_LOG_ENTRIES) {
        strncpy(g_state.logs[g_state.logCount].text, text, 127);
        g_state.logs[g_state.logCount].text[127] = '\0';
        g_state.logs[g_state.logCount].type = type;
        g_state.logCount++;
    } else {
        for (int i = 0; i < MAX_LOG_ENTRIES - 1; i++) {
            g_state.logs[i] = g_state.logs[i + 1];
        }
        strncpy(g_state.logs[MAX_LOG_ENTRIES - 1].text, text, 127);
        g_state.logs[MAX_LOG_ENTRIES - 1].text[127] = '\0';
        g_state.logs[MAX_LOG_ENTRIES - 1].type = type;
    }
}

void AddFloatingText(const char* text, float x, float y, COLORREF color) {
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        if (!g_state.texts[i].active) {
            strncpy(g_state.texts[i].text, text, 31);
            g_state.texts[i].text[31] = '\0';
            g_state.texts[i].x = x;
            g_state.texts[i].y = y;
            g_state.texts[i].vy = -0.7f;
            g_state.texts[i].color = color;
            g_state.texts[i].life = 1.2f;
            g_state.texts[i].active = 1;
            break;
        }
    }
}

void AddSparks(float x, float y, COLORREF color, int count) {
    for (int k = 0; k < count; k++) {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!g_state.particles[i].active) {
                float angle = ((float)rand() / (float)RAND_MAX) * 6.28318f;
                float speed = 0.8f + (((float)rand() / (float)RAND_MAX) * 2.2f);
                g_state.particles[i].x = x;
                g_state.particles[i].y = y;
                g_state.particles[i].vx = (float)cos(angle) * speed;
                g_state.particles[i].vy = (float)sin(angle) * speed;
                g_state.particles[i].color = color;
                g_state.particles[i].life = 1.0f;
                g_state.particles[i].decay = 0.04f + (((float)rand() / (float)RAND_MAX) * 0.05f);
                g_state.particles[i].size = 2.0f;
                g_state.particles[i].active = 1;
                break;
            }
        }
    }
}

void SpawnOreChunk(float x, float y, int oreType, int amount) {
    for (int i = 0; i < MAX_ORE_CHUNKS; i++) {
        if (!g_state.oreChunks[i].active) {
            float angle = ((float)rand() / (float)RAND_MAX) * 6.28318f;
            float speed = 0.5f + (((float)rand() / (float)RAND_MAX) * 1.5f);
            g_state.oreChunks[i].x = x;
            g_state.oreChunks[i].y = y;
            g_state.oreChunks[i].vx = (float)cos(angle) * speed;
            g_state.oreChunks[i].vy = (float)sin(angle) * speed;
            g_state.oreChunks[i].oreType = oreType;
            g_state.oreChunks[i].amount = amount;
            g_state.oreChunks[i].life = 60.0f;
            g_state.oreChunks[i].rot = 0.0f;
            g_state.oreChunks[i].rotSpeed = (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.1f;
            g_state.oreChunks[i].active = 1;
            break;
        }
    }
}

int PickOreForSector(int sectorIdx) {
    if (sectorIdx < 0 || sectorIdx >= 4) sectorIdx = 0;
    const SectorDef* sec = &SECTOR_DEFS[sectorIdx];
    int total = 0;
    for (int i = 0; i < 6; i++) total += sec->oreWeights[i];
    if (total <= 0) return rand() % 6;
    int r = rand() % total;
    for (int i = 0; i < 6; i++) {
        if (r < sec->oreWeights[i]) return i;
        r -= sec->oreWeights[i];
    }
    return 0;
}

void SpawnAsteroid(int index, int oreType) {
    Asteroid* ast = &g_state.asteroids[index];
    float angle = ((float)rand() / (float)RAND_MAX) * 6.28318f;
    float dist = 240.0f + (((float)rand() / (float)RAND_MAX) * 850.0f);
    
    sprintf(ast->id, "AST-%03d", 100 + index * 17 + (rand() % 80));
    ast->x = g_state.shipX + (float)cos(angle) * dist;
    ast->y = g_state.shipY + (float)sin(angle) * dist;
    ast->vx = (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.3f;
    ast->vy = (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.3f;
    ast->radius = 18.0f + (((float)rand() / (float)RAND_MAX) * 24.0f);
    ast->maxHp = (float)((int)(ast->radius * 3.5f));
    ast->hp = ast->maxHp;
    ast->oreType = (oreType >= 0 && oreType < 6) ? oreType : PickOreForSector(g_state.currentSectorIndex);
    ast->richness = 40 + (rand() % 60);
    ast->rot = ((float)rand() / (float)RAND_MAX) * 6.28318f;
    ast->rotSpeed = (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.02f;
    ast->active = 1;
    
    ast->numVerts = 8 + (rand() % 4);
    for (int v = 0; v < ast->numVerts; v++) {
        ast->verts[v].a = ((float)v / (float)ast->numVerts) * 6.28318f;
        ast->verts[v].r = ast->radius * (0.75f + (((float)rand() / (float)RAND_MAX) * 0.35f));
    }
}

void InitSectorField(int sectorIdx) {
    if (sectorIdx < 0 || sectorIdx >= 4) sectorIdx = 0;
    g_state.currentSectorIndex = sectorIdx;
    g_state.selectedSectorIndex = sectorIdx;
    strncpy(g_state.sector, SECTOR_DEFS[sectorIdx].name, 31);
    g_state.sector[31] = '\0';
    
    // Seed Stars
    for (int i = 0; i < MAX_STARS; i++) {
        g_state.stars[i].x = (((float)rand() / (float)RAND_MAX) - 0.5f) * 4500.0f;
        g_state.stars[i].y = (((float)rand() / (float)RAND_MAX) - 0.5f) * 4500.0f;
        g_state.stars[i].size = 1.0f + (((float)rand() / (float)RAND_MAX) * 1.6f);
        g_state.stars[i].brightness = 0.3f + (((float)rand() / (float)RAND_MAX) * 0.7f);
    }
    
    // Seed Asteroids per density
    int count = SECTOR_DEFS[sectorIdx].density;
    if (count > MAX_ASTEROIDS) count = MAX_ASTEROIDS;
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (i < count) {
            SpawnAsteroid(i, PickOreForSector(sectorIdx));
        } else {
            g_state.asteroids[i].active = 0;
        }
    }
    
    for (int i = 0; i < MAX_ORE_CHUNKS; i++) g_state.oreChunks[i].active = 0;
    g_state.selectedAstIndex = -1;
}

void EngageWarpJump(int targetSectorIdx) {
    if (targetSectorIdx < 0 || targetSectorIdx >= 4) return;
    if (targetSectorIdx == g_state.currentSectorIndex) return;
    const SectorDef* target = &SECTOR_DEFS[targetSectorIdx];
    if (g_state.fuel < (float)target->fuelCost) {
        AddLog("CRITICAL: Insufficient fuel for warp jump!", 4);
        return;
    }
    
    g_state.fuel -= (float)target->fuelCost;
    g_state.warpActive = 1;
    g_state.warpTimer = 1.6f;
    g_state.showStarChart = 0;
    TriggerSound(SFX_WARP);
    
    char buf[128];
    sprintf(buf, "Sub-space warp jump initiated to: [%s]", target->name);
    AddLog(buf, 6);
    
    char fTxt[32];
    sprintf(fTxt, "WARP: %s", target->name);
    AddFloatingText(fTxt, g_state.shipX, g_state.shipY - 40.0f, RGB(244, 63, 94));
    
    // Warp particle burst
    for (int i = 0; i < 40; i++) {
        float angle = ((float)rand() / (float)RAND_MAX) * 6.28318f;
        float speed = 8.0f + (((float)rand() / (float)RAND_MAX) * 12.0f);
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (!g_state.particles[p].active) {
                g_state.particles[p].x = g_state.shipX;
                g_state.particles[p].y = g_state.shipY;
                g_state.particles[p].vx = (float)cos(angle) * speed;
                g_state.particles[p].vy = (float)sin(angle) * speed;
                g_state.particles[p].color = (rand() % 2 == 0) ? RGB(244, 63, 94) : RGB(0, 240, 255);
                g_state.particles[p].life = 1.4f;
                g_state.particles[p].decay = 0.02f;
                g_state.particles[p].size = 3.0f;
                g_state.particles[p].active = 1;
                break;
            }
        }
    }
}

int CalculateCargoValue(void) {
    int total = 0;
    for (int i = 0; i < 6; i++) {
        total += g_state.cargoHold[i] * ORE_DEFS[i].value;
    }
    return total;
}

void UpdateCargoTotal(void) {
    int sum = 0;
    for (int i = 0; i < 6; i++) sum += g_state.cargoHold[i];
    g_state.totalCargo = sum;
}

void DrawScanlines(HDC hdc, int x, int y, int w, int h, int mode) {
    if (mode == 0) return; // Off
    HPEN hPenScan = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    HGDIOBJ oldPen = SelectObject(hdc, hPenScan);
    int step = (mode == 2) ? 2 : 3;
    for (int ly = y; ly < y + h; ly += step) {
        MoveToEx(hdc, x, ly, NULL);
        LineTo(hdc, x + w, ly);
    }
    if (mode == 2) {
        // CRT curvature / tube vignette lines on corners
        HPEN hPenVig = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
        SelectObject(hdc, hPenVig);
        MoveToEx(hdc, x, y + 25, NULL); LineTo(hdc, x, y); LineTo(hdc, x + 25, y);
        MoveToEx(hdc, x + w - 25, y, NULL); LineTo(hdc, x + w, y); LineTo(hdc, x + w, y + 25);
        MoveToEx(hdc, x, y + h - 25, NULL); LineTo(hdc, x, y + h); LineTo(hdc, x + 25, y + h);
        MoveToEx(hdc, x + w - 25, y + h, NULL); LineTo(hdc, x + w, y + h); LineTo(hdc, x + w, y + h - 25);
        DeleteObject(hPenVig);
    }
    SelectObject(hdc, oldPen);
    DeleteObject(hPenScan);
}

void CycleTheme(void) {
    g_state.themeIndex = (g_state.themeIndex + 1) % 4;
    TriggerSound(SFX_BEEP);
    char buf[64];
    sprintf(buf, "Cockpit HUD theme switched to: [%s]", THEME_PALETTES[g_state.themeIndex].name);
    AddLog(buf, 0);
    if (g_btnTheme) {
        SetWindowTextA(g_btnTheme, THEME_PALETTES[g_state.themeIndex].name);
    }
}

void CycleScanlines(void) {
    g_state.scanlineMode = (g_state.scanlineMode + 1) % 3;
    TriggerSound(SFX_BEEP);
    const char* names[3] = { "SCANLINES: OFF", "SCANLINES: ON", "SCANLINES: CRT+" };
    char buf[64];
    sprintf(buf, "CRT scanline raster filter: [%s]", names[g_state.scanlineMode]);
    AddLog(buf, 0);
    if (g_btnScanlines) {
        SetWindowTextA(g_btnScanlines, (g_state.scanlineMode == 0) ? "SCAN: OFF" : (g_state.scanlineMode == 1 ? "SCAN: ON" : "SCAN: CRT+"));
    }
}

void InitGame(void) {
    memset(&g_state, 0, sizeof(GameState));
    g_state.credits = 2500;
    g_state.hull = 100.0f;
    g_state.maxHull = 100.0f;
    g_state.shield = 100.0f;
    g_state.maxShield = 100.0f;
    g_state.fuel = 100.0f;
    g_state.maxFuel = 100.0f;
    g_state.heat = 0.0f;
    g_state.maxHeat = 100.0f;
    g_state.reactor = 88.0f;
    g_state.o2 = 100.0f;
    g_state.maxCargo = 200;
    g_state.dampeners = 1;
    g_state.soundEnabled = 1;
    g_state.themeIndex = 0;
    g_state.scanlineMode = 1;
    g_state.shipAngle = -1.57079f; // Facing UP
    g_state.selectedAstIndex = -1;
    
    InitSectorField(0);
    
    AddLog("[SYSTEM] KStarDredge Mk-IV cockpit operational. Core reactor online.", 0);
    AddLog("[MINING] High-frequency mining laser ready. Aim at asteroids and hold [SPACE].", 1);
    AddLog("[TRACTOR] Tractor emitter active. Hold [T] to gather extracted mineral chunks.", 2);
    AddLog("[NAV] Astronavigation computer initialized. Press [N] for Sector Charts.", 0);
}

void UpdateGame(float dt) {
    // Warp Sequence Logic
    if (g_state.warpActive) {
        g_state.warpTimer -= dt;
        if (g_state.warpTimer <= 0.4f && g_state.currentSectorIndex != g_state.selectedSectorIndex) {
            InitSectorField(g_state.selectedSectorIndex);
            g_state.shipVx = 0.0f;
            g_state.shipVy = 0.0f;
        }
        if (g_state.warpTimer <= 0.0f) {
            g_state.warpActive = 0;
            char exBuf[128];
            sprintf(exBuf, "Exited hyperspace into %s. Asteroid radar recalibrated.", SECTOR_DEFS[g_state.currentSectorIndex].name);
            AddLog(exBuf, 5);
            TriggerSound(SFX_COLLECT);
        }
    }

    // Steering
    float rotAccel = 0.05f;
    if (g_state.turningLeft) g_state.shipAngle -= rotAccel;
    if (g_state.turningRight) g_state.shipAngle += rotAccel;
    
    // Propulsion
    float thrustPower = 0.12f;
    if (g_state.thrusting && g_state.fuel > 0.0f) {
        g_state.shipVx += (float)cos(g_state.shipAngle) * thrustPower;
        g_state.shipVy += (float)sin(g_state.shipAngle) * thrustPower;
        g_state.fuel = max(0.0f, g_state.fuel - 0.03f);
        
        // Thrust sparks
        float exAngle = g_state.shipAngle + 3.14159f + (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.4f;
        float exX = g_state.shipX - (float)cos(g_state.shipAngle) * 18.0f;
        float exY = g_state.shipY - (float)sin(g_state.shipAngle) * 18.0f;
        COLORREF thrusterColor = THEME_PALETTES[g_state.themeIndex].vector;
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!g_state.particles[i].active) {
                g_state.particles[i].x = exX;
                g_state.particles[i].y = exY;
                g_state.particles[i].vx = (float)cos(exAngle) * (2.0f + ((float)rand() / (float)RAND_MAX) * 2.0f);
                g_state.particles[i].vy = (float)sin(exAngle) * (2.0f + ((float)rand() / (float)RAND_MAX) * 2.0f);
                g_state.particles[i].color = thrusterColor;
                g_state.particles[i].life = 0.8f;
                g_state.particles[i].decay = 0.06f;
                g_state.particles[i].size = 2.5f;
                g_state.particles[i].active = 1;
                break;
            }
        }
    }
    
    if (g_state.reversing && g_state.fuel > 0.0f) {
        g_state.shipVx -= (float)cos(g_state.shipAngle) * (thrustPower * 0.5f);
        g_state.shipVy -= (float)sin(g_state.shipAngle) * (thrustPower * 0.5f);
        g_state.fuel = max(0.0f, g_state.fuel - 0.015f);
    }
    
    // Inertial Dampeners
    if (g_state.dampeners && !g_state.thrusting && !g_state.reversing) {
        g_state.shipVx *= 0.96f;
        g_state.shipVy *= 0.96f;
    }
    
    // Speed Cap
    float speed = (float)sqrt(g_state.shipVx * g_state.shipVx + g_state.shipVy * g_state.shipVy);
    if (speed > 6.0f) {
        g_state.shipVx = (g_state.shipVx / speed) * 6.0f;
        g_state.shipVy = (g_state.shipVy / speed) * 6.0f;
    }
    
    g_state.shipX += g_state.shipVx;
    g_state.shipY += g_state.shipVy;
    
    // Laser Overheat Logic
    if (g_state.miningActive && !g_state.laserOverheated) {
        g_state.heat += 0.5f;
        if (g_state.heat >= g_state.maxHeat) {
            g_state.laserOverheated = 1;
            g_state.heat = 100.0f;
            TriggerSound(SFX_OVERHEAT);
            AddLog("CRITICAL: Laser optics overheated! Emergency cooling cycle initiated.", 4);
        }
    } else {
        g_state.heat = max(0.0f, g_state.heat - 0.3f);
        if (g_state.laserOverheated && g_state.heat < 25.0f) {
            g_state.laserOverheated = 0;
            AddLog("Laser cooling cycle complete. Optics online.", 5);
        }
    }
    
    // Mining Laser Beam Impact
    if (g_state.miningActive && !g_state.laserOverheated) {
        float laserRange = 250.0f;
        float lx = g_state.shipX + (float)cos(g_state.shipAngle) * 20.0f;
        float ly = g_state.shipY + (float)sin(g_state.shipAngle) * 20.0f;
        float dirX = (float)cos(g_state.shipAngle);
        float dirY = (float)sin(g_state.shipAngle);
        
        for (int i = 0; i < MAX_ASTEROIDS; i++) {
            Asteroid* ast = &g_state.asteroids[i];
            if (!ast->active) continue;
            
            float toAstX = ast->x - lx;
            float toAstY = ast->y - ly;
            float proj = toAstX * dirX + toAstY * dirY;
            
            if (proj > 0.0f && proj < laserRange) {
                float perpX = toAstX - dirX * proj;
                float perpY = toAstY - dirY * proj;
                float distToBeam = (float)sqrt(perpX * perpX + perpY * perpY);
                
                if (distToBeam < ast->radius) {
                    float impactX = lx + dirX * proj;
                    float impactY = ly + dirY * proj;
                    AddSparks(impactX, impactY, THEME_PALETTES[g_state.themeIndex].vector, 2);
                    ast->hp -= 0.6f;
                    
                    if ((rand() % 100) < 14) {
                        TriggerSound(SFX_FRACTURE);
                        SpawnOreChunk(impactX, impactY, ast->oreType, 1);
                        char buf[32];
                        sprintf(buf, "+1 %s", ORE_DEFS[ast->oreType].name);
                        AddFloatingText(buf, impactX, impactY - 10.0f, ORE_DEFS[ast->oreType].color);
                    }
                    
                    if (ast->hp <= 0.0f) {
                        TriggerSound(SFX_FRACTURE);
                        char buf[64];
                        sprintf(buf, "Asteroid %s shattered into rich mineral fragments!", ast->id);
                        AddLog(buf, 1);
                        AddSparks(ast->x, ast->y, RGB(245, 158, 11), 16);
                        for (int c = 0; c < 4 + (rand() % 3); c++) {
                            SpawnOreChunk(ast->x + (((float)rand() / (float)RAND_MAX) - 0.5f) * 20.0f,
                                          ast->y + (((float)rand() / (float)RAND_MAX) - 0.5f) * 20.0f,
                                          ast->oreType, 1);
                        }
                        ast->active = 0;
                        if (g_state.selectedAstIndex == i) g_state.selectedAstIndex = -1;
                        SpawnAsteroid(i, PickOreForSector(g_state.currentSectorIndex));
                    }
                    break;
                }
            }
        }
    }
    
    // Tractor Beam Logic
    if (g_state.tractorActive) {
        float tractorRange = 320.0f;
        for (int i = 0; i < MAX_ORE_CHUNKS; i++) {
            OreChunk* chunk = &g_state.oreChunks[i];
            if (!chunk->active) continue;
            
            float dx = g_state.shipX - chunk->x;
            float dy = g_state.shipY - chunk->y;
            float dist = (float)sqrt(dx * dx + dy * dy);
            
            if (dist < tractorRange && dist > 1.0f) {
                float pullForce = 2.2f / max(1.0f, dist * 0.05f);
                chunk->vx += (dx / dist) * pullForce;
                chunk->vy += (dy / dist) * pullForce;
                chunk->vx *= 0.94f;
                chunk->vy *= 0.94f;
            }
        }
    }
    
    // Ore Chunk Physics & Cargo Scooping
    float scoopRadius = 24.0f;
    for (int i = 0; i < MAX_ORE_CHUNKS; i++) {
        OreChunk* chunk = &g_state.oreChunks[i];
        if (!chunk->active) continue;
        
        chunk->x += chunk->vx;
        chunk->y += chunk->vy;
        chunk->rot += chunk->rotSpeed;
        chunk->life -= dt;
        
        float distToShip = (float)sqrt((g_state.shipX - chunk->x) * (g_state.shipX - chunk->x) +
                                      (g_state.shipY - chunk->y) * (g_state.shipY - chunk->y));
        if (distToShip < scoopRadius) {
            UpdateCargoTotal();
            if (g_state.totalCargo < g_state.maxCargo) {
                g_state.cargoHold[chunk->oreType] += chunk->amount;
                UpdateCargoTotal();
                TriggerSound(SFX_COLLECT);
                char buf[32];
                sprintf(buf, "+%dT %s", chunk->amount, ORE_DEFS[chunk->oreType].name);
                AddFloatingText(buf, g_state.shipX, g_state.shipY - 20.0f, ORE_DEFS[chunk->oreType].color);
                
                char logBuf[64];
                sprintf(logBuf, "Scooped %dT of %s into ore hold.", chunk->amount, ORE_DEFS[chunk->oreType].name);
                AddLog(logBuf, 2);
            } else {
                AddLog("WARNING: Cargo hold FULL! Cannot scoop ore.", 3);
            }
            chunk->active = 0;
            continue;
        }
        
        if (chunk->life <= 0.0f) {
            chunk->active = 0;
        }
    }
    
    // Asteroid Physics & Collisions with Ship
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        Asteroid* ast = &g_state.asteroids[i];
        if (!ast->active) continue;
        
        ast->x += ast->vx;
        ast->y += ast->vy;
        ast->rot += ast->rotSpeed;
        
        float dist = (float)sqrt((g_state.shipX - ast->x) * (g_state.shipX - ast->x) +
                                 (g_state.shipY - ast->y) * (g_state.shipY - ast->y));
        float minDist = ast->radius + 14.0f;
        if (dist < minDist && dist > 0.1f) {
            float angle = (float)atan2(g_state.shipY - ast->y, g_state.shipX - ast->x);
            g_state.shipVx += (float)cos(angle) * 1.5f;
            g_state.shipVy += (float)sin(angle) * 1.5f;
            
            if (g_state.shield > 0.0f) {
                g_state.shield = max(0.0f, g_state.shield - 8.0f);
            } else {
                g_state.hull = max(0.0f, g_state.hull - 5.0f);
            }
            AddSparks(g_state.shipX, g_state.shipY, RGB(239, 68, 68), 8);
            TriggerSound(SFX_FRACTURE);
            char buf[64];
            sprintf(buf, "COLLISION ALERT with %s! Shield deflected impact.", ast->id);
            AddLog(buf, 3);
        }
    }
    
    // Shield Regeneration
    if (g_state.shield < g_state.maxShield) {
        g_state.shield = min(g_state.maxShield, g_state.shield + 0.05f);
    }
    
    // Update Particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!g_state.particles[i].active) continue;
        g_state.particles[i].x += g_state.particles[i].vx;
        g_state.particles[i].y += g_state.particles[i].vy;
        g_state.particles[i].life -= g_state.particles[i].decay;
        if (g_state.particles[i].life <= 0.0f) g_state.particles[i].active = 0;
    }
    
    // Update Floating Text
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        if (!g_state.texts[i].active) continue;
        g_state.texts[i].y += g_state.texts[i].vy;
        g_state.texts[i].life -= 0.02f;
        if (g_state.texts[i].life <= 0.0f) g_state.texts[i].active = 0;
    }
    
    // Radar Sweep
    g_state.radarAngle += 0.05f;
    if (g_state.radarAngle > 6.28318f) g_state.radarAngle -= 6.28318f;
    
    // Auto Target Closest Asteroid
    if (g_state.selectedAstIndex < 0 || !g_state.asteroids[g_state.selectedAstIndex].active) {
        float minDist = 600.0f;
        int bestIdx = -1;
        for (int i = 0; i < MAX_ASTEROIDS; i++) {
            if (!g_state.asteroids[i].active) continue;
            float d = (float)sqrt((g_state.asteroids[i].x - g_state.shipX) * (g_state.asteroids[i].x - g_state.shipX) +
                                  (g_state.asteroids[i].y - g_state.shipY) * (g_state.asteroids[i].y - g_state.shipY));
            if (d < minDist) {
                minDist = d;
                bestIdx = i;
            }
        }
        g_state.selectedAstIndex = bestIdx;
    }
}

void DrawBar(HDC hdc, int x, int y, int w, int h, float pct, COLORREF fillColor, COLORREF bgColor, COLORREF borderCol) {
    RECT rcBg = { x, y, x + w, y + h };
    HBRUSH hBrBg = CreateSolidBrush(bgColor);
    FillRect(hdc, &rcBg, hBrBg);
    DeleteObject(hBrBg);
    
    HPEN hPen = CreatePen(PS_SOLID, 1, borderCol);
    HGDIOBJ oldPen = SelectObject(hdc, hPen);
    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, x, y, x + w, y + h);
    
    int fillW = (int)((w - 2) * max(0.0f, min(1.0f, pct)));
    if (fillW > 0) {
        RECT rcFill = { x + 1, y + 1, x + 1 + fillW, y + h - 1 };
        HBRUSH hBrFill = CreateSolidBrush(fillColor);
        FillRect(hdc, &rcFill, hBrFill);
        DeleteObject(hBrFill);
    }
    
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(hPen);
}

void RenderGame(HDC hdc, RECT* clientRect) {
    int totalW = clientRect->right - clientRect->left;
    int totalH = clientRect->bottom - clientRect->top;
    if (totalW <= 0 || totalH <= 0) return;
    
    const ThemePalette* pal = &THEME_PALETTES[g_state.themeIndex];
    
    // Layout geometry
    int topHeaderH = 34;
    int bottomCtrlH = 140;
    int leftPanelW = 220;
    int rightPanelW = 240;
    
    int mainY = topHeaderH;
    int mainH = totalH - topHeaderH - bottomCtrlH;
    int viewportX = leftPanelW;
    int viewportW = totalW - leftPanelW - rightPanelW;
    int viewportY = mainY;
    int viewportH = mainH;
    
    // 1. Top Header Bar
    RECT rcHeader = { 0, 0, totalW, topHeaderH };
    HBRUSH hBrHeader = CreateSolidBrush(pal->bgHeader);
    FillRect(hdc, &rcHeader, hBrHeader);
    DeleteObject(hBrHeader);
    
    HPEN hPenBorder = CreatePen(PS_SOLID, 1, pal->borderPanel);
    HGDIOBJ oldPen = SelectObject(hdc, hPenBorder);
    MoveToEx(hdc, 0, topHeaderH - 1, NULL);
    LineTo(hdc, totalW, topHeaderH - 1);
    
    SelectObject(hdc, g_fontHeader);
    SetTextColor(hdc, pal->vector);
    SetBkMode(hdc, TRANSPARENT);
    TextOutA(hdc, 12, 7, "☄️ KStarDredge - Heavy Mining Barge", 36);
    
    SelectObject(hdc, g_fontMonoBold);
    char statBuf[128];
    UpdateCargoTotal();
    sprintf(statBuf, "SECTOR: %s   CREDITS: %d CR   HOLD: %d/%dT   HULL: %d%%",
            g_state.sector, g_state.credits, g_state.totalCargo, g_state.maxCargo, (int)g_state.hull);
    SetTextColor(hdc, pal->textBright);
    RECT rcStats = { totalW - 500, 0, totalW - 12, topHeaderH };
    DrawTextA(hdc, statBuf, -1, &rcStats, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    
    // 2. Left Panel: Ship Systems & Telemetry
    RECT rcLeft = { 0, mainY, leftPanelW, mainY + mainH };
    HBRUSH hBrPanel = CreateSolidBrush(pal->bgPanel);
    FillRect(hdc, &rcLeft, hBrPanel);
    
    MoveToEx(hdc, leftPanelW - 1, mainY, NULL);
    LineTo(hdc, leftPanelW - 1, mainY + mainH);
    
    // Panel Header
    RECT rcLeftHdr = { 0, mainY, leftPanelW, mainY + 22 };
    HBRUSH hBrSubHdr = CreateSolidBrush(pal->bgHeader);
    FillRect(hdc, &rcLeftHdr, hBrSubHdr);
    SelectObject(hdc, g_fontMonoBold);
    SetTextColor(hdc, pal->textPrimary);
    TextOutA(hdc, 8, mainY + 4, "BARGE TELEMETRY", 15);
    
    // System Meters
    int my = mainY + 30;
    int meterW = leftPanelW - 20;
    SelectObject(hdc, g_fontSmall);
    
    // Reactor
    SetTextColor(hdc, pal->textPrimary);
    TextOutA(hdc, 10, my, "REACTOR OUTPUT (MW)", 19);
    char valBuf[32];
    sprintf(valBuf, "%d MW", (int)g_state.reactor);
    TextOutA(hdc, leftPanelW - 55, my, valBuf, (int)strlen(valBuf));
    DrawBar(hdc, 10, my + 14, meterW, 8, g_state.reactor / 100.0f, pal->vector, RGB(2, 6, 23), pal->borderPanel);
    my += 28;
    
    // Shield
    SetTextColor(hdc, pal->textPrimary);
    TextOutA(hdc, 10, my, "SHIELD INTEGRITY", 16);
    sprintf(valBuf, "%d%%", (int)g_state.shield);
    TextOutA(hdc, leftPanelW - 45, my, valBuf, (int)strlen(valBuf));
    DrawBar(hdc, 10, my + 14, meterW, 8, g_state.shield / 100.0f, RGB(16, 185, 129), RGB(2, 6, 23), pal->borderPanel);
    my += 28;
    
    // Hull
    SetTextColor(hdc, pal->textPrimary);
    TextOutA(hdc, 10, my, "HULL PLATING", 12);
    sprintf(valBuf, "%d%%", (int)g_state.hull);
    TextOutA(hdc, leftPanelW - 45, my, valBuf, (int)strlen(valBuf));
    COLORREF hullCol = (g_state.hull < 30.0f) ? RGB(239, 68, 68) : pal->vectorDim;
    DrawBar(hdc, 10, my + 14, meterW, 8, g_state.hull / 100.0f, hullCol, RGB(2, 6, 23), pal->borderPanel);
    my += 28;
    
    // Laser Heat
    SetTextColor(hdc, pal->textPrimary);
    TextOutA(hdc, 10, my, "LASER CORE HEAT", 15);
    sprintf(valBuf, "%d%%", (int)g_state.heat);
    TextOutA(hdc, leftPanelW - 45, my, valBuf, (int)strlen(valBuf));
    COLORREF heatCol = (g_state.heat > 80.0f) ? RGB(239, 68, 68) : (g_state.heat > 50.0f ? RGB(245, 158, 11) : pal->vector);
    DrawBar(hdc, 10, my + 14, meterW, 8, g_state.heat / 100.0f, heatCol, RGB(2, 6, 23), pal->borderPanel);
    my += 28;
    
    // Fuel
    SetTextColor(hdc, pal->textPrimary);
    TextOutA(hdc, 10, my, "FUEL RESERVES", 13);
    sprintf(valBuf, "%d%%", (int)g_state.fuel);
    TextOutA(hdc, leftPanelW - 45, my, valBuf, (int)strlen(valBuf));
    DrawBar(hdc, 10, my + 14, meterW, 8, g_state.fuel / 100.0f, RGB(251, 191, 36), RGB(2, 6, 23), pal->borderPanel);
    my += 28;
    
    // O2 / Life Support
    SetTextColor(hdc, pal->textPrimary);
    TextOutA(hdc, 10, my, "O2 / LIFE SUPPORT", 17);
    sprintf(valBuf, "%d%%", (int)g_state.o2);
    TextOutA(hdc, leftPanelW - 45, my, valBuf, (int)strlen(valBuf));
    DrawBar(hdc, 10, my + 14, meterW, 8, g_state.o2 / 100.0f, RGB(16, 185, 129), RGB(2, 6, 23), pal->borderPanel);
    my += 34;
    
    // System Status Summary Box
    RECT rcSysBox = { 10, my, leftPanelW - 10, my + 70 };
    HBRUSH hBrBox = CreateSolidBrush(RGB(3, 7, 18));
    FillRect(hdc, &rcSysBox, hBrBox);
    DeleteObject(hBrBox);
    FrameRect(hdc, &rcSysBox, (HBRUSH)GetStockObject(WHITE_BRUSH));
    
    SetTextColor(hdc, RGB(148, 163, 184));
    TextOutA(hdc, 14, my + 4, "THRUST DAMPENERS:", 17);
    SetTextColor(hdc, g_state.dampeners ? RGB(16, 185, 129) : RGB(245, 158, 11));
    TextOutA(hdc, leftPanelW - 75, my + 4, g_state.dampeners ? "ACTIVE" : "OFFLINE", g_state.dampeners ? 6 : 7);
    
    SetTextColor(hdc, RGB(148, 163, 184));
    TextOutA(hdc, 14, my + 20, "MINING LASER:", 13);
    SetTextColor(hdc, g_state.laserOverheated ? RGB(239, 68, 68) : (g_state.miningActive ? pal->vector : pal->textDim));
    TextOutA(hdc, leftPanelW - 85, my + 20, g_state.laserOverheated ? "OVERHEAT" : (g_state.miningActive ? "FIRING" : "STANDBY"), g_state.laserOverheated ? 8 : (g_state.miningActive ? 6 : 7));
    
    SetTextColor(hdc, RGB(148, 163, 184));
    TextOutA(hdc, 14, my + 36, "TRACTOR BEAM:", 13);
    SetTextColor(hdc, g_state.tractorActive ? pal->vector : pal->textDim);
    TextOutA(hdc, leftPanelW - 85, my + 36, g_state.tractorActive ? "ENGAGED" : "STANDBY", g_state.tractorActive ? 7 : 7);
    
    float drain = 12.0f + (g_state.miningActive ? 28.0f : 0.0f) + (g_state.tractorActive ? 14.0f : 0.0f);
    SetTextColor(hdc, RGB(148, 163, 184));
    TextOutA(hdc, 14, my + 52, "POWER GRID DRAIN:", 17);
    char drnBuf[32];
    sprintf(drnBuf, "%.1f MW", drain);
    SetTextColor(hdc, pal->textBright);
    TextOutA(hdc, leftPanelW - 65, my + 52, drnBuf, (int)strlen(drnBuf));
    
    // 3. Right Panel: Mineral Cargo Hold
    RECT rcRight = { totalW - rightPanelW, mainY, totalW, mainY + mainH };
    FillRect(hdc, &rcRight, hBrPanel);
    MoveToEx(hdc, totalW - rightPanelW, mainY, NULL);
    LineTo(hdc, totalW - rightPanelW, mainY + mainH);
    
    RECT rcRightHdr = { totalW - rightPanelW, mainY, totalW, mainY + 22 };
    FillRect(hdc, &rcRightHdr, hBrSubHdr);
    SelectObject(hdc, g_fontMonoBold);
    SetTextColor(hdc, pal->textPrimary);
    TextOutA(hdc, totalW - rightPanelW + 8, mainY + 4, "MINERAL ORE HOLD", 16);
    char capBuf[32];
    sprintf(capBuf, "%d/%dT", g_state.totalCargo, g_state.maxCargo);
    TextOutA(hdc, totalW - 65, mainY + 4, capBuf, (int)strlen(capBuf));
    
    int cy = mainY + 28;
    for (int i = 0; i < 6; i++) {
        RECT rcItem = { totalW - rightPanelW + 8, cy, totalW - 8, cy + 34 };
        HBRUSH hBrItem = CreateSolidBrush(RGB(3, 7, 18));
        FillRect(hdc, &rcItem, hBrItem);
        DeleteObject(hBrItem);
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, ORE_DEFS[i].color);
        TextOutA(hdc, totalW - rightPanelW + 12, cy + 3, ORE_DEFS[i].name, (int)strlen(ORE_DEFS[i].name));
        
        SelectObject(hdc, g_fontSmall);
        char rateBuf[32];
        sprintf(rateBuf, "%d CR/T", ORE_DEFS[i].value);
        SetTextColor(hdc, RGB(148, 163, 184));
        TextOutA(hdc, totalW - rightPanelW + 12, cy + 18, rateBuf, (int)strlen(rateBuf));
        
        char holdBuf[32];
        sprintf(holdBuf, "%d T  (%d CR)", g_state.cargoHold[i], g_state.cargoHold[i] * ORE_DEFS[i].value);
        SetTextColor(hdc, pal->textBright);
        RECT rcHold = { totalW - rightPanelW + 100, cy + 8, totalW - 14, cy + 26 };
        DrawTextA(hdc, holdBuf, -1, &rcHold, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
        
        cy += 38;
    }
    
    // Cargo Hold Summary
    int estVal = CalculateCargoValue();
    SelectObject(hdc, g_fontMonoBold);
    SetTextColor(hdc, pal->textBright);
    TextOutA(hdc, totalW - rightPanelW + 10, cy + 4, "ESTIMATED VALUE:", 16);
    char estBuf[32];
    sprintf(estBuf, "%d CR", estVal);
    SetTextColor(hdc, RGB(245, 158, 11));
    RECT rcEst = { totalW - 100, cy + 4, totalW - 12, cy + 20 };
    DrawTextA(hdc, estBuf, -1, &rcEst, DT_RIGHT | DT_SINGLELINE);
    
    // 4. Center Viewport (Space & Asteroid Field)
    RECT rcViewport = { viewportX, viewportY, viewportX + viewportW, viewportY + viewportH };
    const SectorDef* curSec = &SECTOR_DEFS[g_state.currentSectorIndex];
    HBRUSH hBrSpace = CreateSolidBrush(curSec->nebulaColor);
    FillRect(hdc, &rcViewport, hBrSpace);
    DeleteObject(hBrSpace);
    
    // Set viewport clipping
    HRGN hRgnClip = CreateRectRgn(viewportX, viewportY, viewportX + viewportW, viewportY + viewportH);
    SelectClipRgn(hdc, hRgnClip);
    
    int cx = viewportX + (viewportW / 2);
    int cyCenter = viewportY + (viewportH / 2);
    
    // Render Background Stars (Parallax)
    for (int i = 0; i < MAX_STARS; i++) {
        int sx = cx + (int)(g_state.stars[i].x - g_state.shipX * 0.15f);
        int sy = cyCenter + (int)(g_state.stars[i].y - g_state.shipY * 0.15f);
        if (sx >= viewportX && sx < viewportX + viewportW && sy >= viewportY && sy < viewportY + viewportH) {
            SetPixel(hdc, sx, sy, curSec->starColor);
            if (g_state.stars[i].size > 1.2f) {
                SetPixel(hdc, sx + 1, sy, curSec->starColor);
                SetPixel(hdc, sx, sy + 1, curSec->starColor);
            }
        }
    }
    
    // Hyperspace Warp Drive Streak Effect
    if (g_state.warpActive) {
        HPEN hPenWarp = CreatePen(PS_SOLID, 2, RGB(244, 63, 94));
        HGDIOBJ oldWpPen = SelectObject(hdc, hPenWarp);
        for (int i = 0; i < 32; i++) {
            float angle = ((float)i / 32.0f) * 6.28318f;
            float len = 60.0f + (float)(rand() % 140);
            int x1 = cx + (int)(cos(angle) * 30.0f);
            int y1 = cyCenter + (int)(sin(angle) * 30.0f);
            int x2 = cx + (int)(cos(angle) * (30.0f + len));
            int y2 = cyCenter + (int)(sin(angle) * (30.0f + len));
            MoveToEx(hdc, x1, y1, NULL);
            LineTo(hdc, x2, y2);
        }
        SelectObject(hdc, oldWpPen);
        DeleteObject(hPenWarp);
    }
    
    // World Space Relative to Ship
    // Draw Tractor Wave Cone
    if (g_state.tractorActive) {
        float tractorRange = 320.0f;
        float angleL = g_state.shipAngle - 0.785f;
        float angleR = g_state.shipAngle + 0.785f;
        
        HPEN hPenTractor = CreatePen(PS_SOLID, 1, pal->vector);
        HGDIOBJ oldTrPen = SelectObject(hdc, hPenTractor);
        
        MoveToEx(hdc, cx, cyCenter, NULL);
        LineTo(hdc, cx + (int)(cos(angleL) * tractorRange), cyCenter + (int)(sin(angleL) * tractorRange));
        MoveToEx(hdc, cx, cyCenter, NULL);
        LineTo(hdc, cx + (int)(cos(angleR) * tractorRange), cyCenter + (int)(sin(angleR) * tractorRange));
        
        SelectObject(hdc, oldTrPen);
        DeleteObject(hPenTractor);
    }
    
    // Draw Mining Laser Beam
    if (g_state.miningActive && !g_state.laserOverheated) {
        float laserRange = 250.0f;
        int lx = cx + (int)(cos(g_state.shipAngle) * 20.0f);
        int ly = cyCenter + (int)(sin(g_state.shipAngle) * 20.0f);
        int endX = cx + (int)(cos(g_state.shipAngle) * laserRange);
        int endY = cyCenter + (int)(sin(g_state.shipAngle) * laserRange);
        
        HPEN hPenLaser = CreatePen(PS_SOLID, 3, pal->laserGlow);
        HGDIOBJ oldLzrPen = SelectObject(hdc, hPenLaser);
        MoveToEx(hdc, lx, ly, NULL);
        LineTo(hdc, endX, endY);
        
        HPEN hPenLaserCore = CreatePen(PS_SOLID, 1, pal->laserCore);
        SelectObject(hdc, hPenLaserCore);
        MoveToEx(hdc, lx, ly, NULL);
        LineTo(hdc, endX, endY);
        
        SelectObject(hdc, oldLzrPen);
        DeleteObject(hPenLaser);
        DeleteObject(hPenLaserCore);
    }
    
    // Draw Asteroids
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        Asteroid* ast = &g_state.asteroids[i];
        if (!ast->active) continue;
        
        int ax = cx + (int)(ast->x - g_state.shipX);
        int ay = cyCenter + (int)(ast->y - g_state.shipY);
        
        if (ax < viewportX - 100 || ax > viewportX + viewportW + 100 ||
            ay < viewportY - 100 || ay > viewportY + viewportH + 100) continue;
        
        POINT pts[12];
        for (int v = 0; v < ast->numVerts; v++) {
            float ca = ast->rot + ast->verts[v].a;
            pts[v].x = ax + (int)(cos(ca) * ast->verts[v].r);
            pts[v].y = ay + (int)(sin(ca) * ast->verts[v].r);
        }
        
        int isTarget = (g_state.selectedAstIndex == i);
        HPEN hPenAst = CreatePen(PS_SOLID, isTarget ? 2 : 1, isTarget ? pal->vector : pal->vectorDim);
        HBRUSH hBrAst = CreateSolidBrush(pal->bgPanel);
        HGDIOBJ oldAstPen = SelectObject(hdc, hPenAst);
        HGDIOBJ oldAstBr = SelectObject(hdc, hBrAst);
        
        Polygon(hdc, pts, ast->numVerts);
        
        // Ore node center
        HBRUSH hBrOreNode = CreateSolidBrush(ORE_DEFS[ast->oreType].color);
        RECT rcNode = { ax - 3, ay - 3, ax + 4, ay + 4 };
        FillRect(hdc, &rcNode, hBrOreNode);
        DeleteObject(hBrOreNode);
        
        SelectObject(hdc, oldAstPen);
        SelectObject(hdc, oldAstBr);
        DeleteObject(hPenAst);
        DeleteObject(hBrAst);
        
        // Asteroid Label
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(148, 163, 184));
        char lbl[32];
        sprintf(lbl, "%s [%s]", ast->id, ORE_DEFS[ast->oreType].name);
        RECT rcLbl = { ax - 60, ay - (int)ast->radius - 14, ax + 60, ay - (int)ast->radius };
        DrawTextA(hdc, lbl, -1, &rcLbl, DT_CENTER | DT_SINGLELINE);
        
        if (isTarget) {
            HPEN hPenTarget = CreatePen(PS_DOT, 1, pal->vector);
            HGDIOBJ oldTPen = SelectObject(hdc, hPenTarget);
            HGDIOBJ oldTBr = SelectObject(hdc, GetStockObject(NULL_BRUSH));
            int bSize = (int)ast->radius + 8;
            Rectangle(hdc, ax - bSize, ay - bSize, ax + bSize, ay + bSize);
            SelectObject(hdc, oldTPen);
            SelectObject(hdc, oldTBr);
            DeleteObject(hPenTarget);
        }
    }
    
    // Draw Floating Ore Chunks
    for (int i = 0; i < MAX_ORE_CHUNKS; i++) {
        OreChunk* chunk = &g_state.oreChunks[i];
        if (!chunk->active) continue;
        
        int ox = cx + (int)(chunk->x - g_state.shipX);
        int oy = cyCenter + (int)(chunk->y - g_state.shipY);
        if (ox < viewportX || ox > viewportX + viewportW || oy < viewportY || oy > viewportY + viewportH) continue;
        
        POINT ptsChunk[4] = {
            { ox, oy - 4 },
            { ox + 4, oy },
            { ox, oy + 4 },
            { ox - 4, oy }
        };
        HBRUSH hBrChunk = CreateSolidBrush(ORE_DEFS[chunk->oreType].color);
        HPEN hPenChunk = CreatePen(PS_SOLID, 1, pal->laserCore);
        HGDIOBJ oldChPen = SelectObject(hdc, hPenChunk);
        HGDIOBJ oldChBr = SelectObject(hdc, hBrChunk);
        
        Polygon(hdc, ptsChunk, 4);
        
        SelectObject(hdc, oldChPen);
        SelectObject(hdc, oldChBr);
        DeleteObject(hPenChunk);
        DeleteObject(hBrChunk);
    }
    
    // Draw Particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!g_state.particles[i].active) continue;
        int px = cx + (int)(g_state.particles[i].x - g_state.shipX);
        int py = cyCenter + (int)(g_state.particles[i].y - g_state.shipY);
        if (px >= viewportX && px < viewportX + viewportW && py >= viewportY && py < viewportY + viewportH) {
            SetPixel(hdc, px, py, g_state.particles[i].color);
            SetPixel(hdc, px + 1, py, g_state.particles[i].color);
        }
    }
    
    // Draw Floating Texts
    SelectObject(hdc, g_fontMonoBold);
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        if (!g_state.texts[i].active) continue;
        int tx = cx + (int)(g_state.texts[i].x - g_state.shipX);
        int ty = cyCenter + (int)(g_state.texts[i].y - g_state.shipY);
        if (tx >= viewportX && tx < viewportX + viewportW && ty >= viewportY && ty < viewportY + viewportH) {
            SetTextColor(hdc, g_state.texts[i].color);
            RECT rcTxt = { tx - 60, ty - 8, tx + 60, ty + 8 };
            DrawTextA(hdc, g_state.texts[i].text, -1, &rcTxt, DT_CENTER | DT_SINGLELINE);
        }
    }
    
    // Draw Dredge Ship (Cockpit Center)
    if (g_state.shield > 0.0f) {
        HPEN hPenShield = CreatePen(PS_SOLID, 1, pal->vectorDim);
        HGDIOBJ oldShPen = SelectObject(hdc, hPenShield);
        HGDIOBJ oldShBr = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Ellipse(hdc, cx - 28, cyCenter - 28, cx + 28, cyCenter + 28);
        SelectObject(hdc, oldShPen);
        SelectObject(hdc, oldShBr);
        DeleteObject(hPenShield);
    }
    
    // Rotate Ship Vertices
    POINT shipLocal[6] = {
        { 22, 0 },
        { -8, -16 },
        { -20, -12 },
        { -16, 0 },
        { -20, 12 },
        { -8, 16 }
    };
    POINT shipWorld[6];
    float cosA = (float)cos(g_state.shipAngle);
    float sinA = (float)sin(g_state.shipAngle);
    for (int i = 0; i < 6; i++) {
        shipWorld[i].x = cx + (int)(shipLocal[i].x * cosA - shipLocal[i].y * sinA);
        shipWorld[i].y = cyCenter + (int)(shipLocal[i].x * sinA + shipLocal[i].y * cosA);
    }
    
    HPEN hPenShip = CreatePen(PS_SOLID, 2, pal->vector);
    HBRUSH hBrShip = CreateSolidBrush(pal->bgPanel);
    HGDIOBJ oldSpPen = SelectObject(hdc, hPenShip);
    HGDIOBJ oldSpBr = SelectObject(hdc, hBrShip);
    Polygon(hdc, shipWorld, 6);
    SelectObject(hdc, oldSpPen);
    SelectObject(hdc, oldSpBr);
    DeleteObject(hPenShip);
    DeleteObject(hBrShip);
    
    // Center Cockpit Glass
    HBRUSH hBrGlass = CreateSolidBrush(pal->vectorDim);
    int gx = cx + (int)(4.0f * cosA);
    int gy = cyCenter + (int)(4.0f * sinA);
    RECT rcGlass = { gx - 3, gy - 3, gx + 4, gy + 4 };
    FillRect(hdc, &rcGlass, hBrGlass);
    DeleteObject(hBrGlass);
    
    // HUD Telemetry Overlays
    SelectObject(hdc, g_fontSmall);
    SetTextColor(hdc, pal->vector);
    TextOutA(hdc, viewportX + 10, viewportY + 10, "RADAR SCANNER: ACTIVE (360)", 27);
    float spd = (float)sqrt(g_state.shipVx * g_state.shipVx + g_state.shipVy * g_state.shipVy);
    char hudBuf[64];
    sprintf(hudBuf, "DRIFT VELOCITY: %.2f km/s", spd);
    TextOutA(hdc, viewportX + 10, viewportY + 24, hudBuf, (int)strlen(hudBuf));
    sprintf(hudBuf, "SECTOR POS: X: %.1f | Y: %.1f", g_state.shipX * 0.1f, g_state.shipY * 0.1f);
    TextOutA(hdc, viewportX + 10, viewportY + 38, hudBuf, (int)strlen(hudBuf));
    
    // Radar Scope in Top Right of Viewport
    int radarR = 55;
    int rcRadarX = viewportX + viewportW - radarR - 15;
    int rcRadarY = viewportY + radarR + 15;
    
    HBRUSH hBrRadar = CreateSolidBrush(pal->radarBg);
    HPEN hPenRadar = CreatePen(PS_SOLID, 1, pal->radarGrid);
    HGDIOBJ oldRdPen = SelectObject(hdc, hPenRadar);
    HGDIOBJ oldRdBr = SelectObject(hdc, hBrRadar);
    Ellipse(hdc, rcRadarX - radarR, rcRadarY - radarR, rcRadarX + radarR, rcRadarY + radarR);
    
    // Radar Range Rings
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Ellipse(hdc, rcRadarX - radarR / 2, rcRadarY - radarR / 2, rcRadarX + radarR / 2, rcRadarY + radarR / 2);
    
    // Radar Sweep Line
    HPEN hPenSweep = CreatePen(PS_SOLID, 1, pal->radarSweep);
    SelectObject(hdc, hPenSweep);
    MoveToEx(hdc, rcRadarX, rcRadarY, NULL);
    LineTo(hdc, rcRadarX + (int)(cos(g_state.radarAngle) * radarR), rcRadarY + (int)(sin(g_state.radarAngle) * radarR));
    DeleteObject(hPenSweep);
    
    // Radar Blips for Asteroids
    float radarScale = 0.07f;
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!g_state.asteroids[i].active) continue;
        float bdx = (g_state.asteroids[i].x - g_state.shipX) * radarScale;
        float bdy = (g_state.asteroids[i].y - g_state.shipY) * radarScale;
        if (bdx * bdx + bdy * bdy < (radarR - 4) * (radarR - 4)) {
            COLORREF blipCol = (i == g_state.selectedAstIndex) ? pal->radarTarget : RGB(245, 158, 11);
            SetPixel(hdc, rcRadarX + (int)bdx, rcRadarY + (int)bdy, blipCol);
            SetPixel(hdc, rcRadarX + (int)bdx + 1, rcRadarY + (int)bdy, blipCol);
        }
    }
    
    // Center Ship Blip on Radar
    SetPixel(hdc, rcRadarX, rcRadarY, pal->vector);
    SetPixel(hdc, rcRadarX + 1, rcRadarY, pal->vector);
    
    SelectObject(hdc, oldRdPen);
    SelectObject(hdc, oldRdBr);
    DeleteObject(hPenRadar);
    DeleteObject(hBrRadar);
    
    // Target Lock Reticle Box on Bottom-Right of Viewport
    if (g_state.selectedAstIndex >= 0 && g_state.asteroids[g_state.selectedAstIndex].active) {
        Asteroid* target = &g_state.asteroids[g_state.selectedAstIndex];
        int tBoxW = 200;
        int tBoxH = 48;
        int tBoxX = viewportX + viewportW - tBoxW - 15;
        int tBoxY = viewportY + viewportH - tBoxH - 15;
        
        RECT rcTBox = { tBoxX, tBoxY, tBoxX + tBoxW, tBoxY + tBoxH };
        HBRUSH hBrTBox = CreateSolidBrush(pal->bgPanel);
        FillRect(hdc, &rcTBox, hBrTBox);
        DeleteObject(hBrTBox);
        FrameRect(hdc, &rcTBox, (HBRUSH)GetStockObject(WHITE_BRUSH));
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, pal->vector);
        TextOutA(hdc, tBoxX + 6, tBoxY + 4, target->id, (int)strlen(target->id));
        
        float dist = (float)sqrt((target->x - g_state.shipX) * (target->x - g_state.shipX) +
                                 (target->y - g_state.shipY) * (target->y - g_state.shipY));
        char distBuf[32];
        sprintf(distBuf, "%d m", (int)dist);
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(148, 163, 184));
        RECT rcDist = { tBoxX + 100, tBoxY + 4, tBoxX + tBoxW - 6, tBoxY + 20 };
        DrawTextA(hdc, distBuf, -1, &rcDist, DT_RIGHT | DT_SINGLELINE);
        
        char oreStr[64];
        sprintf(oreStr, "ORE: %s (%d%%)", ORE_DEFS[target->oreType].name, target->richness);
        SetTextColor(hdc, pal->textBright);
        TextOutA(hdc, tBoxX + 6, tBoxY + 18, oreStr, (int)strlen(oreStr));
        
        DrawBar(hdc, tBoxX + 6, tBoxY + 34, tBoxW - 12, 6, target->hp / target->maxHp, pal->vector, RGB(2, 6, 23), pal->borderPanel);
    }
    
    // Draw CRT Scanlines over Viewport
    DrawScanlines(hdc, viewportX, viewportY, viewportW, viewportH, g_state.scanlineMode);
    
    // Restore clipping
    SelectClipRgn(hdc, NULL);
    DeleteObject(hRgnClip);
    
    // 5. Bottom Panel: Cockpit Controls & Event Terminal
    int botY = totalH - bottomCtrlH;
    RECT rcBottom = { 0, botY, totalW, totalH };
    HBRUSH hBrBot = CreateSolidBrush(pal->bgPanel);
    FillRect(hdc, &rcBottom, hBrBot);
    DeleteObject(hBrBot);
    
    MoveToEx(hdc, 0, botY, NULL);
    LineTo(hdc, totalW, botY);
    
    // Control Section Header
    RECT rcCtrlHdr = { 0, botY, 350, botY + 20 };
    FillRect(hdc, &rcCtrlHdr, hBrSubHdr);
    SelectObject(hdc, g_fontMonoBold);
    SetTextColor(hdc, pal->textPrimary);
    TextOutA(hdc, 10, botY + 3, "COCKPIT CONSOLE  [WASD / SPACE / T / Z / V / C]", 46);
    
    // Terminal Section Header
    RECT rcTermHdr = { 350, botY, totalW, botY + 20 };
    FillRect(hdc, &rcTermHdr, hBrSubHdr);
    MoveToEx(hdc, 350, botY, NULL);
    LineTo(hdc, 350, totalH);
    TextOutA(hdc, 360, botY + 3, "FLIGHT & DREDGE TERMINAL", 24);
    
    // Draw Terminal Event Logs
    int logBoxX = 360;
    int logBoxY = botY + 25;
    int logBoxW = totalW - logBoxX - 10;
    int logBoxH = bottomCtrlH - 35;
    
    RECT rcLogBox = { logBoxX, logBoxY, logBoxX + logBoxW, logBoxY + logBoxH };
    HBRUSH hBrLog = CreateSolidBrush(RGB(2, 5, 14));
    FillRect(hdc, &rcLogBox, hBrLog);
    DeleteObject(hBrLog);
    FrameRect(hdc, &rcLogBox, (HBRUSH)GetStockObject(WHITE_BRUSH));
    
    SelectObject(hdc, g_fontSmall);
    int visibleLogs = min(6, g_state.logCount);
    int startIdx = max(0, g_state.logCount - 6);
    int ly = logBoxY + 4;
    
    for (int i = startIdx; i < g_state.logCount; i++) {
        COLORREF logCol = RGB(148, 163, 184);
        if (g_state.logs[i].type == 0) logCol = pal->textPrimary;
        else if (g_state.logs[i].type == 1) logCol = pal->vector;
        else if (g_state.logs[i].type == 2) logCol = RGB(192, 132, 252);
        else if (g_state.logs[i].type == 3) logCol = RGB(245, 158, 11);
        else if (g_state.logs[i].type == 4) logCol = RGB(239, 68, 68);
        else if (g_state.logs[i].type == 5) logCol = RGB(16, 185, 129);
        else if (g_state.logs[i].type == 6) logCol = RGB(244, 63, 94);
        
        SetTextColor(hdc, logCol);
        TextOutA(hdc, logBoxX + 6, ly, g_state.logs[i].text, (int)strlen(g_state.logs[i].text));
        ly += 16;
    }
    
    // Sector Star Chart Navigation Modal
    if (g_state.showStarChart) {
        int modalW = 660;
        int modalH = 420;
        int mx = (totalW - modalW) / 2;
        int my = (totalH - modalH) / 2;
        
        // Background
        RECT rcModal = { mx, my, mx + modalW, my + modalH };
        HBRUSH hBrModal = CreateSolidBrush(pal->bgPanel);
        FillRect(hdc, &rcModal, hBrModal);
        DeleteObject(hBrModal);
        
        HPEN hPenBorder2 = CreatePen(PS_SOLID, 2, pal->vector);
        HGDIOBJ oldPen2 = SelectObject(hdc, hPenBorder2);
        HGDIOBJ oldBrush2 = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, mx, my, mx + modalW, my + modalH);
        
        // Header
        RECT rcHdr = { mx, my, mx + modalW, my + 30 };
        HBRUSH hBrHdr = CreateSolidBrush(pal->bgHeader);
        FillRect(hdc, &rcHdr, hBrHdr);
        DeleteObject(hBrHdr);
        
        SelectObject(hdc, g_fontHeader);
        SetTextColor(hdc, pal->vector);
        TextOutA(hdc, mx + 14, my + 6, "ASTRONAVIGATION & STAR SECTOR CHARTS", 36);
        
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, pal->textBright);
        TextOutA(hdc, mx + modalW - 120, my + 9, "[N / ESC] CLOSE", 15);
        
        // Subtitle
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(148, 163, 184));
        TextOutA(hdc, mx + 14, my + 36, "Select an asteroid sector (Keys 1-4 or Click) to initiate sub-space warp jump:", 77);
        
        // 2x2 Sector Cards
        int cardW = 305;
        int cardH = 135;
        int gapX = 14;
        int gapY = 10;
        int startX = mx + 14;
        int startY = my + 54;
        
        for (int i = 0; i < 4; i++) {
            int row = i / 2;
            int col = i % 2;
            int scx = startX + col * (cardW + gapX);
            int scy = startY + row * (cardH + gapY);
            
            const SectorDef* sec = &SECTOR_DEFS[i];
            int isCurrent = (i == g_state.currentSectorIndex);
            int isSelected = (i == g_state.selectedSectorIndex);
            
            RECT rcCard = { scx, scy, scx + cardW, scy + cardH };
            HBRUSH hBrCard = CreateSolidBrush(isSelected ? RGB(6, 64, 91) : (isCurrent ? RGB(6, 43, 16) : RGB(3, 7, 18)));
            FillRect(hdc, &rcCard, hBrCard);
            DeleteObject(hBrCard);
            
            HPEN hPenCard = CreatePen(PS_SOLID, isSelected ? 2 : 1, isSelected ? pal->vector : (isCurrent ? RGB(16, 185, 129) : pal->borderPanel));
            SelectObject(hdc, hPenCard);
            Rectangle(hdc, scx, scy, scx + cardW, scy + cardH);
            DeleteObject(hPenCard);
            
            // Sector Name & Shortcut
            char titleBuf[64];
            sprintf(titleBuf, "[%d] %s", i + 1, sec->name);
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, isSelected ? pal->vector : pal->textBright);
            TextOutA(hdc, scx + 8, scy + 6, titleBuf, (int)strlen(titleBuf));
            
            // Hazard Badge
            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, sec->hazardColor);
            TextOutA(hdc, scx + cardW - 100, scy + 6, sec->hazard, (int)strlen(sec->hazard));
            
            if (isCurrent) {
                SetTextColor(hdc, RGB(110, 231, 183));
                TextOutA(hdc, scx + 8, scy + 22, "* CURRENT LOCATION", 18);
            } else {
                SetTextColor(hdc, pal->textDim);
                TextOutA(hdc, scx + 8, scy + 22, sec->type, (int)strlen(sec->type));
            }
            
            // Description
            SetTextColor(hdc, RGB(226, 232, 240));
            RECT rcDesc = { scx + 8, scy + 38, scx + cardW - 8, scy + 80 };
            DrawTextA(hdc, sec->desc, -1, &rcDesc, DT_WORDBREAK);
            
            // Resources & Specs
            SetTextColor(hdc, RGB(245, 158, 11));
            char specBuf[64];
            sprintf(specBuf, "WARP COST: %d%% FUEL   DENSITY: %d ASTEROIDS", sec->fuelCost, sec->density);
            TextOutA(hdc, scx + 8, scy + cardH - 20, specBuf, (int)strlen(specBuf));
        }
        
        // Bottom Warp Action Bar
        int barY = my + modalH - 58;
        RECT rcBar = { mx + 14, barY, mx + modalW - 14, my + modalH - 12 };
        HBRUSH hBrBar = CreateSolidBrush(RGB(2, 5, 14));
        FillRect(hdc, &rcBar, hBrBar);
        DeleteObject(hBrBar);
        FrameRect(hdc, &rcBar, (HBRUSH)GetStockObject(WHITE_BRUSH));
        
        const SectorDef* selSec = &SECTOR_DEFS[g_state.selectedSectorIndex];
        int isCur = (g_state.selectedSectorIndex == g_state.currentSectorIndex);
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, pal->vector);
        char selBuf[64];
        sprintf(selBuf, "SELECTED: %s", selSec->name);
        TextOutA(hdc, mx + 24, barY + 8, selBuf, (int)strlen(selBuf));
        
        SelectObject(hdc, g_fontSmall);
        char fReqBuf[64];
        sprintf(fReqBuf, "WARP FUEL: %d%%  (AVAILABLE: %d%%)", isCur ? 0 : selSec->fuelCost, (int)g_state.fuel);
        SetTextColor(hdc, (g_state.fuel >= selSec->fuelCost || isCur) ? RGB(16, 185, 129) : RGB(239, 68, 68));
        TextOutA(hdc, mx + 24, barY + 26, fReqBuf, (int)strlen(fReqBuf));
        
        // Jump Button box
        int jBtnW = 170;
        int jBtnH = 32;
        int jBtnX = mx + modalW - 14 - jBtnW - 8;
        int jBtnY = barY + 7;
        RECT rcJBtn = { jBtnX, jBtnY, jBtnX + jBtnW, jBtnY + jBtnH };
        
        HBRUSH hBrJBtn = CreateSolidBrush(isCur ? RGB(30, 41, 59) : (g_state.fuel >= selSec->fuelCost ? RGB(131, 24, 67) : RGB(50, 15, 20)));
        FillRect(hdc, &rcJBtn, hBrJBtn);
        DeleteObject(hBrJBtn);
        FrameRect(hdc, &rcJBtn, (HBRUSH)GetStockObject(WHITE_BRUSH));
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, isCur ? RGB(148, 163, 184) : RGB(255, 255, 255));
        const char* jTxt = isCur ? "CURRENT LOCATION" : (g_state.fuel >= selSec->fuelCost ? "ENGAGE WARP [ENTER]" : "LOW FUEL");
        DrawTextA(hdc, jTxt, -1, &rcJBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        SelectObject(hdc, oldPen2);
        SelectObject(hdc, oldBrush2);
        DeleteObject(hPenBorder2);
    }
    
    // Help Overlay Modal
    if (g_state.showHelp) {
        int helpW = 560;
        int helpH = 380;
        int hx = (totalW - helpW) / 2;
        int hy = (totalH - helpH) / 2;
        
        RECT rcHelp = { hx, hy, hx + helpW, hy + helpH };
        HBRUSH hBrHModal = CreateSolidBrush(pal->bgPanel);
        FillRect(hdc, &rcHelp, hBrHModal);
        DeleteObject(hBrHModal);
        FrameRect(hdc, &rcHelp, (HBRUSH)GetStockObject(WHITE_BRUSH));
        
        RECT rcHelpHeader = { hx, hy, hx + helpW, hy + 28 };
        FillRect(hdc, &rcHelpHeader, hBrSubHdr);
        SelectObject(hdc, g_fontHeader);
        SetTextColor(hdc, pal->vector);
        TextOutA(hdc, hx + 12, hy + 6, "KStarDredge - Captain's Flight Manual", 37);
        
        SelectObject(hdc, g_fontMonoBold);
        int myHelp = hy + 36;
        SetTextColor(hdc, pal->textBright);
        TextOutA(hdc, hx + 16, myHelp, "FLIGHT CONTROLS & DREDGING TACTICS:", 35);
        myHelp += 20;
        
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(148, 163, 184));
        TextOutA(hdc, hx + 20, myHelp, "• [W / UP ARROW]: Engage Forward Fusion Thrusters (Consumes Fuel)", 64); myHelp += 17;
        TextOutA(hdc, hx + 20, myHelp, "• [S / DOWN ARROW]: Engage Retro Braking Thrusters", 50); myHelp += 17;
        TextOutA(hdc, hx + 20, myHelp, "• [A / D / LEFT / RIGHT]: Pivot Barge Heading", 45); myHelp += 17;
        TextOutA(hdc, hx + 20, myHelp, "• [SPACEBAR / LASER BTN]: Fire Mining Laser (Watch Laser Heat)", 62); myHelp += 17;
        TextOutA(hdc, hx + 20, myHelp, "• [T / TRACTOR BTN]: Toggle Tractor Magnet to draw floating mineral chunks", 73); myHelp += 17;
        TextOutA(hdc, hx + 20, myHelp, "• [Z / DAMPENER BTN]: Toggle Inertial Dampeners for precise stationkeeping", 74); myHelp += 17;
        TextOutA(hdc, hx + 20, myHelp, "• [N / SECTORS BTN]: Open Star Sector Chart & Engage Sub-space Warp Jumps", 72); myHelp += 17;
        TextOutA(hdc, hx + 20, myHelp, "• [V / THEME BTN]: Cycle Retro CRT Vector Theme (Cyan / Amber / Green / Solar)", 77); myHelp += 17;
        TextOutA(hdc, hx + 20, myHelp, "• [C / SCANLINES BTN]: Toggle CRT Scanlines & Shaders (Off / On / CRT+)", 70); myHelp += 17;
        TextOutA(hdc, hx + 20, myHelp, "• [CLICK VIEWPORT]: Target & Lock asteroid with telemetry computer", 66); myHelp += 17;
        TextOutA(hdc, hx + 20, myHelp, "• [LIQUIDATE]: Sell cargo hold to orbital comm-link for Credits", 62); myHelp += 20;
        
        SetTextColor(hdc, RGB(245, 158, 11));
        TextOutA(hdc, hx + 20, myHelp, "Press [H] or Click 'MANUAL' to close this screen.", 49);
    }
    
    SelectObject(hdc, oldPen);
    DeleteObject(hPenBorder);
    DeleteObject(hBrPanel);
    DeleteObject(hBrSubHdr);
}

void RepositionControls(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int totalW = rc.right - rc.left;
    int totalH = rc.bottom - rc.top;
    if (totalW <= 0 || totalH <= 0) return;
    
    int rightPanelW = 240;
    int bottomCtrlH = 140;
    int botY = totalH - bottomCtrlH;
    
    // Cockpit Action Buttons in bottom-left console (5 per row on top, 4 on bottom)
    int bx = 10;
    int by1 = botY + 28;
    int by2 = botY + 62;
    int bw = 64;
    int bh = 28;
    int gap = 5;
    
    MoveWindow(g_btnLaser,      bx,                  by1, bw, bh, TRUE);
    MoveWindow(g_btnTractor,    bx + (bw + gap),     by1, bw, bh, TRUE);
    MoveWindow(g_btnDampener,   bx + (bw + gap) * 2, by1, bw, bh, TRUE);
    MoveWindow(g_btnScan,       bx + (bw + gap) * 3, by1, bw, bh, TRUE);
    MoveWindow(g_btnNav,        bx + (bw + gap) * 4, by1, bw, bh, TRUE);
    
    MoveWindow(g_btnTheme,      bx,                  by2, bw, bh, TRUE);
    MoveWindow(g_btnScanlines,  bx + (bw + gap),     by2, bw, bh, TRUE);
    MoveWindow(g_btnAudio,      bx + (bw + gap) * 2, by2, bw, bh, TRUE);
    MoveWindow(g_btnHelp,       bx + (bw + gap) * 3, by2, bw, bh, TRUE);
    
    // Right panel buttons: Jettison & Liquidate
    int rightX = totalW - rightPanelW + 10;
    int rightY = totalH - bottomCtrlH - 36;
    int rightBtnW = (rightPanelW - 26) / 2;
    
    MoveWindow(g_btnJettison, rightX,                 rightY, rightBtnW, 26, TRUE);
    MoveWindow(g_btnSell,     rightX + rightBtnW + 6, rightY, rightBtnW, 26, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_hwnd = hwnd;
            g_fontMono = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            g_fontMonoBold = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            g_fontSmall = CreateFontA(11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            g_fontHeader = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            
            InitGame();
            
            // Create Control Buttons
            g_btnLaser     = CreateWindowA("BUTTON", "LASER [SPC]",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_LASER, NULL, NULL);
            g_btnTractor   = CreateWindowA("BUTTON", "TRACTOR [T]",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_TRACTOR, NULL, NULL);
            g_btnDampener  = CreateWindowA("BUTTON", "DAMPENER [Z]",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_DAMPENER, NULL, NULL);
            g_btnScan      = CreateWindowA("BUTTON", "SCAN [S]",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SCAN, NULL, NULL);
            g_btnNav       = CreateWindowA("BUTTON", "SECTORS [N]",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_NAV, NULL, NULL);
            g_btnTheme     = CreateWindowA("BUTTON", "CRT CYAN",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_THEME, NULL, NULL);
            g_btnScanlines = CreateWindowA("BUTTON", "SCAN: ON",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SCANLINES, NULL, NULL);
            g_btnAudio     = CreateWindowA("BUTTON", "AUDIO [M]",     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_AUDIO, NULL, NULL);
            g_btnHelp      = CreateWindowA("BUTTON", "MANUAL [H]",    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);
            
            g_btnJettison  = CreateWindowA("BUTTON", "JETTISON",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_JETTISON, NULL, NULL);
            g_btnSell      = CreateWindowA("BUTTON", "LIQUIDATE",     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SELL, NULL, NULL);
            
            SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL, NULL);
            
            // Audio Thread
            g_hSoundThread = CreateThread(NULL, 0, SoundThreadProc, NULL, 0, NULL);
            return 0;
        }
        
        case WM_SIZE: {
            RepositionControls(hwnd);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case ID_BTN_LASER:
                    g_state.miningActive = !g_state.miningActive;
                    AddLog(g_state.miningActive ? "Mining laser firing." : "Mining laser paused.", 1);
                    break;
                case ID_BTN_TRACTOR:
                    g_state.tractorActive = !g_state.tractorActive;
                    AddLog(g_state.tractorActive ? "Tractor beam engaged." : "Tractor beam standby.", 0);
                    break;
                case ID_BTN_DAMPENER:
                    g_state.dampeners = !g_state.dampeners;
                    AddLog(g_state.dampeners ? "Inertia dampeners engaged." : "Inertia dampeners disengaged.", 0);
                    break;
                case ID_BTN_SCAN:
                    TriggerSound(SFX_BEEP);
                    AddLog("Deep sweep complete: 24 mineral asteroids registered in local sector.", 0);
                    break;
                case ID_BTN_NAV:
                    g_state.showStarChart = !g_state.showStarChart;
                    TriggerSound(SFX_BEEP);
                    break;
                case ID_BTN_THEME:
                    CycleTheme();
                    break;
                case ID_BTN_SCANLINES:
                    CycleScanlines();
                    break;
                case ID_BTN_AUDIO:
                    g_state.soundEnabled = !g_state.soundEnabled;
                    AddLog(g_state.soundEnabled ? "Audio synthesizer unmuted." : "Audio synthesizer muted.", 0);
                    break;
                case ID_BTN_HELP:
                    g_state.showHelp = !g_state.showHelp;
                    break;
                case ID_BTN_JETTISON:
                    UpdateCargoTotal();
                    if (g_state.totalCargo == 0) {
                        AddLog("Cargo hold is already empty.", 3);
                    } else {
                        for (int i = 0; i < 6; i++) g_state.cargoHold[i] = 0;
                        UpdateCargoTotal();
                        AddLog("Cargo hold purged into void space.", 4);
                    }
                    break;
                case ID_BTN_SELL: {
                    int val = CalculateCargoValue();
                    if (val <= 0) {
                        AddLog("No minerals in hold to liquidate.", 3);
                    } else {
                        g_state.credits += val;
                        char fTxt[32];
                        sprintf(fTxt, "+%d CR", val);
                        AddFloatingText(fTxt, g_state.shipX, g_state.shipY - 30.0f, RGB(245, 158, 11));
                        TriggerSound(SFX_COLLECT);
                        for (int i = 0; i < 6; i++) g_state.cargoHold[i] = 0;
                        UpdateCargoTotal();
                        char logB[64];
                        sprintf(logB, "Transferred minerals to station comm-link for %d CR.", val);
                        AddLog(logB, 5);
                    }
                    break;
                }
            }
            SetFocus(hwnd);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        
        case WM_LBUTTONDOWN: {
            int mx = GET_X_LPARAM(lParam);
            int my = GET_Y_LPARAM(lParam);
            
            RECT rc;
            GetClientRect(hwnd, &rc);
            int topHeaderH = 34;
            int bottomCtrlH = 140;
            int leftPanelW = 220;
            int rightPanelW = 240;
            int totalW = rc.right - rc.left;
            int totalH = rc.bottom - rc.top;
            
            if (g_state.showHelp) {
                g_state.showHelp = 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            
            if (g_state.showStarChart) {
                int modalW = 660;
                int modalH = 420;
                int hx = (totalW - modalW) / 2;
                int hy = (totalH - modalH) / 2;
                
                // Check if click close
                if (mx >= hx + modalW - 130 && mx <= hx + modalW - 10 && my >= hy && my <= hy + 30) {
                    g_state.showStarChart = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                
                // Check if click 4 sector cards
                int cardW = 305;
                int cardH = 135;
                int gapX = 14;
                int gapY = 10;
                int startX = hx + 14;
                int startY = hy + 54;
                for (int i = 0; i < 4; i++) {
                    int row = i / 2;
                    int col = i % 2;
                    int scx = startX + col * (cardW + gapX);
                    int scy = startY + row * (cardH + gapY);
                    if (mx >= scx && mx <= scx + cardW && my >= scy && my <= scy + cardH) {
                        g_state.selectedSectorIndex = i;
                        TriggerSound(SFX_BEEP);
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }
                
                // Check if click jump button
                int barY = hy + modalH - 58;
                int jBtnW = 170;
                int jBtnH = 32;
                int jBtnX = hx + modalW - 14 - jBtnW - 8;
                int jBtnY = barY + 7;
                if (mx >= jBtnX && mx <= jBtnX + jBtnW && my >= jBtnY && my <= jBtnY + jBtnH) {
                    EngageWarpJump(g_state.selectedSectorIndex);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                
                // Click outside modal closes it
                if (mx < hx || mx > hx + modalW || my < hy || my > hy + modalH) {
                    g_state.showStarChart = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                return 0;
            }
            
            int viewportX = leftPanelW;
            int viewportW = totalW - leftPanelW - rightPanelW;
            int viewportY = topHeaderH;
            int viewportH = totalH - topHeaderH - bottomCtrlH;
            
            // Check click inside viewport for asteroid targeting
            if (mx >= viewportX && mx < viewportX + viewportW && my >= viewportY && my < viewportY + viewportH) {
                int cx = viewportX + (viewportW / 2);
                int cyCenter = viewportY + (viewportH / 2);
                float clickWorldX = g_state.shipX + (float)(mx - cx);
                float clickWorldY = g_state.shipY + (float)(my - cyCenter);
                
                int bestIdx = -1;
                float minDist = 80.0f;
                for (int i = 0; i < MAX_ASTEROIDS; i++) {
                    if (!g_state.asteroids[i].active) continue;
                    float d = (float)sqrt((g_state.asteroids[i].x - clickWorldX) * (g_state.asteroids[i].x - clickWorldX) +
                                          (g_state.asteroids[i].y - clickWorldY) * (g_state.asteroids[i].y - clickWorldY));
                    if (d < g_state.asteroids[i].radius + 15.0f && d < minDist) {
                        minDist = d;
                        bestIdx = i;
                    }
                }
                
                if (bestIdx >= 0) {
                    g_state.selectedAstIndex = bestIdx;
                    TriggerSound(SFX_BEEP);
                    char tLog[64];
                    sprintf(tLog, "Target locked: %s [%s]", g_state.asteroids[bestIdx].id, ORE_DEFS[g_state.asteroids[bestIdx].oreType].name);
                    AddLog(tLog, 0);
                }
            }
            return 0;
        }
        
        case WM_KEYDOWN: {
            if (g_state.showStarChart) {
                if (wParam == '1') { g_state.selectedSectorIndex = 0; TriggerSound(SFX_BEEP); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '2') { g_state.selectedSectorIndex = 1; TriggerSound(SFX_BEEP); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '3') { g_state.selectedSectorIndex = 2; TriggerSound(SFX_BEEP); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '4') { g_state.selectedSectorIndex = 3; TriggerSound(SFX_BEEP); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == VK_RETURN) { EngageWarpJump(g_state.selectedSectorIndex); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == 'N' || wParam == VK_ESCAPE) { g_state.showStarChart = 0; InvalidateRect(hwnd, NULL, FALSE); return 0; }
            }
            
            switch (wParam) {
                case 'W': case VK_UP:    g_state.thrusting = 1; break;
                case 'S': case VK_DOWN:  g_state.reversing = 1; break;
                case 'A': case VK_LEFT:  g_state.turningLeft = 1; break;
                case 'D': case VK_RIGHT: g_state.turningRight = 1; break;
                case VK_SPACE:
                    g_state.miningActive = 1;
                    break;
                case 'T':
                    g_state.tractorActive = !g_state.tractorActive;
                    AddLog(g_state.tractorActive ? "Tractor emitter magnetized." : "Tractor beam offline.", 0);
                    break;
                case 'Z':
                    g_state.dampeners = !g_state.dampeners;
                    AddLog(g_state.dampeners ? "Inertia dampeners engaged." : "Inertia dampeners disengaged.", 0);
                    break;
                case 'N':
                    g_state.showStarChart = !g_state.showStarChart;
                    TriggerSound(SFX_BEEP);
                    break;
                case 'V':
                    CycleTheme();
                    break;
                case 'C':
                    CycleScanlines();
                    break;
                case 'M':
                    g_state.soundEnabled = !g_state.soundEnabled;
                    AddLog(g_state.soundEnabled ? "Audio synthesizer unmuted." : "Audio synthesizer muted.", 0);
                    break;
                case 'H':
                    g_state.showHelp = !g_state.showHelp;
                    break;
            }
            return 0;
        }
        
        case WM_KEYUP: {
            switch (wParam) {
                case 'W': case VK_UP:    g_state.thrusting = 0; break;
                case 'S': case VK_DOWN:  g_state.reversing = 0; break;
                case 'A': case VK_LEFT:  g_state.turningLeft = 0; break;
                case 'D': case VK_RIGHT: g_state.turningRight = 0; break;
                case VK_SPACE:
                    g_state.miningActive = 0;
                    break;
            }
            return 0;
        }
        
        case WM_TIMER: {
            if (wParam == TIMER_ID) {
                UpdateGame(TIMER_INTERVAL / 1000.0f);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rc;
            GetClientRect(hwnd, &rc);
            int width = rc.right - rc.left;
            int height = rc.bottom - rc.top;
            
            // Double buffering
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBmp = CreateCompatibleBitmap(hdc, width, height);
            HGDIOBJ oldBmp = SelectObject(memDC, memBmp);
            
            RenderGame(memDC, &rc);
            
            BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);
            
            SelectObject(memDC, oldBmp);
            DeleteObject(memBmp);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_ERASEBKGND:
            return 1; // Handled by double buffer
            
        case WM_DESTROY: {
            KillTimer(hwnd, TIMER_ID);
            if (g_fontMono) DeleteObject(g_fontMono);
            if (g_fontMonoBold) DeleteObject(g_fontMonoBold);
            if (g_fontSmall) DeleteObject(g_fontSmall);
            if (g_fontHeader) DeleteObject(g_fontHeader);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KStarDredgeClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(0, "KStarDredgeClass", "KStarDredge - Orbital Salvage & Asteroid Dredger",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 960, 720, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}
