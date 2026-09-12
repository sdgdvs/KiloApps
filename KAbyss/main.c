#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define WINDOW_WIDTH  1060
#define WINDOW_HEIGHT 680
#define VIEWPORT_W   704
#define VIEWPORT_H   520
#define MAP_WIDTH     48
#define MAP_HEIGHT    36
#define TILE_SIZE     32
#define TIMER_ID      1
#define TIMER_INTERVAL 33 // ~30 FPS

// Color definitions (matching web palette)
#define COLOR_BG_ABYSS      RGB(7, 9, 14)
#define COLOR_BG_PANEL      RGB(13, 18, 29)
#define COLOR_BG_PANEL_DARK RGB(9, 12, 20)
#define COLOR_BG_CARD       RGB(19, 26, 41)
#define COLOR_BORDER        RGB(30, 41, 59)
#define COLOR_BORDER_GLOW   RGB(56, 189, 248)
#define COLOR_TEXT_PRIMARY  RGB(203, 213, 225)
#define COLOR_TEXT_BRIGHT   RGB(248, 250, 252)
#define COLOR_TEXT_DIM      RGB(100, 116, 139)
#define COLOR_TEXT_GOLD     RGB(251, 191, 36)
#define COLOR_TEXT_RUNE     RGB(129, 140, 248)
#define COLOR_ACCENT_RED    RGB(239, 68, 68)
#define COLOR_ACCENT_CYAN   RGB(6, 182, 212)
#define COLOR_ACCENT_GREEN  RGB(16, 185, 129)
#define COLOR_ACCENT_PURPLE RGB(168, 85, 247)
#define COLOR_ACCENT_AMBER  RGB(245, 158, 11)

// Tile Types
#define TILE_VOID         0
#define TILE_WALL         1
#define TILE_FLOOR        2
#define TILE_DOOR_CLOSED  3
#define TILE_DOOR_OPEN    4
#define TILE_STAIRS_DOWN  5
#define TILE_STAIRS_UP    6
#define TILE_PILLAR       7
#define TILE_TORCH        8
#define TILE_CHEST        9
#define TILE_RUBBLE       10
#define TILE_WATER        11
#define TILE_CHASM        12
#define TILE_ALTAR        13
#define TILE_CAULDRON     14
#define TILE_EFFIGY       15
#define TILE_SHRINE       16
#define TILE_MERCHANT     17

// Weapon Enchantments
#define ENCHANT_NONE      0
#define ENCHANT_FIRE      1
#define ENCHANT_FROST     2
#define ENCHANT_VOID      3

typedef enum {
    CURSE_NONE = 0,
    CURSE_DARKNESS = 1,
    CURSE_ENFEEBLE = 2,
    CURSE_DECAY = 3,
    CURSE_VOID = 4
} CurseType;

typedef enum {
    ZONE_CATACOMBS = 0,
    ZONE_SUNKEN_GROTTO = 1,
    ZONE_FORGOTTEN_CRYPT = 2,
    ZONE_VOID_ABYSS = 3
} DepthZone;

typedef struct {
    const char* name;
    const char* shortName;
    const char* sub;
    const char* enterMsg;
    const char* sconceName;
    const char* pillarName;
    COLORREF wallColor;
    COLORREF wallBorder;
    COLORREF floorColor;
    COLORREF floorBg;
    COLORREF torchColor;
    COLORREF torchHalo;
    COLORREF particleColor1;
    COLORREF particleColor2;
} ZoneTheme;

static const ZoneTheme g_zoneThemes[4] = {
    {
        "The Catacombs",
        "Catacombs",
        "Ancient limestone crypts & dust",
        "Entered Catacombs Depth B%d. Shrouded in forgotten silence.",
        "Wall Torch Sconce",
        "Limestone Column",
        RGB(30, 41, 59),
        RGB(51, 65, 85),
        RGB(15, 23, 42),
        RGB(7, 10, 18),
        RGB(245, 158, 11),
        RGB(55, 32, 10),
        RGB(245, 158, 11),
        RGB(239, 68, 68)
    },
    {
        "Sunken Grotto",
        "Sunken Grotto",
        "Flooded cavern network & azure spores",
        "Entered Sunken Grotto B%d. Moisture drips into flooded pools.",
        "Bioluminescent Fungi",
        "Stalagmite Spire",
        RGB(13, 43, 56),
        RGB(22, 78, 99),
        RGB(8, 29, 38),
        RGB(3, 19, 26),
        RGB(6, 182, 212),
        RGB(8, 47, 73),
        RGB(6, 182, 212),
        RGB(16, 185, 129)
    },
    {
        "Forgotten Crypt",
        "Forgotten Crypt",
        "Necrotic tombs & runic altars",
        "Entered Forgotten Crypt B%d. Necrotic runes bleed crimson light.",
        "Occult Skull Brazier",
        "Necrotic Monolith",
        RGB(45, 18, 24),
        RGB(127, 29, 29),
        RGB(24, 10, 14),
        RGB(15, 5, 8),
        RGB(239, 68, 68),
        RGB(69, 10, 10),
        RGB(239, 68, 68),
        RGB(168, 85, 247)
    },
    {
        "The Void Abyss",
        "Void Abyss",
        "Cosmic islands over bottomless chasms",
        "Entered Void Abyss B%d. Cosmic chasms yawn beneath obsidian stone.",
        "Void Rift Crystal",
        "Astral Void Pylon",
        RGB(33, 16, 56),
        RGB(88, 28, 135),
        RGB(15, 7, 28),
        RGB(7, 2, 13),
        RGB(168, 85, 247),
        RGB(59, 7, 100),
        RGB(192, 132, 252),
        RGB(56, 189, 248)
    }
};

static DepthZone GetDepthZone(int depth) {
    if (depth <= 3) return ZONE_CATACOMBS;
    if (depth <= 6) return ZONE_SUNKEN_GROTTO;
    if (depth <= 9) return ZONE_FORGOTTEN_CRYPT;
    return ZONE_VOID_ABYSS;
}

#define MAX_TORCHES 64
#define MAX_CHESTS  32
#define MAX_LOG_MSGS 30
#define MAX_ROOMS   20

typedef struct {
    int x, y;
    int w, h;
} Room;

typedef struct {
    int x, y;
    int intensity;
    BOOL lit;
} Torch;

typedef struct {
    int x, y;
    int opened;
    int essence;
} Chest;

typedef struct {
    char text[128];
    COLORREF color;
    int turn;
} LogMessage;

#define RUNE_PYRE     0
#define RUNE_FROST    1
#define RUNE_TEMPEST  2
#define RUNE_VOID     3
#define RUNE_AEGIS    4
#define NUM_RUNES     5

typedef struct {
    const char* name;
    const char* symbol;
    const char* element;
    COLORREF color;
    const char* spellName;
    int cost;
    const char* desc;
    const char* passive;
} RuneDef;

static const RuneDef g_runeDefs[NUM_RUNES] = {
    { "Pyre Rune", "F", "Fire", RGB(249, 115, 22), "Pyre Blast", 12, "Hurls flame 4 tiles (35 Fire DMG). Burns obstacles.", "+3 Might" },
    { "Frost Rune", "I", "Ice", RGB(6, 182, 212), "Glacial Nova", 10, "Freezes 2-tile radius (24 Cryo DMG). Freezes water.", "+3 Warding" },
    { "Tempest Rune", "L", "Lightning", RGB(234, 179, 8), "Chain Bolt", 14, "Piercing bolt 6 tiles (42 Shock DMG). Shatters doors.", "+2 Might, +2 Arcana" },
    { "Void Rune", "V", "Eldritch", RGB(168, 85, 247), "Void Warp", 15, "Phase-shifts 3 paces forward through obstacles & chasms.", "+3 Arcana, +1 Light" },
    { "Aegis Rune", "A", "Warding", RGB(56, 189, 248), "Aegis Ward", 10, "Prismatic barrier absorbs 35 DMG, +15 Sanity.", "+4 Warding, +10 Sanity" }
};

typedef struct {
    const char* name;
    const char* tier;
    int maxSockets;
    int arcanaBonus;
    const char* desc;
} StaffDef;

static const StaffDef g_staffDefs[3] = {
    { "Ashwood Rune Staff", "Tier I", 2, 2, "Petrified ash. Holds 2 ancient elemental runes." },
    { "Cinderwood Scepter", "Tier II", 2, 4, "Magma-forged. Holds 2 runes with +4 Arcana." },
    { "Staff of the Arch-Magi", "Tier III", 3, 6, "Ancient conduit. Holds 3 runes with -2 MP cost." }
};

typedef struct {
    int type; // 0=pyre, 1=frost, 2=tempest, 3=void
    int x, y;
    int x2, y2;
    int radius;
    int duration;
    COLORREF color;
} SpellFX;

#define MAX_SPELL_FX 16
static SpellFX g_spellFX[MAX_SPELL_FX];
static int g_numSpellFX = 0;

typedef enum {
    ITEM_TYPE_CONSUMABLE = 0,
    ITEM_TYPE_EQUIPMENT,
    ITEM_TYPE_REAGENT,
    ITEM_TYPE_KEY
} ItemCategory;

typedef enum {
    SLOT_NONE = 0,
    SLOT_WEAPON,
    SLOT_ARMOR,
    SLOT_RELIC,
    SLOT_AMULET
} EquipSlot;

typedef enum {
    ITEM_NONE = 0,
    // Consumables
    ITEM_HEAL_SALVE,
    ITEM_ELIXIR_VITALITY,
    ITEM_SANITY_INCENSE,
    ITEM_LUCID_DRAUGHT,
    ITEM_AETHER_PHIAL,
    ITEM_STONESKIN_BREW,
    ITEM_LIQUID_FIRE,
    ITEM_PANACEA_DEEP,
    ITEM_FOOD_RATIONS,
    ITEM_CRYPT_MUSHROOM,
    ITEM_PURIFYING_SALT,
    // Weapons
    ITEM_WPN_STAFF,
    ITEM_WPN_RUNIC_BLADE,
    ITEM_WPN_VOID_DAGGER,
    // Armor
    ITEM_ARM_ABYSSAL_MAIL,
    ITEM_ARM_SHADOW_CLOAK,
    ITEM_ARM_AEGIS_CUIRASS,
    // Relics
    ITEM_REL_TORCH,
    ITEM_REL_LANTERN,
    ITEM_REL_CENSER,
    // Amulets
    ITEM_AMU_LIFE,
    ITEM_AMU_STAR,
    ITEM_AMU_VOID,
    // Reagents
    ITEM_ING_BLOOD_LOTUS,
    ITEM_ING_AZURE_SPORES,
    ITEM_ING_BRIMSTONE,
    ITEM_ING_VOID_DUST,
    ITEM_ING_AETHER_BLOSSOM,
    // Keys
    ITEM_KEY_RUNIC,
    // Boss Drops
    ITEM_WPN_CRYPT_GREATSWORD,
    ITEM_ARM_WYRMSCALE,
    ITEM_REL_MONARCH_CROWN,
    NUM_ITEM_DEFS
} ItemId;

typedef struct {
    ItemId id;
    const char* name;
    ItemCategory category;
    EquipSlot slot;
    const char* symbol;
    COLORREF color;
    const char* desc;
    int might;
    int warding;
    int arcana;
    int light;
    int maxHp;
    int maxAether;
    int maxSanity;
} ItemDef;

typedef struct {
    ItemId id;
    int count;
} InventorySlot;

#define MAX_PACK_SLOTS 16

static const ItemDef g_itemDefs[NUM_ITEM_DEFS] = {
    { ITEM_NONE, "None", ITEM_TYPE_KEY, SLOT_NONE, "-", RGB(100,100,100), "None", 0,0,0,0, 0,0,0 },
    { ITEM_HEAL_SALVE, "Healing Salve", ITEM_TYPE_CONSUMABLE, SLOT_NONE, "+", COLOR_ACCENT_GREEN, "Restores +35 HP.", 0,0,0,0, 35,0,0 },
    { ITEM_ELIXIR_VITALITY, "Elixir of Vitality", ITEM_TYPE_CONSUMABLE, SLOT_NONE, "+", COLOR_ACCENT_GREEN, "Restores +65 HP, +10 Sanity.", 0,0,0,0, 65,0,10 },
    { ITEM_SANITY_INCENSE, "Sanity Incense", ITEM_TYPE_CONSUMABLE, SLOT_NONE, "~", COLOR_BORDER_GLOW, "Clears madness (+30 Sanity).", 0,0,0,0, 0,0,30 },
    { ITEM_LUCID_DRAUGHT, "Draught of Lucid Mind", ITEM_TYPE_CONSUMABLE, SLOT_NONE, "~", COLOR_BORDER_GLOW, "Restores +50 Sanity, +10 Aether.", 0,0,0,0, 0,10,50 },
    { ITEM_AETHER_PHIAL, "Aether Phial", ITEM_TYPE_CONSUMABLE, SLOT_NONE, "*", COLOR_ACCENT_CYAN, "Concentrated mana (+40 Aether).", 0,0,0,0, 0,40,0 },
    { ITEM_STONESKIN_BREW, "Stoneskin Brew", ITEM_TYPE_CONSUMABLE, SLOT_NONE, "#", COLOR_ACCENT_AMBER, "Hardens flesh (+40 Ward Shield).", 0,0,0,0, 0,0,0 },
    { ITEM_LIQUID_FIRE, "Liquid Fire Flask", ITEM_TYPE_CONSUMABLE, SLOT_NONE, "!", COLOR_ACCENT_RED, "50 Fire AOE to enemies within 2 tiles.", 0,0,0,0, 0,0,0 },
    { ITEM_PANACEA_DEEP, "Panacea of the Deep", ITEM_TYPE_CONSUMABLE, SLOT_NONE, "@", COLOR_TEXT_GOLD, "+60 HP, +40 Sanity, +35 Aether, cures curses.", 0,0,0,0, 60,35,40 },
    { ITEM_FOOD_RATIONS, "Iron Rations", ITEM_TYPE_CONSUMABLE, SLOT_NONE, "+", COLOR_TEXT_GOLD, "Nutritious dried meat & hardtack (+45 Hunger, +10 HP).", 0,0,0,0, 10,0,0 },
    { ITEM_CRYPT_MUSHROOM, "Crypt Truffle", ITEM_TYPE_CONSUMABLE, SLOT_NONE, "%", COLOR_ACCENT_CYAN, "Cavern fungal mushroom (+25 Hunger, +5 MP, -4 Sanity).", 0,0,0,0, 0,5,-4 },
    { ITEM_PURIFYING_SALT, "Purifying Salt", ITEM_TYPE_CONSUMABLE, SLOT_NONE, "*", COLOR_BORDER_GLOW, "Consecrated crystal salt. Purges curses (+20 Sanity).", 0,0,0,0, 0,0,20 },
    { ITEM_WPN_STAFF, "Ashwood Rune Staff", ITEM_TYPE_EQUIPMENT, SLOT_WEAPON, "/", RGB(168,85,247), "+2 Arcana.", 0,0,2,0, 0,0,0 },
    { ITEM_WPN_RUNIC_BLADE, "Runic Longsword", ITEM_TYPE_EQUIPMENT, SLOT_WEAPON, "/", COLOR_ACCENT_AMBER, "+5 Might, +1 Warding.", 5,1,0,0, 0,0,0 },
    { ITEM_WPN_VOID_DAGGER, "Voidfang Dagger", ITEM_TYPE_EQUIPMENT, SLOT_WEAPON, "/", RGB(168,85,247), "+7 Might, +3 Arcana.", 7,0,3,0, 0,0,0 },
    { ITEM_ARM_ABYSSAL_MAIL, "Abyssal Mail", ITEM_TYPE_EQUIPMENT, SLOT_ARMOR, "[", COLOR_BORDER_GLOW, "+3 Warding, +10 Max HP.", 0,3,0,0, 10,0,0 },
    { ITEM_ARM_SHADOW_CLOAK, "Shadowweave Cloak", ITEM_TYPE_EQUIPMENT, SLOT_ARMOR, "[", RGB(168,85,247), "+4 Warding, +2 Arcana.", 0,4,2,0, 0,0,0 },
    { ITEM_ARM_AEGIS_CUIRASS, "Aegis Cuirass", ITEM_TYPE_EQUIPMENT, SLOT_ARMOR, "[", COLOR_TEXT_RUNE, "+7 Warding, +25 Max HP.", 0,7,0,0, 25,0,0 },
    { ITEM_REL_TORCH, "Torch of Eld", ITEM_TYPE_EQUIPMENT, SLOT_RELIC, "i", COLOR_TEXT_GOLD, "+7 Light Radius.", 0,0,0,7, 0,0,0 },
    { ITEM_REL_LANTERN, "Aether Lantern", ITEM_TYPE_EQUIPMENT, SLOT_RELIC, "i", COLOR_ACCENT_CYAN, "+8 Light Radius, +15 Max Aether.", 0,0,1,8, 0,15,0 },
    { ITEM_REL_CENSER, "Radiant Censer", ITEM_TYPE_EQUIPMENT, SLOT_RELIC, "i", COLOR_TEXT_GOLD, "+9 Light Radius, +20 Max Sanity.", 0,1,1,9, 0,0,20 },
    { ITEM_AMU_LIFE, "Amulet of Vitality", ITEM_TYPE_EQUIPMENT, SLOT_AMULET, "o", COLOR_ACCENT_RED, "+30 Max HP, +1 Might, +1 Warding.", 1,1,0,0, 30,0,0 },
    { ITEM_AMU_STAR, "Astral Star Pendant", ITEM_TYPE_EQUIPMENT, SLOT_AMULET, "o", COLOR_ACCENT_CYAN, "+25 Max Aether, +2 Arcana.", 0,0,2,0, 0,25,0 },
    { ITEM_AMU_VOID, "Void Eye Talisman", ITEM_TYPE_EQUIPMENT, SLOT_AMULET, "o", RGB(168,85,247), "+4 Arcana, +20 Max Sanity.", 0,0,4,0, 0,0,20 },
    { ITEM_ING_BLOOD_LOTUS, "Blood Lotus", ITEM_TYPE_REAGENT, SLOT_NONE, "%", RGB(244,63,94), "Reagent for Vitality & Panacea elixirs.", 0,0,0,0, 0,0,0 },
    { ITEM_ING_AZURE_SPORES, "Azure Spores", ITEM_TYPE_REAGENT, SLOT_NONE, "%", RGB(52,211,153), "Reagent for Lucid Mind & Stoneskin.", 0,0,0,0, 0,0,0 },
    { ITEM_ING_BRIMSTONE, "Brimstone Ash", ITEM_TYPE_REAGENT, SLOT_NONE, "%", RGB(245,158,11), "Reagent for Liquid Fire & Stoneskin.", 0,0,0,0, 0,0,0 },
    { ITEM_ING_VOID_DUST, "Void Dust", ITEM_TYPE_REAGENT, SLOT_NONE, "%", RGB(168,85,247), "Reagent for Aether Phials & Liquid Fire.", 0,0,0,0, 0,0,0 },
    { ITEM_ING_AETHER_BLOSSOM, "Aether Blossom", ITEM_TYPE_REAGENT, SLOT_NONE, "%", RGB(56,189,248), "Mana-rich catalyst for high-tier brews.", 0,0,0,0, 0,0,0 },
    { ITEM_KEY_RUNIC, "Ancient Runic Key", ITEM_TYPE_KEY, SLOT_NONE, "k", COLOR_TEXT_GOLD, "Unlocks crypt chests and sealed doors.", 0,0,0,0, 0,0,0 },
    { ITEM_WPN_CRYPT_GREATSWORD, "Keeper's Greatsword", ITEM_TYPE_EQUIPMENT, SLOT_WEAPON, "/", RGB(245,158,11), "+10 Might, +3 Warding, +15 Max HP. Heavy tomb blade.", 10,3,0,0, 15,0,0 },
    { ITEM_ARM_WYRMSCALE, "Wyrmscale Carapace", ITEM_TYPE_EQUIPMENT, SLOT_ARMOR, "[", RGB(16,185,129), "+9 Warding, +40 Max HP, +2 Might. Impervious to acid.", 2,9,0,0, 40,0,0 },
    { ITEM_REL_MONARCH_CROWN, "Crown of Void Monarch", ITEM_TYPE_EQUIPMENT, SLOT_RELIC, "o", RGB(192,132,252), "+10 Light, +35 Max Aether, +35 Max Sanity, +5 Arcana.", 0,2,5,10, 0,35,35 }
};

#define NUM_RECIPES 6

typedef struct {
    ItemId res;
    const char* name;
    const char* desc;
    struct {
        ItemId id;
        int count;
    } ing[3];
    int numIng;
} AlchemyRecipe;

static const AlchemyRecipe g_recipes[NUM_RECIPES] = {
    { ITEM_ELIXIR_VITALITY, "Elixir of Vitality", "+65 HP, +10 Sanity", { { ITEM_ING_BLOOD_LOTUS, 1 }, { ITEM_ING_AETHER_BLOSSOM, 1 }, { ITEM_NONE, 0 } }, 2 },
    { ITEM_LUCID_DRAUGHT, "Draught of Lucid Mind", "+50 Sanity, +10 Aether", { { ITEM_ING_AZURE_SPORES, 1 }, { ITEM_ING_AETHER_BLOSSOM, 1 }, { ITEM_NONE, 0 } }, 2 },
    { ITEM_AETHER_PHIAL, "Aether Phial", "+40 Aether mana", { { ITEM_ING_VOID_DUST, 1 }, { ITEM_ING_AETHER_BLOSSOM, 1 }, { ITEM_NONE, 0 } }, 2 },
    { ITEM_STONESKIN_BREW, "Stoneskin Brew", "+40 Ward Shield", { { ITEM_ING_AZURE_SPORES, 1 }, { ITEM_ING_BRIMSTONE, 1 }, { ITEM_NONE, 0 } }, 2 },
    { ITEM_LIQUID_FIRE, "Liquid Fire Flask", "50 Fire AOE damage", { { ITEM_ING_BRIMSTONE, 1 }, { ITEM_ING_VOID_DUST, 1 }, { ITEM_NONE, 0 } }, 2 },
    { ITEM_PANACEA_DEEP, "Panacea of the Deep", "+60 HP, +40 San, +35 MP", { { ITEM_ING_BLOOD_LOTUS, 1 }, { ITEM_ING_BRIMSTONE, 1 }, { ITEM_ING_VOID_DUST, 1 } }, 3 }
};

typedef struct {
    int x, y;
    int hp, max_hp;
    int sanity, max_sanity;
    int aether, max_aether;
    int shield;
    int essence;
    int level;
    int exp, max_exp;
    int might;
    int warding;
    int arcana;
    int light_radius;
    int hunger, max_hunger;
    int torchFuel, maxTorchFuel;
    BOOL torchLit;
    CurseType curse;
    int curseTurns;
    int base_hp, base_max_hp;
    int base_sanity, base_max_sanity;
    int base_aether, base_max_aether;
    int base_might;
    int base_warding;
    int base_arcana;
    int base_light_radius;
    int facing; // 0=Up, 1=Right, 2=Down, 3=Left
    int equippedStaff; // 0=Ashwood, 1=Cinder, 2=Arch-Magi
    int staffSockets[3]; // Rune index 0..4 or -1 for empty
    BOOL ownedRunes[NUM_RUNES];
    ItemId equipWeapon;
    ItemId equipArmor;
    ItemId equipRelic;
    ItemId equipAmulet;
    InventorySlot pack[MAX_PACK_SLOTS];
    int numPackItems;
    int weaponEnchant; // 0=None, 1=Fire (Flamebrand), 2=Frost (Frostbite), 3=Void (Voidsever)
} Delver;


typedef struct {
    float x, y;
    float vx, vy;
    float life;
    float decay;
    COLORREF color;
} Ember;

#define MAX_EMBERS 64
static Ember g_embers[MAX_EMBERS];
static int g_numEmbers = 0;

// Global Game State
static int g_dungeon[MAP_HEIGHT][MAP_WIDTH];
static BOOL g_explored[MAP_HEIGHT][MAP_WIDTH];
static BOOL g_visible[MAP_HEIGHT][MAP_WIDTH];
static float g_lightMap[MAP_HEIGHT][MAP_WIDTH];

static Torch g_torches[MAX_TORCHES];
static int g_numTorches = 0;

static Chest g_chests[MAX_CHESTS];
static int g_numChests = 0;

static LogMessage g_logs[MAX_LOG_MSGS];
static int g_logCount = 0;

// Monster Bestiary Definitions
#define MONSTER_SKELETON      0
#define MONSTER_GHOUL         1
#define MONSTER_WRAITH        2
#define MONSTER_ACOLYTE       3
#define MONSTER_LEVIATHAN     4
#define MONSTER_CRYPT_KEEPER  5
#define MONSTER_ABYSSAL_WYRM  6
#define MONSTER_VOID_MONARCH  7
#define NUM_MONSTER_TYPES     8

typedef struct {
    const char* name;
    const char* symbol;
    COLORREF color;
    int baseHp;
    int hpScale;
    int baseAtk;
    int atkScale;
    int exp;
    int essence;
    const char* desc;
    const char* weakness;
} MonsterDef;

static const MonsterDef g_monsterDefs[NUM_MONSTER_TYPES] = {
    { "Crypt Skeleton", "S", RGB(226, 232, 240), 32, 4, 9, 2, 22, 18, "Undead crypt warden with tarnished blade.", "Weak to Fire (Pyre x1.5)" },
    { "Mire Ghoul", "G", RGB(45, 212, 191), 44, 5, 12, 2, 30, 25, "Amphibious beast lurking in flooded shallows.", "Weak to Shock (Tempest x1.4)" },
    { "Void Wraith", "W", RGB(192, 132, 252), 46, 6, 15, 3, 42, 35, "Phases through walls; drains Sanity & Aether.", "Vulnerable to Aegis barrier" },
    { "Crypt Acolyte", "N", RGB(234, 179, 8), 36, 4, 14, 2, 35, 30, "Necromancer hurling long-range Shadow Bolts.", "Weak in close melee" },
    { "Abyssal Leviathan", "L", RGB(244, 63, 94), 95, 12, 24, 4, 80, 75, "Colossal horror with crushing slams.", "Susceptible to Glacial Nova (2-turn Freeze)" },
    { "The Crypt Keeper", "K", RGB(245, 158, 11), 280, 25, 28, 4, 250, 220, "Catacombs Lord. Bone plating, tomb cleaves, summons skeletons.", "Weak to Sacred Fire (Pyre x1.75)" },
    { "Abyssal Wyrm", "Y", RGB(16, 185, 129), 380, 35, 36, 5, 400, 350, "Sunken Grotto Sovereign. Spits caustic acid, coils & burrows.", "Weak to Glacial Frost (Cryo Freeze)" },
    { "The Void Monarch", "M", RGB(192, 132, 252), 520, 45, 46, 6, 650, 500, "Cosmic Void Sovereign. Astral beams, singularity pull, dread aura.", "Weak to Aegis Ward & Tempest Shock" }
};

typedef struct {
    int type;
    int x, y;
    int hp, max_hp;
    int atk;
    int exp, essence;
    int state; // 0=idle, 1=alert
    int freezeTurns;
    int alertRange;
    BOOL isBoss;
    BOOL bonePlated;
    BOOL alive;
} Monster;

#define MAX_MONSTERS 32
static Monster g_monsters[MAX_MONSTERS];
static int g_numMonsters = 0;

typedef struct {
    float x, y;
    char text[32];
    COLORREF color;
    float life;
} CombatText;

#define MAX_COMBAT_TEXTS 16
static CombatText g_combatTexts[MAX_COMBAT_TEXTS];
static int g_numCombatTexts = 0;

static Delver g_player;
static int g_depthLevel = 1;
static int g_turn = 1;
static BOOL g_fovEnabled = TRUE;
static BOOL g_crtEnabled = TRUE;
static int g_activeTab = 0; // 0=Hero, 1=Inventory, 2=Runes, 3=Bestiary
static BOOL g_showHelpModal = FALSE;
static BOOL g_showEnchantModal = FALSE;
static BOOL g_showMerchantModal = FALSE;
static int g_merchantMode = 0; // 0=Buy, 1=Sell
static float g_animFlicker = 0.0f;
static int g_frameCount = 0;

// Camera
static int g_camX = 0;
static int g_camY = 0;

// Function declarations
void InitGame(int depth);
void GenerateCatacombs(int depth);
void GenerateSunkenGrotto(int depth);
void GenerateForgottenCrypt(int depth);
void GenerateVoidAbyss(int depth);
void ComputeFOV(void);
void AddLog(const char* text, COLORREF color);
void MovePlayer(int dx, int dy);
void AdvanceTurn(void);
void RestTurn(void);
void SearchArea(void);
void InteractTile(void);
void CommuneAltar(int x, int y);
void OpenEnchantAltar(void);
void CloseEnchantAltar(void);
void ImbueEnchantment(int enchantType);
void CommuneAltarBenediction(void);
void CommuneShrine(int x, int y);
void OpenMerchantShop(void);
void CloseMerchantShop(void);
void BuyMerchantItem(int itemNum);
void SellPackItemToMerchant(int packIdx);
void CheckLevelUp(void);
void SpawnEmber(float x, float y, BOOL isTorch);
void UpdateEmbers(void);
void CastSpell(int socketIdx);
void SocketRune(int socketIdx, int runeIdx);
void UnsocketRune(int socketIdx);
BOOL CheckLOS(int x0, int y0, int x1, int y1);
void SpawnCombatText(float x, float y, const char* text, COLORREF color);
void SpawnMonsters(int level);
void DamageMonster(int idx, int dmg, const char* dmgType, BOOL isCrit);
void AttackMonster(int idx);
void UpdateMonsters(void);
void RecalcPlayerStats(void);
BOOL AddPackItem(ItemId id, int count);
BOOL RemovePackItem(ItemId id, int count);
int GetPackItemCount(ItemId id);
void UsePackItem(int packIdx);
void EquipPackItem(int packIdx);
void UnequipSlot(EquipSlot slot);
void BrewRecipe(int recipeIdx);

// Custom pseudo random helper
static unsigned int g_randSeed = 123456789;
static int RandInt(int min, int max) {
    if (min >= max) return min;
    g_randSeed = (g_randSeed * 1103515245 + 12345) & 0x7fffffff;
    return min + (int)(g_randSeed % (unsigned int)(max - min + 1));
}

void AddLog(const char* text, COLORREF color) {
    if (g_logCount < MAX_LOG_MSGS) {
        snprintf(g_logs[g_logCount].text, sizeof(g_logs[g_logCount].text), "%s", text);
        g_logs[g_logCount].color = color;
        g_logs[g_logCount].turn = g_turn;
        g_logCount++;
    } else {
        for (int i = 0; i < MAX_LOG_MSGS - 1; i++) {
            g_logs[i] = g_logs[i + 1];
        }
        snprintf(g_logs[MAX_LOG_MSGS - 1].text, sizeof(g_logs[MAX_LOG_MSGS - 1].text), "%s", text);
        g_logs[MAX_LOG_MSGS - 1].color = color;
        g_logs[MAX_LOG_MSGS - 1].turn = g_turn;
    }
}

void RecalcPlayerStats(void) {
    int m = g_player.base_might;
    int w = g_player.base_warding;
    int a = g_player.base_arcana + g_staffDefs[g_player.equippedStaff].arcanaBonus;
    int l = g_player.base_light_radius;
    int mhp = g_player.base_max_hp;
    int mmp = g_player.base_max_aether;
    int msan = g_player.base_max_sanity;

    // Rune socket passives
    for (int s = 0; s < g_staffDefs[g_player.equippedStaff].maxSockets; s++) {
        int r = g_player.staffSockets[s];
        if (r == RUNE_PYRE) m += 3;
        else if (r == RUNE_FROST) w += 3;
        else if (r == RUNE_TEMPEST) { m += 2; a += 2; }
        else if (r == RUNE_VOID) { a += 3; l += 1; }
        else if (r == RUNE_AEGIS) { w += 4; msan += 10; }
    }

    // Equipment slot bonuses
    ItemId eq[4] = { g_player.equipWeapon, g_player.equipArmor, g_player.equipRelic, g_player.equipAmulet };
    for (int i = 0; i < 4; i++) {
        if (eq[i] > ITEM_NONE && eq[i] < NUM_ITEM_DEFS) {
            const ItemDef* id = &g_itemDefs[eq[i]];
            m += id->might;
            w += id->warding;
            a += id->arcana;
            l += id->light;
            mhp += id->maxHp;
            mmp += id->maxAether;
            msan += id->maxSanity;
        }
    }

    // Weapon Enchantment passives
    if (g_player.weaponEnchant == ENCHANT_FIRE) {
        m += 2; // Flamebrand: +2 Might
    } else if (g_player.weaponEnchant == ENCHANT_FROST) {
        w += 3; // Frostbite: +3 Warding
    } else if (g_player.weaponEnchant == ENCHANT_VOID) {
        a += 2; // Voidsever: +2 Arcana
    }

    // Torch extinguished effect
    if (!g_player.torchLit) {
        if (l > 1) l = 1;
    }
    // Curse of Darkness
    if (g_player.curse == CURSE_DARKNESS) {
        l -= 3;
        if (l < 1) l = 1;
    }
    // Curse of Enfeeblement
    if (g_player.curse == CURSE_ENFEEBLE) {
        m -= 4;
        w -= 3;
    }
    // Starvation penalty
    if (g_player.hunger <= 15) {
        m -= 2;
        w -= 1;
    }
    // Deep delirium / low sanity panic
    if (g_player.sanity < 20) {
        m -= 2;
    }
    if (m < 1) m = 1;
    if (w < 0) w = 0;
    if (l < 1) l = 1;

    g_player.might = m;
    g_player.warding = w;
    g_player.arcana = a;
    g_player.light_radius = l;
    g_player.max_hp = mhp;
    g_player.max_aether = mmp;
    g_player.max_sanity = msan;

    if (g_player.hp > g_player.max_hp) g_player.hp = g_player.max_hp;
    if (g_player.aether > g_player.max_aether) g_player.aether = g_player.max_aether;
    if (g_player.sanity > g_player.max_sanity) g_player.sanity = g_player.max_sanity;
}

void CheckLevelUp(void) {
    if (g_player.exp >= g_player.max_exp) {
        g_player.exp -= g_player.max_exp;
        g_player.level++;
        g_player.max_exp = (int)(g_player.max_exp * 1.5f);
        g_player.base_max_hp += 15;
        g_player.base_max_aether += 10;
        g_player.base_might += 2;
        g_player.base_warding += 1;
        g_player.base_arcana += 2;
        RecalcPlayerStats();
        g_player.hp = g_player.max_hp;
        g_player.aether = g_player.max_aether;

        char buf[128];
        snprintf(buf, sizeof(buf), "LEVEL UP! Delver reached Level %d! (+15 HP, +10 Aether, +2 Might, +2 Arcana)", g_player.level);
        AddLog(buf, COLOR_ACCENT_PURPLE);
        Beep(880, 70); Beep(1175, 100);
    }
}

BOOL AddPackItem(ItemId id, int count) {
    if (id <= ITEM_NONE || id >= NUM_ITEM_DEFS || count <= 0) return FALSE;
    const ItemDef* idef = &g_itemDefs[id];
    if (idef->category == ITEM_TYPE_CONSUMABLE || idef->category == ITEM_TYPE_REAGENT || idef->category == ITEM_TYPE_KEY) {
        for (int i = 0; i < g_player.numPackItems; i++) {
            if (g_player.pack[i].id == id) {
                g_player.pack[i].count += count;
                return TRUE;
            }
        }
    }
    if (g_player.numPackItems >= MAX_PACK_SLOTS) {
        AddLog("Delver's Pack is full! Cannot carry more items.", COLOR_ACCENT_RED);
        return FALSE;
    }
    g_player.pack[g_player.numPackItems].id = id;
    g_player.pack[g_player.numPackItems].count = count;
    g_player.numPackItems++;
    return TRUE;
}

BOOL RemovePackItem(ItemId id, int count) {
    for (int i = 0; i < g_player.numPackItems; i++) {
        if (g_player.pack[i].id == id) {
            if (g_player.pack[i].count > count) {
                g_player.pack[i].count -= count;
                return TRUE;
            } else if (g_player.pack[i].count == count) {
                for (int j = i; j < g_player.numPackItems - 1; j++) {
                    g_player.pack[j] = g_player.pack[j + 1];
                }
                g_player.numPackItems--;
                return TRUE;
            } else {
                return FALSE;
            }
        }
    }
    return FALSE;
}

int GetPackItemCount(ItemId id) {
    int total = 0;
    for (int i = 0; i < g_player.numPackItems; i++) {
        if (g_player.pack[i].id == id) {
            total += g_player.pack[i].count;
        }
    }
    return total;
}

void EquipPackItem(int packIdx) {
    if (packIdx < 0 || packIdx >= g_player.numPackItems) return;
    ItemId newItem = g_player.pack[packIdx].id;
    const ItemDef* def = &g_itemDefs[newItem];
    if (def->category != ITEM_TYPE_EQUIPMENT) return;

    ItemId oldItem = ITEM_NONE;
    if (def->slot == SLOT_WEAPON) {
        oldItem = g_player.equipWeapon;
        g_player.equipWeapon = newItem;
    } else if (def->slot == SLOT_ARMOR) {
        oldItem = g_player.equipArmor;
        g_player.equipArmor = newItem;
    } else if (def->slot == SLOT_RELIC) {
        oldItem = g_player.equipRelic;
        g_player.equipRelic = newItem;
    } else if (def->slot == SLOT_AMULET) {
        oldItem = g_player.equipAmulet;
        g_player.equipAmulet = newItem;
    } else {
        return;
    }

    RemovePackItem(newItem, 1);
    if (oldItem != ITEM_NONE) {
        AddPackItem(oldItem, 1);
    }

    RecalcPlayerStats();
    char buf[128];
    snprintf(buf, sizeof(buf), "Equipped %s (%s).", def->name, def->desc);
    AddLog(buf, COLOR_TEXT_RUNE);
    Beep(700, 40); Beep(900, 50);
}

void UnequipSlot(EquipSlot slot) {
    ItemId itemToUnequip = ITEM_NONE;
    if (slot == SLOT_WEAPON) itemToUnequip = g_player.equipWeapon;
    else if (slot == SLOT_ARMOR) itemToUnequip = g_player.equipArmor;
    else if (slot == SLOT_RELIC) itemToUnequip = g_player.equipRelic;
    else if (slot == SLOT_AMULET) itemToUnequip = g_player.equipAmulet;

    if (itemToUnequip == ITEM_NONE) return;

    if (g_player.numPackItems >= MAX_PACK_SLOTS) {
        AddLog("Cannot unequip: Delver's Pack is full!", COLOR_ACCENT_RED);
        return;
    }

    if (slot == SLOT_WEAPON) g_player.equipWeapon = ITEM_NONE;
    else if (slot == SLOT_ARMOR) g_player.equipArmor = ITEM_NONE;
    else if (slot == SLOT_RELIC) g_player.equipRelic = ITEM_NONE;
    else if (slot == SLOT_AMULET) g_player.equipAmulet = ITEM_NONE;

    AddPackItem(itemToUnequip, 1);
    RecalcPlayerStats();

    char buf[128];
    snprintf(buf, sizeof(buf), "Unequipped %s to Delver's Pack.", g_itemDefs[itemToUnequip].name);
    AddLog(buf, COLOR_TEXT_DIM);
    Beep(500, 40);
}

