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

#define STATE_CHAR_CREATE   0
#define STATE_TOWN          1
#define STATE_SHOP          2
#define STATE_DUNGEON       3
#define STATE_COMBAT        4
#define STATE_GAME_OVER     5
#define STATE_CRAFTING      6
#define STATE_MERCENARY     7
#define STATE_QUEST_BOARD   8
#define STATE_TRAINING_HALL 9
#define STATE_BOSS_RUSH     10
#define STATE_INVENTORY     11
#define STATE_SAVE_LOAD     12
#define STATE_ACHIEVEMENTS   13
#define STATE_HELP          14
#define STATE_TAVERN        15
#define STATE_UTILS         16
#define STATE_REPLAYS       17
#define STATE_CONFIG        18
#define STATE_MAGIC_MENU    19
#define STATE_TOWN_PAGE2    20
#define STATE_FACTIONS      21
#define STATE_MOUNTS        22

static int g_KeyBinds[6] = {'1', '2', '3', '4', '5', '6'};
static char g_MatchReplays[50][128];
static int g_MatchReplaysCount = 0;
static int g_BindingAction = -1;

static int g_HelpTab = 0; // 0: How to Play, 1: Controls, 2: Bestiary, 3: Crafting
void RenderHelpTabLog();

#define MAX_INV_SLOTS 30

typedef struct {
    char name[32];
    int type;     // 0: Consumable, 1: Equipment, 2: Material
    int eqType;   // 0: None, 1: Weapon, 2: Armor
    int bonusStr;
    int bonusDef;
    int rarity;   // 0: Common, 1: Uncommon, 2: Rare, 3: Epic, 4: Legendary
    int count;
    int value;    // Gold value per item
    char desc[64];
} InvItem;

static const char* g_RarityNames[5] = {"Common", "Uncommon", "Rare", "Epic", "Legendary"};

typedef struct {
    char name[48];
    int hp, maxHp;
    int str, def;
    int xp, gold;
    int trophies;
} ArenaBossDef;

static const ArenaBossDef g_ArenaBosses[5] = {
    {"👑 Goblin King Prime (Boss Rush)", 150, 150, 24, 10, 250, 200, 2},
    {"☠️ Lich Lord Revenant (Boss Rush)", 200, 200, 30, 14, 400, 350, 3},
    {"🐲 Infernal Dragon (Boss Rush)", 280, 280, 38, 18, 700, 550, 4},
    {"⚡ Storm Titan Sovereign (Boss Rush)", 360, 360, 45, 22, 1000, 800, 5},
    {"🌌 Void Overlord Malakor (Boss Rush)", 480, 480, 54, 26, 1600, 1200, 8}
};

typedef struct {
    int id;
    int tier; // 1: Bronze, 2: Silver, 3: Gold
    int type; // 0: slay, 1: floor, 2: fetch
    char title[48];
    char desc[80];
    int req;
    int current;
    int rewardGold;
    int rewardXp;
    char chestName[32];
    char targetMat[16];
    int accepted;
    int done;
    int claimed;
} BountyContract;

static unsigned int rngSeed = 54321;
static int xrand() {
    rngSeed = rngSeed * 1103515245 + 12345;
    return (int)((rngSeed / 65536) % 32768);
}

static int ContainsSubstr(const char* str, const char* sub) {
    if (!str || !sub) return 0;
    int i, j;
    for (i = 0; str[i] != '\0'; i++) {
        for (j = 0; sub[j] != '\0' && str[i + j] == sub[j]; j++);
        if (sub[j] == '\0') return 1;
    }
    return 0;
}

typedef struct {
    char name[32];
    int hp, maxHp;
    int str, def;
    int xp, gold;
} MonsterDef;

typedef struct {
    char name[32];
    char hazardName[32];
    MonsterDef monsters[5];
    MonsterDef boss;
    int maxPhases;
} BiomeDef;

static const BiomeDef g_Biomes[17] = {
    {
        "Goblin Outpost", "Cave-In",
        {
            {"Cave Goblin", 30, 30, 8, 3, 30, 15},
            {"Goblin Slinger", 25, 25, 10, 2, 35, 18},
            {"Cave Spider", 38, 38, 11, 4, 45, 22},
            {"Mine Taskmaster", 55, 55, 15, 6, 70, 40},
            {"Rock Golem", 75, 75, 17, 10, 95, 55}
        },
        {"Goblin King Prime (Boss)", 140, 140, 22, 10, 280, 180},
        1
    },
    {
        "Skeleton Crypt", "Poison Fog",
        {
            {"Skeleton Archer", 32, 32, 11, 3, 38, 20},
            {"Tomb Ghoul", 45, 45, 13, 5, 52, 28},
            {"Crypt Necromancer", 50, 50, 16, 4, 75, 45},
            {"Dread Wraith", 65, 65, 18, 7, 100, 60},
            {"Bone Colossus", 85, 85, 20, 11, 120, 70}
        },
        {"Lich Lord Malakor (Boss)", 180, 180, 26, 12, 380, 240},
        2
    },
    {
        "Sunken Temple", "Tidal Miasma",
        {
            {"Drowned Zealot", 40, 40, 13, 4, 45, 24},
            {"Coral Naga", 52, 52, 15, 6, 60, 32},
            {"Deep Scuttler", 60, 60, 17, 8, 80, 42},
            {"Mire Siren", 72, 72, 19, 7, 105, 55},
            {"Tidal Elemental", 90, 90, 22, 12, 130, 75}
        },
        {"Abyssal Leviathan (Boss)", 220, 220, 30, 14, 480, 300},
        1
    },
    {
        "Dark Forest", "Toxic Spores",
        {
            {"Venom Stalker", 48, 48, 15, 5, 55, 28},
            {"Shadow Wolf", 58, 58, 17, 6, 72, 38},
            {"Brier Dryad", 68, 68, 19, 8, 92, 50},
            {"Moss Golem", 85, 85, 22, 13, 120, 65},
            {"Corrupted Ent", 105, 105, 25, 15, 150, 85}
        },
        {"Ancient Treant (Boss)", 260, 260, 34, 16, 580, 360},
        1
    },
    {
        "Orc Fortress", "Boiling Oil",
        {
            {"Orc Grunt", 55, 55, 17, 6, 68, 35},
            {"War Berserker", 70, 70, 20, 7, 88, 48},
            {"Iron Shaman", 78, 78, 22, 9, 110, 60},
            {"Siege Ogre", 98, 98, 25, 14, 145, 80},
            {"Thunder Beast", 120, 120, 28, 16, 180, 100}
        },
        {"Orc Warlord Grommash (Boss)", 300, 300, 38, 18, 700, 420},
        2
    },
    {
        "Haunted Mine", "Spectral Chill",
        {
            {"Tomb Banshee", 60, 60, 19, 6, 78, 40},
            {"Phantom Miner", 75, 75, 22, 8, 100, 52},
            {"Spectral Shade", 88, 88, 24, 10, 125, 68},
            {"Curse Spectre", 108, 108, 27, 12, 160, 88},
            {"Phantom Colossus", 130, 130, 30, 17, 200, 110}
        },
        {"Wraith Sovereign Nocturne (Boss)", 340, 340, 42, 20, 820, 500},
        1
    },
    {
        "Frostpeak Caverns", "Blizzard Frostbite",
        {
            {"Ice Yeti", 70, 70, 21, 8, 90, 48},
            {"Frost Wisp", 82, 82, 23, 7, 112, 58},
            {"Glacial Golem", 100, 100, 26, 14, 140, 75},
            {"Snow Drake", 120, 120, 29, 12, 180, 95},
            {"Frozen Stalker", 142, 142, 32, 18, 220, 120}
        },
        {"Frost Giant King Ymir (Boss)", 380, 380, 46, 22, 950, 580},
        1
    },
    {
        "Serpent Swamps", "Acidic Miasma",
        {
            {"Marsh Basilisk", 78, 78, 23, 9, 102, 55},
            {"Poison Viper", 90, 90, 25, 8, 125, 66},
            {"Swamp Troll", 112, 112, 28, 13, 160, 85},
            {"Acid Elemental", 132, 132, 31, 16, 200, 105},
            {"Venomous Hydra", 155, 155, 34, 19, 245, 135}
        },
        {"Hydra Matriarch Gorgon (Boss)", 420, 420, 50, 24, 1100, 660},
        2
    },
    {
        "Infernal Volcano", "Magma Eruption",
        {
            {"Lava Fiend", 85, 85, 25, 9, 115, 60},
            {"Fire Drake", 100, 100, 27, 11, 140, 75},
            {"Pyro Fiend", 120, 120, 30, 13, 178, 95},
            {"Molten Golem", 145, 145, 33, 18, 225, 120},
            {"Magma Charger", 168, 168, 36, 16, 270, 145}
        },
        {"Magma Archon Ignis (Boss)", 460, 460, 54, 26, 1250, 750},
        1
    },
    {
        "Dragon Cavern", "Dragon Flame",
        {
            {"Magma Imp", 95, 95, 27, 10, 130, 70},
            {"Fire Drake Prime", 115, 115, 30, 13, 160, 88},
            {"Obsidian Elemental", 138, 138, 33, 18, 200, 110},
            {"Wyvern Sentinel", 160, 160, 36, 16, 250, 135},
            {"Hellhound Alpha", 185, 185, 39, 20, 300, 160}
        },
        {"Wyrm Overlord Bahamut (Boss)", 520, 520, 60, 28, 1450, 850},
        2
    },
    {
        "Shadow Realm", "Void Decay",
        {
            {"Void Walker", 105, 105, 29, 11, 145, 78},
            {"Shadow Fiend", 128, 128, 32, 14, 180, 98},
            {"Nether Hound", 150, 150, 35, 16, 225, 122},
            {"Abyss Stalker", 175, 175, 38, 19, 280, 150},
            {"Dark Archon", 200, 200, 41, 22, 335, 180}
        },
        {"Shadow Monarch Erebus (Boss)", 580, 580, 65, 30, 1650, 980},
        1
    },
    {
        "Celestial Ruins", "Solar Flare",
        {
            {"Radiant Sentry", 118, 118, 31, 12, 160, 85},
            {"Solar Specter", 140, 140, 34, 15, 200, 110},
            {"Heavenly Sentinel", 165, 165, 37, 18, 250, 135},
            {"Prism Golem", 192, 192, 40, 22, 310, 165},
            {"Archangel Remnant", 220, 220, 44, 25, 375, 200}
        },
        {"Fallen Seraph Gabriel (Boss)", 640, 640, 70, 32, 1900, 1100},
        1
    },
    {
        "Blood Citadel", "Vampiric Drain",
        {
            {"Blood Thrall", 130, 130, 33, 13, 178, 95},
            {"Crimson Bat", 155, 155, 36, 15, 220, 120},
            {"Flesh Golem", 182, 182, 40, 20, 280, 150},
            {"Vampire Knight", 210, 210, 43, 23, 340, 180},
            {"Blood Mage", 240, 240, 47, 26, 410, 220}
        },
        {"Blood Emperor Dracula (Boss)", 700, 700, 75, 34, 2200, 1250},
        2
    },
    {
        "Abyssal Trench", "Crushing Pressure",
        {
            {"Trench Kraken", 145, 145, 36, 15, 200, 110},
            {"Abyss Crawler", 172, 172, 39, 17, 250, 135},
            {"Depth Fiend", 200, 200, 43, 21, 310, 165},
            {"Void Leviathan", 232, 232, 46, 24, 380, 200},
            {"Dark Siren", 265, 265, 50, 28, 450, 240}
        },
        {"Abyssal Titan Oceanus (Boss)", 780, 780, 80, 36, 2500, 1400},
        2
    },
    {
        "Void Citadel", "Dimensional Collapse",
        {
            {"Void Fiend", 160, 160, 39, 16, 230, 125},
            {"Dimensional Anomaly", 190, 190, 43, 19, 290, 155},
            {"Cosmos Weaver", 222, 222, 47, 22, 360, 190},
            {"Oblivion Sentinel", 258, 258, 51, 26, 440, 230},
            {"Void Behemoth", 300, 300, 56, 30, 530, 280}
        },
        {"Void Overlord Malakor (Boss)", 900, 900, 90, 40, 3000, 1800},
        3
    },
    {
        "Crystal Caverns", "Crystal Shards",
        {
            {"Crystal Golem", 180, 180, 42, 20, 260, 140},
            {"Gem Sprite", 210, 210, 46, 22, 320, 170},
            {"Quartz Weaver", 250, 250, 50, 25, 400, 210},
            {"Diamond Sentinel", 300, 300, 55, 29, 490, 260},
            {"Prismatic Behemoth", 350, 350, 60, 34, 590, 310}
        },
        {"Crystal Overlord (Boss)", 1000, 1000, 100, 45, 3500, 2000},
        1
    },
    {
        "Ruined Castle", "Falling Debris",
        {
            {"Spectral Knight", 200, 200, 45, 23, 290, 160},
            {"Cursed Mage", 230, 230, 50, 25, 360, 190},
            {"Gargoyle", 280, 280, 54, 28, 450, 240},
            {"Castle Guard", 330, 330, 59, 32, 550, 290},
            {"Fallen King", 380, 380, 65, 37, 650, 340}
        },
        {"King's Wraith (Boss)", 1100, 1100, 110, 50, 4000, 2500},
        2
    }
};

typedef struct {
    int active; // 0: none, 1: Paladin, 2: Archmage, 3: Cleric
    char name[32];
    char role[32];
    int level;
    int hp, maxHp;
    int upkeep;
    int cost;
    int isDown;
} Companion;

typedef struct {
    char name[32];
    char heroClass[16];
    int level;
    int hp, maxHp;
    int mp, maxMp;
    int str, intStat, def, agi;
    int gold;
    int xp, nextXp;
    int skillPoints;
    int offensePoints;
    int defensePoints;
    int utilityPoints;
    int ironWillTurns;
    int manaSurgeActive;
    int floor;
    int biome;
    int arenaWave;
    int arenaBestWave;
    int arenaActive;
    int arenaTokens;
    char weaponName[32];
    int weaponBonusStr;
    char weaponPrefix[16]; // "", "Flaming", "Vampiric", "Thunderous"
    char armorName[32];
    int armorBonusDef;
    char armorPrefix[16];  // "", "Fortified", "Warded", "Spiked"
    int hpPotions;
    int mpPotions;
    int greaterHpPotions;
    int powerElixirs;
    int fireBombs;
    int phoenixElixirs;
    int holyShieldTurns;
    int berserkTurns;
    int lightningDazeTurns;
    int ironScrap;
    int arcaneDust;
    int elementalCore;
    int questMonstersKilled;
    int questMonstersDone;
    int questBossKilled;
    int questBossDone;
    int maxInvSlots;
    int invCount;
    InvItem inventory[MAX_INV_SLOTS];
    int invFilter; // 0: All, 1: Consumables, 2: Equipment, 3: Materials
    int invSort;   // 0: Rarity, 1: Name, 2: Value
    int selectedInvIdx;
    Companion companion;
    int faction;
    int mount;
    int poisonedTurns;
    int burningTurns;
    int frozenTurns;
    int ngLevel;
} Hero;

typedef struct {
    char name[64];
    int hp, maxHp;
    int str, def;
    int xp, gold;
    int phase;
    int maxPhases;
    int poisonedTurns;
    int burningTurns;
    int frozenTurns;
} Enemy;

static Hero player;
static Enemy currentEnemy;
static int gameState = STATE_CHAR_CREATE;
static int selectedClassIndex = 0; // 0: Warrior, 1: Mage, 2: Rogue

static int g_Achievements[10] = {0};
static const char* g_AchieveNames[10] = {
    "First Blood",
    "Dungeon Explorer",
    "Dragon Slayer",
    "Master Crafter",
    "Mercenary Leader",
    "Arena Champion",
    "Wealthy Hero",
    "Master of Skills",
    "NG+ Pioneer",
    "Save Master"
};
static const char* g_AchieveDescs[10] = {
    "Defeat your first dungeon monster",
    "Reach Floor 5 in the dungeon",
    "Defeat the Obsidian Dragon boss",
    "Craft or imbue items in Forge & Alchemy",
    "Hire a party companion from Mercenary Guild",
    "Reach Wave 5 in Boss Rush Arena",
    "Accumulate 500 Gold",
    "Allocate 5 Skill Points in Training Hall",
    "Enter New Game Plus mode",
    "Save your progress to a save file slot"
};
static const int g_AchieveRewards[10] = {50, 100, 250, 75, 75, 150, 100, 100, 200, 50};
static int g_SelectedSaveSlot = 0; // 0..3

static BountyContract g_BoardBounties[4];
static BountyContract g_ActiveBounties[3];
static int g_ActiveBountyCount = 0;

typedef struct {
    char magic[8]; // "KQUEST12"
    int ngLevel;
    int achievements[10];
    Hero hero;
    BountyContract activeBounties[3];
    int activeBountyCount;
    char timestamp[32];
} SaveSlotData;

void LogMessage(const char* msg);
void SetupButtons();
void UpdateUI();

static void my_memcpy(void* dest, const void* src, size_t n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (n--) *d++ = *s++;
}

static void my_memset(void* dest, int val, size_t n) {
    char* d = (char*)dest;
    while (n--) *d++ = (char)val;
}

// Phase 13: Native Win32 Sound Effects Engine
static int g_SoundEnabled = 1;

static int g_HeroActionTimer = 0;

void SfxCombatHit() {
    g_HeroActionTimer = 15;
    if (!g_SoundEnabled) return;
    Beep(160, 60);
    Beep(100, 70);
}

void SfxSpellCast() {
    if (!g_SoundEnabled) return;
    Beep(350, 60);
    Beep(550, 70);
    Beep(750, 80);
}

void SfxLevelUp() {
    if (!g_SoundEnabled) return;
    Beep(262, 80);  // C4
    Beep(330, 80);  // E4
    Beep(392, 80);  // G4
    Beep(523, 150); // C5
}

void SfxItemPickup() {
    if (!g_SoundEnabled) return;
    Beep(587, 50); // D5
    Beep(880, 80); // A5
}

void SfxDoorOpen() {
    if (!g_SoundEnabled) return;
    Beep(180, 70);
    Beep(320, 100);
}

void SfxDeath() {
    if (!g_SoundEnabled) return;
    Beep(300, 100);
    Beep(220, 120);
    Beep(160, 140);
    Beep(100, 200);
}

void UnlockAchievement(int id) {
    if (id < 0 || id >= 10) return;
    if (!g_Achievements[id]) {
        g_Achievements[id] = 1;
        player.gold += g_AchieveRewards[id];
        char msg[128];
        wsprintfA(msg, "🏆 ACHIEVEMENT UNLOCKED: \"%s\"! (+%d Gold)", g_AchieveNames[id], g_AchieveRewards[id]);
        LogMessage(msg);
        Beep(523, 80); Beep(659, 80); Beep(784, 120);
    }
}

void SaveToSlot(int slotIdx) {
    char fileName[64];
    wsprintfA(fileName, "kquest_slot%d.dat", slotIdx + 1);
    HANDLE hFile = CreateFileA(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        LogMessage("❌ Failed to create save file.");
        return;
    }
    SaveSlotData data;
    my_memset(&data, 0, sizeof(SaveSlotData));
    lstrcpyA(data.magic, "KQUEST12");
    data.ngLevel = player.ngLevel;
    my_memcpy(data.achievements, g_Achievements, sizeof(g_Achievements));
    my_memcpy(&data.hero, &player, sizeof(Hero));
    my_memcpy(data.activeBounties, g_ActiveBounties, sizeof(g_ActiveBounties));
    data.activeBountyCount = g_ActiveBountyCount;

    SYSTEMTIME st;
    GetLocalTime(&st);
    wsprintfA(data.timestamp, "%04d-%02d-%02d %02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);

    DWORD bytesWritten = 0;
    WriteFile(hFile, &data, sizeof(SaveSlotData), &bytesWritten, NULL);
    CloseHandle(hFile);

    UnlockAchievement(9); // Save Master
    char msg[128];
    wsprintfA(msg, "💾 Game saved to Slot %d successfully!", slotIdx + 1);
    LogMessage(msg);
}

void LoadFromSlot(int slotIdx) {
    char fileName[64];
    wsprintfA(fileName, "kquest_slot%d.dat", slotIdx + 1);
    HANDLE hFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        char msg[128];
        wsprintfA(msg, "❌ Save Slot %d is empty!", slotIdx + 1);
        LogMessage(msg);
        return;
    }
    SaveSlotData data;
    my_memset(&data, 0, sizeof(SaveSlotData));
    DWORD bytesRead = 0;
    ReadFile(hFile, &data, sizeof(SaveSlotData), &bytesRead, NULL);
    CloseHandle(hFile);

    if (bytesRead == sizeof(SaveSlotData) && lstrcmpA(data.magic, "KQUEST12") == 0) {
        player = data.hero;
        player.ngLevel = data.ngLevel;
        my_memcpy(g_Achievements, data.achievements, sizeof(g_Achievements));
        my_memcpy(g_ActiveBounties, data.activeBounties, sizeof(g_ActiveBounties));
        g_ActiveBountyCount = data.activeBountyCount;

        char msg[128];
        wsprintfA(msg, "📂 Loaded Save Slot %d! Welcome back, %s!", slotIdx + 1, player.name);
        LogMessage(msg);
        gameState = STATE_TOWN;
        SetupButtons();
        UpdateUI();
    } else {
        LogMessage("❌ Invalid save file data.");
    }
}

void DeleteSlot(int slotIdx) {
    char fileName[64];
    wsprintfA(fileName, "kquest_slot%d.dat", slotIdx + 1);
    DeleteFileA(fileName);
    char msg[128];
    wsprintfA(msg, "🗑️ Cleared Save Slot %d.", slotIdx + 1);
    LogMessage(msg);
}

