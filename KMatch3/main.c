#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#define MAX_ROWS 10
#define MAX_COLS 10
#define BOARD_X 50
#define BOARD_Y 95
#define BOARD_SIZE 400

#define TYPE_NONE 0
#define TYPE_HORIZ 1
#define TYPE_VERT 2
#define TYPE_RAINBOW 3
#define TYPE_BOMB 4

typedef struct {
    int targetScore;
    int moves;
    int timeLimit;
    int iceCount;
    int stoneCount;
    int ironCount;
    int redTarget;
    int greenTarget;
    int blueTarget;
    int bossHP;
    int rows;
    int cols;
    const char* name;
} StageConfig;

static const StageConfig CAMPAIGN_STAGES[20] = {
    {  800, 20,  0,  0,  0,  0,  0,  0,  0,   0, 6, 6, "1: Gem Starter" },
    { 1200, 22,  0,  6,  0,  0,  0,  0,  0,   0, 6, 6, "2: Frosty Fields" },
    { 1500, 20,  0,  6,  4,  0,  0,  0,  0,   0, 7, 7, "3: Stony Path" },
    { 1800,  0, 60,  6,  4,  0,  0,  0,  0,   0, 7, 7, "4: Speed Rush" },
    { 2200, 20,  0,  8,  4,  2,  0,  0,  0,   0, 8, 8, "5: Iron Quarry" },
    { 2600, 18,  0, 12,  4,  2,  0,  0,  0,   0, 8, 8, "6: Ice Citadel" },
    { 3000,  0, 55, 10,  6,  2,  0,  0,  0,   0, 8, 8, "7: Clockwork Cavern" },
    { 3500, 22,  0, 14,  6,  4,  0,  0,  0,   0, 8, 8, "8: Stone Fortress" },
    { 4000, 22,  0,  8,  4,  2, 15,  0,  0,   0, 9, 9, "9: Ruby Collector" },
    { 4500,  0, 50, 16,  8,  4,  0,  0,  0,   0, 9, 9, "10: Blizzard Blitz" },
    { 5000, 20,  0, 18,  8,  4,  0,  0,  0,   0, 9, 9, "11: Glacier Ridge" },
    { 5500, 18,  0, 20, 10,  6,  0,  0,  0,   0, 9, 9, "12: Iron Vault" },
    { 6000,  0, 45, 12,  6,  4,  0, 20,  0,   0,10,10, "13: Emerald Rush" },
    { 7000, 22,  0, 24, 12,  6,  0,  0,  0,   0,10,10, "14: Crystal Mine" },
    { 8000, 24,  0, 16,  8,  4,  0,  0, 25,   0,10,10, "15: Sapphire Temple" },
    { 9000,  0, 40, 25, 14,  6,  0,  0,  0,   0,10,10, "16: Time Vortex" },
    {10000, 20,  0, 28, 16,  8,  0,  0,  0,   0,10,10, "17: Obsidian Lair" },
    {12000, 25,  0, 30, 18, 10,  0,  0,  0,   0,10,10, "18: Diamond Gauntlet" },
    {14000,  0, 45, 32, 20, 10,  0,  0,  0,   0,10,10, "19: Master's Crucible" },
    {    0, 30,  0, 10, 10,  6,  0,  0,  0,  75,10,10, "20: Jewel King Boss" }
};

int CheckMatchPossible();
void ShuffleBoard();

int rows = 8;
int cols = 8;
int cellSize = 50;

int grid[MAX_ROWS][MAX_COLS];
int typeGrid[MAX_ROWS][MAX_COLS] = {0};
int iceGrid[MAX_ROWS][MAX_COLS] = {0};
int stoneGrid[MAX_ROWS][MAX_COLS] = {0};   // 1 = stone (1 hit), 2 = iron (2 hits), 3 = heavy iron (3 hits)
int barrierGrid[MAX_ROWS][MAX_COLS] = {0}; // 1 = boss barrier shield

int powerupMode = 0; // 0 = none, 1 = hammer, 2 = color nuke
int score = 0;
int moves = 20;
int level = 1;
int gameMode = 0; // 0 = Campaign (20 Stages), 1 = Zen, 2 = Timed Rush
int targetScore = 800;

int collectedRed = 0;
int collectedGreen = 0;
int collectedBlue = 0;
int bossHP = 0;
int maxBossHP = 0;
int bossMoveTimer = 0;

int selR = -1, selC = -1;
int isProcessing = 0;

int swapAnim = 0;
int swapR1 = -1, swapC1 = -1, swapR2 = -1, swapC2 = -1;
int lastSwapR1 = -1, lastSwapC1 = -1, lastSwapR2 = -1, lastSwapC2 = -1;
int hintTimer = 0;
int hintR1 = -1, hintC1 = -1, hintR2 = -1, hintC2 = -1;
int popAnim = 0;
int popGrid[MAX_ROWS][MAX_COLS] = {0};
int dropAnim = 0;
int dropCount[MAX_ROWS][MAX_COLS] = {0};
int screenShake = 0;

int statsGamesPlayed = 0;
int statsBestScore = 0;
int statsMaxCombo = 0;

COLORREF colors[] = {
    RGB(255, 64, 64),   // Red
    RGB(64, 255, 64),   // Green
    RGB(64, 64, 255),   // Blue
    RGB(255, 255, 64),  // Yellow
    RGB(255, 64, 255),  // Purple
    RGB(64, 255, 255)   // Cyan
};

#define MAX_PARTICLES 200

typedef struct {
    float x, y;
    float vx, vy;
    float rot, vrot;
    float size;
    float life, decay;
    COLORREF color;
    int type; // 0=glow, 1=debris, 2=smoke
} Particle;

Particle particles[MAX_PARTICLES] = {0};

void CreateParticles(int cx, int cy, COLORREF color) {
    for (int i = 0; i < 30; i++) {
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (particles[p].life <= 0) {
                particles[p].x = (float)cx;
                particles[p].y = (float)cy;
                float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
                particles[p].rot = (float)(rand() % 360);
                particles[p].vrot = (float)((rand() % 40) - 20);
                particles[p].life = 1.0f;
                
                if (i < 10) { // Heavy Debris
                    float speed = (float)((rand() % 80) + 40) / 10.0f;
                    particles[p].vx = cosf(angle) * speed;
                    particles[p].vy = sinf(angle) * speed;
                    particles[p].size = (float)((rand() % 6) + 4);
                    particles[p].decay = (float)((rand() % 5) + 2) / 100.0f;
                    particles[p].color = color;
                    particles[p].type = 1;
                } else if (i < 20) { // Core Sparkles
                    float speed = (float)((rand() % 50) + 20) / 10.0f;
                    particles[p].vx = cosf(angle) * speed;
                    particles[p].vy = sinf(angle) * speed;
                    particles[p].size = (float)((rand() % 3) + 2);
                    particles[p].decay = (float)((rand() % 8) + 4) / 100.0f;
                    particles[p].color = RGB(255, 255, 255);
                    particles[p].type = 0;
                } else { // Expanding Smoke
                    float speed = (float)((rand() % 20) + 5) / 10.0f;
                    particles[p].vx = cosf(angle) * speed;
                    particles[p].vy = sinf(angle) * speed - 1.0f;
                    particles[p].size = (float)((rand() % 10) + 10);
                    particles[p].decay = (float)((rand() % 3) + 1) / 100.0f;
                    particles[p].color = RGB(150, 150, 150);
                    particles[p].type = 2;
                }
                break;
            }
        }
    }
}

void UpdateParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0) {
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            if (particles[i].type == 1) { // Debris has gravity
                particles[i].vy += 0.4f;
            } else if (particles[i].type == 2) { // Smoke expands
                particles[i].size += 0.3f;
                particles[i].vx *= 0.95f;
                particles[i].vy *= 0.95f;
            } else {
                particles[i].vy += 0.1f;
            }
            particles[i].rot += particles[i].vrot;
            particles[i].life -= particles[i].decay;
        }
    }
}

int laserRowTimer[MAX_ROWS] = {0};
int laserColTimer[MAX_COLS] = {0};