void UsePackItem(int packIdx) {
    if (packIdx < 0 || packIdx >= g_player.numPackItems) return;
    ItemId id = g_player.pack[packIdx].id;
    const ItemDef* def = &g_itemDefs[id];

    if (def->category == ITEM_TYPE_EQUIPMENT) {
        EquipPackItem(packIdx);
        return;
    }

    if (def->category != ITEM_TYPE_CONSUMABLE) {
        char buf[128];
        snprintf(buf, sizeof(buf), "%s: %s", def->name, def->desc);
        AddLog(buf, def->color);
        return;
    }

    char logBuf[128];
    logBuf[0] = '\0';

    if (id == ITEM_HEAL_SALVE) {
        int heal = 35;
        g_player.hp += heal;
        if (g_player.hp > g_player.max_hp) g_player.hp = g_player.max_hp;
        snprintf(logBuf, sizeof(logBuf), "Applied Healing Salve (+%d HP).", heal);
        Beep(587, 40); Beep(880, 60);
    } else if (id == ITEM_ELIXIR_VITALITY) {
        g_player.hp += 65;
        if (g_player.hp > g_player.max_hp) g_player.hp = g_player.max_hp;
        g_player.sanity += 10;
        if (g_player.sanity > g_player.max_sanity) g_player.sanity = g_player.max_sanity;
        snprintf(logBuf, sizeof(logBuf), "Drank Elixir of Vitality! (+65 HP, +10 Sanity).");
        Beep(650, 40); Beep(980, 70);
    } else if (id == ITEM_SANITY_INCENSE) {
        g_player.sanity += 30;
        if (g_player.sanity > g_player.max_sanity) g_player.sanity = g_player.max_sanity;
        snprintf(logBuf, sizeof(logBuf), "Burned Sanity Incense (+30 Sanity).");
        Beep(440, 50); Beep(660, 60);
    } else if (id == ITEM_LUCID_DRAUGHT) {
        g_player.sanity += 50;
        if (g_player.sanity > g_player.max_sanity) g_player.sanity = g_player.max_sanity;
        g_player.aether += 10;
        if (g_player.aether > g_player.max_aether) g_player.aether = g_player.max_aether;
        snprintf(logBuf, sizeof(logBuf), "Drank Draught of Lucid Mind! (+50 Sanity, +10 Aether).");
        Beep(520, 50); Beep(780, 60);
    } else if (id == ITEM_AETHER_PHIAL) {
        g_player.aether += 40;
        if (g_player.aether > g_player.max_aether) g_player.aether = g_player.max_aether;
        snprintf(logBuf, sizeof(logBuf), "Drank Aether Phial (+40 Aether).");
        Beep(700, 50); Beep(1050, 70);
    } else if (id == ITEM_STONESKIN_BREW) {
        g_player.shield += 40;
        if (g_player.shield > 80) g_player.shield = 80;
        snprintf(logBuf, sizeof(logBuf), "Drank Stoneskin Brew! Obsidian skin (+40 Ward Shield).");
        Beep(300, 70); Beep(450, 80);
    } else if (id == ITEM_LIQUID_FIRE) {
        int hits = 0;
        for (int m = 0; m < g_numMonsters; m++) {
            if (g_monsters[m].alive && abs(g_monsters[m].x - g_player.x) <= 2 && abs(g_monsters[m].y - g_player.y) <= 2) {
                DamageMonster(m, 50, "FIRE", TRUE);
                hits++;
            }
        }
        if (g_numSpellFX < MAX_SPELL_FX) {
            g_spellFX[g_numSpellFX].type = 0;
            g_spellFX[g_numSpellFX].x = g_player.x;
            g_spellFX[g_numSpellFX].y = g_player.y;
            g_spellFX[g_numSpellFX].radius = 2;
            g_spellFX[g_numSpellFX].duration = 6;
            g_spellFX[g_numSpellFX].color = COLOR_ACCENT_RED;
            g_numSpellFX++;
        }
        snprintf(logBuf, sizeof(logBuf), "Shattered Liquid Fire Flask! Conflagration struck %d enemies!", hits);
        Beep(250, 60); Beep(400, 80);
    } else if (id == ITEM_PANACEA_DEEP) {
        g_player.hp += 60;
        if (g_player.hp > g_player.max_hp) g_player.hp = g_player.max_hp;
        g_player.sanity += 40;
        if (g_player.sanity > g_player.max_sanity) g_player.sanity = g_player.max_sanity;
        g_player.aether += 35;
        if (g_player.aether > g_player.max_aether) g_player.aether = g_player.max_aether;
        g_player.curse = CURSE_NONE;
        g_player.curseTurns = 0;
        snprintf(logBuf, sizeof(logBuf), "Consumed Panacea of the Deep! (+60 HP, +40 Sanity, +35 MP, Curses Cleansed)!");
        Beep(600, 50); Beep(800, 60); Beep(1200, 90);
    } else if (id == ITEM_FOOD_RATIONS) {
        g_player.hunger += 45;
        if (g_player.hunger > g_player.max_hunger) g_player.hunger = g_player.max_hunger;
        g_player.hp += 10;
        if (g_player.hp > g_player.max_hp) g_player.hp = g_player.max_hp;
        snprintf(logBuf, sizeof(logBuf), "Ate Iron Rations (+45 Hunger, +10 HP). Satiated!");
        Beep(450, 40); Beep(600, 50);
    } else if (id == ITEM_CRYPT_MUSHROOM) {
        g_player.hunger += 25;
        if (g_player.hunger > g_player.max_hunger) g_player.hunger = g_player.max_hunger;
        g_player.aether += 5;
        if (g_player.aether > g_player.max_aether) g_player.aether = g_player.max_aether;
        g_player.sanity -= 4;
        if (g_player.sanity < 0) g_player.sanity = 0;
        snprintf(logBuf, sizeof(logBuf), "Ate Crypt Truffle (+25 Hunger, +5 MP, -4 Sanity). Visions swirl!");
        Beep(320, 50); Beep(480, 60);
    } else if (id == ITEM_PURIFYING_SALT) {
        g_player.curse = CURSE_NONE;
        g_player.curseTurns = 0;
        g_player.sanity += 20;
        if (g_player.sanity > g_player.max_sanity) g_player.sanity = g_player.max_sanity;
        snprintf(logBuf, sizeof(logBuf), "Scattered Purifying Salt! All curses dissolved (+20 Sanity)!");
        Beep(660, 50); Beep(880, 80);
    }

    if (logBuf[0]) AddLog(logBuf, def->color);
    RemovePackItem(id, 1);
    AdvanceTurn();
}

void BrewRecipe(int recipeIdx) {
    if (recipeIdx < 0 || recipeIdx >= NUM_RECIPES) return;
    const AlchemyRecipe* rec = &g_recipes[recipeIdx];

    // Check ingredients
    for (int i = 0; i < rec->numIng; i++) {
        if (GetPackItemCount(rec->ing[i].id) < rec->ing[i].count) {
            char buf[128];
            snprintf(buf, sizeof(buf), "Cannot brew %s: Missing %s!", rec->name, g_itemDefs[rec->ing[i].id].name);
            AddLog(buf, COLOR_ACCENT_RED);
            Beep(250, 60);
            return;
        }
    }

    // Check pack space if result is new item
    if (GetPackItemCount(rec->res) == 0 && g_player.numPackItems >= MAX_PACK_SLOTS) {
        BOOL slotFrees = FALSE;
        for (int i = 0; i < rec->numIng; i++) {
            if (GetPackItemCount(rec->ing[i].id) == rec->ing[i].count) {
                slotFrees = TRUE;
                break;
            }
        }
        if (!slotFrees) {
            AddLog("Delver's Pack is full! Make space before brewing.", COLOR_ACCENT_RED);
            return;
        }
    }

    // Consume ingredients
    for (int i = 0; i < rec->numIng; i++) {
        RemovePackItem(rec->ing[i].id, rec->ing[i].count);
    }

    // Add brewed potion
    AddPackItem(rec->res, 1);

    char buf[128];
    snprintf(buf, sizeof(buf), "Ancient Cauldron bubbled! Brewed %s (%s)!", rec->name, rec->desc);
    AddLog(buf, RGB(52, 211, 153));
    Beep(520, 50); Beep(740, 60); Beep(980, 80);
}


void ComputeFOV(void) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            g_visible[y][x] = FALSE;
            g_lightMap[y][x] = 0.0f;
        }
    }

    if (!g_fovEnabled) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                g_visible[y][x] = TRUE;
                g_explored[y][x] = TRUE;
                g_lightMap[y][x] = 1.0f;
            }
        }
        return;
    }

    const int numRays = 360;
    const float radius = (float)g_player.light_radius;

    for (int i = 0; i < numRays; i++) {
        float rad = (float)i * 0.0174532925f; // i * PI / 180
        float cosA = cosf(rad);
        float sinA = sinf(rad);

        float cx = (float)g_player.x + 0.5f;
        float cy = (float)g_player.y + 0.5f;

        for (float d = 0.0f; d <= radius; d += 0.4f) {
            int tx = (int)cx;
            int ty = (int)cy;

            if (tx < 0 || tx >= MAP_WIDTH || ty < 0 || ty >= MAP_HEIGHT) break;

            float dist = sqrtf((float)((tx - g_player.x) * (tx - g_player.x) + (ty - g_player.y) * (ty - g_player.y)));
            if (dist <= radius) {
                g_visible[ty][tx] = TRUE;
                g_explored[ty][tx] = TRUE;
                float falloff = 1.0f - (dist / radius) * 0.75f;
                if (falloff < 0.0f) falloff = 0.0f;
                if (falloff > g_lightMap[ty][tx]) g_lightMap[ty][tx] = falloff;
            }

            int tile = g_dungeon[ty][tx];
            if (tile == TILE_WALL || tile == TILE_DOOR_CLOSED || tile == TILE_PILLAR) {
                break;
            }

            cx += cosA * 0.4f;
            cy += sinA * 0.4f;
        }
    }

    // Ambient torch lighting
    for (int t = 0; t < g_numTorches; t++) {
        if (!g_torches[t].lit) continue;
        int tx = g_torches[t].x;
        int ty = g_torches[t].y;
        if (g_explored[ty][tx]) {
            int tRad = g_torches[t].intensity;
            for (int dy = -tRad; dy <= tRad; dy++) {
                for (int dx = -tRad; dx <= tRad; dx++) {
                    int nx = tx + dx;
                    int ny = ty + dy;
                    if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT) {
                        float d = sqrtf((float)(dx * dx + dy * dy));
                        if (d <= (float)tRad) {
                            float intensity = (1.0f - d / (float)tRad) * 0.6f;
                            g_lightMap[ny][nx] += intensity;
                            if (g_lightMap[ny][nx] > 1.0f) g_lightMap[ny][nx] = 1.0f;
                        }
                    }
                }
            }
        }
    }
}

void SpawnEmber(float x, float y, BOOL isTorch) {
    if (g_numEmbers >= MAX_EMBERS) return;
    DepthZone z = GetDepthZone(g_depthLevel);
    const ZoneTheme* zt = &g_zoneThemes[z];

    g_embers[g_numEmbers].x = x + (float)(RandInt(0, 16) - 8);
    g_embers[g_numEmbers].y = y + (float)(RandInt(0, 12) - 6);
    g_embers[g_numEmbers].vx = ((float)RandInt(0, 100) - 50.0f) * 0.008f;
    g_embers[g_numEmbers].vy = -((float)RandInt(30, 80) * 0.015f);
    g_embers[g_numEmbers].life = 1.0f;
    g_embers[g_numEmbers].decay = 0.02f + ((float)RandInt(0, 50) * 0.0004f);
    g_embers[g_numEmbers].color = isTorch ? (RandInt(0, 10) > 4 ? zt->particleColor1 : zt->particleColor2) : (RandInt(0, 10) > 5 ? COLOR_BORDER_GLOW : COLOR_TEXT_RUNE);
    g_numEmbers++;
}

void UpdateEmbers(void) {
    for (int i = 0; i < g_numEmbers; ) {
        g_embers[i].x += g_embers[i].vx;
        g_embers[i].y += g_embers[i].vy;
        g_embers[i].life -= g_embers[i].decay;
        if (g_embers[i].life <= 0.0f) {
            g_embers[i] = g_embers[g_numEmbers - 1];
            g_numEmbers--;
        } else {
            i++;
        }
    }
}

// --- Monster Bestiary & Tactical Turn-Based AI System ---
BOOL CheckLOS(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    int cx = x0;
    int cy = y0;

    while (1) {
        if (cx == x1 && cy == y1) return TRUE;
        if (cx != x0 || cy != y0) {
            if (cx < 0 || cx >= MAP_WIDTH || cy < 0 || cy >= MAP_HEIGHT) return FALSE;
            int tile = g_dungeon[cy][cx];
            if (tile == TILE_WALL || tile == TILE_PILLAR || tile == TILE_DOOR_CLOSED) {
                return FALSE;
            }
        }
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            cx += sx;
        }
        if (e2 < dx) {
            err += dx;
            cy += sy;
        }
    }
}

void SpawnCombatText(float x, float y, const char* text, COLORREF color) {
    if (g_numCombatTexts < MAX_COMBAT_TEXTS) {
        g_combatTexts[g_numCombatTexts].x = x;
        g_combatTexts[g_numCombatTexts].y = y;
        snprintf(g_combatTexts[g_numCombatTexts].text, sizeof(g_combatTexts[g_numCombatTexts].text), "%s", text);
        g_combatTexts[g_numCombatTexts].color = color;
        g_combatTexts[g_numCombatTexts].life = 1.0f;
        g_numCombatTexts++;
    }
}

void SpawnMonsters(int level) {
    g_numMonsters = 0;
    DepthZone zone = GetDepthZone(level);
    int types[6];
    int numTypes = 0;

    if (zone == ZONE_CATACOMBS) {
        types[numTypes++] = MONSTER_SKELETON;
        types[numTypes++] = MONSTER_SKELETON;
        if (level >= 3) types[numTypes++] = MONSTER_ACOLYTE;
    } else if (zone == ZONE_SUNKEN_GROTTO) {
        types[numTypes++] = MONSTER_SKELETON;
        types[numTypes++] = MONSTER_GHOUL;
        types[numTypes++] = MONSTER_GHOUL;
        if (level >= 5) types[numTypes++] = MONSTER_ACOLYTE;
    } else if (zone == ZONE_FORGOTTEN_CRYPT) {
        types[numTypes++] = MONSTER_SKELETON;
        types[numTypes++] = MONSTER_WRAITH;
        types[numTypes++] = MONSTER_ACOLYTE;
        types[numTypes++] = MONSTER_WRAITH;
    } else { // ZONE_VOID_ABYSS
        types[numTypes++] = MONSTER_WRAITH;
        types[numTypes++] = MONSTER_WRAITH;
        types[numTypes++] = MONSTER_LEVIATHAN;
    }

    int numToSpawn = 5 + (int)(level * 0.8f) + RandInt(0, 2);
    if (numToSpawn > MAX_MONSTERS - 2) numToSpawn = MAX_MONSTERS - 2;

    for (int i = 0; i < numToSpawn; i++) {
        int tIdx = types[RandInt(0, numTypes - 1)];
        const MonsterDef* md = &g_monsterDefs[tIdx];

        int attempts = 0;
        BOOL spawned = FALSE;
        while (attempts < 80 && !spawned) {
            attempts++;
            int rx = RandInt(2, MAP_WIDTH - 3);
            int ry = RandInt(2, MAP_HEIGHT - 3);
            int t = g_dungeon[ry][rx];

            if ((t == TILE_FLOOR || t == TILE_WATER) && t != TILE_STAIRS_UP && t != TILE_STAIRS_DOWN && t != TILE_CHEST && t != TILE_ALTAR && t != TILE_SHRINE && t != TILE_MERCHANT && t != TILE_CAULDRON) {
                int dist = (int)sqrtf((float)((rx - g_player.x) * (rx - g_player.x) + (ry - g_player.y) * (ry - g_player.y)));
                if (dist >= 6) {
                    BOOL occupied = FALSE;
                    for (int m = 0; m < g_numMonsters; m++) {
                        if (g_monsters[m].x == rx && g_monsters[m].y == ry) { occupied = TRUE; break; }
                    }
                    if (!occupied) {
                        int hp = md->baseHp + (level - 1) * md->hpScale;
                        int atk = md->baseAtk + (level - 1) * md->atkScale;
                        g_monsters[g_numMonsters].type = tIdx;
                        g_monsters[g_numMonsters].x = rx;
                        g_monsters[g_numMonsters].y = ry;
                        g_monsters[g_numMonsters].hp = hp;
                        g_monsters[g_numMonsters].max_hp = hp;
                        g_monsters[g_numMonsters].atk = atk;
                        g_monsters[g_numMonsters].exp = md->exp + level * 2;
                        g_monsters[g_numMonsters].essence = md->essence + level * 2;
                        g_monsters[g_numMonsters].state = 0;
                        g_monsters[g_numMonsters].freezeTurns = 0;
                        g_monsters[g_numMonsters].alertRange = (tIdx == MONSTER_LEVIATHAN ? 10 : (tIdx == MONSTER_WRAITH ? 9 : 7));
                        g_monsters[g_numMonsters].isBoss = FALSE;
                        g_monsters[g_numMonsters].bonePlated = FALSE;
                        g_monsters[g_numMonsters].alive = TRUE;
                        g_numMonsters++;
                        spawned = TRUE;
                    }
                }
            }
        }
    }

    // --- Abyssal Lord Boss Encounter Spawning ---
    int bossType = -1;
    if (level == 3) {
        bossType = MONSTER_CRYPT_KEEPER;
    } else if (level == 6) {
        bossType = MONSTER_ABYSSAL_WYRM;
    } else if (level >= 10 && (level == 10 || level % 4 == 0)) {
        bossType = MONSTER_VOID_MONARCH;
    }

    if (bossType >= 0 && g_numMonsters < MAX_MONSTERS) {
        const MonsterDef* bmd = &g_monsterDefs[bossType];
        int bAttempts = 0;
        BOOL bSpawned = FALSE;
        while (bAttempts < 120 && !bSpawned) {
            bAttempts++;
            int rx = RandInt(3, MAP_WIDTH - 4);
            int ry = RandInt(3, MAP_HEIGHT - 4);
            int t = g_dungeon[ry][rx];

            if ((t == TILE_FLOOR || t == TILE_WATER) && t != TILE_STAIRS_UP && t != TILE_STAIRS_DOWN && t != TILE_CHEST && t != TILE_ALTAR && t != TILE_SHRINE && t != TILE_MERCHANT && t != TILE_CAULDRON) {
                int dist = (int)sqrtf((float)((rx - g_player.x) * (rx - g_player.x) + (ry - g_player.y) * (ry - g_player.y)));
                if (dist >= 7) {
                    BOOL occupied = FALSE;
                    for (int m = 0; m < g_numMonsters; m++) {
                        if (g_monsters[m].x == rx && g_monsters[m].y == ry) { occupied = TRUE; break; }
                    }
                    if (!occupied) {
                        int hp = bmd->baseHp + (level - 1) * bmd->hpScale;
                        int atk = bmd->baseAtk + (level - 1) * bmd->atkScale;
                        g_monsters[g_numMonsters].type = bossType;
                        g_monsters[g_numMonsters].x = rx;
                        g_monsters[g_numMonsters].y = ry;
                        g_monsters[g_numMonsters].hp = hp;
                        g_monsters[g_numMonsters].max_hp = hp;
                        g_monsters[g_numMonsters].atk = atk;
                        g_monsters[g_numMonsters].exp = bmd->exp + level * 10;
                        g_monsters[g_numMonsters].essence = bmd->essence + level * 10;
                        g_monsters[g_numMonsters].state = 1; // Alert
                        g_monsters[g_numMonsters].freezeTurns = 0;
                        g_monsters[g_numMonsters].alertRange = 14;
                        g_monsters[g_numMonsters].isBoss = TRUE;
                        g_monsters[g_numMonsters].bonePlated = (bossType == MONSTER_CRYPT_KEEPER);
                        g_monsters[g_numMonsters].alive = TRUE;
                        g_numMonsters++;
                        bSpawned = TRUE;

                        if (bossType == MONSTER_CRYPT_KEEPER) {
                            AddLog("ABYSSAL LORD RISES: The Crypt Keeper stirs in the catacomb sepulcher!", COLOR_ACCENT_AMBER);
                            Beep(220, 60); Beep(160, 100);
                        } else if (bossType == MONSTER_ABYSSAL_WYRM) {
                            AddLog("ABYSSAL LORD RISES: The sunken waters churn—The Abyssal Wyrm coils from the depths!", COLOR_ACCENT_GREEN);
                            Beep(180, 60); Beep(240, 80);
                        } else if (bossType == MONSTER_VOID_MONARCH) {
                            AddLog("ABYSSAL LORD RISES: Reality tears asunder—The Void Monarch commands the abyssal vortex!", COLOR_ACCENT_PURPLE);
                            Beep(140, 80); Beep(280, 120);
                        }
                    }
                }
            }
        }
    }
}