void GetSlotSummary(int slotIdx, char* outBuf, int maxLen) {
    char fileName[64];
    wsprintfA(fileName, "kquest_slot%d.dat", slotIdx + 1);
    HANDLE hFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        wsprintfA(outBuf, "[Slot %d]: Empty Slot", slotIdx + 1);
        return;
    }
    SaveSlotData data;
    my_memset(&data, 0, sizeof(SaveSlotData));
    DWORD bytesRead = 0;
    ReadFile(hFile, &data, sizeof(SaveSlotData), &bytesRead, NULL);
    CloseHandle(hFile);

    if (bytesRead == sizeof(SaveSlotData) && lstrcmpA(data.magic, "KQUEST12") == 0) {
        char ng[16] = "";
        if (data.ngLevel > 0) wsprintfA(ng, " NG+%d", data.ngLevel);
        wsprintfA(outBuf, "[Slot %d]: %s (Lvl %d %s, Fl %d, %dG%s) Saved: %s",
            slotIdx + 1, data.hero.name, data.hero.level, data.hero.heroClass,
            data.hero.floor, data.hero.gold, ng, data.timestamp);
    } else {
        wsprintfA(outBuf, "[Slot %d]: Corrupted Data", slotIdx + 1);
    }
}

void EnterNewGamePlus() {
    player.ngLevel++;
    UnlockAchievement(8); // NG+ Pioneer
    player.floor = 1;
    player.biome = 0;
    player.hp = player.maxHp;
    player.mp = player.maxMp;
    player.questMonstersKilled = 0; player.questMonstersDone = 0;
    player.questBossKilled = 0; player.questBossDone = 0;
    g_ActiveBountyCount = 0;
    
    char msg[128];
    wsprintfA(msg, "✨ ENTERED NEW GAME+ (Cycle NG+%d)! Enemy stats scaled by +%d%%! Rewards increased!", player.ngLevel, player.ngLevel * 50);
    LogMessage(msg);
    Beep(523, 100); Beep(659, 100); Beep(784, 150); Beep(1046, 250);
    gameState = STATE_TOWN;
    SetupButtons();
    UpdateUI();
}

HWND hStatusText;
HWND hInfoText;
HWND hLogEdit;
HWND hBtn1, hBtn2, hBtn3, hBtn4, hBtn5, hBtn6;
static HFONT hUIFont = NULL;

HBRUSH hBgBrush = NULL;
HBRUSH hPanelBrush = NULL;

void GenerateBounties() {
    g_BoardBounties[0].id = 1;
    g_BoardBounties[0].tier = 1;
    g_BoardBounties[0].type = 0; // slay
    lstrcpyA(g_BoardBounties[0].title, "Slay 5 Dungeon Beasts");
    lstrcpyA(g_BoardBounties[0].desc, "Defeat 5 monsters in dungeon chambers.");
    g_BoardBounties[0].req = 5;
    g_BoardBounties[0].current = 0;
    g_BoardBounties[0].rewardGold = 60;
    g_BoardBounties[0].rewardXp = 50;
    lstrcpyA(g_BoardBounties[0].chestName, "Bronze Rune Chest");
    g_BoardBounties[0].accepted = 0; g_BoardBounties[0].done = 0; g_BoardBounties[0].claimed = 0;

    g_BoardBounties[1].id = 2;
    g_BoardBounties[1].tier = 2;
    g_BoardBounties[1].type = 1; // floor
    lstrcpyA(g_BoardBounties[1].title, "Reach Floor 4 Depth");
    lstrcpyA(g_BoardBounties[1].desc, "Descend to dungeon floor 4 or higher.");
    g_BoardBounties[1].req = 4;
    g_BoardBounties[1].current = 0;
    g_BoardBounties[1].rewardGold = 140;
    g_BoardBounties[1].rewardXp = 100;
    lstrcpyA(g_BoardBounties[1].chestName, "Silver Rune Chest");
    g_BoardBounties[1].accepted = 0; g_BoardBounties[1].done = 0; g_BoardBounties[1].claimed = 0;

    g_BoardBounties[2].id = 3;
    g_BoardBounties[2].tier = 2;
    g_BoardBounties[2].type = 2; // fetch
    lstrcpyA(g_BoardBounties[2].title, "Gather 4 Iron Scrap");
    lstrcpyA(g_BoardBounties[2].desc, "Collect 4 Iron Scrap crafting materials.");
    lstrcpyA(g_BoardBounties[2].targetMat, "iron");
    g_BoardBounties[2].req = 4;
    g_BoardBounties[2].current = 0;
    g_BoardBounties[2].rewardGold = 120;
    g_BoardBounties[2].rewardXp = 90;
    lstrcpyA(g_BoardBounties[2].chestName, "Silver Rune Chest");
    g_BoardBounties[2].accepted = 0; g_BoardBounties[2].done = 0; g_BoardBounties[2].claimed = 0;

    g_BoardBounties[3].id = 4;
    g_BoardBounties[3].tier = 3;
    g_BoardBounties[3].type = 0; // slay
    lstrcpyA(g_BoardBounties[3].title, "Elite Bounty: Slay 10 Foes");
    lstrcpyA(g_BoardBounties[3].desc, "Purge 10 monsters from any dungeon biome.");
    g_BoardBounties[3].req = 10;
    g_BoardBounties[3].current = 0;
    g_BoardBounties[3].rewardGold = 300;
    g_BoardBounties[3].rewardXp = 240;
    lstrcpyA(g_BoardBounties[3].chestName, "Gold Rune Chest");
    g_BoardBounties[3].accepted = 0; g_BoardBounties[3].done = 0; g_BoardBounties[3].claimed = 0;
}

void AcceptBounty(int boardIdx) {
    if (boardIdx < 0 || boardIdx >= 4) return;
    if (g_ActiveBountyCount >= 3) {
        if (hLogEdit) {
            int len = GetWindowTextLength(hLogEdit);
            SendMessage(hLogEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
            SendMessage(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)"⚠️ Max 3 active bounties allowed!\r\n");
        }
        return;
    }
    if (g_BoardBounties[boardIdx].accepted) {
        if (hLogEdit) {
            int len = GetWindowTextLength(hLogEdit);
            SendMessage(hLogEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
            SendMessage(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)"Bounty already accepted!\r\n");
        }
        return;
    }
    g_BoardBounties[boardIdx].accepted = 1;
    g_ActiveBounties[g_ActiveBountyCount] = g_BoardBounties[boardIdx];
    
    if (g_ActiveBounties[g_ActiveBountyCount].type == 2) {
        if (lstrcmpA(g_ActiveBounties[g_ActiveBountyCount].targetMat, "iron") == 0) {
            g_ActiveBounties[g_ActiveBountyCount].current = player.ironScrap;
        }
        if (g_ActiveBounties[g_ActiveBountyCount].current >= g_ActiveBounties[g_ActiveBountyCount].req) {
            g_ActiveBounties[g_ActiveBountyCount].done = 1;
        }
    } else if (g_ActiveBounties[g_ActiveBountyCount].type == 1) {
        g_ActiveBounties[g_ActiveBountyCount].current = player.floor;
        if (g_ActiveBounties[g_ActiveBountyCount].current >= g_ActiveBounties[g_ActiveBountyCount].req) {
            g_ActiveBounties[g_ActiveBountyCount].done = 1;
        }
    }

    g_ActiveBountyCount++;
    if (hLogEdit) {
        char msg[128];
        wsprintfA(msg, "📜 Accepted Bounty Contract: %s!\r\n", g_BoardBounties[boardIdx].title);
        int len = GetWindowTextLength(hLogEdit);
        SendMessage(hLogEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
        SendMessage(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)msg);
    }
}

void ClaimBounty(int activeIdx) {
    if (activeIdx < 0 || activeIdx >= g_ActiveBountyCount) return;
    BountyContract* b = &g_ActiveBounties[activeIdx];
    if (!b->done) return;

    player.gold += b->rewardGold;
    player.xp += b->rewardXp;
    if (hLogEdit) {
        char msg[128];
        wsprintfA(msg, "🎉 Claimed Bounty: %s! +%d Gold, +%d XP!\r\n", b->title, b->rewardGold, b->rewardXp);
        int len = GetWindowTextLength(hLogEdit);
        SendMessage(hLogEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
        SendMessage(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)msg);
    }

    if (b->tier == 1) {
        player.gold += 25;
        player.ironScrap += 1;
        player.hp += 15;
        if (player.hp > player.maxHp) player.hp = player.maxHp;
        if (hLogEdit) {
            int len = GetWindowTextLength(hLogEdit);
            SendMessage(hLogEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
            SendMessage(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)"🧰 OPENED BRONZE RUNE CHEST! +25 Gold, +1 Iron Scrap, +15 HP!\r\n");
        }
    } else if (b->tier == 2) {
        player.gold += 60;
        player.arcaneDust += 2;
        player.hpPotions += 1;
        if (hLogEdit) {
            int len = GetWindowTextLength(hLogEdit);
            SendMessage(hLogEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
            SendMessage(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)"🛡️ OPENED SILVER RUNE CHEST! +60 Gold, +2 Arcane Dust, +1 HP Potion!\r\n");
        }
    } else {
        player.gold += 150;
        player.elementalCore += 1;
        player.str += 2;
        player.def += 2;
        if (hLogEdit) {
            int len = GetWindowTextLength(hLogEdit);
            SendMessage(hLogEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
            SendMessage(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)"👑 OPENED GOLD RUNE CHEST! +150 Gold, +1 Core, +2 STR/DEF!\r\n");
        }
    }

    for (int i = activeIdx; i < g_ActiveBountyCount - 1; i++) {
        g_ActiveBounties[i] = g_ActiveBounties[i + 1];
    }
    g_ActiveBountyCount--;
}

void ClaimAllCompletedBounties() {
    int claimed = 0;
    for (int i = g_ActiveBountyCount - 1; i >= 0; i--) {
        if (g_ActiveBounties[i].done) {
            ClaimBounty(i);
            claimed++;
        }
    }
    if (claimed == 0 && hLogEdit) {
        int len = GetWindowTextLength(hLogEdit);
        SendMessage(hLogEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
        SendMessage(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)"No completed bounties ready to claim right now.\r\n");
    }
}

