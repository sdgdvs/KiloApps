#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

#define MAX_ROWS 40
#define MAX_COLS 40
int rows = 10;
int cols = 10;
int mines = 10;
#define CELL_SIZE 22
#define HEADER_HEIGHT 68

// Bitmasks for grid
#define CELL_MINE     0x01
#define CELL_REVEALED 0x02
#define CELL_FLAGGED  0x04
#define CELL_QUESTION 0x08
#define CELL_CHEST    0x10

int grid[MAX_ROWS][MAX_COLS];
int gameOver = 0;
int initialized = 0;
int timeElapsed = 0;
int bestTimes[3] = {999, 999, 999};
int bestRush = 0;
int currentDiff = 0; // 0=Easy, 1=Medium, 2=Hard, 3=Rush
int flagsPlaced = 0;
int campaignMode = 0;
int campaignLevel = 1;

// Power-ups
int shields = 0;   // Blast Shield (S key)
int detectors = 0; // Detector / Flag-Bot (D key)
int sonars = 0;    // Sonar / Radar Scan (R key)

// Rush & Speedrun Mode
int rushMode = 0;
int rushTime = 0;
int rushScore = 0;
int isSpeedrun = 0;
int speedrunTime = 0;

// Rapid-Clear Combo System
int comboCount = 0;
int comboMultiplier = 1;
DWORD lastRevealTick = 0;

// Status Notification Banner
char statusMsg[128] = {0};
DWORD statusMsgTime = 0;

int totalPlayed = 0;
int totalWins = 0;
HWND mainHwnd = NULL;
int mouseCellPressed = 0;

// Particle System (Explosions & Treasure Bursts)
typedef struct {
    float x, y;
    float vx, vy;
    float life;
    float maxLife;
    float size;
    COLORREF color;
} Particle;

#define MAX_PARTICLES 200
Particle particles[MAX_PARTICLES];
int particleCount = 0;

static unsigned int seed = 0;

int my_rand() {
    seed = seed * 214013 + 2531011;
    return (seed >> 16) & 0x7FFF;
}

static int parse_int(const char** p) {
    while (**p && (**p < '0' || **p > '9')) (*p)++;
    if (!**p) return 0;
    int val = 0;
    while (**p >= '0' && **p <= '9') {
        val = val * 10 + (**p - '0');
        (*p)++;
    }
    return val;
}

void SpawnExplosion(float cx, float cy) {
    COLORREF colors[5] = {
        RGB(255, 60, 20),   // Fire Red
        RGB(255, 200, 40),  // Gold Spark
        RGB(255, 120, 0),   // Flame Ember
        RGB(80, 80, 90),    // Dark Smoke
        RGB(240, 240, 240)  // Debris
    };
    for (int i = 0; i < 35; i++) {
        if (particleCount < MAX_PARTICLES) {
            Particle* p = &particles[particleCount++];
            p->x = cx; p->y = cy;
            p->vx = (float)((my_rand() % 100) - 50) / 10.0f;
            p->vy = (float)((my_rand() % 100) - 50) / 10.0f - 1.2f;
            p->maxLife = 15.0f + (float)(my_rand() % 25);
            p->life = p->maxLife;
            p->size = 2.0f + (float)(my_rand() % 4);
            p->color = colors[my_rand() % 5];
        }
    }
}

void SpawnTreasureFX(float cx, float cy) {
    COLORREF colors[4] = {
        RGB(255, 215, 0),   // Gold
        RGB(255, 235, 120), // Light Gold
        RGB(56, 189, 248),  // Magic Cyan
        RGB(255, 255, 255)  // White Sparkle
    };
    for (int i = 0; i < 30; i++) {
        if (particleCount < MAX_PARTICLES) {
            Particle* p = &particles[particleCount++];
            p->x = cx; p->y = cy;
            p->vx = (float)((my_rand() % 100) - 50) / 8.0f;
            p->vy = (float)((my_rand() % 100) - 50) / 8.0f - 2.0f;
            p->maxLife = 20.0f + (float)(my_rand() % 20);
            p->life = p->maxLife;
            p->size = 2.5f + (float)(my_rand() % 3);
            p->color = colors[my_rand() % 4];
        }
    }
}

void UpdateParticles() {
    int active = 0;
    for (int i = 0; i < particleCount; i++) {
        Particle* p = &particles[i];
        if (p->life > 0) {
            p->x += p->vx;
            p->y += p->vy;
            p->vy += 0.15f; // gravity
            p->vx *= 0.96f; // drag
            p->life -= 1.0f;
            particles[active++] = *p;
        }
    }
    particleCount = active;
}

void DrawParticles(HDC hdc) {
    for (int i = 0; i < particleCount; i++) {
        Particle* p = &particles[i];
        if (p->life > 0) {
            int r = (int)p->size;
            HBRUSH hbr = CreateSolidBrush(p->color);
            HPEN hPen = CreatePen(PS_SOLID, 1, p->color);
            HGDIOBJ oldBr = SelectObject(hdc, hbr);
            HGDIOBJ oldPen = SelectObject(hdc, hPen);
            Ellipse(hdc, (int)(p->x - r), (int)(p->y - r), (int)(p->x + r), (int)(p->y + r));
            SelectObject(hdc, oldBr);
            SelectObject(hdc, oldPen);
            DeleteObject(hbr);
            DeleteObject(hPen);
        }
    }
}

