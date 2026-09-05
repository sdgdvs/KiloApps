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
#define ID_BTN_UPGRADES    112
#define ID_BTN_EVA         113
#define ID_BTN_CRISIS      114
#define ID_BTN_REFINERY    115
#define ID_BTN_STATION     116
#define ID_BTN_DEFENSE     117

#define SFX_NONE           0
#define SFX_COLLECT        1
#define SFX_FRACTURE       2
#define SFX_OVERHEAT       3
#define SFX_LASER_PULSE    4
#define SFX_BEEP           5
#define SFX_WARP           6
#define SFX_SCAN_SWEEP     7
#define SFX_RESONANCE      8
#define SFX_PLASMA_CUT     9
#define SFX_BREACH         10
#define SFX_DECRYPT        11
#define SFX_CORE_HARVEST   12
#define SFX_ALARM          13
#define SFX_PLASMA_VENT    14
#define SFX_SEAL_WELD      15
#define SFX_DECON_FLUSH    16
#define SFX_SMELT          17
#define SFX_SYNTH          18
#define SFX_DOCK           19
#define SFX_CONTRACT       20
#define SFX_BARTER         21
#define SFX_RAILGUN        22
#define SFX_FLAK_BURST     23
#define SFX_PDL_FIRE       24
#define SFX_PIRATE_EXPLODE 25
#define SFX_TORPEDO_ALERT  26
#define SFX_THRUSTER_BURN  27
#define SFX_LASER_HUM      28
#define SFX_COLLISION_ALARM 29
#define SFX_ROCK_CRUNCH    30
#define SFX_SHIELD_DEFLECT 31
#define SFX_HULL_IMPACT    32
#define SFX_CHAFF          33

// Phase 10: Orbital Refinery Recipes
typedef struct {
    const char* id;
    const char* name;
    const char* desc;
    int inputs[6]; // Ferrum, Silicates, Platinum, Void Quartz, Dark Geode, Derelict Scrap
    const char* inputStr;
    const char* outputName;
    int value;
    COLORREF color;
    int usable; // 0 or 1
    const char* useLabel;
} RefineryRecipeDef;

static const RefineryRecipeDef REFINERY_RECIPES[6] = {
    { "hyperFerrum", "Hyper-Ferrum Ingot (Fe-Ti)", "Smelt common Ferrum Ore with Silicates flux into high-strength alloy.", { 10, 5, 0, 0, 0, 0 }, "10 Ferrum + 5 Silicates", "Hyper-Ferrum Ingot", 350, RGB(148, 163, 184), 0, "" },
    { "superconductor", "Platinum Superconductor Core", "Infuse Platinum into resonant Void Quartz lattice for quantum conductors.", { 0, 0, 8, 4, 0, 0 }, "8 Platinum + 4 Void Quartz", "Superconductor Core", 1200, RGB(226, 232, 240), 0, "" },
    { "warpCells", "Sub-Space Warp Fuel Cell", "Synthesize crystalline void quartz into pressurized warp plasma (+50% Fuel).", { 5, 10, 0, 6, 0, 0 }, "6 Quartz + 10 Silicates + 5 Ferrum", "Warp Fuel Cell", 900, RGB(245, 158, 11), 1, "USE (+50% FUEL)" },
    { "darkMatrix", "Dark Matter Matrix Ingot", "Compress volatile Dark Matter Geodes in a magnetic platinum matrix.", { 0, 0, 6, 0, 4, 0 }, "4 Dark Geodes + 6 Platinum", "Dark Matter Matrix", 2800, RGB(244, 63, 94), 0, "" },
    { "nanitePaste", "Nanite Bulkhead Hull Paste", "Refine derelict scrap alloy and iron into self-replicating nanite weld paste (+40% Hull).", { 6, 0, 0, 0, 0, 8 }, "8 Scrap + 6 Ferrum", "Nanite Repair Paste", 650, RGB(16, 185, 129), 1, "APPLY (+40% HULL)" },
    { "o2Canister", "Pressurized O2 Canister", "Thermal decompose raw silicates to extract medical-grade pressurized oxygen (+45% O2).", { 0, 8, 0, 0, 0, 0 }, "8 Silicates", "O2 Canister", 300, RGB(96, 165, 250), 1, "DISPENSE (+45% O2)" }
};

// Phase 11: Orbital Spaceports & Black Market Trade Stations
typedef struct {
    const char* id;
    const char* title;
    const char* desc;
    int reqItemIdx;
    int reqQty;
    int reqRefined; // 0=ore, 1=refined
    const char* reqLabel;
    int rewardCredits;
    int rewardRep;
} ContractDef;

typedef struct {
    const char* sectorId;
    const char* name;
    const char* type;
    const char* faction;
    const char* desc;
    int repIndex;
    const char* tariffText;
    float multipliers[12];
    int fuelCostPerPct;
    int repairCostPerPct;
    int shieldCost;
    int deconCost;
    ContractDef contracts[3];
} StationDef;

static const StationDef STATION_DEFS[4] = {
    {
        "alpha",
        "Vanguard-Prime Orbital Foundry",
        "Consortium Heavy Industrial Starport",
        "Sol Mining Consortium",
        "Massive rotating ring citadel. Primary export terminal for refined metals and heavy alloy foundries.",
        0,
        "TARIFF: +30% Ferrum, +40% Hyper-Ferrum, -25% Fuel Cost",
        { 1.30f, 1.15f, 1.0f, 1.0f, 1.0f, 1.0f, 1.40f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
        3, 10, 8, 80,
        {
            { "c_alpha_1", "Foundry Ferrum Bulk Quota", "Supply 30T raw Ferrum Ore for orbital blast furnaces.", 0, 30, 0, "30T Ferrum Ore", 1800, 15 },
            { "c_alpha_2", "Silicate Smelting Flux Batch", "Deliver 20T flux silicates to optimize smelting slag.", 1, 20, 0, "20T Silicates", 1400, 12 },
            { "c_alpha_3", "Titanium-Steel Superstructure Order", "Supply 2 Hyper-Ferrum Ingots for shipyard expansion.", 0, 2, 1, "2x Hyper-Ferrum Ingots", 2200, 25 }
        }
    },
    {
        "kuiper",
        "Cryo-Reach Free Waystation",
        "Deep-Ice Prospector Trading Post",
        "Rim Free Traders Guild",
        "Deep space outpost anchored inside a hollow nickel-iron asteroid. Premier market for superconductor metals.",
        1,
        "TARIFF: +35% Platinum Veins, +45% Superconductors, +20% Silicates",
        { 1.0f, 1.20f, 1.35f, 1.10f, 1.0f, 1.0f, 1.0f, 1.45f, 1.0f, 1.0f, 1.0f, 1.0f },
        4, 15, 10, 100,
        {
            { "c_kuiper_1", "Cryo-Platinum Strategic Reserve", "Deliver 20T raw Platinum for quantum processor arrays.", 2, 20, 0, "20T Platinum Vein", 3200, 20 },
            { "c_kuiper_2", "Zero-Resistance Superconductor Run", "Supply 2 Platinum Superconductor Cores for drive arrays.", 1, 2, 1, "2x Superconductor Cores", 4500, 30 },
            { "c_kuiper_3", "Sub-Zero Thermal Insulators", "Supply 25T pure Silicates for cryo-fuel storage tanks.", 1, 25, 0, "25T Silicates", 1700, 15 }
        }
    },
    {
        "graveyard",
        "Shadow-Haven Pirate Freeport",
        "Black Market Outlaw Citadel",
        "Void Corsairs Syndicate",
        "Unregulated outlaw stronghold forged from welded dreadnought wreckage. Lucrative market for military scrap.",
        2,
        "TARIFF: +50% Derelict Scrap, +40% Nanite Paste, +30% Void Quartz",
        { 1.0f, 1.0f, 1.15f, 1.30f, 1.20f, 1.50f, 1.0f, 1.0f, 1.0f, 1.0f, 1.40f, 1.0f },
        5, 20, 12, 120,
        {
            { "c_grave_1", "Corsair Fleet Armor Stripping", "Deliver 35T Derelict Scrap for pirate gunship plating.", 5, 35, 0, "35T Derelict Scrap", 3800, 25 },
            { "c_grave_2", "Covert Nanite Weld Delivery", "Supply 2 Nanite Bulkhead Pastes for emergency raider repairs.", 4, 2, 1, "2x Nanite Repair Paste", 3200, 30 },
            { "c_grave_3", "Resonance Quartz Contraband", "Provide 15T Void Quartz for cloaking emitter frequency tuning.", 3, 15, 0, "15T Void Quartz", 4200, 35 }
        }
    },
    {
        "nebula",
        "Omega-7 Black Lab Citadel",
        "Classified Sub-Space Research Complex",
        "Void Arcane Research Division",
        "Heavy cloaked science fortress hovering in ionized storm clouds. Studies dark matter singularities.",
        3,
        "TARIFF: +60% Dark Geodes, +50% Dark Matter Matrix, +40% Warp Cells",
        { 1.0f, 1.0f, 1.20f, 1.25f, 1.60f, 1.10f, 1.0f, 1.30f, 1.40f, 1.50f, 1.0f, 1.0f },
        6, 25, 15, 150,
        {
            { "c_neb_1", "Singularity Dark Matter Requisition", "Extract 12T volatile Dark Geodes from ionized storm cells.", 4, 12, 0, "12T Dark Geode", 6800, 40 },
            { "c_neb_2", "Exotic Dark Matrix Commission", "Smelt and deliver 2 Dark Matter Matrices for the prototype gateway.", 3, 2, 1, "2x Dark Matter Matrix", 11500, 50 },
            { "c_neb_3", "High-Warp Plasma Cache", "Supply 2 Sub-Space Warp Fuel Cells for deep anomaly probes.", 2, 2, 1, "2x Warp Fuel Cells", 4200, 35 }
        }
    }
};

#define MAX_COMPARTMENTS 5
typedef struct {
    const char* id;
    const char* name;
    const char* desc;
    int x, y, w, h;
} CompartmentDef;

static const CompartmentDef COMPARTMENT_DEFS[MAX_COMPARTMENTS] = {
    { "bridge", "Bridge & Life Support Hub", "Atmosphere scrubbers & avionics", 190, 60, 75, 30 },
    { "prow", "Forward Mining Prow", "Laser core focal array & ram", 280, 60, 65, 30 },
    { "port", "Port Deflector Bulkhead", "Shield generator & thrusters", 120, 25, 85, 25 },
    { "starboard", "Starboard Ore Vault", "Mineral storage & compression", 120, 100, 85, 25 },
    { "reactor", "Antimatter Reactor Core", "Fusion core & plasma conduits", 30, 55, 75, 40 }
};

#define MAX_STARS 150
#define MAX_ASTEROIDS 24
#define MAX_DERELICTS 6
#define MAX_RAIDERS 8
#define MAX_PROJECTILES 32
#define MAX_ENEMY_PROJECTILES 32
#define MAX_SHOCKWAVES 8
#define MAX_PIRATE_WRECKS 8
#define MAX_ORE_CHUNKS 64
#define MAX_PARTICLES 128
#define MAX_FLOATING_TEXTS 16
#define MAX_LOG_ENTRIES 30
#define MAX_SCAN_WAVES 8

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
    
    // Phase 7: Spectrometer Prospecting Data
    int comp[6]; // Ferrum %, Silicates %, Platinum %, Void Quartz %, Dark Geode %, Derelict Scrap %
    int geodeType; // 0=None, 1=Void Quartz Geode, 2=Dark Matter Geode, 3=Platinum Pocket
    int purity; // 1 to 5 stars
    int volatility; // %
    float freqMhz;
    int scanned; // 0=unscanned, 1=scanned
    int resonantLock; // 0 or 1
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
    float x, y;
    float r, maxR;
    COLORREF color;
    float life;
    int active;
} ScanWave;

typedef struct {
    char text[32];
    float x, y;
    float vy;
    COLORREF color;
    float life;
    int active;
} FloatingText;

// Phase 12: Raider & Weapon System Structs
typedef struct {
    int type; // 0=railgun, 1=flakShell, 2=pdlBeam
    float x, y;
    float vx, vy;
    float x2, y2;
    float life;
    float damage;
    COLORREF color;
    int active;
} Projectile;

typedef struct {
    int type; // 0=blaster, 1=torpedo
    float x, y;
    float vx, vy;
    int homing;
    float hp;
    float life;
    float damage;
    COLORREF color;
    int active;
} EnemyProjectile;

typedef struct {
    float x, y;
    float r, maxR;
    COLORREF color;
    float life;
    int active;
} Shockwave;

typedef struct {
    float x, y;
    float angle;
    int type;
    float life;
    int active;
} PirateWreck;

typedef struct {
    char id[16];
    char name[32];
    int type; // 0=corsair, 1=gunship, 2=dread
    float x, y;
    float vx, vy;
    float angle;
    float hp, maxHp;
    float shield, maxShield;
    float speed;
    int bounty;
    float radius;
    float stunTimer;
    float shootTimer;
    COLORREF color;
    int active;
} Raider;

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

// Modular Upgrade Definitions
typedef struct {
    int level;
    const char* name;
    int cost;
    float dpsBonus;
    float heatRate;
    float chunkBonus;
    const char* desc;
} DrillUpgradeDef;

typedef struct {
    int level;
    const char* name;
    int cost;
    int capacity;
    const char* desc;
} CargoUpgradeDef;

typedef struct {
    int level;
    const char* name;
    int cost;
    float thrust;
    float maxSpeed;
    float rot;
    float fuelBurn;
    const char* desc;
} EngineUpgradeDef;

typedef struct {
    int level;
    const char* name;
    int cost;
    float maxShield;
    float maxHull;
    float regen;
    const char* desc;
} ShieldUpgradeDef;

static const DrillUpgradeDef DRILL_UPGRADES[5] = {
    { 1, "Mk-I Pulse Beam",          0,     1.0f,  1.0f,  0.12f, "Stock pulse laser emitter." },
    { 2, "Mk-II Focused Lance",      800,   1.35f, 0.85f, 0.16f, "+35% DPS, -15% heat, +33% ore." },
    { 3, "Mk-III Heavy Plasma Drill",2400,  1.80f, 0.70f, 0.22f, "+80% DPS, -30% heat, +83% ore." },
    { 4, "Mk-IV Singularity Bore",   6000,  2.40f, 0.55f, 0.30f, "+140% DPS, -45% heat, +150% ore." },
    { 5, "Mk-V Void Annihilator",    14000, 3.20f, 0.40f, 0.40f, "+220% DPS, -60% heat, +233% ore." }
};

static const CargoUpgradeDef CARGO_UPGRADES[5] = {
    { 1, "Mk-I Standard Hold",       0,     200,  "200 T base capacity." },
    { 2, "Mk-II Reinforced Vault",   650,   350,  "350 T capacity (+150T)." },
    { 3, "Mk-III Titan Cargo Bay",   1800,  550,  "550 T capacity (+350T)." },
    { 4, "Mk-IV Super-Freighter",    4500,  850,  "850 T capacity (+650T)." },
    { 5, "Mk-V Dreadnought Vault",   11000, 1300, "1,300 T capacity (+1,100T)." }
};

static const EngineUpgradeDef ENGINE_UPGRADES[5] = {
    { 1, "Mk-I Chemical Jets",       0,     0.12f, 6.0f,  0.045f, 1.0f,  "Stock hydrazine thrusters." },
    { 2, "Mk-II Ion Pulse Drive",    750,   0.16f, 7.2f,  0.052f, 0.80f, "+33% Thrust, +20% Spd, -20% Burn." },
    { 3, "Mk-III Fusion Thrusters",  2200,  0.21f, 8.5f,  0.060f, 0.65f, "+75% Thrust, +42% Spd, -35% Burn." },
    { 4, "Mk-IV Antimatter Drive",   5500,  0.27f, 10.0f, 0.068f, 0.50f, "+125% Thrust, +67% Spd, -50% Burn." },
    { 5, "Mk-V Quantum Gravity Drive",13000,0.35f, 12.0f, 0.078f, 0.35f, "+190% Thrust, +100% Spd, -65% Burn." }
};

static const ShieldUpgradeDef SHIELD_UPGRADES[5] = {
    { 1, "Mk-I Deflector Mesh",      0,     100.0f, 100.0f, 0.05f, "100 Shield / 100 Hull plating." },
    { 2, "Mk-II Particle Shield",    700,   150.0f, 125.0f, 0.08f, "150 Shield / 125 Hull, +60% regen." },
    { 3, "Mk-III Kinetic Barrier",   2000,  220.0f, 160.0f, 0.12f, "220 Shield / 160 Hull, +140% regen." },
    { 4, "Mk-IV Aegis Energy Matrix",5000,  300.0f, 200.0f, 0.18f, "300 Shield / 200 Hull, +260% regen." },
    { 5, "Mk-V Void Phase Barrier",  12000, 420.0f, 260.0f, 0.25f, "420 Shield / 260 Hull, +400% regen." }
};

// Derelict Spaceship Salvage Definitions (Phase 8)
typedef struct {
    const char* name;
    const char* classType;
    int scrapMin, scrapMax;
    int dataValue;
    int coreValue;
    int podsMin, podsMax;
    const char* logArchive;
    COLORREF hullColor;
} DerelictTemplate;

static const DerelictTemplate DERELICT_TEMPLATES[4] = {
    {
        "USN Ghost Frigate \"Hyperion\"",
        "Combat Frigate Wreck",
        6, 12,
        450,
        1200,
        3, 6,
        "LOG ENTRY #402: Emergency bulkheads collapsed during quantum cascade. Sub-light drives seized. Reactor core remains in containment breach status.",
        RGB(239, 68, 68)
    },
    {
        "Bulk Transport \"Valkyrie-7\"",
        "Heavy Ore Hauler",
        10, 18,
        300,
        800,
        5, 9,
        "LOG ENTRY #891: Navigation gyro failure inside asteroid dense zone. Cargo hold compromised. Emergency beacon active for 41 standard cycles.",
        RGB(245, 158, 11)
    },
    {
        "Science Cruiser \"Aegis Prime\"",
        "Deep Void Survey Vessel",
        4, 8,
        850,
        1800,
        2, 4,
        "LOG ENTRY #114: Dark matter anomaly breached research lab containment. Antimatter core oscillating at resonant harmonic. Scramble all shuttle bays.",
        RGB(192, 132, 252)
    },
    {
        "Stealth Corvette \"Void Viper\"",
        "Covert Operative Vessel",
        5, 10,
        600,
        1500,
        2, 5,
        "CLASSIFIED TRANSMISSION: Cloaking field power surge ionized main conduits. Systems offline. Black box contains encrypted telemetry coordinates.",
        RGB(56, 189, 248)
    }
};

typedef struct {
    char id[16];
    char name[48];
    char classType[32];
    float x, y;
    float vx, vy;
    float rot, rotSpeed;
    float length, width;
    COLORREF hullColor;
    
    // EVA Salvage Operations
    int airlockBreached; // 0=Sealed (Plasma Cut), 1=Breached
    int airlockCutProgress; // 0 to 100%
    int blackBoxDecrypted; // 0=Encrypted, 1=Decrypted
    int reactorHarvested; // 0=Unstable Core, 1=Safely Contained & Harvested
    int cargoScavenged; // 0=Unscavenged, 1=Looted
    int scrapPods;
    int dataValue;
    int coreValue;
    char logArchive[256];
    
    float hp, maxHp;
    int active;
} Derelict;

// Game State
typedef struct {
    int credits;
    int currentSectorIndex;
    int selectedSectorIndex;
    int showStarChart;
    int showUpgrades;
    int showSpectrometer;
    int showEva;
    int showCrisis;
    int showRefinery;
    int showStation;
    int showHelp;
    int helpTab; // 0=Flight & Controls, 1=Sector Charts, 2=Mineral Codex, 3=Refinery Recipes, 4=Crisis & Defense
    int catalyticBoost;
    int refined[6]; // hyperFerrum, superconductor, warpCells, darkMatrix, nanitePaste, o2Canister
    float crucibleAnimTime;
    
    // Orbital Station & Spaceport State
    float stationX, stationY;
    float stationRadius;
    float stationAngle;
    float stationRotSpeed;
    int stationDocked;
    float stationBeaconTimer;
    int factionRep[4]; // 0=consortium, 1=freeTraders, 2=corsairs, 3=arcane
    int contractDone[4][3];
    int totalContractsDone;

    int warpActive;
    float warpTimer;
    char sector[32];
    float hull, maxHull;
    float shield, maxShield;
    float fuel, maxFuel;
    float heat, maxHeat;
    float reactor;
    float o2;
    float o2Scrubber;
    float radiation;
    int hullBreaches[MAX_COMPARTMENTS];
    int breachCount;
    int plasmaLeaks;
    int repairDronesActive;
    float droneTimer;
    float alarmTick;
    int cargoHold[6];
    int totalCargo;
    int maxCargo;
    int dampeners;
    int laserOverheated;
    int themeIndex;
    int scanlineMode; // 0=Off, 1=Normal, 2=CRT+
    
    // Modular Upgrades (Tier 0 to 4)
    int upgradeDrill;
    int upgradeCargo;
    int upgradeEngine;
    int upgradeShield;
    
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
    int selectedDerelictIndex;
    float radarAngle;
    int soundEnabled;
    int showDefense;
    int selectedWeapon; // 0=Laser, 1=Railgun, 2=EMP Flak, 3=PDL
    int railgunSlugs;
    int empCharges;
    int autoPDL;
    float railgunCooldown;
    float flakCooldown;
    float pdlCooldown;
    float chaffCooldown;
    int piratesDefeated;
    int bountiesClaimed;

    Star stars[MAX_STARS];
    Asteroid asteroids[MAX_ASTEROIDS];
    Derelict derelicts[MAX_DERELICTS];
    Raider raiders[MAX_RAIDERS];
    Projectile projectiles[MAX_PROJECTILES];
    EnemyProjectile enemyProjectiles[MAX_ENEMY_PROJECTILES];
    Shockwave shockwaves[MAX_SHOCKWAVES];
    PirateWreck pirateWrecks[MAX_PIRATE_WRECKS];
    OreChunk oreChunks[MAX_ORE_CHUNKS];
    Particle particles[MAX_PARTICLES];
    ScanWave scanWaves[MAX_SCAN_WAVES];
    FloatingText texts[MAX_FLOATING_TEXTS];
    LogEntry logs[MAX_LOG_ENTRIES];
    int logCount;
} GameState;

static GameState g_state;
static HWND g_hwnd = NULL;
static HWND g_btnLaser, g_btnTractor, g_btnDampener, g_btnScan, g_btnNav, g_btnStation, g_btnEva, g_btnCrisis, g_btnDefense, g_btnRefinery, g_btnUpgrades, g_btnTheme, g_btnScanlines, g_btnAudio, g_btnHelp, g_btnJettison, g_btnSell;
static HFONT g_fontMono = NULL;
static HFONT g_fontMonoBold = NULL;
static HFONT g_fontSmall = NULL;
static HFONT g_fontHeader = NULL;

// Audio System (Phase 13 Deep Expansion)
#define MAX_SFX_QUEUE 64
static volatile int g_sfxQueue[MAX_SFX_QUEUE];
static volatile int g_sfxHead = 0;
static volatile int g_sfxTail = 0;
static CRITICAL_SECTION g_csSound;
static volatile int g_soundThreadRunning = 1;
static HANDLE g_hSoundThread = NULL;

void TriggerSound(int sfx) {
    if (!g_state.soundEnabled || sfx == SFX_NONE) return;
    EnterCriticalSection(&g_csSound);
    int nextHead = (g_sfxHead + 1) % MAX_SFX_QUEUE;
    if (nextHead != g_sfxTail) {
        g_sfxQueue[g_sfxHead] = sfx;
        g_sfxHead = nextHead;
    }
    LeaveCriticalSection(&g_csSound);
}

DWORD WINAPI SoundThreadProc(LPVOID lpParam) {
    while (g_soundThreadRunning) {
        int sfx = SFX_NONE;
        EnterCriticalSection(&g_csSound);
        if (g_sfxHead != g_sfxTail) {
            sfx = g_sfxQueue[g_sfxTail];
            g_sfxTail = (g_sfxTail + 1) % MAX_SFX_QUEUE;
        }
        LeaveCriticalSection(&g_csSound);

        if (sfx != SFX_NONE && g_state.soundEnabled) {
            switch (sfx) {
                case SFX_COLLECT:
                    Beep(600, 35);
                    Beep(880, 50);
                    break;
                case SFX_FRACTURE:
                case SFX_ROCK_CRUNCH:
                    Beep(180, 60);
                    Beep(110, 80);
                    Beep(240, 40);
                    break;
                case SFX_OVERHEAT:
                    Beep(320, 90);
                    Beep(240, 110);
                    break;
                case SFX_BEEP:
                    Beep(700, 45);
                    break;
                case SFX_LASER_PULSE:
                case SFX_LASER_HUM:
                    Beep(480, 25);
                    Beep(380, 25);
                    break;
                case SFX_THRUSTER_BURN:
                    Beep(75, 40);
                    Beep(60, 40);
                    break;
                case SFX_WARP:
                    Beep(160, 50);
                    Beep(420, 70);
                    Beep(920, 90);
                    Beep(240, 70);
                    break;
                case SFX_SCAN_SWEEP:
                    Beep(450, 45);
                    Beep(750, 60);
                    Beep(1150, 80);
                    break;
                case SFX_RESONANCE:
                    Beep(440, 50);
                    Beep(660, 50);
                    Beep(880, 100);
                    break;
                case SFX_PLASMA_CUT:
                    Beep(750, 35);
                    Beep(920, 25);
                    break;
                case SFX_BREACH:
                    Beep(220, 60);
                    Beep(340, 70);
                    Beep(680, 90);
                    break;
                case SFX_DECRYPT:
                    Beep(880, 35);
                    Beep(1100, 35);
                    Beep(1320, 50);
                    break;
                case SFX_CORE_HARVEST:
                    Beep(330, 60);
                    Beep(550, 60);
                    Beep(770, 80);
                    Beep(1100, 120);
                    break;
                case SFX_ALARM:
                case SFX_COLLISION_ALARM:
                    Beep(880, 70);
                    Beep(440, 70);
                    Beep(880, 70);
                    break;
                case SFX_PLASMA_VENT:
                    Beep(480, 60);
                    Beep(240, 80);
                    Beep(120, 90);
                    break;
                case SFX_SEAL_WELD:
                    Beep(350, 50);
                    Beep(700, 65);
                    break;
                case SFX_DECON_FLUSH:
                    Beep(520, 40);
                    Beep(650, 40);
                    Beep(780, 65);
                    break;
                case SFX_SMELT:
                    Beep(320, 50);
                    Beep(640, 65);
                    Beep(480, 80);
                    break;
                case SFX_SYNTH:
                    Beep(440, 35);
                    Beep(554, 35);
                    Beep(659, 35);
                    Beep(880, 70);
                    break;
                case SFX_DOCK:
                    Beep(440, 60);
                    Beep(660, 70);
                    Beep(880, 120);
                    break;
                case SFX_CONTRACT:
                    Beep(523, 50);
                    Beep(659, 50);
                    Beep(784, 50);
                    Beep(1046, 100);
                    break;
                case SFX_BARTER:
                    Beep(880, 45);
                    Beep(1174, 70);
                    break;
                case SFX_RAILGUN:
                    Beep(1400, 25);
                    Beep(80, 70);
                    Beep(880, 35);
                    break;
                case SFX_FLAK_BURST:
                    Beep(220, 70);
                    Beep(140, 90);
                    Beep(70, 110);
                    break;
                case SFX_PDL_FIRE:
                    Beep(1500, 25);
                    break;
                case SFX_PIRATE_EXPLODE:
                    Beep(240, 70);
                    Beep(120, 110);
                    Beep(60, 160);
                    break;
                case SFX_TORPEDO_ALERT:
                    Beep(900, 40);
                    Beep(600, 40);
                    break;
                case SFX_SHIELD_DEFLECT:
                    Beep(750, 35);
                    Beep(950, 35);
                    break;
                case SFX_HULL_IMPACT:
                    Beep(110, 70);
                    Beep(70, 90);
                    break;
                case SFX_CHAFF:
                    Beep(1400, 30);
                    Beep(800, 45);
                    Beep(400, 45);
                    break;
            }
        }
        Sleep(15);
    }
    return 0;
}

// Forward Declarations
void AddLog(const char* text, int type);
void AddFloatingText(const char* text, float x, float y, COLORREF color);
void AddSparks(float x, float y, COLORREF color, int count);
void AddScanWave(float x, float y, float maxR, COLORREF color);
void SpawnOreChunk(float x, float y, int oreType, int amount);
void SpawnAsteroid(int index, int oreType);
void SpawnDerelict(int index, int templateIdx);
void SpawnRaider(int index, int type);
void FireRailgun(void);
void FireEMPFlak(void);
void TriggerEMPShockwave(float x, float y);
void FirePDL(void);
void DeployChaff(void);
void CraftRailgunSlugs(void);
void RechargeEMPCapacitor(void);
void OverchargeShield(void);
void DeployCombatDrones(void);
void BreachAirlock(int index);
void DecryptBlackBox(int index);
void HarvestReactorCore(int index);
void ScavengeCargoPods(int index);
int PickOreForSector(int sectorIdx);
void InitSectorField(int sectorIdx);
void EngageWarpJump(int targetSectorIdx);
void ScanTargetAsteroid(int index);
void ScanAllWideBand(void);
void TuneLaserResonance(int index);
void InitGame(void);
void BuyUpgrade(int moduleIdx);
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

void AddScanWave(float x, float y, float maxR, COLORREF color) {
    for (int i = 0; i < MAX_SCAN_WAVES; i++) {
        if (!g_state.scanWaves[i].active) {
            g_state.scanWaves[i].x = x;
            g_state.scanWaves[i].y = y;
            g_state.scanWaves[i].r = 10.0f;
            g_state.scanWaves[i].maxR = maxR;
            g_state.scanWaves[i].color = color;
            g_state.scanWaves[i].life = 1.0f;
            g_state.scanWaves[i].active = 1;
            break;
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
    
    // Multi-spectral mineral assay composition
    for (int i = 0; i < 6; i++) ast->comp[i] = 0;
    ast->comp[ast->oreType] = 55 + (rand() % 30);
    int remPct = 100 - ast->comp[ast->oreType];
    
    for (int i = 0; i < 6; i++) {
        if (i == ast->oreType) continue;
        int take = rand() % (remPct + 1);
        if (take > remPct / 2) take = remPct / 2;
        ast->comp[i] = take;
        remPct -= take;
    }
    ast->comp[ast->oreType] += remPct; // distribute remainder
    
    // Geode Core Anomaly Generation
    ast->geodeType = 0;
    ast->purity = 1 + (rand() % 5);
    ast->volatility = 5 + (rand() % 35);
    ast->freqMhz = 120.0f + (((float)rand() / (float)RAND_MAX) * 760.0f);
    ast->scanned = 0;
    ast->resonantLock = 0;
    
    int roll = rand() % 100;
    if (g_state.currentSectorIndex == 3 || roll < 22) { // Dark Geode
        ast->geodeType = 2; // Dark Matter Geode
        if (ast->comp[4] < 25) ast->comp[4] = 25;
    } else if (g_state.currentSectorIndex == 2 || roll < 45) { // Graveyard or Quartz
        ast->geodeType = 1; // Resonant Quartz Geode
        if (ast->comp[3] < 30) ast->comp[3] = 30;
    } else if (roll < 65) {
        ast->geodeType = 3; // Platinum Pocket
        if (ast->comp[2] < 35) ast->comp[2] = 35;
    }
    
    ast->numVerts = 8 + (rand() % 4);
    for (int v = 0; v < ast->numVerts; v++) {
        ast->verts[v].a = ((float)v / (float)ast->numVerts) * 6.28318f;
        ast->verts[v].r = ast->radius * (0.75f + (((float)rand() / (float)RAND_MAX) * 0.35f));
    }
}

void ScanTargetAsteroid(int index) {
    if (index < 0 || index >= MAX_ASTEROIDS || !g_state.asteroids[index].active) {
        AddLog("No asteroid locked in target reticle to prospect.", 3);
        return;
    }
    Asteroid* ast = &g_state.asteroids[index];
    ast->scanned = 1;
    TriggerSound(SFX_SCAN_SWEEP);
    
    float dist = (float)sqrt((ast->x - g_state.shipX) * (ast->x - g_state.shipX) +
                             (ast->y - g_state.shipY) * (ast->y - g_state.shipY));
    AddScanWave(g_state.shipX, g_state.shipY, dist + 60.0f, RGB(0, 240, 255));
    
    char buf[128];
    sprintf(buf, "Multi-spectral assay complete: %s [%s %d%%, Volatility: %d%%]",
            ast->id, ORE_DEFS[ast->oreType].name, ast->comp[ast->oreType], ast->volatility);
    AddLog(buf, 5);
    
    if (ast->geodeType == 2) {
        AddLog("GEODE ANOMALY: Dark Matter Geode core detected!", 2);
        AddFloatingText("GEODE: Dark Matter", ast->x, ast->y - 30.0f, RGB(244, 63, 94));
    } else if (ast->geodeType == 1) {
        AddLog("GEODE ANOMALY: Resonant Void Quartz crystal detected!", 2);
        AddFloatingText("GEODE: Void Quartz", ast->x, ast->y - 30.0f, RGB(192, 132, 252));
    } else {
        char fTxt[32];
        sprintf(fTxt, "ANALYZED: %s", ast->id);
        AddFloatingText(fTxt, ast->x, ast->y - 25.0f, RGB(56, 189, 248));
    }
}

void ScanAllWideBand(void) {
    TriggerSound(SFX_SCAN_SWEEP);
    AddScanWave(g_state.shipX, g_state.shipY, 950.0f, RGB(16, 185, 129));
    
    int totalScanned = 0;
    int geodesFound = 0;
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (g_state.asteroids[i].active) {
            g_state.asteroids[i].scanned = 1;
            totalScanned++;
            if (g_state.asteroids[i].geodeType > 0) geodesFound++;
        }
    }
    
    char buf[128];
    sprintf(buf, "Wide-Band Sweep Complete: %d asteroids mapped with spectrometer! (%d geodes found)", totalScanned, geodesFound);
    AddLog(buf, 5);
    AddFloatingText("WIDE-BAND SWEEP COMPLETE", g_state.shipX, g_state.shipY - 40.0f, RGB(16, 185, 129));
}

void TuneLaserResonance(int index) {
    if (index < 0 || index >= MAX_ASTEROIDS || !g_state.asteroids[index].active) {
        AddLog("Select an analyzed asteroid to tune laser harmonics.", 3);
        return;
    }
    Asteroid* ast = &g_state.asteroids[index];
    if (!ast->scanned) {
        ScanTargetAsteroid(index);
    }
    ast->resonantLock = 1;
    TriggerSound(SFX_RESONANCE);
    
    char buf[128];
    sprintf(buf, "Laser optics tuned to %.1f MHz! +50%% Mining Yield active on %s!", ast->freqMhz, ast->id);
    AddLog(buf, 1);
    AddFloatingText("RESONANCE LOCKED (+50%)", g_state.shipX, g_state.shipY - 35.0f, RGB(245, 158, 11));
}

void SpawnRaider(int index, int type) {
    if (index < 0 || index >= MAX_RAIDERS) return;
    Raider* r = &g_state.raiders[index];
    r->type = type;
    r->active = 1;
    r->stunTimer = 0.0f;
    r->shootTimer = 1.0f + (((float)rand() / (float)RAND_MAX) * 2.0f);

    float angle = ((float)rand() / (float)RAND_MAX) * 6.28318f;
    float dist = 420.0f + (((float)rand() / (float)RAND_MAX) * 380.0f);
    r->x = g_state.shipX + (float)cos(angle) * dist;
    r->y = g_state.shipY + (float)sin(angle) * dist;
    r->vx = 0.0f;
    r->vy = 0.0f;
    r->angle = (float)atan2(g_state.shipY - r->y, g_state.shipX - r->x);

    if (type == 0) { // Corsair Skiff
        sprintf(r->id, "RAID-%03d", 100 + index * 13 + (rand() % 50));
        strncpy(r->name, "Corsair Skiff", 31);
        r->maxHp = 60.0f;
        r->hp = 60.0f;
        r->maxShield = 40.0f;
        r->shield = 40.0f;
        r->speed = 6.2f;
        r->bounty = 250;
        r->radius = 16.0f;
        r->color = RGB(239, 68, 68);
    } else if (type == 1) { // Marauder Gunship
        sprintf(r->id, "GUN-%03d", 200 + index * 17 + (rand() % 50));
        strncpy(r->name, "Marauder Gunship", 31);
        r->maxHp = 140.0f;
        r->hp = 140.0f;
        r->maxShield = 90.0f;
        r->shield = 90.0f;
        r->speed = 4.2f;
        r->bounty = 600;
        r->radius = 22.0f;
        r->color = RGB(245, 158, 11);
    } else { // Void Dread Raider
        sprintf(r->id, "DREAD-%03d", 300 + index * 19 + (rand() % 50));
        strncpy(r->name, "Void Dread Raider", 31);
        r->maxHp = 320.0f;
        r->hp = 320.0f;
        r->maxShield = 220.0f;
        r->shield = 220.0f;
        r->speed = 2.8f;
        r->bounty = 1500;
        r->radius = 32.0f;
        r->color = RGB(244, 63, 94);
    }
}

void FireRailgun(void) {
    if (g_state.railgunCooldown > 0.0f) return;
    if (g_state.railgunSlugs <= 0) {
        AddLog("WARNING: Kinetic Railgun dry! Smelt slugs from Ferrum in Armory [X].", 3);
        TriggerSound(SFX_BEEP);
        return;
    }

    g_state.railgunSlugs--;
    g_state.railgunCooldown = 0.9f;
    float slugSpeed = 16.0f;
    float lx = g_state.shipX + (float)cos(g_state.shipAngle) * 22.0f;
    float ly = g_state.shipY + (float)sin(g_state.shipAngle) * 22.0f;

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!g_state.projectiles[i].active) {
            g_state.projectiles[i].type = 0; // railgun
            g_state.projectiles[i].x = lx;
            g_state.projectiles[i].y = ly;
            g_state.projectiles[i].vx = (float)cos(g_state.shipAngle) * slugSpeed;
            g_state.projectiles[i].vy = (float)sin(g_state.shipAngle) * slugSpeed;
            g_state.projectiles[i].life = 1.2f;
            g_state.projectiles[i].damage = 85.0f;
            g_state.projectiles[i].color = RGB(56, 189, 248);
            g_state.projectiles[i].active = 1;
            break;
        }
    }

    TriggerSound(SFX_RAILGUN);
    AddSparks(lx, ly, RGB(0, 240, 255), 8);
    AddLog("KINETIC RAILGUN FIRED: Hypervelocity slug discharged.", 4);
}

