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

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
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
#define PATH_BORDER RGB(74, 85, 104)
#define CASTLE_COLOR RGB(71, 85, 105)
#define TOWER_SLOT_BG RGB(44, 50, 62)
#define TOWER_SLOT_HOVER RGB(60, 68, 82)
#define GOBLIN_GREEN RGB(22, 163, 74)

#define MAX_SLOTS 12
#define MAX_ENEMIES 64
#define MAX_PROJECTILES 64
#define MAX_FLOATING_TEXTS 32
#define MAX_TRAPS 32
#define MAX_WAYPOINTS 6
#define MAX_MAPS 10

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

typedef struct {
    int x, y;
    BOOL occupied;
    int towerType; // 0 = none, 1 = archer
    int level;
    int cooldown;
    int maxCooldown;
    int range;
    int damage;
    int splash;
} TowerSlot;

typedef struct {
    BOOL active;
    float x, y;
    int hp;
    int maxHp;
    float speed;
    int waypointIndex;
    int id;
    BOOL slowed;
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
    int type; // 1=Spike, 2=Oil, 3=Barricade
    int charges;
    float hp;
    int radius;
} Trap;


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

int g_techStartingGold = 0;
int g_techWallHp = 0;
int g_techHeroCd = 0;
int g_techTowerDmg = 0;
BOOL g_showAcademy = FALSE;
BOOL g_showHelp = FALSE;

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
    int shieldActive;
} Hero;

static Hero g_hero;

#define TOWER_ARCHER 1
#define TOWER_MAGE 2
#define TOWER_CANNON 3
#define TOWER_FROST 4
#define TRAP_SPIKE 5
#define TRAP_OIL 6
#define TRAP_BARRICADE 7

static int g_selectedTowerTypeToBuild = TOWER_ARCHER;

#define MAX_SPAWN_QUEUE 100
static int g_spawnQueue[MAX_SPAWN_QUEUE];
static int g_spawnQueueCount = 0;
static int g_spawnQueueHead = 0;
static int g_spawnTimer = 0;

#define ENEMY_GOBLIN 1
#define ENEMY_ORC 2
#define ENEMY_HOUND 3
#define ENEMY_GARGOYLE 4
#define ENEMY_OGRE 5

static Enemy g_enemies[MAX_ENEMIES];
static Projectile g_projectiles[MAX_PROJECTILES];
static FloatingText g_floatingTexts[MAX_FLOATING_TEXTS];
static Trap g_traps[MAX_TRAPS];
static int g_blizzTimer = 0;

static Point g_waypoints[MAX_WAYPOINTS];
static int g_nextEnemyId = 1;

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

void InitMaps() {
    // 0: Forest Outpost
    lstrcpyA(g_maps[0].name, "Forest Outpost"); g_maps[0].bg = RGB(10, 26, 10); g_maps[0].path = RGB(45, 55, 45);
    Point wp0[] = {{40,200}, {330,200}, {330,390}, {520,390}, {520,220}, {730,220}};
    for(int i=0;i<6;i++) g_maps[0].waypoints[i] = wp0[i];
    Point sl0[] = {{140,110},{270,110},{400,110},{540,110},{140,300},{270,300},{400,300},{540,300},{140,470},{270,470},{400,470},{540,470}};
    g_maps[0].numSlots = 12; for(int i=0;i<12;i++) g_maps[0].slots[i] = sl0[i];
    g_maps[0].numObs = 3; g_maps[0].obs[0]=(Obstacle){100,380,"T"}; g_maps[0].obs[1]=(Obstacle){450,150,"T"}; g_maps[0].obs[2]=(Obstacle){600,400,"T"};

    // 1: Desert Pass
    lstrcpyA(g_maps[1].name, "Desert Pass"); g_maps[1].bg = RGB(42, 28, 10); g_maps[1].path = RGB(74, 53, 24);
    Point wp1[] = {{40,100}, {200,100}, {200,450}, {600,450}, {600,200}, {730,200}};
    for(int i=0;i<6;i++) g_maps[1].waypoints[i] = wp1[i];
    Point sl1[] = {{120,180},{280,180},{280,380},{500,380},{500,280},{680,280},{120,300}};
    g_maps[1].numSlots = 7; for(int i=0;i<7;i++) g_maps[1].slots[i] = sl1[i];
    g_maps[1].numObs = 3; g_maps[1].obs[0]=(Obstacle){350,250,"C"}; g_maps[1].obs[1]=(Obstacle){450,120,"C"}; g_maps[1].obs[2]=(Obstacle){650,350,"R"};

    // 2: Frozen Fortress
    lstrcpyA(g_maps[2].name, "Frozen Fortress"); g_maps[2].bg = RGB(10, 21, 42); g_maps[2].path = RGB(31, 59, 90);
    Point wp2[] = {{40,450}, {400,450}, {400,150}, {600,150}, {600,350}, {730,350}};
    for(int i=0;i<6;i++) g_maps[2].waypoints[i] = wp2[i];
    Point sl2[] = {{150,350},{300,350},{280,250},{480,250},{500,150},{500,450}};
    g_maps[2].numSlots = 6; for(int i=0;i<6;i++) g_maps[2].slots[i] = sl2[i];
    g_maps[2].numObs = 3; g_maps[2].obs[0]=(Obstacle){200,200,"I"}; g_maps[2].obs[1]=(Obstacle){500,80,"S"}; g_maps[2].obs[2]=(Obstacle){150,150,"M"};

    // 3: Volcanic Citadel
    lstrcpyA(g_maps[3].name, "Volcanic Citadel"); g_maps[3].bg = RGB(42, 10, 10); g_maps[3].path = RGB(74, 28, 28);
    Point wp3[] = {{40,150}, {150,150}, {150,400}, {500,400}, {500,150}, {730,150}};
    for(int i=0;i<6;i++) g_maps[3].waypoints[i] = wp3[i];
    Point sl3[] = {{250,250},{350,250},{250,320},{350,320},{600,250},{600,350}};
    g_maps[3].numSlots = 6; for(int i=0;i<6;i++) g_maps[3].slots[i] = sl3[i];
    g_maps[3].numObs = 3; g_maps[3].obs[0]=(Obstacle){300,200,"V"}; g_maps[3].obs[1]=(Obstacle){650,400,"F"}; g_maps[3].obs[2]=(Obstacle){100,300,"F"};

    // 4: Swamp of Sorrows
    lstrcpyA(g_maps[4].name, "Swamp of Sorrows"); g_maps[4].bg = RGB(21, 42, 21); g_maps[4].path = RGB(44, 62, 44);
    Point wp4[] = {{40,300}, {250,300}, {250,150}, {600,150}, {600,450}, {730,450}};
    for(int i=0;i<6;i++) g_maps[4].waypoints[i] = wp4[i];
    Point sl4[] = {{150,200},{150,400},{350,250},{450,250},{500,350},{700,350}};
    g_maps[4].numSlots = 6; for(int i=0;i<6;i++) g_maps[4].slots[i] = sl4[i];
    g_maps[4].numObs = 3; g_maps[4].obs[0]=(Obstacle){200,400,"M"}; g_maps[4].obs[1]=(Obstacle){400,100,"S"}; g_maps[4].obs[2]=(Obstacle){500,250,"W"};

    // 5: Crystal Caves
    lstrcpyA(g_maps[5].name, "Crystal Caves"); g_maps[5].bg = RGB(26, 10, 42); g_maps[5].path = RGB(53, 31, 74);
    Point wp5[] = {{40,400}, {200,400}, {200,200}, {450,200}, {450,350}, {730,350}};
    for(int i=0;i<6;i++) g_maps[5].waypoints[i] = wp5[i];
    Point sl5[] = {{100,300},{300,300},{300,100},{550,150},{550,450},{650,250}};
    g_maps[5].numSlots = 6; for(int i=0;i<6;i++) g_maps[5].slots[i] = sl5[i];
    g_maps[5].numObs = 3; g_maps[5].obs[0]=(Obstacle){150,150,"C"}; g_maps[5].obs[1]=(Obstacle){350,450,"C"}; g_maps[5].obs[2]=(Obstacle){600,100,"C"};

    // 6: Haunted Graveyard
    lstrcpyA(g_maps[6].name, "Haunted Graveyard"); g_maps[6].bg = RGB(10, 12, 16); g_maps[6].path = RGB(31, 41, 55);
    Point wp6[] = {{40,250}, {150,250}, {150,100}, {550,100}, {550,300}, {730,300}};
    for(int i=0;i<6;i++) g_maps[6].waypoints[i] = wp6[i];
    Point sl6[] = {{250,180},{350,180},{450,180},{250,280},{350,280},{450,280}};
    g_maps[6].numSlots = 6; for(int i=0;i<6;i++) g_maps[6].slots[i] = sl6[i];
    g_maps[6].numObs = 3; g_maps[6].obs[0]=(Obstacle){200,400,"G"}; g_maps[6].obs[1]=(Obstacle){400,350,"G"}; g_maps[6].obs[2]=(Obstacle){650,150,"X"};

    // 7: Sky Kingdom
    lstrcpyA(g_maps[7].name, "Sky Kingdom"); g_maps[7].bg = RGB(10, 37, 58); g_maps[7].path = RGB(47, 90, 122);
    Point wp7[] = {{40,100}, {300,100}, {300,450}, {600,450}, {600,250}, {730,250}};
    for(int i=0;i<6;i++) g_maps[7].waypoints[i] = wp7[i];
    Point sl7[] = {{150,180},{200,280},{400,350},{500,350},{500,150},{700,350}};
    g_maps[7].numSlots = 6; for(int i=0;i<6;i++) g_maps[7].slots[i] = sl7[i];
    g_maps[7].numObs = 3; g_maps[7].obs[0]=(Obstacle){100,400,"W"}; g_maps[7].obs[1]=(Obstacle){450,150,"W"}; g_maps[7].obs[2]=(Obstacle){650,100,"W"};

    // 8: Dragon's Peak
    lstrcpyA(g_maps[8].name, "Dragon's Peak"); g_maps[8].bg = RGB(42, 16, 10); g_maps[8].path = RGB(74, 44, 31);
    Point wp8[] = {{40,350}, {350,350}, {350,150}, {550,150}, {550,400}, {730,400}};
    for(int i=0;i<6;i++) g_maps[8].waypoints[i] = wp8[i];
    Point sl8[] = {{200,250},{300,250},{450,250},{450,350},{650,250},{650,150}};
    g_maps[8].numSlots = 6; for(int i=0;i<6;i++) g_maps[8].slots[i] = sl8[i];
    g_maps[8].numObs = 3; g_maps[8].obs[0]=(Obstacle){150,150,"D"}; g_maps[8].obs[1]=(Obstacle){250,450,"F"}; g_maps[8].obs[2]=(Obstacle){500,80,"V"};

    // 9: The Void Abyss
    lstrcpyA(g_maps[9].name, "The Void Abyss"); g_maps[9].bg = RGB(5, 5, 16); g_maps[9].path = RGB(31, 31, 58);
    Point wp9[] = {{40,200}, {150,400}, {350,150}, {550,450}, {650,250}, {730,250}};
    for(int i=0;i<6;i++) g_maps[9].waypoints[i] = wp9[i];
    Point sl9[] = {{150,250},{250,280},{350,300},{450,250},{550,250},{600,150}};
    g_maps[9].numSlots = 6; for(int i=0;i<6;i++) g_maps[9].slots[i] = sl9[i];
    g_maps[9].numObs = 3; g_maps[9].obs[0]=(Obstacle){100,100,"X"}; g_maps[9].obs[1]=(Obstacle){450,100,"X"}; g_maps[9].obs[2]=(Obstacle){300,400,"X"};
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
        g_slots[i].maxCooldown = 18; // ~0.6s
        g_slots[i].range = 130;
        g_slots[i].damage = 12;
        g_slots[i].splash = 0;
    }
}

