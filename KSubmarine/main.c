#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TIMER_ID 1
#define TIMER_INTERVAL 50 // ms

// Control IDs
#define ID_BTN_FLOOD_BALLAST    101
#define ID_BTN_BLOW_BALLAST     102
#define ID_BTN_TRIM_BOW         103
#define ID_BTN_TRIM_STERN       104
#define ID_BTN_SONAR_PING       105
#define ID_BTN_THROTTLE_REV     106
#define ID_BTN_THROTTLE_STOP    107
#define ID_BTN_THROTTLE_HALF    108
#define ID_BTN_THROTTLE_FLANK   109
#define ID_BTN_RUDDER_PORT      110
#define ID_BTN_RUDDER_STBD      111
#define ID_BTN_SEARCHLIGHTS     112
#define ID_BTN_SCRUBBER         113
#define ID_BTN_O2_PURGE         114
#define ID_BTN_BILGE_PUMP       115
#define ID_BTN_LOW_POWER        116
#define ID_BTN_EMERGENCY_BLOW   117
#define ID_BTN_SOUND_TOGGLE     118
#define ID_BTN_THEME_TOGGLE     119
#define ID_BTN_SCANLINES_TOGGLE 120
#define ID_BTN_VIEW_SONAR       121
#define ID_BTN_VIEW_NAVMAP      122
#define ID_BTN_NEXT_WAYPOINT    123
#define ID_BTN_AUTOPILOT        124
#define ID_BTN_SURVEY_SECTOR    125
#define ID_BTN_VIEW_ENG         126
#define ID_BTN_UPG_HULL         127
#define ID_BTN_UPG_BALLAST      128
#define ID_BTN_UPG_BATTERY      129
#define ID_BTN_UPG_LIGHTS       130
#define ID_BTN_FIELD_DIAG       131
#define ID_BTN_VIEW_CODEX       132
#define ID_BTN_LOCK_TARGET      133
#define ID_BTN_SCAN_TARGET      134
#define ID_BTN_HUD_NEXT_TARGET  135
#define ID_BTN_HUD_SCAN_TARGET  136

typedef enum {
    THEME_ABYSS = 0,
    THEME_EMERALD,
    THEME_AMBER,
    THEME_MONOCHROME,
    THEME_COUNT
} ThemeId;

typedef struct {
    const char* name;
    const char* tag;
    COLORREF bgDeep;
    COLORREF bgPanel;
    COLORREF bgHeader;
    COLORREF borderPanel;
    COLORREF borderGlow;
    COLORREF textPrimary;
    COLORREF textBright;
    COLORREF textDim;
    COLORREF accentSonar;
    COLORREF accentEmerald;
    COLORREF accentAmber;
    COLORREF accentRed;
    COLORREF gaugeBg;
    COLORREF btnBg;
    COLORREF btnActive;
    COLORREF radarBg;
    COLORREF radarRing;
    COLORREF scanlineClr;
} SubmarineTheme;

static const SubmarineTheme g_themes[THEME_COUNT] = {
    // 0: Abyssal Cyan
    {
        "Abyssal Cyan", "THEME: CYAN",
        RGB(2, 11, 18), RGB(7, 23, 36), RGB(13, 34, 53),
        RGB(19, 60, 90), RGB(0, 210, 255),
        RGB(56, 189, 248), RGB(224, 242, 254), RGB(2, 132, 199),
        RGB(0, 240, 255), RGB(16, 185, 129), RGB(245, 158, 11), RGB(239, 68, 68),
        RGB(3, 16, 28), RGB(12, 36, 56), RGB(2, 132, 199),
        RGB(1, 6, 10), RGB(0, 90, 130), RGB(0, 25, 45)
    },
    // 1: Emerald Sonar
    {
        "Sonar Emerald", "THEME: EMERALD",
        RGB(2, 15, 9), RGB(6, 30, 20), RGB(13, 47, 33),
        RGB(20, 78, 55), RGB(16, 185, 129),
        RGB(52, 211, 153), RGB(209, 250, 229), RGB(5, 150, 105),
        RGB(16, 185, 129), RGB(52, 211, 153), RGB(251, 191, 36), RGB(248, 113, 113),
        RGB(2, 18, 11), RGB(13, 46, 32), RGB(5, 150, 105),
        RGB(1, 10, 6), RGB(15, 75, 45), RGB(0, 30, 12)
    },
    // 2: Amber Depth
    {
        "Amber Depth", "THEME: AMBER",
        RGB(18, 9, 2), RGB(32, 19, 6), RGB(51, 31, 12),
        RGB(87, 52, 19), RGB(245, 158, 11),
        RGB(251, 191, 36), RGB(254, 243, 199), RGB(180, 83, 9),
        RGB(251, 191, 36), RGB(52, 211, 153), RGB(245, 158, 11), RGB(248, 113, 113),
        RGB(20, 11, 3), RGB(48, 29, 10), RGB(180, 83, 9),
        RGB(10, 6, 1), RGB(90, 55, 15), RGB(35, 18, 0)
    },
    // 3: Monochrome Radar
    {
        "Monochrome Radar", "THEME: MONO",
        RGB(5, 5, 5), RGB(17, 20, 23), RGB(26, 32, 38),
        RGB(51, 65, 85), RGB(226, 232, 240),
        RGB(203, 213, 225), RGB(255, 255, 255), RGB(100, 116, 139),
        RGB(248, 250, 252), RGB(148, 163, 184), RGB(226, 232, 240), RGB(248, 113, 113),
        RGB(9, 11, 14), RGB(25, 33, 44), RGB(100, 116, 139),
        RGB(4, 5, 6), RGB(70, 80, 95), RGB(20, 20, 20)
    }
};

typedef struct {
    char time[12];
    char text[128];
    COLORREF color;
} LogEntry;

#define MAX_LOGS 16

typedef struct {
    char id[8];
    char name[32];
    float x; // km
    float y; // km
    float depth;
    int discovered;
    int pts;
    char info[96];
} Landmark;

typedef struct {
    int id;
    const char* name;
    const char* depthRange;
    float baseSeabed;
    float seabedVariance;
    float thermalBase;
    const char* desc;
    Landmark landmarks[2];
} SectorInfo;

typedef struct {
    char name[32];
    int sectorIdx;
    float x;
    float y;
    float targetDepth;
} NavWaypoint;

#define SECTOR_COUNT 4
#define WAYPOINT_COUNT 8
#define FAUNA_COUNT 12

// --- FAUNA & ANOMALIES STRUCTURE (PHASE 7) ---
typedef struct {
    char id[8];
    char name[32];
    int type; // 0: Fauna, 1: Squid, 2: Flora, 3: Trench, 4: Leviathan
    int sectorIdx;
    float depth;
    float x, y;
    float vx, vy;
    char desc[128];
    char lumens[32];
    char freq[32];
    char behavior[48];
    int pts;
    int discovered;
} FaunaAnomaly;

// --- UPGRADE MODULE STRUCTURES ---
typedef struct {
    int tier;
    const char* name;
    int cost;
    float crushDepth;
    float maxHull;
    const char* desc;
} HullUpgrade;

typedef struct {
    int tier;
    const char* name;
    int cost;
    float maxAir;
    float rate;
    float recharge;
    const char* desc;
} BallastUpgrade;

typedef struct {
    int tier;
    const char* name;
    int cost;
    float drainMult;
    float regen;
    const char* desc;
} BatteryUpgrade;

typedef struct {
    int tier;
    const char* name;
    int cost;
    float range;
    float surveyMult;
    const char* desc;
} LightsUpgrade;

static const HullUpgrade g_hullUpg[4] = {
    { 1, "MK I - STANDARD", 0, 4500.0f, 100.0f, "Standard epipelagic hull." },
    { 2, "MK II - TITANIUM CARBIDE", 150, 6800.0f, 125.0f, "Titanium-carbide shell. Bathyal rated." },
    { 3, "MK III - GRAPHENE MESH", 350, 9200.0f, 150.0f, "Diamondoid graphene mesh bulkheads." },
    { 4, "MK IV - HADAL MATRIX", 650, 12000.0f, 200.0f, "Superdense nano-polymer matrix. Full Hadal rated." }
};

static const BallastUpgrade g_ballastUpg[4] = {
    { 1, "MK I - PNEUMATIC", 0, 300.0f, 10.0f, 5.0f, "Standard compressed air tanks." },
    { 2, "MK II - CRYO-TURBO", 120, 450.0f, 15.0f, 8.0f, "Cryogenic compression pumps." },
    { 3, "MK III - QUAD-HYDRAULIC", 300, 600.0f, 20.0f, 12.0f, "Hydraulic displacement manifolds." },
    { 4, "MK IV - GAS EXPANDER", 550, 850.0f, 25.0f, 18.0f, "Supercritical solid-fuel gas cartridges." }
};

static const BatteryUpgrade g_batteryUpg[4] = {
    { 1, "MK I - LI-POLYMER", 0, 1.0f, 0.0f, "Chemical lithium-polymer bank." },
    { 2, "MK II - SILVER-ZINC", 150, 0.75f, 0.0f, "Silver-zinc deep cells (-25% load)." },
    { 3, "MK III - PLUTONIUM RTG", 350, 0.55f, 0.12f, "Pu-238 RTG micro-core (+0.12%/s regen)." },
    { 4, "MK IV - THORIUM CORE", 600, 0.35f, 0.28f, "Molten thorium reactor (+0.28%/s regen)." }
};

static const LightsUpgrade g_lightsUpg[4] = {
    { 1, "MK I - HALOGEN", 0, 250.0f, 1.0f, "Halogen searchlights." },
    { 2, "MK II - XENON ARCS", 100, 450.0f, 1.25f, "Dual xenon-arc beams (+25% survey)." },
    { 3, "MK III - PULSED LIDAR", 250, 750.0f, 1.6f, "UV pulsed lidar scanner (+60% survey)." },
    { 4, "MK IV - QUANTUM OPTICS", 500, 1200.0f, 2.0f, "Quantum photonic core (2.0x survey)." }
};

static SectorInfo g_sectors[SECTOR_COUNT] = {
    {
        0, "CONTINENTAL SHELF", "0 - 200m", 250.0f, 60.0f, 21.4f,
        "Sunlit epipelagic waters, expansive shallow ridges.",
        {
            { "cs1", "Emerald Kelp Ridge", 1.2f, 1.8f, 140.0f, 0, 50, "Dense bioluminescent kelp forest on shallow granite." },
            { "cs2", "Coral Siphon Reef", -2.1f, 3.4f, 180.0f, 0, 75, "Ancient carbonate pinnacle hosting rare benthic colonies." }
        }
    },
    {
        1, "TWILIGHT DROP-OFF", "200 - 1,000m", 1350.0f, 220.0f, 11.2f,
        "Midnight mesopelagic zone. Steep vertical basalt walls.",
        {
            { "td1", "Basalt Canyon Fault", 4.5f, -2.2f, 650.0f, 0, 100, "Massive vertical subsea fault carved by tectonic shifts." },
            { "td2", "Derelict Bathysphere", 2.8f, -4.5f, 880.0f, 0, 150, "Corroded 1970s deep-sea exploration capsule with data core." }
        }
    },
    {
        2, "HYDROTHERMAL VENTS", "1,000 - 4,000m", 3800.0f, 400.0f, 4.0f,
        "Active bathyal crust. Black smokers and superheated sulfur plumes.",
        {
            { "hv1", "Prometheus Smoker", -5.5f, -6.0f, 2450.0f, 0, 200, "Towering 30m sulfide chimney venting 320 deg C fluid." },
            { "hv2", "Sulfur Caldera Vent", -3.8f, -8.2f, 3200.0f, 0, 250, "Active magma fissure glowing with incandescent basalt." }
        }
    },
    {
        3, "HADAL TRENCH CHASM", "4,000 - 11,000m", 10920.0f, 600.0f, 1.8f,
        "Extreme Hadalpelagic trench. Extreme crush depth and silence.",
        {
            { "ha1", "Challenger Arch", 8.4f, 7.2f, 7200.0f, 0, 350, "Monolithic stone arch bridging the hadal subduction trench." },
            { "ha2", "Abyssal Siren Deep", 9.8f, 10.5f, 10500.0f, 0, 500, "Deepest tectonic rupture on Earth, echoing with acoustic pulses." }
        }
    }
};

static const NavWaypoint g_waypoints[WAYPOINT_COUNT] = {
    { "Emerald Kelp Shelf", 0, 1.2f, 1.8f, 140.0f },
    { "Coral Siphon Reef", 0, -2.1f, 3.4f, 180.0f },
    { "Basalt Canyon Fault", 1, 4.5f, -2.2f, 650.0f },
    { "Derelict Bathysphere", 1, 2.8f, -4.5f, 880.0f },
    { "Prometheus Smoker", 2, -5.5f, -6.0f, 2450.0f },
    { "Sulfur Caldera Vent", 2, -3.8f, -8.2f, 3200.0f },
    { "Challenger Arch", 3, 8.4f, 7.2f, 7200.0f },
    { "Abyssal Siren Deep", 3, 9.8f, 10.5f, 10500.0f }
};