void DrawFacetedGem(HDC hdc, int cx, int cy, int size, int colorIdx, int typeIdx, int isIce, int isBarrier, int isMoving, int popAnim) {
    float scale = (float)size / 50.0f;
    int r19 = (int)(19 * scale);
    int r16 = (int)(16 * scale);
    int r15 = (int)(15 * scale);
    int r14 = (int)(14 * scale);
    int r17 = (int)(17 * scale);
    int r13 = (int)(13 * scale);
    int r11 = (int)(11 * scale);
    int r9  = (int)(9  * scale);
    int r7  = (int)(7  * scale);
    int r5  = (int)(5  * scale);
    int r3  = (int)(3  * scale);

    if (isMoving) {
        HBRUSH shadowB = CreateSolidBrush(RGB(15, 10, 25));
        HPEN shadowP = CreatePen(PS_NULL, 0, 0);
        HBRUSH oB = (HBRUSH)SelectObject(hdc, shadowB);
        HPEN oP = (HPEN)SelectObject(hdc, shadowP);
        Ellipse(hdc, cx - r19 + 8, cy - r19 + 12, cx + r19 + 8, cy + r19 + 12);
        SelectObject(hdc, oB); SelectObject(hdc, oP);
        DeleteObject(shadowB); DeleteObject(shadowP);
    }

    if (typeIdx == TYPE_RAINBOW) {
        DWORD tick = GetTickCount();
        int roffset = (tick / 50) % 16;
        HBRUSH bgRing = CreateSolidBrush(RGB(255, 215, 0));
        HPEN goldPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
        HBRUSH oldB = (HBRUSH)SelectObject(hdc, bgRing);
        HPEN oldP = (HPEN)SelectObject(hdc, goldPen);
        Ellipse(hdc, cx - r19, cy - r19, cx + r19, cy + r19);
        
        POINT pts[16];
        for (int i = 0; i < 16; i++) {
            float r = ((i + roffset) % 2 == 0) ? (20.0f * scale) : (9.0f * scale);
            float a = (float)i * 3.14159f / 8.0f;
            pts[i].x = cx + (int)(cosf(a) * r);
            pts[i].y = cy + (int)(sinf(a) * r);
        }
        Polygon(hdc, pts, 16);

        HBRUSH coreB = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, coreB);
        Ellipse(hdc, cx - r5, cy - r5, cx + r5, cy + r5);
        DeleteObject(coreB);

        SelectObject(hdc, oldB); SelectObject(hdc, oldP);
        DeleteObject(bgRing); DeleteObject(goldPen);
    } else {
        COLORREF mainC = colors[colorIdx % 6];
        COLORREF lightC, darkC;
        switch (colorIdx % 6) {
            case 0: lightC = RGB(255, 120, 140); darkC = RGB(120, 0, 25); break;
            case 1: lightC = RGB(100, 255, 140); darkC = RGB(0, 80, 20); break;
            case 2: lightC = RGB(140, 180, 255); darkC = RGB(0, 25, 120); break;
            case 3: lightC = RGB(255, 235, 100); darkC = RGB(160, 90, 0); break;
            case 4: lightC = RGB(230, 140, 255); darkC = RGB(70, 0, 110); break;
            case 5: default: lightC = RGB(180, 255, 255); darkC = RGB(0, 100, 130); break;
        }

        HBRUSH mainB = CreateSolidBrush(mainC);
        HBRUSH lightB = CreateSolidBrush(lightC);
        HBRUSH darkB = CreateSolidBrush(darkC);
        HBRUSH whiteB = CreateSolidBrush(RGB(255, 255, 255));
        HPEN penDark = CreatePen(PS_SOLID, 1, darkC);
        HPEN oldP = (HPEN)SelectObject(hdc, penDark);
        HBRUSH oldB = (HBRUSH)SelectObject(hdc, mainB);

        int c = colorIdx % 6;
        if (c == 0) {
            POINT pOct[8] = {{cx-r7, cy-r17}, {cx+r7, cy-r17}, {cx+r17, cy-r7}, {cx+r17, cy+r7}, {cx+r7, cy+r17}, {cx-r7, cy+r17}, {cx-r17, cy+r7}, {cx-r17, cy-r7}};
            Polygon(hdc, pOct, 8);
            POINT pTop[4] = {{cx-r7, cy-r17}, {cx+r7, cy-r17}, {cx+r11, cy-r9}, {cx-r11, cy-r9}};
            POINT pBot[4] = {{cx-r11, cy+r9}, {cx+r11, cy+r9}, {cx+r7, cy+r17}, {cx-r7, cy+r17}};
            POINT pLeft[4] = {{cx-r17, cy-r7}, {cx-r11, cy-r9}, {cx-r11, cy+r9}, {cx-r17, cy+r7}};
            POINT pRight[4] = {{cx+r11, cy-r9}, {cx+r17, cy-r7}, {cx+r17, cy+r7}, {cx+r11, cy+r9}};
            POINT pCenter[4] = {{cx-r11, cy-r9}, {cx+r11, cy-r9}, {cx+r11, cy+r9}, {cx-r11, cy+r9}};
            SelectObject(hdc, lightB); Polygon(hdc, pTop, 4); Polygon(hdc, pLeft, 4);
            SelectObject(hdc, darkB); Polygon(hdc, pBot, 4); Polygon(hdc, pRight, 4);
            SelectObject(hdc, mainB); Polygon(hdc, pCenter, 4);
            SelectObject(hdc, whiteB); Ellipse(hdc, cx-r5, cy-r13, cx-r3, cy-r9);
        } else if (c == 1) {
            RECT rOuter = { cx-r16, cy-r14, cx+r16, cy+r14 };
            RoundRect(hdc, rOuter.left, rOuter.top, rOuter.right, rOuter.bottom, (int)(6*scale), (int)(6*scale));
            POINT pTop[4] = {{cx-r16, cy-r14}, {cx+r16, cy-r14}, {cx+r9, cy-r7}, {cx-r9, cy-r7}};
            POINT pBot[4] = {{cx-r9, cy+r7}, {cx+r9, cy+r7}, {cx+r16, cy+r14}, {cx-r16, cy+r14}};
            POINT pLeft[4] = {{cx-r16, cy-r14}, {cx-r9, cy-r7}, {cx-r9, cy+r7}, {cx-r16, cy+r14}};
            POINT pRight[4] = {{cx+r9, cy-r7}, {cx+r16, cy-r14}, {cx+r16, cy+r14}, {cx+r9, cy+r7}};
            SelectObject(hdc, lightB); Polygon(hdc, pTop, 4); Polygon(hdc, pLeft, 4);
            SelectObject(hdc, darkB); Polygon(hdc, pBot, 4); Polygon(hdc, pRight, 4);
            RECT rCenter = { cx-r9, cy-r7, cx+r9, cy+r7 };
            SelectObject(hdc, mainB); FillRect(hdc, &rCenter, mainB);
            SelectObject(hdc, whiteB); Ellipse(hdc, cx-r13, cy-r13, cx-r7, cy-r7);
        } else if (c == 2) {
            RECT rOuter = { cx-r15, cy-r15, cx+r15, cy+r15 };
            RoundRect(hdc, rOuter.left, rOuter.top, rOuter.right, rOuter.bottom, (int)(8*scale), (int)(8*scale));
            POINT pStar1[3] = {{cx, cy-r15}, {cx+r7, cy}, {cx-r7, cy}};
            POINT pStar2[3] = {{cx, cy+r15}, {cx+r7, cy}, {cx-r7, cy}};
            POINT pStar3[3] = {{cx-r15, cy}, {cx, cy-r7}, {cx, cy+r7}};
            POINT pStar4[3] = {{cx+r15, cy}, {cx, cy-r7}, {cx, cy+r7}};
            SelectObject(hdc, lightB); Polygon(hdc, pStar1, 3); Polygon(hdc, pStar3, 3);
            SelectObject(hdc, darkB); Polygon(hdc, pStar2, 3); Polygon(hdc, pStar4, 3);
            SelectObject(hdc, whiteB); Ellipse(hdc, cx-r7, cy-r9, cx-r3, cy-r3);
        } else if (c == 3) {
            POINT pDiamond[4] = {{cx, cy-r17}, {cx+r16, cy}, {cx, cy+r17}, {cx-r16, cy}};
            Polygon(hdc, pDiamond, 4);
            POINT p1[3] = {{cx, cy-r17}, {cx+r16, cy}, {cx, cy}};
            POINT p2[3] = {{cx, cy-r17}, {cx-r16, cy}, {cx, cy}};
            POINT p3[3] = {{cx, cy+r17}, {cx+r16, cy}, {cx, cy}};
            POINT p4[3] = {{cx, cy+r17}, {cx-r16, cy}, {cx, cy}};
            SelectObject(hdc, lightB); Polygon(hdc, p1, 3);
            SelectObject(hdc, whiteB); Polygon(hdc, p2, 3);
            SelectObject(hdc, darkB); Polygon(hdc, p3, 3); Polygon(hdc, p4, 3);
        } else if (c == 4) {
            POINT pHex[6] = {{cx, cy-r17}, {cx+r15, cy-r7}, {cx+r13, cy+r13}, {cx, cy+r17}, {cx-r13, cy+r13}, {cx-r15, cy-r7}};
            Polygon(hdc, pHex, 6);
            POINT p1[3] = {{cx, cy-r17}, {cx+r15, cy-r7}, {cx, cy}};
            POINT p2[3] = {{cx, cy-r17}, {cx-r15, cy-r7}, {cx, cy}};
            POINT p3[3] = {{cx-r15, cy-r7}, {cx-r13, cy+r13}, {cx, cy}};
            POINT p4[3] = {{cx+r15, cy-r7}, {cx+r13, cy+r13}, {cx, cy}};
            SelectObject(hdc, lightB); Polygon(hdc, p1, 3);
            SelectObject(hdc, whiteB); Polygon(hdc, p2, 3);
            SelectObject(hdc, darkB); Polygon(hdc, p3, 3); Polygon(hdc, p4, 3);
        } else {
            Ellipse(hdc, cx-r16, cy-r16, cx+r16, cy+r16);
            POINT pStar1[4] = {{cx, cy-r16}, {cx+r5, cy-r5}, {cx, cy}, {cx-r5, cy-r5}};
            POINT pStar2[4] = {{cx+r16, cy}, {cx+r5, cy+r5}, {cx, cy}, {cx+r5, cy-r5}};
            POINT pStar3[4] = {{cx, cy+r16}, {cx-r5, cy+r5}, {cx, cy}, {cx+r5, cy+r5}};
            POINT pStar4[4] = {{cx-r16, cy}, {cx-r5, cy-r5}, {cx, cy}, {cx-r5, cy+r5}};
            SelectObject(hdc, lightB); Polygon(hdc, pStar1, 4); Polygon(hdc, pStar4, 4);
            SelectObject(hdc, darkB); Polygon(hdc, pStar2, 4); Polygon(hdc, pStar3, 4);
            SelectObject(hdc, whiteB); Ellipse(hdc, cx-r3, cy-r3, cx+r3, cy+r3);
        }

        SelectObject(hdc, oldB); SelectObject(hdc, oldP);
        DeleteObject(mainB); DeleteObject(lightB); DeleteObject(darkB); DeleteObject(whiteB); DeleteObject(penDark);

        HBRUSH sssB = CreateSolidBrush(lightC);
        HBRUSH wB = CreateSolidBrush(RGB(255,255,255));
        HPEN nullP = CreatePen(PS_NULL, 0, 0);
        HPEN oP2 = (HPEN)SelectObject(hdc, nullP);
        
        SelectObject(hdc, sssB);
        Ellipse(hdc, cx - r9, cy + r5, cx + r9, cy + r15);
        
        SelectObject(hdc, wB);
        POINT specTop[3] = {{cx - r5, cy - r14}, {cx + r3, cy - r14}, {cx - r3, cy - r7}};
        Polygon(hdc, specTop, 3);
        Ellipse(hdc, cx + r5, cy - r12, cx + r9, cy - r8);
        
        SelectObject(hdc, oP2);
        DeleteObject(nullP); DeleteObject(sssB); DeleteObject(wB);

        if (typeIdx == TYPE_HORIZ) {
            DWORD tick = GetTickCount();
            float pulse = (sinf(tick * 0.005f) + 1.0f) * 0.5f;
            HPEN goldP = CreatePen(PS_SOLID, 2, RGB(255, 150 + (int)(105 * pulse), 0));
            HBRUSH goldB = CreateSolidBrush(RGB(255, 100 + (int)(70 * pulse), 0));
            HPEN oP = (HPEN)SelectObject(hdc, goldP);
            HBRUSH oB = (HBRUSH)SelectObject(hdc, goldB);
            Ellipse(hdc, cx-r19, cy-r19, cx+r19, cy+r19);
            POINT arrL[3] = {{cx-r16, cy}, {cx-r7, cy-r5}, {cx-r7, cy+r5}};
            POINT arrR[3] = {{cx+r16, cy}, {cx+r7, cy-r5}, {cx+r7, cy+r5}};
            SelectObject(hdc, GetStockObject(WHITE_BRUSH));
            Polygon(hdc, arrL, 3); Polygon(hdc, arrR, 3);
            RECT lineR = {cx-r7, cy-(int)(2*scale), cx+r7, cy+(int)(2*scale)};
            FillRect(hdc, &lineR, (HBRUSH)GetStockObject(WHITE_BRUSH));
            SelectObject(hdc, oP); SelectObject(hdc, oB);
            DeleteObject(goldP); DeleteObject(goldB);
        } else if (typeIdx == TYPE_VERT) {
            DWORD tick = GetTickCount();
            float pulse = (sinf(tick * 0.005f) + 1.0f) * 0.5f;
            HPEN cyanP = CreatePen(PS_SOLID, 2, RGB(0, 150 + (int)(105 * pulse), 255));
            HBRUSH cyanB = CreateSolidBrush(RGB(0, 100 + (int)(100 * pulse), 255));
            HPEN oP = (HPEN)SelectObject(hdc, cyanP);
            HBRUSH oB = (HBRUSH)SelectObject(hdc, cyanB);
            Ellipse(hdc, cx-r19, cy-r19, cx+r19, cy+r19);
            POINT arrU[3] = {{cx, cy-r16}, {cx-r5, cy-r7}, {cx+r5, cy-r7}};
            POINT arrD[3] = {{cx, cy+r16}, {cx-r5, cy+r7}, {cx+r5, cy+r7}};
            SelectObject(hdc, GetStockObject(WHITE_BRUSH));
            Polygon(hdc, arrU, 3); Polygon(hdc, arrD, 3);
            RECT lineR = {cx-(int)(2*scale), cy-r7, cx+(int)(2*scale), cy+r7};
            FillRect(hdc, &lineR, (HBRUSH)GetStockObject(WHITE_BRUSH));
            SelectObject(hdc, oP); SelectObject(hdc, oB);
            DeleteObject(cyanP); DeleteObject(cyanB);
        } else if (typeIdx == TYPE_BOMB) {
            DWORD tick = GetTickCount();
            float pulse = (sinf(tick * 0.01f) + 1.0f) * 0.5f;
            HBRUSH darkBomb = CreateSolidBrush(RGB(15 + (int)(20 * pulse), 15, 15));
            HPEN redP = CreatePen(PS_SOLID, 2, RGB(255, 40 + (int)(60 * pulse), 40));
            HPEN oP = (HPEN)SelectObject(hdc, redP);
            HBRUSH oB = (HBRUSH)SelectObject(hdc, darkBomb);
            Ellipse(hdc, cx-r16, cy-r16, cx+r16, cy+r16);
            HBRUSH redCore = CreateSolidBrush(RGB(255, 40 + (int)(60 * pulse), 40));
            SelectObject(hdc, redCore);
            Ellipse(hdc, cx-r5, cy-r5, cx+r5, cy+r5);
            DeleteObject(redCore);
            HBRUSH sparkB = CreateSolidBrush(RGB(255, 200, 0));
            SelectObject(hdc, sparkB);
            Ellipse(hdc, cx+r7, cy-r16, cx+r13, cy-r9);
            DeleteObject(sparkB);
            SelectObject(hdc, oP); SelectObject(hdc, oB);
            DeleteObject(darkBomb); DeleteObject(redP);
        }
    }

    if (isIce) {
        HBRUSH iceBrush = CreateSolidBrush(RGB(0, 229, 255));
        RECT iRect = { cx-r19, cy-r19, cx+r19, cy+r19 };
        FrameRect(hdc, &iRect, iceBrush);
        DeleteObject(iceBrush);
        HPEN iceP = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        HPEN oP = (HPEN)SelectObject(hdc, iceP);
        MoveToEx(hdc, cx-r13, cy-r9, NULL); LineTo(hdc, cx+r5, cy+r9);
        MoveToEx(hdc, cx+r9, cy-r13, NULL); LineTo(hdc, cx-r9, cy+r13);
        SelectObject(hdc, oP); DeleteObject(iceP);
    }

    if (isBarrier) {
        HPEN shieldP = CreatePen(PS_SOLID, 2, RGB(255, 215, 0));
        HBRUSH shieldB = (HBRUSH)GetStockObject(NULL_BRUSH);
        HPEN oP = (HPEN)SelectObject(hdc, shieldP);
        HBRUSH oB = (HBRUSH)SelectObject(hdc, shieldB);
        Ellipse(hdc, cx-r19-2, cy-r19-2, cx+r19+2, cy+r19+2);
        SelectObject(hdc, oP); SelectObject(hdc, oB);
        DeleteObject(shieldP);
    }

    if (popAnim > 0) {
        HPEN crackP = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
        HPEN crackP2 = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HPEN oPc = (HPEN)SelectObject(hdc, crackP);
        MoveToEx(hdc, cx - r7, cy - r11, NULL);
        LineTo(hdc, cx + r3, cy - r3);
        LineTo(hdc, cx - r3, cy + r5);
        LineTo(hdc, cx + r7, cy + r13);
        SelectObject(hdc, crackP2);
        MoveToEx(hdc, cx - r7, cy - r11, NULL);
        LineTo(hdc, cx + r3, cy - r3);
        LineTo(hdc, cx - r3, cy + r5);
        LineTo(hdc, cx + r7, cy + r13);
        SelectObject(hdc, oPc);
        DeleteObject(crackP); DeleteObject(crackP2);
    }
}

