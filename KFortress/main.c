#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

int _fltused = 1;

float custom_sqrtf(float val) {
    if (val <= 0.0f) return 0.0f;
    float guess = val / 2.0f;
    for (int i = 0; i < 8; i++) {
        guess = 0.5f * (guess + val / guess);
    }
    return guess;
}

float custom_sinf(float x) {
    while (x > 3.14159265f) x -= 6.2831853f;
    while (x < -3.14159265f) x += 6.2831853f;
    float x2 = x * x;
    return x * (1.0f - x2 * (1.0f / 6.0f - x2 * (1.0f / 120.0f - x2 / 5040.0f)));
}

float custom_cosf(float x) {
    return custom_sinf(x + 1.57079632f);
}

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 700
#define TIMER_ID 1
#define TIMER_INTERVAL 33 // ~30 FPS

// Colors
#define BG_COLOR RGB(10, 12, 16)
#define CARD_BG RGB(30, 34, 42)
#define BORDER_COLOR RGB(74, 85, 104)
#define TEXT_GOLD RGB(251, 191, 36)
#define TEXT_RED RGB(244, 63, 94)
#define TEXT_WHITE RGB(226, 232, 240)
#define TEXT_MUTED RGB(148, 163, 184)
#define PATH_COLOR RGB(26, 32, 44)
#define CASTLE_COLOR RGB(71, 85, 105)
#define TOWER_SLOT_BG RGB(44, 50, 62)
#define TOWER_SLOT_HOVER RGB(60, 68, 82)

#define MAX_SLOTS 14
#define MAX_ENEMIES 90
#define MAX_PROJECTILES 100
#define MAX_FLOATING_TEXTS 50
#define MAX_TRAPS 32
#define MAX_WAYPOINTS 6
#define MAX_MAPS 12
#define MAX_MILITIA 8
#define MAX_LAVA_POOLS 24

typedef struct {
    int x, y;
} Point;

typedef struct {
    int x, y;
    char type[4];
} Obstacle;

typedef struct {
    char name[32];
    COLORREF bg;
    COLORREF path;
    Point waypoints[MAX_WAYPOINTS];
    int numSlots;
    Point slots[MAX_SLOTS];
    int numObs;
    Obstacle obs[3];
} MapDef;

static MapDef g_maps[MAX_MAPS];
static int g_currentMap = 0;

// Tower Types
#define TOWER_ARCHER 1
#define TOWER_MAGE 2
#define TOWER_CANNON 3
#define TOWER_FROST 4
#define TOWER_TESLA 5
#define TOWER_BALLISTA 6
#define TOWER_POISON 7
#define TOWER_INFERNO 8
#define TOWER_SUPERCONDUCTOR 9
#define TOWER_VENOMSPITE 10
#define TOWER_SOLAR_BEAM 11

#define TRAP_SPIKE 12
#define TRAP_OIL 13
#define TRAP_BARRICADE 14
#define TRAP_DYNAMITE 15

typedef struct {
    int x, y;
    BOOL occupied;
    int towerType;
    int level; // 1-3, 4=FUSION
    int cooldown;
    int maxCooldown;
    int range;
    int damage;
    int splash;
    int attackAnim;
    int beamTargetId;
} TowerSlot;

#define MAX_PARTICLES 450
typedef struct {
    BOOL active;
    float x, y;
    float vx, vy;
    float drag;
    float gravity;
    float rot;
    float rotSpd;
    float growth;
    COLORREF color;
    int life;
    int initialLife;
    int type; // 0=default, 1=needle spark, 2=smoke puff, 3=tumbling shard, 4=celebration star
    float size;
} Particle;
static Particle g_particles[MAX_PARTICLES];

#define MAX_SHOCKWAVES 32
typedef struct {
    BOOL active;
    float x, y;
    float r;
    float maxR;
    COLORREF color;
    int life;
    int maxLife;
    int width;
} Shockwave;
static Shockwave g_shockwaves[MAX_SHOCKWAVES];

static float g_shakeIntensity = 0.0f;
static float g_shakeAngle = 0.0f;
static int g_globalFrame = 0;

#define MAX_SCORCH_MARKS 64
typedef struct {
    float x, y;
    float radius;
    BOOL active;
    COLORREF color;
} ScorchMark;
static ScorchMark g_scorchMarks[MAX_SCORCH_MARKS];
static int g_scorchIndex = 0;

typedef struct {
    BOOL active;
    float x, y;
    int radius;
    int life;
    int maxLife;
    int damage;
} LavaPool;
static LavaPool g_lavaPools[MAX_LAVA_POOLS];

void SpawnAdvancedParticle(float x, float y, float vx, float vy, COLORREF color, int life, int type, float size) {
    for (int p = 0; p < MAX_PARTICLES; p++) {
        if (!g_particles[p].active) {
            g_particles[p].active = TRUE;
            g_particles[p].x = x; g_particles[p].y = y;
            g_particles[p].vx = vx; g_particles[p].vy = vy;
            g_particles[p].drag = 0.96f;
            g_particles[p].gravity = 0.05f;
            g_particles[p].rot = 0.0f;
            g_particles[p].rotSpd = 0.0f;
            g_particles[p].growth = 0.0f;
            g_particles[p].color = color;
            g_particles[p].life = life;
            g_particles[p].initialLife = life;
            g_particles[p].type = type;
            g_particles[p].size = size;
            break;
        }
    }
}

void SpawnParticleBurst(float x, float y, COLORREF color, int count) {
    for (int i = 0; i < count; i++) {
        float angle = (float)(rand() % 628) / 100.0f;
        float spd = 1.5f + (rand() % 35) / 10.0f;
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (!g_particles[p].active) {
                g_particles[p].active = TRUE;
                g_particles[p].x = x; g_particles[p].y = y;
                g_particles[p].vx = custom_cosf(angle) * spd;
                g_particles[p].vy = custom_sinf(angle) * spd;
                g_particles[p].drag = 0.94f;
                g_particles[p].gravity = 0.04f;
                g_particles[p].rot = 0.0f; g_particles[p].rotSpd = 0.0f; g_particles[p].growth = 0.0f;
                g_particles[p].color = color;
                g_particles[p].life = 15 + (rand() % 12);
                g_particles[p].initialLife = g_particles[p].life;
                g_particles[p].type = 1;
                g_particles[p].size = 2.5f;
                break;
            }
        }
    }
}

void SpawnExplosion(float x, float y, COLORREF color) {
    g_shakeIntensity = min(32.0f, g_shakeIntensity + 14.0f);
    
    // Dual-tier concentric shockwaves
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (!g_shockwaves[i].active) {
            g_shockwaves[i].active = TRUE;
            g_shockwaves[i].x = x; g_shockwaves[i].y = y;
            g_shockwaves[i].r = 6.0f; g_shockwaves[i].maxR = 75.0f;
            g_shockwaves[i].color = color;
            g_shockwaves[i].life = 20; g_shockwaves[i].maxLife = 20;
            g_shockwaves[i].width = 3;
            break;
        }
    }
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (!g_shockwaves[i].active) {
            g_shockwaves[i].active = TRUE;
            g_shockwaves[i].x = x; g_shockwaves[i].y = y;
            g_shockwaves[i].r = 3.0f; g_shockwaves[i].maxR = 110.0f;
            g_shockwaves[i].color = RGB(255, 255, 255);
            g_shockwaves[i].life = 28; g_shockwaves[i].maxLife = 28;
            g_shockwaves[i].width = 1;
            break;
        }
    }

    // Layer 1: Incandescent Core Needle Sparks
    for (int i = 0; i < 18; i++) {
        float angle = (float)(rand() % 628) / 100.0f;
        float spd = 4.0f + (rand() % 60) / 10.0f;
        COLORREF spkCol = (rand() % 10 < 4) ? TEXT_GOLD : ((rand() % 10 < 5) ? RGB(255,255,255) : color);
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (!g_particles[p].active) {
                g_particles[p].active = TRUE;
                g_particles[p].x = x; g_particles[p].y = y;
                g_particles[p].vx = custom_cosf(angle) * spd;
                g_particles[p].vy = custom_sinf(angle) * spd;
                g_particles[p].drag = 0.92f;
                g_particles[p].gravity = 0.08f;
                g_particles[p].color = spkCol;
                g_particles[p].life = 20 + rand() % 15;
                g_particles[p].initialLife = g_particles[p].life;
                g_particles[p].type = 1;
                g_particles[p].size = 2.5f;
                g_particles[p].rot = 0; g_particles[p].rotSpd = 0; g_particles[p].growth = 0;
                break;
            }
        }
    }

    // Layer 2: Expanding Buoyant Smoke Puffs
    for (int i = 0; i < 12; i++) {
        float angle = (float)(rand() % 628) / 100.0f;
        float spd = 0.8f + (rand() % 20) / 10.0f;
        int gray = 70 + rand() % 70;
        COLORREF smkCol = (rand() % 10 < 4) ? color : RGB(gray, gray, gray);
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (!g_particles[p].active) {
                g_particles[p].active = TRUE;
                g_particles[p].x = x + (rand() % 10) - 5; g_particles[p].y = y + (rand() % 10) - 5;
                g_particles[p].vx = custom_cosf(angle) * spd;
                g_particles[p].vy = custom_sinf(angle) * spd - 0.5f;
                g_particles[p].drag = 0.95f;
                g_particles[p].gravity = -0.04f;
                g_particles[p].color = smkCol;
                g_particles[p].life = 28 + rand() % 18;
                g_particles[p].initialLife = g_particles[p].life;
                g_particles[p].type = 2;
                g_particles[p].size = 6.0f;
                g_particles[p].growth = 0.3f;
                g_particles[p].rot = 0; g_particles[p].rotSpd = 0;
                break;
            }
        }
    }

    // Layer 3: Heavy Kinematic Shards
    for (int i = 0; i < 10; i++) {
        float angle = -2.5f + (float)(rand() % 200) / 100.0f;
        float spd = 3.0f + (rand() % 50) / 10.0f;
        COLORREF shdCol = (rand() % 2 == 0) ? RGB(100, 116, 139) : RGB(146, 64, 14);
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (!g_particles[p].active) {
                g_particles[p].active = TRUE;
                g_particles[p].x = x; g_particles[p].y = y;
                g_particles[p].vx = custom_cosf(angle) * spd + (rand()%20 - 10)/10.0f;
                g_particles[p].vy = custom_sinf(angle) * spd;
                g_particles[p].drag = 0.97f;
                g_particles[p].gravity = 0.3f;
                g_particles[p].color = shdCol;
                g_particles[p].life = 30 + rand() % 20;
                g_particles[p].initialLife = g_particles[p].life;
                g_particles[p].type = 3;
                g_particles[p].size = 4.0f + rand() % 4;
                g_particles[p].rot = (float)(rand() % 628) / 100.0f;
                g_particles[p].rotSpd = (float)(rand() % 40 - 20) / 100.0f;
                g_particles[p].growth = 0;
                break;
            }
        }
    }

    // Layer 4: Radiant Celebration Stars
    for (int i = 0; i < 8; i++) {
        float angle = ((float)i / 8.0f) * 6.283f;
        float spd = 2.5f + (rand() % 30) / 10.0f;
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (!g_particles[p].active) {
                g_particles[p].active = TRUE;
                g_particles[p].x = x; g_particles[p].y = y;
                g_particles[p].vx = custom_cosf(angle) * spd;
                g_particles[p].vy = custom_sinf(angle) * spd;
                g_particles[p].drag = 0.94f;
                g_particles[p].gravity = 0.02f;
                g_particles[p].color = TEXT_GOLD;
                g_particles[p].life = 22 + rand() % 14;
                g_particles[p].initialLife = g_particles[p].life;
                g_particles[p].type = 4;
                g_particles[p].size = 3.5f;
                g_particles[p].rot = 0; g_particles[p].rotSpd = 0.15f; g_particles[p].growth = 0;
                break;
            }
        }
    }
}

typedef struct {
    BOOL active;
    float x, y;
    int hp;
    int maxHp;
    float speed;
    int waypointIndex;
    int id;
    BOOL slowed;
    int poisonTicks;
    int poisonDmg;
    int summonTimer;
    int type;
    int radius;
} Enemy;

typedef struct {
    BOOL active;
    float x, y;
    int targetEnemyId;
    float targetX, targetY;
    float speed;
    int damage;
    int type;
    int splash;
    BOOL isCrit;
    BOOL isPiercing;
    float vx, vy;
    int lifeTimer;
} Projectile;

typedef struct {
    BOOL active;
    float x, y;
    char text[32];
    COLORREF color;
    int life;
} FloatingText;

typedef struct {
    BOOL active;
    float x, y;
    int type;
    int charges;
    float hp;
    int radius;
} Trap;

typedef struct {
    BOOL active;
    float x, y;
    float targetX, targetY;
    float hp, maxHp;
    float speed;
    int damage;
    int attackCd;
    int lifeTimer;
} Militia;
static Militia g_militia[MAX_MILITIA];

// Global State
static TowerSlot g_slots[MAX_SLOTS];
static int g_slotCount = 12;
static int g_gold = 100;
static int g_baseHp = 20;
static int g_maxBaseHp = 20;
static int g_wave = 1;
static BOOL g_waveActive = FALSE;
static BOOL g_gameOver = FALSE;
static int g_selectedSlot = -1;

static int g_gameMode = 0; // 0=Campaign, 1=Endless, 2=BossBlitz
static int g_bossesKilled = 0;
static int g_hsEndless = 0;
static int g_hsBoss = 0;

// Mutators
#define MUTATOR_BLOODLUST   0x01
#define MUTATOR_TITAN       0x02
#define MUTATOR_ECLIPSE     0x04
#define MUTATOR_METEOR      0x08
#define MUTATOR_PHASE_SHIFT 0x10
static int g_mutators = 0;
static BOOL g_showMutators = FALSE;

// Castle Siege Weapons
static int g_trebuchetCd = 0;
static int g_maxTrebuchetCd = 360;
static int g_castleBallistaCd = 0;
static int g_meteorTimer = 0;

// Research Academy Techs
int g_techStartingGold = 0;
int g_techWallHp = 0;
int g_techHeroCd = 0;
int g_techTowerDmg = 0;
int g_techMilitia = 0;
int g_techSiegeEng = 0;
int g_techFusion = 0;
int g_techFortTraps = 0;

BOOL g_showAcademy = FALSE;
BOOL g_showHelp = FALSE;

static char g_toastText[128] = "Press [F1] or [H] for Guide | [Space] Wave | [1-5] Skills | [A] Academy | [M] Mutators | [Esc] Close";
static int g_toastTimer = 220;
static COLORREF g_toastColor = RGB(251, 191, 36);

void ShowNativeToast(const char* txt, COLORREF col, int durationFrames) {
    lstrcpynA(g_toastText, txt, sizeof(g_toastText));
    g_toastColor = col;
    g_toastTimer = durationFrames;
}

typedef struct {
    float x, y;
    float targetX, targetY;
    float hp, maxHp;
    float speed;
    int damage;
    int range;
    int attackCd;
    int maxAttackCd;
    int respawnTimer;
    int healCd, maxHealCd;
    int shieldCd, maxShieldCd;
    int meteorCd, maxMeteorCd;
    int summonCd, maxSummonCd;
    int shieldActive;
} Hero;

static Hero g_hero;

static int g_selectedTowerTypeToBuild = TOWER_ARCHER;

#define MAX_SPAWN_QUEUE 140
static int g_spawnQueue[MAX_SPAWN_QUEUE];
static int g_spawnQueueCount = 0;
static int g_spawnQueueHead = 0;
static int g_spawnTimer = 0;

#define ENEMY_GOBLIN 1
#define ENEMY_ORC 2
#define ENEMY_HOUND 3
#define ENEMY_GARGOYLE 4
#define ENEMY_OGRE 5
#define ENEMY_NECROMANCER 6
#define ENEMY_SKELETON 7
#define ENEMY_WYVERN 8
#define ENEMY_GOLEM 9

static Enemy g_enemies[MAX_ENEMIES];
static Projectile g_projectiles[MAX_PROJECTILES];
static FloatingText g_floatingTexts[MAX_FLOATING_TEXTS];
static Trap g_traps[MAX_TRAPS];
static int g_blizzTimer = 0;

static Point g_waypoints[MAX_WAYPOINTS];
static int g_nextEnemyId = 1;

typedef struct {
    float x, y;
    int type;
    float size;
} EnvArt;
static EnvArt g_envArt[150];
static int g_envArtCount = 0;

typedef struct {
    BOOL active;
    float x, y;
    float vx, vy;
    COLORREF color;
    float size;
    int type;
} WeatherParticle;
static WeatherParticle g_weatherParticles[200];

void InitBiomeWeather(int mapIdx) {
    COLORREF biomeCols[] = {
        RGB(74, 222, 128),  // Forest
        RGB(253, 224, 71),  // Desert
        RGB(224, 242, 254), // Frozen
        RGB(249, 115, 22),  // Volcanic
        RGB(168, 85, 247),  // Swamp
        RGB(192, 132, 252), // Crystal
        RGB(148, 163, 184), // Haunted
        RGB(255, 255, 255), // Sky
        RGB(239, 68, 68),   // Dragon
        RGB(129, 140, 248), // Void
        RGB(56, 189, 248),  // Thunder
        RGB(216, 180, 254)  // Eldritch
    };
    COLORREF col = biomeCols[mapIdx % 12];
    for (int p = 0; p < 80; p++) {
        g_weatherParticles[p].active = TRUE;
        g_weatherParticles[p].x = (float)(rand() % 800);
        g_weatherParticles[p].y = (float)(rand() % 560);
        g_weatherParticles[p].vx = 0.5f + (rand() % 15) / 10.0f;
        g_weatherParticles[p].vy = (mapIdx == 3 || mapIdx == 8) ? -(0.8f + (rand() % 15) / 10.0f) : (0.6f + (rand() % 15) / 10.0f);
        g_weatherParticles[p].color = col;
        g_weatherParticles[p].size = 2.0f + (rand() % 3);
        g_weatherParticles[p].type = mapIdx % 12;
    }
    for (int p = 80; p < 200; p++) {
        g_weatherParticles[p].active = FALSE;
    }
}

void DrawFiligreeLBracket(HDC hdc, int cx, int cy, int size, int orientation) {
    HPEN goldPen = CreatePen(PS_SOLID, 2, TEXT_GOLD);
    HPEN oldP = (HPEN)SelectObject(hdc, goldPen);
    
    int dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
    if (orientation == 0) { // Top-Left
        dx1 = size; dy1 = 0; dx2 = 0; dy2 = size;
    } else if (orientation == 1) { // Top-Right
        dx1 = -size; dy1 = 0; dx2 = 0; dy2 = size;
    } else if (orientation == 2) { // Bottom-Right
        dx1 = -size; dy1 = 0; dx2 = 0; dy2 = -size;
    } else { // Bottom-Left
        dx1 = size; dy1 = 0; dx2 = 0; dy2 = -size;
    }
    
    // Outer bracket
    MoveToEx(hdc, cx + dx1, cy, NULL);
    LineTo(hdc, cx, cy);
    LineTo(hdc, cx, cy + dy2);
    
    // Inner tier line
    int inOffset = (orientation == 0 || orientation == 3) ? 4 : -4;
    int inOffsetY = (orientation == 0 || orientation == 1) ? 4 : -4;
    MoveToEx(hdc, cx + dx1/2 + inOffset, cy + inOffsetY, NULL);
    LineTo(hdc, cx + inOffset, cy + inOffsetY);
    LineTo(hdc, cx + inOffset, cy + dy2/2 + inOffsetY);
    
    // Diode / Rivet dot
    HBRUSH goldBrush = CreateSolidBrush(TEXT_GOLD);
    HBRUSH oldB = (HBRUSH)SelectObject(hdc, goldBrush);
    int rX = cx + inOffset * 2;
    int rY = cy + inOffsetY * 2;
    Ellipse(hdc, rX - 2, rY - 2, rX + 3, rY + 3);
    
    SelectObject(hdc, oldB);
    SelectObject(hdc, oldP);
    DeleteObject(goldBrush);
    DeleteObject(goldPen);
}

void AddFloatingText(float x, float y, const char* txt, COLORREF color) {
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        if (!g_floatingTexts[i].active) {
            g_floatingTexts[i].active = TRUE;
            g_floatingTexts[i].x = x;
            g_floatingTexts[i].y = y;
            g_floatingTexts[i].color = color;
            g_floatingTexts[i].life = 25;
            lstrcpynA(g_floatingTexts[i].text, txt, 32);
            break;
        }
    }
}

void SpawnLavaPool(float x, float y, int radius, int life, int dmg) {
    for (int i = 0; i < MAX_LAVA_POOLS; i++) {
        if (!g_lavaPools[i].active) {
            g_lavaPools[i].active = TRUE;
            g_lavaPools[i].x = x;
            g_lavaPools[i].y = y;
            g_lavaPools[i].radius = radius;
            g_lavaPools[i].life = life;
            g_lavaPools[i].maxLife = life;
            g_lavaPools[i].damage = dmg;
            break;
        }
    }
}