void LogMessage(const char* msg) {
    if (!hLogEdit) return;
    if (ContainsSubstr(msg, "attacks") || ContainsSubstr(msg, "casts") || ContainsSubstr(msg, "takes") || ContainsSubstr(msg, "heals") || ContainsSubstr(msg, "Damage")) {
        if (g_MatchReplaysCount < 50) {
            lstrcpyA(g_MatchReplays[g_MatchReplaysCount++], msg);
        } else {
            for(int i=0; i<49; i++) lstrcpyA(g_MatchReplays[i], g_MatchReplays[i+1]);
            lstrcpyA(g_MatchReplays[49], msg);
        }
    }
    int len = GetWindowTextLength(hLogEdit);
    SendMessage(hLogEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessage(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)msg);
    SendMessage(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
}

int AddInvItem(const char* name, int type, int eqType, int bonusStr, int bonusDef, int rarity, int count, int value, const char* desc) {
    if (player.maxInvSlots <= 0) player.maxInvSlots = 10;
    
    if (type != 1) {
        for (int i = 0; i < player.invCount; i++) {
            if (lstrcmpA(player.inventory[i].name, name) == 0 && player.inventory[i].type == type) {
                player.inventory[i].count += count;
                return 1;
            }
        }
    }

    if (player.invCount >= player.maxInvSlots) {
        if (hLogEdit) {
            char msg[128];
            wsprintfA(msg, "🎒 BACKPACK FULL (%d/%d Slots)! Cannot pick up %s!\r\n", player.invCount, player.maxInvSlots, name);
            int len = GetWindowTextLength(hLogEdit);
            SendMessage(hLogEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
            SendMessage(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)msg);
        }
        return 0;
    }

    InvItem* it = &player.inventory[player.invCount];
    lstrcpyA(it->name, name);
    it->type = type;
    it->eqType = eqType;
    it->bonusStr = bonusStr;
    it->bonusDef = bonusDef;
    it->rarity = rarity;
    it->count = count;
    it->value = value;
    lstrcpyA(it->desc, desc);

    player.invCount++;
    SfxItemPickup();
    return 1;
}

void RemoveInvItem(int index) {
    if (index < 0 || index >= player.invCount) return;
    for (int i = index; i < player.invCount - 1; i++) {
        player.inventory[i] = player.inventory[i + 1];
    }
    player.invCount--;
    if (player.selectedInvIdx >= player.invCount && player.invCount > 0) {
        player.selectedInvIdx = player.invCount - 1;
    }
}

void QuickSellCommons() {
    int soldGold = 0;
    int soldCount = 0;
    for (int i = player.invCount - 1; i >= 0; i--) {
        if (player.inventory[i].rarity == 0) {
            int val = player.inventory[i].value * player.inventory[i].count;
            soldGold += val;
            soldCount += player.inventory[i].count;
            RemoveInvItem(i);
        }
    }
    if (soldCount == 0) {
        LogMessage("No Common items found in inventory to quick-sell.");
        return;
    }
    player.gold += soldGold;
    char msg[128];
    wsprintfA(msg, "💰 QUICK-SOLD %d Common item(s) for +%d Gold!", soldCount, soldGold);
    LogMessage(msg);
}

void ExpandBackpackSlots() {
    if (player.maxInvSlots >= 30) {
        LogMessage("🎒 Backpack is already at maximum capacity (30 slots)!");
        return;
    }
    int cost = (player.maxInvSlots / 10) * 50;
    if (player.gold < cost) {
        char msg[128];
        wsprintfA(msg, "Need %d Gold to expand backpack slots!", cost);
        LogMessage(msg);
        return;
    }
    player.gold -= cost;
    player.maxInvSlots += 5;
    char msg[128];
    wsprintfA(msg, "🎒 EXPANDED BACKPACK! Capacity increased to %d Slots!", player.maxInvSlots);
    LogMessage(msg);
}

void SortInventory() {
    for (int i = 0; i < player.invCount - 1; i++) {
        for (int j = i + 1; j < player.invCount; j++) {
            int swapNeeded = 0;
            if (player.invSort == 0) { // Rarity desc
                if (player.inventory[j].rarity > player.inventory[i].rarity) swapNeeded = 1;
            } else if (player.invSort == 1) { // Name asc
                if (lstrcmpA(player.inventory[i].name, player.inventory[j].name) > 0) swapNeeded = 1;
            } else if (player.invSort == 2) { // Value desc
                if (player.inventory[j].value > player.inventory[i].value) swapNeeded = 1;
            }
            if (swapNeeded) {
                InvItem tmp = player.inventory[i];
                player.inventory[i] = player.inventory[j];
                player.inventory[j] = tmp;
            }
        }
    }
}

void InitHero(int classIdx) {
    lstrcpyA(player.name, "Valerius");
    player.level = 1;
    player.xp = 0;
    player.nextXp = 100;
    player.gold = 50;
    player.skillPoints = 0;
    player.offensePoints = 0;
    player.defensePoints = 0;
    player.utilityPoints = 0;
    player.ironWillTurns = 0;
    player.manaSurgeActive = 0;
    player.floor = 1;
    player.biome = 0;
    player.arenaWave = 1;
    player.arenaBestWave = 0;
    player.arenaActive = 0;
    player.arenaTokens = 0;
    player.hpPotions = 3;
    player.mpPotions = 2;
    player.greaterHpPotions = 0;
    player.powerElixirs = 0;
    player.fireBombs = 0;
    player.phoenixElixirs = 1;
    player.holyShieldTurns = 0;
    player.berserkTurns = 0;
    player.lightningDazeTurns = 0;
    player.ironScrap = 2;
    player.arcaneDust = 2;
    player.elementalCore = 1;
    player.weaponPrefix[0] = '\0';
    player.armorPrefix[0] = '\0';
    player.questMonstersKilled = 0;
    player.questMonstersDone = 0;
    player.questBossKilled = 0;
    player.companion.active = 0;
    player.companion.isDown = 0;
    player.maxInvSlots = 10;
    player.invCount = 0;
    player.invFilter = 0;
    player.invSort = 0;
    player.selectedInvIdx = 0;

    AddInvItem("Health Potion", 0, 0, 0, 0, 0, 3, 8, "Restores 35 HP");
    AddInvItem("Mana Potion", 0, 0, 0, 0, 0, 2, 8, "Restores 25 MP");
    AddInvItem("Phoenix Elixir", 0, 0, 0, 0, 3, 1, 35, "Restores 100% HP & MP or Auto-Revives");
    AddInvItem("Iron Scrap", 2, 0, 0, 0, 0, 2, 5, "Crafting material");

    if (classIdx == 0) { // Warrior
        lstrcpyA(player.heroClass, "Warrior");
        player.maxHp = 60; player.hp = 60;
        player.maxMp = 15; player.mp = 15;
        player.str = 15; player.intStat = 6; player.def = 12; player.agi = 8;
        lstrcpyA(player.weaponName, "Iron Shortsword"); player.weaponBonusStr = 4;
        lstrcpyA(player.armorName, "Chainmail Armor"); player.armorBonusDef = 5;
    } else if (classIdx == 1) { // Mage
        lstrcpyA(player.heroClass, "Mage");
        player.maxHp = 40; player.hp = 40;
        player.maxMp = 45; player.mp = 45;
        player.str = 7; player.intStat = 18; player.def = 6; player.agi = 10;
        lstrcpyA(player.weaponName, "Apprentice Staff"); player.weaponBonusStr = 2;
        lstrcpyA(player.armorName, "Silk Robes"); player.armorBonusDef = 2;
    } else if (classIdx == 2) { // Rogue
        lstrcpyA(player.heroClass, "Rogue");
        player.maxHp = 45; player.hp = 45;
        player.maxMp = 25; player.mp = 25;
        player.str = 11; player.intStat = 8; player.def = 8; player.agi = 16;
        lstrcpyA(player.weaponName, "Twin Daggers"); player.weaponBonusStr = 5;
        lstrcpyA(player.armorName, "Leather Vest"); player.armorBonusDef = 3;
    } else if (classIdx == 3) { // Paladin
        lstrcpyA(player.heroClass, "Paladin");
        player.maxHp = 75; player.hp = 75;
        player.maxMp = 20; player.mp = 20;
        player.str = 14; player.intStat = 10; player.def = 16; player.agi = 6;
        lstrcpyA(player.weaponName, "Heavy Mace"); player.weaponBonusStr = 4;
        lstrcpyA(player.armorName, "Knight Plate"); player.armorBonusDef = 6;
    } else if (classIdx == 4) { // Ranger
        lstrcpyA(player.heroClass, "Ranger");
        player.maxHp = 50; player.hp = 50;
        player.maxMp = 25; player.mp = 25;
        player.str = 13; player.intStat = 7; player.def = 7; player.agi = 18;
        lstrcpyA(player.weaponName, "Longbow"); player.weaponBonusStr = 5;
        lstrcpyA(player.armorName, "Hunter Cloak"); player.armorBonusDef = 3;
    }
}

void RenderHelpTabLog() {
    if (!hLogEdit) return;
    if (g_HelpTab == 0) {
        SetWindowTextA(hLogEdit,
            "=========================================================================\r\n"
            "                      🎮 KQUEST - HOW TO PLAY OVERVIEW                   \r\n"
            "=========================================================================\r\n\r\n"
            "1. HERO CLASSES & ATTRIBUTES:\r\n"
            "   - Warrior : Starts with 60 HP, 15 MP, 14 STR, 12 DEF. Shield Bash ability.\r\n"
            "   - Mage    : Starts with 40 HP, 35 MP, 10 INT, 8 DEF. Fireball spell & Mana Surge.\r\n"
            "   - Rogue   : Starts with 45 HP, 20 MP, 14 AGI, 9 DEF. High Crit & Shadow Strike.\r\n"
            "   - Attributes: STR increases physical dmg, INT boosts spell dmg & max MP,\r\n"
            "                 DEF reduces incoming dmg, AGI increases crit chance.\r\n\r\n"
            "2. TOWN FACILITIES:\r\n"
            "   - Dungeon Entrance : Explore 3 distinct Biomes (Mines, Catacombs, Spire).\r\n"
            "   - Boss Rush Arena  : Battle 5 consecutive boss waves for Trophies & Gear.\r\n"
            "   - Inventory Hub    : Filter/sort items, compare stats, quick-sell, expand slots.\r\n"
            "   - Quest & Training : Accept daily contracts & spend Skill Points (Off/Def/Util).\r\n"
            "   - Forge & Alchemy  : Salvage spare gear, brew potions, & enchant weapons/armor.\r\n"
            "   - Mercenary Guild  : Hire Paladin Tank, Archmage DPS, or Cleric Healer companions.\r\n"
            "   - Save/Load & NG+  : Save up to 4 slots. Complete floor 15 or Arena to start NG+.\r\n\r\n"
            "3. COMBAT & HAZARD MECHANICS:\r\n"
            "   - Combat Turn : Attack with weapon, cast Class Spells, use Items, or Flee.\r\n"
            "   - Companions  : Companions automatically attack, heal, or absorb damage each turn.\r\n"
            "   - Hazards     : Cave-In (Mines), Poison Fog (Catacombs), & Lava Burst (Spire)\r\n"
            "                   deal environmental tick damage during exploration & battles.\r\n");
    } else if (g_HelpTab == 1) {
        SetWindowTextA(hLogEdit,
            "=========================================================================\r\n"
            "                    ⌨️ KQUEST - CONTROLS & SHORTCUTS REFERENCE           \r\n"
            "=========================================================================\r\n\r\n"
            "KEYBOARD SHORTCUTS:\r\n"
            "   - [1] - [6] : Select action buttons 1 through 6 in all screens.\r\n"
            "   - [F1] / [H]: Toggle Comprehensive Help & Lore Codex screen anytime.\r\n"
            "   - [S]       : Quick Save Game to Slot 1.\r\n"
            "   - [L]       : Open Save / Load Manager Screen.\r\n"
            "   - [I]       : Open Inventory Hub Screen.\r\n"
            "   - [Esc]     : Exit Help Overlay or return to Oakhaven Town Square.\r\n\r\n"
            "MOUSE NAVIGATION:\r\n"
            "   - Click any Action Button at bottom of window to select options.\r\n"
            "   - Inventory: Select slots to inspect item details, values, & stat comparisons.\r\n"
            "   - Quest Board: Click contract buttons to accept daily bounties.\r\n");
    } else if (g_HelpTab == 2) {
        SetWindowTextA(hLogEdit,
            "=========================================================================\r\n"
            "                   🐉 KQUEST - BESTIARY & WORLD LORE CODEX                \r\n"
            "=========================================================================\r\n\r\n"
            "MONSTERS & BOSSES:\r\n"
            " • ABANDONED MINES (Floors 1-5):\r\n"
            "   - Cave Goblin        : HP 30  | STR 8   | DEF 3  | Dropped: Scrap, Gold\r\n"
            "   - Goblin Slinger     : HP 25  | STR 10  | DEF 2  | Ranged stone thrower\r\n"
            "   - Cave Spider        : HP 38  | STR 11  | DEF 4  | Venomous attack\r\n"
            "   - Mine Taskmaster    : HP 55  | STR 15  | DEF 6  | Iron whip wielder\r\n"
            "   - Rock Golem         : HP 75  | STR 17  | DEF 10 | Animated stone monolith\r\n"
            "   - 👑 Goblin King     : HP 130 | STR 22  | DEF 10 | Mine Biome Boss\r\n\r\n"
            " • ANCIENT CATACOMBS (Floors 6-10):\r\n"
            "   - Skeleton Archer    : HP 32  | STR 11  | DEF 3  | Undead marksman\r\n"
            "   - Tomb Ghoul         : HP 45  | STR 13  | DEF 5  | Vault corpse eater\r\n"
            "   - Crypt Necromancer  : HP 50  | STR 16  | DEF 4  | Dark magic caster\r\n"
            "   - Dread Wraith       : HP 65  | STR 18  | DEF 7  | Life-draining phantom\r\n"
            "   - Bone Colossus      : HP 85  | STR 20  | DEF 11 | Skeletal giant\r\n"
            "   - ☠️ Lich Lord       : HP 160 | STR 26  | DEF 12 | Catacombs Biome Boss\r\n\r\n"
            " • DRAGON SPIRE (Floors 11-15):\r\n"
            "   - Magma Imp          : HP 40  | STR 14  | DEF 4  | Volcanic fiend\r\n"
            "   - Fire Drake         : HP 60  | STR 17  | DEF 7  | Dragon-kin predator\r\n"
            "   - Obsidian Elemental : HP 80  | STR 20  | DEF 12 | Molten glass titan\r\n"
            "   - Wyvern Sentinel    : HP 95  | STR 23  | DEF 10 | Apex spire guardian\r\n"
            "   - Hellhound          : HP 70  | STR 21  | DEF 8  | Demonic dual-head hound\r\n"
            "   - 🐲 Obsidian Dragon  : HP 250 | STR 32  | DEF 18 | Final Spire Boss\r\n\r\n"
            "WORLD LORE:\r\n"
            "   The Obsidian Spire is an ancient black tower towering over Oakhaven.\r\n"
            "   For centuries, dark elemental energies have brewed within its chambers.\r\n"
            "   Only brave champions who master martial prowess, magic, & crafting\r\n"
            "   can conquer the Spire and save Oakhaven from eternal shadow.\r\n");
    } else if (g_HelpTab == 3) {
        SetWindowTextA(hLogEdit,
            "=========================================================================\r\n"
            "                ⚒️ KQUEST - CRAFTING & RECIPE REFERENCE                 \r\n"
            "=========================================================================\r\n\r\n"
            "ALCHEMY BENCH (Consumables):\r\n"
            "   - 💣 Fire Bomb        : Req 1x Iron Scrap, 1x Core | Deals 45 Fire Damage in battle.\r\n"
            "   - 🧪 Greater HP Elixir: Req 2x Arcane Dust, 1x Iron | Restores 70 HP instantly.\r\n"
            "   - ⚡ Elixir of Might  : Req 2x Arcane Dust, 1x Core | Restores 40 MP & +3 STR.\r\n\r\n"
            "ENCHANTER'S FORGE (Weapon Imbuing):\r\n"
            "   - 🔥 Flaming Weapon   : Req 2x Core, 1x Dust | +6 Fire Damage per strike.\r\n"
            "   - 🩸 Vampiric Weapon  : Req 2x Dust, 1x Iron | Siphons 25% attack damage as HP.\r\n"
            "   - ⚡ Thunderous Weapon: Req 2x Core, 1x Iron | +6 STR & 30% Lightning chance.\r\n\r\n"
            "ENCHANTER'S FORGE (Armor Imbuing):\r\n"
            "   - 🛡️ Fortified Armor  : Req 2x Iron, 1x Dust | +5 DEF & +20 Maximum HP.\r\n"
            "   - 🔮 Warded Armor     : Req 2x Dust, 1x Core | +4 DEF & +15 Maximum MP.\r\n"
            "   - 🌵 Spiked Armor     : Req 2x Iron, 1x Core | Reflects 35% damage to attacker.\r\n\r\n"
            "MATERIAL SOURCES & SALVAGE:\r\n"
            "   - ⚙️ Iron Scrap   : Dropped by dungeon beasts, chests, or salvaging loot.\r\n"
            "   - ✨ Arcane Dust  : Dropped by spellcasters, specters, or praying at altars.\r\n"
            "   - 🔥 Elemental Core: Harvested from Spire drakes, elementals, & bosses.\r\n"
            "   - ♻️ Salvage Hub  : Pay 20 Gold at Forge to gain +2 Iron Scrap & +1 Arcane Dust.\r\n");
    }
}

void UpdateUI() {
    char statusBuf[512];
    char infoBuf[1024];

    const char* locStr = "Oakhaven Town";
    if (gameState == STATE_BOSS_RUSH) {
        locStr = "Boss Rush Arena";
    } else if (gameState == STATE_DUNGEON || gameState == STATE_COMBAT) {
        locStr = player.arenaActive ? "Boss Rush Arena" : g_Biomes[player.biome].name;
    } else if (gameState == STATE_INVENTORY) {
        locStr = "Inventory Hub";
    } else if (gameState == STATE_SAVE_LOAD) {
        locStr = "Save/Load Manager";
    } else if (gameState == STATE_FACTIONS || gameState == STATE_MOUNTS) {
        gameState = STATE_TOWN_PAGE2;
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_ACHIEVEMENTS) {
        locStr = "Achievements Hub";
    } else if (gameState == STATE_TAVERN) { LogMessage("Barkeep: 'Welcome! The Ruined Castle is dangerous!'"); return; }
    if (gameState == STATE_TAVERN) { LogMessage("Rumor: 'Equipment can be upgraded at the forge now.'"); return; }
    if (gameState == STATE_TAVERN) {
        if (player.gold >= 5) {
            player.gold -= 5;
            player.hp += 15;
            if (player.hp > player.maxHp) player.hp = player.maxHp;
            LogMessage("🍺 Drank Ale. Recovered 15 HP.");
            UpdateUI();
        } else {
            LogMessage("Not enough gold for Ale.");
        }
        return;
    }
    if (gameState == STATE_TAVERN) {
        if (player.ironScrap >= 3) {
            player.ironScrap -= 3;
            player.gold += 60;
            LogMessage("Side Quest Complete! Traded 3 Scrap for 60 Gold.");
            UpdateUI();
        } else {
            LogMessage("Side Quest: 'Bring me 3 Iron Scrap, I'll pay 60 Gold.'");
        }
        return;
    }
    if (gameState == STATE_HELP) {
        locStr = "Help & Lore Codex";
    }

    char ngStr[32] = "";
    if (player.ngLevel > 0) wsprintfA(ngStr, " [NG+ %d]", player.ngLevel);

    wsprintfA(statusBuf, "Hero: %s (%s)  |  Lvl: %d%s  |  HP: %d/%d  |  MP: %d/%d  |  Gold: %d Gold  |  Loc: %s (Floor %d)",
        player.name, player.heroClass, player.level, ngStr,
        player.hp, player.maxHp, player.mp, player.maxMp,
        player.gold, locStr, player.floor);

    int bonusDef = player.armorBonusDef + (player.defensePoints * 3);
    int bonusInt = 0;
    if (player.companion.active > 0 && !player.companion.isDown) {
        if (player.companion.active == 1) bonusDef += 4;
        if (player.companion.active == 2) bonusInt += 5;
    }

    if (gameState == STATE_TAVERN) { LogMessage("Barkeep: 'Welcome! The Ruined Castle is dangerous!'"); return; }
    if (gameState == STATE_TAVERN) { LogMessage("Rumor: 'Equipment can be upgraded at the forge now.'"); return; }
    if (gameState == STATE_HELP) {
        wsprintfA(statusBuf, "=== COMPREHENSIVE HELP & LORE CODEX (Tab %d/4: %s) ===",
            g_HelpTab + 1,
            g_HelpTab == 0 ? "How to Play" : (g_HelpTab == 1 ? "Controls Ref" : (g_HelpTab == 2 ? "Bestiary & Lore" : "Crafting Recipes")));

        if (g_HelpTab == 0) {
            wsprintfA(infoBuf,
                "🎮 HOW TO PLAY GUIDE  [Press 1: How to Play | 2: Controls | 3: Bestiary | 4: Crafting | 5: Next Tab | 6: Back]\r\n"
                "• Hero Setup: Choose Warrior (High HP/DEF), Mage (High MP/Spell Power), or Rogue (High AGI/Crits).\r\n"
                "• Town Facilities: Spire Dungeon (F1-15), Boss Rush Arena (5 Waves), Forge & Alchemy, Mercenary Guild.\r\n"
                "• Progression: Defeat foes, claim Bounties, allocate Skill Points, master Crafting & launch New Game+ (NG+)!");
        } else if (g_HelpTab == 1) {
            wsprintfA(infoBuf,
                "⌨️ CONTROLS & SHORTCUTS REFERENCE\r\n"
                "• Action Buttons [1 - 6]: Execute current screen options / combat choices.\r\n"
                "• Hotkey [F1] or [H]: Toggle Help & Lore Codex overlay from anywhere in the game.\r\n"
                "• Hotkey [S]: Quick Save Game (Slot 1)  |  Hotkey [L]: Open Save / Load Manager\r\n"
                "• Hotkey [I]: Focus Inventory Hub  |  Hotkey [Esc]: Return to Town Square");
        } else if (g_HelpTab == 2) {
            wsprintfA(infoBuf,
                "🐉 BESTIARY & WORLD LORE CODEX\r\n"
                "• Biome 1 (Abandoned Mines): Cave Goblin, Goblin Slinger, Cave Spider, Taskmaster, Rock Golem, Goblin King.\r\n"
                "• Biome 2 (Ancient Catacombs): Skeleton Archer, Tomb Ghoul, Crypt Necromancer, Dread Wraith, Bone Colossus, Lich Lord.\r\n"
                "• Biome 3 (Dragon Spire): Magma Imp, Fire Drake, Obsidian Elemental, Wyvern Sentinel, Hellhound, Obsidian Dragon.\r\n"
                "• Lore: The Obsidian Spire is an ancient monolithic citadel sealing dark elemental nexuses beneath Oakhaven.");
        } else if (g_HelpTab == 3) {
            wsprintfA(infoBuf,
                "⚒️ CRAFTING RECIPES & MATERIAL DROPS\r\n"
                "• Consumables: Fire Bomb (1 Iron + 1 Core), Greater HP (2 Dust + 1 Iron), Elixir of Might (2 Dust + 1 Core).\r\n"
                "• Weapon Imbuing: Flaming (+6 Fire Dmg), Vampiric (25%% HP Siphon), Thunderous (+6 STR & 30%% Lightning).\r\n"
                "• Armor Imbuing: Fortified (+5 DEF & +20 Max HP), Warded (+4 DEF & +15 MP), Spiked (Reflects 35%% Dmg).\r\n"
                "• Sourcing: Salvage spare loot for 20G (+2 Iron, +1 Dust). Foes & Altars drop Iron, Dust, & Cores.");
        }
        RenderHelpTabLog();
    } else if (gameState == STATE_SAVE_LOAD) {
        char slot1[128], slot2[128], slot3[128], slot4[128];
        GetSlotSummary(0, slot1, 128);
        GetSlotSummary(1, slot2, 128);
        GetSlotSummary(2, slot3, 128);
        GetSlotSummary(3, slot4, 128);

        wsprintfA(infoBuf,
            "💾 SAVE / LOAD GAME SLOTS (Selected: Slot %d)\r\n"
            "%s %s\r\n"
            "%s %s\r\n"
            "%s %s\r\n"
            "%s %s\r\n\r\n"
            "Controls: [Select Slot] to change target slot | [Save Slot] to save game | [Load Slot] to restore hero.",
            g_SelectedSaveSlot + 1,
            g_SelectedSaveSlot == 0 ? ">>" : "  ", slot1,
            g_SelectedSaveSlot == 1 ? ">>" : "  ", slot2,
            g_SelectedSaveSlot == 2 ? ">>" : "  ", slot3,
            g_SelectedSaveSlot == 3 ? ">>" : "  ", slot4);
    } else if (gameState == STATE_FACTIONS || gameState == STATE_MOUNTS) {
        gameState = STATE_TOWN_PAGE2;
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_ACHIEVEMENTS) {
        int count = 0;
        for (int i = 0; i < 10; i++) if (g_Achievements[i]) count++;

        char achList[768]; achList[0] = '\0';
        for (int i = 0; i < 10; i++) {
            char entry[96];
            wsprintfA(entry, "%s [%s] %s: %s (+%dG)\r\n",
                g_Achievements[i] ? "🏆" : "🔒",
                g_Achievements[i] ? "UNLOCKED" : "LOCKED",
                g_AchieveNames[i], g_AchieveDescs[i], g_AchieveRewards[i]);
            lstrcatA(achList, entry);
        }

        wsprintfA(infoBuf,
            "🏆 HERO MILESTONES & ACHIEVEMENTS: %d / 10 Unlocked  |  NG+ Cycle: NG+ %d\r\n"
            "%s",
            count, player.ngLevel, achList);
    } else if (gameState == STATE_INVENTORY) {
        const char* fStr = player.invFilter == 0 ? "All" : (player.invFilter == 1 ? "Consumables" : (player.invFilter == 2 ? "Equipment" : "Materials"));
        const char* sStr = player.invSort == 0 ? "Rarity (Desc)" : (player.invSort == 1 ? "Name (A-Z)" : "Value (Desc)");
        
        char selItemBuf[256];
        char compBuf[256];
        if (player.invCount > 0 && player.selectedInvIdx >= 0 && player.selectedInvIdx < player.invCount) {
            InvItem* it = &player.inventory[player.selectedInvIdx];
            wsprintfA(selItemBuf, "[%s] %s (x%d) | Value: %dG | Desc: %s",
                g_RarityNames[it->rarity], it->name, it->count, it->value, it->desc);

            if (it->type == 1) {
                if (it->eqType == 1) {
                    int diff = it->bonusStr - player.weaponBonusStr;
                    wsprintfA(compBuf, "vs Equipped Weapon [%s (+%d STR)]: %s%d STR %s",
                        player.weaponName, player.weaponBonusStr, diff >= 0 ? "+" : "", diff, diff > 0 ? "(UPGRADE)" : (diff < 0 ? "(DOWNGRADE)" : "(SAME)"));
                } else {
                    int diff = it->bonusDef - player.armorBonusDef;
                    wsprintfA(compBuf, "vs Equipped Armor [%s (+%d DEF)]: %s%d DEF %s",
                        player.armorName, player.armorBonusDef, diff >= 0 ? "+" : "", diff, diff > 0 ? "(UPGRADE)" : (diff < 0 ? "(DOWNGRADE)" : "(SAME)"));
                }
            } else {
                lstrcpyA(compBuf, "N/A (Consumable/Material item)");
            }
        } else {
            lstrcpyA(selItemBuf, "No items in inventory.");
            lstrcpyA(compBuf, "N/A");
        }

        char invListBuf[384];
        invListBuf[0] = '\0';
        for (int i = 0; i < player.invCount && i < 6; i++) {
            char itemEntry[64];
            wsprintfA(itemEntry, "%s%s[%s]%s(x%d)", i > 0 ? " | " : "",
                i == player.selectedInvIdx ? ">>" : "",
                g_RarityNames[player.inventory[i].rarity],
                player.inventory[i].name,
                player.inventory[i].count);
            lstrcatA(invListBuf, itemEntry);
        }
        if (player.invCount > 6) lstrcatA(invListBuf, " ...");

        wsprintfA(infoBuf,
            "🎒 BACKPACK: %d / %d Slots  |  Filter: %s  |  Sort: %s\r\n"
            "SELECTED ITEM [%d/%d]: %s\r\n"
            "STAT COMPARISON: %s\r\n"
            "INVENTORY LIST: %s",
            player.invCount, player.maxInvSlots, fStr, sStr,
            player.invCount > 0 ? player.selectedInvIdx + 1 : 0, player.invCount, selItemBuf,
            compBuf,
            invListBuf);
    } else {
        char compBuf[128];
        if (player.companion.active > 0) {
            wsprintfA(compBuf, "%s (%s Lvl %d, HP: %d/%d, Upkeep: %dG) %s",
                player.companion.name, player.companion.role, player.companion.level,
                player.companion.hp, player.companion.maxHp, player.companion.upkeep,
                player.companion.isDown ? "[DOWNED]" : "[ACTIVE]");
        } else {
            lstrcpyA(compBuf, "None (Hire at Mercenary Guild)");
        }

        char bProgressBuf[256];
        if (g_ActiveBountyCount == 0) {
            lstrcpyA(bProgressBuf, "None active (Accept at Quest Board)");
        } else {
            bProgressBuf[0] = '\0';
            for (int i = 0; i < g_ActiveBountyCount; i++) {
                if (g_ActiveBounties[i].type == 2) {
                    if (lstrcmpA(g_ActiveBounties[i].targetMat, "iron") == 0) {
                        g_ActiveBounties[i].current = player.ironScrap;
                    }
                    if (g_ActiveBounties[i].current >= g_ActiveBounties[i].req) g_ActiveBounties[i].done = 1;
                }
                char itemBuf[96];
                const char* tStr = g_ActiveBounties[i].tier == 1 ? "[Bronze]" : (g_ActiveBounties[i].tier == 2 ? "[Silver]" : "[Gold]");
                wsprintfA(itemBuf, "%s%s %s(%d/%d%s) ", i > 0 ? " | " : "", tStr, g_ActiveBounties[i].title, g_ActiveBounties[i].current, g_ActiveBounties[i].req, g_ActiveBounties[i].done ? " READY" : "");
                lstrcatA(bProgressBuf, itemBuf);
            }
        }

        int offBonusDmg = player.offensePoints * 2;
        wsprintfA(infoBuf,
            "BIOME: %s (Hazard: %s)  |  COMPANION: %s  |  ARENA RECORD: Wave %d (🏆 %d Tr)\r\n"
            "ATTRIBUTES: STR %d (+%d) | INT %d (+%d) | DEF %d (+%d) | AGI %d | XP %d/%d\r\n"
            "EQUIPMENT: Weapon: %s%s%s | Armor: %s%s%s  |  BACKPACK: %d/%d Slots\r\n"
            "MATERIALS: Iron Scrap x%d | Arcane Dust x%d | Core x%d  |  SKILLS: %d SP (Off:%d Def:%d Util:%d)\r\n"
            "BOUNTIES: %s",
            g_Biomes[player.biome].name, g_Biomes[player.biome].hazardName, compBuf, player.arenaBestWave, player.arenaTokens,
            player.str, player.weaponBonusStr + offBonusDmg, player.intStat, bonusInt, player.def, bonusDef, player.agi, player.xp, player.nextXp,
            player.weaponPrefix[0] ? "[" : "", player.weaponPrefix[0] ? player.weaponPrefix : "", player.weaponPrefix[0] ? "] " : "", player.weaponName,
            player.armorPrefix[0] ? "[" : "", player.armorPrefix[0] ? player.armorPrefix : "", player.armorPrefix[0] ? "] " : "", player.armorName,
            player.invCount, player.maxInvSlots,
            player.ironScrap, player.arcaneDust, player.elementalCore,
            player.skillPoints, player.offensePoints, player.defensePoints, player.utilityPoints,
            bProgressBuf);
    }

    if (hStatusText) SetWindowTextA(hStatusText, statusBuf);
    if (hInfoText) SetWindowTextA(hInfoText, infoBuf);
}


void ExportScoreJSON() {
    HANDLE hFile = CreateFileA("kquest_score.json", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[512];
        wsprintfA(buf, "{\r\n  \"name\": \"%s\",\r\n  \"level\": %d,\r\n  \"hp\": %d,\r\n  \"maxHp\": %d,\r\n  \"mp\": %d,\r\n  \"maxMp\": %d,\r\n  \"str\": %d,\r\n  \"def\": %d,\r\n  \"gold\": %d\r\n}",
            player.name, player.level, player.hp, player.maxHp, player.mp, player.maxMp, player.str, player.def, player.gold);
        DWORD written = 0;
        WriteFile(hFile, buf, lstrlenA(buf), &written, NULL);
        CloseHandle(hFile);
        LogMessage("💾 High Score JSON Exported to kquest_score.json!");
    }
}

char* my_strstr(char* str, const char* sub) {
    if (!str || !sub) return 0;
    int i, j;
    for (i = 0; str[i] != '\0'; i++) {
        for (j = 0; sub[j] != '\0' && str[i + j] == sub[j]; j++);
        if (sub[j] == '\0') return &str[i];
    }
    return 0;
}

void ImportScoreJSON() {
    HANDLE hFile = CreateFileA("kquest_score.json", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[1024];
        DWORD bytesRead = 0;
        ReadFile(hFile, buf, sizeof(buf)-1, &bytesRead, NULL);
        buf[bytesRead] = 0;
        CloseHandle(hFile);
        
        char* namePtr = my_strstr(buf, "\"name\": \"");
        if (namePtr) {
            namePtr += 9;
            int i = 0;
            while(namePtr[i] != '"' && namePtr[i] != '\0' && i < 31) {
                player.name[i] = namePtr[i];
                i++;
            }
            player.name[i] = '\0';
        }
        
        #define READ_INT(key, field) do { \
            char search[32]; \
            wsprintfA(search, "\"%s\": ", key); \
            char* ptr = my_strstr(buf, search); \
            if (ptr) { \
                ptr += lstrlenA(search); \
                int val = 0; \
                while(*ptr >= '0' && *ptr <= '9') { val = val * 10 + (*ptr - '0'); ptr++; } \
                field = val; \
            } \
        } while(0)
        
        READ_INT("level", player.level);
        READ_INT("hp", player.hp);
        READ_INT("maxHp", player.maxHp);
        READ_INT("mp", player.mp);
        READ_INT("maxMp", player.maxMp);
        READ_INT("str", player.str);
        READ_INT("def", player.def);
        READ_INT("gold", player.gold);
        
        LogMessage("📂 High Score JSON Imported Successfully!");
        gameState = STATE_TOWN;
        SetupButtons();
        UpdateUI();
    } else {
        LogMessage("Could not open kquest_score.json.");
    }
}

