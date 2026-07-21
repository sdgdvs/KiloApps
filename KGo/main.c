#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ID_BTN_PASS 101
#define ID_BTN_RESIGN 102
#define ID_BTN_NEW 103
#define ID_CB_SIZE 104
#define ID_BTN_SCORE 105
#define ID_CB_AI 106
#define ID_BTN_SAVE 107
#define ID_BTN_LOAD 108
#define ID_BTN_UNDO 109
#define ID_BTN_REDO 110
#define ID_BTN_STATS 111
#define ID_CB_DIFFICULTY 112
#define ID_BTN_HELP 113
#define ID_BTN_CAMPAIGN 114

typedef struct {
    int size;
    int handicap;
    int difficulty;
} CampaignStage;
CampaignStage campaign[10] = {
    {9, 5, 0}, {9, 3, 0}, {9, 0, 1},
    {13, 5, 1}, {13, 3, 1}, {13, 0, 2},
    {19, 9, 1}, {19, 5, 2}, {19, 3, 2}, {19, 0, 2}
};
int currentCampaignStage = -1;


typedef struct {
    int played;
    int aiWins;
    int aiLosses;
    int localB;
    int localW;
} GameStats;
GameStats stats = {0};

void LoadStats() {
    FILE *f = fopen("kgo_stats.dat", "rb");
    if (f) {
        fread(&stats, sizeof(GameStats), 1, f);
        fclose(f);
    }
}
void SaveStats() {
    FILE *f = fopen("kgo_stats.dat", "wb");
    if (f) {
        fwrite(&stats, sizeof(GameStats), 1, f);
        fclose(f);
    }
}
void RecordGameEnd(int winnerPlayer, HWND hwnd) {
    stats.played++;
    if (SendMessage(GetDlgItem(hwnd, ID_CB_AI), BM_GETCHECK, 0, 0) == BST_CHECKED) {
        if (winnerPlayer == 1) stats.aiWins++;
        else stats.aiLosses++;
    } else {
        if (winnerPlayer == 1) stats.localB++;
        else stats.localW++;
    }
    SaveStats();
}

typedef struct {
    char board[19][19];
    char prevBoard[19][19];
    int currentPlayer;
    int captures[3];
} GameState;

#define MAX_HISTORY 500
GameState undoStack[MAX_HISTORY];
int undoCount = 0;
GameState redoStack[MAX_HISTORY];
int redoCount = 0;

int boardSize = 9;
char board[19][19] = {0}; // 0 = empty, 1 = black, 2 = white
char prevBoard[19][19] = {0};
int currentPlayer = 1;
int captures[3] = {0}; // 1 = black, 2 = white
int hoverX = -1, hoverY = -1;
int animX = -1, animY = -1;
int animRadius = 13;
POINT capturedAnimStones[19*19];
int capturedAnimColor[19*19];
int capturedAnimCount = 0;
int captureAnimRadius = 0;

DWORD WINAPI PlaySoundThread(LPVOID lpParam) {
    int type = (int)(intptr_t)lpParam;
    if (type == 1) {
        Beep(300, 80);
    } else if (type == 2) {
        Beep(150, 100);
        Beep(400, 100);
    } else if (type == 3) {
        Beep(440, 150);
        Beep(554, 150);
        Beep(659, 400);
    }
    return 0;
}
void PlayGameSound(int type) {
    CreateThread(NULL, 0, PlaySoundThread, (LPVOID)(intptr_t)type, 0, NULL);
}

void CopyBoard(char dst[19][19], char src[19][19]) {
    memcpy(dst, src, sizeof(char) * 19 * 19);
}

void PushUndo(char bBackup[19][19], int cBackup[3], int pBackup) {
    if (undoCount < MAX_HISTORY) {
        CopyBoard(undoStack[undoCount].board, bBackup);
        CopyBoard(undoStack[undoCount].prevBoard, prevBoard);
        undoStack[undoCount].currentPlayer = pBackup;
        undoStack[undoCount].captures[1] = cBackup[1];
        undoStack[undoCount].captures[2] = cBackup[2];
        undoCount++;
    } else {
        for (int i = 1; i < MAX_HISTORY; i++) {
            undoStack[i-1] = undoStack[i];
        }
        CopyBoard(undoStack[MAX_HISTORY-1].board, bBackup);
        CopyBoard(undoStack[MAX_HISTORY-1].prevBoard, prevBoard);
        undoStack[MAX_HISTORY-1].currentPlayer = pBackup;
        undoStack[MAX_HISTORY-1].captures[1] = cBackup[1];
        undoStack[MAX_HISTORY-1].captures[2] = cBackup[2];
    }
    redoCount = 0;
}

