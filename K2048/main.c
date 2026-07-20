#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

void* __cdecl memcpy(void* dest, const void* src, size_t sz) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (sz--) *d++ = *s++;
    return dest;
}

void* __cdecl memmove(void* dest, const void* src, size_t sz) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    if (d < s) {
        while (sz--) *d++ = *s++;
    } else {
        d += sz;
        s += sz;
        while (sz--) *--d = *--s;
    }
    return dest;
}

#define MAX_GRID 6
#define HEADER_HEIGHT 60
#define MARGIN 10

int grid_size = 4;
int grid[MAX_GRID][MAX_GRID];
int score = 0;
int bestScore = 0;
int gameOver = 0;
int timeOut = 0;
int win = 0;
int hasWon = 0;
HWND mainHwnd = NULL;
static unsigned int seed = 0;
int theme = 1; // 0=Dark, 1=Classic, 2=Pastel

int timeAttackEnabled = 0;
int timeRemaining = 60;
int gameStarted = 0;
int timerActive = 0;

int ruleset = 0; // 0=Classic, 1=Fibonacci

int autoPlayEnabled = 0;
int autoPlayTimerActive = 0;

int obstaclesEnabled = 0;
int moveCount = 0;

int bombsEnabled = 0;

int campaignMode = 0;
int campaignLevel = 1;

int powerups_shuffles = 3;
int powerups_hammers = 3;

#define MAX_HISTORY 50
typedef struct {
    int grid[MAX_GRID][MAX_GRID];
    int score;
} HistoryState;

HistoryState history[MAX_HISTORY];
int historyCount = 0;

int stats_gamesPlayed = 0;
int stats_tilesMerged = 0;
int stats_highestTile = 0;
int stats_timePlayed = 0;

int my_rand() {
    seed = seed * 214013 + 2531011;
    return (seed >> 16) & 0x7FFF;
}

void LoadBest() {
    char filename[32];
    if (ruleset == 1) {
        wsprintfA(filename, "k2048_score_%d_fib.dat", grid_size);
    } else {
        wsprintfA(filename, "k2048_score_%d.dat", grid_size);
    }
    HANDLE hFile = CreateFileA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE && grid_size == 4 && ruleset == 0) {
        hFile = CreateFileA("k2048_score.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[64] = {0};
        DWORD bytesRead;
        if (ReadFile(hFile, buf, sizeof(buf)-1, &bytesRead, NULL)) {
            bestScore = 0;
            for (int i = 0; buf[i] >= '0' && buf[i] <= '9'; i++) {
                bestScore = bestScore * 10 + (buf[i] - '0');
            }
        }
        CloseHandle(hFile);
    }
}

void LoadTheme() {
    HANDLE hFile = CreateFileA("k2048_theme.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[16] = {0};
        DWORD bytesRead;
        if (ReadFile(hFile, buf, sizeof(buf)-1, &bytesRead, NULL)) {
            theme = buf[0] - '0';
        }
        CloseHandle(hFile);
    }
    if (theme < 0 || theme > 2) theme = 1;
}

void LoadStats() {
    HANDLE hFile = CreateFileA("k2048_stats.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesRead;
        ReadFile(hFile, &stats_gamesPlayed, sizeof(int), &bytesRead, NULL);
        ReadFile(hFile, &stats_tilesMerged, sizeof(int), &bytesRead, NULL);
        ReadFile(hFile, &stats_highestTile, sizeof(int), &bytesRead, NULL);
        ReadFile(hFile, &stats_timePlayed, sizeof(int), &bytesRead, NULL);
        CloseHandle(hFile);
    }
}

void SaveBest() {
    char filename[32];
    if (ruleset == 1) {
        wsprintfA(filename, "k2048_score_%d_fib.dat", grid_size);
    } else {
        wsprintfA(filename, "k2048_score_%d.dat", grid_size);
    }
    HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[64];
        int temp = bestScore;
        int len = 0;
        if (temp == 0) buf[len++] = '0';
        else {
            char rev[64];
            int rlen = 0;
            while(temp > 0) { rev[rlen++] = '0' + (temp % 10); temp /= 10; }
            while(rlen > 0) buf[len++] = rev[--rlen];
        }
        buf[len] = 0;
        DWORD bytesWritten;
        WriteFile(hFile, buf, len, &bytesWritten, NULL);
        CloseHandle(hFile);
    }
}

void SaveTheme() {
    HANDLE hFile = CreateFileA("k2048_theme.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[2];
        buf[0] = '0' + theme;
        buf[1] = 0;
        DWORD bytesWritten;
        WriteFile(hFile, buf, 1, &bytesWritten, NULL);
        CloseHandle(hFile);
    }
}

