#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define WINDOW_WIDTH 1020
#define WINDOW_HEIGHT 720
#define IDT_AUTORUN 101

// Terminal Aesthetic Themes
typedef enum {
    THEME_AMBER = 0,
    THEME_RUST,
    THEME_GREEN,
    THEME_MONO,
    THEME_COUNT
} TerminalTheme;

typedef struct {
    const char* name;
    COLORREF bg;
    COLORREF panelBg;
    COLORREF panelHdr;
    COLORREF border;
    COLORREF borderCorner;
    COLORREF textMain;
    COLORREF textDim;
    COLORREF textBright;
    COLORREF amber;
    COLORREF green;
    COLORREF cyan;
    COLORREF red;
    COLORREF darkCard;
    COLORREF btnBg;
    COLORREF btnHover;
    COLORREF barBg;
} ThemePalette;

static const ThemePalette g_palettes[THEME_COUNT] = {
    // THEME_AMBER (P3 Phosphor Amber CRT)
    {
        "AMBER CRT",
        RGB(16, 11, 5),      // bg
        RGB(28, 19, 9),      // panelBg
        RGB(42, 27, 13),     // panelHdr
        RGB(94, 58, 24),     // border
        RGB(217, 119, 6),    // borderCorner
        RGB(251, 191, 36),   // textMain
        RGB(146, 84, 24),    // textDim
        RGB(254, 243, 199),  // textBright
        RGB(245, 158, 11),   // amber
        RGB(52, 211, 153),   // green
        RGB(56, 189, 248),   // cyan
        RGB(248, 113, 113),  // red
        RGB(22, 15, 7),      // darkCard
        RGB(38, 25, 12),     // btnBg
        RGB(61, 39, 19),     // btnHover
        RGB(10, 7, 3)        // barBg
    },
    // THEME_RUST (Wasteland Rust / Scorched Iron)
    {
        "WASTELAND RUST",
        RGB(18, 11, 8),      // bg
        RGB(28, 17, 11),     // panelBg
        RGB(43, 26, 17),     // panelHdr
        RGB(105, 51, 30),    // border
        RGB(234, 88, 12),    // borderCorner
        RGB(249, 115, 22),   // textMain
        RGB(154, 66, 32),    // textDim
        RGB(255, 237, 213),  // textBright
        RGB(245, 158, 11),   // amber
        RGB(16, 185, 129),   // green
        RGB(6, 182, 212),    // cyan
        RGB(239, 68, 68),    // red
        RGB(21, 12, 7),      // darkCard
        RGB(39, 22, 14),     // btnBg
        RGB(62, 34, 23),     // btnHover
        RGB(11, 7, 4)        // barBg
    },
    // THEME_GREEN (Classic Phosphor P1 Green)
    {
        "PHOSPHOR GREEN",
        RGB(11, 15, 12),     // bg
        RGB(18, 25, 19),     // panelBg
        RGB(26, 37, 27),     // panelHdr
        RGB(47, 69, 48),     // border
        RGB(82, 138, 85),    // borderCorner
        RGB(110, 231, 183),  // textMain
        RGB(61, 122, 91),    // textDim
        RGB(167, 243, 208),  // textBright
        RGB(245, 158, 11),   // amber
        RGB(16, 185, 129),   // green
        RGB(6, 182, 212),    // cyan
        RGB(239, 68, 68),    // red
        RGB(15, 22, 16),     // darkCard
        RGB(24, 35, 25),     // btnBg
        RGB(35, 52, 37),     // btnHover
        RGB(8, 12, 9)        // barBg
    },
    // THEME_MONO (P4 Monochrome Radar White)
    {
        "MONOCHROME",
        RGB(12, 15, 20),     // bg
        RGB(19, 25, 35),     // panelBg
        RGB(28, 36, 51),     // panelHdr
        RGB(59, 77, 102),    // border
        RGB(148, 163, 184),  // borderCorner
        RGB(203, 213, 225),  // textMain
        RGB(100, 116, 139),  // textDim
        RGB(248, 250, 252),  // textBright
        RGB(251, 191, 36),   // amber
        RGB(56, 189, 248),   // green
        RGB(226, 232, 240),  // cyan
        RGB(244, 63, 94),    // red
        RGB(16, 21, 29),     // darkCard
        RGB(27, 35, 49),     // btnBg
        RGB(41, 53, 73),     // btnHover
        RGB(8, 10, 14)       // barBg
    }
};

static int g_currentTheme = THEME_AMBER;
static int g_crtScanlines = 1;

#define COL_BG          (g_palettes[g_currentTheme].bg)
#define COL_PANEL_BG    (g_palettes[g_currentTheme].panelBg)
#define COL_PANEL_HDR   (g_palettes[g_currentTheme].panelHdr)
#define COL_BORDER      (g_palettes[g_currentTheme].border)
#define COL_BORDER_HI   (g_palettes[g_currentTheme].borderCorner)
#define COL_TEXT_MAIN   (g_palettes[g_currentTheme].textMain)
#define COL_TEXT_DIM    (g_palettes[g_currentTheme].textDim)
#define COL_TEXT_BRIGHT (g_palettes[g_currentTheme].textBright)
#define COL_AMBER       (g_palettes[g_currentTheme].amber)
#define COL_GREEN       (g_palettes[g_currentTheme].green)
#define COL_CYAN        (g_palettes[g_currentTheme].cyan)
#define COL_RED         (g_palettes[g_currentTheme].red)
#define COL_DARK_CARD   (g_palettes[g_currentTheme].darkCard)
#define COL_BTN_BG      (g_palettes[g_currentTheme].btnBg)
#define COL_BTN_HOVER   (g_palettes[g_currentTheme].btnHover)
#define COL_BAR_BG      (g_palettes[g_currentTheme].barBg)

// Audio Constants & Thread (Win32 Beep SFX)
#define SFX_CLICK        1
#define SFX_CYCLE        2
#define SFX_ALERT        3
#define SFX_SCOUT        4
#define SFX_GEIGER       5
#define SFX_CONSTRUCTION 6
#define SFX_HAZARD       7
#define SFX_TRADE        8
#define SFX_RESEARCH     9
#define SFX_WIND         10
#define SFX_MEDS         11
#define SFX_TURRET       12
#define SFX_SIREN        13
#define SFX_REPAIR       14

static int g_soundEnabled = 1;

static DWORD WINAPI SoundThreadProc(LPVOID lpParam) {
    if (!g_soundEnabled) return 0;
    int type = (int)(LONG_PTR)lpParam;
    switch (type) {
        case SFX_CLICK: // 1: Subtle key click
            Beep(700, 20);
            break;
        case SFX_CYCLE: // 2: Melodic cycle progression
            Beep(440, 50); Beep(554, 50); Beep(659, 70);
            break;
        case SFX_ALERT: // 3: Warning buzzer
            Beep(220, 90); Beep(180, 110);
            break;
        case SFX_SCOUT: // 4: Expedition departure / arrival
            Beep(523, 60); Beep(784, 80);
            break;
        case SFX_GEIGER: // 5: Geiger counter radiation crackles
            Beep(2100, 6); Sleep(18);
            Beep(1800, 6); Sleep(30);
            Beep(2400, 6); Sleep(15);
            Beep(1950, 6); Sleep(25);
            Beep(2250, 6); Sleep(10);
            Beep(1700, 6);
            break;
        case SFX_CONSTRUCTION: // 6: Engine hum and hydraulic impact pulses
            Beep(110, 80); Sleep(15);
            Beep(260, 50); Sleep(15);
            Beep(130, 90); Sleep(15);
            Beep(320, 60);
            break;
        case SFX_HAZARD: // 7: Ominous descending hazard rumble
            Beep(240, 90); Beep(180, 110); Beep(120, 160);
            break;
        case SFX_TRADE: // 8: Cheerful barter / merchant chime
            Beep(659, 45); Beep(880, 55); Beep(1175, 75);
            break;
        case SFX_RESEARCH: // 9: High-tech computing arpeggio
            Beep(988, 35); Beep(1318, 45); Beep(1760, 60); Beep(2093, 75);
            break;
        case SFX_WIND: // 10: Howling wasteland wind whistle
            Beep(190, 90); Beep(240, 120); Beep(210, 130); Beep(160, 110);
            break;
        case SFX_MEDS: // 11: Medical injector treatment
            Beep(523, 50); Beep(659, 50); Beep(880, 75);
            break;
        case SFX_TURRET: // 12: Staccato defense cannon burst
            Beep(330, 25); Sleep(20);
            Beep(290, 25); Sleep(20);
            Beep(350, 25); Sleep(20);
            Beep(310, 25);
            break;
        case SFX_SIREN: // 13: Raider klaxon emergency alarm
            Beep(880, 70); Beep(587, 70);
            Beep(880, 70); Beep(587, 70);
            Beep(880, 90);
            break;
        case SFX_REPAIR: // 14: Metal repair impact
            Beep(450, 30); Sleep(10);
            Beep(600, 30); Sleep(20);
            Beep(300, 60);
            break;
        default:
            Beep(500, 30);
            break;
    }
    return 0;
}

static void PlaySfx(int type) {
    if (!g_soundEnabled) return;
    CreateThread(NULL, 0, SoundThreadProc, (LPVOID)(LONG_PTR)type, 0, NULL);
}

// Data structures
typedef struct {
    char id[16];
    char name[32];
    char desc[80];
    int level;
    int maxWorkers;
    int assigned;
    int powerCost;
    int powerPriority;
    int foodProd;
    int waterProd;
    int scrapProd;
    int powerProd;
} Facility;

#define MAX_SURVIVORS 20
typedef struct {
    char id[16];
    char name[48];
    char role[32];
    int health;
    int hunger;
    int thirst;
    int morale;
    int rads; // 0-100 Radiation sickness
    char job[16]; // "unassigned", "cmd", "gen", "water", "farm", "infirmary", "workshop", "expedition"
    int str, agi, inte;
} Survivor;

#define MAX_EXPEDITIONS 5
typedef struct {
    char id[16];
    char name[40];
    char desc[90];
    int duration;
    char risk[32];
    int riskLevel;
    char assignedScout[16]; // id or empty
    int daysRemaining;
    int potentialFood;
    int potentialScrap;
    int potentialMeds;
    char blueprintId[16];
    char blueprintReward[32];
    int hasStimpack;
} Expedition;

#define MAX_LOG_ENTRIES 60
typedef struct {
    char text[160];
    int type; // 0: info, 1: warn, 2: crit, 3: cycle, 4: scout
    int day;
    int phase;
} LogEntry;

// Construction Blueprints
#define MAX_BLUEPRINTS 16
typedef struct {
    char id[16];
    char name[32];
    char desc[80];
    int cost;
    int powerProd;
    int powerCost;
    int foodProd;
    int waterProd;
    int scrapProd;
    int maxWorkers;
    int powerPriority;
    int popBoost;
    int defenseBoost;
    char benefit[48];
    int built;
    int locked;
    char discoverSource[32];
    char reqTech[24];
    char reqTechName[32];
} RoomBlueprint;

// Candidates for Recruitment
#define MAX_CANDIDATES 6
typedef struct {
    char id[16];
    char name[48];
    char role[32];
    char trait[48];
    int health;
    int morale;
    int str, agi, inte;
} Candidate;

// Game State
typedef struct {
    int day;
    int phase; // 0: Dawn, 1: Midday, 2: Dusk, 3: Night
    int population;
    int maxPop;
    float food;
    float water;
    int powerGen;
    int powerLoad;
    float scrap;
    int meds;
    float morale;
    int defense;
    int barricadeHp;
    int barricadeMaxHp;
    int turretCount;
    int turretOverclock;
    int combatDrillLevel;
    int raidThreatDays;
    char lastRaidClan[48];
    int lastRaidAtk;
    int lastRaidDef;
    int lastRaidWon;
    int lastRaidDmg;
    int lastRaidScrap;
    int lastRaidMeds;
    int lastRaidFoodStolen;
    int lastRaidScrapStolen;
    int lastRaidInjured;
    int showRaidModal;
    float exteriorRads;
    
    // Morale, Health & Water Quality (Phase 12)
    float waterPurity;
    int martialLaw;
    int fortifiedRationsDays;
    int communalFeastDays;
    int addressCooldown;

    // Policies
    int policyFood;  // 0: Feast (1.5), 1: Standard (1.0), 2: Half (0.5), 3: Strict Emergency (0.25)
    int policyWater; // 0: Full Pure (1.0), 1: Strict (0.5), 2: Minimal Silt (0.25)
    int policyPower; // 0: Balanced, 1: Life Support, 2: Production
    
    // Facilities (up to 16)
    Facility facilities[16];
    int numFacilities;
    
    // Construction Blueprints & Sub-tab
    RoomBlueprint blueprints[MAX_BLUEPRINTS];
    int numBlueprints;
    int facilitySubTab; // 0: Active Facilities, 1: Construct Blueprints
    
    // Survivors & Recruitment
    Survivor survivors[MAX_SURVIVORS];
    int numSurvivors;
    Candidate candidates[MAX_CANDIDATES];
    int numCandidates;
    int survivorSubTab; // 0: Active Citizens, 1: Radio Beacon & Airlock
    
    // Expeditions (3)
    Expedition expeditions[MAX_EXPEDITIONS];
    int numExpeditions;
    
    // Logs
    LogEntry logs[MAX_LOG_ENTRIES];
    int logCount;
    int logScrollOffset;
    
    // Summary
    int showSummary;
    int summaryDay;
    float sumFoodDelta;
    float sumWaterDelta;
    float sumScrapDelta;
    int sumMorale;
    
    // Help Modal
    int showHelp;
    
    // Technology Research Tree
    int techWaterFilt;
    int techHydroponics;
    int techSolarArrays;
    int techReinforcedDef;
    int techRadShield;
    int techDeepSensors;
    int techAutoTooling;
    int techCombatStims;
    
    // Environmental Disasters & Weather Hazards (Phase 10)
    int weatherType; // 0: Clear, 1: Rad Storm, 2: Drought, 3: Toxic Rain, 4: Cold Snap
    int weatherDaysLeft;
    int weatherSeverity;
    int forecastType;
    int forecastEtaDays;
    int forecastSeverity;
    int cmRadBulkhead;
    int cmThermalOverdrive;
    int cmAcidNeutralizerDays;

    // Wasteland Caravan Trading System (Phase 11)
    int caravanPresent;
    int caravanFaction;
    int caravanDaysLeft;
    int caravanEtaDays;
    int caravanRepLevel;
    int caravanTradesCount;
    int caravanRareBought[5];

    // Active Tab
    int currentTab; // 0: Facilities, 1: Survivors, 2: Expeditions, 3: Defense, 4: Research, 5: Hazards, 6: Caravan, 7: Directives, 8: Manual
    
    int autoRun;
} GameState;

static GameState g_state;

// Caravan Faction Info
typedef struct {
    char name[48];
    char merchant[48];
    char quote[128];
    char badge[24];
} CaravanFactionInfo;

static const CaravanFactionInfo g_caravanFactions[5] = {
    { "DUST STRIDER GUILD", "Silas Vance", "Clean water and non-irradiated grain from the southern salt pans.", "DUST STRIDERS" },
    { "RUST BROTHERHOOD SCRAPPERS", "Forge-Master Vane", "Heavy alloy plating, reinforced bolts, and salvaged military tech.", "RUST BROTHERHOOD" },
    { "APOTHECARY NOMAD ENCLAVE", "Sister Morrigan", "Sterile pharmaceuticals, anti-rad tonics, and trauma stimpacks.", "APOTHECARY" },
    { "THE ARCHIVE CYBER-SALVAGERS", "Techno-Sage Kael", "Pre-war micro-fusion cells, encrypted keys, and high-gain antenna parts.", "CYBER-ARCHIVE" },
    { "LONE WASTELAND WANDERER", "Echo the Drifter", "Found some strange curios in the buried ruins. Willing to barter.", "LONE WANDERER" }
};

// Clickable UI Region tracking
typedef struct {
    RECT rect;
    int id;
    int param1;
    int param2;
} ClickableButton;

#define MAX_BUTTONS 150
static ClickableButton g_buttons[MAX_BUTTONS];
static int g_buttonCount = 0;

static void ClearButtons() {
    g_buttonCount = 0;
}

static void AddButton(int x, int y, int w, int h, int id, int p1, int p2) {
    if (g_buttonCount < MAX_BUTTONS) {
        g_buttons[g_buttonCount].rect.left = x;
        g_buttons[g_buttonCount].rect.top = y;
        g_buttons[g_buttonCount].rect.right = x + w;
        g_buttons[g_buttonCount].rect.bottom = y + h;
        g_buttons[g_buttonCount].id = id;
        g_buttons[g_buttonCount].param1 = p1;
        g_buttons[g_buttonCount].param2 = p2;
        g_buttonCount++;
    }
}

// Button IDs
enum {
    BTN_NONE = 0,
    BTN_THEME,
    BTN_CRT,
    BTN_AUDIO,
    BTN_HELP,
    BTN_RESET,
    BTN_TAB,
    BTN_ADVANCE,
    BTN_AUTORUN,
    BTN_FASTFORWARD,
    BTN_REPORT,
    BTN_CLEARLOG,
    BTN_FAC_WORKER,
    BTN_FAC_SUBTAB,
    BTN_FAC_UPGRADE,
    BTN_CONSTRUCT_ROOM,
    BTN_SURV_JOB,
    BTN_SURV_SUBTAB,
    BTN_BROADCAST_PING,
    BTN_BROADCAST_SPEC,
    BTN_ADMIT_CANDIDATE,
    BTN_DISMISS_CANDIDATE,
    BTN_DISPATCH_SCOUT,
    BTN_EXP_STIM_TOGGLE,
    BTN_TREAT_SURV,
    BTN_TREAT_RADAWAY,
    BTN_TREAT_DECON,
    BTN_MASS_RADAWAY,
    BTN_MASS_DECON,
    BTN_FLUSH_FILTERS,
    BTN_STERILIZE_WATER,
    BTN_FORTIFY_RATIONS,
    BTN_COMMUNAL_FEAST,
    BTN_DIST_LUXURIES,
    BTN_OVERSEER_ADDRESS,
    BTN_TOGGLE_MARTIAL_LAW,
    BTN_POLICY_FOOD,
    BTN_POLICY_WATER,
    BTN_POLICY_POWER,
    BTN_DEF_REPAIR,
    BTN_DEF_REINFORCE,
    BTN_DEF_TURRET,
    BTN_DEF_OVERCLOCK,
    BTN_DEF_DRILL,
    BTN_DEF_TOGGLE_GUARD,
    BTN_DEF_TEST_RAID,
    BTN_RESEARCH_TECH,
    BTN_HAZARD_TOGGLE_BULKHEAD,
    BTN_HAZARD_OVERPUMP,
    BTN_HAZARD_NEUTRALIZER,
    BTN_HAZARD_TOGGLE_THERMAL,
    BTN_HAZARD_SIMULATE,
    BTN_TRADE_BUY,
    BTN_TRADE_RARE,
    BTN_TRADE_SELL,
    BTN_HAIL_CARAVAN,
    BTN_CLOSE_RAID_MODAL,
    BTN_CLOSE_MODAL
};

// Logging
static void AddLog(const char* text, int type) {
    if (g_state.logCount >= MAX_LOG_ENTRIES) {
        for (int i = 0; i < MAX_LOG_ENTRIES - 1; i++) {
            g_state.logs[i] = g_state.logs[i + 1];
        }
        g_state.logCount = MAX_LOG_ENTRIES - 1;
    }
    LogEntry* le = &g_state.logs[g_state.logCount++];
    strncpy(le->text, text, sizeof(le->text) - 1);
    le->text[sizeof(le->text) - 1] = '\0';
    le->type = type;
    le->day = g_state.day;
    le->phase = g_state.phase;
}

static int GetUnassignedCount() {
    int count = 0;
    for (int i = 0; i < g_state.numSurvivors; i++) {
        if (strcmp(g_state.survivors[i].job, "unassigned") == 0 || strlen(g_state.survivors[i].job) == 0) {
            count++;
        }
    }
    return count;
}

static Survivor* GetFirstUnassignedSurvivor() {
    for (int i = 0; i < g_state.numSurvivors; i++) {
        if (strcmp(g_state.survivors[i].job, "unassigned") == 0 || strlen(g_state.survivors[i].job) == 0) {
            return &g_state.survivors[i];
        }
    }
    return NULL;
}

static float GetWorkerEfficiency(const Survivor* s, const Facility* fac) {
    float eff = 1.0f;
    if (s->health < 40) eff -= 0.35f;
    else if (s->health >= 85) eff += 0.15f;

    if (s->morale < 40) eff -= 0.25f;
    else if (s->morale >= 80) eff += 0.15f;

    // Radiation sickness penalty
    if (s->rads >= 75) eff -= 0.60f;
    else if (s->rads >= 50) eff -= 0.35f;
    else if (s->rads >= 25) eff -= 0.15f;

    // Vitamin fortification bonus
    if (g_state.fortifiedRationsDays > 0) eff += 0.15f;

    // Civil unrest worker slowdown (unless martial law)
    if (g_state.morale < 40.0f && !g_state.martialLaw) eff -= 0.25f;

    float statBonus = 0.0f;
    if (strcmp(fac->id, "farm") == 0 || strcmp(fac->id, "farm_aero") == 0) {
        if (s->agi > 5) statBonus += (s->agi - 5) * 0.15f;
        if (s->str > 5) statBonus += (s->str - 5) * 0.05f;
    } else if (strcmp(fac->id, "water") == 0 || strcmp(fac->id, "water_deep") == 0) {
        if (s->inte > 5) statBonus += (s->inte - 5) * 0.20f;
    } else if (strcmp(fac->id, "gen") == 0 || strcmp(fac->id, "gen_sub") == 0) {
        int combo = s->str + s->inte;
        if (combo > 10) statBonus += (combo - 10) * 0.10f;
    } else if (strcmp(fac->id, "workshop") == 0 || strcmp(fac->id, "smelter") == 0) {
        if (s->str > 5) statBonus += (s->str - 5) * 0.15f;
        if (s->agi > 5) statBonus += (s->agi - 5) * 0.05f;
    } else if (strcmp(fac->id, "cmd") == 0 || strcmp(fac->id, "infirmary") == 0) {
        if (s->inte > 5) statBonus += (s->inte - 5) * 0.15f;
    } else if (strcmp(fac->id, "security") == 0) {
        if (s->str > 5) statBonus += (s->str - 5) * 0.15f;
    }

    eff += statBonus;
    if (eff < 0.15f) eff = 0.15f;
    return eff;
}

// Technology Research Tree Data & Logic
typedef struct {
    int id;
    char name[36];
    char tag[20];
    char desc[90];
    char benefit[90];
    int cost;
    int reqTechId;
    char reqTechName[36];
    char unlockBp[20];
} TechInfo;

static const TechInfo g_techTree[8] = {
    { 0, "ADVANCED WATER FILTRATION", "LIFE SUPPORT",
      "Multi-stage reverse osmosis & particulate filtration eliminates isotopes.",
      "+50% Water yield & Unlocks Deep Filtration Reservoir", 40, -1, "", "bp_adv_water" },
    { 1, "HYDROPONIC CULTIVATION", "LIFE SUPPORT",
      "Nutrient-dense recirculating solution & dual-spectrum UV grow lamps.",
      "+25% Food yield & Unlocks Vertical Hydroponic Tower", 50, 0, "Advanced Water Filtration", "bp_hydro_tower" },
    { 2, "PHOTOVOLTAIC SOLAR ARRAYS", "ENERGY",
      "High-efficiency silicon panels capture ambient radiation and sunlight.",
      "+10 kW aux power boost & Unlocks Surface Solar Array", 45, -1, "", "bp_solar" },
    { 3, "REINFORCED DEFENSE GRID", "SECURITY",
      "Hardened titanium ballistic plating & electrified perimeter barrier.",
      "+100 Barricade HP, +15 Def, -25% Raid Dmg & Heavy Bastion", 55, -1, "", "bp_heavy_bastion" },
    { 4, "RAD-SHIELD NANO-COATINGS", "BIO-TECH",
      "Lead-ceramic polymer sealants applied to vents & shelter airlocks.",
      "-50% Radiation sickness hazard & faster convalescence", 45, -1, "", "" },
    { 5, "DEEP SCAVENGER SENSORS", "EXPLORATION",
      "Subterranean RF radar & magnetic resonance detectors for scouts.",
      "+35% Expedition scrap yield & 2x Blueprint discovery rate", 50, 2, "Photovoltaic Solar Arrays", "" },
    { 6, "AUTOMATED TOOLING BENCHES", "ENGINEERING",
      "Computerized pneumatic presses & plasma cutters for rapid manufacture.",
      "-25% Scrap cost for all rooms, upgrades, and defenses", 60, 1, "Hydroponic Cultivation", "" },
    { 7, "CYBERNETIC COMBAT STIMS", "SECURITY",
      "Adrenal neural injectors & subcutaneous ballistic mesh for guards.",
      "+35% Guard defense power & protects against raid casualties", 65, 3, "Reinforced Defense Grid", "" }
};

static int IsTechResearched(int techId) {
    switch (techId) {
        case 0: return g_state.techWaterFilt;
        case 1: return g_state.techHydroponics;
        case 2: return g_state.techSolarArrays;
        case 3: return g_state.techReinforcedDef;
        case 4: return g_state.techRadShield;
        case 5: return g_state.techDeepSensors;
        case 6: return g_state.techAutoTooling;
        case 7: return g_state.techCombatStims;
    }
    return 0;
}

static int GetResearchedCount() {
    int count = 0;
    for (int i = 0; i < 8; i++) {
        if (IsTechResearched(i)) count++;
    }
    return count;
}

static int GetEffectiveScrapCost(int baseCost) {
    if (g_state.techAutoTooling) {
        return (baseCost * 3) / 4;
    }
    return baseCost;
}

static void DoResearch(int techId) {
    if (techId < 0 || techId >= 8) return;
    if (IsTechResearched(techId)) return;

    const TechInfo* ti = &g_techTree[techId];
    if (ti->reqTechId >= 0 && !IsTechResearched(ti->reqTechId)) {
        char buf[128];
        sprintf(buf, "Research halted: [%s] requires %s!", ti->name, ti->reqTechName);
        AddLog(buf, 1);
        PlaySfx(3);
        return;
    }

    int cost = GetEffectiveScrapCost(ti->cost);
    if (g_state.scrap < (float)cost) {
        char buf[128];
        sprintf(buf, "Insufficient Tech Scrap! Need %d scrap for %s.", cost, ti->name);
        AddLog(buf, 1);
        PlaySfx(3);
        return;
    }

    g_state.scrap -= (float)cost;
    switch (techId) {
        case 0:
            g_state.techWaterFilt = 1;
            break;
        case 1:
            g_state.techHydroponics = 1;
            break;
        case 2:
            g_state.techSolarArrays = 1;
            break;
        case 3:
            g_state.techReinforcedDef = 1;
            g_state.barricadeMaxHp += 100;
            g_state.barricadeHp += 100;
            break;
        case 4:
            g_state.techRadShield = 1;
            break;
        case 5:
            g_state.techDeepSensors = 1;
            break;
        case 6:
            g_state.techAutoTooling = 1;
            break;
        case 7:
            g_state.techCombatStims = 1;
            break;
    }

    // Unlock blueprint if attached
    if (strlen(ti->unlockBp) > 0) {
        for (int b = 0; b < g_state.numBlueprints; b++) {
            if (strcmp(g_state.blueprints[b].id, ti->unlockBp) == 0) {
                g_state.blueprints[b].locked = 0;
            }
        }
    }

    char logBuf[160];
    sprintf(logBuf, "RESEARCH COMPLETE: [%s] operational! (%s)", ti->name, ti->benefit);
    AddLog(logBuf, 3);
    PlaySfx(SFX_RESEARCH);
}

static int CalculateTotalDefense() {
    int baseHull = 10;
    if (g_state.techReinforcedDef) baseHull += 15;
    int maxBar = (g_state.barricadeMaxHp > 0) ? g_state.barricadeMaxHp : 100;
    int barDef = (g_state.barricadeHp * 20) / maxBar;
    int turDef = g_state.turretCount * 18 + (g_state.turretOverclock ? 10 : 0);
    int facDef = 0;
    for (int f = 0; f < g_state.numFacilities; f++) {
        if (strcmp(g_state.facilities[f].id, "security") == 0) {
            facDef += 15 + (g_state.facilities[f].level - 1) * 10;
        } else if (strcmp(g_state.facilities[f].id, "cmd") == 0) {
            facDef += g_state.facilities[f].level * 5;
        } else if (strcmp(g_state.facilities[f].id, "bp_radshield") == 0) {
            facDef += 25;
        } else if (strcmp(g_state.facilities[f].id, "bp_heavy_bastion") == 0) {
            facDef += 30;
        }
    }
    int guardDef = 0;
    int drillBonus = g_state.combatDrillLevel * 3;
    for (int s = 0; s < g_state.numSurvivors; s++) {
        if (strcmp(g_state.survivors[s].job, "security") == 0) {
            guardDef += (g_state.survivors[s].str * 2) + drillBonus;
        } else if (strcmp(g_state.survivors[s].job, "expedition") != 0) {
            guardDef += g_state.survivors[s].str / 2;
        }
    }
    if (g_state.techCombatStims) {
        guardDef = (int)(guardDef * 1.35f);
    }
    return baseHull + barDef + turDef + facDef + guardDef;
}

static void CalculateTotals(float* foodProd, float* foodNeed, float* waterProd, float* waterNeed, int* powerGen, int* powerLoad, float* scrapProd) {
    *foodProd = 0;
    *waterProd = 0;
    *powerGen = 0;
    *powerLoad = 0;
    *scrapProd = 0;

    int isBlackout = (g_state.powerGen < g_state.powerLoad);

    for (int i = 0; i < g_state.numFacilities; i++) {
        Facility* fac = &g_state.facilities[i];

        int assignedCount = 0;
        float effSum = 0.0f;
        for (int s = 0; s < g_state.numSurvivors; s++) {
            if (strcmp(g_state.survivors[s].job, fac->id) == 0) {
                assignedCount++;
                effSum += GetWorkerEfficiency(&g_state.survivors[s], fac);
            }
        }
        fac->assigned = assignedCount;

        if (fac->powerProd > 0) {
            *powerGen += (assignedCount > 0 ? (int)(fac->powerProd + effSum * 8.0f) : 6);
        } else {
            *powerLoad += fac->powerCost;
        }

        if (assignedCount > 0) {
            int isPowered = (!isBlackout || fac->powerPriority <= 2);
            if (isPowered) {
                *foodProd += fac->foodProd * effSum;
                *waterProd += fac->waterProd * effSum;
                *scrapProd += fac->scrapProd * effSum;
            }
        }
    }

    // Technology Tree bonuses
    if (g_state.techWaterFilt) {
        *waterProd *= 1.5f;
    }
    if (g_state.techHydroponics) {
        *foodProd *= 1.25f;
    }
    if (g_state.techSolarArrays) {
        *powerGen += 10;
    }

    // Weather Hazards Impact (Phase 10)
    if (g_state.weatherType == 1) { // Rad Storm
        if (!g_state.techRadShield) {
            *powerGen = (*powerGen > 5) ? (*powerGen - 5) : 6;
        }
    } else if (g_state.weatherType == 2) { // Drought
        *waterProd *= 0.5f;
    } else if (g_state.weatherType == 4) { // Cold Snap
        *powerGen = (int)(*powerGen * 0.65f);
        if (!g_state.cmThermalOverdrive) {
            *powerLoad += 4;
        }
    }

    // Countermeasure Auxiliary Power Draws
    if (g_state.cmRadBulkhead) {
        *powerLoad += 5;
    }
    if (g_state.cmThermalOverdrive) {
        *powerLoad += 4;
    }

    float foodPer = (g_state.policyFood == 0) ? 1.5f : ((g_state.policyFood == 2) ? 0.5f : ((g_state.policyFood == 3) ? 0.25f : 1.0f));
    float waterPer = (g_state.policyWater == 1) ? 0.5f : ((g_state.policyWater == 2) ? 0.25f : 1.0f);
    if (g_state.weatherType == 2) {
        waterPer *= 1.5f; // +50% water thirst during drought
    }

    *foodNeed = g_state.population * foodPer;
    *waterNeed = g_state.population * waterPer;
    g_state.defense = CalculateTotalDefense();
}

