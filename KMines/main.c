#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

#define COLS 10
#define ROWS 10
#define MINES 10
#define CELL_SIZE 20

// Bitmasks for grid
#define CELL_MINE     0x01
#define CELL_REVEALED 0x02
#define CELL_FLAGGED  0x04

int grid[ROWS][COLS];
int gameOver = 0;
int initialized = 0;
static unsigned int seed = 0;

int my_rand() {
    seed = seed * 214013 + 2531011;
    return (seed >> 16) & 0x7FFF;
}

void InitGame(int firstClickX, int firstClickY) {
    memset(grid, 0, sizeof(grid));
    gameOver = 0;
    seed = GetTickCount();

    int placed = 0;
    while (placed < MINES) {
        int r = my_rand() % ROWS;
        int c = my_rand() % COLS;
        if ((grid[r][c] & CELL_MINE) == 0) {
            // Avoid placing mine on first click
            if (r != firstClickY || c != firstClickX) {
                grid[r][c] |= CELL_MINE;
                placed++;
            }
        }
    }
    initialized = 1;
}

int CountMines(int r, int c) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int nr = r + i, nc = c + j;
            if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS) {
                if (grid[nr][nc] & CELL_MINE) count++;
            }
        }
    }
    return count;
}

void Reveal(int r, int c) {
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return;
    if (grid[r][c] & (CELL_REVEALED | CELL_FLAGGED)) return;
    
    grid[r][c] |= CELL_REVEALED;
    
    if (grid[r][c] & CELL_MINE) {
        gameOver = 1;
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

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int x = c * CELL_SIZE;
            int y = r * CELL_SIZE;
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
    for(int r=0; r<ROWS; r++) {
        for(int c=0; c<COLS; c++) {
            if (grid[r][c] & CELL_REVEALED) revealed++;
        }
    }
    return (revealed == (ROWS * COLS - MINES));
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
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
                InvalidateRect(hwnd, NULL, TRUE);
                return 0;
            }
            int x = LOWORD(lParam) / CELL_SIZE;
            int y = HIWORD(lParam) / CELL_SIZE;
            if (x >= 0 && x < COLS && y >= 0 && y < ROWS) {
                if (!initialized) InitGame(x, y);
                if (!(grid[y][x] & CELL_FLAGGED)) {
                    Reveal(y, x);
                    InvalidateRect(hwnd, NULL, FALSE);
                    if (gameOver) {
                        // Reveal all mines
                        for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++) if(grid[r][c]&CELL_MINE) grid[r][c]|=CELL_REVEALED;
                        InvalidateRect(hwnd, NULL, FALSE);
                        MessageBoxA(hwnd, "Boom! Click to restart.", "Game Over", MB_OK);
                    } else if (CheckWin()) {
                        gameOver = 1;
                        MessageBoxA(hwnd, "You Win! Click to restart.", "Congratulations", MB_OK);
                    }
                }
            }
            break;
        }
        case WM_RBUTTONDOWN: {
            if (gameOver) return 0;
            int x = LOWORD(lParam) / CELL_SIZE;
            int y = HIWORD(lParam) / CELL_SIZE;
            if (x >= 0 && x < COLS && y >= 0 && y < ROWS) {
                if (!(grid[y][x] & CELL_REVEALED)) {
                    grid[y][x] ^= CELL_FLAGGED;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            break;
        }
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
    // Client area should be exactly 200x200
    RECT rc = {0, 0, COLS * CELL_SIZE, ROWS * CELL_SIZE};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, FALSE);
    
    HWND hwnd = CreateWindowExA(0, "KMinesClass", "KMines", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
    
    // Adjust window size to ensure the client area is exactly COLS * CELL_SIZE by ROWS * CELL_SIZE
    {
        RECT rcClient;
        RECT rcWindow;
        GetClientRect(hwnd, &rcClient);
        GetWindowRect(hwnd, &rcWindow);
        SetWindowPos(hwnd, NULL, 0, 0,
            (rcWindow.right - rcWindow.left) + (COLS * CELL_SIZE - (rcClient.right - rcClient.left)),
            (rcWindow.bottom - rcWindow.top) + (ROWS * CELL_SIZE - (rcClient.bottom - rcClient.top)),
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
