#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 560
#define H 650
#define TS 50
#define OX 80
#define OY 110

// 0: empty, 1:P, 2:N, 3:B, 4:R, 5:Q, 6:K,  7:p, 8:n, 9:b, 10:r, 11:q, 12:k
int board[8][8];
int defaultBoard[8][8] = {
    {10, 8, 9, 11, 12, 9, 8, 10},
    {7,  7, 7,  7,  7, 7, 7,  7},
    {0,  0, 0,  0,  0, 0, 0,  0},
    {0,  0, 0,  0,  0, 0, 0,  0},
    {0,  0, 0,  0,  0, 0, 0,  0},
    {0,  0, 0,  0,  0, 0, 0,  0},
    {1,  1, 1,  1,  1, 1, 1,  1},
    {4,  2, 3,  5,  6, 3, 2,  4}
};

int selX = -1, selY = -1;
int whiteTurn = 1;
int gameOver = 0;
int winner = 0; // 1 = White, 2 = Black, 3 = Draw
int aiMode = 1; // 1 = vs AI, 0 = vs Player
int gameMode = 0; // 0 = Campaign, 1 = Free Play, 2 = Puzzle Mode, 3 = Blitz Timer Mode
int currentStage = 1;
int puzzleIndex = 0;
int aiPersonality = 4; // 1=Easy, 2=Medium, 3=Hard, 4=Master
char* diffNames[] = { "Easy", "Medium", "Hard", "Master" };

int statsWins = 0, statsLosses = 0, statsDraws = 0;
int pieceValues[] = {0, 100, 320, 330, 500, 900, 20000, 100, 320, 330, 500, 900, 20000};
int lastMoveSx = -1, lastMoveSy = -1, lastMoveTx = -1, lastMoveTy = -1;
HBRUSH hBgBrush = NULL;
HWND g_hwndMain = NULL;

int wKingMoved = 0, wRookLMoved = 0, wRookRMoved = 0;
int bKingMoved = 0, bRookLMoved = 0, bRookRMoved = 0;
int epX = -1, epY = -1;

// Active Skills & Powerups
int freezePowerups = 3;
int blackFrozen = 0;
int hintActive = 0;
int hintSx = -1, hintSy = -1, hintTx = -1, hintTy = -1;
char hintText[64] = {0};

// Move History Stack for Interactive Undo/Redo & PGN
typedef struct {
    int board[8][8];
    int whiteTurn;
    int wkm, wrl, wrr, bkm, brl, brr;
    int epX, epY;
    int lastMoveSx, lastMoveSy, lastMoveTx, lastMoveTy;
    char san[16];
} HistoryState;

HistoryState g_historyStack[256];
int g_historyIndex = -1;

int kbX = 4, kbY = 6;
int kbActive = 0;

// Blitz Timer
float blitzTimeWhite = 180.0f;
float blitzTimeBlack = 180.0f;

// Particles & Slide animation
typedef struct {
    float x, y, vx, vy;
    COLORREF color;
    int life;
} Particle;
Particle g_particles[64];
int g_particleCount = 0;

typedef struct {
    int p;
    float startX, startY, curX, curY, targetX, targetY;
    int active;
    DWORD startTime;
    DWORD duration;
} SlideAnim;
SlideAnim g_slide = {0};

// Freestanding CRT replacements
static unsigned int g_seed = 12345;
static int my_abs(int v) { return v < 0 ? -v : v; }
static void my_srand(unsigned int seed) { g_seed = seed; }
static int my_rand(void) { g_seed = g_seed * 1103515245 + 12345; return (int)((g_seed / 65536) % 32768); }

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) *bytes++ = (char)c;
    return dest;
}

#pragma function(memcpy)
void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}


static void CopyTextToClipboard(HWND hwnd, const char* text) {
    if (!text) return;
    int len = lstrlenA(text);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len + 1);
    if (hMem) {
        char* ptr = (char*)GlobalLock(hMem);
        if (ptr) {
            for (int i = 0; i <= len; i++) ptr[i] = text[i];
            GlobalUnlock(hMem);
            if (OpenClipboard(hwnd)) {
                EmptyClipboard();
                SetClipboardData(CF_TEXT, hMem);
                CloseClipboard();
            }
        }
    }
}

static char* GetTextFromClipboard(HWND hwnd) {
    if (!OpenClipboard(hwnd)) return NULL;
    HANDLE hData = GetClipboardData(CF_TEXT);
    char* text = NULL;
    if (hData) {
        char* ptr = (char*)GlobalLock(hData);
        if (ptr) {
            int len = lstrlenA(ptr);
            text = (char*)GlobalAlloc(GPTR, len + 1);
            if (text) {
                for (int i = 0; i <= len; i++) text[i] = ptr[i];
            }
            GlobalUnlock(hData);
        }
    }
    CloseClipboard();
    return text;
}

static void LoadStatsFreestanding(void) {
    HANDLE hFile = CreateFileA("kchess_stats.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[128] = {0};
        DWORD bytesRead = 0;
        if (ReadFile(hFile, buf, sizeof(buf) - 1, &bytesRead, NULL) && bytesRead > 0) {
            statsWins = 0; statsLosses = 0; statsDraws = 0;
            char* p = buf;
            while (*p && (*p < '0' || *p > '9')) p++;
            while (*p >= '0' && *p <= '9') { statsWins = statsWins * 10 + (*p - '0'); p++; }
            while (*p && (*p < '0' || *p > '9')) p++;
            while (*p >= '0' && *p <= '9') { statsLosses = statsLosses * 10 + (*p - '0'); p++; }
            while (*p && (*p < '0' || *p > '9')) p++;
            while (*p >= '0' && *p <= '9') { statsDraws = statsDraws * 10 + (*p - '0'); p++; }
        }
        CloseHandle(hFile);
    }
}

static void SaveStatsFreestanding(void) {
    HANDLE hFile = CreateFileA("kchess_stats.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[128];
        wsprintfA(buf, "%d %d %d", statsWins, statsLosses, statsDraws);
        DWORD written = 0;
        WriteFile(hFile, buf, (DWORD)lstrlenA(buf), &written, NULL);
        CloseHandle(hFile);
    }
}

static void SpawnCaptureSparks(int px, int py) {
    COLORREF colors[] = { RGB(245, 158, 11), RGB(239, 68, 68), RGB(254, 240, 138), RGB(255, 255, 255), RGB(56, 189, 248) };
    for (int i = 0; i < 20; i++) {
        if (g_particleCount >= 64) break;
        float rx = (float)((my_rand() % 100) - 50) / 10.0f;
        float ry = (float)((my_rand() % 100) - 50) / 10.0f;
        g_particles[g_particleCount].x = (float)px;
        g_particles[g_particleCount].y = (float)py;
        g_particles[g_particleCount].vx = rx;
        g_particles[g_particleCount].vy = ry - 0.5f;
        g_particles[g_particleCount].color = colors[my_rand() % 5];
        g_particles[g_particleCount].life = 20 + my_rand() % 15;
        g_particleCount++;
    }
}

int IsSquareAttacked(int tx, int ty, int byWhite);
int IsValidMove(int sx, int sy, int tx, int ty, int isAttackCheck);

int GetPST(int pType, int x, int y, int isWhite, int isEndgame) {
    int row = isWhite ? y : 7 - y;
    int center = (x >= 3 && x <= 4 && row >= 3 && row <= 4) ? 12 : 
                 (x >= 2 && x <= 5 && row >= 2 && row <= 5) ? 6 : 0;
    
    if (pType == 1) { // Pawn
        if (isEndgame) {
            if (row <= 1) return 80;
            if (row == 2) return 40;
            if (row == 3) return 20;
        } else {
            if (row <= 1) return 50; 
            if (row == 2) return 20;
            if (row == 3) return 10;
        }
        return 0;
    }
    if (pType == 2) return center * 3; // Knight
    if (pType == 3) return center * 2; // Bishop
    if (pType == 6) { // King
        if (isEndgame) {
            return center * 3;
        } else {
            if (row >= 6 && (x <= 2 || x >= 5)) return 20; 
            return -center * 2; 
        }
    }
    return center;
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
    int adx = my_abs(dx);
    int ady = my_abs(dy);
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
                if (tx == 6 && !wRookRMoved && board[7][7] == 4 && board[7][5] == 0 && board[7][6] == 0) {
                    if (!IsSquareAttacked(4, 7, 0) && !IsSquareAttacked(5, 7, 0) && !IsSquareAttacked(6, 7, 0)) return 1;
                }
                if (tx == 2 && !wRookLMoved && board[7][0] == 4 && board[7][1] == 0 && board[7][2] == 0 && board[7][3] == 0) {
                    if (!IsSquareAttacked(4, 7, 0) && !IsSquareAttacked(3, 7, 0) && !IsSquareAttacked(2, 7, 0)) return 1;
                }
            } else {
                if (bKingMoved) return 0;
                if (tx == 6 && !bRookRMoved && board[0][7] == 10 && board[0][5] == 0 && board[0][6] == 0) {
                    if (!IsSquareAttacked(4, 0, 1) && !IsSquareAttacked(5, 0, 1) && !IsSquareAttacked(6, 0, 1)) return 1;
                }
                if (tx == 2 && !bRookLMoved && board[0][0] == 10 && board[0][1] == 0 && board[0][2] == 0 && board[0][3] == 0) {
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

int EvaluateBoardStatic(void) {
    int score = 0;
    int wMat = 0, bMat = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int p = board[y][x];
            if (p != 0 && p != 6 && p != 12 && p != 1 && p != 7) {
                if (p <= 6) wMat += pieceValues[p];
                else bMat += pieceValues[p];
            }
        }
    }
    int isEndgame = (wMat < 1500 && bMat < 1500);

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int p = board[y][x];
            if (p == 0) continue;
            int isWhite = (p <= 6);
            int pType = isWhite ? p : p - 6;
            int val = pieceValues[p];
            int pst = GetPST(pType, x, y, isWhite, isEndgame);
            if (isWhite) score -= (val + pst);
            else score += (val + pst);
        }
    }
    return score;
}

