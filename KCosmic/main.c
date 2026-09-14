#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define M_PI 3.14159265358979323846

// --- UI Colors ---
#define COLOR_BG_DEEP       RGB(5, 8, 17)
#define COLOR_BG_PANEL      RGB(10, 17, 32)
#define COLOR_BG_PANEL_DARK RGB(7, 11, 22)
#define COLOR_BG_CARD       RGB(15, 28, 51)
#define COLOR_BG_CARD_HOV   RGB(22, 40, 74)
#define COLOR_BG_CARD_ACT   RGB(30, 56, 102)
#define COLOR_BORDER        RGB(28, 49, 86)
#define COLOR_CYAN          RGB(0, 240, 255)
#define COLOR_BLUE          RGB(56, 189, 248)
#define COLOR_EMERALD       RGB(16, 185, 129)
#define COLOR_AMBER         RGB(245, 158, 11)
#define COLOR_PURPLE        RGB(168, 85, 247)
#define COLOR_ROSE          RGB(244, 63, 94)
#define COLOR_TEXT_BRIGHT   RGB(248, 250, 252)
#define COLOR_TEXT_PRI      RGB(203, 213, 225)
#define COLOR_TEXT_DIM      RGB(100, 116, 139)
#define COLOR_ORANGE        RGB(255, 120, 50)

// --- Sound Effects ---
#define SFX_CLICK   1
#define SFX_SUCCESS 2
#define SFX_WARN    3
#define SFX_DEPLOY  4

static int g_soundEnabled = 1;

static DWORD WINAPI SoundThread(LPVOID lpParam) {
    int type = (int)(intptr_t)lpParam;
    if (!g_soundEnabled) return 0;
    switch (type) {
        case SFX_CLICK:
            Beep(440, 30);
            break;
        case SFX_SUCCESS:
            Beep(523, 50);
            Beep(659, 50);
            Beep(784, 80);
            break;
        case SFX_WARN:
            Beep(260, 90);
            Beep(220, 120);
            break;
        case SFX_DEPLOY:
            Beep(330, 40);
            Beep(587, 70);
            Beep(880, 90);
            break;
    }
    return 0;
}

static void PlaySoundFx(int type) {
    if (g_soundEnabled) {
        CreateThread(NULL, 0, SoundThread, (LPVOID)(intptr_t)type, 0, NULL);
    }
}

// --- Data Structures ---
#define STAR_COUNT 160
#define ASTEROID_COUNT 110

typedef struct {
    float x, y;
    float size;
    float alpha;
} Star;

typedef struct {
    float dist;
    float angle;
    float speed;
    float size;
    float alpha;
} Asteroid;

typedef struct {
    char id[16];
    char name[32];
    char type[32];
    float orbitRadius;
    float orbitSpeed;
    float angle;
    float radius;
    COLORREF color;
    float currX, currY;
    int isStar;
    int isPlanet;
    int isMoon;
    int isStation;
} CelestialBody;

typedef struct {
    char id[16];
    char name[32];
    char role[32];
    char status[32];
    char mission[48];
    int parentIndex; // 1 = Aethelgard, 2 = Boreas
    float orbitDist;
    float orbitSpeed;
    float angle;
    COLORREF color;
    int hull;
    int targetBelt;
    float x, y;
    float vx, vy;
    float currX, currY;
} Ship;

typedef struct {
    float cycle;
    int speed; // 0=paused, 1=1x, 2=2x, 5=5x
    int paused;
    float time;

    // Resources
    int energy;
    int minerals;
    int volatiles;
    int food;
    int colonists;
    int cryoSleepers;
    int housingCap;
    float morale;

    // Net Deltas
    int deltaEnergy;
    int deltaMinerals;
    int deltaVolatiles;
    int deltaFood;

    // Planetary Parameters
    float pressure;
    float temp;
    float water;
    float oxygen;
    float magnet;
    float habitability;

    // Active Facilities
    int solarMirrors;
    int atmoProcessors;
    int bioseedStations;
    int coreDynamos;
    int hydroTowers;
    int surfaceSolar;

    // Camera
    float camX, camY;
    float zoom;
    int isDragging;
    int lastMouseX, lastMouseY;

    // Selection
    int selectedType; // 0=none, 1=sun, 2=planet, 3=moon, 4=station, 5=ship
    int selectedIndex;

    // Tab: 0=Terraform, 1=Fleet, 2=Colony, 3=Economy
    int activeTab;

    // Log message
    char logMsg[128];
    int logIsWarn;
} Simulation;

static Simulation sim;
static Star stars[STAR_COUNT];
static Asteroid asteroids[ASTEROID_COUNT];
static CelestialBody bodies[4];
static Ship fleet[5];

static void SetLogMsg(const char* txt, int isWarn) {
    strncpy(sim.logMsg, txt, sizeof(sim.logMsg) - 1);
    sim.logMsg[sizeof(sim.logMsg) - 1] = '\0';
    sim.logIsWarn = isWarn;
    PlaySoundFx(isWarn ? SFX_WARN : SFX_SUCCESS);
}

// --- Simulation Logic ---
static void CalculateHabitability(void) {
    float p = sim.pressure;
    float pScore = 0.0f;
    if (p >= 0.2f && p <= 2.2f) {
        pScore = 100.0f - (float)fabs(1.0f - p) * 70.0f;
    }
    if (pScore < 0.0f) pScore = 0.0f;
    if (pScore > 100.0f) pScore = 100.0f;

    float t = sim.temp;
    float tScore = 0.0f;
    if (t >= -45.0f && t <= 55.0f) {
        tScore = 100.0f - (float)fabs(15.0f - t) * 1.6f;
    }
    if (tScore < 0.0f) tScore = 0.0f;
    if (tScore > 100.0f) tScore = 100.0f;

    float w = sim.water;
    float wScore = 0.0f;
    if (w >= 5.0f) {
        wScore = (w / 60.0f) * 100.0f;
        if (wScore > 100.0f) wScore = 100.0f;
    }

    float o = sim.oxygen;
    float oScore = (o / 20.9f) * 100.0f;
    if (oScore > 100.0f) oScore = 100.0f;

    float m = sim.magnet;
    float mScore = (m / 0.5f) * 100.0f;
    if (mScore > 100.0f) mScore = 100.0f;

    float total = (pScore * 0.25f) + (tScore * 0.25f) + (wScore * 0.20f) + (oScore * 0.20f) + (mScore * 0.10f);
    if (total < 0.0f) total = 0.0f;
    if (total > 100.0f) total = 100.0f;
    sim.habitability = total;
}

static void SimTick(void) {
    if (sim.paused || sim.speed <= 0) return;
    int rate = sim.speed;

    // Energy
    int energyGen = 600 + (sim.surfaceSolar * 60);
    int energyDrain = 200 + (sim.solarMirrors * 75) + (sim.atmoProcessors * 60) + (sim.coreDynamos * 80) + (sim.colonists / 1000) * 5;
    sim.deltaEnergy = energyGen - energyDrain;
    sim.energy += (int)(sim.deltaEnergy * 0.05f * rate);
    if (sim.energy < 0) sim.energy = 0;

    // Minerals
    int mineralGain = 20 + (strcmp(fleet[3].status, "Harvesting") == 0 || strcmp(fleet[3].status, "Mining Belt") == 0 ? 25 : 10);
    int mineralDrain = (sim.atmoProcessors * 3);
    sim.deltaMinerals = mineralGain - mineralDrain;
    sim.minerals += (int)(sim.deltaMinerals * 0.05f * rate);
    if (sim.minerals < 0) sim.minerals = 0;

    // Volatiles
    int volGain = 14 + (strcmp(fleet[3].status, "Scooping Ring") == 0 ? 18 : 6);
    int volDrain = (sim.solarMirrors * 2);
    sim.deltaVolatiles = volGain - volDrain;
    sim.volatiles += (int)(sim.deltaVolatiles * 0.05f * rate);
    if (sim.volatiles < 0) sim.volatiles = 0;

    // Food
    int foodGain = 15 + (sim.hydroTowers * 12);
    int foodDrain = sim.colonists / 2000;
    sim.deltaFood = foodGain - foodDrain;
    sim.food += (int)(sim.deltaFood * 0.05f * rate);
    if (sim.food < 0) sim.food = 0;

    // Slow planetary drifts
    if (sim.energy > 500) {
        sim.temp += (sim.solarMirrors * 0.012f * rate);
        sim.pressure += (sim.atmoProcessors * 0.0003f * rate);
        if (sim.temp > -20.0f && sim.water > 10.0f) {
            sim.oxygen += (sim.bioseedStations * 0.0008f * rate);
        }
        if (sim.temp > 0.0f && sim.water < 65.0f) {
            sim.water += (0.001f * rate);
        }
    }

    // Morale
    if (sim.food <= 100 || sim.energy <= 200) {
        sim.morale -= (0.2f * rate);
        if (sim.morale < 10.0f) sim.morale = 10.0f;
    } else if (sim.habitability > 25.0f && sim.morale < 95.0f) {
        sim.morale += (0.05f * rate);
        if (sim.morale > 99.0f) sim.morale = 99.0f;
    }

    CalculateHabitability();
}

