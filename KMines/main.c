#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

#define MAX_ROWS 30
#define MAX_COLS 30
int rows = 10;
int cols = 10;
int mines = 10;
#define CELL_SIZE 20
#define HEADER_HEIGHT 60

// Bitmasks for grid
#define CELL_MINE     0x01
#define CELL_REVEALED 0x02
#define CELL_FLAGGED  0x04

int grid[MAX_ROWS][MAX_COLS];
int gameOver = 0;
int initialized = 0;
int timeElapsed = 0;
int bestTimes[3] = {999, 999, 999};
int currentDiff = 0; // 0=Easy, 1=Medium, 2=Hard
int flagsPlaced = 0;
HWND mainHwnd = NULL;
static unsigned int seed = 0;

int my_rand() {
    seed = seed * 214013 + 2531011;
    return (seed >> 16) & 0x7FFF;
}

void LoadBest() {
    HANDLE hFile = CreateFileA("kmines_scores.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[64] = {0};
        DWORD bytesRead;
        if (ReadFile(hFile, buf, sizeof(buf)-1, &bytesRead, NULL)) {
            sscanf(buf, "%d %d %d", &bestTimes[0], &bestTimes[1], &bestTimes[2]);
        }
        CloseHandle(hFile);
    }
}

void SaveBest() {
    HANDLE hFile = CreateFileA("kmines_scores.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[64];
        wsprintfA(buf, "%d %d %d", bestTimes[0], bestTimes[1], bestTimes[2]);
        DWORD bytesWritten;
        int len = 0; while(buf[len]) len++;
        WriteFile(hFile, buf, len, &bytesWritten, NULL);
        CloseHandle(hFile);
    }
}

void InitGame(int firstClickX, int firstClickY) {
    memset(grid, 0, sizeof(grid));
    gameOver = 0;
    timeElapsed = 0;
    flagsPlaced = 0;
    seed = GetTickCount();

    int placed = 0;
    while (placed < mines) {
        int r = my_rand() % rows;
        int c = my_rand() % cols;
        if ((grid[r][c] & CELL_MINE) == 0) {
            // Avoid placing mine on first click 3x3 area
            if (r < firstClickY - 1 || r > firstClickY + 1 || c < firstClickX - 1 || c > firstClickX + 1) {
                grid[r][c] |= CELL_MINE;
                placed++;
            }
        }
    }
    initialized = 1;
    SetTimer(mainHwnd, 1, 1000, NULL);
}

int CountMines(int r, int c) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int nr = r + i, nc = c + j;
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                if (grid[nr][nc] & CELL_MINE) count++;
            }
        }
    }
    return count;
}

void Reveal(int r, int c) {
    if (r < 0 || r >= rows || c < 0 || c >= cols) return;
    if (grid[r][c] & (CELL_REVEALED | CELL_FLAGGED)) return;
    
    grid[r][c] |= CELL_REVEALED;
    
    if (grid[r][c] & CELL_MINE) {
        gameOver = 1;
        KillTimer(mainHwnd, 1);
        Beep(200, 500);
        return;
    }
    
    if (CountMines(r, c) == 0) {
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                Reveal(r + i, c + j);
            }
        }
    }
}

