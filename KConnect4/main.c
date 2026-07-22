#include <windows.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

const char g_szClassName[] = "KConnect4WindowClass";
#define ROWS 6
#define COLS 7

// Board values: 0=Empty, 1=Player 1 (Red), 2=Player 2 (Yellow), 3=Obstacle (Solid), 4=Crackable Block
int board[ROWS][COLS];
int currentPlayer = 1;
bool gameActive = true;
bool isDraw = false;

// Game modes: 0 = 2 Player, 1 = vs AI, 2 = Campaign, 3 = Time Attack
int gameMode = 1;
int aiDifficulty = 0;
int campaignStage = 1;

// Power-ups: 0 = Normal, 1 = Bomb (3x3 clear), 2 = Drill (column clear)
int selectedPowerup = 0;
int p1Bombs = 1, p2Bombs = 1;
int p1Drills = 1, p2Drills = 1;

// Time attack timer
int turnTimeLeftMs = 7000;

// Animation state
bool isAnimating = false;
int animPlayer = 0;
int animRow = -1;
int animCol = -1;
int animY = 0;
int animTargetY = 0;
int animType = 0; // 0=normal drop, 1=bomb drop, 2=drill drop

int hoverCol = -1;

int winCells[7][2];
int winCellCount = 0;

HWND hModeBtn, hBombBtn, hDrillBtn, hDiffSelect, hUndoBtn, hResetBtn, hMuteBtn, hSaveBtn, hLoadBtn, hHelpBtn;
bool isMuted = false;

void PlaySoundEffect(int type) {
    if (isMuted) return;
    if (type == 1) { // Drop
        Beep(400, 50);
    } else if (type == 2) { // Win
        Beep(400, 80); Beep(500, 80); Beep(600, 80); Beep(800, 120);
    } else if (type == 3) { // Lose
        Beep(300, 150); Beep(200, 200);
    } else if (type == 4) { // Invalid
        Beep(150, 100);
    } else if (type == 5) { // Draw
        Beep(300, 250);
    } else if (type == 6) { // Bomb / Explosion
        Beep(120, 150); Beep(80, 200);
    } else if (type == 7) { // Drill
        Beep(600, 50); Beep(450, 50); Beep(300, 100);
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
} GameStats;

GameStats stats = {0, 0, 0, 0, 0, 0, 1, 0, 0};

typedef struct {
    int board[ROWS][COLS];
    int currentPlayer;
    int p1Bombs, p2Bombs;
    int p1Drills, p2Drills;
    GameStats oldStats;
} MoveRecord;

MoveRecord moveHistory[ROWS * COLS * 2];
int historyCount = 0;

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
    int board[ROWS][COLS];
    int currentPlayer;
    bool gameActive;
    bool isDraw;
    int gameMode;
    int aiDifficulty;
    int campaignStage;
    int p1Bombs, p2Bombs;
    int p1Drills, p2Drills;
    int winCells[7][2];
    int winCellCount;
    MoveRecord moveHistory[ROWS * COLS * 2];
    int historyCount;
} GameState;

void UpdateDiffSelectUI();
void ResetGame();

void SaveGame() {
    GameState state;
    memcpy(state.board, board, sizeof(board));
    state.currentPlayer = currentPlayer;
    state.gameActive = gameActive;
    state.isDraw = isDraw;
    state.gameMode = gameMode;
    state.aiDifficulty = aiDifficulty;
    state.campaignStage = campaignStage;
    state.p1Bombs = p1Bombs;
    state.p2Bombs = p2Bombs;
    state.p1Drills = p1Drills;
    state.p2Drills = p2Drills;
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
            currentPlayer = state.currentPlayer;
            gameActive = state.gameActive;
            isDraw = state.isDraw;
            gameMode = state.gameMode;
            aiDifficulty = state.aiDifficulty;
            campaignStage = state.campaignStage;
            p1Bombs = state.p1Bombs;
            p2Bombs = state.p2Bombs;
            p1Drills = state.p1Drills;
            p2Drills = state.p2Drills;
            memcpy(winCells, state.winCells, sizeof(winCells));
            winCellCount = state.winCellCount;
            memcpy(moveHistory, state.moveHistory, sizeof(moveHistory));
            historyCount = state.historyCount;
            
            if (gameMode == 0) SetWindowText(hModeBtn, "Mode: 2 Player");
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
            
            InvalidateRect(hwnd, NULL, TRUE);
            MessageBox(hwnd, "Game Loaded Successfully!", "KConnect4", MB_OK | MB_ICONINFORMATION);
        }
        fclose(f);
    } else {
        MessageBox(hwnd, "No saved game file found.", "KConnect4", MB_OK | MB_ICONWARNING);
    }
}

