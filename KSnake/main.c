#include <windows.h>

#define CELL_SIZE 15
#define GRID_WIDTH 20
#define GRID_HEIGHT 20
#define TIMER_ID 1

#define ABS(x) (((x) < 0) ? -(x) : (x))

struct Point { int x; int y; };

struct HighScoreEntry {
    char name[4];
    int score;
    char mode[12];
    char date[12];
};

struct CPUSnake {
    struct Point body[50];
    int len;
    int dir_x, dir_y;
    int alive;
    int respawn_timer;
};

struct Boss {
    struct Point body[50];
    int len;
    int hp;
    int max_hp;
    int dir_x, dir_y;
    int alive;
};

// Game State Enum: 0=Menu, 1=Playing, 2=GameOver, 3=Paused, 4=Victory, 5=Leaderboard
int game_state = 0; 
int game_mode = 0; // 0=Classic, 1=Maze, 2=Speed Ramp, 3=Wrap, 4=Campaign
const char* mode_names[] = { "Classic", "Maze", "Ramp", "Wrap", "Campaign" };

struct Point snake[400];
int snake_len = 3;
int dir_x = 1, dir_y = 0;
int last_dir_x = 1, last_dir_y = 0;

// Food & Special Fruits
struct Point food = { 10, 10 };
struct Point golden_apple = { -1, -1 };
int golden_timer = 0;
struct Point poison_berry = { -1, -1 };
int poison_timer = 0, poison_active_timer = 0;
struct Point speed_berry = { -1, -1 };
int speed_timer = 0, speed_active_timer = 0;

// Power-ups (Activated Skills: G, F, M)
int ghost_cd = 0, ghost_active = 0;
int freeze_cd = 0, freeze_active = 0;
int magnet_cd = 0, magnet_active = 0;

// Obstacles & Portals
struct Point obstacles[100];
int num_obstacles = 0;
struct Point portal_a = { -1, -1 };
struct Point portal_b = { -1, -1 };
int portal_active = 0;
int portal_shift_timer = 0;

// CPU Rivals & Boss
struct CPUSnake rivals[2];
int num_rivals = 0;
struct Boss boss;

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

// Forward Declarations
int random_int(int max);
void PlaceFood(void);
void PlaceGoldenApple(void);
void PlacePoisonBerry(void);
void PlaceSpeedBerry(void);
void InitCampaignStage(int level);
void InitCPURivals(void);
void InitGame(void);
void SaveStats(void);
void LoadStats(void);
void SaveGameState(void);
int RestoreGameState(void);
void ExportStatsText(void);
void ImportStatsText(void);

unsigned int rng_state = 12345;
int random_int(int max) {
    if (max <= 0) return 0;
    rng_state = rng_state * 1103515245 + 12345;
    return ((rng_state >> 16) & 0x7FFF) % max;
}

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
    int i;
    wsprintfA(buf, "=== KSNAKE STATS EXPORT ===\r\nGames Played: %d\r\nTotal Apples: %d\r\n\r\n=== TOP 5 LEADERBOARD ===\r\n", games_played, total_apples);
    WriteFile(hFile, buf, lstrlenA(buf), &bw, NULL);

    for(i=0; i<5; i++) {
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
        char* gp_ptr; char* ta_ptr;
        buf[br] = '\0';
        gp_ptr = my_strstr(buf, "Games Played: ");
        if (gp_ptr) games_played = my_atoi(gp_ptr + 14);
        ta_ptr = my_strstr(buf, "Total Apples: ");
        if (ta_ptr) total_apples = my_atoi(ta_ptr + 14);
    }
    CloseHandle(hFile);
    SaveStats();
    MessageBoxA(NULL, "Stats & Leaderboard imported from 'ksnake_export.txt'!", "Import Success", MB_OK | MB_ICONINFORMATION);
}

void PlaceFood() {
    int ok = 0;
    while(!ok) {
        int i;
        food.x = random_int(GRID_WIDTH); food.y = random_int(GRID_HEIGHT);
        ok = 1;
        for(i=0; i<num_obstacles; i++) if (food.x == obstacles[i].x && food.y == obstacles[i].y) ok = 0;
        for(i=0; i<snake_len; i++) if (food.x == snake[i].x && food.y == snake[i].y) ok = 0;
    }
}

void PlaceGoldenApple() {
    int ok = 0;
    while(!ok) {
        int i;
        golden_apple.x = random_int(GRID_WIDTH); golden_apple.y = random_int(GRID_HEIGHT);
        ok = 1;
        for(i=0; i<num_obstacles; i++) if (golden_apple.x == obstacles[i].x && golden_apple.y == obstacles[i].y) ok = 0;
        for(i=0; i<snake_len; i++) if (golden_apple.x == snake[i].x && golden_apple.y == snake[i].y) ok = 0;
        if (golden_apple.x == food.x && golden_apple.y == food.y) ok = 0;
    }
    golden_timer = 50;
}

void PlacePoisonBerry() {
    int ok = 0;
    while(!ok) {
        int i;
        poison_berry.x = random_int(GRID_WIDTH); poison_berry.y = random_int(GRID_HEIGHT);
        ok = 1;
        for(i=0; i<num_obstacles; i++) if (poison_berry.x == obstacles[i].x && poison_berry.y == obstacles[i].y) ok = 0;
        for(i=0; i<snake_len; i++) if (poison_berry.x == snake[i].x && poison_berry.y == snake[i].y) ok = 0;
        if (poison_berry.x == food.x && poison_berry.y == food.y) ok = 0;
    }
    poison_timer = 50;
}