void SetupButtons() {
    ShowWindow(hBtn1, SW_SHOW);
    ShowWindow(hBtn2, SW_SHOW);
    ShowWindow(hBtn3, SW_SHOW);
    ShowWindow(hBtn4, SW_SHOW);
    ShowWindow(hBtn5, SW_SHOW);
    ShowWindow(hBtn6, SW_SHOW);

    switch (gameState) {
        case STATE_CHAR_CREATE:
            SetWindowTextA(hBtn1, "Select Warrior");
            SetWindowTextA(hBtn2, "Select Mage");
            SetWindowTextA(hBtn3, "Select Rogue");
            SetWindowTextA(hBtn4, "Select Paladin");
            SetWindowTextA(hBtn5, "Select Ranger");
            SetWindowTextA(hBtn6, "Begin Quest");
            break;

        case STATE_TOWN: {
            char invBtn[64];
            wsprintfA(invBtn, "🎒 Inventory (%d/%d)", player.invCount, player.maxInvSlots);
            SetWindowTextA(hBtn1, "Enter Dungeon");
            SetWindowTextA(hBtn2, "🏟️ Boss Rush");
            SetWindowTextA(hBtn3, invBtn);
            SetWindowTextA(hBtn4, "📜 Board / Train");
            SetWindowTextA(hBtn5, "💾 Save / Load");
            SetWindowTextA(hBtn6, "▶️ More Options");
            break;
        }

        case STATE_TOWN_PAGE2:
            SetWindowTextA(hBtn1, "🚩 Factions");
            SetWindowTextA(hBtn2, "🐎 Mounts");
            SetWindowTextA(hBtn3, "⚙️ System Utils");
            SetWindowTextA(hBtn4, "---");
            SetWindowTextA(hBtn5, "---");
            SetWindowTextA(hBtn6, "◀️ Back to Town 1");
            break;

        case STATE_SAVE_LOAD: {
            char selBtn[64];
            wsprintfA(selBtn, "Select Slot (%d/4)", g_SelectedSaveSlot + 1);
            char saveBtn[64];
            wsprintfA(saveBtn, "💾 Save Slot %d", g_SelectedSaveSlot + 1);
            char loadBtn[64];
            wsprintfA(loadBtn, "📂 Load Slot %d", g_SelectedSaveSlot + 1);
            char delBtn[64];
            wsprintfA(delBtn, "🗑️ Clear Slot %d", g_SelectedSaveSlot + 1);

            SetWindowTextA(hBtn1, selBtn);
            SetWindowTextA(hBtn2, saveBtn);
            SetWindowTextA(hBtn3, loadBtn);
            SetWindowTextA(hBtn4, delBtn);
            SetWindowTextA(hBtn5, "⚡ Quick Save (Slot 1)");
            SetWindowTextA(hBtn6, "⬅️ Back to Town");
            break;
        }

        case STATE_ACHIEVEMENTS: {
            char ngBtn[64];
            wsprintfA(ngBtn, "✨ Start NG+ (NG+ %d)", player.ngLevel + 1);
            SetWindowTextA(hBtn1, "Refresh");
            SetWindowTextA(hBtn2, "❓ Help & Codex");
            SetWindowTextA(hBtn3, "---");
            SetWindowTextA(hBtn4, "---");
            SetWindowTextA(hBtn5, ngBtn);
            SetWindowTextA(hBtn6, "⬅️ Back to Town");
            break;
        }

        case STATE_HELP: {
            SetWindowTextA(hBtn1, "🎮 How to Play");
            SetWindowTextA(hBtn2, "⌨️ Controls");
            SetWindowTextA(hBtn3, "🐉 Bestiary");
            SetWindowTextA(hBtn4, "⚒️ Recipes");
            SetWindowTextA(hBtn5, "❓ Next Tab");
            SetWindowTextA(hBtn6, "⬅️ Back to Town");
            break;
        }

        case STATE_INVENTORY: {
            SetWindowTextA(hBtn1, "Next Item");
            SetWindowTextA(hBtn2, "Use / Equip");
            SetWindowTextA(hBtn3, "Sell Item");
            SetWindowTextA(hBtn4, "💰 Sell Commons");
            SetWindowTextA(hBtn5, "➕ Expand Slots");
            SetWindowTextA(hBtn6, "⬅️ Back to Town");
            break;
        }

        case STATE_BOSS_RUSH: {
            char wBtn[64];
            wsprintfA(wBtn, "Fight Wave %d", player.arenaWave > 0 ? player.arenaWave : 1);
            SetWindowTextA(hBtn1, wBtn);
            SetWindowTextA(hBtn2, "Sword (5 Tr)");
            SetWindowTextA(hBtn3, "Armor (5 Tr)");
            SetWindowTextA(hBtn4, "Ring (8 Tr)");
            SetWindowTextA(hBtn5, "Elixir (4 Tr)");
            SetWindowTextA(hBtn6, "Back to Town");
            break;
        }

        case STATE_TRAINING_HALL: {
            char oBtn[64], dBtn[64], uBtn[64];
            wsprintfA(oBtn, "Offense (%d/3)", player.offensePoints);
            wsprintfA(dBtn, "Defense (%d/3)", player.defensePoints);
            wsprintfA(uBtn, "Utility (%d/3)", player.utilityPoints);
            SetWindowTextA(hBtn1, oBtn);
            SetWindowTextA(hBtn2, dBtn);
            SetWindowTextA(hBtn3, uBtn);
            SetWindowTextA(hBtn4, "Respec (30G)");
            SetWindowTextA(hBtn5, "Merc Guild");
            SetWindowTextA(hBtn6, "Back to Town");
            break;
        }

        case STATE_QUEST_BOARD:
            SetWindowTextA(hBtn1, "Accept B1 (Bronze)");
            SetWindowTextA(hBtn2, "Accept B2 (Silver)");
            SetWindowTextA(hBtn3, "Accept B3 (Silver)");
            SetWindowTextA(hBtn4, "Accept B4 (Gold)");
            SetWindowTextA(hBtn5, "🎁 Claim Rewards");
            SetWindowTextA(hBtn6, "Back to Town");
            break;

        case STATE_MERCENARY:
            if (player.companion.active == 0) {
                SetWindowTextA(hBtn1, "Hire Paladin (80G)");
                SetWindowTextA(hBtn2, "Hire Archmage (100G)");
                SetWindowTextA(hBtn3, "Hire Cleric (70G)");
                SetWindowTextA(hBtn4, "Hire Ranger (90G)");
                SetWindowTextA(hBtn5, "---");
                SetWindowTextA(hBtn6, "Back to Town");
            } else {
                SetWindowTextA(hBtn1, player.companion.isDown ? "Revive Comp (20G)" : "---");
                SetWindowTextA(hBtn2, "Dismiss Comp");
                SetWindowTextA(hBtn3, "---");
                SetWindowTextA(hBtn4, "---");
                SetWindowTextA(hBtn5, "---");
                SetWindowTextA(hBtn6, "Back to Town");
            }
            break;

                case STATE_SHOP:
            SetWindowTextA(hBtn1, "Buy HP Potion (15G)");
            SetWindowTextA(hBtn2, "Buy MP Potion (15G)");
            SetWindowTextA(hBtn3, "Steel Sword (+6 STR, 50G)");
            SetWindowTextA(hBtn4, "Plate Armor (+9 DEF, 75G)");
            SetWindowTextA(hBtn5, "Back to Town");
            SetWindowTextA(hBtn6, "🍻 Tavern (NPC)");
            break;
            
        case STATE_TAVERN:
            SetWindowTextA(hBtn1, "Talk to Barkeep");
            SetWindowTextA(hBtn2, "Listen to Rumors");
            SetWindowTextA(hBtn3, "Buy Ale (5G)");
            SetWindowTextA(hBtn4, "Ask for Side Quest");
            SetWindowTextA(hBtn5, "Back to Town");
            SetWindowTextA(hBtn6, "---");
            break;

                case STATE_FACTIONS:
            SetWindowTextA(hBtn1, "Join Vanguard (STR)");
            SetWindowTextA(hBtn2, "Join Arcane (INT)");
            SetWindowTextA(hBtn3, "Join Syndicate (AGI)");
            SetWindowTextA(hBtn4, "---");
            SetWindowTextA(hBtn5, "---");
            SetWindowTextA(hBtn6, "Back");
            break;
        case STATE_MOUNTS:
            SetWindowTextA(hBtn1, "Buy Horse(100G)");
            SetWindowTextA(hBtn2, "Buy Wolf(200G)");
            SetWindowTextA(hBtn3, "Buy Dragon(500G)");
            SetWindowTextA(hBtn4, "---");
            SetWindowTextA(hBtn5, "---");
            SetWindowTextA(hBtn6, "Back");
            break;
        case STATE_CRAFTING:
            SetWindowTextA(hBtn1, "Salvage Loot (20G)");
            SetWindowTextA(hBtn2, "Craft Fire Bomb");
            SetWindowTextA(hBtn3, "Craft Greater HP");
            SetWindowTextA(hBtn4, "Craft Masterwork Relic");
            SetWindowTextA(hBtn5, "Imbue Weapon/Armor");
            SetWindowTextA(hBtn6, "Back to Town");
            break;

        case STATE_DUNGEON:
            SetWindowTextA(hBtn1, "Advance Chamber");
            SetWindowTextA(hBtn2, "Descend Staircase");
            SetWindowTextA(hBtn3, "Use HP Potion");
            SetWindowTextA(hBtn4, "Use MP Potion");
            SetWindowTextA(hBtn5, "Return to Town");
            SetWindowTextA(hBtn6, "---");
            break;

        case STATE_MAGIC_MENU:
            if (lstrcmpA(player.heroClass, "Mage") == 0) SetWindowTextA(hBtn1, "Fireball (10 MP)");
            else if (lstrcmpA(player.heroClass, "Rogue") == 0) SetWindowTextA(hBtn1, "Shadow Strike (8 MP)");
            else if (lstrcmpA(player.heroClass, "Paladin") == 0) SetWindowTextA(hBtn1, "Smite (8 MP)");
            else if (lstrcmpA(player.heroClass, "Ranger") == 0) SetWindowTextA(hBtn1, "Aimed Shot (8 MP)");
            else SetWindowTextA(hBtn1, "Shield Bash (5 MP)");

            SetWindowTextA(hBtn2, "Lightning Storm (18 MP)");
            SetWindowTextA(hBtn3, "Holy Shield (14 MP)");
            SetWindowTextA(hBtn4, "Berserk Might (12 MP)");
            SetWindowTextA(hBtn5, "---");
            SetWindowTextA(hBtn6, "Back to Combat");
            break;

        case STATE_COMBAT:
            SetWindowTextA(hBtn1, "Attack");
            SetWindowTextA(hBtn2, "Spells / Magic");
            SetWindowTextA(hBtn3, "Use HP / Gr.HP");
            if (player.fireBombs > 0) {
                SetWindowTextA(hBtn4, "Throw Fire Bomb");
            } else {
                SetWindowTextA(hBtn4, "Use MP Potion");
            }
            SetWindowTextA(hBtn5, "Flee Battle");
            if (player.offensePoints >= 3) {
                SetWindowTextA(hBtn6, "Execute (12 MP)");
            } else if (player.defensePoints >= 3) {
                SetWindowTextA(hBtn6, "Iron Will (8 MP)");
            } else if (player.utilityPoints >= 3) {
                SetWindowTextA(hBtn6, "Mana Surge (0 MP)");
            } else {
                SetWindowTextA(hBtn6, "---");
            }
            break;

        case STATE_GAME_OVER:
            SetWindowTextA(hBtn1, "Restart Journey");
            SetWindowTextA(hBtn2, "---");
            SetWindowTextA(hBtn3, "---");
            SetWindowTextA(hBtn4, "---");
            SetWindowTextA(hBtn5, "---");
            SetWindowTextA(hBtn6, "---");
            break;
    }
}

void TriggerCompanionCombatTurn() {
    if (player.companion.active == 0 || player.companion.isDown || currentEnemy.hp <= 0) return;

    if (player.companion.active == 1) { // Paladin
        int dmg = 14 + player.level * 4;
        currentEnemy.hp -= dmg;
        char msg[128];
        wsprintfA(msg, "🛡️ Companion Sir Gareth performs Holy Taunt for %d melee damage!", dmg);
        LogMessage(msg);
    } else if (player.companion.active == 2) { // Archmage
        int dmg = 25 + player.level * 6;
        currentEnemy.hp -= dmg;
        char msg[128];
        wsprintfA(msg, "🔮 Companion Lady Pyra casts Arcane Burst dealing %d magic damage!", dmg);
        LogMessage(msg);
    } else if (player.companion.active == 3) { // Cleric
        int heal = 18 + player.level * 4;
        player.hp += heal;
        if (player.hp > player.maxHp) player.hp = player.maxHp;
        char msg[128];
        wsprintfA(msg, "✨ Companion Brother Tobias casts Divine Heal, restoring +%d HP to Hero!", heal);
        LogMessage(msg);
    }
}

void PayCompanionUpkeep() {
    if (player.companion.active > 0 && !player.companion.isDown) {
        if (player.gold >= player.companion.upkeep) {
            player.gold -= player.companion.upkeep;
            char msg[128];
            wsprintfA(msg, "🪙 Paid %d Gold upkeep for companion %s.", player.companion.upkeep, player.companion.name);
            LogMessage(msg);
        } else {
            char msg[128];
            wsprintfA(msg, "⚠️ Cannot afford %d Gold upkeep! Companion %s departed your party.", player.companion.upkeep, player.companion.name);
            LogMessage(msg);
            player.companion.active = 0;
        }
    }
}

void CheckLevelUp() {
    if (player.xp >= player.nextXp) {
        player.level++;
        player.skillPoints += 2;
        player.xp -= player.nextXp;
        player.nextXp = (int)(player.nextXp * 1.5);
        player.maxHp += 15; player.hp = player.maxHp;
        player.maxMp += 10; player.mp = player.maxMp;
        player.str += 3; player.intStat += 2; player.def += 2; player.agi += 2;
        SfxLevelUp();
        char msg[128];
        wsprintfA(msg, "🌟 LEVEL UP! Reached Level %d! Gained +2 Skill Points! Visit Training Hall in Town.", player.level);
        LogMessage(msg);

        if (player.companion.active > 0) {
            player.companion.level = player.level;
            player.companion.maxHp += 12;
            player.companion.hp = player.companion.maxHp;
            char cmsg[128];
            wsprintfA(cmsg, "🛡️ Companion %s leveled up to Level %d!", player.companion.name, player.level);
            LogMessage(cmsg);
        }
    }
}

void StartCombat() {
    gameState = STATE_COMBAT;
    const BiomeDef* b = &g_Biomes[player.biome];

    currentEnemy.phase = 1;
    currentEnemy.maxPhases = 1;

    if (player.floor == 5) {
        lstrcpyA(currentEnemy.name, b->boss.name);
        currentEnemy.maxHp = b->boss.maxHp;
        currentEnemy.str = b->boss.str;
        currentEnemy.def = b->boss.def;
        currentEnemy.xp = b->boss.xp;
        currentEnemy.gold = b->boss.gold;
        currentEnemy.maxPhases = b->maxPhases;
    } else if (player.floor >= 10) {
        lstrcpyA(currentEnemy.name, "Obsidian Dragon (Final Boss)");
        currentEnemy.maxHp = 250;
        currentEnemy.str = 32;
        currentEnemy.def = 18;
        currentEnemy.xp = 600;
        currentEnemy.gold = 500;
        currentEnemy.maxPhases = 2;
    } else {
        int r = xrand() % 5;
        const MonsterDef* m = &b->monsters[r];
        lstrcpyA(currentEnemy.name, m->name);
        currentEnemy.maxHp = m->hp;
        currentEnemy.str = m->str;
        currentEnemy.def = m->def;
        currentEnemy.xp = m->xp;
        currentEnemy.gold = m->gold;

        // Scale monster stats with floor
        int floorBonus = player.floor - 1;
        currentEnemy.maxHp += floorBonus * 8;
        currentEnemy.str += floorBonus * 2;
        currentEnemy.def += floorBonus * 1;
        currentEnemy.xp += floorBonus * 10;
        currentEnemy.gold += floorBonus * 8;
    }
    if (player.ngLevel > 0) {
        float mult = 1.0f + (player.ngLevel * 0.5f);
        float rMult = 1.0f + (player.ngLevel * 0.4f);
        currentEnemy.maxHp = (int)(currentEnemy.maxHp * mult);
        currentEnemy.str = (int)(currentEnemy.str * mult);
        currentEnemy.def = (int)(currentEnemy.def * mult);
        currentEnemy.xp = (int)(currentEnemy.xp * rMult);
        currentEnemy.gold = (int)(currentEnemy.gold * rMult);
        char ngName[64];
        wsprintfA(ngName, "%s [NG+%d]", currentEnemy.name, player.ngLevel);
        lstrcpyA(currentEnemy.name, ngName);
    }
    currentEnemy.hp = currentEnemy.maxHp;

    char msg[128];
    wsprintfA(msg, "⚠️ Encounter in %s! A hostile %s (HP: %d, STR: %d, DEF: %d) attacks on Floor %d!",
        b->name, currentEnemy.name, currentEnemy.hp, currentEnemy.str, currentEnemy.def, player.floor);
    LogMessage(msg);
    SetupButtons();
    UpdateUI();
}

void EnemyTurn() {
    if (currentEnemy.hp <= 0) return;

    if (player.lightningDazeTurns > 0) {
        player.lightningDazeTurns--;
        LogMessage("⚡ Target is DAZED by Lightning Storm and misses its turn!");
        UpdateUI();
        return;
    }

    if (player.holyShieldTurns > 0) {
        player.holyShieldTurns--;
        LogMessage("🛡️ HOLY SHIELD completely absorbs incoming attack! (0 Damage taken)");
        UpdateUI();
        return;
    }

    int bonusDef = (player.companion.active == 1 && !player.companion.isDown) ? 4 : 0;
    bonusDef += (player.defensePoints * 3);
    int totalDef = player.def + player.armorBonusDef + bonusDef;
    int dmg = currentEnemy.str - (totalDef / 2);
    if (player.defensePoints >= 2) dmg = (dmg * 90) / 100;
    if (dmg < 2) dmg = 2;

    if (player.ironWillTurns > 0) {
        dmg = dmg / 2;
        if (dmg < 1) dmg = 1;
        player.ironWillTurns--;
        LogMessage("🛡️ Iron Will barrier absorbs 50% incoming damage!");
    }

    if (player.companion.active == 1 && !player.companion.isDown) {
        int absorbed = (dmg * 40) / 100;
        if (absorbed < 1) absorbed = 1;
        dmg -= absorbed;
        player.companion.hp -= absorbed;
        char pmsg[128];
        wsprintfA(pmsg, "🛡️ Companion Sir Gareth absorbs %d damage meant for Hero!", absorbed);
        LogMessage(pmsg);
        if (player.companion.hp <= 0) {
            player.companion.hp = 0;
            player.companion.isDown = 1;
            LogMessage("😵 Companion Sir Gareth fell unconscious in battle!");
        }
    }

    player.hp -= dmg;
    SfxCombatHit();
    char msg[128];
    wsprintfA(msg, "💥 %s attacks you for %d damage!", currentEnemy.name, dmg);
    LogMessage(msg);
    
    int massiveDmg = player.maxHp / 5;
    int isMassive = (dmg >= massiveDmg);
    char ftxt[16];
    wsprintfA(ftxt, "-%d", dmg);
    AddGdiFloatText(ftxt, 140, 70, RGB(243, 139, 168), isMassive);
    if (isMassive) {
        g_GdiScreenShakeTimer = 15;
        g_GdiScreenShakeIntensity = 10;
        g_GdiRedFlashTimer = 20;
    }

    // Spiked Armor Reflection
    if (lstrcmpA(player.armorPrefix, "Spiked") == 0) {
        int reflectDmg = (dmg * 35) / 100;
        if (reflectDmg < 2) reflectDmg = 2;
        currentEnemy.hp -= reflectDmg;
        char rmsg[128];
        wsprintfA(rmsg, "🌵 Spiked Armor reflects %d thorn damage back to %s!", reflectDmg, currentEnemy.name);
        LogMessage(rmsg);
    }

    // Environmental Hazard Ambient Tick during Combat (20% chance)
    if ((xrand() % 100) < 20 && player.hp > 0) {
        const BiomeDef* b = &g_Biomes[player.biome];
        int hDmg = 3 + (player.floor * 3 / 2);
        player.hp -= hDmg;
        if (player.hp < 0) player.hp = 0;
        char hmsg[128];
        wsprintfA(hmsg, "⚠️ %s Ambient Hazard! %s environment deals %d damage!", b->hazardName, b->name, hDmg);
        LogMessage(hmsg);
    }

    if (player.hp <= 0) {
        if (player.phoenixElixirs > 0) {
            player.phoenixElixirs--;
            player.hp = player.maxHp / 2;
            player.mp = player.maxMp / 2;
            SfxLevelUp();
            LogMessage("🔥 PHOENIX ELIXIR AUTO-REVIVE! Rising from the ashes, Hero is restored to 50% HP & MP!");
            UpdateUI();
            return;
        }
        player.hp = 0;
        SfxDeath();
        if (player.arenaActive) {
            player.arenaActive = 0;
            char fmsg[128];
            wsprintfA(fmsg, "💔 Slain in Arena Wave %d! Arena Streak Ended.", player.arenaWave);
            LogMessage(fmsg);
        } else {
            LogMessage("💀 YOU HAVE FALLEN IN COMBAT! Your soul vanishes into darkness.");
        }
        gameState = STATE_GAME_OVER;
        SetupButtons();
    }
    UpdateUI();
}