void LoadGame() {
    FILE *f = fopen("kfortress.dat", "rb");
    if (f) {
        fread(&g_techStartingGold, sizeof(int), 1, f);
        fread(&g_techWallHp, sizeof(int), 1, f);
        fread(&g_techHeroCd, sizeof(int), 1, f);
        fread(&g_techTowerDmg, sizeof(int), 1, f);
        fread(&g_hsEndless, sizeof(int), 1, f);
        fread(&g_hsBoss, sizeof(int), 1, f);
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
        fwrite(&g_hsEndless, sizeof(int), 1, f);
        fwrite(&g_hsBoss, sizeof(int), 1, f);
        fclose(f);
    }
}

void InitGameState() {
    LoadGame();
    g_gold = 100 + g_techStartingGold * 50;
    if (g_gameMode == 2) g_gold += 200;
    g_baseHp = 20 + g_techWallHp * 10;
    g_maxBaseHp = 20 + g_techWallHp * 10;
    g_wave = 1;
    g_bossesKilled = 0;
    g_waveActive = FALSE;
    g_gameOver = FALSE;
    g_selectedSlot = -1;
    g_spawnQueueCount = 0;
    g_spawnQueueHead = 0;
    g_spawnTimer = 0;
    g_blizzTimer = 0;
    for(int i=0; i<MAX_TRAPS; i++) g_traps[i].active = FALSE;

    g_hero.x = 400.0f; g_hero.y = 300.0f;
    g_hero.targetX = 400.0f; g_hero.targetY = 300.0f;
    g_hero.maxHp = 100.0f; g_hero.hp = 100.0f;
    g_hero.speed = 2.5f;
    g_hero.damage = 25; g_hero.range = 60;
    g_hero.attackCd = 0; g_hero.maxAttackCd = 25;
    g_hero.respawnTimer = 0;
    
    float cdMod = 1.0f - (g_techHeroCd * 0.1f);
    g_hero.healCd = 0; g_hero.maxHealCd = (int)(300 * cdMod);
    if(g_hero.maxHealCd < 30) g_hero.maxHealCd = 30;
    g_hero.shieldCd = 0; g_hero.maxShieldCd = (int)(600 * cdMod);
    if(g_hero.maxShieldCd < 60) g_hero.maxShieldCd = 60;
    g_hero.meteorCd = 0; g_hero.maxMeteorCd = (int)(450 * cdMod);
    if(g_hero.maxMeteorCd < 45) g_hero.maxMeteorCd = 45;
    
    g_hero.shieldActive = 0;

    for (int i = 0; i < MAX_ENEMIES; i++) g_enemies[i].active = FALSE;
    for (int i = 0; i < MAX_PROJECTILES; i++) g_projectiles[i].active = FALSE;
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) g_floatingTexts[i].active = FALSE;

    int bfX = 10, bfY = 70, bfW = WINDOW_WIDTH - 220;
    InitMaps();
    LoadCurrentMap(bfX, bfY, bfW);
}