void InitMaps() {
    lstrcpyA(g_maps[0].name, "Forest Outpost"); g_maps[0].bg = RGB(10, 26, 10); g_maps[0].path = RGB(45, 55, 45);
    Point wp0[] = {{40,200}, {330,200}, {330,390}, {520,390}, {520,220}, {730,220}};
    for(int i=0;i<6;i++) g_maps[0].waypoints[i] = wp0[i];
    Point sl0[] = {{140,110},{270,110},{400,110},{540,110},{140,300},{270,300},{400,300},{540,300},{140,470},{270,470},{400,470},{540,470}};
    g_maps[0].numSlots = 12; for(int i=0;i<12;i++) g_maps[0].slots[i] = sl0[i];
    g_maps[0].numObs = 3; g_maps[0].obs[0]=(Obstacle){100,380,"T"}; g_maps[0].obs[1]=(Obstacle){450,150,"T"}; g_maps[0].obs[2]=(Obstacle){600,400,"T"};

    lstrcpyA(g_maps[1].name, "Desert Pass"); g_maps[1].bg = RGB(42, 28, 10); g_maps[1].path = RGB(74, 53, 24);
    Point wp1[] = {{40,100}, {200,100}, {200,450}, {600,450}, {600,200}, {730,200}};
    for(int i=0;i<6;i++) g_maps[1].waypoints[i] = wp1[i];
    Point sl1[] = {{120,180},{280,180},{280,380},{500,380},{500,280},{680,280},{120,300}};
    g_maps[1].numSlots = 7; for(int i=0;i<7;i++) g_maps[1].slots[i] = sl1[i];
    g_maps[1].numObs = 3; g_maps[1].obs[0]=(Obstacle){350,250,"C"}; g_maps[1].obs[1]=(Obstacle){450,120,"C"}; g_maps[1].obs[2]=(Obstacle){650,350,"R"};

    lstrcpyA(g_maps[2].name, "Frozen Fortress"); g_maps[2].bg = RGB(10, 21, 42); g_maps[2].path = RGB(31, 59, 90);
    Point wp2[] = {{40,450}, {400,450}, {400,150}, {600,150}, {600,350}, {730,350}};
    for(int i=0;i<6;i++) g_maps[2].waypoints[i] = wp2[i];
    Point sl2[] = {{150,350},{300,350},{280,250},{480,250},{500,150},{500,450}};
    g_maps[2].numSlots = 6; for(int i=0;i<6;i++) g_maps[2].slots[i] = sl2[i];
    g_maps[2].numObs = 3; g_maps[2].obs[0]=(Obstacle){200,200,"I"}; g_maps[2].obs[1]=(Obstacle){500,80,"S"}; g_maps[2].obs[2]=(Obstacle){150,150,"M"};

    lstrcpyA(g_maps[3].name, "Volcanic Citadel"); g_maps[3].bg = RGB(42, 10, 10); g_maps[3].path = RGB(74, 28, 28);
    Point wp3[] = {{40,150}, {150,150}, {150,400}, {500,400}, {500,150}, {730,150}};
    for(int i=0;i<6;i++) g_maps[3].waypoints[i] = wp3[i];
    Point sl3[] = {{250,250},{350,250},{250,320},{350,320},{600,250},{600,350}};
    g_maps[3].numSlots = 6; for(int i=0;i<6;i++) g_maps[3].slots[i] = sl3[i];
    g_maps[3].numObs = 3; g_maps[3].obs[0]=(Obstacle){300,200,"V"}; g_maps[3].obs[1]=(Obstacle){650,400,"F"}; g_maps[3].obs[2]=(Obstacle){100,300,"F"};

    lstrcpyA(g_maps[4].name, "Swamp of Sorrows"); g_maps[4].bg = RGB(21, 42, 21); g_maps[4].path = RGB(44, 62, 44);
    Point wp4[] = {{40,300}, {250,300}, {250,150}, {600,150}, {600,450}, {730,450}};
    for(int i=0;i<6;i++) g_maps[4].waypoints[i] = wp4[i];
    Point sl4[] = {{150,200},{150,400},{350,250},{450,250},{500,350},{700,350}};
    g_maps[4].numSlots = 6; for(int i=0;i<6;i++) g_maps[4].slots[i] = sl4[i];
    g_maps[4].numObs = 3; g_maps[4].obs[0]=(Obstacle){200,400,"M"}; g_maps[4].obs[1]=(Obstacle){400,100,"S"}; g_maps[4].obs[2]=(Obstacle){500,250,"W"};

    lstrcpyA(g_maps[5].name, "Crystal Caves"); g_maps[5].bg = RGB(26, 10, 42); g_maps[5].path = RGB(53, 31, 74);
    Point wp5[] = {{40,400}, {200,400}, {200,200}, {450,200}, {450,350}, {730,350}};
    for(int i=0;i<6;i++) g_maps[5].waypoints[i] = wp5[i];
    Point sl5[] = {{100,300},{300,300},{300,100},{550,150},{550,450},{650,250}};
    g_maps[5].numSlots = 6; for(int i=0;i<6;i++) g_maps[5].slots[i] = sl5[i];
    g_maps[5].numObs = 3; g_maps[5].obs[0]=(Obstacle){150,150,"C"}; g_maps[5].obs[1]=(Obstacle){350,450,"C"}; g_maps[5].obs[2]=(Obstacle){600,100,"C"};

    lstrcpyA(g_maps[6].name, "Haunted Graveyard"); g_maps[6].bg = RGB(10, 12, 16); g_maps[6].path = RGB(31, 41, 55);
    Point wp6[] = {{40,250}, {150,250}, {150,100}, {550,100}, {550,300}, {730,300}};
    for(int i=0;i<6;i++) g_maps[6].waypoints[i] = wp6[i];
    Point sl6[] = {{250,180},{350,180},{450,180},{250,280},{350,280},{450,280}};
    g_maps[6].numSlots = 6; for(int i=0;i<6;i++) g_maps[6].slots[i] = sl6[i];
    g_maps[6].numObs = 3; g_maps[6].obs[0]=(Obstacle){200,400,"G"}; g_maps[6].obs[1]=(Obstacle){400,350,"G"}; g_maps[6].obs[2]=(Obstacle){650,150,"X"};

    lstrcpyA(g_maps[7].name, "Sky Kingdom"); g_maps[7].bg = RGB(10, 37, 58); g_maps[7].path = RGB(47, 90, 122);
    Point wp7[] = {{40,100}, {300,100}, {300,450}, {600,450}, {600,250}, {730,250}};
    for(int i=0;i<6;i++) g_maps[7].waypoints[i] = wp7[i];
    Point sl7[] = {{150,180},{200,280},{400,350},{500,350},{500,150},{700,350}};
    g_maps[7].numSlots = 6; for(int i=0;i<6;i++) g_maps[7].slots[i] = sl7[i];
    g_maps[7].numObs = 3; g_maps[7].obs[0]=(Obstacle){100,400,"W"}; g_maps[7].obs[1]=(Obstacle){450,150,"W"}; g_maps[7].obs[2]=(Obstacle){650,100,"W"};

    lstrcpyA(g_maps[8].name, "Dragon's Peak"); g_maps[8].bg = RGB(42, 16, 10); g_maps[8].path = RGB(74, 44, 31);
    Point wp8[] = {{40,350}, {350,350}, {350,150}, {550,150}, {550,400}, {730,400}};
    for(int i=0;i<6;i++) g_maps[8].waypoints[i] = wp8[i];
    Point sl8[] = {{200,250},{300,250},{450,250},{450,350},{650,250},{650,150}};
    g_maps[8].numSlots = 6; for(int i=0;i<6;i++) g_maps[8].slots[i] = sl8[i];
    g_maps[8].numObs = 3; g_maps[8].obs[0]=(Obstacle){150,150,"D"}; g_maps[8].obs[1]=(Obstacle){250,450,"F"}; g_maps[8].obs[2]=(Obstacle){500,80,"V"};

    lstrcpyA(g_maps[9].name, "The Void Abyss"); g_maps[9].bg = RGB(5, 5, 16); g_maps[9].path = RGB(31, 31, 58);
    Point wp9[] = {{40,200}, {150,400}, {350,150}, {550,450}, {650,250}, {730,250}};
    for(int i=0;i<6;i++) g_maps[9].waypoints[i] = wp9[i];
    Point sl9[] = {{150,250},{250,280},{350,300},{450,250},{550,250},{600,150}};
    g_maps[9].numSlots = 6; for(int i=0;i<6;i++) g_maps[9].slots[i] = sl9[i];
    g_maps[9].numObs = 3; g_maps[9].obs[0]=(Obstacle){100,100,"X"}; g_maps[9].obs[1]=(Obstacle){450,100,"X"}; g_maps[9].obs[2]=(Obstacle){300,400,"X"};

    lstrcpyA(g_maps[10].name, "Thunder Peak"); g_maps[10].bg = RGB(11, 26, 46); g_maps[10].path = RGB(58, 80, 107);
    Point wp10[] = {{40,150}, {250,150}, {250,420}, {480,420}, {480,180}, {730,180}};
    for(int i=0;i<6;i++) g_maps[10].waypoints[i] = wp10[i];
    Point sl10[] = {{150,230},{150,350},{360,300},{360,120},{580,280},{580,100},{360,480}};
    g_maps[10].numSlots = 7; for(int i=0;i<7;i++) g_maps[10].slots[i] = sl10[i];
    g_maps[10].numObs = 3; g_maps[10].obs[0]=(Obstacle){180,80,"E"}; g_maps[10].obs[1]=(Obstacle){400,250,"E"}; g_maps[10].obs[2]=(Obstacle){620,400,"L"};

    lstrcpyA(g_maps[11].name, "Eldritch Necropolis"); g_maps[11].bg = RGB(18, 9, 28); g_maps[11].path = RGB(46, 27, 64);
    Point wp11[] = {{40,380}, {200,380}, {200,120}, {520,120}, {520,360}, {730,360}};
    for(int i=0;i<6;i++) g_maps[11].waypoints[i] = wp11[i];
    Point sl11[] = {{100,260},{280,260},{360,180},{440,260},{600,240},{600,450},{360,450}};
    g_maps[11].numSlots = 7; for(int i=0;i<7;i++) g_maps[11].slots[i] = sl11[i];
    g_maps[11].numObs = 3; g_maps[11].obs[0]=(Obstacle){120,100,"K"}; g_maps[11].obs[1]=(Obstacle){360,320,"C"}; g_maps[11].obs[2]=(Obstacle){650,120,"P"};
}

void LoadCurrentMap(int bfX, int bfY, int bfW) {
    MapDef *m = &g_maps[g_currentMap];
    for (int i=0; i<6; i++) {
        g_waypoints[i].x = bfX + m->waypoints[i].x - 10;
        g_waypoints[i].y = bfY + m->waypoints[i].y - 20;
    }
    g_slotCount = m->numSlots;
    for (int i=0; i<m->numSlots; i++) {
        g_slots[i].x = bfX + m->slots[i].x - 10;
        g_slots[i].y = bfY + m->slots[i].y - 20;
        g_slots[i].occupied = FALSE;
        g_slots[i].towerType = 0;
        g_slots[i].level = 1;
        g_slots[i].cooldown = 0;
        g_slots[i].maxCooldown = 18;
        g_slots[i].range = 130;
        g_slots[i].damage = 12;
        g_slots[i].splash = 0;
        g_slots[i].attackAnim = 0;
        g_slots[i].beamTargetId = -1;
    }

    g_envArtCount = 0;
    for (int i = 0; i < 5; i++) {
        int x1 = g_waypoints[i].x;
        int y1 = g_waypoints[i].y;
        int x2 = g_waypoints[i+1].x;
        int y2 = g_waypoints[i+1].y;
        float dx = (float)(x2 - x1);
        float dy = (float)(y2 - y1);
        float dist = custom_sqrtf(dx*dx + dy*dy);
        int steps = (int)(dist / 20.0f);
        for(int j = 0; j < steps && g_envArtCount < 150; j++) {
            float t = (float)j / steps;
            float cx = x1 + dx * t;
            float cy = y1 + dy * t;
            float nx = -dy / dist;
            float ny = dx / dist;
            float side = (rand() % 2 == 0) ? 1.0f : -1.0f;
            float offset = 35.0f + (rand() % 20);
            if(rand() % 100 > 30) {
                g_envArt[g_envArtCount].x = cx + nx * side * offset;
                g_envArt[g_envArtCount].y = cy + ny * side * offset;
                g_envArt[g_envArtCount].type = (rand() % 2);
                g_envArt[g_envArtCount].size = 6.0f + (rand() % 10);
                g_envArtCount++;
            }
        }
    }
    InitBiomeWeather(g_currentMap);
}

void LoadGame() {
    FILE *f = fopen("kfortress.dat", "rb");
    if (f) {
        fread(&g_techStartingGold, sizeof(int), 1, f);
        fread(&g_techWallHp, sizeof(int), 1, f);
        fread(&g_techHeroCd, sizeof(int), 1, f);
        fread(&g_techTowerDmg, sizeof(int), 1, f);
        fread(&g_techMilitia, sizeof(int), 1, f);
        fread(&g_hsEndless, sizeof(int), 1, f);
        fread(&g_hsBoss, sizeof(int), 1, f);
        fread(&g_techSiegeEng, sizeof(int), 1, f);
        fread(&g_techFusion, sizeof(int), 1, f);
        fread(&g_techFortTraps, sizeof(int), 1, f);
        fread(&g_mutators, sizeof(int), 1, f);
        fclose(f);
    }
}

void SaveGame() {
    FILE *f = fopen("kfortress.dat", "wb");
    if (f) {
        fwrite(&g_techStartingGold, sizeof(int), 1, f);
        fwrite(&g_techWallHp, sizeof(int), 1, f);
        fwrite(&g_techHeroCd, sizeof(int), 1, f);
        fwrite(&g_techTowerDmg, sizeof(int), 1, f);
        fwrite(&g_techMilitia, sizeof(int), 1, f);
        fwrite(&g_hsEndless, sizeof(int), 1, f);
        fwrite(&g_hsBoss, sizeof(int), 1, f);
        fwrite(&g_techSiegeEng, sizeof(int), 1, f);
        fwrite(&g_techFusion, sizeof(int), 1, f);
        fwrite(&g_techFortTraps, sizeof(int), 1, f);
        fwrite(&g_mutators, sizeof(int), 1, f);
        fclose(f);
    }
}

void InitGameState() {
    LoadGame();
    InitMaps();
    LoadCurrentMap(10, 70, WINDOW_WIDTH - 220);

    g_gold = 100 + (g_techStartingGold * 50);
    if (g_gameMode == 2) g_gold += 200;
    g_maxBaseHp = 20 + (g_techWallHp * 10);
    g_baseHp = g_maxBaseHp;
    g_wave = 1;
    g_waveActive = FALSE;
    g_gameOver = FALSE;
    g_selectedSlot = -1;
    g_spawnQueueCount = 0;
    g_spawnQueueHead = 0;
    g_spawnTimer = 0;
    g_bossesKilled = 0;
    g_blizzTimer = 0;
    g_shakeIntensity = 0.0f;
    g_trebuchetCd = 0;
    g_castleBallistaCd = 0;
    g_meteorTimer = 0;

    float heroCdMod = 1.0f - (g_techHeroCd * 0.1f);
    if (g_mutators & MUTATOR_ECLIPSE) heroCdMod *= 1.4f;

    g_hero.x = 400.0f; g_hero.y = 300.0f;
    g_hero.targetX = 400.0f; g_hero.targetY = 300.0f;
    g_hero.maxHp = 100.0f; g_hero.hp = 100.0f;
    g_hero.speed = 2.2f;
    g_hero.damage = 20; g_hero.range = 60;
    g_hero.maxAttackCd = 25; g_hero.attackCd = 0;
    g_hero.respawnTimer = 0;
    g_hero.maxHealCd = (int)(300 * heroCdMod); g_hero.healCd = 0;
    g_hero.maxShieldCd = (int)(600 * heroCdMod); g_hero.shieldCd = 0;
    g_hero.maxMeteorCd = (int)(450 * heroCdMod); g_hero.meteorCd = 0;
    g_hero.maxSummonCd = (int)(500 * heroCdMod); g_hero.summonCd = 0;
    g_hero.shieldActive = 0;

    for (int i = 0; i < MAX_ENEMIES; i++) g_enemies[i].active = FALSE;
    for (int i = 0; i < MAX_PROJECTILES; i++) g_projectiles[i].active = FALSE;
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) g_floatingTexts[i].active = FALSE;
    for (int i = 0; i < MAX_PARTICLES; i++) g_particles[i].active = FALSE;
    for (int i = 0; i < MAX_TRAPS; i++) g_traps[i].active = FALSE;
    for (int i = 0; i < MAX_MILITIA; i++) g_militia[i].active = FALSE;
    for (int i = 0; i < MAX_SCORCH_MARKS; i++) g_scorchMarks[i].active = FALSE;
    for (int i = 0; i < MAX_LAVA_POOLS; i++) g_lavaPools[i].active = FALSE;
}

void SummonMilitia() {
    int count = 2 + (g_techMilitia > 0 ? 1 : 0);
    float hpBonus = 1.0f + (g_techMilitia * 0.5f);
    int spawned = 0;
    for (int i = 0; i < MAX_MILITIA && spawned < count; i++) {
        if (!g_militia[i].active) {
            g_militia[i].active = TRUE;
            g_militia[i].x = g_hero.x + ((spawned == 0) ? -20.0f : 20.0f);
            g_militia[i].y = g_hero.y + (float)(rand() % 20 - 10);
            g_militia[i].targetX = g_militia[i].x;
            g_militia[i].targetY = g_militia[i].y;
            g_militia[i].maxHp = 70.0f * hpBonus;
            g_militia[i].hp = g_militia[i].maxHp;
            g_militia[i].speed = 1.8f;
            g_militia[i].damage = (int)(18 * hpBonus);
            g_militia[i].attackCd = 0;
            g_militia[i].lifeTimer = 450;
            spawned++;
            SpawnParticleBurst(g_militia[i].x, g_militia[i].y, RGB(59, 130, 246), 8);
        }
    }
}

void TriggerTrebuchetStrike(float targetX, float targetY) {
    if (g_trebuchetCd > 0) return;
    g_trebuchetCd = g_maxTrebuchetCd;
    g_shakeIntensity = 24.0f;
    Beep(120, 150); Beep(80, 150);

    SpawnExplosion(targetX, targetY, RGB(251, 191, 36));
    AddFloatingText(targetX, targetY - 40, "SIEGE TREBUCHET!", TEXT_GOLD);

    float dmgBonus = 1.0f + (g_techSiegeEng * 0.25f);
    int siegeDmg = (int)(220 * dmgBonus);

    for (int e = 0; e < MAX_ENEMIES; e++) {
        if (g_enemies[e].active) {
            float dx = g_enemies[e].x - targetX;
            float dy = g_enemies[e].y - targetY;
            if (custom_sqrtf(dx*dx + dy*dy) <= 135.0f) {
                g_enemies[e].hp -= siegeDmg;
                SpawnParticleBurst(g_enemies[e].x, g_enemies[e].y, RGB(239, 68, 68), 6);
                if (g_enemies[e].hp <= 0) {
                    g_enemies[e].active = FALSE;
                    int bounty = (g_mutators & MUTATOR_BLOODLUST) ? 25 : 15;
                    g_gold += bounty;
                }
            }
        }
    }
}

