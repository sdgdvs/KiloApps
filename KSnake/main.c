#include <windows.h>

#define CELL_SIZE 15
#define GRID_WIDTH 20
#define GRID_HEIGHT 20
#define TIMER_ID 1

struct Point { int x; int y; };

struct HighScoreEntry {
    char name[4];
    int score;
    char mode[12];
    char date[12];
};

// Game State Enum: 0=Menu, 1=Playing, 2=GameOver, 3=Paused, 4=Victory, 5=Leaderboard
int game_state = 0; 
int game_mode = 0; // 0=Classic, 1=Maze, 2=Speed Ramp, 3=Wrap, 4=Campaign
const char* mode_names[] = { "Classic", "Maze", "Ramp", "Wrap", "Campaign" };

struct Point snake[400];
int snake_len = 3;
int dir_x = 1, dir_y = 0;
int last_dir_x = 1, last_dir_y = 0;

struct Point food = { 10, 10 };
struct Point special_food = { -1, -1 }; // Shrink
int special_food_timer = 0;
struct Point ghost_food = { -1, -1 }; // Ghost
int ghost_food_timer = 0, ghost_active_timer = 0;
struct Point ice_food = { -1, -1 }; // Ice Slowdown
int ice_food_timer = 0, ice_active_timer = 0;
struct Point mult_food = { -1, -1 }; // 3x Bonus Multiplier
int mult_food_timer = 0, mult_active_timer = 0;

struct Point obstacles[50];
int num_obstacles = 0;
struct Point spiders[10];
int num_spiders = 0, spider_tick = 0;

int difficulty = 1; // 0=Easy, 1=Medium, 2=Hard
int score = 0;
int current_speed = 150;
int base_speed = 150;
int score_mult = 10;
int wrap_mode = 0;
int campaign_level = 1;
int apples_eaten = 0;
int total_apples = 0;
int games_played = 0;

static int anim_tick = 0;

struct HighScoreEntry leaderboard[5] = {
    { "ACE", 250, "Classic", "2026-07-01" },
    { "SNA", 180, "Maze",    "2026-07-05" },
    { "VIP", 120, "Ramp",    "2026-07-10" },
    { "BOB", 80,  "Wrap",    "2026-07-15" },
    { "NEO", 50,  "Campaign","2026-07-20" }
};

char initials_input[4] = "AAA";
int initials_pos = 0;
int is_high_score_entry = 0;

// Helper String & Conversion functions
char* my_strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
    for (; *haystack; haystack++) {
        if (*haystack == *needle) {
            const char* h = haystack;
            const char* n = needle;
            while (*h && *n && *h == *n) { h++; n++; }
            if (!*n) return (char*)haystack;
        }
    }
    return NULL;
}

int my_atoi(const char* str) {
    int res = 0;
    while (*str && (*str < '0' || *str > '9')) str++;
    while (*str >= '0' && *str <= '9') {
        res = res * 10 + (*str - '0');
        str++;
    }
    return res;
}

// Forward Declarations
int random_int(int max);
void PlaceFood();
void PlaceSpecialFood();
void PlaceGhostFood();
void PlaceIceFood();
void PlaceMultFood();
void InitGame();
void SaveStats();
void LoadStats();
void SaveGameState();
int RestoreGameState();
void ExportStatsText();
void ImportStatsText();

unsigned int rng_state = 12345;
int random_int(int max) {
    rng_state = rng_state * 1103515245 + 12345;
    return ((rng_state >> 16) & 0x7FFF) % max;
}

void LoadStats() {
    HANDLE hFile = CreateFileA("ksnake_stats.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesRead;
        ReadFile(hFile, &games_played, sizeof(int), &bytesRead, NULL);
        ReadFile(hFile, &total_apples, sizeof(int), &bytesRead, NULL);
        ReadFile(hFile, leaderboard, sizeof(leaderboard), &bytesRead, NULL);
        CloseHandle(hFile);
    }
}

void SaveStats() {
    HANDLE hFile = CreateFileA("ksnake_stats.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten;
        WriteFile(hFile, &games_played, sizeof(int), &bytesWritten, NULL);
        WriteFile(hFile, &total_apples, sizeof(int), &bytesWritten, NULL);
        WriteFile(hFile, leaderboard, sizeof(leaderboard), &bytesWritten, NULL);
        CloseHandle(hFile);
    }
}

