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
} MechStats;

MechStats playerStats = { 100, 100, 15, 5 };
MechStats enemyStats = { 80, 80, 12, 3 };

bool isDefending = false;

char battleLogs[15][128];
int logCount = 0;

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
    int effectiveDef = isDefending ? (playerStats.def * 2) : playerStats.def;
    int dmg = (enemyStats.atk - effectiveDef) + (my_rand() % 4);
    if (dmg < 1) dmg = 1;
    playerStats.hp -= dmg;
    
    char buf[128];
    wsprintfA(buf, "Enemy attacks! Took %d damage.", dmg);
    addLog(buf);

    if (playerStats.hp <= 0) {
        playerStats.hp = 0;
        addLog("CRITICAL DAMAGE. MECH DESTROYED. DEFEAT.");
        gameState = STATE_POST_BATTLE;
        playerVictory = false;
    }
}

void ActionAttack() {
    int dmg = (playerStats.atk - enemyStats.def) + (my_rand() % 5);
    if (dmg < 1) dmg = 1;
    enemyStats.hp -= dmg;

    char buf[128];
    wsprintfA(buf, "You attack! Dealt %d damage.", dmg);
    addLog(buf);

    if (enemyStats.hp <= 0) {
        enemyStats.hp = 0;
        addLog("ENEMY DESTROYED. VICTORY.");
        gameState = STATE_POST_BATTLE;
        playerVictory = true;
        return;
    }

    EnemyTurn();
}

void ActionDefend() {
    isDefending = true;
    addLog("You brace for impact (Defending).");
    EnemyTurn();
    isDefending = false;
}

void StartBattle() {
    enemyStats.maxHp = 80;
    enemyStats.hp = 80;
    enemyStats.atk = 12;
    enemyStats.def = 3;
    isDefending = false;
    clearLogs();
    addLog("Enemy mech detected! Engaging...");
    gameState = STATE_BATTLE;
}

void ReturnToGarage() {
    playerStats.hp = playerStats.maxHp;
    gameState = STATE_GARAGE;
}

// Button areas
RECT rectDeploy = { 200, 360, 400, 400 };
RECT rectAttack = { 100, 400, 250, 440 };
RECT rectDefend = { 350, 400, 500, 440 };
RECT rectReturn = { 200, 400, 400, 440 };

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
                }
            } else if (gameState == STATE_BATTLE) {
                if (PtInRectLocal(&rectAttack, x, y)) {
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
                
                char buf[128];
                wsprintfA(buf, "HP: %d/%d   ATK: %d   DEF: %d", playerStats.hp, playerStats.maxHp, playerStats.atk, playerStats.def);
                RECT statsRect = {0, 100, 600, 140};
                DrawTextA(memDC, buf, -1, &statsRect, DT_CENTER | DT_SINGLELINE);
                
                RECT msgRect = {0, 150, 600, 190};
                DrawTextA(memDC, "Welcome, Pilot. Your mech is ready for deployment.", -1, &msgRect, DT_CENTER | DT_SINGLELINE);
                
                DrawButton(memDC, &rectDeploy, "Deploy to Battle");
            } else {
                RECT titleRect = {0, 20, 600, 60};
                DrawTextA(memDC, "COMBAT ZONE", -1, &titleRect, DT_CENTER | DT_SINGLELINE);
                
                // Player Mech Stats
                char bufP[64];
                wsprintfA(bufP, "Player Mech HP: %d", playerStats.hp);
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
                
                const char* pw[] = {
                    "  [==]  ",
                    " /|  |\\ ",
                    "/ |__| \\",
                    "  |  |  ",
                    " /    \\ ",
                    "/      \\"
                };
                for (int i=0; i<6; i++) {
                    TextOutA(memDC, 80, 115 + i*14, pw[i], lstrlenA(pw[i]));
                }
                
                // Enemy Mech Stats
                char bufE[64];
                wsprintfA(bufE, "Enemy Mech HP: %d", enemyStats.hp);
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
                
                const char* ew[] = {
                    "  (oo)  ",
                    " /|  |\\ ",
                    "/ |__| \\",
                    "  |  |  ",
                    " /    \\ ",
                    "/      \\"
                };
                for (int i=0; i<6; i++) {
                    TextOutA(memDC, 400, 115 + i*14, ew[i], lstrlenA(ew[i]));
                }
                SetTextColor(memDC, RGB(0, 255, 0)); // Restore color
                
                DeleteObject(hpBg);
                DeleteObject(hpP);
                DeleteObject(hpE);
                
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
