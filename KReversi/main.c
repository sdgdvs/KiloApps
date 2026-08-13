#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define EMPTY 0
#define BLACK 1
#define WHITE 2
#define BLOCKED 3
#define DOUBLE_FLIP 4

#define MODE_FREEPLAY 0
#define MODE_CAMPAIGN 1

#define AI_ROOKIE 0
#define AI_GREEDY 1
#define AI_POSITIONAL 2
#define AI_GRANDMASTER 3

#define IDM_CAMPAIGN_MODE 1000
#define IDM_FREEPLAY_MODE 1001

#define IDM_STAGE_BASE 1100
#define IDM_STAGE_1  1101
#define IDM_STAGE_2  1102
#define IDM_STAGE_3  1103
#define IDM_STAGE_4  1104
#define IDM_STAGE_5  1105
#define IDM_STAGE_6  1106
#define IDM_STAGE_7  1107
#define IDM_STAGE_8  1108
#define IDM_STAGE_9  1109
#define IDM_STAGE_10 1110
#define IDM_STAGE_11 1111
#define IDM_STAGE_12 1112
#define IDM_STAGE_13 1113
#define IDM_STAGE_14 1114
#define IDM_STAGE_15 1115
#define IDM_STAGE_16 1116
#define IDM_STAGE_17 1117
#define IDM_STAGE_18 1118
#define IDM_STAGE_19 1119
#define IDM_STAGE_20 1120

#define IDM_AI_ROOKIE 1200
#define IDM_AI_GREEDY 1201
#define IDM_AI_POSITIONAL 1202
#define IDM_AI_GRANDMASTER 1203

#define IDM_SIZE_6 1210
#define IDM_SIZE_8 1211
#define IDM_SIZE_10 1212

#define IDM_UNDO 1004
#define IDM_HINT 1009
#define IDM_BOMB 1018
#define IDM_FREEZE 1019

#define IDM_SOUND 1005
#define IDM_STATS 1006
#define IDM_SAVE 1007
#define IDM_LOAD 1008
#define IDM_HELP 1024

#define IDM_TIME_UNTIMED 1010
#define IDM_TIME_BLITZ 1011
#define IDM_TIME_RAPID 1012

#define IDM_THEME_CLASSIC 1020
#define IDM_THEME_CYBER 1021
#define IDM_THEME_CRIMSON 1022
#define IDM_THEME_TERMINAL 1023

typedef struct {
    COLORREF bg;
    COLORREF boardCell;
    COLORREF cellHover;
    COLORREF gridPen;
    COLORREF disc1;
    COLORREF disc2;
    COLORREF text;
    COLORREF evalPanelBg;
    COLORREF hintPen;
    const char* p1Name;
    const char* p2Name;
} BoardTheme;

static const BoardTheme g_themes[4] = {
    // 0: Classic Emerald
    { RGB(18, 18, 18), RGB(26, 86, 44), RGB(35, 120, 59), RGB(34, 34, 34), RGB(20, 20, 20), RGB(220, 220, 220), RGB(220, 220, 220), RGB(26, 26, 26), RGB(255, 215, 0), "Black", "White" },
    // 1: Midnight Cyber
    { RGB(6, 11, 25), RGB(11, 19, 43), RGB(28, 37, 65), RGB(0, 245, 212), RGB(0, 245, 212), RGB(255, 0, 127), RGB(224, 230, 237), RGB(13, 27, 42), RGB(255, 0, 127), "Cyan", "Pink" },
    // 2: Obsidian Crimson
    { RGB(20, 3, 6), RGB(58, 8, 16), RGB(84, 12, 23), RGB(90, 12, 24), RGB(255, 215, 0), RGB(217, 4, 41), RGB(247, 214, 216), RGB(40, 6, 12), RGB(255, 215, 0), "Gold", "Ruby" },
    // 3: Retro Terminal
    { RGB(2, 11, 2), RGB(7, 28, 7), RGB(17, 54, 17), RGB(0, 255, 102), RGB(255, 176, 0), RGB(51, 255, 51), RGB(51, 255, 51), RGB(9, 32, 9), RGB(255, 176, 0), "Amber", "Green" }
};

int currentTheme = 0;
const char g_szClassName[] = "KReversiClass";

int g_boardWidth = 8;
int g_boardHeight = 8;
int board[100]; // Max 10x10

int gameMode = MODE_FREEPLAY;
int campaignStage = 1;
int maxUnlockedStage = 1;

int aiDifficulty = AI_POSITIONAL;
int bombCount = 2;
int freezeCount = 1;
int isBombActive = 0;

int soundEnabled = 1;
int showHint = 1;
int stats[4] = {0, 0, 0, 1}; // Wins, Losses, Draws, MaxStageUnlocked

int moveTimeMode = 0; // 0=Untimed, 5=Blitz 5s, 15=Rapid 15s
int handicapMode = 0;
int moveTimeLeftDeci = 0;
int gameEnded = 0;
int currentPlayer = BLACK;
int hoverIdx = -1;

int gameOverSoundPlayed = 0;
int animatingFlips[100];
int numAnimatingFlips = 0;
int flipProgress = 0;

int dirs[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};

static const int g_weights6[36] = {
    100, -20,  10,  10, -20, 100,
    -20, -50,  -2,  -2, -50, -20,
     10,  -2,  -1,  -1,  -2,  10,
     10,  -2,  -1,  -1,  -2,  10,
    -20, -50,  -2,  -2, -50, -20,
    100, -20,  10,  10, -20, 100
};

static const int g_weights8[64] = {
    100, -20,  10,   5,   5,  10, -20, 100,
    -20, -50,  -2,  -2,  -2,  -2, -50, -20,
     10,  -2,  -1,  -1,  -1,  -1,  -2,  10,
      5,  -2,  -1,  -1,  -1,  -1,  -2,   5,
      5,  -2,  -1,  -1,  -1,  -1,  -2,   5,
     10,  -2,  -1,  -1,  -1,  -1,  -2,  10,
    -20, -50,  -2,  -2,  -2,  -2, -50, -20,
    100, -20,  10,   5,   5,  10, -20, 100
};

static const int g_weights10[100] = {
    100, -20,  15,   5,   5,   5,   5,  15, -20, 100,
    -20, -50,  -2,  -2,  -2,  -2,  -2,  -2, -50, -20,
     15,  -2,   1,   1,   1,   1,   1,   1,  -2,  15,
      5,  -2,   1,  -1,  -1,  -1,  -1,   1,  -2,   5,
      5,  -2,   1,  -1,  -1,  -1,  -1,   1,  -2,   5,
      5,  -2,   1,  -1,  -1,  -1,  -1,   1,  -2,   5,
      5,  -2,   1,  -1,  -1,  -1,  -1,   1,  -2,   5,
     15,  -2,   1,   1,   1,   1,   1,   1,  -2,  15,
    -20, -50,  -2,  -2,  -2,  -2,  -2,  -2, -50, -20,
    100, -20,  15,   5,   5,   5,   5,  15, -20, 100
};

typedef struct {
    int stageNum;
    const char* name;
    int width;
    int height;
    int aiDifficulty;
    int bombs;
    int freezes;
    const char* desc;
    int numHoles;
    int holes[16][2];
    int numBonusTiles;
    int bonusTiles[16][2];
    int customSetupCount;
    int customDiscs[16][3];
} CampaignStageData;

