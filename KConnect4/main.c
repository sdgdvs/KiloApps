#include <windows.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

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
float animY_float = 50.0f;
float animVY = 0.0f;
int animBounceCount = 0;
int animTargetY = 0;
int animType = 0; // 0=normal drop, 1=bomb drop, 2=drill drop

// Particle structure for confetti FX
typedef struct {
    float x, y;
    float vx, vy;
    float rotation, vRot;
    COLORREF color;
    int size;
    int life;
} Particle;

#define MAX_PARTICLES 70
Particle g_particles[MAX_PARTICLES];
int g_particleCount = 0;

void SpawnConfetti() {
    COLORREF palette[] = {
        RGB(255, 82, 82), RGB(255, 215, 0), RGB(79, 195, 247),
        RGB(105, 240, 174), RGB(224, 64, 251), RGB(255, 255, 255)
    };
    g_particleCount = MAX_PARTICLES;
    for (int i = 0; i < MAX_PARTICLES; i++) {
        g_particles[i].x = (float)(rand() % 350 + 10);
        g_particles[i].y = (float)(-(rand() % 40));
        g_particles[i].vx = ((float)(rand() % 100) - 50.0f) / 15.0f;
        g_particles[i].vy = ((float)(rand() % 100)) / 25.0f + 2.0f;
        g_particles[i].rotation = ((float)(rand() % 360)) * 3.14159f / 180.0f;
        g_particles[i].vRot = ((float)(rand() % 100) - 50.0f) / 100.0f;
        g_particles[i].color = palette[rand() % 6];
        g_particles[i].size = rand() % 6 + 4;
        g_particles[i].life = rand() % 60 + 60;
    }
}

void UpdateParticles() {
    for (int i = 0; i < g_particleCount; i++) {
        if (g_particles[i].life > 0) {
            g_particles[i].x += g_particles[i].vx;
            g_particles[i].y += g_particles[i].vy;
            g_particles[i].vy += 0.15f; // Gravity
            g_particles[i].rotation += g_particles[i].vRot;
            g_particles[i].life--;
        }
    }
}

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