void SaveStats() {
    FILE *f = fopen("kmatch3_stats.dat", "wb");
    if (!f) return;
    fwrite(&statsGamesPlayed, sizeof(int), 1, f);
    fwrite(&statsBestScore, sizeof(int), 1, f);
    fwrite(&statsMaxCombo, sizeof(int), 1, f);
    fclose(f);
}

void LoadStats() {
    FILE *f = fopen("kmatch3_stats.dat", "rb");
    if (!f) return;
    fread(&statsGamesPlayed, sizeof(int), 1, f);
    fread(&statsBestScore, sizeof(int), 1, f);
    fread(&statsMaxCombo, sizeof(int), 1, f);
    fclose(f);
}

void PlaySwapSound() { Beep(300, 80); }
void PlayMatchSound(int combo) { Beep(400 + combo * 100, 120); }
void PlayBadSwapSound() { Beep(150, 120); }
void PlayPowerupSound() { Beep(700, 150); }

void InitStage(int stageIdx) {
    if (gameMode == 0) {
        if (stageIdx < 1) stageIdx = 1;
        if (stageIdx > 20) stageIdx = 20;
        level = stageIdx;
        const StageConfig *cfg = &CAMPAIGN_STAGES[level - 1];
        rows = cfg->rows;
        cols = cfg->cols;
        targetScore = cfg->targetScore;
        moves = (cfg->timeLimit > 0) ? cfg->timeLimit : cfg->moves;
        bossHP = cfg->bossHP;
        maxBossHP = cfg->bossHP;
    } else if (gameMode == 1) {
        level = 1;
        rows = 8; cols = 8;
        targetScore = 2000;
        moves = 0;
        bossHP = 0; maxBossHP = 0;
    } else if (gameMode == 2) {
        level = 1;
        rows = 8; cols = 8;
        targetScore = 1000;
        moves = 60;
        bossHP = 0; maxBossHP = 0;
    }

    cellSize = BOARD_SIZE / cols;
    selR = -1; selC = -1;
    collectedRed = 0; collectedGreen = 0; collectedBlue = 0;
    bossMoveTimer = 0;
    powerupMode = 0;

    memset(typeGrid, 0, sizeof(typeGrid));
    memset(iceGrid, 0, sizeof(iceGrid));
    memset(stoneGrid, 0, sizeof(stoneGrid));
    memset(barrierGrid, 0, sizeof(barrierGrid));

    int iceToPlace = (gameMode == 0) ? CAMPAIGN_STAGES[level - 1].iceCount : (gameMode == 1 ? 0 : 4);
    int stoneToPlace = (gameMode == 0) ? CAMPAIGN_STAGES[level - 1].stoneCount : 0;
    int ironToPlace = (gameMode == 0) ? CAMPAIGN_STAGES[level - 1].ironCount : 0;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int color;
            do {
                color = rand() % 6;
            } while (
                (r >= 2 && grid[r-1][c] == color && grid[r-2][c] == color) ||
                (c >= 2 && grid[r][c-1] == color && grid[r][c-2] == color)
            );
            grid[r][c] = color;
        }
    }

    while (ironToPlace > 0) {
        int r = rand() % rows;
        int c = rand() % cols;
        if (!stoneGrid[r][c]) {
            stoneGrid[r][c] = 2 + (rand() % 2); // 2 or 3 hits
            grid[r][c] = -1;
            ironToPlace--;
        }
    }

    while (stoneToPlace > 0) {
        int r = rand() % rows;
        int c = rand() % cols;
        if (!stoneGrid[r][c]) {
            stoneGrid[r][c] = 1; // 1 hit
            grid[r][c] = -1;
            stoneToPlace--;
        }
    }

    while (iceToPlace > 0) {
        int r = rand() % rows;
        int c = rand() % cols;
        if (!stoneGrid[r][c] && !iceGrid[r][c]) {
            iceGrid[r][c] = 1;
            iceToPlace--;
        }
    }

    if (!CheckMatchPossible()) {
        ShuffleBoard();
    }
}