static void InitSimulation(void) {
    srand(1337);
    sim.cycle = 1.00f;
    sim.speed = 1;
    sim.paused = 0;
    sim.time = 0.0f;

    sim.energy = 14250;
    sim.minerals = 8400;
    sim.volatiles = 2150;
    sim.food = 5800;
    sim.colonists = 25000;
    sim.cryoSleepers = 75000;
    sim.housingCap = 30000;
    sim.morale = 86.0f;

    sim.deltaEnergy = 120;
    sim.deltaMinerals = 15;
    sim.deltaVolatiles = 10;
    sim.deltaFood = 25;

    sim.pressure = 0.32f;
    sim.temp = -48.0f;
    sim.water = 12.8f;
    sim.oxygen = 3.1f;
    sim.magnet = 0.18f;
    sim.habitability = 18.4f;

    sim.solarMirrors = 2;
    sim.atmoProcessors = 3;
    sim.bioseedStations = 1;
    sim.coreDynamos = 1;
    sim.hydroTowers = 2;
    sim.surfaceSolar = 3;

    sim.camX = 0;
    sim.camY = 0;
    sim.zoom = 1.0f;
    sim.isDragging = 0;

    sim.selectedType = 0;
    sim.selectedIndex = -1;
    sim.activeTab = 0;

    strcpy(sim.logMsg, "KCosmic Win32 telemetry & simulation engine operational.");
    sim.logIsWarn = 0;

    // Generate Stars
    for (int i = 0; i < STAR_COUNT; i++) {
        stars[i].x = ((float)(rand() % 4000) - 2000.0f);
        stars[i].y = ((float)(rand() % 4000) - 2000.0f);
        stars[i].size = ((float)(rand() % 15) / 10.0f) + 0.5f;
        stars[i].alpha = ((float)(rand() % 70) / 100.0f) + 0.3f;
    }

    // Generate Asteroids (Tartarus belt)
    for (int i = 0; i < ASTEROID_COUNT; i++) {
        asteroids[i].dist = 520.0f + (float)(rand() % 100);
        asteroids[i].angle = ((float)rand() / (float)RAND_MAX) * (float)M_PI * 2.0f;
        asteroids[i].speed = 0.012f + ((float)(rand() % 15) / 1000.0f);
        asteroids[i].size = ((float)(rand() % 20) / 10.0f) + 1.0f;
        asteroids[i].alpha = ((float)(rand() % 60) / 100.0f) + 0.4f;
    }

    // Celestial Bodies
    // 0: Sun
    strcpy(bodies[0].id, "sun");
    strcpy(bodies[0].name, "Kepler-186 Helios");
    strcpy(bodies[0].type, "Red Dwarf Star");
    bodies[0].orbitRadius = 0;
    bodies[0].orbitSpeed = 0;
    bodies[0].angle = 0;
    bodies[0].radius = 48.0f;
    bodies[0].color = COLOR_ORANGE;
    bodies[0].isStar = 1;

    // 1: Aethelgard Prime
    strcpy(bodies[1].id, "aethelgard");
    strcpy(bodies[1].name, "Aethelgard Prime");
    strcpy(bodies[1].type, "Class-IV Exoplanet");
    bodies[1].orbitRadius = 350.0f;
    bodies[1].orbitSpeed = 0.08f;
    bodies[1].angle = 0.4f;
    bodies[1].radius = 36.0f;
    bodies[1].color = COLOR_BLUE;
    bodies[1].isPlanet = 1;

    // 2: Boreas Minor Moon
    strcpy(bodies[2].id, "boreas");
    strcpy(bodies[2].name, "Boreas Minor");
    strcpy(bodies[2].type, "Frozen Ice Moon");
    bodies[2].orbitRadius = 88.0f;
    bodies[2].orbitSpeed = 0.25f;
    bodies[2].angle = 1.2f;
    bodies[2].radius = 11.0f;
    bodies[2].color = RGB(224, 242, 254);
    bodies[2].isMoon = 1;

    // 3: Zephyr Station
    strcpy(bodies[3].id, "zephyr");
    strcpy(bodies[3].name, "Zephyr Station");
    strcpy(bodies[3].type, "Orbital Shipyard");
    bodies[3].orbitRadius = 64.0f;
    bodies[3].orbitSpeed = -0.18f;
    bodies[3].angle = 3.0f;
    bodies[3].radius = 7.0f;
    bodies[3].color = COLOR_EMERALD;
    bodies[3].isStation = 1;

    // Fleet Ships
    strcpy(fleet[0].id, "genesis");
    strcpy(fleet[0].name, "CSS Genesis");
    strcpy(fleet[0].role, "Colony Flagship");
    strcpy(fleet[0].status, "Stationary Orbit");
    strcpy(fleet[0].mission, "Command & Cryo-Vaults");
    fleet[0].parentIndex = 1;
    fleet[0].orbitDist = 50.0f;
    fleet[0].orbitSpeed = 0.12f;
    fleet[0].angle = 0.0f;
    fleet[0].color = COLOR_CYAN;
    fleet[0].hull = 100;
    fleet[0].targetBelt = 0;

    strcpy(fleet[1].id, "vanguard");
    strcpy(fleet[1].name, "Ark Vanguard");
    strcpy(fleet[1].role, "Agronomy Ark");
    strcpy(fleet[1].status, "Lagrange Point 1");
    strcpy(fleet[1].mission, "Hydroponics & Habitation");
    fleet[1].parentIndex = 1;
    fleet[1].orbitDist = 76.0f;
    fleet[1].orbitSpeed = 0.09f;
    fleet[1].angle = 2.2f;
    fleet[1].color = COLOR_EMERALD;
    fleet[1].hull = 100;
    fleet[1].targetBelt = 0;

    strcpy(fleet[2].id, "aeon");
    strcpy(fleet[2].name, "Surveyor Aeon");
    strcpy(fleet[2].role, "Deep Scout");
    strcpy(fleet[2].status, "Scanning Orbit");
    strcpy(fleet[2].mission, "Sector Reconnaissance");
    fleet[2].parentIndex = 1;
    fleet[2].orbitDist = 125.0f;
    fleet[2].orbitSpeed = 0.35f;
    fleet[2].angle = 1.5f;
    fleet[2].color = COLOR_BLUE;
    fleet[2].hull = 98;
    fleet[2].targetBelt = 0;

    strcpy(fleet[3].id, "drake");
    strcpy(fleet[3].name, "Harvester Drake");
    strcpy(fleet[3].role, "Mining Rig");
    strcpy(fleet[3].status, "Harvesting");
    strcpy(fleet[3].mission, "Volatiles & Ore Extraction");
    fleet[3].parentIndex = 0;
    fleet[3].color = COLOR_AMBER;
    fleet[3].hull = 94;
    fleet[3].targetBelt = 1;
    fleet[3].x = 180.0f;
    fleet[3].y = -140.0f;
    fleet[3].vx = 0.2f;
    fleet[3].vy = 0.1f;

    strcpy(fleet[4].id, "titan");
    strcpy(fleet[4].name, "Freighter Titan-1");
    strcpy(fleet[4].role, "Heavy Hauler");
    strcpy(fleet[4].status, "Supply Route");
    strcpy(fleet[4].mission, "Automated Transport");
    fleet[4].parentIndex = 1;
    fleet[4].orbitDist = 108.0f;
    fleet[4].orbitSpeed = -0.15f;
    fleet[4].angle = 4.2f;
    fleet[4].color = COLOR_PURPLE;
    fleet[4].hull = 100;
    fleet[4].targetBelt = 0;

    CalculateHabitability();
}

// --- GDI Helper Functions ---
static void FillSolidRect(HDC hdc, int x, int y, int w, int h, COLORREF color) {
    RECT rc = {x, y, x + w, y + h};
    HBRUSH br = CreateSolidBrush(color);
    FillRect(hdc, &rc, br);
    DeleteObject(br);
}

static void FrameSolidRect(HDC hdc, int x, int y, int w, int h, COLORREF color) {
    RECT rc = {x, y, x + w, y + h};
    HBRUSH br = CreateSolidBrush(color);
    FrameRect(hdc, &rc, br);
    DeleteObject(br);
}

static void DrawProgressBar(HDC hdc, int x, int y, int w, int h, float percent, COLORREF fillColor) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 1.0f) percent = 1.0f;
    FillSolidRect(hdc, x, y, w, h, RGB(7, 12, 24));
    FrameSolidRect(hdc, x, y, w, h, RGB(20, 34, 61));
    int fillW = (int)((w - 2) * percent);
    if (fillW > 0) {
        FillSolidRect(hdc, x + 1, y + 1, fillW, h - 2, fillColor);
    }
}

// --- Procedural Sprite Rendering Engine (GDI) ---

static void DrawPlanetGDI(HDC hdc, int px, int py, int pr, int sunX, int sunY, float z) {
    // Atmosphere halo
    int atmoR = pr + (int)(8 * z * (1.0f + sim.pressure * 0.4f));
    HPEN hAtmoPen = CreatePen(PS_SOLID, 2, (sim.oxygen > 15.0f ? COLOR_EMERALD : COLOR_CYAN));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hAtmoPen);
    HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hNullBrush);
    Ellipse(hdc, px - atmoR, py - atmoR, px + atmoR, py + atmoR);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hAtmoPen);

    // Planet body clipped to disc
    HRGN hRgnPlanet = CreateEllipticRgn(px - pr, py - pr, px + pr + 1, py + pr + 1);
    HRGN hOldRgn = CreateRectRgn(0, 0, 0, 0);
    GetClipRgn(hdc, hOldRgn);
    ExtSelectClipRgn(hdc, hRgnPlanet, RGN_AND);

    COLORREF seaColor;
    COLORREF landColor;
    if (sim.temp < -20.0f) {
        seaColor = RGB(120, 180, 240);
        landColor = RGB(220, 235, 255);
    } else if (sim.water > 40.0f && sim.oxygen > 12.0f) {
        seaColor = RGB(14, 120, 190);
        landColor = RGB(22, 130, 60);
    } else {
        seaColor = RGB(40, 80, 150);
        landColor = RGB(160, 80, 30);
    }

    FillSolidRect(hdc, px - pr, py - pr, pr * 2, pr * 2, seaColor);

    // Continents
    HBRUSH hLandBrush = CreateSolidBrush(landColor);
    SelectObject(hdc, hLandBrush);
    HPEN hLandPen = CreatePen(PS_SOLID, 1, landColor);
    SelectObject(hdc, hLandPen);

    float rot = sim.time * 0.04f;
    for (int c = 0; c < 3; c++) {
        float cLon = (c * 2.1f + rot);
        int cxPos = px + (int)(sinf(cLon) * pr * 0.65f);
        int cyPos = py + (int)(((c % 2 == 0) ? -0.2f : 0.2f) * pr);
        int rw = (int)(pr * 0.45f * fabsf(cosf(cLon)) + 6);
        int rh = (int)(pr * 0.35f);
        Ellipse(hdc, cxPos - rw, cyPos - rh, cxPos + rw, cyPos + rh);
    }
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hLandBrush);
    DeleteObject(hLandPen);

    // Polar Ice Caps
    float iceCov = (15.0f - sim.temp) / 60.0f;
    if (iceCov < 0.1f) iceCov = 0.1f;
    if (iceCov > 0.6f) iceCov = 0.6f;
    int iceH = (int)(pr * iceCov);
    HBRUSH hIceBrush = CreateSolidBrush(RGB(245, 250, 255));
    SelectObject(hdc, hIceBrush);
    HPEN hIcePen = CreatePen(PS_SOLID, 1, RGB(220, 240, 255));
    SelectObject(hdc, hIcePen);
    Ellipse(hdc, px - pr, py - pr - iceH / 2, px + pr, py - pr + iceH * 2);
    Ellipse(hdc, px - pr, py + pr - iceH * 2, px + pr, py + pr + iceH / 2);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hIceBrush);
    DeleteObject(hIcePen);

    SelectClipRgn(hdc, hOldRgn);
    DeleteObject(hOldRgn);
    DeleteObject(hRgnPlanet);
}

static void DrawMoonGDI(HDC hdc, int mx, int my, int mr, int sunX, int sunY, float z) {
    HRGN hRgnMoon = CreateEllipticRgn(mx - mr, my - mr, mx + mr + 1, my + mr + 1);
    HRGN hOldRgn = CreateRectRgn(0, 0, 0, 0);
    GetClipRgn(hdc, hOldRgn);
    ExtSelectClipRgn(hdc, hRgnMoon, RGN_AND);

    FillSolidRect(hdc, mx - mr, my - mr, mr * 2, mr * 2, RGB(225, 232, 242));

    HBRUSH hCraterBrush = CreateSolidBrush(RGB(180, 195, 215));
    HPEN hCraterPen = CreatePen(PS_SOLID, 1, RGB(245, 250, 255));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hCraterBrush);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hCraterPen);
    Ellipse(hdc, mx - mr / 2, my - mr / 3, mx - mr / 6, my + mr / 6);
    Ellipse(hdc, mx + mr / 8, my + mr / 6, mx + mr / 2, my + mr / 2);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hCraterBrush);
    DeleteObject(hCraterPen);

    SelectClipRgn(hdc, hOldRgn);
    DeleteObject(hOldRgn);
    DeleteObject(hRgnMoon);
}

