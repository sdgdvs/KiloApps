#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 540
#define H 560
#define TS 50
#define OX 70
#define OY 70

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

int selX = -1;
int selY = -1;
int whiteTurn = 1;
int gameOver = 0;
int winner = 0; // 1 = White, 2 = Black, 3 = Draw
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
    if (pType == 2) return center * 3; // Knight
    if (pType == 3) return center * 2; // Bishop
    if (pType == 6) { // King
        if (row >= 6 && (x <= 2 || x >= 5)) return 20; 
        return -center * 2; 
    }
    return 0;
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

void ResetGame(void) {
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
        aiDifficulty = 2;
        freezePowerups = 1;
        undoPowerups = 1;
    }
    
    for(int y=0; y<8; y++) {
        for(int x=0; x<8; x++) {
            board[y][x] = initialBoard[y][x];
        }
    }
    selX = -1; selY = -1;
    whiteTurn = 1;
    gameOver = 0;
    winner = 0;
    blackFrozen = 0;
    canUndo = 0;
    lastMoveSx = -1; lastMoveSy = -1; lastMoveTx = -1; lastMoveTy = -1;
    g_particleCount = 0;
    g_slide.active = 0;
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

static void TriggerMove(HWND hwnd, int sx, int sy, int tx, int ty) {
    int isCaptured = (board[ty][tx] != 0);
    int p = board[sy][sx];
    int pType = p > 6 ? p - 6 : p;
    int isWhiteMove = p <= 6;

    if (pType == 1 && tx == epX && ty == (isWhiteMove ? epY - 1 : epY + 1)) {
        board[epY][epX] = 0;
        isCaptured = 1;
    }

    // Trigger slide animation
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
        if (isWhiteMove) { if (sx == 0) wRookLMoved = 1; if (sx == 7) wRookRMoved = 1; }
        else { if (sx == 0) bRookLMoved = 1; if (sx == 7) bRookRMoved = 1; }
    }
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Double buffering
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, W, H);
            HGDIOBJ oldBM = SelectObject(memDC, memBM);

            // Fill dark background
            HBRUSH bgBrush = CreateSolidBrush(RGB(15, 15, 22));
            RECT fullRc = {0, 0, W, H};
            FillRect(memDC, &fullRc, bgBrush);
            DeleteObject(bgBrush);

            // Outer board wood frame border
            HBRUSH frameBrush = CreateSolidBrush(RGB(45, 24, 16));
            RECT frameRc = {15, 15, W - 15, H - 15};
            FillRect(memDC, &frameRc, frameBrush);
            DeleteObject(frameBrush);

            HPEN goldPen = CreatePen(PS_SOLID, 2, RGB(212, 175, 55));
            HGDIOBJ oldPen = SelectObject(memDC, goldPen);
            HGDIOBJ oldNullBrush = SelectObject(memDC, GetStockObject(NULL_BRUSH));
            RoundRect(memDC, 15, 15, W - 15, H - 15, 16, 16);
            Rectangle(memDC, OX - 4, OY - 4, OX + TS * 8 + 4, OY + TS * 8 + 4);
            SelectObject(memDC, oldNullBrush);
            SelectObject(memDC, oldPen);
            DeleteObject(goldPen);

            // Board Rank & File Annotations
            HFONT labelFont = CreateFontA(14, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
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
                RECT rRight = { OX + TS * 8 + 4, OY + i * TS, OX + TS * 8 + 22, OY + (i + 1) * TS };
                DrawTextA(memDC, fStr, 1, &fTop, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                DrawTextA(memDC, fStr, 1, &fBot, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                DrawTextA(memDC, rStr, 1, &rLeft, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                DrawTextA(memDC, rStr, 1, &rRight, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            SelectObject(memDC, oldFont);
            DeleteObject(labelFont);

            // Check warning detection
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

            // Draw Squares
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    RECT rc = { OX + x * TS, OY + y * TS, OX + (x + 1) * TS, OY + (y + 1) * TS };
                    HBRUSH brush;
                    
                    if (x == inCheckX && y == inCheckY) {
                        brush = CreateSolidBrush(RGB(239, 68, 68)); // red-500 for check
                    } else if (x == selX && y == selY) {
                        brush = CreateSolidBrush(RGB(245, 158, 11)); // selected gold
                    } else if ((x == lastMoveSx && y == lastMoveSy) || (x == lastMoveTx && y == lastMoveTy)) {
                        brush = CreateSolidBrush(RGB(253, 230, 138)); // yellow-200 last move
                    } else if ((x + y) % 2 == 0) {
                        brush = CreateSolidBrush(RGB(232, 218, 193)); // light wood
                    } else {
                        brush = CreateSolidBrush(RGB(74, 44, 27)); // dark walnut
                    }

                    FillRect(memDC, &rc, brush);
                    DeleteObject(brush);

                    if (blackFrozen) {
                        HBRUSH freezeOverlay = CreateSolidBrush(RGB(186, 230, 253));
                        FillRect(memDC, &rc, freezeOverlay);
                        DeleteObject(freezeOverlay);
                    }

                    // Bevel border lines
                    HPEN lightPen = CreatePen(PS_SOLID, 1, (x + y) % 2 == 0 ? RGB(245, 235, 224) : RGB(92, 58, 34));
                    oldPen = SelectObject(memDC, lightPen);
                    MoveToEx(memDC, rc.left, rc.bottom - 1, NULL);
                    LineTo(memDC, rc.left, rc.top);
                    LineTo(memDC, rc.right - 1, rc.top);
                    SelectObject(memDC, oldPen);
                    DeleteObject(lightPen);

                    // Valid Move Indicators
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

                    // Piece rendering
                    int p = board[y][x];
                    int isAnimatingThisPiece = (g_slide.active && (int)((g_slide.targetX - OX) / TS) == x && (int)((g_slide.targetY - OY) / TS) == y);
                    if (p != 0 && !isAnimatingThisPiece) {
                        DrawChessPiece(memDC, p, rc.left, rc.top, TS);
                    }
                }
            }

            // Draw sliding piece
            if (g_slide.active) {
                DrawChessPiece(memDC, g_slide.p, (int)g_slide.curX, (int)g_slide.curY, TS);
            }

            // Render Particle Sparks
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

            // Text Headers
            HFONT sFont = CreateFontA(14, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            oldFont = SelectObject(memDC, sFont);
            SetTextColor(memDC, RGB(255, 255, 255));

            RECT modeRc = { 20, 20, W - 150, 45 };
            char modeBuf[128];
            if (campaignMode) {
                wsprintfA(modeBuf, "Campaign: Stage %d | F:Freeze(%d) U:Undo(%d)", currentStage, freezePowerups, undoPowerups);
            } else {
                wsprintfA(modeBuf, "%s | F:Freeze(%d) U:Undo(%d)", aiMode ? "vs AI ('M')" : "vs Player ('M')", freezePowerups, undoPowerups);
            }
            DrawTextA(memDC, modeBuf, -1, &modeRc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

            RECT statsRc = { W - 150, 20, W - 20, 45 };
            char statsBuf[64];
            wsprintfA(statsBuf, "W:%d L:%d D:%d", statsWins, statsLosses, statsDraws);
            DrawTextA(memDC, statsBuf, -1, &statsRc, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

            RECT statusRc = { 20, H - 42, W - 20, H - 15 };
            if (gameOver) {
                SetTextColor(memDC, RGB(245, 158, 11));
                if (winner == 1) DrawTextA(memDC, campaignMode && currentStage < 15 ? "White Wins! Press 'R' for Next Stage" : "Checkmate! White Wins! Press 'R'", -1, &statusRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                else if (winner == 2) DrawTextA(memDC, "Checkmate! Black Wins! Press 'R'", -1, &statusRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                else DrawTextA(memDC, "Stalemate! Draw! Press 'R'", -1, &statusRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            } else {
                SetTextColor(memDC, whiteTurn ? RGB(250, 250, 250) : RGB(148, 163, 184));
                DrawTextA(memDC, whiteTurn ? "White's Turn" : "Black's Turn", -1, &statusRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }

            SelectObject(memDC, oldFont);
            DeleteObject(sFont);

            // Blit to screen
            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);

            SelectObject(memDC, oldBM);
            DeleteObject(memBM);
            DeleteDC(memDC);

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
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'M') {
                if (!campaignMode) {
                    aiMode = !aiMode;
                    ResetGame();
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == 'C') {
                campaignMode = !campaignMode;
                if (campaignMode) { aiMode = 1; currentStage = 1; }
                ResetGame();
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'F') {
                if (!gameOver && whiteTurn && aiMode && freezePowerups > 0) {
                    freezePowerups--;
                    blackFrozen = 1;
                    MessageBeep(MB_OK);
                    InvalidateRect(hwnd, NULL, FALSE);
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
                    g_slide.active = 0;
                    MessageBeep(MB_OK);
                    InvalidateRect(hwnd, NULL, FALSE);
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
                        InvalidateRect(hwnd, NULL, FALSE);
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
                            int chosen = bestMoves[my_rand() % bestCount];
                            
                            TriggerMove(hwnd, moves[chosen].sx, moves[chosen].sy, moves[chosen].tx, moves[chosen].ty);
                            InvalidateRect(hwnd, NULL, FALSE);
                        }
                    }
                }
            } else if (wParam == 2) {
                // Animation & Particle tick
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
                        g_slide.curY = g_slide.startY + (g_slide.targetY - g_slide.startY) * easeT;
                    }
                }

                if (!g_slide.active && activeParticles == 0) {
                    KillTimer(hwnd, 2);
                    g_particleCount = 0;
                }

                InvalidateRect(hwnd, NULL, FALSE);
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

                                    int curSelX = selX;
                                    int curSelY = selY;
                                    selX = -1;
                                    selY = -1;

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
            KillTimer(hwnd, 1);
            KillTimer(hwnd, 2);
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