void TriggerBossAction(HWND hwnd) {
    if (bossHP <= 0) return;
    int count = 0;
    for (int i = 0; i < 2; i++) {
        int r = rand() % rows;
        int c = rand() % cols;
        if (!stoneGrid[r][c]) {
            barrierGrid[r][c] = 1;
            count++;
        }
    }
    if (count > 0) {
        PlayBadSwapSound();
    }
}

void DrawBoard(HDC hdc) {
    char buf[128];
    int isTimedStage = (gameMode == 0 && CAMPAIGN_STAGES[level-1].timeLimit > 0) || (gameMode == 2);
    
    if (gameMode == 0) {
        if (level == 20) {
            sprintf(buf, "Stage %s | BOSS HP: %d/%d | Moves: %d", CAMPAIGN_STAGES[level-1].name, bossHP, maxBossHP, moves);
        } else {
            sprintf(buf, "Stage %s | Score: %d/%d | %s: %d", CAMPAIGN_STAGES[level-1].name, score, targetScore, isTimedStage ? "Time" : "Moves", moves);
        }
    } else if (gameMode == 1) {
        sprintf(buf, "Zen Mode | Score: %d | Level %d", score, level);
    } else if (gameMode == 2) {
        sprintf(buf, "Timed Rush | Score: %d/%d | Time: %ds", score, targetScore, moves);
    }

    SetTextColor(hdc, RGB(255, 215, 0));
    SetBkMode(hdc, TRANSPARENT);
    TextOut(hdc, BOARD_X, 8, buf, strlen(buf));
    
    int iceCount = 0, stoneCount = 0;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (iceGrid[r][c]) iceCount++;
            if (stoneGrid[r][c]) stoneCount++;
        }
    }

    char statsBuf[128];
    const StageConfig *cfg = &CAMPAIGN_STAGES[level - 1];
    if (gameMode == 0 && (cfg->redTarget > 0 || cfg->greenTarget > 0 || cfg->blueTarget > 0)) {
        sprintf(statsBuf, "Red: %d/%d | Grn: %d/%d | Blu: %d/%d | Best: %d", 
            collectedRed, cfg->redTarget, collectedGreen, cfg->greenTarget, collectedBlue, cfg->blueTarget, statsBestScore);
    } else {
        sprintf(statsBuf, "Ice: %d | Stone/Iron: %d | Best: %d | Combo: x%d", iceCount, stoneCount, statsBestScore, statsMaxCombo);
    }
    SetTextColor(hdc, RGB(200, 200, 200));
    TextOut(hdc, BOARD_X, 28, statsBuf, strlen(statsBuf));

    char powerupBuf[128];
    const char *pModeStr = "";
    if (powerupMode == 1) pModeStr = " (HAMMER)";
    else if (powerupMode == 2) pModeStr = " (NUKE)";
    sprintf(powerupBuf, "[H]Hammer [E]+Moves [S]Shuffle [L]Nuke%s", pModeStr);
    SetTextColor(hdc, powerupMode ? RGB(255, 100, 100) : RGB(255, 215, 0));
    TextOut(hdc, BOARD_X, 46, powerupBuf, strlen(powerupBuf));

    // Boss HP Bar
    if (gameMode == 0 && maxBossHP > 0) {
        RECT hpBarBorder = { BOARD_X, 66, BOARD_X + BOARD_SIZE, 78 };
        HBRUSH bB = CreateSolidBrush(RGB(50, 20, 20));
        FillRect(hdc, &hpBarBorder, bB);
        DeleteObject(bB);

        int filledWidth = (int)((float)bossHP / (float)maxBossHP * (float)BOARD_SIZE);
        if (filledWidth > 0) {
            RECT hpBarFill = { BOARD_X, 66, BOARD_X + filledWidth, 78 };
            HBRUSH fB = CreateSolidBrush(RGB(255, 40, 40));
            FillRect(hdc, &hpBarFill, fB);
            DeleteObject(fB);
        }
        HBRUSH fBorder = CreateSolidBrush(RGB(255, 215, 0));
        FrameRect(hdc, &hpBarBorder, fBorder);
        DeleteObject(fBorder);
    }

    // 3D Outer Frame
    RECT outerFrame = { BOARD_X - 10, BOARD_Y - 10, BOARD_X + cols * cellSize + 10, BOARD_Y + rows * cellSize + 10 };
    HBRUSH frameBrush = CreateSolidBrush(RGB(180, 130, 20));
    FillRect(hdc, &outerFrame, frameBrush);
    DeleteObject(frameBrush);

    HPEN framePenHigh = CreatePen(PS_SOLID, 3, RGB(255, 215, 0));
    HPEN framePenLow = CreatePen(PS_SOLID, 3, RGB(90, 60, 10));
    HPEN framePenMid = CreatePen(PS_SOLID, 1, RGB(255, 255, 100));
    HPEN oldPen = (HPEN)SelectObject(hdc, framePenHigh);

    // Bevel Outer
    MoveToEx(hdc, outerFrame.left, outerFrame.bottom, NULL);
    LineTo(hdc, outerFrame.left, outerFrame.top);
    LineTo(hdc, outerFrame.right, outerFrame.top);
    SelectObject(hdc, framePenLow);
    LineTo(hdc, outerFrame.right, outerFrame.bottom);
    LineTo(hdc, outerFrame.left, outerFrame.bottom);
    
    // Inner bevel
    RECT innerF = { outerFrame.left + 4, outerFrame.top + 4, outerFrame.right - 4, outerFrame.bottom - 4 };
    SelectObject(hdc, framePenLow);
    MoveToEx(hdc, innerF.left, innerF.bottom, NULL);
    LineTo(hdc, innerF.left, innerF.top);
    LineTo(hdc, innerF.right, innerF.top);
    SelectObject(hdc, framePenHigh);
    LineTo(hdc, innerF.right, innerF.bottom);
    LineTo(hdc, innerF.left, innerF.bottom);

    // Highlights
    SelectObject(hdc, framePenMid);
    MoveToEx(hdc, outerFrame.left+1, outerFrame.top+1, NULL);
    LineTo(hdc, outerFrame.right-1, outerFrame.top+1);
    
    HBRUSH studB = CreateSolidBrush(RGB(255, 240, 100));
    HBRUSH oldB2 = (HBRUSH)SelectObject(hdc, studB);
    Ellipse(hdc, outerFrame.left+2, outerFrame.top+2, outerFrame.left+9, outerFrame.top+9);
    Ellipse(hdc, outerFrame.right-9, outerFrame.top+2, outerFrame.right-2, outerFrame.top+9);
    Ellipse(hdc, outerFrame.left+2, outerFrame.bottom-9, outerFrame.left+9, outerFrame.bottom-2);
    Ellipse(hdc, outerFrame.right-9, outerFrame.bottom-9, outerFrame.right-2, outerFrame.bottom-2);
    SelectObject(hdc, oldB2);
    SelectObject(hdc, oldPen);
    DeleteObject(studB); DeleteObject(framePenHigh); DeleteObject(framePenLow); DeleteObject(framePenMid);

    // Grid Cells
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            RECT rect = { BOARD_X + c * cellSize, BOARD_Y + r * cellSize, 
                          BOARD_X + (c + 1) * cellSize, BOARD_Y + (r + 1) * cellSize };
            HBRUSH bg = CreateSolidBrush(RGB(22, 16, 28));
            FillRect(hdc, &rect, bg);
            DeleteObject(bg);
            HBRUSH border = CreateSolidBrush(RGB(40, 32, 50));
            FrameRect(hdc, &rect, border);
            DeleteObject(border);
        }
    }

    HRGN hRgn = CreateRectRgn(BOARD_X, BOARD_Y, BOARD_X + cols * cellSize, BOARD_Y + rows * cellSize);
    SelectClipRgn(hdc, hRgn);

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int drawX = BOARD_X + c * cellSize;
            int drawY = BOARD_Y + r * cellSize;

            if (r == swapR1 && c == swapC1) {
                drawX += (swapC2 - swapC1) * cellSize * swapAnim / 10;
                drawY += (swapR2 - swapR1) * cellSize * swapAnim / 10;
            } else if (r == swapR2 && c == swapC2) {
                drawX += (swapC1 - swapC2) * cellSize * swapAnim / 10;
                drawY += (swapR1 - swapR2) * cellSize * swapAnim / 10;
            }

            if (dropAnim > 0 && dropCount[r][c] > 0) {
                int startY = drawY - (dropCount[r][c] * cellSize);
                drawY = startY + (drawY - startY) * dropAnim / 10;
            }

            RECT rect = { drawX, drawY, drawX + cellSize, drawY + cellSize };

            if (popGrid[r][c] && popAnim > 0) {
                int shrink = (cellSize / 2) * popAnim / 10;
                rect.left += shrink;
                rect.top += shrink;
                rect.right -= shrink;
                rect.bottom -= shrink;
                if (rect.right <= rect.left) continue;
            }

            if (stoneGrid[r][c] > 0) {
                int hitsLeft = stoneGrid[r][c];
                COLORREF sCol = (hitsLeft == 1) ? RGB(80, 80, 85) : (hitsLeft == 2 ? RGB(50, 50, 60) : RGB(30, 30, 45));
                HBRUSH stoneBrush = CreateSolidBrush(sCol);
                FillRect(hdc, &rect, stoneBrush);
                DeleteObject(stoneBrush);

                HBRUSH stoneBorder = CreateSolidBrush(RGB(30, 30, 35));
                FrameRect(hdc, &rect, stoneBorder);
                DeleteObject(stoneBorder);

                COLORREF markC = (hitsLeft == 1) ? RGB(255, 215, 0) : RGB(0, 229, 255);
                HPEN pen = CreatePen(PS_SOLID, 2, markC);
                HPEN oldPen = (HPEN)SelectObject(hdc, pen);
                int cx = rect.left + cellSize/2, cy = rect.top + cellSize/2;
                int off = (int)(cellSize * 0.2f);
                MoveToEx(hdc, cx - off, cy, NULL); LineTo(hdc, cx + off, cy);
                if (hitsLeft >= 2) {
                    MoveToEx(hdc, cx, cy - off, NULL); LineTo(hdc, cx, cy + off);
                }
                SelectObject(hdc, oldPen); DeleteObject(pen);
                continue;
            }

            if (grid[r][c] == -1) continue;

            int cx = drawX + cellSize / 2;
            int cy = drawY + cellSize / 2;
            int isMoving = (r == swapR1 && c == swapC1) || (r == swapR2 && c == swapC2) || (dropAnim > 0 && dropCount[r][c] > 0);
            int pAnim = popGrid[r][c] ? popAnim : 0;
            DrawFacetedGem(hdc, cx, cy, cellSize, grid[r][c], typeGrid[r][c], iceGrid[r][c], barrierGrid[r][c], isMoving, pAnim);

            if (r == selR && c == selC) {
                HBRUSH border = CreateSolidBrush((powerupMode > 0) ? RGB(255, 0, 0) : RGB(255, 215, 0));
                FrameRect(hdc, &rect, border);
                DeleteObject(border);
                
                rect.left += 1; rect.top += 1; rect.right -= 1; rect.bottom -= 1;
                border = CreateSolidBrush(RGB(255, 255, 255));
                FrameRect(hdc, &rect, border);
                DeleteObject(border);
            } else if ((r == hintR1 && c == hintC1) || (r == hintR2 && c == hintC2)) {
                int glow = (int)((sin((float)hintTimer * 0.5f) + 1.0f) * 127.0f);
                COLORREF glowC = RGB(128 + glow/2, 128 + glow/2, 255);
                HPEN glowPen = CreatePen(PS_SOLID, 3, glowC);
                HPEN oldGlow = (HPEN)SelectObject(hdc, glowPen);
                HBRUSH nullB = (HBRUSH)GetStockObject(NULL_BRUSH);
                HBRUSH oldB = (HBRUSH)SelectObject(hdc, nullB);
                
                int bz = 2 + (int)((sin((float)hintTimer * 0.5f) + 1.0f) * 2.0f);
                RoundRect(hdc, rect.left - bz, rect.top - bz, rect.right + bz, rect.bottom + bz, 8, 8);
                
                SelectObject(hdc, oldGlow);
                SelectObject(hdc, oldB);
                DeleteObject(glowPen);
            }
        }
    }

    // Laser Beam Overlays
    int offLaser = (int)(cellSize * 0.4f);
    for (int r = 0; r < rows; r++) {
        if (laserRowTimer[r] > 0) {
            RECT lRect = { BOARD_X, BOARD_Y + r * cellSize + offLaser, BOARD_X + cols * cellSize, BOARD_Y + r * cellSize + offLaser + 10 };
            HBRUSH lB = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(hdc, &lRect, lB);
            DeleteObject(lB);
            laserRowTimer[r]--;
        }
    }
    for (int c = 0; c < cols; c++) {
        if (laserColTimer[c] > 0) {
            RECT lRect = { BOARD_X + c * cellSize + offLaser, BOARD_Y, BOARD_X + c * cellSize + offLaser + 10, BOARD_Y + rows * cellSize };
            HBRUSH lB = CreateSolidBrush(RGB(0, 229, 255));
            FillRect(hdc, &lRect, lB);
            DeleteObject(lB);
            laserColTimer[c]--;
        }
    }
    SelectClipRgn(hdc, NULL);
    DeleteObject(hRgn);

    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0) {
            if (particles[i].type == 2) { // Smoke
                HBRUSH pb = CreateSolidBrush(particles[i].color);
                HPEN pp = CreatePen(PS_NULL, 0, 0);
                HBRUSH ob = (HBRUSH)SelectObject(hdc, pb);
                HPEN op = (HPEN)SelectObject(hdc, pp);
                int sz = (int)particles[i].size;
                Ellipse(hdc, (int)particles[i].x - sz, (int)particles[i].y - sz, (int)particles[i].x + sz, (int)particles[i].y + sz);
                SelectObject(hdc, ob);
                SelectObject(hdc, op);
                DeleteObject(pb);
                DeleteObject(pp);
            } else {
                float r = particles[i].rot * 3.14159f / 180.0f;
                float c_rot = cosf(r);
                float s_rot = sinf(r);
                float sz = particles[i].size;
                float px = particles[i].x;
                float py = particles[i].y;
                
                POINT pts[3];
                pts[0].x = (int)(px + c_rot * sz - s_rot * 0);
                pts[0].y = (int)(py + s_rot * sz + c_rot * 0);
                pts[1].x = (int)(px + c_rot * (-sz) - s_rot * sz);
                pts[1].y = (int)(py + s_rot * (-sz) + c_rot * sz);
                pts[2].x = (int)(px + c_rot * (-sz) - s_rot * (-sz));
                pts[2].y = (int)(py + s_rot * (-sz) + c_rot * (-sz));

                HBRUSH pb = CreateSolidBrush(particles[i].color);
                HPEN pp = CreatePen(PS_SOLID, 1, RGB(255,255,255));
                if (particles[i].type == 1) { // Debris
                    DeleteObject(pp);
                    pp = CreatePen(PS_NULL, 0, 0);
                }
                HBRUSH ob = (HBRUSH)SelectObject(hdc, pb);
                HPEN op = (HPEN)SelectObject(hdc, pp);
                Polygon(hdc, pts, 3);
                SelectObject(hdc, ob);
                SelectObject(hdc, op);
                DeleteObject(pb);
                DeleteObject(pp);
            }
        }
    }
}