void CombatVictory() {
    if (currentEnemy.phase < currentEnemy.maxPhases) {
        currentEnemy.phase++;
        currentEnemy.maxHp = (int)(currentEnemy.maxHp * 1.4f);
        currentEnemy.hp = currentEnemy.maxHp;
        currentEnemy.str = (int)(currentEnemy.str * 1.35f);
        currentEnemy.def = (int)(currentEnemy.def * 1.25f);
        char phaseMsg[128];
        wsprintfA(phaseMsg, "🔥 LEGENDARY BOSS PHASE %d TRANSITION! %s enrages into Phase %d with full HP and +35%% power!", currentEnemy.phase, currentEnemy.name, currentEnemy.phase);
        LogMessage(phaseMsg);
        SfxSpellCast();
        SetupButtons();
        UpdateUI();
        return;
    }

    int rewardGold = currentEnemy.gold;
    if (player.utilityPoints >= 2) rewardGold = (rewardGold * 125) / 100;
    char msg[128];
    wsprintfA(msg, "🎉 VICTORY! Defeated %s! Gained +%d XP and +%d Gold!", currentEnemy.name, currentEnemy.xp, rewardGold);
    LogMessage(msg);

    player.xp += currentEnemy.xp;
    player.gold += rewardGold;

    UnlockAchievement(0); // First Blood
    if (ContainsSubstr(currentEnemy.name, "Boss") || ContainsSubstr(currentEnemy.name, "King") || ContainsSubstr(currentEnemy.name, "Lord") || ContainsSubstr(currentEnemy.name, "Dragon") || ContainsSubstr(currentEnemy.name, "Titan") || ContainsSubstr(currentEnemy.name, "Overlord")) {
        UnlockAchievement(2); // Dragon Slayer
    }
    if (player.floor >= 5) UnlockAchievement(1); // Dungeon Explorer
    if (player.gold >= 500) UnlockAchievement(6); // Wealthy Hero

    PayCompanionUpkeep();

    // Material Drops
    if ((xrand() % 100) < 60) {
        int iGot = 1 + (xrand() % 2);
        player.ironScrap += iGot;
        char m1[64]; wsprintfA(m1, "🔧 Looted +%d Iron Scrap!", iGot); LogMessage(m1);
    }
    if ((xrand() % 100) < 40) {
        player.arcaneDust += 1;
        LogMessage("✨ Looted +1 Arcane Dust!");
    }
    if ((xrand() % 100) < 25) {
        player.elementalCore += 1;
        LogMessage("🔥 Looted +1 Elemental Core!");
    }

    if (player.questMonstersKilled < 5) player.questMonstersKilled++;
    if (ContainsSubstr(currentEnemy.name, "Boss") || ContainsSubstr(currentEnemy.name, "King") || ContainsSubstr(currentEnemy.name, "Lord") || ContainsSubstr(currentEnemy.name, "Dragon") || ContainsSubstr(currentEnemy.name, "Titan") || ContainsSubstr(currentEnemy.name, "Overlord")) {
        player.questBossKilled = 1;
    }

    for (int i = 0; i < g_ActiveBountyCount; i++) {
        if (!g_ActiveBounties[i].done && g_ActiveBounties[i].type == 0) { // slay
            g_ActiveBounties[i].current++;
            if (g_ActiveBounties[i].current >= g_ActiveBounties[i].req) {
                g_ActiveBounties[i].done = 1;
                char bmsg[128];
                wsprintfA(bmsg, "📜 Bounty Completed: %s! Claim at Town Quest Board!", g_ActiveBounties[i].title);
                LogMessage(bmsg);
            }
        }
    }

    CheckLevelUp();

    if (player.arenaActive) {
        int trophies = (player.arenaWave <= 5) ? g_ArenaBosses[player.arenaWave - 1].trophies : (player.arenaWave + 3);
        player.arenaTokens += trophies;
        if (player.arenaWave > player.arenaBestWave) player.arenaBestWave = player.arenaWave;
        if (player.arenaWave >= 5) UnlockAchievement(5); // Arena Champion
        
        char amsg[128];
        wsprintfA(amsg, "🏆 ARENA WAVE %d CLEARED! +%d Arena Trophies awarded! Best Record: Wave %d!", player.arenaWave, trophies, player.arenaBestWave);
        LogMessage(amsg);

        int healHp = (player.maxHp * 40) / 100;
        int healMp = (player.maxMp * 40) / 100;
        player.hp += healHp; if (player.hp > player.maxHp) player.hp = player.maxHp;
        player.mp += healMp; if (player.mp > player.maxMp) player.mp = player.maxMp;
        LogMessage("🍷 Arena Champion's Rest: Recovered 40% HP & MP!");

        Beep(523, 100); Beep(659, 100); Beep(784, 150);

        player.arenaWave++;
        player.arenaActive = 0;
        gameState = STATE_BOSS_RUSH;
        SetupButtons();
        UpdateUI();
        return;
    }

    gameState = STATE_DUNGEON;
    SetupButtons();
    UpdateUI();
}

void CastLightningStorm() {
    if (gameState != STATE_COMBAT || currentEnemy.hp <= 0) return;
    int cost = 18;
    if (player.mp < cost) {
        LogMessage("Not enough MP for Lightning Storm (Requires 18 MP)!");
        return;
    }
    player.mp -= cost;
    SfxSpellCast();
    int bonusInt = (player.companion.active == 2 && !player.companion.isDown) ? 5 : 0;
    int dmg = (int)((player.intStat + bonusInt) * 25 + player.str * 8);
    if (player.berserkTurns > 0) dmg *= 2;
    if (player.manaSurgeActive) {
        dmg = (int)(dmg * 1.5f);
        player.manaSurgeActive = 0;
        LogMessage("⚡ Mana Surge empowers Lightning Storm by +50%!");
    }
    char msg[128];
    wsprintfA(msg, "⚡ LIGHTNING STORM! Heavy thunder strikes %s for %d magic damage!", currentEnemy.name, dmg);
    LogMessage(msg);
    currentEnemy.hp -= dmg;
    if ((xrand() % 100) < 45) {
        player.lightningDazeTurns = 1;
        LogMessage("⚡ Target is DAZED by the electrical shockwave!");
    }
    if (currentEnemy.hp <= 0) {
        currentEnemy.hp = 0;
        CombatVictory();
        return;
    }
    TriggerCompanionCombatTurn();
    if (currentEnemy.hp <= 0) {
        currentEnemy.hp = 0;
        CombatVictory();
        return;
    }
    EnemyTurn();
}

void CastHolyShield() {
    if (gameState != STATE_COMBAT || currentEnemy.hp <= 0) return;
    int cost = 14;
    if (player.mp < cost) {
        LogMessage("Not enough MP for Holy Shield (Requires 14 MP)!");
        return;
    }
    player.mp -= cost;
    SfxSpellCast();
    player.holyShieldTurns = 2;
    player.hp += 25;
    if (player.hp > player.maxHp) player.hp = player.maxHp;
    LogMessage("🛡️ HOLY SHIELD ACTIVATED! Granted 100% invulnerability barrier for 2 turns and +25 HP recovery!");
    UpdateUI();
    EnemyTurn();
}

void CastBerserkMight() {
    if (gameState != STATE_COMBAT || currentEnemy.hp <= 0) return;
    int cost = 12;
    if (player.mp < cost) {
        LogMessage("Not enough MP for Berserk Might (Requires 12 MP)!");
        return;
    }
    player.mp -= cost;
    SfxSpellCast();
    player.berserkTurns = 3;
    LogMessage("🔥 BERSERK MIGHT ACTIVATED! Attack damage doubled (+100%) for 3 turns!");
    UpdateUI();
    EnemyTurn();
}

void UsePhoenixElixir() {
    if (player.phoenixElixirs <= 0) {
        LogMessage("No Phoenix Elixirs in inventory!");
        return;
    }
    player.phoenixElixirs--;
    player.hp = player.maxHp;
    player.mp = player.maxMp;
    SfxLevelUp();
    LogMessage("🔥 DRANK PHOENIX ELIXIR! Restored 100% Max HP and 100% Max MP!");
    UpdateUI();
    if (gameState == STATE_COMBAT) EnemyTurn();
}

void HandleButton1() {
    if (gameState == STATE_UTILS) {
        gameState = STATE_ACHIEVEMENTS;
        LogMessage("🏆 Opened Milestones & Achievements Tracker.");
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_CONFIG) {
        g_BindingAction = 0;
        LogMessage("Press any key (A-Z, 0-9) to bind Action 1...");
        return;
    }

    if (gameState == STATE_TAVERN) { LogMessage("Barkeep: 'Welcome! The Ruined Castle is dangerous!'"); return; }
    if (gameState == STATE_TAVERN) { LogMessage("Rumor: 'Equipment can be upgraded at the forge now.'"); return; }
    if (gameState == STATE_HELP) {
        g_HelpTab = 0;
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_SAVE_LOAD) {
        g_SelectedSaveSlot = (g_SelectedSaveSlot + 1) % 4;
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_ACHIEVEMENTS) {
        UpdateUI();
        return;
    }
    if (gameState == STATE_INVENTORY) {
        if (player.invCount > 0) {
            player.selectedInvIdx = (player.selectedInvIdx + 1) % player.invCount;
        }
        UpdateUI();
        return;
    }
    if (gameState == STATE_BOSS_RUSH) {
        if (!player.arenaWave) player.arenaWave = 1;
        player.arenaActive = 1;
        if (player.arenaWave <= 5) {
            const ArenaBossDef* b = &g_ArenaBosses[player.arenaWave - 1];
            lstrcpyA(currentEnemy.name, b->name);
            currentEnemy.maxHp = b->hp; currentEnemy.hp = b->hp;
            currentEnemy.str = b->str; currentEnemy.def = b->def;
            currentEnemy.xp = b->xp; currentEnemy.gold = b->gold;
        } else {
            wsprintfA(currentEnemy.name, "Wave %d Apex Titan (Boss Rush)", player.arenaWave);
            currentEnemy.maxHp = 200 + player.arenaWave * 70; currentEnemy.hp = currentEnemy.maxHp;
            currentEnemy.str = 20 + player.arenaWave * 7; currentEnemy.def = 10 + player.arenaWave * 3;
            currentEnemy.xp = player.arenaWave * 350; currentEnemy.gold = player.arenaWave * 250;
        }
        gameState = STATE_COMBAT;
        Beep(523, 120); Beep(659, 120);
        char wmsg[128];
        wsprintfA(wmsg, "🏟️ ARENA BOSS RUSH - WAVE %d! Challenging %s!", player.arenaWave, currentEnemy.name);
        LogMessage(wmsg);
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_TRAINING_HALL) {
        if (player.skillPoints > 0 && player.offensePoints < 3) {
            player.offensePoints++;
            player.skillPoints--;
            LogMessage("⚔️ Allocated 1 SP into Offense Tree!");
            if (player.offensePoints == 3) LogMessage("🔥 UNLOCKED: Execute (3x Damage Finisher, 12 MP)!");
            SetupButtons();
            UpdateUI();
        } else if (player.offensePoints >= 3) {
            LogMessage("Offense Tree already at max rank!");
        } else {
            LogMessage("No Skill Points available! Level up to earn more.");
        }
        return;
    }
    if (gameState == STATE_QUEST_BOARD) {
        AcceptBounty(0);
        return;
    }
    if (gameState == STATE_MAGIC_MENU) {
        int cost = 5;
        if (lstrcmpA(player.heroClass, "Mage") == 0) cost = 10;
        else if (lstrcmpA(player.heroClass, "Rogue") == 0) cost = 8;
        else if (lstrcmpA(player.heroClass, "Paladin") == 0) cost = 8;
        else if (lstrcmpA(player.heroClass, "Ranger") == 0) cost = 8;

        if (player.mp >= cost) {
            player.mp -= cost;
            SfxSpellCast();
            int surgeMult = player.manaSurgeActive ? 150 : 100;
            if (player.manaSurgeActive) {
                LogMessage("⚡ Mana Surge empowers your spell power by +50%!");
                player.manaSurgeActive = 0;
            }

            int dmg = 0;
            if (lstrcmpA(player.heroClass, "Mage") == 0) {
                int bonusInt = (player.companion.active == 2 && !player.companion.isDown) ? 5 : 0;
                dmg = (int)(((player.intStat + bonusInt) * 22 * surgeMult) / 1000);
                char msg[128]; wsprintfA(msg, "🔥 Cast Fireball! Dealt %d magic damage to %s!", dmg, currentEnemy.name); LogMessage(msg);
            } else if (lstrcmpA(player.heroClass, "Rogue") == 0) {
                dmg = (int)((player.agi * 18 * surgeMult) / 1000);
                char msg[128]; wsprintfA(msg, "🗡️ Shadow Strike! Dealt %d critical damage to %s!", dmg, currentEnemy.name); LogMessage(msg);
            } else if (lstrcmpA(player.heroClass, "Paladin") == 0) {
                int totalStr = player.str + player.weaponBonusStr;
                dmg = (int)(((totalStr * 10 + player.intStat * 10) * surgeMult) / 1000);
                char msg[128]; wsprintfA(msg, "✨ Smite! Dealt %d holy damage to %s!", dmg, currentEnemy.name); LogMessage(msg);
            } else if (lstrcmpA(player.heroClass, "Ranger") == 0) {
                dmg = (int)((player.agi * 20 * surgeMult) / 1000);
                char msg[128]; wsprintfA(msg, "🏹 Aimed Shot! Dealt %d precise damage to %s!", dmg, currentEnemy.name); LogMessage(msg);
            } else {
                int totalStr = player.str + player.weaponBonusStr;
                dmg = (int)((totalStr * 15 * surgeMult) / 1000);
                char msg[128]; wsprintfA(msg, "🛡️ Shield Bash! Dealt %d physical damage to %s!", dmg, currentEnemy.name); LogMessage(msg);
            }
            gameState = STATE_COMBAT;
            currentEnemy.hp -= dmg;
            if (currentEnemy.hp <= 0) {
                currentEnemy.hp = 0; CombatVictory(); return;
            }
            TriggerCompanionCombatTurn();
            if (currentEnemy.hp <= 0) {
                currentEnemy.hp = 0; CombatVictory(); return;
            }
            EnemyTurn();
            UpdateUI();
        } else {
            LogMessage("Not enough MP!");
        }
    } else if (gameState == STATE_CHAR_CREATE) {
        selectedClassIndex = 0;
        InitHero(0);
        LogMessage("Selected Class: Warrior (High HP & Defense).");
        UpdateUI();
    } else if (gameState == STATE_TOWN) {
        gameState = STATE_DUNGEON;
        SfxDoorOpen();
        char msg[128];
        wsprintfA(msg, "You venture into %s...", g_Biomes[player.biome].name);
        LogMessage(msg);
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_TAVERN) {
        gameState = STATE_TOWN;
        LogMessage("Returned to Town.");
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_MERCENARY) {
        if (player.companion.active > 0) {
            if (player.companion.isDown) {
                if (player.gold >= 20) {
                    player.gold -= 20;
                    player.companion.isDown = 0;
                    player.companion.hp = player.companion.maxHp;
                    LogMessage("✨ Revived companion! Health fully restored!");
                    SetupButtons();
                    UpdateUI();
                } else {
                    LogMessage("Need 20 Gold to revive companion!");
                }
            }
        } else if (player.gold >= 80) {
            player.gold -= 80;
            player.companion.active = 1; // Paladin
            lstrcpyA(player.companion.name, "Sir Gareth");
            lstrcpyA(player.companion.role, "Paladin Tank");
            player.companion.level = player.level;
            player.companion.maxHp = 70 + (player.level - 1) * 12;
            player.companion.hp = player.companion.maxHp;
            player.companion.upkeep = 10;
            player.companion.cost = 80;
            player.companion.isDown = 0;
            LogMessage("🛡️ Hired Paladin Tank Sir Gareth! Devotion Aura Active (+4 DEF)!");
            UnlockAchievement(4); // Mercenary Leader
            SetupButtons();
            UpdateUI();
        } else {
            LogMessage("Need 80 Gold to hire Paladin Tank!");
        }
    } else if (gameState == STATE_SHOP) {
        if (player.gold >= 15) {
            player.gold -= 15;
            player.hpPotions++;
            LogMessage("Bought 1x Health Potion (+35 HP) for 15 Gold.");
            UpdateUI();
        } else {
            LogMessage("Not enough gold!");
        }
    } else if (gameState == STATE_SHOP) {
        gameState = STATE_TAVERN;
        LogMessage("Entered the Rusty Dragon Tavern.");
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_CRAFTING) {
        if (player.gold >= 30 && player.ironScrap >= 3) {
            player.gold -= 30;
            player.ironScrap -= 3;
            player.weaponBonusStr += 2;
            player.armorBonusDef += 2;
            SfxSpellCast();
            LogMessage("⬆️ Upgraded Gear! Weapon STR +2, Armor DEF +2!");
            UpdateUI();
        } else {
            LogMessage("Need 30 Gold and 3 Iron Scrap to Upgrade Gear!");
        }
    } else if (gameState == STATE_DUNGEON) {
        int r = xrand() % 100;
        if (r < 50) {
            StartCombat();
        } else if (r < 70) {
            int g = 15 + (xrand() % 25) + (player.floor * 10);
            player.gold += g;
            player.ironScrap += 1;
            player.arcaneDust += 1;
            SfxItemPickup();
            char msg[128];
            wsprintfA(msg, "✨ Found a treasure chest in %s with %d Gold, +1 Iron Scrap, +1 Arcane Dust!", g_Biomes[player.biome].name, g);
            LogMessage(msg);
            UpdateUI();
        } else if (r < 85) {
            const BiomeDef* b = &g_Biomes[player.biome];
            int hDmg = 6 + player.floor * 2 + (xrand() % 5);
            player.hp -= hDmg;
            char msg[128];
            if (player.biome == 0) {
                wsprintfA(msg, "🪨 CAVE-IN HAZARD! Loose rocks crash down! Took %d physical damage!", hDmg);
            } else if (player.biome == 1) {
                wsprintfA(msg, "☠️ POISON FOG HAZARD! Toxic miasma fills the vault! Took %d poison damage!", hDmg);
            } else {
                wsprintfA(msg, "🔥 LAVA BURST HAZARD! Erupting magma geysers shoot up! Took %d fire damage!", hDmg);
            }
            LogMessage(msg);
            if (player.hp <= 0) {
                player.hp = 0;
                SfxDeath();
                LogMessage("💀 Slain by environmental hazard!");
                gameState = STATE_GAME_OVER;
                SetupButtons();
            }
            UpdateUI();
        } else {
            if ((xrand() % 2) == 0) {
                player.hp = player.maxHp;
                LogMessage("⛩️ Prayed at a glowing red altar. Health fully restored!");
            } else {
                player.mp = player.maxMp;
                LogMessage("⛩️ Prayed at a glowing blue altar. Mana fully restored!");
            }
            UpdateUI();
        }
    } else if (gameState == STATE_COMBAT) {
        SfxCombatHit();
        int offMult = 100 + (player.offensePoints * 8);
        int totalStr = player.str + player.weaponBonusStr;
        int dmg = (int)(((totalStr * 12 - currentEnemy.def * 5) * offMult) / 1000);
        if (dmg < 2) dmg = 2;

        BOOL isCrit = ((xrand() % 100) < ((player.agi * 2) + (player.offensePoints >= 2 ? 10 : 0)));
        if (isCrit) {
            dmg = (int)(dmg * 1.75);
            char msg[128];
            wsprintfA(msg, "🎯 CRITICAL HIT! You strike %s for %d damage!", currentEnemy.name, dmg);
            LogMessage(msg);
        } else {
            char msg[128];
            wsprintfA(msg, "⚔️ You attack %s dealing %d damage.", currentEnemy.name, dmg);
            LogMessage(msg);
        }
        
        char ftxt[16];
        wsprintfA(ftxt, "-%d", dmg);
        AddGdiFloatText(ftxt, 560, 65, RGB(249, 226, 175), isCrit);

        // Weapon Enchantment Effects
        if (lstrcmpA(player.weaponPrefix, "Flaming") == 0) {
            dmg += 6;
            LogMessage("🔥 Flaming Enchantment scorches target for +6 fire damage!");
        }
        if (lstrcmpA(player.weaponPrefix, "Vampiric") == 0) {
            int heal = (dmg * 25) / 100;
            if (heal < 2) heal = 2;
            player.hp += heal;
            if (player.hp > player.maxHp) player.hp = player.maxHp;
            char vmsg[128]; wsprintfA(vmsg, "🩸 Vampiric Enchantment siphons +%d HP back!", heal);
            LogMessage(vmsg);
        }
        if (lstrcmpA(player.weaponPrefix, "Thunderous") == 0) {
            if ((xrand() % 100) < 35) {
                dmg += 8;
                LogMessage("⚡ Thunderous Lightning strikes for +8 bonus damage!");
            }
        }

        currentEnemy.hp -= dmg;
        if (currentEnemy.hp <= 0) {
            currentEnemy.hp = 0;
            CombatVictory();
            return;
        }

        TriggerCompanionCombatTurn();
        if (currentEnemy.hp <= 0) {
            currentEnemy.hp = 0;
            CombatVictory();
            return;
        }

        EnemyTurn();
    } else if (gameState == STATE_GAME_OVER) {
        InitHero(selectedClassIndex);
        gameState = STATE_CHAR_CREATE;
        LogMessage("--- New Journey Initialized ---");
        SetupButtons();
        UpdateUI();
    }
}