void PlaceSpeedBerry() {
    int ok = 0;
    while(!ok) {
        int i;
        speed_berry.x = random_int(GRID_WIDTH); speed_berry.y = random_int(GRID_HEIGHT);
        ok = 1;
        for(i=0; i<num_obstacles; i++) if (speed_berry.x == obstacles[i].x && speed_berry.y == obstacles[i].y) ok = 0;
        for(i=0; i<snake_len; i++) if (speed_berry.x == snake[i].x && speed_berry.y == snake[i].y) ok = 0;
        if (speed_berry.x == food.x && speed_berry.y == food.y) ok = 0;
    }
    speed_timer = 50;
}

void InitCPURivals() {
    int r;
    for(r=0; r<num_rivals; r++) {
        int b;
        rivals[r].alive = 1;
        rivals[r].len = 4;
        rivals[r].respawn_timer = 0;
        rivals[r].dir_x = (r == 0) ? -1 : 1;
        rivals[r].dir_y = 0;
        for(b=0; b<rivals[r].len; b++) {
            rivals[r].body[b].x = (r == 0) ? (16 - b) : (3 + b);
            rivals[r].body[b].y = (r == 0) ? 14 : 16;
        }
    }
}

void InitCampaignStage(int level) {
    int i, k, dx, dy;
    num_obstacles = 0;
    portal_active = 0;
    portal_a.x = -1; portal_b.x = -1;
    num_rivals = 0;
    boss.alive = 0;

    if (level == 1) {
        portal_active = 1;
        portal_a.x = 2; portal_a.y = 10;
        portal_b.x = 17; portal_b.y = 10;
    } else if (level == 2) {
        int px[4] = {4, 15, 4, 15};
        int py[4] = {4, 4, 15, 15};
        for(k=0; k<4; k++) {
            for(dx=0; dx<2; dx++) {
                for(dy=0; dy<2; dy++) {
                    obstacles[num_obstacles].x = px[k]+dx;
                    obstacles[num_obstacles].y = py[k]+dy;
                    num_obstacles++;
                }
            }
        }
    } else if (level == 3) {
        for(i=4; i<=15; i++) {
            if (i == 9 || i == 10) continue;
            obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 10; num_obstacles++;
            obstacles[num_obstacles].x = 10; obstacles[num_obstacles].y = i; num_obstacles++;
        }
    } else if (level == 4) {
        for(i=3; i<=16; i++) {
            if (i == 9 || i == 10) continue;
            obstacles[num_obstacles].x = 6; obstacles[num_obstacles].y = i; num_obstacles++;
            obstacles[num_obstacles].x = 13; obstacles[num_obstacles].y = i; num_obstacles++;
        }
    } else if (level == 5) {
        for(i=4; i<=15; i++) {
            if (i != 9 && i != 10) {
                obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 4; num_obstacles++;
                obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 15; num_obstacles++;
                obstacles[num_obstacles].x = 4; obstacles[num_obstacles].y = i; num_obstacles++;
                obstacles[num_obstacles].x = 15; obstacles[num_obstacles].y = i; num_obstacles++;
            }
        }
        num_rivals = 1;
    } else if (level == 6) {
        for(i=3; i<=14; i++) { obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 3; num_obstacles++; }
        for(i=4; i<=15; i++) { obstacles[num_obstacles].x = 16; obstacles[num_obstacles].y = i; num_obstacles++; }
        for(i=5; i<=16; i++) { obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 16; num_obstacles++; }
        for(i=6; i<=13; i++) { obstacles[num_obstacles].x = 3; obstacles[num_obstacles].y = i; num_obstacles++; }
    } else if (level == 7) {
        int x, y;
        for(x=4; x<=16; x+=4) {
            for(y=4; y<=16; y+=4) {
                obstacles[num_obstacles].x = x; obstacles[num_obstacles].y = y; num_obstacles++;
                obstacles[num_obstacles].x = x+1; obstacles[num_obstacles].y = y; num_obstacles++;
            }
        }
        num_rivals = 1;
    } else if (level == 8) {
        for(i=0; i<6; i++) {
            obstacles[num_obstacles].x = 10 - i; obstacles[num_obstacles].y = 3 + i; num_obstacles++;
            obstacles[num_obstacles].x = 10 + i; obstacles[num_obstacles].y = 3 + i; num_obstacles++;
            obstacles[num_obstacles].x = 10 - i; obstacles[num_obstacles].y = 16 - i; num_obstacles++;
            obstacles[num_obstacles].x = 10 + i; obstacles[num_obstacles].y = 16 - i; num_obstacles++;
        }
        portal_active = 1;
        portal_a.x = 10; portal_a.y = 2;
        portal_b.x = 10; portal_b.y = 17;
    } else if (level == 9) {
        for(i=2; i<=17; i++) {
            if (i >= 8 && i <= 11) continue;
            obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 3; num_obstacles++;
            obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 16; num_obstacles++;
        }
        for(i=4; i<=15; i++) {
            obstacles[num_obstacles].x = 9; obstacles[num_obstacles].y = i; num_obstacles++;
            obstacles[num_obstacles].x = 10; obstacles[num_obstacles].y = i; num_obstacles++;
        }
        num_rivals = 1;
    } else if (level == 10) {
        for(i=2; i<=17; i++) {
            if (i != 9 && i != 10) {
                obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 2; num_obstacles++;
                obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 17; num_obstacles++;
            }
        }
        for(i=6; i<=13; i++) {
            if (i != 9 && i != 10) {
                obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 6; num_obstacles++;
                obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 13; num_obstacles++;
            }
        }
        num_rivals = 1;
    } else if (level == 11) {
        int y, x;
        for(y=3; y<=16; y+=3) {
            int start = (y % 6 == 0) ? 0 : 5;
            int end = (y % 6 == 0) ? 14 : 19;
            for(x=start; x<=end; x++) {
                obstacles[num_obstacles].x = x; obstacles[num_obstacles].y = y; num_obstacles++;
            }
        }
        portal_active = 1;
        portal_a.x = 0; portal_a.y = 0;
        portal_b.x = 19; portal_b.y = 19;
    } else if (level == 12) {
        for(i=3; i<=16; i++) {
            if (i != 9 && i != 10) {
                obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 3; num_obstacles++;
                obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 16; num_obstacles++;
                obstacles[num_obstacles].x = 3; obstacles[num_obstacles].y = i; num_obstacles++;
                obstacles[num_obstacles].x = 16; obstacles[num_obstacles].y = i; num_obstacles++;
            }
        }
        num_rivals = 2;
    } else if (level == 13) {
        for(i=3; i<=16; i++) {
            if (i >= 8 && i <= 11) continue;
            obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = i; num_obstacles++;
            obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 19 - i; num_obstacles++;
        }
        num_rivals = 2;
    } else if (level == 14) {
        int isles_x[6] = {3, 10, 16, 3, 10, 16};
        int isles_y[6] = {3, 3, 3, 16, 16, 16};
        for(k=0; k<6; k++) {
            obstacles[num_obstacles].x = isles_x[k]; obstacles[num_obstacles].y = isles_y[k]; num_obstacles++;
            obstacles[num_obstacles].x = isles_x[k]+1; obstacles[num_obstacles].y = isles_y[k]; num_obstacles++;
            obstacles[num_obstacles].x = isles_x[k]; obstacles[num_obstacles].y = isles_y[k]+1; num_obstacles++;
            obstacles[num_obstacles].x = isles_x[k]+1; obstacles[num_obstacles].y = isles_y[k]+1; num_obstacles++;
        }
        num_rivals = 2;
    } else if (level == 15) {
        int i_x, j_y;
        for(i_x=2; i_x<=17; i_x+=3) {
            for(j_y=2; j_y<=17; j_y+=3) {
                obstacles[num_obstacles].x = i_x; obstacles[num_obstacles].y = j_y; num_obstacles++;
            }
        }
        portal_active = 1;
        portal_a.x = 1; portal_a.y = 10;
        portal_b.x = 18; portal_b.y = 10;
        num_rivals = 2;
    } else if (level == 16) {
        int x, y;
        for(x=2; x<=17; x+=2) {
            for(y=1; y<=18; y++) {
                if ((x / 2) % 2 == 0 && y > 14) continue;
                if ((x / 2) % 2 == 1 && y < 5) continue;
                obstacles[num_obstacles].x = x; obstacles[num_obstacles].y = y; num_obstacles++;
            }
        }
        num_rivals = 2;
    } else if (level == 17) {
        for(i=0; i<30; i++) {
            int cx = (i * 7 + 3) % 18 + 1;
            int cy = (i * 11 + 5) % 18 + 1;
            if (cy == 5 && cx >= 2 && cx <= 6) continue;
            obstacles[num_obstacles].x = cx; obstacles[num_obstacles].y = cy; num_obstacles++;
        }
        portal_active = 1;
        portal_a.x = 3; portal_a.y = 3;
        portal_b.x = 16; portal_b.y = 16;
        num_rivals = 2;
    } else if (level == 18) {
        for(i=2; i<=17; i++) {
            obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 5; num_obstacles++;
            obstacles[num_obstacles].x = 19 - i; obstacles[num_obstacles].y = 14; num_obstacles++;
        }
        num_rivals = 2;
    } else if (level == 19) {
        for(i=2; i<=17; i++) {
            if (i % 2 == 0) {
                obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 4; num_obstacles++;
                obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 8; num_obstacles++;
                obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 12; num_obstacles++;
                obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 16; num_obstacles++;
            }
        }
        portal_active = 1;
        portal_a.x = 1; portal_a.y = 1;
        portal_b.x = 18; portal_b.y = 18;
        num_rivals = 2;
    } else if (level == 20) {
        // Stage 20: HYDRA VIPER BOSS LAIR
        int px[4] = {1, 17, 1, 17};
        int py[4] = {1, 1, 17, 17};
        for(k=0; k<4; k++) {
            obstacles[num_obstacles].x = px[k]; obstacles[num_obstacles].y = py[k]; num_obstacles++;
        }
        boss.alive = 1;
        boss.hp = 15;
        boss.max_hp = 15;
        boss.len = 8;
        boss.dir_x = 0; boss.dir_y = 1;
        for(k=0; k<boss.len; k++) {
            boss.body[k].x = 10;
            boss.body[k].y = 2 + k;
        }
    }

    if (num_rivals > 0) InitCPURivals();
}