static const CampaignStageData g_campaignStages[20] = {
    { 1, "Initiation", 6, 6, AI_ROOKIE, 1, 1, "Small 6x6 board. Master the basics.", 0, {}, 0, {}, 0, {} },
    { 2, "Outpost Duel", 6, 6, AI_GREEDY, 1, 1, "6x6 board against an aggressive Greedy AI.", 0, {}, 0, {}, 0, {} },
    { 3, "The Vault", 6, 6, AI_ROOKIE, 1, 1, "6x6 board with 4 obstacle holes blocking corners.", 4, {{1,1},{1,4},{4,1},{4,4}}, 0, {}, 0, {} },
    { 4, "Classic Arena", 8, 8, AI_GREEDY, 1, 1, "Standard 8x8 battle vs Greedy AI.", 0, {}, 0, {}, 0, {} },
    { 5, "Fortress Gates", 8, 8, AI_POSITIONAL, 1, 1, "8x8 board with blocked corners and 2x bonus tiles!", 4, {{0,0},{0,7},{7,0},{7,7}}, 2, {{2,2},{5,5}}, 0, {} },
    { 6, "Crossfire Zone", 8, 8, AI_POSITIONAL, 1, 1, "4 inner obstacle holes split the center.", 4, {{2,2},{2,5},{5,2},{5,5}}, 2, {{1,3},{6,4}}, 0, {} },
    { 7, "Vanguard Siege", 8, 8, AI_GRANDMASTER, 2, 1, "Face Grandmaster AI! You start with 2 extra discs.", 0, {}, 0, {}, 2, {{2,3,BLACK},{5,4,BLACK}} },
    { 8, "Great Colosseum", 10, 10, AI_ROOKIE, 2, 2, "Massive 10x10 arena! Control the expansive board.", 0, {}, 4, {{3,3},{3,6},{6,3},{6,6}}, 0, {} },
    { 9, "Greedy Leviathan", 10, 10, AI_GREEDY, 2, 2, "10x10 board vs high-disc Greedy AI.", 0, {}, 4, {{2,4},{4,7},{7,5},{5,2}}, 0, {} },
    { 10, "Citadel Runes", 10, 10, AI_POSITIONAL, 2, 2, "10x10 board with 8 corner & subcorner holes.", 8, {{0,0},{0,9},{9,0},{9,9},{1,1},{1,8},{8,1},{8,8}}, 4, {{3,4},{4,5},{5,4},{6,5}}, 0, {} },
    { 11, "Diamond Ring", 8, 8, AI_POSITIONAL, 2, 2, "8x8 board with an outer ring of obstacles.", 8, {{1,3},{1,4},{3,1},{4,1},{3,6},{4,6},{6,3},{6,4}}, 4, {{2,2},{2,5},{5,2},{5,5}}, 0, {} },
    { 12, "Labyrinth Grid", 10, 10, AI_GRANDMASTER, 2, 2, "10x10 maze layout with 8 obstacle blocks vs Grandmaster.", 8, {{2,2},{2,7},{7,2},{7,7},{4,2},{4,7},{2,4},{7,4}}, 4, {{3,3},{3,6},{6,3},{6,6}}, 0, {} },
    { 13, "Blitz Trench", 6, 6, AI_GRANDMASTER, 2, 2, "Fast 6x6 board with 5s Blitz timer vs Grandmaster!", 0, {}, 2, {{1,2},{4,3}}, 0, {} },
    { 14, "Dragon's Den", 8, 8, AI_GRANDMASTER, 2, 2, "8x8 board with 4 dragon holes on edge centers.", 4, {{0,3},{3,0},{4,7},{7,4}}, 4, {{2,3},{3,5},{5,2},{4,4}}, 0, {} },
    { 15, "Eclipse Chamber", 10, 10, AI_POSITIONAL, 2, 2, "10x10 board with 12 obstacles surrounding center.", 12, {{1,1},{1,8},{8,1},{8,8},{3,1},{6,1},{3,8},{6,8},{1,3},{1,6},{8,3},{8,6}}, 4, {{4,2},{5,7},{2,5},{7,4}}, 0, {} },
    { 16, "Phantom Matrix", 8, 8, AI_GRANDMASTER, 2, 2, "8x8 board with 6 staggered holes and 4 bonus tiles.", 6, {{0,2},{2,7},{5,0},{7,5},{3,3},{4,4}}, 4, {{1,1},{1,6},{6,1},{6,6}}, 0, {} },
    { 17, "Warlord's Gambit", 10, 10, AI_POSITIONAL, 3, 2, "10x10 battlefield with 8 holes and 6 bonus tiles.", 8, {{0,4},{4,0},{5,9},{9,5},{2,2},{2,7},{7,2},{7,7}}, 6, {{3,2},{2,3},{6,7},{7,6},{3,7},{7,3}}, 0, {} },
    { 18, "Titan Crucible", 10, 10, AI_GRANDMASTER, 3, 2, "10x10 arena with 10 holes vs Grandmaster Minimax.", 10, {{1,4},{1,5},{8,4},{8,5},{4,1},{5,1},{4,8},{5,8},{0,0},{9,9}}, 6, {{2,4},{7,5},{4,2},{5,7},{3,3},{6,6}}, 0, {} },
    { 19, "Iron Apex", 8, 8, AI_GRANDMASTER, 3, 2, "Tight 8x8 board with 4 holes vs ruthless Grandmaster AI.", 4, {{1,2},{2,5},{5,2},{6,5}}, 4, {{0,1},{1,6},{6,1},{7,6}}, 0, {} },
    { 20, "Reversi Grandmaster Challenge", 10, 10, AI_GRANDMASTER, 3, 3, "The ultimate 10x10 stage against the Supreme Reversi Grandmaster!", 12, {{0,0},{0,9},{9,0},{9,9},{2,2},{2,7},{7,2},{7,7},{4,1},{5,8},{1,5},{8,4}}, 6, {{3,4},{4,3},{5,6},{6,5},{1,1},{8,8}}, 0, {} }
};

typedef struct {
    int board[100];
    int boardWidth;
    int boardHeight;
    int currentPlayer;
    int bombCount;
    int freezeCount;
    int isBombActive;
    int gameEnded;
} HistoryState;

HistoryState history[300];
int historyCount = 0;

typedef struct {
    float x, y;
    float vx, vy;
    int life;
    int maxLife;
    COLORREF color;
    int size;
} GdiParticle;

#define MAX_GDI_PARTICLES 150
static GdiParticle g_particles[MAX_GDI_PARTICLES];
static int g_numParticles = 0;

void SpawnSparksGDI(float x, float y, int count, COLORREF color) {
    for (int i = 0; i < count; i++) {
        if (g_numParticles >= MAX_GDI_PARTICLES) break;
        float angle = ((float)rand() / (float)RAND_MAX) * 6.28318f;
        float speed = 1.5f + ((float)rand() / (float)RAND_MAX) * 4.5f;
        g_particles[g_numParticles].x = x;
        g_particles[g_numParticles].y = y;
        g_particles[g_numParticles].vx = cosf(angle) * speed;
        g_particles[g_numParticles].vy = sinf(angle) * speed - 1.0f;
        g_particles[g_numParticles].life = 15 + rand() % 15;
        g_particles[g_numParticles].maxLife = g_particles[g_numParticles].life;
        g_particles[g_numParticles].color = color;
        g_particles[g_numParticles].size = 2 + rand() % 3;
        g_numParticles++;
    }
}

void SpawnVictoryFireworksGDI(int w, int h) {
    COLORREF colors[6] = { RGB(255, 215, 0), RGB(255, 0, 127), RGB(0, 245, 212), RGB(255, 255, 255), RGB(255, 150, 0), RGB(50, 255, 50) };
    for (int i = 0; i < 80; i++) {
        if (g_numParticles >= MAX_GDI_PARTICLES) break;
        g_particles[g_numParticles].x = (float)(rand() % w);
        g_particles[g_numParticles].y = (float)(rand() % (h / 3 + 10));
        g_particles[g_numParticles].vx = -3.0f + ((float)rand() / (float)RAND_MAX) * 6.0f;
        g_particles[g_numParticles].vy = 1.5f + ((float)rand() / (float)RAND_MAX) * 4.0f;
        g_particles[g_numParticles].life = 30 + rand() % 25;
        g_particles[g_numParticles].maxLife = g_particles[g_numParticles].life;
        g_particles[g_numParticles].color = colors[rand() % 6];
        g_particles[g_numParticles].size = 3 + rand() % 3;
        g_numParticles++;
    }
}

void UpdateAndDrawParticlesGDI(HDC hdc) {
    for (int i = g_numParticles - 1; i >= 0; i--) {
        g_particles[i].x += g_particles[i].vx;
        g_particles[i].y += g_particles[i].vy;
        g_particles[i].life--;
        if (g_particles[i].life <= 0) {
            g_particles[i] = g_particles[--g_numParticles];
            continue;
        }
        HBRUSH pBrush = CreateSolidBrush(g_particles[i].color);
        HPEN pPen = CreatePen(PS_SOLID, 1, g_particles[i].color);
        SelectObject(hdc, pBrush);
        SelectObject(hdc, pPen);
        int s = g_particles[i].size;
        int px = (int)g_particles[i].x;
        int py = (int)g_particles[i].y;
        Ellipse(hdc, px - s, py - s, px + s, py + s);
        DeleteObject(pBrush);
        DeleteObject(pPen);
    }
}