void SetupCampaignStage(int stage) {
    for(int r=0; r<ROWS; r++)
        for(int c=0; c<COLS; c++)
            board[r][c] = 0;
            
    aiDifficulty = 1;
    if (stage >= 4) aiDifficulty = 2;
    if (stage >= 8) aiDifficulty = 3;

    if (stage == 2) { board[5][0] = 3; board[5][6] = 3; }
    else if (stage == 3) { board[5][3] = 3; board[4][3] = 3; }
    else if (stage == 4) { board[5][2] = 3; board[5][4] = 3; board[4][3] = 3; }
    else if (stage == 5) { board[5][1] = 3; board[5][3] = 3; board[5][5] = 3; board[4][2] = 3; board[4][4] = 3; }
    else if (stage == 6) { board[3][1] = 3; board[3][5] = 3; }
    else if (stage == 7) { board[2][2] = 3; board[2][3] = 3; board[2][4] = 3; }
    else if (stage == 8) { board[5][3] = 3; board[4][3] = 3; board[3][3] = 3; board[2][3] = 3; }
    else if (stage == 9) { board[5][1] = 3; board[4][1] = 3; board[5][5] = 3; board[4][5] = 3; board[2][3] = 3; }
    else if (stage == 10) { board[5][0]=3; board[5][6]=3; board[4][2]=3; board[4][4]=3; board[2][1]=3; board[2][5]=3; }
    else if (stage == 11) { board[5][2]=4; board[5][4]=4; board[4][3]=3; }
    else if (stage == 12) { board[4][3]=4; board[3][2]=3; board[3][4]=3; board[2][3]=4; }
    else if (stage == 13) { board[5][2]=4; board[5][4]=4; board[4][2]=3; board[4][4]=3; board[1][3]=4; }
    else if (stage == 14) { board[5][1]=3; board[5][5]=3; board[3][3]=4; board[1][2]=3; board[1][4]=3; }
    else if (stage == 15) { board[5][0]=3; board[5][6]=3; board[4][3]=4; board[3][1]=3; board[3][5]=3; board[2][3]=4; }
}

void ResetGame() {
    for(int r=0; r<ROWS; r++)
        for(int c=0; c<COLS; c++)
            board[r][c] = 0;
            
    if (gameMode == 2) {
        SetupCampaignStage(campaignStage);
    }

    currentPlayer = 1;
    gameActive = true;
    isDraw = false;
    isAnimating = false;
    winCellCount = 0;
    historyCount = 0;
    selectedPowerup = 0;
    p1Bombs = 1; p2Bombs = 1;
    p1Drills = 1; p2Drills = 1;
    turnTimeLeftMs = 7000;
}

void UpdateBombDrillButtons() {
    char bBuf[32], dBuf[32];
    int bCount = (currentPlayer == 1) ? p1Bombs : p2Bombs;
    int dCount = (currentPlayer == 1) ? p1Drills : p2Drills;
    if (gameMode > 0 && currentPlayer == 2) { bCount = 0; dCount = 0; }

    if (selectedPowerup == 1) wsprintf(bBuf, "[Bomb]");
    else wsprintf(bBuf, "Bomb (%d)", bCount);

    if (selectedPowerup == 2) wsprintf(dBuf, "[Drill]");
    else wsprintf(dBuf, "Drill (%d)", dCount);

    SetWindowText(hBombBtn, bBuf);
    SetWindowText(hDrillBtn, dBuf);
}

