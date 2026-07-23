#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#pragma function(memset)
void* memset(void* dest, int c, size_t count) {
    unsigned char* bytes = (unsigned char*)dest;
    while (count--) {
        *bytes++ = (unsigned char)c;
    }
    return dest;
}

#define CELL_SIZE 20
#define W 10
#define H 20
#define TIMER_ID 1

// Global state declarations
int grid[H][W];
int current_piece, current_rot, current_x, current_y;
int next_piece, next_rot;
int hold_piece = -1;
int hold_used = 0;
int high_score = 0;
int current_is_bomb = 0, next_is_bomb = 0, hold_is_bomb = 0;
int game_over = 0;
int is_paused = 0;
int start_screen = 1;
int win_screen = 0;
int game_mode = 0;
int campaign_level = 1;
int pieces_placed = 0;
int stat_lines[4] = {0, 0, 0, 0};
int score = 0;
int lines = 0;
int level = 1;
int combo = 0;
int timer_speed = 500;
int gravity_timer = 0;

void InitGame();

void load_high_score() {
    HANDLE hFile = CreateFileA("ktetris_hiscore.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesRead = 0;
        int loaded_score = 0;
        if (ReadFile(hFile, &loaded_score, sizeof(int), &bytesRead, NULL) && bytesRead == sizeof(int)) {
            if (loaded_score >= 0 && loaded_score <= 9999999) {
                high_score = loaded_score;
            }
        }
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

const unsigned short tetrominos[7][4] = {
    {0x0F00, 0x2222, 0x00F0, 0x4444}, // I
    {0x44C0, 0x8E00, 0x6220, 0x0710}, // J
    {0x2260, 0x0E80, 0xC440, 0x2E00}, // L
    {0xCC00, 0xCC00, 0xCC00, 0xCC00}, // O
    {0x06C0, 0x8C40, 0x6C00, 0x4620}, // S
    {0x0E40, 0x4C40, 0x4E00, 0x4640}, // T
    {0x0C60, 0x4C80, 0xC600, 0x2640}  // Z
};

const COLORREF colors[10] = {
    RGB(0,0,0),
    RGB(0,240,240),   // I - Cyan
    RGB(0,0,240),     // J - Blue
    RGB(240,160,0),   // L - Orange
    RGB(240,240,0),   // O - Yellow
    RGB(0,240,0),     // S - Green
    RGB(160,0,240),   // T - Purple
    RGB(240,0,0),     // Z - Red
    RGB(136,136,153), // Garbage - Grey
    RGB(255,51,51)    // Bomb - Bright Red
};

const COLORREF bevel_hi[10] = {
    RGB(0,0,0),
    RGB(153,255,255), // I
    RGB(102,102,255), // J
    RGB(255,204,102), // L
    RGB(255,255,153), // O
    RGB(102,255,102), // S
    RGB(217,153,255), // T
    RGB(255,102,102), // Z
    RGB(187,187,204), // Garbage
    RGB(255,153,153)  // Bomb
};

const COLORREF bevel_sh[10] = {
    RGB(0,0,0),
    RGB(0,136,136),   // I
    RGB(0,0,136),     // J
    RGB(153,85,0),    // L
    RGB(153,153,0),   // O
    RGB(0,136,0),     // S
    RGB(85,0,136),    // T
    RGB(136,0,0),     // Z
    RGB(68,68,85),    // Garbage
    RGB(136,0,0)      // Bomb
};

// Particle and animation structures
typedef struct {
    float x, y;
    float vx, vy;
    COLORREF color;
    int size;
    int life;
    int max_life;
} Particle;
#define MAX_PARTICLES 128
Particle particles[MAX_PARTICLES];
int num_particles = 0;

typedef struct {
    int y;
    int life;
    int max_life;
} LineFlash;
#define MAX_FLASHES 10
LineFlash line_flashes[MAX_FLASHES];
int num_flashes = 0;

typedef struct {
    float x, y;
    char text[32];
    COLORREF color;
    int life;
    int max_life;
} TextPopup;
#define MAX_POPUPS 8
TextPopup text_popups[MAX_POPUPS];
int num_popups = 0;

unsigned int rng_state = 12345;
int random_int(int max) {
    if (max <= 0) return 0;
    rng_state = rng_state * 1103515245 + 12345;
    return ((rng_state >> 16) & 0x7FFF) % max;
}

void AddParticle(float x, float y, float vx, float vy, COLORREF color, int size, int life) {
    if (num_particles < MAX_PARTICLES) {
        particles[num_particles].x = x;
        particles[num_particles].y = y;
        particles[num_particles].vx = vx;
        particles[num_particles].vy = vy;
        particles[num_particles].color = color;
        particles[num_particles].size = size;
        particles[num_particles].life = life;
        particles[num_particles].max_life = life;
        num_particles++;
    }
}

void AddPopup(float x, float y, const char* text, COLORREF color) {
    if (num_popups < MAX_POPUPS) {
        text_popups[num_popups].x = x;
        text_popups[num_popups].y = y;
        lstrcpynA(text_popups[num_popups].text, text, 32);
        text_popups[num_popups].color = color;
        text_popups[num_popups].life = 40;
        text_popups[num_popups].max_life = 40;
        num_popups++;
    }
}

void AddLineFlash(int yRow) {
    if (num_flashes < MAX_FLASHES) {
        line_flashes[num_flashes].y = yRow;
        line_flashes[num_flashes].life = 15;
        line_flashes[num_flashes].max_life = 15;
        num_flashes++;
    }
    for (int i = 0; i < 20; i++) {
        float px = (float)(random_int(W * CELL_SIZE));
        float py = (float)(yRow * CELL_SIZE + CELL_SIZE / 2);
        float vx = (float)((random_int(100) - 50) / 7.0f);
        float vy = (float)((random_int(100) - 50) / 7.0f);
        COLORREF pColors[4] = { RGB(255,255,255), RGB(255,255,0), RGB(0,255,255), RGB(255,100,255) };
        AddParticle(px, py, vx, vy, pColors[random_int(4)], random_int(3) + 2, 20);
    }
}

void SpawnDropParticles(int gridX, int startY, int endY, int colorIdx) {
    COLORREF pColor = colors[colorIdx];
    for (int y = (startY < 0 ? 0 : startY); y <= endY; y++) {
        for (int i = 0; i < 2; i++) {
            float px = (float)(gridX * CELL_SIZE + random_int(CELL_SIZE));
            float py = (float)(y * CELL_SIZE + random_int(CELL_SIZE));
            AddParticle(px, py, (float)((random_int(20) - 10) / 10.0f), (float)(-random_int(20) / 10.0f - 1.0f), pColor, 2, 12);
        }
    }
    for (int i = 0; i < 14; i++) {
        float px = (float)(gridX * CELL_SIZE + CELL_SIZE / 2 + random_int(CELL_SIZE * 2) - CELL_SIZE);
        float py = (float)(endY * CELL_SIZE + CELL_SIZE);
        float vx = (float)((random_int(60) - 30) / 10.0f);
        float vy = (float)((random_int(40) - 20) / 10.0f - 2.0f);
        AddParticle(px, py, vx, vy, pColor, 3, 18);
    }
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

int try_rotate(int dir) {
    int next_r = (current_rot + dir + 4) % 4;
    int offsets[6][2] = {{0,0}, {-1,0}, {1,0}, {0,-1}, {-2,0}, {2,0}};
    for (int i = 0; i < 6; i++) {
        int test_x = current_x + offsets[i][0];
        int test_y = current_y + offsets[i][1];
        if (!check_collision(current_piece, next_r, test_x, test_y)) {
            current_x = test_x;
            current_y = test_y;
            current_rot = next_r;
            return 1;
        }
    }
    return 0;
}

void lock_piece() {
    unsigned short shape = tetrominos[current_piece][current_rot];
    int block_val = current_is_bomb ? 9 : (current_piece + 1);
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (shape & (1 << (15 - (y * 4 + x)))) {
                if (current_y + y >= 0) {
                    grid[current_y + y][current_x + x] = block_val;
                }
            }
        }
    }
    // clear lines
    int lines_cleared = 0;
    int explode_centers[H][W] = {0};
    int has_bombs = 0;
    for (int y = H - 1; y >= 0; y--) {
        int full = 1;
        for (int x = 0; x < W; x++) {
            if (!grid[y][x]) { full = 0; break; }
        }
        if (full) {
            lines_cleared++;
            AddLineFlash(y);
            for (int x = 0; x < W; x++) {
                if (grid[y][x] == 9) {
                    explode_centers[y][x] = 1;
                    has_bombs = 1;
                }
            }
            for (int yy = y; yy > 0; yy--) {
                for (int x = 0; x < W; x++) {
                    grid[yy][x] = grid[yy-1][x];
                    explode_centers[yy][x] = explode_centers[yy-1][x];
                }
            }
            for (int x = 0; x < W; x++) { grid[0][x] = 0; explode_centers[0][x] = 0; }
            y++; // check same line again
        }
    }
    
    if (has_bombs) {
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                if (explode_centers[y][x]) {
                    for (int dy = -2; dy <= 2; dy++) {
                        for (int dx = -2; dx <= 2; dx++) {
                            int ny = y + dy;
                            int nx = x + dx;
                            if (ny >= 0 && ny < H && nx >= 0 && nx < W) {
                                if (grid[ny][nx] > 0) {
                                    COLORREF pCol = colors[grid[ny][nx]];
                                    for (int k = 0; k < 4; k++) AddParticle((float)((nx + 0.5)*CELL_SIZE), (float)((ny + 0.5)*CELL_SIZE), (float)(random_int(8) - 4), (float)(random_int(8) - 4), pCol, 3, 20);
                                }
                                grid[ny][nx] = 0;
                            }
                        }
                    }
                    score += 500;
                    AddPopup((float)(W * CELL_SIZE / 2 - 20), (float)(H * CELL_SIZE / 2), "BOMB BOOM! +500", RGB(255, 50, 50));
                    Beep(150, 100); Beep(100, 200);
                }
            }
        }
    }

    if (lines_cleared > 0) {
        if (lines_cleared >= 1 && lines_cleared <= 4) stat_lines[lines_cleared - 1]++;
        combo++;
        Beep(1000 + lines_cleared * 200 + combo * 100, 100);

        const char* popText[5] = {"", "SINGLE! +100", "DOUBLE! +300", "TRIPLE! +500", "TETRIS! +800"};
        COLORREF popCol[5] = {RGB(0,0,0), RGB(0,255,255), RGB(0,255,0), RGB(255,165,0), RGB(255,0,255)};
        if (lines_cleared <= 4) {
            AddPopup((float)(W * CELL_SIZE / 2 - 25), (float)((current_y + 2) * CELL_SIZE), popText[lines_cleared], popCol[lines_cleared]);
        }
        if (combo > 1) {
            char comboStr[32];
            wsprintfA(comboStr, "COMBO x%d!", combo);
            AddPopup((float)(W * CELL_SIZE / 2 - 15), (float)((current_y + 2) * CELL_SIZE + 16), comboStr, RGB(255, 68, 68));
        }
    } else {
        combo = 0;
        Beep(500, 50);
    }
    lines += lines_cleared;
    
    pieces_placed++;
    if (game_mode == 1 && campaign_level > 5 && campaign_level <= 10) {
        int threshold = 16 - campaign_level;
        if (threshold < 5) threshold = 5;
        if (pieces_placed % threshold == 0) {
            for (int y = 0; y < H - 1; y++) {
                for (int x = 0; x < W; x++) grid[y][x] = grid[y+1][x];
            }
            int hole = random_int(W);
            for (int x = 0; x < W; x++) {
                grid[H - 1][x] = (x == hole) ? 0 : 8;
            }
            Beep(300, 100);
        }
    } else if (game_mode == 1 && campaign_level > 10) {
        int threshold = 18 - campaign_level;
        if (threshold < 3) threshold = 3;
        if (pieces_placed % threshold == 0) {
            int cols = random_int(3) + 1;
            for(int k=0; k<cols; k++) {
                int c = random_int(W);
                for(int y = 0; y < H - 1; y++) grid[y][c] = grid[y+1][c];
                grid[H - 1][c] = 8;
            }
            Beep(250, 80); Beep(200, 80);
        }
    }

    if (game_mode == 1) {
        int goal = campaign_level * 10;
        level = campaign_level;
        score += lines_cleared * 100 * level * (combo > 0 ? combo : 1);
        if (lines >= goal) {
            campaign_level++;
            if (campaign_level > 15) {
                win_screen = 1;
                Beep(800, 150); Beep(1000, 150); Beep(1200, 300);
                return;
            } else {
                InitGame();
                return;
            }
        }
    } else {
        level = (lines / 10) + 1;
        score += lines_cleared * 100 * level * (combo > 0 ? combo : 1);
        int new_speed = 500 - (level - 1) * 30;
        if (new_speed < 50) new_speed = 50;
        timer_speed = new_speed;
    }
}