void UpdateGameLogic() {
    if (g_gameOver) return;

    if ((g_mutators & MUTATOR_METEOR) && g_waveActive) {
        g_meteorTimer++;
        if (g_meteorTimer >= 180) {
            g_meteorTimer = 0;
            int rIdx = rand() % 5;
            float rx = (float)g_waypoints[rIdx].x + (rand() % 60 - 30);
            float ry = (float)g_waypoints[rIdx].y + (rand() % 60 - 30);
            SpawnExplosion(rx, ry, RGB(239, 68, 68));
            AddFloatingText(rx, ry - 30, "METEOR!", TEXT_RED);
            Beep(200, 80);

            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (g_enemies[e].active) {
                    float dx = g_enemies[e].x - rx;
                    float dy = g_enemies[e].y - ry;
                    if (custom_sqrtf(dx*dx + dy*dy) <= 85.0f) {
                        g_enemies[e].hp -= 70;
                        if (g_enemies[e].hp <= 0) { g_enemies[e].active = FALSE; g_gold += 15; }
                    }
                }
            }
        }
    }

    if (g_techSiegeEng > 0 && g_waveActive) {
        g_castleBallistaCd++;
        if (g_castleBallistaCd >= 75) {
            g_castleBallistaCd = 0;
            float castleX = (float)g_waypoints[5].x;
            float castleY = (float)g_waypoints[5].y;
            int bestE = -1;
            float bestDist = 9999.0f;
            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (!g_enemies[e].active) continue;
                float dx = g_enemies[e].x - castleX;
                float dy = g_enemies[e].y - castleY;
                float d = custom_sqrtf(dx*dx + dy*dy);
                if (d <= 270.0f && d < bestDist) {
                    bestDist = d;
                    bestE = e;
                }
            }
            if (bestE != -1) {
                for (int p = 0; p < MAX_PROJECTILES; p++) {
                    if (!g_projectiles[p].active) {
                        g_projectiles[p].active = TRUE;
                        g_projectiles[p].x = castleX;
                        g_projectiles[p].y = castleY;
                        g_projectiles[p].targetEnemyId = g_enemies[bestE].id;
                        g_projectiles[p].targetX = g_enemies[bestE].x;
                        g_projectiles[p].targetY = g_enemies[bestE].y;
                        g_projectiles[p].damage = 45 + (g_techSiegeEng * 20);
                        g_projectiles[p].speed = 18.0f;
                        g_projectiles[p].type = TOWER_BALLISTA;
                        g_projectiles[p].splash = 0;
                        g_projectiles[p].isCrit = FALSE;
                        g_projectiles[p].isPiercing = FALSE;
                        Beep(850, 25);
                        break;
                    }
                }
            }
        }
    }

    if (g_trebuchetCd > 0) g_trebuchetCd--;

    for (int lp = 0; lp < MAX_LAVA_POOLS; lp++) {
        if (!g_lavaPools[lp].active) continue;
        g_lavaPools[lp].life--;
        if (g_lavaPools[lp].life <= 0) {
            g_lavaPools[lp].active = FALSE;
            continue;
        }

        if (g_lavaPools[lp].life % 12 == 0) {
            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (g_enemies[e].active) {
                    float dx = g_enemies[e].x - g_lavaPools[lp].x;
                    float dy = g_enemies[e].y - g_lavaPools[lp].y;
                    if (custom_sqrtf(dx*dx + dy*dy) <= (float)g_lavaPools[lp].radius) {
                        g_enemies[e].hp -= g_lavaPools[lp].damage;
                        SpawnParticleBurst(g_enemies[e].x, g_enemies[e].y, RGB(249, 115, 22), 3);
                        if (g_enemies[e].hp <= 0) {
                            g_enemies[e].active = FALSE;
                            g_gold += 15;
                        }
                    }
                }
            }
        }
    }

    if (g_waveActive && g_spawnQueueHead < g_spawnQueueCount) {
        g_spawnTimer++;
        int interval = (g_gameMode == 2) ? 14 : 22;
        if (g_spawnTimer >= interval) {
            g_spawnTimer = 0;
            int type = g_spawnQueue[g_spawnQueueHead++];

            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (!g_enemies[i].active) {
                    g_enemies[i].active = TRUE;
                    g_enemies[i].type = type;
                    g_enemies[i].id = g_nextEnemyId++;
                    g_enemies[i].x = (float)g_waypoints[0].x;
                    g_enemies[i].y = (float)g_waypoints[0].y;
                    g_enemies[i].waypointIndex = 0;
                    g_enemies[i].slowed = FALSE;
                    g_enemies[i].poisonTicks = 0;
                    g_enemies[i].poisonDmg = 0;
                    g_enemies[i].summonTimer = 0;

                    int baseHp = 25;
                    float speed = 1.6f;
                    int r = 12;

                    if (type == ENEMY_GOBLIN) { baseHp = 25 + g_wave * 5; speed = 1.6f + ((rand() % 10) / 30.0f); r = 12; }
                    else if (type == ENEMY_ORC) { baseHp = 60 + g_wave * 12; speed = 1.0f; r = 14; }
                    else if (type == ENEMY_HOUND) { baseHp = 20 + g_wave * 4; speed = 2.8f; r = 10; }
                    else if (type == ENEMY_GARGOYLE) { baseHp = 30 + g_wave * 5; speed = 1.4f; r = 13; }
                    else if (type == ENEMY_OGRE) {
                        baseHp = 150 + g_wave * 30;
                        if (g_gameMode == 2) baseHp += g_wave * 50;
                        speed = (g_gameMode == 2) ? 0.8f : 0.6f;
                        r = 18;
                    }
                    else if (type == ENEMY_NECROMANCER) { baseHp = 90 + g_wave * 15; speed = 0.9f; r = 14; }
                    else if (type == ENEMY_SKELETON) { baseHp = 18 + g_wave * 3; speed = 2.2f; r = 10; }
                    else if (type == ENEMY_WYVERN) { baseHp = 240 + g_wave * 45; speed = 1.4f; r = 18; }
                    else if (type == ENEMY_GOLEM) { baseHp = 420 + g_wave * 70; speed = 0.5f; r = 20; }

                    if (g_mutators & MUTATOR_TITAN) baseHp = (int)(baseHp * 2.0f);
                    if (g_mutators & MUTATOR_BLOODLUST) speed *= 1.4f;

                    g_enemies[i].hp = baseHp;
                    g_enemies[i].maxHp = baseHp;
                    g_enemies[i].speed = speed;
                    g_enemies[i].radius = r;
                    break;
                }
            }
        }
    }

    if (g_hero.healCd > 0) g_hero.healCd--;
    if (g_hero.shieldCd > 0) g_hero.shieldCd--;
    if (g_hero.meteorCd > 0) g_hero.meteorCd--;
    if (g_hero.summonCd > 0) g_hero.summonCd--;
    if (g_hero.shieldActive > 0) g_hero.shieldActive--;

    if (g_hero.respawnTimer > 0) {
        g_hero.respawnTimer--;
        if (g_hero.respawnTimer <= 0) {
            g_hero.hp = g_hero.maxHp;
            g_hero.x = (float)g_waypoints[5].x;
            g_hero.y = (float)g_waypoints[5].y;
            g_hero.targetX = g_hero.x;
            g_hero.targetY = g_hero.y;
            AddFloatingText(g_hero.x, g_hero.y - 20, "HERO RESPAWNED!", TEXT_GOLD);
        }
    } else {
        float hdx = g_hero.targetX - g_hero.x;
        float hdy = g_hero.targetY - g_hero.y;
        float hdist = custom_sqrtf(hdx * hdx + hdy * hdy);
        if (hdist > g_hero.speed) {
            g_hero.x += (hdx / hdist) * g_hero.speed;
            g_hero.y += (hdy / hdist) * g_hero.speed;
        } else {
            g_hero.x = g_hero.targetX;
            g_hero.y = g_hero.targetY;
        }

        if (g_hero.attackCd > 0) g_hero.attackCd--;
        else {
            int bestE = -1;
            float bestDist = 9999.0f;
            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (!g_enemies[e].active) continue;
                float edx = g_enemies[e].x - g_hero.x;
                float edy = g_enemies[e].y - g_hero.y;
                float ed = custom_sqrtf(edx*edx + edy*edy);
                if (ed <= (float)g_hero.range && ed < bestDist) {
                    bestDist = ed;
                    bestE = e;
                }
            }
            if (bestE != -1) {
                g_hero.attackCd = g_hero.maxAttackCd;
                g_enemies[bestE].hp -= g_hero.damage;
                SpawnParticleBurst(g_enemies[bestE].x, g_enemies[bestE].y, TEXT_GOLD, 6);
                Beep(550, 20);
                if (g_enemies[bestE].hp <= 0) {
                    g_enemies[bestE].active = FALSE;
                    int reward = (g_enemies[bestE].type == ENEMY_OGRE || g_enemies[bestE].type == ENEMY_WYVERN || g_enemies[bestE].type == ENEMY_GOLEM) ? 80 : 15;
                    if (g_mutators & MUTATOR_BLOODLUST) reward = (int)(reward * 1.5f);
                    g_gold += reward;
                    char rBuf[16]; wsprintfA(rBuf, "+%dg", reward);
                    AddFloatingText(g_enemies[bestE].x, g_enemies[bestE].y - 10, rBuf, TEXT_GOLD);
                }
            }
        }
    }

    for (int m = 0; m < MAX_MILITIA; m++) {
        if (!g_militia[m].active) continue;
        g_militia[m].lifeTimer--;
        if (g_militia[m].lifeTimer <= 0 || g_militia[m].hp <= 0) {
            g_militia[m].active = FALSE;
            SpawnParticleBurst(g_militia[m].x, g_militia[m].y, RGB(148, 163, 184), 6);
            continue;
        }

        int targetEnemy = -1;
        float closestDist = 9999.0f;
        for (int e = 0; e < MAX_ENEMIES; e++) {
            if (!g_enemies[e].active) continue;
            float dx = g_enemies[e].x - g_militia[m].x;
            float dy = g_enemies[e].y - g_militia[m].y;
            float d = custom_sqrtf(dx*dx + dy*dy);
            if (d < closestDist && d <= 120.0f) {
                closestDist = d;
                targetEnemy = e;
            }
        }

        if (targetEnemy != -1) {
            float dx = g_enemies[targetEnemy].x - g_militia[m].x;
            float dy = g_enemies[targetEnemy].y - g_militia[m].y;
            if (closestDist > 25.0f) {
                g_militia[m].x += (dx / closestDist) * g_militia[m].speed;
                g_militia[m].y += (dy / closestDist) * g_militia[m].speed;
            } else {
                if (g_militia[m].attackCd > 0) g_militia[m].attackCd--;
                else {
                    g_militia[m].attackCd = 25;
                    g_enemies[targetEnemy].hp -= g_militia[m].damage;
                    g_militia[m].hp -= 5.0f;
                    SpawnParticleBurst(g_enemies[targetEnemy].x, g_enemies[targetEnemy].y, RGB(59, 130, 246), 4);
                    if (g_enemies[targetEnemy].hp <= 0) {
                        g_enemies[targetEnemy].active = FALSE;
                        g_gold += 15;
                    }
                }
            }
        }
    }

    if (g_blizzTimer > 0) g_blizzTimer--;
    int activeEnemyCount = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!g_enemies[i].active) continue;
        activeEnemyCount++;

        if (g_enemies[i].poisonTicks > 0) {
            if (g_enemies[i].poisonTicks % 15 == 0) {
                g_enemies[i].hp -= g_enemies[i].poisonDmg;
                SpawnParticleBurst(g_enemies[i].x, g_enemies[i].y, RGB(34, 197, 94), 3);
            }
            g_enemies[i].poisonTicks--;
            if (g_enemies[i].hp <= 0) {
                g_enemies[i].active = FALSE;
                g_gold += 15;
                continue;
            }
        }

        if (g_enemies[i].type == ENEMY_NECROMANCER) {
            g_enemies[i].summonTimer++;
            if (g_enemies[i].summonTimer >= 130) {
                g_enemies[i].summonTimer = 0;
                for (int s = 0; s < MAX_ENEMIES; s++) {
                    if (!g_enemies[s].active) {
                        g_enemies[s].active = TRUE;
                        g_enemies[s].type = ENEMY_SKELETON;
                        g_enemies[s].id = g_nextEnemyId++;
                        g_enemies[s].x = g_enemies[i].x;
                        g_enemies[s].y = g_enemies[i].y;
                        g_enemies[s].waypointIndex = g_enemies[i].waypointIndex;
                        g_enemies[s].slowed = FALSE;
                        g_enemies[s].hp = 18 + g_wave * 3;
                        if (g_mutators & MUTATOR_TITAN) g_enemies[s].hp *= 2;
                        g_enemies[s].maxHp = g_enemies[s].hp;
                        g_enemies[s].speed = 2.2f;
                        if (g_mutators & MUTATOR_BLOODLUST) g_enemies[s].speed *= 1.4f;
                        g_enemies[s].radius = 10;
                        SpawnParticleBurst(g_enemies[s].x, g_enemies[s].y, RGB(168, 85, 247), 8);
                        AddFloatingText(g_enemies[i].x, g_enemies[i].y - 20, "RAISE!", RGB(168, 85, 247));
                        break;
                    }
                }
            }
        }

        g_enemies[i].slowed = (g_blizzTimer > 0 && g_enemies[i].type != ENEMY_WYVERN && g_enemies[i].type != ENEMY_GOLEM);
        if (!g_enemies[i].slowed && g_enemies[i].type != ENEMY_WYVERN && g_enemies[i].type != ENEMY_GOLEM) {
            for (int t = 0; t < g_slotCount; t++) {
                if (g_slots[t].occupied && (g_slots[t].towerType == TOWER_FROST || g_slots[t].towerType == TOWER_SUPERCONDUCTOR)) {
                    float fdx = g_enemies[i].x - g_slots[t].x;
                    float fdy = g_enemies[i].y - g_slots[t].y;
                    if (custom_sqrtf(fdx*fdx + fdy*fdy) <= (float)g_slots[t].range) {
                        g_enemies[i].slowed = TRUE;
                        break;
                    }
                }
            }
        }
        
        BOOL trapBlocked = FALSE;
        for (int tr = 0; tr < MAX_TRAPS; tr++) {
            if (g_traps[tr].active) {
                float tdx = g_enemies[i].x - g_traps[tr].x;
                float tdy = g_enemies[i].y - g_traps[tr].y;
                if (custom_sqrtf(tdx*tdx + tdy*tdy) <= (float)(g_traps[tr].radius + g_enemies[i].radius)) {
                    if (g_traps[tr].type == TRAP_SPIKE) {
                        int spikeDmg = 50 + (g_techFortTraps * 20);
                        g_enemies[i].hp -= spikeDmg;
                        g_traps[tr].charges--;
                        SpawnParticleBurst(g_enemies[i].x, g_enemies[i].y, RGB(239, 68, 68), 5);
                        if (g_traps[tr].charges <= 0) g_traps[tr].active = FALSE;
                    } else if (g_traps[tr].type == TRAP_OIL) {
                        if (g_enemies[i].type != ENEMY_WYVERN) g_enemies[i].slowed = TRUE;
                    } else if (g_traps[tr].type == TRAP_BARRICADE) {
                        if (g_enemies[i].type == ENEMY_GOLEM) {
                            g_traps[tr].active = FALSE;
                            SpawnExplosion(g_traps[tr].x, g_traps[tr].y, RGB(139, 69, 19));
                        } else {
                            trapBlocked = TRUE;
                            g_traps[tr].hp -= 0.5f;
                            if (g_traps[tr].hp <= 0) g_traps[tr].active = FALSE;
                        }
                    } else if (g_traps[tr].type == TRAP_DYNAMITE) {
                        g_traps[tr].active = FALSE;
                        SpawnExplosion(g_traps[tr].x, g_traps[tr].y, RGB(239, 68, 68));
                        for (int e2 = 0; e2 < MAX_ENEMIES; e2++) {
                            if (g_enemies[e2].active) {
                                float edx = g_enemies[e2].x - g_traps[tr].x;
                                float edy = g_enemies[e2].y - g_traps[tr].y;
                                if (custom_sqrtf(edx*edx + edy*edy) <= (float)g_traps[tr].radius) {
                                    g_enemies[e2].hp -= 140;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        if (g_enemies[i].hp <= 0) {
            g_enemies[i].active = FALSE;
            SpawnParticleBurst(g_enemies[i].x, g_enemies[i].y, RGB(34, 197, 94), 10);
            int bounty = (g_mutators & MUTATOR_BLOODLUST) ? 25 : 15;
            g_gold += bounty;
            continue;
        }

        if (trapBlocked) continue;

        float currentSpeed = g_enemies[i].slowed ? (g_enemies[i].speed * 0.4f) : g_enemies[i].speed;

        if (g_hero.respawnTimer <= 0 && g_hero.shieldActive <= 0) {
            float hdx = g_hero.x - g_enemies[i].x;
            float hdy = g_hero.y - g_enemies[i].y;
            if (custom_sqrtf(hdx*hdx + hdy*hdy) < (float)(g_enemies[i].radius + 12)) {
                g_hero.hp -= 0.5f;
                if (g_hero.hp <= 0) {
                    g_hero.hp = 0;
                    g_hero.respawnTimer = 300;
                    Beep(200, 100);
                    AddFloatingText(g_hero.x, g_hero.y, "HERO FALLEN!", RGB(239, 68, 68));
                }
            }
        }

        int nextIdx = g_enemies[i].waypointIndex + 1;
        if (g_enemies[i].type == ENEMY_GARGOYLE || g_enemies[i].type == ENEMY_WYVERN) nextIdx = MAX_WAYPOINTS - 1;
        
        Point targetWP = g_waypoints[nextIdx];
        float dx = targetWP.x - g_enemies[i].x;
        float dy = targetWP.y - g_enemies[i].y;
        float dist = custom_sqrtf(dx * dx + dy * dy);

        if (dist < currentSpeed) {
            g_enemies[i].x = (float)targetWP.x;
            g_enemies[i].y = (float)targetWP.y;
            
            if (g_enemies[i].type == ENEMY_GARGOYLE || g_enemies[i].type == ENEMY_WYVERN) {
                g_enemies[i].waypointIndex = MAX_WAYPOINTS;
            } else {
                g_enemies[i].waypointIndex++;
            }

            if (g_enemies[i].waypointIndex >= MAX_WAYPOINTS - 1) {
                int dmgToBase = 1;
                if (g_enemies[i].type == ENEMY_OGRE || g_enemies[i].type == ENEMY_WYVERN) dmgToBase = 5;
                else if (g_enemies[i].type == ENEMY_GOLEM) dmgToBase = 3;

                if (g_hero.shieldActive > 0) dmgToBase = 0;
                g_baseHp -= dmgToBase;
                if (dmgToBase > 0) {
                    if (g_enemies[i].type == ENEMY_OGRE || g_enemies[i].type == ENEMY_WYVERN || g_enemies[i].type == ENEMY_GOLEM) g_shakeIntensity = 18.0f;
                    else g_shakeIntensity = 8.0f;
                    Beep(100, 100); Beep(80, 100); Beep(180, 60);
                    char dmgBuf[16]; wsprintfA(dmgBuf, "-%d HP", dmgToBase);
                    AddFloatingText(g_enemies[i].x - 15, g_enemies[i].y - 20, dmgBuf, RGB(239, 68, 68));
                } else {
                    AddFloatingText(g_enemies[i].x - 15, g_enemies[i].y - 20, "BLOCKED!", RGB(59, 130, 246));
                }
                g_enemies[i].active = FALSE;

                if (g_baseHp <= 0) {
                    g_baseHp = 0;
                    g_gameOver = TRUE;
                    g_waveActive = FALSE;
                    Beep(120, 300);
                }
                continue;
            }
        } else {
            g_enemies[i].x += (dx / dist) * currentSpeed;
            g_enemies[i].y += (dy / dist) * currentSpeed;
        }
    }

    for (int i = 0; i < g_slotCount; i++) {
        if (!g_slots[i].occupied) continue;
        if (g_slots[i].towerType == TOWER_FROST) continue;

        if (g_slots[i].attackAnim > 0) g_slots[i].attackAnim--;
        if (g_slots[i].cooldown > 0) {
            g_slots[i].cooldown--;
        } else {
            int targetIdx = -1;
            float maxProgress = -1.0f;

            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (!g_enemies[e].active) continue;

                float edx = g_enemies[e].x - g_slots[i].x;
                float edy = g_enemies[e].y - g_slots[i].y;
                float edist = custom_sqrtf(edx * edx + edy * edy);

                if (edist <= (float)g_slots[i].range) {
                    float progress = g_enemies[e].waypointIndex * 1000.0f + edist;
                    if (progress > maxProgress) {
                        maxProgress = progress;
                        targetIdx = e;
                    }
                }
            }

            if (targetIdx != -1) {
                g_slots[i].cooldown = g_slots[i].maxCooldown;
                g_slots[i].attackAnim = 5;

                if (g_slots[i].towerType == TOWER_SOLAR_BEAM) {
                    g_slots[i].beamTargetId = g_enemies[targetIdx].id;
                    int beamDmg = g_slots[i].damage;
                    if (g_enemies[targetIdx].maxHp > 100) beamDmg += (int)(g_enemies[targetIdx].hp * 0.04f);
                    g_enemies[targetIdx].hp -= beamDmg;
                    SpawnParticleBurst(g_enemies[targetIdx].x, g_enemies[targetIdx].y, RGB(254, 240, 138), 3);
                    if (g_enemies[targetIdx].hp <= 0) {
                        g_enemies[targetIdx].active = FALSE;
                        g_gold += 15;
                    }
                    continue;
                }

                if (g_slots[i].towerType == TOWER_TESLA || g_slots[i].towerType == TOWER_SUPERCONDUCTOR) {
                    int maxChains = (g_slots[i].towerType == TOWER_SUPERCONDUCTOR) ? 5 : 2;
                    g_enemies[targetIdx].hp -= g_slots[i].damage;
                    if (g_slots[i].towerType == TOWER_SUPERCONDUCTOR) g_enemies[targetIdx].slowed = TRUE;
                    SpawnParticleBurst(g_enemies[targetIdx].x, g_enemies[targetIdx].y, (g_slots[i].towerType == TOWER_SUPERCONDUCTOR) ? RGB(14, 165, 233) : RGB(56, 189, 248), 8);
                    
                    int chains = 0;
                    for (int e2 = 0; e2 < MAX_ENEMIES && chains < maxChains; e2++) {
                        if (g_enemies[e2].active && e2 != targetIdx) {
                            float cdx = g_enemies[e2].x - g_enemies[targetIdx].x;
                            float cdy = g_enemies[e2].y - g_enemies[targetIdx].y;
                            if (custom_sqrtf(cdx*cdx + cdy*cdy) <= (g_slots[i].towerType == TOWER_SUPERCONDUCTOR ? 140.0f : 90.0f)) {
                                g_enemies[e2].hp -= (int)(g_slots[i].damage * 0.75f);
                                if (g_slots[i].towerType == TOWER_SUPERCONDUCTOR) g_enemies[e2].slowed = TRUE;
                                SpawnParticleBurst(g_enemies[e2].x, g_enemies[e2].y, RGB(147, 197, 253), 6);
                                chains++;
                            }
                        }
                    }
                    Beep(1100, 30); Beep(1400, 30);
                    if (g_enemies[targetIdx].hp <= 0) {
                        g_enemies[targetIdx].active = FALSE;
                        g_gold += 15;
                    }
                    continue;
                }

                if (g_slots[i].towerType == TOWER_VENOMSPITE) {
                    for (int p = 0; p < MAX_PROJECTILES; p++) {
                        if (!g_projectiles[p].active) {
                            g_projectiles[p].active = TRUE;
                            g_projectiles[p].x = (float)g_slots[i].x;
                            g_projectiles[p].y = (float)g_slots[i].y;
                            g_projectiles[p].targetEnemyId = g_enemies[targetIdx].id;
                            g_projectiles[p].targetX = g_enemies[targetIdx].x;
                            g_projectiles[p].targetY = g_enemies[targetIdx].y;
                            g_projectiles[p].damage = g_slots[i].damage;
                            g_projectiles[p].type = TOWER_VENOMSPITE;
                            g_projectiles[p].splash = g_slots[i].splash;
                            g_projectiles[p].speed = 15.0f;
                            g_projectiles[p].isCrit = TRUE;
                            g_projectiles[p].isPiercing = TRUE;

                            float dx = g_enemies[targetIdx].x - (float)g_slots[i].x;
                            float dy = g_enemies[targetIdx].y - (float)g_slots[i].y;
                            float d = custom_sqrtf(dx*dx + dy*dy);
                            g_projectiles[p].vx = (dx / d) * 15.0f;
                            g_projectiles[p].vy = (dy / d) * 15.0f;
                            g_projectiles[p].lifeTimer = 35;
                            Beep(700, 30);
                            break;
                        }
                    }
                    continue;
                }

                for (int p = 0; p < MAX_PROJECTILES; p++) {
                    if (!g_projectiles[p].active) {
                        g_projectiles[p].active = TRUE;
                        g_projectiles[p].x = (float)g_slots[i].x;
                        g_projectiles[p].y = (float)g_slots[i].y;
                        g_projectiles[p].targetEnemyId = g_enemies[targetIdx].id;
                        g_projectiles[p].targetX = g_enemies[targetIdx].x;
                        g_projectiles[p].targetY = g_enemies[targetIdx].y;
                        g_projectiles[p].damage = g_slots[i].damage;
                        g_projectiles[p].type = g_slots[i].towerType;
                        g_projectiles[p].splash = g_slots[i].splash;
                        g_projectiles[p].isCrit = (g_slots[i].towerType == TOWER_BALLISTA && (rand() % 100 < 35));
                        g_projectiles[p].isPiercing = FALSE;

                        if (g_slots[i].towerType == TOWER_CANNON || g_slots[i].towerType == TOWER_INFERNO) g_projectiles[p].speed = 6.0f;
                        else if (g_slots[i].towerType == TOWER_BALLISTA) g_projectiles[p].speed = 16.0f;
                        else if (g_slots[i].towerType == TOWER_POISON) g_projectiles[p].speed = 8.0f;
                        else g_projectiles[p].speed = 9.0f;

                        if (g_slots[i].towerType == TOWER_CANNON || g_slots[i].towerType == TOWER_INFERNO) Beep(180, 20);
                        else if (g_slots[i].towerType == TOWER_MAGE) Beep(600, 20);
                        else Beep(450, 20);
                        break;
                    }
                }
            } else {
                g_slots[i].beamTargetId = -1;
            }
        }
    }

    for (int p = 0; p < MAX_PROJECTILES; p++) {
        if (!g_projectiles[p].active) continue;

        if (g_projectiles[p].isPiercing) {
            g_projectiles[p].x += g_projectiles[p].vx;
            g_projectiles[p].y += g_projectiles[p].vy;
            g_projectiles[p].lifeTimer--;

            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (!g_enemies[e].active) continue;
                float edx = g_enemies[e].x - g_projectiles[p].x;
                float edy = g_enemies[e].y - g_projectiles[p].y;
                if (custom_sqrtf(edx*edx + edy*edy) <= (float)(g_enemies[e].radius + 10)) {
                    g_enemies[e].hp -= g_projectiles[p].damage;
                    g_enemies[e].poisonTicks = 120;
                    g_enemies[e].poisonDmg = 12;
                    SpawnParticleBurst(g_enemies[e].x, g_enemies[e].y, RGB(34, 197, 94), 6);
                    if (g_enemies[e].hp <= 0) {
                        g_enemies[e].active = FALSE;
                        g_gold += 15;
                    }
                }
            }

            if (g_projectiles[p].lifeTimer <= 0) g_projectiles[p].active = FALSE;
            continue;
        }

        int targetIdx = -1;
        for (int e = 0; e < MAX_ENEMIES; e++) {
            if (g_enemies[e].active && g_enemies[e].id == g_projectiles[p].targetEnemyId) {
                targetIdx = e;
                g_projectiles[p].targetX = g_enemies[e].x;
                g_projectiles[p].targetY = g_enemies[e].y;
                break;
            }
        }

        float pdx = g_projectiles[p].targetX - g_projectiles[p].x;
        float pdy = g_projectiles[p].targetY - g_projectiles[p].y;
        float pdist = custom_sqrtf(pdx * pdx + pdy * pdy);

        if (pdist < g_projectiles[p].speed) {
            g_projectiles[p].active = FALSE;
            int baseDmg = g_projectiles[p].damage;
            if (g_projectiles[p].isCrit) {
                baseDmg *= 2;
                AddFloatingText(g_projectiles[p].targetX, g_projectiles[p].targetY - 20, "CRIT!", TEXT_GOLD);
            }

            if ((g_mutators & MUTATOR_PHASE_SHIFT) && (rand() % 100 < 25) && g_projectiles[p].type != TOWER_BALLISTA) {
                AddFloatingText(g_projectiles[p].targetX, g_projectiles[p].targetY - 15, "PHASE!", RGB(168, 85, 247));
                continue;
            }

            if (g_projectiles[p].type == TOWER_INFERNO) {
                SpawnExplosion(g_projectiles[p].targetX, g_projectiles[p].targetY, RGB(249, 115, 22));
                SpawnLavaPool(g_projectiles[p].targetX, g_projectiles[p].targetY, 65, 180, 18);
            }

            if (g_projectiles[p].splash > 0) {
                if (g_projectiles[p].type == TOWER_CANNON || g_projectiles[p].type == TOWER_POISON || g_projectiles[p].type == TOWER_INFERNO) {
                    if (g_scorchIndex < MAX_SCORCH_MARKS) {
                        g_scorchMarks[g_scorchIndex].active = TRUE;
                        g_scorchMarks[g_scorchIndex].x = g_projectiles[p].targetX;
                        g_scorchMarks[g_scorchIndex].y = g_projectiles[p].targetY;
                        g_scorchMarks[g_scorchIndex].radius = (float)g_projectiles[p].splash;
                        g_scorchMarks[g_scorchIndex].color = (g_projectiles[p].type == TOWER_INFERNO) ? RGB(80, 20, 10) : RGB(20, 20, 20);
                        g_scorchIndex = (g_scorchIndex + 1) % MAX_SCORCH_MARKS;
                    }
                }

                for (int e2 = 0; e2 < MAX_ENEMIES; e2++) {
                    if (g_enemies[e2].active) {
                        float edx = g_enemies[e2].x - g_projectiles[p].targetX;
                        float edy = g_enemies[e2].y - g_projectiles[p].targetY;
                        if (custom_sqrtf(edx*edx + edy*edy) <= (float)g_projectiles[p].splash) {
                            int d = baseDmg;
                            if (g_enemies[e2].type == ENEMY_ORC && g_projectiles[p].type != TOWER_MAGE && g_projectiles[p].type != TOWER_CANNON && g_projectiles[p].type != TOWER_BALLISTA && g_projectiles[p].type != TOWER_INFERNO) {
                                d = d / 2; if (d < 1) d = 1;
                            }
                            g_enemies[e2].hp -= d;
                            if (g_projectiles[p].type == TOWER_POISON) {
                                g_enemies[e2].poisonTicks = 75;
                                g_enemies[e2].poisonDmg = 6;
                            }
                            SpawnParticleBurst(g_enemies[e2].x, g_enemies[e2].y, (g_projectiles[p].type == TOWER_INFERNO) ? RGB(249, 115, 22) : RGB(168, 85, 247), 5);
                            if (g_enemies[e2].hp <= 0) {
                                g_enemies[e2].active = FALSE;
                                int reward = (g_enemies[e2].type == ENEMY_OGRE || g_enemies[e2].type == ENEMY_WYVERN || g_enemies[e2].type == ENEMY_GOLEM) ? 100 : 15;
                                if (g_mutators & MUTATOR_BLOODLUST) reward = (int)(reward * 1.5f);
                                g_gold += reward;
                            }
                        }
                    }
                }
            } else if (targetIdx != -1) {
                int d = baseDmg;
                if (g_enemies[targetIdx].type == ENEMY_ORC && g_projectiles[p].type != TOWER_MAGE && g_projectiles[p].type != TOWER_CANNON && g_projectiles[p].type != TOWER_BALLISTA && g_projectiles[p].type != TOWER_INFERNO) {
                    d = d / 2; if (d < 1) d = 1;
                }
                g_enemies[targetIdx].hp -= d;
                if (g_projectiles[p].type == TOWER_POISON) {
                    g_enemies[targetIdx].poisonTicks = 75;
                    g_enemies[targetIdx].poisonDmg = 6;
                }
                if (g_projectiles[p].type == TOWER_CANNON) SpawnExplosion(g_enemies[targetIdx].x, g_enemies[targetIdx].y, RGB(71, 85, 105));
                else SpawnParticleBurst(g_enemies[targetIdx].x, g_enemies[targetIdx].y, TEXT_GOLD, 4);

                if (g_enemies[targetIdx].hp <= 0) {
                    g_enemies[targetIdx].active = FALSE;
                    int reward = (g_enemies[targetIdx].type == ENEMY_OGRE || g_enemies[targetIdx].type == ENEMY_WYVERN || g_enemies[targetIdx].type == ENEMY_GOLEM) ? 100 : 15;
                    if (g_mutators & MUTATOR_BLOODLUST) reward = (int)(reward * 1.5f);
                    g_gold += reward;
                }
            }
        } else {
            g_projectiles[p].x += (pdx / pdist) * g_projectiles[p].speed;
            g_projectiles[p].y += (pdy / pdist) * g_projectiles[p].speed;
        }
    }

    for (int p = 0; p < MAX_PARTICLES; p++) {
        if (!g_particles[p].active) continue;
        g_particles[p].x += g_particles[p].vx;
        g_particles[p].y += g_particles[p].vy;
        g_particles[p].vx *= g_particles[p].drag;
        g_particles[p].vy *= g_particles[p].drag;
        g_particles[p].vy += g_particles[p].gravity;
        g_particles[p].rot += g_particles[p].rotSpd;
        g_particles[p].size += g_particles[p].growth;
        if (g_particles[p].size < 0.5f) g_particles[p].size = 0.5f;
        g_particles[p].life--;
        if (g_particles[p].life <= 0) g_particles[p].active = FALSE;
    }

    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (!g_shockwaves[i].active) continue;
        g_shockwaves[i].r += (g_shockwaves[i].maxR - g_shockwaves[i].r) * 0.22f + 1.2f;
        g_shockwaves[i].life--;
        if (g_shockwaves[i].life <= 0) g_shockwaves[i].active = FALSE;
    }

    g_globalFrame++;
    for (int p = 0; p < 80; p++) {
        if (g_weatherParticles[p].active) {
            g_weatherParticles[p].x += g_weatherParticles[p].vx;
            g_weatherParticles[p].y += g_weatherParticles[p].vy;
            if (g_weatherParticles[p].x < 0) g_weatherParticles[p].x = 800;
            if (g_weatherParticles[p].x > 800) g_weatherParticles[p].x = 0;
            if (g_weatherParticles[p].y < 0) g_weatherParticles[p].y = 560;
            if (g_weatherParticles[p].y > 560) g_weatherParticles[p].y = 0;
        }
    }

    for (int f = 0; f < MAX_FLOATING_TEXTS; f++) {
        if (!g_floatingTexts[f].active) continue;
        g_floatingTexts[f].y -= 0.8f;
        g_floatingTexts[f].life--;
        if (g_floatingTexts[f].life <= 0) g_floatingTexts[f].active = FALSE;
    }

    if (g_toastTimer > 0) g_toastTimer--;

    if (g_shakeIntensity > 0.05f) {
        g_shakeAngle += 1.8f;
        g_shakeIntensity *= 0.88f;
    } else {
        g_shakeIntensity = 0.0f;
    }

    if (g_waveActive && g_spawnQueueHead == g_spawnQueueCount && activeEnemyCount == 0) {
        g_waveActive = FALSE;
        int bonus = 20 + g_wave * 5;
        if (g_gameMode == 2) bonus = 100 + g_wave * 20;
        if (g_mutators & MUTATOR_TITAN) bonus *= 2;
        g_gold += bonus;

        char buf[64];
        wsprintfA(buf, "WAVE CLEAR! +%dg", bonus);
        AddFloatingText((float)(WINDOW_WIDTH / 2 - 50), (float)(WINDOW_HEIGHT / 2), buf, RGB(16, 185, 129));
        SpawnExplosion((float)(WINDOW_WIDTH / 2 - 50), (float)(WINDOW_HEIGHT / 2), RGB(16, 185, 129));

        char tBuf[128];
        wsprintfA(tBuf, "Wave %d Cleared! +%dg bounty awarded! Press [Space] for next wave.", g_wave, bonus);
        ShowNativeToast(tBuf, RGB(16, 185, 129), 120);

        if (g_gameMode == 1) {
            if (g_wave > g_hsEndless) { g_hsEndless = g_wave; SaveGame(); }
        }

        g_wave++;
        Beep(523, 100); Beep(659, 100); Beep(784, 100); Beep(1046, 200);
    }
}

void DrawRoundedRect(HDC hdc, int left, int top, int right, int bottom, COLORREF fillColor, COLORREF borderColor, int radius) {
    HBRUSH fillBrush = CreateSolidBrush(fillColor);
    HPEN pen = CreatePen(PS_SOLID, 1, borderColor);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, fillBrush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    RoundRect(hdc, left, top, right, bottom, radius, radius);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(fillBrush);
    DeleteObject(pen);
}

static void DrawTrapSprite(HDC memDC, int tx, int ty, int trr, int type, float hp, int charges) {
    if (type == TRAP_SPIKE) {
        // Steel Caltrops: metallic plate with 4 sharp triangular spike heads
        HBRUSH baseB = CreateSolidBrush(RGB(51, 65, 85));
        HPEN baseP = CreatePen(PS_SOLID, 1, RGB(100, 116, 139));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, baseB);
        HPEN oP = (HPEN)SelectObject(memDC, baseP);
        Ellipse(memDC, tx - trr + 2, ty - trr + 2, tx + trr - 2, ty + trr - 2);

        HBRUSH spkB = CreateSolidBrush(RGB(226, 232, 240));
        HPEN spkP = CreatePen(PS_SOLID, 1, RGB(148, 163, 184));
        SelectObject(memDC, spkB); SelectObject(memDC, spkP);
        POINT spk1[3] = { {tx - 8, ty + 2}, {tx - 2, ty - 6}, {tx - 5, ty + 5} };
        POINT spk2[3] = { {tx + 8, ty + 2}, {tx + 2, ty - 6}, {tx + 5, ty + 5} };
        POINT spk3[3] = { {tx, ty - 9}, {tx - 4, ty}, {tx + 4, ty} };
        POINT spk4[3] = { {tx, ty + 8}, {tx - 5, ty + 1}, {tx + 5, ty + 1} };
        Polygon(memDC, spk1, 3);
        Polygon(memDC, spk2, 3);
        Polygon(memDC, spk3, 3);
        Polygon(memDC, spk4, 3);
        
        SelectObject(memDC, oB); SelectObject(memDC, oP);
        DeleteObject(baseB); DeleteObject(baseP); DeleteObject(spkB); DeleteObject(spkP);
    } else if (type == TRAP_OIL) {
        // Dark swirling oil puddle with iridescent sheen rings
        HBRUSH oilB = CreateSolidBrush(RGB(15, 23, 42));
        HPEN oilP = CreatePen(PS_SOLID, 1, RGB(51, 65, 85));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, oilB);
        HPEN oP = (HPEN)SelectObject(memDC, oilP);
        Ellipse(memDC, tx - trr, ty - trr, tx + trr, ty + trr);

        HPEN iris1 = CreatePen(PS_SOLID, 2, RGB(168, 85, 247));
        SelectObject(memDC, iris1);
        Arc(memDC, tx - trr + 4, ty - trr + 4, tx + trr - 4, ty + trr - 4, tx - 5, ty, tx + 5, ty);
        HPEN iris2 = CreatePen(PS_SOLID, 1, RGB(56, 189, 248));
        SelectObject(memDC, iris2);
        Arc(memDC, tx - trr + 7, ty - trr + 7, tx + trr - 7, ty + trr - 7, tx, ty - 5, tx, ty + 5);

        SelectObject(memDC, oB); SelectObject(memDC, oP);
        DeleteObject(oilB); DeleteObject(oilP); DeleteObject(iris1); DeleteObject(iris2);
    } else if (type == TRAP_BARRICADE) {
        // Fortified timber barricade with crossbeams and iron rivets
        HBRUSH woodB = CreateSolidBrush(RGB(120, 53, 15));
        HPEN woodP = CreatePen(PS_SOLID, 1, RGB(69, 26, 3));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, woodB);
        HPEN oP = (HPEN)SelectObject(memDC, woodP);
        
        RoundRect(memDC, tx - 12, ty - 6, tx + 12, ty - 1, 2, 2);
        RoundRect(memDC, tx - 12, ty + 1, tx + 12, ty + 6, 2, 2);
        
        HPEN braceP = CreatePen(PS_SOLID, 2, RGB(180, 83, 9));
        SelectObject(memDC, braceP);
        MoveToEx(memDC, tx - 10, ty - 8, NULL); LineTo(memDC, tx + 10, ty + 8);
        MoveToEx(memDC, tx - 10, ty + 8, NULL); LineTo(memDC, tx + 10, ty - 8);

        HBRUSH rivB = CreateSolidBrush(RGB(203, 213, 225));
        SelectObject(memDC, rivB);
        Ellipse(memDC, tx - 11, ty - 7, tx - 7, ty - 3);
        Ellipse(memDC, tx + 7, ty - 7, tx + 11, ty - 3);
        Ellipse(memDC, tx - 11, ty + 3, tx - 7, ty + 7);
        Ellipse(memDC, tx + 7, ty + 3, tx + 11, ty + 7);

        SelectObject(memDC, oB); SelectObject(memDC, oP);
        DeleteObject(woodB); DeleteObject(woodP); DeleteObject(braceP); DeleteObject(rivB);
    } else if (type == TRAP_DYNAMITE) {
        // Red TNT dynamite bundle strapped with brass wire & sparking fuse
        HBRUSH tntB = CreateSolidBrush(RGB(220, 38, 38));
        HPEN tntP = CreatePen(PS_SOLID, 1, RGB(153, 27, 27));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, tntB);
        HPEN oP = (HPEN)SelectObject(memDC, tntP);

        RoundRect(memDC, tx - 8, ty - 8, tx - 2, ty + 8, 2, 2);
        RoundRect(memDC, tx - 3, ty - 9, tx + 3, ty + 7, 2, 2);
        RoundRect(memDC, tx + 2, ty - 8, tx + 8, ty + 8, 2, 2);

        HBRUSH strapB = CreateSolidBrush(RGB(234, 179, 8));
        SelectObject(memDC, strapB);
        Rectangle(memDC, tx - 9, ty - 2, tx + 9, ty + 2);

        HPEN fuseP = CreatePen(PS_SOLID, 1, RGB(245, 158, 11));
        SelectObject(memDC, fuseP);
        MoveToEx(memDC, tx, ty - 9, NULL);
        LineTo(memDC, tx + 3, ty - 13);
        LineTo(memDC, tx + 6, ty - 12);

        HBRUSH sparkB = CreateSolidBrush(RGB(254, 240, 138));
        SelectObject(memDC, sparkB);
        Ellipse(memDC, tx + 5, ty - 14, tx + 9, ty - 10);

        SelectObject(memDC, oB); SelectObject(memDC, oP);
        DeleteObject(tntB); DeleteObject(tntP); DeleteObject(strapB); DeleteObject(fuseP); DeleteObject(sparkB);
    }
}