void InitGame() {
    int i;
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
    golden_apple.x = -1; golden_timer = 0;
    poison_berry.x = -1; poison_timer = 0; poison_active_timer = 0;
    speed_berry.x = -1; speed_timer = 0; speed_active_timer = 0;
    
    ghost_cd = 0; ghost_active = 0;
    freeze_cd = 0; freeze_active = 0;
    magnet_cd = 0; magnet_active = 0;

    num_obstacles = 0;
    if (game_mode == 1) { // Maze mode
        num_obstacles = (difficulty + 1) * 8;
        for(i=0; i<num_obstacles; i++) {
            int ok = 0;
            while(!ok) {
                obstacles[i].x = random_int(GRID_WIDTH);
                obstacles[i].y = random_int(GRID_HEIGHT);
                ok = 1;
                if (obstacles[i].y == 5 && (obstacles[i].x >= 2 && obstacles[i].x <= 6)) ok = 0;
            }
        }
    } else if (game_mode == 4) { // Campaign Mode
        InitCampaignStage(campaign_level);
    }

    PlaceFood();
}

void UpdateCPURivals() {
    int r;
    for(r=0; r<num_rivals; r++) {
        int b, d, head_x, head_y, min_dist, best_dir_x, best_dir_y;
        struct Point target;
        int dirs[4][2] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };

        if (!rivals[r].alive) {
            if (rivals[r].respawn_timer > 0) {
                rivals[r].respawn_timer--;
                if (rivals[r].respawn_timer == 0) {
                    rivals[r].alive = 1;
                    rivals[r].len = 4;
                    rivals[r].body[0].x = random_int(GRID_WIDTH);
                    rivals[r].body[0].y = random_int(GRID_HEIGHT);
                    for(b=1; b<rivals[r].len; b++) rivals[r].body[b] = rivals[r].body[0];
                }
            }
            continue;
        }

        target = food;
        if (golden_apple.x != -1) target = golden_apple;

        best_dir_x = rivals[r].dir_x;
        best_dir_y = rivals[r].dir_y;
        min_dist = 9999;

        for(d=0; d<4; d++) {
            int dx = dirs[d][0], dy = dirs[d][1];
            int nx, ny, blocked, o, p;
            if (dx == -rivals[r].dir_x && dy == -rivals[r].dir_y) continue;

            nx = rivals[r].body[0].x + dx;
            ny = rivals[r].body[0].y + dy;

            blocked = 0;
            if (nx < 0 || nx >= GRID_WIDTH || ny < 0 || ny >= GRID_HEIGHT) blocked = 1;
            for(o=0; o<num_obstacles; o++) {
                if (nx == obstacles[o].x && ny == obstacles[o].y) { blocked = 1; break; }
            }
            for(p=0; p<snake_len; p++) {
                if (nx == snake[p].x && ny == snake[p].y) { blocked = 1; break; }
            }
            for(b=1; b<rivals[r].len; b++) {
                if (nx == rivals[r].body[b].x && ny == rivals[r].body[b].y) { blocked = 1; break; }
            }

            if (!blocked) {
                int dist = ABS(nx - target.x) + ABS(ny - target.y);
                if (dist < min_dist) {
                    min_dist = dist;
                    best_dir_x = dx;
                    best_dir_y = dy;
                }
            }
        }

        rivals[r].dir_x = best_dir_x;
        rivals[r].dir_y = best_dir_y;

        for(b=rivals[r].len-1; b>0; b--) rivals[r].body[b] = rivals[r].body[b-1];
        rivals[r].body[0].x += rivals[r].dir_x;
        rivals[r].body[0].y += rivals[r].dir_y;

        head_x = rivals[r].body[0].x;
        head_y = rivals[r].body[0].y;
        if (head_x < 0 || head_x >= GRID_WIDTH || head_y < 0 || head_y >= GRID_HEIGHT) {
            rivals[r].alive = 0; rivals[r].respawn_timer = 30; continue;
        }
        for(b=0; b<num_obstacles; b++) {
            if (head_x == obstacles[b].x && head_y == obstacles[b].y) {
                rivals[r].alive = 0; rivals[r].respawn_timer = 30; break;
            }
        }

        if (head_x == food.x && head_y == food.y) {
            if (rivals[r].len < 20) {
                rivals[r].body[rivals[r].len] = rivals[r].body[rivals[r].len-1];
                rivals[r].len++;
            }
            PlaceFood();
        }
    }
}

