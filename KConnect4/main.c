#include <windows.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

const char g_szClassName[] = "KConnect4WindowClass";
#define MAX_ROWS 10
#define MAX_COLS 10

int g_rows = 6;
int g_cols = 7;

// Board values: 0=Empty, 1=Red, 2=Yellow, 3=Obstacle (Solid), 4=Crackable Block
int board[MAX_ROWS][MAX_COLS];
int currentPlayer = 1;
bool gameActive = true;
bool isDraw = false;
float winBeamProgress = 0.0f;

// Game modes: 0 = 2 Player, 1 = vs AI, 2 = Campaign, 3 = Speed
int gameMode = 1;
int aiPersonality = 0; // 0=Rookie, 1=Aggressive, 2=Trapper, 3=Grandmaster
int campaignStage = 1;

// Power-ups: 0 = Normal, 1 = Bomb (3x3 clear), 2 = Drill (crush underneath), 3 = Magnet (pull adjacent)
int selectedPowerup = 0;
int p1Bombs = 2, p2Bombs = 2;
int p1Drills = 2, p2Drills = 2;
int p1Magnets = 2, p2Magnets = 2;

// Active Skills: Freeze (F), Hint (H)
int p1Freezes = 1, p2Freezes = 1;
bool isFreezeMode = false;
int frozenCol = -1;
int frozenTurns = 0;
int frozenPlayer = 0;

int hintCol = -1;
int hintTimer = 0;

// Speed mode timer
int turnTimeLeftMs = 7000;

// Animation state
bool isAnimating = false;
int animPlayer = 0;
int animRow = -1;
int animCol = -1;
int animY = 0;
float animY_float = 50.0f;
float animVY = 0.0f;
int animBounceCount = 0;
int animTargetY = 0;
int animType = 0; // 0=normal, 1=bomb, 2=drill, 3=magnet

// Particles
typedef struct {
    float x, y;
    float vx, vy;
    int size;
    float phase;
} DustMote;

#define MAX_DUST 40
DustMote g_dust[MAX_DUST];
bool g_dustInit = false;

typedef struct {
    float x, y;
    float vx, vy;
    float rotation, vRot;
    COLORREF color;
    int size;
    int life;
} Particle;

#define MAX_PARTICLES 200
Particle g_particles[MAX_PARTICLES];
int g_particleCount = 0;
int g_screenShakeTimer = 0;

void SpawnConfetti() {
    COLORREF palette[] = {
        RGB(255, 82, 82), RGB(255, 215, 0), RGB(79, 195, 247),
        RGB(105, 240, 174), RGB(224, 64, 251), RGB(255, 255, 255)
    };
    g_particleCount = MAX_PARTICLES;
    for (int i = 0; i < MAX_PARTICLES; i++) {
        g_particles[i].x = (float)(rand() % 450 + 10);
        g_particles[i].y = (float)(-(rand() % 40));
        g_particles[i].vx = ((float)(rand() % 100) - 50.0f) / 15.0f;
        g_particles[i].vy = ((float)(rand() % 100)) / 25.0f + 2.0f;
        g_particles[i].rotation = ((float)(rand() % 360)) * 3.14159f / 180.0f;
        g_particles[i].vRot = ((float)(rand() % 100) - 50.0f) / 100.0f;
        g_particles[i].color = palette[rand() % 6];
        g_particles[i].size = rand() % 6 + 4;
        g_particles[i].life = rand() % 60 + 60;
    }
}

void SpawnPowerupParticles(int type, int r, int c, HWND hwnd) {
    g_screenShakeTimer = 20; // Trigger procedural screen shake
    RECT rect; GetClientRect(hwnd, &rect);
    int boardW = g_cols * 44 + 10;
    int boardLeft = (rect.right - rect.left - boardW) / 2;
    int boardTop = 50;
    float px = (float)(boardLeft + 5 + c * 44 + 22);
    float py = (float)(boardTop + 5 + r * 44 + 22);

    g_particleCount = MAX_PARTICLES;
    if (type == 1) { // Bomb
        COLORREF firePal[] = {RGB(255, 68, 0), RGB(255, 136, 0), RGB(255, 204, 0)};
        COLORREF smokePal[] = {RGB(85, 85, 85), RGB(50, 50, 50), RGB(30, 30, 30)};
        for (int i = 0; i < MAX_PARTICLES; i++) {
            float angle = ((float)(rand() % 360)) * 3.14159f / 180.0f;
            float speed = ((float)(rand() % 100)) / 10.0f + 2.0f;
            g_particles[i].x = px; g_particles[i].y = py;
            g_particles[i].vx = cosf(angle) * speed;
            g_particles[i].vy = sinf(angle) * speed;
            g_particles[i].rotation = angle;
            g_particles[i].vRot = ((float)(rand() % 100) - 50.0f) / 100.0f;
            if (i < MAX_PARTICLES * 0.6) {
                g_particles[i].color = firePal[rand() % 3];
                g_particles[i].size = rand() % 12 + 6;
                g_particles[i].life = rand() % 30 + 20;
            } else {
                g_particles[i].color = smokePal[rand() % 3];
                g_particles[i].size = rand() % 20 + 10;
                g_particles[i].life = rand() % 40 + 40;
                g_particles[i].vy -= 2.0f; // Smoke rises
            }
        }
    } else if (type == 2) { // Drill
        py += 44.0f;
        COLORREF pal[] = {RGB(0, 255, 255), RGB(128, 222, 234), RGB(255, 255, 255), RGB(0, 100, 100)};
        for (int i = 0; i < MAX_PARTICLES; i++) {
            g_particles[i].x = px + (float)(rand() % 40 - 20); g_particles[i].y = py + (float)(rand() % 20 - 10);
            g_particles[i].vx = ((float)(rand() % 100) - 50.0f) / 5.0f;
            g_particles[i].vy = -(((float)(rand() % 100)) / 10.0f + 4.0f);
            g_particles[i].rotation = ((float)(rand() % 360)) * 3.14159f / 180.0f;
            g_particles[i].vRot = ((float)(rand() % 100) - 50.0f) / 50.0f;
            g_particles[i].color = pal[rand() % 4];
            g_particles[i].size = rand() % 8 + 3;
            g_particles[i].life = rand() % 35 + 25;
        }
    } else if (type == 3) { // Magnet
        COLORREF pal[] = {RGB(224, 64, 251), RGB(234, 128, 252), RGB(255, 255, 255), RGB(74, 20, 140)};
        for (int i = 0; i < MAX_PARTICLES; i++) {
            float dist = (float)(rand() % 120 + 30);
            float angle = ((float)(rand() % 360)) * 3.14159f / 180.0f;
            g_particles[i].x = px + cosf(angle) * dist; g_particles[i].y = py + sinf(angle) * dist;
            g_particles[i].vx = -cosf(angle) * (dist / 15.0f);
            g_particles[i].vy = -sinf(angle) * (dist / 15.0f);
            g_particles[i].rotation = 0; g_particles[i].vRot = 0;
            g_particles[i].color = pal[rand() % 4];
            g_particles[i].size = rand() % 5 + 2;
            g_particles[i].life = 25; // Fixed lifetime to fly in
        }
    }
}

void UpdateParticles() {
    if (!g_dustInit) {
        for(int i=0; i<MAX_DUST; i++) {
            g_dust[i].x = (float)(rand() % 580);
            g_dust[i].y = (float)(rand() % 780);
            g_dust[i].vx = ((float)(rand() % 100) - 50.0f) / 100.0f;
            g_dust[i].vy = ((float)(rand() % 100) - 50.0f) / 100.0f;
            g_dust[i].size = rand() % 3 + 1;
            g_dust[i].phase = (float)(rand() % 100) / 10.0f;
        }
        g_dustInit = true;
    }
    for(int i=0; i<MAX_DUST; i++) {
        g_dust[i].x += g_dust[i].vx;
        g_dust[i].y += g_dust[i].vy;
        g_dust[i].phase += 0.05f;
        if(g_dust[i].x < 0) g_dust[i].x += 580;
        if(g_dust[i].x > 580) g_dust[i].x -= 580;
        if(g_dust[i].y < 0) g_dust[i].y += 780;
        if(g_dust[i].y > 780) g_dust[i].y -= 780;
    }
    for (int i = 0; i < g_particleCount; i++) {
        if (g_particles[i].life > 0) {
            g_particles[i].x += g_particles[i].vx;
            g_particles[i].y += g_particles[i].vy;
            // Only apply gravity to confetti/bomb/drill, magnet pulls inward and has fixed life behavior
            if (g_particles[i].vRot != 0) g_particles[i].vy += 0.15f; 
            g_particles[i].rotation += g_particles[i].vRot;
            g_particles[i].life--;
        }
    }
}

int hoverCol = -1;
int winCells[MAX_COLS * MAX_ROWS][2];
int winCellCount = 0;

HWND hModeBtn, hBombBtn, hDrillBtn, hMagnetBtn, hHintBtn, hFreezeBtn, hDiffSelect, hUndoBtn, hResetBtn, hMuteBtn, hSaveBtn, hLoadBtn, hHelpBtn;
HFONT hMainFont = NULL;
bool isMuted = false;

void PlaySoundEffect(int type) {
    if (isMuted) return;
    if (type == 1) { Beep(400, 50); }
    else if (type == 2) { Beep(400, 80); Beep(500, 80); Beep(600, 80); Beep(800, 120); }
    else if (type == 3) { Beep(300, 150); Beep(200, 200); }
    else if (type == 4) { Beep(150, 100); }
    else if (type == 5) { Beep(300, 250); }
    else if (type == 6) { Beep(120, 150); Beep(80, 200); }
    else if (type == 7) { Beep(600, 50); Beep(450, 50); Beep(300, 100); }
    else if (type == 8) { Beep(500, 60); Beep(700, 60); Beep(900, 80); }
    else if (type == 9) { Beep(250, 120); Beep(180, 150); }
}

