#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define W 480
#define H 500
#define TS 50
#define OX 30
#define OY 30

// 0: empty, 1:P, 2:N, 3:B, 4:R, 5:Q, 6:K,  7:p, 8:n, 9:b, 10:r, 11:q, 12:k
int board[8][8] = {
    {10, 8, 9, 11, 12, 9, 8, 10},
    {7,  7, 7,  7,  7, 7, 7,  7},
    {0,  0, 0,  0,  0, 0, 0,  0},
    {0,  0, 0,  0,  0, 0, 0,  0},
    {0,  0, 0,  0,  0, 0, 0,  0},
    {0,  0, 0,  0,  0, 0, 0,  0},
    {1,  1, 1,  1,  1, 1, 1,  1},
    {4,  2, 3,  5,  6, 3, 2,  4}
};

const char* pieceChars[] = {
    "", "P", "N", "B", "R", "Q", "K",
    "p", "n", "b", "r", "q", "k"
};

int selX = -1;
int selY = -1;
int whiteTurn = 1;
int gameOver = 0;
int winner = 0; // 1 = White, 2 = Black
int aiMode = 1; // 1 = PvE, 0 = PvP
int campaignMode = 0;
int currentStage = 1;
int aiDifficulty = 2; // 1=Easy, 2=Normal, 3=Hard
int statsWins = 0, statsLosses = 0, statsDraws = 0;
int pieceValues[] = {0, 100, 300, 300, 500, 900, 9000, 100, 300, 300, 500, 900, 9000};
int lastMoveSx = -1, lastMoveSy = -1, lastMoveTx = -1, lastMoveTy = -1;
HBRUSH hBgBrush = NULL;

int wKingMoved = 0, wRookLMoved = 0, wRookRMoved = 0;
int bKingMoved = 0, bRookLMoved = 0, bRookRMoved = 0;
int epX = -1, epY = -1;
int freezePowerups = 1;
int blackFrozen = 0;
int undoPowerups = 1;
int canUndo = 0;
int undoBoard[8][8];
int undoWKM = 0, undoWRL = 0, undoWRR = 0, undoBKM = 0, undoBRL = 0, undoBRR = 0, undoEpX = -1, undoEpY = -1;

int GetPST(int pType, int x, int y, int isWhite) {
    int row = isWhite ? y : 7 - y;
    int center = (x >= 3 && x <= 4 && row >= 3 && row <= 4) ? 10 : 
                 (x >= 2 && x <= 5 && row >= 2 && row <= 5) ? 5 : 0;
    
    if (pType == 1) { // Pawn
        if (row <= 1) return 50; 
        if (row == 2) return 20;
        if (row == 3) return 10;
        return 0;
    }
    if (pType == 2) { // Knight
        return center * 3;
    }
    if (pType == 3) { // Bishop
        return center * 2;
    }
    if (pType == 6) { // King
        if (row >= 6 && (x <= 2 || x >= 5)) return 20; 
        return -center * 2; 
    }
    return 0;
}


void LoadStats() {
    FILE *f = fopen("kchess_stats.txt", "r");
    if (f) {
        fscanf(f, "%d %d %d", &statsWins, &statsLosses, &statsDraws);
        fclose(f);
    }
}
void SaveStats() {
    FILE *f = fopen("kchess_stats.txt", "w");
    if (f) {
        fprintf(f, "%d %d %d", statsWins, statsLosses, statsDraws);
        fclose(f);
    }
}