void UpdateBoss() {
    int b, d, min_dist, best_dir_x, best_dir_y;
    struct Point target;
    int dirs[4][2] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };

    if (!boss.alive) return;

    target = snake[0];
    best_dir_x = boss.dir_x;
    best_dir_y = boss.dir_y;
    min_dist = 9999;

    for(d=0; d<4; d++) {
        int dx = dirs[d][0], dy = dirs[d][1];
        int nx, ny, blocked, o;
        if (dx == -boss.dir_x && dy == -boss.dir_y) continue;
        nx = boss.body[0].x + dx;
        ny = boss.body[0].y + dy;

        blocked = 0;
        if (nx < 0 || nx >= GRID_WIDTH || ny < 0 || ny >= GRID_HEIGHT) blocked = 1;
        for(o=0; o<num_obstacles; o++) {
            if (nx == obstacles[o].x && ny == obstacles[o].y) { blocked = 1; break; }
        }

        if (!blocked) {
            int dist = ABS(nx - target.x) + ABS(ny - target.y);
            if (dist < min_dist) {
                min_dist = dist;
                best_dir_x = dx;
                best_dir_y = dy;
            }
        }
    }

    boss.dir_x = best_dir_x; boss.dir_y = best_dir_y;

    for(b=boss.len-1; b>0; b--) boss.body[b] = boss.body[b-1];
    boss.body[0].x += boss.dir_x;
    boss.body[0].y += boss.dir_y;
}