void DoUndo(HWND hwnd) {
    if (undoCount > 0) {
        if (redoCount < MAX_HISTORY) {
            CopyBoard(redoStack[redoCount].board, board);
            CopyBoard(redoStack[redoCount].prevBoard, prevBoard);
            redoStack[redoCount].currentPlayer = currentPlayer;
            redoStack[redoCount].captures[1] = captures[1];
            redoStack[redoCount].captures[2] = captures[2];
            redoCount++;
        }
        undoCount--;
        CopyBoard(board, undoStack[undoCount].board);
        CopyBoard(prevBoard, undoStack[undoCount].prevBoard);
        currentPlayer = undoStack[undoCount].currentPlayer;
        captures[1] = undoStack[undoCount].captures[1];
        captures[2] = undoStack[undoCount].captures[2];
        capturedAnimCount = 0;
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void DoRedo(HWND hwnd) {
    if (redoCount > 0) {
        if (undoCount < MAX_HISTORY) {
            CopyBoard(undoStack[undoCount].board, board);
            CopyBoard(undoStack[undoCount].prevBoard, prevBoard);
            undoStack[undoCount].currentPlayer = currentPlayer;
            undoStack[undoCount].captures[1] = captures[1];
            undoStack[undoCount].captures[2] = captures[2];
            undoCount++;
        }
        redoCount--;
        CopyBoard(board, redoStack[redoCount].board);
        CopyBoard(prevBoard, redoStack[redoCount].prevBoard);
        currentPlayer = redoStack[redoCount].currentPlayer;
        captures[1] = redoStack[redoCount].captures[1];
        captures[2] = redoStack[redoCount].captures[2];
        capturedAnimCount = 0;
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

int GetLiberties(int x, int y, int color, char visited[19][19]) {
    if (x < 0 || x >= boardSize || y < 0 || y >= boardSize) return 0;
    if (visited[y][x]) return 0;
    visited[y][x] = 1;
    
    if (board[y][x] == 0) return 1;
    if (board[y][x] != color) return 0;
    
    return GetLiberties(x-1, y, color, visited) +
           GetLiberties(x+1, y, color, visited) +
           GetLiberties(x, y-1, color, visited) +
           GetLiberties(x, y+1, color, visited);
}

void GetGroup(int x, int y, int color, char visited[19][19], POINT group[], int *groupSize) {
    if (x < 0 || x >= boardSize || y < 0 || y >= boardSize) return;
    if (visited[y][x]) return;
    if (board[y][x] != color) return;
    
    visited[y][x] = 1;
    group[*groupSize].x = x;
    group[*groupSize].y = y;
    (*groupSize)++;
    
    GetGroup(x-1, y, color, visited, group, groupSize);
    GetGroup(x+1, y, color, visited, group, groupSize);
    GetGroup(x, y-1, color, visited, group, groupSize);
    GetGroup(x, y+1, color, visited, group, groupSize);
}

int CheckCaptures(int color, int realMove) {
    int totalCaptured = 0;
    for (int y = 0; y < boardSize; y++) {
        for (int x = 0; x < boardSize; x++) {
            if (board[y][x] == color) {
                char visited[19][19] = {0};
                if (GetLiberties(x, y, color, visited) == 0) {
                    char groupVisited[19][19] = {0};
                    POINT group[19*19];
                    int groupSize = 0;
                    GetGroup(x, y, color, groupVisited, group, &groupSize);
                    for (int i = 0; i < groupSize; i++) {
                        board[group[i].y][group[i].x] = 0;
                        if (realMove) {
                            capturedAnimStones[capturedAnimCount].x = group[i].x;
                            capturedAnimStones[capturedAnimCount].y = group[i].y;
                            capturedAnimColor[capturedAnimCount] = color;
                            capturedAnimCount++;
                        }
                        totalCaptured++;
                    }
                }
            }
        }
    }
    if (totalCaptured > 0) {
        if (color == 1) captures[2] += totalCaptured;
        else captures[1] += totalCaptured;
    }
    return totalCaptured;
}

void PlaceStone(HWND hwnd, int x, int y) {
    if (board[y][x] != 0) return;
    
    char boardBackup[19][19];
    CopyBoard(boardBackup, board);
    int capBackup[3] = { captures[0], captures[1], captures[2] };
    
    board[y][x] = (char)currentPlayer;
    
    int opp = currentPlayer == 1 ? 2 : 1;
    capturedAnimCount = 0;
    int caps = CheckCaptures(opp, 1);
    
    char visited[19][19] = {0};
    if (GetLiberties(x, y, currentPlayer, visited) == 0) {
        CopyBoard(board, boardBackup);
        captures[1] = capBackup[1];
        captures[2] = capBackup[2];
        if (currentPlayer == 1) MessageBox(hwnd, "Suicide move is not allowed.", "Invalid Move", MB_OK);
        return;
    }
    
    if (memcmp(board, prevBoard, sizeof(char) * 19 * 19) == 0) {
        CopyBoard(board, boardBackup);
        captures[1] = capBackup[1];
        captures[2] = capBackup[2];
        if (currentPlayer == 1) MessageBox(hwnd, "Ko rule violation.", "Invalid Move", MB_OK);
        return;
    }
    
    PushUndo(boardBackup, capBackup, currentPlayer);
    CopyBoard(prevBoard, boardBackup);
    currentPlayer = opp;
    
    if (caps > 0) PlayGameSound(2);
    else PlayGameSound(1);
    
    animX = x;
    animY = y;
    animRadius = 0;
    SetTimer(hwnd, 1, 16, NULL);
    
    if (caps > 0) {
        captureAnimRadius = 13;
        SetTimer(hwnd, 3, 16, NULL);
    }
    
    InvalidateRect(hwnd, NULL, TRUE);
    
    if (currentPlayer == 2 && SendMessage(GetDlgItem(hwnd, ID_CB_AI), BM_GETCHECK, 0, 0) == BST_CHECKED) {
        SetTimer(hwnd, 2, 500, NULL);
    }
}

int IsValidMove(int x, int y, int color, int *outCaptures) {
    if (board[y][x] != 0) return 0;
    
    char boardBackup[19][19];
    CopyBoard(boardBackup, board);
    int capBackup[3] = { captures[0], captures[1], captures[2] };
    
    board[y][x] = (char)color;
    int opp = color == 1 ? 2 : 1;
    int capCount = CheckCaptures(opp, 0);
    if (outCaptures) *outCaptures = capCount;
    
    char visited[19][19] = {0};
    if (GetLiberties(x, y, color, visited) == 0) {
        CopyBoard(board, boardBackup);
        captures[1] = capBackup[1];
        captures[2] = capBackup[2];
        return 0; // suicide
    }
    
    if (memcmp(board, prevBoard, sizeof(char) * 19 * 19) == 0) {
        CopyBoard(board, boardBackup);
        captures[1] = capBackup[1];
        captures[2] = capBackup[2];
        return 0; // ko
    }
    
    CopyBoard(board, boardBackup);
    captures[1] = capBackup[1];
    captures[2] = capBackup[2];
    return 1;
}

void PlaceHandicapStones(int size, int count) {
    if (count <= 0) return;
    int pts9[5][2] = {{2,2}, {6,6}, {6,2}, {2,6}, {4,4}};
    int pts13[5][2] = {{3,3}, {9,9}, {9,3}, {3,9}, {6,6}};
    int pts19[9][2] = {{3,3}, {15,15}, {15,3}, {3,15}, {9,9}, {9,3}, {9,15}, {3,9}, {15,9}};
    int (*pts)[2];
    int maxPts = 0;
    if (size == 9) { pts = pts9; maxPts = 5; }
    else if (size == 13) { pts = pts13; maxPts = 5; }
    else { pts = pts19; maxPts = 9; }
    if (count > maxPts) count = maxPts;
    for(int i=0; i<count; i++) {
        board[pts[i][1]][pts[i][0]] = 1; 
    }
    currentPlayer = 2; 
    CopyBoard(prevBoard, board);
}

void StartCampaignStage(HWND hwnd) {
    if (currentCampaignStage >= 10 || currentCampaignStage < 0) return;
    boardSize = campaign[currentCampaignStage].size;
    int handicap = campaign[currentCampaignStage].handicap;
    int diff = campaign[currentCampaignStage].difficulty;
    
    SendMessage(GetDlgItem(hwnd, ID_CB_SIZE), CB_SETCURSEL, boardSize==9?0:(boardSize==13?1:2), 0);
    SendMessage(GetDlgItem(hwnd, ID_CB_AI), BM_SETCHECK, BST_CHECKED, 0);
    SendMessage(GetDlgItem(hwnd, ID_CB_DIFFICULTY), CB_SETCURSEL, diff, 0);
    
    InitBoard();
    PlaceHandicapStones(boardSize, handicap);
    InvalidateRect(hwnd, NULL, TRUE);
    
    char msg[128];
    sprintf(msg, "Stage %d\nBoard: %dx%d\nHandicap: %d\nDifficulty: %s", 
        currentCampaignStage+1, boardSize, boardSize, handicap, 
        diff==0?"Easy":(diff==1?"Medium":"Hard"));
    MessageBox(hwnd, msg, "Campaign Stage", MB_OK);
}

int EvaluateMove(int x, int y, int color) {
    int opp = color == 1 ? 2 : 1;
    char boardBackup[19][19];
    CopyBoard(boardBackup, board);
    int capBackup[3] = { captures[0], captures[1], captures[2] };
    
    int savingAtari = 0;
    int dx[] = {0, 1, 0, -1};
    int dy[] = {1, 0, -1, 0};
    for(int i=0; i<4; i++) {
        int nx = x+dx[i];
        int ny = y+dy[i];
        if (nx>=0 && nx<boardSize && ny>=0 && ny<boardSize) {
            if (board[ny][nx] == color) {
                char v2[19][19]={0};
                if (GetLiberties(nx, ny, color, v2) == 1) {
                    savingAtari = 1;
                }
            }
        }
    }
    
    board[y][x] = (char)color;
    int caps = CheckCaptures(opp, 0);
    
    int score = caps * 50;
    char visited[19][19] = {0};
    int myLiberties = GetLiberties(x, y, color, visited);
    score += myLiberties;
    
    if (savingAtari && myLiberties > 1) {
        score += 30;
    }
    
    for(int i=0; i<4; i++) {
        int nx = x+dx[i];
        int ny = y+dy[i];
        if (nx>=0 && nx<boardSize && ny>=0 && ny<boardSize) {
             if (board[ny][nx] == opp) {
                 char v2[19][19]={0};
                 if (GetLiberties(nx, ny, opp, v2) == 1) {
                     score += 15;
                 }
             }
        }
    }
    
    int cx = boardSize/2;
    int cy = boardSize/2;
    int dist = abs(x-cx) + abs(y-cy);
    score += (boardSize - dist); 
    
    CopyBoard(board, boardBackup);
    captures[1] = capBackup[1];
    captures[2] = capBackup[2];
    return score;
}

void MakeAIMove(HWND hwnd) {
    if (currentPlayer != 2) return;
    
    int difficulty = SendMessage(GetDlgItem(hwnd, ID_CB_DIFFICULTY), CB_GETCURSEL, 0, 0);
    
    POINT captureMoves[19*19];
    int captureCount = 0;
    POINT validMoves[19*19];
    int validCount = 0;
    
    for (int y = 0; y < boardSize; y++) {
        for (int x = 0; x < boardSize; x++) {
            int caps = 0;
            if (IsValidMove(x, y, 2, &caps)) {
                validMoves[validCount].x = x;
                validMoves[validCount].y = y;
                validCount++;
                if (caps > 0) {
                    captureMoves[captureCount].x = x;
                    captureMoves[captureCount].y = y;
                    captureCount++;
                }
            }
        }
    }
    
    if (difficulty == 2 && validCount > 0) {
        int bestScore = -1;
        int bestMoves[19*19];
        int bestCount = 0;
        for (int i = 0; i < validCount; i++) {
            int score = EvaluateMove(validMoves[i].x, validMoves[i].y, 2);
            if (score > bestScore) {
                bestScore = score;
                bestCount = 0;
                bestMoves[bestCount++] = i;
            } else if (score == bestScore) {
                bestMoves[bestCount++] = i;
            }
        }
        if (bestCount > 0) {
            int r = bestMoves[rand() % bestCount];
            PlaceStone(hwnd, validMoves[r].x, validMoves[r].y);
        } else {
            SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_PASS, 0), 0);
        }
    } else if (difficulty == 1 && captureCount > 0) {
        int r = rand() % captureCount;
        PlaceStone(hwnd, captureMoves[r].x, captureMoves[r].y);
    } else if (validCount > 0) {
        int r = rand() % validCount;
        PlaceStone(hwnd, validMoves[r].x, validMoves[r].y);
    } else {
        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_PASS, 0), 0);
    }
}