void SaveStats() {
    HANDLE hFile = CreateFileA("k2048_stats.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten;
        WriteFile(hFile, &stats_gamesPlayed, sizeof(int), &bytesWritten, NULL);
        WriteFile(hFile, &stats_tilesMerged, sizeof(int), &bytesWritten, NULL);
        WriteFile(hFile, &stats_highestTile, sizeof(int), &bytesWritten, NULL);
        WriteFile(hFile, &stats_timePlayed, sizeof(int), &bytesWritten, NULL);
        CloseHandle(hFile);
    }
}

void AddRandomTile() {
    int emptyCount = 0;
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            if (grid[i][j] == 0) emptyCount++;
        }
    }
    if (emptyCount == 0) return;
    int target = my_rand() % emptyCount;
    emptyCount = 0;
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            if (grid[i][j] == 0) {
                if (emptyCount == target) {
                    if (obstaclesEnabled && moveCount > 0 && moveCount % 5 == 0) {
                        grid[i][j] = -1;
                    } else if (bombsEnabled && my_rand() % 30 == 0) {
                        grid[i][j] = -3; // Bomb
                    } else if (my_rand() % 20 == 0) {
                        grid[i][j] = -2; // Wildcard
                    } else {
                        if (ruleset == 1) grid[i][j] = 1;
                        else if (ruleset == 2) grid[i][j] = (my_rand() % 2 == 0) ? 1 : 2;
                        else grid[i][j] = ((my_rand() % 10 == 0) ? 4 : 2);

                        if (grid[i][j] > stats_highestTile) {
                            stats_highestTile = grid[i][j];
                            SaveStats();
                        }
                    }
                    return;
                }
                emptyCount++;
            }
        }
    }
}

void InitGame() {
    memset(grid, 0, sizeof(grid));
    score = 0;
    gameOver = 0;
    timeOut = 0;
    win = 0;
    hasWon = 0;
    historyCount = 0;
    gameStarted = 0;
    moveCount = 0;
    timeRemaining = 60;
    powerups_shuffles = 3;
    powerups_hammers = 3;
    if (timerActive) {
        KillTimer(mainHwnd, 1);
        timerActive = 0;
    }
    seed = GetTickCount();
    AddRandomTile();
    AddRandomTile();
    LoadBest();
}

void StartCampaignLevel() {
    grid_size = 4; timeAttackEnabled = 0; obstaclesEnabled = 0; bombsEnabled = 0; ruleset = 0;
    if (campaignLevel == 2) { timeAttackEnabled = 1; }
    else if (campaignLevel == 3) { obstaclesEnabled = 1; }
    else if (campaignLevel == 4) { grid_size = 5; timeAttackEnabled = 1; obstaclesEnabled = 1; }
    else if (campaignLevel == 5) { grid_size = 5; timeAttackEnabled = 1; obstaclesEnabled = 1; ruleset = 1; }
    else if (campaignLevel == 6) { grid_size = 5; bombsEnabled = 1; ruleset = 2; }
    else if (campaignLevel == 7) { grid_size = 6; obstaclesEnabled = 1; bombsEnabled = 1; }
    else if (campaignLevel == 8) { grid_size = 6; timeAttackEnabled = 1; bombsEnabled = 1; ruleset = 1; }
    else if (campaignLevel == 9) { grid_size = 4; timeAttackEnabled = 1; obstaclesEnabled = 1; ruleset = 2; }
    else if (campaignLevel == 10) { grid_size = 6; timeAttackEnabled = 1; obstaclesEnabled = 1; bombsEnabled = 1; }
    else if (campaignLevel == 11) { grid_size = 5; timeAttackEnabled = 1; }
    else if (campaignLevel == 12) { grid_size = 5; obstaclesEnabled = 1; bombsEnabled = 1; ruleset = 1; }
    else if (campaignLevel == 13) { grid_size = 4; timeAttackEnabled = 1; ruleset = 2; }
    else if (campaignLevel == 14) { grid_size = 6; obstaclesEnabled = 1; ruleset = 1; }
    else if (campaignLevel == 15) { grid_size = 5; timeAttackEnabled = 1; bombsEnabled = 1; ruleset = 2; }
    else if (campaignLevel == 16) { grid_size = 6; timeAttackEnabled = 1; }
    else if (campaignLevel == 17) { grid_size = 5; obstaclesEnabled = 1; bombsEnabled = 1; ruleset = 1; }
    else if (campaignLevel == 18) { grid_size = 6; obstaclesEnabled = 1; bombsEnabled = 1; ruleset = 2; }
    else if (campaignLevel == 19) { grid_size = 4; timeAttackEnabled = 1; obstaclesEnabled = 1; bombsEnabled = 1; }
    else if (campaignLevel >= 20) { grid_size = 6; timeAttackEnabled = 1; obstaclesEnabled = 1; bombsEnabled = 1; ruleset = 1; }
    InitGame();
    if (campaignMode) {
        if (campaignLevel == 2) timeRemaining = 120;
        else if (campaignLevel == 4) timeRemaining = 90;
        else if (campaignLevel == 5) timeRemaining = 60;
        else if (campaignLevel == 8) timeRemaining = 90;
        else if (campaignLevel == 9) timeRemaining = 60;
        else if (campaignLevel == 10) timeRemaining = 120;
        else if (campaignLevel == 11) timeRemaining = 100;
        else if (campaignLevel == 13) timeRemaining = 50;
        else if (campaignLevel == 15) timeRemaining = 80;
        else if (campaignLevel == 16) timeRemaining = 120;
        else if (campaignLevel == 19) timeRemaining = 45;
        else if (campaignLevel >= 20) timeRemaining = 90;
    }
}