void DrawDisc3D(HDC hdc, int x, int y, int cellType, bool isWinCell) {
    if (cellType >= 100 && cellType <= 700) {
        HBRUSH shadowBrush = CreateSolidBrush(RGB(10, 15, 25));
        HPEN nullPen = GetStockObject(NULL_PEN);
        SelectObject(hdc, nullPen); SelectObject(hdc, shadowBrush);
        Ellipse(hdc, x + 4, y + 15, x + 32, y + 42);
        DeleteObject(shadowBrush);
        y -= 6;
    }

    int cx = x + 18;
    int cy = y + 18;

    if (cellType == 0) { // Empty cutout hole
        HBRUSH bgBrush = CreateSolidBrush(RGB(9, 12, 20));
        HPEN darkPen = CreatePen(PS_SOLID, 1, RGB(4, 6, 10));
        SelectObject(hdc, bgBrush);
        SelectObject(hdc, darkPen);
        Ellipse(hdc, x, y, x + 36, y + 36);
        DeleteObject(bgBrush);
        DeleteObject(darkPen);

        HPEN shadowPen = CreatePen(PS_SOLID, 2, RGB(2, 3, 6));
        SelectObject(hdc, shadowPen);
        Arc(hdc, x + 2, y + 2, x + 34, y + 34, x + 34, y + 2, x + 2, y + 34);
        DeleteObject(shadowPen);
        return;
    }

    if (isWinCell) {
        HBRUSH nullBrush = GetStockObject(NULL_BRUSH);
        SelectObject(hdc, nullBrush);
        
        HPEN glow1 = CreatePen(PS_SOLID, 6, RGB(0, 100, 100));
        SelectObject(hdc, glow1);
        Ellipse(hdc, x - 5, y - 5, x + 41, y + 41);
        DeleteObject(glow1);

        HPEN glow2 = CreatePen(PS_SOLID, 4, RGB(0, 200, 200));
        SelectObject(hdc, glow2);
        Ellipse(hdc, x - 3, y - 3, x + 39, y + 39);
        DeleteObject(glow2);

        HPEN glow3 = CreatePen(PS_SOLID, 2, RGB(0, 255, 255));
        SelectObject(hdc, glow3);
        Ellipse(hdc, x - 1, y - 1, x + 37, y + 37);
        DeleteObject(glow3);
        
        HPEN sparkPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        SelectObject(hdc, sparkPen);
        MoveToEx(hdc, x - 4, cy, NULL); LineTo(hdc, x - 10, cy);
        MoveToEx(hdc, x - 7, cy - 3, NULL); LineTo(hdc, x - 7, cy + 3);
        
        MoveToEx(hdc, x + 40, cy - 10, NULL); LineTo(hdc, x + 46, cy - 10);
        MoveToEx(hdc, x + 43, cy - 13, NULL); LineTo(hdc, x + 43, cy - 7);
        DeleteObject(sparkPen);
    }

    if (cellType == 1 || cellType == 100) { // Red Disc
        COLORREF borderColor = (cellType == 100) ? RGB(140, 40, 40) : RGB(120, 0, 0);
        COLORREF bodyColor   = (cellType == 100) ? RGB(200, 70, 70) : RGB(220, 30, 30);
        COLORREF innerRing   = (cellType == 100) ? RGB(220, 90, 90) : RGB(180, 20, 20);

        HPEN bPen = CreatePen(PS_SOLID, 2, borderColor);
        HBRUSH bBrush = CreateSolidBrush(bodyColor);
        SelectObject(hdc, bPen); SelectObject(hdc, bBrush);
        Ellipse(hdc, x, y, x + 36, y + 36);
        DeleteObject(bPen); DeleteObject(bBrush);

        HPEN rPen = CreatePen(PS_SOLID, 1, innerRing);
        HBRUSH nBrush = GetStockObject(NULL_BRUSH);
        SelectObject(hdc, rPen); SelectObject(hdc, nBrush);
        Ellipse(hdc, x + 3, y + 3, x + 33, y + 33);
        DeleteObject(rPen);

        HPEN nPen = GetStockObject(NULL_PEN);
        SelectObject(hdc, nPen);
        
        HBRUSH hl1 = CreateSolidBrush((cellType == 100) ? RGB(240, 120, 120) : RGB(240, 90, 90));
        SelectObject(hdc, hl1);
        Ellipse(hdc, x + 5, y + 3, x + 31, y + 18);
        DeleteObject(hl1);

        HBRUSH hl2 = CreateSolidBrush((cellType == 100) ? RGB(255, 180, 180) : RGB(255, 140, 140));
        SelectObject(hdc, hl2);
        Ellipse(hdc, x + 7, y + 4, x + 28, y + 14);
        DeleteObject(hl2);

        HBRUSH wBrush = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, wBrush);
        Ellipse(hdc, x + 10, y + 5, x + 20, y + 9);
        DeleteObject(wBrush);

        POINT starPts[10];
        float outerR = 7.5f, innerR = 3.0f;
        for (int i = 0; i < 10; i++) {
            float angle = (i * 36 - 90) * 3.14159f / 180.0f;
            float r_curr = (i % 2 == 0) ? outerR : innerR;
            starPts[i].x = cx + (long)(cosf(angle) * r_curr);
            starPts[i].y = cy + (long)(sinf(angle) * r_curr);
        }
        HBRUSH starBrush = CreateSolidBrush(RGB(255, 215, 0));
        HPEN starPen = CreatePen(PS_SOLID, 1, RGB(180, 140, 0));
        SelectObject(hdc, starPen); SelectObject(hdc, starBrush);
        Polygon(hdc, starPts, 10);
        DeleteObject(starBrush); DeleteObject(starPen);
    }
    else if (cellType == 2 || cellType == 200) { // Yellow Disc
        COLORREF borderColor = (cellType == 200) ? RGB(160, 140, 40) : RGB(160, 100, 0);
        COLORREF bodyColor   = (cellType == 200) ? RGB(230, 210, 80) : RGB(255, 210, 30);
        COLORREF innerRing   = (cellType == 200) ? RGB(240, 220, 100) : RGB(220, 160, 0);

        HPEN bPen = CreatePen(PS_SOLID, 2, borderColor);
        HBRUSH bBrush = CreateSolidBrush(bodyColor);
        SelectObject(hdc, bPen); SelectObject(hdc, bBrush);
        Ellipse(hdc, x, y, x + 36, y + 36);
        DeleteObject(bPen); DeleteObject(bBrush);

        HPEN rPen = CreatePen(PS_SOLID, 1, innerRing);
        HBRUSH nBrush = GetStockObject(NULL_BRUSH);
        SelectObject(hdc, rPen); SelectObject(hdc, nBrush);
        Ellipse(hdc, x + 3, y + 3, x + 33, y + 33);
        DeleteObject(rPen);

        HPEN nPen = GetStockObject(NULL_PEN);
        SelectObject(hdc, nPen);
        
        HBRUSH hl1 = CreateSolidBrush((cellType == 200) ? RGB(240, 230, 120) : RGB(255, 230, 90));
        SelectObject(hdc, hl1);
        Ellipse(hdc, x + 5, y + 3, x + 31, y + 18);
        DeleteObject(hl1);

        HBRUSH hl2 = CreateSolidBrush((cellType == 200) ? RGB(255, 255, 180) : RGB(255, 250, 140));
        SelectObject(hdc, hl2);
        Ellipse(hdc, x + 7, y + 4, x + 28, y + 14);
        DeleteObject(hl2);

        HBRUSH wBrush = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, wBrush);
        Ellipse(hdc, x + 10, y + 5, x + 20, y + 9);
        DeleteObject(wBrush);

        POINT crownPts[7] = {
            {cx - 7, cy + 4}, {cx - 7, cy - 3}, {cx - 3, cy + 1},
            {cx, cy - 5}, {cx + 3, cy + 1}, {cx + 7, cy - 3}, {cx + 7, cy + 4}
        };
        HBRUSH crownBrush = CreateSolidBrush(RGB(24, 60, 150));
        HPEN crownPen = CreatePen(PS_SOLID, 1, RGB(10, 25, 80));
        SelectObject(hdc, crownPen); SelectObject(hdc, crownBrush);
        Polygon(hdc, crownPts, 7);
        DeleteObject(crownBrush); DeleteObject(crownPen);
    }
    else if (cellType == 3) { // Obstacle
        HBRUSH obsBrush = CreateSolidBrush(RGB(45, 45, 52));
        HPEN obsPen = CreatePen(PS_SOLID, 2, RGB(80, 80, 90));
        SelectObject(hdc, obsPen); SelectObject(hdc, obsBrush);
        Ellipse(hdc, x, y, x + 36, y + 36);
        DeleteObject(obsBrush); DeleteObject(obsPen);

        HPEN warnPen = CreatePen(PS_SOLID, 3, RGB(255, 215, 0));
        SelectObject(hdc, warnPen);
        MoveToEx(hdc, x + 8, y + 28, NULL); LineTo(hdc, x + 28, y + 8);
        DeleteObject(warnPen);
    }
    else if (cellType == 4) { // Crackable Block
        HBRUSH woodBrush = CreateSolidBrush(RGB(139, 69, 19));
        HPEN woodPen = CreatePen(PS_SOLID, 2, RGB(210, 130, 40));
        SelectObject(hdc, woodPen); SelectObject(hdc, woodBrush);
        Ellipse(hdc, x, y, x + 36, y + 36);
        DeleteObject(woodBrush);

        HPEN crackPen = CreatePen(PS_SOLID, 2, RGB(50, 25, 5));
        SelectObject(hdc, crackPen);
        MoveToEx(hdc, cx - 5, cy - 8, NULL); LineTo(hdc, cx, cy);
        LineTo(hdc, cx - 3, cy + 5); LineTo(hdc, cx + 6, cy + 8);
        DeleteObject(crackPen); DeleteObject(woodPen);
    }
    else if (cellType == 5 || cellType == 500) { // Bomb
        HBRUSH bBrush = CreateSolidBrush(RGB(25, 25, 25));
        HPEN bPen = CreatePen(PS_SOLID, 2, RGB(255, 68, 0));
        SelectObject(hdc, bPen); SelectObject(hdc, bBrush);
        Ellipse(hdc, x + 2, y + 2, x + 34, y + 34);
        DeleteObject(bBrush); DeleteObject(bPen);

        HPEN sparkPen = CreatePen(PS_SOLID, 2, RGB(255, 215, 0));
        SelectObject(hdc, sparkPen);
        MoveToEx(hdc, cx, cy - 12, NULL); LineTo(hdc, cx + 4, cy - 17);
        DeleteObject(sparkPen);
    }
    else if (cellType == 6 || cellType == 600) { // Drill
        HBRUSH dBrush = CreateSolidBrush(RGB(0, 150, 180));
        HPEN dPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 255));
        SelectObject(hdc, dPen); SelectObject(hdc, dBrush);
        Ellipse(hdc, x + 2, y + 2, x + 34, y + 34);
        DeleteObject(dBrush); DeleteObject(dPen);

        HPEN drillLine = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
        SelectObject(hdc, drillLine);
        MoveToEx(hdc, cx - 8, cy - 8, NULL); LineTo(hdc, cx + 8, cy + 8);
        MoveToEx(hdc, cx - 6, cy, NULL); LineTo(hdc, cx, cy + 6);
        DeleteObject(drillLine);
    }
    else if (cellType == 7 || cellType == 700) { // Magnet
        HBRUSH mBrush = CreateSolidBrush(RGB(138, 43, 226));
        HPEN mPen = CreatePen(PS_SOLID, 2, RGB(224, 64, 251));
        SelectObject(hdc, mPen); SelectObject(hdc, mBrush);
        Ellipse(hdc, x + 2, y + 2, x + 34, y + 34);
        DeleteObject(mBrush); DeleteObject(mPen);

        HPEN magLine = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
        SelectObject(hdc, magLine);
        Arc(hdc, cx - 8, cy - 8, cx + 8, cy + 8, cx - 8, cy, cx + 8, cy);
        DeleteObject(magLine);
    }
}

