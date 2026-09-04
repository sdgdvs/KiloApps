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
#define ID_BTN_VIEW_CARGO       137
#define ID_BTN_TOGGLE_CLAW      138
#define ID_BTN_DREDGE_SEABED    139
#define ID_BTN_OFFLOAD_CARGO    140
#define ID_BTN_VIEW_DAMAGE      141
#define ID_BTN_PUMP_MODE        142
#define ID_BTN_BLEED_VALVE      143
#define ID_BTN_SIM_BREACH       144
#define ID_BTN_DOOR_BAY_0       145
#define ID_BTN_DOOR_BAY_1       146
#define ID_BTN_DOOR_BAY_2       147
#define ID_BTN_DOOR_BAY_3       148
#define ID_BTN_AIRDAM_BAY_0     149
#define ID_BTN_AIRDAM_BAY_1     150
#define ID_BTN_AIRDAM_BAY_2     151
#define ID_BTN_AIRDAM_BAY_3     152
#define ID_BTN_ROUTE_BAY_0      153
#define ID_BTN_ROUTE_BAY_1      154
#define ID_BTN_ROUTE_BAY_2      155
#define ID_BTN_ROUTE_BAY_3      156
#define ID_BTN_PATCH_BAY_0      157
#define ID_BTN_PATCH_BAY_1      158
#define ID_BTN_PATCH_BAY_2      159
#define ID_BTN_PATCH_BAY_3      160
#define ID_BTN_VIEW_LAB         161
#define ID_BTN_INCUBATE_BIO     162
#define ID_BTN_RES_POLYMERS     163
#define ID_BTN_RES_BIOLUM       164
#define ID_BTN_RES_BIOFUEL      165
#define ID_BTN_RES_REGEN        166
#define ID_BTN_VIEW_OUTPOSTS    167
#define ID_BTN_DOCK_OUTPOST     168
#define ID_BTN_UNDOCK_OUTPOST   169
#define ID_BTN_USE_BATTERY      170
#define ID_BTN_USE_REPAIR_KIT   171
#define ID_BTN_TRADE_SELL_ALL   172
#define ID_BTN_TRADE_RECHARGE   173
#define ID_BTN_TRADE_AIR        174
#define ID_BTN_TRADE_REPAIR     175
#define ID_BTN_BUY_TORPEDO      176
#define ID_BTN_BUY_EMP_TORPEDO  177
#define ID_BTN_BUY_PLASMA_TORPEDO 178
#define ID_BTN_BUY_BATTERY_PACK 179
#define ID_BTN_BUY_REPAIR_KIT   180
#define ID_BTN_BARTER_TORPEDO   181
#define ID_BTN_BARTER_EMP       182
#define ID_BTN_BARTER_PLASMA    183
#define ID_BTN_BARTER_BATTERY   184
#define ID_BTN_BARTER_REPAIR    185
#define ID_BTN_SELECT_OUTPOST_0 186
#define ID_BTN_SELECT_OUTPOST_1 187
#define ID_BTN_SELECT_OUTPOST_2 188
#define ID_BTN_SELECT_OUTPOST_3 189
#define ID_BTN_VIEW_COMBAT      190
#define ID_BTN_FIRE_TUBE_0      191
#define ID_BTN_FIRE_TUBE_1      192
#define ID_BTN_FIRE_TUBE_2      193
#define ID_BTN_FIRE_TUBE_3      194
#define ID_BTN_CYCLE_TUBE_0     195
#define ID_BTN_CYCLE_TUBE_1     196
#define ID_BTN_CYCLE_TUBE_2     197
#define ID_BTN_CYCLE_TUBE_3     198
#define ID_BTN_COMBAT_DECOY     199
#define ID_BTN_COMBAT_SHOCKWAVE 200
#define ID_BTN_COMBAT_SILENT    201
#define ID_BTN_LOCK_THREAT_0    202
#define ID_BTN_LOCK_THREAT_1    203
#define ID_BTN_LOCK_THREAT_2    204
#define ID_BTN_LOCK_THREAT_3    205
#define ID_BTN_LOCK_THREAT_4    206
#define ID_BTN_LOCK_THREAT_5    207
#define ID_BTN_FIRE_THREAT_0    208
#define ID_BTN_FIRE_THREAT_1    209
#define ID_BTN_FIRE_THREAT_2    210
#define ID_BTN_FIRE_THREAT_3    211
#define ID_BTN_FIRE_THREAT_4    212
#define ID_BTN_FIRE_THREAT_5    213
#define ID_BTN_QUICK_FIRE_ACOUSTIC 214
#define ID_BTN_QUICK_FIRE_EMP      215
#define ID_BTN_QUICK_FIRE_PLASMA   216
#define ID_BTN_QUICK_LAUNCH_DECOY  217
#define ID_BTN_VIEW_AUDIO          218
#define ID_BTN_TEST_SONAR_PING     219
#define ID_BTN_TEST_WHALE_SONG     220
#define ID_BTN_TEST_BALLAST_BLOW   221
#define ID_BTN_TEST_HULL_GROAN     222
#define ID_BTN_TEST_TORPEDO_LAUNCH 223
#define ID_BTN_TEST_PLASMA_LAUNCH  224
#define ID_BTN_TEST_SHOCKWAVE      225
#define ID_BTN_TEST_EXPLOSION      226
#define ID_BTN_TEST_DECOY          227
#define ID_BTN_TEST_WELDING        228
#define ID_BTN_TEST_ALARM          229
#define ID_BTN_TEST_CENTRIFUGE     230

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

// --- PHASE 10: DEEP-SEA BIOLOGY RESEARCH STRUCTS ---
typedef struct {
    int tier;
    const char* name;
    int cost;
    int reqPlankton;
    float bonusHull;
    float crushReduction;
    const char* desc;
} PolymerResearch;

typedef struct {
    int tier;
    const char* name;
    int cost;
    int reqCephalopod;
    float sonarBonus;
    float surveyBonus;
    const char* desc;
} BiolumResearch;

typedef struct {
    int tier;
    const char* name;
    int cost;
    int reqEnzymes;
    float drainSave;
    float ventCharge;
    const char* desc;
} BiofuelResearch;

typedef struct {
    int tier;
    const char* name;
    int cost;
    int reqHadal;
    float autoHeal;
    int autoPatch;
    const char* desc;
} RegenResearch;

static const PolymerResearch g_polymerRes[4] = {
    { 1, "MK I - SYNTHETIC", 0, 0, 0.0f, 0.0f, "Standard carbon polymer seals and epipelagic gaskets." },
    { 2, "MK II - PIEZOPHILIC MATRIX", 160, 2, 15.0f, 0.20f, "Piezophile-infused resin. +15% hull and -20% crush damage." },
    { 3, "MK III - GRAPHENE ELASTOMER", 320, 4, 30.0f, 0.35f, "Cross-linked graphene elastomer. +30% hull and -35% crush damage." },
    { 4, "MK IV - HADAL FULLERENE", 580, 6, 50.0f, 0.50f, "Ultra-resilient Hadal fullerene shell. +50% hull and -50% crush damage." }
};

static const BiolumResearch g_biolumRes[4] = {
    { 1, "MK I - EXTRACT", 0, 0, 0.0f, 1.0f, "Basic photophore bio-pigments extracted from cephalopod mantles." },
    { 2, "MK II - LUCIFERIN-B ENZYME", 150, 2, 300.0f, 1.25f, "Refined deep luciferase enzyme. +300m sonar range & +25% scan yield." },
    { 3, "MK III - PHOTOPHORE GEL", 300, 4, 600.0f, 1.50f, "High-lumens resonant photophore gel. +600m sonar range & +50% scan yield." },
    { 4, "MK IV - QUANTUM GLOW", 550, 6, 1000.0f, 2.00f, "Quantum-stimulated photonic matrix. +1000m sonar range & 2.0x scan yield." }
};

static const BiofuelResearch g_biofuelRes[4] = {
    { 1, "MK I - CATALYST", 0, 0, 0.0f, 0.0f, "Standard bacterial culture for minor bio-chemical filtering." },
    { 2, "MK II - SULFUR-OXIDIZING CELL", 180, 2, 0.15f, 0.10f, "Chemosynthetic sulfur fuel cells (-15% electrical drain, +0.10%/s at vents)." },
    { 3, "MK III - HYDROTHERMAL REACTOR", 360, 4, 0.28f, 0.22f, "High-yield enzymatic reactor (-28% electrical drain, +0.22%/s at vents)." },
    { 4, "MK IV - GEOTHERMAL BIO-SYNTH", 620, 6, 0.40f, 0.40f, "Master bio-synthetic converter (-40% electrical drain, +0.40%/s at vents)." }
};

static const RegenResearch g_regenRes[4] = {
    { 1, "MK I - CULTURING", 0, 0, 0.0f, 0, "Basic extremophile culture undergoing laboratory synthesis." },
    { 2, "MK II - CELLULAR PATCH", 220, 2, 0.5f, 1, "Self-repairing cellular biopolymer. Seals weeping leaks below 2000m." },
    { 3, "MK III - AUTONOMIC BIOPOLYMER", 420, 4, 1.0f, 1, "Advanced living membrane. Regenerates +1.0% integrity / 10s below 2000m." },
    { 4, "MK IV - LIVING HULL BIOME", 700, 6, 2.0f, 1, "Immortal Hadal piezophile biome. Restores +2.0% integrity / 8s below 2000m." }
};

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


// --- PHASE 11: OUTPOSTS & RESEARCH STATIONS ---
typedef struct {
    char id[8];
    char name[48];
    char subtitle[48];
    int sectorIdx;
    float depth;
    float x, y;
    char desc[128];
} OutpostInfo;

#define OUTPOST_COUNT 4

static const OutpostInfo g_outposts[OUTPOST_COUNT] = {
    { "o01", "Surface Support Tender ORCA-V", "Fleet Logistics & Drydock Flagship", 0, 0.0f, 0.0f, 0.0f, "Surface mother vessel providing air refills, generator charging & acoustic torpedoes." },
    { "o02", "Bathyal Outpost NEPTUNE-PRIME", "Mesopelagic Slope Research Station", 1, 580.0f, 3.2f, -2.8f, "Deep research station anchored to continental slope. Supplies EMP shock torpedoes." },
    { "o03", "Thermal Research Dome VULCAN-7", "Geothermal Smoker Station", 2, 2550.0f, -4.8f, -6.2f, "Pressurized geodesic dome powered by hydrothermal vents. Fabricates thermal plasma torpedoes." },
    { "o04", "Hadal Deep Lab TETHYS-IX", "Ultra-Deep Mariana Chasm Outpost", 3, 8400.0f, 7.8f, 8.2f, "Sub-crustal hadal research outpost trading extreme-depth plasma ordnance & quantum battery packs." }
};

// --- RESOURCE SALVAGE & DREDGING (PHASE 8) ---
typedef struct {
    const char* key;
    const char* name;
    float unitWeight;
    int unitVal;
    const char* desc;
} ResourceDef;

static const ResourceDef g_resDefs[5] = {
    { "manganese", "Manganese Nodules", 15.0f, 20, "Ferromanganese nodules rich in nickel, cobalt & copper." },
    { "sunkenGold", "Sunken Galleon Relics", 35.0f, 70, "Ancient Spanish silver bullion & gold doubloons." },
    { "titaniumScrap", "Titanium Wreckage Scraps", 30.0f, 55, "High-tensile submarine alloy plating & structural beams." },
    { "smokerCrystals", "Black Smoker Crystals", 20.0f, 65, "Hydrothermal copper-iron chalcopyrite & pyrite crystals." },
    { "hadalPrisms", "Mariana Hadal Prisms", 25.0f, 160, "Ultra-compressed Hadal quartz & deep-abyss silica geodes." }
};

typedef struct {
    char id[8];
    char name[32];
    int resKey; // 0: manganese, 1: sunkenGold, 2: titaniumScrap, 3: smokerCrystals, 4: hadalPrisms
    int sectorIdx;
    float depth;
    float x, y;
    int qty;
    float weight;
    int val;
    char desc[128];
    int harvested;
} SalvageNode;

#define SALVAGE_NODE_COUNT 8

static SalvageNode g_salvageNodes[SALVAGE_NODE_COUNT] = {
    // Sector 0: Continental Shelf (0-200m)
    { "s01", "Manganese Nodule Bed Alpha", 0, 0, 140.0f, 0.5f, 1.5f, 4, 60.0f, 80, "High-density ferromanganese nodule field scattered across continental shelf.", 0 },
    { "s02", "Sunken Galleon San Pedro", 1, 0, 175.0f, -1.8f, 2.8f, 2, 70.0f, 140, "17th-century Spanish galleon wreck holding bullion chests and artifacts.", 0 },

    // Sector 1: Twilight Drop-Off (200-1000m)
    { "s03", "Derelict Titanium Sub", 2, 1, 720.0f, 3.5f, -2.0f, 3, 90.0f, 165, "1980s experimental deep-dive vessel with intact titanium bulkheads.", 0 },
    { "s04", "Polymetallic Sulfide Mound", 0, 1, 950.0f, 4.2f, -3.6f, 5, 75.0f, 100, "Massive seafloor mound of concentrated copper, zinc, and silver sulfides.", 0 },

    // Sector 2: Hydrothermal Vents (1000-4000m)
    { "s05", "Smoker Chimney Crystals", 3, 2, 2600.0f, -5.2f, -5.8f, 4, 80.0f, 260, "Hydrothermal crystals precipitated from 350 deg C mineral vent plumes.", 0 },
    { "s06", "Chalcopyrite Magma Fissure", 3, 2, 3400.0f, -3.5f, -8.0f, 3, 60.0f, 195, "Volcanic magma rift coated in chalcopyrite and telluride minerals.", 0 },

    // Sector 3: Mariana Hadal Chasm (4000-11000m)
    { "s07", "Hadal Xenophyophore Geodes", 4, 3, 7800.0f, 8.8f, 7.0f, 2, 50.0f, 320, "Silica quartz geodes crystallized under 800 atm of Hadal pressure.", 0 },
    { "s08", "Mariana Challenger Void Core", 4, 3, 10700.0f, 10.0f, 10.8f, 3, 75.0f, 480, "Prehistoric extraterrestrial meteorite core resting at Challenger abyss.", 0 }
};

typedef struct {
    int isSalvage;
    int index;
    char name[32];
    char id[8];
    float x, y, depth;
    int discovered;
    int type; // 0..4 for fauna, 5 for salvage
    int ptsOrVal;
    char lumens[32];
    char freq[32];
} UnifiedContact;

// Phase 9: Bulkhead Compartments & Damage Control
#define COMPARTMENT_COUNT 4
typedef struct {
    char id[8];
    char name[48];
    char shortCode[8];
    float integrity;      // 0..100%
    float water;          // 0..250 gal
    float maxWater;       // 250.0f gal
    int breachTier;       // 0: INTACT, 1: HAIRLINE WEEP, 2: SEAM RUPTURE, 3: TORRENTIAL FRACTURE
    float leakRate;       // GPM
    int doorSealed;       // 0: OPEN, 1: SEALED
    int airDam;           // 0: OFF, 1: ACTIVE
    int isRepairing;      // 0: NO, 1: WELDING ACTIVE
    float repairProgress; // 0..100%
} CompartmentInfo;

// --- PHASE 12: TORPEDO DEFENSE & UNDERSEA COMBAT STRUCTS ---
typedef struct {
    const char* key;
    const char* name;
    int damage;
    float speed;
    float reloadTime;
    float stunDuration;
    float blastRadius;
    const char* desc;
} OrdnanceDef;

static const OrdnanceDef g_ordDefs[3] = {
    { "torpedoes", "Acoustic Homing Torpedo", 85, 0.38f, 5.0f, 0.0f, 35.0f, "High-velocity acoustic seeker homing on cavitation returns." },
    { "empTorpedoes", "EMP Shock Torpedo", 45, 0.42f, 6.0f, 8.0f, 40.0f, "High-voltage electromagnetic pulse disabling drones & fauna." },
    { "plasmaTorpedoes", "Thermal Plasma Torpedo", 190, 0.52f, 8.0f, 0.0f, 50.0f, "Supercavitating high-yield plasma warhead melting armor." }
};

typedef struct {
    char id[8];
    char name[32];
    int type; // 0: drone, 1: leviathan
    int sectorIdx;
    float depth;
    float x, y;
    float vx, vy;
    int hp, maxHp;
    int atkPower;
    float atkRange;
    float detectRange;
    char desc[96];
    int bountyCredits;
    int bountyMineral; // 0..4 or -1
    int bountyQty;
    int bountyBio; // 0..3 or -1
    int state; // 0: patrol, 1: stalking, 2: attacking, 3: distracted, 4: stunned
    float stunTimer;
    float atkCooldown;
    int defeated;
} HostileThreat;

#define THREAT_COUNT 6

static HostileThreat g_threats[THREAT_COUNT] = {
    { "t01", "Rogue Dredge Drone Alpha", 0, 0, 135.0f, -0.6f, 1.6f, 0.05f, -0.04f, 100, 100, 12, 120.0f, 450.0f, "Automated seabed mineral strip-miner gone rogue.", 120, 2, 2, -1, 0, 0.0f, 0.0f, 0 },
    { "t02", "Apex Megalodon Relic", 1, 1, 450.0f, 2.5f, -1.8f, -0.06f, 0.05f, 180, 180, 20, 130.0f, 550.0f, "Prehistoric 18m apex predator lurking near basalt canyon cliffs.", 180, -1, 0, 1, 0, 0.0f, 0.0f, 0 },
    { "t03", "Autonomous Abyssal Raider", 0, 1, 780.0f, 4.0f, -3.2f, 0.07f, 0.03f, 150, 150, 18, 140.0f, 500.0f, "Armed deep-water security sub enforcing forbidden salvage zones.", 160, 2, 2, -1, 0, 0.0f, 0.0f, 0 },
    { "t04", "Abyssal Phantom Kraken", 1, 2, 2150.0f, -4.2f, -6.5f, -0.05f, -0.06f, 260, 260, 28, 150.0f, 600.0f, "Massive bioluminescent titan with crushing barbed tentacles.", 260, -1, 0, 2, 0, 0.0f, 0.0f, 0 },
    { "t05", "Heavy Cyber-Sub Hunter-X", 0, 2, 3200.0f, -3.0f, -8.8f, 0.06f, 0.05f, 220, 220, 25, 160.0f, 650.0f, "Heavy armored military drone with high-yield thermal torpedoes.", 240, 3, 2, -1, 0, 0.0f, 0.0f, 0 },
    { "t06", "Mariana Hadal Void Serpent", 1, 3, 8600.0f, 8.5f, 7.8f, 0.08f, -0.07f, 380, 380, 36, 180.0f, 750.0f, "Legendary hadal titan generating gravitational acoustic shocks.", 450, 4, 2, 3, 0, 0.0f, 0.0f, 0 }
};

typedef struct {
    int id; // 1..4
    int type; // 0: acoustic, 1: emp, 2: plasma
    int status; // 0: ready, 1: reloading
    float reloadTime;
} TorpedoTube;

typedef struct {
    float x, y, depth;
    float vx, vy, vz;
    int type; // 0: acoustic, 1: emp, 2: plasma
    int damage;
    int targetIdx;
    float life;
    float trailX[12];
    float trailY[12];
    int trailCount;
} ActiveTorpedo;

typedef struct {
    float x, y, depth;
    float life;
    float pulseRadius;
} ActiveDecoy;

typedef struct {
    float x, y;
    float radius;
    float maxRadius;
    float life;
    COLORREF color;
    char text[32];
} ExplosionEffect;

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
    int viewMode;           // 0: Sonar, 1: Nav Map, 2: Codex, 3: Cargo, 4: Engineering, 5: Damage Control, 6: Lab, 7: Outposts, 8: Combat
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

    // Target Locking & Bio-Scan
    int selectedTargetIdx;
    int isScanningTarget;
    float scanProgress;

    // Phase 8: Cargo Hold & Dredging Claw System
    int cargoManganese;
    int cargoSunkenGold;
    int cargoTitaniumScrap;
    int cargoSmokerCrystals;
    int cargoHadalPrisms;
    float cargoTotalWeight;
    float cargoMaxWeight;
    int cargoTotalValue;
    int clawDeployed;
    int isDredging;
    float dredgeProgress;

    // Phase 11: Ordnance, Equipment & Outposts
    int torpedoes;
    int empTorpedoes;
    int plasmaTorpedoes;
    int batteryPacks;
    int repairKits;
    int isDocked;
    int dockedStationIdx;
    int selectedOutpostIdx;

    // Phase 12: Torpedo Defense & Undersea Combat
    int decoys;
    float shockwaveCooldown;
    int silentRunning;
    int selectedThreatIdx;
    TorpedoTube tubes[4];
    ActiveTorpedo activeTorpedoes[8];
    int activeTorpedoCount;
    ActiveDecoy activeDecoys[4];
    int activeDecoyCount;
    ExplosionEffect explosions[8];
    int explosionCount;
    // Phase 10: Deep-Sea Biology & Research Lab
    int bioPlankton;
    int bioCephalopod;
    int bioEnzymes;
    int bioHadal;
    int resPolymers;
    int resBiolum;
    int resBiofuel;
    int resRegen;

    // Phase 9: Damage Control & Bulkhead Compartments
    CompartmentInfo compartments[COMPARTMENT_COUNT];
    float cabinPressure;      // atm (1.00 nominal)
    int bleedValveOpen;       // 0: closed, 1: venting
    int bilgePumpMode;        // 0: OFF, 1: AUTO, 2: BOW, 3: CMD, 4: ENG, 5: AFT, 6: OVERDRIVE
    int emergencySirens;

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
void PlayClawServo(void);
void PlayMineralChime(void);
void PlayLabCentrifuge(void);
void PlayResearchBreakthrough(void);
void PlayDockingClamps(void);
void PlayTorpedoLaunchSound(void);
void PlayTorpedoExplosionSound(void);
void PlayDecoySound(void);
void PlayShockwaveSound(void);
void PlayWhaleSong(void);
void PlayHullPressureGroan(void);
void PlayBallastBlowHiss(void);
void PlayPlasmaLaunchSound(void);
void DrawOutpostTradeView(HDC hdc, int x, int y, int w, int h, const SubmarineTheme* th);
void DrawCombatView(HDC hdc, int x, int y, int w, int h, const SubmarineTheme* th);
void DrawAudioView(HDC hdc, int x, int y, int w, int h, const SubmarineTheme* th);
void FireTorpedoTube(int tubeIdx);
void CycleTubeOrdnance(int tubeIdx);
void LaunchAcousticDecoy(void);
void TriggerShockwaveDischarge(void);
void ToggleSilentRunning(void);
void LockThreatTarget(int tIdx);
void DefeatThreat(int tIdx);
void FireAtThreat(int tIdx);
void AddLog(const char* text, COLORREF color);
void InitSubmarineState(void);
void UpdateSimulation(float dt);
void DrawUI(HDC hdc, RECT* rcClient);
int GetSectorFaunaIndices(int sectorIdx, int outIndices[3]);
int GetUnifiedContacts(int sectorIdx, UnifiedContact outContacts[8]);
void RecalculateCargo(void);

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

DWORD WINAPI ClawServoSoundThreadProc(LPVOID lpParam) {
    Beep(240, 100);
    Sleep(20);
    Beep(160, 120);
    Sleep(20);
    Beep(90, 140);
    return 0;
}

void PlayClawServo(void) {
    if (!g_sub.soundEnabled) return;
    CreateThread(NULL, 0, ClawServoSoundThreadProc, NULL, 0, NULL);
}

DWORD WINAPI MineralChimeSoundThreadProc(LPVOID lpParam) {
    Beep(880, 80);
    Sleep(15);
    Beep(1174, 100);
    Sleep(15);
    Beep(1568, 140);
    return 0;
}

void PlayMineralChime(void) {
    if (!g_sub.soundEnabled) return;
    CreateThread(NULL, 0, MineralChimeSoundThreadProc, NULL, 0, NULL);
}

DWORD WINAPI WeldingSoundThreadProc(LPVOID lpParam) {
    for (int i = 0; i < 4; i++) {
        Beep(520 + (rand() % 300), 30);
        Sleep(15);
    }
    return 0;
}

void PlayWeldingSound(void) {
    if (!g_sub.soundEnabled) return;
    CreateThread(NULL, 0, WeldingSoundThreadProc, NULL, 0, NULL);
}

DWORD WINAPI AlarmKlaxonThreadProc(LPVOID lpParam) {
    Beep(440, 140);
    Sleep(20);
    Beep(880, 180);
    return 0;
}

void PlayAlarmKlaxon(void) {
    if (!g_sub.soundEnabled) return;
    CreateThread(NULL, 0, AlarmKlaxonThreadProc, NULL, 0, NULL);
}

DWORD WINAPI PressureBleedThreadProc(LPVOID lpParam) {
    Beep(1400, 60);
    Sleep(10);
    Beep(1100, 80);
    return 0;
}

void PlayPressureBleed(void) {
    if (!g_sub.soundEnabled) return;
    CreateThread(NULL, 0, PressureBleedThreadProc, NULL, 0, NULL);
}

