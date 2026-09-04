#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

int _fltused = 1;

int CELL = 30;

#define MAX_ROWS 16
#define MAX_COLS 30
#define MAX_MOVES 10000

typedef struct {
    int t;
    int x;
    int y;
    int b;
} Move;

int grid[MAX_ROWS][MAX_COLS];
int state[MAX_ROWS][MAX_COLS]; // 0=hidden, 1=revealed, 2=flagged
int gameOver = 0;
int flagsPlaced = 0;
int timeElapsed = 0;
int firstClick = 1;

int g_dpi = 96;

int cols = 16;
int rows = 16;
int totalMines = 40;
int currentDiff = 1;

int bestTimes[3] = {-1, -1, -1};
Move moves[MAX_MOVES];
int movesCount = 0;
int isReplaying = 0;
DWORD startRealTime = 0;
DWORD replayStartTime = 0;
int replayMoveIdx = 0;

// Screen Shake with quadratic decay
int shakeIntensity = 0;
DWORD shakeTick = 0;
DWORD shakeDuration = 0;

// 16-step trigonometric lookup tables
static const int sin_tab16[16] = { 0, 38, 71, 92, 100, 92, 71, 38, 0, -38, -71, -92, -100, -92, -71, -38 };
static const int cos_tab16[16] = { 100, 92, 71, 38, 0, -38, -71, -92, -100, -92, -71, -38, 0, 38, 71, 92 };

static int FastSin(int idx) {
    idx = ((idx % 16) + 16) % 16;
    return sin_tab16[idx];
}
static int FastCos(int idx) {
    idx = ((idx % 16) + 16) % 16;
    return cos_tab16[idx];
}

// Multi-Layer Kinematic Particle System
// Layer 0: Incandescent core needle sparks
// Layer 1: Expanding buoyant smoke & plasma puffs
// Layer 2: Heavy kinematic rock/debris shards with tumbling rotation & bounce physics
// Layer 3: Radiant golden celebration energy stars
typedef struct {
    float x, y;
    float prev_x, prev_y;
    float vx, vy;
    int life, maxLife;
    float size;
    COLORREF color;
    int layer;
    float rot;
    float vrot;
    float gravity;
    float drag;
} Particle;

#define MAX_PARTICLES 450
Particle particles[MAX_PARTICLES];
int numParticles = 0;
int animFrame = 0;

// Dual-Tier Concentric Shockwaves
typedef struct {
    float x, y;
    float radius;
    float maxRadius;
    float speed;
    float life;
    float decay;
    COLORREF color;
    int isInner;
} Shockwave;

#define MAX_SHOCKWAVES 32
Shockwave shockwaves[MAX_SHOCKWAVES];
int shockwaveCount = 0;

// Ambient Floating Dust Motes
#define MAX_DUST 36
typedef struct {
    float x, y;
    float vx, vy;
    float size;
    COLORREF color;
} DustMote;
DustMote dustMotes[MAX_DUST];
int dustInit = 0;

typedef struct {
    int grid[MAX_ROWS][MAX_COLS];
    int state[MAX_ROWS][MAX_COLS];
    int cols, rows, totalMines, diff;
    int gameOver, flagsPlaced, timeElapsed, firstClick;
    Move moves[MAX_MOVES];
    int movesCount;
    DWORD timeOffset;
} QuickSaveData;

QuickSaveData qs;
int qsValid = 0;

size_t MyStrLen(const char* s) {
    size_t len = 0;
    while (*s++) len++;
    return len;
}

int MyStrncmp(const char* s1, const char* s2, size_t n) {
    while (n--) {
        if (*s1 != *s2) return *s1 - *s2;
        if (!*s1) break;
        s1++; s2++;
    }
    return 0;
}

char* MyStrStr(const char* str, const char* substr) {
    if (!*substr) return (char*)str;
    while (*str) {
        const char* p1 = str;
        const char* p2 = substr;
        while (*p1 && *p2 && *p1 == *p2) { p1++; p2++; }
        if (!*p2) return (char*)str;
        str++;
    }
    return NULL;
}

int MyAtoi(const char* s) {
    int res = 0;
    int sign = 1;
    if (*s == '-') { sign = -1; s++; }
    while (*s >= '0' && *s <= '9') {
        res = res * 10 + (*s - '0');
        s++;
    }
    return res * sign;
}

#pragma function(memcpy)
void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) *bytes++ = (char)c;
    return dest;
}

int randSeed = 42;
int MyRand() {
    randSeed = randSeed * 1103515245 + 12345;
    return (unsigned int)(randSeed / 65536) % 32768;
}

void TriggerScreenShake(int intensity, DWORD durationMs) {
    shakeIntensity = intensity;
    shakeTick = GetTickCount();
    shakeDuration = durationMs;
}

void SpawnShockwave(float cx, float cy, COLORREF color, int isInner, float maxR, float speed) {
    if (shockwaveCount < MAX_SHOCKWAVES) {
        Shockwave* sw = &shockwaves[shockwaveCount++];
        sw->x = cx;
        sw->y = cy;
        sw->radius = 1.0f;
        sw->maxRadius = maxR;
        sw->speed = speed;
        sw->life = 1.0f;
        sw->decay = isInner ? 0.04f : 0.025f;
        sw->color = color;
        sw->isInner = isInner;
    }
}