void SaveGameState() {
    HANDLE hFile = CreateFileA("ksnake_save.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bw;
        WriteFile(hFile, &snake_len, sizeof(int), &bw, NULL);
        WriteFile(hFile, snake, sizeof(struct Point) * snake_len, &bw, NULL);
        WriteFile(hFile, &dir_x, sizeof(int), &bw, NULL);
        WriteFile(hFile, &dir_y, sizeof(int), &bw, NULL);
        WriteFile(hFile, &food, sizeof(struct Point), &bw, NULL);
        WriteFile(hFile, &game_mode, sizeof(int), &bw, NULL);
        WriteFile(hFile, &wrap_mode, sizeof(int), &bw, NULL);
        WriteFile(hFile, &difficulty, sizeof(int), &bw, NULL);
        WriteFile(hFile, &score, sizeof(int), &bw, NULL);
        WriteFile(hFile, &score_mult, sizeof(int), &bw, NULL);
        WriteFile(hFile, &current_speed, sizeof(int), &bw, NULL);
        WriteFile(hFile, &num_obstacles, sizeof(int), &bw, NULL);
        WriteFile(hFile, obstacles, sizeof(struct Point) * num_obstacles, &bw, NULL);
        WriteFile(hFile, &campaign_level, sizeof(int), &bw, NULL);
        CloseHandle(hFile);
    }
}