void DamageMonster(int idx, int dmg, const char* dmgType, BOOL isCrit) {
    if (idx < 0 || idx >= g_numMonsters || !g_monsters[idx].alive) return;
    Monster* m = &g_monsters[idx];
    const MonsterDef* md = &g_monsterDefs[m->type];

    m->hp -= dmg;
    m->state = 1; // Alert

    char txt[32];
    snprintf(txt, sizeof(txt), "%s-%d", isCrit ? "CRIT! " : "", dmg);
    COLORREF col = RGB(248, 113, 113);
    if (strcmp(dmgType, "FIRE") == 0) col = RGB(249, 115, 22);
    else if (strcmp(dmgType, "CRYO") == 0) col = RGB(6, 182, 212);
    else if (strcmp(dmgType, "SHOCK") == 0) col = RGB(234, 179, 8);
    SpawnCombatText((float)m->x, (float)m->y, txt, col);

    if (m->hp <= 0) {
        m->alive = FALSE;
        g_player.exp += m->exp;
        g_player.essence += m->essence;

        char buf[128];
        if (m->isBoss) {
            snprintf(buf, sizeof(buf), "ABYSSAL LORD DEFEATED: You conquered %s! (+%d EXP, +%d Gold)", md->name, m->exp, m->essence);
            AddLog(buf, COLOR_TEXT_GOLD);
            Beep(440, 80); Beep(554, 80); Beep(659, 120); Beep(880, 200);
        } else {
            snprintf(buf, sizeof(buf), "SLAIN: You destroyed %s! (+%d EXP, +%d Essence)", md->name, m->exp, m->essence);
            AddLog(buf, COLOR_TEXT_GOLD);
            Beep(330, 40); Beep(165, 80);
        }

        // Chance to discover an unowned rune
        int unowned[NUM_RUNES];
        int unCount = 0;
        for (int r = 0; r < NUM_RUNES; r++) {
            if (!g_player.ownedRunes[r]) unowned[unCount++] = r;
        }
        if (unCount > 0 && (m->isBoss || RandInt(0, 100) < 35)) {
            int rPick = unowned[RandInt(0, unCount - 1)];
            g_player.ownedRunes[rPick] = TRUE;
            char rBuf[128];
            snprintf(rBuf, sizeof(rBuf), "Bestiary Spoils: Discovered %s (%s)! Inscribe in Tab [3].", g_runeDefs[rPick].name, g_runeDefs[rPick].symbol);
            AddLog(rBuf, COLOR_ACCENT_CYAN);
            Beep(880, 50); Beep(1175, 70);
        }

        // Boss drops
        if (m->type == MONSTER_CRYPT_KEEPER) {
            AddPackItem(ITEM_WPN_CRYPT_GREATSWORD, 1);
            AddPackItem(ITEM_KEY_RUNIC, 1);
            AddLog("Spoils of Catacombs: Discovered Keeper's Greatsword (+10 Might, +3 Ward) & Key!", COLOR_TEXT_GOLD);
        } else if (m->type == MONSTER_ABYSSAL_WYRM) {
            AddPackItem(ITEM_ARM_WYRMSCALE, 1);
            AddPackItem(ITEM_PANACEA_DEEP, 1);
            AddLog("Spoils of Grotto: Discovered Wyrmscale Carapace (+9 Ward, +40 HP) & Panacea!", COLOR_TEXT_GOLD);
        } else if (m->type == MONSTER_VOID_MONARCH) {
            AddPackItem(ITEM_REL_MONARCH_CROWN, 1);
            AddPackItem(ITEM_PANACEA_DEEP, 1);
            AddLog("Spoils of the Void: Discovered Crown of Void Monarch (+10 Light, +35 MP/SAN)!", COLOR_TEXT_GOLD);
        } else if (m->type == MONSTER_GHOUL) {
            if (RandInt(0, 100) < 55) { AddPackItem(ITEM_ING_AZURE_SPORES, 1); AddLog("Harvested Azure Spores from the mire ghoul.", RGB(52, 211, 153)); }
            if (RandInt(0, 100) < 25) { AddPackItem(ITEM_HEAL_SALVE, 1); AddLog("Salvaged Healing Salve from remains.", COLOR_ACCENT_GREEN); }
        } else if (m->type == MONSTER_SKELETON) {
            if (RandInt(0, 100) < 45) { AddPackItem(ITEM_ING_BRIMSTONE, 1); AddLog("Harvested Brimstone Ash from crypt bones.", RGB(245, 158, 11)); }
            if (RandInt(0, 100) < 15) { AddPackItem(ITEM_KEY_RUNIC, 1); AddLog("Found Ancient Runic Key among the bones!", COLOR_TEXT_GOLD); }
        } else if (m->type == MONSTER_WRAITH) {
            if (RandInt(0, 100) < 60) { AddPackItem(ITEM_ING_VOID_DUST, 1); AddLog("Collected Void Dust from the dissipating wraith.", RGB(168, 85, 247)); }
            if (RandInt(0, 100) < 30) { AddPackItem(ITEM_AETHER_PHIAL, 1); AddLog("Found glowing Aether Phial in the ethereal residue.", COLOR_ACCENT_CYAN); }
        } else if (m->type == MONSTER_ACOLYTE) {
            if (RandInt(0, 100) < 50) { AddPackItem(ITEM_ING_BLOOD_LOTUS, 1); AddLog("Harvested Blood Lotus from the acolyte's pouch.", RGB(244, 63, 94)); }
            if (RandInt(0, 100) < 30) { AddPackItem(ITEM_LUCID_DRAUGHT, 1); AddLog("Found Draught of Lucid Mind on the acolyte.", COLOR_BORDER_GLOW); }
        } else if (m->type == MONSTER_LEVIATHAN) {
            AddPackItem(ITEM_ING_AETHER_BLOSSOM, 1);
            AddLog("Harvested luminous Aether Blossom from the fallen Leviathan!", RGB(56, 189, 248));
            if (RandInt(0, 100) < 50) { AddPackItem(ITEM_PANACEA_DEEP, 1); AddLog("Discovered rare Panacea of the Deep!", COLOR_TEXT_GOLD); }
            if (RandInt(0, 100) < 35) { AddPackItem(ITEM_ARM_AEGIS_CUIRASS, 1); AddLog("Discovered Aegis Cuirass (+7 Def)!", COLOR_TEXT_RUNE); }
        }

        CheckLevelUp();
    } else {
        char buf[128];
        snprintf(buf, sizeof(buf), "Hit %s for %d %s DMG! (%d/%d HP)", md->name, dmg, dmgType, m->hp, m->max_hp);
        AddLog(buf, COLOR_ACCENT_RED);
        Beep(520, 30);

        // Dimensional Phase Blink for Void Monarch
        if (m->type == MONSTER_VOID_MONARCH && RandInt(0, 100) < 35) {
            for (int bAtt = 0; bAtt < 15; bAtt++) {
                int bx = m->x + RandInt(-2, 2);
                int by = m->y + RandInt(-2, 2);
                if (bx >= 1 && bx < MAP_WIDTH - 1 && by >= 1 && by < MAP_HEIGHT - 1) {
                    int bt = g_dungeon[by][bx];
                    if ((bt == TILE_FLOOR || bt == TILE_WATER) && (bx != g_player.x || by != g_player.y)) {
                        BOOL occ = FALSE;
                        for (int o = 0; o < g_numMonsters; o++) {
                            if (o != idx && g_monsters[o].alive && g_monsters[o].x == bx && g_monsters[o].y == by) { occ = TRUE; break; }
                        }
                        if (!occ) {
                            m->x = bx; m->y = by;
                            SpawnCombatText((float)bx, (float)by, "PHASE BLINK!", RGB(192, 132, 252));
                            AddLog("DIMENSIONAL BLINK: The Void Monarch folds space, phasing across the chamber!", COLOR_ACCENT_PURPLE);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void AttackMonster(int idx) {
    if (idx < 0 || idx >= g_numMonsters) return;
    int baseDmg = 12 + g_player.might / 2 + g_player.arcana / 3 + RandInt(0, 5);
    baseDmg += g_staffDefs[g_player.equippedStaff].arcanaBonus;

    // Crypt Keeper Bone Plating Check
    if (g_monsters[idx].type == MONSTER_CRYPT_KEEPER && g_monsters[idx].bonePlated) {
        if (g_player.weaponEnchant == ENCHANT_FIRE) {
            g_monsters[idx].bonePlated = FALSE;
            baseDmg = (int)(baseDmg * 1.75f);
            AddLog("BONE SHATTER: Sacred flame incinerates The Crypt Keeper's bone plating (+75% DMG)!", RGB(249, 115, 22));
            SpawnCombatText((float)g_monsters[idx].x, (float)g_monsters[idx].y, "BONE SHIELD BROKEN", RGB(249, 115, 22));
        } else {
            baseDmg = (int)(baseDmg * 0.75f);
            AddLog("BONE PLATING: The Crypt Keeper's heavy ossified armor deflects 25% damage!", COLOR_TEXT_DIM);
        }
    } else if (g_monsters[idx].type == MONSTER_ABYSSAL_WYRM && g_player.weaponEnchant == ENCHANT_FROST) {
        baseDmg = (int)(baseDmg * 1.4f);
        AddLog("GLACIAL FRACTURE: Frost rime crystallizes the Abyssal Wyrm's scales (+40% DMG)!", COLOR_ACCENT_CYAN);
    }

    BOOL isCrit = (RandInt(0, 100) < 15);
    if (isCrit) baseDmg = (int)(baseDmg * 1.5f);

    DamageMonster(idx, baseDmg, "PHYSICAL", isCrit);

    // Phase 10: Weapon Enchantment Elemental Bursts
    if (g_monsters[idx].alive && g_player.weaponEnchant != ENCHANT_NONE) {
        if (g_player.weaponEnchant == ENCHANT_FIRE) {
            int fireDmg = RandInt(10, 16);
            if (g_monsters[idx].type == MONSTER_SKELETON || g_monsters[idx].type == MONSTER_CRYPT_KEEPER) {
                fireDmg = (int)(fireDmg * 1.5f);
            }
            DamageMonster(idx, fireDmg, "FIRE", FALSE);
            AddLog("Flamebrand bursts with primordial fire!", RGB(249, 115, 22));
            Beep(700, 30);
        } else if (g_player.weaponEnchant == ENCHANT_FROST) {
            int cryoDmg = RandInt(8, 14);
            DamageMonster(idx, cryoDmg, "CRYO", FALSE);
            if (RandInt(0, 100) < 35 && g_monsters[idx].alive) {
                g_monsters[idx].freezeTurns = 2;
                char fzBuf[96];
                snprintf(fzBuf, sizeof(fzBuf), "Frostbite encases %s in permafrost (2 turns)!", g_monsterDefs[g_monsters[idx].type].name);
                AddLog(fzBuf, RGB(6, 182, 212));
            }
            Beep(850, 30);
        } else if (g_player.weaponEnchant == ENCHANT_VOID) {
            int voidDmg = RandInt(12, 20);
            DamageMonster(idx, voidDmg, "VOID", FALSE);
            g_player.aether += 4;
            if (g_player.aether > g_player.max_aether) g_player.aether = g_player.max_aether;
            g_player.sanity += 3;
            if (g_player.sanity > g_player.max_sanity) g_player.sanity = g_player.max_sanity;
            AddLog("Voidsever cleaves the soul! (+4 Aether, +3 Sanity siphoned)", RGB(168, 85, 247));
            Beep(920, 35);
        }
    }

    AdvanceTurn();
}

void UpdateMonsters(void) {
    for (int m = 0; m < g_numMonsters; m++) {
        if (!g_monsters[m].alive) continue;
        Monster* mon = &g_monsters[m];
        const MonsterDef* md = &g_monsterDefs[mon->type];

        if (mon->freezeTurns > 0) {
            mon->freezeTurns--;
            char fBuf[128];
            snprintf(fBuf, sizeof(fBuf), "%s is encased in glacial ice and cannot act!", md->name);
            AddLog(fBuf, COLOR_ACCENT_CYAN);
            continue;
        }

        float dist = sqrtf((float)((g_player.x - mon->x) * (g_player.x - mon->x) + (g_player.y - mon->y) * (g_player.y - mon->y)));
        BOOL hasLOS = CheckLOS(mon->x, mon->y, g_player.x, g_player.y);

        if (dist <= (float)mon->alertRange && hasLOS) {
            mon->state = 1;
        }

        // Void Monarch Passive Dread Aura
        if (mon->type == MONSTER_VOID_MONARCH && dist <= 8.0f && hasLOS && (g_turn % 2 == 0)) {
            if (g_player.shield <= 0) {
                g_player.sanity = (g_player.sanity > 0) ? (g_player.sanity - 1) : 0;
                AddLog("DREAD AURA: The Void Monarch crushes your sanity (-1 Sanity)!", COLOR_ACCENT_PURPLE);
                SpawnCombatText((float)g_player.x, (float)g_player.y, "-1 SAN", RGB(192, 132, 252));
            }
        }

        if (mon->state == 1) {
            BOOL isAdj = (abs(g_player.x - mon->x) <= 1 && abs(g_player.y - mon->y) <= 1);
            if (isAdj) {
                // Melee strike
                int rawDmg = mon->atk + RandInt(0, 4) - 2;

                // Crypt Keeper Tomb Cleave
                if (mon->type == MONSTER_CRYPT_KEEPER && RandInt(0, 100) < 35) {
                    rawDmg = (int)(rawDmg * 1.4f);
                    AddLog("TOMB CLEAVE: The Crypt Keeper sweeps his massive greatsword!", COLOR_ACCENT_AMBER);
                    int kdx = (g_player.x > mon->x) ? 1 : ((g_player.x < mon->x) ? -1 : 0);
                    int kdy = (g_player.y > mon->y) ? 1 : ((g_player.y < mon->y) ? -1 : 0);
                    int kx = g_player.x + kdx;
                    int ky = g_player.y + kdy;
                    if (kx >= 1 && kx < MAP_WIDTH - 1 && ky >= 1 && ky < MAP_HEIGHT - 1) {
                        int kt = g_dungeon[ky][kx];
                        if (kt == TILE_FLOOR || kt == TILE_WATER) {
                            g_player.x = kx; g_player.y = ky;
                            SpawnCombatText((float)kx, (float)ky, "KNOCKBACK!", RGB(245, 158, 11));
                        }
                    }
                }

                int netDmg = rawDmg - g_player.warding / 3;
                if (netDmg < 3) netDmg = 3;

                if (g_player.shield > 0) {
                    if (g_player.shield >= netDmg) {
                        g_player.shield -= netDmg;
                        netDmg = 0;
                        SpawnCombatText((float)g_player.x, (float)g_player.y, "ABSORB", RGB(56, 189, 248));
                        AddLog("Aegis Ward absorbs the monster's blow!", COLOR_BORDER_GLOW);
                    } else {
                        netDmg -= g_player.shield;
                        g_player.shield = 0;
                        SpawnCombatText((float)g_player.x, (float)g_player.y, "WARD BROKE", RGB(56, 189, 248));
                        AddLog("Aegis Ward absorbed partial damage and collapsed!", COLOR_ACCENT_AMBER);
                    }
                }

                if (netDmg > 0) {
                    g_player.hp -= netDmg;
                    if (g_player.hp < 0) g_player.hp = 0;
                    char dTxt[32];
                    snprintf(dTxt, sizeof(dTxt), "-%d", netDmg);
                    SpawnCombatText((float)g_player.x, (float)g_player.y, dTxt, COLOR_ACCENT_RED);

                    char aBuf[128];
                    snprintf(aBuf, sizeof(aBuf), "%s strikes you for %d DMG! (%d/%d HP)", md->name, netDmg, g_player.hp, g_player.max_hp);
                    AddLog(aBuf, COLOR_ACCENT_RED);
                    Beep(180, 40);
                }

                if (mon->type == MONSTER_WRAITH) {
                    g_player.sanity = (g_player.sanity > 3) ? (g_player.sanity - 3) : 0;
                    g_player.aether = (g_player.aether > 4) ? (g_player.aether - 4) : 0;
                    AddLog("Void Wraith drains your mind & spirit (-3 Sanity, -4 Aether)!", COLOR_ACCENT_PURPLE);
                } else if (mon->type == MONSTER_LEVIATHAN) {
                    g_player.sanity = (g_player.sanity > 2) ? (g_player.sanity - 2) : 0;
                    AddLog("Abyssal Leviathan's crushing slam shakes the floor (-2 Sanity)!", COLOR_ACCENT_AMBER);
                } else if (mon->type == MONSTER_ABYSSAL_WYRM) {
                    g_player.sanity = (g_player.sanity > 3) ? (g_player.sanity - 3) : 0;
                    AddLog("Abyssal Wyrm coils crush your breath (-3 Sanity)!", COLOR_ACCENT_GREEN);
                } else if (mon->type == MONSTER_VOID_MONARCH) {
                    g_player.sanity = (g_player.sanity > 3) ? (g_player.sanity - 3) : 0;
                    g_player.aether = (g_player.aether > 5) ? (g_player.aether - 5) : 0;
                    AddLog("The Void Monarch siphons soul and mind (-3 Sanity, -5 Aether)!", COLOR_ACCENT_PURPLE);
                }

                // Crypt Keeper minion summons
                if (mon->type == MONSTER_CRYPT_KEEPER && mon->hp < mon->max_hp * 6 / 10 && RandInt(0, 100) < 25) {
                    int skelCount = 0;
                    for (int s = 0; s < g_numMonsters; s++) {
                        if (g_monsters[s].alive && g_monsters[s].type == MONSTER_SKELETON) skelCount++;
                    }
                    if (skelCount < 2 && g_numMonsters < MAX_MONSTERS) {
                        for (int s = 0; s < 2 && g_numMonsters < MAX_MONSTERS; s++) {
                            int sx = mon->x + (s == 0 ? 1 : -1);
                            int sy = mon->y + (s == 0 ? -1 : 1);
                            if (sx >= 1 && sx < MAP_WIDTH - 1 && sy >= 1 && sy < MAP_HEIGHT - 1) {
                                int st = g_dungeon[sy][sx];
                                if (st == TILE_FLOOR || st == TILE_WATER) {
                                    g_monsters[g_numMonsters].type = MONSTER_SKELETON;
                                    g_monsters[g_numMonsters].x = sx;
                                    g_monsters[g_numMonsters].y = sy;
                                    g_monsters[g_numMonsters].hp = g_monsterDefs[MONSTER_SKELETON].baseHp;
                                    g_monsters[g_numMonsters].max_hp = g_monsterDefs[MONSTER_SKELETON].baseHp;
                                    g_monsters[g_numMonsters].atk = g_monsterDefs[MONSTER_SKELETON].baseAtk;
                                    g_monsters[g_numMonsters].exp = 15;
                                    g_monsters[g_numMonsters].essence = 10;
                                    g_monsters[g_numMonsters].state = 1;
                                    g_monsters[g_numMonsters].freezeTurns = 0;
                                    g_monsters[g_numMonsters].alertRange = 8;
                                    g_monsters[g_numMonsters].isBoss = FALSE;
                                    g_monsters[g_numMonsters].bonePlated = FALSE;
                                    g_monsters[g_numMonsters].alive = TRUE;
                                    g_numMonsters++;
                                }
                            }
                        }
                        AddLog("TOMB AWAKENING: The Crypt Keeper summons Crypt Skeletons!", COLOR_ACCENT_AMBER);
                        SpawnCombatText((float)mon->x, (float)mon->y, "SUMMON UNDEAD", RGB(245, 158, 11));
                    }
                }

                if (g_player.hp <= 0) {
                    AddLog("You have fallen in the Abyss! Press F2 / Ctrl+N to descend anew.", COLOR_ACCENT_RED);
                }
            } else if (mon->type == MONSTER_ABYSSAL_WYRM && dist <= 5.0f && hasLOS) {
                // Caustic Acid Spit
                int rawDmg = mon->atk + RandInt(0, 4);
                int netDmg = rawDmg - g_player.warding / 4;
                if (netDmg < 4) netDmg = 4;
                if (g_player.shield > 0) {
                    if (g_player.shield >= netDmg) { g_player.shield -= netDmg; netDmg = 0; }
                    else { netDmg -= g_player.shield; g_player.shield = 0; }
                }
                if (netDmg > 0) {
                    g_player.hp -= netDmg;
                    if (g_player.hp < 0) g_player.hp = 0;
                    g_player.hunger = (g_player.hunger > 8) ? (g_player.hunger - 8) : 0;
                    char dTxt[32];
                    snprintf(dTxt, sizeof(dTxt), "-%d ACID", netDmg);
                    SpawnCombatText((float)g_player.x, (float)g_player.y, dTxt, COLOR_ACCENT_GREEN);
                    char bBuf[128];
                    snprintf(bBuf, sizeof(bBuf), "CAUSTIC BILE: %s spews acid for %d DMG (-8 Hunger)!", md->name, netDmg);
                    AddLog(bBuf, COLOR_ACCENT_GREEN);
                    Beep(210, 40);
                }
            } else if (mon->type == MONSTER_VOID_MONARCH && dist <= 6.0f && hasLOS) {
                // Cosmic Collapse Beam
                int rawDmg = mon->atk + RandInt(0, 6) + 4;
                int netDmg = rawDmg - g_player.warding / 5;
                if (netDmg < 6) netDmg = 6;
                if (g_player.shield > 0) {
                    if (g_player.shield >= netDmg) { g_player.shield -= netDmg; netDmg = 0; }
                    else { netDmg -= g_player.shield; g_player.shield = 0; }
                }
                if (netDmg > 0) {
                    g_player.hp -= netDmg;
                    if (g_player.hp < 0) g_player.hp = 0;
                    char dTxt[32];
                    snprintf(dTxt, sizeof(dTxt), "-%d VOID", netDmg);
                    SpawnCombatText((float)g_player.x, (float)g_player.y, dTxt, COLOR_ACCENT_PURPLE);
                    char bBuf[128];
                    snprintf(bBuf, sizeof(bBuf), "COSMIC COLLAPSE: %s channels astral annihilation for %d DMG!", md->name, netDmg);
                    AddLog(bBuf, COLOR_ACCENT_PURPLE);
                    Beep(160, 60);
                }
            } else if (mon->type == MONSTER_ACOLYTE && dist <= 4.0f && hasLOS) {
                // Ranged Shadow Bolt
                int rawDmg = mon->atk + RandInt(0, 3);
                int netDmg = rawDmg - g_player.warding / 4;
                if (netDmg < 3) netDmg = 3;

                if (g_player.shield > 0) {
                    if (g_player.shield >= netDmg) {
                        g_player.shield -= netDmg;
                        netDmg = 0;
                    } else {
                        netDmg -= g_player.shield;
                        g_player.shield = 0;
                    }
                }

                if (netDmg > 0) {
                    g_player.hp -= netDmg;
                    if (g_player.hp < 0) g_player.hp = 0;
                    char dTxt[32];
                    snprintf(dTxt, sizeof(dTxt), "-%d SHADOW", netDmg);
                    SpawnCombatText((float)g_player.x, (float)g_player.y, dTxt, COLOR_ACCENT_PURPLE);

                    char bBuf[128];
                    snprintf(bBuf, sizeof(bBuf), "%s casts Shadow Bolt for %d DMG!", md->name, netDmg);
                    AddLog(bBuf, COLOR_ACCENT_RED);
                    Beep(260, 40);
                }
            } else {
                // Gravitational Singularity for Void Monarch
                if (mon->type == MONSTER_VOID_MONARCH && dist > 3.0f && RandInt(0, 100) < 35) {
                    int pullDx = (mon->x > g_player.x) ? 1 : ((mon->x < g_player.x) ? -1 : 0);
                    int pullDy = (mon->y > g_player.y) ? 1 : ((mon->y < g_player.y) ? -1 : 0);
                    int px = g_player.x + pullDx;
                    int py = g_player.y + pullDy;
                    if (px >= 1 && px < MAP_WIDTH - 1 && py >= 1 && py < MAP_HEIGHT - 1) {
                        int pt = g_dungeon[py][px];
                        if (pt == TILE_FLOOR || pt == TILE_WATER) {
                            g_player.x = px; g_player.y = py;
                            SpawnCombatText((float)px, (float)py, "GRAVITY PULL", RGB(192, 132, 252));
                            AddLog("GRAVITATIONAL SINGULARITY: The Void Monarch drags you into the vortex!", COLOR_ACCENT_PURPLE);
                        }
                    }
                }
                // Pathfinding towards player
                int bestDx = 0, bestDy = 0;
                float bestDist = dist;

                int dirs[8][2] = {
                    {1, 0}, {-1, 0}, {0, 1}, {0, -1},
                    {1, 1}, {-1, -1}, {1, -1}, {-1, 1}
                };

                for (int d = 0; d < 8; d++) {
                    int testX = mon->x + dirs[d][0];
                    int testY = mon->y + dirs[d][1];
                    if (testX < 1 || testX >= MAP_WIDTH - 1 || testY < 1 || testY >= MAP_HEIGHT - 1) continue;
                    if (testX == g_player.x && testY == g_player.y) continue;

                    int tile = g_dungeon[testY][testX];
                    BOOL isWraith = (mon->type == MONSTER_WRAITH);
                    if (!isWraith) {
                        if (tile == TILE_WALL || tile == TILE_PILLAR || tile == TILE_CHASM || tile == TILE_DOOR_CLOSED || tile == TILE_MERCHANT) continue;
                    } else {
                        if (tile == TILE_WALL && (testX == 0 || testX == MAP_WIDTH - 1 || testY == 0 || testY == MAP_HEIGHT - 1)) continue;
                    }

                    BOOL occ = FALSE;
                    for (int o = 0; o < g_numMonsters; o++) {
                        if (o != m && g_monsters[o].alive && g_monsters[o].x == testX && g_monsters[o].y == testY) {
                            occ = TRUE; break;
                        }
                    }
                    if (occ) continue;

                    float newD = sqrtf((float)((g_player.x - testX) * (g_player.x - testX) + (g_player.y - testY) * (g_player.y - testY)));
                    if (newD < bestDist) {
                        bestDist = newD;
                        bestDx = dirs[d][0];
                        bestDy = dirs[d][1];
                    }
                }

                if (bestDx != 0 || bestDy != 0) {
                    mon->x += bestDx;
                    mon->y += bestDy;
                }
            }
        } else {
            // Wander
            if (RandInt(0, 100) < 25) {
                int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
                int pickD = RandInt(0, 3);
                int testX = mon->x + dirs[pickD][0];
                int testY = mon->y + dirs[pickD][1];
                if (testX > 0 && testX < MAP_WIDTH - 1 && testY > 0 && testY < MAP_HEIGHT - 1) {
                    int t = g_dungeon[testY][testX];
                    if (t == TILE_FLOOR || t == TILE_WATER) {
                        BOOL occ = FALSE;
                        for (int o = 0; o < g_numMonsters; o++) {
                            if (o != m && g_monsters[o].alive && g_monsters[o].x == testX && g_monsters[o].y == testY) {
                                occ = TRUE; break;
                            }
                        }
                        if (!occ) {
                            mon->x = testX;
                            mon->y = testY;
                        }
                    }
                }
            }
        }
    }
}

// 1. Catacombs: Classic Rectangular Crypts & Stone Corridors
void GenerateCatacombs(int depth) {
    Room rooms[MAX_ROOMS];
    int roomCount = 0;
    int targetRooms = 8 + RandInt(0, 4);

    for (int r = 0; r < targetRooms * 4 && roomCount < targetRooms && roomCount < MAX_ROOMS; r++) {
        int rw = RandInt(5, 10);
        int rh = RandInt(4, 8);
        int rx = RandInt(2, MAP_WIDTH - rw - 3);
        int ry = RandInt(2, MAP_HEIGHT - rh - 3);

        BOOL overlap = FALSE;
        for (int i = 0; i < roomCount; i++) {
            if (rx <= rooms[i].x + rooms[i].w + 1 && rx + rw + 1 >= rooms[i].x &&
                ry <= rooms[i].y + rooms[i].h + 1 && ry + rh + 1 >= rooms[i].y) {
                overlap = TRUE;
                break;
            }
        }

        if (!overlap) {
            rooms[roomCount].x = rx;
            rooms[roomCount].y = ry;
            rooms[roomCount].w = rw;
            rooms[roomCount].h = rh;

            for (int y = ry; y < ry + rh; y++) {
                for (int x = rx; x < rx + rw; x++) {
                    g_dungeon[y][x] = TILE_FLOOR;
                }
            }

            if (rw >= 7 && rh >= 6) {
                g_dungeon[ry + 2][rx + 2] = TILE_PILLAR;
                g_dungeon[ry + 2][rx + rw - 3] = TILE_PILLAR;
                g_dungeon[ry + rh - 3][rx + 2] = TILE_PILLAR;
                g_dungeon[ry + rh - 3][rx + rw - 3] = TILE_PILLAR;
            }

            if (g_numTorches < MAX_TORCHES) {
                g_torches[g_numTorches].x = rx + rw / 2;
                g_torches[g_numTorches].y = ry;
                g_torches[g_numTorches].intensity = 4;
                g_torches[g_numTorches].lit = TRUE;
                g_numTorches++;
            }

            roomCount++;
        }
    }

    for (int i = 0; i < roomCount - 1; i++) {
        int cx1 = rooms[i].x + rooms[i].w / 2;
        int cy1 = rooms[i].y + rooms[i].h / 2;
        int cx2 = rooms[i + 1].x + rooms[i + 1].w / 2;
        int cy2 = rooms[i + 1].y + rooms[i + 1].h / 2;

        while (cx1 != cx2) {
            g_dungeon[cy1][cx1] = TILE_FLOOR;
            cx1 += (cx2 > cx1) ? 1 : -1;
        }
        while (cy1 != cy2) {
            g_dungeon[cy1][cx1] = TILE_FLOOR;
            cy1 += (cy2 > cy1) ? 1 : -1;
        }
    }

    for (int i = 0; i < roomCount; i++) {
        for (int x = rooms[i].x; x < rooms[i].x + rooms[i].w; x++) {
            if (g_dungeon[rooms[i].y - 1][x] == TILE_FLOOR && g_dungeon[rooms[i].y][x] == TILE_FLOOR) {
                if (RandInt(0, 100) < 60) g_dungeon[rooms[i].y][x] = TILE_DOOR_CLOSED;
            }
            if (g_dungeon[rooms[i].y + rooms[i].h][x] == TILE_FLOOR && g_dungeon[rooms[i].y + rooms[i].h - 1][x] == TILE_FLOOR) {
                if (RandInt(0, 100) < 60) g_dungeon[rooms[i].y + rooms[i].h - 1][x] = TILE_DOOR_CLOSED;
            }
        }
    }

    if (roomCount > 0) {
        g_player.x = rooms[0].x + rooms[0].w / 2;
        g_player.y = rooms[0].y + rooms[0].h / 2;
        g_dungeon[g_player.y][g_player.x] = TILE_STAIRS_UP;

        int endIdx = roomCount - 1;
        int ex = rooms[endIdx].x + rooms[endIdx].w / 2;
        int ey = rooms[endIdx].y + rooms[endIdx].h / 2;
        g_dungeon[ey][ex] = TILE_STAIRS_DOWN;
    }

    for (int i = 1; i < roomCount - 1; i++) {
        if (RandInt(0, 100) < 70 && g_numChests < MAX_CHESTS) {
            int cx = rooms[i].x + 1 + RandInt(0, rooms[i].w - 3);
            int cy = rooms[i].y + 1 + RandInt(0, rooms[i].h - 3);
            if (g_dungeon[cy][cx] == TILE_FLOOR) {
                g_dungeon[cy][cx] = TILE_CHEST;
                g_chests[g_numChests].x = cx;
                g_chests[g_numChests].y = cy;
                g_chests[g_numChests].opened = 0;
                g_chests[g_numChests].essence = 25 + RandInt(0, 40);
                g_numChests++;
            }
        }
        if (RandInt(0, 100) < 50) {
            int rx = rooms[i].x + 1 + RandInt(0, rooms[i].w - 3);
            int ry = rooms[i].y + 1 + RandInt(0, rooms[i].h - 3);
            if (g_dungeon[ry][rx] == TILE_FLOOR) {
                g_dungeon[ry][rx] = TILE_RUBBLE;
            }
        }
    }

    if (roomCount >= 3) {
        int cr = 1 + RandInt(0, roomCount - 3);
        int cx = rooms[cr].x + rooms[cr].w / 2;
        int cy = rooms[cr].y + rooms[cr].h / 2;
        if (g_dungeon[cy][cx] == TILE_FLOOR) {
            g_dungeon[cy][cx] = TILE_CAULDRON;
        }
    }

    // Place Relic Enchanting Altar in Catacombs
    if (roomCount >= 3) {
        int ar = 1 + RandInt(0, roomCount - 3);
        int ax = rooms[ar].x + 2;
        int ay = rooms[ar].y + rooms[ar].h / 2;
        if (g_dungeon[ay][ax] == TILE_FLOOR) {
            g_dungeon[ay][ax] = TILE_ALTAR;
        }
    }

    // Place Ancient Runic Shrine in Catacombs
    if (roomCount >= 4) {
        int sr = 2 + RandInt(0, roomCount - 4);
        int sx = rooms[sr].x + rooms[sr].w - 2;
        int sy = rooms[sr].y + 2;
        if (g_dungeon[sy][sx] == TILE_FLOOR) {
            g_dungeon[sy][sx] = TILE_SHRINE;
        }
    }

    // Place Cursed Effigy in Catacombs
    if (roomCount >= 4) {
        int efRoom = 1 + RandInt(0, roomCount - 3);
        int ex = rooms[efRoom].x + 2;
        int ey = rooms[efRoom].y + 2;
        if (g_dungeon[ey][ex] == TILE_FLOOR) {
            g_dungeon[ey][ex] = TILE_EFFIGY;
        }
    }

    // Place Subterranean Merchant in Catacombs
    if (roomCount >= 3) {
        int mx = rooms[2].x + 2;
        int my = rooms[2].y + 2;
        if (g_dungeon[my][mx] == TILE_FLOOR) {
            g_dungeon[my][mx] = TILE_MERCHANT;
        }
    }
}

// 2. Sunken Grotto: Organic Caverns, Flooded Water Pools & Cyan Fungi
void GenerateSunkenGrotto(int depth) {
    typedef struct { int x, y, rad; } Cav;
    Cav caverns[12];
    int numCaverns = 0;
    int targetCaverns = 7 + RandInt(0, 3);

    for (int c = 0; c < targetCaverns * 4 && numCaverns < targetCaverns && numCaverns < 12; c++) {
        int cx = RandInt(5, MAP_WIDTH - 10);
        int cy = RandInt(5, MAP_HEIGHT - 10);
        int rad = RandInt(3, 5);

        BOOL overlap = FALSE;
        for (int i = 0; i < numCaverns; i++) {
            int dist = (int)sqrtf((float)((caverns[i].x - cx) * (caverns[i].x - cx) + (caverns[i].y - cy) * (caverns[i].y - cy)));
            if (dist < rad + caverns[i].rad + 2) {
                overlap = TRUE;
                break;
            }
        }

        if (!overlap) {
            caverns[numCaverns].x = cx;
            caverns[numCaverns].y = cy;
            caverns[numCaverns].rad = rad;

            for (int dy = -rad; dy <= rad; dy++) {
                for (int dx = -rad; dx <= rad; dx++) {
                    if (dx * dx + dy * dy <= rad * rad) {
                        int nx = cx + dx;
                        int ny = cy + dy;
                        if (nx > 1 && nx < MAP_WIDTH - 2 && ny > 1 && ny < MAP_HEIGHT - 2) {
                            g_dungeon[ny][nx] = TILE_FLOOR;
                        }
                    }
                }
            }

            // Central flooded pool
            for (int dy = -rad + 1; dy <= rad - 1; dy++) {
                for (int dx = -rad + 1; dx <= rad - 1; dx++) {
                    if (dx * dx + dy * dy <= (rad - 1) * (rad - 1) && RandInt(0, 100) < 55) {
                        int nx = cx + dx;
                        int ny = cy + dy;
                        if (nx > 1 && nx < MAP_WIDTH - 2 && ny > 1 && ny < MAP_HEIGHT - 2) {
                            g_dungeon[ny][nx] = TILE_WATER;
                        }
                    }
                }
            }

            // Stalagmite rock spire
            if (rad >= 4 && RandInt(0, 100) < 70) {
                int px = cx + (RandInt(0, 1) ? 2 : -2);
                int py = cy + (RandInt(0, 1) ? 2 : -2);
                if (g_dungeon[py][px] == TILE_FLOOR) g_dungeon[py][px] = TILE_PILLAR;
            }

            // Bioluminescent fungi
            if (g_numTorches < MAX_TORCHES) {
                g_torches[g_numTorches].x = cx;
                g_torches[g_numTorches].y = cy;
                g_torches[g_numTorches].intensity = 5;
                g_torches[g_numTorches].lit = TRUE;
                g_numTorches++;
            }

            numCaverns++;
        }
    }

    // Connect caverns with natural winding paths
    for (int i = 0; i < numCaverns - 1; i++) {
        int x = caverns[i].x;
        int y = caverns[i].y;
        int tx = caverns[i + 1].x;
        int ty = caverns[i + 1].y;

        while (x != tx || y != ty) {
            if (x > 1 && x < MAP_WIDTH - 2 && y > 1 && y < MAP_HEIGHT - 2) {
                if (g_dungeon[y][x] == TILE_WALL) g_dungeon[y][x] = TILE_FLOOR;
                if (RandInt(0, 100) < 30) {
                    if (g_dungeon[y + 1][x] == TILE_WALL) g_dungeon[y + 1][x] = TILE_FLOOR;
                    if (g_dungeon[y][x + 1] == TILE_WALL) g_dungeon[y][x + 1] = TILE_FLOOR;
                }
            }
            if (RandInt(0, 1) && x != tx) {
                x += (tx > x) ? 1 : -1;
            } else if (y != ty) {
                y += (ty > y) ? 1 : -1;
            } else {
                x += (tx > x) ? 1 : -1;
            }
        }
    }

    if (numCaverns > 0) {
        g_player.x = caverns[0].x;
        g_player.y = caverns[0].y;
        g_dungeon[g_player.y][g_player.x] = TILE_STAIRS_UP;

        int endIdx = numCaverns - 1;
        g_dungeon[caverns[endIdx].y][caverns[endIdx].x] = TILE_STAIRS_DOWN;
    }

    for (int i = 1; i < numCaverns - 1; i++) {
        if (RandInt(0, 100) < 75 && g_numChests < MAX_CHESTS) {
            int cx = caverns[i].x + RandInt(-1, 1);
            int cy = caverns[i].y + RandInt(-1, 1);
            if (g_dungeon[cy][cx] == TILE_FLOOR || g_dungeon[cy][cx] == TILE_WATER) {
                g_dungeon[cy][cx] = TILE_CHEST;
                g_chests[g_numChests].x = cx;
                g_chests[g_numChests].y = cy;
                g_chests[g_numChests].opened = 0;
                g_chests[g_numChests].essence = 50 + RandInt(0, 60);
                g_numChests++;
            }
        }
    }

    if (numCaverns >= 3) {
        int cr = 1 + RandInt(0, numCaverns - 3);
        int cx = caverns[cr].x;
        int cy = caverns[cr].y;
        if (g_dungeon[cy][cx] == TILE_FLOOR || g_dungeon[cy][cx] == TILE_WATER) {
            g_dungeon[cy][cx] = TILE_CAULDRON;
        }
    }

    // Place Relic Enchanting Altar in Sunken Grotto
    if (numCaverns >= 3) {
        int ar = 1 + RandInt(0, numCaverns - 3);
        int ax = caverns[ar].x + 1;
        int ay = caverns[ar].y;
        if (g_dungeon[ay][ax] == TILE_FLOOR || g_dungeon[ay][ax] == TILE_WATER) {
            g_dungeon[ay][ax] = TILE_ALTAR;
        }
    }

    // Place Ancient Runic Shrine in Sunken Grotto
    if (numCaverns >= 4) {
        int sr = 2 + RandInt(0, numCaverns - 4);
        int sx = caverns[sr].x;
        int sy = caverns[sr].y + 1;
        if (g_dungeon[sy][sx] == TILE_FLOOR || g_dungeon[sy][sx] == TILE_WATER) {
            g_dungeon[sy][sx] = TILE_SHRINE;
        }
    }

    // Place Cursed Effigy in Sunken Grotto
    if (numCaverns >= 4) {
        int efCav = 1 + RandInt(0, numCaverns - 3);
        int ex = caverns[efCav].x + 1;
        int ey = caverns[efCav].y + 1;
        if (g_dungeon[ey][ex] == TILE_FLOOR || g_dungeon[ey][ex] == TILE_WATER) {
            g_dungeon[ey][ex] = TILE_EFFIGY;
        }
    }

    // Place Subterranean Merchant in Sunken Grotto
    if (numCaverns >= 3) {
        int mx = caverns[2].x;
        int my = caverns[2].y;
        if (g_dungeon[my][mx] == TILE_FLOOR || g_dungeon[my][mx] == TILE_WATER) {
            g_dungeon[my][mx] = TILE_MERCHANT;
        }
    }
}


// 3. Forgotten Crypt: Dense Necrotic Vaults & Runic Altars
void GenerateForgottenCrypt(int depth) {
    Room vaults[MAX_ROOMS];
    int vaultCount = 0;
    int targetVaults = 10 + RandInt(0, 4);

    for (int v = 0; v < targetVaults * 4 && vaultCount < targetVaults && vaultCount < MAX_ROOMS; v++) {
        int vw = RandInt(4, 7);
        int vh = RandInt(4, 7);
        int vx = RandInt(2, MAP_WIDTH - vw - 3);
        int vy = RandInt(2, MAP_HEIGHT - vh - 3);

        BOOL overlap = FALSE;
        for (int i = 0; i < vaultCount; i++) {
            if (vx <= vaults[i].x + vaults[i].w + 1 && vx + vw + 1 >= vaults[i].x &&
                vy <= vaults[i].y + vaults[i].h + 1 && vy + vh + 1 >= vaults[i].y) {
                overlap = TRUE;
                break;
            }
        }

        if (!overlap) {
            vaults[vaultCount].x = vx;
            vaults[vaultCount].y = vy;
            vaults[vaultCount].w = vw;
            vaults[vaultCount].h = vh;

            for (int y = vy; y < vy + vh; y++) {
                for (int x = vx; x < vx + vw; x++) {
                    g_dungeon[y][x] = TILE_FLOOR;
                }
            }

            if (RandInt(0, 100) < 60) {
                g_dungeon[vy + 1][vx + 1] = TILE_RUBBLE;
            }

            if (g_numTorches < MAX_TORCHES) {
                g_torches[g_numTorches].x = vx + vw / 2;
                g_torches[g_numTorches].y = vy;
                g_torches[g_numTorches].intensity = 4;
                g_torches[g_numTorches].lit = TRUE;
                g_numTorches++;
            }

            vaultCount++;
        }
    }

    for (int i = 0; i < vaultCount - 1; i++) {
        int cx1 = vaults[i].x + vaults[i].w / 2;
        int cy1 = vaults[i].y + vaults[i].h / 2;
        int cx2 = vaults[i + 1].x + vaults[i + 1].w / 2;
        int cy2 = vaults[i + 1].y + vaults[i + 1].h / 2;

        while (cx1 != cx2) {
            g_dungeon[cy1][cx1] = TILE_FLOOR;
            cx1 += (cx2 > cx1) ? 1 : -1;
        }
        while (cy1 != cy2) {
            g_dungeon[cy1][cx1] = TILE_FLOOR;
            cy1 += (cy2 > cy1) ? 1 : -1;
        }
    }

    for (int i = 0; i < vaultCount; i++) {
        for (int x = vaults[i].x; x < vaults[i].x + vaults[i].w; x++) {
            if (g_dungeon[vaults[i].y - 1][x] == TILE_FLOOR && g_dungeon[vaults[i].y][x] == TILE_FLOOR) {
                if (RandInt(0, 100) < 80) g_dungeon[vaults[i].y][x] = TILE_DOOR_CLOSED;
            }
            if (g_dungeon[vaults[i].y + vaults[i].h][x] == TILE_FLOOR && g_dungeon[vaults[i].y + vaults[i].h - 1][x] == TILE_FLOOR) {
                if (RandInt(0, 100) < 80) g_dungeon[vaults[i].y + vaults[i].h - 1][x] = TILE_DOOR_CLOSED;
            }
        }
    }

    // Place 1-2 Runic Altars in vaults
    if (vaultCount > 3) {
        int aIdx1 = vaultCount / 3;
        g_dungeon[vaults[aIdx1].y + vaults[aIdx1].h / 2][vaults[aIdx1].x + vaults[aIdx1].w / 2] = TILE_ALTAR;
        if (vaultCount > 6) {
            int aIdx2 = vaultCount * 2 / 3;
            g_dungeon[vaults[aIdx2].y + vaults[aIdx2].h / 2][vaults[aIdx2].x + vaults[aIdx2].w / 2] = TILE_ALTAR;
        }
    }

    if (vaultCount > 0) {
        g_player.x = vaults[0].x + vaults[0].w / 2;
        g_player.y = vaults[0].y + vaults[0].h / 2;
        g_dungeon[g_player.y][g_player.x] = TILE_STAIRS_UP;

        int endIdx = vaultCount - 1;
        int ex = vaults[endIdx].x + vaults[endIdx].w / 2;
        int ey = vaults[endIdx].y + vaults[endIdx].h / 2;
        g_dungeon[ey][ex] = TILE_STAIRS_DOWN;
    }

    for (int i = 1; i < vaultCount - 1; i++) {
        if (RandInt(0, 100) < 70 && g_numChests < MAX_CHESTS) {
            int cx = vaults[i].x + 1 + RandInt(0, vaults[i].w - 3);
            int cy = vaults[i].y + 1 + RandInt(0, vaults[i].h - 3);
            if (g_dungeon[cy][cx] == TILE_FLOOR) {
                g_dungeon[cy][cx] = TILE_CHEST;
                g_chests[g_numChests].x = cx;
                g_chests[g_numChests].y = cy;
                g_chests[g_numChests].opened = 0;
                g_chests[g_numChests].essence = 90 + RandInt(0, 80);
                g_numChests++;
            }
        }
    }

    if (vaultCount >= 4) {
        int cIdx = vaultCount / 2;
        int cx = vaults[cIdx].x + vaults[cIdx].w / 2;
        int cy = vaults[cIdx].y + vaults[cIdx].h / 2;
        if (g_dungeon[cy][cx] == TILE_FLOOR) {
            g_dungeon[cy][cx] = TILE_CAULDRON;
        }
    }

    // Place Ancient Runic Shrine in Crypt
    if (vaultCount >= 4) {
        int sIdx = vaultCount / 2;
        int sx = vaults[sIdx].x + 1;
        int sy = vaults[sIdx].y + 1;
        if (g_dungeon[sy][sx] == TILE_FLOOR) {
            g_dungeon[sy][sx] = TILE_SHRINE;
        }
    }

    // Place 2 Cursed Effigies in Crypt
    if (vaultCount >= 4) {
        for (int e = 0; e < 2; e++) {
            int ev = 1 + (e * (vaultCount / 3)) % (vaultCount - 1);
            int ex = vaults[ev].x + vaults[ev].w - 2;
            int ey = vaults[ev].y + vaults[ev].h - 2;
            if (g_dungeon[ey][ex] == TILE_FLOOR) {
                g_dungeon[ey][ex] = TILE_EFFIGY;
            }
        }
    }

    // Place Wandering Black Market Hermit in Crypt
    if (vaultCount >= 4) {
        int mx = vaults[2].x + 2;
        int my = vaults[2].y + 2;
        if (g_dungeon[my][mx] == TILE_FLOOR) {
            g_dungeon[my][mx] = TILE_MERCHANT;
        }
    }
}


// 4. The Void Abyss: Obsidian Platforms Over Cosmic Chasms
void GenerateVoidAbyss(int depth) {
    Room plats[MAX_ROOMS];
    int platCount = 0;
    int targetPlats = 8 + RandInt(0, 3);

    for (int p = 0; p < targetPlats * 4 && platCount < targetPlats && platCount < MAX_ROOMS; p++) {
        int pw = RandInt(5, 8);
        int ph = RandInt(5, 8);
        int px = RandInt(3, MAP_WIDTH - pw - 5);
        int py = RandInt(3, MAP_HEIGHT - ph - 5);

        BOOL overlap = FALSE;
        for (int i = 0; i < platCount; i++) {
            if (px <= plats[i].x + plats[i].w + 2 && px + pw + 2 >= plats[i].x &&
                py <= plats[i].y + plats[i].h + 2 && py + ph + 2 >= plats[i].y) {
                overlap = TRUE;
                break;
            }
        }

        if (!overlap) {
            plats[platCount].x = px;
            plats[platCount].y = py;
            plats[platCount].w = pw;
            plats[platCount].h = ph;

            for (int y = py; y < py + ph; y++) {
                for (int x = px; x < px + pw; x++) {
                    g_dungeon[y][x] = TILE_FLOOR;
                }
            }

            // Astral Void Pylons
            if (RandInt(0, 100) < 80) {
                g_dungeon[py + 1][px + 1] = TILE_PILLAR;
                g_dungeon[py + ph - 2][px + pw - 2] = TILE_PILLAR;
            }

            // Void Rift Crystal
            if (g_numTorches < MAX_TORCHES) {
                g_torches[g_numTorches].x = px + pw / 2;
                g_torches[g_numTorches].y = py + ph / 2;
                g_torches[g_numTorches].intensity = 6;
                g_torches[g_numTorches].lit = TRUE;
                g_numTorches++;
            }

            platCount++;
        }
    }

    // Narrow Void Bridges spanning the chasms
    for (int i = 0; i < platCount - 1; i++) {
        int cx1 = plats[i].x + plats[i].w / 2;
        int cy1 = plats[i].y + plats[i].h / 2;
        int cx2 = plats[i + 1].x + plats[i + 1].w / 2;
        int cy2 = plats[i + 1].y + plats[i + 1].h / 2;

        while (cx1 != cx2) {
            g_dungeon[cy1][cx1] = TILE_FLOOR;
            cx1 += (cx2 > cx1) ? 1 : -1;
        }
        while (cy1 != cy2) {
            g_dungeon[cy1][cx1] = TILE_FLOOR;
            cy1 += (cy2 > cy1) ? 1 : -1;
        }
    }

    if (platCount > 0) {
        g_player.x = plats[0].x + 2;
        g_player.y = plats[0].y + 2;
        g_dungeon[g_player.y][g_player.x] = TILE_STAIRS_UP;

        int endIdx = platCount - 1;
        int ex = plats[endIdx].x + plats[endIdx].w / 2;
        int ey = plats[endIdx].y + plats[endIdx].h / 2;
        g_dungeon[ey][ex] = TILE_STAIRS_DOWN;
    }

    for (int i = 1; i < platCount - 1; i++) {
        if (RandInt(0, 100) < 80 && g_numChests < MAX_CHESTS) {
            int cx = plats[i].x + 2 + RandInt(0, plats[i].w - 4);
            int cy = plats[i].y + 2 + RandInt(0, plats[i].h - 4);
            if (g_dungeon[cy][cx] == TILE_FLOOR) {
                g_dungeon[cy][cx] = TILE_CHEST;
                g_chests[g_numChests].x = cx;
                g_chests[g_numChests].y = cy;
                g_chests[g_numChests].opened = 0;
                g_chests[g_numChests].essence = 160 + RandInt(0, 120);
                g_numChests++;
            }
        }
    }

    if (platCount >= 3) {
        int pIdx = platCount / 2;
        int cx = plats[pIdx].x + plats[pIdx].w / 2;
        int cy = plats[pIdx].y + plats[pIdx].h / 2;
        if (g_dungeon[cy][cx] == TILE_FLOOR) {
            g_dungeon[cy][cx] = TILE_CAULDRON;
        }
    }

    // Place Relic Enchanting Altar in Void Abyss
    if (platCount >= 3) {
        int aIdx = 1;
        int ax = plats[aIdx].x + 2;
        int ay = plats[aIdx].y + 2;
        if (g_dungeon[ay][ax] == TILE_FLOOR) {
            g_dungeon[ay][ax] = TILE_ALTAR;
        }
    }

    // Place Ancient Runic Shrine in Void Abyss
    if (platCount >= 4) {
        int sIdx = platCount - 2;
        int sx = plats[sIdx].x + 2;
        int sy = plats[sIdx].y + 2;
        if (g_dungeon[sy][sx] == TILE_FLOOR) {
            g_dungeon[sy][sx] = TILE_SHRINE;
        }
    }

    // Place 2-3 Cursed Effigies in Void Abyss
    if (platCount >= 4) {
        for (int e = 0; e < 2; e++) {
            int ep = 1 + (e * 2) % (platCount - 1);
            int ex = plats[ep].x + 1;
            int ey = plats[ep].y + 1;
            if (g_dungeon[ey][ex] == TILE_FLOOR) {
                g_dungeon[ey][ex] = TILE_EFFIGY;
            }
        }
    }

    // Place Black Market Hermit on Void Abyss platform
    if (platCount >= 4) {
        int mx = plats[2].x + plats[2].w / 2;
        int my = plats[2].y + plats[2].h / 2;
        if (g_dungeon[my][mx] == TILE_FLOOR) {
            g_dungeon[my][mx] = TILE_MERCHANT;
        }
    }
}


void InitGame(int depth) {
    g_depthLevel = depth;
    g_turn = 1;
    g_numTorches = 0;
    g_numChests = 0;
    g_numEmbers = 0;

    DepthZone z = GetDepthZone(depth);
    const ZoneTheme* zt = &g_zoneThemes[z];
    int initTile = (z == ZONE_VOID_ABYSS) ? TILE_CHASM : TILE_WALL;

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (x == 0 || x == MAP_WIDTH - 1 || y == 0 || y == MAP_HEIGHT - 1) {
                g_dungeon[y][x] = TILE_WALL;
            } else {
                g_dungeon[y][x] = initTile;
            }
            g_explored[y][x] = FALSE;
            g_visible[y][x] = FALSE;
            g_lightMap[y][x] = 0.0f;
        }
    }

    if (z == ZONE_CATACOMBS) {
        GenerateCatacombs(depth);
    } else if (z == ZONE_SUNKEN_GROTTO) {
        GenerateSunkenGrotto(depth);
    } else if (z == ZONE_FORGOTTEN_CRYPT) {
        GenerateForgottenCrypt(depth);
    } else {
        GenerateVoidAbyss(depth);
    }

    g_numCombatTexts = 0;
    SpawnMonsters(depth);
    ComputeFOV();

    char buf[128];
    snprintf(buf, sizeof(buf), zt->enterMsg, depth);
    AddLog(buf, COLOR_ACCENT_AMBER);
}

void OpenEnchantAltar(void) {
    g_showEnchantModal = TRUE;
    AddLog("Approached the Ancient Relic Enchanting Altar! Imbue weapon with elements [1..3] or receive Benediction [4].", COLOR_TEXT_GOLD);
    Beep(659, 50); Beep(880, 80);
}

void CloseEnchantAltar(void) {
    g_showEnchantModal = FALSE;
}

void ImbueEnchantment(int enchantType) {
    if (g_player.equipWeapon == ITEM_NONE) {
        AddLog("You must equip a weapon first before imbuing an elemental enchantment!", COLOR_ACCENT_RED);
        Beep(200, 60);
        return;
    }

    if (enchantType == ENCHANT_NONE) {
        if (g_player.weaponEnchant == ENCHANT_NONE) {
            AddLog("Your weapon has no enchantment to dispel.", COLOR_TEXT_DIM);
            return;
        }
        g_player.weaponEnchant = ENCHANT_NONE;
        RecalcPlayerStats();
        AddLog("Dispelled active runes from weapon. Enchantment cleansed.", COLOR_ACCENT_CYAN);
        Beep(440, 50);
        return;
    }

    if (enchantType == ENCHANT_FIRE) {
        BOOL hasMat = (GetPackItemCount(ITEM_ING_BRIMSTONE) > 0);
        BOOL hasEss = (g_player.essence >= 25);
        if (!hasMat && !hasEss) {
            AddLog("Cannot imbue Flamebrand! Requires 25 Essence or 1 Brimstone Ash.", COLOR_ACCENT_RED);
            Beep(200, 60);
            return;
        }
        if (hasMat) {
            RemovePackItem(ITEM_ING_BRIMSTONE, 1);
        } else {
            g_player.essence -= 25;
        }
        g_player.weaponEnchant = ENCHANT_FIRE;
        RecalcPlayerStats();
        AddLog("FLAMEBRAND IMBUED! +10..16 Fire DMG (1.5x vs Undead), +2 Might passive.", RGB(249, 115, 22));
        Beep(523, 60); Beep(659, 80); Beep(784, 100);
    } else if (enchantType == ENCHANT_FROST) {
        BOOL hasMat = (GetPackItemCount(ITEM_ING_AZURE_SPORES) > 0);
        BOOL hasEss = (g_player.essence >= 25);
        if (!hasMat && !hasEss) {
            AddLog("Cannot imbue Frostbite! Requires 25 Essence or 1 Azure Spores.", COLOR_ACCENT_RED);
            Beep(200, 60);
            return;
        }
        if (hasMat) {
            RemovePackItem(ITEM_ING_AZURE_SPORES, 1);
        } else {
            g_player.essence -= 25;
        }
        g_player.weaponEnchant = ENCHANT_FROST;
        RecalcPlayerStats();
        AddLog("FROSTBITE IMBUED! +8..14 Cryo DMG, 35% Freeze (2 turns), +3 Warding passive.", RGB(6, 182, 212));
        Beep(587, 60); Beep(740, 80); Beep(880, 100);
    } else if (enchantType == ENCHANT_VOID) {
        BOOL hasMat = (GetPackItemCount(ITEM_ING_VOID_DUST) > 0);
        BOOL hasEss = (g_player.essence >= 30);
        if (!hasMat && !hasEss) {
            AddLog("Cannot imbue Voidsever! Requires 30 Essence or 1 Void Dust.", COLOR_ACCENT_RED);
            Beep(200, 60);
            return;
        }
        if (hasMat) {
            RemovePackItem(ITEM_ING_VOID_DUST, 1);
        } else {
            g_player.essence -= 30;
        }
        g_player.weaponEnchant = ENCHANT_VOID;
        RecalcPlayerStats();
        AddLog("VOIDSEVER IMBUED! +12..20 Void DMG (pierces def), siphons +4 MP/+3 Sanity, +2 Arcana.", RGB(168, 85, 247));
        Beep(440, 60); Beep(659, 80); Beep(988, 120);
    }
}

void CommuneAltarBenediction(void) {
    g_player.sanity += 25;
    if (g_player.sanity > g_player.max_sanity) g_player.sanity = g_player.max_sanity;
    g_player.hp += 30;
    if (g_player.hp > g_player.max_hp) g_player.hp = g_player.max_hp;
    g_player.aether += 20;
    if (g_player.aether > g_player.max_aether) g_player.aether = g_player.max_aether;

    if (g_player.curse != CURSE_NONE) {
        g_player.curse = CURSE_NONE;
        g_player.curseTurns = 0;
        AddLog("Consecrated benediction purges all subterranean curses from your soul!", COLOR_BORDER_GLOW);
    }
    AddLog("ALTAR BENEDICTION! Consecrated light restores +30 HP, +25 Sanity, +20 Aether.", COLOR_TEXT_GOLD);
    Beep(523, 60); Beep(659, 60); Beep(784, 80);
}

void CommuneAltar(int x, int y) {
    (void)x; (void)y;
    OpenEnchantAltar();
}

void CommuneShrine(int x, int y) {
    g_dungeon[y][x] = TILE_RUBBLE;
    int roll = RandInt(0, 3);
    char buf[160];

    if (roll == 0) {
        g_player.sanity += 35;
        if (g_player.sanity > g_player.max_sanity) g_player.sanity = g_player.max_sanity;
        g_player.hp += 25;
        if (g_player.hp > g_player.max_hp) g_player.hp = g_player.max_hp;
        g_player.aether += 20;
        if (g_player.aether > g_player.max_aether) g_player.aether = g_player.max_aether;
        if (g_player.curse != CURSE_NONE) {
            g_player.curse = CURSE_NONE;
            g_player.curseTurns = 0;
        }
        snprintf(buf, sizeof(buf), "SHRINE OF PURIFYING RADIANCE! Divine luminescence cleanses all curses! (+35 San, +25 HP, +20 MP)");
        AddLog(buf, COLOR_BORDER_GLOW);
    } else if (roll == 1) {
        g_player.aether += 45;
        if (g_player.aether > g_player.max_aether) g_player.aether = g_player.max_aether;
        g_player.essence += 35;
        snprintf(buf, sizeof(buf), "SHRINE OF ELDRITCH AETHER! Primordial torrent infuses your soul! (+45 Aether, +35 Essence)");
        AddLog(buf, RGB(168, 85, 247));
    } else if (roll == 2) {
        g_player.shield += 40;
        g_player.might += 3;
        snprintf(buf, sizeof(buf), "SHRINE OF IRON AEGIS! Prismatic ward wraps your body! (+40 Ward Shield, +3 Might surge)");
        AddLog(buf, RGB(56, 189, 248));
    } else {
        int unowned[NUM_RUNES];
        int unownedCount = 0;
        for (int r = 0; r < NUM_RUNES; r++) {
            if (!g_player.ownedRunes[r]) unowned[unownedCount++] = r;
        }
        if (unownedCount > 0) {
            int pick = unowned[RandInt(0, unownedCount - 1)];
            g_player.ownedRunes[pick] = TRUE;
            snprintf(buf, sizeof(buf), "SHRINE OF PRIMORDIAL RUNES! Ancient glyphs manifest: %s (%s)! Inscribe in Tab [3].", g_runeDefs[pick].name, g_runeDefs[pick].symbol);
            AddLog(buf, COLOR_TEXT_GOLD);
        } else {
            g_player.exp += 60;
            g_player.essence += 40;
            snprintf(buf, sizeof(buf), "SHRINE OF PRIMORDIAL RUNES! Celestial resonance transcends mortality! (+60 EXP, +40 Essence)");
            AddLog(buf, COLOR_TEXT_GOLD);
        }
    }
    SpawnCombatText((float)x, (float)y, "SHRINE BLESSING!", COLOR_TEXT_GOLD);
    Beep(523, 60); Beep(659, 70); Beep(784, 80); Beep(1046, 120);
    CheckLevelUp();
    AdvanceTurn();
}

int GetItemSellValue(ItemId id) {
    if (id <= ITEM_NONE || id >= NUM_ITEM_DEFS) return 5;
    const ItemDef* def = &g_itemDefs[id];
    if (def->category == ITEM_TYPE_EQUIPMENT) {
        if (id == ITEM_WPN_VOID_DAGGER || id == ITEM_ARM_AEGIS_CUIRASS || id == ITEM_AMU_VOID) return 40;
        if (id == ITEM_REL_CENSER || id == ITEM_AMU_STAR) return 35;
        return 25;
    }
    if (def->category == ITEM_TYPE_REAGENT) return 8;
    if (id == ITEM_PANACEA_DEEP) return 22;
    if (id == ITEM_FOOD_RATIONS) return 7;
    if (id == ITEM_PURIFYING_SALT) return 10;
    if (def->category == ITEM_TYPE_CONSUMABLE) return 11;
    return 5;
}

void OpenMerchantShop(void) {
    g_showMerchantModal = TRUE;
    g_showEnchantModal = FALSE;
    g_showHelpModal = FALSE;
    g_merchantMode = 0;
    Beep(520, 60); Beep(660, 80);
    if (g_depthLevel <= 6) {
        AddLog("Subterranean Broker: 'Torches, salves, blade steel... gold shines the same even in the deep.'", COLOR_TEXT_GOLD);
    } else {
        AddLog("Blind Hermit: 'The void whispers truths the sighted can never see... trade your essence, delver.'", COLOR_TEXT_RUNE);
    }
}

void CloseMerchantShop(void) {
    g_showMerchantModal = FALSE;
    Beep(440, 40);
}

void BuyMerchantItem(int itemNum) {
    if (itemNum < 0 || itemNum >= 8) return;
    BOOL isDeep = (g_depthLevel > 6);
    char buf[128];

    if (!isDeep) {
        int prices[8] = { 15, 18, 22, 55, 60, 65, 35, 40 };
        int price = prices[itemNum];

        if (g_player.essence < price) {
            snprintf(buf, sizeof(buf), "Not enough Gold! Need %d Essence (You have %d).", price, g_player.essence);
            AddLog(buf, COLOR_ACCENT_RED);
            Beep(180, 50);
            return;
        }

        if (itemNum == 0) {
            if (AddPackItem(ITEM_FOOD_RATIONS, 1)) {
                g_player.essence -= price;
                AddLog("Purchased Iron Rations for 15 Gold! Added to Pack [2].", COLOR_TEXT_GOLD);
            } else { AddLog("Delver's Pack is full!", COLOR_ACCENT_RED); return; }
        } else if (itemNum == 1) {
            g_player.essence -= price;
            g_player.torchFuel = g_player.maxTorchFuel;
            g_player.torchLit = TRUE;
            RecalcPlayerStats();
            ComputeFOV();
            AddLog("Purchased Torch Pitch & Lamp Oil! Handheld torch fully refueled and rekindled!", COLOR_TEXT_GOLD);
            SpawnCombatText((float)g_player.x, (float)g_player.y, "TORCH LIT!", COLOR_TEXT_GOLD);
        } else if (itemNum == 2) {
            if (AddPackItem(ITEM_HEAL_SALVE, 1)) {
                g_player.essence -= price;
                AddLog("Purchased Healing Salve for 22 Gold! Added to Pack [2].", COLOR_TEXT_GOLD);
            } else { AddLog("Delver's Pack is full!", COLOR_ACCENT_RED); return; }
        } else if (itemNum == 3) {
            if (AddPackItem(ITEM_WPN_RUNIC_BLADE, 1)) {
                g_player.essence -= price;
                AddLog("Purchased Runic Longsword (+5 Might, +1 Ward) for 55 Gold!", COLOR_TEXT_GOLD);
            } else { AddLog("Delver's Pack is full!", COLOR_ACCENT_RED); return; }
        } else if (itemNum == 4) {
            if (AddPackItem(ITEM_ARM_SHADOW_CLOAK, 1)) {
                g_player.essence -= price;
                AddLog("Purchased Shadowweave Cloak (+4 Ward, +2 Arcana) for 60 Gold!", COLOR_TEXT_GOLD);
            } else { AddLog("Delver's Pack is full!", COLOR_ACCENT_RED); return; }
        } else if (itemNum == 5) {
            if (AddPackItem(ITEM_REL_LANTERN, 1)) {
                g_player.essence -= price;
                AddLog("Purchased Aether Lantern (+8 Light, +15 Max MP) for 65 Gold!", COLOR_TEXT_GOLD);
            } else { AddLog("Delver's Pack is full!", COLOR_ACCENT_RED); return; }
        } else if (itemNum == 6) {
            g_player.essence -= price;
            for (int y = 0; y < MAP_HEIGHT; y++) {
                for (int x = 0; x < MAP_WIDTH; x++) {
                    g_explored[y][x] = TRUE;
                }
            }
            AddLog("CARTOGRAPHY REVEALED! The broker unrolls a complete subterranean floor map!", COLOR_ACCENT_CYAN);
            SpawnCombatText((float)g_player.x, (float)g_player.y, "MAP REVEALED!", COLOR_ACCENT_CYAN);
        } else if (itemNum == 7) {
            g_player.essence -= price;
            g_player.curse = CURSE_NONE;
            g_player.curseTurns = 0;
            g_player.sanity += 35;
            if (g_player.sanity > g_player.max_sanity) g_player.sanity = g_player.max_sanity;
            g_player.hp += 30;
            if (g_player.hp > g_player.max_hp) g_player.hp = g_player.max_hp;
            g_player.aether += 25;
            if (g_player.aether > g_player.max_aether) g_player.aether = g_player.max_aether;
            AddLog("MALEDICTION PURGED! Sacred herbs and incense cleanse your body and soul (+35 Sanity, +30 HP, +25 MP)!", COLOR_ACCENT_GREEN);
            SpawnCombatText((float)g_player.x, (float)g_player.y, "CLEANSED!", COLOR_ACCENT_GREEN);
        }
    } else {
        int prices[8] = { 45, 75, 85, 80, 90, 35, 75, 50 };
        int price = prices[itemNum];

        if (g_player.essence < price) {
            snprintf(buf, sizeof(buf), "Not enough Gold! Need %d Essence (You have %d).", price, g_player.essence);
            AddLog(buf, COLOR_ACCENT_RED);
            Beep(180, 50);
            return;
        }

        if (itemNum == 0) {
            if (AddPackItem(ITEM_PANACEA_DEEP, 1)) {
                g_player.essence -= price;
                AddLog("Purchased Panacea of the Deep for 45 Gold! Added to Pack [2].", COLOR_TEXT_GOLD);
            } else { AddLog("Delver's Pack is full!", COLOR_ACCENT_RED); return; }
        } else if (itemNum == 1) {
            if (AddPackItem(ITEM_WPN_VOID_DAGGER, 1)) {
                g_player.essence -= price;
                AddLog("Purchased Voidfang Dagger (+7 Might, +3 Arcana) for 75 Gold!", COLOR_TEXT_GOLD);
            } else { AddLog("Delver's Pack is full!", COLOR_ACCENT_RED); return; }
        } else if (itemNum == 2) {
            if (AddPackItem(ITEM_ARM_AEGIS_CUIRASS, 1)) {
                g_player.essence -= price;
                AddLog("Purchased Aegis Cuirass (+7 Ward, +25 Max HP) for 85 Gold!", COLOR_TEXT_GOLD);
            } else { AddLog("Delver's Pack is full!", COLOR_ACCENT_RED); return; }
        } else if (itemNum == 3) {
            if (AddPackItem(ITEM_REL_CENSER, 1)) {
                g_player.essence -= price;
                AddLog("Purchased Radiant Censer (+9 Light, +20 Max Sanity) for 80 Gold!", COLOR_TEXT_GOLD);
            } else { AddLog("Delver's Pack is full!", COLOR_ACCENT_RED); return; }
        } else if (itemNum == 4) {
            if (AddPackItem(ITEM_AMU_VOID, 1)) {
                g_player.essence -= price;
                AddLog("Purchased Void Eye Talisman (+4 Arcana, +20 Max Sanity) for 90 Gold!", COLOR_TEXT_GOLD);
            } else { AddLog("Delver's Pack is full!", COLOR_ACCENT_RED); return; }
        } else if (itemNum == 5) {
            g_player.essence -= price;
            for (int y = 0; y < MAP_HEIGHT; y++) {
                for (int x = 0; x < MAP_WIDTH; x++) {
                    g_explored[y][x] = TRUE;
                }
            }
            AddLog("CARTOGRAPHY REVEALED! The hermit unravels the celestial abyss map!", COLOR_ACCENT_CYAN);
            SpawnCombatText((float)g_player.x, (float)g_player.y, "MAP REVEALED!", COLOR_ACCENT_CYAN);
        } else if (itemNum == 6) {
            int unowned[NUM_RUNES];
            int unownedCount = 0;
            for (int r = 0; r < NUM_RUNES; r++) {
                if (!g_player.ownedRunes[r]) unowned[unownedCount++] = r;
            }
            g_player.essence -= price;
            if (unownedCount > 0) {
                int pick = unowned[RandInt(0, unownedCount - 1)];
                g_player.ownedRunes[pick] = TRUE;
                snprintf(buf, sizeof(buf), "RUNE SIPHON! Hermit channels ancient glyphs: %s (%s) unlocked! Inscribe in Tab [3].", g_runeDefs[pick].name, g_runeDefs[pick].symbol);
                AddLog(buf, COLOR_TEXT_RUNE);
                SpawnCombatText((float)g_player.x, (float)g_player.y, "RUNE UNLOCKED!", COLOR_TEXT_RUNE);
            } else {
                g_player.exp += 75;
                g_player.base_max_aether += 5;
                RecalcPlayerStats();
                g_player.aether = g_player.max_aether;
                AddLog("RUNE SIPHON: All runes mastered! Hermit expands soul capacity (+75 EXP, +5 Max Aether)!", COLOR_TEXT_GOLD);
                SpawnCombatText((float)g_player.x, (float)g_player.y, "+5 MAX AETHER!", COLOR_TEXT_GOLD);
            }
        } else if (itemNum == 7) {
            ItemId mysteryPool[] = { ITEM_PANACEA_DEEP, ITEM_ARM_AEGIS_CUIRASS, ITEM_WPN_VOID_DAGGER, ITEM_REL_CENSER, ITEM_AMU_VOID, ITEM_STONESKIN_BREW, ITEM_LIQUID_FIRE, ITEM_ELIXIR_VITALITY };
            ItemId pick = mysteryPool[RandInt(0, 7)];
            if (AddPackItem(pick, 1)) {
                g_player.essence -= price;
                snprintf(buf, sizeof(buf), "VOID MYSTERY RELIC! Hermit draws %s from the dark!", g_itemDefs[pick].name);
                AddLog(buf, COLOR_TEXT_GOLD);
                SpawnCombatText((float)g_player.x, (float)g_player.y, "MYSTERY RELIC!", COLOR_TEXT_GOLD);
            } else { AddLog("Delver's Pack is full!", COLOR_ACCENT_RED); return; }
        }
    }

    Beep(650, 40); Beep(850, 60);
    CheckLevelUp();
}

void SellPackItemToMerchant(int packIdx) {
    if (packIdx < 0 || packIdx >= g_player.numPackItems) return;
    InventorySlot* slot = &g_player.pack[packIdx];
    if (slot->id <= ITEM_NONE || slot->id >= NUM_ITEM_DEFS) return;

    ItemId soldId = slot->id;
    int sellVal = GetItemSellValue(soldId);
    g_player.essence += sellVal;
    RemovePackItem(soldId, 1);

    char buf[128];
    snprintf(buf, sizeof(buf), "SOLD: Pawned 1x %s to the merchant for +%d Gold/Essence!", g_itemDefs[soldId].name, sellVal);
    AddLog(buf, COLOR_TEXT_GOLD);

    snprintf(buf, sizeof(buf), "+%d Gold", sellVal);
    SpawnCombatText((float)g_player.x, (float)g_player.y, buf, COLOR_TEXT_GOLD);

    Beep(580, 40); Beep(740, 50);
}

static const char* g_eldritchWhispers[8] = {

    "The stone breathes... can you feel its cold subterranean pulse?",
    "Your shadow detached itself three paces ago and creeps behind you...",
    "The flame flickers... the void reaches out hungry tendrils to claim you.",
    "A chorus of forgotten delvers whispers through the crags: 'Join us...'",
    "Something with too many eyes crawls silently along the ceiling...",
    "Your thoughts fray like rotten parchment in the suffocating deep.",
    "The abyss remembers your true name... and whispers it in your ear.",
    "The effigy smiles... its stone lips parted in soundless mockery."
};

void AdvanceTurn(void) {
    g_turn++;

    // 1. Hunger processing
    int hungerRate = (g_player.curse == CURSE_DECAY) ? 3 : 5;
    if (g_turn % hungerRate == 0 && g_player.hunger > 0) {
        g_player.hunger--;
        if (g_player.hunger == 35) {
            AddLog("Your stomach growls with hunger. Delver is getting peckish.", COLOR_ACCENT_AMBER);
        } else if (g_player.hunger == 15) {
            AddLog("Ravenous hunger sets in (-2 Might, -1 Ward)! Find iron rations.", COLOR_ACCENT_RED);
            Beep(220, 50);
        }
    }
    // Starvation damage
    if (g_player.hunger <= 0) {
        g_player.hunger = 0;
        if (g_turn % 3 == 0) {
            g_player.hp -= 2;
            if (g_player.hp < 1) g_player.hp = 1;
            AddLog("STARVATION! Your body wastes away from hunger (-2 HP)!", COLOR_ACCENT_RED);
            SpawnCombatText((float)g_player.x, (float)g_player.y, "-2 STARVE", COLOR_ACCENT_RED);
            Beep(180, 60);
        }
    }

    // 2. Torch fuel processing
    if (g_player.torchLit) {
        int fuelLoss = (g_player.curse == CURSE_DARKNESS) ? 2 : 1;
        g_player.torchFuel -= fuelLoss;
        if (g_player.torchFuel <= 0) {
            g_player.torchFuel = 0;
            g_player.torchLit = FALSE;
            AddLog("Your torch has burned out into cold cinder! Darkness closes in (Press T to Rekindle)!", COLOR_ACCENT_RED);
            Beep(200, 80); Beep(150, 100);
        } else if (g_player.torchFuel == 25) {
            AddLog("Your torch is sputtering low on oil and pitch! Light is waning...", COLOR_ACCENT_AMBER);
        }
    }

    // 3. Darkness sanity drain
    float currentLight = g_lightMap[g_player.y][g_player.x];
    if (!g_player.torchLit || currentLight < 0.2f) {
        if (g_turn % 4 == 0 && g_player.sanity > 0) {
            g_player.sanity--;
            if (g_turn % 8 == 0) {
                AddLog("The suffocating pitch-black darkness claws at your willpower (-1 Sanity).", COLOR_ACCENT_PURPLE);
            }
        }
    }

    // 4. Proximity to Cursed Effigies
    int effMinY = (g_player.y - 3 < 0) ? 0 : g_player.y - 3;
    int effMaxY = (g_player.y + 3 >= MAP_HEIGHT) ? MAP_HEIGHT - 1 : g_player.y + 3;
    int effMinX = (g_player.x - 3 < 0) ? 0 : g_player.x - 3;
    int effMaxX = (g_player.x + 3 >= MAP_WIDTH) ? MAP_WIDTH - 1 : g_player.x + 3;
    for (int y = effMinY; y <= effMaxY; y++) {
        for (int x = effMinX; x <= effMaxX; x++) {
            if (g_dungeon[y][x] == TILE_EFFIGY) {
                if (g_turn % 3 == 0 && g_player.sanity > 0) {
                    g_player.sanity--;
                    if (g_turn % 6 == 0) {
                        AddLog("The Cursed Effigy hums malevolently nearby... Sanity drained (-1).", COLOR_ACCENT_PURPLE);
                        SpawnCombatText((float)g_player.x, (float)g_player.y, "-1 SANITY", COLOR_ACCENT_PURPLE);
                    }
                }
            }
        }
    }

    // 5. Eldritch Whispers & Sanity Strain
    int whisperInterval = (g_player.curse == CURSE_VOID) ? 12 : 20;
    if ((g_player.sanity < 45 || g_player.curse == CURSE_VOID) && (g_turn % whisperInterval == 0)) {
        int wIdx = RandInt(0, 7);
        char wBuf[256];
        snprintf(wBuf, sizeof(wBuf), "ELDRITCH WHISPER: \"%s\" (-1 Sanity)", g_eldritchWhispers[wIdx]);
        AddLog(wBuf, COLOR_ACCENT_PURPLE);
        if (g_player.sanity > 0) g_player.sanity--;
        Beep(160, 40);
        SpawnCombatText((float)g_player.x, (float)g_player.y, "WHISPER", COLOR_ACCENT_PURPLE);
    }

    // 6. Deep Madness if Sanity is 0
    if (g_player.sanity <= 0) {
        g_player.sanity = 0;
        if (g_turn % 6 == 0) {
            g_player.hp -= 3;
            if (g_player.hp < 1) g_player.hp = 1;
            AddLog("PSYCHIC COLLAPSE! Total madness wracks your mind (-3 HP)!", COLOR_ACCENT_RED);
            SpawnCombatText((float)g_player.x, (float)g_player.y, "-3 MADNESS", COLOR_ACCENT_PURPLE);
            Beep(140, 70);
        }
    }

    // 7. Curse Turn Countdown
    if (g_player.curseTurns > 0) {
        g_player.curseTurns--;
        if (g_player.curseTurns <= 0) {
            g_player.curse = CURSE_NONE;
            AddLog("The subterranean curse has dissolved from your spirit!", COLOR_TEXT_GOLD);
            Beep(587, 60); Beep(880, 80);
        }
    }

    // 8. Wall Torches Snuffed Out by Subterranean Gusts
    if (g_turn % 50 == 0 && RandInt(0, 100) < 35 && g_numTorches > 0) {
        int cand[MAX_TORCHES];
        int candCount = 0;
        for (int t = 0; t < g_numTorches; t++) {
            if (g_torches[t].lit) {
                float dist = sqrtf((float)((g_torches[t].x - g_player.x) * (g_torches[t].x - g_player.x) + (g_torches[t].y - g_player.y) * (g_torches[t].y - g_player.y)));
                if (dist < 14.0f) cand[candCount++] = t;
            }
        }
        if (candCount > 0) {
            int snuffed = cand[RandInt(0, candCount - 1)];
            g_torches[snuffed].lit = FALSE;
            AddLog("An icy subterranean draft sweeps the crypt! A wall torch was snuffed out!", COLOR_ACCENT_AMBER);
            Beep(240, 50);
        }
    }

    RecalcPlayerStats();
    UpdateMonsters();
    ComputeFOV();
}

void MovePlayer(int dx, int dy) {
    int nx = g_player.x + dx;
    int ny = g_player.y + dy;

    if (dx > 0) g_player.facing = 1;
    if (dx < 0) g_player.facing = 3;
    if (dy > 0) g_player.facing = 2;
    if (dy < 0) g_player.facing = 0;

    if (nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT) return;

    // Check for monster bump-to-attack
    for (int m = 0; m < g_numMonsters; m++) {
        if (g_monsters[m].alive && g_monsters[m].x == nx && g_monsters[m].y == ny) {
            AttackMonster(m);
            return;
        }
    }

    int tile = g_dungeon[ny][nx];
    if (tile == TILE_WALL || tile == TILE_PILLAR || tile == TILE_CHASM) {
        if (tile == TILE_CHASM) {
            AddLog("A bottomless void chasm drops into infinity! You dare not step off.", COLOR_ACCENT_AMBER);
            Beep(140, 40);
        } else {
            AddLog("Stone wall blocks your path.", COLOR_TEXT_DIM);
            Beep(180, 30);
        }
        return;
    }

    if (tile == TILE_EFFIGY) {
        g_dungeon[ny][nx] = TILE_RUBBLE;
        g_player.essence += 50;
        g_player.exp += 40;
        if (RandInt(0, 100) < 45) {
            int c = 1 + RandInt(0, 3);
            g_player.curse = (CurseType)c;
            g_player.curseTurns = 35;
            const char* cNames[] = { "None", "Curse of Shadows", "Curse of Enfeeblement", "Curse of Decay", "Curse of the Void" };
            char cBuf[128];
            snprintf(cBuf, sizeof(cBuf), "MALEDICTED! Cursed Effigy shatters and unleashes %s (35 turns)!", cNames[c]);
            AddLog(cBuf, COLOR_ACCENT_PURPLE);
            SpawnCombatText((float)nx, (float)ny, "MALEDICTION!", COLOR_ACCENT_PURPLE);
            Beep(180, 80); Beep(130, 100);
        } else {
            AddLog("Smashed Cursed Effigy! The demonic bone idol crumbles into inert rubble (+50 Essence, +40 EXP).", COLOR_TEXT_GOLD);
            SpawnCombatText((float)nx, (float)ny, "SHATTERED!", COLOR_TEXT_GOLD);
            Beep(440, 50); Beep(660, 60);
        }
        CheckLevelUp();
        AdvanceTurn();
        return;
    }

    if (tile == TILE_DOOR_CLOSED) {
        g_dungeon[ny][nx] = TILE_DOOR_OPEN;
        AddLog("You push open the heavy subterranean door.", COLOR_ACCENT_CYAN);
        Beep(350, 40);
        AdvanceTurn();
        return;
    }

    if (tile == TILE_ALTAR) {
        OpenEnchantAltar();
        return;
    }

    if (tile == TILE_MERCHANT) {
        OpenMerchantShop();
        return;
    }

    if (tile == TILE_SHRINE) {
        CommuneShrine(nx, ny);
        return;
    }

    if (tile == TILE_CAULDRON) {
        AddLog("You approach the Ancient Alchemy Cauldron! Switched to Tab [2] Pack to brew potions.", RGB(52, 211, 153));
        g_activeTab = 1;
        Beep(650, 40); Beep(880, 60);
        return;
    }

    if (tile == TILE_CHEST) {
        for (int c = 0; c < g_numChests; c++) {
            if (g_chests[c].x == nx && g_chests[c].y == ny && !g_chests[c].opened) {
                g_chests[c].opened = 1;
                g_dungeon[ny][nx] = TILE_FLOOR;
                g_player.essence += g_chests[c].essence;
                g_player.exp += 25;

                int unowned[NUM_RUNES];
                int unownedCount = 0;
                for (int r = 0; r < NUM_RUNES; r++) {
                    if (!g_player.ownedRunes[r]) unowned[unownedCount++] = r;
                }

                char buf[128];
                if (unownedCount > 0 && RandInt(0, 100) < 50) {
                    int pick = unowned[RandInt(0, unownedCount - 1)];
                    g_player.ownedRunes[pick] = TRUE;
                    snprintf(buf, sizeof(buf), "Chest opened! +%d Essence and found %s (%s)!", g_chests[c].essence, g_runeDefs[pick].name, g_runeDefs[pick].symbol);
                } else if (g_player.equippedStaff == 0 && g_depthLevel >= 4 && RandInt(0, 100) < 40) {
                    g_player.equippedStaff = 1;
                    RecalcPlayerStats();
                    snprintf(buf, sizeof(buf), "Chest opened! +%d Essence & found Cinderwood Scepter (+4 Arcana)!", g_chests[c].essence);
                } else if (g_player.equippedStaff < 2 && g_depthLevel >= 7 && RandInt(0, 100) < 35) {
                    g_player.equippedStaff = 2;
                    RecalcPlayerStats();
                    snprintf(buf, sizeof(buf), "Chest opened! +%d Essence & found Staff of the Arch-Magi (3 Sockets)!", g_chests[c].essence);
                } else {
                    snprintf(buf, sizeof(buf), "Opened Relic Chest! +%d Essence & +25 EXP!", g_chests[c].essence);
                }

                AddLog(buf, COLOR_TEXT_GOLD);

                // Additional loot: reagents, potions, or gear
                int roll = RandInt(0, 100);
                if (roll < 35) {
                    ItemId ings[] = { ITEM_ING_BLOOD_LOTUS, ITEM_ING_AZURE_SPORES, ITEM_ING_BRIMSTONE, ITEM_ING_VOID_DUST, ITEM_ING_AETHER_BLOSSOM };
                    ItemId pickIng = ings[RandInt(0, 4)];
                    AddPackItem(pickIng, 1);
                    char lbuf[128];
                    snprintf(lbuf, sizeof(lbuf), "Looted %s from chest!", g_itemDefs[pickIng].name);
                    AddLog(lbuf, RGB(52, 211, 153));
                } else if (roll < 65) {
                    ItemId pots[] = { ITEM_HEAL_SALVE, ITEM_SANITY_INCENSE, ITEM_AETHER_PHIAL, ITEM_STONESKIN_BREW, ITEM_FOOD_RATIONS, ITEM_CRYPT_MUSHROOM, ITEM_PURIFYING_SALT };
                    ItemId pickPot = pots[RandInt(0, 6)];
                    AddPackItem(pickPot, 1);
                    char lbuf[128];
                    snprintf(lbuf, sizeof(lbuf), "Found %s in chest!", g_itemDefs[pickPot].name);
                    AddLog(lbuf, COLOR_ACCENT_GREEN);
                } else if (roll < 85) {
                    ItemId gears[] = { ITEM_WPN_RUNIC_BLADE, ITEM_WPN_VOID_DAGGER, ITEM_ARM_SHADOW_CLOAK, ITEM_ARM_AEGIS_CUIRASS, ITEM_REL_LANTERN, ITEM_REL_CENSER, ITEM_AMU_LIFE, ITEM_AMU_STAR, ITEM_AMU_VOID };
                    ItemId pickGear = gears[RandInt(0, 8)];
                    AddPackItem(pickGear, 1);
                    char lbuf[128];
                    snprintf(lbuf, sizeof(lbuf), "Discovered %s (+Gear) in chest!", g_itemDefs[pickGear].name);
                    AddLog(lbuf, COLOR_TEXT_RUNE);
                }

                Beep(600, 40); Beep(800, 50);
                CheckLevelUp();
                AdvanceTurn();
                return;
            }
        }
    }


    g_player.x = nx;
    g_player.y = ny;

    // Relight adjacent extinguished wall torches if player's torch is lit
    for (int t = 0; t < g_numTorches; t++) {
        if (!g_torches[t].lit && abs(g_torches[t].x - g_player.x) <= 1 && abs(g_torches[t].y - g_player.y) <= 1 && g_player.torchLit) {
            g_torches[t].lit = TRUE;
            AddLog("Your burning torch reignites the wall sconce! Warm light spreads.", COLOR_TEXT_GOLD);
            Beep(650, 40);
        }
    }

    if (tile == TILE_WATER) {
        AddLog("You wade through shallow flooded waters. (Splash)", COLOR_ACCENT_CYAN);
        Beep(200, 25);
    } else if (tile == TILE_STAIRS_DOWN) {
        AddLog("Spiraling descent deeper into Abyss. Press [E] to Descend.", COLOR_ACCENT_AMBER);
        Beep(240, 20);
    } else if (tile == TILE_STAIRS_UP) {
        AddLog("The sealed stone portal back to surface remains shut.", COLOR_TEXT_DIM);
        Beep(240, 20);
    } else {
        Beep(240, 20);
    }

    AdvanceTurn();
}

void RestTurn(void) {
    if (g_player.hunger <= 0) {
        AddLog("You are starving and cannot regenerate strength by resting! Eat food rations.", COLOR_ACCENT_RED);
        Beep(180, 50);
        AdvanceTurn();
        return;
    }
    if (g_player.hp < g_player.max_hp) {
        g_player.hp += 2;
        if (g_player.hp > g_player.max_hp) g_player.hp = g_player.max_hp;
    }
    if (g_player.sanity < g_player.max_sanity) {
        g_player.sanity += 1;
        if (g_player.sanity > g_player.max_sanity) g_player.sanity = g_player.max_sanity;
    }
    if (g_player.aether < g_player.max_aether) {
        g_player.aether += 3;
        if (g_player.aether > g_player.max_aether) g_player.aether = g_player.max_aether;
    }
    AddLog("You steady your breath and rest (+2 HP, +1 Sanity, +3 Aether).", COLOR_ACCENT_GREEN);
    Beep(440, 40);
    AdvanceTurn();
}

void RekindleTorch(void) {
    BOOL canRekindle = FALSE;
    BOOL usedBrimstone = FALSE;
    if (GetPackItemCount(ITEM_ING_BRIMSTONE) > 0) {
        canRekindle = TRUE;
        usedBrimstone = TRUE;
    } else if (g_player.ownedRunes[RUNE_PYRE] && g_player.aether >= 5) {
        canRekindle = TRUE;
        g_player.aether -= 5;
    } else {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int tx = g_player.x + dx;
                int ty = g_player.y + dy;
                if (tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT) {
                    if (g_dungeon[ty][tx] == TILE_ALTAR || g_dungeon[ty][tx] == TILE_SHRINE) canRekindle = TRUE;
                    for (int t = 0; t < g_numTorches; t++) {
                        if (g_torches[t].x == tx && g_torches[t].y == ty && g_torches[t].lit) {
                            canRekindle = TRUE;
                        }
                    }
                }
            }
        }
    }

    if (canRekindle) {
        if (usedBrimstone) RemovePackItem(ITEM_ING_BRIMSTONE, 1);
        g_player.torchFuel = g_player.maxTorchFuel;
        g_player.torchLit = TRUE;
        RecalcPlayerStats();
        ComputeFOV();
        AddLog("Rekindled torch! Warm golden light blazes across the crypts (160 turns).", COLOR_TEXT_GOLD);
        Beep(520, 50); Beep(780, 80);
    } else {
        AddLog("Cannot rekindle torch! Requires Brimstone Ash, Pyre spell (5 MP), or adjacent lit sconce.", COLOR_ACCENT_RED);
        Beep(180, 50);
    }
}

// --- Relic & Ancient Rune Magic Spellcasting System ---
void CastSpell(int socketIdx) {
    if (socketIdx < 0 || socketIdx >= g_staffDefs[g_player.equippedStaff].maxSockets) return;
    int runeIdx = g_player.staffSockets[socketIdx];
    if (runeIdx < 0 || runeIdx >= NUM_RUNES) {
        AddLog("Selected staff socket is empty! Inscribe a rune in Tab [3].", COLOR_ACCENT_AMBER);
        return;
    }

    const RuneDef* rd = &g_runeDefs[runeIdx];
    int cost = rd->cost;
    if (g_player.equippedStaff == 2) {
        cost -= 2;
        if (cost < 4) cost = 4;
    }

    if (g_player.aether < cost) {
        char buf[128];
        snprintf(buf, sizeof(buf), "Not enough Aether! Required: %d MP, Available: %d MP.", cost, g_player.aether);
        AddLog(buf, COLOR_ACCENT_RED);
        Beep(160, 40);
        return;
    }

    g_player.aether -= cost;

    int dx = 0, dy = 0;
    if (g_player.facing == 0) dy = -1;
    else if (g_player.facing == 1) dx = 1;
    else if (g_player.facing == 2) dy = 1;
    else if (g_player.facing == 3) dx = -1;

    if (runeIdx == RUNE_PYRE) {
        int curX = g_player.x;
        int curY = g_player.y;
        for (int step = 1; step <= 4; step++) {
            int tx = g_player.x + dx * step;
            int ty = g_player.y + dy * step;
            if (tx < 1 || tx >= MAP_WIDTH - 1 || ty < 1 || ty >= MAP_HEIGHT - 1) break;
            curX = tx; curY = ty;
            g_visible[ty][tx] = TRUE;
            g_explored[ty][tx] = TRUE;
            g_lightMap[ty][tx] = 1.0f;

            // Check monster in blast
            for (int m = 0; m < g_numMonsters; m++) {
                if (g_monsters[m].alive && g_monsters[m].x == tx && g_monsters[m].y == ty) {
                    int dmg = 35 + (int)(g_player.arcana * 0.8f);
                    if (g_monsters[m].type == MONSTER_SKELETON) dmg = (int)(dmg * 1.5f);
                    DamageMonster(m, dmg, "FIRE", FALSE);
                }
            }

            if (g_dungeon[ty][tx] == TILE_WALL || g_dungeon[ty][tx] == TILE_PILLAR || g_dungeon[ty][tx] == TILE_CHASM) {
                break;
            }
            if (g_dungeon[ty][tx] == TILE_DOOR_CLOSED) {
                g_dungeon[ty][tx] = TILE_DOOR_OPEN;
                AddLog("Pyre Blast burns down the door into charred embers!", COLOR_ACCENT_RED);
                break;
            }
            if (g_dungeon[ty][tx] == TILE_RUBBLE) {
                g_dungeon[ty][tx] = TILE_FLOOR;
                AddLog("Pyre Blast incinerates the rubble into ash!", COLOR_ACCENT_RED);
                break;
            }
        }
        if (g_numSpellFX < MAX_SPELL_FX) {
            g_spellFX[g_numSpellFX].type = 0;
            g_spellFX[g_numSpellFX].x = curX;
            g_spellFX[g_numSpellFX].y = curY;
            g_spellFX[g_numSpellFX].radius = 16;
            g_spellFX[g_numSpellFX].duration = 6;
            g_spellFX[g_numSpellFX].color = RGB(249, 115, 22);
            g_numSpellFX++;
        }
        for (int i = 0; i < 12; i++) {
            SpawnEmber((float)(curX * TILE_SIZE + 16), (float)(curY * TILE_SIZE + 16), TRUE);
        }
        AddLog("PYRE BLAST! You cast a roaring fireball 4 tiles ahead (35 Fire DMG)!", RGB(249, 115, 22));
        Beep(440, 40); Beep(220, 60);

    } else if (runeIdx == RUNE_FROST) {
        int frozenCount = 0;
        for (int fdy = -2; fdy <= 2; fdy++) {
            for (int fdx = -2; fdx <= 2; fdx++) {
                int tx = g_player.x + fdx;
                int ty = g_player.y + fdy;
                if (tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT) {
                    g_visible[ty][tx] = TRUE;
                    g_explored[ty][tx] = TRUE;
                    if (g_dungeon[ty][tx] == TILE_WATER) {
                        g_dungeon[ty][tx] = TILE_FLOOR;
                        frozenCount++;
                    }
                }
            }
        }
        // Hit all monsters in radius and freeze them
        for (int m = 0; m < g_numMonsters; m++) {
            if (!g_monsters[m].alive) continue;
            float dist = sqrtf((float)((g_monsters[m].x - g_player.x) * (g_monsters[m].x - g_player.x) + (g_monsters[m].y - g_player.y) * (g_monsters[m].y - g_player.y)));
            if (dist <= 2.3f) {
                int dmg = 24 + (int)(g_player.arcana * 0.5f);
                if (g_monsters[m].type == MONSTER_LEVIATHAN) dmg = (int)(dmg * 1.3f);
                g_monsters[m].freezeTurns = 2;
                DamageMonster(m, dmg, "CRYO", FALSE);
            }
        }
        if (g_numSpellFX < MAX_SPELL_FX) {
            g_spellFX[g_numSpellFX].type = 1;
            g_spellFX[g_numSpellFX].x = g_player.x;
            g_spellFX[g_numSpellFX].y = g_player.y;
            g_spellFX[g_numSpellFX].radius = 14;
            g_spellFX[g_numSpellFX].duration = 8;
            g_spellFX[g_numSpellFX].color = RGB(6, 182, 212);
            g_numSpellFX++;
        }
        if (frozenCount > 0) {
            char fbuf[128];
            snprintf(fbuf, sizeof(fbuf), "GLACIAL NOVA! Freezing wave froze %d water pools into solid ice sheets!", frozenCount);
            AddLog(fbuf, COLOR_ACCENT_CYAN);
        } else {
            AddLog("GLACIAL NOVA! Sub-zero frost radiates outward (24 Cryo DMG)!", COLOR_ACCENT_CYAN);
        }
        Beep(880, 50); Beep(1175, 70);

    } else if (runeIdx == RUNE_TEMPEST) {
        int endX = g_player.x;
        int endY = g_player.y;
        for (int step = 1; step <= 6; step++) {
            int tx = g_player.x + dx * step;
            int ty = g_player.y + dy * step;
            if (tx < 1 || tx >= MAP_WIDTH - 1 || ty < 1 || ty >= MAP_HEIGHT - 1) break;
            endX = tx; endY = ty;
            g_visible[ty][tx] = TRUE;
            g_explored[ty][tx] = TRUE;
            g_lightMap[ty][tx] = 1.0f;

            // Check monster along electric arc
            for (int m = 0; m < g_numMonsters; m++) {
                if (g_monsters[m].alive && g_monsters[m].x == tx && g_monsters[m].y == ty) {
                    int dmg = 42 + (int)(g_player.arcana * 0.9f);
                    if (g_monsters[m].type == MONSTER_GHOUL) dmg = (int)(dmg * 1.4f);
                    DamageMonster(m, dmg, "SHOCK", FALSE);
                }
            }

            if (g_dungeon[ty][tx] == TILE_DOOR_CLOSED) {
                g_dungeon[ty][tx] = TILE_DOOR_OPEN;
                AddLog("Chain Bolt shatters open the door!", COLOR_TEXT_GOLD);
            }
            if (g_dungeon[ty][tx] == TILE_WALL || g_dungeon[ty][tx] == TILE_PILLAR) break;
        }
        if (g_numSpellFX < MAX_SPELL_FX) {
            g_spellFX[g_numSpellFX].type = 2;
            g_spellFX[g_numSpellFX].x = g_player.x;
            g_spellFX[g_numSpellFX].y = g_player.y;
            g_spellFX[g_numSpellFX].x2 = endX;
            g_spellFX[g_numSpellFX].y2 = endY;
            g_spellFX[g_numSpellFX].duration = 6;
            g_spellFX[g_numSpellFX].color = RGB(234, 179, 8);
            g_numSpellFX++;
        }
        AddLog("CHAIN BOLT! Crackling electrical bolt arcs 6 tiles (42 Shock DMG)!", RGB(234, 179, 8));
        Beep(1200, 40); Beep(700, 50);

    } else if (runeIdx == RUNE_VOID) {
        int targetX = g_player.x;
        int targetY = g_player.y;
        BOOL blinked = FALSE;
        for (int dist = 3; dist >= 1; dist--) {
            int tx = g_player.x + dx * dist;
            int ty = g_player.y + dy * dist;
            if (tx > 0 && tx < MAP_WIDTH - 1 && ty > 0 && ty < MAP_HEIGHT - 1) {
                int t = g_dungeon[ty][tx];
                if (t == TILE_FLOOR || t == TILE_WATER || t == TILE_DOOR_OPEN || t == TILE_STAIRS_DOWN || t == TILE_STAIRS_UP) {
                    targetX = tx; targetY = ty;
                    blinked = TRUE;
                    break;
                }
            }
        }
        if (blinked) {
            if (g_numSpellFX < MAX_SPELL_FX) {
                g_spellFX[g_numSpellFX].type = 3;
                g_spellFX[g_numSpellFX].x = targetX;
                g_spellFX[g_numSpellFX].y = targetY;
                g_spellFX[g_numSpellFX].radius = 12;
                g_spellFX[g_numSpellFX].duration = 7;
                g_spellFX[g_numSpellFX].color = RGB(168, 85, 247);
                g_numSpellFX++;
            }
            g_player.x = targetX;
            g_player.y = targetY;

            // Stun adjacent monsters
            for (int m = 0; m < g_numMonsters; m++) {
                if (g_monsters[m].alive && abs(g_monsters[m].x - g_player.x) <= 1 && abs(g_monsters[m].y - g_player.y) <= 1) {
                    if (g_monsters[m].freezeTurns < 1) g_monsters[m].freezeTurns = 1;
                    SpawnCombatText((float)g_monsters[m].x, (float)g_monsters[m].y, "STUNNED", RGB(168, 85, 247));
                }
            }

            AddLog("VOID WARP! You phase-shift through space, slipping through obstacles!", COLOR_ACCENT_PURPLE);
            Beep(200, 80); Beep(550, 60);
        } else {
            AddLog("VOID WARP! Spatial distortions flare, but solid stone blocks destination.", COLOR_ACCENT_PURPLE);
        }

    } else if (runeIdx == RUNE_AEGIS) {
        g_player.shield += 35;
        if (g_player.shield > 60) g_player.shield = 60;
        g_player.sanity += 15;
        if (g_player.sanity > g_player.max_sanity) g_player.sanity = g_player.max_sanity;
        AddLog("AEGIS WARD! Luminous runic barrier envelops you (+35 Shield, +15 Sanity)!", COLOR_BORDER_GLOW);
        Beep(554, 60); Beep(659, 80);
    }

    AdvanceTurn();
}

void SocketRune(int socketIdx, int runeIdx) {
    if (runeIdx < 0 || runeIdx >= NUM_RUNES || !g_player.ownedRunes[runeIdx]) return;
    int maxS = g_staffDefs[g_player.equippedStaff].maxSockets;
    if (socketIdx < 0 || socketIdx >= maxS) return;

    for (int s = 0; s < maxS; s++) {
        if (g_player.staffSockets[s] == runeIdx) {
            g_player.staffSockets[s] = -1;
        }
    }

    g_player.staffSockets[socketIdx] = runeIdx;
    RecalcPlayerStats();
    char buf[128];
    snprintf(buf, sizeof(buf), "Inscribed %s into Socket %d! Granted spell: %s.", g_runeDefs[runeIdx].name, socketIdx + 1, g_runeDefs[runeIdx].spellName);
    AddLog(buf, COLOR_BORDER_GLOW);
    Beep(659, 60);
}

void UnsocketRune(int socketIdx) {
    int maxS = g_staffDefs[g_player.equippedStaff].maxSockets;
    if (socketIdx < 0 || socketIdx >= maxS) return;
    int r = g_player.staffSockets[socketIdx];
    if (r >= 0 && r < NUM_RUNES) {
        g_player.staffSockets[socketIdx] = -1;
        RecalcPlayerStats();
        char buf[128];
        snprintf(buf, sizeof(buf), "Unsocketed %s from Staff Socket %d.", g_runeDefs[r].name, socketIdx + 1);
        AddLog(buf, COLOR_TEXT_DIM);
        Beep(330, 40);
    }
}

void SearchArea(void) {
    AddLog("You search surrounding stonework for hidden secrets...", COLOR_ACCENT_CYAN);
    BOOL found = FALSE;
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            int nx = g_player.x + dx;
            int ny = g_player.y + dy;
            if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT) {
                if (g_dungeon[ny][nx] == TILE_CHEST) {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Detected ancient Relic Coffer at (%d, %d)!", nx, ny);
                    AddLog(buf, COLOR_TEXT_GOLD);
                    found = TRUE;
                } else if (g_dungeon[ny][nx] == TILE_ALTAR) {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Detected consecrated Relic Enchanting Altar at (%d, %d)!", nx, ny);
                    AddLog(buf, COLOR_TEXT_GOLD);
                    found = TRUE;
                } else if (g_dungeon[ny][nx] == TILE_SHRINE) {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Detected glowing Ancient Runic Shrine at (%d, %d)!", nx, ny);
                    AddLog(buf, COLOR_TEXT_RUNE);
                    found = TRUE;
                } else if (g_dungeon[ny][nx] == TILE_MERCHANT) {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Spotted Subterranean Merchant stall at (%d, %d)!", nx, ny);
                    AddLog(buf, COLOR_TEXT_GOLD);
                    found = TRUE;
                } else if (g_dungeon[ny][nx] == TILE_CAULDRON) {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Detected Ancient Alchemy Cauldron at (%d, %d)!", nx, ny);
                    AddLog(buf, RGB(52, 211, 153));
                    found = TRUE;
                }
            }
        }
    }
    if (!found) {
        AddLog("No hidden traps or occult glyphs detected.", COLOR_TEXT_DIM);
    }
    Beep(520, 40);
    AdvanceTurn();
}