int GetMergeResult(int a, int b) {
    if (a == -1 || b == -1) return 0;
    if ((a == -3 && b > 0) || (b == -3 && a > 0)) return -100;
    if (a == -2 && b > 0) {
        if (ruleset == 1) {
            int fibs[] = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181, 6765, 10946, 17711, 28657, 46368, 75025};
            for (int i = 0; i < 23; i++) if (fibs[i] == b) return fibs[i+1];
            return b;
        } else if (ruleset == 2) {
            if (b == 1 || b == 2) return 3;
            return b * 2;
        }
        return b * 2;
    }
    if (b == -2 && a > 0) return GetMergeResult(b, a);
    if (a == -2 && b == -2) return 2;
    
    if (ruleset == 1) {
        if (a == 1 && b == 1) return 2;
        int fibs[] = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181, 6765, 10946, 17711, 28657, 46368, 75025};
        int idxA = -1, idxB = -1;
        for (int i = 0; i < 24; i++) {
            if (fibs[i] == a) idxA = i;
            if (fibs[i] == b) idxB = i;
        }
        if (idxA >= 0 && idxB >= 0) {
            int diff = idxA - idxB;
            if (diff == 1 || diff == -1) return a + b;
        }
        return 0;
    } else if (ruleset == 2) {
        if ((a == 1 && b == 2) || (a == 2 && b == 1)) return 3;
        if (a >= 3 && a == b) return a * 2;
        return 0;
    } else {
        if (a == b) return a * 2;
        return 0;
    }
}

int CheckGameOver() {
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            if (grid[i][j] == 0) return 0;
            if (i < grid_size - 1 && GetMergeResult(grid[i][j], grid[i+1][j]) > 0) return 0;
            if (j < grid_size - 1 && GetMergeResult(grid[i][j], grid[i][j+1]) > 0) return 0;
        }
    }
    return 1;
}