static void DrawEnemySprite(HDC memDC, Enemy* e, int ex, int ey, int r, int walkBob) {
    int t = e->type;
    int y = ey + walkBob;

    // Boss aura for Ogre, Wyvern, Golem
    if (t == ENEMY_OGRE || t == ENEMY_WYVERN || t == ENEMY_GOLEM) {
        HPEN bossPen = CreatePen(PS_SOLID, 2, RGB(239, 68, 68));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));
        HPEN oP = (HPEN)SelectObject(memDC, bossPen);
        int auraPulse = (int)(custom_sinf(g_globalFrame * 0.2f) * 3.0f);
        Ellipse(memDC, ex - r - 6 - auraPulse, y - r - 6 - auraPulse, ex + r + 6 + auraPulse, y + r + 6 + auraPulse);
        
        for (int a = 0; a < 4; a++) {
            float ang = g_globalFrame * 0.08f + a * 1.57f;
            int gx = ex + (int)(custom_cosf(ang) * (r + 7 + auraPulse));
            int gy = y + (int)(custom_sinf(ang) * (r + 7 + auraPulse));
            MoveToEx(memDC, gx - 2, gy, NULL); LineTo(memDC, gx + 2, gy);
            MoveToEx(memDC, gx, gy - 2, NULL); LineTo(memDC, gx, gy + 2);
        }
        SelectObject(memDC, oB); SelectObject(memDC, oP); DeleteObject(bossPen);
    }

    if (t == ENEMY_GOBLIN) {
        HBRUSH eB = CreateSolidBrush(RGB(34, 197, 94));
        HPEN eP = CreatePen(PS_SOLID, 1, RGB(20, 83, 45));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, eB); HPEN oP = (HPEN)SelectObject(memDC, eP);
        Ellipse(memDC, ex - r, y - r, ex + r, y + r);
        POINT leftEar[3] = { {ex - r + 1, y - 2}, {ex - r - 6, y - 6}, {ex - r + 3, y + 2} };
        POINT rightEar[3] = { {ex + r - 1, y - 2}, {ex + r + 6, y - 6}, {ex + r - 3, y + 2} };
        Polygon(memDC, leftEar, 3);
        Polygon(memDC, rightEar, 3);
        HBRUSH eyeB = CreateSolidBrush(RGB(254, 240, 138));
        SelectObject(memDC, eyeB);
        Ellipse(memDC, ex - 4, y - 3, ex - 1, y);
        Ellipse(memDC, ex + 1, y - 3, ex + 4, y);
        HPEN dnP = CreatePen(PS_SOLID, 2, RGB(148, 163, 184));
        SelectObject(memDC, dnP);
        MoveToEx(memDC, ex + r - 1, y + 2, NULL); LineTo(memDC, ex + r + 5, y - 4);
        SelectObject(memDC, oB); SelectObject(memDC, oP);
        DeleteObject(eB); DeleteObject(eP); DeleteObject(eyeB); DeleteObject(dnP);
    } else if (t == ENEMY_ORC) {
        HBRUSH eB = CreateSolidBrush(RGB(21, 128, 61));
        HPEN eP = CreatePen(PS_SOLID, 1, RGB(5, 46, 22));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, eB); HPEN oP = (HPEN)SelectObject(memDC, eP);
        Ellipse(memDC, ex - r, y - r, ex + r, y + r);
        HBRUSH hB = CreateSolidBrush(RGB(71, 85, 105));
        HPEN hP = CreatePen(PS_SOLID, 1, RGB(148, 163, 184));
        SelectObject(memDC, hB); SelectObject(memDC, hP);
        Rectangle(memDC, ex - r + 1, y - r + 1, ex + r - 1, y - 1);
        Rectangle(memDC, ex - 2, y - 1, ex + 2, y + 3);
        HBRUSH tuskB = CreateSolidBrush(RGB(248, 250, 252));
        SelectObject(memDC, tuskB);
        MoveToEx(memDC, ex - 4, y + 4, NULL); LineTo(memDC, ex - 5, y + 1); LineTo(memDC, ex - 3, y + 4);
        MoveToEx(memDC, ex + 4, y + 4, NULL); LineTo(memDC, ex + 5, y + 1); LineTo(memDC, ex + 3, y + 4);
        SelectObject(memDC, oB); SelectObject(memDC, oP);
        DeleteObject(eB); DeleteObject(eP); DeleteObject(hB); DeleteObject(hP); DeleteObject(tuskB);
    } else if (t == ENEMY_HOUND) {
        HBRUSH eB = CreateSolidBrush(RGB(30, 27, 75));
        HPEN eP = CreatePen(PS_SOLID, 1, RGB(79, 70, 229));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, eB); HPEN oP = (HPEN)SelectObject(memDC, eP);
        Ellipse(memDC, ex - r - 2, y - r + 2, ex + r + 2, y + r - 2);
        POINT hEar1[3] = { {ex - r + 2, y - r + 2}, {ex - r - 3, y - r - 4}, {ex - r + 6, y - r} };
        POINT hEar2[3] = { {ex - r + 8, y - r + 2}, {ex - r + 4, y - r - 4}, {ex - r + 12, y - r} };
        Polygon(memDC, hEar1, 3); Polygon(memDC, hEar2, 3);
        HBRUSH eyeB = CreateSolidBrush(RGB(239, 68, 68));
        SelectObject(memDC, eyeB);
        Ellipse(memDC, ex - r + 4, y - 2, ex - r + 7, y + 1);
        HPEN colP = CreatePen(PS_SOLID, 2, RGB(203, 213, 225));
        SelectObject(memDC, colP);
        MoveToEx(memDC, ex, y - r + 2, NULL); LineTo(memDC, ex, y + r - 2);
        SelectObject(memDC, oB); SelectObject(memDC, oP);
        DeleteObject(eB); DeleteObject(eP); DeleteObject(eyeB); DeleteObject(colP);
    } else if (t == ENEMY_GARGOYLE) {
        int flap = (int)(custom_sinf(g_globalFrame * 0.28f + (float)ex) * 5.0f);
        HBRUSH wB = CreateSolidBrush(RGB(71, 85, 105));
        HPEN wP = CreatePen(PS_SOLID, 1, RGB(30, 41, 59));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, wB); HPEN oP = (HPEN)SelectObject(memDC, wP);
        POINT wingL[3] = { {ex - 2, y}, {ex - r - 9, y - 7 + flap}, {ex - r + 1, y + 5} };
        POINT wingR[3] = { {ex + 2, y}, {ex + r + 9, y - 7 + flap}, {ex + r - 1, y + 5} };
        Polygon(memDC, wingL, 3); Polygon(memDC, wingR, 3);
        HBRUSH gB = CreateSolidBrush(RGB(100, 116, 139));
        SelectObject(memDC, gB);
        Ellipse(memDC, ex - r, y - r, ex + r, y + r);
        MoveToEx(memDC, ex - 4, y - r + 2, NULL); LineTo(memDC, ex - 7, y - r - 4);
        MoveToEx(memDC, ex + 4, y - r + 2, NULL); LineTo(memDC, ex + 7, y - r - 4);
        HBRUSH eyB = CreateSolidBrush(RGB(251, 191, 36));
        SelectObject(memDC, eyB);
        Ellipse(memDC, ex - 4, y - 2, ex - 1, y + 1);
        Ellipse(memDC, ex + 1, y - 2, ex + 4, y + 1);
        SelectObject(memDC, oB); SelectObject(memDC, oP);
        DeleteObject(wB); DeleteObject(wP); DeleteObject(gB); DeleteObject(eyB);
    } else if (t == ENEMY_OGRE) {
        HBRUSH eB = CreateSolidBrush(RGB(133, 77, 14));
        HPEN eP = CreatePen(PS_SOLID, 2, RGB(69, 26, 3));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, eB); HPEN oP = (HPEN)SelectObject(memDC, eP);
        Ellipse(memDC, ex - r, y - r, ex + r, y + r);
        HBRUSH pB = CreateSolidBrush(RGB(71, 85, 105));
        SelectObject(memDC, pB);
        Ellipse(memDC, ex - 7, y - 3, ex + 7, y + 9);
        HBRUSH hrnB = CreateSolidBrush(RGB(254, 240, 138));
        SelectObject(memDC, hrnB);
        POINT h1[3] = { {ex - 8, y - r + 6}, {ex - 12, y - r - 4}, {ex - 4, y - r + 4} };
        POINT h2[3] = { {ex + 8, y - r + 6}, {ex + 12, y - r - 4}, {ex + 4, y - r + 4} };
        Polygon(memDC, h1, 3); Polygon(memDC, h2, 3);
        HPEN clP = CreatePen(PS_SOLID, 4, RGB(69, 26, 3));
        SelectObject(memDC, clP);
        MoveToEx(memDC, ex + r - 3, y + 8, NULL); LineTo(memDC, ex + r + 10, y - 14);
        HPEN spkP = CreatePen(PS_SOLID, 2, RGB(226, 232, 240));
        SelectObject(memDC, spkP);
        MoveToEx(memDC, ex + r + 8, y - 10, NULL); LineTo(memDC, ex + r + 13, y - 12);
        MoveToEx(memDC, ex + r + 6, y - 6, NULL); LineTo(memDC, ex + r + 11, y - 8);
        SelectObject(memDC, oB); SelectObject(memDC, oP);
        DeleteObject(eB); DeleteObject(eP); DeleteObject(pB); DeleteObject(hrnB); DeleteObject(clP); DeleteObject(spkP);
    } else if (t == ENEMY_NECROMANCER) {
        HBRUSH rB = CreateSolidBrush(RGB(88, 28, 135));
        HPEN rP = CreatePen(PS_SOLID, 1, RGB(147, 51, 234));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, rB); HPEN oP = (HPEN)SelectObject(memDC, rP);
        POINT cowl[3] = { {ex, y - r - 3}, {ex + r, y + r}, {ex - r, y + r} };
        Polygon(memDC, cowl, 3);
        HBRUSH skB = CreateSolidBrush(RGB(241, 245, 249));
        SelectObject(memDC, skB);
        Ellipse(memDC, ex - 4, y - 3, ex + 4, y + 4);
        HBRUSH eyB = CreateSolidBrush(RGB(34, 197, 94));
        SelectObject(memDC, eyB);
        Ellipse(memDC, ex - 3, y - 1, ex - 1, y + 1);
        Ellipse(memDC, ex + 1, y - 1, ex + 3, y + 1);
        HPEN stP = CreatePen(PS_SOLID, 2, RGB(226, 232, 240));
        SelectObject(memDC, stP);
        MoveToEx(memDC, ex + r + 2, y + 9, NULL); LineTo(memDC, ex + r + 2, y - 13);
        HBRUSH obB = CreateSolidBrush(RGB(74, 222, 128));
        HPEN obP = CreatePen(PS_SOLID, 1, RGB(187, 247, 208));
        SelectObject(memDC, obB); SelectObject(memDC, obP);
        Ellipse(memDC, ex + r - 1, y - 17, ex + r + 5, y - 11);
        SelectObject(memDC, oB); SelectObject(memDC, oP);
        DeleteObject(rB); DeleteObject(rP); DeleteObject(skB); DeleteObject(eyB); DeleteObject(stP); DeleteObject(obB); DeleteObject(obP);
    } else if (t == ENEMY_SKELETON) {
        HBRUSH sB = CreateSolidBrush(RGB(248, 250, 252));
        HPEN sP = CreatePen(PS_SOLID, 1, RGB(148, 163, 184));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, sB); HPEN oP = (HPEN)SelectObject(memDC, sP);
        Ellipse(memDC, ex - 5, y - r, ex + 5, y - r + 8);
        HBRUSH dkB = CreateSolidBrush(RGB(15, 23, 42));
        SelectObject(memDC, dkB);
        Ellipse(memDC, ex - 3, y - r + 3, ex - 1, y - r + 5);
        Ellipse(memDC, ex + 1, y - r + 3, ex + 3, y - r + 5);
        SelectObject(memDC, sP);
        MoveToEx(memDC, ex, y - r + 8, NULL); LineTo(memDC, ex, y + r - 2);
        MoveToEx(memDC, ex - 4, y - 1, NULL); LineTo(memDC, ex + 4, y - 1);
        MoveToEx(memDC, ex - 3, y + 3, NULL); LineTo(memDC, ex + 3, y + 3);
        HPEN swP = CreatePen(PS_SOLID, 1, RGB(203, 213, 225));
        SelectObject(memDC, swP);
        MoveToEx(memDC, ex + 3, y + 4, NULL); LineTo(memDC, ex + 9, y - 4);
        SelectObject(memDC, oB); SelectObject(memDC, oP);
        DeleteObject(sB); DeleteObject(sP); DeleteObject(dkB); DeleteObject(swP);
    } else if (t == ENEMY_WYVERN) {
        int flap = (int)(custom_sinf(g_globalFrame * 0.32f + (float)ex) * 7.0f);
        HBRUSH wB = CreateSolidBrush(RGB(185, 28, 28));
        HPEN wP = CreatePen(PS_SOLID, 2, RGB(127, 29, 29));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, wB); HPEN oP = (HPEN)SelectObject(memDC, wP);
        POINT dWingL[4] = { {ex - 3, y}, {ex - r - 12, y - 10 + flap}, {ex - r - 8, y + 3 + flap/2}, {ex - 2, y + 4} };
        POINT dWingR[4] = { {ex + 3, y}, {ex + r + 12, y - 10 + flap}, {ex + r + 8, y + 3 + flap/2}, {ex + 2, y + 4} };
        Polygon(memDC, dWingL, 4); Polygon(memDC, dWingR, 4);
        Ellipse(memDC, ex - r, y - r + 2, ex + r, y + r - 2);
        POINT snout[3] = { {ex, y - r + 3}, {ex - 4, y - r - 5}, {ex + 4, y - r - 5} };
        Polygon(memDC, snout, 3);
        HBRUSH eyB = CreateSolidBrush(RGB(253, 224, 71));
        SelectObject(memDC, eyB);
        Ellipse(memDC, ex - 4, y - 4, ex - 1, y - 1);
        Ellipse(memDC, ex + 1, y - 4, ex + 4, y - 1);
        SelectObject(memDC, wP);
        MoveToEx(memDC, ex, y + r - 2, NULL); LineTo(memDC, ex + 4, y + r + 6); LineTo(memDC, ex, y + r + 10);
        SelectObject(memDC, oB); SelectObject(memDC, oP);
        DeleteObject(wB); DeleteObject(wP); DeleteObject(eyB);
    } else if (t == ENEMY_GOLEM) {
        HBRUSH gB = CreateSolidBrush(RGB(51, 65, 85));
        HPEN gP = CreatePen(PS_SOLID, 2, RGB(100, 116, 139));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, gB); HPEN oP = (HPEN)SelectObject(memDC, gP);
        RoundRect(memDC, ex - r, y - r, ex + r, y + r, 4, 4);
        HBRUSH shB = CreateSolidBrush(RGB(30, 41, 59));
        SelectObject(memDC, shB);
        RoundRect(memDC, ex - r - 4, y - r + 2, ex - r + 3, y - 2, 2, 2);
        RoundRect(memDC, ex + r - 3, y - r + 2, ex + r + 4, y - 2, 2, 2);
        HPEN rnP = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
        SelectObject(memDC, rnP);
        MoveToEx(memDC, ex, y - 7, NULL); LineTo(memDC, ex, y + 7);
        MoveToEx(memDC, ex - 5, y - 2, NULL); LineTo(memDC, ex, y + 3); LineTo(memDC, ex + 5, y - 2);
        HBRUSH coreB = CreateSolidBrush(RGB(186, 230, 253));
        SelectObject(memDC, coreB);
        Ellipse(memDC, ex - 2, y - 2, ex + 2, y + 2);
        SelectObject(memDC, oB); SelectObject(memDC, oP);
        DeleteObject(gB); DeleteObject(gP); DeleteObject(shB); DeleteObject(rnP); DeleteObject(coreB);
    }

    if (e->slowed) {
        HPEN frP = CreatePen(PS_SOLID, 1, RGB(56, 189, 248));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));
        HPEN oP = (HPEN)SelectObject(memDC, frP);
        Ellipse(memDC, ex - r - 2, y - r - 2, ex + r + 2, y + r + 2);
        SelectObject(memDC, oB); SelectObject(memDC, oP); DeleteObject(frP);
    }
    if (e->poisonTicks > 0) {
        HBRUSH pB = CreateSolidBrush(RGB(34, 197, 94));
        HPEN pP = CreatePen(PS_NULL, 0, 0);
        HBRUSH oB = (HBRUSH)SelectObject(memDC, pB); HPEN oP = (HPEN)SelectObject(memDC, pP);
        Ellipse(memDC, ex - 3, y - r - 5, ex, y - r - 2);
        Ellipse(memDC, ex + 2, y - r - 6, ex + 5, y - r - 3);
        SelectObject(memDC, oB); SelectObject(memDC, oP);
        DeleteObject(pB); DeleteObject(pP);
    }
}

