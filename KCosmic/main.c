#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define M_PI 3.14159265358979323846

// --- UI Colors & CRT Phosphor Palettes ---
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

typedef struct {
    const char* name;
    COLORREF bgDeep;
    COLORREF bgPanel;
    COLORREF bgPanelDark;
    COLORREF bgCard;
    COLORREF bgCardHov;
    COLORREF border;
    COLORREF primary;
    COLORREF secondary;
    COLORREF accent;
    COLORREF textBright;
    COLORREF textPri;
    COLORREF textDim;
    COLORREF grid;
} CRTTheme;

static CRTTheme g_crtThemes[] = {
    {
        "CRT: P4 Cyan",
        RGB(5, 8, 17),
        RGB(10, 17, 32),
        RGB(7, 11, 22),
        RGB(15, 28, 51),
        RGB(22, 40, 74),
        RGB(28, 49, 86),
        RGB(0, 240, 255),
        RGB(56, 189, 248),
        RGB(16, 185, 129),
        RGB(248, 250, 252),
        RGB(203, 213, 225),
        RGB(100, 116, 139),
        RGB(20, 36, 64)
    },
    {
        "CRT: P3 Amber",
        RGB(10, 7, 3),
        RGB(20, 14, 6),
        RGB(13, 9, 4),
        RGB(28, 20, 9),
        RGB(42, 30, 13),
        RGB(74, 48, 16),
        RGB(245, 158, 11),
        RGB(251, 191, 36),
        RGB(251, 146, 60),
        RGB(255, 251, 235),
        RGB(254, 215, 170),
        RGB(154, 106, 56),
        RGB(58, 38, 14)
    },
    {
        "CRT: P1 Green",
        RGB(3, 13, 7),
        RGB(6, 22, 12),
        RGB(4, 15, 8),
        RGB(10, 36, 20),
        RGB(16, 54, 30),
        RGB(19, 78, 42),
        RGB(16, 185, 129),
        RGB(52, 211, 153),
        RGB(110, 231, 183),
        RGB(236, 253, 245),
        RGB(167, 243, 208),
        RGB(61, 122, 88),
        RGB(14, 52, 28)
    },
    {
        "CRT: P7 White",
        RGB(8, 10, 14),
        RGB(15, 19, 26),
        RGB(9, 12, 18),
        RGB(24, 29, 38),
        RGB(35, 42, 55),
        RGB(51, 65, 85),
        RGB(226, 232, 240),
        RGB(148, 163, 184),
        RGB(203, 213, 225),
        RGB(255, 255, 255),
        RGB(203, 213, 225),
        RGB(100, 116, 139),
        RGB(32, 40, 54)
    }
};

static int g_crtTheme = 0;
static int g_showGrid = 1;
static int g_phosphorGlow = 1;

// --- Sound Effects ---
#define SFX_CLICK   1
#define SFX_SUCCESS 2
#define SFX_WARN    3
#define SFX_DEPLOY  4
#define SFX_ALARM   5
#define SFX_EXPLODE 6

// Phase 9: Cosmic Crisis Types
typedef enum {
    CRISIS_NONE = 0,
    CRISIS_FLARE,
    CRISIS_ASTEROID,
    CRISIS_QUAKE,
    CRISIS_BLIGHT,
    CRISIS_STORM
} CrisisType;

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
        case SFX_ALARM:
            Beep(750, 70);
            Beep(920, 70);
            Beep(750, 70);
            break;
        case SFX_EXPLODE:
            Beep(180, 120);
            Beep(120, 180);
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

// --- Exoplanet Classification Canonical Types ---
typedef enum {
    CLASS_BARREN_ROCK = 0,
    CLASS_TOXIC_GREENHOUSE = 1,
    CLASS_FROZEN_TUNDRA = 2,
    CLASS_OCEAN_WORLD = 3,
    CLASS_PRIMORDIAL_GAIA = 4
} PlanetClass;

typedef struct {
    PlanetClass pClass;
    char code[8];
    char name[32];
    char desc[96];
    COLORREF color;
    COLORREF badgeBg;
    COLORREF badgeBorder;
    float defaultPressure;
    float defaultTemp;
    float defaultWater;
    float defaultOxygen;
    float defaultNitrogen;
    float defaultGreenhouse;
    float defaultMagnet;
    float minHabitability;
    float maxHabitability;
    float mineralMult;
    float volatileMult;
} ExoplanetClassInfo;

static ExoplanetClassInfo g_exoplanetClasses[5] = {
    { CLASS_BARREN_ROCK, "BR-I", "Barren Rock", "Airless metallic crust, heavily cratered, basaltic ridges.", RGB(180, 110, 70), RGB(35, 20, 15), RGB(180, 110, 70), 0.05f, -65.0f, 0.0f, 0.2f, 2.0f, 30.0f, 0.05f, 0.0f, 35.0f, 2.2f, 0.2f },
    { CLASS_TOXIC_GREENHOUSE, "TG-II", "Toxic Greenhouse", "Supercritical CO2/sulfur atmosphere, runaway thermal mantle.", RGB(245, 158, 11), RGB(40, 25, 5), RGB(245, 158, 11), 3.40f, 185.0f, 2.0f, 0.1f, 3.5f, 320.0f, 0.12f, 0.0f, 25.0f, 1.4f, 1.8f },
    { CLASS_FROZEN_TUNDRA, "FT-III", "Frozen Tundra", "Sub-zero cryosphere, solid methane glaciers, permafrost sheets.", RGB(56, 189, 248), RGB(10, 25, 45), RGB(56, 189, 248), 0.45f, -88.0f, 68.0f, 2.4f, 14.5f, 45.0f, 0.35f, 5.0f, 55.0f, 0.9f, 2.5f },
    { CLASS_OCEAN_WORLD, "OW-IV", "Ocean World", "Global hyper-deep pelagic abyss, subterranean thermal vents.", RGB(14, 165, 233), RGB(8, 28, 52), RGB(14, 165, 233), 1.15f, 18.0f, 98.0f, 11.5f, 64.0f, 90.0f, 0.48f, 30.0f, 85.0f, 0.7f, 2.0f },
    { CLASS_PRIMORDIAL_GAIA, "PG-V", "Primordial Gaia", "Nascent biosphere, proto-chlorophyll flora, stable hydrosphere.", RGB(16, 185, 129), RGB(8, 38, 22), RGB(16, 185, 129), 0.98f, 14.2f, 54.0f, 18.8f, 76.5f, 102.0f, 0.52f, 60.0f, 100.0f, 1.2f, 1.3f }
};

typedef struct {
    char id[16];
    char name[32];
    char type[32];
    PlanetClass pClass;
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
    int parentIndex; // -1 for star / sun-orbiting; index in bodies[] for moon/station

    // Biometrics for this planet
    float pressure;
    float temp;
    float water;
    float oxygen;
    float nitrogen;
    float greenhouse;
    float magnet;
    float habitability;
} CelestialBody;

#define MAX_SYSTEM_BODIES 12
#define MAX_STAR_SYSTEMS 8

typedef struct {
    char id[16];
    char name[32];
    char spectralClass[32];
    char desc[96];
    int bodyCount;
    CelestialBody celestials[MAX_SYSTEM_BODIES];
    int activePlanetIndex;
    COLORREF starColor;
    COLORREF starCorona;
} StarSystem;

static StarSystem g_systems[MAX_STAR_SYSTEMS];
static int g_systemCount = 0;
static int g_currentSystem = 0;

#define CURR_SYS (g_systems[g_currentSystem])
#define bodies (g_systems[g_currentSystem].celestials)

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

    // Phase 8: Demographics & Colony Infrastructure
    int pctLaborers;
    int pctAgronomists;
    int pctEngineers;
    int pctScientists;
    int demoFocus;         // 0=Balanced, 1=Agronomy, 2=Geo-Eng, 3=Research
    int rationPolicy;      // 0=Spartan, 1=Standard, 2=Abundant
    int geodesicDomes;
    int subterraneanVaults;
    int domedMegacities;
    int aeroponicFarms;
    int algalVats;

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
    float nitrogen;
    float greenhouse;
    float magnet;
    float habitability;

    // Active Facilities & Terraforming Modules
    int solarMirrors;
    int mirrorMode;        // 0=Focus Insolation (Heat), 1=Solar Shade (Cool)
    int atmoProcessors;
    int atmoMode;          // 0=Buffer Injection (+P), 1=Toxic Scrubbing (-P)
    int nitrogenExtractors;
    int greenhouseStations;
    int greenhouseMode;    // 0=PFC Super-Warming (+GHG), 1=Aerosol Cloud (-GHG)
    int bioseedStations;
    int coreDynamos;
    int hydroTowers;
    int surfaceSolar;

    // Logistics Infrastructure
    int orbitalDocks;
    int fuelDepots;
    int massDrivers;

    // Phase 9: Cosmic Crisis & Hazards
    int crisisActive;
    int crisisType;        // CrisisType
    float crisisTimer;
    float crisisMaxTime;
    int crisisSeverity;    // 1 to 5
    int crisisMitigated;
    float crisisShake;
    int earlyWarningRadar; // Tier
    int shieldDeflector;   // Tier
    int repairDrones;      // Tier

    // Camera
    float camX, camY;
    float zoom;
    int isDragging;
    int lastMouseX, lastMouseY;

    // Selection
    int selectedType; // 0=none, 1=sun, 2=planet, 3=moon, 4=station, 5=ship
    int selectedIndex;

    // Tab: 0=Terraform, 1=Fleet, 2=Colony, 3=Research, 4=Megas, 5=Xeno, 6=Hazards, 7=Economy
    int activeTab;

    // Phase 12: Alien Xenobiology & Ancient Precursor Relics
    int xenoSiteStatus[4];    // 0=Surveyed, 1=Excavating, 2=Excavated
    int xenoSiteProgress[4];  // 0 to 100%
    int xenoRelicFound[4];    // 0 or 1
    int precursorTech[4];     // 0 or 1
    float xenoScanAnim;

    // Phase 11: Orbital Megastructures & Planetary Defense Stations
    int orbitalRingStage;     // 0 to 3
    int starElevatorStage;    // 0 to 3
    int shieldGridStage;      // 0 to 3
    int defenseStationStage;  // 0 to 3
    int shieldHP;
    int shieldMaxHP;
    int shieldMode;           // 0=Balanced, 1=Fortified, 2=Standby
    float shieldFlareAnim;
    float defenseFireAnim;

    // Phase 10: Interstellar Research Tree & Terraforming Breakthroughs
    int science;
    int deltaScience;
    int activeTech;
    float techProgress[9];
    int techResearched[9];
    int breakthroughFanfare;
    char lastBreakthrough[48];

    // Log message
    char logMsg[128];
    int logIsWarn;
} Simulation;

typedef struct {
    char id[16];
    char name[32];
    char from[32];
    char to[32];
    char cargo[16];
    int active;
    int freighters;
    int baseYield;
    int costMin;
    int costEnergy;
    float progress;
    float speed;
    COLORREF color;
} TradeRoute;

#define MAX_TECHS 9

typedef enum {
    TECH_PROP_RAMJETS = 0,
    TECH_PROP_WARP,
    TECH_PROP_ANTIMATTER,
    TECH_BIO_CATALYSTS,
    TECH_BIO_THERMAL,
    TECH_BIO_ADAPTED,
    TECH_GEO_DYNAMO,
    TECH_GEO_SUNSHADE,
    TECH_GEO_STABILIZATION
} TechId;

typedef struct {
    int id;
    int branch; // 0=Propulsion, 1=Biosphere, 2=Geo-Eng
    int tier;   // 1, 2, 3
    int isBreakthrough;
    char name[36];
    char code[24];
    char desc[96];
    char effect[96];
    int costScience;
    int costEnergy;
    int costMinerals;
    int prereq;
} TechDef;

static TechDef g_techs[MAX_TECHS] = {
    { TECH_PROP_RAMJETS, 0, 1, 0, "Bussard Ramjets", "PROP-T1", "Magnetic scoop fields harvesting stellar H2.", "+30% Fleet Speed, -20% Transit Cycles", 400, 150, 100, -1 },
    { TECH_PROP_WARP, 0, 2, 0, "Alcubierre Warp Harmonics", "PROP-T2", "Spacetime curvature coils for interstellar jumps.", "2x Fleet Velocity, +50% Freighter Capacity", 950, 300, 250, TECH_PROP_RAMJETS },
    { TECH_PROP_ANTIMATTER, 0, 3, 1, "Antimatter Drives", "PROP-T3", "Positron-antiproton annihilation propulsion matrix.", "BREAKTHROUGH: 3x Speed, -50% Transit, +100% Trade", 2200, 600, 400, TECH_PROP_WARP },

    { TECH_BIO_CATALYSTS, 1, 1, 0, "Atmospheric Catalysts", "BIO-T1", "Synthetic catalysts accelerating tropospheric O2.", "+40% Processor O2/N2 Production", 450, 120, 80, -1 },
    { TECH_BIO_THERMAL, 1, 2, 0, "Cryo-Aquifer Hydro-Melting", "BIO-T2", "Geothermal coils melting sub-surface ice aquifers.", "+50% Water Conversion, +35% Farm Yield", 1000, 250, 200, TECH_BIO_CATALYSTS },
    { TECH_BIO_ADAPTED, 1, 3, 1, "Genetic Adapted Biomes", "BIO-T3", "Engineered extremophile synthetic xenobiotic flora.", "BREAKTHROUGH: +20% Habitation, 2x Food, +15 Morale", 2400, 350, 300, TECH_BIO_THERMAL },

    { TECH_GEO_DYNAMO, 2, 1, 0, "Ionospheric Dynamo Coils", "GEO-T1", "Core induction coils generating planetary magnetosphere.", "+60% Magnetosphere, -50% Flare Damage", 500, 200, 200, -1 },
    { TECH_GEO_SUNSHADE, 2, 2, 0, "Orbital Sunshade Array", "GEO-T2", "Occultation array for fine-tuned planetary cooling.", "+75% Mirror Thermal Power, -15C Hot Worlds", 1100, 350, 350, TECH_GEO_DYNAMO },
    { TECH_GEO_STABILIZATION, 2, 3, 1, "Climate Stabilization Matrix", "GEO-T3", "Planetary nanite grid locking atmospheric stability.", "BREAKTHROUGH: Climate Lock, +15% Hab, +20 Morale", 2600, 650, 500, TECH_GEO_SUNSHADE }
};

static Simulation sim;
static Star stars[STAR_COUNT];
static Asteroid asteroids[ASTEROID_COUNT];
// bodies points to current system bodies via macro above
static Ship fleet[5];
static TradeRoute g_tradeRoutes[4];

static void SetLogMsg(const char* txt, int isWarn);

// --- Star System & Planet Management Helpers ---
static CelestialBody* GetActivePlanet(void) {
    int i;
    StarSystem* sys = &CURR_SYS;
    if (sys->activePlanetIndex >= 0 && sys->activePlanetIndex < sys->bodyCount) {
        return &sys->celestials[sys->activePlanetIndex];
    }
    for (i = 1; i < sys->bodyCount; i++) {
        if (sys->celestials[i].isPlanet) return &sys->celestials[i];
    }
    return &sys->celestials[0];
}

static void SyncSimToActivePlanet(void) {
    CelestialBody* p;
    if (g_systemCount <= 0) return;
    p = GetActivePlanet();
    if (p && p->isPlanet) {
        p->pressure = sim.pressure;
        p->temp = sim.temp;
        p->water = sim.water;
        p->oxygen = sim.oxygen;
        p->nitrogen = sim.nitrogen;
        p->greenhouse = sim.greenhouse;
        p->magnet = sim.magnet;
        p->habitability = sim.habitability;
    }
}

static void CalculateHabitability(void);

static void SetActivePlanet(int bodyIdx) {
    StarSystem* sys = &CURR_SYS;
    CelestialBody* p;
    char msg[128];
    if (bodyIdx < 0 || bodyIdx >= sys->bodyCount) return;
    if (!sys->celestials[bodyIdx].isPlanet) return;

    SyncSimToActivePlanet();
    sys->activePlanetIndex = bodyIdx;
    p = &sys->celestials[bodyIdx];

    sim.pressure = p->pressure;
    sim.temp = p->temp;
    sim.water = p->water;
    sim.oxygen = p->oxygen;
    sim.nitrogen = p->nitrogen;
    sim.greenhouse = p->greenhouse;
    sim.magnet = p->magnet;
    CalculateHabitability();

    sprintf(msg, "Active target set: %s [%s: %s]", p->name, g_exoplanetClasses[p->pClass].code, g_exoplanetClasses[p->pClass].name);
    SetLogMsg(msg, 0);
}

static void LoadStarSystem(int sysIdx) {
    CelestialBody* p;
    char msg[128];
    if (g_systemCount <= 0) return;
    SyncSimToActivePlanet();
    g_currentSystem = sysIdx % g_systemCount;
    if (g_currentSystem < 0) g_currentSystem += g_systemCount;

    p = GetActivePlanet();
    sim.pressure = p->pressure;
    sim.temp = p->temp;
    sim.water = p->water;
    sim.oxygen = p->oxygen;
    sim.nitrogen = p->nitrogen;
    sim.greenhouse = p->greenhouse;
    sim.magnet = p->magnet;
    CalculateHabitability();

    sprintf(msg, "Sector warp: %s (%s) - Target: %s [%s]",
        CURR_SYS.name, CURR_SYS.spectralClass, p->name, g_exoplanetClasses[p->pClass].code);
    SetLogMsg(msg, 0);
}

static void GenerateProceduralStarSystem(void);

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

    float n = sim.nitrogen;
    float nScore = 100.0f - (float)fabs(78.0f - n) * 1.3f;
    if (nScore < 0.0f) nScore = 0.0f;
    if (nScore > 100.0f) nScore = 100.0f;

    float ghg = sim.greenhouse;
    float ghgScore = 100.0f - (float)fabs(100.0f - ghg) * 0.9f;
    if (ghgScore < 0.0f) ghgScore = 0.0f;
    if (ghgScore > 100.0f) ghgScore = 100.0f;

    float m = sim.magnet;
    float mScore = (m / 0.5f) * 100.0f;
    if (mScore > 100.0f) mScore = 100.0f;

    float total = (pScore * 0.20f) + (tScore * 0.20f) + (wScore * 0.15f) + (oScore * 0.15f) + (nScore * 0.15f) + (ghgScore * 0.10f) + (mScore * 0.05f);
    if (sim.techResearched[TECH_BIO_ADAPTED]) total += 20.0f;
    if (sim.techResearched[TECH_GEO_STABILIZATION]) total += 15.0f;
    if (sim.shieldGridStage == 3) total += 10.0f;
    if (sim.precursorTech[0]) total += 10.0f;
    if (sim.precursorTech[1]) total += 5.0f;
    if (total < 0.0f) total = 0.0f;
    if (total > 100.0f) total = 100.0f;
    sim.habitability = total;
}

static void GenerateProceduralStarSystem(void) {
    static const char* s_prefixes[] = { "Kepler", "Gliese", "HD", "Trappist", "Wolf", "Ross", "Tau", "Epsilon", "Vega", "Altair" };
    static const char* s_suffixes[] = { "Prime", "Major", "Australis", "Borealis", "Echo", "Frontier", "Zenith", "Vanguard" };
    static const char* s_spectrals[] = { "Red Dwarf (M-V)", "Orange K-Type", "Yellow G-Type", "White F-Type", "Blue Subgiant" };
    static const COLORREF s_starCols[] = { RGB(255, 100, 40), RGB(255, 170, 50), RGB(255, 230, 110), RGB(220, 240, 255), RGB(130, 200, 255) };
    static const COLORREF s_coronas[] = { RGB(80, 25, 10), RGB(90, 45, 15), RGB(100, 75, 20), RGB(50, 70, 110), RGB(30, 60, 130) };
    static const char* greekLetters[] = { "b", "c", "d", "e", "f" };

    int sysSlot, pIdx, sIdx, spIdx, numPlanets, bCount, targetPlanet, p, cType;
    float orbitDist, oldP, oldT, oldW, oldO, oldN, oldG, oldM;
    StarSystem* sys;
    CelestialBody* star;
    CelestialBody* pl;
    CelestialBody* mn;
    CelestialBody* st;
    ExoplanetClassInfo* cInfo;
    char log[128];

    SyncSimToActivePlanet();

    sysSlot = g_systemCount;
    if (sysSlot >= MAX_STAR_SYSTEMS) {
        sysSlot = MAX_STAR_SYSTEMS - 1; // overwrite last slot
    } else {
        g_systemCount++;
    }

    sys = &g_systems[sysSlot];
    memset(sys, 0, sizeof(StarSystem));

    pIdx = rand() % 10;
    sIdx = rand() % 8;
    spIdx = rand() % 5;
    sprintf(sys->id, "proc_%d", rand() % 9000 + 1000);
    sprintf(sys->name, "%s-%d %s", s_prefixes[pIdx], (rand() % 890) + 100, s_suffixes[sIdx]);
    strncpy(sys->spectralClass, s_spectrals[spIdx], sizeof(sys->spectralClass) - 1);
    sys->starColor = s_starCols[spIdx];
    sys->starCorona = s_coronas[spIdx];

    // Body 0: Central Star
    star = &sys->celestials[0];
    strcpy(star->id, "star");
    sprintf(star->name, "%s Helios", sys->name);
    strcpy(star->type, sys->spectralClass);
    star->orbitRadius = 0.0f;
    star->orbitSpeed = 0.0f;
    star->angle = 0.0f;
    star->radius = 42.0f + (float)(rand() % 16);
    star->color = sys->starColor;
    star->isStar = 1;
    star->parentIndex = -1;

    numPlanets = (rand() % 3) + 3; // 3 to 5 planets
    bCount = 1;
    orbitDist = 180.0f;
    targetPlanet = 1;

    for (p = 0; p < numPlanets && bCount < MAX_SYSTEM_BODIES; p++) {
        pl = &sys->celestials[bCount];
        cType = rand() % 5; // one of 5 canonical exoplanet classes
        cInfo = &g_exoplanetClasses[cType];

        sprintf(pl->id, "p_%d", p);
        sprintf(pl->name, "%s %s", sys->name, greekLetters[p]);
        sprintf(pl->type, "%s (%s)", cInfo->name, cInfo->code);
        pl->pClass = (PlanetClass)cType;
        pl->orbitRadius = orbitDist;
        pl->orbitSpeed = (0.05f + (float)(rand() % 50) / 1000.0f) * ((p % 2 == 0) ? 1.0f : 0.9f);
        pl->angle = ((float)rand() / (float)RAND_MAX) * 6.28f;
        pl->radius = 24.0f + (float)(rand() % 18);
        pl->color = cInfo->color;
        pl->isPlanet = 1;
        pl->parentIndex = 0;

        pl->pressure = cInfo->defaultPressure + ((float)(rand() % 30) - 15.0f) * 0.01f;
        if (pl->pressure < 0.01f) pl->pressure = 0.01f;
        pl->temp = cInfo->defaultTemp + ((float)(rand() % 40) - 20.0f);
        pl->water = cInfo->defaultWater + ((float)(rand() % 20) - 10.0f);
        if (pl->water < 0.0f) pl->water = 0.0f;
        if (pl->water > 100.0f) pl->water = 100.0f;
        pl->oxygen = cInfo->defaultOxygen + ((float)(rand() % 30) - 15.0f) * 0.1f;
        if (pl->oxygen < 0.0f) pl->oxygen = 0.0f;
        pl->nitrogen = cInfo->defaultNitrogen + ((float)(rand() % 20) - 10.0f) * 0.1f;
        if (pl->nitrogen < 0.0f) pl->nitrogen = 0.0f;
        if (pl->nitrogen > 100.0f) pl->nitrogen = 100.0f;
        pl->greenhouse = cInfo->defaultGreenhouse + ((float)(rand() % 30) - 15.0f);
        if (pl->greenhouse < 10.0f) pl->greenhouse = 10.0f;
        pl->magnet = cInfo->defaultMagnet + ((float)(rand() % 20) - 10.0f) * 0.01f;
        if (pl->magnet < 0.02f) pl->magnet = 0.02f;

        oldP = sim.pressure; oldT = sim.temp; oldW = sim.water; oldO = sim.oxygen; oldN = sim.nitrogen; oldG = sim.greenhouse; oldM = sim.magnet;
        sim.pressure = pl->pressure; sim.temp = pl->temp; sim.water = pl->water; sim.oxygen = pl->oxygen; sim.nitrogen = pl->nitrogen; sim.greenhouse = pl->greenhouse; sim.magnet = pl->magnet;
        CalculateHabitability();
        pl->habitability = sim.habitability;
        sim.pressure = oldP; sim.temp = oldT; sim.water = oldW; sim.oxygen = oldO; sim.nitrogen = oldN; sim.greenhouse = oldG; sim.magnet = oldM;

        if (cType == CLASS_PRIMORDIAL_GAIA || cType == CLASS_OCEAN_WORLD || targetPlanet == 1) {
            targetPlanet = bCount;
        }

        bCount++;
        orbitDist += 130.0f + (float)(rand() % 60);
    }

    // Add 1 Moon orbiting target planet
    if (bCount < MAX_SYSTEM_BODIES) {
        mn = &sys->celestials[bCount];
        sprintf(mn->id, "moon_0");
        sprintf(mn->name, "%s-I", sys->celestials[targetPlanet].name);
        strcpy(mn->type, "Silicate Moon");
        mn->orbitRadius = 78.0f;
        mn->orbitSpeed = 0.28f;
        mn->angle = 1.0f;
        mn->radius = 9.0f;
        mn->color = RGB(210, 220, 235);
        mn->isMoon = 1;
        mn->parentIndex = targetPlanet;
        bCount++;
    }

    // Add 1 Orbital Station orbiting target planet
    if (bCount < MAX_SYSTEM_BODIES) {
        st = &sys->celestials[bCount];
        sprintf(st->id, "st_0");
        sprintf(st->name, "Outpost %s", s_suffixes[sIdx]);
        strcpy(st->type, "Orbital Waystation");
        st->orbitRadius = 58.0f;
        st->orbitSpeed = -0.22f;
        st->angle = 2.8f;
        st->radius = 7.0f;
        st->color = COLOR_EMERALD;
        st->isStation = 1;
        st->parentIndex = targetPlanet;
        bCount++;
    }

    sys->bodyCount = bCount;
    sys->activePlanetIndex = targetPlanet;

    LoadStarSystem(sysSlot);
    sprintf(log, "Procedural Scan: %s discovered (%d Exoplanets classified).", sys->name, numPlanets);
    SetLogMsg(log, 0);
    PlaySoundFx(SFX_SUCCESS);
}

// Phase 9: Crisis & Hazards Buttons
#define BID_CRISIS_QUICK    160
#define BID_MIT_FLARE       161
#define BID_MIT_ASTEROID    162
#define BID_MIT_QUAKE       163
#define BID_MIT_BLIGHT      164
#define BID_MIT_STORM       165
#define BID_CRISIS_REPAIR   166
#define BID_UPG_RADAR       167
#define BID_UPG_SHIELD      168
#define BID_UPG_DRONES      169
#define BID_DRILL_FLARE     170
#define BID_DRILL_ASTEROID  171
#define BID_DRILL_QUAKE     172
#define BID_DRILL_BLIGHT    173
#define BID_DRILL_STORM     174

// --- Phase 9: Cosmic Crisis & Planetary Hazards Logic ---
static void TriggerCrisis(int type) {
    char msg[128];
    const char* names[] = {"None", "Coronal Mass Ejection", "Chondrite Asteroid Impact", "Deep Crustal Tectonic Quake", "Virulent Xeno-Blight", "Ionospheric Magnetic Tempest"};
    if (type <= CRISIS_NONE || type > CRISIS_STORM) return;
    sim.crisisActive = 1;
    sim.crisisType = type;
    sim.crisisSeverity = 2 + (rand() % 3);
    sim.crisisMitigated = 0;
    sim.crisisMaxTime = 25.0f + (float)(sim.earlyWarningRadar * 8);
    if (type == CRISIS_ASTEROID) sim.crisisMaxTime += 8.0f;
    sim.crisisTimer = sim.crisisMaxTime;
    sim.crisisShake = 6.0f;
    PlaySoundFx(SFX_ALARM);

    sprintf(msg, "CRISIS ALERT: Class-%d %s detected on collision course!", sim.crisisSeverity, names[type]);
    SetLogMsg(msg, 1);
}

static void MitigateCrisis(int type) {
    int target = (type > CRISIS_NONE) ? type : sim.crisisType;
    char msg[128];
    if (!sim.crisisActive || target <= CRISIS_NONE) return;

    if (target == CRISIS_FLARE) {
        if (sim.energy >= 350 && sim.volatiles >= 150) {
            sim.energy -= 350;
            sim.volatiles -= 150;
            sim.crisisMitigated = 1;
            sim.crisisActive = 0;
            sim.crisisShake = 0.0f;
            sprintf(msg, "Solar Flare deflected! Superconducting magnetic dipole deflected coronal blast.");
            SetLogMsg(msg, 0);
            PlaySoundFx(SFX_SUCCESS);
        } else {
            SetLogMsg("Insufficient resources for Magnetic Surge (Req: 350 kW Energy, 150 t Volatiles).", 1);
        }
    } else if (target == CRISIS_ASTEROID) {
        if (sim.minerals >= 400 && sim.energy >= 200) {
            sim.minerals -= 400;
            sim.energy -= 200;
            sim.crisisMitigated = 1;
            sim.crisisActive = 0;
            sim.crisisShake = 0.0f;
            sprintf(msg, "Asteroid shattered! Orbital kinetic railguns neutralized impactor in high orbit.");
            SetLogMsg(msg, 0);
            PlaySoundFx(SFX_SUCCESS);
        } else {
            SetLogMsg("Insufficient resources for Kinetic Deflection (Req: 400 t Min, 200 kW Energy).", 1);
        }
    } else if (target == CRISIS_QUAKE) {
        if (sim.energy >= 300 && sim.minerals >= 150) {
            sim.energy -= 300;
            sim.minerals -= 150;
            sim.crisisMitigated = 1;
            sim.crisisActive = 0;
            sim.crisisShake = 0.0f;
            sprintf(msg, "Tectonic stress relieved! Geothermal relief shafts dispersed tectonic shockwaves.");
            SetLogMsg(msg, 0);
            PlaySoundFx(SFX_SUCCESS);
        } else {
            SetLogMsg("Insufficient resources for Geothermal Fracturing (Req: 300 kW Energy, 150 t Min).", 1);
        }
    } else if (target == CRISIS_BLIGHT) {
        if (sim.volatiles >= 250 && sim.food >= 150) {
            sim.volatiles -= 250;
            sim.food -= 150;
            sim.crisisMitigated = 1;
            sim.crisisActive = 0;
            sim.crisisShake = 0.0f;
            sprintf(msg, "Xeno-blight cured! Broad-spectrum antiviral aerosol neutralized spores.");
            SetLogMsg(msg, 0);
            PlaySoundFx(SFX_SUCCESS);
        } else {
            SetLogMsg("Insufficient resources for Bio-Antidote (Req: 250 t Vol, 150 t Food).", 1);
        }
    } else if (target == CRISIS_STORM) {
        if (sim.energy >= 250 && sim.minerals >= 100) {
            sim.energy -= 250;
            sim.minerals -= 100;
            sim.energy += 300; // Energy surge absorption bonus
            sim.crisisMitigated = 1;
            sim.crisisActive = 0;
            sim.crisisShake = 0.0f;
            sprintf(msg, "Ion storm absorbed! High-capacity ground grids routed surge (+300 kW stored).");
            SetLogMsg(msg, 0);
            PlaySoundFx(SFX_SUCCESS);
        } else {
            SetLogMsg("Insufficient resources for Grounding Surge (Req: 250 kW Energy, 100 t Min).", 1);
        }
    }
}