void InteractTile(void) {
    int cur = g_dungeon[g_player.y][g_player.x];
    if (cur == TILE_STAIRS_DOWN) {
        g_depthLevel++;
        DepthZone z = GetDepthZone(g_depthLevel);
        char buf[128];
        snprintf(buf, sizeof(buf), "Descended into Depth B%d (%s)...", g_depthLevel, g_zoneThemes[z].shortName);
        AddLog(buf, COLOR_ACCENT_AMBER);
        Beep(300, 60); Beep(450, 80);
        InitGame(g_depthLevel);
    } else if (cur == TILE_ALTAR) {
        OpenEnchantAltar();
    } else if (cur == TILE_MERCHANT) {
        OpenMerchantShop();
    } else if (cur == TILE_SHRINE) {
        CommuneShrine(g_player.x, g_player.y);
    } else if (cur == TILE_CAULDRON) {
        AddLog("Standing before Ancient Alchemy Cauldron! Switched to Tab [2] Pack to brew potions.", RGB(52, 211, 153));
        g_activeTab = 1;
        Beep(650, 50);
    } else if (cur == TILE_DOOR_CLOSED) {
        g_dungeon[g_player.y][g_player.x] = TILE_DOOR_OPEN;
        AddLog("Pushed open the door.", COLOR_ACCENT_CYAN);
        AdvanceTurn();
    } else {
        BOOL interacted = FALSE;
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int nx = g_player.x + dx;
                int ny = g_player.y + dy;
                if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT) {
                    if (g_dungeon[ny][nx] == TILE_ALTAR) {
                        OpenEnchantAltar();
                        interacted = TRUE;
                        break;
                    } else if (g_dungeon[ny][nx] == TILE_MERCHANT) {
                        OpenMerchantShop();
                        interacted = TRUE;
                        break;
                    } else if (g_dungeon[ny][nx] == TILE_SHRINE) {
                        CommuneShrine(nx, ny);
                        interacted = TRUE;
                        break;
                    } else if (g_dungeon[ny][nx] == TILE_CAULDRON) {
                        AddLog("Approached Ancient Alchemy Cauldron! Switched to Tab [2] Pack to brew potions.", RGB(52, 211, 153));
                        g_activeTab = 1;
                        Beep(650, 50);
                        interacted = TRUE;
                        break;
                    } else if (g_dungeon[ny][nx] == TILE_DOOR_CLOSED) {
                        g_dungeon[ny][nx] = TILE_DOOR_OPEN;
                        AddLog("Pushed open the door.", COLOR_ACCENT_CYAN);
                        Beep(350, 40);
                        AdvanceTurn();
                        interacted = TRUE;
                        break;
                    }
                }
            }
            if (interacted) break;
        }
        if (!interacted) {
            AddLog("Nothing to interact with here.", COLOR_TEXT_DIM);
        }
    }
}