void LoadBest() {
    HANDLE hFile = CreateFileA("kmines_scores.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[128] = {0};
        DWORD bytesRead;
        if (ReadFile(hFile, buf, sizeof(buf)-1, &bytesRead, NULL)) {
            const char* ptr = buf;
            bestTimes[0] = parse_int(&ptr);
            bestTimes[1] = parse_int(&ptr);
            bestTimes[2] = parse_int(&ptr);
            totalPlayed = parse_int(&ptr);
            totalWins = parse_int(&ptr);
            bestRush = parse_int(&ptr);
        }
        CloseHandle(hFile);
    }
}

void SaveBest() {
    HANDLE hFile = CreateFileA("kmines_scores.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[128];
        wsprintfA(buf, "%d %d %d %d %d %d", bestTimes[0], bestTimes[1], bestTimes[2], totalPlayed, totalWins, bestRush);
        DWORD bytesWritten;
        int len = 0; while(buf[len]) len++;
        WriteFile(hFile, buf, len, &bytesWritten, NULL);
        CloseHandle(hFile);
    }
}

typedef struct {
    int r, c, m, chests, speedrun;
} StageDef;

StageDef campaignStages[20] = {
    {  8,  8,   8, 1,   0 }, // Stage 1
    {  9,  9,  12, 0,  60 }, // Stage 2 (Speedrun 60s)
    { 10, 10,  15, 2,   0 }, // Stage 3
    { 11, 11,  20, 0,  75 }, // Stage 4 (Speedrun 75s)
    { 12, 12,  25, 2,   0 }, // Stage 5
    { 13, 13,  30, 0,  90 }, // Stage 6 (Speedrun 90s)
    { 14, 14,  35, 3,   0 }, // Stage 7
    { 15, 15,  42, 0, 105 }, // Stage 8 (Speedrun 105s)
    { 16, 16,  50, 3,   0 }, // Stage 9
    { 17, 17,  58, 0, 120 }, // Stage 10 (Speedrun 120s)
    { 18, 18,  66, 3,   0 }, // Stage 11
    { 19, 19,  75, 0, 135 }, // Stage 12 (Speedrun 135s)
    { 20, 20,  85, 4,   0 }, // Stage 13
    { 21, 21,  95, 0, 150 }, // Stage 14 (Speedrun 150s)
    { 22, 22, 105, 4,   0 }, // Stage 15
    { 22, 22, 115, 0, 160 }, // Stage 16 (Speedrun 160s)
    { 23, 23, 125, 4,   0 }, // Stage 17
    { 23, 23, 135, 0, 170 }, // Stage 18 (Speedrun 170s)
    { 24, 24, 145, 5,   0 }, // Stage 19
    { 24, 24, 160, 5, 200 }  // Stage 20 Boss Speedrun (200s + Chests)
};

int chestsToPlace = 0;

void InitGame(int firstClickX, int firstClickY) {
    memset(grid, 0, sizeof(grid));
    gameOver = 0;
    if (!rushMode && !isSpeedrun) timeElapsed = 0;
    flagsPlaced = 0;
    comboCount = 0;
    comboMultiplier = 1;
    lastRevealTick = 0;
    seed = GetTickCount();

    int placed = 0;
    while (placed < mines) {
        int r = my_rand() % rows;
        int c = my_rand() % cols;
        if ((grid[r][c] & CELL_MINE) == 0) {
            if (r < firstClickY - 1 || r > firstClickY + 1 || c < firstClickX - 1 || c > firstClickX + 1) {
                grid[r][c] |= CELL_MINE;
                placed++;
            }
        }
    }

    // Hide Treasure Chests
    int cPlaced = 0;
    while (cPlaced < chestsToPlace) {
        int r = my_rand() % rows;
        int c = my_rand() % cols;
        if ((grid[r][c] & (CELL_MINE | CELL_CHEST)) == 0) {
            if (r < firstClickY - 1 || r > firstClickY + 1 || c < firstClickX - 1 || c > firstClickX + 1) {
                grid[r][c] |= CELL_CHEST;
                cPlaced++;
            }
        }
    }

    initialized = 1;
    totalPlayed++;
    SaveBest();
    SetTimer(mainHwnd, 1, 1000, NULL);
    SetTimer(mainHwnd, 2, 33, NULL);
}

void InitCampaignLevel(HWND hwnd) {
    if (campaignLevel >= 1 && campaignLevel <= 20) {
        StageDef s = campaignStages[campaignLevel - 1];
        rows = s.r; cols = s.c; mines = s.m; chestsToPlace = s.chests;
        if (s.speedrun > 0) {
            isSpeedrun = 1;
            speedrunTime = s.speedrun;
        } else {
            isSpeedrun = 0;
            speedrunTime = 0;
        }
        shields = campaignLevel >= 5 ? (campaignLevel >= 11 ? 3 : 2) : 1;
        detectors = 2 + (campaignLevel / 4);
        sonars = 1 + (campaignLevel / 3);
    } else {
        campaignMode = 0;
        rows = 10; cols = 10; mines = 10; currentDiff = 0;
        shields = 0; detectors = 0; sonars = 0; isSpeedrun = 0; chestsToPlace = 0;
        MessageBoxA(hwnd, "Campaign Complete! You cleared all 20 Stages!", "VICTORY!", MB_OK);
    }

    initialized = 0; gameOver = 0; timeElapsed = 0; flagsPlaced = 0; memset(grid, 0, sizeof(grid));
    void ResizeWindow(HWND);
    ResizeWindow(hwnd);
    InvalidateRect(hwnd, NULL, TRUE);
}