static FaunaAnomaly g_fauna[FAUNA_COUNT] = {
    // Sector 0: Continental Shelf (0-200m)
    { "f01", "Megamouth Filter Shark", 0, 0, 120.0f, 0.8f, 1.2f, 0.04f, 0.02f, "Rare epipelagic shark with luminescent photophores along its upper jaw.", "120 LUX (CYAN)", "180 HZ LOW", "Docile Plankton Grazer", 60, 0 },
    { "f02", "Bioluminescent Kelp Flora", 2, 0, 85.0f, 1.8f, -0.6f, 0.0f, 0.0f, "Emerald chemosynthetic algae anchored on shallow granite shelves.", "340 LUX (EMERALD)", "0 HZ (STATIC)", "Photosynthetic Benthic Organism", 50, 0 },
    { "f03", "Pygmy Cuttlefish Pod", 1, 0, 165.0f, -1.5f, 2.2f, -0.05f, 0.03f, "Small translucent cephalopods displaying shifting chromatic ripples.", "210 LUX (AMBER)", "420 HZ PULSE", "Skittish Schooling Cephalopods", 70, 0 },

    // Sector 1: Twilight Drop-Off (200-1000m)
    { "f04", "Crown Medusa Swarm", 0, 1, 620.0f, 3.8f, -1.4f, 0.02f, -0.03f, "Colossal mesopelagic jellyfish radiating ultraviolet flash patterns.", "480 LUX (VIOLET)", "95 HZ OSC", "Drifting Mesopelagic Cnidarian", 90, 0 },
    { "f05", "Bioluminescent Glass Squid", 1, 1, 480.0f, 2.2f, -3.8f, 0.06f, 0.04f, "Cranchiid squid with crystal mantle and twin photophore eyes.", "380 LUX (AZURE)", "640 HZ CLICKS", "Inquisitive Midwater Predator", 100, 0 },
    { "f06", "Basalt Canyon Rift Fault", 3, 1, 840.0f, 4.8f, -2.6f, 0.0f, 0.0f, "Massive vertical tectonic fissure echoing with sub-crustal frequencies.", "40 LUX (INFRARED)", "24 HZ SUB-BASS", "Active Tectonic Fault Zone", 120, 0 },

    // Sector 2: Hydrothermal Vents (1000-4000m)
    { "f07", "Giant Riftia Tube Worms", 2, 2, 2350.0f, -5.0f, -5.4f, 0.0f, 0.0f, "Extremophile worm spires thriving in 350 deg C mineral plumes.", "180 LUX (RUBY)", "60 HZ THERMAL", "Hydrothermal Extremophile Community", 140, 0 },
    { "f08", "Colossal Architeuthis Squid", 1, 2, 1750.0f, -3.2f, -7.5f, -0.08f, 0.05f, "14m deep squid with luminous feeding tentacles and massive eyes.", "550 LUX (DEEP CYAN)", "120 HZ RESONANCE", "Territorial Abyssal Apex Hunter", 180, 0 },
    { "f09", "Phantom Gulper Eel", 0, 2, 3100.0f, -4.5f, -9.0f, 0.04f, -0.06f, "Abyssal predator with an expandable jaw and glowing caudal lure.", "290 LUX (MAGENTA)", "310 HZ CHIRP", "Elusive Deep-Water Stalker", 160, 0 },

    // Sector 3: Mariana Hadal Chasm (4000-11000m)
    { "f10", "Abyssal Siren Leviathan", 4, 3, 7600.0f, 8.0f, 6.8f, 0.07f, -0.05f, "Colossal primordial hadal megafauna vibrating with harmonic sub-bass songs.", "920 LUX (HARMONIC GOLD)", "18 HZ INFRASOUND", "Ancient Primordial Titan (Revered)", 300, 0 },
    { "f11", "Bioluminescent Ghost Flora", 2, 3, 9200.0f, 9.2f, 8.5f, 0.0f, 0.0f, "Silicate mycelium clinging to hadal basalt walls pulsing with bio-electricity.", "420 LUX (PALE CYAN)", "0 HZ (STATIC)", "Silicate Hadal Chemosynthesis", 220, 0 },
    { "f12", "Challenger Deep Subduction Rift", 3, 3, 10850.0f, 10.2f, 11.0f, 0.0f, 0.0f, "The deepest subduction rupture on Earth, echoing with gravitational distortion.", "150 LUX (GRAVITATIONAL)", "8 HZ INFRASOUND", "Primordial Abyssal Abyss", 400, 0 }
};

typedef struct {
    float depth;            // meters (0 - 11000)
    float vertRate;         // m/s
    float targetVertRate;
    float speed;            // knots (-2.5 to 11.5)
    float targetSpeed;
    int throttleMode;       // 0: REV, 1: STOP, 2: HALF, 3: FLANK
    float heading;          // degrees (0 - 359)
    float pitch;            // degrees (-15 to +15)

    // Navigation & Ocean Coordinates
    float posX;
    float posY;
    float distanceCruised;
    int currentSectorIdx;
    int activeWaypointIdx;
    int autopilot;
    int surveyPoints;
    int viewMode;           // 0: Sonar, 1: Nav Map, 2: Codex, 3: Engineering
    float breadcrumbsX[32];
    float breadcrumbsY[32];
    int breadcrumbCount;
    float seabedElevation;

    // Upgrades & Engineering
    int upgradeHull;
    int upgradeBallast;
    int upgradeBattery;
    int upgradeLights;
    float maxAirReservoir;
    float airRechargeRate;
    float ballastStepRate;
    float powerDrainMult;
    float passiveBatteryRegen;
    float surveyMultiplier;
    float opticalRange;
    int milestone200;
    int milestone1000;
    int milestone4000;
    int milestone6000;
    int milestone10000;

    // Phase 7: Fauna Target Locking & Bio-Scan
    int selectedTargetIdx;
    int isScanningTarget;
    float scanProgress;

    // Vital systems
    float hull;             // 0 - 100%
    float crushDepth;       // 4500m (upgradable)
    float pressure;         // atm
    float hullStress;       // 0 - 100%

    // Life support
    float o2;               // 0 - 100%
    float co2;              // %
    float scrubberStatus;   // 0 - 100%
    int scrubberAuto;
    int o2PurgeCount;

    // Power & Electrical
    float battery;          // 0 - 100%
    float powerDrain;       // kW
    int lowPowerMode;
    int searchlights;

    // Ballast & Bilge
    float ballast;          // 0 - 100% (0 = surface, 45 = neutral, 100 = heavy)
    float airReservoir;     // 0 - 300 BAR (upgradable)
    float bilgeWater;       // gallons
    int bilgePumpActive;
    float waterIntrusionRate; // GPM

    // Environment
    float temp;             // Celsius

    // Sonar
    int isPinging;
    float pingRadius;
    float sweepAngle;

    // Sound
    int soundEnabled;

    // Theme & CRT Scanlines
    int currentTheme;
    int scanlinesEnabled;

    // Log
    LogEntry logs[MAX_LOGS];
    int logCount;
} SubmarineState;

static SubmarineState g_sub;
static HWND g_hWnd = NULL;
static HFONT g_hFontTitle = NULL;
static HFONT g_hFontNormal = NULL;
static HFONT g_hFontSmall = NULL;
static HFONT g_hFontBold = NULL;

void PlaySoundAsync(DWORD freq, DWORD duration);
void PlayLeviathanHarmonic(void);
void AddLog(const char* text, COLORREF color);
void InitSubmarineState(void);
void UpdateSimulation(float dt);
void DrawUI(HDC hdc, RECT* rcClient);
int GetSectorFaunaIndices(int sectorIdx, int outIndices[3]);

DWORD WINAPI SoundThreadProc(LPVOID lpParam) {
    DWORD packed = (DWORD)(UINT_PTR)lpParam;
    DWORD freq = LOWORD(packed);
    DWORD dur = HIWORD(packed);
    Beep(freq, dur);
    return 0;
}

void PlaySoundAsync(DWORD freq, DWORD duration) {
    if (!g_sub.soundEnabled) return;
    DWORD packed = MAKELONG((WORD)freq, (WORD)duration);
    CreateThread(NULL, 0, SoundThreadProc, (LPVOID)(UINT_PTR)packed, 0, NULL);
}

DWORD WINAPI LeviathanSoundThreadProc(LPVOID lpParam) {
    Beep(85, 180);
    Sleep(30);
    Beep(110, 220);
    Sleep(30);
    Beep(65, 300);
    return 0;
}

void PlayLeviathanHarmonic(void) {
    if (!g_sub.soundEnabled) return;
    CreateThread(NULL, 0, LeviathanSoundThreadProc, NULL, 0, NULL);
}

void AddLog(const char* text, COLORREF color) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    if (g_sub.logCount >= MAX_LOGS) {
        for (int i = 0; i < MAX_LOGS - 1; i++) {
            g_sub.logs[i] = g_sub.logs[i + 1];
        }
        g_sub.logCount = MAX_LOGS - 1;
    }
    
    LogEntry* e = &g_sub.logs[g_sub.logCount++];
    snprintf(e->time, sizeof(e->time), "[%02d:%02d:%02d]", st.wHour, st.wMinute, st.wSecond);
    strncpy(e->text, text, sizeof(e->text) - 1);
    e->text[sizeof(e->text) - 1] = '\0';
    e->color = color;
}

int GetSectorFaunaIndices(int sectorIdx, int outIndices[3]) {
    int count = 0;
    for (int i = 0; i < FAUNA_COUNT; i++) {
        if (g_fauna[i].sectorIdx == sectorIdx) {
            if (count < 3) {
                outIndices[count++] = i;
            }
        }
    }
    return count;
}

void InitSubmarineState(void) {
    memset(&g_sub, 0, sizeof(g_sub));
    g_sub.depth = 0.0f;
    g_sub.vertRate = 0.0f;
    g_sub.targetVertRate = 0.0f;
    g_sub.speed = 0.0f;
    g_sub.targetSpeed = 0.0f;
    g_sub.throttleMode = 1; // STOP
    g_sub.heading = 42.0f;
    g_sub.pitch = 0.0f;

    // Navigation & Coordinates
    g_sub.posX = 0.0f;
    g_sub.posY = 0.0f;
    g_sub.distanceCruised = 0.0f;
    g_sub.currentSectorIdx = 0;
    g_sub.activeWaypointIdx = 0;
    g_sub.autopilot = 0;
    g_sub.surveyPoints = 150; // Research Credits
    g_sub.viewMode = 0; // 0: Sonar, 1: Nav Map, 2: Codex, 3: Engineering
    g_sub.breadcrumbCount = 0;
    g_sub.seabedElevation = 250.0f;

    // Upgrades & Engineering
    g_sub.upgradeHull = 1;
    g_sub.upgradeBallast = 1;
    g_sub.upgradeBattery = 1;
    g_sub.upgradeLights = 1;
    g_sub.maxAirReservoir = 300.0f;
    g_sub.airRechargeRate = 5.0f;
    g_sub.ballastStepRate = 10.0f;
    g_sub.powerDrainMult = 1.0f;
    g_sub.passiveBatteryRegen = 0.0f;
    g_sub.surveyMultiplier = 1.0f;
    g_sub.opticalRange = 250.0f;
    g_sub.milestone200 = 0;
    g_sub.milestone1000 = 0;
    g_sub.milestone4000 = 0;
    g_sub.milestone6000 = 0;
    g_sub.milestone10000 = 0;

    // Phase 7 Target Locking & Bio-Scan
    g_sub.selectedTargetIdx = 0;
    g_sub.isScanningTarget = 0;
    g_sub.scanProgress = 0.0f;

    g_sub.hull = 100.0f;
    g_sub.crushDepth = 4500.0f;
    g_sub.pressure = 1.0f;
    g_sub.hullStress = 0.1f;

    g_sub.o2 = 100.0f;
    g_sub.co2 = 0.04f;
    g_sub.scrubberStatus = 99.0f;
    g_sub.scrubberAuto = 1;
    g_sub.o2PurgeCount = 3;

    g_sub.battery = 100.0f;
    g_sub.powerDrain = 0.4f;
    g_sub.lowPowerMode = 0;
    g_sub.searchlights = 0;

    g_sub.ballast = 0.0f;
    g_sub.airReservoir = 300.0f;
    g_sub.bilgeWater = 0.0f;
    g_sub.bilgePumpActive = 0;
    g_sub.waterIntrusionRate = 0.0f;

    g_sub.temp = 21.4f;

    g_sub.isPinging = 0;
    g_sub.pingRadius = 0.0f;
    g_sub.sweepAngle = 0.0f;
    g_sub.soundEnabled = 1;

    g_sub.currentTheme = THEME_ABYSS;
    g_sub.scanlinesEnabled = 1;

    g_sub.logCount = 0;
    AddLog("DSV Abyss Voyager Bathyscaphe computer online. Systems nominal.", g_themes[THEME_ABYSS].textPrimary);
    AddLog("Active sonar & biological hydrophone array online. Listening...", g_themes[THEME_ABYSS].accentEmerald);
}

const char* GetZoneName(float depth) {
    if (depth < 200.0f) return "EPIPELAGIC (0-200M)";
    if (depth < 1000.0f) return "MESOPELAGIC (200-1000M)";
    if (depth < 4000.0f) return "BATHYPELAGIC (1000-4000M)";
    if (depth < 6000.0f) return "ABYSSOPELAGIC (4000-6000M)";
    return "HADALPELAGIC (6000M+)";
}

