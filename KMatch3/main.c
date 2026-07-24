#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#define ROWS 8
#define COLS 8
#define CELL_SIZE 50
#define BOARD_X 50
#define BOARD_Y 70

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
    const char* name;
} StageConfig;

static const StageConfig CAMPAIGN_STAGES[15] = {
    {  800, 20,  0,  0,  0, "1: Gem Starter" },
    { 1200, 22,  0,  4,  0, "2: Frosty Fields" },
    { 1500, 20,  0,  6,  2, "3: Stony Path" },
    { 1800,  0, 60,  5,  2, "4: Speed Rush" },
    { 2200, 20,  0,  8,  4, "5: Frozen Quarry" },
    { 2600, 18,  0, 10,  4, "6: Ice Castle" },
    { 3000,  0, 55,  8,  6, "7: Clockwork Cavern" },
    { 3500, 22,  0, 12,  6, "8: Stone Fortress" },
    { 4000, 20,  0, 14,  8, "9: Deep Freeze" },
    { 4500,  0, 50, 10,  8, "10: Blizzard Blitz" },
    { 5000, 20,  0, 16, 10, "11: Glacier Ridge" },
    { 5500, 18,  0, 18, 10, "12: Titan's Vault" },
    { 6000,  0, 45, 15, 12, "13: Time Vortex" },
    { 7000, 22,  0, 20, 14, "14: Crystal Mine" },
    { 8500, 25,  0, 24, 16, "15: Grand Master" }
};

int grid[ROWS][COLS];
int typeGrid[ROWS][COLS] = {0};
int iceGrid[ROWS][COLS] = {0};
int stoneGrid[ROWS][COLS] = {0};
int powerupMode = 0; // 0 = none, 1 = hammer
int score = 0;
int moves = 20;
int level = 1;
int gameMode = 0; // 0 = Campaign (15 Stages), 1 = Zen, 2 = Timed Rush
int targetScore = 800;
int selR = -1, selC = -1;
int isProcessing = 0;

int swapAnim = 0;
int swapR1 = -1, swapC1 = -1, swapR2 = -1, swapC2 = -1;
int lastSwapR1 = -1, lastSwapC1 = -1, lastSwapR2 = -1, lastSwapC2 = -1;
int popAnim = 0;
int popGrid[ROWS][COLS] = {0};
int dropAnim = 0;
int dropCount[ROWS][COLS] = {0};

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
    float life, decay;
    COLORREF color;
} Particle;

Particle particles[MAX_PARTICLES] = {0};

void CreateParticles(int cx, int cy, COLORREF color) {
    for (int i = 0; i < 15; i++) {
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (particles[p].life <= 0) {
                particles[p].x = (float)cx;
                particles[p].y = (float)cy;
                float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
                float speed = (float)((rand() % 50) + 20) / 10.0f;
                particles[p].vx = cosf(angle) * speed;
                particles[p].vy = sinf(angle) * speed;
                particles[p].life = 1.0f;
                particles[p].decay = (float)((rand() % 5) + 2) / 100.0f;
                particles[p].color = color;
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
            particles[i].vy += 0.2f;
            particles[i].life -= particles[i].decay;
        }
    }
}

int laserRowTimer[ROWS] = {0};
int laserColTimer[COLS] = {0};

