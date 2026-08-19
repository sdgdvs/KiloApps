#include <windows.h>
#include <stdbool.h>

#define STATE_GARAGE 0
#define STATE_BATTLE 1
#define STATE_POST_BATTLE 2

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

int equipWpn = 0;
int equipArm = 0;
int equipSink = 0;
int playerHeatGen = 30;
int playerCooling = 20;

int credits = 100;
int battleCount = 1;

MechStats playerStats = { 100, 100, 15, 5, 100, 0 };
MechStats enemyStats = { 80, 80, 12, 3, 100, 0 };

bool isDefending = false;

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
    enemyStats.heat += 30;
    if (enemyStats.heat > enemyStats.maxHeat) {
        enemyStats.hp -= 15;
        addLog("Enemy overheats! Takes 15 system dmg.");
        if (enemyStats.hp <= 0) {
            enemyStats.hp = 0;
            int reward = 50 + (battleCount * 10);
            credits += reward;
            char buf[128];
            wsprintfA(buf, "ENEMY DESTROYED. VICTORY. Earned %d Credits.", reward);
            addLog(buf);
            battleCount++;
            gameState = STATE_POST_BATTLE;
            playerVictory = true;
            return;
        }
    }

    int target = my_rand() % 5;
    int hitRoll = my_rand() % 100;
    
    if (hitRoll > (int)(limbHitChance[target] * 100.0f)) {
        char buf[128];
        wsprintfA(buf, "Enemy targets %s... Missed!", limbNames[target]);
        addLog(buf);
    } else {
        int effectiveDef = isDefending ? (playerStats.def * 2) : playerStats.def;
        int dmg = (enemyStats.atk - effectiveDef) + (my_rand() % 4);
        if (dmg < 1) dmg = 1;
        dmg = (int)(dmg * limbDmgMult[target]);
        playerStats.hp -= dmg;
        
        char buf[128];
        wsprintfA(buf, "Enemy hits %s! Took %d dmg.", limbNames[target], dmg);
        addLog(buf);
    }

    if (playerStats.hp <= 0) {
        playerStats.hp = 0;
        addLog("CRITICAL DAMAGE. MECH DESTROYED. CAMPAIGN FAILED.");
        battleCount = 1;
        credits = 100;
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
        if (playerStats.hp <= 0) {
            playerStats.hp = 0;
            addLog("CRITICAL DAMAGE. MECH DESTROYED. CAMPAIGN FAILED.");
            battleCount = 1;
            credits = 100;
            gameState = STATE_POST_BATTLE;
            playerVictory = false;
            return;
        }
    }

    int target = currentTargetLimb;
    int hitRoll = my_rand() % 100;
    
    if (hitRoll > (int)(limbHitChance[target] * 100.0f)) {
        char buf[128];
        wsprintfA(buf, "You target %s... Missed!", limbNames[target]);
        addLog(buf);
    } else {
        int dmg = (playerStats.atk - enemyStats.def) + (my_rand() % 5);
        if (dmg < 1) dmg = 1;
        dmg = (int)(dmg * limbDmgMult[target]);
        enemyStats.hp -= dmg;

        char buf[128];
        wsprintfA(buf, "You hit %s! Dealt %d dmg.", limbNames[target], dmg);
        addLog(buf);
    }

    if (enemyStats.hp <= 0) {
        enemyStats.hp = 0;
        int reward = 50 + (battleCount * 10);
        credits += reward;
        char buf[128];
        wsprintfA(buf, "ENEMY DESTROYED. VICTORY. Earned %d Credits.", reward);
        addLog(buf);
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
    clearLogs();
    addLog("Enemy mech detected! Engaging...");
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
RECT rectDeploy = { 200, 320, 400, 360 };
RECT rectRepair = { 200, 270, 400, 310 };
RECT rectTarget = { 50, 400, 190, 440 };
RECT rectAttack = { 210, 400, 350, 440 };
RECT rectDefend = { 370, 400, 510, 440 };
RECT rectReturn = { 200, 400, 400, 440 };

RECT rectWpn = { 50, 200, 190, 240 };
RECT rectArm = { 230, 200, 370, 240 };
RECT rectSink = { 410, 200, 550, 240 };

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
            
            HFONT hFont = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_DONTCARE, "Consolas");
            SelectObject(memDC, hFont);
            SetTextColor(memDC, RGB(0, 255, 0));
            SetBkMode(memDC, TRANSPARENT);
            
            if (gameState == STATE_GARAGE) {
                RECT titleRect = {0, 20, 600, 60};
                DrawTextA(memDC, "KMECH - GARAGE", -1, &titleRect, DT_CENTER | DT_SINGLELINE);
                
                char camBuf[128];
                wsprintfA(camBuf, "CAMPAIGN: BATTLE %d | CREDITS: %d", battleCount, credits);
                RECT camRect = {0, 70, 600, 90};
                DrawTextA(memDC, camBuf, -1, &camRect, DT_CENTER | DT_SINGLELINE);

                char buf[128];
                wsprintfA(buf, "HP: %d/%d  HEAT: %d/%d  ATK: %d  DEF: %d", playerStats.hp, playerStats.maxHp, playerStats.heat, playerStats.maxHeat, playerStats.atk, playerStats.def);
                RECT statsRect = {0, 100, 600, 140};
                DrawTextA(memDC, buf, -1, &statsRect, DT_CENTER | DT_SINGLELINE);
                
                RECT msgRect = {0, 150, 600, 190};
                DrawTextA(memDC, garageInfo, -1, &msgRect, DT_CENTER | DT_SINGLELINE);
                
                RECT lw = {50, 180, 190, 200};
                DrawTextA(memDC, "WEAPON", -1, &lw, DT_CENTER | DT_SINGLELINE);
                DrawButton(memDC, &rectWpn, weapons[equipWpn].name);

                RECT la = {230, 180, 370, 200};
                DrawTextA(memDC, "ARMOR", -1, &la, DT_CENTER | DT_SINGLELINE);
                DrawButton(memDC, &rectArm, armors[equipArm].name);

                RECT ls = {410, 180, 550, 200};
                DrawTextA(memDC, "HEAT SINK", -1, &ls, DT_CENTER | DT_SINGLELINE);
                DrawButton(memDC, &rectSink, sinks[equipSink].name);

                DrawButton(memDC, &rectRepair, "Repair (1 CR = 1 HP)");
                DrawButton(memDC, &rectDeploy, "Deploy to Battle");
            } else {
                RECT titleRect = {0, 20, 600, 60};
                DrawTextA(memDC, "COMBAT ZONE", -1, &titleRect, DT_CENTER | DT_SINGLELINE);
                
                // Player Mech Stats
                char bufP[64];
                wsprintfA(bufP, "Player Mech HP: %d | HEAT: %d", playerStats.hp, playerStats.heat);
                TextOutA(memDC, 30, 70, bufP, lstrlenA(bufP));
                
                HBRUSH hpBg = CreateSolidBrush(RGB(0, 50, 0));
                HBRUSH hpP = CreateSolidBrush(RGB(0, 255, 0));
                RECT pBarBg = {30, 95, 230, 105};
                FillRect(memDC, &pBarBg, hpBg);
                int pWidth = (playerStats.hp * 200) / playerStats.maxHp;
                if (pWidth > 0) {
                    RECT pBar = {30, 95, 30 + pWidth, 105};
                    FillRect(memDC, &pBar, hpP);
                }

                HBRUSH heatBg = CreateSolidBrush(RGB(50, 0, 0));
                HBRUSH heatP = CreateSolidBrush(RGB(255, 165, 0));
                RECT pHeatBg = {30, 108, 230, 113};
                FillRect(memDC, &pHeatBg, heatBg);
                int pHeatW = (playerStats.heat * 200) / playerStats.maxHeat;
                if (pHeatW > 200) pHeatW = 200;
                if (pHeatW > 0) {
                    RECT pHeatBar = {30, 108, 30 + pHeatW, 113};
                    FillRect(memDC, &pHeatBar, heatP);
                }
                
                const char* pw[] = {
                    "  [==]  ",
                    " /|  |\\ ",
                    "/ |__| \\",
                    "  |  |  ",
                    " /    \\ ",
                    "/      \\"
                };
                for (int i=0; i<6; i++) {
                    TextOutA(memDC, 80, 120 + i*14, pw[i], lstrlenA(pw[i]));
                }
                
                // Enemy Mech Stats
                char bufE[64];
                wsprintfA(bufE, "Enemy Mech HP: %d | HEAT: %d", enemyStats.hp, enemyStats.heat);
                SetTextColor(memDC, RGB(255, 0, 0));
                TextOutA(memDC, 350, 70, bufE, lstrlenA(bufE));
                
                HBRUSH hpE = CreateSolidBrush(RGB(255, 0, 0));
                RECT eBarBg = {350, 95, 550, 105};
                FillRect(memDC, &eBarBg, hpBg);
                int eWidth = (enemyStats.hp * 200) / enemyStats.maxHp;
                if (eWidth > 0) {
                    RECT eBar = {350, 95, 350 + eWidth, 105};
                    FillRect(memDC, &eBar, hpE);
                }

                RECT eHeatBg = {350, 108, 550, 113};
                FillRect(memDC, &eHeatBg, heatBg);
                int eHeatW = (enemyStats.heat * 200) / enemyStats.maxHeat;
                if (eHeatW > 200) eHeatW = 200;
                if (eHeatW > 0) {
                    RECT eHeatBar = {350, 108, 350 + eHeatW, 113};
                    FillRect(memDC, &eHeatBar, heatP);
                }
                
                const char* ew[] = {
                    "  (oo)  ",
                    " /|  |\\ ",
                    "/ |__| \\",
                    "  |  |  ",
                    " /    \\ ",
                    "/      \\"
                };
                for (int i=0; i<6; i++) {
                    TextOutA(memDC, 400, 120 + i*14, ew[i], lstrlenA(ew[i]));
                }
                SetTextColor(memDC, RGB(0, 255, 0)); // Restore color
                
                DeleteObject(hpBg);
                DeleteObject(hpP);
                DeleteObject(hpE);
                DeleteObject(heatBg);
                DeleteObject(heatP);
                
                // Draw Logs
                RECT logBg = {30, 210, 550, 390};
                HBRUSH logBrush = CreateSolidBrush(RGB(0, 20, 0));
                FillRect(memDC, &logBg, logBrush);
                DeleteObject(logBrush);
                
                for (int i = 0; i < logCount; i++) {
                    char logOut[130];
                    wsprintfA(logOut, "> %s", battleLogs[i]);
                    TextOutA(memDC, 40, 215 + (i * 16), logOut, lstrlenA(logOut));
                }
                
                if (gameState == STATE_BATTLE) {
                    char targetText[64];
                    wsprintfA(targetText, "TGT: %s", limbNames[currentTargetLimb]);
                    DrawButton(memDC, &rectTarget, targetText);
                    DrawButton(memDC, &rectAttack, "Attack");
                    DrawButton(memDC, &rectDefend, "Defend");
                } else if (gameState == STATE_POST_BATTLE) {
                    DrawButton(memDC, &rectReturn, "Return to Garage");
                }
            }
            
            // Blit and clean up
            BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, SRCCOPY);
            DeleteObject(hFont);
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
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
    my_srand(GetTickCount());
    
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KMechWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClassA(&wc);

    int width = 616; // Adjust for borders
    int height = 500; 
    
    HWND hwnd = CreateWindowA("KMechWindowClass", "KMech", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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