static void ResolveCrisisImpact(void) {
    char msg[128];
    float shieldRed;
    float droneDamp;
    if (!sim.crisisActive) return;
    sim.crisisActive = 0;
    if (sim.crisisMitigated) return;

    // Phase 11: Planetary Defense Citadel Interception
    if (sim.crisisType == CRISIS_ASTEROID && sim.defenseStationStage > 0) {
        float interceptChance = (sim.defenseStationStage == 1) ? 0.50f : ((sim.defenseStationStage == 2) ? 0.85f : 1.0f);
        if (((float)rand() / (float)RAND_MAX) < interceptChance) {
            sim.defenseFireAnim = 1.0f;
            sim.minerals += 60;
            PlaySoundFx(SFX_SUCCESS);
            SetLogMsg("ORBITAL DEFENSE CITADEL: Defense batteries obliterated rogue asteroid in high orbit! +60 t salvage.", 0);
            return;
        }
    }

    // Phase 11: Planetary Shield Grid Absorption
    if (sim.shieldGridStage > 0 && sim.shieldHP > 0) {
        int shieldDmg = 250;
        if (sim.crisisType == CRISIS_ASTEROID) shieldDmg = 500;
        else if (sim.crisisType == CRISIS_FLARE) shieldDmg = 400;
        else if (sim.crisisType == CRISIS_STORM) shieldDmg = 300;
        else if (sim.crisisType == CRISIS_QUAKE) shieldDmg = 150;

        sim.shieldFlareAnim = 1.0f;
        PlaySoundFx(SFX_SUCCESS);
        if (sim.shieldHP >= shieldDmg) {
            sim.shieldHP -= shieldDmg;
            SetLogMsg("PLANETARY SHIELD GRID: Sub-space barrier absorbed cosmic crisis impact! Zero surface casualties.", 0);
            return;
        } else {
            sim.shieldHP = 0;
            SetLogMsg("PLANETARY SHIELD GRID: Shield collapsed under kinetic impact! Shockwave reached surface.", 1);
        }
    }

    PlaySoundFx(SFX_EXPLODE);
    sim.crisisShake = 20.0f;

    shieldRed = (sim.shieldDeflector > 0) ? (0.30f * (float)sim.shieldDeflector) : 0.0f;
    droneDamp = (sim.repairDrones > 0) ? 0.40f : 1.0f;

    if (sim.crisisType == CRISIS_FLARE) {
        int drain = (int)(850.0f * (1.0f - shieldRed));
        sim.energy = (sim.energy > drain) ? (sim.energy - drain) : 0;
        sim.magnet = (sim.magnet > 0.06f) ? (sim.magnet - 0.06f) : 0.02f;
        sim.morale = (sim.morale > 22.0f) ? (sim.morale - 12.0f) : 10.0f;
        sprintf(msg, "CRISIS IMPACT: Coronal blast struck colony! Lost %d kW, Magnetosphere stripped.", drain);
        SetLogMsg(msg, 1);
    } else if (sim.crisisType == CRISIS_ASTEROID) {
        int domeLost = 0;
        int casualties;
        if (sim.geodesicDomes > 1 && (((float)rand() / (float)RAND_MAX) < droneDamp)) {
            sim.geodesicDomes--;
            domeLost = 1;
        }
        casualties = (int)(1200.0f * (1.0f - ((float)sim.subterraneanVaults * 0.15f)));
        sim.colonists = (sim.colonists > casualties) ? (sim.colonists - casualties) : 0;
        sim.morale = (sim.morale > 32.0f) ? (sim.morale - 22.0f) : 10.0f;
        sim.temp -= 4.5f;
        sprintf(msg, "CRISIS IMPACT: Asteroid detonated! %s%d casualties reported.", domeLost ? "Dome breached! " : "", casualties);
        SetLogMsg(msg, 1);
    } else if (sim.crisisType == CRISIS_QUAKE) {
        int minLost = 400;
        sim.minerals = (sim.minerals > minLost) ? (sim.minerals - minLost) : 0;
        sim.water = (sim.water > 1.5f) ? (sim.water - 1.5f) : 0.0f;
        sim.morale = (sim.morale > 26.0f) ? (sim.morale - 16.0f) : 10.0f;
        sprintf(msg, "CRISIS IMPACT: Tectonic quake ruptured bedrock! Lost %d t Minerals, water cracked.", minLost);
        SetLogMsg(msg, 1);
    } else if (sim.crisisType == CRISIS_BLIGHT) {
        int foodLost = (sim.food > 1400) ? 1400 : sim.food;
        sim.food -= foodLost;
        sim.oxygen = (sim.oxygen > 2.5f) ? (sim.oxygen - 2.5f) : 0.0f;
        sim.morale = (sim.morale > 30.0f) ? (sim.morale - 20.0f) : 10.0f;
        CalculateHabitability();
        sprintf(msg, "CRISIS IMPACT: Xeno-blight struck crops! Lost %d t Food, atmospheric O2 depleted.", foodLost);
        SetLogMsg(msg, 1);
    } else if (sim.crisisType == CRISIS_STORM) {
        int drain = (int)(650.0f * (1.0f - shieldRed));
        sim.energy = (sim.energy > drain) ? (sim.energy - drain) : 0;
        sim.pressure = (sim.pressure > 0.15f) ? (sim.pressure - 0.05f) : 0.10f;
        sim.morale = (sim.morale > 20.0f) ? (sim.morale - 10.0f) : 10.0f;
        sprintf(msg, "CRISIS IMPACT: Ion storm fried grid! Lost %d kW Energy.", drain);
        SetLogMsg(msg, 1);
    }

    if (sim.repairDrones > 0) {
        SetLogMsg("Nanite Repair Drones deployed: surface structural integrity restored.", 0);
    }
}

static void HandleCrisisAction(int bid) {
    char buf[128];
    switch (bid) {
        case BID_CRISIS_QUICK:
            MitigateCrisis(sim.crisisType);
            break;
        case BID_MIT_FLARE:
            MitigateCrisis(CRISIS_FLARE);
            break;
        case BID_MIT_ASTEROID:
            MitigateCrisis(CRISIS_ASTEROID);
            break;
        case BID_MIT_QUAKE:
            MitigateCrisis(CRISIS_QUAKE);
            break;
        case BID_MIT_BLIGHT:
            MitigateCrisis(CRISIS_BLIGHT);
            break;
        case BID_MIT_STORM:
            MitigateCrisis(CRISIS_STORM);
            break;
        case BID_CRISIS_REPAIR: {
            if (sim.energy >= 200 && sim.minerals >= 150) {
                sim.energy -= 200;
                sim.minerals -= 150;
                sim.crisisShake = 0.0f;
                sim.morale = (sim.morale < 92.0f) ? (sim.morale + 8.0f) : 100.0f;
                SetLogMsg("Emergency Engineering Corps deployed: structural damage patched (+8% Morale).", 0);
                PlaySoundFx(SFX_SUCCESS);
            } else {
                SetLogMsg("Insufficient resources (Req: 150 t Min, 200 kW Energy).", 1);
            }
            break;
        }
        case BID_UPG_RADAR: {
            int costMin = 250 + (sim.earlyWarningRadar * 100);
            int costEng = 180 + (sim.earlyWarningRadar * 80);
            if (sim.minerals >= costMin && sim.energy >= costEng) {
                sim.minerals -= costMin;
                sim.energy -= costEng;
                sim.earlyWarningRadar++;
                sprintf(buf, "Astrometric Early Warning Radar upgraded to Tier %d (+%ds warning window).", sim.earlyWarningRadar, sim.earlyWarningRadar * 8);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                sprintf(buf, "Insufficient resources (Req: %d Min, %d Energy).", costMin, costEng);
                SetLogMsg(buf, 1);
            }
            break;
        }
        case BID_UPG_SHIELD: {
            int costMin = 350 + (sim.shieldDeflector * 150);
            int costEng = 300 + (sim.shieldDeflector * 120);
            if (sim.minerals >= costMin && sim.energy >= costEng) {
                sim.minerals -= costMin;
                sim.energy -= costEng;
                sim.shieldDeflector++;
                sprintf(buf, "Planetary Magnetic Deflector upgraded to Tier %d (-%d%% crisis impact damage).", sim.shieldDeflector, sim.shieldDeflector * 30);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                sprintf(buf, "Insufficient resources (Req: %d Min, %d Energy).", costMin, costEng);
                SetLogMsg(buf, 1);
            }
            break;
        }
        case BID_UPG_DRONES: {
            int costMin = 300 + (sim.repairDrones * 120);
            int costVol = 200 + (sim.repairDrones * 80);
            int costEng = 150 + (sim.repairDrones * 60);
            if (sim.minerals >= costMin && sim.volatiles >= costVol && sim.energy >= costEng) {
                sim.minerals -= costMin;
                sim.volatiles -= costVol;
                sim.energy -= costEng;
                sim.repairDrones++;
                sprintf(buf, "Automated Nanite Repair Drones expanded to Tier %d (automatic post-crisis reconstruction).", sim.repairDrones);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                sprintf(buf, "Insufficient resources (Req: %d Min, %d Vol, %d Energy).", costMin, costVol, costEng);
                SetLogMsg(buf, 1);
            }
            break;
        }
        case BID_DRILL_FLARE:
            TriggerCrisis(CRISIS_FLARE);
            break;
        case BID_DRILL_ASTEROID:
            TriggerCrisis(CRISIS_ASTEROID);
            break;
        case BID_DRILL_QUAKE:
            TriggerCrisis(CRISIS_QUAKE);
            break;
        case BID_DRILL_BLIGHT:
            TriggerCrisis(CRISIS_BLIGHT);
            break;
        case BID_DRILL_STORM:
            TriggerCrisis(CRISIS_STORM);
            break;
    }
}

static void SimTick(void) {
    if (sim.paused || sim.speed <= 0) return;
    int rate = sim.speed;

    // Phase 10 & 12 Breakthrough Multipliers
    float antiDriveMult = sim.techResearched[TECH_PROP_ANTIMATTER] ? 2.0f : 1.0f;
    float bioAdaptedFoodMult = sim.techResearched[TECH_BIO_ADAPTED] ? 2.0f : 1.0f;
    float xenoTerraMult = sim.precursorTech[0] ? 1.5f : 1.0f;
    float xenoFoodMult = sim.precursorTech[0] ? 1.4f : 1.0f;
    float catalystBoost = (sim.techResearched[TECH_BIO_CATALYSTS] ? 1.4f : 1.0f) * xenoTerraMult;
    float sunshadeBoost = sim.techResearched[TECH_GEO_SUNSHADE] ? 1.75f : 1.0f;
    float hydroBoost = sim.techResearched[TECH_BIO_THERMAL] ? 1.5f : 1.0f;
    int aeroYield = sim.techResearched[TECH_BIO_THERMAL] ? 110 : 80;

    // Logistics Multipliers & Automated Supply Trade Routes
    float ringTradeMult = 1.0f + (sim.orbitalRingStage == 1 ? 0.15f : (sim.orbitalRingStage == 2 ? 0.35f : (sim.orbitalRingStage == 3 ? 0.75f : 0.0f)));
    float dockMult = (1.0f + (float)(sim.orbitalDocks - 1) * 0.20f) * antiDriveMult * ringTradeMult;
    float fuelMult = (1.0f + (float)(sim.fuelDepots - 1) * 0.25f) * antiDriveMult * ringTradeMult;

    int rMin = g_tradeRoutes[0].active ? (int)(g_tradeRoutes[0].baseYield * g_tradeRoutes[0].freighters * dockMult) : 0;
    int rVol = g_tradeRoutes[1].active ? (int)(g_tradeRoutes[1].baseYield * g_tradeRoutes[1].freighters * dockMult) : 0;
    int rFood = g_tradeRoutes[2].active ? (int)(g_tradeRoutes[2].baseYield * g_tradeRoutes[2].freighters * dockMult) : 0;
    int rEnergy = g_tradeRoutes[3].active ? (int)(g_tradeRoutes[3].baseYield * g_tradeRoutes[3].freighters * fuelMult) : 0;
    int massDriverMin = sim.massDrivers * 12;
    int fuelDepotPower = sim.fuelDepots * 40;

    // Phase 8 & 11: Housing Capacity Sync (Domes, Vaults, Megacities, Orbital Ring)
    int ringHousing = (sim.orbitalRingStage == 1 ? 1000 : (sim.orbitalRingStage == 2 ? 3000 : (sim.orbitalRingStage == 3 ? 8000 : 0)));
    sim.housingCap = (sim.geodesicDomes * 15000) + (sim.subterraneanVaults * 30000) + (sim.domedMegacities * 50000) + ringHousing;

    // Phase 8: Demographic Specialization Multipliers
    float agroMult = 1.0f + ((float)(sim.pctAgronomists - 25) * 0.012f);
    int laborerMin = (int)(sim.colonists * ((float)sim.pctLaborers / 100.0f) * 0.0008f);
    float engTerraMult = 1.0f + ((float)(sim.pctEngineers - 20) * 0.01f);

    // Phase 11: Star Elevator Immigration Flow
    if (sim.starElevatorStage > 0 && sim.cryoSleepers > 0) {
        int elevMig = (int)((sim.starElevatorStage == 1 ? 5 : (sim.starElevatorStage == 2 ? 15 : 35)) * rate);
        if (elevMig > sim.cryoSleepers) elevMig = sim.cryoSleepers;
        sim.cryoSleepers -= elevMig;
        sim.colonists += elevMig;
        if (sim.colonists > sim.housingCap) sim.colonists = sim.housingCap;
    }

    // Rationing Multipliers
    float rationMult = 1.0f;
    float rationMoraleMod = 0.0f;
    float rationGrowth = 1.0f;
    if (sim.rationPolicy == 0) {
        rationMult = 0.6f;
        rationMoraleMod = -12.0f;
        rationGrowth = 0.5f;
    } else if (sim.rationPolicy == 2) {
        rationMult = 1.5f;
        rationMoraleMod = 15.0f;
        rationGrowth = 2.0f;
    }

    // Energy (Surface Solar + Domed Megacities + Orbital Ring Power + Zero-Point Tap)
    int ringPower = (sim.orbitalRingStage == 2 ? 50 : (sim.orbitalRingStage == 3 ? 140 : 0));
    int zeroPointPower = sim.precursorTech[3] ? 450 : 0;
    int energyGen = 600 + (sim.surfaceSolar * 60) + (sim.domedMegacities * 30) + ringPower + rEnergy + fuelDepotPower + zeroPointPower;
    int baseEnergyDrain = 200 + (sim.solarMirrors * 75) + (sim.atmoProcessors * 60) + (sim.nitrogenExtractors * 85) + (sim.greenhouseStations * 70) + (sim.coreDynamos * 80) + (sim.colonists / 1000) * 5;
    int engEnergySavings = (int)(baseEnergyDrain * ((float)sim.pctEngineers / 100.0f) * 0.25f);
    int energyDrain = baseEnergyDrain - engEnergySavings;
    if (energyDrain < 80) energyDrain = 80;
    sim.deltaEnergy = energyGen - energyDrain;
    sim.energy += (int)(sim.deltaEnergy * 0.05f * rate);
    if (sim.energy < 0) sim.energy = 0;

    // Minerals (Laborers + Mining Rigs + Defense Debris Salvage)
    int defSalvage = (sim.defenseStationStage == 1 ? 2 : (sim.defenseStationStage == 2 ? 4 : (sim.defenseStationStage == 3 ? 8 : 0)));
    int mineralGain = 20 + laborerMin + (strcmp(fleet[3].status, "Harvesting") == 0 || strcmp(fleet[3].status, "Mining Belt") == 0 ? 25 : 10) + rMin + massDriverMin + defSalvage;
    int mineralDrain = (sim.atmoProcessors * 3) + (sim.nitrogenExtractors * 2);
    sim.deltaMinerals = mineralGain - mineralDrain;
    sim.minerals += (int)(sim.deltaMinerals * 0.05f * rate);
    if (sim.minerals < 0) sim.minerals = 0;

    // Phase 11 & 12: Planetary Shield Grid Maintenance & Recharge
    sim.shieldMaxHP = (sim.shieldGridStage == 1 ? 400 : (sim.shieldGridStage == 2 ? 900 : (sim.shieldGridStage == 3 ? 1800 : 0)));
    if (sim.precursorTech[1]) sim.shieldMaxHP += 500;
    if (sim.shieldMode == 1) sim.shieldMaxHP = (int)(sim.shieldMaxHP * 1.5f);
    if (sim.shieldGridStage > 0) {
        if (sim.shieldMode == 0 && sim.energy > 200) {
            if (sim.shieldHP < sim.shieldMaxHP) {
                sim.shieldHP += (int)(3.0f * (float)rate);
                if (sim.shieldHP > sim.shieldMaxHP) sim.shieldHP = sim.shieldMaxHP;
                sim.energy = (sim.energy > 1) ? (sim.energy - 1) : 0;
            }
        } else if (sim.shieldMode == 1 && sim.energy > 300) {
            if (sim.shieldHP < sim.shieldMaxHP) {
                sim.shieldHP += (int)(7.0f * (float)rate);
                if (sim.shieldHP > sim.shieldMaxHP) sim.shieldHP = sim.shieldMaxHP;
                sim.energy = (sim.energy > 3) ? (sim.energy - 3) : 0;
            }
        }
    }
    if (sim.shieldFlareAnim > 0.0f) { sim.shieldFlareAnim -= 0.05f * (float)rate; if (sim.shieldFlareAnim < 0.0f) sim.shieldFlareAnim = 0.0f; }
    if (sim.defenseFireAnim > 0.0f) { sim.defenseFireAnim -= 0.08f * (float)rate; if (sim.defenseFireAnim < 0.0f) sim.defenseFireAnim = 0.0f; }

    // Volatiles
    int volGain = 14 + (strcmp(fleet[3].status, "Scooping Ring") == 0 ? 18 : 6) + rVol;
    int volDrain = (sim.solarMirrors * 2) + (sim.greenhouseStations * 2) + (sim.nitrogenExtractors * 3);
    sim.deltaVolatiles = volGain - volDrain;
    sim.volatiles += (int)(sim.deltaVolatiles * 0.05f * rate);
    if (sim.volatiles < 0) sim.volatiles = 0;

    // Food (Phase 8 & 10 Advanced Agronomy & Breakthroughs & Phase 12 Xenobiology)
    int baseFoodGain = 15 + (sim.hydroTowers * 25) + (sim.aeroponicFarms * aeroYield) + (sim.algalVats * 45) + (sim.domedMegacities * 20) + rFood;
    int foodGain = (int)(baseFoodGain * agroMult * bioAdaptedFoodMult * xenoFoodMult);
    int foodDrain = (int)((sim.colonists / 1200) * rationMult);
    sim.deltaFood = foodGain - foodDrain;
    sim.food += (int)(sim.deltaFood * 0.05f * rate);
    if (sim.food < 0) sim.food = 0;

    // Slow planetary drifts driven by active modules
    if (sim.energy > 500) {
        // 1. Troposphere Processors Drift
        if (sim.atmoMode == 0) {
            sim.pressure += (sim.atmoProcessors * 0.0004f * rate * engTerraMult * catalystBoost);
        } else {
            if (sim.pressure > 1.0f) {
                sim.pressure -= (sim.atmoProcessors * 0.0007f * rate * engTerraMult);
                if (sim.pressure < 1.0f) sim.pressure = 1.0f;
                sim.minerals += 1;
            }
        }

        // 2. Orbital Solar Mirrors Drift
        if (sim.mirrorMode == 0) {
            sim.temp += (sim.solarMirrors * 0.015f * rate * sunshadeBoost);
        } else {
            sim.temp -= (sim.solarMirrors * 0.018f * rate * sunshadeBoost);
        }

        // 3. Nitrogen Extractors Drift
        if (sim.nitrogen < 78.0f) {
            sim.nitrogen += (sim.nitrogenExtractors * 0.025f * rate * catalystBoost);
            if (sim.nitrogen > 78.0f) sim.nitrogen = 78.0f;
            sim.pressure += (sim.nitrogenExtractors * 0.00015f * rate);
        }

        // 4. Greenhouse Aerosol Seeding Drift
        if (sim.greenhouseMode == 0) {
            if (sim.greenhouse < 250.0f) sim.greenhouse += (sim.greenhouseStations * 0.04f * rate);
        } else {
            if (sim.greenhouse > 30.0f) sim.greenhouse -= (sim.greenhouseStations * 0.05f * rate);
        }
        sim.temp += (sim.greenhouse - 100.0f) * 0.0004f * rate;

        // Biosphere & Hydrosphere drifts
        if (sim.temp > -20.0f && sim.water > 10.0f) {
            sim.oxygen += (sim.bioseedStations * 0.0008f * rate * catalystBoost);
        }
        if (sim.temp > 0.0f && sim.water < 65.0f) {
            sim.water += (0.001f * rate * hydroBoost);
        }
    }

    // Phase 10: Climate Stabilization Matrix Atmospheric Lock
    if (sim.techResearched[TECH_GEO_STABILIZATION]) {
        if (sim.pressure < 1.00f) sim.pressure += (0.0004f * rate);
        else if (sim.pressure > 1.05f) sim.pressure -= (0.0004f * rate);
        if (sim.temp < 14.5f) sim.temp += (0.015f * rate);
        else if (sim.temp > 18.0f) sim.temp -= (0.015f * rate);
        if (sim.oxygen < 20.9f) sim.oxygen += (0.0015f * rate);
        if (sim.nitrogen < 78.0f) sim.nitrogen += (0.015f * rate);
    }

    // Phase 8 & 10: Multi-factor Morale Simulation
    float targetMorale = 65.0f;
    if (sim.food <= 0) targetMorale -= 35.0f;
    else if (sim.food < 300) targetMorale -= 15.0f;
    else if (sim.food > 2000) targetMorale += 10.0f;

    targetMorale += rationMoraleMod;

    float occ = (sim.housingCap > 0) ? ((float)sim.colonists / (float)sim.housingCap) : 2.0f;
    if (occ > 1.0f) targetMorale -= 30.0f;
    else if (occ > 0.85f) targetMorale -= 10.0f;
    else if (occ < 0.70f) targetMorale += 8.0f;

    targetMorale += (sim.habitability * 0.25f);
    targetMorale += (sim.domedMegacities * 6.0f);
    targetMorale += (sim.pctScientists * 0.2f);

    // Breakthrough & Megastructure Morale Buffs
    if (sim.techResearched[TECH_BIO_ADAPTED]) targetMorale += 15.0f;
    if (sim.techResearched[TECH_GEO_STABILIZATION]) targetMorale += 20.0f;
    if (sim.orbitalRingStage == 3) targetMorale += 15.0f;
    if (sim.precursorTech[0]) targetMorale += 15.0f;

    if (targetMorale < 10.0f) targetMorale = 10.0f;
    if (targetMorale > 99.0f) targetMorale = 99.0f;

    sim.morale += (targetMorale - sim.morale) * 0.05f * rate;
    if (sim.morale < 10.0f) sim.morale = 10.0f;
    if (sim.morale > 99.0f) sim.morale = 99.0f;

    // Population Growth / Attrition
    if (sim.food > 50 && sim.morale >= 50.0f && sim.colonists < sim.housingCap) {
        int growth = (int)(sim.colonists * (sim.morale - 40.0f) * 0.00004f * rate * rationGrowth);
        sim.colonists += growth;
        if (sim.colonists > sim.housingCap) sim.colonists = sim.housingCap;
    } else if (sim.food <= 0) {
        int loss = (int)(sim.colonists * 0.003f * rate) + 1;
        sim.colonists -= loss;
        if (sim.colonists < 0) sim.colonists = 0;
    }

    // Phase 10: Science Data Generation & Research Progress
    int sciBase = (int)(sim.colonists * ((float)sim.pctScientists / 100.0f) * 0.0012f);
    float sciMult = (sim.demoFocus == 3) ? 1.75f : 1.0f;
    int surveyorBonus = (strcmp(fleet[2].status, "Scanning Orbit") == 0 || strcmp(fleet[2].status, "Long-Range Scan") == 0) ? 10 : 0;
    int megacityBonus = sim.domedMegacities * 12;
    sim.deltaScience = (int)((sciBase + surveyorBonus + megacityBonus) * sciMult);
    if (sim.deltaScience < 1) sim.deltaScience = 1;
    sim.science += (int)(sim.deltaScience * 0.05f * rate);

    // Advance active tech
    if (sim.activeTech >= 0 && sim.activeTech < MAX_TECHS && !sim.techResearched[sim.activeTech]) {
        sim.techProgress[sim.activeTech] += (sim.deltaScience * 0.05f * rate);
        if (sim.techProgress[sim.activeTech] >= (float)g_techs[sim.activeTech].costScience) {
            sim.techProgress[sim.activeTech] = (float)g_techs[sim.activeTech].costScience;
            sim.techResearched[sim.activeTech] = 1;
            char lBuf[128];
            if (g_techs[sim.activeTech].isBreakthrough) {
                sim.breakthroughFanfare = 60;
                strncpy(sim.lastBreakthrough, g_techs[sim.activeTech].name, sizeof(sim.lastBreakthrough) - 1);
                sprintf(lBuf, "BREAKTHROUGH: [%s] UNLOCKED! %s", g_techs[sim.activeTech].name, g_techs[sim.activeTech].effect);
                SetLogMsg(lBuf, 0);
            } else {
                sprintf(lBuf, "RESEARCH COMPLETED: [%s] online! %s", g_techs[sim.activeTech].name, g_techs[sim.activeTech].effect);
                SetLogMsg(lBuf, 0);
            }
            CalculateHabitability();
            int nextT = -1;
            for (int k = 0; k < MAX_TECHS; k++) {
                if (!sim.techResearched[k]) {
                    if (g_techs[k].prereq < 0 || sim.techResearched[g_techs[k].prereq]) {
                        nextT = k;
                        break;
                    }
                }
            }
            sim.activeTech = nextT;
        }
    }

    // Phase 12: Alien Xenobiology & Precursor Excavations
    for (int i = 0; i < 4; i++) {
        if (sim.xenoSiteStatus[i] == 1) { // Excavating
            if (sim.energy > 30) {
                sim.xenoSiteProgress[i] += (int)(2 * rate);
                if (sim.xenoSiteProgress[i] >= 100) {
                    sim.xenoSiteProgress[i] = 100;
                    sim.xenoSiteStatus[i] = 2; // Excavated
                    sim.xenoRelicFound[i] = 1;
                    sim.science += 250;
                    const char* rNames[] = {
                        "Tachyon Crystalline Matrix",
                        "Precursor Xenobiotic Genome",
                        "Autonomous Nanite Core",
                        "Zero-Point Flux Resonator"
                    };
                    char xBuf[160];
                    sprintf(xBuf, "EXCAVATION SUCCESS! Unearthed [%s]! +250 SP.", rNames[i]);
                    SetLogMsg(xBuf, 0);
                    PlaySoundFx(SFX_SUCCESS);
                }
            }
        }
    }

    // Phase 9: Crisis Events & Hazards
    if (sim.crisisActive) {
        sim.crisisTimer -= 0.1f * rate;
        if (sim.crisisTimer <= 0.0f) {
            ResolveCrisisImpact();
        }
    } else {
        if ((rand() % 500) == 0) {
            int cType = 1 + (rand() % 5);
            TriggerCrisis(cType);
        }
    }

    CalculateHabitability();
}