static void GenerateCandidate(Candidate* c, int isSpecialist) {
    const char* fnames[] = { "Jax", "Talia", "Dex", "Wren", "Rory", "Cassian", "Sloan", "Zeke", "Nova", "Silas", "Daphne", "Gideon", "Kira", "Nolan", "Vera", "Rook" };
    const char* lnames[] = { "Cross", "Vance", "Mercer", "Gant", "Holloway", "Kane", "Sterling", "Blackwood", "Frost", "Carver" };
    int fi = rand() % 16;
    int li = rand() % 10;
    sprintf(c->id, "c_%d", (int)time(NULL) % 10000 + rand() % 1000);
    sprintf(c->name, "%s %s", fnames[fi], lnames[li]);

    if (isSpecialist) {
        int r = rand() % 5;
        if (r == 0) {
            strcpy(c->role, "Master Agronomist");
            strcpy(c->trait, "Bio-Harvester (+30% Crops)");
            c->str = 4 + rand() % 3; c->agi = 7 + rand() % 3; c->inte = 6 + rand() % 3;
        } else if (r == 1) {
            strcpy(c->role, "Chief Grid Engineer");
            strcpy(c->trait, "Turbine Guru (+30% Power)");
            c->str = 6 + rand() % 3; c->agi = 4 + rand() % 3; c->inte = 7 + rand() % 3;
        } else if (r == 2) {
            strcpy(c->role, "Filtration Chemist");
            strcpy(c->trait, "Pure Flow (+30% Water)");
            c->str = 4 + rand() % 3; c->agi = 5 + rand() % 3; c->inte = 8 + rand() % 2;
        } else if (r == 3) {
            strcpy(c->role, "Combat Scavenger");
            strcpy(c->trait, "Salvage Instinct (+30% Scrap)");
            c->str = 7 + rand() % 3; c->agi = 7 + rand() % 3; c->inte = 4 + rand() % 3;
        } else {
            strcpy(c->role, "Trauma Surgeon");
            strcpy(c->trait, "Field Triage (+Healing)");
            c->str = 4 + rand() % 3; c->agi = 6 + rand() % 3; c->inte = 8 + rand() % 2;
        }
    } else {
        int r = rand() % 5;
        if (r == 0) {
            strcpy(c->role, "Wasteland Drifter");
            strcpy(c->trait, "Hardy (+10% Survival)");
            c->str = 4 + rand() % 4; c->agi = 4 + rand() % 4; c->inte = 4 + rand() % 4;
        } else if (r == 1) {
            strcpy(c->role, "Salvage Scout");
            strcpy(c->trait, "Eagle Eye (+Scrap)");
            c->str = 5 + rand() % 3; c->agi = 6 + rand() % 3; c->inte = 4 + rand() % 3;
        } else if (r == 2) {
            strcpy(c->role, "Settlement Farmer");
            strcpy(c->trait, "Crop Tender (+Food)");
            c->str = 4 + rand() % 4; c->agi = 6 + rand() % 4; c->inte = 5 + rand() % 3;
        } else if (r == 3) {
            strcpy(c->role, "Apprentice Mechanic");
            strcpy(c->trait, "Wrench Hand (+Power)");
            c->str = 6 + rand() % 3; c->agi = 4 + rand() % 4; c->inte = 5 + rand() % 3;
        } else {
            strcpy(c->role, "Caravan Outrider");
            strcpy(c->trait, "Vigilant (+Defense)");
            c->str = 6 + rand() % 3; c->agi = 6 + rand() % 3; c->inte = 4 + rand() % 3;
        }
    }

    c->health = 80 + rand() % 20;
    c->morale = 70 + rand() % 25;
}

static float GetCaravanDiscount() {
    int lvl = g_state.caravanRepLevel;
    if (lvl == 2) return 0.05f;
    if (lvl == 3) return 0.10f;
    if (lvl == 4) return 0.15f;
    if (lvl >= 5) return 0.25f;
    return 0.0f;
}

static int GetDiscountedTradeCost(int baseCost) {
    float disc = GetCaravanDiscount();
    int c = (int)((float)baseCost * (1.0f - disc) + 0.5f);
    return c < 1 ? 1 : c;
}

static void AddCaravanTradeRep() {
    g_state.caravanTradesCount++;
    int req = 3;
    if (g_state.caravanRepLevel == 2) req = 8;
    else if (g_state.caravanRepLevel == 3) req = 15;
    else if (g_state.caravanRepLevel == 4) req = 25;

    if (g_state.caravanTradesCount >= req && g_state.caravanRepLevel < 5) {
        g_state.caravanRepLevel++;
        char buf[128];
        const char* repNames[] = { "", "Neutral", "Welcomed (5% Disc)", "Trusted (10% Disc)", "Honored (15% Disc)", "Revered (25% Disc)" };
        sprintf(buf, "★ CARAVAN REPUTATION: Barter Trust raised to Level %d [%s]!", g_state.caravanRepLevel, repNames[g_state.caravanRepLevel]);
        AddLog(buf, 3);
    }
}

static void BuyCaravanItem(int itemId) {
    if (!g_state.caravanPresent) return;
    int costs[] = { 8, 8, 30, 12 };
    if (itemId < 0 || itemId >= 4) return;
    int cost = GetDiscountedTradeCost(costs[itemId]);
    if (g_state.scrap < (float)cost) {
        AddLog("CARAVAN: Insufficient Tech Scrap to complete barter!", 1);
        PlaySfx(SFX_ALERT);
        return;
    }

    g_state.scrap -= (float)cost;
    if (itemId == 0) {
        g_state.food += 10.0f;
        AddLog("CARAVAN: Purchased Small Food Rations Crate (+10 Food).", 3);
    } else if (itemId == 1) {
        g_state.water += 10.0f;
        AddLog("CARAVAN: Purchased Purified Water Jug Pack (+10 Water).", 3);
    } else if (itemId == 2) {
        g_state.food += 25.0f;
        g_state.water += 25.0f;
        AddLog("CARAVAN: Purchased Bulk Food & Water Bulkhead Crate (+25 Food, +25 Water).", 3);
    } else if (itemId == 3) {
        g_state.meds += 2;
        AddLog("CARAVAN: Purchased Surgical Stimpacks Pack (+2 Meds).", 3);
    }

    AddCaravanTradeRep();
    PlaySfx(SFX_TRADE);
}

static void BuyRareItem(int rareIdx) {
    if (!g_state.caravanPresent || rareIdx < 0 || rareIdx >= 5) return;
    if (g_state.caravanRareBought[rareIdx]) return;

    int rCosts[] = { 22, 25, 28, 35, 18 };
    int cost = GetDiscountedTradeCost(rCosts[rareIdx]);
    if (g_state.scrap < (float)cost) {
        AddLog("CARAVAN: Insufficient Tech Scrap to purchase rare artifact!", 1);
        PlaySfx(SFX_ALERT);
        return;
    }

    g_state.scrap -= (float)cost;
    g_state.caravanRareBought[rareIdx] = 1;
    AddCaravanTradeRep();

    if (rareIdx == 0) { // Rad-away
        for (int i = 0; i < g_state.numSurvivors; i++) {
            g_state.survivors[i].health += 20;
            if (g_state.survivors[i].health > 100) g_state.survivors[i].health = 100;
        }
        if (g_state.exteriorRads > 3.0f) g_state.exteriorRads -= 3.0f;
        AddLog("★ RARE ARTIFACT: Rad-Away Anti-Toxin Cask administered! All survivors healed +20 HP.", 3);
    } else if (rareIdx == 1) { // Super enzyme
        g_state.food += 30.0f;
        AddLog("★ RARE ARTIFACT: Hydroponic Super-Growth Enzyme applied (+30 Food).", 3);
    } else if (rareIdx == 2) { // Armor plates
        g_state.barricadeMaxHp += 25;
        g_state.barricadeHp = g_state.barricadeMaxHp;
        AddLog("★ RARE ARTIFACT: Heavy Ballistic Armor welded! Barricades repaired & Max HP raised.", 3);
    } else if (rareIdx == 3) { // Targeting module
        g_state.defense += 12;
        AddLog("★ RARE ARTIFACT: Sentry Targeting Overdrive Module installed (+12 Base Def).", 3);
    } else if (rareIdx == 4) { // Luxury cassettes
        g_state.morale += 20.0f;
        if (g_state.morale > 100.0f) g_state.morale = 100.0f;
        AddLog("★ RARE ARTIFACT: Pre-war Luxury Coffee & Cassettes distributed! Vault Morale +20%.", 3);
    }

    PlaySfx(SFX_TRADE);
}

static void SellSurplusItem(int sellIdx) {
    if (!g_state.caravanPresent) return;
    if (sellIdx == 0) {
        if (g_state.food >= 15.0f) {
            g_state.food -= 15.0f;
            g_state.scrap += 10.0f;
            AddCaravanTradeRep();
            AddLog("CARAVAN: Exported 15 Food Rations (+10 Tech Scrap).", 3);
            PlaySfx(SFX_TRADE);
        } else {
            AddLog("CARAVAN: Insufficient food surplus to export (need 15)!", 1);
            PlaySfx(SFX_ALERT);
        }
    } else if (sellIdx == 1) {
        if (g_state.water >= 15.0f) {
            g_state.water -= 15.0f;
            g_state.scrap += 10.0f;
            AddCaravanTradeRep();
            AddLog("CARAVAN: Exported 15 Purified Water (+10 Tech Scrap).", 3);
            PlaySfx(SFX_TRADE);
        } else {
            AddLog("CARAVAN: Insufficient water surplus to export (need 15)!", 1);
            PlaySfx(SFX_ALERT);
        }
    } else if (sellIdx == 2) {
        if (g_state.scrap >= 20.0f) {
            g_state.scrap -= 20.0f;
            g_state.meds += 2;
            AddCaravanTradeRep();
            AddLog("CARAVAN: Bartered 20 Tech Scrap for 2 Medical Stimpacks.", 3);
            PlaySfx(SFX_TRADE);
        } else {
            AddLog("CARAVAN: Insufficient scrap to barter for meds (need 20)!", 1);
            PlaySfx(SFX_ALERT);
        }
    }
}

static void HailCaravan() {
    if (g_state.scrap >= 15.0f && g_state.powerGen >= 10) {
        g_state.scrap -= 15.0f;
        g_state.caravanPresent = 1;
        g_state.caravanFaction = (g_state.caravanFaction + 1 + rand() % 4) % 5;
        g_state.caravanDaysLeft = 3;
        g_state.caravanEtaDays = 0;
        for (int k = 0; k < 5; k++) g_state.caravanRareBought[k] = 0;

        char buf[128];
        sprintf(buf, "RADIO BEACON: Commercial hail sent! [%s] (%s) arrived at the airlock!", g_caravanFactions[g_state.caravanFaction].badge, g_caravanFactions[g_state.caravanFaction].merchant);
        AddLog(buf, 3);
        PlaySfx(SFX_TRADE);
    } else {
        AddLog("RADIO: Insufficient scrap (15) or reactor power (10 kW) to broadcast trade beacon!", 1);
        PlaySfx(SFX_ALERT);
    }
}

static void ProcessCaravanDay() {
    if (g_state.caravanPresent) {
        g_state.caravanDaysLeft--;
        if (g_state.caravanDaysLeft <= 0) {
            g_state.caravanPresent = 0;
            g_state.caravanEtaDays = 3 + rand() % 2;
            char buf[128];
            sprintf(buf, "CARAVAN DEPARTURE: [%s] departed into the wasteland. Next caravan in %d days.", g_caravanFactions[g_state.caravanFaction].badge, g_state.caravanEtaDays);
            AddLog(buf, 0);
        }
    } else {
        g_state.caravanEtaDays--;
        if (g_state.caravanEtaDays <= 0) {
            g_state.caravanPresent = 1;
            g_state.caravanFaction = (g_state.caravanFaction + 1 + rand() % 3) % 5;
            g_state.caravanDaysLeft = 2 + rand() % 2;
            g_state.caravanEtaDays = 0;
            for (int k = 0; k < 5; k++) g_state.caravanRareBought[k] = 0;

            char buf[128];
            sprintf(buf, "CARAVAN ARRIVAL: Nomadic merchants from [%s] arrived at the airlock!", g_caravanFactions[g_state.caravanFaction].badge);
            AddLog(buf, 3);
            PlaySfx(SFX_TRADE);
        } else if (g_state.caravanEtaDays == 1) {
            AddLog("RADIO SENSORS: Dust cloud detected on radar. Nomadic caravan arriving tomorrow.", 0);
        }
    }
}

static void TriggerDailyEvent() {
    int roll = rand() % 100;
    if (roll < 25) {
        if (g_state.numCandidates < MAX_CANDIDATES) {
            Candidate* c = &g_state.candidates[g_state.numCandidates++];
            GenerateCandidate(c, rand() % 4 == 0);
            char buf[128];
            sprintf(buf, "AIRLOCK SENSOR: Refugee %s (%s) detected! Review at Radio Beacon.", c->name, c->role);
            AddLog(buf, 3);
            PlaySfx(SFX_SCOUT);
        }
    } else if (roll < 42) {
        g_state.exteriorRads += 1.5f;
        if (g_state.exteriorRads > 15.0f) g_state.exteriorRads = 15.0f;
        AddLog("ENVIRONMENTAL: Heavy radioactive dust storm whipping outside. Rads increased.", 1);
    } else if (roll < 58) {
        int scrapBonus = 6 + (rand() % 8);
        g_state.scrap += scrapBonus;
        char buf[128];
        sprintf(buf, "MAINTENANCE: Engineering recovered +%d scrap from disused conduits.", scrapBonus);
        AddLog(buf, 0);
    }
}

static void TriggerRaiderAttack(int isManual) {
    PlaySfx(SFX_SIREN);
    int totalDef = CalculateTotalDefense();
    g_state.defense = totalDef;

    const char* clanNames[] = {
        "Rustfang Marauders",
        "Iron Skull Warband",
        "Rad-Scorpion Reavers",
        "Dune Stalker Syndicate",
        "Super-Mutant Siege"
    };
    const char* clanDescs[] = {
        "Scavenger bandits armed with pipe rifles and scrap cleavers.",
        "Armored wasteland raiders driving spiked battle buggies.",
        "Mutant beasts led by cybernetic wasteland slavers.",
        "High-tech mercenaries with heavy plasma weaponry.",
        "Massive irradiated brutes wielding concrete rebar clubs."
    };
    int baseAtks[] = { 30, 48, 65, 85, 110 };

    int clanIdx = 0;
    if (g_state.day >= 12) clanIdx = 4;
    else if (g_state.day >= 9) clanIdx = 3;
    else if (g_state.day >= 6) clanIdx = 2;
    else if (g_state.day >= 3) clanIdx = 1;

    int variance = (rand() % 15) - 7;
    int assaultPower = baseAtks[clanIdx] + variance;
    if (assaultPower < 20) assaultPower = 20;

    int won = (totalDef >= assaultPower);
    g_state.lastRaidWon = won;
    strncpy(g_state.lastRaidClan, clanNames[clanIdx], sizeof(g_state.lastRaidClan) - 1);
    g_state.lastRaidAtk = assaultPower;
    g_state.lastRaidDef = totalDef;

    if (won) {
        int maxB = (g_state.barricadeMaxHp > 0) ? g_state.barricadeMaxHp : 100;
        int dmg = (assaultPower * 25) / (totalDef > 0 ? totalDef : 1);
        if (g_state.techReinforcedDef) dmg = (dmg * 3) / 4;
        if (dmg < 5) dmg = 5;
        g_state.barricadeHp -= dmg;
        if (g_state.barricadeHp < 0) g_state.barricadeHp = 0;
        g_state.lastRaidDmg = dmg;

        int scrapGained = 20 + rand() % 25;
        int medsGained = (rand() % 100 < 60) ? 1 : 2;
        g_state.scrap += scrapGained;
        g_state.meds += medsGained;
        g_state.morale += 6.0f;
        if (g_state.morale > 100.0f) g_state.morale = 100.0f;

        g_state.lastRaidScrap = scrapGained;
        g_state.lastRaidMeds = medsGained;
        g_state.lastRaidFoodStolen = 0;
        g_state.lastRaidScrapStolen = 0;
        g_state.lastRaidInjured = 0;

        char logBuf[160];
        sprintf(logBuf, "RAID REPELLED: %s (Atk %d) crushed by Vault Defenses (%d pts)! Salvaged +%d Scrap, +%d Meds.",
            clanNames[clanIdx], assaultPower, totalDef, scrapGained, medsGained);
        AddLog(logBuf, 3);
        PlaySfx(SFX_CYCLE);
    } else {
        int deficit = assaultPower - totalDef;
        int dmg = 30 + (deficit * 8) / 10;
        if (g_state.techReinforcedDef) dmg = (dmg * 3) / 4;
        g_state.barricadeHp -= dmg;
        if (g_state.barricadeHp < 0) g_state.barricadeHp = 0;
        g_state.lastRaidDmg = dmg;

        int foodStolen = 10 + deficit / 2;
        if (foodStolen > (int)g_state.food) foodStolen = (int)g_state.food;
        int scrapStolen = 15 + (deficit * 6) / 10;
        if (scrapStolen > (int)g_state.scrap) scrapStolen = (int)g_state.scrap;

        g_state.food -= foodStolen;
        g_state.scrap -= scrapStolen;
        g_state.morale -= 12.0f;
        if (g_state.morale < 15.0f) g_state.morale = 15.0f;

        int injuredCount = 0;
        for (int s = 0; s < g_state.numSurvivors; s++) {
            if (strcmp(g_state.survivors[s].job, "security") == 0 || rand() % 100 < 40) {
                int wound = 15 + rand() % 25;
                if (g_state.techCombatStims) wound = (wound * 65) / 100;
                g_state.survivors[s].health -= wound;
                if (g_state.survivors[s].health < 10) g_state.survivors[s].health = 10;
                injuredCount++;
            }
        }

        g_state.lastRaidScrap = 0;
        g_state.lastRaidMeds = 0;
        g_state.lastRaidFoodStolen = foodStolen;
        g_state.lastRaidScrapStolen = scrapStolen;
        g_state.lastRaidInjured = injuredCount;

        char logBuf[160];
        sprintf(logBuf, "BREACH: %s (Atk %d) breached Defenses (%d pts)! Stole %d Food, %d Scrap. %d injured!",
            clanNames[clanIdx], assaultPower, totalDef, foodStolen, scrapStolen, injuredCount);
        AddLog(logBuf, 2);
        PlaySfx(3);
    }

    g_state.showRaidModal = 1;
}

static void ProcessNewDay() {
    g_state.day++;
    PlaySfx(2);

    float foodP, foodN, waterP, waterN, scrapP;
    int pGen, pLoad;
    CalculateTotals(&foodP, &foodN, &waterP, &waterN, &pGen, &pLoad, &scrapP);

    g_state.powerGen = pGen;
    g_state.powerLoad = pLoad;

    // 1. Water Purity & Filtration Simulation
    int hasWaterStaff = 0;
    for (int f = 0; f < g_state.numFacilities; f++) {
        if ((strcmp(g_state.facilities[f].id, "water") == 0 || strcmp(g_state.facilities[f].id, "water_deep") == 0) && g_state.facilities[f].assigned > 0) {
            hasWaterStaff = 1; break;
        }
    }
    if (hasWaterStaff) {
        g_state.waterPurity += 15.0f;
        if (g_state.waterPurity > 100.0f) g_state.waterPurity = 100.0f;
    } else {
        g_state.waterPurity -= 5.0f;
        if (g_state.waterPurity < 20.0f) g_state.waterPurity = 20.0f;
    }

    if (g_state.policyWater == 2) {
        g_state.waterPurity -= 10.0f;
        if (g_state.waterPurity < 10.0f) g_state.waterPurity = 10.0f;
    }

    // 2. Resource Changes
    float netFood = foodP - foodN;
    float netWater = waterP - waterN;

    g_state.food += netFood;
    if (g_state.food < 0.0f) g_state.food = 0.0f;

    g_state.water += netWater;
    if (g_state.water < 0.0f) g_state.water = 0.0f;

    g_state.scrap += scrapP;

    // 3. Starvation & Thirst Effects
    if (g_state.food <= 0.0f) {
        g_state.morale -= 8.0f;
        if (g_state.morale < 10.0f) g_state.morale = 10.0f;
        for (int i = 0; i < g_state.numSurvivors; i++) {
            g_state.survivors[i].health -= 12;
            if (g_state.survivors[i].health < 10) g_state.survivors[i].health = 10;
            g_state.survivors[i].hunger += 25;
            if (g_state.survivors[i].hunger > 100) g_state.survivors[i].hunger = 100;
        }
        AddLog("CRITICAL: Food storages depleted! Survivors are starving!", 2);
        PlaySfx(3);
    }

    if (g_state.water <= 0.0f) {
        g_state.morale -= 12.0f;
        if (g_state.morale < 10.0f) g_state.morale = 10.0f;
        for (int i = 0; i < g_state.numSurvivors; i++) {
            g_state.survivors[i].health -= 18;
            if (g_state.survivors[i].health < 5) g_state.survivors[i].health = 5;
            g_state.survivors[i].thirst += 30;
            if (g_state.survivors[i].thirst > 100) g_state.survivors[i].thirst = 100;
        }
        AddLog("CRITICAL: Water tanks bone dry! Severe dehydration spreading!", 2);
        PlaySfx(3);
    }

    // 4. Food & Water Policy Daily Effects
    if (g_state.policyFood == 0 && g_state.food > 0.0f) { // Feast
        g_state.morale += 6.0f;
        if (g_state.morale > 100.0f) g_state.morale = 100.0f;
        for (int i = 0; i < g_state.numSurvivors; i++) {
            g_state.survivors[i].health += 5;
            if (g_state.survivors[i].health > 100) g_state.survivors[i].health = 100;
            g_state.survivors[i].morale += 6;
            if (g_state.survivors[i].morale > 100) g_state.survivors[i].morale = 100;
        }
    } else if (g_state.policyFood == 2) { // Half
        g_state.morale -= 4.0f;
        if (g_state.morale < 10.0f) g_state.morale = 10.0f;
    } else if (g_state.policyFood == 3) { // Starve
        g_state.morale -= 12.0f;
        if (g_state.morale < 10.0f) g_state.morale = 10.0f;
        for (int i = 0; i < g_state.numSurvivors; i++) {
            g_state.survivors[i].health -= 8;
            if (g_state.survivors[i].health < 10) g_state.survivors[i].health = 10;
            g_state.survivors[i].hunger += 20;
            if (g_state.survivors[i].hunger > 100) g_state.survivors[i].hunger = 100;
        }
    }

    if (g_state.policyWater == 0 && g_state.water > 0.0f && g_state.waterPurity >= 80.0f) {
        g_state.morale += 2.0f;
        if (g_state.morale > 100.0f) g_state.morale = 100.0f;
    } else if (g_state.policyWater == 1) {
        g_state.morale -= 3.0f;
        if (g_state.morale < 10.0f) g_state.morale = 10.0f;
    } else if (g_state.policyWater == 2) {
        g_state.morale -= 10.0f;
        if (g_state.morale < 10.0f) g_state.morale = 10.0f;
        for (int i = 0; i < g_state.numSurvivors; i++) {
            g_state.survivors[i].rads += 8;
            if (g_state.survivors[i].rads > 100) g_state.survivors[i].rads = 100;
        }
        AddLog("WATER RECYCLING: Silt recycling contaminated dwellers with +8 Rads!", 1);
    }

    // Contaminated Aquifer rad ingestion
    if (g_state.waterPurity < 70.0f && g_state.water > 0.0f) {
        int radIngest = (int)((70.0f - g_state.waterPurity) * 0.25f);
        if (radIngest < 2) radIngest = 2;
        for (int i = 0; i < g_state.numSurvivors; i++) {
            g_state.survivors[i].rads += radIngest;
            if (g_state.survivors[i].rads > 100) g_state.survivors[i].rads = 100;
        }
        char contamBuf[128];
        sprintf(contamBuf, "WATER CONTAMINATION: Silt in unpurified water caused +%d Rads in dwellers.", radIngest);
        AddLog(contamBuf, 1);
    }

    // 5. Radiation Sickness & Natural Convalescence
    int acuteSickCount = 0;
    for (int i = 0; i < g_state.numSurvivors; i++) {
        Survivor* s = &g_state.survivors[i];
        if (s->rads >= 75) {
            s->health -= 7;
            if (s->health < 5) s->health = 5;
            s->morale -= 8;
            if (s->morale < 10) s->morale = 10;
            acuteSickCount++;
        } else if (s->rads >= 50) {
            s->health -= 3;
            if (s->health < 10) s->health = 10;
            s->morale -= 4;
            if (s->morale < 15) s->morale = 15;
            acuteSickCount++;
        } else if (s->rads >= 25) {
            s->health -= 1;
            if (s->health < 15) s->health = 15;
            s->morale -= 2;
            if (s->morale < 20) s->morale = 20;
        }

        if (g_state.food > 0.0f && g_state.water > 0.0f && s->rads < 50) {
            if (s->health < 100) s->health += 4;
            if (s->health > 100) s->health = 100;
            s->hunger -= 15;
            if (s->hunger < 0) s->hunger = 0;
            s->thirst -= 20;
            if (s->thirst < 0) s->thirst = 0;
        }
    }

    if (acuteSickCount > 0) {
        char radAlert[128];
        sprintf(radAlert, "RAD SICKNESS: %d dweller(s) suffering severe radiation poisoning symptoms!", acuteSickCount);
        AddLog(radAlert, 1);
    }

    // 6. Directives & Timers Countdown
    if (g_state.fortifiedRationsDays > 0) {
        g_state.fortifiedRationsDays--;
        if (g_state.fortifiedRationsDays == 0) AddLog("NUTRITIONAL: Vitamin fortification in rations expired.", 0);
    }
    if (g_state.communalFeastDays > 0) g_state.communalFeastDays--;
    if (g_state.addressCooldown > 0) g_state.addressCooldown--;

    // 7. Expeditions update
    for (int i = 0; i < g_state.numExpeditions; i++) {
        Expedition* exp = &g_state.expeditions[i];
        if (strlen(exp->assignedScout) > 0) {
            exp->daysRemaining--;
            if (exp->daysRemaining <= 0) {
                // Return
                Survivor* scout = NULL;
                for (int s = 0; s < g_state.numSurvivors; s++) {
                    if (strcmp(g_state.survivors[s].id, exp->assignedScout) == 0) {
                        scout = &g_state.survivors[s];
                        break;
                    }
                }
                int agi = scout ? scout->agi : 5;
                int str = scout ? scout->str : 5;
                int inte = scout ? scout->inte : 5;

                float fMult = (0.8f + (rand() % 40) / 100.0f) * (1.0f + (agi - 5) * 0.05f);
                float sMult = (0.8f + (rand() % 40) / 100.0f) * (1.0f + (inte - 5) * 0.08f);
                float mMult = (0.7f + (rand() % 60) / 100.0f) * (1.0f + (agi - 5) * 0.05f);

                int fFound = (int)(exp->potentialFood * fMult);
                int sFound = (int)(exp->potentialScrap * sMult);
                if (g_state.techDeepSensors) sFound = (int)(sFound * 1.35f);
                int mFound = (int)(exp->potentialMeds * mMult);
                if (mFound < 1) mFound = 1;

                g_state.food += fFound;
                g_state.scrap += sFound;
                g_state.meds += mFound;

                // Hazard damage & rads
                int baseDmg = (exp->riskLevel == 4) ? (30 + rand() % 25) : ((exp->riskLevel == 3) ? (20 + rand() % 20) : ((exp->riskLevel == 2) ? (12 + rand() % 15) : (5 + rand() % 10)));
                int baseRads = (exp->riskLevel == 4) ? 35 : ((exp->riskLevel == 3) ? 22 : ((exp->riskLevel == 2) ? 12 : 5));
                int dmg = baseDmg - str * 2 - (int)(agi * 1.5f);
                if (dmg < 0) dmg = 0;
                int stimUsed = 0;
                if (exp->hasStimpack && dmg > 10) {
                    dmg = (int)(dmg * 0.4f);
                    stimUsed = 1;
                }

                if (scout) {
                    scout->health -= dmg;
                    if (scout->health < 15) scout->health = 15;
                    scout->rads += baseRads;
                    if (scout->rads > 100) scout->rads = 100;
                    strcpy(scout->job, "unassigned");
                    scout->morale += (dmg > 25) ? -10 : 10;
                    if (scout->morale > 100) scout->morale = 100;
                    if (scout->morale < 20) scout->morale = 20;
                }

                // Blueprint Decryption
                char bpMsg[64] = "";
                if (strlen(exp->blueprintId) > 0) {
                    for (int b = 0; b < g_state.numBlueprints; b++) {
                        if (strcmp(g_state.blueprints[b].id, exp->blueprintId) == 0 && g_state.blueprints[b].locked) {
                            int roll = rand() % 100;
                            int chance = 35 + inte * 7 + (g_state.techDeepSensors ? 30 : 0);
                            if (roll <= chance || exp->riskLevel >= 4) {
                                g_state.blueprints[b].locked = 0;
                                sprintf(bpMsg, " * BLUEPRINT: [%s] unlocked!", g_state.blueprints[b].name);
                            }
                            break;
                        }
                    }
                }

                exp->assignedScout[0] = '\0';
                exp->hasStimpack = 0;

                char buf[160];
                sprintf(buf, "%s returned from %s! Salvaged: +%d Food, +%d Scrap, +%d Meds. [-%d HP, +%d Rads]%s%s", scout ? scout->name : "Scout", exp->name, fFound, sFound, mFound, dmg, baseRads, stimUsed ? " (Stimpack stabilized)" : "", bpMsg);
                AddLog(buf, strlen(bpMsg) > 0 ? 3 : 4);
                PlaySfx(strlen(bpMsg) > 0 ? 2 : 4);
            }
        }
    }

    // 8. Medical triage & Radiation decontamination in Infirmary
    int hasDoctor = 0;
    for (int s = 0; s < g_state.numSurvivors; s++) {
        if (strcmp(g_state.survivors[s].job, "infirmary") == 0) { hasDoctor = 1; break; }
    }
    int hasSurg = 0;
    for (int b = 0; b < g_state.numBlueprints; b++) {
        if (strcmp(g_state.blueprints[b].id, "bp_medsurge") == 0 && g_state.blueprints[b].built) { hasSurg = 1; break; }
    }
    if (hasDoctor || hasSurg) {
        int treatedCount = 0;
        for (int s = 0; s < g_state.numSurvivors; s++) {
            if (g_state.survivors[s].health < 90 || g_state.survivors[s].rads > 20) treatedCount++;
        }
        if (treatedCount > 0 && (g_state.meds > 0 || hasSurg)) {
            if (!hasSurg && g_state.meds > 0) g_state.meds--;
            for (int s = 0; s < g_state.numSurvivors; s++) {
                if (g_state.survivors[s].health < 90 || g_state.survivors[s].rads > 20) {
                    g_state.survivors[s].health += (hasSurg ? 25 : 15);
                    if (g_state.survivors[s].health > 100) g_state.survivors[s].health = 100;
                    g_state.survivors[s].rads -= (hasSurg ? 20 : 10);
                    if (g_state.survivors[s].rads < 0) g_state.survivors[s].rads = 0;
                }
            }
            char medBuf[128];
            sprintf(medBuf, "MED-LAB: Triage & decon wash administered to %d dweller(s).", treatedCount);
            AddLog(medBuf, 0);
        }
    }

    // 9. Raider Incursion Countdown
    g_state.raidThreatDays--;
    if (g_state.raidThreatDays <= 0) {
        TriggerRaiderAttack(0);
        g_state.raidThreatDays = 3 + rand() % 3;
    } else if (g_state.raidThreatDays == 1) {
        AddLog("RADAR WARNING: Raider vanguard spotted 5km away! Raid expected tomorrow!", 1);
        PlaySfx(3);
    }

    // 10. Environmental Disasters & Weather Progression
    if (g_state.cmAcidNeutralizerDays > 0) {
        g_state.cmAcidNeutralizerDays--;
    }

    if (g_state.weatherDaysLeft > 0) {
        g_state.weatherDaysLeft--;
        if (g_state.weatherDaysLeft <= 0) {
            g_state.weatherType = 0;
            AddLog("WEATHER CLEARED: Hostile weather hazard subsided. Atmospheric conditions stable.", 3);
        }
    }

    g_state.forecastEtaDays--;
    if (g_state.forecastEtaDays <= 0) {
        g_state.weatherType = g_state.forecastType;
        g_state.weatherDaysLeft = 2 + (rand() % 2);
        const char* wNames[] = { "Calm Skies", "Ion Radiation Storm", "Scorching Drought", "Corrosive Acid Rain", "Sub-Zero Cold Snap" };
        char wAlert[160];
        sprintf(wAlert, "METEOROLOGICAL ALERT: [%s] struck the shelter! Review Hazards console.", wNames[g_state.weatherType]);
        AddLog(wAlert, 2);
        PlaySfx(SFX_HAZARD);

        g_state.forecastType = 1 + (rand() % 4);
        g_state.forecastEtaDays = 4 + (rand() % 3);
    } else if (g_state.forecastEtaDays == 1) {
        const char* wNames[] = { "Calm Skies", "Ion Radiation Storm", "Scorching Drought", "Corrosive Acid Rain", "Sub-Zero Cold Snap" };
        char fAlert[160];
        sprintf(fAlert, "DOPPLER WARNING: [%s] is 1 day away from striking vault perimeter!", wNames[g_state.forecastType]);
        AddLog(fAlert, 1);
        PlaySfx(SFX_ALERT);
    }

    // Hazard Daily Damage
    if (g_state.weatherType == 1) { // Rad Storm
        g_state.exteriorRads = 13.0f + (rand() % 50) / 10.0f;
        int hasShield = g_state.cmRadBulkhead || g_state.techRadShield;
        if (!hasShield) {
            for (int b = 0; b < g_state.numBlueprints; b++) {
                if (strcmp(g_state.blueprints[b].id, "bp_radshield") == 0 && g_state.blueprints[b].built) {
                    hasShield = 1; break;
                }
            }
        }
        if (hasShield) {
            AddLog("RAD-SHIELD: Lead bulkheads & nano-coatings absorbed gamma fallout storm.", 0);
        } else {
            for (int s = 0; s < g_state.numSurvivors; s++) {
                g_state.survivors[s].health -= 12;
                if (g_state.survivors[s].health < 10) g_state.survivors[s].health = 10;
                g_state.survivors[s].rads += 25;
                if (g_state.survivors[s].rads > 100) g_state.survivors[s].rads = 100;
                g_state.survivors[s].morale -= 8;
                if (g_state.survivors[s].morale < 15) g_state.survivors[s].morale = 15;
            }
            g_state.waterPurity = (g_state.waterPurity > 20.0f) ? (g_state.waterPurity - 20.0f) : 10.0f;
            AddLog("RADIATION SICKNESS: Gamma storm penetrated vents! Dwellers suffered +25 Rads trauma!", 2);
            PlaySfx(SFX_GEIGER);
        }
    } else if (g_state.weatherType == 2) { // Drought
        g_state.exteriorRads = 5.2f + (rand() % 8) / 10.0f;
        AddLog("DROUGHT: Arid heatwave dried up surface aquifers. Water yields depleted.", 1);
    } else if (g_state.weatherType == 3) { // Acid Rain
        g_state.exteriorRads = 6.5f + (rand() % 15) / 10.0f;
        if (g_state.cmAcidNeutralizerDays > 0) {
            AddLog("NEUTRALIZER ACTIVE: Alkaline coating protected barricades from acid rain.", 0);
        } else {
            int acidDmg = 25 + (rand() % 12);
            g_state.barricadeHp -= acidDmg;
            if (g_state.barricadeHp < 0) g_state.barricadeHp = 0;
            for (int s = 0; s < g_state.numSurvivors; s++) {
                if (strcmp(g_state.survivors[s].job, "security") == 0 || strcmp(g_state.survivors[s].job, "unassigned") == 0) {
                    g_state.survivors[s].health -= 6;
                    if (g_state.survivors[s].health < 10) g_state.survivors[s].health = 10;
                }
            }
            char acidBuf[128];
            sprintf(acidBuf, "ACID EROSION: Acid rain corroded barricades (-%d HP) & burned outdoor guards.", acidDmg);
            AddLog(acidBuf, 1);
        }
    } else if (g_state.weatherType == 4) { // Cold Snap
        g_state.exteriorRads = 3.8f + (rand() % 5) / 10.0f;
        int isBlackout = (g_state.powerGen < g_state.powerLoad);
        if (isBlackout && !g_state.cmThermalOverdrive) {
            for (int s = 0; s < g_state.numSurvivors; s++) {
                g_state.survivors[s].health -= 15;
                if (g_state.survivors[s].health < 5) g_state.survivors[s].health = 5;
                g_state.survivors[s].morale -= 12;
                if (g_state.survivors[s].morale < 10) g_state.survivors[s].morale = 10;
            }
            AddLog("HYPOTHERMIA: Power brownout during Sub-Zero Cold Snap froze living bunks (-15 HP)!", 2);
            PlaySfx(SFX_ALERT);
        } else {
            AddLog("NUCLEAR FROST: Sub-zero frost reduced bio-fuel generator power efficiency.", 0);
        }
    } else {
        g_state.exteriorRads = 4.0f + (rand() % 6) / 10.0f;
    }

    // 11. Morale & Civil Unrest / Mutiny Simulation
    if (g_state.martialLaw) {
        if (g_state.morale > 50.0f) g_state.morale = 50.0f;
        AddLog("MARTIAL LAW: Armed security patrols maintain mandatory vault curfew. Civil unrest suppressed.", 0);
    } else {
        if (g_state.morale < 25.0f) {
            if ((rand() % 100) < 65) {
                int fLost = (int)g_state.food;
                if (fLost > 15) fLost = 10 + rand() % 6;
                int sLost = (int)g_state.scrap;
                if (sLost > 20) sLost = 10 + rand() % 11;
                g_state.food -= fLost;
                g_state.scrap -= sLost;
                for (int s = 0; s < g_state.numSurvivors; s++) {
                    if ((rand() % 100) < 40) {
                        g_state.survivors[s].health -= 15;
                        if (g_state.survivors[s].health < 10) g_state.survivors[s].health = 10;
                    }
                }
                char riotBuf[160];
                sprintf(riotBuf, "VAULT MUTINY: Discontent rioters broke into storage! Looted -%d Food, -%d Scrap, and injured dwellers!", fLost, sLost);
                AddLog(riotBuf, 2);
                PlaySfx(3);
            }
        } else if (g_state.morale < 45.0f) {
            if ((rand() % 100) < 45) {
                int sabScrap = (int)g_state.scrap;
                if (sabScrap > 10) sabScrap = 8 + rand() % 8;
                g_state.scrap -= sabScrap;
                g_state.barricadeHp -= 15;
                if (g_state.barricadeHp < 0) g_state.barricadeHp = 0;
                char strikeBuf[160];
                sprintf(strikeBuf, "CIVIL UNREST: Angry workers sabotaged perimeter conduits (-%d Scrap, -15 Barricade HP)!", sabScrap);
                AddLog(strikeBuf, 1);
                PlaySfx(3);
            }
        }
    }

    TriggerDailyEvent();
    ProcessCaravanDay();

    g_state.summaryDay = g_state.day;
    g_state.sumFoodDelta = netFood;
    g_state.sumWaterDelta = netWater;
    g_state.sumScrapDelta = scrapP;
    g_state.sumMorale = (int)g_state.morale;

    char cycBuf[128];
    sprintf(cycBuf, "=== CYCLE DAY %d COMMENCED. Rations distributed, reactor cycled. ===", g_state.day);
    AddLog(cycBuf, 3);
}