typedef struct {
    int srcP, dstP, epX, epY, wkm, wrl, wrr, bkm, brl, brr;
    int capturedEPPawn, capturedEPX, capturedEPY;
} MoveState;

static void MakeMoveSim(int sx, int sy, int tx, int ty, MoveState* ms) {
    ms->srcP = board[sy][sx];
    ms->dstP = board[ty][tx];
    ms->epX = epX; ms->epY = epY;
    ms->wkm = wKingMoved; ms->wrl = wRookLMoved; ms->wrr = wRookRMoved;
    ms->bkm = bKingMoved; ms->brl = bRookLMoved; ms->brr = bRookRMoved;
    ms->capturedEPPawn = 0; ms->capturedEPX = -1; ms->capturedEPY = -1;

    int isWhite = (ms->srcP <= 6);
    int pType = isWhite ? ms->srcP : ms->srcP - 6;

    if (pType == 1 && tx == epX && ty == (isWhite ? epY - 1 : epY + 1)) {
        ms->capturedEPPawn = board[epY][epX];
        ms->capturedEPX = epX;
        ms->capturedEPY = epY;
        board[epY][epX] = 0;
    }

    board[ty][tx] = ms->srcP;
    board[sy][sx] = 0;

    if (pType == 1 && (ty == 0 || ty == 7)) {
        board[ty][tx] = isWhite ? 5 : 11;
    }
    if (pType == 6) { if (isWhite) wKingMoved = 1; else bKingMoved = 1; }
    if (pType == 4) {
        if (isWhite) { if (sx == 0 && sy == 7) wRookLMoved = 1; if (sx == 7 && sy == 7) wRookRMoved = 1; }
        else { if (sx == 0 && sy == 0) bRookLMoved = 1; if (sx == 7 && sy == 0) bRookRMoved = 1; }
    }
    if (tx == 0 && ty == 7) wRookLMoved = 1;
    if (tx == 7 && ty == 7) wRookRMoved = 1;
    if (tx == 0 && ty == 0) bRookLMoved = 1;
    if (tx == 7 && ty == 0) bRookRMoved = 1;

    if (pType == 1 && my_abs(ty - sy) == 2) { epX = tx; epY = ty; } else { epX = -1; epY = -1; }
}

static void UnmakeMoveSim(int sx, int sy, int tx, int ty, const MoveState* ms) {
    board[sy][sx] = ms->srcP;
    board[ty][tx] = ms->dstP;
    if (ms->capturedEPPawn != 0) {
        board[ms->capturedEPY][ms->capturedEPX] = ms->capturedEPPawn;
    }
    epX = ms->epX; epY = ms->epY;
    wKingMoved = ms->wkm; wRookLMoved = ms->wrl; wRookRMoved = ms->wrr;
    bKingMoved = ms->bkm; bRookLMoved = ms->brl; bRookRMoved = ms->brr;
}

