#include <windows.h>
#include <stdbool.h>

#define STATE_GARAGE 0
#define STATE_BATTLE 1
#define STATE_POST_BATTLE 2
#define STATE_HELP 3

static unsigned int g_seed = 0;
void my_srand(unsigned int seed) { g_seed = seed; }
int my_rand() { g_seed = g_seed * 214013 + 2531011; return (g_seed >> 16) & 0x7FFF; }

static float FastSin(float x) {
    while (x > 3.14159f) x -= 6.28318f;
    while (x < -3.14159f) x += 6.28318f;
    float x2 = x * x;
    return x * (1.0f - x2 * (0.16666667f - x2 * 0.00833333f));
}
static float FastCos(float x) {
    return FastSin(x + 1.5707963f);
}

int gameState = STATE_GARAGE;
bool playerVictory = false;

typedef struct {
    int maxHp;
    int hp;
    int atk;
    int def;
    int maxHeat;
    int heat;
} MechStats;

typedef struct {
    const char* name;
    int atk;
    int heatGen;
} Weapon;

typedef struct {
    const char* name;
    int def;
    int maxHp;
} Armor;

typedef struct {
    const char* name;
    int cooling;
    int maxHeat;
} HeatSink;

Weapon weapons[3] = {
    {"Basic Laser", 15, 30},
    {"Heavy Cannon", 25, 50},
    {"Twin Blasters", 20, 40}
};

Armor armors[3] = {
    {"Standard", 5, 100},
    {"Heavy", 10, 120},
    {"Light Scout", 2, 80}
};

HeatSink sinks[3] = {
    {"Basic", 20, 100},
    {"Advanced", 30, 150},
    {"Burst", 40, 80}
};

typedef struct {
    const char* name;
    float evadeBonus;
    int shieldDmgReduction;
} Special;

Special specials[3] = {
    {"None", 0.0f, 0},
    {"Jump Jets", 0.2f, 0},
    {"Energy Shield", 0.0f, 5}
};

int equipWpn = 0;
int equipArm = 0;
int equipSink = 0;
int equipSpec = 0;
int playerHeatGen = 30;
int playerCooling = 20;

int credits = 100;
int salvage = 0;
int battleCount = 1;
int playerLevel = 1;
int playerXp = 0;

MechStats playerStats = { 100, 100, 15, 5, 100, 0 };
MechStats enemyStats = { 80, 80, 12, 3, 100, 0 };

bool isDefending = false;
bool enemyIsDefending = false;
int playerLimbDamage[5] = {0, 0, 0, 0, 0};

int animPlayerOffset = 0;
int animEnemyOffset = 0;
int animPlayerDmg = 0;
int animEnemyDmg = 0;

// Screen shake & visual FX variables
float g_screenShake = 0.0f;
float g_shakePhase = 0.0f;

// 4-Layer Particle Structure
typedef struct {
    float x, y;
    float vx, vy;
    float grav;
    float life;
    float decay;
    int size;
    COLORREF color;
    int type; // 0=Spark, 1=Smoke/Plasma, 2=Debris, 3=Star
} GdiParticle;

#define MAX_PARTICLES 120
GdiParticle g_particles[MAX_PARTICLES];
int g_particleCount = 0;

// Shockwaves
typedef struct {
    float x, y;
    float r, maxR;
    float speed;
    float alpha;
    COLORREF color;
    bool active;
} GdiShockwave;

#define MAX_SHOCKWAVES 8
GdiShockwave g_shockwaves[MAX_SHOCKWAVES];

// Projectile
typedef struct {
    float x, y;
    float startX, startY;
    float targetX, targetY;
    float progress;
    float speed;
    int weaponType;
    bool fromPlayer;
    bool active;
} GdiProjectile;

#define MAX_PROJECTILES 6
GdiProjectile g_projectiles[MAX_PROJECTILES];

void AddShockwave(float x, float y, float maxR, COLORREF color, float speed) {
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (!g_shockwaves[i].active) {
            g_shockwaves[i].x = x;
            g_shockwaves[i].y = y;
            g_shockwaves[i].r = 4.0f;
            g_shockwaves[i].maxR = maxR;
            g_shockwaves[i].speed = speed;
            g_shockwaves[i].alpha = 1.0f;
            g_shockwaves[i].color = color;
            g_shockwaves[i].active = true;
            break;
        }
    }
}

void TriggerScreenShake(float amt) {
    g_screenShake = amt;
}

void SpawnExplosion(float x, float y, int count, bool isEnemy) {
    TriggerScreenShake(10.0f);
    COLORREF coreColor = isEnemy ? RGB(255, 255, 80) : RGB(80, 255, 255);
    COLORREF smokeColor = isEnemy ? RGB(255, 60, 20) : RGB(0, 255, 120);

    AddShockwave(x, y, 50.0f, isEnemy ? RGB(255, 80, 40) : RGB(0, 255, 255), 4.0f);
    AddShockwave(x, y, 75.0f, RGB(255, 255, 255), 6.0f);

    for (int i = 0; i < count && g_particleCount < MAX_PARTICLES; i++) {
        float angle = ((float)(my_rand() % 360)) * (3.14159f / 180.0f);
        float spd = 2.0f + (float)(my_rand() % 50) / 10.0f;
        int ptype = i % 3; // 0=Spark, 1=Smoke, 2=Debris

        g_particles[g_particleCount].x = x;
        g_particles[g_particleCount].y = y;
        g_particles[g_particleCount].vx = FastCos(angle) * spd;
        g_particles[g_particleCount].vy = FastSin(angle) * spd;
        g_particles[g_particleCount].life = 1.0f;
        g_particles[g_particleCount].type = ptype;

        if (ptype == 0) { // Spark
            g_particles[g_particleCount].decay = 0.03f + (float)(my_rand() % 30) / 1000.0f;
            g_particles[g_particleCount].grav = 0.0f;
            g_particles[g_particleCount].size = 2 + (my_rand() % 3);
            g_particles[g_particleCount].color = coreColor;
        } else if (ptype == 1) { // Smoke
            g_particles[g_particleCount].decay = 0.02f + (float)(my_rand() % 20) / 1000.0f;
            g_particles[g_particleCount].grav = -0.1f;
            g_particles[g_particleCount].size = 5 + (my_rand() % 5);
            g_particles[g_particleCount].color = smokeColor;
        } else { // Debris
            g_particles[g_particleCount].decay = 0.015f + (float)(my_rand() % 15) / 1000.0f;
            g_particles[g_particleCount].grav = 0.25f;
            g_particles[g_particleCount].size = 3 + (my_rand() % 4);
            g_particles[g_particleCount].color = RGB(120, 140, 120);
        }
        g_particleCount++;
    }
}

void SpawnVictoryStars(float x, float y) {
    for (int i = 0; i < 30 && g_particleCount < MAX_PARTICLES; i++) {
        float angle = ((float)(my_rand() % 360)) * (3.14159f / 180.0f);
        float spd = 2.0f + (float)(my_rand() % 60) / 10.0f;
        g_particles[g_particleCount].x = x;
        g_particles[g_particleCount].y = y;
        g_particles[g_particleCount].vx = FastCos(angle) * spd;
        g_particles[g_particleCount].vy = FastSin(angle) * spd;
        g_particles[g_particleCount].grav = 0.05f;
        g_particles[g_particleCount].life = 1.0f;
        g_particles[g_particleCount].decay = 0.012f + (float)(my_rand() % 15) / 1000.0f;
        g_particles[g_particleCount].size = 3 + (my_rand() % 3);
        g_particles[g_particleCount].color = (i % 2 == 0) ? RGB(255, 255, 0) : RGB(0, 255, 255);
        g_particles[g_particleCount].type = 3;
        g_particleCount++;
    }
}

void FireProjectile(bool fromPlayer, int weaponType) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!g_projectiles[i].active) {
            g_projectiles[i].startX = fromPlayer ? 150.0f : 440.0f;
            g_projectiles[i].startY = 145.0f;
            g_projectiles[i].targetX = fromPlayer ? 440.0f : 150.0f;
            g_projectiles[i].targetY = 145.0f;
            g_projectiles[i].x = g_projectiles[i].startX;
            g_projectiles[i].y = g_projectiles[i].startY;
            g_projectiles[i].progress = 0.0f;
            g_projectiles[i].speed = fromPlayer ? 0.12f : 0.10f;
            g_projectiles[i].weaponType = weaponType;
            g_projectiles[i].fromPlayer = fromPlayer;
            g_projectiles[i].active = true;
            break;
        }
    }
}

