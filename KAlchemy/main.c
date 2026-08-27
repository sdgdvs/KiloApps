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
#define TOTAL_RECIPES 62
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
    { 54, 30, 55 },// Time + Magic -> Eternity

    // Alternative Recipes (10)
    { 10, 1, 17 }, // Stone + Water -> Sand
    { 11, 1, 15 }, // Plant + Water -> Tree
    { 0, 15, 13 }, // Fire + Tree -> Charcoal
    { 3, 4, 12 },  // Air + Steam -> Cloud
    { 30, 16, 29 },// Magic + Metal -> Golem
    { 30, 28, 41 },// Magic + Life -> Spirit
    { 7, 10, 27 }, // Mud + Stone -> Clay
    { 0, 27, 18 }, // Fire + Clay -> Glass
    { 22, 1, 6 },  // Electricity + Water -> Energy
    { 40, 18, 51 } // Light + Glass -> Starlight
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
    int soundEnabled;       // 1 = Sound Enabled, 0 = Muted
    int showHelpModal;      // 1 = Help Modal Overlay active, 0 = inactive
    int helpActiveTab;      // 0 = How to Play, 1 = Element Tiers, 2 = Lab Controls, 3 = Alchemy Lore
    int screenShakeTime;    // Active screen shake duration
    char lastStatus[128];
    char searchFilter[64];
} AlchemyState;

static const int g_CrucibleCapCosts[5] = { 40, 90, 180, 300, 450 };
static const int g_EssenceYieldCosts[5] = { 30, 70, 140, 250, 400 };
static const int g_AutoSorterCosts[5] = { 50, 100, 220, 400, 600 };
static const int g_CatalystSpeedCosts[5] = { 35, 80, 160, 280, 450 };


#define MAX_PARTICLES 200
typedef struct {
    float x, y;
    float vx, vy;
    float life;
    float decay;
    COLORREF color;
    int type; // 0=core, 1=spark
} Particle;
static Particle g_Particles[MAX_PARTICLES];

static void SpawnExplosion(int cx, int cy, COLORREF color) {
    g_State.screenShakeTime = 15;
    for (int i = 0; i < 80; i++) {
        for (int j = 0; j < MAX_PARTICLES; j++) {
            if (g_Particles[j].life <= 0.0f) {
                g_Particles[j].x = (float)cx;
                g_Particles[j].y = (float)cy;
                if (i % 2 == 0) {
                    g_Particles[j].vx = (float)((FastRand() % 100) - 50) * 0.25f;
                    g_Particles[j].vy = (float)((FastRand() % 100) - 50) * 0.25f - 4.0f;
                    g_Particles[j].color = RGB(255, 255, 255);
                    g_Particles[j].type = 1;
                } else {
                    g_Particles[j].vx = (float)((FastRand() % 100) - 50) * 0.15f;
                    g_Particles[j].vy = (float)((FastRand() % 100) - 50) * 0.15f - 2.0f;
                    g_Particles[j].color = color;
                    g_Particles[j].type = 0;
                }
                g_Particles[j].life = 1.2f;
                g_Particles[j].decay = 0.02f + (float)(FastRand() % 20) * 0.001f;
                break;
            }
        }
    }
}

static AlchemyState g_State;
static HWND g_hGridButtons[GRID_SIZE];
static HWND g_hTierButtons[TOTAL_TIERS + 1];
static HWND g_hEquipButtons[8];
static HWND g_hModeButtons[3];
static HWND g_hBlitzStartButton = NULL;
static HWND g_hPuzzleSkipButton = NULL;
DWORD g_SigilEndTick = 0;
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
static HWND g_hSoundButton = NULL;
static HWND g_hHelpButton = NULL;
static HWND g_hHelpTabButtons[4] = { NULL };
static HWND g_hHelpCloseButton = NULL;

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

// Phase 13 Win32 Beep Sound Effects Synthesizer
static void PlayGlassClink() {
    if (!g_State.soundEnabled) return;
    Beep(1800, 30);
    Beep(2400, 40);
}

static void PlayTransmuteZap() {
    if (!g_State.soundEnabled) return;
    Beep(220, 20);
    Beep(440, 20);
    Beep(880, 25);
    Beep(1760, 30);
    Beep(660, 20);
}

static void PlayDiscoveryChime() {
    if (!g_State.soundEnabled) return;
    Beep(1046, 50);  // C6
    Beep(1318, 50);  // E6
    Beep(1568, 50);  // G6
    Beep(2093, 80);  // C7
}

static void PlayMagicFanfare() {
    if (!g_State.soundEnabled) return;
    Beep(523, 60);   // C5
    Beep(659, 60);   // E5
    Beep(784, 60);   // G5
    Beep(1046, 90);  // C6
    Beep(1318, 120); // E6
}

static void PlayBubbleSimmer() {
    if (!g_State.soundEnabled) return;
    Beep(320, 25);
    Beep(420, 25);
    Beep(360, 25);
    Beep(480, 30);
}

static void PlayAnvilCrushSound() {
    if (!g_State.soundEnabled) return;
    Beep(150, 40);
    Beep(300, 40);
    Beep(600, 50);
}

static void PlayDiscoveryFanfare() {
    PlayDiscoveryChime();
}

static void PlayTierUnlockFanfare() {
    PlayMagicFanfare();
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
    g_State.quests[slotIdx].goldReward = g_Elements[target].tier * 40 + (FastRand() % 30);
    g_State.quests[slotIdx].xpReward = g_Elements[target].tier * 60;
}