static void AdvanceCycle() {
    g_state.phase++;
    if (g_state.phase >= 4) {
        g_state.phase = 0;
        ProcessNewDay();
    } else {
        const char* pnames[] = { "Dawn", "Midday", "Dusk", "Night" };
        char buf[128];
        sprintf(buf, "Sun shifted across the irradiated horizon. Time: %s.", pnames[g_state.phase]);
        AddLog(buf, 0);
        PlaySfx(1);
    }
}

static void InitGameState() {
    memset(&g_state, 0, sizeof(GameState));
    g_state.day = 1;
    g_state.phase = 0;
    g_state.population = 5;
    g_state.maxPop = 10;
    g_state.food = 30.0f;
    g_state.water = 30.0f;
    g_state.powerGen = 22;
    g_state.powerLoad = 16;
    g_state.scrap = 55.0f;
    g_state.meds = 5;
    g_state.morale = 85.0f;
    g_state.defense = 35;
    g_state.barricadeHp = 100;
    g_state.barricadeMaxHp = 100;
    g_state.turretCount = 1;
    g_state.turretOverclock = 0;
    g_state.combatDrillLevel = 1;
    g_state.raidThreatDays = 3;
    strcpy(g_state.lastRaidClan, "None yet");
    g_state.lastRaidWon = 1;
    g_state.showRaidModal = 0;
    g_state.exteriorRads = 4.2f;
    g_state.policyFood = 1;
    g_state.policyWater = 0;
    g_state.policyPower = 0;
    g_state.waterPurity = 95.0f;
    g_state.martialLaw = 0;
    g_state.fortifiedRationsDays = 0;
    g_state.communalFeastDays = 0;
    g_state.addressCooldown = 0;

    // 7 Facilities
    g_state.numFacilities = 7;
    // 0: cmd
    strcpy(g_state.facilities[0].id, "cmd");
    strcpy(g_state.facilities[0].name, "OVERSEER COMMAND");
    strcpy(g_state.facilities[0].desc, "Central security terminal & vital sensor hub.");
    g_state.facilities[0].level = 1;
    g_state.facilities[0].maxWorkers = 1;
    g_state.facilities[0].assigned = 1;
    g_state.facilities[0].powerCost = 2;
    g_state.facilities[0].powerPriority = 1;

    // 1: gen
    strcpy(g_state.facilities[1].id, "gen");
    strcpy(g_state.facilities[1].name, "DIESEL-BIO GENERATOR");
    strcpy(g_state.facilities[1].desc, "Main electrical turbine supplying power grid.");
    g_state.facilities[1].level = 1;
    g_state.facilities[1].maxWorkers = 2;
    g_state.facilities[1].assigned = 1;
    g_state.facilities[1].powerCost = 0;
    g_state.facilities[1].powerProd = 22;
    g_state.facilities[1].powerPriority = 0;

    // 2: water
    strcpy(g_state.facilities[2].id, "water");
    strcpy(g_state.facilities[2].name, "WATER PURIFIER");
    strcpy(g_state.facilities[2].desc, "Deep groundwater radiation filtration system.");
    g_state.facilities[2].level = 1;
    g_state.facilities[2].maxWorkers = 2;
    g_state.facilities[2].assigned = 1;
    g_state.facilities[2].powerCost = 5;
    g_state.facilities[2].waterProd = 8;
    g_state.facilities[2].powerPriority = 2;

    // 3: farm
    strcpy(g_state.facilities[3].id, "farm");
    strcpy(g_state.facilities[3].name, "HYDROPONICS BAY");
    strcpy(g_state.facilities[3].desc, "UV-spectrum crop beds producing nutrient paste.");
    g_state.facilities[3].level = 1;
    g_state.facilities[3].maxWorkers = 2;
    g_state.facilities[3].assigned = 1;
    g_state.facilities[3].powerCost = 5;
    g_state.facilities[3].foodProd = 8;
    g_state.facilities[3].powerPriority = 2;

    // 4: quarters
    strcpy(g_state.facilities[4].id, "quarters");
    strcpy(g_state.facilities[4].name, "LIVING QUARTERS");
    strcpy(g_state.facilities[4].desc, "Bunk beds and privacy partitions for dwellers.");
    g_state.facilities[4].level = 1;
    g_state.facilities[4].maxWorkers = 0;
    g_state.facilities[4].assigned = 0;
    g_state.facilities[4].powerCost = 2;
    g_state.facilities[4].powerPriority = 3;

    // 5: infirmary
    strcpy(g_state.facilities[5].id, "infirmary");
    strcpy(g_state.facilities[5].name, "MED-LAB INFIRMARY");
    strcpy(g_state.facilities[5].desc, "Decontamination showers and surgical beds.");
    g_state.facilities[5].level = 1;
    g_state.facilities[5].maxWorkers = 1;
    g_state.facilities[5].assigned = 1;
    g_state.facilities[5].powerCost = 2;
    g_state.facilities[5].powerPriority = 1;

    // 6: workshop
    strcpy(g_state.facilities[6].id, "workshop");
    strcpy(g_state.facilities[6].name, "SCRAP WORKSHOP");
    strcpy(g_state.facilities[6].desc, "Tool benches to reforge junk into usable tech.");
    g_state.facilities[6].level = 1;
    g_state.facilities[6].maxWorkers = 2;
    g_state.facilities[6].assigned = 0;
    g_state.facilities[6].powerCost = 3;
    g_state.facilities[6].scrapProd = 4;
    g_state.facilities[6].powerPriority = 4;

    // 10 Construction Blueprints & Sub-Tab
    g_state.facilitySubTab = 0;
    g_state.numBlueprints = 10;

    strcpy(g_state.blueprints[0].id, "gen_sub");
    strcpy(g_state.blueprints[0].name, "BIO-TURBINE SUB-STATION");
    strcpy(g_state.blueprints[0].desc, "Auxiliary geothermal & bio-fuel turbine generating power.");
    g_state.blueprints[0].cost = 45;
    g_state.blueprints[0].powerProd = 18;
    g_state.blueprints[0].powerCost = 0;
    g_state.blueprints[0].foodProd = 0;
    g_state.blueprints[0].waterProd = 0;
    g_state.blueprints[0].scrapProd = 0;
    g_state.blueprints[0].maxWorkers = 2;
    g_state.blueprints[0].powerPriority = 0;
    strcpy(g_state.blueprints[0].benefit, "+18 kW Base Power Generation");
    g_state.blueprints[0].built = 0;
    g_state.blueprints[0].locked = 0;

    strcpy(g_state.blueprints[1].id, "water_deep");
    strcpy(g_state.blueprints[1].name, "DEEP WELL PURIFIER");
    strcpy(g_state.blueprints[1].desc, "Subterranean bore-well with reverse-osmosis filtration.");
    g_state.blueprints[1].cost = 40;
    g_state.blueprints[1].powerProd = 0;
    g_state.blueprints[1].foodProd = 0;
    g_state.blueprints[1].waterProd = 7;
    g_state.blueprints[1].scrapProd = 0;
    g_state.blueprints[1].powerCost = 4;
    g_state.blueprints[1].maxWorkers = 2;
    g_state.blueprints[1].powerPriority = 2;
    strcpy(g_state.blueprints[1].benefit, "+7 Water/worker (Draw: -4 kW)");
    g_state.blueprints[1].built = 0;
    g_state.blueprints[1].locked = 0;

    strcpy(g_state.blueprints[2].id, "farm_aero");
    strcpy(g_state.blueprints[2].name, "AEROPONIC GREEN BAY");
    strcpy(g_state.blueprints[2].desc, "High-yield vertical misting racks cultivating crops.");
    g_state.blueprints[2].cost = 40;
    g_state.blueprints[2].powerProd = 0;
    g_state.blueprints[2].foodProd = 7;
    g_state.blueprints[2].waterProd = 0;
    g_state.blueprints[2].scrapProd = 0;
    g_state.blueprints[2].powerCost = 4;
    g_state.blueprints[2].maxWorkers = 2;
    g_state.blueprints[2].powerPriority = 2;
    strcpy(g_state.blueprints[2].benefit, "+7 Food/worker (Draw: -4 kW)");
    g_state.blueprints[2].built = 0;
    g_state.blueprints[2].locked = 0;

    strcpy(g_state.blueprints[3].id, "quarters_ext");
    strcpy(g_state.blueprints[3].name, "BARRACKS EXPANSION");
    strcpy(g_state.blueprints[3].desc, "Reinforced bulkhead sector with triple sleeper pods.");
    g_state.blueprints[3].cost = 35;
    g_state.blueprints[3].powerProd = 0;
    g_state.blueprints[3].foodProd = 0;
    g_state.blueprints[3].waterProd = 0;
    g_state.blueprints[3].scrapProd = 0;
    g_state.blueprints[3].powerCost = 2;
    g_state.blueprints[3].maxWorkers = 0;
    g_state.blueprints[3].powerPriority = 3;
    g_state.blueprints[3].popBoost = 6;
    strcpy(g_state.blueprints[3].benefit, "+6 Vault Citizen Capacity");
    g_state.blueprints[3].built = 0;
    g_state.blueprints[3].locked = 0;

    strcpy(g_state.blueprints[4].id, "security");
    strcpy(g_state.blueprints[4].name, "SECURITY TURRET BASTION");
    strcpy(g_state.blueprints[4].desc, "Automated 50-cal sentry turrets and blast-door armor.");
    g_state.blueprints[4].cost = 50;
    g_state.blueprints[4].powerProd = 0;
    g_state.blueprints[4].foodProd = 0;
    g_state.blueprints[4].waterProd = 0;
    g_state.blueprints[4].scrapProd = 0;
    g_state.blueprints[4].powerCost = 3;
    g_state.blueprints[4].maxWorkers = 1;
    g_state.blueprints[4].powerPriority = 1;
    g_state.blueprints[4].defenseBoost = 15;
    strcpy(g_state.blueprints[4].benefit, "+15 Vault Defense Rating");
    g_state.blueprints[4].built = 0;
    g_state.blueprints[4].locked = 0;

    strcpy(g_state.blueprints[5].id, "smelter");
    strcpy(g_state.blueprints[5].name, "SCRAP SMELTER CRUSHER");
    strcpy(g_state.blueprints[5].desc, "Heavy magnetic furnace to melt down wasteland junk.");
    g_state.blueprints[5].cost = 45;
    g_state.blueprints[5].powerProd = 0;
    g_state.blueprints[5].foodProd = 0;
    g_state.blueprints[5].waterProd = 0;
    g_state.blueprints[5].scrapProd = 5;
    g_state.blueprints[5].powerCost = 3;
    g_state.blueprints[5].maxWorkers = 2;
    g_state.blueprints[5].powerPriority = 4;
    strcpy(g_state.blueprints[5].benefit, "+5 Tech Scrap/worker (-3 kW)");
    g_state.blueprints[5].built = 0;
    g_state.blueprints[5].locked = 0;

    // Discoverable Blueprints
    strcpy(g_state.blueprints[6].id, "bp_fusion");
    strcpy(g_state.blueprints[6].name, "FUSION MICRO-REACTOR");
    strcpy(g_state.blueprints[6].desc, "High-density atomic cell recovered from Substation.");
    g_state.blueprints[6].cost = 65;
    g_state.blueprints[6].powerProd = 35;
    g_state.blueprints[6].powerCost = 0;
    g_state.blueprints[6].foodProd = 0;
    g_state.blueprints[6].waterProd = 0;
    g_state.blueprints[6].scrapProd = 0;
    g_state.blueprints[6].maxWorkers = 2;
    g_state.blueprints[6].powerPriority = 0;
    strcpy(g_state.blueprints[6].benefit, "+35 kW Nuclear Power");
    g_state.blueprints[6].built = 0;
    g_state.blueprints[6].locked = 1;
    strcpy(g_state.blueprints[6].discoverSource, "Substation Ruins");

    strcpy(g_state.blueprints[7].id, "bp_medsurge");
    strcpy(g_state.blueprints[7].name, "AUTOMATED SURGERY WING");
    strcpy(g_state.blueprints[7].desc, "Robotic surgical theater & reconstruction pods.");
    g_state.blueprints[7].cost = 55;
    g_state.blueprints[7].powerProd = 0;
    g_state.blueprints[7].foodProd = 0;
    g_state.blueprints[7].waterProd = 0;
    g_state.blueprints[7].scrapProd = 0;
    g_state.blueprints[7].powerCost = 4;
    g_state.blueprints[7].maxWorkers = 1;
    g_state.blueprints[7].powerPriority = 1;
    strcpy(g_state.blueprints[7].benefit, "+Auto-heals Wounded (+25 HP/d)");
    g_state.blueprints[7].built = 0;
    g_state.blueprints[7].locked = 1;
    strcpy(g_state.blueprints[7].discoverSource, "Hospital Complex");

    strcpy(g_state.blueprints[8].id, "bp_radshield");
    strcpy(g_state.blueprints[8].name, "RAD-SHIELD AIRLOCK GATE");
    strcpy(g_state.blueprints[8].desc, "Electromagnetic shielding plates from Military Armory.");
    g_state.blueprints[8].cost = 60;
    g_state.blueprints[8].powerProd = 0;
    g_state.blueprints[8].foodProd = 0;
    g_state.blueprints[8].waterProd = 0;
    g_state.blueprints[8].scrapProd = 0;
    g_state.blueprints[8].powerCost = 3;
    g_state.blueprints[8].maxWorkers = 1;
    g_state.blueprints[8].powerPriority = 1;
    g_state.blueprints[8].defenseBoost = 25;
    strcpy(g_state.blueprints[8].benefit, "+25 Defense & Rad Protection");
    g_state.blueprints[8].built = 0;
    g_state.blueprints[8].locked = 1;
    strcpy(g_state.blueprints[8].discoverSource, "Military Armory");

    strcpy(g_state.blueprints[9].id, "bp_genevault");
    strcpy(g_state.blueprints[9].name, "HYDROPONIC GENE-VAULT");
    strcpy(g_state.blueprints[9].desc, "Irradiated hybrid seed incubator from Vault 811.");
    g_state.blueprints[9].cost = 60;
    g_state.blueprints[9].powerProd = 0;
    g_state.blueprints[9].foodProd = 12;
    g_state.blueprints[9].waterProd = 0;
    g_state.blueprints[9].scrapProd = 0;
    g_state.blueprints[9].powerCost = 5;
    g_state.blueprints[9].maxWorkers = 2;
    g_state.blueprints[9].powerPriority = 2;
    strcpy(g_state.blueprints[9].benefit, "+12 Food/worker (Super-Crops)");
    g_state.blueprints[9].built = 0;
    g_state.blueprints[9].locked = 1;
    strcpy(g_state.blueprints[9].discoverSource, "Vault 811 Archive");

    // 10: bp_adv_water
    strcpy(g_state.blueprints[10].id, "bp_adv_water");
    strcpy(g_state.blueprints[10].name, "DEEP FILTRATION RESERVOIR");
    strcpy(g_state.blueprints[10].desc, "Ultra-purification ion vats removing all radioactive isotopes.");
    g_state.blueprints[10].cost = 55;
    g_state.blueprints[10].powerProd = 0;
    g_state.blueprints[10].foodProd = 0;
    g_state.blueprints[10].waterProd = 14;
    g_state.blueprints[10].scrapProd = 0;
    g_state.blueprints[10].powerCost = 4;
    g_state.blueprints[10].maxWorkers = 2;
    g_state.blueprints[10].powerPriority = 2;
    strcpy(g_state.blueprints[10].benefit, "+14 Water/worker (Adv Filtration)");
    g_state.blueprints[10].built = 0;
    g_state.blueprints[10].locked = 1;
    strcpy(g_state.blueprints[10].discoverSource, "Adv Water Tech");
    strcpy(g_state.blueprints[10].reqTech, "adv_water");
    strcpy(g_state.blueprints[10].reqTechName, "Adv Water Filtration");

    // 11: bp_hydro_tower
    strcpy(g_state.blueprints[11].id, "bp_hydro_tower");
    strcpy(g_state.blueprints[11].name, "VERTICAL HYDROPONIC TOWER");
    strcpy(g_state.blueprints[11].desc, "Automated aeroponic towers delivering high-yield crop harvests.");
    g_state.blueprints[11].cost = 55;
    g_state.blueprints[11].powerProd = 0;
    g_state.blueprints[11].foodProd = 14;
    g_state.blueprints[11].waterProd = 0;
    g_state.blueprints[11].scrapProd = 0;
    g_state.blueprints[11].powerCost = 4;
    g_state.blueprints[11].maxWorkers = 2;
    g_state.blueprints[11].powerPriority = 2;
    strcpy(g_state.blueprints[11].benefit, "+14 Food/worker (Vertical Farm)");
    g_state.blueprints[11].built = 0;
    g_state.blueprints[11].locked = 1;
    strcpy(g_state.blueprints[11].discoverSource, "Hydroponics Tech");
    strcpy(g_state.blueprints[11].reqTech, "hydroponics");
    strcpy(g_state.blueprints[11].reqTechName, "Hydroponic Cultivation");

    // 12: bp_solar
    strcpy(g_state.blueprints[12].id, "bp_solar");
    strcpy(g_state.blueprints[12].name, "SURFACE SOLAR ARRAY");
    strcpy(g_state.blueprints[12].desc, "Roof-mounted photovoltaic cells providing clean power without fuel.");
    g_state.blueprints[12].cost = 50;
    g_state.blueprints[12].powerProd = 20;
    g_state.blueprints[12].powerCost = 0;
    g_state.blueprints[12].foodProd = 0;
    g_state.blueprints[12].waterProd = 0;
    g_state.blueprints[12].scrapProd = 0;
    g_state.blueprints[12].maxWorkers = 0;
    g_state.blueprints[12].powerPriority = 0;
    strcpy(g_state.blueprints[12].benefit, "+20 kW Clean Power (0 Workers)");
    g_state.blueprints[12].built = 0;
    g_state.blueprints[12].locked = 1;
    strcpy(g_state.blueprints[12].discoverSource, "Solar Arrays Tech");
    strcpy(g_state.blueprints[12].reqTech, "solar_arrays");
    strcpy(g_state.blueprints[12].reqTechName, "Photovoltaic Solar");

    // 13: bp_heavy_bastion
    strcpy(g_state.blueprints[13].id, "bp_heavy_bastion");
    strcpy(g_state.blueprints[13].name, "HEAVY VULCAN BASTION");
    strcpy(g_state.blueprints[13].desc, "Dual rotary autocannons & bunker embrasure shielding the airlock.");
    g_state.blueprints[13].cost = 65;
    g_state.blueprints[13].powerProd = 0;
    g_state.blueprints[13].powerCost = 4;
    g_state.blueprints[13].foodProd = 0;
    g_state.blueprints[13].waterProd = 0;
    g_state.blueprints[13].scrapProd = 0;
    g_state.blueprints[13].maxWorkers = 1;
    g_state.blueprints[13].powerPriority = 1;
    g_state.blueprints[13].defenseBoost = 30;
    strcpy(g_state.blueprints[13].benefit, "+30 Vault Defense Rating");
    g_state.blueprints[13].built = 0;
    g_state.blueprints[13].locked = 1;
    strcpy(g_state.blueprints[13].discoverSource, "Reinforced Def Tech");
    strcpy(g_state.blueprints[13].reqTech, "reinforced_def");
    strcpy(g_state.blueprints[13].reqTechName, "Reinforced Defenses");

    g_state.numBlueprints = 14;

    // Survivors
    g_state.numSurvivors = 5;
    const char* sNames[] = { "Laura Martinez", "Marcus Vance", "Elena Rostova", "Caleb Stone", "Dr. Arthur Ross" };
    const char* sRoles[] = { "Chief Overseer", "Senior Engineer", "Hydrologist", "Agronomist", "Vault Physician" };
    const char* sJobs[] = { "cmd", "gen", "water", "farm", "infirmary" };
    int sHealth[] = { 100, 95, 100, 90, 100 };
    int sMorale[] = { 90, 85, 80, 85, 88 };
    int sStr[] = { 4, 6, 5, 5, 3 };
    int sAgi[] = { 5, 4, 5, 6, 4 };
    int sInt[] = { 8, 7, 6, 6, 9 };

    for (int i = 0; i < 5; i++) {
        sprintf(g_state.survivors[i].id, "s%d", i + 1);
        strcpy(g_state.survivors[i].name, sNames[i]);
        strcpy(g_state.survivors[i].role, sRoles[i]);
        strcpy(g_state.survivors[i].job, sJobs[i]);
        g_state.survivors[i].health = sHealth[i];
        g_state.survivors[i].morale = sMorale[i];
        g_state.survivors[i].hunger = 0;
        g_state.survivors[i].thirst = 0;
        g_state.survivors[i].str = sStr[i];
        g_state.survivors[i].agi = sAgi[i];
        g_state.survivors[i].inte = sInt[i];
    }

    // Candidates & Recruitment
    g_state.survivorSubTab = 0;
    g_state.numCandidates = 2;
    strcpy(g_state.candidates[0].id, "c1");
    strcpy(g_state.candidates[0].name, "Jonas Ward");
    strcpy(g_state.candidates[0].role, "Wasteland Machinist");
    strcpy(g_state.candidates[0].trait, "Overclock: +25% Power");
    g_state.candidates[0].health = 85;
    g_state.candidates[0].morale = 80;
    g_state.candidates[0].str = 7;
    g_state.candidates[0].agi = 4;
    g_state.candidates[0].inte = 7;

    strcpy(g_state.candidates[1].id, "c2");
    strcpy(g_state.candidates[1].name, "Mira Chen");
    strcpy(g_state.candidates[1].role, "Desert Botanist");
    strcpy(g_state.candidates[1].trait, "Green Thumb: +25% Hydro");
    g_state.candidates[1].health = 90;
    g_state.candidates[1].morale = 85;
    g_state.candidates[1].str = 4;
    g_state.candidates[1].agi = 8;
    g_state.candidates[1].inte = 6;

    // Expeditions (5 Ruins)
    g_state.numExpeditions = 5;
    strcpy(g_state.expeditions[0].id, "exp1");
    strcpy(g_state.expeditions[0].name, "Old Supermarket Ruins");
    strcpy(g_state.expeditions[0].desc, "Warehouse 4km north. Food caches & basic supplies.");
    g_state.expeditions[0].duration = 1;
    strcpy(g_state.expeditions[0].risk, "Low Risk");
    g_state.expeditions[0].riskLevel = 1;
    g_state.expeditions[0].assignedScout[0] = '\0';
    g_state.expeditions[0].potentialFood = 18;
    g_state.expeditions[0].potentialScrap = 12;
    g_state.expeditions[0].potentialMeds = 2;
    g_state.expeditions[0].blueprintId[0] = '\0';
    g_state.expeditions[0].blueprintReward[0] = '\0';
    g_state.expeditions[0].hasStimpack = 0;

    strcpy(g_state.expeditions[1].id, "exp2");
    strcpy(g_state.expeditions[1].name, "Derelict Power Substation");
    strcpy(g_state.expeditions[1].desc, "Transformer yard with capacitors & reactor schematics.");
    g_state.expeditions[1].duration = 2;
    strcpy(g_state.expeditions[1].risk, "Moderate Risk");
    g_state.expeditions[1].riskLevel = 2;
    g_state.expeditions[1].assignedScout[0] = '\0';
    g_state.expeditions[1].potentialFood = 6;
    g_state.expeditions[1].potentialScrap = 32;
    g_state.expeditions[1].potentialMeds = 2;
    strcpy(g_state.expeditions[1].blueprintId, "bp_fusion");
    strcpy(g_state.expeditions[1].blueprintReward, "Fusion Micro-Reactor");
    g_state.expeditions[1].hasStimpack = 0;

    strcpy(g_state.expeditions[2].id, "exp3");
    strcpy(g_state.expeditions[2].name, "County Hospital Complex");
    strcpy(g_state.expeditions[2].desc, "Surgical wing. Pharmaceuticals & surgery blueprints.");
    g_state.expeditions[2].duration = 2;
    strcpy(g_state.expeditions[2].risk, "Hazardous (Toxic)");
    g_state.expeditions[2].riskLevel = 2;
    g_state.expeditions[2].assignedScout[0] = '\0';
    g_state.expeditions[2].potentialFood = 8;
    g_state.expeditions[2].potentialScrap = 20;
    g_state.expeditions[2].potentialMeds = 7;
    strcpy(g_state.expeditions[2].blueprintId, "bp_medsurge");
    strcpy(g_state.expeditions[2].blueprintReward, "Automated Surgery Wing");
    g_state.expeditions[2].hasStimpack = 0;

    strcpy(g_state.expeditions[3].id, "exp4");
    strcpy(g_state.expeditions[3].name, "Military Armory Wreckage");
    strcpy(g_state.expeditions[3].desc, "Highway 80 motor pool. Weaponry, alloy & blast armor.");
    g_state.expeditions[3].duration = 2;
    strcpy(g_state.expeditions[3].risk, "High Hazard (Mines)");
    g_state.expeditions[3].riskLevel = 3;
    g_state.expeditions[3].assignedScout[0] = '\0';
    g_state.expeditions[3].potentialFood = 10;
    g_state.expeditions[3].potentialScrap = 45;
    g_state.expeditions[3].potentialMeds = 3;
    strcpy(g_state.expeditions[3].blueprintId, "bp_radshield");
    strcpy(g_state.expeditions[3].blueprintReward, "Rad-Shield Airlock Gate");
    g_state.expeditions[3].hasStimpack = 0;

    strcpy(g_state.expeditions[4].id, "exp5");
    strcpy(g_state.expeditions[4].name, "Vault 811 Tech Archive");
    strcpy(g_state.expeditions[4].desc, "Buried test vault. Experimental gene-vault schematics.");
    g_state.expeditions[4].duration = 3;
    strcpy(g_state.expeditions[4].risk, "Extreme Peril");
    g_state.expeditions[4].riskLevel = 4;
    g_state.expeditions[4].assignedScout[0] = '\0';
    g_state.expeditions[4].potentialFood = 22;
    g_state.expeditions[4].potentialScrap = 75;
    g_state.expeditions[4].potentialMeds = 8;
    strcpy(g_state.expeditions[4].blueprintId, "bp_genevault");
    strcpy(g_state.expeditions[4].blueprintReward, "Hydroponic Gene-Vault");
    g_state.expeditions[4].hasStimpack = 0;

    // Phase 10: Environmental Disasters & Weather
    g_state.weatherType = 0;
    g_state.weatherDaysLeft = 0;
    g_state.weatherSeverity = 1;
    g_state.forecastType = 1;
    g_state.forecastEtaDays = 3;
    g_state.forecastSeverity = 2;
    g_state.cmRadBulkhead = 0;
    g_state.cmThermalOverdrive = 0;
    g_state.cmAcidNeutralizerDays = 0;

    // Phase 11: Wasteland Caravan Trading System
    g_state.caravanPresent = 1;
    g_state.caravanFaction = 0;
    g_state.caravanDaysLeft = 2;
    g_state.caravanEtaDays = 0;
    g_state.caravanRepLevel = 1;
    g_state.caravanTradesCount = 0;
    for (int k = 0; k < 5; k++) g_state.caravanRareBought[k] = 0;

    g_state.logCount = 0;
    AddLog("Vault 704 Overseer System initialized. All security bulkheads sealed.", 3);
    AddLog("Life support and biometric sensors operating on standby power.", 0);
}