int pingpong(int t, int max) {
    int cycle = t % (max * 2);
    if (cycle > max) return (max * 2) - cycle;
    return cycle;
}

int currentTargetLimb = 1; // 0=Head, 1=Torso, 2=LArm, 3=RArm, 4=Legs
const char* limbNames[] = { "Head", "Torso", "L.Arm", "R.Arm", "Legs" };
float limbHitChance[] = { 0.2f, 0.8f, 0.6f, 0.6f, 0.5f };
float limbDmgMult[] = { 3.0f, 1.0f, 1.5f, 1.5f, 1.2f };

char battleLogs[15][128];
int logCount = 0;
char garageInfo[128] = "Welcome, Pilot. Customize and deploy.";

void addLog(const char* msg) {
    if (logCount < 10) {
        lstrcpyA(battleLogs[logCount], msg);
        logCount++;
    } else {
        for (int i = 0; i < 9; i++) {
            lstrcpyA(battleLogs[i], battleLogs[i + 1]);
        }
        lstrcpyA(battleLogs[9], msg);
    }
}

void clearLogs() {
    logCount = 0;
}

void EnemyTurn() {
    if (enemyStats.heat + 30 > enemyStats.maxHeat) {
        enemyIsDefending = true;
        addLog("Enemy vents heat! (Defending)");
        AddShockwave(440.0f, 145.0f, 40.0f, RGB(255, 160, 0), 3.0f);
        return;
    }

    animEnemyOffset = 8;
    enemyIsDefending = false;
    enemyStats.heat += 30;

    int target = my_rand() % 5;
    if ((my_rand() % 100) < 50) {
        int maxDmg = 0;
        for (int i = 0; i < 5; i++) {
            if (playerLimbDamage[i] > maxDmg) {
                maxDmg = playerLimbDamage[i];
                target = i;
            }
        }
    }

    int hitRoll = my_rand() % 100;
    FireProjectile(false, 0);
    
    float effectiveChance = limbHitChance[target] - specials[equipSpec].evadeBonus - ((playerLevel - 1) * 0.05f);
    if (hitRoll > (int)(effectiveChance * 100.0f)) {
        char buf[128];
        if (specials[equipSpec].evadeBonus > 0 || playerLevel > 1) {
            wsprintfA(buf, "Enemy targets %s... Evaded!", limbNames[target]);
        } else {
            wsprintfA(buf, "Enemy targets %s... Missed!", limbNames[target]);
        }
        addLog(buf);
        Beep(300, 100);
    } else {
        int effectiveDef = isDefending ? (playerStats.def * 2) : playerStats.def;
        int dmg = (enemyStats.atk - effectiveDef) + (my_rand() % 4);
        if (dmg < 1) dmg = 1;
        dmg = (int)(dmg * limbDmgMult[target]);
        
        if (specials[equipSpec].shieldDmgReduction > 0) {
            int absorbed = dmg < specials[equipSpec].shieldDmgReduction ? dmg : specials[equipSpec].shieldDmgReduction;
            dmg -= absorbed;
            if (absorbed > 0) {
                char abuf[128];
                wsprintfA(abuf, "Shield absorbed %d damage!", absorbed);
                addLog(abuf);
                AddShockwave(150.0f, 145.0f, 35.0f, RGB(0, 255, 255), 4.0f);
            }
        }

        playerStats.hp -= dmg;
        playerLimbDamage[target] += dmg;
        animPlayerDmg = 10;
        SpawnExplosion(150.0f, 145.0f, 18, false);
        
        char buf[128];
        wsprintfA(buf, "Enemy hits %s! Took %d dmg.", limbNames[target], dmg);
        addLog(buf);
        Beep(900, 50); Beep(700, 50); Beep(500, 100);
        Beep(150, 100); Beep(100, 150);
    }

    if (playerStats.hp <= 0) {
        playerStats.hp = 0;
        addLog("CRITICAL DAMAGE. MECH DESTROYED. CAMPAIGN FAILED.");
        SpawnExplosion(150.0f, 145.0f, 35, false);
        Beep(300, 300); Beep(200, 300); Beep(100, 500);
        battleCount = 1;
        credits = 100;
        salvage = 0;
        playerLevel = 1;
        playerXp = 0;
        gameState = STATE_POST_BATTLE;
        playerVictory = false;
    }
}

void ApplyCooling() {
    playerStats.heat -= playerCooling;
    if (playerStats.heat < 0) playerStats.heat = 0;
    enemyStats.heat -= 20;
    if (enemyStats.heat < 0) enemyStats.heat = 0;
}

void ActionAttack() {
    playerStats.heat += playerHeatGen;
    if (playerStats.heat > playerStats.maxHeat) {
        playerStats.hp -= 15;
        addLog("WARNING: OVERHEAT! Took 15 system dmg.");
        SpawnExplosion(150.0f, 145.0f, 16, false);
        Beep(2000, 100); Beep(2000, 100); Beep(2000, 300);
        if (playerStats.hp <= 0) {
            playerStats.hp = 0;
            addLog("CRITICAL DAMAGE. MECH DESTROYED. CAMPAIGN FAILED.");
            Beep(300, 300); Beep(200, 300); Beep(100, 500);
            battleCount = 1;
            credits = 100;
            salvage = 0;
            playerLevel = 1;
            playerXp = 0;
            gameState = STATE_POST_BATTLE;
            playerVictory = false;
            return;
        }
    }

    animPlayerOffset = 8;
    int target = currentTargetLimb;
    int hitRoll = my_rand() % 100;
    FireProjectile(true, equipWpn);
    
    float hitChance = limbHitChance[target] + ((playerLevel - 1) * 0.05f);
    if (hitChance > 1.0f) hitChance = 1.0f;

    if (hitRoll > (int)(hitChance * 100.0f)) {
        char buf[128];
        wsprintfA(buf, "You target %s... Missed!", limbNames[target]);
        addLog(buf);
        Beep(300, 100);
    } else {
        int effectiveDef = enemyIsDefending ? (enemyStats.def * 2) : enemyStats.def;
        int dmg = (playerStats.atk - effectiveDef) + (my_rand() % 5);
        if (dmg < 1) dmg = 1;
        dmg = (int)(dmg * limbDmgMult[target]);
        enemyStats.hp -= dmg;
        animEnemyDmg = 10;
        SpawnExplosion(440.0f, 145.0f, 20, true);

        char buf[128];
        wsprintfA(buf, "You hit %s! Dealt %d dmg.", limbNames[target], dmg);
        addLog(buf);
        Beep(1000, 50); Beep(800, 50); Beep(600, 100);
        Beep(150, 100); Beep(100, 150);
    }

    if (enemyStats.hp <= 0) {
        enemyStats.hp = 0;
        SpawnVictoryStars(440.0f, 145.0f);
        int reward = 50 + (battleCount * 10);
        int parts = (my_rand() % 3) + 1;
        credits += reward;
        salvage += parts;
        char buf[128];
        wsprintfA(buf, "VICTORY! Earned %d CR & %d Parts.", reward, parts);
        addLog(buf);
        Beep(400, 100); Beep(500, 100); Beep(600, 100); Beep(800, 300);

        int xpGain = 40 + (battleCount * 20);
        playerXp += xpGain;
        wsprintfA(buf, "Gained %d XP.", xpGain);
        addLog(buf);
        
        if (playerXp >= playerLevel * 100) {
            playerXp -= playerLevel * 100;
            playerLevel++;
            wsprintfA(buf, "LEVEL UP! Pilot is now Level %d.", playerLevel);
            addLog(buf);
            Beep(500, 100); Beep(600, 100); Beep(700, 100); Beep(800, 300);
        }

        battleCount++;
        gameState = STATE_POST_BATTLE;
        playerVictory = true;
        return;
    }

    EnemyTurn();
    if (gameState == STATE_BATTLE) {
        ApplyCooling();
    }
}