DWORD WINAPI LabCentrifugeThreadProc(LPVOID lpParam) {
    for (int i = 0; i < 4; i++) {
        Beep(300 + i * 120, 40);
        Sleep(10);
    }
    Beep(980, 100);
    return 0;
}

void PlayLabCentrifuge(void) {
    if (!g_sub.soundEnabled) return;
    CreateThread(NULL, 0, LabCentrifugeThreadProc, NULL, 0, NULL);
}

DWORD WINAPI ResearchBreakthroughThreadProc(LPVOID lpParam) {
    Beep(523, 80);
    Sleep(15);
    Beep(659, 80);
    Sleep(15);
    Beep(784, 80);
    Sleep(15);
    Beep(1046, 160);
    return 0;
}


DWORD WINAPI DockingClampSoundThreadProc(LPVOID lpParam) {
    Beep(140, 150);
    Sleep(20);
    Beep(90, 180);
    Sleep(20);
    Beep(45, 250);
    return 0;
}

void PlayDockingClamps(void) {
    if (!g_sub.soundEnabled) return;
    CreateThread(NULL, 0, DockingClampSoundThreadProc, NULL, 0, NULL);
}

DWORD WINAPI TradeChimeSoundThreadProc(LPVOID lpParam) {
    Beep(587, 80);
    Sleep(15);
    Beep(880, 100);
    Sleep(15);
    Beep(1174, 150);
    return 0;
}

void PlayTradeChime(void) {
    if (!g_sub.soundEnabled) return;
    CreateThread(NULL, 0, TradeChimeSoundThreadProc, NULL, 0, NULL);
}

void PlayResearchBreakthrough(void) {
    if (!g_sub.soundEnabled) return;
    CreateThread(NULL, 0, ResearchBreakthroughThreadProc, NULL, 0, NULL);
}

DWORD WINAPI TorpedoLaunchSoundThreadProc(LPVOID lpParam) {
    Beep(360, 60);
    Sleep(10);
    Beep(720, 80);
    Sleep(10);
    Beep(180, 160);
    return 0;
}

void PlayTorpedoLaunchSound(void) {
    if (!g_sub.soundEnabled) return;
    CreateThread(NULL, 0, TorpedoLaunchSoundThreadProc, NULL, 0, NULL);
}

DWORD WINAPI TorpedoExplosionSoundThreadProc(LPVOID lpParam) {
    Beep(120, 100);
    Sleep(15);
    Beep(65, 180);
    Sleep(15);
    Beep(37, 240);
    return 0;
}

void PlayTorpedoExplosionSound(void) {
    if (!g_sub.soundEnabled) return;
    CreateThread(NULL, 0, TorpedoExplosionSoundThreadProc, NULL, 0, NULL);
}

DWORD WINAPI DecoySoundThreadProc(LPVOID lpParam) {
    Beep(1400, 60);
    Sleep(10);
    Beep(800, 80);
    Sleep(10);
    Beep(400, 120);
    return 0;
}

void PlayDecoySound(void) {
    if (!g_sub.soundEnabled) return;
    CreateThread(NULL, 0, DecoySoundThreadProc, NULL, 0, NULL);
}

DWORD WINAPI ShockwaveSoundThreadProc(LPVOID lpParam) {
    Beep(80, 60);
    Sleep(10);
    Beep(1400, 120);
    Sleep(10);
    Beep(60, 200);
    return 0;
}

void PlayShockwaveSound(void) {
    if (!g_sub.soundEnabled) return;
    CreateThread(NULL, 0, ShockwaveSoundThreadProc, NULL, 0, NULL);
}

DWORD WINAPI WhaleSongThreadProc(LPVOID lpParam) {
    Beep(75, 200);
    Sleep(20);
    Beep(165, 250);
    Sleep(20);
    Beep(195, 220);
    Sleep(20);
    Beep(65, 300);
    return 0;
}

void PlayWhaleSong(void) {
    if (!g_sub.soundEnabled) return;
    CreateThread(NULL, 0, WhaleSongThreadProc, NULL, 0, NULL);
}

DWORD WINAPI HullPressureGroanThreadProc(LPVOID lpParam) {
    Beep(52, 180);
    Sleep(15);
    Beep(68, 220);
    Sleep(15);
    Beep(44, 280);
    return 0;
}

void PlayHullPressureGroan(void) {
    if (!g_sub.soundEnabled) return;
    CreateThread(NULL, 0, HullPressureGroanThreadProc, NULL, 0, NULL);
}

DWORD WINAPI BallastBlowHissThreadProc(LPVOID lpParam) {
    for (int i = 0; i < 5; i++) {
        Beep(1400 - i * 220, 40);
        Sleep(10);
    }
    Beep(170, 120);
    return 0;
}

void PlayBallastBlowHiss(void) {
    if (!g_sub.soundEnabled) return;
    CreateThread(NULL, 0, BallastBlowHissThreadProc, NULL, 0, NULL);
}

DWORD WINAPI PlasmaLaunchThreadProc(LPVOID lpParam) {
    Beep(600, 80);
    Sleep(10);
    Beep(420, 100);
    Sleep(10);
    Beep(180, 160);
    return 0;
}