int IsValidMove(int sx, int sy, int tx, int ty, int isAttackCheck) {
    if (sx == tx && sy == ty) return 0;
    int p = board[sy][sx];
    if (p == 0) return 0;
    int isWhite = p <= 6;
    int dstP = board[ty][tx];
    int dstIsWhite = dstP <= 6;
    if (dstP != 0 && isWhite == dstIsWhite) return 0;

    int dx = tx - sx;
    int dy = ty - sy;
    int adx = abs(dx);
    int ady = abs(dy);
    int pType = p > 6 ? p - 6 : p;

    int pathClear = 1;
    if (adx == ady || adx == 0 || ady == 0) {
        int stepX = dx == 0 ? 0 : dx / adx;
        int stepY = dy == 0 ? 0 : dy / ady;
        int cx = sx + stepX;
        int cy = sy + stepY;
        while (cx != tx || cy != ty) {
            if (board[cy][cx] != 0) { pathClear = 0; break; }
            cx += stepX; cy += stepY;
        }
    }

    if (pType == 1) { // Pawn
        if (isAttackCheck) {
            if (isWhite) return (adx == 1 && dy == -1);
            else return (adx == 1 && dy == 1);
        } else {
            if (isWhite) {
                if (dx == 0 && dy == -1 && dstP == 0) return 1;
                if (dx == 0 && dy == -2 && sy == 6 && dstP == 0 && board[5][tx] == 0) return 1;
                if (adx == 1 && dy == -1 && (dstP != 0 || (tx == epX && ty == epY - 1))) return 1;
            } else {
                if (dx == 0 && dy == 1 && dstP == 0) return 1;
                if (dx == 0 && dy == 2 && sy == 1 && dstP == 0 && board[2][tx] == 0) return 1;
                if (adx == 1 && dy == 1 && (dstP != 0 || (tx == epX && ty == epY + 1))) return 1;
            }
        }
    } else if (pType == 2) { // Knight
        if ((adx == 1 && ady == 2) || (adx == 2 && ady == 1)) return 1;
    } else if (pType == 3) { // Bishop
        if (adx == ady && pathClear) return 1;
    } else if (pType == 4) { // Rook
        if ((adx == 0 || ady == 0) && pathClear) return 1;
    } else if (pType == 5) { // Queen
        if ((adx == ady || adx == 0 || ady == 0) && pathClear) return 1;
    } else if (pType == 6) { // King
        if (adx <= 1 && ady <= 1) return 1;
        if (!isAttackCheck && dy == 0 && adx == 2) {
            if (isWhite) {
                if (wKingMoved) return 0;
                if (tx == 6 && !wRookRMoved && board[7][5] == 0 && board[7][6] == 0) {
                    if (!IsSquareAttacked(4, 7, 0) && !IsSquareAttacked(5, 7, 0) && !IsSquareAttacked(6, 7, 0)) return 1;
                }
                if (tx == 2 && !wRookLMoved && board[7][1] == 0 && board[7][2] == 0 && board[7][3] == 0) {
                    if (!IsSquareAttacked(4, 7, 0) && !IsSquareAttacked(3, 7, 0) && !IsSquareAttacked(2, 7, 0)) return 1;
                }
            } else {
                if (bKingMoved) return 0;
                if (tx == 6 && !bRookRMoved && board[0][5] == 0 && board[0][6] == 0) {
                    if (!IsSquareAttacked(4, 0, 1) && !IsSquareAttacked(5, 0, 1) && !IsSquareAttacked(6, 0, 1)) return 1;
                }
                if (tx == 2 && !bRookLMoved && board[0][1] == 0 && board[0][2] == 0 && board[0][3] == 0) {
                    if (!IsSquareAttacked(4, 0, 1) && !IsSquareAttacked(3, 0, 1) && !IsSquareAttacked(2, 0, 1)) return 1;
                }
            }
        }
    }
    return 0;
}

int IsSquareAttacked(int tx, int ty, int byWhite) {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int p = board[y][x];
            if (p != 0 && (p <= 6) == byWhite) {
                if (IsValidMove(x, y, tx, ty, 1)) return 1;
            }
        }
    }
    return 0;
}

int SimulatedMoveLeavesCheck(int sx, int sy, int tx, int ty, int isWhite) {
    int savedSrc = board[sy][sx];
    int savedDst = board[ty][tx];
    int pType = savedSrc > 6 ? savedSrc - 6 : savedSrc;
    
    int isEP = (pType == 1 && tx == epX && ty == (isWhite ? epY - 1 : epY + 1));
    int savedEP = 0;
    if (isEP) {
        savedEP = board[epY][epX];
        board[epY][epX] = 0;
    }
    
    board[ty][tx] = savedSrc;
    board[sy][sx] = 0;
    
    int kx = -1, ky = -1;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (board[y][x] == (isWhite ? 6 : 12)) {
                kx = x; ky = y;
            }
        }
    }
    
    int inCheck = 0;
    if (kx != -1 && ky != -1) {
        inCheck = IsSquareAttacked(kx, ky, !isWhite);
    }
    
    board[sy][sx] = savedSrc;
    board[ty][tx] = savedDst;
    if (isEP) {
        board[epY][epX] = savedEP;
    }
    return inCheck;
}