// Drawing Helpers
static void FillSolidRect(HDC hdc, int x, int y, int w, int h, COLORREF color) {
    RECT rc = { x, y, x + w, y + h };
    HBRUSH br = CreateSolidBrush(color);
    FillRect(hdc, &rc, br);
    DeleteObject(br);
}

static void DrawBoxBorder(HDC hdc, int x, int y, int w, int h, COLORREF color) {
    RECT rc = { x, y, x + w, y + h };
    HBRUSH br = CreateSolidBrush(color);
    FrameRect(hdc, &rc, br);
    DeleteObject(br);
}

static void DrawStyledBox(HDC hdc, int x, int y, int w, int h, COLORREF bgCol, COLORREF borderCol) {
    FillSolidRect(hdc, x, y, w, h, bgCol);
    DrawBoxBorder(hdc, x, y, w, h, borderCol);

    // Distressed retro-terminal corner brackets
    if (w >= 18 && h >= 18) {
        COLORREF cornerCol = COL_BORDER_HI;
        HPEN hPen = CreatePen(PS_SOLID, 1, cornerCol);
        HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
        int blen = (w > 60 && h > 35) ? 5 : 3;

        // Top-left corner bracket
        MoveToEx(hdc, x, y + blen, NULL);
        LineTo(hdc, x, y);
        LineTo(hdc, x + blen + 1, y);

        // Top-right corner bracket
        MoveToEx(hdc, x + w - 1 - blen, y, NULL);
        LineTo(hdc, x + w - 1, y);
        LineTo(hdc, x + w - 1, y + blen + 1);

        // Bottom-left corner bracket
        MoveToEx(hdc, x, y + h - 1 - blen, NULL);
        LineTo(hdc, x, y + h - 1);
        LineTo(hdc, x + blen + 1, y + h - 1);

        // Bottom-right corner bracket
        MoveToEx(hdc, x + w - 1 - blen, y + h - 1, NULL);
        LineTo(hdc, x + w - 1, y + h - 1);
        LineTo(hdc, x + w - 1, y + h - 1 - blen);

        SelectObject(hdc, oldPen);
        DeleteObject(hPen);
    }
}

static void DrawButtonControl(HDC hdc, HFONT hFont, int x, int y, int w, int h, const char* label, COLORREF textCol, COLORREF bgCol, COLORREF borderCol, int btnId, int p1, int p2) {
    DrawStyledBox(hdc, x, y, w, h, bgCol, borderCol);
    SelectObject(hdc, hFont);
    SetTextColor(hdc, textCol);
    SetBkMode(hdc, TRANSPARENT);
    RECT rc = { x, y, x + w, y + h };
    DrawTextA(hdc, label, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    AddButton(x, y, w, h, btnId, p1, p2);
}

static void DrawProgressBar(HDC hdc, int x, int y, int w, int h, float pct, COLORREF fillCol) {
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 1.0f) pct = 1.0f;
    DrawStyledBox(hdc, x, y, w, h, COL_BAR_BG, COL_BORDER);
    int fillW = (int)((w - 2) * pct);
    if (fillW > 0) {
        FillSolidRect(hdc, x + 1, y + 1, fillW, h - 2, fillCol);
    }
}

// GUI Rendering
static void DrawHUD(HDC hdc, HFONT hFontBold, HFONT hFontSmall, int startY) {
    float fProd, fNeed, wProd, wNeed, sProd;
    int pGen, pLoad;
    CalculateTotals(&fProd, &fNeed, &wProd, &wNeed, &pGen, &pLoad, &sProd);
    g_state.powerGen = pGen;
    g_state.powerLoad = pLoad;

    int cardW = 120;
    int cardH = 58;
    int spacing = 5;
    int curX = 10;

    int idleCount = GetUnassignedCount();
    int sickCount = 0;
    for (int i = 0; i < g_state.numSurvivors; i++) {
        if (g_state.survivors[i].health < 50) sickCount++;
    }

    // 1. Population
    DrawStyledBox(hdc, curX, startY, cardW, cardH, COL_PANEL_BG, COL_BORDER);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, curX + 6, startY + 4, "POPULATION", 10);
    char capBuf[16];
    sprintf(capBuf, "%d/%d", g_state.population, g_state.maxPop);
    TextOutA(hdc, curX + cardW - 35, startY + 4, capBuf, (int)strlen(capBuf));
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    char popVal[16];
    sprintf(popVal, "%d", g_state.population);
    TextOutA(hdc, curX + 6, startY + 18, popVal, (int)strlen(popVal));
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, idleCount > 0 ? COL_GREEN : COL_TEXT_DIM);
    char idleBuf[20];
    sprintf(idleBuf, "(%d Idle)", idleCount);
    TextOutA(hdc, curX + 32, startY + 22, idleBuf, (int)strlen(idleBuf));
    SetTextColor(hdc, COL_TEXT_DIM);
    char healthBuf[32];
    sprintf(healthBuf, "Hlth: %d | Sick: %d", g_state.population - sickCount, sickCount);
    TextOutA(hdc, curX + 6, startY + 40, healthBuf, (int)strlen(healthBuf));
    curX += cardW + spacing;

    // 2. Food Rations
    DrawStyledBox(hdc, curX, startY, cardW, cardH, COL_PANEL_BG, COL_BORDER);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, curX + 6, startY + 4, "FOOD", 4);
    int daysFood = (fNeed > 0.0f) ? (int)(g_state.food / fNeed) : 99;
    char fDaysBuf[16];
    sprintf(fDaysBuf, "%dd left", daysFood);
    TextOutA(hdc, curX + cardW - 48, startY + 4, fDaysBuf, (int)strlen(fDaysBuf));
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    char foodVal[16];
    sprintf(foodVal, "%d", (int)g_state.food);
    TextOutA(hdc, curX + 6, startY + 18, foodVal, (int)strlen(foodVal));
    SelectObject(hdc, hFontSmall);
    float netF = fProd - fNeed;
    SetTextColor(hdc, netF >= 0 ? COL_GREEN : COL_RED);
    char netFBuf[20];
    sprintf(netFBuf, "%s%.1f/d", netF >= 0 ? "+" : "", netF);
    TextOutA(hdc, curX + 44, startY + 22, netFBuf, (int)strlen(netFBuf));
    SetTextColor(hdc, COL_TEXT_DIM);
    char fProdBuf[32];
    sprintf(fProdBuf, "+%.0f / -%.1f req", fProd, fNeed);
    TextOutA(hdc, curX + 6, startY + 40, fProdBuf, (int)strlen(fProdBuf));
    curX += cardW + spacing;

    // 3. Water Purity
    DrawStyledBox(hdc, curX, startY, cardW, cardH, COL_PANEL_BG, COL_BORDER);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, curX + 6, startY + 4, "WATER", 5);
    int daysWater = (wNeed > 0.0f) ? (int)(g_state.water / wNeed) : 99;
    char wDaysBuf[16];
    sprintf(wDaysBuf, "%dd left", daysWater);
    TextOutA(hdc, curX + cardW - 48, startY + 4, wDaysBuf, (int)strlen(wDaysBuf));
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    char waterVal[16];
    sprintf(waterVal, "%d", (int)g_state.water);
    TextOutA(hdc, curX + 6, startY + 18, waterVal, (int)strlen(waterVal));
    SelectObject(hdc, hFontSmall);
    float netW = wProd - wNeed;
    SetTextColor(hdc, netW >= 0 ? COL_GREEN : COL_RED);
    char netWBuf[20];
    sprintf(netWBuf, "%s%.1f/d", netW >= 0 ? "+" : "", netW);
    TextOutA(hdc, curX + 44, startY + 22, netWBuf, (int)strlen(netWBuf));
    SetTextColor(hdc, COL_TEXT_DIM);
    char wProdBuf[32];
    sprintf(wProdBuf, "+%.0f / -%.1f req", wProd, wNeed);
    TextOutA(hdc, curX + 6, startY + 40, wProdBuf, (int)strlen(wProdBuf));
    curX += cardW + spacing;

    // 4. Power Grid
    DrawStyledBox(hdc, curX, startY, cardW, cardH, COL_PANEL_BG, COL_BORDER);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, curX + 6, startY + 4, "POWER GRID", 10);
    int loadPct = (pGen > 0) ? (pLoad * 100 / pGen) : 100;
    char pLoadBuf[16];
    sprintf(pLoadBuf, "%d%% Load", loadPct);
    SetTextColor(hdc, loadPct > 100 ? COL_RED : (loadPct > 85 ? COL_AMBER : COL_TEXT_DIM));
    TextOutA(hdc, curX + cardW - 55, startY + 4, pLoadBuf, (int)strlen(pLoadBuf));
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    char pwrVal[16];
    sprintf(pwrVal, "%d", pGen);
    TextOutA(hdc, curX + 6, startY + 18, pwrVal, (int)strlen(pwrVal));
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, curX + 38, startY + 22, "kW Gen", 6);
    char pSubBuf[32];
    sprintf(pSubBuf, "Demand: %d kW", pLoad);
    TextOutA(hdc, curX + 6, startY + 40, pSubBuf, (int)strlen(pSubBuf));
    curX += cardW + spacing;

    // 5. Scrap
    DrawStyledBox(hdc, curX, startY, cardW, cardH, COL_PANEL_BG, COL_BORDER);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, curX + 6, startY + 4, "SCRAP", 5);
    TextOutA(hdc, curX + cardW - 42, startY + 4, "STOCK", 5);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    char scrapVal[16];
    sprintf(scrapVal, "%d", (int)g_state.scrap);
    TextOutA(hdc, curX + 6, startY + 18, scrapVal, (int)strlen(scrapVal));
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_GREEN);
    char sProdBuf[16];
    sprintf(sProdBuf, "+%.0f/d", sProd);
    TextOutA(hdc, curX + 44, startY + 22, sProdBuf, (int)strlen(sProdBuf));
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, curX + 6, startY + 40, "Salvage parts", 13);
    curX += cardW + spacing;

    // 6. Meds / Stims
    DrawStyledBox(hdc, curX, startY, cardW, cardH, COL_PANEL_BG, COL_BORDER);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, curX + 6, startY + 4, "MEDS/STIMS", 10);
    const char* medStat = (g_state.meds <= 0) ? "NONE" : ((g_state.meds < 3) ? "LOW" : "STOCKED");
    SetTextColor(hdc, (g_state.meds <= 0) ? COL_RED : ((g_state.meds < 3) ? COL_AMBER : COL_GREEN));
    TextOutA(hdc, curX + cardW - 50, startY + 4, medStat, (int)strlen(medStat));
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    char medVal[16];
    sprintf(medVal, "%d", g_state.meds);
    TextOutA(hdc, curX + 6, startY + 18, medVal, (int)strlen(medVal));
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, curX + 30, startY + 22, "packs", 5);
    TextOutA(hdc, curX + 6, startY + 40, "Triage & Scavenge", 17);
    curX += cardW + spacing;

    // 7. Morale
    DrawStyledBox(hdc, curX, startY, cardW, cardH, COL_PANEL_BG, COL_BORDER);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, curX + 6, startY + 4, "MORALE", 6);
    const char* mLabel = "CONTENT";
    if (g_state.morale >= 90.0f) mLabel = "THRIVING";
    else if (g_state.morale < 45.0f) mLabel = "MUTINOUS";
    else if (g_state.morale < 70.0f) mLabel = "ANXIOUS";
    SetTextColor(hdc, g_state.morale >= 80.0f ? COL_GREEN : (g_state.morale < 50.0f ? COL_RED : COL_AMBER));
    TextOutA(hdc, curX + cardW - 55, startY + 4, mLabel, (int)strlen(mLabel));
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    char morVal[16];
    sprintf(morVal, "%d%%", (int)g_state.morale);
    TextOutA(hdc, curX + 6, startY + 18, morVal, (int)strlen(morVal));
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, curX + 6, startY + 40, "Vault stability", 15);
    curX += cardW + spacing;

    // 8. Defense & Rads
    DrawStyledBox(hdc, curX, startY, cardW, cardH, COL_PANEL_BG, COL_BORDER);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, curX + 6, startY + 4, "DEFENSE", 7);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    char defVal[16];
    sprintf(defVal, "%d pts", g_state.defense);
    TextOutA(hdc, curX + 6, startY + 18, defVal, (int)strlen(defVal));
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_AMBER);
    char radBuf[24];
    sprintf(radBuf, "Ext: %.1f R/h", g_state.exteriorRads);
    TextOutA(hdc, curX + 6, startY + 40, radBuf, (int)strlen(radBuf));
}

static void DrawFacilitiesView(HDC hdc, HFONT hFontBold, HFONT hFontSmall, int x, int y, int w, int h) {
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x, y, "SHELTER FACILITIES & ROOM ENGINEERING", 37);
    
    int idle = GetUnassignedCount();
    char idleStr[40];
    sprintf(idleStr, "Unassigned Workers: %d", idle);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, idle > 0 ? COL_GREEN : COL_TEXT_DIM);
    TextOutA(hdc, x + w - 160, y, idleStr, (int)strlen(idleStr));

    // Sub-tab Bar
    int subTabY = y + 20;
    int subTabW = 180;
    char tab1[48], tab2[48];
    sprintf(tab1, "ACTIVE FACILITIES (%d)", g_state.numFacilities);
    int availBp = 0;
    for (int b = 0; b < g_state.numBlueprints; b++) {
        if (!g_state.blueprints[b].built && !g_state.blueprints[b].locked) availBp++;
    }
    sprintf(tab2, "CONSTRUCT ROOMS (%d AVAIL)", availBp);

    COLORREF t1Bg = (g_state.facilitySubTab == 0) ? COL_BTN_HOVER : COL_DARK_CARD;
    COLORREF t1Txt = (g_state.facilitySubTab == 0) ? COL_TEXT_BRIGHT : COL_TEXT_DIM;
    COLORREF t1Bdr = (g_state.facilitySubTab == 0) ? COL_BORDER_HI : COL_BORDER;
    DrawButtonControl(hdc, hFontSmall, x, subTabY, subTabW, 22, tab1, t1Txt, t1Bg, t1Bdr, BTN_FAC_SUBTAB, 0, 0);

    COLORREF t2Bg = (g_state.facilitySubTab == 1) ? COL_BTN_HOVER : COL_DARK_CARD;
    COLORREF t2Txt = (g_state.facilitySubTab == 1) ? COL_TEXT_BRIGHT : COL_TEXT_DIM;
    COLORREF t2Bdr = (g_state.facilitySubTab == 1) ? COL_BORDER_HI : COL_BORDER;
    DrawButtonControl(hdc, hFontSmall, x + subTabW + 8, subTabY, subTabW + 20, 22, tab2, t2Txt, t2Bg, t2Bdr, BTN_FAC_SUBTAB, 1, 0);

    int startY = y + 48;
    int isBlackout = (g_state.powerGen < g_state.powerLoad);

    if (g_state.facilitySubTab == 0) {
        // Active Facilities Grid (2 Columns)
        int colW = (w - 10) / 2;
        int cardH = 72;
        int gapY = 6;

        for (int i = 0; i < g_state.numFacilities; i++) {
            Facility* fac = &g_state.facilities[i];
            int col = i % 2;
            int row = i / 2;
            int cx = x + col * (colW + 10);
            int cy = startY + row * (cardH + gapY);

            DrawStyledBox(hdc, cx, cy, colW, cardH, COL_DARK_CARD, COL_BORDER);

            // Row 1: Name + Level + Status
            SelectObject(hdc, hFontBold);
            SetTextColor(hdc, COL_TEXT_BRIGHT);
            TextOutA(hdc, cx + 6, cy + 4, fac->name, (int)strlen(fac->name));

            char lvlBuf[12];
            sprintf(lvlBuf, "LV%d", fac->level);
            SelectObject(hdc, hFontSmall);
            SetTextColor(hdc, COL_AMBER);
            TextOutA(hdc, cx + 165, cy + 5, lvlBuf, (int)strlen(lvlBuf));

            const char* stText = "ONLINE";
            COLORREF stColor = COL_GREEN;
            if (fac->powerProd > 0) {
                stText = "GEN";
                stColor = COL_GREEN;
            } else if (isBlackout && fac->powerPriority > 2) {
                stText = "BROWNOUT";
                stColor = COL_RED;
            } else if (fac->maxWorkers > 0 && fac->assigned == 0) {
                stText = "UNSTAFFED";
                stColor = COL_AMBER;
            }
            SetTextColor(hdc, stColor);
            TextOutA(hdc, cx + colW - 65, cy + 5, stText, (int)strlen(stText));

            // Row 2: Stats output line with efficiency calculation
            int assignedCount = 0;
            float effSum = 0.0f;
            char staffNames[64] = "";
            for (int s = 0; s < g_state.numSurvivors; s++) {
                if (strcmp(g_state.survivors[s].job, fac->id) == 0) {
                    assignedCount++;
                    float weff = GetWorkerEfficiency(&g_state.survivors[s], fac);
                    effSum += weff;
                    if (strlen(staffNames) < 32) {
                        char namePart[16];
                        sscanf(g_state.survivors[s].name, "%15s", namePart);
                        if (strlen(staffNames) > 0) strcat(staffNames, ", ");
                        char ebuf[24];
                        sprintf(ebuf, "%s(%d%%)", namePart, (int)(weff * 100));
                        strcat(staffNames, ebuf);
                    }
                }
            }

            char outBuf[64];
            if (fac->powerProd > 0) {
                int outP = (assignedCount > 0 ? (int)(fac->powerProd + effSum * 8.0f) : 6);
                sprintf(outBuf, "Out: +%d kW Power | Self-Sufficient", outP);
            } else if (fac->foodProd > 0) {
                sprintf(outBuf, "Out: +%d Food/cyc | Draw: -%d kW", (int)(fac->foodProd * effSum), fac->powerCost);
            } else if (fac->waterProd > 0) {
                sprintf(outBuf, "Out: +%d Water/cyc | Draw: -%d kW", (int)(fac->waterProd * effSum), fac->powerCost);
            } else if (fac->scrapProd > 0) {
                sprintf(outBuf, "Out: +%d Scrap/cyc | Draw: -%d kW", (int)(fac->scrapProd * effSum), fac->powerCost);
            } else if (strcmp(fac->id, "quarters") == 0 || strcmp(fac->id, "quarters_ext") == 0) {
                sprintf(outBuf, "Cap: +%d Dwellers | Draw: -%d kW", 10 + (fac->level - 1) * 4, fac->powerCost);
            } else if (strcmp(fac->id, "security") == 0) {
                sprintf(outBuf, "Defense: +%d Armor | Draw: -%d kW", 15 + (fac->level - 1) * 10 + assignedCount * 8, fac->powerCost);
            } else {
                sprintf(outBuf, "Medical Triage (%d Staff) | -%d kW", assignedCount, fac->powerCost);
            }
            SetTextColor(hdc, COL_TEXT_MAIN);
            TextOutA(hdc, cx + 6, cy + 19, outBuf, (int)strlen(outBuf));

            // Row 3: Staff names
            if (strlen(staffNames) > 0) {
                char sBuf[80];
                sprintf(sBuf, "Staff: %s", staffNames);
                SetTextColor(hdc, COL_TEXT_DIM);
                TextOutA(hdc, cx + 6, cy + 34, sBuf, (int)strlen(sBuf));
            } else if (fac->maxWorkers > 0) {
                SetTextColor(hdc, COL_TEXT_DIM);
                TextOutA(hdc, cx + 6, cy + 34, "Staff: None (Assign idle survivor below)", 40);
            } else {
                SetTextColor(hdc, COL_TEXT_DIM);
                TextOutA(hdc, cx + 6, cy + 34, "Automated Life Support Chamber", 30);
            }

            // Row 4: Staff [-] [+] controls and Upgrade Button
            if (fac->maxWorkers > 0) {
                char staffBuf[20];
                sprintf(staffBuf, "%d/%d Workers", fac->assigned, fac->maxWorkers);
                SetTextColor(hdc, COL_TEXT_DIM);
                TextOutA(hdc, cx + 6, cy + 50, staffBuf, (int)strlen(staffBuf));

                // [-] button
                COLORREF btnMinusBg = (fac->assigned > 0) ? COL_BTN_BG : COL_DARK_CARD;
                COLORREF btnMinusTxt = (fac->assigned > 0) ? COL_TEXT_MAIN : COL_TEXT_DIM;
                DrawButtonControl(hdc, hFontBold, cx + 75, cy + 48, 18, 19, "-", btnMinusTxt, btnMinusBg, COL_BORDER, BTN_FAC_WORKER, i, -1);

                // [+] button
                COLORREF btnPlusBg = (fac->assigned < fac->maxWorkers && idle > 0) ? COL_BTN_BG : COL_DARK_CARD;
                COLORREF btnPlusTxt = (fac->assigned < fac->maxWorkers && idle > 0) ? COL_TEXT_BRIGHT : COL_TEXT_DIM;
                DrawButtonControl(hdc, hFontBold, cx + 96, cy + 48, 18, 19, "+", btnPlusTxt, btnPlusBg, COL_BORDER, BTN_FAC_WORKER, i, 1);
            }

            // Upgrade Button
            if (fac->level < 3) {
                int uCost = fac->level * 30;
                char uBuf[24];
                sprintf(uBuf, "UPG LV%d (%dS)", fac->level + 1, uCost);
                COLORREF uBg = (g_state.scrap >= uCost) ? COL_BTN_BG : COL_DARK_CARD;
                COLORREF uTxt = (g_state.scrap >= uCost) ? COL_AMBER : COL_TEXT_DIM;
                DrawButtonControl(hdc, hFontSmall, cx + colW - 105, cy + 48, 98, 19, uBuf, uTxt, uBg, COL_BORDER, BTN_FAC_UPGRADE, i, 0);
            } else {
                SetTextColor(hdc, COL_TEXT_DIM);
                RECT rcMax = { cx + colW - 105, cy + 48, cx + colW - 6, cy + 67 };
                DrawTextA(hdc, "[MAX LEVEL]", -1, &rcMax, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }
    } else {
        // Room Construction Blueprints (2 Columns, 7 Rows)
        int colW = (w - 10) / 2;
        int cardH = 62;
        int gapY = 4;

        for (int i = 0; i < g_state.numBlueprints; i++) {
            RoomBlueprint* bp = &g_state.blueprints[i];
            int col = i % 2;
            int row = i / 2;
            int cx = x + col * (colW + 10);
            int cy = startY + row * (cardH + gapY);

            DrawStyledBox(hdc, cx, cy, colW, cardH, COL_DARK_CARD, COL_BORDER);

            int cost = GetEffectiveScrapCost(bp->cost);

            // Name + Cost / Status
            SelectObject(hdc, hFontBold);
            SetTextColor(hdc, COL_TEXT_BRIGHT);
            TextOutA(hdc, cx + 6, cy + 4, bp->name, (int)strlen(bp->name));

            SelectObject(hdc, hFontSmall);
            if (bp->built) {
                SetTextColor(hdc, COL_GREEN);
                TextOutA(hdc, cx + colW - 55, cy + 4, "BUILT", 5);
            } else if (bp->locked) {
                SetTextColor(hdc, COL_AMBER);
                TextOutA(hdc, cx + colW - 95, cy + 4, "LOCKED", 6);
            } else {
                char costBuf[16];
                sprintf(costBuf, "%d SCRAP", cost);
                SetTextColor(hdc, COL_AMBER);
                TextOutA(hdc, cx + colW - 68, cy + 4, costBuf, (int)strlen(costBuf));
            }

            // Benefit
            SetTextColor(hdc, COL_GREEN);
            char benBuf[64];
            sprintf(benBuf, "Benefit: %s", bp->benefit);
            TextOutA(hdc, cx + 6, cy + 20, benBuf, (int)strlen(benBuf));

            // Action button
            if (bp->built) {
                SetTextColor(hdc, COL_GREEN);
                RECT rcOp = { cx + colW - 180, cy + 38, cx + colW - 8, cy + 58 };
                DrawTextA(hdc, "[ OPERATIONAL & ONLINE ]", -1, &rcOp, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
            } else if (bp->locked) {
                SetTextColor(hdc, COL_AMBER);
                RECT rcLk = { cx + colW - 240, cy + 38, cx + colW - 8, cy + 58 };
                char lkBuf[64];
                if (strlen(bp->reqTechName) > 0) {
                    sprintf(lkBuf, "[ REQ TECH: %s ]", bp->reqTechName);
                } else {
                    sprintf(lkBuf, "[ DISCOVER IN: %s ]", bp->discoverSource);
                }
                DrawTextA(hdc, lkBuf, -1, &rcLk, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
            } else {
                char bBuf[32];
                sprintf(bBuf, "CONSTRUCT (%d SCRAP)", cost);
                int canAfford = (g_state.scrap >= (float)cost);
                COLORREF cBg = canAfford ? RGB(25, 50, 30) : COL_DARK_CARD;
                COLORREF cTxt = canAfford ? COL_TEXT_BRIGHT : COL_TEXT_DIM;
                COLORREF cBdr = canAfford ? COL_GREEN : COL_BORDER;
                DrawButtonControl(hdc, hFontBold, cx + colW - 185, cy + 38, 178, 20, bBuf, cTxt, cBg, cBdr, BTN_CONSTRUCT_ROOM, i, 0);
            }
        }
    }
}

static void DrawSurvivorsView(HDC hdc, HFONT hFontBold, HFONT hFontSmall, int x, int y, int w, int h) {
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x, y, "SHELTER CITIZENS & RECRUITMENT BEACON", 37);

    char capBuf[32];
    sprintf(capBuf, "Capacity: %d/%d Dwellers", g_state.population, g_state.maxPop);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x + w - 170, y, capBuf, (int)strlen(capBuf));

    // Sub-tab buttons
    int subTabY = y + 20;
    int subTabW = 180;
    char tab1[48], tab2[48];
    sprintf(tab1, "ACTIVE CITIZENS (%d)", g_state.numSurvivors);
    sprintf(tab2, "RADIO BEACON & AIRLOCK (%d)", g_state.numCandidates);

    COLORREF t1Bg = (g_state.survivorSubTab == 0) ? COL_BTN_HOVER : COL_DARK_CARD;
    COLORREF t1Txt = (g_state.survivorSubTab == 0) ? COL_TEXT_BRIGHT : COL_TEXT_DIM;
    COLORREF t1Bdr = (g_state.survivorSubTab == 0) ? COL_BORDER_HI : COL_BORDER;
    DrawButtonControl(hdc, hFontSmall, x, subTabY, subTabW, 22, tab1, t1Txt, t1Bg, t1Bdr, BTN_SURV_SUBTAB, 0, 0);

    COLORREF t2Bg = (g_state.survivorSubTab == 1) ? COL_BTN_HOVER : COL_DARK_CARD;
    COLORREF t2Txt = (g_state.survivorSubTab == 1) ? COL_TEXT_BRIGHT : COL_TEXT_DIM;
    COLORREF t2Bdr = (g_state.survivorSubTab == 1) ? COL_BORDER_HI : COL_BORDER;
    DrawButtonControl(hdc, hFontSmall, x + subTabW + 8, subTabY, subTabW + 40, 22, tab2, t2Txt, t2Bg, t2Bdr, BTN_SURV_SUBTAB, 1, 0);

    int startY = y + 48;

    if (g_state.survivorSubTab == 0) {
        // ACTIVE CITIZENS LIST
        int cardH = 64;
        int gap = 6;

        for (int i = 0; i < g_state.numSurvivors; i++) {
            Survivor* s = &g_state.survivors[i];
            int cy = startY + i * (cardH + gap);
            if (cy + cardH > y + h + 20) break;

            DrawStyledBox(hdc, x, cy, w, cardH, COL_DARK_CARD, COL_BORDER);

            // Name & Role & Mood Badge
            SelectObject(hdc, hFontBold);
            SetTextColor(hdc, COL_TEXT_BRIGHT);
            TextOutA(hdc, x + 8, cy + 5, s->name, (int)strlen(s->name));

            const char* moodStr = (s->morale >= 85) ? "[ECSTATIC]" : ((s->morale >= 65) ? "[CONTENT]" : ((s->morale >= 45) ? "[DISCONTENT]" : ((s->morale >= 25) ? "[UNREST]" : "[MUTINOUS]")));
            COLORREF moodCol = (s->morale >= 65) ? COL_GREEN : ((s->morale >= 45) ? COL_AMBER : COL_RED);
            SelectObject(hdc, hFontSmall);
            SetTextColor(hdc, moodCol);
            TextOutA(hdc, x + 125, cy + 5, moodStr, (int)strlen(moodStr));

            char roleBuf[64];
            sprintf(roleBuf, "%s | STR:%d AGI:%d INT:%d", s->role, s->str, s->agi, s->inte);
            SetTextColor(hdc, COL_TEXT_DIM);
            TextOutA(hdc, x + 8, cy + 22, roleBuf, (int)strlen(roleBuf));

            // Column 2: Health, Morale, Radiation Meters
            // Health Bar
            TextOutA(hdc, x + 200, cy + 4, "HP", 2);
            char hpVal[8];
            sprintf(hpVal, "%d%%", s->health);
            TextOutA(hdc, x + 265, cy + 4, hpVal, (int)strlen(hpVal));
            COLORREF hpColor = s->health < 40 ? COL_RED : (s->health < 75 ? COL_AMBER : COL_GREEN);
            DrawProgressBar(hdc, x + 200, cy + 15, 85, 6, s->health / 100.0f, hpColor);

            // Morale Bar
            TextOutA(hdc, x + 200, cy + 23, "MOR", 3);
            char morVal[8];
            sprintf(morVal, "%d%%", s->morale);
            TextOutA(hdc, x + 265, cy + 23, morVal, (int)strlen(morVal));
            DrawProgressBar(hdc, x + 200, cy + 34, 85, 6, s->morale / 100.0f, COL_AMBER);

            // Radiation Bar
            TextOutA(hdc, x + 200, cy + 42, "RAD", 3);
            char radVal[16];
            sprintf(radVal, "%dR", s->rads);
            COLORREF radColor = (s->rads >= 75) ? COL_RED : ((s->rads >= 50) ? RGB(255, 140, 0) : ((s->rads >= 25) ? COL_AMBER : COL_GREEN));
            SetTextColor(hdc, radColor);
            TextOutA(hdc, x + 265, cy + 42, radVal, (int)strlen(radVal));
            DrawProgressBar(hdc, x + 200, cy + 53, 85, 6, s->rads / 100.0f, radColor);

            // Column 3: Current Job & Action Buttons
            char jobLabel[48];
            if (strcmp(s->job, "unassigned") == 0 || strlen(s->job) == 0) {
                strcpy(jobLabel, "[ Idle / Unassigned ]");
            } else if (strcmp(s->job, "expedition") == 0) {
                strcpy(jobLabel, "[ On Expedition ]");
            } else {
                Facility* fac = NULL;
                for (int f = 0; f < g_state.numFacilities; f++) {
                    if (strcmp(g_state.facilities[f].id, s->job) == 0) {
                        fac = &g_state.facilities[f];
                        break;
                    }
                }
                if (fac) {
                    float eff = GetWorkerEfficiency(s, fac);
                    sprintf(jobLabel, "[ %s (%d%%) ]", fac->name, (int)(eff * 100));
                } else {
                    strcpy(jobLabel, "[ Assigned ]");
                }
            }

            if (strcmp(s->job, "expedition") == 0) {
                DrawStyledBox(hdc, x + w - 215, cy + 8, 210, 24, COL_DARK_CARD, COL_BORDER);
                SelectObject(hdc, hFontSmall);
                SetTextColor(hdc, COL_AMBER);
                RECT rc = { x + w - 215, cy + 8, x + w - 5, cy + 32 };
                DrawTextA(hdc, "ON WASTELAND EXPEDITION", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            } else {
                DrawButtonControl(hdc, hFontSmall, x + w - 215, cy + 6, 210, 24, jobLabel, COL_TEXT_MAIN, COL_BTN_BG, COL_BORDER, BTN_SURV_JOB, i, 0);

                // Row of Action buttons: HEAL (-1M) | RAD-AWAY (-1M) | DECON (-5W)
                int actW = 68;
                int actGap = 3;
                int actX = x + w - 215;

                // HEAL button
                int canHeal = (s->health < 100 && g_state.meds > 0);
                DrawButtonControl(hdc, hFontSmall, actX, cy + 34, actW, 24, "HEAL (-1M)", canHeal ? COL_GREEN : COL_TEXT_DIM, canHeal ? RGB(25, 45, 30) : COL_DARK_CARD, canHeal ? COL_GREEN : COL_BORDER, BTN_TREAT_SURV, i, 0);

                // RAD-AWAY button
                int canRadAway = (s->rads > 0 && g_state.meds > 0);
                DrawButtonControl(hdc, hFontSmall, actX + actW + actGap, cy + 34, actW, 24, "RAD-AWAY", canRadAway ? COL_AMBER : COL_TEXT_DIM, canRadAway ? RGB(45, 40, 20) : COL_DARK_CARD, canRadAway ? COL_AMBER : COL_BORDER, BTN_TREAT_RADAWAY, i, 0);

                // DECON SHOWER button
                int canDecon = (s->rads > 10 && g_state.water >= 5.0f && g_state.scrap >= 5.0f);
                DrawButtonControl(hdc, hFontSmall, actX + (actW + actGap) * 2, cy + 34, actW, 24, "DECON", canDecon ? RGB(100, 200, 255) : COL_TEXT_DIM, canDecon ? RGB(20, 35, 45) : COL_DARK_CARD, canDecon ? RGB(100, 200, 255) : COL_BORDER, BTN_TREAT_DECON, i, 0);
            }
        }
    } else {
        // RADIO BEACON & AIRLOCK TERMINAL
        // Top Broadcast Box
        int boxH = 68;
        DrawStyledBox(hdc, x, startY, w, boxH, COL_DARK_CARD, COL_BORDER);

        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, COL_TEXT_BRIGHT);
        TextOutA(hdc, x + 8, startY + 6, "EMERGENCY BROADCAST TRANSMITTER // FREQ 104.7 MHz", 49);

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COL_GREEN);
        TextOutA(hdc, x + w - 120, startY + 6, "ANTENNA ONLINE", 14);

        SetTextColor(hdc, COL_TEXT_DIM);
        TextOutA(hdc, x + 8, startY + 22, "Pulse radio frequencies across the wastes to guide refugees and specialists to Vault 704's outer airlock.", 105);

        // Broadcast Buttons
        int canPing = (g_state.scrap >= 15 && g_state.powerGen >= 10);
        COLORREF pingBg = canPing ? RGB(25, 45, 30) : COL_DARK_CARD;
        COLORREF pingTxt = canPing ? COL_TEXT_BRIGHT : COL_TEXT_DIM;
        COLORREF pingBdr = canPing ? COL_GREEN : COL_BORDER;
        DrawButtonControl(hdc, hFontBold, x + 8, startY + 38, 260, 22, "TRANSMIT PING (15 Scrap, 10 kW)", pingTxt, pingBg, pingBdr, BTN_BROADCAST_PING, 0, 0);

        int canSpec = (g_state.scrap >= 25 && g_state.food >= 10.0f);
        COLORREF specBg = canSpec ? RGB(45, 35, 15) : COL_DARK_CARD;
        COLORREF specTxt = canSpec ? COL_AMBER : COL_TEXT_DIM;
        COLORREF specBdr = canSpec ? COL_AMBER : COL_BORDER;
        DrawButtonControl(hdc, hFontBold, x + 276, startY + 38, 280, 22, "SPECIALIST BEACON (25 Scrap, 10 Food)", specTxt, specBg, specBdr, BTN_BROADCAST_SPEC, 0, 0);

        // Candidates List Header
        int candStartY = startY + boxH + 8;
        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, COL_TEXT_BRIGHT);
        TextOutA(hdc, x, candStartY, "PENDING AIRLOCK CANDIDATES & REFUGEES", 37);

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COL_TEXT_DIM);
        char clrBuf[48];
        sprintf(clrBuf, "Waiting at Blast Door: %d", g_state.numCandidates);
        TextOutA(hdc, x + w - 170, candStartY, clrBuf, (int)strlen(clrBuf));

        int cardY = candStartY + 18;
        int cardH = 62;
        int gap = 6;

        if (g_state.numCandidates == 0) {
            DrawStyledBox(hdc, x, cardY, w, 60, COL_DARK_CARD, COL_BORDER);
            SelectObject(hdc, hFontSmall);
            SetTextColor(hdc, COL_TEXT_DIM);
            RECT rcEmpty = { x, cardY, x + w, cardY + 60 };
            DrawTextA(hdc, "No wasteland refugees currently at the airlock.\nUse the Emergency Broadcast Transmitter above to pulse radio signals across the wasteland.", -1, &rcEmpty, DT_CENTER | DT_VCENTER);
        } else {
            for (int i = 0; i < g_state.numCandidates; i++) {
                Candidate* c = &g_state.candidates[i];
                int cy = cardY + i * (cardH + gap);
                if (cy + cardH > y + h + 20) break;

                DrawStyledBox(hdc, x, cy, w, cardH, COL_DARK_CARD, COL_BORDER);

                // Candidate Name & Role
                SelectObject(hdc, hFontBold);
                SetTextColor(hdc, COL_TEXT_BRIGHT);
                TextOutA(hdc, x + 8, cy + 6, c->name, (int)strlen(c->name));

                SelectObject(hdc, hFontSmall);
                SetTextColor(hdc, COL_AMBER);
                TextOutA(hdc, x + 180, cy + 7, c->role, (int)strlen(c->role));

                // Trait
                char trBuf[64];
                sprintf(trBuf, "Trait: %s", c->trait);
                SetTextColor(hdc, COL_TEXT_MAIN);
                TextOutA(hdc, x + 8, cy + 24, trBuf, (int)strlen(trBuf));

                // Stats & Condition
                char statBuf[64];
                sprintf(statBuf, "STR:%d AGI:%d INT:%d | Health:%d%% Morale:%d%%", c->str, c->agi, c->inte, c->health, c->morale);
                SetTextColor(hdc, COL_TEXT_DIM);
                TextOutA(hdc, x + 8, cy + 42, statBuf, (int)strlen(statBuf));

                // Actions: Turn Away & Admit
                int canAdmit = (g_state.population < g_state.maxPop && g_state.numSurvivors < MAX_SURVIVORS);

                // Turn away button
                DrawButtonControl(hdc, hFontSmall, x + w - 180, cy + 18, 80, 26, "TURN AWAY", COL_RED, COL_DARK_CARD, COL_BORDER, BTN_DISMISS_CANDIDATE, i, 0);

                // Admit button
                COLORREF adBg = canAdmit ? RGB(25, 45, 30) : COL_DARK_CARD;
                COLORREF adTxt = canAdmit ? COL_GREEN : COL_TEXT_DIM;
                COLORREF adBdr = canAdmit ? COL_GREEN : COL_BORDER;
                DrawButtonControl(hdc, hFontBold, x + w - 94, cy + 18, 90, 26, canAdmit ? "ADMIT" : "FULL", adTxt, adBg, adBdr, BTN_ADMIT_CANDIDATE, i, 0);
            }
        }
    }
}