int RestoreGameState() {
    HANDLE hFile = CreateFileA("ksnake_save.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return 0;
    DWORD br;
    ReadFile(hFile, &snake_len, sizeof(int), &br, NULL);
    ReadFile(hFile, snake, sizeof(struct Point) * snake_len, &br, NULL);
    ReadFile(hFile, &dir_x, sizeof(int), &br, NULL);
    ReadFile(hFile, &dir_y, sizeof(int), &br, NULL);
    ReadFile(hFile, &food, sizeof(struct Point), &br, NULL);
    ReadFile(hFile, &game_mode, sizeof(int), &br, NULL);
    ReadFile(hFile, &wrap_mode, sizeof(int), &br, NULL);
    ReadFile(hFile, &difficulty, sizeof(int), &br, NULL);
    ReadFile(hFile, &score, sizeof(int), &br, NULL);
    ReadFile(hFile, &score_mult, sizeof(int), &br, NULL);
    ReadFile(hFile, &current_speed, sizeof(int), &br, NULL);
    ReadFile(hFile, &num_obstacles, sizeof(int), &br, NULL);
    ReadFile(hFile, obstacles, sizeof(struct Point) * num_obstacles, &br, NULL);
    ReadFile(hFile, &campaign_level, sizeof(int), &br, NULL);
    CloseHandle(hFile);
    DeleteFileA("ksnake_save.dat");
    return 1;
}

void ExportStatsText() {
    HANDLE hFile = CreateFileA("ksnake_export.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    
    char buf[512];
    DWORD bw;
    wsprintfA(buf, "=== KSNAKE STATS EXPORT ===\r\nGames Played: %d\r\nTotal Apples: %d\r\n\r\n=== TOP 5 LEADERBOARD ===\r\n", games_played, total_apples);
    WriteFile(hFile, buf, lstrlenA(buf), &bw, NULL);

    for(int i=0; i<5; i++) {
        wsprintfA(buf, "%d. %s - %d (%s) [%s]\r\n", i+1, leaderboard[i].name, leaderboard[i].score, leaderboard[i].mode, leaderboard[i].date);
        WriteFile(hFile, buf, lstrlenA(buf), &bw, NULL);
    }
    CloseHandle(hFile);
    MessageBoxA(NULL, "Stats & Leaderboard exported to 'ksnake_export.txt'!", "Export Success", MB_OK | MB_ICONINFORMATION);
}

void ImportStatsText() {
    HANDLE hFile = CreateFileA("ksnake_export.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxA(NULL, "Could not find 'ksnake_export.txt' to import!", "Import Error", MB_OK | MB_ICONERROR);
        return;
    }
    char buf[1024];
    DWORD br;
    if (ReadFile(hFile, buf, sizeof(buf) - 1, &br, NULL) && br > 0) {
        buf[br] = '\0';
        char* gp_ptr = my_strstr(buf, "Games Played: ");
        if (gp_ptr) games_played = my_atoi(gp_ptr + 14);
        char* ta_ptr = my_strstr(buf, "Total Apples: ");
        if (ta_ptr) total_apples = my_atoi(ta_ptr + 14);
    }
    CloseHandle(hFile);
    SaveStats();
    MessageBoxA(NULL, "Stats & Leaderboard imported from 'ksnake_export.txt'!", "Import Success", MB_OK | MB_ICONINFORMATION);
}

void PlaceFood() {
    int ok = 0;
    while(!ok) {
        food.x = random_int(GRID_WIDTH); food.y = random_int(GRID_HEIGHT);
        ok = 1;
        for(int i=0; i<num_obstacles; i++) if (food.x == obstacles[i].x && food.y == obstacles[i].y) ok = 0;
        for(int i=0; i<snake_len; i++) if (food.x == snake[i].x && food.y == snake[i].y) ok = 0;
    }
}

void PlaceSpecialFood() {
    int ok = 0;
    while(!ok) {
        special_food.x = random_int(GRID_WIDTH); special_food.y = random_int(GRID_HEIGHT);
        ok = 1;
        for(int i=0; i<num_obstacles; i++) if (special_food.x == obstacles[i].x && special_food.y == obstacles[i].y) ok = 0;
        for(int i=0; i<snake_len; i++) if (special_food.x == snake[i].x && special_food.y == snake[i].y) ok = 0;
        if (special_food.x == food.x && special_food.y == food.y) ok = 0;
    }
    special_food_timer = 40;
}

void PlaceGhostFood() {
    int ok = 0;
    while(!ok) {
        ghost_food.x = random_int(GRID_WIDTH); ghost_food.y = random_int(GRID_HEIGHT);
        ok = 1;
        for(int i=0; i<num_obstacles; i++) if(ghost_food.x==obstacles[i].x && ghost_food.y==obstacles[i].y) ok=0;
        for(int i=0; i<snake_len; i++) if(ghost_food.x==snake[i].x && ghost_food.y==snake[i].y) ok=0;
        if(ghost_food.x == food.x && ghost_food.y == food.y) ok=0;
    }
    ghost_food_timer = 40;
}

void PlaceIceFood() {
    int ok = 0;
    while(!ok) {
        ice_food.x = random_int(GRID_WIDTH); ice_food.y = random_int(GRID_HEIGHT);
        ok = 1;
        for(int i=0; i<num_obstacles; i++) if (ice_food.x == obstacles[i].x && ice_food.y == obstacles[i].y) ok = 0;
        for(int i=0; i<snake_len; i++) if (ice_food.x == snake[i].x && ice_food.y == snake[i].y) ok = 0;
        if (ice_food.x == food.x && ice_food.y == food.y) ok = 0;
    }
    ice_food_timer = 40;
}

void PlaceMultFood() {
    int ok = 0;
    while(!ok) {
        mult_food.x = random_int(GRID_WIDTH); mult_food.y = random_int(GRID_HEIGHT);
        ok = 1;
        for(int i=0; i<num_obstacles; i++) if (mult_food.x == obstacles[i].x && mult_food.y == obstacles[i].y) ok = 0;
        for(int i=0; i<snake_len; i++) if (mult_food.x == snake[i].x && mult_food.y == snake[i].y) ok = 0;
        if (mult_food.x == food.x && mult_food.y == food.y) ok = 0;
    }
    mult_food_timer = 40;
}

void InitGame() {
    games_played++;
    SaveStats();
    apples_eaten = 0;
    snake_len = 3;
    snake[0].x = 5; snake[0].y = 5;
    snake[1].x = 4; snake[1].y = 5;
    snake[2].x = 3; snake[2].y = 5;
    dir_x = 1; dir_y = 0;
    last_dir_x = 1; last_dir_y = 0;
    
    if (difficulty == 0) { base_speed = 200; score_mult = 5; }
    else if (difficulty == 1) { base_speed = 150; score_mult = 10; }
    else { base_speed = 100; score_mult = 20; }
    if (game_mode == 2) base_speed = 160;

    current_speed = base_speed;
    game_state = 1;
    score = 0;
    special_food.x = -1; special_food_timer = 0;
    ghost_food.x = -1; ghost_food_timer = 0; ghost_active_timer = 0;
    ice_food.x = -1; ice_food_timer = 0; ice_active_timer = 0;
    mult_food.x = -1; mult_food_timer = 0; mult_active_timer = 0;
    spider_tick = 0;

    num_obstacles = 0;
    if (game_mode == 1) num_obstacles = (difficulty + 1) * 8; // Maze mode
    else if (game_mode == 4) num_obstacles = campaign_level * 4; // Campaign mode

    for(int i=0; i<num_obstacles; i++) {
        int ok = 0;
        while(!ok) {
            obstacles[i].x = random_int(GRID_WIDTH);
            obstacles[i].y = random_int(GRID_HEIGHT);
            ok = 1;
            if (obstacles[i].y == 5 && (obstacles[i].x >= 2 && obstacles[i].x <= 6)) ok = 0;
        }
    }
    
    num_spiders = (game_mode == 1 || game_mode == 4) ? difficulty : 0;
    for(int i=0; i<num_spiders; i++) {
        spiders[i].x = random_int(GRID_WIDTH); spiders[i].y = random_int(GRID_HEIGHT);
    }

    PlaceFood();
}

void DrawSnakeSegmentGDI(HDC hdc, int x, int y, int index, int total, int is_ghost, int d_x, int d_y) {
    int px = x * CELL_SIZE, py = y * CELL_SIZE;
    int cx = px + CELL_SIZE / 2, cy = py + CELL_SIZE / 2;

    HBRUSH oldBrush; HPEN oldPen;

    if (index == 0) {
        HBRUSH headBrush = CreateSolidBrush(is_ghost ? RGB(0, 210, 211) : (mult_active_timer > 0 ? RGB(155, 89, 182) : RGB(46, 204, 113)));
        HPEN headPen = CreatePen(PS_SOLID, 1, is_ghost ? RGB(0, 180, 200) : RGB(30, 130, 70));

        oldBrush = (HBRUSH)SelectObject(hdc, headBrush);
        oldPen = (HPEN)SelectObject(hdc, headPen);

        Ellipse(hdc, px + 1, py + 1, px + CELL_SIZE - 1, py + CELL_SIZE - 1);

        HBRUSH eyeBrush = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, eyeBrush);

        int eye1_x = cx + d_y * 3 + d_x * 2;
        int eye1_y = cy + d_x * 3 + d_y * 2;
        int eye2_x = cx - d_y * 3 + d_x * 2;
        int eye2_y = cy - d_x * 3 + d_y * 2;

        Ellipse(hdc, eye1_x - 2, eye1_y - 2, eye1_x + 2, eye1_y + 2);
        Ellipse(hdc, eye2_x - 2, eye2_y - 2, eye2_x + 2, eye2_y + 2);

        HBRUSH pupilBrush = CreateSolidBrush(RGB(30, 39, 46));
        SelectObject(hdc, pupilBrush);
        Ellipse(hdc, eye1_x + d_x - 1, eye1_y + d_y - 1, eye1_x + d_x + 1, eye1_y + d_y + 1);
        Ellipse(hdc, eye2_x + d_x - 1, eye2_y + d_y - 1, eye2_x + d_x + 1, eye2_y + d_y + 1);

        DeleteObject(eyeBrush); DeleteObject(pupilBrush);

        SelectObject(hdc, oldBrush); SelectObject(hdc, oldPen);
        DeleteObject(headBrush); DeleteObject(headPen);
    } else {
        HBRUSH bodyBrush = CreateSolidBrush(is_ghost ? RGB(72, 219, 251) : (mult_active_timer > 0 ? RGB(142, 68, 173) : (index % 2 == 0 ? RGB(46, 204, 113) : RGB(33, 140, 116))));
        HPEN bodyPen = CreatePen(PS_SOLID, 1, RGB(25, 110, 90));

        oldBrush = (HBRUSH)SelectObject(hdc, bodyBrush);
        oldPen = (HPEN)SelectObject(hdc, bodyPen);

        int inset = 1 + index / 15;
        if (inset > 4) inset = 4;
        Ellipse(hdc, px + inset, py + inset, px + CELL_SIZE - inset, py + CELL_SIZE - inset);

        SelectObject(hdc, oldBrush); SelectObject(hdc, oldPen);
        DeleteObject(bodyBrush); DeleteObject(bodyPen);
    }
}

void DrawGemGDI(HDC hdc, int x, int y, COLORREF color) {
    int px = x * CELL_SIZE, py = y * CELL_SIZE;
    HBRUSH brush = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    Ellipse(hdc, px + 2, py + 2, px + CELL_SIZE - 2, py + CELL_SIZE - 2);

    SelectObject(hdc, oldBrush); SelectObject(hdc, oldPen);
    DeleteObject(brush); DeleteObject(pen);
}

void DrawObstacleGDI(HDC hdc, int x, int y) {
    int px = x * CELL_SIZE, py = y * CELL_SIZE;
    RECT r = { px, py, px + CELL_SIZE - 1, py + CELL_SIZE - 1 };
    HBRUSH bgBrush = CreateSolidBrush(RGB(75, 101, 132));
    FillRect(hdc, &r, bgBrush);
    DeleteObject(bgBrush);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            LoadStats();
            break;

        case WM_TIMER: {
            if (game_state != 1) break;
            anim_tick++;
            
            last_dir_x = dir_x; last_dir_y = dir_y;
            
            for(int i = snake_len - 1; i > 0; i--) snake[i] = snake[i-1];
            snake[0].x += dir_x; snake[0].y += dir_y;

            int effective_wrap = wrap_mode || (game_mode == 3);
            if (effective_wrap) {
                if (snake[0].x < 0) snake[0].x = GRID_WIDTH - 1;
                else if (snake[0].x >= GRID_WIDTH) snake[0].x = 0;
                if (snake[0].y < 0) snake[0].y = GRID_HEIGHT - 1;
                else if (snake[0].y >= GRID_HEIGHT) snake[0].y = 0;
            } else {
                if (snake[0].x < 0 || snake[0].x >= GRID_WIDTH || snake[0].y < 0 || snake[0].y >= GRID_HEIGHT) {
                    game_state = 2; MessageBeep(MB_ICONHAND);
                }
            }

            if (ghost_active_timer == 0) {
                for(int i = 1; i < snake_len; i++) {
                    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) game_state = 2;
                }
                for(int i = 0; i < num_obstacles; i++) {
                    if (snake[0].x == obstacles[i].x && snake[0].y == obstacles[i].y) game_state = 2;
                }
            }

            if (game_state == 2) {
                // Check if Top 5 Score
                is_high_score_entry = 0;
                if (score > leaderboard[4].score) {
                    is_high_score_entry = 1;
                    initials_input[0] = 'A'; initials_input[1] = 'A'; initials_input[2] = 'A'; initials_input[3] = '\0';
                    initials_pos = 0;
                }
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }

            // Eat regular food
            if (snake[0].x == food.x && snake[0].y == food.y) {
                MessageBeep(MB_OK);
                if (snake_len < 400) { snake[snake_len] = snake[snake_len-1]; snake_len++; }
                int gain = score_mult * (mult_active_timer > 0 ? 3 : 1);
                score += gain;
                total_apples++;
                SaveStats();

                if (game_mode == 2 && current_speed > 35) current_speed -= 5;
                else if (current_speed > 40) current_speed -= 2;

                SetTimer(hwnd, TIMER_ID, ice_active_timer > 0 ? current_speed + 90 : current_speed, NULL);
                PlaceFood();
                
                if (special_food.x == -1 && random_int(100) < 22) PlaceSpecialFood();
                if (ghost_food.x == -1 && random_int(100) < 18) PlaceGhostFood();
                if (ice_food.x == -1 && random_int(100) < 18) PlaceIceFood();
                if (mult_food.x == -1 && random_int(100) < 18) PlaceMultFood();
            }

            if (ghost_active_timer > 0) ghost_active_timer--;
            if (ice_active_timer > 0) {
                ice_active_timer--;
                if (ice_active_timer == 0) SetTimer(hwnd, TIMER_ID, current_speed, NULL);
            }
            if (mult_active_timer > 0) mult_active_timer--;

            if (special_food.x != -1 && snake[0].x == special_food.x && snake[0].y == special_food.y) {
                MessageBeep(MB_ICONASTERISK);
                score += score_mult * 4; snake_len -= 3; if (snake_len < 3) snake_len = 3;
                special_food.x = -1;
            }

            if (ghost_food.x != -1 && snake[0].x == ghost_food.x && snake[0].y == ghost_food.y) {
                MessageBeep(MB_ICONASTERISK); ghost_active_timer = 50; ghost_food.x = -1;
            }

            if (ice_food.x != -1 && snake[0].x == ice_food.x && snake[0].y == ice_food.y) {
                MessageBeep(MB_ICONASTERISK); ice_active_timer = 50; ice_food.x = -1;
                SetTimer(hwnd, TIMER_ID, current_speed + 90, NULL);
            }

            if (mult_food.x != -1 && snake[0].x == mult_food.x && snake[0].y == mult_food.y) {
                MessageBeep(MB_ICONASTERISK); mult_active_timer = 50; mult_food.x = -1;
            }

            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }

        case WM_CHAR: {
            if (game_state == 2 && is_high_score_entry) {
                if (wParam >= 'a' && wParam <= 'z') wParam -= 32;
                if (wParam >= 'A' && wParam <= 'Z') {
                    if (initials_pos < 3) {
                        initials_input[initials_pos++] = (char)wParam;
                    }
                } else if (wParam == VK_BACK) {
                    if (initials_pos > 0) initials_input[--initials_pos] = 'A';
                } else if (wParam == VK_RETURN) {
                    // Save Top 5 score
                    struct HighScoreEntry entry;
                    lstrcpyA(entry.name, initials_input);
                    entry.score = score;
                    lstrcpyA(entry.mode, mode_names[game_mode]);
                    lstrcpyA(entry.date, "2026-07-23");

                    leaderboard[4] = entry;
                    // Sort leaderboard descending
                    for(int i=0; i<4; i++) {
                        for(int j=i+1; j<5; j++) {
                            if (leaderboard[j].score > leaderboard[i].score) {
                                struct HighScoreEntry temp = leaderboard[i];
                                leaderboard[i] = leaderboard[j];
                                leaderboard[j] = temp;
                            }
                        }
                    }
                    SaveStats();
                    is_high_score_entry = 0;
                    game_state = 5; // Show leaderboard
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }

        case WM_KEYDOWN: {
            if (game_state == 0) { // Menu
                if (wParam == 'M') { game_mode = (game_mode + 1) % 5; }
                else if (wParam == '1') difficulty = 0;
                else if (wParam == '2') difficulty = 1;
                else if (wParam == '3') difficulty = 2;
                else if (wParam == 'W') wrap_mode = !wrap_mode;
                else if (wParam == 'H') game_state = 5;
                else if (wParam == 'R') {
                    if (RestoreGameState()) {
                        game_state = 1; SetTimer(hwnd, TIMER_ID, current_speed, NULL);
                    }
                }
                else if (wParam == 'E') ExportStatsText();
                else if (wParam == 'I') ImportStatsText();
                else if (wParam == VK_RETURN) {
                    InitGame(); SetTimer(hwnd, TIMER_ID, current_speed, NULL);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (game_state == 5) { // Leaderboard view
                if (wParam == VK_RETURN || wParam == VK_ESCAPE) { game_state = 0; InvalidateRect(hwnd, NULL, TRUE); }
            } else if (game_state == 3) { // Paused
                if (wParam == 'P') { game_state = 1; SetTimer(hwnd, TIMER_ID, current_speed, NULL); }
                else if (wParam == 'S') { SaveGameState(); game_state = 0; InvalidateRect(hwnd, NULL, TRUE); }
            } else if (game_state == 2) { // Game Over
                if (wParam == VK_RETURN && !is_high_score_entry) { game_state = 0; InvalidateRect(hwnd, NULL, TRUE); }
            } else if (game_state == 1) { // Playing
                if (wParam == VK_UP && last_dir_y != 1) { dir_x = 0; dir_y = -1; }
                else if (wParam == VK_DOWN && last_dir_y != -1) { dir_x = 0; dir_y = 1; }
                else if (wParam == VK_LEFT && last_dir_x != 1) { dir_x = -1; dir_y = 0; }
                else if (wParam == VK_RIGHT && last_dir_x != -1) { dir_x = 1; dir_y = 0; }
                else if (wParam == 'P') { game_state = 3; KillTimer(hwnd, TIMER_ID); InvalidateRect(hwnd, NULL, TRUE); }
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
            HBRUSH bg = CreateSolidBrush(RGB(15, 15, 26)); FillRect(hdc, &ps.rcPaint, bg); DeleteObject(bg);

            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 255, 255));

            if (game_state == 0) { // MENU
                TextOutA(hdc, 70, 30, "KSNAKE ARCADE", 13);
                char buf[64];
                wsprintfA(buf, "M - Mode: %s", mode_names[game_mode]); TextOutA(hdc, 40, 70, buf, lstrlenA(buf));
                wsprintfA(buf, "1-3 - Difficulty: %s", difficulty==0?"Easy":difficulty==1?"Med":"Hard"); TextOutA(hdc, 40, 95, buf, lstrlenA(buf));
                wsprintfA(buf, "W - Toggle Wrap: %s", (wrap_mode||game_mode==3)?"ON":"OFF"); TextOutA(hdc, 40, 120, buf, lstrlenA(buf));
                TextOutA(hdc, 40, 145, "H - High Scores Leaderboard", 27);
                TextOutA(hdc, 40, 170, "R - Resume Saved Game", 21);
                TextOutA(hdc, 40, 195, "E - Export / I - Import Stats", 29);
                TextOutA(hdc, 50, 235, "[ Press ENTER to Play ]", 23);
            } else if (game_state == 5) { // LEADERBOARD
                SetTextColor(hdc, RGB(0, 210, 211));
                TextOutA(hdc, 60, 20, "TOP 5 LEADERBOARD", 17);
                SetTextColor(hdc, RGB(255, 255, 255));
                for(int i=0; i<5; i++) {
                    char lbuf[64];
                    wsprintfA(lbuf, "%d. %s - %d pts (%s)", i+1, leaderboard[i].name, leaderboard[i].score, leaderboard[i].mode);
                    TextOutA(hdc, 30, 60 + i * 28, lbuf, lstrlenA(lbuf));
                }
                TextOutA(hdc, 40, 230, "Press ENTER to Return", 21);
            } else if (game_state == 3) { // PAUSED
                SetTextColor(hdc, RGB(251, 197, 49));
                TextOutA(hdc, 110, 100, "PAUSED", 6);
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, 50, 130, "P: Resume  |  S: Save & Exit", 28);
            } else if (game_state == 2) { // GAME OVER
                SetTextColor(hdc, RGB(255, 71, 87));
                TextOutA(hdc, 95, 80, "GAME OVER", 9);
                SetTextColor(hdc, RGB(255, 255, 255));
                char sbuf[32]; wsprintfA(sbuf, "Final Score: %d", score); TextOutA(hdc, 80, 110, sbuf, lstrlenA(sbuf));

                if (is_high_score_entry) {
                    SetTextColor(hdc, RGB(76, 209, 55));
                    TextOutA(hdc, 60, 140, "NEW HIGH SCORE RANK!", 20);
                    SetTextColor(hdc, RGB(255, 255, 255));
                    char ibuf[32]; wsprintfA(ibuf, "Initials: [%s]", initials_input);
                    TextOutA(hdc, 75, 170, ibuf, lstrlenA(ibuf));
                    TextOutA(hdc, 45, 200, "Type Initials & Press ENTER", 27);
                } else {
                    TextOutA(hdc, 60, 160, "Press ENTER to Return", 21);
                }
            } else { // PLAYING
                for(int i = 0; i < num_obstacles; i++) DrawObstacleGDI(hdc, obstacles[i].x, obstacles[i].y);
                
                // Draw Regular Food
                DrawGemGDI(hdc, food.x, food.y, RGB(255, 71, 87));
                if (special_food.x != -1) DrawGemGDI(hdc, special_food.x, special_food.y, RGB(241, 196, 15));
                if (ghost_food.x != -1) DrawGemGDI(hdc, ghost_food.x, ghost_food.y, RGB(0, 210, 211));
                if (ice_food.x != -1) DrawGemGDI(hdc, ice_food.x, ice_food.y, RGB(116, 185, 255));
                if (mult_food.x != -1) DrawGemGDI(hdc, mult_food.x, mult_food.y, RGB(155, 89, 182));

                for(int i = snake_len - 1; i >= 0; i--) {
                    DrawSnakeSegmentGDI(hdc, snake[i].x, snake[i].y, i, snake_len, ghost_active_timer > 0, dir_x, dir_y);
                }

                char score_text[64];
                wsprintfA(score_text, "Score: %d  Mode: %s", score, mode_names[game_mode]);
                TextOutA(hdc, 5, 5, score_text, lstrlenA(score_text));
            }

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
    wc.lpszClassName = "KSnakeApp";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    int winWidth = GRID_WIDTH * CELL_SIZE + 16;
    int winHeight = GRID_HEIGHT * CELL_SIZE + 39;

    HWND hwnd = CreateWindowEx(0, "KSnakeApp", "KSnake Arcade", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, winWidth, winHeight, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