void SpawnExplosion(float cx, float cy) {
    TriggerScreenShake(16, 500);
    SpawnShockwave(cx, cy, RGB(239, 68, 68), 1, 150.0f, 6.0f);
    SpawnShockwave(cx, cy, RGB(249, 115, 22), 0, 210.0f, 3.5f);

    COLORREF sparkColors[5] = {
        RGB(255, 255, 255),
        RGB(254, 240, 138),
        RGB(249, 115, 22),
        RGB(239, 68, 68),
        RGB(56, 189, 248)
    };

    COLORREF smokeColors[3] = {
        RGB(30, 41, 59),
        RGB(51, 65, 85),
        RGB(15, 23, 42)
    };

    COLORREF debrisColors[4] = {
        RGB(100, 116, 139),
        RGB(71, 85, 105),
        RGB(239, 68, 68),
        RGB(251, 191, 36)
    };

    // Layer 0: Incandescent core needle sparks
    for (int i = 0; i < 35; i++) {
        if (numParticles < MAX_PARTICLES) {
            Particle* p = &particles[numParticles++];
            p->x = cx; p->y = cy;
            p->prev_x = cx; p->prev_y = cy;
            int angleIdx = MyRand() % 16;
            float speed = 3.5f + (float)(MyRand() % 65) / 10.0f;
            p->vx = ((float)FastCos(angleIdx) / 100.0f) * speed;
            p->vy = ((float)FastSin(angleIdx) / 100.0f) * speed - 1.8f;
            p->maxLife = 20 + MyRand() % 25;
            p->life = 0;
            p->size = 1.5f + (float)(MyRand() % 2);
            p->color = sparkColors[MyRand() % 5];
            p->layer = 0;
            p->gravity = 0.10f;
            p->drag = 0.95f;
        }
    }

    // Layer 1: Expanding buoyant smoke puffs
    for (int i = 0; i < 20; i++) {
        if (numParticles < MAX_PARTICLES) {
            Particle* p = &particles[numParticles++];
            p->x = cx; p->y = cy;
            p->prev_x = cx; p->prev_y = cy;
            int angleIdx = MyRand() % 16;
            float speed = 0.8f + (float)(MyRand() % 25) / 10.0f;
            p->vx = ((float)FastCos(angleIdx) / 100.0f) * speed;
            p->vy = ((float)FastSin(angleIdx) / 100.0f) * speed - 1.2f;
            p->maxLife = 30 + MyRand() % 25;
            p->life = 0;
            p->size = 5.0f + (float)(MyRand() % 6);
            p->color = smokeColors[MyRand() % 3];
            p->layer = 1;
            p->gravity = -0.05f;
            p->drag = 0.96f;
        }
    }

    // Layer 2: Heavy rock/debris shards
    for (int i = 0; i < 25; i++) {
        if (numParticles < MAX_PARTICLES) {
            Particle* p = &particles[numParticles++];
            p->x = cx; p->y = cy;
            p->prev_x = cx; p->prev_y = cy;
            int angleIdx = MyRand() % 16;
            float speed = 2.0f + (float)(MyRand() % 50) / 10.0f;
            p->vx = ((float)FastCos(angleIdx) / 100.0f) * speed;
            p->vy = ((float)FastSin(angleIdx) / 100.0f) * speed - 2.5f;
            p->maxLife = 35 + MyRand() % 30;
            p->life = 0;
            p->size = 2.5f + (float)(MyRand() % 3);
            p->color = debrisColors[MyRand() % 4];
            p->layer = 2;
            p->rot = (float)(MyRand() % 16);
            p->vrot = ((float)(MyRand() % 10) - 5.0f) / 10.0f;
            p->gravity = 0.18f;
            p->drag = 0.98f;
        }
    }
}

void SpawnWinCelebration() {
    TriggerScreenShake(12, 600);
    COLORREF starColors[5] = {
        RGB(251, 191, 36),
        RGB(56, 189, 248),
        RGB(74, 222, 128),
        RGB(244, 114, 182),
        RGB(255, 255, 255)
    };
    int w = cols * CELL;
    int h = rows * CELL;

    for (int i = 0; i < 70; i++) {
        if (numParticles < MAX_PARTICLES) {
            Particle* p = &particles[numParticles++];
            p->x = (float)(MyRand() % w);
            p->y = (float)(MyRand() % (h / 2));
            p->prev_x = p->x; p->prev_y = p->y;
            p->vx = ((float)(MyRand() % 60) - 30.0f) / 10.0f;
            p->vy = -((float)(MyRand() % 50) + 15.0f) / 10.0f;
            p->maxLife = 45 + MyRand() % 45;
            p->life = 0;
            p->size = 2.5f + (float)(MyRand() % 3);
            p->color = starColors[MyRand() % 5];
            p->layer = 3;
            p->rot = (float)(MyRand() % 16);
            p->vrot = ((float)(MyRand() % 8) - 4.0f) / 10.0f;
            p->gravity = 0.10f;
            p->drag = 0.97f;
        }
    }
}

void InitDustMotes() {
    int w = cols * CELL;
    int h = rows * CELL;
    for (int i = 0; i < MAX_DUST; i++) {
        dustMotes[i].x = (float)(MyRand() % (w > 0 ? w : 400));
        dustMotes[i].y = (float)(MyRand() % (h > 0 ? h : 400));
        dustMotes[i].vx = ((float)(MyRand() % 20) - 10.0f) / 50.0f;
        dustMotes[i].vy = ((float)(MyRand() % 20) - 15.0f) / 50.0f;
        dustMotes[i].size = 1.0f + (float)(MyRand() % 2);
        dustMotes[i].color = RGB(56, 189, 248);
    }
    dustInit = 1;
}