int CountMines(int r, int c) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int nr = r + i, nc = c + j;
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                if (grid[nr][nc] & CELL_MINE) count++;
            }
        }
    }
    return count;
}

int CheckWin();

void Reveal(int r, int c) {
    if (r < 0 || r >= rows || c < 0 || c >= cols) return;
    if (grid[r][c] & (CELL_REVEALED | CELL_FLAGGED)) return;
    
    grid[r][c] |= CELL_REVEALED;

    // Rapid-Clear Combo logic
    DWORD now = GetTickCount();
    if (lastRevealTick > 0 && (now - lastRevealTick <= 2000)) {
        comboCount++;
    } else {
        comboCount = 1;
    }
    lastRevealTick = now;
    comboMultiplier = 1 + (comboCount / 3);
    if (comboMultiplier > 8) comboMultiplier = 8;

    if (rushMode) {
        rushScore += 10 * comboMultiplier;
    }

    // Treasure Chest Uncovered!
    if (grid[r][c] & CELL_CHEST) {
        grid[r][c] &= ~CELL_CHEST;
        int px = c * CELL_SIZE + CELL_SIZE / 2;
        int py = r * CELL_SIZE + HEADER_HEIGHT + CELL_SIZE / 2;
        SpawnTreasureFX((float)px, (float)py);

        int reward = my_rand() % 100;
        if (reward < 35) {
            sonars++;
            wsprintfA(statusMsg, "CHEST: +1 SONAR SCAN! (R)");
        } else if (reward < 70) {
            detectors++;
            wsprintfA(statusMsg, "CHEST: +1 DETECTOR! (D)");
        } else if (reward < 90) {
            shields++;
            wsprintfA(statusMsg, "CHEST: +1 BLAST SHIELD! (S)");
        } else {
            if (isSpeedrun) {
                speedrunTime += 15;
                wsprintfA(statusMsg, "CHEST: +15 SECONDS!");
            } else {
                rushScore += 500;
                wsprintfA(statusMsg, "CHEST: +500 BONUS SCORE!");
            }
        }
        statusMsgTime = GetTickCount() + 2500;
        Beep(1800, 80); Beep(2400, 120);
    }
    
    if (grid[r][c] & CELL_MINE) {
        int px = c * CELL_SIZE + CELL_SIZE / 2;
        int py = r * CELL_SIZE + HEADER_HEIGHT + CELL_SIZE / 2;
        SpawnExplosion((float)px, (float)py);

        if (shields > 0) {
            shields--;
            grid[r][c] &= ~CELL_MINE;
            mines--;
            wsprintfA(statusMsg, "BLAST SHIELD ABSORBED MINE!");
            statusMsgTime = GetTickCount() + 2000;
            Beep(500, 100);
        } else {
            gameOver = 1;
            KillTimer(mainHwnd, 1);
            Beep(200, 500);
            return;
        }
    }
    
    if (CountMines(r, c) == 0) {
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                Reveal(r + i, c + j);
            }
        }
    }
}

// Power-up Abilities
void UseSonarScan(HWND hwnd) {
    if (sonars <= 0 || !initialized || gameOver) return;
    int targetR = -1, targetC = -1;
    for (int i = 0; i < 1000; i++) {
        int r = my_rand() % rows;
        int c = my_rand() % cols;
        if (!(grid[r][c] & CELL_REVEALED)) {
            targetR = r; targetC = c;
            break;
        }
    }
    if (targetR != -1) {
        sonars--;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                int nr = targetR + i, nc = targetC + j;
                if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                    if (grid[nr][nc] & CELL_MINE) {
                        if (!(grid[nr][nc] & CELL_FLAGGED)) {
                            grid[nr][nc] |= CELL_FLAGGED;
                            flagsPlaced++;
                        }
                    } else {
                        if (!(grid[nr][nc] & CELL_REVEALED) && !(grid[nr][nc] & CELL_FLAGGED)) {
                            Reveal(nr, nc);
                        }
                    }
                }
            }
        }
        Beep(2000, 120);
        wsprintfA(statusMsg, "SONAR SCAN ACTIVATED! (3x3)");
        statusMsgTime = GetTickCount() + 2000;
        InvalidateRect(hwnd, NULL, FALSE);
        if (CheckWin() && !gameOver) {
            gameOver = 1;
            KillTimer(hwnd, 1);
            Beep(1500, 300);
            totalWins++; SaveBest();
            MessageBoxA(hwnd, "Stage Complete! Next Stage...", "Campaign", MB_OK);
            campaignLevel++; InitCampaignLevel(hwnd);
        }
    }
}

void UseDetectorBot(HWND hwnd) {
    if (detectors <= 0 || !initialized || gameOver) return;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if ((grid[r][c] & CELL_MINE) && !(grid[r][c] & CELL_FLAGGED) && !(grid[r][c] & CELL_REVEALED)) {
                grid[r][c] |= CELL_FLAGGED;
                flagsPlaced++;
                detectors--;
                Beep(1400, 100);
                wsprintfA(statusMsg, "FLAG-BOT AUTO-FLAGGED MINE!");
                statusMsgTime = GetTickCount() + 2000;
                InvalidateRect(hwnd, NULL, FALSE);
                return;
            }
        }
    }
}