void ApplyFoodMagnet() {
    int px, py;
    if (magnet_active <= 0) return;
    px = snake[0].x; py = snake[0].y;

    if (food.x != -1) {
        if (food.x < px) food.x++; else if (food.x > px) food.x--;
        else if (food.y < py) food.y++; else if (food.y > py) food.y--;
    }
    if (golden_apple.x != -1) {
        if (golden_apple.x < px) golden_apple.x++; else if (golden_apple.x > px) golden_apple.x--;
        else if (golden_apple.y < py) golden_apple.y++; else if (golden_apple.y > py) golden_apple.y--;
    }
    if (poison_berry.x != -1) {
        if (poison_berry.x < px) poison_berry.x++; else if (poison_berry.x > px) poison_berry.x--;
        else if (poison_berry.y < py) poison_berry.y++; else if (poison_berry.y > py) poison_berry.y--;
    }
    if (speed_berry.x != -1) {
        if (speed_berry.x < px) speed_berry.x++; else if (speed_berry.x > px) speed_berry.x--;
        else if (speed_berry.y < py) speed_berry.y++; else if (speed_berry.y > py) speed_berry.y--;
    }
}

void DrawSnakeSegmentGDI(HDC hdc, int x, int y, int index, int total, int is_ghost, int d_x, int d_y) {
    int px = x * CELL_SIZE, py = y * CELL_SIZE + 30;
    int cx = px + CELL_SIZE / 2, cy = py + CELL_SIZE / 2;

    HBRUSH oldBrush; HPEN oldPen;

    if (index == 0) {
        HBRUSH headBrush = CreateSolidBrush(is_ghost ? RGB(0, 210, 211) : (speed_active_timer > 0 ? RGB(230, 126, 34) : RGB(46, 204, 113)));
        HPEN headPen = CreatePen(PS_SOLID, 1, is_ghost ? RGB(0, 180, 200) : RGB(30, 130, 70));
        HBRUSH eyeBrush, pupilBrush;
        int eye1_x, eye1_y, eye2_x, eye2_y;

        oldBrush = (HBRUSH)SelectObject(hdc, headBrush);
        oldPen = (HPEN)SelectObject(hdc, headPen);

        Ellipse(hdc, px + 1, py + 1, px + CELL_SIZE - 1, py + CELL_SIZE - 1);

        eyeBrush = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, eyeBrush);

        eye1_x = cx + d_y * 3 + d_x * 2;
        eye1_y = cy + d_x * 3 + d_y * 2;
        eye2_x = cx - d_y * 3 + d_x * 2;
        eye2_y = cy - d_x * 3 + d_y * 2;

        Ellipse(hdc, eye1_x - 2, eye1_y - 2, eye1_x + 2, eye1_y + 2);
        Ellipse(hdc, eye2_x - 2, eye2_y - 2, eye2_x + 2, eye2_y + 2);

        pupilBrush = CreateSolidBrush(RGB(30, 39, 46));
        SelectObject(hdc, pupilBrush);
        Ellipse(hdc, eye1_x + d_x - 1, eye1_y + d_y - 1, eye1_x + d_x + 1, eye1_y + d_y + 1);
        Ellipse(hdc, eye2_x + d_x - 1, eye2_y + d_y - 1, eye2_x + d_x + 1, eye2_y + d_y + 1);

        DeleteObject(eyeBrush); DeleteObject(pupilBrush);

        SelectObject(hdc, oldBrush); SelectObject(hdc, oldPen);
        DeleteObject(headBrush); DeleteObject(headPen);
    } else {
        HBRUSH bodyBrush = CreateSolidBrush(is_ghost ? RGB(72, 219, 251) : (speed_active_timer > 0 ? RGB(241, 196, 15) : (index % 2 == 0 ? RGB(46, 204, 113) : RGB(33, 140, 116))));
        HPEN bodyPen = CreatePen(PS_SOLID, 1, RGB(25, 110, 90));
        int inset = 1 + index / 15;
        if (inset > 4) inset = 4;

        oldBrush = (HBRUSH)SelectObject(hdc, bodyBrush);
        oldPen = (HPEN)SelectObject(hdc, bodyPen);

        Ellipse(hdc, px + inset, py + inset, px + CELL_SIZE - inset, py + CELL_SIZE - inset);

        SelectObject(hdc, oldBrush); SelectObject(hdc, oldPen);
        DeleteObject(bodyBrush); DeleteObject(bodyPen);
    }
}

void DrawRivalGDI(HDC hdc, int x, int y, int index) {
    int px = x * CELL_SIZE, py = y * CELL_SIZE + 30;
    HBRUSH brush = CreateSolidBrush(index == 0 ? RGB(155, 89, 182) : RGB(142, 68, 173));
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    Ellipse(hdc, px + 1, py + 1, px + CELL_SIZE - 1, py + CELL_SIZE - 1);

    SelectObject(hdc, oldBrush); SelectObject(hdc, oldPen);
    DeleteObject(brush); DeleteObject(pen);
}