void SaveStats() {
    HANDLE hFile = CreateFileA("kmine_stats.json", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    
    char buf[512];
    char temp[32];
    lstrcpyA(buf, "{");
    for (int i=0; i<3; i++) {
        if (bestTimes[i] == -1) wsprintfA(temp, "\"%d\":null", i);
        else wsprintfA(temp, "\"%d\":%d", i, bestTimes[i]);
        lstrcatA(buf, temp);
        if (i < 2) lstrcatA(buf, ",");
    }
    lstrcatA(buf, "}");
    
    DWORD written;
    WriteFile(hFile, buf, (DWORD)MyStrLen(buf), &written, NULL);
    CloseHandle(hFile);
}

void LoadStats() {
    HANDLE hFile = CreateFileA("kmine_stats.json", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    char buf[512] = {0};
    DWORD bytesRead = 0;
    ReadFile(hFile, buf, 511, &bytesRead, NULL);
    buf[bytesRead] = 0;
    CloseHandle(hFile);
    
    for (int i=0; i<3; i++) {
        char key[16];
        wsprintfA(key, "\"%d\":", i);
        char *p = MyStrStr(buf, key);
        if (p) {
            p += MyStrLen(key);
            while (*p == ' ') p++;
            if (MyStrncmp(p, "null", 4) != 0) {
                bestTimes[i] = MyAtoi(p);
            }
        }
    }
}

void UpdateTitle(HWND hwnd) {
    if (!hwnd) return;
    char bestStr[32] = "--";
    if (bestTimes[currentDiff] != -1) wsprintfA(bestStr, "%d", bestTimes[currentDiff]);

    if (gameOver == 2) {
        char buf[128];
        wsprintfA(buf, "KMine - YOU WIN! 🏆 Time: %ds (Best: %ss) | H/F1 for Help", timeElapsed, bestStr);
        SetWindowTextA(hwnd, buf);
    } else if (gameOver == 1) {
        char buf[128];
        wsprintfA(buf, "KMine - GAME OVER 💥 Time: %ds (Best: %ss) | H/F1 for Help", timeElapsed, bestStr);
        SetWindowTextA(hwnd, buf);
    } else {
        char buf[128];
        wsprintfA(buf, "KMine - Mines: %d | Time: %ds | Best: %ss | H/F1 for Help", totalMines - flagsPlaced, timeElapsed, bestStr);
        SetWindowTextA(hwnd, buf);
    }
}

#define IDM_RESTART 1000
#define IDM_BEGINNER 1001
#define IDM_INTERMEDIATE 1002
#define IDM_EXPERT 1003
#define IDM_HINT 1004
#define IDM_EXPORT_STATS 1005
#define IDM_IMPORT_STATS 1006
#define IDM_WATCH_REPLAY 1007

HMENU hMenu, hSubMenu;

void GenerateMines(int safeX, int safeY) {
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            grid[y][x] = 0;
        }
    }
    int mines = 0;
    while (mines < totalMines) {
        int x = MyRand() % cols;
        int y = MyRand() % rows;
        if (grid[y][x] != 9 && !(x == safeX && y == safeY)) {
            grid[y][x] = 9;
            mines++;
        }
    }
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            if (grid[y][x] == 9) continue;
            int count = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < cols && ny >= 0 && ny < rows && grid[ny][nx] == 9) {
                        count++;
                    }
                }
            }
            grid[y][x] = count;
        }
    }
}

void InitGame(HWND hwnd, int keepGrid) {
    if (!keepGrid) {
        randSeed = GetTickCount();
        for (int y = 0; y < rows; y++) {
            for (int x = 0; x < cols; x++) {
                grid[y][x] = 0;
            }
        }
        movesCount = 0;
        firstClick = 1;
    } else {
        firstClick = 0;
    }
    gameOver = 0;
    flagsPlaced = 0;
    timeElapsed = 0;
    isReplaying = 0;
    numParticles = 0;
    shockwaveCount = 0;
    if (hwnd) {
        KillTimer(hwnd, 1);
        KillTimer(hwnd, 2);
    }
    UpdateTitle(hwnd);
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            state[y][x] = 0;
        }
    }
    InitDustMotes();
}

void Reveal(int x, int y) {
    if (x < 0 || x >= cols || y < 0 || y >= rows || state[y][x] != 0) return;
    state[y][x] = 1;
    if (grid[y][x] == 9) {
        gameOver = 1;
        SpawnExplosion((float)(x * CELL + CELL/2), (float)(y * CELL + CELL/2));
        for (int i=0; i<rows; i++)
            for (int j=0; j<cols; j++)
                if (grid[i][j] == 9) state[i][j] = 1;
        return;
    }
    if (grid[y][x] == 0) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                Reveal(x + dx, y + dy);
            }
        }
    }
}

void ChordCell(int x, int y) {
    if (state[y][x] != 1 || grid[y][x] <= 0 || grid[y][x] >= 9) return;
    int flagCount = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < cols && ny >= 0 && ny < rows) {
                if (state[ny][nx] == 2) flagCount++;
            }
        }
    }
    if (flagCount == grid[y][x]) {
        TriggerScreenShake(4, 200);
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int nx = x + dx;
                int ny = y + dy;
                if (nx >= 0 && nx < cols && ny >= 0 && ny < rows) {
                    if (state[ny][nx] == 0) {
                        Reveal(nx, ny);
                    }
                }
            }
        }
    }
}

void CheckWin(HWND hwnd) {
    int unrevealed = 0;
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            if (state[y][x] != 1) unrevealed++;
        }
    }
    if (unrevealed == totalMines) {
        gameOver = 2; // Win
        if (!isReplaying) KillTimer(hwnd, 1);
        UpdateTitle(hwnd);
        SpawnWinCelebration();
        if (!isReplaying) {
            if (bestTimes[currentDiff] == -1 || timeElapsed < bestTimes[currentDiff]) {
                bestTimes[currentDiff] = timeElapsed;
                SaveStats();
                UpdateTitle(hwnd);
            }
        }
    }
}

void ApplyAction(HWND hwnd, int x, int y, int btn) {
    if (btn == 0) {
        if (firstClick) {
            firstClick = 0;
            GenerateMines(x, y);
            startRealTime = GetTickCount();
            if (!isReplaying) SetTimer(hwnd, 1, 1000, NULL);
        }
        if (state[y][x] == 1) {
            ChordCell(x, y);
        } else if (state[y][x] == 0) {
            Reveal(x, y);
        }
        if (gameOver == 1) {
            if (!isReplaying) KillTimer(hwnd, 1);
            UpdateTitle(hwnd);
        } else {
            CheckWin(hwnd);
        }
    } else if (btn == 2) {
        if (state[y][x] != 1) {
            if (state[y][x] == 0) {
                state[y][x] = 2;
                flagsPlaced++;
                TriggerScreenShake(2, 120);
                // Flag plant mini sparks
                for (int i = 0; i < 5; i++) {
                    if (numParticles < MAX_PARTICLES) {
                        Particle* p = &particles[numParticles++];
                        p->x = (float)(x * CELL + CELL / 2);
                        p->y = (float)(y * CELL + CELL - 4);
                        p->prev_x = p->x; p->prev_y = p->y;
                        p->vx = ((float)(MyRand() % 30) - 15.0f) / 10.0f;
                        p->vy = -((float)(MyRand() % 20) + 10.0f) / 10.0f;
                        p->maxLife = 15;
                        p->life = 0;
                        p->size = 1.5f;
                        p->color = RGB(251, 191, 36);
                        p->layer = 0;
                        p->gravity = 0.15f;
                        p->drag = 0.94f;
                    }
                }
            } else {
                state[y][x] = 0;
                flagsPlaced--;
            }
            UpdateTitle(hwnd);
        }
    }
}