void DrawDisc3D(HDC hdc, int cx, int cy, int radius, int colorType, int scaleXWidth, int yOffset) {
    int w = scaleXWidth;
    if (w < 2) w = 2;
    int rX = w / 2;
    int rY = radius;
    int drawCy = cy + yOffset;

    // Drop Shadow
    HBRUSH shadowBrush = CreateSolidBrush(RGB(10, 10, 10));
    HPEN shadowPen = CreatePen(PS_SOLID, 1, RGB(10, 10, 10));
    SelectObject(hdc, shadowBrush);
    SelectObject(hdc, shadowPen);
    Ellipse(hdc, cx - rX + 3, drawCy - rY + 4, cx + rX + 3, drawCy + rY + 4);
    DeleteObject(shadowBrush);
    DeleteObject(shadowPen);

    if (colorType == BLACK) {
        // Obsidian Black Disc
        HBRUSH rimBrush = CreateSolidBrush(RGB(25, 25, 25));
        HPEN rimPen = CreatePen(PS_SOLID, 2, RGB(212, 175, 55));
        SelectObject(hdc, rimBrush);
        SelectObject(hdc, rimPen);
        Ellipse(hdc, cx - rX, drawCy - rY, cx + rX, drawCy + rY);
        DeleteObject(rimBrush);
        DeleteObject(rimPen);

        for (int step = 0; step < 3; step++) {
            int stepRX = rX - (step * rX / 4) - 2;
            int stepRY = rY - (step * rY / 4) - 2;
            if (stepRX < 1 || stepRY < 1) break;
            int offsetShiftX = cx - (step * rX / 8);
            int offsetShiftY = drawCy - (step * rY / 8);
            COLORREF col = RGB(20 + step * 25, 20 + step * 25, 20 + step * 25);
            HBRUSH cBrush = CreateSolidBrush(col);
            HPEN cPen = CreatePen(PS_SOLID, 1, col);
            SelectObject(hdc, cBrush);
            SelectObject(hdc, cPen);
            Ellipse(hdc, offsetShiftX - stepRX, offsetShiftY - stepRY, offsetShiftX + stepRX, offsetShiftY + stepRY);
            DeleteObject(cBrush);
            DeleteObject(cPen);
        }

        HPEN specPen = CreatePen(PS_SOLID, 2, RGB(180, 180, 180));
        SelectObject(hdc, specPen);
        Arc(hdc, cx - rX + 4, drawCy - rY + 4, cx + rX - 4, drawCy + rY - 4, cx, drawCy - rY, cx - rX, drawCy);
        DeleteObject(specPen);

        if (rX >= 10) {
            POINT crownPts[7];
            crownPts[0].x = cx - rX / 3;     crownPts[0].y = drawCy + rY / 3;
            crownPts[1].x = cx - rX * 4 / 10; crownPts[1].y = drawCy - rY / 5;
            crownPts[2].x = cx - rX / 6;     crownPts[2].y = drawCy;
            crownPts[3].x = cx;           crownPts[3].y = drawCy - rY / 3;
            crownPts[4].x = cx + rX / 6;     crownPts[4].y = drawCy;
            crownPts[5].x = cx + rX * 4 / 10; crownPts[5].y = drawCy - rY / 5;
            crownPts[6].x = cx + rX / 3;     crownPts[6].y = drawCy + rY / 3;

            HBRUSH goldBrush = CreateSolidBrush(RGB(255, 215, 0));
            HPEN goldPen = CreatePen(PS_SOLID, 1, RGB(180, 140, 20));
            SelectObject(hdc, goldBrush);
            SelectObject(hdc, goldPen);
            Polygon(hdc, crownPts, 7);
            DeleteObject(goldBrush);
            DeleteObject(goldPen);
        }
    } else {
        // Pearl White Disc
        HBRUSH rimBrush = CreateSolidBrush(RGB(240, 245, 250));
        HPEN rimPen = CreatePen(PS_SOLID, 2, RGB(180, 190, 200));
        SelectObject(hdc, rimBrush);
        SelectObject(hdc, rimPen);
        Ellipse(hdc, cx - rX, drawCy - rY, cx + rX, drawCy + rY);
        DeleteObject(rimBrush);
        DeleteObject(rimPen);

        for (int step = 0; step < 3; step++) {
            int stepRX = rX - (step * rX / 4) - 2;
            int stepRY = rY - (step * rY / 4) - 2;
            if (stepRX < 1 || stepRY < 1) break;
            int offsetShiftX = cx - (step * rX / 8);
            int offsetShiftY = drawCy - (step * rY / 8);
            COLORREF col = RGB(220 + step * 12, 225 + step * 10, 230 + step * 8);
            HBRUSH cBrush = CreateSolidBrush(col);
            HPEN cPen = CreatePen(PS_SOLID, 1, col);
            SelectObject(hdc, cBrush);
            SelectObject(hdc, cPen);
            Ellipse(hdc, offsetShiftX - stepRX, offsetShiftY - stepRY, offsetShiftX + stepRX, offsetShiftY + stepRY);
            DeleteObject(cBrush);
            DeleteObject(cPen);
        }

        HPEN specPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
        SelectObject(hdc, specPen);
        Arc(hdc, cx - rX + 4, drawCy - rY + 4, cx + rX - 4, drawCy + rY - 4, cx, drawCy - rY, cx - rX, drawCy);
        DeleteObject(specPen);

        if (rX >= 10) {
            POINT starPts[10];
            starPts[0].x = cx;             starPts[0].y = drawCy - rY / 3;
            starPts[1].x = cx + rX / 10;     starPts[1].y = drawCy - rY / 10;
            starPts[2].x = cx + rX / 3;      starPts[2].y = drawCy - rY / 12;
            starPts[3].x = cx + rX / 7;      starPts[3].y = drawCy + rY / 12;
            starPts[4].x = cx + rX / 4;      starPts[4].y = drawCy + rY / 3;
            starPts[5].x = cx;             starPts[5].y = drawCy + rY / 5;
            starPts[6].x = cx - rX / 4;      starPts[6].y = drawCy + rY / 3;
            starPts[7].x = cx - rX / 7;      starPts[7].y = drawCy + rY / 12;
            starPts[8].x = cx - rX / 3;      starPts[8].y = drawCy - rY / 12;
            starPts[9].x = cx - rX / 10;     starPts[9].y = drawCy - rY / 10;

            HBRUSH starBrush = CreateSolidBrush(RGB(80, 95, 115));
            HPEN starPen = CreatePen(PS_SOLID, 1, RGB(50, 65, 85));
            SelectObject(hdc, starBrush);
            SelectObject(hdc, starPen);
            Polygon(hdc, starPts, 10);
            DeleteObject(starBrush);
            DeleteObject(starPen);
        }
    }
}

void DrawMahoganyFrame(HDC hdc, int boardX, int boardY, int boardW, int boardH) {
    int padding = 16;
    RECT frameRect = { boardX - padding, boardY - padding, boardX + boardW + padding, boardY + boardH + padding };

    HBRUSH m1Brush = CreateSolidBrush(RGB(74, 31, 10));
    HPEN m1Pen = CreatePen(PS_SOLID, 3, RGB(140, 60, 20));
    SelectObject(hdc, m1Brush);
    SelectObject(hdc, m1Pen);
    RoundRect(hdc, frameRect.left, frameRect.top, frameRect.right, frameRect.bottom, 14, 14);
    DeleteObject(m1Brush);
    DeleteObject(m1Pen);

    RECT innerFrame = { boardX - 3, boardY - 3, boardX + boardW + 3, boardY + boardH + 3 };
    HBRUSH m2Brush = CreateSolidBrush(RGB(25, 10, 4));
    HPEN m2Pen = CreatePen(PS_SOLID, 2, RGB(90, 35, 12));
    SelectObject(hdc, m2Brush);
    SelectObject(hdc, m2Pen);
    Rectangle(hdc, innerFrame.left, innerFrame.top, innerFrame.right, innerFrame.bottom);
    DeleteObject(m2Brush);
    DeleteObject(m2Pen);

    int studCoords[4][2] = {
        { frameRect.left + 8, frameRect.top + 8 },
        { frameRect.right - 8, frameRect.top + 8 },
        { frameRect.left + 8, frameRect.bottom - 8 },
        { frameRect.right - 8, frameRect.bottom - 8 }
    };
    for (int i = 0; i < 4; i++) {
        int sx = studCoords[i][0];
        int sy = studCoords[i][1];
        HBRUSH studBrush = CreateSolidBrush(RGB(220, 180, 50));
        HPEN studPen = CreatePen(PS_SOLID, 1, RGB(120, 90, 20));
        SelectObject(hdc, studBrush);
        SelectObject(hdc, studPen);
        Ellipse(hdc, sx - 4, sy - 4, sx + 4, sy + 4);
        DeleteObject(studBrush);
        DeleteObject(studPen);

        HBRUSH hlBrush = CreateSolidBrush(RGB(255, 240, 180));
        SelectObject(hdc, hlBrush);
        Ellipse(hdc, sx - 2, sy - 2, sx, sy);
        DeleteObject(hlBrush);
    }
}

const int* GetWeightMatrix(int W, int H) {
    if (W == 6) return g_weights6;
    if (W == 10) return g_weights10;
    return g_weights8;
}

void LoadTheme() {
    FILE* f = fopen("kreversi_theme.dat", "rb");
    if (f) {
        fread(&currentTheme, sizeof(int), 1, f);
        fclose(f);
        if (currentTheme < 0 || currentTheme > 3) currentTheme = 0;
    }
}

void SaveTheme() {
    FILE* f = fopen("kreversi_theme.dat", "wb");
    if (f) {
        fwrite(&currentTheme, sizeof(int), 1, f);
        fclose(f);
    }
}

void LoadStats() {
    FILE* f = fopen("kreversi_stats.dat", "rb");
    if (f) {
        fread(stats, sizeof(int), 4, f);
        fclose(f);
        if (stats[3] < 1) stats[3] = 1;
        maxUnlockedStage = stats[3];
    }
}

void SaveStats() {
    stats[3] = maxUnlockedStage;
    FILE* f = fopen("kreversi_stats.dat", "wb");
    if (f) {
        fwrite(stats, sizeof(int), 4, f);
        fclose(f);
    }
}

void UpdateStats(int bCount, int wCount) {
    if (bCount > wCount) {
        stats[0]++;
        if (gameMode == MODE_CAMPAIGN && campaignStage == maxUnlockedStage && maxUnlockedStage < 20) {
            maxUnlockedStage++;
        }
    } else if (wCount > bCount) {
        stats[1]++;
    } else {
        stats[2]++;
    }
    SaveStats();
}

void ShowStats(HWND hwnd) {
    char msg[256];
    sprintf(msg, "Games Played: %d\nWins: %d\nLosses: %d\nDraws: %d\nMax Campaign Stage: %d / 20",
            stats[0]+stats[1]+stats[2], stats[0], stats[1], stats[2], maxUnlockedStage);
    MessageBox(hwnd, msg, "KReversi Statistics", MB_OK | MB_ICONINFORMATION);
}

