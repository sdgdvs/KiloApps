#include <windows.h>

#pragma function(memset)
void* memset(void* dest, int c, unsigned int count) {
    char* bytes = (char*)dest;
    while (count--) {
        *bytes++ = (char)c;
    }
    return dest;
}

#pragma function(memcpy)
void* memcpy(void* dest, const void* src, unsigned int count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) {
        *d++ = *s++;
    }
    return dest;
}

#define CELL_SIZE 25
#define GRID_WIDTH 20
#define GRID_HEIGHT 20
#define TIMER_ID 1

#define ABS(x) (((x) < 0) ? -(x) : (x))

static const int cos_tab16[16] = { 100, 92, 70, 38, 0, -38, -70, -92, -100, -92, -70, -38, 0, 38, 70, 92 };
static const int sin_tab16[16] = { 0, 38, 70, 92, 100, 92, 70, 38, 0, -38, -70, -92, -100, -92, -70, -38 };

struct Point { int x; int y; };

struct HighScoreEntry {
    char name[4];
    int score;
    char mode[16];
    char date[12];
};

struct CPUSnake {
    struct Point body[50];
    int len;
    int dir_x, dir_y;
    int alive;
    int respawn_timer;
    int type;
};

struct Boss {
    struct Point body[50];
    int len;
    int hp;
    int max_hp;
    int dir_x, dir_y;
    int alive;
    int boss_type; // 0=Hydra, 1=Basilisk, 2=Inferno, 3=Void Ouroboros
    int laser_charge; // For Basilisk
    int laser_row;
    int laser_col;
    int phase_timer; // For Void phase
    int invisible;
};

typedef struct {
    int x, y;
    int vx, vy;
    int life, max_life;
    COLORREF color;
    int type; // 0=Needle spark, 1=Smoke puff, 2=Heavy shard, 3=Energy star
    int size;
    int rot, vrot;
} Particle;
#define MAX_PARTICLES 450
Particle particles[MAX_PARTICLES];
int particle_count = 0;

typedef struct {
    int x, y;
    int r1, r2;
    int max_r;
    int alpha;
    COLORREF color;
} ShockwaveRing;
#define MAX_SHOCKWAVES 16
ShockwaveRing shockwaves[MAX_SHOCKWAVES];
int num_shockwaves = 0;

int screen_shake_timer = 0;
int shockwave_timer = 0;
int shockwave_x = 0, shockwave_y = 0;

struct Point oil_slicks[50];
int num_oil_slicks = 0;

struct Point magma_hazards[50];
int magma_timers[50];
int num_magma = 0;

// Replay System
struct ReplayEvent {
    int tick;
    char action;
};
struct ReplayEvent replay_events[30000];
int replay_event_count = 0;
int is_replay_mode = 0;
unsigned int match_seed = 0;
int match_ticks = 0;
int match_apples_gained = 0;
int replay_playback_idx = 0;
char grid_coverage[GRID_WIDTH][GRID_HEIGHT];
int grid_coverage_count = 0;

// Keybinds
int bind_up = 'W';
int bind_down = 'S';
int bind_left = 'A';
int bind_right = 'D';
int bind_ghost = 'G';
int bind_freeze = 'F';
int bind_mag = 'M';
int bind_pause = 'P';

int config_step = 0;

// Game State Enum: 0=Menu, 1=Playing, 2=GameOver, 3=Paused, 4=Victory, 5=Leaderboard, 6=Config, 7=Stats, 8=Editor, 9=BranchSelect
int game_state = 0; 
#define NUM_MODES 8
int game_mode = 0; // 0=Classic, 1=Maze, 2=Speed Ramp, 3=Wrap, 4=Campaign, 5=VS, 6=Custom Map, 7=Boss Gauntlet
const char* mode_names[NUM_MODES] = { "Classic", "Maze", "Ramp", "Wrap", "Campaign", "VS Mode", "Custom Map", "Gauntlet" };

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
struct Point ghost_berry = { -1, -1 };

// Power-ups (Activated Skills: G, F, M)
int ghost_cd = 0, ghost_active = 0;
int freeze_cd = 0, freeze_active = 0;
int magnet_cd = 0, magnet_active = 0;

// Obstacles & Portals
struct Point obstacles[120];
int num_obstacles = 0;
struct Point portal_a = { -1, -1 };
struct Point portal_b = { -1, -1 };
int portal_active = 0;
int portal_shift_timer = 0;

// Custom Map & Editor
struct Point custom_obstacles[120];
int num_custom_obstacles = 0;
struct Point custom_portal_a = { 2, 10 };
struct Point custom_portal_b = { 17, 10 };
int custom_portal_active = 1;
int editor_cursor_x = 10, editor_cursor_y = 10;
int editor_brush = 0; // 0=Wall, 1=Portal A, 2=Portal B, 3=Erase

// Gauntlet State
int gauntlet_stage = 0; // 0=Hydra, 1=Basilisk, 2=Inferno, 3=Void Ouroboros
const char* boss_names[4] = { "Hydra Viper", "Cyber Basilisk", "Inferno Wyrm", "Void Ouroboros" };
int boss_banner_timer = 0;

// Campaign Branch State
int campaign_branch = 0; // 0=Solar Path, 1=Shadow Path
int campaign_level = 1;
int pending_branch_choice = 0;

// CPU Rivals & Boss
struct CPUSnake rivals[4];
int num_rivals = 0;
struct Boss boss;

int difficulty = 1; // 0=Easy, 1=Medium, 2=Hard
int score = 0;
int current_speed = 150;
int base_speed = 150;
int score_mult = 10;
int wrap_mode = 0;
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
void PlaceGhostBerry(void);
void InitCampaignStage(int level);
void InitGauntletStage(int b_type);
void InitCPURivals(void);
void InitGame(void);
void SaveStats(void);
void LoadStats(void);
void SaveGameState(void);
int RestoreGameState(void);
void SaveCustomMap(void);
void LoadCustomMap(void);
void ClearCustomMap(void);
void GenerateRandomMazeToCustom(void);
void ExportStatsText(void);
void ImportStatsText(void);

unsigned int rng_state = 12345;
int random_int(int max) {
    if (max <= 0) return 0;
    rng_state = rng_state * 1103515245 + 12345;
    return ((rng_state >> 16) & 0x7FFF) % max;
}

static int FastSin(int idx) {
    idx = ((idx % 16) + 16) % 16;
    return sin_tab16[idx];
}
static int FastCos(int idx) {
    idx = ((idx % 16) + 16) % 16;
    return cos_tab16[idx];
}

void AddShockwaveRing(int x, int y, COLORREF color) {
    if (num_shockwaves < MAX_SHOCKWAVES) {
        shockwaves[num_shockwaves].x = x;
        shockwaves[num_shockwaves].y = y;
        shockwaves[num_shockwaves].r1 = 2;
        shockwaves[num_shockwaves].r2 = 1;
        shockwaves[num_shockwaves].max_r = 38;
        shockwaves[num_shockwaves].alpha = 20;
        shockwaves[num_shockwaves].color = color;
        num_shockwaves++;
    }
}

void AddSparks(int cx, int cy, COLORREF color, int count) {
    int i;
    for (i = 0; i < count && particle_count < MAX_PARTICLES; i++) {
        int ang = random_int(16);
        int spd = 2 + random_int(4);
        particles[particle_count].x = cx;
        particles[particle_count].y = cy;
        particles[particle_count].vx = (FastCos(ang) * spd) / 25;
        particles[particle_count].vy = (FastSin(ang) * spd) / 25;
        particles[particle_count].life = 16 + random_int(8);
        particles[particle_count].max_life = particles[particle_count].life;
        particles[particle_count].color = color;
        particles[particle_count].type = 0;
        particles[particle_count].size = 2;
        particles[particle_count].rot = 0;
        particles[particle_count].vrot = 0;
        particle_count++;
    }
}

void SpawnExplosion(int x, int y, COLORREF base_color, int is_big) {
    int i;
    int smoke_count = is_big ? 14 : 7;
    int shard_count = is_big ? 16 : 8;
    int star_count = is_big ? 10 : 5;

    AddShockwaveRing(x, y, base_color);
    if (is_big) AddShockwaveRing(x, y, RGB(255, 255, 255));

    // Layer 0: Incandescent needle core sparks
    AddSparks(x, y, base_color, is_big ? 22 : 12);
    AddSparks(x, y, RGB(255, 255, 255), is_big ? 12 : 6);

    // Layer 1: Expanding buoyant smoke puffs with negative gravity
    for (i = 0; i < smoke_count && particle_count < MAX_PARTICLES; i++) {
        int ang = random_int(16);
        int spd = 1 + random_int(3);
        particles[particle_count].x = x;
        particles[particle_count].y = y;
        particles[particle_count].vx = (FastCos(ang) * spd) / 35;
        particles[particle_count].vy = ((FastSin(ang) * spd) / 35) - 1;
        particles[particle_count].life = 18 + random_int(8);
        particles[particle_count].max_life = particles[particle_count].life;
        particles[particle_count].color = base_color;
        particles[particle_count].type = 1;
        particles[particle_count].size = 3 + random_int(3);
        particles[particle_count].rot = 0;
        particles[particle_count].vrot = 0;
        particle_count++;
    }

    // Layer 2: Heavy kinematic cyber debris shards with tumbling rotation and gravity
    for (i = 0; i < shard_count && particle_count < MAX_PARTICLES; i++) {
        int ang = random_int(16);
        int spd = 2 + random_int(4);
        particles[particle_count].x = x;
        particles[particle_count].y = y;
        particles[particle_count].vx = (FastCos(ang) * spd) / 30;
        particles[particle_count].vy = ((FastSin(ang) * spd) / 30) - 2;
        particles[particle_count].life = 20 + random_int(8);
        particles[particle_count].max_life = particles[particle_count].life;
        particles[particle_count].color = (i % 2 == 0) ? RGB(0, 229, 255) : base_color;
        particles[particle_count].type = 2;
        particles[particle_count].size = 3 + random_int(2);
        particles[particle_count].rot = random_int(16);
        particles[particle_count].vrot = (random_int(3) - 1);
        particle_count++;
    }

    // Layer 3: Radiant golden/cyan celebration energy stars
    for (i = 0; i < star_count && particle_count < MAX_PARTICLES; i++) {
        int ang = random_int(16);
        int spd = 1 + random_int(3);
        particles[particle_count].x = x;
        particles[particle_count].y = y;
        particles[particle_count].vx = (FastCos(ang) * spd) / 35;
        particles[particle_count].vy = (FastSin(ang) * spd) / 35;
        particles[particle_count].life = 22 + random_int(8);
        particles[particle_count].max_life = particles[particle_count].life;
        particles[particle_count].color = (i % 2 == 0) ? RGB(255, 215, 0) : RGB(0, 229, 255);
        particles[particle_count].type = 3;
        particles[particle_count].size = 4 + random_int(2);
        particles[particle_count].rot = 0;
        particles[particle_count].vrot = 1;
        particle_count++;
    }
}

void UpdateParticles(void) {
    int i;
    for (i = 0; i < particle_count; i++) {
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        if (particles[i].type == 1) {
            particles[i].vy -= 1;
        } else if (particles[i].type == 2) {
            particles[i].vy += 1;
            particles[i].rot = (particles[i].rot + particles[i].vrot + 16) % 16;
        }
        particles[i].life--;
        if (particles[i].life <= 0) {
            particles[i] = particles[particle_count - 1];
            particle_count--;
            i--;
        }
    }
}

void UpdateShockwaves(void) {
    int i;
    for (i = 0; i < num_shockwaves; i++) {
        shockwaves[i].r1 += 2;
        shockwaves[i].r2 += 3;
        shockwaves[i].alpha--;
        if (shockwaves[i].alpha <= 0 || shockwaves[i].r1 >= shockwaves[i].max_r) {
            shockwaves[i] = shockwaves[num_shockwaves - 1];
            num_shockwaves--;
            i--;
        }
    }
}

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

void SaveConfig() {
    HANDLE hFile = CreateFileA("ksnake_binds.cfg", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bw;
        int binds[8];
        binds[0] = bind_up;    binds[1] = bind_down;
        binds[2] = bind_left;  binds[3] = bind_right;
        binds[4] = bind_ghost; binds[5] = bind_freeze;
        binds[6] = bind_mag;   binds[7] = bind_pause;
        WriteFile(hFile, binds, sizeof(binds), &bw, NULL);
        CloseHandle(hFile);
    }
}
void LoadConfig() {
    HANDLE hFile = CreateFileA("ksnake_binds.cfg", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD br;
        int binds[8];
        if (ReadFile(hFile, binds, sizeof(binds), &br, NULL) && br == sizeof(binds)) {
            bind_up = binds[0];    bind_down = binds[1];
            bind_left = binds[2];  bind_right = binds[3];
            bind_ghost = binds[4]; bind_freeze = binds[5];
            bind_mag = binds[6];   bind_pause = binds[7];
        }
        CloseHandle(hFile);
    }
}