static void DrawExpeditionsView(HDC hdc, HFONT hFontBold, HFONT hFontSmall, int x, int y, int w, int h) {
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x, y, "WASTELAND SURVEY & SCAVENGING EXPEDITIONS", 41);

    int activeCount = 0;
    for (int i = 0; i < g_state.numExpeditions; i++) {
        if (strlen(g_state.expeditions[i].assignedScout) > 0) activeCount++;
    }
    char actBuf[64];
    sprintf(actBuf, "Active: %d | Meds Reserve: %d Packs", activeCount, g_state.meds);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, activeCount > 0 ? COL_AMBER : COL_TEXT_DIM);
    TextOutA(hdc, x + w - 240, y, actBuf, (int)strlen(actBuf));

    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x, y + 18, "Scout wasteland ruins for Scrap, Food, Meds, and Lost Room Blueprints. Higher hazards yield richer rewards.", 107);

    int startY = y + 36;
    int cardH = 82;
    int gap = 6;
    int unassigned = GetUnassignedCount();

    for (int i = 0; i < g_state.numExpeditions; i++) {
        Expedition* exp = &g_state.expeditions[i];
        int cy = startY + i * (cardH + gap);

        DrawStyledBox(hdc, x, cy, w, cardH, COL_DARK_CARD, COL_BORDER);

        // Name & Hazard Badge
        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, COL_TEXT_BRIGHT);
        TextOutA(hdc, x + 8, cy + 6, exp->name, (int)strlen(exp->name));

        SelectObject(hdc, hFontSmall);
        COLORREF riskCol = (exp->riskLevel == 1) ? COL_GREEN : ((exp->riskLevel == 2) ? COL_AMBER : COL_RED);
        SetTextColor(hdc, riskCol);
        char rBuf[48];
        sprintf(rBuf, "[ %s - TIER %d ]", exp->risk, exp->riskLevel);
        TextOutA(hdc, x + 240, cy + 6, rBuf, (int)strlen(rBuf));

        SetTextColor(hdc, COL_TEXT_DIM);
        char durBuf[32];
        sprintf(durBuf, "Duration: %d Cycle(s)", exp->duration);
        TextOutA(hdc, x + 380, cy + 6, durBuf, (int)strlen(durBuf));

        // Desc
        SetTextColor(hdc, COL_TEXT_MAIN);
        TextOutA(hdc, x + 8, cy + 24, exp->desc, (int)strlen(exp->desc));

        // Yields & Blueprint
        char yldBuf[80];
        sprintf(yldBuf, "Yields: ~%d Food, ~%d Scrap, ~%d Meds", exp->potentialFood, exp->potentialScrap, exp->potentialMeds);
        SetTextColor(hdc, COL_TEXT_BRIGHT);
        TextOutA(hdc, x + 8, cy + 42, yldBuf, (int)strlen(yldBuf));

        if (strlen(exp->blueprintReward) > 0) {
            int isUnlocked = 0;
            for (int b = 0; b < g_state.numBlueprints; b++) {
                if (strcmp(g_state.blueprints[b].id, exp->blueprintId) == 0 && !g_state.blueprints[b].locked) {
                    isUnlocked = 1;
                    break;
                }
            }
            char bpBuf[64];
            if (isUnlocked) {
                sprintf(bpBuf, "[* BLUEPRINT DECRYPTED: %s]", exp->blueprintReward);
                SetTextColor(hdc, COL_GREEN);
            } else {
                sprintf(bpBuf, "[ARCHIVE TECH: %s]", exp->blueprintReward);
                SetTextColor(hdc, COL_AMBER);
            }
            TextOutA(hdc, x + 8, cy + 60, bpBuf, (int)strlen(bpBuf));
        }

        // Action / Status Controls (Right side)
        if (strlen(exp->assignedScout) > 0) {
            Survivor* scout = NULL;
            for (int s = 0; s < g_state.numSurvivors; s++) {
                if (strcmp(g_state.survivors[s].id, exp->assignedScout) == 0) {
                    scout = &g_state.survivors[s];
                    break;
                }
            }
            char progBuf[48];
            sprintf(progBuf, "EXPLORING (%dd left)", exp->daysRemaining);
            DrawStyledBox(hdc, x + w - 215, cy + 10, 205, 30, COL_PANEL_BG, COL_AMBER);
            SelectObject(hdc, hFontBold);
            SetTextColor(hdc, COL_AMBER);
            RECT rc = { x + w - 215, cy + 10, x + w - 10, cy + 40 };
            DrawTextA(hdc, progBuf, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            SelectObject(hdc, hFontSmall);
            SetTextColor(hdc, COL_TEXT_DIM);
            char scBuf[64];
            sprintf(scBuf, "Scout: %s%s", scout ? scout->name : "Scout", exp->hasStimpack ? " (+Stim)" : "");
            TextOutA(hdc, x + w - 215, cy + 46, scBuf, (int)strlen(scBuf));
        } else {
            // Stimpack toggle button
            COLORREF stimBg = exp->hasStimpack ? RGB(25, 45, 30) : COL_DARK_CARD;
            COLORREF stimTxt = exp->hasStimpack ? COL_GREEN : COL_TEXT_DIM;
            COLORREF stimBdr = exp->hasStimpack ? COL_GREEN : COL_BORDER;
            DrawButtonControl(hdc, hFontSmall, x + w - 220, cy + 24, 75, 28, exp->hasStimpack ? "STIM: ON" : "STIM: OFF", stimTxt, stimBg, stimBdr, BTN_EXP_STIM_TOGGLE, i, 0);

            // Dispatch Button
            COLORREF btnBg = (unassigned > 0) ? RGB(25, 45, 30) : COL_DARK_CARD;
            COLORREF btnTxt = (unassigned > 0) ? COL_TEXT_BRIGHT : COL_TEXT_DIM;
            DrawButtonControl(hdc, hFontBold, x + w - 140, cy + 24, 130, 28, "DISPATCH SCOUT", btnTxt, btnBg, unassigned > 0 ? COL_GREEN : COL_BORDER, BTN_DISPATCH_SCOUT, i, 0);
        }
    }
}

static void DrawDefenseView(HDC hdc, HFONT hFontBold, HFONT hFontSmall, int x, int y, int w, int h) {
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x, y, "VAULT DEFENSE PERIMETER & RAIDER SECURITY", 41);

    int totalDef = CalculateTotalDefense();
    g_state.defense = totalDef;

    char totBuf[32];
    sprintf(totBuf, "Total Rating: %d pts", totalDef);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_GREEN);
    TextOutA(hdc, x + w - 160, y, totBuf, (int)strlen(totBuf));

    // 1. Radar Alert Banner
    int radY = y + 20;
    int radH = 46;
    DrawStyledBox(hdc, x, radY, w, radH, RGB(25, 12, 10), COL_RED);

    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x + 8, radY + 4, "SURFACE SEISMIC RADAR // HOSTILE TRACKING", 41);

    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_RED);
    char radarMsg[80];
    if (g_state.raidThreatDays <= 1) {
        sprintf(radarMsg, "HOSTILE WARBAND DETECTED // ASSAULT IMMINENT TOMORROW!");
    } else {
        sprintf(radarMsg, "HOSTILE SCOUTS ON SENSORS // ETA: %d DAYS", g_state.raidThreatDays);
    }
    TextOutA(hdc, x + 8, radY + 18, radarMsg, (int)strlen(radarMsg));

    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_MAIN);
    const char* pClans[] = { "Rustfang Marauders (~35)", "Iron Skull Warband (~50)", "Rad-Scorpion Reavers (~70)", "Dune Stalkers (~90)", "Super-Mutant Siege (~110)" };
    int cIdx = g_state.day / 3;
    if (cIdx > 4) cIdx = 4;
    char pBuf[64];
    sprintf(pBuf, "Projected Warband: %s", pClans[cIdx]);
    TextOutA(hdc, x + 8, radY + 31, pBuf, (int)strlen(pBuf));

    // Scramble / Test Raid Button
    DrawButtonControl(hdc, hFontBold, x + w - 180, radY + 10, 170, 26, "TEST DEFENSES [RAID]", COL_RED, COL_DARK_CARD, COL_RED, BTN_DEF_TEST_RAID, 0, 0);

    // 2. Three Fortification Cards
    int cardY = radY + radH + 8;
    int cardW = (w - 16) / 3;
    int cardH = 100;

    // Card 1: Barricades
    DrawStyledBox(hdc, x, cardY, cardW, cardH, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x + 8, cardY + 6, "PERIMETER BARRICADES", 20);

    char barBuf[32];
    sprintf(barBuf, "%d/%d HP", g_state.barricadeHp, g_state.barricadeMaxHp);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, g_state.barricadeHp < 40 ? COL_RED : (g_state.barricadeHp < 75 ? COL_AMBER : COL_GREEN));
    TextOutA(hdc, x + cardW - 75, cardY + 7, barBuf, (int)strlen(barBuf));

    float barPct = (float)g_state.barricadeHp / (float)(g_state.barricadeMaxHp > 0 ? g_state.barricadeMaxHp : 100);
    DrawProgressBar(hdc, x + 8, cardY + 22, cardW - 16, 6, barPct, g_state.barricadeHp < 40 ? COL_RED : COL_GREEN);

    char bDefBuf[32];
    sprintf(bDefBuf, "Defense: +%d pts (0.2x HP)", (g_state.barricadeHp * 20) / (g_state.barricadeMaxHp > 0 ? g_state.barricadeMaxHp : 100));
    SetTextColor(hdc, COL_TEXT_MAIN);
    TextOutA(hdc, x + 8, cardY + 34, bDefBuf, (int)strlen(bDefBuf));

    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x + 8, cardY + 48, "Steel blast-doors absorb shock", 30);

    COLORREF rBg = (g_state.scrap >= 15 && g_state.barricadeHp < g_state.barricadeMaxHp) ? COL_BTN_BG : COL_DARK_CARD;
    COLORREF rTxt = (g_state.scrap >= 15 && g_state.barricadeHp < g_state.barricadeMaxHp) ? COL_GREEN : COL_TEXT_DIM;
    DrawButtonControl(hdc, hFontSmall, x + 8, cardY + 68, (cardW - 20) / 2, 22, "REPAIR (15S)", rTxt, rBg, COL_BORDER, BTN_DEF_REPAIR, 0, 0);

    COLORREF rfBg = (g_state.scrap >= 35) ? COL_BTN_BG : COL_DARK_CARD;
    COLORREF rfTxt = (g_state.scrap >= 35) ? COL_AMBER : COL_TEXT_DIM;
    DrawButtonControl(hdc, hFontSmall, x + 12 + (cardW - 20) / 2, cardY + 68, (cardW - 20) / 2, 22, "MAX+25 (35S)", rfTxt, rfBg, COL_BORDER, BTN_DEF_REINFORCE, 0, 0);

    // Card 2: Turrets
    int c2X = x + cardW + 8;
    DrawStyledBox(hdc, c2X, cardY, cardW, cardH, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, c2X + 8, cardY + 6, "SENTRY TURRETS", 14);

    char turBuf[24];
    sprintf(turBuf, "%d MOUNTED", g_state.turretCount);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_AMBER);
    TextOutA(hdc, c2X + cardW - 75, cardY + 7, turBuf, (int)strlen(turBuf));

    char tPowerBuf[48];
    sprintf(tPowerBuf, "Output: +%d Def pts", g_state.turretCount * 18 + (g_state.turretOverclock ? 10 : 0));
    SetTextColor(hdc, COL_TEXT_MAIN);
    TextOutA(hdc, c2X + 8, cardY + 22, tPowerBuf, (int)strlen(tPowerBuf));

    char tDrawBuf[48];
    sprintf(tDrawBuf, "Draw: -%d kW | OC: %s", g_state.turretCount * 3, g_state.turretOverclock ? "ON (+10)" : "OFF");
    SetTextColor(hdc, g_state.turretOverclock ? COL_GREEN : COL_TEXT_DIM);
    TextOutA(hdc, c2X + 8, cardY + 36, tDrawBuf, (int)strlen(tDrawBuf));

    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, c2X + 8, cardY + 50, "Twin 50-cal crossfire sentries", 30);

    COLORREF bTurBg = (g_state.scrap >= 45) ? COL_BTN_BG : COL_DARK_CARD;
    COLORREF bTurTxt = (g_state.scrap >= 45) ? COL_GREEN : COL_TEXT_DIM;
    DrawButtonControl(hdc, hFontSmall, c2X + 8, cardY + 68, (cardW - 20) / 2, 22, "+TURRET(45S)", bTurTxt, bTurBg, COL_BORDER, BTN_DEF_TURRET, 0, 0);

    COLORREF ocBg = (!g_state.turretOverclock && g_state.scrap >= 20) ? COL_BTN_BG : COL_DARK_CARD;
    COLORREF ocTxt = (!g_state.turretOverclock && g_state.scrap >= 20) ? COL_AMBER : COL_TEXT_DIM;
    DrawButtonControl(hdc, hFontSmall, c2X + 12 + (cardW - 20) / 2, cardY + 68, (cardW - 20) / 2, 22, "OVERCLK(20S)", ocTxt, ocBg, COL_BORDER, BTN_DEF_OVERCLOCK, 0, 0);

    // Card 3: Combat Readiness
    int c3X = c2X + cardW + 8;
    DrawStyledBox(hdc, c3X, cardY, cardW, cardH, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, c3X + 8, cardY + 6, "COMBAT DRILLS", 13);

    char drBuf[24];
    sprintf(drBuf, "DRILL LV %d", g_state.combatDrillLevel);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_CYAN);
    TextOutA(hdc, c3X + cardW - 75, cardY + 7, drBuf, (int)strlen(drBuf));

    int activeG = 0;
    int guardPower = 0;
    int dBonus = g_state.combatDrillLevel * 3;
    for (int s = 0; s < g_state.numSurvivors; s++) {
        if (strcmp(g_state.survivors[s].job, "security") == 0) {
            activeG++;
            guardPower += (g_state.survivors[s].str * 2) + dBonus;
        } else if (strcmp(g_state.survivors[s].job, "expedition") != 0) {
            guardPower += g_state.survivors[s].str / 2;
        }
    }

    char gPowBuf[48];
    sprintf(gPowBuf, "Guard Def: +%d pts", guardPower);
    SetTextColor(hdc, COL_TEXT_MAIN);
    TextOutA(hdc, c3X + 8, cardY + 22, gPowBuf, (int)strlen(gPowBuf));

    char gCntBuf[48];
    sprintf(gCntBuf, "Guards: %d | Militia: %d", activeG, g_state.numSurvivors - activeG);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, c3X + 8, cardY + 36, gCntBuf, (int)strlen(gCntBuf));

    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, c3X + 8, cardY + 50, "+3 Def bonus per drill level", 28);

    COLORREF drBg = (g_state.scrap >= 12) ? COL_BTN_BG : COL_DARK_CARD;
    COLORREF drTxt = (g_state.scrap >= 12) ? COL_CYAN : COL_TEXT_DIM;
    DrawButtonControl(hdc, hFontSmall, c3X + 8, cardY + 68, cardW - 16, 22, "RUN DRILL (12 SCRAP)", drTxt, drBg, COL_BORDER, BTN_DEF_DRILL, 0, 0);

    // 3. Guard Roster List
    int rosterY = cardY + cardH + 8;
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x, rosterY, "SECURITY POSTINGS & CITIZEN READINESS", 37);

    int rStartY = rosterY + 18;
    int rowH = 24;
    for (int s = 0; s < g_state.numSurvivors && s < 5; s++) {
        Survivor* surv = &g_state.survivors[s];
        int ry = rStartY + s * (rowH + 3);
        int isGuard = (strcmp(surv->job, "security") == 0);

        DrawStyledBox(hdc, x, ry, w, rowH, COL_PANEL_BG, isGuard ? COL_GREEN : COL_BORDER);

        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, COL_TEXT_BRIGHT);
        TextOutA(hdc, x + 8, ry + 4, surv->name, (int)strlen(surv->name));

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COL_TEXT_DIM);
        char roleBuf[48];
        sprintf(roleBuf, "%s (STR:%d AGI:%d)", surv->role, surv->str, surv->agi);
        TextOutA(hdc, x + 150, ry + 5, roleBuf, (int)strlen(roleBuf));

        int contrib = isGuard ? (surv->str * 2 + dBonus) : (surv->str / 2);
        char cBuf[32];
        sprintf(cBuf, "+%d Def", contrib);
        SetTextColor(hdc, isGuard ? COL_GREEN : COL_TEXT_DIM);
        TextOutA(hdc, x + 340, ry + 5, cBuf, (int)strlen(cBuf));

        const char* gText = isGuard ? "RELIEVE GUARD" : "POST GUARD";
        COLORREF gTxt = isGuard ? COL_RED : COL_GREEN;
        COLORREF gBg = isGuard ? RGB(35, 15, 15) : RGB(15, 35, 20);
        DrawButtonControl(hdc, hFontSmall, x + w - 105, ry + 2, 98, 20, gText, gTxt, gBg, isGuard ? COL_RED : COL_BORDER, BTN_DEF_TOGGLE_GUARD, s, 0);
    }

    // 4. Last Battle Tactical Debrief Box
    int debriefY = rStartY + (g_state.numSurvivors < 5 ? g_state.numSurvivors : 5) * (rowH + 3) + 6;
    DrawStyledBox(hdc, x, debriefY, w, 50, COL_DARK_CARD, COL_BORDER);

    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x + 8, debriefY + 5, "LAST INCURSION DEBRIEF:", 23);

    SelectObject(hdc, hFontSmall);
    if (g_state.lastRaidAtk > 0) {
        SetTextColor(hdc, g_state.lastRaidWon ? COL_GREEN : COL_RED);
        TextOutA(hdc, x + 175, debriefY + 5, g_state.lastRaidWon ? "[PERIMETER HELD - VICTORY]" : "[PERIMETER BREACHED]", g_state.lastRaidWon ? 26 : 20);

        char debBuf[160];
        if (g_state.lastRaidWon) {
            sprintf(debBuf, "Repelled %s (Atk %d vs Def %d). Salvaged +%d Scrap, +%d Meds. Barricades: -%d HP.",
                g_state.lastRaidClan, g_state.lastRaidAtk, g_state.lastRaidDef, g_state.lastRaidScrap, g_state.lastRaidMeds, g_state.lastRaidDmg);
        } else {
            sprintf(debBuf, "Breached by %s (Atk %d vs Def %d). Lost %d Food, %d Scrap. %d injured! Barricades: -%d HP.",
                g_state.lastRaidClan, g_state.lastRaidAtk, g_state.lastRaidDef, g_state.lastRaidFoodStolen, g_state.lastRaidScrapStolen, g_state.lastRaidInjured, g_state.lastRaidDmg);
        }
        SetTextColor(hdc, COL_TEXT_MAIN);
        TextOutA(hdc, x + 8, debriefY + 22, debBuf, (int)strlen(debBuf));
    } else {
        SetTextColor(hdc, COL_TEXT_DIM);
        TextOutA(hdc, x + 175, debriefY + 5, "[NO RECENT HOSTILE ENGAGEMENTS]", 31);
        TextOutA(hdc, x + 8, debriefY + 22, "Perimeter sensors report all quiet. Keep turrets powered and barricades reinforced.", 83);
    }
}