// Camera update
void UpdateCamera(void) {
    int targetX = g_player.x * TILE_SIZE + TILE_SIZE / 2 - VIEWPORT_W / 2;
    int targetY = g_player.y * TILE_SIZE + TILE_SIZE / 2 - VIEWPORT_H / 2;

    int maxX = MAP_WIDTH * TILE_SIZE - VIEWPORT_W;
    int maxY = MAP_HEIGHT * TILE_SIZE - VIEWPORT_H;

    if (targetX < 0) targetX = 0;
    if (targetX > maxX) targetX = maxX;
    if (targetY < 0) targetY = 0;
    if (targetY > maxY) targetY = maxY;

    g_camX = targetX;
    g_camY = targetY;
}

// --- Custom GDI Sprite Rendering Engines ---
static void DrawDelverSprite(HDC hdc, int x, int y, int facing, int frame, BOOL hasShield) {
    int cx = x + 16;
    int cy = y + 16;
    int bob = (int)(sinf((float)frame * 0.25f) * 1.5f);
    int by = cy + bob;

    // Floor shadow
    HBRUSH shadowBr = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH oldB = (HBRUSH)SelectObject(hdc, shadowBr);
    HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldP = (HPEN)SelectObject(hdc, nullPen);
    Ellipse(hdc, cx - 8, cy + 8, cx + 8, cy + 14);

    // Cloak / Torso (deep abyssal dark navy/slate)
    HBRUSH cloakBr = CreateSolidBrush(RGB(15, 23, 42));
    SelectObject(hdc, cloakBr);
    POINT cloakPts[4] = {
        {cx - 7, by + 10},
        {cx - 8, by - 2},
        {cx + 8, by - 2},
        {cx + 7, by + 10}
    };
    Polygon(hdc, cloakPts, 4);
    DeleteObject(cloakBr);

    // Pauldrons (steel & cyan runes)
    HBRUSH pauldBr = CreateSolidBrush(RGB(30, 41, 59));
    SelectObject(hdc, pauldBr);
    RECT pR = {cx - 9, by - 3, cx + 9, by + 1};
    FillRect(hdc, &pR, pauldBr);
    DeleteObject(pauldBr);

    HBRUSH runeBr = CreateSolidBrush(RGB(2, 132, 199));
    RECT rL = {cx - 8, by - 2, cx - 5, by};
    RECT rR = {cx + 5, by - 2, cx + 8, by};
    FillRect(hdc, &rL, runeBr);
    FillRect(hdc, &rR, runeBr);
    DeleteObject(runeBr);

    // Dark Hood / Cowl
    HBRUSH hoodBr = CreateSolidBrush(RGB(9, 13, 22));
    SelectObject(hdc, hoodBr);
    HPEN hoodPen = CreatePen(PS_SOLID, 1, RGB(30, 41, 59));
    SelectObject(hdc, hoodPen);
    Ellipse(hdc, cx - 7, by - 9, cx + 7, by + 3);
    SelectObject(hdc, nullPen);
    DeleteObject(hoodPen);
    DeleteObject(hoodBr);

    // Face shadow opening
    HBRUSH faceBr = CreateSolidBrush(RGB(2, 4, 8));
    SelectObject(hdc, faceBr);
    Ellipse(hdc, cx - 4, by - 6, cx + 4, by);
    DeleteObject(faceBr);

    // Glowing visor slits / eyes
    int eyeX = 0, eyeY = 0;
    if (facing == 3) eyeX = -2;      // left
    else if (facing == 1) eyeX = 2;  // right
    else if (facing == 0) eyeY = -2; // up
    else if (facing == 2) eyeY = 1;  // down

    if (facing != 0) {
        COLORREF visorCol = RGB(56, 189, 248);
        SetPixel(hdc, cx - 3 + eyeX, by - 4 + eyeY, visorCol);
        SetPixel(hdc, cx - 2 + eyeX, by - 4 + eyeY, visorCol);
        SetPixel(hdc, cx + 1 + eyeX, by - 4 + eyeY, visorCol);
        SetPixel(hdc, cx + 2 + eyeX, by - 4 + eyeY, visorCol);
    }

    // Off-hand lantern
    int lanX = (facing == 3) ? (cx + 8) : (cx - 8);
    int lanY = by + 2;
    // Chain
    HPEN chainPen = CreatePen(PS_SOLID, 1, RGB(148, 163, 184));
    SelectObject(hdc, chainPen);
    MoveToEx(hdc, cx - 5, by + 1, NULL);
    LineTo(hdc, lanX, lanY - 3);
    SelectObject(hdc, nullPen);
    DeleteObject(chainPen);

    // Lantern iron frame & amber fire
    RECT lanRect = {lanX - 3, lanY - 3, lanX + 3, lanY + 4};
    HBRUSH lanIron = CreateSolidBrush(RGB(69, 26, 3));
    FillRect(hdc, &lanRect, lanIron);
    DeleteObject(lanIron);

    RECT fireRect = {lanX - 2, lanY - 2, lanX + 2, lanY + 3};
    HBRUSH lanFire = CreateSolidBrush(RGB(245, 158, 11));
    FillRect(hdc, &fireRect, lanFire);
    DeleteObject(lanFire);

    SetPixel(hdc, lanX - 1, lanY - 1, RGB(254, 240, 138));
    SetPixel(hdc, lanX, lanY - 1, RGB(254, 240, 138));

    // Main-hand Runic Sword
    int bladeX = (facing == 3) ? (cx - 9) : (cx + 7);
    int bladeY = by - 1;
    HPEN bladePen = CreatePen(PS_SOLID, 1, RGB(203, 213, 225));
    SelectObject(hdc, bladePen);
    MoveToEx(hdc, bladeX, bladeY + 5, NULL);
    LineTo(hdc, bladeX + (facing == 3 ? -2 : 2), bladeY - 6);
    SelectObject(hdc, nullPen);
    DeleteObject(bladePen);

    SetPixel(hdc, bladeX, bladeY - 1, RGB(56, 189, 248));
    SetPixel(hdc, bladeX, bladeY - 2, RGB(56, 189, 248));

    // Crossguard
    HPEN guardPen = CreatePen(PS_SOLID, 1, RGB(217, 119, 6));
    SelectObject(hdc, guardPen);
    MoveToEx(hdc, bladeX - 2, bladeY + 3, NULL);
    LineTo(hdc, bladeX + 2, bladeY + 3);
    SelectObject(hdc, nullPen);
    DeleteObject(guardPen);

    // Prismatic Shield Halo if Ward active
    if (hasShield) {
        HPEN shieldPen = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
        SelectObject(hdc, shieldPen);
        int shRad = 15 + (int)(sinf((float)frame * 0.2f) * 2.0f);
        Ellipse(hdc, cx - shRad, cy - shRad, cx + shRad, cy + shRad);
        SelectObject(hdc, nullPen);
        DeleteObject(shieldPen);
    }

    SelectObject(hdc, oldP);
    SelectObject(hdc, oldB);
    DeleteObject(shadowBr);
}

static void DrawMonsterSprite(HDC hdc, int x, int y, int type, int frame, int state, int freeze) {
    int cx = x + 16;
    int cy = y + 16;

    // Floor shadow
    HBRUSH shadowBr = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH oldB = (HBRUSH)SelectObject(hdc, shadowBr);
    HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldP = (HPEN)SelectObject(hdc, nullPen);
    Ellipse(hdc, cx - 7, cy + 9, cx + 7, cy + 13);

    if (type == MONSTER_SKELETON) {
        int clatter = (int)(sinf((float)frame * 0.35f + (float)x) * 1.0f);
        int sy = cy + clatter;

        // Bleached skull
        HBRUSH skullBr = CreateSolidBrush(RGB(226, 232, 240));
        SelectObject(hdc, skullBr);
        Ellipse(hdc, cx - 5, sy - 9, cx + 5, sy + 1);
        RECT jawR = {cx - 3, sy - 1, cx + 3, sy + 3};
        FillRect(hdc, &jawR, skullBr);
        DeleteObject(skullBr);

        // Dark eye sockets & red eye glints
        HBRUSH eyeBr = CreateSolidBrush(RGB(15, 23, 42));
        RECT eL = {cx - 4, sy - 6, cx - 1, sy - 3};
        RECT eR = {cx + 1, sy - 6, cx + 4, sy - 3};
        FillRect(hdc, &eL, eyeBr);
        FillRect(hdc, &eR, eyeBr);
        DeleteObject(eyeBr);

        SetPixel(hdc, cx - 3, sy - 5, RGB(239, 68, 68));
        SetPixel(hdc, cx + 2, sy - 5, RGB(239, 68, 68));

        // Spine & ribs
        HPEN ribPen = CreatePen(PS_SOLID, 1, RGB(203, 213, 225));
        SelectObject(hdc, ribPen);
        MoveToEx(hdc, cx, sy + 2, NULL); LineTo(hdc, cx, sy + 9);
        MoveToEx(hdc, cx - 4, sy + 4, NULL); LineTo(hdc, cx + 5, sy + 4);
        MoveToEx(hdc, cx - 3, sy + 7, NULL); LineTo(hdc, cx + 4, sy + 7);
        SelectObject(hdc, nullPen);
        DeleteObject(ribPen);

        // Tarnished blade
        HPEN bladePen = CreatePen(PS_SOLID, 1, RGB(148, 163, 184));
        SelectObject(hdc, bladePen);
        MoveToEx(hdc, cx + 5, sy + 7, NULL); LineTo(hdc, cx + 9, sy - 4);
        SelectObject(hdc, nullPen);
        DeleteObject(bladePen);

    } else if (type == MONSTER_GHOUL) {
        int breathe = (int)(sinf((float)frame * 0.25f + (float)y) * 1.5f);
        int gy = cy + breathe;

        // Hunched mire-beast body
        HBRUSH ghoulBr = CreateSolidBrush(RGB(15, 118, 110));
        SelectObject(hdc, ghoulBr);
        Ellipse(hdc, cx - 7, gy - 2, cx + 6, gy + 8);
        DeleteObject(ghoulBr);

        // Dorsal teal spines
        HPEN spinePen = CreatePen(PS_SOLID, 1, RGB(45, 212, 191));
        SelectObject(hdc, spinePen);
        MoveToEx(hdc, cx - 4, gy - 2, NULL); LineTo(hdc, cx - 2, gy - 5); LineTo(hdc, cx, gy - 2);
        MoveToEx(hdc, cx, gy - 2, NULL); LineTo(hdc, cx + 2, gy - 5); LineTo(hdc, cx + 4, gy - 2);
        SelectObject(hdc, nullPen);
        DeleteObject(spinePen);

        // Ghoul head & jaws
        HBRUSH headBr = CreateSolidBrush(RGB(17, 94, 89));
        SelectObject(hdc, headBr);
        Ellipse(hdc, cx + 2, gy - 5, cx + 9, gy + 3);
        DeleteObject(headBr);

        // Glowing predatory eye
        SetPixel(hdc, cx + 5, gy - 3, RGB(250, 204, 21));
        SetPixel(hdc, cx + 6, gy - 3, RGB(4, 47, 46));

        // Sharp claws
        HPEN clawPen = CreatePen(PS_SOLID, 1, RGB(4, 47, 46));
        SelectObject(hdc, clawPen);
        MoveToEx(hdc, cx + 2, gy + 5, NULL); LineTo(hdc, cx + 6, gy + 9);
        MoveToEx(hdc, cx - 3, gy + 5, NULL); LineTo(hdc, cx - 1, gy + 9);
        SelectObject(hdc, nullPen);
        DeleteObject(clawPen);

    } else if (type == MONSTER_WRAITH) {
        int floatY = (int)(sinf((float)frame * 0.2f + (float)x) * 3.0f);
        int wy = cy + floatY;

        // Tattered purple cowl
        HBRUSH wraithBr = CreateSolidBrush(RGB(107, 33, 168));
        SelectObject(hdc, wraithBr);
        Ellipse(hdc, cx - 6, wy - 8, cx + 6, wy + 4);
        RECT tailR = {cx - 5, wy, cx + 5, wy + 6};
        FillRect(hdc, &tailR, wraithBr);
        DeleteObject(wraithBr);

        // Void face hollow
        HBRUSH voidBr = CreateSolidBrush(RGB(5, 2, 10));
        SelectObject(hdc, voidBr);
        Ellipse(hdc, cx - 4, wy - 5, cx + 4, wy + 1);
        DeleteObject(voidBr);

        // Amethyst phantom eyes
        SetPixel(hdc, cx - 2, wy - 3, RGB(233, 213, 255));
        SetPixel(hdc, cx + 1, wy - 3, RGB(233, 213, 255));

        // Spectral wisps
        HPEN wispPen = CreatePen(PS_SOLID, 1, RGB(147, 51, 234));
        SelectObject(hdc, wispPen);
        int w1 = (int)(sinf((float)frame * 0.3f) * 2.0f);
        int w2 = (int)(cosf((float)frame * 0.25f) * 2.0f);
        MoveToEx(hdc, cx - 3, wy + 6, NULL); LineTo(hdc, cx - 3 + w1, wy + 11);
        MoveToEx(hdc, cx + 1, wy + 6, NULL); LineTo(hdc, cx + 1 + w2, wy + 11);
        SelectObject(hdc, nullPen);
        DeleteObject(wispPen);

    } else if (type == MONSTER_ACOLYTE) {
        int chant = (int)(sinf((float)frame * 0.25f + (float)y) * 1.0f);
        int ay = cy + chant;

        // Dark cultist robe
        HBRUSH robeBr = CreateSolidBrush(RGB(41, 37, 36));
        SelectObject(hdc, robeBr);
        POINT robePts[4] = {
            {cx - 6, ay + 9},
            {cx - 7, ay - 2},
            {cx + 7, ay - 2},
            {cx + 6, ay + 9}
        };
        Polygon(hdc, robePts, 4);
        DeleteObject(robeBr);

        // Pointed hood
        HBRUSH hoodBr = CreateSolidBrush(RGB(28, 25, 23));
        SelectObject(hdc, hoodBr);
        POINT hoodPts[3] = {
            {cx, ay - 9},
            {cx + 6, ay - 1},
            {cx - 6, ay - 1}
        };
        Polygon(hdc, hoodPts, 3);
        DeleteObject(hoodBr);

        // Golden eyes in hood shadow
        SetPixel(hdc, cx - 2, ay - 3, RGB(253, 224, 71));
        SetPixel(hdc, cx + 1, ay - 3, RGB(253, 224, 71));

        // Ashwood Staff & glowing orb
        int staffX = cx + 8;
        HPEN staffPen = CreatePen(PS_SOLID, 1, RGB(120, 53, 15));
        SelectObject(hdc, staffPen);
        MoveToEx(hdc, staffX, ay + 9, NULL); LineTo(hdc, staffX, ay - 7);
        SelectObject(hdc, nullPen);
        DeleteObject(staffPen);

        HBRUSH orbBr = CreateSolidBrush(RGB(245, 158, 11));
        SelectObject(hdc, orbBr);
        Ellipse(hdc, staffX - 3, ay - 10, staffX + 3, ay - 4);
        DeleteObject(orbBr);

    } else if (type == MONSTER_LEVIATHAN) {
        int bossPulse = (int)(sinf((float)frame * 0.15f) * 1.5f);
        int by = cy + bossPulse;

        // Obsidian skull & carapace
        HBRUSH carBr = CreateSolidBrush(RGB(24, 24, 27));
        SelectObject(hdc, carBr);
        HPEN carPen = CreatePen(PS_SOLID, 1, RGB(88, 28, 135));
        SelectObject(hdc, carPen);
        Ellipse(hdc, cx - 10, by - 8, cx + 10, by + 8);
        SelectObject(hdc, nullPen);
        DeleteObject(carPen);
        DeleteObject(carBr);

        // Horns
        HPEN hornPen = CreatePen(PS_SOLID, 2, RGB(127, 29, 29));
        SelectObject(hdc, hornPen);
        MoveToEx(hdc, cx - 6, by - 4, NULL); LineTo(hdc, cx - 9, by - 12);
        MoveToEx(hdc, cx + 6, by - 4, NULL); LineTo(hdc, cx + 9, by - 12);
        SelectObject(hdc, nullPen);
        DeleteObject(hornPen);

        // Cluster of crimson eyes
        COLORREF redEye = RGB(239, 68, 68);
        SetPixel(hdc, cx - 4, by - 3, redEye);
        SetPixel(hdc, cx + 3, by - 3, redEye);
        SetPixel(hdc, cx - 2, by - 1, redEye);
        SetPixel(hdc, cx + 1, by - 1, redEye);

        // Fangs
        HPEN fangPen = CreatePen(PS_SOLID, 1, RGB(248, 250, 252));
        SelectObject(hdc, fangPen);
        MoveToEx(hdc, cx - 3, by + 3, NULL); LineTo(hdc, cx - 3, by + 5);
        MoveToEx(hdc, cx, by + 3, NULL); LineTo(hdc, cx, by + 5);
        MoveToEx(hdc, cx + 3, by + 3, NULL); LineTo(hdc, cx + 3, by + 5);
        SelectObject(hdc, nullPen);
        DeleteObject(fangPen);

    } else if (type == MONSTER_CRYPT_KEEPER) {
        int ky = cy + (int)(sinf((float)frame * 0.2f) * 1.5f);

        // Amber Runic Aura
        HPEN auraPen = CreatePen(PS_SOLID, 1, RGB(245, 158, 11));
        SelectObject(hdc, auraPen);
        Ellipse(hdc, cx - 14, ky - 14, cx + 14, ky + 14);
        SelectObject(hdc, nullPen);
        DeleteObject(auraPen);

        // Skull
        HBRUSH skullBr = CreateSolidBrush(RGB(241, 245, 249));
        SelectObject(hdc, skullBr);
        Ellipse(hdc, cx - 7, ky - 8, cx + 7, ky + 2);
        RECT jawR = {cx - 4, ky, cx + 4, ky + 4};
        FillRect(hdc, &jawR, skullBr);
        DeleteObject(skullBr);

        // Ancient Gold Crown
        HBRUSH crownBr = CreateSolidBrush(RGB(251, 191, 36));
        SelectObject(hdc, crownBr);
        POINT crPts[5] = {
            {cx - 7, ky - 7},
            {cx - 4, ky - 14},
            {cx, ky - 9},
            {cx + 4, ky - 14},
            {cx + 7, ky - 7}
        };
        Polygon(hdc, crPts, 5);
        DeleteObject(crownBr);

        // Crimson Glowing Eyes
        HBRUSH eyeBr = CreateSolidBrush(RGB(239, 68, 68));
        RECT eL = {cx - 4, ky - 6, cx - 1, ky - 3};
        RECT eR = {cx + 1, ky - 6, cx + 4, ky - 3};
        FillRect(hdc, &eL, eyeBr);
        FillRect(hdc, &eR, eyeBr);
        DeleteObject(eyeBr);

        // Heavy Runic Breastplate
        HBRUSH plateBr = CreateSolidBrush(RGB(30, 41, 59));
        SelectObject(hdc, plateBr);
        POINT plPts[4] = {
            {cx - 7, ky + 3},
            {cx + 7, ky + 3},
            {cx + 5, ky + 11},
            {cx - 5, ky + 11}
        };
        Polygon(hdc, plPts, 4);
        DeleteObject(plateBr);

        // Colossal Tomb Greatsword
        HPEN swordPen = CreatePen(PS_SOLID, 2, RGB(148, 163, 184));
        SelectObject(hdc, swordPen);
        MoveToEx(hdc, cx + 7, ky + 10, NULL); LineTo(hdc, cx + 12, ky - 11);
        SelectObject(hdc, nullPen);
        DeleteObject(swordPen);
        HPEN runePen = CreatePen(PS_SOLID, 1, RGB(239, 68, 68));
        SelectObject(hdc, runePen);
        MoveToEx(hdc, cx + 8, ky + 4, NULL); LineTo(hdc, cx + 11, ky - 5);
        SelectObject(hdc, nullPen);
        DeleteObject(runePen);

    } else if (type == MONSTER_ABYSSAL_WYRM) {
        // Serpentine body segments
        for (int s = 4; s >= 0; s--) {
            int segX = cx + (int)(sinf((float)frame * 0.35f - (float)s * 0.7f) * (float)(5 - s));
            int segY = cy + (s - 2) * 3;
            int rad = 6 - s;
            if (rad < 3) rad = 3;

            HBRUSH scaleBr = CreateSolidBrush((s % 2 == 0) ? RGB(6, 95, 70) : RGB(4, 120, 87));
            SelectObject(hdc, scaleBr);
            Ellipse(hdc, segX - rad, segY - rad, segX + rad, segY + rad);
            DeleteObject(scaleBr);

            // Glowing toxic dorsal ridge
            SetPixel(hdc, segX, segY - rad, RGB(52, 211, 153));
        }

        // Viper/Dragon Head
        int headY = cy - 6 + (int)(sinf((float)frame * 0.35f) * 2.0f);
        HBRUSH headBr = CreateSolidBrush(RGB(6, 78, 59));
        SelectObject(hdc, headBr);
        Ellipse(hdc, cx - 6, headY - 4, cx + 6, headY + 5);
        DeleteObject(headBr);

        // Toxic eyes
        SetPixel(hdc, cx - 3, headY - 2, RGB(250, 204, 21));
        SetPixel(hdc, cx + 2, headY - 2, RGB(250, 204, 21));

        // Horns
        HPEN hornPen = CreatePen(PS_SOLID, 1, RGB(5, 150, 105));
        SelectObject(hdc, hornPen);
        MoveToEx(hdc, cx - 3, headY - 2, NULL); LineTo(hdc, cx - 7, headY - 7);
        MoveToEx(hdc, cx + 3, headY - 2, NULL); LineTo(hdc, cx + 7, headY - 7);
        SelectObject(hdc, nullPen);
        DeleteObject(hornPen);

        // Acid fangs
        SetPixel(hdc, cx - 2, headY + 3, RGB(248, 250, 252));
        SetPixel(hdc, cx + 1, headY + 3, RGB(248, 250, 252));
        SetPixel(hdc, cx - 1, headY + 5, RGB(16, 185, 129));

    } else if (type == MONSTER_VOID_MONARCH) {
        int my = cy + (int)(sinf((float)frame * 0.2f) * 2.5f);

        // Void Cosmic Aura
        HPEN auraPen = CreatePen(PS_SOLID, 1, RGB(168, 85, 247));
        SelectObject(hdc, auraPen);
        Ellipse(hdc, cx - 15, my - 15, cx + 15, my + 15);
        SelectObject(hdc, nullPen);
        DeleteObject(auraPen);

        // Orbiting Void Crystals
        for (int o = 0; o < 3; o++) {
            float ang = (float)frame * 0.15f + (float)o * 2.094f;
            int ox = cx + (int)(cosf(ang) * 12.0f);
            int oy = my + (int)(sinf(ang) * 6.0f);
            HBRUSH orbBr = CreateSolidBrush(RGB(192, 132, 252));
            SelectObject(hdc, orbBr);
            Ellipse(hdc, ox - 2, oy - 2, ox + 3, oy + 3);
            DeleteObject(orbBr);
        }

        // Obsidian Astral Mantle
        HBRUSH mantleBr = CreateSolidBrush(RGB(59, 7, 100));
        SelectObject(hdc, mantleBr);
        POINT mPts[4] = {
            {cx - 8, my + 10},
            {cx - 10, my - 3},
            {cx + 10, my - 3},
            {cx + 8, my + 10}
        };
        Polygon(hdc, mPts, 4);
        DeleteObject(mantleBr);

        // Pointed Void Crown
        HBRUSH crBr = CreateSolidBrush(RGB(126, 34, 206));
        SelectObject(hdc, crBr);
        POINT crPts[5] = {
            {cx - 7, my - 4},
            {cx - 5, my - 12},
            {cx, my - 7},
            {cx + 5, my - 12},
            {cx + 7, my - 4}
        };
        Polygon(hdc, crPts, 5);
        DeleteObject(crBr);

        // Center Cosmic Eye Singularity
        HBRUSH eyeBg = CreateSolidBrush(RGB(2, 6, 23));
        SelectObject(hdc, eyeBg);
        Ellipse(hdc, cx - 4, my - 4, cx + 4, my + 4);
        DeleteObject(eyeBg);

        HBRUSH eyeCore = CreateSolidBrush(RGB(233, 213, 255));
        SelectObject(hdc, eyeCore);
        Ellipse(hdc, cx - 2, my - 2, cx + 2, my + 2);
        DeleteObject(eyeCore);
    }

    SelectObject(hdc, oldP);
    SelectObject(hdc, oldB);
    DeleteObject(shadowBr);
}