void SaveCustomMap() {
    HANDLE hFile = CreateFileA("ksnake_custom_map.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bw;
        WriteFile(hFile, &num_custom_obstacles, sizeof(int), &bw, NULL);
        WriteFile(hFile, custom_obstacles, sizeof(struct Point) * num_custom_obstacles, &bw, NULL);
        WriteFile(hFile, &custom_portal_active, sizeof(int), &bw, NULL);
        WriteFile(hFile, &custom_portal_a, sizeof(struct Point), &bw, NULL);
        WriteFile(hFile, &custom_portal_b, sizeof(struct Point), &bw, NULL);
        CloseHandle(hFile);
        MessageBoxA(NULL, "Custom Map Saved Successfully!", "Map Editor", MB_OK | MB_ICONINFORMATION);
    }
}

void LoadCustomMap() {
    HANDLE hFile = CreateFileA("ksnake_custom_map.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD br;
        ReadFile(hFile, &num_custom_obstacles, sizeof(int), &br, NULL);
        if (num_custom_obstacles > 120) num_custom_obstacles = 120;
        ReadFile(hFile, custom_obstacles, sizeof(struct Point) * num_custom_obstacles, &br, NULL);
        ReadFile(hFile, &custom_portal_active, sizeof(int), &br, NULL);
        ReadFile(hFile, &custom_portal_a, sizeof(struct Point), &br, NULL);
        ReadFile(hFile, &custom_portal_b, sizeof(struct Point), &br, NULL);
        CloseHandle(hFile);
    }
}

void ClearCustomMap() {
    num_custom_obstacles = 0;
    custom_portal_active = 0;
    custom_portal_a.x = -1; custom_portal_a.y = -1;
    custom_portal_b.x = -1; custom_portal_b.y = -1;
}

void GenerateRandomMazeToCustom() {
    int i, x, y;
    ClearCustomMap();
    for(i=0; i<30; i++) {
        x = random_int(GRID_WIDTH);
        y = random_int(GRID_HEIGHT);
        if (y == 5 && (x >= 2 && x <= 6)) continue;
        if (num_custom_obstacles < 100) {
            custom_obstacles[num_custom_obstacles].x = x;
            custom_obstacles[num_custom_obstacles].y = y;
            num_custom_obstacles++;
        }
    }
    custom_portal_active = 1;
    custom_portal_a.x = 2; custom_portal_a.y = 2;
    custom_portal_b.x = 17; custom_portal_b.y = 17;
}

void ExportReplay() {
    HANDLE hFile = CreateFileA("match.ksr", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bw;
        WriteFile(hFile, &match_seed, sizeof(unsigned int), &bw, NULL);
        WriteFile(hFile, &game_mode, sizeof(int), &bw, NULL);
        WriteFile(hFile, &difficulty, sizeof(int), &bw, NULL);
        WriteFile(hFile, &wrap_mode, sizeof(int), &bw, NULL);
        WriteFile(hFile, &replay_event_count, sizeof(int), &bw, NULL);
        WriteFile(hFile, replay_events, sizeof(struct ReplayEvent) * replay_event_count, &bw, NULL);
        CloseHandle(hFile);
        MessageBoxA(NULL, "Replay saved to match.ksr", "Export", MB_OK);
    }
}
void ImportReplay() {
    HANDLE hFile = CreateFileA("match.ksr", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD br;
        ReadFile(hFile, &match_seed, sizeof(unsigned int), &br, NULL);
        ReadFile(hFile, &game_mode, sizeof(int), &br, NULL);
        ReadFile(hFile, &difficulty, sizeof(int), &br, NULL);
        ReadFile(hFile, &wrap_mode, sizeof(int), &br, NULL);
        ReadFile(hFile, &replay_event_count, sizeof(int), &br, NULL);
        ReadFile(hFile, replay_events, sizeof(struct ReplayEvent) * replay_event_count, &br, NULL);
        CloseHandle(hFile);
        is_replay_mode = 1;
        InitGame();
    } else {
        MessageBoxA(NULL, "match.ksr not found", "Error", MB_OK);
    }
}
void ExportMatchStatsCSV() {
    HANDLE hFile = CreateFileA("match_stats.csv", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bw;
        char buf[256];
        int pct = (grid_coverage_count * 100) / (GRID_WIDTH * GRID_HEIGHT);
        wsprintfA(buf, "Ticks,Apples,CoveragePct\r\n%d,%d,%d\r\n", match_ticks, match_apples_gained, pct);
        WriteFile(hFile, buf, lstrlenA(buf), &bw, NULL);
        CloseHandle(hFile);
        MessageBoxA(NULL, "Stats saved to match_stats.csv", "Export", MB_OK);
    }
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
        WriteFile(hFile, &campaign_branch, sizeof(int), &bw, NULL);
        WriteFile(hFile, &gauntlet_stage, sizeof(int), &bw, NULL);
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
    ReadFile(hFile, &campaign_branch, sizeof(int), &br, NULL);
    ReadFile(hFile, &gauntlet_stage, sizeof(int), &br, NULL);
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

void PlaceGhostBerry() {
    int ok = 0;
    while(!ok) {
        int i;
        ghost_berry.x = random_int(GRID_WIDTH); ghost_berry.y = random_int(GRID_HEIGHT);
        ok = 1;
        for(i=0; i<num_obstacles; i++) if (ghost_berry.x == obstacles[i].x && ghost_berry.y == obstacles[i].y) ok = 0;
        for(i=0; i<snake_len; i++) if (ghost_berry.x == snake[i].x && ghost_berry.y == snake[i].y) ok = 0;
        if (ghost_berry.x == food.x && ghost_berry.y == food.y) ok = 0;
    }
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
        rivals[r].type = r % 2;
        rivals[r].respawn_timer = 0;
        rivals[r].dir_x = (r == 0) ? -1 : 1;
        rivals[r].dir_y = 0;
        for(b=0; b<rivals[r].len; b++) {
            rivals[r].body[b].x = (r == 0) ? (16 - b) : (3 + b);
            rivals[r].body[b].y = (r == 0) ? 14 : 16;
        }
    }
}

void InitGauntletStage(int b_type) {
    int k;
    num_obstacles = 0;
    portal_active = 0;
    num_rivals = 0;
    num_oil_slicks = 0;
    num_magma = 0;
    
    boss.alive = 1;
    boss.boss_type = b_type;
    boss.laser_charge = 0;
    boss.laser_row = -1;
    boss.laser_col = -1;
    boss.phase_timer = 0;
    boss.invisible = 0;
    boss.dir_x = 0;
    boss.dir_y = 1;
    
    if (b_type == 0) { // Hydra Viper
        boss.max_hp = 15; boss.hp = 15; boss.len = 7;
        base_speed = 130;
    } else if (b_type == 1) { // Cyber Basilisk
        boss.max_hp = 20; boss.hp = 20; boss.len = 8;
        base_speed = 120;
        // Tech obstacles
        obstacles[0].x = 4; obstacles[0].y = 4;
        obstacles[1].x = 15; obstacles[1].y = 4;
        obstacles[2].x = 4; obstacles[2].y = 15;
        obstacles[3].x = 15; obstacles[3].y = 15;
        num_obstacles = 4;
    } else if (b_type == 2) { // Inferno Wyrm
        boss.max_hp = 25; boss.hp = 25; boss.len = 9;
        base_speed = 110;
        // Ring of fire pillars
        obstacles[0].x = 5; obstacles[0].y = 10;
        obstacles[1].x = 14; obstacles[1].y = 10;
        obstacles[2].x = 10; obstacles[2].y = 5;
        obstacles[3].x = 10; obstacles[3].y = 14;
        num_obstacles = 4;
    } else { // Void Ouroboros
        boss.max_hp = 30; boss.hp = 30; boss.len = 10;
        base_speed = 100;
        portal_active = 1;
        portal_a.x = 3; portal_a.y = 3;
        portal_b.x = 16; portal_b.y = 16;
    }
    
    current_speed = base_speed;
    for(k=0; k<boss.len; k++) {
        boss.body[k].x = 10;
        boss.body[k].y = 2 + k;
    }
    boss_banner_timer = 20;
}

void InitCampaignStage(int level) {
    int i, k, dx, dy;
    num_obstacles = 0;
    portal_active = 0;
    portal_a.x = -1; portal_a.y = -1;
    portal_b.x = -1; portal_b.y = -1;
    num_rivals = 0;
    boss.alive = 0;
    num_oil_slicks = 0;
    num_magma = 0;

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
    } else if (level >= 6 && level <= 10) {
        if (campaign_branch == 0) { // Solar Path (Portals & Open Speed)
            for(i=2; i<=17; i+=5) {
                obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = i; num_obstacles++;
                obstacles[num_obstacles].x = 19 - i; obstacles[num_obstacles].y = i; num_obstacles++;
            }
            portal_active = 1;
            portal_a.x = (level % 2 == 0) ? 1 : 18; portal_a.y = 10;
            portal_b.x = (level % 2 == 0) ? 18 : 1; portal_b.y = 10;
            num_rivals = (level >= 8) ? 1 : 0;
        } else { // Shadow Path (Tight stealth alleys & extra rivals)
            for(i=3; i<=16; i++) {
                if (i % 3 != 0) {
                    obstacles[num_obstacles].x = 5; obstacles[num_obstacles].y = i; num_obstacles++;
                    obstacles[num_obstacles].x = 14; obstacles[num_obstacles].y = i; num_obstacles++;
                }
            }
            num_rivals = 2;
        }
    } else if (level >= 11 && level <= 14) {
        if (campaign_branch == 0) {
            int y, x;
            for(y=3; y<=16; y+=4) {
                int start = (y % 8 == 0) ? 0 : 6;
                int end = (y % 8 == 0) ? 13 : 19;
                for(x=start; x<=end; x++) {
                    obstacles[num_obstacles].x = x; obstacles[num_obstacles].y = y; num_obstacles++;
                }
            }
            portal_active = 1;
            portal_a.x = 2; portal_a.y = 2;
            portal_b.x = 17; portal_b.y = 17;
            num_rivals = 1;
        } else {
            int x, y;
            for(x=4; x<=16; x+=4) {
                for(y=4; y<=16; y+=4) {
                    obstacles[num_obstacles].x = x; obstacles[num_obstacles].y = y; num_obstacles++;
                    obstacles[num_obstacles].x = x+1; obstacles[num_obstacles].y = y; num_obstacles++;
                }
            }
            num_rivals = 2;
        }
    } else if (level == 15) {
        // Stage 15 Mid-Boss Encounter: Cyber Basilisk!
        boss.alive = 1; boss.boss_type = 1; boss.hp = 18; boss.max_hp = 18; boss.len = 7;
        boss.dir_x = 0; boss.dir_y = 1;
        for(k=0; k<boss.len; k++) { boss.body[k].x = 10; boss.body[k].y = 2 + k; }
        obstacles[0].x = 4; obstacles[0].y = 4; obstacles[1].x = 15; obstacles[1].y = 15;
        num_obstacles = 2;
    } else if (level >= 16 && level <= 19) {
        if (campaign_branch == 0) {
            for(i=2; i<=17; i++) {
                if (i % 2 == 0) {
                    obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 4; num_obstacles++;
                    obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 15; num_obstacles++;
                }
            }
            portal_active = 1; portal_a.x = 1; portal_a.y = 1; portal_b.x = 18; portal_b.y = 18;
            num_rivals = 2;
        } else {
            for(i=2; i<=17; i++) {
                if (i != 9 && i != 10) {
                    obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 3; num_obstacles++;
                    obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 16; num_obstacles++;
                    obstacles[num_obstacles].x = 3; obstacles[num_obstacles].y = i; num_obstacles++;
                    obstacles[num_obstacles].x = 16; obstacles[num_obstacles].y = i; num_obstacles++;
                }
            }
            num_rivals = 3;
        }
    } else if (level == 20) {
        // Stage 20 Boss: Inferno Wyrm
        boss.alive = 1; boss.boss_type = 2; boss.hp = 22; boss.max_hp = 22; boss.len = 8;
        boss.dir_x = 0; boss.dir_y = 1;
        for(k=0; k<boss.len; k++) { boss.body[k].x = 10; boss.body[k].y = 2 + k; }
    } else if (level >= 21 && level <= 24) {
        for(i=2; i<=17; i+=2) {
            for(k=2; k<=17; k+=2) {
                if (i >= 8 && i <= 11 && k >= 8 && k <= 11) continue;
                obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = k; num_obstacles++;
            }
        }
        num_rivals = 3;
    } else if (level == 25) {
        // Stage 25 Boss: Hydra Viper
        boss.alive = 1; boss.boss_type = 0; boss.hp = 25; boss.max_hp = 25; boss.len = 9;
        boss.dir_x = 0; boss.dir_y = 1;
        for(k=0; k<boss.len; k++) { boss.body[k].x = 10; boss.body[k].y = 2 + k; }
        num_rivals = 1;
    } else if (level >= 26 && level <= 29) {
        for(i=1; i<=18; i++) {
            if (i == 9 || i == 10) continue;
            obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 5; num_obstacles++;
            obstacles[num_obstacles].x = i; obstacles[num_obstacles].y = 14; num_obstacles++;
            obstacles[num_obstacles].x = 5; obstacles[num_obstacles].y = i; num_obstacles++;
            obstacles[num_obstacles].x = 14; obstacles[num_obstacles].y = i; num_obstacles++;
        }
        portal_active = 1; portal_a.x = 0; portal_a.y = 0; portal_b.x = 19; portal_b.y = 19;
        num_rivals = 3;
    } else if (level == 30) {
        // Stage 30 Final Grand Boss: Void Ouroboros!
        boss.alive = 1; boss.boss_type = 3; boss.hp = 30; boss.max_hp = 30; boss.len = 10;
        boss.dir_x = 0; boss.dir_y = 1;
        for(k=0; k<boss.len; k++) { boss.body[k].x = 10; boss.body[k].y = 2 + k; }
        portal_active = 1; portal_a.x = 2; portal_a.y = 2; portal_b.x = 17; portal_b.y = 17;
        num_rivals = 2;
    }

    if (num_rivals > 0) InitCPURivals();
}