void SaveGame(HWND hwnd) {
    FILE *f = fopen("kgo_save.dat", "wb");
    if (f) {
        fwrite(&boardSize, sizeof(int), 1, f);
        fwrite(board, sizeof(char), 19*19, f);
        fwrite(prevBoard, sizeof(char), 19*19, f);
        fwrite(&currentPlayer, sizeof(int), 1, f);
        fwrite(captures, sizeof(int), 3, f);
        fwrite(&undoCount, sizeof(int), 1, f);
        fwrite(undoStack, sizeof(GameState), undoCount, f);
        fwrite(&redoCount, sizeof(int), 1, f);
        fwrite(redoStack, sizeof(GameState), redoCount, f);
        fclose(f);
        MessageBox(hwnd, "Game saved.", "Save", MB_OK);
    } else {
        MessageBox(hwnd, "Failed to save game.", "Error", MB_OK);
    }
}

void LoadGame(HWND hwnd) {
    FILE *f = fopen("kgo_save.dat", "rb");
    if (f) {
        fread(&boardSize, sizeof(int), 1, f);
        fread(board, sizeof(char), 19*19, f);
        fread(prevBoard, sizeof(char), 19*19, f);
        fread(&currentPlayer, sizeof(int), 1, f);
        fread(captures, sizeof(int), 3, f);
        fread(&undoCount, sizeof(int), 1, f);
        fread(undoStack, sizeof(GameState), undoCount, f);
        fread(&redoCount, sizeof(int), 1, f);
        fread(redoStack, sizeof(GameState), redoCount, f);
        fclose(f);
        
        int sel = 0;
        if (boardSize == 13) sel = 1;
        else if (boardSize == 19) sel = 2;
        SendMessage(GetDlgItem(hwnd, ID_CB_SIZE), CB_SETCURSEL, sel, 0);
        
        InvalidateRect(hwnd, NULL, TRUE);
        MessageBox(hwnd, "Game loaded.", "Load", MB_OK);
    } else {
        MessageBox(hwnd, "No saved game found.", "Error", MB_OK);
    }
}