int toDestroy[MAX_ROWS][MAX_COLS] = {0};
int stoneToBreak[MAX_ROWS][MAX_COLS] = {0};

void AddDestroy(int r, int c) {
    if (r < 0 || r >= rows || c < 0 || c >= cols) return;
    if (stoneGrid[r][c] > 0) {
        stoneToBreak[r][c] = 1;
        return;
    }
    if (grid[r][c] == -1) return;
    if (toDestroy[r][c]) return;
    
    toDestroy[r][c] = 1;

    int dr[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};
    for (int i = 0; i < 4; i++) {
        int nr = r + dr[i];
        int nc = c + dc[i];
        if (nr >= 0 && nr < rows && nc >= 0 && nc < cols && stoneGrid[nr][nc] > 0) {
            stoneToBreak[nr][nc] = 1;
        }
    }

    if (typeGrid[r][c] == TYPE_HORIZ) {
        laserRowTimer[r] = 10;
        for (int i = 0; i < cols; i++) AddDestroy(r, i);
    } else if (typeGrid[r][c] == TYPE_VERT) {
        laserColTimer[c] = 10;
        for (int i = 0; i < rows; i++) AddDestroy(i, c);
    } else if (typeGrid[r][c] == TYPE_RAINBOW) {
        int tcolor = rand() % 6;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (grid[i][j] == tcolor && typeGrid[i][j] != TYPE_RAINBOW && stoneGrid[i][j] == 0) AddDestroy(i, j);
            }
        }
    } else if (typeGrid[r][c] == TYPE_BOMB) {
        for (int dr2 = -1; dr2 <= 1; dr2++) {
            for (int dc2 = -1; dc2 <= 1; dc2++) {
                AddDestroy(r + dr2, c + dc2);
            }
        }
    }
}

void ClearHint() {
    hintTimer = 0;
    hintR1 = -1; hintC1 = -1; hintR2 = -1; hintC2 = -1;
}