void InitGame() {
    int i, x, y;
    if (!is_replay_mode) {
        match_seed = GetTickCount();
        replay_event_count = 0;
        games_played++;
        SaveStats();
    }
    rng_state = match_seed;
    replay_playback_idx = 0;
    match_ticks = 0;
    match_apples_gained = 0;
    grid_coverage_count = 0;
    num_oil_slicks = 0;
    num_magma = 0;
    screen_shake_timer = 0;
    shockwave_timer = 0;
    for(x=0; x<GRID_WIDTH; x++) for(y=0; y<GRID_HEIGHT; y++) grid_coverage[x][y] = 0;
    
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
    ghost_berry.x = -1;
    
    ghost_cd = 0; ghost_active = 0;
    freeze_cd = 0; freeze_active = 0;
    magnet_cd = 0; magnet_active = 0;

    num_obstacles = 0;
    portal_active = 0;
    boss.alive = 0;
    
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
    } else if (game_mode == 5) { // VS Mode
        num_rivals = 2;
        InitCPURivals();
    } else if (game_mode == 6) { // Custom Map Mode
        num_obstacles = num_custom_obstacles;
        for(i=0; i<num_obstacles; i++) obstacles[i] = custom_obstacles[i];
        portal_active = custom_portal_active;
        portal_a = custom_portal_a;
        portal_b = custom_portal_b;
        num_rivals = 0;
    } else if (game_mode == 7) { // Boss Gauntlet
        gauntlet_stage = 0;
        InitGauntletStage(gauntlet_stage);
    } else {
        num_rivals = 0;
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
        if (rivals[r].type == 1) target = snake[0];

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

    // Boss Specific Abilities
    if (boss.boss_type == 0) { // Hydra Viper: Oil slicks
        if (match_ticks % 25 == 0 && num_oil_slicks < 50) {
            oil_slicks[num_oil_slicks].x = boss.body[boss.len-1].x;
            oil_slicks[num_oil_slicks].y = boss.body[boss.len-1].y;
            num_oil_slicks++;
        }
    } else if (boss.boss_type == 1) { // Cyber Basilisk: Laser charge
        boss.laser_charge++;
        if (boss.laser_charge == 20) {
            boss.laser_row = boss.body[0].y;
            boss.laser_col = boss.body[0].x;
        } else if (boss.laser_charge >= 30) {
            // Fire EMP Laser pulse
            shockwave_timer = 20;
            shockwave_x = boss.body[0].x; shockwave_y = boss.body[0].y;
            if (ghost_active == 0) {
                if (snake[0].y == boss.laser_row || snake[0].x == boss.laser_col) {
                    score -= 100; if (score < 0) score = 0;
                    screen_shake_timer = 20;
                    MessageBeep(MB_ICONHAND);
                }
            }
            boss.laser_charge = 0;
            boss.laser_row = -1; boss.laser_col = -1;
        }
    } else if (boss.boss_type == 2) { // Inferno Wyrm: Magma hazard tiles
        if (match_ticks % 15 == 0 && num_magma < 50) {
            magma_hazards[num_magma].x = boss.body[boss.len-1].x;
            magma_hazards[num_magma].y = boss.body[boss.len-1].y;
            magma_timers[num_magma] = 60; // lingers 60 ticks
            num_magma++;
        }
    } else if (boss.boss_type == 3) { // Void Ouroboros: Invisibility phase & Vortex
        boss.phase_timer++;
        if (boss.phase_timer >= 40) {
            boss.invisible = !boss.invisible;
            boss.phase_timer = 0;
        }
        // Vortex pull
        if (match_ticks % 4 == 0) {
            int bx = boss.body[0].x, by = boss.body[0].y;
            if (food.x < bx) food.x++; else if (food.x > bx) food.x--;
            if (food.y < by) food.y++; else if (food.y > by) food.y--;
        }
    }

    if (boss.max_hp > 0 && boss.hp * 100 / boss.max_hp < 25) {
        if (match_ticks % 2 == 0 && particle_count < 100) {
            particles[particle_count].x = boss.body[0].x * CELL_SIZE + CELL_SIZE/2 + (random_int(20)-10);
            particles[particle_count].y = boss.body[0].y * CELL_SIZE + 45 + CELL_SIZE/2 + (random_int(20)-10);
            particles[particle_count].vx = (random_int(7) - 3);
            particles[particle_count].vy = (random_int(7) - 3);
            particles[particle_count].life = 20;
            int c_rnd = random_int(3);
            particles[particle_count].color = (c_rnd==0)?RGB(255,255,0):(c_rnd==1?RGB(255,100,0):RGB(255,255,255));
            particle_count++;
        }
    }
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
    if (ghost_berry.x != -1) {
        if (ghost_berry.x < px) ghost_berry.x++; else if (ghost_berry.x > px) ghost_berry.x--;
        else if (ghost_berry.y < py) ghost_berry.y++; else if (ghost_berry.y > py) ghost_berry.y--;
    }
}

void DrawSnakeShadowGDI(HDC hdc, int x, int y, int index, int total, int d_x, int d_y) {
    int px = x * CELL_SIZE + 4, py = y * CELL_SIZE + 45 + 4;
    int cx = px + CELL_SIZE / 2, cy = py + CELL_SIZE / 2;

    HBRUSH shadowBrush = CreateSolidBrush(RGB(10, 15, 20));
    HPEN shadowPen = (HPEN)GetStockObject(NULL_PEN);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, shadowBrush);
    HPEN oldPen = (HPEN)SelectObject(hdc, shadowPen);

    if (index == 0) {
        Ellipse(hdc, px - 2, py - 2, px + CELL_SIZE + 2, py + CELL_SIZE + 2);
    } else if (index == total - 1) {
        int prev_x = snake[index - 1].x;
        int prev_y = snake[index - 1].y;
        int dx = x - prev_x;
        int dy = y - prev_y;
        if (dx > 1) dx = -1; else if (dx < -1) dx = 1;
        if (dy > 1) dy = -1; else if (dy < -1) dy = 1;
        POINT pts[3];
        if (dx == -1) { pts[0].x = cx+8; pts[0].y = cy-8; pts[1].x = cx-10; pts[1].y = cy; pts[2].x = cx+8; pts[2].y = cy+8; }
        else if (dx == 1) { pts[0].x = cx-8; pts[0].y = cy-8; pts[1].x = cx+10; pts[1].y = cy; pts[2].x = cx-8; pts[2].y = cy+8; }
        else if (dy == -1) { pts[0].x = cx-8; pts[0].y = cy+8; pts[1].x = cx; pts[1].y = cy-10; pts[2].x = cx+8; pts[2].y = cy+8; }
        else { pts[0].x = cx-8; pts[0].y = cy-8; pts[1].x = cx; pts[1].y = cy+10; pts[2].x = cx+8; pts[2].y = cy-8; }
        Polygon(hdc, pts, 3);
    } else {
        int inset = 1 + index / 12;
        if (inset > 4) inset = 4;
        Ellipse(hdc, px + inset, py + inset, px + CELL_SIZE - inset, py + CELL_SIZE - inset);
    }

    SelectObject(hdc, oldBrush); SelectObject(hdc, oldPen);
    DeleteObject(shadowBrush);
}

void DrawSnakeSegmentGDI(HDC hdc, int x, int y, int index, int total, int is_ghost, int d_x, int d_y) {
    int px = x * CELL_SIZE, py = y * CELL_SIZE + 45;
    int cx = px + CELL_SIZE / 2, cy = py + CELL_SIZE / 2;

    HBRUSH oldBrush; HPEN oldPen;

    if (index == 0) {
        HBRUSH headBrush = CreateSolidBrush(is_ghost ? RGB(0, 210, 211) : (speed_active_timer > 0 ? RGB(230, 126, 34) : RGB(46, 204, 113)));
        HPEN headPen = CreatePen(PS_SOLID, 1, is_ghost ? RGB(0, 180, 200) : RGB(30, 130, 70));
        HBRUSH eyeBrush, pupilBrush, tongueBrush;
        int eye1_x, eye1_y, eye2_x, eye2_y;

        oldBrush = (HBRUSH)SelectObject(hdc, headBrush);
        oldPen = (HPEN)SelectObject(hdc, headPen);

        Ellipse(hdc, px, py, px + CELL_SIZE, py + CELL_SIZE);

        eyeBrush = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, eyeBrush);

        eye1_x = cx + d_y * 3 + d_x * 2;
        eye1_y = cy + d_x * 3 + d_y * 2;
        eye2_x = cx - d_y * 3 + d_x * 2;
        eye2_y = cy - d_x * 3 + d_y * 2;

        Ellipse(hdc, eye1_x - 3, eye1_y - 3, eye1_x + 3, eye1_y + 3);
        Ellipse(hdc, eye2_x - 3, eye2_y - 3, eye2_x + 3, eye2_y + 3);

        pupilBrush = CreateSolidBrush(RGB(30, 39, 46));
        SelectObject(hdc, pupilBrush);
        Ellipse(hdc, eye1_x + d_x - 1, eye1_y + d_y - 1, eye1_x + d_x + 2, eye1_y + d_y + 2);
        Ellipse(hdc, eye2_x + d_x - 1, eye2_y + d_y - 1, eye2_x + d_x + 2, eye2_y + d_y + 2);

        int headSpec = 150 + (ABS((anim_tick * 2) % 20 - 10) * 10);
        HBRUSH headHlBrush = CreateSolidBrush(RGB(headSpec, 255, headSpec + 50));
        SelectObject(hdc, headHlBrush);
        Ellipse(hdc, px + 4, py + 4, px + 8, py + 8);
        DeleteObject(headHlBrush);

        if ((anim_tick % 4) < 2) {
            tongueBrush = CreateSolidBrush(RGB(255, 71, 87));
            SelectObject(hdc, tongueBrush);
            POINT tonguePts[3];
            tonguePts[0].x = cx + d_x * (CELL_SIZE/2);
            tonguePts[0].y = cy + d_y * (CELL_SIZE/2);
            tonguePts[1].x = cx + d_x * (CELL_SIZE/2 + 4) + d_y * 2;
            tonguePts[1].y = cy + d_y * (CELL_SIZE/2 + 4) + d_x * 2;
            tonguePts[2].x = cx + d_x * (CELL_SIZE/2 + 4) - d_y * 2;
            tonguePts[2].y = cy + d_y * (CELL_SIZE/2 + 4) - d_x * 2;
            Polygon(hdc, tonguePts, 3);
            DeleteObject(tongueBrush);
        }

        DeleteObject(eyeBrush); DeleteObject(pupilBrush);
        SelectObject(hdc, oldBrush); SelectObject(hdc, oldPen);
        DeleteObject(headBrush); DeleteObject(headPen);
    } else if (index == total - 1) {
        HBRUSH tailBrush = CreateSolidBrush(is_ghost ? RGB(72, 219, 251) : (speed_active_timer > 0 ? RGB(241, 196, 15) : RGB(39, 174, 96)));
        HPEN tailPen = CreatePen(PS_SOLID, 1, RGB(25, 110, 90));
        oldBrush = (HBRUSH)SelectObject(hdc, tailBrush);
        oldPen = (HPEN)SelectObject(hdc, tailPen);
        
        int prev_x = snake[index - 1].x;
        int prev_y = snake[index - 1].y;
        int dx = x - prev_x;
        int dy = y - prev_y;
        if (dx > 1) dx = -1; else if (dx < -1) dx = 1;
        if (dy > 1) dy = -1; else if (dy < -1) dy = 1;
        
        POINT pts[3];
        if (dx == -1) { pts[0].x = cx+8; pts[0].y = cy-8; pts[1].x = cx-10; pts[1].y = cy; pts[2].x = cx+8; pts[2].y = cy+8; }
        else if (dx == 1) { pts[0].x = cx-8; pts[0].y = cy-8; pts[1].x = cx+10; pts[1].y = cy; pts[2].x = cx-8; pts[2].y = cy+8; }
        else if (dy == -1) { pts[0].x = cx-8; pts[0].y = cy+8; pts[1].x = cx; pts[1].y = cy-10; pts[2].x = cx+8; pts[2].y = cy+8; }
        else { pts[0].x = cx-8; pts[0].y = cy-8; pts[1].x = cx; pts[1].y = cy+10; pts[2].x = cx+8; pts[2].y = cy-8; }
        Polygon(hdc, pts, 3);
        
        SelectObject(hdc, oldBrush); SelectObject(hdc, oldPen);
        DeleteObject(tailBrush); DeleteObject(tailPen);
    } else {
        HBRUSH bodyBrush = CreateSolidBrush(is_ghost ? RGB(72, 219, 251) : (speed_active_timer > 0 ? RGB(241, 196, 15) : (index % 2 == 0 ? RGB(46, 204, 113) : RGB(33, 140, 116))));
        HPEN bodyPen = CreatePen(PS_SOLID, 1, RGB(25, 110, 90));
        int inset = 1 + index / 12;
        if (inset > 4) inset = 4;
        
        oldBrush = (HBRUSH)SelectObject(hdc, bodyBrush);
        oldPen = (HPEN)SelectObject(hdc, bodyPen);

        Ellipse(hdc, px + inset, py + inset, px + CELL_SIZE - inset, py + CELL_SIZE - inset);
        
        HPEN stripePen = CreatePen(PS_SOLID, 1, RGB(20, 90, 70));
        HPEN oldStripe = (HPEN)SelectObject(hdc, stripePen);
        MoveToEx(hdc, px + inset + 4, py + inset + 4, NULL);
        LineTo(hdc, px + CELL_SIZE - inset - 4, py + CELL_SIZE - inset - 4);
        MoveToEx(hdc, px + CELL_SIZE - inset - 4, py + inset + 4, NULL);
        LineTo(hdc, px + inset + 4, py + CELL_SIZE - inset - 4);
        SelectObject(hdc, oldStripe);
        DeleteObject(stripePen);

        int specIntensity = 100 + (ABS((anim_tick + index * 5) % 20 - 10) * 10);
        HBRUSH hlBrush = CreateSolidBrush(RGB(specIntensity, 255, specIntensity + 50));
        SelectObject(hdc, hlBrush);
        Ellipse(hdc, px + inset + 2, py + inset + 2, px + inset + 6, py + inset + 6);
        DeleteObject(hlBrush);

        SelectObject(hdc, oldBrush); SelectObject(hdc, oldPen);
        DeleteObject(bodyBrush); DeleteObject(bodyPen);
    }
}