void UseBlastShield(HWND hwnd) {
    if (shields <= 0 || !initialized || gameOver) return;
    wsprintfA(statusMsg, "BLAST SHIELD ACTIVE (%d CHARGES)", shields);
    statusMsgTime = GetTickCount() + 2000;
    Beep(1200, 100);
}

// Graphics Rendering
void DrawSmiley(HDC hdc, int cx, int cy, int size, int state) {
    int r = size / 2;
    HBRUSH hbrFace = CreateSolidBrush(RGB(255, 215, 0));
    HPEN hPenOutline = CreatePen(PS_SOLID, 2, RGB(180, 130, 0));
    HGDIOBJ oldBr = SelectObject(hdc, hbrFace);
    HGDIOBJ oldPen = SelectObject(hdc, hPenOutline);
    
    Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
    
    HPEN hPenShine = CreatePen(PS_SOLID, 1, RGB(255, 255, 200));
    SelectObject(hdc, hPenShine);
    Arc(hdc, cx - r + 3, cy - r + 3, cx + r - 3, cy + r - 3, cx + r, cy - r, cx - r, cy - r);
    DeleteObject(hPenShine);

    if (state == 1) { // Shocked
        HBRUSH hbrEye = CreateSolidBrush(RGB(20, 20, 20));
        SelectObject(hdc, hbrEye);
        SelectObject(hdc, GetStockObject(NULL_PEN));
        Ellipse(hdc, cx - 6, cy - 6, cx - 2, cy - 2);
        Ellipse(hdc, cx + 2, cy - 6, cx + 6, cy - 2);
        Ellipse(hdc, cx - 4, cy + 1, cx + 4, cy + 9);
        DeleteObject(hbrEye);
    } else if (state == 2) { // Dead
        HPEN hPenX = CreatePen(PS_SOLID, 2, RGB(60, 40, 20));
        SelectObject(hdc, hPenX);
        MoveToEx(hdc, cx - 7, cy - 6, NULL); LineTo(hdc, cx - 3, cy - 2);
        MoveToEx(hdc, cx - 3, cy - 6, NULL); LineTo(hdc, cx - 7, cy - 2);
        MoveToEx(hdc, cx + 3, cy - 6, NULL); LineTo(hdc, cx + 7, cy - 2);
        MoveToEx(hdc, cx + 7, cy - 6, NULL); LineTo(hdc, cx + 3, cy - 2);
        Arc(hdc, cx - 6, cy + 2, cx + 6, cy + 12, cx + 6, cy + 6, cx - 6, cy + 6);
        DeleteObject(hPenX);
    } else if (state == 3) { // Cool Win
        HBRUSH hbrGlass = CreateSolidBrush(RGB(30, 30, 35));
        HPEN hPenGlass = CreatePen(PS_SOLID, 1, RGB(10, 10, 10));
        SelectObject(hdc, hbrGlass);
        SelectObject(hdc, hPenGlass);
        POINT ptL[4] = { {cx - 9, cy - 6}, {cx - 1, cy - 6}, {cx - 2, cy + 1}, {cx - 8, cy + 1} };
        POINT ptR[4] = { {cx + 1, cy - 6}, {cx + 9, cy - 6}, {cx + 8, cy + 1}, {cx + 2, cy + 1} };
        Polygon(hdc, ptL, 4); Polygon(hdc, ptR, 4);
        MoveToEx(hdc, cx - 1, cy - 4, NULL); LineTo(hdc, cx + 1, cy - 4);
        DeleteObject(hbrGlass); DeleteObject(hPenGlass);
        HPEN hPenSmirk = CreatePen(PS_SOLID, 2, RGB(40, 30, 0));
        SelectObject(hdc, hPenSmirk);
        Arc(hdc, cx - 5, cy + 1, cx + 6, cy + 8, cx - 5, cy + 4, cx + 5, cy + 6);
        DeleteObject(hPenSmirk);
    } else { // Idle
        HBRUSH hbrEye = CreateSolidBrush(RGB(20, 20, 20));
        SelectObject(hdc, hbrEye); SelectObject(hdc, GetStockObject(NULL_PEN));
        Ellipse(hdc, cx - 6, cy - 6, cx - 2, cy - 2);
        Ellipse(hdc, cx + 2, cy - 6, cx + 6, cy - 2);
        DeleteObject(hbrEye);
        HPEN hPenSmile = CreatePen(PS_SOLID, 2, RGB(40, 30, 0));
        SelectObject(hdc, hPenSmile);
        Arc(hdc, cx - 6, cy - 3, cx + 6, cy + 7, cx + 6, cy + 3, cx - 6, cy + 3);
        DeleteObject(hPenSmile);
    }

    SelectObject(hdc, oldBr); SelectObject(hdc, oldPen);
    DeleteObject(hbrFace); DeleteObject(hPenOutline);
}