static void DrawTileSprite(HDC hdc, int x, int y, int tile, BOOL isVisible, int frame, const ZoneTheme* zt) {
    int cx = x + 16;
    int cy = y + 16;

    if (tile == TILE_CHEST) {
        // Oak chest body
        RECT cR = {x + 5, y + 8, x + 27, y + 24};
        HBRUSH woodBr = CreateSolidBrush(isVisible ? RGB(69, 26, 3) : RGB(30, 17, 8));
        FillRect(hdc, &cR, woodBr);
        DeleteObject(woodBr);

        // Iron bands
        HBRUSH ironBr = CreateSolidBrush(isVisible ? RGB(30, 41, 59) : RGB(15, 23, 42));
        RECT iL = {x + 5, y + 8, x + 9, y + 24};
        RECT iR = {x + 23, y + 8, x + 27, y + 24};
        RECT iM = {x + 13, y + 8, x + 19, y + 24};
        FillRect(hdc, &iL, ironBr);
        FillRect(hdc, &iR, ironBr);
        FillRect(hdc, &iM, ironBr);
        DeleteObject(ironBr);

        // Brass lock plate & keyhole
        HBRUSH goldBr = CreateSolidBrush(isVisible ? RGB(251, 191, 36) : RGB(120, 53, 15));
        RECT gR = {x + 14, y + 12, x + 18, y + 18};
        FillRect(hdc, &gR, goldBr);
        DeleteObject(goldBr);

        SetPixel(hdc, x + 15, y + 14, RGB(28, 25, 23));
        SetPixel(hdc, x + 16, y + 14, RGB(28, 25, 23));

    } else if (tile == TILE_ALTAR) {
        // Obsidian stepped plinth
        RECT bR1 = {x + 3, y + 6, x + 29, y + 26};
        HBRUSH baseBr = CreateSolidBrush(isVisible ? RGB(28, 25, 23) : RGB(12, 10, 9));
        FillRect(hdc, &bR1, baseBr);
        DeleteObject(baseBr);

        RECT bR2 = {x + 5, y + 4, x + 27, y + 20};
        HBRUSH topBr = CreateSolidBrush(isVisible ? RGB(46, 16, 23) : RGB(20, 6, 10));
        FillRect(hdc, &bR2, topBr);
        DeleteObject(topBr);

        // Crimson runes
        HPEN runePen = CreatePen(PS_SOLID, 1, isVisible ? RGB(239, 68, 68) : RGB(127, 29, 29));
        HPEN oldP = (HPEN)SelectObject(hdc, runePen);
        MoveToEx(hdc, cx - 4, cy - 3, NULL); LineTo(hdc, cx, cy - 7); LineTo(hdc, cx + 4, cy - 3);
        MoveToEx(hdc, cx, cy - 7, NULL); LineTo(hdc, cx, cy);
        SelectObject(hdc, oldP);
        DeleteObject(runePen);

        // Corner candles
        if (isVisible) {
            SetPixel(hdc, x + 5, y + 2, RGB(249, 115, 22));
            SetPixel(hdc, x + 26, y + 2, RGB(249, 115, 22));
        }

    } else if (tile == TILE_CAULDRON) {
        HBRUSH ironBr = CreateSolidBrush(isVisible ? RGB(30, 41, 59) : RGB(15, 23, 42));
        HBRUSH oldB = (HBRUSH)SelectObject(hdc, ironBr);
        HPEN nullP = (HPEN)GetStockObject(NULL_PEN);
        HPEN oldP = (HPEN)SelectObject(hdc, nullP);

        // Pot body
        Ellipse(hdc, cx - 8, cy - 4, cx + 8, cy + 10);

        // Emerald potion surface
        HBRUSH brewBr = CreateSolidBrush(isVisible ? RGB(5, 150, 105) : RGB(2, 44, 34));
        SelectObject(hdc, brewBr);
        Ellipse(hdc, cx - 6, cy - 5, cx + 6, cy - 1);
        DeleteObject(brewBr);

        // Animated bubble
        if (isVisible) {
            int bPhase = (frame * 2) % 10;
            SetPixel(hdc, cx - 2, cy - 3 - bPhase / 2, RGB(52, 211, 153));
        }

        SelectObject(hdc, oldP);
        SelectObject(hdc, oldB);
        DeleteObject(ironBr);

    } else if (tile == TILE_DOOR_CLOSED) {
        // Oak door planks
        RECT dR = {x + 4, y + 2, x + 28, y + 30};
        HBRUSH doorBr = CreateSolidBrush(isVisible ? RGB(120, 53, 15) : RGB(41, 27, 11));
        FillRect(hdc, &dR, doorBr);
        DeleteObject(doorBr);

        // Plank divisions
        HPEN divPen = CreatePen(PS_SOLID, 1, isVisible ? RGB(69, 26, 3) : RGB(24, 15, 6));
        HPEN oldP = (HPEN)SelectObject(hdc, divPen);
        MoveToEx(hdc, x + 12, y + 2, NULL); LineTo(hdc, x + 12, y + 30);
        MoveToEx(hdc, x + 20, y + 2, NULL); LineTo(hdc, x + 20, y + 30);

        // Iron crossbars
        HBRUSH barBr = CreateSolidBrush(isVisible ? RGB(30, 41, 59) : RGB(15, 23, 42));
        RECT bar1 = {x + 4, y + 6, x + 28, y + 9};
        RECT bar2 = {x + 4, y + 22, x + 28, y + 25};
        FillRect(hdc, &bar1, barBr);
        FillRect(hdc, &bar2, barBr);
        DeleteObject(barBr);

        // Iron ring handle
        SelectObject(hdc, (HPEN)GetStockObject(NULL_PEN));
        HBRUSH ringBr = CreateSolidBrush(isVisible ? RGB(203, 213, 225) : RGB(71, 85, 105));
        HBRUSH oldB = (HBRUSH)SelectObject(hdc, ringBr);
        Ellipse(hdc, x + 20, y + 13, x + 25, y + 18);
        SelectObject(hdc, oldB);
        DeleteObject(ringBr);

        SelectObject(hdc, oldP);
        DeleteObject(divPen);

    } else if (tile == TILE_DOOR_OPEN) {
        RECT dR = {x + 4, y + 2, x + 28, y + 30};
        HBRUSH bgBr = CreateSolidBrush(isVisible ? RGB(15, 23, 42) : RGB(7, 10, 18));
        FillRect(hdc, &dR, bgBr);
        DeleteObject(bgBr);

        // Open door leaf on side
        RECT leafR = {x + 4, y + 2, x + 9, y + 30};
        HBRUSH leafBr = CreateSolidBrush(isVisible ? RGB(69, 26, 3) : RGB(24, 15, 6));
        FillRect(hdc, &leafR, leafBr);
        DeleteObject(leafBr);

        HPEN framePen = CreatePen(PS_SOLID, 1, isVisible ? RGB(51, 65, 85) : RGB(30, 41, 59));
        HPEN oldP = (HPEN)SelectObject(hdc, framePen);
        MoveToEx(hdc, x + 3, y + 2, NULL); LineTo(hdc, x + 29, y + 2);
        MoveToEx(hdc, x + 29, y + 2, NULL); LineTo(hdc, x + 29, y + 30);
        SelectObject(hdc, oldP);
        DeleteObject(framePen);

    } else if (tile == TILE_STAIRS_DOWN) {
        RECT sR = {x + 3, y + 3, x + 29, y + 29};
        HBRUSH bgBr = CreateSolidBrush(RGB(3, 5, 8));
        FillRect(hdc, &sR, bgBr);
        DeleteObject(bgBr);

        for (int s = 0; s < 4; s++) {
            int sy = y + 4 + s * 6;
            RECT stepR = {x + 4 + s * 2, sy, x + 28 - s * 2, sy + 5};
            int val = isVisible ? (60 - s * 14) : (35 - s * 8);
            if (val < 0) val = 0;
            HBRUSH stepBr = CreateSolidBrush(RGB(val, val + 5, val + 15));
            FillRect(hdc, &stepR, stepBr);
            DeleteObject(stepBr);
        }

        // Descending purple marker
        HPEN mPen = CreatePen(PS_SOLID, 1, isVisible ? RGB(168, 85, 247) : RGB(88, 28, 135));
        HPEN oldP = (HPEN)SelectObject(hdc, mPen);
        MoveToEx(hdc, cx - 4, cy, NULL); LineTo(hdc, cx, cy + 4); LineTo(hdc, cx + 4, cy);
        SelectObject(hdc, oldP);
        DeleteObject(mPen);

    } else if (tile == TILE_STAIRS_UP) {
        RECT sR = {x + 3, y + 3, x + 29, y + 29};
        HBRUSH bgBr = CreateSolidBrush(RGB(3, 5, 8));
        FillRect(hdc, &sR, bgBr);
        DeleteObject(bgBr);

        for (int s = 0; s < 4; s++) {
            int sy = y + 22 - s * 6;
            RECT stepR = {x + 4 + (3 - s) * 2, sy, x + 28 - (3 - s) * 2, sy + 5};
            int val = isVisible ? (20 + s * 15) : (10 + s * 8);
            HBRUSH stepBr = CreateSolidBrush(RGB(val, val + 15, val + 35));
            FillRect(hdc, &stepR, stepBr);
            DeleteObject(stepBr);
        }

        // Ascending cyan marker
        HPEN mPen = CreatePen(PS_SOLID, 1, isVisible ? RGB(56, 189, 248) : RGB(2, 132, 199));
        HPEN oldP = (HPEN)SelectObject(hdc, mPen);
        MoveToEx(hdc, cx - 4, cy, NULL); LineTo(hdc, cx, cy - 4); LineTo(hdc, cx + 4, cy);
        SelectObject(hdc, oldP);
        DeleteObject(mPen);

    } else if (tile == TILE_PILLAR) {
        RECT colR = {x + 6, y + 2, x + 26, y + 30};
        HBRUSH colBr = CreateSolidBrush(isVisible ? RGB(30, 41, 59) : RGB(10, 15, 24));
        FillRect(hdc, &colR, colBr);
        DeleteObject(colBr);

        HBRUSH capBr = CreateSolidBrush(isVisible ? RGB(71, 85, 105) : RGB(30, 41, 59));
        RECT c1 = {x + 4, y + 2, x + 28, y + 5};
        RECT c2 = {x + 4, y + 27, x + 28, y + 30};
        FillRect(hdc, &c1, capBr);
        FillRect(hdc, &c2, capBr);
        DeleteObject(capBr);

        // Gem in pillar
        HBRUSH gemBr = CreateSolidBrush(isVisible ? zt->torchColor : RGB(49, 46, 129));
        HBRUSH oldB = (HBRUSH)SelectObject(hdc, gemBr);
        HPEN nullP = (HPEN)GetStockObject(NULL_PEN);
        HPEN oldP = (HPEN)SelectObject(hdc, nullP);
        Ellipse(hdc, cx - 3, cy - 3, cx + 3, cy + 3);
        SelectObject(hdc, oldP);
        SelectObject(hdc, oldB);
        DeleteObject(gemBr);
    } else if (tile == TILE_EFFIGY) {
        // Cursed Bone Totem / Horned Effigy
        RECT bR = {x + 6, y + 22, x + 26, y + 29};
        HBRUSH baseBr = CreateSolidBrush(isVisible ? RGB(59, 23, 71) : RGB(25, 10, 30));
        FillRect(hdc, &bR, baseBr);
        DeleteObject(baseBr);

        RECT pR = {x + 11, y + 8, x + 21, y + 23};
        HBRUSH boneBr = CreateSolidBrush(isVisible ? RGB(203, 213, 225) : RGB(70, 70, 80));
        FillRect(hdc, &pR, boneBr);
        DeleteObject(boneBr);

        if (isVisible) {
            HBRUSH skullBr = CreateSolidBrush(RGB(226, 232, 240));
            HBRUSH oldSk = (HBRUSH)SelectObject(hdc, skullBr);
            HPEN nullP = (HPEN)GetStockObject(NULL_PEN);
            HPEN oldP = (HPEN)SelectObject(hdc, nullP);
            Ellipse(hdc, x + 10, y + 3, x + 22, y + 15);

            HPEN hornPen = CreatePen(PS_SOLID, 1, RGB(168, 85, 247));
            SelectObject(hdc, hornPen);
            MoveToEx(hdc, x + 11, y + 6, NULL); LineTo(hdc, x + 7, y + 1);
            MoveToEx(hdc, x + 21, y + 6, NULL); LineTo(hdc, x + 25, y + 1);
            SelectObject(hdc, oldP);
            DeleteObject(hornPen);
            SelectObject(hdc, oldSk);
            DeleteObject(skullBr);

            COLORREF eyeColor = ((frame / 8) % 2 == 0) ? RGB(239, 68, 68) : RGB(168, 85, 247);
            SetPixel(hdc, x + 13, y + 8, eyeColor);
            SetPixel(hdc, x + 14, y + 8, eyeColor);
            SetPixel(hdc, x + 17, y + 8, eyeColor);
            SetPixel(hdc, x + 18, y + 8, eyeColor);
        }
    } else if (tile == TILE_SHRINE) {
        // Ancient Runic Shrine - Stepped pedestal with levitating rune crystal
        RECT bR1 = {x + 4, y + 20, x + 28, y + 29};
        HBRUSH baseBr = CreateSolidBrush(isVisible ? RGB(30, 41, 59) : RGB(15, 20, 30));
        FillRect(hdc, &bR1, baseBr);
        DeleteObject(baseBr);

        RECT bR2 = {x + 8, y + 14, x + 24, y + 21};
        HBRUSH midBr = CreateSolidBrush(isVisible ? RGB(51, 65, 85) : RGB(25, 32, 42));
        FillRect(hdc, &bR2, midBr);
        DeleteObject(midBr);

        // Floating Runic Crystal / Monolith
        if (isVisible) {
            int floatOffset = ((frame / 6) % 4);
            int crystalY = cy - 6 - floatOffset;

            // Celestial glow halo
            HBRUSH glowBr = CreateSolidBrush(RGB(20, 45, 75));
            HBRUSH oldG = (HBRUSH)SelectObject(hdc, glowBr);
            HPEN nullP = (HPEN)GetStockObject(NULL_PEN);
            HPEN oldP = (HPEN)SelectObject(hdc, nullP);
            Ellipse(hdc, cx - 10, crystalY - 10, cx + 10, crystalY + 10);

            // Diamond Crystal
            POINT pts[4] = {
                { cx, crystalY - 8 },
                { cx + 6, crystalY },
                { cx, crystalY + 8 },
                { cx - 6, crystalY }
            };
            COLORREF cryCol = ((frame / 12) % 2 == 0) ? RGB(6, 182, 212) : RGB(129, 140, 248);
            HBRUSH cryBr = CreateSolidBrush(cryCol);
            SelectObject(hdc, cryBr);
            Polygon(hdc, pts, 4);
            DeleteObject(cryBr);

            SelectObject(hdc, oldP);
            SelectObject(hdc, oldG);
            DeleteObject(glowBr);

            // Radiant Core
            SetPixel(hdc, cx, crystalY, RGB(255, 255, 255));
            SetPixel(hdc, cx - 1, crystalY, RGB(224, 242, 254));
            SetPixel(hdc, cx + 1, crystalY, RGB(224, 242, 254));
        }
    } else if (tile == TILE_MERCHANT) {
        // 1. Velvet/Abyssal woven carpet on floor
        RECT rugR = {x + 3, y + 16, x + 29, y + 30};
        HBRUSH rugBr = CreateSolidBrush(isVisible ? ((g_depthLevel > 6) ? RGB(49, 14, 61) : RGB(88, 28, 28)) : RGB(25, 10, 15));
        FillRect(hdc, &rugR, rugBr);
        DeleteObject(rugBr);

        // Carpet gold fringe border
        if (isVisible) {
            HPEN fringePen = CreatePen(PS_SOLID, 1, RGB(217, 119, 6));
            HPEN oldP = (HPEN)SelectObject(hdc, fringePen);
            MoveToEx(hdc, x + 3, y + 16, NULL); LineTo(hdc, x + 29, y + 16);
            MoveToEx(hdc, x + 3, y + 30, NULL); LineTo(hdc, x + 29, y + 30);
            SelectObject(hdc, oldP);
            DeleteObject(fringePen);
        }

        // 2. Merchant Stall Counter
        RECT stallR = {x + 5, y + 13, x + 27, y + 21};
        HBRUSH stallBr = CreateSolidBrush(isVisible ? RGB(69, 26, 3) : RGB(30, 15, 5));
        FillRect(hdc, &stallR, stallBr);
        DeleteObject(stallBr);

        // Counter edge highlight
        if (isVisible) {
            RECT rimR = {x + 5, y + 12, x + 27, y + 14};
            HBRUSH rimBr = CreateSolidBrush(RGB(146, 64, 14));
            FillRect(hdc, &rimR, rimBr);
            DeleteObject(rimBr);
        }

        // 3. Hooded Merchant Figure behind counter
        HBRUSH cloakBr = CreateSolidBrush(isVisible ? ((g_depthLevel > 6) ? RGB(30, 10, 50) : RGB(45, 15, 5)) : RGB(15, 10, 20));
        HBRUSH oldB = (HBRUSH)SelectObject(hdc, cloakBr);
        HPEN nullP = (HPEN)GetStockObject(NULL_PEN);
        HPEN oldP = (HPEN)SelectObject(hdc, nullP);
        Ellipse(hdc, cx - 6, cy - 12, cx + 6, cy + 2);
        SelectObject(hdc, oldP);
        SelectObject(hdc, oldB);
        DeleteObject(cloakBr);

        // 4. Glowing eyes under deep hood
        if (isVisible) {
            int blink = ((frame / 20) % 8 == 0);
            if (!blink) {
                COLORREF eyeCol = (g_depthLevel > 6) ? RGB(168, 85, 247) : RGB(251, 191, 36);
                SetPixel(hdc, cx - 3, cy - 6, eyeCol);
                SetPixel(hdc, cx + 2, cy - 6, eyeCol);
            }

            // 5. Brass lantern on stall
            RECT lR = {x + 7, y + 14, x + 10, y + 19};
            HBRUSH lBr = CreateSolidBrush(RGB(251, 191, 36));
            FillRect(hdc, &lR, lBr);
            DeleteObject(lBr);
            SetPixel(hdc, x + 8, y + 13, RGB(254, 240, 138));

            // 6. Floating Gold Coin / Essence Glint above head
            int coinOff = (int)(sinf((float)frame * 0.25f) * 2.0f);
            SetPixel(hdc, cx, y + 3 + coinOff, RGB(251, 191, 36));
            SetPixel(hdc, cx - 1, y + 3 + coinOff, RGB(254, 240, 138));
            SetPixel(hdc, cx + 1, y + 3 + coinOff, RGB(254, 240, 138));
            SetPixel(hdc, cx, y + 2 + coinOff, RGB(255, 255, 255));
            SetPixel(hdc, cx, y + 4 + coinOff, RGB(217, 119, 6));
        }
    }
}

static void DrawTorchSconce(HDC hdc, int x, int y, int frame, const ZoneTheme* zt, BOOL lit) {
    int cx = x + 16;
    int cy = y + 16;

    // Iron bracket
    RECT bR = {cx - 2, cy - 2, cx + 2, cy + 8};
    HBRUSH bBr = CreateSolidBrush(RGB(51, 65, 85));
    FillRect(hdc, &bR, bBr);
    DeleteObject(bBr);

    // Wood head
    RECT wR = {cx - 2, cy - 6, cx + 2, cy - 2};
    HBRUSH wBr = CreateSolidBrush(lit ? RGB(120, 53, 15) : RGB(40, 30, 25));
    FillRect(hdc, &wR, wBr);
    DeleteObject(wBr);

    if (lit) {
        // Animated flame shape
        int fOff = (int)(sinf((float)frame * 0.4f + (float)x) * 1.5f);
        HBRUSH flameBr = CreateSolidBrush(zt ? zt->torchColor : RGB(249, 115, 22));
        HBRUSH oldB = (HBRUSH)SelectObject(hdc, flameBr);
        HPEN nullP = (HPEN)GetStockObject(NULL_PEN);
        HPEN oldP = (HPEN)SelectObject(hdc, nullP);

        POINT flamePts[4] = {
            {cx - 3, cy - 5},
            {cx + fOff, cy - 12},
            {cx + 3, cy - 5},
            {cx, cy - 3}
        };
        Polygon(hdc, flamePts, 4);
        DeleteObject(flameBr);

        // Inner bright spark
        SetPixel(hdc, cx, cy - 7, RGB(253, 224, 71));
        SetPixel(hdc, cx + (fOff > 0 ? 1 : 0), cy - 8, RGB(254, 240, 138));

        SelectObject(hdc, oldP);
        SelectObject(hdc, oldB);
    } else {
        // Unlit charred wisp
        SetPixel(hdc, cx, cy - 7, RGB(100, 116, 139));
        SetPixel(hdc, cx, cy - 9, RGB(71, 85, 105));
    }
}

