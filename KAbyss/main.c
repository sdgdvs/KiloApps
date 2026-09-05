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

typedef struct {
    int x, y;
    int hp, max_hp;
    int sanity, max_sanity;
    int essence;
    int level;
    int exp, max_exp;
    int might;
    int warding;
    int arcana;
    int light_radius;
    int facing; // 0=Up, 1=Right, 2=Down, 3=Left
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
void ComputeFOV(void);
void AddLog(const char* text, COLORREF color);
void MovePlayer(int dx, int dy);
void RestTurn(void);
void SearchArea(void);
void InteractTile(void);
void CheckLevelUp(void);
void SpawnEmber(float x, float y, BOOL isTorch);
void UpdateEmbers(void);

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
        g_player.might += 2;
        g_player.warding += 1;
        g_player.arcana += 2;

        char buf[128];
        snprintf(buf, sizeof(buf), "LEVEL UP! Delver reached Level %d! (+15 Max HP, +2 Might, +2 Arcana)", g_player.level);
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
    g_embers[g_numEmbers].x = x + (float)(RandInt(0, 16) - 8);
    g_embers[g_numEmbers].y = y + (float)(RandInt(0, 12) - 6);
    g_embers[g_numEmbers].vx = ((float)RandInt(0, 100) - 50.0f) * 0.008f;
    g_embers[g_numEmbers].vy = -((float)RandInt(30, 80) * 0.015f);
    g_embers[g_numEmbers].life = 1.0f;
    g_embers[g_numEmbers].decay = 0.02f + ((float)RandInt(0, 50) * 0.0004f);
    g_embers[g_numEmbers].color = isTorch ? (RandInt(0, 10) > 4 ? COLOR_ACCENT_AMBER : COLOR_ACCENT_RED) : (RandInt(0, 10) > 5 ? COLOR_BORDER_GLOW : COLOR_TEXT_RUNE);
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

void InitGame(int depth) {
    g_depthLevel = depth;
    g_turn = 1;
    g_numTorches = 0;
    g_numChests = 0;
    g_numEmbers = 0;

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            g_dungeon[y][x] = TILE_WALL;
            g_explored[y][x] = FALSE;
            g_visible[y][x] = FALSE;
            g_lightMap[y][x] = 0.0f;
        }
    }

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

    // Connect rooms
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

    // Closed doors on room borders
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

    // Spawn player in first room
    if (roomCount > 0) {
        g_player.x = rooms[0].x + rooms[0].w / 2;
        g_player.y = rooms[0].y + rooms[0].h / 2;
        g_dungeon[g_player.y][g_player.x] = TILE_STAIRS_UP;

        // Spawn stairs down in last room
        int endIdx = roomCount - 1;
        int ex = rooms[endIdx].x + rooms[endIdx].w / 2;
        int ey = rooms[endIdx].y + rooms[endIdx].h / 2;
        g_dungeon[ey][ex] = TILE_STAIRS_DOWN;
    }

    // Spawn chests and rubble in middle rooms
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

    ComputeFOV();

    char buf[128];
    snprintf(buf, sizeof(buf), "Entered Catacombs Depth B%d. Shrouded in forgotten silence.", depth);
    AddLog(buf, COLOR_ACCENT_AMBER);
}