void UpdateGameLogic() {
    if (g_gameOver) return;

    // Spawning Logic
    if (g_waveActive && g_spawnQueueHead < g_spawnQueueCount) {
        g_spawnTimer++;
        if (g_spawnTimer >= 35) { // ~1.1s
            g_spawnTimer = 0;
    g_blizzTimer = 0;
    for(int i=0; i<MAX_TRAPS; i++) g_traps[i].active = FALSE;
            int type = g_spawnQueue[g_spawnQueueHead++];

            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (!g_enemies[i].active) {
                    g_enemies[i].active = TRUE;
                    g_enemies[i].id = g_nextEnemyId++;
                    g_enemies[i].x = (float)g_waypoints[0].x;
                    g_enemies[i].y = (float)g_waypoints[0].y;
                    g_enemies[i].type = type;
                    g_enemies[i].waypointIndex = (type == ENEMY_GARGOYLE) ? (MAX_WAYPOINTS - 1) : 0;
                    
                    if (type == ENEMY_GOBLIN) { g_enemies[i].hp = 25 + g_wave * 5; g_enemies[i].speed = 2.0f; g_enemies[i].radius = 11; }
                    else if (type == ENEMY_ORC) { g_enemies[i].hp = 60 + g_wave * 12; g_enemies[i].speed = 1.2f; g_enemies[i].radius = 13; }
                    else if (type == ENEMY_HOUND) { g_enemies[i].hp = 20 + g_wave * 4; g_enemies[i].speed = 3.5f; g_enemies[i].radius = 9; }
                    else if (type == ENEMY_GARGOYLE) { g_enemies[i].hp = 30 + g_wave * 5; g_enemies[i].speed = 1.8f; g_enemies[i].radius = 12; }
                    else if (type == ENEMY_OGRE) { 
                        g_enemies[i].hp = 150 + g_wave * 30; 
                        if (g_gameMode == 2) g_enemies[i].hp += g_wave * 50;
                        g_enemies[i].speed = (g_gameMode == 2) ? 0.8f : 0.6f; 
                        g_enemies[i].radius = 16; 
                        Beep(100, 100); Beep(80, 100);
                    }
                    
                    g_enemies[i].maxHp = g_enemies[i].hp;
                    break;
                }
            }
        }
    }

    // Hero Update
    if (g_hero.healCd > 0) g_hero.healCd--;
    if (g_hero.shieldCd > 0) g_hero.shieldCd--;
    if (g_hero.meteorCd > 0) g_hero.meteorCd--;
    if (g_hero.shieldActive > 0) g_hero.shieldActive--;

    if (g_hero.respawnTimer > 0) {
        g_hero.respawnTimer--;
        if (g_hero.respawnTimer <= 0) {
            g_hero.hp = g_hero.maxHp;
            g_hero.x = (float)g_waypoints[MAX_WAYPOINTS-1].x;
            g_hero.y = (float)g_waypoints[MAX_WAYPOINTS-1].y;
            g_hero.targetX = g_hero.x;
            g_hero.targetY = g_hero.y;
            AddFloatingText(g_hero.x, g_hero.y - 20, "RESPAWNED!", TEXT_GOLD);
        }
    } else {
        float hdx = g_hero.targetX - g_hero.x;
        float hdy = g_hero.targetY - g_hero.y;
        float hDist = custom_sqrtf(hdx*hdx + hdy*hdy);
        if (hDist > g_hero.speed) {
            g_hero.x += (hdx/hDist) * g_hero.speed;
            g_hero.y += (hdy/hDist) * g_hero.speed;
        } else {
            g_hero.x = g_hero.targetX;
            g_hero.y = g_hero.targetY;
        }

        if (g_hero.attackCd > 0) g_hero.attackCd--;
        else {
            int targetIdx = -1;
            float closestDist = 9999.0f;
            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (g_enemies[e].active) {
                    float dx = g_enemies[e].x - g_hero.x;
                    float dy = g_enemies[e].y - g_hero.y;
                    float dist = custom_sqrtf(dx*dx + dy*dy);
                    if (dist <= g_hero.range && dist < closestDist) {
                        closestDist = dist;
                        targetIdx = e;
                    }
                }
            }
            if (targetIdx != -1) {
                g_hero.attackCd = g_hero.maxAttackCd;
                g_enemies[targetIdx].hp -= g_hero.damage;
                if (g_enemies[targetIdx].hp <= 0) {
                    g_enemies[targetIdx].active = FALSE;
                    int reward = 15;
                    if (g_enemies[targetIdx].type == ENEMY_OGRE) {
                        reward = 100;
                        if (g_gameMode == 2) {
                            g_bossesKilled++;
                            if (g_bossesKilled > g_hsBoss) { g_hsBoss = g_bossesKilled; SaveGame(); }
                        }
                    }
                    else if (g_enemies[targetIdx].type == ENEMY_ORC) reward = 20;
                    else if (g_enemies[targetIdx].type == ENEMY_HOUND) reward = 10;
                    g_gold += reward;
                    char rBuf[16]; wsprintfA(rBuf, "+%dg", reward);
                    AddFloatingText(g_enemies[targetIdx].x, g_enemies[targetIdx].y - 10, rBuf, TEXT_GOLD);
                    Beep(700, 25);
                }
                Beep(550, 20);
            }
        }
    }

    // Update Enemies (Pathfinding)

    if (g_blizzTimer > 0) g_blizzTimer--;
    int activeEnemyCount = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!g_enemies[i].active) continue;
        activeEnemyCount++;

        g_enemies[i].slowed = (g_blizzTimer > 0);
        for (int t = 0; t < g_slotCount; t++) {
            if (!g_enemies[i].slowed && g_slots[t].occupied && g_slots[t].towerType == TOWER_FROST) {
                float fdx = g_enemies[i].x - g_slots[t].x;
                float fdy = g_enemies[i].y - g_slots[t].y;
                if (custom_sqrtf(fdx*fdx + fdy*fdy) <= (float)g_slots[t].range) {
                    g_enemies[i].slowed = TRUE;
                    break;
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
                        g_enemies[i].hp -= 50;
                        g_traps[tr].charges--;
                        if (g_traps[tr].charges <= 0) g_traps[tr].active = FALSE;
                    } else if (g_traps[tr].type == TRAP_OIL) {
                        g_enemies[i].slowed = TRUE;
                    } else if (g_traps[tr].type == TRAP_BARRICADE) {
                        trapBlocked = TRUE;
                        g_traps[tr].hp -= 0.5f;
                        if (g_traps[tr].hp <= 0) g_traps[tr].active = FALSE;
                    }
                }
            }
        }
        
        if (g_enemies[i].hp <= 0) {
            g_enemies[i].active = FALSE;
            g_gold += 15;
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
        if (g_enemies[i].type == ENEMY_GARGOYLE) nextIdx = MAX_WAYPOINTS - 1;
        
        Point targetWP = g_waypoints[nextIdx];
        float dx = targetWP.x - g_enemies[i].x;
        float dy = targetWP.y - g_enemies[i].y;
        float dist = custom_sqrtf(dx * dx + dy * dy);

        if (dist < currentSpeed) {
            g_enemies[i].x = (float)targetWP.x;
            g_enemies[i].y = (float)targetWP.y;
            
            if (g_enemies[i].type == ENEMY_GARGOYLE) {
                g_enemies[i].waypointIndex = MAX_WAYPOINTS;
            } else {
                g_enemies[i].waypointIndex++;
            }

            if (g_enemies[i].waypointIndex >= MAX_WAYPOINTS - 1) {
                // Reached Castle Fortress
                int dmgToBase = (g_enemies[i].type == ENEMY_OGRE) ? 5 : 1;
                if (g_hero.shieldActive > 0) dmgToBase = 0;
                g_baseHp -= dmgToBase;
                if (dmgToBase > 0) {
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

    // Update Towers & Target Acquisition
    for (int i = 0; i < g_slotCount; i++) {
        if (!g_slots[i].occupied) continue;
        if (g_slots[i].towerType == TOWER_FROST) continue;

        if (g_slots[i].cooldown > 0) {
            g_slots[i].cooldown--;
        } else {
            // Find target enemy in range
            int targetIdx = -1;
            float maxProgress = -1.0f;

            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (!g_enemies[e].active) continue;

                float edx = g_enemies[e].x - g_slots[i].x;
                float edy = g_enemies[e].y - g_slots[i].y;
                float edist = custom_sqrtf(edx * edx + edy * edy);

                if (edist <= g_slots[i].range) {
                    float progress = g_enemies[e].waypointIndex * 1000.0f + edist;
                    if (progress > maxProgress) {
                        maxProgress = progress;
                        targetIdx = e;
                    }
                }
            }

            if (targetIdx != -1) {
                g_slots[i].cooldown = g_slots[i].maxCooldown;

                // Spawn Projectile
                for (int p = 0; p < MAX_PROJECTILES; p++) {
                    if (!g_projectiles[p].active) {
                        g_projectiles[p].active = TRUE;
                        g_projectiles[p].x = (float)g_slots[i].x;
                        g_projectiles[p].y = (float)g_slots[i].y;
                        g_projectiles[p].targetEnemyId = g_enemies[targetIdx].id;
                        g_projectiles[p].targetX = g_enemies[targetIdx].x;
                        g_projectiles[p].targetY = g_enemies[targetIdx].y;
                        g_projectiles[p].speed = (g_slots[i].towerType == TOWER_CANNON) ? 7.0f : 10.0f;
                        g_projectiles[p].damage = g_slots[i].damage;
                        g_projectiles[p].type = g_slots[i].towerType;
                        g_projectiles[p].splash = g_slots[i].splash;
                        if (g_slots[i].towerType == TOWER_CANNON) { Beep(150, 80); Beep(100, 80); }
                        else if (g_slots[i].towerType == TOWER_MAGE) { Beep(900, 40); Beep(1200, 40); }
                        else { Beep(800, 30); Beep(600, 30); }
                        break;
                    }
                }
            }
        }
    }

    // Update Projectiles
    for (int p = 0; p < MAX_PROJECTILES; p++) {
        if (!g_projectiles[p].active) continue;

        // Find target enemy
        int targetIdx = -1;
        for (int e = 0; e < MAX_ENEMIES; e++) {
            if (g_enemies[e].active && g_enemies[e].id == g_projectiles[p].targetEnemyId) {
                targetIdx = e;
                g_projectiles[p].targetX = g_enemies[e].x;
                g_projectiles[p].targetY = g_enemies[e].y;
                break;
            }
        }

        float dx = g_projectiles[p].targetX - g_projectiles[p].x;
        float dy = g_projectiles[p].targetY - g_projectiles[p].y;
        float dist = custom_sqrtf(dx * dx + dy * dy);

        if (dist < g_projectiles[p].speed) {
            // Hit target
            if (g_projectiles[p].splash > 0) {
                for (int e2 = 0; e2 < MAX_ENEMIES; e2++) {
                    if (!g_enemies[e2].active) continue;
                    float edx = g_enemies[e2].x - g_projectiles[p].targetX;
                    float edy = g_enemies[e2].y - g_projectiles[p].targetY;
                    if (custom_sqrtf(edx*edx + edy*edy) <= (float)g_projectiles[p].splash) {
                        int dmg = g_projectiles[p].damage;
                        if (g_enemies[e2].type == ENEMY_ORC && g_projectiles[p].type != TOWER_MAGE && g_projectiles[p].type != TOWER_CANNON) {
                            dmg = dmg / 2; if (dmg < 1) dmg = 1;
                        }
                        g_enemies[e2].hp -= dmg;
                        if (g_enemies[e2].hp <= 0) {
                            g_enemies[e2].active = FALSE;
                            int reward = 15;
                            if (g_enemies[e2].type == ENEMY_OGRE) {
                                reward = 100;
                                if (g_gameMode == 2) { g_bossesKilled++; if (g_bossesKilled > g_hsBoss) { g_hsBoss = g_bossesKilled; SaveGame(); } }
                            }
                            else if (g_enemies[e2].type == ENEMY_ORC) reward = 20;
                            else if (g_enemies[e2].type == ENEMY_HOUND) reward = 10;
                            g_gold += reward;
                            char rBuf[16]; wsprintfA(rBuf, "+%dg", reward);
                            AddFloatingText(g_enemies[e2].x, g_enemies[e2].y - 10, rBuf, TEXT_GOLD);
                        }
                    }
                }
            } else {
                if (targetIdx != -1) {
                    int dmg = g_projectiles[p].damage;
                    if (g_enemies[targetIdx].type == ENEMY_ORC && g_projectiles[p].type != TOWER_MAGE && g_projectiles[p].type != TOWER_CANNON) {
                        dmg = dmg / 2; if (dmg < 1) dmg = 1;
                    }
                    g_enemies[targetIdx].hp -= dmg;
                    if (g_enemies[targetIdx].hp <= 0) {
                        g_enemies[targetIdx].active = FALSE;
                        int reward = 15;
                        if (g_enemies[targetIdx].type == ENEMY_OGRE) {
                            reward = 100;
                            if (g_gameMode == 2) { g_bossesKilled++; if (g_bossesKilled > g_hsBoss) { g_hsBoss = g_bossesKilled; SaveGame(); } }
                        }
                        else if (g_enemies[targetIdx].type == ENEMY_ORC) reward = 20;
                        else if (g_enemies[targetIdx].type == ENEMY_HOUND) reward = 10;
                        g_gold += reward;
                        char rBuf[16]; wsprintfA(rBuf, "+%dg", reward);
                        AddFloatingText(g_enemies[targetIdx].x, g_enemies[targetIdx].y - 10, rBuf, TEXT_GOLD);
                        Beep(700, 25);
                    }
                }
            }
            Beep((g_projectiles[p].type == TOWER_CANNON) ? 200 : ((g_projectiles[p].type == TOWER_MAGE) ? 400 : 450), 15);
            g_projectiles[p].active = FALSE;
        } else {
            g_projectiles[p].x += (dx / dist) * g_projectiles[p].speed;
            g_projectiles[p].y += (dy / dist) * g_projectiles[p].speed;
        }
    }

    // Update Floating Texts
    for (int f = 0; f < MAX_FLOATING_TEXTS; f++) {
        if (!g_floatingTexts[f].active) continue;
        g_floatingTexts[f].y -= 0.8f;
        g_floatingTexts[f].life--;
        if (g_floatingTexts[f].life <= 0) {
            g_floatingTexts[f].active = FALSE;
        }
    }

    // Check Wave Completion
    if (g_waveActive && g_spawnQueueHead == g_spawnQueueCount && activeEnemyCount == 0) {
        g_waveActive = FALSE;
        int bonus = 20 + g_wave * 5;
        if (g_gameMode == 2) bonus = 100 + g_wave * 20;
        g_gold += bonus;

        char buf[32];
        wsprintfA(buf, "WAVE CLEAR! +%dg", bonus);
        AddFloatingText((float)(WINDOW_WIDTH / 2 - 50), (float)(WINDOW_HEIGHT / 2), buf, RGB(16, 185, 129));

        if (g_gameMode == 1) {
            if (g_wave > g_hsEndless) { g_hsEndless = g_wave; SaveGame(); }
        }

        g_wave++;
        Beep(523, 150); Beep(659, 150); Beep(784, 150); Beep(1046, 300);
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

void Render(HDC hdc, HWND hwnd) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int w = clientRect.right;
    int h = clientRect.bottom;

    // Create Backbuffer
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, w, h);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // Background
    HBRUSH bgBrush = CreateSolidBrush(BG_COLOR);
    FillRect(memDC, &clientRect, bgBrush);
    DeleteObject(bgBrush);

    SetBkMode(memDC, TRANSPARENT);

    // Top Header / HUD Bar
    DrawRoundedRect(memDC, 10, 10, w - 10, 60, CARD_BG, BORDER_COLOR, 8);

    HFONT hFontTitle = CreateFontA(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    HFONT oldFont = (HFONT)SelectObject(memDC, hFontTitle);

    SetTextColor(memDC, TEXT_GOLD);
    TextOutA(memDC, 25, 22, "KFORTRESS", 9);

    HFONT hFontSub = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SelectObject(memDC, hFontSub);
    SetTextColor(memDC, TEXT_MUTED);
    TextOutA(memDC, 140, 26, "Phase 12: Game Modes", 20);

    // Stats HUD
    char buf[128];
    HFONT hFontStat = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SelectObject(memDC, hFontStat);

    SetTextColor(memDC, TEXT_GOLD);
    wsprintfA(buf, "Gold: %d", g_gold);
    SetTextColor(memDC, RGB(180, 140, 20));
    TextOutA(memDC, w - 321, 25, buf, (int)lstrlenA(buf));
    TextOutA(memDC, w - 319, 23, buf, (int)lstrlenA(buf));
    SetTextColor(memDC, TEXT_GOLD);
    TextOutA(memDC, w - 320, 24, buf, (int)lstrlenA(buf));

    SetTextColor(memDC, TEXT_RED);
    wsprintfA(buf, "Base HP: %d/%d", g_baseHp, g_maxBaseHp);
    SetTextColor(memDC, RGB(150, 30, 50));
    TextOutA(memDC, w - 211, 25, buf, (int)lstrlenA(buf));
    TextOutA(memDC, w - 209, 23, buf, (int)lstrlenA(buf));
    SetTextColor(memDC, TEXT_RED);
    TextOutA(memDC, w - 210, 24, buf, (int)lstrlenA(buf));

    SetTextColor(memDC, TEXT_WHITE);
    wsprintfA(buf, "Wave: %d", g_wave);
    TextOutA(memDC, w - 90, 24, buf, (int)lstrlenA(buf));

    DrawRoundedRect(memDC, w - 410, 20, w - 340, 45, RGB(59, 130, 246), BORDER_COLOR, 4);
    SetTextColor(memDC, TEXT_WHITE);
    SelectObject(memDC, hFontStat);
    TextOutA(memDC, w - 402, 23, "HELP", 4);

    DeleteObject(hFontTitle);
    DeleteObject(hFontSub);
    DeleteObject(hFontStat);

    // Battlefield Area
    int bfX = 10, bfY = 70, bfW = w - 220, bfH = h - 80;
    DrawRoundedRect(memDC, bfX, bfY, bfX + bfW, bfY + bfH, g_maps[g_currentMap].bg, BORDER_COLOR, 8);

    // Draw Map Obstacles
    HFONT hObsFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SelectObject(memDC, hObsFont);
    SetTextColor(memDC, RGB(255,255,255));
    SetBkMode(memDC, TRANSPARENT);
    for (int i=0; i<g_maps[g_currentMap].numObs; i++) {
        Obstacle* obs = &g_maps[g_currentMap].obs[i];
        TextOutA(memDC, bfX + obs->x - 10, bfY + obs->y - 20, obs->type, lstrlenA(obs->type));
    }
    DeleteObject(hObsFont);

    // Draw Winding Path
    POINT pts[MAX_WAYPOINTS];
    for (int i = 0; i < MAX_WAYPOINTS; i++) {
        pts[i].x = g_waypoints[i].x;
        pts[i].y = g_waypoints[i].y;
    }

    HPEN pathPen = CreatePen(PS_SOLID, 42, g_maps[g_currentMap].path);
    HPEN oldPen = (HPEN)SelectObject(memDC, pathPen);
    Polyline(memDC, pts, MAX_WAYPOINTS);

    HPEN pathBorderPen = CreatePen(PS_SOLID, 2, BORDER_COLOR);
    SelectObject(memDC, pathBorderPen);
    Polyline(memDC, pts, MAX_WAYPOINTS);
    SelectObject(memDC, oldPen);
    DeleteObject(pathPen);
    DeleteObject(pathBorderPen);

    // Draw Range Circle for selected slot
    if (g_selectedSlot != -1) {
        HPEN glowPen1 = CreatePen(PS_SOLID, 4, RGB(120, 90, 20));
        HPEN glowPen2 = CreatePen(PS_SOLID, 1, TEXT_GOLD);
        HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
        
        int sx = g_slots[g_selectedSlot].x;
        int sy = g_slots[g_selectedSlot].y;
        int r = g_slots[g_selectedSlot].range;
        
        HPEN oP = (HPEN)SelectObject(memDC, glowPen1);
        HBRUSH oB = (HBRUSH)SelectObject(memDC, nullBrush);
        Ellipse(memDC, sx - r, sy - r, sx + r, sy + r);
        
        SelectObject(memDC, glowPen2);
        Ellipse(memDC, sx - r, sy - r, sx + r, sy + r);

        SelectObject(memDC, oP);
        SelectObject(memDC, oB);
        DeleteObject(glowPen1);
        DeleteObject(glowPen2);
    }

    // Draw Spawn Gate
    DrawRoundedRect(memDC, g_waypoints[0].x - 22, g_waypoints[0].y - 22, g_waypoints[0].x + 22, g_waypoints[0].y + 22, RGB(180, 40, 40), RGB(239, 68, 68), 6);
    SelectObject(memDC, oldFont);
    SetTextColor(memDC, TEXT_WHITE);
    TextOutA(memDC, g_waypoints[0].x - 14, g_waypoints[0].y - 6, "GATE", 4);

    // Draw Castle Fortress Base
    DrawRoundedRect(memDC, g_waypoints[5].x - 30, g_waypoints[5].y - 30, g_waypoints[5].x + 35, g_waypoints[5].y + 35, CASTLE_COLOR, TEXT_GOLD, 8);
    SetTextColor(memDC, TEXT_GOLD);
    TextOutA(memDC, g_waypoints[5].x - 24, g_waypoints[5].y - 8, "CASTLE", 6);

    // Draw Tower Slots
    for (int i = 0; i < g_slotCount; i++) {
        COLORREF fill = (g_selectedSlot == i) ? TOWER_SLOT_HOVER : TOWER_SLOT_BG;
        COLORREF border = (g_selectedSlot == i) ? TEXT_GOLD : BORDER_COLOR;
        DrawRoundedRect(memDC, g_slots[i].x - 22, g_slots[i].y - 22, g_slots[i].x + 22, g_slots[i].y + 22, fill, border, 6);

        if (g_slots[i].occupied) {
            COLORREF tColor = CASTLE_COLOR;
            COLORREF bColor = TEXT_GOLD;
            const char* lbl = "[A]";
            if (g_slots[i].towerType == TOWER_MAGE) { tColor = RGB(126, 34, 206); bColor = RGB(216, 180, 254); lbl = "[M]"; }
            else if (g_slots[i].towerType == TOWER_CANNON) { tColor = RGB(51, 65, 85); bColor = RGB(148, 163, 184); lbl = "[C]"; }
            else if (g_slots[i].towerType == TOWER_FROST) { tColor = RGB(14, 165, 233); bColor = RGB(186, 230, 253); lbl = "[F]"; }

            HBRUSH tBrush = CreateSolidBrush(tColor);
            HPEN tPen = CreatePen(PS_SOLID, 2, bColor);
            HBRUSH oB = (HBRUSH)SelectObject(memDC, tBrush);
            HPEN oP = (HPEN)SelectObject(memDC, tPen);

            Ellipse(memDC, g_slots[i].x - 14, g_slots[i].y - 14, g_slots[i].x + 14, g_slots[i].y + 14);

            SelectObject(memDC, oB);
            SelectObject(memDC, oP);
            DeleteObject(tBrush);
            DeleteObject(tPen);

            if (g_slots[i].towerType == TOWER_FROST) {
                HPEN fPen = CreatePen(PS_SOLID, 1, RGB(14, 165, 233));
                HBRUSH fBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
                HPEN oldFP = (HPEN)SelectObject(memDC, fPen);
                HBRUSH oldFB = (HBRUSH)SelectObject(memDC, fBrush);
                Ellipse(memDC, g_slots[i].x - g_slots[i].range, g_slots[i].y - g_slots[i].range, g_slots[i].x + g_slots[i].range, g_slots[i].y + g_slots[i].range);
                SelectObject(memDC, oldFP);
                SelectObject(memDC, oldFB);
                DeleteObject(fPen);
            }

            SetTextColor(memDC, TEXT_WHITE);
            TextOutA(memDC, g_slots[i].x - 9, g_slots[i].y - 8, lbl, 3);
            
            SetTextColor(memDC, TEXT_GOLD);
            char lvlBuf[4];
            wsprintfA(lvlBuf, "L%d", g_slots[i].level);
            HFONT hLvlFont = CreateFontA(10, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            HFONT oldLF = (HFONT)SelectObject(memDC, hLvlFont);
            TextOutA(memDC, g_slots[i].x + 4, g_slots[i].y + 4, lvlBuf, lstrlenA(lvlBuf));
            SelectObject(memDC, oldLF);
            DeleteObject(hLvlFont);
        } else {
            SetTextColor(memDC, TEXT_MUTED);
            TextOutA(memDC, g_slots[i].x - 4, g_slots[i].y - 12, "+", 1);
            SetTextColor(memDC, TEXT_GOLD);
            HFONT hSmallFont = CreateFontA(10, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            SelectObject(memDC, hSmallFont);
            int cost = 50;
            if (g_selectedTowerTypeToBuild == TOWER_MAGE) cost = 100;
            else if (g_selectedTowerTypeToBuild == TOWER_CANNON) cost = 150;
            else if (g_selectedTowerTypeToBuild == TOWER_FROST) cost = 120;
            char buf[16]; wsprintfA(buf, "%dg", cost);
            TextOutA(memDC, g_slots[i].x - 10, g_slots[i].y + 2, buf, lstrlenA(buf));
            DeleteObject(hSmallFont);
        }
    }

    
    // Draw Traps
    for(int tr=0; tr<MAX_TRAPS; tr++) {
        if(!g_traps[tr].active) continue;
        int tx = (int)g_traps[tr].x; int ty = (int)g_traps[tr].y; int trr = g_traps[tr].radius;
        HBRUSH tB = NULL; HPEN tP = NULL; const char* lbl = "";
        if(g_traps[tr].type == TRAP_SPIKE) { tB = CreateSolidBrush(RGB(100,100,100)); tP = CreatePen(PS_SOLID, 1, RGB(70,70,70)); lbl = "S"; }
        else if(g_traps[tr].type == TRAP_OIL) { tB = CreateSolidBrush(RGB(20,20,20)); tP = CreatePen(PS_SOLID, 1, RGB(0,0,0)); lbl = "O"; }
        else if(g_traps[tr].type == TRAP_BARRICADE) { tB = CreateSolidBrush(RGB(139,69,19)); tP = CreatePen(PS_SOLID, 1, RGB(101,67,33)); lbl = "B"; }
        
        HBRUSH oB = (HBRUSH)SelectObject(memDC, tB); HPEN oP = (HPEN)SelectObject(memDC, tP);
        Ellipse(memDC, tx-trr, ty-trr, tx+trr, ty+trr);
        SelectObject(memDC, oB); SelectObject(memDC, oP);
        DeleteObject(tB); DeleteObject(tP);
        
        SetTextColor(memDC, TEXT_WHITE);
        TextOutA(memDC, tx-4, ty-6, lbl, 1);
        if(g_traps[tr].type == TRAP_BARRICADE) {
            DrawRoundedRect(memDC, tx-10, ty-15, tx+10, ty-12, RGB(0,0,0), RGB(0,0,0), 0);
            HBRUSH hpB = CreateSolidBrush(RGB(34, 197, 94));
            RECT hpR = { tx-10, ty-15, tx-10 + (int)(20*(g_traps[tr].hp/200.0f)), ty-12 };
            FillRect(memDC, &hpR, hpB); DeleteObject(hpB);
        }
    }

    // Draw Enemies

    for (int e = 0; e < MAX_ENEMIES; e++) {
        if (!g_enemies[e].active) continue;

        int ex = (int)g_enemies[e].x;
        int ey = (int)g_enemies[e].y;
        int r = g_enemies[e].radius;
        int t = g_enemies[e].type;

        COLORREF baseColor = GOBLIN_GREEN;
        if (t == ENEMY_ORC) baseColor = RGB(71, 85, 105);
        else if (t == ENEMY_HOUND) baseColor = RGB(30, 27, 75);
        else if (t == ENEMY_GARGOYLE) baseColor = RGB(100, 116, 139);
        else if (t == ENEMY_OGRE) baseColor = RGB(120, 53, 15);

        COLORREF gColor = g_enemies[e].slowed ? RGB(59, 130, 246) : baseColor;
        COLORREF gBorder = g_enemies[e].slowed ? RGB(29, 78, 216) : RGB(15, 23, 42);
        
        HBRUSH gobBrush = CreateSolidBrush(gColor);
        HPEN gobPen = CreatePen(PS_SOLID, 1, gBorder);
        HBRUSH oB = (HBRUSH)SelectObject(memDC, gobBrush);
        HPEN oP = (HPEN)SelectObject(memDC, gobPen);

        Ellipse(memDC, ex - r, ey - r, ex + r, ey + r);

        SelectObject(memDC, oB);
        SelectObject(memDC, oP);
        DeleteObject(gobBrush);
        DeleteObject(gobPen);

        // Draw Label
        const char* lbl = "G";
        if (t == ENEMY_ORC) lbl = "O";
        else if (t == ENEMY_HOUND) lbl = "H";
        else if (t == ENEMY_GARGOYLE) lbl = "F";
        else if (t == ENEMY_OGRE) lbl = "B";

        SetTextColor(memDC, TEXT_WHITE);
        HFONT hEFont = CreateFontA(r + 2, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        HFONT oldEF = (HFONT)SelectObject(memDC, hEFont);
        TextOutA(memDC, ex - r/2, ey - r/2 - 2, lbl, 1);
        SelectObject(memDC, oldEF);
        DeleteObject(hEFont);

        // HP Bar overhead
        int barW = (t == ENEMY_OGRE) ? 40 : 24;
        int barH = (t == ENEMY_OGRE) ? 6 : 4;
        float hpRatio = (float)g_enemies[e].hp / (float)g_enemies[e].maxHp;
        if (hpRatio < 0.0f) hpRatio = 0.0f;

        DrawRoundedRect(memDC, ex - barW / 2, ey - r - 8, ex + barW / 2, ey - r - 8 + barH, RGB(20, 20, 20), RGB(0, 0, 0), 2);
        COLORREF hpColor = hpRatio > 0.5f ? RGB(34, 197, 94) : (hpRatio > 0.25f ? RGB(234, 179, 8) : TEXT_RED);
        
        if (t == ENEMY_OGRE) {
            HPEN hpPenO = CreatePen(PS_SOLID, 2, TEXT_GOLD);
            HPEN oldHpPO = (HPEN)SelectObject(memDC, hpPenO);
            HBRUSH nullB = (HBRUSH)GetStockObject(NULL_BRUSH);
            HBRUSH oldHpB = (HBRUSH)SelectObject(memDC, nullB);
            Rectangle(memDC, ex - barW / 2, ey - r - 8, ex + barW / 2, ey - r - 8 + barH);
            SelectObject(memDC, oldHpPO);
            SelectObject(memDC, oldHpB);
            DeleteObject(hpPenO);
        } else {
            // Neon Glow Outline
            HBRUSH hpBrushGlow = CreateSolidBrush(hpColor);
            RECT hpRGlow = { ex - barW / 2 - 1, ey - r - 8 - 1, ex - barW / 2 + (int)(barW * hpRatio) + 1, ey - r - 8 + barH + 1 };
            FrameRect(memDC, &hpRGlow, hpBrushGlow);
            DeleteObject(hpBrushGlow);
        }

        HBRUSH hpBrush = CreateSolidBrush(hpColor);
        RECT hpR = { ex - barW / 2, ey - r - 8, ex - barW / 2 + (int)(barW * hpRatio), ey - r - 8 + barH };
        FillRect(memDC, &hpR, hpBrush);
        DeleteObject(hpBrush);
    }

    // Draw Projectiles (Arrows)
    for (int p = 0; p < MAX_PROJECTILES; p++) {
        if (!g_projectiles[p].active) continue;

        COLORREF pColor = TEXT_GOLD;
        if (g_projectiles[p].type == TOWER_MAGE) pColor = RGB(216, 180, 254);
        else if (g_projectiles[p].type == TOWER_CANNON) pColor = RGB(30, 41, 59);

        HPEN arrowPen = CreatePen(PS_SOLID, 2, pColor);
        HPEN oP = (HPEN)SelectObject(memDC, arrowPen);

        MoveToEx(memDC, (int)g_projectiles[p].x, (int)g_projectiles[p].y, NULL);
        LineTo(memDC, (int)g_projectiles[p].x + 4, (int)g_projectiles[p].y + 4);

        SelectObject(memDC, oP);
        DeleteObject(arrowPen);
    }

    // Draw Floating Text
    for (int f = 0; f < MAX_FLOATING_TEXTS; f++) {
        if (!g_floatingTexts[f].active) continue;
        SetTextColor(memDC, g_floatingTexts[f].color);
        TextOutA(memDC, (int)g_floatingTexts[f].x, (int)g_floatingTexts[f].y, g_floatingTexts[f].text, (int)lstrlenA(g_floatingTexts[f].text));
    }

    // Draw Hero
    if (g_hero.respawnTimer <= 0) {
        if (g_hero.shieldActive > 0) {
            HBRUSH sBrush = CreateSolidBrush(RGB(59, 130, 246));
            HPEN sPen = CreatePen(PS_SOLID, 2, RGB(96, 165, 250));
            HBRUSH oB = (HBRUSH)SelectObject(memDC, sBrush);
            HPEN oP = (HPEN)SelectObject(memDC, sPen);
            Ellipse(memDC, (int)g_hero.x - 18, (int)g_hero.y - 18, (int)g_hero.x + 18, (int)g_hero.y + 18);
            SelectObject(memDC, oB);
            SelectObject(memDC, oP);
            DeleteObject(sBrush);
            DeleteObject(sPen);
        }
        
        HBRUSH hBrush = CreateSolidBrush(RGB(245, 158, 11)); // Gold
        HPEN hPen = CreatePen(PS_SOLID, 2, TEXT_WHITE);
        HBRUSH oB = (HBRUSH)SelectObject(memDC, hBrush);
        HPEN oP = (HPEN)SelectObject(memDC, hPen);
        Ellipse(memDC, (int)g_hero.x - 12, (int)g_hero.y - 12, (int)g_hero.x + 12, (int)g_hero.y + 12);
        SelectObject(memDC, oB);
        SelectObject(memDC, oP);
        DeleteObject(hBrush);
        DeleteObject(hPen);
        
        SetTextColor(memDC, TEXT_WHITE);
        HFONT hPFont = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        HFONT oldPF = (HFONT)SelectObject(memDC, hPFont);
        TextOutA(memDC, (int)g_hero.x - 5, (int)g_hero.y - 7, "P", 1);
        SelectObject(memDC, oldPF);
        DeleteObject(hPFont);

        // HP bar
        int barW = 24, barH = 4;
        float hpRatio = g_hero.hp / g_hero.maxHp;
        if (hpRatio < 0.0f) hpRatio = 0.0f;
        DrawRoundedRect(memDC, (int)g_hero.x - barW/2, (int)g_hero.y - 20, (int)g_hero.x + barW/2, (int)g_hero.y - 20 + barH, RGB(20,20,20), RGB(0,0,0), 2);
        HBRUSH hpBrush = CreateSolidBrush(RGB(34, 197, 94));
        RECT hpR = { (int)g_hero.x - barW/2, (int)g_hero.y - 20, (int)g_hero.x - barW/2 + (int)(barW * hpRatio), (int)g_hero.y - 20 + barH };
        FillRect(memDC, &hpR, hpBrush);
        DeleteObject(hpBrush);
    }

    // Game Over Overlay Banner
    if (g_gameOver) {
        DrawRoundedRect(memDC, bfX + 50, bfY + 180, bfX + bfW - 50, bfY + 300, RGB(20, 24, 33), RGB(239, 68, 68), 12);
        HFONT hFontGO = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        SelectObject(memDC, hFontGO);
        SetTextColor(memDC, RGB(239, 68, 68));
        TextOutA(memDC, bfX + 180, bfY + 205, "DEFENSE FALLEN - GAME OVER!", 27);

        HFONT hFontSubGO = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        SelectObject(memDC, hFontSubGO);
        SetTextColor(memDC, TEXT_WHITE);
        TextOutA(memDC, bfX + 210, bfY + 245, "Click RESET DEFENSE to play again.", 34);

        DeleteObject(hFontGO);
        DeleteObject(hFontSubGO);
    }

    // Right Control Sidebar
    int sbX = w - 200, sbY = 70, sbW = 190, sbH = h - 80;
    DrawRoundedRect(memDC, sbX, sbY, sbX + sbW, sbY + sbH, CARD_BG, BORDER_COLOR, 8);

    DrawRoundedRect(memDC, sbX + 5, sbY + 5, sbX + 30, sbY + 25, RGB(44, 50, 62), BORDER_COLOR, 4);
    DrawRoundedRect(memDC, sbX + sbW - 35, sbY + 5, sbX + sbW - 10, sbY + 25, RGB(44, 50, 62), BORDER_COLOR, 4);
    HFONT hFontBtn = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SelectObject(memDC, hFontBtn);
    SetTextColor(memDC, TEXT_WHITE);
    TextOutA(memDC, sbX + 12, sbY + 8, "<", 1);
    TextOutA(memDC, sbX + sbW - 25, sbY + 8, ">", 1);
    SetTextColor(memDC, TEXT_GOLD);
    char mBuf[32]; wsprintfA(mBuf, "%s", g_maps[g_currentMap].name);
    TextOutA(memDC, sbX + 35 + (sbW - 70 - lstrlenA(mBuf)*6)/2, sbY + 8, mBuf, lstrlenA(mBuf));
    DeleteObject(hFontBtn);

    sbY += 25; // Shift everything else down

    HFONT hFontHeader = CreateFontA(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SelectObject(memDC, hFontHeader);
    
    // Modes
    SetTextColor(memDC, TEXT_GOLD);
    TextOutA(memDC, sbX + 15, sbY + 10, "GAME MODE", 9);
    
    DrawRoundedRect(memDC, sbX + 5, sbY + 30, sbX + 62, sbY + 55, g_gameMode == 0 ? TEXT_GOLD : RGB(44, 50, 62), BORDER_COLOR, 4);
    SetTextColor(memDC, g_gameMode == 0 ? RGB(0,0,0) : TEXT_WHITE);
    TextOutA(memDC, sbX + 10, sbY + 35, "Camp", 4);
    
    DrawRoundedRect(memDC, sbX + 65, sbY + 30, sbX + 122, sbY + 55, g_gameMode == 1 ? TEXT_GOLD : RGB(44, 50, 62), BORDER_COLOR, 4);
    SetTextColor(memDC, g_gameMode == 1 ? RGB(0,0,0) : TEXT_WHITE);
    TextOutA(memDC, sbX + 70, sbY + 35, "Endl", 4);
    
    DrawRoundedRect(memDC, sbX + 125, sbY + 30, sbX + 185, sbY + 55, g_gameMode == 2 ? TEXT_GOLD : RGB(44, 50, 62), BORDER_COLOR, 4);
    SetTextColor(memDC, g_gameMode == 2 ? RGB(0,0,0) : TEXT_WHITE);
    TextOutA(memDC, sbX + 130, sbY + 35, "Boss", 4);

    HFONT hSmallFont2 = CreateFontA(11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SelectObject(memDC, hSmallFont2);
    SetTextColor(memDC, TEXT_MUTED);
    char hsBuf[64]; wsprintfA(hsBuf, "Best Endl: %d | Boss: %d", g_hsEndless, g_hsBoss);
    TextOutA(memDC, sbX + 10, sbY + 60, hsBuf, lstrlenA(hsBuf));
    DeleteObject(hSmallFont2);
    
    sbY += 65;

    SelectObject(memDC, hFontHeader);
    SetTextColor(memDC, TEXT_GOLD);
    TextOutA(memDC, sbX + 15, sbY + 10, "COMMAND POST", 12);

    
    // Shop Info
    const char* tNames[] = {"Archer [A]", "Mage [M]", "Cannon [C]", "Frost [F]"};
    int tCosts[] = {50, 100, 150, 120};
    
    for (int i=0; i<4; i++) {
        COLORREF bg = (g_selectedTowerTypeToBuild == i+1) ? RGB(60, 70, 85) : RGB(30, 41, 59);
        COLORREF bd = (g_selectedTowerTypeToBuild == i+1) ? TEXT_GOLD : BORDER_COLOR;
        DrawRoundedRect(memDC, sbX + 5, sbY + 35 + i*37, sbX + sbW - 5, sbY + 68 + i*37, bg, bd, 4);
        SetTextColor(memDC, TEXT_WHITE);
        TextOutA(memDC, sbX + 10, sbY + 38 + i*37, tNames[i], lstrlenA(tNames[i]));
        SetTextColor(memDC, TEXT_GOLD);
        char buf[32]; wsprintfA(buf, "%dg", tCosts[i]);
        TextOutA(memDC, sbX + sbW - 35, sbY + 38 + i*37, buf, lstrlenA(buf));
    }
    
    // Traps
    const char* trapNames[] = {"Spike Trap", "Oil Slick", "Barricade"};
    int trapCosts[] = {30, 40, 50};
    for(int i=0; i<3; i++) {
        COLORREF bg = (g_selectedTowerTypeToBuild == TRAP_SPIKE+i) ? RGB(60, 70, 85) : RGB(30, 41, 59);
        COLORREF bd = (g_selectedTowerTypeToBuild == TRAP_SPIKE+i) ? TEXT_GOLD : BORDER_COLOR;
        DrawRoundedRect(memDC, sbX + 5, sbY + 185 + i*28, sbX + sbW - 5, sbY + 210 + i*28, bg, bd, 4);
        SetTextColor(memDC, TEXT_WHITE); TextOutA(memDC, sbX + 10, sbY + 190 + i*28, trapNames[i], lstrlenA(trapNames[i]));
        SetTextColor(memDC, TEXT_GOLD); char buf[32]; wsprintfA(buf, "%dg", trapCosts[i]);
        TextOutA(memDC, sbX + sbW - 35, sbY + 190 + i*28, buf, lstrlenA(buf));
    }

    // Scrolls
    DrawRoundedRect(memDC, sbX + 5, sbY + 270, sbX + 62, sbY + 295, RGB(220,38,38), BORDER_COLOR, 4); TextOutA(memDC, sbX+8, sbY+275, "Fire", 4);
    DrawRoundedRect(memDC, sbX + 65, sbY + 270, sbX + 122, sbY + 295, RGB(37,99,235), BORDER_COLOR, 4); TextOutA(memDC, sbX+68, sbY+275, "Blizz", 5);
    DrawRoundedRect(memDC, sbX + 125, sbY + 270, sbX + 185, sbY + 295, RGB(217,119,6), BORDER_COLOR, 4); TextOutA(memDC, sbX+128, sbY+275, "Magn", 4);

    // Button: Start Wave
    COLORREF btnBg = g_waveActive ? RGB(60, 70, 85) : RGB(16, 185, 129);
    DrawRoundedRect(memDC, sbX + 15, sbY + 300, sbX + sbW - 15, sbY + 335, btnBg, BORDER_COLOR, 6);
    SetTextColor(memDC, TEXT_WHITE);
    SelectObject(memDC, hFontHeader);
    char buf2[32]; wsprintfA(buf2, g_waveActive ? "IN PROGRESS" : "START WAVE %d", g_wave);
    TextOutA(memDC, sbX + 35, sbY + 310, buf2, (int)lstrlenA(buf2));

    // Button: Academy
    DrawRoundedRect(memDC, sbX + 15, sbY + 340, sbX + sbW - 15, sbY + 375, RGB(79, 70, 229), BORDER_COLOR, 6);
    TextOutA(memDC, sbX + 35, sbY + 350, "ACADEMY", 7);

    // Button: Reset Game
    DrawRoundedRect(memDC, sbX + 15, sbY + 380, sbX + sbW - 15, sbY + 400, RGB(225, 29, 72), BORDER_COLOR, 6);
    TextOutA(memDC, sbX + 35, sbY + 385, "RESET DEFENSE", 13);

    // Hero Section
    SetTextColor(memDC, TEXT_GOLD);
    TextOutA(memDC, sbX + 15, sbY + 410, "HERO: PALADIN", 13);
    
    HFONT hFontBody = CreateFontA(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SelectObject(memDC, hFontBody);
    
    char hBuf[32];
    if (g_hero.respawnTimer > 0) {
        SetTextColor(memDC, TEXT_RED);
        wsprintfA(hBuf, "HP: 0/100 | Respawn: %ds", (g_hero.respawnTimer/30) + 1);
    } else {
        SetTextColor(memDC, g_hero.shieldActive > 0 ? RGB(59, 130, 246) : RGB(34, 197, 94));
        wsprintfA(hBuf, "HP: %d/100 | %s", (int)g_hero.hp, g_hero.shieldActive > 0 ? "SHIELDED" : "Active");
    }
    TextOutA(memDC, sbX + 15, sbY + 430, hBuf, lstrlenA(hBuf));

    // Spell Buttons
    int btnW = 50;
    DrawRoundedRect(memDC, sbX + 15, sbY + 450, sbX + 15 + btnW, sbY + 480, g_hero.healCd > 0 ? RGB(100,80,20) : TEXT_GOLD, BORDER_COLOR, 4);
    DrawRoundedRect(memDC, sbX + 70, sbY + 450, sbX + 70 + btnW, sbY + 480, g_hero.shieldCd > 0 ? RGB(30,60,100) : RGB(59, 130, 246), BORDER_COLOR, 4);
    DrawRoundedRect(memDC, sbX + 125, sbY + 450, sbX + 125 + btnW, sbY + 480, g_hero.meteorCd > 0 ? RGB(100,20,20) : TEXT_RED, BORDER_COLOR, 4);
    
    SetTextColor(memDC, RGB(0,0,0));
    char sBuf[16];
    if (g_hero.healCd > 0) wsprintfA(sBuf, "H(%d)", (g_hero.healCd/30)+1); else wsprintfA(sBuf, "Heal(1)");
    TextOutA(memDC, sbX + 22, sbY + 460, sBuf, lstrlenA(sBuf));
    
    SetTextColor(memDC, TEXT_WHITE);
    if (g_hero.shieldCd > 0) wsprintfA(sBuf, "S(%d)", (g_hero.shieldCd/30)+1); else wsprintfA(sBuf, "Shld(2)");
    TextOutA(memDC, sbX + 76, sbY + 460, sBuf, lstrlenA(sBuf));
    
    if (g_hero.meteorCd > 0) wsprintfA(sBuf, "M(%d)", (g_hero.meteorCd/30)+1); else wsprintfA(sBuf, "Mtr(3)");
    TextOutA(memDC, sbX + 130, sbY + 460, sBuf, lstrlenA(sBuf));

    // Info Box
    DrawRoundedRect(memDC, sbX + 15, sbY + 490, sbX + sbW - 15, sbY + sbH - 10, BG_COLOR, BORDER_COLOR, 6);
    SetTextColor(memDC, TEXT_GOLD);
    TextOutA(memDC, sbX + 25, sbY + 500, "Phase 10: Traps", 15);
    SetTextColor(memDC, TEXT_MUTED);
    TextOutA(memDC, sbX + 25, sbY + 520, "Spikes, Oil, Barricades", 23);

    DeleteObject(hFontHeader);
    DeleteObject(hFontBody);

    if (g_showAcademy) {
        int mx = w/2 - 200, my = h/2 - 150;
        DrawRoundedRect(memDC, mx, my, mx + 400, my + 300, CARD_BG, TEXT_GOLD, 12);
        
        HFONT aH = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
        HFONT aB = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
        
        SelectObject(memDC, aH);
        SetTextColor(memDC, TEXT_GOLD);
        TextOutA(memDC, mx + 110, my + 20, "RESEARCH ACADEMY", 16);
        
        const char* tNames[] = {"Starting Gold (+50)", "Wall Durability (+10 HP)", "Hero Cooldowns (-10%)", "Tower Damage (+10%)"};
        int tLevels[] = {g_techStartingGold, g_techWallHp, g_techHeroCd, g_techTowerDmg};
        int tBaseCosts[] = {100, 150, 200, 250};
        
        SelectObject(memDC, aB);
        for (int i=0; i<4; i++) {
            DrawRoundedRect(memDC, mx + 20, my + 60 + i*45, mx + 380, my + 95 + i*45, RGB(20,20,20), BORDER_COLOR, 4);
            SetTextColor(memDC, TEXT_WHITE);
            TextOutA(memDC, mx + 30, my + 65 + i*45, tNames[i], lstrlenA(tNames[i]));
            char lvlBuf[32]; wsprintfA(lvlBuf, "Level %d/5", tLevels[i]);
            SetTextColor(memDC, TEXT_MUTED);
            TextOutA(memDC, mx + 30, my + 78 + i*45, lvlBuf, lstrlenA(lvlBuf));
            
            int cost = tBaseCosts[i] * (tLevels[i] + 1);
            COLORREF btnC = (g_gold >= cost && tLevels[i] < 5) ? RGB(34, 197, 94) : RGB(44, 50, 62);
            DrawRoundedRect(memDC, mx + 310, my + 65 + i*45, mx + 370, my + 90 + i*45, btnC, BORDER_COLOR, 4);
            SetTextColor(memDC, (g_gold >= cost && tLevels[i] < 5) ? RGB(0,0,0) : TEXT_MUTED);
            if (tLevels[i] < 5) {
                char cBuf[16]; wsprintfA(cBuf, "%dg", cost);
                TextOutA(memDC, mx + 325, my + 70 + i*45, cBuf, lstrlenA(cBuf));
            } else {
                TextOutA(memDC, mx + 325, my + 70 + i*45, "MAX", 3);
            }
        }
        
        char gBuf[32]; wsprintfA(gBuf, "Gold: %d", g_gold);
        SetTextColor(memDC, TEXT_GOLD);
        TextOutA(memDC, mx + 20, my + 260, gBuf, lstrlenA(gBuf));
        
        DrawRoundedRect(memDC, mx + 310, my + 250, mx + 380, my + 280, RGB(220,38,38), BORDER_COLOR, 4);
        SetTextColor(memDC, TEXT_WHITE);
        TextOutA(memDC, mx + 326, my + 257, "CLOSE", 5);
        
        DeleteObject(aH);
        DeleteObject(aB);
    }

    if (g_showHelp) {
        int hW = 600, hH = 450;
        int hX = (w - hW) / 2, hY = (h - hH) / 2;
        DrawRoundedRect(memDC, hX, hY, hX + hW, hY + hH, CARD_BG, TEXT_GOLD, 12);
        
        HFONT hTitle = CreateFontA(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
        HFONT hSec = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
        HFONT hTxt = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
        
        SelectObject(memDC, hTitle);
        SetTextColor(memDC, TEXT_GOLD);
        TextOutA(memDC, hX + 160, hY + 15, "COMMANDER'S FIELD GUIDE", 23);
        
        int cy = hY + 50;
        SelectObject(memDC, hSec); SetTextColor(memDC, RGB(34, 197, 94)); TextOutA(memDC, hX + 20, cy, "HOW TO PLAY", 11); cy += 20;
        SelectObject(memDC, hTxt); SetTextColor(memDC, TEXT_WHITE);
        TextOutA(memDC, hX + 20, cy, "Build towers on empty slots (+) using Gold. Earn Gold by defeating enemies.", 75); cy += 18;
        TextOutA(memDC, hX + 20, cy, "Protect your Base HP; if it reaches 0, you lose. Use Hero abilities to survive!", 81); cy += 25;
        
        SelectObject(memDC, hSec); SetTextColor(memDC, RGB(34, 197, 94)); TextOutA(memDC, hX + 20, cy, "TOWER SPEC SHEET", 16); cy += 20;
        SelectObject(memDC, hTxt); SetTextColor(memDC, TEXT_WHITE);
        TextOutA(memDC, hX + 20, cy, "- Archer (50g): Fast firing, medium range. Good vs Goblins.", 59); cy += 18;
        TextOutA(memDC, hX + 20, cy, "- Mage (100g): Medium fire rate. Deals Splash (AoE) damage. Excellent vs swarms.", 82); cy += 18;
        TextOutA(memDC, hX + 20, cy, "- Cannon (150g): Huge single-target damage. Best vs armored Orcs & Ogres.", 73); cy += 18;
        TextOutA(memDC, hX + 20, cy, "- Frost (120g): No damage, creates freezing aura that slows enemies by 60%.", 75); cy += 25;
        
        SelectObject(memDC, hSec); SetTextColor(memDC, RGB(34, 197, 94)); TextOutA(memDC, hX + 20, cy, "ENEMY BESTIARY", 14); cy += 20;
        SelectObject(memDC, hTxt); SetTextColor(memDC, TEXT_WHITE);
        TextOutA(memDC, hX + 20, cy, "- Goblin: Fast, low HP. Basic swarm unit.", 41); cy += 18;
        TextOutA(memDC, hX + 20, cy, "- Orc: Slow, armored (takes half dmg from Archers). Use Cannons/Magic!", 72); cy += 18;
        TextOutA(memDC, hX + 20, cy, "- Hound: Very fast, low HP. Slips past slow towers easily.", 58); cy += 18;
        TextOutA(memDC, hX + 20, cy, "- Gargoyle: Flying unit. Bypasses ground pathing completely.", 60); cy += 18;
        TextOutA(memDC, hX + 20, cy, "- Ogre Boss: Massive HP, slow. Deals 5 dmg to base. Big gold bounty.", 68); cy += 25;
        
        SelectObject(memDC, hSec); SetTextColor(memDC, RGB(34, 197, 94)); TextOutA(memDC, hX + 20, cy, "TACTICS", 7); cy += 20;
        SelectObject(memDC, hTxt); SetTextColor(memDC, TEXT_WHITE);
        TextOutA(memDC, hX + 20, cy, "- Use Hero Shield Wall (2) right before enemies hit your castle to block damage.", 82); cy += 18;
        TextOutA(memDC, hX + 20, cy, "- Place Frost towers near Cannons so enemies stay in range longer.", 65);
        
        DrawRoundedRect(memDC, hX + 225, hY + hH - 45, hX + 375, hY + hH - 15, RGB(16, 185, 129), BORDER_COLOR, 6);
        SelectObject(memDC, hSec); SetTextColor(memDC, RGB(0,0,0));
        TextOutA(memDC, hX + 245, hY + hH - 37, "UNDERSTOOD", 10);
        
        DeleteObject(hTitle); DeleteObject(hSec); DeleteObject(hTxt);
    }

    // Copy Backbuffer to Window DC
    BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);

    // Cleanup
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
            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (g_enemies[e].active) {
                    float dx = g_enemies[e].x - g_hero.x;
                    float dy = g_enemies[e].y - g_hero.y;
                    if (custom_sqrtf(dx*dx + dy*dy) <= 150.0f) {
                        g_enemies[e].hp -= 100;
                        if (g_enemies[e].hp <= 0) {
                            g_enemies[e].active = FALSE;
                            int reward = 15;
                            if (g_enemies[e].type == ENEMY_OGRE) {
                                reward = 100;
                                if (g_gameMode == 2) {
                                    g_bossesKilled++;
                                    if (g_bossesKilled > g_hsBoss) { g_hsBoss = g_bossesKilled; SaveGame(); }
                                }
                            }
                            else if (g_enemies[e].type == ENEMY_ORC) reward = 20;
                            else if (g_enemies[e].type == ENEMY_HOUND) reward = 10;
                            g_gold += reward;
                            char rBuf[16]; wsprintfA(rBuf, "+%dg", reward);
                            AddFloatingText(g_enemies[e].x, g_enemies[e].y - 10, rBuf, TEXT_GOLD);
                        }
                    }
                }
            }
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
            int hW = 600, hH = 450;
            int hX = (w - hW) / 2, hY = (h - hH) / 2;
            if (x >= hX + 225 && x <= hX + 375 && y >= hY + hH - 45 && y <= hY + hH - 15) {
                g_showHelp = FALSE;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        
        if (g_showAcademy) {
            int mx = w/2 - 200, my = h/2 - 150;
            if (x >= mx + 310 && x <= mx + 380 && y >= my + 250 && y <= my + 280) {
                g_showAcademy = FALSE;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            int tBaseCosts[] = {100, 150, 200, 250};
            int* tLevels[] = {&g_techStartingGold, &g_techWallHp, &g_techHeroCd, &g_techTowerDmg};
            for (int i=0; i<4; i++) {
                if (x >= mx + 310 && x <= mx + 370 && y >= my + 65 + i*45 && y <= my + 90 + i*45) {
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

        // Help Button
        if (x >= w - 410 && x <= w - 340 && y >= 20 && y <= 45) {
            g_showHelp = TRUE;
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        int sbX = w - 200, sbY = 70, sbW = 190;

        // Map Selector Clicks
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
        
        sbY += 25;
        
        // Mode Selection
        if (!g_waveActive) {
            if (x >= sbX + 5 && x <= sbX + 62 && y >= sbY + 30 && y <= sbY + 55) { g_gameMode = 0; InitGameState(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
            if (x >= sbX + 65 && x <= sbX + 122 && y >= sbY + 30 && y <= sbY + 55) { g_gameMode = 1; InitGameState(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
            if (x >= sbX + 125 && x <= sbX + 185 && y >= sbY + 30 && y <= sbY + 55) { g_gameMode = 2; InitGameState(); InvalidateRect(hwnd, NULL, FALSE); return 0; }
        }
        sbY += 65;

        // Check button clicks
        // Tower Selection Shop
        for (int i=0; i<4; i++) {
            if (x >= sbX + 5 && x <= sbX + sbW - 5 && y >= sbY + 35 + i*38 && y <= sbY + 70 + i*38) {
                g_selectedTowerTypeToBuild = i + 1;
                Beep(400, 20);
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
        }
        
        // Shop Info traps
        for (int i=0; i<3; i++) {
            if (x >= sbX + 5 && x <= sbX + sbW - 5 && y >= sbY + 185 + i*28 && y <= sbY + 210 + i*28) {
                g_selectedTowerTypeToBuild = TRAP_SPIKE + i;
                Beep(400, 20); InvalidateRect(hwnd, NULL, FALSE); return 0;
            }
        }
        
        // Scrolls
        if (x >= sbX + 5 && x <= sbX + 62 && y >= sbY + 270 && y <= sbY + 295) {
            if (g_gold >= 100) { g_gold -= 100; AddFloatingText(400, 300, "FIRESTORM!", TEXT_RED); Beep(100, 300);
                for(int e=0; e<MAX_ENEMIES; e++) if(g_enemies[e].active) { g_enemies[e].hp -= 150; if(g_enemies[e].hp<=0) { g_enemies[e].active=FALSE; g_gold+=15; } }
            }
            InvalidateRect(hwnd, NULL, FALSE); return 0;
        }
        if (x >= sbX + 65 && x <= sbX + 122 && y >= sbY + 270 && y <= sbY + 295) {
            if (g_gold >= 80) { g_gold -= 80; g_blizzTimer = 300; AddFloatingText(400, 300, "BLIZZARD!", RGB(59, 130, 246)); Beep(600, 300); }
            InvalidateRect(hwnd, NULL, FALSE); return 0;
        }
        if (x >= sbX + 125 && x <= sbX + 185 && y >= sbY + 270 && y <= sbY + 295) {
            if (g_gold >= 60) {
                g_gold -= 60;
                int b = 0; for(int e=0; e<MAX_ENEMIES; e++) if(g_enemies[e].active) b += 15;
                g_gold += b; char buf[32]; wsprintfA(buf, "MAGNET: +%dg", b);
                AddFloatingText(400, 300, buf, TEXT_GOLD); Beep(800, 200);
            }
            InvalidateRect(hwnd, NULL, FALSE); return 0;
        }

        // Start Wave
        if (x >= sbX + 15 && x <= sbX + sbW - 15 && y >= sbY + 300 && y <= sbY + 335) {
            if (!g_waveActive && !g_gameOver) {
                g_waveActive = TRUE;
                
                if (g_gameMode == 2) {
                    g_spawnQueueCount = g_wave * 2;
                    if (g_spawnQueueCount > MAX_SPAWN_QUEUE) g_spawnQueueCount = MAX_SPAWN_QUEUE;
                    g_spawnQueueHead = 0;
                    for (int i = 0; i < g_spawnQueueCount; i++) g_spawnQueue[i] = ENEMY_OGRE;
                } else {
                    g_spawnQueueCount = 5 + g_wave * 3;
                    if (g_spawnQueueCount > MAX_SPAWN_QUEUE) g_spawnQueueCount = MAX_SPAWN_QUEUE;
                    g_spawnQueueHead = 0;
                    for (int i = 0; i < g_spawnQueueCount; i++) {
                        if (g_wave % 5 == 0 && i == 0) g_spawnQueue[i] = ENEMY_OGRE;
                        else {
                            int r = rand() % 100;
                            if (g_wave >= 2 && r < 20) g_spawnQueue[i] = ENEMY_HOUND;
                            else if (g_wave >= 3 && r < 40) g_spawnQueue[i] = ENEMY_ORC;
                            else if (g_wave >= 4 && r < 60) g_spawnQueue[i] = ENEMY_GARGOYLE;
                            else g_spawnQueue[i] = ENEMY_GOBLIN;
                        }
                    }
                }
                g_spawnTimer = 0; Beep(600, 40);
            }
        }
        // Academy
        else if (x >= sbX + 15 && x <= sbX + sbW - 15 && y >= sbY + 340 && y <= sbY + 375) {
            g_showAcademy = TRUE; InvalidateRect(hwnd, NULL, FALSE); return 0;
        }
        // Reset Defense
        else if (x >= sbX + 15 && x <= sbX + sbW - 15 && y >= sbY + 380 && y <= sbY + 400) {
            InitGameState(); Beep(300, 60);
        }
        // Spells
        else if (x >= sbX + 15 && x <= sbX + 65 && y >= sbY + 450 && y <= sbY + 480) SendMessage(hwnd, WM_KEYDOWN, '1', 0);
        else if (x >= sbX + 70 && x <= sbX + 120 && y >= sbY + 450 && y <= sbY + 480) SendMessage(hwnd, WM_KEYDOWN, '2', 0);
        else if (x >= sbX + 125 && x <= sbX + 175 && y >= sbY + 450 && y <= sbY + 480) SendMessage(hwnd, WM_KEYDOWN, '3', 0);

        // Check slot clicks
        else if (!g_gameOver) {
            BOOL clickedSlot = FALSE;
            g_selectedSlot = -1;
            for (int i = 0; i < g_slotCount; i++) {
                int dx = x - g_slots[i].x;
                int dy = y - g_slots[i].y;
                if (dx * dx + dy * dy <= 22 * 22) {
                    g_selectedSlot = i;

                    if (!g_slots[i].occupied) {
                        int cost = 50;
                        int rng = 130;
                        int dmg = 12;
                        int cd = 18; // ~0.6s
                        if (g_selectedTowerTypeToBuild == TOWER_MAGE) { cost = 100; rng = 110; dmg = 15; cd = 30; }
                        else if (g_selectedTowerTypeToBuild == TOWER_CANNON) { cost = 150; rng = 150; dmg = 40; cd = 60; }
                        else if (g_selectedTowerTypeToBuild == TOWER_FROST) { cost = 120; rng = 100; dmg = 0; cd = 15; }
                        
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
                            g_slots[i].splash = (g_selectedTowerTypeToBuild == TOWER_MAGE) ? (int)(50 * dmgMod) : 0;
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
                            
                            int upCost = (int)(baseCost * (g_slots[i].level * 1.5f));
                            if (g_gold >= upCost) {
                                g_gold -= upCost;
                                g_slots[i].level++;
                                g_slots[i].damage = (int)(g_slots[i].damage * 1.4f);
                                g_slots[i].range = (int)(g_slots[i].range * 1.15f);
                                g_slots[i].maxCooldown = (int)(g_slots[i].maxCooldown * 0.85f);
                                if (g_slots[i].maxCooldown < 10) g_slots[i].maxCooldown = 10;
                                if (g_slots[i].splash > 0) g_slots[i].splash = (int)(g_slots[i].splash * 1.25f);
                                
                                char buf[32];
                                wsprintfA(buf, "LVL %d!", g_slots[i].level);
                                AddFloatingText((float)g_slots[i].x, (float)(g_slots[i].y - 20), buf, TEXT_GOLD);
                                Beep(800, 40);
                            } else {
                                char buf[32];
                                wsprintfA(buf, "NEED %dg!", upCost);
                                AddFloatingText((float)g_slots[i].x, (float)(g_slots[i].y - 20), buf, RGB(239, 68, 68));
                                Beep(200, 100);
                            }
                        } else {
                            AddFloatingText((float)g_slots[i].x, (float)(g_slots[i].y - 20), "MAX LEVEL!", TEXT_MUTED);
                            Beep(500, 30);
                        }
                    }
                    clickedSlot = TRUE;
                    break;
                }
            }

            if (!clickedSlot && g_hero.respawnTimer <= 0) {
                if (g_selectedTowerTypeToBuild >= TRAP_SPIKE && g_selectedTowerTypeToBuild <= TRAP_BARRICADE && x < sbX) {
                    int c = (g_selectedTowerTypeToBuild == TRAP_SPIKE) ? 30 : ((g_selectedTowerTypeToBuild == TRAP_OIL) ? 40 : 50);
                    if (g_gold >= c) {
                        for (int tr = 0; tr < MAX_TRAPS; tr++) {
                            if (!g_traps[tr].active) {
                                g_gold -= c;
                                g_traps[tr].active = TRUE;
                                g_traps[tr].x = (float)x; g_traps[tr].y = (float)y;
                                g_traps[tr].type = g_selectedTowerTypeToBuild;
                                g_traps[tr].charges = (g_selectedTowerTypeToBuild == TRAP_SPIKE) ? 3 : -1;
                                g_traps[tr].hp = (g_selectedTowerTypeToBuild == TRAP_BARRICADE) ? 200.0f : 0;
                                g_traps[tr].radius = (g_selectedTowerTypeToBuild == TRAP_OIL) ? 40 : 25;
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
    const char CLASS_NAME[] = "KFortressWindowClass";

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME, "KFortress - Fantasy Tower Defense & Siege Defense",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
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
