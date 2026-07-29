#include <windows.h>

#pragma function(memset, memcpy)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* p = (char*)dest;
    while (count--) *p++ = (char)c;
    return dest;
}

void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}

#define TOTAL_ELEMENTS 56
#define TOTAL_RECIPES 52
#define TOTAL_TIERS 5
#define GRID_SIZE 10

typedef struct {
    int id;
    const char* name;
    int tier;
    int isBasic;
    const char* lore;
} Element;

static const Element g_Elements[TOTAL_ELEMENTS] = {
    // Tier 1: Basic (4)
    { 0, "Fire", 1, 1, "The primordial ember of creation, consuming all in heat and light." },
    { 1, "Water", 1, 1, "The flowing essence of life, cool, adaptable, and perpetual." },
    { 2, "Earth", 1, 1, "The steadfast soil and stone that anchors the living realm." },
    { 3, "Air", 1, 1, "The invisible current of wind that whispers through the sky." },

    // Tier 2: Nature (12)
    { 4, "Steam", 2, 0, "Scalded vapor born of fire's passion meeting water's grace." },
    { 5, "Lava", 2, 0, "Molten earth coursing with unbridled subterranean fire." },
    { 6, "Energy", 2, 0, "Raw electrical spark crackling with volatile arcane power." },
    { 7, "Mud", 2, 0, "A soft slurry of earth and water, pliable and fertile." },
    { 8, "Rain", 2, 0, "Condensing air showering sweet celestial waters upon the realm." },
    { 9, "Dust", 2, 0, "Finely particulate earth suspended in lazy air currents." },
    { 10, "Stone", 2, 0, "Cooled volcanic crust hardened into indestructible stone." },
    { 11, "Plant", 2, 0, "Green life springing forth from rain-soaked earth." },
    { 12, "Cloud", 2, 0, "Vaporous mist floating effortlessly in the high atmosphere." },
    { 13, "Charcoal", 2, 0, "Scorched plant matter glowing with lingering thermal embers." },
    { 14, "Swamp", 2, 0, "Mire of decaying flora and murky waters teeming with life." },
    { 15, "Tree", 2, 0, "Ancient tall flora rooted deep in earth drinking rain." },

    // Tier 3: Metallurgy (12)
    { 16, "Metal", 3, 0, "Refined ore forged by blazing fires from dense stone." },
    { 17, "Sand", 3, 0, "Eroded stone ground into fine golden grains by wind." },
    { 18, "Glass", 3, 0, "Vitreous crystal fused from superheated silica sand." },
    { 19, "Rust", 3, 0, "Corroded metal decaying slowly under moisture's touch." },
    { 20, "Blade", 3, 0, "Honed metal edge shaped by master smiths for battle." },
    { 21, "Boiler", 3, 0, "Heavy iron vessel generating pressurized steam power." },
    { 22, "Electricity", 3, 0, "Harnessing raw energy through conductive metallic veins." },
    { 23, "Wire", 3, 0, "Drawn metallic strand channeling electric currents." },
    { 24, "Gunpowder", 3, 0, "Volatile mixture of charcoal, sulfur, and fine dust." },
    { 25, "Explosion", 3, 0, "Catastrophic detonation releasing heat and pressure." },
    { 26, "Magnet", 3, 0, "Ferromagnetic metal possessing mysterious polarity." },
    { 27, "Clay", 3, 0, "Dense malleable earth baked into pottery by fire." },

    // Tier 4: Arcane (14)
    { 28, "Life", 4, 0, "The divine spark animating inert energy and water." },
    { 29, "Golem", 4, 0, "Inanimate metal automaton brought to life by alchemy." },
    { 30, "Magic", 4, 0, "Supernatural force woven from raw energetic currents." },
    { 31, "Mana", 4, 0, "Liquid magical essence flowing from high arcane fountains." },
    { 32, "Phoenix", 4, 0, "Immortal avian spirit born of sacred flame and life." },
    { 33, "Dragon", 4, 0, "Mythical apex behemoth surging with lava and life." },
    { 34, "Crystal", 4, 0, "Gemstone infused with pure resonant magic lattice." },
    { 35, "Rune", 4, 0, "Sacred glyph carved in stone and bound with mana." },
    { 36, "Potion", 4, 0, "Herbal brew distilled with mana for potent effects." },
    { 37, "Elixir", 4, 0, "Concentrated magical remedy granting sublime power." },
    { 38, "Nether", 4, 0, "Infernal planar realm kindled by nether flames." },
    { 39, "Shadow", 4, 0, "Ethereal gloom born where light is eclipsed by magic." },
    { 40, "Light", 4, 0, "Illuminating radiance bursting from magic and fire." },
    { 41, "Spirit", 4, 0, "Unbound ghostly form lingering between life and air." },

    // Tier 5: Celestial (14)
    { 42, "Sun", 5, 0, "Blazing solar orb illuminating cosmic space." },
    { 43, "Moon", 5, 0, "Luminous celestial mirror reflecting solar light." },
    { 44, "Star", 5, 0, "Distant thermonuclear furnace radiating starlight." },
    { 45, "Comet", 5, 0, "Icy celestial voyager trailing water and starlight." },
    { 46, "Meteor", 5, 0, "Flaming cosmic rock hurtling through celestial space." },
    { 47, "Galaxy", 5, 0, "Spiral vortex of countless stars bound in cloud mist." },
    { 48, "Cosmos", 5, 0, "The infinite expanse enriched with cosmic magic." },
    { 49, "Eclipse", 5, 0, "Mystical alignment when moon obscures the sun." },
    { 50, "Gold", 5, 0, "Precious golden element forged in solar rays." },
    { 51, "Starlight", 5, 0, "Pure concentrated rays beaming from stars." },
    { 52, "Supernova", 5, 0, "Cataclysmic explosion seeding cosmos with gold." },
    { 53, "Black Hole", 5, 0, "Gravitational singularity swallowing light." },
    { 54, "Time", 5, 0, "Temporal river flowing continuously through cosmos." },
    { 55, "Eternity", 5, 0, "Infinite timeless state beyond magic and space." }
};

typedef struct {
    int id;
    const char* name;
    int threshold;
    COLORREF color;
} TierInfo;

static const TierInfo g_Tiers[TOTAL_TIERS] = {
    { 1, "Basic", 0, RGB(52, 152, 219) },
    { 2, "Nature", 4, RGB(46, 204, 113) },
    { 3, "Metallurgy", 12, RGB(230, 126, 34) },
    { 4, "Arcane", 24, RGB(155, 89, 182) },
    { 5, "Celestial", 38, RGB(241, 196, 15) }
};

typedef struct {
    int ingredient1;
    int ingredient2;
    int result;
} Recipe;

static const Recipe g_Recipes[TOTAL_RECIPES] = {
    // T2 Nature (12)
    { 1, 0, 4 },   // Water + Fire -> Steam
    { 2, 0, 5 },   // Earth + Fire -> Lava
    { 0, 3, 6 },   // Fire + Air -> Energy
    { 2, 1, 7 },   // Earth + Water -> Mud
    { 1, 3, 8 },   // Water + Air -> Rain
    { 2, 3, 9 },   // Earth + Air -> Dust
    { 5, 3, 10 },  // Lava + Air -> Stone
    { 2, 8, 11 },  // Earth + Rain -> Plant
    { 3, 8, 12 },  // Air + Rain -> Cloud
    { 0, 11, 13 }, // Fire + Plant -> Charcoal
    { 7, 11, 14 }, // Mud + Plant -> Swamp
    { 11, 2, 15 }, // Plant + Earth -> Tree

    // T3 Metallurgy (12)
    { 10, 0, 16 }, // Stone + Fire -> Metal
    { 10, 3, 17 }, // Stone + Air -> Sand
    { 17, 0, 18 }, // Sand + Fire -> Glass
    { 16, 1, 19 }, // Metal + Water -> Rust
    { 16, 10, 20 },// Metal + Stone -> Blade
    { 16, 4, 21 }, // Metal + Steam -> Boiler
    { 16, 6, 22 }, // Metal + Energy -> Electricity
    { 22, 0, 23 }, // Electricity + Fire -> Wire
    { 0, 9, 24 },  // Fire + Dust -> Gunpowder
    { 24, 0, 25 }, // Gunpowder + Fire -> Explosion
    { 16, 22, 26 },// Metal + Electricity -> Magnet
    { 7, 0, 27 },  // Mud + Fire -> Clay

    // T4 Arcane (14)
    { 6, 1, 28 },  // Energy + Water -> Life
    { 28, 16, 29 },// Life + Metal -> Golem
    { 6, 3, 30 },  // Energy + Air -> Magic
    { 30, 1, 31 }, // Magic + Water -> Mana
    { 0, 28, 32 }, // Fire + Life -> Phoenix
    { 5, 28, 33 }, // Lava + Life -> Dragon
    { 10, 30, 34 },// Stone + Magic -> Crystal
    { 10, 31, 35 },// Stone + Mana -> Rune
    { 11, 31, 36 },// Plant + Mana -> Potion
    { 36, 30, 37 },// Potion + Magic -> Elixir
    { 5, 30, 38 }, // Lava + Magic -> Nether
    { 3, 30, 39 }, // Air + Magic -> Shadow
    { 0, 30, 40 }, // Fire + Magic -> Light
    { 28, 3, 41 }, // Life + Air -> Spirit

    // T5 Celestial (14)
    { 0, 40, 42 }, // Fire + Light -> Sun
    { 10, 40, 43 },// Stone + Light -> Moon
    { 42, 6, 44 }, // Sun + Energy -> Star
    { 44, 1, 45 }, // Star + Water -> Comet
    { 44, 10, 46 },// Star + Stone -> Meteor
    { 44, 12, 47 },// Star + Cloud -> Galaxy
    { 47, 30, 48 },// Galaxy + Magic -> Cosmos
    { 42, 43, 49 },// Sun + Moon -> Eclipse
    { 16, 42, 50 },// Metal + Sun -> Gold
    { 44, 40, 51 },// Star + Light -> Starlight
    { 44, 25, 52 },// Star + Explosion -> Supernova
    { 48, 39, 53 },// Cosmos + Shadow -> Black Hole
    { 48, 6, 54 }, // Cosmos + Energy -> Time
    { 54, 30, 55 } // Time + Magic -> Eternity
};

#define TOTAL_PATRONS 10
static const char* g_Patrons[TOTAL_PATRONS] = {
    "Archmage Vaelen",
    "Royal Blacksmith Thorin",
    "Grand Apothecary Lysandra",
    "Shadow Broker Corvus",
    "Elder Botanist Rowan",
    "High Envoy Cassian",
    "Dragon Slayer Ignis",
    "Astrologer Stella",
    "Master Merchant Barnaby",
    "Guildmaster Aurelius"
};

typedef struct {
    int patronIdx;
    int targetId;
    int goldReward;
    int xpReward;
} Quest;

typedef struct {
    int discovered[TOTAL_ELEMENTS];
    int unlockedTiers[TOTAL_TIERS];
    int discoveredCount;
    int essence;
    int dust;
    int gold;
    int guildXP;
    int guildLevel;
    Quest quests[3];
    int slot1;
    int slot2;
    int selectedEquipment; // 0 = Crucible, 1 = Retort, 2 = Alembic, 3 = Anvil, 4 = Quests, 5 = Workshop, 6 = Potions
    int selectedTierFilter; // 0 = All, 1..5 = T1..T5
    int currentPage;
    int buttonElemMap[GRID_SIZE];
    int upgradeCrucibleCap;
    int upgradeEssenceYield;
    int upgradeAutoSorter;
    int upgradeCatalystSpeed;
    int potionStrength;
    int potionInvisibility;
    int potionMana;
    int potionLife;
    int buffStrengthTimer;
    int buffInvisibilityTimer;
    int buffManaTimer;
    int buffLifeTimer;
    int craftCounts[TOTAL_ELEMENTS];
    int useCounts[TOTAL_ELEMENTS];
    int codexFilter; // 0 = All, 1 = Discovered, 2 = Missing
    int selectedCodexElem;
    int gameMode;           // 0 = Classic, 1 = Blitz, 2 = Puzzle
    int blitzTimeLeft;      // Remaining Blitz seconds (60)
    int blitzScore;         // Current Blitz score
    int blitzHighScore;     // Blitz High Score
    int blitzActive;        // 1 if active, 0 if inactive
    int puzzleTargetId;     // Target element ID
    int puzzleMoves;        // Moves taken in current puzzle
    int puzzleSolvedCount;  // Puzzles solved count
    int puzzleHighScore;    // Puzzle High Score
    char lastStatus[128];
    char searchFilter[64];
} AlchemyState;