void UpdateDiffSelectUI() {
    SendMessage(hDiffSelect, CB_RESETCONTENT, 0, 0);
    if (gameMode == 1 || gameMode == 3) {
        ShowWindow(hDiffSelect, SW_SHOW);
        SendMessage(hDiffSelect, CB_ADDSTRING, 0, (LPARAM)"Easy");
        SendMessage(hDiffSelect, CB_ADDSTRING, 0, (LPARAM)"Medium");
        SendMessage(hDiffSelect, CB_ADDSTRING, 0, (LPARAM)"Hard");
        SendMessage(hDiffSelect, CB_ADDSTRING, 0, (LPARAM)"Expert");
        SendMessage(hDiffSelect, CB_SETCURSEL, aiDifficulty, 0);
    } else if (gameMode == 2) {
        ShowWindow(hDiffSelect, SW_SHOW);
        for(int i=1; i<=15; i++) {
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
        int tempCells[7][2];
        tempCells[0][0] = r;
        tempCells[0][1] = c;
        for(int i=1; i<4; i++) {
            int nr = r + dirs[d][0]*i;
            int nc = c + dirs[d][1]*i;
            if(nr>=0 && nr<ROWS && nc>=0 && nc<COLS && board[nr][nc]==p) {
                tempCells[count][0] = nr;
                tempCells[count][1] = nc;
                count++;
            }
            else break;
        }
        for(int i=1; i<4; i++) {
            int nr = r - dirs[d][0]*i;
            int nc = c - dirs[d][1]*i;
            if(nr>=0 && nr<ROWS && nc>=0 && nc<COLS && board[nr][nc]==p) {
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
    for(int c=0; c<COLS; c++) {
        if(board[0][c] == 0) return false;
    }
    return true;
}

bool checkWinBoard(int b[ROWS][COLS], int p) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS - 3; c++) {
            if (b[r][c] == p && b[r][c+1] == p && b[r][c+2] == p && b[r][c+3] == p) return true;
        }
    }
    for (int c = 0; c < COLS; c++) {
        for (int r = 0; r < ROWS - 3; r++) {
            if (b[r][c] == p && b[r+1][c] == p && b[r+2][c] == p && b[r+3][c] == p) return true;
        }
    }
    for (int r = 0; r < ROWS - 3; r++) {
        for (int c = 0; c < COLS - 3; c++) {
            if (b[r][c] == p && b[r+1][c+1] == p && b[r+2][c+2] == p && b[r+3][c+3] == p) return true;
        }
    }
    for (int r = 3; r < ROWS; r++) {
        for (int c = 0; c < COLS - 3; c++) {
            if (b[r][c] == p && b[r-1][c+1] == p && b[r-2][c+2] == p && b[r-3][c+3] == p) return true;
        }
    }
    return false;
}

void ApplyGravity() {
    bool changed = true;
    while(changed) {
        changed = false;
        for (int c = 0; c < COLS; c++) {
            for (int r = ROWS - 2; r >= 0; r--) {
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
    for (int r = ROWS - 1; r >= 0; r--) {
        if (board[r][col] == 0) {
            board[r][col] = p;
            int tempWin[7][2];
            int tempCount = winCellCount;
            for(int i=0; i<7; i++) { tempWin[i][0]=winCells[i][0]; tempWin[i][1]=winCells[i][1]; }
            bool win = CheckWin(r, col, p);
            for(int i=0; i<7; i++) { winCells[i][0]=tempWin[i][0]; winCells[i][1]=tempWin[i][1]; }
            winCellCount = tempCount;
            board[r][col] = 0;
            return win;
        }
    }
    return false;
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
    else if (pc == 3 && ec == 1) score += 5;
    else if (pc == 2 && ec == 2) score += 2;
    if (oc == 3 && ec == 1) score -= 4;
    return score;
}

int scoreBoard(int b[ROWS][COLS], int piece) {
    int score = 0;
    int centerCount = 0;
    for (int r=0; r<ROWS; r++) if (b[r][3] == piece) centerCount++;
    score += centerCount * 3;

    for (int r=0; r<ROWS; r++) {
        for (int c=0; c<COLS-3; c++) {
            int w[4] = {b[r][c], b[r][c+1], b[r][c+2], b[r][c+3]};
            score += evaluateWindow(w, piece);
        }
    }
    for (int c=0; c<COLS; c++) {
        for (int r=0; r<ROWS-3; r++) {
            int w[4] = {b[r][c], b[r+1][c], b[r+2][c], b[r+3][c]};
            score += evaluateWindow(w, piece);
        }
    }
    for (int r=0; r<ROWS-3; r++) {
        for (int c=0; c<COLS-3; c++) {
            int w[4] = {b[r][c], b[r+1][c+1], b[r+2][c+2], b[r+3][c+3]};
            score += evaluateWindow(w, piece);
        }
    }
    for (int r=3; r<ROWS; r++) {
        for (int c=0; c<COLS-3; c++) {
            int w[4] = {b[r][c], b[r-1][c+1], b[r-2][c+2], b[r-3][c+3]};
            score += evaluateWindow(w, piece);
        }
    }
    return score;
}

int getValidLocations(int b[ROWS][COLS], int locs[COLS]) {
    int count = 0;
    int pref[COLS] = {3, 2, 4, 1, 5, 0, 6};
    for(int i=0; i<COLS; i++) {
        if(b[0][pref[i]] == 0) locs[count++] = pref[i];
    }
    return count;
}

bool isTerminalNode(int b[ROWS][COLS]) {
    if(checkWinBoard(b, 1) || checkWinBoard(b, 2)) return true;
    for(int c=0; c<COLS; c++) if(b[0][c] == 0) return false;
    return true;
}

int getNextOpenRow(int b[ROWS][COLS], int c) {
    for (int r = ROWS - 1; r >= 0; r--) {
        if (b[r][c] == 0) return r;
    }
    return -1;
}

typedef struct {
    int col;
    int score;
} MMResult;

MMResult minimax(int b[ROWS][COLS], int depth, int alpha, int beta, bool maximizingPlayer) {
    int validLocations[COLS];
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

void ExecuteDrop(HWND hwnd, int col, int player, int powerType) {
    for (int r = ROWS - 1; r >= 0; r--) {
        if (board[r][col] == 0) {
            PlaySoundEffect(powerType == 1 ? 6 : (powerType == 2 ? 7 : 1));
            
            // Record history
            memcpy(moveHistory[historyCount].board, board, sizeof(board));
            moveHistory[historyCount].currentPlayer = currentPlayer;
            moveHistory[historyCount].p1Bombs = p1Bombs;
            moveHistory[historyCount].p2Bombs = p2Bombs;
            moveHistory[historyCount].p1Drills = p1Drills;
            moveHistory[historyCount].p2Drills = p2Drills;
            moveHistory[historyCount].oldStats = stats;
            historyCount++;

            if (powerType == 1) {
                if (player == 1) p1Bombs--; else p2Bombs--;
                stats.totalBombs++;
            } else if (powerType == 2) {
                if (player == 1) p1Drills--; else p2Drills--;
                stats.totalDrills++;
            }

            board[r][col] = player;
            animPlayer = player;
            animRow = r;
            animCol = col;
            animY = 50;
            animTargetY = 50 + 5 + r * 45;
            animType = powerType;
            isAnimating = true;
            selectedPowerup = 0;
            UpdateBombDrillButtons();
            
            SetTimer(hwnd, 2, 16, NULL);
            break;
        }
    }
}

void AIMove(HWND hwnd) {
    if (!gameActive || (gameMode == 0)) return;
    
    int bestCol = -1;
    int powerType = 0;
    
    if (aiDifficulty == 0) {
        int available[COLS];
        int count = 0;
        for(int c=0; c<COLS; c++) {
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
        if (aiDifficulty == 1) depth = 3;
        else if (aiDifficulty == 2) depth = 5;
        else if (aiDifficulty == 3) depth = 6;
        
        MMResult res = minimax(board, depth, -20000000, 20000000, true);
        bestCol = res.col;
        if (bestCol == -1) {
            int validLocations[COLS];
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
        for(int dr=-1; dr<=1; dr++) {
            for(int dc=-1; dc<=1; dc++) {
                int nr = animRow + dr, nc = animCol + dc;
                if(nr>=0 && nr<ROWS && nc>=0 && nc<COLS && (board[nr][nc] == 1 || board[nr][nc] == 2 || board[nr][nc] == 4)) {
                    board[nr][nc] = 0;
                }
            }
        }
        ApplyGravity();
    } else if (animType == 2) { // Drill
        for(int r=0; r<ROWS; r++) {
            if(board[r][animCol] == 1 || board[r][animCol] == 2 || board[r][animCol] == 4) {
                board[r][animCol] = 0;
            }
        }
        ApplyGravity();
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
            else PlaySoundEffect(2);
            gameActive = false;
            if(winner == 1) stats.redWins++;
            else stats.yellowWins++;
            
            if(stats.lastWinner == winner) stats.streak++;
            else stats.streak = 1;
            stats.lastWinner = winner;
            if(stats.streak > stats.bestStreak) stats.bestStreak = stats.streak;
            
            if (gameMode == 2 && winner == 1 && campaignStage == stats.maxCampaignStage && campaignStage < 15) {
                stats.maxCampaignStage++;
                UpdateDiffSelectUI();
            }
        }
        SaveStats();
    } else if (CheckWin(animRow, animCol, animPlayer)) {
        if (gameMode > 0 && animPlayer == 2) PlaySoundEffect(3);
        else PlaySoundEffect(2);
        gameActive = false;
        if(animPlayer == 1) stats.redWins++;
        else stats.yellowWins++;
        
        if(stats.lastWinner == animPlayer) stats.streak++;
        else stats.streak = 1;
        stats.lastWinner = animPlayer;
        if(stats.streak > stats.bestStreak) stats.bestStreak = stats.streak;
        
        if (gameMode == 2 && animPlayer == 1 && campaignStage == stats.maxCampaignStage && campaignStage < 15) {
            stats.maxCampaignStage++;
            UpdateDiffSelectUI();
        }
        SaveStats();
    } else if (CheckDraw()) {
        PlaySoundEffect(5);
        gameActive = false;
        isDraw = true;
        stats.draws++;
        stats.streak = 0;
        stats.lastWinner = 0;
        SaveStats();
    } else {
        currentPlayer = (animPlayer == 1) ? 2 : 1;
        turnTimeLeftMs = 7000;
        UpdateBombDrillButtons();
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
            hModeBtn = CreateWindow("BUTTON", "Mode: vs AI", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 335, 100, 30, hwnd, (HMENU)1, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hBombBtn = CreateWindow("BUTTON", "Bomb (1)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 115, 335, 80, 30, hwnd, (HMENU)9, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hDrillBtn = CreateWindow("BUTTON", "Drill (1)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 200, 335, 80, 30, hwnd, (HMENU)10, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hDiffSelect = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 285, 337, 95, 200, hwnd, (HMENU)4, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            
            hUndoBtn = CreateWindow("BUTTON", "Undo", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 372, 70, 30, hwnd, (HMENU)3, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hResetBtn = CreateWindow("BUTTON", "Reset", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 85, 372, 70, 30, hwnd, (HMENU)2, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hMuteBtn = CreateWindow("BUTTON", "Mute", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 160, 372, 70, 30, hwnd, (HMENU)5, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hSaveBtn = CreateWindow("BUTTON", "Save", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 235, 372, 70, 30, hwnd, (HMENU)6, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hLoadBtn = CreateWindow("BUTTON", "Load", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 310, 372, 70, 30, hwnd, (HMENU)7, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            
            hHelpBtn = CreateWindow("BUTTON", "Help", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 408, 70, 30, hwnd, (HMENU)8, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            
            UpdateDiffSelectUI();
            ResetGame();
            break;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == 1) {
                gameMode = (gameMode + 1) % 4;
                if (gameMode == 0) SetWindowText(hModeBtn, "Mode: 2 Player");
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
                    if (selectedPowerup == 1) selectedPowerup = 0;
                    else selectedPowerup = 1;
                    UpdateBombDrillButtons();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (LOWORD(wParam) == 10) { // Drill
                if (!gameActive || (gameMode > 0 && currentPlayer == 2)) break;
                int dCount = (currentPlayer == 1) ? p1Drills : p2Drills;
                if (dCount > 0) {
                    if (selectedPowerup == 2) selectedPowerup = 0;
                    else selectedPowerup = 2;
                    UpdateBombDrillButtons();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (LOWORD(wParam) == 4 && HIWORD(wParam) == CBN_SELCHANGE) {
                int sel = SendMessage(hDiffSelect, CB_GETCURSEL, 0, 0);
                if (gameMode == 1 || gameMode == 3) {
                    aiDifficulty = sel;
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
                MessageBox(hwnd, "KConnect4 - Game Rules & Power-Ups\n\n"
                    "Modes:\n"
                    "- 2 Player: Play locally against a friend.\n"
                    "- vs AI: Play against configurable AI (Easy, Medium, Hard, Expert).\n"
                    "- Campaign: Progress through 15 handcrafted stages with obstacles!\n"
                    "- Speed: Time Attack mode with 7-second turn timers.\n\n"
                    "Power-Ups:\n"
                    "- Bomb: Clears 3x3 surrounding grid and triggers gravity drop.\n"
                    "- Drill: Clears entire column of discs and crackable blocks!\n\n"
                    "Controls:\n"
                    "- Click any column to drop a piece or use selected Power-Up.\n"
                    "- Undo move anytime.", 
                    "Help & Information", MB_OK | MB_ICONINFORMATION);
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
                        currentPlayer = m.currentPlayer;
                        p1Bombs = m.p1Bombs;
                        p2Bombs = m.p2Bombs;
                        p1Drills = m.p1Drills;
                        p2Drills = m.p2Drills;
                        stats = m.oldStats;
                    }
                }
                
                SaveStats();
                gameActive = true;
                isDraw = false;
                winCellCount = 0;
                turnTimeLeftMs = 7000;
                UpdateBombDrillButtons();
                if (gameMode == 3) SetTimer(hwnd, 3, 100, NULL);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
            
        case WM_TIMER:
            if (wParam == 1) { // AI turn timer
                KillTimer(hwnd, 1);
                AIMove(hwnd);
            } else if (wParam == 2) { // Drop animation timer
                animY += 25;
                if (animY >= animTargetY) {
                    animY = animTargetY;
                    isAnimating = false;
                    KillTimer(hwnd, 2);
                    FinishTurnEffects(hwnd);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 3) { // Speed mode countdown timer
                if (gameMode == 3 && gameActive && !isAnimating) {
                    if (!(gameMode > 0 && currentPlayer == 2)) {
                        turnTimeLeftMs -= 100;
                        if (turnTimeLeftMs <= 0) {
                            turnTimeLeftMs = 7000;
                            // Auto drop in random open column
                            int validLocations[COLS];
                            int count = getValidLocations(board, validLocations);
                            if (count > 0) {
                                int col = validLocations[rand() % count];
                                ExecuteDrop(hwnd, col, currentPlayer, 0);
                            }
                        }
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                }
            }
            break;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            HBRUSH bg = CreateSolidBrush(RGB(18, 18, 18));
            FillRect(hdc, &rect, bg);
            DeleteObject(bg);
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(224, 224, 224));
            
            char statusText[64];
            if (!gameActive) {
                if (isDraw) wsprintf(statusText, "It's a Draw! Click to reset.");
                else wsprintf(statusText, "Player %d (%s) Wins!", currentPlayer, (currentPlayer == 1) ? "Red" : "Yellow");
            } else {
                wsprintf(statusText, "Player %d's turn (%s)", currentPlayer, (currentPlayer == 1) ? "Red" : "Yellow");
            }
            TextOut(hdc, 20, 15, statusText, lstrlen(statusText));
            
            // Draw Time Attack bar if Speed mode
            if (gameMode == 3 && gameActive) {
                int barWidth = (315 * turnTimeLeftMs) / 7000;
                RECT timerBg = {20, 38, 335, 44};
                HBRUSH tBg = CreateSolidBrush(RGB(40, 40, 40));
                FillRect(hdc, &timerBg, tBg);
                DeleteObject(tBg);
                
                RECT timerFg = {20, 38, 20 + barWidth, 44};
                HBRUSH tFg = CreateSolidBrush((turnTimeLeftMs < 2000) ? RGB(255, 82, 82) : RGB(76, 175, 80));
                FillRect(hdc, &timerFg, tFg);
                DeleteObject(tFg);
            }
            
            char statsStr[128];
            wsprintf(statsStr, "Wins: Red %d, Yellow %d | Draws: %d | Streak: %d (Best: %d) | Max Stage: %d",
                     stats.redWins, stats.yellowWins, stats.draws, stats.streak, stats.bestStreak, stats.maxCampaignStage);
            TextOut(hdc, 10, 445, statsStr, lstrlen(statsStr));
            
            // Draw board background
            HBRUSH boardBg = CreateSolidBrush(RGB(31, 66, 135));
            RECT boardRect = {20, 50, 20 + COLS * 45 + 5, 50 + ROWS * 45 + 5};
            FillRect(hdc, &boardRect, boardBg);
            DeleteObject(boardBg);
            
            HRGN hRgn = CreateRectRgn(20, 50, 20 + COLS * 45 + 5, 50 + ROWS * 45 + 5);
            SelectClipRgn(hdc, hRgn);

            HBRUSH emptyCell = CreateSolidBrush(RGB(18, 18, 18));
            HBRUSH p1Cell = CreateSolidBrush(RGB(255, 82, 82));
            HBRUSH p2Cell = CreateSolidBrush(RGB(255, 235, 59));
            HBRUSH obsCell = CreateSolidBrush(RGB(85, 85, 85));
            HBRUSH crackCell = CreateSolidBrush(RGB(139, 69, 19));
            HBRUSH hoverP1 = CreateSolidBrush(RGB(120, 40, 40));
            HBRUSH hoverP2 = CreateSolidBrush(RGB(120, 110, 20));
            HBRUSH hoverBomb = CreateSolidBrush(RGB(50, 50, 50));
            HBRUSH hoverDrill = CreateSolidBrush(RGB(0, 150, 200));
            HPEN hlPen = CreatePen(PS_SOLID, 3, RGB(255, 255, 255));
            HPEN nullPen = GetStockObject(NULL_PEN);

            int hoverRow = -1;
            if (hoverCol != -1 && !isAnimating && gameActive && !(gameMode > 0 && currentPlayer == 2)) {
                for (int r = ROWS - 1; r >= 0; r--) {
                    if (board[r][hoverCol] == 0) {
                        hoverRow = r;
                        break;
                    }
                }
            }
            
            for (int r = 0; r < ROWS; r++) {
                for (int c = 0; c < COLS; c++) {
                    int x = 20 + 5 + c * 45;
                    int y = 50 + 5 + r * 45;
                    
                    bool isWinCell = false;
                    for(int i=0; i<winCellCount; i++) {
                        if(winCells[i][0] == r && winCells[i][1] == c) {
                            isWinCell = true;
                            break;
                        }
                    }

                    if (isAnimating && r == animRow && c == animCol) {
                        SelectObject(hdc, emptyCell);
                    } else if (board[r][c] == 1) SelectObject(hdc, p1Cell);
                    else if (board[r][c] == 2) SelectObject(hdc, p2Cell);
                    else if (board[r][c] == 3) SelectObject(hdc, obsCell);
                    else if (board[r][c] == 4) SelectObject(hdc, crackCell);
                    else if (r == hoverRow && c == hoverCol) {
                        if (selectedPowerup == 1) SelectObject(hdc, hoverBomb);
                        else if (selectedPowerup == 2) SelectObject(hdc, hoverDrill);
                        else SelectObject(hdc, currentPlayer == 1 ? hoverP1 : hoverP2);
                    } else SelectObject(hdc, emptyCell);
                    
                    if (isWinCell) {
                        SelectObject(hdc, hlPen);
                    } else {
                        SelectObject(hdc, nullPen);
                    }
                    
                    Ellipse(hdc, x, y, x + 40, y + 40);
                }
            }
            
            if (isAnimating) {
                int x = 20 + 5 + animCol * 45;
                if (animType == 1) SelectObject(hdc, hoverBomb);
                else if (animType == 2) SelectObject(hdc, hoverDrill);
                else if (animPlayer == 1) SelectObject(hdc, p1Cell);
                else SelectObject(hdc, p2Cell);
                
                SelectObject(hdc, nullPen);
                Ellipse(hdc, x, animY, x + 40, animY + 40);
            }
            
            SelectClipRgn(hdc, NULL);
            DeleteObject(hRgn);

            SelectObject(hdc, nullPen);
            DeleteObject(emptyCell);
            DeleteObject(p1Cell);
            DeleteObject(p2Cell);
            DeleteObject(obsCell);
            DeleteObject(crackCell);
            DeleteObject(hoverP1);
            DeleteObject(hoverP2);
            DeleteObject(hoverBomb);
            DeleteObject(hoverDrill);
            DeleteObject(hlPen);
            
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
            
            if (xPos >= 25 && xPos <= 25 + COLS * 45 && yPos >= 55 && yPos <= 55 + ROWS * 45) {
                int c = (xPos - 25) / 45;
                if (c >= 0 && c < COLS) {
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
            
            if (xPos >= 25 && xPos <= 25 + COLS * 45 && yPos >= 55 && yPos <= 55 + ROWS * 45) {
                int c = (xPos - 25) / 45;
                if (c >= 0 && c < COLS) {
                    if (board[0][c] == 0) {
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

    hwnd = CreateWindowEx(
        0, g_szClassName, "KConnect4",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 420, 530,
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