static void DrawResearchView(HDC hdc, HFONT hFontBold, HFONT hFontSmall, int x, int y, int w, int h) {
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x, y, "R&D SCIENCE TERMINAL // TECHNOLOGY RESEARCH TREE", 48);

    char statBuf[64];
    sprintf(statBuf, "Active Technologies: %d / 8 Researched", GetResearchedCount());
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_GREEN);
    TextOutA(hdc, x + w - 240, y, statBuf, (int)strlen(statBuf));

    // Summary banner
    int bannerH = 22;
    DrawStyledBox(hdc, x, y + 20, w, bannerH, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x + 8, y + 23, "Invest Tech Scrap to unlock advanced facilities and permanent shelter-wide passive perks.", 89);
    char scrapBuf[32];
    sprintf(scrapBuf, "Available Scrap: %.0f", g_state.scrap);
    SetTextColor(hdc, COL_AMBER);
    TextOutA(hdc, x + w - 150, y + 23, scrapBuf, (int)strlen(scrapBuf));

    int startY = y + 48;
    int colW = (w - 10) / 2;
    int cardH = 92;
    int gapY = 8;

    for (int i = 0; i < 8; i++) {
        const TechInfo* ti = &g_techTree[i];
        int col = i % 2;
        int row = i / 2;
        int cx = x + col * (colW + 10);
        int cy = startY + row * (cardH + gapY);

        int isRes = IsTechResearched(i);
        int reqMet = (ti->reqTechId < 0) || IsTechResearched(ti->reqTechId);
        int cost = GetEffectiveScrapCost(ti->cost);
        int canAfford = (g_state.scrap >= (float)cost);

        COLORREF borderCol = isRes ? COL_GREEN : (reqMet ? COL_BORDER_HI : COL_BORDER);
        COLORREF bgCol = isRes ? RGB(15, 30, 20) : COL_DARK_CARD;
        DrawStyledBox(hdc, cx, cy, colW, cardH, bgCol, borderCol);

        // Header: Name + Tag
        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, isRes ? COL_GREEN : COL_TEXT_BRIGHT);
        TextOutA(hdc, cx + 8, cy + 5, ti->name, (int)strlen(ti->name));

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COL_AMBER);
        char tagBuf[24];
        sprintf(tagBuf, "[%s]", ti->tag);
        TextOutA(hdc, cx + colW - (int)strlen(tagBuf) * 7 - 8, cy + 5, tagBuf, (int)strlen(tagBuf));

        // Description
        SetTextColor(hdc, COL_TEXT_DIM);
        TextOutA(hdc, cx + 8, cy + 22, ti->desc, (int)strlen(ti->desc));

        // Benefit
        SetTextColor(hdc, COL_GREEN);
        char benBuf[96];
        sprintf(benBuf, "Bonus: %s", ti->benefit);
        TextOutA(hdc, cx + 8, cy + 39, benBuf, (int)strlen(benBuf));

        // Prereq line
        if (strlen(ti->reqTechName) > 0) {
            char reqBuf[64];
            sprintf(reqBuf, "Req: %s %s", ti->reqTechName, reqMet ? "(MET)" : "(LOCKED)");
            SetTextColor(hdc, reqMet ? COL_GREEN : COL_RED);
            TextOutA(hdc, cx + 8, cy + 56, reqBuf, (int)strlen(reqBuf));
        }

        // Action button or Status
        if (isRes) {
            SetTextColor(hdc, COL_GREEN);
            RECT rc = { cx + colW - 190, cy + 68, cx + colW - 8, cy + 88 };
            DrawTextA(hdc, "[* RESEARCH ACTIVE]", -1, &rc, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
        } else if (!reqMet) {
            SetTextColor(hdc, COL_RED);
            RECT rc = { cx + colW - 200, cy + 68, cx + colW - 8, cy + 88 };
            DrawTextA(hdc, "[LOCKED: PREREQ REQUIRED]", -1, &rc, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
        } else {
            char bText[36];
            sprintf(bText, "RESEARCH (%d SCRAP)", cost);
            COLORREF cBg = canAfford ? RGB(25, 45, 25) : COL_PANEL_BG;
            COLORREF cTxt = canAfford ? COL_TEXT_BRIGHT : COL_TEXT_DIM;
            COLORREF cBdr = canAfford ? COL_GREEN : COL_BORDER;
            DrawButtonControl(hdc, hFontBold, cx + colW - 165, cy + 68, 158, 20, bText, cTxt, cBg, cBdr, BTN_RESEARCH_TECH, i, 0);
        }
    }
}


static void DrawHazardsView(HDC hdc, HFONT hFontBold, HFONT hFontSmall, int x, int y, int w, int h) {
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x, y, "ENVIRONMENTAL DISASTERS & WEATHER HAZARDS", 41);

    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_GREEN);
    TextOutA(hdc, x + w - 160, y, "SENSOR ARRAY: ONLINE", 20);

    const char* wNames[] = { "Calm Irradiated Skies", "Ion Radiation Storm", "Scorching Drought", "Corrosive Acid Rain", "Sub-Zero Cold Snap" };
    const char* wDescs[] = {
        "Standard background radiation. Atmospheric currents are stable with minimal ionic disturbance.",
        "Intense gamma fallout storm. Radiation spikes to 15+ Rads/h. Penetrates unshielded airlocks.",
        "Extreme heatwave drying aquifers. Water production cut 50%, dweller thirst increases 50%.",
        "Toxic chemical precipitation dissolving steel armor. Barricades sustain -30 HP/day erosion.",
        "Cryogenic frost freezing generator fuel conduits (-35% Gen). Habitat requires active heating."
    };

    // 1. Current Atmospheric Conditions Box
    int boxH = 68;
    DrawStyledBox(hdc, x, y + 20, w, boxH, COL_DARK_CARD, (g_state.weatherType == 0) ? COL_BORDER : ((g_state.weatherType == 1 || g_state.weatherType == 3) ? COL_RED : COL_AMBER));

    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x + 8, y + 26, "SURFACE IONIZATION & METEOROLOGY TELEMETRY", 42);

    SelectObject(hdc, hFontBold);
    COLORREF wTitleCol = (g_state.weatherType == 0) ? COL_TEXT_BRIGHT : ((g_state.weatherType == 1 || g_state.weatherType == 3) ? COL_RED : COL_AMBER);
    SetTextColor(hdc, wTitleCol);
    char wTitleBuf[64];
    sprintf(wTitleBuf, "[ %s ]", wNames[g_state.weatherType]);
    TextOutA(hdc, x + 8, y + 42, wTitleBuf, (int)strlen(wTitleBuf));

    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_MAIN);
    TextOutA(hdc, x + 8, y + 62, wDescs[g_state.weatherType], (int)strlen(wDescs[g_state.weatherType]));

    char radBuf[48], durBuf[32];
    sprintf(radBuf, "Rads: %.1f Rads/h", g_state.exteriorRads);
    if (g_state.weatherType == 0) strcpy(durBuf, "Duration: Calm");
    else sprintf(durBuf, "Duration: %dd left", g_state.weatherDaysLeft);

    SetTextColor(hdc, (g_state.exteriorRads > 10.0f) ? COL_RED : COL_AMBER);
    TextOutA(hdc, x + w - 140, y + 26, radBuf, (int)strlen(radBuf));
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x + w - 140, y + 44, durBuf, (int)strlen(durBuf));

    // 2. Early Warning Doppler Radar Forecast Box
    int foreY = y + 20 + boxH + 8;
    int foreH = 50;
    DrawStyledBox(hdc, x, foreY, w, foreH, COL_PANEL_BG, COL_BORDER_HI);

    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_AMBER);
    TextOutA(hdc, x + 8, foreY + 6, "DOPPLER RADAR EARLY WARNING FORECAST", 36);

    char etaBuf[32];
    if (g_state.forecastEtaDays <= 1) strcpy(etaBuf, "INCOMING TOMORROW!");
    else sprintf(etaBuf, "INCOMING IN %d DAYS", g_state.forecastEtaDays);
    SetTextColor(hdc, (g_state.forecastEtaDays <= 1) ? COL_RED : COL_AMBER);
    TextOutA(hdc, x + w - 160, foreY + 6, etaBuf, (int)strlen(etaBuf));

    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    char fProjBuf[128];
    sprintf(fProjBuf, "Projected Hazard: %s (Tier %d) - Recommend preparing appropriate countermeasures.", wNames[g_state.forecastType], g_state.forecastSeverity);
    TextOutA(hdc, x + 8, foreY + 26, fProjBuf, (int)strlen(fProjBuf));

    // 3. Hazard Modifiers (4 Cards in 2x2 Grid)
    int modY = foreY + foreH + 8;
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x, modY, "ACTIVE ATMOSPHERIC IMPACT & MODIFIERS", 37);

    int cardStartY = modY + 18;
    int colW = (w - 8) / 2;
    int cardH = 52;

    // Card 1: Rad Exposure
    DrawStyledBox(hdc, x, cardStartY, colW, cardH, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, (g_state.weatherType == 1) ? COL_RED : COL_TEXT_BRIGHT);
    TextOutA(hdc, x + 6, cardStartY + 4, "1. RADIATION EXPOSURE", 21);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, (g_state.weatherType == 1) ? COL_RED : COL_GREEN);
    int hasShield = g_state.cmRadBulkhead || g_state.techRadShield;
    if (g_state.weatherType == 1) {
        TextOutA(hdc, x + 6, cardStartY + 20, hasShield ? "Status: Protected by Bulkhead / Nano-Shield" : "CRITICAL: Unshielded sickness (-12 HP/d)!", hasShield ? 43 : 42);
    } else {
        TextOutA(hdc, x + 6, cardStartY + 20, "Status: Safe baseline radiation levels", 38);
    }
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x + 6, cardStartY + 34, "Shielding blocks gamma storm sickness.", 38);

    // Card 2: Hydrological Drought
    DrawStyledBox(hdc, x + colW + 8, cardStartY, colW, cardH, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, (g_state.weatherType == 2) ? COL_AMBER : COL_TEXT_BRIGHT);
    TextOutA(hdc, x + colW + 14, cardStartY + 4, "2. HYDROLOGICAL WATER IMPACT", 28);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, (g_state.weatherType == 2) ? COL_RED : COL_GREEN);
    if (g_state.weatherType == 2) {
        TextOutA(hdc, x + colW + 14, cardStartY + 20, "DROUGHT ACTIVE: -50% Water Yield, +50% Thirst", 45);
    } else {
        TextOutA(hdc, x + colW + 14, cardStartY + 20, "Aquifer Yield: 100% | Evaporation: Normal", 41);
    }
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x + colW + 14, cardStartY + 34, "Aquifer overpumping restores emergency water.", 45);

    // Card 3: Acid Corrosion
    DrawStyledBox(hdc, x, cardStartY + cardH + 6, colW, cardH, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, (g_state.weatherType == 3) ? COL_RED : COL_TEXT_BRIGHT);
    TextOutA(hdc, x + 6, cardStartY + cardH + 10, "3. ACID RAIN & BARRICADE EROSION", 32);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, (g_state.weatherType == 3) ? COL_RED : COL_GREEN);
    if (g_state.weatherType == 3) {
        TextOutA(hdc, x + 6, cardStartY + cardH + 26, (g_state.cmAcidNeutralizerDays > 0) ? "Neutralizer Active: Acid corrosion blocked" : "CORROSION: Barricades taking -30 HP/day!", (g_state.cmAcidNeutralizerDays > 0) ? 42 : 40);
    } else {
        TextOutA(hdc, x + 6, cardStartY + cardH + 26, "Precipitation: Neutral pH | Corrosion: 0", 40);
    }
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x + 6, cardStartY + cardH + 38, "Alkaline wash repairs & grants 2d acid immunity.", 48);

    // Card 4: Cold Snap Frost
    DrawStyledBox(hdc, x + colW + 8, cardStartY + cardH + 6, colW, cardH, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, (g_state.weatherType == 4) ? COL_CYAN : COL_TEXT_BRIGHT);
    TextOutA(hdc, x + colW + 14, cardStartY + cardH + 10, "4. SUB-ZERO CRYO BLIGHT", 23);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, (g_state.weatherType == 4) ? COL_CYAN : COL_GREEN);
    if (g_state.weatherType == 4) {
        TextOutA(hdc, x + colW + 14, cardStartY + cardH + 26, g_state.cmThermalOverdrive ? "Heating Active: Thermal overdrive online" : "FROST ALERT: -35% Power Gen, +4 kW Heat Load", g_state.cmThermalOverdrive ? 40 : 44);
    } else {
        TextOutA(hdc, x + colW + 14, cardStartY + cardH + 26, "Habitat Temp: 22°C | Power Gen: 100%", 36);
    }
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x + colW + 14, cardStartY + cardH + 38, "Auxiliary thermal overdrive negates cold penalties.", 51);

    // 4. Countermeasures & Mitigation Systems
    int cmY = cardStartY + cardH * 2 + 18;
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x, cmY, "EMERGENCY HAZARD MITIGATION & COUNTERMEASURES", 45);

    int cmBtnY = cmY + 18;
    int cmW = (w - 18) / 4;
    int cmH = 58;

    // CM 1: Bulkhead
    COLORREF b1Bg = g_state.cmRadBulkhead ? RGB(25, 50, 30) : COL_DARK_CARD;
    COLORREF b1Txt = g_state.cmRadBulkhead ? COL_GREEN : COL_TEXT_DIM;
    COLORREF b1Bdr = g_state.cmRadBulkhead ? COL_GREEN : COL_BORDER;
    DrawStyledBox(hdc, x, cmBtnY, cmW, cmH, b1Bg, b1Bdr);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x + 4, cmBtnY + 4, "RAD BULKHEAD SEAL", 17);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x + 4, cmBtnY + 18, "Draw: 5 kW Power", 16);
    DrawButtonControl(hdc, hFontSmall, x + 4, cmBtnY + 34, cmW - 8, 20, g_state.cmRadBulkhead ? "DISENGAGE" : "ENGAGE SEAL", g_state.cmRadBulkhead ? COL_RED : COL_TEXT_BRIGHT, COL_BTN_BG, COL_BORDER, BTN_HAZARD_TOGGLE_BULKHEAD, 0, 0);

    // CM 2: Overpump
    DrawStyledBox(hdc, x + cmW + 6, cmBtnY, cmW, cmH, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x + cmW + 10, cmBtnY + 4, "AQUIFER OVERPUMP", 16);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x + cmW + 10, cmBtnY + 18, "Cost: 15 Scrap (+8 W)", 21);
    int canPump = (g_state.scrap >= 15.0f);
    DrawButtonControl(hdc, hFontSmall, x + cmW + 10, cmBtnY + 34, cmW - 8, 20, "PUMP WATER", canPump ? COL_GREEN : COL_TEXT_DIM, COL_BTN_BG, COL_BORDER, BTN_HAZARD_OVERPUMP, 0, 0);

    // CM 3: Neutralizer
    int isNeut = (g_state.cmAcidNeutralizerDays > 0);
    COLORREF nBg = isNeut ? RGB(25, 45, 30) : COL_DARK_CARD;
    COLORREF nBdr = isNeut ? COL_GREEN : COL_BORDER;
    DrawStyledBox(hdc, x + (cmW + 6) * 2, cmBtnY, cmW, cmH, nBg, nBdr);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x + (cmW + 6) * 2 + 4, cmBtnY + 4, "ACID NEUTRALIZER", 16);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, isNeut ? COL_GREEN : COL_TEXT_DIM);
    char nBuf[32];
    if (isNeut) sprintf(nBuf, "Active (%dd left)", g_state.cmAcidNeutralizerDays);
    else strcpy(nBuf, "Cost: 12S (+25 HP)");
    TextOutA(hdc, x + (cmW + 6) * 2 + 4, cmBtnY + 18, nBuf, (int)strlen(nBuf));
    int canNeut = (g_state.scrap >= 12.0f);
    DrawButtonControl(hdc, hFontSmall, x + (cmW + 6) * 2 + 4, cmBtnY + 34, cmW - 8, 20, "SPRAY WASH", canNeut ? COL_TEXT_BRIGHT : COL_TEXT_DIM, COL_BTN_BG, COL_BORDER, BTN_HAZARD_NEUTRALIZER, 0, 0);

    // CM 4: Thermal Overdrive
    COLORREF tBg = g_state.cmThermalOverdrive ? RGB(20, 45, 55) : COL_DARK_CARD;
    COLORREF tBdr = g_state.cmThermalOverdrive ? COL_CYAN : COL_BORDER;
    DrawStyledBox(hdc, x + (cmW + 6) * 3, cmBtnY, cmW, cmH, tBg, tBdr);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x + (cmW + 6) * 3 + 4, cmBtnY + 4, "THERMAL OVERDRIVE", 17);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x + (cmW + 6) * 3 + 4, cmBtnY + 18, "Draw: 4 kW Power", 16);
    DrawButtonControl(hdc, hFontSmall, x + (cmW + 6) * 3 + 4, cmBtnY + 34, cmW - 8, 20, g_state.cmThermalOverdrive ? "DISABLE" : "ENGAGE HEAT", g_state.cmThermalOverdrive ? COL_RED : COL_CYAN, COL_BTN_BG, COL_BORDER, BTN_HAZARD_TOGGLE_THERMAL, 0, 0);

    // 5. Simulation Drill Buttons
    int simY = cmBtnY + cmH + 10;
    DrawStyledBox(hdc, x, simY, w, 32, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x + 6, simY + 8, "SIMULATE HAZARD DRILL:", 22);

    int sBtnW = 75;
    DrawButtonControl(hdc, hFontSmall, x + 160, simY + 5, sBtnW, 22, "RAD STORM", COL_RED, COL_PANEL_BG, COL_BORDER, BTN_HAZARD_SIMULATE, 1, 0);
    DrawButtonControl(hdc, hFontSmall, x + 160 + (sBtnW + 4), simY + 5, sBtnW, 22, "DROUGHT", COL_AMBER, COL_PANEL_BG, COL_BORDER, BTN_HAZARD_SIMULATE, 2, 0);
    DrawButtonControl(hdc, hFontSmall, x + 160 + (sBtnW + 4) * 2, simY + 5, sBtnW, 22, "ACID RAIN", COL_GREEN, COL_PANEL_BG, COL_BORDER, BTN_HAZARD_SIMULATE, 3, 0);
    DrawButtonControl(hdc, hFontSmall, x + 160 + (sBtnW + 4) * 3, simY + 5, sBtnW, 22, "COLD SNAP", COL_CYAN, COL_PANEL_BG, COL_BORDER, BTN_HAZARD_SIMULATE, 4, 0);
    DrawButtonControl(hdc, hFontSmall, x + 160 + (sBtnW + 4) * 4, simY + 5, sBtnW, 22, "CLEAR SKY", COL_TEXT_MAIN, COL_PANEL_BG, COL_BORDER, BTN_HAZARD_SIMULATE, 0, 0);
}

static void DrawTradingView(HDC hdc, HFONT hFontBold, HFONT hFontSmall, int x, int y, int w, int h) {
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x, y, "WASTELAND NOMADIC CARAVAN & BARTER TRADING", 43);

    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, g_state.caravanPresent ? COL_GREEN : COL_RED);
    TextOutA(hdc, x + w - 160, y, g_state.caravanPresent ? "AIRLOCK DECK: OPEN" : "AIRLOCK DECK: CLOSED", g_state.caravanPresent ? 18 : 20);

    int curY = y + 20;

    // 1. Merchant Header Box
    int bnrH = 58;
    const CaravanFactionInfo* fInfo = &g_caravanFactions[g_state.caravanFaction % 5];
    DrawStyledBox(hdc, x, curY, w, bnrH, COL_DARK_CARD, g_state.caravanPresent ? COL_AMBER : COL_BORDER);

    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, g_state.caravanPresent ? COL_AMBER : COL_TEXT_DIM);
    char fTitle[80];
    if (g_state.caravanPresent) {
        sprintf(fTitle, "[%s] - %s", fInfo->badge, fInfo->merchant);
    } else {
        strcpy(fTitle, "[NO VISITING TRADER] - Airlock Merchant Bay Empty");
    }
    TextOutA(hdc, x + 8, curY + 6, fTitle, (int)strlen(fTitle));

    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_MAIN);
    if (g_state.caravanPresent) {
        TextOutA(hdc, x + 8, curY + 24, fInfo->quote, (int)strlen(fInfo->quote));
    } else {
        char nextBuf[96];
        sprintf(nextBuf, "The caravan traveled into the wastes. Next merchant scheduled to arrive in %d days.", g_state.caravanEtaDays);
        TextOutA(hdc, x + 8, curY + 24, nextBuf, (int)strlen(nextBuf));
    }

    // Trust level
    float disc = GetCaravanDiscount();
    int discPct = (int)(disc * 100.0f);
    const char* repNames[] = { "Neutral", "Welcomed", "Trusted", "Honored", "Revered" };
    int repIdx = g_state.caravanRepLevel - 1;
    if (repIdx < 0) repIdx = 0;
    if (repIdx > 4) repIdx = 4;

    char repBuf[96];
    sprintf(repBuf, "Barter Trust: %s (Lv %d) | Discount: %d%% | Trades Done: %d", repNames[repIdx], g_state.caravanRepLevel, discPct, g_state.caravanTradesCount);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x + 8, curY + 40, repBuf, (int)strlen(repBuf));

    // Status on right + Hail button
    char statBuf[32];
    if (g_state.caravanPresent) sprintf(statBuf, "%d DAYS IN PORT", g_state.caravanDaysLeft);
    else sprintf(statBuf, "ETA: %d DAYS", g_state.caravanEtaDays);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, g_state.caravanPresent ? COL_TEXT_BRIGHT : COL_TEXT_DIM);
    TextOutA(hdc, x + w - 170, curY + 6, statBuf, (int)strlen(statBuf));

    int canHail = (g_state.scrap >= 15.0f && g_state.powerGen >= 10);
    DrawButtonControl(hdc, hFontSmall, x + w - 175, curY + 28, 168, 22, "HAIL CARAVAN (15S, 10kW)", canHail ? COL_AMBER : COL_TEXT_DIM, COL_BTN_BG, COL_BORDER, BTN_HAIL_CARAVAN, 0, 0);

    curY += bnrH + 8;

    // 2. Standard Supplies (4 Cards in 2x2 Grid)
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x, curY, "1. PROVISIONS & VITAL STOCK (BUY WITH SCRAP)", 43);
    curY += 18;

    int colW = (w - 8) / 2;
    int cardH = 50;

    const char* supNames[] = { "FOOD RATIONS (SMALL)", "PURIFIED WATER (JUG)", "BULK FOOD & WATER CRATE", "SURGICAL STIMPACKS (2-PK)" };
    const char* supDescs[] = { "Nutrient biscuits & meat (+10 Food)", "Triple-filtered aquifer water (+10 Water)", "+25 Food & +25 Purified Water", "Sterile syringes & coagulants (+2 Meds)" };
    int supCosts[] = { 8, 8, 30, 12 };

    for (int i = 0; i < 4; i++) {
        int cx = (i % 2 == 0) ? x : x + colW + 8;
        int cy = curY + (i / 2) * (cardH + 6);

        int cost = GetDiscountedTradeCost(supCosts[i]);
        int canAfford = g_state.caravanPresent && (g_state.scrap >= (float)cost);

        DrawStyledBox(hdc, cx, cy, colW, cardH, COL_DARK_CARD, COL_BORDER);

        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, COL_TEXT_BRIGHT);
        TextOutA(hdc, cx + 6, cy + 4, supNames[i], (int)strlen(supNames[i]));

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COL_TEXT_DIM);
        TextOutA(hdc, cx + 6, cy + 20, supDescs[i], (int)strlen(supDescs[i]));

        char bText[32];
        sprintf(bText, "BUY (%dS)", cost);
        DrawButtonControl(hdc, hFontSmall, cx + colW - 85, cy + 14, 78, 22, bText, canAfford ? COL_GREEN : COL_TEXT_DIM, COL_BTN_BG, COL_BORDER, BTN_TRADE_BUY, i, 0);
    }

    curY += (cardH + 6) * 2 + 6;

    // 3. Rare Artifacts (3 Cards in a row)
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_AMBER);
    TextOutA(hdc, x, curY, "2. EXCLUSIVE CARAVAN ARTIFACTS & SPECIALTY TECH", 46);
    curY += 18;

    int rColW = (w - 12) / 3;
    int rCardH = 78;

    const char* rNames[] = { "RAD-AWAY CASK", "SUPER-GROWTH ENZYME", "BALLISTIC PLATES", "TARGETING MODULE", "LUXURY CASSETTES" };
    const char* rDescs[] = { "Cleanses radiation illness", "+30 Food to storage", "+25 Max HP & repair", "+12 Base Defense", "+20% Vault Morale" };
    int rCosts[] = { 22, 25, 28, 35, 18 };

    for (int i = 0; i < 3; i++) {
        int rx = x + i * (rColW + 6);
        int ry = curY;

        int cost = GetDiscountedTradeCost(rCosts[i]);
        int isBought = g_state.caravanRareBought[i];
        int canAfford = g_state.caravanPresent && !isBought && (g_state.scrap >= (float)cost);

        COLORREF rBdr = isBought ? COL_GREEN : (canAfford ? COL_AMBER : COL_BORDER);
        DrawStyledBox(hdc, rx, ry, rColW, rCardH, COL_DARK_CARD, rBdr);

        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, isBought ? COL_GREEN : COL_TEXT_BRIGHT);
        TextOutA(hdc, rx + 6, ry + 4, rNames[i], (int)strlen(rNames[i]));

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COL_TEXT_DIM);
        TextOutA(hdc, rx + 6, ry + 20, rDescs[i], (int)strlen(rDescs[i]));

        char cBuf[32];
        sprintf(cBuf, "Price: %d Tech Scrap", cost);
        SetTextColor(hdc, COL_AMBER);
        TextOutA(hdc, rx + 6, ry + 36, cBuf, (int)strlen(cBuf));

        if (isBought) {
            SetTextColor(hdc, COL_GREEN);
            TextOutA(hdc, rx + 6, ry + 54, "[* ACQUIRED]", 12);
        } else {
            char bTxt[32];
            sprintf(bTxt, "PURCHASE (%dS)", cost);
            DrawButtonControl(hdc, hFontSmall, rx + 6, ry + 52, rColW - 12, 20, bTxt, canAfford ? COL_TEXT_BRIGHT : COL_TEXT_DIM, COL_BTN_BG, COL_BORDER, BTN_TRADE_RARE, i, 0);
        }
    }

    curY += rCardH + 10;

    // 4. Surplus Export (3 Cards in a row)
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x, curY, "3. SHELTER SURPLUS EXPORT BARTER", 32);
    curY += 18;

    const char* expNames[] = { "EXPORT FOOD", "EXPORT WATER", "BARTER FOR MEDS" };
    const char* expDescs[] = { "Sell 15 Food -> +10 Scrap", "Sell 15 Water -> +10 Scrap", "Sell 20 Scrap -> +2 Meds" };

    for (int i = 0; i < 3; i++) {
        int ex = x + i * (rColW + 6);
        int ey = curY;

        int canSell = g_state.caravanPresent;
        if (i == 0 && g_state.food < 15.0f) canSell = 0;
        if (i == 1 && g_state.water < 15.0f) canSell = 0;
        if (i == 2 && g_state.scrap < 20.0f) canSell = 0;

        DrawStyledBox(hdc, ex, ey, rColW, 46, COL_DARK_CARD, COL_BORDER);

        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, COL_TEXT_BRIGHT);
        TextOutA(hdc, ex + 6, ey + 4, expNames[i], (int)strlen(expNames[i]));

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COL_TEXT_DIM);
        TextOutA(hdc, ex + 6, ey + 20, expDescs[i], (int)strlen(expDescs[i]));

        DrawButtonControl(hdc, hFontSmall, ex + rColW - 68, ey + 10, 62, 24, "BARTER", canSell ? COL_AMBER : COL_TEXT_DIM, COL_BTN_BG, COL_BORDER, BTN_TRADE_SELL, i, 0);
    }
}

