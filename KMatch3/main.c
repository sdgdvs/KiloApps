#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#define ROWS 8
#define COLS 8
#define CELL_SIZE 50
#define BOARD_X 50
#define BOARD_Y 50

int grid[ROWS][COLS];
int typeGrid[ROWS][COLS] = {0};
int score = 0;
int moves = 20;
int level = 1;
int gameMode = 0;
int targetScore = 1000;
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

void PlaySwapSound() { Beep(300, 100); }
void PlayMatchSound(int combo) { Beep(400 + combo * 100, 150); }
void PlayBadSwapSound() { Beep(150, 150); }

COLORREF colors[] = {
    RGB(255, 64, 64),
    RGB(64, 255, 64),
    RGB(64, 64, 255),
    RGB(255, 255, 64),
    RGB(255, 64, 255),
    RGB(64, 255, 255)
};

void DrawBoard(HDC hdc) {
    char buf[128];
    if (gameMode == 0) sprintf(buf, "Lvl: %d  Score: %d/%d  Moves: %d", level, score, targetScore, moves);
    else if (gameMode == 1) sprintf(buf, "Lvl: %d  Score: %d  Zen Mode", level, score);
    else if (gameMode == 2) sprintf(buf, "Lvl: %d  Score: %d/%d  Time: %ds", level, score, targetScore, moves);
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    TextOut(hdc, BOARD_X, 10, buf, strlen(buf));
    
    char statsBuf[128];
    sprintf(statsBuf, "[1] Classic [2] Zen [3] Timed | Best: %d | [H] Help", statsBestScore);
    SetTextColor(hdc, RGB(200, 200, 200));
    TextOut(hdc, BOARD_X, 30, statsBuf, strlen(statsBuf));

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
            if (grid[r][c] == -1) continue;

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

            HBRUSH brush = CreateSolidBrush(typeGrid[r][c] == 3 ? RGB(255,255,255) : colors[grid[r][c]]);
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);

            if (typeGrid[r][c] == 1) { // horiz
                RECT hRect = { rect.left, rect.top + CELL_SIZE/2 - 4, rect.right, rect.top + CELL_SIZE/2 + 4 };
                HBRUSH b = CreateSolidBrush(RGB(255,255,255));
                FillRect(hdc, &hRect, b);
                DeleteObject(b);
            } else if (typeGrid[r][c] == 2) { // vert
                RECT vRect = { rect.left + CELL_SIZE/2 - 4, rect.top, rect.left + CELL_SIZE/2 + 4, rect.bottom };
                HBRUSH b = CreateSolidBrush(RGB(255,255,255));
                FillRect(hdc, &vRect, b);
                DeleteObject(b);
            } else if (typeGrid[r][c] == 3) { // color
                RECT cRect = { rect.left + 12, rect.top + 12, rect.right - 12, rect.bottom - 12 };
                HBRUSH b = CreateSolidBrush(RGB(0,0,0));
                FillRect(hdc, &cRect, b);
                DeleteObject(b);
            }

            if (r == selR && c == selC) {
                HBRUSH border = CreateSolidBrush(RGB(255, 255, 255));
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

void AddDestroy(int r, int c) {
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return;
    if (grid[r][c] == -1) return;
    if (toDestroy[r][c]) return;
    
    toDestroy[r][c] = 1;
    if (typeGrid[r][c] == 1) {
        for (int i = 0; i < COLS; i++) AddDestroy(r, i);
    } else if (typeGrid[r][c] == 2) {
        for (int i = 0; i < ROWS; i++) AddDestroy(i, c);
    } else if (typeGrid[r][c] == 3) {
        int tcolor = rand() % 6;
        for(int i=0; i<ROWS; i++) {
            for(int j=0; j<COLS; j++) {
                if (grid[i][j] == tcolor && typeGrid[i][j] != 3) AddDestroy(i, j);
            }
        }
    }
}

int CheckMatchPossible() {
    for (int r=0; r<ROWS; r++) {
        for (int c=0; c<COLS-2; c++) {
            int color = grid[r][c];
            if (color != -1 && typeGrid[r][c] != 3 && grid[r][c+1] == color && typeGrid[r][c+1] != 3 && grid[r][c+2] == color && typeGrid[r][c+2] != 3) return 1;
        }
    }
    for (int c=0; c<COLS; c++) {
        for (int r=0; r<ROWS-2; r++) {
            int color = grid[r][c];
            if (color != -1 && typeGrid[r][c] != 3 && grid[r+1][c] == color && typeGrid[r+1][c] != 3 && grid[r+2][c] == color && typeGrid[r+2][c] != 3) return 1;
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
        int newSpecials[ROWS][COLS] = {0}; 

        if (triggerR != -1) {
            toDestroy[triggerR][triggerC] = 1;
            if (triggerColor == 999) {
                for(int i=0; i<ROWS; i++) for(int j=0; j<COLS; j++) if(grid[i][j]!=-1) AddDestroy(i, j);
            } else if (triggerColor == 888) {
                typeGrid[triggerR][triggerC] = 1; AddDestroy(triggerR, triggerC);
                typeGrid[lastSwapR1][lastSwapC1] = 2; AddDestroy(lastSwapR1, lastSwapC1);
            } else if (triggerColor >= 0 && triggerColor < 6) {
                for(int i=0; i<ROWS; i++) for(int j=0; j<COLS; j++) {
                    if (grid[i][j] == triggerColor) AddDestroy(i, j);
                }
            }
            triggerR = -1; 
        }

        int hMatchLen[ROWS][COLS] = {0};
        int vMatchLen[ROWS][COLS] = {0};
        
        for (int r=0; r<ROWS; r++) {
            for (int c=0; c<COLS-2; c++) {
                int color = grid[r][c];
                if (color != -1 && typeGrid[r][c] != 3 && grid[r][c+1] == color && typeGrid[r][c+1] != 3 && grid[r][c+2] == color && typeGrid[r][c+2] != 3) {
                    int k = c;
                    while(k < COLS && grid[r][k] == color && typeGrid[r][k] != 3) k++;
                    int len = k - c;
                    for(int i = c; i < k; i++) hMatchLen[r][i] = len;
                    c = k - 1;
                }
            }
        }
        for (int c=0; c<COLS; c++) {
            for (int r=0; r<ROWS-2; r++) {
                int color = grid[r][c];
                if (color != -1 && typeGrid[r][c] != 3 && grid[r+1][c] == color && typeGrid[r+1][c] != 3 && grid[r+2][c] == color && typeGrid[r+2][c] != 3) {
                    int k = r;
                    while(k < ROWS && grid[k][c] == color && typeGrid[k][c] != 3) k++;
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
                if (hMatchLen[r][c] >= 5) {
                    int sC = c;
                    for (int k = c; k < c + hMatchLen[r][c]; k++) {
                        if ((r == lastSwapR1 && k == lastSwapC1) || (r == lastSwapR2 && k == lastSwapC2)) sC = k;
                    }
                    newSpecials[r][sC] = 3;
                    c += hMatchLen[r][c] - 1;
                }
            }
        }
        for(int c=0; c<COLS; c++) {
            for(int r=0; r<ROWS; r++) {
                if (vMatchLen[r][c] >= 5) {
                    int sR = r;
                    for (int k = r; k < r + vMatchLen[r][c]; k++) {
                        if ((k == lastSwapR1 && c == lastSwapC1) || (k == lastSwapR2 && c == lastSwapC2)) sR = k;
                    }
                    newSpecials[sR][c] = 3;
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
                    if (!newSpecials[r][sC]) newSpecials[r][sC] = 1;
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
                    if (!newSpecials[sR][c]) newSpecials[sR][c] = 2;
                    r += 3;
                }
            }
        }

        int anyDestroy = 0;
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                if (toDestroy[r][c]) {
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
                    COLORREF pc = (typeGrid[r][c] == 3) ? RGB(255,255,255) : colors[grid[r][c]];
                    CreateParticles(BOARD_X + c * CELL_SIZE + CELL_SIZE / 2, BOARD_Y + r * CELL_SIZE + CELL_SIZE / 2, pc);
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
                    grid[r][c] = -1;
                    typeGrid[r][c] = 0;
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
            int emptyR = ROWS - 1;
            for (int r = ROWS - 1; r >= 0; r--) {
                if (grid[r][c] != -1) {
                    if (emptyR != r) {
                        grid[emptyR][c] = grid[r][c];
                        typeGrid[emptyR][c] = typeGrid[r][c];
                        grid[r][c] = -1;
                        typeGrid[r][c] = 0;
                        dropCount[emptyR][c] = emptyR - r;
                    } else {
                        dropCount[emptyR][c] = 0;
                    }
                    emptyR--;
                }
            }
            for (int r = emptyR; r >= 0; r--) {
                grid[r][c] = rand() % 6;
                typeGrid[r][c] = 0;
                dropCount[r][c] = emptyR + 1;
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

void InitGame() {
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
            typeGrid[r][c] = 0;
        }
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
    fclose(f);
    return 1;
}

void GameOver(HWND hwnd) {
    statsGamesPlayed++;
    SaveStats();
    MessageBox(hwnd, "Game Over!", "KMatch3", MB_OK | MB_ICONWARNING);
    level = 1; score = 0;
    if (gameMode == 0) { moves = 20; targetScore = 1000; }
    else if (gameMode == 1) { moves = 0; targetScore = 0; }
    else if (gameMode == 2) { moves = 60; targetScore = 1000; }
    InitGame();
    InvalidateRect(hwnd, NULL, FALSE);
}

void CheckLevelProgress(HWND hwnd) {
    if (gameMode == 1) {
        if (score >= level * 2000) level++;
        SaveGame();
        return;
    }
    if (score >= targetScore) {
        level++;
        if (gameMode == 0) moves += 15;
        if (gameMode == 2) moves += 30;
        targetScore += 1000 + (level * 500);
        MessageBox(hwnd, "Level Up!", "KMatch3", MB_OK | MB_ICONINFORMATION);
        InvalidateRect(hwnd, NULL, FALSE);
    } else if ((gameMode == 0 || gameMode == 2) && moves <= 0) {
        GameOver(hwnd);
    }
    SaveGame();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            srand((unsigned)time(NULL));
            LoadStats();
            if (!LoadGame()) {
                InitGame();
                SaveGame();
            }
            SetTimer(hwnd, 1, 30, NULL);
            SetTimer(hwnd, 2, 1000, NULL);
            break;
        case WM_TIMER: {
            if (wParam == 2) {
                if (gameMode == 2 && moves > 0 && !isProcessing) {
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
                gameMode = 0; level = 1; score = 0; moves = 20; targetScore = 1000; InitGame(); SaveGame(); InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == '2' || wParam == VK_NUMPAD2) {
                gameMode = 1; level = 1; score = 0; moves = 0; targetScore = 0; InitGame(); SaveGame(); InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == '3' || wParam == VK_NUMPAD3) {
                gameMode = 2; level = 1; score = 0; moves = 60; targetScore = 1000; InitGame(); SaveGame(); InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'H' || wParam == 'h' || wParam == VK_F1) {
                MessageBox(hwnd, "How to Play:\nSwap adjacent gems to form lines of 3+.\n\nSpecial Gems:\n- Match 4: Line Bomb (destroys row/col).\n- Match 5: Color Bomb (destroys all of one color).\n- Swap two specials for big effects!\n\nModes:\n- [1] Classic: Target score in limited moves.\n- [2] Zen: Infinite play.\n- [3] Timed: Target score in time limit.", "Help / How to Play", MB_OK | MB_ICONINFORMATION);
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
                if (selR == -1) {
                    selR = r; selC = c;
                    InvalidateRect(hwnd, NULL, FALSE);
                } else {
                    if (selR == r && selC == c) {
                        selR = -1; selC = -1;
                        InvalidateRect(hwnd, NULL, FALSE);
                    } else if (abs(selR - r) + abs(selC - c) == 1) {
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
                        if (typeGrid[origR][origC] == 3 && typeGrid[r][c] == 3) {
                            triggerR = origR; triggerC = origC; triggerColor = 999;
                        } else if (typeGrid[origR][origC] == 3 && typeGrid[r][c] != 3) {
                            triggerR = origR; triggerC = origC; triggerColor = grid[r][c];
                        } else if (typeGrid[r][c] == 3 && typeGrid[origR][origC] != 3) {
                            triggerR = r; triggerC = c; triggerColor = grid[origR][origC];
                        } else if (typeGrid[origR][origC] > 0 && typeGrid[r][c] > 0 && typeGrid[origR][origC] < 3 && typeGrid[r][c] < 3) {
                            triggerR = r; triggerC = c; triggerColor = 888; // cross
                        }

                        if (triggerR != -1 || CheckMatchPossible()) {
                            if (gameMode == 0) moves--;
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));
    wc.lpszClassName = "KMatch3Class";

    if(!RegisterClassEx(&wc)) return 0;

    HWND hwnd = CreateWindowEx(
        0, "KMatch3Class", "KMatch3",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 550,
        NULL, NULL, hInstance, NULL
    );

    if(!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