void UpdateSimulation(float dt) {
    const SubmarineTheme* th = &g_themes[g_sub.currentTheme];
    float neutralBallast = 45.0f;
    float buoyancyForce = (neutralBallast - g_sub.ballast) * 0.4f;
    float pitchDescent = (g_sub.pitch / 15.0f) * (fabsf(g_sub.speed) * 0.3f);

    g_sub.targetVertRate = -buoyancyForce - pitchDescent;
    g_sub.vertRate += (g_sub.targetVertRate - g_sub.vertRate) * (dt * 1.5f);

    g_sub.depth += g_sub.vertRate * dt;
    if (g_sub.depth <= 0.0f) {
        g_sub.depth = 0.0f;
        if (g_sub.vertRate < 0.0f) g_sub.vertRate = 0.0f;
        if (g_sub.airReservoir < g_sub.maxAirReservoir) g_sub.airReservoir = min(g_sub.maxAirReservoir, g_sub.airReservoir + dt * g_sub.airRechargeRate);
        if (g_sub.battery < 100.0f) g_sub.battery = min(100.0f, g_sub.battery + dt * 2.0f);
    }
    if (g_sub.depth > 11000.0f) g_sub.depth = 11000.0f;

    // Depth milestone checks
    if (g_sub.depth >= 200.0f && !g_sub.milestone200) {
        g_sub.milestone200 = 1;
        g_sub.surveyPoints += 75;
        PlaySoundAsync(880, 120);
        AddLog("DEPTH MILESTONE: Submerged past 200m! (+75 Research Credits)", th->accentEmerald);
    }
    if (g_sub.depth >= 1000.0f && !g_sub.milestone1000) {
        g_sub.milestone1000 = 1;
        g_sub.surveyPoints += 100;
        PlaySoundAsync(880, 120);
        AddLog("DEPTH MILESTONE: Submerged past 1000m! (+100 Research Credits)", th->accentEmerald);
    }
    if (g_sub.depth >= 4000.0f && !g_sub.milestone4000) {
        g_sub.milestone4000 = 1;
        g_sub.surveyPoints += 150;
        PlaySoundAsync(880, 120);
        AddLog("DEPTH MILESTONE: Submerged past 4000m! (+150 Research Credits)", th->accentEmerald);
    }
    if (g_sub.depth >= 6000.0f && !g_sub.milestone6000) {
        g_sub.milestone6000 = 1;
        g_sub.surveyPoints += 250;
        PlaySoundAsync(880, 150);
        AddLog("DEPTH MILESTONE: Submerged into Hadal Trench! (+250 Research Credits)", th->accentEmerald);
    }
    if (g_sub.depth >= 10000.0f && !g_sub.milestone10000) {
        g_sub.milestone10000 = 1;
        g_sub.surveyPoints += 500;
        PlaySoundAsync(880, 200);
        AddLog("DEPTH MILESTONE: Challenger Deep Bottom Reached! (+500 Research Credits)", th->accentEmerald);
    }

    // Sector transition
    int sIdx = 0;
    if (g_sub.depth < 200.0f) sIdx = 0;
    else if (g_sub.depth < 1000.0f) sIdx = 1;
    else if (g_sub.depth < 4000.0f) sIdx = 2;
    else sIdx = 3;

    if (g_sub.currentSectorIdx != sIdx) {
        g_sub.currentSectorIdx = sIdx;
        g_sub.selectedTargetIdx = 0;
        char sMsg[128];
        snprintf(sMsg, sizeof(sMsg), "TRANSITIONING SECTOR: [%s] - %s", g_sectors[sIdx].name, g_sectors[sIdx].desc);
        AddLog(sMsg, th->accentSonar);
        PlaySoundAsync(650, 120);
    }

    // Speed surge
    g_sub.speed += (g_sub.targetSpeed - g_sub.speed) * (dt * 0.8f);

    // Autopilot course correction
    if (g_sub.autopilot) {
        const NavWaypoint* wp = &g_waypoints[g_sub.activeWaypointIdx];
        float dx = wp->x - g_sub.posX;
        float dy = wp->y - g_sub.posY;
        float targetRad = atan2f(dx, dy);
        float targetDeg = fmodf(targetRad * (180.0f / 3.14159265f) + 360.0f, 360.0f);
        float diff = targetDeg - g_sub.heading;
        while (diff < -180.0f) diff += 360.0f;
        while (diff > 180.0f) diff -= 360.0f;
        if (fabsf(diff) > 1.0f) {
            float step = (diff > 0 ? 1.0f : -1.0f) * min(fabsf(diff), 20.0f * dt);
            g_sub.heading = fmodf(g_sub.heading + step + 360.0f, 360.0f);
        }
    }

    // Coordinate traversal (1 knot = 1.852 km/h = 0.0005144 km/s)
    float speedKmS = (g_sub.speed * 1.852f) / 3600.0f;
    float hRad = g_sub.heading * (3.14159265f / 180.0f);
    g_sub.posX += sinf(hRad) * speedKmS * dt;
    g_sub.posY += cosf(hRad) * speedKmS * dt;
    g_sub.distanceCruised += fabsf(speedKmS * dt) * 0.539957f;

    // Breadcrumb trail
    static float s_bcTimer = 0.0f;
    s_bcTimer += dt;
    if (s_bcTimer >= 1.0f) {
        s_bcTimer = 0.0f;
        if (g_sub.breadcrumbCount < 32) {
            g_sub.breadcrumbsX[g_sub.breadcrumbCount] = g_sub.posX;
            g_sub.breadcrumbsY[g_sub.breadcrumbCount] = g_sub.posY;
            g_sub.breadcrumbCount++;
        } else {
            for (int i = 0; i < 31; i++) {
                g_sub.breadcrumbsX[i] = g_sub.breadcrumbsX[i+1];
                g_sub.breadcrumbsY[i] = g_sub.breadcrumbsY[i+1];
            }
            g_sub.breadcrumbsX[31] = g_sub.posX;
            g_sub.breadcrumbsY[31] = g_sub.posY;
        }
    }

    // Fauna Dynamic Roaming (Phase 7)
    for (int i = 0; i < FAUNA_COUNT; i++) {
        if (g_fauna[i].vx != 0.0f || g_fauna[i].vy != 0.0f) {
            g_fauna[i].x += g_fauna[i].vx * dt * 0.2f;
            g_fauna[i].y += g_fauna[i].vy * dt * 0.2f;
            if (fabsf(g_fauna[i].x) > 14.0f) g_fauna[i].vx *= -1.0f;
            if (fabsf(g_fauna[i].y) > 14.0f) g_fauna[i].vy *= -1.0f;
        }
    }

    // Bio-Scan Progress Update (Phase 7)
    if (g_sub.isScanningTarget) {
        g_sub.scanProgress += dt * 60.0f;
        if (g_sub.scanProgress >= 100.0f) {
            g_sub.isScanningTarget = 0;
            g_sub.scanProgress = 0.0f;

            int secFauna[3];
            int fCount = GetSectorFaunaIndices(g_sub.currentSectorIdx, secFauna);
            if (fCount > 0 && g_sub.selectedTargetIdx < fCount) {
                int fIdx = secFauna[g_sub.selectedTargetIdx];
                FaunaAnomaly* target = &g_fauna[fIdx];
                if (!target->discovered) {
                    target->discovered = 1;
                    int ptsAwarded = (int)(target->pts * g_sub.surveyMultiplier);
                    g_sub.surveyPoints += ptsAwarded;

                    if (target->type == 4) { // Leviathan
                        PlayLeviathanHarmonic();
                        char msg[128];
                        snprintf(msg, sizeof(msg), "🚨 LEVIATHAN SCANNED: [%s]! %s (+%d PTS)", target->name, target->desc, ptsAwarded);
                        AddLog(msg, RGB(244, 63, 94));
                    } else {
                        PlaySoundAsync(1100, 200);
                        char msg[128];
                        snprintf(msg, sizeof(msg), "BIO-SCAN COMPLETE: [%s]! %s (+%d PTS)", target->name, target->desc, ptsAwarded);
                        AddLog(msg, th->accentEmerald);
                    }
                }
            }
        }
    }

    // Dynamic Seabed
    SectorInfo* curSec = &g_sectors[g_sub.currentSectorIdx];
    float terrainNoise = sinf(g_sub.posX * 1.5f) * cosf(g_sub.posY * 1.5f) * curSec->seabedVariance;
    g_sub.seabedElevation = curSec->baseSeabed + terrainNoise;

    if (g_sub.depth >= g_sub.seabedElevation) {
        g_sub.depth = g_sub.seabedElevation;
        if (g_sub.vertRate > 0.0f) g_sub.vertRate = 0.0f;
        if (fabsf(g_sub.speed) > 2.0f && (rand() % 100) < 3) {
            g_sub.hull = max(0.0f, g_sub.hull - 0.5f * dt);
            PlaySoundAsync(140, 100);
            AddLog("WARNING: Keel scraping seabed rock shelf!", th->accentAmber);
        }
    }

    g_sub.pressure = 1.0f + (g_sub.depth * 0.0995f);
    g_sub.hullStress = min(100.0f, (g_sub.depth / g_sub.crushDepth) * 100.0f);

    if (g_sub.depth > g_sub.crushDepth) {
        float excess = g_sub.depth - g_sub.crushDepth;
        float hullDamageReduction = 1.0f - (g_sub.upgradeHull - 1) * 0.2f;
        if (hullDamageReduction < 0.3f) hullDamageReduction = 0.3f;
        float hullDamage = (excess * 0.02f + 0.5f) * dt * hullDamageReduction;
        g_sub.hull = max(0.0f, g_sub.hull - hullDamage);
        g_sub.waterIntrusionRate = excess * 0.05f * hullDamageReduction;
        if ((rand() % 100) < 3) {
            PlaySoundAsync(150, 200);
            AddLog("CRUSH WARNING: Extreme hydrostatic pressure deforming hull!", th->accentRed);
        }
    } else {
        g_sub.waterIntrusionRate = 0.0f;
    }

    if (g_sub.waterIntrusionRate > 0.0f) {
        g_sub.bilgeWater += g_sub.waterIntrusionRate * dt;
    }
    if (g_sub.bilgePumpActive && g_sub.bilgeWater > 0.0f && g_sub.battery > 0.0f) {
        float pumped = min(g_sub.bilgeWater, (10.0f + (g_sub.upgradeBallast - 1) * 5.0f) * dt);
        g_sub.bilgeWater -= pumped;
    }

    float baseDrain = 0.3f;
    if (g_sub.searchlights) baseDrain += (0.8f / g_sub.upgradeLights);
    if (g_sub.throttleMode == 2) baseDrain += 1.2f;
    if (g_sub.throttleMode == 3) baseDrain += 3.5f;
    if (g_sub.bilgePumpActive) baseDrain += 0.6f;
    if (g_sub.scrubberAuto) baseDrain += 0.4f;
    if (g_sub.autopilot) baseDrain += 0.3f;
    if (g_sub.lowPowerMode) baseDrain *= 0.45f;

    baseDrain *= g_sub.powerDrainMult;
    g_sub.powerDrain = baseDrain;

    if (g_sub.depth > 0.0f) {
        g_sub.battery = max(0.0f, g_sub.battery - (baseDrain * 0.015f * dt));
        if (g_sub.passiveBatteryRegen > 0.0f) {
            g_sub.battery = min(100.0f, g_sub.battery + g_sub.passiveBatteryRegen * dt * 2.0f);
        }
    }

    if (g_sub.scrubberAuto && g_sub.battery > 0.0f) {
        g_sub.scrubberStatus = max(10.0f, g_sub.scrubberStatus - dt * 0.01f);
        g_sub.co2 = min(2.0f, g_sub.co2 + dt * 0.002f);
        g_sub.o2 = max(0.0f, g_sub.o2 - dt * 0.015f);
    } else {
        g_sub.co2 = min(8.0f, g_sub.co2 + dt * 0.02f);
        g_sub.o2 = max(0.0f, g_sub.o2 - dt * 0.05f);
    }

    // Seawater Temp & Thermal Smoker Anomaly
    float ambientTemp = 21.4f;
    if (g_sub.depth < 200.0f) {
        ambientTemp = 21.4f - (g_sub.depth / 200.0f) * 8.0f;
    } else if (g_sub.depth < 1000.0f) {
        ambientTemp = 13.4f - ((g_sub.depth - 200.0f) / 800.0f) * 9.0f;
    } else {
        ambientTemp = max(1.2f, 4.4f - ((g_sub.depth - 1000.0f) / 9000.0f) * 3.2f);
    }

    float thermalBoost = 0.0f;
    if (g_sub.currentSectorIdx == 2) {
        float dx = -5.5f - g_sub.posX;
        float dy = -6.0f - g_sub.posY;
        float distToVent = sqrtf(dx * dx + dy * dy);
        if (distToVent < 1.0f) {
            thermalBoost = (1.0f - distToVent) * 140.0f;
        }
    }
    g_sub.temp = ambientTemp + thermalBoost;

    g_sub.sweepAngle += 0.035f;
    if (g_sub.sweepAngle >= 6.2831853f) g_sub.sweepAngle -= 6.2831853f;

    if (g_sub.isPinging) {
        g_sub.pingRadius += 5.0f;
        if (g_sub.pingRadius > 180.0f) {
            g_sub.isPinging = 0;
            g_sub.pingRadius = 0.0f;
            AddLog("Active sonar omnidirectional ping cycle completed.", th->textDim);
        }
    }
}

void DrawGaugeBar(HDC hdc, int x, int y, int w, int h, float percent, COLORREF fillClr, const SubmarineTheme* th) {
    RECT rcTrack = { x, y, x + w, y + h };
    HBRUSH hBrTrack = CreateSolidBrush(th->gaugeBg);
    HBRUSH hBrBorder = CreateSolidBrush(th->borderPanel);
    FillRect(hdc, &rcTrack, hBrTrack);
    FrameRect(hdc, &rcTrack, hBrBorder);
    DeleteObject(hBrTrack);
    DeleteObject(hBrBorder);

    if (percent > 0.0f) {
        int fillW = (int)((w - 2) * (percent / 100.0f));
        if (fillW > w - 2) fillW = w - 2;
        if (fillW > 0) {
            RECT rcFill = { x + 1, y + 1, x + 1 + fillW, y + h - 1 };
            HBRUSH hBrFill = CreateSolidBrush(fillClr);
            FillRect(hdc, &rcFill, hBrFill);
            DeleteObject(hBrFill);
        }
    }
}

void DrawPanelBox(HDC hdc, int x, int y, int w, int h, const char* title, const char* status, COLORREF statusClr, const SubmarineTheme* th) {
    RECT rcBox = { x, y, x + w, y + h };
    HBRUSH hBrPanel = CreateSolidBrush(th->bgPanel);
    HBRUSH hBrBorder = CreateSolidBrush(th->borderPanel);
    FillRect(hdc, &rcBox, hBrPanel);
    FrameRect(hdc, &rcBox, hBrBorder);
    DeleteObject(hBrPanel);

    RECT rcHdr = { x, y, x + w, y + 24 };
    HBRUSH hBrHdr = CreateSolidBrush(th->bgHeader);
    FillRect(hdc, &rcHdr, hBrHdr);
    FrameRect(hdc, &rcHdr, hBrBorder);
    DeleteObject(hBrHdr);
    DeleteObject(hBrBorder);

    SelectObject(hdc, g_hFontBold);
    SetTextColor(hdc, th->textBright);
    SetBkMode(hdc, TRANSPARENT);
    TextOutA(hdc, x + 8, y + 4, title, (int)strlen(title));

    if (status && status[0]) {
        SetTextColor(hdc, statusClr);
        SIZE sz;
        GetTextExtentPoint32A(hdc, status, (int)strlen(status), &sz);
        TextOutA(hdc, x + w - sz.cx - 8, y + 4, status, (int)strlen(status));
    }
}

void DrawCustomButton(HDC hdc, int id, int x, int y, int w, int h, const char* label, int isActive, COLORREF accentClr, const SubmarineTheme* th) {
    RECT rc = { x, y, x + w, y + h };
    HBRUSH hBr = CreateSolidBrush(isActive ? th->btnActive : th->btnBg);
    HBRUSH hBrBorder = CreateSolidBrush(isActive ? accentClr : th->borderPanel);
    FillRect(hdc, &rc, hBr);
    FrameRect(hdc, &rc, hBrBorder);
    DeleteObject(hBr);
    DeleteObject(hBrBorder);

    SelectObject(hdc, g_hFontSmall);
    SetTextColor(hdc, isActive ? th->textBright : (accentClr != th->borderGlow ? accentClr : th->textPrimary));
    SetBkMode(hdc, TRANSPARENT);
    
    SIZE sz;
    GetTextExtentPoint32A(hdc, label, (int)strlen(label), &sz);
    int tx = x + (w - sz.cx) / 2;
    int ty = y + (h - sz.cy) / 2;
    TextOutA(hdc, tx, ty, label, (int)strlen(label));
}