void FindHint() {
    for (int r=0; r<rows; r++) {
        for (int c=0; c<cols; c++) {
            if (grid[r][c] == -1 || stoneGrid[r][c] > 0 || iceGrid[r][c]) continue;
            if (c < cols - 1 && grid[r][c+1] != -1 && stoneGrid[r][c+1] == 0 && !iceGrid[r][c+1]) {
                int temp = grid[r][c]; grid[r][c] = grid[r][c+1]; grid[r][c+1] = temp;
                int m = CheckMatchPossible();
                temp = grid[r][c]; grid[r][c] = grid[r][c+1]; grid[r][c+1] = temp;
                if (m) { hintR1 = r; hintC1 = c; hintR2 = r; hintC2 = c+1; return; }
            }
            if (r < rows - 1 && grid[r+1][c] != -1 && stoneGrid[r+1][c] == 0 && !iceGrid[r+1][c]) {
                int temp = grid[r][c]; grid[r][c] = grid[r+1][c]; grid[r+1][c] = temp;
                int m = CheckMatchPossible();
                temp = grid[r][c]; grid[r][c] = grid[r+1][c]; grid[r+1][c] = temp;
                if (m) { hintR1 = r; hintC1 = c; hintR2 = r+1; hintC2 = c; return; }
            }
        }
    }
}

int CheckMatchPossible() {
    for (int r=0; r<rows; r++) {
        for (int c=0; c<cols-2; c++) {
            int color = grid[r][c];
            if (color != -1 && stoneGrid[r][c] == 0 && typeGrid[r][c] != TYPE_RAINBOW &&
                grid[r][c+1] == color && stoneGrid[r][c+1] == 0 && typeGrid[r][c+1] != TYPE_RAINBOW &&
                grid[r][c+2] == color && stoneGrid[r][c+2] == 0 && typeGrid[r][c+2] != TYPE_RAINBOW) return 1;
        }
    }
    for (int c=0; c<cols; c++) {
        for (int r=0; r<rows-2; r++) {
            int color = grid[r][c];
            if (color != -1 && stoneGrid[r][c] == 0 && typeGrid[r][c] != TYPE_RAINBOW &&
                grid[r+1][c] == color && stoneGrid[r+1][c] == 0 && typeGrid[r+1][c] != TYPE_RAINBOW &&
                grid[r+2][c] == color && stoneGrid[r+2][c] == 0 && typeGrid[r+2][c] != TYPE_RAINBOW) return 1;
        }
    }
    return 0;
}

void AnimateSwap(HWND hwnd, int r1, int c1, int r2, int c2) {
    swapR1 = r1; swapC1 = c1; swapR2 = r2; swapC2 = c2;
    float easeVals[10] = {0.2f, 0.5f, 0.8f, 1.0f, 1.1f, 1.05f, 0.95f, 1.0f, 1.0f, 1.0f};
    for (int i = 0; i < 10; i++) {
        swapAnim = (int)(easeVals[i] * 10.0f);
        UpdateParticles();
        InvalidateRect(hwnd, NULL, FALSE);
        UpdateWindow(hwnd);
        Sleep(20);
    }
    swapR1 = -1;
}

void ProcessMatches(HWND hwnd, int triggerR, int triggerC, int triggerColor) {
    int comboMultiplier = 1;
    int hasMatches = 1;
    
    while (hasMatches) {
        memset(toDestroy, 0, sizeof(toDestroy));
        memset(stoneToBreak, 0, sizeof(stoneToBreak));
        int newSpecials[MAX_ROWS][MAX_COLS] = {0}; 

        if (triggerR != -1) {
            if (stoneGrid[triggerR][triggerC] > 0) {
                stoneToBreak[triggerR][triggerC] = 1;
            } else {
                toDestroy[triggerR][triggerC] = 1;
            }

            if (triggerColor == 999) { // Rainbow + Rainbow
                for(int i=0; i<rows; i++) for(int j=0; j<cols; j++) if(stoneGrid[i][j] == 0) AddDestroy(i, j);
            } else if (triggerColor == 888) { // Line + Line
                typeGrid[triggerR][triggerC] = TYPE_HORIZ; AddDestroy(triggerR, triggerC);
                typeGrid[lastSwapR1][lastSwapC1] = TYPE_VERT; AddDestroy(lastSwapR1, lastSwapC1);
            } else if (triggerColor == 777) { // Hammer
                AddDestroy(triggerR, triggerC);
            } else if (triggerColor >= 900 && triggerColor < 906) { // Color Nuke
                int targetC = triggerColor - 900;
                for(int i=0; i<rows; i++) for(int j=0; j<cols; j++) {
                    if (grid[i][j] == targetC && stoneGrid[i][j] == 0) AddDestroy(i, j);
                }
            } else if (triggerColor >= 0 && triggerColor < 6) { // Rainbow + Color
                for(int i=0; i<rows; i++) for(int j=0; j<cols; j++) {
                    if (grid[i][j] == triggerColor && stoneGrid[i][j] == 0) AddDestroy(i, j);
                }
            }
            triggerR = -1; 
        }

        int hMatchLen[MAX_ROWS][MAX_COLS] = {0};
        int vMatchLen[MAX_ROWS][MAX_COLS] = {0};
        
        for (int r=0; r<rows; r++) {
            for (int c=0; c<cols-2; c++) {
                int color = grid[r][c];
                if (color != -1 && stoneGrid[r][c] == 0 && typeGrid[r][c] != TYPE_RAINBOW && 
                    grid[r][c+1] == color && stoneGrid[r][c+1] == 0 && typeGrid[r][c+1] != TYPE_RAINBOW && 
                    grid[r][c+2] == color && stoneGrid[r][c+2] == 0 && typeGrid[r][c+2] != TYPE_RAINBOW) {
                    int k = c;
                    while(k < cols && grid[r][k] == color && stoneGrid[r][k] == 0 && typeGrid[r][k] != TYPE_RAINBOW) k++;
                    int len = k - c;
                    for(int i = c; i < k; i++) hMatchLen[r][i] = len;
                    c = k - 1;
                }
            }
        }
        for (int c=0; c<cols; c++) {
            for (int r=0; r<rows-2; r++) {
                int color = grid[r][c];
                if (color != -1 && stoneGrid[r][c] == 0 && typeGrid[r][c] != TYPE_RAINBOW && 
                    grid[r+1][c] == color && stoneGrid[r+1][c] == 0 && typeGrid[r+1][c] != TYPE_RAINBOW && 
                    grid[r+2][c] == color && stoneGrid[r+2][c] == 0 && typeGrid[r+2][c] != TYPE_RAINBOW) {
                    int k = r;
                    while(k < rows && grid[k][c] == color && stoneGrid[k][c] == 0 && typeGrid[k][c] != TYPE_RAINBOW) k++;
                    int len = k - r;
                    for(int i = r; i < k; i++) vMatchLen[i][c] = len;
                    r = k - 1;
                }
            }
        }
        
        for(int r=0; r<rows; r++) {
            for(int c=0; c<cols; c++) {
                if (hMatchLen[r][c] >= 3 || vMatchLen[r][c] >= 3) {
                    AddDestroy(r, c);
                }
            }
        }

        for(int r=0; r<rows; r++) {
            for(int c=0; c<cols; c++) {
                if (hMatchLen[r][c] >= 3 && vMatchLen[r][c] >= 3) { // T / L Bomb Gem
                    newSpecials[r][c] = TYPE_BOMB;
                } else if (hMatchLen[r][c] >= 5) {
                    int sC = c;
                    for (int k = c; k < c + hMatchLen[r][c]; k++) {
                        if ((r == lastSwapR1 && k == lastSwapC1) || (r == lastSwapR2 && k == lastSwapC2)) sC = k;
                    }
                    newSpecials[r][sC] = TYPE_RAINBOW;
                    c += hMatchLen[r][c] - 1;
                }
            }
        }
        for(int c=0; c<cols; c++) {
            for(int r=0; r<rows; r++) {
                if (vMatchLen[r][c] >= 5 && !newSpecials[r][c]) {
                    int sR = r;
                    for (int k = r; k < r + vMatchLen[r][c]; k++) {
                        if ((k == lastSwapR1 && c == lastSwapC1) || (k == lastSwapR2 && c == lastSwapC2)) sR = k;
                    }
                    newSpecials[sR][c] = TYPE_RAINBOW;
                    r += vMatchLen[r][c] - 1;
                }
            }
        }
        for(int r=0; r<rows; r++) {
            for(int c=0; c<cols; c++) {
                if (hMatchLen[r][c] == 4 && !newSpecials[r][c]) {
                    int sC = c;
                    for (int k = c; k < c + 4; k++) {
                        if ((r == lastSwapR1 && k == lastSwapC1) || (r == lastSwapR2 && k == lastSwapC2)) sC = k;
                    }
                    if (!newSpecials[r][sC]) newSpecials[r][sC] = TYPE_HORIZ;
                    c += 3;
                }
            }
        }
        for(int c=0; c<cols; c++) {
            for(int r=0; r<rows; r++) {
                if (vMatchLen[r][c] == 4 && !newSpecials[r][c]) {
                    int sR = r;
                    for (int k = r; k < r + 4; k++) {
                        if ((k == lastSwapR1 && c == lastSwapC1) || (k == lastSwapR2 && c == lastSwapC2)) sR = k;
                    }
                    if (!newSpecials[sR][c]) newSpecials[sR][c] = TYPE_VERT;
                    r += 3;
                }
            }
        }

        int anyDestroy = 0;
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                if (toDestroy[r][c] || stoneToBreak[r][c]) {
                    anyDestroy = 1;
                }
            }
        }
        if (!anyDestroy) {
            hasMatches = 0;
            break;
        }

        for(int r=0; r<rows; r++) {
            for(int c=0; c<cols; c++) {
                if (newSpecials[r][c]) {
                    toDestroy[r][c] = 0;
                }
            }
        }

        int matchCount = 0;
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                if (toDestroy[r][c]) {
                    popGrid[r][c] = 1;
                    matchCount++;
                    if (grid[r][c] == 0) collectedRed++;
                    else if (grid[r][c] == 1) collectedGreen++;
                    else if (grid[r][c] == 2) collectedBlue++;

                    if (bossHP > 0) {
                        bossHP--;
                        if (typeGrid[r][c] != TYPE_NONE) bossHP -= 4;
                        if (bossHP < 0) bossHP = 0;
                    }

                    COLORREF pc = (typeGrid[r][c] == TYPE_RAINBOW) ? RGB(255,255,255) : colors[grid[r][c]];
                    CreateParticles(BOARD_X + c * cellSize + cellSize / 2, BOARD_Y + r * cellSize + cellSize / 2, pc);
                    if (typeGrid[r][c] != TYPE_NONE) screenShake += 20 + comboMultiplier * 5;
                    else screenShake += 5 + comboMultiplier * 2;
                    if (screenShake > 50) screenShake = 50;
                } else if (stoneToBreak[r][c]) {
                    popGrid[r][c] = 1;
                    score += 20;
                    CreateParticles(BOARD_X + c * cellSize + cellSize / 2, BOARD_Y + r * cellSize + cellSize / 2, RGB(160, 160, 160));
                }
            }
        }
        score += matchCount * 10 * comboMultiplier;
        if (score > statsBestScore) {
            statsBestScore = score;
            SaveStats();
        }
        PlayMatchSound(comboMultiplier);
        
        for (int i = 1; i <= 10; i++) {
            popAnim = i;
            UpdateParticles();
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
            Sleep(15);
        }
        popAnim = 0;

        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                if (popGrid[r][c]) {
                    if (iceGrid[r][c] > 0) iceGrid[r][c] = 0;
                    if (barrierGrid[r][c] > 0) barrierGrid[r][c] = 0;
                    if (stoneToBreak[r][c]) {
                        stoneGrid[r][c]--;
                        if (stoneGrid[r][c] <= 0) stoneGrid[r][c] = 0;
                        stoneToBreak[r][c] = 0;
                    }
                    if (toDestroy[r][c]) {
                        grid[r][c] = -1;
                        typeGrid[r][c] = TYPE_NONE;
                    }
                    popGrid[r][c] = 0;
                }
            }
        }

        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                if (newSpecials[r][c]) {
                    typeGrid[r][c] = newSpecials[r][c];
                }
            }
        }

        comboMultiplier++;
        if (comboMultiplier > statsMaxCombo) {
            statsMaxCombo = comboMultiplier;
            SaveStats();
        }

        for (int c = 0; c < cols; c++) {
            int targetR = rows - 1;
            for (int r = rows - 1; r >= 0; r--) {
                if (stoneGrid[r][c] > 0) {
                    while (targetR > r) {
                        grid[targetR][c] = rand() % 6;
                        typeGrid[targetR][c] = TYPE_NONE;
                        iceGrid[targetR][c] = 0;
                        barrierGrid[targetR][c] = 0;
                        dropCount[targetR][c] = targetR - r;
                        targetR--;
                    }
                    targetR = r - 1;
                    continue;
                }
                if (grid[r][c] != -1) {
                    if (targetR != r) {
                        grid[targetR][c] = grid[r][c];
                        typeGrid[targetR][c] = typeGrid[r][c];
                        iceGrid[targetR][c] = iceGrid[r][c];
                        barrierGrid[targetR][c] = barrierGrid[r][c];
                        grid[r][c] = -1;
                        typeGrid[r][c] = TYPE_NONE;
                        iceGrid[r][c] = 0;
                        barrierGrid[r][c] = 0;
                        dropCount[targetR][c] = targetR - r;
                    } else {
                        dropCount[targetR][c] = 0;
                    }
                    targetR--;
                }
            }
            for (int r = targetR; r >= 0; r--) {
                if (stoneGrid[r][c] == 0) {
                    grid[r][c] = rand() % 6;
                    typeGrid[r][c] = TYPE_NONE;
                    iceGrid[r][c] = 0;
                    barrierGrid[r][c] = 0;
                    dropCount[r][c] = targetR + 1;
                }
            }
        }

        for (int i = 1; i <= 10; i++) {
            dropAnim = i;
            UpdateParticles();
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
            Sleep(15);
        }
        dropAnim = 0;
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                dropCount[r][c] = 0;
            }
        }
        Sleep(50);
        lastSwapR1 = -1; 
    }
}