static void DrawPoliciesView(HDC hdc, HFONT hFontBold, HFONT hFontSmall, int x, int y, int w, int h) {
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x, y, "OVERSEER PROTOCOLS, HEALTH & MORALE DIRECTIVES", 46);

    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x + w - 210, y, "Directives take effect on cycle advance", 39);

    int curY = y + 24;
    int topCardW = (w - 12) / 3;
    int topCardH = 148;

    // --- TOP OVERVIEW CARDS (HEALTH, WATER PURITY, MORALE ORDER) ---
    // 1. Shelter Health & Rads Card
    int c1X = x;
    DrawStyledBox(hdc, c1X, curY, topCardW, topCardH, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, c1X + 8, curY + 6, "1. HEALTH & RADIATION", 21);

    int totalSurvs = g_state.numSurvivors > 0 ? g_state.numSurvivors : 1;
    int sumHp = 0, sumRads = 0, acuteCount = 0;
    for (int s = 0; s < g_state.numSurvivors; s++) {
        sumHp += g_state.survivors[s].health;
        sumRads += g_state.survivors[s].rads;
        if (g_state.survivors[s].rads >= 50) acuteCount++;
    }
    int avgHp = sumHp / totalSurvs;
    int avgRads = sumRads / totalSurvs;

    char hpTxt[64], radTxt[64];
    sprintf(hpTxt, "Avg Health: %d%% | Dwellers: %d", avgHp, g_state.numSurvivors);
    sprintf(radTxt, "Avg Radiation: %d Rads (%d Sick)", avgRads, acuteCount);

    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_MAIN);
    TextOutA(hdc, c1X + 8, curY + 26, hpTxt, (int)strlen(hpTxt));
    SetTextColor(hdc, (avgRads >= 50) ? COL_RED : ((avgRads >= 25) ? COL_AMBER : COL_GREEN));
    TextOutA(hdc, c1X + 8, curY + 42, radTxt, (int)strlen(radTxt));

    COLORREF radCol = (avgRads >= 50) ? COL_RED : ((avgRads >= 25) ? COL_AMBER : COL_GREEN);
    DrawProgressBar(hdc, c1X + 8, curY + 58, topCardW - 16, 8, avgRads / 100.0f, radCol);

    int canMassRad = (g_state.meds >= 2 && g_state.water >= 10.0f);
    DrawButtonControl(hdc, hFontSmall, c1X + 8, curY + 76, topCardW - 16, 28, "MASS RAD-AWAY (2M, 10W)", canMassRad ? COL_AMBER : COL_TEXT_DIM, canMassRad ? RGB(45, 40, 20) : COL_DARK_CARD, canMassRad ? COL_AMBER : COL_BORDER, BTN_MASS_RADAWAY, 0, 0);

    int canDeconFlush = (g_state.water >= 15.0f && g_state.scrap >= 15.0f);
    DrawButtonControl(hdc, hFontSmall, c1X + 8, curY + 110, topCardW - 16, 28, "DECON FLUSH (15W, 15S)", canDeconFlush ? RGB(100, 200, 255) : COL_TEXT_DIM, canDeconFlush ? RGB(20, 35, 45) : COL_DARK_CARD, canDeconFlush ? RGB(100, 200, 255) : COL_BORDER, BTN_MASS_DECON, 0, 0);

    // 2. Aquifer Water Purity Card
    int c2X = x + topCardW + 6;
    DrawStyledBox(hdc, c2X, curY, topCardW, topCardH, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, c2X + 8, curY + 6, "2. AQUIFER WATER PURITY", 23);

    char purTxt[64], statusTxt[64];
    sprintf(purTxt, "Purity Index: %.0f%%", g_state.waterPurity);
    sprintf(statusTxt, "Quality: %s", (g_state.waterPurity >= 90.0f) ? "Sterile Pure" : ((g_state.waterPurity >= 70.0f) ? "Filtered" : "CONTAMINATED SILT"));

    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, (g_state.waterPurity >= 80.0f) ? COL_GREEN : ((g_state.waterPurity >= 60.0f) ? COL_AMBER : COL_RED));
    TextOutA(hdc, c2X + 8, curY + 26, purTxt, (int)strlen(purTxt));
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, c2X + 8, curY + 42, statusTxt, (int)strlen(statusTxt));

    COLORREF purCol = (g_state.waterPurity >= 80.0f) ? COL_GREEN : ((g_state.waterPurity >= 60.0f) ? COL_AMBER : COL_RED);
    DrawProgressBar(hdc, c2X + 8, curY + 58, topCardW - 16, 8, g_state.waterPurity / 100.0f, purCol);

    int canFlushFilters = (g_state.scrap >= 10.0f);
    DrawButtonControl(hdc, hFontSmall, c2X + 8, curY + 76, topCardW - 16, 28, "CARBON FILTER FLUSH (10S)", canFlushFilters ? COL_GREEN : COL_TEXT_DIM, canFlushFilters ? RGB(25, 45, 30) : COL_DARK_CARD, canFlushFilters ? COL_GREEN : COL_BORDER, BTN_FLUSH_FILTERS, 0, 0);

    int canSterilize = (g_state.meds >= 1);
    DrawButtonControl(hdc, hFontSmall, c2X + 8, curY + 110, topCardW - 16, 28, "STERILIZE RESERVOIR (1M)", canSterilize ? RGB(100, 200, 255) : COL_TEXT_DIM, canSterilize ? RGB(20, 35, 45) : COL_DARK_CARD, canSterilize ? RGB(100, 200, 255) : COL_BORDER, BTN_STERILIZE_WATER, 0, 0);

    // 3. Morale & Civil Order Card
    int c3X = x + (topCardW + 6) * 2;
    DrawStyledBox(hdc, c3X, curY, topCardW, topCardH, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, c3X + 8, curY + 6, "3. MORALE & CIVIL ORDER", 23);

    char morTxt[64], unrestTxt[64];
    sprintf(morTxt, "Citizen Morale: %.0f%%", g_state.morale);
    const char* uRisk = g_state.martialLaw ? "CURFEW (Suppressed)" : ((g_state.morale < 25.0f) ? "MUTINY IMMINENT" : ((g_state.morale < 45.0f) ? "STRIKE / SABOTAGE" : "Nominal (0% Risk)"));
    sprintf(unrestTxt, "Civil Order: %s", uRisk);

    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, (g_state.morale >= 70.0f) ? COL_GREEN : ((g_state.morale >= 45.0f) ? COL_AMBER : COL_RED));
    TextOutA(hdc, c3X + 8, curY + 26, morTxt, (int)strlen(morTxt));
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, c3X + 8, curY + 42, unrestTxt, (int)strlen(unrestTxt));

    COLORREF mCol = (g_state.morale >= 70.0f) ? COL_GREEN : ((g_state.morale >= 45.0f) ? COL_AMBER : COL_RED);
    DrawProgressBar(hdc, c3X + 8, curY + 58, topCardW - 16, 8, g_state.morale / 100.0f, mCol);

    int btnHalfW = (topCardW - 20) / 2;
    int canFeast = (g_state.food >= 15.0f);
    DrawButtonControl(hdc, hFontSmall, c3X + 8, curY + 76, btnHalfW, 28, "FEAST (15F)", canFeast ? COL_AMBER : COL_TEXT_DIM, canFeast ? RGB(45, 40, 20) : COL_DARK_CARD, canFeast ? COL_AMBER : COL_BORDER, BTN_COMMUNAL_FEAST, 0, 0);

    int canLux = (g_state.scrap >= 20.0f);
    DrawButtonControl(hdc, hFontSmall, c3X + 8 + btnHalfW + 4, curY + 76, btnHalfW, 28, "LUXURIES (20S)", canLux ? COL_GREEN : COL_TEXT_DIM, canLux ? RGB(25, 45, 30) : COL_DARK_CARD, canLux ? COL_GREEN : COL_BORDER, BTN_DIST_LUXURIES, 0, 0);

    int canAddress = (g_state.addressCooldown <= 0);
    char addrBuf[24];
    sprintf(addrBuf, (g_state.addressCooldown > 0) ? "SPEECH (%dd)" : "ADDRESS (+6M)", g_state.addressCooldown);
    DrawButtonControl(hdc, hFontSmall, c3X + 8, curY + 110, btnHalfW, 28, addrBuf, canAddress ? COL_TEXT_BRIGHT : COL_TEXT_DIM, COL_BTN_BG, COL_BORDER, BTN_OVERSEER_ADDRESS, 0, 0);

    DrawButtonControl(hdc, hFontSmall, c3X + 8 + btnHalfW + 4, curY + 110, btnHalfW, 28, g_state.martialLaw ? "LIFT LAW" : "MARTIAL LAW", g_state.martialLaw ? RGB(255, 255, 255) : COL_RED, g_state.martialLaw ? COL_RED : RGB(45, 20, 20), COL_RED, BTN_TOGGLE_MARTIAL_LAW, 0, 0);

    curY += topCardH + 10;

    // --- BOTTOM SECTION: RATIONING POLICIES ---
    // Food Policy (4 options across)
    DrawStyledBox(hdc, x, curY, w, 106, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x + 10, curY + 6, "FOOD RATIONS PROTOCOL", 21);

    // Fortify button right aligned
    int canFort = (g_state.meds >= 1);
    char fortBuf[32];
    sprintf(fortBuf, (g_state.fortifiedRationsDays > 0) ? "FORTIFIED (%dd)" : "FORTIFY RATIONS (1M)", g_state.fortifiedRationsDays);
    DrawButtonControl(hdc, hFontSmall, x + w - 190, curY + 4, 180, 20, fortBuf, canFort ? COL_GREEN : COL_TEXT_DIM, canFort ? RGB(25, 45, 30) : COL_DARK_CARD, canFort ? COL_GREEN : COL_BORDER, BTN_FORTIFY_RATIONS, 0, 0);

    const char* fNames[] = { "Feast Protocol", "Standard Rations", "Half Rations", "Strict Emergency" };
    const char* fDesc1[] = { "1.5x Food/citizen.", "1.0x Food/citizen.", "0.5x Food/citizen.", "0.25x Food/citizen." };
    const char* fDesc2[] = { "+6% Morale, Rapid heal.", "Normal baseline.", "-4% Morale, saves food.", "-12% Morale, Starvation!" };

    int fOptW = (w - 32) / 4;
    for (int i = 0; i < 4; i++) {
        int bx = x + 10 + i * (fOptW + 4);
        int by = curY + 28;
        int active = (g_state.policyFood == i);
        COLORREF bg = active ? COL_BTN_HOVER : COL_PANEL_BG;
        COLORREF bdr = active ? COL_BORDER_HI : COL_BORDER;

        DrawStyledBox(hdc, bx, by, fOptW, 70, bg, bdr);
        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, active ? COL_TEXT_BRIGHT : COL_TEXT_MAIN);
        TextOutA(hdc, bx + 6, by + 4, fNames[i], (int)strlen(fNames[i]));

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COL_TEXT_DIM);
        TextOutA(hdc, bx + 6, by + 22, fDesc1[i], (int)strlen(fDesc1[i]));
        TextOutA(hdc, bx + 6, by + 38, fDesc2[i], (int)strlen(fDesc2[i]));

        AddButton(bx, by, fOptW, 70, BTN_POLICY_FOOD, i, 0);
    }
    curY += 114;

    // Water Policy (3 options across)
    int botHalfW = (w - 6) / 2;
    DrawStyledBox(hdc, x, curY, botHalfW, 100, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x + 10, curY + 6, "WATER CONSERVATION PROTOCOL", 27);

    const char* wNames[] = { "Full Pure", "Strict Rationing", "Recycled Silt" };
    const char* wDesc1[] = { "1.0x Water (+2% Morale)", "0.5x Water (-3% Morale)", "0.25x Water (+8 Rads!)" };
    int wOptW = (botHalfW - 20) / 3;
    for (int i = 0; i < 3; i++) {
        int bx = x + 10 + i * (wOptW + 4);
        int by = curY + 26;
        int active = (g_state.policyWater == i);
        COLORREF bg = active ? COL_BTN_HOVER : COL_PANEL_BG;
        COLORREF bdr = active ? COL_BORDER_HI : COL_BORDER;

        DrawStyledBox(hdc, bx, by, wOptW, 66, bg, bdr);
        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, active ? COL_TEXT_BRIGHT : COL_TEXT_MAIN);
        TextOutA(hdc, bx + 4, by + 4, wNames[i], (int)strlen(wNames[i]));

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COL_TEXT_DIM);
        TextOutA(hdc, bx + 4, by + 22, wDesc1[i], (int)strlen(wDesc1[i]));

        AddButton(bx, by, wOptW, 66, BTN_POLICY_WATER, i, 0);
    }

    // Power Grid Priority (3 options across)
    int pColX = x + botHalfW + 6;
    DrawStyledBox(hdc, pColX, curY, botHalfW, 100, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, pColX + 10, curY + 6, "REACTOR POWER GRID PRIORITY", 27);

    const char* pNames[] = { "Balanced Grid", "Life Support", "Production Priority" };
    const char* pDesc1[] = { "Standard distribution.", "Infirmary & Quarters.", "Farms & Purifiers on." };
    int pOptW = (botHalfW - 20) / 3;
    for (int i = 0; i < 3; i++) {
        int bx = pColX + 10 + i * (pOptW + 4);
        int by = curY + 26;
        int active = (g_state.policyPower == i);
        COLORREF bg = active ? COL_BTN_HOVER : COL_PANEL_BG;
        COLORREF bdr = active ? COL_BORDER_HI : COL_BORDER;

        DrawStyledBox(hdc, bx, by, pOptW, 66, bg, bdr);
        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, active ? COL_TEXT_BRIGHT : COL_TEXT_MAIN);
        TextOutA(hdc, bx + 4, by + 4, pNames[i], (int)strlen(pNames[i]));

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COL_TEXT_DIM);
        TextOutA(hdc, bx + 4, by + 22, pDesc1[i], (int)strlen(pDesc1[i]));

        AddButton(bx, by, pOptW, 66, BTN_POLICY_POWER, i, 0);
    }
}

static void DrawManualView(HDC hdc, HFONT hFontBold, HFONT hFontSmall, int x, int y, int w, int h) {
    DrawStyledBox(hdc, x, y, w, h, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x + 12, y + 10, "OVERSEER SURVIVAL HANDBOOK & DIRECTIVES", 39);

    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_MAIN);
    int curY = y + 36;
    int lineH = 17;

    TextOutA(hdc, x + 12, curY, "WELCOME, OVERSEER. Your mission is to shepherd Vault 704 through nuclear fallout.", 80); curY += lineH + 4;
    
    SetTextColor(hdc, COL_AMBER);
    TextOutA(hdc, x + 12, curY, "1. RESOURCE MANAGEMENT:", 23); curY += lineH;
    SetTextColor(hdc, COL_TEXT_MAIN);
    TextOutA(hdc, x + 20, curY, "* Food & Water: Citizens consume rations each day cycle based on Overseer Directives.", 85); curY += lineH;
    TextOutA(hdc, x + 20, curY, "* Power Grid: Facilities draw wattage. Insufficient power triggers brownouts in non-essentials.", 95); curY += lineH;
    TextOutA(hdc, x + 20, curY, "* Scrap / Tech: Essential for expansion, crafting, and emergency repairs.", 73); curY += lineH + 6;

    SetTextColor(hdc, COL_AMBER);
    TextOutA(hdc, x + 12, curY, "2. WORKFORCE ASSIGNMENT:", 24); curY += lineH;
    SetTextColor(hdc, COL_TEXT_MAIN);
    TextOutA(hdc, x + 20, curY, "* Staff Hydroponics, Purifiers, and Bio-Generators with workers to dramatically boost output.", 92); curY += lineH;
    TextOutA(hdc, x + 20, curY, "* Click [-] and [+] on the Facilities tab, or click the job button in Survivor Roster to reassign.", 98); curY += lineH + 6;

    SetTextColor(hdc, COL_AMBER);
    TextOutA(hdc, x + 12, curY, "3. EXPEDITIONS & SURFACE SCAVENGING:", 36); curY += lineH;
    SetTextColor(hdc, COL_TEXT_MAIN);
    TextOutA(hdc, x + 20, curY, "* Dispatch idle survivors into ruins to forage for emergency food stockpiles and scrap caches.", 94); curY += lineH;
    TextOutA(hdc, x + 20, curY, "* Higher risk expeditions offer larger payouts but take longer to return.", 72); curY += lineH + 6;

    SetTextColor(hdc, COL_AMBER);
    TextOutA(hdc, x + 12, curY, "4. RETRO-TERMINAL CRT THEMES & SCANLINES:", 41); curY += lineH;
    SetTextColor(hdc, COL_TEXT_MAIN);
    TextOutA(hdc, x + 20, curY, "* 4 Color Schemes: Amber CRT (P3), Wasteland Rust, Phosphor Green (P1), and Monochrome (P4).", 92); curY += lineH;
    TextOutA(hdc, x + 20, curY, "* Click THEME or press [T] to cycle. Click CRT or press [C] to toggle raster scanlines.", 87); curY += lineH + 6;

    SetTextColor(hdc, COL_AMBER);
    TextOutA(hdc, x + 12, curY, "5. ROOM CONSTRUCTION & FACILITY EXPANSION:", 42); curY += lineH;
    SetTextColor(hdc, COL_TEXT_MAIN);
    TextOutA(hdc, x + 20, curY, "* Upgrade Facilities: Click [UPG] to upgrade rooms up to Lv 3, boosting output & worker slots.", 94); curY += lineH;
    TextOutA(hdc, x + 20, curY, "* Construct Rooms: Switch to [CONSTRUCT ROOMS] subtab to excavate new generators, wells, farms.", 95); curY += lineH + 6;

    SetTextColor(hdc, COL_AMBER);
    TextOutA(hdc, x + 12, curY, "6. WASTELAND EXPLORATION, MEDS & BLUEPRINT DISCOVERY:", 53); curY += lineH;
    SetTextColor(hdc, COL_TEXT_MAIN);
    TextOutA(hdc, x + 20, curY, "* 5 Ruins: Explore Supermarket, Substation, Hospital, Armory, and Vault 811 Tech Archive.", 88); curY += lineH;
    TextOutA(hdc, x + 20, curY, "* Scout Attributes: STR lowers damage, AGI boosts forage yields, INT decrypts lost blueprints.", 93); curY += lineH;
    TextOutA(hdc, x + 20, curY, "* Medical Triage: Use Meds on wounded citizens (+35 HP) or equip Stimpacks before expeditions.", 94); curY += lineH + 4;

    SetTextColor(hdc, COL_AMBER);
    TextOutA(hdc, x + 12, curY, "7. BASE DEFENSE & RAIDER WARBANDS:", 34); curY += lineH;
    SetTextColor(hdc, COL_TEXT_MAIN);
    TextOutA(hdc, x + 20, curY, "* Barricades & Sentry Turrets: Blast-doors absorb trauma; twin 50-cal turrets deal suppressive fire.", 99); curY += lineH;
    TextOutA(hdc, x + 20, curY, "* Drills & Guards: Assign high-STR survivors as guards; run combat drills to boost defense.", 90); curY += lineH;
    TextOutA(hdc, x + 20, curY, "* Raider Assaults: Surface radar tracks raiders. Exceeding attack power repels raiders & salvages loot!", 102); curY += lineH + 4;

    SetTextColor(hdc, COL_AMBER);
    TextOutA(hdc, x + 12, curY, "8. TECHNOLOGY RESEARCH & UPGRADE BLUEPRINTS:", 44); curY += lineH;
    SetTextColor(hdc, COL_TEXT_MAIN);
    TextOutA(hdc, x + 20, curY, "* Research Tree: Invest scrap in [5] RESEARCH to unlock advanced water filtration, hydroponics,", 95); curY += lineH;
    TextOutA(hdc, x + 20, curY, "  photovoltaic solar arrays, reinforced blast defenses, sensor radars, and stimpacks.", 85); curY += lineH + 4;

    SetTextColor(hdc, COL_AMBER);
    TextOutA(hdc, x + 12, curY, "9. ENVIRONMENTAL DISASTERS & WEATHER HAZARDS (PHASE 10):", 56); curY += lineH;
    SetTextColor(hdc, COL_TEXT_MAIN);
    TextOutA(hdc, x + 20, curY, "* Radiation Storms: Surges rads to 15+ Rads/h. Engage Airlock Rad-Bulkheads or research Rad Shield.", 99); curY += lineH;
    TextOutA(hdc, x + 20, curY, "* Droughts: Halves water purifier production and increases thirst. Activate Deep Aquifer Overpump.", 98); curY += lineH;
    TextOutA(hdc, x + 20, curY, "* Acid Rain: Corrodes blast barricades (-30 HP/d). Spray Alkaline Corrosion Neutralizer wash.", 93); curY += lineH;
    TextOutA(hdc, x + 20, curY, "* Cold Snaps: Freezes bio-generators (-35% Gen). Engage Auxiliary Thermal Overdrive heaters.", 92); curY += lineH + 4;

    SetTextColor(hdc, COL_AMBER);
    TextOutA(hdc, x + 12, curY, "10. WASTELAND CARAVAN TRADING SYSTEM (PHASE 11):", 48); curY += lineH;
    SetTextColor(hdc, COL_TEXT_MAIN);
    TextOutA(hdc, x + 20, curY, "* Nomadic Caravans: 5 merchant factions visit the airlock (Dust Striders, Rust Brotherhood, etc).", 98); curY += lineH;
    TextOutA(hdc, x + 20, curY, "* Barter Economy: Buy food, purified water, stimpacks, and scrap. Sell vault surplus back to traders.", 101); curY += lineH;
    TextOutA(hdc, x + 20, curY, "* Rare Artifacts: Buy Rad-Away casks, Super-Growth enzymes, Armor Plates (+25 Max HP), Targeting chips.", 103); curY += lineH;
    TextOutA(hdc, x + 20, curY, "* Barter Trust & Hailing: Trades raise trust up to 25% discount. Broadcast radio hail to summon traders.", 103); curY += lineH + 4;

    SetTextColor(hdc, COL_AMBER);
    TextOutA(hdc, x + 12, curY, "11. KEYBOARD SHORTCUTS:", 23); curY += lineH;
    SetTextColor(hdc, COL_TEXT_MAIN);
    TextOutA(hdc, x + 20, curY, "[SPACE] Advance Cycle | [1-9] Tabs | [T] Theme | [C] CRT | [A] Auto | [H] Help | [R] Reset", 90);
}

static void DrawSidebar(HDC hdc, HFONT hFontBold, HFONT hFontSmall, int x, int y, int w, int h) {
    // 1. Cycle Controller Box
    int cycleH = 120;
    DrawStyledBox(hdc, x, y, w, cycleH, COL_PANEL_BG, COL_GREEN);

    char dayBuf[32];
    sprintf(dayBuf, "CYCLE: DAY %d", g_state.day);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x + 10, y + 8, dayBuf, (int)strlen(dayBuf));

    const char* phases[] = { "DAWN (06:00)", "MIDDAY (12:00)", "DUSK (18:00)", "NIGHT (24:00)" };
    const char* curPhase = phases[g_state.phase % 4];
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_AMBER);
    TextOutA(hdc, x + w - 95, y + 10, curPhase, (int)strlen(curPhase));

    // Progress bar for day phase
    float progress = (g_state.phase + 1) * 0.25f;
    DrawProgressBar(hdc, x + 10, y + 28, w - 20, 6, progress, COL_GREEN);

    // Big ADVANCE button
    DrawButtonControl(hdc, hFontBold, x + 10, y + 42, w - 20, 36, "ADVANCE DAY CYCLE [SPACE]", RGB(236, 253, 245), RGB(25, 59, 33), COL_GREEN, BTN_ADVANCE, 0, 0);

    // Minor buttons row
    int subBtnW = (w - 28) / 3;
    const char* autoText = g_state.autoRun ? "AUTO: ON" : "AUTO: OFF";
    COLORREF autoCol = g_state.autoRun ? COL_GREEN : COL_TEXT_DIM;
    DrawButtonControl(hdc, hFontSmall, x + 10, y + 86, subBtnW, 24, autoText, autoCol, COL_BTN_BG, COL_BORDER, BTN_AUTORUN, 0, 0);
    DrawButtonControl(hdc, hFontSmall, x + 14 + subBtnW, y + 86, subBtnW, 24, "+3 DAYS", COL_TEXT_MAIN, COL_BTN_BG, COL_BORDER, BTN_FASTFORWARD, 0, 0);
    DrawButtonControl(hdc, hFontSmall, x + 18 + subBtnW * 2, y + 86, subBtnW, 24, "REPORT", COL_TEXT_MAIN, COL_BTN_BG, COL_BORDER, BTN_REPORT, 0, 0);

    // 2. Terminal Log Box
    int logY = y + cycleH + 8;
    int logH = h - cycleH - 8;
    DrawStyledBox(hdc, x, logY, w, logH, COL_PANEL_BG, COL_BORDER);

    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x + 10, logY + 8, "OVERSEER TERMINAL LOG", 21);

    DrawButtonControl(hdc, hFontSmall, x + w - 55, logY + 6, 45, 18, "CLEAR", COL_TEXT_DIM, COL_BTN_BG, COL_BORDER, BTN_CLEARLOG, 0, 0);

    // Messages list
    int logStartY = logY + 30;
    int maxDisplay = (logH - 36) / 18;
    int startIdx = g_state.logCount - maxDisplay;
    if (startIdx < 0) startIdx = 0;

    SelectObject(hdc, hFontSmall);
    for (int i = startIdx; i < g_state.logCount; i++) {
        LogEntry* le = &g_state.logs[i];
        int ly = logStartY + (i - startIdx) * 18;

        COLORREF tColor = COL_TEXT_MAIN;
        if (le->type == 1) tColor = COL_AMBER;
        else if (le->type == 2) tColor = COL_RED;
        else if (le->type == 3) tColor = COL_TEXT_BRIGHT;
        else if (le->type == 4) tColor = COL_CYAN;

        SetTextColor(hdc, tColor);
        char lineBuf[160];
        sprintf(lineBuf, "[D%d P%d] %s", le->day, le->phase + 1, le->text);
        TextOutA(hdc, x + 10, ly, lineBuf, (int)strlen(lineBuf));
    }
}

static void DrawSummaryModal(HDC hdc, HFONT hFontBold, HFONT hFontSmall) {
    // Dim overlay
    FillSolidRect(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, RGB(5, 10, 6));

    int modalW = 480;
    int modalH = 260;
    int mx = (WINDOW_WIDTH - modalW) / 2;
    int my = (WINDOW_HEIGHT - modalH) / 2;

    DrawStyledBox(hdc, mx, my, modalW, modalH, COL_PANEL_BG, COL_GREEN);

    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    char titleBuf[64];
    sprintf(titleBuf, "OVERSEER LOG // CYCLE DAY %d SUMMARY", g_state.summaryDay > 0 ? g_state.summaryDay : g_state.day);
    TextOutA(hdc, mx + 16, my + 14, titleBuf, (int)strlen(titleBuf));

    DrawButtonControl(hdc, hFontBold, mx + modalW - 32, my + 10, 22, 22, "X", COL_TEXT_DIM, COL_PANEL_BG, COL_BORDER, BTN_CLOSE_MODAL, 0, 0);

    // Inner box
    int inX = mx + 16;
    int inY = my + 44;
    int inW = modalW - 32;
    int inH = 150;
    DrawStyledBox(hdc, inX, inY, inW, inH, COL_DARK_CARD, COL_BORDER);

    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_MAIN);
    int lineY = inY + 12;
    char b1[64], b2[64], b3[64], b4[64], b5[64];
    sprintf(b1, "* Population: %d / %d Survivors", g_state.population, g_state.maxPop);
    sprintf(b2, "* Food Balance: %s%.1f / cycle", g_state.sumFoodDelta >= 0 ? "+" : "", g_state.sumFoodDelta);
    sprintf(b3, "* Water Balance: %s%.1f / cycle", g_state.sumWaterDelta >= 0 ? "+" : "", g_state.sumWaterDelta);
    sprintf(b4, "* Power Grid: %d kW Gen / %d kW Demand", g_state.powerGen, g_state.powerLoad);
    sprintf(b5, "* Vault Morale: %d%%", g_state.sumMorale);

    TextOutA(hdc, inX + 12, lineY, b1, (int)strlen(b1)); lineY += 24;
    TextOutA(hdc, inX + 12, lineY, b2, (int)strlen(b2)); lineY += 24;
    TextOutA(hdc, inX + 12, lineY, b3, (int)strlen(b3)); lineY += 24;
    TextOutA(hdc, inX + 12, lineY, b4, (int)strlen(b4)); lineY += 24;
    TextOutA(hdc, inX + 12, lineY, b5, (int)strlen(b5));

    DrawButtonControl(hdc, hFontBold, mx + (modalW - 140) / 2, my + modalH - 42, 140, 28, "CLOSE REPORT", COL_TEXT_BRIGHT, COL_BTN_BG, COL_GREEN, BTN_CLOSE_MODAL, 0, 0);
}

static void DrawRaidModal(HDC hdc, HFONT hFontBold, HFONT hFontSmall) {
    // Dim overlay
    FillSolidRect(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, RGB(5, 10, 6));

    int modalW = 520;
    int modalH = 260;
    int mx = (WINDOW_WIDTH - modalW) / 2;
    int my = (WINDOW_HEIGHT - modalH) / 2;

    COLORREF borderCol = g_state.lastRaidWon ? COL_GREEN : COL_RED;
    DrawStyledBox(hdc, mx, my, modalW, modalH, COL_PANEL_BG, borderCol);

    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, borderCol);
    TextOutA(hdc, mx + 16, my + 14, g_state.lastRaidWon ? "★ BATTLE DEBRIEF // PERIMETER HELD" : "⚠ PERIMETER BREACH // CASUALTY REPORT", g_state.lastRaidWon ? 34 : 37);

    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    char clanBuf[64];
    sprintf(clanBuf, "Enemy: %s (Assault Power: %d pts)", g_state.lastRaidClan, g_state.lastRaidAtk);
    TextOutA(hdc, mx + 16, my + 42, clanBuf, (int)strlen(clanBuf));

    SelectObject(hdc, hFontSmall);
    char defBuf[64];
    sprintf(defBuf, "Vault Total Defense Rating: %d pts", g_state.lastRaidDef);
    SetTextColor(hdc, COL_GREEN);
    TextOutA(hdc, mx + 16, my + 64, defBuf, (int)strlen(defBuf));

    int curY = my + 92;
    SetTextColor(hdc, COL_TEXT_MAIN);
    if (g_state.lastRaidWon) {
        TextOutA(hdc, mx + 16, curY, "TACTICAL VICTORY: Twin sentry turrets and fortified barricades held the line!", 78); curY += 22;
        char lBuf[80];
        sprintf(lBuf, "* Salvaged Loot: +%d Tech Scrap, +%d Medpacks from fallen raiders.", g_state.lastRaidScrap, g_state.lastRaidMeds);
        SetTextColor(hdc, COL_GREEN);
        TextOutA(hdc, mx + 16, curY, lBuf, (int)strlen(lBuf)); curY += 22;
        char bBuf[80];
        sprintf(bBuf, "* Perimeter Impact: Barricades absorbed explosive shrapnel (-%d HP).", g_state.lastRaidDmg);
        SetTextColor(hdc, COL_TEXT_DIM);
        TextOutA(hdc, mx + 16, curY, bBuf, (int)strlen(bBuf));
    } else {
        SetTextColor(hdc, COL_RED);
        TextOutA(hdc, mx + 16, curY, "BREACH COMPROMISE: Raider assault broke through outer blast bulkheads!", 70); curY += 22;
        char stBuf[80];
        sprintf(stBuf, "* Supplies Stolen: -%d Food Rations, -%d Tech Scrap.", g_state.lastRaidFoodStolen, g_state.lastRaidScrapStolen);
        TextOutA(hdc, mx + 16, curY, stBuf, (int)strlen(stBuf)); curY += 22;
        char casBuf[80];
        sprintf(casBuf, "* Casualties: %d citizens wounded. Barricades sustained -%d HP trauma.", g_state.lastRaidInjured, g_state.lastRaidDmg);
        SetTextColor(hdc, COL_AMBER);
        TextOutA(hdc, mx + 16, curY, casBuf, (int)strlen(casBuf));
    }

    DrawButtonControl(hdc, hFontBold, mx + modalW - 200, my + modalH - 40, 180, 26, "ACKNOWLEDGE [OK]", COL_TEXT_BRIGHT, COL_BTN_BG, borderCol, BTN_CLOSE_RAID_MODAL, 0, 0);
}