void HandleButton2() {
    if (gameState == STATE_UTILS) {
        ExportScoreJSON();
        return;
    }
    if (gameState == STATE_CONFIG) {
        g_BindingAction = 1;
        LogMessage("Press any key to bind Action 2...");
        return;
    }

    if (gameState == STATE_TAVERN) { LogMessage("Barkeep: 'Welcome! The Ruined Castle is dangerous!'"); return; }
    if (gameState == STATE_TAVERN) { LogMessage("Rumor: 'Equipment can be upgraded at the forge now.'"); return; }
    if (gameState == STATE_HELP) {
        g_HelpTab = 1;
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_ACHIEVEMENTS) {
        gameState = STATE_HELP;
        g_HelpTab = 0;
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_SAVE_LOAD) {
        SaveToSlot(g_SelectedSaveSlot);
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_INVENTORY) {
        if (player.invCount > 0 && player.selectedInvIdx >= 0 && player.selectedInvIdx < player.invCount) {
            InvItem* it = &player.inventory[player.selectedInvIdx];
            if (it->type == 0) {
                if (ContainsSubstr(it->name, "Health")) {
                    player.hp = (player.hp + 35 > player.maxHp) ? player.maxHp : player.hp + 35;
                    LogMessage("🧪 Drank Health Potion (+35 HP)!");
                } else if (ContainsSubstr(it->name, "Mana")) {
                    player.mp = (player.mp + 25 > player.maxMp) ? player.maxMp : player.mp + 25;
                    LogMessage("🧪 Drank Mana Potion (+25 MP)!");
                } else if (ContainsSubstr(it->name, "Greater")) {
                    player.hp = (player.hp + 70 > player.maxHp) ? player.maxHp : player.hp + 70;
                    LogMessage("🧪 Drank Greater HP Elixir (+70 HP)!");
                } else if (ContainsSubstr(it->name, "Might")) {
                    player.mp = (player.mp + 40 > player.maxMp) ? player.maxMp : player.mp + 40;
                    player.str += 3;
                    LogMessage("⚡ Drank Elixir of Might (+40 MP & +3 STR)!");
                }
                it->count--;
                if (it->count <= 0) RemoveInvItem(player.selectedInvIdx);
            } else if (it->type == 1) {
                if (it->eqType == 1) {
                    char oldName[32]; int oldVal = player.weaponBonusStr;
                    lstrcpyA(oldName, player.weaponName);
                    lstrcpyA(player.weaponName, it->name);
                    player.weaponBonusStr = it->bonusStr;
                    char emsg[128]; wsprintfA(emsg, "⚔️ Equipped weapon %s (+%d STR)!", it->name, it->bonusStr);
                    LogMessage(emsg);
                    it->count--;
                    if (it->count <= 0) RemoveInvItem(player.selectedInvIdx);
                    AddInvItem(oldName, 1, 1, oldVal, 0, 0, 1, 15, "Previous weapon");
                } else if (it->eqType == 2) {
                    char oldName[32]; int oldVal = player.armorBonusDef;
                    lstrcpyA(oldName, player.armorName);
                    lstrcpyA(player.armorName, it->name);
                    player.armorBonusDef = it->bonusDef;
                    char emsg[128]; wsprintfA(emsg, "🛡️ Equipped armor %s (+%d DEF)!", it->name, it->bonusDef);
                    LogMessage(emsg);
                    it->count--;
                    if (it->count <= 0) RemoveInvItem(player.selectedInvIdx);
                    AddInvItem(oldName, 1, 2, 0, oldVal, 0, 1, 15, "Previous armor");
                }
            }
        }
        UpdateUI();
        return;
    }
    if (gameState == STATE_BOSS_RUSH) {
        if (player.arenaTokens >= 5) {
            player.arenaTokens -= 5;
            lstrcpyA(player.weaponName, "Conqueror's Blade");
            player.weaponBonusStr = 15;
            Beep(880, 150);
            LogMessage("🗡️ Purchased Conqueror's Blade (+15 STR) with 5 Arena Trophies!");
            UpdateUI();
        } else {
            LogMessage("Need 5 Arena Trophies for Conqueror's Blade!");
        }
        return;
    }
    if (gameState == STATE_TRAINING_HALL) {
        if (player.skillPoints > 0 && player.defensePoints < 3) {
            player.defensePoints++;
            player.skillPoints--;
            player.maxHp += 10; player.hp += 10;
            LogMessage("🛡️ Allocated 1 SP into Defense Tree (+10 Max HP & +3 DEF)!");
            if (player.defensePoints == 3) LogMessage("🔥 UNLOCKED: Iron Will (50% Damage Reduction Barrier, 8 MP)!");
            SetupButtons();
            UpdateUI();
        } else if (player.defensePoints >= 3) {
            LogMessage("Defense Tree already at max rank!");
        } else {
            LogMessage("No Skill Points available! Level up to earn more.");
        }
        return;
    }
    if (gameState == STATE_QUEST_BOARD) {
        AcceptBounty(1);
        return;
    }
    if (gameState == STATE_MAGIC_MENU) {
        gameState = STATE_COMBAT;
        CastLightningStorm();
        UpdateUI();
    } else if (gameState == STATE_CHAR_CREATE) {
        selectedClassIndex = 1;
        InitHero(1);
        LogMessage("Selected Class: Mage (High Mana & Spell Power).");
        UpdateUI();
    } else if (gameState == STATE_TOWN) {
        gameState = STATE_BOSS_RUSH;
        Beep(440, 100); Beep(554, 100); Beep(659, 150);
        LogMessage("🏟️ Entered Grand Colosseum - Boss Rush Arena!");
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_MERCENARY) {
        if (player.companion.active > 0) {
            LogMessage("Dismissed your party companion.");
            player.companion.active = 0;
            SetupButtons();
            UpdateUI();
        } else if (player.gold >= 100) {
            player.gold -= 100;
            player.companion.active = 2; // Archmage
            lstrcpyA(player.companion.name, "Lady Pyra");
            lstrcpyA(player.companion.role, "Archmage DPS");
            player.companion.level = player.level;
            player.companion.maxHp = 45 + (player.level - 1) * 12;
            player.companion.hp = player.companion.maxHp;
            player.companion.upkeep = 15;
            player.companion.cost = 100;
            player.companion.isDown = 0;
            LogMessage("🔮 Hired Archmage DPS Lady Pyra! Arcane Intellect Active (+5 INT)!");
            UnlockAchievement(4); // Mercenary Leader
            SetupButtons();
            UpdateUI();
        } else {
            LogMessage("Need 100 Gold to hire Archmage DPS!");
        }
    } else if (gameState == STATE_SHOP) {
        if (player.gold >= 15) {
            player.gold -= 15;
            player.mpPotions++;
            LogMessage("Bought 1x Mana Potion (+25 MP) for 15 Gold.");
            UpdateUI();
        } else {
            LogMessage("Not enough gold!");
        }
    } else if (gameState == STATE_CRAFTING) {
        if (player.gold >= 30 && player.ironScrap >= 3) {
            player.gold -= 30;
            player.ironScrap -= 3;
            player.weaponBonusStr += 2;
            player.armorBonusDef += 2;
            SfxSpellCast();
            LogMessage("⬆️ Upgraded Gear! Weapon STR +2, Armor DEF +2!");
            UpdateUI();
        } else {
            LogMessage("Need 30 Gold and 3 Iron Scrap to Upgrade Gear!");
        }
    } else if (gameState == STATE_DUNGEON) {
        player.floor++;
        SfxDoorOpen();
        PayCompanionUpkeep();

        for (int i = 0; i < g_ActiveBountyCount; i++) {
            if (!g_ActiveBounties[i].done && g_ActiveBounties[i].type == 1) { // floor
                if (player.floor > g_ActiveBounties[i].current) g_ActiveBounties[i].current = player.floor;
                if (g_ActiveBounties[i].current >= g_ActiveBounties[i].req) {
                    g_ActiveBounties[i].done = 1;
                    char bmsg[128];
                    wsprintfA(bmsg, "📜 Bounty Completed: %s! Claim at Town Quest Board!", g_ActiveBounties[i].title);
                    LogMessage(bmsg);
                }
            }
        }

        char msg[128];
        wsprintfA(msg, "🪜 Descended to Floor %d of %s.", player.floor, g_Biomes[player.biome].name);
        LogMessage(msg);
        UpdateUI();
    } else if (gameState == STATE_COMBAT) {
        gameState = STATE_MAGIC_MENU;
        SetupButtons();
        UpdateUI();
    }
}

void HandleButton3() {
    if (gameState == STATE_UTILS) {
        ImportScoreJSON();
        return;
    }
    if (gameState == STATE_CONFIG) {
        g_BindingAction = 2;
        LogMessage("Press any key to bind Action 3...");
        return;
    }

    if (gameState == STATE_TAVERN) { LogMessage("Barkeep: 'Welcome! The Ruined Castle is dangerous!'"); return; }
    if (gameState == STATE_TAVERN) { LogMessage("Rumor: 'Equipment can be upgraded at the forge now.'"); return; }
    if (gameState == STATE_HELP) {
        g_HelpTab = 2;
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_SAVE_LOAD) {
        LoadFromSlot(g_SelectedSaveSlot);
        return;
    }
    if (gameState == STATE_INVENTORY) {
        if (player.invCount > 0 && player.selectedInvIdx >= 0 && player.selectedInvIdx < player.invCount) {
            InvItem* it = &player.inventory[player.selectedInvIdx];
            player.gold += it->value;
            char smsg[128];
            wsprintfA(smsg, "💰 Sold 1x %s for %d Gold.", it->name, it->value);
            LogMessage(smsg);
            it->count--;
            if (it->count <= 0) RemoveInvItem(player.selectedInvIdx);
        }
        UpdateUI();
        return;
    }
    if (gameState == STATE_BOSS_RUSH) {
        if (player.arenaTokens >= 5) {
            player.arenaTokens -= 5;
            lstrcpyA(player.armorName, "Champion Plate");
            player.armorBonusDef = 14;
            Beep(880, 150);
            LogMessage("🛡️ Purchased Champion Plate (+14 DEF) with 5 Arena Trophies!");
            UpdateUI();
        } else {
            LogMessage("Need 5 Arena Trophies for Champion Plate!");
        }
        return;
    }
    if (gameState == STATE_TRAINING_HALL) {
        if (player.skillPoints > 0 && player.utilityPoints < 3) {
            player.utilityPoints++;
            player.skillPoints--;
            player.maxMp += 10; player.mp += 10;
            LogMessage("⚡ Allocated 1 SP into Utility Tree (+10 Max MP & Loot Boost)!");
            if (player.utilityPoints == 3) LogMessage("🔥 UNLOCKED: Mana Surge (Free Mana Restore & Spell Boost, 0 MP)!");
            SetupButtons();
            UpdateUI();
        } else if (player.utilityPoints >= 3) {
            LogMessage("Utility Tree already at max rank!");
        } else {
            LogMessage("No Skill Points available! Level up to earn more.");
        }
        return;
    }
    if (gameState == STATE_QUEST_BOARD) {
        AcceptBounty(2);
        return;
    }
    if (gameState == STATE_MAGIC_MENU) {
        gameState = STATE_COMBAT;
        CastHolyShield();
        UpdateUI();
    } else if (gameState == STATE_CHAR_CREATE) {
        selectedClassIndex = 2;
        InitHero(2);
        LogMessage("Selected Class: Rogue (High Agility & Crits).");
        UpdateUI();
    } else if (gameState == STATE_TOWN) {
        player.biome = (player.biome + 1) % 15;
        player.floor = 1;
        SfxDoorOpen();
        char msg[128];
        wsprintfA(msg, "MAP Selected Dungeon Biome: %s (Hazard: %s)", g_Biomes[player.biome].name, g_Biomes[player.biome].hazardName);
        LogMessage(msg);
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_MERCENARY) {
        if (player.gold >= 70) {
            player.gold -= 70;
            player.companion.active = 3; // Cleric
            lstrcpyA(player.companion.name, "Brother Tobias");
            lstrcpyA(player.companion.role, "Cleric Healer");
            player.companion.level = player.level;
            player.companion.maxHp = 55 + (player.level - 1) * 12;
            player.companion.hp = player.companion.maxHp;
            player.companion.upkeep = 8;
            player.companion.cost = 70;
            player.companion.isDown = 0;
            LogMessage("✨ Hired Cleric Healer Brother Tobias! Blessed Grace Active (+10 Max HP)!");
            UnlockAchievement(4); // Mercenary Leader
            SetupButtons();
            UpdateUI();
        } else {
            LogMessage("Need 70 Gold to hire Cleric Healer!");
        }
    } else if (gameState == STATE_SHOP) {
        if (player.gold >= 60) {
            player.gold -= 60;
            lstrcpyA(player.weaponName, "Steel Longsword");
            player.weaponBonusStr = 8;
            LogMessage("Equipped Steel Longsword (+8 STR)!");
            UpdateUI();
        } else {
            LogMessage("Not enough gold!");
        }
    } else if (gameState == STATE_CRAFTING) {
        if (player.gold >= 30 && player.ironScrap >= 3) {
            player.gold -= 30;
            player.ironScrap -= 3;
            player.weaponBonusStr += 2;
            player.armorBonusDef += 2;
            SfxSpellCast();
            LogMessage("⬆️ Upgraded Gear! Weapon STR +2, Armor DEF +2!");
            UpdateUI();
        } else {
            LogMessage("Need 30 Gold and 3 Iron Scrap to Upgrade Gear!");
        }
    } else if (gameState == STATE_DUNGEON || gameState == STATE_COMBAT) {
        if (player.greaterHpPotions > 0) {
            player.greaterHpPotions--;
            player.hp = (player.hp + 70 > player.maxHp) ? player.maxHp : player.hp + 70;
            SfxSpellCast();
            LogMessage("🧪 Drank Greater HP Elixir (+70 HP)!");
            UpdateUI();
            if (gameState == STATE_COMBAT) EnemyTurn();
        } else if (player.hpPotions > 0) {
            player.hpPotions--;
            player.hp = (player.hp + 35 > player.maxHp) ? player.maxHp : player.hp + 35;
            SfxSpellCast();
            LogMessage("🧪 Drank Health Potion (+35 HP)!");
            UpdateUI();
            if (gameState == STATE_COMBAT) EnemyTurn();
        } else {
            LogMessage("No Health Potions remaining!");
        }
    }
}

void HandleButton4() {
    if (gameState == STATE_UTILS) {
        gameState = STATE_REPLAYS;
        LogMessage("🎥 Match Replay Viewer Opened.");
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_CONFIG) {
        g_BindingAction = 3;
        LogMessage("Press any key to bind Action 4...");
        return;
    }

    if (gameState == STATE_TAVERN) { LogMessage("Barkeep: 'Welcome! The Ruined Castle is dangerous!'"); return; }
    if (gameState == STATE_TAVERN) { LogMessage("Rumor: 'Equipment can be upgraded at the forge now.'"); return; }
    if (gameState == STATE_HELP) {
        g_HelpTab = 3;
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_SAVE_LOAD) {
        DeleteSlot(g_SelectedSaveSlot);
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_INVENTORY) {
        QuickSellCommons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_BOSS_RUSH) {
        if (player.arenaTokens >= 8) {
            player.arenaTokens -= 8;
            player.str += 8;
            player.intStat += 8;
            player.agi += 8;
            Beep(880, 150);
            LogMessage("💍 Equipping Gladiator Ring boosted STR, INT, & AGI by +8!");
            UpdateUI();
        } else {
            LogMessage("Need 8 Arena Trophies for Gladiator Ring!");
        }
        return;
    }
    if (gameState == STATE_TRAINING_HALL) {
        int total = player.offensePoints + player.defensePoints + player.utilityPoints;
        if (total == 0) {
            LogMessage("No skills allocated to respec!");
            return;
        }
        if (player.gold >= 30) {
            player.gold -= 30;
            player.skillPoints += total;
            player.maxHp -= player.defensePoints * 10;
            if (player.hp > player.maxHp) player.hp = player.maxHp;
            player.maxMp -= player.utilityPoints * 10;
            if (player.mp > player.maxMp) player.mp = player.maxMp;
            player.offensePoints = 0;
            player.defensePoints = 0;
            player.utilityPoints = 0;
            LogMessage("🔄 Respec completed! Refunded Skill Points for 30 Gold.");
            SetupButtons();
            UpdateUI();
        } else {
            LogMessage("Need 30 Gold to respec skills!");
        }
        return;
    }
    if (gameState == STATE_QUEST_BOARD) {
        AcceptBounty(3);
        return;
    }
    if (gameState == STATE_CHAR_CREATE) {
        LogMessage("✨ Character created! Welcome to Oakhaven Town.");
        gameState = STATE_TOWN;
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_TOWN) {
        if (player.gold >= 10) {
            player.gold -= 10;
            player.hp = player.maxHp;
            player.mp = player.maxMp;
            if (player.companion.active > 0) {
                player.companion.isDown = 0;
                player.companion.hp = player.companion.maxHp;
            }
            LogMessage("🍺 Rested at Dragon's Rest Inn. HP, MP, & Party fully restored!");
            UpdateUI();
        } else {
            gameState = STATE_SHOP;
            LogMessage("Entered Oakhaven Shop.");
            SetupButtons();
            UpdateUI();
        }
    } else if (gameState == STATE_MERCENARY) {
        if (player.companion.active > 0) {
            if (player.companion.isDown) {
                if (player.gold >= 20) {
                    player.gold -= 20;
                    player.companion.isDown = 0;
                    player.companion.hp = player.companion.maxHp;
                    LogMessage("✨ Revived companion! Health fully restored.");
                    SetupButtons();
                    UpdateUI();
                } else {
                    LogMessage("Need 20 Gold to revive companion!");
                }
            } else {
                char msg[128];
                wsprintfA(msg, "Dismissed companion %s.", player.companion.name);
                LogMessage(msg);
                player.companion.active = 0;
                SetupButtons();
                UpdateUI();
            }
        } else {
            LogMessage("No active companion to dismiss or revive.");
        }
    } else if (gameState == STATE_SHOP) {
        if (player.gold >= 75) {
            player.gold -= 75;
            lstrcpyA(player.armorName, "Plate Armor");
            player.armorBonusDef = 9;
            LogMessage("Equipped Plate Armor (+9 DEF)!");
            UpdateUI();
        } else {
            LogMessage("Not enough gold!");
        }
    } else if (gameState == STATE_CRAFTING) {
        if (player.gold >= 30 && player.ironScrap >= 3) {
            player.gold -= 30;
            player.ironScrap -= 3;
            player.weaponBonusStr += 2;
            player.armorBonusDef += 2;
            SfxSpellCast();
            LogMessage("⬆️ Upgraded Gear! Weapon STR +2, Armor DEF +2!");
            UpdateUI();
        } else {
            LogMessage("Need 30 Gold and 3 Iron Scrap to Upgrade Gear!");
        }
    } else if (gameState == STATE_DUNGEON) {
        if (player.mpPotions > 0) {
            player.mpPotions--;
            player.mp = (player.mp + 25 > player.maxMp) ? player.maxMp : player.mp + 25;
            LogMessage("🧪 Drank Mana Potion (+25 MP)!");
            UpdateUI();
        } else {
            LogMessage("No Mana Potions remaining!");
        }
    } else if (gameState == STATE_COMBAT) {
        if (player.fireBombs > 0) {
            player.fireBombs--;
            currentEnemy.hp -= 45;
            LogMessage("💣 FIRE BOMB EXPLOSION! Threw a Fire Bomb dealing 45 fire damage!");
            UpdateUI();
            if (currentEnemy.hp <= 0) {
                currentEnemy.hp = 0;
                CombatVictory();
            } else {
                EnemyTurn();
            }
        } else if (player.mpPotions > 0) {
            player.mpPotions--;
            player.mp = (player.mp + 25 > player.maxMp) ? player.maxMp : player.mp + 25;
            LogMessage("🧪 Drank Mana Potion (+25 MP)!");
            UpdateUI();
            EnemyTurn();
        } else {
            LogMessage("No Fire Bombs or Mana Potions!");
        }
    }
}