void DrawCell(HDC hdc, int x, int y, int val, int cell_size) {
    RECT r = { x, y, x + cell_size - 8, y + cell_size - 8 };
    
    int rCol = 200, gCol = 200, bCol = 200;
    int txtCol = 0x000000;
    
    int originalVal = val;
    if (ruleset == 1 && val > 0) {
        if (val == 1) val = 2;
        else if (val == 3) val = 4;
        else if (val == 5) val = 8;
        else if (val == 13) val = 16;
        else if (val == 21) val = 32;
        else if (val == 34) val = 64;
        else if (val == 55) val = 128;
        else if (val == 89) val = 256;
        else if (val == 144) val = 512;
        else if (val == 233) val = 1024;
        else if (val >= 377) val = 2048;
    } else if (ruleset == 2 && val > 0) {
        if (val == 1) val = 2;
        else if (val == 2) val = 4;
        else if (val == 3) val = 8;
        else if (val == 6) val = 16;
        else if (val == 12) val = 32;
        else if (val == 24) val = 64;
        else if (val == 48) val = 128;
        else if (val == 96) val = 256;
        else if (val == 192) val = 512;
        else if (val == 384) val = 1024;
        else if (val >= 768) val = 2048;
    }

    if (theme == 0) { // Dark
        if (val == 2) { rCol=60; gCol=60; bCol=70; txtCol=0xdddddd; }
        else if (val == 4) { rCol=80; gCol=80; bCol=90; txtCol=0xeeeeee; }
        else if (val == 8) { rCol=242; gCol=177; bCol=121; txtCol=0xFFFFFF; }
        else if (val == 16) { rCol=245; gCol=149; bCol=99; txtCol=0xFFFFFF; }
        else if (val == 32) { rCol=246; gCol=124; bCol=95; txtCol=0xFFFFFF; }
        else if (val == 64) { rCol=246; gCol=94; bCol=59; txtCol=0xFFFFFF; }
        else if (val == 128) { rCol=237; gCol=207; bCol=114; txtCol=0xFFFFFF; }
        else if (val == 256) { rCol=237; gCol=204; bCol=97; txtCol=0xFFFFFF; }
        else if (val == 512) { rCol=237; gCol=200; bCol=80; txtCol=0xFFFFFF; }
        else if (val == 1024) { rCol=237; gCol=197; bCol=63; txtCol=0xFFFFFF; }
        else if (val == 2048) { rCol=237; gCol=194; bCol=46; txtCol=0xFFFFFF; }
        else if (val > 2048) { rCol=60; gCol=58; bCol=50; txtCol=0xFFFFFF; }
        if (val == -3) { rCol=220; gCol=20; bCol=20; txtCol=0xFFFFFF; }
        if (val == -2) { rCol=255; gCol=215; bCol=0; txtCol=0x000000; }
        if (val == -1) { rCol=85; gCol=85; bCol=85; txtCol=0xFFFFFF; }
        if (val == 0) { rCol = 40; gCol = 40; bCol = 40; }
    } else if (theme == 1) { // Classic
        if (val == 2) { rCol=238; gCol=228; bCol=218; txtCol=RGB(119,110,101); }
        else if (val == 4) { rCol=237; gCol=224; bCol=200; txtCol=RGB(119,110,101); }
        else if (val == 8) { rCol=242; gCol=177; bCol=121; txtCol=0xFFFFFF; }
        else if (val == 16) { rCol=245; gCol=149; bCol=99; txtCol=0xFFFFFF; }
        else if (val == 32) { rCol=246; gCol=124; bCol=95; txtCol=0xFFFFFF; }
        else if (val == 64) { rCol=246; gCol=94; bCol=59; txtCol=0xFFFFFF; }
        else if (val == 128) { rCol=237; gCol=207; bCol=114; txtCol=0xFFFFFF; }
        else if (val == 256) { rCol=237; gCol=204; bCol=97; txtCol=0xFFFFFF; }
        else if (val == 512) { rCol=237; gCol=200; bCol=80; txtCol=0xFFFFFF; }
        else if (val == 1024) { rCol=237; gCol=197; bCol=63; txtCol=0xFFFFFF; }
        else if (val == 2048) { rCol=237; gCol=194; bCol=46; txtCol=0xFFFFFF; }
        else if (val > 2048) { rCol=60; gCol=58; bCol=50; txtCol=0xFFFFFF; }
        if (val == -3) { rCol=220; gCol=20; bCol=20; txtCol=0xFFFFFF; }
        if (val == -2) { rCol=255; gCol=215; bCol=0; txtCol=0x000000; }
        if (val == -1) { rCol=85; gCol=85; bCol=85; txtCol=0xFFFFFF; }
        if (val == 0) { rCol = 204; gCol = 192; bCol = 179; }
    } else { // Pastel
        if (val == 2) { rCol=225; gCol=190; bCol=231; txtCol=RGB(74,20,140); }
        else if (val == 4) { rCol=209; gCol=196; bCol=233; txtCol=RGB(74,20,140); }
        else if (val == 8) { rCol=197; gCol=202; bCol=233; txtCol=RGB(74,20,140); }
        else if (val == 16) { rCol=187; gCol=222; bCol=251; txtCol=RGB(74,20,140); }
        else if (val == 32) { rCol=179; gCol=229; bCol=252; txtCol=RGB(74,20,140); }
        else if (val == 64) { rCol=178; gCol=235; bCol=242; txtCol=RGB(74,20,140); }
        else if (val == 128) { rCol=178; gCol=223; bCol=219; txtCol=RGB(74,20,140); }
        else if (val == 256) { rCol=200; gCol=230; bCol=201; txtCol=RGB(74,20,140); }
        else if (val == 512) { rCol=220; gCol=237; bCol=200; txtCol=RGB(74,20,140); }
        else if (val == 1024) { rCol=240; gCol=244; bCol=195; txtCol=RGB(74,20,140); }
        else if (val == 2048) { rCol=255; gCol=249; bCol=196; txtCol=RGB(74,20,140); }
        else if (val > 2048) { rCol=150; gCol=150; bCol=150; txtCol=RGB(74,20,140); }
        if (val == -3) { rCol=220; gCol=20; bCol=20; txtCol=0xFFFFFF; }
        if (val == -2) { rCol=255; gCol=215; bCol=0; txtCol=0x000000; }
        if (val == -1) { rCol=100; gCol=100; bCol=100; txtCol=0xFFFFFF; }
        if (val == 0) { rCol = 255; gCol = 255; bCol = 255; }
    }

    HBRUSH bg = CreateSolidBrush(RGB(rCol, gCol, bCol));
    FillRect(hdc, &r, bg);
    DeleteObject(bg);

    if (originalVal != 0) {
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, txtCol);
        char buf[16];
        if (originalVal == -3) {
            buf[0] = 'B';
            buf[1] = 0;
        } else if (originalVal == -2) {
            buf[0] = 'W';
            buf[1] = 0;
        } else if (originalVal == -1) {
            buf[0] = 'X';
            buf[1] = 0;
        } else {
            int temp = originalVal;
            int len = 0;
            char rev[16];
            int rlen = 0;
            while(temp > 0) { rev[rlen++] = '0' + (temp % 10); temp /= 10; }
            while(rlen > 0) buf[len++] = rev[--rlen];
            buf[len] = 0;
        }

        int fontSize = cell_size / 3;
        if (fontSize < 12) fontSize = 12;
        HFONT hFont = CreateFontA(fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
        HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
        DrawTextA(hdc, buf, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, oldFont);
        DeleteObject(hFont);
    }
}