void FireEMPFlak(void) {
    if (g_state.flakCooldown > 0.0f) return;
    if (g_state.empCharges <= 0) {
        AddLog("WARNING: EMP Flak capacitors depleted! Recharge from Silicates in Armory [X].", 3);
        TriggerSound(SFX_BEEP);
        return;
    }

    g_state.empCharges--;
    g_state.flakCooldown = 2.5f;
    float shellSpeed = 7.5f;
    float lx = g_state.shipX + (float)cos(g_state.shipAngle) * 22.0f;
    float ly = g_state.shipY + (float)sin(g_state.shipAngle) * 22.0f;

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!g_state.projectiles[i].active) {
            g_state.projectiles[i].type = 1; // flakShell
            g_state.projectiles[i].x = lx;
            g_state.projectiles[i].y = ly;
            g_state.projectiles[i].vx = (float)cos(g_state.shipAngle) * shellSpeed;
            g_state.projectiles[i].vy = (float)sin(g_state.shipAngle) * shellSpeed;
            g_state.projectiles[i].life = 0.45f;
            g_state.projectiles[i].color = RGB(192, 132, 252);
            g_state.projectiles[i].active = 1;
            break;
        }
    }

    TriggerSound(SFX_FLAK_BURST);
    AddLog("EMP FLAK SHELL LAUNCHED: Proximity detonator armed.", 3);
}

void TriggerEMPShockwave(float x, float y) {
    TriggerSound(SFX_FLAK_BURST);
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (!g_state.shockwaves[i].active) {
            g_state.shockwaves[i].x = x;
            g_state.shockwaves[i].y = y;
            g_state.shockwaves[i].r = 10.0f;
            g_state.shockwaves[i].maxR = 240.0f;
            g_state.shockwaves[i].life = 1.0f;
            g_state.shockwaves[i].color = RGB(192, 132, 252);
            g_state.shockwaves[i].active = 1;
            break;
        }
    }
    AddSparks(x, y, RGB(192, 132, 252), 20);

    // Destroy all enemy torpedoes in blast radius
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        if (!g_state.enemyProjectiles[i].active) continue;
        float d = (float)sqrt((g_state.enemyProjectiles[i].x - x) * (g_state.enemyProjectiles[i].x - x) +
                              (g_state.enemyProjectiles[i].y - y) * (g_state.enemyProjectiles[i].y - y));
        if (d < 240.0f) {
            AddSparks(g_state.enemyProjectiles[i].x, g_state.enemyProjectiles[i].y, RGB(245, 158, 11), 12);
            g_state.enemyProjectiles[i].active = 0;
            AddFloatingText("TORPEDO NEUTRALIZED!", g_state.enemyProjectiles[i].x, g_state.enemyProjectiles[i].y - 15.0f, RGB(192, 132, 252));
        }
    }

    // Damage and stun raiders in blast radius
    for (int i = 0; i < MAX_RAIDERS; i++) {
        Raider* r = &g_state.raiders[i];
        if (!r->active) continue;
        float d = (float)sqrt((r->x - x) * (r->x - x) + (r->y - y) * (r->y - y));
        if (d < 240.0f) {
            r->shield = max(0.0f, r->shield - 140.0f);
            r->stunTimer = 4.0f;
            AddSparks(r->x, r->y, RGB(192, 132, 252), 15);
            AddFloatingText("EMP STUNNED (4s)!", r->x, r->y - 25.0f, RGB(192, 132, 252));
            char bLog[64];
            sprintf(bLog, "EMP BLAST: Systems offline on %s!", r->name);
            AddLog(bLog, 5);
        }
    }
}

void FirePDL(void) {
    if (g_state.pdlCooldown > 0.0f) return;
    g_state.pdlCooldown = 0.22f;

    // Target closest hostile torpedo or raider
    float minTorpDist = 280.0f;
    int targetTorpIdx = -1;
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        if (!g_state.enemyProjectiles[i].active || g_state.enemyProjectiles[i].type != 1) continue;
        float d = (float)sqrt((g_state.enemyProjectiles[i].x - g_state.shipX) * (g_state.enemyProjectiles[i].x - g_state.shipX) +
                              (g_state.enemyProjectiles[i].y - g_state.shipY) * (g_state.enemyProjectiles[i].y - g_state.shipY));
        if (d < minTorpDist) { minTorpDist = d; targetTorpIdx = i; }
    }

    float tgtX = 0, tgtY = 0;
    int hitType = 0; // 1=torpedo, 2=raider

    if (targetTorpIdx >= 0) {
        tgtX = g_state.enemyProjectiles[targetTorpIdx].x;
        tgtY = g_state.enemyProjectiles[targetTorpIdx].y;
        hitType = 1;
    } else {
        float minRaiderDist = 260.0f;
        int targetRIdx = -1;
        for (int i = 0; i < MAX_RAIDERS; i++) {
            if (!g_state.raiders[i].active) continue;
            float d = (float)sqrt((g_state.raiders[i].x - g_state.shipX) * (g_state.raiders[i].x - g_state.shipX) +
                                  (g_state.raiders[i].y - g_state.shipY) * (g_state.raiders[i].y - g_state.shipY));
            if (d < minRaiderDist) { minRaiderDist = d; targetRIdx = i; }
        }
        if (targetRIdx >= 0) {
            tgtX = g_state.raiders[targetRIdx].x;
            tgtY = g_state.raiders[targetRIdx].y;
            hitType = 2;
        }
    }

    if (hitType > 0) {
        TriggerSound(SFX_PDL_FIRE);
        AddSparks(tgtX, tgtY, RGB(16, 185, 129), 4);

        for (int i = 0; i < MAX_PROJECTILES; i++) {
            if (!g_state.projectiles[i].active) {
                g_state.projectiles[i].type = 2; // pdlBeam
                g_state.projectiles[i].x = g_state.shipX;
                g_state.projectiles[i].y = g_state.shipY;
                g_state.projectiles[i].x2 = tgtX;
                g_state.projectiles[i].y2 = tgtY;
                g_state.projectiles[i].life = 0.12f;
                g_state.projectiles[i].color = RGB(16, 185, 129);
                g_state.projectiles[i].active = 1;
                break;
            }
        }

        if (hitType == 1 && targetTorpIdx >= 0) {
            g_state.enemyProjectiles[targetTorpIdx].hp -= 20.0f;
            if (g_state.enemyProjectiles[targetTorpIdx].hp <= 0.0f) {
                AddSparks(tgtX, tgtY, RGB(245, 158, 11), 14);
                g_state.enemyProjectiles[targetTorpIdx].active = 0;
                AddFloatingText("TORPEDO INTERCEPTED!", tgtX, tgtY - 15.0f, RGB(16, 185, 129));
                AddLog("PDL INTERCEPTION: Incoming raider torpedo destroyed.", 5);
            }
        } else if (hitType == 2) {
            for (int i = 0; i < MAX_RAIDERS; i++) {
                if (g_state.raiders[i].active && fabs(g_state.raiders[i].x - tgtX) < 1.0f && fabs(g_state.raiders[i].y - tgtY) < 1.0f) {
                    if (g_state.raiders[i].shield > 0.0f) g_state.raiders[i].shield = max(0.0f, g_state.raiders[i].shield - 15.0f);
                    else g_state.raiders[i].hp = max(0.0f, g_state.raiders[i].hp - 15.0f);
                    AddFloatingText("-15", tgtX, tgtY - 15.0f, RGB(16, 185, 129));
                    break;
                }
            }
        }
    }
}

void DeployChaff(void) {
    if (g_state.chaffCooldown > 0.0f) return;
    g_state.chaffCooldown = 8.0f;
    TriggerSound(SFX_CHAFF);

    for (int i = 0; i < 30; i++) {
        float a = ((float)rand() / (float)RAND_MAX) * 6.28318f;
        float s = 1.5f + (((float)rand() / (float)RAND_MAX) * 3.5f);
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (!g_state.particles[p].active) {
                g_state.particles[p].x = g_state.shipX;
                g_state.particles[p].y = g_state.shipY;
                g_state.particles[p].vx = (float)cos(a) * s;
                g_state.particles[p].vy = (float)sin(a) * s;
                g_state.particles[p].color = RGB(254, 240, 138);
                g_state.particles[p].life = 1.5f;
                g_state.particles[p].decay = 0.02f;
                g_state.particles[p].size = 2.5f;
                g_state.particles[p].active = 1;
                break;
            }
        }
    }

    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        if (g_state.enemyProjectiles[i].active && g_state.enemyProjectiles[i].type == 1) {
            g_state.enemyProjectiles[i].homing = 0;
            g_state.enemyProjectiles[i].vx += (((float)rand() / (float)RAND_MAX) - 0.5f) * 4.0f;
            g_state.enemyProjectiles[i].vy += (((float)rand() / (float)RAND_MAX) - 0.5f) * 4.0f;
        }
    }

    AddFloatingText("CHAFF DEPLOYED - LOCKS SCRAMBLED!", g_state.shipX, g_state.shipY - 30.0f, RGB(254, 240, 138));
    AddLog("EMERGENCY CHAFF CLOUD: All incoming torpedo tracking scrambled.", 5);
}

void CraftRailgunSlugs(void) {
    if (g_state.cargoHold[0] < 5) {
        AddLog("WARNING: Insufficient Ferrum Ore! Need 5T Ferrum to smelt 5 slugs.", 3);
        return;
    }
    g_state.cargoHold[0] -= 5;
    g_state.railgunSlugs += 5;
    UpdateCargoTotal();
    TriggerSound(SFX_SYNTH);
    AddLog("Manufactured +5 Heavy Railgun Tungsten-Ferrum Slugs.", 5);
    AddFloatingText("+5 SLUGS", g_state.shipX, g_state.shipY - 30.0f, RGB(56, 189, 248));
}

void RechargeEMPCapacitor(void) {
    if (g_state.cargoHold[1] < 4) {
        AddLog("WARNING: Insufficient Silicates! Need 4T Silicates to charge 2 EMP capacitors.", 3);
        return;
    }
    g_state.cargoHold[1] -= 4;
    g_state.empCharges += 2;
    UpdateCargoTotal();
    TriggerSound(SFX_SYNTH);
    AddLog("Synthesized +2 EMP Flak Pulse Capacitors.", 5);
    AddFloatingText("+2 EMP CHARGES", g_state.shipX, g_state.shipY - 30.0f, RGB(192, 132, 252));
}

void OverchargeShield(void) {
    if (g_state.credits < 200) {
        AddLog("WARNING: Insufficient Credits! Need 200 CR to overcharge shields.", 3);
        return;
    }
    g_state.credits -= 200;
    g_state.shield = g_state.maxShield * 1.3f;
    TriggerSound(SFX_SYNTH);
    AddFloatingText("SHIELD OVERCHARGED (130%)!", g_state.shipX, g_state.shipY - 25.0f, RGB(16, 185, 129));
    AddLog("Deflector matrix overcharged to 130% capacity.", 5);
}

void DeployCombatDrones(void) {
    if (g_state.credits < 350) {
        AddLog("WARNING: Insufficient Credits! Need 350 CR to deploy combat drones.", 3);
        return;
    }
    g_state.credits -= 350;
    g_state.repairDronesActive = 1;
    g_state.droneTimer = 18.0f;
    TriggerSound(SFX_SYNTH);
    AddFloatingText("COMBAT REPAIR DRONES ACTIVE!", g_state.shipX, g_state.shipY - 25.0f, RGB(56, 189, 248));
    AddLog("Deployed Autonomous Combat & Hull Drone Swarm (18s active).", 5);
}