static void InitSimulation(void) {
    srand(1337);
    sim.cycle = 1.00f;
    sim.speed = 1;
    sim.paused = 0;
    sim.time = 0.0f;
    sim.activeTab = 0;

    sim.energy = 14250;
    sim.minerals = 8400;
    sim.volatiles = 2150;
    sim.food = 5800;
    sim.colonists = 25000;
    sim.cryoSleepers = 75000;
    sim.morale = 86.0f;

    // Phase 10: Interstellar Research Tree & Breakthroughs
    sim.science = 850;
    sim.deltaScience = 18;
    sim.activeTech = 0; // TECH_PROP_RAMJETS
    for (int t = 0; t < MAX_TECHS; t++) {
        sim.techProgress[t] = 0.0f;
        sim.techResearched[t] = 0;
    }
    sim.techProgress[0] = 120.0f;
    sim.breakthroughFanfare = 0;
    sim.lastBreakthrough[0] = '\0';

    // Phase 12: Alien Xenobiology & Ancient Precursor Relics
    for (int i = 0; i < 4; i++) {
        sim.xenoSiteStatus[i] = 0;   // 0=Surveyed, 1=Excavating, 2=Excavated
        sim.xenoSiteProgress[i] = 0;
        sim.xenoRelicFound[i] = 0;
        sim.precursorTech[i] = 0;
    }
    sim.xenoScanAnim = 0.0f;

    // Phase 11: Orbital Megastructures & Planetary Defense Stations
    sim.orbitalRingStage = 0;
    sim.starElevatorStage = 0;
    sim.shieldGridStage = 1;
    sim.defenseStationStage = 0;
    sim.shieldHP = 400;
    sim.shieldMaxHP = 400;
    sim.shieldMode = 0;
    sim.shieldFlareAnim = 0.0f;
    sim.defenseFireAnim = 0.0f;

    // Phase 8 Demographics & Infrastructure defaults
    sim.pctLaborers = 40;
    sim.pctAgronomists = 25;
    sim.pctEngineers = 20;
    sim.pctScientists = 15;
    sim.demoFocus = 0;
    sim.rationPolicy = 1; // Standard
    sim.geodesicDomes = 2;
    sim.subterraneanVaults = 0;
    sim.domedMegacities = 0;
    sim.hydroTowers = 2;
    sim.aeroponicFarms = 0;
    sim.algalVats = 0;
    sim.housingCap = (sim.geodesicDomes * 15000) + (sim.subterraneanVaults * 30000) + (sim.domedMegacities * 50000);

    // Phase 9 Crisis & Defense Infrastructure defaults
    sim.earlyWarningRadar = 1;
    sim.shieldDeflector = 0;
    sim.repairDrones = 0;
    sim.crisisActive = 0;
    sim.crisisType = CRISIS_NONE;
    sim.crisisTimer = 0.0f;
    sim.crisisMaxTime = 30.0f;
    sim.crisisSeverity = 1;
    sim.crisisMitigated = 0;
    sim.crisisShake = 0.0f;

    sim.deltaEnergy = 120;
    sim.deltaMinerals = 15;
    sim.deltaVolatiles = 10;
    sim.deltaFood = 25;

    sim.pressure = 0.32f;
    sim.temp = -48.0f;
    sim.water = 12.8f;
    sim.oxygen = 3.1f;
    sim.nitrogen = 14.5f;
    sim.greenhouse = 45.0f;
    sim.magnet = 0.18f;
    sim.habitability = 18.4f;

    sim.solarMirrors = 2;
    sim.mirrorMode = 0;       // 0=Heat
    sim.atmoProcessors = 3;
    sim.atmoMode = 0;         // 0=Inject
    sim.nitrogenExtractors = 1;
    sim.greenhouseStations = 1;
    sim.greenhouseMode = 0;   // 0=Warm
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

    // Initialize 4 Star Systems Catalog
    g_systemCount = 4;
    memset(g_systems, 0, sizeof(g_systems));

    // SYSTEM 0: Kepler-186
    strcpy(g_systems[0].id, "kepler186");
    strcpy(g_systems[0].name, "Kepler-186");
    strcpy(g_systems[0].spectralClass, "Red Dwarf (M-V)");
    strcpy(g_systems[0].desc, "Anchor frontier sector with habitable-zone rocky exoplanets.");
    g_systems[0].starColor = COLOR_ORANGE;
    g_systems[0].starCorona = RGB(120, 45, 10);
    g_systems[0].bodyCount = 5;
    g_systems[0].activePlanetIndex = 2; // Aethelgard Prime

    // Star
    strcpy(g_systems[0].celestials[0].id, "k_sun");
    strcpy(g_systems[0].celestials[0].name, "Kepler-186 Helios");
    strcpy(g_systems[0].celestials[0].type, "Red Dwarf Star");
    g_systems[0].celestials[0].radius = 48.0f;
    g_systems[0].celestials[0].color = COLOR_ORANGE;
    g_systems[0].celestials[0].isStar = 1;
    g_systems[0].celestials[0].parentIndex = -1;

    // Kepler-186b (Barren Rock)
    strcpy(g_systems[0].celestials[1].id, "k_b");
    strcpy(g_systems[0].celestials[1].name, "Kepler-186b");
    strcpy(g_systems[0].celestials[1].type, "Barren Rock (BR-I)");
    g_systems[0].celestials[1].pClass = CLASS_BARREN_ROCK;
    g_systems[0].celestials[1].orbitRadius = 210.0f;
    g_systems[0].celestials[1].orbitSpeed = 0.16f;
    g_systems[0].celestials[1].angle = 1.8f;
    g_systems[0].celestials[1].radius = 26.0f;
    g_systems[0].celestials[1].color = RGB(180, 110, 70);
    g_systems[0].celestials[1].isPlanet = 1;
    g_systems[0].celestials[1].parentIndex = 0;
    g_systems[0].celestials[1].pressure = 0.04f;
    g_systems[0].celestials[1].temp = 145.0f;
    g_systems[0].celestials[1].water = 0.0f;
    g_systems[0].celestials[1].oxygen = 0.0f;
    g_systems[0].celestials[1].magnet = 0.08f;
    g_systems[0].celestials[1].habitability = 2.4f;

    // Aethelgard Prime (Barren Rock / Terraforming Target)
    strcpy(g_systems[0].celestials[2].id, "aethelgard");
    strcpy(g_systems[0].celestials[2].name, "Aethelgard Prime");
    strcpy(g_systems[0].celestials[2].type, "Barren Rock (BR-I)");
    g_systems[0].celestials[2].pClass = CLASS_BARREN_ROCK;
    g_systems[0].celestials[2].orbitRadius = 350.0f;
    g_systems[0].celestials[2].orbitSpeed = 0.08f;
    g_systems[0].celestials[2].angle = 0.4f;
    g_systems[0].celestials[2].radius = 36.0f;
    g_systems[0].celestials[2].color = COLOR_BLUE;
    g_systems[0].celestials[2].isPlanet = 1;
    g_systems[0].celestials[2].parentIndex = 0;
    g_systems[0].celestials[2].pressure = sim.pressure;
    g_systems[0].celestials[2].temp = sim.temp;
    g_systems[0].celestials[2].water = sim.water;
    g_systems[0].celestials[2].oxygen = sim.oxygen;
    g_systems[0].celestials[2].magnet = sim.magnet;
    g_systems[0].celestials[2].habitability = sim.habitability;

    // Boreas Minor Moon (Orbiting Aethelgard)
    strcpy(g_systems[0].celestials[3].id, "boreas");
    strcpy(g_systems[0].celestials[3].name, "Boreas Minor");
    strcpy(g_systems[0].celestials[3].type, "Frozen Ice Moon");
    g_systems[0].celestials[3].orbitRadius = 88.0f;
    g_systems[0].celestials[3].orbitSpeed = 0.25f;
    g_systems[0].celestials[3].angle = 1.2f;
    g_systems[0].celestials[3].radius = 11.0f;
    g_systems[0].celestials[3].color = RGB(224, 242, 254);
    g_systems[0].celestials[3].isMoon = 1;
    g_systems[0].celestials[3].parentIndex = 2;

    // Zephyr Station (Orbiting Aethelgard)
    strcpy(g_systems[0].celestials[4].id, "zephyr");
    strcpy(g_systems[0].celestials[4].name, "Zephyr Station");
    strcpy(g_systems[0].celestials[4].type, "Orbital Shipyard");
    g_systems[0].celestials[4].orbitRadius = 64.0f;
    g_systems[0].celestials[4].orbitSpeed = -0.18f;
    g_systems[0].celestials[4].angle = 3.0f;
    g_systems[0].celestials[4].radius = 7.0f;
    g_systems[0].celestials[4].color = COLOR_EMERALD;
    g_systems[0].celestials[4].isStation = 1;
    g_systems[0].celestials[4].parentIndex = 2;

    // SYSTEM 1: TRAPPIST-1
    strcpy(g_systems[1].id, "trappist1");
    strcpy(g_systems[1].name, "TRAPPIST-1");
    strcpy(g_systems[1].spectralClass, "Ultra-Cool Dwarf (M-VIII)");
    strcpy(g_systems[1].desc, "Compact resonant system with 3 habitable-zone worlds.");
    g_systems[1].starColor = RGB(255, 90, 30);
    g_systems[1].starCorona = RGB(100, 30, 10);
    g_systems[1].bodyCount = 5;
    g_systems[1].activePlanetIndex = 2; // Trappist-1d Primordial Gaia

    strcpy(g_systems[1].celestials[0].id, "t_sun");
    strcpy(g_systems[1].celestials[0].name, "Trappist Helios");
    strcpy(g_systems[1].celestials[0].type, "Ultra-Cool Red Dwarf");
    g_systems[1].celestials[0].radius = 42.0f;
    g_systems[1].celestials[0].color = RGB(255, 90, 30);
    g_systems[1].celestials[0].isStar = 1;
    g_systems[1].celestials[0].parentIndex = -1;

    strcpy(g_systems[1].celestials[1].id, "t_b");
    strcpy(g_systems[1].celestials[1].name, "Trappist-1b");
    strcpy(g_systems[1].celestials[1].type, "Toxic Greenhouse (TG-II)");
    g_systems[1].celestials[1].pClass = CLASS_TOXIC_GREENHOUSE;
    g_systems[1].celestials[1].orbitRadius = 200.0f;
    g_systems[1].celestials[1].orbitSpeed = 0.14f;
    g_systems[1].celestials[1].angle = 0.8f;
    g_systems[1].celestials[1].radius = 28.0f;
    g_systems[1].celestials[1].color = RGB(245, 158, 11);
    g_systems[1].celestials[1].isPlanet = 1;
    g_systems[1].celestials[1].parentIndex = 0;
    g_systems[1].celestials[1].pressure = 3.65f;
    g_systems[1].celestials[1].temp = 195.0f;
    g_systems[1].celestials[1].water = 1.5f;
    g_systems[1].celestials[1].oxygen = 0.0f;
    g_systems[1].celestials[1].magnet = 0.12f;
    g_systems[1].celestials[1].habitability = 1.8f;

    strcpy(g_systems[1].celestials[2].id, "t_d");
    strcpy(g_systems[1].celestials[2].name, "Trappist-1d");
    strcpy(g_systems[1].celestials[2].type, "Primordial Gaia (PG-V)");
    g_systems[1].celestials[2].pClass = CLASS_PRIMORDIAL_GAIA;
    g_systems[1].celestials[2].orbitRadius = 310.0f;
    g_systems[1].celestials[2].orbitSpeed = 0.09f;
    g_systems[1].celestials[2].angle = 2.4f;
    g_systems[1].celestials[2].radius = 34.0f;
    g_systems[1].celestials[2].color = RGB(16, 185, 129);
    g_systems[1].celestials[2].isPlanet = 1;
    g_systems[1].celestials[2].parentIndex = 0;
    g_systems[1].celestials[2].pressure = 1.05f;
    g_systems[1].celestials[2].temp = 16.5f;
    g_systems[1].celestials[2].water = 58.0f;
    g_systems[1].celestials[2].oxygen = 19.2f;
    g_systems[1].celestials[2].magnet = 0.54f;
    g_systems[1].celestials[2].habitability = 89.2f;

    strcpy(g_systems[1].celestials[3].id, "t_e");
    strcpy(g_systems[1].celestials[3].name, "Trappist-1e");
    strcpy(g_systems[1].celestials[3].type, "Ocean World (OW-IV)");
    g_systems[1].celestials[3].pClass = CLASS_OCEAN_WORLD;
    g_systems[1].celestials[3].orbitRadius = 430.0f;
    g_systems[1].celestials[3].orbitSpeed = 0.06f;
    g_systems[1].celestials[3].angle = 4.1f;
    g_systems[1].celestials[3].radius = 32.0f;
    g_systems[1].celestials[3].color = RGB(14, 165, 233);
    g_systems[1].celestials[3].isPlanet = 1;
    g_systems[1].celestials[3].parentIndex = 0;
    g_systems[1].celestials[3].pressure = 1.20f;
    g_systems[1].celestials[3].temp = 12.0f;
    g_systems[1].celestials[3].water = 96.0f;
    g_systems[1].celestials[3].oxygen = 10.5f;
    g_systems[1].celestials[3].magnet = 0.46f;
    g_systems[1].celestials[3].habitability = 68.4f;

    strcpy(g_systems[1].celestials[4].id, "t_nodus");
    strcpy(g_systems[1].celestials[4].name, "Nodus Station");
    strcpy(g_systems[1].celestials[4].type, "Orbital Haven");
    g_systems[1].celestials[4].orbitRadius = 60.0f;
    g_systems[1].celestials[4].orbitSpeed = 0.22f;
    g_systems[1].celestials[4].angle = 0.5f;
    g_systems[1].celestials[4].radius = 7.0f;
    g_systems[1].celestials[4].color = COLOR_EMERALD;
    g_systems[1].celestials[4].isStation = 1;
    g_systems[1].celestials[4].parentIndex = 2;

    // SYSTEM 2: Gliese-667C
    strcpy(g_systems[2].id, "gliese667c");
    strcpy(g_systems[2].name, "Gliese-667C");
    strcpy(g_systems[2].spectralClass, "Red Dwarf Star");
    strcpy(g_systems[2].desc, "Trinary system companion with deep abyssal ocean worlds.");
    g_systems[2].starColor = RGB(255, 120, 50);
    g_systems[2].starCorona = RGB(110, 40, 15);
    g_systems[2].bodyCount = 5;
    g_systems[2].activePlanetIndex = 2; // Gliese-667Cc Ocean World

    strcpy(g_systems[2].celestials[0].id, "g_sun");
    strcpy(g_systems[2].celestials[0].name, "Gliese Prime");
    strcpy(g_systems[2].celestials[0].type, "Red Dwarf Star");
    g_systems[2].celestials[0].radius = 44.0f;
    g_systems[2].celestials[0].color = RGB(255, 120, 50);
    g_systems[2].celestials[0].isStar = 1;
    g_systems[2].celestials[0].parentIndex = -1;

    strcpy(g_systems[2].celestials[1].id, "g_b");
    strcpy(g_systems[2].celestials[1].name, "Gliese-667Cb");
    strcpy(g_systems[2].celestials[1].type, "Toxic Greenhouse (TG-II)");
    g_systems[2].celestials[1].pClass = CLASS_TOXIC_GREENHOUSE;
    g_systems[2].celestials[1].orbitRadius = 200.0f;
    g_systems[2].celestials[1].orbitSpeed = 0.15f;
    g_systems[2].celestials[1].angle = 3.2f;
    g_systems[2].celestials[1].radius = 30.0f;
    g_systems[2].celestials[1].color = RGB(245, 158, 11);
    g_systems[2].celestials[1].isPlanet = 1;
    g_systems[2].celestials[1].parentIndex = 0;
    g_systems[2].celestials[1].pressure = 2.90f;
    g_systems[2].celestials[1].temp = 160.0f;
    g_systems[2].celestials[1].water = 4.0f;
    g_systems[2].celestials[1].oxygen = 0.2f;
    g_systems[2].celestials[1].magnet = 0.15f;
    g_systems[2].celestials[1].habitability = 4.2f;

    strcpy(g_systems[2].celestials[2].id, "g_c");
    strcpy(g_systems[2].celestials[2].name, "Gliese-667Cc");
    strcpy(g_systems[2].celestials[2].type, "Ocean World (OW-IV)");
    g_systems[2].celestials[2].pClass = CLASS_OCEAN_WORLD;
    g_systems[2].celestials[2].orbitRadius = 320.0f;
    g_systems[2].celestials[2].orbitSpeed = 0.08f;
    g_systems[2].celestials[2].angle = 1.1f;
    g_systems[2].celestials[2].radius = 35.0f;
    g_systems[2].celestials[2].color = RGB(14, 165, 233);
    g_systems[2].celestials[2].isPlanet = 1;
    g_systems[2].celestials[2].parentIndex = 0;
    g_systems[2].celestials[2].pressure = 1.12f;
    g_systems[2].celestials[2].temp = 22.0f;
    g_systems[2].celestials[2].water = 99.0f;
    g_systems[2].celestials[2].oxygen = 12.8f;
    g_systems[2].celestials[2].magnet = 0.50f;
    g_systems[2].celestials[2].habitability = 74.5f;

    strcpy(g_systems[2].celestials[3].id, "g_e");
    strcpy(g_systems[2].celestials[3].name, "Gliese-667Ce");
    strcpy(g_systems[2].celestials[3].type, "Frozen Tundra (FT-III)");
    g_systems[2].celestials[3].pClass = CLASS_FROZEN_TUNDRA;
    g_systems[2].celestials[3].orbitRadius = 440.0f;
    g_systems[2].celestials[3].orbitSpeed = 0.05f;
    g_systems[2].celestials[3].angle = 5.2f;
    g_systems[2].celestials[3].radius = 31.0f;
    g_systems[2].celestials[3].color = RGB(56, 189, 248);
    g_systems[2].celestials[3].isPlanet = 1;
    g_systems[2].celestials[3].parentIndex = 0;
    g_systems[2].celestials[3].pressure = 0.40f;
    g_systems[2].celestials[3].temp = -78.0f;
    g_systems[2].celestials[3].water = 75.0f;
    g_systems[2].celestials[3].oxygen = 3.5f;
    g_systems[2].celestials[3].magnet = 0.30f;
    g_systems[2].celestials[3].habitability = 22.1f;

    strcpy(g_systems[2].celestials[4].id, "g_mn");
    strcpy(g_systems[2].celestials[4].name, "Astraea Moon");
    strcpy(g_systems[2].celestials[4].type, "Silicate Moon");
    g_systems[2].celestials[4].orbitRadius = 76.0f;
    g_systems[2].celestials[4].orbitSpeed = 0.26f;
    g_systems[2].celestials[4].angle = 2.0f;
    g_systems[2].celestials[4].radius = 10.0f;
    g_systems[2].celestials[4].color = RGB(215, 225, 240);
    g_systems[2].celestials[4].isMoon = 1;
    g_systems[2].celestials[4].parentIndex = 2;

    // SYSTEM 3: Tau Ceti
    strcpy(g_systems[3].id, "tauceti");
    strcpy(g_systems[3].name, "Tau Ceti");
    strcpy(g_systems[3].spectralClass, "Yellow Dwarf (G-VIII)");
    strcpy(g_systems[3].desc, "Sol-like system with massive ice sheets and cryo-reservoirs.");
    g_systems[3].starColor = RGB(255, 230, 110);
    g_systems[3].starCorona = RGB(120, 90, 25);
    g_systems[3].bodyCount = 5;
    g_systems[3].activePlanetIndex = 2; // Tau Ceti-f Frozen Tundra

    strcpy(g_systems[3].celestials[0].id, "tc_sun");
    strcpy(g_systems[3].celestials[0].name, "Tau Ceti Helios");
    strcpy(g_systems[3].celestials[0].type, "Yellow Dwarf Star");
    g_systems[3].celestials[0].radius = 50.0f;
    g_systems[3].celestials[0].color = RGB(255, 230, 110);
    g_systems[3].celestials[0].isStar = 1;
    g_systems[3].celestials[0].parentIndex = -1;

    strcpy(g_systems[3].celestials[1].id, "tc_e");
    strcpy(g_systems[3].celestials[1].name, "Tau Ceti-e");
    strcpy(g_systems[3].celestials[1].type, "Toxic Greenhouse (TG-II)");
    g_systems[3].celestials[1].pClass = CLASS_TOXIC_GREENHOUSE;
    g_systems[3].celestials[1].orbitRadius = 220.0f;
    g_systems[3].celestials[1].orbitSpeed = 0.13f;
    g_systems[3].celestials[1].angle = 0.3f;
    g_systems[3].celestials[1].radius = 32.0f;
    g_systems[3].celestials[1].color = RGB(245, 158, 11);
    g_systems[3].celestials[1].isPlanet = 1;
    g_systems[3].celestials[1].parentIndex = 0;
    g_systems[3].celestials[1].pressure = 3.10f;
    g_systems[3].celestials[1].temp = 175.0f;
    g_systems[3].celestials[1].water = 3.0f;
    g_systems[3].celestials[1].oxygen = 0.1f;
    g_systems[3].celestials[1].magnet = 0.14f;
    g_systems[3].celestials[1].habitability = 3.5f;

    strcpy(g_systems[3].celestials[2].id, "tc_f");
    strcpy(g_systems[3].celestials[2].name, "Tau Ceti-f");
    strcpy(g_systems[3].celestials[2].type, "Frozen Tundra (FT-III)");
    g_systems[3].celestials[2].pClass = CLASS_FROZEN_TUNDRA;
    g_systems[3].celestials[2].orbitRadius = 360.0f;
    g_systems[3].celestials[2].orbitSpeed = 0.07f;
    g_systems[3].celestials[2].angle = 2.7f;
    g_systems[3].celestials[2].radius = 36.0f;
    g_systems[3].celestials[2].color = RGB(56, 189, 248);
    g_systems[3].celestials[2].isPlanet = 1;
    g_systems[3].celestials[2].parentIndex = 0;
    g_systems[3].celestials[2].pressure = 0.52f;
    g_systems[3].celestials[2].temp = -84.0f;
    g_systems[3].celestials[2].water = 72.0f;
    g_systems[3].celestials[2].oxygen = 2.8f;
    g_systems[3].celestials[2].magnet = 0.38f;
    g_systems[3].celestials[2].habitability = 28.5f;

    strcpy(g_systems[3].celestials[3].id, "tc_g");
    strcpy(g_systems[3].celestials[3].name, "Tau Ceti-g");
    strcpy(g_systems[3].celestials[3].type, "Barren Rock (BR-I)");
    g_systems[3].celestials[3].pClass = CLASS_BARREN_ROCK;
    g_systems[3].celestials[3].orbitRadius = 480.0f;
    g_systems[3].celestials[3].orbitSpeed = 0.05f;
    g_systems[3].celestials[3].angle = 4.8f;
    g_systems[3].celestials[3].radius = 26.0f;
    g_systems[3].celestials[3].color = RGB(180, 110, 70);
    g_systems[3].celestials[3].isPlanet = 1;
    g_systems[3].celestials[3].parentIndex = 0;
    g_systems[3].celestials[3].pressure = 0.03f;
    g_systems[3].celestials[3].temp = -120.0f;
    g_systems[3].celestials[3].water = 0.0f;
    g_systems[3].celestials[3].oxygen = 0.0f;
    g_systems[3].celestials[3].magnet = 0.04f;
    g_systems[3].celestials[3].habitability = 0.8f;

    strcpy(g_systems[3].celestials[4].id, "tc_depot");
    strcpy(g_systems[3].celestials[4].name, "Horizon Depot");
    strcpy(g_systems[3].celestials[4].type, "Orbital Waystation");
    g_systems[3].celestials[4].orbitRadius = 64.0f;
    g_systems[3].celestials[4].orbitSpeed = -0.20f;
    g_systems[3].celestials[4].angle = 1.6f;
    g_systems[3].celestials[4].radius = 7.0f;
    g_systems[3].celestials[4].color = COLOR_EMERALD;
    g_systems[3].celestials[4].isStation = 1;
    g_systems[3].celestials[4].parentIndex = 2;

    // Populate default nitrogen and greenhouse on all catalog planets
    for (int s = 0; s < g_systemCount; s++) {
        for (int b = 0; b < g_systems[s].bodyCount; b++) {
            if (g_systems[s].celestials[b].isPlanet) {
                PlanetClass pc = g_systems[s].celestials[b].pClass;
                g_systems[s].celestials[b].nitrogen = g_exoplanetClasses[pc].defaultNitrogen;
                g_systems[s].celestials[b].greenhouse = g_exoplanetClasses[pc].defaultGreenhouse;
            }
        }
    }
    // Set target planet values to match sim
    g_systems[0].celestials[2].nitrogen = sim.nitrogen;
    g_systems[0].celestials[2].greenhouse = sim.greenhouse;

    // Load Default System 0 (Kepler-186)
    g_currentSystem = 0;

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

    // Logistics Infrastructure
    sim.orbitalDocks = 1;
    sim.fuelDepots = 1;
    sim.massDrivers = 1;

    // Automated Supply Trade Routes
    strcpy(g_tradeRoutes[0].id, "minerals");
    strcpy(g_tradeRoutes[0].name, "Tartarus Mineral Corridor");
    strcpy(g_tradeRoutes[0].from, "Tartarus Belt");
    strcpy(g_tradeRoutes[0].to, "Surface Depot");
    strcpy(g_tradeRoutes[0].cargo, "Minerals");
    g_tradeRoutes[0].active = 1;
    g_tradeRoutes[0].freighters = 2;
    g_tradeRoutes[0].baseYield = 28;
    g_tradeRoutes[0].costMin = 120;
    g_tradeRoutes[0].costEnergy = 80;
    g_tradeRoutes[0].progress = 0.15f;
    g_tradeRoutes[0].speed = 0.007f;
    g_tradeRoutes[0].color = COLOR_AMBER;

    strcpy(g_tradeRoutes[1].id, "volatiles");
    strcpy(g_tradeRoutes[1].name, "Boreas Cryo-Volatiles");
    strcpy(g_tradeRoutes[1].from, "Boreas Ice Shell");
    strcpy(g_tradeRoutes[1].to, "Zephyr Docks");
    strcpy(g_tradeRoutes[1].cargo, "Volatiles");
    g_tradeRoutes[1].active = 1;
    g_tradeRoutes[1].freighters = 1;
    g_tradeRoutes[1].baseYield = 20;
    g_tradeRoutes[1].costMin = 100;
    g_tradeRoutes[1].costEnergy = 90;
    g_tradeRoutes[1].progress = 0.45f;
    g_tradeRoutes[1].speed = 0.006f;
    g_tradeRoutes[1].color = COLOR_BLUE;

    strcpy(g_tradeRoutes[2].id, "food");
    strcpy(g_tradeRoutes[2].name, "Agro-Dome Sustenance");
    strcpy(g_tradeRoutes[2].from, "Ground Hydroponics");
    strcpy(g_tradeRoutes[2].to, "Ark Flotilla");
    strcpy(g_tradeRoutes[2].cargo, "Food");
    g_tradeRoutes[2].active = 1;
    g_tradeRoutes[2].freighters = 1;
    g_tradeRoutes[2].baseYield = 22;
    g_tradeRoutes[2].costMin = 110;
    g_tradeRoutes[2].costEnergy = 70;
    g_tradeRoutes[2].progress = 0.75f;
    g_tradeRoutes[2].speed = 0.008f;
    g_tradeRoutes[2].color = COLOR_EMERALD;

    strcpy(g_tradeRoutes[3].id, "fuel");
    strcpy(g_tradeRoutes[3].name, "Helios Plasma Skimmer");
    strcpy(g_tradeRoutes[3].from, "Helios Corona");
    strcpy(g_tradeRoutes[3].to, "He-3 Fuel Depot");
    strcpy(g_tradeRoutes[3].cargo, "He-3 Fuel");
    g_tradeRoutes[3].active = 1;
    g_tradeRoutes[3].freighters = 1;
    g_tradeRoutes[3].baseYield = 150;
    g_tradeRoutes[3].costMin = 150;
    g_tradeRoutes[3].costEnergy = 120;
    g_tradeRoutes[3].progress = 0.30f;
    g_tradeRoutes[3].speed = 0.005f;
    g_tradeRoutes[3].color = COLOR_ORANGE;

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

// --- Procedural Exoplanet Visual Shaders (GDI) ---

static void DrawBarrenRockGDI(HDC hdc, CelestialBody* b, int px, int py, int pr, int sunX, int sunY, float z, float rot) {
    // 1. Basaltic Regolith Crust
    FillSolidRect(hdc, px - pr, py - pr, pr * 2, pr * 2, RGB(88, 62, 50));

    // 2. Dark Basaltic Maria Plains
    HBRUSH hMareBrush = CreateSolidBrush(RGB(55, 38, 30));
    HPEN hMarePen = CreatePen(PS_SOLID, 1, RGB(55, 38, 30));
    HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hMareBrush);
    HPEN hOldP = (HPEN)SelectObject(hdc, hMarePen);

    for (int i = 0; i < 3; i++) {
        float ang = rot * 0.4f + i * 2.1f;
        int mx = px + (int)(sinf(ang) * pr * 0.55f);
        int my = py + (int)(((i % 2 == 0) ? -0.25f : 0.25f) * pr);
        int mw = (int)(pr * 0.35f * fabsf(cosf(ang)) + 4);
        int mh = (int)(pr * 0.25f);
        Ellipse(hdc, mx - mw, my - mh, mx + mw, my + mh);
    }

    // 3. Impact Craters with 3D Lit Rims
    HPEN hRimPen = CreatePen(PS_SOLID, 1, RGB(180, 145, 120));
    HBRUSH hBowlBrush = CreateSolidBrush(RGB(35, 24, 18));
    SelectObject(hdc, hRimPen);
    SelectObject(hdc, hBowlBrush);

    for (int c = 0; c < 5; c++) {
        float cAng = rot * 0.5f + c * 1.35f;
        int cx = px + (int)(sinf(cAng) * pr * 0.65f);
        int cy = py + (int)(((c * 3) % 7 - 3) * 0.22f * pr);
        int cr = (int)(pr * (0.12f + (c % 3) * 0.04f));
        if (cr < 3) cr = 3;
        Ellipse(hdc, cx - cr, cy - cr, cx + cr, cy + cr);
    }

    // 4. Equatorial Fracture Canyon / Rift
    HPEN hRiftPen = CreatePen(PS_SOLID, 1, RGB(38, 26, 20));
    SelectObject(hdc, hRiftPen);
    int ry = py + (int)(pr * 0.1f);
    MoveToEx(hdc, px - (int)(pr * 0.7f), ry, NULL);
    LineTo(hdc, px - (int)(pr * 0.2f), ry + (int)(pr * 0.08f));
    LineTo(hdc, px + (int)(pr * 0.3f), ry - (int)(pr * 0.05f));
    LineTo(hdc, px + (int)(pr * 0.75f), ry + (int)(pr * 0.04f));

    SelectObject(hdc, hOldB);
    SelectObject(hdc, hOldP);
    DeleteObject(hMareBrush);
    DeleteObject(hMarePen);
    DeleteObject(hRimPen);
    DeleteObject(hBowlBrush);
    DeleteObject(hRiftPen);
}

static void DrawToxicGreenhouseGDI(HDC hdc, CelestialBody* b, int px, int py, int pr, int sunX, int sunY, float z, float rot) {
    // 1. Superheated Scorched Mantle
    FillSolidRect(hdc, px - pr, py - pr, pr * 2, pr * 2, RGB(160, 70, 15));

    // 2. Glowing Magma Veins & Thermal Fissures
    HPEN hMagmaPen = CreatePen(PS_SOLID, 2, RGB(255, 95, 20));
    HPEN hOldP = (HPEN)SelectObject(hdc, hMagmaPen);
    for (int v = 0; v < 3; v++) {
        int vy = py + (int)(((v - 1) * 0.45f) * pr);
        MoveToEx(hdc, px - (int)(pr * 0.6f), vy, NULL);
        LineTo(hdc, px - (int)(pr * 0.1f), vy + (int)(sinf(rot * 2.0f + v) * pr * 0.15f));
        LineTo(hdc, px + (int)(pr * 0.5f), vy - (int)(cosf(rot * 2.0f + v) * pr * 0.12f));
    }
    SelectObject(hdc, hOldP);
    DeleteObject(hMagmaPen);

    // 3. Dense Supercritical Sulfur / Acid Cloud Bands
    COLORREF bandCols[4] = { RGB(225, 155, 30), RGB(190, 115, 18), RGB(245, 190, 55), RGB(170, 95, 12) };
    for (int i = 0; i < 4; i++) {
        int by = py + (int)(((i - 1.5f) * 0.45f) * pr);
        int bh = (int)(pr * 0.32f);
        HBRUSH hBandBr = CreateSolidBrush(bandCols[i]);
        HPEN hBandP = CreatePen(PS_SOLID, 1, bandCols[i]);
        SelectObject(hdc, hBandBr);
        SelectObject(hdc, hBandP);
        Ellipse(hdc, px - pr - 5, by - bh / 2, px + pr + 5, by + bh / 2);
        DeleteObject(hBandBr);
        DeleteObject(hBandP);
    }

    // 4. Great Acid Storm Vortex
    int svX = px + (int)(sinf(rot * 0.7f) * pr * 0.45f);
    int svY = py + (int)(pr * 0.18f);
    int sw = (int)(pr * 0.35f);
    int sh = (int)(pr * 0.22f);
    HBRUSH hStormBr = CreateSolidBrush(RGB(245, 95, 25));
    HPEN hStormP = CreatePen(PS_SOLID, 2, RGB(255, 185, 40));
    SelectObject(hdc, hStormBr);
    SelectObject(hdc, hStormP);
    Ellipse(hdc, svX - sw, svY - sh, svX + sw, svY + sh);
    DeleteObject(hStormBr);
    DeleteObject(hStormP);
}

static void DrawFrozenTundraGDI(HDC hdc, CelestialBody* b, int px, int py, int pr, int sunX, int sunY, float z, float rot) {
    // 1. Cryosphere Permafrost / Solid Glacial Base
    FillSolidRect(hdc, px - pr, py - pr, pr * 2, pr * 2, RGB(125, 180, 225));

    // 2. Solid Ice Glaciers and Permafrost Sheets
    HBRUSH hIceBr = CreateSolidBrush(RGB(235, 248, 255));
    HPEN hIceP = CreatePen(PS_SOLID, 1, RGB(210, 238, 255));
    HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hIceBr);
    HPEN hOldP = (HPEN)SelectObject(hdc, hIceP);

    for (int g = 0; g < 4; g++) {
        float gAng = rot * 0.35f + g * 1.6f;
        int gx = px + (int)(sinf(gAng) * pr * 0.6f);
        int gy = py + (int)(((g % 2 == 0) ? -0.2f : 0.2f) * pr);
        int gw = (int)(pr * 0.45f * fabsf(cosf(gAng)) + 5);
        int gh = (int)(pr * 0.32f);
        Ellipse(hdc, gx - gw, gy - gh, gx + gw, gy + gh);
    }

    // 3. Deep Blue Glacial Chasms & Crevasses
    HPEN hCrevasseP = CreatePen(PS_SOLID, 1, RGB(45, 95, 165));
    SelectObject(hdc, hCrevasseP);
    for (int c = 0; c < 3; c++) {
        int cy = py + (int)(((c - 1) * 0.35f) * pr);
        MoveToEx(hdc, px - (int)(pr * 0.5f), cy, NULL);
        LineTo(hdc, px - (int)(pr * 0.1f), cy + (int)(sinf(c * 2.0f) * pr * 0.1f));
        LineTo(hdc, px + (int)(pr * 0.45f), cy - (int)(cosf(c * 1.5f) * pr * 0.08f));
    }

    // 4. Immense Polar Ice Caps (Extending across 55% of hemisphere)
    int iceH = (int)(pr * 0.42f);
    SelectObject(hdc, hIceBr);
    SelectObject(hdc, hIceP);
    Ellipse(hdc, px - pr, py - pr - iceH / 2, px + pr, py - pr + iceH * 2);
    Ellipse(hdc, px - pr, py + pr - iceH * 2, px + pr, py + pr + iceH / 2);

    SelectObject(hdc, hOldB);
    SelectObject(hdc, hOldP);
    DeleteObject(hIceBr);
    DeleteObject(hIceP);
    DeleteObject(hCrevasseP);
}

static void DrawOceanWorldGDI(HDC hdc, CelestialBody* b, int px, int py, int pr, int sunX, int sunY, float z, float rot) {
    // 1. Deep Pelagic Cobalt Abyss Base
    FillSolidRect(hdc, px - pr, py - pr, pr * 2, pr * 2, RGB(10, 48, 115));

    // 2. Shallower Oceanic Ridges & Turquoise Ocean Currents
    HBRUSH hRidgeBr = CreateSolidBrush(RGB(16, 115, 190));
    HPEN hRidgeP = CreatePen(PS_SOLID, 1, RGB(16, 115, 190));
    HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hRidgeBr);
    HPEN hOldP = (HPEN)SelectObject(hdc, hRidgeP);

    for (int r = 0; r < 3; r++) {
        float rAng = rot * 0.45f + r * 2.1f;
        int rx = px + (int)(sinf(rAng) * pr * 0.5f);
        int ry = py + (int)(((r % 2 == 0) ? -0.22f : 0.22f) * pr);
        int rw = (int)(pr * 0.5f * fabsf(cosf(rAng)) + 6);
        int rh = (int)(pr * 0.28f);
        Ellipse(hdc, rx - rw, ry - rh, rx + rw, ry + rh);
    }

    // 3. Swirling White Cyclonic Storm Arcs
    HPEN hStormP = CreatePen(PS_SOLID, 2, RGB(240, 248, 255));
    SelectObject(hdc, hStormP);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    for (int s = 0; s < 3; s++) {
        float sAng = rot * 0.7f + s * 2.0f;
        int sx = px + (int)(sinf(sAng) * pr * 0.4f);
        int sy = py + (int)(((s - 1) * 0.35f) * pr);
        int sr = (int)(pr * 0.26f);
        Arc(hdc, sx - sr, sy - sr, sx + sr, sy + sr,
                 sx - sr, sy, sx + sr, sy);
    }

    // 4. Underwater Hydrothermal Vent Glows
    for (int v = 0; v < 4; v++) {
        int vx = px + (int)(((v * 2) % 5 - 2) * 0.25f * pr);
        int vy = py + (int)(((v * 3) % 7 - 3) * 0.18f * pr);
        FillSolidRect(hdc, vx - 1, vy - 1, 3, 3, RGB(0, 245, 255));
    }

    SelectObject(hdc, hOldB);
    SelectObject(hdc, hOldP);
    DeleteObject(hRidgeBr);
    DeleteObject(hRidgeP);
    DeleteObject(hStormP);
}

static void DrawPrimordialGaiaGDI(HDC hdc, CelestialBody* b, int px, int py, int pr, int sunX, int sunY, float z, float rot) {
    // 1. Rich Azure Sapphire Oceans
    FillSolidRect(hdc, px - pr, py - pr, pr * 2, pr * 2, RGB(14, 90, 168));

    // 2. Verdant Proto-Continents & Flora Landmasses
    HBRUSH hLandBr = CreateSolidBrush(RGB(24, 135, 62));
    HPEN hLandP = CreatePen(PS_SOLID, 1, RGB(42, 170, 85));
    HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hLandBr);
    HPEN hOldP = (HPEN)SelectObject(hdc, hLandP);

    for (int c = 0; c < 3; c++) {
        float cLon = (c * 2.1f + rot);
        int cxPos = px + (int)(sinf(cLon) * pr * 0.65f);
        int cyPos = py + (int)(((c % 2 == 0) ? -0.2f : 0.2f) * pr);
        int rw = (int)(pr * 0.45f * fabsf(cosf(cLon)) + 6);
        int rh = (int)(pr * 0.35f);
        Ellipse(hdc, cxPos - rw, cyPos - rh, cxPos + rw, cyPos + rh);
    }

    // 3. Polar Ice Caps
    int iceH = (int)(pr * 0.25f);
    HBRUSH hIceBr = CreateSolidBrush(RGB(245, 250, 255));
    HPEN hIceP = CreatePen(PS_SOLID, 1, RGB(220, 240, 255));
    SelectObject(hdc, hIceBr);
    SelectObject(hdc, hIceP);
    Ellipse(hdc, px - pr, py - pr - iceH / 2, px + pr, py - pr + iceH * 2);
    Ellipse(hdc, px - pr, py + pr - iceH * 2, px + pr, py + pr + iceH / 2);
    DeleteObject(hIceBr);
    DeleteObject(hIceP);

    // 4. Swirling White Cloud Belts
    HPEN hCloudP = CreatePen(PS_SOLID, 2, RGB(248, 252, 255));
    SelectObject(hdc, hCloudP);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    for (int w = 0; w < 3; w++) {
        float wAng = rot * 1.2f + w * 2.0f;
        int wx = px + (int)(sinf(wAng) * pr * 0.4f);
        int wy = py + (int)(((w - 1) * 0.38f) * pr);
        int wr = (int)(pr * 0.24f);
        Arc(hdc, wx - wr, wy - wr, wx + wr, wy + wr, wx - wr, wy, wx + wr, wy);
    }

    SelectObject(hdc, hOldB);
    SelectObject(hdc, hOldP);
    DeleteObject(hLandBr);
    DeleteObject(hLandP);
    DeleteObject(hCloudP);
}