static void DrawStationGDI(HDC hdc, int stX, int stY, int stR, float z, float simTime) {
    int sz = stR;
    if (sz < 7) sz = 7;

    // Solar Wings (Left and Right)
    FillSolidRect(hdc, stX - sz * 2 - 4, stY - sz / 2, sz * 2, sz, RGB(15, 25, 48));
    FrameSolidRect(hdc, stX - sz * 2 - 4, stY - sz / 2, sz * 2, sz, COLOR_AMBER);
    FillSolidRect(hdc, stX + 4, stY - sz / 2, sz * 2, sz, RGB(15, 25, 48));
    FrameSolidRect(hdc, stX + 4, stY - sz / 2, sz * 2, sz, COLOR_AMBER);

    // Docking Ring
    HPEN hRingPen = CreatePen(PS_SOLID, 1, COLOR_EMERALD);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hRingPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    int ringR = (int)(sz * 1.2f);
    Ellipse(hdc, stX - ringR, stY - ringR, stX + ringR, stY + ringR);

    // Central Station Hub
    HBRUSH hHubBrush = CreateSolidBrush(RGB(28, 42, 65));
    SelectObject(hdc, hHubBrush);
    HPEN hHubPen = CreatePen(PS_SOLID, 1, COLOR_CYAN);
    SelectObject(hdc, hHubPen);
    Ellipse(hdc, stX - sz / 2, stY - sz / 2, stX + sz / 2, stY + sz / 2);
    DeleteObject(hHubBrush);
    DeleteObject(hHubPen);

    // Blinking Docking Nav Light
    if (sinf(simTime * 6.0f) > 0.0f) {
        FillSolidRect(hdc, stX - 2, stY - sz - 4, 4, 4, COLOR_EMERALD);
    }

    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hRingPen);
}

static void DrawShipGDI(HDC hdc, Ship* s, int sx, int sy, float z, int i, float simTime) {
    float heading;
    if (s->targetBelt) {
        heading = atan2f(s->vy, s->vx);
    } else {
        heading = s->angle + 1.5707963f; // PI/2
    }

    float cosH = cosf(heading);
    float sinH = sinf(heading);

    // Thruster exhaust flame
    int flameLen = (int)(8 * z + sinf(simTime * 25.0f + sx) * 2.0f);
    HPEN hFlamePen = CreatePen(PS_SOLID, 2, s->color);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hFlamePen);
    MoveToEx(hdc, sx, sy, NULL);
    LineTo(hdc, sx - (int)(cosH * flameLen), sy - (int)(sinH * flameLen));
    SelectObject(hdc, hOldPen);
    DeleteObject(hFlamePen);

    if (i == 0) {
        // CSS Genesis: Flagship Ark with Command Spine & Habitat Ring
        int hlen = (int)(10 * z);
        int hwid = (int)(4 * z);
        POINT pts[4];
        pts[0].x = sx + (int)(cosH * hlen);
        pts[0].y = sy + (int)(sinH * hlen);
        pts[1].x = sx - (int)(cosH * (hlen / 2) + sinH * hwid);
        pts[1].y = sy - (int)(sinH * (hlen / 2) - cosH * hwid);
        pts[2].x = sx - (int)(cosH * hlen);
        pts[2].y = sy - (int)(sinH * hlen);
        pts[3].x = sx - (int)(cosH * (hlen / 2) - sinH * hwid);
        pts[3].y = sy - (int)(sinH * (hlen / 2) + cosH * hwid);

        HBRUSH hHullBrush = CreateSolidBrush(RGB(20, 36, 60));
        HPEN hHullPen = CreatePen(PS_SOLID, 1, COLOR_CYAN);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hHullBrush);
        HPEN hOldP = (HPEN)SelectObject(hdc, hHullPen);
        Polygon(hdc, pts, 4);

        // Habitat Torus
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        int ringR = (int)(7 * z);
        Ellipse(hdc, sx - ringR, sy - ringR, sx + ringR, sy + ringR);

        SelectObject(hdc, hOldBrush);
        SelectObject(hdc, hOldP);
        DeleteObject(hHullBrush);
        DeleteObject(hHullPen);

    } else if (i == 1) {
        // Ark Vanguard: Agronomy Ark with 3 Biodomes
        int hlen = (int)(8 * z);
        HPEN hSpinePen = CreatePen(PS_SOLID, 3, RGB(50, 70, 90));
        HPEN hOldP = (HPEN)SelectObject(hdc, hSpinePen);
        MoveToEx(hdc, sx - (int)(cosH * hlen), sy - (int)(sinH * hlen), NULL);
        LineTo(hdc, sx + (int)(cosH * hlen), sy + (int)(sinH * hlen));
        SelectObject(hdc, hOldP);
        DeleteObject(hSpinePen);

        // 3 Emerald domes
        HBRUSH hDomeBrush = CreateSolidBrush(COLOR_EMERALD);
        HPEN hDomePen = CreatePen(PS_SOLID, 1, RGB(160, 240, 200));
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hDomeBrush);
        hOldP = (HPEN)SelectObject(hdc, hDomePen);
        int dr = (int)(3 * z);
        if (dr < 2) dr = 2;
        Ellipse(hdc, sx - dr, sy - dr, sx + dr, sy + dr);
        int fOffX = (int)(cosH * (hlen * 0.6f));
        int fOffY = (int)(sinH * (hlen * 0.6f));
        Ellipse(hdc, sx + fOffX - dr, sy + fOffY - dr, sx + fOffX + dr, sy + fOffY + dr);
        Ellipse(hdc, sx - fOffX - dr, sy - fOffY - dr, sx - fOffX + dr, sy - fOffY + dr);
        SelectObject(hdc, hOldBrush);
        SelectObject(hdc, hOldP);
        DeleteObject(hDomeBrush);
        DeleteObject(hDomePen);

    } else if (i == 2) {
        // Surveyor Aeon: Sleek Delta Dart
        int dlen = (int)(11 * z);
        int dwid = (int)(6 * z);
        POINT pts[4];
        pts[0].x = sx + (int)(cosH * dlen);
        pts[0].y = sy + (int)(sinH * dlen);
        pts[1].x = sx - (int)(cosH * dlen * 0.6f + sinH * dwid);
        pts[1].y = sy - (int)(sinH * dlen * 0.6f - cosH * dwid);
        pts[2].x = sx - (int)(cosH * dlen * 0.3f);
        pts[2].y = sy - (int)(sinH * dlen * 0.3f);
        pts[3].x = sx - (int)(cosH * dlen * 0.6f - sinH * dwid);
        pts[3].y = sy - (int)(sinH * dlen * 0.6f + cosH * dwid);

        HBRUSH hDartBrush = CreateSolidBrush(RGB(15, 30, 50));
        HPEN hDartPen = CreatePen(PS_SOLID, 1, COLOR_BLUE);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hDartBrush);
        HPEN hOldP = (HPEN)SelectObject(hdc, hDartPen);
        Polygon(hdc, pts, 4);
        SelectObject(hdc, hOldBrush);
        SelectObject(hdc, hOldP);
        DeleteObject(hDartBrush);
        DeleteObject(hDartPen);

    } else if (i == 3) {
        // Harvester Drake: Industrial Mining Rig with Claws
        int bsz = (int)(5 * z);
        if (bsz < 3) bsz = 3;
        FillSolidRect(hdc, sx - bsz, sy - bsz, bsz * 2, bsz * 2, RGB(25, 20, 15));
        FrameSolidRect(hdc, sx - bsz, sy - bsz, bsz * 2, bsz * 2, COLOR_AMBER);
        HPEN hClawPen = CreatePen(PS_SOLID, 2, COLOR_AMBER);
        HPEN hOldP = (HPEN)SelectObject(hdc, hClawPen);
        int cx1 = sx + (int)(cosH * bsz * 1.8f - sinH * bsz * 0.8f);
        int cy1 = sy + (int)(sinH * bsz * 1.8f + cosH * bsz * 0.8f);
        int cx2 = sx + (int)(cosH * bsz * 1.8f + sinH * bsz * 0.8f);
        int cy2 = sy + (int)(sinH * bsz * 1.8f - cosH * bsz * 0.8f);
        MoveToEx(hdc, sx, sy, NULL); LineTo(hdc, cx1, cy1);
        MoveToEx(hdc, sx, sy, NULL); LineTo(hdc, cx2, cy2);
        SelectObject(hdc, hOldP);
        DeleteObject(hClawPen);

    } else {
        // Freighter Titan-1: Modular Container Hauler
        int fwid = (int)(4 * z);
        if (fwid < 3) fwid = 3;
        FillSolidRect(hdc, sx - fwid * 2, sy - fwid, fwid * 4, fwid * 2, RGB(80, 40, 140));
        FrameSolidRect(hdc, sx - fwid * 2, sy - fwid, fwid * 4, fwid * 2, COLOR_PURPLE);
    }
}

// --- Button Hit Testing & Layout ---
typedef struct {
    int id;
    RECT rect;
    char text[48];
    char subtext[64];
    int isEnabled;
} UIButton;

#define MAX_BUTTONS 32
static UIButton g_buttons[MAX_BUTTONS];
static int g_buttonCount = 0;

static void ClearButtons(void) {
    g_buttonCount = 0;
}

static void AddButton(int id, int x, int y, int w, int h, const char* txt, const char* subtxt, int enabled) {
    if (g_buttonCount < MAX_BUTTONS) {
        g_buttons[g_buttonCount].id = id;
        SetRect(&g_buttons[g_buttonCount].rect, x, y, x + w, y + h);
        strncpy(g_buttons[g_buttonCount].text, txt, sizeof(g_buttons[g_buttonCount].text) - 1);
        if (subtxt) strncpy(g_buttons[g_buttonCount].subtext, subtxt, sizeof(g_buttons[g_buttonCount].subtext) - 1);
        else g_buttons[g_buttonCount].subtext[0] = '\0';
        g_buttons[g_buttonCount].isEnabled = enabled;
        g_buttonCount++;
    }
}

// Button IDs
#define BID_TAB_TERRA       10
#define BID_TAB_FLEET       11
#define BID_TAB_COLONY      12
#define BID_TAB_ECONOMY     13

#define BID_FOCUS_PLANET    20
#define BID_FOCUS_ARK       21
#define BID_ZOOM_IN         22
#define BID_ZOOM_OUT        23
#define BID_RESET_VIEW      24

#define BID_SPEED_PAUSE     30
#define BID_SPEED_1X        31
#define BID_SPEED_2X        32
#define BID_SPEED_5X        33
#define BID_AUDIO_TOGGLE    34