void spawn_piece() {
    current_piece = next_piece;
    current_rot = next_rot;
    current_x = W / 2 - 2;
    current_y = -2;
    current_is_bomb = next_is_bomb;
    next_piece = get_next_pc();
    next_rot = 0;
    next_is_bomb = (random_int(15) == 0) ? 1 : 0;
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
            
    pieces_placed = 0;
    num_particles = 0;
    num_flashes = 0;
    num_popups = 0;
    gravity_timer = 0;

    if (game_mode == 1) {
        int garbage = campaign_level * 2;
        if (garbage > 12) garbage = 12;
        for (int r = 0; r < garbage; r++) {
            int hole = random_int(W);
            for (int x = 0; x < W; x++) {
                grid[H - 1 - r][x] = (x == hole) ? 0 : 8;
            }
        }
        lines = 0;
        level = campaign_level;
        timer_speed = 500 - (level - 1) * 30;
    } else {
        lines = 0;
        level = 1;
        timer_speed = 500;
        for (int i=0; i<4; i++) stat_lines[i] = 0;
    }
    combo = 0;
    bag_index = 7;
    game_over = 0;
    is_paused = 0;
    win_screen = 0;
    hold_piece = -1;
    hold_used = 0;
    hold_is_bomb = 0;
    next_piece = get_next_pc();
    next_rot = 0;
    next_is_bomb = (random_int(15) == 0) ? 1 : 0;
    spawn_piece();
}