void InitBoard() {
    memset(board, 0, sizeof(board));
    memset(prevBoard, 0, sizeof(prevBoard));
    currentPlayer = 1;
    captures[1] = 0;
    captures[2] = 0;
    animX = -1;
    animY = -1;
    capturedAnimCount = 0;
    undoCount = 0;
    redoCount = 0;
}

void CalculateScore(HWND hwnd) {
    int terrB = 0;
    int terrW = 0;
    char visited[19][19] = {0};
    
    for (int y = 0; y < boardSize; y++) {
        for (int x = 0; x < boardSize; x++) {
            if (board[y][x] == 0 && !visited[y][x]) {
                int touchesB = 0;
                int touchesW = 0;
                
                POINT queue[19*19];
                int head = 0, tail = 0;
                queue[tail].x = x;
                queue[tail].y = y;
                tail++;
                visited[y][x] = 1;
                
                int emptyCount = 0;
                
                while (head < tail) {
                    POINT p = queue[head++];
                    emptyCount++;
                    
                    int dx[] = {0, 1, 0, -1};
                    int dy[] = {1, 0, -1, 0};
                    for (int i = 0; i < 4; i++) {
                        int nx = p.x + dx[i];
                        int ny = p.y + dy[i];
                        if (nx >= 0 && nx < boardSize && ny >= 0 && ny < boardSize) {
                            if (board[ny][nx] == 1) touchesB = 1;
                            else if (board[ny][nx] == 2) touchesW = 1;
                            else if (!visited[ny][nx]) {
                                visited[ny][nx] = 1;
                                queue[tail].x = nx;
                                queue[tail].y = ny;
                                tail++;
                            }
                        }
                    }
                }
                
                if (touchesB && !touchesW) terrB += emptyCount;
                if (touchesW && !touchesB) terrW += emptyCount;
            }
        }
    }
    
    float totalB = captures[1] + terrB;
    float totalW = captures[2] + terrW + 6.5f;
    
    int winner = (totalB > totalW) ? 1 : 2;
    RecordGameEnd(winner, hwnd);

    char msg[512];
    if (currentCampaignStage != -1) {
        if (winner == 1) { // Black wins
            currentCampaignStage++;
            if (currentCampaignStage >= 10) {
                sprintf(msg, "Black: %g vs White: %g\n\nBlack wins!\n\nCongratulations! You completed the 10-Stage Campaign!", totalB, totalW);
                currentCampaignStage = -1;
            } else {
                sprintf(msg, "Black: %g vs White: %g\n\nBlack wins!\n\nAdvancing to Campaign Stage %d!", totalB, totalW, currentCampaignStage + 1);
            }
        } else {
            sprintf(msg, "Black: %g vs White: %g\n\nWhite wins!\n\nYou failed Campaign Stage %d. Try again.", totalB, totalW, currentCampaignStage + 1);
        }
        PlayGameSound(3);
        MessageBox(hwnd, msg, "Game Score", MB_OK);
        if (currentCampaignStage != -1) {
            StartCampaignStage(hwnd);
        } else {
            InitBoard();
            InvalidateRect(hwnd, NULL, TRUE);
        }
    } else {
        sprintf(msg, "Black: %d territory + %d captures = %g\n"
                     "White: %d territory + %d captures + 6.5 komi = %g\n\n"
                     "%s wins!", 
                     terrB, captures[1], totalB,
                     terrW, captures[2], totalW,
                     (totalB > totalW) ? "Black" : "White");
        PlayGameSound(3);
        MessageBox(hwnd, msg, "Game Score", MB_OK);
        InitBoard();
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hBtnPass, hBtnResign, hBtnNew, hCbSize, hCbAi, hCbDifficulty;

    switch (uMsg) {
        case WM_CREATE:
            hBtnPass = CreateWindow("BUTTON", "Pass", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                20, 600, 60, 30, hwnd, (HMENU)ID_BTN_PASS, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hBtnResign = CreateWindow("BUTTON", "Resign", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                90, 600, 60, 30, hwnd, (HMENU)ID_BTN_RESIGN, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Score", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                160, 600, 60, 30, hwnd, (HMENU)ID_BTN_SCORE, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hBtnNew = CreateWindow("BUTTON", "New Game", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                230, 600, 80, 30, hwnd, (HMENU)ID_BTN_NEW, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hCbSize = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                320, 600, 80, 100, hwnd, (HMENU)ID_CB_SIZE, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            SendMessage(hCbSize, CB_ADDSTRING, 0, (LPARAM)"9x9");
            SendMessage(hCbSize, CB_ADDSTRING, 0, (LPARAM)"13x13");
            SendMessage(hCbSize, CB_ADDSTRING, 0, (LPARAM)"19x19");
            SendMessage(hCbSize, CB_SETCURSEL, 0, 0);
            hCbAi = CreateWindow("BUTTON", "Play vs AI (White)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                420, 600, 150, 30, hwnd, (HMENU)ID_CB_AI, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            SendMessage(hCbAi, BM_SETCHECK, BST_CHECKED, 0);
            hCbDifficulty = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                420, 640, 80, 100, hwnd, (HMENU)ID_CB_DIFFICULTY, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            SendMessage(hCbDifficulty, CB_ADDSTRING, 0, (LPARAM)"Easy");
            SendMessage(hCbDifficulty, CB_ADDSTRING, 0, (LPARAM)"Medium");
            SendMessage(hCbDifficulty, CB_ADDSTRING, 0, (LPARAM)"Hard");
            SendMessage(hCbDifficulty, CB_SETCURSEL, 1, 0);
            CreateWindow("BUTTON", "Save", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                20, 640, 50, 30, hwnd, (HMENU)ID_BTN_SAVE, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Load", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                75, 640, 50, 30, hwnd, (HMENU)ID_BTN_LOAD, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Undo", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                130, 640, 50, 30, hwnd, (HMENU)ID_BTN_UNDO, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Redo", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                185, 640, 50, 30, hwnd, (HMENU)ID_BTN_REDO, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Stats", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                240, 640, 50, 30, hwnd, (HMENU)ID_BTN_STATS, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Help", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                295, 640, 50, 30, hwnd, (HMENU)ID_BTN_HELP, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Campaign", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                350, 640, 65, 30, hwnd, (HMENU)ID_BTN_CAMPAIGN, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            return 0;

        case WM_TIMER:
            if (wParam == 1) {
                animRadius += 2;
                if (animRadius >= 13) {
                    animRadius = 13;
                    KillTimer(hwnd, 1);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 3) {
                captureAnimRadius -= 2;
                if (captureAnimRadius <= 0) {
                    captureAnimRadius = 0;
                    capturedAnimCount = 0;
                    KillTimer(hwnd, 3);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 2) {
                KillTimer(hwnd, 2);
                MakeAIMove(hwnd);
            }
            return 0;

        case WM_MOUSEMOVE: {
            int x = (short)LOWORD(lParam);
            int y = (short)HIWORD(lParam);
            int padding = 40;
            int cellSize = 30;
            
            if (x >= padding - cellSize/2 && x <= padding + (boardSize-1)*cellSize + cellSize/2 &&
                y >= padding - cellSize/2 && y <= padding + (boardSize-1)*cellSize + cellSize/2) {
                int col = (x - padding + cellSize / 2) / cellSize;
                int row = (y - padding + cellSize / 2) / cellSize;
                if (col >= 0 && col < boardSize && row >= 0 && row < boardSize) {
                    if (hoverX != col || hoverY != row) {
                        hoverX = col;
                        hoverY = row;
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                }
            } else {
                if (hoverX != -1 || hoverY != -1) {
                    hoverX = -1; hoverY = -1;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            return 0;
        }

        case WM_LBUTTONDOWN: {
            int x = (short)LOWORD(lParam);
            int y = (short)HIWORD(lParam);
            int padding = 40;
            int cellSize = 30;
            
            // Check if within bounds roughly
            int col = (x - padding + cellSize / 2) / cellSize;
            int row = (y - padding + cellSize / 2) / cellSize;
            
            if (col >= 0 && col < boardSize && row >= 0 && row < boardSize) {
                PlaceStone(hwnd, col, row);
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HBRUSH hBrush = CreateSolidBrush(RGB(18, 18, 18));
            FillRect(hdc, &ps.rcPaint, hBrush);
            DeleteObject(hBrush);

            int padding = 40;
            int cellSize = 30;
            
            HBRUSH boardBrush = CreateSolidBrush(RGB(220, 179, 92)); // #dcb35c
            RECT boardRect = { padding, padding, padding + (boardSize-1)*cellSize, padding + (boardSize-1)*cellSize };
            boardRect.left -= 15; boardRect.top -= 15;
            boardRect.right += 15; boardRect.bottom += 15;
            FillRect(hdc, &boardRect, boardBrush);
            DeleteObject(boardBrush);

            HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            SelectObject(hdc, hPen);
            for (int i = 0; i < boardSize; i++) {
                MoveToEx(hdc, padding, padding + i * cellSize, NULL);
                LineTo(hdc, padding + (boardSize - 1) * cellSize, padding + i * cellSize);
                MoveToEx(hdc, padding + i * cellSize, padding, NULL);
                LineTo(hdc, padding + i * cellSize, padding + (boardSize - 1) * cellSize);
            }
            DeleteObject(hPen);
            
            for (int r = 0; r < boardSize; r++) {
                for (int c = 0; c < boardSize; c++) {
                    if (board[r][c] != 0) {
                        int cx = padding + c * cellSize;
                        int cy = padding + r * cellSize;
                        HBRUSH stoneBrush = CreateSolidBrush(board[r][c] == 1 ? RGB(17, 17, 17) : RGB(238, 238, 238));
                        HPEN stonePen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
                        HBRUSH oldBrush = SelectObject(hdc, stoneBrush);
                        HPEN oldPen = SelectObject(hdc, stonePen);
                        
                        int radius = (r == animY && c == animX) ? animRadius : 13;
                        Ellipse(hdc, cx - radius, cy - radius, cx + radius, cy + radius);
                        
                        SelectObject(hdc, oldBrush);
                        SelectObject(hdc, oldPen);
                        DeleteObject(stoneBrush);
                        DeleteObject(stonePen);
                    }
                }
            }

            for (int i = 0; i < capturedAnimCount; i++) {
                int cx = padding + capturedAnimStones[i].x * cellSize;
                int cy = padding + capturedAnimStones[i].y * cellSize;
                HBRUSH stoneBrush = CreateSolidBrush(capturedAnimColor[i] == 1 ? RGB(17, 17, 17) : RGB(238, 238, 238));
                HPEN stonePen = CreatePen(PS_SOLID, 2, RGB(255, 68, 68)); // Red border for capture highlight
                HBRUSH oldBrush = SelectObject(hdc, stoneBrush);
                HPEN oldPen = SelectObject(hdc, stonePen);
                
                Ellipse(hdc, cx - captureAnimRadius, cy - captureAnimRadius, cx + captureAnimRadius, cy + captureAnimRadius);
                
                SelectObject(hdc, oldBrush);
                SelectObject(hdc, oldPen);
                DeleteObject(stoneBrush);
                DeleteObject(stonePen);
            }

            if (hoverX != -1 && hoverY != -1 && board[hoverY][hoverX] == 0) {
                int cx = padding + hoverX * cellSize;
                int cy = padding + hoverY * cellSize;
                HPEN ghostPen = CreatePen(PS_SOLID, 2, currentPlayer == 1 ? RGB(100, 100, 100) : RGB(200, 200, 200));
                HBRUSH ghostBrush = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
                HPEN oldPen = SelectObject(hdc, ghostPen);
                HBRUSH oldBrush = SelectObject(hdc, ghostBrush);
                Ellipse(hdc, cx - 13, cy - 13, cx + 13, cy + 13);
                SelectObject(hdc, oldBrush);
                SelectObject(hdc, oldPen);
                DeleteObject(ghostPen);
            }
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(224, 224, 224));
            char status[64];
            sprintf(status, "KGo - Current Turn: %s", currentPlayer == 1 ? "Black" : "White");
            TextOut(hdc, 20, 10, status, strlen(status));

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_BTN_PASS) {
                int capBackup[3] = { captures[0], captures[1], captures[2] };
                char bBackup[19][19];
                CopyBoard(bBackup, board);
                PushUndo(bBackup, capBackup, currentPlayer);
                currentPlayer = currentPlayer == 1 ? 2 : 1;
                InvalidateRect(hwnd, NULL, TRUE);
                if (currentPlayer == 2 && SendMessage(GetDlgItem(hwnd, ID_CB_AI), BM_GETCHECK, 0, 0) == BST_CHECKED) {
                    SetTimer(hwnd, 2, 500, NULL);
                }
            } else if (LOWORD(wParam) == ID_BTN_RESIGN) {
                int winner = currentPlayer == 1 ? 2 : 1;
                RecordGameEnd(winner, hwnd);
                char msg[256];
                if (currentCampaignStage != -1) {
                    sprintf(msg, "%s wins by resignation!\n\nYou failed Campaign Stage %d. Try again.", 
                            currentPlayer == 1 ? "White" : "Black", currentCampaignStage + 1);
                } else {
                    sprintf(msg, "%s wins by resignation!", currentPlayer == 1 ? "White" : "Black");
                }
                PlayGameSound(3);
                MessageBox(hwnd, msg, "Game Over", MB_OK);
                if (currentCampaignStage != -1) {
                    StartCampaignStage(hwnd);
                } else {
                    InitBoard();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (LOWORD(wParam) == ID_BTN_SCORE) {
                CalculateScore(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_NEW) {
                currentCampaignStage = -1;
                InitBoard();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == ID_BTN_SAVE) {
                SaveGame(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_LOAD) {
                LoadGame(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_UNDO) {
                DoUndo(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_REDO) {
                DoRedo(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_STATS) {
                char smsg[256];
                sprintf(smsg, "Statistics:\nGames Played: %d\n\nvs AI Mode:\nWins: %d\nLosses: %d\n\nLocal Mode:\nBlack Wins: %d\nWhite Wins: %d",
                        stats.played, stats.aiWins, stats.aiLosses, stats.localB, stats.localW);
                MessageBox(hwnd, smsg, "Statistics", MB_OK);
            } else if (LOWORD(wParam) == ID_BTN_CAMPAIGN) {
                currentCampaignStage = 0;
                StartCampaignStage(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_HELP) {
                MessageBox(hwnd, 
                    "Goal: Control more territory (empty points) than your opponent.\n\n"
                    "- Players (Black & White) take turns placing stones on intersections.\n"
                    "- Stones must have at least one empty adjacent point (liberty).\n"
                    "- If surrounded, stones are captured and removed.\n"
                    "- Ko Rule: Cannot recreate the exact previous board position.\n"
                    "- Suicide Rule: Cannot place a stone with no liberties unless it captures.\n"
                    "- Score: Empty intersections surrounded + captured stones.\n"
                    "  (White gets 6.5 extra points as Komi).",
                    "How to Play KGo", MB_OK | MB_ICONINFORMATION);
            } else if (LOWORD(wParam) == ID_CB_SIZE && HIWORD(wParam) == CBN_SELCHANGE) {
                int sel = SendMessage(hCbSize, CB_GETCURSEL, 0, 0);
                if (sel == 0) boardSize = 9;
                else if (sel == 1) boardSize = 13;
                else if (sel == 2) boardSize = 19;
                InitBoard();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KGoWindowClass";
    srand(GetTickCount());

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(18, 18, 18));

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KGo",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 620, 700,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    LoadStats();

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