#define BID_ACT_SOLAR_MIRROR 40
#define BID_ACT_ATMO_PROC   41
#define BID_ACT_COMET_DROP  42
#define BID_ACT_BIOSEED     43
#define BID_ACT_CORE_DYNAMO 44
#define BID_ACT_ALGAE       45

#define BID_COL_AWAKEN      50
#define BID_COL_DOME        51
#define BID_COL_HYDRO       52
#define BID_COL_SOLAR       53

#define BID_ORDER_GEN_HOLD  60
#define BID_ORDER_GEN_BOOST 61
#define BID_ORDER_VAN_RATION 62
#define BID_ORDER_VAN_ORBIT  63
#define BID_ORDER_AEO_SCAN   64
#define BID_ORDER_DRA_MINE   65
#define BID_ORDER_DRA_SCOOP  66
#define BID_ORDER_DRA_DOCK   67
#define BID_ORDER_TIT_LOOP   68
#define BID_ORDER_TIT_HOLD   69

#define BID_SEL_ACT1        80
#define BID_SEL_ACT2        81
#define BID_SEL_CLOSE       82

// --- Action Handlers ---
static void HandleIntervention(int bid) {
    switch (bid) {
        case BID_ACT_SOLAR_MIRROR:
            if (sim.energy >= 350 && sim.minerals >= 150) {
                sim.energy -= 350;
                sim.minerals -= 150;
                sim.solarMirrors++;
                sim.temp += 1.8f;
                CalculateHabitability();
                char buf[128];
                sprintf(buf, "Orbital Solar Mirror #%d deployed (+1.8 C insolation).", sim.solarMirrors);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                SetLogMsg("Insufficient resources (Req: 350 kW Energy, 150 t Minerals).", 1);
            }
            break;
        case BID_ACT_ATMO_PROC:
            if (sim.minerals >= 180 && sim.energy >= 100) {
                sim.minerals -= 180;
                sim.energy -= 100;
                sim.atmoProcessors++;
                sim.pressure += 0.05f;
                CalculateHabitability();
                char buf[128];
                sprintf(buf, "Troposphere Gas Injector #%d constructed (+0.05 atm).", sim.atmoProcessors);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                SetLogMsg("Insufficient resources (Req: 180 t Minerals, 100 kW Energy).", 1);
            }
            break;
        case BID_ACT_COMET_DROP:
            if (sim.volatiles >= 300 && sim.energy >= 150) {
                sim.volatiles -= 300;
                sim.energy -= 150;
                sim.water += 2.5f;
                sim.pressure += 0.02f;
                CalculateHabitability();
                SetLogMsg("Ice comet deflected to polar basin (+2.5% Water, +0.02 atm).", 0);
                PlaySoundFx(SFX_SUCCESS);
            } else {
                SetLogMsg("Insufficient resources (Req: 300 t Volatiles, 150 kW Energy).", 1);
            }
            break;
        case BID_ACT_BIOSEED:
            if (sim.food >= 120 && sim.energy >= 100) {
                sim.food -= 120;
                sim.energy -= 100;
                sim.bioseedStations++;
                sim.oxygen += 0.9f;
                CalculateHabitability();
                char buf[128];
                sprintf(buf, "Lichen bioseeding distributed on barren crags (+0.9%% O2).", sim.bioseedStations);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_SUCCESS);
            } else {
                SetLogMsg("Insufficient resources (Req: 120 t Food/Biomass, 100 kW Energy).", 1);
            }
            break;
        case BID_ACT_CORE_DYNAMO:
            if (sim.minerals >= 400 && sim.energy >= 250) {
                sim.minerals -= 400;
                sim.energy -= 250;
                sim.coreDynamos++;
                sim.magnet += 0.08f;
                CalculateHabitability();
                char buf[128];
                sprintf(buf, "Core Magnetic Dynamo #%d primed (+0.08 Gauss Shielding).", sim.coreDynamos);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                SetLogMsg("Insufficient resources (Req: 400 t Minerals, 250 kW Energy).", 1);
            }
            break;
        case BID_ACT_ALGAE:
            if (sim.water < 20.0f) {
                SetLogMsg("Algae seeding requires oceans (Surface water must exceed 20%).", 1);
                return;
            }
            if (sim.volatiles >= 200 && sim.food >= 150) {
                sim.volatiles -= 200;
                sim.food -= 150;
                sim.oxygen += 1.4f;
                CalculateHabitability();
                SetLogMsg("Pelagic cyanobacteria blooms seeded into lakes (+1.4% O2).", 0);
                PlaySoundFx(SFX_SUCCESS);
            } else {
                SetLogMsg("Insufficient resources (Req: 200 t Volatiles, 150 t Food).", 1);
            }
            break;
    }
}