void ActionDefend() {
    isDefending = true;
    addLog("You brace for impact (Defending).");
    AddShockwave(150.0f, 145.0f, 40.0f, RGB(0, 255, 255), 3.0f);
    EnemyTurn();
    isDefending = false;
    if (gameState == STATE_BATTLE) {
        ApplyCooling();
    }
}

void StartBattle() {
    enemyStats.maxHp = 80 + (battleCount * 10);
    enemyStats.hp = enemyStats.maxHp;
    enemyStats.atk = 12 + (battleCount * 2);
    enemyStats.def = 3 + battleCount;
    enemyStats.maxHeat = 100 + (battleCount * 5);
    enemyStats.heat = 0;
    playerStats.heat = 0;
    isDefending = false;
    enemyIsDefending = false;
    for (int i=0; i<5; i++) playerLimbDamage[i] = 0;
    clearLogs();
    addLog("Enemy mech detected! Engaging...");
    Beep(400, 100); Beep(600, 100); Beep(800, 200);
    gameState = STATE_BATTLE;
}

void ReturnToGarage() {
    if (playerStats.hp <= 0) {
        playerStats.hp = playerStats.maxHp;
        lstrcpyA(garageInfo, "Mech rebuilt. Campaign restarted.");
    } else {
        lstrcpyA(garageInfo, "Returned to garage. Repairs needed.");
    }
    gameState = STATE_GARAGE;
}

// Button areas
RECT rectRepair  = {  40, 290, 260, 325 };
RECT rectDeploy  = { 300, 290, 520, 325 };
RECT rectUseSal  = {  40, 335, 260, 370 };
RECT rectSellSal = { 300, 335, 520, 370 };

RECT rectTarget = { 40, 410, 180, 445 };
RECT rectAttack = { 200, 410, 340, 445 };
RECT rectDefend = { 360, 410, 500, 445 };
RECT rectReturn = { 180, 410, 400, 445 };

RECT rectHelp = { 180, 380, 380, 415 };
RECT rectReturnHelp = { 180, 430, 400, 465 };

RECT rectWpn = { 20, 230, 140, 265 };
RECT rectArm = { 150, 230, 270, 265 };
RECT rectSink = { 280, 230, 400, 265 };
RECT rectSpec = { 410, 230, 530, 265 };

bool PtInRectLocal(const RECT* r, int x, int y) {
    return (x >= r->left && x <= r->right && y >= r->top && y <= r->bottom);
}

void DrawButton(HDC hdc, const RECT* r, const char* text) {
    HBRUSH bgBrush = CreateSolidBrush(RGB(5, 25, 10));
    HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 100));
    HGDIOBJ oldBrush = SelectObject(hdc, bgBrush);
    HGDIOBJ oldPen = SelectObject(hdc, borderPen);
    
    RoundRect(hdc, r->left, r->top, r->right, r->bottom, 4, 4);
    
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 255, 130));
    DrawTextA(hdc, text, -1, (RECT*)r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(bgBrush);
    DeleteObject(borderPen);
}

