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
    int facing; // 0=Up, 1=Right, 2=Down, 3=Left
    int equippedStaff; // 0=Ashwood, 1=Cinder, 2=Arch-Magi
    int staffSockets[3]; // Rune index 0..4 or -1 for empty
    BOOL ownedRunes[NUM_RUNES];
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

static Delver g_player;
static int g_depthLevel = 1;
static int g_turn = 1;
static BOOL g_fovEnabled = TRUE;
static BOOL g_crtEnabled = TRUE;
static int g_activeTab = 0; // 0=Hero, 1=Inventory, 2=Runes
static BOOL g_showHelpModal = FALSE;
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
void CheckLevelUp(void);
void SpawnEmber(float x, float y, BOOL isTorch);
void UpdateEmbers(void);
void CastSpell(int socketIdx);
void SocketRune(int socketIdx, int runeIdx);
void UnsocketRune(int socketIdx);

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

void CheckLevelUp(void) {
    if (g_player.exp >= g_player.max_exp) {
        g_player.exp -= g_player.max_exp;
        g_player.level++;
        g_player.max_exp = (int)(g_player.max_exp * 1.5f);
        g_player.max_hp += 15;
        g_player.hp = g_player.max_hp;
        g_player.max_aether += 10;
        g_player.aether = g_player.max_aether;
        g_player.might += 2;
        g_player.warding += 1;
        g_player.arcana += 2;

        char buf[128];
        snprintf(buf, sizeof(buf), "LEVEL UP! Delver reached Level %d! (+15 HP, +10 Aether, +2 Might, +2 Arcana)", g_player.level);
        AddLog(buf, COLOR_ACCENT_PURPLE);
        Beep(880, 70); Beep(1175, 100);
    }
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
}

// 4. Void Abyss: Floating Obsidian Platforms over Cosmic Chasms & Void Rifts
void GenerateVoidAbyss(int depth) {
    Room plats[MAX_ROOMS];
    int platCount = 0;
    int targetPlats = 7 + RandInt(0, 3);

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

    ComputeFOV();

    char buf[128];
    snprintf(buf, sizeof(buf), zt->enterMsg, depth);
    AddLog(buf, COLOR_ACCENT_AMBER);
}

void CommuneAltar(int x, int y) {
    g_dungeon[y][x] = TILE_RUBBLE;
    g_player.sanity += 30;
    if (g_player.sanity > g_player.max_sanity) g_player.sanity = g_player.max_sanity;
    g_player.hp += 20;
    if (g_player.hp > g_player.max_hp) g_player.hp = g_player.max_hp;
    g_player.aether += 25;
    if (g_player.aether > g_player.max_aether) g_player.aether = g_player.max_aether;
    g_player.essence += 35;
    g_player.exp += 30;

    int unowned[NUM_RUNES];
    int unownedCount = 0;
    for (int r = 0; r < NUM_RUNES; r++) {
        if (!g_player.ownedRunes[r]) unowned[unownedCount++] = r;
    }

    if (unownedCount > 0 && RandInt(0, 100) < 60) {
        int pick = unowned[RandInt(0, unownedCount - 1)];
        g_player.ownedRunes[pick] = TRUE;
        char buf[128];
        snprintf(buf, sizeof(buf), "RUNIC COMMUNION! Altar reveals the %s (%s)! Inscribe in Tab [3].", g_runeDefs[pick].name, g_runeDefs[pick].symbol);
        AddLog(buf, COLOR_TEXT_GOLD);
        Beep(523, 60); Beep(784, 80);
    } else {
        AddLog("ALTAR COMMUNION! Primordial runic energies infuse you! (+30 Sanity, +20 HP, +25 Aether, +35 Essence, +30 EXP)", COLOR_TEXT_GOLD);
        Beep(523, 60); Beep(659, 60); Beep(784, 80);
    }
    CheckLevelUp();
    AdvanceTurn();
}