static void UpdateEquipmentUI(HWND hwnd) {
    if (g_State.showHelpModal) {
        for (int t = 0; t < 4; t++) {
            if (g_hHelpTabButtons[t]) ShowWindow(g_hHelpTabButtons[t], SW_SHOW);
        }
        if (g_hHelpCloseButton) ShowWindow(g_hHelpCloseButton, SW_SHOW);

        for (int k = 0; k < GRID_SIZE; k++) if (g_hGridButtons[k]) ShowWindow(g_hGridButtons[k], SW_HIDE);
        for (int t = 0; t <= TOTAL_TIERS; t++) if (g_hTierButtons[t]) ShowWindow(g_hTierButtons[t], SW_HIDE);
        for (int e = 0; e < 8; e++) if (g_hEquipButtons[e]) ShowWindow(g_hEquipButtons[e], SW_HIDE);
        for (int m = 0; m < 3; m++) if (g_hModeButtons[m]) ShowWindow(g_hModeButtons[m], SW_HIDE);
        if (g_hBlitzStartButton) ShowWindow(g_hBlitzStartButton, SW_HIDE);
        if (g_hPuzzleSkipButton) ShowWindow(g_hPuzzleSkipButton, SW_HIDE);
        if (g_hSoundButton) ShowWindow(g_hSoundButton, SW_HIDE);
        if (g_hHelpButton) ShowWindow(g_hHelpButton, SW_HIDE);
        if (g_hSlot1Button) ShowWindow(g_hSlot1Button, SW_HIDE);
        if (g_hSlot2Button) ShowWindow(g_hSlot2Button, SW_HIDE);
        if (g_hMainActionButton) ShowWindow(g_hMainActionButton, SW_HIDE);
        if (g_hAutoFillButton) ShowWindow(g_hAutoFillButton, SW_HIDE);
        if (g_hSearchEdit) ShowWindow(g_hSearchEdit, SW_HIDE);
        if (g_hJournalEdit) ShowWindow(g_hJournalEdit, SW_HIDE);
        if (g_hPrevButton) ShowWindow(g_hPrevButton, SW_HIDE);
        if (g_hNextButton) ShowWindow(g_hNextButton, SW_HIDE);
        if (g_hPageText) ShowWindow(g_hPageText, SW_HIDE);
        if (g_hQuestRerollButton) ShowWindow(g_hQuestRerollButton, SW_HIDE);
        for (int c = 0; c < 3; c++) if (g_hCodexFilterBtns[c]) ShowWindow(g_hCodexFilterBtns[c], SW_HIDE);
        for (int p = 0; p < 4; p++) if (g_hPotionDrinkButtons[p]) ShowWindow(g_hPotionDrinkButtons[p], SW_HIDE);
        for (int q = 0; q < 3; q++) if (g_hQuestTurnInButtons[q]) ShowWindow(g_hQuestTurnInButtons[q], SW_HIDE);
        for (int u = 0; u < 4; u++) if (g_hUpgradeButtons[u]) ShowWindow(g_hUpgradeButtons[u], SW_HIDE);
        return;
    } else {
        for (int t = 0; t < 4; t++) {
            if (g_hHelpTabButtons[t]) ShowWindow(g_hHelpTabButtons[t], SW_HIDE);
        }
        if (g_hHelpCloseButton) ShowWindow(g_hHelpCloseButton, SW_HIDE);

        for (int t = 0; t <= TOTAL_TIERS; t++) if (g_hTierButtons[t]) ShowWindow(g_hTierButtons[t], SW_SHOW);
        for (int e = 0; e < 8; e++) if (g_hEquipButtons[e]) ShowWindow(g_hEquipButtons[e], SW_SHOW);
        for (int m = 0; m < 3; m++) if (g_hModeButtons[m]) ShowWindow(g_hModeButtons[m], SW_SHOW);
        if (g_hSoundButton) ShowWindow(g_hSoundButton, SW_SHOW);
        if (g_hHelpButton) ShowWindow(g_hHelpButton, SW_SHOW);
        if (g_hSearchEdit) ShowWindow(g_hSearchEdit, SW_SHOW);
        if (g_hJournalEdit) ShowWindow(g_hJournalEdit, SW_SHOW);
        if (g_hPrevButton) ShowWindow(g_hPrevButton, SW_SHOW);
        if (g_hNextButton) ShowWindow(g_hNextButton, SW_SHOW);
        if (g_hPageText) ShowWindow(g_hPageText, SW_SHOW);
        UpdateGrimoireGrid();
    }

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
    } else {
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
    g_State.soundEnabled = 1;
    g_State.showHelpModal = 0;
    g_State.helpActiveTab = 0;
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

            hTitleFont = CreateFontA(-21, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hHeaderFont = CreateFontA(-15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hUIFont = CreateFontA(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hSlotFont = CreateFontA(-14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hBadgeFont = CreateFontA(-12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

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
            g_hModeButtons[0] = CreateWindowA("BUTTON", "Classic", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 276, 70, 45, 22, hwnd, (HMENU)1200, NULL, NULL);
            g_hModeButtons[1] = CreateWindowA("BUTTON", "Blitz", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 323, 70, 38, 22, hwnd, (HMENU)1201, NULL, NULL);
            g_hModeButtons[2] = CreateWindowA("BUTTON", "Puzzle", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 363, 70, 44, 22, hwnd, (HMENU)1202, NULL, NULL);

            g_hBlitzStartButton = CreateWindowA("BUTTON", "▶️ Start", WS_CHILD | BS_PUSHBUTTON, 409, 70, 50, 22, hwnd, (HMENU)1203, NULL, NULL);
            g_hPuzzleSkipButton = CreateWindowA("BUTTON", "🔄 Skip", WS_CHILD | BS_PUSHBUTTON, 409, 70, 50, 22, hwnd, (HMENU)1204, NULL, NULL);
            g_hSoundButton = CreateWindowA("BUTTON", "🔊 ON", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 451, 70, 48, 22, hwnd, (HMENU)1300, NULL, NULL);
            g_hHelpButton = CreateWindowA("BUTTON", "📘 Manual (H)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 502, 70, 85, 22, hwnd, (HMENU)1400, NULL, NULL);

            // Help Modal Tab Buttons & Close Button (Initially hidden)
            g_hHelpTabButtons[0] = CreateWindowA("BUTTON", "🎮 How to Play", WS_CHILD | BS_PUSHBUTTON, 35, 55, 110, 26, hwnd, (HMENU)1401, NULL, NULL);
            g_hHelpTabButtons[1] = CreateWindowA("BUTTON", "📊 Element Tiers", WS_CHILD | BS_PUSHBUTTON, 150, 55, 110, 26, hwnd, (HMENU)1402, NULL, NULL);
            g_hHelpTabButtons[2] = CreateWindowA("BUTTON", "⚗️ Lab Controls", WS_CHILD | BS_PUSHBUTTON, 265, 55, 110, 26, hwnd, (HMENU)1403, NULL, NULL);
            g_hHelpTabButtons[3] = CreateWindowA("BUTTON", "📜 Alchemy Lore", WS_CHILD | BS_PUSHBUTTON, 380, 55, 110, 26, hwnd, (HMENU)1404, NULL, NULL);
            g_hHelpCloseButton = CreateWindowA("BUTTON", "✕ Close Manual", WS_CHILD | BS_PUSHBUTTON, 640, 55, 105, 26, hwnd, (HMENU)1405, NULL, NULL);

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

            AddJournalLog("Welcome Apprentice Alchemist!\r\nSelect elements from your Grimoire to combine in the Crucible across 5 Tiers!\r\n\r\nPress 'H' to open the Grandmaster Manual for help!");
            break;
        }

        case WM_KEYDOWN: {
            int key = (int)wParam;
            if (key == VK_ESCAPE) {
                if (g_State.showHelpModal) {
                    g_State.showHelpModal = 0;
                    UpdateEquipmentUI(hwnd);
                    PlayGlassClink();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (key == 'H' || key == 'h') {
                g_State.showHelpModal = !g_State.showHelpModal;
                UpdateEquipmentUI(hwnd);
                PlayGlassClink();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (!g_State.showHelpModal) {
                if (key == 'C' || key == 'c') {
                    g_State.slot1 = -1;
                    g_State.slot2 = -1;
                    UpdateSlotButtonText();
                    PlayGlassClink();
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (key == 'A' || key == 'a') {
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(904, 0), 0);
                } else if (key >= '0' && key <= '5') {
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(500 + (key - '0'), 0), 0);
                }
            }
            break;
        }

        case WM_TIMER: {
            int changed = 0;
            if (g_State.buffStrengthTimer > 0) { g_State.buffStrengthTimer--; changed = 1; }
            if (g_State.buffInvisibilityTimer > 0) { g_State.buffInvisibilityTimer--; changed = 1; }
            if (g_State.buffManaTimer > 0) { g_State.buffManaTimer--; changed = 1; }
            if (g_State.buffLifeTimer > 0) { g_State.buffLifeTimer--; changed = 1; }
            if (g_State.screenShakeTime > 0) { g_State.screenShakeTime--; changed = 1; }
            // Particle update
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (g_Particles[i].life > 0.0f) {
                    g_Particles[i].x += g_Particles[i].vx;
                    g_Particles[i].y += g_Particles[i].vy;
                    g_Particles[i].life -= g_Particles[i].decay;
                    changed = 1;
                }
            }
            // Animation update
            RECT vesselRect = { 300, 110, 480, 230 };
            InvalidateRect(hwnd, &vesselRect, FALSE);
            changed = 1;

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

            // Open / Toggle Help Modal (1400)
            if (id == 1400) {
                g_State.showHelpModal = !g_State.showHelpModal;
                UpdateEquipmentUI(hwnd);
                PlayGlassClink();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Switch Help Modal Tabs (1401..1404)
            else if (id >= 1401 && id <= 1404) {
                g_State.helpActiveTab = id - 1401;
                PlayGlassClink();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Close Help Modal (1405)
            else if (id == 1405) {
                g_State.showHelpModal = 0;
                UpdateEquipmentUI(hwnd);
                PlayGlassClink();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Search filter edit box changed
            else if (id == 401 && code == EN_CHANGE) {
                GetWindowTextA(g_hSearchEdit, g_State.searchFilter, sizeof(g_State.searchFilter));
                g_State.currentPage = 0;
                UpdateGrimoireGrid();
            }
            // Sound Toggle (1300)
            else if (id == 1300) {
                g_State.soundEnabled = !g_State.soundEnabled;
                if (g_hSoundButton) {
                    SetWindowTextA(g_hSoundButton, g_State.soundEnabled ? "🔊 ON" : "🔇 OFF");
                }
                if (g_State.soundEnabled) PlayGlassClink();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Tier Filter Buttons (500 = All, 501..505 = T1..T5)
            else if (id >= 500 && id <= 500 + TOTAL_TIERS) {
                g_State.selectedTierFilter = id - 500;
                g_State.currentPage = 0;
                UpdateGrimoireGrid();
                PlayGlassClink();
            }
            // Grid element buttons (100 to 109)
            else if (id >= 100 && id < 100 + GRID_SIZE) {
                int btnIdx = id - 100;
                int elemIdx = g_State.buttonElemMap[btnIdx];
                if (elemIdx >= 0 && elemIdx < TOTAL_ELEMENTS) {
                    if (g_State.selectedEquipment == 7) {
                        g_State.selectedCodexElem = elemIdx;
                        InvalidateRect(hwnd, NULL, TRUE);
                        PlayGlassClink();
                    } else if (g_State.discovered[elemIdx]) {
                        if (g_State.slot1 == -1) {
                            g_State.slot1 = elemIdx;
                        } else if (g_State.slot2 == -1) {
                            g_State.slot2 = elemIdx;
                        } else {
                            g_State.slot1 = elemIdx;
                        }
                        UpdateSlotButtonText();
                        PlayGlassClink();
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                }
            }
            // Page navigation
            else if (id == 601) { // Prev
                if (g_State.currentPage > 0) {
                    g_State.currentPage--;
                    UpdateGrimoireGrid();
                    PlayGlassClink();
                }
            }
            else if (id == 602) { // Next
                g_State.currentPage++;
                UpdateGrimoireGrid();
                PlayGlassClink();
            }
            // Clear Slot 1
            else if (id == 301) {
                g_State.slot1 = -1;
                UpdateSlotButtonText();
                PlayGlassClink();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Clear Slot 2
            else if (id == 302) {
                g_State.slot2 = -1;
                UpdateSlotButtonText();
                PlayGlassClink();
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
                PlayGlassClink();
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
                    PlayBubbleSimmer(); PlayMagicFanfare();
                } else if (pIdx == 1 && g_State.potionInvisibility > 0) {
                    g_State.potionInvisibility--;
                    g_State.buffInvisibilityTimer += 60;
                    AddJournalLog("🧪 EFFECT TESTER: Consumed Invisibility Elixir! (Stealth Aura Active 60s)");
                    wsprintfA(g_State.lastStatus, "Active Buff: Invisibility Elixir (60s)");
                    PlayBubbleSimmer(); PlayMagicFanfare();
                } else if (pIdx == 2 && g_State.potionMana > 0) {
                    g_State.potionMana--;
                    g_State.buffManaTimer += 60;
                    AddJournalLog("🧪 EFFECT TESTER: Consumed Mana Elixir! (Mana Surge Active 60s)");
                    wsprintfA(g_State.lastStatus, "Active Buff: Mana Elixir (60s)");
                    PlayBubbleSimmer(); PlayMagicFanfare();
                } else if (pIdx == 3 && g_State.potionLife > 0) {
                    g_State.potionLife--;
                    g_State.buffLifeTimer += 60;
                    AddJournalLog("🧪 EFFECT TESTER: Consumed Elixir of Life! (Divine Radiance Active 60s)");
                    wsprintfA(g_State.lastStatus, "Active Buff: Elixir of Life (60s)");
                    PlayBubbleSimmer(); PlayMagicFanfare();
                } else {
                    AddJournalLog("⚠️ No potions of this type in inventory!");
                    PlayGlassClink();
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
                        PlayGlassClink(); PlayMagicFanfare();
                        UpdateEquipmentUI(hwnd);
                    } else {
                        AddJournalLog("⚠️ Not enough Gold for Workshop Upgrade!");
                        PlayGlassClink();
                    }
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Auto-Fill Crucible Button (904)
            else if (id == 904) {
                if (g_State.upgradeAutoSorter <= 0) {
                    AddJournalLog("⚠️ Unlock Auto-Sorter in the Enchanter Workshop first!");
                    PlayGlassClink();
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
                        PlayTransmuteZap();
                    } else {
                        AddJournalLog("⚡ Auto-Sorter: No valid ingredient combinations found in Grimoire!");
                        PlayGlassClink();
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

                    PlayMagicFanfare();

                    GenerateQuest(qIdx);
                    UpdateEquipmentUI(hwnd);
                    InvalidateRect(hwnd, NULL, TRUE);
                } else {
                    AddJournalLog("⚠️ You must discover/craft the requested element before turning in!");
                    PlayGlassClink();
                }
            }
            // Quest Reroll Button (803)
            else if (id == 803) {
                const int COST = 15;
                if (g_State.gold < COST) {
                    lstrcpyA(g_State.lastStatus, "Need 15 Gold to reroll quests!");
                    AddJournalLog("⚠️ Not enough Gold to reroll quest board! (Cost: 15 Gold)");
                    PlayGlassClink();
                } else {
                    g_State.gold -= COST;
                    GenerateQuest(0);
                    GenerateQuest(1);
                    GenerateQuest(2);
                    AddJournalLog("🔄 Rerolled Master Alchemist Guild Quest Board (-15 Gold).");
                    lstrcpyA(g_State.lastStatus, "Guild Quests Rerolled");
                    PlayGlassClink();
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
                PlayGlassClink();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (id == 1201) {
                g_State.gameMode = 1;
                AddJournalLog("⚡ TIMED ALCHEMY BLITZ: Click 'Start Blitz' to begin the 60s challenge!");
                lstrcpyA(g_State.lastStatus, "Blitz Challenge Ready");
                UpdateEquipmentUI(hwnd);
                PlayGlassClink();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (id == 1202) {
                g_State.gameMode = 2;
                AddJournalLog("🧩 PUZZLE CRUCIBLE: Synthesize the target element goal!");
                lstrcpyA(g_State.lastStatus, "Puzzle Crucible Active");
                UpdateEquipmentUI(hwnd);
                PlayGlassClink();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (id == 1203) { // Start/Restart Blitz
                g_State.blitzTimeLeft = 60;
                g_State.blitzScore = 0;
                g_State.blitzActive = 1;
                AddJournalLog("⚡ BLITZ STARTED! Discover compounds before 60s expires!");
                lstrcpyA(g_State.lastStatus, "Blitz Challenge Active!");
                PlayMagicFanfare();
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
                PlayGlassClink();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Main Action (Transmute / Distill / Extract / Crush)
            else if (id == 201) {
                if (g_State.selectedEquipment == 0) { // Crucible Transmute
                    if (g_State.slot1 < 0 || g_State.slot2 < 0) {
                        lstrcpyA(g_State.lastStatus, "Select 2 elements for Crucible!");
                        AddJournalLog("Place two elements into the Crucible before transmuting.");
                        PlayGlassClink();
                    } else {
                        PlayTransmuteZap();
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
                                PlayGlassClink();
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
                                        PlayMagicFanfare();

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
                                    SpawnExplosion(392, 168, g_Tiers[resTier - 1].color);
                                    
                                    extern DWORD g_SigilEndTick;
                                    if (resTier >= 3) {
                                        g_SigilEndTick = GetTickCount() + 1500;
                                    }

                                    char logMsg[320];
                                    wsprintfA(logMsg, "✨ NEW DISCOVERY! You created %s [Tier %d %s] by combining %s + %s! (+%d Dust)%s",
                                        g_Elements[res].name, resTier, g_Tiers[resTier - 1].name, g_Elements[e1].name, g_Elements[e2].name, dustGain, critLogStr);
                                    AddJournalLog(logMsg);

                                    PlayDiscoveryChime();
                                } else {
                                    wsprintfA(g_State.lastStatus, "Created %s (Known)", g_Elements[res].name);
                                    char logMsg[256];
                                    wsprintfA(logMsg, "Created %s (%s + %s). Already recorded in Grimoire.",
                                        g_Elements[res].name, g_Elements[e1].name, g_Elements[e2].name);
                                    AddJournalLog(logMsg);
                                    PlayBubbleSimmer();
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
                            PlayGlassClink();
                        }
                    }
                } else if (g_State.selectedEquipment == 1) { // Retort Distillation
                    if (g_State.slot1 < 0) {
                        lstrcpyA(g_State.lastStatus, "Place complex element in Slot 1!");
                        AddJournalLog("Place a complex element into Slot 1 to distill in the Retort.");
                        PlayGlassClink();
                    } else {
                        int e1 = g_State.slot1;
                        if (g_Elements[e1].isBasic || g_Elements[e1].tier == 1) {
                            wsprintfA(g_State.lastStatus, "Primordial %s cannot be distilled!", g_Elements[e1].name);
                            AddJournalLog("⚠️ Primordial base elements cannot be distilled!");
                            PlayGlassClink();
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
                                PlayBubbleSimmer(); PlayGlassClink();
                            }
                        }
                    }
                } else if (g_State.selectedEquipment == 2) { // Alembic Extraction
                    if (g_State.slot1 < 0) {
                        lstrcpyA(g_State.lastStatus, "Place complex element in Slot 1!");
                        AddJournalLog("Place a complex element into Slot 1 to extract in the Alembic.");
                        PlayGlassClink();
                    } else {
                        int e1 = g_State.slot1;
                        if (g_Elements[e1].isBasic || g_Elements[e1].tier == 1) {
                            wsprintfA(g_State.lastStatus, "Primordial %s cannot be extracted!", g_Elements[e1].name);
                            AddJournalLog("⚠️ Primordial base elements cannot be extracted!");
                            PlayGlassClink();
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
                                PlayBubbleSimmer(); PlayGlassClink();
                            }
                        }
                    }
                } else if (g_State.selectedEquipment == 3) { // Anvil Crushing
                    if (g_State.slot1 < 0) {
                        lstrcpyA(g_State.lastStatus, "Place complex element in Slot 1!");
                        AddJournalLog("Place a complex element into Slot 1 to crush on the Anvil.");
                        PlayGlassClink();
                    } else {
                        int e1 = g_State.slot1;
                        if (g_Elements[e1].isBasic || g_Elements[e1].tier == 1) {
                            wsprintfA(g_State.lastStatus, "Primordial %s cannot be crushed!", g_Elements[e1].name);
                            AddJournalLog("⚠️ Primordial base elements cannot be crushed!");
                            PlayGlassClink();
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
                            PlayAnvilCrushSound();
                        }
                    }
                } else if (g_State.selectedEquipment == 6) { // Potion Brewing
                    if (g_State.slot1 < 0 || g_State.slot2 < 0) {
                        lstrcpyA(g_State.lastStatus, "Select herb and essence for brewing!");
                        AddJournalLog("Select an herb/ingredient and essence into slots to brew in Cauldron.");
                        PlayGlassClink();
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
                            PlayBubbleSimmer(); PlayGlassClink();
                        } else if (brewedType == 1) {
                            g_State.potionInvisibility++;
                            AddJournalLog("🥣 BREWING: Combined ingredients to brew Invisibility Elixir (👻)! (+1 Potion)");
                            wsprintfA(g_State.lastStatus, "Brewed Invisibility Elixir (👻)!");
                            PlayBubbleSimmer(); PlayGlassClink();
                        } else if (brewedType == 2) {
                            g_State.potionMana++;
                            AddJournalLog("🥣 BREWING: Combined ingredients to brew Mana Elixir (🔮)! (+1 Potion)");
                            wsprintfA(g_State.lastStatus, "Brewed Mana Elixir (🔮)!");
                            PlayBubbleSimmer(); PlayGlassClink();
                        } else if (brewedType == 3) {
                            g_State.potionLife++;
                            AddJournalLog("🥣 BREWING: Combined ingredients to brew Elixir of Life (❤️)! (+1 Potion)");
                            wsprintfA(g_State.lastStatus, "Brewed Elixir of Life (❤️)!");
                            PlayBubbleSimmer(); PlayGlassClink();
                        } else {
                            g_State.essence += 15;
                            g_State.dust += 15;
                            AddJournalLog("🧪 BREWING: Brewed Minor Tonic! (+15 Essence, +15 Dust)");
                            wsprintfA(g_State.lastStatus, "Brewed Minor Tonic (+15 Ess, +15 Dust)");
                            PlayBubbleSimmer();
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
                PlayGlassClink();
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
                    PlayGlassClink();
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
                    PlayGlassClink();
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
                        PlayDiscoveryChime();
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

                        PlayDiscoveryChime();
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
                    PlayGlassClink();
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
                        PlayDiscoveryChime();
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

                        PlayMagicFanfare();
                    }
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            if (g_State.screenShakeTime > 0) {
                int sx = (FastRand() % 10) - 5;
                int sy = (FastRand() % 10) - 5;
                SetWindowOrgEx(hdc, sx, sy, NULL);
            }

            SetBkMode(hdc, TRANSPARENT);

            // Environmental Art Background: Esoteric geometric patterns
            RECT clRect;
            GetClientRect(hwnd, &clRect);
            FillRect(hdc, &clRect, hBgBrush);
            
            HGDIOBJ oldPenBg = SelectObject(hdc, CreatePen(PS_SOLID, 1, RGB(25, 25, 45)));
            HGDIOBJ oldBrushBg = SelectObject(hdc, GetStockObject(NULL_BRUSH));
            for(int i = -100; i < clRect.right + 100; i += 150) {
                for(int j = -100; j < clRect.bottom + 100; j += 150) {
                    Ellipse(hdc, i, j, i+200, j+200);
                    Rectangle(hdc, i+50, j+50, i+150, j+150);
                    MoveToEx(hdc, i, j, NULL); LineTo(hdc, i+200, j+200);
                    MoveToEx(hdc, i+200, j, NULL); LineTo(hdc, i, j+200);
                }
            }
            DeleteObject(SelectObject(hdc, oldPenBg));
            SelectObject(hdc, oldBrushBg);

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

                // Draw Custom Equipment Graphics with Animations
                SelectObject(hdc, hGoldPen);
                HGDIOBJ pNullB = GetStockObject(NULL_BRUSH);
                SelectObject(hdc, pNullB);
                
                int cx = 392;
                int cy = 168;
                DWORD tick = GetTickCount();

                // Draw pulsing rune ring
                int pulse = (tick / 50) % 10;
                if (pulse > 5) pulse = 10 - pulse;
                Ellipse(hdc, cx - 55 - pulse, cy - 25 - pulse, cx + 55 + pulse, cy + 25 + pulse);

                // Dynamic 3D drop-shadow
                int shadowOffset = 8 + (pulse / 2);
                HBRUSH hShadowBrush = CreateSolidBrush(RGB(10, 10, 15));
                HGDIOBJ oldShadow = SelectObject(hdc, hShadowBrush);
                SelectObject(hdc, GetStockObject(NULL_PEN));
                if (g_State.selectedEquipment == 3) {
                    POINT sAnvil[8] = {{cx-30+shadowOffset, cy-15+shadowOffset}, {cx+30+shadowOffset, cy-15+shadowOffset}, {cx+15+shadowOffset, cy-5+shadowOffset}, {cx+15+shadowOffset, cy+10+shadowOffset}, {cx+30+shadowOffset, cy+20+shadowOffset}, {cx-30+shadowOffset, cy+20+shadowOffset}, {cx-15+shadowOffset, cy+10+shadowOffset}, {cx-15+shadowOffset, cy-5+shadowOffset}};
                    Polygon(hdc, sAnvil, 8);
                } else if (g_State.selectedEquipment == 1 || g_State.selectedEquipment == 2) {
                    Ellipse(hdc, cx - 25 + shadowOffset, cy - 20 + shadowOffset, cx + 25 + shadowOffset, cy + 20 + shadowOffset);
                } else {
                    POINT sFlaskNeck[4] = {{cx-15+shadowOffset, cy-25+shadowOffset}, {cx+15+shadowOffset, cy-25+shadowOffset}, {cx+15+shadowOffset, cy-5+shadowOffset}, {cx-15+shadowOffset, cy-5+shadowOffset}};
                    Polygon(hdc, sFlaskNeck, 4);
                    Ellipse(hdc, cx - 35 + shadowOffset, cy - 10 + shadowOffset, cx + 35 + shadowOffset, cy + 25 + shadowOffset);
                }
                SelectObject(hdc, oldShadow);
                DeleteObject(hShadowBrush);

                SelectObject(hdc, hVesselBrush);
                SelectObject(hdc, hPurplePen);

                if (g_State.selectedEquipment == 3) {
                    // Anvil
                    POINT anvil[8] = {{cx-30, cy-15}, {cx+30, cy-15}, {cx+15, cy-5}, {cx+15, cy+10}, {cx+30, cy+20}, {cx-30, cy+20}, {cx-15, cy+10}, {cx-15, cy-5}};
                    Polygon(hdc, anvil, 8);
                } else if (g_State.selectedEquipment == 1 || g_State.selectedEquipment == 2) {
                    // Retort / Alembic
                    Ellipse(hdc, cx - 25, cy - 20, cx + 25, cy + 20); // Bulb
                    MoveToEx(hdc, cx + 20, cy - 10, NULL);
                    LineTo(hdc, cx + 50, cy + 15); // Tube
                } else {
                    // Crucible / Flask
                    POINT flaskNeck[4] = {{cx-15, cy-25}, {cx+15, cy-25}, {cx+15, cy-5}, {cx-15, cy-5}};
                    Polygon(hdc, flaskNeck, 4);
                    Ellipse(hdc, cx - 35, cy - 10, cx + 35, cy + 25);
                    
                    // Bubbling animation
                    int isValidCombo = 0;
                    if (g_State.slot1 >= 0 && g_State.slot2 >= 0) {
                        for (int r = 0; r < TOTAL_RECIPES; r++) {
                            if ((g_Recipes[r].ingredient1 == g_State.slot1 && g_Recipes[r].ingredient2 == g_State.slot2) ||
                                (g_Recipes[r].ingredient1 == g_State.slot2 && g_Recipes[r].ingredient2 == g_State.slot1)) {
                                isValidCombo = 1;
                                break;
                            }
                        }
                    }

                    SelectObject(hdc, hGoldPen);
                    int bubbleCount = isValidCombo ? 18 : 5;
                    for (int i=0; i<bubbleCount; i++) {
                        int bx = cx - 20 + ((i * 37) % 40);
                        int speed = isValidCombo ? 10 : 20;
                        int by = cy + 20 - ((tick / speed + i * 15) % 30);
                        Ellipse(hdc, bx - 2, by - 2, bx + 2, by + 2);
                    }
                }
                
                // Procedural specular highlights for glass/brass
                SelectObject(hdc, GetStockObject(NULL_PEN));
                HBRUSH hHighlight = CreateSolidBrush(RGB(200, 200, 255));
                HGDIOBJ oldHi = SelectObject(hdc, hHighlight);
                if (g_State.selectedEquipment == 3) {
                    POINT hi[3] = {{cx-20, cy-12}, {cx-10, cy-12}, {cx-15, cy-8}};
                    Polygon(hdc, hi, 3);
                } else if (g_State.selectedEquipment == 1 || g_State.selectedEquipment == 2) {
                    Ellipse(hdc, cx - 18, cy - 15, cx - 8, cy - 5);
                } else {
                    Ellipse(hdc, cx - 25, cy - 5, cx - 15, cy + 10);
                }
                SelectObject(hdc, oldHi);
                DeleteObject(hHighlight);


                // Draw elemental component variations around the slots
                if (g_State.slot1 == 0 || g_State.slot1 == 1) {
                    HPEN varPen = CreatePen(PS_SOLID, 2, g_State.slot1 == 0 ? RGB(255, 100, 0) : RGB(0, 150, 255));
                    HGDIOBJ oldVar = SelectObject(hdc, varPen);
                    int sx = 295 + 45; int sy = 200 + 25;
                    for (int w = 0; w < 8; w++) {
                        int r = 30 + ((tick / 30 + w * 10) % 15);
                        if (g_State.slot1 == 0) { // Fire flame patterns
                            MoveToEx(hdc, sx - 20 + w*5, sy + 25, NULL);
                            LineTo(hdc, sx - 20 + w*5 + (FastRand()%10 - 5), sy + 25 - r);
                        } else { // Water wave distortions
                            int offY = (int)(sin((tick + w*200) * 0.01) * 10.0);
                            MoveToEx(hdc, sx - 30 + w*8, sy + 25, NULL);
                            LineTo(hdc, sx - 30 + w*8, sy + 25 + offY);
                        }
                    }
                    SelectObject(hdc, oldVar); DeleteObject(varPen);
                }
                if (g_State.slot2 == 0 || g_State.slot2 == 1) {
                    HPEN varPen = CreatePen(PS_SOLID, 2, g_State.slot2 == 0 ? RGB(255, 100, 0) : RGB(0, 150, 255));
                    HGDIOBJ oldVar = SelectObject(hdc, varPen);
                    int sx = 405 + 45; int sy = 200 + 25; // Slot 2 position
                    for (int w = 0; w < 8; w++) {
                        int r = 30 + ((tick / 30 + w * 10) % 15);
                        if (g_State.slot2 == 0) {
                            MoveToEx(hdc, sx - 20 + w*5, sy + 25, NULL);
                            LineTo(hdc, sx - 20 + w*5 + (FastRand()%10 - 5), sy + 25 - r);
                        } else {
                            int offY = (int)(sin((tick + w*200) * 0.01) * 10.0);
                            MoveToEx(hdc, sx - 30 + w*8, sy + 25, NULL);
                            LineTo(hdc, sx - 30 + w*8, sy + 25 + offY);
                        }
                    }
                    SelectObject(hdc, oldVar); DeleteObject(varPen);
                }

                // Draw particles as magical sparkles
                for (int i = 0; i < MAX_PARTICLES; i++) {
                    if (g_Particles[i].life > 0.0f) {
                        int px = (int)g_Particles[i].x;
                        int py = (int)g_Particles[i].y;
                        int size = (int)(g_Particles[i].life * (g_Particles[i].type == 1 ? 3.0f : 6.0f));
                        if (size < 1) size = 1;
                        HPEN pPen = CreatePen(PS_SOLID, 1, g_Particles[i].color);
                        HGDIOBJ oldPp = SelectObject(hdc, pPen);
                        if (g_Particles[i].type == 1) {
                            MoveToEx(hdc, px - size, py, NULL); LineTo(hdc, px + size, py);
                            MoveToEx(hdc, px, py - size, NULL); LineTo(hdc, px, py + size);
                        } else {
                            MoveToEx(hdc, px - size, py, NULL); LineTo(hdc, px + size + 1, py);
                            MoveToEx(hdc, px, py - size, NULL); LineTo(hdc, px, py + size + 1);
                            MoveToEx(hdc, px - size/2, py - size/2, NULL); LineTo(hdc, px + size/2 + 1, py + size/2 + 1);
                            MoveToEx(hdc, px - size/2, py + size/2, NULL); LineTo(hdc, px + size/2 + 1, py - size/2 - 1);
                        }
                        SelectObject(hdc, oldPp);
                        DeleteObject(pPen);
                    }
                }

                // Distinctly stylized multi-layered magical sigil
                extern DWORD g_SigilEndTick;
                if (tick < g_SigilEndTick) {
                    int scx = 392, scy = 168;
                    HPEN sigilPen = CreatePen(PS_SOLID, 3, RGB(241, 196, 15));
                    HGDIOBJ oldSigil = SelectObject(hdc, sigilPen);
                    HBRUSH oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    
                    int radius = 90 + (tick % 15);
                    Ellipse(hdc, scx - radius, scy - radius, scx + radius, scy + radius);
                    POINT star[6] = {
                        {scx, scy - radius}, {scx + radius*86/100, scy + radius/2}, 
                        {scx - radius*86/100, scy + radius/2}, {scx, scy + radius},
                        {scx - radius*86/100, scy - radius/2}, {scx + radius*86/100, scy - radius/2}
                    };
                    Polygon(hdc, star, 3);
                    Polygon(hdc, &star[3], 3);

                    SelectObject(hdc, oldBrush);
                    SelectObject(hdc, oldSigil);
                    DeleteObject(sigilPen);
                }

                SelectObject(hdc, hSlotFont);
                SetTextColor(hdc, RGB(241, 196, 15));
                RECT vLabelRect = { cx - 50, cy - 15, cx + 50, cy + 15 };
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

            // Phase 14: Grandmaster Help & Manual Modal Overlay Drawing
            if (g_State.showHelpModal) {
                RECT modalBg = { 20, 20, 765, 523 };
                FillRect(hdc, &modalBg, hBgBrush);

                HGDIOBJ oldP = SelectObject(hdc, hGoldPen);
                Rectangle(hdc, modalBg.left, modalBg.top, modalBg.right, modalBg.bottom);
                SelectObject(hdc, hPurplePen);
                Rectangle(hdc, modalBg.left + 2, modalBg.top + 2, modalBg.right - 2, modalBg.bottom - 2);

                SelectObject(hdc, hTitleFont);
                SetTextColor(hdc, RGB(243, 156, 18));
                TextOutA(hdc, 35, 26, "GRANDMASTER ALCHEMY MANUAL & LAB REFERENCE", 42);

                SelectObject(hdc, hBadgeFont);
                SetTextColor(hdc, RGB(176, 92, 219));
                TextOutA(hdc, 500, 30, "Arcane Knowledge Codex", 22);

                // Tab Separator Line
                SelectObject(hdc, hGoldPen);
                MoveToEx(hdc, 25, 87, NULL);
                LineTo(hdc, 760, 87);

                SelectObject(hdc, hUIFont);
                SetTextColor(hdc, RGB(226, 232, 240));

                if (g_State.helpActiveTab == 0) { // How to Play
                    SelectObject(hdc, hHeaderFont);
                    SetTextColor(hdc, RGB(241, 196, 15));
                    TextOutA(hdc, 35, 98, "Core Alchemy Loop & Transmutation Rules:", 41);

                    SelectObject(hdc, hUIFont);
                    SetTextColor(hdc, RGB(226, 232, 240));
                    TextOutA(hdc, 35, 122, "1. Select Slot 1 and Slot 2 from your Elemental Grimoire.", 57);
                    TextOutA(hdc, 35, 140, "2. Click 'Transmute Elements' to forge new compounds in the Crucible.", 69);
                    TextOutA(hdc, 35, 158, "3. Discovered elements unlock permanently across 5 Alchemical Tiers.", 67);
                    TextOutA(hdc, 35, 176, "4. Failed combinations yield Slag or Smoke (+5 Arcane Dust research bonus).", 74);

                    SelectObject(hdc, hHeaderFont);
                    SetTextColor(hdc, RGB(241, 196, 15));
                    TextOutA(hdc, 35, 205, "Laboratory Equipment & Workstations:", 36);

                    SelectObject(hdc, hUIFont);
                    SetTextColor(hdc, RGB(226, 232, 240));
                    TextOutA(hdc, 35, 228, "• Crucible: Standard 2-element synthesis vessel.", 48);
                    TextOutA(hdc, 35, 246, "• Retort: Purifies elements into Arcane Essence & Dust.", 55);
                    TextOutA(hdc, 35, 264, "• Alembic: Distills magical reagents into potent elixirs.", 56);
                    TextOutA(hdc, 35, 282, "• Anvil: Shatters high-tier artifacts into base components.", 58);
                    TextOutA(hdc, 35, 300, "• Quests: Fulfill Guild patron orders for Gold & XP rewards.", 58);
                    TextOutA(hdc, 35, 318, "• Shop: Upgrade lab equipment capacity & unlock Auto-Sorter.", 59);
                    TextOutA(hdc, 35, 336, "• Potions: Consume elixirs for temporary yield & XP boosts.", 57);
                    TextOutA(hdc, 35, 354, "• Codex: Comprehensive elemental directory with full recipes & lore.", 67);

                    SelectObject(hdc, hHeaderFont);
                    SetTextColor(hdc, RGB(241, 196, 15));
                    TextOutA(hdc, 35, 385, "Challenge Game Modes & Alchemist's Oracle:", 42);

                    SelectObject(hdc, hUIFont);
                    SetTextColor(hdc, RGB(226, 232, 240));
                    TextOutA(hdc, 35, 408, "• Classic: Sandbox element discovery at your own pace.", 54);
                    TextOutA(hdc, 35, 426, "• Timed Blitz: 60-second speed challenge to rack up high scores!", 64);
                    TextOutA(hdc, 35, 444, "• Puzzle Crucible: Synthesize target elements in minimum moves.", 63);
                    TextOutA(hdc, 35, 462, "• Oracle: Spend Arcane Dust for Vague Hints (20 Dust) or Vision (50 Dust).", 72);

                } else if (g_State.helpActiveTab == 1) { // Element Tiers Guide
                    SelectObject(hdc, hHeaderFont);
                    SetTextColor(hdc, RGB(52, 152, 219));
                    TextOutA(hdc, 35, 98, "Tier 1 — Primordial Basics (4 Elements)", 39);
                    SelectObject(hdc, hUIFont); SetTextColor(hdc, RGB(226, 232, 240));
                    TextOutA(hdc, 35, 120, "Fire, Water, Earth, Air — Natural building blocks provided at apprenticeship start.", 82);

                    SelectObject(hdc, hHeaderFont);
                    SetTextColor(hdc, RGB(46, 204, 113));
                    TextOutA(hdc, 35, 155, "Tier 2 — Nature & Raw Resources (10 Elements)", 45);
                    SelectObject(hdc, hUIFont); SetTextColor(hdc, RGB(226, 232, 240));
                    TextOutA(hdc, 35, 178, "Steam, Mud, Lava, Dust, Energy, Rain, Plant, Metal, Pressure, Stone.", 68);

                    SelectObject(hdc, hHeaderFont);
                    SetTextColor(hdc, RGB(230, 126, 34));
                    TextOutA(hdc, 35, 215, "Tier 3 — Metallurgy & Crafting Reagents (14 Elements)", 53);
                    SelectObject(hdc, hUIFont); SetTextColor(hdc, RGB(226, 232, 240));
                    TextOutA(hdc, 35, 238, "Glass, Steel, Life, Lightning, Firestorm, Clay, Charcoal, Cloud, etc.", 69);

                    SelectObject(hdc, hHeaderFont);
                    SetTextColor(hdc, RGB(155, 89, 182));
                    TextOutA(hdc, 35, 275, "Tier 4 — Arcane Transmutations (16 Elements)", 44);
                    SelectObject(hdc, hUIFont); SetTextColor(hdc, RGB(226, 232, 240));
                    TextOutA(hdc, 35, 298, "Golem, Electricity, Crystal, Magma Orb, Phoenix, Mana Core, etc.", 64);

                    SelectObject(hdc, hHeaderFont);
                    SetTextColor(hdc, RGB(241, 196, 15));
                    TextOutA(hdc, 35, 335, "Tier 5 — Celestial Apex Artifacts (12 Elements)", 47);
                    SelectObject(hdc, hUIFont); SetTextColor(hdc, RGB(226, 232, 240));
                    TextOutA(hdc, 35, 358, "Philosopher's Stone, Elixir of Life, Celestial Star, Primal Void, Cosmarch Orb.", 79);

                } else if (g_State.helpActiveTab == 2) { // Lab Controls Reference
                    SelectObject(hdc, hHeaderFont);
                    SetTextColor(hdc, RGB(241, 196, 15));
                    TextOutA(hdc, 35, 98, "Mouse & Workstation Controls:", 29);

                    SelectObject(hdc, hUIFont); SetTextColor(hdc, RGB(226, 232, 240));
                    TextOutA(hdc, 35, 122, "• Click Grimoire Element: Auto-loads element into active Crucible slot.", 70);
                    TextOutA(hdc, 35, 142, "• Click Slot 1 / Slot 2: Clears element from that slot.", 55);
                    TextOutA(hdc, 35, 162, "• Auto-Fill Button: (Requires Auto-Sorter) Loads valid missing recipe.", 70);
                    TextOutA(hdc, 35, 182, "• Sound Button: Click top 'ON' / 'OFF' button to toggle audio FX.", 64);

                    SelectObject(hdc, hHeaderFont);
                    SetTextColor(hdc, RGB(241, 196, 15));
                    TextOutA(hdc, 35, 222, "Keyboard Shortcuts Reference:", 29);

                    SelectObject(hdc, hUIFont); SetTextColor(hdc, RGB(226, 232, 240));
                    TextOutA(hdc, 35, 246, "[1] - [5]  : Switch Tier Filters (T1 to T5)", 42);
                    TextOutA(hdc, 35, 269, "[0]        : Show All Grimoire Elements", 38);
                    TextOutA(hdc, 35, 292, "[C]        : Clear Crucible Slots", 33);
                    TextOutA(hdc, 35, 315, "[A]        : Trigger Auto-Sorter Fill", 37);
                    TextOutA(hdc, 35, 338, "[H]        : Open / Toggle Grandmaster Manual", 45);
                    TextOutA(hdc, 35, 361, "[ESC]      : Close Manual Overlay", 33);

                } else if (g_State.helpActiveTab == 3) { // Alchemy Lore Encyclopedia
                    SelectObject(hdc, hHeaderFont);
                    SetTextColor(hdc, RGB(241, 196, 15));
                    TextOutA(hdc, 35, 98, "The Dawn of the Great Synthesis:", 31);
                    SelectObject(hdc, hUIFont); SetTextColor(hdc, RGB(226, 232, 240));
                    TextOutA(hdc, 35, 120, "In the dawn of creation, the Primal Tetrahedron split into Fire, Water, Earth, and Air.", 86);
                    TextOutA(hdc, 35, 138, "Alchemists discovered that all physical matter is but a transient state awaiting elevation.", 91);

                    SelectObject(hdc, hHeaderFont);
                    SetTextColor(hdc, RGB(241, 196, 15));
                    TextOutA(hdc, 35, 175, "The Guild of Grandmaster Alchemists:", 35);
                    SelectObject(hdc, hUIFont); SetTextColor(hdc, RGB(226, 232, 240));
                    TextOutA(hdc, 35, 198, "Founded in the Citadel of Aethelgard, the Guild maps all 56 Sacred Reagents across 5 Tiers.", 90);
                    TextOutA(hdc, 35, 216, "Only those who forge the Philosopher's Stone attain the supreme rank of Grandmaster.", 84);

                    SelectObject(hdc, hHeaderFont);
                    SetTextColor(hdc, RGB(241, 196, 15));
                    TextOutA(hdc, 35, 255, "The Law of Equivalent Harmonics:", 31);
                    SelectObject(hdc, hUIFont); SetTextColor(hdc, RGB(226, 232, 240));
                    TextOutA(hdc, 35, 278, "No essence is lost during synthesis. When two reagents react in harmonic resonance,", 83);
                    TextOutA(hdc, 35, 296, "their molecular structures realign, yielding a higher compound and Arcane Dust.", 79);

                    SelectObject(hdc, hHeaderFont);
                    SetTextColor(hdc, RGB(241, 196, 15));
                    TextOutA(hdc, 35, 335, "Secrets of Apex Celestial Reagents:", 34);
                    SelectObject(hdc, hUIFont); SetTextColor(hdc, RGB(226, 232, 240));
                    TextOutA(hdc, 35, 358, "Tier 5 Reagents transcend earthly physical matter, fusing physical essence with cosmic soul", 92);
                    TextOutA(hdc, 35, 376, "energy to produce artifacts capable of reshaping reality.", 57);
                }

                SelectObject(hdc, oldP);
            }

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
    SetProcessDPIAware();

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = NULL;
    wc.lpszClassName = "KAlchemyClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassA(&wc);

    DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    RECT rect = { 0, 0, 800, 570 };
    AdjustWindowRect(&rect, dwStyle, FALSE);

    HWND hwnd = CreateWindowA("KAlchemyClass", "KAlchemy - Fantasy Crafting & Element Discovery",
                               dwStyle,
                               CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
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