void DrawMineSprite(HDC hdc, int x, int y, int size, int isDetonated, DWORD tick) {
    int cx = x + size / 2, cy = y + size / 2, r = size / 3;

    if (isDetonated) {
        HBRUSH hbrGlow = CreateSolidBrush(RGB(240, 50, 50));
        RECT rcCell = { x, y, x + size, y + size };
        FillRect(hdc, &rcCell, hbrGlow);
        DeleteObject(hbrGlow);
    }

    HPEN hPenSpike = CreatePen(PS_SOLID, 2, RGB(30, 30, 35));
    HGDIOBJ oldPen = SelectObject(hdc, hPenSpike);
    MoveToEx(hdc, cx - r - 2, cy, NULL); LineTo(hdc, cx + r + 2, cy);
    MoveToEx(hdc, cx, cy - r - 2, NULL); LineTo(hdc, cx, cy + r + 2);
    MoveToEx(hdc, cx - r + 1, cy - r + 1, NULL); LineTo(hdc, cx + r - 1, cy + r - 1);
    MoveToEx(hdc, cx + r - 1, cy - r + 1, NULL); LineTo(hdc, cx - r + 1, cy + r - 1);
    DeleteObject(hPenSpike);

    HBRUSH hbrBody = CreateSolidBrush(RGB(40, 44, 52));
    HPEN hPenBody = CreatePen(PS_SOLID, 1, RGB(15, 18, 22));
    SelectObject(hdc, hbrBody); SelectObject(hdc, hPenBody);
    Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
    DeleteObject(hbrBody); DeleteObject(hPenBody);

    SelectObject(hdc, oldPen);
}

void DrawChestSprite(HDC hdc, int x, int y, int size) {
    int cx = x + size / 2, cy = y + size / 2;
    HBRUSH hbrGold = CreateSolidBrush(RGB(245, 180, 30));
    HPEN hPenDark = CreatePen(PS_SOLID, 1, RGB(120, 80, 10));
    HGDIOBJ oldBr = SelectObject(hdc, hbrGold);
    HGDIOBJ oldPen = SelectObject(hdc, hPenDark);
    
    // Chest Box
    Rectangle(hdc, cx - 7, cy - 4, cx + 7, cy + 6);
    // Lid
    HBRUSH hbrLid = CreateSolidBrush(RGB(255, 215, 0));
    SelectObject(hdc, hbrLid);
    RoundRect(hdc, cx - 8, cy - 8, cx + 8, cy - 2, 4, 4);
    DeleteObject(hbrLid);
    // Lock
    HBRUSH hbrLock = CreateSolidBrush(RGB(50, 50, 60));
    SelectObject(hdc, hbrLock);
    Ellipse(hdc, cx - 2, cy - 3, cx + 2, cy + 1);
    DeleteObject(hbrLock);

    SelectObject(hdc, oldBr); SelectObject(hdc, oldPen);
    DeleteObject(hbrGold); DeleteObject(hPenDark);
}

void DrawFlagSprite(HDC hdc, int x, int y, int size, DWORD tick) {
    int cx = x + size / 2;
    HBRUSH hbrBase = CreateSolidBrush(RGB(60, 60, 70));
    HPEN hPenBase = CreatePen(PS_SOLID, 1, RGB(30, 30, 35));
    HGDIOBJ oldBr = SelectObject(hdc, hbrBase);
    HGDIOBJ oldPen = SelectObject(hdc, hPenBase);
    Ellipse(hdc, cx - 6, y + size - 6, cx + 6, y + size - 2);
    DeleteObject(hbrBase); DeleteObject(hPenBase);

    HPEN hPenPole = CreatePen(PS_SOLID, 2, RGB(200, 205, 215));
    SelectObject(hdc, hPenPole);
    MoveToEx(hdc, cx - 3, y + 4, NULL); LineTo(hdc, cx - 3, y + size - 4);
    DeleteObject(hPenPole);

    int waveOffset = ((tick / 150) % 2) * 2;
    POINT pts[3] = { { cx - 2, y + 5 }, { cx + size / 2 + 3 + waveOffset, y + 9 }, { cx - 2, y + 15 } };
    HBRUSH hbrCloth = CreateSolidBrush(RGB(240, 45, 65));
    HPEN hPenCloth = CreatePen(PS_SOLID, 1, RGB(160, 20, 35));
    SelectObject(hdc, hbrCloth); SelectObject(hdc, hPenCloth);
    Polygon(hdc, pts, 3);
    DeleteObject(hbrCloth); DeleteObject(hPenCloth);

    SelectObject(hdc, oldBr); SelectObject(hdc, oldPen);
}

void DrawQuestionSprite(HDC hdc, int x, int y, int size) {
    RECT rc = { x, y, x + size, y + size };
    HFONT hFont = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    HGDIOBJ oldFont = SelectObject(hdc, hFont);
    SetTextColor(hdc, RGB(56, 189, 248));
    DrawTextA(hdc, "?", 1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont); DeleteObject(hFont);
}

void Draw3DTile(HDC hdc, int x, int y, int size, int isRevealed, int isPressed) {
    RECT rc = { x, y, x + size, y + size };
    if (isRevealed) {
        HBRUSH hbrRev = CreateSolidBrush(RGB(24, 28, 40));
        FillRect(hdc, &rc, hbrRev); DeleteObject(hbrRev);
        HPEN hDarkInner = CreatePen(PS_SOLID, 1, RGB(15, 18, 26));
        HGDIOBJ oldPen = SelectObject(hdc, hDarkInner);
        MoveToEx(hdc, x, y + size - 1, NULL); LineTo(hdc, x, y); LineTo(hdc, x + size - 1, y);
        SelectObject(hdc, oldPen); DeleteObject(hDarkInner);
    } else {
        HBRUSH hbrUnrev = CreateSolidBrush(RGB(65, 75, 100));
        FillRect(hdc, &rc, hbrUnrev); DeleteObject(hbrUnrev);
        HPEN hWhiteHi = CreatePen(PS_SOLID, 2, RGB(140, 155, 190));
        HPEN hDarkLo  = CreatePen(PS_SOLID, 2, RGB(30, 35, 50));
        HGDIOBJ oldPen = SelectObject(hdc, hWhiteHi);
        MoveToEx(hdc, x, y + size - 1, NULL); LineTo(hdc, x, y); LineTo(hdc, x + size - 1, y);
        SelectObject(hdc, hDarkLo);
        LineTo(hdc, x + size - 1, y + size - 1); LineTo(hdc, x - 1, y + size - 1);
        SelectObject(hdc, oldPen);
        DeleteObject(hWhiteHi); DeleteObject(hDarkLo);
    }
}