/* ==================== PROCEDURAL GDI MECH RENDERERS ==================== */
void DrawPlayerMechGDI(HDC hdc, int cx, int cy, float scale, bool isDead) {
    if (isDead) {
        // Render smoking wrecked chassis
        HPEN wreckPen = CreatePen(PS_SOLID, 2, RGB(80, 100, 80));
        HBRUSH wreckBrush = CreateSolidBrush(RGB(20, 30, 20));
        HGDIOBJ oldP = SelectObject(hdc, wreckPen);
        HGDIOBJ oldB = SelectObject(hdc, wreckBrush);
        Rectangle(hdc, cx - 20, cy - 10, cx + 20, cy + 20);
        SelectObject(hdc, oldP);
        SelectObject(hdc, oldB);
        DeleteObject(wreckPen);
        DeleteObject(wreckBrush);
        return;
    }

    bool isHeavy = (equipArm == 1);
    bool isLight = (equipArm == 2);
    bool hasJets = (equipSpec == 1);
    bool hasShield = (equipSpec == 2);

    COLORREF primaryGreen = isHeavy ? RGB(0, 255, 120) : (isLight ? RGB(80, 255, 180) : RGB(0, 255, 80));
    COLORREF armorFill = isHeavy ? RGB(10, 45, 20) : (isLight ? RGB(8, 35, 15) : RGB(15, 55, 25));
    COLORREF visorColor = (playerStats.heat > 75) ? RGB(255, 50, 0) : ((playerStats.heat > 40) ? RGB(255, 180, 0) : RGB(0, 255, 255));

    HPEN pPen = CreatePen(PS_SOLID, (int)(2 * scale), primaryGreen);
    HBRUSH pBrush = CreateSolidBrush(armorFill);
    HBRUSH pPauldronBrush = CreateSolidBrush(RGB(0, 140, 60));
    HGDIOBJ oldPen = SelectObject(hdc, pPen);
    HGDIOBJ oldBrush = SelectObject(hdc, pBrush);

    // Jump Jet Thrusters
    if (hasJets) {
        HPEN jetPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 200));
        HBRUSH jetBrush = CreateSolidBrush(RGB(0, 100, 60));
        HGDIOBJ oP = SelectObject(hdc, jetPen);
        HGDIOBJ oB = SelectObject(hdc, jetBrush);
        Rectangle(hdc, cx - (int)(26 * scale), cy - (int)(32 * scale), cx - (int)(18 * scale), cy - (int)(18 * scale));
        Rectangle(hdc, cx + (int)(18 * scale), cy - (int)(32 * scale), cx + (int)(26 * scale), cy - (int)(18 * scale));
        // Exhaust flame
        HPEN flamePen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
        HBRUSH flameBrush = CreateSolidBrush(RGB(0, 220, 255));
        SelectObject(hdc, flamePen);
        SelectObject(hdc, flameBrush);
        POINT f1[3] = { {cx - (int)(24 * scale), cy - (int)(32 * scale)}, {cx - (int)(20 * scale), cy - (int)(32 * scale)}, {cx - (int)(22 * scale), cy - (int)(40 * scale)} };
        Polygon(hdc, f1, 3);
        POINT f2[3] = { {cx + (int)(20 * scale), cy - (int)(32 * scale)}, {cx + (int)(24 * scale), cy - (int)(32 * scale)}, {cx + (int)(22 * scale), cy - (int)(40 * scale)} };
        Polygon(hdc, f2, 3);
        SelectObject(hdc, oP);
        SelectObject(hdc, oB);
        DeleteObject(jetPen);
        DeleteObject(jetBrush);
        DeleteObject(flamePen);
        DeleteObject(flameBrush);
    }

    // Energy Shield Barrier
    if (hasShield) {
        HPEN shldPen = CreatePen(PS_DOT, 1, RGB(0, 255, 255));
        HGDIOBJ oP = SelectObject(hdc, shldPen);
        HGDIOBJ oB = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        POINT hex[6] = {
            {cx, cy - (int)(46 * scale)},
            {cx + (int)(38 * scale), cy - (int)(22 * scale)},
            {cx + (int)(38 * scale), cy + (int)(26 * scale)},
            {cx, cy + (int)(48 * scale)},
            {cx - (int)(38 * scale), cy + (int)(26 * scale)},
            {cx - (int)(38 * scale), cy - (int)(22 * scale)}
        };
        Polygon(hdc, hex, 6);
        SelectObject(hdc, oP);
        SelectObject(hdc, oB);
        DeleteObject(shldPen);
    }

    // Legs & Greaves
    SelectObject(hdc, pPen);
    SelectObject(hdc, pBrush);
    POINT legL[4] = {
        {cx - (int)(12 * scale), cy + (int)(15 * scale)},
        {cx - (int)(20 * scale), cy + (int)(35 * scale)},
        {cx - (int)(6 * scale), cy + (int)(44 * scale)},
        {cx - (int)(6 * scale), cy + (int)(15 * scale)}
    };
    Polygon(hdc, legL, 4);

    POINT legR[4] = {
        {cx + (int)(6 * scale), cy + (int)(15 * scale)},
        {cx + (int)(6 * scale), cy + (int)(44 * scale)},
        {cx + (int)(20 * scale), cy + (int)(35 * scale)},
        {cx + (int)(12 * scale), cy + (int)(15 * scale)}
    };
    Polygon(hdc, legR, 4);

    // Knee armor plates
    SelectObject(hdc, pPauldronBrush);
    Rectangle(hdc, cx - (int)(18 * scale), cy + (int)(24 * scale), cx - (int)(8 * scale), cy + (int)(32 * scale));
    Rectangle(hdc, cx + (int)(8 * scale), cy + (int)(24 * scale), cx + (int)(18 * scale), cy + (int)(32 * scale));

    // Left Arm (Sub / Shield arm)
    SelectObject(hdc, pBrush);
    POINT armL[4] = {
        {cx - (int)(20 * scale), cy - (int)(10 * scale)},
        {cx - (int)(32 * scale), cy + (int)(5 * scale)},
        {cx - (int)(26 * scale), cy + (int)(24 * scale)},
        {cx - (int)(18 * scale), cy + (int)(6 * scale)}
    };
    Polygon(hdc, armL, 4);

    // Torso Chassis
    int torsoW = isHeavy ? (int)(26 * scale) : (isLight ? (int)(18 * scale) : (int)(22 * scale));
    POINT torsoP[4] = {
        {cx - torsoW, cy - (int)(18 * scale)},
        {cx + torsoW, cy - (int)(18 * scale)},
        {cx + (int)(14 * scale), cy + (int)(18 * scale)},
        {cx - (int)(14 * scale), cy + (int)(18 * scale)}
    };
    Polygon(hdc, torsoP, 4);

    // Center Core Reactor
    HBRUSH coreBrush = CreateSolidBrush(visorColor);
    HGDIOBJ oCore = SelectObject(hdc, coreBrush);
    Ellipse(hdc, cx - (int)(5 * scale), cy - (int)(4 * scale), cx + (int)(5 * scale), cy + (int)(6 * scale));
    SelectObject(hdc, oCore);
    DeleteObject(coreBrush);

    // Head & Cockpit
    int headW = isHeavy ? (int)(14 * scale) : (isLight ? (int)(9 * scale) : (int)(11 * scale));
    Rectangle(hdc, cx - headW, cy - (int)(34 * scale), cx + headW, cy - (int)(18 * scale));
    
    // Visor bar
    HPEN visorPen = CreatePen(PS_SOLID, 2, visorColor);
    HGDIOBJ oV = SelectObject(hdc, visorPen);
    MoveToEx(hdc, cx - headW + (int)(3 * scale), cy - (int)(26 * scale), NULL);
    LineTo(hdc, cx + headW - (int)(3 * scale), cy - (int)(26 * scale));
    SelectObject(hdc, oV);
    DeleteObject(visorPen);

    // Shoulder Pauldrons
    SelectObject(hdc, pPauldronBrush);
    POINT pL[3] = { {cx - torsoW - (int)(6 * scale), cy - (int)(16 * scale)}, {cx - (int)(10 * scale), cy - (int)(22 * scale)}, {cx - (int)(14 * scale), cy - (int)(6 * scale)} };
    Polygon(hdc, pL, 3);
    POINT pR[3] = { {cx + torsoW + (int)(6 * scale), cy - (int)(16 * scale)}, {cx + (int)(10 * scale), cy - (int)(22 * scale)}, {cx + (int)(14 * scale), cy - (int)(6 * scale)} };
    Polygon(hdc, pR, 3);

    // Right Weapon Arm
    SelectObject(hdc, pBrush);
    POINT armR[4] = {
        {cx + (int)(20 * scale), cy - (int)(10 * scale)},
        {cx + (int)(30 * scale), cy + (int)(4 * scale)},
        {cx + (int)(30 * scale), cy + (int)(22 * scale)},
        {cx + (int)(18 * scale), cy + (int)(6 * scale)}
    };
    Polygon(hdc, armR, 4);

    // Weapon Hardpoint Model
    if (equipWpn == 0) { // Basic Laser: Cyan Emitter
        HPEN wpnPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 255));
        HGDIOBJ oWP = SelectObject(hdc, wpnPen);
        MoveToEx(hdc, cx + (int)(26 * scale), cy + (int)(10 * scale), NULL);
        LineTo(hdc, cx + (int)(42 * scale), cy + (int)(10 * scale));
        Ellipse(hdc, cx + (int)(40 * scale), cy + (int)(8 * scale), cx + (int)(44 * scale), cy + (int)(12 * scale));
        SelectObject(hdc, oWP);
        DeleteObject(wpnPen);
    } else if (equipWpn == 1) { // Heavy Cannon: Large dark barrel
        HBRUSH cBrush = CreateSolidBrush(RGB(70, 90, 70));
        HGDIOBJ oB = SelectObject(hdc, cBrush);
        Rectangle(hdc, cx + (int)(26 * scale), cy + (int)(6 * scale), cx + (int)(46 * scale), cy + (int)(14 * scale));
        Rectangle(hdc, cx + (int)(42 * scale), cy + (int)(4 * scale), cx + (int)(47 * scale), cy + (int)(16 * scale));
        SelectObject(hdc, oB);
        DeleteObject(cBrush);
    } else { // Twin Blasters: Dual Orange Barrels
        HPEN wpnPen = CreatePen(PS_SOLID, 2, RGB(255, 160, 0));
        HGDIOBJ oWP = SelectObject(hdc, wpnPen);
        MoveToEx(hdc, cx + (int)(26 * scale), cy + (int)(6 * scale), NULL);
        LineTo(hdc, cx + (int)(40 * scale), cy + (int)(6 * scale));
        MoveToEx(hdc, cx + (int)(26 * scale), cy + (int)(14 * scale), NULL);
        LineTo(hdc, cx + (int)(40 * scale), cy + (int)(14 * scale));
        SelectObject(hdc, oWP);
        DeleteObject(wpnPen);
    }

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pPen);
    DeleteObject(pBrush);
    DeleteObject(pPauldronBrush);
}