void DrawRivalGDI(HDC hdc, int x, int y, int index, int type) {
    int px = x * CELL_SIZE, py = y * CELL_SIZE + 45;
    int is_aggro = (type == 1);
    HBRUSH brush = CreateSolidBrush(is_aggro ? (index == 0 ? RGB(192, 57, 43) : RGB(231, 76, 60)) : (index == 0 ? RGB(44, 62, 80) : RGB(52, 73, 94)));
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(20, 30, 40));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    if (index == 0 && is_aggro) {
        POINT pts[3];
        pts[0].x = px + CELL_SIZE/2; pts[0].y = py + 2;
        pts[1].x = px + CELL_SIZE - 2; pts[1].y = py + CELL_SIZE - 2;
        pts[2].x = px + 2; pts[2].y = py + CELL_SIZE - 2;
        Polygon(hdc, pts, 3);
    } else {
        Ellipse(hdc, px + 1, py + 1, px + CELL_SIZE - 1, py + CELL_SIZE - 1);
    }
    
    if (index == 0) {
        HBRUSH eyeBrush = CreateSolidBrush(RGB(231, 76, 60));
        SelectObject(hdc, eyeBrush);
        if (is_aggro) {
            Ellipse(hdc, px+6, py+8, px+10, py+12);
            Ellipse(hdc, px+CELL_SIZE-10, py+8, px+CELL_SIZE-6, py+12);
        } else {
            Ellipse(hdc, px+4, py+4, px+8, py+8);
            Ellipse(hdc, px+CELL_SIZE-8, py+4, px+CELL_SIZE-4, py+8);
        }
        DeleteObject(eyeBrush);
    }
    
    SelectObject(hdc, oldBrush); SelectObject(hdc, oldPen);
    DeleteObject(brush); DeleteObject(pen);
}

void DrawBossGDI(HDC hdc, int x, int y, int index) {
    if (boss.invisible && (anim_tick % 4 < 2)) return; // Void phasing
    int px = x * CELL_SIZE, py = y * CELL_SIZE + 45;
    COLORREF bColor;
    if (boss.boss_type == 0) bColor = (index == 0 ? RGB(45, 52, 54) : RGB(99, 110, 114));
    else if (boss.boss_type == 1) bColor = (index == 0 ? RGB(0, 180, 216) : RGB(144, 224, 239)); // Cyber Basilisk
    else if (boss.boss_type == 2) bColor = (index == 0 ? RGB(230, 57, 70) : RGB(244, 162, 97)); // Inferno Wyrm
    else bColor = (index == 0 ? RGB(114, 9, 183) : RGB(181, 23, 158)); // Void Ouroboros

    HBRUSH brush = CreateSolidBrush(bColor);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    Rectangle(hdc, px, py, px + CELL_SIZE, py + CELL_SIZE);
    
    if (index == 0) {
        HBRUSH eyeBrush = CreateSolidBrush(RGB(255, 255, 0));
        SelectObject(hdc, eyeBrush);
        Ellipse(hdc, px+4, py+4, px+CELL_SIZE-4, py+CELL_SIZE-4);
        DeleteObject(eyeBrush);
    }

    SelectObject(hdc, oldBrush); SelectObject(hdc, oldPen);
    DeleteObject(brush); DeleteObject(pen);
}

void DrawGemGDI(HDC hdc, int x, int y, COLORREF color) {
    int px = x * CELL_SIZE, py = y * CELL_SIZE + 45;
    HBRUSH brush = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    if (color == RGB(255, 215, 0)) { // Star
        POINT pts[10];
        int cx = px + CELL_SIZE/2, cy = py + CELL_SIZE/2;
        int i;
        int star_x[10] = { 0, 3, 11, 5, 7, 0, -7, -5, -11, -3 };
        int star_y[10] = { -12, -4, -4, 1, 9, 4, 9, 1, -4, -4 };
        for (i = 0; i < 10; i++) {
            pts[i].x = cx + star_x[i];
            pts[i].y = cy + star_y[i];
        }
        Polygon(hdc, pts, 10);
    } else {
        Ellipse(hdc, px + 2, py + 2, px + CELL_SIZE - 2, py + CELL_SIZE - 2);
        HBRUSH shine = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, shine);
        Ellipse(hdc, px+4, py+4, px+6, py+6);
        DeleteObject(shine);
        
        if (color == RGB(255, 71, 87)) {
            HPEN stem = CreatePen(PS_SOLID, 2, RGB(116, 81, 45));
            SelectObject(hdc, stem);
            MoveToEx(hdc, px+CELL_SIZE/2, py+2, NULL);
            LineTo(hdc, px+CELL_SIZE/2, py-2);
            DeleteObject(stem);
        }
    }

    SelectObject(hdc, oldBrush); SelectObject(hdc, oldPen);
    DeleteObject(brush); DeleteObject(pen);
}

