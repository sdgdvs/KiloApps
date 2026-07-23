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
            particles[i].life -= particles[i].decay;
        }
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

    SetTextColor(hdc, RGB(255, 255, 255));
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

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            RECT rect = { BOARD_X + c * CELL_SIZE, BOARD_Y + r * CELL_SIZE, 
                          BOARD_X + (c + 1) * CELL_SIZE, BOARD_Y + (r + 1) * CELL_SIZE };
            HBRUSH bg = CreateSolidBrush(RGB(45, 45, 45));
            FillRect(hdc, &rect, bg);
            DeleteObject(bg);
            HBRUSH border = CreateSolidBrush(RGB(85, 85, 85));
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
                HBRUSH stoneBrush = CreateSolidBrush(RGB(100, 100, 100));
                FillRect(hdc, &rect, stoneBrush);
                DeleteObject(stoneBrush);

                HBRUSH stoneBorder = CreateSolidBrush(RGB(60, 60, 60));
                FrameRect(hdc, &rect, stoneBorder);
                DeleteObject(stoneBorder);

                HPEN pen = CreatePen(PS_SOLID, 2, RGB(140, 140, 140));
                HPEN oldPen = (HPEN)SelectObject(hdc, pen);
                MoveToEx(hdc, rect.left + 5, rect.top + 5, NULL);
                LineTo(hdc, rect.right - 5, rect.bottom - 5);
                MoveToEx(hdc, rect.right - 5, rect.top + 5, NULL);
                LineTo(hdc, rect.left + 5, rect.bottom - 5);
                SelectObject(hdc, oldPen);
                DeleteObject(pen);
                continue;
            }

            if (grid[r][c] == -1) continue;

            HBRUSH brush = CreateSolidBrush(typeGrid[r][c] == TYPE_RAINBOW ? RGB(255, 255, 255) : colors[grid[r][c]]);
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);

            if (typeGrid[r][c] == TYPE_HORIZ) {
                RECT hRect = { rect.left, rect.top + CELL_SIZE/2 - 4, rect.right, rect.top + CELL_SIZE/2 + 4 };
                HBRUSH b = CreateSolidBrush(RGB(255, 255, 255));
                FillRect(hdc, &hRect, b);
                DeleteObject(b);
            } else if (typeGrid[r][c] == TYPE_VERT) {
                RECT vRect = { rect.left + CELL_SIZE/2 - 4, rect.top, rect.left + CELL_SIZE/2 + 4, rect.bottom };
                HBRUSH b = CreateSolidBrush(RGB(255, 255, 255));
                FillRect(hdc, &vRect, b);
                DeleteObject(b);
            } else if (typeGrid[r][c] == TYPE_RAINBOW) {
                RECT cRect = { rect.left + 12, rect.top + 12, rect.right - 12, rect.bottom - 12 };
                HBRUSH b = CreateSolidBrush(RGB(255, 215, 0));
                FillRect(hdc, &cRect, b);
                DeleteObject(b);
            } else if (typeGrid[r][c] == TYPE_BOMB) {
                HBRUSH b = CreateSolidBrush(RGB(40, 40, 40));
                RECT bRect = { rect.left + 10, rect.top + 10, rect.right - 10, rect.bottom - 10 };
                FillRect(hdc, &bRect, b);
                DeleteObject(b);

                HBRUSH dot = CreateSolidBrush(RGB(255, 50, 50));
                RECT dRect = { rect.left + 18, rect.top + 18, rect.right - 18, rect.bottom - 18 };
                FillRect(hdc, &dRect, dot);
                DeleteObject(dot);
            }

            if (iceGrid[r][c]) {
                RECT iRect = { rect.left + 2, rect.top + 2, rect.right - 2, rect.bottom - 2 };
                HBRUSH b = CreateHatchBrush(HS_BDIAGONAL, RGB(0, 255, 255));
                FillRect(hdc, &iRect, b);
                DeleteObject(b);
            }

            if (r == selR && c == selC) {
                HBRUSH border = CreateSolidBrush((powerupMode == 1) ? RGB(255, 0, 0) : RGB(255, 255, 255));
                FrameRect(hdc, &rect, border);
                DeleteObject(border);
                
                rect.left += 1; rect.top += 1; rect.right -= 1; rect.bottom -= 1;
                border = CreateSolidBrush(RGB(255, 255, 255));
                FrameRect(hdc, &rect, border);
                DeleteObject(border);
            }
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
        for (int i = 0; i < COLS; i++) AddDestroy(r, i);
    } else if (typeGrid[r][c] == TYPE_VERT) {
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

