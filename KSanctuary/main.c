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

#define MAX_EXPEDITIONS 3
typedef struct {
    char id[16];
    char name[40];
    char desc[90];
    int duration;
    char risk[24];
    char assignedScout[16]; // id or empty
    int daysRemaining;
    int potentialFood;
    int potentialScrap;
} Expedition;

#define MAX_LOG_ENTRIES 60
typedef struct {
    char text[128];
    int type; // 0: info, 1: warn, 2: crit, 3: cycle, 4: scout
    int day;
    int phase;
} LogEntry;

// Construction Blueprints
#define MAX_BLUEPRINTS 6
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
} RoomBlueprint;

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
    float morale;
    int defense;
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
    
    // Survivors
    Survivor survivors[MAX_SURVIVORS];
    int numSurvivors;
    
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
    int currentTab; // 0: Facilities, 1: Survivors, 2: Expeditions, 3: Directives, 4: Manual
    
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
    BTN_DISPATCH_SCOUT,
    BTN_POLICY_FOOD,
    BTN_POLICY_WATER,
    BTN_POLICY_POWER,
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

static void CalculateTotals(float* foodProd, float* foodNeed, float* waterProd, float* waterNeed, int* powerGen, int* powerLoad, float* scrapProd) {
    *foodProd = 0;
    *waterProd = 0;
    *powerGen = 0;
    *powerLoad = 0;
    *scrapProd = 0;

    int isBlackout = (g_state.powerGen < g_state.powerLoad);

    for (int i = 0; i < g_state.numFacilities; i++) {
        Facility* fac = &g_state.facilities[i];
        if (fac->powerProd > 0) {
            *powerGen += (fac->assigned > 0 ? (fac->powerProd + (fac->assigned - 1) * 8) : 6);
        } else {
            *powerLoad += fac->powerCost;
        }

        if (fac->assigned > 0) {
            int isPowered = (!isBlackout || fac->powerPriority <= 2);
            if (isPowered) {
                *foodProd += fac->foodProd * fac->assigned;
                *waterProd += fac->waterProd * fac->assigned;
                *scrapProd += fac->scrapProd * fac->assigned;
            }
        }
    }

    float foodPer = (g_state.policyFood == 1) ? 0.5f : ((g_state.policyFood == 2) ? 0.25f : 1.0f);
    float waterPer = (g_state.policyWater == 1) ? 0.5f : ((g_state.policyWater == 2) ? 0.25f : 1.0f);

    *foodNeed = g_state.population * foodPer;
    *waterNeed = g_state.population * waterPer;
}