static void HandleColonyProject(int bid) {
    switch (bid) {
        case BID_COL_AWAKEN:
            if (sim.colonists + 2500 > sim.housingCap) {
                SetLogMsg("Cannot awaken sleepers: Insufficient dome housing capacity.", 1);
                return;
            }
            if (sim.cryoSleepers >= 2500 && sim.food >= 100) {
                sim.cryoSleepers -= 2500;
                sim.colonists += 2500;
                sim.food -= 100;
                SetLogMsg("2,500 colonists revived from CSS Genesis cryo-stasis.", 0);
                PlaySoundFx(SFX_SUCCESS);
            } else {
                SetLogMsg("Insufficient sleepers or food reserves for revivification.", 1);
            }
            break;
        case BID_COL_DOME:
            if (sim.minerals >= 450 && sim.energy >= 150) {
                sim.minerals -= 450;
                sim.energy -= 150;
                sim.housingCap += 15000;
                SetLogMsg("Pressurized Geodesic Dome Beta completed (+15,000 Housing).", 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                SetLogMsg("Insufficient resources (Req: 450 t Minerals, 150 kW Energy).", 1);
            }
            break;
        case BID_COL_HYDRO:
            if (sim.minerals >= 200 && sim.volatiles >= 100) {
                sim.minerals -= 200;
                sim.volatiles -= 100;
                sim.hydroTowers++;
                char buf[128];
                sprintf(buf, "Vertical Hydroponic Agronomy Tower #%d operational (+25 Food/cyc).", sim.hydroTowers);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                SetLogMsg("Insufficient resources (Req: 200 t Minerals, 100 t Volatiles).", 1);
            }
            break;
        case BID_COL_SOLAR:
            if (sim.minerals >= 220) {
                sim.minerals -= 220;
                sim.surfaceSolar++;
                char buf[128];
                sprintf(buf, "Surface Photovoltaic Field #%d connected to power grid (+180 kW).", sim.surfaceSolar);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                SetLogMsg("Insufficient resources (Req: 220 t Minerals).", 1);
            }
            break;
    }
}

static void HandleShipOrder(int bid) {
    switch (bid) {
        case BID_ORDER_GEN_HOLD:
            strcpy(fleet[0].status, "Stationary Orbit");
            SetLogMsg("CSS Genesis anchored in stable geosynchronous orbit.", 0);
            PlaySoundFx(SFX_CLICK);
            break;
        case BID_ORDER_GEN_BOOST:
            SetLogMsg("CSS Genesis boosted main subspace array (Sensor telemetry +40%).", 0);
            PlaySoundFx(SFX_CLICK);
            break;
        case BID_ORDER_VAN_RATION:
            SetLogMsg("Ark Vanguard optimized hydroponics nutrient mixture (+5 t Food/cyc).", 0);
            PlaySoundFx(SFX_CLICK);
            break;
        case BID_ORDER_VAN_ORBIT:
            strcpy(fleet[1].status, "Low Orbit");
            SetLogMsg("Ark Vanguard transitioned to low equatorial orbit.", 0);
            PlaySoundFx(SFX_CLICK);
            break;
        case BID_ORDER_AEO_SCAN:
            strcpy(fleet[2].status, "Long-Range Scan");
            SetLogMsg("Surveyor Aeon initiating high-resolution planetary sensor sweep.", 0);
            PlaySoundFx(SFX_CLICK);
            break;
        case BID_ORDER_DRA_MINE:
            fleet[3].targetBelt = 1;
            strcpy(fleet[3].status, "Mining Belt");
            SetLogMsg("Harvester Drake dispatched to Tartarus Belt for heavy mineral excavation.", 0);
            PlaySoundFx(SFX_CLICK);
            break;
        case BID_ORDER_DRA_SCOOP:
            fleet[3].targetBelt = 0;
            fleet[3].parentIndex = 2; // Boreas
            strcpy(fleet[3].status, "Scooping Ring");
            SetLogMsg("Harvester Drake repositioned to Boreas Minor to scoop ice volatiles.", 0);
            PlaySoundFx(SFX_CLICK);
            break;
        case BID_ORDER_DRA_DOCK:
            fleet[3].targetBelt = 0;
            fleet[3].parentIndex = 1;
            strcpy(fleet[3].status, "Docked Depot");
            SetLogMsg("Harvester Drake docked at Zephyr Orbital Depot for maintenance.", 0);
            PlaySoundFx(SFX_CLICK);
            break;
        case BID_ORDER_TIT_LOOP:
            strcpy(fleet[4].status, "Supply Route");
            SetLogMsg("Freighter Titan-1 maintaining automated freight conveyor.", 0);
            PlaySoundFx(SFX_CLICK);
            break;
        case BID_ORDER_TIT_HOLD:
            strcpy(fleet[4].status, "Standby");
            SetLogMsg("Freighter Titan-1 idling in orbital parking loop.", 0);
            PlaySoundFx(SFX_CLICK);
            break;
    }
}

// --- Render Implementation ---
static void RenderUI(HDC hdc, int width, int height) {
    ClearButtons();

    // Fonts
    HFONT hFontMain = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
    HFONT hFontBold = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
    HFONT hFontTitle = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
    HFONT hFontSmall = CreateFontA(11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

    HFONT hOldFont = (HFONT)SelectObject(hdc, hFontMain);
    SetBkMode(hdc, TRANSPARENT);

    // 1. Fill Deep Space Background
    FillSolidRect(hdc, 0, 0, width, height, COLOR_BG_DEEP);

    // Layout Dimensions
    int headerH = 46;
    int footerH = 34;
    int sidebarW = 380;
    if (sidebarW > width / 2) sidebarW = width / 2;
    int viewportW = width - sidebarW;
    int viewportH = height - headerH - footerH;

    // 2. Draw Top Header Bar
    FillSolidRect(hdc, 0, 0, width, headerH, COLOR_BG_PANEL_DARK);
    FillSolidRect(hdc, 0, headerH - 1, width, 1, COLOR_BORDER);

    // Brand Logo
    SelectObject(hdc, hFontTitle);
    SetTextColor(hdc, COLOR_CYAN);
    TextOutA(hdc, 12, 6, "KCOSMIC", 7);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COLOR_TEXT_DIM);
    TextOutA(hdc, 84, 9, "// Fleet Logistics & Terraforming", 33);

    // Header Badges
    SelectObject(hdc, hFontMain);
    int badgeX = 360;
    if (badgeX > width - 420) badgeX = width - 420;
    if (badgeX < 240) badgeX = 240;

    char buf[128];
    // Cycle
    sprintf(buf, "CYC: %.2f", sim.cycle);
    SetTextColor(hdc, COLOR_CYAN);
    TextOutA(hdc, badgeX, 7, buf, (int)strlen(buf));

    // Energy
    sprintf(buf, "ENG: %d kW (%+d)", sim.energy, sim.deltaEnergy);
    SetTextColor(hdc, COLOR_EMERALD);
    TextOutA(hdc, badgeX + 90, 7, buf, (int)strlen(buf));

    // Minerals
    sprintf(buf, "MIN: %d t (%+d)", sim.minerals, sim.deltaMinerals);
    SetTextColor(hdc, COLOR_BLUE);
    TextOutA(hdc, badgeX + 240, 7, buf, (int)strlen(buf));

    // Volatiles
    sprintf(buf, "VOL: %d t (%+d)", sim.volatiles, sim.deltaVolatiles);
    SetTextColor(hdc, COLOR_PURPLE);
    TextOutA(hdc, badgeX + 380, 7, buf, (int)strlen(buf));

    // Habitability
    sprintf(buf, "HAB: %.1f%%", sim.habitability);
    SetTextColor(hdc, COLOR_AMBER);
    TextOutA(hdc, badgeX + 90, 24, buf, (int)strlen(buf));

    // Colonists
    sprintf(buf, "POP: %d", sim.colonists);
    SetTextColor(hdc, COLOR_TEXT_BRIGHT);
    TextOutA(hdc, badgeX + 240, 24, buf, (int)strlen(buf));

    // Food
    sprintf(buf, "FOOD: %d t (%+d)", sim.food, sim.deltaFood);
    SetTextColor(hdc, COLOR_EMERALD);
    TextOutA(hdc, badgeX + 380, 24, buf, (int)strlen(buf));

    // 3. Viewport Stellar Canvas (Left Area)
    int cx = viewportW / 2 + (int)sim.camX;
    int cy = headerH + viewportH / 2 + (int)sim.camY;
    float z = sim.zoom;

    // Viewport Clipping
    HRGN hRgnViewport = CreateRectRgn(0, headerH, viewportW, headerH + viewportH);
    SelectClipRgn(hdc, hRgnViewport);

    // A. Starfield
    for (int i = 0; i < STAR_COUNT; i++) {
        int sx = cx + (int)(stars[i].x * z);
        int sy = cy + (int)(stars[i].y * z);
        if (sx >= 0 && sx < viewportW && sy >= headerH && sy < headerH + viewportH) {
            int brightness = (int)(stars[i].alpha * 220.0f);
            COLORREF starColor = RGB(brightness, brightness, brightness + 20);
            int sz = (int)(stars[i].size * z);
            if (sz < 1) sz = 1;
            FillSolidRect(hdc, sx, sy, sz, sz, starColor);
        }
    }

    // B. Coordinate Grid
    HPEN hGridPen = CreatePen(PS_SOLID, 1, RGB(20, 36, 64));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hGridPen);
    int step = (int)(80 * z);
    if (step < 30) step = 30;
    int startX = (cx % step);
    int startY = (cy % step);
    for (int x = startX; x < viewportW; x += step) {
        MoveToEx(hdc, x, headerH, NULL);
        LineTo(hdc, x, headerH + viewportH);
    }
    for (int y = startY; y < headerH + viewportH; y += step) {
        MoveToEx(hdc, 0, y, NULL);
        LineTo(hdc, viewportW, y);
    }
    SelectObject(hdc, hOldPen);
    DeleteObject(hGridPen);

    // C. Central Sun (Kepler-186 Helios)
    int sunX = cx + (int)(-300.0f * z);
    int sunY = cy;
    int sunR = (int)(bodies[0].radius * z);
    bodies[0].currX = (float)sunX;
    bodies[0].currY = (float)sunY;

    // Outer Glow Rings
    HPEN hCoronaPen = CreatePen(PS_SOLID, 1, RGB(180, 70, 20));
    SelectObject(hdc, hCoronaPen);
    HBRUSH hCoronaBrush = CreateSolidBrush(RGB(50, 18, 5));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hCoronaBrush);
    Ellipse(hdc, sunX - sunR * 2, sunY - sunR * 2, sunX + sunR * 2, sunY + sunR * 2);
    DeleteObject(hCoronaBrush);

    // Sun Core
    HBRUSH hSunBrush = CreateSolidBrush(COLOR_ORANGE);
    SelectObject(hdc, hSunBrush);
    Ellipse(hdc, sunX - sunR, sunY - sunR, sunX + sunR, sunY + sunR);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hSunBrush);
    DeleteObject(hCoronaPen);

    // Sun Label
    SetTextColor(hdc, RGB(255, 180, 100));
    SelectObject(hdc, hFontSmall);
    TextOutA(hdc, sunX - 45, sunY + sunR + 4, bodies[0].name, (int)strlen(bodies[0].name));

    // D. Tartarus Asteroid Belt
    for (int i = 0; i < ASTEROID_COUNT; i++) {
        if (!sim.paused && sim.speed > 0) {
            asteroids[i].angle += asteroids[i].speed * 0.005f * sim.speed;
        }
        int ax = sunX + (int)(cos(asteroids[i].angle) * (asteroids[i].dist * z));
        int ay = sunY + (int)(sin(asteroids[i].angle) * (asteroids[i].dist * z * 0.7f));
        int asz = (int)(asteroids[i].size * z);
        if (asz < 1) asz = 1;
        FillSolidRect(hdc, ax, ay, asz, asz, RGB(160, 175, 200));
    }

    // E. Target Planet: Aethelgard Prime
    if (!sim.paused && sim.speed > 0) {
        bodies[1].angle += bodies[1].orbitSpeed * 0.008f * sim.speed;
    }
    float planetOrbitR = bodies[1].orbitRadius * z;
    int px = sunX + (int)(cos(bodies[1].angle) * planetOrbitR);
    int py = sunY + (int)(sin(bodies[1].angle) * (planetOrbitR * 0.7f));
    int pr = (int)(bodies[1].radius * z);
    bodies[1].currX = (float)px;
    bodies[1].currY = (float)py;

    // Orbit ellipse
    HPEN hOrbitPen = CreatePen(PS_DOT, 1, RGB(40, 70, 110));
    SelectObject(hdc, hOrbitPen);
    Arc(hdc, sunX - (int)planetOrbitR, sunY - (int)(planetOrbitR * 0.7f),
             sunX + (int)planetOrbitR, sunY + (int)(planetOrbitR * 0.7f), 0, 0, 0, 0);
    DeleteObject(hOrbitPen);

    // Planet Body & Surface
    DrawPlanetGDI(hdc, px, py, pr, sunX, sunY, z);

    // Selected reticle on planet
    if (sim.selectedType == 2) {
        HPEN hSelPen = CreatePen(PS_SOLID, 2, COLOR_CYAN);
        SelectObject(hdc, hSelPen);
        Rectangle(hdc, px - pr - 6, py - pr - 6, px + pr + 6, py + pr + 6);
        DeleteObject(hSelPen);
    }

    // Planet Label
    sprintf(buf, "%s [%.1f%%]", bodies[1].name, sim.habitability);
    SetTextColor(hdc, COLOR_BLUE);
    SelectObject(hdc, hFontSmall);
    TextOutA(hdc, px - 45, py + pr + 4, buf, (int)strlen(buf));

    // F. Boreas Minor Moon
    if (!sim.paused && sim.speed > 0) {
        bodies[2].angle += bodies[2].orbitSpeed * 0.02f * sim.speed;
    }
    float moonDist = bodies[2].orbitRadius * z;
    int mx = px + (int)(cos(bodies[2].angle) * moonDist);
    int my = py + (int)(sin(bodies[2].angle) * (moonDist * 0.6f));
    int mr = (int)(bodies[2].radius * z);
    bodies[2].currX = (float)mx;
    bodies[2].currY = (float)my;

    DrawMoonGDI(hdc, mx, my, mr, sunX, sunY, z);

    if (sim.selectedType == 3) {
        HPEN hSelPen = CreatePen(PS_SOLID, 1, COLOR_CYAN);
        SelectObject(hdc, hSelPen);
        Rectangle(hdc, mx - mr - 4, my - mr - 4, mx + mr + 4, my + mr + 4);
        DeleteObject(hSelPen);
    }
    SetTextColor(hdc, COLOR_TEXT_PRI);
    TextOutA(hdc, mx - 30, my + mr + 2, bodies[2].name, (int)strlen(bodies[2].name));

    // G. Zephyr Station
    if (!sim.paused && sim.speed > 0) {
        bodies[3].angle += bodies[3].orbitSpeed * 0.02f * sim.speed;
    }
    float stDist = bodies[3].orbitRadius * z;
    int stX = px + (int)(cos(bodies[3].angle) * stDist);
    int stY = py + (int)(sin(bodies[3].angle) * (stDist * 0.6f));
    int stR = (int)(bodies[3].radius * z);
    bodies[3].currX = (float)stX;
    bodies[3].currY = (float)stY;

    DrawStationGDI(hdc, stX, stY, stR, z, sim.time);
    if (sim.selectedType == 4) {
        FrameSolidRect(hdc, stX - stR - 3, stY - stR - 3, stR * 2 + 6, stR * 2 + 6, COLOR_CYAN);
    }

    // H. Fleet Ships
    for (int i = 0; i < 5; i++) {
        int sx, sy;
        if (fleet[i].targetBelt) {
            if (!sim.paused && sim.speed > 0) {
                fleet[i].x += fleet[i].vx * sim.speed;
                fleet[i].y += fleet[i].vy * sim.speed;
                if (fleet[i].x > 350.0f || fleet[i].x < -100.0f) fleet[i].vx *= -1.0f;
                if (fleet[i].y > 200.0f || fleet[i].y < -200.0f) fleet[i].vy *= -1.0f;
            }
            sx = cx + (int)(fleet[i].x * z);
            sy = cy + (int)(fleet[i].y * z);
        } else {
            if (!sim.paused && sim.speed > 0) {
                fleet[i].angle += fleet[i].orbitSpeed * 0.02f * sim.speed;
            }
            int centerTargetX = (fleet[i].parentIndex == 2 ? mx : px);
            int centerTargetY = (fleet[i].parentIndex == 2 ? my : py);
            float dist = fleet[i].orbitDist * z;
            sx = centerTargetX + (int)(cos(fleet[i].angle) * dist);
            sy = centerTargetY + (int)(sin(fleet[i].angle) * (dist * 0.7f));
        }

        fleet[i].currX = (float)sx;
        fleet[i].currY = (float)sy;

        // Render Ship Sprite
        DrawShipGDI(hdc, &fleet[i], sx, sy, z, i, sim.time);

        if (sim.selectedType == 5 && sim.selectedIndex == i) {
            int retSz = (int)(10 * z);
            if (retSz < 6) retSz = 6;
            FrameSolidRect(hdc, sx - retSz, sy - retSz, retSz * 2, retSz * 2, fleet[i].color);
        }

        SetTextColor(hdc, fleet[i].color);
        TextOutA(hdc, sx + (int)(10 * z) + 4, sy - 5, fleet[i].name, (int)strlen(fleet[i].name));
    }

    // I. Viewport Top Overlay Card
    FillSolidRect(hdc, 10, headerH + 10, 270, 52, COLOR_BG_PANEL);
    FrameSolidRect(hdc, 10, headerH + 10, 270, 52, COLOR_BORDER);
    FillSolidRect(hdc, 10, headerH + 10, 3, 52, COLOR_CYAN);
    SetTextColor(hdc, COLOR_CYAN);
    SelectObject(hdc, hFontSmall);
    TextOutA(hdc, 20, headerH + 15, "SECTOR: Kepler-186e / Prime Anchor", 34);
    SetTextColor(hdc, COLOR_TEXT_BRIGHT);
    TextOutA(hdc, 20, headerH + 30, "Target: Aethelgard Prime [Hostile IV]", 37);
    TextOutA(hdc, 20, headerH + 44, "Fleet: 5 Ships Active | Relics: Detected", 40);

    // J. Viewport Navigation Buttons (Bottom-Left of Viewport)
    int navY = headerH + viewportH - 30;
    AddButton(BID_FOCUS_PLANET, 10, navY, 82, 22, "Focus Planet", NULL, 1);
    AddButton(BID_FOCUS_ARK, 96, navY, 74, 22, "Focus Ark", NULL, 1);
    AddButton(BID_ZOOM_IN, 174, navY, 56, 22, "Zoom +", NULL, 1);
    AddButton(BID_ZOOM_OUT, 234, navY, 56, 22, "Zoom -", NULL, 1);
    AddButton(BID_RESET_VIEW, 294, navY, 52, 22, "Reset", NULL, 1);

    // K. Viewport Selection Card (if something selected)
    if (sim.selectedType != 0) {
        int scX = viewportW - 220;
        int scY = headerH + 10;
        FillSolidRect(hdc, scX, scY, 210, 110, COLOR_BG_PANEL);
        FrameSolidRect(hdc, scX, scY, 210, 110, COLOR_BLUE);

        const char* selName = "Object";
        const char* selType = "Target";
        if (sim.selectedType == 1) { selName = bodies[0].name; selType = bodies[0].type; }
        else if (sim.selectedType == 2) { selName = bodies[1].name; selType = bodies[1].type; }
        else if (sim.selectedType == 3) { selName = bodies[2].name; selType = bodies[2].type; }
        else if (sim.selectedType == 4) { selName = bodies[3].name; selType = bodies[3].type; }
        else if (sim.selectedType == 5 && sim.selectedIndex >= 0) {
            selName = fleet[sim.selectedIndex].name;
            selType = fleet[sim.selectedIndex].role;
        }

        SetTextColor(hdc, COLOR_CYAN);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, scX + 8, scY + 6, selName, (int)strlen(selName));

        SetTextColor(hdc, COLOR_TEXT_DIM);
        SelectObject(hdc, hFontSmall);
        TextOutA(hdc, scX + 8, scY + 22, selType, (int)strlen(selType));

        AddButton(BID_SEL_CLOSE, scX + 186, scY + 4, 18, 16, "X", NULL, 1);
        AddButton(BID_SEL_ACT1, scX + 8, scY + 48, 194, 24, "Inspect Telemetry", NULL, 1);
        AddButton(BID_SEL_ACT2, scX + 8, scY + 76, 194, 24, "Reposition Orbit", NULL, 1);
    }

    SelectClipRgn(hdc, NULL);
    DeleteObject(hRgnViewport);

    // 4. Right Sidebar Area
    int sbX = viewportW;
    FillSolidRect(hdc, sbX, headerH, sidebarW, viewportH, COLOR_BG_PANEL);
    FillSolidRect(hdc, sbX, headerH, 1, viewportH, COLOR_BORDER);

    // Tab Header
    int tabW = sidebarW / 4;
    AddButton(BID_TAB_TERRA, sbX, headerH, tabW, 28, "TERRAFORM", NULL, 1);
    AddButton(BID_TAB_FLEET, sbX + tabW, headerH, tabW, 28, "FLEET", NULL, 1);
    AddButton(BID_TAB_COLONY, sbX + tabW * 2, headerH, tabW, 28, "COLONY", NULL, 1);
    AddButton(BID_TAB_ECONOMY, sbX + tabW * 3, headerH, tabW, 28, "ECONOMY", NULL, 1);

    int contentY = headerH + 34;

    // TAB 0: TERRAFORM
    if (sim.activeTab == 0) {
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, contentY, "PLANETARY BIOMETRICS", 20);

        const char* habClass = "Hostile";
        COLORREF habColor = COLOR_ROSE;
        if (sim.habitability >= 75.0f) { habClass = "Garden World"; habColor = COLOR_EMERALD; }
        else if (sim.habitability >= 50.0f) { habClass = "Developing"; habColor = COLOR_BLUE; }
        else if (sim.habitability >= 25.0f) { habClass = "Harsh"; habColor = COLOR_AMBER; }

        sprintf(buf, "%s (%.1f%%)", habClass, sim.habitability);
        SetTextColor(hdc, habColor);
        TextOutA(hdc, sbX + sidebarW - 130, contentY, buf, (int)strlen(buf));

        // Metric 1: Pressure
        int my = contentY + 22;
        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_TEXT_PRI);
        TextOutA(hdc, sbX + 12, my, "Atmospheric Pressure", 20);
        sprintf(buf, "%.2f atm", sim.pressure);
        SetTextColor(hdc, COLOR_CYAN);
        TextOutA(hdc, sbX + sidebarW - 75, my, buf, (int)strlen(buf));
        DrawProgressBar(hdc, sbX + 12, my + 14, sidebarW - 24, 7, sim.pressure / 1.5f, COLOR_CYAN);

        // Metric 2: Temperature
        my += 28;
        SetTextColor(hdc, COLOR_TEXT_PRI);
        TextOutA(hdc, sbX + 12, my, "Equilibrium Surface Temp", 24);
        sprintf(buf, "%.1f C", sim.temp);
        SetTextColor(hdc, COLOR_AMBER);
        TextOutA(hdc, sbX + sidebarW - 75, my, buf, (int)strlen(buf));
        DrawProgressBar(hdc, sbX + 12, my + 14, sidebarW - 24, 7, (sim.temp + 60.0f) / 100.0f, COLOR_AMBER);

        // Metric 3: Water
        my += 28;
        SetTextColor(hdc, COLOR_TEXT_PRI);
        TextOutA(hdc, sbX + 12, my, "Hydrosphere / Liquid Water", 26);
        sprintf(buf, "%.1f%%", sim.water);
        SetTextColor(hdc, COLOR_BLUE);
        TextOutA(hdc, sbX + sidebarW - 75, my, buf, (int)strlen(buf));
        DrawProgressBar(hdc, sbX + 12, my + 14, sidebarW - 24, 7, sim.water / 100.0f, COLOR_BLUE);

        // Metric 4: Oxygen
        my += 28;
        SetTextColor(hdc, COLOR_TEXT_PRI);
        TextOutA(hdc, sbX + 12, my, "Oxygen (O2 Concentration)", 25);
        sprintf(buf, "%.1f%%", sim.oxygen);
        SetTextColor(hdc, COLOR_EMERALD);
        TextOutA(hdc, sbX + sidebarW - 75, my, buf, (int)strlen(buf));
        DrawProgressBar(hdc, sbX + 12, my + 14, sidebarW - 24, 7, sim.oxygen / 21.0f, COLOR_EMERALD);

        // Metric 5: Magnetosphere
        my += 28;
        SetTextColor(hdc, COLOR_TEXT_PRI);
        TextOutA(hdc, sbX + 12, my, "Magnetosphere / Shielding", 25);
        sprintf(buf, "%.2f Gauss", sim.magnet);
        SetTextColor(hdc, COLOR_PURPLE);
        TextOutA(hdc, sbX + sidebarW - 85, my, buf, (int)strlen(buf));
        DrawProgressBar(hdc, sbX + 12, my + 14, sidebarW - 24, 7, sim.magnet / 0.6f, COLOR_PURPLE);

        // Interventions Section
        my += 30;
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, my, "ORBITAL & SURFACE INTERVENTIONS", 31);

        int gridY = my + 18;
        int btnW = (sidebarW - 30) / 2;
        int btnH = 38;

        AddButton(BID_ACT_SOLAR_MIRROR, sbX + 12, gridY, btnW, btnH, "Orbital Solar Mirror", "+1.8C (350E, 150M)", 1);
        AddButton(BID_ACT_ATMO_PROC, sbX + 18 + btnW, gridY, btnW, btnH, "Atmo Gas Injector", "+0.05atm (180M, 100E)", 1);

        AddButton(BID_ACT_COMET_DROP, sbX + 12, gridY + btnH + 6, btnW, btnH, "Redirect Ice Comet", "+2.5% Water (300V)", 1);
        AddButton(BID_ACT_BIOSEED, sbX + 18 + btnW, gridY + btnH + 6, btnW, btnH, "Extremophile Lichen", "+0.9% O2 (120F, 100E)", 1);

        AddButton(BID_ACT_CORE_DYNAMO, sbX + 12, gridY + (btnH + 6) * 2, btnW, btnH, "Core Dynamo Ring", "+0.08G Mag (400M, 250E)", 1);
        AddButton(BID_ACT_ALGAE, sbX + 18 + btnW, gridY + (btnH + 6) * 2, btnW, btnH, "Ocean Algae Seed", "+1.4% O2 (200V, 150F)", 1);

        // Continuous Facilities
        int facY = gridY + (btnH + 6) * 3 + 12;
        SetTextColor(hdc, COLOR_BLUE);
        TextOutA(hdc, sbX + 12, facY, "CONTINUOUS FACILITIES ONLINE", 28);

        FillSolidRect(hdc, sbX + 12, facY + 16, sidebarW - 24, 60, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, facY + 16, sidebarW - 24, 60, COLOR_BORDER);

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_TEXT_PRI);
        sprintf(buf, "Solar Mirrors: %d Deployed   |   Atmo Processors: %d Online", sim.solarMirrors, sim.atmoProcessors);
        TextOutA(hdc, sbX + 20, facY + 24, buf, (int)strlen(buf));
        sprintf(buf, "Bioseeding Stations: %d Active | Core Dynamos: %d Primed", sim.bioseedStations, sim.coreDynamos);
        TextOutA(hdc, sbX + 20, facY + 44, buf, (int)strlen(buf));
    }
    // TAB 1: FLEET
    else if (sim.activeTab == 1) {
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, contentY, "ARK FLEET ROSTER (5 ACTIVE)", 27);

        int sy = contentY + 18;
        int shipCardH = 62;

        for (int i = 0; i < 5; i++) {
            FillSolidRect(hdc, sbX + 12, sy, sidebarW - 24, shipCardH, COLOR_BG_CARD);
            FrameSolidRect(hdc, sbX + 12, sy, sidebarW - 24, shipCardH, COLOR_BORDER);

            SetTextColor(hdc, fleet[i].color);
            SelectObject(hdc, hFontBold);
            TextOutA(hdc, sbX + 18, sy + 6, fleet[i].name, (int)strlen(fleet[i].name));

            SetTextColor(hdc, COLOR_TEXT_DIM);
            SelectObject(hdc, hFontSmall);
            sprintf(buf, "Status: %s | Hull: %d%%", fleet[i].status, fleet[i].hull);
            TextOutA(hdc, sbX + 18, sy + 22, buf, (int)strlen(buf));

            sprintf(buf, "Role: %s // %s", fleet[i].role, fleet[i].mission);
            TextOutA(hdc, sbX + 18, sy + 34, buf, (int)strlen(buf));

            if (i == 0) {
                AddButton(BID_ORDER_GEN_HOLD, sbX + sidebarW - 140, sy + 10, 56, 18, "Hold", NULL, 1);
                AddButton(BID_ORDER_GEN_BOOST, sbX + sidebarW - 78, sy + 10, 56, 18, "Boost", NULL, 1);
            } else if (i == 1) {
                AddButton(BID_ORDER_VAN_RATION, sbX + sidebarW - 140, sy + 10, 56, 18, "Opt Yield", NULL, 1);
                AddButton(BID_ORDER_VAN_ORBIT, sbX + sidebarW - 78, sy + 10, 56, 18, "Low Orbit", NULL, 1);
            } else if (i == 2) {
                AddButton(BID_ORDER_AEO_SCAN, sbX + sidebarW - 84, sy + 10, 62, 18, "Scan Orbit", NULL, 1);
            } else if (i == 3) {
                AddButton(BID_ORDER_DRA_MINE, sbX + sidebarW - 140, sy + 10, 56, 18, "Mine Belt", NULL, 1);
                AddButton(BID_ORDER_DRA_SCOOP, sbX + sidebarW - 78, sy + 10, 56, 18, "Scoop Ice", NULL, 1);
            } else if (i == 4) {
                AddButton(BID_ORDER_TIT_LOOP, sbX + sidebarW - 140, sy + 10, 56, 18, "Route", NULL, 1);
                AddButton(BID_ORDER_TIT_HOLD, sbX + sidebarW - 78, sy + 10, 56, 18, "Hold", NULL, 1);
            }

            sy += shipCardH + 6;
        }

        // Logistics lines
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, sy + 6, "AUTOMATED FREIGHT CONVEYORS", 27);
        FillSolidRect(hdc, sbX + 12, sy + 22, sidebarW - 24, 48, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, sy + 22, sidebarW - 24, 48, COLOR_BORDER);
        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_EMERALD);
        TextOutA(hdc, sbX + 20, sy + 28, "Tartarus Belt -> Planet Depot: Active (15 t/cyc)", 48);
        TextOutA(hdc, sbX + 20, sy + 44, "Boreas Ice Rings -> Genesis: Active (10 t/cyc)", 46);
    }
    // TAB 2: COLONY
    else if (sim.activeTab == 2) {
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, contentY, "SURFACE HABITAT DOMES", 21);

        FillSolidRect(hdc, sbX + 12, contentY + 18, sidebarW - 24, 76, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, contentY + 18, sidebarW - 24, 76, COLOR_BORDER);

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_TEXT_PRI);
        sprintf(buf, "Active Colonists: %d / %d Housing Capacity", sim.colonists, sim.housingCap);
        TextOutA(hdc, sbX + 20, contentY + 26, buf, (int)strlen(buf));
        DrawProgressBar(hdc, sbX + 20, contentY + 42, sidebarW - 40, 7, (float)sim.colonists / (float)sim.housingCap, COLOR_EMERALD);

        sprintf(buf, "Cryo-Sleepers in Genesis Vaults: %d", sim.cryoSleepers);
        TextOutA(hdc, sbX + 20, contentY + 54, buf, (int)strlen(buf));
        sprintf(buf, "Colonist Morale: %.0f%% (Optimistic) | Life Support: 99.4%% Stable", sim.morale);
        TextOutA(hdc, sbX + 20, contentY + 68, buf, (int)strlen(buf));

        // Expansion Projects
        int expY = contentY + 104;
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, expY, "COLONY INFRASTRUCTURE EXPANSION", 32);

        int btnW = (sidebarW - 30) / 2;
        int btnH = 38;
        AddButton(BID_COL_AWAKEN, sbX + 12, expY + 18, btnW, btnH, "Awaken 2,500 Sleepers", "+2.5k Pop (-100 Food)", 1);
        AddButton(BID_COL_DOME, sbX + 18 + btnW, expY + 18, btnW, btnH, "Expand Habitats", "+15k Housing (450M, 150E)", 1);

        AddButton(BID_COL_HYDRO, sbX + 12, expY + 18 + btnH + 6, btnW, btnH, "Hydroponic Tower", "+25 Food/cyc (200M, 100V)", 1);
        AddButton(BID_COL_SOLAR, sbX + 18 + btnW, expY + 18 + btnH + 6, btnW, btnH, "Surface Solar Grid", "+180 kW (220 Min)", 1);

        int bioY = expY + 18 + (btnH + 6) * 2 + 10;
        SetTextColor(hdc, COLOR_BLUE);
        TextOutA(hdc, sbX + 12, bioY, "BIOSPHERE INTEGRATION", 21);
        DrawProgressBar(hdc, sbX + 12, bioY + 18, sidebarW - 24, 8, sim.habitability * 0.009f, COLOR_CYAN);
    }
    // TAB 3: ECONOMY
    else if (sim.activeTab == 3) {
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, contentY, "SECTOR RESOURCE LOOPS", 21);

        int ey = contentY + 18;
        FillSolidRect(hdc, sbX + 12, ey, sidebarW - 24, 120, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, ey, sidebarW - 24, 120, COLOR_BORDER);

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_EMERALD);
        sprintf(buf, "ENERGY: +%d kW Gen  |  -%d kW Drain  ->  Net: %+d kW/cyc",
                600 + (sim.surfaceSolar * 60),
                200 + (sim.solarMirrors * 75) + (sim.atmoProcessors * 60) + (sim.coreDynamos * 80) + (sim.colonists / 1000) * 5,
                sim.deltaEnergy);
        TextOutA(hdc, sbX + 20, ey + 12, buf, (int)strlen(buf));

        SetTextColor(hdc, COLOR_BLUE);
        sprintf(buf, "MINERALS: +%d t Mined  |  -%d t Built  ->  Net: %+d t/cyc",
                20 + (strcmp(fleet[3].status, "Mining Belt") == 0 ? 25 : 10),
                sim.atmoProcessors * 3,
                sim.deltaMinerals);
        TextOutA(hdc, sbX + 20, ey + 38, buf, (int)strlen(buf));

        SetTextColor(hdc, COLOR_PURPLE);
        sprintf(buf, "VOLATILES: +%d t Scooped  |  -%d t Injected  ->  Net: %+d t/cyc",
                14 + (strcmp(fleet[3].status, "Scooping Ring") == 0 ? 18 : 6),
                sim.solarMirrors * 2,
                sim.deltaVolatiles);
        TextOutA(hdc, sbX + 20, ey + 64, buf, (int)strlen(buf));

        SetTextColor(hdc, COLOR_AMBER);
        sprintf(buf, "FOOD: +%d t Harvested  |  -%d t Eaten  ->  Net: %+d t/cyc",
                15 + (sim.hydroTowers * 12),
                sim.colonists / 2000,
                sim.deltaFood);
        TextOutA(hdc, sbX + 20, ey + 90, buf, (int)strlen(buf));

        int intelY = ey + 132;
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, intelY, "CELESTIAL SECTOR INTEL", 22);

        FillSolidRect(hdc, sbX + 12, intelY + 16, sidebarW - 24, 110, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, intelY + 16, sidebarW - 24, 110, COLOR_BORDER);

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_TEXT_PRI);
        TextOutA(hdc, sbX + 20, intelY + 24, "Helios Core: Type M1V Red Dwarf Star", 36);
        TextOutA(hdc, sbX + 20, intelY + 42, "Tartarus Ring: Heavy Ferrous & Titanium Deposits", 48);
        TextOutA(hdc, sbX + 20, intelY + 60, "Boreas Minor: Nitrogen/Ammonia Ice Shell", 40);
        TextOutA(hdc, sbX + 20, intelY + 78, "Zephyr Station: Fleet Automated Drydocks", 40);
        SetTextColor(hdc, COLOR_AMBER);
        TextOutA(hdc, sbX + 20, intelY + 96, "Precursor Resonance: Signal 420 MHz detected!", 46);
    }

    // 5. Draw All Buttons
    for (int i = 0; i < g_buttonCount; i++) {
        UIButton* b = &g_buttons[i];
        int bx = b->rect.left;
        int by = b->rect.top;
        int bw = b->rect.right - b->rect.left;
        int bh = b->rect.bottom - b->rect.top;

        int isTab = (b->id >= BID_TAB_TERRA && b->id <= BID_TAB_ECONOMY);
        int isActiveTab = isTab && (b->id - BID_TAB_TERRA == sim.activeTab);

        COLORREF btnBg = COLOR_BG_CARD;
        COLORREF btnBorder = COLOR_BORDER;
        COLORREF btnText = COLOR_TEXT_PRI;

        if (isActiveTab) {
            btnBg = COLOR_BG_CARD_ACT;
            btnBorder = COLOR_CYAN;
            btnText = COLOR_CYAN;
        } else if (isTab) {
            btnBg = COLOR_BG_PANEL_DARK;
            btnBorder = COLOR_BORDER;
            btnText = COLOR_TEXT_DIM;
        }

        FillSolidRect(hdc, bx, by, bw, bh, btnBg);
        FrameSolidRect(hdc, bx, by, bw, bh, btnBorder);

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, btnText);

        if (b->subtext[0] != '\0') {
            TextOutA(hdc, bx + 6, by + 4, b->text, (int)strlen(b->text));
            SetTextColor(hdc, COLOR_TEXT_DIM);
            TextOutA(hdc, bx + 6, by + 18, b->subtext, (int)strlen(b->subtext));
        } else {
            SIZE sz;
            GetTextExtentPoint32A(hdc, b->text, (int)strlen(b->text), &sz);
            int tx = bx + (bw - sz.cx) / 2;
            int ty = by + (bh - sz.cy) / 2;
            TextOutA(hdc, tx, ty, b->text, (int)strlen(b->text));
        }
    }

    // 6. Draw Bottom Footer Bar
    int footY = height - footerH;
    FillSolidRect(hdc, 0, footY, width, footerH, COLOR_BG_PANEL_DARK);
    FillSolidRect(hdc, 0, footY, width, 1, COLOR_BORDER);

    // Speed controls
    AddButton(BID_SPEED_PAUSE, 8, footY + 5, 48, 22, "PAUSE", NULL, 1);
    AddButton(BID_SPEED_1X, 60, footY + 5, 34, 22, "1x", NULL, 1);
    AddButton(BID_SPEED_2X, 98, footY + 5, 34, 22, "2x", NULL, 1);
    AddButton(BID_SPEED_5X, 136, footY + 5, 34, 22, "5x", NULL, 1);
    AddButton(BID_AUDIO_TOGGLE, 176, footY + 5, 84, 22, g_soundEnabled ? "AUDIO: ON" : "AUDIO: OFF", NULL, 1);

    // Footer Log Message Banner
    SetTextColor(hdc, sim.logIsWarn ? COLOR_ROSE : COLOR_CYAN);
    SelectObject(hdc, hFontSmall);
    TextOutA(hdc, 276, footY + 9, sim.logIsWarn ? "[ALERT] " : "[FLEET DISPATCH] ", (int)strlen(sim.logIsWarn ? "[ALERT] " : "[FLEET DISPATCH] "));
    SetTextColor(hdc, COLOR_TEXT_PRI);
    TextOutA(hdc, 396, footY + 9, sim.logMsg, (int)strlen(sim.logMsg));

    // Right-aligned engine tag
    SetTextColor(hdc, COLOR_TEXT_DIM);
    TextOutA(hdc, width - 150, footY + 9, "KCosmic Native v0.3", 19);

    // Cleanup GDI objects
    SelectObject(hdc, hOldFont);
    DeleteObject(hFontMain);
    DeleteObject(hFontBold);
    DeleteObject(hFontTitle);
    DeleteObject(hFontSmall);
}