typedef struct {
    int redWins;
    int yellowWins;
    int draws;
    int streak;
    int bestStreak;
    int lastWinner;
    int maxCampaignStage;
    int totalBombs;
    int totalDrills;
    int totalMagnets;
} GameStats;

GameStats stats = {0, 0, 0, 0, 0, 0, 1, 0, 0, 0};

typedef struct {
    int board[MAX_ROWS][MAX_COLS];
    int rows, cols;
    int currentPlayer;
    int p1Bombs, p2Bombs;
    int p1Drills, p2Drills;
    int p1Magnets, p2Magnets;
    int p1Freezes, p2Freezes;
    int frozenCol, frozenTurns, frozenPlayer;
    GameStats oldStats;
} MoveRecord;

MoveRecord moveHistory[MAX_ROWS * MAX_COLS * 2];
int historyCount = 0;
int replayIndex = -1;

void SaveStats();

void ExportJSON() {
    FILE *f = fopen("kconnect4_data.json", "w");
    if (f) {
        fprintf(f, "{\n  \"stats\": {\n");
        fprintf(f, "    \"redWins\": %d,\n", stats.redWins);
        fprintf(f, "    \"yellowWins\": %d,\n", stats.yellowWins);
        fprintf(f, "    \"draws\": %d,\n", stats.draws);
        fprintf(f, "    \"streak\": %d,\n", stats.streak);
        fprintf(f, "    \"bestStreak\": %d,\n", stats.bestStreak);
        fprintf(f, "    \"lastWinner\": %d,\n", stats.lastWinner);
        fprintf(f, "    \"maxCampaignStage\": %d\n", stats.maxCampaignStage);
        fprintf(f, "  }\n}\n");
        fclose(f);
        MessageBox(NULL, "Exported to kconnect4_data.json", "Export", MB_OK | MB_ICONINFORMATION);
    }
}