void DrawEnemyMechGDI(HDC hdc, int cx, int cy, float scale, int battleNum, bool isDead) {
    if (isDead) {
        HPEN wreckPen = CreatePen(PS_SOLID, 2, RGB(100, 40, 40));
        HBRUSH wreckBrush = CreateSolidBrush(RGB(30, 10, 10));
        HGDIOBJ oldP = SelectObject(hdc, wreckPen);
        HGDIOBJ oldB = SelectObject(hdc, wreckBrush);
        Rectangle(hdc, cx - 20, cy - 10, cx + 20, cy + 20);
        SelectObject(hdc, oldP);
        SelectObject(hdc, oldB);
        DeleteObject(wreckPen);
        DeleteObject(wreckBrush);
        return;
    }

    int enemyType = (battleNum >= 5) ? 2 : ((battleNum >= 3) ? 1 : 0);
    COLORREF enemyRed = RGB(255, 50, 50);
    COLORREF enemyDark = RGB(55, 10, 10);
    COLORREF enemyEye = RGB(255, 255, 0);

    HPEN ePen = CreatePen(PS_SOLID, (int)(2 * scale), enemyRed);
    HBRUSH eBrush = CreateSolidBrush(enemyDark);
    HGDIOBJ oldPen = SelectObject(hdc, ePen);
    HGDIOBJ oldBrush = SelectObject(hdc, eBrush);

    if (enemyType == 0) { // Scout Raider (Battle 1-2)
        // Digitigrade Legs
        MoveToEx(hdc, cx - (int)(12 * scale), cy + (int)(15 * scale), NULL);
        LineTo(hdc, cx - (int)(24 * scale), cy + (int)(28 * scale));
        LineTo(hdc, cx - (int)(16 * scale), cy + (int)(44 * scale));
        LineTo(hdc, cx - (int)(26 * scale), cy + (int)(46 * scale));

        MoveToEx(hdc, cx + (int)(12 * scale), cy + (int)(15 * scale), NULL);
        LineTo(hdc, cx + (int)(24 * scale), cy + (int)(28 * scale));
        LineTo(hdc, cx + (int)(16 * scale), cy + (int)(44 * scale));
        LineTo(hdc, cx + (int)(26 * scale), cy + (int)(46 * scale));

        // Torso
        POINT torsoE[4] = {
            {cx - (int)(18 * scale), cy - (int)(18 * scale)},
            {cx + (int)(18 * scale), cy - (int)(18 * scale)},
            {cx + (int)(12 * scale), cy + (int)(16 * scale)},
            {cx - (int)(12 * scale), cy + (int)(16 * scale)}
        };
        Polygon(hdc, torsoE, 4);

        // Head
        Ellipse(hdc, cx - (int)(10 * scale), cy - (int)(32 * scale), cx + (int)(10 * scale), cy - (int)(14 * scale));
        HPEN eyePen = CreatePen(PS_SOLID, 2, enemyEye);
        HGDIOBJ oE = SelectObject(hdc, eyePen);
        MoveToEx(hdc, cx - (int)(6 * scale), cy - (int)(24 * scale), NULL);
        LineTo(hdc, cx + (int)(6 * scale), cy - (int)(24 * scale));
        SelectObject(hdc, oE);
        DeleteObject(eyePen);

        // Needle Gun Arms
        MoveToEx(hdc, cx - (int)(18 * scale), cy - (int)(6 * scale), NULL);
        LineTo(hdc, cx - (int)(34 * scale), cy + (int)(16 * scale));
        MoveToEx(hdc, cx + (int)(18 * scale), cy - (int)(6 * scale), NULL);
        LineTo(hdc, cx + (int)(34 * scale), cy + (int)(16 * scale));

    } else if (enemyType == 1) { // Assault Goliath (Battle 3-4)
        // Heavy Torso
        POINT torsoE[4] = {
            {cx - (int)(26 * scale), cy - (int)(20 * scale)},
            {cx + (int)(26 * scale), cy - (int)(20 * scale)},
            {cx + (int)(16 * scale), cy + (int)(18 * scale)},
            {cx - (int)(16 * scale), cy + (int)(18 * scale)}
        };
        Polygon(hdc, torsoE, 4);

        // Heavy Legs
        Rectangle(hdc, cx - (int)(22 * scale), cy + (int)(16 * scale), cx - (int)(8 * scale), cy + (int)(44 * scale));
        Rectangle(hdc, cx + (int)(8 * scale), cy + (int)(16 * scale), cx + (int)(22 * scale), cy + (int)(44 * scale));

        // Head & Visor
        Rectangle(hdc, cx - (int)(12 * scale), cy - (int)(34 * scale), cx + (int)(12 * scale), cy - (int)(18 * scale));
        HPEN eyePen = CreatePen(PS_SOLID, 2, enemyEye);
        HGDIOBJ oE = SelectObject(hdc, eyePen);
        MoveToEx(hdc, cx - (int)(8 * scale), cy - (int)(26 * scale), NULL);
        LineTo(hdc, cx + (int)(8 * scale), cy - (int)(26 * scale));
        SelectObject(hdc, oE);
        DeleteObject(eyePen);

        // Shoulder Missile Pod
        Rectangle(hdc, cx - (int)(32 * scale), cy - (int)(30 * scale), cx - (int)(18 * scale), cy - (int)(14 * scale));

        // Heavy Cannon Left Arm
        Rectangle(hdc, cx - (int)(38 * scale), cy - (int)(4 * scale), cx - (int)(26 * scale), cy + (int)(26 * scale));

    } else { // Siege Titan Dreadnought (Battle 5+)
        // Fortress Torso
        POINT torsoE[4] = {
            {cx - (int)(32 * scale), cy - (int)(22 * scale)},
            {cx + (int)(32 * scale), cy - (int)(22 * scale)},
            {cx + (int)(20 * scale), cy + (int)(20 * scale)},
            {cx - (int)(20 * scale), cy + (int)(20 * scale)}
        };
        Polygon(hdc, torsoE, 4);

        // Exhaust smoke stacks
        Rectangle(hdc, cx - (int)(26 * scale), cy - (int)(36 * scale), cx - (int)(20 * scale), cy - (int)(20 * scale));
        Rectangle(hdc, cx + (int)(20 * scale), cy - (int)(36 * scale), cx + (int)(26 * scale), cy - (int)(20 * scale));

        // Quad Legs
        Rectangle(hdc, cx - (int)(30 * scale), cy + (int)(18 * scale), cx - (int)(16 * scale), cy + (int)(46 * scale));
        Rectangle(hdc, cx + (int)(16 * scale), cy + (int)(18 * scale), cx + (int)(30 * scale), cy + (int)(46 * scale));

        // Central Cyclops Eye
        HBRUSH eyeBrush = CreateSolidBrush(enemyEye);
        HGDIOBJ oEye = SelectObject(hdc, eyeBrush);
        Ellipse(hdc, cx - (int)(8 * scale), cy - (int)(14 * scale), cx + (int)(8 * scale), cy + (int)(2 * scale));
        SelectObject(hdc, oEye);
        DeleteObject(eyeBrush);

        // Quad Turrets
        Rectangle(hdc, cx - (int)(42 * scale), cy - (int)(6 * scale), cx - (int)(30 * scale), cy + (int)(26 * scale));
        Rectangle(hdc, cx + (int)(30 * scale), cy - (int)(6 * scale), cx + (int)(42 * scale), cy + (int)(26 * scale));
    }

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(ePen);
    DeleteObject(eBrush);
}