void DrawNavMapChart(HDC hdc, int cx, int cy, int mapW, int mapH, const SubmarineTheme* th) {
    RECT rcMap = { cx - mapW / 2, cy - mapH / 2, cx + mapW / 2, cy + mapH / 2 };
    HBRUSH hBrMap = CreateSolidBrush(RGB(1, 8, 14));
    FillRect(hdc, &rcMap, hBrMap);
    DeleteObject(hBrMap);

    float scale = 18.0f; // 1 km = 18 px

    // Coordinate Grid Lines
    HPEN hPenGrid = CreatePen(PS_SOLID, 1, RGB(19, 60, 90));
    HPEN hPenOld = (HPEN)SelectObject(hdc, hPenGrid);

    for (int gx = -15; gx <= 15; gx += 3) {
        int sx = cx + (int)((gx - g_sub.posX) * scale);
        if (sx >= rcMap.left && sx <= rcMap.right) {
            MoveToEx(hdc, sx, rcMap.top, NULL);
            LineTo(hdc, sx, rcMap.bottom);
        }
    }
    for (int gy = -15; gy <= 15; gy += 3) {
        int sy = cy + (int)((gy - g_sub.posY) * scale);
        if (sy >= rcMap.top && sy <= rcMap.bottom) {
            MoveToEx(hdc, rcMap.left, sy, NULL);
            LineTo(hdc, rcMap.right, sy);
        }
    }

    // Sector Region Circles
    HPEN hPenSec = CreatePen(PS_SOLID, 1, RGB(0, 180, 220));
    SelectObject(hdc, hPenSec);
    HBRUSH hBrNull = (HBRUSH)GetStockObject(NULL_BRUSH);
    SelectObject(hdc, hBrNull);

    for (int i = 0; i < SECTOR_COUNT; i++) {
        float sxCoord = (i == 0 ? 0.0f : (i == 1 ? 3.0f : (i == 2 ? -4.0f : 8.0f)));
        float syCoord = (i == 0 ? 2.0f : (i == 1 ? -3.0f : (i == 2 ? -7.0f : 8.0f)));
        int scx = cx + (int)((sxCoord - g_sub.posX) * scale);
        int scy = cy + (int)((syCoord - g_sub.posY) * scale);
        int rad = (int)(3.5f * scale);
        Ellipse(hdc, scx - rad, scy - rad, scx + rad, scy + rad);
        SetTextColor(hdc, th->textDim);
        TextOutA(hdc, scx - 40, scy - rad - 12, g_sectors[i].name, (int)strlen(g_sectors[i].name));
    }
    DeleteObject(hPenSec);

    // Hydrothermal Hotspot Circle
    int vx = cx + (int)((-5.5f - g_sub.posX) * scale);
    int vy = cy + (int)((-6.0f - g_sub.posY) * scale);
    HPEN hPenVent = CreatePen(PS_SOLID, 1, RGB(239, 68, 68));
    SelectObject(hdc, hPenVent);
    Ellipse(hdc, vx - 20, vy - 20, vx + 20, vy + 20);
    SetTextColor(hdc, RGB(239, 68, 68));
    TextOutA(hdc, vx - 50, vy + 22, "THERMAL SPOUT", 13);
    DeleteObject(hPenVent);

    // Breadcrumbs
    if (g_sub.breadcrumbCount > 1) {
        HPEN hPenTrail = CreatePen(PS_SOLID, 1, th->accentEmerald);
        SelectObject(hdc, hPenTrail);
        for (int i = 0; i < g_sub.breadcrumbCount; i++) {
            int bx = cx + (int)((g_sub.breadcrumbsX[i] - g_sub.posX) * scale);
            int by = cy + (int)((g_sub.breadcrumbsY[i] - g_sub.posY) * scale);
            if (i == 0) MoveToEx(hdc, bx, by, NULL);
            else LineTo(hdc, bx, by);
        }
        DeleteObject(hPenTrail);
    }

    // Waypoints
    for (int i = 0; i < WAYPOINT_COUNT; i++) {
        const NavWaypoint* wp = &g_waypoints[i];
        int wx = cx + (int)((wp->x - g_sub.posX) * scale);
        int wy = cy + (int)((wp->y - g_sub.posY) * scale);
        int isTarget = (i == g_sub.activeWaypointIdx);

        HBRUSH hBrWp = CreateSolidBrush(isTarget ? th->accentAmber : th->accentSonar);
        SelectObject(hdc, hBrWp);
        Ellipse(hdc, wx - 4, wy - 4, wx + 4, wy + 4);
        DeleteObject(hBrWp);

        char buf[64];
        snprintf(buf, sizeof(buf), "WP-%d: %s (%.0fm)", i + 1, wp->name, wp->targetDepth);
        SetTextColor(hdc, isTarget ? th->textBright : th->textDim);
        TextOutA(hdc, wx + 6, wy - 6, buf, (int)strlen(buf));

        if (isTarget) {
            HPEN hPenCourse = CreatePen(PS_DOT, 1, th->accentAmber);
            SelectObject(hdc, hPenCourse);
            MoveToEx(hdc, cx, cy, NULL);
            LineTo(hdc, wx, wy);
            DeleteObject(hPenCourse);
        }
    }

    // Vessel Center
    HBRUSH hBrSub = CreateSolidBrush(th->accentEmerald);
    SelectObject(hdc, hBrSub);
    Ellipse(hdc, cx - 5, cy - 5, cx + 5, cy + 5);
    DeleteObject(hBrSub);

    float hRad = (g_sub.heading - 90.0f) * (3.14159265f / 180.0f);
    HPEN hPenHeading = CreatePen(PS_SOLID, 2, th->accentEmerald);
    SelectObject(hdc, hPenHeading);
    MoveToEx(hdc, cx, cy, NULL);
    LineTo(hdc, cx + (int)(cosf(hRad) * 22), cy + (int)(sinf(hRad) * 22));
    DeleteObject(hPenHeading);

    SetTextColor(hdc, th->textBright);
    TextOutA(hdc, cx + 8, cy - 18, "DSV ABYSS (YOU)", 15);

    SelectObject(hdc, hPenOld);
    DeleteObject(hPenGrid);
}

// --- DRAW FAUNA & ANOMALIES CODEX VIEW (PHASE 7) ---
void DrawFaunaCodex(HDC hdc, int x, int y, int w, int h, const SubmarineTheme* th) {
    RECT rcBg = { x, y, x + w, y + h };
    HBRUSH hBr = CreateSolidBrush(th->bgDeep);
    FillRect(hdc, &rcBg, hBr);
    DeleteObject(hBr);

    int discoveredCount = 0;
    for (int i = 0; i < FAUNA_COUNT; i++) {
        if (g_fauna[i].discovered) discoveredCount++;
    }

    char hdrBuf[128];
    snprintf(hdrBuf, sizeof(hdrBuf), "ABYSSAL CODEX: %d / %d DISCOVERED (%d%%)  |  RESEARCH CREDITS: %d PTS",
             discoveredCount, FAUNA_COUNT, (discoveredCount * 100) / FAUNA_COUNT, g_sub.surveyPoints);

    SelectObject(hdc, g_hFontBold);
    SetTextColor(hdc, th->accentEmerald);
    TextOutA(hdc, x + 10, y + 6, hdrBuf, (int)strlen(hdrBuf));

    int margin = 6;
    int cols = 3;
    int rows = 4;
    int cardW = (w - margin * (cols + 1)) / cols;
    int cardH = (h - 26 - margin * (rows + 1)) / rows;

    for (int i = 0; i < FAUNA_COUNT; i++) {
        FaunaAnomaly* f = &g_fauna[i];
        int c = i % cols;
        int r = i / cols;
        int cx = x + margin + c * (cardW + margin);
        int cy = y + 26 + margin + r * (cardH + margin);

        RECT rcCard = { cx, cy, cx + cardW, cy + cardH };
        HBRUSH hBrP = CreateSolidBrush(th->bgPanel);
        HBRUSH hBrB = CreateSolidBrush(f->discovered ? th->accentSonar : th->borderPanel);
        FillRect(hdc, &rcCard, hBrP);
        FrameRect(hdc, &rcCard, hBrB);
        DeleteObject(hBrP);
        DeleteObject(hBrB);

        const char* typeStr = f->type == 4 ? "[LEVIATHAN]" : (f->type == 1 ? "[SQUID]" : (f->type == 2 ? "[FLORA]" : (f->type == 3 ? "[TRENCH]" : "[FAUNA]")));
        COLORREF typeClr = f->type == 4 ? RGB(244, 63, 94) : (f->type == 1 ? RGB(244, 114, 182) : (f->type == 2 ? th->accentEmerald : (f->type == 3 ? th->accentAmber : th->accentSonar)));

        SelectObject(hdc, g_hFontSmall);
        SetTextColor(hdc, typeClr);
        TextOutA(hdc, cx + 6, cy + 4, typeStr, (int)strlen(typeStr));

        if (f->discovered) {
            SelectObject(hdc, g_hFontBold);
            SetTextColor(hdc, th->textBright);
            TextOutA(hdc, cx + 65, cy + 4, f->name, (int)strlen(f->name));

            SelectObject(hdc, g_hFontSmall);
            SetTextColor(hdc, th->textDim);
            char dBuf[64];
            snprintf(dBuf, sizeof(dBuf), "Depth: %.0fm  %s", f->depth, f->lumens);
            TextOutA(hdc, cx + 6, cy + 18, dBuf, (int)strlen(dBuf));

            SetTextColor(hdc, th->accentEmerald);
            snprintf(dBuf, sizeof(dBuf), "SCANNED (+%d PTS)", (int)(f->pts * g_sub.surveyMultiplier));
            TextOutA(hdc, cx + 6, cy + cardH - 14, dBuf, (int)strlen(dBuf));
        } else {
            SelectObject(hdc, g_hFontSmall);
            SetTextColor(hdc, th->textDim);
            TextOutA(hdc, cx + 65, cy + 4, "[UNDISCOVERED]", 14);

            char dBuf[64];
            snprintf(dBuf, sizeof(dBuf), "Depth: ~%.0fm  Freq: %s", f->depth, f->freq);
            TextOutA(hdc, cx + 6, cy + 18, dBuf, (int)strlen(dBuf));

            SetTextColor(hdc, th->accentAmber);
            snprintf(dBuf, sizeof(dBuf), "UNSCANNED (YIELD: +%d PTS)", f->pts);
            TextOutA(hdc, cx + 6, cy + cardH - 14, dBuf, (int)strlen(dBuf));
        }
    }
}

void DrawEngineeringBay(HDC hdc, int x, int y, int w, int h, const SubmarineTheme* th) {
    RECT rcBg = { x, y, x + w, y + h };
    HBRUSH hBr = CreateSolidBrush(th->bgDeep);
    FillRect(hdc, &rcBg, hBr);
    DeleteObject(hBr);

    int margin = 6;
    int gridW = (w - margin * 3) / 2;
    int gridH = (h - margin * 3) / 2;

    int c1x = x + margin;
    int c2x = x + margin * 2 + gridW;
    int r1y = y + margin;
    int r2y = y + margin * 2 + gridH;

    char buf[128];

    // Card 1: Titanium Hull Plating
    {
        RECT rcCard = { c1x, r1y, c1x + gridW, r1y + gridH };
        HBRUSH hBrP = CreateSolidBrush(th->bgPanel);
        HBRUSH hBrB = CreateSolidBrush(th->borderPanel);
        FillRect(hdc, &rcCard, hBrP);
        FrameRect(hdc, &rcCard, hBrB);
        DeleteObject(hBrP);
        DeleteObject(hBrB);

        int tier = g_sub.upgradeHull;
        const HullUpgrade* upg = &g_hullUpg[tier - 1];

        SelectObject(hdc, g_hFontBold);
        SetTextColor(hdc, th->textBright);
        TextOutA(hdc, c1x + 8, r1y + 8, "TITANIUM HULL PLATING", 21);

        SelectObject(hdc, g_hFontSmall);
        SetTextColor(hdc, tier == 4 ? th->accentEmerald : th->accentSonar);
        TextOutA(hdc, c1x + 8, r1y + 24, upg->name, (int)strlen(upg->name));

        SetTextColor(hdc, th->textDim);
        TextOutA(hdc, c1x + 8, r1y + 38, upg->desc, (int)strlen(upg->desc));

        snprintf(buf, sizeof(buf), "CRUSH: %.0fm   MAX HULL: %.0f%%", upg->crushDepth, upg->maxHull);
        SetTextColor(hdc, th->textPrimary);
        TextOutA(hdc, c1x + 8, r1y + 54, buf, (int)strlen(buf));

        if (tier < 4) {
            const HullUpgrade* nextUpg = &g_hullUpg[tier];
            snprintf(buf, sizeof(buf), "UPGRADE -> %s (%d PTS)", nextUpg->name, nextUpg->cost);
            int canAfford = g_sub.surveyPoints >= nextUpg->cost;
            DrawCustomButton(hdc, ID_BTN_UPG_HULL, c1x + 8, r1y + gridH - 26, gridW - 16, 20, buf, 0, canAfford ? th->accentSonar : th->textDim, th);
        } else {
            DrawCustomButton(hdc, ID_BTN_UPG_HULL, c1x + 8, r1y + gridH - 26, gridW - 16, 20, "MAX TIER [OPTIMAL]", 1, th->accentEmerald, th);
        }
    }

    // Card 2: High-Output Ballast Pumps
    {
        RECT rcCard = { c2x, r1y, c2x + gridW, r1y + gridH };
        HBRUSH hBrP = CreateSolidBrush(th->bgPanel);
        HBRUSH hBrB = CreateSolidBrush(th->borderPanel);
        FillRect(hdc, &rcCard, hBrP);
        FrameRect(hdc, &rcCard, hBrB);
        DeleteObject(hBrP);
        DeleteObject(hBrB);

        int tier = g_sub.upgradeBallast;
        const BallastUpgrade* upg = &g_ballastUpg[tier - 1];

        SelectObject(hdc, g_hFontBold);
        SetTextColor(hdc, th->textBright);
        TextOutA(hdc, c2x + 8, r1y + 8, "BALLAST PUMPS", 13);

        SelectObject(hdc, g_hFontSmall);
        SetTextColor(hdc, tier == 4 ? th->accentEmerald : th->accentSonar);
        TextOutA(hdc, c2x + 8, r1y + 24, upg->name, (int)strlen(upg->name));

        SetTextColor(hdc, th->textDim);
        TextOutA(hdc, c2x + 8, r1y + 38, upg->desc, (int)strlen(upg->desc));

        snprintf(buf, sizeof(buf), "MAX AIR: %.0f BAR   RATE: %.0f%%/STEP", upg->maxAir, upg->rate);
        SetTextColor(hdc, th->textPrimary);
        TextOutA(hdc, c2x + 8, r1y + 54, buf, (int)strlen(buf));

        if (tier < 4) {
            const BallastUpgrade* nextUpg = &g_ballastUpg[tier];
            snprintf(buf, sizeof(buf), "UPGRADE -> %s (%d PTS)", nextUpg->name, nextUpg->cost);
            int canAfford = g_sub.surveyPoints >= nextUpg->cost;
            DrawCustomButton(hdc, ID_BTN_UPG_BALLAST, c2x + 8, r1y + gridH - 26, gridW - 16, 20, buf, 0, canAfford ? th->accentSonar : th->textDim, th);
        } else {
            DrawCustomButton(hdc, ID_BTN_UPG_BALLAST, c2x + 8, r1y + gridH - 26, gridW - 16, 20, "MAX TIER [OPTIMAL]", 1, th->accentEmerald, th);
        }
    }

    // Card 3: Nuclear Battery Bank
    {
        RECT rcCard = { c1x, r2y, c1x + gridW, r2y + gridH };
        HBRUSH hBrP = CreateSolidBrush(th->bgPanel);
        HBRUSH hBrB = CreateSolidBrush(th->borderPanel);
        FillRect(hdc, &rcCard, hBrP);
        FrameRect(hdc, &rcCard, hBrB);
        DeleteObject(hBrP);
        DeleteObject(hBrB);

        int tier = g_sub.upgradeBattery;
        const BatteryUpgrade* upg = &g_batteryUpg[tier - 1];

        SelectObject(hdc, g_hFontBold);
        SetTextColor(hdc, th->textBright);
        TextOutA(hdc, c1x + 8, r2y + 8, "NUCLEAR BATTERY BANK", 20);

        SelectObject(hdc, g_hFontSmall);
        SetTextColor(hdc, tier == 4 ? th->accentEmerald : th->accentSonar);
        TextOutA(hdc, c1x + 8, r2y + 24, upg->name, (int)strlen(upg->name));

        SetTextColor(hdc, th->textDim);
        TextOutA(hdc, c1x + 8, r2y + 38, upg->desc, (int)strlen(upg->desc));

        snprintf(buf, sizeof(buf), "DRAIN: -%.0f%%   REGEN: %s", (1.0f - upg->drainMult) * 100.0f, upg->regen > 0.0f ? "+0.2%/s AT DEPTH" : "SURFACE");
        SetTextColor(hdc, th->textPrimary);
        TextOutA(hdc, c1x + 8, r2y + 54, buf, (int)strlen(buf));

        if (tier < 4) {
            const BatteryUpgrade* nextUpg = &g_batteryUpg[tier];
            snprintf(buf, sizeof(buf), "UPGRADE -> %s (%d PTS)", nextUpg->name, nextUpg->cost);
            int canAfford = g_sub.surveyPoints >= nextUpg->cost;
            DrawCustomButton(hdc, ID_BTN_UPG_BATTERY, c1x + 8, r2y + gridH - 26, gridW - 16, 20, buf, 0, canAfford ? th->accentSonar : th->textDim, th);
        } else {
            DrawCustomButton(hdc, ID_BTN_UPG_BATTERY, c1x + 8, r2y + gridH - 26, gridW - 16, 20, "MAX TIER [OPTIMAL]", 1, th->accentEmerald, th);
        }
    }

    // Card 4: Searchlights & Optics
    {
        RECT rcCard = { c2x, r2y, c2x + gridW, r2y + gridH };
        HBRUSH hBrP = CreateSolidBrush(th->bgPanel);
        HBRUSH hBrB = CreateSolidBrush(th->borderPanel);
        FillRect(hdc, &rcCard, hBrP);
        FrameRect(hdc, &rcCard, hBrB);
        DeleteObject(hBrP);
        DeleteObject(hBrB);

        int tier = g_sub.upgradeLights;
        const LightsUpgrade* upg = &g_lightsUpg[tier - 1];

        SelectObject(hdc, g_hFontBold);
        SetTextColor(hdc, th->textBright);
        TextOutA(hdc, c2x + 8, r2y + 8, "SEARCHLIGHTS & OPTICS", 21);

        SelectObject(hdc, g_hFontSmall);
        SetTextColor(hdc, tier == 4 ? th->accentEmerald : th->accentSonar);
        TextOutA(hdc, c2x + 8, r2y + 24, upg->name, (int)strlen(upg->name));

        SetTextColor(hdc, th->textDim);
        TextOutA(hdc, c2x + 8, r2y + 38, upg->desc, (int)strlen(upg->desc));

        snprintf(buf, sizeof(buf), "RANGE: %.0fm   SURVEY: %.2fx PTS", upg->range, upg->surveyMult);
        SetTextColor(hdc, th->textPrimary);
        TextOutA(hdc, c2x + 8, r2y + 54, buf, (int)strlen(buf));

        if (tier < 4) {
            const LightsUpgrade* nextUpg = &g_lightsUpg[tier];
            snprintf(buf, sizeof(buf), "UPGRADE -> %s (%d PTS)", nextUpg->name, nextUpg->cost);
            int canAfford = g_sub.surveyPoints >= nextUpg->cost;
            DrawCustomButton(hdc, ID_BTN_UPG_LIGHTS, c2x + 8, r2y + gridH - 26, gridW - 16, 20, buf, 0, canAfford ? th->accentSonar : th->textDim, th);
        } else {
            DrawCustomButton(hdc, ID_BTN_UPG_LIGHTS, c2x + 8, r2y + gridH - 26, gridW - 16, 20, "MAX TIER [OPTIMAL]", 1, th->accentEmerald, th);
        }
    }
}

