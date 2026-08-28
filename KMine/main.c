#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#define W 200
#define H 200
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

typedef struct {
    float x, y, vx, vy;
    int life, maxLife;
    COLORREF color;
} Particle;
#define MAX_PARTICLES 1000
Particle particles[MAX_PARTICLES];
int numParticles = 0;
int animFrame = 0;

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
    WriteFile(hFile, buf, MyStrLen(buf), &written, NULL);
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
        wsprintfA(buf, "KMine - YOU WIN! Time: %ds (Best: %ss) | H for Help", timeElapsed, bestStr);
        SetWindowTextA(hwnd, buf);
    } else if (gameOver == 1) {
        char buf[128];
        wsprintfA(buf, "KMine - GAME OVER! Time: %ds (Best: %ss) | H for Help", timeElapsed, bestStr);
        SetWindowTextA(hwnd, buf);
    } else {
        char buf[128];
        wsprintfA(buf, "KMine - Mines: %d | Time: %ds | Best: %ss | H for Help", totalMines - flagsPlaced, timeElapsed, bestStr);
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

int randSeed = 42;
int MyRand() {
    randSeed = randSeed * 1103515245 + 12345;
    return (unsigned int)(randSeed / 65536) % 32768;
}

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
}

void Reveal(int x, int y) {
    if (x < 0 || x >= cols || y < 0 || y >= rows || state[y][x] != 0) return;
    state[y][x] = 1;
    if (grid[y][x] == 9) {
        gameOver = 1;
        for (int i=0; i<30; i++) {
            if (numParticles >= MAX_PARTICLES) break;
            particles[numParticles].x = (float)(x * CELL + CELL/2);
            particles[numParticles].y = (float)(y * CELL + CELL/2);
            particles[numParticles].vx = ((MyRand() % 100) - 50) / 10.0f;
            particles[numParticles].vy = ((MyRand() % 100) - 50) / 10.0f;
            particles[numParticles].life = 0;
            particles[numParticles].maxLife = 20 + MyRand() % 20;
            particles[numParticles].color = RGB(255, MyRand() % 128, 0);
            numParticles++;
        }
        // reveal all
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
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, TRUE);
    SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);
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
            DWORD w; WriteFile(hFile, buf, MyStrLen(buf), &w, NULL);
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
            AppendMenuA(hSubMenu, MF_STRING, IDM_HINT, "Help/Hint\tH");
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
            if (wParam == 'H') {
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
                int needsRedraw = 0;
                if (numParticles > 0) {
                    for (int i=0; i<numParticles; i++) {
                        particles[i].x += particles[i].vx;
                        particles[i].y += particles[i].vy;
                        particles[i].life++;
                        if (particles[i].life >= particles[i].maxLife) {
                            particles[i] = particles[numParticles-1];
                            numParticles--;
                            i--;
                        }
                    }
                    needsRedraw = 1;
                }
                if (flagsPlaced > 0 && (animFrame % 5 == 0)) {
                    needsRedraw = 1;
                }
                if (needsRedraw) InvalidateRect(hwnd, NULL, FALSE);
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
            SelectObject(memDC, memBM);
            
            HBRUSH bg = CreateSolidBrush(RGB(192, 192, 192));
            RECT full = {0, 0, w, h};
            FillRect(memDC, &full, bg);
            DeleteObject(bg);
            
            HFONT hFont = CreateFontA(-(20 * CELL / 30), 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            HGDIOBJ oldFont = SelectObject(memDC, hFont);
            SetBkMode(memDC, TRANSPARENT);
            
            for (int y = 0; y < rows; y++) {
                for (int x = 0; x < cols; x++) {
                    RECT r;
                    r.left = x * CELL;
                    r.top = y * CELL;
                    r.right = (x + 1) * CELL;
                    r.bottom = (y + 1) * CELL;
                    if (state[y][x] == 0) {
                        DrawEdge(memDC, &r, EDGE_RAISED, BF_RECT | BF_MIDDLE);
                    } else if (state[y][x] == 2) {
                        DrawEdge(memDC, &r, EDGE_RAISED, BF_RECT | BF_MIDDLE);
                        int pw = CELL * 3 / 30;
                        if (pw < 1) pw = 1;
                        int pleft = r.left + CELL/2 - pw/2;
                        int ptop = r.top + CELL * 6 / 30;
                        int pbottom = r.bottom - CELL * 6 / 30;
                        HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
                        RECT poleRect = { pleft, ptop, pleft + pw, pbottom };
                        FillRect(memDC, &poleRect, blackBrush);
                        DeleteObject(blackBrush);
                        
                        HBRUSH redBrush = CreateSolidBrush(RGB(255, 0, 0));
                        int wave = ((animFrame / 5) % 2 == 0) ? (CELL * 2 / 30) : 0;
                        POINT pts[3] = { {r.left + CELL/2, ptop}, {r.left + CELL/2 + CELL*12/30 - wave, ptop + CELL*5/30}, {r.left + CELL/2, ptop + CELL*10/30} };
                        HGDIOBJ oldBrush = SelectObject(memDC, redBrush);
                        HGDIOBJ oldPen = SelectObject(memDC, GetStockObject(NULL_PEN));
                        Polygon(memDC, pts, 3);
                        SelectObject(memDC, oldPen);
                        SelectObject(memDC, oldBrush);
                        DeleteObject(redBrush);
                    } else if (state[y][x] == 1) {
                        DrawEdge(memDC, &r, EDGE_SUNKEN, BF_RECT | BF_MIDDLE);
                        if (grid[y][x] == 9) {
                            int cx = r.left + CELL/2;
                            int cy = r.top + CELL/2;
                            int r1 = CELL * 11 / 30;
                            int r2 = CELL * 8 / 30;
                            HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
                            int pw = CELL * 2 / 30;
                            if (pw < 1) pw = 1;
                            HPEN blackPen = CreatePen(PS_SOLID, pw, RGB(0, 0, 0));
                            HGDIOBJ oldBrush = SelectObject(memDC, blackBrush);
                            HGDIOBJ oldPen = SelectObject(memDC, blackPen);
                            MoveToEx(memDC, cx, cy - r1, NULL); LineTo(memDC, cx, cy + r1);
                            MoveToEx(memDC, cx - r1, cy, NULL); LineTo(memDC, cx + r1, cy);
                            MoveToEx(memDC, cx - r2, cy - r2, NULL); LineTo(memDC, cx + r2, cy + r2);
                            MoveToEx(memDC, cx - r2, cy + r2, NULL); LineTo(memDC, cx + r2, cy - r2);
                            Ellipse(memDC, cx - r2, cy - r2, cx + r2, cy + r2);
                            SelectObject(memDC, oldBrush);
                            SelectObject(memDC, oldPen);
                            DeleteObject(blackBrush);
                            DeleteObject(blackPen);
                            
                            HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
                            oldBrush = SelectObject(memDC, whiteBrush);
                            oldPen = SelectObject(memDC, GetStockObject(NULL_PEN));
                            Ellipse(memDC, cx - CELL*4/30, cy - CELL*4/30, cx - CELL/30, cy - CELL/30);
                            SelectObject(memDC, oldBrush);
                            SelectObject(memDC, oldPen);
                            DeleteObject(whiteBrush);
                        } else if (grid[y][x] > 0) {
                            char buf[2];
                            buf[0] = '0' + grid[y][x];
                            buf[1] = 0;
                            if (grid[y][x] == 1) SetTextColor(memDC, RGB(0, 0, 255));
                            else if (grid[y][x] == 2) SetTextColor(memDC, RGB(0, 128, 0));
                            else SetTextColor(memDC, RGB(255, 0, 0));
                            DrawTextA(memDC, buf, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                        }
                    }
                }
            }
            
            if (firstClick && gameOver == 0) {
                RECT rBox;
                rBox.left = w/2 - (130 * CELL)/30;
                rBox.top = h/2 - (30 * CELL)/30;
                rBox.right = w/2 + (130 * CELL)/30;
                rBox.bottom = h/2 + (30 * CELL)/30;
                HBRUSH whiteBrush = CreateSolidBrush(RGB(240, 240, 240));
                FillRect(memDC, &rBox, whiteBrush);
                DeleteObject(whiteBrush);
                DrawEdge(memDC, &rBox, EDGE_RAISED, BF_RECT);
                
                SetBkMode(memDC, TRANSPARENT);
                SetTextColor(memDC, RGB(0, 0, 0));
                HFONT smallFont = CreateFontA(-(16 * CELL / 30), 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
                HGDIOBJ oldSmallFont = SelectObject(memDC, smallFont);
                
                RECT rText1 = { rBox.left, rBox.top + 5, rBox.right, rBox.top + 25 };
                RECT rText2 = { rBox.left, rBox.top + 30, rBox.right, rBox.top + 50 };
                DrawTextA(memDC, "L-Click: Reveal | R-Click: Flag", -1, &rText1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                DrawTextA(memDC, "Press 'H' for Help", -1, &rText2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                
                SelectObject(memDC, oldSmallFont);
                DeleteObject(smallFont);
            }

            for (int i=0; i<numParticles; i++) {
                int px = (int)particles[i].x;
                int py = (int)particles[i].y;
                int size = 2 + (particles[i].maxLife - particles[i].life) * 4 / particles[i].maxLife;
                HBRUSH pBrush = CreateSolidBrush(particles[i].color);
                RECT pRect = { px - size, py - size, px + size, py + size };
                FillRect(memDC, &pRect, pBrush);
                DeleteObject(pBrush);
            }

            SelectObject(memDC, oldFont);
            DeleteObject(hFont);
            
            BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);
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

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) *bytes++ = (char)c;
    return dest;
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
        int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
        CELL = 30 * dpi / 96;
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
    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, TRUE);
    HWND hwnd = CreateWindowEx(0, "KMineApp", "KMine", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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