void DrawSciFiHUDCornerFiligree(HDC hdc, int x, int y, int size, int cornerType) {
    HPEN fPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 170));
    HGDIOBJ oldP = SelectObject(hdc, fPen);
    
    if (cornerType == 0) { // Top-Left
        MoveToEx(hdc, x, y + size, NULL); LineTo(hdc, x, y); LineTo(hdc, x + size, y);
        SetPixel(hdc, x + 2, y + 2, RGB(0, 255, 255));
    } else if (cornerType == 1) { // Top-Right
        MoveToEx(hdc, x, y + size, NULL); LineTo(hdc, x, y); LineTo(hdc, x - size, y);
        SetPixel(hdc, x - 2, y + 2, RGB(0, 255, 255));
    } else if (cornerType == 2) { // Bottom-Left
        MoveToEx(hdc, x, y - size, NULL); LineTo(hdc, x, y); LineTo(hdc, x + size, y);
        SetPixel(hdc, x + 2, y - 2, RGB(0, 255, 255));
    } else { // Bottom-Right
        MoveToEx(hdc, x, y - size, NULL); LineTo(hdc, x, y); LineTo(hdc, x - size, y);
        SetPixel(hdc, x - 2, y - 2, RGB(0, 255, 255));
    }

    SelectObject(hdc, oldP);
    DeleteObject(fPen);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: {
            SetTimer(hwnd, 1, 33, NULL); // 30 FPS update loop
            break;
        }
        case WM_TIMER: {
            bool needsRedraw = false;
            if (animPlayerOffset > 0) { animPlayerOffset--; needsRedraw = true; }
            if (animEnemyOffset > 0) { animEnemyOffset--; needsRedraw = true; }
            if (animPlayerDmg > 0) { animPlayerDmg--; needsRedraw = true; }
            if (animEnemyDmg > 0) { animEnemyDmg--; needsRedraw = true; }

            // Update particles
            for (int i = 0; i < g_particleCount; ) {
                g_particles[i].x += g_particles[i].vx;
                g_particles[i].y += g_particles[i].vy;
                g_particles[i].vy += g_particles[i].grav;
                g_particles[i].life -= g_particles[i].decay;
                if (g_particles[i].life <= 0.0f) {
                    g_particles[i] = g_particles[g_particleCount - 1];
                    g_particleCount--;
                } else {
                    i++;
                }
                needsRedraw = true;
            }

            // Update shockwaves
            for (int i = 0; i < MAX_SHOCKWAVES; i++) {
                if (g_shockwaves[i].active) {
                    g_shockwaves[i].r += g_shockwaves[i].speed;
                    g_shockwaves[i].alpha = 1.0f - (g_shockwaves[i].r / g_shockwaves[i].maxR);
                    if (g_shockwaves[i].r >= g_shockwaves[i].maxR) {
                        g_shockwaves[i].active = false;
                    }
                    needsRedraw = true;
                }
            }

            // Update projectiles
            for (int i = 0; i < MAX_PROJECTILES; i++) {
                if (g_projectiles[i].active) {
                    g_projectiles[i].progress += g_projectiles[i].speed;
                    g_projectiles[i].x = g_projectiles[i].startX + (g_projectiles[i].targetX - g_projectiles[i].startX) * g_projectiles[i].progress;
                    g_projectiles[i].y = g_projectiles[i].startY + (g_projectiles[i].targetY - g_projectiles[i].startY) * g_projectiles[i].progress;
                    if (g_projectiles[i].progress >= 1.0f) {
                        g_projectiles[i].active = false;
                    }
                    needsRedraw = true;
                }
            }

            // Update screen shake
            if (g_screenShake > 0.05f) {
                g_shakePhase += 0.8f;
                g_screenShake *= 0.88f;
                needsRedraw = true;
            } else {
                g_screenShake = 0.0f;
            }

            if (gameState == STATE_BATTLE || gameState == STATE_POST_BATTLE || gameState == STATE_GARAGE) needsRedraw = true;
            if (needsRedraw) InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            if (gameState == STATE_GARAGE) {
                if (PtInRectLocal(&rectDeploy, x, y)) {
                    StartBattle();
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (PtInRectLocal(&rectRepair, x, y)) {
                    int missingHp = playerStats.maxHp - playerStats.hp;
                    if (missingHp <= 0) {
                        lstrcpyA(garageInfo, "Mech is already at maximum structural integrity.");
                    } else if (credits < 1) {
                        lstrcpyA(garageInfo, "Insufficient credits for repair.");
                    } else {
                        int cost = missingHp < credits ? missingHp : credits;
                        playerStats.hp += cost;
                        credits -= cost;
                        wsprintfA(garageInfo, "Repaired %d HP for %d Credits.", cost, cost);
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (PtInRectLocal(&rectUseSal, x, y)) {
                    if (salvage > 0) {
                        int missingHp = playerStats.maxHp - playerStats.hp;
                        if (missingHp <= 0) {
                            lstrcpyA(garageInfo, "Mech is already at max HP.");
                        } else {
                            int heal = missingHp < 50 ? missingHp : 50;
                            playerStats.hp += heal;
                            salvage--;
                            wsprintfA(garageInfo, "Used 1 Salvage Part to repair %d HP.", heal);
                        }
                    } else {
                        lstrcpyA(garageInfo, "No salvage parts available.");
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (PtInRectLocal(&rectSellSal, x, y)) {
                    if (salvage > 0) {
                        salvage--;
                        credits += 50;
                        lstrcpyA(garageInfo, "Sold 1 Salvage Part for 50 CR.");
                    } else {
                        lstrcpyA(garageInfo, "No salvage parts available.");
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (PtInRectLocal(&rectWpn, x, y)) {
                    equipWpn = (equipWpn + 1) % 3;
                    playerStats.atk = weapons[equipWpn].atk;
                    playerHeatGen = weapons[equipWpn].heatGen;
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (PtInRectLocal(&rectArm, x, y)) {
                    equipArm = (equipArm + 1) % 3;
                    playerStats.def = armors[equipArm].def;
                    playerStats.maxHp = armors[equipArm].maxHp;
                    playerStats.hp = playerStats.maxHp;
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (PtInRectLocal(&rectSink, x, y)) {
                    equipSink = (equipSink + 1) % 3;
                    playerStats.maxHeat = sinks[equipSink].maxHeat;
                    playerCooling = sinks[equipSink].cooling;
                    if (playerStats.heat > playerStats.maxHeat) playerStats.heat = playerStats.maxHeat;
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (PtInRectLocal(&rectSpec, x, y)) {
                    equipSpec = (equipSpec + 1) % 3;
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (PtInRectLocal(&rectHelp, x, y)) {
                    gameState = STATE_HELP;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (gameState == STATE_BATTLE) {
                if (PtInRectLocal(&rectTarget, x, y)) {
                    currentTargetLimb = (currentTargetLimb + 1) % 5;
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (PtInRectLocal(&rectAttack, x, y)) {
                    ActionAttack();
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (PtInRectLocal(&rectDefend, x, y)) {
                    ActionDefend();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (gameState == STATE_POST_BATTLE) {
                if (PtInRectLocal(&rectReturn, x, y)) {
                    ReturnToGarage();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (gameState == STATE_HELP) {
                if (PtInRectLocal(&rectReturnHelp, x, y)) {
                    gameState = STATE_GARAGE;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            
            // Double buffering
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
            SelectObject(memDC, memBitmap);
            
            // Background
            HBRUSH bgBrush = CreateSolidBrush(RGB(3, 10, 4));
            FillRect(memDC, &clientRect, bgBrush);
            DeleteObject(bgBrush);
            
            // Grid lines
            HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(0, 35, 15));
            HGDIOBJ oldPen = SelectObject(memDC, gridPen);
            for (int i = 0; i < clientRect.right; i += 20) {
                MoveToEx(memDC, i, 0, NULL);
                LineTo(memDC, i, clientRect.bottom);
            }
            for (int i = 0; i < clientRect.bottom; i += 20) {
                MoveToEx(memDC, 0, i, NULL);
                LineTo(memDC, clientRect.right, i);
            }
            SelectObject(memDC, oldPen);
            DeleteObject(gridPen);
            
            // Filigree corners
            DrawSciFiHUDCornerFiligree(memDC, 6, 6, 12, 0);
            DrawSciFiHUDCornerFiligree(memDC, clientRect.right - 6, 6, 12, 1);
            DrawSciFiHUDCornerFiligree(memDC, 6, clientRect.bottom - 6, 12, 2);
            DrawSciFiHUDCornerFiligree(memDC, clientRect.right - 6, clientRect.bottom - 6, 12, 3);

            int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
            int fontHeight = -MulDiv(12, dpi, 72);
            HFONT hFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_DONTCARE, "Consolas");
            SelectObject(memDC, hFont);
            SetTextColor(memDC, RGB(0, 255, 120));
            SetBkMode(memDC, TRANSPARENT);
            
            if (gameState == STATE_GARAGE) {
                RECT titleRect = {0, 10, 600, 35};
                DrawTextA(memDC, "⚡ KMECH - HANGAR & GARAGE", -1, &titleRect, DT_CENTER | DT_SINGLELINE);
                
                char camBuf[128];
                wsprintfA(camBuf, "BATTLE %d | CR: %d CR | SALVAGE: %d", battleCount, credits, salvage);
                RECT camRect = {20, 38, 320, 58};
                DrawTextA(memDC, camBuf, -1, &camRect, DT_LEFT | DT_SINGLELINE);

                char lvlBuf[128];
                wsprintfA(lvlBuf, "PILOT LVL: %d | XP: %d/%d", playerLevel, playerXp, playerLevel * 100);
                RECT lvlRect = {20, 58, 320, 78};
                DrawTextA(memDC, lvlBuf, -1, &lvlRect, DT_LEFT | DT_SINGLELINE);

                char buf[128];
                wsprintfA(buf, "HP: %d/%d  HEAT: %d/%d  ATK: %d  DEF: %d", playerStats.hp, playerStats.maxHp, playerStats.heat, playerStats.maxHeat, playerStats.atk, playerStats.def);
                RECT statsRect = {20, 80, 340, 105};
                DrawTextA(memDC, buf, -1, &statsRect, DT_LEFT | DT_SINGLELINE);
                
                RECT instRect = {20, 105, 340, 125};
                SetTextColor(memDC, RGB(140, 255, 170));
                DrawTextA(memDC, "[D] Deploy | [R] Repair | [U/S] Salvage", -1, &instRect, DT_LEFT | DT_SINGLELINE);
                SetTextColor(memDC, RGB(0, 255, 120));

                RECT msgRect = {20, 130, 340, 165};
                DrawTextA(memDC, garageInfo, -1, &msgRect, DT_LEFT | DT_WORDBREAK);

                // Garage Hangar Bay Blueprint Box
                RECT hangarRect = { 360, 38, 560, 168 };
                HBRUSH hangarBrush = CreateSolidBrush(RGB(5, 20, 10));
                HPEN hangarPen = CreatePen(PS_SOLID, 1, RGB(0, 180, 80));
                HGDIOBJ oB = SelectObject(memDC, hangarBrush);
                HGDIOBJ oP = SelectObject(memDC, hangarPen);
                RoundRect(memDC, hangarRect.left, hangarRect.top, hangarRect.right, hangarRect.bottom, 6, 6);
                SelectObject(memDC, oB);
                SelectObject(memDC, oP);
                DeleteObject(hangarBrush);
                DeleteObject(hangarPen);

                // Draw tactical Mech in hangar viewport
                DrawPlayerMechGDI(memDC, 460, 108, 0.95f, false);
                
                // Blueprint text
                RECT bpText = { 365, 42, 555, 60 };
                SetTextColor(memDC, RGB(0, 255, 255));
                DrawTextA(memDC, "[ HANGAR DIAGNOSTICS ]", -1, &bpText, DT_CENTER | DT_SINGLELINE);
                SetTextColor(memDC, RGB(0, 255, 120));
                
                RECT lw = {20, 205, 140, 225};
                DrawTextA(memDC, "[1] WEAPON", -1, &lw, DT_CENTER | DT_SINGLELINE);
                DrawButton(memDC, &rectWpn, weapons[equipWpn].name);

                RECT la = {150, 205, 270, 225};
                DrawTextA(memDC, "[2] ARMOR", -1, &la, DT_CENTER | DT_SINGLELINE);
                DrawButton(memDC, &rectArm, armors[equipArm].name);

                RECT ls = {280, 205, 400, 225};
                DrawTextA(memDC, "[3] HEAT SINK", -1, &ls, DT_CENTER | DT_SINGLELINE);
                DrawButton(memDC, &rectSink, sinks[equipSink].name);

                RECT lsp = {410, 205, 530, 225};
                DrawTextA(memDC, "[4] SPECIAL", -1, &lsp, DT_CENTER | DT_SINGLELINE);
                DrawButton(memDC, &rectSpec, specials[equipSpec].name);

                DrawButton(memDC, &rectRepair, "Repair [R]");
                DrawButton(memDC, &rectDeploy, "Deploy to Battle [D]");
                DrawButton(memDC, &rectUseSal, "Use Salvage [U] (+50 HP)");
                DrawButton(memDC, &rectSellSal, "Sell Salvage [S] (+50 CR)");
                DrawButton(memDC, &rectHelp, "Pilot's Manual [F1]");
            } else if (gameState == STATE_BATTLE || gameState == STATE_POST_BATTLE) {
                // Apply Screen Shake offset
                int shakeX = 0, shakeY = 0;
                if (g_screenShake > 0.05f) {
                    shakeX = (int)(FastCos(g_shakePhase) * g_screenShake);
                    shakeY = (int)(FastSin(g_shakePhase * 1.3f) * g_screenShake);
                }
                SetViewportOrgEx(memDC, shakeX, shakeY, NULL);

                RECT titleRect = {0, 10, 600, 35};
                DrawTextA(memDC, "⚔️ COMBAT ZONE - ENGAGED", -1, &titleRect, DT_CENTER | DT_SINGLELINE);
                
                // Player Mech Stats
                char bufP[64];
                wsprintfA(bufP, "Player Mech HP: %d | HEAT: %d", playerStats.hp, playerStats.heat);
                TextOutA(memDC, 30, 42, bufP, lstrlenA(bufP));
                
                HBRUSH hpBg = CreateSolidBrush(RGB(0, 40, 0));
                HBRUSH hpP = CreateSolidBrush(RGB(0, 255, 80));
                RECT pBarBg = {30, 62, 230, 70};
                FillRect(memDC, &pBarBg, hpBg);
                int pWidth = (playerStats.hp * 200) / playerStats.maxHp;
                if (pWidth > 0) {
                    RECT pBar = {30, 62, 30 + pWidth, 70};
                    FillRect(memDC, &pBar, hpP);
                }

                HBRUSH heatBg = CreateSolidBrush(RGB(40, 10, 0));
                HBRUSH heatP = CreateSolidBrush(RGB(255, 160, 0));
                RECT pHeatBg = {30, 73, 230, 78};
                FillRect(memDC, &pHeatBg, heatBg);
                int pHeatW = (playerStats.heat * 200) / playerStats.maxHeat;
                if (pHeatW > 200) pHeatW = 200;
                if (pHeatW > 0) {
                    RECT pHeatBar = {30, 73, 30 + pHeatW, 78};
                    FillRect(memDC, &pHeatBar, heatP);
                }
                
                int tick = GetTickCount();
                bool playerDead = (playerStats.hp <= 0);
                int playerBob = playerDead ? 8 : (pingpong(tick / 150, 6) - 3);
                int pOffX = 0, pOffY = playerBob;
                if (!playerDead && animPlayerOffset > 0) {
                    pOffX += (animPlayerOffset > 4) ? (8 - animPlayerOffset) * 4 : animPlayerOffset * 4;
                }
                if (!playerDead && animPlayerDmg > 0) {
                    pOffX += (my_rand() % 7) - 3; pOffY += (my_rand() % 7) - 3;
                }

                DrawPlayerMechGDI(memDC, 130 + pOffX, 135 + pOffY, 1.0f, playerDead);
                
                // Enemy Mech Stats
                char bufE[64];
                wsprintfA(bufE, "Enemy Mech HP: %d | HEAT: %d", enemyStats.hp, enemyStats.heat);
                SetTextColor(memDC, RGB(255, 80, 80));
                TextOutA(memDC, 350, 42, bufE, lstrlenA(bufE));
                
                HBRUSH hpE = CreateSolidBrush(RGB(255, 50, 50));
                RECT eBarBg = {350, 62, 550, 70};
                FillRect(memDC, &eBarBg, hpBg);
                int eWidth = (enemyStats.hp * 200) / enemyStats.maxHp;
                if (eWidth > 0) {
                    RECT eBar = {350, 62, 350 + eWidth, 70};
                    FillRect(memDC, &eBar, hpE);
                }

                RECT eHeatBg = {350, 73, 550, 78};
                FillRect(memDC, &eHeatBg, heatBg);
                int eHeatW = (enemyStats.heat * 200) / enemyStats.maxHeat;
                if (eHeatW > 200) eHeatW = 200;
                if (eHeatW > 0) {
                    RECT eHeatBar = {350, 73, 350 + eHeatW, 78};
                    FillRect(memDC, &eHeatBar, heatP);
                }
                
                bool enemyDead = (enemyStats.hp <= 0);
                int enemyBob = enemyDead ? 8 : (pingpong((tick + 300) / 150, 6) - 3);
                int eOffX = 0, eOffY = enemyBob;
                if (!enemyDead && animEnemyOffset > 0) {
                    eOffX -= (animEnemyOffset > 4) ? (8 - animEnemyOffset) * 4 : animEnemyOffset * 4;
                }
                if (!enemyDead && animEnemyDmg > 0) {
                    eOffX += (my_rand() % 7) - 3; eOffY += (my_rand() % 7) - 3;
                }

                DrawEnemyMechGDI(memDC, 450 + eOffX, 135 + eOffY, 1.0f, battleCount, enemyDead);
                SetTextColor(memDC, RGB(0, 255, 120)); // Restore color
                
                // Draw active projectiles
                for (int i = 0; i < MAX_PROJECTILES; i++) {
                    if (g_projectiles[i].active) {
                        HPEN projPen;
                        if (g_projectiles[i].fromPlayer) {
                            if (g_projectiles[i].weaponType == 0) projPen = CreatePen(PS_SOLID, 3, RGB(0, 255, 255));
                            else if (g_projectiles[i].weaponType == 1) projPen = CreatePen(PS_SOLID, 4, RGB(255, 180, 0));
                            else projPen = CreatePen(PS_SOLID, 2, RGB(255, 140, 0));
                        } else {
                            projPen = CreatePen(PS_SOLID, 3, RGB(255, 50, 50));
                        }
                        HGDIOBJ oldProjP = SelectObject(memDC, projPen);
                        MoveToEx(memDC, (int)g_projectiles[i].x - 12, (int)g_projectiles[i].y, NULL);
                        LineTo(memDC, (int)g_projectiles[i].x + 12, (int)g_projectiles[i].y);
                        SelectObject(memDC, oldProjP);
                        DeleteObject(projPen);
                    }
                }

                // Draw shockwaves
                for (int i = 0; i < MAX_SHOCKWAVES; i++) {
                    if (g_shockwaves[i].active) {
                        HPEN swPen = CreatePen(PS_SOLID, 2, g_shockwaves[i].color);
                        HGDIOBJ oSWP = SelectObject(memDC, swPen);
                        HGDIOBJ oSWB = SelectObject(memDC, GetStockObject(NULL_BRUSH));
                        int r = (int)g_shockwaves[i].r;
                        Ellipse(memDC, (int)g_shockwaves[i].x - r, (int)g_shockwaves[i].y - r, (int)g_shockwaves[i].x + r, (int)g_shockwaves[i].y + r);
                        SelectObject(memDC, oSWP);
                        SelectObject(memDC, oSWB);
                        DeleteObject(swPen);
                    }
                }

                // Draw particles
                for (int i = 0; i < g_particleCount; i++) {
                    HPEN ptPen = CreatePen(PS_SOLID, 1, g_particles[i].color);
                    HBRUSH ptBrush = CreateSolidBrush(g_particles[i].color);
                    HGDIOBJ oPtP = SelectObject(memDC, ptPen);
                    HGDIOBJ oPtB = SelectObject(memDC, ptBrush);
                    int pr = g_particles[i].size;
                    Ellipse(memDC, (int)g_particles[i].x - pr, (int)g_particles[i].y - pr, (int)g_particles[i].x + pr, (int)g_particles[i].y + pr);
                    SelectObject(memDC, oPtP);
                    SelectObject(memDC, oPtB);
                    DeleteObject(ptPen);
                    DeleteObject(ptBrush);
                }

                SetViewportOrgEx(memDC, 0, 0, NULL);
                
                DeleteObject(hpBg);
                DeleteObject(hpP);
                DeleteObject(hpE);
                DeleteObject(heatBg);
                DeleteObject(heatP);
                
                // Draw Logs
                RECT logBg = {30, 205, 550, 395};
                HBRUSH logBrush = CreateSolidBrush(RGB(0, 18, 8));
                FillRect(memDC, &logBg, logBrush);
                DeleteObject(logBrush);
                
                for (int i = 0; i < logCount; i++) {
                    char logOut[130];
                    wsprintfA(logOut, "> %s", battleLogs[i]);
                    TextOutA(memDC, 40, 210 + (i * 18), logOut, lstrlenA(logOut));
                }
                
                if (gameState == STATE_BATTLE) {
                    char targetText[64];
                    wsprintfA(targetText, "TGT: %s [T]", limbNames[currentTargetLimb]);
                    DrawButton(memDC, &rectTarget, targetText);
                    DrawButton(memDC, &rectAttack, "Attack [A/Space]");
                    DrawButton(memDC, &rectDefend, "Defend [D]");
                } else if (gameState == STATE_POST_BATTLE) {
                    DrawButton(memDC, &rectReturn, "Return to Garage [R/Space]");
                }
            } else if (gameState == STATE_HELP) {
                RECT titleRect = {0, 15, 600, 45};
                DrawTextA(memDC, "📖 PILOT'S MANUAL & CONTROLS", -1, &titleRect, DT_CENTER | DT_SINGLELINE);

                const char* helpText = 
                    "CONTROLS & SHORTCUTS:\n"
                    "- Garage: [D/Enter] Deploy | [R] Repair | [U] Use Salvage | [S] Sell Salvage\n"
                    "- Equipment: [1] Weapon | [2] Armor | [3] Heat Sink | [4] Special\n"
                    "- Combat: [A/Space/1] Attack | [D/2] Defend | [T] Cycle Target Limb\n"
                    "- Post-Battle: [R/Space/Enter/Esc] Return to Garage\n"
                    "- Anywhere: [F1/H/Esc] Open / Close Pilot's Manual\n\n"
                    "TACTICAL COMBAT & HEAT:\n"
                    "- Attack: Deals weapon damage and builds heat. Exceeding Max Heat causes -15 HP core burnout!\n"
                    "- Defend: Doubles DEF plating, 0 heat gen, cools reactors.\n"
                    "- Targeting: Head (3.0x dmg, low hit%), Torso (1.0x, high hit%), Limbs (1.5x).\n\n"
                    "PARTS & UPGRADES:\n"
                    "- Pilot Leveling: Adds +5% base accuracy & evasion per level.\n"
                    "- Jump Jets: +20% Evasion bonus | Energy Shield: Absorbs 5 dmg/hit.";
                
                RECT textRect = {35, 55, 565, 415};
                DrawTextA(memDC, helpText, -1, &textRect, DT_LEFT | DT_WORDBREAK);

                DrawButton(memDC, &rectReturnHelp, "Back to Garage [Esc/Enter]");
            }
            
            // Blit and clean up
            BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, SRCCOPY);
            DeleteObject(hFont);
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_KEYDOWN: {
            if (wParam == VK_F1 || wParam == 'H') {
                if (gameState != STATE_HELP) {
                    gameState = STATE_HELP;
                } else {
                    gameState = STATE_GARAGE;
                }
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            if (gameState == STATE_HELP) {
                if (wParam == VK_ESCAPE || wParam == VK_RETURN || wParam == VK_SPACE || wParam == VK_BACK) {
                    gameState = STATE_GARAGE;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (gameState == STATE_GARAGE) {
                if (wParam == 'D' || wParam == VK_RETURN) {
                    StartBattle();
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (wParam == 'R') {
                    int missingHp = playerStats.maxHp - playerStats.hp;
                    if (missingHp <= 0) {
                        lstrcpyA(garageInfo, "Mech hull is already at max integrity.");
                    } else if (credits < 1) {
                        lstrcpyA(garageInfo, "Insufficient credits for repair.");
                    } else {
                        int cost = missingHp < credits ? missingHp : credits;
                        playerStats.hp += cost;
                        credits -= cost;
                        wsprintfA(garageInfo, "Repaired %d HP for %d Credits.", cost, cost);
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (wParam == 'U') {
                    if (salvage > 0) {
                        int missingHp = playerStats.maxHp - playerStats.hp;
                        if (missingHp <= 0) {
                            lstrcpyA(garageInfo, "Mech is already at max HP.");
                        } else {
                            int heal = missingHp < 50 ? missingHp : 50;
                            playerStats.hp += heal;
                            salvage--;
                            wsprintfA(garageInfo, "Used 1 Salvage Part to repair %d HP.", heal);
                        }
                    } else {
                        lstrcpyA(garageInfo, "No salvage parts available.");
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (wParam == 'S') {
                    if (salvage > 0) {
                        salvage--;
                        credits += 50;
                        lstrcpyA(garageInfo, "Sold 1 Salvage Part for 50 CR.");
                    } else {
                        lstrcpyA(garageInfo, "No salvage parts available.");
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (wParam == '1') {
                    equipWpn = (equipWpn + 1) % 3;
                    playerStats.atk = weapons[equipWpn].atk;
                    playerHeatGen = weapons[equipWpn].heatGen;
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (wParam == '2') {
                    equipArm = (equipArm + 1) % 3;
                    playerStats.def = armors[equipArm].def;
                    playerStats.maxHp = armors[equipArm].maxHp;
                    playerStats.hp = playerStats.maxHp;
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (wParam == '3') {
                    equipSink = (equipSink + 1) % 3;
                    playerStats.maxHeat = sinks[equipSink].maxHeat;
                    playerCooling = sinks[equipSink].cooling;
                    if (playerStats.heat > playerStats.maxHeat) playerStats.heat = playerStats.maxHeat;
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (wParam == '4') {
                    equipSpec = (equipSpec + 1) % 3;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (gameState == STATE_BATTLE) {
                if (wParam == 'A' || wParam == VK_SPACE || wParam == '1') {
                    ActionAttack();
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (wParam == 'D' || wParam == '2') {
                    ActionDefend();
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (wParam == 'T') {
                    currentTargetLimb = (currentTargetLimb + 1) % 5;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (gameState == STATE_POST_BATTLE) {
                if (wParam == VK_RETURN || wParam == VK_SPACE || wParam == 'R' || wParam == VK_ESCAPE) {
                    ReturnToGarage();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    SetProcessDPIAware();
    my_srand(GetTickCount());
    
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KMechWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClassA(&wc);

    DWORD style = (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN;
    RECT rect = { 0, 0, 600, 500 };
    AdjustWindowRect(&rect, style, FALSE);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    
    HWND hwnd = CreateWindowA("KMechWindowClass", "KMech - Combat Simulator [F1 for Help]", style,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ExitProcess(0);
}