void DrawTetrisBlock(HDC hdc, int px, int py, int colorIdx, int size, int isGhost) {
    if (colorIdx <= 0 || colorIdx >= 10) return;
    if (isGhost) {
        HPEN pen = CreatePen(PS_SOLID, 1, colors[colorIdx]);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, px + 1, py + 1, px + size - 1, py + size - 1);
        MoveToEx(hdc, px + 4, py + size / 2, NULL); LineTo(hdc, px + size - 4, py + size / 2);
        MoveToEx(hdc, px + size / 2, py + 4, NULL); LineTo(hdc, px + size / 2, py + size - 4);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(pen);
        return;
    }

    int bSize = size / 6;
    if (bSize < 2) bSize = 2;

    // Main background
    HBRUSH bgBrush = CreateSolidBrush(colors[colorIdx]);
    RECT rMain = { px + 1, py + 1, px + size - 1, py + size - 1 };
    FillRect(hdc, &rMain, bgBrush);
    DeleteObject(bgBrush);

    // Top-Left bevel (Highlight)
    HBRUSH hiBrush = CreateSolidBrush(bevel_hi[colorIdx]);
    HPEN nullPen = CreatePen(PS_NULL, 0, 0);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, hiBrush);

    POINT ptHi[6] = {
        { px + 1, py + 1 },
        { px + size - 1, py + 1 },
        { px + size - 1 - bSize, py + 1 + bSize },
        { px + 1 + bSize, py + 1 + bSize },
        { px + 1 + bSize, py + size - 1 - bSize },
        { px + 1, py + size - 1 }
    };
    Polygon(hdc, ptHi, 6);

    // Bottom-Right bevel (Shadow)
    HBRUSH shBrush = CreateSolidBrush(bevel_sh[colorIdx]);
    SelectObject(hdc, shBrush);
    POINT ptSh[6] = {
        { px + size - 1, py + 1 },
        { px + size - 1, py + size - 1 },
        { px + 1, py + size - 1 },
        { px + 1 + bSize, py + size - 1 - bSize },
        { px + size - 1 - bSize, py + size - 1 - bSize },
        { px + size - 1 - bSize, py + 1 + bSize }
    };
    Polygon(hdc, ptSh, 6);

    // Unselect custom brushes & pens before deleting them
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(hiBrush);
    DeleteObject(shBrush);
    DeleteObject(nullPen);

    // Center shine box
    HBRUSH innerBrush = CreateSolidBrush(colors[colorIdx]);
    RECT rInner = { px + bSize + 1, py + bSize + 1, px + size - 1 - bSize, py + size - 1 - bSize };
    FillRect(hdc, &rInner, innerBrush);
    DeleteObject(innerBrush);

    // Corner flare dot
    HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
    RECT rFlare = { px + bSize + 1, py + bSize + 1, px + bSize + 4, py + bSize + 4 };
    FillRect(hdc, &rFlare, whiteBrush);
    DeleteObject(whiteBrush);

    // Special Bomb symbol
    if (colorIdx == 9) {
        HBRUSH yellowBrush = CreateSolidBrush(RGB(255, 255, 0));
        HBRUSH bOld = (HBRUSH)SelectObject(hdc, yellowBrush);
        HPEN pNull = CreatePen(PS_NULL, 0, 0);
        HPEN pOld = (HPEN)SelectObject(hdc, pNull);
        Ellipse(hdc, px + size / 4, py + size / 4, px + size * 3 / 4, py + size * 3 / 4);
        SelectObject(hdc, bOld);
        SelectObject(hdc, pOld);
        DeleteObject(yellowBrush);
        DeleteObject(pNull);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            load_high_score();
            InitGame();
            SetTimer(hwnd, TIMER_ID, 20, NULL);
            break;
        case WM_TIMER:
            if (!game_over && !is_paused && !start_screen && !win_screen) {
                gravity_timer += 20;
                if (gravity_timer >= timer_speed) {
                    gravity_timer = 0;
                    if (!check_collision(current_piece, current_rot, current_x, current_y + 1)) {
                        current_y++;
                    } else {
                        int old_level = campaign_level;
                        lock_piece();
                        if (!win_screen && (game_mode == 0 || campaign_level == old_level)) {
                            spawn_piece();
                        }
                    }
                }

                // Update particle physics
                for (int i = num_particles - 1; i >= 0; i--) {
                    particles[i].x += particles[i].vx;
                    particles[i].y += particles[i].vy;
                    particles[i].vy += 0.15f;
                    particles[i].life--;
                    if (particles[i].life <= 0) {
                        particles[i] = particles[num_particles - 1];
                        num_particles--;
                    }
                }

                // Update popups
                for (int i = num_popups - 1; i >= 0; i--) {
                    text_popups[i].y -= 0.5f;
                    text_popups[i].life--;
                    if (text_popups[i].life <= 0) {
                        text_popups[i] = text_popups[num_popups - 1];
                        num_popups--;
                    }
                }

                // Update line flashes
                for (int i = num_flashes - 1; i >= 0; i--) {
                    line_flashes[i].life--;
                    if (line_flashes[i].life <= 0) {
                        line_flashes[i] = line_flashes[num_flashes - 1];
                        num_flashes--;
                    }
                }

                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_KEYDOWN: {
            WPARAM k = wParam;
            if (start_screen) {
                if (k == '1') { game_mode = 0; start_screen = 0; score = 0; InitGame(); }
                if (k == '2') { game_mode = 1; start_screen = 0; campaign_level = 1; score = 0; InitGame(); }
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if ((win_screen || game_over) && (k == VK_RETURN || k == 'R' || k == 'r')) {
                start_screen = 1; win_screen = 0; game_over = 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (game_over || win_screen) return 0;

            if (k == 'P' || k == 'p') {
                is_paused = !is_paused;
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            if (k == 'R' || k == 'r') {
                start_screen = 1;
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            if (is_paused) break;

            if ((k == VK_LEFT || k == 'A' || k == 'a') && !check_collision(current_piece, current_rot, current_x - 1, current_y)) current_x--;
            if ((k == VK_RIGHT || k == 'D' || k == 'd') && !check_collision(current_piece, current_rot, current_x + 1, current_y)) current_x++;
            if ((k == VK_DOWN || k == 'S' || k == 's') && !check_collision(current_piece, current_rot, current_x, current_y + 1)) {
                current_y++; score += 1; gravity_timer = 0;
            }
            if (k == VK_UP || k == 'W' || k == 'w' || k == 'X' || k == 'x') {
                try_rotate(1);
            }
            if (k == 'Z' || k == 'z') {
                try_rotate(-1);
            }
            if (k == 'C' || k == 'c' || k == VK_SHIFT) {
                if (!hold_used) {
                    if (hold_piece == -1) {
                        hold_piece = current_piece;
                        hold_is_bomb = current_is_bomb;
                        spawn_piece();
                    } else {
                        int temp = current_piece;
                        int temp_bomb = current_is_bomb;
                        current_piece = hold_piece;
                        current_is_bomb = hold_is_bomb;
                        hold_piece = temp;
                        hold_is_bomb = temp_bomb;
                        current_rot = 0;
                        current_x = W / 2 - 2;
                        current_y = -2;
                    }
                    hold_used = 1;
                }
            }
            if (k == VK_SPACE) {
                int start_y = current_y;
                int drop_dist = 0;
                while (!check_collision(current_piece, current_rot, current_x, current_y + 1)) {
                    current_y++;
                    drop_dist++;
                }
                score += drop_dist * 2;
                SpawnDropParticles(current_x, start_y, current_y, current_is_bomb ? 9 : (current_piece + 1));
                int old_level = campaign_level;
                lock_piece();
                gravity_timer = 0;
                if (!win_screen && (game_mode == 0 || campaign_level == old_level)) {
                    spawn_piece();
                }
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, W * CELL_SIZE + 120, H * CELL_SIZE);
            HBITMAP hOld = (HBITMAP)SelectObject(memDC, hbm);
            
            HBRUSH bg = CreateSolidBrush(RGB(18, 18, 22));
            RECT fullRc = {0, 0, W * CELL_SIZE + 120, H * CELL_SIZE};
            FillRect(memDC, &fullRc, bg);
            DeleteObject(bg);
            
            // Draw grid lines
            HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(35, 35, 45));
            HPEN oldPen = (HPEN)SelectObject(memDC, gridPen);
            for (int x = 0; x <= W; x++) {
                MoveToEx(memDC, x * CELL_SIZE, 0, NULL); LineTo(memDC, x * CELL_SIZE, H * CELL_SIZE);
            }
            for (int y = 0; y <= H; y++) {
                MoveToEx(memDC, 0, y * CELL_SIZE, NULL); LineTo(memDC, W * CELL_SIZE, y * CELL_SIZE);
            }
            SelectObject(memDC, oldPen);
            DeleteObject(gridPen);

            // Draw locked grid blocks
            for (int y = 0; y < H; y++) {
                for (int x = 0; x < W; x++) {
                    if (grid[y][x]) {
                        DrawTetrisBlock(memDC, x * CELL_SIZE, y * CELL_SIZE, grid[y][x], CELL_SIZE, 0);
                    }
                }
            }
            
            // Draw current piece & Ghost piece
            if (!game_over && !is_paused && !start_screen && !win_screen) {
                unsigned short shape = tetrominos[current_piece][current_rot];
                int draw_val = current_is_bomb ? 9 : (current_piece + 1);
                
                // Ghost piece
                int ghost_y = current_y;
                while (!check_collision(current_piece, current_rot, current_x, ghost_y + 1)) {
                    ghost_y++;
                }
                for (int y = 0; y < 4; y++) {
                    for (int x = 0; x < 4; x++) {
                        if (shape & (1 << (15 - (y * 4 + x)))) {
                            if (ghost_y + y >= 0) {
                                DrawTetrisBlock(memDC, (current_x + x) * CELL_SIZE, (ghost_y + y) * CELL_SIZE, draw_val, CELL_SIZE, 1);
                            }
                        }
                    }
                }

                // Active piece
                for (int y = 0; y < 4; y++) {
                    for (int x = 0; x < 4; x++) {
                        if (shape & (1 << (15 - (y * 4 + x)))) {
                            int py = current_y + y;
                            if (py >= 0) {
                                DrawTetrisBlock(memDC, (current_x + x) * CELL_SIZE, py * CELL_SIZE, draw_val, CELL_SIZE, 0);
                            }
                        }
                    }
                }
            }

            // Draw Line Flashes
            for (int i = 0; i < num_flashes; i++) {
                HBRUSH flashBrush = CreateSolidBrush(RGB(255, 255, 255));
                RECT rFlash = { 0, line_flashes[i].y * CELL_SIZE, W * CELL_SIZE, (line_flashes[i].y + 1) * CELL_SIZE };
                FillRect(memDC, &rFlash, flashBrush);
                DeleteObject(flashBrush);
            }

            // Draw Particles
            for (int i = 0; i < num_particles; i++) {
                HBRUSH pBrush = CreateSolidBrush(particles[i].color);
                RECT rP = { (int)particles[i].x, (int)particles[i].y, (int)particles[i].x + particles[i].size, (int)particles[i].y + particles[i].size };
                FillRect(memDC, &rP, pBrush);
                DeleteObject(pBrush);
            }

            // Draw Text Popups
            SetBkMode(memDC, TRANSPARENT);
            for (int i = 0; i < num_popups; i++) {
                SetTextColor(memDC, text_popups[i].color);
                TextOutA(memDC, (int)text_popups[i].x, (int)text_popups[i].y, text_popups[i].text, lstrlenA(text_popups[i].text));
            }

            // UI Screens
            if (game_over) {
                SetTextColor(memDC, RGB(255, 51, 51));
                TextOutA(memDC, 45, H * CELL_SIZE / 2 - 10, "GAME OVER", 9);
                SetTextColor(memDC, RGB(180, 180, 180));
                TextOutA(memDC, 35, H * CELL_SIZE / 2 + 10, "PRESS ENTER / R", 15);
            } else if (win_screen) {
                SetTextColor(memDC, RGB(0, 255, 100));
                TextOutA(memDC, 50, H * CELL_SIZE / 2 - 10, "YOU WIN!", 8);
                SetTextColor(memDC, RGB(255, 255, 255));
                TextOutA(memDC, 30, H * CELL_SIZE / 2 + 10, "PRESS ENTER / R", 15);
            } else if (start_screen) {
                SetTextColor(memDC, RGB(0, 255, 255));
                TextOutA(memDC, 45, H * CELL_SIZE / 2 - 40, "K-TETRIS", 8);
                SetTextColor(memDC, RGB(255, 255, 255));
                TextOutA(memDC, 35, H * CELL_SIZE / 2 - 10, "1. Endless Mode", 15);
                TextOutA(memDC, 35, H * CELL_SIZE / 2 + 10, "2. Campaign Mode", 16);
            } else if (is_paused) {
                SetTextColor(memDC, RGB(255, 255, 0));
                TextOutA(memDC, 55, H * CELL_SIZE / 2, "PAUSED", 6);
            }
            
            // UI Panel
            SetTextColor(memDC, RGB(255, 255, 255));
            char score_str[32];
            wsprintfA(score_str, "SCORE: %d", score);
            TextOutA(memDC, W * CELL_SIZE + 10, 20, score_str, lstrlenA(score_str));
            
            char hi_str[32];
            wsprintfA(hi_str, "HI: %d", high_score);
            TextOutA(memDC, W * CELL_SIZE + 10, 40, hi_str, lstrlenA(hi_str));
            
            char level_str[32];
            if (game_mode == 1) wsprintfA(level_str, "STAGE: %d/15", campaign_level);
            else wsprintfA(level_str, "LEVEL: %d", level);
            TextOutA(memDC, W * CELL_SIZE + 10, 60, level_str, lstrlenA(level_str));
            
            char lines_str[32];
            if (game_mode == 1) wsprintfA(lines_str, "LINES: %d/%d", lines, campaign_level * 10);
            else wsprintfA(lines_str, "LINES: %d", lines);
            TextOutA(memDC, W * CELL_SIZE + 10, 80, lines_str, lstrlenA(lines_str));
            
            if (game_mode == 0 && !start_screen) {
                char stat1[32], stat2[32], stat3[32], stat4[32];
                wsprintfA(stat1, "SGL: %d", stat_lines[0]);
                wsprintfA(stat2, "DBL: %d", stat_lines[1]);
                wsprintfA(stat3, "TPL: %d", stat_lines[2]);
                wsprintfA(stat4, "TET: %d", stat_lines[3]);
                SetTextColor(memDC, RGB(136, 136, 255));
                TextOutA(memDC, W * CELL_SIZE + 10, 290, stat1, lstrlenA(stat1));
                TextOutA(memDC, W * CELL_SIZE + 10, 310, stat2, lstrlenA(stat2));
                TextOutA(memDC, W * CELL_SIZE + 10, 330, stat3, lstrlenA(stat3));
                TextOutA(memDC, W * CELL_SIZE + 10, 350, stat4, lstrlenA(stat4));
                SetTextColor(memDC, RGB(255, 255, 255));
            }
            
            if (combo > 1) {
                SetTextColor(memDC, RGB(255, 85, 85));
                char combo_str[32];
                wsprintfA(combo_str, "COMBO x%d!", combo);
                TextOutA(memDC, W * CELL_SIZE + 10, 100, combo_str, lstrlenA(combo_str));
                SetTextColor(memDC, RGB(255, 255, 255));
            }
            
            TextOutA(memDC, W * CELL_SIZE + 10, 120, "NEXT:", 5);
            if (!game_over) {
                unsigned short next_shape = tetrominos[next_piece][next_rot];
                int next_draw_val = next_is_bomb ? 9 : (next_piece + 1);
                for (int y = 0; y < 4; y++) {
                    for (int x = 0; x < 4; x++) {
                        if (next_shape & (1 << (15 - (y * 4 + x)))) {
                            DrawTetrisBlock(memDC, W * CELL_SIZE + 10 + x * 16, 140 + y * 16, next_draw_val, 16, 0);
                        }
                    }
                }
            }
            
            TextOutA(memDC, W * CELL_SIZE + 10, 220, "HOLD:", 5);
            if (hold_piece != -1) {
                unsigned short hold_shape = tetrominos[hold_piece][0];
                int hold_draw_val = hold_is_bomb ? 9 : (hold_piece + 1);
                for (int y = 0; y < 4; y++) {
                    for (int x = 0; x < 4; x++) {
                        if (hold_shape & (1 << (15 - (y * 4 + x)))) {
                            DrawTetrisBlock(memDC, W * CELL_SIZE + 10 + x * 16, 240 + y * 16, hold_draw_val, 16, 0);
                        }
                    }
                }
            }
            
            HPEN hPen = CreatePen(PS_SOLID, 2, RGB(42, 42, 53));
            HPEN hOldPen = (HPEN)SelectObject(memDC, hPen);
            MoveToEx(memDC, W * CELL_SIZE, 0, NULL);
            LineTo(memDC, W * CELL_SIZE, H * CELL_SIZE);
            SelectObject(memDC, hOldPen);
            DeleteObject(hPen);
            
            BitBlt(hdc, 0, 0, W * CELL_SIZE + 120, H * CELL_SIZE, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, hOld);
            DeleteObject(hbm);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            KillTimer(hwnd, TIMER_ID);
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