void Render(HDC hdc, HWND hwnd) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int w = clientRect.right;
    int h = clientRect.bottom;

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, w, h);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    HBRUSH bgBrush = CreateSolidBrush(BG_COLOR);
    FillRect(memDC, &clientRect, bgBrush);
    DeleteObject(bgBrush);

    int dpi = GetDpiForWindow(hwnd);
    if (dpi == 0) dpi = 96;

    DrawRoundedRect(memDC, 10, 10, w - 10, 60, CARD_BG, BORDER_COLOR, 8);

    HFONT hFontTitle = CreateFontA(-MulDiv(20, dpi, 72), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    HFONT hFontSub = CreateFontA(-MulDiv(11, dpi, 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    HFONT hFontStat = CreateFontA(-MulDiv(14, dpi, 72), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

    HFONT oldFont = (HFONT)SelectObject(memDC, hFontTitle);
    SetBkMode(memDC, TRANSPARENT);
    SetTextColor(memDC, TEXT_GOLD);
    TextOutA(memDC, 25, 20, "KFORTRESS", 9);

    SelectObject(memDC, hFontSub);
    SetTextColor(memDC, TEXT_MUTED);
    TextOutA(memDC, 150, 26, "Loop 8: Citadel Visuals & Balance", 33);

    SelectObject(memDC, hFontStat);
    char buf[64];
    wsprintfA(buf, "Gold: %d", g_gold);
    SetTextColor(memDC, TEXT_GOLD);
    TextOutA(memDC, w - 380, 24, buf, (int)lstrlenA(buf));

    wsprintfA(buf, "Base HP: %d/%d", g_baseHp, g_maxBaseHp);
    SetTextColor(memDC, TEXT_RED);
    TextOutA(memDC, w - 270, 24, buf, (int)lstrlenA(buf));

    wsprintfA(buf, "Wave: %d", g_wave);
    SetTextColor(memDC, TEXT_WHITE);
    TextOutA(memDC, w - 140, 24, buf, (int)lstrlenA(buf));

    if (g_mutators != 0) {
        DrawRoundedRect(memDC, w - 520, 20, w - 410, 45, RGB(168, 85, 247), BORDER_COLOR, 4);
        SetTextColor(memDC, TEXT_WHITE);
        TextOutA(memDC, w - 510, 23, "MUTATORS ON", 11);
    }

    DrawRoundedRect(memDC, w - 95, 20, w - 15, 45, RGB(59, 130, 246), BORDER_COLOR, 4);
    SetTextColor(memDC, TEXT_WHITE);
    TextOutA(memDC, w - 90, 23, "GUIDE [F1]", 10);

    DeleteObject(hFontTitle);
    DeleteObject(hFontSub);
    DeleteObject(hFontStat);

    int bfX = 10, bfY = 70, bfW = w - 220, bfH = h - 80;
    
    // Continuous Physics-Decay Screen Shake
    if (g_shakeIntensity > 0.0f) {
        bfX += (int)(custom_cosf(g_shakeAngle) * g_shakeIntensity);
        bfY += (int)(custom_sinf(g_shakeAngle * 1.3f) * g_shakeIntensity);
    }
    
    DrawRoundedRect(memDC, bfX, bfY, bfX + bfW, bfY + bfH, g_maps[g_currentMap].bg, BORDER_COLOR, 8);

    // Floating Usability Toast Banner
    if (g_toastTimer > 0) {
        int tLen = lstrlenA(g_toastText);
        int tW = tLen * 7 + 24;
        if (tW < 240) tW = 240;
        int tX = bfX + (bfW - tW) / 2;
        int tY = bfY + 8;
        DrawRoundedRect(memDC, tX, tY, tX + tW, tY + 26, RGB(15, 23, 42), g_toastColor, 5);
        SetTextColor(memDC, g_toastColor);
        TextOutA(memDC, tX + 12, tY + 5, g_toastText, tLen);
    }

    // Ambient Weather Particles
    for (int p = 0; p < 80; p++) {
        if (g_weatherParticles[p].active) {
            int wx = bfX + (int)g_weatherParticles[p].x;
            int wy = bfY + (int)g_weatherParticles[p].y;
            int wsz = (int)g_weatherParticles[p].size;
            if (wx >= bfX && wx <= bfX + bfW && wy >= bfY && wy <= bfY + bfH) {
                HBRUSH wB = CreateSolidBrush(g_weatherParticles[p].color);
                HPEN wP = CreatePen(PS_NULL, 0, 0);
                HBRUSH oB = (HBRUSH)SelectObject(memDC, wB); HPEN oP = (HPEN)SelectObject(memDC, wP);
                Ellipse(memDC, wx - wsz, wy - wsz, wx + wsz, wy + wsz);
                SelectObject(memDC, oB); SelectObject(memDC, oP);
                DeleteObject(wB); DeleteObject(wP);
            }
        }
    }

    // Environmental Art (Rocks, Trees, Props)
    for (int i = 0; i < g_envArtCount; i++) {
        int ex = (int)g_envArt[i].x;
        int ey = (int)g_envArt[i].y;
        int sz = (int)g_envArt[i].size;
        if (ex - sz < bfX || ex + sz > bfX + bfW || ey - sz < bfY || ey + sz > bfY + bfH) continue;

        if (g_envArt[i].type == 0) {
            POINT rPts[] = {{ex - sz, ey + sz/2}, {ex + sz, ey + sz/2}, {ex, ey - sz}};
            HBRUSH rB = CreateSolidBrush(RGB(71, 85, 105)); HPEN rP = CreatePen(PS_NULL, 0, 0);
            HBRUSH oB = (HBRUSH)SelectObject(memDC, rB); HPEN oP = (HPEN)SelectObject(memDC, rP);
            Polygon(memDC, rPts, 3);
            SelectObject(memDC, oB); SelectObject(memDC, oP); DeleteObject(rB); DeleteObject(rP);
        } else {
            HBRUSH gB = CreateSolidBrush(RGB(22, 101, 52)); HPEN gP = CreatePen(PS_NULL, 0, 0);
            HBRUSH oB = (HBRUSH)SelectObject(memDC, gB); HPEN oP = (HPEN)SelectObject(memDC, gP);
            Pie(memDC, ex - sz, ey - sz, ex + sz, ey + sz, ex + sz, ey, ex - sz, ey);
            SelectObject(memDC, oB); SelectObject(memDC, oP); DeleteObject(gB); DeleteObject(gP);
        }
    }

    HFONT hObsFont = CreateFontA(-MulDiv(20, dpi, 72), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SelectObject(memDC, hObsFont);
    SetTextColor(memDC, RGB(255,255,255));
    SetBkMode(memDC, TRANSPARENT);
    for (int i=0; i<g_maps[g_currentMap].numObs; i++) {
        Obstacle* obs = &g_maps[g_currentMap].obs[i];
        TextOutA(memDC, bfX + obs->x - 10, bfY + obs->y - 20, obs->type, lstrlenA(obs->type));
    }
    DeleteObject(hObsFont);

    POINT pts[MAX_WAYPOINTS];
    for (int i = 0; i < MAX_WAYPOINTS; i++) {
        pts[i].x = g_waypoints[i].x;
        pts[i].y = g_waypoints[i].y;
    }

    HPEN pathPen = CreatePen(PS_SOLID, 44, g_maps[g_currentMap].path);
    HPEN oldPen = (HPEN)SelectObject(memDC, pathPen);
    Polyline(memDC, pts, MAX_WAYPOINTS);

    HPEN pathBorderPen = CreatePen(PS_SOLID, 2, BORDER_COLOR);
    SelectObject(memDC, pathBorderPen);
    Polyline(memDC, pts, MAX_WAYPOINTS);
    SelectObject(memDC, oldPen);
    DeleteObject(pathPen);
    DeleteObject(pathBorderPen);

    for (int i = 0; i < MAX_LAVA_POOLS; i++) {
        if (g_lavaPools[i].active) {
            float lx = g_lavaPools[i].x;
            float ly = g_lavaPools[i].y;
            float lr = (float)g_lavaPools[i].radius;
            HBRUSH lpB = CreateSolidBrush(RGB(234, 88, 12));
            HPEN lpP = CreatePen(PS_SOLID, 2, RGB(251, 146, 60));
            HBRUSH ob = (HBRUSH)SelectObject(memDC, lpB); HPEN op = (HPEN)SelectObject(memDC, lpP);
            Ellipse(memDC, (int)(lx - lr), (int)(ly - lr), (int)(lx + lr), (int)(ly + lr));
            
            // Inner Core
            HBRUSH icB = CreateSolidBrush(RGB(254, 240, 138));
            SelectObject(memDC, icB);
            Ellipse(memDC, (int)(lx - lr*0.5f), (int)(ly - lr*0.5f), (int)(lx + lr*0.5f), (int)(ly + lr*0.5f));
            SelectObject(memDC, ob); SelectObject(memDC, op);
            DeleteObject(icB); DeleteObject(lpB); DeleteObject(lpP);
        }
    }

    for (int i = 0; i < MAX_SCORCH_MARKS; i++) {
        if (g_scorchMarks[i].active) {
            float sx = g_scorchMarks[i].x;
            float sy = g_scorchMarks[i].y;
            float sr = g_scorchMarks[i].radius;
            
            HBRUSH sm1 = CreateSolidBrush(g_scorchMarks[i].color);
            HPEN np = CreatePen(PS_NULL, 0, 0);
            HBRUSH ob = (HBRUSH)SelectObject(memDC, sm1);
            HPEN op = (HPEN)SelectObject(memDC, np);
            Ellipse(memDC, (int)(sx - sr), (int)(sy - sr), (int)(sx + sr), (int)(sy + sr));
            SelectObject(memDC, ob); SelectObject(memDC, op);
            DeleteObject(sm1); DeleteObject(np);
        }
    }

    if (g_selectedSlot != -1) {
        HPEN glowPen = CreatePen(PS_SOLID, 2, TEXT_GOLD);
        HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
        int sx = g_slots[g_selectedSlot].x;
        int sy = g_slots[g_selectedSlot].y;
        int r = g_slots[g_selectedSlot].range;
        
        HPEN oP = (HPEN)SelectObject(memDC, glowPen);
        HBRUSH oB = (HBRUSH)SelectObject(memDC, nullBrush);
        Ellipse(memDC, sx - r, sy - r, sx + r, sy + r);
        SelectObject(memDC, oP); SelectObject(memDC, oB);
        DeleteObject(glowPen);
    }

    // Nether Rift Spawn Gate (Obsidian frame, twin runic obelisks, pulsing vortex core)
    int gateX = g_waypoints[0].x, gateY = g_waypoints[0].y;
    DrawRoundedRect(memDC, gateX - 24, gateY - 24, gateX + 24, gateY + 24, RGB(15, 23, 42), RGB(88, 28, 135), 8);
    DrawRoundedRect(memDC, gateX - 22, gateY - 20, gateX - 14, gateY + 20, RGB(30, 41, 59), RGB(239, 68, 68), 2);
    DrawRoundedRect(memDC, gateX + 14, gateY - 20, gateX + 22, gateY + 20, RGB(30, 41, 59), RGB(239, 68, 68), 2);
    HBRUSH vorB = CreateSolidBrush(RGB(153, 27, 27));
    HPEN vorP = CreatePen(PS_SOLID, 2, RGB(239, 68, 68));
    HBRUSH oVorB = (HBRUSH)SelectObject(memDC, vorB); HPEN oVorP = (HPEN)SelectObject(memDC, vorP);
    Ellipse(memDC, gateX - 11, gateY - 16, gateX + 11, gateY + 16);
    int eyePulse = (int)(custom_sinf(g_globalFrame * 0.2f) * 2.0f);
    HBRUSH eyeB = CreateSolidBrush(RGB(254, 202, 202));
    SelectObject(memDC, eyeB);
    Ellipse(memDC, gateX - 4 - eyePulse, gateY - 7 - eyePulse, gateX + 4 + eyePulse, gateY + 7 + eyePulse);
    SelectObject(memDC, oVorB); SelectObject(memDC, oVorP);
    DeleteObject(vorB); DeleteObject(vorP); DeleteObject(eyeB);
    SetTextColor(memDC, RGB(254, 202, 202));
    TextOutA(memDC, gateX - 14, gateY - 6, "RIFT", 4);

    // Royal Citadel Castle Keep with ashlar masonry, watchtowers, portcullis, and fluttering banner
    int cx = g_waypoints[5].x, cy = g_waypoints[5].y;
    DrawRoundedRect(memDC, cx - 32, cy - 30, cx + 36, cy + 34, CASTLE_COLOR, TEXT_GOLD, 8);
    HPEN masP = CreatePen(PS_SOLID, 1, RGB(51, 65, 85));
    HPEN oMasP = (HPEN)SelectObject(memDC, masP);
    MoveToEx(memDC, cx - 28, cy - 10, NULL); LineTo(memDC, cx + 32, cy - 10);
    MoveToEx(memDC, cx - 28, cy + 8, NULL); LineTo(memDC, cx + 32, cy + 8);
    SelectObject(memDC, oMasP); DeleteObject(masP);
    DrawRoundedRect(memDC, cx - 34, cy - 28, cx - 20, cy + 24, RGB(71, 85, 105), TEXT_GOLD, 3);
    DrawRoundedRect(memDC, cx + 24, cy - 28, cx + 38, cy + 24, RGB(71, 85, 105), TEXT_GOLD, 3);
    DrawRoundedRect(memDC, cx - 10, cy + 10, cx + 14, cy + 34, RGB(15, 23, 42), RGB(100, 116, 139), 4);
    for (int bx = -30; bx < 32; bx += 12) {
        DrawRoundedRect(memDC, cx + bx, cy - 34, cx + bx + 7, cy - 28, RGB(100, 116, 139), TEXT_GOLD, 1);
    }
    int banWave = (int)(custom_sinf(g_globalFrame * 0.25f) * 3.0f);
    HPEN poleP = CreatePen(PS_SOLID, 2, RGB(217, 119, 6));
    HPEN oPolP = (HPEN)SelectObject(memDC, poleP);
    MoveToEx(memDC, cx + 2, cy - 34, NULL); LineTo(memDC, cx + 2, cy - 48);
    SelectObject(memDC, oPolP); DeleteObject(poleP);
    HBRUSH banB = CreateSolidBrush(RGB(59, 130, 246));
    HPEN banP = CreatePen(PS_SOLID, 1, TEXT_GOLD);
    HBRUSH oBanB = (HBRUSH)SelectObject(memDC, banB); HPEN oBanP = (HPEN)SelectObject(memDC, banP);
    POINT banPts[3] = { {cx + 2, cy - 48}, {cx + 18 + banWave, cy - 43}, {cx + 2, cy - 38} };
    Polygon(memDC, banPts, 3);
    SelectObject(memDC, oBanB); SelectObject(memDC, oBanP);
    DeleteObject(banB); DeleteObject(banP);
    SetTextColor(memDC, TEXT_GOLD);
    TextOutA(memDC, cx - 22, cy - 6, "CITADEL", 7);

    for (int i = 0; i < g_slotCount; i++) {
        COLORREF fill = (g_selectedSlot == i) ? TOWER_SLOT_HOVER : TOWER_SLOT_BG;
        COLORREF border = (g_selectedSlot == i) ? TEXT_GOLD : BORDER_COLOR;
        DrawRoundedRect(memDC, g_slots[i].x - 22, g_slots[i].y - 22, g_slots[i].x + 22, g_slots[i].y + 22, fill, border, 6);

        if (g_slots[i].occupied) {
            int anim = g_slots[i].attackAnim;
            int type = g_slots[i].towerType;

            int lvl = g_slots[i].level;

            if (type == TOWER_ARCHER) {
                // Base Timber platform
                HBRUSH bB = CreateSolidBrush((lvl >= 3) ? RGB(71, 85, 105) : RGB(120, 53, 15));
                HPEN bP = CreatePen(PS_SOLID, 1, (lvl >= 3) ? TEXT_GOLD : RGB(69, 26, 3));
                HBRUSH oB = (HBRUSH)SelectObject(memDC, bB); HPEN oP = (HPEN)SelectObject(memDC, bP);
                Rectangle(memDC, g_slots[i].x - 14, g_slots[i].y - 14, g_slots[i].x + 14, g_slots[i].y + 14);

                // Corner timber / iron posts
                HBRUSH pB = CreateSolidBrush((lvl >= 2) ? RGB(148, 163, 184) : RGB(180, 83, 9));
                SelectObject(memDC, pB);
                Rectangle(memDC, g_slots[i].x - 14, g_slots[i].y - 14, g_slots[i].x - 9, g_slots[i].y - 9);
                Rectangle(memDC, g_slots[i].x + 9, g_slots[i].y - 14, g_slots[i].x + 14, g_slots[i].y - 9);
                Rectangle(memDC, g_slots[i].x - 14, g_slots[i].y + 9, g_slots[i].x - 9, g_slots[i].y + 14);
                Rectangle(memDC, g_slots[i].x + 9, g_slots[i].y + 9, g_slots[i].x + 14, g_slots[i].y + 14);

                // Bows: L1=single recurve bow, L2=dual arbalests, L3=triple gilded arbalests + pennants
                HPEN bowP = CreatePen(PS_SOLID, (lvl >= 3) ? 2 : 1, (lvl >= 3) ? TEXT_GOLD : RGB(217, 119, 6));
                SelectObject(memDC, bowP);
                if (lvl == 1) {
                    Arc(memDC, g_slots[i].x - 8, g_slots[i].y - 8, g_slots[i].x + 8, g_slots[i].y + 8, g_slots[i].x, g_slots[i].y - 8, g_slots[i].x, g_slots[i].y + 8);
                } else if (lvl == 2) {
                    Arc(memDC, g_slots[i].x - 9, g_slots[i].y - 9, g_slots[i].x + 5, g_slots[i].y + 5, g_slots[i].x - 2, g_slots[i].y - 9, g_slots[i].x - 2, g_slots[i].y + 5);
                    Arc(memDC, g_slots[i].x - 5, g_slots[i].y - 5, g_slots[i].x + 9, g_slots[i].y + 9, g_slots[i].x + 2, g_slots[i].y - 5, g_slots[i].x + 2, g_slots[i].y + 9);
                } else {
                    Arc(memDC, g_slots[i].x - 9, g_slots[i].y - 9, g_slots[i].x + 3, g_slots[i].y + 3, g_slots[i].x - 3, g_slots[i].y - 9, g_slots[i].x - 3, g_slots[i].y + 3);
                    Arc(memDC, g_slots[i].x - 6, g_slots[i].y - 6, g_slots[i].x + 6, g_slots[i].y + 6, g_slots[i].x, g_slots[i].y - 6, g_slots[i].x, g_slots[i].y + 6);
                    Arc(memDC, g_slots[i].x - 3, g_slots[i].y - 3, g_slots[i].x + 9, g_slots[i].y + 9, g_slots[i].x + 3, g_slots[i].y - 3, g_slots[i].x + 3, g_slots[i].y + 9);
                    // Archer pennant
                    MoveToEx(memDC, g_slots[i].x - 11, g_slots[i].y - 12, NULL); LineTo(memDC, g_slots[i].x - 11, g_slots[i].y - 17);
                    LineTo(memDC, g_slots[i].x - 6, g_slots[i].y - 15); LineTo(memDC, g_slots[i].x - 11, g_slots[i].y - 13);
                }
                SelectObject(memDC, oB); SelectObject(memDC, oP); DeleteObject(bB); DeleteObject(bP); DeleteObject(pB); DeleteObject(bowP);
            } else if (type == TOWER_MAGE) {
                // Arcane obsidian base & amethyst crystals: L1=single shard, L2=twin shards + rune ring, L3=tripartite prisms + halo
                HBRUSH mB = CreateSolidBrush(RGB(59, 7, 100)); HPEN mP = CreatePen(PS_SOLID, (lvl >= 2) ? 2 : 1, (lvl >= 3) ? TEXT_GOLD : RGB(168, 85, 247));
                HBRUSH oB = (HBRUSH)SelectObject(memDC, mB); HPEN oP = (HPEN)SelectObject(memDC, mP);
                Ellipse(memDC, g_slots[i].x - 14, g_slots[i].y - 14, g_slots[i].x + 14, g_slots[i].y + 14);

                if (lvl >= 2) {
                    HPEN rnP = CreatePen(PS_SOLID, 1, RGB(192, 132, 252));
                    SelectObject(memDC, rnP);
                    Ellipse(memDC, g_slots[i].x - 10, g_slots[i].y - 10, g_slots[i].x + 10, g_slots[i].y + 10);
                    DeleteObject(rnP);
                }

                HBRUSH crB = CreateSolidBrush((lvl >= 3) ? RGB(245, 208, 254) : RGB(192, 132, 252));
                SelectObject(memDC, crB);
                if (lvl == 1) {
                    POINT pts[4] = { {g_slots[i].x, g_slots[i].y - 9}, {g_slots[i].x + 6, g_slots[i].y}, {g_slots[i].x, g_slots[i].y + 9}, {g_slots[i].x - 6, g_slots[i].y} };
                    Polygon(memDC, pts, 4);
                } else if (lvl == 2) {
                    POINT pts1[4] = { {g_slots[i].x - 4, g_slots[i].y - 8}, {g_slots[i].x, g_slots[i].y - 2}, {g_slots[i].x - 4, g_slots[i].y + 4}, {g_slots[i].x - 8, g_slots[i].y - 2} };
                    POINT pts2[4] = { {g_slots[i].x + 4, g_slots[i].y - 4}, {g_slots[i].x + 8, g_slots[i].y + 2}, {g_slots[i].x + 4, g_slots[i].y + 8}, {g_slots[i].x, g_slots[i].y + 2} };
                    Polygon(memDC, pts1, 4); Polygon(memDC, pts2, 4);
                } else {
                    POINT pts1[4] = { {g_slots[i].x, g_slots[i].y - 10}, {g_slots[i].x + 4, g_slots[i].y - 3}, {g_slots[i].x, g_slots[i].y + 2}, {g_slots[i].x - 4, g_slots[i].y - 3} };
                    POINT pts2[4] = { {g_slots[i].x - 6, g_slots[i].y + 1}, {g_slots[i].x - 2, g_slots[i].y + 8}, {g_slots[i].x - 6, g_slots[i].y + 11}, {g_slots[i].x - 10, g_slots[i].y + 5} };
                    POINT pts3[4] = { {g_slots[i].x + 6, g_slots[i].y + 1}, {g_slots[i].x + 10, g_slots[i].y + 5}, {g_slots[i].x + 6, g_slots[i].y + 11}, {g_slots[i].x + 2, g_slots[i].y + 8} };
                    Polygon(memDC, pts1, 4); Polygon(memDC, pts2, 4); Polygon(memDC, pts3, 4);
                }

                // Orbiting mana spark
                HBRUSH spB = CreateSolidBrush(RGB(245, 208, 254));
                SelectObject(memDC, spB);
                int sx = g_slots[i].x + (int)(custom_cosf(g_globalFrame * 0.1f) * 11.0f);
                int sy = g_slots[i].y + (int)(custom_sinf(g_globalFrame * 0.1f) * 11.0f);
                Ellipse(memDC, sx - 2, sy - 2, sx + 2, sy + 2);
                if (lvl >= 2) {
                    int sx2 = g_slots[i].x + (int)(custom_cosf(g_globalFrame * 0.1f + 3.14f) * 11.0f);
                    int sy2 = g_slots[i].y + (int)(custom_sinf(g_globalFrame * 0.1f + 3.14f) * 11.0f);
                    Ellipse(memDC, sx2 - 2, sy2 - 2, sx2 + 2, sy2 + 2);
                }
                SelectObject(memDC, oB); SelectObject(memDC, oP); DeleteObject(mB); DeleteObject(mP); DeleteObject(crB); DeleteObject(spB);
            } else if (type == TOWER_CANNON) {
                // Cannon: L1=single barrel, L2=twin battery barrels, L3=triple siege mortar battery
                HBRUSH cB = CreateSolidBrush(RGB(71, 85, 105)); HPEN cP = CreatePen(PS_SOLID, 1, RGB(148, 163, 184));
                HBRUSH oB = (HBRUSH)SelectObject(memDC, cB); HPEN oP = (HPEN)SelectObject(memDC, cP);
                Ellipse(memDC, g_slots[i].x - 14, g_slots[i].y - 14, g_slots[i].x + 14, g_slots[i].y + 14);

                HBRUSH brB = CreateSolidBrush(RGB(30, 41, 59));
                SelectObject(memDC, brB);
                HBRUSH mzB = CreateSolidBrush((lvl >= 3) ? TEXT_GOLD : RGB(217, 119, 6));

                if (lvl == 1) {
                    Rectangle(memDC, g_slots[i].x - 8, g_slots[i].y - 5, g_slots[i].x + 9, g_slots[i].y + 5);
                    SelectObject(memDC, mzB);
                    Rectangle(memDC, g_slots[i].x + 7, g_slots[i].y - 6, g_slots[i].x + 10, g_slots[i].y + 6);
                } else if (lvl == 2) {
                    Rectangle(memDC, g_slots[i].x - 8, g_slots[i].y - 7, g_slots[i].x + 9, g_slots[i].y - 2);
                    Rectangle(memDC, g_slots[i].x - 8, g_slots[i].y + 2, g_slots[i].x + 9, g_slots[i].y + 7);
                    SelectObject(memDC, mzB);
                    Rectangle(memDC, g_slots[i].x + 7, g_slots[i].y - 8, g_slots[i].x + 10, g_slots[i].y - 1);
                    Rectangle(memDC, g_slots[i].x + 7, g_slots[i].y + 1, g_slots[i].x + 10, g_slots[i].y + 8);
                } else {
                    Rectangle(memDC, g_slots[i].x - 8, g_slots[i].y - 8, g_slots[i].x + 10, g_slots[i].y - 4);
                    Rectangle(memDC, g_slots[i].x - 8, g_slots[i].y - 2, g_slots[i].x + 11, g_slots[i].y + 2);
                    Rectangle(memDC, g_slots[i].x - 8, g_slots[i].y + 4, g_slots[i].x + 10, g_slots[i].y + 8);
                    SelectObject(memDC, mzB);
                    Rectangle(memDC, g_slots[i].x + 8, g_slots[i].y - 9, g_slots[i].x + 11, g_slots[i].y - 3);
                    Rectangle(memDC, g_slots[i].x + 9, g_slots[i].y - 3, g_slots[i].x + 12, g_slots[i].y + 3);
                    Rectangle(memDC, g_slots[i].x + 8, g_slots[i].y + 3, g_slots[i].x + 11, g_slots[i].y + 9);
                }

                // Stack of cannonballs
                HBRUSH blB = CreateSolidBrush(RGB(15, 23, 42));
                SelectObject(memDC, blB);
                Ellipse(memDC, g_slots[i].x - 8, g_slots[i].y + 4, g_slots[i].x - 2, g_slots[i].y + 10);
                Ellipse(memDC, g_slots[i].x - 3, g_slots[i].y + 4, g_slots[i].x + 3, g_slots[i].y + 10);
                if (lvl >= 2) Ellipse(memDC, g_slots[i].x - 5, g_slots[i].y - 1, g_slots[i].x + 1, g_slots[i].y + 5);
                SelectObject(memDC, oB); SelectObject(memDC, oP); DeleteObject(cB); DeleteObject(cP); DeleteObject(brB); DeleteObject(mzB); DeleteObject(blB);
            } else if (type == TOWER_FROST) {
                // Frost: L1=pedestal+obelisk, L2=twin stalagmites+frost ring, L3=glacial monolith+4 crystal shards
                HBRUSH fB = CreateSolidBrush(RGB(3, 105, 161)); HPEN fP = CreatePen(PS_SOLID, 1, RGB(56, 189, 248));
                HBRUSH oB = (HBRUSH)SelectObject(memDC, fB); HPEN oP = (HPEN)SelectObject(memDC, fP);
                Ellipse(memDC, g_slots[i].x - 14, g_slots[i].y - 14, g_slots[i].x + 14, g_slots[i].y + 14);

                HBRUSH icB = CreateSolidBrush(RGB(186, 230, 253));
                SelectObject(memDC, icB);
                if (lvl == 1) {
                    POINT pts[4] = { {g_slots[i].x, g_slots[i].y - 11}, {g_slots[i].x + 6, g_slots[i].y + 2}, {g_slots[i].x, g_slots[i].y + 10}, {g_slots[i].x - 6, g_slots[i].y + 2} };
                    Polygon(memDC, pts, 4);
                } else if (lvl == 2) {
                    POINT p1[4] = { {g_slots[i].x - 3, g_slots[i].y - 12}, {g_slots[i].x + 2, g_slots[i].y}, {g_slots[i].x - 3, g_slots[i].y + 8}, {g_slots[i].x - 7, g_slots[i].y} };
                    POINT p2[4] = { {g_slots[i].x + 4, g_slots[i].y - 8}, {g_slots[i].x + 8, g_slots[i].y + 3}, {g_slots[i].x + 4, g_slots[i].y + 11}, {g_slots[i].x, g_slots[i].y + 3} };
                    Polygon(memDC, p1, 4); Polygon(memDC, p2, 4);
                } else {
                    POINT p1[4] = { {g_slots[i].x, g_slots[i].y - 13}, {g_slots[i].x + 5, g_slots[i].y}, {g_slots[i].x, g_slots[i].y + 10}, {g_slots[i].x - 5, g_slots[i].y} };
                    Polygon(memDC, p1, 4);
                    // 4 levitating ice shards
                    Ellipse(memDC, g_slots[i].x - 10, g_slots[i].y - 10, g_slots[i].x - 6, g_slots[i].y - 6);
                    Ellipse(memDC, g_slots[i].x + 6, g_slots[i].y - 10, g_slots[i].x + 10, g_slots[i].y - 6);
                    Ellipse(memDC, g_slots[i].x - 10, g_slots[i].y + 6, g_slots[i].x - 6, g_slots[i].y + 10);
                    Ellipse(memDC, g_slots[i].x + 6, g_slots[i].y + 6, g_slots[i].x + 10, g_slots[i].y + 10);
                }
                SelectObject(memDC, oB); SelectObject(memDC, oP); DeleteObject(fB); DeleteObject(fP); DeleteObject(icB);
            } else if (type == TOWER_TESLA) {
                // Tesla: L1=pylon+dual electrodes, L2=quad coil stack+lightning spire, L3=dynamo gyro-ring+4 spark nodes
                HBRUSH tB = CreateSolidBrush(RGB(51, 65, 85)); HPEN tP = CreatePen(PS_SOLID, 1, RGB(100, 116, 139));
                HBRUSH oB = (HBRUSH)SelectObject(memDC, tB); HPEN oP = (HPEN)SelectObject(memDC, tP);
                Rectangle(memDC, g_slots[i].x - 12, g_slots[i].y - 12, g_slots[i].x + 12, g_slots[i].y + 12);

                HBRUSH cpB = CreateSolidBrush(RGB(180, 83, 9));
                SelectObject(memDC, cpB);
                Rectangle(memDC, g_slots[i].x - 6, g_slots[i].y - 7, g_slots[i].x + 6, g_slots[i].y - 4);
                Rectangle(memDC, g_slots[i].x - 6, g_slots[i].y - 2, g_slots[i].x + 6, g_slots[i].y + 1);
                Rectangle(memDC, g_slots[i].x - 6, g_slots[i].y + 3, g_slots[i].x + 6, g_slots[i].y + 6);
                if (lvl >= 2) Rectangle(memDC, g_slots[i].x - 8, g_slots[i].y - 10, g_slots[i].x + 8, g_slots[i].y - 8);

                HBRUSH elB = CreateSolidBrush((lvl >= 3) ? RGB(254, 240, 138) : TEXT_GOLD);
                SelectObject(memDC, elB);
                Ellipse(memDC, g_slots[i].x - 8, g_slots[i].y - 10, g_slots[i].x - 2, g_slots[i].y - 4);
                Ellipse(memDC, g_slots[i].x + 2, g_slots[i].y - 10, g_slots[i].x + 8, g_slots[i].y - 4);
                if (lvl >= 3) {
                    Ellipse(memDC, g_slots[i].x - 11, g_slots[i].y - 1, g_slots[i].x - 6, g_slots[i].y + 4);
                    Ellipse(memDC, g_slots[i].x + 6, g_slots[i].y - 1, g_slots[i].x + 11, g_slots[i].y + 4);
                }
                SelectObject(memDC, oB); SelectObject(memDC, oP); DeleteObject(tB); DeleteObject(tP); DeleteObject(cpB); DeleteObject(elB);
            } else if (type == TOWER_BALLISTA) {
                // Ballista: L1=single limb+bolt, L2=compound limbs+dual carriage, L3=heavy scorpion+triple harpoons
                HBRUSH blB = CreateSolidBrush(RGB(87, 48, 16)); HPEN blP = CreatePen(PS_SOLID, 1, TEXT_GOLD);
                HBRUSH oB = (HBRUSH)SelectObject(memDC, blB); HPEN oP = (HPEN)SelectObject(memDC, blP);
                Ellipse(memDC, g_slots[i].x - 14, g_slots[i].y - 14, g_slots[i].x + 14, g_slots[i].y + 14);

                HPEN limbP = CreatePen(PS_SOLID, (lvl >= 2) ? 3 : 2, RGB(146, 64, 14));
                SelectObject(memDC, limbP);
                MoveToEx(memDC, g_slots[i].x - 9, g_slots[i].y - 8, NULL);
                LineTo(memDC, g_slots[i].x - 2, g_slots[i].y);
                LineTo(memDC, g_slots[i].x - 9, g_slots[i].y + 8);
                if (lvl >= 2) {
                    MoveToEx(memDC, g_slots[i].x - 12, g_slots[i].y - 10, NULL);
                    LineTo(memDC, g_slots[i].x - 5, g_slots[i].y);
                    LineTo(memDC, g_slots[i].x - 12, g_slots[i].y + 10);
                }

                HPEN boltP = CreatePen(PS_SOLID, 2, (lvl >= 3) ? TEXT_GOLD : RGB(226, 232, 240));
                SelectObject(memDC, boltP);
                if (lvl == 1) {
                    MoveToEx(memDC, g_slots[i].x - 6, g_slots[i].y, NULL); LineTo(memDC, g_slots[i].x + 11, g_slots[i].y);
                } else if (lvl == 2) {
                    MoveToEx(memDC, g_slots[i].x - 6, g_slots[i].y - 3, NULL); LineTo(memDC, g_slots[i].x + 11, g_slots[i].y - 3);
                    MoveToEx(memDC, g_slots[i].x - 6, g_slots[i].y + 3, NULL); LineTo(memDC, g_slots[i].x + 11, g_slots[i].y + 3);
                } else {
                    MoveToEx(memDC, g_slots[i].x - 6, g_slots[i].y - 5, NULL); LineTo(memDC, g_slots[i].x + 11, g_slots[i].y - 5);
                    MoveToEx(memDC, g_slots[i].x - 6, g_slots[i].y, NULL); LineTo(memDC, g_slots[i].x + 12, g_slots[i].y);
                    MoveToEx(memDC, g_slots[i].x - 6, g_slots[i].y + 5, NULL); LineTo(memDC, g_slots[i].x + 11, g_slots[i].y + 5);
                }
                SelectObject(memDC, oB); SelectObject(memDC, oP); DeleteObject(blB); DeleteObject(blP); DeleteObject(limbP); DeleteObject(boltP);
            } else if (type == TOWER_POISON) {
                // Poison: L1=mortar+flask, L2=dual alembics+pipe, L3=triple vats+luminescent conduits
                HBRUSH pB = CreateSolidBrush(RGB(20, 83, 45)); HPEN pP = CreatePen(PS_SOLID, 1, RGB(34, 197, 94));
                HBRUSH oB = (HBRUSH)SelectObject(memDC, pB); HPEN oP = (HPEN)SelectObject(memDC, pP);
                Ellipse(memDC, g_slots[i].x - 14, g_slots[i].y - 14, g_slots[i].x + 14, g_slots[i].y + 14);

                HBRUSH flB = CreateSolidBrush(RGB(22, 163, 74));
                SelectObject(memDC, flB);
                if (lvl == 1) {
                    Ellipse(memDC, g_slots[i].x - 7, g_slots[i].y - 5, g_slots[i].x + 7, g_slots[i].y + 7);
                    HBRUSH nkB = CreateSolidBrush(RGB(134, 239, 172));
                    SelectObject(memDC, nkB);
                    Rectangle(memDC, g_slots[i].x - 2, g_slots[i].y - 10, g_slots[i].x + 3, g_slots[i].y - 5);
                    DeleteObject(nkB);
                } else if (lvl == 2) {
                    Ellipse(memDC, g_slots[i].x - 9, g_slots[i].y - 4, g_slots[i].x - 1, g_slots[i].y + 6);
                    Ellipse(memDC, g_slots[i].x + 1, g_slots[i].y - 4, g_slots[i].x + 9, g_slots[i].y + 6);
                } else {
                    Ellipse(memDC, g_slots[i].x - 10, g_slots[i].y - 2, g_slots[i].x - 3, g_slots[i].y + 6);
                    Ellipse(memDC, g_slots[i].x - 3, g_slots[i].y - 8, g_slots[i].x + 4, g_slots[i].y - 1);
                    Ellipse(memDC, g_slots[i].x + 3, g_slots[i].y - 2, g_slots[i].x + 10, g_slots[i].y + 6);
                }
                SelectObject(memDC, oB); SelectObject(memDC, oP); DeleteObject(pB); DeleteObject(pP); DeleteObject(flB);
            } else if (type == TOWER_INFERNO) {
                // Basalt volcanic fortress with molten core & fire corona
                HBRUSH inB = CreateSolidBrush(RGB(69, 10, 10)); HPEN inP = CreatePen(PS_SOLID, 2, RGB(239, 68, 68));
                HBRUSH oB = (HBRUSH)SelectObject(memDC, inB); HPEN oP = (HPEN)SelectObject(memDC, inP);
                Ellipse(memDC, g_slots[i].x - 16, g_slots[i].y - 16, g_slots[i].x + 16, g_slots[i].y + 16);
                HBRUSH crB = CreateSolidBrush(RGB(249, 115, 22));
                SelectObject(memDC, crB);
                Ellipse(memDC, g_slots[i].x - 7, g_slots[i].y - 7, g_slots[i].x + 7, g_slots[i].y + 7);
                HBRUSH hB = CreateSolidBrush(RGB(254, 240, 138));
                SelectObject(memDC, hB);
                Ellipse(memDC, g_slots[i].x - 3, g_slots[i].y - 3, g_slots[i].x + 3, g_slots[i].y + 3);
                SelectObject(memDC, oB); SelectObject(memDC, oP); DeleteObject(inB); DeleteObject(inP); DeleteObject(crB); DeleteObject(hB);
            } else if (type == TOWER_SUPERCONDUCTOR) {
                // Arcane superconductor ring & hyper-conductive crystal
                HBRUSH scB = CreateSolidBrush(RGB(12, 74, 110)); HPEN scP = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
                HBRUSH oB = (HBRUSH)SelectObject(memDC, scB); HPEN oP = (HPEN)SelectObject(memDC, scP);
                Ellipse(memDC, g_slots[i].x - 16, g_slots[i].y - 16, g_slots[i].x + 16, g_slots[i].y + 16);
                HBRUSH plB = CreateSolidBrush(RGB(186, 230, 253));
                SelectObject(memDC, plB);
                Rectangle(memDC, g_slots[i].x - 6, g_slots[i].y - 6, g_slots[i].x + 6, g_slots[i].y + 6);
                HBRUSH whB = CreateSolidBrush(RGB(255, 255, 255));
                SelectObject(memDC, whB);
                Ellipse(memDC, g_slots[i].x - 3, g_slots[i].y - 3, g_slots[i].x + 3, g_slots[i].y + 3);
                SelectObject(memDC, oB); SelectObject(memDC, oP); DeleteObject(scB); DeleteObject(scP); DeleteObject(plB); DeleteObject(whB);
            } else if (type == TOWER_VENOMSPITE) {
                // Bone siege platform & double poison harpoons
                HBRUSH vnB = CreateSolidBrush(RGB(6, 78, 59)); HPEN vnP = CreatePen(PS_SOLID, 2, RGB(16, 185, 129));
                HBRUSH oB = (HBRUSH)SelectObject(memDC, vnB); HPEN oP = (HPEN)SelectObject(memDC, vnP);
                Rectangle(memDC, g_slots[i].x - 15, g_slots[i].y - 15, g_slots[i].x + 15, g_slots[i].y + 15);
                HPEN hpP = CreatePen(PS_SOLID, 2, RGB(34, 197, 94));
                SelectObject(memDC, hpP);
                MoveToEx(memDC, g_slots[i].x - 8, g_slots[i].y - 4, NULL); LineTo(memDC, g_slots[i].x + 10, g_slots[i].y - 4);
                MoveToEx(memDC, g_slots[i].x - 8, g_slots[i].y + 4, NULL); LineTo(memDC, g_slots[i].x + 10, g_slots[i].y + 4);
                SelectObject(memDC, oB); SelectObject(memDC, oP); DeleteObject(vnB); DeleteObject(vnP); DeleteObject(hpP);
            } else if (type == TOWER_SOLAR_BEAM) {
                // Golden armillary celestial lens
                HBRUSH sbB = CreateSolidBrush(RGB(113, 63, 18)); HPEN sbP = CreatePen(PS_SOLID, 2, TEXT_GOLD);
                HBRUSH oB = (HBRUSH)SelectObject(memDC, sbB); HPEN oP = (HPEN)SelectObject(memDC, sbP);
                Ellipse(memDC, g_slots[i].x - 16, g_slots[i].y - 16, g_slots[i].x + 16, g_slots[i].y + 16);
                HBRUSH suB = CreateSolidBrush(RGB(254, 240, 138));
                SelectObject(memDC, suB);
                Ellipse(memDC, g_slots[i].x - 6, g_slots[i].y - 6, g_slots[i].x + 6, g_slots[i].y + 6);
                SelectObject(memDC, oB); SelectObject(memDC, oP); DeleteObject(sbB); DeleteObject(sbP); DeleteObject(suB);
            }

            // Fusion Rotating Crown Aura
            if (lvl == 4) {
                HPEN crownPen = CreatePen(PS_SOLID, 1, RGB(254, 240, 138));
                HPEN oldCrown = (HPEN)SelectObject(memDC, crownPen);
                for (int c = 0; c < 4; c++) {
                    float cAng = g_globalFrame * 0.08f + c * 1.57f;
                    int cx1 = g_slots[i].x + (int)(custom_cosf(cAng) * 17.0f);
                    int cy1 = g_slots[i].y + (int)(custom_sinf(cAng) * 17.0f);
                    MoveToEx(memDC, cx1 - 2, cy1, NULL); LineTo(memDC, cx1 + 2, cy1);
                    MoveToEx(memDC, cx1, cy1 - 2, NULL); LineTo(memDC, cx1, cy1 + 2);
                }
                SelectObject(memDC, oldCrown); DeleteObject(crownPen);
            }
            
            SetTextColor(memDC, (g_slots[i].level == 4) ? RGB(254, 240, 138) : TEXT_GOLD);
            char lvlBuf[8];
            if (g_slots[i].level == 4) wsprintfA(lvlBuf, "FUS");
            else wsprintfA(lvlBuf, "L%d", g_slots[i].level);
            TextOutA(memDC, g_slots[i].x + 4, g_slots[i].y + 4, lvlBuf, lstrlenA(lvlBuf));
        } else {
            SetTextColor(memDC, TEXT_MUTED);
            TextOutA(memDC, g_slots[i].x - 4, g_slots[i].y - 12, "+", 1);
        }
    }

    for (int tr = 0; tr < MAX_TRAPS; tr++) {
        if (!g_traps[tr].active) continue;
        int tx = (int)g_traps[tr].x; int ty = (int)g_traps[tr].y; int trr = g_traps[tr].radius;
        DrawTrapSprite(memDC, tx, ty, trr, g_traps[tr].type, g_traps[tr].hp, g_traps[tr].charges);
    }

    for (int m = 0; m < MAX_MILITIA; m++) {
        if (!g_militia[m].active) continue;
        int mx = (int)g_militia[m].x;
        int my = (int)g_militia[m].y;
        int mBob = (int)(custom_sinf(g_globalFrame * 0.2f + m) * 1.5f);
        // Gambeson body
        DrawRoundedRect(memDC, mx - 7, my - 7 + mBob, mx + 7, my + 7 + mBob, RGB(120, 53, 15), RGB(69, 26, 3), 2);
        // Kettle helm
        DrawRoundedRect(memDC, mx - 8, my - 11 + mBob, mx + 8, my - 6 + mBob, RGB(100, 116, 139), RGB(71, 85, 105), 2);
        // Round shield
        HBRUSH shB = CreateSolidBrush(RGB(146, 64, 14)); HPEN shP = CreatePen(PS_SOLID, 1, RGB(226, 232, 240));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, shB); HPEN oP = (HPEN)SelectObject(memDC, shP);
        Ellipse(memDC, mx - 10, my - 3 + mBob, mx - 2, my + 5 + mBob);
        SelectObject(memDC, oB); SelectObject(memDC, oP); DeleteObject(shB); DeleteObject(shP);
        // Spear line
        HPEN spP = CreatePen(PS_SOLID, 2, RGB(180, 83, 9));
        HPEN oSpP = (HPEN)SelectObject(memDC, spP);
        MoveToEx(memDC, mx + 5, my + 8 + mBob, NULL); LineTo(memDC, mx + 5, my - 13 + mBob);
        SelectObject(memDC, oSpP); DeleteObject(spP);
        
        float mRatio = g_militia[m].hp / g_militia[m].maxHp;
        DrawRoundedRect(memDC, mx - 10, my - 14 + mBob, mx + 10, my - 11 + mBob, RGB(20,20,20), RGB(0,0,0), 0);
        HBRUSH mHpB = CreateSolidBrush(RGB(34, 197, 94));
        RECT mHpR = { mx - 10, my - 14 + mBob, mx - 10 + (int)(20 * mRatio), my - 11 + mBob };
        FillRect(memDC, &mHpR, mHpB); DeleteObject(mHpB);
    }

    for (int e = 0; e < MAX_ENEMIES; e++) {
        if (!g_enemies[e].active) continue;

        int ex = (int)g_enemies[e].x;
        int ey = (int)g_enemies[e].y;
        int r = g_enemies[e].radius;
        int t = g_enemies[e].type;
        int walkBob = (int)(custom_sinf(g_globalFrame * 0.25f + e) * 2.0f);

        DrawEnemySprite(memDC, &g_enemies[e], ex, ey, r, walkBob);

        int barW = (t == ENEMY_OGRE || t == ENEMY_WYVERN || t == ENEMY_GOLEM) ? 36 : 22;
        float hpRatio = (float)g_enemies[e].hp / (float)g_enemies[e].maxHp;
        DrawRoundedRect(memDC, ex - barW/2, ey + walkBob - r - 8, ex + barW/2, ey + walkBob - r - 4, RGB(20,20,20), RGB(0,0,0), 0);
        HBRUSH hpB = CreateSolidBrush(RGB(239, 68, 68));
        RECT hpR = { ex - barW/2, ey + walkBob - r - 8, ex - barW/2 + (int)(barW * hpRatio), ey + walkBob - r - 4 };
        FillRect(memDC, &hpR, hpB); DeleteObject(hpB);
    }

    if (g_hero.respawnTimer <= 0) {
        int heroBob = (int)(custom_sinf(g_globalFrame * 0.18f) * 1.5f);
        if (g_hero.shieldActive > 0) {
            HPEN shP = CreatePen(PS_SOLID, 2, RGB(96, 165, 250));
            HBRUSH oB = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));
            HPEN oP = (HPEN)SelectObject(memDC, shP);
            Ellipse(memDC, (int)g_hero.x - 26, (int)g_hero.y + heroBob - 26, (int)g_hero.x + 26, (int)g_hero.y + heroBob + 26);
            SelectObject(memDC, oB); SelectObject(memDC, oP); DeleteObject(shP);
        }
        // Cape
        DrawRoundedRect(memDC, (int)g_hero.x - 14, (int)g_hero.y + heroBob - 4, (int)g_hero.x - 7, (int)g_hero.y + heroBob + 18, RGB(30, 64, 175), RGB(59, 130, 246), 2);
        // Golden Cuirass
        DrawRoundedRect(memDC, (int)g_hero.x - 10, (int)g_hero.y + heroBob - 10, (int)g_hero.x + 10, (int)g_hero.y + heroBob + 10, TEXT_GOLD, RGB(255,255,255), 3);
        // Visored helm
        DrawRoundedRect(memDC, (int)g_hero.x - 7, (int)g_hero.y + heroBob - 17, (int)g_hero.x + 7, (int)g_hero.y + heroBob - 9, RGB(226, 232, 240), RGB(100, 116, 139), 2);
        // Red helm plume
        HPEN plmP = CreatePen(PS_SOLID, 3, RGB(220, 38, 38));
        HPEN oPlm = (HPEN)SelectObject(memDC, plmP);
        MoveToEx(memDC, (int)g_hero.x, (int)g_hero.y + heroBob - 17, NULL); LineTo(memDC, (int)g_hero.x - 6, (int)g_hero.y + heroBob - 22);
        SelectObject(memDC, oPlm); DeleteObject(plmP);
        // Heater shield with cross
        HBRUSH shB = CreateSolidBrush(RGB(30, 64, 175)); HPEN shPen = CreatePen(PS_SOLID, 1, TEXT_GOLD);
        HBRUSH oShB = (HBRUSH)SelectObject(memDC, shB); HPEN oShP = (HPEN)SelectObject(memDC, shPen);
        Rectangle(memDC, (int)g_hero.x - 12, (int)g_hero.y + heroBob - 4, (int)g_hero.x - 4, (int)g_hero.y + heroBob + 8);
        SelectObject(memDC, oShB); SelectObject(memDC, oShP); DeleteObject(shB); DeleteObject(shPen);
        // Broadsword
        HPEN swP = CreatePen(PS_SOLID, 2, RGB(226, 232, 240));
        HPEN oSwP = (HPEN)SelectObject(memDC, swP);
        MoveToEx(memDC, (int)g_hero.x + 8, (int)g_hero.y + heroBob + 4, NULL); LineTo(memDC, (int)g_hero.x + 16, (int)g_hero.y + heroBob - 12);
        SelectObject(memDC, oSwP); DeleteObject(swP);
    }

    for (int i = 0; i < g_slotCount; i++) {
        if (g_slots[i].occupied && g_slots[i].towerType == TOWER_SOLAR_BEAM && g_slots[i].beamTargetId != -1) {
            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (g_enemies[e].active && g_enemies[e].id == g_slots[i].beamTargetId) {
                    HPEN beamPen = CreatePen(PS_SOLID, 3, RGB(254, 240, 138));
                    HPEN oldP = (HPEN)SelectObject(memDC, beamPen);
                    MoveToEx(memDC, g_slots[i].x, g_slots[i].y, NULL);
                    LineTo(memDC, (int)g_enemies[e].x, (int)g_enemies[e].y);
                    SelectObject(memDC, oldP);
                    DeleteObject(beamPen);
                    break;
                }
            }
        }
    }

    for (int p = 0; p < MAX_PROJECTILES; p++) {
        if (!g_projectiles[p].active) continue;
        int px = (int)g_projectiles[p].x;
        int py = (int)g_projectiles[p].y;
        COLORREF pColor = TEXT_GOLD;
        if (g_projectiles[p].type == TOWER_MAGE) pColor = RGB(216, 180, 254);
        else if (g_projectiles[p].type == TOWER_CANNON) pColor = RGB(30, 41, 59);
        else if (g_projectiles[p].type == TOWER_POISON) pColor = RGB(34, 197, 94);
        else if (g_projectiles[p].type == TOWER_BALLISTA) pColor = RGB(251, 191, 36);
        else if (g_projectiles[p].type == TOWER_INFERNO) pColor = RGB(249, 115, 22);
        else if (g_projectiles[p].type == TOWER_VENOMSPITE) pColor = RGB(34, 197, 94);

        HBRUSH prjB = CreateSolidBrush(pColor);
        HPEN prjP = CreatePen(PS_SOLID, 1, RGB(255,255,255));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, prjB);
        HPEN oP = (HPEN)SelectObject(memDC, prjP);
        int sz = (g_projectiles[p].type == TOWER_CANNON || g_projectiles[p].type == TOWER_BALLISTA || g_projectiles[p].type == TOWER_INFERNO) ? 5 : 3;
        Ellipse(memDC, px - sz, py - sz, px + sz, py + sz);
        SelectObject(memDC, oB); SelectObject(memDC, oP);
        DeleteObject(prjB); DeleteObject(prjP);
    }

    // Shockwaves
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (!g_shockwaves[i].active) continue;
        int sx = (int)g_shockwaves[i].x;
        int sy = (int)g_shockwaves[i].y;
        int sr = (int)g_shockwaves[i].r;
        HPEN swPen = CreatePen(PS_SOLID, g_shockwaves[i].width, g_shockwaves[i].color);
        HBRUSH oB = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));
        HPEN oP = (HPEN)SelectObject(memDC, swPen);
        Ellipse(memDC, sx - sr, sy - sr, sx + sr, sy + sr);
        SelectObject(memDC, oB); SelectObject(memDC, oP);
        DeleteObject(swPen);
    }

    // Kinematic Multi-Layered Particles
    for (int p = 0; p < MAX_PARTICLES; p++) {
        if (!g_particles[p].active) continue;
        int px = (int)g_particles[p].x;
        int py = (int)g_particles[p].y;
        int sz = (int)g_particles[p].size;
        if (sz < 1) sz = 1;

        if (g_particles[p].type == 1) { // Needle Spark
            HPEN spkPen = CreatePen(PS_SOLID, sz, g_particles[p].color);
            HPEN oP = (HPEN)SelectObject(memDC, spkPen);
            MoveToEx(memDC, px, py, NULL);
            LineTo(memDC, px - (int)(g_particles[p].vx * 2.0f), py - (int)(g_particles[p].vy * 2.0f));
            SelectObject(memDC, oP); DeleteObject(spkPen);
        } else if (g_particles[p].type == 2) { // Smoke
            HBRUSH smkB = CreateSolidBrush(g_particles[p].color);
            HPEN smkP = CreatePen(PS_NULL, 0, 0);
            HBRUSH oB = (HBRUSH)SelectObject(memDC, smkB); HPEN oP = (HPEN)SelectObject(memDC, smkP);
            Ellipse(memDC, px - sz, py - sz, px + sz, py + sz);
            SelectObject(memDC, oB); SelectObject(memDC, oP);
            DeleteObject(smkB); DeleteObject(smkP);
        } else if (g_particles[p].type == 3) { // Shard
            HBRUSH shdB = CreateSolidBrush(g_particles[p].color);
            HPEN shdP = CreatePen(PS_SOLID, 1, RGB(0,0,0));
            HBRUSH oB = (HBRUSH)SelectObject(memDC, shdB); HPEN oP = (HPEN)SelectObject(memDC, shdP);
            Rectangle(memDC, px - sz/2, py - sz/2, px + sz/2, py + sz/2);
            SelectObject(memDC, oB); SelectObject(memDC, oP);
            DeleteObject(shdB); DeleteObject(shdP);
        } else { // Star / Default
            HBRUSH stB = CreateSolidBrush(g_particles[p].color);
            HPEN stP = CreatePen(PS_NULL, 0, 0);
            HBRUSH oB = (HBRUSH)SelectObject(memDC, stB); HPEN oP = (HPEN)SelectObject(memDC, stP);
            Rectangle(memDC, px - sz, py - sz/3, px + sz, py + sz/3);
            Rectangle(memDC, px - sz/3, py - sz, px + sz/3, py + sz);
            SelectObject(memDC, oB); SelectObject(memDC, oP);
            DeleteObject(stB); DeleteObject(stP);
        }
    }

    // Corner Filigree L-Brackets on Battlefield
    DrawFiligreeLBracket(memDC, bfX + 4, bfY + 4, 18, 0);
    DrawFiligreeLBracket(memDC, bfX + bfW - 4, bfY + 4, 18, 1);
    DrawFiligreeLBracket(memDC, bfX + bfW - 4, bfY + bfH - 4, 18, 2);
    DrawFiligreeLBracket(memDC, bfX + 4, bfY + bfH - 4, 18, 3);


    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        if (g_floatingTexts[i].active) {
            SetTextColor(memDC, g_floatingTexts[i].color);
            TextOutA(memDC, (int)g_floatingTexts[i].x, (int)g_floatingTexts[i].y, g_floatingTexts[i].text, lstrlenA(g_floatingTexts[i].text));
        }
    }

    int sbX = w - 200, sbY = 70, sbW = 190, sbH = h - 80;
    DrawRoundedRect(memDC, sbX, sbY, sbX + sbW, sbY + sbH, CARD_BG, BORDER_COLOR, 8);

    DrawRoundedRect(memDC, sbX + 5, sbY + 5, sbX + 30, sbY + 25, RGB(44, 50, 62), BORDER_COLOR, 4);
    DrawRoundedRect(memDC, sbX + sbW - 35, sbY + 5, sbX + sbW - 10, sbY + 25, RGB(44, 50, 62), BORDER_COLOR, 4);
    SetTextColor(memDC, TEXT_WHITE);
    TextOutA(memDC, sbX + 12, sbY + 8, "<", 1);
    TextOutA(memDC, sbX + sbW - 25, sbY + 8, ">", 1);
    SetTextColor(memDC, TEXT_GOLD);
    char mBuf[32]; wsprintfA(mBuf, "%s", g_maps[g_currentMap].name);
    TextOutA(memDC, sbX + 35 + (sbW - 70 - lstrlenA(mBuf)*6)/2, sbY + 8, mBuf, lstrlenA(mBuf));

    sbY += 28;

    DrawRoundedRect(memDC, sbX + 5, sbY + 3, sbX + 45, sbY + 23, g_gameMode == 0 ? TEXT_GOLD : RGB(44, 50, 62), BORDER_COLOR, 4);
    DrawRoundedRect(memDC, sbX + 50, sbY + 3, sbX + 90, sbY + 23, g_gameMode == 1 ? TEXT_GOLD : RGB(44, 50, 62), BORDER_COLOR, 4);
    DrawRoundedRect(memDC, sbX + 95, sbY + 3, sbX + 135, sbY + 23, g_gameMode == 2 ? TEXT_GOLD : RGB(44, 50, 62), BORDER_COLOR, 4);
    DrawRoundedRect(memDC, sbX + 140, sbY + 3, sbX + 185, sbY + 23, g_mutators ? RGB(168, 85, 247) : RGB(44, 50, 62), BORDER_COLOR, 4);
    SetTextColor(memDC, g_gameMode == 0 ? RGB(0,0,0) : TEXT_WHITE); TextOutA(memDC, sbX + 10, sbY + 6, "Camp", 4);
    SetTextColor(memDC, g_gameMode == 1 ? RGB(0,0,0) : TEXT_WHITE); TextOutA(memDC, sbX + 55, sbY + 6, "Endl", 4);
    SetTextColor(memDC, g_gameMode == 2 ? RGB(0,0,0) : TEXT_WHITE); TextOutA(memDC, sbX + 102, sbY + 6, "Boss", 4);
    SetTextColor(memDC, TEXT_WHITE); TextOutA(memDC, sbX + 142, sbY + 6, "Muts[M]", 7);

    sbY += 28;

    const char* tNames[] = {"Archer (50g)", "Mage (100g)", "Cannon (150g)", "Frost (120g)", "Tesla (180g)", "Ballista (160g)", "Poison (140g)"};
    for (int i = 0; i < 7; i++) {
        COLORREF bg = (g_selectedTowerTypeToBuild == i+1) ? RGB(60, 70, 85) : RGB(30, 41, 59);
        COLORREF bd = (g_selectedTowerTypeToBuild == i+1) ? TEXT_GOLD : BORDER_COLOR;
        DrawRoundedRect(memDC, sbX + 5, sbY + i*21, sbX + sbW - 5, sbY + (i+1)*21 - 2, bg, bd, 3);
        SetTextColor(memDC, (g_selectedTowerTypeToBuild == i+1) ? TEXT_GOLD : TEXT_WHITE);
        TextOutA(memDC, sbX + 10, sbY + i*21 + 2, tNames[i], lstrlenA(tNames[i]));
    }

    sbY += 7 * 21 + 4;

    const char* trapNames[] = {"Spike (30g)", "Oil (40g)", "Barricade (50g)", "Dynamite (60g)"};
    for (int i = 0; i < 4; i++) {
        COLORREF bg = (g_selectedTowerTypeToBuild == TRAP_SPIKE + i) ? RGB(60, 70, 85) : RGB(30, 41, 59);
        COLORREF bd = (g_selectedTowerTypeToBuild == TRAP_SPIKE + i) ? TEXT_GOLD : BORDER_COLOR;
        DrawRoundedRect(memDC, sbX + 5, sbY + i*20, sbX + sbW - 5, sbY + (i+1)*20 - 2, bg, bd, 3);
        SetTextColor(memDC, (g_selectedTowerTypeToBuild == TRAP_SPIKE + i) ? TEXT_GOLD : TEXT_WHITE);
        TextOutA(memDC, sbX + 10, sbY + i*20 + 2, trapNames[i], lstrlenA(trapNames[i]));
    }

    sbY += 4 * 20 + 4;

    DrawRoundedRect(memDC, sbX + 5, sbY, sbX + 58, sbY + 22, (g_trebuchetCd > 0) ? RGB(60,40,20) : RGB(217, 119, 6), BORDER_COLOR, 3);
    DrawRoundedRect(memDC, sbX + 62, sbY, sbX + 122, sbY + 22, RGB(220,38,38), BORDER_COLOR, 3);
    DrawRoundedRect(memDC, sbX + 126, sbY, sbX + 185, sbY + 22, RGB(37,99,235), BORDER_COLOR, 3);
    SetTextColor(memDC, TEXT_WHITE);
    TextOutA(memDC, sbX+8, sbY+4, "Siege(5)", 8);
    TextOutA(memDC, sbX+66, sbY+4, "Fire(F)", 7);
    TextOutA(memDC, sbX+130, sbY+4, "Bliz(B)", 7);

    sbY += 26;

    COLORREF btnBg = g_waveActive ? RGB(60, 70, 85) : RGB(16, 185, 129);
    DrawRoundedRect(memDC, sbX + 5, sbY, sbX + sbW - 5, sbY + 28, btnBg, BORDER_COLOR, 5);
    SetTextColor(memDC, TEXT_WHITE);
    char buf2[32];
    if (g_waveActive) wsprintfA(buf2, "IN PROGRESS");
    else wsprintfA(buf2, "WAVE %d [Space]", g_wave);
    TextOutA(memDC, sbX + (g_waveActive ? 45 : 20), sbY + 6, buf2, (int)lstrlenA(buf2));

    sbY += 32;

    DrawRoundedRect(memDC, sbX + 5, sbY, sbX + sbW/2 - 3, sbY + 22, RGB(79, 70, 229), BORDER_COLOR, 4);
    DrawRoundedRect(memDC, sbX + sbW/2 + 3, sbY, sbX + sbW - 5, sbY + 22, RGB(225, 29, 72), BORDER_COLOR, 4);
    TextOutA(memDC, sbX + 10, sbY + 3, "ACADEMY [A]", 11);
    TextOutA(memDC, sbX + sbW/2 + 10, sbY + 3, "RESET [R]", 9);

    sbY += 26;

    SetTextColor(memDC, TEXT_GOLD);
    TextOutA(memDC, sbX + 10, sbY, "HERO & SKILLS [1-5]", 19);
    sbY += 16;

    int btnW = 40;
    DrawRoundedRect(memDC, sbX + 5, sbY, sbX + 5 + btnW, sbY + 22, g_hero.healCd > 0 ? RGB(100,80,20) : TEXT_GOLD, BORDER_COLOR, 3);
    DrawRoundedRect(memDC, sbX + 50, sbY, sbX + 50 + btnW, sbY + 22, g_hero.shieldCd > 0 ? RGB(30,60,100) : RGB(59, 130, 246), BORDER_COLOR, 3);
    DrawRoundedRect(memDC, sbX + 95, sbY, sbX + 95 + btnW, sbY + 22, g_hero.meteorCd > 0 ? RGB(100,20,20) : TEXT_RED, BORDER_COLOR, 3);
    DrawRoundedRect(memDC, sbX + 140, sbY, sbX + 140 + btnW, sbY + 22, g_hero.summonCd > 0 ? RGB(60,60,60) : RGB(34, 197, 94), BORDER_COLOR, 3);
    
    SetTextColor(memDC, RGB(0,0,0)); TextOutA(memDC, sbX + 10, sbY + 3, "H(1)", 4);
    SetTextColor(memDC, TEXT_WHITE); TextOutA(memDC, sbX + 55, sbY + 3, "S(2)", 4);
    TextOutA(memDC, sbX + 100, sbY + 3, "M(3)", 4);
    SetTextColor(memDC, RGB(0,0,0)); TextOutA(memDC, sbX + 145, sbY + 3, "R(4)", 4);

    if (g_showAcademy) {
        int mx = w/2 - 240, my = h/2 - 220;
        DrawRoundedRect(memDC, mx, my, mx + 480, my + 440, CARD_BG, TEXT_GOLD, 12);
        SetTextColor(memDC, TEXT_GOLD);
        TextOutA(memDC, mx + 150, my + 12, "RESEARCH ACADEMY", 16);
        
        const char* tNames[] = {
            "Starting Gold (+50)", "Wall Durability (+10 HP)", "Hero Cooldowns (-10%)",
            "Tower Damage (+10%)", "Militia Training (+50%)", "Siege Weaponry (Auto-Ballista & Mortar)",
            "Fusion Mastery (+25% Dmg/AoE)", "Fortified Traps (+1 Charge, +75 HP)"
        };
        int* tLevels[] = {&g_techStartingGold, &g_techWallHp, &g_techHeroCd, &g_techTowerDmg, &g_techMilitia, &g_techSiegeEng, &g_techFusion, &g_techFortTraps};
        int tBaseCosts[] = {100, 150, 200, 250, 200, 300, 350, 180};
        
        for (int i=0; i<8; i++) {
            DrawRoundedRect(memDC, mx + 15, my + 40 + i*44, mx + 465, my + 78 + i*44, RGB(20,20,20), BORDER_COLOR, 4);
            SetTextColor(memDC, TEXT_WHITE);
            TextOutA(memDC, mx + 25, my + 44 + i*44, tNames[i], lstrlenA(tNames[i]));
            char lvlBuf[32]; wsprintfA(lvlBuf, "Level %d/5", *tLevels[i]);
            SetTextColor(memDC, TEXT_MUTED);
            TextOutA(memDC, mx + 25, my + 59 + i*44, lvlBuf, lstrlenA(lvlBuf));
            
            int cost = tBaseCosts[i] * (*tLevels[i] + 1);
            COLORREF btnC = (g_gold >= cost && *tLevels[i] < 5) ? RGB(34, 197, 94) : RGB(44, 50, 62);
            DrawRoundedRect(memDC, mx + 380, my + 45 + i*44, mx + 455, my + 73 + i*44, btnC, BORDER_COLOR, 4);
            SetTextColor(memDC, (g_gold >= cost && *tLevels[i] < 5) ? RGB(0,0,0) : TEXT_MUTED);
            if (*tLevels[i] < 5) {
                char cBuf[16]; wsprintfA(cBuf, "%dg", cost);
                TextOutA(memDC, mx + 395, my + 51 + i*44, cBuf, lstrlenA(cBuf));
            } else {
                TextOutA(memDC, mx + 400, my + 51 + i*44, "MAX", 3);
            }
        }
        
        DrawRoundedRect(memDC, mx + 380, my + 398, mx + 455, my + 428, RGB(220,38,38), BORDER_COLOR, 4);
        SetTextColor(memDC, TEXT_WHITE);
        TextOutA(memDC, mx + 395, my + 405, "CLOSE", 5);
    }

    if (g_showMutators) {
        int mx = w/2 - 220, my = h/2 - 160;
        DrawRoundedRect(memDC, mx, my, mx + 440, my + 320, CARD_BG, RGB(168, 85, 247), 12);
        SetTextColor(memDC, RGB(168, 85, 247));
        TextOutA(memDC, mx + 130, my + 15, "CHALLENGE MUTATORS", 18);

        const char* mutNames[] = {
            "Bloodlust (+40% Speed, +50% Gold)",
            "Titan Brood (+100% HP, 2x Boss, 2x Score)",
            "Arcane Eclipse (Hero CD +40%, Towers +30% Spd)",
            "Meteor Tempest (Sky Meteor Cataclysms)",
            "Phase Shift (25% Dodge Chance, +75% Gold)"
        };
        int mBits[] = {MUTATOR_BLOODLUST, MUTATOR_TITAN, MUTATOR_ECLIPSE, MUTATOR_METEOR, MUTATOR_PHASE_SHIFT};

        for (int i=0; i<5; i++) {
            BOOL active = (g_mutators & mBits[i]) != 0;
            DrawRoundedRect(memDC, mx + 20, my + 45 + i*48, mx + 420, my + 85 + i*48, RGB(20,20,20), active ? RGB(168, 85, 247) : BORDER_COLOR, 4);
            SetTextColor(memDC, active ? RGB(254, 240, 138) : TEXT_WHITE);
            TextOutA(memDC, mx + 30, my + 57 + i*48, mutNames[i], lstrlenA(mutNames[i]));

            DrawRoundedRect(memDC, mx + 355, my + 52 + i*48, mx + 410, my + 78 + i*48, active ? RGB(168, 85, 247) : RGB(44, 50, 62), BORDER_COLOR, 4);
            SetTextColor(memDC, active ? RGB(255,255,255) : TEXT_MUTED);
            TextOutA(memDC, mx + 365, my + 56 + i*48, active ? "ON" : "OFF", active ? 2 : 3);
        }

        DrawRoundedRect(memDC, mx + 340, my + 285, mx + 410, my + 312, RGB(220,38,38), BORDER_COLOR, 4);
        SetTextColor(memDC, TEXT_WHITE);
        TextOutA(memDC, mx + 355, my + 291, "DONE", 4);
    }

    if (g_showHelp) {
        int hW = 680, hH = 550;
        int hX = (w - hW) / 2, hY = (h - hH) / 2;
        DrawRoundedRect(memDC, hX, hY, hX + hW, hY + hH, CARD_BG, TEXT_GOLD, 12);
        
        SetTextColor(memDC, TEXT_GOLD);
        TextOutA(memDC, hX + 180, hY + 15, "COMMANDER'S FIELD GUIDE & SHORTCUTS", 35);
        
        int cy = hY + 45;
        SetTextColor(memDC, RGB(34, 197, 94)); TextOutA(memDC, hX + 20, cy, "ELEMENTAL TOWER FUSIONS (MAX LEVEL 3 -> FUSION)", 47); cy += 18;
        SetTextColor(memDC, TEXT_WHITE);
        TextOutA(memDC, hX + 20, cy, "- Inferno Mortar: Blazing magma mortars that ignite lingering lava puddles.", 75); cy += 16;
        TextOutA(memDC, hX + 20, cy, "- Superconductor: Cryo-electric storms arcing to 5 targets with 65% freeze slow.", 80); cy += 16;
        TextOutA(memDC, hX + 20, cy, "- Venomspite Piercer: Heavy plague javelins piercing entire enemy lines with acid DoT.", 86); cy += 16;
        TextOutA(memDC, hX + 20, cy, "- Solar Prism Beam: Continuous focused photon laser ramping damage vs bosses.", 77); cy += 22;
        
        SetTextColor(memDC, RGB(34, 197, 94)); TextOutA(memDC, hX + 20, cy, "SIEGE DEFENSES & SPELLS", 23); cy += 18;
        SetTextColor(memDC, TEXT_WHITE);
        TextOutA(memDC, hX + 20, cy, "- Castle Trebuchet (5): Calls down massive siege boulders at target location.", 77); cy += 16;
        TextOutA(memDC, hX + 20, cy, "- Firestorm Spell (F / 100g) & Blizzard Freeze Spell (B / 80g).", 63); cy += 16;
        TextOutA(memDC, hX + 20, cy, "- Automated Castle Wall Ballistas: Researched in Academy for auto gate defense.", 78); cy += 22;
        
        SetTextColor(memDC, RGB(34, 197, 94)); TextOutA(memDC, hX + 20, cy, "KEYBOARD SHORTCUTS & CONTROLS", 29); cy += 18;
        SetTextColor(memDC, TEXT_WHITE);
        TextOutA(memDC, hX + 20, cy, "- [Space]: Start Wave | [Esc]: Close Modals | [F1] / [H]: Field Guide | [R]: Reset Battle", 88); cy += 16;
        TextOutA(memDC, hX + 20, cy, "- [1-4]: Hero Skills (Heal, Shield Wall, Meteor Strike, Militia Reinforcements)", 79); cy += 16;
        TextOutA(memDC, hX + 20, cy, "- [5]: Siege Trebuchet | [F]: Firestorm | [B]: Blizzard | [A]: Academy | [M]: Mutators", 86); cy += 25;
        
        DrawRoundedRect(memDC, hX + 260, hY + hH - 45, hX + 420, hY + hH - 15, RGB(16, 185, 129), BORDER_COLOR, 6);
        SetTextColor(memDC, RGB(0,0,0));
        TextOutA(memDC, hX + 295, hY + hH - 37, "UNDERSTOOD", 10);
    }

    BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        InitGameState();
        SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL, NULL);
        break;

    case WM_TIMER:
        UpdateGameLogic();
        InvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_KEYDOWN: {
        if (wParam == VK_ESCAPE) {
            if (g_showHelp || g_showMutators || g_showAcademy) {
                g_showHelp = FALSE;
                g_showMutators = FALSE;
                g_showAcademy = FALSE;
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
        }
        if (wParam == 'h' || wParam == 'H' || wParam == VK_F1) {
            g_showHelp = !g_showHelp;
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        if (wParam == 'm' || wParam == 'M') {
            g_showMutators = !g_showMutators;
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        if (wParam == 'a' || wParam == 'A') {
            g_showAcademy = !g_showAcademy;
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        if (wParam == 'r' || wParam == 'R') {
            InitGameState();
            Beep(300, 60);
            ShowNativeToast("Battlefield Reset", TEXT_GOLD, 90);
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        if (wParam == 'f' || wParam == 'F') {
            if (g_gold >= 100) {
                g_gold -= 100;
                AddFloatingText(400, 300, "FIRESTORM!", TEXT_RED);
                Beep(100, 300);
                for (int e = 0; e < MAX_ENEMIES; e++) {
                    if (g_enemies[e].active) {
                        g_enemies[e].hp -= 150;
                        if (g_enemies[e].hp <= 0) {
                            g_enemies[e].active = FALSE;
                            g_gold += 15;
                        }
                    }
                }
                ShowNativeToast("Firestorm Cast! (-100g)", TEXT_RED, 90);
            } else {
                ShowNativeToast("Need 100g for Firestorm!", RGB(239, 68, 68), 90);
                Beep(200, 80);
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        if (wParam == 'b' || wParam == 'B') {
            if (g_gold >= 80) {
                g_gold -= 80;
                g_blizzTimer = 300;
                AddFloatingText(400, 300, "BLIZZARD!", RGB(59, 130, 246));
                Beep(600, 300);
                ShowNativeToast("Blizzard Active! (-80g)", RGB(59, 130, 246), 90);
            } else {
                ShowNativeToast("Need 80g for Blizzard!", RGB(239, 68, 68), 90);
                Beep(200, 80);
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        if (wParam == VK_SPACE) {
            if (!g_waveActive && !g_gameOver) {
                g_waveActive = TRUE;
                if (g_gameMode == 2) {
                    g_spawnQueueCount = g_wave * 2;
                    if (g_spawnQueueCount > MAX_SPAWN_QUEUE) g_spawnQueueCount = MAX_SPAWN_QUEUE;
                    g_spawnQueueHead = 0;
                    for (int i = 0; i < g_spawnQueueCount; i++) g_spawnQueue[i] = (i % 2 == 0) ? ENEMY_OGRE : ENEMY_WYVERN;
                } else {
                    g_spawnQueueCount = 6 + g_wave * 3;
                    if (g_spawnQueueCount > MAX_SPAWN_QUEUE) g_spawnQueueCount = MAX_SPAWN_QUEUE;
                    g_spawnQueueHead = 0;
                    for (int i = 0; i < g_spawnQueueCount; i++) {
                        if (g_wave % 5 == 0 && i == 0) g_spawnQueue[i] = ENEMY_OGRE;
                        else if (g_wave >= 6 && i == 1 && g_wave % 3 == 0) g_spawnQueue[i] = ENEMY_WYVERN;
                        else if (g_wave >= 8 && i == 2 && g_wave % 4 == 0) g_spawnQueue[i] = ENEMY_GOLEM;
                        else {
                            int r = rand() % 100;
                            if (g_wave >= 5 && r < 15) g_spawnQueue[i] = ENEMY_NECROMANCER;
                            else if (g_wave >= 4 && r < 35) g_spawnQueue[i] = ENEMY_GARGOYLE;
                            else if (g_wave >= 3 && r < 55) g_spawnQueue[i] = ENEMY_ORC;
                            else if (g_wave >= 2 && r < 75) g_spawnQueue[i] = ENEMY_HOUND;
                            else g_spawnQueue[i] = ENEMY_GOBLIN;
                        }
                    }
                }
                g_spawnTimer = 0;
                Beep(600, 40);
                ShowNativeToast("Wave Started!", RGB(16, 185, 129), 90);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        if (wParam == '1' && g_hero.healCd <= 0 && g_hero.respawnTimer <= 0) {
            g_hero.healCd = g_hero.maxHealCd;
            g_hero.hp += 50.0f; if(g_hero.hp > g_hero.maxHp) g_hero.hp = g_hero.maxHp;
            g_baseHp += 5; if(g_baseHp > g_maxBaseHp) g_baseHp = g_maxBaseHp;
            AddFloatingText(g_hero.x, g_hero.y - 20, "HEAL!", TEXT_GOLD);
            Beep(800, 50);
        } else if (wParam == '2' && g_hero.shieldCd <= 0 && g_hero.respawnTimer <= 0) {
            g_hero.shieldCd = g_hero.maxShieldCd;
            g_hero.shieldActive = 150;
            AddFloatingText(g_hero.x, g_hero.y - 20, "SHIELD WALL!", RGB(59, 130, 246));
            Beep(300, 60);
        } else if (wParam == '3' && g_hero.meteorCd <= 0 && g_hero.respawnTimer <= 0) {
            g_hero.meteorCd = g_hero.maxMeteorCd;
            AddFloatingText(g_hero.x, g_hero.y - 40, "METEOR STRIKE!", TEXT_RED);
            Beep(150, 100);
            SpawnExplosion(g_hero.x, g_hero.y, RGB(239, 68, 68));
            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (g_enemies[e].active) {
                    float dx = g_enemies[e].x - g_hero.x;
                    float dy = g_enemies[e].y - g_hero.y;
                    if (custom_sqrtf(dx*dx + dy*dy) <= 150.0f) {
                        g_enemies[e].hp -= 100;
                        if (g_enemies[e].hp <= 0) {
                            g_enemies[e].active = FALSE;
                            g_gold += 15;
                        }
                    }
                }
            }
        } else if (wParam == '4' && g_hero.summonCd <= 0 && g_hero.respawnTimer <= 0) {
            g_hero.summonCd = g_hero.maxSummonCd;
            SummonMilitia();
            AddFloatingText(g_hero.x, g_hero.y - 30, "MILITIA REINFORCEMENTS!", RGB(59, 130, 246));
            Beep(600, 60); Beep(900, 80);
        } else if (wParam == '5') {
            TriggerTrebuchetStrike(g_hero.targetX, g_hero.targetY);
        }
        break;
    }

    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        int w = clientRect.right;
        int h = clientRect.bottom;
        
        if (g_showHelp) {
            int hW = 680, hH = 550;
            int hX = (w - hW) / 2, hY = (h - hH) / 2;
            if (x >= hX + 260 && x <= hX + 420 && y >= hY + hH - 45 && y <= hY + hH - 15) {
                g_showHelp = FALSE;
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (x < hX || x > hX + hW || y < hY || y > hY + hH) {
                g_showHelp = FALSE;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }

        if (g_showMutators) {
            int mx = w/2 - 220, my = h/2 - 160;
            if (x >= mx + 340 && x <= mx + 410 && y >= my + 285 && y <= my + 312) {
                g_showMutators = FALSE;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (x < mx || x > mx + 440 || y < my || y > my + 320) {
                g_showMutators = FALSE;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            int mBits[] = {MUTATOR_BLOODLUST, MUTATOR_TITAN, MUTATOR_ECLIPSE, MUTATOR_METEOR, MUTATOR_PHASE_SHIFT};
            for (int i=0; i<5; i++) {
                if (x >= mx + 20 && x <= mx + 420 && y >= my + 45 + i*48 && y <= my + 85 + i*48) {
                    g_mutators ^= mBits[i];
                    SaveGame();
                    Beep(500, 40);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            }
            return 0;
        }
        
        if (g_showAcademy) {
            int mx = w/2 - 240, my = h/2 - 220;
            if (x >= mx + 380 && x <= mx + 455 && y >= my + 398 && y <= my + 428) {
                g_showAcademy = FALSE;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (x < mx || x > mx + 480 || y < my || y > my + 440) {
                g_showAcademy = FALSE;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            int tBaseCosts[] = {100, 150, 200, 250, 200, 300, 350, 180};
            int* tLevels[] = {&g_techStartingGold, &g_techWallHp, &g_techHeroCd, &g_techTowerDmg, &g_techMilitia, &g_techSiegeEng, &g_techFusion, &g_techFortTraps};
            for (int i=0; i<8; i++) {
                if (x >= mx + 380 && x <= mx + 455 && y >= my + 45 + i*44 && y <= my + 73 + i*44) {
                    int cost = tBaseCosts[i] * (*tLevels[i] + 1);
                    if (*tLevels[i] < 5 && g_gold >= cost) {
                        g_gold -= cost;
                        (*tLevels[i])++;
                        SaveGame();
                        Beep(600, 50); Beep(900, 50);
                    } else {
                        Beep(200, 50);
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            }
            return 0;
        }

        if (x >= w - 95 && x <= w - 15 && y >= 20 && y <= 45) {
            g_showHelp = TRUE;
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        int sbX = w - 200, sbY = 70, sbW = 190;

        if (x >= sbX + 5 && x <= sbX + 30 && y >= sbY + 5 && y <= sbY + 25) {
            if (!g_waveActive) {
                g_currentMap = (g_currentMap - 1 + MAX_MAPS) % MAX_MAPS;
                InitGameState();
                Beep(300, 40);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        if (x >= sbX + sbW - 35 && x <= sbX + sbW - 10 && y >= sbY + 5 && y <= sbY + 25) {
            if (!g_waveActive) {
                g_currentMap = (g_currentMap + 1) % MAX_MAPS;
                InitGameState();
                Beep(300, 40);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        
        sbY += 28;
        
        if (!g_waveActive) {
            if (x >= sbX + 5 && x <= sbX + 45 && y >= sbY + 3 && y <= sbY + 23) { g_gameMode = 0; InitGameState(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
            if (x >= sbX + 50 && x <= sbX + 90 && y >= sbY + 3 && y <= sbY + 23) { g_gameMode = 1; InitGameState(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
            if (x >= sbX + 95 && x <= sbX + 135 && y >= sbY + 3 && y <= sbY + 23) { g_gameMode = 2; InitGameState(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
            if (x >= sbX + 140 && x <= sbX + 185 && y >= sbY + 3 && y <= sbY + 23) { g_showMutators = TRUE; InvalidateRect(hwnd, NULL, FALSE); return 0; }
        }
        sbY += 28;

        for (int i=0; i<7; i++) {
            if (x >= sbX + 5 && x <= sbX + sbW - 5 && y >= sbY + i*21 && y <= sbY + (i+1)*21 - 2) {
                g_selectedTowerTypeToBuild = i + 1;
                Beep(400, 20);
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
        }
        sbY += 7 * 21 + 4;
        
        for (int i=0; i<4; i++) {
            if (x >= sbX + 5 && x <= sbX + sbW - 5 && y >= sbY + i*20 && y <= sbY + (i+1)*20 - 2) {
                g_selectedTowerTypeToBuild = TRAP_SPIKE + i;
                Beep(400, 20); InvalidateRect(hwnd, NULL, FALSE); return 0;
            }
        }
        sbY += 4 * 20 + 4;
        
        if (x >= sbX + 5 && x <= sbX + 58 && y >= sbY && y <= sbY + 22) {
            TriggerTrebuchetStrike(g_hero.targetX, g_hero.targetY);
            InvalidateRect(hwnd, NULL, FALSE); return 0;
        }
        if (x >= sbX + 62 && x <= sbX + 122 && y >= sbY && y <= sbY + 22) {
            if (g_gold >= 100) { g_gold -= 100; AddFloatingText(400, 300, "FIRESTORM!", TEXT_RED); Beep(100, 300);
                for(int e=0; e<MAX_ENEMIES; e++) if(g_enemies[e].active) { g_enemies[e].hp -= 150; if(g_enemies[e].hp<=0) { g_enemies[e].active=FALSE; g_gold+=15; } }
            }
            InvalidateRect(hwnd, NULL, FALSE); return 0;
        }
        if (x >= sbX + 126 && x <= sbX + 185 && y >= sbY && y <= sbY + 22) {
            if (g_gold >= 80) { g_gold -= 80; g_blizzTimer = 300; AddFloatingText(400, 300, "BLIZZARD!", RGB(59, 130, 246)); Beep(600, 300); }
            InvalidateRect(hwnd, NULL, FALSE); return 0;
        }

        sbY += 26;

        if (x >= sbX + 10 && x <= sbX + sbW - 10 && y >= sbY && y <= sbY + 28) {
            if (!g_waveActive && !g_gameOver) {
                g_waveActive = TRUE;
                if (g_gameMode == 2) {
                    g_spawnQueueCount = g_wave * 2;
                    if (g_spawnQueueCount > MAX_SPAWN_QUEUE) g_spawnQueueCount = MAX_SPAWN_QUEUE;
                    g_spawnQueueHead = 0;
                    for (int i = 0; i < g_spawnQueueCount; i++) {
                        g_spawnQueue[i] = (i % 3 == 0) ? ENEMY_OGRE : ((i % 3 == 1) ? ENEMY_WYVERN : ENEMY_GOLEM);
                    }
                } else {
                    g_spawnQueueCount = 6 + g_wave * 3;
                    if (g_spawnQueueCount > MAX_SPAWN_QUEUE) g_spawnQueueCount = MAX_SPAWN_QUEUE;
                    g_spawnQueueHead = 0;
                    for (int i = 0; i < g_spawnQueueCount; i++) {
                        if (g_wave % 5 == 0 && i == 0) {
                            g_spawnQueue[i] = (g_wave % 15 == 5) ? ENEMY_OGRE : ((g_wave % 15 == 10) ? ENEMY_WYVERN : ENEMY_GOLEM);
                        }
                        else if (g_wave >= 6 && i == 1 && g_wave % 3 == 0) g_spawnQueue[i] = ENEMY_WYVERN;
                        else if (g_wave >= 8 && i == 2 && g_wave % 4 == 0) g_spawnQueue[i] = ENEMY_GOLEM;
                        else {
                            int r = rand() % 100;
                            if (g_wave >= 5 && r < 15) g_spawnQueue[i] = ENEMY_NECROMANCER;
                            else if (g_wave >= 4 && r < 35) g_spawnQueue[i] = ENEMY_GARGOYLE;
                            else if (g_wave >= 3 && r < 55) g_spawnQueue[i] = ENEMY_ORC;
                            else if (g_wave >= 2 && r < 75) g_spawnQueue[i] = ENEMY_HOUND;
                            else g_spawnQueue[i] = ENEMY_GOBLIN;
                        }
                    }
                }
                g_spawnTimer = 0; Beep(600, 40);
            }
        }
        sbY += 32;

        if (x >= sbX + 10 && x <= sbX + sbW/2 - 5 && y >= sbY && y <= sbY + 22) {
            g_showAcademy = TRUE; InvalidateRect(hwnd, NULL, FALSE); return 0;
        }
        if (x >= sbX + sbW/2 + 5 && x <= sbX + sbW - 10 && y >= sbY && y <= sbY + 22) {
            InitGameState(); Beep(300, 60);
        }
        sbY += 26;

        if (x >= sbX + 5 && x <= sbX + 45 && y >= sbY + 16 && y <= sbY + 38) SendMessage(hwnd, WM_KEYDOWN, '1', 0);
        else if (x >= sbX + 50 && x <= sbX + 90 && y >= sbY + 16 && y <= sbY + 38) SendMessage(hwnd, WM_KEYDOWN, '2', 0);
        else if (x >= sbX + 95 && x <= sbX + 135 && y >= sbY + 16 && y <= sbY + 38) SendMessage(hwnd, WM_KEYDOWN, '3', 0);
        else if (x >= sbX + 140 && x <= sbX + 180 && y >= sbY + 16 && y <= sbY + 38) SendMessage(hwnd, WM_KEYDOWN, '4', 0);

        else if (!g_gameOver) {
            BOOL clickedSlot = FALSE;
            g_selectedSlot = -1;
            for (int i = 0; i < g_slotCount; i++) {
                int dx = x - g_slots[i].x;
                int dy = y - g_slots[i].y;
                if (dx * dx + dy * dy <= 22 * 22) {
                    g_selectedSlot = i;

                    if (!g_slots[i].occupied) {
                        int cost = 50, rng = 130, dmg = 12, cd = 18, splash = 0;
                        if (g_selectedTowerTypeToBuild == TOWER_MAGE) { cost = 100; rng = 110; dmg = 15; cd = 30; splash = 50; }
                        else if (g_selectedTowerTypeToBuild == TOWER_CANNON) { cost = 150; rng = 150; dmg = 40; cd = 60; }
                        else if (g_selectedTowerTypeToBuild == TOWER_FROST) { cost = 120; rng = 100; dmg = 0; cd = 15; }
                        else if (g_selectedTowerTypeToBuild == TOWER_TESLA) { cost = 180; rng = 135; dmg = 28; cd = 24; }
                        else if (g_selectedTowerTypeToBuild == TOWER_BALLISTA) { cost = 160; rng = 190; dmg = 75; cd = 45; }
                        else if (g_selectedTowerTypeToBuild == TOWER_POISON) { cost = 140; rng = 120; dmg = 12; cd = 28; splash = 45; }
                        
                        if (g_gold >= cost) {
                            g_gold -= cost;
                            g_slots[i].occupied = TRUE;
                            g_slots[i].towerType = g_selectedTowerTypeToBuild;
                            g_slots[i].level = 1;
                            float dmgMod = 1.0f + (g_techTowerDmg * 0.1f);
                            g_slots[i].range = rng;
                            g_slots[i].damage = (int)(dmg * dmgMod);
                            g_slots[i].maxCooldown = cd;
                            g_slots[i].cooldown = 0;
                            g_slots[i].splash = splash;
                            char buf[16]; wsprintfA(buf, "-%dg", cost);
                            AddFloatingText((float)g_slots[i].x, (float)(g_slots[i].y - 20), buf, RGB(239, 68, 68));
                            Beep(800, 40);
                        } else {
                            AddFloatingText((float)g_slots[i].x, (float)(g_slots[i].y - 20), "NEED MORE GOLD!", RGB(239, 68, 68));
                            Beep(200, 100);
                        }
                    } else {
                        if (g_slots[i].level < 3) {
                            int baseCost = 50;
                            if (g_slots[i].towerType == TOWER_MAGE) baseCost = 100;
                            else if (g_slots[i].towerType == TOWER_CANNON) baseCost = 150;
                            else if (g_slots[i].towerType == TOWER_FROST) baseCost = 120;
                            else if (g_slots[i].towerType == TOWER_TESLA) baseCost = 180;
                            else if (g_slots[i].towerType == TOWER_BALLISTA) baseCost = 160;
                            else if (g_slots[i].towerType == TOWER_POISON) baseCost = 140;
                            
                            int upCost = (int)(baseCost * (g_slots[i].level * 1.5f));
                            if (g_gold >= upCost) {
                                g_gold -= upCost;
                                g_slots[i].level++;
                                g_slots[i].damage = (int)(g_slots[i].damage * 1.4f);
                                g_slots[i].range = (int)(g_slots[i].range * 1.15f);
                                g_slots[i].maxCooldown = (int)(g_slots[i].maxCooldown * 0.85f);
                                if (g_slots[i].maxCooldown < 10) g_slots[i].maxCooldown = 10;
                                if (g_slots[i].splash > 0) g_slots[i].splash = (int)(g_slots[i].splash * 1.25f);
                                
                                char buf[32]; wsprintfA(buf, "LVL %d!", g_slots[i].level);
                                AddFloatingText((float)g_slots[i].x, (float)(g_slots[i].y - 20), buf, TEXT_GOLD);
                                Beep(800, 40);
                            } else {
                                char buf[32]; wsprintfA(buf, "NEED %dg!", upCost);
                                AddFloatingText((float)g_slots[i].x, (float)(g_slots[i].y - 20), buf, RGB(239, 68, 68));
                                Beep(200, 100);
                            }
                        } else if (g_slots[i].level == 3) {
                            int fusCost = 220;
                            if (g_gold >= fusCost) {
                                g_gold -= fusCost;
                                g_slots[i].level = 4;
                                float fusBonus = 1.0f + (g_techFusion * 0.25f);
                                
                                if (g_slots[i].towerType == TOWER_MAGE || g_slots[i].towerType == TOWER_CANNON) {
                                    g_slots[i].towerType = TOWER_INFERNO;
                                    g_slots[i].damage = (int)(90 * fusBonus);
                                    g_slots[i].splash = 70;
                                    g_slots[i].range = 160;
                                    g_slots[i].maxCooldown = 40;
                                    AddFloatingText((float)g_slots[i].x, (float)(g_slots[i].y - 20), "INFERNO FUSION!", RGB(249, 115, 22));
                                } else if (g_slots[i].towerType == TOWER_TESLA || g_slots[i].towerType == TOWER_FROST) {
                                    g_slots[i].towerType = TOWER_SUPERCONDUCTOR;
                                    g_slots[i].damage = (int)(65 * fusBonus);
                                    g_slots[i].range = 160;
                                    g_slots[i].maxCooldown = 22;
                                    AddFloatingText((float)g_slots[i].x, (float)(g_slots[i].y - 20), "SUPERCONDUCTOR!", RGB(14, 165, 233));
                                } else if (g_slots[i].towerType == TOWER_POISON || g_slots[i].towerType == TOWER_BALLISTA) {
                                    g_slots[i].towerType = TOWER_VENOMSPITE;
                                    g_slots[i].damage = (int)(110 * fusBonus);
                                    g_slots[i].range = 210;
                                    g_slots[i].maxCooldown = 32;
                                    AddFloatingText((float)g_slots[i].x, (float)(g_slots[i].y - 20), "VENOMSPITE FUSION!", RGB(34, 197, 94));
                                } else {
                                    g_slots[i].towerType = TOWER_SOLAR_BEAM;
                                    g_slots[i].damage = (int)(16 * fusBonus);
                                    g_slots[i].range = 180;
                                    g_slots[i].maxCooldown = 4;
                                    AddFloatingText((float)g_slots[i].x, (float)(g_slots[i].y - 20), "SOLAR BEAM FUSION!", RGB(254, 240, 138));
                                }
                                Beep(1000, 60); Beep(1300, 80);
                            } else {
                                char buf[32]; wsprintfA(buf, "FUSE: NEED %dg!", fusCost);
                                AddFloatingText((float)g_slots[i].x, (float)(g_slots[i].y - 20), buf, RGB(239, 68, 68));
                                Beep(200, 100);
                            }
                        } else {
                            AddFloatingText((float)g_slots[i].x, (float)(g_slots[i].y - 20), "FUSION MASTERED!", TEXT_GOLD);
                            Beep(500, 30);
                        }
                    }
                    clickedSlot = TRUE;
                    break;
                }
            }

            if (!clickedSlot && g_hero.respawnTimer <= 0) {
                if (g_selectedTowerTypeToBuild >= TRAP_SPIKE && g_selectedTowerTypeToBuild <= TRAP_DYNAMITE && x < sbX) {
                    int c = 30;
                    if (g_selectedTowerTypeToBuild == TRAP_OIL) c = 40;
                    else if (g_selectedTowerTypeToBuild == TRAP_BARRICADE) c = 50;
                    else if (g_selectedTowerTypeToBuild == TRAP_DYNAMITE) c = 60;

                    if (g_gold >= c) {
                        for (int tr = 0; tr < MAX_TRAPS; tr++) {
                            if (!g_traps[tr].active) {
                                g_gold -= c;
                                g_traps[tr].active = TRUE;
                                g_traps[tr].x = (float)x; g_traps[tr].y = (float)y;
                                g_traps[tr].type = g_selectedTowerTypeToBuild;
                                g_traps[tr].charges = (g_selectedTowerTypeToBuild == TRAP_SPIKE) ? (3 + g_techFortTraps) : -1;
                                g_traps[tr].hp = (g_selectedTowerTypeToBuild == TRAP_BARRICADE) ? (200.0f + g_techFortTraps * 75.0f) : 0;
                                g_traps[tr].radius = (g_selectedTowerTypeToBuild == TRAP_OIL || g_selectedTowerTypeToBuild == TRAP_DYNAMITE) ? 45 : 25;
                                Beep(300, 40);
                                break;
                            }
                        }
                    } else { Beep(200, 100); }
                } else {
                    g_hero.targetX = (float)x;
                    g_hero.targetY = (float)y;
                    Beep(400, 20);
                }
            }
        }
        InvalidateRect(hwnd, NULL, FALSE);
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        Render(hdc, hwnd);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_ERASEBKGND:
        return 1;

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
    SetProcessDPIAware();
    const char CLASS_NAME[] = "KFortressWindowClass";

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
        0, CLASS_NAME, "KFortress - Fantasy Tower Defense & Siege Defense [F1/H: Guide | Space: Wave | 1-5: Skills]",
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