void ImportJSON(HWND hwnd) {
    FILE *f = fopen("kconnect4_data.json", "r");
    if (f) {
        char line[256];
        while(fgets(line, sizeof(line), f)) {
            sscanf(line, " \"redWins\": %d,", &stats.redWins);
            sscanf(line, " \"yellowWins\": %d,", &stats.yellowWins);
            sscanf(line, " \"draws\": %d,", &stats.draws);
            sscanf(line, " \"streak\": %d,", &stats.streak);
            sscanf(line, " \"bestStreak\": %d,", &stats.bestStreak);
            sscanf(line, " \"lastWinner\": %d,", &stats.lastWinner);
            sscanf(line, " \"maxCampaignStage\": %d", &stats.maxCampaignStage);
        }
        fclose(f);
        SaveStats();
        InvalidateRect(hwnd, NULL, TRUE);
        MessageBox(hwnd, "Imported kconnect4_data.json", "Import", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBox(hwnd, "kconnect4_data.json not found", "Import Error", MB_OK | MB_ICONWARNING);
    }
}

void LoadStats() {
    FILE *f = fopen("kconnect4_stats.bin", "rb");
    if(f) {
        fread(&stats, sizeof(GameStats), 1, f);
        if(stats.maxCampaignStage < 1) stats.maxCampaignStage = 1;
        fclose(f);
    }
}

void SaveStats() {
    FILE *f = fopen("kconnect4_stats.bin", "wb");
    if(f) {
        fwrite(&stats, sizeof(GameStats), 1, f);
        fclose(f);
    }
}

typedef struct {
    int board[MAX_ROWS][MAX_COLS];
    int rows, cols;
    int currentPlayer;
    bool gameActive;
    bool isDraw;
    int gameMode;
    int aiPersonality;
    int campaignStage;
    int p1Bombs, p2Bombs;
    int p1Drills, p2Drills;
    int p1Magnets, p2Magnets;
    int p1Freezes, p2Freezes;
    int frozenCol, frozenTurns, frozenPlayer;
    int winCells[MAX_COLS * MAX_ROWS][2];
    int winCellCount;
    MoveRecord moveHistory[MAX_ROWS * MAX_COLS * 2];
    int historyCount;
} GameState;

void UpdateDiffSelectUI();
void ResetGame();

void SaveGame() {
    GameState state;
    memcpy(state.board, board, sizeof(board));
    state.rows = g_rows; state.cols = g_cols;
    state.currentPlayer = currentPlayer;
    state.gameActive = gameActive;
    state.isDraw = isDraw;
    state.gameMode = gameMode;
    state.aiPersonality = aiPersonality;
    state.campaignStage = campaignStage;
    state.p1Bombs = p1Bombs; state.p2Bombs = p2Bombs;
    state.p1Drills = p1Drills; state.p2Drills = p2Drills;
    state.p1Magnets = p1Magnets; state.p2Magnets = p2Magnets;
    state.p1Freezes = p1Freezes; state.p2Freezes = p2Freezes;
    state.frozenCol = frozenCol; state.frozenTurns = frozenTurns; state.frozenPlayer = frozenPlayer;
    memcpy(state.winCells, winCells, sizeof(winCells));
    state.winCellCount = winCellCount;
    memcpy(state.moveHistory, moveHistory, sizeof(moveHistory));
    state.historyCount = historyCount;
    
    FILE *f = fopen("kconnect4_save.dat", "wb");
    if(f) {
        fwrite(&state, sizeof(GameState), 1, f);
        fclose(f);
        MessageBox(NULL, "Game Saved Successfully!", "KConnect4", MB_OK | MB_ICONINFORMATION);
    }
}

void LoadGame(HWND hwnd) {
    FILE *f = fopen("kconnect4_save.dat", "rb");
    if(f) {
        GameState state;
        if (fread(&state, sizeof(GameState), 1, f) == 1) {
            memcpy(board, state.board, sizeof(board));
            g_rows = state.rows; g_cols = state.cols;
            currentPlayer = state.currentPlayer;
            gameActive = state.gameActive;
            isDraw = state.isDraw;
            gameMode = state.gameMode;
            aiPersonality = state.aiPersonality;
            campaignStage = state.campaignStage;
            p1Bombs = state.p1Bombs; p2Bombs = state.p2Bombs;
            p1Drills = state.p1Drills; p2Drills = state.p2Drills;
            p1Magnets = state.p1Magnets; p2Magnets = state.p2Magnets;
            p1Freezes = state.p1Freezes; p2Freezes = state.p2Freezes;
            frozenCol = state.frozenCol; frozenTurns = state.frozenTurns; frozenPlayer = state.frozenPlayer;
            memcpy(winCells, state.winCells, sizeof(winCells));
            winCellCount = state.winCellCount;
            memcpy(moveHistory, state.moveHistory, sizeof(moveHistory));
            historyCount = state.historyCount;
            
            if (gameMode == 0) SetWindowText(hModeBtn, "Mode: 2P");
            else if (gameMode == 1) SetWindowText(hModeBtn, "Mode: vs AI");
            else if (gameMode == 2) SetWindowText(hModeBtn, "Mode: Campaign");
            else SetWindowText(hModeBtn, "Mode: Speed");
            
            UpdateDiffSelectUI();
            
            isAnimating = false;
            selectedPowerup = 0;
            turnTimeLeftMs = 7000;
            KillTimer(hwnd, 1);
            KillTimer(hwnd, 2);
            if (gameMode == 3 && gameActive) SetTimer(hwnd, 3, 100, NULL);
            else KillTimer(hwnd, 3);
            
            replayIndex = (gameActive) ? -1 : historyCount - 1;
            
            InvalidateRect(hwnd, NULL, TRUE);
            MessageBox(hwnd, "Game Loaded Successfully!", "KConnect4", MB_OK | MB_ICONINFORMATION);
        }
        fclose(f);
    } else {
        MessageBox(hwnd, "No saved game file found.", "KConnect4", MB_OK | MB_ICONWARNING);
    }
}

void GetStageGridSize(int stage, int *outRows, int *outCols) {
    if (stage <= 5) { *outRows = 6; *outCols = 7; }
    else if (stage <= 10) { *outRows = 7; *outCols = 8; }
    else if (stage <= 15) { *outRows = 8; *outCols = 9; }
    else { *outRows = 8; *outCols = 10; }
}

int GetStageAIPersonality(int stage) {
    if (stage <= 3) return 0; // Rookie
    if (stage <= 6) return 1; // Aggressive
    if (stage <= 11) return 2; // Trapper
    return 3; // Grandmaster
}

void SetupCampaignStage(int stage) {
    GetStageGridSize(stage, &g_rows, &g_cols);
    for(int r=0; r<MAX_ROWS; r++)
        for(int c=0; c<MAX_COLS; c++)
            board[r][c] = 0;
            
    aiPersonality = GetStageAIPersonality(stage);

    int R = g_rows;
    int C = g_cols;

    if (stage == 2) { board[R-1][0] = 3; board[R-1][C-1] = 3; }
    else if (stage == 3) { board[R-1][C/2] = 4; board[R-2][C/2] = 4; }
    else if (stage == 4) { board[R-1][2] = 3; board[R-1][C-3] = 3; board[R-2][C/2] = 4; }
    else if (stage == 5) { board[R-1][1] = 3; board[R-1][C/2] = 3; board[R-1][C-2] = 3; board[R-2][2] = 4; board[R-2][C-3] = 4; }
    else if (stage == 6) { board[R-1][2] = 4; board[R-1][C-3] = 4; board[R-2][3] = 3; board[R-2][C-4] = 3; }
    else if (stage == 7) { board[R-3][2] = 3; board[R-3][3] = 3; board[R-3][4] = 3; board[R-3][5] = 3; }
    else if (stage == 8) { board[R-1][3] = 3; board[R-2][3] = 3; board[R-3][3] = 3; board[R-4][3] = 3; }
    else if (stage == 9) { board[R-1][1] = 3; board[R-2][1] = 3; board[R-1][C-2] = 3; board[R-2][C-2] = 3; board[R-3][C/2] = 4; board[R-4][C/2] = 4; }
    else if (stage == 10) { board[R-1][0] = 3; board[R-1][C-1] = 3; board[R-2][2] = 3; board[R-2][C-3] = 3; board[R-4][1] = 4; board[R-4][C-2] = 4; }
    else if (stage == 11) { board[R-1][2] = 4; board[R-1][C-3] = 4; board[R-2][3] = 4; board[R-2][C-4] = 4; board[R-3][C/2] = 3; }
    else if (stage == 12) { board[R-1][2] = 3; board[R-2][2] = 3; board[R-1][C-3] = 3; board[R-2][C-3] = 3; }
    else if (stage == 13) { board[R-1][3] = 4; board[R-1][C-4] = 4; board[R-2][2] = 4; board[R-2][C-3] = 4; board[R-3][C/2] = 3; board[R-4][C/2] = 3; }
    else if (stage == 14) { board[R-1][1] = 3; board[R-1][C-2] = 3; board[R-3][C/2] = 4; board[R-5][2] = 3; board[R-5][C-3] = 3; }
    else if (stage == 15) { board[R-1][0] = 3; board[R-1][C-1] = 3; board[R-2][C/2] = 4; board[R-3][2] = 4; board[R-3][C-3] = 4; board[R-4][C/2] = 4; }
    else if (stage == 16) { board[R-1][0] = 3; board[R-1][1] = 3; board[R-1][C-2] = 3; board[R-1][C-1] = 3; board[R-2][4] = 4; board[R-2][5] = 4; }
    else if (stage == 17) { board[R-2][C/2-1] = 4; board[R-2][C/2] = 4; board[R-3][C/2-2] = 4; board[R-3][C/2+1] = 4; board[R-4][C/2-1] = 3; board[R-4][C/2] = 3; }
    else if (stage == 18) { board[R-1][2] = 3; board[R-2][2] = 4; board[R-3][2] = 3; board[R-1][C-3] = 4; board[R-2][C-3] = 3; board[R-3][C-3] = 4; }
    else if (stage == 19) { board[R-1][1] = 3; board[R-1][3] = 4; board[R-1][5] = 3; board[R-1][7] = 4; board[R-2][2] = 4; board[R-2][4] = 3; board[R-2][6] = 4; }
    else if (stage == 20) { // Stage 20 Grandmaster Challenge!
        board[R-1][1] = 3; board[R-1][4] = 3; board[R-1][5] = 3; board[R-1][8] = 3;
        board[R-2][2] = 4; board[R-2][3] = 4; board[R-2][6] = 4; board[R-2][7] = 4;
        board[R-3][4] = 4; board[R-3][5] = 4; board[R-4][2] = 3; board[R-4][7] = 3;
    }
}

void ResetGame() {
    g_rows = 6; g_cols = 7;
    for(int r=0; r<MAX_ROWS; r++)
        for(int c=0; c<MAX_COLS; c++)
            board[r][c] = 0;
            
    if (gameMode == 2) {
        SetupCampaignStage(campaignStage);
    }

    currentPlayer = 1;
    gameActive = true;
    isDraw = false;
    isAnimating = false;
    winBeamProgress = 0.0f;
    winCellCount = 0;
    historyCount = 0;
    replayIndex = -1;
    selectedPowerup = 0;
    p1Bombs = 2; p2Bombs = 2;
    p1Drills = 2; p2Drills = 2;
    p1Magnets = 2; p2Magnets = 2;
    p1Freezes = 1; p2Freezes = 1;
    isFreezeMode = false;
    frozenCol = -1; frozenTurns = 0; frozenPlayer = 0;
    hintCol = -1; hintTimer = 0;
    turnTimeLeftMs = 7000;
}

void UpdatePowerupButtons() {
    char bBuf[32], dBuf[32], mBuf[32], fBuf[32];
    int bCount = (currentPlayer == 1) ? p1Bombs : p2Bombs;
    int dCount = (currentPlayer == 1) ? p1Drills : p2Drills;
    int mCount = (currentPlayer == 1) ? p1Magnets : p2Magnets;
    int fCount = (currentPlayer == 1) ? p1Freezes : p2Freezes;
    if (gameMode > 0 && currentPlayer == 2) { bCount = 0; dCount = 0; mCount = 0; fCount = 0; }

    wsprintf(bBuf, (selectedPowerup == 1) ? "[Bomb]" : "Bomb (%d)", bCount);
    wsprintf(dBuf, (selectedPowerup == 2) ? "[Drill]" : "Drill (%d)", dCount);
    wsprintf(mBuf, (selectedPowerup == 3) ? "[Mag]" : "Mag (%d)", mCount);
    wsprintf(fBuf, isFreezeMode ? "[Freeze]" : "Freeze (%d)", fCount);

    SetWindowText(hBombBtn, bBuf);
    SetWindowText(hDrillBtn, dBuf);
    SetWindowText(hMagnetBtn, mBuf);
    SetWindowText(hFreezeBtn, fBuf);
}

void UpdateDiffSelectUI() {
    SendMessage(hDiffSelect, CB_RESETCONTENT, 0, 0);
    if (gameMode == 1 || gameMode == 3) {
        ShowWindow(hDiffSelect, SW_SHOW);
        SendMessage(hDiffSelect, CB_ADDSTRING, 0, (LPARAM)"Rookie");
        SendMessage(hDiffSelect, CB_ADDSTRING, 0, (LPARAM)"Aggressive");
        SendMessage(hDiffSelect, CB_ADDSTRING, 0, (LPARAM)"Trapper");
        SendMessage(hDiffSelect, CB_ADDSTRING, 0, (LPARAM)"Grandmaster");
        SendMessage(hDiffSelect, CB_SETCURSEL, aiPersonality, 0);
    } else if (gameMode == 2) {
        ShowWindow(hDiffSelect, SW_SHOW);
        for(int i=1; i<=20; i++) {
            char label[32];
            if (i <= stats.maxCampaignStage) wsprintf(label, "Stage %d", i);
            else wsprintf(label, "Locked %d", i);
            SendMessage(hDiffSelect, CB_ADDSTRING, 0, (LPARAM)label);
        }
        SendMessage(hDiffSelect, CB_SETCURSEL, campaignStage - 1, 0);
    } else {
        ShowWindow(hDiffSelect, SW_HIDE);
    }
}

bool CheckWin(int r, int c, int p) {
    int dirs[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};
    for(int d=0; d<4; d++) {
        int count = 1;
        int tempCells[MAX_COLS * MAX_ROWS][2];
        tempCells[0][0] = r;
        tempCells[0][1] = c;
        for(int i=1; i<4; i++) {
            int nr = r + dirs[d][0]*i;
            int nc = c + dirs[d][1]*i;
            if(nr>=0 && nr<g_rows && nc>=0 && nc<g_cols && board[nr][nc]==p) {
                tempCells[count][0] = nr;
                tempCells[count][1] = nc;
                count++;
            }
            else break;
        }
        for(int i=1; i<4; i++) {
            int nr = r - dirs[d][0]*i;
            int nc = c - dirs[d][1]*i;
            if(nr>=0 && nr<g_rows && nc>=0 && nc<g_cols && board[nr][nc]==p) {
                tempCells[count][0] = nr;
                tempCells[count][1] = nc;
                count++;
            }
            else break;
        }
        if(count >= 4) {
            for(int k=0; k<count; k++) {
                winCells[k][0] = tempCells[k][0];
                winCells[k][1] = tempCells[k][1];
            }
            winCellCount = count;
            return true;
        }
    }
    return false;
}

bool CheckDraw() {
    for(int c=0; c<g_cols; c++) {
        if(board[0][c] == 0) return false;
    }
    return true;
}

bool checkWinBoard(int b[MAX_ROWS][MAX_COLS], int p) {
    for (int r = 0; r < g_rows; r++) {
        for (int c = 0; c < g_cols - 3; c++) {
            if (b[r][c] == p && b[r][c+1] == p && b[r][c+2] == p && b[r][c+3] == p) return true;
        }
    }
    for (int c = 0; c < g_cols; c++) {
        for (int r = 0; r < g_rows - 3; r++) {
            if (b[r][c] == p && b[r+1][c] == p && b[r+2][c] == p && b[r+3][c] == p) return true;
        }
    }
    for (int r = 0; r < g_rows - 3; r++) {
        for (int c = 0; c < g_cols - 3; c++) {
            if (b[r][c] == p && b[r+1][c+1] == p && b[r+2][c+2] == p && b[r+3][c+3] == p) return true;
        }
    }
    for (int r = 3; r < g_rows; r++) {
        for (int c = 0; c < g_cols - 3; c++) {
            if (b[r][c] == p && b[r-1][c+1] == p && b[r-2][c+2] == p && b[r-3][c+3] == p) return true;
        }
    }
    return false;
}

void ApplyGravity() {
    bool changed = true;
    while(changed) {
        changed = false;
        for (int c = 0; c < g_cols; c++) {
            for (int r = g_rows - 2; r >= 0; r--) {
                if ((board[r][c] == 1 || board[r][c] == 2) && board[r+1][c] == 0) {
                    board[r+1][c] = board[r][c];
                    board[r][c] = 0;
                    changed = true;
                }
            }
        }
    }
}

bool SimDrop(int col, int p) {
    int targetRow = -1;
    for (int r = 0; r < g_rows; r++) {
        if (board[r][col] != 0) { targetRow = r - 1; break; }
    }
    if (targetRow == -1 && board[g_rows-1][col] == 0) targetRow = g_rows - 1;
    if (targetRow < 0) return false;

    int r = targetRow;
    board[r][col] = p;
    int tempWin[MAX_COLS * MAX_ROWS][2];
    int tempCount = winCellCount;
    for(int i=0; i<winCellCount; i++) { tempWin[i][0]=winCells[i][0]; tempWin[i][1]=winCells[i][1]; }
    bool win = CheckWin(r, col, p);
    for(int i=0; i<tempCount; i++) { winCells[i][0]=tempWin[i][0]; winCells[i][1]=tempWin[i][1]; }
    winCellCount = tempCount;
    board[r][col] = 0;
    return win;
}

int evaluateWindow(int w[4], int piece) {
    int score = 0;
    int oppPiece = (piece == 1) ? 2 : 1;
    int pc = 0, ec = 0, oc = 0, obs = 0;
    for(int i=0; i<4; i++) {
        if(w[i] == piece) pc++;
        else if(w[i] == 0) ec++;
        else if(w[i] == oppPiece) oc++;
        else if(w[i] == 3 || w[i] == 4) obs++;
    }
    if (obs > 0) return 0;
    if (pc == 4) score += 100;
    else if (pc == 3 && ec == 1) {
        if (aiPersonality == 3) score += 15;
        else score += (piece == 2 && aiPersonality == 1) ? 15 : 5;
    }
    else if (pc == 2 && ec == 2) {
        if (aiPersonality == 3) score += 4;
        else score += (piece == 2 && aiPersonality == 1) ? 6 : 2;
    }
    
    if (oc == 3 && ec == 1) {
        if (aiPersonality == 3) score -= 80;
        else score -= 8;
    } else if (oc == 2 && ec == 2) {
        if (aiPersonality == 3) score -= 10;
    }
    return score;
}

int scoreBoard(int b[MAX_ROWS][MAX_COLS], int piece) {
    int score = 0;
    int centerCol = g_cols / 2;
    int centerCount = 0;
    for (int r=0; r<g_rows; r++) if (b[r][centerCol] == piece) centerCount++;
    score += centerCount * 4;

    for (int r=0; r<g_rows; r++) {
        for (int c=0; c<g_cols-3; c++) {
            int w[4] = {b[r][c], b[r][c+1], b[r][c+2], b[r][c+3]};
            score += evaluateWindow(w, piece);
        }
    }
    for (int c=0; c<g_cols; c++) {
        for (int r=0; r<g_rows-3; r++) {
            int w[4] = {b[r][c], b[r+1][c], b[r+2][c], b[r+3][c]};
            score += evaluateWindow(w, piece);
        }
    }
    for (int r=0; r<g_rows-3; r++) {
        for (int c=0; c<g_cols-3; c++) {
            int w[4] = {b[r][c], b[r+1][c+1], b[r+2][c+2], b[r+3][c+3]};
            score += evaluateWindow(w, piece);
        }
    }
    for (int r=3; r<g_rows; r++) {
        for (int c=0; c<g_cols-3; c++) {
            int w[4] = {b[r][c], b[r-1][c+1], b[r-2][c+2], b[r-3][c+3]};
            score += evaluateWindow(w, piece);
        }
    }
    return score;
}

int getValidLocations(int b[MAX_ROWS][MAX_COLS], int locs[MAX_COLS]) {
    int count = 0;
    int center = g_cols / 2;
    int pref[MAX_COLS];
    pref[0] = center;
    int pIdx = 1;
    for(int step=1; step<g_cols; step++) {
        if (center - step >= 0) pref[pIdx++] = center - step;
        if (center + step < g_cols) pref[pIdx++] = center + step;
    }

    for(int i=0; i<g_cols; i++) {
        int c = pref[i];
        if (frozenTurns > 0 && frozenCol == c) continue;
        if (b[0][c] == 0) locs[count++] = c;
    }
    return count;
}

bool isTerminalNode(int b[MAX_ROWS][MAX_COLS]) {
    if(checkWinBoard(b, 1) || checkWinBoard(b, 2)) return true;
    for(int c=0; c<g_cols; c++) if(b[0][c] == 0) return false;
    return true;
}

int getNextOpenRow(int b[MAX_ROWS][MAX_COLS], int c) {
    for (int r = 0; r < g_rows; r++) {
        if (b[r][c] != 0) return r - 1;
    }
    return g_rows - 1;
}

typedef struct {
    int col;
    int score;
} MMResult;

MMResult minimax(int b[MAX_ROWS][MAX_COLS], int depth, int alpha, int beta, bool maximizingPlayer) {
    int validLocations[MAX_COLS];
    int count = getValidLocations(b, validLocations);
    bool isTerminal = isTerminalNode(b);
    
    if (depth == 0 || isTerminal) {
        MMResult res;
        res.col = -1;
        if (isTerminal) {
            if (checkWinBoard(b, 2)) res.score = 10000000;
            else if (checkWinBoard(b, 1)) res.score = -10000000;
            else res.score = 0;
        } else {
            res.score = scoreBoard(b, 2);
        }
        return res;
    }
    
    if (maximizingPlayer) {
        int value = -20000000;
        int column = (count > 0) ? validLocations[rand() % count] : -1;
        for (int i=0; i<count; i++) {
            int c = validLocations[i];
            int r = getNextOpenRow(b, c);
            if (r == -1) continue;
            b[r][c] = 2;
            MMResult newRes = minimax(b, depth - 1, alpha, beta, false);
            b[r][c] = 0;
            if (newRes.score > value) {
                value = newRes.score;
                column = c;
            }
            if (value > alpha) alpha = value;
            if (alpha >= beta) break;
        }
        MMResult res = {column, value};
        return res;
    } else {
        int value = 20000000;
        int column = (count > 0) ? validLocations[rand() % count] : -1;
        for (int i=0; i<count; i++) {
            int c = validLocations[i];
            int r = getNextOpenRow(b, c);
            if (r == -1) continue;
            b[r][c] = 1;
            MMResult newRes = minimax(b, depth - 1, alpha, beta, true);
            b[r][c] = 0;
            if (newRes.score < value) {
                value = newRes.score;
                column = c;
            }
            if (value < beta) beta = value;
            if (alpha >= beta) break;
        }
        MMResult res = {column, value};
        return res;
    }
}

int GetBestMoveAI(int player) {
    int depth = 5;
    if (g_cols > 8) depth = 4;
    if (aiPersonality == 3) depth = (g_cols > 8) ? 6 : 7;
    MMResult res = minimax(board, depth, -20000000, 20000000, (player == 2));
    if (res.col != -1) return res.col;
    int valid[MAX_COLS];
    int count = getValidLocations(board, valid);
    if (count > 0) return valid[rand() % count];
    return 0;
}

void ExecuteDrop(HWND hwnd, int col, int player, int powerType) {
    int targetRow = -1;
    for (int r = 0; r < g_rows; r++) {
        if (board[r][col] != 0) { targetRow = r - 1; break; }
    }
    if (targetRow == -1 && board[g_rows-1][col] == 0) targetRow = g_rows - 1;
    
    if (targetRow >= 0) {
        int r = targetRow;
        int snd = 1;
        if (powerType == 1) snd = 6;
        else if (powerType == 2) snd = 7;
        else if (powerType == 3) snd = 8;
        PlaySoundEffect(snd);
        
        // Record history
        memcpy(moveHistory[historyCount].board, board, sizeof(board));
        moveHistory[historyCount].rows = g_rows;
        moveHistory[historyCount].cols = g_cols;
        moveHistory[historyCount].currentPlayer = currentPlayer;
        moveHistory[historyCount].p1Bombs = p1Bombs; moveHistory[historyCount].p2Bombs = p2Bombs;
        moveHistory[historyCount].p1Drills = p1Drills; moveHistory[historyCount].p2Drills = p2Drills;
        moveHistory[historyCount].p1Magnets = p1Magnets; moveHistory[historyCount].p2Magnets = p2Magnets;
        moveHistory[historyCount].p1Freezes = p1Freezes; moveHistory[historyCount].p2Freezes = p2Freezes;
        moveHistory[historyCount].frozenCol = frozenCol; moveHistory[historyCount].frozenTurns = frozenTurns; moveHistory[historyCount].frozenPlayer = frozenPlayer;
        moveHistory[historyCount].oldStats = stats;
        historyCount++;

        if (powerType == 1) {
            if (player == 1) p1Bombs--; else p2Bombs--;
            stats.totalBombs++;
        } else if (powerType == 2) {
            if (player == 1) p1Drills--; else p2Drills--;
            stats.totalDrills++;
        } else if (powerType == 3) {
            if (player == 1) p1Magnets--; else p2Magnets--;
            stats.totalMagnets++;
        }

        board[r][col] = player;
        animPlayer = player;
        animRow = r;
        animCol = col;
        animY = 50;
        animY_float = 50.0f;
        animVY = 0.0f;
        animBounceCount = 0;

        int colWidth = 40;
        animTargetY = 50 + 5 + r * 44;
        animType = powerType;
        isAnimating = true;
        selectedPowerup = 0;
        UpdatePowerupButtons();
        
        SetTimer(hwnd, 2, 16, NULL);
    }
}

void AIMove(HWND hwnd) {
    if (!gameActive || (gameMode == 0)) return;
    
    int bestCol = -1;
    int powerType = 0;
    
    if (aiPersonality == 0) { // Rookie
        int available[MAX_COLS];
        int count = 0;
        for(int c=0; c<g_cols; c++) {
            if (frozenTurns > 0 && frozenCol == c) continue;
            if (board[0][c] == 0) available[count++] = c;
        }
        if (count == 0) return;
        
        for(int i=0; i<count; i++) {
            if(SimDrop(available[i], 2)) { bestCol = available[i]; break; }
        }
        if(bestCol == -1) {
            for(int i=0; i<count; i++) {
                if(SimDrop(available[i], 1)) { bestCol = available[i]; break; }
            }
        }
        if(bestCol == -1) {
            bestCol = available[rand() % count];
        }
    } else {
        int depth = 3;
        if (aiPersonality == 1) depth = 4;
        else if (aiPersonality == 2) depth = 5;
        else if (aiPersonality == 3) depth = (g_cols > 8) ? 6 : 7;
        
        MMResult res = minimax(board, depth, -20000000, 20000000, true);
        bestCol = res.col;
        if (bestCol == -1) {
            int validLocations[MAX_COLS];
            int count = getValidLocations(board, validLocations);
            if (count > 0) bestCol = validLocations[rand() % count];
        }
    }
    
    if (bestCol != -1) {
        ExecuteDrop(hwnd, bestCol, 2, powerType);
    }
}

void FinishTurnEffects(HWND hwnd) {
    if (animType == 1) { // Bomb
        SpawnPowerupParticles(1, animRow, animCol, hwnd);
        SetTimer(hwnd, 4, 25, NULL);
        for(int dr=-1; dr<=1; dr++) {
            for(int dc=-1; dc<=1; dc++) {
                int nr = animRow + dr, nc = animCol + dc;
                if(nr>=0 && nr<g_rows && nc>=0 && nc<g_cols && (board[nr][nc] == 1 || board[nr][nc] == 2 || board[nr][nc] == 4)) {
                    board[nr][nc] = 0;
                }
            }
        }
        ApplyGravity();
    } else if (animType == 2) { // Drill / Heavy Anvil (destroys cell directly underneath)
        SpawnPowerupParticles(2, animRow, animCol, hwnd);
        SetTimer(hwnd, 4, 25, NULL);
        if (animRow + 1 < g_rows && (board[animRow + 1][animCol] == 1 || board[animRow + 1][animCol] == 2 || board[animRow + 1][animCol] == 4)) {
            board[animRow + 1][animCol] = 0;
        }
        ApplyGravity();
    } else if (animType == 3) { // Magnet Disc (pulls friendly discs from adjacent cols)
        SpawnPowerupParticles(3, animRow, animCol, hwnd);
        SetTimer(hwnd, 4, 25, NULL);
        int adjCols[2] = {animCol - 1, animCol + 1};
        for (int i = 0; i < 2; i++) {
            int ac = adjCols[i];
            if (ac >= 0 && ac < g_cols) {
                for (int r = g_rows - 1; r >= 0; r--) {
                    if (board[r][ac] == animPlayer) {
                        int targetRow = -1;
                        for (int rTarget = 0; rTarget < g_rows; rTarget++) {
                            if (board[rTarget][animCol] != 0) { targetRow = rTarget - 1; break; }
                        }
                        if (targetRow == -1 && board[g_rows-1][animCol] == 0) targetRow = g_rows - 1;
                        if (targetRow >= 0) {
                            board[targetRow][animCol] = animPlayer;
                            board[r][ac] = 0;
                            break;
                        }
                    }
                }
            }
        }
        ApplyGravity();
    }
    
    // Decrement freeze status
    if (frozenTurns > 0) {
        frozenTurns--;
        if (frozenTurns == 0) frozenCol = -1;
    }

    bool w1 = checkWinBoard(board, 1);
    bool w2 = checkWinBoard(board, 2);
    
    if (w1 || w2) {
        if (w1 && w2) {
            PlaySoundEffect(5);
            gameActive = false;
            isDraw = true;
            stats.draws++;
            stats.streak = 0;
            stats.lastWinner = 0;
        } else {
            int winner = w1 ? 1 : 2;
            if (gameMode > 0 && winner == 2) PlaySoundEffect(3);
            else { PlaySoundEffect(2); SpawnConfetti(); SetTimer(hwnd, 4, 25, NULL); }
            gameActive = false;
            winBeamProgress = 0.0f;
            if(winner == 1) stats.redWins++;
            else stats.yellowWins++;
            
            if(stats.lastWinner == winner) stats.streak++;
            else stats.streak = 1;
            stats.lastWinner = winner;
            if(stats.streak > stats.bestStreak) stats.bestStreak = stats.streak;
            
            if (gameMode == 2 && winner == 1 && campaignStage == stats.maxCampaignStage && campaignStage < 20) {
                stats.maxCampaignStage++;
                UpdateDiffSelectUI();
            }
        }
        memcpy(moveHistory[historyCount].board, board, sizeof(board));
        moveHistory[historyCount].rows = g_rows; moveHistory[historyCount].cols = g_cols;
        moveHistory[historyCount].currentPlayer = currentPlayer;
        replayIndex = historyCount;
        historyCount++;
        SaveStats();
    } else if (CheckWin(animRow, animCol, animPlayer)) {
        if (gameMode > 0 && animPlayer == 2) PlaySoundEffect(3);
        else { PlaySoundEffect(2); SpawnConfetti(); SetTimer(hwnd, 4, 25, NULL); }
        gameActive = false;
        winBeamProgress = 0.0f;
        if(animPlayer == 1) stats.redWins++;
        else stats.yellowWins++;
        
        if(stats.lastWinner == animPlayer) stats.streak++;
        else stats.streak = 1;
        stats.lastWinner = animPlayer;
        if(stats.streak > stats.bestStreak) stats.bestStreak = stats.streak;
        
        if (gameMode == 2 && animPlayer == 1 && campaignStage == stats.maxCampaignStage && campaignStage < 20) {
            stats.maxCampaignStage++;
            UpdateDiffSelectUI();
        }
        memcpy(moveHistory[historyCount].board, board, sizeof(board));
        moveHistory[historyCount].rows = g_rows; moveHistory[historyCount].cols = g_cols;
        moveHistory[historyCount].currentPlayer = currentPlayer;
        replayIndex = historyCount;
        historyCount++;
        SaveStats();
    } else if (CheckDraw()) {
        PlaySoundEffect(5);
        gameActive = false;
        isDraw = true;
        stats.draws++;
        stats.streak = 0;
        stats.lastWinner = 0;
        memcpy(moveHistory[historyCount].board, board, sizeof(board));
        moveHistory[historyCount].rows = g_rows; moveHistory[historyCount].cols = g_cols;
        moveHistory[historyCount].currentPlayer = currentPlayer;
        replayIndex = historyCount;
        historyCount++;
        SaveStats();
    } else {
        currentPlayer = (animPlayer == 1) ? 2 : 1;
        turnTimeLeftMs = 7000;
        UpdatePowerupButtons();
        if (gameMode > 0 && currentPlayer == 2 && gameActive) {
            SetTimer(hwnd, 1, 300, NULL);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            srand((unsigned int)time(NULL));
            LoadStats();
            HDC hdc = GetDC(hwnd);
            int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(hwnd, hdc);
            int fontHeight = -MulDiv(12, dpi, 72);
            hMainFont = CreateFont(fontHeight, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            // Row 1 buttons
            hModeBtn = CreateWindow("BUTTON", "Mode: vs AI", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 520, 95, 28, hwnd, (HMENU)1, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hDiffSelect = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 110, 522, 105, 200, hwnd, (HMENU)4, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hHintBtn = CreateWindow("BUTTON", "Hint (T)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 220, 520, 75, 28, hwnd, (HMENU)11, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hFreezeBtn = CreateWindow("BUTTON", "Freeze (1)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 300, 520, 80, 28, hwnd, (HMENU)12, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hUndoBtn = CreateWindow("BUTTON", "Undo (U)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 385, 520, 75, 28, hwnd, (HMENU)3, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            // Row 2 buttons
            hBombBtn = CreateWindow("BUTTON", "Bomb (2)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 555, 80, 28, hwnd, (HMENU)9, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hDrillBtn = CreateWindow("BUTTON", "Drill (2)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 95, 555, 80, 28, hwnd, (HMENU)10, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hMagnetBtn = CreateWindow("BUTTON", "Mag (2)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 180, 555, 80, 28, hwnd, (HMENU)13, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hResetBtn = CreateWindow("BUTTON", "Reset", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 265, 555, 65, 28, hwnd, (HMENU)2, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hMuteBtn = CreateWindow("BUTTON", "Mute", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 335, 555, 60, 28, hwnd, (HMENU)5, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hHelpBtn = CreateWindow("BUTTON", "Help", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 400, 555, 60, 28, hwnd, (HMENU)8, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            // Row 3 buttons
            hSaveBtn = CreateWindow("BUTTON", "Save (F5)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 590, 75, 28, hwnd, (HMENU)6, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hLoadBtn = CreateWindow("BUTTON", "Load (F9)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 90, 590, 75, 28, hwnd, (HMENU)7, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            
            HWND hExportBtn = CreateWindow("BUTTON", "Export JSON", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 170, 590, 95, 28, hwnd, (HMENU)14, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            HWND hImportBtn = CreateWindow("BUTTON", "Import JSON", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 270, 590, 95, 28, hwnd, (HMENU)15, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            
            SendMessage(hModeBtn, WM_SETFONT, (WPARAM)hMainFont, TRUE);
            SendMessage(hDiffSelect, WM_SETFONT, (WPARAM)hMainFont, TRUE);
            SendMessage(hHintBtn, WM_SETFONT, (WPARAM)hMainFont, TRUE);
            SendMessage(hFreezeBtn, WM_SETFONT, (WPARAM)hMainFont, TRUE);
            SendMessage(hUndoBtn, WM_SETFONT, (WPARAM)hMainFont, TRUE);
            SendMessage(hBombBtn, WM_SETFONT, (WPARAM)hMainFont, TRUE);
            SendMessage(hDrillBtn, WM_SETFONT, (WPARAM)hMainFont, TRUE);
            SendMessage(hMagnetBtn, WM_SETFONT, (WPARAM)hMainFont, TRUE);
            SendMessage(hResetBtn, WM_SETFONT, (WPARAM)hMainFont, TRUE);
            SendMessage(hMuteBtn, WM_SETFONT, (WPARAM)hMainFont, TRUE);
            SendMessage(hHelpBtn, WM_SETFONT, (WPARAM)hMainFont, TRUE);
            SendMessage(hSaveBtn, WM_SETFONT, (WPARAM)hMainFont, TRUE);
            SendMessage(hLoadBtn, WM_SETFONT, (WPARAM)hMainFont, TRUE);
            SendMessage(hExportBtn, WM_SETFONT, (WPARAM)hMainFont, TRUE);
            SendMessage(hImportBtn, WM_SETFONT, (WPARAM)hMainFont, TRUE);

            UpdateDiffSelectUI();
            ResetGame();
            SetTimer(hwnd, 4, 30, NULL);
            break;

        case WM_KEYDOWN: {
            if (isAnimating) break;
            int key = (int)wParam;
            if (key == VK_F1 || key == 'H' || key == 'h') { // F1 or H Help
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(8, 0), 0);
            } else if (key == VK_F5) { // F5 Quicksave
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(6, 0), 0);
            } else if (key == VK_F9) { // F9 Quickload
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(7, 0), 0);
            } else if (key == VK_LEFT && !gameActive) { // Replay Prev
                if (replayIndex > 0) {
                    replayIndex--;
                    memcpy(board, moveHistory[replayIndex].board, sizeof(board));
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (key == VK_RIGHT && !gameActive) { // Replay Next
                if (replayIndex < historyCount - 1) {
                    replayIndex++;
                    memcpy(board, moveHistory[replayIndex].board, sizeof(board));
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (key == 'T' || key == 't') { // AI Hint skill
                if (gameActive && !(gameMode > 0 && currentPlayer == 2)) {
                    hintCol = GetBestMoveAI(currentPlayer);
                    hintTimer = 150; // show for 150 frames (~3 sec)
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (key == 'U' || key == 'u') { // Undo move
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(3, 0), 0);
            } else if (key == 'F' || key == 'f') { // Column Freeze skill
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(12, 0), 0);
            } else if (key == 'B' || key == 'b' || key == '1') { // Bomb Disc
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(9, 0), 0);
            } else if (key == 'D' || key == 'd' || key == '2') { // Drill Disc
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(10, 0), 0);
            } else if (key == 'M' || key == 'm' || key == '3') { // Magnet Disc
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(13, 0), 0);
            }
            break;
        }
            
        case WM_COMMAND:
            if (LOWORD(wParam) == 1) { // Mode
                gameMode = (gameMode + 1) % 4;
                if (gameMode == 0) SetWindowText(hModeBtn, "Mode: 2P");
                else if (gameMode == 1) SetWindowText(hModeBtn, "Mode: vs AI");
                else if (gameMode == 2) SetWindowText(hModeBtn, "Mode: Campaign");
                else SetWindowText(hModeBtn, "Mode: Speed");
                
                UpdateDiffSelectUI();
                ResetGame();
                if (gameMode == 3 && gameActive) SetTimer(hwnd, 3, 100, NULL);
                else KillTimer(hwnd, 3);
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 9) { // Bomb
                if (!gameActive || (gameMode > 0 && currentPlayer == 2)) break;
                int bCount = (currentPlayer == 1) ? p1Bombs : p2Bombs;
                if (bCount > 0) {
                    selectedPowerup = (selectedPowerup == 1) ? 0 : 1;
                    isFreezeMode = false;
                    UpdatePowerupButtons();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (LOWORD(wParam) == 10) { // Drill
                if (!gameActive || (gameMode > 0 && currentPlayer == 2)) break;
                int dCount = (currentPlayer == 1) ? p1Drills : p2Drills;
                if (dCount > 0) {
                    selectedPowerup = (selectedPowerup == 2) ? 0 : 2;
                    isFreezeMode = false;
                    UpdatePowerupButtons();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (LOWORD(wParam) == 13) { // Magnet
                if (!gameActive || (gameMode > 0 && currentPlayer == 2)) break;
                int mCount = (currentPlayer == 1) ? p1Magnets : p2Magnets;
                if (mCount > 0) {
                    selectedPowerup = (selectedPowerup == 3) ? 0 : 3;
                    isFreezeMode = false;
                    UpdatePowerupButtons();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (LOWORD(wParam) == 11) { // Optimal AI Hint
                if (gameActive && !(gameMode > 0 && currentPlayer == 2)) {
                    hintCol = GetBestMoveAI(currentPlayer);
                    hintTimer = 150;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (LOWORD(wParam) == 12) { // Column Freeze skill
                if (!gameActive || (gameMode > 0 && currentPlayer == 2)) break;
                int fCount = (currentPlayer == 1) ? p1Freezes : p2Freezes;
                if (fCount > 0) {
                    isFreezeMode = !isFreezeMode;
                    selectedPowerup = 0;
                    UpdatePowerupButtons();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (LOWORD(wParam) == 4 && HIWORD(wParam) == CBN_SELCHANGE) {
                int sel = SendMessage(hDiffSelect, CB_GETCURSEL, 0, 0);
                if (gameMode == 1 || gameMode == 3) {
                    aiPersonality = sel;
                } else if (gameMode == 2) {
                    if (sel + 1 <= stats.maxCampaignStage) {
                        campaignStage = sel + 1;
                        ResetGame();
                        InvalidateRect(hwnd, NULL, TRUE);
                    } else {
                        SendMessage(hDiffSelect, CB_SETCURSEL, campaignStage - 1, 0);
                    }
                }
            } else if (LOWORD(wParam) == 2) {
                ResetGame();
                if (gameMode == 3 && gameActive) SetTimer(hwnd, 3, 100, NULL);
                else KillTimer(hwnd, 3);
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 5) {
                isMuted = !isMuted;
                SetWindowText(hMuteBtn, isMuted ? "Unmute" : "Mute");
            } else if (LOWORD(wParam) == 6) {
                SaveGame();
            } else if (LOWORD(wParam) == 7) {
                LoadGame(hwnd);
            } else if (LOWORD(wParam) == 8) {
                MessageBox(hwnd, "KConnect4 - Loop 7 Campaign Expansion\n\n"
                    "Campaign Mode: 20 Stages featuring dynamic board dimensions (7x6, 8x7, 9x8, 10x8) and 4 AI Personalities (Rookie, Aggressive, Trapper, Grandmaster Minimax Alpha-Beta)!\n\n"
                    "Special Disc Types:\n"
                    "- Bomb Disc (1/B): Explodes 3x3 surrounding cells.\n"
                    "- Drill/Anvil Disc (2/D): Crushes cell directly underneath.\n"
                    "- Magnet Disc (3/M): Pulls friendly discs from adjacent columns.\n\n"
                    "Active Skills:\n"
                    "- AI Hint (T): Highlights optimal column.\n"
                    "- Undo Move (U): Reverts last turn pair.\n"
                    "- Column Freeze (F): Locks 1 opponent column for 2 turns.\n"
                    "- Replay Viewer (Left/Right Arrows): Step through match after game ends.", 
                    "Help & Information", MB_OK | MB_ICONINFORMATION);
            } else if (LOWORD(wParam) == 14) { // Export JSON
                ExportJSON();
            } else if (LOWORD(wParam) == 15) { // Import JSON
                ImportJSON(hwnd);
            } else if (LOWORD(wParam) == 3) { // Undo
                if (historyCount == 0 || isAnimating) break;
                
                KillTimer(hwnd, 1);
                int toPop = 1;
                if (gameMode > 0) {
                    if (gameActive && currentPlayer == 1 && historyCount >= 2) toPop = 2;
                    if (!gameActive && historyCount >= 2 && moveHistory[historyCount - 1].currentPlayer == 2) toPop = 2;
                }
                
                for (int i = 0; i < toPop; i++) {
                    if (historyCount > 0) {
                        historyCount--;
                        MoveRecord m = moveHistory[historyCount];
                        memcpy(board, m.board, sizeof(board));
                        g_rows = m.rows; g_cols = m.cols;
                        currentPlayer = m.currentPlayer;
                        p1Bombs = m.p1Bombs; p2Bombs = m.p2Bombs;
                        p1Drills = m.p1Drills; p2Drills = m.p2Drills;
                        p1Magnets = m.p1Magnets; p2Magnets = m.p2Magnets;
                        p1Freezes = m.p1Freezes; p2Freezes = m.p2Freezes;
                        frozenCol = m.frozenCol; frozenTurns = m.frozenTurns; frozenPlayer = m.frozenPlayer;
                        stats = m.oldStats;
                    }
                }
                
                SaveStats();
                gameActive = true;
                isDraw = false;
                winCellCount = 0;
                turnTimeLeftMs = 7000;
                isFreezeMode = false;
                UpdatePowerupButtons();
                if (gameMode == 3) SetTimer(hwnd, 3, 100, NULL);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
            
        case WM_TIMER:
            if (wParam == 1) {
                KillTimer(hwnd, 1);
                AIMove(hwnd);
            } else if (wParam == 2) {
                animVY += 3.0f;
                animY_float += animVY;
                if (animY_float >= animTargetY) {
                    animY_float = (float)animTargetY;
                    if (animVY > 5.0f && animBounceCount < 3) {
                        g_screenShakeTimer = (int)(animVY / 2.5f);
                        animVY = -animVY * 0.45f;
                        animBounceCount++;
                    } else {
                        animY = animTargetY;
                        isAnimating = false;
                        KillTimer(hwnd, 2);
                        FinishTurnEffects(hwnd);
                    }
                }
                animY = (int)animY_float;
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 3) {
                if (gameMode == 3 && gameActive && !isAnimating) {
                    if (!(gameMode > 0 && currentPlayer == 2)) {
                        turnTimeLeftMs -= 100;
                        if (turnTimeLeftMs <= 0) {
                            turnTimeLeftMs = 7000;
                            int validLocations[MAX_COLS];
                            int count = getValidLocations(board, validLocations);
                            if (count > 0) {
                                int col = validLocations[rand() % count];
                                ExecuteDrop(hwnd, col, currentPlayer, 0);
                            }
                        }
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                }
            } else if (wParam == 4) {
                if (g_screenShakeTimer > 0) g_screenShakeTimer--;
                UpdateParticles();
                if (hintTimer > 0) {
                    hintTimer--;
                    if (hintTimer == 0) hintCol = -1;
                }
                if (!gameActive && winCellCount >= 4 && winBeamProgress < 1.0f) {
                    winBeamProgress += 0.05f;
                    if (winBeamProgress > 1.0f) winBeamProgress = 1.0f;
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            
            HBRUSH bg = CreateSolidBrush(RGB(27, 38, 59));
            FillRect(hdc, &rect, bg);
            DeleteObject(bg);
            
            HBRUSH dotBrush = CreateSolidBrush(RGB(45, 62, 94));
            HPEN nullPen = GetStockObject(NULL_PEN);
            SelectObject(hdc, dotBrush);
            SelectObject(hdc, nullPen);
            for (int y = 0; y < rect.bottom; y += 40) {
                for (int x = 0; x < rect.right; x += 40) {
                    Ellipse(hdc, x-2, y-2, x+2, y+2);
                    Ellipse(hdc, x+20-2, y+20-2, x+20+2, y+20+2);
                }
            }
            DeleteObject(dotBrush);
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(79, 195, 247));
            if (hMainFont) SelectObject(hdc, hMainFont);
            
            char statusText[96];
            if (!gameActive) {
                if (isDraw) wsprintf(statusText, "It's a Draw! Click to reset.");
                else wsprintf(statusText, "Player %d (%s) Wins!", currentPlayer, (currentPlayer == 1) ? "Red" : "Yellow");
            } else if (isFreezeMode) {
                wsprintf(statusText, "Freeze Skill: Click a column to freeze!");
            } else {
                wsprintf(statusText, "Player %d's turn (%s)%s", currentPlayer, (currentPlayer == 1) ? "Red" : "Yellow",
                         (frozenTurns > 0) ? " [Col Frozen]" : "");
            }
            if (!gameActive && historyCount > 0) {
                char replayText[64];
                wsprintf(replayText, " | Replay: %d/%d (Arrows L/R)", replayIndex, historyCount - 1);
                lstrcat(statusText, replayText);
            }
            TextOut(hdc, 20, 15, statusText, lstrlen(statusText));
            
            if (gameMode == 3 && gameActive) {
                int barWidth = (410 * turnTimeLeftMs) / 7000;
                RECT timerBg = {20, 38, 430, 44};
                HBRUSH tBg = CreateSolidBrush(RGB(30, 40, 55));
                FillRect(hdc, &timerBg, tBg);
                DeleteObject(tBg);
                
                RECT timerFg = {20, 38, 20 + barWidth, 44};
                HBRUSH tFg = CreateSolidBrush((turnTimeLeftMs < 2000) ? RGB(255, 82, 82) : RGB(76, 175, 80));
                FillRect(hdc, &timerFg, tFg);
                DeleteObject(tFg);
            }
            
            SetTextColor(hdc, RGB(200, 200, 200));
            TextOut(hdc, 10, 630, "Press 'H' for Help Menu", 23);

            SetTextColor(hdc, RGB(170, 170, 170));
            char statsStr[128];
            wsprintf(statsStr, "Wins: Red %d, Yellow %d | Draws: %d | Streak: %d (Best: %d) | Max Stage: %d/20",
                     stats.redWins, stats.yellowWins, stats.draws, stats.streak, stats.bestStreak, stats.maxCampaignStage);
            TextOut(hdc, 10, 655, statsStr, lstrlen(statsStr));
            
            // --- Board Layout Calculations ---
            int colWidth = 40;
            int gap = 4;
            int boardW = g_cols * 44 + 10;
            int boardH = g_rows * 44 + 10;
            int boardLeft = (rect.right - rect.left - boardW) / 2;
            int boardTop = 50;

            if (g_screenShakeTimer > 0) {
                int shakeAmp = g_screenShakeTimer / 2;
                boardLeft += (rand() % (shakeAmp * 2 + 1)) - shakeAmp;
                boardTop += (rand() % (shakeAmp * 2 + 1)) - shakeAmp;
            }

            int tableTop = boardTop + boardH - 20;
            RECT tableRect = {0, tableTop, rect.right, rect.bottom};
            HBRUSH tableBg = CreateSolidBrush(RGB(139, 69, 19));
            FillRect(hdc, &tableRect, tableBg);
            DeleteObject(tableBg);
            
            HPEN tableEdge = CreatePen(PS_SOLID, 4, RGB(160, 82, 45));
            SelectObject(hdc, tableEdge);
            MoveToEx(hdc, 0, tableTop, NULL);
            LineTo(hdc, rect.right, tableTop);
            DeleteObject(tableEdge);

            RECT frameOuter = {boardLeft - 6, boardTop - 6, boardLeft + boardW + 6, boardTop + boardH + 6};
            HBRUSH frameBg = CreateSolidBrush(RGB(19, 45, 105));
            FillRect(hdc, &frameOuter, frameBg);
            DeleteObject(frameBg);

            HPEN topLight = CreatePen(PS_SOLID, 2, RGB(60, 115, 220));
            HPEN botDark  = CreatePen(PS_SOLID, 2, RGB(10, 22, 55));
            SelectObject(hdc, topLight);
            MoveToEx(hdc, boardLeft - 6, boardTop + boardH + 6, NULL); LineTo(hdc, boardLeft - 6, boardTop - 6); LineTo(hdc, boardLeft + boardW + 6, boardTop - 6);
            SelectObject(hdc, botDark);
            MoveToEx(hdc, boardLeft + boardW + 6, boardTop - 6, NULL); LineTo(hdc, boardLeft + boardW + 6, boardTop + boardH + 6); LineTo(hdc, boardLeft - 6, boardTop + boardH + 6);
            DeleteObject(topLight); DeleteObject(botDark);

            RECT boardRect = {boardLeft, boardTop, boardLeft + boardW, boardTop + boardH};
            HBRUSH boardBg = CreateSolidBrush(RGB(24, 60, 138));
            FillRect(hdc, &boardRect, boardBg);
            DeleteObject(boardBg);

            HRGN clipRgn = CreateRectRgn(boardLeft, boardTop, boardLeft + boardW, boardTop + boardH);
            SelectClipRgn(hdc, clipRgn);
            HPEN sheenPen = CreatePen(PS_SOLID, 10, RGB(45, 85, 175));
            SelectObject(hdc, sheenPen);
            for(int sh=boardLeft-boardH; sh<boardLeft+boardW; sh+=60) {
                MoveToEx(hdc, sh, boardTop, NULL);
                LineTo(hdc, sh+boardH, boardTop+boardH);
            }
            DeleteObject(sheenPen);
            SelectClipRgn(hdc, NULL);
            DeleteObject(clipRgn);
            
            HRGN hRgn = CreateRectRgn(boardLeft, boardTop, boardLeft + boardW, boardTop + boardH);
            SelectClipRgn(hdc, hRgn);

            int hoverRow = -1;
            if (hoverCol != -1 && !isAnimating && gameActive && !(gameMode > 0 && currentPlayer == 2)) {
                for (int r = 0; r < g_rows; r++) {
                    if (board[r][hoverCol] != 0) { hoverRow = r - 1; break; }
                }
                if (hoverRow == -1 && board[g_rows-1][hoverCol] == 0) hoverRow = g_rows - 1;
            }
            
            // Draw grid cells with 3D Glossy Discs
            for (int r = 0; r < g_rows; r++) {
                for (int c = 0; c < g_cols; c++) {
                    int x = boardLeft + 5 + c * 44;
                    int y = boardTop + 5 + r * 44;
                    
                    bool isWinCell = false;
                    for(int i=0; i<winCellCount; i++) {
                        if(winCells[i][0] == r && winCells[i][1] == c) {
                            isWinCell = true;
                            break;
                        }
                    }

                    int typeToDraw = board[r][c];
                    if (isAnimating && r == animRow && c == animCol) {
                        typeToDraw = 0;
                    } else if (board[r][c] == 0 && r == hoverRow && c == hoverCol) {
                        if (selectedPowerup == 1) typeToDraw = 500;
                        else if (selectedPowerup == 2) typeToDraw = 600;
                        else if (selectedPowerup == 3) typeToDraw = 700;
                        else typeToDraw = (currentPlayer == 1) ? 100 : 200;
                    }
                    
                    DrawDisc3D(hdc, x, y, typeToDraw, isWinCell);

                    // Frozen Column Overlay
                    if (frozenTurns > 0 && frozenCol == c) {
                        HBRUSH iceBrush = CreateSolidBrush(RGB(0, 180, 255));
                        HPEN icePen = CreatePen(PS_SOLID, 1, RGB(200, 240, 255));
                        SelectObject(hdc, icePen);
                        SelectObject(hdc, iceBrush);
                        Rectangle(hdc, x - 2, boardTop + 2, x + 38, boardTop + boardH - 2);
                        DeleteObject(iceBrush); DeleteObject(icePen);
                    }
                }
            }

            // AI Hint Beam
            if (hintCol >= 0 && hintCol < g_cols) {
                int hx = boardLeft + 5 + hintCol * 44;
                HPEN hintPen = CreatePen(PS_SOLID, 3, RGB(255, 215, 0));
                HBRUSH nullB = GetStockObject(NULL_BRUSH);
                SelectObject(hdc, hintPen); SelectObject(hdc, nullB);
                Rectangle(hdc, hx, boardTop + 2, hx + 36, boardTop + boardH - 2);
                DeleteObject(hintPen);
            }
            
            // Draw dropping disc
            if (isAnimating) {
                int x = boardLeft + 5 + animCol * 44;
                int dropType = (animType == 1) ? 5 : ((animType == 2) ? 6 : ((animType == 3) ? 7 : animPlayer));
                DrawDisc3D(hdc, x, animY, dropType, false);
            }

            // Draw Winning 4-in-a-row Neon Beam Line
            if (winCellCount >= 4) {
                int x1 = boardLeft + 5 + winCells[0][1] * 44 + 18;
                int y1 = boardTop + 5 + winCells[0][0] * 44 + 18;
                int x2 = boardLeft + 5 + winCells[winCellCount - 1][1] * 44 + 18;
                int y2 = boardTop + 5 + winCells[winCellCount - 1][0] * 44 + 18;

                int curX2 = x1 + (int)((x2 - x1) * winBeamProgress);
                int curY2 = y1 + (int)((y2 - y1) * winBeamProgress);

                HPEN auraPen = CreatePen(PS_SOLID, 12, RGB(0, 220, 255));
                SelectObject(hdc, auraPen);
                MoveToEx(hdc, x1, y1, NULL); LineTo(hdc, curX2, curY2);
                DeleteObject(auraPen);

                HPEN midPen = CreatePen(PS_SOLID, 6, RGB(160, 245, 255));
                SelectObject(hdc, midPen);
                MoveToEx(hdc, x1, y1, NULL); LineTo(hdc, curX2, curY2);
                DeleteObject(midPen);

                HPEN corePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                SelectObject(hdc, corePen);
                MoveToEx(hdc, x1, y1, NULL); LineTo(hdc, curX2, curY2);
                DeleteObject(corePen);
            }
            
            SelectClipRgn(hdc, NULL);
            DeleteObject(hRgn);

            // Draw Dust Motes
            if (g_dustInit) {
                HPEN nullP = GetStockObject(NULL_PEN);
                SelectObject(hdc, nullP);
                for(int i=0; i<MAX_DUST; i++) {
                    int alpha = (int)((sinf(g_dust[i].phase) + 1.0f) * 100.0f);
                    if (alpha < 0) alpha = 0;
                    if (alpha > 200) alpha = 200;
                    HBRUSH dBrush = CreateSolidBrush(RGB(200 + alpha/5, 200 + alpha/5, 255));
                    SelectObject(hdc, dBrush);
                    int px = (int)g_dust[i].x;
                    int py = (int)g_dust[i].y;
                    int sz = g_dust[i].size;
                    Ellipse(hdc, px - sz, py - sz, px + sz, py + sz);
                    DeleteObject(dBrush);
                }
            }

            // Draw Confetti Particles
            for (int i = 0; i < g_particleCount; i++) {
                if (g_particles[i].life > 0) {
                    HBRUSH pBrush = CreateSolidBrush(g_particles[i].color);
                    HPEN pPen = GetStockObject(NULL_PEN);
                    SelectObject(hdc, pBrush); SelectObject(hdc, pPen);
                    int px = (int)g_particles[i].x;
                    int py = (int)g_particles[i].y;
                    int sz = g_particles[i].size;
                    Rectangle(hdc, px - sz/2, py - sz/2, px + sz/2, py + sz/2);
                    DeleteObject(pBrush);
                }
            }
            
            EndPaint(hwnd, &ps);
            break;
        }
        
        case WM_MOUSEMOVE: {
            if (isAnimating || !gameActive || (gameMode > 0 && currentPlayer == 2)) {
                if (hoverCol != -1) {
                    hoverCol = -1;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                break;
            }
            int xPos = LOWORD(lParam);
            int yPos = HIWORD(lParam);
            int boardW = g_cols * 44 + 10;
            RECT rect; GetClientRect(hwnd, &rect);
            int boardLeft = (rect.right - rect.left - boardW) / 2;
            
            if (xPos >= boardLeft && xPos <= boardLeft + boardW && yPos >= 50 && yPos <= 50 + g_rows * 44 + 10) {
                int c = (xPos - boardLeft - 5) / 44;
                if (c >= 0 && c < g_cols) {
                    if (hoverCol != c) {
                        hoverCol = c;
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                } else if (hoverCol != -1) {
                    hoverCol = -1;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (hoverCol != -1) {
                hoverCol = -1;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hwnd;
            TrackMouseEvent(&tme);
            break;
        }
        case WM_MOUSELEAVE: {
            if (hoverCol != -1) {
                hoverCol = -1;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            if (isAnimating) break;
            if (!gameActive) {
                ResetGame();
                if (gameMode == 3 && gameActive) SetTimer(hwnd, 3, 100, NULL);
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            if (gameMode > 0 && currentPlayer == 2) break;
            int xPos = LOWORD(lParam);
            int yPos = HIWORD(lParam);
            int boardW = g_cols * 44 + 10;
            RECT rect; GetClientRect(hwnd, &rect);
            int boardLeft = (rect.right - rect.left - boardW) / 2;
            
            if (xPos >= boardLeft && xPos <= boardLeft + boardW && yPos >= 50 && yPos <= 50 + g_rows * 44 + 10) {
                int c = (xPos - boardLeft - 5) / 44;
                if (c >= 0 && c < g_cols) {
                    if (isFreezeMode) {
                        int fCount = (currentPlayer == 1) ? p1Freezes : p2Freezes;
                        if (fCount > 0) {
                            frozenCol = c;
                            frozenTurns = 2;
                            frozenPlayer = currentPlayer;
                            if (currentPlayer == 1) p1Freezes--; else p2Freezes--;
                            isFreezeMode = false;
                            PlaySoundEffect(9);
                            UpdatePowerupButtons();
                            InvalidateRect(hwnd, NULL, TRUE);
                        }
                    } else if (frozenTurns > 0 && frozenCol == c) {
                        PlaySoundEffect(4); // Column frozen
                    } else if (board[0][c] == 0) {
                        ExecuteDrop(hwnd, c, currentPlayer, selectedPowerup);
                    } else {
                        PlaySoundEffect(4);
                    }
                }
            }
            break;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    SetProcessDPIAware();
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    RECT winRect = { 0, 0, 580, 780 };
    AdjustWindowRect(&winRect, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, FALSE);
    hwnd = CreateWindowEx(
        0, g_szClassName, "KConnect4 - Loop 7 Campaign Expansion (Press H for Help)",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, winRect.right - winRect.left, winRect.bottom - winRect.top,
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while(GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}
