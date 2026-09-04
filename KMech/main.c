#include <windows.h>
#include <stdbool.h>

#define STATE_GARAGE 0
#define STATE_BATTLE 1
#define STATE_POST_BATTLE 2
#define STATE_HELP 3

static unsigned int g_seed = 0;
void my_srand(unsigned int seed) { g_seed = seed; }
int my_rand() { g_seed = g_seed * 214013 + 2531011; return (g_seed >> 16) & 0x7FFF; }

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
            }
        }

        playerStats.hp -= dmg;
        playerLimbDamage[target] += dmg;
        animPlayerDmg = 10;
        
        char buf[128];
        wsprintfA(buf, "Enemy hits %s! Took %d dmg.", limbNames[target], dmg);
        addLog(buf);
        Beep(900, 50); Beep(700, 50); Beep(500, 100); // Enemy laser
        Beep(150, 100); Beep(100, 150); // Impact
    }

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

        char buf[128];
        wsprintfA(buf, "You hit %s! Dealt %d dmg.", limbNames[target], dmg);
        addLog(buf);
        Beep(1000, 50); Beep(800, 50); Beep(600, 100); // Player laser
        Beep(150, 100); Beep(100, 150); // Impact
    }

    if (enemyStats.hp <= 0) {
        enemyStats.hp = 0;
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
RECT rectRepair  = {  80, 260, 280, 300 };
RECT rectDeploy  = { 320, 260, 520, 300 };
RECT rectUseSal  = {  80, 310, 280, 350 };
RECT rectSellSal = { 320, 310, 520, 350 };

RECT rectTarget = { 50, 400, 190, 440 };
RECT rectAttack = { 210, 400, 350, 440 };
RECT rectDefend = { 370, 400, 510, 440 };
RECT rectReturn = { 200, 400, 400, 440 };

RECT rectHelp = { 200, 360, 400, 400 };
RECT rectReturnHelp = { 200, 430, 400, 470 };

RECT rectWpn = { 20, 200, 150, 240 };
RECT rectArm = { 160, 200, 290, 240 };
RECT rectSink = { 300, 200, 430, 240 };
RECT rectSpec = { 440, 200, 570, 240 };

bool PtInRectLocal(const RECT* r, int x, int y) {
    return (x >= r->left && x <= r->right && y >= r->top && y <= r->bottom);
}

void DrawButton(HDC hdc, const RECT* r, const char* text) {
    HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0));
    HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
    HGDIOBJ oldBrush = SelectObject(hdc, bgBrush);
    HGDIOBJ oldPen = SelectObject(hdc, borderPen);
    
    Rectangle(hdc, r->left, r->top, r->right, r->bottom);
    
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 255, 0));
    DrawTextA(hdc, text, -1, (RECT*)r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(bgBrush);
    DeleteObject(borderPen);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: {
            SetTimer(hwnd, 1, 50, NULL);
            break;
        }
        case WM_TIMER: {
            bool needsRedraw = false;
            if (animPlayerOffset > 0) { animPlayerOffset--; needsRedraw = true; }
            if (animEnemyOffset > 0) { animEnemyOffset--; needsRedraw = true; }
            if (animPlayerDmg > 0) { animPlayerDmg--; needsRedraw = true; }
            if (animEnemyDmg > 0) { animEnemyDmg--; needsRedraw = true; }
            if (gameState == STATE_BATTLE || gameState == STATE_POST_BATTLE) needsRedraw = true;
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
            
            // Draw background
            HBRUSH bgBrush = CreateSolidBrush(RGB(5, 5, 5));
            FillRect(memDC, &clientRect, bgBrush);
            DeleteObject(bgBrush);
            
            // Draw grid
            HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(0, 40, 0));
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
            
            int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
            int fontHeight = -MulDiv(12, dpi, 72);
            HFONT hFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_DONTCARE, "Consolas");
            SelectObject(memDC, hFont);
            SetTextColor(memDC, RGB(0, 255, 0));
            SetBkMode(memDC, TRANSPARENT);
            
            if (gameState == STATE_GARAGE) {
                RECT titleRect = {0, 15, 600, 45};
                DrawTextA(memDC, "⚡ KMECH - GARAGE", -1, &titleRect, DT_CENTER | DT_SINGLELINE);
                
                char camBuf[128];
                wsprintfA(camBuf, "BATTLE %d | CR: %d CR | SALVAGE: %d", battleCount, credits, salvage);
                RECT camRect = {0, 45, 600, 65};
                DrawTextA(memDC, camBuf, -1, &camRect, DT_CENTER | DT_SINGLELINE);

                char lvlBuf[128];
                wsprintfA(lvlBuf, "PILOT LVL: %d | XP: %d/%d", playerLevel, playerXp, playerLevel * 100);
                RECT lvlRect = {0, 65, 600, 85};
                DrawTextA(memDC, lvlBuf, -1, &lvlRect, DT_CENTER | DT_SINGLELINE);

                char buf[128];
                wsprintfA(buf, "HP: %d/%d  HEAT: %d/%d  ATK: %d  DEF: %d", playerStats.hp, playerStats.maxHp, playerStats.heat, playerStats.maxHeat, playerStats.atk, playerStats.def);
                RECT statsRect = {0, 90, 600, 115};
                DrawTextA(memDC, buf, -1, &statsRect, DT_CENTER | DT_SINGLELINE);
                
                RECT instRect = {0, 115, 600, 135};
                SetTextColor(memDC, RGB(140, 255, 170));
                DrawTextA(memDC, "[D/Enter] Deploy | [R] Repair | [U] Salvage | [S] Sell | [1-4] Gear | [F1] Manual", -1, &instRect, DT_CENTER | DT_SINGLELINE);
                SetTextColor(memDC, RGB(0, 255, 0));

                RECT msgRect = {0, 140, 600, 175};
                DrawTextA(memDC, garageInfo, -1, &msgRect, DT_CENTER | DT_SINGLELINE);
                
                RECT lw = {20, 175, 150, 195};
                DrawTextA(memDC, "[1] WEAPON", -1, &lw, DT_CENTER | DT_SINGLELINE);
                DrawButton(memDC, &rectWpn, weapons[equipWpn].name);

                RECT la = {160, 175, 290, 195};
                DrawTextA(memDC, "[2] ARMOR", -1, &la, DT_CENTER | DT_SINGLELINE);
                DrawButton(memDC, &rectArm, armors[equipArm].name);

                RECT ls = {300, 175, 430, 195};
                DrawTextA(memDC, "[3] HEAT SINK", -1, &ls, DT_CENTER | DT_SINGLELINE);
                DrawButton(memDC, &rectSink, sinks[equipSink].name);

                RECT lsp = {440, 175, 570, 195};
                DrawTextA(memDC, "[4] SPECIAL", -1, &lsp, DT_CENTER | DT_SINGLELINE);
                DrawButton(memDC, &rectSpec, specials[equipSpec].name);

                DrawButton(memDC, &rectRepair, "Repair [R]");
                DrawButton(memDC, &rectDeploy, "Deploy to Battle [D]");
                DrawButton(memDC, &rectUseSal, "Use Salvage [U] (+50 HP)");
                DrawButton(memDC, &rectSellSal, "Sell Salvage [S] (+50 CR)");
                DrawButton(memDC, &rectHelp, "Pilot's Manual [F1]");
            } else if (gameState == STATE_BATTLE || gameState == STATE_POST_BATTLE) {
                RECT titleRect = {0, 15, 600, 45};
                DrawTextA(memDC, "⚔️ COMBAT ZONE", -1, &titleRect, DT_CENTER | DT_SINGLELINE);
                
                // Player Mech Stats
                char bufP[64];
                wsprintfA(bufP, "Player Mech HP: %d | HEAT: %d", playerStats.hp, playerStats.heat);
                TextOutA(memDC, 30, 55, bufP, lstrlenA(bufP));
                
                HBRUSH hpBg = CreateSolidBrush(RGB(0, 50, 0));
                HBRUSH hpP = CreateSolidBrush(RGB(0, 255, 0));
                RECT pBarBg = {30, 80, 230, 90};
                FillRect(memDC, &pBarBg, hpBg);
                int pWidth = (playerStats.hp * 200) / playerStats.maxHp;
                if (pWidth > 0) {
                    RECT pBar = {30, 80, 30 + pWidth, 90};
                    FillRect(memDC, &pBar, hpP);
                }

                HBRUSH heatBg = CreateSolidBrush(RGB(50, 0, 0));
                HBRUSH heatP = CreateSolidBrush(RGB(255, 165, 0));
                RECT pHeatBg = {30, 93, 230, 98};
                FillRect(memDC, &pHeatBg, heatBg);
                int pHeatW = (playerStats.heat * 200) / playerStats.maxHeat;
                if (pHeatW > 200) pHeatW = 200;
                if (pHeatW > 0) {
                    RECT pHeatBar = {30, 93, 30 + pHeatW, 98};
                    FillRect(memDC, &pHeatBar, heatP);
                }
                
                int tick = GetTickCount();
                bool playerDead = (playerStats.hp <= 0);
                int playerBob = playerDead ? 10 : (pingpong(tick / 150, 6) - 3);
                int pOffX = 0, pOffY = playerBob;
                if (!playerDead && animPlayerOffset > 0) {
                    pOffX += (animPlayerOffset > 4) ? (8 - animPlayerOffset) * 4 : animPlayerOffset * 4;
                }
                if (!playerDead && animPlayerDmg > 0) {
                    pOffX += (my_rand() % 7) - 3; pOffY += (my_rand() % 7) - 3;
                }
                SetViewportOrgEx(memDC, pOffX, pOffY, NULL);

                HPEN pPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
                HBRUSH pBrush = CreateSolidBrush(RGB(0, 50, 0));
                HGDIOBJ pOldPen = SelectObject(memDC, pPen);
                HGDIOBJ pOldBrush = SelectObject(memDC, pBrush);
                Rectangle(memDC, 100, 105, 120, 120);
                MoveToEx(memDC, 105, 110, NULL); LineTo(memDC, 115, 110);
                POINT torsoP[4] = { {90, 120}, {130, 120}, {120, 155}, {100, 155} };
                Polygon(memDC, torsoP, 4);
                Rectangle(memDC, 100, 125, 120, 145);
                MoveToEx(memDC, 90, 125, NULL); LineTo(memDC, 70, 145); LineTo(memDC, 70, 165);
                Ellipse(memDC, 66, 165, 74, 173);
                MoveToEx(memDC, 130, 125, NULL); LineTo(memDC, 150, 145); LineTo(memDC, 150, 165);
                Rectangle(memDC, 145, 165, 155, 180);
                MoveToEx(memDC, 100, 155, NULL); LineTo(memDC, 90, 185); LineTo(memDC, 80, 185);
                MoveToEx(memDC, 120, 155, NULL); LineTo(memDC, 130, 185); LineTo(memDC, 140, 185);
                SelectObject(memDC, pOldPen);
                SelectObject(memDC, pOldBrush);
                DeleteObject(pPen);
                DeleteObject(pBrush);

                if (animPlayerDmg > 0) {
                    HPEN dmgPen = CreatePen(PS_SOLID, 2, RGB(255, 100, 0));
                    HGDIOBJ old = SelectObject(memDC, dmgPen);
                    for (int i=0; i<5; i++) {
                        int px = 110 + (my_rand() % 60) - 30;
                        int py = 135 + (my_rand() % 60) - 30;
                        int pr = 3 + (my_rand() % 7);
                        Ellipse(memDC, px-pr, py-pr, px+pr, py+pr);
                    }
                    SelectObject(memDC, old);
                    DeleteObject(dmgPen);
                }
                if (playerDead) {
                    HPEN firePen = CreatePen(PS_SOLID, 1, RGB(255, 100, 0));
                    HBRUSH fireBrush = CreateSolidBrush(RGB(255, 50, 0));
                    HGDIOBJ oldP = SelectObject(memDC, firePen);
                    HGDIOBJ oldB = SelectObject(memDC, fireBrush);
                    for (int i=0; i<10; i++) {
                        int px = 110 + (my_rand() % 50) - 25;
                        int py = 145 + (my_rand() % 40) - 20;
                        int pr = 4 + (my_rand() % 8);
                        Ellipse(memDC, px-pr, py-pr, px+pr, py+pr);
                    }
                    SelectObject(memDC, oldP);
                    SelectObject(memDC, oldB);
                    DeleteObject(fireBrush);
                    DeleteObject(firePen);
                }
                SetViewportOrgEx(memDC, 0, 0, NULL);
                
                // Enemy Mech Stats
                char bufE[64];
                wsprintfA(bufE, "Enemy Mech HP: %d | HEAT: %d", enemyStats.hp, enemyStats.heat);
                SetTextColor(memDC, RGB(255, 0, 0));
                TextOutA(memDC, 350, 55, bufE, lstrlenA(bufE));
                
                HBRUSH hpE = CreateSolidBrush(RGB(255, 0, 0));
                RECT eBarBg = {350, 80, 550, 90};
                FillRect(memDC, &eBarBg, hpBg);
                int eWidth = (enemyStats.hp * 200) / enemyStats.maxHp;
                if (eWidth > 0) {
                    RECT eBar = {350, 80, 350 + eWidth, 90};
                    FillRect(memDC, &eBar, hpE);
                }

                RECT eHeatBg = {350, 93, 550, 98};
                FillRect(memDC, &eHeatBg, heatBg);
                int eHeatW = (enemyStats.heat * 200) / enemyStats.maxHeat;
                if (eHeatW > 200) eHeatW = 200;
                if (eHeatW > 0) {
                    RECT eHeatBar = {350, 93, 350 + eHeatW, 98};
                    FillRect(memDC, &eHeatBar, heatP);
                }
                
                bool enemyDead = (enemyStats.hp <= 0);
                int enemyBob = enemyDead ? 10 : (pingpong((tick + 300) / 150, 6) - 3);
                int eOffX = 0, eOffY = enemyBob;
                if (!enemyDead && animEnemyOffset > 0) {
                    eOffX -= (animEnemyOffset > 4) ? (8 - animEnemyOffset) * 4 : animEnemyOffset * 4;
                }
                if (!enemyDead && animEnemyDmg > 0) {
                    eOffX += (my_rand() % 7) - 3; eOffY += (my_rand() % 7) - 3;
                }
                SetViewportOrgEx(memDC, eOffX, eOffY, NULL);

                HPEN ePen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
                HBRUSH eBrush = CreateSolidBrush(RGB(50, 0, 0));
                HBRUSH eRedBrush = CreateSolidBrush(RGB(255, 0, 0));
                HGDIOBJ eOldPen = SelectObject(memDC, ePen);
                HGDIOBJ eOldBrush = SelectObject(memDC, eBrush);
                Ellipse(memDC, 410, 105, 430, 125);
                SelectObject(memDC, eRedBrush);
                Ellipse(memDC, 414, 111, 418, 115);
                Ellipse(memDC, 422, 111, 426, 115);
                SelectObject(memDC, eBrush);
                POINT torsoE[4] = { {390, 125}, {450, 125}, {440, 155}, {400, 155} };
                Polygon(memDC, torsoE, 4);
                MoveToEx(memDC, 390, 130, NULL); LineTo(memDC, 370, 145); LineTo(memDC, 380, 165);
                MoveToEx(memDC, 450, 130, NULL); LineTo(memDC, 470, 145); LineTo(memDC, 460, 165);
                MoveToEx(memDC, 400, 155, NULL); LineTo(memDC, 390, 185); LineTo(memDC, 380, 185);
                MoveToEx(memDC, 440, 155, NULL); LineTo(memDC, 450, 185); LineTo(memDC, 460, 185);
                SelectObject(memDC, eOldPen);
                SelectObject(memDC, eOldBrush);
                DeleteObject(ePen);
                DeleteObject(eBrush);
                DeleteObject(eRedBrush);

                if (animEnemyDmg > 0) {
                    HPEN dmgPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 0));
                    HGDIOBJ old = SelectObject(memDC, dmgPen);
                    for (int i=0; i<5; i++) {
                        int ex = 420 + (my_rand() % 60) - 30;
                        int ey = 135 + (my_rand() % 60) - 30;
                        int er = 3 + (my_rand() % 7);
                        Ellipse(memDC, ex-er, ey-er, ex+er, ey+er);
                    }
                    SelectObject(memDC, old);
                    DeleteObject(dmgPen);
                }
                if (enemyDead) {
                    HPEN firePen = CreatePen(PS_SOLID, 1, RGB(255, 100, 0));
                    HBRUSH fireBrush = CreateSolidBrush(RGB(255, 50, 0));
                    HGDIOBJ oldP = SelectObject(memDC, firePen);
                    HGDIOBJ oldB = SelectObject(memDC, fireBrush);
                    for (int i=0; i<10; i++) {
                        int ex = 420 + (my_rand() % 50) - 25;
                        int ey = 145 + (my_rand() % 40) - 20;
                        int er = 4 + (my_rand() % 8);
                        Ellipse(memDC, ex-er, ey-er, ex+er, ey+er);
                    }
                    SelectObject(memDC, oldP);
                    SelectObject(memDC, oldB);
                    DeleteObject(fireBrush);
                    DeleteObject(firePen);
                }
                SetViewportOrgEx(memDC, 0, 0, NULL);
                SetTextColor(memDC, RGB(0, 255, 0)); // Restore color
                
                DeleteObject(hpBg);
                DeleteObject(hpP);
                DeleteObject(hpE);
                DeleteObject(heatBg);
                DeleteObject(heatP);
                
                // Draw Logs
                RECT logBg = {30, 205, 550, 385};
                HBRUSH logBrush = CreateSolidBrush(RGB(0, 20, 0));
                FillRect(memDC, &logBg, logBrush);
                DeleteObject(logBrush);
                
                for (int i = 0; i < logCount; i++) {
                    char logOut[130];
                    wsprintfA(logOut, "> %s", battleLogs[i]);
                    TextOutA(memDC, 40, 210 + (i * 16), logOut, lstrlenA(logOut));
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