void AdvanceTurn(void) {
    g_turn++;
    if (g_turn % 40 == 0 && g_player.sanity > 10) {
        g_player.sanity -= 2;
        AddLog("Subterranean echoes fray your willpower (-2 Sanity).", COLOR_ACCENT_AMBER);
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
    if (tile == TILE_WALL || tile == TILE_PILLAR) {
        AddLog("Stone wall blocks your path.", COLOR_TEXT_DIM);
        Beep(180, 30);
        return;
    }

    if (tile == TILE_DOOR_CLOSED) {
        g_dungeon[ny][nx] = TILE_DOOR_OPEN;
        AddLog("You push open the heavy subterranean door.", COLOR_ACCENT_CYAN);
        Beep(350, 40);
        AdvanceTurn();
        return;
    }

    if (tile == TILE_CHEST) {
        for (int c = 0; c < g_numChests; c++) {
            if (g_chests[c].x == nx && g_chests[c].y == ny && !g_chests[c].opened) {
                g_chests[c].opened = 1;
                g_dungeon[ny][nx] = TILE_FLOOR;
                g_player.essence += g_chests[c].essence;
                g_player.exp += 20;

                char buf[128];
                snprintf(buf, sizeof(buf), "Opened Relic Chest! +%d Essence & +20 EXP!", g_chests[c].essence);
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
    Beep(240, 20);

    if (tile == TILE_STAIRS_DOWN) {
        AddLog("Spiraling descent deeper into Abyss. Press [E] to Descend.", COLOR_ACCENT_AMBER);
    } else if (tile == TILE_STAIRS_UP) {
        AddLog("The sealed stone portal back to surface remains shut.", COLOR_TEXT_DIM);
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
    AddLog("You steady your breath and rest (+2 HP, +1 Sanity).", COLOR_ACCENT_GREEN);
    Beep(440, 40);
    AdvanceTurn();
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
        char buf[128];
        snprintf(buf, sizeof(buf), "Descended into Depth B%d! The air thickens...", g_depthLevel);
        AddLog(buf, COLOR_ACCENT_AMBER);
        Beep(300, 60); Beep(450, 80);
        InitGame(g_depthLevel);
    } else if (cur == TILE_DOOR_CLOSED) {
        g_dungeon[g_player.y][g_player.x] = TILE_DOOR_OPEN;
        AddLog("Pushed open the door.", COLOR_ACCENT_CYAN);
        AdvanceTurn();
    } else {
        AddLog("Nothing to interact with here.", COLOR_TEXT_DIM);
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
    snprintf(badgeBuf, sizeof(badgeBuf), "DEPTH: B%d", g_depthLevel);
    SetTextColor(memDC, COLOR_ACCENT_PURPLE);
    TextOutA(memDC, 450, 11, badgeBuf, (int)strlen(badgeBuf));

    // HP Badge
    snprintf(badgeBuf, sizeof(badgeBuf), "HP: %d/%d", g_player.hp, g_player.max_hp);
    SetTextColor(memDC, COLOR_ACCENT_RED);
    TextOutA(memDC, 570, 11, badgeBuf, (int)strlen(badgeBuf));

    // Sanity Badge
    snprintf(badgeBuf, sizeof(badgeBuf), "SANITY: %d/%d", g_player.sanity, g_player.max_sanity);
    SetTextColor(memDC, COLOR_ACCENT_CYAN);
    TextOutA(memDC, 700, 11, badgeBuf, (int)strlen(badgeBuf));

    // Essence Badge
    snprintf(badgeBuf, sizeof(badgeBuf), "ESSENCE: %d*", g_player.essence);
    SetTextColor(memDC, COLOR_TEXT_GOLD);
    TextOutA(memDC, 870, 11, badgeBuf, (int)strlen(badgeBuf));

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
                int r = isVisible ? (int)(30 * (0.4f + light * 0.6f)) : 13;
                int g = isVisible ? (int)(41 * (0.4f + light * 0.6f)) : 19;
                int b = isVisible ? (int)(59 * (0.4f + light * 0.6f)) : 31;
                tileColor = RGB(r, g, b);
            } else if (tile == TILE_FLOOR || tile == TILE_RUBBLE) {
                int r = isVisible ? (int)(15 * (0.3f + light * 0.7f)) : 7;
                int g = isVisible ? (int)(23 * (0.3f + light * 0.7f)) : 10;
                int b = isVisible ? (int)(42 * (0.3f + light * 0.7f)) : 18;
                tileColor = RGB(r, g, b);
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
                HPEN brickPen = CreatePen(PS_SOLID, 1, RGB(15, 23, 42));
                HPEN oldBrPen = (HPEN)SelectObject(memDC, brickPen);
                MoveToEx(memDC, scrX + 4, scrY + 10, NULL);
                LineTo(memDC, scrX + TILE_SIZE - 4, scrY + 10);
                MoveToEx(memDC, scrX + 4, scrY + 22, NULL);
                LineTo(memDC, scrX + TILE_SIZE - 4, scrY + 22);
                SelectObject(memDC, oldBrPen);
                DeleteObject(brickPen);
            } else if (tile == TILE_PILLAR) {
                HBRUSH pBr = CreateSolidBrush(isVisible ? COLOR_TEXT_RUNE : RGB(49, 46, 129));
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
            HBRUSH sconceHalo = CreateSolidBrush(RGB(55, 32, 10));
            HBRUSH oldSc = (HBRUSH)SelectObject(memDC, sconceHalo);
            HPEN sconcePen = CreatePen(PS_SOLID, 1, RGB(180, 83, 9));
            HPEN oldScPen = (HPEN)SelectObject(memDC, sconcePen);
            Ellipse(memDC, scrX - 8 - tFlicker, scrY - 8 - tFlicker, scrX + TILE_SIZE + 8 + tFlicker, scrY + TILE_SIZE + 8 + tFlicker);
            SelectObject(memDC, oldScPen);
            DeleteObject(sconcePen);
            SelectObject(memDC, oldSc);
            DeleteObject(sconceHalo);

            SelectObject(memDC, fontBold);
            SetTextColor(memDC, COLOR_ACCENT_AMBER);
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
    SetTextColor(memDC, COLOR_TEXT_RUNE);
    char zoneText[64];
    snprintf(zoneText, sizeof(zoneText), "Forgotten Crypts - B%d", g_depthLevel);
    TextOutA(memDC, vpX + 18, vpY + 15, zoneText, (int)strlen(zoneText));

    SetTextColor(memDC, COLOR_TEXT_DIM);
    char turnText[64];
    snprintf(turnText, sizeof(turnText), "Turn: %d | Light: 100%% | Torch: Lit", g_turn);
    TextOutA(memDC, vpX + 18, vpY + 33, turnText, (int)strlen(turnText));

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
        TextOutA(memDC, sbX + 16, contentY + 62, "Sanity (Willpower)", 18);
        char sanTxt[32];
        snprintf(sanTxt, sizeof(sanTxt), "%d / %d", g_player.sanity, g_player.max_sanity);
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + sbW - 85, contentY + 62, sanTxt, (int)strlen(sanTxt));

        RECT sanBarBg = {sbX + 16, contentY + 78, sbX + sbW - 16, contentY + 84};
        FillRect(memDC, &sanBarBg, barDark);
        int sanW = (int)((float)(sbW - 32) * ((float)g_player.sanity / (float)g_player.max_sanity));
        if (sanW < 0) sanW = 0; if (sanW > sbW - 32) sanW = sbW - 32;
        RECT sanBarFill = {sbX + 16, contentY + 78, sbX + 16 + sanW, contentY + 84};
        HBRUSH sanFill = CreateSolidBrush(COLOR_BORDER_GLOW);
        FillRect(memDC, &sanBarFill, sanFill);
        DeleteObject(sanFill);

        // EXP Bar
        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 92, "Experience", 10);
        char expTxt[32];
        snprintf(expTxt, sizeof(expTxt), "%d / %d", g_player.exp, g_player.max_exp);
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + sbW - 85, contentY + 92, expTxt, (int)strlen(expTxt));

        RECT expBarBg = {sbX + 16, contentY + 108, sbX + sbW - 16, contentY + 114};
        FillRect(memDC, &expBarBg, barDark);
        DeleteObject(barDark);
        int expW = (int)((float)(sbW - 32) * ((float)g_player.exp / (float)g_player.max_exp));
        if (expW < 0) expW = 0; if (expW > sbW - 32) expW = sbW - 32;
        RECT expBarFill = {sbX + 16, contentY + 108, sbX + 16 + expW, contentY + 114};
        HBRUSH expFill = CreateSolidBrush(COLOR_ACCENT_PURPLE);
        FillRect(memDC, &expBarFill, expFill);
        DeleteObject(expFill);

        // Stats rows
        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 126, "Class:", 6);
        SetTextColor(memDC, COLOR_ACCENT_PURPLE);
        TextOutA(memDC, sbX + 130, contentY + 126, "Rune Knight", 11);

        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 144, "Might (Atk):", 12);
        char stBuf[32];
        snprintf(stBuf, sizeof(stBuf), "%d (+%d)", g_player.might, (g_player.might - 10) / 2);
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + 130, contentY + 144, stBuf, (int)strlen(stBuf));

        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 162, "Warding (Def):", 14);
        snprintf(stBuf, sizeof(stBuf), "%d (+%d)", g_player.warding, (g_player.warding - 10) / 2);
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + 130, contentY + 162, stBuf, (int)strlen(stBuf));

        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 180, "Arcana (Magic):", 15);
        snprintf(stBuf, sizeof(stBuf), "%d (+%d)", g_player.arcana, (g_player.arcana - 10) / 2);
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + 130, contentY + 180, stBuf, (int)strlen(stBuf));

        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 198, "Light Radius:", 13);
        snprintf(stBuf, sizeof(stBuf), "%d Tiles", g_player.light_radius);
        SetTextColor(memDC, COLOR_TEXT_GOLD);
        TextOutA(memDC, sbX + 130, contentY + 198, stBuf, (int)strlen(stBuf));

        // GEAR SECTION
        int gearY = contentY + 238;
        RECT gearCard = {sbX + 8, gearY, sbX + sbW - 8, gearY + 70};
        HBRUSH gCardBg = CreateSolidBrush(COLOR_BG_CARD);
        FillRect(memDC, &gearCard, gCardBg);
        DeleteObject(gCardBg);
        FrameRect(memDC, &gearCard, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

        SelectObject(memDC, fontBold);
        SetTextColor(memDC, COLOR_TEXT_RUNE);
        TextOutA(memDC, sbX + 16, gearY + 6, "EQUIPPED RELICS & GEAR", 22);

        SelectObject(memDC, fontSmall);
        SetTextColor(memDC, COLOR_TEXT_BRIGHT);
        TextOutA(memDC, sbX + 16, gearY + 26, "[Wpn] Runic Longsword (+4 Atk)", 30);
        TextOutA(memDC, sbX + 16, gearY + 44, "[Arm] Abyssal Mail (+3 Def)", 27);

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
        // RUNES
        RECT cardRect = {sbX + 8, contentY, sbX + sbW - 8, contentY + 308};
        HBRUSH cardBg = CreateSolidBrush(COLOR_BG_CARD);
        FillRect(memDC, &cardRect, cardBg);
        DeleteObject(cardBg);
        FrameRect(memDC, &cardRect, (HBRUSH)GetStockObject(DKGRAY_BRUSH));

        SelectObject(memDC, fontBold);
        SetTextColor(memDC, COLOR_TEXT_RUNE);
        TextOutA(memDC, sbX + 16, contentY + 8, "INSCRIBED ANCIENT RUNES", 23);

        SelectObject(memDC, fontBold);
        SetTextColor(memDC, COLOR_BORDER_GLOW);
        TextOutA(memDC, sbX + 16, contentY + 36, "* Rune of Flare", 15);
        SelectObject(memDC, fontSmall);
        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 54, "Illuminates crypts for 20 turns.", 32);

        SelectObject(memDC, fontBold);
        SetTextColor(memDC, COLOR_ACCENT_PURPLE);
        TextOutA(memDC, sbX + 16, contentY + 80, "* Rune of Aegis", 15);
        SelectObject(memDC, fontSmall);
        SetTextColor(memDC, COLOR_TEXT_DIM);
        TextOutA(memDC, sbX + 16, contentY + 98, "Absorbs 30 subterranean dmg.", 28);
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
        RECT modalRect = {width / 2 - 280, height / 2 - 180, width / 2 + 280, height / 2 + 180};
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
        TextOutA(memDC, modalRect.left + 20, my, "- WASD / Arrow Keys / Numpad / Vi: Navigate grid", 48); my += 20;
        TextOutA(memDC, modalRect.left + 20, my, "- Space: Rest 1 turn (Recuperates +2 HP, +1 Sanity)", 51); my += 20;
        TextOutA(memDC, modalRect.left + 20, my, "- R/X: Search surrounding area for secret coffers & traps", 57); my += 20;
        TextOutA(memDC, modalRect.left + 20, my, "- E / Enter: Interact / Descend into deeper abyss depths", 56); my += 20;
        TextOutA(memDC, modalRect.left + 20, my, "- 1, 2, 3: Switch Sidebar Tabs (Delver / Relics / Runes)", 56); my += 20;
        TextOutA(memDC, modalRect.left + 20, my, "- C: Toggle CRT Scanlines & Atmospheric Phosphor Grid", 53); my += 20;
        TextOutA(memDC, modalRect.left + 20, my, "- F: Toggle Field of View (FOV Omnivision)", 42); my += 20;
        TextOutA(memDC, modalRect.left + 20, my, "- N: Start New Descent", 22); my += 24;

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
        g_player.essence = 0;
        g_player.level = 1;
        g_player.exp = 0;
        g_player.max_exp = 100;
        g_player.might = 14;
        g_player.warding = 12;
        g_player.arcana = 16;
        g_player.light_radius = 7;
        g_player.facing = 2; // Down

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
        case 'X':
            SearchArea();
            break;

        // Toggle FOV
        case 'F':
        case 'V':
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

        // Check toolbar buttons
        int vpX = 12;
        if (mouseY >= 574 && mouseY <= 608 && mouseX >= vpX && mouseX <= vpX + VIEWPORT_W) {
            if (mouseX < vpX + 105) {
                // New Descent
                g_player.hp = g_player.max_hp;
                g_player.sanity = g_player.max_sanity;
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