void DrawBoard(HWND hwnd, HDC hdc) {
    HFONT hFont = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);

    RECT rcHeader = {0, 0, cols * CELL_SIZE, HEADER_HEIGHT};
    HBRUSH hbrHeader = CreateSolidBrush(RGB(50, 50, 50));
    FillRect(hdc, &rcHeader, hbrHeader);
    DeleteObject(hbrHeader);

    char szHeader[128];
    wsprintfA(szHeader, "T:%03d M:%02d B:%03d", timeElapsed, mines - flagsPlaced, bestTimes[currentDiff]);
    SetTextColor(hdc, RGB(255, 255, 255));
    RECT rcTop = {0, 0, cols * CELL_SIZE, HEADER_HEIGHT / 2};
    RECT rcBot = {0, HEADER_HEIGHT / 2, cols * CELL_SIZE, HEADER_HEIGHT};
    DrawTextA(hdc, szHeader, -1, &rcTop, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DrawTextA(hdc, "Press: 1-Easy 2-Med 3-Hard", -1, &rcBot, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int x = c * CELL_SIZE;
            int y = r * CELL_SIZE + HEADER_HEIGHT;
            RECT rc = { x, y, x + CELL_SIZE, y + CELL_SIZE };
            
            if (grid[r][c] & CELL_REVEALED) {
                if (grid[r][c] & CELL_MINE) {
                    HBRUSH hbr = CreateSolidBrush(RGB(255, 0, 0));
                    FillRect(hdc, &rc, hbr);
                    DeleteObject(hbr);
                } else {
                    HBRUSH hbr = CreateSolidBrush(RGB(200, 200, 200));
                    FillRect(hdc, &rc, hbr);
                    DeleteObject(hbr);
                    
                    int m = CountMines(r, c);
                    if (m > 0) {
                        char sz[2] = { m + '0', 0 };
                        if(m==1) SetTextColor(hdc, RGB(0,0,255));
                        else if(m==2) SetTextColor(hdc, RGB(0,128,0));
                        else SetTextColor(hdc, RGB(255,0,0));
                        DrawTextA(hdc, sz, 1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    }
                }
            } else {
                HBRUSH hbr = CreateSolidBrush(RGB(150, 150, 150));
                FillRect(hdc, &rc, hbr);
                DeleteObject(hbr);
                
                // Draw 3D effect borders for unrevealed cells
                HPEN hWhite = CreatePen(PS_SOLID, 1, RGB(255,255,255));
                HPEN hGray = CreatePen(PS_SOLID, 1, RGB(100,100,100));
                
                SelectObject(hdc, hWhite);
                MoveToEx(hdc, x, y+CELL_SIZE-1, NULL);
                LineTo(hdc, x, y);
                LineTo(hdc, x+CELL_SIZE-1, y);
                
                SelectObject(hdc, hGray);
                LineTo(hdc, x+CELL_SIZE-1, y+CELL_SIZE-1);
                LineTo(hdc, x-1, y+CELL_SIZE-1);
                
                DeleteObject(hWhite);
                DeleteObject(hGray);

                if (grid[r][c] & CELL_FLAGGED) {
                    SetTextColor(hdc, RGB(255, 0, 0));
                    DrawTextA(hdc, "F", 1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
            }
            
            // Draw grid outline
            HPEN hBlack = CreatePen(PS_SOLID, 1, RGB(0,0,0));
            SelectObject(hdc, hBlack);
            MoveToEx(hdc, x, y, NULL);
            LineTo(hdc, x+CELL_SIZE, y);
            LineTo(hdc, x+CELL_SIZE, y+CELL_SIZE);
            LineTo(hdc, x, y+CELL_SIZE);
            LineTo(hdc, x, y);
            DeleteObject(hBlack);
        }
    }
    DeleteObject(hFont);
}

int CheckWin() {
    int revealed = 0;
    for(int r=0; r<rows; r++) {
        for(int c=0; c<cols; c++) {
            if (grid[r][c] & CELL_REVEALED) revealed++;
        }
    }
    return (revealed == (rows * cols - mines));
}

void HandleReveal(HWND hwnd, int x, int y) {
    if (gameOver || !initialized) return;
    if (grid[y][x] & CELL_FLAGGED) return;

    if (grid[y][x] & CELL_REVEALED) {
        int flagged = 0;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                int nr = y + i, nc = x + j;
                if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                    if (grid[nr][nc] & CELL_FLAGGED) flagged++;
                }
            }
        }
        if (flagged == CountMines(y, x)) {
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    int nr = y + i, nc = x + j;
                    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                        if (!(grid[nr][nc] & CELL_FLAGGED) && !(grid[nr][nc] & CELL_REVEALED)) {
                            Reveal(nr, nc);
                        }
                    }
                }
            }
        } else {
            return;
        }
    } else {
        Reveal(y, x);
    }

    if (!gameOver) Beep(1000, 20);
    InvalidateRect(hwnd, NULL, FALSE);
    
    if (gameOver) {
        for(int r=0;r<rows;r++) for(int c=0;c<cols;c++) if(grid[r][c]&CELL_MINE) grid[r][c]|=CELL_REVEALED;
        InvalidateRect(hwnd, NULL, FALSE);
        MessageBoxA(hwnd, "Boom! Click to restart.", "Game Over", MB_OK);
    } else if (CheckWin()) {
        gameOver = 1;
        KillTimer(hwnd, 1);
        Beep(1500, 300);
        if (timeElapsed < bestTimes[currentDiff]) {
            bestTimes[currentDiff] = timeElapsed;
            SaveBest();
        }
        MessageBoxA(hwnd, "You Win! Click to restart.", "Congratulations", MB_OK);
    }
}

