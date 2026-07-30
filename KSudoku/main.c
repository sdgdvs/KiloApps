#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

// Colors
COLORREF themes[3][9] = {
    // Classic Dark
    { RGB(15, 23, 42), RGB(248, 250, 252), RGB(51, 65, 85), RGB(248, 250, 252), RGB(248, 250, 252), RGB(59, 130, 246), RGB(30, 64, 112), RGB(24, 44, 85), RGB(239, 68, 68) },
    // Neon Blue
    { RGB(0, 0, 0), RGB(0, 255, 255), RGB(0, 128, 128), RGB(224, 255, 255), RGB(224, 255, 255), RGB(0, 255, 255), RGB(0, 128, 128), RGB(0, 64, 64), RGB(255, 0, 0) },
    // Crimson Red
    { RGB(26, 5, 5), RGB(255, 100, 100), RGB(128, 20, 20), RGB(255, 240, 240), RGB(255, 240, 240), RGB(244, 63, 94), RGB(128, 20, 40), RGB(64, 10, 20), RGB(255, 0, 0) }
};
#define T_BG 0
#define T_GRID_THICK 1
#define T_GRID_THIN 2
#define T_TEXT 3
#define T_FIXED 4
#define T_MUTABLE 5
#define T_SEL 6
#define T_HL 7
#define T_ERR 8

#define MAX_GRID 16

int gridSize = 9;
int boxW = 3;
int boxH = 3;

int board[MAX_GRID][MAX_GRID];
int solution[MAX_GRID][MAX_GRID];
int fixed[MAX_GRID][MAX_GRID];
int error_cells[MAX_GRID][MAX_GRID];
int notes[MAX_GRID][MAX_GRID][17]; // 1..16
int fog_cells[MAX_GRID][MAX_GRID];
int awarded[MAX_GRID][MAX_GRID];

// Cage sum constraints
int cage_id[MAX_GRID][MAX_GRID];
int cage_sum[MAX_GRID * MAX_GRID];
int cage_is_topleft[MAX_GRID][MAX_GRID];
int num_cages = 0;

int sel_r = -1, sel_c = -1;
int notesMode = 0;
int elapsedTime = 0;
int score = 0;
int timerActive = 0;
int freezeTime = 0;

HWND hBtnNew, hBtnNotes, hBtnValidate, hBtnHint, hBtnUndo, hBtnRedo, hBtnSettings, hBtnAutoFill, hBtnCampaign, hBtnMagic, hBtnShield, hBtnFreeze, hBtnRush;
HFONT hFont, hFontSmall, hFontTiny;

typedef struct {
    int theme;
    int highlightSame;
    int soundEnabled;
} Prefs;
Prefs prefs = {0, 1, 1};

typedef struct {
    float x, y;
    float vx, vy;
    COLORREF color;
    int size;
    int life;
    int shape;
} WinParticle;

#define MAX_WIN_PARTICLES 120
WinParticle winParticles[MAX_WIN_PARTICLES];
int winFxActive = 0;

void TriggerVictoryParticles() {
    winFxActive = 1;
    COLORREF colors[] = {
        RGB(245, 158, 11), RGB(16, 185, 129), RGB(59, 130, 246),
        RGB(236, 72, 153), RGB(139, 92, 246), RGB(239, 68, 68), RGB(56, 189, 248)
    };
    int numColors = sizeof(colors)/sizeof(colors[0]);
    for(int i = 0; i < MAX_WIN_PARTICLES; i++) {
        winParticles[i].x = 220.0f + (rand() % 80 - 40);
        winParticles[i].y = 255.0f + (rand() % 80 - 40);
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 3.0f + (rand() % 100) / 10.0f;
        winParticles[i].vx = cosf(angle) * speed;
        winParticles[i].vy = sinf(angle) * speed - 5.0f;
        winParticles[i].color = colors[rand() % numColors];
        winParticles[i].size = 4 + (rand() % 6);
        winParticles[i].life = 40 + (rand() % 40);
        winParticles[i].shape = rand() % 2;
    }
}

void UpdateVictoryParticles() {
    if (!winFxActive) return;
    int anyAlive = 0;
    for(int i = 0; i < MAX_WIN_PARTICLES; i++) {
        if (winParticles[i].life > 0) {
            winParticles[i].x += winParticles[i].vx;
            winParticles[i].y += winParticles[i].vy;
            winParticles[i].vy += 0.35f;
            winParticles[i].life--;
            if (winParticles[i].life > 0) anyAlive = 1;
        }
    }
    if (!anyAlive) winFxActive = 0;
}

void LoadPrefs() {
    FILE *f = fopen("ksudoku_prefs.dat", "rb");
    if(f) { fread(&prefs, sizeof(Prefs), 1, f); fclose(f); }
}
void SavePrefs() {
    FILE *f = fopen("ksudoku_prefs.dat", "wb");
    if(f) { fwrite(&prefs, sizeof(Prefs), 1, f); fclose(f); }
}

void PlaySudokuSound(int soundType) {
    if (!prefs.soundEnabled) return;
    switch(soundType) {
        case 1: Beep(523, 20); break;  // Select cell
        case 2: Beep(659, 30); break;  // Input note/number
        case 3: Beep(784, 50); break;  // Correct placement
        case 4: Beep(220, 150); break; // Error strike
        case 5: Beep(1047, 80); break; // Wand / Shield / Freeze skill
        case 6:                        // Win fanfare
            Beep(523, 70); Beep(659, 70); Beep(784, 70); Beep(1047, 140);
            break;
        case 7:                        // Game Over / Fail
            Beep(300, 90); Beep(250, 90); Beep(200, 180);
            break;
    }
}

typedef struct {
    int board[MAX_GRID][MAX_GRID];
    int notes[MAX_GRID][MAX_GRID][17];
    int score;
} ActionState;

ActionState* undoStack = NULL;
int undoCapacity = 0;
int undoCount = 0;

ActionState* redoStack = NULL;
int redoCapacity = 0;
int redoCount = 0;

void PushState() {
    if (undoCount == undoCapacity) {
        undoCapacity = undoCapacity == 0 ? 16 : undoCapacity * 2;
        undoStack = (ActionState*)realloc(undoStack, undoCapacity * sizeof(ActionState));
    }
    for(int r=0; r<gridSize; r++) {
        for(int c=0; c<gridSize; c++) {
            undoStack[undoCount].board[r][c] = board[r][c];
            for(int i=0; i<=16; i++) undoStack[undoCount].notes[r][c][i] = notes[r][c][i];
        }
    }
    undoStack[undoCount].score = score;
    undoCount++;
    redoCount = 0;
}

void Undo() {
    if (undoCount > 0) {
        if (redoCount == redoCapacity) {
            redoCapacity = redoCapacity == 0 ? 16 : redoCapacity * 2;
            redoStack = (ActionState*)realloc(redoStack, redoCapacity * sizeof(ActionState));
        }
        for(int r=0; r<gridSize; r++) {
            for(int c=0; c<gridSize; c++) {
                redoStack[redoCount].board[r][c] = board[r][c];
                for(int i=0; i<=16; i++) redoStack[redoCount].notes[r][c][i] = notes[r][c][i];
            }
        }
        redoStack[redoCount].score = score;
        redoCount++;
        
        undoCount--;
        for(int r=0; r<gridSize; r++) {
            for(int c=0; c<gridSize; c++) {
                board[r][c] = undoStack[undoCount].board[r][c];
                for(int i=0; i<=16; i++) notes[r][c][i] = undoStack[undoCount].notes[r][c][i];
                error_cells[r][c] = 0;
            }
        }
        score = undoStack[undoCount].score;
        PlaySudokuSound(1);
    }
}

void Redo() {
    if (redoCount > 0) {
        if (undoCount == undoCapacity) {
            undoCapacity = undoCapacity == 0 ? 16 : undoCapacity * 2;
            undoStack = (ActionState*)realloc(undoStack, undoCapacity * sizeof(ActionState));
        }
        for(int r=0; r<gridSize; r++) {
            for(int c=0; c<gridSize; c++) {
                undoStack[undoCount].board[r][c] = board[r][c];
                for(int i=0; i<=16; i++) undoStack[undoCount].notes[r][c][i] = notes[r][c][i];
            }
        }
        undoStack[undoCount].score = score;
        undoCount++;
        
        redoCount--;
        for(int r=0; r<gridSize; r++) {
            for(int c=0; c<gridSize; c++) {
                board[r][c] = redoStack[redoCount].board[r][c];
                for(int i=0; i<=16; i++) notes[r][c][i] = redoStack[redoCount].notes[r][c][i];
                error_cells[r][c] = 0;
            }
        }
        score = redoStack[redoCount].score;
        PlaySudokuSound(1);
    }
}