void SaveGame() {
    FILE *f = fopen("kmatch3_save.dat", "wb");
    if (!f) return;
    fwrite(&level, sizeof(int), 1, f);
    fwrite(&score, sizeof(int), 1, f);
    fwrite(&moves, sizeof(int), 1, f);
    fwrite(&targetScore, sizeof(int), 1, f);
    fwrite(&gameMode, sizeof(int), 1, f);
    fwrite(grid, sizeof(int), MAX_ROWS * MAX_COLS, f);
    fwrite(typeGrid, sizeof(int), MAX_ROWS * MAX_COLS, f);
    fwrite(iceGrid, sizeof(int), MAX_ROWS * MAX_COLS, f);
    fwrite(stoneGrid, sizeof(int), MAX_ROWS * MAX_COLS, f);
    fwrite(barrierGrid, sizeof(int), MAX_ROWS * MAX_COLS, f);
    fwrite(&bossHP, sizeof(int), 1, f);
    fclose(f);
}

int LoadGame() {
    FILE *f = fopen("kmatch3_save.dat", "rb");
    if (!f) return 0;
    fread(&level, sizeof(int), 1, f);
    fread(&score, sizeof(int), 1, f);
    fread(&moves, sizeof(int), 1, f);
    fread(&targetScore, sizeof(int), 1, f);
    if (fread(&gameMode, sizeof(int), 1, f) != 1) gameMode = 0;
    fread(grid, sizeof(int), MAX_ROWS * MAX_COLS, f);
    fread(typeGrid, sizeof(int), MAX_ROWS * MAX_COLS, f);
    fread(iceGrid, sizeof(int), MAX_ROWS * MAX_COLS, f);
    fread(stoneGrid, sizeof(int), MAX_ROWS * MAX_COLS, f);
    fread(barrierGrid, sizeof(int), MAX_ROWS * MAX_COLS, f);
    fread(&bossHP, sizeof(int), 1, f);
    fclose(f);

    if (gameMode == 0 && level >= 1 && level <= 20) {
        rows = CAMPAIGN_STAGES[level - 1].rows;
        cols = CAMPAIGN_STAGES[level - 1].cols;
        maxBossHP = CAMPAIGN_STAGES[level - 1].bossHP;
    } else {
        rows = 8; cols = 8;
        maxBossHP = 0;
    }
    cellSize = BOARD_SIZE / cols;
    return 1;
}

void GameOver(HWND hwnd) {
    statsGamesPlayed++;
    SaveStats();
    MessageBox(hwnd, "Game Over! Stage failed.", "KMatch3", MB_OK | MB_ICONWARNING);
    InitStage(level);
    InvalidateRect(hwnd, NULL, FALSE);
}

void CheckLevelProgress(HWND hwnd) {
    if (gameMode == 1) {
        if (score >= level * 2000) level++;
        SaveGame();
        return;
    }
    
    const StageConfig *cfg = &CAMPAIGN_STAGES[level - 1];
    int cleared = 0;

    if (gameMode == 0) {
        if (level == 20) {
            cleared = (bossHP <= 0);
        } else {
            int scoreMet = (score >= targetScore);
            int redMet = (cfg->redTarget <= 0 || collectedRed >= cfg->redTarget);
            int grnMet = (cfg->greenTarget <= 0 || collectedGreen >= cfg->greenTarget);
            int bluMet = (cfg->blueTarget <= 0 || collectedBlue >= cfg->blueTarget);
            cleared = (scoreMet && redMet && grnMet && bluMet);
        }
    } else if (gameMode == 2) {
        cleared = (score >= targetScore);
    }

    if (cleared) {
        PlayPowerupSound();
        if (gameMode == 0) {
            if (level < 20) {
                MessageBox(hwnd, "Stage Cleared! Advancing to next stage.", "KMatch3", MB_OK | MB_ICONINFORMATION);
                level++;
                InitStage(level);
            } else {
                MessageBox(hwnd, "VICTORY!\nYou have defeated the Jewel King and completed all 20 stages of KMatch3!", "Jewel King Defeated!", MB_OK | MB_ICONINFORMATION);
                level = 1;
                InitStage(1);
            }
        } else if (gameMode == 2) {
            level++;
            moves += 30;
            targetScore += 1000 + (level * 500);
            MessageBox(hwnd, "Level Up!", "KMatch3", MB_OK | MB_ICONINFORMATION);
        }
        SaveGame();
        InvalidateRect(hwnd, NULL, FALSE);
    } else if (((gameMode == 0) || (gameMode == 2)) && moves <= 0) {
        GameOver(hwnd);
    }
    SaveGame();
}

void UseExtraMoves() {
    if (score >= 300) {
        score -= 300;
        PlayPowerupSound();
        int isTimed = (gameMode == 0 && CAMPAIGN_STAGES[level-1].timeLimit > 0) || (gameMode == 2);
        moves += isTimed ? 15 : 5;
    }
}

void ShuffleBoard() {
    do {
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                if (stoneGrid[r][c] == 0 && iceGrid[r][c] == 0) {
                    int color;
                    do {
                        color = rand() % 6;
                    } while (
                        (r >= 2 && grid[r-1][c] == color && grid[r-2][c] == color) ||
                        (c >= 2 && grid[r][c-1] == color && grid[r][c-2] == color)
                    );
                    grid[r][c] = color;
                }
            }
        }
    } while (!CheckMatchPossible());
}

