#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

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

int board[9][9];
int solution[9][9];
int fixed[9][9];
int sel_r = -1, sel_c = -1;
int error_cells[9][9];
int notes[9][9][10];
int fog_cells[9][9];
int notesMode = 0;
int elapsedTime = 0; // In Rush mode, countdown from 180
int score = 0;
int awarded[9][9];
int timerActive = 0;

HWND hBtnNew, hBtnNotes, hBtnValidate, hBtnHint, hBtnUndo, hBtnRedo, hBtnSettings, hBtnAutoFill, hBtnCampaign, hBtnMagic, hBtnShield, hBtnRush;
HFONT hFont, hFontSmall;

typedef struct {
    int theme;
    int highlightSame;
    int soundEnabled;
} Prefs;
Prefs prefs = {0, 1, 1};

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
        case 5: Beep(1047, 80); break; // Wand / Shield powerup
        case 6:                        // Win fanfare
            Beep(523, 70); Beep(659, 70); Beep(784, 70); Beep(1047, 140);
            break;
        case 7:                        // Game Over / Fail
            Beep(300, 90); Beep(250, 90); Beep(200, 180);
            break;
    }
}

typedef struct {
    int board[9][9];
    int notes[9][9][10];
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
    for(int r=0; r<9; r++) {
        for(int c=0; c<9; c++) {
            undoStack[undoCount].board[r][c] = board[r][c];
            for(int i=0; i<10; i++) undoStack[undoCount].notes[r][c][i] = notes[r][c][i];
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
        for(int r=0; r<9; r++) {
            for(int c=0; c<9; c++) {
                redoStack[redoCount].board[r][c] = board[r][c];
                for(int i=0; i<10; i++) redoStack[redoCount].notes[r][c][i] = notes[r][c][i];
            }
        }
        redoStack[redoCount].score = score;
        redoCount++;
        
        undoCount--;
        for(int r=0; r<9; r++) {
            for(int c=0; c<9; c++) {
                board[r][c] = undoStack[undoCount].board[r][c];
                for(int i=0; i<10; i++) notes[r][c][i] = undoStack[undoCount].notes[r][c][i];
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
        for(int r=0; r<9; r++) {
            for(int c=0; c<9; c++) {
                undoStack[undoCount].board[r][c] = board[r][c];
                for(int i=0; i<10; i++) undoStack[undoCount].notes[r][c][i] = notes[r][c][i];
            }
        }
        undoStack[undoCount].score = score;
        undoCount++;
        
        redoCount--;
        for(int r=0; r<9; r++) {
            for(int c=0; c<9; c++) {
                board[r][c] = redoStack[redoCount].board[r][c];
                for(int i=0; i<10; i++) notes[r][c][i] = redoStack[redoCount].notes[r][c][i];
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
int campaignStage = 0; // 0 to 14 (15 stages total)
int magicWands = 3;
int shields = 1;
int shieldActive = 0;
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
    int board[9][9];
    int solution[9][9];
    int fixed[9][9];
    int notes[9][9][10];
    int fog_cells[9][9];
    int elapsedTime;
    int score;
    int awarded[9][9];
    int currentDiffIdx;
    int gameActive;
    int isDailyGame;
    int isCampaignMode;
    int campaignStage;
    int magicWands;
    int shields;
    int shieldActive;
    int strikes;
    int isRushMode;
} GameState;

int LoadGameState() {
    FILE *f = fopen("ksudoku_save.dat", "rb");
    if(!f) return 0;
    GameState state;
    if(fread(&state, sizeof(GameState), 1, f) == 1 && state.gameActive) {
        for(int r=0; r<9; r++) {
            for(int c=0; c<9; c++) {
                board[r][c] = state.board[r][c];
                solution[r][c] = state.solution[r][c];
                fixed[r][c] = state.fixed[r][c];
                awarded[r][c] = state.awarded[r][c];
                fog_cells[r][c] = state.fog_cells[r][c];
                error_cells[r][c] = 0;
                for(int i=0; i<10; i++) notes[r][c][i] = state.notes[r][c][i];
            }
        }
        elapsedTime = state.elapsedTime;
        score = state.score;
        currentDiffIdx = state.currentDiffIdx;
        gameActive = 1;
        isDailyGame = state.isDailyGame;
        isCampaignMode = state.isCampaignMode;
        campaignStage = state.campaignStage;
        magicWands = state.magicWands;
        shields = state.shields;
        shieldActive = state.shieldActive;
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
        GameState state;
        for(int r=0; r<9; r++) {
            for(int c=0; c<9; c++) {
                state.board[r][c] = board[r][c];
                state.solution[r][c] = solution[r][c];
                state.fixed[r][c] = fixed[r][c];
                state.awarded[r][c] = awarded[r][c];
                state.fog_cells[r][c] = fog_cells[r][c];
                for(int i=0; i<10; i++) state.notes[r][c][i] = notes[r][c][i];
            }
        }
        state.elapsedTime = elapsedTime;
        state.score = score;
        state.currentDiffIdx = currentDiffIdx;
        state.gameActive = gameActive;
        state.isDailyGame = isDailyGame;
        state.isCampaignMode = isCampaignMode;
        state.campaignStage = campaignStage;
        state.magicWands = magicWands;
        state.shields = shields;
        state.shieldActive = shieldActive;
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
        if (nr >= 0 && nr < 9 && nc >= 0 && nc < 9) fog_cells[nr][nc] = 0;
    }
    int br = (r/3)*3, bc = (c/3)*3;
    for(int i=0; i<3; i++) {
        for(int j=0; j<3; j++) {
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
}

void GenerateBoardEx(int removal, int isDaily, int fogCount, int isRush) {
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
    int base[9][9] = {
        {1,2,3, 4,5,6, 7,8,9},
        {4,5,6, 7,8,9, 1,2,3},
        {7,8,9, 1,2,3, 4,5,6},
        {2,3,1, 5,6,4, 8,9,7},
        {5,6,4, 8,9,7, 2,3,1},
        {8,9,7, 2,3,1, 5,6,4},
        {3,1,2, 6,4,5, 9,7,8},
        {6,4,5, 9,7,8, 3,1,2},
        {9,7,8, 3,1,2, 6,4,5}
    };
    int nums[9] = {1,2,3,4,5,6,7,8,9};
    ShuffleArray(nums, 9);
    for(int r=0; r<9; r++)
        for(int c=0; c<9; c++)
            solution[r][c] = nums[base[r][c]-1];
            
    for(int band=0; band<3; band++) {
        int rows[3] = {0,1,2};
        ShuffleArray(rows, 3);
        int temp[3][9];
        for(int i=0; i<3; i++)
            for(int c=0; c<9; c++) temp[i][c] = solution[band*3 + rows[i]][c];
        for(int i=0; i<3; i++)
            for(int c=0; c<9; c++) solution[band*3 + i][c] = temp[i][c];
    }
    
    for(int band=0; band<3; band++) {
        int cols[3] = {0,1,2};
        ShuffleArray(cols, 3);
        int temp[9][3];
        for(int i=0; i<3; i++)
            for(int r=0; r<9; r++) temp[r][i] = solution[r][band*3 + cols[i]];
        for(int i=0; i<3; i++)
            for(int r=0; r<9; r++) solution[r][band*3 + i] = temp[r][i];
    }
    
    for(int r=0; r<9; r++) {
        for(int c=0; c<9; c++) {
            board[r][c] = solution[r][c];
            fixed[r][c] = 1;
            error_cells[r][c] = 0;
            fog_cells[r][c] = 0;
        }
    }
    
    while(removal > 0) {
        int r = rand() % 9;
        int c = rand() % 9;
        if(board[r][c] != 0) {
            board[r][c] = 0;
            fixed[r][c] = 0;
            removal--;
        }
    }
    
    // Apply fog cells among empty cells
    if (fogCount > 0) {
        int emptyCells[81][2];
        int numEmpty = 0;
        for(int r=0; r<9; r++) {
            for(int c=0; c<9; c++) {
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
    
    for(int r=0; r<9; r++) {
        for(int c=0; c<9; c++) {
            for(int i=0; i<10; i++) notes[r][c][i] = 0;
            awarded[r][c] = 0;
        }
    }
    
    if (isRushMode) {
        elapsedTime = 180; // 180s countdown timer
    } else {
        elapsedTime = 0;
    }
    
    score = 0;
    timerActive = 1;
    gameActive = 1;
    UpdatePowerupButtons();
    SaveGameState();
}

void StartCampaignStage(HWND hwnd, int stage) {
    isCampaignMode = 1;
    isRushMode = 0;
    campaignStage = stage;
    if (campaignStage > 14) campaignStage = 14;
    magicWands = 3;
    shields = 1;
    shieldActive = 0;
    strikes = 0;
    
    int campaignRems[15] = {25, 30, 35, 40, 45, 48, 51, 54, 57, 60, 63, 65, 67, 69, 72};
    int campaignFogCount[15] = {0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 5, 6, 6, 7, 8};
    
    GenerateBoardEx(campaignRems[campaignStage], 0, campaignFogCount[campaignStage], 0);
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
    currentDiffIdx = 3;
    stats[3].played++;
    SaveStats();
    GenerateBoardEx(45, 0, 0, 1);
    sel_r = -1; sel_c = -1;
    UpdatePowerupButtons();
    InvalidateRect(hwnd, NULL, TRUE);
}

void CheckWin(HWND hwnd) {
    for(int r=0; r<9; r++)
        for(int c=0; c<9; c++)
            if(board[r][c] != solution[r][c]) return;
            
    timerActive = 0;
    PlaySudokuSound(6);
    
    if (isRushMode) {
        score += elapsedTime * 10 + 500; // Completion bonus for Rush
        stats[3].won++;
        if (stats[3].bestTime == -1 || elapsedTime > stats[3].bestTime) stats[3].bestTime = elapsedTime; // For Rush, higher time left is better
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
            if (campaignStage < 14) {
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
                sprintf(msg, "CONGRATULATIONS!!\nYou completed all 15 Stages of the KSudoku Campaign!\nFinal Score: %d", score);
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
                GenerateBoardEx(40, 0, 0, 0);
            }
            HWND hComboDifficulty = CreateWindowA("COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 10, 8, 70, 100, hwnd, (HMENU)4, NULL, NULL);
            SendMessageA(hComboDifficulty, CB_ADDSTRING, 0, (LPARAM)"Easy");
            SendMessageA(hComboDifficulty, CB_ADDSTRING, 0, (LPARAM)"Medium");
            SendMessageA(hComboDifficulty, CB_ADDSTRING, 0, (LPARAM)"Hard");
            SendMessageA(hComboDifficulty, CB_SETCURSEL, currentDiffIdx < 3 ? currentDiffIdx : 1, 0);
            
            hBtnNew = CreateWindowA("BUTTON", "New", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 85, 8, 40, 28, hwnd, (HMENU)1, NULL, NULL);
            hBtnCampaign = CreateWindowA("BUTTON", "Campaign", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 130, 8, 65, 28, hwnd, (HMENU)13, NULL, NULL);
            hBtnRush = CreateWindowA("BUTTON", "Rush", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 200, 8, 45, 28, hwnd, (HMENU)14, NULL, NULL);
            HWND hBtnDaily = CreateWindowA("BUTTON", "Daily", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 250, 8, 40, 28, hwnd, (HMENU)10, NULL, NULL);
            HWND hBtnStats = CreateWindowA("BUTTON", "Stats", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 295, 8, 45, 28, hwnd, (HMENU)6, NULL, NULL);
            hBtnSettings = CreateWindowA("BUTTON", "Settings", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 345, 8, 55, 28, hwnd, (HMENU)9, NULL, NULL);
            
            hBtnNotes = CreateWindowA("BUTTON", "Notes: OFF", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 40, 65, 28, hwnd, (HMENU)3, NULL, NULL);
            hBtnValidate = CreateWindowA("BUTTON", "Validate", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 80, 40, 55, 28, hwnd, (HMENU)2, NULL, NULL);
            hBtnHint = CreateWindowA("BUTTON", "Hint", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 140, 40, 40, 28, hwnd, (HMENU)5, NULL, NULL);
            hBtnMagic = CreateWindowA("BUTTON", "Wand (3)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 185, 40, 55, 28, hwnd, (HMENU)12, NULL, NULL);
            hBtnShield = CreateWindowA("BUTTON", "Shield (1)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 245, 40, 55, 28, hwnd, (HMENU)15, NULL, NULL);
            hBtnAutoFill = CreateWindowA("BUTTON", "Auto", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 305, 40, 40, 28, hwnd, (HMENU)11, NULL, NULL);
            hBtnUndo = CreateWindowA("BUTTON", "Undo", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 350, 40, 40, 28, hwnd, (HMENU)7, NULL, NULL);
            hBtnRedo = CreateWindowA("BUTTON", "Redo", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 395, 40, 40, 28, hwnd, (HMENU)8, NULL, NULL);

            hFont = CreateFontA(24, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            hFontSmall = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            SetTimer(hwnd, 1, 1000, NULL);
            UpdatePowerupButtons();
            break;
        }
        case WM_TIMER: {
            if(timerActive) {
                if (isRushMode) {
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
                GenerateBoardEx(removal, 0, 0, 0);
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
                    GenerateBoardEx(40, 1, 0, 0);
                    sel_r = -1; sel_c = -1;
                    PlaySudokuSound(1);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (LOWORD(wParam) == 6) { // Stats
                char msg[1024] = "";
                time_t t = time(NULL);
                struct tm* tm_info = localtime(&t);
                int todayStr = (tm_info->tm_year + 1900) * 10000 + (tm_info->tm_mon + 1) * 100 + tm_info->tm_mday;
                sprintf(msg, "[Daily Challenge & Progress]\nDaily Streak: %d | Completed Today: %s\nCampaign Max Stage: %d / 15\nWands Used: %d | Shields Used: %d\n\n",
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
                for(int r=0; r<9; r++) {
                    for(int c=0; c<9; c++) {
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
            } else if (LOWORD(wParam) == 5) { // Hint
                if (sel_r >= 0 && sel_c >= 0 && !fixed[sel_r][sel_c]) {
                    if (board[sel_r][sel_c] != solution[sel_r][sel_c]) {
                        PushState();
                        score = max(0, score - 150);
                        board[sel_r][sel_c] = solution[sel_r][sel_c];
                        awarded[sel_r][sel_c] = 1;
                        error_cells[sel_r][sel_c] = 0;
                        ClearFogAround(sel_r, sel_c);
                        PlaySudokuSound(3);
                        CheckWin(hwnd);
                    }
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
            } else if (LOWORD(wParam) == 11) { // Auto-fill Notes
                PushState();
                for(int r=0; r<9; r++) {
                    for(int c=0; c<9; c++) {
                        if(board[r][c] == 0) {
                            for(int i=1; i<=9; i++) notes[r][c][i] = 0;
                            for(int num=1; num<=9; num++) {
                                int invalid = 0;
                                for(int i=0; i<9; i++) {
                                    if(i != c && board[r][i] == num) invalid = 1;
                                    if(i != r && board[i][c] == num) invalid = 1;
                                }
                                int br = (r/3)*3, bc = (c/3)*3;
                                for(int i=0; i<3; i++) {
                                    for(int j=0; j<3; j++) {
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
                StartCampaignStage(hwnd, maxCampaignStage > 14 ? 14 : maxCampaignStage);
            } else if (LOWORD(wParam) == 12) { // Magic Wand
                if (magicWands > 0) {
                    int emptyCount = 0;
                    for(int r=0; r<9; r++)
                        for(int c=0; c<9; c++)
                            if(!fixed[r][c] && board[r][c] != solution[r][c]) emptyCount++;
                    if(emptyCount > 0) {
                        int target = rand() % emptyCount;
                        int idx = 0;
                        for(int r=0; r<9; r++) {
                            for(int c=0; c<9; c++) {
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
                if(wParam == VK_DOWN) sel_r = min(8, sel_r + 1);
                if(wParam == VK_LEFT) sel_c = max(0, sel_c - 1);
                if(wParam == VK_RIGHT) sel_c = min(8, sel_c + 1);
            }
            PlaySudokuSound(1);
            
            if(wParam >= '1' && wParam <= '9') {
                if(!fixed[sel_r][sel_c]) {
                    int num = wParam - '0';
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
                            for(int i=0; i<9; i++) {
                                if(i != sel_c && board[sel_r][i] == num) invalid = 1;
                                if(i != sel_r && board[i][sel_c] == num) invalid = 1;
                            }
                            int br = (sel_r/3)*3, bc = (sel_c/3)*3;
                            for(int r=0; r<3; r++) {
                                for(int c=0; c<3; c++) {
                                    if((br+r != sel_r || bc+c != sel_c) && board[br+r][bc+c] == num) invalid = 1;
                                }
                            }
                            
                            if(invalid) {
                                if (shieldActive) {
                                    shieldActive = 0; // Shield absorbs error!
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
                        for(int i=0; i<10; i++) notes[sel_r][sel_c][i] = 0;
                        PlaySudokuSound(2);
                    } else {
                        int hasNotes = 0;
                        for(int i=0; i<10; i++) if (notes[sel_r][sel_c][i]) hasNotes = 1;
                        if (hasNotes) {
                            PushState();
                            for(int i=0; i<10; i++) notes[sel_r][sel_c][i] = 0;
                            PlaySudokuSound(2);
                        }
                    }
                }
            } else if(wParam == 'Z' && (GetKeyState(VK_CONTROL) & 0x8000)) {
                Undo();
            } else if(wParam == 'Y' && (GetKeyState(VK_CONTROL) & 0x8000)) {
                Redo();
            } else if(wParam == 'N') {
                SendMessageA(hwnd, WM_COMMAND, 3, 0);
            }
            SaveGameState();
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            int start_x = 40, start_y = 75, cell_sz = 40;
            if(x >= start_x && x < start_x + 9*cell_sz && y >= start_y && y < start_y + 9*cell_sz) {
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
            
            // Background
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            HBRUSH hbg = CreateSolidBrush(themes[prefs.theme][T_BG]);
            FillRect(hdc, &clientRect, hbg);
            DeleteObject(hbg);
            
            HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, themes[prefs.theme][T_MUTABLE]);
            
            char info[128];
            if (isCampaignMode) {
                sprintf(info, "Stage: %d/15   Time: %02d:%02d   Score: %d", campaignStage+1, elapsedTime/60, elapsedTime%60, score);
                TextOutA(hdc, 40, 48, info, strlen(info));
            } else if (isRushMode) {
                sprintf(info, "RUSH MODE   Time Left: %02d:%02d   Score: %d", elapsedTime/60, elapsedTime%60, score);
                TextOutA(hdc, 40, 48, info, strlen(info));
            } else {
                sprintf(info, "Time: %02d:%02d   Score: %d", elapsedTime/60, elapsedTime%60, score);
                TextOutA(hdc, 40, 48, info, strlen(info));
            }
            
            int start_x = 40, start_y = 75, cell_sz = 40;
            
            int highlight_val = 0;
            if(sel_r >= 0 && sel_c >= 0 && board[sel_r][sel_c] != 0) {
                highlight_val = board[sel_r][sel_c];
            }
            
            // Draw cells
            for(int r=0; r<9; r++) {
                for(int c=0; c<9; c++) {
                    RECT rc = {start_x + c*cell_sz, start_y + r*cell_sz, start_x + (c+1)*cell_sz, start_y + (r+1)*cell_sz};
                    
                    HBRUSH cellBg = NULL;
                    if(r == sel_r && c == sel_c) {
                        cellBg = CreateSolidBrush(themes[prefs.theme][T_SEL]);
                    } else if(sel_r >= 0 && (r == sel_r || c == sel_c || (r/3 == sel_r/3 && c/3 == sel_c/3))) {
                        cellBg = CreateSolidBrush(themes[prefs.theme][T_HL]);
                    } else if(prefs.highlightSame && highlight_val && board[r][c] == highlight_val) {
                        cellBg = CreateSolidBrush(themes[prefs.theme][T_HL]);
                    }
                    
                    if(cellBg) {
                        FillRect(hdc, &rc, cellBg);
                        DeleteObject(cellBg);
                    }
                    
                    if (fog_cells[r][c] && board[r][c] == 0) {
                        // Shrouded / Fog Cell
                        HFONT oldFnt = (HFONT)SelectObject(hdc, hFont);
                        SetTextColor(hdc, RGB(148, 163, 184));
                        RECT textRc = rc;
                        textRc.top += 2;
                        DrawTextA(hdc, "?", -1, &textRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                        SelectObject(hdc, oldFnt);
                    } else if(board[r][c] != 0) {
                        char buf[2];
                        sprintf(buf, "%d", board[r][c]);
                        if(error_cells[r][c]) SetTextColor(hdc, themes[prefs.theme][T_ERR]);
                        else if(fixed[r][c]) SetTextColor(hdc, themes[prefs.theme][T_FIXED]); 
                        else SetTextColor(hdc, themes[prefs.theme][T_MUTABLE]);
                        
                        RECT textRc = rc;
                        textRc.top += 2;
                        DrawTextA(hdc, buf, -1, &textRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    } else {
                        int hasNotes = 0;
                        for(int i=1; i<=9; i++) if(notes[r][c][i]) hasNotes = 1;
                        if(hasNotes) {
                            HFONT oldFnt = (HFONT)SelectObject(hdc, hFontSmall);
                            SetTextColor(hdc, RGB(148, 163, 184));
                            for(int i=1; i<=9; i++) {
                                if(notes[r][c][i]) {
                                    int sub_r = (i-1) / 3;
                                    int sub_c = (i-1) % 3;
                                    RECT textRc = rc;
                                    textRc.left += sub_c * (cell_sz / 3);
                                    textRc.right = textRc.left + (cell_sz / 3);
                                    textRc.top += sub_r * (cell_sz / 3);
                                    textRc.bottom = textRc.top + (cell_sz / 3);
                                    
                                    char buf[2];
                                    sprintf(buf, "%d", i);
                                    DrawTextA(hdc, buf, -1, &textRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                                }
                            }
                            SelectObject(hdc, oldFnt);
                        }
                    }
                }
            }
            
            // Draw grid lines
            for(int i=0; i<=9; i++) {
                int thick = (i % 3 == 0);
                HPEN hPen = CreatePen(PS_SOLID, thick ? 3 : 1, themes[prefs.theme][thick ? T_GRID_THICK : T_GRID_THIN]);
                HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
                
                MoveToEx(hdc, start_x + i*cell_sz, start_y, NULL);
                LineTo(hdc, start_x + i*cell_sz, start_y + 9*cell_sz);
                
                MoveToEx(hdc, start_x, start_y + i*cell_sz, NULL);
                LineTo(hdc, start_x + 9*cell_sz, start_y + i*cell_sz);
                
                SelectObject(hdc, oldPen);
                DeleteObject(hPen);
            }
            
            SelectObject(hdc, oldFont);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            DeleteObject(hFont);
            DeleteObject(hFontSmall);
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
    
    RECT rc = {0, 0, 440, 480}; 
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

