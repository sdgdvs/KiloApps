#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 200
#define H 200
#define CELL 20

#define MAX_ROWS 16
#define MAX_COLS 30

int grid[MAX_ROWS][MAX_COLS];
int state[MAX_ROWS][MAX_COLS]; // 0=hidden, 1=revealed, 2=flagged
int gameOver = 0;
int flagsPlaced = 0;
int timeElapsed = 0;
int firstClick = 1;

int cols = 10;
int rows = 10;
int totalMines = 15;

void UpdateTitle(HWND hwnd) {
    if (!hwnd) return;
    if (gameOver == 2) {
        char buf[64];
        wsprintfA(buf, "KMine - YOU WIN! Time: %ds", timeElapsed);
        SetWindowTextA(hwnd, buf);
    } else if (gameOver == 1) {
        char buf[64];
        wsprintfA(buf, "KMine - GAME OVER! Time: %ds", timeElapsed);
        SetWindowTextA(hwnd, buf);
    } else {
        char buf[64];
        wsprintfA(buf, "KMine - Mines: %d | Time: %ds", totalMines - flagsPlaced, timeElapsed);
        SetWindowTextA(hwnd, buf);
    }
}

#define IDM_RESTART 1000
#define IDM_BEGINNER 1001
#define IDM_INTERMEDIATE 1002
#define IDM_EXPERT 1003
#define IDM_HINT 1004
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

void InitGame(HWND hwnd) {
    randSeed = GetTickCount();
    gameOver = 0;
    flagsPlaced = 0;
    timeElapsed = 0;
    firstClick = 1;
    if (hwnd) KillTimer(hwnd, 1);
    UpdateTitle(hwnd);
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            grid[y][x] = 0;
            state[y][x] = 0;
        }
    }
}

void Reveal(int x, int y) {
    if (x < 0 || x >= cols || y < 0 || y >= rows || state[y][x] != 0) return;
    state[y][x] = 1;
    if (grid[y][x] == 9) {
        gameOver = 1;
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

void Chord(int x, int y) {
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
        KillTimer(hwnd, 1);
        UpdateTitle(hwnd);
    }
}

void GiveHint(HWND hwnd) {
    if (gameOver != 0) return;
    if (firstClick) {
        int safeX = cols / 2;
        int safeY = rows / 2;
        firstClick = 0;
        GenerateMines(safeX, safeY);
        SetTimer(hwnd, 1, 1000, NULL);
        Reveal(safeX, safeY);
        CheckWin(hwnd);
        InvalidateRect(hwnd, NULL, FALSE);
        return;
    }
    int count = 0;
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            if (state[y][x] == 0 && grid[y][x] != 9) {
                count++;
            }
        }
    }
    if (count > 0) {
        int target = MyRand() % count;
        int current = 0;
        for (int y = 0; y < rows; y++) {
            for (int x = 0; x < cols; x++) {
                if (state[y][x] == 0 && grid[y][x] != 9) {
                    if (current == target) {
                        Reveal(x, y);
                        if (gameOver == 1) {
                            KillTimer(hwnd, 1);
                            UpdateTitle(hwnd);
                        } else {
                            CheckWin(hwnd);
                        }
                        InvalidateRect(hwnd, NULL, FALSE);
                        return;
                    }
                    current++;
                }
            }
        }
    }
}

void SetDifficulty(HWND hwnd, int c, int r, int m) {
    cols = c;
    rows = r;
    totalMines = m;
    RECT rc = {0, 0, cols * CELL, rows * CELL};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, TRUE);
    SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);
    InitGame(hwnd);
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            hMenu = CreateMenu();
            hSubMenu = CreatePopupMenu();
            AppendMenuA(hSubMenu, MF_STRING, IDM_RESTART, "New Game");
            AppendMenuA(hSubMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hSubMenu, MF_STRING, IDM_BEGINNER, "Beginner");
            AppendMenuA(hSubMenu, MF_STRING, IDM_INTERMEDIATE, "Intermediate");
            AppendMenuA(hSubMenu, MF_STRING, IDM_EXPERT, "Expert");
            AppendMenuA(hSubMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hSubMenu, MF_STRING, IDM_HINT, "Hint");
            AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, "Game");
            SetMenu(hwnd, hMenu);
            SetDifficulty(hwnd, 10, 10, 15);
            break;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDM_RESTART) {
                InitGame(hwnd);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (LOWORD(wParam) == IDM_BEGINNER) {
                SetDifficulty(hwnd, 10, 10, 15);
            } else if (LOWORD(wParam) == IDM_INTERMEDIATE) {
                SetDifficulty(hwnd, 16, 16, 40);
            } else if (LOWORD(wParam) == IDM_EXPERT) {
                SetDifficulty(hwnd, 30, 16, 99);
            } else if (LOWORD(wParam) == IDM_HINT) {
                GiveHint(hwnd);
            }
            break;
        case WM_TIMER:
            if (!gameOver) {
                timeElapsed++;
                UpdateTitle(hwnd);
            }
            break;
        case WM_LBUTTONDOWN: {
            if (gameOver) {
                InitGame(hwnd);
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            int x = LOWORD(lParam) / CELL;
            int y = HIWORD(lParam) / CELL;
            if (x < 0 || x >= cols || y < 0 || y >= rows) break;
            
            if (firstClick) {
                firstClick = 0;
                GenerateMines(x, y);
                SetTimer(hwnd, 1, 1000, NULL);
            }
            if (state[y][x] == 1) {
                Chord(x, y);
            } else if (state[y][x] == 0) {
                Reveal(x, y);
            }
            if (gameOver == 1) {
                KillTimer(hwnd, 1);
                UpdateTitle(hwnd);
            } else {
                CheckWin(hwnd);
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_RBUTTONDOWN: {
            if (gameOver) break;
            int x = LOWORD(lParam) / CELL;
            int y = HIWORD(lParam) / CELL;
            if (x >= 0 && x < cols && y >= 0 && y < rows && state[y][x] != 1) {
                if (state[y][x] == 0) {
                    state[y][x] = 2;
                    flagsPlaced++;
                } else {
                    state[y][x] = 0;
                    flagsPlaced--;
                }
                UpdateTitle(hwnd);
                InvalidateRect(hwnd, NULL, FALSE);
            }
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
            
            HFONT hFont = CreateFontA(14, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            HGDIOBJ oldFont = SelectObject(memDC, hFont);
            SetBkMode(memDC, TRANSPARENT);
            
            for (int y = 0; y < rows; y++) {
                for (int x = 0; x < cols; x++) {
                    RECT r = { x * CELL, y * CELL, (x + 1) * CELL, (y + 1) * CELL };
                    if (state[y][x] == 0) {
                        DrawEdge(memDC, &r, EDGE_RAISED, BF_RECT | BF_MIDDLE);
                    } else if (state[y][x] == 2) {
                        DrawEdge(memDC, &r, EDGE_RAISED, BF_RECT | BF_MIDDLE);
                        SetTextColor(memDC, RGB(255, 0, 0));
                        DrawTextA(memDC, "F", -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    } else if (state[y][x] == 1) {
                        DrawEdge(memDC, &r, EDGE_SUNKEN, BF_RECT | BF_MIDDLE);
                        if (grid[y][x] == 9) {
                            SetTextColor(memDC, RGB(0, 0, 0));
                            DrawTextA(memDC, "*", -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
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
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KMineApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    RECT r = {0, 0, 10 * CELL, 10 * CELL};
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