static void TriggerDailyEvent() {
    int roll = rand() % 100;
    if (roll < 22) {
        if (g_state.population < g_state.maxPop && g_state.numSurvivors < MAX_SURVIVORS) {
            const char* fnames[] = { "Silas", "Daphne", "Gideon", "Kira", "Nolan", "Vera", "Rook" };
            const char* lnames[] = { "Cross", "Vance", "Mercer", "Gant", "Holloway", "Kane" };
            int fi = rand() % 7;
            int li = rand() % 6;
            Survivor* ns = &g_state.survivors[g_state.numSurvivors++];
            sprintf(ns->id, "s_%d", (int)time(NULL) % 10000 + g_state.numSurvivors);
            sprintf(ns->name, "%s %s", fnames[fi], lnames[li]);
            strcpy(ns->role, "Wasteland Refugee");
            ns->health = 85;
            ns->hunger = 10;
            ns->thirst = 15;
            ns->morale = 75;
            strcpy(ns->job, "unassigned");
            ns->str = 4 + rand() % 4;
            ns->agi = 4 + rand() % 4;
            ns->inte = 4 + rand() % 4;
            g_state.population = g_state.numSurvivors;

            char buf[128];
            sprintf(buf, "AIRLOCK ALERT: Wanderer %s was admitted to the vault!", ns->name);
            AddLog(buf, 3);
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
                int fFound = (int)(exp->potentialFood * (0.8f + (rand() % 40) / 100.0f));
                int sFound = (int)(exp->potentialScrap * (0.8f + (rand() % 40) / 100.0f));
                g_state.food += fFound;
                g_state.scrap += sFound;
                if (scout) {
                    strcpy(scout->job, "unassigned");
                    scout->morale += 10;
                    if (scout->morale > 100) scout->morale = 100;
                }
                char buf[128];
                sprintf(buf, "%s returned from %s! Salvaged: +%d Food, +%d Scrap.", scout ? scout->name : "Scout", exp->name, fFound, sFound);
                AddLog(buf, 4);
                exp->assignedScout[0] = '\0';
                PlaySfx(4);
            }
        }
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
    g_state.morale = 85.0f;
    g_state.defense = 15;
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

    // 6 Construction Blueprints & Sub-Tab
    g_state.facilitySubTab = 0;
    g_state.numBlueprints = 6;

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

    // Expeditions
    g_state.numExpeditions = 3;
    strcpy(g_state.expeditions[0].id, "exp1");
    strcpy(g_state.expeditions[0].name, "Old Supermarket Ruins");
    strcpy(g_state.expeditions[0].desc, "Warehouse 4km north. High food caches.");
    g_state.expeditions[0].duration = 1;
    strcpy(g_state.expeditions[0].risk, "Low Risk");
    g_state.expeditions[0].assignedScout[0] = '\0';
    g_state.expeditions[0].potentialFood = 15;
    g_state.expeditions[0].potentialScrap = 10;

    strcpy(g_state.expeditions[1].id, "exp2");
    strcpy(g_state.expeditions[1].name, "Derelict Power Substation");
    strcpy(g_state.expeditions[1].desc, "Transformer yard with capacitors and copper wiring.");
    g_state.expeditions[1].duration = 2;
    strcpy(g_state.expeditions[1].risk, "Moderate Risk");
    g_state.expeditions[1].assignedScout[0] = '\0';
    g_state.expeditions[1].potentialFood = 5;
    g_state.expeditions[1].potentialScrap = 28;

    strcpy(g_state.expeditions[2].id, "exp3");
    strcpy(g_state.expeditions[2].name, "Military Convoy Wreckage");
    strcpy(g_state.expeditions[2].desc, "Armored trucks along highway 80. Weaponry & parts.");
    g_state.expeditions[2].duration = 2;
    strcpy(g_state.expeditions[2].risk, "High Hazard");
    g_state.expeditions[2].assignedScout[0] = '\0';
    g_state.expeditions[2].potentialFood = 12;
    g_state.expeditions[2].potentialScrap = 40;

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

    int cardW = 138;
    int cardH = 58;
    int spacing = 6;
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
    TextOutA(hdc, curX + 6, startY + 4, "FOOD RATIONS", 12);
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
    TextOutA(hdc, curX + 6, startY + 4, "WATER PURITY", 12);
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
    TextOutA(hdc, curX + 40, startY + 22, "kW Gen", 6);
    char pSubBuf[32];
    sprintf(pSubBuf, "Demand: %d kW", pLoad);
    TextOutA(hdc, curX + 6, startY + 40, pSubBuf, (int)strlen(pSubBuf));
    curX += cardW + spacing;

    // 5. Scrap
    DrawStyledBox(hdc, curX, startY, cardW, cardH, COL_PANEL_BG, COL_BORDER);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, curX + 6, startY + 4, "SCRAP / TECH", 12);
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

    // 6. Morale
    DrawStyledBox(hdc, curX, startY, cardW, cardH, COL_PANEL_BG, COL_BORDER);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, curX + 6, startY + 4, "MORALE", 6);
    const char* mLabel = "CONTENT";
    if (g_state.morale >= 90.0f) mLabel = "THRIVING";
    else if (g_state.morale < 45.0f) mLabel = "MUTINOUS";
    else if (g_state.morale < 70.0f) mLabel = "ANXIOUS";
    SetTextColor(hdc, g_state.morale >= 80.0f ? COL_GREEN : (g_state.morale < 50.0f ? COL_RED : COL_AMBER));
    TextOutA(hdc, curX + cardW - 60, startY + 4, mLabel, (int)strlen(mLabel));
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    char morVal[16];
    sprintf(morVal, "%d%%", (int)g_state.morale);
    TextOutA(hdc, curX + 6, startY + 18, morVal, (int)strlen(morVal));
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, curX + 6, startY + 40, "Vault stability", 15);
    curX += cardW + spacing;

    // 7. Defense & Rads
    DrawStyledBox(hdc, curX, startY, cardW, cardH, COL_PANEL_BG, COL_BORDER);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, curX + 6, startY + 4, "DEFENSE / RADS", 14);
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
        if (!g_state.blueprints[b].built) availBp++;
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
        int cardH = 68;
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
            TextOutA(hdc, cx + 6, cy + 5, fac->name, (int)strlen(fac->name));

            char lvlBuf[12];
            sprintf(lvlBuf, "LV%d", fac->level);
            SelectObject(hdc, hFontSmall);
            SetTextColor(hdc, COL_AMBER);
            TextOutA(hdc, cx + 165, cy + 6, lvlBuf, (int)strlen(lvlBuf));

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
            TextOutA(hdc, cx + colW - 65, cy + 6, stText, (int)strlen(stText));

            // Row 2: Stats output line
            char outBuf[64];
            if (fac->powerProd > 0) {
                int outP = fac->assigned > 0 ? (fac->powerProd + (fac->assigned - 1) * 8) : 6;
                sprintf(outBuf, "Out: +%d kW Power | Self-Sufficient", outP);
            } else if (fac->foodProd > 0) {
                sprintf(outBuf, "Out: +%d Food/cyc | Draw: -%d kW", fac->foodProd * fac->assigned, fac->powerCost);
            } else if (fac->waterProd > 0) {
                sprintf(outBuf, "Out: +%d Water/cyc | Draw: -%d kW", fac->waterProd * fac->assigned, fac->powerCost);
            } else if (fac->scrapProd > 0) {
                sprintf(outBuf, "Out: +%d Scrap/cyc | Draw: -%d kW", fac->scrapProd * fac->assigned, fac->powerCost);
            } else if (strcmp(fac->id, "quarters") == 0 || strcmp(fac->id, "quarters_ext") == 0) {
                sprintf(outBuf, "Cap: +%d Dwellers | Draw: -%d kW", 10 + (fac->level - 1) * 4, fac->powerCost);
            } else if (strcmp(fac->id, "security") == 0) {
                sprintf(outBuf, "Defense: +%d Armor | Draw: -%d kW", 15 + (fac->level - 1) * 10, fac->powerCost);
            } else {
                sprintf(outBuf, "Medical Triage | Draw: -%d kW", fac->powerCost);
            }
            SetTextColor(hdc, COL_TEXT_MAIN);
            TextOutA(hdc, cx + 6, cy + 24, outBuf, (int)strlen(outBuf));

            // Row 3: Staff [-] [+] controls and Upgrade Button
            if (fac->maxWorkers > 0) {
                char staffBuf[20];
                sprintf(staffBuf, "Staff: %d/%d", fac->assigned, fac->maxWorkers);
                SetTextColor(hdc, COL_TEXT_DIM);
                TextOutA(hdc, cx + 6, cy + 45, staffBuf, (int)strlen(staffBuf));

                // [-] button
                COLORREF btnMinusBg = (fac->assigned > 0) ? COL_BTN_BG : COL_DARK_CARD;
                COLORREF btnMinusTxt = (fac->assigned > 0) ? COL_TEXT_MAIN : COL_TEXT_DIM;
                DrawButtonControl(hdc, hFontBold, cx + 80, cy + 42, 18, 20, "-", btnMinusTxt, btnMinusBg, COL_BORDER, BTN_FAC_WORKER, i, -1);

                // [+] button
                COLORREF btnPlusBg = (fac->assigned < fac->maxWorkers && idle > 0) ? COL_BTN_BG : COL_DARK_CARD;
                COLORREF btnPlusTxt = (fac->assigned < fac->maxWorkers && idle > 0) ? COL_TEXT_BRIGHT : COL_TEXT_DIM;
                DrawButtonControl(hdc, hFontBold, cx + 102, cy + 42, 18, 20, "+", btnPlusTxt, btnPlusBg, COL_BORDER, BTN_FAC_WORKER, i, 1);
            } else {
                SetTextColor(hdc, COL_TEXT_DIM);
                TextOutA(hdc, cx + 6, cy + 45, "Automated Facility", 18);
            }

            // Upgrade Button
            if (fac->level < 3) {
                int uCost = fac->level * 30;
                char uBuf[24];
                sprintf(uBuf, "UPG LV%d (%dS)", fac->level + 1, uCost);
                COLORREF uBg = (g_state.scrap >= uCost) ? COL_BTN_BG : COL_DARK_CARD;
                COLORREF uTxt = (g_state.scrap >= uCost) ? COL_AMBER : COL_TEXT_DIM;
                DrawButtonControl(hdc, hFontSmall, cx + colW - 110, cy + 42, 104, 20, uBuf, uTxt, uBg, COL_BORDER, BTN_FAC_UPGRADE, i, 0);
            } else {
                SetTextColor(hdc, COL_TEXT_DIM);
                RECT rcMax = { cx + colW - 110, cy + 42, cx + colW - 6, cy + 62 };
                DrawTextA(hdc, "[MAX LEVEL]", -1, &rcMax, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }
    } else {
        // Room Construction Blueprints (2 Columns)
        int colW = (w - 10) / 2;
        int cardH = 92;
        int gapY = 8;

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
            TextOutA(hdc, cx + 8, cy + 6, bp->name, (int)strlen(bp->name));

            SelectObject(hdc, hFontSmall);
            if (bp->built) {
                SetTextColor(hdc, COL_GREEN);
                TextOutA(hdc, cx + colW - 55, cy + 6, "BUILT", 5);
            } else {
                char costBuf[16];
                sprintf(costBuf, "%d SCRAP", bp->cost);
                SetTextColor(hdc, COL_AMBER);
                TextOutA(hdc, cx + colW - 68, cy + 6, costBuf, (int)strlen(costBuf));
            }

            // Desc
            SetTextColor(hdc, COL_TEXT_DIM);
            TextOutA(hdc, cx + 8, cy + 25, bp->desc, (int)strlen(bp->desc));

            // Benefit
            SetTextColor(hdc, COL_GREEN);
            char benBuf[64];
            sprintf(benBuf, "Benefit: %s", bp->benefit);
            TextOutA(hdc, cx + 8, cy + 45, benBuf, (int)strlen(benBuf));

            // Action button
            if (bp->built) {
                SetTextColor(hdc, COL_GREEN);
                RECT rcOp = { cx + colW - 180, cy + 64, cx + colW - 10, cy + 86 };
                DrawTextA(hdc, "[ OPERATIONAL & ONLINE ]", -1, &rcOp, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            } else {
                char bBuf[32];
                sprintf(bBuf, "CONSTRUCT (%d SCRAP)", bp->cost);
                int canAfford = (g_state.scrap >= bp->cost);
                COLORREF cBg = canAfford ? RGB(25, 50, 30) : COL_DARK_CARD;
                COLORREF cTxt = canAfford ? COL_TEXT_BRIGHT : COL_TEXT_DIM;
                COLORREF cBdr = canAfford ? COL_GREEN : COL_BORDER;
                DrawButtonControl(hdc, hFontBold, cx + colW - 185, cy + 64, 178, 22, bBuf, cTxt, cBg, cBdr, BTN_CONSTRUCT_ROOM, i, 0);
            }
        }
    }
}