void ResizeWindow(HWND hwnd) {
    RECT rcClient, rcWindow;
    GetClientRect(hwnd, &rcClient);
    GetWindowRect(hwnd, &rcWindow);
    SetWindowPos(hwnd, NULL, 0, 0,
        (rcWindow.right - rcWindow.left) + (cols * CELL_SIZE - (rcClient.right - rcClient.left)),
        (rcWindow.bottom - rcWindow.top) + (rows * CELL_SIZE + HEADER_HEIGHT - (rcClient.bottom - rcClient.top)),
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            mainHwnd = hwnd;
            LoadBest();
            break;
        case WM_TIMER:
            if (!gameOver && initialized) {
                timeElapsed++;
                if (timeElapsed > 999) timeElapsed = 999;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            DrawBoard(hwnd, hdc);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_LBUTTONDOWN: {
            if (gameOver) {
                initialized = 0;
                memset(grid, 0, sizeof(grid));
                gameOver = 0;
                timeElapsed = 0;
                flagsPlaced = 0;
                InvalidateRect(hwnd, NULL, TRUE);
                return 0;
            }
            int x = LOWORD(lParam) / CELL_SIZE;
            int y = (HIWORD(lParam) - HEADER_HEIGHT) / CELL_SIZE;
            if (x >= 0 && x < cols && y >= 0 && y < rows && HIWORD(lParam) >= HEADER_HEIGHT) {
                if (!initialized) InitGame(x, y);
                HandleReveal(hwnd, x, y);
            }
            break;
        }
        case WM_RBUTTONDOWN: {
            if (gameOver || !initialized) return 0;
            int x = LOWORD(lParam) / CELL_SIZE;
            int y = (HIWORD(lParam) - HEADER_HEIGHT) / CELL_SIZE;
            if (x >= 0 && x < cols && y >= 0 && y < rows && HIWORD(lParam) >= HEADER_HEIGHT) {
                if (!(grid[y][x] & CELL_REVEALED)) {
                    grid[y][x] ^= CELL_FLAGGED;
                    if (grid[y][x] & CELL_FLAGGED) flagsPlaced++;
                    else flagsPlaced--;
                    Beep(800, 20);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            break;
        }
        case WM_KEYDOWN:
            if (wParam == '1') { rows=10; cols=10; mines=10; currentDiff=0; initialized=0; gameOver=0; timeElapsed=0; flagsPlaced=0; memset(grid,0,sizeof(grid)); ResizeWindow(hwnd); }
            if (wParam == '2') { rows=16; cols=16; mines=40; currentDiff=1; initialized=0; gameOver=0; timeElapsed=0; flagsPlaced=0; memset(grid,0,sizeof(grid)); ResizeWindow(hwnd); }
            if (wParam == '3') { rows=16; cols=30; mines=99; currentDiff=2; initialized=0; gameOver=0; timeElapsed=0; flagsPlaced=0; memset(grid,0,sizeof(grid)); ResizeWindow(hwnd); }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KMinesClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);
    // Client area should be exactly 200x260 for easy by default
    RECT rc = {0, 0, cols * CELL_SIZE, rows * CELL_SIZE + HEADER_HEIGHT};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, FALSE);
    
    HWND hwnd = CreateWindowExA(0, "KMinesClass", "KMines", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
    
    // Adjust window size to ensure the client area is exactly cols * CELL_SIZE by rows * CELL_SIZE + HEADER_HEIGHT
    ResizeWindow(hwnd);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