void UseShuffle() {
    if (score >= 300) {
        score -= 300;
        PlayPowerupSound();
        ShuffleBoard();
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            srand((unsigned)time(NULL));
            LoadStats();
            if (!LoadGame()) {
                InitStage(1);
                SaveGame();
            }
            SetTimer(hwnd, 1, 30, NULL);
            SetTimer(hwnd, 2, 1000, NULL);
            break;
        case WM_TIMER: {
            if (wParam == 2) {
                int isTimed = (gameMode == 0 && CAMPAIGN_STAGES[level-1].timeLimit > 0) || (gameMode == 2);
                if (isTimed && moves > 0 && !isProcessing) {
                    moves--;
                    InvalidateRect(hwnd, NULL, FALSE);
                    if (moves <= 0) GameOver(hwnd);
                }
                if (!isProcessing) {
                    hintTimer++;
                    if (hintTimer >= 4 && hintR1 == -1) {
                        FindHint();
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                }
                break;
            }
            int needsUpdate = 0;
            if (screenShake > 0) {
                screenShake = (int)(screenShake * 0.9f) - 1;
                if (screenShake < 0) screenShake = 0;
                needsUpdate = 1;
            }
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (particles[i].life > 0) {
                    needsUpdate = 1;
                    break;
                }
            }
            if (needsUpdate) {
                UpdateParticles();
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rect;
            GetClientRect(hwnd, &rect);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
            
            // Glowing backdrop gradient
            for (int i = 0; i < rect.bottom; i += 4) {
                int r = 38 - (i * 28 / rect.bottom);
                int g = 31 - (i * 23 / rect.bottom);
                int b = 48 - (i * 35 / rect.bottom);
                if (r < 0) r = 0; if (g < 0) g = 0; if (b < 0) b = 0;
                RECT rowR = {0, i, rect.right, i + 4};
                HBRUSH rowB = CreateSolidBrush(RGB(r, g, b));
                FillRect(memDC, &rowR, rowB);
                DeleteObject(rowB);
            }
            // Ancient ruins background
            HBRUSH ruinsBrush = CreateSolidBrush(RGB(25, 20, 35));
            RECT pillar1 = { 20, rect.bottom - 180, 60, rect.bottom };
            FillRect(memDC, &pillar1, ruinsBrush);
            RECT pillar2 = { rect.right - 60, rect.bottom - 120, rect.right - 20, rect.bottom };
            FillRect(memDC, &pillar2, ruinsBrush);
            RECT arch = { 20, rect.bottom - 180, 100, rect.bottom - 150 };
            FillRect(memDC, &arch, ruinsBrush);
            DeleteObject(ruinsBrush);
            
            // Atmospheric magical dust
            for (int i = 0; i < 150; i++) {
                int sx = (i * 137) % rect.right;
                int sy = (i * 251) % rect.bottom;
                int c = 100 + ((i * 73) % 155);
                SetPixel(memDC, sx, sy, RGB(c, c, 255));
                if (i % 3 == 0) SetPixel(memDC, sx+1, sy, RGB(c, c, 255));
            }
            
            DrawBoard(memDC);
            
            int shakeX = (screenShake > 0) ? ((rand() % (screenShake*2 + 1)) - screenShake) : 0;
            int shakeY = (screenShake > 0) ? ((rand() % (screenShake*2 + 1)) - screenShake) : 0;
            BitBlt(hdc, shakeX, shakeY, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);
            
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_KEYDOWN: {
            if (wParam == '1' || wParam == VK_NUMPAD1) {
                gameMode = 0; InitStage(1); SaveGame(); InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == '2' || wParam == VK_NUMPAD2) {
                gameMode = 1; InitStage(1); SaveGame(); InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == '3' || wParam == VK_NUMPAD3) {
                gameMode = 2; InitStage(1); SaveGame(); InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'S' || wParam == 's') {
                UseShuffle(); SaveGame(); InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'E' || wParam == 'e' || wParam == 'M' || wParam == 'm') {
                UseExtraMoves(); SaveGame(); InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'H' || wParam == 'h') {
                if (score >= 300) {
                    powerupMode = 1; PlayPowerupSound(); InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == 'L' || wParam == 'l') {
                if (score >= 300) {
                    powerupMode = 2; PlayPowerupSound(); InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == VK_F1) {
                MessageBox(hwnd, "How to Play KMatch3:\nSwap adjacent gems to form lines of 3+.\n\nSpecial Gems:\n- Match 4: Line Blaster (clears row/col).\n- Match 5: Rainbow Gem (clears all of selected color).\n- T/L Shape: 3x3 Bomb Gem.\n- Stone/Iron Tiles: 1-3 hits to shatter!\n- Boss: Stage 20 Jewel King Boss (75 HP, Barrier Gems).\n\nActive Skills (Cost 300):\n- [H] Hammer: Smash any single tile/gem.\n- [E] +Moves/+15s: Add extra moves or timer.\n- [S] Shuffle: Rearrange all board gems.\n- [L] Color Nuke: Nuke all gems of selected color.\n\nModes:\n- [1] Campaign (20 Stages)\n- [2] Zen Mode\n- [3] Timed Rush", "Help / How to Play", MB_OK | MB_ICONINFORMATION);
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            if (isProcessing) break;
            ClearHint();
            InvalidateRect(hwnd, NULL, FALSE);
            int x = LOWORD(lParam) - BOARD_X;
            int y = HIWORD(lParam) - BOARD_Y;
            if (x >= 0 && x < cols * cellSize && y >= 0 && y < rows * cellSize) {
                int c = x / cellSize;
                int r = y / cellSize;
                if (powerupMode == 1) { // Hammer
                    score -= 300;
                    powerupMode = 0;
                    isProcessing = 1;
                    ProcessMatches(hwnd, r, c, 777);
                    CheckLevelProgress(hwnd);
                    SaveGame();
                    isProcessing = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                } else if (powerupMode == 2) { // Color Nuke
                    score -= 300;
                    powerupMode = 0;
                    isProcessing = 1;
                    int targetC = (grid[r][c] >= 0) ? grid[r][c] : (rand() % 6);
                    ProcessMatches(hwnd, r, c, 900 + targetC);
                    CheckLevelProgress(hwnd);
                    SaveGame();
                    isProcessing = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }

                if (stoneGrid[r][c] > 0) break;

                if (selR == -1) {
                    selR = r; selC = c;
                    InvalidateRect(hwnd, NULL, FALSE);
                } else {
                    if (selR == r && selC == c) {
                        selR = -1; selC = -1;
                        InvalidateRect(hwnd, NULL, FALSE);
                    } else if (abs(selR - r) + abs(selC - c) == 1) {
                        if (stoneGrid[selR][selC] > 0 || stoneGrid[r][c] > 0 || iceGrid[selR][selC] || iceGrid[r][c]) {
                            PlayBadSwapSound();
                            selR = -1; selC = -1;
                            InvalidateRect(hwnd, NULL, FALSE);
                            break;
                        }
                        isProcessing = 1;
                        int origR = selR, origC = selC;
                        selR = -1; selC = -1;
                        PlaySwapSound();
                        AnimateSwap(hwnd, origR, origC, r, c);
                        lastSwapR1 = origR; lastSwapC1 = origC;
                        lastSwapR2 = r; lastSwapC2 = c;

                        int temp = grid[origR][origC];
                        grid[origR][origC] = grid[r][c];
                        grid[r][c] = temp;
                        int tempT = typeGrid[origR][origC];
                        typeGrid[origR][origC] = typeGrid[r][c];
                        typeGrid[r][c] = tempT;

                        int triggerR = -1, triggerC = -1, triggerColor = -1;
                        if (typeGrid[origR][origC] == TYPE_RAINBOW && typeGrid[r][c] == TYPE_RAINBOW) {
                            triggerR = origR; triggerC = origC; triggerColor = 999;
                        } else if (typeGrid[origR][origC] == TYPE_RAINBOW && typeGrid[r][c] != TYPE_RAINBOW) {
                            triggerR = origR; triggerC = origC; triggerColor = grid[r][c];
                        } else if (typeGrid[r][c] == TYPE_RAINBOW && typeGrid[origR][origC] != TYPE_RAINBOW) {
                            triggerR = r; triggerC = c; triggerColor = grid[origR][origC];
                        } else if (typeGrid[origR][origC] > 0 && typeGrid[r][c] > 0 && typeGrid[origR][origC] < TYPE_RAINBOW && typeGrid[r][c] < TYPE_RAINBOW) {
                            triggerR = r; triggerC = c; triggerColor = 888;
                        }

                        if (triggerR != -1 || CheckMatchPossible()) {
                            int isTimed = (gameMode == 0 && CAMPAIGN_STAGES[level-1].timeLimit > 0) || (gameMode == 2);
                            if (!isTimed && gameMode == 0) moves--;

                            bossMoveTimer++;
                            if (gameMode == 0 && bossHP > 0 && bossMoveTimer >= 3) {
                                bossMoveTimer = 0;
                                TriggerBossAction(hwnd);
                            }

                            ProcessMatches(hwnd, triggerR, triggerC, triggerColor);
                            CheckLevelProgress(hwnd);
                            SaveGame();
                        } else {
                            PlayBadSwapSound();
                            AnimateSwap(hwnd, origR, origC, r, c);
                            temp = grid[origR][origC];
                            grid[origR][origC] = grid[r][c];
                            grid[r][c] = temp;
                            tempT = typeGrid[origR][origC];
                            typeGrid[origR][origC] = typeGrid[r][c];
                            typeGrid[r][c] = tempT;
                            InvalidateRect(hwnd, NULL, FALSE); UpdateWindow(hwnd);
                        }
                        isProcessing = 0;
                    } else {
                        selR = r; selC = c;
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
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
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));
    wc.lpszClassName = "KMatch3Class";

    if(!RegisterClassEx(&wc)) ExitProcess(0);

    HWND hwnd = CreateWindowEx(
        0, "KMatch3Class", "KMatch3",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 520, 580,
        NULL, NULL, hInstance, NULL
    );

    if(!hwnd) ExitProcess(0);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