// --- Window Procedure & Interaction ---
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static int winW = 1100;
    static int winH = 720;
    static HDC memDC = NULL;
    static HBITMAP memBmp = NULL;
    static HBITMAP oldBmp = NULL;
    static int lastTickTime = 0;

    switch (msg) {
        case WM_CREATE: {
            InitSimulation();
            SetTimer(hwnd, 1, 33, NULL); // ~30 FPS timer
            lastTickTime = GetTickCount();
            return 0;
        }

        case WM_TIMER: {
            int now = GetTickCount();
            if (!sim.paused && sim.speed > 0) {
                sim.time += 0.033f * sim.speed;
                sim.cycle += 0.003f * sim.speed;

                if (now - lastTickTime >= 1000) {
                    SimTick();
                    lastTickTime = now;
                }
            }
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        case WM_SIZE: {
            winW = LOWORD(lParam);
            winH = HIWORD(lParam);
            if (memDC) {
                SelectObject(memDC, oldBmp);
                DeleteObject(memBmp);
                DeleteDC(memDC);
                memDC = NULL;
            }
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1; // Prevent flicker

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            if (!memDC) {
                memDC = CreateCompatibleDC(hdc);
                memBmp = CreateCompatibleBitmap(hdc, winW, winH);
                oldBmp = (HBITMAP)SelectObject(memDC, memBmp);
            }

            RenderUI(memDC, winW, winH);
            BitBlt(hdc, 0, 0, winW, winH, memDC, 0, 0, SRCCOPY);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);

            // Check button clicks first
            for (int i = 0; i < g_buttonCount; i++) {
                if (PtInRect(&g_buttons[i].rect, (POINT){mx, my})) {
                    int bid = g_buttons[i].id;
                    if (bid >= BID_TAB_TERRA && bid <= BID_TAB_ECONOMY) {
                        sim.activeTab = bid - BID_TAB_TERRA;
                        PlaySoundFx(SFX_CLICK);
                    } else if (bid == BID_FOCUS_PLANET) {
                        sim.camX = -bodies[1].currX + (winW - 380) / 2 + sim.camX;
                        sim.camY = -bodies[1].currY + (winH - 80) / 2 + sim.camY;
                        sim.selectedType = 2;
                        PlaySoundFx(SFX_CLICK);
                    } else if (bid == BID_FOCUS_ARK) {
                        sim.camX = -fleet[0].currX + (winW - 380) / 2 + sim.camX;
                        sim.camY = -fleet[0].currY + (winH - 80) / 2 + sim.camY;
                        sim.selectedType = 5;
                        sim.selectedIndex = 0;
                        PlaySoundFx(SFX_CLICK);
                    } else if (bid == BID_ZOOM_IN) {
                        sim.zoom *= 1.25f;
                        if (sim.zoom > 2.5f) sim.zoom = 2.5f;
                        PlaySoundFx(SFX_CLICK);
                    } else if (bid == BID_ZOOM_OUT) {
                        sim.zoom *= 0.8f;
                        if (sim.zoom < 0.4f) sim.zoom = 0.4f;
                        PlaySoundFx(SFX_CLICK);
                    } else if (bid == BID_RESET_VIEW) {
                        sim.camX = 0;
                        sim.camY = 0;
                        sim.zoom = 1.0f;
                        PlaySoundFx(SFX_CLICK);
                    } else if (bid == BID_SPEED_PAUSE) {
                        sim.paused = 1;
                        sim.speed = 0;
                        PlaySoundFx(SFX_CLICK);
                    } else if (bid == BID_SPEED_1X) {
                        sim.paused = 0;
                        sim.speed = 1;
                        PlaySoundFx(SFX_CLICK);
                    } else if (bid == BID_SPEED_2X) {
                        sim.paused = 0;
                        sim.speed = 2;
                        PlaySoundFx(SFX_CLICK);
                    } else if (bid == BID_SPEED_5X) {
                        sim.paused = 0;
                        sim.speed = 5;
                        PlaySoundFx(SFX_CLICK);
                    } else if (bid == BID_AUDIO_TOGGLE) {
                        g_soundEnabled = !g_soundEnabled;
                        PlaySoundFx(SFX_CLICK);
                    } else if (bid >= BID_ACT_SOLAR_MIRROR && bid <= BID_ACT_ALGAE) {
                        HandleIntervention(bid);
                    } else if (bid >= BID_COL_AWAKEN && bid <= BID_COL_SOLAR) {
                        HandleColonyProject(bid);
                    } else if (bid >= BID_ORDER_GEN_HOLD && bid <= BID_ORDER_TIT_HOLD) {
                        HandleShipOrder(bid);
                    } else if (bid == BID_SEL_CLOSE) {
                        sim.selectedType = 0;
                        PlaySoundFx(SFX_CLICK);
                    } else if (bid == BID_SEL_ACT1) {
                        if (sim.selectedType == 2) sim.activeTab = 0;
                        else if (sim.selectedType == 5) sim.activeTab = 1;
                        PlaySoundFx(SFX_CLICK);
                    } else if (bid == BID_SEL_ACT2) {
                        if (sim.selectedType == 5 && sim.selectedIndex >= 0) {
                            strcpy(fleet[sim.selectedIndex].status, "Stationary Orbit");
                            SetLogMsg("Ship repositioned to stable geosynchronous orbit.", 0);
                        } else if (sim.selectedType == 2) {
                            HandleIntervention(BID_ACT_SOLAR_MIRROR);
                        }
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            }

            // Check if clicked in Viewport
            if (mx < winW - 380 && my >= 46 && my < winH - 34) {
                // Check Ships
                int found = 0;
                for (int i = 0; i < 5; i++) {
                    float dist = (float)hypot(fleet[i].currX - mx, fleet[i].currY - my);
                    if (dist < 18.0f) {
                        sim.selectedType = 5;
                        sim.selectedIndex = i;
                        found = 1;
                        PlaySoundFx(SFX_CLICK);
                        break;
                    }
                }
                // Check Celestial Bodies
                if (!found) {
                    for (int i = 1; i < 4; i++) {
                        float dist = (float)hypot(bodies[i].currX - mx, bodies[i].currY - my);
                        if (dist < bodies[i].radius * sim.zoom + 10.0f) {
                            sim.selectedType = i + 1; // 2=planet, 3=moon, 4=station
                            found = 1;
                            PlaySoundFx(SFX_CLICK);
                            break;
                        }
                    }
                }
                if (!found) {
                    float dist = (float)hypot(bodies[0].currX - mx, bodies[0].currY - my);
                    if (dist < bodies[0].radius * sim.zoom + 10.0f) {
                        sim.selectedType = 1; // 1=sun
                        found = 1;
                        PlaySoundFx(SFX_CLICK);
                    }
                }

                // If not clicking an object, start panning drag
                sim.isDragging = 1;
                sim.lastMouseX = mx;
                sim.lastMouseY = my;
                SetCapture(hwnd);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }

        case WM_MOUSEMOVE: {
            if (sim.isDragging) {
                int mx = LOWORD(lParam);
                int my = HIWORD(lParam);
                sim.camX += (mx - sim.lastMouseX);
                sim.camY += (my - sim.lastMouseY);
                sim.lastMouseX = mx;
                sim.lastMouseY = my;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }

        case WM_LBUTTONUP: {
            if (sim.isDragging) {
                sim.isDragging = 0;
                ReleaseCapture();
            }
            return 0;
        }

        case WM_MOUSEWHEEL: {
            short delta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (delta > 0) sim.zoom *= 1.15f;
            else sim.zoom *= 0.85f;
            if (sim.zoom < 0.4f) sim.zoom = 0.4f;
            if (sim.zoom > 2.5f) sim.zoom = 2.5f;
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        case WM_KEYDOWN: {
            switch (wParam) {
                case VK_SPACE:
                    sim.paused = !sim.paused;
                    PlaySoundFx(SFX_CLICK);
                    break;
                case '1':
                    sim.paused = 0; sim.speed = 1;
                    PlaySoundFx(SFX_CLICK);
                    break;
                case '2':
                    sim.paused = 0; sim.speed = 2;
                    PlaySoundFx(SFX_CLICK);
                    break;
                case '5':
                    sim.paused = 0; sim.speed = 5;
                    PlaySoundFx(SFX_CLICK);
                    break;
                case VK_TAB:
                    sim.activeTab = (sim.activeTab + 1) % 4;
                    PlaySoundFx(SFX_CLICK);
                    break;
                case 'M':
                case 'm':
                    g_soundEnabled = !g_soundEnabled;
                    PlaySoundFx(SFX_CLICK);
                    break;
                case 'R':
                case 'r':
                    sim.camX = 0; sim.camY = 0; sim.zoom = 1.0f;
                    PlaySoundFx(SFX_CLICK);
                    break;
                case 'P':
                case 'p':
                    sim.selectedType = 2;
                    sim.camX = -bodies[1].currX + (winW - 380) / 2 + sim.camX;
                    sim.camY = -bodies[1].currY + (winH - 80) / 2 + sim.camY;
                    PlaySoundFx(SFX_CLICK);
                    break;
                case VK_ESCAPE:
                    sim.selectedType = 0;
                    break;
            }
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        case WM_DESTROY: {
            KillTimer(hwnd, 1);
            if (memDC) {
                SelectObject(memDC, oldBmp);
                DeleteObject(memBmp);
                DeleteDC(memDC);
            }
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KCosmicWin32Class";

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL; // Handled in WM_PAINT

    if (!RegisterClassA(&wc)) {
        return 0;
    }

    HWND hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "KCosmic - Interstellar Fleet Logistics & Planetary Terraforming",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1140, 740,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return (int)msg.wParam;
}