static void DrawExoplanetGDI(HDC hdc, CelestialBody* b, int px, int py, int pr, int sunX, int sunY, float z, float simTime, int isActiveTarget) {
    float sunAngle = atan2f((float)(sunY - py), (float)(sunX - px));
    float rot = simTime * 0.04f;

    // Atmospheric Corona Colors based on Classification
    COLORREF cHaloOuter, cHaloMid, cHaloInner;
    switch (b->pClass) {
        case CLASS_BARREN_ROCK:
            cHaloOuter = RGB(75, 45, 25);
            cHaloMid   = RGB(130, 80, 45);
            cHaloInner = RGB(190, 125, 75);
            break;
        case CLASS_TOXIC_GREENHOUSE:
            cHaloOuter = RGB(110, 60, 10);
            cHaloMid   = RGB(200, 110, 20);
            cHaloInner = RGB(245, 165, 20);
            break;
        case CLASS_FROZEN_TUNDRA:
            cHaloOuter = RGB(35, 75, 135);
            cHaloMid   = RGB(65, 140, 210);
            cHaloInner = RGB(160, 220, 255);
            break;
        case CLASS_OCEAN_WORLD:
            cHaloOuter = RGB(10, 60, 140);
            cHaloMid   = RGB(14, 130, 210);
            cHaloInner = RGB(56, 195, 255);
            break;
        case CLASS_PRIMORDIAL_GAIA:
        default:
            cHaloOuter = RGB(0, 100, 110);
            cHaloMid   = RGB(0, 180, 190);
            cHaloInner = RGB(0, 240, 255);
            break;
    }

    // Outer Stepped Corona Rings
    HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hNullBrush);

    int atmoR3 = pr + (int)(14 * z * (1.0f + b->pressure * 0.4f));
    int atmoR2 = pr + (int)(9 * z * (1.0f + b->pressure * 0.3f));
    int atmoR1 = pr + (int)(4 * z * (1.0f + b->pressure * 0.2f));

    HPEN hPen3 = CreatePen(PS_SOLID, 1, cHaloOuter);
    HPEN hOldP = (HPEN)SelectObject(hdc, hPen3);
    Ellipse(hdc, px - atmoR3, py - atmoR3, px + atmoR3, py + atmoR3);
    DeleteObject(hPen3);

    HPEN hPen2 = CreatePen(PS_SOLID, 1, cHaloMid);
    SelectObject(hdc, hPen2);
    Ellipse(hdc, px - atmoR2, py - atmoR2, px + atmoR2, py + atmoR2);
    DeleteObject(hPen2);

    HPEN hPen1 = CreatePen(PS_SOLID, 2, cHaloInner);
    SelectObject(hdc, hPen1);
    Ellipse(hdc, px - atmoR1, py - atmoR1, px + atmoR1, py + atmoR1);
    DeleteObject(hPen1);

    // Sunward Mie Scattering Limb Arc
    HPEN hLimbPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    SelectObject(hdc, hLimbPen);
    int limbSX = px + (int)(cosf(sunAngle - 1.2f) * (pr + 2));
    int limbSY = py + (int)(sinf(sunAngle - 1.2f) * (pr + 2));
    int limbEX = px + (int)(cosf(sunAngle + 1.2f) * (pr + 2));
    int limbEY = py + (int)(sinf(sunAngle + 1.2f) * (pr + 2));
    Arc(hdc, px - pr - 2, py - pr - 2, px + pr + 2, py + pr + 2, limbSX, limbSY, limbEX, limbEY);
    DeleteObject(hLimbPen);

    // Polar Auroras
    if (b->magnet > 0.15f) {
        float wave = sinf(simTime * 3.5f) * 3.0f * z;
        HPEN hAuroraPen1 = CreatePen(PS_SOLID, 2, RGB(16, 230, 160));
        SelectObject(hdc, hAuroraPen1);
        Arc(hdc, px - (int)(pr * 0.6f) + (int)wave, py - pr - (int)(5 * z),
                 px + (int)(pr * 0.6f) + (int)wave, py - pr + (int)(7 * z),
                 px - (int)(pr * 0.5f), py - pr, px + (int)(pr * 0.5f), py - pr);
        DeleteObject(hAuroraPen1);

        HPEN hAuroraPen2 = CreatePen(PS_SOLID, 2, RGB(180, 90, 245));
        SelectObject(hdc, hAuroraPen2);
        Arc(hdc, px - (int)(pr * 0.6f) - (int)wave, py + pr - (int)(7 * z),
                 px + (int)(pr * 0.6f) - (int)wave, py + pr + (int)(5 * z),
                 px + (int)(pr * 0.5f), py + pr, px - (int)(pr * 0.5f), py + pr);
        DeleteObject(hAuroraPen2);
    }

    SelectObject(hdc, hOldP);
    SelectObject(hdc, hOldBrush);

    // Clip to Planet Disc
    HRGN hRgnPlanet = CreateEllipticRgn(px - pr, py - pr, px + pr + 1, py + pr + 1);
    HRGN hOldRgn = CreateRectRgn(0, 0, 0, 0);
    GetClipRgn(hdc, hOldRgn);
    ExtSelectClipRgn(hdc, hRgnPlanet, RGN_AND);

    // Dispatch Surface Shader
    switch (b->pClass) {
        case CLASS_BARREN_ROCK:
            DrawBarrenRockGDI(hdc, b, px, py, pr, sunX, sunY, z, rot);
            break;
        case CLASS_TOXIC_GREENHOUSE:
            DrawToxicGreenhouseGDI(hdc, b, px, py, pr, sunX, sunY, z, rot);
            break;
        case CLASS_FROZEN_TUNDRA:
            DrawFrozenTundraGDI(hdc, b, px, py, pr, sunX, sunY, z, rot);
            break;
        case CLASS_OCEAN_WORLD:
            DrawOceanWorldGDI(hdc, b, px, py, pr, sunX, sunY, z, rot);
            break;
        case CLASS_PRIMORDIAL_GAIA:
        default:
            DrawPrimordialGaiaGDI(hdc, b, px, py, pr, sunX, sunY, z, rot);
            break;
    }

    // Day/Night Terminator Darkening Shading
    float darkAngle = sunAngle + 3.14159265f;
    int shadeDist = (int)(pr * 0.45f);
    int shadeX = px + (int)(cosf(darkAngle) * shadeDist);
    int shadeY = py + (int)(sinf(darkAngle) * shadeDist);

    HBRUSH hDarkBr = CreateSolidBrush(RGB(4, 7, 14));
    HPEN hDarkP = CreatePen(PS_SOLID, 1, RGB(4, 7, 14));
    HBRUSH hOldBr2 = (HBRUSH)SelectObject(hdc, hDarkBr);
    HPEN hOldP2 = (HPEN)SelectObject(hdc, hDarkP);
    Ellipse(hdc, shadeX - (int)(pr * 0.95f), shadeY - (int)(pr * 0.95f),
                 shadeX + (int)(pr * 0.95f), shadeY + (int)(pr * 0.95f));
    SelectObject(hdc, hOldBr2);
    SelectObject(hdc, hOldP2);
    DeleteObject(hDarkBr);
    DeleteObject(hDarkP);

    // Night-Side Colony Settlement Lights & Megacity Arcologies (Phase 8)
    if (isActiveTarget && sim.colonists > 0) {
        int cityOffsets[4][2] = {
            { (int)(pr * 0.35f), (int)(-pr * 0.15f) },
            { (int)(pr * 0.45f), (int)(pr * 0.10f) },
            { (int)(pr * 0.25f), (int)(pr * 0.28f) },
            { (int)(pr * 0.52f), (int)(-pr * 0.05f) }
        };
        COLORREF cityCols[4] = { RGB(255, 220, 110), RGB(0, 240, 255), RGB(16, 230, 160), RGB(255, 180, 50) };
        for (int i = 0; i < 4; i++) {
            int cxDot = shadeX + cityOffsets[i][0] / 2;
            int cyDot = shadeY + cityOffsets[i][1] / 2;
            int dotSz = (int)(2 * z);
            if (dotSz < 2) dotSz = 2;
            FillSolidRect(hdc, cxDot - dotSz / 2, cyDot - dotSz / 2, dotSz, dotSz, cityCols[i]);
        }

        // Domed Megacity Arcology rings
        if (sim.domedMegacities > 0) {
            HPEN hMegaPen = CreatePen(PS_SOLID, 1, RGB(0, 240, 255));
            HPEN hOldP4 = (HPEN)SelectObject(hdc, hMegaPen);
            for (int m = 0; m < sim.domedMegacities && m < 3; m++) {
                int mx = shadeX + (int)(pr * (0.20f + m * 0.15f));
                int my = shadeY + (int)(pr * (-0.10f + m * 0.18f));
                int mr = (int)((4 + m) * z);
                if (mr < 3) mr = 3;
                Arc(hdc, mx - mr, my - mr, mx + mr, my + mr, mx - mr, my, mx + mr, my);
                FillSolidRect(hdc, mx - 1, my - 1, 3, 3, RGB(255, 220, 120));
            }
            SelectObject(hdc, hOldP4);
            DeleteObject(hMegaPen);
        }

        // Agricultural Arrays (Hydroponics / Aeroponics)
        if ((sim.hydroTowers + sim.aeroponicFarms) > 0) {
            int fx = shadeX + (int)(pr * 0.30f);
            int fy = shadeY + (int)(pr * 0.08f);
            int fSz = (int)(2 * z);
            if (fSz < 2) fSz = 2;
            for (int f = 0; f < 4 && f < (sim.hydroTowers + sim.aeroponicFarms); f++) {
                FillSolidRect(hdc, fx + (f % 2) * (fSz + 1), fy + (f / 2) * (fSz + 1), fSz, fSz, RGB(16, 230, 130));
            }
        }
    }

    SelectClipRgn(hdc, hOldRgn);
    DeleteObject(hOldRgn);
    DeleteObject(hRgnPlanet);

    // Active Target Terraforming Visual Effects
    if (isActiveTarget) {
        // 1. Stratospheric Greenhouse Aerosol Haze Ring
        if (sim.greenhouseStations > 0 || sim.greenhouse > 5.0f) {
            float pulse = sinf(simTime * 3.0f) * 2.0f * z;
            int ghgR = pr + (int)(6 * z) + (int)pulse;
            COLORREF ghgCol = (sim.greenhouseMode == 0) ? RGB(255, 140, 50) : RGB(100, 200, 255);
            HPEN hGhgPen = CreatePen(PS_DOT, 1, ghgCol);
            HPEN hOldP3 = (HPEN)SelectObject(hdc, hGhgPen);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Ellipse(hdc, px - ghgR, py - ghgR, px + ghgR, py + ghgR);
            SelectObject(hdc, hOldP3);
            DeleteObject(hGhgPen);
        }

        // 2. Tropospheric Gas Injector & Nitrogen Mantle Exhaust Plumes
        if (sim.atmoProcessors > 0) {
            HPEN hPlumePen = CreatePen(PS_SOLID, 2, (sim.atmoMode == 0) ? RGB(0, 240, 255) : RGB(255, 180, 80));
            HPEN hOldP4 = (HPEN)SelectObject(hdc, hPlumePen);
            float pAng = 0.85f;
            int plx = px + (int)(cosf(pAng) * pr);
            int ply = py + (int)(sinf(pAng) * pr);
            float flen = 9.0f * z + sinf(simTime * 14.0f) * 2.5f * z;
            MoveToEx(hdc, plx, ply, NULL);
            LineTo(hdc, plx + (int)(cosf(pAng) * flen), ply + (int)(sinf(pAng) * flen));
            SelectObject(hdc, hOldP4);
            DeleteObject(hPlumePen);
        }
        if (sim.nitrogenExtractors > 0) {
            HPEN hN2Pen = CreatePen(PS_SOLID, 2, RGB(180, 90, 255));
            HPEN hOldP5 = (HPEN)SelectObject(hdc, hN2Pen);
            float nAng = 2.45f;
            int nlx = px + (int)(cosf(nAng) * pr);
            int nly = py + (int)(sinf(nAng) * pr);
            float nlen = 8.5f * z + sinf(simTime * 16.0f + 1.2f) * 2.0f * z;
            MoveToEx(hdc, nlx, nly, NULL);
            LineTo(hdc, nlx + (int)(cosf(nAng) * nlen), nly + (int)(sinf(nAng) * nlen));
            SelectObject(hdc, hOldP5);
            DeleteObject(hN2Pen);
        }

        // 3. Orbital Solar Mirrors & Insolation Beams / Shade Arcs
        if (sim.solarMirrors > 0) {
            int count = sim.solarMirrors;
            if (count > 8) count = 8;
            float mOrbitR = (float)pr + 20.0f * z;
            for (int m = 0; m < count; m++) {
                float mAng = simTime * 0.35f + (float)m * (6.2831853f / (float)count);
                int mx = px + (int)(cosf(mAng) * mOrbitR);
                int my = py + (int)(sinf(mAng) * (mOrbitR * 0.7f));

                // Facet Statite Box
                int msz = (int)(4 * z);
                if (msz < 3) msz = 3;
                FillSolidRect(hdc, mx - msz / 2, my - msz / 2, msz, msz, (sim.mirrorMode == 0) ? RGB(255, 230, 100) : RGB(100, 210, 255));

                if (sim.mirrorMode == 0) {
                    // Golden Focus Beam down to planetary target
                    HPEN hBmPen = CreatePen(PS_SOLID, 1, RGB(255, 210, 50));
                    HPEN hOldBm = (HPEN)SelectObject(hdc, hBmPen);
                    MoveToEx(hdc, mx, my, NULL);
                    LineTo(hdc, px + (int)(cosf(mAng) * pr * 0.45f), py + (int)(sinf(mAng) * pr * 0.45f));
                    SelectObject(hdc, hOldBm);
                    DeleteObject(hBmPen);
                } else {
                    // Cyan Deflection Shade Arc
                    HPEN hShPen = CreatePen(PS_DOT, 1, RGB(100, 200, 255));
                    HPEN hOldSh = (HPEN)SelectObject(hdc, hShPen);
                    int shR = (int)(6 * z);
                    Arc(hdc, mx - shR, my - shR, mx + shR, my + shR, mx - shR, my, mx + shR, my);
                    SelectObject(hdc, hOldSh);
                    DeleteObject(hShPen);
                }
            }
        }

        // Phase 11: 4. Planetary Shield Grid Forcefield Envelope
        if (sim.shieldGridStage > 0 && sim.shieldHP > 0) {
            int sR = pr + (int)(14 * z);
            COLORREF cShield = (sim.shieldMode == 1) ? RGB(168, 85, 247) : RGB(0, 240, 255);
            HPEN hSPen = CreatePen(PS_DOT, 1, cShield);
            HPEN hOldSP = (HPEN)SelectObject(hdc, hSPen);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Ellipse(hdc, px - sR, py - sR, px + sR, py + sR);
            SelectObject(hdc, hOldSP);
            DeleteObject(hSPen);

            if (sim.shieldFlareAnim > 0.01f) {
                int flR = sR + (int)((1.0f - sim.shieldFlareAnim) * 22.0f * z);
                HPEN hFlPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                HPEN hOldFl = (HPEN)SelectObject(hdc, hFlPen);
                Ellipse(hdc, px - flR, py - flR, px + flR, py + flR);
                SelectObject(hdc, hOldFl);
                DeleteObject(hFlPen);
            }
        }

        // Phase 11: 5. Equatorial Orbital Ring
        if (sim.orbitalRingStage > 0) {
            int rW = pr + (int)(28 * z);
            int rH = (int)(rW * 0.38f);
            COLORREF cRing = (sim.orbitalRingStage == 3) ? RGB(245, 158, 11) : RGB(56, 189, 248);
            HPEN hRingPen = CreatePen(PS_SOLID, (sim.orbitalRingStage == 3) ? 2 : 1, cRing);
            HPEN hOldRing = (HPEN)SelectObject(hdc, hRingPen);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Ellipse(hdc, px - rW, py - rH, px + rW, py + rH);
            Ellipse(hdc, px - (rW - 3), py - (rH - 2), px + (rW - 3), py + (rH - 2));

            // Spokes to planet
            MoveToEx(hdc, px, py - rH, NULL); LineTo(hdc, px, py - pr);
            MoveToEx(hdc, px, py + rH, NULL); LineTo(hdc, px, py + pr);
            MoveToEx(hdc, px - rW, py, NULL); LineTo(hdc, px - pr, py);
            MoveToEx(hdc, px + rW, py, NULL); LineTo(hdc, px + pr, py);
            SelectObject(hdc, hOldRing);
            DeleteObject(hRingPen);

            // Ring Hub Nodes
            int hubCount = (sim.orbitalRingStage == 3) ? 8 : 4;
            for (int h = 0; h < hubCount; h++) {
                float hAng = simTime * 0.1f + (float)h * (6.2831853f / (float)hubCount);
                int hx = px + (int)(cosf(hAng) * (float)rW);
                int hy = py + (int)(sinf(hAng) * (float)rH);
                FillSolidRect(hdc, hx - 2, hy - 2, 5, 5, (sim.orbitalRingStage == 3) ? RGB(251, 191, 36) : RGB(0, 240, 255));
            }
        }

        // Phase 11: 6. Star Elevator (Space Elevator Ribbon)
        if (sim.starElevatorStage > 0) {
            int elTopY = py - (pr + (int)(36 * z));
            HPEN hElPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 200));
            HPEN hOldEl = (HPEN)SelectObject(hdc, hElPen);
            MoveToEx(hdc, px, py - pr, NULL);
            LineTo(hdc, px, elTopY);
            SelectObject(hdc, hOldEl);
            DeleteObject(hElPen);

            // Geostationary anchor terminal
            FillSolidRect(hdc, px - 3, elTopY - 3, 6, 6, RGB(0, 240, 255));

            // Climber pod
            float podProg = fmodf(simTime * 0.35f, 1.0f);
            int podY = (py - pr) - (int)((36.0f * z) * podProg);
            FillSolidRect(hdc, px - 2, podY - 2, 4, 4, RGB(254, 240, 138));
        }

        // Phase 11: 7. Armed Planetary Defense Bastions
        if (sim.defenseStationStage > 0) {
            int numDef = sim.defenseStationStage;
            float defDist = (float)pr + 44.0f * z;
            for (int d = 0; d < numDef; d++) {
                float dAng = simTime * 0.22f + (float)d * (6.2831853f / (float)numDef);
                int dx = px + (int)(cosf(dAng) * defDist);
                int dy = py + (int)(sinf(dAng) * (defDist * 0.7f));
                FillSolidRect(hdc, dx - 3, dy - 3, 6, 6, RGB(244, 63, 94));
                FrameSolidRect(hdc, dx - 4, dy - 4, 8, 8, RGB(253, 164, 175));

                if (sim.defenseFireAnim > 0.01f) {
                    HPEN hBm = CreatePen(PS_SOLID, 2, RGB(255, 60, 60));
                    HPEN hOldBm = (HPEN)SelectObject(hdc, hBm);
                    MoveToEx(hdc, dx, dy, NULL);
                    LineTo(hdc, dx + (int)(cosf(dAng) * 140.0f * z), dy + (int)(sinf(dAng) * 140.0f * z));
                    SelectObject(hdc, hOldBm);
                    DeleteObject(hBm);
                }
            }
        }

        // Phase 12: 8. Ancient Precursor Ruins & Xenobiology Sites
        static const float s_xenoAngles[4] = { -0.85f, 0.60f, 2.05f, 3.50f };
        for (int x = 0; x < 4; x++) {
            float xAng = s_xenoAngles[x];
            int sx = px + (int)(cosf(xAng) * (float)pr);
            int sy = py + (int)(sinf(xAng) * (float)pr);

            if (sim.xenoSiteStatus[x] == 2) {
                // Excavated Golden / Tachyon Crystalline Spire
                int spireTipX = px + (int)(cosf(xAng) * (float)(pr + (int)(10.0f * z)));
                int spireTipY = py + (int)(sinf(xAng) * (float)(pr + (int)(10.0f * z)));

                HPEN hSpire = CreatePen(PS_SOLID, 2, RGB(245, 158, 11));
                HPEN hOldSpire = (HPEN)SelectObject(hdc, hSpire);
                MoveToEx(hdc, sx, sy, NULL);
                LineTo(hdc, spireTipX, spireTipY);
                SelectObject(hdc, hOldSpire);
                DeleteObject(hSpire);

                FillSolidRect(hdc, spireTipX - 2, spireTipY - 2, 5, 5, RGB(251, 191, 36));

                // Pulsing Tachyon Halo
                int pulseR = (int)((4.0f + sinf(simTime * 3.0f + (float)x) * 2.0f) * z);
                HPEN hHalo = CreatePen(PS_SOLID, 1, RGB(168, 85, 247));
                HPEN hOldHalo = (HPEN)SelectObject(hdc, hHalo);
                SelectObject(hdc, GetStockObject(NULL_BRUSH));
                Ellipse(hdc, spireTipX - pulseR, spireTipY - pulseR, spireTipX + pulseR, spireTipY + pulseR);
                SelectObject(hdc, hOldHalo);
                DeleteObject(hHalo);
            } else if (sim.xenoSiteStatus[x] == 1) {
                // Active Excavation Drone & Laser Beam
                FillSolidRect(hdc, sx - 2, sy - 2, 5, 5, RGB(245, 158, 11));

                float droneDist = (float)pr + 32.0f * z;
                int droneX = px + (int)(cosf(xAng) * droneDist);
                int droneY = py + (int)(sinf(xAng) * droneDist);

                FillSolidRect(hdc, droneX - 3, droneY - 2, 6, 4, RGB(56, 189, 248));

                HPEN hLaser = CreatePen(PS_SOLID, 1, RGB(0, 240, 255));
                HPEN hOldLaser = (HPEN)SelectObject(hdc, hLaser);
                MoveToEx(hdc, droneX, droneY, NULL);
                LineTo(hdc, sx, sy);
                SelectObject(hdc, hOldLaser);
                DeleteObject(hLaser);

                // Laser impact spark
                FillSolidRect(hdc, sx - 1, sy - 1, 3, 3, RGB(255, 255, 255));
            } else {
                // Surveyed Ruin Anomaly Marker
                FillSolidRect(hdc, sx - 2, sy - 2, 4, 4, RGB(168, 85, 247));
            }
        }
    }
}