void DrawBossGDI(HDC hdc, int x, int y, int index) {
    int px = x * CELL_SIZE, py = y * CELL_SIZE + 30;
    HBRUSH brush = CreateSolidBrush(index == 0 ? RGB(255, 0, 85) : RGB(192, 57, 43));
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 215, 0));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    Rectangle(hdc, px, py, px + CELL_SIZE, py + CELL_SIZE);

    SelectObject(hdc, oldBrush); SelectObject(hdc, oldPen);
    DeleteObject(brush); DeleteObject(pen);
}

void DrawGemGDI(HDC hdc, int x, int y, COLORREF color) {
    int px = x * CELL_SIZE, py = y * CELL_SIZE + 30;
    HBRUSH brush = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    Ellipse(hdc, px + 2, py + 2, px + CELL_SIZE - 2, py + CELL_SIZE - 2);

    SelectObject(hdc, oldBrush); SelectObject(hdc, oldPen);
    DeleteObject(brush); DeleteObject(pen);
}

void DrawPortalGDI(HDC hdc, int x, int y, COLORREF color) {
    int px = x * CELL_SIZE, py = y * CELL_SIZE + 30;
    HBRUSH brush = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    Ellipse(hdc, px + 1, py + 1, px + CELL_SIZE - 1, py + CELL_SIZE - 1);

    SelectObject(hdc, oldBrush); SelectObject(hdc, oldPen);
    DeleteObject(brush); DeleteObject(pen);
}

