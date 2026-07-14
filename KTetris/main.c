#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define CELL_SIZE 20
#define W 10
#define H 20
#define TIMER_ID 1

int grid[H][W];
int current_piece, current_rot, current_x, current_y;
int next_piece, next_rot;
int hold_piece = -1;
int hold_used = 0;
int high_score = 0;

void load_high_score() {
    HANDLE hFile = CreateFileA("ktetris_hiscore.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesRead;
        ReadFile(hFile, &high_score, sizeof(int), &bytesRead, NULL);
        CloseHandle(hFile);
    }
}

void save_high_score() {
    if (score > high_score) {
        high_score = score;
        HANDLE hFile = CreateFileA("ktetris_hiscore.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD bytesWritten;
            WriteFile(hFile, &high_score, sizeof(int), &bytesWritten, NULL);
            CloseHandle(hFile);
        }
    }
}
int game_over = 0;
int is_paused = 0;
int score = 0;
int lines = 0;
int level = 1;
int timer_speed = 500;

const unsigned short tetrominos[7][4] = {
    {0x0F00, 0x2222, 0x00F0, 0x4444}, // I
    {0x44C0, 0x8E00, 0x6220, 0x0710}, // J
    {0x2260, 0x0E80, 0xC440, 0x2E00}, // L
    {0xCC00, 0xCC00, 0xCC00, 0xCC00}, // O
    {0x06C0, 0x8C40, 0x6C00, 0x4620}, // S
    {0x0E40, 0x4C40, 0x4E00, 0x4640}, // T
    {0x0C60, 0x4C80, 0xC600, 0x2640}  // Z
};

const COLORREF colors[8] = {
    RGB(0,0,0),
    RGB(0,255,255), // I
    RGB(0,0,255),   // J
    RGB(255,165,0), // L
    RGB(255,255,0), // O
    RGB(0,255,0),   // S
    RGB(128,0,128), // T
    RGB(255,0,0)    // Z
};

unsigned int rng_state = 12345;
int random_int(int max) {
    rng_state = rng_state * 1103515245 + 12345;
    return ((rng_state >> 16) & 0x7FFF) % max;
}

int bag[7];
int bag_index = 7;
void fill_bag() {
    for (int i = 0; i < 7; i++) bag[i] = i;
    for (int i = 6; i > 0; i--) {
        int j = random_int(i + 1);
        int temp = bag[i];
        bag[i] = bag[j];
        bag[j] = temp;
    }
    bag_index = 0;
}

int get_next_pc() {
    if (bag_index >= 7) fill_bag();
    return bag[bag_index++];
}

int check_collision(int p, int rot, int px, int py) {
    unsigned short shape = tetrominos[p][rot];
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (shape & (1 << (15 - (y * 4 + x)))) {
                int nx = px + x;
                int ny = py + y;
                if (nx < 0 || nx >= W || ny >= H || (ny >= 0 && grid[ny][nx]))
                    return 1;
            }
        }
    }
    return 0;
}

void lock_piece() {
    unsigned short shape = tetrominos[current_piece][current_rot];
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (shape & (1 << (15 - (y * 4 + x)))) {
                if (current_y + y >= 0) {
                    grid[current_y + y][current_x + x] = current_piece + 1;
                }
            }
        }
    }
    // clear lines
    int lines_cleared = 0;
    for (int y = H - 1; y >= 0; y--) {
        int full = 1;
        for (int x = 0; x < W; x++) {
            if (!grid[y][x]) { full = 0; break; }
        }
        if (full) {
            lines_cleared++;
            for (int yy = y; yy > 0; yy--) {
                for (int x = 0; x < W; x++) grid[yy][x] = grid[yy-1][x];
            }
            for (int x = 0; x < W; x++) grid[0][x] = 0;
            y++; // check same line again
        }
    }
    if (lines_cleared > 0) {
        Beep(1000 + lines_cleared * 200, 100);
    } else {
        Beep(500, 50);
    }
    lines += lines_cleared;
    level = (lines / 10) + 1;
    score += lines_cleared * 100 * level;
    int new_speed = 500 - (level - 1) * 30;
    if (new_speed < 50) new_speed = 50;
    timer_speed = new_speed;
}

void spawn_piece() {
    current_piece = next_piece;
    current_rot = next_rot;
    current_x = W / 2 - 2;
    current_y = -2;
    next_piece = get_next_pc();
    next_rot = 0;
    hold_used = 0;
    if (check_collision(current_piece, current_rot, current_x, current_y + 1)) {
        game_over = 1;
        Beep(200, 500);
        save_high_score();
    }
}