void PlayPlasmaLaunchSound(void) {
    if (!g_sub.soundEnabled) return;
    CreateThread(NULL, 0, PlasmaLaunchThreadProc, NULL, 0, NULL);
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

int GetUnifiedContacts(int sectorIdx, UnifiedContact outContacts[8]) {
    int count = 0;
    for (int i = 0; i < FAUNA_COUNT; i++) {
        if (g_fauna[i].sectorIdx == sectorIdx) {
            if (count < 8) {
                UnifiedContact* c = &outContacts[count++];
                c->isSalvage = 0;
                c->index = i;
                strncpy(c->name, g_fauna[i].name, sizeof(c->name) - 1);
                c->name[sizeof(c->name) - 1] = '\0';
                strncpy(c->id, g_fauna[i].id, sizeof(c->id) - 1);
                c->id[sizeof(c->id) - 1] = '\0';
                c->x = g_fauna[i].x;
                c->y = g_fauna[i].y;
                c->depth = g_fauna[i].depth;
                c->discovered = g_fauna[i].discovered;
                c->type = g_fauna[i].type;
                c->ptsOrVal = g_fauna[i].pts;
                strncpy(c->lumens, g_fauna[i].lumens, sizeof(c->lumens) - 1);
                c->lumens[sizeof(c->lumens) - 1] = '\0';
                strncpy(c->freq, g_fauna[i].freq, sizeof(c->freq) - 1);
                c->freq[sizeof(c->freq) - 1] = '\0';
            }
        }
    }
    for (int j = 0; j < SALVAGE_NODE_COUNT; j++) {
        if (g_salvageNodes[j].sectorIdx == sectorIdx) {
            if (count < 8) {
                UnifiedContact* c = &outContacts[count++];
                c->isSalvage = 1;
                c->index = j;
                strncpy(c->name, g_salvageNodes[j].name, sizeof(c->name) - 1);
                c->name[sizeof(c->name) - 1] = '\0';
                strncpy(c->id, g_salvageNodes[j].id, sizeof(c->id) - 1);
                c->id[sizeof(c->id) - 1] = '\0';
                c->x = g_salvageNodes[j].x;
                c->y = g_salvageNodes[j].y;
                c->depth = g_salvageNodes[j].depth;
                c->discovered = g_salvageNodes[j].harvested;
                c->type = 5; // salvage
                c->ptsOrVal = g_salvageNodes[j].val;
                strncpy(c->lumens, "0 LUX (REFLECT)", sizeof(c->lumens) - 1);
                c->lumens[sizeof(c->lumens) - 1] = '\0';
                strncpy(c->freq, "SONAR 220 HZ", sizeof(c->freq) - 1);
                c->freq[sizeof(c->freq) - 1] = '\0';
            }
        }
    }
    return count;
}

void RecalculateCargo(void) {
    g_sub.cargoTotalWeight = (g_sub.cargoManganese * g_resDefs[0].unitWeight) +
                             (g_sub.cargoSunkenGold * g_resDefs[1].unitWeight) +
                             (g_sub.cargoTitaniumScrap * g_resDefs[2].unitWeight) +
                             (g_sub.cargoSmokerCrystals * g_resDefs[3].unitWeight) +
                             (g_sub.cargoHadalPrisms * g_resDefs[4].unitWeight);

    g_sub.cargoTotalValue = (g_sub.cargoManganese * g_resDefs[0].unitVal) +
                            (g_sub.cargoSunkenGold * g_resDefs[1].unitVal) +
                            (g_sub.cargoTitaniumScrap * g_resDefs[2].unitVal) +
                            (g_sub.cargoSmokerCrystals * g_resDefs[3].unitVal) +
                            (g_sub.cargoHadalPrisms * g_resDefs[4].unitVal);
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
    g_sub.viewMode = 0; // 0: Sonar, 1: Nav Map, 2: Codex, 3: Cargo, 4: Engineering
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

    // Target Locking & Bio-Scan
    g_sub.selectedTargetIdx = 0;
    g_sub.isScanningTarget = 0;
    g_sub.scanProgress = 0.0f;

    // Phase 8 Cargo Hold & Dredging Claw
    g_sub.cargoManganese = 0;
    g_sub.cargoSunkenGold = 0;
    g_sub.cargoTitaniumScrap = 0;
    g_sub.cargoSmokerCrystals = 0;
    g_sub.cargoHadalPrisms = 0;
    g_sub.cargoMaxWeight = 500.0f;
    g_sub.clawDeployed = 0;
    g_sub.isDredging = 0;
    g_sub.dredgeProgress = 0.0f;
    RecalculateCargo();

    // Phase 10 Deep-Sea Biology & Research Lab
    g_sub.bioPlankton = 2;
    g_sub.bioCephalopod = 0;
    g_sub.bioEnzymes = 0;
    g_sub.bioHadal = 0;
    g_sub.resPolymers = 1;
    g_sub.resBiolum = 1;
    g_sub.resBiofuel = 1;
    g_sub.resRegen = 1;

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

    // Phase 9: Initialize 4 compartments
    const char* cIds[4] = { "bow", "cmd", "eng", "aft" };
    const char* cNames[4] = { "Forward Sonar Bay", "Command Sphere Deck", "Reactor Bay", "Aft Bilge Manifold" };
    const char* cCodes[4] = { "BOW", "CMD", "ENG", "AFT" };
    for (int i = 0; i < COMPARTMENT_COUNT; i++) {
        strncpy(g_sub.compartments[i].id, cIds[i], sizeof(g_sub.compartments[i].id) - 1);
        strncpy(g_sub.compartments[i].name, cNames[i], sizeof(g_sub.compartments[i].name) - 1);
        strncpy(g_sub.compartments[i].shortCode, cCodes[i], sizeof(g_sub.compartments[i].shortCode) - 1);
        g_sub.compartments[i].integrity = 100.0f;
        g_sub.compartments[i].water = 0.0f;
        g_sub.compartments[i].maxWater = 250.0f;
        g_sub.compartments[i].breachTier = 0;
        g_sub.compartments[i].leakRate = 0.0f;
        g_sub.compartments[i].doorSealed = 0;
        g_sub.compartments[i].airDam = 0;
        g_sub.compartments[i].isRepairing = 0;
        g_sub.compartments[i].repairProgress = 0.0f;
    }
    g_sub.cabinPressure = 1.00f;
    g_sub.bleedValveOpen = 0;
    g_sub.bilgePumpMode = 1; // AUTO-BALANCED
    g_sub.emergencySirens = 0;

    // Phase 12: Torpedo Defense & Undersea Combat
    g_sub.torpedoes = 6;
    g_sub.empTorpedoes = 2;
    g_sub.plasmaTorpedoes = 1;
    g_sub.batteryPacks = 2;
    g_sub.repairKits = 2;
    g_sub.decoys = 3;
    g_sub.shockwaveCooldown = 0.0f;
    g_sub.silentRunning = 0;
    g_sub.selectedThreatIdx = 0;
    g_sub.activeTorpedoCount = 0;
    g_sub.activeDecoyCount = 0;
    g_sub.explosionCount = 0;

    for (int i = 0; i < 4; i++) {
        g_sub.tubes[i].id = i + 1;
        g_sub.tubes[i].type = (i == 0 ? 0 : (i == 1 ? 0 : (i == 2 ? 1 : 2)));
        g_sub.tubes[i].status = 0; // ready
        g_sub.tubes[i].reloadTime = 0.0f;
    }

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
    AddLog("Hydraulic dredging claw & mineral cargo bay calibrated.", g_themes[THEME_ABYSS].accentAmber);
    AddLog("Bulkhead damage control & flood isolation manifolds ready.", g_themes[THEME_ABYSS].accentSonar);
    AddLog("Torpedo fire-control, acoustic decoys & threat matrix active.", g_themes[THEME_ABYSS].accentAmber);
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
    float cargoPenalty = g_sub.cargoTotalWeight * 0.04f;
    float buoyancyForce = (neutralBallast - g_sub.ballast) * 0.4f - (cargoPenalty * 0.01f);
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

    // Bio-Scan Progress Update
    if (g_sub.isScanningTarget) {
        g_sub.scanProgress += dt * 60.0f;
        if (g_sub.scanProgress >= 100.0f) {
            g_sub.isScanningTarget = 0;
            g_sub.scanProgress = 0.0f;

            UnifiedContact contacts[8];
            int cCount = GetUnifiedContacts(g_sub.currentSectorIdx, contacts);
            if (cCount > 0 && g_sub.selectedTargetIdx < cCount) {
                UnifiedContact* tgt = &contacts[g_sub.selectedTargetIdx];
                if (!tgt->isSalvage) {
                    FaunaAnomaly* target = &g_fauna[tgt->index];
                    if (!target->discovered) {
                        target->discovered = 1;
                        float biolumYield = g_biolumRes[g_sub.resBiolum - 1].surveyBonus;
                        int ptsAwarded = (int)(target->pts * g_sub.surveyMultiplier * biolumYield);
                        g_sub.surveyPoints += ptsAwarded;

                        const char* sName = "+2 Plankton";
                        if (target->type == 1) {
                            g_sub.bioCephalopod += 2;
                            sName = "+2 Cephalopod DNA";
                        } else if (target->type == 2 || target->type == 3) {
                            g_sub.bioEnzymes += 2;
                            sName = "+2 Enzymes";
                        } else if (target->type == 4) {
                            g_sub.bioHadal += 3;
                            sName = "+3 Hadal Biomass";
                        } else {
                            g_sub.bioPlankton += 2;
                        }

                        if (target->type == 4) { // Leviathan
                            PlayLeviathanHarmonic();
                            char msg[128];
                            snprintf(msg, sizeof(msg), "🚨 LEVIATHAN SCANNED: [%s]! %s (+%d PTS & %s)", target->name, target->desc, ptsAwarded, sName);
                            AddLog(msg, RGB(244, 63, 94));
                        } else {
                            PlaySoundAsync(1100, 200);
                            char msg[128];
                            snprintf(msg, sizeof(msg), "BIO-SCAN COMPLETE: [%s]! %s (+%d PTS & %s)", target->name, target->desc, ptsAwarded, sName);
                            AddLog(msg, th->accentEmerald);
                        }
                    }
                }
            }
        }
    }

    // Dredging Claw Action Progress (Phase 8)
    if (g_sub.isDredging) {
        g_sub.dredgeProgress += dt * 50.0f;
        if (g_sub.dredgeProgress >= 100.0f) {
            g_sub.isDredging = 0;
            g_sub.dredgeProgress = 0.0f;

            UnifiedContact contacts[8];
            int cCount = GetUnifiedContacts(g_sub.currentSectorIdx, contacts);
            UnifiedContact* target = (cCount > 0 && g_sub.selectedTargetIdx < cCount) ? &contacts[g_sub.selectedTargetIdx] : NULL;

            SalvageNode* harvestNode = NULL;
            if (target && target->isSalvage && !target->discovered) {
                float dx = target->x - g_sub.posX;
                float dy = target->y - g_sub.posY;
                float distM = sqrtf(dx * dx + dy * dy) * 1000.0f;
                float depthDiff = fabsf(g_sub.depth - target->depth);
                if (distM <= 220.0f && depthDiff <= 90.0f) {
                    harvestNode = &g_salvageNodes[target->index];
                }
            }

            if (harvestNode && !harvestNode->harvested) {
                harvestNode->harvested = 1;
                int rk = harvestNode->resKey;
                if (rk == 0) g_sub.cargoManganese += harvestNode->qty;
                else if (rk == 1) g_sub.cargoSunkenGold += harvestNode->qty;
                else if (rk == 2) g_sub.cargoTitaniumScrap += harvestNode->qty;
                else if (rk == 3) g_sub.cargoSmokerCrystals += harvestNode->qty;
                else if (rk == 4) g_sub.cargoHadalPrisms += harvestNode->qty;

                int bonusPts = (int)(harvestNode->val * g_sub.surveyMultiplier);
                g_sub.surveyPoints += bonusPts;
                RecalculateCargo();
                PlayMineralChime();

                char sMsg[128];
                snprintf(sMsg, sizeof(sMsg), "💎 SALVAGE RECOVERED: [%s]! Harvested %dx %s (+%d PTS)",
                         harvestNode->name, harvestNode->qty, g_resDefs[rk].name, bonusPts);
                AddLog(sMsg, th->accentEmerald);
            } else {
                // General seabed dredging
                g_sub.cargoManganese += 2;
                int yieldPts = (int)(30 * g_sub.surveyMultiplier);
                g_sub.surveyPoints += yieldPts;
                RecalculateCargo();
                PlayMineralChime();

                char sMsg[128];
                snprintf(sMsg, sizeof(sMsg), "⛏️ BENTHIC DREDGING COMPLETE: Harvested 2x Manganese Nodules (+%d PTS)", yieldPts);
                AddLog(sMsg, th->accentEmerald);
            }
        }
    }

    // Dynamic Seabed Collision
    SectorInfo* curSec = &g_sectors[g_sub.currentSectorIdx];
    float terrainNoise = sinf(g_sub.posX * 1.5f) * cosf(g_sub.posY * 1.5f) * curSec->seabedVariance;
    g_sub.seabedElevation = curSec->baseSeabed + terrainNoise;

    if (g_sub.depth >= g_sub.seabedElevation) {
        g_sub.depth = g_sub.seabedElevation;
        if (g_sub.vertRate > 0.0f) g_sub.vertRate = 0.0f;
        if (fabsf(g_sub.speed) > 2.0f && (rand() % 100) < 4) {
            g_sub.hull = max(0.0f, g_sub.hull - 0.5f * dt);
            CompartmentInfo* bowBay = &g_sub.compartments[0];
            bowBay->integrity = max(0.0f, bowBay->integrity - 1.5f * dt);
            if (bowBay->integrity < 70.0f && bowBay->breachTier == 0 && (rand() % 100) < 15) {
                bowBay->breachTier = 1;
                PlayAlarmKlaxon();
                AddLog("⚠️ HULL SEEPAGE: Bow torpedo bay scraped seabed rock shelf! Hairline leak.", th->accentAmber);
            }
            PlaySoundAsync(140, 100);
            AddLog("WARNING: Keel scraping seabed rock shelf!", th->accentAmber);
        }
    }

    g_sub.pressure = 1.0f + (g_sub.depth * 0.0995f);
    g_sub.hullStress = min(100.0f, (g_sub.depth / g_sub.crushDepth) * 100.0f);

    const PolymerResearch* polyRes = &g_polymerRes[g_sub.resPolymers - 1];
    const RegenResearch* regRes = &g_regenRes[g_sub.resRegen - 1];
    const BiofuelResearch* fuelRes = &g_biofuelRes[g_sub.resBiofuel - 1];

    if (g_sub.depth > g_sub.crushDepth) {
        float excess = g_sub.depth - g_sub.crushDepth;
        float hullDamageReduction = (1.0f - (g_sub.upgradeHull - 1) * 0.2f) * (1.0f - polyRes->crushReduction);
        if (hullDamageReduction < 0.15f) hullDamageReduction = 0.15f;
        float hullDamage = (excess * 0.02f + 0.5f) * dt * hullDamageReduction;
        g_sub.hull = max(0.0f, g_sub.hull - hullDamage);

        // Compartment overcrush damage
        if ((rand() % 100) < 3) {
            int rIdx = rand() % COMPARTMENT_COUNT;
            if (g_sub.compartments[rIdx].breachTier < 3) {
                g_sub.compartments[rIdx].breachTier++;
                PlayAlarmKlaxon();
                char cMsg[128];
                snprintf(cMsg, sizeof(cMsg), "CRUSH BREACH: [%s] hull seams ruptured under %.1f atm!", g_sub.compartments[rIdx].shortCode, g_sub.pressure);
                AddLog(cMsg, th->accentRed);
            }
        }

        if ((rand() % 100) < 3) {
            PlayHullPressureGroan();
            AddLog("CRUSH WARNING: Extreme hydrostatic pressure deforming hull!", th->accentRed);
        }
    }

    // Phase 9: Compartment Flooding, Air Damming, Bilge Pumping & Atmospheric Pressure
    float totalWater = 0.0f;
    float totalIngress = 0.0f;

    for (int i = 0; i < COMPARTMENT_COUNT; i++) {
        CompartmentInfo* c = &g_sub.compartments[i];

        // Piezophilic cellular regenerator passive healing & auto-patch below 2000m
        if (g_sub.depth >= 2000.0f && regRes->autoHeal > 0.0f) {
            c->integrity = min(100.0f, c->integrity + (regRes->autoHeal / 10.0f) * dt);
            if (regRes->autoPatch && c->breachTier == 1 && (rand() % 100) < 2) {
                c->breachTier = 0;
                AddLog("🧬 LIVING HULL BIOME: Cellular regenerator sealed hairline fracture!", th->accentEmerald);
            }
        }

        if (c->breachTier > 0) {
            float baseRate = c->breachTier == 1 ? 4.0f : (c->breachTier == 2 ? 18.0f : 55.0f);
            float leakRate = baseRate * (1.0f + g_sub.pressure * 0.04f);

            if (c->airDam) {
                if (g_sub.airReservoir > 5.0f) {
                    leakRate *= 0.40f; // 60% reduction
                    g_sub.airReservoir = max(0.0f, g_sub.airReservoir - dt * 0.45f);
                    g_sub.cabinPressure += dt * 0.008f;
                } else {
                    c->airDam = 0;
                    AddLog("Air dam collapsed: Compressed air bank depleted!", th->accentAmber);
                }
            }

            c->leakRate = leakRate;
            c->water = min(c->maxWater, c->water + c->leakRate * (dt / 60.0f) * 12.0f);
            c->integrity = max(0.0f, c->integrity - dt * 0.08f * c->breachTier);
        } else {
            c->leakRate = 0.0f;
        }

        // Water spillover if bay is full and adjacent doors unsealed
        if (c->water >= c->maxWater) {
            if (i > 0 && !g_sub.compartments[i - 1].doorSealed) {
                g_sub.compartments[i - 1].water = min(g_sub.compartments[i - 1].maxWater, g_sub.compartments[i - 1].water + 2.5f * dt);
            }
            if (i < COMPARTMENT_COUNT - 1 && !g_sub.compartments[i + 1].doorSealed) {
                g_sub.compartments[i + 1].water = min(g_sub.compartments[i + 1].maxWater, g_sub.compartments[i + 1].water + 2.5f * dt);
            }
        }

        // Repair Progress
        if (c->isRepairing) {
            float rRate = g_sub.cargoTitaniumScrap > 0 ? 30.0f : 16.0f;
            c->repairProgress += rRate * dt;
            if (c->repairProgress >= 100.0f) {
                c->repairProgress = 0.0f;
                c->isRepairing = 0;
                if (c->breachTier > 0) c->breachTier--;
                c->integrity = min(100.0f, c->integrity + 30.0f);
                PlayMineralChime();
                char rMsg[128];
                snprintf(rMsg, sizeof(rMsg), "✅ BULKHEAD REPAIRED: [%s] structural patch sealed!", c->shortCode);
                AddLog(rMsg, th->accentEmerald);
            }
        }

        totalWater += c->water;
        totalIngress += c->leakRate;
    }

    // Bilge Pumping Simulation
    g_sub.bilgePumpActive = (g_sub.bilgePumpMode > 0 && g_sub.battery > 0.0f);
    if (g_sub.bilgePumpActive && totalWater > 0.0f) {
        float pumpCap = (12.0f + (g_sub.upgradeBallast - 1) * 6.0f) * (g_sub.bilgePumpMode == 6 ? 2.5f : 1.0f) * dt * 0.35f;
        if (g_sub.bilgePumpMode == 1 || g_sub.bilgePumpMode == 6) {
            int floodedCount = 0;
            for (int i = 0; i < COMPARTMENT_COUNT; i++) {
                if (g_sub.compartments[i].water > 0.0f) floodedCount++;
            }
            if (floodedCount > 0) {
                float share = pumpCap / floodedCount;
                for (int i = 0; i < COMPARTMENT_COUNT; i++) {
                    if (g_sub.compartments[i].water > 0.0f) {
                        g_sub.compartments[i].water = max(0.0f, g_sub.compartments[i].water - share);
                    }
                }
            }
        } else if (g_sub.bilgePumpMode >= 2 && g_sub.bilgePumpMode <= 5) {
            int targetIdx = g_sub.bilgePumpMode - 2;
            g_sub.compartments[targetIdx].water = max(0.0f, g_sub.compartments[targetIdx].water - pumpCap);
        }
    }

    // Internal Cabin Pressure Dynamics
    float targetCabinPres = 1.0f + (totalWater / 1000.0f) * 1.6f;
    g_sub.cabinPressure += (targetCabinPres - g_sub.cabinPressure) * (dt * 0.3f);
    if (g_sub.bleedValveOpen) {
        g_sub.cabinPressure = max(1.0f, g_sub.cabinPressure - dt * 0.35f);
    }

    g_sub.bilgeWater = totalWater;
    g_sub.waterIntrusionRate = totalIngress;

    float avgInt = 0.0f;
    for (int i = 0; i < COMPARTMENT_COUNT; i++) avgInt += g_sub.compartments[i].integrity;
    avgInt /= 4.0f;
    g_sub.hull = min(g_sub.hull, avgInt);

    float baseDrain = 0.3f;
    if (g_sub.searchlights) baseDrain += (0.8f / g_sub.upgradeLights);
    if (g_sub.clawDeployed) baseDrain += 0.4f;
    if (g_sub.throttleMode == 2) baseDrain += 1.2f;
    if (g_sub.throttleMode == 3) baseDrain += 3.5f;
    if (g_sub.bilgePumpMode == 6) baseDrain += 2.2f;
    else if (g_sub.bilgePumpMode > 0) baseDrain += 0.6f;
    if (g_sub.scrubberAuto) baseDrain += 0.4f;
    if (g_sub.autopilot) baseDrain += 0.3f;
    if (g_sub.lowPowerMode) baseDrain *= 0.45f;

    baseDrain *= g_sub.powerDrainMult * (1.0f - fuelRes->drainSave);
    g_sub.powerDrain = baseDrain;

    if (g_sub.depth > 0.0f) {
        g_sub.battery = max(0.0f, g_sub.battery - (baseDrain * 0.015f * dt));
        if (g_sub.passiveBatteryRegen > 0.0f) {
            g_sub.battery = min(100.0f, g_sub.battery + g_sub.passiveBatteryRegen * dt * 2.0f);
        }
        if (fuelRes->ventCharge > 0.0f && g_sub.currentSectorIdx == 2) {
            g_sub.battery = min(100.0f, g_sub.battery + fuelRes->ventCharge * dt * 1.5f);
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

    // Phase 12: Undersea Combat, Torpedoes & Threat AI Simulation
    if (g_sub.shockwaveCooldown > 0.0f) {
        g_sub.shockwaveCooldown = max(0.0f, g_sub.shockwaveCooldown - dt);
    }

    // Torpedo tubes reload timers
    for (int i = 0; i < 4; i++) {
        if (g_sub.tubes[i].status == 1) {
            g_sub.tubes[i].reloadTime = max(0.0f, g_sub.tubes[i].reloadTime - dt);
            if (g_sub.tubes[i].reloadTime <= 0.0f) {
                g_sub.tubes[i].status = 0; // ready
                PlaySoundAsync(880, 80);
            }
        }
    }

    // Active Decoys Update
    for (int i = g_sub.activeDecoyCount - 1; i >= 0; i--) {
        ActiveDecoy* dec = &g_sub.activeDecoys[i];
        dec->life -= dt;
        dec->pulseRadius = fmodf(dec->pulseRadius + dt * 35.0f, 60.0f);
        if (dec->life <= 0.0f) {
            for (int k = i; k < g_sub.activeDecoyCount - 1; k++) {
                g_sub.activeDecoys[k] = g_sub.activeDecoys[k + 1];
            }
            g_sub.activeDecoyCount--;
        }
    }

    // Active Torpedoes Update
    for (int i = g_sub.activeTorpedoCount - 1; i >= 0; i--) {
        ActiveTorpedo* torp = &g_sub.activeTorpedoes[i];
        torp->life -= dt;

        // Seeker homing towards locked threat
        if (torp->targetIdx >= 0 && torp->targetIdx < THREAT_COUNT) {
            HostileThreat* targetThreat = &g_threats[torp->targetIdx];
            if (!targetThreat->defeated) {
                float tDx = targetThreat->x - torp->x;
                float tDy = targetThreat->y - torp->y;
                float tDist = sqrtf(tDx * tDx + tDy * tDy);
                if (tDist > 0.001f) {
                    float desiredAngle = atan2f(tDy, tDx);
                    float currentAngle = atan2f(torp->vy, torp->vx);
                    float angleDiff = desiredAngle - currentAngle;
                    while (angleDiff < -3.14159265f) angleDiff += 6.2831853f;
                    while (angleDiff > 3.14159265f) angleDiff -= 6.2831853f;

                    float turnSpeed = 3.0f * dt;
                    float step = (angleDiff > 0 ? 1.0f : -1.0f) * min(fabsf(angleDiff), turnSpeed);
                    float newAngle = currentAngle + step;
                    float spd = sqrtf(torp->vx * torp->vx + torp->vy * torp->vy);
                    torp->vx = cosf(newAngle) * spd;
                    torp->vy = sinf(newAngle) * spd;
                }
                float dZ = targetThreat->depth - torp->depth;
                torp->vz += (dZ > 0 ? 1.0f : -1.0f) * min(fabsf(dZ), 35.0f * dt);
            }
        }

        torp->x += torp->vx * dt;
        torp->y += torp->vy * dt;
        torp->depth += torp->vz * dt;

        // Record trail
        if (torp->trailCount < 12) {
            torp->trailX[torp->trailCount] = torp->x;
            torp->trailY[torp->trailCount] = torp->y;
            torp->trailCount++;
        } else {
            for (int k = 0; k < 11; k++) {
                torp->trailX[k] = torp->trailX[k + 1];
                torp->trailY[k] = torp->trailY[k + 1];
            }
            torp->trailX[11] = torp->x;
            torp->trailY[11] = torp->y;
        }

        // Collision check with any threat in sector
        int hit = 0;
        for (int j = 0; j < THREAT_COUNT; j++) {
            HostileThreat* thr = &g_threats[j];
            if (thr->defeated) continue;

            float dx = (thr->x - torp->x) * 1000.0f;
            float dy = (thr->y - torp->y) * 1000.0f;
            float distM = sqrtf(dx * dx + dy * dy);
            float depthDiff = fabsf(thr->depth - torp->depth);

            if (distM <= 40.0f && depthDiff <= 65.0f) {
                hit = 1;
                thr->hp = max(0, thr->hp - torp->damage);
                if (g_ordDefs[torp->type].stunDuration > 0.0f) {
                    thr->stunTimer = g_ordDefs[torp->type].stunDuration;
                }

                if (g_sub.explosionCount < 8) {
                    ExplosionEffect* exp = &g_sub.explosions[g_sub.explosionCount++];
                    exp->x = thr->x;
                    exp->y = thr->y;
                    exp->radius = 8.0f;
                    exp->maxRadius = g_ordDefs[torp->type].blastRadius * 1.5f;
                    exp->life = 0.9f;
                    exp->color = (torp->type == 0 ? RGB(0, 240, 255) : (torp->type == 1 ? RGB(16, 185, 129) : RGB(239, 68, 68)));
                    snprintf(exp->text, sizeof(exp->text), "-%d HP", torp->damage);
                }

                PlayTorpedoExplosionSound();
                char hMsg[128];
                snprintf(hMsg, sizeof(hMsg), "💥 DIRECT TORPEDO HIT: [%s] struck [%s] for %d DMG! (%d/%d HP)",
                         g_ordDefs[torp->type].name, thr->name, torp->damage, thr->hp, thr->maxHp);
                AddLog(hMsg, th->accentEmerald);

                if (thr->hp <= 0 && !thr->defeated) {
                    DefeatThreat(j);
                }
                break;
            }
        }

        if (hit || torp->life <= 0.0f) {
            for (int k = i; k < g_sub.activeTorpedoCount - 1; k++) {
                g_sub.activeTorpedoes[k] = g_sub.activeTorpedoes[k + 1];
            }
            g_sub.activeTorpedoCount--;
        }
    }

    // Hostile Threats AI Simulation
    float noise = fabsf(g_sub.speed) * 20.0f;
    if (g_sub.isPinging) noise += 120.0f;
    if (g_sub.searchlights) noise += 40.0f;
    if (g_sub.silentRunning) noise *= 0.25f;

    for (int i = 0; i < THREAT_COUNT; i++) {
        HostileThreat* thr = &g_threats[i];
        if (thr->defeated) continue;

        if (thr->stunTimer > 0.0f) {
            thr->state = 4; // stunned
            thr->stunTimer = max(0.0f, thr->stunTimer - dt);
            continue;
        }

        if (thr->atkCooldown > 0.0f) {
            thr->atkCooldown = max(0.0f, thr->atkCooldown - dt);
        }

        // Nearest decoy check
        ActiveDecoy* nearestDec = NULL;
        float nearestDecDist = 999999.0f;
        for (int d = 0; d < g_sub.activeDecoyCount; d++) {
            ActiveDecoy* dec = &g_sub.activeDecoys[d];
            float ddx = (dec->x - thr->x) * 1000.0f;
            float ddy = (dec->y - thr->y) * 1000.0f;
            float ddist = sqrtf(ddx * ddx + ddy * ddy);
            if (ddist < 550.0f && ddist < nearestDecDist) {
                nearestDecDist = ddist;
                nearestDec = dec;
            }
        }

        if (nearestDec) {
            thr->state = 3; // distracted
            float decDx = nearestDec->x - thr->x;
            float decDy = nearestDec->y - thr->y;
            float decDist = sqrtf(decDx * decDx + decDy * decDy);
            if (decDist > 0.01f) {
                thr->x += (decDx / decDist) * 0.08f * dt;
                thr->y += (decDy / decDist) * 0.08f * dt;
            }
            continue;
        }

        // Submarine distance calculation
        float sDx = (g_sub.posX - thr->x) * 1000.0f;
        float sDy = (g_sub.posY - thr->y) * 1000.0f;
        float sDist = sqrtf(sDx * sDx + sDy * sDy);
        float sDepthDiff = fabsf(g_sub.depth - thr->depth);

        float effectiveDetectRange = thr->detectRange + (noise * 0.5f);

        if (sDist <= effectiveDetectRange && sDepthDiff <= 300.0f) {
            if (sDist <= thr->atkRange && sDepthDiff <= 80.0f) {
                thr->state = 2; // attacking
                if (thr->atkCooldown <= 0.0f) {
                    thr->atkCooldown = 4.0f;
                    float dmgMult = max(0.3f, 1.0f - (g_sub.upgradeHull - 1) * 0.18f);
                    float dmg = thr->atkPower * dmgMult;
                    g_sub.hull = max(0.0f, g_sub.hull - dmg);

                    int rBay = rand() % COMPARTMENT_COUNT;
                    g_sub.compartments[rBay].integrity = max(0.0f, g_sub.compartments[rBay].integrity - dmg * 1.5f);
                    if (g_sub.compartments[rBay].integrity < 60.0f && g_sub.compartments[rBay].breachTier < 3 && (rand() % 100) < 50) {
                        g_sub.compartments[rBay].breachTier++;
                        PlayAlarmKlaxon();
                        char aMsg[128];
                        snprintf(aMsg, sizeof(aMsg), "🚨 COMBAT BREACH: [%s] pierced [%s] bulkheads! Seawater ingress!", thr->name, g_sub.compartments[rBay].shortCode);
                        AddLog(aMsg, th->accentRed);
                    }

                    PlayAlarmKlaxon();
                    PlaySoundAsync(130, 250);
                    char atMsg[128];
                    snprintf(atMsg, sizeof(atMsg), "⚠️ HOSTILE STRIKE: [%s] rammed hull for %.1f DMG! Hull: %.1f%%", thr->name, dmg, g_sub.hull);
                    AddLog(atMsg, th->accentRed);
                }
            } else {
                thr->state = 1; // stalking
                float sLen = sqrtf(sDx * sDx + sDy * sDy);
                if (sLen > 0.01f) {
                    thr->x += (sDx / sLen) * 0.06f * dt;
                    thr->y += (sDy / sLen) * 0.06f * dt;
                }
            }
        } else {
            thr->state = 0; // patrol
            thr->x += thr->vx * dt * 0.15f;
            thr->y += thr->vy * dt * 0.15f;
            if (fabsf(thr->x) > 14.0f) thr->vx *= -1.0f;
            if (fabsf(thr->y) > 14.0f) thr->vy *= -1.0f;
        }
    }

    // Explosions Update
    for (int i = g_sub.explosionCount - 1; i >= 0; i--) {
        ExplosionEffect* exp = &g_sub.explosions[i];
        exp->life -= dt;
        exp->radius += dt * 50.0f;
        if (exp->life <= 0.0f) {
            for (int k = i; k < g_sub.explosionCount - 1; k++) {
                g_sub.explosions[k] = g_sub.explosions[k + 1];
            }
            g_sub.explosionCount--;
        }
    }
}

void FireTorpedoTube(int tubeIdx) {
    if (tubeIdx < 0 || tubeIdx >= 4) return;
    TorpedoTube* tube = &g_sub.tubes[tubeIdx];
    if (tube->status != 0) return;

    const OrdnanceDef* ord = &g_ordDefs[tube->type];
    int ammo = (tube->type == 0 ? g_sub.torpedoes : (tube->type == 1 ? g_sub.empTorpedoes : g_sub.plasmaTorpedoes));
    if (ammo <= 0) {
        PlaySoundAsync(200, 150);
        char bMsg[128];
        snprintf(bMsg, sizeof(bMsg), "FIRE ABORTED: Tube #%d out of %s ammo! Restock at outposts.", tube->id, ord->name);
        AddLog(bMsg, g_themes[g_sub.currentTheme].accentAmber);
        return;
    }

    if (tube->type == 0) g_sub.torpedoes--;
    else if (tube->type == 1) g_sub.empTorpedoes--;
    else g_sub.plasmaTorpedoes--;

    tube->status = 1; // reloading
    tube->reloadTime = ord->reloadTime;

    if (g_sub.activeTorpedoCount < 8) {
        ActiveTorpedo* torp = &g_sub.activeTorpedoes[g_sub.activeTorpedoCount++];
        torp->x = g_sub.posX;
        torp->y = g_sub.posY;
        torp->depth = g_sub.depth;
        float hRad = (g_sub.heading - 90.0f) * (3.14159265f / 180.0f);
        torp->vx = cosf(hRad) * ord->speed;
        torp->vy = sinf(hRad) * ord->speed;
        torp->vz = (g_sub.pitch / 15.0f) * 15.0f;
        torp->type = tube->type;
        torp->damage = ord->damage;
        torp->targetIdx = g_sub.selectedThreatIdx;
        torp->life = 6.0f;
        torp->trailCount = 0;
    }

    if (tube->type == 2) PlayPlasmaLaunchSound();
    else PlayTorpedoLaunchSound();
    char lMsg[128];
    snprintf(lMsg, sizeof(lMsg), "🚀 TORPEDO LAUNCH: Tube #%d fired [%s]! Seeker acquiring target...", tube->id, ord->name);
    AddLog(lMsg, g_themes[g_sub.currentTheme].accentSonar);
}

void CycleTubeOrdnance(int tubeIdx) {
    if (tubeIdx < 0 || tubeIdx >= 4) return;
    g_sub.tubes[tubeIdx].type = (g_sub.tubes[tubeIdx].type + 1) % 3;
    PlaySoundAsync(520, 60);
    char cMsg[128];
    snprintf(cMsg, sizeof(cMsg), "Tube #%d re-armed with [%s].", g_sub.tubes[tubeIdx].id, g_ordDefs[g_sub.tubes[tubeIdx].type].name);
    AddLog(cMsg, g_themes[g_sub.currentTheme].textPrimary);
}

void LaunchAcousticDecoy(void) {
    if (g_sub.decoys <= 0) {
        PlaySoundAsync(220, 150);
        AddLog("COUNTERMEASURES EMPTY: No acoustic decoys remaining in launcher!", g_themes[g_sub.currentTheme].accentAmber);
        return;
    }
    g_sub.decoys--;
    if (g_sub.activeDecoyCount < 4) {
        ActiveDecoy* dec = &g_sub.activeDecoys[g_sub.activeDecoyCount++];
        dec->x = g_sub.posX;
        dec->y = g_sub.posY;
        dec->depth = g_sub.depth;
        dec->life = 14.0f;
        dec->pulseRadius = 0.0f;
    }
    PlayDecoySound();
    char dMsg[128];
    snprintf(dMsg, sizeof(dMsg), "🚨 ACOUSTIC DECOY DEPLOYED! Emitting 180dB cavitation signature... (%d left)", g_sub.decoys);
    AddLog(dMsg, g_themes[g_sub.currentTheme].accentEmerald);
}

void TriggerShockwaveDischarge(void) {
    if (g_sub.battery < 15.0f) {
        PlaySoundAsync(220, 150);
        AddLog("SHOCKWAVE ERROR: Insufficient battery power (<15%)!", g_themes[g_sub.currentTheme].accentAmber);
        return;
    }
    if (g_sub.shockwaveCooldown > 0.0f) {
        PlaySoundAsync(240, 80);
        return;
    }

    g_sub.battery = max(0.0f, g_sub.battery - 15.0f);
    g_sub.shockwaveCooldown = 10.0f;
    PlayShockwaveSound();

    if (g_sub.explosionCount < 8) {
        ExplosionEffect* exp = &g_sub.explosions[g_sub.explosionCount++];
        exp->x = g_sub.posX;
        exp->y = g_sub.posY;
        exp->radius = 10.0f;
        exp->maxRadius = 180.0f;
        exp->life = 1.2f;
        exp->color = RGB(0, 240, 255);
        strncpy(exp->text, "⚡ EMP SHOCKWAVE", sizeof(exp->text) - 1);
    }

    int hits = 0;
    for (int i = 0; i < THREAT_COUNT; i++) {
        HostileThreat* t = &g_threats[i];
        if (t->defeated) continue;
        float dx = (t->x - g_sub.posX) * 1000.0f;
        float dy = (t->y - g_sub.posY) * 1000.0f;
        float distM = sqrtf(dx * dx + dy * dy);
        float depthDiff = fabsf(g_sub.depth - t->depth);

        if (distM <= 220.0f && depthDiff <= 100.0f) {
            t->hp = max(0, t->hp - 60);
            t->stunTimer = 6.0f;
            hits++;
            if (t->hp <= 0 && !t->defeated) {
                DefeatThreat(i);
            }
        }
    }

    char sMsg[128];
    snprintf(sMsg, sizeof(sMsg), "⚡ EMP SHOCKWAVE DISCHARGED: 360-deg pulse emitted! %d hostiles stunned.", hits);
    AddLog(sMsg, g_themes[g_sub.currentTheme].accentEmerald);
}

void ToggleSilentRunning(void) {
    g_sub.silentRunning = !g_sub.silentRunning;
    PlaySoundAsync(g_sub.silentRunning ? 420 : 320, 80);
    char sMsg[128];
    snprintf(sMsg, sizeof(sMsg), "Silent Running %s (Acoustic emissions dampened -75%%).", g_sub.silentRunning ? "ENGAGED" : "DISENGAGED");
    AddLog(sMsg, g_themes[g_sub.currentTheme].textPrimary);
}

void LockThreatTarget(int tIdx) {
    if (tIdx < 0 || tIdx >= THREAT_COUNT) return;
    g_sub.selectedThreatIdx = tIdx;
    PlaySoundAsync(750, 60);
    char tMsg[128];
    snprintf(tMsg, sizeof(tMsg), "🎯 Combat fire-control locked onto [%s].", g_threats[tIdx].name);
    AddLog(tMsg, g_themes[g_sub.currentTheme].accentSonar);
}

void DefeatThreat(int tIdx) {
    if (tIdx < 0 || tIdx >= THREAT_COUNT) return;
    HostileThreat* t = &g_threats[tIdx];
    t->defeated = 1;
    t->hp = 0;
    int bonusPts = (int)(t->bountyCredits * g_sub.surveyMultiplier);
    g_sub.surveyPoints += bonusPts;

    if (t->bountyMineral >= 0) {
        if (t->bountyMineral == 0) g_sub.cargoManganese += t->bountyQty;
        else if (t->bountyMineral == 1) g_sub.cargoSunkenGold += t->bountyQty;
        else if (t->bountyMineral == 2) g_sub.cargoTitaniumScrap += t->bountyQty;
        else if (t->bountyMineral == 3) g_sub.cargoSmokerCrystals += t->bountyQty;
        else if (t->bountyMineral == 4) g_sub.cargoHadalPrisms += t->bountyQty;
        RecalculateCargo();
    }
    if (t->bountyBio >= 0) {
        if (t->bountyBio == 0) g_sub.bioPlankton += 1;
        else if (t->bountyBio == 1) g_sub.bioCephalopod += 1;
        else if (t->bountyBio == 2) g_sub.bioEnzymes += 1;
        else if (t->bountyBio == 3) g_sub.bioHadal += 1;
    }

    PlayTorpedoExplosionSound();
    PlayMineralChime();
    char bMsg[128];
    snprintf(bMsg, sizeof(bMsg), "🏆 THREAT DESTROYED: [%s] neutralized! Bounty: +%d Credits.", t->name, bonusPts);
    AddLog(bMsg, g_themes[g_sub.currentTheme].accentEmerald);
}

void FireAtThreat(int tIdx) {
    LockThreatTarget(tIdx);
    for (int i = 0; i < 4; i++) {
        TorpedoTube* tube = &g_sub.tubes[i];
        int ammo = (tube->type == 0 ? g_sub.torpedoes : (tube->type == 1 ? g_sub.empTorpedoes : g_sub.plasmaTorpedoes));
        if (tube->status == 0 && ammo > 0) {
            FireTorpedoTube(i);
            return;
        }
    }
    PlaySoundAsync(200, 150);
    AddLog("ALL TUBES BUSY OR EMPTY: Reload in progress or out of torpedoes.", g_themes[g_sub.currentTheme].accentAmber);
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

    
    // Outposts on Nav Map
    for (int i = 0; i < OUTPOST_COUNT; i++) {
        const OutpostInfo* out = &g_outposts[i];
        int ox = cx + (int)((out->x - g_sub.posX) * scale);
        int oy = cy + (int)((out->y - g_sub.posY) * scale);
        if (ox >= rcMap.left + 5 && ox <= rcMap.right - 5 && oy >= rcMap.top + 5 && oy <= rcMap.bottom - 5) {
            int isDocked = (g_sub.isDocked && g_sub.dockedStationIdx == i);
            HBRUSH hBrOut = CreateSolidBrush(isDocked ? th->accentEmerald : th->accentSonar);
            SelectObject(hdc, hBrOut);
            RECT rcO = { ox - 5, oy - 5, ox + 5, oy + 5 };
            FillRect(hdc, &rcO, hBrOut);
            DeleteObject(hBrOut);

            SetTextColor(hdc, isDocked ? th->accentEmerald : th->textBright);
            TextOutA(hdc, ox + 7, oy - 6, out->name, (int)strlen(out->name));
        }
    }

    // Salvage Nodes on Nav Map
    for (int i = 0; i < SALVAGE_NODE_COUNT; i++) {
        const SalvageNode* sn = &g_salvageNodes[i];
        int sx = cx + (int)((sn->x - g_sub.posX) * scale);
        int sy = cy + (int)((sn->y - g_sub.posY) * scale);

        if (sx >= rcMap.left + 5 && sx <= rcMap.right - 5 && sy >= rcMap.top + 5 && sy <= rcMap.bottom - 5) {
            HBRUSH hBrSn = CreateSolidBrush(sn->harvested ? th->borderPanel : RGB(251, 191, 36));
            SelectObject(hdc, hBrSn);
            POINT pts[4] = { { sx, sy - 5 }, { sx + 5, sy }, { sx, sy + 5 }, { sx - 5, sy } };
            Polygon(hdc, pts, 4);
            DeleteObject(hBrSn);

            if (!sn->harvested) {
                SetTextColor(hdc, RGB(251, 191, 36));
                TextOutA(hdc, sx + 7, sy - 6, sn->name, (int)strlen(sn->name));
            }
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

// --- DRAW CARGO HOLD & RESOURCE SALVAGE VIEW (PHASE 8) ---
void DrawCargoHoldView(HDC hdc, int x, int y, int w, int h, const SubmarineTheme* th) {
    RECT rcBg = { x, y, x + w, y + h };
    HBRUSH hBr = CreateSolidBrush(th->bgDeep);
    FillRect(hdc, &rcBg, hBr);
    DeleteObject(hBr);

    RecalculateCargo();
    int pct = (int)((g_sub.cargoTotalWeight / g_sub.cargoMaxWeight) * 100.0f);

    char hdrBuf[128];
    snprintf(hdrBuf, sizeof(hdrBuf), "HOLD PAYLOAD: %.0f / %.0f KG (%d%%) | EST. VALUE: %d PTS",
             g_sub.cargoTotalWeight, g_sub.cargoMaxWeight, pct, g_sub.cargoTotalValue);

    SelectObject(hdc, g_hFontBold);
    SetTextColor(hdc, th->accentAmber);
    TextOutA(hdc, x + 10, y + 6, hdrBuf, (int)strlen(hdrBuf));

    // Transship Button in Header
    char offloadBuf[64];
    snprintf(offloadBuf, sizeof(offloadBuf), "TRANSSHIP (+%d PTS)", g_sub.cargoTotalValue);
    DrawCustomButton(hdc, ID_BTN_OFFLOAD_CARGO, x + w - 170, y + 4, 160, 20, offloadBuf, 0, g_sub.cargoTotalValue > 0 ? th->accentEmerald : th->textDim, th);

    // Left Column: 5 Resource Categories Grid
    int leftGridW = (w * 54) / 100;
    int rightListW = w - leftGridW - 14;
    int leftGridX = x + 6;
    int rightListX = leftGridX + leftGridW + 8;
    int contentY = y + 28;
    int contentH = h - 34;

    int itemH = (contentH - 8) / 5;
    for (int i = 0; i < 5; i++) {
        const ResourceDef* r = &g_resDefs[i];
        int qty = (i == 0 ? g_sub.cargoManganese :
                  (i == 1 ? g_sub.cargoSunkenGold :
                  (i == 2 ? g_sub.cargoTitaniumScrap :
                  (i == 3 ? g_sub.cargoSmokerCrystals : g_sub.cargoHadalPrisms))));

        float curWeight = qty * r->unitWeight;
        int curVal = qty * r->unitVal;

        int iy = contentY + i * (itemH + 2);
        RECT rcCard = { leftGridX, iy, leftGridX + leftGridW, iy + itemH };
        HBRUSH hBrP = CreateSolidBrush(th->bgPanel);
        HBRUSH hBrB = CreateSolidBrush(qty > 0 ? th->accentAmber : th->borderPanel);
        FillRect(hdc, &rcCard, hBrP);
        FrameRect(hdc, &rcCard, hBrB);
        DeleteObject(hBrP);
        DeleteObject(hBrB);

        SelectObject(hdc, g_hFontBold);
        SetTextColor(hdc, th->textBright);
        TextOutA(hdc, leftGridX + 6, iy + 3, r->name, (int)strlen(r->name));

        char qBuf[32];
        snprintf(qBuf, sizeof(qBuf), "x%d", qty);
        SetTextColor(hdc, qty > 0 ? th->accentAmber : th->textDim);
        SIZE sz;
        GetTextExtentPoint32A(hdc, qBuf, (int)strlen(qBuf), &sz);
        TextOutA(hdc, leftGridX + leftGridW - sz.cx - 8, iy + 3, qBuf, (int)strlen(qBuf));

        SelectObject(hdc, g_hFontSmall);
        SetTextColor(hdc, th->textDim);
        TextOutA(hdc, leftGridX + 6, iy + 16, r->desc, (int)strlen(r->desc));

        char sBuf[64];
        snprintf(sBuf, sizeof(sBuf), "Payload: %.0f KG   Market: +%d PTS", curWeight, curVal);
        SetTextColor(hdc, th->textPrimary);
        TextOutA(hdc, leftGridX + 6, iy + itemH - 14, sBuf, (int)strlen(sBuf));
    }

    // Right Column: Salvage Nodes in Sector
    SelectObject(hdc, g_hFontBold);
    SetTextColor(hdc, th->textBright);
    TextOutA(hdc, rightListX + 4, contentY + 2, "SECTOR SALVAGE NODES", 20);

    int nodeItemH = (contentH - 24) / 4;
    int snCount = 0;
    for (int i = 0; i < SALVAGE_NODE_COUNT; i++) {
        SalvageNode* sn = &g_salvageNodes[i];
        if (sn->sectorIdx != g_sub.currentSectorIdx) continue;

        int ny = contentY + 20 + snCount * (nodeItemH + 4);
        RECT rcNode = { rightListX, ny, rightListX + rightListW, ny + nodeItemH };
        HBRUSH hBrP = CreateSolidBrush(th->bgPanel);
        HBRUSH hBrB = CreateSolidBrush(sn->harvested ? th->borderPanel : th->accentAmber);
        FillRect(hdc, &rcNode, hBrP);
        FrameRect(hdc, &rcNode, hBrB);
        DeleteObject(hBrP);
        DeleteObject(hBrB);

        float dx = sn->x - g_sub.posX;
        float dy = sn->y - g_sub.posY;
        float distM = sqrtf(dx * dx + dy * dy) * 1000.0f;
        float depthDiff = fabsf(g_sub.depth - sn->depth);

        SelectObject(hdc, g_hFontBold);
        SetTextColor(hdc, sn->harvested ? th->textDim : th->textBright);
        TextOutA(hdc, rightListX + 6, ny + 4, sn->name, (int)strlen(sn->name));

        SelectObject(hdc, g_hFontSmall);
        SetTextColor(hdc, th->textDim);
        char dBuf[64];
        snprintf(dBuf, sizeof(dBuf), "Depth: %.0fm | Dist: %.0fm (ΔZ: %.0fm)", sn->depth, distM, depthDiff);
        TextOutA(hdc, rightListX + 6, ny + 18, dBuf, (int)strlen(dBuf));

        if (sn->harvested) {
            SetTextColor(hdc, th->textDim);
            TextOutA(hdc, rightListX + 6, ny + nodeItemH - 14, "[HARVESTED / DEPLETED]", 22);
        } else {
            char yBuf[64];
            snprintf(yBuf, sizeof(yBuf), "YIELD: %dx %s (+%d PTS)", sn->qty, g_resDefs[sn->resKey].name, (int)(sn->val * g_sub.surveyMultiplier));
            SetTextColor(hdc, th->accentAmber);
            TextOutA(hdc, rightListX + 6, ny + nodeItemH - 14, yBuf, (int)strlen(yBuf));
        }

        snCount++;
        if (snCount >= 4) break;
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

// --- DRAW DAMAGE CONTROL & BULKHEAD FLOOD VIEW (PHASE 9) ---
void DrawDamageControlView(HDC hdc, int x, int y, int w, int h, const SubmarineTheme* th) {
    RECT rcBg = { x, y, x + w, y + h };
    HBRUSH hBr = CreateSolidBrush(th->bgDeep);
    FillRect(hdc, &rcBg, hBr);
    DeleteObject(hBr);

    float totalWater = 0.0f;
    float totalIngress = 0.0f;
    for (int i = 0; i < COMPARTMENT_COUNT; i++) {
        totalWater += g_sub.compartments[i].water;
        totalIngress += g_sub.compartments[i].leakRate;
    }
    int floodPct = (int)((totalWater / 1000.0f) * 100.0f);

    char hdrBuf[128];
    snprintf(hdrBuf, sizeof(hdrBuf), "BILGE: %.1f/1000 GAL (%d%%) | INGRESS: %.1f GPM", totalWater, floodPct, totalIngress);
    SelectObject(hdc, g_hFontBold);
    SetTextColor(hdc, totalWater > 200.0f ? th->accentRed : (totalWater > 50.0f ? th->accentAmber : th->accentEmerald));
    TextOutA(hdc, x + 8, y + 6, hdrBuf, (int)strlen(hdrBuf));

    // Top Action Buttons
    char bleedBuf[64];
    snprintf(bleedBuf, sizeof(bleedBuf), "CABIN: %.2f ATM [%s]", g_sub.cabinPressure, g_sub.bleedValveOpen ? "BLEED" : "SEAL");
    DrawCustomButton(hdc, ID_BTN_BLEED_VALVE, x + w - 380, y + 4, 130, 20, bleedBuf, g_sub.bleedValveOpen, th->accentSonar, th);

    const char* pumpModeLabels[7] = { "OFF", "AUTO", "BOW", "CMD", "ENG", "AFT", "2.5x OVERDRIVE" };
    char pumpBuf[64];
    snprintf(pumpBuf, sizeof(pumpBuf), "PUMP: %s", pumpModeLabels[g_sub.bilgePumpMode]);
    DrawCustomButton(hdc, ID_BTN_PUMP_MODE, x + w - 244, y + 4, 130, 20, pumpBuf, g_sub.bilgePumpMode == 6, g_sub.bilgePumpMode == 6 ? th->accentRed : th->accentEmerald, th);

    DrawCustomButton(hdc, ID_BTN_SIM_BREACH, x + w - 108, y + 4, 100, 20, "TEST DRILL", 0, th->accentAmber, th);

    int contentY = y + 28;
    int contentH = h - 34;

    // 4 Compartment Bay Panels
    int margin = 6;
    int cardW = (w - margin * 5) / 4;
    int cardH = contentH - 4;

    const char* tierNames[4] = { "INTACT", "HAIRLINE", "RUPTURE", "TORRENTIAL" };

    for (int i = 0; i < COMPARTMENT_COUNT; i++) {
        CompartmentInfo* comp = &g_sub.compartments[i];
        int cx = x + margin + i * (cardW + margin);
        int cy = contentY;

        RECT rcCard = { cx, cy, cx + cardW, cy + cardH };
        HBRUSH hBrP = CreateSolidBrush(th->bgPanel);
        COLORREF bClr = comp->breachTier > 0 ? th->accentRed : (comp->water > 0.0f ? th->accentAmber : th->borderPanel);
        HBRUSH hBrB = CreateSolidBrush(bClr);
        FillRect(hdc, &rcCard, hBrP);
        FrameRect(hdc, &rcCard, hBrB);
        DeleteObject(hBrP);
        DeleteObject(hBrB);

        // Header: Code & Status
        SelectObject(hdc, g_hFontBold);
        SetTextColor(hdc, th->textBright);
        TextOutA(hdc, cx + 6, cy + 4, comp->shortCode, (int)strlen(comp->shortCode));

        COLORREF tClr = comp->breachTier == 0 ? th->accentEmerald : (comp->breachTier == 1 ? th->accentAmber : th->accentRed);
        SetTextColor(hdc, tClr);
        const char* tName = tierNames[comp->breachTier];
        SIZE tSz;
        GetTextExtentPoint32A(hdc, tName, (int)strlen(tName), &tSz);
        TextOutA(hdc, cx + cardW - tSz.cx - 6, cy + 4, tName, (int)strlen(tName));

        // Compartment Full Name
        SelectObject(hdc, g_hFontSmall);
        SetTextColor(hdc, th->textDim);
        TextOutA(hdc, cx + 6, cy + 18, comp->name, (int)strlen(comp->name));

        // Water Chamber Visual Box
        int chY = cy + 32;
        int chH = 44;
        int chW = cardW - 12;
        RECT rcChamber = { cx + 6, chY, cx + 6 + chW, chY + chH };
        HBRUSH hBrChBg = CreateSolidBrush(th->gaugeBg);
        FillRect(hdc, &rcChamber, hBrChBg);
        FrameRect(hdc, &rcChamber, hBrB);
        DeleteObject(hBrChBg);

        int fillH = (int)((comp->water / comp->maxWater) * chH);
        if (fillH > 0) {
            RECT rcFill = { cx + 7, chY + chH - fillH, cx + 5 + chW, chY + chH - 1 };
            HBRUSH hBrFill = CreateSolidBrush(RGB(2, 132, 199));
            FillRect(hdc, &rcFill, hBrFill);
            DeleteObject(hBrFill);
        }

        char wTxt[64];
        snprintf(wTxt, sizeof(wTxt), "%.1f/250 GAL (%d%%)", comp->water, (int)((comp->water / comp->maxWater) * 100.0f));
        SetTextColor(hdc, RGB(255, 255, 255));
        SIZE wSz;
        GetTextExtentPoint32A(hdc, wTxt, (int)strlen(wTxt), &wSz);
        TextOutA(hdc, cx + 6 + (chW - wSz.cx) / 2, chY + (chH - wSz.cy) / 2, wTxt, (int)strlen(wTxt));

        // Stats rows
        int sy = chY + chH + 6;
        char sBuf[64];
        snprintf(sBuf, sizeof(sBuf), "INTEGRITY: %.1f%%", comp->integrity);
        SetTextColor(hdc, comp->integrity < 60.0f ? th->accentRed : (comp->integrity < 85.0f ? th->accentAmber : th->accentEmerald));
        TextOutA(hdc, cx + 6, sy, sBuf, (int)strlen(sBuf));
        sy += 14;

        snprintf(sBuf, sizeof(sBuf), "INGRESS: %.1f GPM", comp->leakRate);
        SetTextColor(hdc, comp->leakRate > 0.0f ? th->accentRed : th->textDim);
        TextOutA(hdc, cx + 6, sy, sBuf, (int)strlen(sBuf));
        sy += 16;

        // Repair bar
        if (comp->isRepairing) {
            DrawGaugeBar(hdc, cx + 6, sy, chW, 6, comp->repairProgress, th->accentAmber, th);
            sy += 10;
        } else {
            sy += 4;
        }

        // Action Buttons inside Card
        int btnH = 18;
        char dBtn[32];
        snprintf(dBtn, sizeof(dBtn), comp->doorSealed ? "DOOR: SEALED" : "DOOR: OPEN");
        DrawCustomButton(hdc, ID_BTN_DOOR_BAY_0 + i, cx + 6, sy, chW, btnH, dBtn, comp->doorSealed, comp->doorSealed ? th->accentRed : th->accentEmerald, th);
        sy += btnH + 3;

        char aBtn[32];
        snprintf(aBtn, sizeof(aBtn), comp->airDam ? "AIR DAM: ON" : "AIR DAM: OFF");
        DrawCustomButton(hdc, ID_BTN_AIRDAM_BAY_0 + i, cx + 6, sy, chW, btnH, aBtn, comp->airDam, comp->airDam ? th->accentSonar : th->textDim, th);
        sy += btnH + 3;

        int isRouted = (g_sub.bilgePumpMode == i + 2);
        DrawCustomButton(hdc, ID_BTN_ROUTE_BAY_0 + i, cx + 6, sy, chW, btnH, isRouted ? "PUMP PRIORITY" : "ROUTE PUMP", isRouted, isRouted ? th->accentEmerald : th->textPrimary, th);
        sy += btnH + 3;

        char pBtn[32];
        if (comp->isRepairing) snprintf(pBtn, sizeof(pBtn), "WELDING %d%%", (int)comp->repairProgress);
        else snprintf(pBtn, sizeof(pBtn), "WELD PATCH");
        DrawCustomButton(hdc, ID_BTN_PATCH_BAY_0 + i, cx + 6, sy, chW, btnH, pBtn, comp->isRepairing, th->accentAmber, th);
    }
}

// --- DRAW RESEARCH LAB & DEEP-SEA BIOLOGY VIEW (PHASE 10) ---
void DrawResearchLabView(HDC hdc, int x, int y, int w, int h, const SubmarineTheme* th) {
    RECT rcBg = { x, y, x + w, y + h };
    HBRUSH hBr = CreateSolidBrush(th->bgDeep);
    FillRect(hdc, &rcBg, hBr);
    DeleteObject(hBr);

    // Specimen Reservoir Header Bar
    char bioBuf[160];
    snprintf(bioBuf, sizeof(bioBuf), "🧬 BIO-SAMPLES: %d Plankton | %d Cephalopod | %d Enzymes | %d Hadal  |  CREDITS: %d PTS",
             g_sub.bioPlankton, g_sub.bioCephalopod, g_sub.bioEnzymes, g_sub.bioHadal, g_sub.surveyPoints);

    SelectObject(hdc, g_hFontBold);
    SetTextColor(hdc, th->accentEmerald);
    TextOutA(hdc, x + 8, y + 6, bioBuf, (int)strlen(bioBuf));

    // Incubate Button in Header
    int canIncubate = (g_sub.surveyPoints >= 45);
    DrawCustomButton(hdc, ID_BTN_INCUBATE_BIO, x + w - 190, y + 4, 180, 20, "INCUBATE (+2 BIO / 45 PTS)", 0, canIncubate ? th->accentEmerald : th->textDim, th);

    int contentY = y + 28;
    int contentH = h - 34;

    int margin = 6;
    int gridW = (w - margin * 3) / 2;
    int gridH = (contentH - margin * 3) / 2;

    int c1x = x + margin;
    int c2x = x + margin * 2 + gridW;
    int r1y = contentY + margin;
    int r2y = contentY + margin * 2 + gridH;

    char buf[128];

    // Project 1: Pressure-Resistant Polymers
    {
        RECT rcCard = { c1x, r1y, c1x + gridW, r1y + gridH };
        HBRUSH hBrP = CreateSolidBrush(th->bgPanel);
        HBRUSH hBrB = CreateSolidBrush(th->borderPanel);
        FillRect(hdc, &rcCard, hBrP);
        FrameRect(hdc, &rcCard, hBrB);
        DeleteObject(hBrP);
        DeleteObject(hBrB);

        int tier = g_sub.resPolymers;
        const PolymerResearch* res = &g_polymerRes[tier - 1];

        SelectObject(hdc, g_hFontBold);
        SetTextColor(hdc, th->textBright);
        TextOutA(hdc, c1x + 8, r1y + 6, "PRESSURE-RESISTANT POLYMERS", 27);

        SelectObject(hdc, g_hFontSmall);
        SetTextColor(hdc, tier == 4 ? th->accentEmerald : th->accentSonar);
        TextOutA(hdc, c1x + 8, r1y + 22, res->name, (int)strlen(res->name));

        SetTextColor(hdc, th->textDim);
        TextOutA(hdc, c1x + 8, r1y + 36, res->desc, (int)strlen(res->desc));

        snprintf(buf, sizeof(buf), "HULL BONUS: +%.0f%%   CRUSH MITIGATION: -%.0f%%", res->bonusHull, res->crushReduction * 100.0f);
        SetTextColor(hdc, th->textPrimary);
        TextOutA(hdc, c1x + 8, r1y + 50, buf, (int)strlen(buf));

        if (tier < 4) {
            const PolymerResearch* nextRes = &g_polymerRes[tier];
            snprintf(buf, sizeof(buf), "RESEARCH -> %s (%d PTS + %d PLANKTON)", nextRes->name, nextRes->cost, nextRes->reqPlankton);
            int canAfford = (g_sub.surveyPoints >= nextRes->cost && g_sub.bioPlankton >= nextRes->reqPlankton);
            DrawCustomButton(hdc, ID_BTN_RES_POLYMERS, c1x + 8, r1y + gridH - 24, gridW - 16, 18, buf, 0, canAfford ? th->accentEmerald : th->textDim, th);
        } else {
            DrawCustomButton(hdc, ID_BTN_RES_POLYMERS, c1x + 8, r1y + gridH - 24, gridW - 16, 18, "MAX TIER [BREAKTHROUGH]", 1, th->accentEmerald, th);
        }
    }

    // Project 2: Bioluminescent Enzymes
    {
        RECT rcCard = { c2x, r1y, c2x + gridW, r1y + gridH };
        HBRUSH hBrP = CreateSolidBrush(th->bgPanel);
        HBRUSH hBrB = CreateSolidBrush(th->borderPanel);
        FillRect(hdc, &rcCard, hBrP);
        FrameRect(hdc, &rcCard, hBrB);
        DeleteObject(hBrP);
        DeleteObject(hBrB);

        int tier = g_sub.resBiolum;
        const BiolumResearch* res = &g_biolumRes[tier - 1];

        SelectObject(hdc, g_hFontBold);
        SetTextColor(hdc, th->textBright);
        TextOutA(hdc, c2x + 8, r1y + 6, "BIOLUMINESCENT ENZYMES", 22);

        SelectObject(hdc, g_hFontSmall);
        SetTextColor(hdc, tier == 4 ? th->accentEmerald : th->accentSonar);
        TextOutA(hdc, c2x + 8, r1y + 22, res->name, (int)strlen(res->name));

        SetTextColor(hdc, th->textDim);
        TextOutA(hdc, c2x + 8, r1y + 36, res->desc, (int)strlen(res->desc));

        snprintf(buf, sizeof(buf), "SONAR RANGE: +%.0fm   SCAN YIELD: %.2fx", res->sonarBonus, res->surveyBonus);
        SetTextColor(hdc, th->textPrimary);
        TextOutA(hdc, c2x + 8, r1y + 50, buf, (int)strlen(buf));

        if (tier < 4) {
            const BiolumResearch* nextRes = &g_biolumRes[tier];
            snprintf(buf, sizeof(buf), "RESEARCH -> %s (%d PTS + %d CEPHALOPOD)", nextRes->name, nextRes->cost, nextRes->reqCephalopod);
            int canAfford = (g_sub.surveyPoints >= nextRes->cost && g_sub.bioCephalopod >= nextRes->reqCephalopod);
            DrawCustomButton(hdc, ID_BTN_RES_BIOLUM, c2x + 8, r1y + gridH - 24, gridW - 16, 18, buf, 0, canAfford ? th->accentEmerald : th->textDim, th);
        } else {
            DrawCustomButton(hdc, ID_BTN_RES_BIOLUM, c2x + 8, r1y + gridH - 24, gridW - 16, 18, "MAX TIER [BREAKTHROUGH]", 1, th->accentEmerald, th);
        }
    }

    // Project 3: Chemosynthetic Bio-Fuel Cells
    {
        RECT rcCard = { c1x, r2y, c1x + gridW, r2y + gridH };
        HBRUSH hBrP = CreateSolidBrush(th->bgPanel);
        HBRUSH hBrB = CreateSolidBrush(th->borderPanel);
        FillRect(hdc, &rcCard, hBrP);
        FrameRect(hdc, &rcCard, hBrB);
        DeleteObject(hBrP);
        DeleteObject(hBrB);

        int tier = g_sub.resBiofuel;
        const BiofuelResearch* res = &g_biofuelRes[tier - 1];

        SelectObject(hdc, g_hFontBold);
        SetTextColor(hdc, th->textBright);
        TextOutA(hdc, c1x + 8, r2y + 6, "CHEMOSYNTHETIC BIO-FUEL CELLS", 29);

        SelectObject(hdc, g_hFontSmall);
        SetTextColor(hdc, tier == 4 ? th->accentEmerald : th->accentSonar);
        TextOutA(hdc, c1x + 8, r2y + 22, res->name, (int)strlen(res->name));

        SetTextColor(hdc, th->textDim);
        TextOutA(hdc, c1x + 8, r2y + 36, res->desc, (int)strlen(res->desc));

        snprintf(buf, sizeof(buf), "LOAD REDUCTION: -%.0f%%   VENT REGEN: %s", res->drainSave * 100.0f, res->ventCharge > 0.0f ? "+0.2%/s NEAR VENTS" : "OFFLINE");
        SetTextColor(hdc, th->textPrimary);
        TextOutA(hdc, c1x + 8, r2y + 50, buf, (int)strlen(buf));

        if (tier < 4) {
            const BiofuelResearch* nextRes = &g_biofuelRes[tier];
            snprintf(buf, sizeof(buf), "RESEARCH -> %s (%d PTS + %d ENZYMES)", nextRes->name, nextRes->cost, nextRes->reqEnzymes);
            int canAfford = (g_sub.surveyPoints >= nextRes->cost && g_sub.bioEnzymes >= nextRes->reqEnzymes);
            DrawCustomButton(hdc, ID_BTN_RES_BIOFUEL, c1x + 8, r2y + gridH - 24, gridW - 16, 18, buf, 0, canAfford ? th->accentEmerald : th->textDim, th);
        } else {
            DrawCustomButton(hdc, ID_BTN_RES_BIOFUEL, c1x + 8, r2y + gridH - 24, gridW - 16, 18, "MAX TIER [BREAKTHROUGH]", 1, th->accentEmerald, th);
        }
    }

    // Project 4: Piezophilic Cellular Regenerator
    {
        RECT rcCard = { c2x, r2y, c2x + gridW, r2y + gridH };
        HBRUSH hBrP = CreateSolidBrush(th->bgPanel);
        HBRUSH hBrB = CreateSolidBrush(th->borderPanel);
        FillRect(hdc, &rcCard, hBrP);
        FrameRect(hdc, &rcCard, hBrB);
        DeleteObject(hBrP);
        DeleteObject(hBrB);

        int tier = g_sub.resRegen;
        const RegenResearch* res = &g_regenRes[tier - 1];

        SelectObject(hdc, g_hFontBold);
        SetTextColor(hdc, th->textBright);
        TextOutA(hdc, c2x + 8, r2y + 6, "PIEZOPHILIC CELL REGENERATOR", 28);

        SelectObject(hdc, g_hFontSmall);
        SetTextColor(hdc, tier == 4 ? th->accentEmerald : th->accentSonar);
        TextOutA(hdc, c2x + 8, r2y + 22, res->name, (int)strlen(res->name));

        SetTextColor(hdc, th->textDim);
        TextOutA(hdc, c2x + 8, r2y + 36, res->desc, (int)strlen(res->desc));

        snprintf(buf, sizeof(buf), "HEALING: +%.1f%%/10s AT DEPTH   AUTO-PATCH: %s", res->autoHeal, res->autoPatch ? "SEALS WEEPS" : "OFFLINE");
        SetTextColor(hdc, th->textPrimary);
        TextOutA(hdc, c2x + 8, r2y + 50, buf, (int)strlen(buf));

        if (tier < 4) {
            const RegenResearch* nextRes = &g_regenRes[tier];
            snprintf(buf, sizeof(buf), "RESEARCH -> %s (%d PTS + %d HADAL)", nextRes->name, nextRes->cost, nextRes->reqHadal);
            int canAfford = (g_sub.surveyPoints >= nextRes->cost && g_sub.bioHadal >= nextRes->reqHadal);
            DrawCustomButton(hdc, ID_BTN_RES_REGEN, c2x + 8, r2y + gridH - 24, gridW - 16, 18, buf, 0, canAfford ? th->accentEmerald : th->textDim, th);
        } else {
            DrawCustomButton(hdc, ID_BTN_RES_REGEN, c2x + 8, r2y + gridH - 24, gridW - 16, 18, "MAX TIER [BREAKTHROUGH]", 1, th->accentEmerald, th);
        }
    }
}


void DrawOutpostTradeView(HDC hdc, int x, int y, int w, int h, const SubmarineTheme* th) {
    RECT rcBg = { x, y, x + w, y + h };
    HBRUSH hBr = CreateSolidBrush(th->bgDeep);
    FillRect(hdc, &rcBg, hBr);
    DeleteObject(hBr);

    char hdrBuf[128];
    if (g_sub.isDocked) {
        snprintf(hdrBuf, sizeof(hdrBuf), "⚓ DOCKED AT: %s  |  CREDITS: %d PTS",
                 g_outposts[g_sub.dockedStationIdx].name, g_sub.surveyPoints);
    } else {
        snprintf(hdrBuf, sizeof(hdrBuf), "OUTPOST COMMISSARY: CRUISING AT SEA  |  RESEARCH CREDITS: %d PTS",
                 g_sub.surveyPoints);
    }

    SelectObject(hdc, g_hFontBold);
    SetTextColor(hdc, g_sub.isDocked ? th->accentEmerald : th->accentSonar);
    TextOutA(hdc, x + 8, y + 6, hdrBuf, (int)strlen(hdrBuf));

    // Emergency consumable quick buttons in header
    char batBuf[32];
    snprintf(batBuf, sizeof(batBuf), "⚡ USE BATTERY (x%d)", g_sub.batteryPacks);
    DrawCustomButton(hdc, ID_BTN_USE_BATTERY, x + w - 270, y + 4, 130, 20, batBuf, 0, g_sub.batteryPacks > 0 ? th->accentSonar : th->textDim, th);

    char repBuf[32];
    snprintf(repBuf, sizeof(repBuf), "🛠️ USE PATCH (x%d)", g_sub.repairKits);
    DrawCustomButton(hdc, ID_BTN_USE_REPAIR_KIT, x + w - 134, y + 4, 126, 20, repBuf, 0, g_sub.repairKits > 0 ? th->accentEmerald : th->textDim, th);

    // Ordnance Magazine status row
    int ordY = y + 28;
    int ordW = (w - 20) / 5;
    const char* ordNames[5] = { "Acoustic Torp", "EMP Torpedo", "Plasma Torp", "Battery Packs", "Nanopatch Kits" };
    int ordCounts[5] = { g_sub.torpedoes, g_sub.empTorpedoes, g_sub.plasmaTorpedoes, g_sub.batteryPacks, g_sub.repairKits };
    int ordMax[5] = { 12, 6, 4, 8, 6 };

    for (int i = 0; i < 5; i++) {
        int ox = x + 6 + i * (ordW + 2);
        RECT rcOrd = { ox, ordY, ox + ordW, ordY + 22 };
        HBRUSH hBrP = CreateSolidBrush(th->bgPanel);
        HBRUSH hBrB = CreateSolidBrush(th->borderPanel);
        FillRect(hdc, &rcOrd, hBrP);
        FrameRect(hdc, &rcOrd, hBrB);
        DeleteObject(hBrP);
        DeleteObject(hBrB);

        SelectObject(hdc, g_hFontSmall);
        SetTextColor(hdc, th->textDim);
        TextOutA(hdc, ox + 4, ordY + 4, ordNames[i], (int)strlen(ordNames[i]));

        char cBuf[32];
        snprintf(cBuf, sizeof(cBuf), "%d/%d", ordCounts[i], ordMax[i]);
        SetTextColor(hdc, ordCounts[i] > 0 ? th->accentSonar : th->textDim);
        SIZE sz;
        GetTextExtentPoint32A(hdc, cBuf, (int)strlen(cBuf), &sz);
        TextOutA(hdc, ox + ordW - sz.cx - 4, ordY + 4, cBuf, (int)strlen(cBuf));
    }

    // Top section: 4 Outpost Cards
    int cardsY = ordY + 26;
    int cardH = 92;
    int cardW = (w - 20) / 4;

    for (int i = 0; i < OUTPOST_COUNT; i++) {
        const OutpostInfo* out = &g_outposts[i];
        int cx = x + 6 + i * (cardW + 2);

        float dx = out->x - g_sub.posX;
        float dy = out->y - g_sub.posY;
        float distM = sqrtf(dx * dx + dy * dy) * 1000.0f;
        float depthDiff = fabsf(g_sub.depth - out->depth);
        int inRange = (distM <= 300.0f && depthDiff <= (i == 0 ? 25.0f : (i == 3 ? 70.0f : 50.0f)));
        int isDockedHere = (g_sub.isDocked && g_sub.dockedStationIdx == i);
        int isSelected = (g_sub.selectedOutpostIdx == i);

        RECT rcCard = { cx, cardsY, cx + cardW, cardsY + cardH };
        HBRUSH hBrP = CreateSolidBrush(isDockedHere ? RGB(6, 30, 20) : th->bgPanel);
        HBRUSH hBrB = CreateSolidBrush(isDockedHere ? th->accentEmerald : (inRange ? th->accentSonar : th->borderPanel));
        FillRect(hdc, &rcCard, hBrP);
        FrameRect(hdc, &rcCard, hBrB);
        DeleteObject(hBrP);
        DeleteObject(hBrB);

        SelectObject(hdc, g_hFontBold);
        SetTextColor(hdc, isDockedHere ? th->accentEmerald : th->textBright);
        TextOutA(hdc, cx + 4, cardsY + 4, out->name, (int)strlen(out->name));

        SelectObject(hdc, g_hFontSmall);
        SetTextColor(hdc, th->textDim);
        char sBuf[64];
        snprintf(sBuf, sizeof(sBuf), "%.0fm | %s", out->depth, g_sectors[out->sectorIdx].name);
        TextOutA(hdc, cx + 4, cardsY + 18, sBuf, (int)strlen(sBuf));

        snprintf(sBuf, sizeof(sBuf), "Dist: %.0fm (ΔZ: %.0fm)", distM, depthDiff);
        SetTextColor(hdc, inRange ? th->accentSonar : th->textDim);
        TextOutA(hdc, cx + 4, cardsY + 32, sBuf, (int)strlen(sBuf));

        SetTextColor(hdc, isDockedHere ? th->accentEmerald : (inRange ? th->accentEmerald : th->accentAmber));
        const char* statusStr = isDockedHere ? "⚓ DOCKED" : (inRange ? "🟢 IN DOCKING RANGE" : "OUT OF RANGE");
        TextOutA(hdc, cx + 4, cardsY + 46, statusStr, (int)strlen(statusStr));

        // Action Buttons: Lock Nav & Dock
        int btnW = (cardW - 12) / 2;
        DrawCustomButton(hdc, ID_BTN_SELECT_OUTPOST_0 + i, cx + 4, cardsY + cardH - 24, btnW, 20, isSelected ? "TARGETED" : "NAV LOCK", isSelected, th->accentSonar, th);
        if (isDockedHere) {
            DrawCustomButton(hdc, ID_BTN_UNDOCK_OUTPOST, cx + 6 + btnW, cardsY + cardH - 24, btnW, 20, "UNDOCK", 1, th->accentAmber, th);
        } else {
            DrawCustomButton(hdc, ID_BTN_DOCK_OUTPOST, cx + 6 + btnW, cardsY + cardH - 24, btnW, 20, "DOCK", 0, inRange ? th->accentEmerald : th->textDim, th);
        }
    }

    // Bottom section: Station Utilities & Ordnance Commissary
    int btmY = cardsY + cardH + 6;
    int btmH = h - (btmY - y) - 6;
    int halfW = (w - 18) / 2;

    const OutpostInfo* actStation = g_sub.isDocked ? &g_outposts[g_sub.dockedStationIdx] : &g_outposts[g_sub.selectedOutpostIdx];

    // Left Box: Station Drydock & Utilities
    RECT rcUtil = { x + 6, btmY, x + 6 + halfW, btmY + btmH };
    HBRUSH hBrP = CreateSolidBrush(th->bgPanel);
    HBRUSH hBrB = CreateSolidBrush(th->borderPanel);
    FillRect(hdc, &rcUtil, hBrP);
    FrameRect(hdc, &rcUtil, hBrB);

    SelectObject(hdc, g_hFontBold);
    SetTextColor(hdc, th->textBright);
    TextOutA(hdc, x + 12, btmY + 6, "⚓ DRYDOCK & STATION UTILITIES", 29);

    SelectObject(hdc, g_hFontSmall);
    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, x + 12, btmY + 22, actStation->desc, (int)strlen(actStation->desc));

    int uBtnY = btmY + 40;
    int uBtnW = halfW - 16;

    DrawCustomButton(hdc, ID_BTN_TRADE_RECHARGE, x + 12, uBtnY, uBtnW, 20, "⚡ FULL BATTERY & O2 RECHARGE (FREE/25 PTS)", 0, g_sub.isDocked ? th->accentEmerald : th->textDim, th);
    uBtnY += 24;

    DrawCustomButton(hdc, ID_BTN_TRADE_AIR, x + 12, uBtnY, uBtnW, 20, "💨 RECHARGE COMPRESSED AIR TANK (FREE/20 PTS)", 0, g_sub.isDocked ? th->accentSonar : th->textDim, th);
    uBtnY += 24;

    DrawCustomButton(hdc, ID_BTN_TRADE_REPAIR, x + 12, uBtnY, uBtnW, 20, "🛠️ OVERHAUL HULL & PATCH LEAKS (40-100 PTS)", 0, g_sub.isDocked ? th->accentAmber : th->textDim, th);
    uBtnY += 26;

    char sellBuf[64];
    snprintf(sellBuf, sizeof(sellBuf), "💎 SELL CARGO MINERALS (+%d PTS)", g_sub.cargoTotalValue);
    DrawCustomButton(hdc, ID_BTN_TRADE_SELL_ALL, x + 12, uBtnY, uBtnW, 20, sellBuf, 0, g_sub.cargoTotalValue > 0 ? th->accentEmerald : th->textDim, th);

    // Right Box: Ordnance & Commissary Trade
    int rx = x + 12 + halfW;
    RECT rcComm = { rx, btmY, rx + halfW, btmY + btmH };
    FillRect(hdc, &rcComm, hBrP);
    FrameRect(hdc, &rcComm, hBrB);
    DeleteObject(hBrP);
    DeleteObject(hBrB);

    SelectObject(hdc, g_hFontBold);
    SetTextColor(hdc, th->textBright);
    TextOutA(hdc, rx + 8, btmY + 6, "🚀 ORDNANCE & AMMUNITION COMMISSARY", 35);

    SelectObject(hdc, g_hFontSmall);
    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, rx + 8, btmY + 22, "Purchase torpedoes & batteries with credits or barter minerals:", 62);

    int tBtnY = btmY + 38;
    int tBtnW = (halfW - 20) / 2;

    // Torpedo 1: Acoustic
    DrawCustomButton(hdc, ID_BTN_BUY_TORPEDO, rx + 8, tBtnY, tBtnW, 18, "🚀 ACOUSTIC (60 PTS)", 0, g_sub.isDocked && g_sub.torpedoes < 12 && g_sub.surveyPoints >= 60 ? th->accentSonar : th->textDim, th);
    DrawCustomButton(hdc, ID_BTN_BARTER_TORPEDO, rx + 12 + tBtnW, tBtnY, tBtnW, 18, "BARTER: 1x MANGANESE", 0, g_sub.isDocked && g_sub.torpedoes < 12 && g_sub.cargoManganese >= 1 ? th->accentAmber : th->textDim, th);
    tBtnY += 21;

    // Torpedo 2: EMP
    DrawCustomButton(hdc, ID_BTN_BUY_EMP_TORPEDO, rx + 8, tBtnY, tBtnW, 18, "⚡ EMP SHOCK (90 PTS)", 0, g_sub.isDocked && g_sub.empTorpedoes < 6 && g_sub.surveyPoints >= 90 ? th->accentSonar : th->textDim, th);
    DrawCustomButton(hdc, ID_BTN_BARTER_EMP, rx + 12 + tBtnW, tBtnY, tBtnW, 18, "BARTER: 1x TITANIUM", 0, g_sub.isDocked && g_sub.empTorpedoes < 6 && g_sub.cargoTitaniumScrap >= 1 ? th->accentAmber : th->textDim, th);
    tBtnY += 21;

    // Torpedo 3: Plasma
    DrawCustomButton(hdc, ID_BTN_BUY_PLASMA_TORPEDO, rx + 8, tBtnY, tBtnW, 18, "🔥 PLASMA TORP (150 PTS)", 0, g_sub.isDocked && g_sub.plasmaTorpedoes < 4 && g_sub.surveyPoints >= 150 ? th->accentSonar : th->textDim, th);
    DrawCustomButton(hdc, ID_BTN_BARTER_PLASMA, rx + 12 + tBtnW, tBtnY, tBtnW, 18, "BARTER: 1x SMOKER/HADAL", 0, g_sub.isDocked && g_sub.plasmaTorpedoes < 4 && (g_sub.cargoSmokerCrystals >= 1 || g_sub.cargoHadalPrisms >= 1) ? th->accentAmber : th->textDim, th);
    tBtnY += 21;

    // Batteries
    DrawCustomButton(hdc, ID_BTN_BUY_BATTERY_PACK, rx + 8, tBtnY, tBtnW, 18, "🔋 BATTERY CELL (40 PTS)", 0, g_sub.isDocked && g_sub.batteryPacks < 8 && g_sub.surveyPoints >= 40 ? th->accentSonar : th->textDim, th);
    DrawCustomButton(hdc, ID_BTN_BARTER_BATTERY, rx + 12 + tBtnW, tBtnY, tBtnW, 18, "BARTER: 1x MANGANESE", 0, g_sub.isDocked && g_sub.batteryPacks < 8 && g_sub.cargoManganese >= 1 ? th->accentAmber : th->textDim, th);
    tBtnY += 21;

    // Nanopatch Kits
    DrawCustomButton(hdc, ID_BTN_BUY_REPAIR_KIT, rx + 8, tBtnY, tBtnW, 18, "🧰 NANOPATCH KIT (50 PTS)", 0, g_sub.isDocked && g_sub.repairKits < 6 && g_sub.surveyPoints >= 50 ? th->accentEmerald : th->textDim, th);
    DrawCustomButton(hdc, ID_BTN_BARTER_REPAIR, rx + 12 + tBtnW, tBtnY, tBtnW, 18, "BARTER: 1x TITANIUM", 0, g_sub.isDocked && g_sub.repairKits < 6 && g_sub.cargoTitaniumScrap >= 1 ? th->accentAmber : th->textDim, th);
}

// --- DRAW COMBAT & DEFENSE VIEW (PHASE 12) ---
void DrawCombatView(HDC hdc, int x, int y, int w, int h, const SubmarineTheme* th) {
    RECT rcBg = { x, y, x + w, y + h };
    HBRUSH hBr = CreateSolidBrush(th->bgDeep);
    FillRect(hdc, &rcBg, hBr);
    DeleteObject(hBr);

    // Threat assessment
    int activeThreats = 0;
    int attackingCount = 0;
    for (int i = 0; i < THREAT_COUNT; i++) {
        if (!g_threats[i].defeated && g_threats[i].sectorIdx == g_sub.currentSectorIdx) {
            activeThreats++;
            if (g_threats[i].state == 2 || g_threats[i].state == 1) attackingCount++;
        }
    }

    char hdrBuf[128];
    if (attackingCount > 0) {
        snprintf(hdrBuf, sizeof(hdrBuf), "🚨 RED ALERT: %d HOSTILE TARGETS ENGAGING DSV!  |  CREDITS: %d PTS", attackingCount, g_sub.surveyPoints);
    } else if (activeThreats > 0) {
        snprintf(hdrBuf, sizeof(hdrBuf), "⚠️ THREAT MATRIX: %d CONTACTS IN SECTOR  |  CREDITS: %d PTS", activeThreats, g_sub.surveyPoints);
    } else {
        snprintf(hdrBuf, sizeof(hdrBuf), "THREAT STATUS: SECTOR SECURE (ALL THREATS CLEARED)  |  CREDITS: %d PTS", g_sub.surveyPoints);
    }

    SelectObject(hdc, g_hFontBold);
    SetTextColor(hdc, attackingCount > 0 ? th->accentRed : (activeThreats > 0 ? th->accentAmber : th->accentEmerald));
    TextOutA(hdc, x + 8, y + 6, hdrBuf, (int)strlen(hdrBuf));

    // Top Countermeasures Toolbar in Header
    char decBuf[32];
    snprintf(decBuf, sizeof(decBuf), "🚨 DECOY (x%d)", g_sub.decoys);
    DrawCustomButton(hdc, ID_BTN_COMBAT_DECOY, x + w - 380, y + 4, 115, 20, decBuf, 0, g_sub.decoys > 0 ? th->accentEmerald : th->textDim, th);

    char shkBuf[32];
    if (g_sub.shockwaveCooldown > 0.0f) snprintf(shkBuf, sizeof(shkBuf), "⚡ SHOCK (%.1fs)", g_sub.shockwaveCooldown);
    else snprintf(shkBuf, sizeof(shkBuf), "⚡ EMP SHOCK (15%%)");
    DrawCustomButton(hdc, ID_BTN_COMBAT_SHOCKWAVE, x + w - 260, y + 4, 130, 20, shkBuf, g_sub.shockwaveCooldown > 0.0f, g_sub.battery >= 15.0f && g_sub.shockwaveCooldown <= 0.0f ? th->accentSonar : th->textDim, th);

    DrawCustomButton(hdc, ID_BTN_COMBAT_SILENT, x + w - 124, y + 4, 116, 20, g_sub.silentRunning ? "🤫 SILENT: ON" : "🤫 SILENT: OFF", g_sub.silentRunning, th->accentSonar, th);

    int contentY = y + 26;

    // --- TOP HALF: 4 TORPEDO TUBES ---
    int tubesH = 78;
    int margin = 6;
    int tubeW = (w - margin * 5) / 4;

    for (int i = 0; i < 4; i++) {
        TorpedoTube* tube = &g_sub.tubes[i];
        int tx = x + margin + i * (tubeW + margin);
        int ty = contentY;

        const OrdnanceDef* ord = &g_ordDefs[tube->type];
        int ammo = (tube->type == 0 ? g_sub.torpedoes : (tube->type == 1 ? g_sub.empTorpedoes : g_sub.plasmaTorpedoes));
        int isReady = (tube->status == 0);

        RECT rcTube = { tx, ty, tx + tubeW, ty + tubesH };
        HBRUSH hBrP = CreateSolidBrush(th->bgPanel);
        COLORREF brClr = isReady ? (ammo > 0 ? th->accentEmerald : th->borderPanel) : th->accentAmber;
        HBRUSH hBrB = CreateSolidBrush(brClr);
        FillRect(hdc, &rcTube, hBrP);
        FrameRect(hdc, &rcTube, hBrB);
        DeleteObject(hBrP);
        DeleteObject(hBrB);

        SelectObject(hdc, g_hFontBold);
        SetTextColor(hdc, th->textBright);
        char tHdr[32];
        snprintf(tHdr, sizeof(tHdr), "TUBE #%d", tube->id);
        TextOutA(hdc, tx + 6, ty + 4, tHdr, (int)strlen(tHdr));

        SetTextColor(hdc, isReady ? (ammo > 0 ? th->accentEmerald : th->textDim) : th->accentAmber);
        char stBuf[32];
        if (isReady) snprintf(stBuf, sizeof(stBuf), ammo > 0 ? "READY" : "EMPTY");
        else snprintf(stBuf, sizeof(stBuf), "RELOAD %.1fs", tube->reloadTime);
        SIZE stSz;
        GetTextExtentPoint32A(hdc, stBuf, (int)strlen(stBuf), &stSz);
        TextOutA(hdc, tx + tubeW - stSz.cx - 6, ty + 4, stBuf, (int)strlen(stBuf));

        SelectObject(hdc, g_hFontSmall);
        SetTextColor(hdc, tube->type == 0 ? th->accentSonar : (tube->type == 1 ? th->accentEmerald : th->accentRed));
        char ordBuf[48];
        snprintf(ordBuf, sizeof(ordBuf), "%s (x%d)", ord->name, ammo);
        TextOutA(hdc, tx + 6, ty + 18, ordBuf, (int)strlen(ordBuf));

        float reloadPct = isReady ? 100.0f : max(0.0f, (1.0f - (tube->reloadTime / ord->reloadTime)) * 100.0f);
        DrawGaugeBar(hdc, tx + 6, ty + 32, tubeW - 12, 6, reloadPct, tube->type == 0 ? th->accentSonar : (tube->type == 1 ? th->accentEmerald : th->accentRed), th);

        char statBuf[48];
        snprintf(statBuf, sizeof(statBuf), "DMG: %d HP | BLAST: %.0fm", ord->damage, ord->blastRadius);
        SetTextColor(hdc, th->textDim);
        TextOutA(hdc, tx + 6, ty + 40, statBuf, (int)strlen(statBuf));

        int btnW = (tubeW - 16) / 2;
        char fireBuf[32];
        snprintf(fireBuf, sizeof(fireBuf), "🚀 FIRE [%d]", tube->id);
        DrawCustomButton(hdc, ID_BTN_FIRE_TUBE_0 + i, tx + 6, ty + 54, btnW, 18, fireBuf, 0, isReady && ammo > 0 ? th->accentSonar : th->textDim, th);
        DrawCustomButton(hdc, ID_BTN_CYCLE_TUBE_0 + i, tx + 10 + btnW, ty + 54, btnW, 18, "CYCLE", 0, th->textPrimary, th);
    }

    // --- BOTTOM HALF: 6 HOSTILE THREAT CARDS (3 cols x 2 rows) ---
    int thrY = contentY + tubesH + 6;
    int thrH = h - (thrY - y) - 6;
    int cols = 3;
    int rows = 2;
    int cardW = (w - margin * (cols + 1)) / cols;
    int cardH = (thrH - margin * (rows + 1)) / rows;

    for (int i = 0; i < THREAT_COUNT; i++) {
        HostileThreat* thr = &g_threats[i];
        int c = i % cols;
        int r = i / cols;
        int cx = x + margin + c * (cardW + margin);
        int cy = thrY + margin + r * (cardH + margin);

        float dx = thr->x - g_sub.posX;
        float dy = thr->y - g_sub.posY;
        float distM = sqrtf(dx * dx + dy * dy) * 1000.0f;
        float depthDiff = fabsf(g_sub.depth - thr->depth);
        int isLocked = (g_sub.selectedThreatIdx == i);
        int isSameSector = (thr->sectorIdx == g_sub.currentSectorIdx);

        RECT rcCard = { cx, cy, cx + cardW, cy + cardH };
        HBRUSH hBrP = CreateSolidBrush(thr->defeated ? RGB(10, 15, 20) : (thr->state == 2 ? RGB(40, 10, 10) : th->bgPanel));
        COLORREF brdr = thr->defeated ? th->borderPanel : (isLocked ? th->accentSonar : (thr->state == 2 ? th->accentRed : (thr->type == 1 ? RGB(244, 63, 94) : th->accentAmber)));
        HBRUSH hBrB = CreateSolidBrush(brdr);
        FillRect(hdc, &rcCard, hBrP);
        FrameRect(hdc, &rcCard, hBrB);
        DeleteObject(hBrP);
        DeleteObject(hBrB);

        SelectObject(hdc, g_hFontBold);
        SetTextColor(hdc, thr->defeated ? th->textDim : (thr->type == 1 ? th->accentRed : th->accentAmber));
        char nameBuf[64];
        snprintf(nameBuf, sizeof(nameBuf), "%s %s", thr->type == 1 ? "🐉" : "🤖", thr->name);
        TextOutA(hdc, cx + 6, cy + 4, nameBuf, (int)strlen(nameBuf));

        const char* stateLabels[5] = { "PATROL", "STALKING", "ATTACKING", "DISTRACTED", "STUNNED" };
        const char* stLabel = thr->defeated ? "NEUTRALIZED" : (thr->stunTimer > 0.0f ? "STUNNED" : stateLabels[thr->state]);
        COLORREF stClr = thr->defeated ? th->textDim : (thr->stunTimer > 0.0f ? th->accentEmerald : (thr->state == 2 ? th->accentRed : (thr->state == 3 ? th->accentAmber : th->accentSonar)));
        SelectObject(hdc, g_hFontSmall);
        SetTextColor(hdc, stClr);
        SIZE sSz;
        GetTextExtentPoint32A(hdc, stLabel, (int)strlen(stLabel), &sSz);
        TextOutA(hdc, cx + cardW - sSz.cx - 6, cy + 4, stLabel, (int)strlen(stLabel));

        // Description
        SetTextColor(hdc, th->textDim);
        TextOutA(hdc, cx + 6, cy + 18, thr->desc, (int)strlen(thr->desc));

        // HP Bar
        float hpPct = (float)thr->hp / (float)thr->maxHp * 100.0f;
        DrawGaugeBar(hdc, cx + 6, cy + 32, cardW - 12, 6, hpPct, hpPct < 30.0f ? th->accentRed : (hpPct < 70.0f ? th->accentAmber : th->accentEmerald), th);

        char hpBuf[48];
        snprintf(hpBuf, sizeof(hpBuf), "HP: %d/%d (%.0f%%)", thr->hp, thr->maxHp, hpPct);
        SetTextColor(hdc, th->textBright);
        TextOutA(hdc, cx + 6, cy + 40, hpBuf, (int)strlen(hpBuf));

        // Telemetry & Sector
        char telBuf[64];
        snprintf(telBuf, sizeof(telBuf), "Dist: %.0fm (ΔZ: %.0fm) | %s", distM, depthDiff, isSameSector ? "IN SECTOR" : g_sectors[thr->sectorIdx].name);
        SetTextColor(hdc, distM < 250.0f && isSameSector ? th->accentRed : th->textDim);
        TextOutA(hdc, cx + 6, cy + 54, telBuf, (int)strlen(telBuf));

        char atkBuf[48];
        snprintf(atkBuf, sizeof(atkBuf), "ATK: %d DMG | BOUNTY: +%d PTS", thr->atkPower, thr->bountyCredits);
        SetTextColor(hdc, th->accentEmerald);
        TextOutA(hdc, cx + 6, cy + 68, atkBuf, (int)strlen(atkBuf));

        // Action Buttons
        int aBtnW = (cardW - 16) / 2;
        DrawCustomButton(hdc, ID_BTN_LOCK_THREAT_0 + i, cx + 6, cy + cardH - 22, aBtnW, 18, isLocked ? "TARGETED" : "LOCK", isLocked, th->accentSonar, th);
        DrawCustomButton(hdc, ID_BTN_FIRE_THREAT_0 + i, cx + 10 + aBtnW, cy + cardH - 22, aBtnW, 18, "FIRE BEST", 0, !thr->defeated ? th->accentRed : th->textDim, th);
    }
}

// --- PHASE 13: ACOUSTIC HYDROPHONE & SOUND MATRIX VIEW ---
typedef struct {
    const char* name;
    const char* badge;
    COLORREF badgeClr;
    const char* freq;
    const char* desc;
    int btnId;
} AudioMatrixCard;

static const AudioMatrixCard g_audioCards[12] = {
    { "Active Sonar Ping & Echo", "ACOUSTIC PING", RGB(0, 240, 255), "1920 Hz -> 540 Hz (FM)", "High-frequency omnidirectional acoustic chirp with seabed reverberation.", ID_BTN_TEST_SONAR_PING },
    { "Abyssal Whale & Leviathan Song", "CETACEAN BIO", RGB(52, 211, 153), "45 Hz - 380 Hz (Harmonics)", "Resonant multi-harmonic vocalization of deep baleen whales and titans.", ID_BTN_TEST_WHALE_SONG },
    { "High-Pressure Ballast Blow", "PNEUMATIC HISS", RGB(245, 158, 11), "60 Hz - 1800 Hz (Noise)", "Supercritical 300-BAR compressed air blowing Kingston flood valves.", ID_BTN_TEST_BALLAST_BLOW },
    { "Hull Hydrostatic Pressure Groan", "STRUCTURAL STRAIN", RGB(239, 68, 68), "40 Hz - 160 Hz (Sub-Bass)", "Metallic creak, shear stress resonance, and titanium lattice flex.", ID_BTN_TEST_HULL_GROAN },
    { "Acoustic Torpedo Ejection", "ORDNANCE LAUNCH", RGB(0, 240, 255), "180 Hz - 720 Hz (Cavitation)", "Pneumatic launch tube piston impulse and supercavitating propeller wake.", ID_BTN_TEST_TORPEDO_LAUNCH },
    { "Thermal Plasma Torpedo Flare", "THERMAL IGNITION", RGB(239, 68, 68), "180 Hz - 600 Hz (Thermal)", "High-energy superheated plasma ignition with bubble displacement.", ID_BTN_TEST_PLASMA_LAUNCH },
    { "Hull EMP Shockwave Burst", "ELECTROMAGNETIC", RGB(0, 240, 255), "60 Hz - 1800 Hz (Shock)", "High-voltage electromagnetic pulse discharge radiating through water.", ID_BTN_TEST_SHOCKWAVE },
    { "Undersea Depth Detonation", "EXPLOSIVE BLAST", RGB(239, 68, 68), "35 Hz - 120 Hz (Sub-Bass)", "Muffled underwater high-yield warhead blast and cavitation collapse.", ID_BTN_TEST_EXPLOSION },
    { "Acoustic Decoy Cavitation", "COUNTERMEASURE", RGB(52, 211, 153), "400 Hz - 1400 Hz (Chitter)", "Micro-cavitation bubble generator simulating fake Doppler echoes.", ID_BTN_TEST_DECOY },
    { "Bulkhead Arc Weld Torch", "PLASMA WELDING", RGB(245, 158, 11), "400 Hz - 1200 Hz (Crackle)", "High-temperature plasma welding torch sealing bulkhead fractures.", ID_BTN_TEST_WELDING },
    { "Flood & Collision Klaxon", "SIREN ALERT", RGB(239, 68, 68), "440 Hz / 880 Hz (Dual Tone)", "Command sphere master caution klaxon warning of hull breach/grounding.", ID_BTN_TEST_ALARM },
    { "Bio-Scan Sonar & Centrifuge", "LABORATORY BIO", RGB(52, 211, 153), "320 Hz - 2400 Hz (FM Chirp)", "High-definition bio-acoustic imaging chirp and research centrifuge.", ID_BTN_TEST_CENTRIFUGE }
};

void DrawAudioView(HDC hdc, int x, int y, int w, int h, const SubmarineTheme* th) {
    RECT rcBg = { x, y, x + w, y + h };
    HBRUSH hBr = CreateSolidBrush(th->bgDeep);
    FillRect(hdc, &rcBg, hBr);
    DeleteObject(hBr);

    SelectObject(hdc, g_hFontBold);
    SetTextColor(hdc, th->accentSonar);
    TextOutA(hdc, x + 8, y + 6, "ACOUSTIC HYDROPHONE & SOUND MATRIX // PASSIVE LISTENING ARRAY", 61);

    // Oscilloscope Preview Box
    int scopeH = 34;
    int scopeY = y + 26;
    RECT rcScope = { x + 6, scopeY, x + w - 6, scopeY + scopeH };
    HBRUSH hBrSc = CreateSolidBrush(RGB(2, 11, 18));
    HBRUSH hBrScB = CreateSolidBrush(th->borderPanel);
    FillRect(hdc, &rcScope, hBrSc);
    FrameRect(hdc, &rcScope, hBrScB);
    DeleteObject(hBrSc);
    DeleteObject(hBrScB);

    HPEN hPenGrid = CreatePen(PS_DOT, 1, RGB(19, 60, 90));
    HPEN hPenOld = (HPEN)SelectObject(hdc, hPenGrid);
    MoveToEx(hdc, x + 6, scopeY + scopeH / 2, NULL);
    LineTo(hdc, x + w - 6, scopeY + scopeH / 2);
    DeleteObject(hPenGrid);

    // Oscilloscope live waveform
    HPEN hPenWave = CreatePen(PS_SOLID, 2, th->accentSonar);
    SelectObject(hdc, hPenWave);
    int midY = scopeY + scopeH / 2;
    static float s_scopePhase = 0.0f;
    s_scopePhase += 0.15f;
    for (int px = x + 8; px < x + w - 8; px++) {
        float normX = (float)(px - (x + 8)) / (float)(w - 16);
        int wy = midY + (int)(sinf(normX * 22.0f + s_scopePhase) * 10.0f + sinf(normX * 45.0f - s_scopePhase * 1.5f) * 4.0f);
        if (px == x + 8) MoveToEx(hdc, px, wy, NULL);
        else LineTo(hdc, px, wy);
    }
    SelectObject(hdc, hPenOld);
    DeleteObject(hPenWave);

    SelectObject(hdc, g_hFontSmall);
    SetTextColor(hdc, th->accentEmerald);
    TextOutA(hdc, x + 12, scopeY + 4, "HYDROPHONE OSCILLOSCOPE [REALTIME]", 34);

    // Grid of 12 Sound Cards (3 cols x 4 rows)
    int cardsY = scopeY + scopeH + 6;
    int cardsH = h - (cardsY - y) - 4;
    int cols = 3;
    int rows = 4;
    int margin = 6;
    int cardW = (w - margin * (cols + 1)) / cols;
    int cardH = (cardsH - margin * (rows + 1)) / rows;

    for (int i = 0; i < 12; i++) {
        const AudioMatrixCard* c = &g_audioCards[i];
        int col = i % cols;
        int row = i / cols;
        int cx = x + margin + col * (cardW + margin);
        int cy = cardsY + margin + row * (cardH + margin);

        RECT rcCard = { cx, cy, cx + cardW, cy + cardH };
        HBRUSH hBrP = CreateSolidBrush(th->bgPanel);
        HBRUSH hBrB = CreateSolidBrush(th->borderPanel);
        FillRect(hdc, &rcCard, hBrP);
        FrameRect(hdc, &rcCard, hBrB);
        DeleteObject(hBrP);
        DeleteObject(hBrB);

        SelectObject(hdc, g_hFontBold);
        SetTextColor(hdc, th->textBright);
        TextOutA(hdc, cx + 6, cy + 4, c->name, (int)strlen(c->name));

        SelectObject(hdc, g_hFontSmall);
        SetTextColor(hdc, c->badgeClr);
        SIZE sz;
        GetTextExtentPoint32A(hdc, c->badge, (int)strlen(c->badge), &sz);
        TextOutA(hdc, cx + cardW - sz.cx - 6, cy + 4, c->badge, (int)strlen(c->badge));

        SetTextColor(hdc, th->textDim);
        TextOutA(hdc, cx + 6, cy + 18, c->desc, (int)strlen(c->desc));

        SetTextColor(hdc, th->accentSonar);
        char fBuf[48];
        snprintf(fBuf, sizeof(fBuf), "FREQ: %s", c->freq);
        TextOutA(hdc, cx + 6, cy + 30, fBuf, (int)strlen(fBuf));

        DrawCustomButton(hdc, c->btnId, cx + 6, cy + cardH - 20, cardW - 12, 17, "▶ EMIT SIGNATURE", 0, th->accentSonar, th);
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
    gy += 22;

    // Phase 8: Cargo Hold Payload Gauge
    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, gx, gy, "CARGO HOLD PAYLOAD", 18);
    RecalculateCargo();
    snprintf(buf, sizeof(buf), "%.0f / %.0f KG", g_sub.cargoTotalWeight, g_sub.cargoMaxWeight);
    SetTextColor(hdc, th->textBright);
    GetTextExtentPoint32A(hdc, buf, (int)strlen(buf), &sz);
    TextOutA(hdc, gx + gw - sz.cx, gy, buf, (int)strlen(buf));
    gy += 15;
    float cargoPct = min(100.0f, (g_sub.cargoTotalWeight / g_sub.cargoMaxWeight) * 100.0f);
    DrawGaugeBar(hdc, gx, gy, gw, 10, cargoPct, th->accentAmber, th);
    gy += 12;
    snprintf(buf, sizeof(buf), "EST: %d PTS   CLAW: %s", g_sub.cargoTotalValue, g_sub.clawDeployed ? "DEPLOYED" : "STOWED");
    SetTextColor(hdc, g_sub.clawDeployed ? th->accentAmber : th->textDim);
    TextOutA(hdc, gx, gy, buf, (int)strlen(buf));
    gy += 22;

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

    // View switch buttons inside center panel header (10 views)
    int btnViewW = (centerW - 132) / 10;
    DrawCustomButton(hdc, ID_BTN_VIEW_SONAR, centerX + 8, panelY + 28, btnViewW - 2, 20, "SONAR", g_sub.viewMode == 0, th->accentSonar, th);
    DrawCustomButton(hdc, ID_BTN_VIEW_NAVMAP, centerX + 6 + btnViewW, panelY + 28, btnViewW - 2, 20, "TRENCH", g_sub.viewMode == 1, th->accentSonar, th);
    DrawCustomButton(hdc, ID_BTN_VIEW_CODEX, centerX + 4 + btnViewW * 2, panelY + 28, btnViewW - 2, 20, "CODEX", g_sub.viewMode == 2, th->accentSonar, th);
    DrawCustomButton(hdc, ID_BTN_VIEW_CARGO, centerX + 2 + btnViewW * 3, panelY + 28, btnViewW - 2, 20, "CARGO", g_sub.viewMode == 3, th->accentAmber, th);
    DrawCustomButton(hdc, ID_BTN_VIEW_LAB, centerX + btnViewW * 4, panelY + 28, btnViewW - 2, 20, "LAB", g_sub.viewMode == 6, th->accentEmerald, th);
    DrawCustomButton(hdc, ID_BTN_VIEW_ENG, centerX - 2 + btnViewW * 5, panelY + 28, btnViewW - 2, 20, "ENG", g_sub.viewMode == 4, th->accentSonar, th);
    DrawCustomButton(hdc, ID_BTN_VIEW_DAMAGE, centerX - 4 + btnViewW * 6, panelY + 28, btnViewW - 2, 20, "DAMAGE", g_sub.viewMode == 5, th->accentRed, th);
    DrawCustomButton(hdc, ID_BTN_VIEW_OUTPOSTS, centerX - 6 + btnViewW * 7, panelY + 28, btnViewW - 2, 20, "TRADE", g_sub.viewMode == 7, th->accentEmerald, th);
    DrawCustomButton(hdc, ID_BTN_VIEW_COMBAT, centerX - 8 + btnViewW * 8, panelY + 28, btnViewW - 2, 20, "COMBAT", g_sub.viewMode == 8, th->accentRed, th);
    DrawCustomButton(hdc, ID_BTN_VIEW_AUDIO, centerX - 10 + btnViewW * 9, panelY + 28, btnViewW - 2, 20, "SOUND", g_sub.viewMode == 9, th->accentSonar, th);
    DrawCustomButton(hdc, ID_BTN_FIELD_DIAG, centerX + centerW - 124, panelY + 28, 116, 20, "+35 PTS DIAG", 0, th->accentAmber, th);

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

        // Unified Sonar Contacts (Fauna + Salvage Nodes)
        UnifiedContact contacts[8];
        int fCount = GetUnifiedContacts(g_sub.currentSectorIdx, contacts);

        SelectObject(hdc, g_hFontSmall);
        for (int i = 0; i < fCount; i++) {
            UnifiedContact* c = &contacts[i];
            float relX = (c->x - g_sub.posX) / 2.0f;
            float relY = (c->y - g_sub.posY) / 2.0f;
            float distNorm = sqrtf(relX * relX + relY * relY);

            if (distNorm > 1.05f) continue;

            int fcx = scx + (int)(relX * sRadius);
            int fcy = scy + (int)(relY * sRadius);

            float angleToC = fmodf(atan2f(relY, relX) + 6.2831853f, 6.2831853f);
            float angleDiff = fabsf(g_sub.sweepAngle - angleToC);
            int isSwept = (angleDiff < 0.25f) || (g_sub.isPinging && fabsf(g_sub.pingRadius - sRadius * distNorm) < 20.0f);
            int isSelected = (i == g_sub.selectedTargetIdx);

            COLORREF cClr = th->accentSonar;
            if (c->isSalvage) cClr = RGB(251, 191, 36);
            else if (c->type == 4) cClr = RGB(244, 63, 94); // Leviathan
            else if (c->type == 1) cClr = RGB(244, 114, 182); // Squid
            else if (c->type == 2) cClr = th->accentEmerald; // Flora
            else if (c->type == 3) cClr = th->accentAmber; // Trench

            HBRUSH hBrContact = CreateSolidBrush(isSwept || isSelected ? cClr : th->radarRing);
            SelectObject(hdc, hBrContact);

            if (c->isSalvage) {
                POINT pts[4] = { { fcx, fcy - 4 }, { fcx + 4, fcy }, { fcx, fcy + 4 }, { fcx - 4, fcy } };
                Polygon(hdc, pts, 4);
            } else {
                int dotRad = (c->type == 4 ? 6 : (c->type == 1 ? 4 : 3));
                Ellipse(hdc, fcx - dotRad, fcy - dotRad, fcx + dotRad, fcy + dotRad);
            }
            DeleteObject(hBrContact);

            if (isSwept || isSelected) {
                SetTextColor(hdc, th->textBright);
                const char* dName = c->discovered ? c->name : (c->isSalvage ? "[SALVAGE SITE]" : (c->type == 4 ? "[LEVIATHAN]" : (c->type == 1 ? "[SQUID]" : (c->type == 2 ? "[FLORA]" : "[FAUNA]"))));
                TextOutA(hdc, fcx + 8, fcy - 6, dName, (int)strlen(dName));
                snprintf(buf, sizeof(buf), "%.0fm", distNorm * 2000.0f);
                SetTextColor(hdc, th->textDim);
                TextOutA(hdc, fcx + 8, fcy + 4, buf, (int)strlen(buf));
            }

            // Targeting Reticle for Locked Target
            if (isSelected) {
                HPEN hPenReticle = CreatePen(PS_SOLID, 1, c->isSalvage ? RGB(251, 191, 36) : th->textPrimary);
                SelectObject(hdc, hPenReticle);
                int bs = 8;
                MoveToEx(hdc, fcx - bs, fcy - bs + 3, NULL); LineTo(hdc, fcx - bs, fcy - bs); LineTo(hdc, fcx - bs + 3, fcy - bs);
                MoveToEx(hdc, fcx + bs - 3, fcy - bs, NULL); LineTo(hdc, fcx + bs, fcy - bs); LineTo(hdc, fcx + bs, fcy - bs + 3);
                MoveToEx(hdc, fcx - bs, fcy + bs - 3, NULL); LineTo(hdc, fcx - bs, fcy + bs); LineTo(hdc, fcx - bs + 3, fcy + bs);
                MoveToEx(hdc, fcx + bs - 3, fcy + bs, NULL); LineTo(hdc, fcx + bs, fcy + bs); LineTo(hdc, fcx + bs, fcy + bs - 3);

                // Dotted course line from center
                HPEN hPenVec = CreatePen(PS_DOT, 1, c->isSalvage ? RGB(251, 191, 36) : th->textPrimary);
                SelectObject(hdc, hPenVec);
                MoveToEx(hdc, scx, scy, NULL);
                LineTo(hdc, fcx, fcy);
                DeleteObject(hPenVec);
                DeleteObject(hPenReticle);
            }
        }

        // Top-Right Target HUD Box in Sonar View
        if (fCount > 0 && g_sub.selectedTargetIdx < fCount) {
            UnifiedContact* tgt = &contacts[g_sub.selectedTargetIdx];
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
            HBRUSH hBrHdrB = CreateSolidBrush(tgt->isSalvage ? RGB(251, 191, 36) : th->borderPanel);
            FillRect(hdc, &rcHud, hBrHud);
            FrameRect(hdc, &rcHud, hBrHdrB);
            DeleteObject(hBrHud);
            DeleteObject(hBrHdrB);

            SelectObject(hdc, g_hFontSmall);
            SetTextColor(hdc, tgt->isSalvage ? RGB(251, 191, 36) : th->accentEmerald);
            TextOutA(hdc, hudX + 6, hudY + 4, tgt->isSalvage ? "💎 SALVAGE TRACKER" : "🎯 TARGET TRACKER", 18);

            SetTextColor(hdc, th->textBright);
            snprintf(buf, sizeof(buf), "%.16s", tgt->discovered ? tgt->name : tgt->id);
            TextOutA(hdc, hudX + 6, hudY + 18, buf, (int)strlen(buf));

            SetTextColor(hdc, th->textDim);
            snprintf(buf, sizeof(buf), "Dist: %.0fm  ΔZ: %.0fm", distM, depthDiff);
            TextOutA(hdc, hudX + 6, hudY + 32, buf, (int)strlen(buf));

            snprintf(buf, sizeof(buf), "%s: %s", tgt->isSalvage ? "Freq" : "Lumens", tgt->isSalvage ? tgt->freq : tgt->lumens);
            TextOutA(hdc, hudX + 6, hudY + 46, buf, (int)strlen(buf));

            if (g_sub.isScanningTarget || g_sub.isDredging) {
                DrawGaugeBar(hdc, hudX + 6, hudY + 62, hudW - 12, 8, g_sub.isDredging ? g_sub.dredgeProgress : g_sub.scanProgress, tgt->isSalvage ? th->accentAmber : th->accentEmerald, th);
            } else {
                DrawCustomButton(hdc, ID_BTN_HUD_NEXT_TARGET, hudX + 6, hudY + 58, (hudW - 16) / 2, 18, "NEXT", 0, th->textPrimary, th);
                const char* actLabel = tgt->isSalvage ? "DREDGE" : "SCAN";
                DrawCustomButton(hdc, ID_BTN_HUD_SCAN_TARGET, hudX + 8 + (hudW - 16) / 2, hudY + 58, (hudW - 16) / 2, 18, actLabel, 0, tgt->isSalvage ? th->accentAmber : th->accentEmerald, th);
            }
        }

        SelectObject(hdc, hPenOld);
        SelectObject(hdc, hBrOld);
        DeleteObject(hPenRing);
    } else if (g_sub.viewMode == 1) {
        DrawNavMapChart(hdc, scx, scy, centerW - 20, sonarContentH - 8, th);
    } else if (g_sub.viewMode == 2) {
        DrawFaunaCodex(hdc, centerX + 6, sonarContentY + 4, centerW - 12, sonarContentH - 8, th);
    } else if (g_sub.viewMode == 3) {
        DrawCargoHoldView(hdc, centerX + 6, sonarContentY + 4, centerW - 12, sonarContentH - 8, th);
    } else if (g_sub.viewMode == 4) {
        DrawEngineeringBay(hdc, centerX + 6, sonarContentY + 4, centerW - 12, sonarContentH - 8, th);
    } else if (g_sub.viewMode == 5) {
        DrawDamageControlView(hdc, centerX + 6, sonarContentY + 4, centerW - 12, sonarContentH - 8, th);
    } else if (g_sub.viewMode == 6) {
        DrawResearchLabView(hdc, centerX + 6, sonarContentY + 4, centerW - 12, sonarContentH - 8, th);
    } else if (g_sub.viewMode == 7) {
        DrawOutpostTradeView(hdc, centerX + 6, sonarContentY + 4, centerW - 12, sonarContentH - 8, th);
    } else if (g_sub.viewMode == 8) {
        DrawCombatView(hdc, centerX + 6, sonarContentY + 4, centerW - 12, sonarContentH - 8, th);
    } else if (g_sub.viewMode == 9) {
        DrawAudioView(hdc, centerX + 6, sonarContentY + 4, centerW - 12, sonarContentH - 8, th);
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

    // SONAR TARGET TRACKER
    SelectObject(hdc, g_hFontSmall);
    SetTextColor(hdc, th->textDim);
    TextOutA(hdc, bx, cy, "SONAR TARGET TRACKER & SCANNER", 30);
    cy += 14;

    UnifiedContact rContacts[8];
    int rfCount = GetUnifiedContacts(g_sub.currentSectorIdx, rContacts);
    if (rfCount > 0 && g_sub.selectedTargetIdx < rfCount) {
        UnifiedContact* tgt = &rContacts[g_sub.selectedTargetIdx];
        snprintf(buf, sizeof(buf), "🎯 LOCKED: %.14s", tgt->discovered ? tgt->name : tgt->id);
    } else {
        snprintf(buf, sizeof(buf), "🎯 TARGET LOCK: NONE");
    }
    DrawCustomButton(hdc, ID_BTN_LOCK_TARGET, bx, cy, rightW - 18, 20, buf, 0, th->accentSonar, th);
    cy += 22;

    DrawCustomButton(hdc, ID_BTN_SCAN_TARGET, bx, cy, rightW - 18, 20, "🔬 BIO-SCAN / HARVEST TARGET", g_sub.isScanningTarget || g_sub.isDredging, th->accentEmerald, th);
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
    TextOutA(hdc, bx, cy, "SUBSYSTEMS & DREDGING CLAW", 26);
    cy += 14;

    snprintf(buf, sizeof(buf), "🗜️ DREDGING CLAW: %s", g_sub.clawDeployed ? "DEPLOYED [ACTIVE]" : "RETRACTED");
    DrawCustomButton(hdc, ID_BTN_TOGGLE_CLAW, bx, cy, rightW - 18, 18, buf, g_sub.clawDeployed, th->accentAmber, th);
    cy += 20;

    snprintf(buf, sizeof(buf), "⛏️ DREDGE SEABED / SALVAGE");
    DrawCustomButton(hdc, ID_BTN_DREDGE_SEABED, bx, cy, rightW - 18, 18, buf, g_sub.isDredging, th->accentAmber, th);
    cy += 20;

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
    TextOutA(hdc, bx + 6, cy + 28, "- Harvest 8/8 Salvage Sites", 27);
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

    // View toggles in center panel (10 buttons)
    if (my >= panelY + 28 && my <= panelY + 48) {
        int btnViewW = (centerW - 132) / 10;
        if (mx >= centerX + 8 && mx <= centerX + 8 + btnViewW - 2) return ID_BTN_VIEW_SONAR;
        if (mx >= centerX + 6 + btnViewW && mx <= centerX + 6 + btnViewW * 2 - 2) return ID_BTN_VIEW_NAVMAP;
        if (mx >= centerX + 4 + btnViewW * 2 && mx <= centerX + 4 + btnViewW * 3 - 2) return ID_BTN_VIEW_CODEX;
        if (mx >= centerX + 2 + btnViewW * 3 && mx <= centerX + 2 + btnViewW * 4 - 2) return ID_BTN_VIEW_CARGO;
        if (mx >= centerX + btnViewW * 4 && mx <= centerX + btnViewW * 5 - 2) return ID_BTN_VIEW_LAB;
        if (mx >= centerX - 2 + btnViewW * 5 && mx <= centerX - 2 + btnViewW * 6 - 2) return ID_BTN_VIEW_ENG;
        if (mx >= centerX - 4 + btnViewW * 6 && mx <= centerX - 4 + btnViewW * 7 - 2) return ID_BTN_VIEW_DAMAGE;
        if (mx >= centerX - 6 + btnViewW * 7 && mx <= centerX - 6 + btnViewW * 8 - 2) return ID_BTN_VIEW_OUTPOSTS;
        if (mx >= centerX - 8 + btnViewW * 8 && mx <= centerX - 8 + btnViewW * 9 - 2) return ID_BTN_VIEW_COMBAT;
        if (mx >= centerX - 10 + btnViewW * 9 && mx <= centerX - 10 + btnViewW * 10 - 2) return ID_BTN_VIEW_AUDIO;
        if (mx >= centerX + centerW - 124 && mx <= centerX + centerW - 8) return ID_BTN_FIELD_DIAG;
    }

    // Audio Matrix View (viewMode 9) 12 sound buttons
    if (g_sub.viewMode == 9) {
        int sonarContentY = panelY + 52;
        int sonarContentH = (panelH * 60) / 100 - 58;
        int abX = centerX + 6;
        int abY = sonarContentY + 4;
        int abW = centerW - 12;

        int scopeH = 34;
        int scopeY = abY + 26;
        int cardsY = scopeY + scopeH + 6;
        int cardsH = sonarContentH - (cardsY - abY) - 4;
        int cols = 3;
        int rows = 4;
        int margin = 6;
        int cardW = (abW - margin * (cols + 1)) / cols;
        int cardH = (cardsH - margin * (rows + 1)) / rows;

        for (int i = 0; i < 12; i++) {
            int col = i % cols;
            int row = i / cols;
            int cx = abX + margin + col * (cardW + margin);
            int cy = cardsY + margin + row * (cardH + margin);

            if (my >= cy + cardH - 20 && my <= cy + cardH - 3 && mx >= cx + 6 && mx <= cx + cardW - 6) {
                return g_audioCards[i].btnId;
            }
        }
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

    // Cargo View (viewMode 3) Transship button
    if (g_sub.viewMode == 3) {
        int sonarContentY = panelY + 52;
        int offX = centerX + 6 + centerW - 12 - 170;
        int offY = sonarContentY + 4 + 4;
        if (my >= offY && my <= offY + 20 && mx >= offX && mx <= offX + 160) {
            return ID_BTN_OFFLOAD_CARGO;
        }
    }

        // Outposts & Commissary View (viewMode 7)
    if (g_sub.viewMode == 7) {
        int sonarContentY = panelY + 52;
        int sonarContentH = (panelH * 60) / 100 - 58;
        int ebX = centerX + 6;
        int ebY = sonarContentY + 4;
        int ebW = centerW - 12;

        // Header buttons: Use Battery & Use Repair Kit
        if (my >= ebY + 4 && my <= ebY + 24) {
            if (mx >= ebX + ebW - 270 && mx <= ebX + ebW - 140) return ID_BTN_USE_BATTERY;
            if (mx >= ebX + ebW - 134 && mx <= ebX + ebW - 8) return ID_BTN_USE_REPAIR_KIT;
        }

        int ordY = ebY + 28;
        int cardsY = ordY + 26;
        int cardH = 92;
        int cardW = (ebW - 8) / 4;

        // 4 Outpost cards
        if (my >= cardsY + cardH - 24 && my <= cardsY + cardH - 4) {
            for (int i = 0; i < OUTPOST_COUNT; i++) {
                int cx = ebX + i * (cardW + 2);
                int btnW = (cardW - 12) / 2;
                if (mx >= cx + 4 && mx <= cx + 4 + btnW) return ID_BTN_SELECT_OUTPOST_0 + i;
                if (mx >= cx + 6 + btnW && mx <= cx + 6 + btnW * 2) {
                    return (g_sub.isDocked && g_sub.dockedStationIdx == i) ? ID_BTN_UNDOCK_OUTPOST : ID_BTN_DOCK_OUTPOST;
                }
            }
        }

        // Bottom utilities & commissary
        int btmY = cardsY + cardH + 6;
        int halfW = (ebW - 6) / 2;

        // Left panel (Utilities)
        if (mx >= ebX + 6 && mx <= ebX + 6 + halfW) {
            int uBtnY = btmY + 40;
            if (my >= uBtnY && my <= uBtnY + 20) return ID_BTN_TRADE_RECHARGE;
            uBtnY += 24;
            if (my >= uBtnY && my <= uBtnY + 20) return ID_BTN_TRADE_AIR;
            uBtnY += 24;
            if (my >= uBtnY && my <= uBtnY + 20) return ID_BTN_TRADE_REPAIR;
            uBtnY += 26;
            if (my >= uBtnY && my <= uBtnY + 20) return ID_BTN_TRADE_SELL_ALL;
        }

        // Right panel (Commissary)
        int rx = ebX + 12 + halfW;
        if (mx >= rx && mx <= rx + halfW) {
            int tBtnY = btmY + 38;
            int tBtnW = (halfW - 20) / 2;
            for (int i = 0; i < 5; i++) {
                if (my >= tBtnY && my <= tBtnY + 18) {
                    if (mx >= rx + 8 && mx <= rx + 8 + tBtnW) {
                        return (i == 0 ? ID_BTN_BUY_TORPEDO : (i == 1 ? ID_BTN_BUY_EMP_TORPEDO : (i == 2 ? ID_BTN_BUY_PLASMA_TORPEDO : (i == 3 ? ID_BTN_BUY_BATTERY_PACK : ID_BTN_BUY_REPAIR_KIT))));
                    }
                    if (mx >= rx + 12 + tBtnW && mx <= rx + 12 + tBtnW * 2) {
                        return (i == 0 ? ID_BTN_BARTER_TORPEDO : (i == 1 ? ID_BTN_BARTER_EMP : (i == 2 ? ID_BTN_BARTER_PLASMA : (i == 3 ? ID_BTN_BARTER_BATTERY : ID_BTN_BARTER_REPAIR))));
                    }
                }
                tBtnY += 21;
            }
        }
    }

    if (g_sub.viewMode == 4) { // Engineering Bay
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

    if (g_sub.viewMode == 5) { // Damage Control & Bulkhead Flood View
        int sonarContentY = panelY + 52;
        int dcX = centerX + 6;
        int dcY = sonarContentY + 4;
        int dcW = centerW - 12;

        // Top action buttons
        if (my >= dcY + 4 && my <= dcY + 24) {
            if (mx >= dcX + dcW - 380 && mx <= dcX + dcW - 250) return ID_BTN_BLEED_VALVE;
            if (mx >= dcX + dcW - 244 && mx <= dcX + dcW - 114) return ID_BTN_PUMP_MODE;
            if (mx >= dcX + dcW - 108 && mx <= dcX + dcW - 8) return ID_BTN_SIM_BREACH;
        }

        int contentY = dcY + 28;
        int marginDc = 6;
        int cardW = (dcW - marginDc * 5) / 4;

        for (int i = 0; i < COMPARTMENT_COUNT; i++) {
            int cx = dcX + marginDc + i * (cardW + marginDc);
            int chY = contentY + 32;
            int chH = 44;
            int sy = chY + chH + 6 + 14 + 16 + (g_sub.compartments[i].isRepairing ? 10 : 4);
            int btnH = 18;

            if (mx >= cx + 6 && mx <= cx + cardW - 6) {
                if (my >= sy && my <= sy + btnH) return ID_BTN_DOOR_BAY_0 + i;
                if (my >= sy + btnH + 3 && my <= sy + (btnH + 3) + btnH) return ID_BTN_AIRDAM_BAY_0 + i;
                if (my >= sy + (btnH + 3) * 2 && my <= sy + (btnH + 3) * 2 + btnH) return ID_BTN_ROUTE_BAY_0 + i;
                if (my >= sy + (btnH + 3) * 3 && my <= sy + (btnH + 3) * 3 + btnH) return ID_BTN_PATCH_BAY_0 + i;
            }
        }
    }

    if (g_sub.viewMode == 6) { // Research Lab & Deep-Sea Biology View
        int sonarH = (panelH * 60) / 100;
        int sonarContentY = panelY + 52;
        int sonarContentH = sonarH - 58;
        int labX = centerX + 6;
        int labY = sonarContentY + 4;
        int labW = centerW - 12;

        // Incubate button in header
        if (my >= labY + 4 && my <= labY + 24 && mx >= labX + labW - 190 && mx <= labX + labW - 10) {
            return ID_BTN_INCUBATE_BIO;
        }

        int contentY = labY + 28;
        int contentH = sonarContentH - 8 - 34;

        int marginLab = 6;
        int gridW = (labW - marginLab * 3) / 2;
        int gridH = (contentH - marginLab * 3) / 2;
        int c1x = labX + marginLab;
        int c2x = labX + marginLab * 2 + gridW;
        int r1y = contentY + marginLab;
        int r2y = contentY + marginLab * 2 + gridH;

        if (my >= r1y + gridH - 24 && my <= r1y + gridH - 6) {
            if (mx >= c1x + 8 && mx <= c1x + gridW - 8) return ID_BTN_RES_POLYMERS;
            if (mx >= c2x + 8 && mx <= c2x + gridW - 8) return ID_BTN_RES_BIOLUM;
        }
        if (my >= r2y + gridH - 24 && my <= r2y + gridH - 6) {
            if (mx >= c1x + 8 && mx <= c1x + gridW - 8) return ID_BTN_RES_BIOFUEL;
            if (mx >= c2x + 8 && mx <= c2x + gridW - 8) return ID_BTN_RES_REGEN;
        }
    }

    if (g_sub.viewMode == 8) { // Combat & Defense View
        int sonarContentY = panelY + 52;
        int sonarContentH = (panelH * 60) / 100 - 58;
        int cbX = centerX + 6;
        int cbY = sonarContentY + 4;
        int cbW = centerW - 12;

        // Header countermeasures buttons
        if (my >= cbY + 4 && my <= cbY + 24) {
            if (mx >= cbX + cbW - 380 && mx <= cbX + cbW - 265) return ID_BTN_COMBAT_DECOY;
            if (mx >= cbX + cbW - 260 && mx <= cbX + cbW - 130) return ID_BTN_COMBAT_SHOCKWAVE;
            if (mx >= cbX + cbW - 124 && mx <= cbX + cbW - 8) return ID_BTN_COMBAT_SILENT;
        }

        int contentY = cbY + 26;
        int tubesH = 78;
        int margin = 6;
        int tubeW = (cbW - margin * 5) / 4;

        // 4 Torpedo Tubes action buttons
        if (my >= contentY + 54 && my <= contentY + 72) {
            int btnW = (tubeW - 16) / 2;
            for (int i = 0; i < 4; i++) {
                int tx = cbX + margin + i * (tubeW + margin);
                if (mx >= tx + 6 && mx <= tx + 6 + btnW) return ID_BTN_FIRE_TUBE_0 + i;
                if (mx >= tx + 10 + btnW && mx <= tx + 10 + btnW * 2) return ID_BTN_CYCLE_TUBE_0 + i;
            }
        }

        // 6 Hostile Threat action buttons
        int thrY = contentY + tubesH + 6;
        int thrH = sonarContentH - (thrY - cbY) - 6;
        int cols = 3;
        int rows = 2;
        int cardW = (cbW - margin * (cols + 1)) / cols;
        int cardH = (thrH - margin * (rows + 1)) / rows;

        for (int i = 0; i < THREAT_COUNT; i++) {
            int c = i % cols;
            int r = i / cols;
            int cx = cbX + margin + c * (cardW + margin);
            int cy = thrY + margin + r * (cardH + margin);
            int aBtnW = (cardW - 16) / 2;

            if (my >= cy + cardH - 22 && my <= cy + cardH - 4) {
                if (mx >= cx + 6 && mx <= cx + 6 + aBtnW) return ID_BTN_LOCK_THREAT_0 + i;
                if (mx >= cx + 10 + aBtnW && mx <= cx + 10 + aBtnW * 2) return ID_BTN_FIRE_THREAT_0 + i;
            }
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

    if (my >= cy && my <= cy + 18 && mx >= bx && mx <= bx + rightW - 18) return ID_BTN_TOGGLE_CLAW;
    cy += 20;
    if (my >= cy && my <= cy + 18 && mx >= bx && mx <= bx + rightW - 18) return ID_BTN_DREDGE_SEABED;
    cy += 20;
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

        case ID_BTN_VIEW_CARGO:
            g_sub.viewMode = 3;
            PlaySoundAsync(540, 80);
            break;

        case ID_BTN_VIEW_LAB:
            g_sub.viewMode = 6;
            PlaySoundAsync(680, 80);
            break;

        case ID_BTN_VIEW_ENG:
            g_sub.viewMode = 4;
            PlaySoundAsync(580, 80);
            break;

        case ID_BTN_VIEW_DAMAGE:
            g_sub.viewMode = 5;
            PlaySoundAsync(500, 80);
            break;

        case ID_BTN_INCUBATE_BIO: {
            if (g_sub.surveyPoints >= 45) {
                g_sub.surveyPoints -= 45;
                int r = rand() % 4;
                const char* sName = "Plankton";
                if (r == 0) { g_sub.bioPlankton += 2; sName = "Plankton"; }
                else if (r == 1) { g_sub.bioCephalopod += 2; sName = "Cephalopod DNA"; }
                else if (r == 2) { g_sub.bioEnzymes += 2; sName = "Chemosynthetic Enzymes"; }
                else { g_sub.bioHadal += 2; sName = "Hadal Biomass"; }
                PlayLabCentrifuge();
                snprintf(msg, sizeof(msg), "🧬 SPECIMEN INCUBATOR: Cultured 2x %s (-45 Credits).", sName);
                AddLog(msg, th->accentEmerald);
            } else {
                PlaySoundAsync(220, 120);
                AddLog("INCUBATOR ERROR: Need 45 research credits to cultivate bio-specimens.", th->accentAmber);
            }
            break;
        }

        case ID_BTN_RES_POLYMERS: {
            if (g_sub.resPolymers < 4) {
                const PolymerResearch* nextP = &g_polymerRes[g_sub.resPolymers];
                if (g_sub.surveyPoints >= nextP->cost && g_sub.bioPlankton >= nextP->reqPlankton) {
                    g_sub.surveyPoints -= nextP->cost;
                    g_sub.bioPlankton -= nextP->reqPlankton;
                    g_sub.resPolymers++;
                    PlayResearchBreakthrough();
                    snprintf(msg, sizeof(msg), "🔬 RESEARCH BREAKTHROUGH: [%s] synthesized! %s", nextP->name, nextP->desc);
                    AddLog(msg, th->accentEmerald);
                }
            }
            break;
        }

        case ID_BTN_RES_BIOLUM: {
            if (g_sub.resBiolum < 4) {
                const BiolumResearch* nextB = &g_biolumRes[g_sub.resBiolum];
                if (g_sub.surveyPoints >= nextB->cost && g_sub.bioCephalopod >= nextB->reqCephalopod) {
                    g_sub.surveyPoints -= nextB->cost;
                    g_sub.bioCephalopod -= nextB->reqCephalopod;
                    g_sub.resBiolum++;
                    PlayResearchBreakthrough();
                    snprintf(msg, sizeof(msg), "🔬 RESEARCH BREAKTHROUGH: [%s] synthesized! %s", nextB->name, nextB->desc);
                    AddLog(msg, th->accentEmerald);
                }
            }
            break;
        }

        case ID_BTN_RES_BIOFUEL: {
            if (g_sub.resBiofuel < 4) {
                const BiofuelResearch* nextF = &g_biofuelRes[g_sub.resBiofuel];
                if (g_sub.surveyPoints >= nextF->cost && g_sub.bioEnzymes >= nextF->reqEnzymes) {
                    g_sub.surveyPoints -= nextF->cost;
                    g_sub.bioEnzymes -= nextF->reqEnzymes;
                    g_sub.resBiofuel++;
                    PlayResearchBreakthrough();
                    snprintf(msg, sizeof(msg), "🔬 RESEARCH BREAKTHROUGH: [%s] synthesized! %s", nextF->name, nextF->desc);
                    AddLog(msg, th->accentEmerald);
                }
            }
            break;
        }

        case ID_BTN_VIEW_OUTPOSTS:
            g_sub.viewMode = 7;
            PlaySoundAsync(580, 80);
            break;

        case ID_BTN_VIEW_COMBAT:
            g_sub.viewMode = 8;
            PlaySoundAsync(600, 80);
            break;

        case ID_BTN_VIEW_AUDIO:
            g_sub.viewMode = 9;
            PlaySoundAsync(720, 80);
            break;

        case ID_BTN_TEST_SONAR_PING:
            PlaySoundAsync(1920, 250);
            AddLog("Acoustic hydrophone test: Active Sonar Ping & Echo.", th->accentSonar);
            break;

        case ID_BTN_TEST_WHALE_SONG:
            PlayWhaleSong();
            AddLog("Acoustic hydrophone test: Abyssal Whale & Leviathan Song.", th->accentEmerald);
            break;

        case ID_BTN_TEST_BALLAST_BLOW:
            PlayBallastBlowHiss();
            AddLog("Acoustic hydrophone test: High-Pressure Ballast Blow Hiss.", th->accentAmber);
            break;

        case ID_BTN_TEST_HULL_GROAN:
            PlayHullPressureGroan();
            AddLog("Acoustic hydrophone test: Hydrostatic Hull Pressure Groan.", th->accentRed);
            break;

        case ID_BTN_TEST_TORPEDO_LAUNCH:
            PlayTorpedoLaunchSound();
            AddLog("Acoustic hydrophone test: Torpedo Ejection & Propeller Cavitation.", th->accentSonar);
            break;

        case ID_BTN_TEST_PLASMA_LAUNCH:
            PlayPlasmaLaunchSound();
            AddLog("Acoustic hydrophone test: Thermal Plasma Torpedo Flare.", th->accentRed);
            break;

        case ID_BTN_TEST_SHOCKWAVE:
            PlayShockwaveSound();
            AddLog("Acoustic hydrophone test: Hull EMP Shockwave Burst.", th->accentSonar);
            break;

        case ID_BTN_TEST_EXPLOSION:
            PlayTorpedoExplosionSound();
            AddLog("Acoustic hydrophone test: Undersea Depth Detonation Shockwave.", th->accentRed);
            break;

        case ID_BTN_TEST_DECOY:
            PlayDecoySound();
            AddLog("Acoustic hydrophone test: Acoustic Decoy & Cavitation Screen.", th->accentEmerald);
            break;

        case ID_BTN_TEST_WELDING:
            PlayWeldingSound();
            AddLog("Acoustic hydrophone test: Damage Control Bulkhead Arc Weld.", th->accentAmber);
            break;

        case ID_BTN_TEST_ALARM:
            PlayAlarmKlaxon();
            AddLog("Acoustic hydrophone test: Flood & Collision Siren Alert.", th->accentRed);
            break;

        case ID_BTN_TEST_CENTRIFUGE:
            PlayLabCentrifuge();
            AddLog("Acoustic hydrophone test: Laboratory Centrifuge & Bio-Imaging.", th->accentEmerald);
            break;

        case ID_BTN_FIRE_TUBE_0:
        case ID_BTN_FIRE_TUBE_1:
        case ID_BTN_FIRE_TUBE_2:
        case ID_BTN_FIRE_TUBE_3:
            FireTorpedoTube(cmdId - ID_BTN_FIRE_TUBE_0);
            break;

        case ID_BTN_CYCLE_TUBE_0:
        case ID_BTN_CYCLE_TUBE_1:
        case ID_BTN_CYCLE_TUBE_2:
        case ID_BTN_CYCLE_TUBE_3:
            CycleTubeOrdnance(cmdId - ID_BTN_CYCLE_TUBE_0);
            break;

        case ID_BTN_COMBAT_DECOY:
        case ID_BTN_QUICK_LAUNCH_DECOY:
            LaunchAcousticDecoy();
            break;

        case ID_BTN_COMBAT_SHOCKWAVE:
            TriggerShockwaveDischarge();
            break;

        case ID_BTN_COMBAT_SILENT:
            ToggleSilentRunning();
            break;

        case ID_BTN_LOCK_THREAT_0:
        case ID_BTN_LOCK_THREAT_1:
        case ID_BTN_LOCK_THREAT_2:
        case ID_BTN_LOCK_THREAT_3:
        case ID_BTN_LOCK_THREAT_4:
        case ID_BTN_LOCK_THREAT_5:
            LockThreatTarget(cmdId - ID_BTN_LOCK_THREAT_0);
            break;

        case ID_BTN_FIRE_THREAT_0:
        case ID_BTN_FIRE_THREAT_1:
        case ID_BTN_FIRE_THREAT_2:
        case ID_BTN_FIRE_THREAT_3:
        case ID_BTN_FIRE_THREAT_4:
        case ID_BTN_FIRE_THREAT_5:
            FireAtThreat(cmdId - ID_BTN_FIRE_THREAT_0);
            break;

        case ID_BTN_QUICK_FIRE_ACOUSTIC: {
            int fired = 0;
            for (int i = 0; i < 4; i++) {
                if (g_sub.tubes[i].type == 0 && g_sub.tubes[i].status == 0 && g_sub.torpedoes > 0) {
                    FireTorpedoTube(i);
                    fired = 1;
                    break;
                }
            }
            if (!fired) {
                for (int i = 0; i < 4; i++) {
                    if (g_sub.tubes[i].status == 0 && g_sub.torpedoes > 0) {
                        g_sub.tubes[i].type = 0;
                        FireTorpedoTube(i);
                        fired = 1;
                        break;
                    }
                }
            }
            if (!fired) {
                PlaySoundAsync(200, 150);
                AddLog("No ready tubes for acoustic torpedo firing!", th->accentAmber);
            }
            break;
        }

        case ID_BTN_QUICK_FIRE_EMP: {
            int fired = 0;
            for (int i = 0; i < 4; i++) {
                if (g_sub.tubes[i].type == 1 && g_sub.tubes[i].status == 0 && g_sub.empTorpedoes > 0) {
                    FireTorpedoTube(i);
                    fired = 1;
                    break;
                }
            }
            if (!fired) {
                for (int i = 0; i < 4; i++) {
                    if (g_sub.tubes[i].status == 0 && g_sub.empTorpedoes > 0) {
                        g_sub.tubes[i].type = 1;
                        FireTorpedoTube(i);
                        fired = 1;
                        break;
                    }
                }
            }
            if (!fired) {
                PlaySoundAsync(200, 150);
                AddLog("No ready tubes for EMP torpedo firing!", th->accentAmber);
            }
            break;
        }

        case ID_BTN_QUICK_FIRE_PLASMA: {
            int fired = 0;
            for (int i = 0; i < 4; i++) {
                if (g_sub.tubes[i].type == 2 && g_sub.tubes[i].status == 0 && g_sub.plasmaTorpedoes > 0) {
                    FireTorpedoTube(i);
                    fired = 1;
                    break;
                }
            }
            if (!fired) {
                for (int i = 0; i < 4; i++) {
                    if (g_sub.tubes[i].status == 0 && g_sub.plasmaTorpedoes > 0) {
                        g_sub.tubes[i].type = 2;
                        FireTorpedoTube(i);
                        fired = 1;
                        break;
                    }
                }
            }
            if (!fired) {
                PlaySoundAsync(200, 150);
                AddLog("No ready tubes for Plasma torpedo firing!", th->accentAmber);
            }
            break;
        }

        case ID_BTN_SELECT_OUTPOST_0:
        case ID_BTN_SELECT_OUTPOST_1:
        case ID_BTN_SELECT_OUTPOST_2:
        case ID_BTN_SELECT_OUTPOST_3: {
            int selIdx = cmdId - ID_BTN_SELECT_OUTPOST_0;
            g_sub.selectedOutpostIdx = selIdx;
            PlaySoundAsync(620, 80);
            char sMsg[128];
            snprintf(sMsg, sizeof(sMsg), "🧭 Station Nav Lock set: [%s] (Depth: %.0fm)", g_outposts[selIdx].name, g_outposts[selIdx].depth);
            AddLog(sMsg, th->accentSonar);
            break;
        }

        case ID_BTN_DOCK_OUTPOST: {
            int selIdx = g_sub.selectedOutpostIdx;
            const OutpostInfo* out = &g_outposts[selIdx];
            float dx = out->x - g_sub.posX;
            float dy = out->y - g_sub.posY;
            float distM = sqrtf(dx * dx + dy * dy) * 1000.0f;
            float depthDiff = fabsf(g_sub.depth - out->depth);
            int inRange = (distM <= 300.0f && depthDiff <= (selIdx == 0 ? 25.0f : (selIdx == 3 ? 70.0f : 50.0f)));

            if (inRange) {
                g_sub.isDocked = 1;
                g_sub.dockedStationIdx = selIdx;
                g_sub.speed = 0.0f;
                g_sub.targetSpeed = 0.0f;
                g_sub.throttleMode = 1; // STOP
                PlayDockingClamps();
                PlayTradeChime();
                char dMsg[128];
                snprintf(dMsg, sizeof(dMsg), "⚓ DOCKING CLAMPS LOCKED: Moored at [%s]! Utilities & trading online.", out->name);
                AddLog(dMsg, th->accentEmerald);
            } else {
                PlaySoundAsync(220, 150);
                AddLog("DOCKING REJECTED: Approach station within 300m range and depth level!", th->accentAmber);
            }
            break;
        }

        case ID_BTN_UNDOCK_OUTPOST:
            if (g_sub.isDocked) {
                g_sub.isDocked = 0;
                PlayDockingClamps();
                AddLog("⚓ DOCKING CLAMPS RELEASED: Disembarked from station. Resuming cruise.", th->textDim);
            }
            break;

        case ID_BTN_USE_BATTERY:
            if (g_sub.batteryPacks > 0 && g_sub.battery < 98.0f) {
                g_sub.batteryPacks--;
                g_sub.battery = min(100.0f, g_sub.battery + 40.0f);
                PlaySoundAsync(880, 200);
                AddLog("⚡ EMERGENCY POWER: 1x Reserve Battery Pack discharged (+40% charge).", th->accentEmerald);
            } else if (g_sub.batteryPacks == 0) {
                PlaySoundAsync(220, 150);
                AddLog("No reserve battery packs in magazine!", th->accentAmber);
            }
            break;

        case ID_BTN_USE_REPAIR_KIT:
            if (g_sub.repairKits > 0) {
                g_sub.repairKits--;
                g_sub.hull = min(100.0f, g_sub.hull + 30.0f);
                for (int i = 0; i < COMPARTMENT_COUNT; i++) {
                    g_sub.compartments[i].integrity = min(100.0f, g_sub.compartments[i].integrity + 35.0f);
                    if (g_sub.compartments[i].breachTier > 0) g_sub.compartments[i].breachTier--;
                }
                PlayMineralChime();
                AddLog("🛠️ NANOPATCH APPLIED: Structural repair resin healed bulkheads (+30% Hull / Leaks reduced).", th->accentEmerald);
            } else {
                PlaySoundAsync(220, 150);
                AddLog("No structural repair kits in magazine!", th->accentAmber);
            }
            break;

        case ID_BTN_TRADE_SELL_ALL:
            if (g_sub.cargoTotalValue > 0) {
                int offVal = g_sub.cargoTotalValue;
                g_sub.surveyPoints += offVal;
                g_sub.cargoManganese = 0;
                g_sub.cargoSunkenGold = 0;
                g_sub.cargoTitaniumScrap = 0;
                g_sub.cargoSmokerCrystals = 0;
                g_sub.cargoHadalPrisms = 0;
                RecalculateCargo();
                PlayTradeChime();
                char trMsg[128];
                snprintf(trMsg, sizeof(trMsg), "💎 MINERAL TRADE COMPLETE: Sold all cargo to station for +%d Research Credits!", offVal);
                AddLog(trMsg, th->accentEmerald);
            }
            break;

        case ID_BTN_TRADE_RECHARGE:
            if (g_sub.isDocked) {
                g_sub.battery = 100.0f;
                g_sub.o2 = 100.0f;
                g_sub.co2 = 0.04f;
                g_sub.scrubberStatus = 100.0f;
                PlayTradeChime();
                AddLog("⚡ STATION UTILITY: Main battery bank & life support scrubbers restored to 100%!", th->accentEmerald);
            }
            break;

        case ID_BTN_TRADE_AIR:
            if (g_sub.isDocked) {
                g_sub.airReservoir = g_sub.maxAirReservoir;
                PlaySoundAsync(1200, 150);
                AddLog("💨 STATION UTILITY: High-pressure air reservoir fully recharged to max!", th->accentEmerald);
            }
            break;

        case ID_BTN_TRADE_REPAIR:
            if (g_sub.isDocked && g_sub.surveyPoints >= 40) {
                g_sub.surveyPoints -= 40;
                g_sub.hull = 100.0f;
                for (int i = 0; i < COMPARTMENT_COUNT; i++) {
                    g_sub.compartments[i].integrity = 100.0f;
                    g_sub.compartments[i].water = 0.0f;
                    g_sub.compartments[i].breachTier = 0;
                    g_sub.compartments[i].leakRate = 0.0f;
                }
                g_sub.bilgeWater = 0.0f;
                PlayMineralChime();
                AddLog("🛠️ DRYDOCK OVERHAUL: Submersible hull restored to 100% and all compartment breaches repaired (-40 PTS)!", th->accentEmerald);
            }
            break;

        case ID_BTN_BUY_TORPEDO:
            if (g_sub.isDocked && g_sub.torpedoes < 12 && g_sub.surveyPoints >= 60) {
                g_sub.surveyPoints -= 60;
                g_sub.torpedoes = min(12, g_sub.torpedoes + 2);
                PlayTradeChime();
                AddLog("🛒 PURCHASE: Acquired 2x Acoustic Torpedoes (-60 PTS)!", th->accentEmerald);
            }
            break;

        case ID_BTN_BARTER_TORPEDO:
            if (g_sub.isDocked && g_sub.torpedoes < 12 && g_sub.cargoManganese >= 1) {
                g_sub.cargoManganese -= 1;
                g_sub.torpedoes = min(12, g_sub.torpedoes + 2);
                RecalculateCargo();
                PlayTradeChime();
                AddLog("🛒 BARTER: Exchanged 1x Manganese for 2x Acoustic Torpedoes!", th->accentEmerald);
            }
            break;

        case ID_BTN_BUY_EMP_TORPEDO:
            if (g_sub.isDocked && g_sub.empTorpedoes < 6 && g_sub.surveyPoints >= 90) {
                g_sub.surveyPoints -= 90;
                g_sub.empTorpedoes = min(6, g_sub.empTorpedoes + 1);
                PlayTradeChime();
                AddLog("🛒 PURCHASE: Acquired 1x EMP Shock Torpedo (-90 PTS)!", th->accentEmerald);
            }
            break;

        case ID_BTN_BARTER_EMP:
            if (g_sub.isDocked && g_sub.empTorpedoes < 6 && g_sub.cargoTitaniumScrap >= 1) {
                g_sub.cargoTitaniumScrap -= 1;
                g_sub.empTorpedoes = min(6, g_sub.empTorpedoes + 1);
                RecalculateCargo();
                PlayTradeChime();
                AddLog("🛒 BARTER: Exchanged 1x Titanium Scrap for 1x EMP Shock Torpedo!", th->accentEmerald);
            }
            break;

        case ID_BTN_BUY_PLASMA_TORPEDO:
            if (g_sub.isDocked && g_sub.plasmaTorpedoes < 4 && g_sub.surveyPoints >= 150) {
                g_sub.surveyPoints -= 150;
                g_sub.plasmaTorpedoes = min(4, g_sub.plasmaTorpedoes + 1);
                PlayTradeChime();
                AddLog("🛒 PURCHASE: Acquired 1x Thermal Plasma Torpedo (-150 PTS)!", th->accentEmerald);
            }
            break;

        case ID_BTN_BARTER_PLASMA:
            if (g_sub.isDocked && g_sub.plasmaTorpedoes < 4) {
                if (g_sub.cargoSmokerCrystals >= 1) {
                    g_sub.cargoSmokerCrystals -= 1;
                    g_sub.plasmaTorpedoes = min(4, g_sub.plasmaTorpedoes + 1);
                    RecalculateCargo();
                    PlayTradeChime();
                    AddLog("🛒 BARTER: Exchanged 1x Smoker Crystal for 1x Thermal Plasma Torpedo!", th->accentEmerald);
                } else if (g_sub.cargoHadalPrisms >= 1) {
                    g_sub.cargoHadalPrisms -= 1;
                    g_sub.plasmaTorpedoes = min(4, g_sub.plasmaTorpedoes + 1);
                    RecalculateCargo();
                    PlayTradeChime();
                    AddLog("🛒 BARTER: Exchanged 1x Hadal Prism for 1x Thermal Plasma Torpedo!", th->accentEmerald);
                }
            }
            break;

        case ID_BTN_BUY_BATTERY_PACK:
            if (g_sub.isDocked && g_sub.batteryPacks < 8 && g_sub.surveyPoints >= 40) {
                g_sub.surveyPoints -= 40;
                g_sub.batteryPacks = min(8, g_sub.batteryPacks + 1);
                PlayTradeChime();
                AddLog("🛒 PURCHASE: Acquired 1x Reserve Battery Pack (-40 PTS)!", th->accentEmerald);
            }
            break;

        case ID_BTN_BARTER_BATTERY:
            if (g_sub.isDocked && g_sub.batteryPacks < 8 && g_sub.cargoManganese >= 1) {
                g_sub.cargoManganese -= 1;
                g_sub.batteryPacks = min(8, g_sub.batteryPacks + 1);
                RecalculateCargo();
                PlayTradeChime();
                AddLog("🛒 BARTER: Exchanged 1x Manganese for 1x Reserve Battery Pack!", th->accentEmerald);
            }
            break;

        case ID_BTN_BUY_REPAIR_KIT:
            if (g_sub.isDocked && g_sub.repairKits < 6 && g_sub.surveyPoints >= 50) {
                g_sub.surveyPoints -= 50;
                g_sub.repairKits = min(6, g_sub.repairKits + 1);
                PlayTradeChime();
                AddLog("🛒 PURCHASE: Acquired 1x Nanopatch Repair Kit (-50 PTS)!", th->accentEmerald);
            }
            break;

        case ID_BTN_BARTER_REPAIR:
            if (g_sub.isDocked && g_sub.repairKits < 6 && g_sub.cargoTitaniumScrap >= 1) {
                g_sub.cargoTitaniumScrap -= 1;
                g_sub.repairKits = min(6, g_sub.repairKits + 1);
                RecalculateCargo();
                PlayTradeChime();
                AddLog("🛒 BARTER: Exchanged 1x Titanium Scrap for 1x Nanopatch Repair Kit!", th->accentEmerald);
            }
            break;

        case ID_BTN_RES_REGEN: {
            if (g_sub.resRegen < 4) {
                const RegenResearch* nextR = &g_regenRes[g_sub.resRegen];
                if (g_sub.surveyPoints >= nextR->cost && g_sub.bioHadal >= nextR->reqHadal) {
                    g_sub.surveyPoints -= nextR->cost;
                    g_sub.bioHadal -= nextR->reqHadal;
                    g_sub.resRegen++;
                    PlayResearchBreakthrough();
                    snprintf(msg, sizeof(msg), "🔬 RESEARCH BREAKTHROUGH: [%s] synthesized! %s", nextR->name, nextR->desc);
                    AddLog(msg, th->accentEmerald);
                }
            }
            break;
        }

        case ID_BTN_BLEED_VALVE:
            g_sub.bleedValveOpen = !g_sub.bleedValveOpen;
            if (g_sub.bleedValveOpen) {
                PlayPressureBleed();
                AddLog("Cabin equalization bleed valve OPENED: equalizing overpressure.", th->accentAmber);
            } else {
                PlaySoundAsync(350, 80);
                AddLog("Cabin equalization bleed valve CLOSED.", th->textDim);
            }
            break;

        case ID_BTN_PUMP_MODE: {
            g_sub.bilgePumpMode = (g_sub.bilgePumpMode + 1) % 7;
            g_sub.bilgePumpActive = (g_sub.bilgePumpMode > 0);
            PlaySoundAsync(480, 80);
            const char* modeNames[] = { "STANDBY", "AUTO-BALANCE", "ROUTE BOW", "ROUTE CMD", "ROUTE ENG", "ROUTE AFT", "OVERDRIVE 2.5x" };
            snprintf(msg, sizeof(msg), "Bilge pumping system switched to: [%s]", modeNames[g_sub.bilgePumpMode]);
            AddLog(msg, g_sub.bilgePumpMode == 6 ? th->accentAmber : th->accentEmerald);
            break;
        }

        case ID_BTN_SIM_BREACH: {
            int bay = rand() % COMPARTMENT_COUNT;
            int newTier = (g_sub.compartments[bay].breachTier < 3) ? g_sub.compartments[bay].breachTier + 1 : 3;
            g_sub.compartments[bay].breachTier = newTier;
            g_sub.compartments[bay].integrity = max(10.0f, g_sub.compartments[bay].integrity - 30.0f);
            g_sub.cabinPressure = min(2.5f, g_sub.cabinPressure + 0.15f);
            PlayAlarmKlaxon();
            g_sub.viewMode = 5;
            const char* tNames[] = { "INTACT", "HAIRLINE WEEP", "SEAM RUPTURE", "TORRENTIAL FRACTURE" };
            snprintf(msg, sizeof(msg), "🚨 HULL BREACH TRIGGERED in [%s]: %s! Integrity %.0f%%",
                     g_sub.compartments[bay].name, tNames[newTier], g_sub.compartments[bay].integrity);
            AddLog(msg, th->accentRed);
            break;
        }

        case ID_BTN_DOOR_BAY_0:
        case ID_BTN_DOOR_BAY_1:
        case ID_BTN_DOOR_BAY_2:
        case ID_BTN_DOOR_BAY_3: {
            int idx = cmdId - ID_BTN_DOOR_BAY_0;
            g_sub.compartments[idx].doorSealed = !g_sub.compartments[idx].doorSealed;
            PlaySoundAsync(g_sub.compartments[idx].doorSealed ? 300 : 600, 100);
            snprintf(msg, sizeof(msg), "Bulkhead watertight door [%s]: %s",
                     g_sub.compartments[idx].name, g_sub.compartments[idx].doorSealed ? "SEALED [ISOLATED]" : "UNSEALED [OPEN]");
            AddLog(msg, g_sub.compartments[idx].doorSealed ? th->accentAmber : th->textDim);
            break;
        }

        case ID_BTN_AIRDAM_BAY_0:
        case ID_BTN_AIRDAM_BAY_1:
        case ID_BTN_AIRDAM_BAY_2:
        case ID_BTN_AIRDAM_BAY_3: {
            int idx = cmdId - ID_BTN_AIRDAM_BAY_0;
            g_sub.compartments[idx].airDam = !g_sub.compartments[idx].airDam;
            if (g_sub.compartments[idx].airDam) {
                PlayPressureBleed();
                g_sub.airReservoir = max(0.0f, g_sub.airReservoir - 12.0f);
            } else {
                PlaySoundAsync(350, 80);
            }
            snprintf(msg, sizeof(msg), "HP Air Damming on [%s]: %s (-60%% leak rate).",
                     g_sub.compartments[idx].name, g_sub.compartments[idx].airDam ? "CHARGED [ACTIVE]" : "VENTED [OFF]");
            AddLog(msg, g_sub.compartments[idx].airDam ? th->accentAmber : th->textDim);
            break;
        }

        case ID_BTN_ROUTE_BAY_0:
        case ID_BTN_ROUTE_BAY_1:
        case ID_BTN_ROUTE_BAY_2:
        case ID_BTN_ROUTE_BAY_3: {
            int idx = cmdId - ID_BTN_ROUTE_BAY_0;
            g_sub.bilgePumpMode = idx + 2; // Route Bow(2), Route Cmd(3), Route Eng(4), Route Aft(5)
            g_sub.bilgePumpActive = 1;
            PlaySoundAsync(520, 80);
            snprintf(msg, sizeof(msg), "Bilge suction routed directly to compartment [%s]!", g_sub.compartments[idx].name);
            AddLog(msg, th->accentEmerald);
            break;
        }

        case ID_BTN_PATCH_BAY_0:
        case ID_BTN_PATCH_BAY_1:
        case ID_BTN_PATCH_BAY_2:
        case ID_BTN_PATCH_BAY_3: {
            int idx = cmdId - ID_BTN_PATCH_BAY_0;
            CompartmentInfo* c = &g_sub.compartments[idx];
            if (c->breachTier > 0 || c->integrity < 100.0f) {
                if (!c->isRepairing) {
                    c->isRepairing = 1;
                    c->repairProgress = 0.0f;
                    PlayWeldingSound();
                    snprintf(msg, sizeof(msg), "⚡ Damage control crew deploying underwater plasma welder to [%s]...", c->name);
                    AddLog(msg, th->accentAmber);
                }
            } else {
                PlaySoundAsync(300, 60);
                snprintf(msg, sizeof(msg), "Compartment [%s] is intact. No structural repairs required.", c->name);
                AddLog(msg, th->textDim);
            }
            break;
        }

        case ID_BTN_LOCK_TARGET:
        case ID_BTN_HUD_NEXT_TARGET: {
            UnifiedContact contacts[8];
            int fCount = GetUnifiedContacts(g_sub.currentSectorIdx, contacts);
            if (fCount > 0) {
                g_sub.selectedTargetIdx = (g_sub.selectedTargetIdx + 1) % fCount;
                UnifiedContact* tgt = &contacts[g_sub.selectedTargetIdx];
                PlaySoundAsync(700, 80);
                snprintf(msg, sizeof(msg), "Sonar tracking locked onto [%s] (%s).", tgt->discovered ? tgt->name : tgt->id, tgt->isSalvage ? "SALVAGE" : "FAUNA");
                AddLog(msg, tgt->isSalvage ? th->accentAmber : th->accentSonar);
            }
            break;
        }

        case ID_BTN_SCAN_TARGET:
        case ID_BTN_HUD_SCAN_TARGET: {
            UnifiedContact contacts[8];
            int fCount = GetUnifiedContacts(g_sub.currentSectorIdx, contacts);
            if (fCount > 0 && g_sub.selectedTargetIdx < fCount) {
                UnifiedContact* tgt = &contacts[g_sub.selectedTargetIdx];
                float dx = tgt->x - g_sub.posX;
                float dy = tgt->y - g_sub.posY;
                float distM = sqrtf(dx * dx + dy * dy) * 1000.0f;
                float depthDiff = fabsf(g_sub.depth - tgt->depth);

                if (tgt->isSalvage) {
                    if (distM <= 220.0f && depthDiff <= 90.0f) {
                        if (!g_sub.clawDeployed) {
                            g_sub.clawDeployed = 1;
                            PlayClawServo();
                        }
                        if (!g_sub.isDredging) {
                            g_sub.isDredging = 1;
                            g_sub.dredgeProgress = 0.0f;
                            PlayClawServo();
                            snprintf(msg, sizeof(msg), "🗜️ Salvage claw dredging site [%s]...", tgt->name);
                            AddLog(msg, th->accentAmber);
                        }
                    } else {
                        PlaySoundAsync(220, 150);
                        snprintf(msg, sizeof(msg), "SALVAGE SITE OUT OF REACH: %.0fm away (Depth diff: %.0fm). Steer closer!", distM, depthDiff);
                        AddLog(msg, th->accentAmber);
                    }
                } else {
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
            }
            break;
        }

        case ID_BTN_TOGGLE_CLAW:
            g_sub.clawDeployed = !g_sub.clawDeployed;
            PlayClawServo();
            snprintf(msg, sizeof(msg), "Hydraulic dredging claw arm %s.", g_sub.clawDeployed ? "DEPLOYED [ACTIVE]" : "RETRACTED [STOWED]");
            AddLog(msg, th->accentAmber);
            break;

        case ID_BTN_DREDGE_SEABED: {
            if (!g_sub.clawDeployed) {
                g_sub.clawDeployed = 1;
                PlayClawServo();
            }

            if (g_sub.isDredging) break;

            if (fabsf(g_sub.speed) > 3.0f) {
                PlaySoundAsync(200, 150);
                snprintf(msg, sizeof(msg), "DREDGING ABORTED: Speed too high (%.1f kts). Reduce throttle below 3.0 kts!", g_sub.speed);
                AddLog(msg, th->accentAmber);
                break;
            }

            RecalculateCargo();
            if (g_sub.cargoTotalWeight >= g_sub.cargoMaxWeight) {
                PlaySoundAsync(200, 150);
                AddLog("DREDGING ERROR: Cargo hold full! Transship minerals to support ship.", th->accentRed);
                break;
            }

            float distToSeabed = max(0.0f, g_sub.seabedElevation - g_sub.depth);
            UnifiedContact contacts[8];
            int cCount = GetUnifiedContacts(g_sub.currentSectorIdx, contacts);
            UnifiedContact* curTgt = (cCount > 0 && g_sub.selectedTargetIdx < cCount) ? &contacts[g_sub.selectedTargetIdx] : NULL;

            int nearSalvage = 0;
            if (curTgt && curTgt->isSalvage) {
                float dx = curTgt->x - g_sub.posX;
                float dy = curTgt->y - g_sub.posY;
                float distM = sqrtf(dx * dx + dy * dy) * 1000.0f;
                float depthDiff = fabsf(g_sub.depth - curTgt->depth);
                if (distM <= 220.0f && depthDiff <= 90.0f) nearSalvage = 1;
            }

            if (!nearSalvage && distToSeabed > 45.0f) {
                PlaySoundAsync(220, 150);
                snprintf(msg, sizeof(msg), "DREDGING CLAW OUT OF REACH: Sub is %.0fm above seabed. Dive closer (<45m)!", distToSeabed);
                AddLog(msg, th->accentAmber);
                break;
            }

            g_sub.isDredging = 1;
            g_sub.dredgeProgress = 0.0f;
            PlayClawServo();
            AddLog("🗜️ Hydraulic dredging claw arm clamping into seabed sediment strata...", th->textPrimary);
            break;
        }

        case ID_BTN_OFFLOAD_CARGO: {
            RecalculateCargo();
            if (g_sub.cargoTotalValue <= 0) {
                PlaySoundAsync(220, 100);
                AddLog("Cargo hold is empty. Dredge nodules or salvage wrecks first.", th->textDim);
            } else {
                int pts = g_sub.cargoTotalValue;
                float offWeight = g_sub.cargoTotalWeight;
                g_sub.surveyPoints += pts;
                g_sub.cargoManganese = 0;
                g_sub.cargoSunkenGold = 0;
                g_sub.cargoTitaniumScrap = 0;
                g_sub.cargoSmokerCrystals = 0;
                g_sub.cargoHadalPrisms = 0;
                RecalculateCargo();
                PlayMineralChime();
                snprintf(msg, sizeof(msg), "🚢 TRANSSHIPMENT COMPLETE: Offloaded %.0f KG of minerals for +%d Research Credits!", offWeight, pts);
                AddLog(msg, th->accentEmerald);
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
                PlayBallastBlowHiss();
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
                        PlayWhaleSong();
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
                PlayBallastBlowHiss();
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
            PlayBallastBlowHiss();
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

        case WM_KEYDOWN: {
            if (wParam >= '1' && wParam <= '4') {
                FireTorpedoTube((int)(wParam - '1'));
                InvalidateRect(hWnd, NULL, FALSE);
            } else if (wParam == 'X' || wParam == 'x') {
                TriggerShockwaveDischarge();
                InvalidateRect(hWnd, NULL, FALSE);
            } else if (wParam == 'C' || wParam == 'c') {
                ToggleSilentRunning();
                InvalidateRect(hWnd, NULL, FALSE);
            } else if (wParam == VK_TAB) {
                g_sub.selectedThreatIdx = (g_sub.selectedThreatIdx + 1) % THREAT_COUNT;
                PlaySoundAsync(650, 60);
                InvalidateRect(hWnd, NULL, FALSE);
            }
            return 0;
        }

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