void GiveHint(HWND hwnd) {
    if (gameOver != 0 || isReplaying) return;
    int actionX = -1, actionY = -1;
    if (firstClick) {
        actionX = cols / 2;
        actionY = rows / 2;
    } else {
        int count = 0;
        for (int y = 0; y < rows; y++) {
            for (int x = 0; x < cols; x++) {
                if (state[y][x] == 0 && grid[y][x] != 9) count++;
            }
        }
        if (count > 0) {
            int target = MyRand() % count;
            int current = 0;
            for (int y = 0; y < rows; y++) {
                for (int x = 0; x < cols; x++) {
                    if (state[y][x] == 0 && grid[y][x] != 9) {
                        if (current == target) {
                            actionX = x; actionY = y;
                            break;
                        }
                        current++;
                    }
                }
                if (actionX != -1) break;
            }
        }
    }
    if (actionX != -1 && movesCount < MAX_MOVES) {
        moves[movesCount].t = GetTickCount() - startRealTime;
        moves[movesCount].x = actionX;
        moves[movesCount].y = actionY;
        moves[movesCount].b = 0;
        movesCount++;
        ApplyAction(hwnd, actionX, actionY, 0);
        InvalidateRect(hwnd, NULL, FALSE);
    }
}

void SetDifficultyWindow(HWND hwnd, int c, int r) {
    cols = c; rows = r;
    RECT rc;
    rc.left = 0; rc.top = 0; rc.right = cols * CELL; rc.bottom = rows * CELL;
    AdjustWindowRect(&rc, (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN, TRUE);
    SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);
    InitDustMotes();
}

void SetDifficulty(HWND hwnd, int diff) {
    currentDiff = diff;
    if (diff == 0) { SetDifficultyWindow(hwnd, 10, 10); totalMines = 15; }
    else if (diff == 1) { SetDifficultyWindow(hwnd, 16, 16); totalMines = 40; }
    else if (diff == 2) { SetDifficultyWindow(hwnd, 30, 16); totalMines = 99; }
    InitGame(hwnd, 0);
    InvalidateRect(hwnd, NULL, TRUE);
}

void QuickSave(HWND hwnd) {
    if (gameOver != 0 || firstClick || isReplaying) {
        MessageBoxA(hwnd, "Cannot quicksave right now.", "Info", MB_OK);
        return;
    }
    memcpy(qs.grid, grid, sizeof(grid));
    memcpy(qs.state, state, sizeof(state));
    qs.cols = cols; qs.rows = rows; qs.totalMines = totalMines; qs.diff = currentDiff;
    qs.gameOver = gameOver; qs.flagsPlaced = flagsPlaced; qs.timeElapsed = timeElapsed;
    qs.firstClick = firstClick;
    memcpy(qs.moves, moves, sizeof(Move) * movesCount);
    qs.movesCount = movesCount;
    qs.timeOffset = GetTickCount() - startRealTime;
    qsValid = 1;
    HANDLE hFile = CreateFileA("kmine_qs.bin", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) { DWORD w; WriteFile(hFile, &qs, sizeof(qs), &w, NULL); CloseHandle(hFile); }
    MessageBoxA(hwnd, "Quicksaved!", "Info", MB_OK);
}

void QuickLoad(HWND hwnd) {
    if (!qsValid) {
        HANDLE hFile = CreateFileA("kmine_qs.bin", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) { DWORD r; ReadFile(hFile, &qs, sizeof(qs), &r, NULL); CloseHandle(hFile); qsValid = 1; }
    }
    if (!qsValid) {
        MessageBoxA(hwnd, "No quicksave found.", "Info", MB_OK);
        return;
    }
    isReplaying = 0;
    KillTimer(hwnd, 1);
    KillTimer(hwnd, 2);
    
    currentDiff = qs.diff;
    SetDifficultyWindow(hwnd, qs.cols, qs.rows);
    totalMines = qs.totalMines;
    
    memcpy(grid, qs.grid, sizeof(grid));
    memcpy(state, qs.state, sizeof(state));
    gameOver = qs.gameOver; flagsPlaced = qs.flagsPlaced; timeElapsed = qs.timeElapsed;
    firstClick = qs.firstClick;
    memcpy(moves, qs.moves, sizeof(Move) * qs.movesCount);
    movesCount = qs.movesCount;
    startRealTime = GetTickCount() - qs.timeOffset;
    
    SetTimer(hwnd, 1, 1000, NULL);
    UpdateTitle(hwnd);
    InvalidateRect(hwnd, NULL, TRUE);
}

void StartReplay(HWND hwnd) {
    if (movesCount == 0) return;
    InitGame(hwnd, 1);
    isReplaying = 1;
    replayStartTime = GetTickCount();
    replayMoveIdx = 0;
    SetTimer(hwnd, 2, 16, NULL);
}

void DoExportStats(HWND hwnd) {
    OPENFILENAMEA ofn = {0};
    char szFile[260] = "kmine_stats.json";
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "JSON Files\0*.json\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    if (GetSaveFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            char buf[512]; char temp[32]; lstrcpyA(buf, "{");
            for (int i=0; i<3; i++) {
                if (bestTimes[i] == -1) wsprintfA(temp, "\"%d\":null", i);
                else wsprintfA(temp, "\"%d\":%d", i, bestTimes[i]);
                lstrcatA(buf, temp); if (i < 2) lstrcatA(buf, ",");
            }
            lstrcatA(buf, "}");
            DWORD w; WriteFile(hFile, buf, (DWORD)MyStrLen(buf), &w, NULL);
            CloseHandle(hFile);
            MessageBoxA(hwnd, "Stats exported successfully.", "Export", MB_OK);
        }
    }
}