void DrawBoard(HWND hwnd, HDC hdc) {
    DWORD tick = GetTickCount();

    RECT rcFull = { 0, 0, cols * CELL_SIZE, rows * CELL_SIZE + HEADER_HEIGHT };
    HBRUSH hbrBg = CreateSolidBrush(RGB(18, 20, 29));
    FillRect(hdc, &rcFull, hbrBg); DeleteObject(hbrBg);

    RECT rcHeader = { 0, 0, cols * CELL_SIZE, HEADER_HEIGHT };
    HBRUSH hbrHeader = CreateSolidBrush(RGB(30, 35, 50));
    FillRect(hdc, &rcHeader, hbrHeader); DeleteObject(hbrHeader);

    HPEN hHeaderBorder = CreatePen(PS_SOLID, 2, RGB(60, 70, 95));
    HGDIOBJ oldPen = SelectObject(hdc, hHeaderBorder);
    MoveToEx(hdc, 0, HEADER_HEIGHT - 1, NULL); LineTo(hdc, cols * CELL_SIZE, HEADER_HEIGHT - 1);
    SelectObject(hdc, oldPen); DeleteObject(hHeaderBorder);

    HFONT hLcdFont = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Courier New");
    HGDIOBJ oldFont = SelectObject(hdc, hLcdFont);
    SetBkMode(hdc, OPAQUE); SetBkColor(hdc, RGB(10, 10, 15));

    char szMines[16];
    wsprintfA(szMines, "%03d", mines - flagsPlaced);
    RECT rcMineBox = { 10, 10, 60, 38 };
    HBRUSH hbrLcd = CreateSolidBrush(RGB(10, 10, 15));
    FillRect(hdc, &rcMineBox, hbrLcd);
    SetTextColor(hdc, RGB(247, 118, 142));
    DrawTextA(hdc, szMines, -1, &rcMineBox, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    char szTime[16];
    int displayTime = rushMode ? rushTime : (isSpeedrun ? speedrunTime : timeElapsed);
    wsprintfA(szTime, "%03d", displayTime);
    RECT rcTimeBox = { cols * CELL_SIZE - 60, 10, cols * CELL_SIZE - 10, 38 };
    FillRect(hdc, &rcTimeBox, hbrLcd);
    SetTextColor(hdc, isSpeedrun ? RGB(250, 204, 21) : RGB(122, 162, 247));
    DrawTextA(hdc, szTime, -1, &rcTimeBox, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DeleteObject(hbrLcd);

    int faceCx = cols * CELL_SIZE / 2;
    int faceCy = 24;
    int smileyState = 0;
    if (gameOver) smileyState = 2;
    else if (CheckWin()) smileyState = 3;
    else if (mouseCellPressed) smileyState = 1;
    DrawSmiley(hdc, faceCx, faceCy, 30, smileyState);

    // Mode & Power-up Sub-Header text
    SetBkMode(hdc, TRANSPARENT);
    HFONT hSubFont = CreateFontA(11, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    SelectObject(hdc, hSubFont);
    RECT rcMode = { 0, 42, cols * CELL_SIZE, HEADER_HEIGHT - 2 };
    char szMode[128];
    if (tick < statusMsgTime) {
        SetTextColor(hdc, RGB(255, 235, 120));
        DrawTextA(hdc, statusMsg, -1, &rcMode, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    } else {
        SetTextColor(hdc, RGB(180, 190, 210));
        if (rushMode) {
            wsprintfA(szMode, "RUSH MODE - Score:%d (x%d Combo!) Best:%d", rushScore, comboMultiplier, bestRush);
        } else if (campaignMode) {
            wsprintfA(szMode, "STAGE %d/20 %s| Sh[S]:%d Det[D]:%d Son[R]:%d", campaignLevel, isSpeedrun ? "[TIME] " : "", shields, detectors, sonars);
        } else {
            wsprintfA(szMode, "1-Ez 2-Md 3-Hd C-Camp(20) 4-Rush | Sh[S]:%d Det[D]:%d Son[R]:%d", shields, detectors, sonars);
        }
        DrawTextA(hdc, szMode, -1, &rcMode, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    DeleteObject(hSubFont);

    // Grid Cells
    HFONT hNumFont = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    SelectObject(hdc, hNumFont);

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int x = c * CELL_SIZE;
            int y = r * CELL_SIZE + HEADER_HEIGHT;
            RECT rcCell = { x, y, x + CELL_SIZE, y + CELL_SIZE };
            
            int isRev = (grid[r][c] & CELL_REVEALED);
            Draw3DTile(hdc, x, y, CELL_SIZE, isRev, 0);

            if (isRev) {
                if (grid[r][c] & CELL_MINE) {
                    DrawMineSprite(hdc, x, y, CELL_SIZE, 1, tick);
                } else {
                    int m = CountMines(r, c);
                    if (m > 0) {
                        char szNum[2] = { m + '0', 0 };
                        COLORREF numColors[8] = {
                            RGB(56, 189, 248),  RGB(74, 222, 128),  RGB(248, 113, 113), RGB(192, 132, 252),
                            RGB(250, 204, 21),  RGB(45, 212, 191),  RGB(226, 232, 240), RGB(148, 163, 184)
                        };
                        SetTextColor(hdc, numColors[m - 1]);
                        DrawTextA(hdc, szNum, 1, &rcCell, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    }
                }
            } else {
                if (grid[r][c] & CELL_FLAGGED) {
                    DrawFlagSprite(hdc, x, y, CELL_SIZE, tick);
                } else if (grid[r][c] & CELL_QUESTION) {
                    DrawQuestionSprite(hdc, x, y, CELL_SIZE);
                }
            }
        }
    }
    DeleteObject(hNumFont);
    SelectObject(hdc, oldFont);
    DeleteObject(hLcdFont);

    DrawParticles(hdc);
}

int CheckWin() {
    int revealed = 0;
    for(int r=0; r<rows; r++) {
        for(int c=0; c<cols; c++) {
            if (grid[r][c] & CELL_REVEALED) revealed++;
        }
    }
    return (revealed == (rows * cols - mines));
}

void HandleReveal(HWND hwnd, int x, int y) {
    if (gameOver || !initialized) return;
    if (grid[y][x] & CELL_FLAGGED) return;

    if (grid[y][x] & CELL_REVEALED) {
        int flagged = 0;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                int nr = y + i, nc = x + j;
                if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                    if (grid[nr][nc] & CELL_FLAGGED) flagged++;
                }
            }
        }
        if (flagged == CountMines(y, x)) {
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    int nr = y + i, nc = x + j;
                    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                        if (!(grid[nr][nc] & CELL_FLAGGED) && !(grid[nr][nc] & CELL_REVEALED)) {
                            Reveal(nr, nc);
                        }
                    }
                }
            }
        } else {
            return;
        }
    } else {
        Reveal(y, x);
    }

    if (!gameOver) Beep(1000, 20);
    InvalidateRect(hwnd, NULL, FALSE);
    
    if (gameOver) {
        for(int r=0;r<rows;r++) for(int c=0;c<cols;c++) if(grid[r][c]&CELL_MINE) grid[r][c]|=CELL_REVEALED;
        InvalidateRect(hwnd, NULL, FALSE);
        MessageBoxA(hwnd, "Boom! Click Smiley to restart.", "Game Over", MB_OK);
    } else if (CheckWin()) {
        gameOver = 1;
        KillTimer(hwnd, 1);
        Beep(1500, 300);
        totalWins++;
        SaveBest();
        if (!campaignMode && timeElapsed < bestTimes[currentDiff]) {
            bestTimes[currentDiff] = timeElapsed;
            SaveBest();
        }
        if (campaignMode) {
            MessageBoxA(hwnd, "Stage Complete! Next Stage...", "Campaign Victory", MB_OK);
            campaignLevel++;
            InitCampaignLevel(hwnd);
        } else if (rushMode) {
            rushScore += 100 * comboMultiplier;
            rushTime += 15;
            if (rushTime > 999) rushTime = 999;
            if (rushScore > bestRush) bestRush = rushScore;
            SaveBest();
            Beep(1200, 200);
            initialized = 0;
            memset(grid, 0, sizeof(grid));
            gameOver = 0;
            flagsPlaced = 0;
            InvalidateRect(hwnd, NULL, TRUE);
        } else {
            MessageBoxA(hwnd, "You Win! Click Smiley to restart.", "Congratulations", MB_OK);
        }
    }
}