void DrawFacetedGem(HDC hdc, int cx, int cy, int size, int colorIdx, int typeIdx, int isIce) {
    if (typeIdx == TYPE_RAINBOW) {
        HBRUSH bgRing = CreateSolidBrush(RGB(255, 215, 0));
        HPEN goldPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
        HBRUSH oldB = (HBRUSH)SelectObject(hdc, bgRing);
        HPEN oldP = (HPEN)SelectObject(hdc, goldPen);
        Ellipse(hdc, cx - 19, cy - 19, cx + 19, cy + 19);
        
        POINT pts[16];
        for (int i = 0; i < 16; i++) {
            float r = (i % 2 == 0) ? 20.0f : 9.0f;
            float a = (float)i * 3.14159f / 8.0f;
            pts[i].x = cx + (int)(cosf(a) * r);
            pts[i].y = cy + (int)(sinf(a) * r);
        }
        Polygon(hdc, pts, 16);

        HBRUSH coreB = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, coreB);
        Ellipse(hdc, cx - 5, cy - 5, cx + 5, cy + 5);
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
            POINT pOct[8] = {{cx-7, cy-17}, {cx+7, cy-17}, {cx+17, cy-7}, {cx+17, cy+7}, {cx+7, cy+17}, {cx-7, cy+17}, {cx-17, cy+7}, {cx-17, cy-7}};
            Polygon(hdc, pOct, 8);
            POINT pTop[4] = {{cx-7, cy-17}, {cx+7, cy-17}, {cx+11, cy-9}, {cx-11, cy-9}};
            POINT pBot[4] = {{cx-11, cy+9}, {cx+11, cy+9}, {cx+7, cy+17}, {cx-7, cy+17}};
            POINT pLeft[4] = {{cx-17, cy-7}, {cx-11, cy-9}, {cx-11, cy+9}, {cx-17, cy+7}};
            POINT pRight[4] = {{cx+11, cy-9}, {cx+17, cy-7}, {cx+17, cy+7}, {cx+11, cy+9}};
            POINT pCenter[4] = {{cx-11, cy-9}, {cx+11, cy-9}, {cx+11, cy+9}, {cx-11, cy+9}};
            SelectObject(hdc, lightB); Polygon(hdc, pTop, 4); Polygon(hdc, pLeft, 4);
            SelectObject(hdc, darkB); Polygon(hdc, pBot, 4); Polygon(hdc, pRight, 4);
            SelectObject(hdc, mainB); Polygon(hdc, pCenter, 4);
            SelectObject(hdc, whiteB); Ellipse(hdc, cx-5, cy-13, cx-1, cy-9);
        } else if (c == 1) {
            RECT rOuter = { cx-16, cy-14, cx+16, cy+14 };
            RoundRect(hdc, rOuter.left, rOuter.top, rOuter.right, rOuter.bottom, 6, 6);
            POINT pTop[4] = {{cx-16, cy-14}, {cx+16, cy-14}, {cx+10, cy-8}, {cx-10, cy-8}};
            POINT pBot[4] = {{cx-10, cy+8}, {cx+10, cy+8}, {cx+16, cy+14}, {cx-16, cy+14}};
            POINT pLeft[4] = {{cx-16, cy-14}, {cx-10, cy-8}, {cx-10, cy+8}, {cx-16, cy+14}};
            POINT pRight[4] = {{cx+10, cy-8}, {cx+16, cy-14}, {cx+16, cy+14}, {cx+10, cy+8}};
            SelectObject(hdc, lightB); Polygon(hdc, pTop, 4); Polygon(hdc, pLeft, 4);
            SelectObject(hdc, darkB); Polygon(hdc, pBot, 4); Polygon(hdc, pRight, 4);
            RECT rCenter = { cx-10, cy-8, cx+10, cy+8 };
            SelectObject(hdc, mainB); FillRect(hdc, &rCenter, mainB);
            SelectObject(hdc, whiteB); Ellipse(hdc, cx-12, cy-12, cx-7, cy-7);
        } else if (c == 2) {
            RECT rOuter = { cx-15, cy-15, cx+15, cy+15 };
            RoundRect(hdc, rOuter.left, rOuter.top, rOuter.right, rOuter.bottom, 8, 8);
            POINT pStar1[3] = {{cx, cy-15}, {cx+7, cy}, {cx-7, cy}};
            POINT pStar2[3] = {{cx, cy+15}, {cx+7, cy}, {cx-7, cy}};
            POINT pStar3[3] = {{cx-15, cy}, {cx, cy-7}, {cx, cy+7}};
            POINT pStar4[3] = {{cx+15, cy}, {cx, cy-7}, {cx, cy+7}};
            SelectObject(hdc, lightB); Polygon(hdc, pStar1, 3); Polygon(hdc, pStar3, 3);
            SelectObject(hdc, darkB); Polygon(hdc, pStar2, 3); Polygon(hdc, pStar4, 3);
            SelectObject(hdc, whiteB); Ellipse(hdc, cx-7, cy-9, cx-2, cy-4);
        } else if (c == 3) {
            POINT pDiamond[4] = {{cx, cy-18}, {cx+17, cy}, {cx, cy+18}, {cx-17, cy}};
            Polygon(hdc, pDiamond, 4);
            POINT p1[3] = {{cx, cy-18}, {cx+17, cy}, {cx, cy}};
            POINT p2[3] = {{cx, cy-18}, {cx-17, cy}, {cx, cy}};
            POINT p3[3] = {{cx, cy+18}, {cx+17, cy}, {cx, cy}};
            POINT p4[3] = {{cx, cy+18}, {cx-17, cy}, {cx, cy}};
            SelectObject(hdc, lightB); Polygon(hdc, p1, 3);
            SelectObject(hdc, whiteB); Polygon(hdc, p2, 3);
            SelectObject(hdc, darkB); Polygon(hdc, p3, 3); Polygon(hdc, p4, 3);
        } else if (c == 4) {
            POINT pHex[6] = {{cx, cy-17}, {cx+15, cy-8}, {cx+13, cy+13}, {cx, cy+17}, {cx-13, cy+13}, {cx-15, cy-8}};
            Polygon(hdc, pHex, 6);
            POINT p1[3] = {{cx, cy-17}, {cx+15, cy-8}, {cx, cy}};
            POINT p2[3] = {{cx, cy-17}, {cx-15, cy-8}, {cx, cy}};
            POINT p3[3] = {{cx-15, cy-8}, {cx-13, cy+13}, {cx, cy}};
            POINT p4[3] = {{cx+15, cy-8}, {cx+13, cy+13}, {cx, cy}};
            SelectObject(hdc, lightB); Polygon(hdc, p1, 3);
            SelectObject(hdc, whiteB); Polygon(hdc, p2, 3);
            SelectObject(hdc, darkB); Polygon(hdc, p3, 3); Polygon(hdc, p4, 3);
        } else {
            Ellipse(hdc, cx-16, cy-16, cx+16, cy+16);
            POINT pStar1[4] = {{cx, cy-16}, {cx+6, cy-6}, {cx, cy}, {cx-6, cy-6}};
            POINT pStar2[4] = {{cx+16, cy}, {cx+6, cy+6}, {cx, cy}, {cx+6, cy-6}};
            POINT pStar3[4] = {{cx, cy+16}, {cx-6, cy+6}, {cx, cy}, {cx+6, cy+6}};
            POINT pStar4[4] = {{cx-17, cy}, {cx-6, cy-6}, {cx, cy}, {cx-6, cy+6}};
            SelectObject(hdc, lightB); Polygon(hdc, pStar1, 4); Polygon(hdc, pStar4, 4);
            SelectObject(hdc, darkB); Polygon(hdc, pStar2, 4); Polygon(hdc, pStar3, 4);
            SelectObject(hdc, whiteB); Ellipse(hdc, cx-3, cy-3, cx+3, cy+3);
        }

        SelectObject(hdc, oldB); SelectObject(hdc, oldP);
        DeleteObject(mainB); DeleteObject(lightB); DeleteObject(darkB); DeleteObject(whiteB); DeleteObject(penDark);

        if (typeIdx == TYPE_HORIZ) {
            HPEN goldP = CreatePen(PS_SOLID, 2, RGB(255, 215, 0));
            HBRUSH goldB = CreateSolidBrush(RGB(255, 170, 0));
            HPEN oP = (HPEN)SelectObject(hdc, goldP);
            HBRUSH oB = (HBRUSH)SelectObject(hdc, goldB);
            Ellipse(hdc, cx-19, cy-19, cx+19, cy+19);
            POINT arrL[3] = {{cx-16, cy}, {cx-7, cy-5}, {cx-7, cy+5}};
            POINT arrR[3] = {{cx+16, cy}, {cx+7, cy-5}, {cx+7, cy+5}};
            SelectObject(hdc, GetStockObject(WHITE_BRUSH));
            Polygon(hdc, arrL, 3); Polygon(hdc, arrR, 3);
            RECT lineR = {cx-7, cy-2, cx+7, cy+2};
            FillRect(hdc, &lineR, (HBRUSH)GetStockObject(WHITE_BRUSH));
            SelectObject(hdc, oP); SelectObject(hdc, oB);
            DeleteObject(goldP); DeleteObject(goldB);
        } else if (typeIdx == TYPE_VERT) {
            HPEN cyanP = CreatePen(PS_SOLID, 2, RGB(0, 229, 255));
            HBRUSH cyanB = CreateSolidBrush(RGB(0, 136, 255));
            HPEN oP = (HPEN)SelectObject(hdc, cyanP);
            HBRUSH oB = (HBRUSH)SelectObject(hdc, cyanB);
            Ellipse(hdc, cx-19, cy-19, cx+19, cy+19);
            POINT arrU[3] = {{cx, cy-16}, {cx-5, cy-7}, {cx+5, cy-7}};
            POINT arrD[3] = {{cx, cy+16}, {cx-5, cy+7}, {cx+5, cy+7}};
            SelectObject(hdc, GetStockObject(WHITE_BRUSH));
            Polygon(hdc, arrU, 3); Polygon(hdc, arrD, 3);
            RECT lineR = {cx-2, cy-7, cx+2, cy+7};
            FillRect(hdc, &lineR, (HBRUSH)GetStockObject(WHITE_BRUSH));
            SelectObject(hdc, oP); SelectObject(hdc, oB);
            DeleteObject(cyanP); DeleteObject(cyanB);
        } else if (typeIdx == TYPE_BOMB) {
            HBRUSH darkBomb = CreateSolidBrush(RGB(15, 15, 15));
            HPEN redP = CreatePen(PS_SOLID, 2, RGB(255, 40, 40));
            HPEN oP = (HPEN)SelectObject(hdc, redP);
            HBRUSH oB = (HBRUSH)SelectObject(hdc, darkBomb);
            Ellipse(hdc, cx-16, cy-16, cx+16, cy+16);
            HBRUSH redCore = CreateSolidBrush(RGB(255, 40, 40));
            SelectObject(hdc, redCore);
            Ellipse(hdc, cx-6, cy-6, cx+6, cy+6);
            DeleteObject(redCore);
            HBRUSH sparkB = CreateSolidBrush(RGB(255, 200, 0));
            SelectObject(hdc, sparkB);
            Ellipse(hdc, cx+8, cy-17, cx+14, cy-11);
            DeleteObject(sparkB);
            SelectObject(hdc, oP); SelectObject(hdc, oB);
            DeleteObject(darkBomb); DeleteObject(redP);
        }
    }

    if (isIce) {
        HBRUSH iceBrush = CreateSolidBrush(RGB(0, 229, 255));
        RECT iRect = { cx-19, cy-19, cx+19, cy+19 };
        FrameRect(hdc, &iRect, iceBrush);
        DeleteObject(iceBrush);
        HPEN iceP = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        HPEN oP = (HPEN)SelectObject(hdc, iceP);
        MoveToEx(hdc, cx-14, cy-9, NULL); LineTo(hdc, cx+4, cy+9);
        MoveToEx(hdc, cx+9, cy-14, NULL); LineTo(hdc, cx-9, cy+14);
        SelectObject(hdc, oP); DeleteObject(iceP);
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
        if (stageIdx > 15) stageIdx = 15;
        level = stageIdx;
        targetScore = CAMPAIGN_STAGES[level - 1].targetScore;
        if (CAMPAIGN_STAGES[level - 1].timeLimit > 0) {
            moves = CAMPAIGN_STAGES[level - 1].timeLimit;
        } else {
            moves = CAMPAIGN_STAGES[level - 1].moves;
        }
    } else if (gameMode == 1) {
        level = 1;
        targetScore = 2000;
        moves = 0;
    } else if (gameMode == 2) {
        level = 1;
        targetScore = 1000;
        moves = 60;
    }

    selR = -1; selC = -1;
    memset(typeGrid, 0, sizeof(typeGrid));
    memset(iceGrid, 0, sizeof(iceGrid));
    memset(stoneGrid, 0, sizeof(stoneGrid));

    int iceToPlace = (gameMode == 0) ? CAMPAIGN_STAGES[level - 1].iceCount : (gameMode == 1 ? 0 : 4);
    int stoneToPlace = (gameMode == 0) ? CAMPAIGN_STAGES[level - 1].stoneCount : 0;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
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

    while (stoneToPlace > 0) {
        int r = rand() % ROWS;
        int c = rand() % COLS;
        if (!stoneGrid[r][c]) {
            stoneGrid[r][c] = 1;
            grid[r][c] = -1;
            stoneToPlace--;
        }
    }

    while (iceToPlace > 0) {
        int r = rand() % ROWS;
        int c = rand() % COLS;
        if (!stoneGrid[r][c] && !iceGrid[r][c]) {
            iceGrid[r][c] = 1;
            iceToPlace--;
        }
    }
}