void DrawObstacleGDI(HDC hdc, int x, int y) {
    int px = x * CELL_SIZE, py = y * CELL_SIZE + 30;
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
            int i, r, effective_wrap, gain, spd;
            if (game_state != 1) break;
            anim_tick++;

            // Cooldown & Active Timers
            if (ghost_active > 0) ghost_active--;
            if (ghost_cd > 0) ghost_cd--;
            if (freeze_active > 0) freeze_active--;
            if (freeze_cd > 0) freeze_cd--;
            if (magnet_active > 0) {
                magnet_active--;
                ApplyFoodMagnet();
            }
            if (magnet_cd > 0) magnet_cd--;

            if (poison_active_timer > 0) poison_active_timer--;
            if (speed_active_timer > 0) speed_active_timer--;

            // Update CPUSnakes & Boss
            UpdateCPURivals();
            UpdateBoss();

            // Portals Shift Timer
            if (portal_active) {
                portal_shift_timer++;
                if (portal_shift_timer >= 60) {
                    portal_shift_timer = 0;
                    portal_a.x = random_int(GRID_WIDTH); portal_a.y = random_int(GRID_HEIGHT);
                    portal_b.x = random_int(GRID_WIDTH); portal_b.y = random_int(GRID_HEIGHT);
                }
            }
            
            last_dir_x = dir_x; last_dir_y = dir_y;
            
            for(i = snake_len - 1; i > 0; i--) snake[i] = snake[i-1];
            snake[0].x += dir_x; snake[0].y += dir_y;

            // Check Portals
            if (portal_active) {
                if (snake[0].x == portal_a.x && snake[0].y == portal_a.y) {
                    snake[0].x = portal_b.x; snake[0].y = portal_b.y;
                } else if (snake[0].x == portal_b.x && snake[0].y == portal_b.y) {
                    snake[0].x = portal_a.x; snake[0].y = portal_a.y;
                }
            }

            effective_wrap = wrap_mode || (game_mode == 3);
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

            if (ghost_active == 0) {
                for(i = 1; i < snake_len; i++) {
                    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) game_state = 2;
                }
                for(i = 0; i < num_obstacles; i++) {
                    if (snake[0].x == obstacles[i].x && snake[0].y == obstacles[i].y) game_state = 2;
                }
                for(r = 0; r < num_rivals; r++) {
                    if (!rivals[r].alive) continue;
                    for(i = 0; i < rivals[r].len; i++) {
                        if (snake[0].x == rivals[r].body[i].x && snake[0].y == rivals[r].body[i].y) game_state = 2;
                    }
                }
                if (boss.alive) {
                    for(i = 0; i < boss.len; i++) {
                        if (snake[0].x == boss.body[i].x && snake[0].y == boss.body[i].y) game_state = 2;
                    }
                }
            }

            if (game_state == 2) {
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
                gain = score_mult;
                score += gain;
                total_apples++;
                apples_eaten++;
                SaveStats();

                if (boss.alive) {
                    boss.hp -= 1;
                    if (boss.hp <= 0) {
                        boss.alive = 0;
                        game_state = 4; // VICTORY!
                    }
                }

                if (game_mode == 4 && apples_eaten >= 8 && campaign_level < 20) {
                    campaign_level++;
                    InitCampaignStage(campaign_level);
                    apples_eaten = 0;
                }

                if (game_mode == 2 && current_speed > 35) current_speed -= 5;
                else if (current_speed > 40) current_speed -= 2;

                PlaceFood();
                
                if (golden_apple.x == -1 && random_int(100) < 20) PlaceGoldenApple();
                if (poison_berry.x == -1 && random_int(100) < 20) PlacePoisonBerry();
                if (speed_berry.x == -1 && random_int(100) < 20) PlaceSpeedBerry();
            }

            if (golden_apple.x != -1 && snake[0].x == golden_apple.x && snake[0].y == golden_apple.y) {
                MessageBeep(MB_ICONASTERISK);
                score += 500; snake_len -= 2; if (snake_len < 3) snake_len = 3;
                if (boss.alive) {
                    boss.hp -= 3;
                    if (boss.hp <= 0) { boss.alive = 0; game_state = 4; }
                }
                golden_apple.x = -1;
            }

            if (poison_berry.x != -1 && snake[0].x == poison_berry.x && snake[0].y == poison_berry.y) {
                MessageBeep(MB_ICONHAND);
                score -= 200; if (score < 0) score = 0;
                poison_active_timer = 50;
                poison_berry.x = -1;
            }

            if (speed_berry.x != -1 && snake[0].x == speed_berry.x && snake[0].y == speed_berry.y) {
                MessageBeep(MB_ICONASTERISK);
                score += 300;
                speed_active_timer = 50;
                speed_berry.x = -1;
            }

            spd = current_speed;
            if (freeze_active > 0 || poison_active_timer > 0) spd += 80;
            if (speed_active_timer > 0 && spd > 40) spd -= 40;

            SetTimer(hwnd, TIMER_ID, spd, NULL);
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }

        case WM_CHAR: {
            if (game_state == 2 && is_high_score_entry) {
                if (wParam >= 'a' && wParam <= 'z') wParam -= 32;
                if (wParam >= 'A' && wParam <= 'Z') {
                    if (initials_pos < 3) initials_input[initials_pos++] = (char)wParam;
                } else if (wParam == VK_BACK) {
                    if (initials_pos > 0) initials_input[--initials_pos] = 'A';
                } else if (wParam == VK_RETURN) {
                    int i, j;
                    struct HighScoreEntry entry;
                    lstrcpyA(entry.name, initials_input);
                    entry.score = score;
                    lstrcpyA(entry.mode, mode_names[game_mode]);
                    lstrcpyA(entry.date, "2026-07-23");

                    leaderboard[4] = entry;
                    for(i=0; i<4; i++) {
                        for(j=i+1; j<5; j++) {
                            if (leaderboard[j].score > leaderboard[i].score) {
                                struct HighScoreEntry temp = leaderboard[i];
                                leaderboard[i] = leaderboard[j];
                                leaderboard[j] = temp;
                            }
                        }
                    }
                    SaveStats();
                    is_high_score_entry = 0;
                    game_state = 5;
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
            } else if (game_state == 5) {
                if (wParam == VK_RETURN || wParam == VK_ESCAPE) { game_state = 0; InvalidateRect(hwnd, NULL, TRUE); }
            } else if (game_state == 3) {
                if (wParam == 'P') { game_state = 1; SetTimer(hwnd, TIMER_ID, current_speed, NULL); }
                else if (wParam == 'S') { SaveGameState(); game_state = 0; InvalidateRect(hwnd, NULL, TRUE); }
            } else if (game_state == 2 || game_state == 4) {
                if (wParam == VK_RETURN && !is_high_score_entry) { game_state = 0; InvalidateRect(hwnd, NULL, TRUE); }
            } else if (game_state == 1) { // Playing
                if (wParam == VK_UP && last_dir_y != 1) { dir_x = 0; dir_y = -1; }
                else if (wParam == VK_DOWN && last_dir_y != -1) { dir_x = 0; dir_y = 1; }
                else if (wParam == VK_LEFT && last_dir_x != 1) { dir_x = -1; dir_y = 0; }
                else if (wParam == VK_RIGHT && last_dir_x != -1) { dir_x = 1; dir_y = 0; }
                else if (wParam == 'G' && ghost_cd == 0) { ghost_active = 50; ghost_cd = 150; }
                else if (wParam == 'F' && freeze_cd == 0) { freeze_active = 100; freeze_cd = 200; }
                else if (wParam == 'M' && magnet_cd == 0) { magnet_active = 80; magnet_cd = 150; }
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
                char buf[64];
                TextOutA(hdc, 70, 20, "KSNAKE ARCADE", 13);
                wsprintfA(buf, "M - Mode: %s", mode_names[game_mode]); TextOutA(hdc, 40, 55, buf, lstrlenA(buf));
                wsprintfA(buf, "1-3 - Difficulty: %s", difficulty==0?"Easy":difficulty==1?"Med":"Hard"); TextOutA(hdc, 40, 80, buf, lstrlenA(buf));
                wsprintfA(buf, "W - Toggle Wrap: %s", (wrap_mode||game_mode==3)?"ON":"OFF"); TextOutA(hdc, 40, 105, buf, lstrlenA(buf));
                TextOutA(hdc, 40, 130, "H - High Scores Leaderboard", 27);
                TextOutA(hdc, 40, 155, "R - Resume Saved Game", 21);
                TextOutA(hdc, 40, 180, "E - Export / I - Import Stats", 29);
                TextOutA(hdc, 40, 205, "Skills: [G]host [F]reeze [M]agnet", 33);
                TextOutA(hdc, 50, 245, "[ Press ENTER to Play ]", 23);
            } else if (game_state == 5) {
                int i;
                SetTextColor(hdc, RGB(0, 210, 211));
                TextOutA(hdc, 60, 20, "TOP 5 LEADERBOARD", 17);
                SetTextColor(hdc, RGB(255, 255, 255));
                for(i=0; i<5; i++) {
                    char lbuf[64];
                    wsprintfA(lbuf, "%d. %s - %d pts (%s)", i+1, leaderboard[i].name, leaderboard[i].score, leaderboard[i].mode);
                    TextOutA(hdc, 30, 60 + i * 28, lbuf, lstrlenA(lbuf));
                }
                TextOutA(hdc, 40, 230, "Press ENTER to Return", 21);
            } else if (game_state == 3) {
                SetTextColor(hdc, RGB(251, 197, 49));
                TextOutA(hdc, 110, 100, "PAUSED", 6);
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, 50, 130, "P: Resume  |  S: Save & Exit", 28);
            } else if (game_state == 2) {
                char sbuf[32];
                SetTextColor(hdc, RGB(255, 71, 87));
                TextOutA(hdc, 95, 80, "GAME OVER", 9);
                SetTextColor(hdc, RGB(255, 255, 255));
                wsprintfA(sbuf, "Final Score: %d", score); TextOutA(hdc, 80, 110, sbuf, lstrlenA(sbuf));

                if (is_high_score_entry) {
                    char ibuf[32];
                    SetTextColor(hdc, RGB(76, 209, 55));
                    TextOutA(hdc, 60, 140, "NEW HIGH SCORE RANK!", 20);
                    SetTextColor(hdc, RGB(255, 255, 255));
                    wsprintfA(ibuf, "Initials: [%s]", initials_input);
                    TextOutA(hdc, 75, 170, ibuf, lstrlenA(ibuf));
                    TextOutA(hdc, 45, 200, "Type Initials & Press ENTER", 27);
                } else {
                    TextOutA(hdc, 60, 160, "Press ENTER to Return", 21);
                }
            } else if (game_state == 4) { // VICTORY!
                SetTextColor(hdc, RGB(255, 215, 0));
                TextOutA(hdc, 60, 80, "CAMPAIGN CONQUERED!", 19);
                SetTextColor(hdc, RGB(76, 209, 55));
                TextOutA(hdc, 45, 110, "HYDRA VIPER DEFEATED!", 21);
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, 60, 160, "Press ENTER to Return", 21);
            } else { // PLAYING
                int i, r;
                char score_text[64];
                char hud_text[64];

                for(i = 0; i < num_obstacles; i++) DrawObstacleGDI(hdc, obstacles[i].x, obstacles[i].y);
                
                if (portal_active) {
                    DrawPortalGDI(hdc, portal_a.x, portal_a.y, RGB(0, 210, 211));
                    DrawPortalGDI(hdc, portal_b.x, portal_b.y, RGB(155, 89, 182));
                }

                // Draw Regular & Special Fruits
                DrawGemGDI(hdc, food.x, food.y, RGB(255, 71, 87));
                if (golden_apple.x != -1) DrawGemGDI(hdc, golden_apple.x, golden_apple.y, RGB(255, 215, 0));
                if (poison_berry.x != -1) DrawGemGDI(hdc, poison_berry.x, poison_berry.y, RGB(142, 68, 173));
                if (speed_berry.x != -1) DrawGemGDI(hdc, speed_berry.x, speed_berry.y, RGB(230, 126, 34));

                for(r = 0; r < num_rivals; r++) {
                    if (!rivals[r].alive) continue;
                    for(i = rivals[r].len - 1; i >= 0; i--) DrawRivalGDI(hdc, rivals[r].body[i].x, rivals[r].body[i].y, i);
                }

                if (boss.alive) {
                    for(i = boss.len - 1; i >= 0; i--) DrawBossGDI(hdc, boss.body[i].x, boss.body[i].y, i);
                }

                for(i = snake_len - 1; i >= 0; i--) {
                    DrawSnakeSegmentGDI(hdc, snake[i].x, snake[i].y, i, snake_len, ghost_active > 0, dir_x, dir_y);
                }

                if (game_mode == 4) {
                    if (campaign_level == 20 && boss.alive) {
                        wsprintfA(score_text, "Score: %d  L20 BOSS HP: %d/%d", score, boss.hp, boss.max_hp);
                    } else {
                        wsprintfA(score_text, "Score: %d  Stage: %d/20", score, campaign_level);
                    }
                } else {
                    wsprintfA(score_text, "Score: %d  Mode: %s", score, mode_names[game_mode]);
                }
                TextOutA(hdc, 5, 5, score_text, lstrlenA(score_text));

                // Bottom Skills HUD
                wsprintfA(hud_text, "[G]: %s  [F]: %s  [M]: %s",
                    ghost_active > 0 ? "GHOST!" : (ghost_cd == 0 ? "READY" : "CD"),
                    freeze_active > 0 ? "SLOW!" : (freeze_cd == 0 ? "READY" : "CD"),
                    magnet_active > 0 ? "MAG!" : (magnet_cd == 0 ? "READY" : "CD"));
                SetTextColor(hdc, RGB(72, 219, 251));
                TextOutA(hdc, 5, 335, hud_text, lstrlenA(hud_text));
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
    int winWidth, winHeight;
    HWND hwnd;
    MSG msg;

    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KSnakeApp";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    winWidth = GRID_WIDTH * CELL_SIZE + 20;
    winHeight = GRID_HEIGHT * CELL_SIZE + 95;

    hwnd = CreateWindowEx(0, "KSnakeApp", "KSnake Arcade - Loop 7", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, winWidth, winHeight, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    while(GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
