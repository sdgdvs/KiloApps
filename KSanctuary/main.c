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

// Audio Thread
static int g_soundEnabled = 1;

static DWORD WINAPI SoundThreadProc(LPVOID lpParam) {
    if (!g_soundEnabled) return 0;
    int type = (int)(LONG_PTR)lpParam;
    if (type == 1) { // Click
        Beep(600, 30);
    } else if (type == 2) { // Cycle
        Beep(440, 60); Beep(554, 60); Beep(659, 80);
    } else if (type == 3) { // Alert
        Beep(220, 100); Beep(180, 120);
    } else if (type == 4) { // Scout dispatch/return
        Beep(523, 70); Beep(784, 90);
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
#define MAX_BLUEPRINTS 10
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
    int lastRaidScrap;
    int lastRaidMeds;
    int lastRaidDmg;
    int lastRaidFoodStolen;
    int lastRaidScrapStolen;
    int lastRaidInjured;
    int showRaidModal;
    float exteriorRads;
    
    // Policies
    int policyFood;  // 0: Standard (1.0), 1: Half (0.5), 2: Strict Emergency (0.25)
    int policyWater; // 0: Full (1.0), 1: Strict (0.5), 2: Minimal (0.25)
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
    
    // Active Tab
    int currentTab; // 0: Facilities, 1: Survivors, 2: Expeditions, 3: Defense, 4: Directives, 5: Manual
    
    int autoRun;
} GameState;

static GameState g_state;

// Clickable UI Region tracking
typedef struct {
    RECT rect;
    int id;
    int param1;
    int param2;
} ClickableButton;

#define MAX_BUTTONS 100
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
    if (eff < 0.2f) eff = 0.2f;
    return eff;
}