void DrawBoard(HDC hdc) {
    char buf[128];
    int isTimedStage = (gameMode == 0 && CAMPAIGN_STAGES[level-1].timeLimit > 0) || (gameMode == 2);
    
    if (gameMode == 0) {
        sprintf(buf, "Stage %s | Score: %d/%d | %s: %d", CAMPAIGN_STAGES[level-1].name, score, targetScore, isTimedStage ? "Time" : "Moves", moves);
    } else if (gameMode == 1) {
        sprintf(buf, "Zen Mode | Score: %d | Level %d", score, level);
    } else if (gameMode == 2) {
        sprintf(buf, "Timed Rush | Score: %d/%d | Time: %ds", score, targetScore, moves);
    }

    SetTextColor(hdc, RGB(255, 215, 0));
    SetBkMode(hdc, TRANSPARENT);
    TextOut(hdc, BOARD_X, 8, buf, strlen(buf));
    
    int iceCount = 0, stoneCount = 0;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (iceGrid[r][c]) iceCount++;
            if (stoneGrid[r][c]) stoneCount++;
        }
    }

    char statsBuf[128];
    sprintf(statsBuf, "Ice: %d | Stone: %d | Best: %d | Combo: x%d", iceCount, stoneCount, statsBestScore, statsMaxCombo);
    SetTextColor(hdc, RGB(200, 200, 200));
    TextOut(hdc, BOARD_X, 28, statsBuf, strlen(statsBuf));

    char powerupBuf[128];
    sprintf(powerupBuf, "[H] Hammer(500) [M] +Moves/+15s(500) [S] Shuffle(500) [1-3] Mode", powerupMode ? " (ACTIVE)" : "");
    SetTextColor(hdc, powerupMode ? RGB(255, 100, 100) : RGB(255, 215, 0));
    TextOut(hdc, BOARD_X, 46, powerupBuf, strlen(powerupBuf));

    // 3D Golden/Stone Board Outer Frame
    RECT outerFrame = { BOARD_X - 8, BOARD_Y - 8, BOARD_X + COLS * CELL_SIZE + 8, BOARD_Y + ROWS * CELL_SIZE + 8 };
    HBRUSH frameBrush = CreateSolidBrush(RGB(160, 115, 15));
    FillRect(hdc, &outerFrame, frameBrush);
    DeleteObject(frameBrush);

    HPEN framePenHigh = CreatePen(PS_SOLID, 2, RGB(240, 190, 40));
    HPEN framePenLow = CreatePen(PS_SOLID, 2, RGB(70, 50, 5));
    HPEN oldPen = (HPEN)SelectObject(hdc, framePenHigh);

    MoveToEx(hdc, outerFrame.left, outerFrame.bottom, NULL);
    LineTo(hdc, outerFrame.left, outerFrame.top);
    LineTo(hdc, outerFrame.right, outerFrame.top);
    SelectObject(hdc, framePenLow);
    LineTo(hdc, outerFrame.right, outerFrame.bottom);
    LineTo(hdc, outerFrame.left, outerFrame.bottom);

    // Corner Studs
    HBRUSH studB = CreateSolidBrush(RGB(255, 215, 0));
    SelectObject(hdc, studB);
    Ellipse(hdc, outerFrame.left+2, outerFrame.top+2, outerFrame.left+7, outerFrame.top+7);
    Ellipse(hdc, outerFrame.right-7, outerFrame.top+2, outerFrame.right-2, outerFrame.top+7);
    Ellipse(hdc, outerFrame.left+2, outerFrame.bottom-7, outerFrame.left+7, outerFrame.bottom-2);
    Ellipse(hdc, outerFrame.right-7, outerFrame.bottom-7, outerFrame.right-2, outerFrame.bottom-2);
    SelectObject(hdc, oldPen);
    DeleteObject(studB); DeleteObject(framePenHigh); DeleteObject(framePenLow);

    // Grid Cell Sockets
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            RECT rect = { BOARD_X + c * CELL_SIZE, BOARD_Y + r * CELL_SIZE, 
                          BOARD_X + (c + 1) * CELL_SIZE, BOARD_Y + (r + 1) * CELL_SIZE };
            HBRUSH bg = CreateSolidBrush(RGB(22, 16, 28));
            FillRect(hdc, &rect, bg);
            DeleteObject(bg);
            HBRUSH border = CreateSolidBrush(RGB(40, 32, 50));
            FrameRect(hdc, &rect, border);
            DeleteObject(border);
        }
    }

    HRGN hRgn = CreateRectRgn(BOARD_X, BOARD_Y, BOARD_X + COLS * CELL_SIZE, BOARD_Y + ROWS * CELL_SIZE);
    SelectClipRgn(hdc, hRgn);

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int drawX = BOARD_X + c * CELL_SIZE;
            int drawY = BOARD_Y + r * CELL_SIZE;

            if (r == swapR1 && c == swapC1) {
                drawX += (swapC2 - swapC1) * CELL_SIZE * swapAnim / 10;
                drawY += (swapR2 - swapR1) * CELL_SIZE * swapAnim / 10;
            } else if (r == swapR2 && c == swapC2) {
                drawX += (swapC1 - swapC2) * CELL_SIZE * swapAnim / 10;
                drawY += (swapR1 - swapR2) * CELL_SIZE * swapAnim / 10;
            }

            if (dropAnim > 0 && dropCount[r][c] > 0) {
                int startY = drawY - (dropCount[r][c] * CELL_SIZE);
                drawY = startY + (drawY - startY) * dropAnim / 10;
            }

            RECT rect = { drawX, drawY, drawX + CELL_SIZE, drawY + CELL_SIZE };

            if (popGrid[r][c] && popAnim > 0) {
                int shrink = (CELL_SIZE / 2) * popAnim / 10;
                rect.left += shrink;
                rect.top += shrink;
                rect.right -= shrink;
                rect.bottom -= shrink;
                if (rect.right <= rect.left) continue;
            }

            if (stoneGrid[r][c]) {
                HBRUSH stoneBrush = CreateSolidBrush(RGB(80, 80, 85));
                FillRect(hdc, &rect, stoneBrush);
                DeleteObject(stoneBrush);

                HBRUSH stoneBorder = CreateSolidBrush(RGB(50, 50, 55));
                FrameRect(hdc, &rect, stoneBorder);
                DeleteObject(stoneBorder);

                HPEN pen = CreatePen(PS_SOLID, 2, RGB(255, 215, 0));
                HPEN oldPen = (HPEN)SelectObject(hdc, pen);
                int cx = rect.left + CELL_SIZE/2, cy = rect.top + CELL_SIZE/2;
                MoveToEx(hdc, cx - 10, cy, NULL); LineTo(hdc, cx + 10, cy);
                MoveToEx(hdc, cx, cy - 10, NULL); LineTo(hdc, cx, cy + 10);
                SelectObject(hdc, oldPen); DeleteObject(pen);
                continue;
            }

            if (grid[r][c] == -1) continue;

            int cx = drawX + CELL_SIZE / 2;
            int cy = drawY + CELL_SIZE / 2;
            DrawFacetedGem(hdc, cx, cy, CELL_SIZE, grid[r][c], typeGrid[r][c], iceGrid[r][c]);

            if (r == selR && c == selC) {
                HBRUSH border = CreateSolidBrush((powerupMode == 1) ? RGB(255, 0, 0) : RGB(255, 215, 0));
                FrameRect(hdc, &rect, border);
                DeleteObject(border);
                
                rect.left += 1; rect.top += 1; rect.right -= 1; rect.bottom -= 1;
                border = CreateSolidBrush(RGB(255, 255, 255));
                FrameRect(hdc, &rect, border);
                DeleteObject(border);
            }
        }
    }

    // Laser Beam Overlays
    for (int r = 0; r < ROWS; r++) {
        if (laserRowTimer[r] > 0) {
            RECT lRect = { BOARD_X, BOARD_Y + r * CELL_SIZE + 18, BOARD_X + COLS * CELL_SIZE, BOARD_Y + r * CELL_SIZE + 32 };
            HBRUSH lB = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(hdc, &lRect, lB);
            DeleteObject(lB);
            laserRowTimer[r]--;
        }
    }
    for (int c = 0; c < COLS; c++) {
        if (laserColTimer[c] > 0) {
            RECT lRect = { BOARD_X + c * CELL_SIZE + 18, BOARD_Y, BOARD_X + c * CELL_SIZE + 32, BOARD_Y + ROWS * CELL_SIZE };
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
            RECT pr = { (int)particles[i].x - 2, (int)particles[i].y - 2, (int)particles[i].x + 3, (int)particles[i].y + 3 };
            HBRUSH pb = CreateSolidBrush(particles[i].color);
            FillRect(hdc, &pr, pb);
            DeleteObject(pb);
        }
    }
}