void DrawUI(HDC hdc, RECT* rcClient) {
    int clientW = rcClient->right - rcClient->left;
    int clientH = rcClient->bottom - rcClient->top;
    const SubmarineTheme* th = &g_themes[g_sub.currentTheme];

    HBRUSH hBrBg = CreateSolidBrush(th->bgDeep);
    FillRect(hdc, rcClient, hBrBg);
    DeleteObject(hBrBg);

    RECT rcHeader = { 0, 0, clientW, 36 };
    HBRUSH hBrHdr = CreateSolidBrush(th->bgHeader);
    HBRUSH hBrBrd = CreateSolidBrush(th->borderPanel);
    FillRect(hdc, &rcHeader, hBrHdr);
    FrameRect(hdc, &rcHeader, hBrBrd);
    DeleteObject(hBrHdr);
    DeleteObject(hBrBrd);

    SelectObject(hdc, g_hFontTitle);
    SetTextColor(hdc, th->textBright);
    SetBkMode(hdc, TRANSPARENT);
    TextOutA(hdc, 12, 8, "DSV ABYSS VOYAGER // SUB-09", 27);

    SelectObject(hdc, g_hFontBold);
    const char* zoneStr = GetZoneName(g_sub.depth);
    RECT rcZone = { 240, 8, 440, 28 };
    HBRUSH hBrZone = CreateSolidBrush(th->textDim);
    FillRect(hdc, &rcZone, hBrZone);
    DeleteObject(hBrZone);
    SetTextColor(hdc, RGB(255, 255, 255));
    DrawTextA(hdc, zoneStr, -1, &rcZone, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    DrawCustomButton(hdc, ID_BTN_THEME_TOGGLE, clientW - 530, 6, 130, 24, th->tag, 1, th->accentSonar, th);
    DrawCustomButton(hdc, ID_BTN_SCANLINES_TOGGLE, clientW - 392, 6, 116, 24, g_sub.scanlinesEnabled ? "SCANLINES: ON" : "SCANLINES: OFF", g_sub.scanlinesEnabled, th->accentSonar, th);
    DrawCustomButton(hdc, ID_BTN_SOUND_TOGGLE, clientW - 270, 6, 120, 24, g_sub.soundEnabled ? "AUDIO: ON" : "AUDIO: OFF", g_sub.soundEnabled, th->accentSonar, th);
    DrawCustomButton(hdc, ID_BTN_EMERGENCY_BLOW, clientW - 142, 6, 130, 24, "BLOW BALLAST", 0, th->accentRed, th);

    int margin = 8;
    int panelY = 44;
    int panelH = clientH - panelY - margin;
    int leftW = 280;
    int rightW = 290;
    int centerW = clientW - leftW - rightW - (margin * 4);
    int centerX = leftW + (margin * 2);
    int rightX = clientW - rightW - margin;

    const char* statusText = g_sub.hull < 35.0f ? "CRITICAL RISK" : (g_sub.hull < 70.0f || g_sub.hullStress > 80.0f ? "HIGH STRESS" : "NOMINAL");
    COLORREF statusClr = g_sub.hull < 35.0f ? th->accentRed : (g_sub.hull < 70.0f || g_sub.hullStress > 80.0f ? th->accentAmber : th->accentEmerald);
    DrawPanelBox(hdc, margin, panelY, leftW, panelH, "VITAL TELEMETRY", statusText, statusClr, th);

    int gy = panelY + 32;
    int gw = leftW - 20;
    int gx = margin + 10;
    char buf[128];

    SelectObject(hdc, g_hFontSmall);
    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, gx, gy, "DEPTH / VERTICAL RATE", 22);
    snprintf(buf, sizeof(buf), "%.1f m", g_sub.depth);
    SetTextColor(hdc, th->textBright);
    SIZE sz;
    GetTextExtentPoint32A(hdc, buf, (int)strlen(buf), &sz);
    TextOutA(hdc, gx + gw - sz.cx, gy, buf, (int)strlen(buf));
    gy += 15;
    DrawGaugeBar(hdc, gx, gy, gw, 10, min(100.0f, (g_sub.depth / 5000.0f) * 100.0f), th->accentSonar, th);
    gy += 12;
    snprintf(buf, sizeof(buf), "CRUSH: %.0fm   RATE: %+.2f m/s", g_sub.crushDepth, g_sub.vertRate);
    SetTextColor(hdc, g_sub.vertRate > 0.0f ? th->accentAmber : (g_sub.vertRate < 0.0f ? th->accentEmerald : th->textDim));
    TextOutA(hdc, gx, gy, buf, (int)strlen(buf));
    gy += 22;

    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, gx, gy, "HULL INTEGRITY", 14);
    snprintf(buf, sizeof(buf), "%.1f%%", g_sub.hull);
    SetTextColor(hdc, th->textBright);
    GetTextExtentPoint32A(hdc, buf, (int)strlen(buf), &sz);
    TextOutA(hdc, gx + gw - sz.cx, gy, buf, (int)strlen(buf));
    gy += 15;
    COLORREF hullClr = g_sub.hull < 35.0f ? th->accentRed : (g_sub.hull < 70.0f ? th->accentAmber : th->accentSonar);
    DrawGaugeBar(hdc, gx, gy, gw, 10, g_sub.hull, hullClr, th);
    gy += 12;
    snprintf(buf, sizeof(buf), "PRESSURE: %.1f atm   STRESS: %.1f%%", g_sub.pressure, g_sub.hullStress);
    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, gx, gy, buf, (int)strlen(buf));
    gy += 22;

    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, gx, gy, "O2 CONCENTRATION", 16);
    snprintf(buf, sizeof(buf), "%.1f%%", g_sub.o2);
    SetTextColor(hdc, th->textBright);
    GetTextExtentPoint32A(hdc, buf, (int)strlen(buf), &sz);
    TextOutA(hdc, gx + gw - sz.cx, gy, buf, (int)strlen(buf));
    gy += 15;
    COLORREF o2Clr = g_sub.o2 < 40.0f ? th->accentRed : (g_sub.o2 < 70.0f ? th->accentAmber : th->accentSonar);
    DrawGaugeBar(hdc, gx, gy, gw, 10, g_sub.o2, o2Clr, th);
    gy += 12;
    snprintf(buf, sizeof(buf), "CO2: %.2f%%   SCRUBBER: %.0f%%", g_sub.co2, g_sub.scrubberStatus);
    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, gx, gy, buf, (int)strlen(buf));
    gy += 22;

    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, gx, gy, "BATTERY BANK", 12);
    snprintf(buf, sizeof(buf), "%.1f%%", g_sub.battery);
    SetTextColor(hdc, th->textBright);
    GetTextExtentPoint32A(hdc, buf, (int)strlen(buf), &sz);
    TextOutA(hdc, gx + gw - sz.cx, gy, buf, (int)strlen(buf));
    gy += 15;
    COLORREF batClr = g_sub.battery < 20.0f ? th->accentRed : (g_sub.battery < 45.0f ? th->accentAmber : th->accentEmerald);
    DrawGaugeBar(hdc, gx, gy, gw, 10, g_sub.battery, batClr, th);
    gy += 12;
    snprintf(buf, sizeof(buf), "LOAD: %.2f kW   GRID: %s", g_sub.powerDrain, g_sub.depth <= 0.0f ? "SURFACE AUX" : (g_sub.upgradeBattery >= 3 ? "NUCLEAR RTG" : "INTERNAL"));
    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, gx, gy, buf, (int)strlen(buf));
    gy += 22;

    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, gx, gy, "BALLAST FLOOD", 13);
    snprintf(buf, sizeof(buf), "%.0f%% (%s)", g_sub.ballast, g_sub.ballast < 40.0f ? "BUOYANT" : (g_sub.ballast > 55.0f ? "HEAVY" : "NEUTRAL"));
    SetTextColor(hdc, th->textBright);
    GetTextExtentPoint32A(hdc, buf, (int)strlen(buf), &sz);
    TextOutA(hdc, gx + gw - sz.cx, gy, buf, (int)strlen(buf));
    gy += 15;
    DrawGaugeBar(hdc, gx, gy, gw, 10, g_sub.ballast, th->textPrimary, th);
    gy += 12;
    int netBuoy = (int)((45.0f - g_sub.ballast) * 25.0f);
    snprintf(buf, sizeof(buf), "AIR RES: %.0f/%.0f BAR   BUOY: %+d KG", g_sub.airReservoir, g_sub.maxAirReservoir, netBuoy);
    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, gx, gy, buf, (int)strlen(buf));
    gy += 22;

    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, gx, gy, "BILGE ACCUMULATION", 18);
    snprintf(buf, sizeof(buf), "%.1f GAL", g_sub.bilgeWater);
    SetTextColor(hdc, th->textBright);
    GetTextExtentPoint32A(hdc, buf, (int)strlen(buf), &sz);
    TextOutA(hdc, gx + gw - sz.cx, gy, buf, (int)strlen(buf));
    gy += 15;
    DrawGaugeBar(hdc, gx, gy, gw, 10, min(100.0f, g_sub.bilgeWater * 2.5f), th->accentAmber, th);
    gy += 12;
    snprintf(buf, sizeof(buf), "INTRUSION: %.1f GPM  PUMP: %s", g_sub.waterIntrusionRate, g_sub.bilgePumpActive ? "ACTIVE" : "STANDBY");
    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, gx, gy, buf, (int)strlen(buf));
    gy += 24;

    snprintf(buf, sizeof(buf), "PITCH TRIM: %+.1f deg (%s)", g_sub.pitch, g_sub.pitch == 0.0f ? "LEVEL" : (g_sub.pitch > 0.0f ? "STERN DOWN" : "BOW DOWN"));
    SetTextColor(hdc, th->textBright);
    TextOutA(hdc, gx, gy, buf, (int)strlen(buf));
    gy += 16;
    snprintf(buf, sizeof(buf), "SEAWATER TEMP: %.1f deg C", g_sub.temp);
    SetTextColor(hdc, g_sub.temp > 50.0f ? th->accentRed : (g_sub.temp > 25.0f ? th->accentAmber : th->accentSonar));
    TextOutA(hdc, gx, gy, buf, (int)strlen(buf));

    // Center Stage Panels
    int sonarH = (panelH * 60) / 100;
    int logH = panelH - sonarH - 8;
    int logY = panelY + sonarH + 8;

    char secTag[64];
    snprintf(secTag, sizeof(secTag), "SECTOR: %s | SURVEY: %d PTS", g_sectors[g_sub.currentSectorIdx].name, g_sub.surveyPoints);
    DrawPanelBox(hdc, centerX, panelY, centerW, sonarH, "DEEP OCEAN & FAUNA EXPLORATION", secTag, th->accentEmerald, th);

    // View switch buttons inside center panel header
    int btnViewW = (centerW - 140) / 4;
    DrawCustomButton(hdc, ID_BTN_VIEW_SONAR, centerX + 8, panelY + 28, btnViewW - 2, 20, "SONAR RADAR", g_sub.viewMode == 0, th->accentSonar, th);
    DrawCustomButton(hdc, ID_BTN_VIEW_NAVMAP, centerX + 6 + btnViewW, panelY + 28, btnViewW - 2, 20, "TRENCH NAV", g_sub.viewMode == 1, th->accentSonar, th);
    DrawCustomButton(hdc, ID_BTN_VIEW_CODEX, centerX + 4 + btnViewW * 2, panelY + 28, btnViewW - 2, 20, "FAUNA CODEX", g_sub.viewMode == 2, th->accentSonar, th);
    DrawCustomButton(hdc, ID_BTN_VIEW_ENG, centerX + 2 + btnViewW * 3, panelY + 28, btnViewW - 2, 20, "ENGINEERING", g_sub.viewMode == 3, th->accentSonar, th);
    DrawCustomButton(hdc, ID_BTN_FIELD_DIAG, centerX + centerW - 128, panelY + 28, 120, 20, "+35 PTS DIAG", 0, th->accentAmber, th);

    int sonarContentY = panelY + 52;
    int sonarContentH = sonarH - 58;
    int scx = centerX + centerW / 2;
    int scy = sonarContentY + sonarContentH / 2;
    int sRadius = min(centerW, sonarContentH) / 2 - 12;

    if (g_sub.viewMode == 0) {
        RECT rcRadar = { scx - sRadius - 10, scy - sRadius - 10, scx + sRadius + 10, scy + sRadius + 10 };
        HBRUSH hBrRadar = CreateSolidBrush(th->radarBg);
        FillRect(hdc, &rcRadar, hBrRadar);
        DeleteObject(hBrRadar);

        HPEN hPenRing = CreatePen(PS_SOLID, 1, th->radarRing);
        HPEN hPenOld = (HPEN)SelectObject(hdc, hPenRing);
        HBRUSH hBrNull = (HBRUSH)GetStockObject(NULL_BRUSH);
        HBRUSH hBrOld = (HBRUSH)SelectObject(hdc, hBrNull);

        for (int r = 1; r <= 4; r++) {
            int curR = (sRadius * r) / 4;
            Ellipse(hdc, scx - curR, scy - curR, scx + curR, scy + curR);
        }
        MoveToEx(hdc, scx - sRadius, scy, NULL);
        LineTo(hdc, scx + sRadius, scy);
        MoveToEx(hdc, scx, scy - sRadius, NULL);
        LineTo(hdc, scx, scy + sRadius);

        HPEN hPenSweep = CreatePen(PS_SOLID, 2, th->accentSonar);
        SelectObject(hdc, hPenSweep);
        int sx = scx + (int)(cosf(g_sub.sweepAngle) * sRadius);
        int sy = scy + (int)(sinf(g_sub.sweepAngle) * sRadius);
        MoveToEx(hdc, scx, scy, NULL);
        LineTo(hdc, sx, sy);
        DeleteObject(hPenSweep);

        if (g_sub.isPinging && g_sub.pingRadius > 0.0f) {
            HPEN hPenPing = CreatePen(PS_SOLID, 2, th->accentSonar);
            SelectObject(hdc, hPenPing);
            int pr = (int)min((float)sRadius, g_sub.pingRadius);
            Ellipse(hdc, scx - pr, scy - pr, scx + pr, scy + pr);
            DeleteObject(hPenPing);
        }

        HBRUSH hBrSub = CreateSolidBrush(th->accentEmerald);
        SelectObject(hdc, hBrSub);
        Ellipse(hdc, scx - 4, scy - 4, scx + 4, scy + 4);
        DeleteObject(hBrSub);

        float hRad = (g_sub.heading - 90.0f) * (3.14159265f / 180.0f);
        HPEN hPenHeading = CreatePen(PS_SOLID, 2, th->accentEmerald);
        SelectObject(hdc, hPenHeading);
        MoveToEx(hdc, scx, scy, NULL);
        LineTo(hdc, scx + (int)(cosf(hRad) * 16), scy + (int)(sinf(hRad) * 16));
        DeleteObject(hPenHeading);

        // Sonar Contacts from Current Sector Fauna (Phase 7)
        int secFauna[3];
        int fCount = GetSectorFaunaIndices(g_sub.currentSectorIdx, secFauna);

        SelectObject(hdc, g_hFontSmall);
        for (int i = 0; i < fCount; i++) {
            FaunaAnomaly* f = &g_fauna[secFauna[i]];
            float relX = (f->x - g_sub.posX) / 2.0f;
            float relY = (f->y - g_sub.posY) / 2.0f;
            float distNorm = sqrtf(relX * relX + relY * relY);

            if (distNorm > 1.05f) continue;

            int fcx = scx + (int)(relX * sRadius);
            int fcy = scy + (int)(relY * sRadius);

            float angleToC = fmodf(atan2f(relY, relX) + 6.2831853f, 6.2831853f);
            float angleDiff = fabsf(g_sub.sweepAngle - angleToC);
            int isSwept = (angleDiff < 0.25f) || (g_sub.isPinging && fabsf(g_sub.pingRadius - sRadius * distNorm) < 20.0f);
            int isSelected = (i == g_sub.selectedTargetIdx);

            COLORREF cClr = th->accentSonar;
            if (f->type == 4) cClr = RGB(244, 63, 94); // Leviathan
            else if (f->type == 1) cClr = RGB(244, 114, 182); // Squid
            else if (f->type == 2) cClr = th->accentEmerald; // Flora
            else if (f->type == 3) cClr = th->accentAmber; // Trench

            HBRUSH hBrContact = CreateSolidBrush(isSwept || isSelected ? cClr : th->radarRing);
            SelectObject(hdc, hBrContact);
            int dotRad = (f->type == 4 ? 6 : (f->type == 1 ? 4 : 3));
            Ellipse(hdc, fcx - dotRad, fcy - dotRad, fcx + dotRad, fcy + dotRad);
            DeleteObject(hBrContact);

            if (isSwept || isSelected) {
                SetTextColor(hdc, th->textBright);
                const char* dName = f->discovered ? f->name : (f->type == 4 ? "[LEVIATHAN]" : (f->type == 1 ? "[SQUID]" : (f->type == 2 ? "[FLORA]" : "[FAUNA]")));
                TextOutA(hdc, fcx + 8, fcy - 6, dName, (int)strlen(dName));
                snprintf(buf, sizeof(buf), "%.0fm", distNorm * 2000.0f);
                SetTextColor(hdc, th->textDim);
                TextOutA(hdc, fcx + 8, fcy + 4, buf, (int)strlen(buf));
            }

            // Targeting Reticle for Locked Target
            if (isSelected) {
                HPEN hPenReticle = CreatePen(PS_SOLID, 1, th->textPrimary);
                SelectObject(hdc, hPenReticle);
                int bs = 8;
                MoveToEx(hdc, fcx - bs, fcy - bs + 3, NULL); LineTo(hdc, fcx - bs, fcy - bs); LineTo(hdc, fcx - bs + 3, fcy - bs);
                MoveToEx(hdc, fcx + bs - 3, fcy - bs, NULL); LineTo(hdc, fcx + bs, fcy - bs); LineTo(hdc, fcx + bs, fcy - bs + 3);
                MoveToEx(hdc, fcx - bs, fcy + bs - 3, NULL); LineTo(hdc, fcx - bs, fcy + bs); LineTo(hdc, fcx - bs + 3, fcy + bs);
                MoveToEx(hdc, fcx + bs - 3, fcy + bs, NULL); LineTo(hdc, fcx + bs, fcy + bs); LineTo(hdc, fcx + bs, fcy + bs - 3);

                // Dotted course line from center
                HPEN hPenVec = CreatePen(PS_DOT, 1, th->textPrimary);
                SelectObject(hdc, hPenVec);
                MoveToEx(hdc, scx, scy, NULL);
                LineTo(hdc, fcx, fcy);
                DeleteObject(hPenVec);
                DeleteObject(hPenReticle);
            }
        }

        // Top-Right Target HUD Box in Sonar View
        if (fCount > 0 && g_sub.selectedTargetIdx < fCount) {
            FaunaAnomaly* tgt = &g_fauna[secFauna[g_sub.selectedTargetIdx]];
            float dx = tgt->x - g_sub.posX;
            float dy = tgt->y - g_sub.posY;
            float distM = sqrtf(dx * dx + dy * dy) * 1000.0f;
            float depthDiff = fabsf(g_sub.depth - tgt->depth);

            int hudW = 180;
            int hudH = 80;
            int hudX = centerX + centerW - hudW - 12;
            int hudY = sonarContentY + 8;

            RECT rcHud = { hudX, hudY, hudX + hudW, hudY + hudH };
            HBRUSH hBrHud = CreateSolidBrush(RGB(7, 23, 36));
            HBRUSH hBrHdrB = CreateSolidBrush(th->borderPanel);
            FillRect(hdc, &rcHud, hBrHud);
            FrameRect(hdc, &rcHud, hBrHdrB);
            DeleteObject(hBrHud);
            DeleteObject(hBrHdrB);

            SelectObject(hdc, g_hFontSmall);
            SetTextColor(hdc, th->accentEmerald);
            TextOutA(hdc, hudX + 6, hudY + 4, "🎯 TARGET TRACKER", 17);

            SetTextColor(hdc, th->textBright);
            snprintf(buf, sizeof(buf), "%.16s", tgt->discovered ? tgt->name : tgt->id);
            TextOutA(hdc, hudX + 6, hudY + 18, buf, (int)strlen(buf));

            SetTextColor(hdc, th->textDim);
            snprintf(buf, sizeof(buf), "Dist: %.0fm  ΔZ: %.0fm", distM, depthDiff);
            TextOutA(hdc, hudX + 6, hudY + 32, buf, (int)strlen(buf));

            snprintf(buf, sizeof(buf), "Lumens: %.12s", tgt->lumens);
            TextOutA(hdc, hudX + 6, hudY + 46, buf, (int)strlen(buf));

            if (g_sub.isScanningTarget) {
                DrawGaugeBar(hdc, hudX + 6, hudY + 62, hudW - 12, 8, g_sub.scanProgress, th->accentEmerald, th);
            } else {
                DrawCustomButton(hdc, ID_BTN_HUD_NEXT_TARGET, hudX + 6, hudY + 58, (hudW - 16) / 2, 18, "NEXT", 0, th->textPrimary, th);
                DrawCustomButton(hdc, ID_BTN_HUD_SCAN_TARGET, hudX + 8 + (hudW - 16) / 2, hudY + 58, (hudW - 16) / 2, 18, "SCAN", 0, th->accentEmerald, th);
            }
        }

        SelectObject(hdc, hPenOld);
        SelectObject(hdc, hBrOld);
        DeleteObject(hPenRing);
    } else if (g_sub.viewMode == 1) {
        DrawNavMapChart(hdc, scx, scy, centerW - 20, sonarContentH - 8, th);
    } else if (g_sub.viewMode == 2) {
        DrawFaunaCodex(hdc, centerX + 6, sonarContentY + 4, centerW - 12, sonarContentH - 8, th);
    } else {
        DrawEngineeringBay(hdc, centerX + 6, sonarContentY + 4, centerW - 12, sonarContentH - 8, th);
    }

    SelectObject(hdc, g_hFontSmall);
    SetTextColor(hdc, th->textPrimary);
    const NavWaypoint* curWp = &g_waypoints[g_sub.activeWaypointIdx];
    float wdx = curWp->x - g_sub.posX;
    float wdy = curWp->y - g_sub.posY;
    float wDist = sqrtf(wdx * wdx + wdy * wdy);
    snprintf(buf, sizeof(buf), "COORDS: X:%+.2fkm Y:%+.2fkm | SEABED: %.0fM (DIST: %.0fm) | WP-%d: %s (%.2fkm)",
             g_sub.posX, g_sub.posY, g_sub.seabedElevation, max(0.0f, g_sub.seabedElevation - g_sub.depth),
             g_sub.activeWaypointIdx + 1, curWp->name, wDist);
    TextOutA(hdc, centerX + 10, sonarH + 16, buf, (int)strlen(buf));

    DrawPanelBox(hdc, centerX, logY, centerW, logH, "SYSTEM LOG & TELEMETRY STREAM", "LIVE", th->accentSonar, th);
    int logLineY = logY + 28;
    for (int i = 0; i < g_sub.logCount; i++) {
        LogEntry* e = &g_sub.logs[i];
        SetTextColor(hdc, th->textDim);
        TextOutA(hdc, centerX + 10, logLineY, e->time, (int)strlen(e->time));
        SetTextColor(hdc, e->color);
        TextOutA(hdc, centerX + 80, logLineY, e->text, (int)strlen(e->text));
        logLineY += 15;
        if (logLineY > logY + logH - 16) break;
    }

    DrawPanelBox(hdc, rightX, panelY, rightW, panelH, "HELM & NAVIGATION", "CMD-CTRL", th->accentSonar, th);

    int cy = panelY + 30;
    int bw = (rightW - 24) / 2;
    int bx = rightX + 8;

    // SONAR TARGET SCANNER (PHASE 7)
    SelectObject(hdc, g_hFontSmall);
    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, bx, cy, "SONAR TARGET TRACKER & BIO-SCAN", 31);
    cy += 14;

    int secFauna[3];
    int fCount = GetSectorFaunaIndices(g_sub.currentSectorIdx, secFauna);
    if (fCount > 0 && g_sub.selectedTargetIdx < fCount) {
        FaunaAnomaly* tgt = &g_fauna[secFauna[g_sub.selectedTargetIdx]];
        snprintf(buf, sizeof(buf), "🎯 LOCKED: %.14s", tgt->discovered ? tgt->name : tgt->id);
    } else {
        snprintf(buf, sizeof(buf), "🎯 TARGET LOCK: NONE");
    }
    DrawCustomButton(hdc, ID_BTN_LOCK_TARGET, bx, cy, rightW - 18, 20, buf, 0, th->accentSonar, th);
    cy += 22;

    DrawCustomButton(hdc, ID_BTN_SCAN_TARGET, bx, cy, rightW - 18, 20, "🔬 BIO-ACOUSTIC SCAN TARGET", g_sub.isScanningTarget, th->accentEmerald, th);
    cy += 26;

    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, bx, cy, "OCEAN NAVIGATION & WAYPOINTS", 28);
    cy += 14;

    snprintf(buf, sizeof(buf), "WAYPOINT [WP-%d]: %.10s", g_sub.activeWaypointIdx + 1, g_waypoints[g_sub.activeWaypointIdx].name);
    DrawCustomButton(hdc, ID_BTN_NEXT_WAYPOINT, bx, cy, rightW - 18, 20, buf, 0, th->accentSonar, th);
    cy += 22;

    DrawCustomButton(hdc, ID_BTN_AUTOPILOT, bx, cy, bw, 20, g_sub.autopilot ? "AUTOPILOT: ON" : "AUTOPILOT: OFF", g_sub.autopilot, th->accentEmerald, th);
    DrawCustomButton(hdc, ID_BTN_SURVEY_SECTOR, bx + bw + 6, cy, bw, 20, "SURVEY REGION", 0, th->textPrimary, th);
    cy += 24;

    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, bx, cy, "BALLAST DIVE ENGINE", 19);
    cy += 14;

    DrawCustomButton(hdc, ID_BTN_FLOOD_BALLAST, bx, cy, bw, 20, "FLOOD BALLAST (+)", 0, th->textPrimary, th);
    DrawCustomButton(hdc, ID_BTN_BLOW_BALLAST, bx + bw + 6, cy, bw, 20, "BLOW BALLAST (-)", 0, th->accentEmerald, th);
    cy += 22;

    DrawCustomButton(hdc, ID_BTN_TRIM_BOW, bx, cy, bw, 20, "TRIM BOW (-1 deg)", 0, th->textPrimary, th);
    DrawCustomButton(hdc, ID_BTN_TRIM_STERN, bx + bw + 6, cy, bw, 20, "TRIM STERN (+1 deg)", 0, th->textPrimary, th);
    cy += 22;

    DrawCustomButton(hdc, ID_BTN_SONAR_PING, bx, cy, rightW - 18, 22, "ACOUSTIC SONAR PING", g_sub.isPinging, th->accentSonar, th);
    cy += 26;

    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, bx, cy, "PROPULSION THROTTLE", 19);
    cy += 14;

    int bw3 = (rightW - 28) / 3;
    DrawCustomButton(hdc, ID_BTN_THROTTLE_REV, bx, cy, bw3, 20, "REV", g_sub.throttleMode == 0, th->accentAmber, th);
    DrawCustomButton(hdc, ID_BTN_THROTTLE_STOP, bx + bw3 + 4, cy, bw3, 20, "STOP", g_sub.throttleMode == 1, th->accentSonar, th);
    DrawCustomButton(hdc, ID_BTN_THROTTLE_HALF, bx + (bw3 + 4) * 2, cy, bw3, 20, "HALF", g_sub.throttleMode == 2, th->accentSonar, th);
    cy += 22;

    DrawCustomButton(hdc, ID_BTN_THROTTLE_FLANK, bx, cy, rightW - 18, 20, "FLANK SPEED (FULL AHEAD)", g_sub.throttleMode == 3, th->accentAmber, th);
    cy += 22;

    DrawCustomButton(hdc, ID_BTN_RUDDER_PORT, bx, cy, bw, 20, "< RUDDER PORT", 0, th->textPrimary, th);
    DrawCustomButton(hdc, ID_BTN_RUDDER_STBD, bx + bw + 6, cy, bw, 20, "RUDDER STBD >", 0, th->textPrimary, th);
    cy += 24;

    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, bx, cy, "SUBSYSTEM MANAGEMENT", 20);
    cy += 14;

    snprintf(buf, sizeof(buf), "SEARCHLIGHTS: %s", g_sub.searchlights ? "ENGAGED [HIGH LUX]" : "OFF");
    DrawCustomButton(hdc, ID_BTN_SEARCHLIGHTS, bx, cy, rightW - 18, 18, buf, g_sub.searchlights, th->accentSonar, th);
    cy += 20;

    snprintf(buf, sizeof(buf), "O2 SCRUBBER: %s", g_sub.scrubberAuto ? "AUTO [ONLINE]" : "MANUAL [STANDBY]");
    DrawCustomButton(hdc, ID_BTN_SCRUBBER, bx, cy, rightW - 18, 18, buf, g_sub.scrubberAuto, th->accentEmerald, th);
    cy += 20;

    snprintf(buf, sizeof(buf), "PURGE EMERGENCY O2 (%d LEFT)", g_sub.o2PurgeCount);
    DrawCustomButton(hdc, ID_BTN_O2_PURGE, bx, cy, rightW - 18, 18, buf, 0, th->textPrimary, th);
    cy += 20;

    snprintf(buf, sizeof(buf), "BILGE PUMPS: %s", g_sub.bilgePumpActive ? "RUNNING [MAX]" : "AUTO (STANDBY)");
    DrawCustomButton(hdc, ID_BTN_BILGE_PUMP, bx, cy, rightW - 18, 18, buf, g_sub.bilgePumpActive, th->accentAmber, th);
    cy += 20;

    snprintf(buf, sizeof(buf), "ECO LOW-POWER: %s", g_sub.lowPowerMode ? "ACTIVE" : "OFF");
    DrawCustomButton(hdc, ID_BTN_LOW_POWER, bx, cy, rightW - 18, 18, buf, g_sub.lowPowerMode, th->accentEmerald, th);
    cy += 22;

    RECT rcDirect = { bx, cy, rightX + rightW - 10, panelY + panelH - 8 };
    HBRUSH hBrDirect = CreateSolidBrush(th->bgDeep);
    FillRect(hdc, &rcDirect, hBrDirect);
    FrameRect(hdc, &rcDirect, hBrBrd);
    DeleteObject(hBrDirect);
    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, bx + 6, cy + 2, "DIRECTIVES:", 11);
    SetTextColor(hdc, th->textBright);
    TextOutA(hdc, bx + 6, cy + 15, "- Detect Leviathans & Squids", 28);
    TextOutA(hdc, bx + 6, cy + 28, "- Catalog 12/12 Fauna Codex", 27);
    TextOutA(hdc, bx + 6, cy + 41, "- Dive Hadal Trench (4000m+)", 28);

    // Deep-water CRT scanlines raster overlay
    if (g_sub.scanlinesEnabled) {
        HPEN hPenScan = CreatePen(PS_SOLID, 1, th->scanlineClr);
        HPEN hPenOldScan = (HPEN)SelectObject(hdc, hPenScan);
        for (int sy = 0; sy < clientH; sy += 3) {
            MoveToEx(hdc, 0, sy, NULL);
            LineTo(hdc, clientW, sy);
        }
        SelectObject(hdc, hPenOldScan);
        DeleteObject(hPenScan);
    }
}