typedef struct {
    int played;
    int won;
    int bestTime;
    int bestScore;
} DifficultyStats;

DifficultyStats stats[4] = {0}; // 0: Easy, 1: Medium, 2: Hard, 3: Rush Mode
int currentDiffIdx = 1;
int gameActive = 0;

int dailyStreak = 0;
int lastDailyDate = 0;
int isDailyGame = 0;

int isCampaignMode = 0;
int campaignStage = 0; // 0 to 19 (20 stages total)
int magicWands = 3;
int shields = 1;
int shieldActive = 0;
int freezeCharges = 2;
int strikes = 0;
int maxCampaignStage = 0;
int totalWandsUsed = 0;
int totalShieldsUsed = 0;

int isRushMode = 0;

void LoadStats() {
    FILE *f = fopen("ksudoku_stats.dat", "rb");
    if(f) {
        fread(stats, sizeof(DifficultyStats), 4, f);
        fread(&totalWandsUsed, sizeof(int), 1, f);
        fread(&totalShieldsUsed, sizeof(int), 1, f);
        fclose(f);
    } else {
        for(int i=0; i<4; i++) {
            stats[i].played = 0;
            stats[i].won = 0;
            stats[i].bestTime = -1;
            stats[i].bestScore = 0;
        }
    }
}

void SaveStats() {
    FILE *f = fopen("ksudoku_stats.dat", "wb");
    if(f) {
        fwrite(stats, sizeof(DifficultyStats), 4, f);
        fwrite(&totalWandsUsed, sizeof(int), 1, f);
        fwrite(&totalShieldsUsed, sizeof(int), 1, f);
        fclose(f);
    }
}

void LoadCampaignStats() {
    FILE *f = fopen("ksudoku_camp.dat", "rb");
    if(f) { fread(&maxCampaignStage, sizeof(int), 1, f); fclose(f); }
}

void SaveCampaignStats() {
    FILE *f = fopen("ksudoku_camp.dat", "wb");
    if(f) { fwrite(&maxCampaignStage, sizeof(int), 1, f); fclose(f); }
}

void LoadDailyStats() {
    FILE* f = fopen("ksudoku_daily.dat", "rb");
    if (f) {
        fread(&dailyStreak, sizeof(int), 1, f);
        fread(&lastDailyDate, sizeof(int), 1, f);
        fclose(f);
    }
}

void SaveDailyStats() {
    FILE* f = fopen("ksudoku_daily.dat", "wb");
    if (f) {
        fwrite(&dailyStreak, sizeof(int), 1, f);
        fwrite(&lastDailyDate, sizeof(int), 1, f);
        fclose(f);
    }
}

typedef struct {
    int gridSize, boxW, boxH;
    int board[MAX_GRID][MAX_GRID];
    int solution[MAX_GRID][MAX_GRID];
    int fixed[MAX_GRID][MAX_GRID];
    int notes[MAX_GRID][MAX_GRID][17];
    int fog_cells[MAX_GRID][MAX_GRID];
    int cage_id[MAX_GRID][MAX_GRID];
    int cage_sum[MAX_GRID * MAX_GRID];
    int cage_is_topleft[MAX_GRID][MAX_GRID];
    int num_cages;
    int elapsedTime;
    int freezeTime;
    int score;
    int awarded[MAX_GRID][MAX_GRID];
    int currentDiffIdx;
    int gameActive;
    int isDailyGame;
    int isCampaignMode;
    int campaignStage;
    int magicWands;
    int shields;
    int shieldActive;
    int freezeCharges;
    int strikes;
    int isRushMode;
} GameState;

int LoadGameState() {
    FILE *f = fopen("ksudoku_save.dat", "rb");
    if(!f) return 0;
    GameState state;
    if(fread(&state, sizeof(GameState), 1, f) == 1 && state.gameActive) {
        gridSize = state.gridSize > 0 ? state.gridSize : 9;
        boxW = state.boxW > 0 ? state.boxW : 3;
        boxH = state.boxH > 0 ? state.boxH : 3;
        num_cages = state.num_cages;

        for(int r=0; r<gridSize; r++) {
            for(int c=0; c<gridSize; c++) {
                board[r][c] = state.board[r][c];
                solution[r][c] = state.solution[r][c];
                fixed[r][c] = state.fixed[r][c];
                awarded[r][c] = state.awarded[r][c];
                fog_cells[r][c] = state.fog_cells[r][c];
                cage_id[r][c] = state.cage_id[r][c];
                cage_is_topleft[r][c] = state.cage_is_topleft[r][c];
                error_cells[r][c] = 0;
                for(int i=0; i<=16; i++) notes[r][c][i] = state.notes[r][c][i];
            }
        }
        for(int k=0; k<=num_cages; k++) cage_sum[k] = state.cage_sum[k];

        elapsedTime = state.elapsedTime;
        freezeTime = state.freezeTime;
        score = state.score;
        currentDiffIdx = state.currentDiffIdx;
        gameActive = 1;
        isDailyGame = state.isDailyGame;
        isCampaignMode = state.isCampaignMode;
        campaignStage = state.campaignStage;
        magicWands = state.magicWands;
        shields = state.shields;
        shieldActive = state.shieldActive;
        freezeCharges = state.freezeCharges;
        strikes = state.strikes;
        isRushMode = state.isRushMode;
        timerActive = 1;
        undoCount = 0;
        redoCount = 0;
        fclose(f);
        return 1;
    }
    fclose(f);
    return 0;
}

void SaveGameState() {
    if (!gameActive) {
        remove("ksudoku_save.dat");
        return;
    }
    FILE *f = fopen("ksudoku_save.dat", "wb");
    if (f) {
        GameState state = {0};
        state.gridSize = gridSize;
        state.boxW = boxW;
        state.boxH = boxH;
        state.num_cages = num_cages;

        for(int r=0; r<gridSize; r++) {
            for(int c=0; c<gridSize; c++) {
                state.board[r][c] = board[r][c];
                state.solution[r][c] = solution[r][c];
                state.fixed[r][c] = fixed[r][c];
                state.awarded[r][c] = awarded[r][c];
                state.fog_cells[r][c] = fog_cells[r][c];
                state.cage_id[r][c] = cage_id[r][c];
                state.cage_is_topleft[r][c] = cage_is_topleft[r][c];
                for(int i=0; i<=16; i++) state.notes[r][c][i] = notes[r][c][i];
            }
        }
        for(int k=0; k<=num_cages; k++) state.cage_sum[k] = cage_sum[k];

        state.elapsedTime = elapsedTime;
        state.freezeTime = freezeTime;
        state.score = score;
        state.currentDiffIdx = currentDiffIdx;
        state.gameActive = gameActive;
        state.isDailyGame = isDailyGame;
        state.isCampaignMode = isCampaignMode;
        state.campaignStage = campaignStage;
        state.magicWands = magicWands;
        state.shields = shields;
        state.shieldActive = shieldActive;
        state.freezeCharges = freezeCharges;
        state.strikes = strikes;
        state.isRushMode = isRushMode;
        fwrite(&state, sizeof(GameState), 1, f);
        fclose(f);
    }
}