void DrawPortalGDI(HDC hdc, int x, int y, COLORREF color) {
    int px = x * CELL_SIZE, py = y * CELL_SIZE + 45;
    int cx = px + CELL_SIZE/2, cy = py + CELL_SIZE/2;
    HBRUSH brush = CreateSolidBrush(RGB(20, 0, 30));
    HPEN pen = CreatePen(PS_SOLID, 2, color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    POINT pts[16];
    int i;
    for (i = 0; i < 16; i++) {
        int jitter = ((anim_tick * 3 + i * 7) % 11) - 5; 
        int r = (CELL_SIZE/2 - 2) + jitter;
        if (i % 2 == 0) r -= 4;
        pts[i].x = cx + (cos_tab16[i] * r) / 100;
        pts[i].y = cy + (sin_tab16[i] * r) / 100;
    }
    Polygon(hdc, pts, 16);

    HBRUSH voidBrush = CreateSolidBrush(RGB(0, 0, 0));
    SelectObject(hdc, voidBrush);
    POINT pts_inner[16];
    for (i = 0; i < 16; i++) {
        int jitter = ((anim_tick * 5 + i * 13) % 7) - 3; 
        int r = (CELL_SIZE/4) + jitter;
        if (i % 2 == 0) r -= 2;
        pts_inner[i].x = cx + (cos_tab16[i] * r) / 100;
        pts_inner[i].y = cy + (sin_tab16[i] * r) / 100;
    }
    Polygon(hdc, pts_inner, 16);
    DeleteObject(voidBrush);

    SelectObject(hdc, oldBrush); SelectObject(hdc, oldPen);
    DeleteObject(brush); DeleteObject(pen);
}

void DrawObstacleGDI(HDC hdc, int x, int y) {
    int px = x * CELL_SIZE, py = y * CELL_SIZE + 45;
    RECT r = { px, py, px + CELL_SIZE, py + CELL_SIZE };
    HBRUSH bgBrush = CreateSolidBrush(RGB(87, 101, 116));
    FillRect(hdc, &r, bgBrush);
    
    RECT hlt = { px, py, px + CELL_SIZE, py + 2 };
    HBRUSH hlBrush = CreateSolidBrush(RGB(131, 149, 167));
    FillRect(hdc, &hlt, hlBrush);
    RECT hll = { px, py, px + 2, py + CELL_SIZE };
    FillRect(hdc, &hll, hlBrush);
    
    RECT sdt = { px + CELL_SIZE - 2, py, px + CELL_SIZE, py + CELL_SIZE };
    HBRUSH sdBrush = CreateSolidBrush(RGB(34, 47, 62));
    FillRect(hdc, &sdt, sdBrush);
    RECT sdb = { px, py + CELL_SIZE - 2, px + CELL_SIZE, py + CELL_SIZE };
    FillRect(hdc, &sdb, sdBrush);
    
    HPEN crackPen = CreatePen(PS_SOLID, 1, RGB(34, 47, 62));
    HPEN oldPen = (HPEN)SelectObject(hdc, crackPen);
    MoveToEx(hdc, px+3, py+3, NULL);
    LineTo(hdc, px+7, py+7);
    LineTo(hdc, px+5, py+11);
    SelectObject(hdc, oldPen);
    DeleteObject(crackPen);
    
    DeleteObject(bgBrush); DeleteObject(hlBrush); DeleteObject(sdBrush);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            LoadStats();
            LoadConfig();
            LoadCustomMap();
            break;

        case WM_LBUTTONDOWN: {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            HDC hdc = GetDC(hwnd);
            int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
            ReleaseDC(hwnd, hdc);
            float scale = dpi / 96.0f;
            int gx = (int)(mx / (scale * CELL_SIZE));
            int gy = (int)((my / scale - 45) / CELL_SIZE);
            if (game_state == 8) { // Map Editor
                if (gx >= 0 && gx < GRID_WIDTH && gy >= 0 && gy < GRID_HEIGHT) {
                    editor_cursor_x = gx; editor_cursor_y = gy;
                    if (editor_brush == 0) { // Toggle Wall
                        int i, found = -1;
                        for(i=0; i<num_custom_obstacles; i++) {
                            if (custom_obstacles[i].x == gx && custom_obstacles[i].y == gy) { found = i; break; }
                        }
                        if (found >= 0) {
                            custom_obstacles[found] = custom_obstacles[num_custom_obstacles-1];
                            num_custom_obstacles--;
                        } else if (num_custom_obstacles < 120) {
                            custom_obstacles[num_custom_obstacles].x = gx;
                            custom_obstacles[num_custom_obstacles].y = gy;
                            num_custom_obstacles++;
                        }
                    } else if (editor_brush == 1) {
                        custom_portal_a.x = gx; custom_portal_a.y = gy; custom_portal_active = 1;
                    } else if (editor_brush == 2) {
                        custom_portal_b.x = gx; custom_portal_b.y = gy; custom_portal_active = 1;
                    } else if (editor_brush == 3) {
                        int i;
                        for(i=0; i<num_custom_obstacles; i++) {
                            if (custom_obstacles[i].x == gx && custom_obstacles[i].y == gy) {
                                custom_obstacles[i] = custom_obstacles[num_custom_obstacles-1];
                                num_custom_obstacles--;
                                break;
                            }
                        }
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            break;
        }

        case WM_TIMER: {
            int i, r, effective_wrap, gain, spd;
            if (game_state == 1 || game_state == 2) {
                if (screen_shake_timer > 0) { screen_shake_timer--; InvalidateRect(hwnd, NULL, FALSE); }
                if (shockwave_timer > 0) { shockwave_timer--; InvalidateRect(hwnd, NULL, FALSE); }
                UpdateParticles();
                UpdateShockwaves();
                if (particle_count > 0 || num_shockwaves > 0) InvalidateRect(hwnd, NULL, FALSE);
            }
            if (boss_banner_timer > 0) boss_banner_timer--;
            if (game_state != 1) break;
            anim_tick++;
            match_ticks++;
            if (!grid_coverage[snake[0].x][snake[0].y]) {
                grid_coverage[snake[0].x][snake[0].y] = 1;
                grid_coverage_count++;
            }

            // Emit speed dust trail motes
            if (snake_len > 0 && particle_count < MAX_PARTICLES) {
                particles[particle_count].x = snake[snake_len - 1].x * CELL_SIZE + CELL_SIZE/2 + (random_int(5) - 2);
                particles[particle_count].y = snake[snake_len - 1].y * CELL_SIZE + 45 + CELL_SIZE/2 + (random_int(5) - 2);
                particles[particle_count].vx = 0;
                particles[particle_count].vy = 0;
                particles[particle_count].life = 6 + random_int(4);
                particles[particle_count].max_life = particles[particle_count].life;
                particles[particle_count].color = (ghost_active > 0) ? RGB(0, 210, 211) : (speed_active_timer > 0 ? RGB(230, 126, 34) : RGB(46, 204, 113));
                particles[particle_count].type = 1;
                particles[particle_count].size = 2;
                particles[particle_count].rot = 0;
                particles[particle_count].vrot = 0;
                particle_count++;
            }

            if (is_replay_mode) {
                while(replay_playback_idx < replay_event_count && replay_events[replay_playback_idx].tick == match_ticks) {
                    char a = replay_events[replay_playback_idx].action;
                    if (a == 'U' && last_dir_y != 1) { dir_x = 0; dir_y = -1; }
                    if (a == 'D' && last_dir_y != -1) { dir_x = 0; dir_y = 1; }
                    if (a == 'L' && last_dir_x != 1) { dir_x = -1; dir_y = 0; }
                    if (a == 'R' && last_dir_x != -1) { dir_x = 1; dir_y = 0; }
                    if (a == 'G') { ghost_active = 50; ghost_cd = 150; }
                    if (a == 'F') { freeze_active = 100; freeze_cd = 200; }
                    if (a == 'M') { magnet_active = 80; magnet_cd = 150; }
                    replay_playback_idx++;
                }
            }

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

            // Magma Hazards decay
            for(i=0; i<num_magma; i++) {
                magma_timers[i]--;
                if (magma_timers[i] <= 0) {
                    magma_hazards[i] = magma_hazards[num_magma-1];
                    magma_timers[i] = magma_timers[num_magma-1];
                    num_magma--;
                    i--;
                }
            }

            if (golden_apple.x != -1 && match_ticks % 2 == 0) {
                if (particle_count < 500) {
                    particles[particle_count].x = golden_apple.x * CELL_SIZE + CELL_SIZE/2 + (random_int(20)-10);
                    particles[particle_count].y = golden_apple.y * CELL_SIZE + 45 + CELL_SIZE/2 + (random_int(20)-10);
                    particles[particle_count].vx = (random_int(5) - 2);
                    particles[particle_count].vy = (random_int(5) - 2);
                    particles[particle_count].life = 15;
                    int c_rnd = random_int(4);
                    if (c_rnd == 0) particles[particle_count].color = RGB(255, 215, 0);
                    else if (c_rnd == 1) particles[particle_count].color = RGB(255, 235, 59);
                    else if (c_rnd == 2) particles[particle_count].color = RGB(255, 255, 255);
                    else particles[particle_count].color = RGB(255, 152, 0);
                    particle_count++;
                }
            }
            {
                int pc = 0;
                for(i=0; i<particle_count; i++) {
                    particles[i].x += particles[i].vx;
                    particles[i].y += particles[i].vy;
                    particles[i].life--;
                    if(particles[i].life > 0) particles[pc++] = particles[i];
                }
                particle_count = pc;
            }
            
            // Dynamic growing maze walls
            if (game_mode == 1 && match_ticks % 100 == 0 && num_obstacles < 99) {
                int ox, oy, ok = 0;
                while(!ok) {
                    ox = random_int(GRID_WIDTH); oy = random_int(GRID_HEIGHT);
                    ok = 1;
                    if (oy == 5 && (ox >= 2 && ox <= 6)) ok = 0;
                    if (snake[0].x == ox && snake[0].y == oy) ok = 0;
                    if (food.x == ox && food.y == oy) ok = 0;
                }
                obstacles[num_obstacles].x = ox;
                obstacles[num_obstacles].y = oy;
                num_obstacles++;
            }

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

            int was_playing = (game_state == 1);
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
                if (boss.alive && !boss.invisible) {
                    for(i = 0; i < boss.len; i++) {
                        if (snake[0].x == boss.body[i].x && snake[0].y == boss.body[i].y) game_state = 2;
                    }
                }
                for(i = 0; i < num_oil_slicks; i++) {
                    if (snake[0].x == oil_slicks[i].x && snake[0].y == oil_slicks[i].y) {
                        MessageBeep(MB_ICONHAND);
                        score -= 50; if (score < 0) score = 0;
                        poison_active_timer = 50;
                        screen_shake_timer = 15;
                        shockwave_timer = 30;
                        shockwave_x = snake[0].x; shockwave_y = snake[0].y;
                        oil_slicks[i] = oil_slicks[num_oil_slicks-1];
                        num_oil_slicks--;
                        i--;
                    }
                }
                for(i = 0; i < num_magma; i++) {
                    if (snake[0].x == magma_hazards[i].x && snake[0].y == magma_hazards[i].y) {
                        MessageBeep(MB_ICONHAND);
                        score -= 100; if (score < 0) score = 0;
                        screen_shake_timer = 20;
                        shockwave_timer = 20;
                        shockwave_x = snake[0].x; shockwave_y = snake[0].y;
                        SpawnExplosion(snake[0].x * CELL_SIZE + CELL_SIZE/2, snake[0].y * CELL_SIZE + 45 + CELL_SIZE/2, RGB(230, 57, 70), 0);
                    }
                }
            }

            if (game_state == 2 && was_playing) {
                screen_shake_timer = 30;
                shockwave_timer = 30;
                shockwave_x = snake[0].x;
                shockwave_y = snake[0].y;
                SpawnExplosion(shockwave_x * CELL_SIZE + CELL_SIZE/2, shockwave_y * CELL_SIZE + 45 + CELL_SIZE/2, RGB(255, 71, 87), 1);
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
                SpawnExplosion(food.x * CELL_SIZE + CELL_SIZE/2, food.y * CELL_SIZE + 45 + CELL_SIZE/2, RGB(255, 71, 87), 0);
                if (snake_len < 400) { snake[snake_len] = snake[snake_len-1]; snake_len++; }

                gain = score_mult;
                if (game_mode == 4 && campaign_branch == 1) gain = (gain * 5) / 2; // Shadow Path bonus
                score += gain;
                total_apples++;
                apples_eaten++;
                match_apples_gained++;

                SaveStats();

                if (boss.alive) {
                    boss.hp -= 1;
                    if (boss.hp <= 0) {
                        boss.alive = 0;
                        SpawnExplosion(boss.body[0].x * CELL_SIZE + CELL_SIZE/2, boss.body[0].y * CELL_SIZE + 45 + CELL_SIZE/2, RGB(255, 215, 0), 1);
                        if (game_mode == 7) { // Gauntlet
                            if (gauntlet_stage < 3) {
                                gauntlet_stage++;
                                score += 1000 * gauntlet_stage;
                                InitGauntletStage(gauntlet_stage);
                                PlaceGoldenApple();
                            } else {
                                score += 5000;
                                game_state = 4; // GRAND GAUNTLET VICTORY!
                            }
                        } else if (game_mode == 4 && campaign_level == 30) {
                            score += 10000;
                            game_state = 4; // CAMPAIGN CONQUERED!
                        }
                    }
                }

                if (game_mode == 4 && apples_eaten >= 8 && campaign_level < 30) {
                    if (campaign_level == 5 || campaign_level == 10 || campaign_level == 15 || campaign_level == 20) {
                        // Trigger Branch Choice Checkpoint!
                        game_state = 9;
                        pending_branch_choice = campaign_level;
                    } else {
                        campaign_level++;
                        InitCampaignStage(campaign_level);
                        apples_eaten = 0;
                    }
                }

                if (game_mode == 2 && current_speed > 35) current_speed -= 5;
                else if (current_speed > 40) current_speed -= 2;

                PlaceFood();
                
                if (golden_apple.x == -1 && random_int(100) < 20) PlaceGoldenApple();
                if (poison_berry.x == -1 && random_int(100) < 20) PlacePoisonBerry();
                if (speed_berry.x == -1 && random_int(100) < 20) PlaceSpeedBerry();
                if (ghost_berry.x == -1 && random_int(100) < 20) PlaceGhostBerry();
            }

            if (golden_apple.x != -1 && snake[0].x == golden_apple.x && snake[0].y == golden_apple.y) {
                MessageBeep(MB_ICONASTERISK);
                SpawnExplosion(golden_apple.x * CELL_SIZE + CELL_SIZE/2, golden_apple.y * CELL_SIZE + 45 + CELL_SIZE/2, RGB(255, 215, 0), 1);
                score += 500; snake_len -= 2; if (snake_len < 3) snake_len = 3;
                if (boss.alive) {
                    boss.hp -= 3;
                    if (boss.hp <= 0) {
                        boss.alive = 0;
                        SpawnExplosion(boss.body[0].x * CELL_SIZE + CELL_SIZE/2, boss.body[0].y * CELL_SIZE + 45 + CELL_SIZE/2, RGB(255, 215, 0), 1);
                        if (game_mode == 7) {
                            if (gauntlet_stage < 3) {
                                gauntlet_stage++;
                                score += 1000 * gauntlet_stage;
                                InitGauntletStage(gauntlet_stage);
                            } else {
                                game_state = 4;
                            }
                        } else if (game_mode == 4 && campaign_level == 30) {
                            game_state = 4;
                        }
                    }
                }
                golden_apple.x = -1;
            }

            if (poison_berry.x != -1 && snake[0].x == poison_berry.x && snake[0].y == poison_berry.y) {
                MessageBeep(MB_ICONHAND);
                SpawnExplosion(poison_berry.x * CELL_SIZE + CELL_SIZE/2, poison_berry.y * CELL_SIZE + 45 + CELL_SIZE/2, RGB(142, 68, 173), 1);
                score -= 200; if (score < 0) score = 0;
                poison_active_timer = 50;
                screen_shake_timer = 30;
                shockwave_timer = 30;
                shockwave_x = snake[0].x; shockwave_y = snake[0].y;
                poison_berry.x = -1;
            }

            if (speed_berry.x != -1 && snake[0].x == speed_berry.x && snake[0].y == speed_berry.y) {
                MessageBeep(MB_ICONASTERISK);
                SpawnExplosion(speed_berry.x * CELL_SIZE + CELL_SIZE/2, speed_berry.y * CELL_SIZE + 45 + CELL_SIZE/2, RGB(230, 126, 34), 1);
                score += 300;
                speed_active_timer = 50;
                speed_berry.x = -1;
            }
            if (ghost_berry.x != -1 && snake[0].x == ghost_berry.x && snake[0].y == ghost_berry.y) {
                MessageBeep(MB_ICONASTERISK);
                SpawnExplosion(ghost_berry.x * CELL_SIZE + CELL_SIZE/2, ghost_berry.y * CELL_SIZE + 45 + CELL_SIZE/2, RGB(0, 210, 211), 1);
                score += 300;
                ghost_active = 150;
                ghost_berry.x = -1;
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
                    lstrcpyA(entry.date, "2026-08-31");

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
                if (wParam == 'M') { game_mode = (game_mode + 1) % NUM_MODES; }
                else if (wParam == '1') difficulty = 0;
                else if (wParam == '2') difficulty = 1;
                else if (wParam == '3') difficulty = 2;
                else if (wParam == 'W') wrap_mode = !wrap_mode;
                else if (wParam == 'H' || wParam == VK_F1) game_state = 5;
                else if (wParam == 'R') {
                    if (RestoreGameState()) {
                        game_state = 1; SetTimer(hwnd, TIMER_ID, current_speed, NULL);
                    }
                }
                else if (wParam == 'E' || wParam == 'O') { game_state = 8; InvalidateRect(hwnd, NULL, TRUE); }
                else if (wParam == 'I') ImportStatsText();
                else if (wParam == 'C') { game_state = 6; config_step = 0; InvalidateRect(hwnd, NULL, TRUE); }
                else if (wParam == 'X') { ImportReplay(); }
                else if (wParam == 'S') { game_state = 7; InvalidateRect(hwnd, NULL, TRUE); }
                else if (wParam == VK_ESCAPE) { PostMessage(hwnd, WM_CLOSE, 0, 0); }
                else if (wParam == VK_RETURN) {
                    is_replay_mode = 0;
                    InitGame(); SetTimer(hwnd, TIMER_ID, current_speed, NULL);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (game_state == 8) { // Map Editor
                if (wParam == VK_UP || wParam == 'W') { if (editor_cursor_y > 0) editor_cursor_y--; }
                else if (wParam == VK_DOWN || wParam == 'S') { if (editor_cursor_y < GRID_HEIGHT - 1) editor_cursor_y++; }
                else if (wParam == VK_LEFT || wParam == 'A') { if (editor_cursor_x > 0) editor_cursor_x--; }
                else if (wParam == VK_RIGHT || wParam == 'D') { if (editor_cursor_x < GRID_WIDTH - 1) editor_cursor_x++; }
                else if (wParam == VK_SPACE) {
                    if (editor_brush == 0) {
                        int i, found = -1;
                        for(i=0; i<num_custom_obstacles; i++) {
                            if (custom_obstacles[i].x == editor_cursor_x && custom_obstacles[i].y == editor_cursor_y) { found = i; break; }
                        }
                        if (found >= 0) {
                            custom_obstacles[found] = custom_obstacles[num_custom_obstacles-1];
                            num_custom_obstacles--;
                        } else if (num_custom_obstacles < 120) {
                            custom_obstacles[num_custom_obstacles].x = editor_cursor_x;
                            custom_obstacles[num_custom_obstacles].y = editor_cursor_y;
                            num_custom_obstacles++;
                        }
                    } else if (editor_brush == 1) {
                        custom_portal_a.x = editor_cursor_x; custom_portal_a.y = editor_cursor_y; custom_portal_active = 1;
                    } else if (editor_brush == 2) {
                        custom_portal_b.x = editor_cursor_x; custom_portal_b.y = editor_cursor_y; custom_portal_active = 1;
                    }
                }
                else if (wParam == '1') editor_brush = 0; // Wall
                else if (wParam == '2') editor_brush = 1; // Portal A
                else if (wParam == '3') editor_brush = 2; // Portal B
                else if (wParam == '4') editor_brush = 3; // Erase
                else if (wParam == 'C') ClearCustomMap();
                else if (wParam == 'B') { // Border
                    int x, y;
                    for(x=0; x<GRID_WIDTH; x++) {
                        if (num_custom_obstacles < 118) {
                            custom_obstacles[num_custom_obstacles].x = x; custom_obstacles[num_custom_obstacles].y = 0; num_custom_obstacles++;
                            custom_obstacles[num_custom_obstacles].x = x; custom_obstacles[num_custom_obstacles].y = GRID_HEIGHT-1; num_custom_obstacles++;
                        }
                    }
                }
                else if (wParam == 'R') GenerateRandomMazeToCustom();
                else if (wParam == 'S') SaveCustomMap();
                else if (wParam == 'L') LoadCustomMap();
                else if (wParam == 'T' || wParam == VK_RETURN) {
                    game_mode = 6; // Custom map mode
                    InitGame();
                    SetTimer(hwnd, TIMER_ID, current_speed, NULL);
                }
                else if (wParam == VK_ESCAPE) { game_state = 0; }
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (game_state == 9) { // Branch Choice Checkpoint
                if (wParam == '1') {
                    campaign_branch = 0; // Solar Path
                    campaign_level++;
                    InitCampaignStage(campaign_level);
                    apples_eaten = 0;
                    game_state = 1;
                } else if (wParam == '2') {
                    campaign_branch = 1; // Shadow Path
                    campaign_level++;
                    InitCampaignStage(campaign_level);
                    apples_eaten = 0;
                    game_state = 1;
                }
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (game_state == 6) { // Config
                if (wParam == VK_ESCAPE) { game_state = 0; }
                else if (config_step == 0) bind_up = wParam;
                else if (config_step == 1) bind_down = wParam;
                else if (config_step == 2) bind_left = wParam;
                else if (config_step == 3) bind_right = wParam;
                else if (config_step == 4) bind_ghost = wParam;
                else if (config_step == 5) bind_freeze = wParam;
                else if (config_step == 6) bind_mag = wParam;
                else if (config_step == 7) { bind_pause = wParam; SaveConfig(); game_state = 0; }
                if (game_state == 6) config_step++;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (game_state == 7) { // Stats Export
                if (wParam == 'C') ExportMatchStatsCSV();
                else if (wParam == 'R') ExportReplay();
                else if (wParam == VK_ESCAPE || wParam == VK_RETURN || wParam == 'S') game_state = 0;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (game_state == 5) {
                if (wParam == VK_RETURN || wParam == VK_ESCAPE || wParam == 'H' || wParam == VK_F1) { game_state = 0; InvalidateRect(hwnd, NULL, TRUE); }
            } else if (game_state == 3) {
                if (wParam == bind_pause || wParam == 'P' || wParam == VK_ESCAPE) { game_state = 1; SetTimer(hwnd, TIMER_ID, current_speed, NULL); }
                else if (wParam == 'Q' || wParam == 'S') { SaveGameState(); game_state = 0; InvalidateRect(hwnd, NULL, TRUE); }
            } else if (game_state == 2 || game_state == 4) {
                if ((wParam == VK_RETURN || wParam == VK_ESCAPE) && !is_high_score_entry) { game_state = 0; InvalidateRect(hwnd, NULL, TRUE); }
            } else if (game_state == 1) { // Playing
                if (!is_replay_mode) {
                    char a = 0;
                    if ((wParam == VK_UP || wParam == bind_up) && last_dir_y != 1) { dir_x = 0; dir_y = -1; a = 'U'; }
                    else if ((wParam == VK_DOWN || wParam == bind_down) && last_dir_y != -1) { dir_x = 0; dir_y = 1; a = 'D'; }
                    else if ((wParam == VK_LEFT || wParam == bind_left) && last_dir_x != 1) { dir_x = -1; dir_y = 0; a = 'L'; }
                    else if ((wParam == VK_RIGHT || wParam == bind_right) && last_dir_x != -1) { dir_x = 1; dir_y = 0; a = 'R'; }
                    else if (wParam == bind_ghost && ghost_cd == 0) { ghost_active = 50; ghost_cd = 150; a = 'G'; }
                    else if (wParam == bind_freeze && freeze_cd == 0) { freeze_active = 100; freeze_cd = 200; a = 'F'; }
                    else if (wParam == bind_mag && magnet_cd == 0) { magnet_active = 80; magnet_cd = 150; a = 'M'; }
                    
                    if (a != 0 && replay_event_count < 30000) {
                        replay_events[replay_event_count].tick = match_ticks + 1;
                        replay_events[replay_event_count].action = a;
                        replay_event_count++;
                    }
                }
                if (wParam == bind_pause || wParam == 'P' || wParam == VK_ESCAPE) { game_state = 3; KillTimer(hwnd, TIMER_ID); InvalidateRect(hwnd, NULL, TRUE); }
                else if (wParam == VK_F1 || wParam == 'H') { game_state = 5; KillTimer(hwnd, TIMER_ID); InvalidateRect(hwnd, NULL, TRUE); }
            }
            break;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_PAINT: {
            PAINTSTRUCT ps; HDC paintDC = BeginPaint(hwnd, &ps);
            int dpi = GetDeviceCaps(paintDC, LOGPIXELSX);
            float scale = dpi / 96.0f;
            
            HDC hdc = CreateCompatibleDC(paintDC);
            HBITMAP hbm = CreateCompatibleBitmap(paintDC, 520, 620);
            HBITMAP oldBmp = (HBITMAP)SelectObject(hdc, hbm);

            float dx = 0.0f, dy = 0.0f;
            if (screen_shake_timer > 0) {
                dx = (FastSin(anim_tick * 3) * screen_shake_timer * 12) / 1000.0f;
                dy = (FastCos(anim_tick * 4) * screen_shake_timer * 12) / 1000.0f;
            }
            SetWindowOrgEx(hdc, -(int)dx, -(int)dy, NULL);

            RECT logicalRect = {0, 0, 520, 620};
            HBRUSH bg = CreateSolidBrush(RGB(15, 15, 26)); FillRect(hdc, &logicalRect, bg); DeleteObject(bg);

            HFONT hFont = CreateFontA(-20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            HFONT hFontSmall = CreateFontA(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 255, 255));

            if (game_state == 0) { // MENU
                char buf[64];
                SetTextColor(hdc, RGB(0, 210, 211));
                TextOutA(hdc, 160, 15, "KSNAKE ARCADE", 13);

                SelectObject(hdc, hFontSmall);
                SetTextColor(hdc, RGB(255, 255, 255));
                wsprintfA(buf, "M - Mode: %s", mode_names[game_mode]); TextOutA(hdc, 110, 50, buf, lstrlenA(buf));
                wsprintfA(buf, "1-3 - Difficulty: %s", difficulty==0?"Easy":difficulty==1?"Med":"Hard"); TextOutA(hdc, 110, 75, buf, lstrlenA(buf));
                wsprintfA(buf, "W - Toggle Wrap: %s", (wrap_mode||game_mode==3)?"ON":"OFF"); TextOutA(hdc, 110, 100, buf, lstrlenA(buf));
                TextOutA(hdc, 110, 125, "E - Custom Map Editor", 21);
                TextOutA(hdc, 110, 150, "C - Config Keys", 15);
                TextOutA(hdc, 110, 175, "S - Match Stats (Last)", 22);
                TextOutA(hdc, 110, 200, "X - Play Replay (.ksr)", 22);
                TextOutA(hdc, 110, 225, "R - Resume Saved Game", 21);
                TextOutA(hdc, 110, 250, "Move: WASD/Arrows | Skills: G,F,M | Pause: P/ESC", 48);

                SetTextColor(hdc, RGB(72, 219, 251));
                TextOutA(hdc, 110, 280, "[F1 / H] Full Help & Scores", 27);

                SelectObject(hdc, hFont);
                SetTextColor(hdc, RGB(76, 209, 55));
                TextOutA(hdc, 120, 320, "[ Press ENTER to Play ]", 23);

                SelectObject(hdc, hFontSmall);
                SetTextColor(hdc, RGB(251, 197, 49));
                wsprintfA(buf, "Top Record: %s - %d pts (%s)", leaderboard[0].name, leaderboard[0].score, leaderboard[0].mode);
                TextOutA(hdc, 90, 370, buf, lstrlenA(buf));

                SetTextColor(hdc, RGB(164, 176, 190));
                TextOutA(hdc, 150, 410, "[ESC] Exit Game", 15);
            } else if (game_state == 8) { // MAP EDITOR
                int gx, gy, i;
                char buf[128];
                SetTextColor(hdc, RGB(251, 197, 49));
                TextOutA(hdc, 160, 10, "MAP EDITOR LAB", 14);
                SetTextColor(hdc, RGB(200, 200, 220));
                
                // Draw Grid
                for (gx = 0; gx < GRID_WIDTH; gx++) {
                    for (gy = 0; gy < GRID_HEIGHT; gy++) {
                        RECT tile = { gx * CELL_SIZE, gy * CELL_SIZE + 45, (gx + 1) * CELL_SIZE, (gy + 1) * CELL_SIZE + 45 };
                        HBRUSH bBrush = CreateSolidBrush((gx + gy) % 2 == 0 ? RGB(20, 24, 38) : RGB(25, 30, 48));
                        FillRect(hdc, &tile, bBrush);
                        DeleteObject(bBrush);
                    }
                }
                // Draw Custom Walls
                for(i=0; i<num_custom_obstacles; i++) DrawObstacleGDI(hdc, custom_obstacles[i].x, custom_obstacles[i].y);
                // Draw Portals
                if (custom_portal_active) {
                    if (custom_portal_a.x >= 0) DrawPortalGDI(hdc, custom_portal_a.x, custom_portal_a.y, RGB(0, 210, 211));
                    if (custom_portal_b.x >= 0) DrawPortalGDI(hdc, custom_portal_b.x, custom_portal_b.y, RGB(155, 89, 182));
                }
                // Draw Cursor
                {
                    RECT curRect = { editor_cursor_x * CELL_SIZE, editor_cursor_y * CELL_SIZE + 45, (editor_cursor_x + 1) * CELL_SIZE, (editor_cursor_y + 1) * CELL_SIZE + 45 };
                    HPEN curPen = CreatePen(PS_SOLID, 2, RGB(255, 215, 0));
                    HPEN oldP = (HPEN)SelectObject(hdc, curPen);
                    HBRUSH oldB = (HBRUSH)SelectObject(hdc, (HBRUSH)GetStockObject(HOLLOW_BRUSH));
                    Rectangle(hdc, curRect.left, curRect.top, curRect.right, curRect.bottom);
                    SelectObject(hdc, oldP); SelectObject(hdc, oldB);
                    DeleteObject(curPen);
                }
                // Editor Controls HUD
                SelectObject(hdc, hFontSmall);
                wsprintfA(buf, "Tool: %s | [1]Wall [2]Portal A [3]Portal B [4]Erase", 
                    editor_brush==0?"Wall":(editor_brush==1?"Portal A":(editor_brush==2?"Portal B":"Erase")));
                SetTextColor(hdc, RGB(72, 219, 251));
                TextOutA(hdc, 10, 550, buf, lstrlenA(buf));
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, 10, 570, "[SPACE/Click] Draw  |  [C] Clear  |  [B] Border  |  [R] Maze", 60);
                TextOutA(hdc, 10, 590, "[T/ENTER] Play  |  [S] Save  |  [ESC] Menu", 42);
            } else if (game_state == 9) { // BRANCH SELECTION OVERLAY
                SetTextColor(hdc, RGB(251, 197, 49));
                TextOutA(hdc, 120, 80, "CHOOSE CAMPAIGN ROUTE", 21);
                
                // Route A Card: Solar Highway
                RECT cardA = { 40, 130, 230, 380 };
                HBRUSH bA = CreateSolidBrush(RGB(24, 32, 54)); FillRect(hdc, &cardA, bA); DeleteObject(bA);
                HPEN pA = CreatePen(PS_SOLID, 2, RGB(0, 210, 211)); HPEN opA = (HPEN)SelectObject(hdc, pA);
                SelectObject(hdc, (HBRUSH)GetStockObject(HOLLOW_BRUSH));
                Rectangle(hdc, cardA.left, cardA.top, cardA.right, cardA.bottom);
                SelectObject(hdc, opA); DeleteObject(pA);
                
                SetTextColor(hdc, RGB(0, 210, 211));
                TextOutA(hdc, 60, 150, "1. SOLAR HIGHWAY", 16);
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, 55, 190, "- Warp Portals", 14);
                TextOutA(hdc, 55, 220, "- Speed Berries", 15);
                TextOutA(hdc, 55, 250, "- High Speed Flow", 17);
                SetTextColor(hdc, RGB(76, 209, 55));
                TextOutA(hdc, 60, 330, "[ Press 1 ]", 11);

                // Route B Card: Shadow Labyrinth
                RECT cardB = { 270, 130, 460, 380 };
                HBRUSH bB = CreateSolidBrush(RGB(40, 20, 45)); FillRect(hdc, &cardB, bB); DeleteObject(bB);
                HPEN pB = CreatePen(PS_SOLID, 2, RGB(231, 76, 60)); HPEN opB = (HPEN)SelectObject(hdc, pB);
                SelectObject(hdc, (HBRUSH)GetStockObject(HOLLOW_BRUSH));
                Rectangle(hdc, cardB.left, cardB.top, cardB.right, cardB.bottom);
                SelectObject(hdc, opB); DeleteObject(pB);

                SetTextColor(hdc, RGB(231, 76, 60));
                TextOutA(hdc, 285, 150, "2. SHADOW LABYRINTH", 19);
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, 285, 190, "- Rival Vipers", 14);
                TextOutA(hdc, 285, 220, "- Narrow Corridors", 18);
                TextOutA(hdc, 285, 250, "- 2.5x Score Bonus", 18);
                SetTextColor(hdc, RGB(76, 209, 55));
                TextOutA(hdc, 305, 330, "[ Press 2 ]", 11);
            } else if (game_state == 6) {
                char* prompts[] = {"Press UP key...", "Press DOWN key...", "Press LEFT key...", "Press RIGHT key...", "Press GHOST key...", "Press FREEZE key...", "Press MAGNET key...", "Press PAUSE key..."};
                SetTextColor(hdc, RGB(251, 197, 49));
                TextOutA(hdc, 180, 40, "KEY CONFIG", 10);
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, 140, 100, prompts[config_step], lstrlenA(prompts[config_step]));
            } else if (game_state == 7) {
                char buf[128];
                SetTextColor(hdc, RGB(0, 210, 211));
                TextOutA(hdc, 180, 40, "MATCH STATS", 11);
                SetTextColor(hdc, RGB(255, 255, 255));
                wsprintfA(buf, "Ticks Alive: %d", match_ticks); TextOutA(hdc, 120, 100, buf, lstrlenA(buf));
                wsprintfA(buf, "Apples Eaten: %d", match_apples_gained); TextOutA(hdc, 120, 140, buf, lstrlenA(buf));
                wsprintfA(buf, "Coverage: %d%%", (grid_coverage_count*100)/(GRID_WIDTH*GRID_HEIGHT)); TextOutA(hdc, 120, 180, buf, lstrlenA(buf));
                SetTextColor(hdc, RGB(76, 209, 55));
                TextOutA(hdc, 120, 260, "C - Export CSV", 14);
                TextOutA(hdc, 120, 300, "R - Export Replay", 17);
                TextOutA(hdc, 120, 340, "ESC - Back to Menu", 18);
            } else if (game_state == 5) {
                int i;
                SetTextColor(hdc, RGB(0, 210, 211));
                TextOutA(hdc, 160, 30, "HIGH SCORES & HELP", 18);

                SelectObject(hdc, hFontSmall);
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, 40, 75, "Controls: WASD/Arrows to Move, P/ESC to Pause", 45);
                TextOutA(hdc, 40, 100, "Skills: G=Ghost, F=Freeze, M=Magnet", 35);
                TextOutA(hdc, 40, 125, "Modes: Classic, Maze, Ramp, Campaign, VS, Custom, Gauntlet", 59);

                SetTextColor(hdc, RGB(251, 197, 49));
                for(i=0; i<5; i++) {
                    char lbuf[64];
                    wsprintfA(lbuf, "%d. %s - %d pts (%s)", i+1, leaderboard[i].name, leaderboard[i].score, leaderboard[i].mode);
                    TextOutA(hdc, 80, 160 + i * 28, lbuf, lstrlenA(lbuf));
                }
                SelectObject(hdc, hFont);
                SetTextColor(hdc, RGB(0, 210, 211));
                TextOutA(hdc, 100, 330, "[ ENTER / ESC / F1 to Return ]", 30);
            } else if (game_state == 3) {
                SetTextColor(hdc, RGB(251, 197, 49));
                TextOutA(hdc, 210, 200, "PAUSED", 6);
                SelectObject(hdc, hFontSmall);
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, 110, 250, "[P / ESC] Resume   |   [S / Q] Save & Exit", 42);
                SelectObject(hdc, hFont);
            } else if (game_state == 2) {
                char sbuf[32];
                SetTextColor(hdc, RGB(255, 71, 87));
                TextOutA(hdc, 190, 160, "GAME OVER", 9);
                SetTextColor(hdc, RGB(255, 255, 255));
                wsprintfA(sbuf, "Final Score: %d", score); TextOutA(hdc, 160, 210, sbuf, lstrlenA(sbuf));

                if (is_high_score_entry) {
                    char ibuf[32];
                    SetTextColor(hdc, RGB(76, 209, 55));
                    TextOutA(hdc, 130, 270, "NEW HIGH SCORE RANK!", 20);
                    SetTextColor(hdc, RGB(255, 255, 255));
                    wsprintfA(ibuf, "Initials: [%s]", initials_input);
                    TextOutA(hdc, 170, 320, ibuf, lstrlenA(ibuf));
                    SelectObject(hdc, hFontSmall);
                    TextOutA(hdc, 130, 370, "Type Initials & Press ENTER", 27);
                    SelectObject(hdc, hFont);
                } else {
                    SelectObject(hdc, hFontSmall);
                    TextOutA(hdc, 130, 270, "Press ENTER or ESC to Return", 28);
                    SelectObject(hdc, hFont);
                }
            } else if (game_state == 4) { // VICTORY!
                SetTextColor(hdc, RGB(255, 215, 0));
                if (game_mode == 7) {
                    TextOutA(hdc, 110, 160, "GAUNTLET CHAMPION!", 18);
                    SetTextColor(hdc, RGB(76, 209, 55));
                    TextOutA(hdc, 100, 210, "ALL 4 BOSSES DEFEATED!", 22);
                } else {
                    TextOutA(hdc, 120, 160, "CAMPAIGN CONQUERED!", 19);
                    SetTextColor(hdc, RGB(76, 209, 55));
                    TextOutA(hdc, 110, 210, "STAGE 30 MASTER VICTORY!", 24);
                }
                SelectObject(hdc, hFontSmall);
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, 130, 300, "Press ENTER or ESC to Return", 28);
                SelectObject(hdc, hFont);
            } else { // PLAYING
                int i, r, gx, gy;
                char score_text[64];
                char hud_text[64];
                HBRUSH bg1, bg2, detail;

                // Grid background
                bg1 = CreateSolidBrush(RGB(15, 17, 26));
                bg2 = CreateSolidBrush(RGB(20, 23, 36));
                detail = CreateSolidBrush(RGB(34, 39, 61));
                for (gx = 0; gx < GRID_WIDTH; gx++) {
                    for (gy = 0; gy < GRID_HEIGHT; gy++) {
                        RECT tile = { gx * CELL_SIZE, gy * CELL_SIZE + 45, (gx + 1) * CELL_SIZE, (gy + 1) * CELL_SIZE + 45 };
                        FillRect(hdc, &tile, (gx + gy) % 2 == 0 ? bg1 : bg2);
                        if ((gx * 13 + gy * 7) % 17 == 0) {
                            RECT d_rect = { gx * CELL_SIZE + 5, gy * CELL_SIZE + 45 + 5, gx * CELL_SIZE + 9, gy * CELL_SIZE + 45 + 9 };
                            FillRect(hdc, &d_rect, detail);
                        }
                    }
                }
                DeleteObject(bg1); DeleteObject(bg2); DeleteObject(detail);

                // Pulsating Energy Perimeter Inlay Border
                int periPulse = 120 + (FastSin(anim_tick * 2) * 60) / 100;
                HPEN periPen = CreatePen(PS_SOLID, 1, RGB(10, periPulse / 2, periPulse));
                HPEN oldPeriP = (HPEN)SelectObject(hdc, periPen);
                SelectObject(hdc, (HBRUSH)GetStockObject(HOLLOW_BRUSH));
                Rectangle(hdc, 2, 47, 498, 543);
                SelectObject(hdc, oldPeriP);
                DeleteObject(periPen);

                // Traveling Specular Glint along perimeter (perimeter = 496 * 4 = 1984)
                int periTotal = 1984;
                int gDist = (anim_tick * 8) % periTotal;
                int gx_glint = 2, gy_glint = 47;
                if (gDist < 496) { gx_glint = 2 + gDist; gy_glint = 47; }
                else if (gDist < 992) { gx_glint = 498; gy_glint = 47 + (gDist - 496); }
                else if (gDist < 1488) { gx_glint = 498 - (gDist - 992); gy_glint = 543; }
                else { gx_glint = 2; gy_glint = 543 - (gDist - 1488); }
                HBRUSH glintBr = CreateSolidBrush(RGB(255, 255, 255));
                HBRUSH oldGlint = (HBRUSH)SelectObject(hdc, glintBr);
                HPEN oldNullP = (HPEN)SelectObject(hdc, (HPEN)GetStockObject(NULL_PEN));
                Ellipse(hdc, gx_glint - 2, gy_glint - 2, gx_glint + 3, gy_glint + 3);
                SelectObject(hdc, oldGlint); SelectObject(hdc, oldNullP);
                DeleteObject(glintBr);

                // Ornate Cybernetic Arcade HUD Corner Reticle L-Brackets with Tech Notches
                HPEN reticlePen = CreatePen(PS_SOLID, 2, RGB(0, 210, 211));
                HPEN oldRetP = (HPEN)SelectObject(hdc, reticlePen);
                int armLen = 14;
                // Top-Left (4, 49)
                MoveToEx(hdc, 4, 49 + armLen, NULL); LineTo(hdc, 4, 49); LineTo(hdc, 4 + armLen, 49);
                MoveToEx(hdc, 7, 52, NULL); LineTo(hdc, 10, 52);
                // Top-Right (496, 49)
                MoveToEx(hdc, 496 - armLen, 49, NULL); LineTo(hdc, 496, 49); LineTo(hdc, 496, 49 + armLen);
                MoveToEx(hdc, 493, 52, NULL); LineTo(hdc, 490, 52);
                // Bottom-Left (4, 541)
                MoveToEx(hdc, 4, 541 - armLen, NULL); LineTo(hdc, 4, 541); LineTo(hdc, 4 + armLen, 541);
                MoveToEx(hdc, 7, 538, NULL); LineTo(hdc, 10, 538);
                // Bottom-Right (496, 541)
                MoveToEx(hdc, 496 - armLen, 541, NULL); LineTo(hdc, 496, 541); LineTo(hdc, 496, 541 - armLen);
                MoveToEx(hdc, 493, 538, NULL); LineTo(hdc, 490, 538);
                SelectObject(hdc, oldRetP);
                DeleteObject(reticlePen);

                // Glowing Status Diodes
                COLORREF diodeColors[4] = { RGB(0, 255, 200), RGB(255, 215, 0), RGB(255, 71, 87), RGB(0, 210, 211) };
                HBRUSH diodeBr = CreateSolidBrush(diodeColors[(anim_tick / 5) % 4]);
                HBRUSH oldDiode = (HBRUSH)SelectObject(hdc, diodeBr);
                HPEN oldDiodeP = (HPEN)SelectObject(hdc, (HPEN)GetStockObject(NULL_PEN));
                Ellipse(hdc, 3, 48, 7, 52);
                Ellipse(hdc, 493, 48, 497, 52);
                Ellipse(hdc, 3, 538, 7, 542);
                Ellipse(hdc, 493, 538, 497, 542);
                SelectObject(hdc, oldDiode); SelectObject(hdc, oldDiodeP);
                DeleteObject(diodeBr);

                for(i = 0; i < num_obstacles; i++) DrawObstacleGDI(hdc, obstacles[i].x, obstacles[i].y);
                
                for(i = 0; i < num_oil_slicks; i++) {
                    int px = oil_slicks[i].x * CELL_SIZE, py = oil_slicks[i].y * CELL_SIZE + 45;
                    HBRUSH oilBrush = CreateSolidBrush(RGB(20, 20, 20));
                    HBRUSH oldB = (HBRUSH)SelectObject(hdc, oilBrush);
                    HPEN oldP = (HPEN)SelectObject(hdc, (HPEN)GetStockObject(NULL_PEN));
                    Ellipse(hdc, px + 2, py + 4, px + CELL_SIZE - 2, py + CELL_SIZE - 2);
                    SelectObject(hdc, oldB); SelectObject(hdc, oldP);
                    DeleteObject(oilBrush);
                }

                // Magma Hazards
                for(i = 0; i < num_magma; i++) {
                    int px = magma_hazards[i].x * CELL_SIZE, py = magma_hazards[i].y * CELL_SIZE + 45;
                    HBRUSH mBrush = CreateSolidBrush(RGB(230, 57, 70));
                    HBRUSH oldB = (HBRUSH)SelectObject(hdc, mBrush);
                    HPEN oldP = (HPEN)SelectObject(hdc, (HPEN)GetStockObject(NULL_PEN));
                    Ellipse(hdc, px + 3, py + 3, px + CELL_SIZE - 3, py + CELL_SIZE - 3);
                    SelectObject(hdc, oldB); SelectObject(hdc, oldP);
                    DeleteObject(mBrush);
                }

                // Cyber Basilisk Laser Telegraph
                if (boss.alive && boss.boss_type == 1 && boss.laser_charge >= 20) {
                    HPEN lPen = CreatePen(PS_SOLID, (anim_tick % 2 == 0) ? 3 : 1, RGB(0, 210, 211));
                    HPEN oldLP = (HPEN)SelectObject(hdc, lPen);
                    if (boss.laser_row >= 0) {
                        MoveToEx(hdc, 0, boss.laser_row * CELL_SIZE + 45 + CELL_SIZE/2, NULL);
                        LineTo(hdc, 500, boss.laser_row * CELL_SIZE + 45 + CELL_SIZE/2);
                    }
                    if (boss.laser_col >= 0) {
                        MoveToEx(hdc, boss.laser_col * CELL_SIZE + CELL_SIZE/2, 45, NULL);
                        LineTo(hdc, boss.laser_col * CELL_SIZE + CELL_SIZE/2, 545);
                    }
                    SelectObject(hdc, oldLP);
                    DeleteObject(lPen);
                }

                if (portal_active) {
                    DrawPortalGDI(hdc, portal_a.x, portal_a.y, RGB(0, 210, 211));
                    DrawPortalGDI(hdc, portal_b.x, portal_b.y, RGB(155, 89, 182));
                }

                // Draw Regular & Special Fruits
                DrawGemGDI(hdc, food.x, food.y, RGB(255, 71, 87));
                if (golden_apple.x != -1) DrawGemGDI(hdc, golden_apple.x, golden_apple.y, RGB(255, 215, 0));
                if (poison_berry.x != -1) DrawGemGDI(hdc, poison_berry.x, poison_berry.y, RGB(142, 68, 173));
                if (speed_berry.x != -1) DrawGemGDI(hdc, speed_berry.x, speed_berry.y, RGB(230, 126, 34));
                if (ghost_berry.x != -1) DrawGemGDI(hdc, ghost_berry.x, ghost_berry.y, RGB(0, 210, 211));

                for(r = 0; r < num_rivals; r++) {
                    if (!rivals[r].alive) continue;
                    for(i = rivals[r].len - 1; i >= 0; i--) DrawRivalGDI(hdc, rivals[r].body[i].x, rivals[r].body[i].y, i, rivals[r].type);
                }

                if (boss.alive) {
                    for(i = boss.len - 1; i >= 0; i--) DrawBossGDI(hdc, boss.body[i].x, boss.body[i].y, i);
                }

                for (i = snake_len - 1; i >= 0; i--) {
                    DrawSnakeShadowGDI(hdc, snake[i].x, snake[i].y, i, snake_len, (i==0?dir_x:0), (i==0?dir_y:0));
                }

                for (i = snake_len - 1; i >= 0; i--) {
                    DrawSnakeSegmentGDI(hdc, snake[i].x, snake[i].y, i, snake_len, ghost_active > 0, (i==0?dir_x:0), (i==0?dir_y:0));
                }

                // 4-Layer Particles rendering
                for(i = 0; i < particle_count; i++) {
                    if (particles[i].type == 0) { // Layer 0: Needle sparks with line trails
                        HPEN sparkPen = CreatePen(PS_SOLID, 1, particles[i].color);
                        HPEN op = (HPEN)SelectObject(hdc, sparkPen);
                        MoveToEx(hdc, particles[i].x, particles[i].y, NULL);
                        LineTo(hdc, particles[i].x - particles[i].vx * 2, particles[i].y - particles[i].vy * 2);
                        SelectObject(hdc, op);
                        DeleteObject(sparkPen);
                    } else if (particles[i].type == 1) { // Layer 1: Buoyant smoke puffs
                        HBRUSH smkBr = CreateSolidBrush(particles[i].color);
                        HBRUSH ob = (HBRUSH)SelectObject(hdc, smkBr);
                        HPEN op = (HPEN)SelectObject(hdc, (HPEN)GetStockObject(NULL_PEN));
                        int s = particles[i].size;
                        Ellipse(hdc, particles[i].x - s, particles[i].y - s, particles[i].x + s + 1, particles[i].y + s + 1);
                        SelectObject(hdc, ob); SelectObject(hdc, op);
                        DeleteObject(smkBr);
                    } else if (particles[i].type == 2) { // Layer 2: Heavy debris shards with tumbling rotation
                        HBRUSH debBr = CreateSolidBrush(particles[i].color);
                        HBRUSH ob = (HBRUSH)SelectObject(hdc, debBr);
                        HPEN debPen = CreatePen(PS_SOLID, 1, RGB(10, 20, 30));
                        HPEN op = (HPEN)SelectObject(hdc, debPen);
                        int s = particles[i].size;
                        int c = FastCos(particles[i].rot) * s / 100;
                        int sn = FastSin(particles[i].rot) * s / 100;
                        POINT dpts[4];
                        dpts[0].x = particles[i].x - c; dpts[0].y = particles[i].y - sn;
                        dpts[1].x = particles[i].x + sn; dpts[1].y = particles[i].y - c;
                        dpts[2].x = particles[i].x + c; dpts[2].y = particles[i].y + sn;
                        dpts[3].x = particles[i].x - sn; dpts[3].y = particles[i].y + c;
                        Polygon(hdc, dpts, 4);
                        SelectObject(hdc, ob); SelectObject(hdc, op);
                        DeleteObject(debBr); DeleteObject(debPen);
                    } else if (particles[i].type == 3) { // Layer 3: Radiant celebration energy stars
                        HPEN starPen = CreatePen(PS_SOLID, 1, particles[i].color);
                        HPEN op = (HPEN)SelectObject(hdc, starPen);
                        int s = particles[i].size;
                        MoveToEx(hdc, particles[i].x - s, particles[i].y, NULL);
                        LineTo(hdc, particles[i].x + s + 1, particles[i].y);
                        MoveToEx(hdc, particles[i].x, particles[i].y - s, NULL);
                        LineTo(hdc, particles[i].x, particles[i].y + s + 1);
                        SelectObject(hdc, op);
                        DeleteObject(starPen);
                    }
                }

                // Weather Effects: Rain / Sparks
                {
                    HPEN rainPen = CreatePen(PS_SOLID, 1, (campaign_branch == 1) ? RGB(180, 100, 180) : RGB(100, 150, 200));
                    HPEN oldPen2 = (HPEN)SelectObject(hdc, rainPen);
                    for(i=0; i<30; i++) {
                        int rx = (i * 37 + (anim_tick * 4)) % 520;
                        int ry = (i * 53 + (anim_tick * 16)) % 620;
                        MoveToEx(hdc, rx, ry, NULL);
                        LineTo(hdc, rx - 2, ry + 6);
                    }
                    SelectObject(hdc, oldPen2);
                    DeleteObject(rainPen);
                }

                // HUD Bar
                if (game_mode == 7) { // Gauntlet
                    wsprintfA(score_text, "Score: %d | BOSS [%d/4] %s HP:%d/%d", score, gauntlet_stage+1, boss_names[gauntlet_stage], boss.hp, boss.max_hp);
                } else if (game_mode == 4) {
                    if (boss.alive) {
                        wsprintfA(score_text, "Score: %d | L%d BOSS HP: %d/%d", score, campaign_level, boss.hp, boss.max_hp);
                    } else {
                        wsprintfA(score_text, "Score: %d | Stage %d/30 [%s]", score, campaign_level, campaign_branch==0?"Solar":"Shadow");
                    }
                } else {
                    wsprintfA(score_text, "Score: %d | Mode: %s", score, mode_names[game_mode]);
                }
                TextOutA(hdc, 5, 5, score_text, lstrlenA(score_text));

                // Bottom Skills HUD
                wsprintfA(hud_text, "[G]: %s  [F]: %s  [M]: %s",
                    ghost_active > 0 ? "GHOST!" : (ghost_cd == 0 ? "READY" : "CD"),
                    freeze_active > 0 ? "SLOW!" : (freeze_cd == 0 ? "READY" : "CD"),
                    magnet_active > 0 ? "MAG!" : (magnet_cd == 0 ? "READY" : "CD"));
                SetTextColor(hdc, RGB(72, 219, 251));
                TextOutA(hdc, 10, 560, hud_text, lstrlenA(hud_text));

                // Dual-tier shockwaves
                for (i = 0; i < num_shockwaves; i++) {
                    int scx = shockwaves[i].x, scy = shockwaves[i].y;
                    int r1 = shockwaves[i].r1, r2 = shockwaves[i].r2;
                    HPEN sw1Pen = CreatePen(PS_SOLID, 2, shockwaves[i].color);
                    HPEN oP1 = (HPEN)SelectObject(hdc, sw1Pen);
                    HBRUSH oB1 = (HBRUSH)SelectObject(hdc, (HBRUSH)GetStockObject(HOLLOW_BRUSH));
                    Ellipse(hdc, scx - r1, scy - r1, scx + r1, scy + r1);
                    SelectObject(hdc, oP1);
                    DeleteObject(sw1Pen);

                    HPEN sw2Pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                    HPEN oP2 = (HPEN)SelectObject(hdc, sw2Pen);
                    Ellipse(hdc, scx - r2, scy - r2, scx + r2, scy + r2);
                    SelectObject(hdc, oP2); SelectObject(hdc, oB1);
                    DeleteObject(sw2Pen);
                }
            }

            SelectObject(hdc, oldFont);
            DeleteObject(hFont);
            DeleteObject(hFontSmall);

            SetWindowOrgEx(hdc, 0, 0, NULL);
            XFORM xform = { scale, 0.0f, 0.0f, scale, 0.0f, 0.0f };
            SetGraphicsMode(paintDC, GM_ADVANCED);
            SetWorldTransform(paintDC, &xform);
            BitBlt(paintDC, 0, 0, 520, 620, hdc, 0, 0, SRCCOPY);
            SelectObject(hdc, oldBmp);
            DeleteObject(hbm);
            DeleteDC(hdc);

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
    SetProcessDPIAware();
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

    HDC hdcScreen = GetDC(NULL);
    int dpi = GetDeviceCaps(hdcScreen, LOGPIXELSX);
    ReleaseDC(NULL, hdcScreen);
    float scale = dpi / 96.0f;

    winWidth = (int)((GRID_WIDTH * CELL_SIZE + 20) * scale);
    winHeight = (int)((GRID_HEIGHT * CELL_SIZE + 115) * scale);

    RECT rect = {0, 0, winWidth, winHeight};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);

    hwnd = CreateWindowExA(0, "KSnakeApp", "KSnake Arcade [Press H or F1 for Help]", (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN) & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    while(GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