void ResizeWindow(HWND hwnd) {
    RECT rcClient, rcWindow;
    GetClientRect(hwnd, &rcClient);
    GetWindowRect(hwnd, &rcWindow);
    SetWindowPos(hwnd, NULL, 0, 0,
        (rcWindow.right - rcWindow.left) + (cols * CELL_SIZE - (rcClient.right - rcClient.left)),
        (rcWindow.bottom - rcWindow.top) + (rows * CELL_SIZE + HEADER_HEIGHT - (rcClient.bottom - rcClient.top)),
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            mainHwnd = hwnd;
            LoadBest();
            SetTimer(hwnd, 2, 33, NULL);
            break;
        case WM_TIMER:
            if (wParam == 1 && !gameOver && initialized) {
                if (rushMode) {
                    rushTime--;
                    if (rushTime <= 0) {
                        rushTime = 0; gameOver = 1; KillTimer(hwnd, 1); Beep(200, 500);
                        for(int r=0;r<rows;r++) for(int c=0;c<cols;c++) if(grid[r][c]&CELL_MINE) grid[r][c]|=CELL_REVEALED;
                        InvalidateRect(hwnd, NULL, FALSE);
                        MessageBoxA(hwnd, "Time's Up! Click Smiley to restart.", "Game Over", MB_OK);
                    }
                } else if (isSpeedrun) {
                    speedrunTime--;
                    if (speedrunTime <= 0) {
                        speedrunTime = 0; gameOver = 1; KillTimer(hwnd, 1); Beep(200, 500);
                        for(int r=0;r<rows;r++) for(int c=0;c<cols;c++) if(grid[r][c]&CELL_MINE) grid[r][c]|=CELL_REVEALED;
                        InvalidateRect(hwnd, NULL, FALSE);
                        MessageBoxA(hwnd, "Speedrun Time Expired! Click Smiley to restart.", "Stage Failed", MB_OK);
                    }
                } else {
                    timeElapsed++;
                    if (timeElapsed > 999) timeElapsed = 999;
                }
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 2) {
                UpdateParticles();
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            DrawBoard(hwnd, hdc);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_LBUTTONDOWN: {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            
            int faceCx = cols * CELL_SIZE / 2;
            int faceCy = 24;
            if ((mx - faceCx)*(mx - faceCx) + (my - faceCy)*(my - faceCy) <= 256) {
                if (campaignMode) { InitCampaignLevel(hwnd); return 0; }
                if (rushMode) { rushTime = 60; rushScore = 0; }
                initialized = 0; memset(grid, 0, sizeof(grid)); gameOver = 0;
                if (!rushMode && !isSpeedrun) timeElapsed = 0;
                flagsPlaced = 0; shields = 0; detectors = 0; sonars = 0; particleCount = 0;
                InvalidateRect(hwnd, NULL, TRUE);
                return 0;
            }

            if (gameOver) return 0;

            mouseCellPressed = 1;
            InvalidateRect(hwnd, NULL, FALSE);

            int x = mx / CELL_SIZE;
            int y = (my - HEADER_HEIGHT) / CELL_SIZE;
            if (x >= 0 && x < cols && y >= 0 && y < rows && my >= HEADER_HEIGHT) {
                if (!initialized) InitGame(x, y);
                HandleReveal(hwnd, x, y);
            }
            break;
        }
        case WM_LBUTTONUP: {
            mouseCellPressed = 0;
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_RBUTTONDOWN: {
            if (gameOver || !initialized) return 0;
            int x = LOWORD(lParam) / CELL_SIZE;
            int y = (HIWORD(lParam) - HEADER_HEIGHT) / CELL_SIZE;
            if (x >= 0 && x < cols && y >= 0 && y < rows && HIWORD(lParam) >= HEADER_HEIGHT) {
                if (!(grid[y][x] & CELL_REVEALED)) {
                    if (grid[y][x] & CELL_FLAGGED) {
                        grid[y][x] &= ~CELL_FLAGGED;
                        grid[y][x] |= CELL_QUESTION;
                        flagsPlaced--;
                    } else if (grid[y][x] & CELL_QUESTION) {
                        grid[y][x] &= ~CELL_QUESTION;
                    } else {
                        grid[y][x] |= CELL_FLAGGED;
                        flagsPlaced++;
                    }
                    Beep(800, 20);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            break;
        }
        case WM_KEYDOWN:
            if (wParam == '1') { rushMode=0; campaignMode=0; shields=0; detectors=0; sonars=0; isSpeedrun=0; rows=10; cols=10; mines=10; currentDiff=0; initialized=0; gameOver=0; timeElapsed=0; flagsPlaced=0; memset(grid,0,sizeof(grid)); ResizeWindow(hwnd); }
            if (wParam == '2') { rushMode=0; campaignMode=0; shields=0; detectors=0; sonars=0; isSpeedrun=0; rows=16; cols=16; mines=40; currentDiff=1; initialized=0; gameOver=0; timeElapsed=0; flagsPlaced=0; memset(grid,0,sizeof(grid)); ResizeWindow(hwnd); }
            if (wParam == '3') { rushMode=0; campaignMode=0; shields=0; detectors=0; sonars=0; isSpeedrun=0; rows=16; cols=30; mines=99; currentDiff=2; initialized=0; gameOver=0; timeElapsed=0; flagsPlaced=0; memset(grid,0,sizeof(grid)); ResizeWindow(hwnd); }
            if (wParam == '4') { rushMode=1; rushTime=60; rushScore=0; campaignMode=0; shields=0; detectors=0; sonars=0; isSpeedrun=0; rows=10; cols=10; mines=10; currentDiff=3; initialized=0; gameOver=0; timeElapsed=0; flagsPlaced=0; memset(grid,0,sizeof(grid)); ResizeWindow(hwnd); }
            if (wParam == 'C') { rushMode=0; campaignMode = !campaignMode; if (campaignMode) { campaignLevel = 1; InitCampaignLevel(hwnd); } else { campaignMode=0; shields=0; detectors=0; sonars=0; isSpeedrun=0; rows=10; cols=10; mines=10; currentDiff=0; initialized=0; gameOver=0; timeElapsed=0; flagsPlaced=0; memset(grid,0,sizeof(grid)); ResizeWindow(hwnd); } }
            if (wParam == 'R') { UseSonarScan(hwnd); }
            if (wParam == 'D') { UseDetectorBot(hwnd); }
            if (wParam == 'S') { UseBlastShield(hwnd); }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KMinesClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);
    RECT rc = {0, 0, cols * CELL_SIZE, rows * CELL_SIZE + HEADER_HEIGHT};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, FALSE);
    
    HWND hwnd = CreateWindowExA(0, "KMinesClass", "KMines", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
    
    ResizeWindow(hwnd);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