static void DrawPlanetGDI(HDC hdc, int px, int py, int pr, int sunX, int sunY, float z) {
    DrawExoplanetGDI(hdc, GetActivePlanet(), px, py, pr, sunX, sunY, z, sim.time, 1);
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

#define MAX_BUTTONS 160
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
#define BID_TAB_RESEARCH    13
#define BID_TAB_MEGAS       14
#define BID_TAB_XENO        15
#define BID_TAB_HAZARDS     16
#define BID_TAB_ECONOMY     17

#define BID_XENO_SCAN           1700
#define BID_XENO_EXCAV_1        1701
#define BID_XENO_EXCAV_2        1702
#define BID_XENO_EXCAV_3        1703
#define BID_XENO_EXCAV_4        1704
#define BID_XENO_FAST_1         1705
#define BID_XENO_FAST_2         1706
#define BID_XENO_FAST_3         1707
#define BID_XENO_FAST_4         1708
#define BID_XENO_TECH_1         1711
#define BID_XENO_TECH_2         1712
#define BID_XENO_TECH_3         1713
#define BID_XENO_TECH_4         1714

#define BID_MEGA_RING_UPG       1601
#define BID_MEGA_ELEV_UPG       1602
#define BID_MEGA_SHIELD_UPG     1603
#define BID_MEGA_DEF_UPG        1604
#define BID_MEGA_SHIELD_CHARGE  1605
#define BID_MEGA_SHIELD_MODE    1606
#define BID_MEGA_DEF_TEST       1607

#define BID_TECH_FOCUS_BASE 1500
#define BID_TECH_UNLOCK_BASE 1520

#define BID_FOCUS_PLANET    20
#define BID_FOCUS_ARK       21
#define BID_ZOOM_IN         22
#define BID_ZOOM_OUT        23
#define BID_RESET_VIEW      24
#define BID_THEME_TOGGLE    25
#define BID_GRID_TOGGLE     26
#define BID_GLOW_TOGGLE     27

#define BID_SPEED_PAUSE     30
#define BID_SPEED_1X        31
#define BID_SPEED_2X        32
#define BID_SPEED_5X        33
#define BID_AUDIO_TOGGLE    34
#define BID_SYS_CYCLE       35
#define BID_SYS_SCAN        36
#define BID_TARGET_PLANET   37
#define BID_ROSTER_BASE     150

#define BID_ACT_SOLAR_MIRROR 40
#define BID_TOG_MIRROR_MODE  41
#define BID_ACT_ATMO_PROC    42
#define BID_TOG_ATMO_MODE    43
#define BID_ACT_N2_EXTRACTOR 44
#define BID_ACT_GREENHOUSE   45
#define BID_TOG_GHG_MODE     46
#define BID_ACT_COMET_DROP   47
#define BID_ACT_BIOSEED      48
#define BID_ACT_CORE_DYNAMO  49
#define BID_ACT_ALGAE        50

#define BID_COL_AWAKEN      55
#define BID_COL_DOME        56
#define BID_COL_HYDRO       57
#define BID_COL_SOLAR       58
#define BID_COL_MEGACITY    85
#define BID_COL_LAVATUBES   86
#define BID_COL_AERO_FARM   87
#define BID_COL_ALGAL_VAT   88
#define BID_COL_RATION_0    89
#define BID_COL_RATION_1    90
#define BID_COL_RATION_2    91
#define BID_COL_FOCUS_0     92
#define BID_COL_FOCUS_1     93
#define BID_COL_FOCUS_2     94
#define BID_COL_FOCUS_3     95

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

#define BID_ROUTE_TOG_0     110
#define BID_ROUTE_TOG_1     111
#define BID_ROUTE_TOG_2     112
#define BID_ROUTE_TOG_3     113
#define BID_ROUTE_ADD_0     114
#define BID_ROUTE_ADD_1     115
#define BID_ROUTE_ADD_2     116
#define BID_ROUTE_ADD_3     117
#define BID_UPG_DOCKS       120
#define BID_UPG_FUEL        121
#define BID_UPG_MASS        122

// --- Action Handlers ---
static void HandleIntervention(int bid) {
    char buf[128];
    switch (bid) {
        case BID_ACT_SOLAR_MIRROR:
            if (sim.energy >= 350 && sim.minerals >= 150) {
                sim.energy -= 350;
                sim.minerals -= 150;
                sim.solarMirrors++;
                if (sim.mirrorMode == 0) {
                    sim.temp += 1.8f;
                    sprintf(buf, "Orbital Solar Mirror #%d deployed (+1.8 C insolation).", sim.solarMirrors);
                } else {
                    sim.temp -= 2.2f;
                    sprintf(buf, "Orbital Solar Shade #%d deployed (-2.2 C occultation).", sim.solarMirrors);
                }
                CalculateHabitability();
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                SetLogMsg("Insufficient resources (Req: 350 kW Energy, 150 t Minerals).", 1);
            }
            break;
        case BID_TOG_MIRROR_MODE:
            sim.mirrorMode = !sim.mirrorMode;
            sprintf(buf, "Solar Mirror Mode: %s", sim.mirrorMode == 0 ? "FOCUS INSOLATION (+T)" : "SOLAR SHADE COOLING (-T)");
            SetLogMsg(buf, 0);
            PlaySoundFx(SFX_CLICK);
            break;
        case BID_ACT_ATMO_PROC:
            if (sim.minerals >= 180 && sim.energy >= 100) {
                sim.minerals -= 180;
                sim.energy -= 100;
                sim.atmoProcessors++;
                if (sim.atmoMode == 0) {
                    sim.pressure += 0.05f;
                    sprintf(buf, "Troposphere Gas Injector #%d constructed (+0.05 atm).", sim.atmoProcessors);
                } else {
                    sim.pressure -= 0.08f;
                    if (sim.pressure < 1.0f) sim.pressure = 1.0f;
                    sim.minerals += 25;
                    sprintf(buf, "Catalytic Gas Scrubber #%d constructed (-0.08 atm toxic scrub).", sim.atmoProcessors);
                }
                CalculateHabitability();
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                SetLogMsg("Insufficient resources (Req: 180 t Minerals, 100 kW Energy).", 1);
            }
            break;
        case BID_TOG_ATMO_MODE:
            sim.atmoMode = !sim.atmoMode;
            sprintf(buf, "Atmo Processor Mode: %s", sim.atmoMode == 0 ? "BUFFER INJECTION (+P)" : "TOXIC SCRUBBING (-P)");
            SetLogMsg(buf, 0);
            PlaySoundFx(SFX_CLICK);
            break;
        case BID_ACT_N2_EXTRACTOR:
            if (sim.energy >= 280 && sim.minerals >= 180 && sim.volatiles >= 80) {
                sim.energy -= 280;
                sim.minerals -= 180;
                sim.volatiles -= 80;
                sim.nitrogenExtractors++;
                sim.nitrogen += 4.5f;
                if (sim.nitrogen > 78.0f) sim.nitrogen = 78.0f;
                sim.pressure += 0.02f;
                CalculateHabitability();
                sprintf(buf, "Nitrogen Mantle Bore #%d online (+4.5%% N2, +0.02 atm).", sim.nitrogenExtractors);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                SetLogMsg("Insufficient resources (Req: 280 kW Energy, 180 t Min, 80 t Vol).", 1);
            }
            break;
        case BID_ACT_GREENHOUSE:
            if (sim.energy >= 200 && sim.volatiles >= 140 && sim.food >= 60) {
                sim.energy -= 200;
                sim.volatiles -= 140;
                sim.food -= 60;
                sim.greenhouseStations++;
                if (sim.greenhouseMode == 0) {
                    sim.greenhouse += 5.0f;
                    if (sim.greenhouse > 250.0f) sim.greenhouse = 250.0f;
                    sprintf(buf, "Greenhouse Seeding Grid #%d active (+5.0%% PFC warming).", sim.greenhouseStations);
                } else {
                    sim.greenhouse -= 6.0f;
                    if (sim.greenhouse < 30.0f) sim.greenhouse = 30.0f;
                    sprintf(buf, "Greenhouse Aerosol Veil #%d active (-6.0%% albedo cooling).", sim.greenhouseStations);
                }
                CalculateHabitability();
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                SetLogMsg("Insufficient resources (Req: 200 kW Energy, 140 t Vol, 60 t Food).", 1);
            }
            break;
        case BID_TOG_GHG_MODE:
            sim.greenhouseMode = !sim.greenhouseMode;
            sprintf(buf, "Greenhouse Seeding Mode: %s", sim.greenhouseMode == 0 ? "PFC SUPER-WARMING (+GHG)" : "AEROSOL REFLECTIVE CLOUD (-GHG)");
            SetLogMsg(buf, 0);
            PlaySoundFx(SFX_CLICK);
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
    char buf[128];
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
                sim.geodesicDomes++;
                sim.housingCap = (sim.geodesicDomes * 15000) + (sim.subterraneanVaults * 30000) + (sim.domedMegacities * 50000);
                sprintf(buf, "Geodesic Dome #%d constructed (+15,000 Housing Capacity).", sim.geodesicDomes);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                SetLogMsg("Insufficient resources (Req: 450 t Minerals, 150 kW Energy).", 1);
            }
            break;
        case BID_COL_LAVATUBES:
            if (sim.minerals >= 650 && sim.energy >= 100) {
                sim.minerals -= 650;
                sim.energy -= 100;
                sim.subterraneanVaults++;
                sim.housingCap = (sim.geodesicDomes * 15000) + (sim.subterraneanVaults * 30000) + (sim.domedMegacities * 50000);
                sprintf(buf, "Subterranean Lava Tube Vault #%d sealed (+30,000 Shielded Housing).", sim.subterraneanVaults);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                SetLogMsg("Insufficient resources (Req: 650 t Minerals, 100 kW Energy).", 1);
            }
            break;
        case BID_COL_MEGACITY:
            if (sim.minerals >= 1200 && sim.energy >= 450 && sim.volatiles >= 300) {
                sim.minerals -= 1200;
                sim.energy -= 450;
                sim.volatiles -= 300;
                sim.domedMegacities++;
                sim.housingCap = (sim.geodesicDomes * 15000) + (sim.subterraneanVaults * 30000) + (sim.domedMegacities * 50000);
                sprintf(buf, "Biosphere Domed Megacity Arcology #%d inaugurated (+50,000 Housing, +6%% Morale).", sim.domedMegacities);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_SUCCESS);
            } else {
                SetLogMsg("Insufficient resources (Req: 1,200 Min, 450 kW, 300 Volatiles).", 1);
            }
            break;
        case BID_COL_HYDRO:
            if (sim.minerals >= 200 && sim.volatiles >= 100) {
                sim.minerals -= 200;
                sim.volatiles -= 100;
                sim.hydroTowers++;
                sprintf(buf, "Vertical Hydroponic Agronomy Tower #%d operational (+25 Food/cyc).", sim.hydroTowers);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                SetLogMsg("Insufficient resources (Req: 200 t Minerals, 100 t Volatiles).", 1);
            }
            break;
        case BID_COL_AERO_FARM:
            if (sim.minerals >= 550 && sim.volatiles >= 250 && sim.energy >= 120) {
                sim.minerals -= 550;
                sim.volatiles -= 250;
                sim.energy -= 120;
                sim.aeroponicFarms++;
                sprintf(buf, "Aeroponic Biosphere Mega-Farm #%d commissioned (+80 Food/cyc).", sim.aeroponicFarms);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                SetLogMsg("Insufficient resources (Req: 550 Min, 250 Vol, 120 kW).", 1);
            }
            break;
        case BID_COL_ALGAL_VAT:
            if (sim.minerals >= 320 && sim.energy >= 180) {
                sim.minerals -= 320;
                sim.energy -= 180;
                sim.algalVats++;
                sprintf(buf, "Algal Protein Synthesis Bio-Vat #%d online (+45 Food/cyc).", sim.algalVats);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                SetLogMsg("Insufficient resources (Req: 320 Min, 180 kW).", 1);
            }
            break;
        case BID_COL_SOLAR:
            if (sim.minerals >= 220) {
                sim.minerals -= 220;
                sim.surfaceSolar++;
                sprintf(buf, "Surface Photovoltaic Field #%d connected to power grid (+180 kW).", sim.surfaceSolar);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                SetLogMsg("Insufficient resources (Req: 220 t Minerals).", 1);
            }
            break;
        case BID_COL_RATION_0:
            sim.rationPolicy = 0;
            SetLogMsg("Rations Policy set to SPARTAN (0.6x Food, -12% Morale, 0.5x Growth).", 0);
            PlaySoundFx(SFX_CLICK);
            break;
        case BID_COL_RATION_1:
            sim.rationPolicy = 1;
            SetLogMsg("Rations Policy set to STANDARD SUSTENANCE (1.0x Food, Baseline Morale).", 0);
            PlaySoundFx(SFX_CLICK);
            break;
        case BID_COL_RATION_2:
            sim.rationPolicy = 2;
            SetLogMsg("Rations Policy set to ABUNDANT FEASTS (1.5x Food, +15% Morale, 2.0x Growth).", 0);
            PlaySoundFx(SFX_CLICK);
            break;
        case BID_COL_FOCUS_0:
            sim.demoFocus = 0;
            sim.pctLaborers = 40; sim.pctAgronomists = 25; sim.pctEngineers = 20; sim.pctScientists = 15;
            SetLogMsg("Demographics Focus: BALANCED (40% Lab, 25% Agro, 20% Eng, 15% Sci).", 0);
            PlaySoundFx(SFX_CLICK);
            break;
        case BID_COL_FOCUS_1:
            sim.demoFocus = 1;
            sim.pctLaborers = 25; sim.pctAgronomists = 50; sim.pctEngineers = 15; sim.pctScientists = 10;
            SetLogMsg("Demographics Focus: AGRONOMY & BIOSPHERE (50% Agro, +30% Food Yield).", 0);
            PlaySoundFx(SFX_CLICK);
            break;
        case BID_COL_FOCUS_2:
            sim.demoFocus = 2;
            sim.pctLaborers = 25; sim.pctAgronomists = 20; sim.pctEngineers = 40; sim.pctScientists = 15;
            SetLogMsg("Demographics Focus: GEO-ENGINEERING (40% Eng, Power Savings & Faster Terraforming).", 0);
            PlaySoundFx(SFX_CLICK);
            break;
        case BID_COL_FOCUS_3:
            sim.demoFocus = 3;
            sim.pctLaborers = 25; sim.pctAgronomists = 20; sim.pctEngineers = 15; sim.pctScientists = 40;
            SetLogMsg("Demographics Focus: XENOSCIENCE & RESEARCH (40% Sci, Morale Buffer & Fast Integration).", 0);
            PlaySoundFx(SFX_CLICK);
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

static void HandleTradeRoute(int bid) {
    char buf[128];
    if (bid >= BID_ROUTE_TOG_0 && bid <= BID_ROUTE_TOG_3) {
        int idx = bid - BID_ROUTE_TOG_0;
        g_tradeRoutes[idx].active = !g_tradeRoutes[idx].active;
        sprintf(buf, "%s %s.", g_tradeRoutes[idx].name, g_tradeRoutes[idx].active ? "resumed and active" : "suspended");
        SetLogMsg(buf, 0);
        PlaySoundFx(SFX_CLICK);
    } else if (bid >= BID_ROUTE_ADD_0 && bid <= BID_ROUTE_ADD_3) {
        int idx = bid - BID_ROUTE_ADD_0;
        TradeRoute* r = &g_tradeRoutes[idx];
        if (sim.minerals >= r->costMin && sim.energy >= r->costEnergy) {
            sim.minerals -= r->costMin;
            sim.energy -= r->costEnergy;
            r->freighters++;
            r->costMin += 35;
            r->costEnergy += 25;
            sprintf(buf, "Commissioned additional hauler for %s (Total: %d).", r->name, r->freighters);
            SetLogMsg(buf, 0);
            PlaySoundFx(SFX_DEPLOY);
        } else {
            sprintf(buf, "Insufficient resources (Req: %d Min, %d Energy).", r->costMin, r->costEnergy);
            SetLogMsg(buf, 1);
        }
    }
}

static void HandleInfrastructure(int bid) {
    char buf[128];
    switch (bid) {
        case BID_UPG_DOCKS: {
            int costMin = 260 + (sim.orbitalDocks - 1) * 80;
            int costEng = 180 + (sim.orbitalDocks - 1) * 60;
            if (sim.minerals >= costMin && sim.energy >= costEng) {
                sim.minerals -= costMin;
                sim.energy -= costEng;
                sim.orbitalDocks++;
                sprintf(buf, "Zephyr Orbital Docks upgraded to Tier %d (+%d%% Throughput).", sim.orbitalDocks, sim.orbitalDocks * 20);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                sprintf(buf, "Insufficient resources (Req: %d Min, %d Energy).", costMin, costEng);
                SetLogMsg(buf, 1);
            }
            break;
        }
        case BID_UPG_FUEL: {
            int costMin = 200 + (sim.fuelDepots - 1) * 70;
            int costVol = 180 + (sim.fuelDepots - 1) * 60;
            if (sim.minerals >= costMin && sim.volatiles >= costVol) {
                sim.minerals -= costMin;
                sim.volatiles -= costVol;
                sim.fuelDepots++;
                sprintf(buf, "He-3 Fuel Depot expanded to Tier %d (+%d kW, +%d%% Speed).", sim.fuelDepots, sim.fuelDepots * 40, sim.fuelDepots * 25);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                sprintf(buf, "Insufficient resources (Req: %d Min, %d Volatiles).", costMin, costVol);
                SetLogMsg(buf, 1);
            }
            break;
        }
        case BID_UPG_MASS: {
            int costMin = 300 + (sim.massDrivers - 1) * 90;
            int costEng = 200 + (sim.massDrivers - 1) * 70;
            if (sim.minerals >= costMin && sim.energy >= costEng) {
                sim.minerals -= costMin;
                sim.energy -= costEng;
                sim.massDrivers++;
                sprintf(buf, "Surface Mass Driver expanded to Tier %d (+%d t/cyc catapult).", sim.massDrivers, sim.massDrivers * 12);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                sprintf(buf, "Insufficient resources (Req: %d Min, %d Energy).", costMin, costEng);
                SetLogMsg(buf, 1);
            }
            break;
        }
        // Phase 11: Orbital Megastructures & Planetary Defense Handlers
        case BID_MEGA_RING_UPG: {
            int costMin = 1200, costEng = 400;
            if (sim.orbitalRingStage == 1) { costMin = 2800; costEng = 900; }
            else if (sim.orbitalRingStage == 2) { costMin = 5500; costEng = 1800; }
            else if (sim.orbitalRingStage >= 3) break;

            if (sim.precursorTech[2]) {
                costMin = (int)(costMin * 0.75f);
                costEng = (int)(costEng * 0.75f);
            }

            if (sim.minerals >= costMin && sim.energy >= costEng) {
                sim.minerals -= costMin;
                sim.energy -= costEng;
                sim.orbitalRingStage++;
                sprintf(buf, "Orbital Ring upgraded to Stage %d! Habitation & trade expanded.", sim.orbitalRingStage);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_SUCCESS);
            } else {
                sprintf(buf, "Insufficient resources for Orbital Ring (Req: %d Min, %d kW).", costMin, costEng);
                SetLogMsg(buf, 1);
            }
            break;
        }
        case BID_MEGA_ELEV_UPG: {
            int costMin = 1000, costEng = 350;
            if (sim.starElevatorStage == 1) { costMin = 2400; costEng = 750; }
            else if (sim.starElevatorStage == 2) { costMin = 4800; costEng = 1500; }
            else if (sim.starElevatorStage >= 3) break;

            if (sim.precursorTech[2]) {
                costMin = (int)(costMin * 0.75f);
                costEng = (int)(costEng * 0.75f);
            }

            if (sim.minerals >= costMin && sim.energy >= costEng) {
                sim.minerals -= costMin;
                sim.energy -= costEng;
                sim.starElevatorStage++;
                sprintf(buf, "Star Elevator advanced to Stage %d! Ground-to-orbit tether online.", sim.starElevatorStage);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_SUCCESS);
            } else {
                sprintf(buf, "Insufficient resources for Star Elevator (Req: %d Min, %d kW).", costMin, costEng);
                SetLogMsg(buf, 1);
            }
            break;
        }
        case BID_MEGA_SHIELD_UPG: {
            int costMin = 3200, costEng = 1200;
            if (sim.shieldGridStage == 2) { costMin = 6000; costEng = 2200; }
            else if (sim.shieldGridStage >= 3) break;

            if (sim.precursorTech[2]) {
                costMin = (int)(costMin * 0.75f);
                costEng = (int)(costEng * 0.75f);
            }

            if (sim.minerals >= costMin && sim.energy >= costEng) {
                sim.minerals -= costMin;
                sim.energy -= costEng;
                sim.shieldGridStage++;
                sim.shieldMaxHP = (sim.shieldGridStage == 2 ? 900 : 1800);
                if (sim.precursorTech[1]) sim.shieldMaxHP += 500;
                if (sim.shieldMode == 1) sim.shieldMaxHP = (int)(sim.shieldMaxHP * 1.5f);
                sim.shieldHP = sim.shieldMaxHP;
                sim.shieldFlareAnim = 1.0f;
                CalculateHabitability();
                sprintf(buf, "Planetary Shield Grid advanced to Stage %d! Deflector Max: %d HP.", sim.shieldGridStage, sim.shieldMaxHP);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_SUCCESS);
            } else {
                sprintf(buf, "Insufficient resources for Shield Grid (Req: %d Min, %d kW).", costMin, costEng);
                SetLogMsg(buf, 1);
            }
            break;
        }
        case BID_MEGA_DEF_UPG: {
            int costMin = 1100, costEng = 300;
            if (sim.defenseStationStage == 1) { costMin = 2600; costEng = 800; }
            else if (sim.defenseStationStage == 2) { costMin = 5200; costEng = 1600; }
            else if (sim.defenseStationStage >= 3) break;

            if (sim.precursorTech[2]) {
                costMin = (int)(costMin * 0.75f);
                costEng = (int)(costEng * 0.75f);
            }

            if (sim.minerals >= costMin && sim.energy >= costEng) {
                sim.minerals -= costMin;
                sim.energy -= costEng;
                sim.defenseStationStage++;
                sprintf(buf, "Planetary Defense Citadel upgraded to Stage %d! Orbital weapons armed.", sim.defenseStationStage);
                SetLogMsg(buf, 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                sprintf(buf, "Insufficient resources for Defense Station (Req: %d Min, %d kW).", costMin, costEng);
                SetLogMsg(buf, 1);
            }
            break;
        }
        case BID_MEGA_SHIELD_CHARGE: {
            if (sim.energy >= 180) {
                sim.energy -= 180;
                sim.shieldHP += 300;
                if (sim.shieldHP > sim.shieldMaxHP) sim.shieldHP = sim.shieldMaxHP;
                sim.shieldFlareAnim = 1.0f;
                SetLogMsg("Emergency power dumped into deflector lattice! +300 Shield HP infused.", 0);
                PlaySoundFx(SFX_SUCCESS);
            } else {
                SetLogMsg("Insufficient energy for shield overcharge (Req: 180 kW).", 1);
            }
            break;
        }
        case BID_MEGA_SHIELD_MODE: {
            sim.shieldMode = (sim.shieldMode + 1) % 3;
            sim.shieldMaxHP = (sim.shieldGridStage == 1 ? 400 : (sim.shieldGridStage == 2 ? 900 : (sim.shieldGridStage == 3 ? 1800 : 0)));
            if (sim.precursorTech[1]) sim.shieldMaxHP += 500;
            if (sim.shieldMode == 1) sim.shieldMaxHP = (int)(sim.shieldMaxHP * 1.5f);
            if (sim.shieldHP > sim.shieldMaxHP) sim.shieldHP = sim.shieldMaxHP;
            const char* mNames[] = { "BALANCED (30 kW, +15 HP)", "FORTIFIED (+50% Cap, 80 kW, +35 HP)", "STANDBY (0 kW, Passive)" };
            sprintf(buf, "Shield Grid Mode: %s", mNames[sim.shieldMode]);
            SetLogMsg(buf, 0);
            PlaySoundFx(SFX_CLICK);
            break;
        }
        case BID_MEGA_DEF_TEST: {
            if (sim.defenseStationStage > 0) {
                sim.defenseFireAnim = 1.0f;
                SetLogMsg("Defense citadel: test fire burst discharged across orbital coordinates.", 0);
                PlaySoundFx(SFX_DEPLOY);
            } else {
                SetLogMsg("No orbital defense stations commissioned yet.", 1);
            }
            break;
        }
        // Phase 12: Alien Xenobiology & Precursor Handlers
        case BID_XENO_SCAN: {
            if (sim.energy >= 60) {
                sim.energy -= 60;
                sim.science += 20;
                sim.xenoScanAnim = 1.0f;
                SetLogMsg("Astrometric resonance bioscan completed! Detected precursor signatures. +20 SP.", 0);
                PlaySoundFx(SFX_SUCCESS);
            } else {
                SetLogMsg("Insufficient energy for bioscan (Req: 60 kW).", 1);
            }
            break;
        }
        case BID_XENO_EXCAV_1:
        case BID_XENO_EXCAV_2:
        case BID_XENO_EXCAV_3:
        case BID_XENO_EXCAV_4: {
            int sIdx = bid - BID_XENO_EXCAV_1;
            static const int s_costMin[4] = { 400, 600, 900, 1400 };
            static const int s_costEng[4] = { 250, 350, 500, 800 };
            static const int s_costSci[4] = { 30, 50, 80, 120 };
            static const char* s_names[4] = { "Sunken Monolith", "Biosphere Vault", "Nanite Citadel", "Zero-Point Conduit" };

            int cMin = sim.precursorTech[2] ? (int)(s_costMin[sIdx] * 0.75f) : s_costMin[sIdx];
            int cEng = sim.precursorTech[2] ? (int)(s_costEng[sIdx] * 0.75f) : s_costEng[sIdx];
            int cSci = s_costSci[sIdx];

            if (sim.xenoSiteStatus[sIdx] == 0) {
                if (sim.minerals >= cMin && sim.energy >= cEng && sim.science >= cSci) {
                    sim.minerals -= cMin;
                    sim.energy -= cEng;
                    sim.science -= cSci;
                    sim.xenoSiteStatus[sIdx] = 1; // Excavating
                    sim.xenoSiteProgress[sIdx] = 15;
                    sprintf(buf, "Expedition team deployed to %s! Automated laser excavation begun.", s_names[sIdx]);
                    SetLogMsg(buf, 0);
                    PlaySoundFx(SFX_DEPLOY);
                } else {
                    sprintf(buf, "Insufficient resources (Req: %d Min, %d kW, %d SP).", cMin, cEng, cSci);
                    SetLogMsg(buf, 1);
                }
            }
            break;
        }
        case BID_XENO_FAST_1:
        case BID_XENO_FAST_2:
        case BID_XENO_FAST_3:
        case BID_XENO_FAST_4: {
            int sIdx = bid - BID_XENO_FAST_1;
            int fMin = sim.precursorTech[2] ? 110 : 150;
            int fEng = sim.precursorTech[2] ? 75 : 100;
            if (sim.xenoSiteStatus[sIdx] != 2) {
                if (sim.minerals >= fMin && sim.energy >= fEng) {
                    sim.minerals -= fMin;
                    sim.energy -= fEng;
                    sim.xenoSiteProgress[sIdx] += 25;
                    if (sim.xenoSiteProgress[sIdx] >= 100) {
                        sim.xenoSiteProgress[sIdx] = 100;
                        sim.xenoSiteStatus[sIdx] = 2;
                        sim.xenoRelicFound[sIdx] = 1;
                        sim.science += 250;
                        static const char* s_relics[4] = { "Tachyon Matrix", "Xenobiotic Genome", "Nanite Core", "Zero-Point Resonator" };
                        sprintf(buf, "EXCAVATION SUCCESS! Unearthed [%s]! +250 SP.", s_relics[sIdx]);
                        SetLogMsg(buf, 0);
                        PlaySoundFx(SFX_SUCCESS);
                    } else {
                        sprintf(buf, "Sonic drill advanced excavation progress to %d%%.", sim.xenoSiteProgress[sIdx]);
                        SetLogMsg(buf, 0);
                        PlaySoundFx(SFX_DEPLOY);
                    }
                } else {
                    sprintf(buf, "Insufficient resources for sonic drill (Req: %d Min, %d kW).", fMin, fEng);
                    SetLogMsg(buf, 1);
                }
            }
            break;
        }
        case BID_XENO_TECH_1:
        case BID_XENO_TECH_2:
        case BID_XENO_TECH_3:
        case BID_XENO_TECH_4: {
            int pIdx = bid - BID_XENO_TECH_1;
            static const char* s_techTitles[4] = {
                "Xenobiological Bio-Catalysis",
                "Crystalline Deflector Harmonics",
                "Molecular Nanite Assemblers",
                "Zero-Point Resonant Tap"
            };
            if (!sim.precursorTech[pIdx]) {
                if (!sim.xenoRelicFound[pIdx]) {
                    SetLogMsg("Cannot synthesize: Corresponding ancient precursor relic required.", 1);
                } else if (sim.science >= 150) {
                    sim.science -= 150;
                    sim.precursorTech[pIdx] = 1;
                    if (pIdx == 1) {
                        sim.shieldMaxHP += 500;
                        sim.shieldHP += 500;
                    }
                    CalculateHabitability();
                    sprintf(buf, "PRECURSOR BREAKTHROUGH: [%s] Synthesized!", s_techTitles[pIdx]);
                    SetLogMsg(buf, 0);
                    PlaySoundFx(SFX_SUCCESS);
                } else {
                    SetLogMsg("Insufficient Science Data for precursor synthesis (Req: 150 SP).", 1);
                }
            }
            break;
        }
    }
}