void SpawnDerelict(int index, int templateIdx) {
    if (index < 0 || index >= MAX_DERELICTS) return;
    if (templateIdx < 0 || templateIdx >= 4) templateIdx = 0;
    
    Derelict* d = &g_state.derelicts[index];
    const DerelictTemplate* t = &DERELICT_TEMPLATES[templateIdx];
    
    sprintf(d->id, "DER-%03d", 100 + index * 23 + (rand() % 70));
    strncpy(d->name, t->name, 47);
    d->name[47] = '\0';
    strncpy(d->classType, t->classType, 31);
    d->classType[31] = '\0';
    
    float angle = ((float)rand() / (float)RAND_MAX) * 6.28318f;
    float dist = 380.0f + (((float)rand() / (float)RAND_MAX) * 720.0f);
    d->x = g_state.shipX + (float)cos(angle) * dist;
    d->y = g_state.shipY + (float)sin(angle) * dist;
    d->vx = (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.15f;
    d->vy = (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.15f;
    d->rot = ((float)rand() / (float)RAND_MAX) * 6.28318f;
    d->rotSpeed = (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.008f;
    d->length = 50.0f + (((float)rand() / (float)RAND_MAX) * 20.0f);
    d->width = 24.0f + (((float)rand() / (float)RAND_MAX) * 10.0f);
    d->hullColor = t->hullColor;
    
    d->airlockBreached = 0;
    d->airlockCutProgress = 0;
    d->blackBoxDecrypted = 0;
    d->reactorHarvested = 0;
    d->cargoScavenged = 0;
    d->scrapPods = t->podsMin + (rand() % (t->podsMax - t->podsMin + 1));
    d->dataValue = t->dataValue;
    d->coreValue = t->coreValue;
    strncpy(d->logArchive, t->logArchive, 255);
    d->logArchive[255] = '\0';
    
    d->maxHp = 200.0f;
    d->hp = d->maxHp;
    d->active = 1;
}

void BreachAirlock(int index) {
    if (index < 0 || index >= MAX_DERELICTS || !g_state.derelicts[index].active) return;
    Derelict* d = &g_state.derelicts[index];
    if (d->airlockBreached) {
        AddLog("Airlock is already breached and depressurized!", 3);
        return;
    }
    
    d->airlockCutProgress += 35;
    if (d->airlockCutProgress < 100) {
        TriggerSound(SFX_PLASMA_CUT);
        char buf[128];
        sprintf(buf, "Plasma torch cutting through %s reinforced airlock: %d%%...", d->id, d->airlockCutProgress);
        AddLog(buf, 1);
        AddFloatingText("PLASMA TORCH CUTTING...", g_state.shipX, g_state.shipY - 25.0f, RGB(245, 158, 11));
    } else {
        d->airlockCutProgress = 100;
        d->airlockBreached = 1;
        TriggerSound(SFX_BREACH);
        char buf[128];
        sprintf(buf, "EVA SUCCESS: Airlock breached on %s! Interior decks accessible.", d->id);
        AddLog(buf, 5);
        AddFloatingText("AIRLOCK BREACHED!", g_state.shipX, g_state.shipY - 30.0f, RGB(16, 185, 129));
        
        // Yield immediate scrap metal from cut door
        g_state.cargoHold[5] += 2; // Derelict Scrap
        UpdateCargoTotal();
        AddFloatingText("+2T DERELICT SCRAP", g_state.shipX, g_state.shipY - 45.0f, ORE_DEFS[5].color);
    }
}

void DecryptBlackBox(int index) {
    if (index < 0 || index >= MAX_DERELICTS || !g_state.derelicts[index].active) return;
    Derelict* d = &g_state.derelicts[index];
    if (!d->airlockBreached) {
        AddLog("Airlock must be breached first before accessing avionics bridge!", 4);
        return;
    }
    if (d->blackBoxDecrypted) {
        AddLog("Black box telemetry data has already been extracted from this vessel.", 3);
        return;
    }
    
    d->blackBoxDecrypted = 1;
    g_state.credits += d->dataValue;
    TriggerSound(SFX_DECRYPT);
    
    char buf[128];
    sprintf(buf, "RECOVERED BLACK BOX: Decrypted telemetry archive from %s! Bounty: +%d CR.", d->id, d->dataValue);
    AddLog(buf, 5);
    char fTxt[32];
    sprintf(fTxt, "+%d CR (FLIGHT LOGS)", d->dataValue);
    AddFloatingText(fTxt, g_state.shipX, g_state.shipY - 30.0f, RGB(56, 189, 248));
}

void HarvestReactorCore(int index) {
    if (index < 0 || index >= MAX_DERELICTS || !g_state.derelicts[index].active) return;
    Derelict* d = &g_state.derelicts[index];
    if (!d->airlockBreached) {
        AddLog("Airlock must be breached before engineering containment can be reached!", 4);
        return;
    }
    if (d->reactorHarvested) {
        AddLog("Reactor containment core has already been safely harvested.", 3);
        return;
    }
    
    d->reactorHarvested = 1;
    g_state.credits += d->coreValue;
    TriggerSound(SFX_CORE_HARVEST);
    
    char buf[128];
    sprintf(buf, "CRITICAL EVA: Safely stabilized & harvested antimatter core from %s! Yield: +%d CR.", d->id, d->coreValue);
    AddLog(buf, 6);
    char fTxt[32];
    sprintf(fTxt, "+%d CR (REACTOR CORE)", d->coreValue);
    AddFloatingText(fTxt, g_state.shipX, g_state.shipY - 35.0f, RGB(244, 63, 94));
}

void ScavengeCargoPods(int index) {
    if (index < 0 || index >= MAX_DERELICTS || !g_state.derelicts[index].active) return;
    Derelict* d = &g_state.derelicts[index];
    if (d->cargoScavenged) {
        AddLog("Cargo holds have already been completely stripped of scrap pods.", 3);
        return;
    }
    
    UpdateCargoTotal();
    int spaceLeft = g_state.maxCargo - g_state.totalCargo;
    if (spaceLeft <= 0) {
        AddLog("WARNING: Cargo hold FULL! Cannot scavenge scrap pods.", 3);
        return;
    }
    
    int podsTake = min(d->scrapPods, spaceLeft);
    if (podsTake <= 0) podsTake = 1;
    
    d->cargoScavenged = 1;
    g_state.cargoHold[5] += podsTake; // Derelict scrap
    UpdateCargoTotal();
    TriggerSound(SFX_COLLECT);
    
    char buf[128];
    sprintf(buf, "EVA SCAVENGE: Extracted %dT of high-grade Derelict Scrap pods from %s.", podsTake, d->id);
    AddLog(buf, 2);
    char fTxt[32];
    sprintf(fTxt, "+%dT DERELICT SCRAP", podsTake);
    AddFloatingText(fTxt, g_state.shipX, g_state.shipY - 30.0f, ORE_DEFS[5].color);
}

// --- Phase 9: Life Support Crisis & Damage Control Actions ---
void ActionSealBreaches(void) {
    if (g_state.breachCount == 0) {
        AddLog("No active hull breaches detected. Hull is currently airtight.", 0);
        return;
    }
    int breachCost = 50 * g_state.breachCount;
    if (g_state.credits < breachCost && g_state.cargoHold[0] < 5 && g_state.cargoHold[5] < 3) {
        AddLog("Insufficient credits or materials (5 Ferrum or 3 Scrap) to deploy nanite sealant.", 4);
        return;
    }
    
    if (g_state.cargoHold[5] >= 3) {
        g_state.cargoHold[5] -= 3;
        AddLog("Used 3T Derelict Scrap to weld structural patches.", 0);
    } else if (g_state.cargoHold[0] >= 5) {
        g_state.cargoHold[0] -= 5;
        AddLog("Used 5T Ferrum Ore to synthesize bulkheads.", 0);
    } else {
        g_state.credits -= breachCost;
        char cBuf[64];
        sprintf(cBuf, "Deducted %d CR for nanite sealant injection.", breachCost);
        AddLog(cBuf, 0);
    }
    
    UpdateCargoTotal();
    int count = g_state.breachCount;
    for (int i = 0; i < MAX_COMPARTMENTS; i++) g_state.hullBreaches[i] = 0;
    g_state.breachCount = 0;
    g_state.hull = min(g_state.maxHull, g_state.hull + 15.0f);
    TriggerSound(SFX_SEAL_WELD);
    
    char logB[128];
    sprintf(logB, "DAMAGE CONTROL: Sealed %d hull breaches! Atmosphere leak stopped. +15%% Hull restored.", count);
    AddLog(logB, 5);
    AddFloatingText("BREACHES SEALED", g_state.shipX, g_state.shipY - 35.0f, RGB(16, 185, 129));
}

void ActionVentPlasma(void) {
    TriggerSound(SFX_PLASMA_VENT);
    g_state.plasmaLeaks = 0;
    g_state.heat = max(0.0f, g_state.heat - 45.0f);
    g_state.laserOverheated = 0;
    
    // Generate venting particle spray
    for (int i = 0; i < 24; i++) {
        float angle = g_state.shipAngle + 3.14159f + (((float)rand() / (float)RAND_MAX) - 0.5f) * 1.2f;
        float spd = 4.0f + (((float)rand() / (float)RAND_MAX) * 6.0f);
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (!g_state.particles[p].active) {
                g_state.particles[p].x = g_state.shipX - (float)cos(g_state.shipAngle) * 20.0f;
                g_state.particles[p].y = g_state.shipY - (float)sin(g_state.shipAngle) * 20.0f;
                g_state.particles[p].vx = (float)cos(angle) * spd;
                g_state.particles[p].vy = (float)sin(angle) * spd;
                g_state.particles[p].color = (rand() % 2 == 0) ? RGB(244, 63, 94) : RGB(245, 158, 11);
                g_state.particles[p].life = 1.0f;
                g_state.particles[p].decay = 0.05f;
                g_state.particles[p].size = 3.0f;
                g_state.particles[p].active = 1;
                break;
            }
        }
    }
    
    AddLog("EMERGENCY VENT: Superheated plasma manifold purged into space! -45% Heat dumped, conduit fires extinguished.", 3);
    AddFloatingText("PLASMA PURGED (-45% HEAT)", g_state.shipX, g_state.shipY - 35.0f, RGB(245, 158, 11));
}

void ActionFlushRadiation(void) {
    if (g_state.radiation <= 0.0f) {
        AddLog("Radiation dosimeter reading 0 Rads (Clean).", 0);
        return;
    }
    TriggerSound(SFX_DECON_FLUSH);
    int purged = (int)g_state.radiation;
    g_state.radiation = 0.0f;
    char logB[128];
    sprintf(logB, "DECONTAMINATION: Lead nano-scrubbers flushed %d Rads of stellar radiation from crew quarters!", purged);
    AddLog(logB, 5);
    AddFloatingText("RADIATION CLEARED", g_state.shipX, g_state.shipY - 35.0f, RGB(16, 185, 129));
}

void ActionServiceO2Scrubbers(void) {
    TriggerSound(SFX_DECON_FLUSH);
    g_state.o2Scrubber = 100.0f;
    g_state.o2 = min(100.0f, g_state.o2 + 35.0f);
    AddLog("LIFE SUPPORT: CO2 filtration scrubbers cleansed & fresh O2 synthesized from silicates. +35% Oxygen restored.", 5);
    AddFloatingText("+35% O2 RESTORED", g_state.shipX, g_state.shipY - 35.0f, RGB(16, 185, 129));
}

void ActionDeployRepairDrones(void) {
    if (g_state.repairDronesActive) {
        AddLog("Repair drone swarm is already deployed and active.", 3);
        return;
    }
    TriggerSound(SFX_COLLECT);
    g_state.repairDronesActive = 1;
    g_state.droneTimer = 10.0f;
    AddLog("REPAIR SWARM: Autonomous nano-repair drones deployed outside the hull!", 5);
    AddFloatingText("REPAIR DRONES LAUNCHED", g_state.shipX, g_state.shipY - 35.0f, RGB(56, 189, 248));
}

// Phase 10: Refinery Smelting & Fuel Lab Functions
void SmeltRecipe(int recipeIdx, int times) {
    if (recipeIdx < 0 || recipeIdx >= 6) return;
    const RefineryRecipeDef* r = &REFINERY_RECIPES[recipeIdx];
    
    int maxCraft = 999;
    for (int i = 0; i < 6; i++) {
        if (r->inputs[i] > 0) {
            int possible = g_state.cargoHold[i] / r->inputs[i];
            if (possible < maxCraft) maxCraft = possible;
        }
    }
    
    if (maxCraft <= 0) {
        char buf[128];
        sprintf(buf, "Insufficient minerals for [%s]. Req: %s", r->name, r->inputStr);
        AddLog(buf, 3);
        return;
    }
    
    int craftCount = min(times, maxCraft);
    for (int i = 0; i < 6; i++) {
        g_state.cargoHold[i] -= r->inputs[i] * craftCount;
    }
    UpdateCargoTotal();
    
    int bonusCount = 0;
    if (g_state.catalyticBoost) {
        for (int i = 0; i < craftCount; i++) {
            if ((rand() % 100) < 25) bonusCount++;
        }
        g_state.heat = min(100.0f, g_state.heat + 3.5f * craftCount);
    }
    
    int totalOutput = craftCount + bonusCount;
    g_state.refined[recipeIdx] += totalOutput;
    
    if (recipeIdx == 2 || recipeIdx == 4) TriggerSound(SFX_SYNTH);
    else TriggerSound(SFX_SMELT);
    
    char logB[128];
    if (bonusCount > 0) {
        sprintf(logB, "SMELTED: Refined %dx [%s] (+%d Catalyst Bonus!) from %d batch(es)!", totalOutput, r->outputName, bonusCount, craftCount);
    } else {
        sprintf(logB, "SMELTED: Refined %dx [%s] from %d batch(es)!", totalOutput, r->outputName, craftCount);
    }
    AddLog(logB, 5);
    
    char fTxt[32];
    sprintf(fTxt, "+%d %s", totalOutput, r->outputName);
    AddFloatingText(fTxt, g_state.shipX, g_state.shipY - 35.0f, r->color);
}

void UseRefinedItem(int itemIdx) {
    if (itemIdx < 0 || itemIdx >= 6) return;
    if (g_state.refined[itemIdx] <= 0) return;
    
    if (itemIdx == 2) { // Warp Fuel Cell
        g_state.refined[itemIdx]--;
        g_state.fuel = min(100.0f, g_state.fuel + 50.0f);
        TriggerSound(SFX_COLLECT);
        char logB[128];
        sprintf(logB, "WARP SYNTHESIS: Injected sub-space warp fuel cell. Fuel reserves: %d%%!", (int)g_state.fuel);
        AddLog(logB, 5);
        AddFloatingText("+50% WARP FUEL", g_state.shipX, g_state.shipY - 35.0f, RGB(245, 158, 11));
    } else if (itemIdx == 4) { // Nanite Paste
        g_state.refined[itemIdx]--;
        g_state.hull = min(g_state.maxHull, g_state.hull + 40.0f);
        for (int i = 0; i < MAX_COMPARTMENTS; i++) {
            if (g_state.hullBreaches[i]) {
                g_state.hullBreaches[i] = 0;
                break;
            }
        }
        TriggerSound(SFX_SEAL_WELD);
        char logB[128];
        sprintf(logB, "NANITE WELD: Applied bulkhead repair paste. Hull integrity: %d/%d!", (int)g_state.hull, (int)g_state.maxHull);
        AddLog(logB, 5);
        AddFloatingText("+40% HULL REPAIRED", g_state.shipX, g_state.shipY - 35.0f, RGB(16, 185, 129));
    } else if (itemIdx == 5) { // O2 Canister
        g_state.refined[itemIdx]--;
        g_state.o2 = min(100.0f, g_state.o2 + 45.0f);
        g_state.o2Scrubber = min(100.0f, g_state.o2Scrubber + 30.0f);
        TriggerSound(SFX_DECON_FLUSH);
        char logB[128];
        sprintf(logB, "LIFE SUPPORT: Dispensed medical O2 canister. Atmosphere replenished: %d%%!", (int)g_state.o2);
        AddLog(logB, 5);
        AddFloatingText("+45% O2 RESTORED", g_state.shipX, g_state.shipY - 35.0f, RGB(96, 165, 250));
    }
}

void SellAllRefined(void) {
    int totalCR = 0;
    int totalItems = 0;
    for (int i = 0; i < 6; i++) {
        int count = g_state.refined[i];
        if (count > 0) {
            totalCR += count * REFINERY_RECIPES[i].value;
            totalItems += count;
            g_state.refined[i] = 0;
        }
    }
    if (totalCR == 0) {
        AddLog("No refined goods currently in storage vault to liquidate.", 3);
        return;
    }
    g_state.credits += totalCR;
    TriggerSound(SFX_COLLECT);
    char logB[128];
    sprintf(logB, "COMMERCE: Liquidated %d refined hyper-alloys & synthetic goods for +%d CR!", totalItems, totalCR);
    AddLog(logB, 5);
    char fTxt[32];
    sprintf(fTxt, "+%d CR", totalCR);
    AddFloatingText(fTxt, g_state.shipX, g_state.shipY - 40.0f, RGB(251, 191, 36));
}

void RefineAllOres(void) {
    int craftedTotal = 0;
    for (int r = 0; r < 6; r++) {
        int maxCraft = 999;
        for (int i = 0; i < 6; i++) {
            if (REFINERY_RECIPES[r].inputs[i] > 0) {
                int possible = g_state.cargoHold[i] / REFINERY_RECIPES[r].inputs[i];
                if (possible < maxCraft) maxCraft = possible;
            }
        }
        if (maxCraft > 0) {
            SmeltRecipe(r, maxCraft);
            craftedTotal += maxCraft;
        }
    }
    if (craftedTotal == 0) {
        AddLog("Insufficient raw mineral ores in cargo hold for automated smelting batch.", 3);
    }
}

void ToggleCatalyticBoost(void) {
    g_state.catalyticBoost = !g_state.catalyticBoost;
    TriggerSound(SFX_BEEP);
    char logB[128];
    sprintf(logB, "Refinery catalytic reaction accelerator %s.", g_state.catalyticBoost ? "ENGAGED (+25% bonus yield chance)" : "DISENGAGED");
    AddLog(logB, 0);
}

void ToggleDocking(void) {
    if (g_state.currentSectorIndex < 0 || g_state.currentSectorIndex >= 4) return;
    const StationDef* st = &STATION_DEFS[g_state.currentSectorIndex];
    float dist = (float)sqrt((g_state.shipX - g_state.stationX) * (g_state.shipX - g_state.stationX) +
                             (g_state.shipY - g_state.stationY) * (g_state.shipY - g_state.stationY));
    
    if (g_state.stationDocked) {
        g_state.stationDocked = 0;
        // Eject slightly away
        float ang = (float)atan2(g_state.shipY - g_state.stationY, g_state.shipX - g_state.stationX);
        g_state.shipVx = (float)cos(ang) * 1.5f;
        g_state.shipVy = (float)sin(ang) * 1.5f;
        TriggerSound(SFX_BEEP);
        char logB[128];
        sprintf(logB, "UNDOCKED from %s. Magnetic clamp disengaged.", st->name);
        AddLog(logB, 0);
        AddFloatingText("UNDOCKED", g_state.shipX, g_state.shipY - 35.0f, RGB(251, 191, 36));
    } else {
        if (dist > 300.0f) {
            AddLog("Cannot dock: Vessel is out of spaceport magnetic tractor mooring range (>300m).", 3);
            return;
        }
        g_state.stationDocked = 1;
        g_state.shipVx = 0.0f;
        g_state.shipVy = 0.0f;
        TriggerSound(SFX_DOCK);
        char logB[128];
        sprintf(logB, "DOCKED with %s. Mooring clamps locked.", st->name);
        AddLog(logB, 5);
        AddFloatingText("DOCKED WITH STATION", g_state.shipX, g_state.shipY - 35.0f, RGB(56, 189, 248));
    }
}

void ServiceRepairHull(void) {
    const StationDef* st = &STATION_DEFS[g_state.currentSectorIndex];
    float missing = g_state.maxHull - g_state.hull;
    if (missing <= 0.0f) {
        AddLog("Station Drydock: Vessel armor plating and structural integrity are at 100%.", 3);
        return;
    }
    int totalCost = (int)ceil(missing * (float)st->repairCostPerPct);
    if (g_state.credits < totalCost) {
        AddLog("Station Drydock: Insufficient credits for full shipyard drydock armor restoration.", 4);
        return;
    }
    g_state.credits -= totalCost;
    g_state.hull = g_state.maxHull;
    for (int i = 0; i < MAX_COMPARTMENTS; i++) g_state.hullBreaches[i] = 0;
    g_state.breachCount = 0;
    TriggerSound(SFX_SEAL_WELD);
    char logB[128];
    sprintf(logB, "DRYDOCK: Shipyard repaired hull armor & sealed all breaches for %d CR.", totalCost);
    AddLog(logB, 5);
    char fTxt[32];
    sprintf(fTxt, "HULL RESTORED (-%d CR)", totalCost);
    AddFloatingText(fTxt, g_state.shipX, g_state.shipY - 35.0f, RGB(16, 185, 129));
}

void ServiceRechargeShield(void) {
    const StationDef* st = &STATION_DEFS[g_state.currentSectorIndex];
    if (g_state.shield >= g_state.maxShield) {
        AddLog("Station Power Grid: Deflector grid capacitor banks are fully charged.", 3);
        return;
    }
    if (g_state.credits < st->shieldCost) {
        AddLog("Station Power Grid: Insufficient credits for deflector grid supercharge.", 4);
        return;
    }
    g_state.credits -= st->shieldCost;
    g_state.shield = g_state.maxShield;
    TriggerSound(SFX_SYNTH);
    char logB[128];
    sprintf(logB, "GRID: Station capacitor conduit supercharged deflector shields for %d CR.", st->shieldCost);
    AddLog(logB, 5);
    char fTxt[32];
    sprintf(fTxt, "SHIELDS FULL (-%d CR)", st->shieldCost);
    AddFloatingText(fTxt, g_state.shipX, g_state.shipY - 35.0f, RGB(56, 189, 248));
}

void ServiceRefuel(void) {
    const StationDef* st = &STATION_DEFS[g_state.currentSectorIndex];
    float missing = g_state.maxFuel - g_state.fuel;
    if (missing <= 0.0f) {
        AddLog("Station Bunkers: Hydrazine fuel reaction propellant tanks are 100% full.", 3);
        return;
    }
    int totalCost = (int)ceil(missing * (float)st->fuelCostPerPct);
    if (g_state.credits < totalCost) {
        AddLog("Station Bunkers: Insufficient credits to replenish propellant tanks.", 4);
        return;
    }
    g_state.credits -= totalCost;
    g_state.fuel = g_state.maxFuel;
    TriggerSound(SFX_DECON_FLUSH);
    char logB[128];
    sprintf(logB, "BUNKERS: Piped %.0fL high-grade hydrazine propellant into main tanks for %d CR.", missing, totalCost);
    AddLog(logB, 5);
    char fTxt[32];
    sprintf(fTxt, "TANKS TOPPED (-%d CR)", totalCost);
    AddFloatingText(fTxt, g_state.shipX, g_state.shipY - 35.0f, RGB(251, 191, 36));
}

void ServiceDecon(void) {
    const StationDef* st = &STATION_DEFS[g_state.currentSectorIndex];
    if (g_state.radiation <= 0.0f && g_state.o2 >= 100.0f && g_state.o2Scrubber >= 100.0f) {
        AddLog("Station Bio-Bay: Bio-hazard scrubbers indicate 0 rads and 100% atmospheric purity.", 3);
        return;
    }
    if (g_state.credits < st->deconCost) {
        AddLog("Station Bio-Bay: Insufficient credits for radiological decontamination.", 4);
        return;
    }
    g_state.credits -= st->deconCost;
    g_state.radiation = 0.0f;
    g_state.o2 = 100.0f;
    g_state.o2Scrubber = 100.0f;
    g_state.plasmaLeaks = 0;
    TriggerSound(SFX_DECON_FLUSH);
    char logB[128];
    sprintf(logB, "BIO-BAY: Pressurized decontamination cycle flushed all radiation and restored atmosphere for %d CR.", st->deconCost);
    AddLog(logB, 5);
    char fTxt[32];
    sprintf(fTxt, "DECON COMPLETE (-%d CR)", st->deconCost);
    AddFloatingText(fTxt, g_state.shipX, g_state.shipY - 35.0f, RGB(16, 185, 129));
}

void BarterSellAllWithTariff(void) {
    const StationDef* st = &STATION_DEFS[g_state.currentSectorIndex];
    int totalCR = 0;
    int itemsCount = 0;
    for (int i = 0; i < 6; i++) {
        int count = g_state.cargoHold[i];
        if (count > 0) {
            float mult = st->multipliers[i];
            int unitPrice = (int)round((float)ORE_DEFS[i].value * mult);
            totalCR += count * unitPrice;
            itemsCount += count;
            g_state.cargoHold[i] = 0;
        }
    }
    // Also sell refined goods if any
    for (int i = 0; i < 6; i++) {
        int count = g_state.refined[i];
        if (count > 0) {
            float mult = st->multipliers[6 + i];
            int unitPrice = (int)round((float)REFINERY_RECIPES[i].value * mult);
            totalCR += count * unitPrice;
            itemsCount += count;
            g_state.refined[i] = 0;
        }
    }
    UpdateCargoTotal();
    if (totalCR == 0) {
        AddLog("Trade Terminal: No raw minerals or refined cargo pods in hold to barter.", 3);
        return;
    }
    g_state.credits += totalCR;
    g_state.factionRep[g_state.currentSectorIndex] += min(15, itemsCount / 4 + 1);
    TriggerSound(SFX_BARTER);
    char logB[128];
    sprintf(logB, "BLACK MARKET: Bartered %d cargo crates with %s tariffs for +%d CR!", itemsCount, st->name, totalCR);
    AddLog(logB, 5);
    char fTxt[32];
    sprintf(fTxt, "+%d CR (TARIFF BOOST)", totalCR);
    AddFloatingText(fTxt, g_state.shipX, g_state.shipY - 40.0f, RGB(251, 191, 36));
}

void BarterBuySupply(int supplyType) {
    if (supplyType == 0) { // 2x Warp Cells for 1400 CR
        if (g_state.credits < 1400) { AddLog("Black Market: Insufficient credits for Warp Fuel Cells.", 4); return; }
        g_state.credits -= 1400;
        g_state.refined[2] += 2;
        TriggerSound(SFX_COLLECT);
        AddLog("COMMERCE: Acquired 2x Sub-Space Warp Fuel Cells from station black market for 1,400 CR.", 5);
        AddFloatingText("+2 WARP CELLS", g_state.shipX, g_state.shipY - 35.0f, RGB(244, 63, 94));
    } else if (supplyType == 1) { // 2x Nanite Paste for 1200 CR
        if (g_state.credits < 1200) { AddLog("Black Market: Insufficient credits for Nanite Bulkhead Paste.", 4); return; }
        g_state.credits -= 1200;
        g_state.refined[4] += 2;
        TriggerSound(SFX_COLLECT);
        AddLog("COMMERCE: Acquired 2x Nanite Bulkhead Pastes from station black market for 1,200 CR.", 5);
        AddFloatingText("+2 NANITE PASTE", g_state.shipX, g_state.shipY - 35.0f, RGB(16, 185, 129));
    } else if (supplyType == 2) { // 2x O2 Canisters for 400 CR
        if (g_state.credits < 400) { AddLog("Black Market: Insufficient credits for Medical O2 Canisters.", 4); return; }
        g_state.credits -= 400;
        g_state.refined[5] += 2;
        TriggerSound(SFX_COLLECT);
        AddLog("COMMERCE: Acquired 2x Medical O2 Canisters from station black market for 400 CR.", 5);
        AddFloatingText("+2 O2 CANISTERS", g_state.shipX, g_state.shipY - 35.0f, RGB(96, 165, 250));
    }
}

void ClaimContract(int contractIdx) {
    if (contractIdx < 0 || contractIdx >= 3) return;
    int sec = g_state.currentSectorIndex;
    if (g_state.contractDone[sec][contractIdx]) {
        AddLog("Contracts Guild: This dredging requisition has already been fulfilled.", 3);
        return;
    }
    const StationDef* st = &STATION_DEFS[sec];
    const ContractDef* c = &st->contracts[contractIdx];
    
    if (c->reqRefined) {
        int have = g_state.refined[c->reqItemIdx];
        if (have < c->reqQty) {
            char logB[128];
            sprintf(logB, "Contract requirement incomplete: Need %d units of %s (Hold has %d).", c->reqQty, c->reqLabel, have);
            AddLog(logB, 3);
            return;
        }
        g_state.refined[c->reqItemIdx] -= c->reqQty;
    } else {
        int have = g_state.cargoHold[c->reqItemIdx];
        if (have < c->reqQty) {
            char logB[128];
            sprintf(logB, "Contract requirement incomplete: Need %dT of %s (Hold has %dT).", c->reqQty, c->reqLabel, have);
            AddLog(logB, 3);
            return;
        }
        g_state.cargoHold[c->reqItemIdx] -= c->reqQty;
        UpdateCargoTotal();
    }
    
    g_state.contractDone[sec][contractIdx] = 1;
    g_state.totalContractsDone++;
    g_state.credits += c->rewardCredits;
    g_state.factionRep[sec] += c->rewardRep;
    
    TriggerSound(SFX_CONTRACT);
    char logB[128];
    sprintf(logB, "CONTRACT COMPLETED: [%s] fulfilled! +%d CR | +%d %s Standing!", c->title, c->rewardCredits, c->rewardRep, st->faction);
    AddLog(logB, 5);
    char fTxt[48];
    sprintf(fTxt, "CONTRACT FULFILLED! +%d CR", c->rewardCredits);
    AddFloatingText(fTxt, g_state.shipX, g_state.shipY - 40.0f, RGB(251, 191, 36));
}

void InitSectorField(int sectorIdx) {
    if (sectorIdx < 0 || sectorIdx >= 4) sectorIdx = 0;
    g_state.currentSectorIndex = sectorIdx;
    g_state.selectedSectorIndex = sectorIdx;
    strncpy(g_state.sector, SECTOR_DEFS[sectorIdx].name, 31);
    g_state.sector[31] = '\0';
    
    // Position Station for this sector
    g_state.stationX = (sectorIdx % 2 == 0) ? 350.0f : -350.0f;
    g_state.stationY = (sectorIdx < 2) ? -300.0f : 300.0f;
    g_state.stationRadius = 65.0f;
    g_state.stationAngle = 0.0f;
    g_state.stationRotSpeed = 0.008f + ((float)sectorIdx * 0.003f);
    g_state.stationDocked = 0;
    g_state.stationBeaconTimer = 0.0f;
    
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
    
    // Seed Derelicts
    int derelictCount = 2;
    if (sectorIdx == 1) derelictCount = 3;
    else if (sectorIdx == 2) derelictCount = 5; // Derelict Graveyard
    else if (sectorIdx == 3) derelictCount = 3; // Nebula
    
    for (int i = 0; i < MAX_DERELICTS; i++) {
        if (i < derelictCount) {
            SpawnDerelict(i, rand() % 4);
        } else {
            g_state.derelicts[i].active = 0;
        }
    }
    
    // Seed Raiders per sector threat level
    int raiderCount = (sectorIdx == 0) ? 1 : ((sectorIdx == 1) ? 2 : ((sectorIdx == 2) ? 3 : 4));
    for (int i = 0; i < MAX_RAIDERS; i++) {
        if (i < raiderCount) {
            int rType = (sectorIdx == 0) ? 0 : ((sectorIdx == 1) ? (i % 2) : ((sectorIdx == 2) ? (1 + (i % 2)) : (i % 3)));
            SpawnRaider(i, rType);
        } else {
            g_state.raiders[i].active = 0;
        }
    }

    for (int i = 0; i < MAX_PROJECTILES; i++) g_state.projectiles[i].active = 0;
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) g_state.enemyProjectiles[i].active = 0;
    for (int i = 0; i < MAX_SHOCKWAVES; i++) g_state.shockwaves[i].active = 0;
    for (int i = 0; i < MAX_PIRATE_WRECKS; i++) g_state.pirateWrecks[i].active = 0;
    for (int i = 0; i < MAX_ORE_CHUNKS; i++) g_state.oreChunks[i].active = 0;
    g_state.selectedAstIndex = -1;
    g_state.selectedDerelictIndex = -1;
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

void BuyUpgrade(int moduleIdx) {
    char buf[128];
    char fTxt[32];
    if (moduleIdx == 0) { // Drill
        if (g_state.upgradeDrill >= 4) { AddLog("Heavy Plasma Drill is already at maximum tier!", 3); return; }
        const DrillUpgradeDef* nextTier = &DRILL_UPGRADES[g_state.upgradeDrill + 1];
        if (g_state.credits < nextTier->cost) { AddLog("Insufficient credits for drill upgrade!", 4); return; }
        g_state.credits -= nextTier->cost;
        g_state.upgradeDrill++;
        sprintf(buf, "Installed [%s] for %d CR! Drill DPS +%.0f%%.", nextTier->name, nextTier->cost, (nextTier->dpsBonus - 1.0f) * 100.0f);
        AddLog(buf, 5);
        sprintf(fTxt, "RETROFIT: %s", nextTier->name);
        AddFloatingText(fTxt, g_state.shipX, g_state.shipY - 30.0f, RGB(16, 185, 129));
        TriggerSound(SFX_COLLECT);
    } else if (moduleIdx == 1) { // Cargo
        if (g_state.upgradeCargo >= 4) { AddLog("Titan Cargo Bay is already at maximum tier!", 3); return; }
        const CargoUpgradeDef* nextTier = &CARGO_UPGRADES[g_state.upgradeCargo + 1];
        if (g_state.credits < nextTier->cost) { AddLog("Insufficient credits for cargo bay expansion!", 4); return; }
        g_state.credits -= nextTier->cost;
        g_state.upgradeCargo++;
        g_state.maxCargo = nextTier->capacity;
        sprintf(buf, "Installed [%s] for %d CR! Max capacity expanded to %d T.", nextTier->name, nextTier->cost, nextTier->capacity);
        AddLog(buf, 5);
        sprintf(fTxt, "EXPAND: %d T", nextTier->capacity);
        AddFloatingText(fTxt, g_state.shipX, g_state.shipY - 30.0f, RGB(16, 185, 129));
        TriggerSound(SFX_COLLECT);
    } else if (moduleIdx == 2) { // Engine
        if (g_state.upgradeEngine >= 4) { AddLog("Fusion Thrusters are already at maximum tier!", 3); return; }
        const EngineUpgradeDef* nextTier = &ENGINE_UPGRADES[g_state.upgradeEngine + 1];
        if (g_state.credits < nextTier->cost) { AddLog("Insufficient credits for thruster upgrade!", 4); return; }
        g_state.credits -= nextTier->cost;
        g_state.upgradeEngine++;
        sprintf(buf, "Installed [%s] for %d CR! Top speed increased to %.1f km/s.", nextTier->name, nextTier->cost, nextTier->maxSpeed);
        AddLog(buf, 5);
        sprintf(fTxt, "DRIVE: %s", nextTier->name);
        AddFloatingText(fTxt, g_state.shipX, g_state.shipY - 30.0f, RGB(16, 185, 129));
        TriggerSound(SFX_COLLECT);
    } else if (moduleIdx == 3) { // Shield
        if (g_state.upgradeShield >= 4) { AddLog("Particle Shields are already at maximum tier!", 3); return; }
        const ShieldUpgradeDef* nextTier = &SHIELD_UPGRADES[g_state.upgradeShield + 1];
        if (g_state.credits < nextTier->cost) { AddLog("Insufficient credits for particle shield upgrade!", 4); return; }
        g_state.credits -= nextTier->cost;
        g_state.upgradeShield++;
        g_state.maxShield = nextTier->maxShield;
        g_state.maxHull = nextTier->maxHull;
        g_state.shield = g_state.maxShield;
        g_state.hull = g_state.maxHull;
        sprintf(buf, "Installed [%s] for %d CR! Shield max %d / Hull max %d.", nextTier->name, nextTier->cost, (int)nextTier->maxShield, (int)nextTier->maxHull);
        AddLog(buf, 5);
        sprintf(fTxt, "SHIELD: %.0f", nextTier->maxShield);
        AddFloatingText(fTxt, g_state.shipX, g_state.shipY - 30.0f, RGB(16, 185, 129));
        TriggerSound(SFX_COLLECT);
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
    g_state.o2Scrubber = 100.0f;
    g_state.radiation = 0.0f;
    for (int i = 0; i < MAX_COMPARTMENTS; i++) g_state.hullBreaches[i] = 0;
    g_state.breachCount = 0;
    g_state.plasmaLeaks = 0;
    g_state.repairDronesActive = 0;
    g_state.droneTimer = 0.0f;
    g_state.showCrisis = 0;
    g_state.showDefense = 0;
    g_state.selectedWeapon = 0;
    g_state.railgunSlugs = 15;
    g_state.empCharges = 6;
    g_state.autoPDL = 1;
    g_state.railgunCooldown = 0.0f;
    g_state.flakCooldown = 0.0f;
    g_state.pdlCooldown = 0.0f;
    g_state.chaffCooldown = 0.0f;
    g_state.piratesDefeated = 0;
    g_state.bountiesClaimed = 0;
    g_state.maxCargo = 200;
    g_state.dampeners = 1;
    g_state.soundEnabled = 1;
    g_state.themeIndex = 0;
    g_state.scanlineMode = 1;
    g_state.shipAngle = -1.57079f; // Facing UP
    g_state.selectedAstIndex = -1;
    g_state.upgradeDrill = 0;
    g_state.upgradeCargo = 0;
    g_state.upgradeEngine = 0;
    g_state.upgradeShield = 0;
    g_state.showUpgrades = 0;
    g_state.showRefinery = 0;
    g_state.catalyticBoost = 1;
    memset(g_state.refined, 0, sizeof(g_state.refined));
    g_state.crucibleAnimTime = 0.0f;
    
    InitSectorField(0);
    
    AddLog("[SYSTEM] KStarDredge Mk-IV cockpit operational. Core reactor online.", 0);
    AddLog("[DEFENSE] Armory online: Railguns, EMP Flak, Auto-PDL active [X].", 0);
    AddLog("[MINING] High-frequency mining laser ready. Aim at asteroids and hold [SPACE].", 1);
    AddLog("[TRACTOR] Tractor emitter active. Hold [T] to gather extracted mineral chunks.", 2);
    AddLog("[UPGRADES] Modular engineering bay online. Press [U] for Barge Retrofits.", 0);
    AddLog("[REFINERY] Metallurgical refinery & fuel lab online. Press [R] for Smelting.", 0);
    AddLog("[NAV] Astronavigation computer initialized. Press [N] for Sector Charts.", 0);
    AddLog("[CRISIS] Damage control station ready. Press [K] for Life Support & Breaches.", 0);
}

void UpdateGame(float dt) {
    const EngineUpgradeDef* engDef = &ENGINE_UPGRADES[g_state.upgradeEngine];
    const DrillUpgradeDef* drillDef = &DRILL_UPGRADES[g_state.upgradeDrill];
    const ShieldUpgradeDef* shieldDef = &SHIELD_UPGRADES[g_state.upgradeShield];

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
    float rotAccel = engDef->rot;
    if (g_state.turningLeft) g_state.shipAngle -= rotAccel;
    if (g_state.turningRight) g_state.shipAngle += rotAccel;
    
    // Propulsion
    float thrustPower = engDef->thrust;
    float fuelDrainRate = 0.03f * engDef->fuelBurn;
    if (g_state.thrusting && g_state.fuel > 0.0f) {
        g_state.shipVx += (float)cos(g_state.shipAngle) * thrustPower;
        g_state.shipVy += (float)sin(g_state.shipAngle) * thrustPower;
        g_state.fuel = max(0.0f, g_state.fuel - fuelDrainRate);
        if (rand() % 30 == 0) TriggerSound(SFX_THRUSTER_BURN);
        
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
        g_state.fuel = max(0.0f, g_state.fuel - fuelDrainRate * 0.5f);
        if (rand() % 35 == 0) TriggerSound(SFX_THRUSTER_BURN);
    }
    
    // Inertial Dampeners
    if (g_state.dampeners && !g_state.thrusting && !g_state.reversing) {
        g_state.shipVx *= 0.96f;
        g_state.shipVy *= 0.96f;
    }
    
    // Speed Cap
    float speed = (float)sqrt(g_state.shipVx * g_state.shipVx + g_state.shipVy * g_state.shipVy);
    if (speed > engDef->maxSpeed) {
        g_state.shipVx = (g_state.shipVx / speed) * engDef->maxSpeed;
        g_state.shipVy = (g_state.shipVy / speed) * engDef->maxSpeed;
    }
    
    g_state.shipX += g_state.shipVx;
    g_state.shipY += g_state.shipVy;
    
    // Laser Overheat Logic
    if (g_state.miningActive && !g_state.laserOverheated) {
        g_state.heat += 0.5f * drillDef->heatRate;
        if (rand() % 28 == 0) TriggerSound(SFX_LASER_HUM);
        if (g_state.heat > 85.0f && (rand() % 350 == 0) && !g_state.plasmaLeaks) {
            g_state.plasmaLeaks = 1;
            AddLog("WARNING: High thermal load breached plasma manifold conduit!", 4);
            TriggerSound(SFX_ALARM);
            AddFloatingText("PLASMA CONDUIT LEAK!", g_state.shipX, g_state.shipY - 35.0f, RGB(244, 63, 94));
        }
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
    
    // Plasma leak ongoing heat & hull burn
    if (g_state.plasmaLeaks) {
        g_state.heat = min(100.0f, g_state.heat + 1.8f * dt);
        g_state.hull = max(0.0f, g_state.hull - 0.35f * dt);
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
                    
                    float dpsMultiplier = ast->resonantLock ? 1.5f : 1.0f;
                    ast->hp -= 0.6f * drillDef->dpsBonus * dpsMultiplier;
                    
                    int chunkChance = (int)(drillDef->chunkBonus * (ast->resonantLock ? 1.5f : 1.0f) * 100.0f);
                    if ((rand() % 100) < chunkChance) {
                        TriggerSound(SFX_FRACTURE);
                        // Pick ore chunk based on ast->comp
                        int roll = rand() % 100;
                        int dropOre = ast->oreType;
                        int cumulative = 0;
                        for (int o = 0; o < 6; o++) {
                            cumulative += ast->comp[o];
                            if (roll < cumulative) { dropOre = o; break; }
                        }
                        SpawnOreChunk(impactX, impactY, dropOre, 1);
                        char buf[32];
                        sprintf(buf, "+1 %s%s", ORE_DEFS[dropOre].name, ast->resonantLock ? " (RESONANT)" : "");
                        AddFloatingText(buf, impactX, impactY - 10.0f, ORE_DEFS[dropOre].color);
                    }
                    
                    if (ast->hp <= 0.0f) {
                        TriggerSound(SFX_FRACTURE);
                        char buf[128];
                        sprintf(buf, "Asteroid %s shattered into rich mineral fragments!", ast->id);
                        AddLog(buf, 1);
                        AddSparks(ast->x, ast->y, RGB(245, 158, 11), 18);
                        
                        // Base chunks from composition
                        for (int c = 0; c < 5 + (rand() % 3); c++) {
                            int roll = rand() % 100;
                            int dropOre = ast->oreType;
                            int cumulative = 0;
                            for (int o = 0; o < 6; o++) {
                                cumulative += ast->comp[o];
                                if (roll < cumulative) { dropOre = o; break; }
                            }
                            SpawnOreChunk(ast->x + (((float)rand() / (float)RAND_MAX) - 0.5f) * 25.0f,
                                          ast->y + (((float)rand() / (float)RAND_MAX) - 0.5f) * 25.0f,
                                          dropOre, 1);
                        }
                        
                        // Guaranteed Geode Drops
                        if (ast->geodeType == 2) { // Dark Matter
                            for (int g = 0; g < 3; g++) {
                                SpawnOreChunk(ast->x + (((float)rand() / (float)RAND_MAX) - 0.5f) * 20.0f,
                                              ast->y + (((float)rand() / (float)RAND_MAX) - 0.5f) * 20.0f,
                                              4, 1);
                            }
                            AddLog("CRITICAL: Dark Matter Geode core ruptured! High-value dark matter clusters released!", 6);
                            AddFloatingText("DARK MATTER GEODE HARVESTED!", ast->x, ast->y - 25.0f, RGB(244, 63, 94));
                        } else if (ast->geodeType == 1) { // Void Quartz
                            for (int g = 0; g < 3; g++) {
                                SpawnOreChunk(ast->x + (((float)rand() / (float)RAND_MAX) - 0.5f) * 20.0f,
                                              ast->y + (((float)rand() / (float)RAND_MAX) - 0.5f) * 20.0f,
                                              3, 1);
                            }
                            AddLog("GEODE HARVEST: Resonant Void Quartz crystal structure shattered into pure crystals!", 2);
                            AddFloatingText("VOID QUARTZ GEODE HARVESTED!", ast->x, ast->y - 25.0f, RGB(192, 132, 252));
                        } else if (ast->geodeType == 3) { // Platinum
                            for (int g = 0; g < 3; g++) {
                                SpawnOreChunk(ast->x + (((float)rand() / (float)RAND_MAX) - 0.5f) * 20.0f,
                                              ast->y + (((float)rand() / (float)RAND_MAX) - 0.5f) * 20.0f,
                                              2, 1);
                            }
                            AddLog("GEODE HARVEST: Concentrated Platinum Pocket liberated!", 5);
                            AddFloatingText("PLATINUM POCKET HARVESTED!", ast->x, ast->y - 25.0f, RGB(224, 242, 254));
                        }
                        
                        ast->active = 0;
                        if (g_state.selectedAstIndex == i) g_state.selectedAstIndex = -1;
                        SpawnAsteroid(i, PickOreForSector(g_state.currentSectorIndex));
                    }
                    break;
                }
            }
        }
        
        // Also check raycast against derelict spaceships
        for (int i = 0; i < MAX_DERELICTS; i++) {
            Derelict* d = &g_state.derelicts[i];
            if (!d->active) continue;
            
            float toDerX = d->x - lx;
            float toDerY = d->y - ly;
            float proj = toDerX * dirX + toDerY * dirY;
            if (proj > 0.0f && proj < laserRange) {
                float perpX = toDerX - dirX * proj;
                float perpY = toDerY - dirY * proj;
                float distToBeam = (float)sqrt(perpX * perpX + perpY * perpY);
                if (distToBeam < d->length * 0.5f) {
                    float impactX = lx + dirX * proj;
                    float impactY = ly + dirY * proj;
                    AddSparks(impactX, impactY, RGB(245, 158, 11), 3);
                    if (!d->airlockBreached && (rand() % 15 == 0)) {
                        d->airlockCutProgress += 5;
                        if (d->airlockCutProgress >= 100) {
                            d->airlockCutProgress = 100;
                            d->airlockBreached = 1;
                            TriggerSound(SFX_BREACH);
                            AddLog("Mining laser breached derelict airlock!", 5);
                            AddFloatingText("AIRLOCK BREACHED!", impactX, impactY - 20.0f, RGB(16, 185, 129));
                        }
                    }
                    if ((rand() % 100) < 14) {
                        SpawnOreChunk(impactX, impactY, 5, 1); // Derelict scrap
                        AddFloatingText("+1T SCRAP", impactX, impactY - 10.0f, ORE_DEFS[5].color);
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
                if (g_state.breachCount < MAX_COMPARTMENTS && (rand() % 100 < 25)) {
                    int comp = rand() % MAX_COMPARTMENTS;
                    if (!g_state.hullBreaches[comp]) {
                        g_state.hullBreaches[comp] = 1;
                        g_state.breachCount++;
                        TriggerSound(SFX_ALARM);
                        char bLog[64];
                        sprintf(bLog, "CRITICAL: Asteroid collision breached %s!", COMPARTMENT_DEFS[comp].name);
                        AddLog(bLog, 4);
                        AddFloatingText("HULL BREACH!", g_state.shipX, g_state.shipY - 30.0f, RGB(239, 68, 68));
                    }
                }
            }
            AddSparks(g_state.shipX, g_state.shipY, RGB(239, 68, 68), 8);
            TriggerSound(SFX_FRACTURE);
            char buf[64];
            sprintf(buf, "COLLISION ALERT with %s! Shield deflected impact.", ast->id);
            AddLog(buf, 3);
        }
    }
    
    // Derelict Spaceship Physics & Collisions with Ship
    for (int i = 0; i < MAX_DERELICTS; i++) {
        Derelict* d = &g_state.derelicts[i];
        if (!d->active) continue;
        
        d->x += d->vx;
        d->y += d->vy;
        d->rot += d->rotSpeed;
        
        float dist = (float)sqrt((g_state.shipX - d->x) * (g_state.shipX - d->x) +
                                 (g_state.shipY - d->y) * (g_state.shipY - d->y));
        float minDist = (d->length * 0.45f) + 14.0f;
        if (dist < minDist && dist > 0.1f) {
            float angle = (float)atan2(g_state.shipY - d->y, g_state.shipX - d->x);
            g_state.shipVx += (float)cos(angle) * 1.8f;
            g_state.shipVy += (float)sin(angle) * 1.8f;
            
            if (g_state.shield > 0.0f) {
                g_state.shield = max(0.0f, g_state.shield - 12.0f);
            } else {
                g_state.hull = max(0.0f, g_state.hull - 8.0f);
                if (g_state.breachCount < MAX_COMPARTMENTS && (rand() % 100 < 35)) {
                    int comp = rand() % MAX_COMPARTMENTS;
                    if (!g_state.hullBreaches[comp]) {
                        g_state.hullBreaches[comp] = 1;
                        g_state.breachCount++;
                        TriggerSound(SFX_ALARM);
                        char bLog[64];
                        sprintf(bLog, "CRITICAL: Derelict impact breached %s!", COMPARTMENT_DEFS[comp].name);
                        AddLog(bLog, 4);
                        AddFloatingText("HULL BREACH!", g_state.shipX, g_state.shipY - 30.0f, RGB(239, 68, 68));
                    }
                }
            }
            AddSparks(g_state.shipX, g_state.shipY, RGB(239, 68, 68), 10);
            TriggerSound(SFX_FRACTURE);
            char buf[64];
            sprintf(buf, "HULL IMPACT with derelict %s! Deflector grid active.", d->id);
            AddLog(buf, 3);
        }
    }
    
    // Shield Regeneration
    if (g_state.shield < g_state.maxShield) {
        g_state.shield = min(g_state.maxShield, g_state.shield + shieldDef->regen);
    }
    
    // Environmental Radiation Accumulation
    float radRate = 0.1f;
    if (g_state.currentSectorIndex == 1) radRate = 0.35f;
    else if (g_state.currentSectorIndex == 2) radRate = 0.75f;
    else if (g_state.currentSectorIndex == 3) radRate = 1.4f;
    for (int i = 0; i < MAX_DERELICTS; i++) {
        if (!g_state.derelicts[i].active) continue;
        float ddx = g_state.shipX - g_state.derelicts[i].x;
        float ddy = g_state.shipY - g_state.derelicts[i].y;
        float dDist = (float)sqrt(ddx * ddx + ddy * ddy);
        if (dDist < 200.0f) {
            radRate += 0.8f * (1.0f - (dDist / 200.0f));
        }
    }
    g_state.radiation = min(1000.0f, g_state.radiation + radRate * dt);
    if (g_state.radiation >= 500.0f) {
        g_state.hull = max(0.0f, g_state.hull - 0.25f * dt);
    }
    
    // Life Support & O2 Scrubber Simulation
    g_state.o2Scrubber = max(0.0f, g_state.o2Scrubber - 0.05f * dt);
    float breachO2Loss = g_state.breachCount * 0.40f;
    float scrubberLoss = (g_state.o2Scrubber < 30.0f) ? (0.25f * (1.0f - g_state.o2Scrubber / 30.0f)) : 0.0f;
    g_state.o2 = max(0.0f, g_state.o2 - (scrubberLoss + breachO2Loss) * dt);
    if (g_state.o2 <= 0.0f) {
        g_state.hull = max(0.0f, g_state.hull - 1.5f * dt);
    }
    
    // Autonomous Repair Drones
    if (g_state.repairDronesActive) {
        g_state.droneTimer -= dt;
        g_state.hull = min(g_state.maxHull, g_state.hull + 3.0f * dt);
        g_state.shield = min(g_state.maxShield, g_state.shield + 5.0f * dt);
        if (g_state.droneTimer <= 0.0f) {
            g_state.repairDronesActive = 0;
            AddLog("Repair drone swarm completed patrol cycle.", 0);
        }
    }
    
    // Crisis Alarm Periodic Chime
    if (g_state.breachCount > 0 || g_state.plasmaLeaks || g_state.o2 < 20.0f || g_state.radiation > 600.0f) {
        g_state.alarmTick += dt;
        if (g_state.alarmTick >= 5.0f) {
            g_state.alarmTick = 0.0f;
            TriggerSound(SFX_ALARM);
        }
    }
    
    // Update Particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!g_state.particles[i].active) continue;
        g_state.particles[i].x += g_state.particles[i].vx;
        g_state.particles[i].y += g_state.particles[i].vy;
        g_state.particles[i].life -= g_state.particles[i].decay;
        if (g_state.particles[i].life <= 0.0f) g_state.particles[i].active = 0;
    }
    
    // Update Scan Waves
    for (int i = 0; i < MAX_SCAN_WAVES; i++) {
        if (!g_state.scanWaves[i].active) continue;
        g_state.scanWaves[i].r += dt * 450.0f;
        g_state.scanWaves[i].life -= dt * 1.2f;
        if (g_state.scanWaves[i].life <= 0.0f || g_state.scanWaves[i].r >= g_state.scanWaves[i].maxR) {
            g_state.scanWaves[i].active = 0;
        }
    }
    
    // Update Floating Text
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        if (!g_state.texts[i].active) continue;
        g_state.texts[i].y += g_state.texts[i].vy;
        g_state.texts[i].life -= 0.02f;
        if (g_state.texts[i].life <= 0.0f) g_state.texts[i].active = 0;
    }
    
    // --- Phase 12: Defense & Combat Systems Update ---
    if (g_state.railgunCooldown > 0.0f) g_state.railgunCooldown = max(0.0f, g_state.railgunCooldown - dt);
    if (g_state.flakCooldown > 0.0f) g_state.flakCooldown = max(0.0f, g_state.flakCooldown - dt);
    if (g_state.pdlCooldown > 0.0f) g_state.pdlCooldown = max(0.0f, g_state.pdlCooldown - dt);
    if (g_state.chaffCooldown > 0.0f) g_state.chaffCooldown = max(0.0f, g_state.chaffCooldown - dt);

    // Auto-PDL routine
    if (g_state.autoPDL && g_state.pdlCooldown <= 0.0f) {
        int foundThreat = 0;
        for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
            if (!g_state.enemyProjectiles[i].active || g_state.enemyProjectiles[i].type != 1) continue;
            float d = (float)sqrt((g_state.enemyProjectiles[i].x - g_state.shipX) * (g_state.enemyProjectiles[i].x - g_state.shipX) +
                                  (g_state.enemyProjectiles[i].y - g_state.shipY) * (g_state.enemyProjectiles[i].y - g_state.shipY));
            if (d < 280.0f) { foundThreat = 1; break; }
        }
        if (!foundThreat) {
            for (int i = 0; i < MAX_RAIDERS; i++) {
                if (!g_state.raiders[i].active) continue;
                float d = (float)sqrt((g_state.raiders[i].x - g_state.shipX) * (g_state.raiders[i].x - g_state.shipX) +
                                      (g_state.raiders[i].y - g_state.shipY) * (g_state.raiders[i].y - g_state.shipY));
                if (d < 240.0f) { foundThreat = 1; break; }
            }
        }
        if (foundThreat) FirePDL();
    }

    // Update Player Projectiles
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        Projectile* p = &g_state.projectiles[i];
        if (!p->active) continue;
        p->life -= dt;
        if (p->life <= 0.0f) {
            if (p->type == 1) TriggerEMPShockwave(p->x, p->y); // Flak detonation
            p->active = 0;
            continue;
        }

        if (p->type == 0 || p->type == 1) { // Railgun slug or flak shell
            p->x += p->vx;
            p->y += p->vy;

            // Check collision with raiders
            for (int r = 0; r < MAX_RAIDERS; r++) {
                Raider* rd = &g_state.raiders[r];
                if (!rd->active) continue;
                float d = (float)sqrt((rd->x - p->x) * (rd->x - p->x) + (rd->y - p->y) * (rd->y - p->y));
                if (d < rd->radius + 6.0f) {
                    if (p->type == 0) { // Railgun slug impact
                        float dmg = p->damage;
                        if (rd->shield > 0.0f) {
                            float shAbs = min(rd->shield, dmg);
                            rd->shield -= shAbs;
                            dmg -= shAbs;
                        }
                        if (dmg > 0.0f) rd->hp = max(0.0f, rd->hp - dmg);
                        AddSparks(p->x, p->y, RGB(0, 240, 255), 14);
                        char dTxt[32];
                        sprintf(dTxt, "-%.0f HP", p->damage);
                        AddFloatingText(dTxt, rd->x, rd->y - 20.0f, RGB(56, 189, 248));
                        p->active = 0;
                    } else if (p->type == 1) { // Flak impact
                        TriggerEMPShockwave(p->x, p->y);
                        p->active = 0;
                    }
                    break;
                }
            }
        }
    }

    // Update Enemy Projectiles (Blasters & Torpedoes)
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        EnemyProjectile* ep = &g_state.enemyProjectiles[i];
        if (!ep->active) continue;
        ep->life -= dt;
        if (ep->life <= 0.0f) { ep->active = 0; continue; }

        if (ep->type == 1 && ep->homing) { // Homing Torpedo tracking ship
            float targetAngle = (float)atan2(g_state.shipY - ep->y, g_state.shipX - ep->x);
            float curAngle = (float)atan2(ep->vy, ep->vx);
            float diff = targetAngle - curAngle;
            while (diff > 3.14159f) diff -= 6.28318f;
            while (diff < -3.14159f) diff += 6.28318f;
            float newAngle = curAngle + diff * 0.08f;
            float spd = 3.6f;
            ep->vx = (float)cos(newAngle) * spd;
            ep->vy = (float)sin(newAngle) * spd;

            if (rand() % 4 == 0) {
                for (int pt = 0; pt < MAX_PARTICLES; pt++) {
                    if (!g_state.particles[pt].active) {
                        g_state.particles[pt].x = ep->x;
                        g_state.particles[pt].y = ep->y;
                        g_state.particles[pt].vx = -ep->vx * 0.3f;
                        g_state.particles[pt].vy = -ep->vy * 0.3f;
                        g_state.particles[pt].color = RGB(245, 158, 11);
                        g_state.particles[pt].life = 0.5f;
                        g_state.particles[pt].decay = 0.05f;
                        g_state.particles[pt].size = 2.0f;
                        g_state.particles[pt].active = 1;
                        break;
                    }
                }
            }
        }

        ep->x += ep->vx;
        ep->y += ep->vy;

        // Collision with player ship
        float distToShip = (float)sqrt((g_state.shipX - ep->x) * (g_state.shipX - ep->x) +
                                      (g_state.shipY - ep->y) * (g_state.shipY - ep->y));
        if (distToShip < 22.0f) {
            float dmg = ep->damage;
            if (g_state.shield > 0.0f) {
                float shAbs = min(g_state.shield, dmg);
                g_state.shield -= shAbs;
                dmg -= shAbs;
            }
            if (dmg > 0.0f) {
                g_state.hull = max(0.0f, g_state.hull - dmg);
                if (ep->type == 1 && g_state.breachCount < MAX_COMPARTMENTS && (rand() % 100 < 40)) {
                    int comp = rand() % MAX_COMPARTMENTS;
                    if (!g_state.hullBreaches[comp]) {
                        g_state.hullBreaches[comp] = 1;
                        g_state.breachCount++;
                        TriggerSound(SFX_ALARM);
                        char bLog[64];
                        sprintf(bLog, "CRITICAL: Torpedo impact breached %s!", COMPARTMENT_DEFS[comp].name);
                        AddLog(bLog, 4);
                        AddFloatingText("HULL BREACH!", g_state.shipX, g_state.shipY - 30.0f, RGB(239, 68, 68));
                    }
                }
            }
            TriggerSound(SFX_FRACTURE);
            AddSparks(ep->x, ep->y, RGB(239, 68, 68), 16);
            char hTxt[32];
            sprintf(hTxt, "-%.0f HP", ep->damage);
            AddFloatingText(hTxt, g_state.shipX, g_state.shipY - 25.0f, RGB(239, 68, 68));
            ep->active = 0;
        }
    }

    // Update Shockwaves
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        Shockwave* sw = &g_state.shockwaves[i];
        if (!sw->active) continue;
        sw->r += dt * 380.0f;
        sw->life -= dt * 1.5f;
        if (sw->life <= 0.0f || sw->r >= sw->maxR) sw->active = 0;
    }

    // Update Pirate Wrecks
    for (int i = 0; i < MAX_PIRATE_WRECKS; i++) {
        PirateWreck* pw = &g_state.pirateWrecks[i];
        if (!pw->active) continue;
        pw->life -= dt;
        if (pw->life <= 0.0f) { pw->active = 0; continue; }

        float d = (float)sqrt((pw->x - g_state.shipX) * (pw->x - g_state.shipX) + (pw->y - g_state.shipY) * (pw->y - g_state.shipY));
        if (d < 45.0f) {
            int scrapGet = 3 + (rand() % 4);
            g_state.cargoHold[5] += scrapGet;
            UpdateCargoTotal();
            TriggerSound(SFX_COLLECT);
            char bLg[64];
            sprintf(bLg, "SALVAGE: Stripped %dT scrap from destroyed pirate wreck.", scrapGet);
            AddLog(bLg, 5);
            char fT[32];
            sprintf(fT, "+%dT SCRAP", scrapGet);
            AddFloatingText(fT, g_state.shipX, g_state.shipY - 30.0f, RGB(251, 191, 36));
            pw->active = 0;
        }
    }

    // Update Raider AI
    for (int i = 0; i < MAX_RAIDERS; i++) {
        Raider* r = &g_state.raiders[i];
        if (!r->active) continue;

        if (r->stunTimer > 0.0f) {
            r->stunTimer -= dt;
            continue;
        }

        // Raider AI Movement: Circle and strafe towards player
        float dx = g_state.shipX - r->x;
        float dy = g_state.shipY - r->y;
        float dist = (float)sqrt(dx * dx + dy * dy);
        r->angle = (float)atan2(dy, dx);

        float desiredDist = (r->type == 0) ? 140.0f : ((r->type == 1) ? 220.0f : 300.0f);
        float moveDirX = dx / max(1.0f, dist);
        float moveDirY = dy / max(1.0f, dist);

        if (dist > desiredDist + 30.0f) {
            r->vx += moveDirX * r->speed * 0.04f;
            r->vy += moveDirY * r->speed * 0.04f;
        } else if (dist < desiredDist - 30.0f) {
            r->vx -= moveDirX * r->speed * 0.04f;
            r->vy -= moveDirY * r->speed * 0.04f;
        }

        // Lateral strafe
        float strafeX = -moveDirY;
        float strafeY = moveDirX;
        r->vx += strafeX * r->speed * 0.02f;
        r->vy += strafeY * r->speed * 0.02f;

        r->vx *= 0.94f;
        r->vy *= 0.94f;
        r->x += r->vx;
        r->y += r->vy;

        // Raider Shooting
        r->shootTimer -= dt;
        if (r->shootTimer <= 0.0f && dist < 420.0f) {
            if (r->type == 0) { // Corsair: dual rapid blaster
                r->shootTimer = 1.8f + (((float)rand() / (float)RAND_MAX) * 1.0f);
                for (int ep = 0; ep < MAX_ENEMY_PROJECTILES; ep++) {
                    if (!g_state.enemyProjectiles[ep].active) {
                        g_state.enemyProjectiles[ep].type = 0;
                        g_state.enemyProjectiles[ep].x = r->x;
                        g_state.enemyProjectiles[ep].y = r->y;
                        g_state.enemyProjectiles[ep].vx = (float)cos(r->angle) * 7.5f;
                        g_state.enemyProjectiles[ep].vy = (float)sin(r->angle) * 7.5f;
                        g_state.enemyProjectiles[ep].damage = 14.0f;
                        g_state.enemyProjectiles[ep].life = 1.4f;
                        g_state.enemyProjectiles[ep].color = RGB(239, 68, 68);
                        g_state.enemyProjectiles[ep].active = 1;
                        break;
                    }
                }
            } else if (r->type == 1) { // Gunship: Heavy Blaster + Homing Torpedo
                r->shootTimer = 2.4f + (((float)rand() / (float)RAND_MAX) * 1.5f);
                int launchTorpedo = (rand() % 100 < 50);
                for (int ep = 0; ep < MAX_ENEMY_PROJECTILES; ep++) {
                    if (!g_state.enemyProjectiles[ep].active) {
                        g_state.enemyProjectiles[ep].type = launchTorpedo ? 1 : 0;
                        g_state.enemyProjectiles[ep].x = r->x;
                        g_state.enemyProjectiles[ep].y = r->y;
                        g_state.enemyProjectiles[ep].vx = (float)cos(r->angle) * (launchTorpedo ? 3.5f : 6.8f);
                        g_state.enemyProjectiles[ep].vy = (float)sin(r->angle) * (launchTorpedo ? 3.5f : 6.8f);
                        g_state.enemyProjectiles[ep].homing = launchTorpedo;
                        g_state.enemyProjectiles[ep].hp = 30.0f;
                        g_state.enemyProjectiles[ep].damage = launchTorpedo ? 40.0f : 22.0f;
                        g_state.enemyProjectiles[ep].life = launchTorpedo ? 4.5f : 1.5f;
                        g_state.enemyProjectiles[ep].color = launchTorpedo ? RGB(245, 158, 11) : RGB(239, 68, 68);
                        g_state.enemyProjectiles[ep].active = 1;
                        if (launchTorpedo) {
                            TriggerSound(SFX_TORPEDO_ALERT);
                            AddLog("TORPEDO ALERT: Raider gunship launched homing torpedo! Deploy Chaff or use PDL.", 4);
                        }
                        break;
                    }
                }
            } else { // Dread Raider: Heavy Barrage
                r->shootTimer = 2.0f + (((float)rand() / (float)RAND_MAX) * 1.2f);
                for (int ep = 0; ep < MAX_ENEMY_PROJECTILES; ep++) {
                    if (!g_state.enemyProjectiles[ep].active) {
                        g_state.enemyProjectiles[ep].type = 1; // Homing Torpedo
                        g_state.enemyProjectiles[ep].x = r->x;
                        g_state.enemyProjectiles[ep].y = r->y;
                        g_state.enemyProjectiles[ep].vx = (float)cos(r->angle) * 3.2f;
                        g_state.enemyProjectiles[ep].vy = (float)sin(r->angle) * 3.2f;
                        g_state.enemyProjectiles[ep].homing = 1;
                        g_state.enemyProjectiles[ep].hp = 45.0f;
                        g_state.enemyProjectiles[ep].damage = 55.0f;
                        g_state.enemyProjectiles[ep].life = 5.0f;
                        g_state.enemyProjectiles[ep].color = RGB(244, 63, 94);
                        g_state.enemyProjectiles[ep].active = 1;
                        TriggerSound(SFX_TORPEDO_ALERT);
                        AddLog("DREAD TORPEDO DETECTED: Heavy homing warhead locked onto our reactor.", 4);
                        break;
                    }
                }
            }
        }

        // Raider Destruction Check
        if (r->hp <= 0.0f) {
            r->active = 0;
            g_state.piratesDefeated++;
            g_state.bountiesClaimed += r->bounty;
            g_state.credits += r->bounty;
            TriggerSound(SFX_PIRATE_EXPLODE);
            AddSparks(r->x, r->y, RGB(245, 158, 11), 35);

            // Leave salvageable pirate wreck
            for (int pw = 0; pw < MAX_PIRATE_WRECKS; pw++) {
                if (!g_state.pirateWrecks[pw].active) {
                    g_state.pirateWrecks[pw].x = r->x;
                    g_state.pirateWrecks[pw].y = r->y;
                    g_state.pirateWrecks[pw].angle = r->angle;
                    g_state.pirateWrecks[pw].type = r->type;
                    g_state.pirateWrecks[pw].life = 45.0f;
                    g_state.pirateWrecks[pw].active = 1;
                    break;
                }
            }

            char bLog[128];
            sprintf(bLog, "THREAT ELIMINATED: %s destroyed! Bounty Claimed: +%d CR.", r->name, r->bounty);
            AddLog(bLog, 5);
            char bTxt[48];
            sprintf(bTxt, "DESTROYED! +%d CR", r->bounty);
            AddFloatingText(bTxt, r->x, r->y - 25.0f, RGB(16, 185, 129));
        }
    }
    
    // Station Rotation & Docking Dynamics
    g_state.stationAngle += g_state.stationRotSpeed;
    if (g_state.stationAngle > 6.28318f) g_state.stationAngle -= 6.28318f;
    g_state.stationBeaconTimer += dt * 3.0f;
    
    if (g_state.stationDocked) {
        // Tether ship to station mooring point
        float targetX = g_state.stationX + (float)cos(g_state.stationAngle) * 80.0f;
        float targetY = g_state.stationY + (float)sin(g_state.stationAngle) * 80.0f;
        g_state.shipX += (targetX - g_state.shipX) * 0.12f;
        g_state.shipY += (targetY - g_state.shipY) * 0.12f;
        g_state.shipVx = 0.0f;
        g_state.shipVy = 0.0f;
        
        // Passive trickle repair and shield charge when docked
        g_state.shield = min(g_state.maxShield, g_state.shield + 2.0f * dt);
        g_state.heat = max(0.0f, g_state.heat - 5.0f * dt);
    }
    
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
    
    // Derelict Proximity Check
    float minDerDist = 550.0f;
    int bestDerIdx = -1;
    for (int i = 0; i < MAX_DERELICTS; i++) {
        if (!g_state.derelicts[i].active) continue;
        float d = (float)sqrt((g_state.derelicts[i].x - g_state.shipX) * (g_state.derelicts[i].x - g_state.shipX) +
                              (g_state.derelicts[i].y - g_state.shipY) * (g_state.derelicts[i].y - g_state.shipY));
        if (d < minDerDist) {
            minDerDist = d;
            bestDerIdx = i;
        }
    }
    g_state.selectedDerelictIndex = bestDerIdx;
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
    COLORREF o2Col = (g_state.o2 < 25.0f) ? RGB(239, 68, 68) : (g_state.o2 < 50.0f ? RGB(245, 158, 11) : RGB(16, 185, 129));
    DrawBar(hdc, 10, my + 14, meterW, 8, g_state.o2 / 100.0f, o2Col, RGB(2, 6, 23), pal->borderPanel);
    my += 26;
    
    // Radiation Dosimeter
    SetTextColor(hdc, pal->textPrimary);
    TextOutA(hdc, 10, my, "CREW DOSIMETER", 14);
    sprintf(valBuf, "%d RADS", (int)g_state.radiation);
    TextOutA(hdc, leftPanelW - 65, my, valBuf, (int)strlen(valBuf));
    COLORREF radCol = (g_state.radiation > 500.0f) ? RGB(239, 68, 68) : (g_state.radiation > 200.0f ? RGB(245, 158, 11) : RGB(16, 185, 129));
    DrawBar(hdc, 10, my + 14, meterW, 8, g_state.radiation / 1000.0f, radCol, RGB(2, 6, 23), pal->borderPanel);
    my += 26;
    
    // O2 Scrubber
    SetTextColor(hdc, pal->textPrimary);
    TextOutA(hdc, 10, my, "O2 SCRUBBERS", 12);
    sprintf(valBuf, "%d%%", (int)g_state.o2Scrubber);
    TextOutA(hdc, leftPanelW - 45, my, valBuf, (int)strlen(valBuf));
    COLORREF scCol = (g_state.o2Scrubber < 30.0f) ? RGB(239, 68, 68) : RGB(56, 189, 248);
    DrawBar(hdc, 10, my + 14, meterW, 8, g_state.o2Scrubber / 100.0f, scCol, RGB(2, 6, 23), pal->borderPanel);
    my += 30;
    
    // System Status Summary Box
    RECT rcSysBox = { 10, my, leftPanelW - 10, my + 86 };
    HBRUSH hBrBox = CreateSolidBrush(RGB(3, 7, 18));
    FillRect(hdc, &rcSysBox, hBrBox);
    DeleteObject(hBrBox);
    FrameRect(hdc, &rcSysBox, (HBRUSH)GetStockObject(WHITE_BRUSH));
    
    SetTextColor(hdc, RGB(148, 163, 184));
    TextOutA(hdc, 14, my + 4, "HULL BREACHES:", 14);
    SetTextColor(hdc, g_state.breachCount > 0 ? RGB(239, 68, 68) : RGB(16, 185, 129));
    char brBuf[32];
    if (g_state.breachCount > 0) sprintf(brBuf, "%d ACTIVE [!]", g_state.breachCount);
    else sprintf(brBuf, "AIRTIGHT");
    TextOutA(hdc, leftPanelW - 85, my + 4, brBuf, (int)strlen(brBuf));
    
    SetTextColor(hdc, RGB(148, 163, 184));
    TextOutA(hdc, 14, my + 20, "PLASMA CONDUIT:", 15);
    SetTextColor(hdc, g_state.plasmaLeaks ? RGB(244, 63, 94) : RGB(16, 185, 129));
    TextOutA(hdc, leftPanelW - 85, my + 20, g_state.plasmaLeaks ? "RUPTURE" : "NOMINAL", g_state.plasmaLeaks ? 7 : 7);
    
    SetTextColor(hdc, RGB(148, 163, 184));
    TextOutA(hdc, 14, my + 36, "MINING LASER:", 13);
    SetTextColor(hdc, g_state.laserOverheated ? RGB(239, 68, 68) : (g_state.miningActive ? pal->vector : pal->textDim));
    TextOutA(hdc, leftPanelW - 85, my + 36, g_state.laserOverheated ? "OVERHEAT" : (g_state.miningActive ? "FIRING" : "STANDBY"), g_state.laserOverheated ? 8 : (g_state.miningActive ? 6 : 7));
    
    SetTextColor(hdc, RGB(148, 163, 184));
    TextOutA(hdc, 14, my + 52, "TRACTOR BEAM:", 13);
    SetTextColor(hdc, g_state.tractorActive ? pal->vector : pal->textDim);
    TextOutA(hdc, leftPanelW - 85, my + 52, g_state.tractorActive ? "ENGAGED" : "STANDBY", g_state.tractorActive ? 7 : 7);
    
    float drain = 12.0f + (g_state.miningActive ? 28.0f : 0.0f) + (g_state.tractorActive ? 14.0f : 0.0f);
    SetTextColor(hdc, RGB(148, 163, 184));
    TextOutA(hdc, 14, my + 68, "GRID DRAIN:", 11);
    char drnBuf[32];
    sprintf(drnBuf, "%.1f MW", drain);
    SetTextColor(hdc, pal->textBright);
    TextOutA(hdc, leftPanelW - 65, my + 68, drnBuf, (int)strlen(drnBuf));
    
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
    
    // Draw Scan Waves
    for (int i = 0; i < MAX_SCAN_WAVES; i++) {
        if (!g_state.scanWaves[i].active) continue;
        int wx = cx + (int)(g_state.scanWaves[i].x - g_state.shipX);
        int wy = cyCenter + (int)(g_state.scanWaves[i].y - g_state.shipY);
        int rad = (int)g_state.scanWaves[i].r;
        HPEN hPenWave = CreatePen(PS_SOLID, 2, g_state.scanWaves[i].color);
        HGDIOBJ oldWvPen = SelectObject(hdc, hPenWave);
        HGDIOBJ oldWvBr = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Ellipse(hdc, wx - rad, wy - rad, wx + rad, wy + rad);
        SelectObject(hdc, oldWvPen);
        SelectObject(hdc, oldWvBr);
        DeleteObject(hPenWave);
    }
    
    // Draw Orbital Spaceport & Trade Station
    {
        int stX = cx + (int)(g_state.stationX - g_state.shipX);
        int stY = cyCenter + (int)(g_state.stationY - g_state.shipY);
        int stR = (int)g_state.stationRadius;
        
        if (stX >= viewportX - 150 && stX <= viewportX + viewportW + 150 &&
            stY >= viewportY - 150 && stY <= viewportY + viewportH + 150) {
            
            // Outer rotating ring (Dotted / segmented)
            HPEN hPenStRing = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
            HBRUSH hBrStHub = CreateSolidBrush(RGB(10, 20, 45));
            HGDIOBJ oldStP = SelectObject(hdc, hPenStRing);
            HGDIOBJ oldStB = SelectObject(hdc, hBrStHub);
            
            // Station central hub
            Ellipse(hdc, stX - 26, stY - 26, stX + 26, stY + 26);
            
            // Outer Torus Ring
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Ellipse(hdc, stX - stR, stY - stR, stX + stR, stY + stR);
            Ellipse(hdc, stX - stR + 12, stY - stR + 12, stX + stR - 12, stY + stR - 12);
            
            // 4 Rotating Radial Spokes
            HPEN hPenSpoke = CreatePen(PS_SOLID, 1, RGB(14, 165, 233));
            SelectObject(hdc, hPenSpoke);
            for (int sp = 0; sp < 4; sp++) {
                float a = g_state.stationAngle + ((float)sp * 1.57079f);
                int sx1 = stX + (int)(cos(a) * 26.0f);
                int sy1 = stY + (int)(sin(a) * 26.0f);
                int sx2 = stX + (int)(cos(a) * stR);
                int sy2 = stY + (int)(sin(a) * stR);
                MoveToEx(hdc, sx1, sy1, NULL);
                LineTo(hdc, sx2, sy2);
            }
            DeleteObject(hPenSpoke);
            
            // Mooring Dock Bay Indicator
            float dockA = g_state.stationAngle;
            int dkX = stX + (int)(cos(dockA) * (stR - 6));
            int dkY = stY + (int)(sin(dockA) * (stR - 6));
            HBRUSH hBrDockBeacon = CreateSolidBrush(g_state.stationDocked ? RGB(16, 185, 129) : RGB(251, 191, 36));
            RECT rcDkBc = { dkX - 4, dkY - 4, dkX + 5, dkY + 5 };
            FillRect(hdc, &rcDkBc, hBrDockBeacon);
            DeleteObject(hBrDockBeacon);
            
            // Docking Mooring Tether
            if (g_state.stationDocked) {
                HPEN hPenTether = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
                SelectObject(hdc, hPenTether);
                MoveToEx(hdc, cx, cyCenter, NULL);
                LineTo(hdc, dkX, dkY);
                DeleteObject(hPenTether);
            }
            
            // Pulsing Navigation Beacons (4 cardinal lights on outer ring)
            int beaconOn = ((int)g_state.stationBeaconTimer % 2 == 0);
            HBRUSH hBrBeacon = CreateSolidBrush(beaconOn ? RGB(239, 68, 68) : RGB(30, 58, 138));
            for (int b = 0; b < 4; b++) {
                float ba = g_state.stationAngle + ((float)b * 1.57079f) + 0.78539f;
                int bx = stX + (int)(cos(ba) * stR);
                int by = stY + (int)(sin(ba) * stR);
                RECT rcBc = { bx - 3, by - 3, bx + 4, by + 4 };
                FillRect(hdc, &rcBc, hBrBeacon);
            }
            DeleteObject(hBrBeacon);
            
            SelectObject(hdc, oldStP);
            SelectObject(hdc, oldStB);
            DeleteObject(hPenStRing);
            DeleteObject(hBrStHub);
            
            // Station Name Tag
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, RGB(56, 189, 248));
            const StationDef* curStDef = &STATION_DEFS[g_state.currentSectorIndex];
            char stLbl[80];
            sprintf(stLbl, "✦ %s [%s]", curStDef->name, g_state.stationDocked ? "DOCKED" : "ORBITAL PORT");
            RECT rcStLbl = { stX - 140, stY - stR - 18, stX + 140, stY - stR };
            DrawTextA(hdc, stLbl, -1, &rcStLbl, DT_CENTER | DT_SINGLELINE);
        }
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
        
        // Scanned Asteroid Halo
        if (ast->scanned) {
            HPEN hPenScHalo = CreatePen(PS_DOT, 1, RGB(56, 189, 248));
            HGDIOBJ oldScPen = SelectObject(hdc, hPenScHalo);
            HGDIOBJ oldScBr = SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Ellipse(hdc, ax - (int)ast->radius - 4, ay - (int)ast->radius - 4, ax + (int)ast->radius + 4, ay + (int)ast->radius + 4);
            SelectObject(hdc, oldScPen);
            SelectObject(hdc, oldScBr);
            DeleteObject(hPenScHalo);
        }
        
        // Asteroid Label
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(148, 163, 184));
        char lbl[32];
        sprintf(lbl, "%s [%s]", ast->id, ORE_DEFS[ast->oreType].name);
        RECT rcLbl = { ax - 60, ay - (int)ast->radius - 14, ax + 60, ay - (int)ast->radius };
        DrawTextA(hdc, lbl, -1, &rcLbl, DT_CENTER | DT_SINGLELINE);
        
        if (ast->scanned) {
            if (ast->geodeType == 2) {
                SetTextColor(hdc, RGB(244, 63, 94));
                TextOutA(hdc, ax - 45, ay + (int)ast->radius + 2, "[DARK GEODE]", 12);
            } else if (ast->geodeType == 1) {
                SetTextColor(hdc, RGB(192, 132, 252));
                TextOutA(hdc, ax - 48, ay + (int)ast->radius + 2, "[QUARTZ GEODE]", 14);
            } else if (ast->geodeType == 3) {
                SetTextColor(hdc, RGB(224, 242, 254));
                TextOutA(hdc, ax - 40, ay + (int)ast->radius + 2, "[PT POCKET]", 11);
            }
            if (ast->resonantLock) {
                SetTextColor(hdc, RGB(245, 158, 11));
                TextOutA(hdc, ax - 50, ay + (int)ast->radius + 14, "[RESONANT +50%]", 15);
            }
        }
        
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
    
    // Draw Derelict Spaceships
    for (int i = 0; i < MAX_DERELICTS; i++) {
        Derelict* d = &g_state.derelicts[i];
        if (!d->active) continue;
        
        int dx = cx + (int)(d->x - g_state.shipX);
        int dy = cyCenter + (int)(d->y - g_state.shipY);
        
        if (dx < viewportX - 120 || dx > viewportX + viewportW + 120 ||
            dy < viewportY - 120 || dy > viewportY + viewportH + 120) continue;
            
        float cosD = (float)cos(d->rot);
        float sinD = (float)sin(d->rot);
        
        POINT ptsDerLocal[8] = {
            { (int)(d->length * 0.5f), 0 },
            { (int)(d->length * 0.2f), (int)(d->width * 0.5f) },
            { (int)(-d->length * 0.3f), (int)(d->width * 0.45f) },
            { (int)(-d->length * 0.5f), (int)(d->width * 0.3f) },
            { (int)(-d->length * 0.4f), 0 },
            { (int)(-d->length * 0.5f), (int)(-d->width * 0.3f) },
            { (int)(-d->length * 0.3f), (int)(-d->width * 0.45f) },
            { (int)(d->length * 0.2f), (int)(-d->width * 0.5f) }
        };
        
        POINT ptsDerWorld[8];
        for (int v = 0; v < 8; v++) {
            ptsDerWorld[v].x = dx + (int)(ptsDerLocal[v].x * cosD - ptsDerLocal[v].y * sinD);
            ptsDerWorld[v].y = dy + (int)(ptsDerLocal[v].x * sinD + ptsDerLocal[v].y * cosD);
        }
        
        int isSelectedDer = (g_state.selectedDerelictIndex == i);
        HPEN hPenDer = CreatePen(PS_SOLID, isSelectedDer ? 2 : 1, isSelectedDer ? RGB(245, 158, 11) : d->hullColor);
        HBRUSH hBrDer = CreateSolidBrush(RGB(15, 23, 42));
        HGDIOBJ oldDerPen = SelectObject(hdc, hPenDer);
        HGDIOBJ oldDerBr = SelectObject(hdc, hBrDer);
        
        Polygon(hdc, ptsDerWorld, 8);
        
        // Internal structural rib / bulkhead
        MoveToEx(hdc, ptsDerWorld[1].x, ptsDerWorld[1].y, NULL);
        LineTo(hdc, ptsDerWorld[7].x, ptsDerWorld[7].y);
        MoveToEx(hdc, ptsDerWorld[2].x, ptsDerWorld[2].y, NULL);
        LineTo(hdc, ptsDerWorld[6].x, ptsDerWorld[6].y);
        
        // Airlock Door indicator
        int alX = dx + (int)(4.0f * cosD);
        int alY = dy + (int)(4.0f * sinD);
        HBRUSH hBrAl = CreateSolidBrush(d->airlockBreached ? RGB(16, 185, 129) : RGB(239, 68, 68));
        RECT rcAl = { alX - 3, alY - 3, alX + 4, alY + 4 };
        FillRect(hdc, &rcAl, hBrAl);
        DeleteObject(hBrAl);
        
        // Blinking Beacon
        int bcX = dx - (int)(d->length * 0.45f * cosD);
        int bcY = dy - (int)(d->length * 0.45f * sinD);
        HBRUSH hBrBc = CreateSolidBrush((rand() % 4 == 0) ? RGB(255, 255, 255) : RGB(239, 68, 68));
        RECT rcBc = { bcX - 2, bcY - 2, bcX + 3, bcY + 3 };
        FillRect(hdc, &rcBc, hBrBc);
        DeleteObject(hBrBc);
        
        SelectObject(hdc, oldDerPen);
        SelectObject(hdc, oldDerBr);
        DeleteObject(hPenDer);
        DeleteObject(hBrDer);
        
        // Derelict Label
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(251, 191, 36));
        char derLbl[64];
        sprintf(derLbl, "✦ %s [%s]", d->id, d->airlockBreached ? "BREACHED" : "SEALED");
        RECT rcDerLbl = { dx - 90, dy - (int)(d->width * 0.5f) - 16, dx + 90, dy - (int)(d->width * 0.5f) };
        DrawTextA(hdc, derLbl, -1, &rcDerLbl, DT_CENTER | DT_SINGLELINE);
        
        if (isSelectedDer) {
            HPEN hPenTarget = CreatePen(PS_DOT, 1, RGB(245, 158, 11));
            HGDIOBJ oldTPen = SelectObject(hdc, hPenTarget);
            HGDIOBJ oldTBr = SelectObject(hdc, GetStockObject(NULL_BRUSH));
            int bSize = (int)(d->length * 0.5f) + 6;
            Rectangle(hdc, dx - bSize, dy - bSize, dx + bSize, dy + bSize);
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
    
    // --- Phase 12: Draw Pirate Raider Ships ---
    for (int i = 0; i < MAX_RAIDERS; i++) {
        Raider* r = &g_state.raiders[i];
        if (!r->active) continue;

        int rx = cx + (int)(r->x - g_state.shipX);
        int ry = cyCenter + (int)(r->y - g_state.shipY);
        if (rx < viewportX - 80 || rx > viewportX + viewportW + 80 ||
            ry < viewportY - 80 || ry > viewportY + viewportH + 80) continue;

        float cosR = (float)cos(r->angle);
        float sinR = (float)sin(r->angle);

        // Raider Shield Bubble
        if (r->shield > 0.0f) {
            HPEN hPenRSh = CreatePen(PS_SOLID, 1, RGB(56, 189, 248));
            HGDIOBJ oldRShP = SelectObject(hdc, hPenRSh);
            HGDIOBJ oldRShB = SelectObject(hdc, GetStockObject(NULL_BRUSH));
            int shRad = (int)r->radius + 4;
            Ellipse(hdc, rx - shRad, ry - shRad, rx + shRad, ry + shRad);
            SelectObject(hdc, oldRShP);
            SelectObject(hdc, oldRShB);
            DeleteObject(hPenRSh);
        }

        // Raider Hull Geometry
        POINT ptsRLocal[5] = {
            { (int)(r->radius), 0 },
            { (int)(-r->radius * 0.8f), (int)(r->radius * 0.7f) },
            { (int)(-r->radius * 0.4f), 0 },
            { (int)(-r->radius * 0.8f), (int)(-r->radius * 0.7f) },
            { (int)(r->radius), 0 }
        };
        POINT ptsRWorld[5];
        for (int v = 0; v < 5; v++) {
            ptsRWorld[v].x = rx + (int)(ptsRLocal[v].x * cosR - ptsRLocal[v].y * sinR);
            ptsRWorld[v].y = ry + (int)(ptsRLocal[v].x * sinR + ptsRLocal[v].y * cosR);
        }

        HPEN hPenRHull = CreatePen(PS_SOLID, 2, r->stunTimer > 0.0f ? RGB(192, 132, 252) : r->color);
        HBRUSH hBrRHull = CreateSolidBrush(RGB(20, 5, 10));
        HGDIOBJ oldRP = SelectObject(hdc, hPenRHull);
        HGDIOBJ oldRB = SelectObject(hdc, hBrRHull);
        Polygon(hdc, ptsRWorld, 5);
        SelectObject(hdc, oldRP);
        SelectObject(hdc, oldRB);
        DeleteObject(hPenRHull);
        DeleteObject(hBrRHull);

        // Raider Name & Health Bar
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, r->color);
        char rLbl[64];
        sprintf(rLbl, "%s [HP: %d/%d]", r->id, (int)r->hp, (int)r->maxHp);
        RECT rcRLbl = { rx - 70, ry - (int)r->radius - 16, rx + 70, ry - (int)r->radius };
        DrawTextA(hdc, rLbl, -1, &rcRLbl, DT_CENTER | DT_SINGLELINE);

        int hbW = (int)(r->radius * 1.8f);
        int hbH = 4;
        DrawBar(hdc, rx - hbW / 2, ry + (int)r->radius + 4, hbW, hbH, r->hp / r->maxHp, r->color, RGB(2, 6, 23), RGB(60, 20, 20));
    }

    // --- Phase 12: Draw Pirate Wrecks ---
    for (int i = 0; i < MAX_PIRATE_WRECKS; i++) {
        PirateWreck* pw = &g_state.pirateWrecks[i];
        if (!pw->active) continue;
        int wx = cx + (int)(pw->x - g_state.shipX);
        int wy = cyCenter + (int)(pw->y - g_state.shipY);
        if (wx < viewportX || wx > viewportX + viewportW || wy < viewportY || wy > viewportY + viewportH) continue;

        HPEN hPenWreck = CreatePen(PS_DOT, 1, RGB(251, 191, 36));
        HGDIOBJ oldWP = SelectObject(hdc, hPenWreck);
        HGDIOBJ oldWB = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, wx - 10, wy - 10, wx + 10, wy + 10);
        MoveToEx(hdc, wx - 10, wy - 10, NULL); LineTo(hdc, wx + 10, wy + 10);
        MoveToEx(hdc, wx + 10, wy - 10, NULL); LineTo(hdc, wx - 10, wy + 10);
        SelectObject(hdc, oldWP);
        SelectObject(hdc, oldWB);
        DeleteObject(hPenWreck);

        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(251, 191, 36));
        RECT rcWkLbl = { wx - 50, wy - 20, wx + 50, wy - 6 };
        DrawTextA(hdc, "☠ WRECKAGE", -1, &rcWkLbl, DT_CENTER | DT_SINGLELINE);
    }

    // --- Phase 12: Draw Shockwaves ---
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        Shockwave* sw = &g_state.shockwaves[i];
        if (!sw->active) continue;
        int sx = cx + (int)(sw->x - g_state.shipX);
        int sy = cyCenter + (int)(sw->y - g_state.shipY);
        int rad = (int)sw->r;
        HPEN hPenSw = CreatePen(PS_SOLID, 2, sw->color);
        HGDIOBJ oldSwP = SelectObject(hdc, hPenSw);
        HGDIOBJ oldSwB = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Ellipse(hdc, sx - rad, sy - rad, sx + rad, sy + rad);
        SelectObject(hdc, oldSwP);
        SelectObject(hdc, oldSwB);
        DeleteObject(hPenSw);
    }

    // --- Phase 12: Draw Player Projectiles (Railgun slugs, Flak shells, PDL beams) ---
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        Projectile* p = &g_state.projectiles[i];
        if (!p->active) continue;
        if (p->type == 0) { // Railgun slug
            int px = cx + (int)(p->x - g_state.shipX);
            int py = cyCenter + (int)(p->y - g_state.shipY);
            HPEN hPenSlug = CreatePen(PS_SOLID, 2, p->color);
            HGDIOBJ oldSlgP = SelectObject(hdc, hPenSlug);
            MoveToEx(hdc, px - (int)(p->vx * 1.5f), py - (int)(p->vy * 1.5f), NULL);
            LineTo(hdc, px, py);
            SelectObject(hdc, oldSlgP);
            DeleteObject(hPenSlug);
        } else if (p->type == 1) { // Flak shell
            int px = cx + (int)(p->x - g_state.shipX);
            int py = cyCenter + (int)(p->y - g_state.shipY);
            HBRUSH hBrFlak = CreateSolidBrush(p->color);
            RECT rcFlk = { px - 3, py - 3, px + 4, py + 4 };
            FillRect(hdc, &rcFlk, hBrFlak);
            DeleteObject(hBrFlak);
        } else if (p->type == 2) { // PDL beam
            int x1 = cx + (int)(p->x - g_state.shipX);
            int y1 = cyCenter + (int)(p->y - g_state.shipY);
            int x2 = cx + (int)(p->x2 - g_state.shipX);
            int y2 = cyCenter + (int)(p->y2 - g_state.shipY);
            HPEN hPenBeam = CreatePen(PS_SOLID, 2, p->color);
            HGDIOBJ oldBmP = SelectObject(hdc, hPenBeam);
            MoveToEx(hdc, x1, y1, NULL);
            LineTo(hdc, x2, y2);
            SelectObject(hdc, oldBmP);
            DeleteObject(hPenBeam);
        }
    }

    // --- Phase 12: Draw Enemy Projectiles (Hostile Blasters & Homing Torpedoes) ---
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        EnemyProjectile* ep = &g_state.enemyProjectiles[i];
        if (!ep->active) continue;
        int epx = cx + (int)(ep->x - g_state.shipX);
        int epy = cyCenter + (int)(ep->y - g_state.shipY);
        if (ep->type == 0) { // Raider blaster
            HPEN hPenBlst = CreatePen(PS_SOLID, 2, ep->color);
            HGDIOBJ oldBstP = SelectObject(hdc, hPenBlst);
            MoveToEx(hdc, epx - (int)(ep->vx * 1.2f), epy - (int)(ep->vy * 1.2f), NULL);
            LineTo(hdc, epx, epy);
            SelectObject(hdc, oldBstP);
            DeleteObject(hPenBlst);
        } else if (ep->type == 1) { // Homing Torpedo
            HBRUSH hBrTorp = CreateSolidBrush(ep->color);
            HPEN hPenTorp = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
            HGDIOBJ oldTrpP = SelectObject(hdc, hPenTorp);
            HGDIOBJ oldTrpB = SelectObject(hdc, hBrTorp);
            Ellipse(hdc, epx - 5, epy - 5, epx + 5, epy + 5);
            SelectObject(hdc, oldTrpP);
            SelectObject(hdc, oldTrpB);
            DeleteObject(hPenTorp);
            DeleteObject(hBrTorp);
        }
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
    
    // Radar Blips for Derelicts (Crimson Diamond)
    for (int i = 0; i < MAX_DERELICTS; i++) {
        if (!g_state.derelicts[i].active) continue;
        float bdx = (g_state.derelicts[i].x - g_state.shipX) * radarScale;
        float bdy = (g_state.derelicts[i].y - g_state.shipY) * radarScale;
        if (bdx * bdx + bdy * bdy < (radarR - 4) * (radarR - 4)) {
            COLORREF blipCol = (i == g_state.selectedDerelictIndex) ? RGB(245, 158, 11) : RGB(244, 63, 94);
            SetPixel(hdc, rcRadarX + (int)bdx, rcRadarY + (int)bdy, blipCol);
            SetPixel(hdc, rcRadarX + (int)bdx + 1, rcRadarY + (int)bdy, blipCol);
            SetPixel(hdc, rcRadarX + (int)bdx, rcRadarY + (int)bdy + 1, blipCol);
            SetPixel(hdc, rcRadarX + (int)bdx + 1, rcRadarY + (int)bdy + 1, blipCol);
        }
    }
    
    // Radar Blip for Orbital Spaceport (Cyan Star Hex)
    {
        float bdx = (g_state.stationX - g_state.shipX) * radarScale;
        float bdy = (g_state.stationY - g_state.shipY) * radarScale;
        if (bdx * bdx + bdy * bdy < (radarR - 4) * (radarR - 4)) {
            COLORREF stBlip = RGB(56, 189, 248);
            int sx = rcRadarX + (int)bdx;
            int sy = rcRadarY + (int)bdy;
            SetPixel(hdc, sx, sy, stBlip);
            SetPixel(hdc, sx - 1, sy, stBlip);
            SetPixel(hdc, sx + 1, sy, stBlip);
            SetPixel(hdc, sx, sy - 1, stBlip);
            SetPixel(hdc, sx, sy + 1, stBlip);
        }
    }

    // --- Phase 12: Radar Blips for Raiders & Hostile Torpedoes ---
    for (int i = 0; i < MAX_RAIDERS; i++) {
        if (!g_state.raiders[i].active) continue;
        float bdx = (g_state.raiders[i].x - g_state.shipX) * radarScale;
        float bdy = (g_state.raiders[i].y - g_state.shipY) * radarScale;
        if (bdx * bdx + bdy * bdy < (radarR - 4) * (radarR - 4)) {
            COLORREF rCol = g_state.raiders[i].color;
            int rx = rcRadarX + (int)bdx;
            int ry = rcRadarY + (int)bdy;
            SetPixel(hdc, rx, ry - 1, rCol);
            SetPixel(hdc, rx - 1, ry, rCol);
            SetPixel(hdc, rx, ry, rCol);
            SetPixel(hdc, rx + 1, ry, rCol);
            SetPixel(hdc, rx, ry + 1, rCol);
        }
    }
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        if (!g_state.enemyProjectiles[i].active || g_state.enemyProjectiles[i].type != 1) continue;
        float bdx = (g_state.enemyProjectiles[i].x - g_state.shipX) * radarScale;
        float bdy = (g_state.enemyProjectiles[i].y - g_state.shipY) * radarScale;
        if (bdx * bdx + bdy * bdy < (radarR - 4) * (radarR - 4)) {
            int tx = rcRadarX + (int)bdx;
            int ty = rcRadarY + (int)bdy;
            SetPixel(hdc, tx, ty, RGB(254, 240, 138));
            SetPixel(hdc, tx + 1, ty, RGB(254, 240, 138));
        }
    }
    
    // Center Ship Blip on Radar
    SetPixel(hdc, rcRadarX, rcRadarY, pal->vector);
    SetPixel(hdc, rcRadarX + 1, rcRadarY, pal->vector);
    
    SelectObject(hdc, oldRdPen);
    SelectObject(hdc, oldRdBr);
    DeleteObject(hPenRadar);
    DeleteObject(hBrRadar);
    
    // Proximity Spaceport Docking HUD Prompt
    {
        float stDist = (float)sqrt((g_state.stationX - g_state.shipX) * (g_state.stationX - g_state.shipX) +
                                   (g_state.stationY - g_state.shipY) * (g_state.stationY - g_state.shipY));
        if (stDist < 280.0f || g_state.stationDocked) {
            int hudW = 420;
            int hudH = 30;
            int hudX = viewportX + (viewportW - hudW) / 2;
            int hudY = viewportY + viewportH - 78;
            
            RECT rcStPrompt = { hudX, hudY, hudX + hudW, hudY + hudH };
            HBRUSH hBrStP = CreateSolidBrush(g_state.stationDocked ? RGB(10, 35, 60) : RGB(15, 23, 42));
            FillRect(hdc, &rcStPrompt, hBrStP);
            DeleteObject(hBrStP);
            FrameRect(hdc, &rcStPrompt, (HBRUSH)GetStockObject(WHITE_BRUSH));
            
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, g_state.stationDocked ? RGB(110, 231, 183) : RGB(56, 189, 248));
            char stPromptBuf[80];
            const StationDef* curStDef = &STATION_DEFS[g_state.currentSectorIndex];
            if (g_state.stationDocked) {
                sprintf(stPromptBuf, "⚓ DOCKED AT %s • PRESS [D] FOR STATION TERMINAL", curStDef->name);
            } else {
                sprintf(stPromptBuf, "⚓ [D] DOCK WITH %s (%dm)", curStDef->name, (int)stDist);
            }
            DrawTextA(hdc, stPromptBuf, -1, &rcStPrompt, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
    }
    
    // Proximity EVA HUD Prompt
    if (g_state.selectedDerelictIndex >= 0 && g_state.derelicts[g_state.selectedDerelictIndex].active) {
        Derelict* d = &g_state.derelicts[g_state.selectedDerelictIndex];
        float dist = (float)sqrt((d->x - g_state.shipX) * (d->x - g_state.shipX) +
                                 (d->y - g_state.shipY) * (d->y - g_state.shipY));
        if (dist < 200.0f) {
            int hudW = 380;
            int hudH = 30;
            int hudX = viewportX + (viewportW - hudW) / 2;
            int hudY = viewportY + viewportH - 45;
            
            RECT rcEvaPrompt = { hudX, hudY, hudX + hudW, hudY + hudH };
            HBRUSH hBrEvaP = CreateSolidBrush(RGB(15, 23, 42));
            FillRect(hdc, &rcEvaPrompt, hBrEvaP);
            DeleteObject(hBrEvaP);
            FrameRect(hdc, &rcEvaPrompt, (HBRUSH)GetStockObject(WHITE_BRUSH));
            
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, RGB(251, 191, 36));
            char evaPromptBuf[64];
            sprintf(evaPromptBuf, "✦ [E] INITIATE EVA SALVAGE ON %s (%dm)", d->id, (int)dist);
            DrawTextA(hdc, evaPromptBuf, -1, &rcEvaPrompt, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
    }
    
    // Target Lock Reticle Box on Bottom-Right of Viewport
    if (g_state.selectedAstIndex >= 0 && g_state.asteroids[g_state.selectedAstIndex].active) {
        Asteroid* target = &g_state.asteroids[g_state.selectedAstIndex];
        int tBoxW = 220;
        int tBoxH = target->scanned ? 78 : 54;
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
        RECT rcDist = { tBoxX + 110, tBoxY + 4, tBoxX + tBoxW - 6, tBoxY + 20 };
        DrawTextA(hdc, distBuf, -1, &rcDist, DT_RIGHT | DT_SINGLELINE);
        
        char oreStr[64];
        sprintf(oreStr, "PRIMARY: %s (%d%%)", ORE_DEFS[target->oreType].name, target->richness);
        SetTextColor(hdc, pal->textBright);
        TextOutA(hdc, tBoxX + 6, tBoxY + 18, oreStr, (int)strlen(oreStr));
        
        DrawBar(hdc, tBoxX + 6, tBoxY + 34, tBoxW - 12, 6, target->hp / target->maxHp, pal->vector, RGB(2, 6, 23), pal->borderPanel);
        
        if (target->scanned) {
            char spcStr[64];
            sprintf(spcStr, "Fe:%d%% Si:%d%% Pt:%d%% Vq:%d%% Dm:%d%%", target->comp[0], target->comp[1], target->comp[2], target->comp[3], target->comp[4]);
            SetTextColor(hdc, RGB(56, 189, 248));
            TextOutA(hdc, tBoxX + 6, tBoxY + 44, spcStr, (int)strlen(spcStr));
            
            char fqStr[64];
            sprintf(fqStr, "FREQ: %.1f MHz  [P] SPECTRO%s", target->freqMhz, target->resonantLock ? " (LOCKED)" : "");
            SetTextColor(hdc, target->resonantLock ? RGB(245, 158, 11) : RGB(110, 231, 183));
            TextOutA(hdc, tBoxX + 6, tBoxY + 58, fqStr, (int)strlen(fqStr));
        } else {
            SetTextColor(hdc, RGB(245, 158, 11));
            TextOutA(hdc, tBoxX + 6, tBoxY + 42, "[P] PROSPECT TO ASSAY MINERALS", 30);
        }
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
    
    // Modular Upgrades Engineering Bay Modal
    if (g_state.showUpgrades) {
        int modalW = 680;
        int modalH = 430;
        int mx = (totalW - modalW) / 2;
        int my = (totalH - modalH) / 2;
        
        RECT rcModal = { mx, my, mx + modalW, my + modalH };
        HBRUSH hBrModal = CreateSolidBrush(pal->bgPanel);
        FillRect(hdc, &rcModal, hBrModal);
        DeleteObject(hBrModal);
        
        HPEN hPenBorder3 = CreatePen(PS_SOLID, 2, RGB(245, 158, 11));
        HGDIOBJ oldPen3 = SelectObject(hdc, hPenBorder3);
        HGDIOBJ oldBrush3 = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, mx, my, mx + modalW, my + modalH);
        
        // Header
        RECT rcHdr = { mx, my, mx + modalW, my + 30 };
        HBRUSH hBrHdr = CreateSolidBrush(pal->bgHeader);
        FillRect(hdc, &rcHdr, hBrHdr);
        DeleteObject(hBrHdr);
        
        SelectObject(hdc, g_fontHeader);
        SetTextColor(hdc, RGB(251, 191, 36));
        TextOutA(hdc, mx + 14, my + 6, "MODULAR BARGE ENGINEERING BAY", 29);
        
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, pal->textBright);
        TextOutA(hdc, mx + modalW - 120, my + 9, "[U / ESC] CLOSE", 15);
        
        // Status Bar
        RECT rcStat = { mx + 14, my + 34, mx + modalW - 14, my + 60 };
        HBRUSH hBrStat = CreateSolidBrush(RGB(3, 7, 18));
        FillRect(hdc, &rcStat, hBrStat);
        DeleteObject(hBrStat);
        FrameRect(hdc, &rcStat, (HBRUSH)GetStockObject(WHITE_BRUSH));
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, pal->vector);
        TextOutA(hdc, mx + 24, my + 39, "BARGE RETROFIT DOCK: KStarDredge Mk-IV Heavy Frame", 50);
        
        char credBuf[64];
        sprintf(credBuf, "AVAILABLE CREDITS: %d CR", g_state.credits);
        SetTextColor(hdc, RGB(245, 158, 11));
        TextOutA(hdc, mx + modalW - 220, my + 39, credBuf, (int)strlen(credBuf));
        
        // 2x2 Upgrade Cards
        int cardW = 318;
        int cardH = 145;
        int gapX = 16;
        int gapY = 10;
        int startX = mx + 14;
        int startY = my + 68;
        
        for (int m = 0; m < 4; m++) {
            int row = m / 2;
            int col = m % 2;
            int ucx = startX + col * (cardW + gapX);
            int ucy = startY + row * (cardH + gapY);
            
            RECT rcCard = { ucx, ucy, ucx + cardW, ucy + cardH };
            HBRUSH hBrCard = CreateSolidBrush(RGB(5, 12, 28));
            FillRect(hdc, &rcCard, hBrCard);
            DeleteObject(hBrCard);
            
            HPEN hPenCard = CreatePen(PS_SOLID, 1, pal->borderPanel);
            SelectObject(hdc, hPenCard);
            Rectangle(hdc, ucx, ucy, ucx + cardW, ucy + cardH);
            DeleteObject(hPenCard);
            
            const char* modTitle = "";
            int curTierIdx = 0;
            int nextCost = 0;
            const char* curName = "";
            char curStatStr[64] = "";
            char nextStatStr[64] = "";
            
            if (m == 0) { // Drill
                modTitle = "[1] HEAVY PLASMA DRILL";
                curTierIdx = g_state.upgradeDrill;
                const DrillUpgradeDef* cur = &DRILL_UPGRADES[curTierIdx];
                curName = cur->name;
                sprintf(curStatStr, "DPS: +%.0f%% | Heat: -%.0f%% | Drop: %.0f%%", (cur->dpsBonus - 1.0f) * 100.0f, (1.0f - cur->heatRate) * 100.0f, cur->chunkBonus * 100.0f);
                if (curTierIdx < 4) {
                    const DrillUpgradeDef* nxt = &DRILL_UPGRADES[curTierIdx + 1];
                    nextCost = nxt->cost;
                    sprintf(nextStatStr, "Next: %s (+%.0f%% DPS)", nxt->name, (nxt->dpsBonus - 1.0f) * 100.0f);
                }
            } else if (m == 1) { // Cargo
                modTitle = "[2] TITAN CARGO BAY";
                curTierIdx = g_state.upgradeCargo;
                const CargoUpgradeDef* cur = &CARGO_UPGRADES[curTierIdx];
                curName = cur->name;
                sprintf(curStatStr, "Ore Capacity: %d Tonnage", cur->capacity);
                if (curTierIdx < 4) {
                    const CargoUpgradeDef* nxt = &CARGO_UPGRADES[curTierIdx + 1];
                    nextCost = nxt->cost;
                    sprintf(nextStatStr, "Next: %s (%d T capacity)", nxt->name, nxt->capacity);
                }
            } else if (m == 2) { // Engine
                modTitle = "[3] FUSION THRUSTERS";
                curTierIdx = g_state.upgradeEngine;
                const EngineUpgradeDef* cur = &ENGINE_UPGRADES[curTierIdx];
                curName = cur->name;
                sprintf(curStatStr, "Speed: %.1f km/s | Thrust: %.2f | Burn: -%.0f%%", cur->maxSpeed, cur->thrust, (1.0f - cur->fuelBurn) * 100.0f);
                if (curTierIdx < 4) {
                    const EngineUpgradeDef* nxt = &ENGINE_UPGRADES[curTierIdx + 1];
                    nextCost = nxt->cost;
                    sprintf(nextStatStr, "Next: %s (%.1f km/s)", nxt->name, nxt->maxSpeed);
                }
            } else if (m == 3) { // Shield
                modTitle = "[4] PARTICLE SHIELDS";
                curTierIdx = g_state.upgradeShield;
                const ShieldUpgradeDef* cur = &SHIELD_UPGRADES[curTierIdx];
                curName = cur->name;
                sprintf(curStatStr, "Shield: %.0f | Hull: %.0f | Regen: %.2f/f", cur->maxShield, cur->maxHull, cur->regen);
                if (curTierIdx < 4) {
                    const ShieldUpgradeDef* nxt = &SHIELD_UPGRADES[curTierIdx + 1];
                    nextCost = nxt->cost;
                    sprintf(nextStatStr, "Next: %s (%.0f Shield / %.0f Hull)", nxt->name, nxt->maxShield, nxt->maxHull);
                }
            }
            
            // Title
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, pal->vector);
            TextOutA(hdc, ucx + 8, ucy + 6, modTitle, (int)strlen(modTitle));
            
            // 5 Tier Pips
            for (int p = 0; p < 5; p++) {
                RECT rcPip = { ucx + cardW - 80 + (p * 14), ucy + 8, ucx + cardW - 80 + (p * 14) + 11, ucy + 16 };
                HBRUSH hBrPip = CreateSolidBrush(p <= curTierIdx ? RGB(245, 158, 11) : RGB(30, 41, 59));
                FillRect(hdc, &rcPip, hBrPip);
                DeleteObject(hBrPip);
                FrameRect(hdc, &rcPip, (HBRUSH)GetStockObject(WHITE_BRUSH));
            }
            
            // Current Tier Name
            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, RGB(255, 255, 255));
            char curTierLabel[64];
            sprintf(curTierLabel, "Current: %s", curName);
            TextOutA(hdc, ucx + 8, ucy + 26, curTierLabel, (int)strlen(curTierLabel));
            
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutA(hdc, ucx + 8, ucy + 42, curStatStr, (int)strlen(curStatStr));
            
            // Next Tier Info or Max Level
            if (curTierIdx >= 4) {
                SetTextColor(hdc, RGB(16, 185, 129));
                TextOutA(hdc, ucx + 8, ucy + 64, "* FULLY RETROFITTED (MAX TIER)", 30);
                
                RECT rcBtnMax = { ucx + 8, ucy + cardH - 34, ucx + cardW - 8, ucy + cardH - 8 };
                HBRUSH hBrMax = CreateSolidBrush(RGB(6, 78, 59));
                FillRect(hdc, &rcBtnMax, hBrMax);
                DeleteObject(hBrMax);
                SelectObject(hdc, g_fontMonoBold);
                SetTextColor(hdc, RGB(110, 231, 183));
                DrawTextA(hdc, "TIER MAXIMUM INSTALLED", -1, &rcBtnMax, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            } else {
                SetTextColor(hdc, RGB(56, 189, 248));
                TextOutA(hdc, ucx + 8, ucy + 64, nextStatStr, (int)strlen(nextStatStr));
                
                int canAfford = (g_state.credits >= nextCost);
                RECT rcBtnBuy = { ucx + 8, ucy + cardH - 34, ucx + cardW - 8, ucy + cardH - 8 };
                HBRUSH hBrBuy = CreateSolidBrush(canAfford ? RGB(30, 58, 138) : RGB(30, 41, 59));
                FillRect(hdc, &rcBtnBuy, hBrBuy);
                DeleteObject(hBrBuy);
                FrameRect(hdc, &rcBtnBuy, (HBRUSH)GetStockObject(WHITE_BRUSH));
                
                SelectObject(hdc, g_fontMonoBold);
                SetTextColor(hdc, canAfford ? RGB(255, 255, 255) : RGB(148, 163, 184));
                char buyBtnTxt[64];
                sprintf(buyBtnTxt, "INSTALL [%d]  -  %d CR", m + 1, nextCost);
                DrawTextA(hdc, buyBtnTxt, -1, &rcBtnBuy, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }
        
        // Footer hint
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(245, 158, 11));
        TextOutA(hdc, mx + 16, my + modalH - 24, "Press Keys [1-4] or Click Upgrade Buttons to Install Retrofits • [U / ESC] Close", 79);
        
        SelectObject(hdc, oldPen3);
        SelectObject(hdc, oldBrush3);
        DeleteObject(hPenBorder3);
    }
    
    // Prospector Multi-Spectral Emission Spectrometer Modal
    if (g_state.showSpectrometer) {
        int modalW = 700;
        int modalH = 460;
        int mx = (totalW - modalW) / 2;
        int my = (totalH - modalH) / 2;
        
        RECT rcModal = { mx, my, mx + modalW, my + modalH };
        HBRUSH hBrModal = CreateSolidBrush(pal->bgPanel);
        FillRect(hdc, &rcModal, hBrModal);
        DeleteObject(hBrModal);
        
        HPEN hPenBorderSpec = CreatePen(PS_SOLID, 2, pal->vector);
        HGDIOBJ oldPenSpec = SelectObject(hdc, hPenBorderSpec);
        HGDIOBJ oldBrushSpec = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, mx, my, mx + modalW, my + modalH);
        
        // Header
        RECT rcHdr = { mx, my, mx + modalW, my + 30 };
        HBRUSH hBrHdr = CreateSolidBrush(pal->bgHeader);
        FillRect(hdc, &rcHdr, hBrHdr);
        DeleteObject(hBrHdr);
        
        SelectObject(hdc, g_fontHeader);
        SetTextColor(hdc, pal->vector);
        TextOutA(hdc, mx + 14, my + 6, "PROSPECTOR MULTI-SPECTRAL EMISSION SPECTROMETER", 47);
        
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, pal->textBright);
        TextOutA(hdc, mx + modalW - 120, my + 9, "[P / ESC] CLOSE", 15);
        
        if (g_state.selectedAstIndex < 0 || !g_state.asteroids[g_state.selectedAstIndex].active) {
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, RGB(239, 68, 68));
            TextOutA(hdc, mx + 30, my + 70, "NO ASTEROID TARGET LOCKED IN RETICLE", 36);
            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutA(hdc, mx + 30, my + 95, "Click any asteroid in the viewport to lock reticle or trigger a Wide-Band Sweep.", 80);
        } else {
            Asteroid* target = &g_state.asteroids[g_state.selectedAstIndex];
            
            // Top status line
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, pal->textBright);
            char infoBuf[128];
            sprintf(infoBuf, "TARGET: %s   CLASS: %s   STATUS: %s   PURITY: LV.%d   VOLATILITY: %d%%",
                    target->id, ORE_DEFS[target->oreType].name, target->scanned ? "ANALYZED" : "UNPROSPECTED", target->purity, target->volatility);
            TextOutA(hdc, mx + 16, my + 36, infoBuf, (int)strlen(infoBuf));
            
            // Left: Spectral Emission Waveform Scope (320x205)
            int scopeX = mx + 16;
            int scopeY = my + 56;
            int scopeW = 320;
            int scopeH = 205;
            RECT rcScope = { scopeX, scopeY, scopeX + scopeW, scopeY + scopeH };
            HBRUSH hBrScope = CreateSolidBrush(RGB(2, 6, 18));
            FillRect(hdc, &rcScope, hBrScope);
            DeleteObject(hBrScope);
            FrameRect(hdc, &rcScope, (HBRUSH)GetStockObject(WHITE_BRUSH));
            
            // Scope Grid & Axis
            HPEN hPenGrid = CreatePen(PS_DOT, 1, RGB(30, 58, 138));
            SelectObject(hdc, hPenGrid);
            for (int gy = 1; gy < 5; gy++) {
                int yline = scopeY + (gy * scopeH / 5);
                MoveToEx(hdc, scopeX, yline, NULL);
                LineTo(hdc, scopeX + scopeW, yline);
            }
            for (int gx = 1; gx < 6; gx++) {
                int xline = scopeX + (gx * scopeW / 6);
                MoveToEx(hdc, xline, scopeY, NULL);
                LineTo(hdc, xline, scopeY + scopeH);
            }
            DeleteObject(hPenGrid);
            
            // Scope Waveform
            HPEN hPenWave = CreatePen(PS_SOLID, 2, target->scanned ? (target->resonantLock ? RGB(245, 158, 11) : pal->vector) : RGB(100, 116, 139));
            SelectObject(hdc, hPenWave);
            int prevWX = scopeX;
            int prevWY = scopeY + scopeH / 2;
            MoveToEx(hdc, prevWX, prevWY, NULL);
            for (int x = 0; x < scopeW; x += 3) {
                float normX = (float)x / (float)scopeW;
                float wave = 0.0f;
                if (target->scanned) {
                    for (int e = 0; e < 6; e++) {
                        float center = ((float)e + 0.5f) / 6.0f;
                        float dist = (float)fabs(normX - center);
                        float height = (float)target->comp[e] / 100.0f;
                        wave += (float)exp(-dist * dist * 90.0f) * height;
                    }
                    wave += (float)sin(normX * 30.0f + (float)(target->freqMhz * 0.05f)) * 0.08f;
                } else {
                    wave = (float)sin(normX * 15.0f) * 0.2f + (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.1f;
                }
                int wy = scopeY + scopeH - 15 - (int)(wave * (scopeH - 30));
                if (wy < scopeY + 10) wy = scopeY + 10;
                if (wy > scopeY + scopeH - 10) wy = scopeY + scopeH - 10;
                LineTo(hdc, scopeX + x, wy);
            }
            DeleteObject(hPenWave);
            
            // Scope Labels
            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutA(hdc, scopeX + 6, scopeY + 6, "EMISSION SPECTRUM (100 - 900 MHz)", 33);
            char fqLabel[64];
            sprintf(fqLabel, "RESONANCE: %.1f MHz", target->freqMhz);
            SetTextColor(hdc, target->resonantLock ? RGB(245, 158, 11) : pal->textPrimary);
            TextOutA(hdc, scopeX + scopeW - 140, scopeY + 6, fqLabel, (int)strlen(fqLabel));
            
            // Right: Mineral Assay Composition Table (334x205)
            int tblX = mx + 350;
            int tblY = my + 56;
            int tblW = 334;
            int tblH = 205;
            RECT rcTbl = { tblX, tblY, tblX + tblW, tblY + tblH };
            HBRUSH hBrTbl = CreateSolidBrush(RGB(5, 12, 28));
            FillRect(hdc, &rcTbl, hBrTbl);
            DeleteObject(hBrTbl);
            FrameRect(hdc, &rcTbl, (HBRUSH)GetStockObject(WHITE_BRUSH));
            
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, pal->textBright);
            TextOutA(hdc, tblX + 8, tblY + 6, "MINERAL ASSAY BREAKDOWN", 23);
            
            int estValue = 0;
            for (int o = 0; o < 6; o++) {
                int rowY = tblY + 26 + (o * 23);
                int pct = target->scanned ? target->comp[o] : 0;
                int oreYield = (int)((target->radius * 0.8f) * (pct / 100.0f));
                if (target->scanned && pct > 0 && oreYield == 0) oreYield = 1;
                int oreVal = oreYield * ORE_DEFS[o].value;
                estValue += oreVal;
                
                SelectObject(hdc, g_fontSmall);
                SetTextColor(hdc, ORE_DEFS[o].color);
                TextOutA(hdc, tblX + 8, rowY, ORE_DEFS[o].name, (int)strlen(ORE_DEFS[o].name));
                
                // Mini Bar
                DrawBar(hdc, tblX + 110, rowY + 2, 110, 8, pct / 100.0f, ORE_DEFS[o].color, RGB(2, 6, 23), pal->borderPanel);
                
                char pctBuf[32];
                if (target->scanned) {
                    sprintf(pctBuf, "%d%% (~%dT)", pct, oreYield);
                } else {
                    sprintf(pctBuf, "???");
                }
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, tblX + 230, rowY, pctBuf, (int)strlen(pctBuf));
            }
            
            // Valuation summary row
            int valY = tblY + tblH - 20;
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, RGB(245, 158, 11));
            char valBuf[64];
            if (target->scanned) {
                sprintf(valBuf, "ESTIMATED VEIN VALUE: ~%d CR", estValue);
            } else {
                sprintf(valBuf, "ESTIMATED VALUE: UNPROSPECTED");
            }
            TextOutA(hdc, tblX + 8, valY, valBuf, (int)strlen(valBuf));
            
            // Geode Core Readout Box
            int gBoxX = mx + 16;
            int gBoxY = my + 270;
            int gBoxW = modalW - 32;
            int gBoxH = 86;
            RECT rcGBox = { gBoxX, gBoxY, gBoxX + gBoxW, gBoxY + gBoxH };
            HBRUSH hBrGBox = CreateSolidBrush(RGB(3, 8, 22));
            FillRect(hdc, &rcGBox, hBrGBox);
            DeleteObject(hBrGBox);
            FrameRect(hdc, &rcGBox, (HBRUSH)GetStockObject(WHITE_BRUSH));
            
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, pal->vector);
            TextOutA(hdc, gBoxX + 10, gBoxY + 8, "ANOMALY & GEODE CORE DETECTOR:", 30);
            
            SelectObject(hdc, g_fontSmall);
            if (!target->scanned) {
                SetTextColor(hdc, RGB(148, 163, 184));
                TextOutA(hdc, gBoxX + 10, gBoxY + 30, "Awaiting spectrometer assay. Press [1] or Click 'PROSPECT TARGET' to detect internal crystallizations.", 104);
                TextOutA(hdc, gBoxX + 10, gBoxY + 48, "Geode cores contain rare unrefined Dark Matter, Resonant Void Quartz, and dense Platinum pockets.", 96);
            } else if (target->geodeType == 2) {
                SetTextColor(hdc, RGB(244, 63, 94));
                TextOutA(hdc, gBoxX + 10, gBoxY + 30, "✦ DARK MATTER GEODE CORE DETECTED: Dense gravitational anomaly localized at core coordinates.", 93);
                SetTextColor(hdc, RGB(254, 205, 211));
                TextOutA(hdc, gBoxX + 10, gBoxY + 48, "Shattering this asteroid is guaranteed to extract pure Dark Matter clusters (+1,200 CR / T).", 92);
            } else if (target->geodeType == 1) {
                SetTextColor(hdc, RGB(192, 132, 252));
                TextOutA(hdc, gBoxX + 10, gBoxY + 30, "✦ RESONANT VOID QUARTZ CRYSTAL: High-frequency piezo-electric crystal cavity discovered.", 89);
                SetTextColor(hdc, RGB(243, 232, 255));
                TextOutA(hdc, gBoxX + 10, gBoxY + 48, "Tune laser harmonics to lock resonance and yield pristine Void Quartz crystals (+350 CR / T).", 93);
            } else if (target->geodeType == 3) {
                SetTextColor(hdc, RGB(224, 242, 254));
                TextOutA(hdc, gBoxX + 10, gBoxY + 30, "✦ DENSE PLATINUM POCKET: Sub-surface heavy precious metallic deposit identified.", 81);
                SetTextColor(hdc, RGB(207, 250, 254));
                TextOutA(hdc, gBoxX + 10, gBoxY + 48, "Refining yield bonus active on fracturing (+120 CR / T).", 56);
            } else {
                SetTextColor(hdc, RGB(16, 185, 129));
                TextOutA(hdc, gBoxX + 10, gBoxY + 30, "✔ STANDARD ORE MATRIX: Homogeneous mineral distribution. No volatile anomaly detected.", 86);
                SetTextColor(hdc, RGB(148, 163, 184));
                TextOutA(hdc, gBoxX + 10, gBoxY + 48, "Optimal candidate for sustained high-temperature plasma drill extraction.", 73);
            }
        }
        
        // Bottom 3 Action Buttons: [1] Prospect Target, [2] Wide Sweep, [3] Tune Resonance
        int actY = my + modalH - 74;
        int btnW = (modalW - 48) / 3;
        int btnH = 40;
        
        for (int b = 0; b < 3; b++) {
            int bx = mx + 16 + b * (btnW + 8);
            RECT rcBtn = { bx, actY, bx + btnW, actY + btnH };
            
            COLORREF bBg = RGB(30, 58, 138);
            if (b == 1) bBg = RGB(6, 78, 59);
            if (b == 2) bBg = RGB(120, 53, 15);
            
            HBRUSH hBrBtn = CreateSolidBrush(bBg);
            FillRect(hdc, &rcBtn, hBrBtn);
            DeleteObject(hBrBtn);
            FrameRect(hdc, &rcBtn, (HBRUSH)GetStockObject(WHITE_BRUSH));
            
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, RGB(255, 255, 255));
            const char* bTitle = (b == 0) ? "[1] PROSPECT TARGET" : (b == 1 ? "[2] WIDE-BAND SWEEP" : "[3] TUNE RESONANCE");
            DrawTextA(hdc, bTitle, -1, &rcBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        
        // Footer instruction
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(245, 158, 11));
        TextOutA(hdc, mx + 16, my + modalH - 24, "Keys [1] Prospect Target • [2] Wide Sweep • [3] Laser Resonance • [P / ESC] Close", 80);
        
        SelectObject(hdc, oldPenSpec);
        SelectObject(hdc, oldBrushSpec);
        DeleteObject(hPenBorderSpec);
    }
    
    // Derelict Spaceship Exploration & EVA Salvage Ops Modal (Phase 8)
    if (g_state.showEva) {
        int modalW = 720;
        int modalH = 470;
        int mx = (totalW - modalW) / 2;
        int my = (totalH - modalH) / 2;
        
        RECT rcModal = { mx, my, mx + modalW, my + modalH };
        HBRUSH hBrModal = CreateSolidBrush(pal->bgPanel);
        FillRect(hdc, &rcModal, hBrModal);
        DeleteObject(hBrModal);
        
        HPEN hPenBorderEva = CreatePen(PS_SOLID, 2, RGB(245, 158, 11));
        HGDIOBJ oldPenEva = SelectObject(hdc, hPenBorderEva);
        HGDIOBJ oldBrushEva = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, mx, my, mx + modalW, my + modalH);
        
        // Header
        RECT rcHdr = { mx, my, mx + modalW, my + 30 };
        HBRUSH hBrHdr = CreateSolidBrush(pal->bgHeader);
        FillRect(hdc, &rcHdr, hBrHdr);
        DeleteObject(hBrHdr);
        
        SelectObject(hdc, g_fontHeader);
        SetTextColor(hdc, RGB(251, 191, 36));
        TextOutA(hdc, mx + 14, my + 6, "DERELICT SHIP EXPLORATION & EVA SALVAGE OPS", 43);
        
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, pal->textBright);
        TextOutA(hdc, mx + modalW - 120, my + 9, "[E / ESC] CLOSE", 15);
        
        if (g_state.selectedDerelictIndex < 0 || !g_state.derelicts[g_state.selectedDerelictIndex].active) {
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, RGB(239, 68, 68));
            TextOutA(hdc, mx + 30, my + 70, "NO DERELICT VESSEL IN EVA TELEMETRY RANGE", 41);
            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutA(hdc, mx + 30, my + 95, "Fly near any derelict warship (<200m) and lock target to initiate EVA operations.", 81);
        } else {
            Derelict* d = &g_state.derelicts[g_state.selectedDerelictIndex];
            
            // Top status line
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, pal->textBright);
            char infoBuf[128];
            sprintf(infoBuf, "VESSEL: %s   CLASS: %s   AIRLOCK: %s   STATUS: ABANDONED",
                    d->id, d->classType, d->airlockBreached ? "BREACHED" : "SEALED");
            TextOutA(hdc, mx + 16, my + 36, infoBuf, (int)strlen(infoBuf));
            
            // Left Column: Wireframe Schematic Scope (310x175) & Archive Box (310x175)
            int scpX = mx + 16;
            int scpY = my + 56;
            int scpW = 310;
            int scpH = 175;
            
            RECT rcScp = { scpX, scpY, scpX + scpW, scpY + scpH };
            HBRUSH hBrScp = CreateSolidBrush(RGB(2, 6, 18));
            FillRect(hdc, &rcScp, hBrScp);
            DeleteObject(hBrScp);
            FrameRect(hdc, &rcScp, (HBRUSH)GetStockObject(WHITE_BRUSH));
            
            // Draw Blueprint Schematic
            int sCenterCX = scpX + (scpW / 2);
            int sCenterCY = scpY + (scpH / 2);
            
            HPEN hPenScpGrid = CreatePen(PS_DOT, 1, RGB(30, 58, 138));
            SelectObject(hdc, hPenScpGrid);
            MoveToEx(hdc, scpX, sCenterCY, NULL); LineTo(hdc, scpX + scpW, sCenterCY);
            MoveToEx(hdc, sCenterCX, scpY, NULL); LineTo(hdc, sCenterCX, scpY + scpH);
            DeleteObject(hPenScpGrid);
            
            // Wireframe Hull
            POINT scpPts[8] = {
                { sCenterCX + 110, sCenterCY },
                { sCenterCX + 45,  sCenterCY + 35 },
                { sCenterCX - 65,  sCenterCY + 30 },
                { sCenterCX - 110, sCenterCY + 20 },
                { sCenterCX - 85,  sCenterCY },
                { sCenterCX - 110, sCenterCY - 20 },
                { sCenterCX - 65,  sCenterCY - 30 },
                { sCenterCX + 45,  sCenterCY - 35 }
            };
            HPEN hPenWire = CreatePen(PS_SOLID, 2, RGB(0, 240, 255));
            SelectObject(hdc, hPenWire);
            Polygon(hdc, scpPts, 8);
            
            // Interior bulkhead lines
            MoveToEx(hdc, sCenterCX + 45, sCenterCY - 35, NULL); LineTo(hdc, sCenterCX + 45, sCenterCY + 35);
            MoveToEx(hdc, sCenterCX - 20, sCenterCY - 25, NULL); LineTo(hdc, sCenterCX - 20, sCenterCY + 25);
            MoveToEx(hdc, sCenterCX - 65, sCenterCY - 30, NULL); LineTo(hdc, sCenterCX - 65, sCenterCY + 30);
            DeleteObject(hPenWire);
            
            // Core & Bridge markers on blueprint
            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, d->reactorHarvested ? RGB(16, 185, 129) : RGB(244, 63, 94));
            TextOutA(hdc, sCenterCX - 55, sCenterCY - 6, "[REACTOR]", 9);
            SetTextColor(hdc, d->blackBoxDecrypted ? RGB(16, 185, 129) : RGB(56, 189, 248));
            TextOutA(hdc, sCenterCX + 10, sCenterCY - 6, "[AVIONICS]", 10);
            
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutA(hdc, scpX + 8, scpY + 6, "BLUEPRINT: STRUCTURAL WIREFRAME", 31);
            
            // Ship Log Archive Box
            int arcX = scpX;
            int arcY = scpY + scpH + 8;
            int arcW = scpW;
            int arcH = 175;
            
            RECT rcArc = { arcX, arcY, arcX + arcW, arcY + arcH };
            HBRUSH hBrArc = CreateSolidBrush(RGB(5, 12, 28));
            FillRect(hdc, &rcArc, hBrArc);
            DeleteObject(hBrArc);
            FrameRect(hdc, &rcArc, (HBRUSH)GetStockObject(WHITE_BRUSH));
            
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, RGB(251, 191, 36));
            TextOutA(hdc, arcX + 8, arcY + 6, "VESSEL IDENTITY & ARCHIVE LOGS", 30);
            
            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, RGB(224, 242, 254));
            TextOutA(hdc, arcX + 8, arcY + 24, d->name, (int)strlen(d->name));
            
            SetTextColor(hdc, RGB(148, 163, 184));
            RECT rcLogText = { arcX + 8, arcY + 44, arcX + arcW - 8, arcY + arcH - 8 };
            DrawTextA(hdc, d->logArchive, -1, &rcLogText, DT_WORDBREAK);
            
            // Right Column: 4 Interactive Operations Modules (360px wide)
            int opX = mx + 340;
            int opY = my + 56;
            int opW = modalW - 356;
            int cardH = 84;
            int gapY = 8;
            
            // 1. Airlock Breach Card
            RECT rcOp1 = { opX, opY, opX + opW, opY + cardH };
            HBRUSH hBrOp1 = CreateSolidBrush(d->airlockBreached ? RGB(6, 43, 16) : RGB(26, 16, 2));
            FillRect(hdc, &rcOp1, hBrOp1);
            DeleteObject(hBrOp1);
            HPEN hPenOp1 = CreatePen(PS_SOLID, 1, d->airlockBreached ? RGB(16, 185, 129) : RGB(245, 158, 11));
            SelectObject(hdc, hPenOp1);
            Rectangle(hdc, opX, opY, opX + opW, opY + cardH);
            DeleteObject(hPenOp1);
            
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, RGB(255, 255, 255));
            TextOutA(hdc, opX + 10, opY + 6, "[1] AIRLOCK BREACH (PLASMA CUTTER)", 34);
            
            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutA(hdc, opX + 10, opY + 24, "Cut through blast hatch with plasma torch.", 42);
            
            DrawBar(hdc, opX + 10, opY + 40, opW - 130, 8, d->airlockCutProgress / 100.0f, RGB(245, 158, 11), RGB(2, 6, 23), pal->borderPanel);
            
            char cutPctBuf[32];
            sprintf(cutPctBuf, "%d%% CUT", d->airlockCutProgress);
            SetTextColor(hdc, RGB(251, 191, 36));
            TextOutA(hdc, opX + opW - 110, opY + 38, cutPctBuf, (int)strlen(cutPctBuf));
            
            RECT rcBtnCut = { opX + 10, opY + 54, opX + opW - 10, opY + 76 };
            HBRUSH hBrBtnCut = CreateSolidBrush(d->airlockBreached ? RGB(6, 78, 59) : RGB(120, 53, 15));
            FillRect(hdc, &rcBtnCut, hBrBtnCut);
            DeleteObject(hBrBtnCut);
            FrameRect(hdc, &rcBtnCut, (HBRUSH)GetStockObject(WHITE_BRUSH));
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, d->airlockBreached ? RGB(110, 231, 183) : RGB(255, 255, 255));
            DrawTextA(hdc, d->airlockBreached ? "✔ AIRLOCK FULLY BREACHED" : "CUT AIRLOCK [KEY 1]", -1, &rcBtnCut, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
            // 2. Black Box Decryption Card
            int opY2 = opY + cardH + gapY;
            RECT rcOp2 = { opX, opY2, opX + opW, opY2 + cardH };
            HBRUSH hBrOp2 = CreateSolidBrush(d->blackBoxDecrypted ? RGB(6, 43, 16) : (d->airlockBreached ? RGB(5, 12, 28) : RGB(20, 20, 20)));
            FillRect(hdc, &rcOp2, hBrOp2);
            DeleteObject(hBrOp2);
            HPEN hPenOp2 = CreatePen(PS_SOLID, 1, d->blackBoxDecrypted ? RGB(16, 185, 129) : (d->airlockBreached ? RGB(56, 189, 248) : RGB(60, 60, 60)));
            SelectObject(hdc, hPenOp2);
            Rectangle(hdc, opX, opY2, opX + opW, opY2 + cardH);
            DeleteObject(hPenOp2);
            
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, d->airlockBreached ? RGB(56, 189, 248) : RGB(100, 116, 139));
            TextOutA(hdc, opX + 10, opY2 + 6, "[2] BLACK BOX TELEMETRY DECRYPTION", 34);
            
            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, RGB(148, 163, 184));
            char bbDesc[64];
            sprintf(bbDesc, "Avionics bridge flight records. Bounty: +%d CR.", d->dataValue);
            TextOutA(hdc, opX + 10, opY2 + 24, bbDesc, (int)strlen(bbDesc));
            
            RECT rcBtnBB = { opX + 10, opY2 + 48, opX + opW - 10, opY2 + 74 };
            HBRUSH hBrBtnBB = CreateSolidBrush(d->blackBoxDecrypted ? RGB(6, 78, 59) : (d->airlockBreached ? RGB(30, 58, 138) : RGB(30, 41, 59)));
            FillRect(hdc, &rcBtnBB, hBrBtnBB);
            DeleteObject(hBrBtnBB);
            FrameRect(hdc, &rcBtnBB, (HBRUSH)GetStockObject(WHITE_BRUSH));
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, d->blackBoxDecrypted ? RGB(110, 231, 183) : (d->airlockBreached ? RGB(255, 255, 255) : RGB(100, 116, 139)));
            char bbTxt[64];
            sprintf(bbTxt, d->blackBoxDecrypted ? "✔ FLIGHT LOGS EXTRACTED (+%d CR)" : (d->airlockBreached ? "DECRYPT BLACK BOX [KEY 2] (+%d CR)" : "LOCKED: REQUIRE BREACHED AIRLOCK"), d->dataValue);
            DrawTextA(hdc, bbTxt, -1, &rcBtnBB, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
            // 3. Antimatter Reactor Harvest Card
            int opY3 = opY2 + cardH + gapY;
            RECT rcOp3 = { opX, opY3, opX + opW, opY3 + cardH };
            HBRUSH hBrOp3 = CreateSolidBrush(d->reactorHarvested ? RGB(6, 43, 16) : (d->airlockBreached ? RGB(30, 6, 12) : RGB(20, 20, 20)));
            FillRect(hdc, &rcOp3, hBrOp3);
            DeleteObject(hBrOp3);
            HPEN hPenOp3 = CreatePen(PS_SOLID, 1, d->reactorHarvested ? RGB(16, 185, 129) : (d->airlockBreached ? RGB(244, 63, 94) : RGB(60, 60, 60)));
            SelectObject(hdc, hPenOp3);
            Rectangle(hdc, opX, opY3, opX + opW, opY3 + cardH);
            DeleteObject(hPenOp3);
            
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, d->airlockBreached ? RGB(244, 63, 94) : RGB(100, 116, 139));
            TextOutA(hdc, opX + 10, opY3 + 6, "[3] ANTIMATTER REACTOR CORE HARVEST", 35);
            
            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, RGB(148, 163, 184));
            char rcDesc[64];
            sprintf(rcDesc, "Extract antimatter core. Value: +%d CR.", d->coreValue);
            TextOutA(hdc, opX + 10, opY3 + 24, rcDesc, (int)strlen(rcDesc));
            
            RECT rcBtnCore = { opX + 10, opY3 + 48, opX + opW - 10, opY3 + 74 };
            HBRUSH hBrBtnCore = CreateSolidBrush(d->reactorHarvested ? RGB(6, 78, 59) : (d->airlockBreached ? RGB(131, 24, 67) : RGB(30, 41, 59)));
            FillRect(hdc, &rcBtnCore, hBrBtnCore);
            DeleteObject(hBrBtnCore);
            FrameRect(hdc, &rcBtnCore, (HBRUSH)GetStockObject(WHITE_BRUSH));
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, d->reactorHarvested ? RGB(110, 231, 183) : (d->airlockBreached ? RGB(255, 255, 255) : RGB(100, 116, 139)));
            char coreTxt[64];
            sprintf(coreTxt, d->reactorHarvested ? "✔ CORE SAFELY HARVESTED (+%d CR)" : (d->airlockBreached ? "HARVEST REACTOR CORE [KEY 3] (+%d CR)" : "LOCKED: REQUIRE BREACHED AIRLOCK"), d->coreValue);
            DrawTextA(hdc, coreTxt, -1, &rcBtnCore, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
            // 4. Cargo Pod Scavenging Card
            int opY4 = opY3 + cardH + gapY;
            RECT rcOp4 = { opX, opY4, opX + opW, opY4 + cardH };
            HBRUSH hBrOp4 = CreateSolidBrush(d->cargoScavenged ? RGB(6, 43, 16) : RGB(26, 16, 2));
            FillRect(hdc, &rcOp4, hBrOp4);
            DeleteObject(hBrOp4);
            HPEN hPenOp4 = CreatePen(PS_SOLID, 1, d->cargoScavenged ? RGB(16, 185, 129) : RGB(251, 191, 36));
            SelectObject(hdc, hPenOp4);
            Rectangle(hdc, opX, opY4, opX + opW, opY4 + cardH);
            DeleteObject(hPenOp4);
            
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, RGB(251, 191, 36));
            TextOutA(hdc, opX + 10, opY4 + 6, "[4] CARGO POD SCAVENGING", 24);
            
            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, RGB(148, 163, 184));
            char scvDesc[64];
            sprintf(scvDesc, "Strip intact pods: %d T of Derelict Scrap available.", d->scrapPods);
            TextOutA(hdc, opX + 10, opY4 + 24, scvDesc, (int)strlen(scvDesc));
            
            RECT rcBtnScv = { opX + 10, opY4 + 48, opX + opW - 10, opY4 + 74 };
            HBRUSH hBrBtnScv = CreateSolidBrush(d->cargoScavenged ? RGB(6, 78, 59) : RGB(120, 53, 15));
            FillRect(hdc, &rcBtnScv, hBrBtnScv);
            DeleteObject(hBrBtnScv);
            FrameRect(hdc, &rcBtnScv, (HBRUSH)GetStockObject(WHITE_BRUSH));
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, d->cargoScavenged ? RGB(110, 231, 183) : RGB(255, 255, 255));
            char scvTxt[64];
            sprintf(scvTxt, d->cargoScavenged ? "✔ CARGO FULLY SCAVENGED" : "SCAVENGE SCRAP PODS [KEY 4] (+%dT)", d->scrapPods);
            DrawTextA(hdc, scvTxt, -1, &rcBtnScv, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        
        // Footer instruction
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(245, 158, 11));
        TextOutA(hdc, mx + 16, my + modalH - 24, "Keys [1] Breach Airlock • [2] Decrypt Black Box • [3] Harvest Core • [4] Scavenge Cargo • [E / ESC] Close", 106);
        
        SelectObject(hdc, oldPenEva);
        SelectObject(hdc, oldBrushEva);
        DeleteObject(hPenBorderEva);
    }
    
    // Crisis Management & Damage Control Modal (Phase 9)
    if (g_state.showCrisis) {
        int modalW = 730;
        int modalH = 470;
        int mx = (totalW - modalW) / 2;
        int my = (totalH - modalH) / 2;
        
        RECT rcModal = { mx, my, mx + modalW, my + modalH };
        HBRUSH hBrModal = CreateSolidBrush(pal->bgPanel);
        FillRect(hdc, &rcModal, hBrModal);
        DeleteObject(hBrModal);
        
        COLORREF alertBorderCol = (g_state.breachCount > 0 || g_state.plasmaLeaks || g_state.radiation > 500.0f) ? RGB(239, 68, 68) : RGB(245, 158, 11);
        HPEN hPenBorderCrisis = CreatePen(PS_SOLID, 2, alertBorderCol);
        HGDIOBJ oldPenCrisis = SelectObject(hdc, hPenBorderCrisis);
        HGDIOBJ oldBrushCrisis = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, mx, my, mx + modalW, my + modalH);
        
        // Header
        RECT rcHdr = { mx, my, mx + modalW, my + 30 };
        HBRUSH hBrHdr = CreateSolidBrush(pal->bgHeader);
        FillRect(hdc, &rcHdr, hBrHdr);
        DeleteObject(hBrHdr);
        
        SelectObject(hdc, g_fontHeader);
        SetTextColor(hdc, alertBorderCol);
        TextOutA(hdc, mx + 14, my + 6, "HULL INTEGRITY & DAMAGE CONTROL STATION", 39);
        
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, pal->textBright);
        TextOutA(hdc, mx + modalW - 120, my + 9, "[K / ESC] CLOSE", 15);
        
        // Left Column: Ship Structural Schematic Wireframe Box (310x230)
        int schX = mx + 16;
        int schY = my + 40;
        int schW = 310;
        int schH = 230;
        RECT rcSch = { schX, schY, schX + schW, schY + schH };
        HBRUSH hBrSch = CreateSolidBrush(RGB(2, 6, 18));
        FillRect(hdc, &rcSch, hBrSch);
        DeleteObject(hBrSch);
        FrameRect(hdc, &rcSch, (HBRUSH)GetStockObject(WHITE_BRUSH));
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, pal->vector);
        TextOutA(hdc, schX + 8, schY + 6, "SHIP TACTICAL WIREFRAME", 23);
        
        // Draw 5 compartment boxes
        struct { int cx, cy, cw, ch; const char* label; } compBoxes[5] = {
            { schX + 105, schY + 80, 100, 45, "BRIDGE [O2]" },
            { schX + 105, schY + 30, 100, 40, "MINING PROW" },
            { schX + 15,  schY + 80, 80,  70, "PORT BULKHEAD" },
            { schX + 215, schY + 80, 80,  70, "STBD ORE VAULT" },
            { schX + 105, schY + 135, 100, 50, "REACTOR CORE" }
        };
        
        for (int c = 0; c < 5; c++) {
            int bx = compBoxes[c].cx;
            int by = compBoxes[c].cy;
            int bw = compBoxes[c].cw;
            int bh = compBoxes[c].ch;
            int breached = g_state.hullBreaches[c];
            
            RECT rcC = { bx, by, bx + bw, by + bh };
            HBRUSH hBrC = CreateSolidBrush(breached ? RGB(45, 10, 15) : RGB(6, 26, 40));
            FillRect(hdc, &rcC, hBrC);
            DeleteObject(hBrC);
            
            HPEN hPenC = CreatePen(PS_SOLID, 1, breached ? RGB(239, 68, 68) : (pal->vector));
            SelectObject(hdc, hPenC);
            Rectangle(hdc, bx, by, bx + bw, by + bh);
            DeleteObject(hPenC);
            
            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, breached ? RGB(244, 63, 94) : RGB(224, 242, 254));
            RECT rcLbl = { bx + 2, by + 4, bx + bw - 2, by + 20 };
            DrawTextA(hdc, compBoxes[c].label, -1, &rcLbl, DT_CENTER | DT_SINGLELINE);
            
            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, breached ? RGB(239, 68, 68) : RGB(16, 185, 129));
            RECT rcStat = { bx + 2, by + bh - 18, bx + bw - 2, by + bh - 2 };
            DrawTextA(hdc, breached ? "BREACHED [!]" : "AIRTIGHT", -1, &rcStat, DT_CENTER | DT_SINGLELINE);
        }
        
        // Compartment summary box below wireframe (310x150)
        int sumX = schX;
        int sumY = schY + schH + 10;
        int sumW = schW;
        int sumH = 140;
        RECT rcSum = { sumX, sumY, sumX + sumW, sumY + sumH };
        HBRUSH hBrSum = CreateSolidBrush(RGB(4, 10, 24));
        FillRect(hdc, &rcSum, hBrSum);
        DeleteObject(hBrSum);
        FrameRect(hdc, &rcSum, (HBRUSH)GetStockObject(WHITE_BRUSH));
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, pal->textBright);
        TextOutA(hdc, sumX + 8, sumY + 6, "LIFE SUPPORT & RADIATION DOSIMETRY", 34);
        
        SelectObject(hdc, g_fontSmall);
        char l1[64], l2[64], l3[64], l4[64], l5[64];
        sprintf(l1, "• Breaches Active: %d Compartments (%s)", g_state.breachCount, g_state.breachCount > 0 ? "ATMOSPHERE LEAKING" : "SEALED");
        sprintf(l2, "• Plasma Manifold: %s", g_state.plasmaLeaks ? "CONDUIT FIRE / RUPTURE [!]" : "NOMINAL PRESSURE");
        sprintf(l3, "• Radiation Dosage: %d Rads (%s)", (int)g_state.radiation, g_state.radiation > 500.0f ? "LETHAL DOSE" : (g_state.radiation > 200.0f ? "WARNING" : "SAFE"));
        sprintf(l4, "• O2 Scrubbers: %d%% Efficiency (Drain: %.2f%%/s)", (int)g_state.o2Scrubber, 0.05f + g_state.breachCount * 0.40f);
        sprintf(l5, "• Repair Drones: %s", g_state.repairDronesActive ? "DEPLOYED (SWARM ACTIVE)" : "DOCKED IN HANGAR");
        
        SetTextColor(hdc, g_state.breachCount > 0 ? RGB(239, 68, 68) : RGB(148, 163, 184));
        TextOutA(hdc, sumX + 8, sumY + 26, l1, (int)strlen(l1));
        SetTextColor(hdc, g_state.plasmaLeaks ? RGB(244, 63, 94) : RGB(148, 163, 184));
        TextOutA(hdc, sumX + 8, sumY + 46, l2, (int)strlen(l2));
        SetTextColor(hdc, g_state.radiation > 500.0f ? RGB(239, 68, 68) : (g_state.radiation > 200.0f ? RGB(245, 158, 11) : RGB(148, 163, 184)));
        TextOutA(hdc, sumX + 8, sumY + 66, l3, (int)strlen(l3));
        SetTextColor(hdc, g_state.o2Scrubber < 30.0f ? RGB(239, 68, 68) : RGB(148, 163, 184));
        TextOutA(hdc, sumX + 8, sumY + 86, l4, (int)strlen(l4));
        SetTextColor(hdc, g_state.repairDronesActive ? RGB(56, 189, 248) : RGB(148, 163, 184));
        TextOutA(hdc, sumX + 8, sumY + 106, l5, (int)strlen(l5));
        
        // Right Column: 5 Interactive Crisis Actions (370px wide)
        int actX = mx + 340;
        int actY = my + 40;
        int actW = modalW - 356;
        int cardH = 70;
        int gapY = 8;
        
        // 1. Seal Breaches Card
        RECT rcA1 = { actX, actY, actX + actW, actY + cardH };
        HBRUSH hBrA1 = CreateSolidBrush(g_state.breachCount > 0 ? RGB(36, 10, 14) : RGB(5, 12, 28));
        FillRect(hdc, &rcA1, hBrA1);
        DeleteObject(hBrA1);
        HPEN hPenA1 = CreatePen(PS_SOLID, 1, g_state.breachCount > 0 ? RGB(239, 68, 68) : pal->borderPanel);
        SelectObject(hdc, hPenA1);
        Rectangle(hdc, actX, actY, actX + actW, actY + cardH);
        DeleteObject(hPenA1);
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, g_state.breachCount > 0 ? RGB(239, 68, 68) : RGB(255, 255, 255));
        TextOutA(hdc, actX + 10, actY + 6, "[1] EMERGENCY HULL WELD & SEAL", 30);
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(148, 163, 184));
        TextOutA(hdc, actX + 10, actY + 22, "Nanite weld sealant (Cost: 3 Scrap / 5 Ferrum / 50 CR per breach). +15% Hull.", 77);
        
        RECT rcBtnA1 = { actX + 10, actY + 40, actX + actW - 10, actY + 64 };
        HBRUSH hBrBtnA1 = CreateSolidBrush(g_state.breachCount > 0 ? RGB(153, 27, 27) : RGB(30, 41, 59));
        FillRect(hdc, &rcBtnA1, hBrBtnA1);
        DeleteObject(hBrBtnA1);
        FrameRect(hdc, &rcBtnA1, (HBRUSH)GetStockObject(WHITE_BRUSH));
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, g_state.breachCount > 0 ? RGB(255, 255, 255) : RGB(148, 163, 184));
        char b1Txt[64];
        sprintf(b1Txt, g_state.breachCount > 0 ? "SEAL %d BREACHES [KEY 1]" : "HULL IS CURRENTLY AIRTIGHT", g_state.breachCount);
        DrawTextA(hdc, b1Txt, -1, &rcBtnA1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        // 2. Vent Plasma Card
        int actY2 = actY + cardH + gapY;
        RECT rcA2 = { actX, actY2, actX + actW, actY2 + cardH };
        HBRUSH hBrA2 = CreateSolidBrush(g_state.plasmaLeaks ? RGB(36, 16, 8) : RGB(5, 12, 28));
        FillRect(hdc, &rcA2, hBrA2);
        DeleteObject(hBrA2);
        HPEN hPenA2 = CreatePen(PS_SOLID, 1, g_state.plasmaLeaks ? RGB(245, 158, 11) : pal->borderPanel);
        SelectObject(hdc, hPenA2);
        Rectangle(hdc, actX, actY2, actX + actW, actY2 + cardH);
        DeleteObject(hPenA2);
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, g_state.plasmaLeaks ? RGB(245, 158, 11) : RGB(255, 255, 255));
        TextOutA(hdc, actX + 10, actY2 + 6, "[2] EMERGENCY PLASMA CONDUIT PURGE", 34);
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(148, 163, 184));
        TextOutA(hdc, actX + 10, actY2 + 22, "Dump superheated plasma into vacuum. Dumps -45% Heat & clears leaks.", 68);
        
        RECT rcBtnA2 = { actX + 10, actY2 + 40, actX + actW - 10, actY2 + 64 };
        HBRUSH hBrBtnA2 = CreateSolidBrush(g_state.plasmaLeaks ? RGB(180, 83, 9) : RGB(30, 58, 138));
        FillRect(hdc, &rcBtnA2, hBrBtnA2);
        DeleteObject(hBrBtnA2);
        FrameRect(hdc, &rcBtnA2, (HBRUSH)GetStockObject(WHITE_BRUSH));
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, RGB(255, 255, 255));
        DrawTextA(hdc, g_state.plasmaLeaks ? "PURGE PLASMA CONDUIT [KEY 2] (-45% HEAT)" : "VENT PLASMA MANIFOLD [KEY 2] (-45% HEAT)", -1, &rcBtnA2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        // 3. Flush Radiation Card
        int actY3 = actY2 + cardH + gapY;
        RECT rcA3 = { actX, actY3, actX + actW, actY3 + cardH };
        HBRUSH hBrA3 = CreateSolidBrush(RGB(5, 12, 28));
        FillRect(hdc, &rcA3, hBrA3);
        DeleteObject(hBrA3);
        HPEN hPenA3 = CreatePen(PS_SOLID, 1, pal->borderPanel);
        SelectObject(hdc, hPenA3);
        Rectangle(hdc, actX, actY3, actX + actW, actY3 + cardH);
        DeleteObject(hPenA3);
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, RGB(255, 255, 255));
        TextOutA(hdc, actX + 10, actY3 + 6, "[3] LEAD NANO-SCRUBBER RADIATION PURGE", 38);
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(148, 163, 184));
        TextOutA(hdc, actX + 10, actY3 + 22, "Flush ion contaminants from crew quarters. Clears all Rads to 0.", 64);
        
        RECT rcBtnA3 = { actX + 10, actY3 + 40, actX + actW - 10, actY3 + 64 };
        HBRUSH hBrBtnA3 = CreateSolidBrush(g_state.radiation > 0 ? RGB(6, 78, 59) : RGB(30, 41, 59));
        FillRect(hdc, &rcBtnA3, hBrBtnA3);
        DeleteObject(hBrBtnA3);
        FrameRect(hdc, &rcBtnA3, (HBRUSH)GetStockObject(WHITE_BRUSH));
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, g_state.radiation > 0 ? RGB(110, 231, 183) : RGB(148, 163, 184));
        char b3Txt[64];
        sprintf(b3Txt, "PURGE DOSIMETER (%d RADS) [KEY 3]", (int)g_state.radiation);
        DrawTextA(hdc, b3Txt, -1, &rcBtnA3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        // 4. Service O2 Scrubbers Card
        int actY4 = actY3 + cardH + gapY;
        RECT rcA4 = { actX, actY4, actX + actW, actY4 + cardH };
        HBRUSH hBrA4 = CreateSolidBrush(RGB(5, 12, 28));
        FillRect(hdc, &rcA4, hBrA4);
        DeleteObject(hBrA4);
        HPEN hPenA4 = CreatePen(PS_SOLID, 1, pal->borderPanel);
        SelectObject(hdc, hPenA4);
        Rectangle(hdc, actX, actY4, actX + actW, actY4 + cardH);
        DeleteObject(hPenA4);
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, RGB(255, 255, 255));
        TextOutA(hdc, actX + 10, actY4 + 6, "[4] O2 SCRUBBER RECALIBRATION & SYNTHESIS", 41);
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(148, 163, 184));
        TextOutA(hdc, actX + 10, actY4 + 22, "Cleanse CO2 filters & synthesize fresh Oxygen (+35% O2 / 100% Scrubbers).", 73);
        
        RECT rcBtnA4 = { actX + 10, actY4 + 40, actX + actW - 10, actY4 + 64 };
        HBRUSH hBrBtnA4 = CreateSolidBrush(RGB(6, 78, 59));
        FillRect(hdc, &rcBtnA4, hBrBtnA4);
        DeleteObject(hBrBtnA4);
        FrameRect(hdc, &rcBtnA4, (HBRUSH)GetStockObject(WHITE_BRUSH));
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, RGB(110, 231, 183));
        DrawTextA(hdc, "SERVICE SCRUBBERS & INJECT O2 [KEY 4]", -1, &rcBtnA4, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        // 5. Deploy Repair Drones Card
        int actY5 = actY4 + cardH + gapY;
        RECT rcA5 = { actX, actY5, actX + actW, actY5 + cardH };
        HBRUSH hBrA5 = CreateSolidBrush(RGB(5, 12, 28));
        FillRect(hdc, &rcA5, hBrA5);
        DeleteObject(hBrA5);
        HPEN hPenA5 = CreatePen(PS_SOLID, 1, pal->borderPanel);
        SelectObject(hdc, hPenA5);
        Rectangle(hdc, actX, actY5, actX + actW, actY5 + cardH);
        DeleteObject(hPenA5);
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, RGB(255, 255, 255));
        TextOutA(hdc, actX + 10, actY5 + 6, "[5] DEPLOY REPAIR DRONE SWARM", 29);
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(148, 163, 184));
        TextOutA(hdc, actX + 10, actY5 + 22, "Launch autonomous drones: Restores +30% Hull & +50% Shield over 10s.", 68);
        
        RECT rcBtnA5 = { actX + 10, actY5 + 40, actX + actW - 10, actY5 + 64 };
        HBRUSH hBrBtnA5 = CreateSolidBrush(g_state.repairDronesActive ? RGB(30, 41, 59) : RGB(30, 58, 138));
        FillRect(hdc, &rcBtnA5, hBrBtnA5);
        DeleteObject(hBrBtnA5);
        FrameRect(hdc, &rcBtnA5, (HBRUSH)GetStockObject(WHITE_BRUSH));
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, g_state.repairDronesActive ? RGB(148, 163, 184) : RGB(255, 255, 255));
        char b5Txt[64];
        sprintf(b5Txt, g_state.repairDronesActive ? "SWARM ACTIVE (%.1fs REMAINING)" : "DEPLOY DRONE SWARM [KEY 5]", g_state.droneTimer);
        DrawTextA(hdc, b5Txt, -1, &rcBtnA5, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        // Footer instruction
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(245, 158, 11));
        TextOutA(hdc, mx + 16, my + modalH - 24, "Keys [1] Seal Breaches • [2] Vent Plasma • [3] Flush Rads • [4] O2 Scrubbers • [5] Drones • [K / ESC] Close", 108);
        
        SelectObject(hdc, oldPenCrisis);
        SelectObject(hdc, oldBrushCrisis);
        DeleteObject(hPenBorderCrisis);
    }
    
    // Phase 10: Orbital Refinery & Metallurgical Smelting Lab Modal
    if (g_state.showRefinery) {
        int modalW = 760;
        int modalH = 490;
        int mx = (totalW - modalW) / 2;
        int my = (totalH - modalH) / 2;
        
        RECT rcModal = { mx, my, mx + modalW, my + modalH };
        HBRUSH hBrModal = CreateSolidBrush(pal->bgPanel);
        FillRect(hdc, &rcModal, hBrModal);
        DeleteObject(hBrModal);
        
        HPEN hPenBorderRef = CreatePen(PS_SOLID, 2, RGB(245, 158, 11));
        HGDIOBJ oldPenRef = SelectObject(hdc, hPenBorderRef);
        HGDIOBJ oldBrushRef = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, mx, my, mx + modalW, my + modalH);
        
        // Header
        RECT rcHdr = { mx, my, mx + modalW, my + 30 };
        HBRUSH hBrHdr = CreateSolidBrush(pal->bgHeader);
        FillRect(hdc, &rcHdr, hBrHdr);
        DeleteObject(hBrHdr);
        
        SelectObject(hdc, g_fontHeader);
        SetTextColor(hdc, RGB(251, 191, 36));
        TextOutA(hdc, mx + 14, my + 6, "ORBITAL METALLURGICAL REFINERY & FUEL LAB", 42);
        
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, pal->textBright);
        TextOutA(hdc, mx + modalW - 120, my + 9, "[R / ESC] CLOSE", 15);
        
        // Top Status Bar (Controls: Catalytic Boost, Refine All, Sell All)
        int barY = my + 34;
        RECT rcStat = { mx + 14, barY, mx + modalW - 14, barY + 30 };
        HBRUSH hBrStat = CreateSolidBrush(RGB(3, 7, 18));
        FillRect(hdc, &rcStat, hBrStat);
        DeleteObject(hBrStat);
        FrameRect(hdc, &rcStat, (HBRUSH)GetStockObject(WHITE_BRUSH));
        
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(245, 158, 11));
        TextOutA(hdc, mx + 20, barY + 8, "CRUCIBLE: 1,850°C", 17);
        
        // Catalytic Boost button
        int bBtnX = mx + 150;
        int bBtnW = 195;
        RECT rcBoost = { bBtnX, barY + 4, bBtnX + bBtnW, barY + 26 };
        HBRUSH hBrBoost = CreateSolidBrush(g_state.catalyticBoost ? RGB(6, 78, 59) : RGB(20, 20, 30));
        FillRect(hdc, &rcBoost, hBrBoost);
        DeleteObject(hBrBoost);
        FrameRect(hdc, &rcBoost, (HBRUSH)GetStockObject(WHITE_BRUSH));
        SetTextColor(hdc, g_state.catalyticBoost ? RGB(110, 231, 183) : RGB(148, 163, 184));
        DrawTextA(hdc, g_state.catalyticBoost ? "⚡ BOOST [B]: ON (+25% YIELD)" : "⚡ BOOST [B]: OFF", -1, &rcBoost, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        // Refine All button
        int rBtnX = bBtnX + bBtnW + 8;
        int rBtnW = 180;
        RECT rcRefAll = { rBtnX, barY + 4, rBtnX + rBtnW, barY + 26 };
        HBRUSH hBrRefAll = CreateSolidBrush(RGB(30, 58, 138));
        FillRect(hdc, &rcRefAll, hBrRefAll);
        DeleteObject(hBrRefAll);
        FrameRect(hdc, &rcRefAll, (HBRUSH)GetStockObject(WHITE_BRUSH));
        SetTextColor(hdc, RGB(240, 249, 255));
        DrawTextA(hdc, "🔥 REFINE ALL ORES [A]", -1, &rcRefAll, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        // Sell All button
        int sBtnX = rBtnX + rBtnW + 8;
        int sBtnW = modalW - (sBtnX - mx) - 20;
        RECT rcSellAll = { sBtnX, barY + 4, sBtnX + sBtnW, barY + 26 };
        HBRUSH hBrSellAll = CreateSolidBrush(RGB(120, 53, 15));
        FillRect(hdc, &rcSellAll, hBrSellAll);
        DeleteObject(hBrSellAll);
        FrameRect(hdc, &rcSellAll, (HBRUSH)GetStockObject(WHITE_BRUSH));
        SetTextColor(hdc, RGB(254, 240, 138));
        DrawTextA(hdc, "💰 SELL ALL [S]", -1, &rcSellAll, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        // Left Column: Crucible Animation (290x120) & Refined Vault (290x230)
        int leftX = mx + 14;
        int leftY = barY + 36;
        int leftW = 290;
        
        // Crucible Frame
        int crucH = 120;
        RECT rcCruc = { leftX, leftY, leftX + leftW, leftY + crucH };
        HBRUSH hBrCruc = CreateSolidBrush(RGB(2, 6, 18));
        FillRect(hdc, &rcCruc, hBrCruc);
        DeleteObject(hBrCruc);
        HPEN hPenCruc = CreatePen(PS_SOLID, 1, RGB(56, 189, 248));
        SelectObject(hdc, hPenCruc);
        Rectangle(hdc, leftX, leftY, leftX + leftW, leftY + crucH);
        DeleteObject(hPenCruc);
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, RGB(251, 191, 36));
        TextOutA(hdc, leftX + 8, leftY + 6, "PLASMA INDUCTION CRUCIBLE", 25);
        
        // Coils
        HPEN hPenCoil = CreatePen(PS_SOLID, 1, g_state.catalyticBoost ? RGB(245, 158, 11) : RGB(56, 189, 248));
        SelectObject(hdc, hPenCoil);
        for (int cy = leftY + 28; cy < leftY + crucH - 25; cy += 14) {
            MoveToEx(hdc, leftX + 15, cy, NULL);
            for (int cx = leftX + 15; cx < leftX + leftW - 15; cx += 5) {
                int wy = cy + (int)(sin((cx * 0.08f) + g_state.crucibleAnimTime * 3.0f) * 3.0f);
                LineTo(hdc, cx, wy);
            }
        }
        DeleteObject(hPenCoil);
        
        // Molten pool
        RECT rcPool = { leftX + 25, leftY + crucH - 22, leftX + leftW - 25, leftY + crucH - 6 };
        HBRUSH hBrPool = CreateSolidBrush(g_state.catalyticBoost ? RGB(239, 68, 68) : RGB(245, 158, 11));
        FillRect(hdc, &rcPool, hBrPool);
        DeleteObject(hBrPool);
        
        // Refined Inventory Vault
        int vaultY = leftY + crucH + 10;
        int vaultH = modalH - (vaultY - my) - 30;
        RECT rcVault = { leftX, vaultY, leftX + leftW, vaultY + vaultH };
        HBRUSH hBrVault = CreateSolidBrush(RGB(4, 10, 24));
        FillRect(hdc, &rcVault, hBrVault);
        DeleteObject(hBrVault);
        FrameRect(hdc, &rcVault, (HBRUSH)GetStockObject(WHITE_BRUSH));
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, pal->textBright);
        TextOutA(hdc, leftX + 8, vaultY + 6, "REFINED PRODUCTS STORAGE VAULT", 30);
        
        SelectObject(hdc, g_fontSmall);
        int vy = vaultY + 26;
        int totalRefinedVal = 0;
        for (int i = 0; i < 6; i++) {
            const RefineryRecipeDef* r = &REFINERY_RECIPES[i];
            int count = g_state.refined[i];
            int val = count * r->value;
            totalRefinedVal += val;
            
            char itemStr[64];
            sprintf(itemStr, "%s: %d (Val: %d CR)", r->outputName, count, val);
            SetTextColor(hdc, count > 0 ? r->color : RGB(100, 116, 139));
            TextOutA(hdc, leftX + 10, vy, itemStr, (int)strlen(itemStr));
            
            if (r->usable && count > 0) {
                RECT rcUse = { leftX + leftW - 65, vy - 1, leftX + leftW - 8, vy + 15 };
                HBRUSH hBrUse = CreateSolidBrush(RGB(6, 78, 59));
                FillRect(hdc, &rcUse, hBrUse);
                DeleteObject(hBrUse);
                FrameRect(hdc, &rcUse, (HBRUSH)GetStockObject(WHITE_BRUSH));
                SetTextColor(hdc, RGB(110, 231, 183));
                DrawTextA(hdc, "USE", -1, &rcUse, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            vy += 22;
        }
        
        char vTotalStr[64];
        sprintf(vTotalStr, "TOTAL VAULT VALUE: %d CR", totalRefinedVal);
        SetTextColor(hdc, RGB(251, 191, 36));
        TextOutA(hdc, leftX + 10, vaultY + vaultH - 20, vTotalStr, (int)strlen(vTotalStr));
        
        // Right Column: 6 Smelting Recipe Cards
        int rcX = leftX + leftW + 12;
        int rcY = leftY;
        int rcW = modalW - (rcX - mx) - 14;
        int rCardH = 60;
        int rGapY = 5;
        
        for (int i = 0; i < 6; i++) {
            const RefineryRecipeDef* r = &REFINERY_RECIPES[i];
            int cyCard = rcY + i * (rCardH + rGapY);
            
            int maxCraft = 999;
            for (int k = 0; k < 6; k++) {
                if (r->inputs[k] > 0) {
                    int possible = g_state.cargoHold[k] / r->inputs[k];
                    if (possible < maxCraft) maxCraft = possible;
                }
            }
            int canCraft = (maxCraft > 0);
            
            RECT rcCard = { rcX, cyCard, rcX + rcW, cyCard + rCardH };
            HBRUSH hBrCard = CreateSolidBrush(RGB(5, 12, 28));
            FillRect(hdc, &rcCard, hBrCard);
            DeleteObject(hBrCard);
            HPEN hPenCard = CreatePen(PS_SOLID, 1, canCraft ? pal->borderPanel : RGB(40, 45, 60));
            SelectObject(hdc, hPenCard);
            Rectangle(hdc, rcX, cyCard, rcX + rcW, cyCard + rCardH);
            DeleteObject(hPenCard);
            
            // Recipe Title & Key
            char rTitle[64];
            sprintf(rTitle, "[%d] %s", i + 1, r->name);
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, canCraft ? r->color : RGB(148, 163, 184));
            TextOutA(hdc, rcX + 8, cyCard + 4, rTitle, (int)strlen(rTitle));
            
            // Value tag
            char vTag[32];
            sprintf(vTag, "%d CR", r->value);
            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, RGB(251, 191, 36));
            TextOutA(hdc, rcX + rcW - 65, cyCard + 4, vTag, (int)strlen(vTag));
            
            // Requirements & Can Smelt
            char reqStr[96];
            sprintf(reqStr, "Req: %s  | Can Smelt: %d Units", r->inputStr, maxCraft);
            SetTextColor(hdc, canCraft ? RGB(16, 185, 129) : RGB(239, 68, 68));
            TextOutA(hdc, rcX + 8, cyCard + 20, reqStr, (int)strlen(reqStr));
            
            // Smelt 1x and Smelt Max buttons
            int b1X = rcX + 8;
            int b1Y = cyCard + 36;
            int b1W = 120;
            int b1H = 20;
            RECT rcB1 = { b1X, b1Y, b1X + b1W, b1Y + b1H };
            HBRUSH hBrB1 = CreateSolidBrush(canCraft ? RGB(15, 39, 68) : RGB(20, 25, 35));
            FillRect(hdc, &rcB1, hBrB1);
            DeleteObject(hBrB1);
            FrameRect(hdc, &rcB1, (HBRUSH)GetStockObject(WHITE_BRUSH));
            SetTextColor(hdc, canCraft ? RGB(56, 189, 248) : RGB(100, 116, 139));
            char b1Label[32];
            sprintf(b1Label, "SMELT 1x [%d]", i + 1);
            DrawTextA(hdc, b1Label, -1, &rcB1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
            int bMaxX = b1X + b1W + 8;
            int bMaxW = 130;
            RECT rcBMax = { bMaxX, b1Y, bMaxX + bMaxW, b1Y + b1H };
            HBRUSH hBrBMax = CreateSolidBrush(canCraft ? RGB(30, 58, 138) : RGB(20, 25, 35));
            FillRect(hdc, &rcBMax, hBrBMax);
            DeleteObject(hBrBMax);
            FrameRect(hdc, &rcBMax, (HBRUSH)GetStockObject(WHITE_BRUSH));
            SetTextColor(hdc, canCraft ? RGB(110, 231, 183) : RGB(100, 116, 139));
            char bMaxLabel[32];
            sprintf(bMaxLabel, "SMELT MAX (%dx)", maxCraft);
            DrawTextA(hdc, bMaxLabel, -1, &rcBMax, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        
        // Footer instruction
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(245, 158, 11));
        TextOutA(hdc, mx + 16, my + modalH - 24, "Keys [1-6] Smelt 1x • [B] Toggle Boost • [A] Refine All • [S] Sell All • [R / ESC] Close", 88);
        
        SelectObject(hdc, oldPenRef);
        SelectObject(hdc, oldBrushRef);
        DeleteObject(hPenBorderRef);
    }
    
    // Phase 11: Orbital Spaceport & Black Market Trade Station Modal
    if (g_state.showStation) {
        int modalW = 760;
        int modalH = 480;
        int mx = (totalW - modalW) / 2;
        int my = (totalH - modalH) / 2;
        
        const StationDef* curSt = &STATION_DEFS[g_state.currentSectorIndex];
        
        RECT rcModal = { mx, my, mx + modalW, my + modalH };
        HBRUSH hBrModal = CreateSolidBrush(pal->bgPanel);
        HPEN hPenBorderSt = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
        HGDIOBJ oldPenSt = SelectObject(hdc, hPenBorderSt);
        HGDIOBJ oldBrushSt = SelectObject(hdc, hBrModal);
        
        Rectangle(hdc, mx, my, mx + modalW, my + modalH);
        DeleteObject(hBrModal);
        
        // Header Banner
        RECT rcHdr = { mx, my, mx + modalW, my + 32 };
        HBRUSH hBrHdr = CreateSolidBrush(pal->bgHeader);
        FillRect(hdc, &rcHdr, hBrHdr);
        DeleteObject(hBrHdr);
        
        SelectObject(hdc, g_fontHeader);
        SetTextColor(hdc, RGB(56, 189, 248));
        TextOutA(hdc, mx + 14, my + 6, curSt->name, (int)strlen(curSt->name));
        
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, pal->textBright);
        TextOutA(hdc, mx + modalW - 120, my + 10, "[D / ESC] CLOSE", 15);
        
        // Faction & Tariff Sub-header Banner
        int barY = my + 36;
        RECT rcStat = { mx + 14, barY, mx + modalW - 14, barY + 32 };
        HBRUSH hBrStat = CreateSolidBrush(RGB(3, 7, 18));
        FillRect(hdc, &rcStat, hBrStat);
        DeleteObject(hBrStat);
        FrameRect(hdc, &rcStat, (HBRUSH)GetStockObject(WHITE_BRUSH));
        
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(251, 191, 36));
        char repStr[96];
        sprintf(repStr, "FACTION: %s  |  STANDING: %d REP  |  STATUS: %s", curSt->faction, g_state.factionRep[g_state.currentSectorIndex], g_state.stationDocked ? "DOCKED" : "ORBITING (REMOTE)");
        TextOutA(hdc, mx + 20, barY + 4, repStr, (int)strlen(repStr));
        
        SetTextColor(hdc, RGB(56, 189, 248));
        TextOutA(hdc, mx + 20, barY + 17, curSt->tariffText, (int)strlen(curSt->tariffText));
        
        // Left Column: Quick Services & Black Market Barter (320px wide)
        int leftX = mx + 14;
        int leftY = barY + 38;
        int leftW = 340;
        
        // Station Services Box
        int srvH = 195;
        RECT rcSrv = { leftX, leftY, leftX + leftW, leftY + srvH };
        HBRUSH hBrSrv = CreateSolidBrush(RGB(2, 6, 18));
        FillRect(hdc, &rcSrv, hBrSrv);
        DeleteObject(hBrSrv);
        HPEN hPenSrv = CreatePen(PS_SOLID, 1, RGB(30, 58, 138));
        SelectObject(hdc, hPenSrv);
        Rectangle(hdc, leftX, leftY, leftX + leftW, leftY + srvH);
        DeleteObject(hPenSrv);
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, RGB(56, 189, 248));
        TextOutA(hdc, leftX + 8, leftY + 6, "STATION DRYDOCK & MAINTENANCE", 29);
        
        // 4 Service Buttons (Repair, Shield, Refuel, Decon)
        int srvBtnH = 34;
        int srvGap = 6;
        for (int s = 0; s < 4; s++) {
            int sy = leftY + 28 + s * (srvBtnH + srvGap);
            RECT rcSBtn = { leftX + 8, sy, leftX + leftW - 8, sy + srvBtnH };
            HBRUSH hBrSBtn = CreateSolidBrush(RGB(15, 23, 42));
            FillRect(hdc, &rcSBtn, hBrSBtn);
            DeleteObject(hBrSBtn);
            FrameRect(hdc, &rcSBtn, (HBRUSH)GetStockObject(WHITE_BRUSH));
            
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, RGB(255, 255, 255));
            char sTitle[64];
            char sCost[32];
            if (s == 0) {
                float missH = g_state.maxHull - g_state.hull;
                int cst = (int)ceil(missH * (float)curSt->repairCostPerPct);
                sprintf(sTitle, "[1] REPAIR HULL ARMOR");
                sprintf(sCost, "%d CR", cst);
            } else if (s == 1) {
                sprintf(sTitle, "[2] CHARGE SHIELD CAPACITOR");
                sprintf(sCost, "%d CR", curSt->shieldCost);
            } else if (s == 2) {
                float missF = g_state.maxFuel - g_state.fuel;
                int cst = (int)ceil(missF * (float)curSt->fuelCostPerPct);
                sprintf(sTitle, "[3] REFUEL PROPELLANT");
                sprintf(sCost, "%d CR", cst);
            } else {
                sprintf(sTitle, "[4] DECONTAMINATE & O2 BIO-BAY");
                sprintf(sCost, "%d CR", curSt->deconCost);
            }
            TextOutA(hdc, leftX + 12, sy + 4, sTitle, (int)strlen(sTitle));
            
            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, RGB(251, 191, 36));
            RECT rcCost = { leftX + leftW - 85, sy + 4, leftX + leftW - 12, sy + 18 };
            DrawTextA(hdc, sCost, -1, &rcCost, DT_RIGHT | DT_SINGLELINE);
            
            SetTextColor(hdc, RGB(148, 163, 184));
            const char* sSub[4] = {
                "Weld nanite plating & seal breaches",
                "Supercharge deflector capacitor bank",
                "Pump pressurized hydrazine fuel",
                "Flush rads & replenish pure oxygen"
            };
            TextOutA(hdc, leftX + 12, sy + 18, sSub[s], (int)strlen(sSub[s]));
        }
        
        // Black Market Barter Box (Below Services)
        int barY2 = leftY + srvH + 8;
        int barH2 = modalH - (barY2 - my) - 30;
        RECT rcBarter = { leftX, barY2, leftX + leftW, barY2 + barH2 };
        HBRUSH hBrBarter = CreateSolidBrush(RGB(2, 6, 18));
        FillRect(hdc, &rcBarter, hBrBarter);
        DeleteObject(hBrBarter);
        HPEN hPenBrt = CreatePen(PS_SOLID, 1, RGB(120, 53, 15));
        SelectObject(hdc, hPenBrt);
        Rectangle(hdc, leftX, barY2, leftX + leftW, barY2 + barH2);
        DeleteObject(hPenBrt);
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, RGB(245, 158, 11));
        TextOutA(hdc, leftX + 8, barY2 + 6, "BLACK MARKET COMMODITY EXCHANGE", 31);
        
        // Sell All with Tariff button
        RECT rcSellTariff = { leftX + 8, barY2 + 24, leftX + leftW - 8, barY2 + 50 };
        HBRUSH hBrStBtn = CreateSolidBrush(RGB(120, 53, 15));
        FillRect(hdc, &rcSellTariff, hBrStBtn);
        DeleteObject(hBrStBtn);
        FrameRect(hdc, &rcSellTariff, (HBRUSH)GetStockObject(WHITE_BRUSH));
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, RGB(254, 240, 138));
        DrawTextA(hdc, "💰 SELL ALL CARGO (WITH TARIFFS) [S]", -1, &rcSellTariff, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        // 3 Black Market Quick Purchase Supply Buttons
        int supY = barY2 + 54;
        int supW = (leftW - 20) / 3;
        const char* supNames[3] = { "2x WARP", "2x NANITE", "2x O2" };
        const char* supCosts[3] = { "1.4k CR", "1.2k CR", "400 CR" };
        for (int p = 0; p < 3; p++) {
            int px = leftX + 8 + p * (supW + 2);
            RECT rcSup = { px, supY, px + supW, supY + 28 };
            HBRUSH hBrSup = CreateSolidBrush(RGB(15, 23, 42));
            FillRect(hdc, &rcSup, hBrSup);
            DeleteObject(hBrSup);
            FrameRect(hdc, &rcSup, (HBRUSH)GetStockObject(WHITE_BRUSH));
            
            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, RGB(56, 189, 248));
            TextOutA(hdc, px + 4, supY + 3, supNames[p], (int)strlen(supNames[p]));
            SetTextColor(hdc, RGB(251, 191, 36));
            TextOutA(hdc, px + 4, supY + 15, supCosts[p], (int)strlen(supCosts[p]));
        }
        
        // Right Column: 3 Dredging Contracts (380px wide)
        int rcX = leftX + leftW + 12;
        int rcY = leftY;
        int rcW = modalW - (rcX - mx) - 14;
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, RGB(56, 189, 248));
        TextOutA(hdc, rcX + 4, rcY - 14, "SECTOR DREDGING CONTRACTS GUILD", 31);
        
        int cCardH = 110;
        int cGapY = 8;
        for (int c = 0; c < 3; c++) {
            const ContractDef* cd = &curSt->contracts[c];
            int cyCard = rcY + c * (cCardH + cGapY);
            int isDone = g_state.contractDone[g_state.currentSectorIndex][c];
            
            int have = cd->reqRefined ? g_state.refined[cd->reqItemIdx] : g_state.cargoHold[cd->reqItemIdx];
            int canClaim = (!isDone && have >= cd->reqQty);
            
            RECT rcCCard = { rcX, cyCard, rcX + rcW, cyCard + cCardH };
            HBRUSH hBrCCard = CreateSolidBrush(isDone ? RGB(6, 30, 20) : (canClaim ? RGB(10, 25, 50) : RGB(5, 12, 28)));
            FillRect(hdc, &rcCCard, hBrCCard);
            DeleteObject(hBrCCard);
            HPEN hPenCCard = CreatePen(PS_SOLID, 1, isDone ? RGB(16, 185, 129) : (canClaim ? RGB(56, 189, 248) : RGB(40, 45, 60)));
            SelectObject(hdc, hPenCCard);
            Rectangle(hdc, rcX, cyCard, rcX + rcW, cyCard + cCardH);
            DeleteObject(hPenCCard);
            
            // Contract Title & Key
            char cTitle[64];
            sprintf(cTitle, "[%d] %s", c + 5, cd->title);
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, isDone ? RGB(16, 185, 129) : (canClaim ? RGB(56, 189, 248) : RGB(255, 255, 255)));
            TextOutA(hdc, rcX + 8, cyCard + 6, cTitle, (int)strlen(cTitle));
            
            // Reward tag
            char rewStr[64];
            sprintf(rewStr, "+%d CR | +%d REP", cd->rewardCredits, cd->rewardRep);
            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, RGB(251, 191, 36));
            RECT rcRew = { rcX + rcW - 130, cyCard + 6, rcX + rcW - 8, cyCard + 20 };
            DrawTextA(hdc, rewStr, -1, &rcRew, DT_RIGHT | DT_SINGLELINE);
            
            // Description
            SetTextColor(hdc, RGB(148, 163, 184));
            RECT rcDesc = { rcX + 8, cyCard + 24, rcX + rcW - 8, cyCard + 54 };
            DrawTextA(hdc, cd->desc, -1, &rcDesc, DT_WORDBREAK);
            
            // Requirements & Status
            char reqStr[96];
            sprintf(reqStr, "Requisition: %s (%d / %d in hold)", cd->reqLabel, have, cd->reqQty);
            SetTextColor(hdc, isDone ? RGB(16, 185, 129) : (have >= cd->reqQty ? RGB(56, 189, 248) : RGB(239, 68, 68)));
            TextOutA(hdc, rcX + 8, cyCard + 58, reqStr, (int)strlen(reqStr));
            
            // Claim button
            RECT rcClaim = { rcX + 8, cyCard + 78, rcX + rcW - 8, cyCard + 102 };
            HBRUSH hBrClaim = CreateSolidBrush(isDone ? RGB(6, 78, 59) : (canClaim ? RGB(30, 58, 138) : RGB(20, 25, 35)));
            FillRect(hdc, &rcClaim, hBrClaim);
            DeleteObject(hBrClaim);
            FrameRect(hdc, &rcClaim, (HBRUSH)GetStockObject(WHITE_BRUSH));
            
            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, isDone ? RGB(110, 231, 183) : (canClaim ? RGB(240, 249, 255) : RGB(100, 116, 139)));
            char claimTxt[64];
            if (isDone) sprintf(claimTxt, "✓ CONTRACT FULFILLED");
            else if (canClaim) sprintf(claimTxt, "CLAIM CONTRACT REWARD [%d] (+%d CR)", c + 5, cd->rewardCredits);
            else sprintf(claimTxt, "INCOMPLETE: %d / %d DELIVERED", have, cd->reqQty);
            DrawTextA(hdc, claimTxt, -1, &rcClaim, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        
        // Footer instruction
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(245, 158, 11));
        TextOutA(hdc, mx + 16, my + modalH - 24, "Keys [1-4] Services • [5-7] Claim Contracts • [S] Sell All Tariffs • [U] Undock • [D / ESC] Close", 99);
        
        SelectObject(hdc, oldPenSt);
        SelectObject(hdc, oldBrushSt);
        DeleteObject(hPenBorderSt);
    }
    
    // Phase 12: Void Pirate Defense & Weapon Systems Armory Modal
    if (g_state.showDefense) {
        int modalW = 760;
        int modalH = 490;
        int mx = (totalW - modalW) / 2;
        int my = (totalH - modalH) / 2;

        RECT rcModal = { mx, my, mx + modalW, my + modalH };
        HBRUSH hBrModal = CreateSolidBrush(pal->bgPanel);
        HPEN hPenBorderDef = CreatePen(PS_SOLID, 2, RGB(239, 68, 68));
        HGDIOBJ oldPenDef = SelectObject(hdc, hPenBorderDef);
        HGDIOBJ oldBrushDef = SelectObject(hdc, hBrModal);
        Rectangle(hdc, mx, my, mx + modalW, my + modalH);
        DeleteObject(hBrModal);

        // Header Banner
        RECT rcHdr = { mx, my, mx + modalW, my + 32 };
        HBRUSH hBrHdr = CreateSolidBrush(pal->bgHeader);
        FillRect(hdc, &rcHdr, hBrHdr);
        DeleteObject(hBrHdr);

        SelectObject(hdc, g_fontHeader);
        SetTextColor(hdc, RGB(239, 68, 68));
        TextOutA(hdc, mx + 14, my + 6, "BARGE DEFENSE MATRIX & ARMORY SYSTEMS", 37);

        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, pal->textBright);
        TextOutA(hdc, mx + modalW - 120, my + 10, "[X / ESC] CLOSE", 15);

        // Subheader Threat Assessment
        int barY = my + 36;
        RECT rcStat = { mx + 14, barY, mx + modalW - 14, barY + 30 };
        HBRUSH hBrStat = CreateSolidBrush(RGB(3, 7, 18));
        FillRect(hdc, &rcStat, hBrStat);
        DeleteObject(hBrStat);
        FrameRect(hdc, &rcStat, (HBRUSH)GetStockObject(WHITE_BRUSH));

        int activeRaiders = 0;
        for (int i = 0; i < MAX_RAIDERS; i++) if (g_state.raiders[i].active) activeRaiders++;

        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, activeRaiders > 0 ? RGB(239, 68, 68) : RGB(16, 185, 129));
        char threatBuf[128];
        sprintf(threatBuf, "SECTOR THREAT: %s  |  HOSTILES: %d DETECTED  |  PIRATES DEFEATED: %d  |  BOUNTIES: %d CR",
                activeRaiders > 0 ? "HOSTILE RAIDERS IN SCANNER RANGE" : "SECTOR CLEAR",
                activeRaiders, g_state.piratesDefeated, g_state.bountiesClaimed);
        TextOutA(hdc, mx + 20, barY + 8, threatBuf, (int)strlen(threatBuf));

        // Left Column: 4 Weapon Loadout Cards (350px wide)
        int leftX = mx + 14;
        int leftY = barY + 36;
        int leftW = 350;
        int wCardH = 88;
        int wGapY = 6;

        struct {
            const char* name;
            const char* desc;
            const char* ammo;
            COLORREF color;
        } wDefs[4] = {
            { "[1] Heavy Mining Laser", "Continuous thermal beam. High sustained DPS against unshielded asteroids & hulls.", "Ammo: Unlimited (Generates Heat)", RGB(0, 240, 255) },
            { "[2] Kinetic Railgun Cannon", "Hypervelocity tungsten-ferrum slug. Devastating kinetic armor-piercing damage.", "Ammo: 15 / 15 Slugs", RGB(56, 189, 248) },
            { "[3] EMP Flak Cannon", "Proximity flak detonation. Emits expanding EMP shockwave that stuns raiders & clears torpedoes.", "Ammo: 6 / 6 Charges", RGB(192, 132, 252) },
            { "[4] Point-Defense Laser", "Automated tracking pulse turret. Instantly intercepts incoming torpedoes and raiders.", "Ammo: Unlimited (Auto-Tracking)", RGB(16, 185, 129) }
        };

        for (int w = 0; w < 4; w++) {
            int wy = leftY + w * (wCardH + wGapY);
            int isSelected = (g_state.selectedWeapon == w);

            RECT rcWCard = { leftX, wy, leftX + leftW, wy + wCardH };
            HBRUSH hBrWCard = CreateSolidBrush(isSelected ? RGB(10, 30, 55) : RGB(2, 6, 18));
            FillRect(hdc, &rcWCard, hBrWCard);
            DeleteObject(hBrWCard);

            HPEN hPenWCard = CreatePen(PS_SOLID, isSelected ? 2 : 1, isSelected ? RGB(56, 189, 248) : RGB(30, 45, 70));
            SelectObject(hdc, hPenWCard);
            Rectangle(hdc, leftX, wy, leftX + leftW, wy + wCardH);
            DeleteObject(hPenWCard);

            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, wDefs[w].color);
            TextOutA(hdc, leftX + 8, wy + 6, wDefs[w].name, (int)strlen(wDefs[w].name));

            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, RGB(148, 163, 184));
            RECT rcWDesc = { leftX + 8, wy + 24, leftX + leftW - 8, wy + 54 };
            DrawTextA(hdc, wDefs[w].desc, -1, &rcWDesc, DT_WORDBREAK);

            // Ammo & Status
            char ammoStr[64];
            if (w == 0) sprintf(ammoStr, "Status: %s (Heat: %d%%)", g_state.miningActive ? "FIRING" : "STANDBY", (int)g_state.heat);
            else if (w == 1) sprintf(ammoStr, "Ammo: %d Slugs (Hold [F] to Fire)", g_state.railgunSlugs);
            else if (w == 2) sprintf(ammoStr, "Capacitors: %d Charges (Hold [G] to Detonate)", g_state.empCharges);
            else sprintf(ammoStr, "Auto-PDL: %s (Intercept Range: 280m)", g_state.autoPDL ? "ONLINE" : "STANDBY");

            SetTextColor(hdc, isSelected ? RGB(251, 191, 36) : RGB(148, 163, 184));
            TextOutA(hdc, leftX + 8, wy + 68, ammoStr, (int)strlen(ammoStr));

            if (isSelected) {
                SetTextColor(hdc, RGB(16, 185, 129));
                TextOutA(hdc, leftX + leftW - 65, wy + 6, "ARMED", 5);
            }
        }

        // Right Column: Munitions Workshop & Tactical Defenses (370px wide)
        int rcX = leftX + leftW + 12;
        int rcY = leftY;
        int rcW = modalW - (rcX - mx) - 14;

        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, RGB(56, 189, 248));
        TextOutA(hdc, rcX + 4, rcY - 14, "MUNITIONS WORKSHOP & TACTICAL ACTIONS", 37);

        int aCardH = 58;
        int aGapY = 6;

        struct {
            const char* title;
            const char* cost;
            const char* desc;
            int canDo;
        } actDefs[6] = {
            { "1. Smelt Railgun Slugs (+5 Slugs)", "5 Ferrum", "Cast high-density tungsten-ferrum kinetic slugs", g_state.cargoHold[0] >= 5 },
            { "2. Recharge EMP Capacitors (+2 Charges)", "4 Silicates", "Refill high-voltage electromagnetic discharge capacitor bank", g_state.cargoHold[1] >= 4 },
            { "3. Overcharge Deflector Shields (+130%)", "200 CR", "Route aux power into shield emitters for 130% overcharge", g_state.credits >= 200 },
            { "4. Emergency Combat Drones (18s)", "350 CR", "Launch autonomous drone swarm for combat & continuous hull repairs", g_state.credits >= 350 },
            { "5. Toggle Auto-Point Defense Turret", g_state.autoPDL ? "ACTIVE" : "OFFLINE", "Automatically shoot down incoming enemy homing torpedoes", 1 },
            { "6. Deploy Emergency Chaff Cloud", "Cooldown: 8s", "Release dense metallic chaff cloud to break all torpedo tracking locks", g_state.chaffCooldown <= 0.0f }
        };

        for (int a = 0; a < 6; a++) {
            int ay = rcY + a * (aCardH + aGapY);
            RECT rcACard = { rcX, ay, rcX + rcW, ay + aCardH };
            HBRUSH hBrACard = CreateSolidBrush(actDefs[a].canDo ? RGB(5, 12, 28) : RGB(15, 15, 20));
            FillRect(hdc, &rcACard, hBrACard);
            DeleteObject(hBrACard);

            HPEN hPenACard = CreatePen(PS_SOLID, 1, actDefs[a].canDo ? RGB(56, 189, 248) : RGB(40, 45, 60));
            SelectObject(hdc, hPenACard);
            Rectangle(hdc, rcX, ay, rcX + rcW, ay + aCardH);
            DeleteObject(hPenACard);

            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, actDefs[a].canDo ? RGB(255, 255, 255) : RGB(120, 120, 120));
            TextOutA(hdc, rcX + 8, ay + 6, actDefs[a].title, (int)strlen(actDefs[a].title));

            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, RGB(251, 191, 36));
            RECT rcCost = { rcX + rcW - 100, ay + 6, rcX + rcW - 8, ay + 20 };
            DrawTextA(hdc, actDefs[a].cost, -1, &rcCost, DT_RIGHT | DT_SINGLELINE);

            SetTextColor(hdc, RGB(148, 163, 184));
            RECT rcADesc = { rcX + 8, ay + 24, rcX + rcW - 8, ay + 42 };
            DrawTextA(hdc, actDefs[a].desc, -1, &rcADesc, DT_WORDBREAK);

            // Action Execute Button
            RECT rcBtnAct = { rcX + 8, ay + 40, rcX + rcW - 8, ay + 54 };
            HBRUSH hBrBtnAct = CreateSolidBrush(actDefs[a].canDo ? RGB(30, 58, 138) : RGB(25, 30, 40));
            FillRect(hdc, &rcBtnAct, hBrBtnAct);
            DeleteObject(hBrBtnAct);
            FrameRect(hdc, &rcBtnAct, (HBRUSH)GetStockObject(WHITE_BRUSH));

            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, actDefs[a].canDo ? RGB(110, 231, 183) : RGB(100, 116, 139));
            const char* bLabels[6] = { "CRAFT +5 SLUGS", "RECHARGE +2 EMP", "OVERCHARGE (200 CR)", "DEPLOY COMBAT DRONES (350 CR)", "TOGGLE AUTO-PDL", "DEPLOY CHAFF [KEY C]" };
            DrawTextA(hdc, bLabels[a], -1, &rcBtnAct, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        // Footer instruction
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(245, 158, 11));
        TextOutA(hdc, mx + 16, my + modalH - 22, "Weapons [1-4] • [F] Railgun • [G] EMP Flak • [X / ESC] Close Armory", 67);

        SelectObject(hdc, oldPenDef);
        SelectObject(hdc, oldBrushDef);
        DeleteObject(hPenBorderDef);
    }

    // Phase 14: Comprehensive Help & Dredger Captain's Manual Modal (5 Detailed Tabs)
    if (g_state.showHelp) {
        int helpW = 780;
        int helpH = 500;
        int hx = (totalW - helpW) / 2;
        int hy = (totalH - helpH) / 2;

        RECT rcHelp = { hx, hy, hx + helpW, hy + helpH };
        HBRUSH hBrHModal = CreateSolidBrush(pal->bgPanel);
        FillRect(hdc, &rcHelp, hBrHModal);
        DeleteObject(hBrHModal);

        HPEN hPenHBorder = CreatePen(PS_SOLID, 2, pal->borderGlow);
        HGDIOBJ oldPenH = SelectObject(hdc, hPenHBorder);
        FrameRect(hdc, &rcHelp, (HBRUSH)GetStockObject(WHITE_BRUSH));

        // Top Header Bar
        RECT rcHelpHeader = { hx, hy, hx + helpW, hy + 30 };
        FillRect(hdc, &rcHelpHeader, hBrSubHdr);
        SelectObject(hdc, g_fontHeader);
        SetTextColor(hdc, pal->vector);
        TextOutA(hdc, hx + 12, hy + 6, "KStarDredge - Dredger Captain's Manual & Codex", 46);

        // Close button at top right
        RECT rcCloseBtn = { hx + helpW - 120, hy + 4, hx + helpW - 10, hy + 26 };
        HBRUSH hBrClose = CreateSolidBrush(RGB(127, 29, 29));
        FillRect(hdc, &rcCloseBtn, hBrClose);
        DeleteObject(hBrClose);
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(254, 202, 202));
        DrawTextA(hdc, "[X] CLOSE [ESC]", -1, &rcCloseBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // 5 Navigation Tabs Bar (at hy + 32 to hy + 56)
        const char* tabNames[5] = {
            "[1] FLIGHT & CONTROLS",
            "[2] SECTOR CHARTS",
            "[3] MINERAL CODEX",
            "[4] REFINERY LAB",
            "[5] CRISIS & DEFENSE"
        };
        int tabW = (helpW - 24) / 5;
        int tabY = hy + 32;

        for (int t = 0; t < 5; t++) {
            RECT rcTab = { hx + 12 + t * tabW, tabY, hx + 12 + (t + 1) * tabW - 4, tabY + 24 };
            int isActive = (g_state.helpTab == t);
            HBRUSH hBrTab = CreateSolidBrush(isActive ? RGB(15, 28, 63) : RGB(10, 16, 30));
            FillRect(hdc, &rcTab, hBrTab);
            DeleteObject(hBrTab);

            HPEN hPenTab = CreatePen(PS_SOLID, 1, isActive ? pal->borderGlow : pal->borderPanel);
            HGDIOBJ oldPenTab = SelectObject(hdc, hPenTab);
            MoveToEx(hdc, rcTab.left, rcTab.bottom, NULL);
            LineTo(hdc, rcTab.left, rcTab.top);
            LineTo(hdc, rcTab.right, rcTab.top);
            LineTo(hdc, rcTab.right, rcTab.bottom);
            SelectObject(hdc, oldPenTab);
            DeleteObject(hPenTab);

            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, isActive ? pal->vector : RGB(100, 116, 139));
            DrawTextA(hdc, tabNames[t], -1, &rcTab, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        int cY = hy + 62;
        SelectObject(hdc, g_fontSmall);

        // --- TAB 0: FLIGHT CONTROLS & DREDGING TACTICS ---
        if (g_state.helpTab == 0) {
            // Left Column: Keybinds Table
            int col1X = hx + 14;
            int col1W = 360;
            int col2X = col1X + col1W + 12;
            int col2W = helpW - (col2X - hx) - 14;

            RECT rcCol1 = { col1X, cY, col1X + col1W, cY + 400 };
            HBRUSH hBrC1 = CreateSolidBrush(RGB(5, 10, 22));
            FillRect(hdc, &rcCol1, hBrC1);
            DeleteObject(hBrC1);
            FrameRect(hdc, &rcCol1, (HBRUSH)GetStockObject(WHITE_BRUSH));

            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, pal->textBright);
            TextOutA(hdc, col1X + 10, cY + 8, "COCKPIT KEYBOARD CONTROLS:", 26);

            SelectObject(hdc, g_fontSmall);
            int ky = cY + 30;
            const char* binds[14] = {
                "• [W / UP ARROW]    Forward Fusion Thrusters",
                "• [S / DOWN ARROW]  Retro Braking Thrusters",
                "• [A / D / ARROWS]  Pivot Mining Barge Heading",
                "• [SPACEBAR]        Fire Heavy Mining Laser",
                "• [1, 2, 3, 4]      Select Active Weapon System",
                "• [F]               Fire Kinetic Railgun Cannon",
                "• [G]               Launch EMP Flak Cannon",
                "• [C]               Deploy Emergency Chaff Cloud",
                "• [T]               Tractor Beam Vacuum Magnet",
                "• [Z]               Inertia Dampeners Drift Brake",
                "• [P]               Multi-Spectral Spectrometer",
                "• [U]               Engineering Bay & Upgrades",
                "• [N]               Star Sector Charts & Warp",
                "• [D] / [E] / [K]   Spaceport / EVA / Crisis Ops"
            };
            for (int i = 0; i < 14; i++) {
                SetTextColor(hdc, (i == 3 || i == 4 || i == 5) ? RGB(244, 63, 94) : RGB(148, 163, 184));
                TextOutA(hdc, col1X + 10, ky, binds[i], (int)strlen(binds[i]));
                ky += 18;
            }

            // Right Column: Telemetry & Operations
            RECT rcCol2 = { col2X, cY, col2X + col2W, cY + 400 };
            HBRUSH hBrC2 = CreateSolidBrush(RGB(5, 10, 22));
            FillRect(hdc, &rcCol2, hBrC2);
            DeleteObject(hBrC2);
            FrameRect(hdc, &rcCol2, (HBRUSH)GetStockObject(WHITE_BRUSH));

            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, pal->textBright);
            TextOutA(hdc, col2X + 10, cY + 8, "SUB-SYSTEMS & TELEMETRY GUIDE:", 30);

            SelectObject(hdc, g_fontSmall);
            int sy = cY + 30;
            SetTextColor(hdc, RGB(56, 189, 248));
            TextOutA(hdc, col2X + 10, sy, "1. Laser Optics & Heat Dissipation:", 35); sy += 15;
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutA(hdc, col2X + 14, sy, "Continuous firing heats optics. At 100% heat,", 45); sy += 14;
            TextOutA(hdc, col2X + 14, sy, "emergency cooldown shuts down laser until <25%.", 47); sy += 18;

            SetTextColor(hdc, RGB(16, 185, 129));
            TextOutA(hdc, col2X + 10, sy, "2. Tractor Emitter & Ore Scooping:", 34); sy += 15;
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutA(hdc, col2X + 14, sy, "Draws floating ore nuggets and scrap into hold.", 47); sy += 14;
            TextOutA(hdc, col2X + 14, sy, "Keep tractor active [T] while mining fields.", 44); sy += 18;

            SetTextColor(hdc, RGB(245, 158, 11));
            TextOutA(hdc, col2X + 10, sy, "3. Inertia Dampeners & Drift Brake:", 35); sy += 15;
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutA(hdc, col2X + 14, sy, "Arrests barge drift when maneuvering near wrecks.", 49); sy += 14;
            TextOutA(hdc, col2X + 14, sy, "Toggle with [Z] for precision docking.", 38); sy += 18;

            SetTextColor(hdc, RGB(192, 132, 252));
            TextOutA(hdc, col2X + 10, sy, "4. Radar Mini-Scope Blip Identifiers:", 37); sy += 15;
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutA(hdc, col2X + 14, sy, "Cyan=Asteroids, Red Diamond=Hostile Raiders,", 44); sy += 14;
            TextOutA(hdc, col2X + 14, sy, "Flashing Dot=Torpedo, Red Box=Derelict Hulk.", 44); sy += 14;
            TextOutA(hdc, col2X + 14, sy, "Green Box=Sector Orbital Spaceport.", 35);
        }
        // --- TAB 1: SECTOR & ASTEROID CHARTS ---
        else if (g_state.helpTab == 1) {
            int cardW = (helpW - 36) / 2;
            int cardH = 150;

            for (int s = 0; s < 4; s++) {
                int cxCard = hx + 14 + (s % 2) * (cardW + 8);
                int cyCard = cY + (s / 2) * (cardH + 8);
                RECT rcCard = { cxCard, cyCard, cxCard + cardW, cyCard + cardH };

                HBRUSH hBrSec = CreateSolidBrush(RGB(5, 10, 22));
                FillRect(hdc, &rcCard, hBrSec);
                DeleteObject(hBrSec);
                FrameRect(hdc, &rcCard, (HBRUSH)GetStockObject(WHITE_BRUSH));

                SelectObject(hdc, g_fontMonoBold);
                SetTextColor(hdc, (s == 0) ? RGB(56, 189, 248) : ((s == 1) ? RGB(167, 139, 250) : ((s == 2) ? RGB(244, 63, 94) : RGB(239, 68, 68))));
                char hTxt[64];
                sprintf(hTxt, "SECTOR %d: %s", s + 1, SECTOR_DEFS[s].name);
                TextOutA(hdc, cxCard + 10, cyCard + 8, hTxt, (int)strlen(hTxt));

                SelectObject(hdc, g_fontSmall);
                int py = cyCard + 28;
                SetTextColor(hdc, RGB(251, 191, 36));
                const char* threatLabels[4] = { "Threat: 10% (Low)  •  Radiation: 0.00 Rad/s", "Threat: 25% (Mod)  •  Radiation: 0.06 Rad/s", "Threat: 60% (High) •  Radiation: 0.15 Rad/s", "Threat: 85% (Extr) •  Radiation: 0.35 Rad/s" };
                TextOutA(hdc, cxCard + 10, py, threatLabels[s], (int)strlen(threatLabels[s])); py += 16;

                SetTextColor(hdc, RGB(148, 163, 184));
                const char* secDescs[4][3] = {
                    { "Dense asteroid field rich in Ferrum and Silicates.", "Home to Sol Mining Consortium Vanguard Foundry.", "Safe dredging zone for novice captains." },
                    { "Outer deep-freeze ring with rich Platinum veins.", "Trade depot for Free Miner Haven spaceport.", "Sub-zero temperatures and moderate pirate skiffs." },
                    { "Graveyard of ruined battlecruisers and wreckage.", "Rich in Derelict Scrap, Cores, and Black Boxes.", "Governed by ruthless Black Sun Syndicate." },
                    { "Ionizing nebula rich in Dark Matter Geodes.", "Extreme hazard zone with dreadnought raiders.", "Protected by heavily armed Void Vanguard Bastion." }
                };
                TextOutA(hdc, cxCard + 10, py, secDescs[s][0], (int)strlen(secDescs[s][0])); py += 14;
                TextOutA(hdc, cxCard + 10, py, secDescs[s][1], (int)strlen(secDescs[s][1])); py += 14;
                TextOutA(hdc, cxCard + 10, py, secDescs[s][2], (int)strlen(secDescs[s][2])); py += 16;

                SetTextColor(hdc, RGB(110, 231, 183));
                char wTxt[64];
                sprintf(wTxt, "Warp Fuel Jump Cost: %d%% Fuel", SECTOR_DEFS[s].fuelCost);
                TextOutA(hdc, cxCard + 10, py, wTxt, (int)strlen(wTxt));
            }

            // Spectrometer Bottom Note
            int noteY = cY + cardH * 2 + 20;
            RECT rcNote = { hx + 14, noteY, hx + helpW - 14, noteY + 60 };
            HBRUSH hBrNote = CreateSolidBrush(RGB(15, 28, 63));
            FillRect(hdc, &rcNote, hBrNote);
            DeleteObject(hBrNote);
            FrameRect(hdc, &rcNote, (HBRUSH)GetStockObject(WHITE_BRUSH));

            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, pal->vector);
            TextOutA(hdc, hx + 22, noteY + 6, "PROSPECTING TIP: SPECTROMETRY & HARMONIC RESONANCE", 50);

            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, RGB(226, 232, 240));
            TextOutA(hdc, hx + 22, noteY + 24, "Target any asteroid and press [P] to analyze elemental emission wavelengths. Clicking 'TUNE RESONANCE'", 102);
            TextOutA(hdc, hx + 22, noteY + 40, "synchronizes laser optics to unlock +50% Mining DPS, +50% Chunk Rate, and reveals hidden Dark Geodes!", 103);
        }
        // --- TAB 2: MINERAL & ORE CODEX ---
        else if (g_state.helpTab == 2) {
            int cardW = (helpW - 36) / 3;
            int cardH = 175;

            const char* oTitles[6] = {
                "Ferrum Ore (Fe-56)",
                "Dense Silicates (SiO2)",
                "Platinum Vein (Pt-78)",
                "Void Quartz (SiO4-Q)",
                "Dark Geode (DM-Omega)",
                "Derelict Scrap (Ti-Cr)"
            };
            const char* oValues[6] = { "15 CR / T", "25 CR / T", "75 CR / T", "140 CR / T", "300 CR / T", "50 CR / T" };
            const char* oDescs[6][5] = {
                { "Abundant structural heavy metal.", "Found in inner belt sectors.", "Smelts into Hyper-Ferrum Ingots.", "Used to craft Railgun Slugs.", "Base market: 15 CR/T." },
                { "High-purity quartz and flux sands.", "Essential metallurgical catalyst.", "Synthesized into EMP capacitors.", "Decomposed into fresh O2 canisters.", "Base market: 25 CR/T." },
                { "Precious conductive noble metal.", "High corrosion & heat resistance.", "Infused into quantum superconductors.", "Used to forge Dark Matter matrices.", "Base market: 75 CR/T." },
                { "Resonant piezoelectric crystal.", "Harvested in deep void rings.", "Catalyzes sub-space warp plasma.", "Used in precision laser optic lenses.", "Base market: 140 CR/T." },
                { "Ultra-rare gravitational anomaly.", "Harvested from nebula cores.", "Used in Dark Matter matrix ingots.", "Extreme commercial value across Sol.", "Base market: 300 CR/T." },
                { "Titanium warship armor fragments.", "Salvaged from derelict hulls.", "Refined with iron into nanite paste.", "Instantly repairs hull breaches.", "Base market: 50 CR/T." }
            };

            for (int i = 0; i < 6; i++) {
                int cxCard = hx + 14 + (i % 3) * (cardW + 6);
                int cyCard = cY + (i / 3) * (cardH + 10);
                RECT rcCard = { cxCard, cyCard, cxCard + cardW, cyCard + cardH };

                HBRUSH hBrOre = CreateSolidBrush(RGB(5, 10, 22));
                FillRect(hdc, &rcCard, hBrOre);
                DeleteObject(hBrOre);
                FrameRect(hdc, &rcCard, (HBRUSH)GetStockObject(WHITE_BRUSH));

                SelectObject(hdc, g_fontMonoBold);
                SetTextColor(hdc, ORE_DEFS[i].color);
                TextOutA(hdc, cxCard + 8, cyCard + 8, oTitles[i], (int)strlen(oTitles[i]));

                SelectObject(hdc, g_fontSmall);
                SetTextColor(hdc, RGB(251, 191, 36));
                TextOutA(hdc, cxCard + 8, cyCard + 26, oValues[i], (int)strlen(oValues[i]));

                SetTextColor(hdc, RGB(148, 163, 184));
                int ly = cyCard + 46;
                for (int d = 0; d < 5; d++) {
                    TextOutA(hdc, cxCard + 8, ly, oDescs[i][d], (int)strlen(oDescs[i][d]));
                    ly += 16;
                }
            }
        }
        // --- TAB 3: REFINERY LAB RECIPES ---
        else if (g_state.helpTab == 3) {
            int cardW = (helpW - 36) / 2;
            int cardH = 115;

            for (int i = 0; i < 6; i++) {
                const RefineryRecipeDef* r = &REFINERY_RECIPES[i];
                int cxCard = hx + 14 + (i % 2) * (cardW + 8);
                int cyCard = cY + (i / 2) * (cardH + 8);
                RECT rcCard = { cxCard, cyCard, cxCard + cardW, cyCard + cardH };

                HBRUSH hBrRec = CreateSolidBrush(RGB(5, 10, 22));
                FillRect(hdc, &rcCard, hBrRec);
                DeleteObject(hBrRec);
                FrameRect(hdc, &rcCard, (HBRUSH)GetStockObject(WHITE_BRUSH));

                SelectObject(hdc, g_fontMonoBold);
                SetTextColor(hdc, r->color);
                char rHead[64];
                sprintf(rHead, "%d. %s", i + 1, r->name);
                TextOutA(hdc, cxCard + 10, cyCard + 8, rHead, (int)strlen(rHead));

                SelectObject(hdc, g_fontSmall);
                SetTextColor(hdc, RGB(251, 191, 36));
                char rCost[64];
                sprintf(rCost, "Cost: %s  |  Val: %d CR", r->inputStr, r->value);
                TextOutA(hdc, cxCard + 10, cyCard + 28, rCost, (int)strlen(rCost));

                SetTextColor(hdc, RGB(148, 163, 184));
                RECT rcDesc = { cxCard + 10, cyCard + 48, cxCard + cardW - 10, cyCard + cardH - 24 };
                DrawTextA(hdc, r->desc, -1, &rcDesc, DT_WORDBREAK);

                if (r->usable) {
                    SetTextColor(hdc, RGB(16, 185, 129));
                    TextOutA(hdc, cxCard + 10, cyCard + cardH - 18, r->useLabel, (int)strlen(r->useLabel));
                }
            }

            // Refinery Bottom Note
            int noteY = cY + cardH * 3 + 12;
            RECT rcNote = { hx + 14, noteY, hx + helpW - 14, noteY + 45 };
            HBRUSH hBrNote = CreateSolidBrush(RGB(15, 28, 63));
            FillRect(hdc, &rcNote, hBrNote);
            DeleteObject(hBrNote);
            FrameRect(hdc, &rcNote, (HBRUSH)GetStockObject(WHITE_BRUSH));

            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, pal->vector);
            TextOutA(hdc, hx + 22, noteY + 6, "SMELTING CATALYTIC BOOST [B]:", 29);

            SelectObject(hdc, g_fontSmall);
            SetTextColor(hdc, RGB(226, 232, 240));
            TextOutA(hdc, hx + 22, noteY + 24, "Toggle Catalytic Boost [B] in the Refinery [R] for +25% bonus alloy yield on all smelted batches!", 97);
        }
        // --- TAB 4: CRISIS & DEFENSE TACTICS ---
        else if (g_state.helpTab == 4) {
            int col1X = hx + 14;
            int col1W = 360;
            int col2X = col1X + col1W + 12;
            int col2W = helpW - (col2X - hx) - 14;

            // Left: Life Support Crisis
            RECT rcCol1 = { col1X, cY, col1X + col1W, cY + 400 };
            HBRUSH hBrC1 = CreateSolidBrush(RGB(5, 10, 22));
            FillRect(hdc, &rcCol1, hBrC1);
            DeleteObject(hBrC1);
            FrameRect(hdc, &rcCol1, (HBRUSH)GetStockObject(WHITE_BRUSH));

            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, RGB(244, 63, 94));
            TextOutA(hdc, col1X + 10, cY + 8, "DAMAGE CONTROL & CRISIS OPS [K]:", 32);

            SelectObject(hdc, g_fontSmall);
            int cy1 = cY + 30;
            SetTextColor(hdc, RGB(239, 68, 68));
            TextOutA(hdc, col1X + 10, cy1, "1. Hull Breaches & Decompression:", 33); cy1 += 15;
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutA(hdc, col1X + 14, cy1, "Kinetic impacts breach compartments (Bridge, Hold,", 50); cy1 += 14;
            TextOutA(hdc, col1X + 14, cy1, "Reactor, Life Support). Deploy nanite sealant [1].", 50); cy1 += 18;

            SetTextColor(hdc, RGB(245, 158, 11));
            TextOutA(hdc, col1X + 10, cy1, "2. Superheated Plasma Conduit Leaks:", 36); cy1 += 15;
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutA(hdc, col1X + 14, cy1, "Laser overheating ruptures plasma manifolds.", 44); cy1 += 14;
            TextOutA(hdc, col1X + 14, cy1, "Vent superheated plasma [2] to avert meltdown.", 46); cy1 += 18;

            SetTextColor(hdc, RGB(16, 185, 129));
            TextOutA(hdc, col1X + 10, cy1, "3. CO2 Scrubbers & Radiation Flush:", 35); cy1 += 15;
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutA(hdc, col1X + 14, cy1, "Dust degrades filtration. Overcharge scrubbers [4]", 50); cy1 += 14;
            TextOutA(hdc, col1X + 14, cy1, "and flush radiation decon [3] to safeguard crew.", 48);

            // Right: Void Pirate Defense
            RECT rcCol2 = { col2X, cY, col2X + col2W, cY + 400 };
            HBRUSH hBrC2 = CreateSolidBrush(RGB(5, 10, 22));
            FillRect(hdc, &rcCol2, hBrC2);
            DeleteObject(hBrC2);
            FrameRect(hdc, &rcCol2, (HBRUSH)GetStockObject(WHITE_BRUSH));

            SelectObject(hdc, g_fontMonoBold);
            SetTextColor(hdc, RGB(239, 68, 68));
            TextOutA(hdc, col2X + 10, cY + 8, "VOID PIRATE INTEL & ARMORY [X]:", 31);

            SelectObject(hdc, g_fontSmall);
            int cy2 = cY + 30;
            SetTextColor(hdc, RGB(248, 113, 113));
            TextOutA(hdc, col2X + 10, cy2, "• Corsair Skiff (90 HP / 450 CR):", 33); cy2 += 15;
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutA(hdc, col2X + 14, cy2, "Agile raider with twin blasters. Easy Railgun kill.", 51); cy2 += 18;

            SetTextColor(hdc, RGB(251, 191, 36));
            TextOutA(hdc, col2X + 10, cy2, "• Marauder Gunship (220 HP / 1,100 CR):", 39); cy2 += 15;
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutA(hdc, col2X + 14, cy2, "Armored gunship firing lethal Homing Torpedoes.", 47); cy2 += 14;
            TextOutA(hdc, col2X + 14, cy2, "Deploy Chaff [C] or PDL Turret [4] to intercept!", 48); cy2 += 18;

            SetTextColor(hdc, RGB(244, 63, 94));
            TextOutA(hdc, col2X + 10, cy2, "• Void Dread Raider (450 HP / 2,800 CR):", 40); cy2 += 15;
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutA(hdc, col2X + 14, cy2, "Heavy dreadnought. Fire EMP Flak [G] to strip", 46); cy2 += 14;
            TextOutA(hdc, col2X + 14, cy2, "shields & stun for 4s, then finish with Railgun [F]!", 52);
        }

        // Footer Instruction
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(245, 158, 11));
        TextOutA(hdc, hx + 16, hy + helpH - 22, "Tabs [1-5] • [H / ESC] Close Manual • Click Viewport or Buttons to operate barge", 80);

        SelectObject(hdc, oldPenH);
        DeleteObject(hPenHBorder);
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
    
    // Cockpit Action Buttons in bottom-left console (8 in row 1, 7 in row 2)
    int bx = 8;
    int by1 = botY + 28;
    int by2 = botY + 62;
    int bw = 40;
    int bh = 28;
    int gap = 3;
    
    MoveWindow(g_btnLaser,      bx,                  by1, bw, bh, TRUE);
    MoveWindow(g_btnTractor,    bx + (bw + gap),     by1, bw, bh, TRUE);
    MoveWindow(g_btnDampener,   bx + (bw + gap) * 2, by1, bw, bh, TRUE);
    MoveWindow(g_btnScan,       bx + (bw + gap) * 3, by1, bw, bh, TRUE);
    MoveWindow(g_btnNav,        bx + (bw + gap) * 4, by1, bw, bh, TRUE);
    MoveWindow(g_btnStation,    bx + (bw + gap) * 5, by1, bw, bh, TRUE);
    MoveWindow(g_btnEva,        bx + (bw + gap) * 6, by1, bw, bh, TRUE);
    MoveWindow(g_btnCrisis,     bx + (bw + gap) * 7, by1, bw, bh, TRUE);
    
    int bw2 = 42;
    MoveWindow(g_btnDefense,    bx,                   by2, bw2 + 2, bh, TRUE);
    MoveWindow(g_btnUpgrades,   bx + (bw2 + gap),     by2, bw2 + 2, bh, TRUE);
    MoveWindow(g_btnRefinery,   bx + (bw2 + gap) * 2, by2, bw2 + 2, bh, TRUE);
    MoveWindow(g_btnTheme,      bx + (bw2 + gap) * 3 + 2, by2, bw2 + 6, bh, TRUE);
    MoveWindow(g_btnScanlines,  bx + (bw2 + gap) * 4 + 8, by2, bw2 + 6, bh, TRUE);
    MoveWindow(g_btnAudio,      bx + (bw2 + gap) * 5 + 14, by2, bw2 + 2, bh, TRUE);
    MoveWindow(g_btnHelp,       bx + (bw2 + gap) * 6 + 16, by2, bw2 + 2, bh, TRUE);
    
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
            g_btnScan      = CreateWindowA("BUTTON", "PROSPECT [P]",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SCAN, NULL, NULL);
            g_btnNav       = CreateWindowA("BUTTON", "SECTORS [N]",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_NAV, NULL, NULL);
            g_btnStation   = CreateWindowA("BUTTON", "STATION [D]",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_STATION, NULL, NULL);
            g_btnEva       = CreateWindowA("BUTTON", "EVA OPS [E]",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_EVA, NULL, NULL);
            g_btnCrisis    = CreateWindowA("BUTTON", "CRISIS [K]",    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_CRISIS, NULL, NULL);
            g_btnDefense   = CreateWindowA("BUTTON", "ARMORY [X]",    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_DEFENSE, NULL, NULL);
            g_btnUpgrades  = CreateWindowA("BUTTON", "UPGRADE [U]",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_UPGRADES, NULL, NULL);
            g_btnRefinery  = CreateWindowA("BUTTON", "SMELT [R]",     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_REFINERY, NULL, NULL);
            g_btnTheme     = CreateWindowA("BUTTON", "CRT CYAN",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_THEME, NULL, NULL);
            g_btnScanlines = CreateWindowA("BUTTON", "SCAN: ON",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SCANLINES, NULL, NULL);
            g_btnAudio     = CreateWindowA("BUTTON", "AUDIO [M]",     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_AUDIO, NULL, NULL);
            g_btnHelp      = CreateWindowA("BUTTON", "MANUAL [H]",    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);
            
            g_btnJettison  = CreateWindowA("BUTTON", "JETTISON",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_JETTISON, NULL, NULL);
            g_btnSell      = CreateWindowA("BUTTON", "LIQUIDATE",     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SELL, NULL, NULL);
            
            SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL, NULL);
            
            // Audio Thread (Phase 13)
            InitializeCriticalSection(&g_csSound);
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
                    g_state.showSpectrometer = !g_state.showSpectrometer;
                    if (g_state.showSpectrometer) { g_state.showStarChart = 0; g_state.showUpgrades = 0; g_state.showEva = 0; g_state.showCrisis = 0; g_state.showRefinery = 0; g_state.showStation = 0; g_state.showDefense = 0; }
                    TriggerSound(SFX_BEEP);
                    break;
                case ID_BTN_NAV:
                    g_state.showStarChart = !g_state.showStarChart;
                    if (g_state.showStarChart) { g_state.showUpgrades = 0; g_state.showSpectrometer = 0; g_state.showEva = 0; g_state.showCrisis = 0; g_state.showRefinery = 0; g_state.showStation = 0; g_state.showDefense = 0; }
                    TriggerSound(SFX_BEEP);
                    break;
                case ID_BTN_STATION:
                    g_state.showStation = !g_state.showStation;
                    if (g_state.showStation) { g_state.showStarChart = 0; g_state.showUpgrades = 0; g_state.showSpectrometer = 0; g_state.showEva = 0; g_state.showCrisis = 0; g_state.showRefinery = 0; g_state.showDefense = 0; }
                    TriggerSound(SFX_BEEP);
                    break;
                case ID_BTN_EVA:
                    g_state.showEva = !g_state.showEva;
                    if (g_state.showEva) { g_state.showStarChart = 0; g_state.showUpgrades = 0; g_state.showSpectrometer = 0; g_state.showCrisis = 0; g_state.showRefinery = 0; g_state.showStation = 0; g_state.showDefense = 0; }
                    TriggerSound(SFX_BEEP);
                    break;
                case ID_BTN_CRISIS:
                    g_state.showCrisis = !g_state.showCrisis;
                    if (g_state.showCrisis) { g_state.showStarChart = 0; g_state.showUpgrades = 0; g_state.showSpectrometer = 0; g_state.showEva = 0; g_state.showRefinery = 0; g_state.showStation = 0; g_state.showDefense = 0; }
                    TriggerSound(SFX_BEEP);
                    break;
                case ID_BTN_DEFENSE:
                    g_state.showDefense = !g_state.showDefense;
                    if (g_state.showDefense) { g_state.showStarChart = 0; g_state.showUpgrades = 0; g_state.showSpectrometer = 0; g_state.showEva = 0; g_state.showCrisis = 0; g_state.showRefinery = 0; g_state.showStation = 0; }
                    TriggerSound(SFX_BEEP);
                    break;
                case ID_BTN_UPGRADES:
                    g_state.showUpgrades = !g_state.showUpgrades;
                    if (g_state.showUpgrades) { g_state.showStarChart = 0; g_state.showSpectrometer = 0; g_state.showEva = 0; g_state.showCrisis = 0; g_state.showRefinery = 0; g_state.showStation = 0; g_state.showDefense = 0; }
                    TriggerSound(SFX_BEEP);
                    break;
                case ID_BTN_REFINERY:
                    g_state.showRefinery = !g_state.showRefinery;
                    if (g_state.showRefinery) { g_state.showStarChart = 0; g_state.showSpectrometer = 0; g_state.showEva = 0; g_state.showCrisis = 0; g_state.showUpgrades = 0; g_state.showStation = 0; g_state.showDefense = 0; }
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
                int helpW = 780;
                int helpH = 500;
                int hx = (totalW - helpW) / 2;
                int hy = (totalH - helpH) / 2;

                // Close button top-right
                if (mx >= hx + helpW - 130 && mx <= hx + helpW - 10 && my >= hy && my <= hy + 30) {
                    g_state.showHelp = 0;
                    TriggerSound(SFX_BEEP);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }

                // Tab buttons at hy + 32 to hy + 56
                int tabY = hy + 32;
                int tabW = (helpW - 24) / 5;
                for (int t = 0; t < 5; t++) {
                    int tx = hx + 12 + t * tabW;
                    if (mx >= tx && mx <= tx + tabW - 4 && my >= tabY && my <= tabY + 24) {
                        g_state.helpTab = t;
                        TriggerSound(SFX_BEEP);
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }

                // Click outside modal closes it
                if (mx < hx || mx > hx + helpW || my < hy || my > hy + helpH) {
                    g_state.showHelp = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                return 0;
            }

            if (g_state.showDefense) {
                int modalW = 760;
                int modalH = 490;
                int hx = (totalW - modalW) / 2;
                int hy = (totalH - modalH) / 2;

                // Close button top-right
                if (mx >= hx + modalW - 130 && mx <= hx + modalW - 10 && my >= hy && my <= hy + 32) {
                    g_state.showDefense = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }

                // Left Column: 4 weapon selection cards
                int barY = hy + 36;
                int leftX = hx + 14;
                int leftY = barY + 36;
                int leftW = 350;
                int wCardH = 88;
                int wGapY = 6;
                for (int w = 0; w < 4; w++) {
                    int wy = leftY + w * (wCardH + wGapY);
                    if (mx >= leftX && mx <= leftX + leftW && my >= wy && my <= wy + wCardH) {
                        g_state.selectedWeapon = w;
                        TriggerSound(SFX_BEEP);
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }

                // Right Column: 6 Munitions Workshop Action buttons
                int rcX = leftX + leftW + 12;
                int rcY = leftY;
                int rcW = modalW - (rcX - hx) - 14;
                int aCardH = 58;
                int aGapY = 6;
                for (int a = 0; a < 6; a++) {
                    int ay = rcY + a * (aCardH + aGapY);
                    if (mx >= rcX + 8 && mx <= rcX + rcW - 8 && my >= ay + 38 && my <= ay + 56) {
                        if (a == 0) CraftRailgunSlugs();
                        else if (a == 1) RechargeEMPCapacitor();
                        else if (a == 2) OverchargeShield();
                        else if (a == 3) DeployCombatDrones();
                        else if (a == 4) {
                            g_state.autoPDL = !g_state.autoPDL;
                            TriggerSound(SFX_BEEP);
                            AddLog(g_state.autoPDL ? "Automated Point-Defense Laser turret armed." : "Point-Defense Laser turret set to standby.", 0);
                        } else if (a == 5) DeployChaff();
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }

                // Click outside modal closes it
                if (mx < hx || mx > hx + modalW || my < hy || my > hy + modalH) {
                    g_state.showDefense = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                return 0;
            }
            
            if (g_state.showStation) {
                int modalW = 760;
                int modalH = 480;
                int hx = (totalW - modalW) / 2;
                int hy = (totalH - modalH) / 2;
                
                // Close button top-right
                if (mx >= hx + modalW - 130 && mx <= hx + modalW - 10 && my >= hy && my <= hy + 32) {
                    g_state.showStation = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                
                int barY = hy + 36;
                int leftX = hx + 14;
                int leftY = barY + 38;
                int leftW = 340;
                int srvH = 195;
                
                // 4 Maintenance Service Buttons
                int srvBtnH = 34;
                int srvGap = 6;
                for (int s = 0; s < 4; s++) {
                    int sy = leftY + 28 + s * (srvBtnH + srvGap);
                    if (mx >= leftX + 8 && mx <= leftX + leftW - 8 && my >= sy && my <= sy + srvBtnH) {
                        if (s == 0) ServiceRepairHull();
                        else if (s == 1) ServiceRechargeShield();
                        else if (s == 2) ServiceRefuel();
                        else if (s == 3) ServiceDecon();
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }
                
                // Black Market Barter Box
                int barY2 = leftY + srvH + 8;
                // Sell All with Tariffs
                if (mx >= leftX + 8 && mx <= leftX + leftW - 8 && my >= barY2 + 24 && my <= barY2 + 50) {
                    BarterSellAllWithTariff();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                // 3 Supply Purchase Buttons
                int supY = barY2 + 54;
                int supW = (leftW - 20) / 3;
                for (int p = 0; p < 3; p++) {
                    int px = leftX + 8 + p * (supW + 2);
                    if (mx >= px && mx <= px + supW && my >= supY && my <= supY + 28) {
                        BarterBuySupply(p);
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }
                
                // Right Column: 3 Dredging Contracts
                int rcX = leftX + leftW + 12;
                int rcY = leftY;
                int rcW = modalW - (rcX - hx) - 14;
                int cCardH = 110;
                int cGapY = 8;
                for (int c = 0; c < 3; c++) {
                    int cyCard = rcY + c * (cCardH + cGapY);
                    if (mx >= rcX + 8 && mx <= rcX + rcW - 8 && my >= cyCard + 78 && my <= cyCard + 102) {
                        ClaimContract(c);
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }
                
                // Click outside modal closes it
                if (mx < hx || mx > hx + modalW || my < hy || my > hy + modalH) {
                    g_state.showStation = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                return 0;
            }
            
            if (g_state.showRefinery) {
                int modalW = 740;
                int modalH = 475;
                int hx = (totalW - modalW) / 2;
                int hy = (totalH - modalH) / 2;
                
                // Close button top-right
                if (mx >= hx + modalW - 130 && mx <= hx + modalW - 10 && my >= hy && my <= hy + 30) {
                    g_state.showRefinery = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                
                // Status bar buttons
                int barY = hy + 34;
                int bBtnX = hx + 150;
                int bBtnW = 195;
                if (mx >= bBtnX && mx <= bBtnX + bBtnW && my >= barY + 4 && my <= barY + 26) {
                    ToggleCatalyticBoost();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                
                int rBtnX = bBtnX + bBtnW + 8;
                int rBtnW = 180;
                if (mx >= rBtnX && mx <= rBtnX + rBtnW && my >= barY + 4 && my <= barY + 26) {
                    RefineAllOres();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                
                int sBtnX = rBtnX + rBtnW + 8;
                int sBtnW = modalW - (sBtnX - hx) - 20;
                if (mx >= sBtnX && mx <= sBtnX + sBtnW && my >= barY + 4 && my <= barY + 26) {
                    SellAllRefined();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                
                // Left Column: Refined vault usable items
                int leftX = hx + 14;
                int leftY = barY + 36;
                int leftW = 290;
                int crucH = 120;
                int vaultY = leftY + crucH + 10;
                int vy = vaultY + 26;
                for (int i = 0; i < 6; i++) {
                    const RefineryRecipeDef* r = &REFINERY_RECIPES[i];
                    if (r->usable && g_state.refined[i] > 0) {
                        int useX1 = leftX + leftW - 65;
                        int useY1 = vy - 1;
                        int useX2 = leftX + leftW - 8;
                        int useY2 = vy + 15;
                        if (mx >= useX1 && mx <= useX2 && my >= useY1 && my <= useY2) {
                            UseRefinedItem(i);
                            InvalidateRect(hwnd, NULL, FALSE);
                            return 0;
                        }
                    }
                    vy += 22;
                }
                
                // Right Column: 6 Smelting Recipe Cards
                int rcX = leftX + leftW + 12;
                int rcY = leftY;
                int rcW = modalW - (rcX - hx) - 14;
                int rCardH = 60;
                int rGapY = 5;
                
                for (int i = 0; i < 6; i++) {
                    int cyCard = rcY + i * (rCardH + rGapY);
                    int b1X = rcX + 8;
                    int b1Y = cyCard + 36;
                    int b1W = 120;
                    int b1H = 20;
                    
                    int bMaxX = b1X + b1W + 8;
                    int bMaxW = 130;
                    
                    // Click Smelt 1x
                    if (mx >= b1X && mx <= b1X + b1W && my >= b1Y && my <= b1Y + b1H) {
                        SmeltRecipe(i, 1);
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                    
                    // Click Smelt Max
                    if (mx >= bMaxX && mx <= bMaxX + bMaxW && my >= b1Y && my <= b1Y + b1H) {
                        SmeltRecipe(i, 9999);
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }
                
                // Click outside modal closes it
                if (mx < hx || mx > hx + modalW || my < hy || my > hy + modalH) {
                    g_state.showRefinery = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                return 0;
            }
            
            if (g_state.showCrisis) {
                int modalW = 730;
                int modalH = 470;
                int hx = (totalW - modalW) / 2;
                int hy = (totalH - modalH) / 2;
                
                // Close button top-right
                if (mx >= hx + modalW - 130 && mx <= hx + modalW - 10 && my >= hy && my <= hy + 30) {
                    g_state.showCrisis = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                
                int actX = hx + 340;
                int actY = hy + 40;
                int actW = modalW - 356;
                int cardH = 70;
                int gapY = 8;
                
                // 1. Seal Breaches Button
                if (mx >= actX + 10 && mx <= actX + actW - 10 && my >= actY + 40 && my <= actY + 64) {
                    ActionSealBreaches();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                // 2. Vent Plasma Button
                int actY2 = actY + cardH + gapY;
                if (mx >= actX + 10 && mx <= actX + actW - 10 && my >= actY2 + 40 && my <= actY2 + 64) {
                    ActionVentPlasma();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                // 3. Flush Radiation Button
                int actY3 = actY2 + cardH + gapY;
                if (mx >= actX + 10 && mx <= actX + actW - 10 && my >= actY3 + 40 && my <= actY3 + 64) {
                    ActionFlushRadiation();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                // 4. Service O2 Scrubbers Button
                int actY4 = actY3 + cardH + gapY;
                if (mx >= actX + 10 && mx <= actX + actW - 10 && my >= actY4 + 40 && my <= actY4 + 64) {
                    ActionServiceO2Scrubbers();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                // 5. Deploy Repair Drones Button
                int actY5 = actY4 + cardH + gapY;
                if (mx >= actX + 10 && mx <= actX + actW - 10 && my >= actY5 + 40 && my <= actY5 + 64) {
                    ActionDeployRepairDrones();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                
                // Click outside modal closes it
                if (mx < hx || mx > hx + modalW || my < hy || my > hy + modalH) {
                    g_state.showCrisis = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                return 0;
            }
            
            if (g_state.showEva) {
                int modalW = 720;
                int modalH = 470;
                int hx = (totalW - modalW) / 2;
                int hy = (totalH - modalH) / 2;
                
                // Close button top-right
                if (mx >= hx + modalW - 130 && mx <= hx + modalW - 10 && my >= hy && my <= hy + 30) {
                    g_state.showEva = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                
                if (g_state.selectedDerelictIndex >= 0 && g_state.derelicts[g_state.selectedDerelictIndex].active) {
                    int opX = hx + 340;
                    int opY = hy + 56;
                    int opW = modalW - 356;
                    int cardH = 84;
                    int gapY = 8;
                    
                    int opY2 = opY + cardH + gapY;
                    int opY3 = opY2 + cardH + gapY;
                    int opY4 = opY3 + cardH + gapY;
                    
                    // 1. Airlock Breach
                    if (mx >= opX + 10 && mx <= opX + opW - 10 && my >= opY + 54 && my <= opY + 76) {
                        BreachAirlock(g_state.selectedDerelictIndex);
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                    // 2. Black Box
                    if (mx >= opX + 10 && mx <= opX + opW - 10 && my >= opY2 + 48 && my <= opY2 + 74) {
                        DecryptBlackBox(g_state.selectedDerelictIndex);
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                    // 3. Reactor Core
                    if (mx >= opX + 10 && mx <= opX + opW - 10 && my >= opY3 + 48 && my <= opY3 + 74) {
                        HarvestReactorCore(g_state.selectedDerelictIndex);
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                    // 4. Cargo Scavenge
                    if (mx >= opX + 10 && mx <= opX + opW - 10 && my >= opY4 + 48 && my <= opY4 + 74) {
                        ScavengeCargoPods(g_state.selectedDerelictIndex);
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }
                
                // Click outside modal closes it
                if (mx < hx || mx > hx + modalW || my < hy || my > hy + modalH) {
                    g_state.showEva = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                return 0;
            }
            
            if (g_state.showSpectrometer) {
                int modalW = 700;
                int modalH = 460;
                int hx = (totalW - modalW) / 2;
                int hy = (totalH - modalH) / 2;
                
                // Close button top-right
                if (mx >= hx + modalW - 130 && mx <= hx + modalW - 10 && my >= hy && my <= hy + 30) {
                    g_state.showSpectrometer = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                
                // 3 Action Buttons at bottom
                int actY = hy + modalH - 74;
                int btnW = (modalW - 48) / 3;
                int btnH = 40;
                for (int b = 0; b < 3; b++) {
                    int bx = hx + 16 + b * (btnW + 8);
                    if (mx >= bx && mx <= bx + btnW && my >= actY && my <= actY + btnH) {
                        if (b == 0) ScanTargetAsteroid(g_state.selectedAstIndex);
                        else if (b == 1) ScanAllWideBand();
                        else if (b == 2) TuneLaserResonance(g_state.selectedAstIndex);
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }
                
                // Click outside modal closes it
                if (mx < hx || mx > hx + modalW || my < hy || my > hy + modalH) {
                    g_state.showSpectrometer = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                return 0;
            }
            
            if (g_state.showUpgrades) {
                int modalW = 680;
                int modalH = 430;
                int hx = (totalW - modalW) / 2;
                int hy = (totalH - modalH) / 2;
                
                // Close button top-right
                if (mx >= hx + modalW - 130 && mx <= hx + modalW - 10 && my >= hy && my <= hy + 30) {
                    g_state.showUpgrades = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                
                // 4 Upgrade Cards Buy Buttons
                int cardW = 318;
                int cardH = 145;
                int gapX = 16;
                int gapY = 10;
                int startX = hx + 14;
                int startY = hy + 68;
                
                for (int m = 0; m < 4; m++) {
                    int row = m / 2;
                    int col = m % 2;
                    int ucx = startX + col * (cardW + gapX);
                    int ucy = startY + row * (cardH + gapY);
                    
                    int btnX1 = ucx + 8;
                    int btnY1 = ucy + cardH - 34;
                    int btnX2 = ucx + cardW - 8;
                    int btnY2 = ucy + cardH - 8;
                    
                    if (mx >= btnX1 && mx <= btnX2 && my >= btnY1 && my <= btnY2) {
                        BuyUpgrade(m);
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }
                
                // Click outside modal closes it
                if (mx < hx || mx > hx + modalW || my < hy || my > hy + modalH) {
                    g_state.showUpgrades = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
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
            
            // Check click inside viewport for asteroid or derelict targeting
            if (mx >= viewportX && mx < viewportX + viewportW && my >= viewportY && my < viewportY + viewportH) {
                int cx = viewportX + (viewportW / 2);
                int cyCenter = viewportY + (viewportH / 2);
                float clickWorldX = g_state.shipX + (float)(mx - cx);
                float clickWorldY = g_state.shipY + (float)(my - cyCenter);
                
                // Check derelict click first
                int bestDer = -1;
                float minDerDist = 90.0f;
                for (int i = 0; i < MAX_DERELICTS; i++) {
                    if (!g_state.derelicts[i].active) continue;
                    float d = (float)sqrt((g_state.derelicts[i].x - clickWorldX) * (g_state.derelicts[i].x - clickWorldX) +
                                          (g_state.derelicts[i].y - clickWorldY) * (g_state.derelicts[i].y - clickWorldY));
                    if (d < g_state.derelicts[i].length * 0.6f + 15.0f && d < minDerDist) {
                        minDerDist = d;
                        bestDer = i;
                    }
                }
                if (bestDer >= 0) {
                    g_state.selectedDerelictIndex = bestDer;
                    TriggerSound(SFX_BEEP);
                    char tLog[64];
                    sprintf(tLog, "Target locked: %s [%s]", g_state.derelicts[bestDer].id, g_state.derelicts[bestDer].name);
                    AddLog(tLog, 0);
                    return 0;
                }
                
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
        }
        
        case WM_KEYDOWN: {
            if (g_state.showDefense) {
                if (wParam == '1') { g_state.selectedWeapon = 0; TriggerSound(SFX_BEEP); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '2') { g_state.selectedWeapon = 1; TriggerSound(SFX_BEEP); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '3') { g_state.selectedWeapon = 2; TriggerSound(SFX_BEEP); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '4') { g_state.selectedWeapon = 3; TriggerSound(SFX_BEEP); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == 'F') { FireRailgun(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == 'G') { FireEMPFlak(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == 'C') { DeployChaff(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == 'X' || wParam == VK_ESCAPE) { g_state.showDefense = 0; InvalidateRect(hwnd, NULL, FALSE); return 0; }
            }

            if (g_state.showStation) {
                if (wParam == '1') { ServiceRepairHull(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '2') { ServiceRechargeShield(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '3') { ServiceRefuel(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '4') { ServiceDecon(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '5') { ClaimContract(0); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '6') { ClaimContract(1); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '7') { ClaimContract(2); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == 'S') { BarterSellAllWithTariff(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == 'U') { ToggleDocking(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == 'D' || wParam == VK_ESCAPE) { g_state.showStation = 0; InvalidateRect(hwnd, NULL, FALSE); return 0; }
            }
            
            if (g_state.showRefinery) {
                if (wParam >= '1' && wParam <= '6') { SmeltRecipe((int)(wParam - '1'), 1); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == 'B') { ToggleCatalyticBoost(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == 'A') { RefineAllOres(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == 'S') { SellAllRefined(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == 'R' || wParam == VK_ESCAPE) { g_state.showRefinery = 0; InvalidateRect(hwnd, NULL, FALSE); return 0; }
            }
            
            if (g_state.showCrisis) {
                if (wParam == '1') { ActionSealBreaches(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '2') { ActionVentPlasma(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '3') { ActionFlushRadiation(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '4') { ActionServiceO2Scrubbers(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '5') { ActionDeployRepairDrones(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == 'K' || wParam == VK_ESCAPE) { g_state.showCrisis = 0; InvalidateRect(hwnd, NULL, FALSE); return 0; }
            }
            
            if (g_state.showEva) {
                if (wParam == '1') { BreachAirlock(g_state.selectedDerelictIndex); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '2') { DecryptBlackBox(g_state.selectedDerelictIndex); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '3') { HarvestReactorCore(g_state.selectedDerelictIndex); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '4') { ScavengeCargoPods(g_state.selectedDerelictIndex); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == 'E' || wParam == VK_ESCAPE) { g_state.showEva = 0; InvalidateRect(hwnd, NULL, FALSE); return 0; }
            }
            
            if (g_state.showSpectrometer) {
                if (wParam == '1') { ScanTargetAsteroid(g_state.selectedAstIndex); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '2') { ScanAllWideBand(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '3') { TuneLaserResonance(g_state.selectedAstIndex); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == 'P' || wParam == VK_ESCAPE) { g_state.showSpectrometer = 0; InvalidateRect(hwnd, NULL, FALSE); return 0; }
            }
            
            if (g_state.showUpgrades) {
                if (wParam == '1') { BuyUpgrade(0); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '2') { BuyUpgrade(1); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '3') { BuyUpgrade(2); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '4') { BuyUpgrade(3); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == 'U' || wParam == VK_ESCAPE) { g_state.showUpgrades = 0; InvalidateRect(hwnd, NULL, FALSE); return 0; }
            }
            
            if (g_state.showStarChart) {
                if (wParam == '1') { g_state.selectedSectorIndex = 0; TriggerSound(SFX_BEEP); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '2') { g_state.selectedSectorIndex = 1; TriggerSound(SFX_BEEP); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '3') { g_state.selectedSectorIndex = 2; TriggerSound(SFX_BEEP); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == '4') { g_state.selectedSectorIndex = 3; TriggerSound(SFX_BEEP); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == VK_RETURN) { EngageWarpJump(g_state.selectedSectorIndex); InvalidateRect(hwnd, NULL, FALSE); return 0; }
                if (wParam == 'N' || wParam == VK_ESCAPE) { g_state.showStarChart = 0; InvalidateRect(hwnd, NULL, FALSE); return 0; }
            }

            if (g_state.showHelp) {
                if (wParam >= '1' && wParam <= '5') {
                    g_state.helpTab = (int)(wParam - '1');
                    TriggerSound(SFX_BEEP);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (wParam == VK_ESCAPE || wParam == 'H') {
                    g_state.showHelp = 0;
                    TriggerSound(SFX_BEEP);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            }
            
            switch (wParam) {
                case 'W': case VK_UP:    g_state.thrusting = 1; break;
                case 'S': case VK_DOWN:  g_state.reversing = 1; break;
                case 'A': case VK_LEFT:  g_state.turningLeft = 1; break;
                case VK_RIGHT:           g_state.turningRight = 1; break;
                case 'D': {
                    float stDist = (float)sqrt((g_state.stationX - g_state.shipX) * (g_state.stationX - g_state.shipX) +
                                               (g_state.stationY - g_state.shipY) * (g_state.stationY - g_state.shipY));
                    if (stDist < 280.0f && !g_state.stationDocked) {
                        ToggleDocking();
                        g_state.showStation = 1;
                        g_state.showStarChart = 0; g_state.showUpgrades = 0; g_state.showSpectrometer = 0; g_state.showEva = 0; g_state.showCrisis = 0; g_state.showRefinery = 0;
                    } else if (g_state.stationDocked) {
                        g_state.showStation = !g_state.showStation;
                        if (g_state.showStation) {
                            g_state.showStarChart = 0; g_state.showUpgrades = 0; g_state.showSpectrometer = 0; g_state.showEva = 0; g_state.showCrisis = 0; g_state.showRefinery = 0;
                        }
                    } else {
                        // Regular turn right if not in proximity
                        g_state.turningRight = 1;
                    }
                    break;
                }
                case VK_SPACE:
                    g_state.miningActive = 1;
                    break;
                case '1':
                    g_state.selectedWeapon = 0;
                    AddLog("Selected Weapon: Heavy Mining Thermal Beam [1].", 1);
                    TriggerSound(SFX_BEEP);
                    break;
                case '2':
                    g_state.selectedWeapon = 1;
                    AddLog("Selected Weapon: Kinetic Railgun Cannon [2] (Press [F] to fire).", 4);
                    TriggerSound(SFX_BEEP);
                    break;
                case '3':
                    g_state.selectedWeapon = 2;
                    AddLog("Selected Weapon: EMP Flak Cannon [3] (Press [G] to fire).", 3);
                    TriggerSound(SFX_BEEP);
                    break;
                case '4':
                    g_state.selectedWeapon = 3;
                    AddLog("Selected Weapon: Automated Point-Defense Turret [4].", 5);
                    TriggerSound(SFX_BEEP);
                    break;
                case 'F':
                    FireRailgun();
                    break;
                case 'G':
                    FireEMPFlak();
                    break;
                case 'X':
                    g_state.showDefense = !g_state.showDefense;
                    if (g_state.showDefense) { g_state.showStarChart = 0; g_state.showUpgrades = 0; g_state.showSpectrometer = 0; g_state.showEva = 0; g_state.showCrisis = 0; g_state.showRefinery = 0; g_state.showStation = 0; }
                    TriggerSound(SFX_BEEP);
                    break;
                case 'T':
                    g_state.tractorActive = !g_state.tractorActive;
                    AddLog(g_state.tractorActive ? "Tractor emitter magnetized." : "Tractor beam offline.", 0);
                    break;
                case 'Z':
                    g_state.dampeners = !g_state.dampeners;
                    AddLog(g_state.dampeners ? "Inertia dampeners engaged." : "Inertia dampeners disengaged.", 0);
                    break;
                case 'P':
                    g_state.showSpectrometer = !g_state.showSpectrometer;
                    if (g_state.showSpectrometer) { g_state.showStarChart = 0; g_state.showUpgrades = 0; g_state.showEva = 0; g_state.showCrisis = 0; g_state.showRefinery = 0; }
                    TriggerSound(SFX_BEEP);
                    break;
                case 'E':
                    g_state.showEva = !g_state.showEva;
                    if (g_state.showEva) { g_state.showStarChart = 0; g_state.showUpgrades = 0; g_state.showSpectrometer = 0; g_state.showCrisis = 0; g_state.showRefinery = 0; }
                    TriggerSound(SFX_BEEP);
                    break;
                case 'K':
                    g_state.showCrisis = !g_state.showCrisis;
                    if (g_state.showCrisis) { g_state.showStarChart = 0; g_state.showUpgrades = 0; g_state.showSpectrometer = 0; g_state.showEva = 0; g_state.showRefinery = 0; }
                    TriggerSound(SFX_BEEP);
                    break;
                case 'R':
                    g_state.showRefinery = !g_state.showRefinery;
                    if (g_state.showRefinery) { g_state.showStarChart = 0; g_state.showUpgrades = 0; g_state.showSpectrometer = 0; g_state.showEva = 0; g_state.showCrisis = 0; }
                    TriggerSound(SFX_BEEP);
                    break;
                case 'U':
                    g_state.showUpgrades = !g_state.showUpgrades;
                    if (g_state.showUpgrades) { g_state.showStarChart = 0; g_state.showSpectrometer = 0; g_state.showEva = 0; g_state.showCrisis = 0; g_state.showRefinery = 0; }
                    TriggerSound(SFX_BEEP);
                    break;
                case 'N':
                    g_state.showStarChart = !g_state.showStarChart;
                    if (g_state.showStarChart) { g_state.showUpgrades = 0; g_state.showSpectrometer = 0; g_state.showEva = 0; g_state.showCrisis = 0; g_state.showRefinery = 0; }
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