int HitTestButton(int mx, int my, int clientW, int clientH) {
    int margin = 8;
    int panelY = 44;
    int panelH = clientH - panelY - margin;
    int leftW = 280;
    int rightW = 290;
    int centerW = clientW - leftW - rightW - (margin * 4);
    int centerX = leftW + (margin * 2);
    int rightX = clientW - rightW - margin;

    if (my >= 6 && my <= 30) {
        if (mx >= clientW - 530 && mx <= clientW - 400) return ID_BTN_THEME_TOGGLE;
        if (mx >= clientW - 392 && mx <= clientW - 276) return ID_BTN_SCANLINES_TOGGLE;
        if (mx >= clientW - 270 && mx <= clientW - 150) return ID_BTN_SOUND_TOGGLE;
        if (mx >= clientW - 142 && mx <= clientW - 12) return ID_BTN_EMERGENCY_BLOW;
    }

    // View toggles in center panel
    if (my >= panelY + 28 && my <= panelY + 48) {
        int btnViewW = (centerW - 140) / 4;
        if (mx >= centerX + 8 && mx <= centerX + 8 + btnViewW - 2) return ID_BTN_VIEW_SONAR;
        if (mx >= centerX + 6 + btnViewW && mx <= centerX + 6 + btnViewW * 2 - 2) return ID_BTN_VIEW_NAVMAP;
        if (mx >= centerX + 4 + btnViewW * 2 && mx <= centerX + 4 + btnViewW * 3 - 2) return ID_BTN_VIEW_CODEX;
        if (mx >= centerX + 2 + btnViewW * 3 && mx <= centerX + 2 + btnViewW * 4 - 2) return ID_BTN_VIEW_ENG;
        if (mx >= centerX + centerW - 128 && mx <= centerX + centerW - 8) return ID_BTN_FIELD_DIAG;
    }

    // HUD buttons in Sonar view
    if (g_sub.viewMode == 0) {
        int sonarContentY = panelY + 52;
        int hudW = 180;
        int hudX = centerX + centerW - hudW - 12;
        int hudY = sonarContentY + 8;
        if (my >= hudY + 58 && my <= hudY + 76) {
            if (mx >= hudX + 6 && mx <= hudX + 6 + (hudW - 16) / 2) return ID_BTN_HUD_NEXT_TARGET;
            if (mx >= hudX + 8 + (hudW - 16) / 2 && mx <= hudX + hudW - 8) return ID_BTN_HUD_SCAN_TARGET;
        }
    }

    if (g_sub.viewMode == 3) { // Engineering Bay
        int sonarH = (panelH * 60) / 100;
        int sonarContentY = panelY + 52;
        int sonarContentH = sonarH - 58;
        int ebX = centerX + 6;
        int ebY = sonarContentY + 4;
        int ebW = centerW - 12;

        int marginEb = 6;
        int gridW = (ebW - marginEb * 3) / 2;
        int gridH = (sonarContentH - 8 - marginEb * 3) / 2;
        int c1x = ebX + marginEb;
        int c2x = ebX + marginEb * 2 + gridW;
        int r1y = ebY + marginEb;
        int r2y = ebY + marginEb * 2 + gridH;

        if (my >= r1y + gridH - 26 && my <= r1y + gridH - 6) {
            if (mx >= c1x + 8 && mx <= c1x + gridW - 8) return ID_BTN_UPG_HULL;
            if (mx >= c2x + 8 && mx <= c2x + gridW - 8) return ID_BTN_UPG_BALLAST;
        }
        if (my >= r2y + gridH - 26 && my <= r2y + gridH - 6) {
            if (mx >= c1x + 8 && mx <= c1x + gridW - 8) return ID_BTN_UPG_BATTERY;
            if (mx >= c2x + 8 && mx <= c2x + gridW - 8) return ID_BTN_UPG_LIGHTS;
        }
    }

    int cy = panelY + 30;
    int bw = (rightW - 24) / 2;
    int bx = rightX + 8;
    cy += 14;

    // SONAR TARGET TRACKER
    if (my >= cy && my <= cy + 20 && mx >= bx && mx <= bx + rightW - 18) return ID_BTN_LOCK_TARGET;
    cy += 22;
    if (my >= cy && my <= cy + 20 && mx >= bx && mx <= bx + rightW - 18) return ID_BTN_SCAN_TARGET;
    cy += 26 + 14;

    if (my >= cy && my <= cy + 20 && mx >= bx && mx <= bx + rightW - 18) return ID_BTN_NEXT_WAYPOINT;
    cy += 22;

    if (my >= cy && my <= cy + 20) {
        if (mx >= bx && mx <= bx + bw) return ID_BTN_AUTOPILOT;
        if (mx >= bx + bw + 6 && mx <= bx + bw * 2 + 6) return ID_BTN_SURVEY_SECTOR;
    }
    cy += 24 + 14;

    if (my >= cy && my <= cy + 20) {
        if (mx >= bx && mx <= bx + bw) return ID_BTN_FLOOD_BALLAST;
        if (mx >= bx + bw + 6 && mx <= bx + bw * 2 + 6) return ID_BTN_BLOW_BALLAST;
    }
    cy += 22;

    if (my >= cy && my <= cy + 20) {
        if (mx >= bx && mx <= bx + bw) return ID_BTN_TRIM_BOW;
        if (mx >= bx + bw + 6 && mx <= bx + bw * 2 + 6) return ID_BTN_TRIM_STERN;
    }
    cy += 22;

    if (my >= cy && my <= cy + 22 && mx >= bx && mx <= bx + rightW - 18) {
        return ID_BTN_SONAR_PING;
    }
    cy += 26 + 14;

    int bw3 = (rightW - 28) / 3;
    if (my >= cy && my <= cy + 20) {
        if (mx >= bx && mx <= bx + bw3) return ID_BTN_THROTTLE_REV;
        if (mx >= bx + bw3 + 4 && mx <= bx + bw3 * 2 + 4) return ID_BTN_THROTTLE_STOP;
        if (mx >= bx + (bw3 + 4) * 2 && mx <= bx + (bw3 + 4) * 3) return ID_BTN_THROTTLE_HALF;
    }
    cy += 22;

    if (my >= cy && my <= cy + 20 && mx >= bx && mx <= bx + rightW - 18) {
        return ID_BTN_THROTTLE_FLANK;
    }
    cy += 22;

    if (my >= cy && my <= cy + 20) {
        if (mx >= bx && mx <= bx + bw) return ID_BTN_RUDDER_PORT;
        if (mx >= bx + bw + 6 && mx <= bx + bw * 2 + 6) return ID_BTN_RUDDER_STBD;
    }
    cy += 24 + 14;

    if (my >= cy && my <= cy + 18 && mx >= bx && mx <= bx + rightW - 18) return ID_BTN_SEARCHLIGHTS;
    cy += 20;
    if (my >= cy && my <= cy + 18 && mx >= bx && mx <= bx + rightW - 18) return ID_BTN_SCRUBBER;
    cy += 20;
    if (my >= cy && my <= cy + 18 && mx >= bx && mx <= bx + rightW - 18) return ID_BTN_O2_PURGE;
    cy += 20;
    if (my >= cy && my <= cy + 18 && mx >= bx && mx <= bx + rightW - 18) return ID_BTN_BILGE_PUMP;
    cy += 20;
    if (my >= cy && my <= cy + 18 && mx >= bx && mx <= bx + rightW - 18) return ID_BTN_LOW_POWER;

    return 0;
}