static void DrawTradeRoutesGDI(HDC hdc, int cx, int cy, float z, float simTime) {
    CelestialBody* actP = GetActivePlanet();
    if (!actP) return;
    int sunX = cx + (int)(-300.0f * z);
    int sunY = cy;

    // Endpoints:
    // 0: Tartarus Belt -> Active Planet
    float beltAngle = 2.1f;
    float beltDist = 560.0f * z;
    int beltX = sunX + (int)(cosf(beltAngle) * beltDist);
    int beltY = sunY + (int)(sinf(beltAngle) * (beltDist * 0.7f));

    // Active Planet
    int pX = (int)actP->currX;
    int pY = (int)actP->currY;

    // Boreas and Zephyr
    int mX = pX - (int)(70.0f * z);
    int mY = pY - (int)(50.0f * z);
    int zX = pX + (int)(60.0f * z);
    int zY = pY + (int)(40.0f * z);

    for (int i = 1; i < CURR_SYS.bodyCount; i++) {
        if (strcmp(CURR_SYS.celestials[i].id, "boreas") == 0) {
            mX = (int)CURR_SYS.celestials[i].currX;
            mY = (int)CURR_SYS.celestials[i].currY;
        } else if (strcmp(CURR_SYS.celestials[i].id, "zephyr") == 0) {
            zX = (int)CURR_SYS.celestials[i].currX;
            zY = (int)CURR_SYS.celestials[i].currY;
        }
    }

    // Genesis Ark
    int gX = (int)fleet[0].currX;
    int gY = (int)fleet[0].currY;

    // Helios Corona -> He-3 Depot
    int sX = sunX + (int)(bodies[0].radius * z * 1.3f);
    int sY = sunY;
    int dX = zX + (int)(25.0f * z);
    int dY = zY - (int)(20.0f * z);

    struct { int sx, sy, ex, ey; const char* tag; } routes[4] = {
        { beltX, beltY, pX, pY, "MINERALS" },
        { mX, mY, zX, zY, "VOLATILES" },
        { pX, pY, gX, gY, "FOOD" },
        { sX, sY, dX, dY, "HE-3 FUEL" }
    };

    float fuelSpeedBonus = 1.0f + (float)(sim.fuelDepots - 1) * 0.20f;

    for (int r = 0; r < 4; r++) {
        if (!g_tradeRoutes[r].active) continue;

        if (!sim.paused && sim.speed > 0) {
            g_tradeRoutes[r].progress = fmodf(g_tradeRoutes[r].progress + g_tradeRoutes[r].speed * fuelSpeedBonus * sim.speed, 1.0f);
        }

        int sx = routes[r].sx;
        int sy = routes[r].sy;
        int ex = routes[r].ex;
        int ey = routes[r].ey;

        // Draw dotted conduit lane
        HPEN hLanePen = CreatePen(PS_DOT, 1, g_tradeRoutes[r].color);
        HPEN hOldP = (HPEN)SelectObject(hdc, hLanePen);
        MoveToEx(hdc, sx, sy, NULL);
        LineTo(hdc, ex, ey);
        SelectObject(hdc, hOldP);
        DeleteObject(hLanePen);

        // Draw Convoy Freighters along route
        int count = g_tradeRoutes[r].freighters;
        if (count < 1) count = 1;
        for (int f = 0; f < count; f++) {
            float frac = fmodf(g_tradeRoutes[r].progress + ((float)f / (float)count), 1.0f);
            float tPos;
            int isOutbound;
            if (frac < 0.5f) {
                tPos = frac * 2.0f;
                isOutbound = 1;
            } else {
                tPos = 1.0f - (frac - 0.5f) * 2.0f;
                isOutbound = 0;
            }

            int fx = sx + (int)((ex - sx) * tPos);
            int fy = sy + (int)((ey - sy) * tPos);

            // Mini freighter barge (diamond/triangle)
            int bsz = (int)(4.0f * z);
            if (bsz < 2) bsz = 2;
            HBRUSH hShipBrush = CreateSolidBrush(g_tradeRoutes[r].color);
            HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hShipBrush);
            POINT pts[4];
            pts[0].x = fx; pts[0].y = fy - bsz;
            pts[1].x = fx + bsz; pts[1].y = fy;
            pts[2].x = fx; pts[2].y = fy + bsz;
            pts[3].x = fx - bsz; pts[3].y = fy;
            Polygon(hdc, pts, 4);

            if (isOutbound) {
                // Cargo container box in center
                FillSolidRect(hdc, fx - 1, fy - 1, 3, 3, RGB(255, 255, 255));
            }
            SelectObject(hdc, hOldB);
            DeleteObject(hShipBrush);
        }

        // Midpoint Route Label
        int midX = (sx + ex) / 2;
        int midY = (sy + ey) / 2;
        SetTextColor(hdc, g_tradeRoutes[r].color);
        TextOutA(hdc, midX - 20, midY - 6, routes[r].tag, (int)strlen(routes[r].tag));
    }

    // Draw Zephyr Orbital Docks Gantry Ring
    HPEN hDockPen = CreatePen(PS_SOLID, 1, COLOR_CYAN);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hDockPen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    int dockR = (int)((16 + sim.orbitalDocks * 3) * z);
    Ellipse(hdc, zX - dockR, zY - dockR, zX + dockR, zY + dockR);

    // Cross gantry spokes
    MoveToEx(hdc, zX - dockR, zY, NULL); LineTo(hdc, zX + dockR, zY);
    MoveToEx(hdc, zX, zY - dockR, NULL); LineTo(hdc, zX, zY + dockR);
    SelectObject(hdc, hOldPen);
    DeleteObject(hDockPen);

    // Draw Surface Mass Driver catapult on active planet limb
    if (sim.massDrivers > 0 && actP) {
        float launchAngle = -0.7f;
        int pr = (int)(actP->radius * z);
        int lx = pX + (int)(cosf(launchAngle) * pr);
        int ly = pY + (int)(sinf(launchAngle) * pr);
        int barrelLen = (int)((12 + sim.massDrivers * 4) * z);
        int bx2 = lx + (int)(cosf(launchAngle) * barrelLen);
        int by2 = ly + (int)(sinf(launchAngle) * barrelLen);

        HPEN hDriverPen = CreatePen(PS_SOLID, 2, COLOR_EMERALD);
        HPEN hOldDP = (HPEN)SelectObject(hdc, hDriverPen);
        MoveToEx(hdc, lx, ly, NULL);
        LineTo(hdc, bx2, by2);
        SelectObject(hdc, hOldDP);
        DeleteObject(hDriverPen);

        // Animated launch slug
        float pulseT = fmodf(simTime * 2.0f, 2.0f);
        if (pulseT < 0.6f) {
            float slugDist = barrelLen + (pulseT * 80.0f * z);
            int sx = lx + (int)(cosf(launchAngle) * slugDist);
            int sy = ly + (int)(sinf(launchAngle) * slugDist);
            FillSolidRect(hdc, sx - 2, sy - 2, 5, 5, RGB(160, 255, 200));
        }
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

    CRTTheme* theme = &g_crtThemes[g_crtTheme];

    // 1. Fill Deep Space Background
    FillSolidRect(hdc, 0, 0, width, height, theme->bgDeep);

    // Layout Dimensions
    int headerH = 46;
    int footerH = 34;
    int sidebarW = 380;
    if (sidebarW > width / 2) sidebarW = width / 2;
    int viewportW = width - sidebarW;
    int viewportH = height - headerH - footerH;

    // 2. Draw Top Header Bar
    FillSolidRect(hdc, 0, 0, width, headerH, theme->bgPanelDark);
    FillSolidRect(hdc, 0, headerH - 1, width, 1, theme->border);

    // Brand Logo
    SelectObject(hdc, hFontTitle);
    SetTextColor(hdc, theme->primary);
    TextOutA(hdc, 12, 6, "KCOSMIC", 7);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, theme->textDim);
    TextOutA(hdc, 84, 9, "// Fleet Logistics & Terraforming", 33);

    // Header Badges
    SelectObject(hdc, hFontMain);
    int badgeX = 360;
    if (badgeX > width - 420) badgeX = width - 420;
    if (badgeX < 240) badgeX = 240;

    char buf[128];
    // Cycle
    sprintf(buf, "CYC: %.2f", sim.cycle);
    SetTextColor(hdc, theme->primary);
    TextOutA(hdc, badgeX, 7, buf, (int)strlen(buf));

    // Energy
    sprintf(buf, "ENG: %d kW (%+d)", sim.energy, sim.deltaEnergy);
    SetTextColor(hdc, COLOR_EMERALD);
    TextOutA(hdc, badgeX + 85, 7, buf, (int)strlen(buf));

    // Minerals
    sprintf(buf, "MIN: %d t (%+d)", sim.minerals, sim.deltaMinerals);
    SetTextColor(hdc, COLOR_BLUE);
    TextOutA(hdc, badgeX + 225, 7, buf, (int)strlen(buf));

    // Volatiles
    sprintf(buf, "VOL: %d t (%+d)", sim.volatiles, sim.deltaVolatiles);
    SetTextColor(hdc, COLOR_PURPLE);
    TextOutA(hdc, badgeX + 365, 7, buf, (int)strlen(buf));

    // Science (Phase 10)
    sprintf(buf, "SCI: %d SP (%+d)", sim.science, sim.deltaScience);
    SetTextColor(hdc, RGB(216, 180, 254));
    TextOutA(hdc, badgeX, 24, buf, (int)strlen(buf));

    // Habitability
    sprintf(buf, "HAB: %.1f%%", sim.habitability);
    SetTextColor(hdc, COLOR_AMBER);
    TextOutA(hdc, badgeX + 85, 24, buf, (int)strlen(buf));

    // Colonists
    sprintf(buf, "POP: %d", sim.colonists);
    SetTextColor(hdc, theme->textBright);
    TextOutA(hdc, badgeX + 225, 24, buf, (int)strlen(buf));

    // Food
    sprintf(buf, "FOOD: %d t (%+d)", sim.food, sim.deltaFood);
    SetTextColor(hdc, COLOR_EMERALD);
    TextOutA(hdc, badgeX + 365, 24, buf, (int)strlen(buf));

    // Phase 10: Breakthrough Fanfare Banner
    if (sim.breakthroughFanfare > 0) {
        sim.breakthroughFanfare--;
        int bannerW = 460;
        int bannerH = 58;
        int bx = (width - bannerW) / 2;
        int by = headerH + 18;
        FillSolidRect(hdc, bx, by, bannerW, bannerH, RGB(25, 18, 8));
        FrameSolidRect(hdc, bx, by, bannerW, bannerH, RGB(245, 158, 11));
        FrameSolidRect(hdc, bx + 1, by + 1, bannerW - 2, bannerH - 2, RGB(255, 215, 60));

        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, RGB(245, 158, 11));
        TextOutA(hdc, bx + 14, by + 6, "⭐ SCIENTIFIC BREAKTHROUGH ACHIEVED ⭐", 41);

        SelectObject(hdc, hFontMain);
        SetTextColor(hdc, COLOR_TEXT_BRIGHT);
        TextOutA(hdc, bx + 14, by + 22, sim.lastBreakthrough, (int)strlen(sim.lastBreakthrough));

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_EMERALD);
        TextOutA(hdc, bx + 14, by + 38, "Permanent Interstellar Bonus Engaged Across Fleet & Colonies", 60);
    }

    // 3. Viewport Stellar Canvas (Left Area)
    int shakeX = 0, shakeY = 0;
    if (sim.crisisShake > 0.0f) {
        shakeX = (int)((((float)rand() / (float)RAND_MAX) - 0.5f) * sim.crisisShake);
        shakeY = (int)((((float)rand() / (float)RAND_MAX) - 0.5f) * sim.crisisShake);
        sim.crisisShake *= 0.92f;
        if (sim.crisisShake < 0.1f) sim.crisisShake = 0.0f;
    }
    int cx = viewportW / 2 + (int)sim.camX + shakeX;
    int cy = headerH + viewportH / 2 + (int)sim.camY + shakeY;
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

    // B. Astrometric Stellar Cartography Grid & Range Rings
    if (g_showGrid) {
        HPEN hGridPen = CreatePen(PS_SOLID, 1, theme->grid);
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

        // Concentric AU Range Rings centered on Kepler-186 Helios
        int sunCenterOriginX = cx + (int)(-300.0f * z);
        int sunCenterOriginY = cy;
        HPEN hRingPen = CreatePen(PS_DOT, 1, theme->grid);
        hOldPen = (HPEN)SelectObject(hdc, hRingPen);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, theme->secondary);
        SelectObject(hdc, hFontSmall);

        int auDistances[5] = {100, 200, 300, 360, 500};
        const char* auLabels[5] = {
            "100 AU // INNER TRANSIT",
            "200 AU // TARTARUS BELT",
            "300 AU // STELLAR CORRIDOR",
            "360 AU // HABITABLE ZONE",
            "500 AU // PERIMETER ORBIT"
        };

        for (int ri = 0; ri < 5; ri++) {
            int rad = (int)(auDistances[ri] * z);
            Arc(hdc, sunCenterOriginX - rad, sunCenterOriginY - (int)(rad * 0.7f),
                     sunCenterOriginX + rad, sunCenterOriginY + (int)(rad * 0.7f), 0, 0, 0, 0);

            // Label at -35 degrees
            int lx = sunCenterOriginX + (int)(cosf(-0.55f) * rad) + 4;
            int ly = sunCenterOriginY + (int)(sinf(-0.55f) * rad * 0.7f) - 6;
            if (lx > 10 && lx < viewportW - 140 && ly > headerH + 10 && ly < headerH + viewportH - 20) {
                TextOutA(hdc, lx, ly, auLabels[ri], (int)strlen(auLabels[ri]));
            }
        }

        // Radial azimuth bearing spokes every 45 deg
        for (int deg = 0; deg < 360; deg += 45) {
            float radAngle = (float)deg * 0.0174532925f;
            int rx1 = sunCenterOriginX + (int)(cosf(radAngle) * 60 * z);
            int ry1 = sunCenterOriginY + (int)(sinf(radAngle) * 60 * z * 0.7f);
            int rx2 = sunCenterOriginX + (int)(cosf(radAngle) * 520 * z);
            int ry2 = sunCenterOriginY + (int)(sinf(radAngle) * 520 * z * 0.7f);
            MoveToEx(hdc, rx1, ry1, NULL);
            LineTo(hdc, rx2, ry2);

            int sx2 = sunCenterOriginX + (int)(cosf(radAngle) * 530 * z);
            int sy2 = sunCenterOriginY + (int)(sinf(radAngle) * 530 * z * 0.7f);
            if (sx2 > 15 && sx2 < viewportW - 50 && sy2 > headerH + 15 && sy2 < headerH + viewportH - 35) {
                char degBuf[16];
                sprintf(degBuf, "%03d deg", deg);
                TextOutA(hdc, sx2, sy2, degBuf, (int)strlen(degBuf));
            }
        }
        SelectObject(hdc, hOldPen);
        DeleteObject(hRingPen);

        // Corner Astrometric Registration Marks
        HPEN hCornerPen = CreatePen(PS_SOLID, 2, theme->primary);
        hOldPen = (HPEN)SelectObject(hdc, hCornerPen);
        // Top-left
        MoveToEx(hdc, 12, headerH + 12, NULL); LineTo(hdc, 24, headerH + 12);
        MoveToEx(hdc, 12, headerH + 12, NULL); LineTo(hdc, 12, headerH + 24);
        // Top-right
        MoveToEx(hdc, viewportW - 12, headerH + 12, NULL); LineTo(hdc, viewportW - 24, headerH + 12);
        MoveToEx(hdc, viewportW - 12, headerH + 12, NULL); LineTo(hdc, viewportW - 12, headerH + 24);
        // Bottom-left
        MoveToEx(hdc, 12, headerH + viewportH - 12, NULL); LineTo(hdc, 24, headerH + viewportH - 12);
        MoveToEx(hdc, 12, headerH + viewportH - 12, NULL); LineTo(hdc, 12, headerH + viewportH - 24);
        // Bottom-right
        MoveToEx(hdc, viewportW - 12, headerH + viewportH - 12, NULL); LineTo(hdc, viewportW - 24, headerH + viewportH - 12);
        MoveToEx(hdc, viewportW - 12, headerH + viewportH - 12, NULL); LineTo(hdc, viewportW - 12, headerH + viewportH - 24);
        SelectObject(hdc, hOldPen);
        DeleteObject(hCornerPen);

        // Top-Right Astrometric Telemetry Badge
        int tbW = 270;
        int tbH = 20;
        int tbX = viewportW - tbW - 12;
        int tbY = headerH + 10;
        FillSolidRect(hdc, tbX, tbY, tbW, tbH, theme->bgPanel);
        FrameSolidRect(hdc, tbX, tbY, tbW, tbH, theme->border);
        FillSolidRect(hdc, tbX, tbY, 2, tbH, theme->primary);
        SetTextColor(hdc, theme->primary);
        TextOutA(hdc, tbX + 8, tbY + 3, "RA 19h 54m | DEC +44 01' | ATLAS: ON", 36);
    }

    // C. Central Sun of Current System
    int sunX = cx + (int)(-300.0f * z);
    int sunY = cy;
    int sunR = (int)(bodies[0].radius * z);
    bodies[0].currX = (float)sunX;
    bodies[0].currY = (float)sunY;

    // Outer Glow Rings using System's Star Corona
    HPEN hCoronaPen = CreatePen(PS_SOLID, 1, CURR_SYS.starCorona);
    HPEN hOldP = (HPEN)SelectObject(hdc, hCoronaPen);
    HBRUSH hCoronaBrush = CreateSolidBrush(RGB(
        GetRValue(CURR_SYS.starCorona) / 3,
        GetGValue(CURR_SYS.starCorona) / 3,
        GetBValue(CURR_SYS.starCorona) / 3
    ));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hCoronaBrush);
    Ellipse(hdc, sunX - sunR * 2, sunY - sunR * 2, sunX + sunR * 2, sunY + sunR * 2);
    DeleteObject(hCoronaBrush);

    // Sun Core using System's Star Color
    HBRUSH hSunBrush = CreateSolidBrush(CURR_SYS.starColor);
    SelectObject(hdc, hSunBrush);
    Ellipse(hdc, sunX - sunR, sunY - sunR, sunX + sunR, sunY + sunR);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldP);
    DeleteObject(hSunBrush);
    DeleteObject(hCoronaPen);

    // Sun Label
    SetTextColor(hdc, RGB(255, 200, 140));
    SelectObject(hdc, hFontSmall);
    TextOutA(hdc, sunX - 45, sunY + sunR + 4, bodies[0].name, (int)strlen(bodies[0].name));

    // D. Asteroid Belt around Star
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

    // E. Dynamic Rendering for All Celestial Bodies in Current Star System
    for (int i = 1; i < CURR_SYS.bodyCount; i++) {
        CelestialBody* b = &CURR_SYS.celestials[i];
        if (b->isPlanet) {
            if (!sim.paused && sim.speed > 0) {
                b->angle += b->orbitSpeed * 0.008f * sim.speed;
            }
            float planetOrbitR = b->orbitRadius * z;
            int px = sunX + (int)(cosf(b->angle) * planetOrbitR);
            int py = sunY + (int)(sinf(b->angle) * (planetOrbitR * 0.7f));
            int pr = (int)(b->radius * z);
            b->currX = (float)px;
            b->currY = (float)py;

            // Orbit path ellipse
            HPEN hOrbitPen = CreatePen(PS_DOT, 1, RGB(40, 70, 110));
            HPEN hOldP2 = (HPEN)SelectObject(hdc, hOrbitPen);
            Arc(hdc, sunX - (int)planetOrbitR, sunY - (int)(planetOrbitR * 0.7f),
                     sunX + (int)planetOrbitR, sunY + (int)(planetOrbitR * 0.7f), 0, 0, 0, 0);
            SelectObject(hdc, hOldP2);
            DeleteObject(hOrbitPen);

            // Draw Exoplanet with its procedural classification surface shader
            DrawExoplanetGDI(hdc, b, px, py, pr, sunX, sunY, z, sim.time, (CURR_SYS.activePlanetIndex == i));

            // Selected reticle on planet
            if (sim.selectedType == 2 && sim.selectedIndex == i) {
                HPEN hSelPen = CreatePen(PS_SOLID, 2, COLOR_CYAN);
                HPEN hOldP3 = (HPEN)SelectObject(hdc, hSelPen);
                SelectObject(hdc, GetStockObject(NULL_BRUSH));
                Rectangle(hdc, px - pr - 6, py - pr - 6, px + pr + 6, py + pr + 6);
                SelectObject(hdc, hOldP3);
                DeleteObject(hSelPen);
            }

            // Active Terraforming Target Reticle
            if (CURR_SYS.activePlanetIndex == i) {
                HPEN hTarPen = CreatePen(PS_SOLID, 1, COLOR_EMERALD);
                HPEN hOldP4 = (HPEN)SelectObject(hdc, hTarPen);
                int bk = pr + 8;
                MoveToEx(hdc, px - bk, py - bk + 6, NULL); LineTo(hdc, px - bk, py - bk); LineTo(hdc, px - bk + 6, py - bk);
                MoveToEx(hdc, px + bk, py - bk + 6, NULL); LineTo(hdc, px + bk, py - bk); LineTo(hdc, px + bk - 6, py - bk);
                MoveToEx(hdc, px - bk, py + bk - 6, NULL); LineTo(hdc, px - bk, py + bk); LineTo(hdc, px - bk + 6, py + bk);
                MoveToEx(hdc, px + bk, py + bk - 6, NULL); LineTo(hdc, px + bk, py + bk); LineTo(hdc, px + bk - 6, py + bk);
                SelectObject(hdc, hOldP4);
                DeleteObject(hTarPen);
            }

            // Planet Label
            sprintf(buf, "%s [%s: %.1f%%]", b->name, g_exoplanetClasses[b->pClass].code, b->habitability);
            SetTextColor(hdc, (CURR_SYS.activePlanetIndex == i) ? COLOR_EMERALD : COLOR_BLUE);
            SelectObject(hdc, hFontSmall);
            TextOutA(hdc, px - 45, py + pr + 4, buf, (int)strlen(buf));

        } else if (b->isMoon) {
            int parX = (b->parentIndex >= 0 && b->parentIndex < CURR_SYS.bodyCount) ? (int)CURR_SYS.celestials[b->parentIndex].currX : sunX;
            int parY = (b->parentIndex >= 0 && b->parentIndex < CURR_SYS.bodyCount) ? (int)CURR_SYS.celestials[b->parentIndex].currY : sunY;
            if (!sim.paused && sim.speed > 0) {
                b->angle += b->orbitSpeed * 0.02f * sim.speed;
            }
            float moonDist = b->orbitRadius * z;
            int mx = parX + (int)(cosf(b->angle) * moonDist);
            int my = parY + (int)(sinf(b->angle) * (moonDist * 0.6f));
            int mr = (int)(b->radius * z);
            b->currX = (float)mx;
            b->currY = (float)my;

            DrawMoonGDI(hdc, mx, my, mr, sunX, sunY, z);

            if (sim.selectedType == 3 && sim.selectedIndex == i) {
                HPEN hSelPen = CreatePen(PS_SOLID, 1, COLOR_CYAN);
                HPEN hOldP5 = (HPEN)SelectObject(hdc, hSelPen);
                SelectObject(hdc, GetStockObject(NULL_BRUSH));
                Rectangle(hdc, mx - mr - 4, my - mr - 4, mx + mr + 4, my + mr + 4);
                SelectObject(hdc, hOldP5);
                DeleteObject(hSelPen);
            }
            SetTextColor(hdc, COLOR_TEXT_PRI);
            TextOutA(hdc, mx - 30, my + mr + 2, b->name, (int)strlen(b->name));

        } else if (b->isStation) {
            int parX = (b->parentIndex >= 0 && b->parentIndex < CURR_SYS.bodyCount) ? (int)CURR_SYS.celestials[b->parentIndex].currX : sunX;
            int parY = (b->parentIndex >= 0 && b->parentIndex < CURR_SYS.bodyCount) ? (int)CURR_SYS.celestials[b->parentIndex].currY : sunY;
            if (!sim.paused && sim.speed > 0) {
                b->angle += b->orbitSpeed * 0.02f * sim.speed;
            }
            float stDist = b->orbitRadius * z;
            int stX = parX + (int)(cosf(b->angle) * stDist);
            int stY = parY + (int)(sinf(b->angle) * (stDist * 0.6f));
            int stR = (int)(b->radius * z);
            b->currX = (float)stX;
            b->currY = (float)stY;

            DrawStationGDI(hdc, stX, stY, stR, z, sim.time);
            if (sim.selectedType == 4 && sim.selectedIndex == i) {
                FrameSolidRect(hdc, stX - stR - 3, stY - stR - 3, stR * 2 + 6, stR * 2 + 6, COLOR_CYAN);
            }
            SetTextColor(hdc, COLOR_EMERALD);
            TextOutA(hdc, stX - 25, stY + stR + 2, b->name, (int)strlen(b->name));
        }
    }

    // F. Fleet Ships Anchored to Active Planet
    CelestialBody* actP = GetActivePlanet();
    int actPx = (int)actP->currX;
    int actPy = (int)actP->currY;

    for (int i = 0; i < 5; i++) {
        int sx, sy;
        float fleetSpeedMult = 1.0f;
        if (sim.techResearched[TECH_PROP_ANTIMATTER]) fleetSpeedMult = 3.0f;
        else if (sim.techResearched[TECH_PROP_WARP]) fleetSpeedMult = 2.0f;
        else if (sim.techResearched[TECH_PROP_RAMJETS]) fleetSpeedMult = 1.3f;

        if (fleet[i].targetBelt) {
            if (!sim.paused && sim.speed > 0) {
                fleet[i].x += fleet[i].vx * sim.speed * fleetSpeedMult;
                fleet[i].y += fleet[i].vy * sim.speed * fleetSpeedMult;
                if (fleet[i].x > 350.0f || fleet[i].x < -100.0f) fleet[i].vx *= -1.0f;
                if (fleet[i].y > 200.0f || fleet[i].y < -200.0f) fleet[i].vy *= -1.0f;
            }
            sx = cx + (int)(fleet[i].x * z);
            sy = cy + (int)(fleet[i].y * z);
        } else {
            if (!sim.paused && sim.speed > 0) {
                fleet[i].angle += fleet[i].orbitSpeed * 0.02f * sim.speed * fleetSpeedMult;
            }
            float dist = fleet[i].orbitDist * z;
            sx = actPx + (int)(cosf(fleet[i].angle) * dist);
            sy = actPy + (int)(sinf(fleet[i].angle) * (dist * 0.7f));
        }

        fleet[i].currX = (float)sx;
        fleet[i].currY = (float)sy;

        // Ship Velocity Trajectory Vector
        if (g_showGrid) {
            HPEN hVecPen = CreatePen(PS_DOT, 1, fleet[i].color);
            HPEN hOldVec = (HPEN)SelectObject(hdc, hVecPen);
            int vLen = (int)(22 * z);
            float hAng = fleet[i].targetBelt ? atan2f(fleet[i].vy, fleet[i].vx) : (fleet[i].angle + 1.5707963f);
            int vx2 = sx + (int)(cosf(hAng) * vLen);
            int vy2 = sy + (int)(sinf(hAng) * vLen);
            MoveToEx(hdc, sx, sy, NULL);
            LineTo(hdc, vx2, vy2);
            SelectObject(hdc, hOldVec);
            DeleteObject(hVecPen);
            FillSolidRect(hdc, vx2 - 1, vy2 - 1, 3, 3, fleet[i].color);
        }

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

    // G. Automated Supply Trade Routes & Logistics Infrastructure
    DrawTradeRoutesGDI(hdc, cx, cy, z, sim.time);

    // I. Viewport Top Overlay Card
    int ovW = 340;
    int ovH = 68;
    FillSolidRect(hdc, 10, headerH + 10, ovW, ovH, theme->bgPanel);
    FrameSolidRect(hdc, 10, headerH + 10, ovW, ovH, theme->border);
    FillSolidRect(hdc, 10, headerH + 10, 3, ovH, theme->primary);

    SetTextColor(hdc, theme->primary);
    SelectObject(hdc, hFontSmall);
    sprintf(buf, "SECTOR: %s [%s]", CURR_SYS.name, CURR_SYS.spectralClass);
    TextOutA(hdc, 20, headerH + 15, buf, (int)strlen(buf));

    SetTextColor(hdc, theme->textBright);
    sprintf(buf, "Target: %s [%s: %s]", actP->name, g_exoplanetClasses[actP->pClass].code, g_exoplanetClasses[actP->pClass].name);
    TextOutA(hdc, 20, headerH + 30, buf, (int)strlen(buf));

    SetTextColor(hdc, COLOR_EMERALD);
    sprintf(buf, "Biometrics: %.2fatm | %.1fC | %.1f%% O2 | Hab: %.1f%%",
        sim.pressure, sim.temp, sim.oxygen, sim.habitability);
    TextOutA(hdc, 20, headerH + 46, buf, (int)strlen(buf));

    // Next System & Scan Sector buttons
    AddButton(BID_SYS_CYCLE, 10 + ovW + 8, headerH + 10, 100, 24, "Next Sys [S]", NULL, 1);
    AddButton(BID_SYS_SCAN, 10 + ovW + 8, headerH + 38, 100, 24, "Scan Sector [G]", NULL, 1);

    // Planet Roster Buttons across top
    int rstX = 10 + ovW + 116;
    for (int p = 1; p < CURR_SYS.bodyCount; p++) {
        if (CURR_SYS.celestials[p].isPlanet && rstX < viewportW - 90) {
            char rstTxt[32];
            sprintf(rstTxt, "[%s] %s", g_exoplanetClasses[CURR_SYS.celestials[p].pClass].code, CURR_SYS.celestials[p].name);
            AddButton(BID_ROSTER_BASE + p, rstX, headerH + 10, 95, 24, rstTxt, NULL, 1);
            rstX += 100;
        }
    }

    // CRT Raster Scanlines Post-Effect
    if (g_phosphorGlow) {
        HPEN hScanlinePen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HPEN hOldScanPen = (HPEN)SelectObject(hdc, hScanlinePen);
        for (int syScan = headerH; syScan < headerH + viewportH; syScan += 4) {
            MoveToEx(hdc, 0, syScan, NULL);
            LineTo(hdc, viewportW, syScan);
        }
        SelectObject(hdc, hOldScanPen);
        DeleteObject(hScanlinePen);
    }

    // J. Viewport Navigation Buttons (Bottom-Left of Viewport)
    int navY = headerH + viewportH - 30;
    AddButton(BID_FOCUS_PLANET, 10, navY, 82, 22, "Focus Planet", NULL, 1);
    AddButton(BID_FOCUS_ARK, 96, navY, 74, 22, "Focus Ark", NULL, 1);
    AddButton(BID_ZOOM_IN, 174, navY, 56, 22, "Zoom +", NULL, 1);
    AddButton(BID_ZOOM_OUT, 234, navY, 56, 22, "Zoom -", NULL, 1);
    AddButton(BID_RESET_VIEW, 294, navY, 52, 22, "Reset", NULL, 1);
    AddButton(BID_THEME_TOGGLE, 350, navY, 96, 22, g_crtThemes[g_crtTheme].name, NULL, 1);
    AddButton(BID_GRID_TOGGLE, 450, navY, 68, 22, g_showGrid ? "Grid: ON" : "Grid: OFF", NULL, 1);
    AddButton(BID_GLOW_TOGGLE, 522, navY, 68, 22, g_phosphorGlow ? "Glow: ON" : "Glow: OFF", NULL, 1);

    // K. Viewport Selection Card (if something selected)
    if (sim.selectedType != 0) {
        int scX = viewportW - 225;
        int scY = headerH + 10;
        int scH = (sim.selectedType == 2) ? 134 : 110;
        FillSolidRect(hdc, scX, scY, 215, scH, theme->bgPanel);
        FrameSolidRect(hdc, scX, scY, 215, scH, theme->secondary);

        const char* selName = "Object";
        const char* selType = "Target";
        if (sim.selectedType == 1) { selName = bodies[0].name; selType = bodies[0].type; }
        else if (sim.selectedType == 2 && sim.selectedIndex >= 0 && sim.selectedIndex < CURR_SYS.bodyCount) {
            selName = CURR_SYS.celestials[sim.selectedIndex].name;
            selType = CURR_SYS.celestials[sim.selectedIndex].type;
        }
        else if (sim.selectedType == 3 && sim.selectedIndex >= 0 && sim.selectedIndex < CURR_SYS.bodyCount) {
            selName = CURR_SYS.celestials[sim.selectedIndex].name;
            selType = CURR_SYS.celestials[sim.selectedIndex].type;
        }
        else if (sim.selectedType == 4 && sim.selectedIndex >= 0 && sim.selectedIndex < CURR_SYS.bodyCount) {
            selName = CURR_SYS.celestials[sim.selectedIndex].name;
            selType = CURR_SYS.celestials[sim.selectedIndex].type;
        }
        else if (sim.selectedType == 5 && sim.selectedIndex >= 0) {
            selName = fleet[sim.selectedIndex].name;
            selType = fleet[sim.selectedIndex].role;
        }

        SetTextColor(hdc, theme->primary);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, scX + 8, scY + 6, selName, (int)strlen(selName));

        SetTextColor(hdc, theme->textDim);
        SelectObject(hdc, hFontSmall);
        TextOutA(hdc, scX + 8, scY + 22, selType, (int)strlen(selType));

        AddButton(BID_SEL_CLOSE, scX + 190, scY + 4, 18, 16, "X", NULL, 1);

        if (sim.selectedType == 2) {
            AddButton(BID_TARGET_PLANET, scX + 8, scY + 42, 198, 24, "Designate Target [T]", NULL, 1);
            AddButton(BID_SEL_ACT1, scX + 8, scY + 70, 198, 24, "Inspect Biometrics", NULL, 1);
            AddButton(BID_SEL_ACT2, scX + 8, scY + 98, 198, 24, "Solar Mirror Orbit", NULL, 1);
        } else {
            AddButton(BID_SEL_ACT1, scX + 8, scY + 48, 198, 24, "Inspect Telemetry", NULL, 1);
            AddButton(BID_SEL_ACT2, scX + 8, scY + 76, 198, 24, "Reposition Orbit", NULL, 1);
        }
    }

    // Phase 9: Crisis Viewport Overlays & Top Alert Card
    if (sim.crisisActive && sim.crisisType > CRISIS_NONE) {
        CelestialBody* p = GetActivePlanet();
        int px = cx + (int)(p->currX - cx);
        int py = cy + (int)(p->currY - cy);
        float progress = 1.0f - (sim.crisisTimer / sim.crisisMaxTime);
        HPEN hAlertPen, hOldP;
        HBRUSH hOldB;
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;

        // Viewport Perimeter Alert Border
        hAlertPen = CreatePen(PS_SOLID, 4, COLOR_ROSE);
        hOldP = (HPEN)SelectObject(hdc, hAlertPen);
        hOldB = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, 2, headerH + 2, viewportW - 2, headerH + viewportH - 2);
        SelectObject(hdc, hOldB);
        SelectObject(hdc, hOldP);
        DeleteObject(hAlertPen);

        if (sim.crisisType == CRISIS_FLARE) {
            int sx = cx + (int)(-300.0f * z);
            int sy = cy;
            int dist = (int)hypot(px - sx, py - sy);
            int waveR = (int)(dist * (progress * 1.1f));
            HPEN hWavePen = CreatePen(PS_SOLID, 3, COLOR_ORANGE);
            hOldP = (HPEN)SelectObject(hdc, hWavePen);
            hOldB = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Ellipse(hdc, sx - waveR, sy - waveR, sx + waveR, sy + waveR);
            SelectObject(hdc, hOldB);
            SelectObject(hdc, hOldP);
            DeleteObject(hWavePen);
        } else if (sim.crisisType == CRISIS_ASTEROID) {
            int startDist = (int)(320.0f * z);
            int curDist = (int)(startDist * (1.0f - progress));
            int ax = px - curDist;
            int ay = py - curDist;
            HPEN hAstPen = CreatePen(PS_DOT, 1, COLOR_ROSE);
            hOldP = (HPEN)SelectObject(hdc, hAstPen);
            MoveToEx(hdc, px - startDist, py - startDist, NULL);
            LineTo(hdc, px, py);
            SelectObject(hdc, hOldP);
            DeleteObject(hAstPen);

            FillSolidRect(hdc, ax - 4, ay - 4, 8, 8, COLOR_ROSE);
            FrameSolidRect(hdc, px - 20, py - 20, 40, 40, COLOR_ROSE);
        } else if (sim.crisisType == CRISIS_QUAKE) {
            int qR = (int)(p->radius * z + 12.0f);
            HPEN hQPen = CreatePen(PS_SOLID, 2, COLOR_ORANGE);
            hOldP = (HPEN)SelectObject(hdc, hQPen);
            hOldB = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Ellipse(hdc, px - qR, py - qR, px + qR, py + qR);
            SelectObject(hdc, hOldB);
            SelectObject(hdc, hOldP);
            DeleteObject(hQPen);
        } else if (sim.crisisType == CRISIS_BLIGHT) {
            int bR = (int)(p->radius * z + 16.0f);
            HPEN hBPen = CreatePen(PS_SOLID, 2, COLOR_EMERALD);
            hOldP = (HPEN)SelectObject(hdc, hBPen);
            hOldB = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Ellipse(hdc, px - bR, py - bR, px + bR, py + bR);
            SelectObject(hdc, hOldB);
            SelectObject(hdc, hOldP);
            DeleteObject(hBPen);
        } else if (sim.crisisType == CRISIS_STORM) {
            int sR = (int)(p->radius * z + 14.0f);
            HPEN hSPen = CreatePen(PS_SOLID, 2, COLOR_PURPLE);
            hOldP = (HPEN)SelectObject(hdc, hSPen);
            hOldB = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Ellipse(hdc, px - sR, py - sR, px + sR, py + sR);
            SelectObject(hdc, hOldB);
            SelectObject(hdc, hOldP);
            DeleteObject(hSPen);
        }

        // Viewport Top Crisis Banner
        {
            int banW = 420;
            int banH = 36;
            int banX = (viewportW - banW) / 2;
            int banY = headerH + 12;
            const char* cNames[] = {"None", "CORONAL MASS EJECTION", "ASTEROID IMPACT", "TECTONIC QUAKE", "XENO-BLIGHT", "IONIC STORM"};
            FillSolidRect(hdc, banX, banY, banW, banH, RGB(45, 10, 20));
            FrameSolidRect(hdc, banX, banY, banW, banH, COLOR_ROSE);

            SelectObject(hdc, hFontBold);
            SetTextColor(hdc, COLOR_ROSE);
            sprintf(buf, "[CRISIS ALERT] %s -- %.1fs", cNames[sim.crisisType], (sim.crisisTimer > 0.0f ? sim.crisisTimer : 0.0f));
            TextOutA(hdc, banX + 12, banY + 9, buf, (int)strlen(buf));

            AddButton(BID_CRISIS_QUICK, banX + banW - 105, banY + 6, 95, 24, "MITIGATE", NULL, 1);
        }
    }

    SelectClipRgn(hdc, NULL);
    DeleteObject(hRgnViewport);

    // 4. Right Sidebar Area
    int sbX = viewportW;
    FillSolidRect(hdc, sbX, headerH, sidebarW, viewportH, theme->bgPanel);
    FillSolidRect(hdc, sbX, headerH, 1, viewportH, theme->border);

    // Tab Header: 8 Tabs
    int tabW = sidebarW / 8;
    AddButton(BID_TAB_TERRA, sbX, headerH, tabW, 28, "TERRA", NULL, 1);
    AddButton(BID_TAB_FLEET, sbX + tabW, headerH, tabW, 28, "FLEET", NULL, 1);
    AddButton(BID_TAB_COLONY, sbX + tabW * 2, headerH, tabW, 28, "COLONY", NULL, 1);
    AddButton(BID_TAB_RESEARCH, sbX + tabW * 3, headerH, tabW, 28, "TECH", NULL, 1);
    AddButton(BID_TAB_MEGAS, sbX + tabW * 4, headerH, tabW, 28, "MEGAS", NULL, 1);
    AddButton(BID_TAB_XENO, sbX + tabW * 5, headerH, tabW, 28, "XENO", NULL, 1);
    AddButton(BID_TAB_HAZARDS, sbX + tabW * 6, headerH, tabW, 28, "HAZARD", NULL, 1);
    AddButton(BID_TAB_ECONOMY, sbX + tabW * 7, headerH, sidebarW - tabW * 7, 28, "ECON", NULL, 1);

    int contentY = headerH + 34;

    // TAB 0: TERRAFORM
    if (sim.activeTab == 0) {
        CelestialBody* curP = GetActivePlanet();
        ExoplanetClassInfo* curInfo = &g_exoplanetClasses[curP->pClass];

        // Classification Dossier Card
        int dosH = 50;
        FillSolidRect(hdc, sbX + 12, contentY, sidebarW - 24, dosH, curInfo->badgeBg);
        FrameSolidRect(hdc, sbX + 12, contentY, sidebarW - 24, dosH, curInfo->badgeBorder);
        FillSolidRect(hdc, sbX + 12, contentY, 4, dosH, curInfo->color);

        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, curInfo->color);
        sprintf(buf, "[%s] %s -- %s", curInfo->code, curInfo->name, curP->name);
        TextOutA(hdc, sbX + 22, contentY + 6, buf, (int)strlen(buf));

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_TEXT_PRI);
        TextOutA(hdc, sbX + 22, contentY + 20, curInfo->desc, (int)strlen(curInfo->desc));

        SetTextColor(hdc, theme->secondary);
        sprintf(buf, "Yields: Min x%.1f | Vol x%.1f | Range: %.0f%%-%.0f%%",
            curInfo->mineralMult, curInfo->volatileMult, curInfo->minHabitability, curInfo->maxHabitability);
        TextOutA(hdc, sbX + 22, contentY + 34, buf, (int)strlen(buf));

        contentY += dosH + 8;

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
        int my = contentY + 18;
        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_TEXT_PRI);
        TextOutA(hdc, sbX + 12, my, "Atmospheric Pressure", 20);
        sprintf(buf, "%.2f atm", sim.pressure);
        SetTextColor(hdc, COLOR_CYAN);
        TextOutA(hdc, sbX + sidebarW - 75, my, buf, (int)strlen(buf));
        DrawProgressBar(hdc, sbX + 12, my + 13, sidebarW - 24, 5, sim.pressure / 1.5f, COLOR_CYAN);

        // Metric 2: Temperature
        my += 20;
        SetTextColor(hdc, COLOR_TEXT_PRI);
        TextOutA(hdc, sbX + 12, my, "Equilibrium Surface Temp", 24);
        sprintf(buf, "%.1f C", sim.temp);
        SetTextColor(hdc, COLOR_AMBER);
        TextOutA(hdc, sbX + sidebarW - 75, my, buf, (int)strlen(buf));
        DrawProgressBar(hdc, sbX + 12, my + 13, sidebarW - 24, 5, (sim.temp + 60.0f) / 100.0f, COLOR_AMBER);

        // Metric 3: Water
        my += 20;
        SetTextColor(hdc, COLOR_TEXT_PRI);
        TextOutA(hdc, sbX + 12, my, "Hydrosphere / Liquid Water", 26);
        sprintf(buf, "%.1f%%", sim.water);
        SetTextColor(hdc, COLOR_BLUE);
        TextOutA(hdc, sbX + sidebarW - 75, my, buf, (int)strlen(buf));
        DrawProgressBar(hdc, sbX + 12, my + 13, sidebarW - 24, 5, sim.water / 100.0f, COLOR_BLUE);

        // Metric 4: Oxygen
        my += 20;
        SetTextColor(hdc, COLOR_TEXT_PRI);
        TextOutA(hdc, sbX + 12, my, "Oxygen Concentration (O2)", 25);
        sprintf(buf, "%.1f%%", sim.oxygen);
        SetTextColor(hdc, COLOR_EMERALD);
        TextOutA(hdc, sbX + sidebarW - 75, my, buf, (int)strlen(buf));
        DrawProgressBar(hdc, sbX + 12, my + 13, sidebarW - 24, 5, sim.oxygen / 21.0f, COLOR_EMERALD);

        // Metric 5: Nitrogen Buffer
        my += 20;
        SetTextColor(hdc, COLOR_TEXT_PRI);
        TextOutA(hdc, sbX + 12, my, "Nitrogen Buffer (N2)", 20);
        sprintf(buf, "%.1f%%", sim.nitrogen);
        SetTextColor(hdc, RGB(180, 100, 255));
        TextOutA(hdc, sbX + sidebarW - 75, my, buf, (int)strlen(buf));
        DrawProgressBar(hdc, sbX + 12, my + 13, sidebarW - 24, 5, sim.nitrogen / 78.0f, RGB(180, 100, 255));

        // Metric 6: Greenhouse Factor
        my += 20;
        SetTextColor(hdc, COLOR_TEXT_PRI);
        TextOutA(hdc, sbX + 12, my, "Greenhouse Factor (GHG)", 23);
        sprintf(buf, "%.1f%%", sim.greenhouse);
        SetTextColor(hdc, RGB(255, 150, 50));
        TextOutA(hdc, sbX + sidebarW - 75, my, buf, (int)strlen(buf));
        DrawProgressBar(hdc, sbX + 12, my + 13, sidebarW - 24, 5, sim.greenhouse / 150.0f, RGB(255, 150, 50));

        // Metric 7: Magnetosphere
        my += 20;
        SetTextColor(hdc, COLOR_TEXT_PRI);
        TextOutA(hdc, sbX + 12, my, "Magnetosphere / Shielding", 25);
        sprintf(buf, "%.2f Gauss", sim.magnet);
        SetTextColor(hdc, COLOR_PURPLE);
        TextOutA(hdc, sbX + sidebarW - 85, my, buf, (int)strlen(buf));
        DrawProgressBar(hdc, sbX + 12, my + 13, sidebarW - 24, 5, sim.magnet / 0.6f, COLOR_PURPLE);

        // Interventions Section
        my += 24;
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, my, "ORBITAL & SURFACE INTERVENTIONS", 31);

        int gridY = my + 16;
        int btnW = (sidebarW - 30) / 2;
        int btnH = 34;
        int btnStep = btnH + 4;

        // Row 0: Solar Mirror Deploy & Mode Toggle
        AddButton(BID_ACT_SOLAR_MIRROR, sbX + 12, gridY, btnW, btnH,
                  "Solar Mirror Array", (sim.mirrorMode == 0) ? "+1.8C (350E, 150M)" : "-1.8C (350E, 150M)", 1);
        AddButton(BID_TOG_MIRROR_MODE, sbX + 18 + btnW, gridY, btnW, btnH,
                  (sim.mirrorMode == 0) ? "Mirror: [FOCUS/HEAT]" : "Mirror: [SHADE/COOL]", "Toggle Solar Mode", 1);

        // Row 1: Troposphere Processor Deploy & Mode Toggle
        AddButton(BID_ACT_ATMO_PROC, sbX + 12, gridY + btnStep, btnW, btnH,
                  "Troposphere Proc", (sim.atmoMode == 0) ? "+0.05atm (180M, 100E)" : "-0.05atm (180M, 100E)", 1);
        AddButton(BID_TOG_ATMO_MODE, sbX + 18 + btnW, gridY + btnStep, btnW, btnH,
                  (sim.atmoMode == 0) ? "Atmo: [INJECT +P]" : "Atmo: [SCRUB -P]", "Toggle Buffer Mode", 1);

        // Row 2: Nitrogen Extractor & Greenhouse Seeding
        AddButton(BID_ACT_N2_EXTRACTOR, sbX + 12, gridY + btnStep * 2, btnW, btnH,
                  "Nitrogen Extractor", "+0.6% N2 (220M, 140E)", 1);
        AddButton(BID_ACT_GREENHOUSE, sbX + 18 + btnW, gridY + btnStep * 2, btnW, btnH,
                  "Greenhouse Seeding", (sim.greenhouseMode == 0) ? "+1.2% GHG (160V, 120E)" : "-1.2% GHG (160V, 120E)", 1);

        // Row 3: Greenhouse Mode & Ice Comet Drop
        AddButton(BID_TOG_GHG_MODE, sbX + 12, gridY + btnStep * 3, btnW, btnH,
                  (sim.greenhouseMode == 0) ? "GHG: [WARMING +]" : "GHG: [COOLING -]", "Toggle Gas Formulation", 1);
        AddButton(BID_ACT_COMET_DROP, sbX + 18 + btnW, gridY + btnStep * 3, btnW, btnH,
                  "Redirect Ice Comet", "+2.5% Water (300V)", 1);

        // Row 4: Lichen Bioseeding & Algae Seeding
        AddButton(BID_ACT_BIOSEED, sbX + 12, gridY + btnStep * 4, btnW, btnH,
                  "Extremophile Lichen", "+0.9% O2 (120F, 100E)", 1);
        AddButton(BID_ACT_ALGAE, sbX + 18 + btnW, gridY + btnStep * 4, btnW, btnH,
                  "Ocean Algae Seed", "+1.4% O2 (200V, 150F)", 1);

        // Row 5: Core Magnetic Dynamo Ring
        AddButton(BID_ACT_CORE_DYNAMO, sbX + 12, gridY + btnStep * 5, sidebarW - 24, btnH,
                  "Core Magnetic Dynamo Ring", "+0.08G Magnetosphere (400M, 250E)", 1);

        // Continuous Facilities Online Card
        int facY = gridY + btnStep * 6 + 6;
        SetTextColor(hdc, COLOR_BLUE);
        TextOutA(hdc, sbX + 12, facY, "CONTINUOUS FACILITIES ONLINE", 28);

        FillSolidRect(hdc, sbX + 12, facY + 16, sidebarW - 24, 62, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, facY + 16, sidebarW - 24, 62, COLOR_BORDER);

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_TEXT_PRI);
        sprintf(buf, "Mirrors: %d [%s]  |  Atmo Proc: %d [%s]",
                sim.solarMirrors, (sim.mirrorMode == 0) ? "Focus" : "Shade",
                sim.atmoProcessors, (sim.atmoMode == 0) ? "Inject" : "Scrub");
        TextOutA(hdc, sbX + 18, facY + 22, buf, (int)strlen(buf));

        sprintf(buf, "N2 Extractors: %d  |  Greenhouse: %d [%s]",
                sim.nitrogenExtractors, sim.greenhouseStations, (sim.greenhouseMode == 0) ? "Warm" : "Cool");
        TextOutA(hdc, sbX + 18, facY + 38, buf, (int)strlen(buf));

        sprintf(buf, "Bioseed: %d  |  Dynamos: %d Primed",
                sim.bioseedStations, sim.coreDynamos);
        TextOutA(hdc, sbX + 18, facY + 54, buf, (int)strlen(buf));
    }
    // TAB 1: FLEET
    else if (sim.activeTab == 1) {
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, contentY, "ARK FLEET ROSTER (5 ACTIVE)", 27);

        int sy = contentY + 18;
        int shipCardH = 40;

        for (int i = 0; i < 5; i++) {
            FillSolidRect(hdc, sbX + 12, sy, sidebarW - 24, shipCardH, COLOR_BG_CARD);
            FrameSolidRect(hdc, sbX + 12, sy, sidebarW - 24, shipCardH, COLOR_BORDER);

            SetTextColor(hdc, fleet[i].color);
            SelectObject(hdc, hFontBold);
            TextOutA(hdc, sbX + 18, sy + 4, fleet[i].name, (int)strlen(fleet[i].name));

            SetTextColor(hdc, COLOR_TEXT_DIM);
            SelectObject(hdc, hFontSmall);
            sprintf(buf, "%s | Hull: %d%% | %s", fleet[i].status, fleet[i].hull, fleet[i].role);
            TextOutA(hdc, sbX + 18, sy + 20, buf, (int)strlen(buf));

            if (i == 0) {
                AddButton(BID_ORDER_GEN_HOLD, sbX + sidebarW - 130, sy + 5, 52, 18, "Hold", NULL, 1);
                AddButton(BID_ORDER_GEN_BOOST, sbX + sidebarW - 74, sy + 5, 52, 18, "Boost", NULL, 1);
            } else if (i == 1) {
                AddButton(BID_ORDER_VAN_RATION, sbX + sidebarW - 130, sy + 5, 52, 18, "Yield", NULL, 1);
                AddButton(BID_ORDER_VAN_ORBIT, sbX + sidebarW - 74, sy + 5, 52, 18, "Orbit", NULL, 1);
            } else if (i == 2) {
                AddButton(BID_ORDER_AEO_SCAN, sbX + sidebarW - 74, sy + 5, 52, 18, "Scan", NULL, 1);
            } else if (i == 3) {
                AddButton(BID_ORDER_DRA_MINE, sbX + sidebarW - 130, sy + 5, 52, 18, "Mine", NULL, 1);
                AddButton(BID_ORDER_DRA_SCOOP, sbX + sidebarW - 74, sy + 5, 52, 18, "Scoop", NULL, 1);
            } else if (i == 4) {
                AddButton(BID_ORDER_TIT_LOOP, sbX + sidebarW - 130, sy + 5, 52, 18, "Route", NULL, 1);
                AddButton(BID_ORDER_TIT_HOLD, sbX + sidebarW - 74, sy + 5, 52, 18, "Hold", NULL, 1);
            }

            sy += shipCardH + 4;
        }

        // Section: Automated Supply Trade Routes
        sy += 4;
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        int activeRoutes = 0;
        for (int r = 0; r < 4; r++) if (g_tradeRoutes[r].active) activeRoutes++;
        sprintf(buf, "AUTOMATED SUPPLY TRADE ROUTES (%d ACTIVE)", activeRoutes);
        TextOutA(hdc, sbX + 12, sy, buf, (int)strlen(buf));

        sy += 16;
        int rCardH = 46;
        for (int r = 0; r < 4; r++) {
            TradeRoute* tr = &g_tradeRoutes[r];
            FillSolidRect(hdc, sbX + 12, sy, sidebarW - 24, rCardH, COLOR_BG_CARD);
            FrameSolidRect(hdc, sbX + 12, sy, sidebarW - 24, rCardH, COLOR_BORDER);
            FillSolidRect(hdc, sbX + 12, sy, 3, rCardH, tr->color);

            SelectObject(hdc, hFontBold);
            SetTextColor(hdc, tr->color);
            TextOutA(hdc, sbX + 18, sy + 4, tr->name, (int)strlen(tr->name));

            SelectObject(hdc, hFontSmall);
            SetTextColor(hdc, tr->active ? COLOR_EMERALD : COLOR_TEXT_DIM);
            const char* stBadge = tr->active ? "ACTIVE" : "SUSPENDED";
            TextOutA(hdc, sbX + sidebarW - 80, sy + 4, stBadge, (int)strlen(stBadge));

            SetTextColor(hdc, COLOR_TEXT_PRI);
            sprintf(buf, "%s -> %s | Yield: +%d %s",
                    tr->from, tr->to, tr->baseYield * tr->freighters, tr->cargo);
            TextOutA(hdc, sbX + 18, sy + 18, buf, (int)strlen(buf));

            // Controls
            AddButton(BID_ROUTE_TOG_0 + r, sbX + sidebarW - 162, sy + 24, 68, 18,
                      tr->active ? "Suspend" : "Resume", NULL, 1);

            char addTxt[32];
            sprintf(addTxt, "+ Hauler (%dM)", tr->costMin);
            AddButton(BID_ROUTE_ADD_0 + r, sbX + sidebarW - 90, sy + 24, 78, 18,
                      addTxt, NULL, 1);

            sy += rCardH + 4;
        }

        // Section: Orbital Docks & Logistics Depots
        sy += 4;
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, sy, "ORBITAL DOCKS & LOGISTICS DEPOTS", 32);

        sy += 16;
        int infraH = 34;

        // Infra 1: Orbital Docks
        FillSolidRect(hdc, sbX + 12, sy, sidebarW - 24, infraH, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, sy, sidebarW - 24, infraH, COLOR_BORDER);
        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, COLOR_CYAN);
        sprintf(buf, "Zephyr Docks (Tier %d)", sim.orbitalDocks);
        TextOutA(hdc, sbX + 18, sy + 3, buf, (int)strlen(buf));
        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_TEXT_DIM);
        sprintf(buf, "+%d%% Throughput Multiplier", sim.orbitalDocks * 20);
        TextOutA(hdc, sbX + 18, sy + 17, buf, (int)strlen(buf));
        int dockCostMin = 260 + (sim.orbitalDocks - 1) * 80;
        char upgDockTxt[32];
        sprintf(upgDockTxt, "+ Upg (%dM)", dockCostMin);
        AddButton(BID_UPG_DOCKS, sbX + sidebarW - 90, sy + 6, 78, 22, upgDockTxt, NULL, 1);
        sy += infraH + 4;

        // Infra 2: Fuel Depot
        FillSolidRect(hdc, sbX + 12, sy, sidebarW - 24, infraH, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, sy, sidebarW - 24, infraH, COLOR_BORDER);
        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, COLOR_AMBER);
        sprintf(buf, "He-3 Fuel Depot (Tier %d)", sim.fuelDepots);
        TextOutA(hdc, sbX + 18, sy + 3, buf, (int)strlen(buf));
        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_TEXT_DIM);
        sprintf(buf, "+%d kW Power | +%d%% Speed", sim.fuelDepots * 40, sim.fuelDepots * 25);
        TextOutA(hdc, sbX + 18, sy + 17, buf, (int)strlen(buf));
        int fuelCostMin = 200 + (sim.fuelDepots - 1) * 70;
        char upgFuelTxt[32];
        sprintf(upgFuelTxt, "+ Upg (%dM)", fuelCostMin);
        AddButton(BID_UPG_FUEL, sbX + sidebarW - 90, sy + 6, 78, 22, upgFuelTxt, NULL, 1);
        sy += infraH + 4;

        // Infra 3: Mass Driver
        FillSolidRect(hdc, sbX + 12, sy, sidebarW - 24, infraH, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, sy, sidebarW - 24, infraH, COLOR_BORDER);
        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, COLOR_EMERALD);
        sprintf(buf, "Surface Mass Driver (Tier %d)", sim.massDrivers);
        TextOutA(hdc, sbX + 18, sy + 3, buf, (int)strlen(buf));
        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_TEXT_DIM);
        sprintf(buf, "+%d t/cyc Catapult", sim.massDrivers * 12);
        TextOutA(hdc, sbX + 18, sy + 17, buf, (int)strlen(buf));
        int massCostMin = 300 + (sim.massDrivers - 1) * 90;
        char upgMassTxt[32];
        sprintf(upgMassTxt, "+ Upg (%dM)", massCostMin);
        AddButton(BID_UPG_MASS, sbX + sidebarW - 90, sy + 6, 78, 22, upgMassTxt, NULL, 1);
    }
    // TAB 2: COLONY (PHASE 8 DEMOGRAPHICS, MORALE, HOUSING & FOOD FARMS)
    else if (sim.activeTab == 2) {
        int cy = contentY;

        // 1. Colony Overview & Morale
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, cy, "ACTIVE SURFACE COLONY & MORALE", 30);

        int card1H = 74;
        FillSolidRect(hdc, sbX + 12, cy + 16, sidebarW - 24, card1H, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, cy + 16, sidebarW - 24, card1H, COLOR_BORDER);

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_TEXT_PRI);
        float occPct = (sim.housingCap > 0) ? (((float)sim.colonists / (float)sim.housingCap) * 100.0f) : 100.0f;
        sprintf(buf, "Active Population: %d / %d Housing (%.0f%%)", sim.colonists, sim.housingCap, occPct);
        TextOutA(hdc, sbX + 20, cy + 22, buf, (int)strlen(buf));
        DrawProgressBar(hdc, sbX + 20, cy + 36, sidebarW - 40, 6, (float)sim.colonists / (float)sim.housingCap, COLOR_EMERALD);

        // Growth / Famine
        if (sim.food <= 0) {
            SetTextColor(hdc, COLOR_ROSE);
            sprintf(buf, "FAMINE ALERT! Colonists starving (-0.3%%/cyc) | Cryo: %d", sim.cryoSleepers);
        } else {
            SetTextColor(hdc, COLOR_EMERALD);
            float rGrowth = (sim.rationPolicy == 0) ? 0.5f : (sim.rationPolicy == 2 ? 2.0f : 1.0f);
            int estGrowth = (int)(sim.colonists * (sim.morale - 40.0f) * 0.00004f * rGrowth);
            if (estGrowth < 0) estGrowth = 0;
            sprintf(buf, "Net Growth: +%d/cyc | Ark Cryo-Sleepers: %d", estGrowth, sim.cryoSleepers);
        }
        TextOutA(hdc, sbX + 20, cy + 47, buf, (int)strlen(buf));

        // Morale status
        const char* mTag = "Content";
        COLORREF mCol = COLOR_BLUE;
        if (sim.morale >= 88.0f) { mTag = "Euphoric"; mCol = COLOR_EMERALD; }
        else if (sim.morale >= 70.0f) { mTag = "Optimistic"; mCol = COLOR_CYAN; }
        else if (sim.morale >= 50.0f) { mTag = "Content"; mCol = COLOR_BLUE; }
        else if (sim.morale >= 30.0f) { mTag = "Discontent"; mCol = COLOR_AMBER; }
        else { mTag = "Despondent"; mCol = COLOR_ROSE; }

        SetTextColor(hdc, mCol);
        sprintf(buf, "Morale: %.0f%% [%s] | Rations:%s, Megacities:+%d%%",
            sim.morale, mTag,
            sim.rationPolicy == 0 ? "Spartan" : (sim.rationPolicy == 2 ? "Abundant" : "Standard"),
            sim.domedMegacities * 6);
        TextOutA(hdc, sbX + 20, cy + 61, buf, (int)strlen(buf));

        // 2. Workforce Demographics
        cy += card1H + 22;
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        const char* fTitle = sim.demoFocus == 1 ? "AGRONOMY" : (sim.demoFocus == 2 ? "GEO-ENG" : (sim.demoFocus == 3 ? "SCIENCE" : "BALANCED"));
        sprintf(buf, "WORKFORCE DEMOGRAPHICS [%s ROSTER]", fTitle);
        TextOutA(hdc, sbX + 12, cy, buf, (int)strlen(buf));

        int demoCardH = 68;
        FillSolidRect(hdc, sbX + 12, cy + 16, sidebarW - 24, demoCardH, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, cy + 16, sidebarW - 24, demoCardH, COLOR_BORDER);

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_AMBER);
        sprintf(buf, "Pioneers/Laborers (%d%%): %d Pop (+%d t Min/cyc)",
            sim.pctLaborers, (int)(sim.colonists * sim.pctLaborers / 100), (int)(sim.colonists * (sim.pctLaborers / 100.0f) * 0.0008f));
        TextOutA(hdc, sbX + 20, cy + 22, buf, (int)strlen(buf));

        SetTextColor(hdc, COLOR_EMERALD);
        sprintf(buf, "Agronomists (%d%%): %d Pop (+%d%% Crop Yield)",
            sim.pctAgronomists, (int)(sim.colonists * sim.pctAgronomists / 100), (int)((sim.pctAgronomists - 25) * 1.2f));
        TextOutA(hdc, sbX + 20, cy + 34, buf, (int)strlen(buf));

        SetTextColor(hdc, COLOR_CYAN);
        sprintf(buf, "Geo-Engineers (%d%%): %d Pop (-%d%% Energy Drain)",
            sim.pctEngineers, (int)(sim.colonists * sim.pctEngineers / 100), (int)(sim.pctEngineers * 0.25f));
        TextOutA(hdc, sbX + 20, cy + 46, buf, (int)strlen(buf));

        SetTextColor(hdc, COLOR_PURPLE);
        sprintf(buf, "Xenoscientists (%d%%): %d Pop (+Telemetry & Morale Buffer)",
            sim.pctScientists, (int)(sim.colonists * sim.pctScientists / 100));
        TextOutA(hdc, sbX + 20, cy + 58, buf, (int)strlen(buf));

        // 4 Focus Presets
        cy += demoCardH + 18;
        int fBtnW = (sidebarW - 36) / 4;
        AddButton(BID_COL_FOCUS_0, sbX + 12 + 0 * (fBtnW + 4), cy, fBtnW, 18, sim.demoFocus == 0 ? "[Balanced]" : "Balanced", NULL, 1);
        AddButton(BID_COL_FOCUS_1, sbX + 12 + 1 * (fBtnW + 4), cy, fBtnW, 18, sim.demoFocus == 1 ? "[Agronomy]" : "Agronomy", NULL, 1);
        AddButton(BID_COL_FOCUS_2, sbX + 12 + 2 * (fBtnW + 4), cy, fBtnW, 18, sim.demoFocus == 2 ? "[Geo-Eng]" : "Geo-Eng", NULL, 1);
        AddButton(BID_COL_FOCUS_3, sbX + 12 + 3 * (fBtnW + 4), cy, fBtnW, 18, sim.demoFocus == 3 ? "[Science]" : "Science", NULL, 1);

        // 3. Food Reserves & Rationing
        cy += 24;
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        sprintf(buf, "FOOD SUPPLY: %d t (%+d/cyc)", sim.food, sim.deltaFood);
        TextOutA(hdc, sbX + 12, cy, buf, (int)strlen(buf));

        cy += 16;
        int rBtnW = (sidebarW - 32) / 3;
        AddButton(BID_COL_RATION_0, sbX + 12 + 0 * (rBtnW + 4), cy, rBtnW, 18, sim.rationPolicy == 0 ? "[Spartan 0.6x]" : "Spartan 0.6x", NULL, 1);
        AddButton(BID_COL_RATION_1, sbX + 12 + 1 * (rBtnW + 4), cy, rBtnW, 18, sim.rationPolicy == 1 ? "[Standard 1.0x]" : "Standard 1.0x", NULL, 1);
        AddButton(BID_COL_RATION_2, sbX + 12 + 2 * (rBtnW + 4), cy, rBtnW, 18, sim.rationPolicy == 2 ? "[Abundant 1.5x]" : "Abundant 1.5x", NULL, 1);

        // 4. Housing Habitats & Megacities
        cy += 24;
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        sprintf(buf, "HABITATS: %d Domes | %d Vaults | %d Megacities",
            sim.geodesicDomes, sim.subterraneanVaults, sim.domedMegacities);
        TextOutA(hdc, sbX + 12, cy, buf, (int)strlen(buf));

        cy += 16;
        int btnW = (sidebarW - 30) / 2;
        int btnH = 28;
        AddButton(BID_COL_AWAKEN, sbX + 12, cy, btnW, btnH, "Awaken 2.5k Sleepers", "+2.5k Pop (-100 Food)", 1);
        AddButton(BID_COL_DOME, sbX + 18 + btnW, cy, btnW, btnH, "+ Geodesic Dome", "+15k Housing (450M, 150E)", 1);

        cy += btnH + 4;
        AddButton(BID_COL_LAVATUBES, sbX + 12, cy, btnW, btnH, "+ Subterranean Vaults", "+30k Shielded (650M, 100E)", 1);
        AddButton(BID_COL_MEGACITY, sbX + 18 + btnW, cy, btnW, btnH, "+ Domed Megacity", "+50k Housing (1200M, 450E)", 1);

        // 5. Agronomy & Food Farms
        cy += btnH + 8;
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        sprintf(buf, "FOOD FARMS: %d Hydro | %d Aero | %d Algal Vats",
            sim.hydroTowers, sim.aeroponicFarms, sim.algalVats);
        TextOutA(hdc, sbX + 12, cy, buf, (int)strlen(buf));

        cy += 16;
        AddButton(BID_COL_HYDRO, sbX + 12, cy, btnW, btnH, "+ Hydroponic Tower", "+25 Food/cyc (200M, 100V)", 1);
        AddButton(BID_COL_AERO_FARM, sbX + 18 + btnW, cy, btnW, btnH, "+ Aeroponic Mega-Farm", "+80 Food (550M, 250V, 120E)", 1);

        cy += btnH + 4;
        AddButton(BID_COL_ALGAL_VAT, sbX + 12, cy, btnW, btnH, "+ Algal Protein Vats", "+45 Food (320M, 180E)", 1);
        AddButton(BID_COL_SOLAR, sbX + 18 + btnW, cy, btnW, btnH, "+ Surface Solar Grid", "+180 kW Grid (220 Min)", 1);
    }
    // TAB 3: RESEARCH & BREAKTHROUGHS (Phase 10)
    else if (sim.activeTab == 3) {
        int ry = contentY;

        // 1. Science Directorate HUD Card
        int hudH = 50;
        FillSolidRect(hdc, sbX + 12, ry, sidebarW - 24, hudH, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, ry, sidebarW - 24, hudH, COLOR_BORDER);
        FillSolidRect(hdc, sbX + 12, ry, 4, hudH, RGB(168, 85, 247)); // Purple accent

        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, RGB(216, 180, 254));
        sprintf(buf, "SCIENCE DIRECTORATE: %d SP (+%d/cyc)", sim.science, sim.deltaScience);
        TextOutA(hdc, sbX + 22, ry + 6, buf, (int)strlen(buf));

        SelectObject(hdc, hFontSmall);
        if (sim.activeTech >= 0 && sim.activeTech < MAX_TECHS && !sim.techResearched[sim.activeTech]) {
            TechDef* at = &g_techs[sim.activeTech];
            SetTextColor(hdc, COLOR_TEXT_PRI);
            sprintf(buf, "Active Focus: [%s] %s (%.0f/%d SP - %.0f%%)",
                at->code, at->name, sim.techProgress[sim.activeTech], at->costScience,
                (sim.techProgress[sim.activeTech] / (float)at->costScience) * 100.0f);
            TextOutA(hdc, sbX + 22, ry + 22, buf, (int)strlen(buf));
            DrawProgressBar(hdc, sbX + 22, ry + 37, sidebarW - 44, 6,
                sim.techProgress[sim.activeTech] / (float)at->costScience, RGB(168, 85, 247));
        } else {
            SetTextColor(hdc, COLOR_EMERALD);
            TextOutA(hdc, sbX + 22, ry + 22, "Active Focus: None (Click 'Focus' below to direct science)", 57);
            SetTextColor(hdc, COLOR_TEXT_DIM);
            TextOutA(hdc, sbX + 22, ry + 36, "Breakthrough technologies unlock permanent interstellar bonuses.", 64);
        }

        ry += hudH + 6;

        // 2. Render 9 Tech Cards
        for (int t = 0; t < MAX_TECHS; t++) {
            TechDef* td = &g_techs[t];
            int isResearched = sim.techResearched[t];
            int isActive = (!isResearched && sim.activeTech == t);
            int canUnlock = (!isResearched && (td->prereq < 0 || sim.techResearched[td->prereq]));
            int cardH = 37;

            COLORREF cardBg = isResearched ? RGB(10, 26, 18) : (isActive ? RGB(28, 20, 10) : COLOR_BG_CARD);
            COLORREF cardBorder = isResearched ? RGB(16, 100, 60) : (isActive ? RGB(245, 158, 11) : (canUnlock ? COLOR_BORDER : RGB(30, 41, 59)));
            COLORREF accentCol = isResearched ? COLOR_EMERALD : (isActive ? RGB(245, 158, 11) : (canUnlock ? RGB(168, 85, 247) : RGB(75, 85, 99)));

            FillSolidRect(hdc, sbX + 12, ry, sidebarW - 24, cardH, cardBg);
            FrameSolidRect(hdc, sbX + 12, ry, sidebarW - 24, cardH, cardBorder);
            FillSolidRect(hdc, sbX + 12, ry, 3, cardH, accentCol);

            // Title & Tier
            SelectObject(hdc, hFontBold);
            if (td->isBreakthrough) {
                SetTextColor(hdc, isResearched ? COLOR_EMERALD : RGB(245, 158, 11));
                sprintf(buf, "⭐ [%s] %s", td->code, td->name);
            } else {
                SetTextColor(hdc, isResearched ? COLOR_EMERALD : (canUnlock ? COLOR_TEXT_BRIGHT : COLOR_TEXT_DIM));
                sprintf(buf, "[%s] %s", td->code, td->name);
            }
            TextOutA(hdc, sbX + 20, ry + 4, buf, (int)strlen(buf));

            // Status label or Cost on right of title
            SelectObject(hdc, hFontSmall);
            if (isResearched) {
                SetTextColor(hdc, COLOR_EMERALD);
                TextOutA(hdc, sbX + sidebarW - 95, ry + 4, "[ONLINE]", 8);
            } else if (isActive) {
                SetTextColor(hdc, RGB(245, 158, 11));
                sprintf(buf, "%.0f%%", (sim.techProgress[t] / (float)td->costScience) * 100.0f);
                TextOutA(hdc, sbX + sidebarW - 65, ry + 4, buf, (int)strlen(buf));
            } else if (canUnlock) {
                SetTextColor(hdc, RGB(216, 180, 254));
                sprintf(buf, "%d SP", td->costScience);
                TextOutA(hdc, sbX + sidebarW - 75, ry + 4, buf, (int)strlen(buf));
            } else {
                SetTextColor(hdc, RGB(100, 116, 139));
                TextOutA(hdc, sbX + sidebarW - 85, ry + 4, "[LOCKED]", 8);
            }

            // Effect / Desc on second line
            SelectObject(hdc, hFontSmall);
            SetTextColor(hdc, isResearched ? COLOR_TEXT_PRI : (canUnlock ? COLOR_TEXT_DIM : RGB(71, 85, 105)));
            TextOutA(hdc, sbX + 20, ry + 18, td->effect, (int)strlen(td->effect));

            // Interactive Buttons on the right
            if (!isResearched && canUnlock) {
                AddButton(BID_TECH_FOCUS_BASE + t, sbX + sidebarW - 122, ry + 16, 44, 17, isActive ? "[Active]" : "Focus", NULL, 1);
                AddButton(BID_TECH_UNLOCK_BASE + t, sbX + sidebarW - 74, ry + 16, 54, 17, "Unlock", NULL, (sim.science >= td->costScience ? 1 : 0));
            }

            // Progress bar at bottom of card if active
            if (isActive) {
                DrawProgressBar(hdc, sbX + 15, ry + cardH - 3, sidebarW - 30, 2, sim.techProgress[t] / (float)td->costScience, RGB(245, 158, 11));
            }

            ry += cardH + 3;
        }

        // 3. Breakthrough Matrix Summary Card
        ry += 4;
        FillSolidRect(hdc, sbX + 12, ry, sidebarW - 24, 60, RGB(18, 20, 32));
        FrameSolidRect(hdc, sbX + 12, ry, sidebarW - 24, 60, COLOR_BORDER);

        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, RGB(245, 158, 11));
        TextOutA(hdc, sbX + 20, ry + 4, "TERRAFORMING BREAKTHROUGH STATUS", 32);

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, sim.techResearched[TECH_PROP_ANTIMATTER] ? COLOR_EMERALD : COLOR_TEXT_DIM);
        TextOutA(hdc, sbX + 20, ry + 18, sim.techResearched[TECH_PROP_ANTIMATTER] ? "[x] Antimatter: 3x Speed & 2x Trade Yields" : "[ ] Antimatter Drives: 3x Speed & 2x Trade (Offline)", sim.techResearched[TECH_PROP_ANTIMATTER] ? 42 : 51);

        SetTextColor(hdc, sim.techResearched[TECH_BIO_ADAPTED] ? COLOR_EMERALD : COLOR_TEXT_DIM);
        TextOutA(hdc, sbX + 20, ry + 31, sim.techResearched[TECH_BIO_ADAPTED] ? "[x] Adapted Biomes: +20% Hab & 2x Agronomy" : "[ ] Genetic Biomes: +20% Hab & 2x Food (Offline)", sim.techResearched[TECH_BIO_ADAPTED] ? 42 : 47);

        SetTextColor(hdc, sim.techResearched[TECH_GEO_STABILIZATION] ? COLOR_EMERALD : COLOR_TEXT_DIM);
        TextOutA(hdc, sbX + 20, ry + 44, sim.techResearched[TECH_GEO_STABILIZATION] ? "[x] Climate Matrix: Atmo Locked & +15% Hab" : "[ ] Climate Matrix: Locks Atmo & +15% Hab (Offline)", sim.techResearched[TECH_GEO_STABILIZATION] ? 42 : 50);
    }
    // TAB 4: ORBITAL MEGASTRUCTURES & PLANETARY DEFENSE
    else if (sim.activeTab == 4) {
        int my = contentY;

        // 1. Planetary Shield Grid Status Card
        SetTextColor(hdc, COLOR_CYAN);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, my, "PLANETARY SHIELD GRID ARRAY", 27);

        int shieldCardH = 76;
        FillSolidRect(hdc, sbX + 12, my + 16, sidebarW - 24, shieldCardH, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, my + 16, sidebarW - 24, shieldCardH, COLOR_BORDER);

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_TEXT_PRI);
        float shieldPct = (sim.shieldMaxHP > 0) ? ((float)sim.shieldHP / (float)sim.shieldMaxHP) : 0.0f;
        if (shieldPct < 0.0f) shieldPct = 0.0f;
        if (shieldPct > 1.0f) shieldPct = 1.0f;
        const char* mNames[] = { "BALANCED", "FORTIFIED", "STANDBY" };
        sprintf(buf, "Lattice Integrity: %d / %d HP (%.0f%%) [%s]",
            sim.shieldHP, sim.shieldMaxHP, shieldPct * 100.0f, mNames[sim.shieldMode]);
        TextOutA(hdc, sbX + 20, my + 20, buf, (int)strlen(buf));
        DrawProgressBar(hdc, sbX + 20, my + 34, sidebarW - 40, 6, shieldPct, RGB(0, 240, 255));

        SetTextColor(hdc, COLOR_CYAN);
        sprintf(buf, "Drain: %d kW/cyc | Flare/Impact Deflection: %s",
            (sim.shieldMode == 0 ? 30 : (sim.shieldMode == 1 ? 80 : 0)),
            sim.shieldHP > 0 ? "ONLINE" : "DEPLETED");
        TextOutA(hdc, sbX + 20, my + 44, buf, (int)strlen(buf));

        int btnW = (sidebarW - 46) / 2;
        AddButton(BID_MEGA_SHIELD_MODE, sbX + 20, my + 60, btnW, 22, "Shield Mode", NULL, sim.shieldGridStage > 0 ? 1 : 0);
        AddButton(BID_MEGA_SHIELD_CHARGE, sbX + 26 + btnW, my + 60, btnW, 22, "+300HP (180kW)", NULL, (sim.shieldGridStage > 0 && sim.energy >= 180) ? 1 : 0);

        my += shieldCardH + 24;

        // 2. Megastructure Engineering Projects Header
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, my, "MEGA-ENGINEERING PROJECTS", 25);
        my += 16;

        // Project 1: Orbital Ring Array
        int cardH = 64;
        FillSolidRect(hdc, sbX + 12, my, sidebarW - 24, cardH, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, my, sidebarW - 24, cardH, COLOR_BORDER);
        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_CYAN);
        const char* ringNames[] = { "Unbuilt", "Equatorial Truss", "Habitation Torus", "Sovereign Ringworld" };
        sprintf(buf, "Orbital Ring Array [Stage %d/3: %s]", sim.orbitalRingStage, ringNames[sim.orbitalRingStage]);
        TextOutA(hdc, sbX + 20, my + 6, buf, (int)strlen(buf));
        SetTextColor(hdc, COLOR_TEXT_PRI);
        if (sim.orbitalRingStage == 0) sprintf(buf, "Equatorial ring: +1k Housing, +50kW, +15%% Trade");
        else if (sim.orbitalRingStage == 1) sprintf(buf, "Torus ring: +3k Housing, +140kW, +35%% Trade");
        else if (sim.orbitalRingStage == 2) sprintf(buf, "Full Ringworld: +8k Housing, +300kW, +75%% Trade, +15%% Morale");
        else sprintf(buf, "Full Ringworld active: +8k Housing, +300kW, +75%% Trade, +15%% Morale");
        TextOutA(hdc, sbX + 20, my + 20, buf, (int)strlen(buf));

        int ringCostMin = sim.orbitalRingStage == 0 ? 1200 : (sim.orbitalRingStage == 1 ? 2800 : 5500);
        int ringCostEng = sim.orbitalRingStage == 0 ? 400 : (sim.orbitalRingStage == 1 ? 900 : 1800);
        if (sim.orbitalRingStage < 3) {
            sprintf(buf, "Upgrade (%d Min, %d kW)", ringCostMin, ringCostEng);
            int canUpg = (sim.minerals >= ringCostMin && sim.energy >= ringCostEng) ? 1 : 0;
            AddButton(BID_MEGA_RING_UPG, sbX + 20, my + 36, sidebarW - 40, 20, buf, NULL, canUpg);
        } else {
            AddButton(BID_MEGA_RING_UPG, sbX + 20, my + 36, sidebarW - 40, 20, "[MAX STAGE REACHED]", NULL, 0);
        }
        my += cardH + 6;

        // Project 2: Star Elevator Tether
        FillSolidRect(hdc, sbX + 12, my, sidebarW - 24, cardH, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, my, sidebarW - 24, cardH, COLOR_BORDER);
        SetTextColor(hdc, COLOR_CYAN);
        const char* elevNames[] = { "Unbuilt", "Carbon Nano-Ribbon", "Dual Climber Track", "Skyhook Super-Tether" };
        sprintf(buf, "Star Elevator Tether [Stage %d/3: %s]", sim.starElevatorStage, elevNames[sim.starElevatorStage]);
        TextOutA(hdc, sbX + 20, my + 6, buf, (int)strlen(buf));
        SetTextColor(hdc, COLOR_TEXT_PRI);
        if (sim.starElevatorStage == 0) sprintf(buf, "Ground-orbit tether: +5 Cryo colonists/cyc, Zero-g lifts");
        else if (sim.starElevatorStage == 1) sprintf(buf, "Dual track climber: +15 Cryo colonists/cyc, Free logistics");
        else if (sim.starElevatorStage == 2) sprintf(buf, "Apex Skyhook: +35 Cryo colonists/cyc, Rapid planetfall");
        else sprintf(buf, "Apex Skyhook active: +35 Cryo colonists/cyc, Instant surface transit");
        TextOutA(hdc, sbX + 20, my + 20, buf, (int)strlen(buf));

        int elevCostMin = sim.starElevatorStage == 0 ? 1000 : (sim.starElevatorStage == 1 ? 2400 : 4800);
        int elevCostEng = sim.starElevatorStage == 0 ? 350 : (sim.starElevatorStage == 1 ? 750 : 1500);
        if (sim.starElevatorStage < 3) {
            sprintf(buf, "Upgrade (%d Min, %d kW)", elevCostMin, elevCostEng);
            int canUpg = (sim.minerals >= elevCostMin && sim.energy >= elevCostEng) ? 1 : 0;
            AddButton(BID_MEGA_ELEV_UPG, sbX + 20, my + 36, sidebarW - 40, 20, buf, NULL, canUpg);
        } else {
            AddButton(BID_MEGA_ELEV_UPG, sbX + 20, my + 36, sidebarW - 40, 20, "[MAX STAGE REACHED]", NULL, 0);
        }
        my += cardH + 6;

        // Project 3: Planetary Shield Grid
        FillSolidRect(hdc, sbX + 12, my, sidebarW - 24, cardH, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, my, sidebarW - 24, cardH, COLOR_BORDER);
        SetTextColor(hdc, COLOR_CYAN);
        const char* shieldNames[] = { "Unbuilt", "Sub-Atmo Deflector (400HP)", "Flux Lattice (900HP)", "Aegis Barrier (1800HP, +10% Hab)" };
        sprintf(buf, "Planetary Shield Grid [Stage %d/3]", sim.shieldGridStage);
        TextOutA(hdc, sbX + 20, my + 6, buf, (int)strlen(buf));
        SetTextColor(hdc, COLOR_TEXT_PRI);
        if (sim.shieldGridStage == 1) sprintf(buf, "Flux Lattice Upg: 900 Max HP, 75%% deflection absorption");
        else if (sim.shieldGridStage == 2) sprintf(buf, "Aegis Barrier Upg: 1800 Max HP, 100%% deflection, +10%% Hab");
        else sprintf(buf, "Aegis Barrier active: 1800 Max HP envelope, +10%% Habitability");
        TextOutA(hdc, sbX + 20, my + 20, buf, (int)strlen(buf));

        int shCostMin = (sim.shieldGridStage == 1) ? 3200 : 6000;
        int shCostEng = (sim.shieldGridStage == 1) ? 1200 : 2200;
        if (sim.shieldGridStage < 3) {
            sprintf(buf, "Upgrade (%d Min, %d kW)", shCostMin, shCostEng);
            int canUpg = (sim.minerals >= shCostMin && sim.energy >= shCostEng) ? 1 : 0;
            AddButton(BID_MEGA_SHIELD_UPG, sbX + 20, my + 36, sidebarW - 40, 20, buf, NULL, canUpg);
        } else {
            AddButton(BID_MEGA_SHIELD_UPG, sbX + 20, my + 36, sidebarW - 40, 20, "[MAX STAGE REACHED]", NULL, 0);
        }
        my += cardH + 6;

        // Project 4: Planetary Defense Bastion
        int defCardH = 74;
        FillSolidRect(hdc, sbX + 12, my, sidebarW - 24, defCardH, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, my, sidebarW - 24, defCardH, COLOR_BORDER);
        SetTextColor(hdc, COLOR_ROSE);
        const char* defNames[] = { "Unbuilt", "Point-Defense Turrets", "Orbital Lance Battery", "Hyper-Kinetic Citadel" };
        sprintf(buf, "Planetary Defense Citadel [Stage %d/3: %s]", sim.defenseStationStage, defNames[sim.defenseStationStage]);
        TextOutA(hdc, sbX + 20, my + 6, buf, (int)strlen(buf));
        SetTextColor(hdc, COLOR_TEXT_PRI);
        if (sim.defenseStationStage == 0) sprintf(buf, "1 Citadel: 50%% Asteroid intercept, +2t Debris salvage");
        else if (sim.defenseStationStage == 1) sprintf(buf, "2 Citadels: 85%% Asteroid intercept, +4t Debris salvage");
        else if (sim.defenseStationStage == 2) sprintf(buf, "3 Citadels: 100%% Asteroid intercept, +8t Debris salvage");
        else sprintf(buf, "3 Citadels: 100%% Asteroid intercept, +8t Debris salvage/cyc");
        TextOutA(hdc, sbX + 20, my + 20, buf, (int)strlen(buf));

        int defCostMin = sim.defenseStationStage == 0 ? 1100 : (sim.defenseStationStage == 1 ? 2600 : 5200);
        int defCostEng = sim.defenseStationStage == 0 ? 300 : (sim.defenseStationStage == 1 ? 800 : 1600);
        int defBtnW = (sidebarW - 46) / 2;
        if (sim.defenseStationStage < 3) {
            sprintf(buf, "Upgrade (%d Min)", defCostMin);
            int canUpg = (sim.minerals >= defCostMin && sim.energy >= defCostEng) ? 1 : 0;
            AddButton(BID_MEGA_DEF_UPG, sbX + 20, my + 38, defBtnW, 22, buf, NULL, canUpg);
        } else {
            AddButton(BID_MEGA_DEF_UPG, sbX + 20, my + 38, defBtnW, 22, "[MAX STAGE]", NULL, 0);
        }
        AddButton(BID_MEGA_DEF_TEST, sbX + 26 + defBtnW, my + 38, defBtnW, 22, "Test Fire Citadel", NULL, sim.defenseStationStage > 0 ? 1 : 0);
    }
    // TAB 5: ALIEN XENOBIOLOGY & PRECURSOR RELICS
    else if (sim.activeTab == 5) {
        SetTextColor(hdc, COLOR_CYAN);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, contentY, "PRECURSOR RUINS & ARTIFACT VAULT", 32);

        int ry = contentY + 18;
        FillSolidRect(hdc, sbX + 12, ry, sidebarW - 24, 72, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, ry, sidebarW - 24, 72, COLOR_BORDER);

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_AMBER);
        int relicsCount = 0;
        for (int r = 0; r < 4; r++) if (sim.xenoRelicFound[r]) relicsCount++;
        sprintf(buf, "Relics Recovered: %d / 4 Precursor Artifacts", relicsCount);
        TextOutA(hdc, sbX + 20, ry + 8, buf, (int)strlen(buf));

        SetTextColor(hdc, COLOR_TEXT_PRI);
        sprintf(buf, "Resonance Sensor Array: %s | Vault Matrix: Online",
            sim.xenoScanAnim > 0.0f ? "SCANNING ACTIVE..." : "STANDBY");
        TextOutA(hdc, sbX + 20, ry + 24, buf, (int)strlen(buf));

        AddButton(BID_XENO_SCAN, sbX + 20, ry + 42, sidebarW - 40, 22, "Sub-Surface Bioscan (100 kW)", NULL, sim.energy >= 100 ? 1 : 0);

        int my = ry + 80;

        // 1. Excavation Sites Header
        SetTextColor(hdc, COLOR_PURPLE);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, my, "EXCAVATION SECTORS", 18);
        my += 16;

        const char* siteNames[4] = { "Site Alpha (Tachyon Monolith)", "Site Beta (Sub-Crustal Biosphere)", "Site Gamma (Orbital Lattice)", "Site Delta (Primordial Gate)" };
        const char* relicNames[4] = { "Zero-Point Siphon Core", "Hyper-Spore Bio-Catalyst", "Crystalline Harmonics Lattice", "Precursor Nanite Core" };

        for (int i = 0; i < 4; i++) {
            int cardH = 58;
            FillSolidRect(hdc, sbX + 12, my, sidebarW - 24, cardH, COLOR_BG_CARD);
            FrameSolidRect(hdc, sbX + 12, my, sidebarW - 24, cardH, COLOR_BORDER);

            SelectObject(hdc, hFontSmall);
            SetTextColor(hdc, COLOR_CYAN);
            sprintf(buf, "%s", siteNames[i]);
            TextOutA(hdc, sbX + 20, my + 4, buf, (int)strlen(buf));

            SetTextColor(hdc, COLOR_TEXT_PRI);
            if (sim.xenoRelicFound[i]) {
                SetTextColor(hdc, COLOR_EMERALD);
                sprintf(buf, "[EXCAVATED] %s secured!", relicNames[i]);
                TextOutA(hdc, sbX + 20, my + 18, buf, (int)strlen(buf));
                AddButton(BID_XENO_EXCAV_1 + i, sbX + 20, my + 32, sidebarW - 40, 20, "[RELIC SECURED IN VAULT]", NULL, 0);
            } else if (sim.xenoSiteStatus[i] == 1) {
                SetTextColor(hdc, COLOR_AMBER);
                sprintf(buf, "Excavation Progress: %d%%", sim.xenoSiteProgress[i]);
                TextOutA(hdc, sbX + 20, my + 18, buf, (int)strlen(buf));
                DrawProgressBar(hdc, sbX + 20, my + 32, (sidebarW - 46) / 2, 18, sim.xenoSiteProgress[i] / 100.0f, RGB(220, 160, 40));
                AddButton(BID_XENO_FAST_1 + i, sbX + 26 + (sidebarW - 46) / 2, my + 32, (sidebarW - 46) / 2, 20, "Sonic Drill (+25% / 200E)", NULL, sim.energy >= 200 ? 1 : 0);
            } else {
                SetTextColor(hdc, COLOR_TEXT_DIM);
                sprintf(buf, "Sub-surface relic signal detected. Requires expedition team.");
                TextOutA(hdc, sbX + 20, my + 18, buf, (int)strlen(buf));
                int canDig = (sim.minerals >= 300 && sim.energy >= 150) ? 1 : 0;
                AddButton(BID_XENO_EXCAV_1 + i, sbX + 20, my + 32, sidebarW - 40, 20, "Begin Excavation (300 Min, 150 kW)", NULL, canDig);
            }

            my += cardH + 6;
        }

        // 2. Precursor Technologies Header
        my += 4;
        SetTextColor(hdc, COLOR_EMERALD);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, my, "PRECURSOR TECH ARTIFACT SYNTHESIS", 33);
        my += 16;

        const char* techNames[4] = { "Zero-Point Tap (+450 kW Power)", "Bio-Catalysis (+25% Terra/Food)", "Crystalline Harmonics (+500 Shield HP)", "Nanite Assemblers (-25% Megastructure Cost)" };
        for (int i = 0; i < 4; i++) {
            int cardH = 52;
            FillSolidRect(hdc, sbX + 12, my, sidebarW - 24, cardH, COLOR_BG_CARD);
            FrameSolidRect(hdc, sbX + 12, my, sidebarW - 24, cardH, COLOR_BORDER);

            SelectObject(hdc, hFontSmall);
            SetTextColor(hdc, COLOR_TEXT_PRI);
            sprintf(buf, "%s", techNames[i]);
            TextOutA(hdc, sbX + 20, my + 4, buf, (int)strlen(buf));

            if (sim.precursorTech[i]) {
                SetTextColor(hdc, COLOR_EMERALD);
                AddButton(BID_XENO_TECH_1 + i, sbX + 20, my + 24, sidebarW - 40, 20, "[TECHNOLOGY ACTIVE]", NULL, 0);
            } else if (!sim.xenoRelicFound[i]) {
                SetTextColor(hdc, COLOR_TEXT_DIM);
                sprintf(buf, "Requires %s excavated first", relicNames[i]);
                TextOutA(hdc, sbX + 20, my + 20, buf, (int)strlen(buf));
                AddButton(BID_XENO_TECH_1 + i, sbX + 20, my + 24, sidebarW - 40, 20, "[LOCKED - RELIC REQUIRED]", NULL, 0);
            } else {
                int canSyn = (sim.energy >= 500 && sim.volatiles >= 250) ? 1 : 0;
                AddButton(BID_XENO_TECH_1 + i, sbX + 20, my + 24, sidebarW - 40, 20, "Synthesize Matrix (500 kW, 250 Vol)", NULL, canSyn);
            }

            my += cardH + 6;
        }
    }
    // TAB 6: HAZARDS & CRISIS MANAGEMENT
    else if (sim.activeTab == 6) {
        SetTextColor(hdc, COLOR_ROSE);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, contentY, "ASTROMETRIC THREAT RADAR", 24);

        int ry = contentY + 18;
        COLORREF radarBg = sim.crisisActive ? RGB(40, 12, 18) : COLOR_BG_CARD;
        COLORREF radarBorder = sim.crisisActive ? COLOR_ROSE : COLOR_BORDER;
        FillSolidRect(hdc, sbX + 12, ry, sidebarW - 24, 80, radarBg);
        FrameSolidRect(hdc, sbX + 12, ry, sidebarW - 24, 80, radarBorder);

        SelectObject(hdc, hFontBold);
        if (sim.crisisActive) {
            const char* names[] = {"None", "SOLAR FLARE / CORONAL WAVE", "ASTEROID / CHONDRITE IMPACT", "TECTONIC CRUSTAL QUAKE", "VIRULENT XENO-BLIGHT", "IONOSPHERIC MAGNETIC TEMPEST"};
            SetTextColor(hdc, COLOR_ROSE);
            sprintf(buf, "[CRITICAL ALERT] %s", names[sim.crisisType]);
            TextOutA(hdc, sbX + 20, ry + 10, buf, (int)strlen(buf));

            SelectObject(hdc, hFontSmall);
            SetTextColor(hdc, COLOR_TEXT_BRIGHT);
            sprintf(buf, "Class-%d Event | Time to Impact: %.1fs", sim.crisisSeverity, (sim.crisisTimer > 0.0f ? sim.crisisTimer : 0.0f));
            TextOutA(hdc, sbX + 20, ry + 32, buf, (int)strlen(buf));

            // Countdown progress bar
            FillSolidRect(hdc, sbX + 20, ry + 54, sidebarW - 40, 10, RGB(50, 15, 20));
            float pct = (sim.crisisMaxTime > 0.0f) ? (sim.crisisTimer / sim.crisisMaxTime) : 0.0f;
            if (pct < 0.0f) pct = 0.0f; if (pct > 1.0f) pct = 1.0f;
            FillSolidRect(hdc, sbX + 20, ry + 54, (int)((sidebarW - 40) * pct), 10, COLOR_ROSE);
        } else {
            SetTextColor(hdc, COLOR_EMERALD);
            TextOutA(hdc, sbX + 20, ry + 12, "[STATUS] THREAT LEVEL: NOMINAL", 30);
            SelectObject(hdc, hFontSmall);
            SetTextColor(hdc, COLOR_TEXT_PRI);
            TextOutA(hdc, sbX + 20, ry + 34, "Astrometric sensor telemetry: No imminent", 41);
            TextOutA(hdc, sbX + 20, ry + 50, "solar flares, chondrites, or quakes detected.", 45);
        }

        // Tactical Countermeasures Section
        int cty = ry + 88;
        SetTextColor(hdc, COLOR_AMBER);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, cty, "TACTICAL COUNTERMEASURES", 24);

        int my = cty + 18;
        if (sim.crisisActive) {
            AddButton(BID_CRISIS_QUICK, sbX + 12, my, sidebarW - 24, 26, "EXECUTE EMERGENCY MITIGATION", NULL, 1);
            my += 30;
        }

        AddButton(BID_MIT_FLARE, sbX + 12, my, (sidebarW - 28) / 2, 24, "Flare Deflect (350E,150V)", NULL, 1);
        AddButton(BID_MIT_ASTEROID, sbX + 12 + (sidebarW - 28) / 2 + 4, my, (sidebarW - 28) / 2, 24, "Kinetic Railgun (400M,200E)", NULL, 1);

        AddButton(BID_MIT_QUAKE, sbX + 12, my + 28, (sidebarW - 28) / 2, 24, "Geothermal Frac (300E,150M)", NULL, 1);
        AddButton(BID_MIT_BLIGHT, sbX + 12 + (sidebarW - 28) / 2 + 4, my + 28, (sidebarW - 28) / 2, 24, "Bio-Antidote (250V,150F)", NULL, 1);

        AddButton(BID_MIT_STORM, sbX + 12, my + 56, (sidebarW - 28) / 2, 24, "Ion Grounding (250E,100M)", NULL, 1);
        AddButton(BID_CRISIS_REPAIR, sbX + 12 + (sidebarW - 28) / 2 + 4, my + 56, (sidebarW - 28) / 2, 24, "Repair Corps (150M,200E)", NULL, 1);

        // Defense Infrastructure Section
        int dfy = my + 88;
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, dfy, "PLANETARY DEFENSE INFRASTRUCTURE", 32);

        int dy = dfy + 18;
        FillSolidRect(hdc, sbX + 12, dy, sidebarW - 24, 110, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, dy, sidebarW - 24, 110, COLOR_BORDER);

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_TEXT_PRI);
        sprintf(buf, "Astrometric Warning Radar: Tier %d (+%ds window)", sim.earlyWarningRadar, sim.earlyWarningRadar * 8);
        TextOutA(hdc, sbX + 20, dy + 10, buf, (int)strlen(buf));
        AddButton(BID_UPG_RADAR, sbX + sidebarW - 120, dy + 6, 96, 20, "+ Upg Radar", NULL, 1);

        sprintf(buf, "Magnetic Deflector: Tier %d (-%d%% dmg)", sim.shieldDeflector, sim.shieldDeflector * 30);
        TextOutA(hdc, sbX + 20, dy + 42, buf, (int)strlen(buf));
        AddButton(BID_UPG_SHIELD, sbX + sidebarW - 120, dy + 38, 96, 20, "+ Upg Shield", NULL, 1);

        sprintf(buf, "Automated Nanite Drones: Tier %d (Auto-reconstruction)", sim.repairDrones);
        TextOutA(hdc, sbX + 20, dy + 74, buf, (int)strlen(buf));
        AddButton(BID_UPG_DRONES, sbX + sidebarW - 120, dy + 70, 96, 20, "+ Upg Drones", NULL, 1);

        // Crisis Drills Section
        int dry = dy + 118;
        SetTextColor(hdc, COLOR_TEXT_DIM);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, dry, "HAZARD DRILLS & CRISIS SIMULATOR", 32);

        int drillW = (sidebarW - 32) / 5;
        AddButton(BID_DRILL_FLARE, sbX + 12, dry + 18, drillW, 22, "Flare", NULL, 1);
        AddButton(BID_DRILL_ASTEROID, sbX + 12 + drillW + 2, dry + 18, drillW, 22, "Asteroid", NULL, 1);
        AddButton(BID_DRILL_QUAKE, sbX + 12 + (drillW + 2) * 2, dry + 18, drillW, 22, "Quake", NULL, 1);
        AddButton(BID_DRILL_BLIGHT, sbX + 12 + (drillW + 2) * 3, dry + 18, drillW, 22, "Blight", NULL, 1);
        AddButton(BID_DRILL_STORM, sbX + 12 + (drillW + 2) * 4, dry + 18, drillW, 22, "Storm", NULL, 1);
    }
    // TAB 7: ECONOMY
    else if (sim.activeTab == 7) {
        SetTextColor(hdc, COLOR_BLUE);
        SelectObject(hdc, hFontBold);
        TextOutA(hdc, sbX + 12, contentY, "SECTOR RESOURCE LOOPS", 21);

        int ey = contentY + 18;
        FillSolidRect(hdc, sbX + 12, ey, sidebarW - 24, 120, COLOR_BG_CARD);
        FrameSolidRect(hdc, sbX + 12, ey, sidebarW - 24, 120, COLOR_BORDER);

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COLOR_EMERALD);
        float dockMult = 1.0f + (float)(sim.orbitalDocks - 1) * 0.20f;
        float fuelMult = 1.0f + (float)(sim.fuelDepots - 1) * 0.25f;
        int rMin = g_tradeRoutes[0].active ? (int)(g_tradeRoutes[0].baseYield * g_tradeRoutes[0].freighters * dockMult) : 0;
        int rVol = g_tradeRoutes[1].active ? (int)(g_tradeRoutes[1].baseYield * g_tradeRoutes[1].freighters * dockMult) : 0;
        int rFood = g_tradeRoutes[2].active ? (int)(g_tradeRoutes[2].baseYield * g_tradeRoutes[2].freighters * dockMult) : 0;
        int rEnergy = g_tradeRoutes[3].active ? (int)(g_tradeRoutes[3].baseYield * g_tradeRoutes[3].freighters * fuelMult) : 0;
        int massDriverMin = sim.massDrivers * 12;
        int fuelDepotPower = sim.fuelDepots * 40;

        int energyGen = 600 + (sim.surfaceSolar * 60) + rEnergy + fuelDepotPower;
        int energyDrain = 200 + (sim.solarMirrors * 75) + (sim.atmoProcessors * 60) + (sim.nitrogenExtractors * 85) + (sim.greenhouseStations * 70) + (sim.coreDynamos * 80) + (sim.colonists / 1000) * 5;
        sprintf(buf, "ENERGY: +%d kW Gen  |  -%d kW Drain  ->  Net: %+d kW/cyc", energyGen, energyDrain, sim.deltaEnergy);
        TextOutA(hdc, sbX + 20, ey + 12, buf, (int)strlen(buf));

        SetTextColor(hdc, COLOR_BLUE);
        int mineralGain = 20 + (strcmp(fleet[3].status, "Harvesting") == 0 || strcmp(fleet[3].status, "Mining Belt") == 0 ? 25 : 10) + rMin + massDriverMin;
        int mineralDrain = (sim.atmoProcessors * 3) + (sim.nitrogenExtractors * 2);
        sprintf(buf, "MINERALS: +%d t Mined  |  -%d t Built  ->  Net: %+d t/cyc", mineralGain, mineralDrain, sim.deltaMinerals);
        TextOutA(hdc, sbX + 20, ey + 38, buf, (int)strlen(buf));

        SetTextColor(hdc, COLOR_PURPLE);
        int volGain = 14 + (strcmp(fleet[3].status, "Scooping Ring") == 0 ? 18 : 6) + rVol;
        int volDrain = (sim.solarMirrors * 2) + (sim.greenhouseStations * 2) + (sim.nitrogenExtractors * 3);
        sprintf(buf, "VOLATILES: +%d t Scooped  |  -%d t Injected  ->  Net: %+d t/cyc", volGain, volDrain, sim.deltaVolatiles);
        TextOutA(hdc, sbX + 20, ey + 64, buf, (int)strlen(buf));

        SetTextColor(hdc, COLOR_AMBER);
        int foodGain = 15 + (sim.hydroTowers * 12) + rFood;
        int foodDrain = sim.colonists / 2000;
        sprintf(buf, "FOOD: +%d t Harvested  |  -%d t Eaten  ->  Net: %+d t/cyc", foodGain, foodDrain, sim.deltaFood);
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
                    } else if (bid == BID_THEME_TOGGLE) {
                        g_crtTheme = (g_crtTheme + 1) % 4;
                        char buf[128];
                        sprintf(buf, "CRT Phosphor mode changed to %s.", g_crtThemes[g_crtTheme].name);
                        SetLogMsg(buf, 0);
                        PlaySoundFx(SFX_CLICK);
                    } else if (bid == BID_GRID_TOGGLE) {
                        g_showGrid = !g_showGrid;
                        SetLogMsg(g_showGrid ? "Astrometric Cartography Grid engaged." : "Cartography Grid dimmed.", 0);
                        PlaySoundFx(SFX_CLICK);
                    } else if (bid == BID_GLOW_TOGGLE) {
                        g_phosphorGlow = !g_phosphorGlow;
                        SetLogMsg(g_phosphorGlow ? "CRT Phosphor bloom shader online." : "Phosphor bloom disabled.", 0);
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
                    } else if (bid == BID_SYS_CYCLE) {
                        LoadStarSystem((g_currentSystem + 1) % g_systemCount);
                        PlaySoundFx(SFX_SUCCESS);
                    } else if (bid == BID_SYS_SCAN) {
                        GenerateProceduralStarSystem();
                    } else if (bid == BID_TARGET_PLANET) {
                        if (sim.selectedType == 2 && sim.selectedIndex >= 0 && sim.selectedIndex < CURR_SYS.bodyCount) {
                            SetActivePlanet(sim.selectedIndex);
                            PlaySoundFx(SFX_SUCCESS);
                        }
                    } else if (bid >= BID_ROSTER_BASE && bid < BID_ROSTER_BASE + 20) {
                        int rIdx = bid - BID_ROSTER_BASE;
                        if (rIdx >= 0 && rIdx < CURR_SYS.bodyCount) {
                            SetActivePlanet(rIdx);
                            sim.selectedType = 2;
                            sim.selectedIndex = rIdx;
                            PlaySoundFx(SFX_CLICK);
                        }
                    } else if (bid >= BID_ACT_SOLAR_MIRROR && bid <= BID_ACT_ALGAE) {
                        HandleIntervention(bid);
                    } else if ((bid >= BID_COL_AWAKEN && bid <= BID_COL_SOLAR) ||
                               (bid >= BID_COL_MEGACITY && bid <= BID_COL_FOCUS_3)) {
                        HandleColonyProject(bid);
                    } else if (bid >= BID_ORDER_GEN_HOLD && bid <= BID_ORDER_TIT_HOLD) {
                        HandleShipOrder(bid);
                    } else if ((bid >= BID_ROUTE_TOG_0 && bid <= BID_ROUTE_TOG_3) ||
                               (bid >= BID_ROUTE_ADD_0 && bid <= BID_ROUTE_ADD_3)) {
                        HandleTradeRoute(bid);
                    } else if (bid >= BID_UPG_DOCKS && bid <= BID_UPG_MASS) {
                        HandleInfrastructure(bid);
                    } else if (bid >= BID_CRISIS_QUICK && bid <= BID_DRILL_STORM) {
                        HandleCrisisAction(bid);
                    } else if (bid >= BID_TECH_FOCUS_BASE && bid < BID_TECH_FOCUS_BASE + MAX_TECHS) {
                        int tIdx = bid - BID_TECH_FOCUS_BASE;
                        if (!sim.techResearched[tIdx]) {
                            sim.activeTech = tIdx;
                            char lBuf[128];
                            sprintf(lBuf, "Science Directorate focus set to: %s", g_techs[tIdx].name);
                            SetLogMsg(lBuf, 0);
                            PlaySoundFx(SFX_CLICK);
                        }
                    } else if (bid >= BID_TECH_UNLOCK_BASE && bid < BID_TECH_UNLOCK_BASE + MAX_TECHS) {
                        int tIdx = bid - BID_TECH_UNLOCK_BASE;
                        if (!sim.techResearched[tIdx]) {
                            int canUnlock = (g_techs[tIdx].prereq < 0 || sim.techResearched[g_techs[tIdx].prereq]);
                            int cost = g_techs[tIdx].costScience;
                            if (canUnlock && sim.science >= cost) {
                                sim.science -= cost;
                                sim.techProgress[tIdx] = (float)cost;
                                sim.techResearched[tIdx] = 1;
                                char lBuf[128];
                                if (g_techs[tIdx].isBreakthrough) {
                                    sim.breakthroughFanfare = 60;
                                    strncpy(sim.lastBreakthrough, g_techs[tIdx].name, sizeof(sim.lastBreakthrough) - 1);
                                    sprintf(lBuf, "BREAKTHROUGH: [%s] INSTANTLY UNLOCKED! %s", g_techs[tIdx].name, g_techs[tIdx].effect);
                                    SetLogMsg(lBuf, 0);
                                    PlaySoundFx(SFX_SUCCESS);
                                } else {
                                    sprintf(lBuf, "RESEARCH COMPLETED: [%s] online! %s", g_techs[tIdx].name, g_techs[tIdx].effect);
                                    SetLogMsg(lBuf, 0);
                                    PlaySoundFx(SFX_CLICK);
                                }
                                CalculateHabitability();
                                if (sim.activeTech == tIdx) {
                                    int nextT = -1;
                                    for (int k = 0; k < MAX_TECHS; k++) {
                                        if (!sim.techResearched[k] && (g_techs[k].prereq < 0 || sim.techResearched[g_techs[k].prereq])) {
                                            nextT = k;
                                            break;
                                        }
                                    }
                                    sim.activeTech = nextT;
                                }
                            } else if (!canUnlock) {
                                SetLogMsg("Prerequisite technology required before researching this tier.", 1);
                                PlaySoundFx(SFX_WARN);
                            } else {
                                SetLogMsg("Insufficient Science Points (SP) to instant-fund breakthrough.", 1);
                                PlaySoundFx(SFX_WARN);
                            }
                        }
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
                // Check Celestial Bodies across current star system
                if (!found) {
                    for (int i = 1; i < CURR_SYS.bodyCount; i++) {
                        float dist = (float)hypot(CURR_SYS.celestials[i].currX - mx, CURR_SYS.celestials[i].currY - my);
                        if (dist < CURR_SYS.celestials[i].radius * sim.zoom + 10.0f) {
                            if (CURR_SYS.celestials[i].isPlanet) sim.selectedType = 2;
                            else if (CURR_SYS.celestials[i].isMoon) sim.selectedType = 3;
                            else if (CURR_SYS.celestials[i].isStation) sim.selectedType = 4;
                            sim.selectedIndex = i;
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
                        sim.selectedIndex = 0;
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
                    sim.activeTab = (sim.activeTab + 1) % 8;
                    PlaySoundFx(SFX_CLICK);
                    break;
                case 'M':
                case 'm':
                    g_soundEnabled = !g_soundEnabled;
                    PlaySoundFx(SFX_CLICK);
                    break;
                case 'S':
                case 's':
                    LoadStarSystem((g_currentSystem + 1) % g_systemCount);
                    PlaySoundFx(SFX_SUCCESS);
                    break;
                case 'G':
                case 'g':
                    GenerateProceduralStarSystem();
                    break;
                case 'T':
                case 't':
                    if (sim.selectedType == 2 && sim.selectedIndex >= 0 && sim.selectedIndex < CURR_SYS.bodyCount) {
                        SetActivePlanet(sim.selectedIndex);
                        PlaySoundFx(SFX_SUCCESS);
                    }
                    break;
                case 'R':
                case 'r':
                    sim.camX = 0; sim.camY = 0; sim.zoom = 1.0f;
                    PlaySoundFx(SFX_CLICK);
                    break;
                case 'P':
                case 'p': {
                    CelestialBody* curActP = GetActivePlanet();
                    sim.selectedType = 2;
                    sim.selectedIndex = CURR_SYS.activePlanetIndex;
                    sim.camX = -curActP->currX + (winW - 380) / 2 + sim.camX;
                    sim.camY = -curActP->currY + (winH - 80) / 2 + sim.camY;
                    PlaySoundFx(SFX_CLICK);
                    break;
                }
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