void DrawDisc3D(HDC hdc, int x, int y, int cellType, bool isWinCell) {
    int cx = x + 20;
    int cy = y + 20;

    if (cellType == 0) { // Empty cutout hole
        HBRUSH bgBrush = CreateSolidBrush(RGB(9, 12, 20));
        HPEN darkPen = CreatePen(PS_SOLID, 1, RGB(4, 6, 10));
        SelectObject(hdc, bgBrush);
        SelectObject(hdc, darkPen);
        Ellipse(hdc, x, y, x + 40, y + 40);
        DeleteObject(bgBrush);
        DeleteObject(darkPen);

        // Top-left inner shadow arc
        HPEN shadowPen = CreatePen(PS_SOLID, 2, RGB(2, 3, 6));
        SelectObject(hdc, shadowPen);
        Arc(hdc, x + 2, y + 2, x + 38, y + 38, x + 38, y + 2, x + 2, y + 38);
        DeleteObject(shadowPen);
        return;
    }

    if (isWinCell) {
        // Glowing cyan aura ring
        HPEN glowPen = CreatePen(PS_SOLID, 4, RGB(0, 255, 255));
        HBRUSH nullBrush = GetStockObject(NULL_BRUSH);
        SelectObject(hdc, glowPen);
        SelectObject(hdc, nullBrush);
        Ellipse(hdc, x - 2, y - 2, x + 42, y + 42);
        DeleteObject(glowPen);
    }

    if (cellType == 1 || cellType == 100) { // Red Disc (100 = hover)
        COLORREF borderColor = (cellType == 100) ? RGB(140, 40, 40) : RGB(120, 0, 0);
        COLORREF bodyColor   = (cellType == 100) ? RGB(200, 70, 70) : RGB(220, 30, 30);
        COLORREF innerRing   = (cellType == 100) ? RGB(220, 90, 90) : RGB(180, 20, 20);

        HPEN bPen = CreatePen(PS_SOLID, 2, borderColor);
        HBRUSH bBrush = CreateSolidBrush(bodyColor);
        SelectObject(hdc, bPen);
        SelectObject(hdc, bBrush);
        Ellipse(hdc, x, y, x + 40, y + 40);
        DeleteObject(bPen);
        DeleteObject(bBrush);

        // Outer ridged ring detail
        HPEN rPen = CreatePen(PS_SOLID, 1, innerRing);
        HBRUSH nBrush = GetStockObject(NULL_BRUSH);
        SelectObject(hdc, rPen);
        SelectObject(hdc, nBrush);
        Ellipse(hdc, x + 3, y + 3, x + 37, y + 37);
        DeleteObject(rPen);

        // 3D Glossy Specular Highlight
        HBRUSH hlBrush = CreateSolidBrush((cellType == 100) ? RGB(255, 200, 200) : RGB(255, 170, 170));
        HPEN nPen = GetStockObject(NULL_PEN);
        SelectObject(hdc, nPen);
        SelectObject(hdc, hlBrush);
        Ellipse(hdc, x + 7, y + 5, x + 25, y + 17);
        DeleteObject(hlBrush);

        // White core highlight dot
        HBRUSH wBrush = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, wBrush);
        Ellipse(hdc, x + 10, y + 7, x + 18, y + 13);
        DeleteObject(wBrush);

        // Gold 5-point star emblem
        POINT starPts[10];
        float outerR = 8.5f, innerR = 3.5f;
        for (int i = 0; i < 10; i++) {
            float angle = (i * 36 - 90) * 3.14159f / 180.0f;
            float r_curr = (i % 2 == 0) ? outerR : innerR;
            starPts[i].x = cx + (long)(cosf(angle) * r_curr);
            starPts[i].y = cy + (long)(sinf(angle) * r_curr);
        }
        HBRUSH starBrush = CreateSolidBrush(RGB(255, 215, 0));
        HPEN starPen = CreatePen(PS_SOLID, 1, RGB(180, 140, 0));
        SelectObject(hdc, starPen);
        SelectObject(hdc, starBrush);
        Polygon(hdc, starPts, 10);
        DeleteObject(starBrush);
        DeleteObject(starPen);
    }
    else if (cellType == 2 || cellType == 200) { // Yellow Disc (200 = hover)
        COLORREF borderColor = (cellType == 200) ? RGB(160, 140, 40) : RGB(160, 100, 0);
        COLORREF bodyColor   = (cellType == 200) ? RGB(230, 210, 80) : RGB(255, 210, 30);
        COLORREF innerRing   = (cellType == 200) ? RGB(240, 220, 100) : RGB(220, 160, 0);

        HPEN bPen = CreatePen(PS_SOLID, 2, borderColor);
        HBRUSH bBrush = CreateSolidBrush(bodyColor);
        SelectObject(hdc, bPen);
        SelectObject(hdc, bBrush);
        Ellipse(hdc, x, y, x + 40, y + 40);
        DeleteObject(bPen);
        DeleteObject(bBrush);

        // Inner ridged ring
        HPEN rPen = CreatePen(PS_SOLID, 1, innerRing);
        HBRUSH nBrush = GetStockObject(NULL_BRUSH);
        SelectObject(hdc, rPen);
        SelectObject(hdc, nBrush);
        Ellipse(hdc, x + 3, y + 3, x + 37, y + 37);
        DeleteObject(rPen);

        // 3D Glossy Highlight
        HBRUSH hlBrush = CreateSolidBrush(RGB(255, 255, 200));
        HPEN nPen = GetStockObject(NULL_PEN);
        SelectObject(hdc, nPen);
        SelectObject(hdc, hlBrush);
        Ellipse(hdc, x + 7, y + 5, x + 25, y + 17);
        DeleteObject(hlBrush);

        // White core highlight
        HBRUSH wBrush = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, wBrush);
        Ellipse(hdc, x + 10, y + 7, x + 18, y + 13);
        DeleteObject(wBrush);

        // Royal Blue Crown Emblem
        POINT crownPts[7] = {
            {cx - 8, cy + 5}, {cx - 8, cy - 3}, {cx - 4, cy + 1},
            {cx, cy - 6}, {cx + 4, cy + 1}, {cx + 8, cy - 3}, {cx + 8, cy + 5}
        };
        HBRUSH crownBrush = CreateSolidBrush(RGB(24, 60, 150));
        HPEN crownPen = CreatePen(PS_SOLID, 1, RGB(10, 25, 80));
        SelectObject(hdc, crownPen);
        SelectObject(hdc, crownBrush);
        Polygon(hdc, crownPts, 7);
        DeleteObject(crownBrush);
        DeleteObject(crownPen);
    }
    else if (cellType == 3) { // Obstacle
        HBRUSH obsBrush = CreateSolidBrush(RGB(45, 45, 52));
        HPEN obsPen = CreatePen(PS_SOLID, 2, RGB(80, 80, 90));
        SelectObject(hdc, obsPen);
        SelectObject(hdc, obsBrush);
        Ellipse(hdc, x, y, x + 40, y + 40);
        DeleteObject(obsBrush);
        DeleteObject(obsPen);

        // Warning diagonal hazard line
        HPEN warnPen = CreatePen(PS_SOLID, 3, RGB(255, 215, 0));
        SelectObject(hdc, warnPen);
        MoveToEx(hdc, x + 10, y + 30, NULL);
        LineTo(hdc, x + 30, y + 10);
        DeleteObject(warnPen);
    }
    else if (cellType == 4) { // Crackable Block
        HBRUSH woodBrush = CreateSolidBrush(RGB(139, 69, 19));
        HPEN woodPen = CreatePen(PS_SOLID, 2, RGB(210, 130, 40));
        SelectObject(hdc, woodPen);
        SelectObject(hdc, woodBrush);
        Ellipse(hdc, x, y, x + 40, y + 40);
        DeleteObject(woodBrush);

        // Crack line overlay
        HPEN crackPen = CreatePen(PS_SOLID, 2, RGB(50, 25, 5));
        SelectObject(hdc, crackPen);
        MoveToEx(hdc, cx - 6, cy - 10, NULL);
        LineTo(hdc, cx, cy);
        LineTo(hdc, cx - 4, cy + 6);
        LineTo(hdc, cx + 8, cy + 10);
        DeleteObject(crackPen);
        DeleteObject(woodPen);
    }
    else if (cellType == 5 || cellType == 500) { // Bomb
        HBRUSH bBrush = CreateSolidBrush(RGB(25, 25, 25));
        HPEN bPen = CreatePen(PS_SOLID, 2, RGB(255, 68, 0));
        SelectObject(hdc, bPen);
        SelectObject(hdc, bBrush);
        Ellipse(hdc, x + 2, y + 2, x + 38, y + 38);
        DeleteObject(bBrush);
        DeleteObject(bPen);

        // Fuse spark
        HPEN sparkPen = CreatePen(PS_SOLID, 2, RGB(255, 215, 0));
        SelectObject(hdc, sparkPen);
        MoveToEx(hdc, cx, cy - 15, NULL);
        LineTo(hdc, cx + 4, cy - 20);
        DeleteObject(sparkPen);
    }
    else if (cellType == 6 || cellType == 600) { // Drill
        HBRUSH dBrush = CreateSolidBrush(RGB(0, 150, 180));
        HPEN dPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 255));
        SelectObject(hdc, dPen);
        SelectObject(hdc, dBrush);
        Ellipse(hdc, x + 2, y + 2, x + 38, y + 38);
        DeleteObject(dBrush);
        DeleteObject(dPen);

        // Spiral drill line
        HPEN drillLine = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
        SelectObject(hdc, drillLine);
        MoveToEx(hdc, cx - 10, cy - 10, NULL);
        LineTo(hdc, cx + 10, cy + 10);
        MoveToEx(hdc, cx - 8, cy, NULL);
        LineTo(hdc, cx, cy + 8);
        DeleteObject(drillLine);
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
            animY_float = 50.0f;
            animVY = 0.0f;
            animBounceCount = 0;
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
            else { PlaySoundEffect(2); SpawnConfetti(); SetTimer(hwnd, 4, 25, NULL); }
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
        else { PlaySoundEffect(2); SpawnConfetti(); SetTimer(hwnd, 4, 25, NULL); }
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
            } else if (wParam == 2) { // Drop animation timer with bounce physics
                animVY += 3.0f;
                animY_float += animVY;
                if (animY_float >= animTargetY) {
                    animY_float = (float)animTargetY;
                    if (animVY > 5.0f && animBounceCount < 2) {
                        animVY = -animVY * 0.38f;
                        animBounceCount++;
                    } else {
                        animY = animTargetY;
                        isAnimating = false;
                        KillTimer(hwnd, 2);
                        FinishTurnEffects(hwnd);
                    }
                }
                animY = (int)animY_float;
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 3) { // Speed mode countdown timer
                if (gameMode == 3 && gameActive && !isAnimating) {
                    if (!(gameMode > 0 && currentPlayer == 2)) {
                        turnTimeLeftMs -= 100;
                        if (turnTimeLeftMs <= 0) {
                            turnTimeLeftMs = 7000;
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
            } else if (wParam == 4) { // Confetti particle timer
                UpdateParticles();
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            
            // Dark futuristic background fill
            HBRUSH bg = CreateSolidBrush(RGB(16, 20, 29));
            FillRect(hdc, &rect, bg);
            DeleteObject(bg);
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(79, 195, 247));
            
            char statusText[64];
            if (!gameActive) {
                if (isDraw) wsprintf(statusText, "It's a Draw! Click to reset.");
                else wsprintf(statusText, "Player %d (%s) Wins!", currentPlayer, (currentPlayer == 1) ? "Red" : "Yellow");
            } else {
                wsprintf(statusText, "Player %d's turn (%s)", currentPlayer, (currentPlayer == 1) ? "Red" : "Yellow");
            }
            TextOut(hdc, 20, 15, statusText, lstrlen(statusText));
            
            // Speed mode timer bar
            if (gameMode == 3 && gameActive) {
                int barWidth = (315 * turnTimeLeftMs) / 7000;
                RECT timerBg = {20, 38, 335, 44};
                HBRUSH tBg = CreateSolidBrush(RGB(30, 40, 55));
                FillRect(hdc, &timerBg, tBg);
                DeleteObject(tBg);
                
                RECT timerFg = {20, 38, 20 + barWidth, 44};
                HBRUSH tFg = CreateSolidBrush((turnTimeLeftMs < 2000) ? RGB(255, 82, 82) : RGB(76, 175, 80));
                FillRect(hdc, &timerFg, tFg);
                DeleteObject(tFg);
            }
            
            SetTextColor(hdc, RGB(170, 170, 170));
            char statsStr[128];
            wsprintf(statsStr, "Wins: Red %d, Yellow %d | Draws: %d | Streak: %d (Best: %d) | Max Stage: %d",
                     stats.redWins, stats.yellowWins, stats.draws, stats.streak, stats.bestStreak, stats.maxCampaignStage);
            TextOut(hdc, 10, 445, statsStr, lstrlen(statsStr));
            
            // --- 3D Blue Plastic Grid Board Frame ---
            RECT frameOuter = {14, 44, 341, 331};
            HBRUSH frameBg = CreateSolidBrush(RGB(19, 45, 105));
            FillRect(hdc, &frameOuter, frameBg);
            DeleteObject(frameBg);

            // Bevel edges for plastic frame
            HPEN topLight = CreatePen(PS_SOLID, 2, RGB(60, 115, 220));
            HPEN botDark  = CreatePen(PS_SOLID, 2, RGB(10, 22, 55));
            SelectObject(hdc, topLight);
            MoveToEx(hdc, 14, 331, NULL); LineTo(hdc, 14, 44); LineTo(hdc, 341, 44);
            SelectObject(hdc, botDark);
            MoveToEx(hdc, 341, 44, NULL); LineTo(hdc, 341, 331); LineTo(hdc, 14, 331);
            DeleteObject(topLight);
            DeleteObject(botDark);

            // Inner grid board surface
            RECT boardRect = {20, 50, 20 + COLS * 45 + 5, 50 + ROWS * 45 + 5};
            HBRUSH boardBg = CreateSolidBrush(RGB(24, 60, 138));
            FillRect(hdc, &boardRect, boardBg);
            DeleteObject(boardBg);

            // Corner silver metallic screws
            int screwCoords[4][2] = {{17, 47}, {333, 47}, {17, 323}, {333, 323}};
            HBRUSH screwBrush = CreateSolidBrush(RGB(180, 185, 195));
            HPEN screwPen = CreatePen(PS_SOLID, 1, RGB(60, 65, 75));
            SelectObject(hdc, screwBrush);
            SelectObject(hdc, screwPen);
            for(int k=0; k<4; k++) {
                Ellipse(hdc, screwCoords[k][0], screwCoords[k][1], screwCoords[k][0] + 6, screwCoords[k][1] + 6);
            }
            DeleteObject(screwBrush);
            DeleteObject(screwPen);
            
            HRGN hRgn = CreateRectRgn(20, 50, 20 + COLS * 45 + 5, 50 + ROWS * 45 + 5);
            SelectClipRgn(hdc, hRgn);

            int hoverRow = -1;
            if (hoverCol != -1 && !isAnimating && gameActive && !(gameMode > 0 && currentPlayer == 2)) {
                for (int r = ROWS - 1; r >= 0; r--) {
                    if (board[r][hoverCol] == 0) {
                        hoverRow = r;
                        break;
                    }
                }
            }
            
            // Draw grid cells with 3D Glossy Discs
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

                    int typeToDraw = board[r][c];
                    if (isAnimating && r == animRow && c == animCol) {
                        typeToDraw = 0;
                    } else if (board[r][c] == 0 && r == hoverRow && c == hoverCol) {
                        if (selectedPowerup == 1) typeToDraw = 500;
                        else if (selectedPowerup == 2) typeToDraw = 600;
                        else typeToDraw = (currentPlayer == 1) ? 100 : 200;
                    }
                    
                    DrawDisc3D(hdc, x, y, typeToDraw, isWinCell);
                }
            }
            
            // Draw dropping disc
            if (isAnimating) {
                int x = 20 + 5 + animCol * 45;
                int dropType = (animType == 1) ? 5 : ((animType == 2) ? 6 : animPlayer);
                DrawDisc3D(hdc, x, animY, dropType, false);
            }

            // Draw Winning 4-in-a-row Neon Beam Line
            if (winCellCount >= 4) {
                int x1 = 20 + 5 + winCells[0][1] * 45 + 20;
                int y1 = 50 + 5 + winCells[0][0] * 45 + 20;
                int x2 = 20 + 5 + winCells[winCellCount - 1][1] * 45 + 20;
                int y2 = 50 + 5 + winCells[winCellCount - 1][0] * 45 + 20;

                // Outer cyan aura beam
                HPEN auraPen = CreatePen(PS_SOLID, 12, RGB(0, 220, 255));
                SelectObject(hdc, auraPen);
                MoveToEx(hdc, x1, y1, NULL); LineTo(hdc, x2, y2);
                DeleteObject(auraPen);

                // Mid cyan beam
                HPEN midPen = CreatePen(PS_SOLID, 6, RGB(160, 245, 255));
                SelectObject(hdc, midPen);
                MoveToEx(hdc, x1, y1, NULL); LineTo(hdc, x2, y2);
                DeleteObject(midPen);

                // Inner white laser core
                HPEN corePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                SelectObject(hdc, corePen);
                MoveToEx(hdc, x1, y1, NULL); LineTo(hdc, x2, y2);
                DeleteObject(corePen);
            }
            
            SelectClipRgn(hdc, NULL);
            DeleteObject(hRgn);

            // Draw Confetti Particles
            for (int i = 0; i < g_particleCount; i++) {
                if (g_particles[i].life > 0) {
                    HBRUSH pBrush = CreateSolidBrush(g_particles[i].color);
                    HPEN pPen = GetStockObject(NULL_PEN);
                    SelectObject(hdc, pBrush);
                    SelectObject(hdc, pPen);
                    int px = (int)g_particles[i].x;
                    int py = (int)g_particles[i].y;
                    int sz = g_particles[i].size;
                    Rectangle(hdc, px - sz/2, py - sz/2, px + sz/2, py + sz/2);
                    DeleteObject(pBrush);
                }
            }
            
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
