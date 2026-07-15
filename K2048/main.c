#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

#define CELL_SIZE 80
#define HEADER_HEIGHT 60
#define MARGIN 10
#define GRID_SIZE 4

int grid[GRID_SIZE][GRID_SIZE];
int score = 0;
int bestScore = 0;
int gameOver = 0;
int win = 0;
int hasWon = 0;
HWND mainHwnd = NULL;
static unsigned int seed = 0;

int my_rand() {
    seed = seed * 214013 + 2531011;
    return (seed >> 16) & 0x7FFF;
}

void LoadBest() {
    HANDLE hFile = CreateFileA("k2048_score.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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

void SaveBest() {
    HANDLE hFile = CreateFileA("k2048_score.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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

void AddRandomTile() {
    int emptyCount = 0;
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 0) emptyCount++;
        }
    }
    if (emptyCount == 0) return;
    int target = my_rand() % emptyCount;
    emptyCount = 0;
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
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
    win = 0;
    hasWon = 0;
    seed = GetTickCount();
    AddRandomTile();
    AddRandomTile();
    LoadBest();
}

int CheckGameOver() {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 0) return 0;
            if (i < GRID_SIZE - 1 && grid[i][j] == grid[i+1][j]) return 0;
            if (j < GRID_SIZE - 1 && grid[i][j] == grid[i][j+1]) return 0;
        }
    }
    return 1;
}

void DrawCell(HDC hdc, int x, int y, int val) {
    RECT r = { x, y, x + CELL_SIZE - 8, y + CELL_SIZE - 8 };
    
    int rCol = 200, gCol = 200, bCol = 200;
    int txtCol = 0x000000;
    
    if (val == 2) { rCol=238; gCol=228; bCol=218; }
    else if (val == 4) { rCol=237; gCol=224; bCol=200; }
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
    
    if (val == 0) {
        rCol = 204; gCol = 192; bCol = 179;
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

        HFONT hFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
        HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
        DrawTextA(hdc, buf, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, oldFont);
        DeleteObject(hFont);
    }
}

void DrawBoard(HDC hdc) {
    RECT bgRect = {0, 0, 800, 600};
    HBRUSH bgb = CreateSolidBrush(RGB(250, 248, 239));
    FillRect(hdc, &bgRect, bgb);
    DeleteObject(bgb);

    // Header
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(119, 110, 101));
    HFONT hFontTitle = CreateFontA(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
    HFONT oldFont = (HFONT)SelectObject(hdc, hFontTitle);
    
    RECT titleRect = { MARGIN, MARGIN, 200, HEADER_HEIGHT };
    DrawTextA(hdc, "2048", -1, &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    
    SelectObject(hdc, oldFont);
    DeleteObject(hFontTitle);

    char scoreBuf[64];
    wsprintfA(scoreBuf, "Score: %d   Best: %d", score, bestScore);
    
    HFONT hFontText = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
    oldFont = (HFONT)SelectObject(hdc, hFontText);
    
    RECT scoreRect = { 200, MARGIN, MARGIN + GRID_SIZE*CELL_SIZE, HEADER_HEIGHT };
    DrawTextA(hdc, scoreBuf, -1, &scoreRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

    RECT boardBg = { MARGIN, HEADER_HEIGHT, MARGIN + GRID_SIZE*CELL_SIZE + 8, HEADER_HEIGHT + GRID_SIZE*CELL_SIZE + 8 };
    HBRUSH boardBrush = CreateSolidBrush(RGB(187, 173, 160));
    FillRect(hdc, &boardBg, boardBrush);
    DeleteObject(boardBrush);

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            DrawCell(hdc, MARGIN + 8 + j*CELL_SIZE, HEADER_HEIGHT + 8 + i*CELL_SIZE, grid[i][j]);
        }
    }

    if (gameOver || win) {
        HBRUSH overlay = CreateSolidBrush(gameOver ? RGB(238,228,218) : RGB(237,194,46));
        // Pseudo-transparency is hard in basic GDI, we just draw a solid rect in the middle
        RECT msgRect = { MARGIN + 20, HEADER_HEIGHT + 100, MARGIN + GRID_SIZE*CELL_SIZE - 12, HEADER_HEIGHT + 200 };
        FillRect(hdc, &msgRect, overlay);
        DeleteObject(overlay);
        SetTextColor(hdc, gameOver ? RGB(119,110,101) : RGB(255,255,255));
        DrawTextA(hdc, gameOver ? "Game Over! (Press R)" : "You Win! (Press R)", -1, &msgRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    SelectObject(hdc, oldFont);
    DeleteObject(hFontText);
}

int Move(int dx, int dy) {
    if (gameOver || win) return 0;
    int moved = 0;
    
    int startI = (dy == 1) ? GRID_SIZE - 1 : 0;
    int endI = (dy == 1) ? -1 : GRID_SIZE;
    int stepI = (dy == 1) ? -1 : 1;
    
    int startJ = (dx == 1) ? GRID_SIZE - 1 : 0;
    int endJ = (dx == 1) ? -1 : GRID_SIZE;
    int stepJ = (dx == 1) ? -1 : 1;
    
    int merged[GRID_SIZE][GRID_SIZE] = {0};

    for (int i = startI; i != endI; i += stepI) {
        for (int j = startJ; j != endJ; j += stepJ) {
            if (grid[i][j] != 0) {
                int ci = i;
                int cj = j;
                while (1) {
                    int ni = ci + dy;
                    int nj = cj + dx;
                    if (ni < 0 || ni >= GRID_SIZE || nj < 0 || nj >= GRID_SIZE) break;
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
                        if (score > bestScore) bestScore = score;
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
        MessageBeep(MB_OK); // simple sound effect
        AddRandomTile();
        if (CheckGameOver()) {
            gameOver = 1;
            SaveBest();
        }
    }
    return moved;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            mainHwnd = hwnd;
            InitGame();
            return 0;
        case WM_ERASEBKGND:
            return 1; // Prevent background clear to fix flickering
        case WM_KEYDOWN:
            if (wParam == 'R' || wParam == 'r') {
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
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
    
    int winWidth = MARGIN * 2 + GRID_SIZE * CELL_SIZE + 8 + 16;
    int winHeight = HEADER_HEIGHT + MARGIN + GRID_SIZE * CELL_SIZE + 8 + 39;

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