static const int g_CrucibleCapCosts[5] = { 50, 100, 200, 350, 500 };
static const int g_EssenceYieldCosts[5] = { 40, 80, 160, 300, 500 };
static const int g_AutoSorterCosts[5] = { 60, 120, 250, 450, 700 };
static const int g_CatalystSpeedCosts[5] = { 45, 90, 180, 320, 500 };

static AlchemyState g_State;
static HWND g_hGridButtons[GRID_SIZE];
static HWND g_hTierButtons[TOTAL_TIERS + 1];
static HWND g_hEquipButtons[8];
static HWND g_hModeButtons[3];
static HWND g_hBlitzStartButton = NULL;
static HWND g_hPuzzleSkipButton = NULL;
static HWND g_hCodexFilterBtns[3];
static HWND g_hPotionDrinkButtons[4];
static HWND g_hQuestTurnInButtons[3];
static HWND g_hQuestRerollButton = NULL;
static HWND g_hUpgradeButtons[4];
static HWND g_hAutoFillButton = NULL;
static HWND g_hMainActionButton = NULL;
static HWND g_hSlot1Button = NULL;
static HWND g_hSlot2Button = NULL;
static HWND g_hSearchEdit = NULL;
static HWND g_hJournalEdit = NULL;
static HWND g_hPrevButton = NULL;
static HWND g_hNextButton = NULL;
static HWND g_hPageText = NULL;

static HBRUSH hBgBrush = NULL;
static HBRUSH hPanelBrush = NULL;
static HBRUSH hCrucibleBrush = NULL;
static HBRUSH hVesselBrush = NULL;
static HBRUSH hGoldBadgeBrush = NULL;
static HPEN hVesselPen = NULL;
static HPEN hGoldPen = NULL;
static HPEN hPurplePen = NULL;
static HPEN hInnerGlowPen = NULL;
static HFONT hTitleFont = NULL;
static HFONT hHeaderFont = NULL;
static HFONT hUIFont = NULL;
static HFONT hSlotFont = NULL;
static HFONT hBadgeFont = NULL;