void DoImportStats(HWND hwnd) {
    OPENFILENAMEA ofn = {0};
    char szFile[260] = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "JSON Files\0*.json\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (GetOpenFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            char buf[512] = {0}; DWORD r = 0; ReadFile(hFile, buf, 511, &r, NULL); buf[r] = 0; CloseHandle(hFile);
            for (int i=0; i<3; i++) {
                char key[16]; wsprintfA(key, "\"%d\":", i);
                char *p = MyStrStr(buf, key);
                if (p) {
                    p += MyStrLen(key); while (*p == ' ') p++;
                    if (MyStrncmp(p, "null", 4) != 0) {
                        int v = MyAtoi(p);
                        if (bestTimes[i] == -1 || v < bestTimes[i]) bestTimes[i] = v;
                    }
                }
            }
            SaveStats();
            UpdateTitle(hwnd);
            MessageBoxA(hwnd, "Stats imported successfully.", "Import", MB_OK);
        }
    }
}

void DrawCornerFiligreeGDI(HDC hdc, int x, int y, int size, int flipX, int flipY) {
    int sx = flipX ? -1 : 1;
    int sy = flipY ? -1 : 1;

    HPEN bluePen = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
    HGDIOBJ oldPen = SelectObject(hdc, bluePen);
    
    MoveToEx(hdc, x, y + size * sy, NULL);
    LineTo(hdc, x, y);
    LineTo(hdc, x + size * sx, y);
    
    HPEN darkBluePen = CreatePen(PS_SOLID, 1, RGB(2, 132, 199));
    SelectObject(hdc, darkBluePen);
    DeleteObject(bluePen);
    
    MoveToEx(hdc, x + 4 * sx, y + (size - 4) * sy, NULL);
    LineTo(hdc, x + 4 * sx, y + 4 * sy);
    LineTo(hdc, x + (size - 4) * sx, y + 4 * sy);
    
    HBRUSH goldBrush = CreateSolidBrush(RGB(250, 204, 21));
    HGDIOBJ oldBrush = SelectObject(hdc, goldBrush);
    SelectObject(hdc, GetStockObject(NULL_PEN));
    
    int rx = x + 6 * sx;
    int ry = y + 6 * sy;
    Ellipse(hdc, rx - 2, ry - 2, rx + 3, ry + 3);
    
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(darkBluePen);
    DeleteObject(goldBrush);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            LoadStats();
            hMenu = CreateMenu();
            hSubMenu = CreatePopupMenu();
            AppendMenuA(hSubMenu, MF_STRING, IDM_RESTART, "New Game\tF2");
            AppendMenuA(hSubMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hSubMenu, MF_STRING, IDM_BEGINNER, "Beginner");
            AppendMenuA(hSubMenu, MF_STRING, IDM_INTERMEDIATE, "Intermediate");
            AppendMenuA(hSubMenu, MF_STRING, IDM_EXPERT, "Expert");
            AppendMenuA(hSubMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hSubMenu, MF_STRING, IDM_HINT, "Help/Hint\tH/F1");
            AppendMenuA(hSubMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hSubMenu, MF_STRING, IDM_EXPORT_STATS, "Export Stats");
            AppendMenuA(hSubMenu, MF_STRING, IDM_IMPORT_STATS, "Import Stats");
            AppendMenuA(hSubMenu, MF_STRING, IDM_WATCH_REPLAY, "Watch Replay");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, "Game");
            SetMenu(hwnd, hMenu);
            SetDifficulty(hwnd, 1);
            SetTimer(hwnd, 3, 30, NULL);
            break;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDM_RESTART) {
                InitGame(hwnd, 0);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (LOWORD(wParam) == IDM_BEGINNER) {
                SetDifficulty(hwnd, 0);
            } else if (LOWORD(wParam) == IDM_INTERMEDIATE) {
                SetDifficulty(hwnd, 1);
            } else if (LOWORD(wParam) == IDM_EXPERT) {
                SetDifficulty(hwnd, 2);
            } else if (LOWORD(wParam) == IDM_HINT) {
                GiveHint(hwnd);
            } else if (LOWORD(wParam) == IDM_EXPORT_STATS) {
                DoExportStats(hwnd);
            } else if (LOWORD(wParam) == IDM_IMPORT_STATS) {
                DoImportStats(hwnd);
            } else if (LOWORD(wParam) == IDM_WATCH_REPLAY) {
                StartReplay(hwnd);
            }
            break;
        case WM_KEYDOWN:
            if (wParam == 'H' || wParam == VK_F1) {
                GiveHint(hwnd);
            } else if (wParam == VK_F2) {
                InitGame(hwnd, 0);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == VK_F5) {
                QuickSave(hwnd);
            } else if (wParam == VK_F9) {
                QuickLoad(hwnd);
            }
            break;
        case WM_TIMER:
            if (wParam == 1) { // Game timer
                if (!gameOver && !isReplaying) {
                    timeElapsed++;
                    UpdateTitle(hwnd);
                }
            } else if (wParam == 2) { // Replay timer
                if (isReplaying) {
                    DWORD now = GetTickCount() - replayStartTime;
                    while (replayMoveIdx < movesCount && now >= moves[replayMoveIdx].t) {
                        Move m = moves[replayMoveIdx];
                        ApplyAction(hwnd, m.x, m.y, m.b);
                        replayMoveIdx++;
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                    if (replayMoveIdx >= movesCount) {
                        isReplaying = 0;
                        KillTimer(hwnd, 2);
                    }
                }
            } else if (wParam == 3) {
                animFrame++;
                int needsRedraw = 1; // Continuous smooth render
                
                // Update Dust Motes
                int w = cols * CELL;
                int h = rows * CELL;
                for (int i = 0; i < MAX_DUST; i++) {
                    dustMotes[i].x += dustMotes[i].vx;
                    dustMotes[i].y += dustMotes[i].vy;
                    if (dustMotes[i].x < 0) dustMotes[i].x = (float)w;
                    if (dustMotes[i].x > w) dustMotes[i].x = 0;
                    if (dustMotes[i].y < 0) dustMotes[i].y = (float)h;
                    if (dustMotes[i].y > h) dustMotes[i].y = 0;
                }

                // Update Shockwaves
                for (int i = 0; i < shockwaveCount; i++) {
                    shockwaves[i].radius += shockwaves[i].speed;
                    shockwaves[i].life -= shockwaves[i].decay;
                    if (shockwaves[i].life <= 0 || shockwaves[i].radius >= shockwaves[i].maxRadius) {
                        shockwaves[i] = shockwaves[shockwaveCount - 1];
                        shockwaveCount--;
                        i--;
                    }
                }

                // Update Particles
                if (numParticles > 0) {
                    for (int i=0; i<numParticles; i++) {
                        particles[i].prev_x = particles[i].x;
                        particles[i].prev_y = particles[i].y;
                        particles[i].vx *= particles[i].drag;
                        particles[i].vy *= particles[i].drag;
                        particles[i].vy += particles[i].gravity;
                        particles[i].x += particles[i].vx;
                        particles[i].y += particles[i].vy;
                        particles[i].life++;

                        // Floor bounce for heavy shards
                        if (particles[i].layer == 2 && particles[i].y >= h - 4) {
                            particles[i].y = (float)(h - 4);
                            particles[i].vy = -particles[i].vy * 0.45f;
                            particles[i].vx *= 0.8f;
                        }

                        if (particles[i].life >= particles[i].maxLife) {
                            particles[i] = particles[numParticles-1];
                            numParticles--;
                            i--;
                        }
                    }
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN: {
            if (gameOver || isReplaying) {
                if (gameOver) { InitGame(hwnd, 0); InvalidateRect(hwnd, NULL, FALSE); }
                break;
            }
            int x = LOWORD(lParam) / CELL;
            int y = HIWORD(lParam) / CELL;
            if (x < 0 || x >= cols || y < 0 || y >= rows) break;
            
            int btn = (msg == WM_LBUTTONDOWN) ? 0 : 2;
            
            if (firstClick) startRealTime = GetTickCount();
            if (movesCount < MAX_MOVES) {
                moves[movesCount].t = GetTickCount() - startRealTime;
                moves[movesCount].x = x;
                moves[movesCount].y = y;
                moves[movesCount].b = btn;
                movesCount++;
            }
            
            ApplyAction(hwnd, x, y, btn);
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HDC memDC = CreateCompatibleDC(hdc);
            int w = cols * CELL;
            int h = rows * CELL;
            HBITMAP memBM = CreateCompatibleBitmap(hdc, w, h);
            HGDIOBJ oldBM = SelectObject(memDC, memBM);
            
            // Background Dark Cyber Grid
            HBRUSH bg = CreateSolidBrush(RGB(9, 13, 22));
            RECT full = {0, 0, w, h};
            FillRect(memDC, &full, bg);
            DeleteObject(bg);

            // Ambient Dust Motes
            for (int i = 0; i < MAX_DUST; i++) {
                int dx = (int)dustMotes[i].x;
                int dy = (int)dustMotes[i].y;
                int sz = (int)dustMotes[i].size;
                HBRUSH dustBrush = CreateSolidBrush(RGB(30, 60, 90));
                RECT dr = { dx - sz, dy - sz, dx + sz + 1, dy + sz + 1 };
                FillRect(memDC, &dr, dustBrush);
                DeleteObject(dustBrush);
            }
            
            int fontHeight = -MulDiv(15, g_dpi, 72);
            HFONT hFont = CreateFontA(fontHeight, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            HGDIOBJ oldFont = SelectObject(memDC, hFont);
            SetBkMode(memDC, TRANSPARENT);
            
            int glintDiag = (animFrame % (cols + rows + 10));

            for (int y = 0; y < rows; y++) {
                for (int x = 0; x < cols; x++) {
                    RECT r;
                    r.left = x * CELL;
                    r.top = y * CELL;
                    r.right = (x + 1) * CELL;
                    r.bottom = (y + 1) * CELL;

                    if (state[y][x] == 0 || state[y][x] == 2) {
                        // Unrevealed Raised 3D Cell
                        int isGlint = ((x + y) == glintDiag);
                        COLORREF cellColor = isGlint ? RGB(51, 65, 85) : RGB(30, 41, 59);
                        HBRUSH cellBrush = CreateSolidBrush(cellColor);
                        RECT innerR = { r.left + 1, r.top + 1, r.right - 1, r.bottom - 1 };
                        FillRect(memDC, &innerR, cellBrush);
                        DeleteObject(cellBrush);

                        // 3D Bevel Highlights (Top & Left)
                        COLORREF hlColor = isGlint ? RGB(148, 163, 184) : RGB(71, 85, 105);
                        HBRUSH hlBrush = CreateSolidBrush(hlColor);
                        RECT hlTop = { r.left, r.top, r.right, r.top + 2 };
                        RECT hlLeft = { r.left, r.top, r.left + 2, r.bottom };
                        FillRect(memDC, &hlTop, hlBrush);
                        FillRect(memDC, &hlLeft, hlBrush);
                        DeleteObject(hlBrush);

                        // 3D Bevel Shadows (Bottom & Right)
                        HBRUSH shBrush = CreateSolidBrush(RGB(15, 23, 42));
                        RECT shBottom = { r.left, r.bottom - 2, r.right, r.bottom };
                        RECT shRight = { r.right - 2, r.top, r.right, r.bottom };
                        FillRect(memDC, &shBottom, shBrush);
                        FillRect(memDC, &shRight, shBrush);
                        DeleteObject(shBrush);

                        // Flag Sprite
                        if (state[y][x] == 2) {
                            int cx = r.left + CELL / 2;
                            int ptop = r.top + 5;
                            int pbottom = r.bottom - 4;
                            int pw = CELL * 2 / 30;
                            if (pw < 2) pw = 2;

                            // Weighted Base
                            HBRUSH baseBrush = CreateSolidBrush(RGB(51, 65, 85));
                            HGDIOBJ oldBrush = SelectObject(memDC, baseBrush);
                            HGDIOBJ oldPen = SelectObject(memDC, GetStockObject(NULL_PEN));
                            Ellipse(memDC, cx - CELL * 6 / 30, pbottom - 4, cx + CELL * 6 / 30, pbottom + 2);
                            SelectObject(memDC, oldBrush);
                            DeleteObject(baseBrush);

                            // Brass Flagpole
                            HBRUSH poleBrush = CreateSolidBrush(RGB(202, 138, 4));
                            RECT poleRect = { cx - pw/2, ptop, cx + pw/2 + 1, pbottom };
                            FillRect(memDC, &poleRect, poleBrush);
                            DeleteObject(poleBrush);

                            // Golden Finial Ball at Apex
                            HBRUSH orbBrush = CreateSolidBrush(RGB(250, 204, 21));
                            oldBrush = SelectObject(memDC, orbBrush);
                            Ellipse(memDC, cx - pw - 1, ptop - pw - 1, cx + pw + 2, ptop + pw + 2);
                            SelectObject(memDC, oldBrush);
                            DeleteObject(orbBrush);

                            // Animated Waving Pennant
                            int waveOffset = ((animFrame / 4) % 2 == 0) ? (CELL * 2 / 30) : 0;
                            POINT pts[3] = { 
                                { cx, ptop + 2 }, 
                                { cx + CELL * 12 / 30 - waveOffset, ptop + CELL * 6 / 30 }, 
                                { cx, ptop + CELL * 11 / 30 } 
                            };
                            HBRUSH redBrush = CreateSolidBrush(RGB(220, 38, 38));
                            HPEN goldTrim = CreatePen(PS_SOLID, 1, RGB(254, 240, 138));
                            oldBrush = SelectObject(memDC, redBrush);
                            oldPen = SelectObject(memDC, goldTrim);
                            Polygon(memDC, pts, 3);
                            SelectObject(memDC, oldPen);
                            SelectObject(memDC, oldBrush);
                            DeleteObject(redBrush);
                            DeleteObject(goldTrim);
                        }
                    } else if (state[y][x] == 1) {
                        // Revealed Recessed Cell
                        HBRUSH revBrush = CreateSolidBrush(RGB(15, 23, 42));
                        FillRect(memDC, &r, revBrush);
                        DeleteObject(revBrush);

                        // Recessed Inset Shadows
                        HBRUSH darkSh = CreateSolidBrush(RGB(2, 6, 23));
                        RECT sh1 = { r.left, r.top, r.right, r.top + 1 };
                        RECT sh2 = { r.left, r.top, r.left + 1, r.bottom };
                        FillRect(memDC, &sh1, darkSh);
                        FillRect(memDC, &sh2, darkSh);
                        DeleteObject(darkSh);

                        HBRUSH lightSh = CreateSolidBrush(RGB(30, 41, 59));
                        RECT sh3 = { r.left, r.bottom - 1, r.right, r.bottom };
                        RECT sh4 = { r.right - 1, r.top, r.right, r.bottom };
                        FillRect(memDC, &sh3, lightSh);
                        FillRect(memDC, &sh4, lightSh);
                        DeleteObject(lightSh);

                        if (grid[y][x] == 9) {
                            // Enhanced 3D Mine Sprite
                            int cx = r.left + CELL / 2;
                            int cy = r.top + CELL / 2;
                            int r1 = CELL * 12 / 30;
                            int r2 = CELL * 8 / 30;

                            // 8-directional spikes with red detonator caps
                            HPEN spikePen = CreatePen(PS_SOLID, 2, RGB(30, 41, 59));
                            HGDIOBJ oldPen = SelectObject(memDC, spikePen);
                            MoveToEx(memDC, cx, cy - r1, NULL); LineTo(memDC, cx, cy + r1);
                            MoveToEx(memDC, cx - r1, cy, NULL); LineTo(memDC, cx + r1, cy);
                            MoveToEx(memDC, cx - r2, cy - r2, NULL); LineTo(memDC, cx + r2, cy + r2);
                            MoveToEx(memDC, cx - r2, cy + r2, NULL); LineTo(memDC, cx + r2, cy - r2);
                            SelectObject(memDC, oldPen);
                            DeleteObject(spikePen);

                            // Red detonator caps
                            HBRUSH capBrush = CreateSolidBrush(RGB(239, 68, 68));
                            HGDIOBJ oldBrush = SelectObject(memDC, capBrush);
                            oldPen = SelectObject(memDC, GetStockObject(NULL_PEN));
                            Ellipse(memDC, cx - 2, cy - r1 - 2, cx + 3, cy - r1 + 3);
                            Ellipse(memDC, cx - 2, cy + r1 - 2, cx + 3, cy + r1 + 3);
                            Ellipse(memDC, cx - r1 - 2, cy - 2, cx - r1 + 3, cy + 3);
                            Ellipse(memDC, cx + r1 - 2, cy - 2, cx + r1 + 3, cy + 3);

                            // Main Spherical Body
                            HBRUSH mineBody = CreateSolidBrush(RGB(15, 23, 42));
                            SelectObject(memDC, mineBody);
                            Ellipse(memDC, cx - r2, cy - r2, cx + r2, cy + r2);
                            DeleteObject(mineBody);

                            // Blinking Red LED Core
                            int ledPulse = ((animFrame / 4) % 2 == 0);
                            HBRUSH ledBrush = CreateSolidBrush(ledPulse ? RGB(239, 68, 68) : RGB(127, 29, 29));
                            SelectObject(memDC, ledBrush);
                            Ellipse(memDC, cx - 3, cy - 3, cx + 4, cy + 4);
                            DeleteObject(ledBrush);

                            // Specular Reflection Arc
                            HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
                            SelectObject(memDC, whiteBrush);
                            Ellipse(memDC, cx - CELL * 4 / 30, cy - CELL * 4 / 30, cx - CELL / 30, cy - CELL / 30);
                            SelectObject(memDC, oldBrush);
                            SelectObject(memDC, oldPen);
                            DeleteObject(capBrush);
                            DeleteObject(whiteBrush);
                        } else if (grid[y][x] > 0) {
                            char buf[2];
                            buf[0] = '0' + grid[y][x];
                            buf[1] = 0;
                            
                            COLORREF numCol = RGB(59, 130, 246);
                            if (grid[y][x] == 2) numCol = RGB(34, 197, 94);
                            else if (grid[y][x] == 3) numCol = RGB(239, 68, 68);
                            else if (grid[y][x] == 4) numCol = RGB(129, 140, 248);
                            else if (grid[y][x] == 5) numCol = RGB(245, 158, 11);
                            else if (grid[y][x] == 6) numCol = RGB(6, 182, 212);
                            else if (grid[y][x] == 7) numCol = RGB(217, 70, 239);
                            else if (grid[y][x] == 8) numCol = RGB(148, 163, 184);
                            
                            // Drop Shadow
                            RECT shadowR = r;
                            OffsetRect(&shadowR, 1, 1);
                            SetTextColor(memDC, RGB(0, 0, 0));
                            DrawTextA(memDC, buf, -1, &shadowR, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                            // Main Number
                            SetTextColor(memDC, numCol);
                            DrawTextA(memDC, buf, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                        }
                    }
                }
            }

            // Draw Shockwaves
            for (int i = 0; i < shockwaveCount; i++) {
                int cx = (int)shockwaves[i].x;
                int cy = (int)shockwaves[i].y;
                int rad = (int)shockwaves[i].radius;
                HPEN swPen = CreatePen(PS_SOLID, shockwaves[i].isInner ? 2 : 1, shockwaves[i].color);
                HGDIOBJ oldPen = SelectObject(memDC, swPen);
                HGDIOBJ oldBrush = SelectObject(memDC, GetStockObject(NULL_BRUSH));
                Ellipse(memDC, cx - rad, cy - rad, cx + rad, cy + rad);
                SelectObject(memDC, oldBrush);
                SelectObject(memDC, oldPen);
                DeleteObject(swPen);
            }

            // Draw Kinematic Particles
            for (int i=0; i<numParticles; i++) {
                int px = (int)particles[i].x;
                int py = (int)particles[i].y;
                int size = (int)particles[i].size;
                if (size < 1) size = 1;

                HBRUSH pBrush = CreateSolidBrush(particles[i].color);
                RECT pRect = { px - size, py - size, px + size, py + size };
                FillRect(memDC, &pRect, pBrush);
                DeleteObject(pBrush);
            }

            // Pulsating Energy Perimeter Inlay Border
            HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(56, 189, 248));
            HGDIOBJ oldPen = SelectObject(memDC, borderPen);
            HGDIOBJ oldBrush = SelectObject(memDC, GetStockObject(NULL_BRUSH));
            Rectangle(memDC, 0, 0, w, h);
            SelectObject(memDC, oldBrush);
            SelectObject(memDC, oldPen);
            DeleteObject(borderPen);

            // Corner Filigree L-Brackets with Rivets
            int cornerSz = 12;
            DrawCornerFiligreeGDI(memDC, 2, 2, cornerSz, 0, 0);
            DrawCornerFiligreeGDI(memDC, w - 3, 2, cornerSz, 1, 0);
            DrawCornerFiligreeGDI(memDC, 2, h - 3, cornerSz, 0, 1);
            DrawCornerFiligreeGDI(memDC, w - 3, h - 3, cornerSz, 1, 1);
            
            // Helper Hint Overlay on start
            if (firstClick && gameOver == 0) {
                int boxW = 260 * CELL / 30;
                int boxH = 50 * CELL / 30;
                RECT rBox;
                rBox.left = w/2 - boxW/2;
                rBox.top = h/2 - boxH/2;
                rBox.right = w/2 + boxW/2;
                rBox.bottom = h/2 + boxH/2;

                HBRUSH whiteBrush = CreateSolidBrush(RGB(15, 23, 42));
                FillRect(memDC, &rBox, whiteBrush);
                DeleteObject(whiteBrush);

                HPEN boxPen = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
                oldPen = SelectObject(memDC, boxPen);
                oldBrush = SelectObject(memDC, GetStockObject(NULL_BRUSH));
                Rectangle(memDC, rBox.left, rBox.top, rBox.right, rBox.bottom);
                SelectObject(memDC, oldBrush);
                SelectObject(memDC, oldPen);
                DeleteObject(boxPen);
                
                SetBkMode(memDC, TRANSPARENT);
                SetTextColor(memDC, RGB(248, 250, 252));
                int smallFontHeight = -MulDiv(12, g_dpi, 72);
                HFONT smallFont = CreateFontA(smallFontHeight, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
                HGDIOBJ oldSmallFont = SelectObject(memDC, smallFont);
                
                RECT rText1 = { rBox.left, rBox.top + 6, rBox.right, rBox.top + 24 };
                RECT rText2 = { rBox.left, rBox.top + 26, rBox.right, rBox.top + 44 };
                DrawTextA(memDC, "L-Click: Reveal | R-Click: Flag", -1, &rText1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                SetTextColor(memDC, RGB(56, 189, 248));
                DrawTextA(memDC, "Press 'H' or 'F1' for Help", -1, &rText2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                
                SelectObject(memDC, oldSmallFont);
                DeleteObject(smallFont);
            }

            SelectObject(memDC, oldFont);
            DeleteObject(hFont);
            
            // Screen Shake Viewport Blit
            int ox = 0, oy = 0;
            if (shakeIntensity > 0) {
                DWORD elapsed = GetTickCount() - shakeTick;
                if (elapsed < shakeDuration) {
                    float decay = 1.0f - ((float)elapsed / (float)shakeDuration);
                    int curInt = (int)(shakeIntensity * decay * decay);
                    if (curInt > 0) {
                        ox = ((MyRand() % (curInt * 2 + 1)) - curInt);
                        oy = ((MyRand() % (curInt * 2 + 1)) - curInt);
                    }
                } else {
                    shakeIntensity = 0;
                }
            }

            BitBlt(hdc, ox, oy, w, h, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBM);
            DeleteObject(memBM);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
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

    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        typedef BOOL(WINAPI* SetProcessDPIAwareFunc)();
        SetProcessDPIAwareFunc setDpiAware = (SetProcessDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (setDpiAware) setDpiAware();
    }
    HDC hdc = GetDC(NULL);
    if (hdc) {
        g_dpi = GetDeviceCaps(hdc, LOGPIXELSX);
        CELL = 30 * g_dpi / 96;
        ReleaseDC(NULL, hdc);
    }

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KMineApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    RECT r;
    r.left = 0; r.top = 0; r.right = 16 * CELL; r.bottom = 16 * CELL;
    AdjustWindowRect(&r, (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN, TRUE);
    HWND hwnd = CreateWindowEx(0, "KMineApp", "KMine", (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}