void InitGame() {
    rng_state = GetTickCount();
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            grid[y][x] = 0;
    score = 0;
    lines = 0;
    level = 1;
    timer_speed = 500;
    bag_index = 7;
    game_over = 0;
    is_paused = 0;
    hold_piece = -1;
    hold_used = 0;
    next_piece = get_next_pc();
    next_rot = 0;
    spawn_piece();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            load_high_score();
            InitGame();
            SetTimer(hwnd, TIMER_ID, 500, NULL);
            break;
        case WM_TIMER:
            if (!game_over && !is_paused) {
                if (!check_collision(current_piece, current_rot, current_x, current_y + 1)) {
                    current_y++;
                } else {
                    lock_piece();
                    spawn_piece();
                    SetTimer(hwnd, TIMER_ID, timer_speed, NULL);
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_KEYDOWN:
            if (game_over && wParam == VK_RETURN) {
                InitGame();
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (!game_over) {
                if (wParam == 'P') {
                    is_paused = !is_paused;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }
                if (is_paused) break;

                if (wParam == VK_LEFT && !check_collision(current_piece, current_rot, current_x - 1, current_y)) current_x--;
                if (wParam == VK_RIGHT && !check_collision(current_piece, current_rot, current_x + 1, current_y)) current_x++;
                if (wParam == VK_DOWN && !check_collision(current_piece, current_rot, current_x, current_y + 1)) current_y++;
                if (wParam == VK_UP) {
                    int next_rot = (current_rot + 1) % 4;
                    if (!check_collision(current_piece, next_rot, current_x, current_y)) current_rot = next_rot;
                }
                if (wParam == 'C' || wParam == VK_SHIFT) {
                    if (!hold_used) {
                        if (hold_piece == -1) {
                            hold_piece = current_piece;
                            spawn_piece();
                        } else {
                            int temp = current_piece;
                            current_piece = hold_piece;
                            hold_piece = temp;
                            current_rot = 0;
                            current_x = W / 2 - 2;
                            current_y = -2;
                        }
                        hold_used = 1;
                    }
                }
                if (wParam == VK_SPACE) {
                    while (!check_collision(current_piece, current_rot, current_x, current_y + 1)) {
                        current_y++;
                    }
                    lock_piece();
                    spawn_piece();
                    SetTimer(hwnd, TIMER_ID, timer_speed, NULL);
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // double buffering (rudimentary)
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, W * CELL_SIZE + 120, H * CELL_SIZE);
            HBITMAP hOld = (HBITMAP)SelectObject(memDC, hbm);
            
            HBRUSH bg = CreateSolidBrush(RGB(30, 30, 30));
            RECT fullRc = {0, 0, W * CELL_SIZE + 120, H * CELL_SIZE};
            FillRect(memDC, &fullRc, bg);
            DeleteObject(bg);
            
            HBRUSH brushes[8];
            for (int i = 0; i < 8; i++) brushes[i] = CreateSolidBrush(colors[i]);
            
            // Draw grid
            for (int y = 0; y < H; y++) {
                for (int x = 0; x < W; x++) {
                    if (grid[y][x]) {
                        RECT r = { x * CELL_SIZE + 1, y * CELL_SIZE + 1, (x + 1) * CELL_SIZE - 1, (y + 1) * CELL_SIZE - 1 };
                        FillRect(memDC, &r, brushes[grid[y][x]]);
                    }
                }
            }
            
            // Draw current piece
            if (!game_over && !is_paused) {
                unsigned short shape = tetrominos[current_piece][current_rot];
                // Draw Ghost piece
                int ghost_y = current_y;
                while (!check_collision(current_piece, current_rot, current_x, ghost_y + 1)) {
                    ghost_y++;
                }
                for (int y = 0; y < 4; y++) {
                    for (int x = 0; x < 4; x++) {
                        if (shape & (1 << (15 - (y * 4 + x)))) {
                            if (ghost_y + y >= 0) {
                                RECT r = { (current_x + x) * CELL_SIZE + 1, (ghost_y + y) * CELL_SIZE + 1, (current_x + x + 1) * CELL_SIZE - 1, (ghost_y + y + 1) * CELL_SIZE - 1 };
                                FrameRect(memDC, &r, brushes[current_piece + 1]);
                            }
                        }
                    }
                }

                for (int y = 0; y < 4; y++) {
                    for (int x = 0; x < 4; x++) {
                        if (shape & (1 << (15 - (y * 4 + x)))) {
                            int py = current_y + y;
                            if (py >= 0) {
                                RECT r = { (current_x + x) * CELL_SIZE + 1, py * CELL_SIZE + 1, (current_x + x + 1) * CELL_SIZE - 1, (py + 1) * CELL_SIZE - 1 };
                                FillRect(memDC, &r, brushes[current_piece + 1]);
                            }
                        }
                    }
                }
            } else if (game_over) {
                SetTextColor(memDC, RGB(255, 0, 0));
                SetBkMode(memDC, TRANSPARENT);
                TextOutA(memDC, 20, H * CELL_SIZE / 2, "GAME OVER", 9);
            } else if (is_paused) {
                SetTextColor(memDC, RGB(255, 255, 0));
                SetBkMode(memDC, TRANSPARENT);
                TextOutA(memDC, 20, H * CELL_SIZE / 2, "PAUSED", 6);
            }
            
            // Draw UI panel
            SetTextColor(memDC, RGB(255, 255, 255));
            SetBkMode(memDC, TRANSPARENT);
            char score_str[32];
            wsprintfA(score_str, "SCORE: %d", score);
            TextOutA(memDC, W * CELL_SIZE + 10, 20, score_str, lstrlenA(score_str));
            
            char hi_str[32];
            wsprintfA(hi_str, "HI: %d", high_score);
            TextOutA(memDC, W * CELL_SIZE + 10, 40, hi_str, lstrlenA(hi_str));
            
            char level_str[32];
            wsprintfA(level_str, "LEVEL: %d", level);
            TextOutA(memDC, W * CELL_SIZE + 10, 60, level_str, lstrlenA(level_str));
            
            char lines_str[32];
            wsprintfA(lines_str, "LINES: %d", lines);
            TextOutA(memDC, W * CELL_SIZE + 10, 80, lines_str, lstrlenA(lines_str));
            
            TextOutA(memDC, W * CELL_SIZE + 10, 110, "NEXT:", 5);
            if (!game_over) {
                unsigned short next_shape = tetrominos[next_piece][next_rot];
                for (int y = 0; y < 4; y++) {
                    for (int x = 0; x < 4; x++) {
                        if (next_shape & (1 << (15 - (y * 4 + x)))) {
                            RECT r = { W * CELL_SIZE + 10 + x * CELL_SIZE + 1, 130 + y * CELL_SIZE + 1, W * CELL_SIZE + 10 + (x + 1) * CELL_SIZE - 1, 130 + (y + 1) * CELL_SIZE - 1 };
                            FillRect(memDC, &r, brushes[next_piece + 1]);
                        }
                    }
                }
            }
            
            TextOutA(memDC, W * CELL_SIZE + 10, 180, "HOLD:", 5);
            if (hold_piece != -1) {
                unsigned short hold_shape = tetrominos[hold_piece][0];
                for (int y = 0; y < 4; y++) {
                    for (int x = 0; x < 4; x++) {
                        if (hold_shape & (1 << (15 - (y * 4 + x)))) {
                            RECT r = { W * CELL_SIZE + 10 + x * CELL_SIZE + 1, 200 + y * CELL_SIZE + 1, W * CELL_SIZE + 10 + (x + 1) * CELL_SIZE - 1, 200 + (y + 1) * CELL_SIZE - 1 };
                            FillRect(memDC, &r, brushes[hold_piece + 1]);
                        }
                    }
                }
            }
            
            HPEN hPen = CreatePen(PS_SOLID, 2, RGB(100, 100, 100));
            HPEN hOldPen = (HPEN)SelectObject(memDC, hPen);
            MoveToEx(memDC, W * CELL_SIZE, 0, NULL);
            LineTo(memDC, W * CELL_SIZE, H * CELL_SIZE);
            SelectObject(memDC, hOldPen);
            DeleteObject(hPen);
            
            for (int i = 0; i < 8; i++) DeleteObject(brushes[i]);
            
            BitBlt(hdc, 0, 0, W * CELL_SIZE + 120, H * CELL_SIZE, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, hOld);
            DeleteObject(hbm);
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
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KTetrisApp";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    int winWidth = W * CELL_SIZE + 120 + 16;
    int winHeight = H * CELL_SIZE + 39;

    HWND hwnd = CreateWindowEx(0, "KTetrisApp", "KTetris", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, winWidth, winHeight, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