int HasLegalMoves(int isWhite) {
    for (int sy = 0; sy < 8; sy++) {
        for (int sx = 0; sx < 8; sx++) {
            int p = board[sy][sx];
            if (p != 0 && (p <= 6) == isWhite) {
                for (int ty = 0; ty < 8; ty++) {
                    for (int tx = 0; tx < 8; tx++) {
                        int dstP = board[ty][tx];
                        if (dstP != 0 && (dstP <= 6) == isWhite) continue;
                        if (IsValidMove(sx, sy, tx, ty, 0)) {
                            if (!SimulatedMoveLeavesCheck(sx, sy, tx, ty, isWhite)) {
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

void ResetGame() {
    int initialBoard[8][8] = {
        {10, 8, 9, 11, 12, 9, 8, 10},
        {7,  7, 7,  7,  7, 7, 7,  7},
        {0,  0, 0,  0,  0, 0, 0,  0},
        {0,  0, 0,  0,  0, 0, 0,  0},
        {0,  0, 0,  0,  0, 0, 0,  0},
        {0,  0, 0,  0,  0, 0, 0,  0},
        {1,  1, 1,  1,  1, 1, 1,  1},
        {4,  2, 3,  5,  6, 3, 2,  4}
    };
    
    wKingMoved = 0; wRookLMoved = 0; wRookRMoved = 0;
    bKingMoved = 0; bRookLMoved = 0; bRookRMoved = 0;
    epX = -1; epY = -1;
    if (campaignMode) {
        aiDifficulty = currentStage > 3 ? 3 : currentStage;
        if (currentStage == 4) initialBoard[7][1] = 0; 
        if (currentStage == 5) initialBoard[7][3] = 0; 
        if (currentStage == 6) {
            for(int x=0; x<8; x++) { initialBoard[0][x] = 0; initialBoard[1][x] = 7; initialBoard[2][x] = 7; }
            initialBoard[0][4] = 12;
        }
        if (currentStage == 7) {
            initialBoard[7][3] = 0; initialBoard[0][3] = 0;
            initialBoard[7][0] = 0; initialBoard[0][0] = 0;
        }
        if (currentStage == 8) {
            initialBoard[7][2] = 2; initialBoard[7][5] = 2;
            initialBoard[0][2] = 8; initialBoard[0][5] = 8;
        }
        if (currentStage == 9) {
            initialBoard[7][0] = 0; initialBoard[7][7] = 0;
            initialBoard[0][3] = 0;
        }
        if (currentStage == 10) {
            initialBoard[7][3] = 0;
        }
        if (currentStage == 11) {
            for(int x=0; x<8; x++) initialBoard[2][x] = 7; 
        }
        if (currentStage == 12) {
            initialBoard[7][1] = 0; initialBoard[7][6] = 0;
            initialBoard[2][2] = 8; initialBoard[2][5] = 8;
        }
        if (currentStage == 13) {
            initialBoard[0][2] = 11;
        }
        if (currentStage == 14) {
            initialBoard[7][2] = 0; initialBoard[7][5] = 0;
            initialBoard[2][0] = 10; initialBoard[2][7] = 10;
        }
        if (currentStage == 15) {
            initialBoard[0][2] = 11; 
            initialBoard[2][1] = 8; initialBoard[2][6] = 8;
        }
        freezePowerups = 1;
        undoPowerups = 1;
    } else {
        aiDifficulty = 2; // Default Normal
        freezePowerups = 1;
        undoPowerups = 1;
    }
    
    for(int y=0; y<8; y++) {
        for(int x=0; x<8; x++) {
            board[y][x] = initialBoard[y][x];
        }
    }
    selX = -1;
    selY = -1;
    whiteTurn = 1;
    gameOver = 0;
    winner = 0;
    blackFrozen = 0;
    canUndo = 0;
    lastMoveSx = -1; lastMoveSy = -1; lastMoveTx = -1; lastMoveTy = -1;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HFONT hFont = CreateFontA(32, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            HGDIOBJ oldFont = SelectObject(hdc, hFont);
            SetBkMode(hdc, TRANSPARENT);
            
            int inCheckX = -1, inCheckY = -1;
            for (int cy = 0; cy < 8; cy++) {
                for (int cx = 0; cx < 8; cx++) {
                    int p = board[cy][cx];
                    if (p == 6 || p == 12) {
                        int isWhite = p <= 6;
                        if (IsSquareAttacked(cx, cy, !isWhite)) {
                            inCheckX = cx;
                            inCheckY = cy;
                        }
                    }
                }
            }

            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    RECT rc = { OX + x * TS, OY + y * TS, OX + (x + 1) * TS, OY + (y + 1) * TS };
                    HBRUSH brush;
                    
                    if (x == inCheckX && y == inCheckY) {
                        brush = CreateSolidBrush(RGB(239, 68, 68)); // red-500 for check
                    } else if (x == selX && y == selY) {
                        brush = CreateSolidBrush(RGB(245, 158, 11)); // selected
                    } else if ((x == lastMoveSx && y == lastMoveSy) || (x == lastMoveTx && y == lastMoveTy)) {
                        brush = CreateSolidBrush(RGB(253, 230, 138)); // yellow-200 for last move
                    } else if ((x + y) % 2 == 0) {
                        brush = CreateSolidBrush(RGB(203, 213, 225)); // light
                    } else {
                        brush = CreateSolidBrush(RGB(30, 41, 59)); // dark
                    }

                    
                    FillRect(hdc, &rc, brush);
                    DeleteObject(brush);
                    
                    if (selX != -1 && IsValidMove(selX, selY, x, y, 0)) {
                        if (!SimulatedMoveLeavesCheck(selX, selY, x, y, whiteTurn)) {
                            HBRUSH dotBrush = CreateSolidBrush(RGB(134, 239, 172)); // green-300
                            HGDIOBJ oldBrush = SelectObject(hdc, dotBrush);
                            HPEN oldPen = SelectObject(hdc, GetStockObject(NULL_PEN));
                            Ellipse(hdc, rc.left + 15, rc.top + 15, rc.right - 15, rc.bottom - 15);
                            SelectObject(hdc, oldPen);
                            SelectObject(hdc, oldBrush);
                            DeleteObject(dotBrush);
                        }
                    }
                    
                    int p = board[y][x];
                    if (p != 0) {
                        if (p <= 6) SetTextColor(hdc, RGB(255, 255, 255)); // White pieces
                        else SetTextColor(hdc, RGB(0, 0, 0)); // Black pieces
                        
                        DrawTextA(hdc, pieceChars[p], -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    }
                }
            }
            
            SelectObject(hdc, oldFont);
            DeleteObject(hFont);
            
            RECT statusRc = { 10, H - 60, W - 10, H - 30 };
            HFONT sFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            oldFont = SelectObject(hdc, sFont);
            SetTextColor(hdc, RGB(255, 255, 255));
            
            if (gameOver) {
                if (winner == 1) DrawTextA(hdc, campaignMode && currentStage < 15 ? "White Wins! Press 'R' for Next Stage" : "Checkmate! White Wins! Press 'R'", -1, &statusRc, DT_LEFT);
                else if (winner == 2) DrawTextA(hdc, "Checkmate! Black Wins! Press 'R'", -1, &statusRc, DT_LEFT);
                else DrawTextA(hdc, "Stalemate! Draw! Press 'R'", -1, &statusRc, DT_LEFT);
            } else {
                if (whiteTurn) {
                    DrawTextA(hdc, "White's Turn", -1, &statusRc, DT_LEFT);
                } else {
                    DrawTextA(hdc, "Black's Turn", -1, &statusRc, DT_LEFT);
                }
            }
            
            RECT modeRc = { 10, 5, W - 10, 25 };
            char modeBuf[128];
            if (campaignMode) {
                sprintf(modeBuf, "Campaign: Stage %d | F:Freeze(%d) U:Undo(%d)", currentStage, freezePowerups, undoPowerups);
            } else {
                sprintf(modeBuf, "vs AI ('M') | F:Freeze(%d) U:Undo(%d) | 'C' Cmpn", freezePowerups, undoPowerups);
            }
            DrawTextA(hdc, modeBuf, -1, &modeRc, DT_LEFT);

            RECT statsRc = { W - 150, 5, W - 10, 25 };
            char statsBuf[64];
            sprintf(statsBuf, "W:%d L:%d D:%d", statsWins, statsLosses, statsDraws);
            DrawTextA(hdc, statsBuf, -1, &statsRc, DT_RIGHT);
            
            SelectObject(hdc, oldFont);
            DeleteObject(sFont);
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_KEYDOWN: {
            if (wParam == 'R') {
                if (gameOver && campaignMode && winner == 1) {
                    if (currentStage < 15) currentStage++;
                    else { campaignMode = 0; currentStage = 1; }
                }
                ResetGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'M') {
                if (!campaignMode) {
                    aiMode = !aiMode;
                    ResetGame();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (wParam == 'C') {
                campaignMode = !campaignMode;
                if (campaignMode) { aiMode = 1; currentStage = 1; }
                ResetGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'F') {
                if (!gameOver && whiteTurn && aiMode && freezePowerups > 0) {
                    freezePowerups--;
                    blackFrozen = 1;
                    MessageBeep(MB_OK);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (wParam == 'U') {
                if (!gameOver && whiteTurn && aiMode && undoPowerups > 0 && canUndo) {
                    undoPowerups--;
                    for(int y=0; y<8; y++) {
                        for(int x=0; x<8; x++) {
                            board[y][x] = undoBoard[y][x];
                        }
                    }
                    wKingMoved = undoWKM; wRookLMoved = undoWRL; wRookRMoved = undoWRR;
                    bKingMoved = undoBKM; bRookLMoved = undoBRL; bRookRMoved = undoBRR;
                    epX = undoEpX; epY = undoEpY;
                    selX = -1; selY = -1; lastMoveSx = -1; lastMoveSy = -1; lastMoveTx = -1; lastMoveTy = -1;
                    canUndo = 0;
                    MessageBeep(MB_OK);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            break;
        }
        case WM_TIMER: {
            if (wParam == 1) {
                KillTimer(hwnd, 1);
                if (!gameOver && !whiteTurn && aiMode) {
                    if (blackFrozen) {
                        blackFrozen = 0;
                        whiteTurn = 1;
                        InvalidateRect(hwnd, NULL, TRUE);
                    } else {
                        struct Move { int sx, sy, tx, ty, score; } moves[1024];
                        int moveCount = 0;
                        for (int sy = 0; sy < 8; sy++) {
                            for (int sx = 0; sx < 8; sx++) {
                                if (board[sy][sx] > 6) { 
                                    for (int ty = 0; ty < 8; ty++) {
                                        for (int tx = 0; tx < 8; tx++) {
                                            if (IsValidMove(sx, sy, tx, ty, 0)) {
                                                if (!SimulatedMoveLeavesCheck(sx, sy, tx, ty, 0)) {
                                                    moves[moveCount].sx = sx; moves[moveCount].sy = sy;
                                                    moves[moveCount].tx = tx; moves[moveCount].ty = ty;
                                                    int dstP = board[ty][tx];
                                                    int baseScore = dstP != 0 ? pieceValues[dstP] : 0;
                                                    int pType = board[sy][sx] > 6 ? board[sy][sx] - 6 : board[sy][sx];
                                                    if (aiDifficulty >= 2) {
                                                        baseScore += GetPST(pType, tx, ty, 0) - GetPST(pType, sx, sy, 0);
                                                        if (IsSquareAttacked(tx, ty, 1)) baseScore -= (pieceValues[board[sy][sx]] / 10);
                                                    }
                                                    if (aiDifficulty >= 3) {
                                                        int savedSrc = board[sy][sx];
                                                        int savedDst = board[ty][tx];
                                                        board[ty][tx] = savedSrc;
                                                        board[sy][sx] = 0;
                                                        int maxOppScore = 0;
                                                        for (int osy = 0; osy < 8; osy++) {
                                                            for (int osx = 0; osx < 8; osx++) {
                                                                if (board[osy][osx] != 0 && board[osy][osx] <= 6) {
                                                                    for (int oty = 0; oty < 8; oty++) {
                                                                        for (int otx = 0; otx < 8; otx++) {
                                                                            if (IsValidMove(osx, osy, otx, oty, 0)) {
                                                                                int oDstP = board[oty][otx];
                                                                                if (oDstP != 0) {
                                                                                    int oScore = pieceValues[oDstP];
                                                                                    if (oScore > maxOppScore) maxOppScore = oScore;
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                        board[sy][sx] = savedSrc;
                                                        board[ty][tx] = savedDst;
                                                        baseScore -= maxOppScore;
                                                    }
                                                    moves[moveCount].score = baseScore;
                                                    moveCount++;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        if (moveCount > 0) {
                            int bestScore = -9999999;
                            for (int i = 0; i < moveCount; i++) if (moves[i].score > bestScore) bestScore = moves[i].score;
                            int bestMoves[1024], bestCount = 0;
                            for (int i = 0; i < moveCount; i++) if (moves[i].score == bestScore) bestMoves[bestCount++] = i;
                            int chosen = bestMoves[rand() % bestCount];
                            
                            int sx = moves[chosen].sx; int sy = moves[chosen].sy;
                            int tx = moves[chosen].tx; int ty = moves[chosen].ty;
                            
                            int pType = board[sy][sx] > 6 ? board[sy][sx] - 6 : board[sy][sx];
                            int isWhiteMove = 0;
                            if (pType == 1 && tx == epX && ty == epY + 1) board[epY][epX] = 0;
                            board[ty][tx] = board[sy][sx];
                            if (pType == 1 && ty == 7) board[ty][tx] = 11;
                            if (pType == 6 && abs(tx - sx) == 2) {
                                if (tx == 6) { board[ty][5] = board[ty][7]; board[ty][7] = 0; }
                                else if (tx == 2) { board[ty][3] = board[ty][0]; board[ty][0] = 0; }
                            }
                            if (pType == 6) bKingMoved = 1;
                            if (pType == 4) { if (sx == 0) bRookLMoved = 1; if (sx == 7) bRookRMoved = 1; }
                            if (pType == 1 && abs(ty - sy) == 2) { epX = tx; epY = ty; } else { epX = -1; epY = -1; }
                            board[sy][sx] = 0;
                            whiteTurn = 1;
                            lastMoveSx = sx; lastMoveSy = sy; lastMoveTx = tx; lastMoveTy = ty;
                            MessageBeep(MB_OK);
                            
                            if (!HasLegalMoves(whiteTurn)) {
                                gameOver = 1;
                                int kx = -1, ky = -1;
                                for(int cy=0; cy<8; cy++) {
                                    for(int cx=0; cx<8; cx++) {
                                        if(board[cy][cx] == (whiteTurn ? 6 : 12)) { kx = cx; ky = cy; }
                                    }
                                }
                                if (kx != -1 && ky != -1 && IsSquareAttacked(kx, ky, !whiteTurn)) {
                                    winner = !whiteTurn ? 1 : 2;
                                    if (aiMode) { if (winner == 1) statsWins++; else statsLosses++; SaveStats(); }
                                } else {
                                    winner = 3;
                                    if (aiMode) { statsDraws++; SaveStats(); }
                                }
                            }
                            InvalidateRect(hwnd, NULL, TRUE);
                        }
                    }
                }
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            if (gameOver) break;
            if (aiMode && !whiteTurn) break;
            
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            
            int tx = (mx - OX) / TS;
            int ty = (my - OY) / TS;
            
            if (tx >= 0 && tx < 8 && ty >= 0 && ty < 8) {
                if (selX == -1) {
                    if (board[ty][tx] != 0) {
                        int isWhite = board[ty][tx] <= 6;
                        if ((whiteTurn && isWhite) || (!whiteTurn && !isWhite)) {
                            selX = tx;
                            selY = ty;
                        }
                    }
                } else {
                    if (selX == tx && selY == ty) {
                        selX = -1;
                        selY = -1;
                    } else {
                        int p = board[selY][selX];
                        int isWhite = p <= 6;
                        int dstP = board[ty][tx];
                        int dstIsWhite = dstP <= 6;
                        
                        if (dstP != 0 && isWhite == dstIsWhite) {
                            selX = tx;
                            selY = ty;
                        } else {
                            if (IsValidMove(selX, selY, tx, ty, 0)) {
                                if (!SimulatedMoveLeavesCheck(selX, selY, tx, ty, whiteTurn)) {
                                    if (whiteTurn) {
                                        for(int y=0; y<8; y++) for(int x=0; x<8; x++) undoBoard[y][x] = board[y][x];
                                        undoWKM = wKingMoved; undoWRL = wRookLMoved; undoWRR = wRookRMoved;
                                        undoBKM = bKingMoved; undoBRL = bRookLMoved; undoBRR = bRookRMoved;
                                        undoEpX = epX; undoEpY = epY;
                                        canUndo = 1;
                                    }
                                    
                                    int pType = board[selY][selX] > 6 ? board[selY][selX] - 6 : board[selY][selX];
                                    int isWhiteMove = selY == 7 || board[selY][selX] <= 6;
                                    if (pType == 1 && tx == epX && ty == (isWhiteMove ? epY - 1 : epY + 1)) board[epY][epX] = 0;
                                    board[ty][tx] = board[selY][selX];
                                    if (pType == 1) {
                                        if (isWhiteMove && ty == 0) board[ty][tx] = 5;
                                        else if (!isWhiteMove && ty == 7) board[ty][tx] = 11;
                                    }
                                    if (pType == 6 && abs(tx - selX) == 2) {
                                        if (tx == 6) { board[ty][5] = board[ty][7]; board[ty][7] = 0; }
                                        else if (tx == 2) { board[ty][3] = board[ty][0]; board[ty][0] = 0; }
                                    }
                                    if (pType == 6) {
                                        if (isWhiteMove) wKingMoved = 1; else bKingMoved = 1;
                                    }
                                    if (pType == 4) {
                                        if (isWhiteMove) { if (selX == 0) wRookLMoved = 1; if (selX == 7) wRookRMoved = 1; }
                                        else { if (selX == 0) bRookLMoved = 1; if (selX == 7) bRookRMoved = 1; }
                                    }
                                    if (pType == 1 && abs(ty - selY) == 2) { epX = tx; epY = ty; } else { epX = -1; epY = -1; }
                                    lastMoveSx = selX; lastMoveSy = selY; lastMoveTx = tx; lastMoveTy = ty;
                                    board[selY][selX] = 0;
                                    selX = -1;
                                    selY = -1;
                                    whiteTurn = !whiteTurn;
                                    MessageBeep(MB_OK);
                                    
                                    if (!HasLegalMoves(whiteTurn)) {
                                        gameOver = 1;
                                        int kx = -1, ky = -1;
                                        for(int cy=0; cy<8; cy++) {
                                            for(int cx=0; cx<8; cx++) {
                                                if(board[cy][cx] == (whiteTurn ? 6 : 12)) { kx = cx; ky = cy; }
                                            }
                                        }
                                        if (kx != -1 && ky != -1 && IsSquareAttacked(kx, ky, !whiteTurn)) {
                                            winner = !whiteTurn ? 1 : 2;
                                            if (aiMode) { if (winner == 1) statsWins++; else statsLosses++; SaveStats(); }
                                        } else {
                                            winner = 3;
                                            if (aiMode) { statsDraws++; SaveStats(); }
                                        }
                                    }
                                    
                                    if (aiMode && !whiteTurn && !gameOver) {
                                        SetTimer(hwnd, 1, 300, NULL);
                                    }
                                } else {
                                    MessageBeep(MB_ICONWARNING);
                                }
                            }
                        }
                    }
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }
        case WM_DESTROY:
            if (hBgBrush) DeleteObject(hBgBrush);
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) *bytes++ = (char)c;
    return dest;
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KChessApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    hBgBrush = CreateSolidBrush(RGB(15, 23, 42));
    wc.hbrBackground = hBgBrush;
    RegisterClass(&wc);

    srand((unsigned int)time(NULL));
    LoadStats();

    HWND hwnd = CreateWindowEx(0, "KChessApp", "KChess", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, W, H, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