void DrawBoard(HDC hdc) {
    RECT bgRect = {0, 0, 800, 600};
    
    int bgR = 250, bgG = 248, bgB = 239;
    int txtR = 119, txtG = 110, txtB = 101;
    int boardR = 187, boardG = 173, boardB = 160;
    
    if (theme == 0) {
        bgR = 30; bgG = 30; bgB = 30;
        txtR = 255; txtG = 255; txtB = 255;
        boardR = 20; boardG = 20; boardB = 20;
    } else if (theme == 2) {
        bgR = 252; bgG = 228; bgB = 236;
        txtR = 74; txtG = 20; txtB = 140;
        boardR = 248; boardG = 187; boardB = 208;
    }

    HBRUSH bgb = CreateSolidBrush(RGB(bgR, bgG, bgB));
    FillRect(hdc, &bgRect, bgb);
    DeleteObject(bgb);

    // Header
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(txtR, txtG, txtB));
    HFONT hFontTitle = CreateFontA(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
    HFONT oldFont = (HFONT)SelectObject(hdc, hFontTitle);
    
    RECT titleRect = { MARGIN, MARGIN, 200, HEADER_HEIGHT };
    DrawTextA(hdc, "2048", -1, &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    
    SelectObject(hdc, oldFont);
    DeleteObject(hFontTitle);

    char scoreBuf[128];
    if (campaignMode) {
        int target = 128;
        if (campaignLevel == 3) target = 256;
        else if (campaignLevel == 4) target = 512;
        else if (campaignLevel == 5) target = 55;
        else if (campaignLevel == 6) target = 192;
        else if (campaignLevel == 7) target = 1024;
        else if (campaignLevel == 8) target = 144;
        else if (campaignLevel == 9) target = 96;
        else if (campaignLevel >= 10) target = 2048;
        wsprintfA(scoreBuf, "Lvl %d  Score: %d  Target: %d", campaignLevel, score, target);
        if (timeAttackEnabled) {
            wsprintfA(scoreBuf, "Lvl %d Target: %d Time: %ds", campaignLevel, target, timeRemaining);
        }
    } else if (timeAttackEnabled) {
        wsprintfA(scoreBuf, "Score: %d  Best: %d  Time: %ds", score, bestScore, timeRemaining);
    } else {
        wsprintfA(scoreBuf, "Score: %d  Best: %d", score, bestScore);
    }
    
    HFONT hFontText = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
    oldFont = (HFONT)SelectObject(hdc, hFontText);
    
    int cell_size = 320 / grid_size;
    RECT scoreRect = { 150, MARGIN, MARGIN + grid_size*cell_size, HEADER_HEIGHT / 2 + MARGIN };
    DrawTextA(hdc, scoreBuf, -1, &scoreRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

    char helpBuf[128];
    wsprintfA(helpBuf, "[H]elp [C]amp [M]ode [F]Rule [U]ndo");
    RECT helpRect = { 150, HEADER_HEIGHT / 2, MARGIN + grid_size*cell_size, HEADER_HEIGHT };
    DrawTextA(hdc, helpBuf, -1, &helpRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

    char helpBuf2[128];
    wsprintfA(helpBuf2, "Pwrs: Shuffle(E):%d Hammer(Q):%d", powerups_shuffles, powerups_hammers);
    RECT helpRect2 = { MARGIN, HEADER_HEIGHT / 2, 250, HEADER_HEIGHT };
    DrawTextA(hdc, helpBuf2, -1, &helpRect2, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    RECT boardBg = { MARGIN, HEADER_HEIGHT, MARGIN + grid_size*cell_size + 8, HEADER_HEIGHT + grid_size*cell_size + 8 };
    HBRUSH boardBrush = CreateSolidBrush(RGB(boardR, boardG, boardB));
    FillRect(hdc, &boardBg, boardBrush);
    DeleteObject(boardBrush);

    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            DrawCell(hdc, MARGIN + 8 + j*cell_size, HEADER_HEIGHT + 8 + i*cell_size, grid[i][j], cell_size);
        }
    }

    if (gameOver || win) {
        HBRUSH overlay = CreateSolidBrush(gameOver ? RGB(238,228,218) : RGB(237,194,46));
        // Pseudo-transparency is hard in basic GDI, we just draw a solid rect in the middle
        RECT msgRect = { MARGIN + 20, HEADER_HEIGHT + 100, MARGIN + grid_size*cell_size - 12, HEADER_HEIGHT + 200 };
        FillRect(hdc, &msgRect, overlay);
        DeleteObject(overlay);
        SetTextColor(hdc, gameOver ? RGB(119,110,101) : RGB(255,255,255));
        
        char* msg = "You Win! (Press R)";
        if (gameOver) {
            msg = timeOut ? "Time's Up! (Press R)" : "Game Over! (Press R)";
        }
        DrawTextA(hdc, msg, -1, &msgRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    SelectObject(hdc, oldFont);
    DeleteObject(hFontText);
}

int Move(int dx, int dy) {
    if (gameOver || win) return 0;
    int moved = 0;

    int tempGrid[MAX_GRID][MAX_GRID];
    int tempScore = score;
    memcpy(tempGrid, grid, sizeof(grid));
    
    int startI = (dy == 1) ? grid_size - 1 : 0;
    int endI = (dy == 1) ? -1 : grid_size;
    int stepI = (dy == 1) ? -1 : 1;
    
    int startJ = (dx == 1) ? grid_size - 1 : 0;
    int endJ = (dx == 1) ? -1 : grid_size;
    int stepJ = (dx == 1) ? -1 : 1;
    
    int merged[MAX_GRID][MAX_GRID] = {0};

    for (int i = startI; i != endI; i += stepI) {
        for (int j = startJ; j != endJ; j += stepJ) {
            if (grid[i][j] != 0) {
                int ci = i;
                int cj = j;
                while (1) {
                    int ni = ci + dy;
                    int nj = cj + dx;
                    if (ni < 0 || ni >= grid_size || nj < 0 || nj >= grid_size) break;
                    if (grid[ni][nj] == 0) {
                        grid[ni][nj] = grid[ci][cj];
                        grid[ci][cj] = 0;
                        ci = ni;
                        cj = nj;
                        moved = 1;
                    } else if (GetMergeResult(grid[ci][cj], grid[ni][nj]) != 0 && !merged[ni][nj]) {
                        int mergeRes = GetMergeResult(grid[ci][cj], grid[ni][nj]);
                        if (mergeRes == -100) {
                            grid[ni][nj] = 0;
                            grid[ci][cj] = 0;
                            score += 10; // small bonus
                        } else {
                            grid[ni][nj] = mergeRes;
                            grid[ci][cj] = 0;
                            score += grid[ni][nj];
                            stats_tilesMerged++;
                            if (grid[ni][nj] > stats_highestTile) {
                                stats_highestTile = grid[ni][nj];
                            }
                            if (grid[ni][nj] == 2048 && !hasWon && !campaignMode) {
                                win = 1;
                                hasWon = 1;
                            }
                        }
                        if (score > bestScore) {
                            bestScore = score;
                            SaveBest();
                        }
                        merged[ni][nj] = 1;
                        moved = 1;
                        if (campaignMode && mergeRes > 0) {
                            int target = 128;
                            if (campaignLevel == 3) target = 256;
                            else if (campaignLevel == 4) target = 512;
                            else if (campaignLevel == 5) target = 55;
                            else if (campaignLevel == 6) target = 192;
                            else if (campaignLevel == 7) target = 1024;
                            else if (campaignLevel == 8) target = 144;
                            else if (campaignLevel == 9) target = 96;
                            else if (campaignLevel == 10) target = 2048;
                            else if (campaignLevel == 11) target = 1024;
                            else if (campaignLevel == 12) target = 144;
                            else if (campaignLevel == 13) target = 48;
                            else if (campaignLevel == 14) target = 1597;
                            else if (campaignLevel == 15) target = 384;
                            else if (campaignLevel == 16) target = 4096;
                            else if (campaignLevel == 17) target = 610;
                            else if (campaignLevel == 18) target = 768;
                            else if (campaignLevel == 19) target = 8192;
                            else if (campaignLevel >= 20) target = 2584;

                            if (grid[ni][nj] >= target) {
                                char msg[64];
                                wsprintfA(msg, "Level %d Complete!", campaignLevel);
                                MessageBoxA(mainHwnd, msg, "Campaign", MB_OK);
                                campaignLevel++;
                                StartCampaignLevel();
                                return 1;
                            }
                        }
                        break;
                    } else {
                        break;
                    }
                }
            }
        }
    }
    
    if (moved) {
        moveCount++;
        if (!gameStarted) {
            gameStarted = 1;
            if (timeAttackEnabled) {
                SetTimer(mainHwnd, 1, 1000, NULL);
                timerActive = 1;
            }
        }

        if (historyCount < MAX_HISTORY) {
            memcpy(&history[historyCount].grid, tempGrid, sizeof(tempGrid));
            history[historyCount].score = tempScore;
            historyCount++;
        } else {
            memmove(&history[0], &history[1], sizeof(HistoryState) * (MAX_HISTORY - 1));
            memcpy(&history[MAX_HISTORY - 1].grid, tempGrid, sizeof(tempGrid));
            history[MAX_HISTORY - 1].score = tempScore;
        }

        int note = 400;
        int max_val = 0;
        for (int i=0;i<grid_size;i++) for(int j=0;j<grid_size;j++) if(grid[i][j] > max_val) max_val = grid[i][j];
        int v = max_val;
        while (v > 1) { note += 50; v >>= 1; }
        Beep(note, 30);
        AddRandomTile();
        SaveStats();
        if (CheckGameOver()) {
            gameOver = 1;
            stats_gamesPlayed++;
            SaveStats();
            if (timerActive) { KillTimer(mainHwnd, 1); timerActive = 0; }
            SaveBest();
        }
    }
    return moved;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            mainHwnd = hwnd;
            LoadTheme();
            LoadStats();
            InitGame();
            SetTimer(hwnd, 3, 1000, NULL);
            return 0;
        case WM_ERASEBKGND:
            return 1; // Prevent background clear to fix flickering
        case WM_TIMER:
            if (wParam == 3) {
                if (!gameOver && gameStarted && !win) {
                    stats_timePlayed++;
                    if (stats_timePlayed % 10 == 0) SaveStats();
                }
                return 0;
            } else if (wParam == 1) {
                timeRemaining--;
                if (timeRemaining <= 0) {
                    KillTimer(hwnd, 1);
                    timerActive = 0;
                    gameOver = 1;
                    timeOut = 1;
                    stats_gamesPlayed++;
                    SaveStats();
                }
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 2) {
                if (!gameOver && !win) {
                    int dirs[4][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}};
                    int startIdx = my_rand() % 4;
                    int moved = 0;
                    for (int i = 0; i < 4; i++) {
                        int idx = (startIdx + i) % 4;
                        if (Move(dirs[idx][0], dirs[idx][1])) {
                            moved = 1;
                            break;
                        }
                    }
                    if (moved) InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            return 0;
        case WM_KEYDOWN:
            if (wParam >= '3' && wParam <= '6') {
                grid_size = wParam - '0';
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'C' || wParam == 'c') {
                campaignMode = !campaignMode;
                campaignLevel = 1;
                if (campaignMode) StartCampaignLevel(); else InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'M' || wParam == 'm') {
                timeAttackEnabled = !timeAttackEnabled;
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'R' || wParam == 'r') {
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'P' || wParam == 'p') {
                autoPlayEnabled = !autoPlayEnabled;
                if (autoPlayEnabled) {
                    SetTimer(hwnd, 2, 250, NULL);
                    autoPlayTimerActive = 1;
                } else {
                    KillTimer(hwnd, 2);
                    autoPlayTimerActive = 0;
                }
            } else if (wParam == 'O' || wParam == 'o') {
                obstaclesEnabled = !obstaclesEnabled;
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'B' || wParam == 'b') {
                bombsEnabled = !bombsEnabled;
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'F' || wParam == 'f') {
                ruleset = (ruleset + 1) % 3;
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'T' || wParam == 't') {
                theme = (theme + 1) % 3;
                SaveTheme();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'I' || wParam == 'i') {
                char statsBuf[256];
                wsprintfA(statsBuf, "Games Played: %d\nTiles Merged: %d\nHighest Tile: %d\nTime Played: %dm %ds", 
                    stats_gamesPlayed, stats_tilesMerged, stats_highestTile, stats_timePlayed / 60, stats_timePlayed % 60);
                MessageBoxA(hwnd, statsBuf, "Statistics", MB_OK | MB_ICONINFORMATION);
            } else if (wParam == 'H' || wParam == 'h') {
                char helpText[1024];
                wsprintfA(helpText,
                    "Controls: Arrow Keys or WASD to move tiles.\n\n"
                    "Hotkeys:\n"
                    "H: Help\n"
                    "U / Ctrl+Z: Undo Move\n"
                    "E: Shuffle Powerup\n"
                    "Q: Hammer Powerup\n"
                    "P: Auto-Play Toggle\n"
                    "O: Obstacles Toggle\n"
                    "B: Bombs Toggle\n"
                    "F: Cycle Ruleset (Classic/Fib/Threes)\n"
                    "T: Change Theme (Dark/Classic/Pastel)\n"
                    "I: View Statistics\n"
                    "M: Time Attack Mode Toggle\n"
                    "R: Restart Game\n"
                    "3, 4, 5, 6: Change Grid Size\n\n"
                    "Fibonacci/Threes Mode: Merge Fib numbers or 1+2=3, 3+3=6.\n"
                    "Obstacles/Bombs: 'X' blocks movement. 'B' destroys any tile.\n"
                    "Time Attack: Get the highest score in 60 seconds."
                );
                MessageBoxA(hwnd, helpText, "How to Play", MB_OK | MB_ICONINFORMATION);
            } else if (wParam == 'U' || wParam == 'u' || (wParam == 'Z' && (GetKeyState(VK_CONTROL) & 0x8000))) {
                if (historyCount > 0) {
                    historyCount--;
                    memcpy(grid, &history[historyCount].grid, sizeof(grid));
                    score = history[historyCount].score;
                    gameOver = 0;
                    timeOut = 0;
                    win = 0;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (wParam == 'E' || wParam == 'e') {
                if (powerups_shuffles > 0 && !gameOver && !win) {
                    powerups_shuffles--;
                    int tiles[MAX_GRID * MAX_GRID];
                    int count = 0;
                    for (int i=0; i<grid_size; i++) {
                        for (int j=0; j<grid_size; j++) {
                            if (grid[i][j] != 0 && grid[i][j] != -1) {
                                tiles[count++] = grid[i][j];
                                grid[i][j] = 0;
                            }
                        }
                    }
                    for (int i=0; i<count; i++) {
                        int swapIdx = i + (my_rand() % (count - i));
                        int temp = tiles[i];
                        tiles[i] = tiles[swapIdx];
                        tiles[swapIdx] = temp;
                    }
                    int idx = 0;
                    for (int i=0; i<grid_size; i++) {
                        for (int j=0; j<grid_size; j++) {
                            if (grid[i][j] != -1 && idx < count) {
                                grid[i][j] = tiles[idx++];
                            }
                        }
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (wParam == 'Q' || wParam == 'q') {
                if (powerups_hammers > 0 && !gameOver && !win) {
                    int minVal = 9999999;
                    int minI = -1, minJ = -1;
                    for (int i=0; i<grid_size; i++) {
                        for (int j=0; j<grid_size; j++) {
                            if (grid[i][j] > 0 && grid[i][j] < minVal) {
                                minVal = grid[i][j];
                                minI = i; minJ = j;
                            }
                        }
                    }
                    if (minI != -1) {
                        powerups_hammers--;
                        grid[minI][minJ] = 0;
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                }
            } else if (!gameOver && !win) {
                if (autoPlayEnabled) {
                    autoPlayEnabled = 0;
                    KillTimer(hwnd, 2);
                    autoPlayTimerActive = 0;
                }
                int moved = 0;
                if (wParam == VK_LEFT || wParam == 'A') moved = Move(-1, 0);
                else if (wParam == VK_RIGHT || wParam == 'D') moved = Move(1, 0);
                else if (wParam == VK_UP || wParam == 'W') moved = Move(0, -1);
                else if (wParam == VK_DOWN || wParam == 'S') moved = Move(0, 1);
                if (moved) InvalidateRect(hwnd, NULL, TRUE);
            } else if (win) {
                // allow playing past 2048
                if (wParam == VK_SPACE || wParam == VK_RETURN) {
                    win = 0;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Double buffering
            RECT rc;
            GetClientRect(hwnd, &rc);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
            
            DrawBoard(memDC);
            
            BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            SaveBest();
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

#ifndef EXCLUDE_MAIN
void __stdcall WINUSERAPI PostQuitMessage(int nExitCode);
LRESULT __stdcall WINUSERAPI DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

int WINAPI MainEntry() {
    HINSTANCE hInstance = GetModuleHandleA(NULL);
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "K2048Class";
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursorA(NULL, (LPCSTR)32512); // IDC_ARROW
    
    RegisterClassA(&wc);
    
    int winWidth = MARGIN * 2 + 4 * 80 + 8 + 16;
    int winHeight = HEADER_HEIGHT + MARGIN + 4 * 80 + 8 + 39;

    HWND hwnd = CreateWindowExA(
        0,
        "K2048Class",
        "KiloOS - 2048",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, winWidth, winHeight,
        NULL, NULL, hInstance, NULL
    );
    
    if (!hwnd) return 0;
    
    ShowWindow(hwnd, SW_SHOW);
    
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return 0;
}
#endif