void HandleButton5() {
    if (gameState == STATE_UTILS) {
        gameState = STATE_CONFIG;
        LogMessage("⌨️ Keybinds Configuration Opened.");
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_CONFIG) {
        g_BindingAction = 4;
        LogMessage("Press any key to bind Action 5...");
        return;
    }

    if (gameState == STATE_TAVERN) { LogMessage("Barkeep: 'Welcome! The Ruined Castle is dangerous!'"); return; }
    if (gameState == STATE_TAVERN) { LogMessage("Rumor: 'Equipment can be upgraded at the forge now.'"); return; }
    if (gameState == STATE_HELP) {
        g_HelpTab = (g_HelpTab + 1) % 4;
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_SAVE_LOAD) {
        SaveToSlot(0);
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_ACHIEVEMENTS) {
        EnterNewGamePlus();
        return;
    }
    if (gameState == STATE_INVENTORY) {
        ExpandBackpackSlots();
        UpdateUI();
        return;
    }
    if (gameState == STATE_BOSS_RUSH) {
        if (player.arenaTokens >= 4) {
            player.arenaTokens -= 4;
            player.maxHp += 30; player.hp += 30;
            player.maxMp += 20; player.mp += 20;
            Beep(880, 150);
            LogMessage("🧪 Drank Elixir of Valor! Permanently gained +30 Max HP & +20 Max MP!");
            UpdateUI();
        } else {
            LogMessage("Need 4 Arena Trophies for Elixir of Valor!");
        }
        return;
    }
    if (gameState == STATE_TRAINING_HALL) {
        gameState = STATE_MERCENARY;
        LogMessage("Entered Mercenary Guild.");
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_QUEST_BOARD) {
        ClaimAllCompletedBounties();
        return;
    }
    if (gameState == STATE_TOWN) {
        gameState = STATE_SAVE_LOAD;
        LogMessage("💾 Opened Save / Load Game Manager.");
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_MERCENARY) {
        int claimed = 0;
        if (player.questMonstersKilled >= 5 && !player.questMonstersDone) {
            player.questMonstersDone = 1;
            player.gold += 50;
            LogMessage("📜 Quest Completed: Clear Dungeon Chambers! +50 Gold!");
            claimed++;
        }
        if (player.questBossKilled >= 1 && !player.questBossDone) {
            player.questBossDone = 1;
            player.gold += 150;
            LogMessage("📜 Quest Completed: Slay Biome Boss! +150 Gold!");
            claimed++;
        }
        if (claimed == 0) {
            LogMessage("No completed quest rewards to claim right now.");
        }
        UpdateUI();
    } else if (gameState == STATE_SHOP) {
        gameState = STATE_TOWN;
        LogMessage("Returned to Town.");
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_CRAFTING) {
        // Imbue Equipment - Cycles through enchantments
        if (lstrcmpA(player.weaponPrefix, "Flaming") != 0 && player.elementalCore >= 2 && player.arcaneDust >= 1) {
            player.elementalCore -= 2;
            player.arcaneDust -= 1;
            lstrcpyA(player.weaponPrefix, "Flaming");
            SfxSpellCast();
            LogMessage("🔥 Imbued weapon with Flaming Enchantment (+6 Fire Dmg)!");
        } else if (lstrcmpA(player.weaponPrefix, "Vampiric") != 0 && player.arcaneDust >= 2 && player.ironScrap >= 1) {
            player.arcaneDust -= 2;
            player.ironScrap -= 1;
            lstrcpyA(player.weaponPrefix, "Vampiric");
            SfxSpellCast();
            LogMessage("🩸 Imbued weapon with Vampiric Enchantment (25% Lifesteal)!");
        } else if (lstrcmpA(player.armorPrefix, "Fortified") != 0 && player.ironScrap >= 2 && player.arcaneDust >= 1) {
            player.ironScrap -= 2;
            player.arcaneDust -= 1;
            lstrcpyA(player.armorPrefix, "Fortified");
            player.armorBonusDef += 5;
            player.maxHp += 20; player.hp += 20;
            SfxSpellCast();
            LogMessage("🛡️ Imbued armor with Fortified Enchantment (+5 DEF, +20 Max HP)!");
        } else if (lstrcmpA(player.armorPrefix, "Spiked") != 0 && player.ironScrap >= 2 && player.elementalCore >= 1) {
            player.ironScrap -= 2;
            player.elementalCore -= 1;
            lstrcpyA(player.armorPrefix, "Spiked");
            player.armorBonusDef += 3;
            SfxSpellCast();
            LogMessage("🌵 Imbued armor with Spiked Enchantment (Reflects 35% damage)!");
        } else {
            LogMessage("Need materials for next imbuing tier (e.g. 2 Cores/Dust & 1 Scrap/Dust)!");
        }
        UpdateUI();
    } else if (gameState == STATE_DUNGEON) {
        gameState = STATE_TOWN;
        LogMessage("Returned safely to Oakhaven Town.");
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_COMBAT) {
        if ((xrand() % 100) < 60) {
            LogMessage("🏃 Fled safely from combat!");
            if (player.arenaActive) {
                player.arenaActive = 0;
                gameState = STATE_BOSS_RUSH;
            } else {
                gameState = STATE_DUNGEON;
            }
            SetupButtons();
            UpdateUI();
        } else {
            LogMessage("Failed to flee!");
            EnemyTurn();
        }
    }
}

void HandleButton6() {
    if (gameState == STATE_TOWN) {
        gameState = STATE_UTILS;
        LogMessage("⚙️ Opened System Utilities.");
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_UTILS || gameState == STATE_REPLAYS || gameState == STATE_CONFIG) {
        gameState = STATE_TOWN;
        LogMessage("Returned to Town Square.");
        SetupButtons();
        UpdateUI();
        return;
    }

    if (gameState == STATE_SAVE_LOAD || gameState == STATE_ACHIEVEMENTS || gameState == STATE_HELP) {
        gameState = STATE_TOWN;
        LogMessage("Returned to Town Square.");
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_INVENTORY) {
        gameState = STATE_TOWN;
        LogMessage("Returned to Town Square.");
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_BOSS_RUSH) {
        gameState = STATE_TOWN;
        LogMessage("Returned to Town Square.");
        SetupButtons();
        UpdateUI();
        return;
    }

    if (gameState == STATE_QUEST_BOARD) {
        gameState = STATE_TOWN;
        LogMessage("Returned to Town Square.");
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_MERCENARY) {
        gameState = STATE_TOWN;
        LogMessage("Returned to Town Square.");
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_CRAFTING) {
        gameState = STATE_TOWN;
        LogMessage("Returned to Town Square.");
        SetupButtons();
        UpdateUI();
        return;
    }
    if (gameState == STATE_COMBAT) {
        if (player.offensePoints >= 3) {
            // Execute Ability
            if (player.mp >= 12) {
                player.mp -= 12;
                SfxCombatHit();
                int offMult = 100 + (player.offensePoints * 8);
                int totalStr = player.str + player.weaponBonusStr;
                int dmg = (int)(totalStr * 3 * offMult / 100);
                if (currentEnemy.hp < (currentEnemy.maxHp * 40 / 100)) {
                    dmg *= 2;
                    char emsg[128];
                    wsprintfA(emsg, "💀 EXECUTE CRITICAL! Target HP < 40%! Dealt %d fatal physical damage to %s!", dmg, currentEnemy.name);
                    LogMessage(emsg);
                } else {
                    char emsg[128];
                    wsprintfA(emsg, "⚔️ Executed heavy strike dealing %d physical damage to %s!", dmg, currentEnemy.name);
                    LogMessage(emsg);
                }
                currentEnemy.hp -= dmg;
                if (currentEnemy.hp <= 0) {
                    currentEnemy.hp = 0;
                    CombatVictory();
                    return;
                }
                TriggerCompanionCombatTurn();
                if (currentEnemy.hp <= 0) {
                    currentEnemy.hp = 0;
                    CombatVictory();
                    return;
                }
                EnemyTurn();
            } else {
                LogMessage("Not enough MP for Execute (Requires 12 MP)!");
            }
        } else if (player.defensePoints >= 3) {
            // Iron Will Ability
            if (player.mp >= 8) {
                player.mp -= 8;
                SfxSpellCast();
                player.ironWillTurns = 2;
                player.hp += 20;
                if (player.hp > player.maxHp) player.hp = player.maxHp;
                LogMessage("🛡️ Activated IRON WILL! Restored +20 HP and granted 50% damage reduction for 2 turns!");
                UpdateUI();
                EnemyTurn();
            } else {
                LogMessage("Not enough MP for Iron Will (Requires 8 MP)!");
            }
        } else if (player.utilityPoints >= 3) {
            // Mana Surge Ability
            player.mp += 35;
            if (player.mp > player.maxMp) player.mp = player.maxMp;
            player.manaSurgeActive = 1;
            SfxSpellCast();
            LogMessage("⚡ Activated MANA SURGE! Recovered +35 MP! Next spell power boosted by +50%!");
            UpdateUI();
            SetupButtons();
        } else {
            LogMessage("No Specialization Mastery Ability unlocked yet!");
        }
        return;
    }
}

// --- GRAPHICS & ANIMATION ENGINE (WIN32 GDI) ---
static int g_GfxFrame = 0;
static HWND hGfxCanvas = NULL;
static WNDPROC g_OldCanvasProc = NULL;

typedef struct {
    char text[32];
    float x, y, vx, vy;
    COLORREF color;
    int life, maxLife;
    int isCrit;
} GdiFloatText;

static GdiFloatText g_GdiFloatTexts[16];
static int g_GdiFloatCount = 0;

void AddGdiFloatText(const char* text, int x, int y, COLORREF color, int isCrit) {
    if (g_GdiFloatCount < 16) {
        lstrcpyA(g_GdiFloatTexts[g_GdiFloatCount].text, text);
        g_GdiFloatTexts[g_GdiFloatCount].x = (float)x;
        g_GdiFloatTexts[g_GdiFloatCount].y = (float)y;
        g_GdiFloatTexts[g_GdiFloatCount].vx = (float)((xrand() % 9) - 4) * (isCrit ? 1.5f : 0.5f);
        g_GdiFloatTexts[g_GdiFloatCount].vy = isCrit ? -8.0f : -5.0f;
        g_GdiFloatTexts[g_GdiFloatCount].color = color;
        g_GdiFloatTexts[g_GdiFloatCount].life = 0;
        g_GdiFloatTexts[g_GdiFloatCount].maxLife = 40;
        g_GdiFloatTexts[g_GdiFloatCount].isCrit = isCrit;
        g_GdiFloatCount++;
    }
}

typedef struct {
    int x, y, vx, vy;
    COLORREF color;
    int life, maxLife;
} GdiParticle;

static GdiParticle g_GdiParticles[32];
static int g_GdiParticleCount = 0;

void AddGdiParticles(int x, int y, COLORREF color, int count) {
    for (int i = 0; i < count && g_GdiParticleCount < 32; i++) {
        g_GdiParticles[g_GdiParticleCount].x = x;
        g_GdiParticles[g_GdiParticleCount].y = y;
        g_GdiParticles[g_GdiParticleCount].vx = (xrand() % 9) - 4;
        g_GdiParticles[g_GdiParticleCount].vy = (xrand() % 7) - 4;
        g_GdiParticles[g_GdiParticleCount].color = color;
        g_GdiParticles[g_GdiParticleCount].life = 0;
        g_GdiParticles[g_GdiParticleCount].maxLife = 20 + (xrand() % 15);
        g_GdiParticleCount++;
    }
}

static int g_GdiSpellFxType = 0;
static int g_GdiSpellFxProgress = 0;
static int g_GdiSpellFxMax = 20;

void TriggerGdiSpellFX(int type) {
    g_GdiSpellFxType = type;
    g_GdiSpellFxProgress = 0;
    if (type == 1) g_GdiSpellFxMax = 20;
    else if (type == 2) g_GdiSpellFxMax = 15;
    else g_GdiSpellFxMax = 25;
}

static int g_GdiBannerActive = 0;
static int g_GdiBannerTimer = 0;
static char g_GdiBannerTitle[32] = "VICTORY!";

static int g_GdiScreenShakeTimer = 0;
static int g_GdiScreenShakeIntensity = 0;
static int g_GdiRedFlashTimer = 0;

void TriggerGdiVictoryBanner(const char* title) {
    g_GdiBannerActive = 1;
    g_GdiBannerTimer = 0;
    lstrcpyA(g_GdiBannerTitle, title);
}

void DrawGdiHeroSprite(HDC hdc, int x, int y, const char* heroClass, int frame, const char* wpnName, const char* armName) {
    int bob = (frame % 8 < 4) ? 0 : 2;
    int sy = y + bob;
    
    COLORREF bladeC = RGB(205, 214, 244);
    COLORREF hiltC = RGB(249, 226, 175);
    if (ContainsSubstr(wpnName, "Wood")) { bladeC = RGB(210, 105, 30); hiltC = RGB(140, 74, 23); }
    else if (ContainsSubstr(wpnName, "Gold")) { bladeC = RGB(249, 226, 175); hiltC = RGB(243, 139, 168); }
    else if (ContainsSubstr(wpnName, "Diamond")) { bladeC = RGB(137, 220, 235); hiltC = RGB(249, 226, 175); }

    COLORREF armC = RGB(137, 180, 250);
    COLORREF trimC = RGB(249, 226, 175);
    if (ContainsSubstr(armName, "Leather") || ContainsSubstr(armName, "Wood")) { armC = RGB(166, 124, 82); trimC = RGB(230, 195, 135); }
    else if (ContainsSubstr(armName, "Gold")) { armC = RGB(249, 226, 175); trimC = RGB(243, 139, 168); }
    else if (ContainsSubstr(armName, "Diamond")) { armC = RGB(137, 220, 235); trimC = RGB(205, 214, 244); }
    int sy = y + bob;

    HBRUSH hShB = CreateSolidBrush(RGB(15, 15, 20));
    HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hShB);
    Ellipse(hdc, x - 18, y + 30, x + 18, y + 40);
    SelectObject(hdc, hOldB); DeleteObject(hShB);

    if (lstrcmpA(heroClass, "Mage") == 0) {
        HBRUSH hRobeB = CreateSolidBrush(RGB(203, 166, 247));
        SelectObject(hdc, hRobeB);
        POINT pts[4] = {{x - 14, sy - 10}, {x + 14, sy - 10}, {x + 18, sy + 32}, {x - 18, sy + 32}};
        Polygon(hdc, pts, 4);
        DeleteObject(hRobeB);

        HBRUSH hFaceB = CreateSolidBrush(RGB(245, 224, 220));
        SelectObject(hdc, hFaceB);
        Ellipse(hdc, x - 8, sy - 24, x + 8, sy - 8);
        DeleteObject(hFaceB);

        HBRUSH hHatB = CreateSolidBrush(RGB(180, 190, 254));
        SelectObject(hdc, hHatB);
        POINT hpts[3] = {{x - 16, sy - 22}, {x + 16, sy - 22}, {x, sy - 48}};
        Polygon(hdc, hpts, 3);
        DeleteObject(hHatB);

        HPEN hStP = CreatePen(PS_SOLID, 3, RGB(250, 179, 135));
        HPEN hOldP = (HPEN)SelectObject(hdc, hStP);
        MoveToEx(hdc, x + 16, sy - 25, NULL); LineTo(hdc, x + 16, sy + 32);
        SelectObject(hdc, hOldP); DeleteObject(hStP);

        HBRUSH hOrbB = CreateSolidBrush(RGB(137, 220, 235));
        SelectObject(hdc, hOrbB);
        Ellipse(hdc, x + 11, sy - 35, x + 21, sy - 25);
        DeleteObject(hOrbB);

    } else if (lstrcmpA(heroClass, "Rogue") == 0) {
        HBRUSH hCloakB = CreateSolidBrush(RGB(49, 50, 68));
        SelectObject(hdc, hCloakB);
        RECT rR = {x - 12, sy - 10, x + 12, sy + 32};
        FillRect(hdc, &rR, hCloakB);
        DeleteObject(hCloakB);

        HBRUSH hVestB = CreateSolidBrush(RGB(166, 227, 161));
        SelectObject(hdc, hVestB);
        RECT vR = {x - 8, sy - 8, x + 8, sy + 18};
        FillRect(hdc, &vR, hVestB);
        DeleteObject(hVestB);

        HBRUSH hHoodB = CreateSolidBrush(RGB(30, 30, 46));
        SelectObject(hdc, hHoodB);
        Ellipse(hdc, x - 10, sy - 25, x + 10, sy - 5);
        DeleteObject(hHoodB);

        HPEN hDagP = CreatePen(PS_SOLID, 2, RGB(148, 226, 213));
        HPEN hOldP = (HPEN)SelectObject(hdc, hDagP);
        MoveToEx(hdc, x - 16, sy - 2, NULL); LineTo(hdc, x - 16, sy + 20);
        MoveToEx(hdc, x + 16, sy - 2, NULL); LineTo(hdc, x + 16, sy + 20);
        SelectObject(hdc, hOldP); DeleteObject(hDagP);

    } else { // Warrior
        HBRUSH hCapeB = CreateSolidBrush(RGB(243, 139, 168));
        SelectObject(hdc, hCapeB);
        POINT cpts[3] = {{x - 10, sy - 5}, {x - 20, sy + 28}, {x - 2, sy + 30}};
        Polygon(hdc, cpts, 3);
        DeleteObject(hCapeB);

        HBRUSH hArmB = CreateSolidBrush(armC);
        SelectObject(hdc, hArmB);
        RECT aR = {x - 12, sy - 10, x + 12, sy + 22};
        FillRect(hdc, &aR, hArmB);
        DeleteObject(hArmB);

        HBRUSH hHelmB = CreateSolidBrush(RGB(180, 190, 254));
        SelectObject(hdc, hHelmB);
        RECT hR = {x - 10, sy - 28, x + 10, sy - 10};
        FillRect(hdc, &hR, hHelmB);
        DeleteObject(hHelmB);

        HBRUSH hVisB = CreateSolidBrush(RGB(137, 220, 235));
        SelectObject(hdc, hVisB);
        RECT visR = {x - 5, sy - 20, x + 5, sy - 17};
        FillRect(hdc, &visR, hVisB);
        DeleteObject(hVisB);

        HBRUSH hShldB = CreateSolidBrush(trimC);
        SelectObject(hdc, hShldB);
        POINT spts[3] = {{x - 22, sy - 8}, {x - 8, sy - 8}, {x - 15, sy + 18}};
        Polygon(hdc, spts, 3);
        DeleteObject(hShldB);

        if (g_HeroActionTimer > 0) {
            HPEN hArcP = CreatePen(PS_SOLID, 8, RGB(255, 255, 255));
            HPEN hOldArc = (HPEN)SelectObject(hdc, hArcP);
            Arc(hdc, x + 5, sy - 30, x + 45, sy + 10, x + 25, sy - 30, x + 45, sy);
            SelectObject(hdc, hOldArc); DeleteObject(hArcP);
        }

        HPEN hSwP = CreatePen(PS_SOLID, 3, bladeC);
        HPEN hOldP = (HPEN)SelectObject(hdc, hSwP);
        if (g_HeroActionTimer > 0) {
            MoveToEx(hdc, x + 14, sy + 15, NULL); LineTo(hdc, x + 35, sy + 5);
        } else {
            MoveToEx(hdc, x + 14, sy + 15, NULL); LineTo(hdc, x + 14, sy - 20);
        }
        SelectObject(hdc, hOldP); DeleteObject(hSwP);
    }

    if (player.poisonedTurns > 0) {
        HBRUSH hP = CreateSolidBrush(RGB(166, 227, 161));
        HBRUSH hOP = (HBRUSH)SelectObject(hdc, hP);
        Ellipse(hdc, x - 5, sy + 25, x + 5, sy + 35);
        SelectObject(hdc, hOP); DeleteObject(hP);
    }
    if (player.burningTurns > 0) {
        HBRUSH hP = CreateSolidBrush(RGB(243, 139, 168));
        HBRUSH hOP = (HBRUSH)SelectObject(hdc, hP);
        Ellipse(hdc, x - 5, sy + 25, x + 5, sy + 35);
        SelectObject(hdc, hOP); DeleteObject(hP);
    }
    if (player.frozenTurns > 0) {
        HBRUSH hP = CreateSolidBrush(RGB(137, 220, 235));
        HBRUSH hOP = (HBRUSH)SelectObject(hdc, hP);
        RECT fR = {x - 25, sy - 10, x + 25, sy + 40};
        FrameRect(hdc, &fR, hP);
        SelectObject(hdc, hOP); DeleteObject(hP);
    }
}

void DrawGdiMonsterSprite(HDC hdc, int x, int y, const char* name, int frame, int poisonedTurns, int burningTurns, int frozenTurns) {
    int bob = (frame % 6 < 3) ? 0 : 3;
    int my = y + bob;

    HBRUSH hShB = CreateSolidBrush(RGB(15, 15, 20));
    HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hShB);
    Ellipse(hdc, x - 22, y + 32, x + 22, y + 42);
    SelectObject(hdc, hOldB); DeleteObject(hShB);

    if (ContainsSubstr(name, "Slime") || ContainsSubstr(name, "Spider") || ContainsSubstr(name, "Scuttler")) {
        HBRUSH hSlmB = CreateSolidBrush(RGB(166, 227, 161));
        SelectObject(hdc, hSlmB);
        Ellipse(hdc, x - 25, my - 10, x + 25, my + 30);
        DeleteObject(hSlmB);

        HBRUSH hEyeB = CreateSolidBrush(RGB(17, 17, 27));
        SelectObject(hdc, hEyeB);
        RECT e1 = {x - 8, my, x - 4, my + 6}; FillRect(hdc, &e1, hEyeB);
        RECT e2 = {x + 4, my, x + 8, my + 6}; FillRect(hdc, &e2, hEyeB);
        DeleteObject(hEyeB);

    } else if (ContainsSubstr(name, "Goblin") || ContainsSubstr(name, "Slinger") || ContainsSubstr(name, "Taskmaster")) {
        HBRUSH hGobB = CreateSolidBrush(RGB(166, 227, 161));
        SelectObject(hdc, hGobB);
        RECT bodyR = {x - 12, my - 5, x + 12, my + 25}; FillRect(hdc, &bodyR, hGobB);
        RECT headR = {x - 10, my - 22, x + 10, my - 5}; FillRect(hdc, &headR, hGobB);
        POINT e1[3] = {{x - 10, my - 20}, {x - 24, my - 16}, {x - 10, my - 10}}; Polygon(hdc, e1, 3);
        POINT e2[3] = {{x + 10, my - 20}, {x + 24, my - 16}, {x + 10, my - 10}}; Polygon(hdc, e2, 3);
        DeleteObject(hGobB);

        HBRUSH hClbB = CreateSolidBrush(RGB(249, 226, 175));
        SelectObject(hdc, hClbB);
        RECT cR = {x - 22, my - 15, x - 15, my + 20}; FillRect(hdc, &cR, hClbB);
        DeleteObject(hClbB);

    } else if (ContainsSubstr(name, "Skeleton") || ContainsSubstr(name, "Ghoul") || ContainsSubstr(name, "Bone") || ContainsSubstr(name, "Lich")) {
        HBRUSH hBoneB = CreateSolidBrush(RGB(230, 233, 239));
        SelectObject(hdc, hBoneB);
        RECT bR = {x - 8, my - 5, x + 8, my + 25}; FillRect(hdc, &bR, hBoneB);
        RECT sR = {x - 10, my - 25, x + 10, my - 5}; FillRect(hdc, &sR, hBoneB);
        DeleteObject(hBoneB);

        HBRUSH hSoulB = CreateSolidBrush(RGB(137, 220, 235));
        SelectObject(hdc, hSoulB);
        RECT e1 = {x - 6, my - 18, x - 2, my - 13}; FillRect(hdc, &e1, hSoulB);
        RECT e2 = {x + 2, my - 18, x + 6, my - 13}; FillRect(hdc, &e2, hSoulB);
        DeleteObject(hSoulB);

    } else if (ContainsSubstr(name, "Orc") || ContainsSubstr(name, "Berserk") || ContainsSubstr(name, "Ogre") || ContainsSubstr(name, "Grommash")) {
        HBRUSH hOrcB = CreateSolidBrush(RGB(64, 160, 43));
        SelectObject(hdc, hOrcB);
        RECT bR = {x - 18, my - 12, x + 18, my + 28}; FillRect(hdc, &bR, hOrcB);
        DeleteObject(hOrcB);

        HBRUSH hHelmB = CreateSolidBrush(RGB(49, 50, 68));
        SelectObject(hdc, hHelmB);
        RECT hR = {x - 12, my - 32, x + 12, my - 12}; FillRect(hdc, &hR, hHelmB);
        DeleteObject(hHelmB);

        HPEN hAxeP = CreatePen(PS_SOLID, 4, RGB(137, 180, 250));
        HPEN hOldP = (HPEN)SelectObject(hdc, hAxeP);
        MoveToEx(hdc, x + 22, my - 35, NULL); LineTo(hdc, x + 22, my + 25);
        SelectObject(hdc, hOldP); DeleteObject(hAxeP);

    } else if (ContainsSubstr(name, "Dragon") || ContainsSubstr(name, "Drake") || ContainsSubstr(name, "Wyrm") || ContainsSubstr(name, "Bahamut")) {
        HBRUSH hDraB = CreateSolidBrush(RGB(243, 139, 168));
        SelectObject(hdc, hDraB);
        Ellipse(hdc, x - 28, my - 15, x + 28, my + 25);
        POINT w1[3] = {{x - 10, my - 5}, {x - 45, my - 25}, {x - 20, my + 15}}; Polygon(hdc, w1, 3);
        POINT w2[3] = {{x + 10, my - 5}, {x + 45, my - 25}, {x + 20, my + 15}}; Polygon(hdc, w2, 3);
        DeleteObject(hDraB);

    } else {
        HBRUSH hDemB = CreateSolidBrush(RGB(30, 30, 46));
        SelectObject(hdc, hDemB);
        Ellipse(hdc, x - 26, my - 20, x + 26, my + 26);
        DeleteObject(hDemB);

        HPEN hPurpP = CreatePen(PS_SOLID, 3, RGB(203, 166, 247));
        HPEN hOldP = (HPEN)SelectObject(hdc, hPurpP);
        Ellipse(hdc, x - 32, my - 26, x + 32, my + 32);
        SelectObject(hdc, hOldP); DeleteObject(hPurpP);
    }

    if (poisonedTurns > 0) {
        HBRUSH hP = CreateSolidBrush(RGB(166, 227, 161));
        HBRUSH hOP = (HBRUSH)SelectObject(hdc, hP);
        Ellipse(hdc, x - 8, my - 35, x + 8, my - 25);
        SelectObject(hdc, hOP); DeleteObject(hP);
    }
    if (burningTurns > 0) {
        HBRUSH hP = CreateSolidBrush(RGB(243, 139, 168));
        HBRUSH hOP = (HBRUSH)SelectObject(hdc, hP);
        Ellipse(hdc, x - 8, my - 35, x + 8, my - 25);
        SelectObject(hdc, hOP); DeleteObject(hP);
    }
    if (frozenTurns > 0) {
        HBRUSH hP = CreateSolidBrush(RGB(137, 220, 235));
        HBRUSH hOP = (HBRUSH)SelectObject(hdc, hP);
        RECT fR = {x - 30, my - 30, x + 30, my + 40};
        FrameRect(hdc, &fR, hP);
        SelectObject(hdc, hOP); DeleteObject(hP);
    }
}