int MinimaxAB(int depth, int alpha, int beta, int isMaximizing) {
    if (depth <= 0) return EvaluateBoardStatic();

    if (isMaximizing) { // Black's turn
        int maxEval = -999999;
        int movesFound = 0;
        for (int sy = 0; sy < 8; sy++) {
            for (int sx = 0; sx < 8; sx++) {
                if (board[sy][sx] > 6) {
                    for (int ty = 0; ty < 8; ty++) {
                        for (int tx = 0; tx < 8; tx++) {
                            if (IsValidMove(sx, sy, tx, ty, 0)) {
                                if (!SimulatedMoveLeavesCheck(sx, sy, tx, ty, 0)) {
                                    movesFound = 1;
                                    MoveState ms;
                                    MakeMoveSim(sx, sy, tx, ty, &ms);
                                    int eval = MinimaxAB(depth - 1, alpha, beta, 0);
                                    UnmakeMoveSim(sx, sy, tx, ty, &ms);
                                    if (eval > maxEval) maxEval = eval;
                                    if (eval > alpha) alpha = eval;
                                    if (beta <= alpha) return maxEval;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (!movesFound) {
            int kx = -1, ky = -1;
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    if (board[y][x] == 12) { kx = x; ky = y; break; }
                }
            }
            if (kx != -1 && IsSquareAttacked(kx, ky, 1)) {
                return -100000 + depth;
            }
            return 0;
        }
        return maxEval;
    } else { // White's turn
        int minEval = 999999;
        int movesFound = 0;
        for (int sy = 0; sy < 8; sy++) {
            for (int sx = 0; sx < 8; sx++) {
                if (board[sy][sx] != 0 && board[sy][sx] <= 6) {
                    for (int ty = 0; ty < 8; ty++) {
                        for (int tx = 0; tx < 8; tx++) {
                            if (IsValidMove(sx, sy, tx, ty, 0)) {
                                if (!SimulatedMoveLeavesCheck(sx, sy, tx, ty, 1)) {
                                    movesFound = 1;
                                    MoveState ms;
                                    MakeMoveSim(sx, sy, tx, ty, &ms);
                                    int eval = MinimaxAB(depth - 1, alpha, beta, 1);
                                    UnmakeMoveSim(sx, sy, tx, ty, &ms);
                                    if (eval < minEval) minEval = eval;
                                    if (eval < beta) beta = eval;
                                    if (beta <= alpha) return minEval;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (!movesFound) {
            int kx = -1, ky = -1;
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    if (board[y][x] == 6) { kx = x; ky = y; break; }
                }
            }
            if (kx != -1 && IsSquareAttacked(kx, ky, 0)) {
                return 100000 - depth;
            }
            return 0;
        }
        return minEval;
    }
}

void GetOptimalHintMove(int* outSx, int* outSy, int* outTx, int* outTy) {
    *outSx = -1; *outSy = -1; *outTx = -1; *outTy = -1;
    int bestValue = whiteTurn ? 999999 : -999999;
    for (int sy = 0; sy < 8; sy++) {
        for (int sx = 0; sx < 8; sx++) {
            int p = board[sy][sx];
            if (p != 0 && (p <= 6) == whiteTurn) {
                for (int ty = 0; ty < 8; ty++) {
                    for (int tx = 0; tx < 8; tx++) {
                        if (IsValidMove(sx, sy, tx, ty, 0)) {
                            if (!SimulatedMoveLeavesCheck(sx, sy, tx, ty, whiteTurn)) {
                                MoveState ms;
                                MakeMoveSim(sx, sy, tx, ty, &ms);
                                int score = MinimaxAB(2, -999999, 999999, !whiteTurn);
                                UnmakeMoveSim(sx, sy, tx, ty, &ms);
                                if (whiteTurn) {
                                    if (score < bestValue) {
                                        bestValue = score;
                                        *outSx = sx; *outSy = sy; *outTx = tx; *outTy = ty;
                                    }
                                } else {
                                    if (score > bestValue) {
                                        bestValue = score;
                                        *outSx = sx; *outSy = sy; *outTx = tx; *outTy = ty;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

static void GetSAN(int sx, int sy, int tx, int ty, int p, int isCapture, char* outBuf) {
    int pType = p > 6 ? p - 6 : p;
    char files[] = "abcdefgh";
    char targetSquare[4];
    targetSquare[0] = files[tx];
    targetSquare[1] = '8' - ty;
    targetSquare[2] = '\0';

    if (pType == 6 && my_abs(tx - sx) == 2) {
        if (tx == 6) { wsprintfA(outBuf, "O-O"); }
        else { wsprintfA(outBuf, "O-O-O"); }
        return;
    }

    char pLetters[] = "  NBRQK";
    char letter = pLetters[pType];

    if (pType == 1) { // Pawn
        if (isCapture) {
            wsprintfA(outBuf, "%cx%s", files[sx], targetSquare);
        } else {
            wsprintfA(outBuf, "%s", targetSquare);
        }
    } else {
        if (isCapture) {
            wsprintfA(outBuf, "%cx%s", letter, targetSquare);
        } else {
            wsprintfA(outBuf, "%c%s", letter, targetSquare);
        }
    }
}

static void PushHistoryState(const char* san) {
    if (g_historyIndex < 255) {
        g_historyIndex++;
        HistoryState* st = &g_historyStack[g_historyIndex];
        for (int y = 0; y < 8; y++) for (int x = 0; x < 8; x++) st->board[y][x] = board[y][x];
        st->whiteTurn = whiteTurn;
        st->wkm = wKingMoved; st->wrl = wRookLMoved; st->wrr = wRookRMoved;
        st->bkm = bKingMoved; st->brl = bRookLMoved; st->brr = bRookRMoved;
        st->epX = epX; st->epY = epY;
        st->lastMoveSx = lastMoveSx; st->lastMoveSy = lastMoveSy;
        st->lastMoveTx = lastMoveTx; st->lastMoveTy = lastMoveTy;
        if (san) lstrcpyA(st->san, san); else st->san[0] = '\0';
    }
}

static void RestoreHistoryState(int idx) {
    if (idx < 0 || idx > g_historyIndex) return;
    HistoryState* st = &g_historyStack[idx];
    for (int y = 0; y < 8; y++) for (int x = 0; x < 8; x++) board[y][x] = st->board[y][x];
    whiteTurn = st->whiteTurn;
    wKingMoved = st->wkm; wRookLMoved = st->wrl; wRookRMoved = st->wrr;
    bKingMoved = st->bkm; bRookLMoved = st->brl; bRookRMoved = st->brr;
    epX = st->epX; epY = st->epY;
    lastMoveSx = st->lastMoveSx; lastMoveSy = st->lastMoveSy;
    lastMoveTx = st->lastMoveTx; lastMoveTy = st->lastMoveTy;
    g_historyIndex = idx;
    selX = -1; selY = -1; hintActive = 0; gameOver = 0; winner = 0;
}

static void UndoMove(void) {
    if (g_historyIndex <= 0) return;
    int target = g_historyIndex - 1;
    if (aiMode && target > 0 && g_historyStack[target].whiteTurn != 1) {
        target--;
    }
    RestoreHistoryState(target);
    MessageBeep(MB_OK);
}

static void RedoMove(void) {
    if (g_historyIndex >= 255) return;
    if (g_historyStack[g_historyIndex + 1].san[0] == '\0' && g_historyIndex >= 0) return;
    int target = g_historyIndex + 1;
    if (aiMode && target < 255 && g_historyStack[target].whiteTurn != 1) {
        target++;
    }
    RestoreHistoryState(target);
    MessageBeep(MB_OK);
}

static void GenerateFEN(char* outFen) {
    int pos = 0;
    const char pieceMap[] = " PNBQKpnbqk";
    for (int y = 0; y < 8; y++) {
        int empty = 0;
        for (int x = 0; x < 8; x++) {
            int p = board[y][x];
            if (p == 0) {
                empty++;
            } else {
                if (empty > 0) {
                    outFen[pos++] = '0' + empty;
                    empty = 0;
                }
                outFen[pos++] = pieceMap[p];
            }
        }
        if (empty > 0) outFen[pos++] = '0' + empty;
        if (y < 7) outFen[pos++] = '/';
    }
    outFen[pos++] = ' ';
    outFen[pos++] = whiteTurn ? 'w' : 'b';
    outFen[pos++] = ' ';

    int hasCastle = 0;
    if (!wKingMoved) {
        if (!wRookRMoved && board[7][7] == 4) { outFen[pos++] = 'K'; hasCastle = 1; }
        if (!wRookLMoved && board[7][0] == 4) { outFen[pos++] = 'Q'; hasCastle = 1; }
    }
    if (!bKingMoved) {
        if (!bRookRMoved && board[0][7] == 10) { outFen[pos++] = 'k'; hasCastle = 1; }
        if (!bRookLMoved && board[0][0] == 10) { outFen[pos++] = 'q'; hasCastle = 1; }
    }
    if (!hasCastle) outFen[pos++] = '-';
    outFen[pos++] = ' ';

    if (epX != -1 && epY != -1) {
        outFen[pos++] = 'a' + epX;
        outFen[pos++] = '8' - (whiteTurn ? epY - 1 : epY + 1);
    } else {
        outFen[pos++] = '-';
    }

    outFen[pos++] = ' ';
    outFen[pos++] = '0';
    outFen[pos++] = ' ';
    wsprintfA(outFen + pos, "%d", (g_historyIndex > 0 ? g_historyIndex / 2 + 1 : 1));
}

static int LoadFEN(const char* fen) {
    if (!fen || lstrlenA(fen) < 5) return 0;
    int y = 0, x = 0;
    int pBoard[8][8];
    for (int r = 0; r < 8; r++) for (int c = 0; c < 8; c++) pBoard[r][c] = 0;

    const char* p = fen;
    while (*p && y < 8) {
        if (*p == '/') {
            y++; x = 0;
        } else if (*p >= '1' && *p <= '8') {
            x += (*p - '0');
        } else {
            int pVal = 0;
            switch (*p) {
                case 'P': pVal = 1; break; case 'N': pVal = 2; break;
                case 'B': pVal = 3; break; case 'R': pVal = 4; break;
                case 'Q': pVal = 5; break; case 'K': pVal = 6; break;
                case 'p': pVal = 7; break; case 'n': pVal = 8; break;
                case 'b': pVal = 9; break; case 'r': pVal = 10; break;
                case 'q': pVal = 11; break; case 'k': pVal = 12; break;
            }
            if (pVal > 0 && x < 8) {
                pBoard[y][x] = pVal;
                x++;
            }
        }
        p++;
        if (*p == ' ') break;
    }

    for (int r = 0; r < 8; r++) for (int c = 0; c < 8; c++) board[r][c] = pBoard[r][c];

    while (*p && *p == ' ') p++;
    if (*p == 'b') whiteTurn = 0; else whiteTurn = 1;
    while (*p && *p != ' ') p++;

    wKingMoved = 1; wRookLMoved = 1; wRookRMoved = 1;
    bKingMoved = 1; bRookLMoved = 1; bRookRMoved = 1;
    while (*p && *p == ' ') p++;
    if (*p && *p != '-') {
        while (*p && *p != ' ') {
            if (*p == 'K') { wKingMoved = 0; wRookRMoved = 0; }
            if (*p == 'Q') { wKingMoved = 0; wRookLMoved = 0; }
            if (*p == 'k') { bKingMoved = 0; bRookRMoved = 0; }
            if (*p == 'q') { bKingMoved = 0; bRookLMoved = 0; }
            p++;
        }
    } else if (*p == '-') { p++; }

    epX = -1; epY = -1;
    while (*p && *p == ' ') p++;
    if (*p >= 'a' && *p <= 'h') {
        epX = *p - 'a';
        p++;
        if (*p >= '1' && *p <= '8') {
            int rank = *p - '0';
            epY = 8 - rank;
        }
    }

    selX = -1; selY = -1; lastMoveSx = -1; lastMoveSy = -1; lastMoveTx = -1; lastMoveTy = -1;
    g_historyIndex = -1;
    PushHistoryState("");
    gameOver = 0; winner = 0; hintActive = 0;
    return 1;
}

static void GeneratePGN(char* outBuf, int maxLen) {
    int pos = wsprintfA(outBuf,
        "[Event \"KChess Match\"]\r\n"
        "[Site \"KiloOS\"]\r\n"
        "[Date \"2026.07.28\"]\r\n"
        "[White \"White Player\"]\r\n"
        "[Black \"%s\"]\r\n"
        "[Result \"%s\"]\r\n\r\n",
        aiMode ? diffNames[aiPersonality - 1] : "Black Player",
        gameOver ? (winner == 1 ? "1-0" : (winner == 2 ? "0-1" : "1/2-1/2")) : "*"
    );

    int moveNum = 1;
    for (int i = 1; i <= g_historyIndex; i++) {
        if (g_historyStack[i].san[0] != '\0') {
            if (i % 2 == 1) {
                pos += wsprintfA(outBuf + pos, "%d. %s ", moveNum, g_historyStack[i].san);
            } else {
                pos += wsprintfA(outBuf + pos, "%s ", g_historyStack[i].san);
                moveNum++;
            }
        }
        if (pos > maxLen - 64) break;
    }
    wsprintfA(outBuf + pos, "%s", gameOver ? (winner == 1 ? "1-0" : (winner == 2 ? "0-1" : "1/2-1/2")) : "*");
}

void ResetGame(void) {
    wKingMoved = 0; wRookLMoved = 0; wRookRMoved = 0;
    bKingMoved = 0; bRookLMoved = 0; bRookRMoved = 0;
    epX = -1; epY = -1;
    hintActive = 0; hintSx = -1; hintSy = -1; hintTx = -1; hintTy = -1; hintText[0] = '\0';
    blackFrozen = 0;
    blitzTimeWhite = 180.0f; blitzTimeBlack = 180.0f;

    for (int y = 0; y < 8; y++) for (int x = 0; x < 8; x++) board[y][x] = 0;

    int targetStage = currentStage;
    if (gameMode == 2) {
        int puzzleStageMap[] = { 1, 2, 3, 4, 11, 19 };
        targetStage = puzzleStageMap[puzzleIndex % 6];
    }

    if (gameMode == 0 || gameMode == 2) {
        switch (targetStage) {
            case 1:
                board[0][4] = 12; board[1][0] = 7; board[1][1] = 7; board[1][2] = 7;
                board[1][5] = 7; board[1][6] = 7; board[1][7] = 7;
                board[7][6] = 6; board[6][5] = 1; board[6][6] = 1; board[6][7] = 1; board[7][0] = 4;
                aiPersonality = 1; break;
            case 2:
                board[0][7] = 12; board[0][6] = 10; board[1][6] = 7; board[1][7] = 7;
                board[7][5] = 6; board[2][4] = 2;
                aiPersonality = 1; break;
            case 3:
                board[0][4] = 12; board[0][2] = 10;
                for(int x=0; x<8; x++) if(x!=3 && x!=4) board[1][x] = 7;
                board[7][6] = 6; board[6][5] = 1; board[6][6] = 1; board[6][7] = 1;
                board[1][2] = 5; board[7][0] = 4;
                aiPersonality = 2; break;
            case 4:
                board[0][6] = 12; board[1][5] = 7; board[1][6] = 7; board[1][7] = 7; board[2][5] = 8;
                board[7][6] = 6; board[6][5] = 1; board[6][6] = 1; board[6][7] = 1;
                board[5][3] = 3; board[6][2] = 5;
                aiPersonality = 2; break;
            case 5:
                board[2][5] = 12; board[3][7] = 7; board[4][2] = 6; board[4][0] = 1;
                aiPersonality = 3; break;
            case 6:
                board[1][5] = 12; board[6][0] = 10; board[0][3] = 6; board[1][3] = 1; board[7][4] = 4;
                aiPersonality = 3; break;
            case 7:
                for(int y=0;y<8;y++) for(int x=0;x<8;x++) board[y][x] = defaultBoard[y][x];
                board[0][0] = 0; board[0][7] = 0; aiPersonality = 1; break;
            case 8:
                for(int y=0;y<8;y++) for(int x=0;x<8;x++) board[y][x] = defaultBoard[y][x];
                board[0][3] = 0; aiPersonality = 2; break;
            case 9:
                for(int y=0;y<8;y++) for(int x=0;x<8;x++) board[y][x] = defaultBoard[y][x];
                board[2][2] = 7; board[2][3] = 7; board[2][4] = 7; board[2][5] = 7;
                aiPersonality = 3; break;
            case 10:
                for(int y=0;y<8;y++) for(int x=0;x<8;x++) board[y][x] = defaultBoard[y][x];
                aiPersonality = 1; break;
            case 11:
                board[0][7] = 12; board[1][6] = 7; board[1][7] = 7; board[1][5] = 6; board[5][3] = 3; board[2][4] = 2;
                aiPersonality = 2; break;
            case 12:
                board[0][4] = 12; board[1][4] = 10; board[4][4] = 6; board[4][3] = 5;
                aiPersonality = 3; break;
            case 13:
                for(int y=0;y<8;y++) for(int x=0;x<8;x++) board[y][x] = defaultBoard[y][x];
                aiPersonality = 2; break;
            case 14:
                board[0][6] = 12; board[0][5] = 10; board[1][6] = 7; board[1][7] = 7;
                board[7][6] = 6; board[6][5] = 1; board[6][6] = 1; board[6][7] = 1;
                board[2][5] = 5; board[7][4] = 4;
                aiPersonality = 3; break;
            case 15:
                for(int y=0;y<8;y++) for(int x=0;x<8;x++) board[y][x] = defaultBoard[y][x];
                aiPersonality = 3; break;
            case 16:
                board[2][4] = 12; board[0][0] = 10; board[0][7] = 10; board[2][3] = 7; board[3][4] = 7;
                board[5][4] = 6; board[7][0] = 4; board[7][7] = 4; board[4][3] = 1; board[4][4] = 1;
                aiPersonality = 4; break;
            case 17:
                for(int y=0;y<8;y++) for(int x=0;x<8;x++) board[y][x] = defaultBoard[y][x];
                board[0][3] = 11; board[0][4] = 11; board[0][2] = 12;
                aiPersonality = 4; break;
            case 18:
                for(int y=0;y<8;y++) for(int x=0;x<8;x++) board[y][x] = defaultBoard[y][x];
                aiPersonality = 4; break;
            case 19:
                board[0][2] = 12; board[0][3] = 10; board[1][0] = 7; board[1][1] = 7; board[1][2] = 7;
                board[7][6] = 6; board[4][5] = 3; board[5][4] = 3; board[2][2] = 5;
                aiPersonality = 4; break;
            case 20:
            default:
                for(int y=0;y<8;y++) for(int x=0;x<8;x++) board[y][x] = defaultBoard[y][x];
                aiPersonality = 4; break;
        }
    } else {
        for(int y=0;y<8;y++) for(int x=0;x<8;x++) board[y][x] = defaultBoard[y][x];
    }

    freezePowerups = 3;
    selX = -1; selY = -1;
    whiteTurn = 1;
    gameOver = 0;
    winner = 0;
    lastMoveSx = -1; lastMoveSy = -1; lastMoveTx = -1; lastMoveTy = -1;
    g_particleCount = 0;
    g_slide.active = 0;

    g_historyIndex = -1;
    PushHistoryState("");
}

static void DrawChessPiece(HDC hdc, int p, int x, int y, int ts) {
    if (p == 0) return;
    int isWhite = (p <= 6);
    int pType = isWhite ? p : p - 6;

    int cx = x + ts / 2;
    int cy = y + ts / 2;
    int r = (int)(ts * 0.42f);

    COLORREF fillColor = isWhite ? RGB(250, 248, 240) : RGB(30, 40, 55);
    COLORREF outlineColor = isWhite ? RGB(15, 23, 42) : RGB(226, 232, 240);
    COLORREF detailColor = isWhite ? RGB(71, 85, 105) : RGB(203, 213, 225);
    COLORREF goldColor = RGB(245, 158, 11);

    HBRUSH fillBrush = CreateSolidBrush(fillColor);
    HPEN mainPen = CreatePen(PS_SOLID, 2, outlineColor);
    HPEN detailPen = CreatePen(PS_SOLID, 1, detailColor);
    HPEN goldPen = CreatePen(PS_SOLID, 1, goldColor);
    HBRUSH goldBrush = CreateSolidBrush(goldColor);

    HGDIOBJ oldBrush = SelectObject(hdc, fillBrush);
    HGDIOBJ oldPen = SelectObject(hdc, mainPen);

    // Draw Base
    Ellipse(hdc, cx - (int)(r * 0.72f), cy + (int)(r * 0.50f), cx + (int)(r * 0.72f), cy + (int)(r * 0.82f));
    Ellipse(hdc, cx - (int)(r * 0.58f), cy + (int)(r * 0.38f), cx + (int)(r * 0.58f), cy + (int)(r * 0.64f));

    if (pType == 1) { // PAWN
        POINT stem[4] = {
            { cx - (int)(r * 0.38f), cy + (int)(r * 0.48f) },
            { cx - (int)(r * 0.22f), cy - (int)(r * 0.08f) },
            { cx + (int)(r * 0.22f), cy - (int)(r * 0.08f) },
            { cx + (int)(r * 0.38f), cy + (int)(r * 0.48f) }
        };
        Polygon(hdc, stem, 4);
        Ellipse(hdc, cx - (int)(r * 0.36f), cy - (int)(r * 0.16f), cx + (int)(r * 0.36f), cy - (int)(r * 0.02f));
        Ellipse(hdc, cx - (int)(r * 0.34f), cy - (int)(r * 0.72f), cx + (int)(r * 0.34f), cy - (int)(r * 0.08f));
        Ellipse(hdc, cx - (int)(r * 0.08f), cy - (int)(r * 0.86f), cx + (int)(r * 0.08f), cy - (int)(r * 0.70f));
    } else if (pType == 2) { // KNIGHT
        POINT knight[8] = {
            { cx - (int)(r * 0.52f), cy + (int)(r * 0.48f) },
            { cx - (int)(r * 0.50f), cy - (int)(r * 0.10f) },
            { cx - (int)(r * 0.22f), cy - (int)(r * 0.68f) },
            { cx - (int)(r * 0.10f), cy - (int)(r * 0.48f) },
            { cx + (int)(r * 0.58f), cy - (int)(r * 0.25f) },
            { cx + (int)(r * 0.50f), cy - (int)(r * 0.05f) },
            { cx + (int)(r * 0.15f), cy - (int)(r * 0.05f) },
            { cx + (int)(r * 0.48f), cy + (int)(r * 0.48f) }
        };
        Polygon(hdc, knight, 8);
        SelectObject(hdc, isWhite ? GetStockObject(BLACK_BRUSH) : GetStockObject(WHITE_BRUSH));
        Ellipse(hdc, cx + (int)(r * 0.12f), cy - (int)(r * 0.35f), cx + (int)(r * 0.24f), cy - (int)(r * 0.23f));
    } else if (pType == 3) { // BISHOP
        POINT stem[4] = {
            { cx - (int)(r * 0.40f), cy + (int)(r * 0.48f) },
            { cx - (int)(r * 0.24f), cy - (int)(r * 0.12f) },
            { cx + (int)(r * 0.24f), cy - (int)(r * 0.12f) },
            { cx + (int)(r * 0.40f), cy + (int)(r * 0.48f) }
        };
        Polygon(hdc, stem, 4);
        Ellipse(hdc, cx - (int)(r * 0.38f), cy - (int)(r * 0.80f), cx + (int)(r * 0.38f), cy - (int)(r * 0.10f));
        Ellipse(hdc, cx - (int)(r * 0.10f), cy - (int)(r * 0.96f), cx + (int)(r * 0.10f), cy - (int)(r * 0.76f));
        SelectObject(hdc, detailPen);
        MoveToEx(hdc, cx - (int)(r * 0.18f), cy - (int)(r * 0.55f), NULL);
        LineTo(hdc, cx + (int)(r * 0.15f), cy - (int)(r * 0.35f));
    } else if (pType == 4) { // ROOK
        POINT body[4] = {
            { cx - (int)(r * 0.42f), cy + (int)(r * 0.48f) },
            { cx - (int)(r * 0.34f), cy - (int)(r * 0.20f) },
            { cx + (int)(r * 0.34f), cy - (int)(r * 0.20f) },
            { cx + (int)(r * 0.42f), cy + (int)(r * 0.48f) }
        };
        Polygon(hdc, body, 4);
        POINT battlement[12] = {
            { cx - (int)(r * 0.44f), cy - (int)(r * 0.20f) },
            { cx - (int)(r * 0.44f), cy - (int)(r * 0.70f) },
            { cx - (int)(r * 0.26f), cy - (int)(r * 0.70f) },
            { cx - (int)(r * 0.26f), cy - (int)(r * 0.50f) },
            { cx - (int)(r * 0.08f), cy - (int)(r * 0.50f) },
            { cx - (int)(r * 0.08f), cy - (int)(r * 0.70f) },
            { cx + (int)(r * 0.08f), cy - (int)(r * 0.70f) },
            { cx + (int)(r * 0.08f), cy - (int)(r * 0.50f) },
            { cx + (int)(r * 0.26f), cy - (int)(r * 0.50f) },
            { cx + (int)(r * 0.26f), cy - (int)(r * 0.70f) },
            { cx + (int)(r * 0.44f), cy - (int)(r * 0.70f) },
            { cx + (int)(r * 0.44f), cy - (int)(r * 0.20f) }
        };
        Polygon(hdc, battlement, 12);
        SelectObject(hdc, detailPen);
        MoveToEx(hdc, cx - (int)(r * 0.36f), cy, NULL);
        LineTo(hdc, cx + (int)(r * 0.36f), cy);
    } else if (pType == 5) { // QUEEN
        POINT stem[4] = {
            { cx - (int)(r * 0.42f), cy + (int)(r * 0.48f) },
            { cx - (int)(r * 0.24f), cy - (int)(r * 0.10f) },
            { cx + (int)(r * 0.24f), cy - (int)(r * 0.10f) },
            { cx + (int)(r * 0.42f), cy + (int)(r * 0.48f) }
        };
        Polygon(hdc, stem, 4);
        POINT crown[7] = {
            { cx - (int)(r * 0.45f), cy - (int)(r * 0.10f) },
            { cx - (int)(r * 0.48f), cy - (int)(r * 0.60f) },
            { cx - (int)(r * 0.24f), cy - (int)(r * 0.32f) },
            { cx, cy - (int)(r * 0.78f) },
            { cx + (int)(r * 0.24f), cy - (int)(r * 0.32f) },
            { cx + (int)(r * 0.48f), cy - (int)(r * 0.60f) },
            { cx + (int)(r * 0.45f), cy - (int)(r * 0.10f) }
        };
        Polygon(hdc, crown, 7);
        SelectObject(hdc, goldBrush); SelectObject(hdc, goldPen);
        Ellipse(hdc, cx - (int)(r * 0.54f), cy - (int)(r * 0.68f), cx - (int)(r * 0.42f), cy - (int)(r * 0.52f));
        Ellipse(hdc, cx - (int)(r * 0.08f), cy - (int)(r * 0.86f), cx + (int)(r * 0.08f), cy - (int)(r * 0.70f));
        Ellipse(hdc, cx + (int)(r * 0.42f), cy - (int)(r * 0.68f), cx + (int)(r * 0.54f), cy - (int)(r * 0.52f));
    } else if (pType == 6) { // KING
        POINT stem[4] = {
            { cx - (int)(r * 0.45f), cy + (int)(r * 0.48f) },
            { cx - (int)(r * 0.24f), cy - (int)(r * 0.15f) },
            { cx + (int)(r * 0.24f), cy - (int)(r * 0.15f) },
            { cx + (int)(r * 0.45f), cy + (int)(r * 0.48f) }
        };
        Polygon(hdc, stem, 4);
        RECT rim = { cx - (int)(r * 0.42f), cy - (int)(r * 0.50f), cx + (int)(r * 0.42f), cy - (int)(r * 0.20f) };
        Rectangle(hdc, rim.left, rim.top, rim.right, rim.bottom);
        SelectObject(hdc, goldBrush); SelectObject(hdc, goldPen);
        RECT vbar = { cx - (int)(r * 0.08f), cy - (int)(r * 0.92f), cx + (int)(r * 0.08f), cy - (int)(r * 0.50f) };
        RECT hbar = { cx - (int)(r * 0.22f), cy - (int)(r * 0.80f), cx + (int)(r * 0.22f), cy - (int)(r * 0.66f) };
        Rectangle(hdc, vbar.left, vbar.top, vbar.right, vbar.bottom);
        Rectangle(hdc, hbar.left, hbar.top, hbar.right, hbar.bottom);
    }

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(fillBrush);
    DeleteObject(mainPen);
    DeleteObject(detailPen);
    DeleteObject(goldPen);
    DeleteObject(goldBrush);
}

static void DrawHintArrow(HDC hdc, int sx, int sy, int tx, int ty) {
    int fromX = OX + sx * TS + TS / 2;
    int fromY = OY + sy * TS + TS / 2;
    int toX = OX + tx * TS + TS / 2;
    int toY = OY + ty * TS + TS / 2;

    HPEN cyanPen = CreatePen(PS_SOLID, 4, RGB(56, 189, 248));
    HGDIOBJ oldPen = SelectObject(hdc, cyanPen);
    MoveToEx(hdc, fromX, fromY, NULL);
    LineTo(hdc, toX, toY);

    int dx = toX - fromX;
    int dy = toY - fromY;
    float len = (float)my_abs(dx) + (float)my_abs(dy);
    if (len < 1.0f) len = 1.0f;
    float ux = (float)dx / len;
    float uy = (float)dy / len;
    float px = -uy;
    float py = ux;

    POINT arrowHead[3] = {
        { toX, toY },
        { (int)(toX - ux * 14.0f + px * 7.0f), (int)(toY - uy * 14.0f + py * 7.0f) },
        { (int)(toX - ux * 14.0f - px * 7.0f), (int)(toY - uy * 14.0f - py * 7.0f) }
    };
    HBRUSH cyanBrush = CreateSolidBrush(RGB(56, 189, 248));
    HGDIOBJ oldBrush = SelectObject(hdc, cyanBrush);
    Polygon(hdc, arrowHead, 3);
    SelectObject(hdc, oldBrush);
    DeleteObject(cyanBrush);

    SelectObject(hdc, oldPen);
    DeleteObject(cyanPen);
}

static void TriggerMove(HWND hwnd, int sx, int sy, int tx, int ty) {
    int isCaptured = (board[ty][tx] != 0);
    int p = board[sy][sx];
    int pType = p > 6 ? p - 6 : p;
    int isWhiteMove = p <= 6;

    if (pType == 1 && tx == epX && ty == (isWhiteMove ? epY - 1 : epY + 1)) {
        board[epY][epX] = 0;
        isCaptured = 1;
    }

    char san[16];
    GetSAN(sx, sy, tx, ty, p, isCaptured, san);

    // Slide animation
    g_slide.p = p;
    g_slide.startX = (float)(OX + sx * TS);
    g_slide.startY = (float)(OY + sy * TS);
    g_slide.targetX = (float)(OX + tx * TS);
    g_slide.targetY = (float)(OY + ty * TS);
    g_slide.curX = g_slide.startX;
    g_slide.curY = g_slide.startY;
    g_slide.startTime = GetTickCount();
    g_slide.duration = 150;
    g_slide.active = 1;

    board[ty][tx] = p;
    if (pType == 1) {
        if (isWhiteMove && ty == 0) board[ty][tx] = 5;
        else if (!isWhiteMove && ty == 7) board[ty][tx] = 11;
    }
    if (pType == 6 && my_abs(tx - sx) == 2) {
        if (tx == 6) { board[ty][5] = board[ty][7]; board[ty][7] = 0; }
        else if (tx == 2) { board[ty][3] = board[ty][0]; board[ty][0] = 0; }
    }
    if (pType == 6) {
        if (isWhiteMove) wKingMoved = 1; else bKingMoved = 1;
    }
    if (pType == 4) {
        if (isWhiteMove) { if (sx == 0 && sy == 7) wRookLMoved = 1; if (sx == 7 && sy == 7) wRookRMoved = 1; }
        else { if (sx == 0 && sy == 0) bRookLMoved = 1; if (sx == 7 && sy == 0) bRookRMoved = 1; }
    }
    if (tx == 0 && ty == 7) wRookLMoved = 1;
    if (tx == 7 && ty == 7) wRookRMoved = 1;
    if (tx == 0 && ty == 0) bRookLMoved = 1;
    if (tx == 7 && ty == 0) bRookRMoved = 1;

    if (pType == 1 && my_abs(ty - sy) == 2) { epX = tx; epY = ty; } else { epX = -1; epY = -1; }

    board[sy][sx] = 0;
    lastMoveSx = sx; lastMoveSy = sy; lastMoveTx = tx; lastMoveTy = ty;

    if (isCaptured) {
        SpawnCaptureSparks(OX + tx * TS + TS / 2, OY + ty * TS + TS / 2);
        MessageBeep(MB_OK);
    } else {
        MessageBeep(MB_OK);
    }

    whiteTurn = !whiteTurn;
    PushHistoryState(san);

    int pieceCount = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (board[r][c] != 0) pieceCount++;
        }
    }
    if (pieceCount <= 2) {
        gameOver = 1;
        winner = 3;
        if (aiMode) { statsDraws++; SaveStatsFreestanding(); }
        SetTimer(hwnd, 2, 30, NULL);
        return;
    }

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
            if (aiMode) { if (winner == 1) statsWins++; else statsLosses++; SaveStatsFreestanding(); }
        } else {
            winner = 3;
            if (aiMode) { statsDraws++; SaveStatsFreestanding(); }
        }
    }

    SetTimer(hwnd, 2, 30, NULL);
}

void DoBlackAIMove(void) {
    if (gameOver || whiteTurn || !aiMode) return;
    if (blackFrozen) {
        blackFrozen = 0;
        whiteTurn = 1;
        InvalidateRect(g_hwndMain, NULL, FALSE);
        return;
    }

    struct AIMove { int sx, sy, tx, ty, score; } moves[512];
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

                                int score = 0;
                                int dstP = board[ty][tx];
                                int pType = board[sy][sx] - 6;

                                int isEndgame = 0; // We can approximate or recalculate
                                int wMat = 0, bMat = 0;
                                for (int ry = 0; ry < 8; ry++) {
                                    for (int rx = 0; rx < 8; rx++) {
                                        int rp = board[ry][rx];
                                        if (rp != 0 && rp != 6 && rp != 12 && rp != 1 && rp != 7) {
                                            if (rp <= 6) wMat += pieceValues[rp]; else bMat += pieceValues[rp];
                                        }
                                    }
                                }
                                isEndgame = (wMat < 1500 && bMat < 1500);

                                if (aiPersonality == 1) { // Easy
                                    score = (dstP != 0 ? pieceValues[dstP] : 0) + (my_rand() % 40 - 20);
                                    if (IsSquareAttacked(tx, ty, 1)) score -= (pieceValues[board[sy][sx]] / 2);
                                } else if (aiPersonality == 2) { // Medium
                                    score = (int)(dstP != 0 ? pieceValues[dstP] * 1.5f : 0) + GetPST(pType, tx, ty, 0, isEndgame);
                                    if (IsSquareAttacked(tx, ty, 1)) score -= (pieceValues[board[sy][sx]] / 2);
                                } else if (aiPersonality == 3) { // Hard
                                    MoveState ms;
                                    MakeMoveSim(sx, sy, tx, ty, &ms);
                                    score = MinimaxAB(1, -999999, 999999, 0); // 2-ply total
                                    UnmakeMoveSim(sx, sy, tx, ty, &ms);
                                } else { // Master Minimax
                                    MoveState ms;
                                    MakeMoveSim(sx, sy, tx, ty, &ms);
                                    int searchDepth = (gameMode == 0 && currentStage == 20) ? 4 : 3;
                                    score = MinimaxAB(searchDepth - 1, -999999, 999999, 0);
                                    UnmakeMoveSim(sx, sy, tx, ty, &ms);
                                }
                                moves[moveCount].score = score;
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
        for (int i = 0; i < moveCount; i++) {
            if (moves[i].score > bestScore) bestScore = moves[i].score;
        }
        int bestMoves[512], bestCount = 0;
        for (int i = 0; i < moveCount; i++) {
            if (moves[i].score == bestScore) bestMoves[bestCount++] = i;
        }
        int chosen = bestMoves[my_rand() % bestCount];
        TriggerMove(g_hwndMain, moves[chosen].sx, moves[chosen].sy, moves[chosen].tx, moves[chosen].ty);
        InvalidateRect(g_hwndMain, NULL, FALSE);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            g_hwndMain = hwnd;
            ResetGame();
            SetTimer(hwnd, 3, 100, NULL); // Blitz timer tick
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, W, H);
            HGDIOBJ oldBM = SelectObject(memDC, memBM);

            HBRUSH bgBrush = CreateSolidBrush(RGB(15, 15, 22));
            RECT fullRc = {0, 0, W, H};
            FillRect(memDC, &fullRc, bgBrush);
            DeleteObject(bgBrush);

            HBRUSH frameBrush = CreateSolidBrush(RGB(45, 24, 16));
            RECT frameRc = {12, 12, W - 12, H - 12};
            FillRect(memDC, &frameRc, frameBrush);
            DeleteObject(frameBrush);

            HPEN goldPen = CreatePen(PS_SOLID, 2, RGB(212, 175, 55));
            HGDIOBJ oldPen = SelectObject(memDC, goldPen);
            HGDIOBJ oldNullBrush = SelectObject(memDC, GetStockObject(NULL_BRUSH));
            RoundRect(memDC, 12, 12, W - 12, H - 12, 16, 16);
            Rectangle(memDC, OX - 4, OY - 4, OX + TS * 8 + 4, OY + TS * 8 + 4);
            SelectObject(memDC, oldNullBrush);
            SelectObject(memDC, oldPen);
            DeleteObject(goldPen);

            HFONT labelFont = CreateFontA(13, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            HGDIOBJ oldFont = SelectObject(memDC, labelFont);
            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(212, 175, 55));

            char files[] = "ABCDEFGH";
            for (int i = 0; i < 8; i++) {
                char fStr[2] = { files[i], 0 };
                char rStr[2] = { '8' - i, 0 };
                RECT fTop = { OX + i * TS, OY - 22, OX + (i + 1) * TS, OY - 4 };
                RECT fBot = { OX + i * TS, OY + TS * 8 + 4, OX + (i + 1) * TS, OY + TS * 8 + 22 };
                RECT rLeft = { OX - 22, OY + i * TS, OX - 4, OY + (i + 1) * TS };
                RECT rRight = { OX + TS * 8 + 4, OY + i * TS, OX + TS * 8 + 22, OY + TS * 8 + 22 };
                DrawTextA(memDC, fStr, 1, &fTop, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                DrawTextA(memDC, fStr, 1, &fBot, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                DrawTextA(memDC, rStr, 1, &rLeft, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                DrawTextA(memDC, rStr, 1, &rRight, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            SelectObject(memDC, oldFont);
            DeleteObject(labelFont);

            int inCheckX = -1, inCheckY = -1;
            for (int cy = 0; cy < 8; cy++) {
                for (int cx = 0; cx < 8; cx++) {
                    int p = board[cy][cx];
                    if (p == 6 || p == 12) {
                        int isWhite = p <= 6;
                        if (IsSquareAttacked(cx, cy, !isWhite)) {
                            inCheckX = cx; inCheckY = cy;
                        }
                    }
                }
            }

            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    RECT rc = { OX + x * TS, OY + y * TS, OX + (x + 1) * TS, OY + (y + 1) * TS };
                    HBRUSH brush;
                    
                    if (x == inCheckX && y == inCheckY) {
                        brush = CreateSolidBrush(RGB(239, 68, 68));
                    } else if (x == selX && y == selY) {
                        brush = CreateSolidBrush(RGB(245, 158, 11));
                    } else if ((x == lastMoveSx && y == lastMoveSy) || (x == lastMoveTx && y == lastMoveTy)) {
                        brush = CreateSolidBrush(RGB(253, 230, 138));
                    } else if (hintActive && ((x == hintSx && y == hintSy) || (x == hintTx && y == hintTy))) {
                        brush = CreateSolidBrush(RGB(56, 189, 248)); // Cyan hint
                    } else if ((x + y) % 2 == 0) {
                        brush = CreateSolidBrush(RGB(232, 218, 193));
                    } else {
                        brush = CreateSolidBrush(RGB(74, 44, 27));
                    }

                    FillRect(memDC, &rc, brush);
                    DeleteObject(brush);

                    if (blackFrozen) {
                        HBRUSH freezeOverlay = CreateSolidBrush(RGB(186, 230, 253));
                        FillRect(memDC, &rc, freezeOverlay);
                        DeleteObject(freezeOverlay);
                    }

                    HPEN lightPen = CreatePen(PS_SOLID, 1, (x + y) % 2 == 0 ? RGB(245, 235, 224) : RGB(92, 58, 34));
                    oldPen = SelectObject(memDC, lightPen);
                    MoveToEx(memDC, rc.left, rc.bottom - 1, NULL);
                    LineTo(memDC, rc.left, rc.top);
                    LineTo(memDC, rc.right - 1, rc.top);
                    SelectObject(memDC, oldPen);
                    DeleteObject(lightPen);

                    if (selX != -1 && IsValidMove(selX, selY, x, y, 0)) {
                        if (!SimulatedMoveLeavesCheck(selX, selY, x, y, whiteTurn)) {
                            int isCapture = (board[y][x] != 0 || (x == epX && y == (whiteTurn ? epY - 1 : epY + 1)));
                            HBRUSH dotBrush = CreateSolidBrush(isCapture ? RGB(239, 68, 68) : RGB(74, 222, 128));
                            HGDIOBJ oldBrush2 = SelectObject(memDC, dotBrush);
                            HPEN oldPen2 = SelectObject(memDC, GetStockObject(NULL_PEN));
                            if (isCapture) {
                                Ellipse(memDC, rc.left + 10, rc.top + 10, rc.right - 10, rc.bottom - 10);
                            } else {
                                Ellipse(memDC, rc.left + 18, rc.top + 18, rc.right - 18, rc.bottom - 18);
                            }
                            SelectObject(memDC, oldPen2);
                            SelectObject(memDC, oldBrush2);
                            DeleteObject(dotBrush);
                        }
                    }

                    if (kbActive && x == kbX && y == kbY) {
                        HPEN kbPen = CreatePen(PS_SOLID, 3, RGB(56, 189, 248));
                        oldPen = SelectObject(memDC, kbPen);
                        HGDIOBJ oldNull = SelectObject(memDC, GetStockObject(NULL_BRUSH));
                        Rectangle(memDC, rc.left + 3, rc.top + 3, rc.right - 3, rc.bottom - 3);
                        SelectObject(memDC, oldNull);
                        SelectObject(memDC, oldPen);
                        DeleteObject(kbPen);
                    }

                    int p = board[y][x];
                    int isAnimatingThisPiece = (g_slide.active && (int)((g_slide.targetX - OX) / TS) == x && (int)((g_slide.targetY - OY) / TS) == y);
                    if (p != 0 && !isAnimatingThisPiece) {
                        int bob = 0;
                        int phase = ((GetTickCount() / 250) + x + y) % 4;
                        if (phase == 1 || phase == 3) bob = 1;
                        else if (phase == 2) bob = 2;
                        DrawChessPiece(memDC, p, rc.left, rc.top + bob, TS);
                    }
                }
            }

            if (hintActive && hintSx != -1 && hintTx != -1) {
                DrawHintArrow(memDC, hintSx, hintSy, hintTx, hintTy);
            }

            if (g_slide.active) {
                float dx = g_slide.targetX - g_slide.startX;
                float dy = g_slide.targetY - g_slide.startY;
                DWORD elapsed = GetTickCount() - g_slide.startTime;
                float t = (float)elapsed / (float)g_slide.duration;
                for (int i = 3; i >= 1; i--) {
                    float trailT = t - (i * 0.15f);
                    if (trailT < 0.0f) trailT = 0.0f;
                    float trailEase = 1.0f - (1.0f - trailT) * (1.0f - trailT);
                    int tx = (int)(g_slide.startX + dx * trailEase);
                    int ty = (int)(g_slide.startY + dy * trailEase);
                    DrawChessPiece(memDC, g_slide.p, tx, ty, TS);
                }
                DrawChessPiece(memDC, g_slide.p, (int)g_slide.curX, (int)g_slide.curY, TS);
            }

            for (int i = 0; i < g_particleCount; i++) {
                if (g_particles[i].life > 0) {
                    HBRUSH pBrush = CreateSolidBrush(g_particles[i].color);
                    HGDIOBJ oldPBrush = SelectObject(memDC, pBrush);
                    HPEN oldPPen = SelectObject(memDC, GetStockObject(NULL_PEN));
                    int size = (g_particles[i].life > 10) ? 3 : 2;
                    Ellipse(memDC, (int)g_particles[i].x - size, (int)g_particles[i].y - size, (int)g_particles[i].x + size, (int)g_particles[i].y + size);
                    SelectObject(memDC, oldPPen);
                    SelectObject(memDC, oldPBrush);
                    DeleteObject(pBrush);
                }
            }

            if (gameOver && (winner == 1 || winner == 2)) {
                int loserKing = (winner == 1) ? 12 : 6;
                for (int cy = 0; cy < 8; cy++) {
                    for (int cx = 0; cx < 8; cx++) {
                        if (board[cy][cx] == loserKing) {
                            float t = (float)(GetTickCount() % 1500) / 1500.0f;
                            int radius = (int)(TS * 2.0f * t);
                            HPEN shockPen = CreatePen(PS_SOLID, (int)(4.0f * (1.0f - t)) + 1, RGB(239, 68, 68));
                            HGDIOBJ oldPenS = SelectObject(memDC, shockPen);
                            HGDIOBJ oldBrushS = SelectObject(memDC, GetStockObject(NULL_BRUSH));
                            Ellipse(memDC, OX + cx*TS + TS/2 - radius, OY + cy*TS + TS/2 - radius, OX + cx*TS + TS/2 + radius, OY + cy*TS + TS/2 + radius);
                            SelectObject(memDC, oldBrushS);
                            SelectObject(memDC, oldPenS);
                            DeleteObject(shockPen);
                        }
                    }
                }
            }

            HFONT sFont = CreateFontA(14, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            oldFont = SelectObject(memDC, sFont);
            SetTextColor(memDC, RGB(255, 255, 255));

            RECT modeRc = { 20, 18, W - 150, 42 };
            char modeBuf[128];
            if (gameMode == 0) {
                wsprintfA(modeBuf, "Campaign: Stage %d/20 [%s]", currentStage, diffNames[aiPersonality - 1]);
            } else if (gameMode == 1) {
                wsprintfA(modeBuf, "%s [%s]", aiMode ? "vs AI" : "vs Player", diffNames[aiPersonality - 1]);
            } else if (gameMode == 2) {
                wsprintfA(modeBuf, "Puzzle #%d [%s]", (puzzleIndex % 6) + 1, diffNames[aiPersonality - 1]);
            } else {
                wsprintfA(modeBuf, "Blitz Timer (W:%ds B:%ds)", (int)blitzTimeWhite, (int)blitzTimeBlack);
            }
            DrawTextA(memDC, modeBuf, -1, &modeRc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

            RECT statsRc = { W - 150, 18, W - 20, 42 };
            char statsBuf[64];
            wsprintfA(statsBuf, "W:%d L:%d D:%d", statsWins, statsLosses, statsDraws);
            DrawTextA(memDC, statsBuf, -1, &statsRc, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

            // Captured Pieces & Material Score Summary
            int pieceCounts[13] = {0};
            for (int r = 0; r < 8; r++) for (int c = 0; c < 8; c++) if (board[r][c] > 0) pieceCounts[board[r][c]]++;
            int initialCounts[] = {0, 8, 2, 2, 2, 1, 1, 8, 2, 2, 2, 1, 1};

            int wLossVal = 0, bLossVal = 0;
            int capX = 25;
            for (int p = 1; p <= 5; p++) {
                int lost = initialCounts[p] - pieceCounts[p];
                if (lost > 0) {
                    wLossVal += (pieceValues[p] / 100) * lost;
                    for (int k = 0; k < lost; k++) {
                        DrawChessPiece(memDC, p, capX, 48, 24);
                        capX += 20;
                    }
                }
            }
            capX = 25;
            for (int p = 7; p <= 11; p++) {
                int lost = initialCounts[p] - pieceCounts[p];
                if (lost > 0) {
                    bLossVal += (pieceValues[p] / 100) * lost;
                    for (int k = 0; k < lost; k++) {
                        DrawChessPiece(memDC, p, capX, 76, 24);
                        capX += 20;
                    }
                }
            }
            int diff = bLossVal - wLossVal;
            if (diff != 0) {
                RECT matRc = { W - 200, 62, W - 20, 82 };
                char matBuf[32];
                wsprintfA(matBuf, "Advantage: %+d", diff);
                SetTextColor(memDC, diff > 0 ? RGB(94, 234, 212) : RGB(244, 63, 94));
                DrawTextA(memDC, matBuf, -1, &matRc, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
            }

            // Skill & Utility Buttons Row at bottom (Y: 530..560)
            struct Button { int x, y, w, h; char* text; } btns[6] = {
                { 25, 530, 70, 28, "Hint (H)" },
                { 105, 530, 70, 28, "Undo (U)" },
                { 185, 530, 70, 28, "Redo (Y)" },
                { 265, 530, 80, 28, "Freeze (F)" },
                { 355, 530, 90, 28, diffNames[aiPersonality - 1] },
                { 455, 530, 80, 28, "FEN/PGN" }
            };

            for (int i = 0; i < 6; i++) {
                RECT btnRc = { btns[i].x, btns[i].y, btns[i].x + btns[i].w, btns[i].y + btns[i].h };
                HBRUSH btnBrush = CreateSolidBrush(RGB(30, 41, 59));
                FillRect(memDC, &btnRc, btnBrush);
                DeleteObject(btnBrush);

                HPEN btnPen = CreatePen(PS_SOLID, 1, RGB(94, 234, 212));
                oldPen = SelectObject(memDC, btnPen);
                HGDIOBJ oldNull = SelectObject(memDC, GetStockObject(NULL_BRUSH));
                RoundRect(memDC, btnRc.left, btnRc.top, btnRc.right, btnRc.bottom, 6, 6);
                SelectObject(memDC, oldNull);
                SelectObject(memDC, oldPen);
                DeleteObject(btnPen);

                SetTextColor(memDC, RGB(255, 255, 255));
                DrawTextA(memDC, btns[i].text, -1, &btnRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }

            RECT statusRc = { 20, H - 45, W - 20, H - 15 };
            if (gameOver) {
                SetTextColor(memDC, RGB(245, 158, 11));
                if (winner == 1) DrawTextA(memDC, gameMode == 0 && currentStage < 20 ? "White Wins! Press 'R' / Click for Next Stage" : "Checkmate! White Wins! Press 'R'", -1, &statusRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                else if (winner == 2) DrawTextA(memDC, "Checkmate! Black Wins! Press 'R'", -1, &statusRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                else DrawTextA(memDC, "Stalemate / Draw! Press 'R'", -1, &statusRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            } else if (hintActive && hintText[0] != '\0') {
                SetTextColor(memDC, RGB(56, 189, 248));
                DrawTextA(memDC, hintText, -1, &statusRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            } else {
                SetTextColor(memDC, whiteTurn ? RGB(250, 250, 250) : RGB(148, 163, 184));
                char turnBuf[128];
                char* lastSAN = (g_historyIndex > 0 && g_historyStack[g_historyIndex].san[0] != '\0') ? g_historyStack[g_historyIndex].san : "";
                wsprintfA(turnBuf, "%s %s %s%s",
                    whiteTurn ? "White's Turn" : "Black's Turn",
                    blackFrozen ? "(Black Frozen!)" : "",
                    lastSAN[0] != '\0' ? "| Last: " : "",
                    lastSAN
                );
                DrawTextA(memDC, turnBuf, -1, &statusRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }

            SelectObject(memDC, oldFont);
            DeleteObject(sFont);

            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);

            SelectObject(memDC, oldBM);
            DeleteObject(memBM);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_KEYDOWN: {
            if (wParam == VK_UP) { kbY = (kbY > 0) ? kbY - 1 : 0; kbActive = 1; InvalidateRect(hwnd, NULL, FALSE); break; }
            if (wParam == VK_DOWN) { kbY = (kbY < 7) ? kbY + 1 : 7; kbActive = 1; InvalidateRect(hwnd, NULL, FALSE); break; }
            if (wParam == VK_LEFT) { kbX = (kbX > 0) ? kbX - 1 : 0; kbActive = 1; InvalidateRect(hwnd, NULL, FALSE); break; }
            if (wParam == VK_RIGHT) { kbX = (kbX < 7) ? kbX + 1 : 7; kbActive = 1; InvalidateRect(hwnd, NULL, FALSE); break; }
            if (wParam == VK_ESCAPE) { selX = -1; selY = -1; kbActive = 0; InvalidateRect(hwnd, NULL, FALSE); break; }
            if (wParam == VK_RETURN || wParam == VK_SPACE) {
                kbActive = 1;
                LPARAM lp = MAKELPARAM(OX + kbX * TS + TS / 2, OY + kbY * TS + TS / 2);
                SendMessage(hwnd, WM_LBUTTONDOWN, 0, lp);
                break;
            }
            if (wParam == 'R') {
                if (gameOver && gameMode == 0 && winner == 1) {
                    if (currentStage < 20) currentStage++;
                    else { currentStage = 1; }
                }
                ResetGame();
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'M') {
                gameMode = (gameMode + 1) % 4;
                if (gameMode == 0) aiMode = 1;
                ResetGame();
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'P') {
                aiPersonality = (aiPersonality % 4) + 1;
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'H') { // OPTIMAL AI HINT SKILL
                if (!gameOver) {
                    GetOptimalHintMove(&hintSx, &hintSy, &hintTx, &hintTy);
                    if (hintSx != -1) {
                        hintActive = 1;
                        char sanBuf[16];
                        GetSAN(hintSx, hintSy, hintTx, hintTy, board[hintSy][hintSx], board[hintTy][hintTx] != 0, sanBuf);
                        wsprintfA(hintText, "Best Move: %s", sanBuf);
                        MessageBeep(MB_OK);
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                }
            } else if (wParam == 'F') { // TIME FREEZE SKILL
                if (!gameOver && whiteTurn && freezePowerups > 0) {
                    freezePowerups--;
                    blackFrozen = 1;
                    if (gameMode == 3) blitzTimeWhite += 15.0f;
                    MessageBeep(MB_OK);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == 'U') { // UNDO MOVE SKILL
                UndoMove();
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'Y') { // REDO MOVE SKILL
                RedoMove();
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'E') { // COPY FEN TO CLIPBOARD
                char fenBuf[128];
                GenerateFEN(fenBuf);
                CopyTextToClipboard(hwnd, fenBuf);
                wsprintfA(hintText, "FEN Copied to Clipboard!");
                hintActive = 1;
                MessageBeep(MB_OK);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'G') { // COPY PGN TO CLIPBOARD
                char pgnBuf[2048];
                GeneratePGN(pgnBuf, 2048);
                CopyTextToClipboard(hwnd, pgnBuf);
                wsprintfA(hintText, "PGN Copied to Clipboard!");
                hintActive = 1;
                MessageBeep(MB_OK);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'L') { // LOAD FEN FROM CLIPBOARD
                char* clipText = GetTextFromClipboard(hwnd);
                if (clipText) {
                    if (LoadFEN(clipText)) {
                        wsprintfA(hintText, "FEN Loaded from Clipboard!");
                        hintActive = 1;
                        MessageBeep(MB_OK);
                    } else {
                        MessageBeep(MB_ICONWARNING);
                    }
                    GlobalFree((HGLOBAL)clipText);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            break;
        }
        case WM_TIMER: {
            if (wParam == 1) {
                KillTimer(hwnd, 1);
                DoBlackAIMove();
            } else if (wParam == 2) {
                int activeParticles = 0;
                for (int i = 0; i < g_particleCount; i++) {
                    if (g_particles[i].life > 0) {
                        g_particles[i].x += g_particles[i].vx;
                        g_particles[i].y += g_particles[i].vy;
                        g_particles[i].vy += 0.15f;
                        g_particles[i].life--;
                        if (g_particles[i].life > 0) activeParticles++;
                    }
                }

                if (g_slide.active) {
                    DWORD now = GetTickCount();
                    DWORD elapsed = now - g_slide.startTime;
                    if (elapsed >= g_slide.duration) {
                        g_slide.active = 0;
                    } else {
                        float t = (float)elapsed / (float)g_slide.duration;
                        float easeT = 1.0f - (1.0f - t) * (1.0f - t);
                        g_slide.curX = g_slide.startX + (g_slide.targetX - g_slide.startX) * easeT;
                        g_slide.curY = g_slide.startY + (g_slide.targetY - g_slide.startX) * easeT;
                    }
                }

                if (!g_slide.active && activeParticles == 0) {
                    KillTimer(hwnd, 2);
                    g_particleCount = 0;
                }

                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 3) { // Blitz Clock Tick
                if (gameMode == 3 && !gameOver) {
                    if (whiteTurn) {
                        blitzTimeWhite -= 0.1f;
                        if (blitzTimeWhite <= 0.0f) { blitzTimeWhite = 0; gameOver = 1; winner = 2; statsLosses++; SaveStatsFreestanding(); }
                    } else {
                        blitzTimeBlack -= 0.1f;
                        if (blitzTimeBlack <= 0.0f) { blitzTimeBlack = 0; gameOver = 1; winner = 1; statsWins++; SaveStatsFreestanding(); }
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);

            // Check skill & action buttons (Y: 530..560)
            if (my >= 530 && my <= 560) {
                if (mx >= 25 && mx <= 95) { SendMessage(hwnd, WM_KEYDOWN, 'H', 0); return 0; }
                if (mx >= 105 && mx <= 175) { SendMessage(hwnd, WM_KEYDOWN, 'U', 0); return 0; }
                if (mx >= 185 && mx <= 255) { SendMessage(hwnd, WM_KEYDOWN, 'Y', 0); return 0; }
                if (mx >= 265 && mx <= 345) { SendMessage(hwnd, WM_KEYDOWN, 'F', 0); return 0; }
                if (mx >= 355 && mx <= 445) { SendMessage(hwnd, WM_KEYDOWN, 'P', 0); return 0; }
                if (mx >= 455 && mx <= 535) { SendMessage(hwnd, WM_KEYDOWN, 'E', 0); return 0; }
            }

            if (gameOver) {
                if (gameMode == 0 && winner == 1 && currentStage < 20) {
                    currentStage++;
                }
                ResetGame();
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }

            if (aiMode && !whiteTurn) break;
            
            int tx = (mx - OX) / TS;
            int ty = (my - OY) / TS;
            
            if (tx >= 0 && tx < 8 && ty >= 0 && ty < 8) {
                hintActive = 0; // Clear hint on click
                if (selX == -1) {
                    if (board[ty][tx] != 0) {
                        int isWhite = board[ty][tx] <= 6;
                        if ((whiteTurn && isWhite) || (!whiteTurn && !isWhite)) {
                            selX = tx; selY = ty;
                        }
                    }
                } else {
                    if (selX == tx && selY == ty) {
                        selX = -1; selY = -1;
                    } else {
                        int p = board[selY][selX];
                        int isWhite = p <= 6;
                        int dstP = board[ty][tx];
                        int dstIsWhite = dstP <= 6;
                        
                        if (dstP != 0 && isWhite == dstIsWhite) {
                            selX = tx; selY = ty;
                        } else {
                            if (IsValidMove(selX, selY, tx, ty, 0)) {
                                if (!SimulatedMoveLeavesCheck(selX, selY, tx, ty, whiteTurn)) {
                                    int curSelX = selX; int curSelY = selY;
                                    selX = -1; selY = -1;

                                    TriggerMove(hwnd, curSelX, curSelY, tx, ty);

                                    if (aiMode && !whiteTurn && !gameOver) {
                                        SetTimer(hwnd, 1, 350, NULL);
                                    }
                                } else {
                                    MessageBeep(MB_ICONWARNING);
                                }
                            }
                        }
                    }
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_DESTROY:
            if (hBgBrush) DeleteObject(hBgBrush);
            KillTimer(hwnd, 1); KillTimer(hwnd, 2); KillTimer(hwnd, 3);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry(void) {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KChessApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    hBgBrush = CreateSolidBrush(RGB(15, 15, 22));
    wc.hbrBackground = hBgBrush;
    RegisterClass(&wc);

    my_srand(GetTickCount());
    LoadStatsFreestanding();

    HWND hwnd = CreateWindowEx(0, "KChessApp", "KChess - AI & Utility Chess Engine", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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