static int CalculateTotalDefense() {
    int baseHull = 10;
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

    float foodPer = (g_state.policyFood == 1) ? 0.5f : ((g_state.policyFood == 2) ? 0.25f : 1.0f);
    float waterPer = (g_state.policyWater == 1) ? 0.5f : ((g_state.policyWater == 2) ? 0.25f : 1.0f);

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

static void TriggerDailyEvent() {
    int roll = rand() % 100;
    if (roll < 25) {
        if (g_state.numCandidates < MAX_CANDIDATES) {
            Candidate* c = &g_state.candidates[g_state.numCandidates++];
            GenerateCandidate(c, rand() % 4 == 0);
            char buf[128];
            sprintf(buf, "AIRLOCK SENSOR: Refugee %s (%s) detected! Review at Radio Beacon.", c->name, c->role);
            AddLog(buf, 3);
            PlaySfx(4);
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
        PlaySfx(2);
    } else {
        int deficit = assaultPower - totalDef;
        int dmg = 30 + (deficit * 8) / 10;
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

    float netFood = foodP - foodN;
    float netWater = waterP - waterN;

    g_state.food += netFood;
    if (g_state.food < 0.0f) g_state.food = 0.0f;

    g_state.water += netWater;
    if (g_state.water < 0.0f) g_state.water = 0.0f;

    g_state.scrap += scrapP;

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

    if (g_state.food > 0.0f && g_state.water > 0.0f) {
        for (int i = 0; i < g_state.numSurvivors; i++) {
            if (g_state.survivors[i].health < 100) g_state.survivors[i].health += 5;
            if (g_state.survivors[i].health > 100) g_state.survivors[i].health = 100;
            g_state.survivors[i].hunger -= 15;
            if (g_state.survivors[i].hunger < 0) g_state.survivors[i].hunger = 0;
            g_state.survivors[i].thirst -= 20;
            if (g_state.survivors[i].thirst < 0) g_state.survivors[i].thirst = 0;
        }
        g_state.morale += 1.0f;
        if (g_state.morale > 100.0f) g_state.morale = 100.0f;
    }

    // Expeditions update
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
                int mFound = (int)(exp->potentialMeds * mMult);
                if (mFound < 1) mFound = 1;

                g_state.food += fFound;
                g_state.scrap += sFound;
                g_state.meds += mFound;

                // Hazard damage
                int baseDmg = (exp->riskLevel == 4) ? (30 + rand() % 25) : ((exp->riskLevel == 3) ? (20 + rand() % 20) : ((exp->riskLevel == 2) ? (12 + rand() % 15) : (5 + rand() % 10)));
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
                            int chance = 35 + inte * 7;
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
                sprintf(buf, "%s returned from %s! Salvaged: +%d Food, +%d Scrap, +%d Meds. [-%d HP]%s%s", scout ? scout->name : "Scout", exp->name, fFound, sFound, mFound, dmg, stimUsed ? " (Stimpack stabilized)" : "", bpMsg);
                AddLog(buf, strlen(bpMsg) > 0 ? 3 : 4);
                PlaySfx(strlen(bpMsg) > 0 ? 2 : 4);
            }
        }
    }

    // Medical triage in Infirmary
    int hasDoctor = 0;
    for (int s = 0; s < g_state.numSurvivors; s++) {
        if (strcmp(g_state.survivors[s].job, "infirmary") == 0) { hasDoctor = 1; break; }
    }
    int hasSurg = 0;
    for (int b = 0; b < g_state.numBlueprints; b++) {
        if (strcmp(g_state.blueprints[b].id, "bp_medsurge") == 0 && g_state.blueprints[b].built) { hasSurg = 1; break; }
    }
    if (hasDoctor || hasSurg) {
        int woundedCount = 0;
        for (int s = 0; s < g_state.numSurvivors; s++) {
            if (g_state.survivors[s].health < 90) woundedCount++;
        }
        if (woundedCount > 0 && (g_state.meds > 0 || hasSurg)) {
            if (!hasSurg && g_state.meds > 0) g_state.meds--;
            for (int s = 0; s < g_state.numSurvivors; s++) {
                if (g_state.survivors[s].health < 90) {
                    g_state.survivors[s].health += (hasSurg ? 25 : 15);
                    if (g_state.survivors[s].health > 100) g_state.survivors[s].health = 100;
                }
            }
            char medBuf[128];
            sprintf(medBuf, "MED-LAB: Triage treatment administered to %d injured dweller(s).", woundedCount);
            AddLog(medBuf, 0);
        }
    }

    // Raider Incursion Countdown
    g_state.raidThreatDays--;
    if (g_state.raidThreatDays <= 0) {
        TriggerRaiderAttack(0);
        g_state.raidThreatDays = 3 + rand() % 3;
    } else if (g_state.raidThreatDays == 1) {
        AddLog("RADAR WARNING: Raider vanguard spotted 5km away! Raid expected tomorrow!", 1);
        PlaySfx(3);
    }

    TriggerDailyEvent();

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

    g_state.policyFood = 0;
    g_state.policyWater = 0;
    g_state.policyPower = 0;

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
        // Room Construction Blueprints (2 Columns, 5 Rows)
        int colW = (w - 10) / 2;
        int cardH = 78;
        int gapY = 6;

        for (int i = 0; i < g_state.numBlueprints; i++) {
            RoomBlueprint* bp = &g_state.blueprints[i];
            int col = i % 2;
            int row = i / 2;
            int cx = x + col * (colW + 10);
            int cy = startY + row * (cardH + gapY);

            DrawStyledBox(hdc, cx, cy, colW, cardH, COL_DARK_CARD, COL_BORDER);

            // Name + Cost / Status
            SelectObject(hdc, hFontBold);
            SetTextColor(hdc, COL_TEXT_BRIGHT);
            TextOutA(hdc, cx + 8, cy + 5, bp->name, (int)strlen(bp->name));

            SelectObject(hdc, hFontSmall);
            if (bp->built) {
                SetTextColor(hdc, COL_GREEN);
                TextOutA(hdc, cx + colW - 55, cy + 5, "BUILT", 5);
            } else if (bp->locked) {
                SetTextColor(hdc, COL_AMBER);
                TextOutA(hdc, cx + colW - 105, cy + 5, "LOCKED BLUEPRINT", 16);
            } else {
                char costBuf[16];
                sprintf(costBuf, "%d SCRAP", bp->cost);
                SetTextColor(hdc, COL_AMBER);
                TextOutA(hdc, cx + colW - 68, cy + 5, costBuf, (int)strlen(costBuf));
            }

            // Desc
            SetTextColor(hdc, COL_TEXT_DIM);
            TextOutA(hdc, cx + 8, cy + 22, bp->desc, (int)strlen(bp->desc));

            // Benefit
            SetTextColor(hdc, COL_GREEN);
            char benBuf[64];
            sprintf(benBuf, "Benefit: %s", bp->benefit);
            TextOutA(hdc, cx + 8, cy + 39, benBuf, (int)strlen(benBuf));

            // Action button
            if (bp->built) {
                SetTextColor(hdc, COL_GREEN);
                RECT rcOp = { cx + colW - 180, cy + 54, cx + colW - 10, cy + 74 };
                DrawTextA(hdc, "[ OPERATIONAL & ONLINE ]", -1, &rcOp, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            } else if (bp->locked) {
                SetTextColor(hdc, COL_AMBER);
                RECT rcLk = { cx + colW - 220, cy + 54, cx + colW - 10, cy + 74 };
                char lkBuf[64];
                sprintf(lkBuf, "[ LOCKED: %s ]", bp->discoverSource);
                DrawTextA(hdc, lkBuf, -1, &rcLk, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
            } else {
                char bBuf[32];
                sprintf(bBuf, "CONSTRUCT (%d SCRAP)", bp->cost);
                int canAfford = (g_state.scrap >= bp->cost);
                COLORREF cBg = canAfford ? RGB(25, 50, 30) : COL_DARK_CARD;
                COLORREF cTxt = canAfford ? COL_TEXT_BRIGHT : COL_TEXT_DIM;
                COLORREF cBdr = canAfford ? COL_GREEN : COL_BORDER;
                DrawButtonControl(hdc, hFontBold, cx + colW - 185, cy + 54, 178, 20, bBuf, cTxt, cBg, cBdr, BTN_CONSTRUCT_ROOM, i, 0);
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
        int cardH = 58;
        int gap = 6;

        for (int i = 0; i < g_state.numSurvivors; i++) {
            Survivor* s = &g_state.survivors[i];
            int cy = startY + i * (cardH + gap);
            if (cy + cardH > y + h + 20) break;

            DrawStyledBox(hdc, x, cy, w, cardH, COL_DARK_CARD, COL_BORDER);

            // Name & Role
            SelectObject(hdc, hFontBold);
            SetTextColor(hdc, COL_TEXT_BRIGHT);
            TextOutA(hdc, x + 8, cy + 6, s->name, (int)strlen(s->name));

            char roleBuf[64];
            sprintf(roleBuf, "%s | STR:%d AGI:%d INT:%d", s->role, s->str, s->agi, s->inte);
            SelectObject(hdc, hFontSmall);
            SetTextColor(hdc, COL_TEXT_DIM);
            TextOutA(hdc, x + 8, cy + 24, roleBuf, (int)strlen(roleBuf));

            // Health Bar
            TextOutA(hdc, x + 230, cy + 6, "HEALTH", 6);
            char hpVal[8];
            sprintf(hpVal, "%d%%", s->health);
            TextOutA(hdc, x + 315, cy + 6, hpVal, (int)strlen(hpVal));
            COLORREF hpColor = s->health < 40 ? COL_RED : (s->health < 75 ? COL_AMBER : COL_GREEN);
            DrawProgressBar(hdc, x + 230, cy + 20, 110, 8, s->health / 100.0f, hpColor);

            // Morale Bar
            TextOutA(hdc, x + 230, cy + 32, "MORALE", 6);
            char morVal[8];
            sprintf(morVal, "%d%%", s->morale);
            TextOutA(hdc, x + 315, cy + 32, morVal, (int)strlen(morVal));
            DrawProgressBar(hdc, x + 230, cy + 44, 110, 8, s->morale / 100.0f, COL_AMBER);

            // Current Job & Efficiency Badge
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
                DrawStyledBox(hdc, x + w - 195, cy + 14, 190, 28, COL_DARK_CARD, COL_BORDER);
                SelectObject(hdc, hFontSmall);
                SetTextColor(hdc, COL_AMBER);
                RECT rc = { x + w - 195, cy + 14, x + w - 5, cy + 42 };
                DrawTextA(hdc, "ON WASTELAND EXP", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            } else {
                int jobW = (s->health < 100 && g_state.meds > 0) ? 132 : 190;
                DrawButtonControl(hdc, hFontSmall, x + w - 195, cy + 14, jobW, 28, jobLabel, COL_TEXT_MAIN, COL_BTN_BG, COL_BORDER, BTN_SURV_JOB, i, 0);
                if (s->health < 100 && g_state.meds > 0) {
                    DrawButtonControl(hdc, hFontSmall, x + w - 58, cy + 14, 54, 28, "HEAL (-1M)", COL_GREEN, RGB(25, 45, 30), COL_GREEN, BTN_TREAT_SURV, i, 0);
                }
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

static void DrawPoliciesView(HDC hdc, HFONT hFontBold, HFONT hFontSmall, int x, int y, int w, int h) {
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x, y, "OVERSEER PROTOCOLS & RATIONING DIRECTIVES", 41);

    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x + w - 210, y, "Directives take effect on cycle advance", 39);

    int curY = y + 26;
    int optW = 185;
    int optH = 74;
    int spacing = 10;

    // 1. Food Policies
    DrawStyledBox(hdc, x, curY, w, 116, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x + 10, curY + 8, "FOOD RATIONS PROTOCOL", 21);

    const char* fNames[] = { "Standard Rations", "Half Rations", "Strict Emergency" };
    const char* fDesc1[] = { "1.0 Food per citizen.", "0.5 Food per citizen.", "0.25 Food per citizen." };
    const char* fDesc2[] = { "Normal morale, 0 malnutrition.", "Conserves food, -4% morale.", "High thirst & starvation risk!" };

    for (int i = 0; i < 3; i++) {
        int bx = x + 10 + i * (optW + spacing);
        int by = curY + 30;
        int active = (g_state.policyFood == i);
        COLORREF bg = active ? COL_BTN_HOVER : COL_PANEL_BG;
        COLORREF bdr = active ? COL_BORDER_HI : COL_BORDER;

        DrawStyledBox(hdc, bx, by, optW, optH, bg, bdr);
        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, active ? COL_TEXT_BRIGHT : COL_TEXT_MAIN);
        TextOutA(hdc, bx + 6, by + 6, fNames[i], (int)strlen(fNames[i]));

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COL_TEXT_DIM);
        TextOutA(hdc, bx + 6, by + 26, fDesc1[i], (int)strlen(fDesc1[i]));
        TextOutA(hdc, bx + 6, by + 42, fDesc2[i], (int)strlen(fDesc2[i]));

        AddButton(bx, by, optW, optH, BTN_POLICY_FOOD, i, 0);
    }
    curY += 126;

    // 2. Water Policies
    DrawStyledBox(hdc, x, curY, w, 116, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x + 10, curY + 8, "WATER CONSERVATION PROTOCOL", 27);

    const char* wNames[] = { "Full Allocation", "Strict Rationing", "Moisture Recovery" };
    const char* wDesc1[] = { "1.0 Water per citizen.", "0.5 Water per citizen.", "0.25 Water per citizen." };
    const char* wDesc2[] = { "Crisp & filtered, +2% morale.", "Conserves water, -3% morale.", "Dehydration risk, -10% morale." };

    for (int i = 0; i < 3; i++) {
        int bx = x + 10 + i * (optW + spacing);
        int by = curY + 30;
        int active = (g_state.policyWater == i);
        COLORREF bg = active ? COL_BTN_HOVER : COL_PANEL_BG;
        COLORREF bdr = active ? COL_BORDER_HI : COL_BORDER;

        DrawStyledBox(hdc, bx, by, optW, optH, bg, bdr);
        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, active ? COL_TEXT_BRIGHT : COL_TEXT_MAIN);
        TextOutA(hdc, bx + 6, by + 6, wNames[i], (int)strlen(wNames[i]));

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COL_TEXT_DIM);
        TextOutA(hdc, bx + 6, by + 26, wDesc1[i], (int)strlen(wDesc1[i]));
        TextOutA(hdc, bx + 6, by + 42, wDesc2[i], (int)strlen(wDesc2[i]));

        AddButton(bx, by, optW, optH, BTN_POLICY_WATER, i, 0);
    }
    curY += 126;

    // 3. Power Policies
    DrawStyledBox(hdc, x, curY, w, 116, COL_DARK_CARD, COL_BORDER);
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x + 10, curY + 8, "REACTOR POWER GRID PRIORITY", 27);

    const char* pNames[] = { "Balanced Grid", "Life Support Priority", "Resource Focus" };
    const char* pDesc1[] = { "Distribute power evenly.", "Infirmary & Quarters stay on.", "Farms & Purifiers stay on." };
    const char* pDesc2[] = { "Standard grid balancing.", "Prevents citizen casualties.", "Prevents rationing collapse." };

    for (int i = 0; i < 3; i++) {
        int bx = x + 10 + i * (optW + spacing);
        int by = curY + 30;
        int active = (g_state.policyPower == i);
        COLORREF bg = active ? COL_BTN_HOVER : COL_PANEL_BG;
        COLORREF bdr = active ? COL_BORDER_HI : COL_BORDER;

        DrawStyledBox(hdc, bx, by, optW, optH, bg, bdr);
        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, active ? COL_TEXT_BRIGHT : COL_TEXT_MAIN);
        TextOutA(hdc, bx + 6, by + 6, pNames[i], (int)strlen(pNames[i]));

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COL_TEXT_DIM);
        TextOutA(hdc, bx + 6, by + 26, pDesc1[i], (int)strlen(pDesc1[i]));
        TextOutA(hdc, bx + 6, by + 42, pDesc2[i], (int)strlen(pDesc2[i]));

        AddButton(bx, by, optW, optH, BTN_POLICY_POWER, i, 0);
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
    TextOutA(hdc, x + 12, curY, "8. KEYBOARD SHORTCUTS:", 22); curY += lineH;
    SetTextColor(hdc, COL_TEXT_MAIN);
    TextOutA(hdc, x + 20, curY, "[SPACE] Advance Cycle | [1-6] Tabs | [T] Theme | [C] CRT | [A] Auto | [H] Help | [R] Reset", 90);
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
            } else if (wParam >= '1' && wParam <= '6') {
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
                g_state.currentTab = 5;
                PlaySfx(1);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'T' || wParam == 't') {
                g_currentTheme = (g_currentTheme + 1) % THEME_COUNT;
                PlaySfx(1);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'C' || wParam == 'c') {
                g_crtScanlines = !g_crtScanlines;
                PlaySfx(1);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'R' || wParam == 'r') {
                InitGameState();
                PlaySfx(3);
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
                        g_state.currentTab = 5;
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
                        int uCost = fac->level * 30;
                        if (g_state.scrap >= uCost && fac->level < 3) {
                            g_state.scrap -= uCost;
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
                            PlaySfx(2);
                        } else {
                            PlaySfx(3);
                        }
                    } else if (bId == BTN_CONSTRUCT_ROOM) {
                        RoomBlueprint* bp = &g_state.blueprints[p1];
                        if (g_state.scrap >= bp->cost && !bp->built && !bp->locked && g_state.numFacilities < 16) {
                            g_state.scrap -= bp->cost;
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
                            PlaySfx(2);
                        } else {
                            PlaySfx(3);
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
                        PlaySfx(1);
                    } else if (bId == BTN_SURV_SUBTAB) {
                        g_state.survivorSubTab = p1;
                        PlaySfx(1);
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
                            PlaySfx(4);
                        } else {
                            AddLog("RADIO: Insufficient scrap (15) or reactor power (10 kW) to broadcast ping!", 1);
                            PlaySfx(3);
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
                                PlaySfx(2);
                            }
                        } else {
                            AddLog("RADIO: Insufficient scrap (25) or food (10) for specialist beacon!", 1);
                            PlaySfx(3);
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
                            PlaySfx(2);

                            for (int k = p1; k < g_state.numCandidates - 1; k++) {
                                g_state.candidates[k] = g_state.candidates[k + 1];
                            }
                            g_state.numCandidates--;
                        } else {
                            AddLog("AIRLOCK: Quarters are full! Expand living barracks first.", 1);
                            PlaySfx(3);
                        }
                    } else if (bId == BTN_DISMISS_CANDIDATE) {
                        if (p1 >= 0 && p1 < g_state.numCandidates) {
                            char buf[128];
                            sprintf(buf, "AIRLOCK: Clearance denied. %s was turned away into the wastes.", g_state.candidates[p1].name);
                            AddLog(buf, 0);
                            PlaySfx(1);

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
                            PlaySfx(4);
                        }
                    } else if (bId == BTN_EXP_STIM_TOGGLE) {
                        Expedition* exp = &g_state.expeditions[p1];
                        if (strlen(exp->assignedScout) == 0) {
                            if (!exp->hasStimpack) {
                                if (g_state.meds > 0) {
                                    exp->hasStimpack = 1;
                                    PlaySfx(1);
                                } else {
                                    AddLog("No medical supplies in vault stock to equip Stimpack!", 1);
                                    PlaySfx(3);
                                }
                            } else {
                                exp->hasStimpack = 0;
                                PlaySfx(1);
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
                                PlaySfx(2);
                            }
                        }
                    } else if (bId == BTN_POLICY_FOOD) {
                        g_state.policyFood = p1;
                        const char* pText[] = { "STANDARD", "HALF", "STRICT EMERGENCY" };
                        char buf[128];
                        sprintf(buf, "Overseer updated FOOD protocol to [%s].", pText[p1]);
                        AddLog(buf, 0);
                        PlaySfx(1);
                    } else if (bId == BTN_POLICY_WATER) {
                        g_state.policyWater = p1;
                        const char* pText[] = { "FULL", "STRICT", "MINIMAL" };
                        char buf[128];
                        sprintf(buf, "Overseer updated WATER protocol to [%s].", pText[p1]);
                        AddLog(buf, 0);
                        PlaySfx(1);
                    } else if (bId == BTN_POLICY_POWER) {
                        g_state.policyPower = p1;
                        const char* pText[] = { "BALANCED", "LIFE SUPPORT", "PRODUCTION" };
                        char buf[128];
                        sprintf(buf, "Overseer updated POWER protocol to [%s].", pText[p1]);
                        AddLog(buf, 0);
                        PlaySfx(1);
                    } else if (bId == BTN_DEF_REPAIR) {
                        if (g_state.scrap >= 15 && g_state.barricadeHp < g_state.barricadeMaxHp) {
                            g_state.scrap -= 15;
                            g_state.barricadeHp += 30;
                            if (g_state.barricadeHp > g_state.barricadeMaxHp) g_state.barricadeHp = g_state.barricadeMaxHp;
                            char buf[128];
                            sprintf(buf, "DEFENSE: Perimeter barricades repaired to %d/%d HP!", g_state.barricadeHp, g_state.barricadeMaxHp);
                            AddLog(buf, 3);
                            PlaySfx(2);
                        } else {
                            PlaySfx(3);
                        }
                    } else if (bId == BTN_DEF_REINFORCE) {
                        if (g_state.scrap >= 35) {
                            g_state.scrap -= 35;
                            g_state.barricadeMaxHp += 25;
                            g_state.barricadeHp += 25;
                            char buf[128];
                            sprintf(buf, "DEFENSE: Blast-doors reinforced! Max HP now %d (+5 Def).", g_state.barricadeMaxHp);
                            AddLog(buf, 3);
                            PlaySfx(2);
                        } else {
                            PlaySfx(3);
                        }
                    } else if (bId == BTN_DEF_TURRET) {
                        if (g_state.scrap >= 45) {
                            g_state.scrap -= 45;
                            g_state.turretCount++;
                            char buf[128];
                            sprintf(buf, "DEFENSE: 50-Cal Sentry Turret #%d mounted on outer bulkhead! (+18 Def)", g_state.turretCount);
                            AddLog(buf, 3);
                            PlaySfx(2);
                        } else {
                            PlaySfx(3);
                        }
                    } else if (bId == BTN_DEF_OVERCLOCK) {
                        if (!g_state.turretOverclock && g_state.scrap >= 20) {
                            g_state.scrap -= 20;
                            g_state.turretOverclock = 1;
                            AddLog("DEFENSE: Sentry targeting overclocked! Auto-aim precision maximized (+10 Def).", 3);
                            PlaySfx(2);
                        } else {
                            PlaySfx(3);
                        }
                    } else if (bId == BTN_DEF_DRILL) {
                        if (g_state.scrap >= 12) {
                            g_state.scrap -= 12;
                            g_state.combatDrillLevel++;
                            g_state.morale += 3.0f;
                            if (g_state.morale > 100.0f) g_state.morale = 100.0f;
                            char buf[128];
                            sprintf(buf, "DEFENSE: Combat drill complete! Citizen tactical readiness raised to Lv %d.", g_state.combatDrillLevel);
                            AddLog(buf, 3);
                            PlaySfx(2);
                        } else {
                            PlaySfx(3);
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
                "[5] POLICIES",
                "[6] MANUAL"
            };
            int tabX = 10;
            int tabW = 104;
            for (int t = 0; t < 6; t++) {
                int active = (g_state.currentTab == t);
                COLORREF bg = active ? COL_BTN_HOVER : COL_DARK_CARD;
                COLORREF bdr = active ? COL_BORDER_HI : COL_BORDER;
                COLORREF txt = active ? COL_TEXT_BRIGHT : COL_TEXT_DIM;
                DrawButtonControl(memDC, hFontSmall, tabX, 114, tabW, 26, tabNames[t], txt, bg, bdr, BTN_TAB, t, 0);
                tabX += tabW + 3;
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
                DrawPoliciesView(memDC, hFontBold, hFontSmall, 20, areaY + 10, contentW - 20, areaH - 20);
            } else if (g_state.currentTab == 5) {
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