static void DrawSurvivorsView(HDC hdc, HFONT hFontBold, HFONT hFontSmall, int x, int y, int w, int h) {
    SelectObject(hdc, hFontBold);
    SetTextColor(hdc, COL_TEXT_BRIGHT);
    TextOutA(hdc, x, y, "SHELTER CITIZENS & HEALTH STATUS", 32);

    char capBuf[32];
    sprintf(capBuf, "Capacity: %d/%d Dwellers", g_state.population, g_state.maxPop);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x + w - 170, y, capBuf, (int)strlen(capBuf));

    int startY = y + 22;
    int cardH = 58;
    int gap = 6;

    for (int i = 0; i < g_state.numSurvivors; i++) {
        Survivor* s = &g_state.survivors[i];
        int cy = startY + i * (cardH + gap);

        DrawStyledBox(hdc, x, cy, w, cardH, COL_DARK_CARD, COL_BORDER);

        // Name & Role
        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, COL_TEXT_BRIGHT);
        TextOutA(hdc, x + 8, cy + 6, s->name, (int)strlen(s->name));

        char roleBuf[48];
        sprintf(roleBuf, "%s | S:%d A:%d I:%d", s->role, s->str, s->agi, s->inte);
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

        // Current Job & Reassignment Button
        char jobLabel[32];
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
            if (fac) sprintf(jobLabel, "[ %s ]", fac->name);
            else strcpy(jobLabel, "[ Assigned ]");
        }

        if (strcmp(s->job, "expedition") == 0) {
            DrawStyledBox(hdc, x + w - 190, cy + 14, 180, 28, COL_DARK_CARD, COL_BORDER);
            SelectObject(hdc, hFontSmall);
            SetTextColor(hdc, COL_AMBER);
            RECT rc = { x + w - 190, cy + 14, x + w - 10, cy + 42 };
            DrawTextA(hdc, "ON WASTELAND EXP", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        } else {
            DrawButtonControl(hdc, hFontSmall, x + w - 190, cy + 14, 180, 28, jobLabel, COL_TEXT_MAIN, COL_BTN_BG, COL_BORDER, BTN_SURV_JOB, i, 0);
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
    char actBuf[32];
    sprintf(actBuf, "Active Expeditions: %d", activeCount);
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, activeCount > 0 ? COL_AMBER : COL_TEXT_DIM);
    TextOutA(hdc, x + w - 160, y, actBuf, (int)strlen(actBuf));

    SetTextColor(hdc, COL_TEXT_DIM);
    TextOutA(hdc, x, y + 20, "Send unassigned survivors past the airlock to scavenge for food, scrap, and tech.", 81);

    int startY = y + 42;
    int cardH = 100;
    int gap = 12;
    int unassigned = GetUnassignedCount();

    for (int i = 0; i < g_state.numExpeditions; i++) {
        Expedition* exp = &g_state.expeditions[i];
        int cy = startY + i * (cardH + gap);

        DrawStyledBox(hdc, x, cy, w, cardH, COL_DARK_CARD, COL_BORDER);

        // Name
        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, COL_AMBER);
        TextOutA(hdc, x + 10, cy + 10, exp->name, (int)strlen(exp->name));

        // Risk & Duration
        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, COL_TEXT_DIM);
        char riskBuf[64];
        sprintf(riskBuf, "%s | Est. Duration: %d Cycle(s)", exp->risk, exp->duration);
        TextOutA(hdc, x + 10, cy + 30, riskBuf, (int)strlen(riskBuf));

        // Desc
        SetTextColor(hdc, COL_TEXT_MAIN);
        TextOutA(hdc, x + 10, cy + 50, exp->desc, (int)strlen(exp->desc));

        // Yield
        char yldBuf[64];
        sprintf(yldBuf, "Potential Yield: ~%d Food, ~%d Scrap", exp->potentialFood, exp->potentialScrap);
        SetTextColor(hdc, COL_TEXT_BRIGHT);
        TextOutA(hdc, x + 10, cy + 70, yldBuf, (int)strlen(yldBuf));

        // Button / Status
        if (strlen(exp->assignedScout) > 0) {
            char progBuf[48];
            sprintf(progBuf, "IN PROGRESS (%dd left)", exp->daysRemaining);
            DrawStyledBox(hdc, x + w - 180, cy + 34, 165, 32, COL_PANEL_BG, COL_AMBER);
            SelectObject(hdc, hFontBold);
            SetTextColor(hdc, COL_AMBER);
            RECT rc = { x + w - 180, cy + 34, x + w - 15, cy + 66 };
            DrawTextA(hdc, progBuf, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        } else {
            COLORREF btnBg = (unassigned > 0) ? RGB(25, 45, 30) : COL_DARK_CARD;
            COLORREF btnTxt = (unassigned > 0) ? COL_TEXT_BRIGHT : COL_TEXT_DIM;
            DrawButtonControl(hdc, hFontBold, x + w - 180, cy + 34, 165, 32, "DISPATCH SCOUT", btnTxt, btnBg, unassigned > 0 ? COL_GREEN : COL_BORDER, BTN_DISPATCH_SCOUT, i, 0);
        }
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
    TextOutA(hdc, x + 12, curY, "6. KEYBOARD SHORTCUTS:", 22); curY += lineH;
    SetTextColor(hdc, COL_TEXT_MAIN);
    TextOutA(hdc, x + 20, curY, "[SPACE] Advance Cycle | [1-5] Tabs | [T] Theme | [C] CRT | [A] Auto | [H] Help | [R] Reset", 90);
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
            } else if (wParam >= '1' && wParam <= '5') {
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
                g_state.currentTab = 4;
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
                        g_state.currentTab = 4;
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
                        if (g_state.scrap >= bp->cost && !bp->built && g_state.numFacilities < 16) {
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
                    } else if (bId == BTN_DISPATCH_SCOUT) {
                        Expedition* exp = &g_state.expeditions[p1];
                        Survivor* scout = GetFirstUnassignedSurvivor();
                        if (scout && strlen(exp->assignedScout) == 0) {
                            strcpy(scout->job, "expedition");
                            strcpy(exp->assignedScout, scout->id);
                            exp->daysRemaining = exp->duration;
                            char buf[128];
                            sprintf(buf, "Scout %s dispatched to %s.", scout->name, exp->name);
                            AddLog(buf, 4);
                            PlaySfx(4);
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
                "[1] FACILITIES & POWER",
                "[2] SURVIVOR ROSTER",
                "[3] WASTELAND SCAVENGING",
                "[4] OVERSEER DIRECTIVES",
                "[5] HELP / MANUAL"
            };
            int tabX = 10;
            int tabW = 126;
            for (int t = 0; t < 5; t++) {
                int active = (g_state.currentTab == t);
                COLORREF bg = active ? COL_BTN_HOVER : COL_DARK_CARD;
                COLORREF bdr = active ? COL_BORDER_HI : COL_BORDER;
                COLORREF txt = active ? COL_TEXT_BRIGHT : COL_TEXT_DIM;
                DrawButtonControl(memDC, hFontSmall, tabX, 114, tabW, 26, tabNames[t], txt, bg, bdr, BTN_TAB, t, 0);
                tabX += tabW + 5;
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
                DrawPoliciesView(memDC, hFontBold, hFontSmall, 20, areaY + 10, contentW - 20, areaH - 20);
            } else if (g_state.currentTab == 4) {
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