static void AddJournalLog(const char* text) {
    if (!g_hJournalEdit) return;
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buf[512];
    wsprintfA(buf, "[%02d:%02d:%02d] %s\r\n\r\n", st.wHour, st.wMinute, st.wSecond, text);

    int len = GetWindowTextLengthA(g_hJournalEdit);
    SendMessageA(g_hJournalEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessageA(g_hJournalEdit, EM_REPLACESEL, FALSE, (LPARAM)buf);
    SendMessageA(g_hJournalEdit, EM_SCROLLCARET, 0, 0);
}

static void ToLowerStr(char* dest, const char* src) {
    while (*src) {
        char c = *src++;
        if (c >= 'A' && c <= 'Z') c += ('a' - 'A');
        *dest++ = c;
    }
    *dest = '\0';
}

static int StrContainsIgnoreCase(const char* haystack, const char* needle) {
    if (!needle || !needle[0]) return 1;
    char hLower[128], nLower[128];
    ToLowerStr(hLower, haystack);
    ToLowerStr(nLower, needle);

    for (int i = 0; hLower[i] != '\0'; i++) {
        int j = 0;
        while (hLower[i + j] != '\0' && nLower[j] != '\0' && hLower[i + j] == nLower[j]) {
            j++;
        }
        if (nLower[j] == '\0') return 1;
    }
    return 0;
}

static void PlayDiscoveryFanfare() {
    Beep(523, 70);   // C5
    Beep(659, 70);   // E5
    Beep(784, 70);   // G5
    Beep(1046, 120); // C6
}

static void PlayTierUnlockFanfare() {
    Beep(440, 60);  // A4
    Beep(554, 60);  // C#5
    Beep(659, 60);  // E5
    Beep(880, 150); // A5
}

static void UpdateGrimoireGrid() {
    int matches[TOTAL_ELEMENTS];
    int matchCount = 0;

    for (int i = 0; i < TOTAL_ELEMENTS; i++) {
        int isDisc = g_State.discovered[i];
        if (g_State.selectedEquipment == 7) {
            // Codex Mode
            if (g_State.codexFilter == 1 && !isDisc) continue;
            if (g_State.codexFilter == 2 && isDisc) continue;
        } else {
            // Standard Grimoire Mode
            if (!isDisc) continue;
        }

        if (g_State.selectedTierFilter > 0 && g_Elements[i].tier != g_State.selectedTierFilter) continue;
        if (g_State.searchFilter[0] != '\0' && !StrContainsIgnoreCase(g_Elements[i].name, g_State.searchFilter) && !StrContainsIgnoreCase(g_Elements[i].lore, g_State.searchFilter)) continue;

        matches[matchCount++] = i;
    }

    int totalPages = (matchCount + GRID_SIZE - 1) / GRID_SIZE;
    if (totalPages < 1) totalPages = 1;
    if (g_State.currentPage >= totalPages) g_State.currentPage = totalPages - 1;
    if (g_State.currentPage < 0) g_State.currentPage = 0;

    int startIndex = g_State.currentPage * GRID_SIZE;

    for (int k = 0; k < GRID_SIZE; k++) {
        int matchIdx = startIndex + k;
        if (matchIdx < matchCount) {
            int elemIdx = matches[matchIdx];
            g_State.buttonElemMap[k] = elemIdx;
            char btnText[64];
            if (g_State.discovered[elemIdx]) {
                if (g_State.selectedEquipment == 7) {
                    wsprintfA(btnText, "[T%d] %s (x%d)", g_Elements[elemIdx].tier, g_Elements[elemIdx].name, g_State.craftCounts[elemIdx]);
                } else {
                    wsprintfA(btnText, "[T%d] %s", g_Elements[elemIdx].tier, g_Elements[elemIdx].name);
                }
            } else {
                wsprintfA(btnText, "[T%d] ??? (Locked)", g_Elements[elemIdx].tier);
            }
            SetWindowTextA(g_hGridButtons[k], btnText);
            ShowWindow(g_hGridButtons[k], SW_SHOW);
        } else {
            g_State.buttonElemMap[k] = -1;
            ShowWindow(g_hGridButtons[k], SW_HIDE);
        }
    }

    if (g_hPageText) {
        char pageStr[32];
        wsprintfA(pageStr, "%d/%d", g_State.currentPage + 1, totalPages);
        SetWindowTextA(g_hPageText, pageStr);
    }
}

static int CheckTierUnlocks() {
    int highestTier = 1;
    for (int t = 0; t < TOTAL_TIERS; t++) {
        if (g_State.discoveredCount >= g_Tiers[t].threshold) {
            highestTier = g_Tiers[t].id;
            if (!g_State.unlockedTiers[t]) {
                g_State.unlockedTiers[t] = 1;
                char msg[256];
                wsprintfA(msg, "🔓 TIER UNLOCKED! Tier %d (%s) is now accessible!", g_Tiers[t].id, g_Tiers[t].name);
                AddJournalLog(msg);
                PlayTierUnlockFanfare();
            }
        }
    }
    return highestTier;
}

static unsigned int g_RandSeed = 12345;

static int FastRand() {
    g_RandSeed = g_RandSeed * 1103515245 + 12345;
    return (int)((g_RandSeed / 65536) & 0x7FFF);
}

static void GenerateQuest(int slotIdx) {
    if (slotIdx < 0 || slotIdx >= 3) return;
    g_State.quests[slotIdx].patronIdx = FastRand() % TOTAL_PATRONS;

    int highestTier = 1;
    for (int t = 0; t < TOTAL_TIERS; t++) {
        if (g_State.discoveredCount >= g_Tiers[t].threshold) highestTier = g_Tiers[t].id;
    }
    int candidates[TOTAL_ELEMENTS];
    int candCount = 0;
    for (int i = 0; i < TOTAL_ELEMENTS; i++) {
        if (g_Elements[i].tier <= highestTier) {
            candidates[candCount++] = i;
        }
    }
    if (candCount == 0) candidates[candCount++] = 0;
    int target = candidates[FastRand() % candCount];

    g_State.quests[slotIdx].targetId = target;
    g_State.quests[slotIdx].goldReward = g_Elements[target].tier * 35 + (FastRand() % 20);
    g_State.quests[slotIdx].xpReward = g_Elements[target].tier * 50;
}

static void UpdateEquipmentUI(HWND hwnd) {
    int isQuests = (g_State.selectedEquipment == 4);
    int isWorkshop = (g_State.selectedEquipment == 5);
    int isBrewing = (g_State.selectedEquipment == 6);
    int isCodex = (g_State.selectedEquipment == 7);
    int isCrucible = (g_State.selectedEquipment == 0);

    if (isQuests || isWorkshop || isCodex) {
        if (g_hSlot1Button) ShowWindow(g_hSlot1Button, SW_HIDE);
        if (g_hSlot2Button) ShowWindow(g_hSlot2Button, SW_HIDE);
        if (g_hMainActionButton) ShowWindow(g_hMainActionButton, SW_HIDE);
    } else {
        if (g_hSlot1Button) ShowWindow(g_hSlot1Button, SW_SHOW);
        if (g_hSlot2Button) ShowWindow(g_hSlot2Button, SW_SHOW);
        if (g_hMainActionButton) ShowWindow(g_hMainActionButton, SW_SHOW);
    }

    for (int cb = 0; cb < 3; cb++) {
        if (g_hCodexFilterBtns[cb]) ShowWindow(g_hCodexFilterBtns[cb], isCodex ? SW_SHOW : SW_HIDE);
    }

    if (isQuests || isWorkshop) {
        if (g_hSlot1Button) ShowWindow(g_hSlot1Button, SW_HIDE);
        if (g_hSlot2Button) ShowWindow(g_hSlot2Button, SW_HIDE);
        if (g_hMainActionButton) ShowWindow(g_hMainActionButton, SW_HIDE);
    } else {
        if (g_hSlot1Button) ShowWindow(g_hSlot1Button, SW_SHOW);
        if (g_hSlot2Button) ShowWindow(g_hSlot2Button, SW_SHOW);
        if (g_hMainActionButton) ShowWindow(g_hMainActionButton, SW_SHOW);
    }

    if (g_hAutoFillButton) {
        if (isCrucible && g_State.upgradeAutoSorter > 0) {
            ShowWindow(g_hAutoFillButton, SW_SHOW);
        } else {
            ShowWindow(g_hAutoFillButton, SW_HIDE);
        }
    }

    for (int p = 0; p < 4; p++) {
        if (g_hPotionDrinkButtons[p]) {
            ShowWindow(g_hPotionDrinkButtons[p], isBrewing ? SW_SHOW : SW_HIDE);
            if (isBrewing) {
                int count = 0;
                if (p == 0) count = g_State.potionStrength;
                else if (p == 1) count = g_State.potionInvisibility;
                else if (p == 2) count = g_State.potionMana;
                else if (p == 3) count = g_State.potionLife;
                EnableWindow(g_hPotionDrinkButtons[p], count > 0 ? TRUE : FALSE);
            }
        }
    }

    if (isQuests) {
        for (int q = 0; q < 3; q++) {
            if (g_hQuestTurnInButtons[q]) {
                ShowWindow(g_hQuestTurnInButtons[q], SW_SHOW);
                int targetId = g_State.quests[q].targetId;
                if (g_State.discovered[targetId]) {
                    EnableWindow(g_hQuestTurnInButtons[q], TRUE);
                    SetWindowTextA(g_hQuestTurnInButtons[q], "Turn In");
                } else {
                    EnableWindow(g_hQuestTurnInButtons[q], FALSE);
                    SetWindowTextA(g_hQuestTurnInButtons[q], "Locked");
                }
            }
        }
        if (g_hQuestRerollButton) ShowWindow(g_hQuestRerollButton, SW_SHOW);
    } else {
        for (int q = 0; q < 3; q++) {
            if (g_hQuestTurnInButtons[q]) ShowWindow(g_hQuestTurnInButtons[q], SW_HIDE);
        }
        if (g_hQuestRerollButton) ShowWindow(g_hQuestRerollButton, SW_HIDE);
    }

    if (isWorkshop) {
        int lvls[4] = { g_State.upgradeCrucibleCap, g_State.upgradeEssenceYield, g_State.upgradeAutoSorter, g_State.upgradeCatalystSpeed };
        int costs[4] = {
            (lvls[0] < 5 ? g_CrucibleCapCosts[lvls[0]] : 0),
            (lvls[1] < 5 ? g_EssenceYieldCosts[lvls[1]] : 0),
            (lvls[2] < 5 ? g_AutoSorterCosts[lvls[2]] : 0),
            (lvls[3] < 5 ? g_CatalystSpeedCosts[lvls[3]] : 0)
        };

        for (int u = 0; u < 4; u++) {
            if (g_hUpgradeButtons[u]) {
                ShowWindow(g_hUpgradeButtons[u], SW_SHOW);
                if (lvls[u] >= 5) {
                    SetWindowTextA(g_hUpgradeButtons[u], "MAX");
                    EnableWindow(g_hUpgradeButtons[u], FALSE);
                } else if (g_State.gold < costs[u]) {
                    SetWindowTextA(g_hUpgradeButtons[u], "Upgrade");
                    EnableWindow(g_hUpgradeButtons[u], FALSE);
                } else {
                    SetWindowTextA(g_hUpgradeButtons[u], "Upgrade");
                    EnableWindow(g_hUpgradeButtons[u], TRUE);
                }
            }
        }
        for (int u = 0; u < 4; u++) {
            if (g_hUpgradeButtons[u]) ShowWindow(g_hUpgradeButtons[u], SW_HIDE);
        }
    }

    if (g_hBlitzStartButton) {
        ShowWindow(g_hBlitzStartButton, (g_State.gameMode == 1) ? SW_SHOW : SW_HIDE);
    }
    if (g_hPuzzleSkipButton) {
        ShowWindow(g_hPuzzleSkipButton, (g_State.gameMode == 2) ? SW_SHOW : SW_HIDE);
    }
}

static void InitGameState() {
    g_RandSeed = GetTickCount();
    memset(&g_State, 0, sizeof(AlchemyState));
    g_State.discovered[0] = 1; // Fire
    g_State.discovered[1] = 1; // Water
    g_State.discovered[2] = 1; // Earth
    g_State.discovered[3] = 1; // Air
    g_State.unlockedTiers[0] = 1; // Basic
    g_State.unlockedTiers[1] = 1; // Nature (threshold 4)
    g_State.discoveredCount = 4;
    g_State.essence = 100;
    g_State.dust = 100;
    g_State.gold = 100;
    g_State.guildXP = 0;
    g_State.guildLevel = 1;
    g_State.slot1 = -1;
    g_State.slot2 = -1;
    g_State.selectedTierFilter = 0;
    g_State.currentPage = 0;
    g_State.gameMode = 0;
    g_State.blitzTimeLeft = 60;
    g_State.blitzScore = 0;
    g_State.blitzHighScore = 0;
    g_State.blitzActive = 0;
    g_State.puzzleTargetId = 4; // Steam
    g_State.puzzleMoves = 0;
    g_State.puzzleSolvedCount = 0;
    g_State.puzzleHighScore = 0;
    lstrcpyA(g_State.lastStatus, "Transmutation Crucible Ready");
    g_State.searchFilter[0] = '\0';
    for (int q = 0; q < 3; q++) {
        GenerateQuest(q);
    }
}

static void UpdateSlotButtonText() {
    if (g_hSlot1Button) {
        if (g_State.slot1 >= 0 && g_State.slot1 < TOTAL_ELEMENTS) {
            SetWindowTextA(g_hSlot1Button, g_Elements[g_State.slot1].name);
        } else {
            SetWindowTextA(g_hSlot1Button, "[ Slot 1 ]");
        }
    }
    if (g_hSlot2Button) {
        if (g_State.slot2 >= 0 && g_State.slot2 < TOTAL_ELEMENTS) {
            SetWindowTextA(g_hSlot2Button, g_Elements[g_State.slot2].name);
        } else {
            SetWindowTextA(g_hSlot2Button, "[ Slot 2 ]");
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            InitGameState();

            hBgBrush = CreateSolidBrush(RGB(9, 9, 20));          // Deep violet/slate
            hPanelBrush = CreateSolidBrush(RGB(22, 21, 46));       // Mystical panel background
            hCrucibleBrush = CreateSolidBrush(RGB(29, 24, 61));    // Arcane laboratory crucible
            hVesselBrush = CreateSolidBrush(RGB(55, 38, 95));      // Arcane purple vessel
            hGoldBadgeBrush = CreateSolidBrush(RGB(45, 35, 12));   // Golden badge fill
            hVesselPen = CreatePen(PS_DASH, 2, RGB(155, 89, 182));
            hPurplePen = CreatePen(PS_SOLID, 2, RGB(155, 89, 182));
            hGoldPen = CreatePen(PS_SOLID, 2, RGB(243, 156, 18));
            hInnerGlowPen = CreatePen(PS_SOLID, 1, RGB(241, 196, 15));

            hTitleFont = CreateFontA(21, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hHeaderFont = CreateFontA(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hUIFont = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hSlotFont = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hBadgeFont = CreateFontA(12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

            // Tier Filter Buttons
            g_hTierButtons[0] = CreateWindowA("BUTTON", "All", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 30, 96, 35, 22, hwnd, (HMENU)500, NULL, NULL);
            for (int t = 1; t <= TOTAL_TIERS; t++) {
                char tStr[16];
                wsprintfA(tStr, "T%d", t);
                g_hTierButtons[t] = CreateWindowA("BUTTON", tStr, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 30 + t * 37, 96, 35, 22, hwnd, (HMENU)(UINT_PTR)(500 + t), NULL, NULL);
            }

            // Grimoire Search Input
            g_hSearchEdit = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                30, 122, 225, 24, hwnd, (HMENU)401, NULL, NULL);

            // Paginated Grid Buttons (2 columns of 5)
            for (int k = 0; k < GRID_SIZE; k++) {
                int col = k % 2;
                int row = k / 2;
                int x = 30 + col * 115;
                int y = 152 + row * 38;
                g_hGridButtons[k] = CreateWindowA("BUTTON", "",
                    WS_CHILD | BS_PUSHBUTTON,
                    x, y, 110, 34, hwnd, (HMENU)(UINT_PTR)(100 + k), NULL, NULL);
            }

            // Pagination Controls
            g_hPrevButton = CreateWindowA("BUTTON", "< Prev", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 30, 348, 65, 26, hwnd, (HMENU)601, NULL, NULL);
            g_hPageText = CreateWindowA("STATIC", "1/1", WS_CHILD | WS_VISIBLE | SS_CENTER, 100, 353, 85, 20, hwnd, (HMENU)602, NULL, NULL);
            g_hNextButton = CreateWindowA("BUTTON", "Next >", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 190, 348, 65, 26, hwnd, (HMENU)602, NULL, NULL);

            UpdateGrimoireGrid();

            // Game Mode Selection Buttons
            g_hModeButtons[0] = CreateWindowA("BUTTON", "Classic", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 276, 70, 48, 22, hwnd, (HMENU)1200, NULL, NULL);
            g_hModeButtons[1] = CreateWindowA("BUTTON", "Blitz", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 327, 70, 42, 22, hwnd, (HMENU)1201, NULL, NULL);
            g_hModeButtons[2] = CreateWindowA("BUTTON", "Puzzle", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 372, 70, 48, 22, hwnd, (HMENU)1202, NULL, NULL);

            g_hBlitzStartButton = CreateWindowA("BUTTON", "▶️ Start Blitz", WS_CHILD | BS_PUSHBUTTON, 423, 70, 85, 22, hwnd, (HMENU)1203, NULL, NULL);
            g_hPuzzleSkipButton = CreateWindowA("BUTTON", "🔄 Skip Target", WS_CHILD | BS_PUSHBUTTON, 423, 70, 85, 22, hwnd, (HMENU)1204, NULL, NULL);

            // Laboratory Equipment Nav Buttons
            g_hEquipButtons[0] = CreateWindowA("BUTTON", "Crucible", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 276, 96, 33, 22, hwnd, (HMENU)700, NULL, NULL);
            g_hEquipButtons[1] = CreateWindowA("BUTTON", "Retort", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 310, 96, 31, 22, hwnd, (HMENU)701, NULL, NULL);
            g_hEquipButtons[2] = CreateWindowA("BUTTON", "Alembic", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 342, 96, 34, 22, hwnd, (HMENU)702, NULL, NULL);
            g_hEquipButtons[3] = CreateWindowA("BUTTON", "Anvil", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 377, 96, 28, 22, hwnd, (HMENU)703, NULL, NULL);
            g_hEquipButtons[4] = CreateWindowA("BUTTON", "Quests", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 406, 96, 34, 22, hwnd, (HMENU)704, NULL, NULL);
            g_hEquipButtons[5] = CreateWindowA("BUTTON", "Shop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 441, 96, 31, 22, hwnd, (HMENU)705, NULL, NULL);
            g_hEquipButtons[6] = CreateWindowA("BUTTON", "Potions", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 473, 96, 35, 22, hwnd, (HMENU)706, NULL, NULL);
            g_hEquipButtons[7] = CreateWindowA("BUTTON", "Codex", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 509, 96, 35, 22, hwnd, (HMENU)707, NULL, NULL);

            // Codex Filter Buttons (Initially hidden)
            g_hCodexFilterBtns[0] = CreateWindowA("BUTTON", "All", WS_CHILD | BS_PUSHBUTTON, 282, 120, 36, 22, hwnd, (HMENU)1100, NULL, NULL);
            g_hCodexFilterBtns[1] = CreateWindowA("BUTTON", "Disc", WS_CHILD | BS_PUSHBUTTON, 320, 120, 38, 22, hwnd, (HMENU)1101, NULL, NULL);
            g_hCodexFilterBtns[2] = CreateWindowA("BUTTON", "Lock", WS_CHILD | BS_PUSHBUTTON, 360, 120, 38, 22, hwnd, (HMENU)1102, NULL, NULL);

            // Potion Drink Buttons (Initially hidden)
            g_hPotionDrinkButtons[0] = CreateWindowA("BUTTON", "Drink Strength", WS_CHILD | BS_PUSHBUTTON, 282, 140, 108, 26, hwnd, (HMENU)1000, NULL, NULL);
            g_hPotionDrinkButtons[1] = CreateWindowA("BUTTON", "Drink Invis", WS_CHILD | BS_PUSHBUTTON, 394, 140, 108, 26, hwnd, (HMENU)1001, NULL, NULL);
            g_hPotionDrinkButtons[2] = CreateWindowA("BUTTON", "Drink Mana", WS_CHILD | BS_PUSHBUTTON, 282, 170, 108, 26, hwnd, (HMENU)1002, NULL, NULL);
            g_hPotionDrinkButtons[3] = CreateWindowA("BUTTON", "Drink Life", WS_CHILD | BS_PUSHBUTTON, 394, 170, 108, 26, hwnd, (HMENU)1003, NULL, NULL);

            SetTimer(hwnd, 1, 1000, NULL);

            // Workshop Upgrade Buttons (Initially hidden)
            g_hUpgradeButtons[0] = CreateWindowA("BUTTON", "Upgrade", WS_CHILD | BS_PUSHBUTTON, 432, 142, 65, 24, hwnd, (HMENU)900, NULL, NULL);
            g_hUpgradeButtons[1] = CreateWindowA("BUTTON", "Upgrade", WS_CHILD | BS_PUSHBUTTON, 432, 202, 65, 24, hwnd, (HMENU)901, NULL, NULL);
            g_hUpgradeButtons[2] = CreateWindowA("BUTTON", "Upgrade", WS_CHILD | BS_PUSHBUTTON, 432, 262, 65, 24, hwnd, (HMENU)902, NULL, NULL);
            g_hUpgradeButtons[3] = CreateWindowA("BUTTON", "Upgrade", WS_CHILD | BS_PUSHBUTTON, 432, 322, 65, 24, hwnd, (HMENU)903, NULL, NULL);

            // Auto-Fill Button for Crucible
            g_hAutoFillButton = CreateWindowA("BUTTON", "⚡ Auto", WS_CHILD | BS_PUSHBUTTON, 280, 315, 46, 28, hwnd, (HMENU)904, NULL, NULL);

            // Crucible Slots
            g_hSlot1Button = CreateWindowA("BUTTON", "[ Slot 1 ]", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 295, 200, 90, 50, hwnd, (HMENU)301, NULL, NULL);
            g_hSlot2Button = CreateWindowA("BUTTON", "[ Slot 2 ]", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 400, 200, 90, 50, hwnd, (HMENU)302, NULL, NULL);

            // Action Buttons
            g_hMainActionButton = CreateWindowA("BUTTON", "✨ Transmute", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 315, 265, 155, 40, hwnd, (HMENU)201, NULL, NULL);
            CreateWindowA("BUTTON", "Clear Crucible", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 330, 315, 125, 28, hwnd, (HMENU)202, NULL, NULL);
            CreateWindowA("BUTTON", "Reset Progress", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 330, 350, 125, 26, hwnd, (HMENU)203, NULL, NULL);

            // Quest Controls (Initially hidden until Quests tab selected)
            g_hQuestTurnInButtons[0] = CreateWindowA("BUTTON", "Turn In", WS_CHILD | BS_PUSHBUTTON, 430, 155, 65, 26, hwnd, (HMENU)800, NULL, NULL);
            g_hQuestTurnInButtons[1] = CreateWindowA("BUTTON", "Turn In", WS_CHILD | BS_PUSHBUTTON, 430, 235, 65, 26, hwnd, (HMENU)801, NULL, NULL);
            g_hQuestTurnInButtons[2] = CreateWindowA("BUTTON", "Turn In", WS_CHILD | BS_PUSHBUTTON, 430, 315, 65, 26, hwnd, (HMENU)802, NULL, NULL);
            g_hQuestRerollButton = CreateWindowA("BUTTON", "🔄 Reroll (15 Gold)", WS_CHILD | BS_PUSHBUTTON, 325, 385, 140, 28, hwnd, (HMENU)803, NULL, NULL);

            // Oracle & Research Hint Buttons
            CreateWindowA("BUTTON", "💡 Hint (20)", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 285, 440, 105, 30, hwnd, (HMENU)204, NULL, NULL);
            CreateWindowA("BUTTON", "👁️ Oracle (50)", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 395, 440, 105, 30, hwnd, (HMENU)205, NULL, NULL);

            // Journal Log Edit Control (Read-Only)
            g_hJournalEdit = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_BORDER,
                530, 96, 225, 415, hwnd, (HMENU)402, NULL, NULL);

            AddJournalLog("Welcome Apprentice Alchemist!\r\nSelect elements from your Grimoire to combine in the Crucible across 5 Tiers!");
            break;
        }

        case WM_TIMER: {
            int changed = 0;
            if (g_State.buffStrengthTimer > 0) { g_State.buffStrengthTimer--; changed = 1; }
            if (g_State.buffInvisibilityTimer > 0) { g_State.buffInvisibilityTimer--; changed = 1; }
            if (g_State.buffManaTimer > 0) { g_State.buffManaTimer--; changed = 1; }
            if (g_State.buffLifeTimer > 0) { g_State.buffLifeTimer--; changed = 1; }

            if (g_State.gameMode == 1 && g_State.blitzActive) {
                if (g_State.blitzTimeLeft > 0) {
                    g_State.blitzTimeLeft--;
                    if (g_State.blitzTimeLeft == 0) {
                        g_State.blitzActive = 0;
                        if (g_State.blitzScore > g_State.blitzHighScore) {
                            g_State.blitzHighScore = g_State.blitzScore;
                            char logMsg[256];
                            wsprintfA(logMsg, "🏆 NEW BLITZ HIGH SCORE! You achieved %d Blitz Points!", g_State.blitzScore);
                            AddJournalLog(logMsg);
                            PlayDiscoveryFanfare();
                        } else {
                            char logMsg[256];
                            wsprintfA(logMsg, "⏰ BLITZ TIME'S UP! Final Score: %d Pts (High Score: %d Pts)", g_State.blitzScore, g_State.blitzHighScore);
                            AddJournalLog(logMsg);
                            Beep(350, 150);
                        }
                    }
                }
                changed = 1;
            }
            if (changed) InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);

            // Search filter edit box changed
            if (id == 401 && code == EN_CHANGE) {
                GetWindowTextA(g_hSearchEdit, g_State.searchFilter, sizeof(g_State.searchFilter));
                g_State.currentPage = 0;
                UpdateGrimoireGrid();
            }
            // Tier Filter Buttons (500 = All, 501..505 = T1..T5)
            else if (id >= 500 && id <= 500 + TOTAL_TIERS) {
                g_State.selectedTierFilter = id - 500;
                g_State.currentPage = 0;
                UpdateGrimoireGrid();
                Beep(450, 40);
            }
            // Grid element buttons (100 to 109)
            else if (id >= 100 && id < 100 + GRID_SIZE) {
                int btnIdx = id - 100;
                int elemIdx = g_State.buttonElemMap[btnIdx];
                if (elemIdx >= 0 && elemIdx < TOTAL_ELEMENTS) {
                    if (g_State.selectedEquipment == 7) {
                        g_State.selectedCodexElem = elemIdx;
                        InvalidateRect(hwnd, NULL, TRUE);
                        Beep(600, 50);
                    } else if (g_State.discovered[elemIdx]) {
                        if (g_State.slot1 == -1) {
                            g_State.slot1 = elemIdx;
                        } else if (g_State.slot2 == -1) {
                            g_State.slot2 = elemIdx;
                        } else {
                            g_State.slot1 = elemIdx;
                        }
                        UpdateSlotButtonText();
                        Beep(520, 60);
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                }
            }
            // Page navigation
            else if (id == 601) { // Prev
                if (g_State.currentPage > 0) {
                    g_State.currentPage--;
                    UpdateGrimoireGrid();
                    Beep(400, 40);
                }
            }
            else if (id == 602) { // Next
                g_State.currentPage++;
                UpdateGrimoireGrid();
                Beep(400, 40);
            }
            // Clear Slot 1
            else if (id == 301) {
                g_State.slot1 = -1;
                UpdateSlotButtonText();
                Beep(350, 50);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Clear Slot 2
            else if (id == 302) {
                g_State.slot2 = -1;
                UpdateSlotButtonText();
                Beep(350, 50);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Equipment Selector (700 = Crucible, 701 = Retort, 702 = Alembic, 703 = Anvil, 704 = Quests, 705 = Shop, 706 = Potions)
            else if (id >= 700 && id <= 706) {
                g_State.selectedEquipment = id - 700;
                if (g_hMainActionButton) {
                    if (g_State.selectedEquipment == 0) SetWindowTextA(g_hMainActionButton, "✨ Transmute");
                    else if (g_State.selectedEquipment == 1) SetWindowTextA(g_hMainActionButton, "⚗️ Distill Retort");
                    else if (g_State.selectedEquipment == 2) SetWindowTextA(g_hMainActionButton, "🧪 Extract Alembic");
                    else if (g_State.selectedEquipment == 3) SetWindowTextA(g_hMainActionButton, "🔨 Crush Anvil");
                    else if (g_State.selectedEquipment == 6) SetWindowTextA(g_hMainActionButton, "🥣 Brew Elixir");
                }
                UpdateEquipmentUI(hwnd);
                Beep(450, 40);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Potion Drink Commands (1000 = Strength, 1001 = Invis, 1002 = Mana, 1003 = Life)
            else if (id >= 1000 && id <= 1003) {
                int pIdx = id - 1000;
                if (pIdx == 0 && g_State.potionStrength > 0) {
                    g_State.potionStrength--;
                    g_State.buffStrengthTimer += 60;
                    AddJournalLog("🧪 EFFECT TESTER: Consumed Strength Elixir! (+50% Yield Active 60s)");
                    wsprintfA(g_State.lastStatus, "Active Buff: Strength Elixir (60s)");
                    Beep(300, 50); Beep(500, 50); Beep(700, 70);
                } else if (pIdx == 1 && g_State.potionInvisibility > 0) {
                    g_State.potionInvisibility--;
                    g_State.buffInvisibilityTimer += 60;
                    AddJournalLog("🧪 EFFECT TESTER: Consumed Invisibility Elixir! (Stealth Aura Active 60s)");
                    wsprintfA(g_State.lastStatus, "Active Buff: Invisibility Elixir (60s)");
                    Beep(400, 50); Beep(600, 50); Beep(800, 70);
                } else if (pIdx == 2 && g_State.potionMana > 0) {
                    g_State.potionMana--;
                    g_State.buffManaTimer += 60;
                    AddJournalLog("🧪 EFFECT TESTER: Consumed Mana Elixir! (Mana Surge Active 60s)");
                    wsprintfA(g_State.lastStatus, "Active Buff: Mana Elixir (60s)");
                    Beep(500, 50); Beep(700, 50); Beep(900, 70);
                } else if (pIdx == 3 && g_State.potionLife > 0) {
                    g_State.potionLife--;
                    g_State.buffLifeTimer += 60;
                    AddJournalLog("🧪 EFFECT TESTER: Consumed Elixir of Life! (Divine Radiance Active 60s)");
                    wsprintfA(g_State.lastStatus, "Active Buff: Elixir of Life (60s)");
                    Beep(600, 50); Beep(800, 50); Beep(1000, 90);
                } else {
                    AddJournalLog("⚠️ No potions of this type in inventory!");
                    Beep(220, 100);
                }
                UpdateEquipmentUI(hwnd);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Workshop Upgrades (900, 901, 902, 903)
            else if (id >= 900 && id <= 903) {
                int uIdx = id - 900;
                int* pLvl = NULL;
                const int* pCosts = NULL;
                const char* uName = NULL;

                if (uIdx == 0) { pLvl = &g_State.upgradeCrucibleCap; pCosts = g_CrucibleCapCosts; uName = "Crucible Capacity"; }
                else if (uIdx == 1) { pLvl = &g_State.upgradeEssenceYield; pCosts = g_EssenceYieldCosts; uName = "Essence Extraction Yield"; }
                else if (uIdx == 2) { pLvl = &g_State.upgradeAutoSorter; pCosts = g_AutoSorterCosts; uName = "Auto-Sorter"; }
                else if (uIdx == 3) { pLvl = &g_State.upgradeCatalystSpeed; pCosts = g_CatalystSpeedCosts; uName = "Catalyst Speed"; }

                if (pLvl && pCosts && *pLvl < 5) {
                    int cost = pCosts[*pLvl];
                    if (g_State.gold >= cost) {
                        g_State.gold -= cost;
                        (*pLvl)++;
                        char logMsg[256];
                        wsprintfA(logMsg, "🧙 WORKSHOP UPGRADE: Upgraded %s to Level %d! (-%d Gold)", uName, *pLvl, cost);
                        AddJournalLog(logMsg);
                        wsprintfA(g_State.lastStatus, "Upgraded %s to Lvl %d!", uName, *pLvl);
                        Beep(523, 60); Beep(659, 60); Beep(784, 80);
                        UpdateEquipmentUI(hwnd);
                    } else {
                        AddJournalLog("⚠️ Not enough Gold for Workshop Upgrade!");
                        Beep(220, 100);
                    }
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Auto-Fill Crucible Button (904)
            else if (id == 904) {
                if (g_State.upgradeAutoSorter <= 0) {
                    AddJournalLog("⚠️ Unlock Auto-Sorter in the Enchanter Workshop first!");
                    Beep(220, 100);
                } else {
                    int matchIdx = -1;
                    int foundBothDiscovered = -1;

                    for (int r = 0; r < TOTAL_RECIPES; r++) {
                        int e1 = g_Recipes[r].ingredient1;
                        int e2 = g_Recipes[r].ingredient2;
                        int res = g_Recipes[r].result;
                        if (g_State.discovered[e1] && g_State.discovered[e2]) {
                            if (!g_State.discovered[res] && foundBothDiscovered == -1) {
                                foundBothDiscovered = r;
                            }
                            if (matchIdx == -1) matchIdx = r;
                        }
                    }

                    if (foundBothDiscovered >= 0) matchIdx = foundBothDiscovered;

                    if (matchIdx >= 0) {
                        g_State.slot1 = g_Recipes[matchIdx].ingredient1;
                        g_State.slot2 = g_Recipes[matchIdx].ingredient2;
                        UpdateSlotButtonText();
                        char logMsg[256];
                        wsprintfA(logMsg, "⚡ AUTO-SORTER: Placed %s and %s into Crucible!",
                            g_Elements[g_State.slot1].name, g_Elements[g_State.slot2].name);
                        AddJournalLog(logMsg);
                        wsprintfA(g_State.lastStatus, "Auto-Sorter Loaded %s + %s",
                            g_Elements[g_State.slot1].name, g_Elements[g_State.slot2].name);
                        Beep(600, 80);
                    } else {
                        AddJournalLog("⚡ Auto-Sorter: No valid ingredient combinations found in Grimoire!");
                        Beep(220, 100);
                    }
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Quest Turn In Buttons (800, 801, 802)
            else if (id >= 800 && id <= 802) {
                int qIdx = id - 800;
                int targetId = g_State.quests[qIdx].targetId;
                if (g_State.discovered[targetId]) {
                    int gRew = g_State.quests[qIdx].goldReward;
                    int xRew = g_State.quests[qIdx].xpReward;
                    if (g_State.buffStrengthTimer > 0) gRew = gRew * 150 / 100;
                    if (g_State.buffLifeTimer > 0) xRew *= 2;

                    g_State.gold += gRew;
                    g_State.guildXP += xRew;
                    g_State.guildLevel = 1 + (g_State.guildXP / 200);

                    char logMsg[256];
                    wsprintfA(logMsg, "📜 QUEST COMPLETED! Delivered %s to %s! (+%d Gold, +%d Guild XP)",
                        g_Elements[targetId].name, g_Patrons[g_State.quests[qIdx].patronIdx],
                        gRew, xRew);
                    AddJournalLog(logMsg);
                    wsprintfA(g_State.lastStatus, "Fulfilled %s's order!", g_Patrons[g_State.quests[qIdx].patronIdx]);

                    Beep(523, 60); Beep(659, 60); Beep(784, 60); Beep(1046, 100);

                    GenerateQuest(qIdx);
                    UpdateEquipmentUI(hwnd);
                    InvalidateRect(hwnd, NULL, TRUE);
                } else {
                    AddJournalLog("⚠️ You must discover/craft the requested element before turning in!");
                    Beep(220, 100);
                }
            }
            // Quest Reroll Button (803)
            else if (id == 803) {
                const int COST = 15;
                if (g_State.gold < COST) {
                    lstrcpyA(g_State.lastStatus, "Need 15 Gold to reroll quests!");
                    AddJournalLog("⚠️ Not enough Gold to reroll quest board! (Cost: 15 Gold)");
                    Beep(220, 100);
                } else {
                    g_State.gold -= COST;
                    GenerateQuest(0);
                    GenerateQuest(1);
                    GenerateQuest(2);
                    AddJournalLog("🔄 Rerolled Master Alchemist Guild Quest Board (-15 Gold).");
                    lstrcpyA(g_State.lastStatus, "Guild Quests Rerolled");
                    Beep(500, 80);
                    UpdateEquipmentUI(hwnd);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Game Mode Commands (1200 = Classic, 1201 = Blitz, 1202 = Puzzle, 1203 = Start Blitz, 1204 = Skip Target)
            else if (id == 1200) {
                g_State.gameMode = 0;
                g_State.blitzActive = 0;
                AddJournalLog("Switched to Classic Discovery Mode.");
                lstrcpyA(g_State.lastStatus, "Classic Mode Active");
                UpdateEquipmentUI(hwnd);
                Beep(450, 40);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (id == 1201) {
                g_State.gameMode = 1;
                AddJournalLog("⚡ TIMED ALCHEMY BLITZ: Click 'Start Blitz' to begin the 60s challenge!");
                lstrcpyA(g_State.lastStatus, "Blitz Challenge Ready");
                UpdateEquipmentUI(hwnd);
                Beep(450, 40);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (id == 1202) {
                g_State.gameMode = 2;
                AddJournalLog("🧩 PUZZLE CRUCIBLE: Synthesize the target element goal!");
                lstrcpyA(g_State.lastStatus, "Puzzle Crucible Active");
                UpdateEquipmentUI(hwnd);
                Beep(450, 40);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (id == 1203) { // Start/Restart Blitz
                g_State.blitzTimeLeft = 60;
                g_State.blitzScore = 0;
                g_State.blitzActive = 1;
                AddJournalLog("⚡ BLITZ STARTED! Discover compounds before 60s expires!");
                lstrcpyA(g_State.lastStatus, "Blitz Challenge Active!");
                Beep(880, 80);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (id == 1204) { // Skip Puzzle Target
                int r = FastRand() % TOTAL_RECIPES;
                g_State.puzzleTargetId = g_Recipes[r].result;
                g_State.puzzleMoves = 0;
                char logMsg[128];
                wsprintfA(logMsg, "🧩 NEW PUZZLE TARGET: Synthesize %s (Tier %d)!",
                    g_Elements[g_State.puzzleTargetId].name, g_Elements[g_State.puzzleTargetId].tier);
                AddJournalLog(logMsg);
                Beep(554, 60);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Main Action (Transmute / Distill / Extract / Crush)
            else if (id == 201) {
                if (g_State.selectedEquipment == 0) { // Crucible Transmute
                    if (g_State.slot1 < 0 || g_State.slot2 < 0) {
                        lstrcpyA(g_State.lastStatus, "Select 2 elements for Crucible!");
                        AddJournalLog("Place two elements into the Crucible before transmuting.");
                        Beep(220, 100);
                    } else {
                        int e1 = g_State.slot1;
                        int e2 = g_State.slot2;
                        int matchIdx = -1;

                        for (int r = 0; r < TOTAL_RECIPES; r++) {
                            if ((g_Recipes[r].ingredient1 == e1 && g_Recipes[r].ingredient2 == e2) ||
                                (g_Recipes[r].ingredient1 == e2 && g_Recipes[r].ingredient2 == e1)) {
                                matchIdx = r;
                                break;
                            }
                        }

                        if (matchIdx >= 0) {
                            int res = g_Recipes[matchIdx].result;
                            int resTier = g_Elements[res].tier;
                            int reqThreshold = g_Tiers[resTier - 1].threshold;

                            if (g_State.discoveredCount < reqThreshold) {
                                wsprintfA(g_State.lastStatus, "🔒 Tier %d (%s) Locked!", resTier, g_Tiers[resTier - 1].name);
                                char logMsg[256];
                                wsprintfA(logMsg, "🔒 TIER LOCKED! Crafting %s requires Tier %d (%s) - discover %d elements!",
                                    g_Elements[res].name, resTier, g_Tiers[resTier - 1].name, reqThreshold);
                                AddJournalLog(logMsg);
                                Beep(180, 150);
                            } else {
                                int isNew = !g_State.discovered[res];

                                // Phase 12 Mode Checks
                                if (g_State.gameMode == 1 && g_State.blitzActive) {
                                    int pts = resTier * 50;
                                    if (isNew) pts += 100;
                                    g_State.blitzScore += pts;
                                    if (g_State.blitzScore > g_State.blitzHighScore) g_State.blitzHighScore = g_State.blitzScore;
                                }

                                if (g_State.gameMode == 2) {
                                    g_State.puzzleMoves++;
                                    if (res == g_State.puzzleTargetId) {
                                        g_State.puzzleSolvedCount++;
                                        if (g_State.puzzleSolvedCount > g_State.puzzleHighScore) g_State.puzzleHighScore = g_State.puzzleSolvedCount;
                                        char logMsg[256];
                                        wsprintfA(logMsg, "🧩 PUZZLE SOLVED! Successfully synthesized %s in %d moves! (Solved: %d)",
                                            g_Elements[res].name, g_State.puzzleMoves, g_State.puzzleSolvedCount);
                                        AddJournalLog(logMsg);
                                        PlayDiscoveryFanfare();

                                        int rNext = FastRand() % TOTAL_RECIPES;
                                        g_State.puzzleTargetId = g_Recipes[rNext].result;
                                        g_State.puzzleMoves = 0;
                                    }
                                }

                                if (isNew) {
                                    g_State.discovered[res] = 1;
                                    g_State.discoveredCount++;

                                    int capMult = 100 + (g_State.upgradeCrucibleCap * 15);
                                    int essGain = 25 * capMult / 100;
                                    int dustGain = 25 * capMult / 100;

                                    if (g_State.buffStrengthTimer > 0) essGain = essGain * 150 / 100;
                                    if (g_State.buffInvisibilityTimer > 0) dustGain += 25;

                                    g_State.essence += essGain;
                                    g_State.dust += dustGain;

                                    int critChance = g_State.upgradeCatalystSpeed * 15;
                                    int isCrit = (g_State.buffLifeTimer > 0) || ((FastRand() % 100) < critChance);
                                    char critLogStr[64] = "";
                                    if (isCrit) {
                                        g_State.essence += 20;
                                        g_State.gold += 15;
                                        lstrcpyA(critLogStr, " [CRITICAL TRANSMUTE! +20 Ess, +15 Gold]");
                                        if (g_State.gameMode == 1 && g_State.blitzActive) {
                                            g_State.blitzScore += 50;
                                            if (g_State.blitzScore > g_State.blitzHighScore) g_State.blitzHighScore = g_State.blitzScore;
                                        }
                                    }

                                    CheckTierUnlocks();
                                    UpdateGrimoireGrid();

                                    wsprintfA(g_State.lastStatus, "DISCOVERY! Created %s!", g_Elements[res].name);

                                    char logMsg[320];
                                    wsprintfA(logMsg, "✨ NEW DISCOVERY! You created %s [Tier %d %s] by combining %s + %s! (+%d Dust)%s",
                                        g_Elements[res].name, resTier, g_Tiers[resTier - 1].name, g_Elements[e1].name, g_Elements[e2].name, dustGain, critLogStr);
                                    AddJournalLog(logMsg);

                                    PlayDiscoveryFanfare();
                                } else {
                                    wsprintfA(g_State.lastStatus, "Created %s (Known)", g_Elements[res].name);
                                    char logMsg[256];
                                    wsprintfA(logMsg, "Created %s (%s + %s). Already recorded in Grimoire.",
                                        g_Elements[res].name, g_Elements[e1].name, g_Elements[e2].name);
                                    AddJournalLog(logMsg);
                                    Beep(659, 120);
                                }
                            }
                        } else {
                            if (g_State.gameMode == 2) {
                                g_State.puzzleMoves++;
                            }
                            wsprintfA(g_State.lastStatus, "Reaction Fizzled!");
                            char logMsg[256];
                            wsprintfA(logMsg, "Reaction fizzled! No transmutation for %s + %s.",
                                g_Elements[e1].name, g_Elements[e2].name);
                            AddJournalLog(logMsg);
                            Beep(180, 150);
                        }
                    }
                } else if (g_State.selectedEquipment == 1) { // Retort Distillation
                    if (g_State.slot1 < 0) {
                        lstrcpyA(g_State.lastStatus, "Place complex element in Slot 1!");
                        AddJournalLog("Place a complex element into Slot 1 to distill in the Retort.");
                        Beep(220, 100);
                    } else {
                        int e1 = g_State.slot1;
                        if (g_Elements[e1].isBasic || g_Elements[e1].tier == 1) {
                            wsprintfA(g_State.lastStatus, "Primordial %s cannot be distilled!", g_Elements[e1].name);
                            AddJournalLog("⚠️ Primordial base elements cannot be distilled!");
                            Beep(220, 100);
                        } else {
                            int matchIdx = -1;
                            for (int r = 0; r < TOTAL_RECIPES; r++) {
                                if (g_Recipes[r].result == e1) { matchIdx = r; break; }
                            }
                            if (matchIdx >= 0) {
                                int ing1 = g_Recipes[matchIdx].ingredient1;
                                if (!g_State.discovered[ing1]) {
                                    g_State.discovered[ing1] = 1;
                                    g_State.discoveredCount++;
                                    CheckTierUnlocks();
                                    UpdateGrimoireGrid();
                                }
                                int yieldMult = 100 + (g_State.upgradeEssenceYield * 25);
                                int essGain = 30 * yieldMult / 100;
                                int dustGain = 15 * yieldMult / 100;
                                g_State.essence += essGain;
                                g_State.dust += dustGain;
                                wsprintfA(g_State.lastStatus, "Retort: Distilled %s -> %s!", g_Elements[e1].name, g_Elements[ing1].name);
                                char logMsg[256];
                                wsprintfA(logMsg, "⚗️ RETORT DISTILLATION: Distilled %s to extract primary essence %s! (+%d Essence, +%d Dust)",
                                    g_Elements[e1].name, g_Elements[ing1].name, essGain, dustGain);
                                AddJournalLog(logMsg);
                                Beep(350, 50); Beep(440, 50); Beep(554, 50); Beep(659, 70);
                            }
                        }
                    }
                } else if (g_State.selectedEquipment == 2) { // Alembic Extraction
                    if (g_State.slot1 < 0) {
                        lstrcpyA(g_State.lastStatus, "Place complex element in Slot 1!");
                        AddJournalLog("Place a complex element into Slot 1 to extract in the Alembic.");
                        Beep(220, 100);
                    } else {
                        int e1 = g_State.slot1;
                        if (g_Elements[e1].isBasic || g_Elements[e1].tier == 1) {
                            wsprintfA(g_State.lastStatus, "Primordial %s cannot be extracted!", g_Elements[e1].name);
                            AddJournalLog("⚠️ Primordial base elements cannot be extracted!");
                            Beep(220, 100);
                        } else {
                            int matchIdx = -1;
                            for (int r = 0; r < TOTAL_RECIPES; r++) {
                                if (g_Recipes[r].result == e1) { matchIdx = r; break; }
                            }
                            if (matchIdx >= 0) {
                                int ing2 = g_Recipes[matchIdx].ingredient2;
                                if (!g_State.discovered[ing2]) {
                                    g_State.discovered[ing2] = 1;
                                    g_State.discoveredCount++;
                                    CheckTierUnlocks();
                                    UpdateGrimoireGrid();
                                }
                                int yieldMult = 100 + (g_State.upgradeEssenceYield * 25);
                                int essGain = 30 * yieldMult / 100;
                                int dustGain = 15 * yieldMult / 100;
                                g_State.essence += essGain;
                                g_State.dust += dustGain;
                                wsprintfA(g_State.lastStatus, "Alembic: Extracted %s -> %s!", g_Elements[e1].name, g_Elements[ing2].name);
                                char logMsg[256];
                                wsprintfA(logMsg, "🧪 ALEMBIC EXTRACTION: Extracted secondary essence %s from %s! (+%d Essence, +%d Dust)",
                                    g_Elements[ing2].name, g_Elements[e1].name, essGain, dustGain);
                                AddJournalLog(logMsg);
                                Beep(523, 50); Beep(659, 50); Beep(784, 50); Beep(1046, 70);
                            }
                        }
                    }
                } else if (g_State.selectedEquipment == 3) { // Anvil Crushing
                    if (g_State.slot1 < 0) {
                        lstrcpyA(g_State.lastStatus, "Place complex element in Slot 1!");
                        AddJournalLog("Place a complex element into Slot 1 to crush on the Anvil.");
                        Beep(220, 100);
                    } else {
                        int e1 = g_State.slot1;
                        if (g_Elements[e1].isBasic || g_Elements[e1].tier == 1) {
                            wsprintfA(g_State.lastStatus, "Primordial %s cannot be crushed!", g_Elements[e1].name);
                            AddJournalLog("⚠️ Primordial base elements cannot be crushed!");
                            Beep(220, 100);
                        } else {
                            int tier = g_Elements[e1].tier;
                            int yieldMult = 100 + (g_State.upgradeEssenceYield * 25);
                            int essGain = (tier * 30) * yieldMult / 100;
                            int dustGain = (tier * 25) * yieldMult / 100;
                            g_State.essence += essGain;
                            g_State.dust += dustGain;
                            wsprintfA(g_State.lastStatus, "Anvil: Smashed %s (+%d Ess, +%d Dust)!", g_Elements[e1].name, essGain, dustGain);
                            char logMsg[256];
                            wsprintfA(logMsg, "🔨 ANVIL ESSENCE HARVEST: Smashed %s [Tier %d] into pure base essence! (+%d Essence, +%d Dust)",
                                g_Elements[e1].name, tier, essGain, dustGain);
                            AddJournalLog(logMsg);
                        }
                    }
                } else if (g_State.selectedEquipment == 6) { // Potion Brewing
                    if (g_State.slot1 < 0 || g_State.slot2 < 0) {
                        lstrcpyA(g_State.lastStatus, "Select herb and essence for brewing!");
                        AddJournalLog("Select an herb/ingredient and essence into slots to brew in Cauldron.");
                        Beep(220, 100);
                    } else {
                        int e1 = g_State.slot1;
                        int e2 = g_State.slot2;

                        int isHerb1 = (e1 == 11 || e1 == 15 || e1 == 14 || e1 == 7 || e1 == 36);
                        int isHerb2 = (e2 == 11 || e2 == 15 || e2 == 14 || e2 == 7 || e2 == 36);

                        int brewedType = -1; // 0=Strength, 1=Invis, 2=Mana, 3=Life
                        if (isHerb1 || isHerb2) {
                            int other = isHerb1 ? e2 : e1;
                            if (other == 6 || other == 0 || other == 16) brewedType = 0;
                            else if (other == 39 || other == 3 || other == 41) brewedType = 1;
                            else if (other == 31 || other == 30 || other == 1) brewedType = 2;
                            else if (other == 28 || other == 42 || other == 40 || other == 37) brewedType = 3;
                        }

                        if (brewedType == 0) {
                            g_State.potionStrength++;
                            AddJournalLog("🥣 BREWING: Combined ingredients to brew Strength Elixir (💪)! (+1 Potion)");
                            wsprintfA(g_State.lastStatus, "Brewed Strength Elixir (💪)!");
                            Beep(440, 50); Beep(554, 50); Beep(659, 70);
                        } else if (brewedType == 1) {
                            g_State.potionInvisibility++;
                            AddJournalLog("🥣 BREWING: Combined ingredients to brew Invisibility Elixir (👻)! (+1 Potion)");
                            wsprintfA(g_State.lastStatus, "Brewed Invisibility Elixir (👻)!");
                            Beep(523, 50); Beep(659, 50); Beep(784, 70);
                        } else if (brewedType == 2) {
                            g_State.potionMana++;
                            AddJournalLog("🥣 BREWING: Combined ingredients to brew Mana Elixir (🔮)! (+1 Potion)");
                            wsprintfA(g_State.lastStatus, "Brewed Mana Elixir (🔮)!");
                            Beep(659, 50); Beep(784, 50); Beep(880, 70);
                        } else if (brewedType == 3) {
                            g_State.potionLife++;
                            AddJournalLog("🥣 BREWING: Combined ingredients to brew Elixir of Life (❤️)! (+1 Potion)");
                            wsprintfA(g_State.lastStatus, "Brewed Elixir of Life (❤️)!");
                            Beep(523, 50); Beep(659, 50); Beep(880, 50); Beep(1046, 90);
                        } else {
                            g_State.essence += 15;
                            g_State.dust += 15;
                            AddJournalLog("🧪 BREWING: Brewed Minor Tonic! (+15 Essence, +15 Dust)");
                            wsprintfA(g_State.lastStatus, "Brewed Minor Tonic (+15 Ess, +15 Dust)");
                            Beep(350, 70);
                        }
                        UpdateEquipmentUI(hwnd);
                    }
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Clear Crucible
            else if (id == 202) {
                g_State.slot1 = -1;
                g_State.slot2 = -1;
                UpdateSlotButtonText();
                lstrcpyA(g_State.lastStatus, "Crucible Cleared");
                Beep(300, 60);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Reset Progress
            else if (id == 203) {
                if (MessageBoxA(hwnd, "Reset all discovered elements back to starter 4?", "Reset KAlchemy", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    InitGameState();
                    SetWindowTextA(g_hSearchEdit, "");
                    UpdateSlotButtonText();
                    UpdateGrimoireGrid();
                    SetWindowTextA(g_hJournalEdit, "");
                    AddJournalLog("Journal reset. Basic elements restored.");
                    Beep(400, 100);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            // Vague Research Hint (204)
            else if (id == 204) {
                int COST = 20;
                if (g_State.buffInvisibilityTimer > 0) COST = 0;
                else if (g_State.buffManaTimer > 0) COST = 10;
                if (g_State.dust < COST) {
                    lstrcpyA(g_State.lastStatus, "Need Dust for Research Hint!");
                    char msg[256];
                    wsprintfA(msg, "⚠️ Not enough Alchemical Dust! Need %d Dust (You have %d).", COST, g_State.dust);
                    AddJournalLog(msg);
                    Beep(220, 100);
                } else {
                    int matchIdx = -1;
                    int foundBothKnown = -1;
                    int foundOneKnown = -1;

                    for (int r = 0; r < TOTAL_RECIPES; r++) {
                        int res = g_Recipes[r].result;
                        if (!g_State.discovered[res]) {
                            int e1 = g_Recipes[r].ingredient1;
                            int e2 = g_Recipes[r].ingredient2;
                            if (g_State.discovered[e1] && g_State.discovered[e2]) {
                                if (foundBothKnown == -1) foundBothKnown = r;
                            } else if (g_State.discovered[e1] || g_State.discovered[e2]) {
                                if (foundOneKnown == -1) foundOneKnown = r;
                            }
                            if (matchIdx == -1) matchIdx = r;
                        }
                    }

                    if (foundOneKnown >= 0) matchIdx = foundOneKnown;
                    else if (foundBothKnown >= 0) matchIdx = foundBothKnown;

                    if (matchIdx < 0) {
                        AddJournalLog("🔮 The Oracle Whispers: All elements in the cosmos have already been discovered!");
                        Beep(880, 150);
                    } else {
                        g_State.dust -= COST;
                        int res = g_Recipes[matchIdx].result;
                        int e1 = g_Recipes[matchIdx].ingredient1;
                        int e2 = g_Recipes[matchIdx].ingredient2;
                        int knownIng = g_State.discovered[e1] ? e1 : (g_State.discovered[e2] ? e2 : e1);

                        wsprintfA(g_State.lastStatus, "💡 Hint: %s + ??? -> %s", g_Elements[knownIng].name, g_Elements[res].name);
                        char logMsg[256];
                        wsprintfA(logMsg, "💡 RESEARCH HINT (-%d Dust): Element '%s' (Tier %d) is crafted using '%s' and another element!",
                            COST, g_Elements[res].name, g_Elements[res].tier, g_Elements[knownIng].name);
                        AddJournalLog(logMsg);

                        Beep(349, 80);
                        Beep(440, 80);
                        Beep(523, 100);
                    }
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Oracle Vision (205)
            else if (id == 205) {
                int COST = 50;
                if (g_State.buffInvisibilityTimer > 0) COST = 0;
                else if (g_State.buffManaTimer > 0) COST = 25;
                if (g_State.dust < COST) {
                    lstrcpyA(g_State.lastStatus, "Need Dust for Oracle Vision!");
                    char msg[256];
                    wsprintfA(msg, "⚠️ Not enough Alchemical Dust! Need %d Dust (You have %d).", COST, g_State.dust);
                    AddJournalLog(msg);
                    Beep(220, 100);
                } else {
                    int foundBothKnown = -1;
                    int matchIdx = -1;

                    for (int r = 0; r < TOTAL_RECIPES; r++) {
                        int res = g_Recipes[r].result;
                        if (!g_State.discovered[res]) {
                            int e1 = g_Recipes[r].ingredient1;
                            int e2 = g_Recipes[r].ingredient2;
                            if (g_State.discovered[e1] && g_State.discovered[e2]) {
                                foundBothKnown = r;
                                break;
                            }
                            if (matchIdx == -1) matchIdx = r;
                        }
                    }

                    if (foundBothKnown >= 0) matchIdx = foundBothKnown;

                    if (matchIdx < 0) {
                        AddJournalLog("🔮 The Oracle Whispers: All elements in the cosmos have already been discovered!");
                        Beep(880, 150);
                    } else {
                        g_State.dust -= COST;
                        int res = g_Recipes[matchIdx].result;
                        int e1 = g_Recipes[matchIdx].ingredient1;
                        int e2 = g_Recipes[matchIdx].ingredient2;

                        if (g_State.discovered[e1] && g_State.discovered[e2]) {
                            g_State.slot1 = e1;
                            g_State.slot2 = e2;
                            UpdateSlotButtonText();
                            wsprintfA(g_State.lastStatus, "🔮 Oracle: %s + %s -> %s (Placed!)", g_Elements[e1].name, g_Elements[e2].name, g_Elements[res].name);
                            char logMsg[256];
                            wsprintfA(logMsg, "👁️ ORACLE VISION (-%d Dust): Transmute %s + %s = %s! (Ingredients placed in Crucible!)",
                                COST, g_Elements[e1].name, g_Elements[e2].name, g_Elements[res].name);
                            AddJournalLog(logMsg);
                        } else {
                            wsprintfA(g_State.lastStatus, "🔮 Oracle: %s + %s -> %s", g_Elements[e1].name, g_Elements[e2].name, g_Elements[res].name);
                            char logMsg[256];
                            wsprintfA(logMsg, "👁️ ORACLE VISION (-%d Dust): Recipe revealed: %s + %s = %s!",
                                COST, g_Elements[e1].name, g_Elements[e2].name, g_Elements[res].name);
                            AddJournalLog(logMsg);
                        }

                        Beep(440, 70);
                        Beep(554, 70);
                        Beep(659, 70);
                        Beep(880, 70);
                        Beep(1108, 120);
                    }
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            SetBkMode(hdc, TRANSPARENT);

            // Helper macro for drawing glowing panel borders
            #define DRAW_RUNE_PANEL(r, brush) { \
                FillRect(hdc, &(r), (brush)); \
                HGDIOBJ pOldP = SelectObject(hdc, hPurplePen); \
                Rectangle(hdc, (r).left, (r).top, (r).right, (r).bottom); \
                SelectObject(hdc, hGoldPen); \
                Rectangle(hdc, (r).left + 2, (r).top + 2, (r).right - 2, (r).bottom - 2); \
                SelectObject(hdc, pOldP); \
            }

            // Top Header Bar
            RECT headerRect = { 20, 10, 765, 60 };
            DRAW_RUNE_PANEL(headerRect, hPanelBrush);

            SelectObject(hdc, hTitleFont);
            SetTextColor(hdc, RGB(243, 156, 18));
            TextOutA(hdc, 35, 18, "KALCHEMY - Arcane Laboratory", 28);

            // Rank calculation
            const char* rankStr = "Apprentice";
            if (g_State.discoveredCount >= 50) rankStr = "Celestial Master";
            else if (g_State.discoveredCount >= 38) rankStr = "Grand Alchemist";
            else if (g_State.discoveredCount >= 24) rankStr = "Master Alchemist";
            else if (g_State.discoveredCount >= 12) rankStr = "Journeyman";
            else if (g_State.discoveredCount >= 4) rankStr = "Adept";

            int highestTier = CheckTierUnlocks();

            // Golden Element Badges in Header
            SelectObject(hdc, hBadgeFont);
            HGDIOBJ oldBrush = SelectObject(hdc, hGoldBadgeBrush);
            HGDIOBJ oldPen = SelectObject(hdc, hGoldPen);

            // Badge 1: Discovered
            RECT badge1 = { 205, 18, 285, 52 };
            RoundRect(hdc, badge1.left, badge1.top, badge1.right, badge1.bottom, 6, 6);
            char b1Str[32];
            wsprintfA(b1Str, "Disc: %d/%d", g_State.discoveredCount, TOTAL_ELEMENTS);
            SetTextColor(hdc, RGB(255, 215, 0));
            DrawTextA(hdc, b1Str, -1, &badge1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

            // Badge 2: Highest Tier
            RECT badge2 = { 290, 18, 375, 52 };
            RoundRect(hdc, badge2.left, badge2.top, badge2.right, badge2.bottom, 6, 6);
            char b2Str[32];
            wsprintfA(b2Str, "T%d %s", highestTier, g_Tiers[highestTier - 1].name);
            SetTextColor(hdc, RGB(255, 215, 0));
            DrawTextA(hdc, b2Str, -1, &badge2, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

            // Badge 3: Essence
            RECT badge3 = { 380, 18, 455, 52 };
            RoundRect(hdc, badge3.left, badge3.top, badge3.right, badge3.bottom, 6, 6);
            char b3Str[32];
            wsprintfA(b3Str, "Ess: %d", g_State.essence);
            SetTextColor(hdc, RGB(255, 215, 0));
            DrawTextA(hdc, b3Str, -1, &badge3, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

            // Badge 4: Alchemical Dust
            RECT badge4 = { 460, 18, 535, 52 };
            RoundRect(hdc, badge4.left, badge4.top, badge4.right, badge4.bottom, 6, 6);
            char b4Str[32];
            wsprintfA(b4Str, "Dust: %d", g_State.dust);
            SetTextColor(hdc, RGB(255, 215, 0));
            DrawTextA(hdc, b4Str, -1, &badge4, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

            // Badge 5: Gold
            RECT badge5 = { 540, 18, 615, 52 };
            RoundRect(hdc, badge5.left, badge5.top, badge5.right, badge5.bottom, 6, 6);
            char b5Str[32];
            wsprintfA(b5Str, "Gold: %d", g_State.gold);
            SetTextColor(hdc, RGB(255, 215, 0));
            DrawTextA(hdc, b5Str, -1, &badge5, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

            // Badge 6: Guild Level
            RECT badge6 = { 620, 18, 695, 52 };
            RoundRect(hdc, badge6.left, badge6.top, badge6.right, badge6.bottom, 6, 6);
            char b6Str[32];
            wsprintfA(b6Str, "Lvl %d", g_State.guildLevel);
            SetTextColor(hdc, RGB(255, 215, 0));
            DrawTextA(hdc, b6Str, -1, &badge6, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

            // Badge 7: Rank
            RECT badge7 = { 700, 18, 755, 52 };
            RoundRect(hdc, badge7.left, badge7.top, badge7.right, badge7.bottom, 6, 6);
            SetTextColor(hdc, RGB(255, 215, 0));
            DrawTextA(hdc, rankStr, -1, &badge7, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

            SelectObject(hdc, oldBrush);
            SelectObject(hdc, oldPen);

            // Left Panel - Element Grimoire
            RECT leftPanel = { 20, 68, 265, 523 };
            DRAW_RUNE_PANEL(leftPanel, hPanelBrush);

            SelectObject(hdc, hHeaderFont);
            SetTextColor(hdc, RGB(243, 156, 18));
            TextOutA(hdc, 30, 74, "Elemental Grimoire", 18);

            char countStr[32];
            wsprintfA(countStr, "(%d Elements)", g_State.discoveredCount);
            SelectObject(hdc, hUIFont);
            SetTextColor(hdc, RGB(180, 160, 220));
            TextOutA(hdc, 175, 76, countStr, lstrlenA(countStr));

            // Center Panel - Transmutation Crucible / Laboratory Equipment
            RECT centerPanel = { 275, 68, 510, 523 };
            DRAW_RUNE_PANEL(centerPanel, hCrucibleBrush);

            const char* modeTitle = "Transmutation Crucible";
            const char* vesselLabel = "Crucible";
            if (g_State.selectedEquipment == 1) { modeTitle = "Retort Distillation"; vesselLabel = "Retort"; }
            else if (g_State.selectedEquipment == 2) { modeTitle = "Alembic Extraction"; vesselLabel = "Alembic"; }
            else if (g_State.selectedEquipment == 3) { modeTitle = "Anvil Essence Smelter"; vesselLabel = "Anvil"; }
            else if (g_State.selectedEquipment == 4) { modeTitle = "Guild Quest Board"; vesselLabel = "Quests"; }
            else if (g_State.selectedEquipment == 5) { modeTitle = "Enchanter Workshop"; vesselLabel = "Workshop"; }

            SelectObject(hdc, hHeaderFont);
            SetTextColor(hdc, RGB(243, 156, 18));
            TextOutA(hdc, 290, 74, modeTitle, lstrlenA(modeTitle));

            if (g_State.selectedEquipment == 4) {
                SelectObject(hdc, hUIFont);
                SetTextColor(hdc, RGB(180, 160, 220));
                char subTitle[64];
                wsprintfA(subTitle, "Guild Level %d (%d XP)", g_State.guildLevel, g_State.guildXP);
                TextOutA(hdc, 290, 122, subTitle, lstrlenA(subTitle));

                for (int q = 0; q < 3; q++) {
                    RECT qCard = { 285, 142 + q * 80, 420, 212 + q * 80 };
                    FillRect(hdc, &qCard, hPanelBrush);
                    HGDIOBJ oldP = SelectObject(hdc, hGoldPen);
                    Rectangle(hdc, qCard.left, qCard.top, qCard.right, qCard.bottom);

                    int pIdx = g_State.quests[q].patronIdx;
                    int tId = g_State.quests[q].targetId;

                    SelectObject(hdc, hUIFont);
                    SetTextColor(hdc, RGB(241, 196, 15));
                    char patronStr[64];
                    wsprintfA(patronStr, "%s", g_Patrons[pIdx]);
                    TextOutA(hdc, qCard.left + 6, qCard.top + 6, patronStr, lstrlenA(patronStr));

                    SetTextColor(hdc, RGB(226, 232, 240));
                    char reqStr[64];
                    wsprintfA(reqStr, "Order: %s (T%d)", g_Elements[tId].name, g_Elements[tId].tier);
                    TextOutA(hdc, qCard.left + 6, qCard.top + 26, reqStr, lstrlenA(reqStr));

                    SetTextColor(hdc, RGB(46, 204, 113));
                    char rewStr[64];
                    wsprintfA(rewStr, "+%d Gold, +%d XP", g_State.quests[q].goldReward, g_State.quests[q].xpReward);
                    TextOutA(hdc, qCard.left + 6, qCard.top + 46, rewStr, lstrlenA(rewStr));
                    SelectObject(hdc, oldP);
                }
            } else if (g_State.selectedEquipment == 5) {
                SelectObject(hdc, hUIFont);
                SetTextColor(hdc, RGB(180, 160, 220));
                TextOutA(hdc, 290, 120, "Laboratory Upgrades & Shop", 26);

                const char* upgNames[4] = { "Crucible Cap", "Essence Yield", "Auto-Sorter", "Catalyst Speed" };
                const char* upgDescs[4] = {
                    "+15% transmute yield/lvl",
                    "+25% extraction yield",
                    "Unlocks Auto-Fill button",
                    "+15% critical chance"
                };
                int lvls[4] = { g_State.upgradeCrucibleCap, g_State.upgradeEssenceYield, g_State.upgradeAutoSorter, g_State.upgradeCatalystSpeed };
                int costs[4] = {
                    (lvls[0] < 5 ? g_CrucibleCapCosts[lvls[0]] : 0),
                    (lvls[1] < 5 ? g_EssenceYieldCosts[lvls[1]] : 0),
                    (lvls[2] < 5 ? g_AutoSorterCosts[lvls[2]] : 0),
                    (lvls[3] < 5 ? g_CatalystSpeedCosts[lvls[3]] : 0)
                };

                for (int u = 0; u < 4; u++) {
                    RECT uCard = { 285, 138 + u * 60, 425, 192 + u * 60 };
                    FillRect(hdc, &uCard, hPanelBrush);
                    HGDIOBJ oldP = SelectObject(hdc, hGoldPen);
                    Rectangle(hdc, uCard.left, uCard.top, uCard.right, uCard.bottom);

                    SelectObject(hdc, hBadgeFont);
                    SetTextColor(hdc, RGB(241, 196, 15));
                    char nameStr[64];
                    wsprintfA(nameStr, "%s (Lvl %d/5)", upgNames[u], lvls[u]);
                    TextOutA(hdc, uCard.left + 6, uCard.top + 4, nameStr, lstrlenA(nameStr));

                    SelectObject(hdc, hUIFont);
                    SetTextColor(hdc, RGB(200, 210, 225));
                    TextOutA(hdc, uCard.left + 6, uCard.top + 18, upgDescs[u], lstrlenA(upgDescs[u]));

                    if (lvls[u] >= 5) {
                        SetTextColor(hdc, RGB(46, 204, 113));
                        TextOutA(hdc, uCard.left + 6, uCard.top + 34, "MAX LEVEL", 9);
                    } else {
                        SetTextColor(hdc, RGB(241, 196, 15));
                        char costStr[32];
                        wsprintfA(costStr, "Cost: %d Gold", costs[u]);
                        TextOutA(hdc, uCard.left + 6, uCard.top + 34, costStr, lstrlenA(costStr));
                    }
                    SelectObject(hdc, oldP);
                }
            } else if (g_State.selectedEquipment == 6) {
                SelectObject(hdc, hUIFont);
                SetTextColor(hdc, RGB(180, 160, 220));
                TextOutA(hdc, 290, 120, "Cauldron Brewing & Effect Tester", 32);

                // Draw Potion Inventory Cards (4 boxes)
                const char* pNames[4] = { "Strength", "Invis", "Mana", "Life" };
                int pCounts[4] = { g_State.potionStrength, g_State.potionInvisibility, g_State.potionMana, g_State.potionLife };

                for (int p = 0; p < 4; p++) {
                    int col = p % 2;
                    int row = p / 2;
                    RECT pCard = { 282 + col * 112, 138 + row * 30, 390 + col * 112, 166 + row * 30 };
                    FillRect(hdc, &pCard, hPanelBrush);
                    HGDIOBJ oldP = SelectObject(hdc, hGoldPen);
                    Rectangle(hdc, pCard.left, pCard.top, pCard.right, pCard.bottom);

                    SelectObject(hdc, hBadgeFont);
                    SetTextColor(hdc, RGB(241, 196, 15));
                    char pStr[32];
                    wsprintfA(pStr, "%s: x%d", pNames[p], pCounts[p]);
                    DrawTextA(hdc, pStr, -1, &pCard, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
                    SelectObject(hdc, oldP);
                }

                // Effect Tester Status Monitor Box
                RECT monRect = { 282, 330, 502, 430 };
                FillRect(hdc, &monRect, hPanelBrush);
                HGDIOBJ oldP = SelectObject(hdc, hPurplePen);
                Rectangle(hdc, monRect.left, monRect.top, monRect.right, monRect.bottom);

                SelectObject(hdc, hBadgeFont);
                SetTextColor(hdc, RGB(243, 156, 18));
                TextOutA(hdc, monRect.left + 8, monRect.top + 6, "EFFECT TESTER MONITOR", 21);

                SelectObject(hdc, hUIFont);
                int yOffset = monRect.top + 26;
                int hasBuffs = 0;

                if (g_State.buffStrengthTimer > 0) {
                    SetTextColor(hdc, RGB(241, 196, 15));
                    char bStr[64]; wsprintfA(bStr, "* Strength (%ds): +50%% Yield", g_State.buffStrengthTimer);
                    TextOutA(hdc, monRect.left + 8, yOffset, bStr, lstrlenA(bStr));
                    yOffset += 18; hasBuffs = 1;
                }
                if (g_State.buffInvisibilityTimer > 0) {
                    SetTextColor(hdc, RGB(155, 89, 182));
                    char bStr[64]; wsprintfA(bStr, "* Invis (%ds): Free Hints", g_State.buffInvisibilityTimer);
                    TextOutA(hdc, monRect.left + 8, yOffset, bStr, lstrlenA(bStr));
                    yOffset += 18; hasBuffs = 1;
                }
                if (g_State.buffManaTimer > 0) {
                    SetTextColor(hdc, RGB(54, 152, 219));
                    char bStr[64]; wsprintfA(bStr, "* Mana (%ds): 50%% Discount", g_State.buffManaTimer);
                    TextOutA(hdc, monRect.left + 8, yOffset, bStr, lstrlenA(bStr));
                    yOffset += 18; hasBuffs = 1;
                }
                if (g_State.buffLifeTimer > 0) {
                    SetTextColor(hdc, RGB(46, 204, 113));
                    char bStr[64]; wsprintfA(bStr, "* Life (%ds): 2x XP & 100%% Crit", g_State.buffLifeTimer);
                    TextOutA(hdc, monRect.left + 8, yOffset, bStr, lstrlenA(bStr));
                    yOffset += 18; hasBuffs = 1;
                }

                if (!hasBuffs) {
                    SetTextColor(hdc, RGB(160, 170, 190));
                    TextOutA(hdc, monRect.left + 8, yOffset, "No active buffs. Drink an elixir!", 33);
                }

                SelectObject(hdc, oldP);
            } else if (g_State.selectedEquipment == 7) {
                SelectObject(hdc, hUIFont);
                SetTextColor(hdc, RGB(241, 196, 15));
                TextOutA(hdc, 282, 145, "Master Alchemist Codex & Recipe Book", 36);

                // Draw Tier Discovery & Missing Breakdown Summary
                int yT = 165;
                SelectObject(hdc, hBadgeFont);
                for (int t = 1; t <= TOTAL_TIERS; t++) {
                    int discT = 0, totalT = 0;
                    for (int e = 0; e < TOTAL_ELEMENTS; e++) {
                        if (g_Elements[e].tier == t) {
                            totalT++;
                            if (g_State.discovered[e]) discT++;
                        }
                    }
                    int missingT = totalT - discT;
                    SetTextColor(hdc, g_Tiers[t-1].color);
                    char tSummary[64];
                    wsprintfA(tSummary, "T%d %s: %d/%d Disc (%d Missing)", t, g_Tiers[t-1].name, discT, totalT, missingT);
                    TextOutA(hdc, 282, yT, tSummary, lstrlenA(tSummary));
                    yT += 16;
                }

                // Draw Codex Inspector Detail Card
                int selIdx = g_State.selectedCodexElem;
                if (selIdx >= 0 && selIdx < TOTAL_ELEMENTS) {
                    RECT cCard = { 282, 255, 502, 435 };
                    FillRect(hdc, &cCard, hPanelBrush);
                    HGDIOBJ oldP = SelectObject(hdc, hGoldPen);
                    Rectangle(hdc, cCard.left, cCard.top, cCard.right, cCard.bottom);

                    int isDisc = g_State.discovered[selIdx];

                    // Title & Tier
                    SelectObject(hdc, hHeaderFont);
                    SetTextColor(hdc, isDisc ? RGB(241, 196, 15) : RGB(140, 150, 165));
                    char eTitle[64];
                    wsprintfA(eTitle, "%s (T%d %s)", isDisc ? g_Elements[selIdx].name : "Undiscovered Element", g_Elements[selIdx].tier, g_Elements[selIdx].isBasic ? "Basic" : "Compound");
                    TextOutA(hdc, cCard.left + 8, cCard.top + 8, eTitle, lstrlenA(eTitle));

                    // Lore Text Quote
                    SelectObject(hdc, hUIFont);
                    SetTextColor(hdc, RGB(220, 225, 235));
                    RECT loreRect = { cCard.left + 8, cCard.top + 28, cCard.right - 8, cCard.top + 78 };
                    DrawTextA(hdc, isDisc ? g_Elements[selIdx].lore : "Lore locked. Transmute elements to discover formulas.", -1, &loreRect, DT_WORDBREAK);

                    // Synthesis Recipe
                    SetTextColor(hdc, RGB(243, 156, 18));
                    TextOutA(hdc, cCard.left + 8, cCard.top + 82, "SYNTHESIS RECIPE:", 17);

                    SetTextColor(hdc, RGB(200, 210, 225));
                    if (g_Elements[selIdx].isBasic) {
                        TextOutA(hdc, cCard.left + 8, cCard.top + 98, "Primordial Element (Found naturally)", 36);
                    } else {
                        // Find recipe
                        int foundR = 0;
                        for (int r = 0; r < TOTAL_RECIPES; r++) {
                            if (g_Recipes[r].result == selIdx) {
                                int i1 = g_Recipes[r].ingredient1;
                                int i2 = g_Recipes[r].ingredient2;
                                char rStr[128];
                                wsprintfA(rStr, "%s + %s -> %s",
                                    g_State.discovered[i1] ? g_Elements[i1].name : "???",
                                    g_State.discovered[i2] ? g_Elements[i2].name : "???",
                                    g_Elements[selIdx].name);
                                TextOutA(hdc, cCard.left + 8, cCard.top + 98, rStr, lstrlenA(rStr));
                                foundR = 1;
                                break;
                            }
                        }
                        if (!foundR) TextOutA(hdc, cCard.left + 8, cCard.top + 98, "Unknown Formula", 15);
                    }

                    // Transmutation Uses
                    SetTextColor(hdc, RGB(243, 156, 18));
                    TextOutA(hdc, cCard.left + 8, cCard.top + 118, "TRANSMUTATION USES:", 19);

                    int yUse = cCard.top + 134;
                    int useCount = 0;
                    for (int r = 0; r < TOTAL_RECIPES; r++) {
                        if (g_Recipes[r].ingredient1 == selIdx || g_Recipes[r].ingredient2 == selIdx) {
                            int res = g_Recipes[r].result;
                            int otherIng = (g_Recipes[r].ingredient1 == selIdx) ? g_Recipes[r].ingredient2 : g_Recipes[r].ingredient1;
                            char uStr[128];
                            wsprintfA(uStr, "+ %s -> %s [%s]",
                                g_Elements[otherIng].name,
                                g_Elements[res].name,
                                g_State.discovered[res] ? "Discovered" : "Locked");
                            SetTextColor(hdc, g_State.discovered[res] ? RGB(46, 204, 113) : RGB(140, 150, 165));
                            TextOutA(hdc, cCard.left + 8, yUse, uStr, lstrlenA(uStr));
                            yUse += 16;
                            useCount++;
                            if (useCount >= 2) break; // fit in box
                        }
                    }
                    if (useCount == 0) {
                        SetTextColor(hdc, RGB(160, 170, 185));
                        TextOutA(hdc, cCard.left + 8, yUse, "Apex Element (No further combinations)", 38);
                    }

                    // Combination Stats
                    SetTextColor(hdc, RGB(180, 160, 220));
                    char sStr[128];
                    wsprintfA(sStr, "Crafted: x%d | Used in Crucible: x%d", g_State.craftCounts[selIdx], g_State.useCounts[selIdx]);
                    TextOutA(hdc, cCard.left + 8, cCard.top + 164, sStr, lstrlenA(sStr));

                    SelectObject(hdc, oldP);
                }
            } else {
                // Phase 12 Mode HUD Drawing
                if (g_State.gameMode == 1) {
                    RECT blitzBox = { 280, 120, 505, 142 };
                    FillRect(hdc, &blitzBox, hPanelBrush);
                    HGDIOBJ oldP = SelectObject(hdc, hGoldPen);
                    Rectangle(hdc, blitzBox.left, blitzBox.top, blitzBox.right, blitzBox.bottom);
                    SelectObject(hdc, hBadgeFont);
                    SetTextColor(hdc, RGB(241, 196, 15));
                    char bHudStr[128];
                    wsprintfA(bHudStr, "Time: %ds | Score: %d | Best: %d", g_State.blitzTimeLeft, g_State.blitzScore, g_State.blitzHighScore);
                    DrawTextA(hdc, bHudStr, -1, &blitzBox, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
                    SelectObject(hdc, oldP);
                } else if (g_State.gameMode == 2) {
                    RECT puzzleBox = { 280, 120, 505, 142 };
                    FillRect(hdc, &puzzleBox, hPanelBrush);
                    HGDIOBJ oldP = SelectObject(hdc, hGoldPen);
                    Rectangle(hdc, puzzleBox.left, puzzleBox.top, puzzleBox.right, puzzleBox.bottom);
                    SelectObject(hdc, hBadgeFont);
                    SetTextColor(hdc, RGB(241, 196, 15));
                    char pHudStr[128];
                    int tId = g_State.puzzleTargetId;
                    wsprintfA(pHudStr, "Target: %s (T%d) | Moves: %d | Solved: %d (Best: %d)",
                        g_Elements[tId].name, g_Elements[tId].tier, g_State.puzzleMoves, g_State.puzzleSolvedCount, g_State.puzzleHighScore);
                    DrawTextA(hdc, pHudStr, -1, &puzzleBox, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
                    SelectObject(hdc, oldP);
                }

                // Draw Outer Glowing Arcane Rune Ring & Crucible Vessel
                SelectObject(hdc, hGoldPen);
                HGDIOBJ pNullB = GetStockObject(NULL_BRUSH);
                SelectObject(hdc, pNullB);
                Ellipse(hdc, 340, 145, 445, 193); // Outer golden rune ring

                SelectObject(hdc, hVesselBrush);
                SelectObject(hdc, hPurplePen);
                Ellipse(hdc, 350, 148, 435, 189); // Core vessel ellipse

                SelectObject(hdc, hSlotFont);
                SetTextColor(hdc, RGB(241, 196, 15));
                RECT vLabelRect = { 350, 150, 435, 185 };
                DrawTextA(hdc, vesselLabel, -1, &vLabelRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

                // Status message centered below crucible buttons
                SelectObject(hdc, hUIFont);
                SetTextColor(hdc, RGB(120, 230, 180));
                RECT statusRect = { 285, 390, 500, 440 };
                DrawTextA(hdc, g_State.lastStatus, -1, &statusRect, DT_CENTER | DT_WORDBREAK);
            }

            // Right Panel - Journal & Log
            RECT rightPanel = { 520, 68, 765, 523 };
            DRAW_RUNE_PANEL(rightPanel, hPanelBrush);

            SelectObject(hdc, hHeaderFont);
            SetTextColor(hdc, RGB(243, 156, 18));
            TextOutA(hdc, 535, 74, "Alchemist's Journal", 19);

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT: {
            HDC hdcEdit = (HDC)wParam;
            HWND hEdit = (HWND)lParam;
            if (hEdit == g_hJournalEdit || hEdit == g_hSearchEdit || hEdit == g_hPageText) {
                SetTextColor(hdcEdit, RGB(226, 232, 240));
                SetBkColor(hdcEdit, RGB(16, 14, 32));
                static HBRUSH hEditBg = NULL;
                if (!hEditBg) hEditBg = CreateSolidBrush(RGB(16, 14, 32));
                return (INT_PTR)hEditBg;
            }
            break;
        }

        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, hBgBrush);
            return 1;
        }

        case WM_DESTROY:
            if (hBgBrush) DeleteObject(hBgBrush);
            if (hPanelBrush) DeleteObject(hPanelBrush);
            if (hCrucibleBrush) DeleteObject(hCrucibleBrush);
            if (hVesselBrush) DeleteObject(hVesselBrush);
            if (hGoldBadgeBrush) DeleteObject(hGoldBadgeBrush);
            if (hVesselPen) DeleteObject(hVesselPen);
            if (hPurplePen) DeleteObject(hPurplePen);
            if (hGoldPen) DeleteObject(hGoldPen);
            if (hInnerGlowPen) DeleteObject(hInnerGlowPen);
            if (hTitleFont) DeleteObject(hTitleFont);
            if (hHeaderFont) DeleteObject(hHeaderFont);
            if (hUIFont) DeleteObject(hUIFont);
            if (hSlotFont) DeleteObject(hSlotFont);
            if (hBadgeFont) DeleteObject(hBadgeFont);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = NULL;
    wc.lpszClassName = "KAlchemyClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowA("KAlchemyClass", "KAlchemy - Fantasy Crafting & Element Discovery",
                               WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
                               CW_USEDEFAULT, CW_USEDEFAULT, 800, 570,
                               NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}

void MainEntry() {
    int ret = WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWDEFAULT);
    ExitProcess(ret);
}
