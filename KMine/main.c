#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 200
#define H 200
#define CELL 20

int grid[10][10];
int state[10][10]; // 0=hidden, 1=revealed, 2=flagged
int gameOver = 0;

int randSeed = 42;
int MyRand() {
    randSeed = randSeed * 1103515245 + 12345;
    return (unsigned int)(randSeed / 65536) % 32768;
}

void InitGame() {
    randSeed = GetTickCount();
    gameOver = 0;
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            grid[y][x] = 0;
            state[y][x] = 0;
        }
    }
    
    int mines = 0;
    while (mines < 15) {
        int x = MyRand() % 10;
        int y = MyRand() % 10;
        if (grid[y][x] != 9) {
            grid[y][x] = 9;
            mines++;
        }
    }
    
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            if (grid[y][x] == 9) continue;
            int count = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < 10 && ny >= 0 && ny < 10 && grid[ny][nx] == 9) {
                        count++;
                    }
                }
            }
            grid[y][x] = count;
        }
    }
}

void Reveal(int x, int y) {
    if (x < 0 || x >= 10 || y < 0 || y >= 10 || state[y][x] != 0) return;
    state[y][x] = 1;
    if (grid[y][x] == 9) {
        gameOver = 1;
        // reveal all
        for (int i=0; i<10; i++)
            for (int j=0; j<10; j++)
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            InitGame();
            break;
        case WM_LBUTTONDOWN: {
            if (gameOver) {
                InitGame();
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            int x = LOWORD(lParam) / CELL;
            int y = HIWORD(lParam) / CELL;
            Reveal(x, y);
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_RBUTTONDOWN: {
            if (gameOver) break;
            int x = LOWORD(lParam) / CELL;
            int y = HIWORD(lParam) / CELL;
            if (x >= 0 && x < 10 && y >= 0 && y < 10 && state[y][x] != 1) {
                state[y][x] = (state[y][x] == 0) ? 2 : 0;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, W, H);
            SelectObject(memDC, memBM);
            
            HBRUSH bg = CreateSolidBrush(RGB(192, 192, 192));
            RECT full = {0, 0, W, H};
            FillRect(memDC, &full, bg);
            DeleteObject(bg);
            
            HFONT hFont = CreateFontA(14, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            HGDIOBJ oldFont = SelectObject(memDC, hFont);
            SetBkMode(memDC, TRANSPARENT);
            
            for (int y = 0; y < 10; y++) {
                for (int x = 0; x < 10; x++) {
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
            
            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
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

    RECT r = {0, 0, W, H};
    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);
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