int toDestroy[ROWS][COLS] = {0};
int stoneToBreak[ROWS][COLS] = {0};

void AddDestroy(int r, int c) {
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return;
    if (stoneGrid[r][c]) {
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
        if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS && stoneGrid[nr][nc]) {
            stoneToBreak[nr][nc] = 1;
        }
    }

    if (typeGrid[r][c] == TYPE_HORIZ) {
        laserRowTimer[r] = 10;
        for (int i = 0; i < COLS; i++) AddDestroy(r, i);
    } else if (typeGrid[r][c] == TYPE_VERT) {
        laserColTimer[c] = 10;
        for (int i = 0; i < ROWS; i++) AddDestroy(i, c);
    } else if (typeGrid[r][c] == TYPE_RAINBOW) {
        int tcolor = rand() % 6;
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                if (grid[i][j] == tcolor && typeGrid[i][j] != TYPE_RAINBOW && !stoneGrid[i][j]) AddDestroy(i, j);
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

int CheckMatchPossible() {
    for (int r=0; r<ROWS; r++) {
        for (int c=0; c<COLS-2; c++) {
            int color = grid[r][c];
            if (color != -1 && !stoneGrid[r][c] && typeGrid[r][c] != TYPE_RAINBOW &&
                grid[r][c+1] == color && !stoneGrid[r][c+1] && typeGrid[r][c+1] != TYPE_RAINBOW &&
                grid[r][c+2] == color && !stoneGrid[r][c+2] && typeGrid[r][c+2] != TYPE_RAINBOW) return 1;
        }
    }
    for (int c=0; c<COLS; c++) {
        for (int r=0; r<ROWS-2; r++) {
            int color = grid[r][c];
            if (color != -1 && !stoneGrid[r][c] && typeGrid[r][c] != TYPE_RAINBOW &&
                grid[r+1][c] == color && !stoneGrid[r+1][c] && typeGrid[r+1][c] != TYPE_RAINBOW &&
                grid[r+2][c] == color && !stoneGrid[r+2][c] && typeGrid[r+2][c] != TYPE_RAINBOW) return 1;
        }
    }
    return 0;
}

void AnimateSwap(HWND hwnd, int r1, int c1, int r2, int c2) {
    swapR1 = r1; swapC1 = c1; swapR2 = r2; swapC2 = c2;
    for (int i = 1; i <= 10; i++) {
        swapAnim = i;
        UpdateParticles();
        InvalidateRect(hwnd, NULL, FALSE);
        UpdateWindow(hwnd);
        Sleep(15);
    }
    swapR1 = -1;
}

void ProcessMatches(HWND hwnd, int triggerR, int triggerC, int triggerColor) {
    int comboMultiplier = 1;
    int hasMatches = 1;
    
    while (hasMatches) {
        memset(toDestroy, 0, sizeof(toDestroy));
        memset(stoneToBreak, 0, sizeof(stoneToBreak));
        int newSpecials[ROWS][COLS] = {0}; 

        if (triggerR != -1) {
            if (stoneGrid[triggerR][triggerC]) {
                stoneToBreak[triggerR][triggerC] = 1;
            } else {
                toDestroy[triggerR][triggerC] = 1;
            }

            if (triggerColor == 999) { // Rainbow + Rainbow
                for(int i=0; i<ROWS; i++) for(int j=0; j<COLS; j++) if(!stoneGrid[i][j]) AddDestroy(i, j);
            } else if (triggerColor == 888) { // Line + Line
                typeGrid[triggerR][triggerC] = TYPE_HORIZ; AddDestroy(triggerR, triggerC);
                typeGrid[lastSwapR1][lastSwapC1] = TYPE_VERT; AddDestroy(lastSwapR1, lastSwapC1);
            } else if (triggerColor == 777) { // Hammer
                AddDestroy(triggerR, triggerC);
            } else if (triggerColor >= 0 && triggerColor < 6) { // Rainbow + Color
                for(int i=0; i<ROWS; i++) for(int j=0; j<COLS; j++) {
                    if (grid[i][j] == triggerColor && !stoneGrid[i][j]) AddDestroy(i, j);
                }
            }
            triggerR = -1; 
        }

        int hMatchLen[ROWS][COLS] = {0};
        int vMatchLen[ROWS][COLS] = {0};
        
        for (int r=0; r<ROWS; r++) {
            for (int c=0; c<COLS-2; c++) {
                int color = grid[r][c];
                if (color != -1 && !stoneGrid[r][c] && typeGrid[r][c] != TYPE_RAINBOW && 
                    grid[r][c+1] == color && !stoneGrid[r][c+1] && typeGrid[r][c+1] != TYPE_RAINBOW && 
                    grid[r][c+2] == color && !stoneGrid[r][c+2] && typeGrid[r][c+2] != TYPE_RAINBOW) {
                    int k = c;
                    while(k < COLS && grid[r][k] == color && !stoneGrid[r][k] && typeGrid[r][k] != TYPE_RAINBOW) k++;
                    int len = k - c;
                    for(int i = c; i < k; i++) hMatchLen[r][i] = len;
                    c = k - 1;
                }
            }
        }
        for (int c=0; c<COLS; c++) {
            for (int r=0; r<ROWS-2; r++) {
                int color = grid[r][c];
                if (color != -1 && !stoneGrid[r][c] && typeGrid[r][c] != TYPE_RAINBOW && 
                    grid[r+1][c] == color && !stoneGrid[r+1][c] && typeGrid[r+1][c] != TYPE_RAINBOW && 
                    grid[r+2][c] == color && !stoneGrid[r+2][c] && typeGrid[r+2][c] != TYPE_RAINBOW) {
                    int k = r;
                    while(k < ROWS && grid[k][c] == color && !stoneGrid[k][c] && typeGrid[k][c] != TYPE_RAINBOW) k++;
                    int len = k - r;
                    for(int i = r; i < k; i++) vMatchLen[i][c] = len;
                    r = k - 1;
                }
            }
        }
        
        for(int r=0; r<ROWS; r++) {
            for(int c=0; c<COLS; c++) {
                if (hMatchLen[r][c] >= 3 || vMatchLen[r][c] >= 3) {
                    AddDestroy(r, c);
                }
            }
        }

        for(int r=0; r<ROWS; r++) {
            for(int c=0; c<COLS; c++) {
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
        for(int c=0; c<COLS; c++) {
            for(int r=0; r<ROWS; r++) {
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
        for(int r=0; r<ROWS; r++) {
            for(int c=0; c<COLS; c++) {
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
        for(int c=0; c<COLS; c++) {
            for(int r=0; r<ROWS; r++) {
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
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                if (toDestroy[r][c] || stoneToBreak[r][c]) {
                    anyDestroy = 1;
                }
            }
        }
        if (!anyDestroy) {
            hasMatches = 0;
            break;
        }

        for(int r=0; r<ROWS; r++) {
            for(int c=0; c<COLS; c++) {
                if (newSpecials[r][c]) {
                    toDestroy[r][c] = 0;
                }
            }
        }

        int matchCount = 0;
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                if (toDestroy[r][c]) {
                    popGrid[r][c] = 1;
                    matchCount++;
                    COLORREF pc = (typeGrid[r][c] == TYPE_RAINBOW) ? RGB(255,255,255) : colors[grid[r][c]];
                    CreateParticles(BOARD_X + c * CELL_SIZE + CELL_SIZE / 2, BOARD_Y + r * CELL_SIZE + CELL_SIZE / 2, pc);
                } else if (stoneToBreak[r][c]) {
                    popGrid[r][c] = 1;
                    score += 20;
                    CreateParticles(BOARD_X + c * CELL_SIZE + CELL_SIZE / 2, BOARD_Y + r * CELL_SIZE + CELL_SIZE / 2, RGB(160, 160, 160));
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

        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                if (popGrid[r][c]) {
                    if (iceGrid[r][c] > 0) iceGrid[r][c] = 0;
                    if (stoneToBreak[r][c]) {
                        stoneGrid[r][c] = 0;
                        stoneToBreak[r][c] = 0;
                    }
                    grid[r][c] = -1;
                    typeGrid[r][c] = TYPE_NONE;
                    popGrid[r][c] = 0;
                }
            }
        }

        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
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

        for (int c = 0; c < COLS; c++) {
            int targetR = ROWS - 1;
            for (int r = ROWS - 1; r >= 0; r--) {
                if (stoneGrid[r][c]) {
                    targetR = r - 1;
                    continue;
                }
                if (grid[r][c] != -1) {
                    if (targetR != r) {
                        grid[targetR][c] = grid[r][c];
                        typeGrid[targetR][c] = typeGrid[r][c];
                        iceGrid[targetR][c] = iceGrid[r][c];
                        grid[r][c] = -1;
                        typeGrid[r][c] = TYPE_NONE;
                        iceGrid[r][c] = 0;
                        dropCount[targetR][c] = targetR - r;
                    } else {
                        dropCount[targetR][c] = 0;
                    }
                    targetR--;
                }
            }
            for (int r = targetR; r >= 0; r--) {
                if (!stoneGrid[r][c]) {
                    grid[r][c] = rand() % 6;
                    typeGrid[r][c] = TYPE_NONE;
                    iceGrid[r][c] = 0;
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
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
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
    fwrite(grid, sizeof(int), ROWS * COLS, f);
    fwrite(typeGrid, sizeof(int), ROWS * COLS, f);
    fwrite(iceGrid, sizeof(int), ROWS * COLS, f);
    fwrite(stoneGrid, sizeof(int), ROWS * COLS, f);
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
    fread(grid, sizeof(int), ROWS * COLS, f);
    fread(typeGrid, sizeof(int), ROWS * COLS, f);
    if (fread(iceGrid, sizeof(int), ROWS * COLS, f) != ROWS * COLS) memset(iceGrid, 0, sizeof(iceGrid));
    if (fread(stoneGrid, sizeof(int), ROWS * COLS, f) != ROWS * COLS) memset(stoneGrid, 0, sizeof(stoneGrid));
    fclose(f);
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
    if (score >= targetScore) {
        PlayPowerupSound();
        if (gameMode == 0) {
            if (level < 15) {
                MessageBox(hwnd, "Stage Cleared! Advancing to next stage.", "KMatch3", MB_OK | MB_ICONINFORMATION);
                level++;
                InitStage(level);
            } else {
                MessageBox(hwnd, "CONGRATULATIONS!\nYou have completed all 15 stages of KMatch3 Campaign Mode!", "Victory!", MB_OK | MB_ICONINFORMATION);
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
    if (score >= 500) {
        score -= 500;
        PlayPowerupSound();
        int isTimed = (gameMode == 0 && CAMPAIGN_STAGES[level-1].timeLimit > 0) || (gameMode == 2);
        moves += isTimed ? 15 : 5;
    }
}

void UseShuffle() {
    if (score >= 500) {
        score -= 500;
        PlayPowerupSound();
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                if (!stoneGrid[r][c] && !iceGrid[r][c]) {
                    grid[r][c] = rand() % 6;
                }
            }
        }
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
                break;
            }
            int needsUpdate = 0;
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
            
            HBRUSH bgBrush = CreateSolidBrush(RGB(30, 30, 30));
            FillRect(memDC, &rect, bgBrush);
            DeleteObject(bgBrush);
            
            DrawBoard(memDC);
            
            BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);
            
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
            } else if (wParam == 'M' || wParam == 'm') {
                UseExtraMoves(); SaveGame(); InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'H' || wParam == 'h') {
                if (score >= 500) {
                    powerupMode = 1; PlayPowerupSound(); InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == VK_F1) {
                MessageBox(hwnd, "How to Play KMatch3:\nSwap adjacent gems to form lines of 3+.\n\nSpecial Gems:\n- Match 4: Line Bomb (clears row/col).\n- Match 5: Rainbow Gem (clears all of selected color).\n- T/L Shape: 3x3 Bomb Gem.\n- Stone Tiles: Break by adjacent matches or bombs!\n- Ice Tiles: Break by matching internal gem.\n\nPower-ups (Cost 500):\n- [H] Hammer: Smash any single tile/gem.\n- [M] +Moves/+15s: Add extra moves or timer seconds.\n- [S] Shuffle: Rearrange all board gems.\n\nModes:\n- [1] Campaign (15 Stages)\n- [2] Zen Mode\n- [3] Timed Rush", "Help / How to Play", MB_OK | MB_ICONINFORMATION);
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            if (isProcessing) break;
            int x = LOWORD(lParam) - BOARD_X;
            int y = HIWORD(lParam) - BOARD_Y;
            if (x >= 0 && x < COLS * CELL_SIZE && y >= 0 && y < ROWS * CELL_SIZE) {
                int c = x / CELL_SIZE;
                int r = y / CELL_SIZE;
                if (powerupMode == 1) {
                    score -= 500;
                    powerupMode = 0;
                    isProcessing = 1;
                    ProcessMatches(hwnd, r, c, 777);
                    CheckLevelProgress(hwnd);
                    SaveGame();
                    isProcessing = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }
                if (stoneGrid[r][c]) break;

                if (selR == -1) {
                    selR = r; selC = c;
                    InvalidateRect(hwnd, NULL, FALSE);
                } else {
                    if (selR == r && selC == c) {
                        selR = -1; selC = -1;
                        InvalidateRect(hwnd, NULL, FALSE);
                    } else if (abs(selR - r) + abs(selC - c) == 1) {
                        if (stoneGrid[selR][selC] || stoneGrid[r][c] || iceGrid[selR][selC] || iceGrid[r][c]) {
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
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 560,
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