// Rendering
void RenderGame(HDC hdc, HWND hwnd) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;

    // Double buffering
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBM = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

    // Background
    HBRUSH bgBrush = CreateSolidBrush(COLOR_BG_ABYSS);
    FillRect(memDC, &clientRect, bgBrush);
    DeleteObject(bgBrush);

    // Fonts
    HFONT fontMono = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
    HFONT fontBold = CreateFontA(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
    HFONT fontSmall = CreateFontA(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
    HFONT fontTitle = CreateFontA(17, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

    HFONT oldFont = (HFONT)SelectObject(memDC, fontMono);
    SetBkMode(memDC, TRANSPARENT);

    // 1. TOP HEADER BAR (0..36)
    RECT headerRect = {0, 0, width, 36};
    HBRUSH panelDarkBrush = CreateSolidBrush(COLOR_BG_PANEL_DARK);
    FillRect(memDC, &headerRect, panelDarkBrush);

    // Header Title
    SelectObject(memDC, fontTitle);
    SetTextColor(memDC, COLOR_TEXT_BRIGHT);
    TextOutA(memDC, 14, 8, "KABYSS", 6);
    SelectObject(memDC, fontSmall);
    SetTextColor(memDC, COLOR_TEXT_DIM);
    TextOutA(memDC, 84, 11, "Abyssal Crypt Crawler v0.3", 26);

    // Header Badges
    SelectObject(memDC, fontSmall);
    char badgeBuf[64];
    
    // Depth Badge
    DepthZone z = GetDepthZone(g_depthLevel);
    const ZoneTheme* zt = &g_zoneThemes[z];
    snprintf(badgeBuf, sizeof(badgeBuf), "DEPTH: B%d", g_depthLevel);
    SetTextColor(memDC, COLOR_ACCENT_PURPLE);
    TextOutA(memDC, 220, 11, badgeBuf, (int)strlen(badgeBuf));

    // HP Badge
    snprintf(badgeBuf, sizeof(badgeBuf), "HP: %d/%d", g_player.hp, g_player.max_hp);
    SetTextColor(memDC, COLOR_ACCENT_RED);
    TextOutA(memDC, 340, 11, badgeBuf, (int)strlen(badgeBuf));

    // Sanity Badge
    snprintf(badgeBuf, sizeof(badgeBuf), "SAN: %d/%d", g_player.sanity, g_player.max_sanity);
    SetTextColor(memDC, COLOR_ACCENT_CYAN);
    TextOutA(memDC, 450, 11, badgeBuf, (int)strlen(badgeBuf));

    // Hunger Badge
    snprintf(badgeBuf, sizeof(badgeBuf), "HUNGER: %d/%d", g_player.hunger, g_player.max_hunger);
    SetTextColor(memDC, g_player.hunger <= 15 ? COLOR_ACCENT_RED : COLOR_ACCENT_AMBER);
    TextOutA(memDC, 570, 11, badgeBuf, (int)strlen(badgeBuf));

    // Aether Badge
    if (g_player.shield > 0) {
        snprintf(badgeBuf, sizeof(badgeBuf), "MP: %d/%d [+%d]", g_player.aether, g_player.max_aether, g_player.shield);
    } else {
        snprintf(badgeBuf, sizeof(badgeBuf), "MP: %d/%d", g_player.aether, g_player.max_aether);
    }
    SetTextColor(memDC, RGB(168, 85, 247));
    TextOutA(memDC, 705, 11, badgeBuf, (int)strlen(badgeBuf));

    // Essence Badge
    snprintf(badgeBuf, sizeof(badgeBuf), "ESSENCE: %d*", g_player.essence);
    SetTextColor(memDC, COLOR_TEXT_GOLD);
    TextOutA(memDC, 835, 11, badgeBuf, (int)strlen(badgeBuf));

    // Curse Badge (if active)
    if (g_player.curse != CURSE_NONE) {
        snprintf(badgeBuf, sizeof(badgeBuf), "!CURSE [%dt]", g_player.curseTurns);
        SetTextColor(memDC, RGB(239, 68, 68));
        TextOutA(memDC, 945, 11, badgeBuf, (int)strlen(badgeBuf));
    }

    // Header border line
    HPEN borderPen = CreatePen(PS_SOLID, 1, COLOR_BORDER);
    HPEN oldPen = (HPEN)SelectObject(memDC, borderPen);
    MoveToEx(memDC, 0, 36, NULL);
    LineTo(memDC, width, 36);

    // 2. MAIN DUNGEON VIEWPORT (12..716, 46..566)
    int vpX = 12;
    int vpY = 46;
    UpdateCamera();

    RECT vpRect = {vpX, vpY, vpX + VIEWPORT_W, vpY + VIEWPORT_H};
    HBRUSH vpBg = CreateSolidBrush(RGB(2, 3, 5));
    FillRect(memDC, &vpRect, vpBg);
    DeleteObject(vpBg);

    // Render Tiles
    int startCol = g_camX / TILE_SIZE;
    int endCol = (g_camX + VIEWPORT_W) / TILE_SIZE + 1;
    if (endCol >= MAP_WIDTH) endCol = MAP_WIDTH - 1;

    int startRow = g_camY / TILE_SIZE;
    int endRow = (g_camY + VIEWPORT_H) / TILE_SIZE + 1;
    if (endRow >= MAP_HEIGHT) endRow = MAP_HEIGHT - 1;

    float flicker = sinf((float)g_frameCount * 0.15f) * 0.05f;

    for (int y = startRow; y <= endRow; y++) {
        for (int x = startCol; x <= endCol; x++) {
            int scrX = vpX + (x * TILE_SIZE - g_camX);
            int scrY = vpY + (y * TILE_SIZE - g_camY);

            if (scrX + TILE_SIZE < vpX || scrX >= vpX + VIEWPORT_W ||
                scrY + TILE_SIZE < vpY || scrY >= vpY + VIEWPORT_H) continue;

            BOOL isExplored = g_explored[y][x];
            BOOL isVisible = g_visible[y][x];
            float light = isVisible ? (g_lightMap[y][x] + flicker) : 0.0f;
            if (light < 0.0f) light = 0.0f;
            if (light > 1.0f) light = 1.0f;

            if (!isExplored) {
                RECT tr = {scrX, scrY, scrX + TILE_SIZE, scrY + TILE_SIZE};
                HBRUSH voidBr = CreateSolidBrush(RGB(1, 2, 4));
                FillRect(memDC, &tr, voidBr);
                DeleteObject(voidBr);
                continue;
            }

            int tile = g_dungeon[y][x];
            COLORREF tileColor = RGB(7, 10, 18);

            if (tile == TILE_WALL) {
                int baseR = GetRValue(zt->wallColor);
                int baseG = GetGValue(zt->wallColor);
                int baseB = GetBValue(zt->wallColor);
                int r = isVisible ? (int)(baseR * (0.4f + light * 0.6f)) : (baseR / 3);
                int g = isVisible ? (int)(baseG * (0.4f + light * 0.6f)) : (baseG / 3);
                int b = isVisible ? (int)(baseB * (0.4f + light * 0.6f)) : (baseB / 3);
                tileColor = RGB(r, g, b);
            } else if (tile == TILE_FLOOR || tile == TILE_RUBBLE) {
                int baseR = GetRValue(zt->floorColor);
                int baseG = GetGValue(zt->floorColor);
                int baseB = GetBValue(zt->floorColor);
                int r = isVisible ? (int)(baseR * (0.3f + light * 0.7f)) : (baseR / 3);
                int g = isVisible ? (int)(baseG * (0.3f + light * 0.7f)) : (baseG / 3);
                int b = isVisible ? (int)(baseB * (0.3f + light * 0.7f)) : (baseB / 3);
                tileColor = RGB(r, g, b);
            } else if (tile == TILE_WATER) {
                tileColor = isVisible ? RGB(6, 78, 100) : RGB(4, 34, 45);
            } else if (tile == TILE_CHASM) {
                tileColor = RGB(2, 1, 8);
            } else if (tile == TILE_ALTAR) {
                tileColor = isVisible ? RGB(42, 10, 20) : RGB(20, 5, 10);
            } else if (tile == TILE_CAULDRON) {
                tileColor = isVisible ? RGB(6, 40, 30) : RGB(3, 20, 15);
            } else if (tile == TILE_PILLAR) {
                tileColor = isVisible ? RGB(30, 27, 75) : RGB(10, 12, 20);
            } else if (tile == TILE_DOOR_CLOSED) {
                tileColor = isVisible ? RGB(120, 53, 15) : RGB(41, 27, 11);
            } else if (tile == TILE_DOOR_OPEN) {
                tileColor = isVisible ? RGB(30, 41, 59) : RGB(10, 15, 29);
            } else if (tile == TILE_STAIRS_DOWN || tile == TILE_STAIRS_UP) {
                tileColor = isVisible ? RGB(30, 27, 75) : RGB(9, 12, 20);
            } else if (tile == TILE_CHEST) {
                tileColor = isVisible ? RGB(30, 41, 59) : RGB(7, 10, 18);
            } else if (tile == TILE_EFFIGY) {
                tileColor = isVisible ? RGB(35, 10, 45) : RGB(14, 5, 18);
            } else if (tile == TILE_SHRINE) {
                tileColor = isVisible ? RGB(16, 32, 54) : RGB(8, 16, 27);
            }

            RECT tr = {scrX, scrY, scrX + TILE_SIZE, scrY + TILE_SIZE};
            HBRUSH tileBr = CreateSolidBrush(tileColor);
            FillRect(memDC, &tr, tileBr);
            DeleteObject(tileBr);

            // Tile Glyphs / Details
            if (tile == TILE_WALL && isVisible) {
                HPEN brickPen = CreatePen(PS_SOLID, 1, zt->wallBorder);
                HPEN oldBrPen = (HPEN)SelectObject(memDC, brickPen);
                MoveToEx(memDC, scrX + 4, scrY + 10, NULL);
                LineTo(memDC, scrX + TILE_SIZE - 4, scrY + 10);
                MoveToEx(memDC, scrX + 4, scrY + 22, NULL);
                LineTo(memDC, scrX + TILE_SIZE - 4, scrY + 22);
                SelectObject(memDC, oldBrPen);
                DeleteObject(brickPen);
            } else if (tile == TILE_WATER && isVisible) {
                SelectObject(memDC, fontSmall);
                SetTextColor(memDC, RGB(56, 189, 248));
                TextOutA(memDC, scrX + 11, scrY + 8, "~", 1);
            } else if (tile == TILE_CHASM) {
                if (isExplored) {
                    SetPixel(memDC, scrX + 14, scrY + 14, RGB(168, 85, 247));
                    SetPixel(memDC, scrX + 15, scrY + 14, RGB(168, 85, 247));
                }
            } else if (tile == TILE_ALTAR || tile == TILE_SHRINE || tile == TILE_CAULDRON || tile == TILE_PILLAR ||
                       tile == TILE_DOOR_CLOSED || tile == TILE_DOOR_OPEN ||
                       tile == TILE_STAIRS_DOWN || tile == TILE_STAIRS_UP || tile == TILE_CHEST ||
                       tile == TILE_EFFIGY || tile == TILE_MERCHANT) {
                DrawTileSprite(memDC, scrX, scrY, tile, isVisible, g_frameCount, zt);
            }

            // Memory fog overlay for explored but not currently visible
            if (isExplored && !isVisible) {
                HBRUSH fogBr = CreateSolidBrush(RGB(4, 7, 12));
                RECT fRect = {scrX, scrY, scrX + TILE_SIZE, scrY + TILE_SIZE};
                FillRect(memDC, &fRect, fogBr);
                DeleteObject(fogBr);
            }
        }
    }

    // Draw Ambient Sconce Torches with warm halo and ember emission
    for (int t = 0; t < g_numTorches; t++) {
        int tx = g_torches[t].x;
        int ty = g_torches[t].y;
        if (g_visible[ty][tx]) {
            int scrX = vpX + (tx * TILE_SIZE - g_camX);
            int scrY = vpY + (ty * TILE_SIZE - g_camY);

            if (g_torches[t].lit) {
                // Sconce Warm Halo Glow
                int tFlicker = (int)(sinf((float)g_frameCount * 0.2f + (float)tx) * 3.0f);
                HBRUSH sconceHalo = CreateSolidBrush(zt->torchHalo);
                HBRUSH oldSc = (HBRUSH)SelectObject(memDC, sconceHalo);
                HPEN sconcePen = CreatePen(PS_SOLID, 1, zt->torchColor);
                HPEN oldScPen = (HPEN)SelectObject(memDC, sconcePen);
                Ellipse(memDC, scrX - 8 - tFlicker, scrY - 8 - tFlicker, scrX + TILE_SIZE + 8 + tFlicker, scrY + TILE_SIZE + 8 + tFlicker);
                SelectObject(memDC, oldScPen);
                DeleteObject(sconcePen);
                SelectObject(memDC, oldSc);
                DeleteObject(sconceHalo);

                // Sconce Iron Bracket & Animated Flame
                DrawTorchSconce(memDC, scrX, scrY, g_frameCount, zt, TRUE);

                if (RandInt(0, 10) < 2) {
                    SpawnEmber((float)(scrX + 16), (float)(scrY + 12), TRUE);
                }
            } else {
                DrawTorchSconce(memDC, scrX, scrY, g_frameCount, zt, FALSE);
            }
        }
    }

    // Draw Active Monsters
    SelectObject(memDC, fontBold);
    for (int m = 0; m < g_numMonsters; m++) {
        if (!g_monsters[m].alive) continue;
        int mx = g_monsters[m].x;
        int my = g_monsters[m].y;
        if (g_fovEnabled && !g_visible[my][mx]) continue;
        if (!g_fovEnabled && !g_explored[my][mx]) continue;

        int mScrX = vpX + (mx * TILE_SIZE - g_camX);
        int mScrY = vpY + (my * TILE_SIZE - g_camY);
        if (mScrX < vpX - TILE_SIZE || mScrX >= vpX + VIEWPORT_W ||
            mScrY < vpY - TILE_SIZE || mScrY >= vpY + VIEWPORT_H) continue;

        const MonsterDef* mdef = &g_monsterDefs[g_monsters[m].type];

        // Monster Custom Sprite & Animations
        DrawMonsterSprite(memDC, mScrX, mScrY, g_monsters[m].type, g_frameCount, g_monsters[m].state, g_monsters[m].freezeTurns);

        // Mini HP Bar
        int barW = TILE_SIZE - 8;
        int barH = 3;
        int barY = mScrY + 2;
        RECT mBarBg = { mScrX + 4, barY, mScrX + 4 + barW, barY + barH };
        HBRUSH mBarDark = CreateSolidBrush(RGB(10, 10, 10));
        FillRect(memDC, &mBarBg, mBarDark);
        DeleteObject(mBarDark);

        int curW = (int)((float)barW * ((float)g_monsters[m].hp / (float)g_monsters[m].max_hp));
        if (curW < 0) curW = 0; if (curW > barW) curW = barW;
        RECT mBarFill = { mScrX + 4, barY, mScrX + 4 + curW, barY + barH };
        HBRUSH mFill = CreateSolidBrush(COLOR_ACCENT_RED);
        FillRect(memDC, &mBarFill, mFill);
        DeleteObject(mFill);

        // Alert or Freeze indicator
        if (g_monsters[m].freezeTurns > 0) {
            SetTextColor(memDC, RGB(56, 189, 248));
            TextOutA(memDC, mScrX + 2, mScrY + 5, "*", 1);
        } else if (g_monsters[m].state == 1) {
            SetTextColor(memDC, COLOR_ACCENT_AMBER);
            TextOutA(memDC, mScrX + 2, mScrY + 5, "!", 1);
        }
    }

    // Draw Player / Delver with Torchlight Illumination Shader
    int plScrX = vpX + (g_player.x * TILE_SIZE - g_camX);
    int plScrY = vpY + (g_player.y * TILE_SIZE - g_camY);

    if (plScrX >= vpX - TILE_SIZE * 3 && plScrX < vpX + VIEWPORT_W + TILE_SIZE * 3 &&
        plScrY >= vpY - TILE_SIZE * 3 && plScrY < vpY + VIEWPORT_H + TILE_SIZE * 3) {
        
        int pFlicker = (int)(sinf((float)g_frameCount * 0.18f) * 4.0f);
        int glowRad = g_player.light_radius * TILE_SIZE / 2 + pFlicker;

        // Outer Ethereal Fringe Halo
        HBRUSH outerHalo = CreateSolidBrush(RGB(12, 28, 45));
        HBRUSH oldOH = (HBRUSH)SelectObject(memDC, outerHalo);
        HPEN outerPen = CreatePen(PS_SOLID, 1, RGB(30, 58, 85));
        HPEN oldOP = (HPEN)SelectObject(memDC, outerPen);
        Ellipse(memDC, plScrX + 16 - glowRad, plScrY + 16 - glowRad, plScrX + 16 + glowRad, plScrY + 16 + glowRad);
        SelectObject(memDC, oldOP);
        DeleteObject(outerPen);
        SelectObject(memDC, oldOH);
        DeleteObject(outerHalo);

        // Inner Warm Golden Torch Core Halo
        int innerRad = glowRad / 2 + 6;
        HBRUSH innerHalo = CreateSolidBrush(RGB(45, 30, 12));
        HBRUSH oldIH = (HBRUSH)SelectObject(memDC, innerHalo);
        HPEN innerPen = CreatePen(PS_SOLID, 1, RGB(180, 83, 9));
        HPEN oldIP = (HPEN)SelectObject(memDC, innerPen);
        Ellipse(memDC, plScrX + 16 - innerRad, plScrY + 16 - innerRad, plScrX + 16 + innerRad, plScrY + 16 + innerRad);
        SelectObject(memDC, oldIP);
        DeleteObject(innerPen);
        SelectObject(memDC, oldIH);
        DeleteObject(innerHalo);

        // Delver Custom Sprite with Cloak, Cowl, Lantern & Blade
        DrawDelverSprite(memDC, plScrX, plScrY, g_player.facing, g_frameCount, g_player.shield > 0);

        if (RandInt(0, 10) < 3) {
            SpawnEmber((float)(plScrX + 16), (float)(plScrY + 14), TRUE);
        }
    }

    // Draw Subterranean Embers Particles
    for (int i = 0; i < g_numEmbers; i++) {
        int ex = (int)g_embers[i].x;
        int ey = (int)g_embers[i].y;
        if (ex >= vpX && ex < vpX + VIEWPORT_W && ey >= vpY && ey < vpY + VIEWPORT_H) {
            SetPixel(memDC, ex, ey, g_embers[i].color);
            SetPixel(memDC, ex + 1, ey, g_embers[i].color);
            SetPixel(memDC, ex, ey + 1, g_embers[i].color);
        }
    }

    // Draw Active Elemental Spell VFX
    for (int i = 0; i < g_numSpellFX; i++) {
        SpellFX* fx = &g_spellFX[i];
        int fxScrX = vpX + (fx->x * TILE_SIZE - g_camX);
        int fxScrY = vpY + (fx->y * TILE_SIZE - g_camY);
        HPEN fxPen = CreatePen(PS_SOLID, 2, fx->color);
        HPEN oldFxP = (HPEN)SelectObject(memDC, fxPen);

        if (fx->type == 2) {
            // Tempest Chain Bolt Line
            int fxScrX2 = vpX + (fx->x2 * TILE_SIZE - g_camX) + 16;
            int fxScrY2 = vpY + (fx->y2 * TILE_SIZE - g_camY) + 16;
            MoveToEx(memDC, fxScrX + 16, fxScrY + 16, NULL);
            LineTo(memDC, fxScrX2, fxScrY2);
        } else {
            // Radial spell wave (Pyre blast, Glacial nova, Void warp)
            HBRUSH oldFxB = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));
            int r = fx->radius + (8 - fx->duration) * 2;
            Ellipse(memDC, fxScrX + 16 - r, fxScrY + 16 - r, fxScrX + 16 + r, fxScrY + 16 + r);
            SelectObject(memDC, oldFxB);
        }

        SelectObject(memDC, oldFxP);
        DeleteObject(fxPen);

        fx->duration--;
        if (fx->duration <= 0) {
            g_spellFX[i] = g_spellFX[g_numSpellFX - 1];
            g_numSpellFX--;
            i--;
        }
    }

    // Draw Floating Combat Texts
    SelectObject(memDC, fontSmall);
    for (int i = 0; i < g_numCombatTexts; i++) {
        CombatText* ct = &g_combatTexts[i];
        int ctScrX = vpX + (int)(ct->x * TILE_SIZE - g_camX) + 8;
        int ctScrY = vpY + (int)(ct->y * TILE_SIZE - g_camY) - (14 - ct->life) * 2;
        if (ctScrX >= vpX && ctScrX < vpX + VIEWPORT_W - 50 && ctScrY >= vpY && ctScrY < vpY + VIEWPORT_H) {
            SetTextColor(memDC, ct->color);
            TextOutA(memDC, ctScrX, ctScrY, ct->text, (int)strlen(ct->text));
        }
        ct->life--;
        ct->y -= 0.05f;
        if (ct->life <= 0) {
            g_combatTexts[i] = g_combatTexts[g_numCombatTexts - 1];
            g_numCombatTexts--;
            i--;
        }
    }

    // CRT Scanlines Shader Pass (Subtle horizontal scan line rasterization)
    if (g_crtEnabled) {
        HPEN crtPen = CreatePen(PS_SOLID, 1, RGB(2, 4, 7));
        HPEN oldCP = (HPEN)SelectObject(memDC, crtPen);
        for (int y = vpY; y < vpY + VIEWPORT_H; y += 3) {
            MoveToEx(memDC, vpX, y, NULL);
            LineTo(memDC, vpX + VIEWPORT_W, y);
        }
        SelectObject(memDC, oldCP);
        DeleteObject(crtPen);
    }

    // Viewport Border
    HPEN vpPen = CreatePen(PS_SOLID, 1, COLOR_BORDER);
    HPEN oldVpP = (HPEN)SelectObject(memDC, vpPen);
    SelectObject(memDC, GetStockObject(NULL_BRUSH));
    Rectangle(memDC, vpX - 1, vpY - 1, vpX + VIEWPORT_W + 1, vpY + VIEWPORT_H + 1);
    SelectObject(memDC, oldVpP);
    DeleteObject(vpPen);

    // Overlay HUD inside Viewport (Top Left)
    RECT hudBox = {vpX + 10, vpY + 10, vpX + 260, vpY + 54};
    HBRUSH hudBg = CreateSolidBrush(RGB(9, 12, 20));
    FillRect(memDC, &hudBox, hudBg);
    DeleteObject(hudBg);
    FrameRect(memDC, &hudBox, (HBRUSH)GetStockObject(GRAY_BRUSH));

    SelectObject(memDC, fontSmall);
    DepthZone curZ = GetDepthZone(g_depthLevel);
    const ZoneTheme* ztHud = &g_zoneThemes[curZ];
    SetTextColor(memDC, ztHud->torchColor);
    char zoneText[64];
    snprintf(zoneText, sizeof(zoneText), "%s - B%d", ztHud->name, g_depthLevel);
    TextOutA(memDC, vpX + 18, vpY + 15, zoneText, (int)strlen(zoneText));

    SetTextColor(memDC, COLOR_TEXT_DIM);
    char turnText[80];
    int fuelPct = (g_player.maxTorchFuel > 0) ? (g_player.torchFuel * 100 / g_player.maxTorchFuel) : 0;
    snprintf(turnText, sizeof(turnText), "Turn: %d | Torch: %s (%d%%) | Food: %d",
             g_turn, g_player.torchLit ? "LIT" : "UNLIT [T]", fuelPct, g_player.hunger);
    TextOutA(memDC, vpX + 18, vpY + 33, turnText, (int)strlen(turnText));

    // Boss HUD Bar Overlay (Top Center of Viewport)
    int bossIdx = -1;
    for (int m = 0; m < g_numMonsters; m++) {
        if (g_monsters[m].alive && g_monsters[m].isBoss) {
            bossIdx = m;
            break;
        }
    }
    if (bossIdx >= 0) {
        const Monster* bm = &g_monsters[bossIdx];
        const MonsterDef* bDef = &g_monsterDefs[bm->type];
        int bBarW = 340;
        int bBarH = 40;
        int bBarX = vpX + (VIEWPORT_W - bBarW) / 2;
        int bBarY = vpY + 10;
        RECT bBarBox = {bBarX, bBarY, bBarX + bBarW, bBarY + bBarH};
        HBRUSH bBg = CreateSolidBrush(RGB(18, 12, 28));
        FillRect(memDC, &bBarBox, bBg);
        DeleteObject(bBg);

        HPEN bPen = CreatePen(PS_SOLID, 2, bDef->color);
        HPEN oldBP = (HPEN)SelectObject(memDC, bPen);
        SelectObject(memDC, GetStockObject(NULL_BRUSH));
        Rectangle(memDC, bBarBox.left, bBarBox.top, bBarBox.right, bBarBox.bottom);
        SelectObject(memDC, oldBP);
        DeleteObject(bPen);

        SelectObject(memDC, fontBold);
        SetTextColor(memDC, bDef->color);
        char bTitle[80];
        snprintf(bTitle, sizeof(bTitle), "%s (B%d LORD)", bDef->name, g_depthLevel);
        TextOutA(memDC, bBarX + 12, bBarY + 4, bTitle, (int)strlen(bTitle));

        // Boss Health Bar Track
        int trackX = bBarX + 12;
        int trackY = bBarY + 22;
        int trackW = bBarW - 24;
        int trackH = 10;
        RECT trackBg = {trackX, trackY, trackX + trackW, trackY + trackH};
        HBRUSH trkBgBr = CreateSolidBrush(RGB(35, 15, 25));
        FillRect(memDC, &trackBg, trkBgBr);
        DeleteObject(trkBgBr);

        int maxHp = bDef->baseHp + (g_depthLevel - 1) * 20;
        int curHp = bm->hp;
        int fillW = (maxHp > 0) ? (curHp * trackW / maxHp) : 0;
        if (fillW < 0) fillW = 0;
        if (fillW > trackW) fillW = trackW;
        if (fillW > 0) {
            RECT fillR = {trackX, trackY, trackX + fillW, trackY + trackH};
            HBRUSH fillBr = CreateSolidBrush(bDef->color);
            FillRect(memDC, &fillR, fillBr);
            DeleteObject(fillBr);
        }

        SelectObject(memDC, fontSmall);
        char hpNum[32];
        snprintf(hpNum, sizeof(hpNum), "%d / %d HP", curHp, maxHp);
        SetTextColor(memDC, RGB(255, 255, 255));
        TextOutA(memDC, bBarX + bBarW - 85, bBarY + 5, hpNum, (int)strlen(hpNum));
    }

    // Spell Hotbar Overlay inside Viewport (Bottom Center/Left)
    {
        int hbY = vpY + VIEWPORT_H - 34;
        int maxS = g_staffDefs[g_player.equippedStaff].maxSockets;
        const char* keyLabels[3] = {"[Z]", "[X]", "[V]"};
        for (int s = 0; s < maxS; s++) {
            int slotX = vpX + 10 + s * 160;
            RECT slotRect = {slotX, hbY, slotX + 152, hbY + 26};
            int runeIdx = g_player.staffSockets[s];

            HBRUSH slotBg = CreateSolidBrush(runeIdx >= 0 ? RGB(16, 22, 38) : RGB(10, 14, 22));
            FillRect(memDC, &slotRect, slotBg);
            DeleteObject(slotBg);

            COLORREF bCol = (runeIdx >= 0) ? g_runeDefs[runeIdx].color : RGB(45, 55, 75);
            HPEN slotPen = CreatePen(PS_SOLID, 1, bCol);
            HPEN oldSlP = (HPEN)SelectObject(memDC, slotPen);
            SelectObject(memDC, GetStockObject(NULL_BRUSH));
            Rectangle(memDC, slotRect.left, slotRect.top, slotRect.right, slotRect.bottom);
            SelectObject(memDC, oldSlP);
            DeleteObject(slotPen);

            SelectObject(memDC, fontSmall);
            if (runeIdx >= 0) {
                const RuneDef* rd = &g_runeDefs[runeIdx];
                int c = rd->cost;
                if (g_player.equippedStaff == 2 && c > 4) c -= 2;

                char sBuf[64];
                snprintf(sBuf, sizeof(sBuf), "%s %s (%d)", keyLabels[s], rd->symbol, c);
                SetTextColor(memDC, rd->color);
                TextOutA(memDC, slotX + 6, hbY + 6, sBuf, (int)strlen(sBuf));
            } else {
                char sBuf[64];
                snprintf(sBuf, sizeof(sBuf), "%s [Empty]", keyLabels[s]);
                SetTextColor(memDC, COLOR_TEXT_DIM);
                TextOutA(memDC, slotX + 6, hbY + 6, sBuf, (int)strlen(sBuf));
            }
        }
    }

    // 3. BOTTOM VIEWPORT TOOLBAR (12..716, 574..606)
    RECT tbRect = {vpX, 574, vpX + VIEWPORT_W, 608};
    FillRect(memDC, &tbRect, panelDarkBrush);

    SelectObject(memDC, fontSmall);
    SetTextColor(memDC, COLOR_TEXT_BRIGHT);
    TextOutA(memDC, vpX + 4, 584, "[N] New", 7);
    TextOutA(memDC, vpX + 70, 584, "[Space] Rest", 12);
    TextOutA(memDC, vpX + 164, 584, "[R] Search", 10);
    TextOutA(memDC, vpX + 248, 584, "[E] Descend", 11);
    TextOutA(memDC, vpX + 338, 584, g_player.torchLit ? "[T] Rekindle" : "[T] LIGHT", g_player.torchLit ? 12 : 9);
    SetTextColor(memDC, COLOR_TEXT_GOLD);
    TextOutA(memDC, vpX + 434, 584, "[I] Enchant", 11);
    SetTextColor(memDC, COLOR_TEXT_BRIGHT);
    TextOutA(memDC, vpX + 524, 584, g_crtEnabled ? "[C] CRT" : "[C] CRT", 7);
    TextOutA(memDC, vpX + 586, 584, g_fovEnabled ? "[F] FOV" : "[F] FOV", 7);
    TextOutA(memDC, vpX + 648, 584, "[H] Tome", 8);

    // 4. RIGHT SIDEBAR (728..1036, 46..608)
    int sbX = 728;
    int sbY = 46;
    int sbW = 308;
    int sbH = 562;

    RECT sbRect = {sbX, sbY, sbX + sbW, sbY + sbH};
    HBRUSH sbBg = CreateSolidBrush(COLOR_BG_PANEL);
    FillRect(memDC, &sbRect, sbBg);
    DeleteObject(sbBg);
    FrameRect(memDC, &sbRect, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

    // Tab Header
    RECT tabHeader = {sbX, sbY, sbX + sbW, sbY + 28};
    FillRect(memDC, &tabHeader, panelDarkBrush);

    SelectObject(memDC, fontSmall);
    // Tab 0: Delver
    SetTextColor(memDC, g_activeTab == 0 ? COLOR_BORDER_GLOW : COLOR_TEXT_DIM);
    TextOutA(memDC, sbX + 8, sbY + 7, "[1] DELV", 8);
    // Tab 1: Relics
    SetTextColor(memDC, g_activeTab == 1 ? COLOR_BORDER_GLOW : COLOR_TEXT_DIM);
    TextOutA(memDC, sbX + 82, sbY + 7, "[2] PACK", 8);
    // Tab 2: Runes
    SetTextColor(memDC, g_activeTab == 2 ? COLOR_BORDER_GLOW : COLOR_TEXT_DIM);
    TextOutA(memDC, sbX + 156, sbY + 7, "[3] RUNES", 9);
    // Tab 3: Bestiary
    SetTextColor(memDC, g_activeTab == 3 ? COLOR_BORDER_GLOW : COLOR_TEXT_DIM);
    TextOutA(memDC, sbX + 234, sbY + 7, "[4] BEASTS", 10);

    int contentY = sbY + 36;

    if (g_activeTab == 0) {
        // DELVER ATTRIBUTES CARD
        RECT cardRect = {sbX + 8, contentY, sbX + sbW - 8, contentY + 230};
        HBRUSH cardBg = CreateSolidBrush(COLOR_BG_CARD);
        FillRect(memDC, &cardRect, cardBg);
        DeleteObject(cardBg);
        FrameRect(memDC, &cardRect, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

        SelectObject(memDC, fontBold);
        SetTextColor(memDC, COLOR_TEXT_RUNE);
        TextOutA(memDC, sbX + 16, contentY + 8, "DELVER ATTRIBUTES", 17);
        char lvlBuf[32];
        snprintf(lvlBuf, sizeof(lvlBuf), "Lvl %d", g_player.level);
        SetTextColor(memDC, COLOR_ACCENT_PURPLE);
        TextOutA(memDC, sbX + sbW - 60, contentY + 8, lvlBuf, (int)strlen(lvlBuf));

        // HP Bar
        SelectObject(memDC, fontSmall);
        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 32, "Health", 6);
        char hpTxt[32];
        snprintf(hpTxt, sizeof(hpTxt), "%d / %d", g_player.hp, g_player.max_hp);
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + sbW - 85, contentY + 32, hpTxt, (int)strlen(hpTxt));

        RECT hpBarBg = {sbX + 16, contentY + 48, sbX + sbW - 16, contentY + 54};
        HBRUSH barDark = CreateSolidBrush(RGB(10, 14, 23));
        FillRect(memDC, &hpBarBg, barDark);
        int hpW = (int)((float)(sbW - 32) * ((float)g_player.hp / (float)g_player.max_hp));
        if (hpW < 0) hpW = 0; if (hpW > sbW - 32) hpW = sbW - 32;
        RECT hpBarFill = {sbX + 16, contentY + 48, sbX + 16 + hpW, contentY + 54};
        HBRUSH hpFill = CreateSolidBrush(COLOR_ACCENT_RED);
        FillRect(memDC, &hpBarFill, hpFill);
        DeleteObject(hpFill);

        // Sanity Bar
        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 60, "Sanity (Willpower)", 18);
        char sanTxt[32];
        snprintf(sanTxt, sizeof(sanTxt), "%d / %d", g_player.sanity, g_player.max_sanity);
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + sbW - 85, contentY + 60, sanTxt, (int)strlen(sanTxt));

        RECT sanBarBg = {sbX + 16, contentY + 74, sbX + sbW - 16, contentY + 79};
        FillRect(memDC, &sanBarBg, barDark);
        int sanW = (int)((float)(sbW - 32) * ((float)g_player.sanity / (float)g_player.max_sanity));
        if (sanW < 0) sanW = 0; if (sanW > sbW - 32) sanW = sbW - 32;
        RECT sanBarFill = {sbX + 16, contentY + 74, sbX + 16 + sanW, contentY + 79};
        HBRUSH sanFill = CreateSolidBrush(COLOR_BORDER_GLOW);
        FillRect(memDC, &sanBarFill, sanFill);
        DeleteObject(sanFill);

        // Aether (Mana) Bar
        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 84, "Aether (Mana)", 13);
        char mpTxt[32];
        if (g_player.shield > 0) {
            snprintf(mpTxt, sizeof(mpTxt), "%d / %d [+%d]", g_player.aether, g_player.max_aether, g_player.shield);
        } else {
            snprintf(mpTxt, sizeof(mpTxt), "%d / %d", g_player.aether, g_player.max_aether);
        }
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + sbW - 85, contentY + 84, mpTxt, (int)strlen(mpTxt));

        RECT mpBarBg = {sbX + 16, contentY + 98, sbX + sbW - 16, contentY + 103};
        FillRect(memDC, &mpBarBg, barDark);
        int mpW = (int)((float)(sbW - 32) * ((float)g_player.aether / (float)g_player.max_aether));
        if (mpW < 0) mpW = 0; if (mpW > sbW - 32) mpW = sbW - 32;
        RECT mpBarFill = {sbX + 16, contentY + 98, sbX + 16 + mpW, contentY + 103};
        HBRUSH mpFill = CreateSolidBrush(RGB(168, 85, 247));
        FillRect(memDC, &mpBarFill, mpFill);
        DeleteObject(mpFill);

        // Hunger Bar
        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 108, "Hunger (Nourish)", 16);
        char hngTxt[32];
        snprintf(hngTxt, sizeof(hngTxt), "%d / %d", g_player.hunger, g_player.max_hunger);
        SetTextColor(memDC, g_player.hunger <= 15 ? RGB(239, 68, 68) : COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + sbW - 85, contentY + 108, hngTxt, (int)strlen(hngTxt));

        RECT hngBarBg = {sbX + 16, contentY + 122, sbX + sbW - 16, contentY + 127};
        FillRect(memDC, &hngBarBg, barDark);
        int hngW = (int)((float)(sbW - 32) * ((float)g_player.hunger / (float)g_player.max_hunger));
        if (hngW < 0) hngW = 0; if (hngW > sbW - 32) hngW = sbW - 32;
        RECT hngBarFill = {sbX + 16, contentY + 122, sbX + 16 + hngW, contentY + 127};
        HBRUSH hngFill = CreateSolidBrush(g_player.hunger <= 15 ? RGB(220, 38, 38) : RGB(217, 119, 6));
        FillRect(memDC, &hngBarFill, hngFill);
        DeleteObject(hngFill);

        // EXP Bar
        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 132, "Experience", 10);
        char expTxt[32];
        snprintf(expTxt, sizeof(expTxt), "%d / %d", g_player.exp, g_player.max_exp);
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + sbW - 85, contentY + 132, expTxt, (int)strlen(expTxt));

        RECT expBarBg = {sbX + 16, contentY + 144, sbX + sbW - 16, contentY + 149};
        FillRect(memDC, &expBarBg, barDark);
        DeleteObject(barDark);
        int expW = (int)((float)(sbW - 32) * ((float)g_player.exp / (float)g_player.max_exp));
        if (expW < 0) expW = 0; if (expW > sbW - 32) expW = sbW - 32;
        RECT expBarFill = {sbX + 16, contentY + 144, sbX + 16 + expW, contentY + 149};
        HBRUSH expFill = CreateSolidBrush(COLOR_TEXT_GOLD);
        FillRect(memDC, &expBarFill, expFill);
        DeleteObject(expFill);

        // Stats rows
        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 154, "Class:", 6);
        SetTextColor(memDC, COLOR_ACCENT_PURPLE);
        TextOutA(memDC, sbX + 130, contentY + 154, "Rune Knight", 11);

        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 168, "Might (Atk):", 12);
        char stBuf[32];
        snprintf(stBuf, sizeof(stBuf), "%d (+%d)", g_player.might, (g_player.might - 10) / 2);
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + 130, contentY + 168, stBuf, (int)strlen(stBuf));

        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 182, "Warding (Def):", 14);
        snprintf(stBuf, sizeof(stBuf), "%d (+%d)", g_player.warding, (g_player.warding - 10) / 2);
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + 130, contentY + 182, stBuf, (int)strlen(stBuf));

        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 196, "Arcana (Magic):", 15);
        int totalArc = g_player.arcana + g_staffDefs[g_player.equippedStaff].arcanaBonus;
        snprintf(stBuf, sizeof(stBuf), "%d (+%d)", totalArc, (totalArc - 10) / 2);
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + 130, contentY + 196, stBuf, (int)strlen(stBuf));

        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 210, "Torch & Light:", 14);
        char torchStatBuf[48];
        if (g_player.torchLit) {
            snprintf(torchStatBuf, sizeof(torchStatBuf), "Lit (%dt) | Rad %d", g_player.torchFuel, g_player.light_radius);
            SetTextColor(memDC, COLOR_TEXT_GOLD);
        } else {
            snprintf(torchStatBuf, sizeof(torchStatBuf), "UNLIT [T:Rekindle] (Rad 1)");
            SetTextColor(memDC, RGB(239, 68, 68));
        }
        TextOutA(memDC, sbX + 130, contentY + 210, torchStatBuf, (int)strlen(torchStatBuf));

        // Occult Curse Status row (if afflicted)
        if (g_player.curse != CURSE_NONE) {
            SetTextColor(memDC, RGB(239, 68, 68));
            TextOutA(memDC, sbX + 16, contentY + 224, "Active Curse:", 13);
            const char* cName = "Darkness";
            if (g_player.curse == CURSE_ENFEEBLE) cName = "Enfeeblement";
            else if (g_player.curse == CURSE_DECAY) cName = "Decaying Hunger";
            else if (g_player.curse == CURSE_VOID) cName = "Void Delirium";
            char curBuf[48];
            snprintf(curBuf, sizeof(curBuf), "%s (%dt left)", cName, g_player.curseTurns);
            TextOutA(memDC, sbX + 130, contentY + 224, curBuf, (int)strlen(curBuf));
        }

        // GEAR SECTION
        int gearY = contentY + 242;
        RECT gearCard = {sbX + 8, gearY, sbX + sbW - 8, gearY + 115};
        HBRUSH gCardBg = CreateSolidBrush(COLOR_BG_CARD);
        FillRect(memDC, &gearCard, gCardBg);
        DeleteObject(gCardBg);
        FrameRect(memDC, &gearCard, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

        SelectObject(memDC, fontBold);
        SetTextColor(memDC, COLOR_TEXT_RUNE);
        TextOutA(memDC, sbX + 16, gearY + 5, "EQUIPMENT (Click to Unequip)", 28);

        SelectObject(memDC, fontSmall);
        // Staff
        SetTextColor(memDC, RGB(168, 85, 247));
        char staffStr[80];
        snprintf(staffStr, sizeof(staffStr), "[Stf] %s", g_staffDefs[g_player.equippedStaff].name);
        TextOutA(memDC, sbX + 16, gearY + 23, staffStr, (int)strlen(staffStr));

        // Weapon
        COLORREF wpnCol = (g_player.equipWeapon != ITEM_NONE) ? COLOR_ACCENT_AMBER : COLOR_TEXT_DIM;
        const char* encTag = "";
        if (g_player.weaponEnchant == ENCHANT_FIRE) { encTag = " [Flamebrand]"; wpnCol = RGB(249, 115, 22); }
        else if (g_player.weaponEnchant == ENCHANT_FROST) { encTag = " [Frostbite]"; wpnCol = RGB(6, 182, 212); }
        else if (g_player.weaponEnchant == ENCHANT_VOID) { encTag = " [Voidsever]"; wpnCol = RGB(168, 85, 247); }
        SetTextColor(memDC, wpnCol);
        char wpnStr[96];
        snprintf(wpnStr, sizeof(wpnStr), "[Wpn] %s%s", g_player.equipWeapon != ITEM_NONE ? g_itemDefs[g_player.equipWeapon].name : "(Empty Weapon Slot)", encTag);
        TextOutA(memDC, sbX + 16, gearY + 41, wpnStr, (int)strlen(wpnStr));

        // Armor
        SetTextColor(memDC, g_player.equipArmor != ITEM_NONE ? COLOR_BORDER_GLOW : COLOR_TEXT_DIM);
        char armStr[80];
        snprintf(armStr, sizeof(armStr), "[Arm] %s", g_player.equipArmor != ITEM_NONE ? g_itemDefs[g_player.equipArmor].name : "(Empty Armor Slot)");
        TextOutA(memDC, sbX + 16, gearY + 59, armStr, (int)strlen(armStr));

        // Relic
        SetTextColor(memDC, g_player.equipRelic != ITEM_NONE ? COLOR_TEXT_GOLD : COLOR_TEXT_DIM);
        char relStr[80];
        snprintf(relStr, sizeof(relStr), "[Rel] %s", g_player.equipRelic != ITEM_NONE ? g_itemDefs[g_player.equipRelic].name : "(Empty Relic Slot)");
        TextOutA(memDC, sbX + 16, gearY + 77, relStr, (int)strlen(relStr));

        // Amulet
        SetTextColor(memDC, g_player.equipAmulet != ITEM_NONE ? COLOR_ACCENT_PURPLE : COLOR_TEXT_DIM);
        char amuStr[80];
        snprintf(amuStr, sizeof(amuStr), "[Amu] %s", g_player.equipAmulet != ITEM_NONE ? g_itemDefs[g_player.equipAmulet].name : "(Empty Amulet Slot)");
        TextOutA(memDC, sbX + 16, gearY + 95, amuStr, (int)strlen(amuStr));

    } else if (g_activeTab == 1) {
        // TOP CARD: DELVER'S PACK INVENTORY
        int packCardH = 230;
        RECT packCard = {sbX + 8, contentY, sbX + sbW - 8, contentY + packCardH};
        HBRUSH cardBg = CreateSolidBrush(COLOR_BG_CARD);
        FillRect(memDC, &packCard, cardBg);
        DeleteObject(cardBg);
        FrameRect(memDC, &packCard, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

        SelectObject(memDC, fontBold);
        SetTextColor(memDC, COLOR_TEXT_RUNE);
        char pHeader[64];
        snprintf(pHeader, sizeof(pHeader), "DELVER'S PACK (%d/%d) - [Click: Use/Equip]", g_player.numPackItems, MAX_PACK_SLOTS);
        TextOutA(memDC, sbX + 16, contentY + 6, pHeader, (int)strlen(pHeader));

        SelectObject(memDC, fontSmall);
        int itemY = contentY + 25;
        for (int i = 0; i < 8; i++) {
            if (i < g_player.numPackItems) {
                const ItemDef* def = &g_itemDefs[g_player.pack[i].id];
                char rowBuf[100];
                if (def->category == ITEM_TYPE_EQUIPMENT) {
                    snprintf(rowBuf, sizeof(rowBuf), "%d. [%s] %s (Equip)", i + 1, def->symbol, def->name);
                    SetTextColor(memDC, COLOR_BORDER_GLOW);
                } else if (def->category == ITEM_TYPE_CONSUMABLE) {
                    snprintf(rowBuf, sizeof(rowBuf), "%d. [%s] %s x%d", i + 1, def->symbol, def->name, g_player.pack[i].count);
                    SetTextColor(memDC, COLOR_ACCENT_GREEN);
                } else if (def->category == ITEM_TYPE_REAGENT) {
                    snprintf(rowBuf, sizeof(rowBuf), "%d. [%s] %s x%d", i + 1, def->symbol, def->name, g_player.pack[i].count);
                    SetTextColor(memDC, COLOR_TEXT_GOLD);
                } else {
                    snprintf(rowBuf, sizeof(rowBuf), "%d. [%s] %s", i + 1, def->symbol, def->name);
                    SetTextColor(memDC, COLOR_TEXT_RUNE);
                }
                TextOutA(memDC, sbX + 16, itemY, rowBuf, (int)strlen(rowBuf));
            } else {
                char emptyBuf[32];
                snprintf(emptyBuf, sizeof(emptyBuf), "%d. [ Empty Slot ]", i + 1);
                SetTextColor(memDC, COLOR_TEXT_DIM);
                TextOutA(memDC, sbX + 16, itemY, emptyBuf, (int)strlen(emptyBuf));
            }
            itemY += 21;
        }

        SetTextColor(memDC, RGB(148, 163, 184));
        TextOutA(memDC, sbX + 16, contentY + packCardH - 18, "Hint: [U] Use Consumable | [Del] Discard Slot 1", 47);

        // BOTTOM CARD: ANCIENT ALCHEMY CAULDRON
        int caulY = contentY + packCardH + 8;
        int caulH = 275;
        RECT caulCard = {sbX + 8, caulY, sbX + sbW - 8, caulY + caulH};
        HBRUSH cBg = CreateSolidBrush(COLOR_BG_CARD);
        FillRect(memDC, &caulCard, cBg);
        DeleteObject(cBg);
        FrameRect(memDC, &caulCard, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

        SelectObject(memDC, fontBold);
        SetTextColor(memDC, RGB(52, 211, 153));
        TextOutA(memDC, sbX + 16, caulY + 6, "{U} ANCIENT ALCHEMY CAULDRON", 28);

        SelectObject(memDC, fontSmall);
        // Reagents count bar
        char rgtBuf[100];
        snprintf(rgtBuf, sizeof(rgtBuf), "Lotus:%d Spore:%d Ash:%d Dust:%d Blsm:%d",
                 GetPackItemCount(ITEM_ING_BLOOD_LOTUS),
                 GetPackItemCount(ITEM_ING_AZURE_SPORES),
                 GetPackItemCount(ITEM_ING_BRIMSTONE),
                 GetPackItemCount(ITEM_ING_VOID_DUST),
                 GetPackItemCount(ITEM_ING_AETHER_BLOSSOM));
        SetTextColor(memDC, COLOR_TEXT_GOLD);
        TextOutA(memDC, sbX + 16, caulY + 24, rgtBuf, (int)strlen(rgtBuf));

        // 6 Recipes list
        int recY = caulY + 42;
        for (int r = 0; r < NUM_RECIPES; r++) {
            const AlchemyRecipe* rec = &g_recipes[r];
            BOOL canBrew = TRUE;
            for (int k = 0; k < rec->numIng; k++) {
                if (GetPackItemCount(rec->ing[k].id) < rec->ing[k].count) {
                    canBrew = FALSE;
                    break;
                }
            }

            RECT rBtn = {sbX + 14, recY, sbX + sbW - 14, recY + 34};
            HBRUSH rBr = CreateSolidBrush(canBrew ? RGB(6, 40, 30) : RGB(14, 18, 28));
            FillRect(memDC, &rBtn, rBr);
            DeleteObject(rBr);

            HPEN rPen = CreatePen(PS_SOLID, 1, canBrew ? RGB(16, 185, 129) : RGB(40, 50, 68));
            HPEN oldRP = (HPEN)SelectObject(memDC, rPen);
            SelectObject(memDC, GetStockObject(NULL_BRUSH));
            Rectangle(memDC, rBtn.left, rBtn.top, rBtn.right, rBtn.bottom);
            SelectObject(memDC, oldRP);
            DeleteObject(rPen);

            char rLine1[80];
            snprintf(rLine1, sizeof(rLine1), "[Brew %d] %s", r + 1, rec->name);
            SetTextColor(memDC, canBrew ? RGB(110, 231, 183) : COLOR_TEXT_DIM);
            TextOutA(memDC, sbX + 20, recY + 3, rLine1, (int)strlen(rLine1));

            char rLine2[80];
            snprintf(rLine2, sizeof(rLine2), "%s", rec->desc);
            SetTextColor(memDC, canBrew ? COLOR_TEXT_BRIGHT : RGB(80, 95, 115));
            TextOutA(memDC, sbX + 20, recY + 17, rLine2, (int)strlen(rLine2));

            recY += 38;
        }
    } else if (g_activeTab == 2) {
        // TAB 2: RUNIC FORGE & SOCKETING STATION
        RECT cardRect = {sbX + 8, contentY, sbX + sbW - 8, contentY + 308};
        HBRUSH cardBg = CreateSolidBrush(COLOR_BG_CARD);
        FillRect(memDC, &cardRect, cardBg);
        DeleteObject(cardBg);
        FrameRect(memDC, &cardRect, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

        SelectObject(memDC, fontBold);
        SetTextColor(memDC, COLOR_TEXT_RUNE);
        TextOutA(memDC, sbX + 16, contentY + 8, "RUNIC FORGE & SOCKETS", 21);

        // Staff Info
        SelectObject(memDC, fontSmall);
        SetTextColor(memDC, RGB(168, 85, 247));
        char stfBuf[80];
        const StaffDef* curStf = &g_staffDefs[g_player.equippedStaff];
        snprintf(stfBuf, sizeof(stfBuf), "%s (%s)", curStf->name, curStf->tier);
        TextOutA(memDC, sbX + 16, contentY + 28, stfBuf, (int)strlen(stfBuf));

        // Render Sockets
        int curY = contentY + 46;
        for (int s = 0; s < curStf->maxSockets; s++) {
            RECT sockRect = {sbX + 16, curY, sbX + sbW - 16, curY + 30};
            int rIdx = g_player.staffSockets[s];

            HBRUSH sBr = CreateSolidBrush(rIdx >= 0 ? RGB(18, 24, 40) : RGB(10, 14, 22));
            FillRect(memDC, &sockRect, sBr);
            DeleteObject(sBr);

            HPEN sPen = CreatePen(PS_SOLID, 1, rIdx >= 0 ? g_runeDefs[rIdx].color : RGB(50, 60, 80));
            HPEN oldSP2 = (HPEN)SelectObject(memDC, sPen);
            SelectObject(memDC, GetStockObject(NULL_BRUSH));
            Rectangle(memDC, sockRect.left, sockRect.top, sockRect.right, sockRect.bottom);
            SelectObject(memDC, oldSP2);
            DeleteObject(sPen);

            char sockLine[80];
            if (rIdx >= 0) {
                const RuneDef* rd = &g_runeDefs[rIdx];
                snprintf(sockLine, sizeof(sockLine), "Socket %d: [%s] %s (%d MP)", s + 1, rd->symbol, rd->spellName, rd->cost);
                SetTextColor(memDC, rd->color);
                TextOutA(memDC, sbX + 22, curY + 4, sockLine, (int)strlen(sockLine));
                SetTextColor(memDC, COLOR_TEXT_DIM);
                char passLine[80];
                snprintf(passLine, sizeof(passLine), "%s | Click to remove", rd->passive);
                TextOutA(memDC, sbX + 22, curY + 16, passLine, (int)strlen(passLine));
            } else {
                snprintf(sockLine, sizeof(sockLine), "Socket %d: [ Empty Slot ]", s + 1);
                SetTextColor(memDC, COLOR_TEXT_DIM);
                TextOutA(memDC, sbX + 22, curY + 4, sockLine, (int)strlen(sockLine));
                TextOutA(memDC, sbX + 22, curY + 16, "Click a rune below to socket", 28);
            }

            curY += 34;
        }

        // Delver's Rune Stash Header
        curY += 4;
        SelectObject(memDC, fontBold);
        SetTextColor(memDC, COLOR_TEXT_GOLD);
        TextOutA(memDC, sbX + 16, curY, "COLLECTED ANCIENT RUNES", 23);
        curY += 20;

        // Render Owned Runes
        SelectObject(memDC, fontSmall);
        for (int r = 0; r < NUM_RUNES; r++) {
            BOOL owned = g_player.ownedRunes[r];
            BOOL socketed = FALSE;
            for (int s = 0; s < curStf->maxSockets; s++) {
                if (g_player.staffSockets[s] == r) { socketed = TRUE; break; }
            }

            RECT runeRect = {sbX + 16, curY, sbX + sbW - 16, curY + 22};
            HBRUSH rBr = CreateSolidBrush(owned ? RGB(14, 20, 32) : RGB(8, 10, 16));
            FillRect(memDC, &runeRect, rBr);
            DeleteObject(rBr);

            const RuneDef* rd = &g_runeDefs[r];
            char rBuf[80];
            if (owned) {
                snprintf(rBuf, sizeof(rBuf), "[%s] %s - %s %s", rd->symbol, rd->name, rd->spellName, socketed ? "(Inscribed)" : "(Click to Socket)");
                SetTextColor(memDC, rd->color);
            } else {
                snprintf(rBuf, sizeof(rBuf), "[?] %s (Undiscovered in Crypts)", rd->name);
                SetTextColor(memDC, RGB(60, 70, 90));
            }
            TextOutA(memDC, sbX + 22, curY + 4, rBuf, (int)strlen(rBuf));
            curY += 24;
        }
    } else if (g_activeTab == 3) {
        // TAB 3: BESTIARY & THREAT RADAR
        RECT cardRect = {sbX + 8, contentY, sbX + sbW - 8, contentY + 308};
        HBRUSH cardBg = CreateSolidBrush(COLOR_BG_CARD);
        FillRect(memDC, &cardRect, cardBg);
        DeleteObject(cardBg);
        FrameRect(memDC, &cardRect, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

        SelectObject(memDC, fontBold);
        SetTextColor(memDC, COLOR_TEXT_RUNE);
        TextOutA(memDC, sbX + 16, contentY + 8, "BESTIARY & THREAT RADAR", 23);

        int livingCount = 0;
        for (int m = 0; m < g_numMonsters; m++) {
            if (g_monsters[m].alive) livingCount++;
        }
        SelectObject(memDC, fontSmall);
        char radBuf[80];
        snprintf(radBuf, sizeof(radBuf), "Hostiles on Floor: %d | Threat: %s", livingCount, livingCount > 6 ? "HIGH" : livingCount > 2 ? "MED" : "LOW");
        SetTextColor(memDC, COLOR_ACCENT_AMBER);
        TextOutA(memDC, sbX + 16, contentY + 28, radBuf, (int)strlen(radBuf));

        int bY = contentY + 46;
        for (int b = 0; b < NUM_MONSTER_TYPES && bY < contentY + 295; b++) {
            const MonsterDef* md = &g_monsterDefs[b];
            BOOL isLord = (b >= 5);
            int rowH = isLord ? 30 : 25;
            RECT bRect = {sbX + 14, bY, sbX + sbW - 14, bY + rowH};
            HBRUSH bBr = CreateSolidBrush(isLord ? RGB(26, 16, 32) : RGB(14, 18, 28));
            FillRect(memDC, &bRect, bBr);
            DeleteObject(bBr);

            SelectObject(memDC, fontBold);
            SetTextColor(memDC, md->color);
            char bName[64];
            snprintf(bName, sizeof(bName), "[%s] %s%s", md->symbol, md->name, isLord ? " (LORD)" : "");
            TextOutA(memDC, sbX + 20, bY + 2, bName, (int)strlen(bName));

            SelectObject(memDC, fontSmall);
            SetTextColor(memDC, isLord ? COLOR_TEXT_GOLD : COLOR_TEXT_DIM);
            char bStats[80];
            snprintf(bStats, sizeof(bStats), "HP:%d Atk:%d Exp:%d", md->baseHp, md->baseAtk, md->exp);
            TextOutA(memDC, sbX + 200, bY + 2, bStats, (int)strlen(bStats));

            SetTextColor(memDC, isLord ? RGB(253, 230, 138) : RGB(148, 163, 184));
            TextOutA(memDC, sbX + 20, bY + 13, md->weakness, (int)strlen(md->weakness));

            bY += rowH + 2;
        }
    }

    // MESSAGE CHRONICLE LOG (sbX + 8, 380..600)
    int logY = sbY + 320;
    RECT logCard = {sbX + 8, logY, sbX + sbW - 8, sbY + sbH - 8};
    FillRect(memDC, &logCard, panelDarkBrush);
    FrameRect(memDC, &logCard, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

    SelectObject(memDC, fontBold);
    SetTextColor(memDC, COLOR_TEXT_RUNE);
    TextOutA(memDC, sbX + 14, logY + 6, "ABYSSAL CHRONICLE", 17);

    SelectObject(memDC, fontSmall);
    int startIdx = (g_logCount > 10) ? (g_logCount - 10) : 0;
    int curLogY = logY + 26;
    for (int i = startIdx; i < g_logCount && curLogY < sbY + sbH - 20; i++) {
        char msgLine[140];
        snprintf(msgLine, sizeof(msgLine), "[T%d] %s", g_logs[i].turn, g_logs[i].text);
        SetTextColor(memDC, g_logs[i].color);
        TextOutA(memDC, sbX + 14, curLogY, msgLine, (int)strlen(msgLine));
        curLogY += 18;
    }

    // 5. HELP MODAL DIALOG (When H or F1 pressed)
    if (g_showHelpModal) {
        RECT modalRect = {width / 2 - 310, height / 2 - 210, width / 2 + 310, height / 2 + 210};
        HBRUSH modalBg = CreateSolidBrush(COLOR_BG_PANEL);
        FillRect(memDC, &modalRect, modalBg);
        DeleteObject(modalBg);

        HPEN glowModalPen = CreatePen(PS_SOLID, 2, COLOR_BORDER_GLOW);
        HPEN oldMP = (HPEN)SelectObject(memDC, glowModalPen);
        SelectObject(memDC, GetStockObject(NULL_BRUSH));
        Rectangle(memDC, modalRect.left, modalRect.top, modalRect.right, modalRect.bottom);
        SelectObject(memDC, oldMP);
        DeleteObject(glowModalPen);

        SelectObject(memDC, fontTitle);
        SetTextColor(memDC, COLOR_BORDER_GLOW);
        TextOutA(memDC, modalRect.left + 20, modalRect.top + 14, "DELVER'S TOME & SURVIVAL MANUAL", 31);

        SelectObject(memDC, fontSmall);
        SetTextColor(memDC, COLOR_TEXT_PRIMARY);
        int my = modalRect.top + 40;
        TextOutA(memDC, modalRect.left + 20, my, "- WASD / Arrows / Vi / Numpad: Navigate subterranean grid", 57); my += 16;
        TextOutA(memDC, modalRect.left + 20, my, "- Space / Num5: Rest 1 turn (+2 HP, +1 Sanity, +3 Aether, disallow when starving)", 82); my += 16;
        TextOutA(memDC, modalRect.left + 20, my, "- Bump Combat: Walk into monsters to strike in melee", 52); my += 16;
        TextOutA(memDC, modalRect.left + 20, my, "- Z / X / V: Cast Elemental Spells from Staff Sockets 1 / 2 / 3", 63); my += 16;
        TextOutA(memDC, modalRect.left + 20, my, "- R / Search: Search surrounding area for secret coffers & altars", 65); my += 16;
        TextOutA(memDC, modalRect.left + 20, my, "- E / Enter: Interact / Descend stairs / Commune with Altars", 60); my += 16;
        TextOutA(memDC, modalRect.left + 20, my, "- T / Rekindle: Re-ignite torch via Pyre spell, Brimstone Ash, or adjacent fire", 79); my += 16;
        TextOutA(memDC, modalRect.left + 20, my, "- Hunger & Darkness: Starving (0 food) drains HP. Pitch blackness drains Sanity!", 80); my += 16;
        TextOutA(memDC, modalRect.left + 20, my, "- Cursed Effigies [!]: Walk into totems to shatter them for essence (beware curses)", 84); my += 16;
        TextOutA(memDC, modalRect.left + 20, my, "- Curses & Salt: Purifying Salt / Altars purge curses (Darkness, Enfeeble, Decay)", 81); my += 16;
        TextOutA(memDC, modalRect.left + 20, my, "- 1, 2, 3, 4: Sidebar Tabs (Delver stats / Pack & Alchemy / Rune Forge / Bestiary)", 82); my += 16;
        TextOutA(memDC, modalRect.left + 20, my, "- I / Altar: Open Relic Enchanting Altar to imbue Flamebrand / Frostbite / Voidsever", 84); my += 16;
        TextOutA(memDC, modalRect.left + 20, my, "- Ancient Shrines: Touch glowing runic monoliths for divine blessings & ancient runes", 85); my += 16;
        TextOutA(memDC, modalRect.left + 20, my, "- M / Merchant: Trade Gold for relics, survival goods & sell pack items [Phase 11]", 83); my += 16;
        TextOutA(memDC, modalRect.left + 20, my, "- Biomes: B1-3 Catacombs | B4-6 Sunken Grotto | B7-9 Crypt | B10+ Void", 70); my += 16;
        TextOutA(memDC, modalRect.left + 20, my, "- Abyssal Lords: B3 Crypt Keeper | B6 Abyssal Wyrm | B10+ Void Monarch", 70); my += 16;
        TextOutA(memDC, modalRect.left + 20, my, "- C: CRT Phosphors | F: Field of View | Ctrl+N / F2: New Descent", 64); my += 18;

        SetTextColor(memDC, COLOR_TEXT_GOLD);
        TextOutA(memDC, modalRect.left + 20, my, "Press [H], [F1], or [ESC] to close manual.", 42);
    }

    // 5b. ENCHANTING ALTAR MODAL DIALOG (When I pressed or Altar visited)
    if (g_showEnchantModal) {
        RECT modalRect = {width / 2 - 320, height / 2 - 220, width / 2 + 320, height / 2 + 220};
        HBRUSH modalBg = CreateSolidBrush(RGB(10, 15, 26));
        FillRect(memDC, &modalRect, modalBg);
        DeleteObject(modalBg);

        HPEN glowModalPen = CreatePen(PS_SOLID, 2, RGB(251, 191, 36));
        HPEN oldMP = (HPEN)SelectObject(memDC, glowModalPen);
        SelectObject(memDC, GetStockObject(NULL_BRUSH));
        Rectangle(memDC, modalRect.left, modalRect.top, modalRect.right, modalRect.bottom);
        SelectObject(memDC, oldMP);
        DeleteObject(glowModalPen);

        SelectObject(memDC, fontTitle);
        SetTextColor(memDC, COLOR_TEXT_GOLD);
        TextOutA(memDC, modalRect.left + 20, modalRect.top + 14, "ANCIENT RELIC ENCHANTING ALTAR", 30);

        SelectObject(memDC, fontSmall);
        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, modalRect.left + 20, modalRect.top + 36, "Imbue equipped weapon with elemental runic enchantments & invoke benedictions", 77);

        int my = modalRect.top + 58;

        // Current Weapon & Active Enchantment status card
        RECT wCard = {modalRect.left + 20, my, modalRect.right - 20, my + 44};
        HBRUSH wCardBg = CreateSolidBrush(RGB(16, 23, 38));
        FillRect(memDC, &wCard, wCardBg);
        DeleteObject(wCardBg);
        FrameRect(memDC, &wCard, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

        SelectObject(memDC, fontBold);
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        char wpnInfo[128];
        const char* encTitle = "Unenchanted";
        COLORREF encCol = COLOR_TEXT_DIM;
        if (g_player.weaponEnchant == ENCHANT_FIRE) { encTitle = "Flamebrand (Fire I)"; encCol = RGB(249, 115, 22); }
        else if (g_player.weaponEnchant == ENCHANT_FROST) { encTitle = "Frostbite (Cryo I)"; encCol = RGB(6, 182, 212); }
        else if (g_player.weaponEnchant == ENCHANT_VOID) { encTitle = "Voidsever (Void I)"; encCol = RGB(168, 85, 247); }

        snprintf(wpnInfo, sizeof(wpnInfo), "Equipped Weapon: %s", (g_player.equipWeapon != ITEM_NONE) ? g_itemDefs[g_player.equipWeapon].name : "None (Equip a weapon first!)");
        TextOutA(memDC, modalRect.left + 28, my + 6, wpnInfo, (int)strlen(wpnInfo));

        SelectObject(memDC, fontSmall);
        SetTextColor(memDC, encCol);
        char encInfo[128];
        snprintf(encInfo, sizeof(encInfo), "Current Imbuement: %s", encTitle);
        TextOutA(memDC, modalRect.left + 28, my + 24, encInfo, (int)strlen(encInfo));

        // Reagents Stash line
        char stashInfo[128];
        snprintf(stashInfo, sizeof(stashInfo), "Essence: %d* | Brimstone: %d | Azure: %d | Void Dust: %d",
                 g_player.essence, GetPackItemCount(ITEM_ING_BRIMSTONE), GetPackItemCount(ITEM_ING_AZURE_SPORES), GetPackItemCount(ITEM_ING_VOID_DUST));
        SetTextColor(memDC, COLOR_TEXT_GOLD);
        TextOutA(memDC, modalRect.left + 280, my + 24, stashInfo, (int)strlen(stashInfo));

        my += 54;

        // Option 1: Flamebrand
        RECT opt1 = {modalRect.left + 20, my, modalRect.right - 20, my + 44};
        HBRUSH o1Bg = CreateSolidBrush(g_player.weaponEnchant == ENCHANT_FIRE ? RGB(35, 20, 10) : RGB(18, 22, 34));
        FillRect(memDC, &opt1, o1Bg);
        DeleteObject(o1Bg);
        FrameRect(memDC, &opt1, (HBRUSH)GetStockObject(DKGRAY_BRUSH));
        SelectObject(memDC, fontBold);
        SetTextColor(memDC, RGB(249, 115, 22));
        TextOutA(memDC, modalRect.left + 28, my + 6, "[1] FLAMEBRAND (Fire) - Cost: 25 Essence OR 1 Brimstone Ash", 59);
        SelectObject(memDC, fontSmall);
        SetTextColor(memDC, COLOR_TEXT_PRIMARY);
        TextOutA(memDC, modalRect.left + 28, my + 24, "Effect: +10..16 Fire DMG (1.5x vs Undead/Skeletons). Passive: +2 Might.", 71);
        my += 50;

        // Option 2: Frostbite
        RECT opt2 = {modalRect.left + 20, my, modalRect.right - 20, my + 44};
        HBRUSH o2Bg = CreateSolidBrush(g_player.weaponEnchant == ENCHANT_FROST ? RGB(10, 25, 35) : RGB(18, 22, 34));
        FillRect(memDC, &opt2, o2Bg);
        DeleteObject(o2Bg);
        FrameRect(memDC, &opt2, (HBRUSH)GetStockObject(DKGRAY_BRUSH));
        SelectObject(memDC, fontBold);
        SetTextColor(memDC, RGB(6, 182, 212));
        TextOutA(memDC, modalRect.left + 28, my + 6, "[2] FROSTBITE (Cryo) - Cost: 25 Essence OR 1 Azure Spores", 57);
        SelectObject(memDC, fontSmall);
        SetTextColor(memDC, COLOR_TEXT_PRIMARY);
        TextOutA(memDC, modalRect.left + 28, my + 24, "Effect: +8..14 Cryo DMG, 35% chance to Freeze target 2 turns. Passive: +3 Warding.", 82);
        my += 50;

        // Option 3: Voidsever
        RECT opt3 = {modalRect.left + 20, my, modalRect.right - 20, my + 44};
        HBRUSH o3Bg = CreateSolidBrush(g_player.weaponEnchant == ENCHANT_VOID ? RGB(28, 15, 38) : RGB(18, 22, 34));
        FillRect(memDC, &opt3, o3Bg);
        DeleteObject(o3Bg);
        FrameRect(memDC, &opt3, (HBRUSH)GetStockObject(DKGRAY_BRUSH));
        SelectObject(memDC, fontBold);
        SetTextColor(memDC, RGB(168, 85, 247));
        TextOutA(memDC, modalRect.left + 28, my + 6, "[3] VOIDSEVER (Void) - Cost: 30 Essence OR 1 Void Dust", 54);
        SelectObject(memDC, fontSmall);
        SetTextColor(memDC, COLOR_TEXT_PRIMARY);
        TextOutA(memDC, modalRect.left + 28, my + 24, "Effect: +12..20 Void DMG (pierces def), siphons +4 MP & +3 Sanity. Passive: +2 Arcana.", 86);
        my += 50;

        // Option 4: Altar Benediction & Cleanse
        RECT opt4 = {modalRect.left + 20, my, modalRect.right - 20, my + 44};
        HBRUSH o4Bg = CreateSolidBrush(RGB(18, 22, 34));
        FillRect(memDC, &opt4, o4Bg);
        DeleteObject(o4Bg);
        FrameRect(memDC, &opt4, (HBRUSH)GetStockObject(DKGRAY_BRUSH));
        SelectObject(memDC, fontBold);
        SetTextColor(memDC, COLOR_TEXT_GOLD);
        TextOutA(memDC, modalRect.left + 28, my + 6, "[4] ALTAR BENEDICTION (Free) - Consecrated Renewal", 50);
        SelectObject(memDC, fontSmall);
        SetTextColor(memDC, COLOR_TEXT_PRIMARY);
        TextOutA(memDC, modalRect.left + 28, my + 24, "Purges all active curses, restores +30 HP, +25 Sanity, and +20 Aether.", 70);
        my += 52;

        // Controls bar
        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, modalRect.left + 20, my, "[0/C] Disenchant Weapon  |  [1..4] Select Action  |  [ESC / I] Return to Dungeon", 79);
    }

    // 5c. SUBTERRANEAN BLACK MARKET MODAL DIALOG (When M pressed or Merchant visited)
    if (g_showMerchantModal) {
        RECT modalRect = {width / 2 - 340, height / 2 - 240, width / 2 + 340, height / 2 + 240};
        HBRUSH modalBg = CreateSolidBrush(RGB(10, 14, 24));
        FillRect(memDC, &modalRect, modalBg);
        DeleteObject(modalBg);

        HPEN glowModalPen = CreatePen(PS_SOLID, 2, RGB(251, 191, 36));
        HPEN oldMP = (HPEN)SelectObject(memDC, glowModalPen);
        SelectObject(memDC, GetStockObject(NULL_BRUSH));
        Rectangle(memDC, modalRect.left, modalRect.top, modalRect.right, modalRect.bottom);
        SelectObject(memDC, oldMP);
        DeleteObject(glowModalPen);

        BOOL isDeep = (g_depthLevel > 6);
        const char* title = isDeep ? "MALAKOR'S BLACK MARKET SANCTUARY" : "SUBTERRANEAN BLACK MARKET";
        const char* sub = isDeep ? "Malakor the Blind Hermit | Deep Crypt & Void Outcast" : "Grimhollow the Abyssal Broker | Catacombs Outpost";

        SelectObject(memDC, fontTitle);
        SetTextColor(memDC, COLOR_TEXT_GOLD);
        TextOutA(memDC, modalRect.left + 20, modalRect.top + 12, title, (int)strlen(title));

        SelectObject(memDC, fontSmall);
        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, modalRect.left + 20, modalRect.top + 34, sub, (int)strlen(sub));

        // Header bar: Quote & Gold
        RECT qBar = {modalRect.left + 20, modalRect.top + 52, modalRect.right - 20, modalRect.top + 80};
        HBRUSH qBr = CreateSolidBrush(RGB(15, 23, 42));
        FillRect(memDC, &qBar, qBr);
        DeleteObject(qBr);
        FrameRect(memDC, &qBar, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

        SelectObject(memDC, fontSmall);
        SetTextColor(memDC, RGB(203, 213, 225));
        const char* quote = isDeep ? "\"The dark whispers truths the sighted can never see... trade your essence, delver.\"" :
                                    "\"Torches, salves, blade steel... gold shines the same even in the deepest abyss.\"";
        TextOutA(memDC, modalRect.left + 28, modalRect.top + 58, quote, (int)strlen(quote));

        char goldTxt[64];
        snprintf(goldTxt, sizeof(goldTxt), "Gold: %d *", g_player.essence);
        SelectObject(memDC, fontBold);
        SetTextColor(memDC, COLOR_TEXT_GOLD);
        TextOutA(memDC, modalRect.right - 140, modalRect.top + 58, goldTxt, (int)strlen(goldTxt));

        // Tab Mode Buttons (BUY / SELL)
        int tabY = modalRect.top + 86;
        RECT bTabBuy = {modalRect.left + 20, tabY, modalRect.left + 180, tabY + 24};
        RECT bTabSell = {modalRect.left + 190, tabY, modalRect.left + 350, tabY + 24};

        HBRUSH buyBr = CreateSolidBrush(g_merchantMode == 0 ? RGB(69, 26, 3) : RGB(20, 27, 45));
        HBRUSH sellBr = CreateSolidBrush(g_merchantMode == 1 ? RGB(6, 78, 59) : RGB(20, 27, 45));
        FillRect(memDC, &bTabBuy, buyBr);
        FillRect(memDC, &bTabSell, sellBr);
        DeleteObject(buyBr);
        DeleteObject(sellBr);

        FrameRect(memDC, &bTabBuy, (HBRUSH)GetStockObject(DKGRAY_BRUSH));
        FrameRect(memDC, &bTabSell, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

        SelectObject(memDC, fontBold);
        SetTextColor(memDC, g_merchantMode == 0 ? RGB(254, 240, 138) : COLOR_TEXT_DIM);
        TextOutA(memDC, modalRect.left + 34, tabY + 4, "[1..8] BUY WARES", 16);

        SetTextColor(memDC, g_merchantMode == 1 ? RGB(167, 243, 208) : COLOR_TEXT_DIM);
        TextOutA(memDC, modalRect.left + 204, tabY + 4, "[S/TAB] SELL PACK", 17);

        int rowY = tabY + 30;

        if (g_merchantMode == 0) {
            // BUY MODE (8 Wares)
            const char* wareNames[2][8] = {
                { "Iron Rations (Food)", "Torch Pitch & Oil", "Healing Salve", "Runic Longsword", "Shadowweave Cloak", "Aether Lantern", "Floor Cartography", "Malediction Cleanse" },
                { "Panacea of the Deep", "Voidfang Dagger", "Aegis Cuirass", "Radiant Censer", "Void Eye Talisman", "Floor Cartography", "Primordial Rune Siphon", "Void Mystery Relic" }
            };
            const char* wareDescs[2][8] = {
                { "+45 Hunger, +10 HP (Survival)", "Full torch refill (+80 turns) & relight", "+35 HP, +5 Sanity potion", "+5 Might, +1 Ward weapon", "+4 Ward, +2 Arcana armor", "+8 Light Radius, +15 Max Aether", "Reveals all floor rooms & stairs", "Purges all curses, +35 San, +30 HP, +25 MP" },
                { "+60 HP, +40 San, +35 MP, cleanses curses", "+7 Might, +3 Arcana weapon", "+7 Ward, +25 Max HP armor", "+9 Light Radius, +20 Max Sanity relic", "+4 Arcana, +20 Max Sanity amulet", "Reveals all floor rooms & stairs", "Unlocks an unmastered Ancient Rune", "Draws random high-tier treasure/elixir" }
            };
            int warePrices[2][8] = {
                { 15, 18, 22, 55, 60, 65, 35, 40 },
                { 45, 75, 85, 80, 90, 35, 75, 50 }
            };
            int dIdx = isDeep ? 1 : 0;

            for (int i = 0; i < 8; i++) {
                RECT rR = {modalRect.left + 20, rowY, modalRect.right - 20, rowY + 38};
                BOOL afford = (g_player.essence >= warePrices[dIdx][i]);
                HBRUSH rBr = CreateSolidBrush(afford ? RGB(16, 23, 38) : RGB(10, 14, 22));
                FillRect(memDC, &rR, rBr);
                DeleteObject(rBr);
                FrameRect(memDC, &rR, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

                SelectObject(memDC, fontBold);
                SetTextColor(memDC, afford ? RGB(255, 255, 255) : COLOR_TEXT_DIM);
                char lineTitle[128];
                snprintf(lineTitle, sizeof(lineTitle), "[%d] %s", i + 1, wareNames[dIdx][i]);
                TextOutA(memDC, modalRect.left + 28, rowY + 4, lineTitle, (int)strlen(lineTitle));

                SelectObject(memDC, fontSmall);
                SetTextColor(memDC, COLOR_TEXT_DIM);
                TextOutA(memDC, modalRect.left + 28, rowY + 20, wareDescs[dIdx][i], (int)strlen(wareDescs[dIdx][i]));

                char pBuf[32];
                snprintf(pBuf, sizeof(pBuf), "%d Gold", warePrices[dIdx][i]);
                SelectObject(memDC, fontBold);
                SetTextColor(memDC, afford ? COLOR_TEXT_GOLD : RGB(150, 50, 50));
                TextOutA(memDC, modalRect.right - 110, rowY + 10, pBuf, (int)strlen(pBuf));

                rowY += 41;
            }
        } else {
            // SELL MODE (Display Pack items up to 8)
            if (g_player.numPackItems == 0) {
                SelectObject(memDC, fontMono);
                SetTextColor(memDC, COLOR_TEXT_DIM);
                TextOutA(memDC, modalRect.left + 40, rowY + 60, "Your Delver's Pack is empty. Delve into the crypts to find treasures!", 69);
            } else {
                int showCount = g_player.numPackItems > 8 ? 8 : g_player.numPackItems;
                for (int i = 0; i < showCount; i++) {
                    InventorySlot* slot = &g_player.pack[i];
                    const ItemDef* it = &g_itemDefs[slot->id];
                    int sVal = GetItemSellValue(slot->id);

                    RECT rR = {modalRect.left + 20, rowY, modalRect.right - 20, rowY + 38};
                    HBRUSH rBr = CreateSolidBrush(RGB(12, 28, 22));
                    FillRect(memDC, &rR, rBr);
                    DeleteObject(rBr);
                    FrameRect(memDC, &rR, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

                    SelectObject(memDC, fontBold);
                    SetTextColor(memDC, RGB(236, 253, 245));
                    char sTitle[128];
                    snprintf(sTitle, sizeof(sTitle), "[%d] %s (Qty: %d)", i + 1, it->name, slot->count);
                    TextOutA(memDC, modalRect.left + 28, rowY + 4, sTitle, (int)strlen(sTitle));

                    SelectObject(memDC, fontSmall);
                    SetTextColor(memDC, COLOR_TEXT_DIM);
                    TextOutA(memDC, modalRect.left + 28, rowY + 20, it->desc, (int)strlen(it->desc));

                    char pBuf[32];
                    snprintf(pBuf, sizeof(pBuf), "+%d Gold", sVal);
                    SelectObject(memDC, fontBold);
                    SetTextColor(memDC, RGB(52, 211, 153));
                    TextOutA(memDC, modalRect.right - 110, rowY + 10, pBuf, (int)strlen(pBuf));

                    rowY += 41;
                }
            }
        }

        // Bottom status line
        SelectObject(memDC, fontSmall);
        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, modalRect.left + 20, modalRect.bottom - 22,
                 "Shortcuts: [1..8] Trade Item  |  [S/TAB] Switch Buy/Sell  |  [ESC / M] Close Market", 83);
    }

    // 6. BOTTOM FOOTER (0..width, height-24..height)
    RECT footerRect = {0, height - 24, width, height};
    FillRect(memDC, &footerRect, panelDarkBrush);
    DeleteObject(panelDarkBrush);

    SelectObject(memDC, fontSmall);
    SetTextColor(memDC, COLOR_TEXT_DIM);
    TextOutA(memDC, 14, height - 18, "WASD/Arrows: Move | Space: Rest | E: Interact | R/X: Search | C: CRT | F: FOV | H: Manual", 89);
    TextOutA(memDC, width - 240, height - 18, "KAbyss Native Engine v0.4", 25);

    // Cleanup GDI objects
    SelectObject(memDC, oldFont);
    SelectObject(memDC, oldPen);
    DeleteObject(borderPen);
    DeleteObject(fontMono);
    DeleteObject(fontBold);
    DeleteObject(fontSmall);
    DeleteObject(fontTitle);

    // Blit to screen
    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBM);
    DeleteObject(memBM);
    DeleteDC(memDC);
}

void ResetPlayerRun(void) {
    g_player.base_max_hp = 100;
    g_player.base_max_sanity = 100;
    g_player.base_max_aether = 50;
    g_player.base_might = 14;
    g_player.base_warding = 12;
    g_player.base_arcana = 14;
    g_player.base_light_radius = 7;
    g_player.hp = 100;
    g_player.sanity = 100;
    g_player.hunger = 100;
    g_player.max_hunger = 100;
    g_player.torchFuel = 160;
    g_player.maxTorchFuel = 160;
    g_player.torchLit = TRUE;
    g_player.curse = CURSE_NONE;
    g_player.curseTurns = 0;
    g_player.aether = 50;
    g_player.shield = 0;
    g_player.essence = 0;
    g_player.level = 1;
    g_player.exp = 0;
    g_player.max_exp = 100;
    g_player.facing = 2; // Down
    g_player.equippedStaff = 0; // Ashwood Rune Staff
    g_player.staffSockets[0] = 0; // Pyre Rune socketed
    g_player.staffSockets[1] = -1; // Empty
    g_player.staffSockets[2] = -1;
    for (int r = 0; r < NUM_RUNES; r++) g_player.ownedRunes[r] = FALSE;
    g_player.ownedRunes[0] = TRUE; // Delver starts with Pyre Rune

    // Starting Equipment & Pack
    g_player.equipWeapon = ITEM_WPN_RUNIC_BLADE;
    g_player.weaponEnchant = ENCHANT_NONE;
    g_player.equipArmor = ITEM_ARM_ABYSSAL_MAIL;
    g_player.equipRelic = ITEM_REL_TORCH;
    g_player.equipAmulet = ITEM_NONE;
    g_player.numPackItems = 0;
    AddPackItem(ITEM_HEAL_SALVE, 2);
    AddPackItem(ITEM_SANITY_INCENSE, 1);
    AddPackItem(ITEM_FOOD_RATIONS, 2);
    AddPackItem(ITEM_PURIFYING_SALT, 1);
    AddPackItem(ITEM_KEY_RUNIC, 1);
    AddPackItem(ITEM_ING_BRIMSTONE, 1);
    AddPackItem(ITEM_ING_BLOOD_LOTUS, 1);
    AddPackItem(ITEM_ING_AETHER_BLOSSOM, 1);
    RecalcPlayerStats();
}

// Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        ResetPlayerRun();
        InitGame(1);
        SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL, NULL);
        break;


    case WM_TIMER:
        if (wParam == TIMER_ID) {
            g_frameCount++;
            g_animFlicker = sinf((float)g_frameCount * 0.15f) * 0.05f;
            UpdateEmbers();
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;

    case WM_KEYDOWN:
        if (g_showHelpModal) {
            if (wParam == 'H' || wParam == VK_F1 || wParam == VK_ESCAPE || wParam == VK_SPACE || wParam == VK_RETURN) {
                g_showHelpModal = FALSE;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
        }

        if (g_showEnchantModal) {
            if (wParam == '1') {
                ImbueEnchantment(ENCHANT_FIRE);
            } else if (wParam == '2') {
                ImbueEnchantment(ENCHANT_FROST);
            } else if (wParam == '3') {
                ImbueEnchantment(ENCHANT_VOID);
            } else if (wParam == '4') {
                CommuneAltarBenediction();
            } else if (wParam == '0' || wParam == 'C') {
                ImbueEnchantment(ENCHANT_NONE);
            } else if (wParam == 'I' || wParam == VK_ESCAPE || wParam == VK_SPACE || wParam == VK_RETURN) {
                g_showEnchantModal = FALSE;
            }
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        if (g_showMerchantModal) {
            if (wParam >= '1' && wParam <= '8') {
                int idx = (int)(wParam - '1');
                if (g_merchantMode == 0) {
                    BuyMerchantItem(idx);
                } else {
                    SellPackItemToMerchant(idx);
                }
            } else if (wParam == 'S' || wParam == VK_TAB) {
                g_merchantMode = 1 - g_merchantMode;
                Beep(480, 30);
            } else if (wParam == 'M' || wParam == VK_ESCAPE || wParam == VK_SPACE || wParam == VK_RETURN) {
                CloseMerchantShop();
            }
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        switch (wParam) {
        // Movement: WASD / Arrows / Vi / Numpad
        case VK_UP:
        case 'W':
        case 'K':
        case VK_NUMPAD8:
            MovePlayer(0, -1);
            break;

        case VK_DOWN:
        case 'S':
            MovePlayer(0, 1);
            break;

        case 'J':
        case VK_NUMPAD2:
            MovePlayer(0, 1);
            break;

        case VK_LEFT:
        case 'A':
            MovePlayer(-1, 0);
            break;

        case 'H':
            g_showHelpModal = !g_showHelpModal;
            break;

        case VK_NUMPAD4:
            MovePlayer(-1, 0);
            break;

        case VK_RIGHT:
        case 'D':
        case 'L':
        case VK_NUMPAD6:
            MovePlayer(1, 0);
            break;

        // Diagonals
        case 'Y':
        case VK_NUMPAD7:
            MovePlayer(-1, -1);
            break;

        case 'U':
            if (g_activeTab == 1) {
                // Find and use first consumable
                BOOL used = FALSE;
                for (int i = 0; i < g_player.numPackItems; i++) {
                    if (g_itemDefs[g_player.pack[i].id].category == ITEM_TYPE_CONSUMABLE) {
                        UsePackItem(i);
                        used = TRUE;
                        break;
                    }
                }
                if (!used) AddLog("No consumable potion or salve in Delver's Pack.", COLOR_TEXT_DIM);
            } else {
                MovePlayer(1, -1);
            }
            break;

        case VK_NUMPAD9:
            MovePlayer(1, -1);
            break;

        case 'B':
            if (g_activeTab != 1) {
                g_activeTab = 1;
                AddLog("Opened Delver's Pack & Ancient Alchemy Cauldron [2].", RGB(52, 211, 153));
            } else {
                // Brew first available recipe!
                BOOL brewed = FALSE;
                for (int r = 0; r < NUM_RECIPES; r++) {
                    const AlchemyRecipe* rec = &g_recipes[r];
                    BOOL canBrew = TRUE;
                    for (int k = 0; k < rec->numIng; k++) {
                        if (GetPackItemCount(rec->ing[k].id) < rec->ing[k].count) {
                            canBrew = FALSE;
                            break;
                        }
                    }
                    if (canBrew) {
                        BrewRecipe(r);
                        brewed = TRUE;
                        break;
                    }
                }
                if (!brewed) {
                    AddLog("No cauldron recipes can currently be brewed with available reagents.", COLOR_TEXT_DIM);
                }
            }
            break;

        case VK_DELETE:
            if (g_activeTab == 1 && g_player.numPackItems > 0) {
                ItemId dropId = g_player.pack[0].id;
                char buf[128];
                snprintf(buf, sizeof(buf), "Discarded %s from Delver's Pack.", g_itemDefs[dropId].name);
                RemovePackItem(dropId, 1);
                AddLog(buf, COLOR_TEXT_DIM);
                Beep(300, 30);
            }
            break;

        case VK_NUMPAD1:
            MovePlayer(-1, 1);
            break;

        case 'N':
            if (GetAsyncKeyState(VK_CONTROL)) {
                ResetPlayerRun();
                InitGame(1);
                AddLog("Embarking on a brand new descent into the Abyss.", COLOR_ACCENT_AMBER);
            } else {
                MovePlayer(1, 1);
            }
            break;

        case VK_NUMPAD3:
            MovePlayer(1, 1);
            break;

        // CRT Toggle
        case 'C':
            g_crtEnabled = !g_crtEnabled;
            AddLog(g_crtEnabled ? "CRT Phosphors & Scanlines: ENABLED." : "CRT Scanlines: DISABLED.", COLOR_ACCENT_CYAN);
            break;

        // Rest
        case VK_SPACE:
        case VK_NUMPAD5:
        case VK_OEM_PERIOD:
            RestTurn();
            break;

        // Interact / Descend
        case 'E':
        case VK_RETURN:
            InteractTile();
            break;

        // Search
        case 'R':
            SearchArea();
            break;

        // Rekindle Torch
        case 'T':
            RekindleTorch();
            break;

        // Spells from Staff Sockets
        case 'Z':
            CastSpell(0);
            break;

        case 'X':
            CastSpell(1);
            break;

        case 'V':
            if (g_staffDefs[g_player.equippedStaff].maxSockets >= 3) {
                CastSpell(2);
            } else {
                g_fovEnabled = !g_fovEnabled;
                ComputeFOV();
                AddLog(g_fovEnabled ? "Field of View: ENABLED." : "Field of View: DISABLED (Omnivision).", COLOR_BORDER_GLOW);
            }
            break;

        // Toggle FOV
        case 'F':
            g_fovEnabled = !g_fovEnabled;
            ComputeFOV();
            AddLog(g_fovEnabled ? "Field of View: ENABLED." : "Field of View: DISABLED (Omnivision).", COLOR_BORDER_GLOW);
            break;

        // Tabs
        case '1':
            g_activeTab = 0;
            break;
        case '2':
            g_activeTab = 1;
            break;
        case '3':
            g_activeTab = 2;
            break;
        case '4':
            g_activeTab = 3;
            break;

        case VK_F2:
            ResetPlayerRun();
            InitGame(1);
            AddLog("Embarking on a brand new descent into the Abyss.", COLOR_ACCENT_AMBER);
            break;

        case VK_F1:
            g_showHelpModal = !g_showHelpModal;
            break;

        case 'I':
            g_showEnchantModal = !g_showEnchantModal;
            break;

        case 'M':
            if (g_showMerchantModal) {
                CloseMerchantShop();
            } else {
                OpenMerchantShop();
            }
            break;

        case VK_ESCAPE:
            if (g_showHelpModal) g_showHelpModal = FALSE;
            if (g_showEnchantModal) g_showEnchantModal = FALSE;
            if (g_showMerchantModal) g_showMerchantModal = FALSE;
            break;
        }

        InvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_LBUTTONDOWN: {
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);

        if (g_showHelpModal) {
            g_showHelpModal = FALSE;
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        if (g_showMerchantModal) {
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;
            RECT modalRect = {width / 2 - 340, height / 2 - 240, width / 2 + 340, height / 2 + 240};

            int tabY = modalRect.top + 86;
            if (mouseY >= tabY && mouseY <= tabY + 24) {
                if (mouseX >= modalRect.left + 20 && mouseX <= modalRect.left + 180) {
                    g_merchantMode = 0;
                } else if (mouseX >= modalRect.left + 190 && mouseX <= modalRect.left + 350) {
                    g_merchantMode = 1;
                }
            } else if (mouseX >= modalRect.left + 20 && mouseX <= modalRect.right - 20) {
                int rowY = tabY + 30;
                for (int i = 0; i < 8; i++) {
                    if (mouseY >= rowY && mouseY <= rowY + 38) {
                        if (g_merchantMode == 0) {
                            BuyMerchantItem(i);
                        } else {
                            SellPackItemToMerchant(i);
                        }
                        break;
                    }
                    rowY += 41;
                }
            } else {
                CloseMerchantShop();
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        if (g_showEnchantModal) {
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;
            RECT modalRect = {width / 2 - 320, height / 2 - 220, width / 2 + 320, height / 2 + 220};

            int opt1Y = modalRect.top + 58 + 54;
            int opt2Y = opt1Y + 50;
            int opt3Y = opt2Y + 50;
            int opt4Y = opt3Y + 50;
            int opt0Y = opt4Y + 52;

            if (mouseX >= modalRect.left + 20 && mouseX <= modalRect.right - 20) {
                if (mouseY >= opt1Y && mouseY <= opt1Y + 44) {
                    ImbueEnchantment(ENCHANT_FIRE);
                } else if (mouseY >= opt2Y && mouseY <= opt2Y + 44) {
                    ImbueEnchantment(ENCHANT_FROST);
                } else if (mouseY >= opt3Y && mouseY <= opt3Y + 44) {
                    ImbueEnchantment(ENCHANT_VOID);
                } else if (mouseY >= opt4Y && mouseY <= opt4Y + 44) {
                    CommuneAltarBenediction();
                } else if (mouseY >= opt0Y && mouseY <= opt0Y + 24) {
                    ImbueEnchantment(ENCHANT_NONE);
                } else if (mouseY < modalRect.top + 45) {
                    g_showEnchantModal = FALSE;
                }
            } else {
                g_showEnchantModal = FALSE;
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        // Check tabs clicks
        int sbX = 728;
        int sbY = 46;
        if (mouseY >= sbY && mouseY <= sbY + 28 && mouseX >= sbX && mouseX <= sbX + 308) {
            int t = (mouseX - sbX) / 77;
            if (t < 0) t = 0;
            if (t > 3) t = 3;
            g_activeTab = t;
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        // Check Tab 0 (Delver Equipment) clicks to unequip
        if (g_activeTab == 0 && mouseX >= sbX + 16 && mouseX <= sbX + 308 - 16) {
            int gearY = sbY + 36 + 242;
            if (mouseY >= gearY + 38 && mouseY <= gearY + 56) {
                UnequipSlot(SLOT_WEAPON);
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            } else if (mouseY > gearY + 56 && mouseY <= gearY + 74) {
                UnequipSlot(SLOT_ARMOR);
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            } else if (mouseY > gearY + 74 && mouseY <= gearY + 92) {
                UnequipSlot(SLOT_RELIC);
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            } else if (mouseY > gearY + 92 && mouseY <= gearY + 115) {
                UnequipSlot(SLOT_AMULET);
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
        }

        // Check Tab 1 (Delver's Pack & Cauldron) clicks
        if (g_activeTab == 1 && mouseX >= sbX + 14 && mouseX <= sbX + 308 - 14) {
            int contentY = sbY + 36;
            // Check Pack items (0..7)
            int itemY = contentY + 25;
            for (int i = 0; i < 8; i++) {
                if (mouseY >= itemY && mouseY <= itemY + 20) {
                    if (i < g_player.numPackItems) {
                        UsePackItem(i);
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }
                itemY += 21;
            }

            // Check Cauldron Recipes (0..5)
            int caulY = contentY + 230 + 8;
            int recY = caulY + 42;
            for (int r = 0; r < NUM_RECIPES; r++) {
                if (mouseY >= recY && mouseY <= recY + 34) {
                    BrewRecipe(r);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                recY += 38;
            }
        }


        // Check Tab 2 (Runic Forge) clicks
        if (g_activeTab == 2 && mouseX >= sbX + 16 && mouseX <= sbX + 308 - 16) {
            int curStfMax = g_staffDefs[g_player.equippedStaff].maxSockets;
            int sy = sbY + 36 + 46;
            // Check clicks on Staff Sockets (to unsocket)
            for (int s = 0; s < curStfMax; s++) {
                if (mouseY >= sy && mouseY <= sy + 30) {
                    UnsocketRune(s);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                sy += 34;
            }

            // Check clicks on Owned Runes (to socket into first available slot)
            sy += 24; // header offset
            for (int r = 0; r < NUM_RUNES; r++) {
                if (mouseY >= sy && mouseY <= sy + 22) {
                    if (g_player.ownedRunes[r]) {
                        // Find first empty socket, or socket into slot 0
                        int targetSlot = 0;
                        for (int s = 0; s < curStfMax; s++) {
                            if (g_player.staffSockets[s] == -1) {
                                targetSlot = s;
                                break;
                            }
                        }
                        SocketRune(targetSlot, r);
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }
                sy += 24;
            }
        }

        // Check toolbar buttons
        int vpX = 12;
        if (mouseY >= 574 && mouseY <= 608 && mouseX >= vpX && mouseX <= vpX + VIEWPORT_W) {
            if (mouseX < vpX + 65) {
                // New Descent
                ResetPlayerRun();
                InitGame(1);
            } else if (mouseX < vpX + 155) {
                // Rest
                RestTurn();
            } else if (mouseX < vpX + 240) {
                // Search
                SearchArea();
            } else if (mouseX < vpX + 330) {
                // Descend
                InteractTile();
            } else if (mouseX < vpX + 425) {
                // Rekindle Torch
                RekindleTorch();
            } else if (mouseX < vpX + 515) {
                // Enchant Altar
                g_showEnchantModal = !g_showEnchantModal;
            } else if (mouseX < vpX + 580) {
                // Toggle CRT
                g_crtEnabled = !g_crtEnabled;
                AddLog(g_crtEnabled ? "CRT Phosphors & Scanlines: ENABLED." : "CRT Scanlines: DISABLED.", COLOR_ACCENT_CYAN);
            } else if (mouseX < vpX + 640) {
                // Toggle FOV
                g_fovEnabled = !g_fovEnabled;
                ComputeFOV();
            } else {
                // Help
                g_showHelpModal = TRUE;
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        // Check Spell Hotbar clicks inside Viewport (hbY = 46 + VIEWPORT_H - 34 = 532)
        if (mouseY >= 46 + VIEWPORT_H - 34 && mouseY <= 46 + VIEWPORT_H - 8 && mouseX >= vpX + 10) {
            int maxS = g_staffDefs[g_player.equippedStaff].maxSockets;
            for (int s = 0; s < maxS; s++) {
                int slotX = vpX + 10 + s * 160;
                if (mouseX >= slotX && mouseX <= slotX + 152) {
                    CastSpell(s);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            }
        }

        // Check viewport click
        if (mouseX >= vpX && mouseX < vpX + VIEWPORT_W &&
            mouseY >= 46 && mouseY < 46 + VIEWPORT_H) {
            int clickX = mouseX - vpX + g_camX;
            int clickY = mouseY - 46 + g_camY;

            int tileX = clickX / TILE_SIZE;
            int tileY = clickY / TILE_SIZE;

            if (tileX >= 0 && tileX < MAP_WIDTH && tileY >= 0 && tileY < MAP_HEIGHT) {
                int dx = tileX - g_player.x;
                int dy = tileY - g_player.y;

                if (abs(dx) <= 1 && abs(dy) <= 1 && (dx != 0 || dy != 0)) {
                    MovePlayer(dx, dy);
                } else if (g_explored[tileY][tileX]) {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "Surveying tile (%d, %d)...", tileX, tileY);
                    AddLog(buf, COLOR_TEXT_DIM);
                }
            }
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RenderGame(hdc, hwnd);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_ERASEBKGND:
        return 1; // Prevent flicker

    case WM_DESTROY:
        KillTimer(hwnd, TIMER_ID);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KAbyssWindowClass";

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassA(&wc);

    RECT wr = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    AdjustWindowRect(&wr, (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN, FALSE);

    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME, "KAbyss - Abyssal Crypt Crawler [WASD/Arrows: Move | Space: Rest | E: Descend | H/F1: Manual]",
        (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