void ShuffleArray(int* arr, int len) {
    for (int i = len - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

void ClearFogAround(int r, int c) {
    fog_cells[r][c] = 0;
    int dr[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};
    for(int i=0; i<4; i++) {
        int nr = r + dr[i];
        int nc = c + dc[i];
        if (nr >= 0 && nr < gridSize && nc >= 0 && nc < gridSize) fog_cells[nr][nc] = 0;
    }
    int br = (r / boxH) * boxH, bc = (c / boxW) * boxW;
    for(int i=0; i<boxH; i++) {
        for(int j=0; j<boxW; j++) {
            fog_cells[br+i][bc+j] = 0;
        }
    }
}

void UpdatePowerupButtons() {
    if (hBtnMagic) {
        char buf[32];
        sprintf(buf, "Wand (%d)", magicWands);
        SetWindowTextA(hBtnMagic, buf);
    }
    if (hBtnShield) {
        char buf[32];
        if (shieldActive) sprintf(buf, "Shield ON");
        else sprintf(buf, "Shield (%d)", shields);
        SetWindowTextA(hBtnShield, buf);
    }
    if (hBtnFreeze) {
        char buf[32];
        if (freezeTime > 0) sprintf(buf, "Frozen (%ds)", freezeTime);
        else sprintf(buf, "Freeze (%d)", freezeCharges);
        SetWindowTextA(hBtnFreeze, buf);
    }
}

int IsValidPlacement(int r, int c, int num) {
    for(int i=0; i<gridSize; i++) {
        if(i != c && board[r][i] == num) return 0;
        if(i != r && board[i][c] == num) return 0;
    }
    int br = (r/boxH)*boxH, bc = (c/boxW)*boxW;
    for(int i=0; i<boxH; i++) {
        for(int j=0; j<boxW; j++) {
            if((br+i != r || bc+j != c) && board[br+i][bc+j] == num) return 0;
        }
    }
    // Killer Sudoku Cage Check
    if (cage_id[r][c] > 0) {
        int cid = cage_id[r][c];
        int cageSum = 0, cageCountCells = 0, cageFilled = 0;
        for (int cr=0; cr<gridSize; cr++) {
            for (int cc=0; cc<gridSize; cc++) {
                if (cage_id[cr][cc] == cid) {
                    cageCountCells++;
                    int val = (cr == r && cc == c) ? num : board[cr][cc];
                    if (val > 0) {
                        cageFilled++;
                        cageSum += val;
                    }
                }
            }
        }
        if (cageSum > cage_sum[cid] || (cageFilled == cageCountCells && cageSum != cage_sum[cid])) {
            return 0;
        }
    }
    return 1;
}

int CountSolutionsCore(int r, int c, int* count) {
    if (r == gridSize) {
        (*count)++;
        return (*count > 1) ? 1 : 0;
    }
    int nr = c == gridSize - 1 ? r + 1 : r;
    int nc = c == gridSize - 1 ? 0 : c + 1;
    if (board[r][c] != 0) {
        return CountSolutionsCore(nr, nc, count);
    }
    for (int v = 1; v <= gridSize; v++) {
        if (IsValidPlacement(r, c, v)) {
            board[r][c] = v;
            if (CountSolutionsCore(nr, nc, count)) {
                board[r][c] = 0;
                return 1;
            }
            board[r][c] = 0;
        }
    }
    return 0;
}


void GenerateBoardEx(int gSize, int removal, int isDaily, int fogCount, int cageCount, int isRush) {
    gridSize = gSize;
    if (gridSize == 4) { boxW = 2; boxH = 2; }
    else if (gridSize == 16) { boxW = 4; boxH = 4; }
    else { gridSize = 9; boxW = 3; boxH = 3; }

    isDailyGame = isDaily;
    isRushMode = isRush;
    if (isDaily) {
        time_t t = time(NULL);
        struct tm* tm_info = localtime(&t);
        int dateSeed = (tm_info->tm_year + 1900) * 10000 + (tm_info->tm_mon + 1) * 100 + tm_info->tm_mday;
        srand(dateSeed);
    } else {
        srand((unsigned int)time(NULL));
    }

    // Base Sudoku solution matrix: (r*boxW + r/boxW + c) % gridSize + 1
    for(int r=0; r<gridSize; r++) {
        for(int c=0; c<gridSize; c++) {
            solution[r][c] = ((r * boxW + r / boxW + c) % gridSize) + 1;
        }
    }

    // Symbol permutation
    int nums[17];
    for(int i=0; i<gridSize; i++) nums[i] = i+1;
    ShuffleArray(nums, gridSize);
    for(int r=0; r<gridSize; r++)
        for(int c=0; c<gridSize; c++)
            solution[r][c] = nums[solution[r][c]-1];

    // Row band shuffle
    for(int band=0; band<boxW; band++) {
        int rows[4] = {0,1,2,3};
        ShuffleArray(rows, boxH);
        int temp[4][MAX_GRID];
        for(int i=0; i<boxH; i++)
            for(int c=0; c<gridSize; c++) temp[i][c] = solution[band*boxH + rows[i]][c];
        for(int i=0; i<boxH; i++)
            for(int c=0; c<gridSize; c++) solution[band*boxH + i][c] = temp[i][c];
    }

    // Col band shuffle
    for(int band=0; band<boxH; band++) {
        int cols[4] = {0,1,2,3};
        ShuffleArray(cols, boxW);
        int temp[MAX_GRID][4];
        for(int i=0; i<boxW; i++)
            for(int r=0; r<gridSize; r++) temp[r][i] = solution[r][band*boxW + cols[i]];
        for(int i=0; i<boxW; i++)
            for(int r=0; r<gridSize; r++) solution[r][band*boxW + i] = temp[r][i];
    }

    for(int r=0; r<gridSize; r++) {
        for(int c=0; c<gridSize; c++) {
            board[r][c] = solution[r][c];
            fixed[r][c] = 1;
            error_cells[r][c] = 0;
            fog_cells[r][c] = 0;
            cage_id[r][c] = 0;
            cage_is_topleft[r][c] = 0;
        }
    }

    int removalTarget = removal;
    int requireUnique = (removalTarget <= (gridSize * gridSize) / 2);
    
    int pos[MAX_GRID * MAX_GRID];
    for(int i=0; i<gridSize*gridSize; i++) pos[i] = i;
    ShuffleArray(pos, gridSize*gridSize);

    int pIdx = 0;
    while(removal > 0 && pIdx < gridSize*gridSize) {
        int r = pos[pIdx] / gridSize;
        int c = pos[pIdx] % gridSize;
        pIdx++;
        if(board[r][c] != 0) {
            int temp = board[r][c];
            board[r][c] = 0;
            
            if (requireUnique) {
                int count = 0;
                CountSolutionsCore(0, 0, &count);
                if (count > 1) {
                    board[r][c] = temp;
                    continue;
                }
            }
            fixed[r][c] = 0;
            removal--;
        }
    }

    // Apply Fog cells among empty cells
    if (fogCount > 0) {
        int emptyCells[MAX_GRID * MAX_GRID][2];
        int numEmpty = 0;
        for(int r=0; r<gridSize; r++) {
            for(int c=0; c<gridSize; c++) {
                if(board[r][c] == 0) {
                    emptyCells[numEmpty][0] = r;
                    emptyCells[numEmpty][1] = c;
                    numEmpty++;
                }
            }
        }
        for(int f=0; f<fogCount && numEmpty > 0; f++) {
            int idx = rand() % numEmpty;
            fog_cells[emptyCells[idx][0]][emptyCells[idx][1]] = 1;
            emptyCells[idx][0] = emptyCells[numEmpty-1][0];
            emptyCells[idx][1] = emptyCells[numEmpty-1][1];
            numEmpty--;
        }
    }

    // Apply Killer Sudoku Cages
    num_cages = 0;
    int dr[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    for (int k = 1; k <= cageCount; k++) {
        int r = rand() % gridSize;
        int c = rand() % gridSize;
        int tries = 0;
        while (cage_id[r][c] != 0 && tries < 100) {
            r = rand() % gridSize;
            c = rand() % gridSize;
            tries++;
        }
        if (cage_id[r][c] != 0) continue;

        int targetSize = 2 + (rand() % (gridSize == 4 ? 2 : 3));
        int cageCells[4][2];
        int currentSize = 1;
        cageCells[0][0] = r;
        cageCells[0][1] = c;
        cage_id[r][c] = k;

        while (currentSize < targetSize) {
            int baseIdx = rand() % currentSize;
            int br = cageCells[baseIdx][0];
            int bc = cageCells[baseIdx][1];
            int dir = rand() % 4;
            int nr = br + dr[dir];
            int nc = bc + dc[dir];
            if (nr >= 0 && nr < gridSize && nc >= 0 && nc < gridSize && cage_id[nr][nc] == 0) {
                cage_id[nr][nc] = k;
                cageCells[currentSize][0] = nr;
                cageCells[currentSize][1] = nc;
                currentSize++;
            } else {
                int found = 0;
                for (int d = 0; d < 4; d++) {
                    int tr = br + dr[d], tc = bc + dc[d];
                    if (tr >= 0 && tr < gridSize && tc >= 0 && tc < gridSize && cage_id[tr][tc] == 0) {
                        cage_id[tr][tc] = k;
                        cageCells[currentSize][0] = tr;
                        cageCells[currentSize][1] = tc;
                        currentSize++;
                        found = 1;
                        break;
                    }
                }
                if (!found) break;
            }
        }

        int sum = 0;
        int minPos = 9999;
        int topR = 0, topC = 0;
        for (int i = 0; i < currentSize; i++) {
            int cr = cageCells[i][0];
            int cc = cageCells[i][1];
            sum += solution[cr][cc];
            int pos = cr * gridSize + cc;
            if (pos < minPos) {
                minPos = pos;
                topR = cr;
                topC = cc;
            }
        }
        cage_sum[k] = sum;
        cage_is_topleft[topR][topC] = 1;
        num_cages = k;
    }

    for(int r=0; r<gridSize; r++) {
        for(int c=0; c<gridSize; c++) {
            for(int i=0; i<=16; i++) notes[r][c][i] = 0;
            awarded[r][c] = 0;
        }
    }

    if (isRushMode) {
        elapsedTime = 180;
    } else {
        elapsedTime = 0;
    }
    freezeTime = 0;
    score = 0;
    timerActive = 1;
    gameActive = 1;
    UpdatePowerupButtons();
    SaveGameState();
}

// 20 Stages Campaign Table
typedef struct {
    int gSize;
    int removal;
    int fogCount;
    int cageCount;
} CampaignStageDef;

CampaignStageDef campaignStages[20] = {
    {4,  6,  0, 0}, // Stage 1: 4x4 Mini Easy
    {4,  8,  1, 1}, // Stage 2: 4x4 Mini Medium
    {4,  10, 2, 2}, // Stage 3: 4x4 Mini Hard
    {9,  28, 0, 0}, // Stage 4: 9x9 Classic Easy
    {9,  32, 1, 1}, // Stage 5: 9x9 Classic Easy+
    {9,  38, 2, 2}, // Stage 6: 9x9 Classic Medium
    {9,  42, 3, 3}, // Stage 7: 9x9 Classic Medium+
    {9,  48, 4, 4}, // Stage 8: 9x9 Classic Hard
    {9,  52, 5, 5}, // Stage 9: 9x9 Classic Hard+
    {9,  56, 6, 6}, // Stage 10: 9x9 Classic Expert
    {9,  60, 7, 7}, // Stage 11: 9x9 Classic Expert+
    {16, 80, 2, 2}, // Stage 12: 16x16 Hexadoku Easy
    {16, 100,4, 4}, // Stage 13: 16x16 Hexadoku Medium
    {16, 120,6, 6}, // Stage 14: 16x16 Hexadoku Hard
    {9,  58, 6, 8}, // Stage 15: 9x9 Killer Master
    {9,  62, 10,6}, // Stage 16: 9x9 Fog Nightmare
    {16, 140,8, 8}, // Stage 17: 16x16 Hexadoku Expert
    {9,  65, 8, 8}, // Stage 18: 9x9 Fiendish Blitz
    {16, 155,10,10},// Stage 19: 16x16 Hexadoku Fiendish
    {16, 165,12,12} // Stage 20: Stage 20 Fiendish Master Challenge
};

void StartCampaignStage(HWND hwnd, int stage) {
    isCampaignMode = 1;
    isRushMode = 0;
    campaignStage = stage;
    if (campaignStage > 19) campaignStage = 19;
    magicWands = 3;
    shields = 1;
    shieldActive = 0;
    freezeCharges = 2;
    strikes = 0;
    
    CampaignStageDef def = campaignStages[campaignStage];
    GenerateBoardEx(def.gSize, def.removal, 0, def.fogCount, def.cageCount, 0);
    sel_r = -1; sel_c = -1;
    UpdatePowerupButtons();
    InvalidateRect(hwnd, NULL, TRUE);
}

void StartRushMode(HWND hwnd) {
    isCampaignMode = 0;
    isRushMode = 1;
    shields = 1;
    shieldActive = 0;
    magicWands = 1;
    freezeCharges = 1;
    currentDiffIdx = 3;
    stats[3].played++;
    SaveStats();
    GenerateBoardEx(9, 45, 0, 0, 0, 1);
    sel_r = -1; sel_c = -1;
    UpdatePowerupButtons();
    InvalidateRect(hwnd, NULL, TRUE);
}

void CheckWin(HWND hwnd) {
    for(int r=0; r<gridSize; r++)
        for(int c=0; c<gridSize; c++)
            if(board[r][c] != solution[r][c]) return;
            
    timerActive = 0;
    PlaySudokuSound(6);
    TriggerVictoryParticles();
    
    if (isRushMode) {
        score += elapsedTime * 10 + 500;
        stats[3].won++;
        if (stats[3].bestTime == -1 || elapsedTime > stats[3].bestTime) stats[3].bestTime = elapsedTime;
        if (score > stats[3].bestScore) stats[3].bestScore = score;
        SaveStats();
        gameActive = 0;
        SaveGameState();
        char msg[256];
        sprintf(msg, "RUSH MODE VICTORY!\nTime Remaining: %02d:%02d\nFinal Score: %d", elapsedTime/60, elapsedTime%60, score);
        MessageBoxA(hwnd, msg, "KSudoku Rush", MB_OK | MB_ICONINFORMATION);
        return;
    }
    
    int timeBonus = 3000 - elapsedTime * 2;
    if(timeBonus < 0) timeBonus = 0;
    score += timeBonus;
    
    if (gameActive) {
        if (isCampaignMode) {
            if (campaignStage > maxCampaignStage) { maxCampaignStage = campaignStage; SaveCampaignStats(); }
            if (campaignStage < 19) {
                char msg[256];
                sprintf(msg, "Stage %d Clear!\nTime: %02d:%02d\nScore: %d\nReady for Stage %d?", campaignStage+1, elapsedTime/60, elapsedTime%60, score, campaignStage+2);
                MessageBoxA(hwnd, msg, "KSudoku Campaign", MB_OK);
                int keepScore = score;
                campaignStage++;
                StartCampaignStage(hwnd, campaignStage);
                score = keepScore;
                SaveGameState();
                InvalidateRect(hwnd, NULL, TRUE);
                return;
            } else {
                char msg[256];
                sprintf(msg, "CONGRATULATIONS!!\nYou completed all 20 Stages of the KSudoku Campaign!\nFinal Score: %d", score);
                MessageBoxA(hwnd, msg, "Campaign Complete!", MB_OK);
                gameActive = 0;
                SaveGameState();
                return;
            }
        } else {
            gameActive = 0;
            SaveGameState();
            if (isDailyGame) {
                time_t t = time(NULL);
                struct tm* tm_info = localtime(&t);
                int todayStr = (tm_info->tm_year + 1900) * 10000 + (tm_info->tm_mon + 1) * 100 + tm_info->tm_mday;
                if (lastDailyDate != todayStr) {
                    t -= 86400;
                    struct tm* ytm = localtime(&t);
                    int yestStr = (ytm->tm_year + 1900) * 10000 + (ytm->tm_mon + 1) * 100 + ytm->tm_mday;
                    if (lastDailyDate == yestStr) dailyStreak++;
                    else dailyStreak = 1;
                    lastDailyDate = todayStr;
                    SaveDailyStats();
                }
            } else {
                stats[currentDiffIdx].won++;
                if (stats[currentDiffIdx].bestTime == -1 || elapsedTime < stats[currentDiffIdx].bestTime) 
                    stats[currentDiffIdx].bestTime = elapsedTime;
                if (score > stats[currentDiffIdx].bestScore) 
                    stats[currentDiffIdx].bestScore = score;
                SaveStats();
            }
        }
    }
    
    char msg[256];
    sprintf(msg, "Congratulations! You solved the puzzle!\nTime: %02d:%02d\nScore: %d", elapsedTime/60, elapsedTime%60, score);
    MessageBoxA(hwnd, msg, "KSudoku", MB_OK);
}

HWND hSettingsWnd = NULL;
LRESULT CALLBACK SettingsWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: {
            CreateWindowA("STATIC", "Theme:", WS_CHILD|WS_VISIBLE, 10, 10, 50, 20, hwnd, NULL, NULL, NULL);
            HWND hCmb = CreateWindowA("COMBOBOX", "", WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST, 70, 10, 120, 100, hwnd, (HMENU)1, NULL, NULL);
            SendMessageA(hCmb, CB_ADDSTRING, 0, (LPARAM)"Classic Dark");
            SendMessageA(hCmb, CB_ADDSTRING, 0, (LPARAM)"Neon Blue");
            SendMessageA(hCmb, CB_ADDSTRING, 0, (LPARAM)"Crimson Red");
            SendMessageA(hCmb, CB_SETCURSEL, prefs.theme, 0);
            
            CreateWindowA("BUTTON", "Highlight Same Numbers", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 10, 40, 180, 20, hwnd, (HMENU)2, NULL, NULL);
            SendDlgItemMessageA(hwnd, 2, BM_SETCHECK, prefs.highlightSame ? BST_CHECKED : BST_UNCHECKED, 0);
            
            CreateWindowA("BUTTON", "Sound Effects", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 10, 65, 180, 20, hwnd, (HMENU)4, NULL, NULL);
            SendDlgItemMessageA(hwnd, 4, BM_SETCHECK, prefs.soundEnabled ? BST_CHECKED : BST_UNCHECKED, 0);
            
            CreateWindowA("BUTTON", "Save & Close", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, 50, 95, 100, 30, hwnd, (HMENU)3, NULL, NULL);
            break;
        }
        case WM_COMMAND:
            if(LOWORD(wParam) == 3) {
                prefs.theme = SendDlgItemMessageA(hwnd, 1, CB_GETCURSEL, 0, 0);
                prefs.highlightSame = SendDlgItemMessageA(hwnd, 2, BM_GETCHECK, 0, 0) == BST_CHECKED;
                prefs.soundEnabled = SendDlgItemMessageA(hwnd, 4, BM_GETCHECK, 0, 0) == BST_CHECKED;
                SavePrefs();
                HWND hParent = GetWindow(hwnd, GW_OWNER);
                InvalidateRect(hParent, NULL, TRUE);
                DestroyWindow(hwnd);
            }
            break;
        case WM_DESTROY:
            hSettingsWnd = NULL;
            break;
        default: return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: {
            srand((unsigned int)time(NULL));
            LoadStats();
            LoadDailyStats();
            LoadCampaignStats();
            LoadPrefs();
            if(!LoadGameState()) {
                currentDiffIdx = 1;
                stats[currentDiffIdx].played++;
                SaveStats();
                GenerateBoardEx(9, 40, 0, 0, 0, 0);
            }
            HWND hComboDifficulty = CreateWindowA("COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 10, 8, 65, 100, hwnd, (HMENU)4, NULL, NULL);
            SendMessageA(hComboDifficulty, CB_ADDSTRING, 0, (LPARAM)"Easy");
            SendMessageA(hComboDifficulty, CB_ADDSTRING, 0, (LPARAM)"Medium");
            SendMessageA(hComboDifficulty, CB_ADDSTRING, 0, (LPARAM)"Hard");
            SendMessageA(hComboDifficulty, CB_SETCURSEL, currentDiffIdx < 3 ? currentDiffIdx : 1, 0);
            
            hBtnNew = CreateWindowA("BUTTON", "New", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 80, 8, 40, 28, hwnd, (HMENU)1, NULL, NULL);
            hBtnCampaign = CreateWindowA("BUTTON", "Campaign", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 125, 8, 65, 28, hwnd, (HMENU)13, NULL, NULL);
            hBtnRush = CreateWindowA("BUTTON", "Rush", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 195, 8, 45, 28, hwnd, (HMENU)14, NULL, NULL);
            HWND hBtnDaily = CreateWindowA("BUTTON", "Daily", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 245, 8, 40, 28, hwnd, (HMENU)10, NULL, NULL);
            HWND hBtnStats = CreateWindowA("BUTTON", "Stats", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 290, 8, 45, 28, hwnd, (HMENU)6, NULL, NULL);
            hBtnSettings = CreateWindowA("BUTTON", "Settings", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 340, 8, 55, 28, hwnd, (HMENU)9, NULL, NULL);
            
            hBtnNotes = CreateWindowA("BUTTON", "Notes: OFF", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 40, 65, 28, hwnd, (HMENU)3, NULL, NULL);
            hBtnValidate = CreateWindowA("BUTTON", "Validate", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 80, 40, 50, 28, hwnd, (HMENU)2, NULL, NULL);
            hBtnHint = CreateWindowA("BUTTON", "Hint (H)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 135, 40, 50, 28, hwnd, (HMENU)5, NULL, NULL);
            hBtnMagic = CreateWindowA("BUTTON", "Wand (3)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 190, 40, 50, 28, hwnd, (HMENU)12, NULL, NULL);
            hBtnShield = CreateWindowA("BUTTON", "Shield (1)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 245, 40, 55, 28, hwnd, (HMENU)15, NULL, NULL);
            hBtnAutoFill = CreateWindowA("BUTTON", "Auto (P)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 305, 40, 45, 28, hwnd, (HMENU)11, NULL, NULL);
            hBtnFreeze = CreateWindowA("BUTTON", "Freeze (F)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 355, 40, 55, 28, hwnd, (HMENU)16, NULL, NULL);
            hBtnUndo = CreateWindowA("BUTTON", "Undo", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 415, 40, 40, 28, hwnd, (HMENU)7, NULL, NULL);
            hBtnRedo = CreateWindowA("BUTTON", "Redo", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 460, 40, 40, 28, hwnd, (HMENU)8, NULL, NULL);

            hFont = CreateFontA(24, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            hFontSmall = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            hFontTiny = CreateFontA(10, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");

            SetTimer(hwnd, 1, 1000, NULL);
            SetTimer(hwnd, 2, 30, NULL);
            UpdatePowerupButtons();
            break;
        }
        case WM_TIMER: {
            if (wParam == 2) {
                if (winFxActive) {
                    UpdateVictoryParticles();
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == 1 && timerActive) {
                if (freezeTime > 0) {
                    freezeTime--;
                    UpdatePowerupButtons();
                } else if (isRushMode) {
                    elapsedTime--;
                    if (elapsedTime <= 0) {
                        elapsedTime = 0;
                        timerActive = 0;
                        gameActive = 0;
                        PlaySudokuSound(7);
                        SaveGameState();
                        MessageBoxA(hwnd, "Time's Up! Rush Mode Failed.", "Rush Mode", MB_OK | MB_ICONWARNING);
                    }
                } else {
                    elapsedTime++;
                }
                SaveGameState();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) { // New Game
                HWND hCombo = GetDlgItem(hwnd, 4);
                int sel = SendMessageA(hCombo, CB_GETCURSEL, 0, 0);
                int removal = 40;
                currentDiffIdx = 1;
                if(sel == 0) { removal = 30; currentDiffIdx = 0; }
                else if(sel == 2) { removal = 50; currentDiffIdx = 2; }
                stats[currentDiffIdx].played++;
                SaveStats();
                undoCount = 0; redoCount = 0;
                isCampaignMode = 0; isRushMode = 0;
                GenerateBoardEx(9, removal, 0, 0, 0, 0);
                sel_r = -1; sel_c = -1;
                PlaySudokuSound(1);
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 14) { // Rush Mode
                StartRushMode(hwnd);
            } else if (LOWORD(wParam) == 10) { // Daily Challenge
                time_t t = time(NULL);
                struct tm* tm_info = localtime(&t);
                int todayStr = (tm_info->tm_year + 1900) * 10000 + (tm_info->tm_mon + 1) * 100 + tm_info->tm_mday;
                if (lastDailyDate == todayStr) {
                    MessageBoxA(hwnd, "You have already completed today's challenge!", "Daily Challenge", MB_OK | MB_ICONINFORMATION);
                } else {
                    currentDiffIdx = 1;
                    undoCount = 0; redoCount = 0;
                    isCampaignMode = 0; isRushMode = 0;
                    GenerateBoardEx(9, 40, 1, 0, 0, 0);
                    sel_r = -1; sel_c = -1;
                    PlaySudokuSound(1);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (LOWORD(wParam) == 6) { // Stats
                char msg[1024] = "";
                time_t t = time(NULL);
                struct tm* tm_info = localtime(&t);
                int todayStr = (tm_info->tm_year + 1900) * 10000 + (tm_info->tm_mon + 1) * 100 + tm_info->tm_mday;
                sprintf(msg, "[Daily Challenge & Progress]\nDaily Streak: %d | Completed Today: %s\nCampaign Max Stage: %d / 20\nWands Used: %d | Shields Used: %d\n\n",
                        dailyStreak, (lastDailyDate == todayStr) ? "Yes" : "No", maxCampaignStage + 1, totalWandsUsed, totalShieldsUsed);

                const char* diffs[] = {"Easy", "Medium", "Hard", "Rush Mode"};
                for(int i=0; i<4; i++) {
                    int rate = stats[i].played > 0 ? (stats[i].won * 100) / stats[i].played : 0;
                    char bt[32];
                    if(stats[i].bestTime == -1) sprintf(bt, "--:--");
                    else sprintf(bt, "%02d:%02d", stats[i].bestTime/60, stats[i].bestTime%60);
                    char buf[128];
                    sprintf(buf, "[%s]\nPlayed: %d | Won: %d | Win Rate: %d%%\nBest %s: %s | Best Score: %d\n\n", 
                            diffs[i], stats[i].played, stats[i].won, rate, i == 3 ? "Time Left" : "Time", bt, stats[i].bestScore);
                    strcat(msg, buf);
                }
                MessageBoxA(hwnd, msg, "Statistics", MB_OK | MB_ICONINFORMATION);
            } else if (LOWORD(wParam) == 3) { // Toggle Notes
                notesMode = !notesMode;
                SetWindowTextA(hBtnNotes, notesMode ? "Notes: ON" : "Notes: OFF");
                PlaySudokuSound(1);
            } else if (LOWORD(wParam) == 2) { // Validate
                int errorCount = 0;
                for(int r=0; r<gridSize; r++) {
                    for(int c=0; c<gridSize; c++) {
                        if (!fixed[r][c] && board[r][c] != 0 && board[r][c] != solution[r][c]) {
                            error_cells[r][c] = 1;
                            errorCount++;
                        } else {
                            error_cells[r][c] = 0;
                        }
                    }
                }
                if (errorCount > 0) {
                    score = max(0, score - errorCount * 20);
                    PlaySudokuSound(4);
                } else {
                    PlaySudokuSound(3);
                }
                SaveGameState();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 5) { // Smart Hint
                int targetR = sel_r, targetC = sel_c;
                if (targetR < 0 || targetC < 0 || fixed[targetR][targetC] || board[targetR][targetC] == solution[targetR][targetC]) {
                    targetR = -1; targetC = -1;
                    for (int r=0; r<gridSize; r++) {
                        for (int c=0; c<gridSize; c++) {
                            if (!fixed[r][c] && board[r][c] != solution[r][c]) {
                                targetR = r; targetC = c; break;
                            }
                        }
                        if (targetR >= 0) break;
                    }
                }
                if (targetR >= 0 && targetC >= 0) {
                    PushState();
                    score = max(0, score - 150);
                    board[targetR][targetC] = solution[targetR][targetC];
                    awarded[targetR][targetC] = 1;
                    error_cells[targetR][targetC] = 0;
                    ClearFogAround(targetR, targetC);
                    PlaySudokuSound(3);
                    CheckWin(hwnd);
                }
                SaveGameState();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 7) { // Undo
                Undo();
                SaveGameState();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 8) { // Redo
                Redo();
                SaveGameState();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 11) { // Auto-fill Pencil Notes
                PushState();
                for(int r=0; r<gridSize; r++) {
                    for(int c=0; c<gridSize; c++) {
                        if(board[r][c] == 0 && !fog_cells[r][c]) {
                            for(int i=1; i<=gridSize; i++) notes[r][c][i] = 0;
                            for(int num=1; num<=gridSize; num++) {
                                int invalid = 0;
                                for(int i=0; i<gridSize; i++) {
                                    if(i != c && board[r][i] == num) invalid = 1;
                                    if(i != r && board[i][c] == num) invalid = 1;
                                }
                                int br = (r/boxH)*boxH, bc = (c/boxW)*boxW;
                                for(int i=0; i<boxH; i++) {
                                    for(int j=0; j<boxW; j++) {
                                        if((br+i != r || bc+j != c) && board[br+i][bc+j] == num) invalid = 1;
                                    }
                                }
                                if(!invalid) notes[r][c][num] = 1;
                            }
                        }
                    }
                }
                PlaySudokuSound(2);
                SaveGameState();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 13) { // Campaign
                StartCampaignStage(hwnd, maxCampaignStage > 19 ? 19 : maxCampaignStage);
            } else if (LOWORD(wParam) == 12) { // Magic Wand
                if (magicWands > 0) {
                    int emptyCount = 0;
                    for(int r=0; r<gridSize; r++)
                        for(int c=0; c<gridSize; c++)
                            if(!fixed[r][c] && board[r][c] != solution[r][c]) emptyCount++;
                    if(emptyCount > 0) {
                        int target = rand() % emptyCount;
                        int idx = 0;
                        for(int r=0; r<gridSize; r++) {
                            for(int c=0; c<gridSize; c++) {
                                if(!fixed[r][c] && board[r][c] != solution[r][c]) {
                                    if(idx == target) {
                                        PushState();
                                        board[r][c] = solution[r][c];
                                        awarded[r][c] = 1;
                                        error_cells[r][c] = 0;
                                        ClearFogAround(r, c);
                                        magicWands--;
                                        totalWandsUsed++;
                                        score += 50;
                                        PlaySudokuSound(5);
                                        UpdatePowerupButtons();
                                        CheckWin(hwnd);
                                        SaveGameState();
                                        SaveStats();
                                        InvalidateRect(hwnd, NULL, TRUE);
                                        break;
                                    }
                                    idx++;
                                }
                            }
                        }
                    }
                }
            } else if (LOWORD(wParam) == 15) { // Shield Power-up
                if (shields > 0 && !shieldActive) {
                    shields--;
                    shieldActive = 1;
                    totalShieldsUsed++;
                    PlaySudokuSound(5);
                    UpdatePowerupButtons();
                    SaveStats();
                    SaveGameState();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (LOWORD(wParam) == 16) { // Time Freeze
                if (freezeCharges > 0 || freezeTime > 0) {
                    if (freezeCharges > 0) freezeCharges--;
                    freezeTime += 20;
                    PlaySudokuSound(5);
                    UpdatePowerupButtons();
                    SaveGameState();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (LOWORD(wParam) == 9) { // Settings
                if(!hSettingsWnd) {
                    RECT rc; GetWindowRect(hwnd, &rc);
                    hSettingsWnd = CreateWindowA("SettingsClass", "Settings", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                        rc.left + 50, rc.top + 50, 220, 170, hwnd, NULL, GetModuleHandle(NULL), NULL);
                    ShowWindow(hSettingsWnd, SW_SHOW);
                }
                SetFocus(hSettingsWnd);
                return 0;
            }
            SetFocus(hwnd);
            break;
        }
        case WM_KEYDOWN: {
            if(sel_r == -1) sel_r = 0, sel_c = 0;
            else {
                if(wParam == VK_UP) sel_r = max(0, sel_r - 1);
                if(wParam == VK_DOWN) sel_r = min(gridSize - 1, sel_r + 1);
                if(wParam == VK_LEFT) sel_c = max(0, sel_c - 1);
                if(wParam == VK_RIGHT) sel_c = min(gridSize - 1, sel_c + 1);
            }
            PlaySudokuSound(1);
            
            int inputNum = 0;
            if (wParam >= '1' && wParam <= '9') inputNum = wParam - '0';
            else if (gridSize == 16 && (wParam >= 'A' && wParam <= 'G')) inputNum = 10 + (wParam - 'A');
            else if (gridSize == 16 && (wParam >= 'a' && wParam <= 'g')) inputNum = 10 + (wParam - 'a');

            if (wParam == 'H' || wParam == 'h') { SendMessageA(hwnd, WM_COMMAND, 5, 0); return 0; }
            if (wParam == 'P' || wParam == 'p') { SendMessageA(hwnd, WM_COMMAND, 11, 0); return 0; }
            if (wParam == 'S' || wParam == 's') { SendMessageA(hwnd, WM_COMMAND, 15, 0); return 0; }
            if (wParam == 'F' || wParam == 'f') {
                if (gridSize == 16 && (GetKeyState(VK_SHIFT) & 0x8000)) inputNum = 15;
                else { SendMessageA(hwnd, WM_COMMAND, 16, 0); return 0; }
            }
            if (wParam == 'W' || wParam == 'w') { SendMessageA(hwnd, WM_COMMAND, 12, 0); return 0; }
            if (wParam == 'N' || wParam == 'n') { SendMessageA(hwnd, WM_COMMAND, 3, 0); return 0; }

            if(inputNum >= 1 && inputNum <= gridSize) {
                if(!fixed[sel_r][sel_c]) {
                    int num = inputNum;
                    if (notesMode) {
                        if (board[sel_r][sel_c] == 0) {
                            PushState();
                            notes[sel_r][sel_c][num] = !notes[sel_r][sel_c][num];
                            PlaySudokuSound(2);
                        }
                    } else {
                        if (board[sel_r][sel_c] != num) {
                            PushState();
                            int invalid = 0;
                            for(int i=0; i<gridSize; i++) {
                                if(i != sel_c && board[sel_r][i] == num) invalid = 1;
                                if(i != sel_r && board[i][sel_c] == num) invalid = 1;
                            }
                            int br = (sel_r/boxH)*boxH, bc = (sel_c/boxW)*boxW;
                            for(int r=0; r<boxH; r++) {
                                for(int c=0; c<boxW; c++) {
                                    if((br+r != sel_r || bc+c != sel_c) && board[br+r][bc+c] == num) invalid = 1;
                                }
                            }

                            // Killer Sudoku Cage Check
                            if (cage_id[sel_r][sel_c] > 0) {
                                int cid = cage_id[sel_r][sel_c];
                                int cageSum = 0, cageCountCells = 0, cageFilled = 0;
                                for (int r=0; r<gridSize; r++) {
                                    for (int c=0; c<gridSize; c++) {
                                        if (cage_id[r][c] == cid) {
                                            cageCountCells++;
                                            int val = (r == sel_r && c == sel_c) ? num : board[r][c];
                                            if (val > 0) {
                                                cageFilled++;
                                                cageSum += val;
                                            }
                                        }
                                    }
                                }
                                if (cageSum > cage_sum[cid] || (cageFilled == cageCountCells && cageSum != cage_sum[cid])) {
                                    invalid = 1;
                                }
                            }
                            
                            if(invalid) {
                                if (shieldActive) {
                                    shieldActive = 0;
                                    UpdatePowerupButtons();
                                    PlaySudokuSound(5);
                                } else {
                                    PlaySudokuSound(4);
                                    score = max(0, score - 50);
                                    if (isRushMode) {
                                        elapsedTime = max(0, elapsedTime - 15);
                                    }
                                    if (isCampaignMode) {
                                        strikes++;
                                        if (strikes >= 3) {
                                            PlaySudokuSound(7);
                                            MessageBoxA(hwnd, "3 Strikes! Stage Failed. Restarting stage.", "Game Over", MB_OK);
                                            StartCampaignStage(hwnd, campaignStage);
                                            return 0;
                                        }
                                    }
                                }
                            } else {
                                PlaySudokuSound(3);
                            }
                            board[sel_r][sel_c] = num;
                            error_cells[sel_r][sel_c] = 0;
                            ClearFogAround(sel_r, sel_c);
                            
                            if (num == solution[sel_r][sel_c]) {
                                if (!awarded[sel_r][sel_c]) {
                                    awarded[sel_r][sel_c] = 1;
                                    score += 100;
                                    if (isRushMode) elapsedTime += 10;
                                }
                            }
                            CheckWin(hwnd);
                        }
                    }
                }
            } else if(wParam == VK_BACK || wParam == VK_DELETE || wParam == '0') {
                if(!fixed[sel_r][sel_c]) {
                    if (board[sel_r][sel_c] != 0) {
                        PushState();
                        board[sel_r][sel_c] = 0;
                        error_cells[sel_r][sel_c] = 0;
                        for(int i=0; i<=16; i++) notes[sel_r][sel_c][i] = 0;
                        PlaySudokuSound(2);
                    } else {
                        int hasNotes = 0;
                        for(int i=0; i<=16; i++) if (notes[sel_r][sel_c][i]) hasNotes = 1;
                        if (hasNotes) {
                            PushState();
                            for(int i=0; i<=16; i++) notes[sel_r][sel_c][i] = 0;
                            PlaySudokuSound(2);
                        }
                    }
                }
            } else if(wParam == 'Z' && (GetKeyState(VK_CONTROL) & 0x8000)) {
                Undo();
            } else if(wParam == 'Y' && (GetKeyState(VK_CONTROL) & 0x8000)) {
                Redo();
            }
            SaveGameState();
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            int cell_sz = (gridSize == 4) ? 80 : (gridSize == 16 ? 22 : 40);
            int start_x = 40, start_y = 75;
            if(x >= start_x && x < start_x + gridSize*cell_sz && y >= start_y && y < start_y + gridSize*cell_sz) {
                sel_c = (x - start_x) / cell_sz;
                sel_r = (y - start_y) / cell_sz;
                PlaySudokuSound(1);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            SetFocus(hwnd);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            HBRUSH hbg = CreateSolidBrush(themes[prefs.theme][T_BG]);
            FillRect(hdc, &clientRect, hbg);
            DeleteObject(hbg);
            
            HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, themes[prefs.theme][T_MUTABLE]);
            
            char info[128];
            char freezeStr[32] = "";
            if (freezeTime > 0) sprintf(freezeStr, " [FROZEN %ds]", freezeTime);

            if (isCampaignMode) {
                sprintf(info, "Stage: %d/20 (%dx%d)   Strikes: %d/3   Time: %02d:%02d%s   Score: %d", campaignStage+1, gridSize, gridSize, strikes, elapsedTime/60, elapsedTime%60, freezeStr, score);
                TextOutA(hdc, 20, 48, info, strlen(info));
            } else if (isRushMode) {
                sprintf(info, "RUSH MODE   Time Left: %02d:%02d%s   Score: %d", elapsedTime/60, elapsedTime%60, freezeStr, score);
                TextOutA(hdc, 20, 48, info, strlen(info));
            } else {
                sprintf(info, "Time: %02d:%02d%s   Score: %d", elapsedTime/60, elapsedTime%60, freezeStr, score);
                TextOutA(hdc, 20, 48, info, strlen(info));
            }
            
            int cell_sz = (gridSize == 4) ? 80 : (gridSize == 16 ? 22 : 40);
            int start_x = 40, start_y = 75;
            int board_w = gridSize * cell_sz;

            RECT outerFrame = {start_x - 6, start_y - 6, start_x + board_w + 6, start_y + board_w + 6};
            HBRUSH hFrameBrush = CreateSolidBrush(RGB(30, 41, 59));
            FillRect(hdc, &outerFrame, hFrameBrush);
            DeleteObject(hFrameBrush);

            HPEN hLightPen = CreatePen(PS_SOLID, 2, RGB(71, 85, 105));
            HPEN hDarkPen = CreatePen(PS_SOLID, 2, RGB(15, 23, 42));
            HPEN oldPen = (HPEN)SelectObject(hdc, hLightPen);

            MoveToEx(hdc, outerFrame.left, outerFrame.bottom, NULL);
            LineTo(hdc, outerFrame.left, outerFrame.top);
            LineTo(hdc, outerFrame.right, outerFrame.top);

            SelectObject(hdc, hDarkPen);
            LineTo(hdc, outerFrame.right, outerFrame.bottom);
            LineTo(hdc, outerFrame.left, outerFrame.bottom);

            SelectObject(hdc, oldPen);
            DeleteObject(hLightPen);
            DeleteObject(hDarkPen);
            
            int highlight_val = 0;
            if(sel_r >= 0 && sel_c >= 0 && board[sel_r][sel_c] != 0) {
                highlight_val = board[sel_r][sel_c];
            }
            
            // Draw cells
            for(int r=0; r<gridSize; r++) {
                for(int c=0; c<gridSize; c++) {
                    RECT rc = {start_x + c*cell_sz, start_y + r*cell_sz, start_x + (c+1)*cell_sz, start_y + (r+1)*cell_sz};
                    RECT cellInner = {rc.left + 1, rc.top + 1, rc.right - 1, rc.bottom - 1};
                    
                    HBRUSH cellBg = NULL;
                    if(r == sel_r && c == sel_c) {
                        cellBg = CreateSolidBrush(RGB(37, 99, 235));
                    } else if(sel_r >= 0 && (r == sel_r || c == sel_c || (r/boxH == sel_r/boxH && c/boxW == sel_c/boxW))) {
                        cellBg = CreateSolidBrush(themes[prefs.theme][T_HL]);
                    } else if(prefs.highlightSame && highlight_val && board[r][c] == highlight_val) {
                        cellBg = CreateSolidBrush(themes[prefs.theme][T_HL]);
                    } else if(cage_id[r][c] > 0) {
                        cellBg = CreateSolidBrush(RGB(42, 45, 25));
                    } else if(fixed[r][c] && board[r][c] != 0) {
                        cellBg = CreateSolidBrush(RGB(24, 34, 56));
                    } else {
                        cellBg = CreateSolidBrush(themes[prefs.theme][T_BG]);
                    }
                    
                    FillRect(hdc, &cellInner, cellBg);
                    DeleteObject(cellBg);

                    // Cage dotted border
                    if (cage_id[r][c] > 0) {
                        HPEN hCagePen = CreatePen(PS_DOT, 1, RGB(234, 179, 8));
                        oldPen = (HPEN)SelectObject(hdc, hCagePen);
                        HBRUSH hNullB = (HBRUSH)GetStockObject(NULL_BRUSH);
                        HBRUSH oldB = (HBRUSH)SelectObject(hdc, hNullB);
                        Rectangle(hdc, cellInner.left, cellInner.top, cellInner.right, cellInner.bottom);
                        SelectObject(hdc, oldB);
                        SelectObject(hdc, oldPen);
                        DeleteObject(hCagePen);
                    }

                    // Recessed inset cell bevel
                    HPEN hInsetShadow = CreatePen(PS_SOLID, 1, RGB(15, 23, 42));
                    HPEN hInsetHighlight = CreatePen(PS_SOLID, 1, RGB(51, 65, 85));
                    oldPen = (HPEN)SelectObject(hdc, hInsetShadow);

                    MoveToEx(hdc, cellInner.left, cellInner.bottom, NULL);
                    LineTo(hdc, cellInner.left, cellInner.top);
                    LineTo(hdc, cellInner.right, cellInner.top);

                    SelectObject(hdc, hInsetHighlight);
                    LineTo(hdc, cellInner.right, cellInner.bottom);
                    LineTo(hdc, cellInner.left, cellInner.bottom);

                    SelectObject(hdc, oldPen);
                    DeleteObject(hInsetShadow);
                    DeleteObject(hInsetHighlight);

                    // Error Box
                    if(error_cells[r][c]) {
                        HPEN hErrPen = CreatePen(PS_SOLID, 2, RGB(239, 68, 68));
                        oldPen = (HPEN)SelectObject(hdc, hErrPen);
                        HBRUSH hNullB = (HBRUSH)GetStockObject(NULL_BRUSH);
                        HBRUSH oldB = (HBRUSH)SelectObject(hdc, hNullB);
                        Rectangle(hdc, rc.left - 1, rc.top - 1, rc.right + 1, rc.bottom + 1);
                        SelectObject(hdc, oldB);
                        SelectObject(hdc, oldPen);
                        DeleteObject(hErrPen);
                    }

                    // Selected Aura
                    if(r == sel_r && c == sel_c) {
                        HPEN hSelPen = CreatePen(PS_SOLID, 2, RGB(147, 197, 253));
                        oldPen = (HPEN)SelectObject(hdc, hSelPen);
                        HBRUSH hNullB = (HBRUSH)GetStockObject(NULL_BRUSH);
                        HBRUSH oldB = (HBRUSH)SelectObject(hdc, hNullB);
                        Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
                        SelectObject(hdc, oldB);
                        SelectObject(hdc, oldPen);
                        DeleteObject(hSelPen);
                    }
                    
                    // Top-Left Cage Sum Text
                    if (cage_is_topleft[r][c] && cage_id[r][c] > 0) {
                        HFONT oldF = (HFONT)SelectObject(hdc, hFontTiny);
                        SetTextColor(hdc, RGB(250, 204, 21));
                        char cbuf[8];
                        sprintf(cbuf, "%d", cage_sum[cage_id[r][c]]);
                        TextOutA(hdc, rc.left + 2, rc.top + 1, cbuf, strlen(cbuf));
                        SelectObject(hdc, oldF);
                    }

                    if (fog_cells[r][c] && board[r][c] == 0) {
                        HFONT oldFnt = (HFONT)SelectObject(hdc, gridSize == 16 ? hFontSmall : hFont);
                        SetTextColor(hdc, RGB(148, 163, 184));
                        RECT textRc = rc;
                        textRc.top += 2;
                        DrawTextA(hdc, "?", -1, &textRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                        SelectObject(hdc, oldFnt);
                    } else if(board[r][c] != 0) {
                        char buf[4];
                        int val = board[r][c];
                        if (gridSize == 16 && val >= 10) sprintf(buf, "%c", 'A' + (val - 10));
                        else sprintf(buf, "%d", val);
                        
                        if(fixed[r][c] && !error_cells[r][c]) {
                            SetTextColor(hdc, RGB(0, 0, 0));
                            RECT shadowRc = rc; shadowRc.left += 1; shadowRc.top += 2;
                            HFONT curF = (HFONT)SelectObject(hdc, gridSize == 16 ? hFontSmall : hFont);
                            DrawTextA(hdc, buf, -1, &shadowRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                            SelectObject(hdc, curF);
                            SetTextColor(hdc, RGB(248, 250, 252));
                        } else if(error_cells[r][c]) {
                            SetTextColor(hdc, RGB(248, 113, 113));
                        } else {
                            SetTextColor(hdc, themes[prefs.theme][T_MUTABLE]);
                        }
                        
                        HFONT curF = (HFONT)SelectObject(hdc, gridSize == 16 ? hFontSmall : hFont);
                        RECT textRc = rc;
                        if (cage_is_topleft[r][c]) textRc.top += 4;
                        DrawTextA(hdc, buf, -1, &textRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                        SelectObject(hdc, curF);
                    } else {
                        int hasNotes = 0;
                        for(int i=1; i<=gridSize; i++) if(notes[r][c][i]) hasNotes = 1;
                        if(hasNotes && gridSize <= 9) {
                            HFONT oldFnt = (HFONT)SelectObject(hdc, hFontSmall);
                            for(int i=1; i<=gridSize; i++) {
                                if(notes[r][c][i]) {
                                    int sub_r = (i-1) / boxW;
                                    int sub_c = (i-1) % boxW;
                                    RECT noteRc = rc;
                                    noteRc.left += sub_c * (cell_sz / boxW) + 1;
                                    noteRc.right = noteRc.left + (cell_sz / boxW) - 1;
                                    noteRc.top += sub_r * (cell_sz / boxH) + 1;
                                    noteRc.bottom = noteRc.top + (cell_sz / boxH) - 1;
                                    
                                    HBRUSH hPipBrush = CreateSolidBrush(RGB(30, 58, 100));
                                    FillRect(hdc, &noteRc, hPipBrush);
                                    DeleteObject(hPipBrush);

                                    SetTextColor(hdc, RGB(147, 197, 253));
                                    char buf[2];
                                    sprintf(buf, "%d", i);
                                    DrawTextA(hdc, buf, -1, &noteRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                                }
                            }
                            SelectObject(hdc, oldFnt);
                        }
                    }
                }
            }
            
            // Draw grid lines with thick block dividers
            for(int i=0; i<=gridSize; i++) {
                int thickV = (i % boxW == 0);
                COLORREF gridColV = thickV ? RGB(248, 250, 252) : themes[prefs.theme][T_GRID_THIN];
                HPEN hPenV = CreatePen(PS_SOLID, thickV ? 3 : 1, gridColV);
                HPEN oldPenV = (HPEN)SelectObject(hdc, hPenV);
                MoveToEx(hdc, start_x + i*cell_sz, start_y, NULL);
                LineTo(hdc, start_x + i*cell_sz, start_y + gridSize*cell_sz);
                SelectObject(hdc, oldPenV);
                DeleteObject(hPenV);

                int thickH = (i % boxH == 0);
                COLORREF gridColH = thickH ? RGB(248, 250, 252) : themes[prefs.theme][T_GRID_THIN];
                HPEN hPenH = CreatePen(PS_SOLID, thickH ? 3 : 1, gridColH);
                HPEN oldPenH = (HPEN)SelectObject(hdc, hPenH);
                MoveToEx(hdc, start_x, start_y + i*cell_sz, NULL);
                LineTo(hdc, start_x + gridSize*cell_sz, start_y + i*cell_sz);
                SelectObject(hdc, oldPenH);
                DeleteObject(hPenH);
            }

            // Draw Victory Confetti Particles
            if (winFxActive) {
                for(int i = 0; i < MAX_WIN_PARTICLES; i++) {
                    if (winParticles[i].life > 0) {
                        HBRUSH pBrush = CreateSolidBrush(winParticles[i].color);
                        HPEN pPen = CreatePen(PS_SOLID, 1, winParticles[i].color);
                        HPEN oldP = (HPEN)SelectObject(hdc, pPen);
                        HBRUSH oldB = (HBRUSH)SelectObject(hdc, pBrush);

                        int px = (int)winParticles[i].x;
                        int py = (int)winParticles[i].y;
                        int psz = winParticles[i].size;

                        if (winParticles[i].shape == 0) {
                            Rectangle(hdc, px, py, px + psz, py + psz + 2);
                        } else {
                            Ellipse(hdc, px, py, px + psz, py + psz);
                        }

                        SelectObject(hdc, oldP);
                        SelectObject(hdc, oldB);
                        DeleteObject(pPen);
                        DeleteObject(pBrush);
                    }
                }
            }
            
            SelectObject(hdc, oldFont);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            DeleteObject(hFont);
            DeleteObject(hFontSmall);
            DeleteObject(hFontTiny);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "KSudokuClass";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClassA(&wc);
    
    WNDCLASSA wcs = {0};
    wcs.lpfnWndProc = SettingsWndProc;
    wcs.hInstance = hInstance;
    wcs.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcs.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wcs.lpszClassName = "SettingsClass";
    RegisterClassA(&wcs);
    
    RECT rc = {0, 0, 520, 520}; 
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX), FALSE);
    
    HWND hwnd = CreateWindowA("KSudokuClass", "KSudoku", WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
        
    ShowWindow(hwnd, nCmdShow);
    
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void MainEntry() {
    HINSTANCE hInst = GetModuleHandle(NULL);
    int ret = WinMain(hInst, NULL, GetCommandLineA(), SW_SHOWDEFAULT);
    ExitProcess(ret);
}