void HandleCommand(int cmdId) {
    char msg[128];
    const SubmarineTheme* th = &g_themes[g_sub.currentTheme];

    switch (cmdId) {
        case ID_BTN_THEME_TOGGLE: {
            g_sub.currentTheme = (g_sub.currentTheme + 1) % THEME_COUNT;
            const SubmarineTheme* newTh = &g_themes[g_sub.currentTheme];
            PlaySoundAsync(420, 80);
            snprintf(msg, sizeof(msg), "Oceanic CRT phosphor calibrated: %s.", newTh->name);
            AddLog(msg, newTh->accentSonar);
            break;
        }

        case ID_BTN_SCANLINES_TOGGLE: {
            g_sub.scanlinesEnabled = !g_sub.scanlinesEnabled;
            PlaySoundAsync(g_sub.scanlinesEnabled ? 520 : 320, 80);
            snprintf(msg, sizeof(msg), "Deep-water CRT scanline raster %s.", g_sub.scanlinesEnabled ? "ENGAGED" : "BYPASSED");
            AddLog(msg, th->textDim);
            break;
        }

        case ID_BTN_VIEW_SONAR:
            g_sub.viewMode = 0;
            PlaySoundAsync(450, 60);
            break;

        case ID_BTN_VIEW_NAVMAP:
            g_sub.viewMode = 1;
            PlaySoundAsync(520, 60);
            break;

        case ID_BTN_VIEW_CODEX:
            g_sub.viewMode = 2;
            PlaySoundAsync(620, 80);
            break;

        case ID_BTN_VIEW_ENG:
            g_sub.viewMode = 3;
            PlaySoundAsync(580, 80);
            break;

        case ID_BTN_LOCK_TARGET:
        case ID_BTN_HUD_NEXT_TARGET: {
            int secFauna[3];
            int fCount = GetSectorFaunaIndices(g_sub.currentSectorIdx, secFauna);
            if (fCount > 0) {
                g_sub.selectedTargetIdx = (g_sub.selectedTargetIdx + 1) % fCount;
                FaunaAnomaly* tgt = &g_fauna[secFauna[g_sub.selectedTargetIdx]];
                PlaySoundAsync(700, 80);
                snprintf(msg, sizeof(msg), "Acoustic tracking locked onto target [%s].", tgt->discovered ? tgt->name : tgt->id);
                AddLog(msg, th->accentSonar);
            }
            break;
        }

        case ID_BTN_SCAN_TARGET:
        case ID_BTN_HUD_SCAN_TARGET: {
            int secFauna[3];
            int fCount = GetSectorFaunaIndices(g_sub.currentSectorIdx, secFauna);
            if (fCount > 0 && g_sub.selectedTargetIdx < fCount) {
                FaunaAnomaly* tgt = &g_fauna[secFauna[g_sub.selectedTargetIdx]];
                float dx = tgt->x - g_sub.posX;
                float dy = tgt->y - g_sub.posY;
                float distM = sqrtf(dx * dx + dy * dy) * 1000.0f;
                float depthDiff = fabsf(g_sub.depth - tgt->depth);
                float maxRange = max(900.0f, g_sub.opticalRange * 1.8f);

                if (distM <= maxRange && depthDiff <= 450.0f) {
                    if (!g_sub.isScanningTarget) {
                        g_sub.isScanningTarget = 1;
                        g_sub.scanProgress = 0.0f;
                        PlaySoundAsync(1200, 150);
                        snprintf(msg, sizeof(msg), "Bio-acoustic hydrophone scan initiated on [%s]...", tgt->name);
                        AddLog(msg, th->textPrimary);
                    }
                } else {
                    PlaySoundAsync(220, 150);
                    snprintf(msg, sizeof(msg), "TARGET OUT OF RANGE: %.0fm away (Depth diff: %.0fm). Steer closer!", distM, depthDiff);
                    AddLog(msg, th->accentAmber);
                }
            }
            break;
        }

        case ID_BTN_FIELD_DIAG:
            g_sub.surveyPoints += 35;
            PlaySoundAsync(640, 100);
            AddLog("Submersible telemetry calibrated (+35 Research Credits).", th->textPrimary);
            break;

        case ID_BTN_UPG_HULL:
            if (g_sub.upgradeHull < 4) {
                const HullUpgrade* nextUpg = &g_hullUpg[g_sub.upgradeHull];
                if (g_sub.surveyPoints >= nextUpg->cost) {
                    g_sub.surveyPoints -= nextUpg->cost;
                    g_sub.upgradeHull++;
                    g_sub.crushDepth = nextUpg->crushDepth;
                    PlaySoundAsync(920, 150);
                    snprintf(msg, sizeof(msg), "ENGINEERING UPGRADE: [%s] installed! Crush depth: %.0fm", nextUpg->name, nextUpg->crushDepth);
                    AddLog(msg, th->accentEmerald);
                }
            }
            break;

        case ID_BTN_UPG_BALLAST:
            if (g_sub.upgradeBallast < 4) {
                const BallastUpgrade* nextUpg = &g_ballastUpg[g_sub.upgradeBallast];
                if (g_sub.surveyPoints >= nextUpg->cost) {
                    g_sub.surveyPoints -= nextUpg->cost;
                    g_sub.upgradeBallast++;
                    g_sub.maxAirReservoir = nextUpg->maxAir;
                    g_sub.ballastStepRate = nextUpg->rate;
                    g_sub.airRechargeRate = nextUpg->recharge;
                    PlaySoundAsync(920, 150);
                    snprintf(msg, sizeof(msg), "ENGINEERING UPGRADE: [%s] installed! Max air: %.0f BAR", nextUpg->name, nextUpg->maxAir);
                    AddLog(msg, th->accentEmerald);
                }
            }
            break;

        case ID_BTN_UPG_BATTERY:
            if (g_sub.upgradeBattery < 4) {
                const BatteryUpgrade* nextUpg = &g_batteryUpg[g_sub.upgradeBattery];
                if (g_sub.surveyPoints >= nextUpg->cost) {
                    g_sub.surveyPoints -= nextUpg->cost;
                    g_sub.upgradeBattery++;
                    g_sub.powerDrainMult = nextUpg->drainMult;
                    g_sub.passiveBatteryRegen = nextUpg->regen;
                    PlaySoundAsync(920, 150);
                    snprintf(msg, sizeof(msg), "ENGINEERING UPGRADE: [%s] installed! Power drain reduced.", nextUpg->name);
                    AddLog(msg, th->accentEmerald);
                }
            }
            break;

        case ID_BTN_UPG_LIGHTS:
            if (g_sub.upgradeLights < 4) {
                const LightsUpgrade* nextUpg = &g_lightsUpg[g_sub.upgradeLights];
                if (g_sub.surveyPoints >= nextUpg->cost) {
                    g_sub.surveyPoints -= nextUpg->cost;
                    g_sub.upgradeLights++;
                    g_sub.opticalRange = nextUpg->range;
                    g_sub.surveyMultiplier = nextUpg->surveyMult;
                    PlaySoundAsync(920, 150);
                    snprintf(msg, sizeof(msg), "ENGINEERING UPGRADE: [%s] installed! Range: %.0fm (%.2fx yield)", nextUpg->name, nextUpg->range, nextUpg->surveyMult);
                    AddLog(msg, th->accentEmerald);
                }
            }
            break;

        case ID_BTN_NEXT_WAYPOINT: {
            g_sub.activeWaypointIdx = (g_sub.activeWaypointIdx + 1) % WAYPOINT_COUNT;
            PlaySoundAsync(500, 80);
            const NavWaypoint* wp = &g_waypoints[g_sub.activeWaypointIdx];
            snprintf(msg, sizeof(msg), "Nav locked to [WP-%d]: %s (Target Depth: %.0fm)", g_sub.activeWaypointIdx + 1, wp->name, wp->targetDepth);
            AddLog(msg, th->accentEmerald);
            break;
        }

        case ID_BTN_AUTOPILOT:
            g_sub.autopilot = !g_sub.autopilot;
            PlaySoundAsync(g_sub.autopilot ? 600 : 350, 100);
            snprintf(msg, sizeof(msg), "Submersible autopilot %s.", g_sub.autopilot ? "ENGAGED" : "DISENGAGED");
            AddLog(msg, th->textPrimary);
            break;

        case ID_BTN_SURVEY_SECTOR: {
            SectorInfo* sec = &g_sectors[g_sub.currentSectorIdx];
            int found = 0;
            for (int i = 0; i < 2; i++) {
                Landmark* lm = &sec->landmarks[i];
                float dx = lm->x - g_sub.posX;
                float dy = lm->y - g_sub.posY;
                float distKm = sqrtf(dx * dx + dy * dy);
                float depthDiff = fabsf(g_sub.depth - lm->depth);
                float maxRangeKm = max(1.5f, (g_sub.opticalRange / 1000.0f) * 1.6f);

                if (distKm <= maxRangeKm && depthDiff <= 350.0f) {
                    if (!lm->discovered) {
                        lm->discovered = 1;
                        int pts = (int)(lm->pts * g_sub.surveyMultiplier);
                        g_sub.surveyPoints += pts;
                        found = 1;
                        PlaySoundAsync(780, 200);
                        snprintf(msg, sizeof(msg), "🌟 DISCOVERY LOGGED: [%s]! %s (+%d PTS)", lm->name, lm->info, pts);
                        AddLog(msg, th->accentEmerald);
                    }
                }
            }
            if (!found) {
                PlaySoundAsync(320, 80);
                AddLog("Survey complete. No new uncataloged landmarks in sensor cone.", th->textDim);
            }
            break;
        }

        case ID_BTN_FLOOD_BALLAST:
            if (g_sub.ballast < 100.0f) {
                g_sub.ballast = min(100.0f, g_sub.ballast + g_sub.ballastStepRate);
                PlaySoundAsync(280, 100);
                snprintf(msg, sizeof(msg), "Kingston flood valves opened: Ballast %.0f%% flooded.", g_sub.ballast);
                AddLog(msg, th->textPrimary);
            }
            break;

        case ID_BTN_BLOW_BALLAST:
            if (g_sub.airReservoir > 5.0f && g_sub.ballast > 0.0f) {
                g_sub.ballast = max(0.0f, g_sub.ballast - g_sub.ballastStepRate);
                g_sub.airReservoir = max(0.0f, g_sub.airReservoir - 8.0f);
                PlaySoundAsync(600, 180);
                snprintf(msg, sizeof(msg), "HP air blown: Ballast %.0f%%, Air Res: %.0f BAR.", g_sub.ballast, g_sub.airReservoir);
                AddLog(msg, th->accentEmerald);
            } else if (g_sub.airReservoir <= 5.0f) {
                PlaySoundAsync(180, 200);
                AddLog("WARNING: Low compressed air reserve to blow ballast!", th->accentRed);
            }
            break;

        case ID_BTN_TRIM_BOW:
            g_sub.pitch = max(-15.0f, g_sub.pitch - 2.5f);
            PlaySoundAsync(330, 80);
            snprintf(msg, sizeof(msg), "Bow trim adjusted. Pitch: %+.1f deg", g_sub.pitch);
            AddLog(msg, th->textPrimary);
            break;

        case ID_BTN_TRIM_STERN:
            g_sub.pitch = min(15.0f, g_sub.pitch + 2.5f);
            PlaySoundAsync(330, 80);
            snprintf(msg, sizeof(msg), "Stern trim adjusted. Pitch: %+.1f deg", g_sub.pitch);
            AddLog(msg, th->textPrimary);
            break;

        case ID_BTN_SONAR_PING:
            if (!g_sub.isPinging) {
                g_sub.isPinging = 1;
                g_sub.pingRadius = 0.0f;
                g_sub.battery = max(0.0f, g_sub.battery - 0.2f);
                PlaySoundAsync(1920, 250);
                AddLog("Active sonar omnidirectional ping emitted.", th->accentSonar);

                int secFauna[3];
                int fCount = GetSectorFaunaIndices(g_sub.currentSectorIdx, secFauna);
                for (int i = 0; i < fCount; i++) {
                    FaunaAnomaly* f = &g_fauna[secFauna[i]];
                    if (f->type == 4 && (rand() % 100) < 50) {
                        PlayLeviathanHarmonic();
                    }
                }
            }
            break;

        case ID_BTN_THROTTLE_REV:
            g_sub.throttleMode = 0;
            g_sub.targetSpeed = -2.5f;
            PlaySoundAsync(380, 80);
            AddLog("Engine telegraph: REVERSE (1/3).", th->textPrimary);
            break;

        case ID_BTN_THROTTLE_STOP:
            g_sub.throttleMode = 1;
            g_sub.targetSpeed = 0.0f;
            PlaySoundAsync(380, 80);
            AddLog("Engine telegraph: ALL STOP.", th->textPrimary);
            break;

        case ID_BTN_THROTTLE_HALF:
            g_sub.throttleMode = 2;
            g_sub.targetSpeed = 5.0f;
            PlaySoundAsync(380, 80);
            AddLog("Engine telegraph: AHEAD HALF (2/3).", th->textPrimary);
            break;

        case ID_BTN_THROTTLE_FLANK:
            g_sub.throttleMode = 3;
            g_sub.targetSpeed = 11.5f;
            PlaySoundAsync(480, 120);
            AddLog("Engine telegraph: FLANK SPEED (MAX AHEAD).", th->accentAmber);
            break;

        case ID_BTN_RUDDER_PORT:
            g_sub.heading = fmodf(g_sub.heading - 5.0f + 360.0f, 360.0f);
            PlaySoundAsync(450, 60);
            snprintf(msg, sizeof(msg), "Rudder Port 5 deg -> Heading: %.0f deg", g_sub.heading);
            AddLog(msg, th->textPrimary);
            break;

        case ID_BTN_RUDDER_STBD:
            g_sub.heading = fmodf(g_sub.heading + 5.0f, 360.0f);
            PlaySoundAsync(450, 60);
            snprintf(msg, sizeof(msg), "Rudder Starboard 5 deg -> Heading: %.0f deg", g_sub.heading);
            AddLog(msg, th->textPrimary);
            break;

        case ID_BTN_SEARCHLIGHTS:
            g_sub.searchlights = !g_sub.searchlights;
            PlaySoundAsync(520, 80);
            snprintf(msg, sizeof(msg), "High-lux forward searchlights %s.", g_sub.searchlights ? "ENGAGED" : "OFF");
            AddLog(msg, th->textPrimary);
            break;

        case ID_BTN_SCRUBBER:
            g_sub.scrubberAuto = !g_sub.scrubberAuto;
            PlaySoundAsync(400, 80);
            snprintf(msg, sizeof(msg), "O2 Scrubber system set to %s.", g_sub.scrubberAuto ? "AUTO" : "MANUAL");
            AddLog(msg, th->textPrimary);
            break;

        case ID_BTN_O2_PURGE:
            if (g_sub.o2PurgeCount > 0) {
                g_sub.o2PurgeCount--;
                g_sub.o2 = min(100.0f, g_sub.o2 + 25.0f);
                g_sub.co2 = max(0.04f, g_sub.co2 - 0.5f);
                PlaySoundAsync(750, 200);
                snprintf(msg, sizeof(msg), "Emergency O2 canister purged! O2: %.1f%% (%d left).", g_sub.o2, g_sub.o2PurgeCount);
                AddLog(msg, th->accentEmerald);
            } else {
                PlaySoundAsync(180, 200);
                AddLog("Emergency O2 reserve canisters depleted!", th->accentRed);
            }
            break;

        case ID_BTN_BILGE_PUMP:
            g_sub.bilgePumpActive = !g_sub.bilgePumpActive;
            PlaySoundAsync(360, 80);
            snprintf(msg, sizeof(msg), "Bilge drainage pumps %s.", g_sub.bilgePumpActive ? "RUNNING [MAX]" : "AUTO [STANDBY]");
            AddLog(msg, th->textPrimary);
            break;

        case ID_BTN_LOW_POWER:
            g_sub.lowPowerMode = !g_sub.lowPowerMode;
            PlaySoundAsync(480, 80);
            snprintf(msg, sizeof(msg), "Submersible ECO low-power mode %s.", g_sub.lowPowerMode ? "ACTIVE" : "OFF");
            AddLog(msg, th->textPrimary);
            break;

        case ID_BTN_EMERGENCY_BLOW:
            g_sub.ballast = 0.0f;
            g_sub.pitch = 10.0f;
            g_sub.airReservoir = max(0.0f, g_sub.airReservoir - 50.0f);
            PlaySoundAsync(700, 300);
            AddLog("EMERGENCY MAIN BALLAST BLOW EXECUTED! Maximum positive ascent!", th->accentRed);
            break;
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            g_hWnd = hWnd;
            InitSubmarineState();
            SetTimer(hWnd, TIMER_ID, TIMER_INTERVAL, NULL);

            g_hFontTitle = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            g_hFontNormal = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            g_hFontSmall = CreateFontA(11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            g_hFontBold = CreateFontA(12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            return 0;
        }

        case WM_TIMER:
            if (wParam == TIMER_ID) {
                UpdateSimulation(TIMER_INTERVAL / 1000.0f);
                InvalidateRect(hWnd, NULL, FALSE);
            }
            return 0;

        case WM_LBUTTONDOWN: {
            int mx = GET_X_LPARAM(lParam);
            int my = GET_Y_LPARAM(lParam);
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            int cmd = HitTestButton(mx, my, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
            if (cmd > 0) {
                HandleCommand(cmd);
                InvalidateRect(hWnd, NULL, FALSE);
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);

            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

            DrawUI(hdcMem, &rcClient);

            BitBlt(hdc, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, hdcMem, 0, 0, SRCCOPY);

            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);

            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            KillTimer(hWnd, TIMER_ID);
            if (g_hFontTitle) DeleteObject(g_hFontTitle);
            if (g_hFontNormal) DeleteObject(g_hFontNormal);
            if (g_hFontSmall) DeleteObject(g_hFontSmall);
            if (g_hFontBold) DeleteObject(g_hFontBold);
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEXA wcex;
    memset(&wcex, 0, sizeof(wcex));
    wcex.cbSize = sizeof(WNDCLASSEXA);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszClassName = "KSubmarineClass";

    if (!RegisterClassExA(&wcex)) return 1;

    HWND hWnd = CreateWindowA(
        "KSubmarineClass",
        "KSubmarine - Bathyscaphe Deep-Sea Submersible Dashboard",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1080, 680,
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd) return 1;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