void ShowHelp(HWND hwnd) {
    const char* helpText =
        "=== KREVERSI HOW TO PLAY & STRATEGY GUIDE ===\n\n"
        "1. CORE RULES & OBJECTIVE\n"
        "• Objective: Have the majority of your colored discs when neither player has legal moves.\n"
        "• Sandwich & Flip: Trap opponent discs horizontally, vertically, or diagonally.\n"
        "• Blocked Cells: Obstacle holes cannot hold discs and block flips across them.\n"
        "• Double-Flip Bonus Tiles (2X): Landing or flipping onto a 2X tile triggers an explosive 3x3 bonus flip!\n\n"
        "2. POWER-UPS & SHORTCUTS\n"
        "• Optimal Hint (H): Highlights top-evaluated move based on positional weights & minimax.\n"
        "• Undo Move (U): Reverts last turn move pair against AI.\n"
        "• Bomb Disc (B): Place a disc on ANY empty cell to flip 3x3 surrounding enemy discs!\n"
        "• Freeze AI (F): Forces the AI to skip 1 turn, giving you an extra turn!\n\n"
        "3. CAMPAIGN MODE (20 STAGES)\n"
        "• Conquer 20 diverse stages with dynamic 6x6, 8x8, and 10x10 boards, blocked holes, 2X bonus tiles, and 4 AI personalities (Rookie, Greedy, Positional, Grandmaster Minimax).\n\n"
        "4. CORNER & MOBILITY STRATEGY\n"
        "• Corners (A1, H1, etc.) can NEVER be flipped once captured.\n"
        "• Avoid X-squares (B2, G2) adjacent to open corners!";

    MessageBox(hwnd, helpText, "KReversi Rules & Strategy Guide", MB_OK | MB_ICONINFORMATION);
}

void PlaySoundEffect(int type) {
    if (!soundEnabled) return;
    if (type == 1) { // Place
        Beep(600, 50);
    } else if (type == 2) { // Game Over
        Beep(400, 150);
        Beep(200, 150);
    } else if (type == 3) { // Bomb
        Beep(220, 80);
        Beep(140, 120);
    } else if (type == 4) { // Freeze
        Beep(800, 80);
        Beep(1200, 120);
    } else if (type == 5) { // Bonus
        Beep(1000, 60);
        Beep(1400, 80);
        Beep(1800, 100);
    }
}

void SetupBoard() {
    int totalCells = g_boardWidth * g_boardHeight;
    for (int i = 0; i < totalCells; i++) board[i] = EMPTY;

    if (gameMode == MODE_CAMPAIGN) {
        const CampaignStageData* stage = &g_campaignStages[campaignStage - 1];
        g_boardWidth = stage->width;
        g_boardHeight = stage->height;
        aiDifficulty = stage->aiDifficulty;
        bombCount = stage->bombs;
        freezeCount = stage->freezes;
        moveTimeMode = (campaignStage == 13 ? 5 : 0);

        for (int i = 0; i < g_boardWidth * g_boardHeight; i++) board[i] = EMPTY;

        // Apply holes
        for (int i = 0; i < stage->numHoles; i++) {
            int hr = stage->holes[i][0];
            int hc = stage->holes[i][1];
            if (hr >= 0 && hr < g_boardHeight && hc >= 0 && hc < g_boardWidth) {
                board[hr * g_boardWidth + hc] = BLOCKED;
            }
        }

        // Apply bonus double-flip tiles
        for (int i = 0; i < stage->numBonusTiles; i++) {
            int br = stage->bonusTiles[i][0];
            int bc = stage->bonusTiles[i][1];
            if (br >= 0 && br < g_boardHeight && bc >= 0 && bc < g_boardWidth) {
                if (board[br * g_boardWidth + bc] == EMPTY) {
                    board[br * g_boardWidth + bc] = DOUBLE_FLIP;
                }
            }
        }

        // Standard center 4 setup
        int midR = g_boardHeight / 2;
        int midC = g_boardWidth / 2;
        board[(midR - 1) * g_boardWidth + (midC - 1)] = WHITE;
        board[(midR - 1) * g_boardWidth + midC] = BLACK;
        board[midR * g_boardWidth + (midC - 1)] = BLACK;
        board[midR * g_boardWidth + midC] = WHITE;

        // Custom discs
        for (int i = 0; i < stage->customSetupCount; i++) {
            int cr = stage->customDiscs[i][0];
            int cc = stage->customDiscs[i][1];
            int color = stage->customDiscs[i][2];
            if (cr >= 0 && cr < g_boardHeight && cc >= 0 && cc < g_boardWidth) {
                board[cr * g_boardWidth + cc] = color;
            }
        }
    } else {
        // Free play mode
        bombCount = 2;
        freezeCount = 2;
        int midR = g_boardHeight / 2;
        int midC = g_boardWidth / 2;
        board[(midR - 1) * g_boardWidth + (midC - 1)] = WHITE;
        board[(midR - 1) * g_boardWidth + midC] = BLACK;
        board[midR * g_boardWidth + (midC - 1)] = BLACK;
        board[midR * g_boardWidth + midC] = WHITE;

        if (g_boardWidth == 8) {
            if (handicapMode == 1) { // Black Corner
                board[0] = BLACK;
            } else if (handicapMode == 2) { // White Corner
                board[63] = WHITE;
            } else if (handicapMode == 3) { // Black Adv
                board[19] = BLACK; board[44] = BLACK;
            } else if (handicapMode == 4) { // 4 Corners
                board[0] = BLACK; board[7] = WHITE;
                board[56] = WHITE; board[63] = BLACK;
            }
        }
    }
}

void ResetMoveTimer(HWND hwnd) {
    if (hwnd) KillTimer(hwnd, 3);
    if (moveTimeMode > 0 && !gameEnded && hwnd) {
        moveTimeLeftDeci = moveTimeMode * 10;
        SetTimer(hwnd, 3, 100, NULL);
    }
}

