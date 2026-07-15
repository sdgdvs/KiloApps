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

#define MAX_HISTORY 50
typedef struct {
    int grid[MAX_GRID][MAX_GRID];
    int score;
} HistoryState;

HistoryState history[MAX_HISTORY];
int historyCount = 0;

int my_rand() {
    seed = seed * 214013 + 2531011;
    return (seed >> 16) & 0x7FFF;
}

void LoadBest() {
    char filename[32];
    wsprintfA(filename, "k2048_score_%d.dat", grid_size);
    HANDLE hFile = CreateFileA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE && grid_size == 4) {
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

void SaveBest() {
    char filename[32];
    wsprintfA(filename, "k2048_score_%d.dat", grid_size);
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
                    grid[i][j] = (my_rand() % 10 == 0) ? 4 : 2;
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
    timeRemaining = 60;
    if (timerActive) {
        KillTimer(mainHwnd, 1);
        timerActive = 0;
    }
    seed = GetTickCount();
    AddRandomTile();
    AddRandomTile();
    LoadBest();
}

int CheckGameOver() {
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            if (grid[i][j] == 0) return 0;
            if (i < grid_size - 1 && grid[i][j] == grid[i+1][j]) return 0;
            if (j < grid_size - 1 && grid[i][j] == grid[i][j+1]) return 0;
        }
    }
    return 1;
}

void DrawCell(HDC hdc, int x, int y, int val, int cell_size) {
    RECT r = { x, y, x + cell_size - 8, y + cell_size - 8 };
    
    int rCol = 200, gCol = 200, bCol = 200;
    int txtCol = 0x000000;
    
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
        if (val == 0) { rCol = 255; gCol = 255; bCol = 255; }
    }

    HBRUSH bg = CreateSolidBrush(RGB(rCol, gCol, bCol));
    FillRect(hdc, &r, bg);
    DeleteObject(bg);

    if (val > 0) {
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, txtCol);
        char buf[16];
        int temp = val;
        int len = 0;
        char rev[16];
        int rlen = 0;
        while(temp > 0) { rev[rlen++] = '0' + (temp % 10); temp /= 10; }
        while(rlen > 0) buf[len++] = rev[--rlen];
        buf[len] = 0;

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
    if (timeAttackEnabled) {
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
    wsprintfA(helpBuf, "Size(3-6) [M]ode [U]ndo [T]heme");
    RECT helpRect = { 150, HEADER_HEIGHT / 2, MARGIN + grid_size*cell_size, HEADER_HEIGHT };
    DrawTextA(hdc, helpBuf, -1, &helpRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

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
                    } else if (grid[ni][nj] == grid[ci][cj] && !merged[ni][nj]) {
                        grid[ni][nj] *= 2;
                        grid[ci][cj] = 0;
                        score += grid[ni][nj];
                        if (score > bestScore) {
                            bestScore = score;
                            SaveBest();
                        }
                        merged[ni][nj] = 1;
                        moved = 1;
                        if (grid[ni][nj] == 2048 && !hasWon) {
                            win = 1;
                            hasWon = 1;
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
        if (timeAttackEnabled && !gameStarted) {
            gameStarted = 1;
            SetTimer(mainHwnd, 1, 1000, NULL);
            timerActive = 1;
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

        MessageBeep(MB_OK); // simple sound effect
        AddRandomTile();
        if (CheckGameOver()) {
            gameOver = 1;
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
            InitGame();
            return 0;
        case WM_ERASEBKGND:
            return 1; // Prevent background clear to fix flickering
        case WM_TIMER:
            if (wParam == 1) {
                timeRemaining--;
                if (timeRemaining <= 0) {
                    KillTimer(hwnd, 1);
                    timerActive = 0;
                    gameOver = 1;
                    timeOut = 1;
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;
        case WM_KEYDOWN:
            if (wParam >= '3' && wParam <= '6') {
                grid_size = wParam - '0';
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'M' || wParam == 'm') {
                timeAttackEnabled = !timeAttackEnabled;
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'R' || wParam == 'r') {
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'T' || wParam == 't') {
                theme = (theme + 1) % 3;
                SaveTheme();
                InvalidateRect(hwnd, NULL, TRUE);
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
            } else if (!gameOver && !win) {
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
            SelectObject(memDC, memBitmap);
            
            DrawBoard(memDC);
            
            BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
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