// Main Window Procedure
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            srand((unsigned int)time(NULL));
            InitGameState();
            break;
        }
        case WM_TIMER: {
            if (wParam == IDT_AUTORUN && g_state.autoRun) {
                AdvanceCycle();
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_KEYDOWN: {
            if (wParam == VK_SPACE) {
                AdvanceCycle();
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam >= '1' && wParam <= '9') {
                g_state.currentTab = (int)(wParam - '1');
                PlaySfx(1);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'A' || wParam == 'a') {
                g_state.autoRun = !g_state.autoRun;
                if (g_state.autoRun) SetTimer(hwnd, IDT_AUTORUN, 1800, NULL);
                else KillTimer(hwnd, IDT_AUTORUN);
                PlaySfx(1);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'H' || wParam == 'h') {
                g_state.currentTab = 8;
                PlaySfx(1);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'T' || wParam == 't') {
                g_currentTheme = (g_currentTheme + 1) % THEME_COUNT;
                PlaySfx(SFX_CLICK);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'C' || wParam == 'c') {
                g_crtScanlines = !g_crtScanlines;
                PlaySfx(SFX_CLICK);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'S' || wParam == 's') {
                g_soundEnabled = !g_soundEnabled;
                PlaySfx(SFX_CLICK);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'W' || wParam == 'w') {
                PlaySfx(SFX_WIND);
            } else if (wParam == 'R' || wParam == 'r') {
                InitGameState();
                PlaySfx(SFX_ALERT);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);

            // Check buttons
            for (int i = 0; i < g_buttonCount; i++) {
                RECT r = g_buttons[i].rect;
                if (mx >= r.left && mx <= r.right && my >= r.top && my <= r.bottom) {
                    int bId = g_buttons[i].id;
                    int p1 = g_buttons[i].param1;
                    int p2 = g_buttons[i].param2;

                    if (bId == BTN_THEME) {
                        g_currentTheme = (g_currentTheme + 1) % THEME_COUNT;
                        PlaySfx(1);
                    } else if (bId == BTN_CRT) {
                        g_crtScanlines = !g_crtScanlines;
                        PlaySfx(1);
                    } else if (bId == BTN_AUDIO) {
                        g_soundEnabled = !g_soundEnabled;
                        PlaySfx(1);
                    } else if (bId == BTN_HELP) {
                        g_state.currentTab = 8;
                        PlaySfx(1);
                    } else if (bId == BTN_RESET) {
                        if (MessageBoxA(hwnd, "Initiate Vault Emergency Reboot? All progress resets to Day 1.", "Reset Sanctuary", MB_YESNO | MB_ICONWARNING) == IDYES) {
                            InitGameState();
                            PlaySfx(3);
                        }
                    } else if (bId == BTN_TAB) {
                        g_state.currentTab = p1;
                        PlaySfx(1);
                    } else if (bId == BTN_ADVANCE) {
                        AdvanceCycle();
                    } else if (bId == BTN_AUTORUN) {
                        g_state.autoRun = !g_state.autoRun;
                        if (g_state.autoRun) SetTimer(hwnd, IDT_AUTORUN, 1800, NULL);
                        else KillTimer(hwnd, IDT_AUTORUN);
                        PlaySfx(1);
                    } else if (bId == BTN_FASTFORWARD) {
                        for (int f = 0; f < 3; f++) ProcessNewDay();
                    } else if (bId == BTN_REPORT) {
                        g_state.showSummary = 1;
                        PlaySfx(1);
                    } else if (bId == BTN_CLEARLOG) {
                        g_state.logCount = 0;
                        PlaySfx(1);
                    } else if (bId == BTN_FAC_WORKER) {
                        Facility* fac = &g_state.facilities[p1];
                        if (p2 > 0) { // Add
                            Survivor* surv = GetFirstUnassignedSurvivor();
                            if (surv && fac->assigned < fac->maxWorkers) {
                                strcpy(surv->job, fac->id);
                                fac->assigned++;
                                char buf[128];
                                sprintf(buf, "%s assigned to %s.", surv->name, fac->name);
                                AddLog(buf, 0);
                                PlaySfx(1);
                            }
                        } else { // Remove
                            if (fac->assigned > 0) {
                                for (int s = 0; s < g_state.numSurvivors; s++) {
                                    if (strcmp(g_state.survivors[s].job, fac->id) == 0) {
                                        strcpy(g_state.survivors[s].job, "unassigned");
                                        break;
                                    }
                                }
                                fac->assigned--;
                                char buf[128];
                                sprintf(buf, "Worker removed from %s.", fac->name);
                                AddLog(buf, 0);
                                PlaySfx(1);
                            }
                        }
                    } else if (bId == BTN_FAC_SUBTAB) {
                        g_state.facilitySubTab = p1;
                        PlaySfx(1);
                    } else if (bId == BTN_FAC_UPGRADE) {
                        Facility* fac = &g_state.facilities[p1];
                        int baseCost = fac->level * 30;
                        int uCost = GetEffectiveScrapCost(baseCost);
                        if (g_state.scrap >= (float)uCost && fac->level < 3) {
                            g_state.scrap -= (float)uCost;
                            fac->level++;
                            char bonusMsg[48] = "";
                            if (fac->powerProd > 0) {
                                fac->powerProd += 10;
                                fac->maxWorkers++;
                                strcpy(bonusMsg, "+10 kW & +1 staff slot");
                            } else if (fac->foodProd > 0) {
                                fac->foodProd += 3;
                                fac->maxWorkers++;
                                strcpy(bonusMsg, "+3 Food & +1 staff slot");
                            } else if (fac->waterProd > 0) {
                                fac->waterProd += 3;
                                fac->maxWorkers++;
                                strcpy(bonusMsg, "+3 Water & +1 staff slot");
                            } else if (fac->scrapProd > 0) {
                                fac->scrapProd += 2;
                                fac->maxWorkers++;
                                strcpy(bonusMsg, "+2 Scrap & +1 staff slot");
                            } else if (strcmp(fac->id, "quarters") == 0 || strcmp(fac->id, "quarters_ext") == 0) {
                                g_state.maxPop += 4;
                                strcpy(bonusMsg, "+4 Vault Capacity");
                            } else if (strcmp(fac->id, "security") == 0) {
                                g_state.defense += 10;
                                strcpy(bonusMsg, "+10 Defense Rating");
                            } else if (strcmp(fac->id, "cmd") == 0) {
                                g_state.defense += 5;
                                strcpy(bonusMsg, "+5 Sensor Defense");
                            } else if (strcmp(fac->id, "infirmary") == 0) {
                                strcpy(bonusMsg, "Faster Medical Recovery");
                            }
                            char buf[128];
                            sprintf(buf, "UPGRADE: %s upgraded to Level %d! (%s)", fac->name, fac->level, bonusMsg);
                            AddLog(buf, 3);
                            PlaySfx(SFX_CONSTRUCTION);
                        } else {
                            PlaySfx(SFX_ALERT);
                        }
                    } else if (bId == BTN_CONSTRUCT_ROOM) {
                        RoomBlueprint* bp = &g_state.blueprints[p1];
                        int cost = GetEffectiveScrapCost(bp->cost);
                        if (g_state.scrap >= (float)cost && !bp->built && !bp->locked && g_state.numFacilities < 16) {
                            g_state.scrap -= (float)cost;
                            bp->built = 1;
                            Facility* nf = &g_state.facilities[g_state.numFacilities++];
                            strcpy(nf->id, bp->id);
                            strcpy(nf->name, bp->name);
                            strcpy(nf->desc, bp->desc);
                            nf->level = 1;
                            nf->maxWorkers = bp->maxWorkers;
                            nf->assigned = 0;
                            nf->powerCost = bp->powerCost;
                            nf->powerProd = bp->powerProd;
                            nf->foodProd = bp->foodProd;
                            nf->waterProd = bp->waterProd;
                            nf->scrapProd = bp->scrapProd;
                            nf->powerPriority = bp->powerPriority;

                            if (bp->popBoost > 0) g_state.maxPop += bp->popBoost;
                            if (bp->defenseBoost > 0) g_state.defense += bp->defenseBoost;

                            char buf[128];
                            sprintf(buf, "CONSTRUCTION: %s excavated and brought online!", bp->name);
                            AddLog(buf, 3);
                            PlaySfx(SFX_CONSTRUCTION);
                        } else {
                            PlaySfx(SFX_ALERT);
                        }
                    } else if (bId == BTN_SURV_JOB) {
                        Survivor* surv = &g_state.survivors[p1];
                        // Cycle jobs through available facilities
                        // Current job index:
                        int curIdx = -1;
                        for (int f = 0; f < g_state.numFacilities; f++) {
                            if (strcmp(g_state.facilities[f].id, surv->job) == 0) {
                                curIdx = f;
                                break;
                            }
                        }
                        // Unassign from old
                        if (curIdx >= 0 && g_state.facilities[curIdx].assigned > 0) {
                            g_state.facilities[curIdx].assigned--;
                        }
                        // Try next facilities
                        int assigned = 0;
                        for (int step = 1; step <= g_state.numFacilities; step++) {
                            int nextIdx = (curIdx + step) % (g_state.numFacilities + 1);
                            if (nextIdx == g_state.numFacilities) {
                                // unassigned
                                strcpy(surv->job, "unassigned");
                                assigned = 1;
                                break;
                            } else {
                                Facility* nfac = &g_state.facilities[nextIdx];
                                if (nfac->maxWorkers > 0 && nfac->assigned < nfac->maxWorkers) {
                                    nfac->assigned++;
                                    strcpy(surv->job, nfac->id);
                                    assigned = 1;
                                    break;
                                }
                            }
                        }
                        if (!assigned) strcpy(surv->job, "unassigned");
                        PlaySfx(SFX_CLICK);
                    } else if (bId == BTN_SURV_SUBTAB) {
                        g_state.survivorSubTab = p1;
                        PlaySfx(SFX_CLICK);
                    } else if (bId == BTN_BROADCAST_PING) {
                        if (g_state.scrap >= 15 && g_state.powerGen >= 10) {
                            g_state.scrap -= 15;
                            int count = (rand() % 100 > 40) ? 2 : 1;
                            for (int k = 0; k < count; k++) {
                                if (g_state.numCandidates < MAX_CANDIDATES) {
                                    GenerateCandidate(&g_state.candidates[g_state.numCandidates++], 0);
                                }
                            }
                            char buf[128];
                            sprintf(buf, "RADIO: Emergency signal ping attracted %d refugee(s) to the airlock!", count);
                            AddLog(buf, 4);
                            PlaySfx(SFX_SCOUT);
                        } else {
                            AddLog("RADIO: Insufficient scrap (15) or reactor power (10 kW) to broadcast ping!", 1);
                            PlaySfx(SFX_ALERT);
                        }
                    } else if (bId == BTN_BROADCAST_SPEC) {
                        if (g_state.scrap >= 25 && g_state.food >= 10.0f) {
                            g_state.scrap -= 25;
                            g_state.food -= 10.0f;
                            if (g_state.numCandidates < MAX_CANDIDATES) {
                                Candidate* cand = &g_state.candidates[g_state.numCandidates++];
                                GenerateCandidate(cand, 1);
                                char buf[128];
                                sprintf(buf, "RADIO: Directional beacon attracted specialist %s (%s) to the airlock!", cand->name, cand->role);
                                AddLog(buf, 3);
                                PlaySfx(SFX_CYCLE);
                            }
                        } else {
                            AddLog("RADIO: Insufficient scrap (25) or food (10) for specialist beacon!", 1);
                            PlaySfx(SFX_ALERT);
                        }
                    } else if (bId == BTN_ADMIT_CANDIDATE) {
                        if (g_state.population < g_state.maxPop && g_state.numSurvivors < MAX_SURVIVORS && p1 >= 0 && p1 < g_state.numCandidates) {
                            Candidate* c = &g_state.candidates[p1];
                            Survivor* s = &g_state.survivors[g_state.numSurvivors++];
                            sprintf(s->id, "s_%d", (int)time(NULL) % 10000 + g_state.numSurvivors);
                            strcpy(s->name, c->name);
                            strcpy(s->role, c->role);
                            strcpy(s->job, "unassigned");
                            s->health = c->health;
                            s->morale = c->morale;
                            s->hunger = 0;
                            s->thirst = 0;
                            s->str = c->str;
                            s->agi = c->agi;
                            s->inte = c->inte;
                            g_state.population = g_state.numSurvivors;

                            char buf[128];
                            sprintf(buf, "AIRLOCK: Clearance granted! %s (%s) admitted to the vault.", c->name, c->role);
                            AddLog(buf, 3);
                            PlaySfx(SFX_CYCLE);

                            for (int k = p1; k < g_state.numCandidates - 1; k++) {
                                g_state.candidates[k] = g_state.candidates[k + 1];
                            }
                            g_state.numCandidates--;
                        } else {
                            AddLog("AIRLOCK: Quarters are full! Expand living barracks first.", 1);
                            PlaySfx(SFX_ALERT);
                        }
                    } else if (bId == BTN_DISMISS_CANDIDATE) {
                        if (p1 >= 0 && p1 < g_state.numCandidates) {
                            char buf[128];
                            sprintf(buf, "AIRLOCK: Clearance denied. %s was turned away into the wastes.", g_state.candidates[p1].name);
                            AddLog(buf, 0);
                            PlaySfx(SFX_CLICK);

                            for (int k = p1; k < g_state.numCandidates - 1; k++) {
                                g_state.candidates[k] = g_state.candidates[k + 1];
                            }
                            g_state.numCandidates--;
                        }
                    } else if (bId == BTN_DISPATCH_SCOUT) {
                        Expedition* exp = &g_state.expeditions[p1];
                        Survivor* scout = GetFirstUnassignedSurvivor();
                        if (scout && strlen(exp->assignedScout) == 0) {
                            if (exp->hasStimpack) {
                                if (g_state.meds > 0) {
                                    g_state.meds--;
                                } else {
                                    exp->hasStimpack = 0;
                                }
                            }
                            strcpy(scout->job, "expedition");
                            strcpy(exp->assignedScout, scout->id);
                            exp->daysRemaining = exp->duration;
                            char buf[128];
                            sprintf(buf, "Scout %s dispatched to %s%s.", scout->name, exp->name, exp->hasStimpack ? " (Stimpack equipped)" : "");
                            AddLog(buf, 4);
                            PlaySfx(SFX_SCOUT);
                        }
                    } else if (bId == BTN_EXP_STIM_TOGGLE) {
                        Expedition* exp = &g_state.expeditions[p1];
                        if (strlen(exp->assignedScout) == 0) {
                            if (!exp->hasStimpack) {
                                if (g_state.meds > 0) {
                                    exp->hasStimpack = 1;
                                    PlaySfx(SFX_CLICK);
                                } else {
                                    AddLog("No medical supplies in vault stock to equip Stimpack!", 1);
                                    PlaySfx(SFX_ALERT);
                                }
                            } else {
                                exp->hasStimpack = 0;
                                PlaySfx(SFX_CLICK);
                            }
                        }
                    } else if (bId == BTN_TREAT_SURV) {
                        if (p1 >= 0 && p1 < g_state.numSurvivors && g_state.meds > 0) {
                            Survivor* surv = &g_state.survivors[p1];
                            if (surv->health < 100) {
                                g_state.meds--;
                                surv->health += 35;
                                if (surv->health > 100) surv->health = 100;
                                surv->morale += 10;
                                if (surv->morale > 100) surv->morale = 100;
                                char buf[128];
                                sprintf(buf, "MEDICAL: Stimpack administered to %s. Health restored to %d%%.", surv->name, surv->health);
                                AddLog(buf, 3);
                                PlaySfx(SFX_MEDS);
                            }
                        }
                    } else if (bId == BTN_TREAT_RADAWAY) {
                        if (p1 >= 0 && p1 < g_state.numSurvivors && g_state.meds > 0) {
                            Survivor* surv = &g_state.survivors[p1];
                            if (surv->rads > 0) {
                                g_state.meds--;
                                surv->rads -= 45;
                                if (surv->rads < 0) surv->rads = 0;
                                surv->health += 10;
                                if (surv->health > 100) surv->health = 100;
                                surv->morale += 6;
                                if (surv->morale > 100) surv->morale = 100;
                                char buf[128];
                                sprintf(buf, "RAD-AWAY: Anti-rad chelating agent dosed to %s (%d Rads remaining).", surv->name, surv->rads);
                                AddLog(buf, 3);
                                PlaySfx(SFX_GEIGER);
                            }
                        }
                    } else if (bId == BTN_TREAT_DECON) {
                        if (p1 >= 0 && p1 < g_state.numSurvivors && g_state.water >= 5.0f && g_state.scrap >= 5.0f) {
                            Survivor* surv = &g_state.survivors[p1];
                            g_state.water -= 5.0f;
                            g_state.scrap -= 5.0f;
                            surv->rads -= 25;
                            if (surv->rads < 0) surv->rads = 0;
                            surv->morale += 3;
                            if (surv->morale > 100) surv->morale = 100;
                            char buf[128];
                            sprintf(buf, "DECON SHOWER: %s cycled through isotope wash (%d Rads remaining).", surv->name, surv->rads);
                            AddLog(buf, 0);
                            PlaySfx(SFX_MEDS);
                        }
                    } else if (bId == BTN_MASS_RADAWAY) {
                        if (g_state.meds >= 2 && g_state.water >= 10.0f) {
                            g_state.meds -= 2;
                            g_state.water -= 10.0f;
                            int treated = 0;
                            for (int s = 0; s < g_state.numSurvivors; s++) {
                                if (g_state.survivors[s].rads > 0) {
                                    g_state.survivors[s].rads -= 30;
                                    if (g_state.survivors[s].rads < 0) g_state.survivors[s].rads = 0;
                                    g_state.survivors[s].health += 10;
                                    if (g_state.survivors[s].health > 100) g_state.survivors[s].health = 100;
                                    g_state.survivors[s].morale += 4;
                                    if (g_state.survivors[s].morale > 100) g_state.survivors[s].morale = 100;
                                    treated++;
                                }
                            }
                            char buf[128];
                            sprintf(buf, "MASS RADAWAY: Distributed anti-rad casks! %d irradiated dwellers treated (-30 Rads).", treated);
                            AddLog(buf, 3);
                            PlaySfx(SFX_GEIGER);
                        }
                    } else if (bId == BTN_MASS_DECON) {
                        if (g_state.water >= 15.0f && g_state.scrap >= 15.0f) {
                            g_state.water -= 15.0f;
                            g_state.scrap -= 15.0f;
                            for (int s = 0; s < g_state.numSurvivors; s++) {
                                g_state.survivors[s].rads -= 20;
                                if (g_state.survivors[s].rads < 0) g_state.survivors[s].rads = 0;
                                g_state.survivors[s].morale += 3;
                                if (g_state.survivors[s].morale > 100) g_state.survivors[s].morale = 100;
                            }
                            AddLog("DECON FLUSH: High-pressure saline de-ionizing wash cycled through all airlocks and bunks.", 0);
                            PlaySfx(SFX_MEDS);
                        }
                    } else if (bId == BTN_FLUSH_FILTERS) {
                        if (g_state.scrap >= 10.0f) {
                            g_state.scrap -= 10.0f;
                            g_state.waterPurity += 30.0f;
                            if (g_state.waterPurity > 100.0f) g_state.waterPurity = 100.0f;
                            char buf[128];
                            sprintf(buf, "FILTRATION FLUSH: Replaced activated charcoal filters. Aquifer purity: %.0f%%.", g_state.waterPurity);
                            AddLog(buf, 3);
                            PlaySfx(SFX_MEDS);
                        }
                    } else if (bId == BTN_STERILIZE_WATER) {
                        if (g_state.meds >= 1) {
                            g_state.meds -= 1;
                            g_state.waterPurity = 100.0f;
                            AddLog("STERILIZATION: Dosed medical iodine into reservoirs. Purity restored to 100% (Sterile).", 3);
                            PlaySfx(SFX_MEDS);
                        }
                    } else if (bId == BTN_FORTIFY_RATIONS) {
                        if (g_state.meds >= 1) {
                            g_state.meds -= 1;
                            g_state.fortifiedRationsDays = 3;
                            AddLog("NUTRITIONAL DIRECTIVE: Enriched vitamins blended into rations (+15% Worker Efficiency for 3 cycles).", 3);
                            PlaySfx(SFX_MEDS);
                        }
                    } else if (bId == BTN_COMMUNAL_FEAST) {
                        if (g_state.food >= 15.0f) {
                            g_state.food -= 15.0f;
                            g_state.communalFeastDays = 3;
                            g_state.morale += 15.0f;
                            if (g_state.morale > 100.0f) g_state.morale = 100.0f;
                            for (int s = 0; s < g_state.numSurvivors; s++) {
                                g_state.survivors[s].morale += 15;
                                if (g_state.survivors[s].morale > 100) g_state.survivors[s].morale = 100;
                                g_state.survivors[s].hunger = 0;
                            }
                            AddLog("COMMUNAL FEAST: Overseer hosted a vault feast (+15% Morale, Unrest suppressed)!", 3);
                            PlaySfx(SFX_CYCLE);
                        }
                    } else if (bId == BTN_DIST_LUXURIES) {
                        if (g_state.scrap >= 20.0f) {
                            g_state.scrap -= 20.0f;
                            g_state.morale += 12.0f;
                            if (g_state.morale > 100.0f) g_state.morale = 100.0f;
                            for (int s = 0; s < g_state.numSurvivors; s++) {
                                g_state.survivors[s].morale += 12;
                                if (g_state.survivors[s].morale > 100) g_state.survivors[s].morale = 100;
                            }
                            AddLog("LUXURY BROADCAST: Distributed comfort goods & broadcasted radio orchestra (+12% Morale).", 0);
                            PlaySfx(SFX_TRADE);
                        }
                    } else if (bId == BTN_OVERSEER_ADDRESS) {
                        if (g_state.addressCooldown <= 0) {
                            g_state.addressCooldown = 3;
                            g_state.morale += 6.0f;
                            if (g_state.morale > 100.0f) g_state.morale = 100.0f;
                            for (int s = 0; s < g_state.numSurvivors; s++) {
                                g_state.survivors[s].morale += 6;
                                if (g_state.survivors[s].morale > 100) g_state.survivors[s].morale = 100;
                            }
                            AddLog("OVERSEER ADDRESS: Broadcasted an inspiring survival speech over vault PA (+6% Morale).", 0);
                            PlaySfx(SFX_CLICK);
                        }
                    } else if (bId == BTN_TOGGLE_MARTIAL_LAW) {
                        g_state.martialLaw = !g_state.martialLaw;
                        if (g_state.martialLaw) {
                            if (g_state.morale > 50.0f) g_state.morale = 50.0f;
                            AddLog("SECURITY DIRECTIVE: MARTIAL LAW DECLARED. Armed patrols enforce curfew. Strikes & sabotage halted.", 2);
                            PlaySfx(SFX_ALERT);
                        } else {
                            AddLog("SECURITY DIRECTIVE: Martial Law lifted. Normal vault civil liberties restored.", 0);
                            PlaySfx(SFX_CLICK);
                        }
                    } else if (bId == BTN_POLICY_FOOD) {
                        g_state.policyFood = p1;
                        const char* pText[] = { "FEAST", "STANDARD", "HALF", "STRICT EMERGENCY" };
                        char buf[128];
                        sprintf(buf, "Overseer updated FOOD protocol to [%s].", pText[p1]);
                        AddLog(buf, 0);
                        PlaySfx(SFX_CLICK);
                    } else if (bId == BTN_POLICY_WATER) {
                        g_state.policyWater = p1;
                        const char* pText[] = { "FULL PURE", "STRICT", "RECYCLED SILT" };
                        char buf[128];
                        sprintf(buf, "Overseer updated WATER protocol to [%s].", pText[p1]);
                        AddLog(buf, 0);
                        PlaySfx(SFX_CLICK);
                    } else if (bId == BTN_POLICY_POWER) {
                        g_state.policyPower = p1;
                        const char* pText[] = { "BALANCED", "LIFE SUPPORT", "PRODUCTION" };
                        char buf[128];
                        sprintf(buf, "Overseer updated POWER protocol to [%s].", pText[p1]);
                        AddLog(buf, 0);
                        PlaySfx(SFX_CLICK);
                    } else if (bId == BTN_DEF_REPAIR) {
                        int cost = GetEffectiveScrapCost(15);
                        if (g_state.scrap >= (float)cost && g_state.barricadeHp < g_state.barricadeMaxHp) {
                            g_state.scrap -= (float)cost;
                            g_state.barricadeHp += 30;
                            if (g_state.barricadeHp > g_state.barricadeMaxHp) g_state.barricadeHp = g_state.barricadeMaxHp;
                            char buf[128];
                            sprintf(buf, "DEFENSE: Perimeter barricades repaired to %d/%d HP!", g_state.barricadeHp, g_state.barricadeMaxHp);
                            AddLog(buf, 3);
                            PlaySfx(SFX_REPAIR);
                        } else {
                            PlaySfx(SFX_ALERT);
                        }
                    } else if (bId == BTN_DEF_REINFORCE) {
                        int cost = GetEffectiveScrapCost(35);
                        if (g_state.scrap >= (float)cost) {
                            g_state.scrap -= (float)cost;
                            g_state.barricadeMaxHp += 25;
                            g_state.barricadeHp += 25;
                            char buf[128];
                            sprintf(buf, "DEFENSE: Blast-doors reinforced! Max HP now %d (+5 Def).", g_state.barricadeMaxHp);
                            AddLog(buf, 3);
                            PlaySfx(SFX_CONSTRUCTION);
                        } else {
                            PlaySfx(SFX_ALERT);
                        }
                    } else if (bId == BTN_DEF_TURRET) {
                        int cost = GetEffectiveScrapCost(45);
                        if (g_state.scrap >= (float)cost) {
                            g_state.scrap -= (float)cost;
                            g_state.turretCount++;
                            char buf[128];
                            sprintf(buf, "DEFENSE: 50-Cal Sentry Turret #%d mounted on outer bulkhead! (+18 Def)", g_state.turretCount);
                            AddLog(buf, 3);
                            PlaySfx(SFX_CONSTRUCTION);
                        } else {
                            PlaySfx(SFX_ALERT);
                        }
                    } else if (bId == BTN_DEF_OVERCLOCK) {
                        int cost = GetEffectiveScrapCost(20);
                        if (!g_state.turretOverclock && g_state.scrap >= (float)cost) {
                            g_state.scrap -= (float)cost;
                            g_state.turretOverclock = 1;
                            AddLog("DEFENSE: Sentry targeting overclocked! Auto-aim precision maximized (+10 Def).", 3);
                            PlaySfx(SFX_TURRET);
                        } else {
                            PlaySfx(SFX_ALERT);
                        }
                    } else if (bId == BTN_DEF_DRILL) {
                        int cost = GetEffectiveScrapCost(12);
                        if (g_state.scrap >= (float)cost) {
                            g_state.scrap -= (float)cost;
                            g_state.combatDrillLevel++;
                            g_state.morale += 3.0f;
                            if (g_state.morale > 100.0f) g_state.morale = 100.0f;
                            char buf[128];
                            sprintf(buf, "DEFENSE: Combat drill complete! Citizen tactical readiness raised to Lv %d.", g_state.combatDrillLevel);
                            AddLog(buf, 3);
                            PlaySfx(SFX_TURRET);
                        } else {
                            PlaySfx(SFX_ALERT);
                        }
                    } else if (bId == BTN_DEF_TOGGLE_GUARD) {
                        if (p1 >= 0 && p1 < g_state.numSurvivors) {
                            Survivor* surv = &g_state.survivors[p1];
                            if (strcmp(surv->job, "security") == 0) {
                                strcpy(surv->job, "unassigned");
                                char buf[128];
                                sprintf(buf, "GUARD: %s relieved from perimeter watch.", surv->name);
                                AddLog(buf, 0);
                            } else {
                                strcpy(surv->job, "security");
                                char buf[128];
                                sprintf(buf, "GUARD: %s (STR %d) assigned to perimeter security duty!", surv->name, surv->str);
                                AddLog(buf, 3);
                            }
                            PlaySfx(1);
                        }
                    } else if (bId == BTN_DEF_TEST_RAID) {
                        TriggerRaiderAttack(1);
                    } else if (bId == BTN_RESEARCH_TECH) {
                        DoResearch(p1);
                    } else if (bId == BTN_TRADE_BUY) {
                        BuyCaravanItem(p1);
                    } else if (bId == BTN_TRADE_RARE) {
                        BuyRareItem(p1);
                    } else if (bId == BTN_TRADE_SELL) {
                        SellSurplusItem(p1);
                    } else if (bId == BTN_HAIL_CARAVAN) {
                        HailCaravan();
                    } else if (bId == BTN_CLOSE_RAID_MODAL) {
                        g_state.showRaidModal = 0;
                        PlaySfx(1);
                    } else if (bId == BTN_CLOSE_MODAL) {
                        g_state.showSummary = 0;
                        PlaySfx(1);
                    }

                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }
            }
            break;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rcClient;
            GetClientRect(hwnd, &rcClient);
            int clientW = rcClient.right - rcClient.left;
            int clientH = rcClient.bottom - rcClient.top;

            // Double Buffer
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, clientW, clientH);
            HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

            // Fonts
            HFONT hFontTitle = CreateFontA(19, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH, "Consolas");
            HFONT hFontBold  = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH, "Consolas");
            HFONT hFontSmall = CreateFontA(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH, "Consolas");

            ClearButtons();

            // Background
            FillSolidRect(memDC, 0, 0, clientW, clientH, COL_BG);

            // Header (top 40px)
            DrawStyledBox(memDC, 10, 8, clientW - 20, 36, COL_PANEL_BG, COL_BORDER);
            SelectObject(memDC, hFontTitle);
            SetTextColor(memDC, COL_TEXT_BRIGHT);
            SetBkMode(memDC, TRANSPARENT);
            TextOutA(memDC, 20, 16, "KSANCTUARY", 10);

            SelectObject(memDC, hFontSmall);
            SetTextColor(memDC, COL_TEXT_DIM);
            TextOutA(memDC, 130, 20, "VAULT 704 // OVERSEER OS v1.2", 29);

            // Header Buttons
            int rightX = clientW - 20;
            DrawButtonControl(memDC, hFontSmall, rightX - 55, 14, 50, 24, "RESET", COL_TEXT_DIM, COL_BTN_BG, COL_BORDER, BTN_RESET, 0, 0);
            DrawButtonControl(memDC, hFontSmall, rightX - 145, 14, 85, 24, "HELP / MANUAL", COL_TEXT_MAIN, COL_BTN_BG, COL_BORDER, BTN_HELP, 0, 0);
            const char* crtLabel = g_crtScanlines ? "CRT: ON" : "CRT: OFF";
            DrawButtonControl(memDC, hFontSmall, rightX - 215, 14, 65, 24, crtLabel, g_crtScanlines ? COL_GREEN : COL_TEXT_DIM, COL_BTN_BG, COL_BORDER, BTN_CRT, 0, 0);
            const char* audLabel = g_soundEnabled ? "AUDIO: ON" : "AUDIO: OFF";
            DrawButtonControl(memDC, hFontSmall, rightX - 295, 14, 75, 24, audLabel, g_soundEnabled ? COL_GREEN : COL_TEXT_DIM, COL_BTN_BG, COL_BORDER, BTN_AUDIO, 0, 0);
            char themeLabel[40];
            sprintf(themeLabel, "THEME: %s", g_palettes[g_currentTheme].name);
            DrawButtonControl(memDC, hFontSmall, rightX - 445, 14, 145, 24, themeLabel, COL_TEXT_BRIGHT, COL_BTN_BG, COL_BORDER_HI, BTN_THEME, 0, 0);

            // Resource HUD (y: 48 to 110)
            DrawHUD(memDC, hFontBold, hFontSmall, 48);

            // Tabs Bar (y: 114 to 142)
            const char* tabNames[] = {
                "[1] FACILITIES",
                "[2] CITIZENS",
                "[3] SCAVENGE",
                "[4] DEFENSE",
                "[5] RESEARCH",
                "[6] HAZARDS",
                "[7] CARAVAN",
                "[8] DIRECTIVES",
                "[9] MANUAL"
            };
            int tabX = 10;
            int tabW = 68;
            for (int t = 0; t < 9; t++) {
                int active = (g_state.currentTab == t);
                COLORREF bg = active ? COL_BTN_HOVER : COL_DARK_CARD;
                COLORREF bdr = active ? COL_BORDER_HI : COL_BORDER;
                COLORREF txt = active ? COL_TEXT_BRIGHT : COL_TEXT_DIM;
                DrawButtonControl(memDC, hFontSmall, tabX, 114, tabW, 26, tabNames[t], txt, bg, bdr, BTN_TAB, t, 0);
                tabX += tabW + 2;
            }

            // Main Content Area
            int contentW = 635;
            int sidebarW = clientW - contentW - 30;
            int areaY = 146;
            int areaH = clientH - areaY - 26;

            DrawStyledBox(memDC, 10, areaY, contentW, areaH, COL_PANEL_BG, COL_BORDER);

            if (g_state.currentTab == 0) {
                DrawFacilitiesView(memDC, hFontBold, hFontSmall, 20, areaY + 10, contentW - 20, areaH - 20);
            } else if (g_state.currentTab == 1) {
                DrawSurvivorsView(memDC, hFontBold, hFontSmall, 20, areaY + 10, contentW - 20, areaH - 20);
            } else if (g_state.currentTab == 2) {
                DrawExpeditionsView(memDC, hFontBold, hFontSmall, 20, areaY + 10, contentW - 20, areaH - 20);
            } else if (g_state.currentTab == 3) {
                DrawDefenseView(memDC, hFontBold, hFontSmall, 20, areaY + 10, contentW - 20, areaH - 20);
            } else if (g_state.currentTab == 4) {
                DrawResearchView(memDC, hFontBold, hFontSmall, 20, areaY + 10, contentW - 20, areaH - 20);
            } else if (g_state.currentTab == 5) {
                DrawHazardsView(memDC, hFontBold, hFontSmall, 20, areaY + 10, contentW - 20, areaH - 20);
            } else if (g_state.currentTab == 6) {
                DrawTradingView(memDC, hFontBold, hFontSmall, 20, areaY + 10, contentW - 20, areaH - 20);
            } else if (g_state.currentTab == 7) {
                DrawPoliciesView(memDC, hFontBold, hFontSmall, 20, areaY + 10, contentW - 20, areaH - 20);
            } else if (g_state.currentTab == 8) {
                DrawManualView(memDC, hFontBold, hFontSmall, 20, areaY + 10, contentW - 20, areaH - 20);
            }

            // Sidebar
            DrawSidebar(memDC, hFontBold, hFontSmall, 10 + contentW + 10, areaY, sidebarW, areaH);

            // Footer (bottom 20px)
            int footY = clientH - 22;
            DrawStyledBox(memDC, 10, footY, clientW - 20, 18, COL_PANEL_BG, COL_BORDER);
            SelectObject(memDC, hFontSmall);
            SetTextColor(memDC, COL_TEXT_DIM);
            TextOutA(memDC, 18, footY + 2, "KSANCTUARY // OVERSEER OPERATING PROTOCOL 704", 45);
            TextOutA(memDC, clientW - 370, footY + 2, "STATUS: SECURE | AIRLOCK SEALED | SENSORS NORMAL", 48);

            // Summary Modal if open
            if (g_state.showSummary) {
                DrawSummaryModal(memDC, hFontBold, hFontSmall);
            }

            // Raid Modal if open
            if (g_state.showRaidModal) {
                DrawRaidModal(memDC, hFontBold, hFontSmall);
            }

            // CRT scanlines overlay
            if (g_crtScanlines) {
                HPEN hScanPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
                HPEN oldScanPen = (HPEN)SelectObject(memDC, hScanPen);
                for (int sy = 0; sy < clientH; sy += 4) {
                    MoveToEx(memDC, 0, sy, NULL);
                    LineTo(memDC, clientW, sy);
                }
                SelectObject(memDC, oldScanPen);
                DeleteObject(hScanPen);
            }

            // Blit to screen
            BitBlt(hdc, 0, 0, clientW, clientH, memDC, 0, 0, SRCCOPY);

            // Cleanup
            SelectObject(memDC, oldBM);
            DeleteObject(memBM);
            DeleteDC(memDC);

            DeleteObject(hFontTitle);
            DeleteObject(hFontBold);
            DeleteObject(hFontSmall);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY: {
            KillTimer(hwnd, IDT_AUTORUN);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KSanctuaryWindowClass";

    WNDCLASSA wc = {0};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;

    RegisterClassA(&wc);

    RECT wr = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    AdjustWindowRect(&wr, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

    HWND hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "KSanctuary - Vault 704 Overseer OS",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left, wr.bottom - wr.top,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) {
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