void InitGame(HWND hwnd) {
    SetupBoard();
    currentPlayer = BLACK;
    historyCount = 0;
    gameOverSoundPlayed = 0;
    gameEnded = 0;
    isBombActive = 0;
    numAnimatingFlips = 0;
    if (hwnd) {
        KillTimer(hwnd, 1);
        KillTimer(hwnd, 2);
        ResetMoveTimer(hwnd);
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void PushHistory() {
    if (historyCount < 300) {
        for (int i = 0; i < g_boardWidth * g_boardHeight; i++) history[historyCount].board[i] = board[i];
        history[historyCount].boardWidth = g_boardWidth;
        history[historyCount].boardHeight = g_boardHeight;
        history[historyCount].currentPlayer = currentPlayer;
        history[historyCount].bombCount = bombCount;
        history[historyCount].freezeCount = freezeCount;
        history[historyCount].isBombActive = isBombActive;
        history[historyCount].gameEnded = gameEnded;
        historyCount++;
    }
}

void UndoMove(HWND hwnd) {
    KillTimer(hwnd, 1);
    KillTimer(hwnd, 2);
    numAnimatingFlips = 0;
    isBombActive = 0;
    if (historyCount == 0) return;
    
    HistoryState lastState = history[--historyCount];
    while(lastState.currentPlayer != BLACK && historyCount > 0) {
        lastState = history[--historyCount];
    }
    
    g_boardWidth = lastState.boardWidth;
    g_boardHeight = lastState.boardHeight;
    for(int i = 0; i < g_boardWidth * g_boardHeight; i++) board[i] = lastState.board[i];
    currentPlayer = lastState.currentPlayer;
    bombCount = lastState.bombCount;
    freezeCount = lastState.freezeCount;
    gameEnded = 0;
    gameOverSoundPlayed = 0;
    
    ResetMoveTimer(hwnd);
    InvalidateRect(hwnd, NULL, TRUE);
}

void DoFreezeAI(HWND hwnd) {
    if (freezeCount <= 0 || gameEnded || currentPlayer != BLACK) return;
    freezeCount--;
    if (hwnd) KillTimer(hwnd, 1);
    currentPlayer = BLACK;
    PlaySoundEffect(4);
    RECT clientRect;
    if (hwnd) GetClientRect(hwnd, &clientRect);
    else { clientRect.right = 480; clientRect.bottom = 560; }
    SpawnSparksGDI((float)(clientRect.right / 2), (float)(clientRect.bottom / 2), 35, RGB(0, 245, 212));
    if (hwnd) SetTimer(hwnd, 4, 25, NULL);
    ResetMoveTimer(hwnd);
    InvalidateRect(hwnd, NULL, TRUE);
}

int GetFlippableOnBoard(const int* b, int W, int H, int index, int player, int* flippable) {
    if (index < 0 || index >= W * H) return 0;
    if (b[index] != EMPTY && b[index] != DOUBLE_FLIP) return 0;
    int r = index / W;
    int c = index % W;
    int opponent = (player == BLACK) ? WHITE : BLACK;
    int count = 0;

    for (int d = 0; d < 8; d++) {
        int nr = r + dirs[d][0];
        int nc = c + dirs[d][1];
        int curDirFlippable[16];
        int curCount = 0;

        while (nr >= 0 && nr < H && nc >= 0 && nc < W && b[nr * W + nc] == opponent) {
            curDirFlippable[curCount++] = nr * W + nc;
            nr += dirs[d][0];
            nc += dirs[d][1];
        }
        if (nr >= 0 && nr < H && nc >= 0 && nc < W && b[nr * W + nc] == player) {
            for (int i = 0; i < curCount; i++) {
                flippable[count++] = curDirFlippable[i];
            }
        }
    }
    return count;
}

int GetFlippable(int index, int player, int* flippable) {
    return GetFlippableOnBoard(board, g_boardWidth, g_boardHeight, index, player, flippable);
}

int HasValidMovesOnBoard(const int* b, int W, int H, int player) {
    int dummy[100];
    for(int i = 0; i < W * H; i++) {
        if((b[i] == EMPTY || b[i] == DOUBLE_FLIP) && GetFlippableOnBoard(b, W, H, i, player, dummy) > 0) return 1;
    }
    return 0;
}

int HasValidMoves(int player) {
    return HasValidMovesOnBoard(board, g_boardWidth, g_boardHeight, player);
}

int EvaluateBoardState(const int* b, int W, int H, int player) {
    const int* weights = GetWeightMatrix(W, H);
    int score = 0;
    int pCount = 0, oppCount = 0;
    int opponent = (player == BLACK) ? WHITE : BLACK;

    for (int i = 0; i < W * H; i++) {
        if (b[i] == player) {
            score += weights[i] + 5;
            pCount++;
        } else if (b[i] == opponent) {
            score -= (weights[i] + 5);
            oppCount++;
        }
    }
    score += (pCount - oppCount) * 2;
    return score;
}

int Minimax(int* b, int W, int H, int depth, int alpha, int beta, int isMaxPlayer, int player) {
    if (depth == 0) {
        return EvaluateBoardState(b, W, H, player);
    }
    int opponent = (player == BLACK) ? WHITE : BLACK;
    int currPlayer = isMaxPlayer ? player : opponent;
    
    int moves[100];
    int moveCount = 0;
    for (int i = 0; i < W * H; i++) {
        if (b[i] == EMPTY || b[i] == DOUBLE_FLIP) {
            int dummy[100];
            if (GetFlippableOnBoard(b, W, H, i, currPlayer, dummy) > 0) {
                moves[moveCount++] = i;
            }
        }
    }
    
    if (moveCount == 0) {
        if (!HasValidMovesOnBoard(b, W, H, opponent)) {
            return EvaluateBoardState(b, W, H, player);
        }
        return Minimax(b, W, H, depth - 1, alpha, beta, !isMaxPlayer, player);
    }
    
    if (isMaxPlayer) {
        int maxEval = -999999;
        for (int i = 0; i < moveCount; i++) {
            int tempBoard[100];
            memcpy(tempBoard, b, W * H * sizeof(int));
            
            // Apply move
            int flips[100];
            int count = GetFlippableOnBoard(tempBoard, W, H, moves[i], currPlayer, flips);
            tempBoard[moves[i]] = currPlayer;
            for (int f = 0; f < count; f++) tempBoard[flips[f]] = currPlayer;
            
            int eval = Minimax(tempBoard, W, H, depth - 1, alpha, beta, 0, player);
            if (eval > maxEval) maxEval = eval;
            if (eval > alpha) alpha = eval;
            if (beta <= alpha) break;
        }
        return maxEval;
    } else {
        int minEval = 999999;
        for (int i = 0; i < moveCount; i++) {
            int tempBoard[100];
            memcpy(tempBoard, b, W * H * sizeof(int));
            
            // Apply move
            int flips[100];
            int count = GetFlippableOnBoard(tempBoard, W, H, moves[i], currPlayer, flips);
            tempBoard[moves[i]] = currPlayer;
            for (int f = 0; f < count; f++) tempBoard[flips[f]] = currPlayer;
            
            int eval = Minimax(tempBoard, W, H, depth - 1, alpha, beta, 1, player);
            if (eval < minEval) minEval = eval;
            if (eval < beta) beta = eval;
            if (beta <= alpha) break;
        }
        return minEval;
    }
}

int GetBestMoveForPlayer(int player) {
    int moves[100];
    int moveCount = 0;
    for (int i = 0; i < g_boardWidth * g_boardHeight; i++) {
        if (board[i] == EMPTY || board[i] == DOUBLE_FLIP) {
            int dummy[100];
            if (GetFlippable(i, player, dummy) > 0) {
                moves[moveCount++] = i;
            }
        }
    }
    if (moveCount == 0) return -1;

    int bestMove = moves[0];
    int maxVal = -999999;
    const int* weights = GetWeightMatrix(g_boardWidth, g_boardHeight);

    for (int i = 0; i < moveCount; i++) {
        int m = moves[i];
        int flips[100];
        int count = GetFlippable(m, player, flips);
        
        int val = 0;
        if (aiDifficulty == AI_ROOKIE) {
            val = rand() % 100;
        } else if (aiDifficulty == AI_GREEDY) {
            val = count * 10;
        } else if (aiDifficulty == AI_POSITIONAL) {
            val = count * 2 + weights[m];
        } else { // AI_GRANDMASTER Minimax
            int tempBoard[100];
            memcpy(tempBoard, board, g_boardWidth * g_boardHeight * sizeof(int));
            tempBoard[m] = player;
            for (int f = 0; f < count; f++) tempBoard[flips[f]] = player;
            int depth = (g_boardWidth == 10) ? 5 : ((g_boardWidth == 8) ? 6 : 7);
            val = Minimax(tempBoard, g_boardWidth, g_boardHeight, depth, -999999, 999999, 0, player);
        }

        if (val > maxVal) {
            maxVal = val;
            bestMove = m;
        } else if (val == maxVal && (rand() % 2 == 0)) {
            bestMove = m;
        }
    }
    return bestMove;
}

void DoMove(int index, int player, HWND hwnd) {
    int count = GetFlippable(index, player, animatingFlips);
    if (count > 0) {
        int hasBonusTile = (board[index] == DOUBLE_FLIP);
        for (int i = 0; i < count; i++) {
            if (board[animatingFlips[i]] == DOUBLE_FLIP) hasBonusTile = 1;
        }

        board[index] = player;
        numAnimatingFlips = count;
        flipProgress = 0;
        SetTimer(hwnd, 2, 30, NULL);
        
        int cellSize = (g_boardWidth == 6) ? 55 : ((g_boardWidth == 8) ? 45 : 38);
        int boardStartX = 20;
        int boardStartY = 95;
        int r = index / g_boardWidth;
        int c = index % g_boardWidth;
        float cx = (float)(boardStartX + c * cellSize + cellSize / 2);
        float cy = (float)(boardStartY + r * cellSize + cellSize / 2);

        if (hasBonusTile) {
            SpawnSparksGDI(cx, cy, 30, RGB(255, 215, 0));
            PlaySoundEffect(5);
            // Double Flip Bonus Ripple: flip adjacent 8-neighbor opponent discs
            int opponent = (player == BLACK) ? WHITE : BLACK;
            for (int d = 0; d < 8; d++) {
                int nr = r + dirs[d][0];
                int nc = c + dirs[d][1];
                if (nr >= 0 && nr < g_boardHeight && nc >= 0 && nc < g_boardWidth) {
                    int nidx = nr * g_boardWidth + nc;
                    if (board[nidx] == opponent) {
                        board[nidx] = player;
                        if (numAnimatingFlips < 100) animatingFlips[numAnimatingFlips++] = nidx;
                    }
                }
            }
        } else {
            SpawnSparksGDI(cx, cy, 14, player == BLACK ? RGB(255, 215, 0) : RGB(0, 245, 212));
            PlaySoundEffect(1);
        }

        if (hwnd) SetTimer(hwnd, 4, 25, NULL);

        for(int i = 0; i < count; i++) {
            board[animatingFlips[i]] = player;
        }
    }
}

void DoBombMove(int index, int player, HWND hwnd) {
    if ((board[index] != EMPTY && board[index] != DOUBLE_FLIP) || bombCount <= 0) return;
    PushHistory();
    board[index] = player;
    bombCount--;
    isBombActive = 0;
    
    int r = index / g_boardWidth;
    int c = index % g_boardWidth;
    int opponent = (player == BLACK) ? WHITE : BLACK;
    int flippableCount = 0;
    
    int cellSize = (g_boardWidth == 6) ? 55 : ((g_boardWidth == 8) ? 45 : 38);
    int boardStartX = 20;
    int boardStartY = 95;
    float cx = (float)(boardStartX + c * cellSize + cellSize / 2);
    float cy = (float)(boardStartY + r * cellSize + cellSize / 2);
    SpawnSparksGDI(cx, cy, 35, RGB(255, 69, 0));
    if (hwnd) SetTimer(hwnd, 4, 25, NULL);

    for (int d = 0; d < 8; d++) {
        int nr = r + dirs[d][0];
        int nc = c + dirs[d][1];
        if (nr >= 0 && nr < g_boardHeight && nc >= 0 && nc < g_boardWidth) {
            int idx = nr * g_boardWidth + nc;
            if (board[idx] == opponent) {
                board[idx] = player;
                animatingFlips[flippableCount++] = idx;
            }
        }
    }
    numAnimatingFlips = flippableCount;
    flipProgress = 0;
    SetTimer(hwnd, 2, 30, NULL);
    PlaySoundEffect(3);
}

void AIMove(HWND hwnd) {
    if (gameEnded) return;
    if (!HasValidMoves(WHITE)) {
        currentPlayer = BLACK;
        ResetMoveTimer(hwnd);
        InvalidateRect(hwnd, NULL, TRUE);
        return;
    }
    
    int bestMove = GetBestMoveForPlayer(WHITE);
    
    if (bestMove != -1) {
        PushHistory();
        DoMove(bestMove, WHITE, hwnd);
    }
    currentPlayer = BLACK;
    if (!HasValidMoves(BLACK)) {
        currentPlayer = WHITE;
        if (HasValidMoves(WHITE)) {
            SetTimer(hwnd, 1, 400, NULL);
        }
    }
    ResetMoveTimer(hwnd);
    InvalidateRect(hwnd, NULL, TRUE);
}

void SaveGame(HWND hwnd) {
    FILE* f = fopen("kreversi_save.dat", "wb");
    if (f) {
        fwrite(&gameMode, sizeof(int), 1, f);
        fwrite(&campaignStage, sizeof(int), 1, f);
        fwrite(&g_boardWidth, sizeof(int), 1, f);
        fwrite(&g_boardHeight, sizeof(int), 1, f);
        fwrite(board, sizeof(int), g_boardWidth * g_boardHeight, f);
        fwrite(&currentPlayer, sizeof(int), 1, f);
        fwrite(&bombCount, sizeof(int), 1, f);
        fwrite(&freezeCount, sizeof(int), 1, f);
        fwrite(&aiDifficulty, sizeof(int), 1, f);
        fwrite(&historyCount, sizeof(int), 1, f);
        fwrite(history, sizeof(HistoryState), historyCount, f);
        fwrite(&currentTheme, sizeof(int), 1, f);
        fclose(f);
        MessageBox(hwnd, "Game saved successfully.", "Save Game", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBox(hwnd, "Failed to save game.", "Error", MB_OK | MB_ICONERROR);
    }
}

void LoadGame(HWND hwnd) {
    FILE* f = fopen("kreversi_save.dat", "rb");
    if (f) {
        fread(&gameMode, sizeof(int), 1, f);
        fread(&campaignStage, sizeof(int), 1, f);
        fread(&g_boardWidth, sizeof(int), 1, f);
        fread(&g_boardHeight, sizeof(int), 1, f);
        fread(board, sizeof(int), g_boardWidth * g_boardHeight, f);
        fread(&currentPlayer, sizeof(int), 1, f);
        fread(&bombCount, sizeof(int), 1, f);
        fread(&freezeCount, sizeof(int), 1, f);
        fread(&aiDifficulty, sizeof(int), 1, f);
        fread(&historyCount, sizeof(int), 1, f);
        fread(history, sizeof(HistoryState), historyCount, f);
        if (fread(&currentTheme, sizeof(int), 1, f) == 1) {
            if (currentTheme < 0 || currentTheme > 3) currentTheme = 0;
        }
        fclose(f);
        KillTimer(hwnd, 1);
        KillTimer(hwnd, 2);
        numAnimatingFlips = 0;
        gameOverSoundPlayed = 0;
        gameEnded = 0;
        isBombActive = 0;
        ResetMoveTimer(hwnd);
        InvalidateRect(hwnd, NULL, TRUE);
        MessageBox(hwnd, "Game loaded successfully.", "Load Game", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBox(hwnd, "No saved game found.", "Load Game", MB_OK | MB_ICONWARNING);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: {
            srand((unsigned int)time(NULL));
            LoadTheme();
            LoadStats();
            
            HMENU hMenu = CreateMenu();
            
            HMENU hModeMenu = CreatePopupMenu();
            AppendMenu(hModeMenu, MF_STRING | (gameMode == MODE_FREEPLAY ? MF_CHECKED : 0), IDM_FREEPLAY_MODE, "Free Play");
            AppendMenu(hModeMenu, MF_STRING | (gameMode == MODE_CAMPAIGN ? MF_CHECKED : 0), IDM_CAMPAIGN_MODE, "Campaign Mode (20 Stages)");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hModeMenu, "Mode");

            HMENU hStageMenu = CreatePopupMenu();
            for (int s = 1; s <= 20; s++) {
                char stageName[64];
                sprintf(stageName, "Stage %d: %s %s", s, g_campaignStages[s-1].name, s <= maxUnlockedStage ? "" : "(Locked)");
                AppendMenu(hStageMenu, MF_STRING | (s <= maxUnlockedStage ? 0 : MF_GRAYED), IDM_STAGE_BASE + s, stageName);
            }
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hStageMenu, "Campaign Stage");

            HMENU hAIMenu = CreatePopupMenu();
            AppendMenu(hAIMenu, MF_STRING, IDM_AI_ROOKIE, "Rookie AI");
            AppendMenu(hAIMenu, MF_STRING, IDM_AI_GREEDY, "Greedy AI");
            AppendMenu(hAIMenu, MF_STRING | MF_CHECKED, IDM_AI_POSITIONAL, "Positional AI");
            AppendMenu(hAIMenu, MF_STRING, IDM_AI_GRANDMASTER, "Grandmaster Minimax AI");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hAIMenu, "AI Difficulty");

            HMENU hSizeMenu = CreatePopupMenu();
            AppendMenu(hSizeMenu, MF_STRING, IDM_SIZE_6, "6x6 Board");
            AppendMenu(hSizeMenu, MF_STRING | MF_CHECKED, IDM_SIZE_8, "8x8 Board");
            AppendMenu(hSizeMenu, MF_STRING, IDM_SIZE_10, "10x10 Board");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSizeMenu, "Board Size");

            HMENU hThemeMenu = CreatePopupMenu();
            AppendMenu(hThemeMenu, MF_STRING | (currentTheme == 0 ? MF_CHECKED : 0), IDM_THEME_CLASSIC, "Classic Emerald");
            AppendMenu(hThemeMenu, MF_STRING | (currentTheme == 1 ? MF_CHECKED : 0), IDM_THEME_CYBER, "Midnight Cyber");
            AppendMenu(hThemeMenu, MF_STRING | (currentTheme == 2 ? MF_CHECKED : 0), IDM_THEME_CRIMSON, "Obsidian Crimson");
            AppendMenu(hThemeMenu, MF_STRING | (currentTheme == 3 ? MF_CHECKED : 0), IDM_THEME_TERMINAL, "Retro Terminal");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hThemeMenu, "Theme");
            
            HMENU hTimeMenu = CreatePopupMenu();
            AppendMenu(hTimeMenu, MF_STRING | MF_CHECKED, IDM_TIME_UNTIMED, "Untimed");
            AppendMenu(hTimeMenu, MF_STRING, IDM_TIME_BLITZ, "Blitz (5s)");
            AppendMenu(hTimeMenu, MF_STRING, IDM_TIME_RAPID, "Rapid (15s)");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hTimeMenu, "Timer");

            HMENU hActionMenu = CreatePopupMenu();
            AppendMenu(hActionMenu, MF_STRING, IDM_BOMB, "Activate Bomb Disc (B)");
            AppendMenu(hActionMenu, MF_STRING, IDM_FREEZE, "Freeze AI (F)");
            AppendMenu(hActionMenu, MF_STRING, IDM_HINT, "Optimal Hint (H)");
            AppendMenu(hActionMenu, MF_STRING, IDM_UNDO, "Undo Move (U)");
            AppendMenu(hActionMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hActionMenu, MF_STRING, IDM_SAVE, "Save Game");
            AppendMenu(hActionMenu, MF_STRING, IDM_LOAD, "Load Game");
            AppendMenu(hActionMenu, MF_STRING, IDM_STATS, "Statistics");
            AppendMenu(hActionMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hActionMenu, MF_STRING | MF_CHECKED, IDM_SOUND, "Sound Effects");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hActionMenu, "Actions");
            
            HMENU hHelpMenu = CreatePopupMenu();
            AppendMenu(hHelpMenu, MF_STRING, IDM_HELP, "Strategy Guide & Rules");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, "Help");
            
            SetMenu(hwnd, hMenu);
            InitGame(hwnd);
            break;
        }
        case WM_MOUSEMOVE: {
            int xPos = (int)(short)LOWORD(lParam);
            int yPos = (int)(short)HIWORD(lParam);
            int cellSize = (g_boardWidth == 6) ? 55 : ((g_boardWidth == 8) ? 45 : 38);
            int boardStartX = 20;
            int boardStartY = 95;
            
            int c = (xPos - boardStartX) / cellSize;
            int r = (yPos - boardStartY) / cellSize;
            
            int newHoverIdx = -1;
            if (xPos >= boardStartX && yPos >= boardStartY && c >= 0 && c < g_boardWidth && r >= 0 && r < g_boardHeight) {
                newHoverIdx = r * g_boardWidth + c;
            }
            if (hoverIdx != newHoverIdx) {
                hoverIdx = newHoverIdx;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id >= IDM_STAGE_1 && id <= IDM_STAGE_20) {
                int selectedStage = id - IDM_STAGE_BASE;
                if (selectedStage <= maxUnlockedStage) {
                    campaignStage = selectedStage;
                    gameMode = MODE_CAMPAIGN;
                    InitGame(hwnd);
                }
                break;
            }
            switch(id) {
                case IDM_FREEPLAY_MODE: {
                    gameMode = MODE_FREEPLAY;
                    InitGame(hwnd);
                    break;
                }
                case IDM_CAMPAIGN_MODE: {
                    gameMode = MODE_CAMPAIGN;
                    InitGame(hwnd);
                    break;
                }
                case IDM_AI_ROOKIE:
                case IDM_AI_GREEDY:
                case IDM_AI_POSITIONAL:
                case IDM_AI_GRANDMASTER: {
                    aiDifficulty = id - IDM_AI_ROOKIE;
                    break;
                }
                case IDM_SIZE_6: g_boardWidth = 6; g_boardHeight = 6; InitGame(hwnd); break;
                case IDM_SIZE_8: g_boardWidth = 8; g_boardHeight = 8; InitGame(hwnd); break;
                case IDM_SIZE_10: g_boardWidth = 10; g_boardHeight = 10; InitGame(hwnd); break;
                
                case IDM_THEME_CLASSIC:
                case IDM_THEME_CYBER:
                case IDM_THEME_CRIMSON:
                case IDM_THEME_TERMINAL: {
                    currentTheme = id - IDM_THEME_CLASSIC;
                    SaveTheme();
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                }
                case IDM_TIME_UNTIMED: moveTimeMode = 0; ResetMoveTimer(hwnd); InvalidateRect(hwnd, NULL, TRUE); break;
                case IDM_TIME_BLITZ: moveTimeMode = 5; ResetMoveTimer(hwnd); InvalidateRect(hwnd, NULL, TRUE); break;
                case IDM_TIME_RAPID: moveTimeMode = 15; ResetMoveTimer(hwnd); InvalidateRect(hwnd, NULL, TRUE); break;

                case IDM_BOMB: {
                    if (bombCount > 0 && !gameEnded && currentPlayer == BLACK) {
                        isBombActive = !isBombActive;
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                    break;
                }
                case IDM_FREEZE: DoFreezeAI(hwnd); break;
                case IDM_HINT: showHint = !showHint; InvalidateRect(hwnd, NULL, TRUE); break;
                case IDM_UNDO: UndoMove(hwnd); break;
                case IDM_STATS: ShowStats(hwnd); break;
                case IDM_SAVE: SaveGame(hwnd); break;
                case IDM_LOAD: LoadGame(hwnd); break;
                case IDM_SOUND: soundEnabled = !soundEnabled; break;
                case IDM_HELP: ShowHelp(hwnd); break;
            }
            break;
        }
        case WM_KEYDOWN: {
            if (wParam == 'B' || wParam == 'b') {
                if (bombCount > 0 && !gameEnded && currentPlayer == BLACK) {
                    isBombActive = !isBombActive;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (wParam == 'F' || wParam == 'f') {
                DoFreezeAI(hwnd);
            } else if (wParam == 'H' || wParam == 'h') {
                showHint = !showHint;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'U' || wParam == 'u') {
                UndoMove(hwnd);
            } else if (wParam == 'R' || wParam == 'r') {
                InitGame(hwnd);
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            if (currentPlayer != BLACK || gameEnded) break;
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            int cellSize = (g_boardWidth == 6) ? 55 : ((g_boardWidth == 8) ? 45 : 38);
            int boardStartX = 20;
            int boardStartY = 95;
            
            int c = (x - boardStartX) / cellSize;
            int r = (y - boardStartY) / cellSize;
            
            if (r >= 0 && r < g_boardHeight && c >= 0 && c < g_boardWidth) {
                int idx = r * g_boardWidth + c;
                if (isBombActive) {
                    if ((board[idx] == EMPTY || board[idx] == DOUBLE_FLIP) && bombCount > 0) {
                        DoBombMove(idx, BLACK, hwnd);
                        currentPlayer = WHITE;
                        if (!HasValidMoves(WHITE)) {
                            currentPlayer = BLACK;
                        } else {
                            SetTimer(hwnd, 1, 400, NULL);
                        }
                        ResetMoveTimer(hwnd);
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                } else {
                    int flips[100];
                    int count = GetFlippable(idx, BLACK, flips);
                    if (count > 0) {
                        PushHistory();
                        DoMove(idx, BLACK, hwnd);
                        currentPlayer = WHITE;
                        if (!HasValidMoves(WHITE)) {
                            currentPlayer = BLACK;
                        } else {
                            SetTimer(hwnd, 1, 400, NULL);
                        }
                        ResetMoveTimer(hwnd);
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                }
            }
            break;
        }
        case WM_TIMER: {
            if (wParam == 1) {
                KillTimer(hwnd, 1);
                AIMove(hwnd);
            } else if (wParam == 2) {
                flipProgress++;
                InvalidateRect(hwnd, NULL, FALSE);
                if (flipProgress >= 10) {
                    KillTimer(hwnd, 2);
                    numAnimatingFlips = 0;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (wParam == 3) {
                if (moveTimeMode > 0 && !gameEnded) {
                    moveTimeLeftDeci--;
                    if (moveTimeLeftDeci <= 0) {
                        KillTimer(hwnd, 3);
                        gameEnded = 1;
                        int bCount = 0, wCount = 0;
                        for(int i = 0; i < g_boardWidth * g_boardHeight; i++) {
                            if(board[i] == BLACK) bCount++;
                            if(board[i] == WHITE) wCount++;
                        }
                        if (currentPlayer == BLACK) stats[1]++; else stats[0]++;
                        SaveStats();
                        PlaySoundEffect(2);
                        InvalidateRect(hwnd, NULL, TRUE);
                    } else {
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                }
            } else if (wParam == 4) {
                if (g_numParticles > 0) {
                    InvalidateRect(hwnd, NULL, FALSE);
                } else {
                    KillTimer(hwnd, 4);
                }
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            const BoardTheme* theme = &g_themes[currentTheme];

            HBRUSH bgBrush = CreateSolidBrush(theme->bg);
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            FillRect(hdc, &clientRect, bgBrush);
            DeleteObject(bgBrush);

            SetBkMode(hdc, OPAQUE);
            SetBkColor(hdc, theme->bg);
            SetTextColor(hdc, theme->text);
            
            int bCount = 0, wCount = 0;
            for(int i = 0; i < g_boardWidth * g_boardHeight; i++) {
                if(board[i] == BLACK) bCount++;
                if(board[i] == WHITE) wCount++;
            }
            
            char headerStr[128];
            if (gameMode == MODE_CAMPAIGN) {
                const CampaignStageData* stg = &g_campaignStages[campaignStage - 1];
                sprintf(headerStr, "Stage %d: %s | AI: %s | Bombs: %d | Freezes: %d",
                        campaignStage, stg->name,
                        aiDifficulty == 0 ? "Rookie" : (aiDifficulty == 1 ? "Greedy" : (aiDifficulty == 2 ? "Positional" : "Grandmaster")),
                        bombCount, freezeCount);
            } else {
                sprintf(headerStr, "Free Play (%dx%d) | AI: %s | Bombs: %d | Freezes: %d",
                        g_boardWidth, g_boardHeight,
                        aiDifficulty == 0 ? "Rookie" : (aiDifficulty == 1 ? "Greedy" : (aiDifficulty == 2 ? "Positional" : "Grandmaster")),
                        bombCount, freezeCount);
            }
            TextOut(hdc, 10, 8, headerStr, strlen(headerStr));

            char scoreStr[128];
            sprintf(scoreStr, "%s: %d   %s: %d   Turn: %s", theme->p1Name, bCount, theme->p2Name, wCount, currentPlayer == BLACK ? theme->p1Name : theme->p2Name);
            TextOut(hdc, 10, 26, scoreStr, strlen(scoreStr));
            
            if (gameEnded) {
                char winMsg[128];
                if (moveTimeMode > 0 && moveTimeLeftDeci <= 0) {
                    sprintf(winMsg, "%s Timeout!", currentPlayer == BLACK ? theme->p1Name : theme->p2Name);
                } else if (bCount > wCount) {
                    if (gameMode == MODE_CAMPAIGN) {
                        sprintf(winMsg, "STAGE %d CLEARED! WINNER!", campaignStage);
                    } else sprintf(winMsg, "%s WINS!", theme->p1Name);
                } else if (wCount > bCount) {
                    sprintf(winMsg, "%s WINS!", theme->p2Name);
                } else sprintf(winMsg, "DRAW!");
                TextOut(hdc, 10, 44, winMsg, strlen(winMsg));
            } else if (!HasValidMoves(BLACK) && !HasValidMoves(WHITE)) {
                gameEnded = 1;
                KillTimer(hwnd, 3);
                char winMsg[128];
                if (bCount > wCount) {
                    if (gameMode == MODE_CAMPAIGN) {
                        sprintf(winMsg, "STAGE %d CLEARED! WINNER!", campaignStage);
                    } else sprintf(winMsg, "%s WINS!", theme->p1Name);
                    SpawnVictoryFireworksGDI(clientRect.right, clientRect.bottom);
                    SetTimer(hwnd, 4, 25, NULL);
                } else if (wCount > bCount) {
                    sprintf(winMsg, "%s WINS!", theme->p2Name);
                } else sprintf(winMsg, "DRAW!");
                TextOut(hdc, 10, 44, winMsg, strlen(winMsg));
                
                if (!gameOverSoundPlayed) {
                    gameOverSoundPlayed = 1;
                    PlaySoundEffect(2);
                    UpdateStats(bCount, wCount);
                }
            } else if (isBombActive) {
                TextOut(hdc, 10, 44, "BOMB ACTIVE: Click ANY empty cell to detonate 3x3 explosion!", 60);
            }

            int evalScore = EvaluateBoardState(board, g_boardWidth, g_boardHeight, BLACK);
            int bestMoveIdx = (currentPlayer == BLACK && HasValidMoves(BLACK) && !gameEnded) ? GetBestMoveForPlayer(BLACK) : -1;
            
            char evalStr[160];
            char clockStr[32];
            if (moveTimeMode > 0) sprintf(clockStr, "Clock: %.1fs", moveTimeLeftDeci / 10.0);
            else sprintf(clockStr, "Clock: Off");

            if (bestMoveIdx != -1 && showHint) {
                int col = bestMoveIdx % g_boardWidth;
                int row = bestMoveIdx / g_boardWidth;
                sprintf(evalStr, "Eval: %+d | Best: %c%d | %s | [H]int [U]ndo [B]omb [F]reeze", 
                        evalScore, 'A' + col, row + 1, clockStr);
            } else {
                sprintf(evalStr, "Eval: %+d | Best: - | %s | [H]int [U]ndo [B]omb [F]reeze", 
                        evalScore, clockStr);
            }
            TextOut(hdc, 10, 68, evalStr, strlen(evalStr));

            HBRUSH blockedBrush = CreateHatchBrush(HS_DIAGCROSS, RGB(100, 100, 100));
            
            int cellSize = (g_boardWidth == 6) ? 55 : ((g_boardWidth == 8) ? 45 : 38);
            int boardStartX = 20;
            int boardStartY = 95;

            // 3D Mahogany Frame Surround
            DrawMahoganyFrame(hdc, boardStartX, boardStartY, g_boardWidth * cellSize, g_boardHeight * cellSize);

            for(int r = 0; r < g_boardHeight; r++) {
                for(int c = 0; c < g_boardWidth; c++) {
                    RECT rect = { 
                        boardStartX + c * cellSize, 
                        boardStartY + r * cellSize, 
                        boardStartX + (c + 1) * cellSize, 
                        boardStartY + (r + 1) * cellSize 
                    };
                    int idx = r * g_boardWidth + c;

                    if (board[idx] == BLOCKED) {
                        SelectObject(hdc, blockedBrush);
                        Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
                    } else if (board[idx] == DOUBLE_FLIP) {
                        HBRUSH goldCellBrush = CreateSolidBrush(RGB(70, 55, 10));
                        HPEN goldBorderPen = CreatePen(PS_SOLID, 2, RGB(255, 215, 0));
                        SelectObject(hdc, goldCellBrush);
                        SelectObject(hdc, goldBorderPen);
                        Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
                        DeleteObject(goldCellBrush);
                        DeleteObject(goldBorderPen);

                        SetBkMode(hdc, TRANSPARENT);
                        SetTextColor(hdc, RGB(255, 215, 0));
                        RECT tRect = rect;
                        DrawText(hdc, "2X", 2, &tRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                        SetBkMode(hdc, OPAQUE);
                        SetBkColor(hdc, theme->bg);
                        SetTextColor(hdc, theme->text);
                    } else {
                        int cX = rect.left + cellSize / 2;
                        int cY = rect.top + cellSize / 2;
                        int bCenterX = boardStartX + (g_boardWidth * cellSize) / 2;
                        int bCenterY = boardStartY + (g_boardHeight * cellSize) / 2;
                        int distSq = (cX - bCenterX) * (cX - bCenterX) + (cY - bCenterY) * (cY - bCenterY);
                        int maxDistSq = (g_boardWidth * cellSize / 2) * (g_boardWidth * cellSize / 2) + (g_boardHeight * cellSize / 2) * (g_boardHeight * cellSize / 2);
                        float distRatio = (float)distSq / (float)(maxDistSq + 1);
                        if (distRatio > 1.0f) distRatio = 1.0f;
                        
                        COLORREF baseColor = theme->boardCell;
                        int rC = GetRValue(baseColor);
                        int gC = GetGValue(baseColor);
                        int bC = GetBValue(baseColor);
                        rC = (int)(rC * (1.0f - distRatio * 0.4f));
                        gC = (int)(gC * (1.0f - distRatio * 0.4f));
                        bC = (int)(bC * (1.0f - distRatio * 0.4f));
                        
                        HBRUSH cFeltBrush = CreateSolidBrush(RGB(rC, gC, bC));
                        HPEN cHiPen = CreatePen(PS_SOLID, 1, RGB(rC + 20 > 255 ? 255 : rC + 20, gC + 30 > 255 ? 255 : gC + 30, bC + 20 > 255 ? 255 : bC + 20));
                        HPEN cShPen = CreatePen(PS_SOLID, 1, RGB(rC / 2, gC / 2, bC / 2));
                        SelectObject(hdc, cFeltBrush);
                        SelectObject(hdc, cHiPen);
                        Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
                        
                        // Felt Texture
                        for (int ty = rect.top + 2; ty < rect.bottom - 2; ty += 4) {
                            for (int tx = rect.left + 2; tx < rect.right - 2; tx += 4) {
                                SetPixel(hdc, tx + (rand() % 3), ty + (rand() % 3), RGB(rC - 10 < 0 ? 0 : rC - 10, gC - 15 < 0 ? 0 : gC - 15, bC - 10 < 0 ? 0 : bC - 10));
                            }
                        }
                        
                        SelectObject(hdc, cShPen);
                        MoveToEx(hdc, rect.right - 1, rect.top, NULL);
                        LineTo(hdc, rect.right - 1, rect.bottom - 1);
                        LineTo(hdc, rect.left, rect.bottom - 1);

                        DeleteObject(cFeltBrush);
                        DeleteObject(cHiPen);
                        DeleteObject(cShPen);
                    }

                    int h1 = g_boardWidth == 6 ? 1 : 2;
                    int h2 = g_boardWidth == 6 ? 4 : (g_boardWidth == 8 ? 5 : 7);
                    if (board[idx] != BLOCKED && (r == h1 || r == h2) && (c == h1 || c == h2)) {
                        HBRUSH hoshiBrush = CreateSolidBrush(RGB(212, 175, 55));
                        HPEN hoshiPen = CreatePen(PS_SOLID, 1, RGB(160, 120, 20));
                        SelectObject(hdc, hoshiBrush);
                        SelectObject(hdc, hoshiPen);
                        int hcx = rect.left + cellSize / 2;
                        int hcy = rect.top + cellSize / 2;
                        Ellipse(hdc, hcx - 3, hcy - 3, hcx + 3, hcy + 3);
                        DeleteObject(hoshiBrush);
                        DeleteObject(hoshiPen);
                    }
                    
                    if (board[idx] == BLACK || board[idx] == WHITE) {
                        int isAnimating = 0;
                        if (numAnimatingFlips > 0 && flipProgress < 10) {
                            for(int k=0; k<numAnimatingFlips; k++) {
                                if (animatingFlips[k] == idx) { isAnimating = 1; break; }
                            }
                        }
                        
                        int radius = (cellSize - (cellSize >= 45 ? 10 : 6)) / 2;
                        int cx = rect.left + cellSize / 2;
                        int cy = rect.top + cellSize / 2;

                        if (isAnimating) {
                            int oldColor = (board[idx] == BLACK) ? WHITE : BLACK;
                            int newColor = board[idx];
                            int displayColor = (flipProgress < 5) ? oldColor : newColor;
                            
                            int fullW = radius * 2;
                            int currentW = fullW;
                            if (flipProgress < 5) currentW = fullW - (flipProgress * (fullW / 5)); 
                            else currentW = (flipProgress - 5) * (fullW / 5); 
                            if (currentW < 2) currentW = 2;

                            int yOff = (flipProgress < 5) ? -flipProgress * 2 : -(10 - flipProgress) * 2;
                            DrawDisc3D(hdc, cx, cy, radius, displayColor, currentW, yOff);
                        } else {
                            DrawDisc3D(hdc, cx, cy, radius, board[idx], radius * 2, 0);
                        }
                    } else if (currentPlayer == BLACK && !gameEnded && (board[idx] == EMPTY || board[idx] == DOUBLE_FLIP)) {
                        int dummy[100];
                        if (isBombActive) {
                            HPEN bombPen = CreatePen(PS_SOLID, 2, RGB(255, 69, 0));
                            SelectObject(hdc, GetStockObject(NULL_BRUSH));
                            SelectObject(hdc, bombPen);
                            Ellipse(hdc, rect.left + 8, rect.top + 8, rect.right - 8, rect.bottom - 8);
                            DeleteObject(bombPen);
                        } else if (GetFlippable(idx, BLACK, dummy) > 0) {
                            if (hoverIdx == idx) {
                                int radius = (cellSize - (cellSize >= 45 ? 10 : 6)) / 2;
                                int hcx = rect.left + cellSize / 2;
                                int hcy = rect.top + cellSize / 2;
                                HBRUSH ghostBrush = CreateHatchBrush(HS_BDIAGONAL, RGB(100, 100, 100));
                                HPEN ghostPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
                                SelectObject(hdc, ghostBrush);
                                SelectObject(hdc, ghostPen);
                                Ellipse(hdc, hcx - radius, hcy - radius, hcx + radius, hcy + radius);
                                DeleteObject(ghostBrush);
                                DeleteObject(ghostPen);
                            }
                            if (showHint && idx == bestMoveIdx) {
                                HBRUSH hintBrush = CreateSolidBrush(RGB(255, 215, 0));
                                HPEN hintPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 180));
                                SelectObject(hdc, hintBrush);
                                SelectObject(hdc, hintPen);
                                int hcx = rect.left + cellSize / 2;
                                int hcy = rect.top + cellSize / 2;
                                Ellipse(hdc, hcx - 7, hcy - 7, hcx + 7, hcy + 7);
                                DeleteObject(hintBrush);
                                DeleteObject(hintPen);
                            } else {
                                HBRUSH hintBrush = CreateSolidBrush(RGB(255, 215, 0));
                                HPEN hintPen = CreatePen(PS_SOLID, 1, RGB(200, 160, 0));
                                SelectObject(hdc, hintBrush);
                                SelectObject(hdc, hintPen);
                                int hcx = rect.left + cellSize / 2;
                                int hcy = rect.top + cellSize / 2;
                                Ellipse(hdc, hcx - 4, hcy - 4, hcx + 4, hcy + 4);
                                DeleteObject(hintBrush);
                                DeleteObject(hintPen);
                            }
                        }
                    }
                }
            }
            DeleteObject(blockedBrush);
            
            UpdateAndDrawParticlesGDI(hdc);

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_CLOSE: DestroyWindow(hwnd); break;
        case WM_DESTROY: PostQuitMessage(0); break;
        default: return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
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
    wc.hbrBackground = CreateSolidBrush(RGB(18, 18, 18));
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "KReversi",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 480, 560,
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