void AdvanceTurn(void) {
    g_turn++;
    int sanityInterval = (g_depthLevel >= 10) ? 25 : 40;
    if (g_turn % sanityInterval == 0 && g_player.sanity > 10) {
        g_player.sanity -= 2;
        if (g_depthLevel >= 10) {
            AddLog("Cosmic whispers from the Void Abyss twist your willpower (-2 Sanity).", COLOR_ACCENT_PURPLE);
        } else {
            AddLog("Subterranean echoes fray your willpower (-2 Sanity).", COLOR_ACCENT_AMBER);
        }
    }
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

    if (tile == TILE_DOOR_CLOSED) {
        g_dungeon[ny][nx] = TILE_DOOR_OPEN;
        AddLog("You push open the heavy subterranean door.", COLOR_ACCENT_CYAN);
        Beep(350, 40);
        AdvanceTurn();
        return;
    }

    if (tile == TILE_ALTAR) {
        CommuneAltar(nx, ny);
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
                    snprintf(buf, sizeof(buf), "Chest opened! +%d Essence & found Cinderwood Scepter (+4 Arcana)!", g_chests[c].essence);
                } else if (g_player.equippedStaff < 2 && g_depthLevel >= 7 && RandInt(0, 100) < 35) {
                    g_player.equippedStaff = 2;
                    snprintf(buf, sizeof(buf), "Chest opened! +%d Essence & found Staff of the Arch-Magi (3 Sockets)!", g_chests[c].essence);
                } else {
                    snprintf(buf, sizeof(buf), "Opened Relic Chest! +%d Essence & +25 EXP!", g_chests[c].essence);
                }

                AddLog(buf, COLOR_TEXT_GOLD);
                Beep(600, 40); Beep(800, 50);
                CheckLevelUp();
                AdvanceTurn();
                return;
            }
        }
    }

    g_player.x = nx;
    g_player.y = ny;

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
                    snprintf(buf, sizeof(buf), "Detected consecrated Runic Altar at (%d, %d)!", nx, ny);
                    AddLog(buf, COLOR_TEXT_GOLD);
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
        CommuneAltar(g_player.x, g_player.y);
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
                        CommuneAltar(nx, ny);
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
    snprintf(badgeBuf, sizeof(badgeBuf), "DEPTH: B%d (%s)", g_depthLevel, zt->shortName);
    SetTextColor(memDC, COLOR_ACCENT_PURPLE);
    TextOutA(memDC, 330, 11, badgeBuf, (int)strlen(badgeBuf));

    // HP Badge
    snprintf(badgeBuf, sizeof(badgeBuf), "HP: %d/%d", g_player.hp, g_player.max_hp);
    SetTextColor(memDC, COLOR_ACCENT_RED);
    TextOutA(memDC, 490, 11, badgeBuf, (int)strlen(badgeBuf));

    // Sanity Badge
    snprintf(badgeBuf, sizeof(badgeBuf), "SANITY: %d/%d", g_player.sanity, g_player.max_sanity);
    SetTextColor(memDC, COLOR_ACCENT_CYAN);
    TextOutA(memDC, 600, 11, badgeBuf, (int)strlen(badgeBuf));

    // Aether Badge
    if (g_player.shield > 0) {
        snprintf(badgeBuf, sizeof(badgeBuf), "MP: %d/%d [+%d]", g_player.aether, g_player.max_aether, g_player.shield);
    } else {
        snprintf(badgeBuf, sizeof(badgeBuf), "MP: %d/%d", g_player.aether, g_player.max_aether);
    }
    SetTextColor(memDC, RGB(168, 85, 247));
    TextOutA(memDC, 740, 11, badgeBuf, (int)strlen(badgeBuf));

    // Essence Badge
    snprintf(badgeBuf, sizeof(badgeBuf), "ESSENCE: %d*", g_player.essence);
    SetTextColor(memDC, COLOR_TEXT_GOLD);
    TextOutA(memDC, 895, 11, badgeBuf, (int)strlen(badgeBuf));

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
            } else if (tile == TILE_ALTAR) {
                SelectObject(memDC, fontBold);
                SetTextColor(memDC, isVisible ? COLOR_ACCENT_RED : RGB(140, 20, 20));
                TextOutA(memDC, scrX + 7, scrY + 8, "[+]", 3);
            } else if (tile == TILE_PILLAR) {
                HBRUSH pBr = CreateSolidBrush(isVisible ? zt->torchColor : RGB(49, 46, 129));
                HBRUSH oldP = (HBRUSH)SelectObject(memDC, pBr);
                Ellipse(memDC, scrX + 6, scrY + 6, scrX + TILE_SIZE - 6, scrY + TILE_SIZE - 6);
                SelectObject(memDC, oldP);
                DeleteObject(pBr);
            } else if (tile == TILE_RUBBLE) {
                SelectObject(memDC, fontSmall);
                SetTextColor(memDC, isVisible ? RGB(148, 163, 184) : RGB(51, 65, 85));
                TextOutA(memDC, scrX + 11, scrY + 8, "::", 2);
            } else if (tile == TILE_STAIRS_DOWN) {
                SelectObject(memDC, fontBold);
                SetTextColor(memDC, isVisible ? COLOR_ACCENT_PURPLE : RGB(76, 29, 149));
                TextOutA(memDC, scrX + 10, scrY + 8, "v", 1);
            } else if (tile == TILE_STAIRS_UP) {
                SelectObject(memDC, fontBold);
                SetTextColor(memDC, isVisible ? COLOR_BORDER_GLOW : RGB(3, 105, 161));
                TextOutA(memDC, scrX + 10, scrY + 8, "^", 1);
            } else if (tile == TILE_CHEST) {
                SelectObject(memDC, fontBold);
                SetTextColor(memDC, isVisible ? COLOR_TEXT_GOLD : RGB(120, 53, 15));
                TextOutA(memDC, scrX + 9, scrY + 8, "[$]", 3);
            } else if (tile == TILE_DOOR_CLOSED) {
                SelectObject(memDC, fontBold);
                SetTextColor(memDC, isVisible ? COLOR_TEXT_GOLD : RGB(120, 53, 15));
                TextOutA(memDC, scrX + 10, scrY + 8, "+", 1);
            } else if (tile == TILE_DOOR_OPEN) {
                SelectObject(memDC, fontBold);
                SetTextColor(memDC, COLOR_TEXT_DIM);
                TextOutA(memDC, scrX + 10, scrY + 8, "/", 1);
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

            SelectObject(memDC, fontBold);
            SetTextColor(memDC, zt->torchColor);
            TextOutA(memDC, scrX + 10, scrY + 7, "*", 1);

            if (RandInt(0, 10) < 2) {
                SpawnEmber((float)(scrX + 16), (float)(scrY + 12), TRUE);
            }
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

        // Delver Body Avatar
        HBRUSH bodyBr = CreateSolidBrush(RGB(8, 47, 73));
        HBRUSH oldB = (HBRUSH)SelectObject(memDC, bodyBr);
        HPEN bodyPen = CreatePen(PS_SOLID, 2, COLOR_BORDER_GLOW);
        HPEN oldBP = (HPEN)SelectObject(memDC, bodyPen);
        Ellipse(memDC, plScrX + 5, plScrY + 5, plScrX + TILE_SIZE - 5, plScrY + TILE_SIZE - 5);
        SelectObject(memDC, oldBP);
        DeleteObject(bodyPen);
        SelectObject(memDC, oldB);
        DeleteObject(bodyBr);

        // Player Symbol
        SelectObject(memDC, fontBold);
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, plScrX + 11, plScrY + 7, "@", 1);

        // Facing pointer
        int fx = plScrX + 16;
        int fy = plScrY + 16;
        if (g_player.facing == 0) fy -= 10;
        else if (g_player.facing == 2) fy += 10;
        else if (g_player.facing == 3) fx -= 10;
        else if (g_player.facing == 1) fx += 10;
        SetPixel(memDC, fx, fy, RGB(255, 255, 255));
        SetPixel(memDC, fx + 1, fy, RGB(255, 255, 255));
        SetPixel(memDC, fx, fy + 1, RGB(255, 255, 255));

        // Prismatic Shield Halo if Ward active
        if (g_player.shield > 0) {
            HPEN shieldPen = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
            HPEN oldSP = (HPEN)SelectObject(memDC, shieldPen);
            HBRUSH oldSB = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));
            int shRad = 15 + (int)(sinf((float)g_frameCount * 0.2f) * 2.0f);
            Ellipse(memDC, plScrX + 16 - shRad, plScrY + 16 - shRad, plScrX + 16 + shRad, plScrY + 16 + shRad);
            SelectObject(memDC, oldSB);
            SelectObject(memDC, oldSP);
            DeleteObject(shieldPen);
        }

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
    char turnText[64];
    snprintf(turnText, sizeof(turnText), "Turn: %d | Light: 100%% | %s: Lit", g_turn, ztHud->sconceName);
    TextOutA(memDC, vpX + 18, vpY + 33, turnText, (int)strlen(turnText));

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
    TextOutA(memDC, vpX + 6, 584, "[N] Descent", 11);
    TextOutA(memDC, vpX + 112, 584, "[Space] Rest", 12);
    TextOutA(memDC, vpX + 218, 584, "[R/X] Search", 12);
    TextOutA(memDC, vpX + 320, 584, "[E] Descend", 11);
    TextOutA(memDC, vpX + 418, 584, g_crtEnabled ? "[C] CRT: ON" : "[C] CRT: OFF", g_crtEnabled ? 11 : 12);
    TextOutA(memDC, vpX + 526, 584, g_fovEnabled ? "[F] FOV: ON" : "[F] FOV: OFF", g_fovEnabled ? 11 : 12);
    TextOutA(memDC, vpX + 636, 584, "[H] Tome", 8);

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
    TextOutA(memDC, sbX + 20, sbY + 7, "[1] DELVER", 10);
    // Tab 1: Relics
    SetTextColor(memDC, g_activeTab == 1 ? COLOR_BORDER_GLOW : COLOR_TEXT_DIM);
    TextOutA(memDC, sbX + 120, sbY + 7, "[2] RELICS", 10);
    // Tab 2: Runes
    SetTextColor(memDC, g_activeTab == 2 ? COLOR_BORDER_GLOW : COLOR_TEXT_DIM);
    TextOutA(memDC, sbX + 220, sbY + 7, "[3] RUNES", 9);

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

        // EXP Bar
        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 108, "Experience", 10);
        char expTxt[32];
        snprintf(expTxt, sizeof(expTxt), "%d / %d", g_player.exp, g_player.max_exp);
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + sbW - 85, contentY + 108, expTxt, (int)strlen(expTxt));

        RECT expBarBg = {sbX + 16, contentY + 122, sbX + sbW - 16, contentY + 127};
        FillRect(memDC, &expBarBg, barDark);
        DeleteObject(barDark);
        int expW = (int)((float)(sbW - 32) * ((float)g_player.exp / (float)g_player.max_exp));
        if (expW < 0) expW = 0; if (expW > sbW - 32) expW = sbW - 32;
        RECT expBarFill = {sbX + 16, contentY + 122, sbX + 16 + expW, contentY + 127};
        HBRUSH expFill = CreateSolidBrush(COLOR_TEXT_GOLD);
        FillRect(memDC, &expBarFill, expFill);
        DeleteObject(expFill);

        // Stats rows
        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 134, "Class:", 6);
        SetTextColor(memDC, COLOR_ACCENT_PURPLE);
        TextOutA(memDC, sbX + 130, contentY + 134, "Rune Knight", 11);

        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 150, "Might (Atk):", 12);
        char stBuf[32];
        snprintf(stBuf, sizeof(stBuf), "%d (+%d)", g_player.might, (g_player.might - 10) / 2);
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + 130, contentY + 150, stBuf, (int)strlen(stBuf));

        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 166, "Warding (Def):", 14);
        snprintf(stBuf, sizeof(stBuf), "%d (+%d)", g_player.warding, (g_player.warding - 10) / 2);
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + 130, contentY + 166, stBuf, (int)strlen(stBuf));

        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 182, "Arcana (Magic):", 15);
        int totalArc = g_player.arcana + g_staffDefs[g_player.equippedStaff].arcanaBonus;
        snprintf(stBuf, sizeof(stBuf), "%d (+%d)", totalArc, (totalArc - 10) / 2);
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + 130, contentY + 182, stBuf, (int)strlen(stBuf));

        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 198, "Light Radius:", 13);
        snprintf(stBuf, sizeof(stBuf), "%d Tiles", g_player.light_radius);
        SetTextColor(memDC, COLOR_TEXT_GOLD);
        TextOutA(memDC, sbX + 130, contentY + 198, stBuf, (int)strlen(stBuf));

        // GEAR SECTION
        int gearY = contentY + 224;
        RECT gearCard = {sbX + 8, gearY, sbX + sbW - 8, gearY + 84};
        HBRUSH gCardBg = CreateSolidBrush(COLOR_BG_CARD);
        FillRect(memDC, &gearCard, gCardBg);
        DeleteObject(gCardBg);
        FrameRect(memDC, &gearCard, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

        SelectObject(memDC, fontBold);
        SetTextColor(memDC, COLOR_TEXT_RUNE);
        TextOutA(memDC, sbX + 16, gearY + 6, "EQUIPPED RELICS & STAFF", 23);

        SelectObject(memDC, fontSmall);
        SetTextColor(memDC, RGB(168, 85, 247));
        char staffStr[80];
        snprintf(staffStr, sizeof(staffStr), "[Stf] %s", g_staffDefs[g_player.equippedStaff].name);
        TextOutA(memDC, sbX + 16, gearY + 24, staffStr, (int)strlen(staffStr));

        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + 16, gearY + 42, "[Wpn] Runic Longsword (+4 Atk)", 30);
        TextOutA(memDC, sbX + 16, gearY + 60, "[Arm] Abyssal Mail (+3 Def)", 27);

    } else if (g_activeTab == 1) {
        // RELICS / PACK INVENTORY
        RECT cardRect = {sbX + 8, contentY, sbX + sbW - 8, contentY + 308};
        HBRUSH cardBg = CreateSolidBrush(COLOR_BG_CARD);
        FillRect(memDC, &cardRect, cardBg);
        DeleteObject(cardBg);
        FrameRect(memDC, &cardRect, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

        SelectObject(memDC, fontBold);
        SetTextColor(memDC, COLOR_TEXT_RUNE);
        TextOutA(memDC, sbX + 16, contentY + 8, "DELVER'S PACK (3/12)", 20);

        SelectObject(memDC, fontSmall);
        SetTextColor(memDC, COLOR_ACCENT_GREEN);
        TextOutA(memDC, sbX + 16, contentY + 34, "1. Healing Salve (+35 HP)", 25);
        SetTextColor(memDC, COLOR_BORDER_GLOW);
        TextOutA(memDC, sbX + 16, contentY + 54, "2. Sanity Incense (+25 Sanity)", 30);
        SetTextColor(memDC, COLOR_TEXT_GOLD);
        TextOutA(memDC, sbX + 16, contentY + 74, "3. Ancient Runic Key", 20);
        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 94, "4. [Empty Slot]", 15);
        TextOutA(memDC, sbX + 16, contentY + 114, "5. [Empty Slot]", 15);
        TextOutA(memDC, sbX + 16, contentY + 134, "6. [Empty Slot]", 15);

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
        RECT modalRect = {width / 2 - 290, height / 2 - 190, width / 2 + 290, height / 2 + 190};
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
        TextOutA(memDC, modalRect.left + 20, modalRect.top + 16, "DELVER'S TOME & SURVIVAL MANUAL", 31);

        SelectObject(memDC, fontSmall);
        SetTextColor(memDC, COLOR_TEXT_PRIMARY);
        int my = modalRect.top + 46;
        TextOutA(memDC, modalRect.left + 20, my, "- WASD / Arrow Keys / Numpad / Vi: Navigate grid", 48); my += 18;
        TextOutA(memDC, modalRect.left + 20, my, "- Space: Rest 1 turn (+2 HP, +1 Sanity, +3 Aether)", 50); my += 18;
        TextOutA(memDC, modalRect.left + 20, my, "- Z / X / V: Cast Elemental Spells from Staff Sockets 1 / 2 / 3", 63); my += 18;
        TextOutA(memDC, modalRect.left + 20, my, "- R: Search surrounding area for secret coffers & altars", 56); my += 18;
        TextOutA(memDC, modalRect.left + 20, my, "- E / Enter: Interact / Descend stairs / Commune with Altars", 60); my += 18;
        TextOutA(memDC, modalRect.left + 20, my, "- 1, 2, 3: Switch Sidebar Tabs (Delver / Relics / Rune Forge)", 61); my += 18;
        TextOutA(memDC, modalRect.left + 20, my, "- Runes: Pyre (Fire), Frost (Ice), Tempest (Shock), Void, Aegis", 63); my += 18;
        TextOutA(memDC, modalRect.left + 20, my, "- Biomes: B1-3 Catacombs | B4-6 Sunken Grotto | B7-9 Crypt | B10+ Void", 70); my += 18;
        TextOutA(memDC, modalRect.left + 20, my, "- C: Toggle CRT Scanlines | F: Toggle Field of View", 51); my += 18;
        TextOutA(memDC, modalRect.left + 20, my, "- Ctrl+N / F2: Start New Descent", 32); my += 22;

        SetTextColor(memDC, COLOR_TEXT_GOLD);
        TextOutA(memDC, modalRect.left + 20, my, "Press [H], [F1], or [ESC] to close manual.", 42);
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

// Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        g_player.hp = 100;
        g_player.max_hp = 100;
        g_player.sanity = 100;
        g_player.max_sanity = 100;
        g_player.aether = 50;
        g_player.max_aether = 50;
        g_player.shield = 0;
        g_player.essence = 0;
        g_player.level = 1;
        g_player.exp = 0;
        g_player.max_exp = 100;
        g_player.might = 14;
        g_player.warding = 12;
        g_player.arcana = 16;
        g_player.light_radius = 7;
        g_player.facing = 2; // Down
        g_player.equippedStaff = 0; // Ashwood Rune Staff
        g_player.staffSockets[0] = 0; // Pyre Rune socketed
        g_player.staffSockets[1] = -1; // Empty
        g_player.staffSockets[2] = -1;
        for (int r = 0; r < NUM_RUNES; r++) g_player.ownedRunes[r] = FALSE;
        g_player.ownedRunes[0] = TRUE; // Delver starts with Pyre Rune

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
        case VK_NUMPAD9:
            MovePlayer(1, -1);
            break;

        case 'B':
        case VK_NUMPAD1:
            MovePlayer(-1, 1);
            break;

        case 'N':
            if (GetAsyncKeyState(VK_CONTROL)) {
                g_player.hp = g_player.max_hp;
                g_player.sanity = g_player.max_sanity;
                g_player.aether = g_player.max_aether;
                g_player.shield = 0;
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

        case VK_F2:
            g_player.hp = g_player.max_hp;
            g_player.sanity = g_player.max_sanity;
            g_player.aether = g_player.max_aether;
            g_player.shield = 0;
            InitGame(1);
            AddLog("Embarking on a brand new descent into the Abyss.", COLOR_ACCENT_AMBER);
            break;

        case VK_F1:
            g_showHelpModal = !g_showHelpModal;
            break;

        case VK_ESCAPE:
            if (g_showHelpModal) g_showHelpModal = FALSE;
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

        // Check tabs clicks
        int sbX = 728;
        int sbY = 46;
        if (mouseY >= sbY && mouseY <= sbY + 28 && mouseX >= sbX && mouseX <= sbX + 308) {
            if (mouseX < sbX + 100) g_activeTab = 0;
            else if (mouseX < sbX + 200) g_activeTab = 1;
            else g_activeTab = 2;
            InvalidateRect(hwnd, NULL, FALSE);
            break;
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
            if (mouseX < vpX + 105) {
                // New Descent
                g_player.hp = g_player.max_hp;
                g_player.sanity = g_player.max_sanity;
                g_player.aether = g_player.max_aether;
                g_player.shield = 0;
                InitGame(1);
            } else if (mouseX < vpX + 210) {
                // Rest
                RestTurn();
            } else if (mouseX < vpX + 315) {
                // Search
                SearchArea();
            } else if (mouseX < vpX + 410) {
                // Descend
                InteractTile();
            } else if (mouseX < vpX + 520) {
                // Toggle CRT
                g_crtEnabled = !g_crtEnabled;
                AddLog(g_crtEnabled ? "CRT Phosphors & Scanlines: ENABLED." : "CRT Scanlines: DISABLED.", COLOR_ACCENT_CYAN);
            } else if (mouseX < vpX + 630) {
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