void DrawGdiNPCSprite(HDC hdc, int x, int y, int frame) {
    int bob = (frame % 8 < 4) ? 0 : 2;
    HBRUSH hNpcB = CreateSolidBrush(RGB(249, 226, 175));
    RECT nR = {x - 10, y - 8 + bob, x + 10, y + 25 + bob};
    FillRect(hdc, &nR, hNpcB);
    DeleteObject(hNpcB);

    HBRUSH hFaceB = CreateSolidBrush(RGB(245, 224, 220));
    SelectObject(hdc, hFaceB);
    Ellipse(hdc, x - 8, y - 22 + bob, x + 8, y - 8 + bob);
    DeleteObject(hFaceB);
}

void RenderGdiScene(HDC hdc, int w, int h) {
    g_GfxFrame++;

    int shakeX = 0, shakeY = 0;
    if (g_GdiScreenShakeTimer > 0) {
        g_GdiScreenShakeTimer--;
        shakeX = (xrand() % (g_GdiScreenShakeIntensity * 2 + 1)) - g_GdiScreenShakeIntensity;
        shakeY = (xrand() % (g_GdiScreenShakeIntensity * 2 + 1)) - g_GdiScreenShakeIntensity;
    }
    SetViewportOrgEx(hdc, shakeX, shakeY, NULL);

    COLORREF bgTop = RGB(17, 17, 27);
    COLORREF bgBot = RGB(24, 24, 37);

    if (gameState == STATE_DUNGEON || gameState == STATE_COMBAT) {
        if (player.biome == 8 || player.biome == 9) { bgTop = RGB(46, 24, 30); bgBot = RGB(69, 30, 36); }
        else if (player.biome == 1 || player.biome == 12) { bgTop = RGB(28, 28, 46); bgBot = RGB(43, 28, 56); }
        else if (player.biome == 6) { bgTop = RGB(24, 36, 56); bgBot = RGB(36, 56, 74); }
        else if (player.biome == 3 || player.biome == 7) { bgTop = RGB(24, 46, 34); bgBot = RGB(28, 61, 42); }
    }

    RECT bgRect = {0, 0, w, h};
    HBRUSH hBgB = CreateSolidBrush(bgTop);
    FillRect(hdc, &bgRect, hBgB);
    DeleteObject(hBgB);

    HPEN hGridPen = CreatePen(PS_SOLID, 1, RGB(49, 50, 68));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hGridPen);
    for (int y = 10; y < 90; y += 20) {
        MoveToEx(hdc, 0, y, NULL); LineTo(hdc, w, y);
        int offset = (y % 40 == 10) ? 0 : 25;
        for (int x = offset; x < w; x += 50) {
            MoveToEx(hdc, x, y, NULL); LineTo(hdc, x, y + 20);
        }
    }
    SelectObject(hdc, hOldPen); DeleteObject(hGridPen);

    HBRUSH hDustB = CreateSolidBrush(RGB(205, 214, 244));
    HBRUSH hOldDB = (HBRUSH)SelectObject(hdc, hDustB);
    for (int i = 0; i < 15; i++) {
        int px = (g_GfxFrame * 2 + i * 37) % w;
        int py = ((g_GfxFrame / 2) + i * 23) % 90;
        Ellipse(hdc, px, py, px + 2, py + 2);
    }
    SelectObject(hdc, hOldDB); DeleteObject(hDustB);

    RECT floorRect = {0, 90, w, h};
    HBRUSH hFloorB = CreateSolidBrush(RGB(30, 30, 46));
    FillRect(hdc, &floorRect, hFloorB);
    DeleteObject(hFloorB);

    if (gameState == STATE_DUNGEON || gameState == STATE_COMBAT) {
        if (player.biome == 8 || player.biome == 9) {
            RECT lavaR = {0, 115, w, h};
            HBRUSH hLavaB = CreateSolidBrush(RGB(243, 139, 168));
            FillRect(hdc, &lavaR, hLavaB);
            DeleteObject(hLavaB);
        } else if (player.biome == 2 || player.biome == 13) {
            RECT waterR = {0, 118, w, h};
            HBRUSH hWaterB = CreateSolidBrush(RGB(137, 180, 250));
            FillRect(hdc, &waterR, hWaterB);
            DeleteObject(hWaterB);
        }
    }

    int heroX = 140;
    int heroY = 70;
    int monsterX = 560;
    int monsterY = 65;

    if (g_HeroActionTimer > 0) {
        g_HeroActionTimer--;
        heroX += (int)(sin((1.0 - g_HeroActionTimer / 15.0) * 3.14159) * 30);
    }

    DrawGdiHeroSprite(hdc, heroX, heroY, player.heroClass, g_GfxFrame, player.weaponName, player.armorName);

    if ((gameState == STATE_COMBAT || gameState == STATE_BOSS_RUSH) && currentEnemy.hp > 0) {
        DrawGdiMonsterSprite(hdc, monsterX, monsterY, currentEnemy.name, g_GfxFrame, currentEnemy.poisonedTurns, currentEnemy.burningTurns, currentEnemy.frozenTurns);
    } else if (gameState == STATE_TOWN || gameState == STATE_TOWN_PAGE2 || gameState == STATE_FACTIONS || gameState == STATE_MOUNTS || gameState == STATE_SHOP || gameState == STATE_CRAFTING) {
        DrawGdiNPCSprite(hdc, 560, 70, g_GfxFrame);
    }

    if (g_GdiSpellFxType > 0) {
        g_GdiSpellFxProgress++;
        float p = (float)g_GdiSpellFxProgress / g_GdiSpellFxMax;
        if (g_GdiSpellFxType == 1) {
            int fx = (int)(heroX + 40 + p * (monsterX - heroX - 60));
            int fy = heroY - 10;
            HBRUSH hFbB = CreateSolidBrush(RGB(250, 179, 135));
            HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hFbB);
            Ellipse(hdc, fx - 12, fy - 12, fx + 12, fy + 12);
            SelectObject(hdc, hOldB); DeleteObject(hFbB);
            AddGdiParticles(fx, fy, RGB(243, 139, 168), 2);
        } else if (g_GdiSpellFxType == 2) {
            HPEN hLtP = CreatePen(PS_SOLID, 3, RGB(137, 220, 235));
            HPEN hOldP = (HPEN)SelectObject(hdc, hLtP);
            MoveToEx(hdc, monsterX, 0, NULL);
            LineTo(hdc, monsterX - 10, 20);
            LineTo(hdc, monsterX + 10, 40);
            LineTo(hdc, monsterX, monsterY + 10);
            SelectObject(hdc, hOldP); DeleteObject(hLtP);
        }
        if (g_GdiSpellFxProgress >= g_GdiSpellFxMax) g_GdiSpellFxType = 0;
    }

    for (int i = g_GdiParticleCount - 1; i >= 0; i--) {
        GdiParticle* pt = &g_GdiParticles[i];
        pt->x += pt->vx;
        pt->y += pt->vy;
        pt->life++;
        HBRUSH hPtB = CreateSolidBrush(pt->color);
        HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hPtB);
        Ellipse(hdc, pt->x - 2, pt->y - 2, pt->x + 3, pt->y + 3);
        SelectObject(hdc, hOldB); DeleteObject(hPtB);
        if (pt->life >= pt->maxLife) {
            g_GdiParticles[i] = g_GdiParticles[g_GdiParticleCount - 1];
            g_GdiParticleCount--;
        }
    }

    SetBkMode(hdc, TRANSPARENT);
    HFONT hFont = CreateFontA(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    for (int i = g_GdiFloatCount - 1; i >= 0; i--) {
        GdiFloatText* ft = &g_GdiFloatTexts[i];
        ft->life++;
        ft->x += ft->vx;
        ft->y += ft->vy;
        ft->vy += 0.4f; // gravity
        
        // Shadow
        SetTextColor(hdc, RGB(17, 17, 27));
        TextOutA(hdc, (int)ft->x + 2, (int)ft->y + 2, ft->text, lstrlenA(ft->text));
        
        SetTextColor(hdc, ft->color);
        TextOutA(hdc, (int)ft->x, (int)ft->y, ft->text, lstrlenA(ft->text));
        if (ft->life >= ft->maxLife) {
            g_GdiFloatTexts[i] = g_GdiFloatTexts[g_GdiFloatCount - 1];
            g_GdiFloatCount--;
        }
    }
    SelectObject(hdc, hOldFont); DeleteObject(hFont);

    SetViewportOrgEx(hdc, 0, 0, NULL); // Reset shake

    if (g_GdiRedFlashTimer > 0) {
        g_GdiRedFlashTimer--;
        HPEN hFlashPen = CreatePen(PS_INSIDEFRAME, g_GdiRedFlashTimer * 2, RGB(243, 139, 168));
        HBRUSH hNull = (HBRUSH)GetStockObject(NULL_BRUSH);
        HPEN hOldP = (HPEN)SelectObject(hdc, hFlashPen);
        HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hNull);
        Rectangle(hdc, 0, 0, w, h);
        SelectObject(hdc, hOldP); SelectObject(hdc, hOldB);
        DeleteObject(hFlashPen);
    }

    if (g_GdiBannerActive) {
        g_GdiBannerTimer++;
        int bannerY = (g_GdiBannerTimer * 4 < 25) ? g_GdiBannerTimer * 4 : 25;
        RECT bannerR = {w / 2 - 120, bannerY, w / 2 + 120, bannerY + 36};
        HBRUSH hBanB = CreateSolidBrush(RGB(249, 226, 175));
        FillRect(hdc, &bannerR, hBanB);
        DeleteObject(hBanB);
        FrameRect(hdc, &bannerR, (HBRUSH)GetStockObject(BLACK_BRUSH));

        SetTextColor(hdc, RGB(17, 17, 27));
        SetBkMode(hdc, TRANSPARENT);
        HFONT hBanFont = CreateFontA(-20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        HFONT hOldF = (HFONT)SelectObject(hdc, hBanFont);
        DrawTextA(hdc, g_GdiBannerTitle, -1, &bannerR, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, hOldF); DeleteObject(hBanFont);

        if (g_GdiBannerTimer > 90) g_GdiBannerActive = 0;
    }
}

LRESULT CALLBACK GfxCanvasProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;

        HDC hMemDC = CreateCompatibleDC(hdc);
        HBITMAP hMemBmp = CreateCompatibleBitmap(hdc, w, h);
        HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hMemBmp);

        RenderGdiScene(hMemDC, w, h);

        BitBlt(hdc, 0, 0, w, h, hMemDC, 0, 0, SRCCOPY);

        SelectObject(hMemDC, hOldBmp);
        DeleteObject(hMemBmp);
        DeleteDC(hMemDC);

        EndPaint(hwnd, &ps);
        return 0;
    }
    if (msg == WM_ERASEBKGND) return 1;
    return CallWindowProcA(g_OldCanvasProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hBgBrush = CreateSolidBrush(RGB(17, 17, 27));
            hPanelBrush = CreateSolidBrush(RGB(30, 30, 46));

            hGfxCanvas = CreateWindowA("STATIC", "", WS_CHILD | WS_VISIBLE, 15, 10, 755, 130, hwnd, (HMENU)100, GetModuleHandle(NULL), NULL);
            g_OldCanvasProc = (WNDPROC)SetWindowLongPtrA(hGfxCanvas, GWLP_WNDPROC, (LONG_PTR)GfxCanvasProc);

            hStatusText = CreateWindowA("STATIC", "",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                15, 145, 755, 20, hwnd, (HMENU)101, GetModuleHandle(NULL), NULL);

            hInfoText = CreateWindowA("STATIC", "",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                15, 168, 755, 85, hwnd, (HMENU)102, GetModuleHandle(NULL), NULL);

            hLogEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                15, 260, 755, 158, hwnd, (HMENU)103, GetModuleHandle(NULL), NULL);

            hBtn1 = CreateWindowA("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 15,  430, 118, 38, hwnd, (HMENU)201, GetModuleHandle(NULL), NULL);
            hBtn2 = CreateWindowA("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 142, 430, 118, 38, hwnd, (HMENU)202, GetModuleHandle(NULL), NULL);
            hBtn3 = CreateWindowA("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 269, 430, 118, 38, hwnd, (HMENU)203, GetModuleHandle(NULL), NULL);
            hBtn4 = CreateWindowA("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 396, 430, 118, 38, hwnd, (HMENU)204, GetModuleHandle(NULL), NULL);
            hBtn5 = CreateWindowA("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 523, 430, 118, 38, hwnd, (HMENU)205, GetModuleHandle(NULL), NULL);
            hBtn6 = CreateWindowA("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 650, 430, 118, 38, hwnd, (HMENU)206, GetModuleHandle(NULL), NULL);

            hUIFont = CreateFontA(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            SendMessage(hStatusText, WM_SETFONT, (WPARAM)hUIFont, FALSE);
            SendMessage(hInfoText, WM_SETFONT, (WPARAM)hUIFont, FALSE);
            SendMessage(hLogEdit, WM_SETFONT, (WPARAM)hUIFont, FALSE);
            SendMessage(hBtn1, WM_SETFONT, (WPARAM)hUIFont, FALSE);
            SendMessage(hBtn2, WM_SETFONT, (WPARAM)hUIFont, FALSE);
            SendMessage(hBtn3, WM_SETFONT, (WPARAM)hUIFont, FALSE);
            SendMessage(hBtn4, WM_SETFONT, (WPARAM)hUIFont, FALSE);
            SendMessage(hBtn5, WM_SETFONT, (WPARAM)hUIFont, FALSE);
            SendMessage(hBtn6, WM_SETFONT, (WPARAM)hUIFont, FALSE);

            SetTimer(hwnd, 1, 40, NULL);

            InitHero(0);
            SetupButtons();
            UpdateUI();
            LogMessage("=== Welcome to KQuest: Fantasy Dungeon RPG ===");
            LogMessage("Press 'H' at any time to view the Help & Codex.");
            LogMessage("Phase 14: Comprehensive Help & Lore Codex Active (Press F1 / H or click Help)!");
            break;
        }
        case WM_TIMER:
            if (hGfxCanvas) InvalidateRect(hGfxCanvas, NULL, FALSE);
            break;

        case WM_KEYDOWN: {
            if (g_BindingAction != -1) {
                if (wParam >= 0x30 && wParam <= 0x5A) {
                    g_KeyBinds[g_BindingAction] = (int)wParam;
                    char msg[64];
                    wsprintfA(msg, "Action %d bound to '%c'!", g_BindingAction + 1, (char)wParam);
                    LogMessage(msg);
                } else {
                    LogMessage("Invalid key for bind! Try a letter or number.");
                }
                g_BindingAction = -1;
                SetupButtons();
                return 0;
            }

            if (wParam == VK_F5) {
                SaveToSlot(0);
                return 0;
            } else if (wParam == VK_F9) {
                LoadFromSlot(0);
                return 0;
            }

            if (wParam == VK_F1 || wParam == 'H' || wParam == 'h') {
                if (gameState == STATE_TAVERN) { LogMessage("Barkeep: 'Welcome! The Ruined Castle is dangerous!'"); return 0; }
    if (gameState == STATE_TAVERN) { LogMessage("Rumor: 'Equipment can be upgraded at the forge now.'"); return 0; }
    if (gameState == STATE_HELP) {
                    gameState = STATE_TOWN;
                    LogMessage("Closed Help Overlay. Returned to Town.");
                } else {
                    g_HelpTab = 0;
                    gameState = STATE_HELP;
                    LogMessage("=== Opened Help & Lore Codex (F1 / H) ===");
                }
                SetupButtons();
                UpdateUI();
            } else if (wParam == g_KeyBinds[0]) { SendMessage(hwnd, WM_COMMAND, 201, 0);
            } else if (wParam == g_KeyBinds[1]) { SendMessage(hwnd, WM_COMMAND, 202, 0);
            } else if (wParam == g_KeyBinds[2]) { SendMessage(hwnd, WM_COMMAND, 203, 0);
            } else if (wParam == g_KeyBinds[3]) { SendMessage(hwnd, WM_COMMAND, 204, 0);
            } else if (wParam == g_KeyBinds[4]) { SendMessage(hwnd, WM_COMMAND, 205, 0);
            } else if (wParam == g_KeyBinds[5]) { SendMessage(hwnd, WM_COMMAND, 206, 0);
            } else if (wParam == 'S' || wParam == 's') {
                if (gameState == STATE_COMBAT) {
                    CastHolyShield();
                } else {
                    SaveToSlot(0);
                    LogMessage("⚡ Quick Saved game to Slot 1!");
                    SetupButtons();
                    UpdateUI();
                }
            } else if (wParam == 'L' || wParam == 'l') {
                if (gameState == STATE_COMBAT) {
                    CastLightningStorm();
                } else {
                    gameState = STATE_SAVE_LOAD;
                    SetupButtons();
                    UpdateUI();
                }
            } else if (wParam == 'P' || wParam == 'p') {
                UsePhoenixElixir();
            } else if (wParam == 'B' || wParam == 'b') {
                if (gameState == STATE_COMBAT) {
                    CastBerserkMight();
                }
            } else if (wParam == 'I' || wParam == 'i') {
                gameState = STATE_INVENTORY;
                SetupButtons();
                UpdateUI();
            } else if (wParam == VK_ESCAPE) {
                if (gameState == STATE_HELP || gameState == STATE_INVENTORY || gameState == STATE_SAVE_LOAD || gameState == STATE_ACHIEVEMENTS || gameState == STATE_QUEST_BOARD || gameState == STATE_TRAINING_HALL || gameState == STATE_SHOP || gameState == STATE_CRAFTING || gameState == STATE_TAVERN || gameState == STATE_MERCENARY || gameState == STATE_BOSS_RUSH) {
                    gameState = STATE_TOWN;
                    LogMessage("Returned to Town Square.");
                    SetupButtons();
                    UpdateUI();
                }
            }
            break;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case 201: HandleButton1(); break;
                case 202: HandleButton2(); break;
                case 203: HandleButton3(); break;
                case 204: HandleButton4(); break;
                case 205: HandleButton5(); break;
                case 206: HandleButton6(); break;
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(205, 214, 244));
            SetBkColor(hdcStatic, RGB(30, 30, 46));
            return (INT_PTR)hPanelBrush;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdcEdit = (HDC)wParam;
            SetTextColor(hdcEdit, RGB(205, 214, 244));
            SetBkColor(hdcEdit, RGB(24, 24, 37));
            static HBRUSH hEditBg = NULL;
            if (!hEditBg) hEditBg = CreateSolidBrush(RGB(24, 24, 37));
            return (INT_PTR)hEditBg;
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
    wc.lpszClassName = "KQuestClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassA(&wc);

    RECT wr = {0, 0, 785, 520};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);
    HWND hwnd = CreateWindowA("KQuestClass", "KQuest - Fantasy Dungeon RPG (Press H for Help)", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
                              CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
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
