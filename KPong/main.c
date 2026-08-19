#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#pragma comment(lib, "msvcrt.lib")

#define W 450
#define H 320
#define PAD_W 10
#define BALL_SIZE 10
#define TIMER_ID 1

#define MAX_PARTICLES 80
#define MAX_TRAIL 10
#define MAX_SHOCKWAVES 10
#define MAX_BALLS 6

typedef struct {
    float x, y;
    float vx, vy;
    float life;
    float max_life;
    int size;
    COLORREF color;
} Particle;

typedef struct {
    int x, y;
    COLORREF color;
} BallTrailNode;

typedef struct {
    int x, y;
    int radius;
    int max_radius;
    COLORREF color;
} Shockwave;

typedef struct {
    float x, y;
    float dx, dy;
    int active;
    int is_fireball;
    int last_hitter;
    BallTrailNode trail[MAX_TRAIL];
    int trail_count;
} Ball;

typedef struct {
    int rally;
    int mode;
    char date[16];
} LeaderboardEntry;

typedef struct {
    int total_games;
    int wins;
    int losses;
    int high_rally;
    LeaderboardEntry top_scores[5];
} StatsData;

typedef struct {
    int p1_score, p2_score;
    int rally;
    int p1_y, p2_y;
    int game_mode;
    int is_pvp;
    int campaign_level;
    int difficulty;
    int theme_index;
    int p1_buff_timer, p2_buff_timer;
    int p1_debuff_timer, p2_debuff_timer;
    int p1_freeze_timer, p2_freeze_timer;
    int p1_shield_timer, p2_shield_timer;
    int skill_slow_timer, skill_slow_cooldown;
    int skill_mega_timer, skill_mega_cooldown;
    int skill_fireball_ready, skill_fireball_cooldown;
    Ball balls[MAX_BALLS];
} SaveState;

Particle particles[MAX_PARTICLES];
Shockwave shockwaves[MAX_SHOCKWAVES];
Ball balls[MAX_BALLS];
StatsData stats = {0};

int p1_pad_h = 50;
int p2_pad_h = 50;
int campaign_level = 1;
int win_screen = 0;
int is_paused = 0;
int show_stats_overlay = 0;
int show_help_overlay = 1;

int game_mode = 0; // 0: Classic, 1: Obstacle Arena, 2: Power-Up Frenzy, 3: Multi-Ball, 4: Campaign
int is_pvp = 0;    // 0: 1P vs AI, 1: 2P Local PvP
int theme_index = 0; // 0: Cyberpunk, 1: Retro, 2: Vaporwave, 3: Matrix

int powerup_x = -1;
int powerup_y = -1;
int powerup_active = 0;
int powerup_type = 0; // 0: Expand, 1: Shrink, 2: Freeze, 3: Multi-Ball, 4: Speed Boost, 5: Shield

int p1_buff_timer = 0;
int p2_buff_timer = 0;
int p1_debuff_timer = 0;
int p2_debuff_timer = 0;
int p1_freeze_timer = 0;
int p2_freeze_timer = 0;
int p1_shield_timer = 0;
int p2_shield_timer = 0;

float p1_hit_ripple = 0.0f;
float p2_hit_ripple = 0.0f;

// Active Skills
int skill_slow_timer = 0;
int skill_slow_cooldown = 0;
int skill_mega_timer = 0;
int skill_mega_cooldown = 0;
int skill_fireball_ready = 0;
int skill_fireball_cooldown = 0;

// Dynamic Arena Features
int obs_y = H / 2 - 20;
int obs_dy = 3;
int obs1_active = 1;
int obs1_respawn = 0;

int obs2_x = W / 2 - 20;
int obs2_dx = 4;
int obs2_active = 1;
int obs2_respawn = 0;

int gw_x = W / 2;
int gw_y = H / 2;

int portal1_x = W / 4 + 20;
int portal1_y = H / 4;
int portal2_x = 3 * W / 4 - 20;
int portal2_y = 3 * H / 4;
int portal_cooldown = 0;

// Stage 20 Boss
int boss_shield_hp = 3;
int boss_shield_timer = 0;
int boss_shield_y = H / 2 - 50;
int boss_shield_dy = 2;
int boss_shield_h = 100;

int p1_y = H / 2 - 25;
int p2_y = H / 2 - 25;
int p1_vy = 0;
int p2_vy = 0;

int p1_score = 0;
int p2_score = 0;
int rally = 0;
int game_over = 0;
int difficulty = 2;

int ai_speeds[] = {0, 2, 4, 6};
int start_speeds[] = {0, 4, 5, 7};

COLORREF GetPrimaryColor() {
    if (theme_index == 1) return RGB(0, 255, 102);  // Retro Green
    if (theme_index == 2) return RGB(255, 113, 206); // Vaporwave Pink
    if (theme_index == 3) return RGB(0, 255, 65);   // Matrix Green
    return RGB(0, 229, 255);                        // Cyberpunk Cyan
}

COLORREF GetSecondaryColor() {
    if (theme_index == 1) return RGB(51, 255, 153);
    if (theme_index == 2) return RGB(1, 205, 254);  // Cyan
    if (theme_index == 3) return RGB(0, 143, 17);
    return RGB(255, 0, 204);                        // Magenta
}

COLORREF GetBgColor() {
    if (theme_index == 1) return RGB(2, 13, 4);
    if (theme_index == 2) return RGB(26, 8, 38);
    if (theme_index == 3) return RGB(0, 5, 0);
    return RGB(8, 12, 20);
}

void AddParticles(float x, float y, COLORREF color, int count, float speed) {
    for (int i = 0; i < count; i++) {
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (particles[p].life <= 0) {
                float angle = ((float)(rand() % 360)) * 3.14159f / 180.0f;
                float spd = (0.2f + ((float)(rand() % 80) / 100.0f)) * speed;
                particles[p].x = x;
                particles[p].y = y;
                particles[p].vx = cosf(angle) * spd;
                particles[p].vy = sinf(angle) * spd;
                particles[p].life = 1.0f;
                particles[p].max_life = 1.0f;
                particles[p].size = 2 + (rand() % 3);
                particles[p].color = color;
                break;
            }
        }
    }
}

void AddShockwave(int x, int y, COLORREF color) {
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (shockwaves[i].radius <= 0) {
            shockwaves[i].x = x;
            shockwaves[i].y = y;
            shockwaves[i].radius = 4;
            shockwaves[i].max_radius = 28;
            shockwaves[i].color = color;
            break;
        }
    }
}

void UpdateParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0) {
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            particles[i].life -= 0.05f;
        }
    }
}

void UpdateShockwaves() {
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (shockwaves[i].radius > 0) {
            shockwaves[i].radius += 2;
            if (shockwaves[i].radius >= shockwaves[i].max_radius) {
                shockwaves[i].radius = 0;
            }
        }
    }
}

void LoadStats() {
    FILE* f = fopen("kpong_stats_v2.dat", "rb");
    if (f) {
        fread(&stats, sizeof(StatsData), 1, f);
        fclose(f);
    }
}

void SaveStats() {
    FILE* f = fopen("kpong_stats_v2.dat", "wb");
    if (f) {
        fwrite(&stats, sizeof(StatsData), 1, f);
        fclose(f);
    }
}

void RecordScore(int r) {
    stats.total_games++;
    if (r > stats.high_rally) stats.high_rally = r;

    // Check top 5 high scores
    for (int i = 0; i < 5; i++) {
        if (r > stats.top_scores[i].rally) {
            for (int j = 4; j > i; j--) {
                stats.top_scores[j] = stats.top_scores[j - 1];
            }
            stats.top_scores[i].rally = r;
            stats.top_scores[i].mode = game_mode;
            SYSTEMTIME st;
            GetLocalTime(&st);
            wsprintfA(stats.top_scores[i].date, "%02d/%02d/%04d", st.wMonth, st.wDay, st.wYear);
            break;
        }
    }
    SaveStats();
}

void SaveGameState() {
    FILE* f = fopen("kpong_save.dat", "wb");
    if (f) {
        SaveState s;
        s.p1_score = p1_score; s.p2_score = p2_score; s.rally = rally;
        s.p1_y = p1_y; s.p2_y = p2_y; s.game_mode = game_mode; s.is_pvp = is_pvp;
        s.campaign_level = campaign_level; s.difficulty = difficulty; s.theme_index = theme_index;
        s.p1_buff_timer = p1_buff_timer; s.p2_buff_timer = p2_buff_timer;
        s.p1_debuff_timer = p1_debuff_timer; s.p2_debuff_timer = p2_debuff_timer;
        s.p1_freeze_timer = p1_freeze_timer; s.p2_freeze_timer = p2_freeze_timer;
        s.p1_shield_timer = p1_shield_timer; s.p2_shield_timer = p2_shield_timer;
        s.skill_slow_timer = skill_slow_timer; s.skill_slow_cooldown = skill_slow_cooldown;
        s.skill_mega_timer = skill_mega_timer; s.skill_mega_cooldown = skill_mega_cooldown;
        s.skill_fireball_ready = skill_fireball_ready; s.skill_fireball_cooldown = skill_fireball_cooldown;
        for (int i = 0; i < MAX_BALLS; i++) s.balls[i] = balls[i];
        fwrite(&s, sizeof(SaveState), 1, f);
        fclose(f);
        MessageBeep(MB_OK);
    }
}

void LoadGameState() {
    FILE* f = fopen("kpong_save.dat", "rb");
    if (f) {
        SaveState s;
        if (fread(&s, sizeof(SaveState), 1, f) == 1) {
            p1_score = s.p1_score; p2_score = s.p2_score; rally = s.rally;
            p1_y = s.p1_y; p2_y = s.p2_y; game_mode = s.game_mode; is_pvp = s.is_pvp;
            campaign_level = s.campaign_level; difficulty = s.difficulty; theme_index = s.theme_index;
            p1_buff_timer = s.p1_buff_timer; p2_buff_timer = s.p2_buff_timer;
            p1_debuff_timer = s.p1_debuff_timer; p2_debuff_timer = s.p2_debuff_timer;
            p1_freeze_timer = s.p1_freeze_timer; p2_freeze_timer = s.p2_freeze_timer;
            p1_shield_timer = s.p1_shield_timer; p2_shield_timer = s.p2_shield_timer;
            skill_slow_timer = s.skill_slow_timer; skill_slow_cooldown = s.skill_slow_cooldown;
            skill_mega_timer = s.skill_mega_timer; skill_mega_cooldown = s.skill_mega_cooldown;
            skill_fireball_ready = s.skill_fireball_ready; skill_fireball_cooldown = s.skill_fireball_cooldown;
            for (int i = 0; i < MAX_BALLS; i++) balls[i] = s.balls[i];
            game_over = 0; is_paused = 0;
            MessageBeep(MB_OK);
        }
        fclose(f);
    }
}

void ResetBalls() {
    for (int i = 0; i < MAX_BALLS; i++) {
        balls[i].active = 0;
        balls[i].is_fireball = 0;
        balls[i].trail_count = 0;
        balls[i].last_hitter = 0;
    }
    int base_spd = (game_mode == 4) ? (4 + (campaign_level / 2)) : start_speeds[difficulty];
    if (base_spd > 11) base_spd = 11;
    int ball_count = (game_mode == 3) ? 3 : 1;

    for (int i = 0; i < ball_count; i++) {
        balls[i].x = W / 2 - BALL_SIZE / 2;
        balls[i].y = H / 2 - BALL_SIZE / 2 + (i * 15 - 15);
        int dir = (i % 2 == 0) ? 1 : -1;
        balls[i].dx = dir * base_spd;
        balls[i].dy = (rand() % 2 == 0) ? 3 : -3;
        balls[i].active = 1;
    }
}

void SpawnExtraBall() {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (!balls[i].active) {
            balls[i].x = W / 2 - BALL_SIZE / 2;
            balls[i].y = H / 2 - BALL_SIZE / 2;
            float angle = ((float)(rand() % 360)) * 3.14159f / 180.0f;
            float spd = 5.0f + (rand() % 3);
            balls[i].dx = cosf(angle) * spd;
            if (balls[i].dx > -2.5f && balls[i].dx < 2.5f) balls[i].dx = (balls[i].dx < 0) ? -4.5f : 4.5f;
            balls[i].dy = sinf(angle) * spd;
            balls[i].active = 1;
            balls[i].is_fireball = 0;
            balls[i].last_hitter = 0;
            balls[i].trail_count = 0;
            AddShockwave(W / 2, H / 2, GetPrimaryColor());
            AddParticles(W / 2, H / 2, GetPrimaryColor(), 15, 6.0f);
            break;
        }
    }
}

int ActiveBallCount() {
    int count = 0;
    for (int i = 0; i < MAX_BALLS; i++) {
        if (balls[i].active) count++;
    }
    return count;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            ResetBalls();
            SetTimer(hwnd, TIMER_ID, 30, NULL);
            break;
        case WM_KEYDOWN:
            if (wParam == VK_SPACE) is_paused = !is_paused;
            if (wParam == 'M') { game_mode = (game_mode + 1) % 5; p1_score = 0; p2_score = 0; rally = 0; ResetBalls(); }
            if (wParam == 'V') is_pvp = !is_pvp;
            if (wParam == 'T') theme_index = (theme_index + 1) % 4;
            if (wParam == 'L') show_stats_overlay = !show_stats_overlay;
            if (wParam == 'H') show_help_overlay = !show_help_overlay;
            if (wParam == VK_F5) SaveGameState();
            if (wParam == VK_F9) LoadGameState();
            break;

        case WM_TIMER:
            if (is_paused) { InvalidateRect(hwnd, NULL, FALSE); break; }

            if (game_over) {
                if (GetAsyncKeyState('R') & 0x8000) {
                    p1_score = 0; p2_score = 0; rally = 0;
                    game_over = 0; win_screen = 0;
                    skill_slow_timer = 0; skill_slow_cooldown = 0;
                    skill_mega_timer = 0; skill_mega_cooldown = 0;
                    skill_fireball_ready = 0; skill_fireball_cooldown = 0;
                    boss_shield_hp = 3; boss_shield_timer = 0;
                    obs1_active = 1; obs2_active = 1;
                    ResetBalls();
                }
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }

            if (GetAsyncKeyState('1') & 0x8000) game_mode = 0;
            if (GetAsyncKeyState('2') & 0x8000) game_mode = 1;
            if (GetAsyncKeyState('3') & 0x8000) game_mode = 2;
            if (GetAsyncKeyState('4') & 0x8000) game_mode = 3;
            if (GetAsyncKeyState('5') & 0x8000) game_mode = 4;

            // Skills Activation
            if ((GetAsyncKeyState('F') & 0x8000) && skill_slow_cooldown == 0) {
                skill_slow_timer = 180; skill_slow_cooldown = 360;
                AddShockwave(W / 2, H / 2, GetPrimaryColor());
                MessageBeep(MB_OK);
            }
            if (((GetAsyncKeyState('E') & 0x8000) || (GetAsyncKeyState('P') & 0x8000)) && skill_mega_cooldown == 0) {
                skill_mega_timer = 240; skill_mega_cooldown = 450;
                AddShockwave(20, p1_y + p1_pad_h / 2, RGB(255, 215, 0));
                MessageBeep(MB_OK);
            }
            if ((GetAsyncKeyState('B') & 0x8000) && skill_fireball_cooldown == 0) {
                skill_fireball_ready = 1; skill_fireball_cooldown = 300;
                AddShockwave(20, p1_y + p1_pad_h / 2, RGB(255, 100, 0));
                MessageBeep(MB_OK);
            }

            // Timers Update
            if (skill_slow_timer > 0) skill_slow_timer--;
            if (skill_slow_cooldown > 0) skill_slow_cooldown--;
            if (skill_mega_timer > 0) skill_mega_timer--;
            if (skill_mega_cooldown > 0) skill_mega_cooldown--;
            if (skill_fireball_cooldown > 0) skill_fireball_cooldown--;

            if (p1_buff_timer > 0) p1_buff_timer--;
            if (p2_buff_timer > 0) p2_buff_timer--;
            if (p1_debuff_timer > 0) p1_debuff_timer--;
            if (p2_debuff_timer > 0) p2_debuff_timer--;
            if (p1_freeze_timer > 0) p1_freeze_timer--;
            if (p2_freeze_timer > 0) p2_freeze_timer--;
            if (p1_shield_timer > 0) p1_shield_timer--;
            if (p2_shield_timer > 0) p2_shield_timer--;

            if (p1_hit_ripple > 0.0f) { p1_hit_ripple -= 0.05f; if (p1_hit_ripple < 0.0f) p1_hit_ripple = 0.0f; }
            if (p2_hit_ripple > 0.0f) { p2_hit_ripple -= 0.05f; if (p2_hit_ripple < 0.0f) p2_hit_ripple = 0.0f; }

            // Paddle Heights
            p1_pad_h = 50 - (rally * 2); if (p1_pad_h < 20) p1_pad_h = 20;
            p2_pad_h = 50 - (rally * 2); if (p2_pad_h < 20) p2_pad_h = 20;

            if (game_mode == 4 && campaign_level >= 3 && campaign_level < 20) { p2_pad_h -= 10; }
            if (game_mode == 4 && (campaign_level == 10 || campaign_level == 15)) { p2_pad_h += 40; }
            if (game_mode == 4 && campaign_level == 20) { p2_pad_h = 75; }

            if (p1_buff_timer > 0) p1_pad_h += 30;
            if (p2_buff_timer > 0) p2_buff_timer += 30;
            if (p1_debuff_timer > 0) p1_pad_h = 15;
            if (p2_debuff_timer > 0) p2_pad_h = 15;
            if (skill_mega_timer > 0) p1_pad_h = (int)(p1_pad_h * 2.5f);

            if (p1_pad_h < 15) p1_pad_h = 15;
            if (p2_pad_h < 15) p2_pad_h = 15;

            // Player 1 Movement (W/S or Up/Down if 1P mode)
            int p1_prev_y = p1_y;
            if (p1_freeze_timer == 0) {
                if ((GetAsyncKeyState('W') & 0x8000) || (!is_pvp && (GetAsyncKeyState(VK_UP) & 0x8000))) p1_y -= 6;
                if ((GetAsyncKeyState('S') & 0x8000) || (!is_pvp && (GetAsyncKeyState(VK_DOWN) & 0x8000))) p1_y += 6;
            }
            if (p1_y < 0) p1_y = 0;
            if (p1_y > H - p1_pad_h) p1_y = H - p1_pad_h;
            p1_vy = p1_y - p1_prev_y;

            // Player 2 Movement (AI or 2P PvP)
            int p2_prev_y = p2_y;
            if (p2_freeze_timer == 0) {
                if (is_pvp) {
                    if (GetAsyncKeyState(VK_UP) & 0x8000) p2_y -= 6;
                    if (GetAsyncKeyState(VK_DOWN) & 0x8000) p2_y += 6;
                } else {
                    int ai_spd = ai_speeds[difficulty];
                    if (game_mode == 4) {
                        if (campaign_level == 1) ai_spd = 2;
                        else if (campaign_level == 2) ai_spd = 3;
                        else if (campaign_level <= 5) ai_spd = 4;
                        else if (campaign_level <= 9) ai_spd = 5;
                        else if (campaign_level <= 15) ai_spd = 6;
                        else if (campaign_level <= 19) ai_spd = 8;
                        else ai_spd = 10;
                    }
                    int target_ball = 0;
                    float min_x = -1.0f;
                    for (int i = 0; i < MAX_BALLS; i++) {
                        if (balls[i].active && balls[i].dx > 0 && balls[i].x > min_x) {
                            min_x = balls[i].x;
                            target_ball = i;
                        }
                    }
                    int target_y = (int)(balls[target_ball].y);
                    if (target_y > p2_y + p2_pad_h / 2 + 8) p2_y += ai_spd;
                    if (target_y < p2_y + p2_pad_h / 2 - 8) p2_y -= ai_spd;
                }
            }
            if (p2_y < 0) p2_y = 0;
            if (p2_y > H - p2_pad_h) p2_y = H - p2_pad_h;
            p2_vy = p2_y - p2_prev_y;

            // Powerup Capsule Pickup Spawning
            if (powerup_active > 0) powerup_active--;
            int spawn_chance = (game_mode == 2) ? 50 : (game_mode != 0 ? 200 : 0);
            if (spawn_chance > 0 && powerup_x == -1 && (rand() % spawn_chance == 0)) {
                powerup_x = W / 4 + rand() % (W / 2);
                powerup_y = H / 4 + rand() % (H / 2);
                powerup_active = 260;
                powerup_type = rand() % 6; // 0..5
            }
            if (powerup_active == 0) { powerup_x = -1; powerup_y = -1; }

            // Obstacles Respawn
            int has_obstacles = (game_mode == 1 || (game_mode == 4 && campaign_level >= 5));
            if (!obs1_active) { obs1_respawn--; if (obs1_respawn <= 0) obs1_active = 1; }
            if (!obs2_active) { obs2_respawn--; if (obs2_respawn <= 0) obs2_active = 1; }

            if (has_obstacles && obs1_active) {
                obs_y += obs_dy;
                if (obs_y < 10) { obs_y = 10; obs_dy = -obs_dy; }
                if (obs_y > H - 50) { obs_y = H - 50; obs_dy = -obs_dy; }
            }
            if (has_obstacles && obs2_active) {
                obs2_x += obs2_dx;
                if (obs2_x < 60) { obs2_x = 60; obs2_dx = -obs2_dx; }
                if (obs2_x > W - 100) { obs2_x = W - 100; obs2_dx = -obs2_dx; }
            }

            // Boss Shield (Campaign Stage 20)
            if (game_mode == 4 && campaign_level == 20) {
                if (boss_shield_hp > 0) {
                    boss_shield_y += boss_shield_dy;
                    if (boss_shield_y < 10) { boss_shield_y = 10; boss_shield_dy = -boss_shield_dy; }
                    if (boss_shield_y > H - boss_shield_h - 10) { boss_shield_y = H - boss_shield_h - 10; boss_shield_dy = -boss_shield_dy; }
                } else {
                    boss_shield_timer--;
                    if (boss_shield_timer <= 0) { boss_shield_hp = 3; AddShockwave(W - 55, H / 2, RGB(0, 255, 200)); }
                }
            }

            if (portal_cooldown > 0) portal_cooldown--;
            float spd_scale = (skill_slow_timer > 0) ? 0.5f : 1.0f;

            // Ball Updates Loop
            for (int b = 0; b < MAX_BALLS; b++) {
                if (!balls[b].active) continue;

                // Gravity Well
                if (game_mode == 1 || (game_mode == 4 && campaign_level >= 9)) {
                    float dx_gw = gw_x - balls[b].x;
                    float dy_gw = gw_y - balls[b].y;
                    float dist_sq = dx_gw * dx_gw + dy_gw * dy_gw;
                    if (dist_sq < 90.0f * 90.0f && dist_sq > 25.0f) {
                        float dist = sqrtf(dist_sq);
                        balls[b].dx += (dx_gw / dist) * 0.28f * spd_scale;
                        balls[b].dy += (dy_gw / dist) * 0.28f * spd_scale;
                    }
                }

                balls[b].x += balls[b].dx * spd_scale;
                balls[b].y += balls[b].dy * spd_scale;

                // Portals
                if ((game_mode == 1 || (game_mode == 4 && campaign_level >= 13)) && portal_cooldown == 0) {
                    float d1 = sqrtf((balls[b].x - portal1_x)*(balls[b].x - portal1_x) + (balls[b].y - portal1_y)*(balls[b].y - portal1_y));
                    if (d1 < 18.0f) {
                        balls[b].x = portal2_x + (balls[b].dx > 0 ? 18.0f : -18.0f); balls[b].y = portal2_y;
                        portal_cooldown = 30;
                        AddShockwave(portal1_x, portal1_y, GetPrimaryColor());
                        AddShockwave(portal2_x, portal2_y, GetSecondaryColor());
                        MessageBeep(MB_OK);
                    } else {
                        float d2 = sqrtf((balls[b].x - portal2_x)*(balls[b].x - portal2_x) + (balls[b].y - portal2_y)*(balls[b].y - portal2_y));
                        if (d2 < 18.0f) {
                            balls[b].x = portal1_x + (balls[b].dx > 0 ? 18.0f : -18.0f); balls[b].y = portal1_y;
                            portal_cooldown = 30;
                            AddShockwave(portal2_x, portal2_y, GetSecondaryColor());
                            AddShockwave(portal1_x, portal1_y, GetPrimaryColor());
                            MessageBeep(MB_OK);
                        }
                    }
                }

                // Trail
                float spd = sqrtf(balls[b].dx * balls[b].dx + balls[b].dy * balls[b].dy);
                int r_dyn = (int)(sinf(spd * 0.5f + rally * 0.2f) * 127 + 128);
                int g_dyn = (int)(sinf(spd * 0.5f + rally * 0.2f + 2.0f) * 127 + 128);
                int b_dyn = (int)(sinf(spd * 0.5f + rally * 0.2f + 4.0f) * 127 + 128);
                COLORREF dynColor = RGB(r_dyn, g_dyn, b_dyn);
                COLORREF bTrailColor = balls[b].is_fireball ? RGB(255, 100, 0) : dynColor;
                
                for (int i = MAX_TRAIL - 1; i > 0; i--) balls[b].trail[i] = balls[b].trail[i - 1];
                balls[b].trail[0].x = (int)balls[b].x + BALL_SIZE / 2;
                balls[b].trail[0].y = (int)balls[b].y + BALL_SIZE / 2;
                balls[b].trail[0].color = bTrailColor;
                if (balls[b].trail_count < MAX_TRAIL) balls[b].trail_count++;

                // Powerup Capsule Pickup
                if (powerup_x != -1 && balls[b].x < powerup_x + 18 && balls[b].x + BALL_SIZE > powerup_x &&
                    balls[b].y < powerup_y + 18 && balls[b].y + BALL_SIZE > powerup_y) {
                    COLORREF pColor = GetPrimaryColor();
                    AddParticles((float)(powerup_x + 9), (float)(powerup_y + 9), pColor, 20, 7.0f);
                    AddShockwave(powerup_x + 9, powerup_y + 9, pColor);

                    if (powerup_type == 0) {
                        if (balls[b].last_hitter == 1) p1_buff_timer = 200; else if (balls[b].last_hitter == 2) p2_buff_timer = 200;
                    } else if (powerup_type == 1) {
                        if (balls[b].last_hitter == 1) p2_debuff_timer = 200; else if (balls[b].last_hitter == 2) p1_debuff_timer = 200;
                    } else if (powerup_type == 2) {
                        if (balls[b].last_hitter == 1) p2_freeze_timer = 100; else if (balls[b].last_hitter == 2) p1_freeze_timer = 100;
                    } else if (powerup_type == 3) {
                        SpawnExtraBall(); SpawnExtraBall();
                    } else if (powerup_type == 4) {
                        balls[b].dx *= 1.3f; balls[b].dy *= 1.3f;
                    } else if (powerup_type == 5) {
                        if (balls[b].last_hitter == 1) p1_shield_timer = 300; else if (balls[b].last_hitter == 2) p2_shield_timer = 300;
                    }
                    powerup_x = -1; powerup_y = -1;
                    MessageBeep(MB_ICONASTERISK);
                }

                // Walls
                if (balls[b].y < 0) {
                    balls[b].y = 0; balls[b].dy = -balls[b].dy;
                    AddParticles(balls[b].x + BALL_SIZE / 2, 0, RGB(255, 255, 255), 8, 4.0f);
                    AddShockwave(balls[b].x + BALL_SIZE / 2, 0, GetPrimaryColor());
                    MessageBeep(0xFFFFFFFF);
                }
                if (balls[b].y > H - BALL_SIZE) {
                    balls[b].y = H - BALL_SIZE; balls[b].dy = -balls[b].dy;
                    AddParticles(balls[b].x + BALL_SIZE / 2, (float)H, RGB(255, 255, 255), 8, 4.0f);
                    AddShockwave(balls[b].x + BALL_SIZE / 2, H, GetSecondaryColor());
                    MessageBeep(0xFFFFFFFF);
                }

                // Obstacles
                if (has_obstacles && obs1_active && balls[b].x + BALL_SIZE > W / 2 - 10 && balls[b].x < W / 2 + 10 && balls[b].y + BALL_SIZE > obs_y && balls[b].y < obs_y + 40) {
                    if (balls[b].is_fireball) { obs1_active = 0; obs1_respawn = 150; AddParticles(W / 2, obs_y + 20, RGB(255, 100, 0), 25, 8.0f); }
                    else { balls[b].dx = -balls[b].dx; AddParticles(W / 2, balls[b].y + BALL_SIZE / 2, GetPrimaryColor(), 10, 4.0f); }
                    MessageBeep(0xFFFFFFFF);
                }
                if (has_obstacles && obs2_active && balls[b].x + BALL_SIZE > obs2_x && balls[b].x < obs2_x + 40 && balls[b].y + BALL_SIZE > H / 2 - 10 && balls[b].y < H / 2 + 10) {
                    if (balls[b].is_fireball) { obs2_active = 0; obs2_respawn = 150; AddParticles(obs2_x + 20, H / 2, RGB(255, 100, 0), 25, 8.0f); }
                    else { balls[b].dy = -balls[b].dy; AddParticles(obs2_x + 20, H / 2, GetSecondaryColor(), 10, 4.0f); }
                    MessageBeep(0xFFFFFFFF);
                }

                // Stage 20 Boss Shield
                if (game_mode == 4 && campaign_level == 20 && boss_shield_hp > 0 &&
                    balls[b].x + BALL_SIZE >= W - 55 && balls[b].x <= W - 45 &&
                    balls[b].y + BALL_SIZE >= boss_shield_y && balls[b].y <= boss_shield_y + boss_shield_h && balls[b].dx > 0) {
                    if (balls[b].is_fireball) { boss_shield_hp = 0; boss_shield_timer = 360; AddParticles(W - 50, balls[b].y + BALL_SIZE / 2, RGB(255, 50, 0), 30, 10.0f); }
                    else { boss_shield_hp--; balls[b].dx = -balls[b].dx; AddParticles(W - 50, balls[b].y + BALL_SIZE / 2, RGB(255, 215, 0), 15, 6.0f); if (boss_shield_hp <= 0) boss_shield_timer = 360; }
                    MessageBeep(0xFFFFFFFF);
                }

                // Player Shields (Powerup)
                if (p1_shield_timer > 0 && balls[b].x < 10 && balls[b].dx < 0) {
                    balls[b].dx = -balls[b].dx; AddShockwave(5, (int)balls[b].y, GetPrimaryColor()); MessageBeep(MB_OK);
                }
                if (p2_shield_timer > 0 && balls[b].x > W - 10 - BALL_SIZE && balls[b].dx > 0) {
                    balls[b].dx = -balls[b].dx; AddShockwave(W - 5, (int)balls[b].y, GetSecondaryColor()); MessageBeep(MB_OK);
                }

                // P1 Paddle
                if (balls[b].x < 20 + PAD_W && balls[b].y + BALL_SIZE > p1_y && balls[b].y < p1_y + p1_pad_h) {
                    balls[b].x = 20 + PAD_W; balls[b].dx = -balls[b].dx; balls[b].last_hitter = 1; p1_hit_ripple = 1.0f;
                    if (skill_fireball_ready) { balls[b].is_fireball = 1; skill_fireball_ready = 0; AddShockwave(balls[b].x, balls[b].y + BALL_SIZE / 2, RGB(255, 100, 0)); }
                    rally++; RecordScore(rally);
                    float hit_pos = (float)((balls[b].y + BALL_SIZE / 2.0f) - (p1_y + p1_pad_h / 2.0f)) / (p1_pad_h / 2.0f);
                    balls[b].dy += (int)(hit_pos * 5.0f);
                    if (balls[b].dy > 10) balls[b].dy = 10; if (balls[b].dy < -10) balls[b].dy = -10;
                    if (balls[b].dx < 15 && balls[b].dx > -15) { balls[b].dx = balls[b].dx > 0 ? balls[b].dx + 1 : balls[b].dx - 1; }
                    COLORREF pColor = balls[b].is_fireball ? RGB(255, 100, 0) : GetPrimaryColor();
                    AddParticles((float)balls[b].x, (float)(balls[b].y + BALL_SIZE / 2), pColor, 18, 9.0f);
                    AddShockwave(balls[b].x, balls[b].y + BALL_SIZE / 2, pColor);
                    MessageBeep(0xFFFFFFFF);
                }

                // P2 Paddle
                if (balls[b].x + BALL_SIZE > W - 20 - PAD_W && balls[b].y + BALL_SIZE > p2_y && balls[b].y < p2_y + p2_pad_h) {
                    balls[b].x = W - 20 - PAD_W - BALL_SIZE; balls[b].dx = -balls[b].dx; balls[b].last_hitter = 2; balls[b].is_fireball = 0; p2_hit_ripple = 1.0f;
                    rally++; RecordScore(rally);
                    float hit_pos = (float)((balls[b].y + BALL_SIZE / 2.0f) - (p2_y + p2_pad_h / 2.0f)) / (p2_pad_h / 2.0f);
                    balls[b].dy += (int)(hit_pos * 5.0f);
                    if (game_mode == 4 && campaign_level == 20) balls[b].dy += (int)(p2_vy * 0.8f);
                    if (balls[b].dy > 12) balls[b].dy = 12; if (balls[b].dy < -12) balls[b].dy = -12;
                    if (balls[b].dx < 15 && balls[b].dx > -15) { balls[b].dx = balls[b].dx > 0 ? balls[b].dx + 1 : balls[b].dx - 1; }
                    AddParticles((float)(balls[b].x + BALL_SIZE), (float)(balls[b].y + BALL_SIZE / 2), GetSecondaryColor(), 18, 9.0f);
                    AddShockwave(balls[b].x + BALL_SIZE, balls[b].y + BALL_SIZE / 2, GetSecondaryColor());
                    MessageBeep(0xFFFFFFFF);
                }

                // Scoring
                if (balls[b].x < -15) {
                    balls[b].active = 0; p2_score++; rally = 0;
                    AddParticles(0, (float)(balls[b].y + BALL_SIZE / 2), GetSecondaryColor(), 35, 12.0f);
                    AddShockwave(0, (int)(balls[b].y + BALL_SIZE / 2), GetSecondaryColor());
                    MessageBeep(MB_ICONEXCLAMATION);
                    if (ActiveBallCount() == 0) ResetBalls();
                }
                if (balls[b].x > W + 15) {
                    balls[b].active = 0; p1_score++; rally = 0;
                    AddParticles((float)W, (float)(balls[b].y + BALL_SIZE / 2), GetPrimaryColor(), 35, 12.0f);
                    AddShockwave(W, (int)(balls[b].y + BALL_SIZE / 2), GetPrimaryColor());
                    MessageBeep(MB_ICONEXCLAMATION);
                    if (ActiveBallCount() == 0) ResetBalls();
                }
            }

            // Win Condition
            int target_score = (game_mode == 4 && campaign_level == 20) ? 7 : (game_mode == 4 ? 5 : 11);
            if (p1_score >= target_score) {
                if (game_mode == 4) {
                    campaign_level++; p1_score = 0; p2_score = 0; rally = 0; boss_shield_hp = 3; boss_shield_timer = 0;
                    if (campaign_level > 20) { win_screen = 1; game_over = 1; stats.wins++; SaveStats(); }
                    else { ResetBalls(); }
                } else {
                    game_over = 1; stats.wins++; SaveStats();
                }
            }
            if (p2_score >= target_score) {
                game_over = 1; stats.losses++; SaveStats();
            }

            UpdateParticles();
            UpdateShockwaves();

            InvalidateRect(hwnd, NULL, FALSE);
            break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int cw = clientRect.right;
            int ch = clientRect.bottom;
            if (cw == 0 || ch == 0) { EndPaint(hwnd, &ps); break; }

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, cw, ch);
            HBITMAP hOld = (HBITMAP)SelectObject(memDC, hbm);

            SetGraphicsMode(memDC, GM_ADVANCED);
            XFORM xform = {0};
            xform.eM11 = (float)cw / W;
            xform.eM22 = (float)ch / H;
            SetWorldTransform(memDC, &xform);

            HFONT hFont = CreateFontA(-12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
            HFONT hOldFont = (HFONT)SelectObject(memDC, hFont);


            // Background
            HBRUSH bg = CreateSolidBrush(GetBgColor());
            RECT fullRc = {0, 0, W, H};
            FillRect(memDC, &fullRc, bg);
            DeleteObject(bg);

            // Atmospheric Digital Dust (Loop 3)
            COLORREF dustCol = (theme_index == 1) ? RGB(0, 150, 60) : ((theme_index == 3) ? RGB(0, 150, 40) : ((theme_index == 2) ? RGB(150, 60, 120) : RGB(0, 140, 160)));
            HBRUSH dustBrush = CreateSolidBrush(dustCol);
            HPEN dustPen = CreatePen(PS_NULL, 0, 0);
            HPEN oldDP = (HPEN)SelectObject(memDC, dustPen);
            HBRUSH oldDB = (HBRUSH)SelectObject(memDC, dustBrush);
            float tNowBg = GetTickCount() * 0.001f;
            for (int i = 0; i < 40; i++) {
                int x = (int)((sinf(i * 12.34f + tNowBg * (0.2f + (i%3)*0.1f)) * (W/2) + (W/2) + tNowBg * 15.0f * (i%2 ? 1 : -1))) % W;
                if (x < 0) x += W;
                int y = (int)((cosf(i * 45.67f + tNowBg * (0.1f + (i%2)*0.1f)) * (H/2) + (H/2) - tNowBg * 20.0f)) % H;
                if (y < 0) y += H;
                int s = 1 + (i % 3);
                Ellipse(memDC, x, y, x + s*2, y + s*2);
            }
            SelectObject(memDC, oldDP); SelectObject(memDC, oldDB);
            DeleteObject(dustPen); DeleteObject(dustBrush);

            // 3D Perspective Grid Background (Loop 2)
            COLORREF gridCol = (theme_index == 2) ? RGB(100, 30, 80) : RGB(0, 70, 80);
            HPEN gridPen = CreatePen(PS_SOLID, 1, gridCol);
            HPEN oldGridPen = (HPEN)SelectObject(memDC, gridPen);
            for (int i = -W; i < W*2; i += 40) {
                MoveToEx(memDC, W/2, H/2, NULL); LineTo(memDC, i, H);
                MoveToEx(memDC, W/2, H/2, NULL); LineTo(memDC, i, 0);
            }
            int tNow = GetTickCount() / 15;
            for (int i = 1; i <= 12; i++) {
                int yOff = ((i * 10 + tNow) % 120);
                int yBot = H/2 + (yOff * yOff) / 30;
                int yTop = H/2 - (yOff * yOff) / 30;
                MoveToEx(memDC, 0, yBot, NULL); LineTo(memDC, W, yBot);
                MoveToEx(memDC, 0, yTop, NULL); LineTo(memDC, W, yTop);
            }
            SelectObject(memDC, oldGridPen); DeleteObject(gridPen);

            // Scanlines
            HPEN scanPen = CreatePen(PS_SOLID, 1, RGB(4, 6, 10));
            HPEN oldPen = (HPEN)SelectObject(memDC, scanPen);
            for (int y = 0; y < H; y += 4) { MoveToEx(memDC, 0, y, NULL); LineTo(memDC, W, y); }
            SelectObject(memDC, oldPen); DeleteObject(scanPen);

            // Border Accent
            COLORREF borderCol = (skill_slow_timer > 0) ? GetPrimaryColor() : RGB(0, 80, 120);
            HPEN borderPen = CreatePen(PS_SOLID, 1, borderCol);
            oldPen = (HPEN)SelectObject(memDC, borderPen);
            MoveToEx(memDC, 2, 2, NULL); LineTo(memDC, W - 2, 2);
            LineTo(memDC, W - 2, H - 2); LineTo(memDC, 2, H - 2); LineTo(memDC, 2, 2);
            SelectObject(memDC, oldPen); DeleteObject(borderPen);

            // Center Line & Arena Circle
            HPEN dashPen = CreatePen(PS_DOT, 1, RGB(60, 80, 100));
            oldPen = (HPEN)SelectObject(memDC, dashPen);
            MoveToEx(memDC, W / 2, 0, NULL); LineTo(memDC, W / 2, H);
            SelectObject(memDC, oldPen); DeleteObject(dashPen);

            HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            HPEN circlePen = CreatePen(PS_SOLID, 1, RGB(30, 45, 60));
            oldPen = (HPEN)SelectObject(memDC, circlePen);
            HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, nullBrush);
            Ellipse(memDC, W / 2 - 45, H / 2 - 45, W / 2 + 45, H / 2 + 45);
            SelectObject(memDC, oldPen); SelectObject(memDC, oldBrush); DeleteObject(circlePen);

            // Gravity Well
            if (game_mode == 1 || (game_mode == 4 && campaign_level >= 9)) {
                HPEN gwPen = CreatePen(PS_SOLID, 1, GetSecondaryColor());
                oldPen = (HPEN)SelectObject(memDC, gwPen); oldBrush = (HBRUSH)SelectObject(memDC, nullBrush);
                int r_gw = 35 + (rand() % 6);
                Ellipse(memDC, gw_x - r_gw, gw_y - r_gw, gw_x + r_gw, gw_y + r_gw);
                Ellipse(memDC, gw_x - 15, gw_y - 15, gw_x + 15, gw_y + 15);
                SelectObject(memDC, oldPen); SelectObject(memDC, oldBrush); DeleteObject(gwPen);
            }

            // Portals
            if (game_mode == 1 || (game_mode == 4 && campaign_level >= 13)) {
                HPEN p1Pen = CreatePen(PS_SOLID, 2, GetPrimaryColor());
                HPEN p2Pen = CreatePen(PS_SOLID, 2, GetSecondaryColor());
                oldBrush = (HBRUSH)SelectObject(memDC, nullBrush);
                oldPen = (HPEN)SelectObject(memDC, p1Pen); Ellipse(memDC, portal1_x - 14, portal1_y - 14, portal1_x + 14, portal1_y + 14);
                SelectObject(memDC, p2Pen); Ellipse(memDC, portal2_x - 14, portal2_y - 14, portal2_x + 14, portal2_y + 14);
                SelectObject(memDC, oldPen); SelectObject(memDC, oldBrush); DeleteObject(p1Pen); DeleteObject(p2Pen);
            }

            // Obstacles
            int has_obstacles = (game_mode == 1 || (game_mode == 4 && campaign_level >= 5));
            if (has_obstacles && obs1_active) {
                HBRUSH obsB = CreateSolidBrush(RGB(25, 35, 50)); HPEN obsP = CreatePen(PS_SOLID, 1, GetPrimaryColor());
                oldPen = (HPEN)SelectObject(memDC, obsP); oldBrush = (HBRUSH)SelectObject(memDC, obsB);
                Rectangle(memDC, W / 2 - 10, obs_y, W / 2 + 10, obs_y + 40);
                SelectObject(memDC, oldPen); SelectObject(memDC, oldBrush); DeleteObject(obsB); DeleteObject(obsP);
            }
            if (has_obstacles && obs2_active) {
                HBRUSH obsB = CreateSolidBrush(RGB(35, 25, 50)); HPEN obsP = CreatePen(PS_SOLID, 1, GetSecondaryColor());
                oldPen = (HPEN)SelectObject(memDC, obsP); oldBrush = (HBRUSH)SelectObject(memDC, obsB);
                Rectangle(memDC, obs2_x, H / 2 - 10, obs2_x + 40, H / 2 + 10);
                SelectObject(memDC, oldPen); SelectObject(memDC, oldBrush); DeleteObject(obsB); DeleteObject(obsP);
            }

            // Boss Shield
            if (game_mode == 4 && campaign_level == 20 && boss_shield_hp > 0) {
                COLORREF shCol = (boss_shield_hp == 3) ? GetPrimaryColor() : ((boss_shield_hp == 2) ? RGB(255, 215, 0) : RGB(255, 51, 102));
                HBRUSH shB = CreateSolidBrush(shCol); HPEN shP = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                oldPen = (HPEN)SelectObject(memDC, shP); oldBrush = (HBRUSH)SelectObject(memDC, shB);
                RoundRect(memDC, W - 55, boss_shield_y, W - 45, boss_shield_y + boss_shield_h, 6, 6);
                SelectObject(memDC, oldPen); SelectObject(memDC, oldBrush); DeleteObject(shB); DeleteObject(shP);
            }

            // Player Protective Shields
            if (p1_shield_timer > 0) {
                HPEN shP = CreatePen(PS_SOLID, 3, GetPrimaryColor()); oldPen = (HPEN)SelectObject(memDC, shP);
                MoveToEx(memDC, 4, 0, NULL); LineTo(memDC, 4, H); SelectObject(memDC, oldPen); DeleteObject(shP);
            }
            if (p2_shield_timer > 0) {
                HPEN shP = CreatePen(PS_SOLID, 3, GetSecondaryColor()); oldPen = (HPEN)SelectObject(memDC, shP);
                MoveToEx(memDC, W - 4, 0, NULL); LineTo(memDC, W - 4, H); SelectObject(memDC, oldPen); DeleteObject(shP);
            }

            // Render Ball Trails & Balls
            for (int b = 0; b < MAX_BALLS; b++) {
                if (!balls[b].active) continue;
                for (int i = balls[b].trail_count - 1; i >= 0; i--) {
                    int factor = (MAX_TRAIL - i); int r = (BALL_SIZE / 2) * factor / MAX_TRAIL; if (r < 1) r = 1;
                    BYTE red = (BYTE)((GetRValue(balls[b].trail[i].color) * factor) / MAX_TRAIL);
                    BYTE green = (BYTE)((GetGValue(balls[b].trail[i].color) * factor) / MAX_TRAIL);
                    BYTE blue = (BYTE)((GetBValue(balls[b].trail[i].color) * factor) / MAX_TRAIL);
                    HBRUSH tBrush = CreateSolidBrush(RGB(red, green, blue)); HPEN tPen = CreatePen(PS_SOLID, 1, RGB(red, green, blue));
                    HPEN pOld = (HPEN)SelectObject(memDC, tPen); HBRUSH bOld = (HBRUSH)SelectObject(memDC, tBrush);
                    Ellipse(memDC, balls[b].trail[i].x - r, balls[b].trail[i].y - r, balls[b].trail[i].x + r, balls[b].trail[i].y + r);
                    SelectObject(memDC, pOld); SelectObject(memDC, bOld); DeleteObject(tBrush); DeleteObject(tPen);
                }

                COLORREF ballGlowColor = balls[b].is_fireball ? RGB(255, 100, 0) :
                    ((balls[b].last_hitter == 1) ? GetPrimaryColor() :
                    ((balls[b].last_hitter == 2) ? GetSecondaryColor() : RGB(255, 255, 255)));

                HBRUSH ballB = CreateSolidBrush(balls[b].is_fireball ? RGB(255, 200, 50) : RGB(200, 200, 200));
                HPEN ballP = CreatePen(PS_SOLID, 2, ballGlowColor);
                oldPen = (HPEN)SelectObject(memDC, ballP); oldBrush = (HBRUSH)SelectObject(memDC, ballB);
                Ellipse(memDC, (int)balls[b].x - 1, (int)balls[b].y - 1, (int)balls[b].x + BALL_SIZE + 1, (int)balls[b].y + BALL_SIZE + 1);
                SelectObject(memDC, oldPen); SelectObject(memDC, oldBrush); DeleteObject(ballB); DeleteObject(ballP);
                
                // Glass-like specular reflection shifted based on velocity
                float spd = sqrtf(balls[b].dx * balls[b].dx + balls[b].dy * balls[b].dy);
                float normDx = spd > 0.0f ? balls[b].dx / spd : 0.0f;
                float normDy = spd > 0.0f ? balls[b].dy / spd : 0.0f;
                
                int specX = (int)(balls[b].x + BALL_SIZE / 2 - normDx * (BALL_SIZE / 4.0f));
                int specY = (int)(balls[b].y + BALL_SIZE / 2 - normDy * (BALL_SIZE / 4.0f));
                
                HBRUSH specB = CreateSolidBrush(RGB(255, 255, 255));
                HPEN specP = CreatePen(PS_NULL, 0, 0);
                oldPen = (HPEN)SelectObject(memDC, specP); oldBrush = (HBRUSH)SelectObject(memDC, specB);
                Ellipse(memDC, specX - 2, specY - 2, specX + 2, specY + 2);
                SelectObject(memDC, oldPen); SelectObject(memDC, oldBrush); DeleteObject(specB); DeleteObject(specP);
            }

            // Draw Paddles
            {
                int x = 20, y = p1_y, w = PAD_W, h = p1_pad_h;
                
                float stretch = p1_vy > 0 ? p1_vy * 0.02f : -p1_vy * 0.02f;
                if (stretch > 0.4f) stretch = 0.4f;
                float squash = p1_hit_ripple * 0.4f;
                float scaleY = 1.0f + stretch - squash;
                float scaleX = 1.0f - stretch*0.5f + squash * 1.5f;
                int cX = x + w / 2, cY = y + h / 2;
                int drawW = (int)(w * scaleX), drawH = (int)(h * scaleY);
                int drawX = cX - drawW / 2, drawY = cY - drawH / 2;

                COLORREF mainColor = (skill_mega_timer > 0) ? RGB(255, 215, 0) : GetPrimaryColor();
                COLORREF darkColor = (skill_mega_timer > 0) ? RGB(100, 80, 0) : RGB(0, 50, 80);
                
                if (p1_buff_timer > 0 || skill_mega_timer > 0) {
                    HBRUSH goldB = CreateSolidBrush(RGB(255, 215, 0)); RECT rG = { drawX - 2, drawY - 2, drawX + drawW + 2, drawY + drawH + 2 };
                    FrameRect(memDC, &rG, goldB); DeleteObject(goldB);
                }
                
                HBRUSH p1B = CreateSolidBrush(darkColor); HPEN p1P = CreatePen(PS_SOLID, 1, mainColor);
                HPEN pOld = (HPEN)SelectObject(memDC, p1P); HBRUSH bOld = (HBRUSH)SelectObject(memDC, p1B);
                RoundRect(memDC, drawX, drawY, drawX + drawW, drawY + drawH, 6, 6);
                
                // Procedural metallic reflection
                float tOffset = GetTickCount() * 0.002f;
                int mPosOffset = (int)(sinf(tOffset) * (drawH / 3.0f));
                int hlY = drawY + drawH / 2 + mPosOffset - 4;
                if (hlY < drawY + 2) hlY = drawY + 2;
                if (hlY + 8 > drawY + drawH - 2) hlY = drawY + drawH - 10;
                if (hlY < drawY + 2) hlY = drawY + 2; // bound again
                RECT hlRect = { drawX + 2, hlY, drawX + drawW - 2, hlY + 8 };
                HBRUSH hlB = CreateSolidBrush(RGB(150, 200, 255));
                FillRect(memDC, &hlRect, hlB);
                DeleteObject(hlB);
                
                // Animated energy conduits
                int conduitThickness = 1 + (int)((p1_vy > 0 ? p1_vy : -p1_vy) * 0.15f);
                if (conduitThickness > 3) conduitThickness = 3;
                HPEN coreP = CreatePen(PS_SOLID, conduitThickness, RGB(255, 255, 255)); SelectObject(memDC, coreP);
                int dashOffset = (GetTickCount() / 20 * (p1_vy < 0 ? -1 : 1)) % 8;
                if (dashOffset < 0) dashOffset += 8;
                for (int cy = drawY + 4 + dashOffset; cy < drawY + drawH - 4; cy += 8) {
                    if (cy + 4 <= drawY + drawH - 4) {
                        MoveToEx(memDC, drawX + drawW / 2, cy, NULL); LineTo(memDC, drawX + drawW / 2, cy + 4);
                    }
                }
                
                SelectObject(memDC, pOld); SelectObject(memDC, bOld); DeleteObject(p1B); DeleteObject(p1P); DeleteObject(coreP);

                if (p1_hit_ripple > 0.0f) {
                    int rSize = (int)((1.0f - p1_hit_ripple) * 10.0f);
                    HPEN rP = CreatePen(PS_SOLID, 2, RGB((BYTE)(255 * p1_hit_ripple), (BYTE)(255 * p1_hit_ripple), (BYTE)(255 * p1_hit_ripple)));
                    pOld = (HPEN)SelectObject(memDC, rP); bOld = (HBRUSH)SelectObject(memDC, (HBRUSH)GetStockObject(NULL_BRUSH));
                    RoundRect(memDC, drawX - rSize, drawY - rSize, drawX + drawW + rSize, drawY + drawH + rSize, 6, 6);
                    SelectObject(memDC, pOld); SelectObject(memDC, bOld); DeleteObject(rP);
                }
            }
            {
                int x = W - 20 - PAD_W, y = p2_y, w = PAD_W, h = p2_pad_h;
                
                float stretch = p2_vy > 0 ? p2_vy * 0.02f : -p2_vy * 0.02f;
                if (stretch > 0.4f) stretch = 0.4f;
                float squash = p2_hit_ripple * 0.4f;
                float scaleY = 1.0f + stretch - squash;
                float scaleX = 1.0f - stretch*0.5f + squash * 1.5f;
                int cX = x + w / 2, cY = y + h / 2;
                int drawW = (int)(w * scaleX), drawH = (int)(h * scaleY);
                int drawX = cX - drawW / 2, drawY = cY - drawH / 2;

                COLORREF mainColor = (game_mode == 4 && campaign_level == 20) ? RGB(255, 50, 0) : GetSecondaryColor();
                COLORREF darkColor = (game_mode == 4 && campaign_level == 20) ? RGB(100, 10, 0) : RGB(80, 0, 50);
                HBRUSH p2B = CreateSolidBrush(darkColor); HPEN p2P = CreatePen(PS_SOLID, 1, mainColor);
                HPEN pOld = (HPEN)SelectObject(memDC, p2P); HBRUSH bOld = (HBRUSH)SelectObject(memDC, p2B);
                RoundRect(memDC, drawX, drawY, drawX + drawW, drawY + drawH, 6, 6);
                
                // Procedural metallic reflection
                float tOffset = GetTickCount() * 0.002f;
                int mPosOffset = (int)(sinf(tOffset + 3.14f) * (drawH / 3.0f));
                int hlY = drawY + drawH / 2 + mPosOffset - 4;
                if (hlY < drawY + 2) hlY = drawY + 2;
                if (hlY + 8 > drawY + drawH - 2) hlY = drawY + drawH - 10;
                if (hlY < drawY + 2) hlY = drawY + 2;
                RECT hlRect = { drawX + 2, hlY, drawX + drawW - 2, hlY + 8 };
                HBRUSH hlB = CreateSolidBrush(RGB(150, 200, 255));
                FillRect(memDC, &hlRect, hlB);
                DeleteObject(hlB);
                
                int conduitThickness = 1 + (int)((p2_vy > 0 ? p2_vy : -p2_vy) * 0.15f);
                if (conduitThickness > 3) conduitThickness = 3;
                HPEN coreP = CreatePen(PS_SOLID, conduitThickness, RGB(255, 255, 255)); SelectObject(memDC, coreP);
                int dashOffset = (GetTickCount() / 20 * (p2_vy < 0 ? -1 : 1)) % 8;
                if (dashOffset < 0) dashOffset += 8;
                for (int cy = drawY + 4 + dashOffset; cy < drawY + drawH - 4; cy += 8) {
                    if (cy + 4 <= drawY + drawH - 4) {
                        MoveToEx(memDC, drawX + drawW / 2, cy, NULL); LineTo(memDC, drawX + drawW / 2, cy + 4);
                    }
                }
                
                SelectObject(memDC, pOld); SelectObject(memDC, bOld); DeleteObject(p2B); DeleteObject(p2P); DeleteObject(coreP);

                if (p2_hit_ripple > 0.0f) {
                    int rSize = (int)((1.0f - p2_hit_ripple) * 10.0f);
                    HPEN rP = CreatePen(PS_SOLID, 2, RGB((BYTE)(255 * p2_hit_ripple), (BYTE)(255 * p2_hit_ripple), (BYTE)(255 * p2_hit_ripple)));
                    pOld = (HPEN)SelectObject(memDC, rP); bOld = (HBRUSH)SelectObject(memDC, (HBRUSH)GetStockObject(NULL_BRUSH));
                    RoundRect(memDC, drawX - rSize, drawY - rSize, drawX + drawW + rSize, drawY + drawH + rSize, 6, 6);
                    SelectObject(memDC, pOld); SelectObject(memDC, bOld); DeleteObject(rP);
                }
            }

            // Powerup Capsule Graphic
            if (powerup_x != -1) {
                COLORREF puColor = RGB(255,255,255);
                if(powerup_type == 0) puColor = RGB(255, 215, 0);
                else if(powerup_type == 1) puColor = RGB(255, 51, 102);
                else if(powerup_type == 2) puColor = RGB(0, 229, 255);
                else if(powerup_type == 3) puColor = RGB(200, 50, 255);
                else if(powerup_type == 4) puColor = RGB(255, 153, 0);
                else if(powerup_type == 5) puColor = RGB(0, 255, 170);

                HPEN puP = CreatePen(PS_SOLID, 2, puColor);
                oldPen = (HPEN)SelectObject(memDC, puP);
                HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, (HBRUSH)GetStockObject(NULL_BRUSH));

                if (powerup_type == 0) { // Expand: Plus shape
                    MoveToEx(memDC, powerup_x + 9, powerup_y + 3, NULL); LineTo(memDC, powerup_x + 9, powerup_y + 15);
                    MoveToEx(memDC, powerup_x + 3, powerup_y + 9, NULL); LineTo(memDC, powerup_x + 15, powerup_y + 9);
                } else if (powerup_type == 1) { // Shrink: Minus
                    MoveToEx(memDC, powerup_x + 4, powerup_y + 9, NULL); LineTo(memDC, powerup_x + 14, powerup_y + 9);
                } else if (powerup_type == 2) { // Freeze: Diamond
                    MoveToEx(memDC, powerup_x + 9, powerup_y + 2, NULL); LineTo(memDC, powerup_x + 16, powerup_y + 9);
                    LineTo(memDC, powerup_x + 9, powerup_y + 16); LineTo(memDC, powerup_x + 2, powerup_y + 9); LineTo(memDC, powerup_x + 9, powerup_y + 2);
                } else if (powerup_type == 3) { // Multi: 3 small circles
                    Ellipse(memDC, powerup_x+3, powerup_y+9, powerup_x+7, powerup_y+13);
                    Ellipse(memDC, powerup_x+11, powerup_y+9, powerup_x+15, powerup_y+13);
                    Ellipse(memDC, powerup_x+7, powerup_y+3, powerup_x+11, powerup_y+7);
                } else if (powerup_type == 4) { // Speed: Double arrows
                    MoveToEx(memDC, powerup_x+4, powerup_y+4, NULL); LineTo(memDC, powerup_x+10, powerup_y+9); LineTo(memDC, powerup_x+4, powerup_y+14);
                    MoveToEx(memDC, powerup_x+9, powerup_y+4, NULL); LineTo(memDC, powerup_x+15, powerup_y+9); LineTo(memDC, powerup_x+9, powerup_y+14);
                } else if (powerup_type == 5) { // Shield: Hexagon or Shield
                    MoveToEx(memDC, powerup_x+4, powerup_y+3, NULL); LineTo(memDC, powerup_x+14, powerup_y+3);
                    LineTo(memDC, powerup_x+14, powerup_y+10); LineTo(memDC, powerup_x+9, powerup_y+16);
                    LineTo(memDC, powerup_x+4, powerup_y+10); LineTo(memDC, powerup_x+4, powerup_y+3);
                }
                SelectObject(memDC, oldPen); SelectObject(memDC, oldBrush); DeleteObject(puP);
            }

            // Shockwaves & Particles
            for (int i = 0; i < MAX_SHOCKWAVES; i++) {
                if (shockwaves[i].radius > 0) {
                    HPEN swP = CreatePen(PS_SOLID, 1, shockwaves[i].color);
                    oldPen = (HPEN)SelectObject(memDC, swP); oldBrush = (HBRUSH)SelectObject(memDC, nullBrush);
                    int r = shockwaves[i].radius; Ellipse(memDC, shockwaves[i].x - r, shockwaves[i].y - r, shockwaves[i].x + r, shockwaves[i].y + r);
                    SelectObject(memDC, oldPen); SelectObject(memDC, oldBrush); DeleteObject(swP);
                }
            }
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (particles[i].life > 0) {
                    HBRUSH pB = CreateSolidBrush(particles[i].color);
                    RECT pR = { (int)particles[i].x - particles[i].size / 2, (int)particles[i].y - particles[i].size / 2, (int)particles[i].x + particles[i].size / 2 + 1, (int)particles[i].y + particles[i].size / 2 + 1 };
                    FillRect(memDC, &pR, pB); DeleteObject(pB);
                }
            }

            // HUD Header
            SetBkMode(memDC, TRANSPARENT);
            char scoreStr[48];
            wsprintfA(scoreStr, "%d - %d", p1_score, p2_score);
            SetTextColor(memDC, RGB(255, 255, 255));
            TextOutA(memDC, W / 2 - 20, 8, scoreStr, lstrlenA(scoreStr));

            char modeHud[80];
            char* mNames[] = {"Classic", "Obstacles", "Frenzy", "Multi-Ball", "Campaign"};
            wsprintfA(modeHud, "%s | %s | 'H' Help", is_pvp ? "2P PVP" : "1P AI", mNames[game_mode]);
            SetTextColor(memDC, RGB(255, 215, 0));
            TextOutA(memDC, W / 2 - 65, 26, modeHud, lstrlenA(modeHud));

            // Skills Footer HUD Bar
            char skillsStr[128];
            char sSlow[16], sMega[16], sFire[16];
            if (skill_slow_timer > 0) wsprintfA(sSlow, "ACTIVE"); else if (skill_slow_cooldown == 0) wsprintfA(sSlow, "READY"); else wsprintfA(sSlow, "%ds", skill_slow_cooldown / 30);
            if (skill_mega_timer > 0) wsprintfA(sMega, "ACTIVE"); else if (skill_mega_cooldown == 0) wsprintfA(sMega, "READY"); else wsprintfA(sMega, "%ds", skill_mega_cooldown / 30);
            if (skill_fireball_ready) wsprintfA(sFire, "READY!"); else if (skill_fireball_cooldown == 0) wsprintfA(sFire, "READY"); else wsprintfA(sFire, "%ds", skill_fireball_cooldown / 30);

            wsprintfA(skillsStr, "[F]Slow:%s [E]Mega:%s [B]Fire:%s [M]Mode [T]Theme [F5]Save", sSlow, sMega, sFire);
            SetTextColor(memDC, GetPrimaryColor());
            TextOutA(memDC, 8, H - 20, skillsStr, lstrlenA(skillsStr));

            // Stats / Leaderboard Overlay
            if (show_stats_overlay) {
                HBRUSH statB = CreateSolidBrush(RGB(10, 18, 30));
                RECT statR = { 30, 30, W - 30, H - 30 };
                FillRect(memDC, &statR, statB);
                FrameRect(memDC, &statR, (HBRUSH)GetStockObject(WHITE_BRUSH));
                DeleteObject(statB);

                SetTextColor(memDC, RGB(0, 229, 255));
                TextOutA(memDC, 45, 40, "=== LEADERBOARD & STATS ===", 27);

                char stBuf[64];
                wsprintfA(stBuf, "Games: %d  Wins: %d  Losses: %d  High Rally: %d", stats.total_games, stats.wins, stats.losses, stats.high_rally);
                SetTextColor(memDC, RGB(255, 255, 255));
                TextOutA(memDC, 45, 60, stBuf, lstrlenA(stBuf));

                TextOutA(memDC, 45, 85, "Top Scores:", 11);
                for (int i = 0; i < 5; i++) {
                    if (stats.top_scores[i].rally > 0) {
                        char scLine[64];
                        wsprintfA(scLine, "#%d Rally: %d  Date: %s", i + 1, stats.top_scores[i].rally, stats.top_scores[i].date);
                        TextOutA(memDC, 55, 105 + (i * 20), scLine, lstrlenA(scLine));
                    }
                }
                TextOutA(memDC, 45, H - 55, "Press 'L' to Close Overlay", 26);
            }

            if (show_help_overlay) {
                HBRUSH helpB = CreateSolidBrush(RGB(10, 30, 20));
                RECT helpR = { 30, 30, W - 30, H - 30 };
                FillRect(memDC, &helpR, helpB);
                FrameRect(memDC, &helpR, (HBRUSH)GetStockObject(WHITE_BRUSH));
                DeleteObject(helpB);

                SetTextColor(memDC, RGB(0, 255, 150));
                TextOutA(memDC, 45, 40, "=== HOW TO PLAY ===", 19);

                SetTextColor(memDC, RGB(255, 255, 255));
                TextOutA(memDC, 45, 65, "P1: W/S (or Up/Down)", 20);
                TextOutA(memDC, 45, 85, "P2: Up/Down (in PvP)", 20);
                TextOutA(memDC, 45, 110, "Skills: [F] Slow, [E] Mega, [B] Fireball", 40);
                TextOutA(memDC, 45, 130, "System: [Space] Pause, [M] Mode", 31);
                TextOutA(memDC, 45, 150, "        [V] PvP, [T] Theme, [L] Stats", 37);
                TextOutA(memDC, 45, 175, "[F5] Save Game  |  [F9] Load Game", 33);
                
                SetTextColor(memDC, RGB(200, 200, 200));
                TextOutA(memDC, 45, H - 55, "Press 'H' to Close Help", 23);
            }

            if (is_paused) {
                HBRUSH overlayB = CreateSolidBrush(RGB(5, 10, 20));
                RECT overR = { 0, 0, W, H };
                FillRect(memDC, &overR, overlayB);
                DeleteObject(overlayB);
                SetTextColor(memDC, RGB(255, 255, 255));
                TextOutA(memDC, W / 2 - 45, H / 2 - 10, "GAME PAUSED", 11);
                TextOutA(memDC, W / 2 - 75, H / 2 + 10, "Press Space to Resume", 21);
            }

            if (game_over) {
                HBRUSH overlayB = CreateSolidBrush(RGB(5, 10, 20));
                RECT overR = { 0, 0, W, H };
                FillRect(memDC, &overR, overlayB);
                DeleteObject(overlayB);

                if (win_screen) {
                    SetTextColor(memDC, RGB(255, 215, 0));
                    char* winStr = "CAMPAIGN VICTORY! 20 STAGES CLEARED!";
                    TextOutA(memDC, W / 2 - 120, H / 2 - 20, winStr, lstrlenA(winStr));
                } else {
                    SetTextColor(memDC, (p1_score > p2_score) ? GetPrimaryColor() : GetSecondaryColor());
                    char* winStr = (p1_score > p2_score) ? (is_pvp ? "PLAYER 1 WINS!" : "YOU WIN!") : (is_pvp ? "PLAYER 2 WINS!" : "CPU WINS!");
                    TextOutA(memDC, W / 2 - 45, H / 2 - 20, winStr, lstrlenA(winStr));
                }
                SetTextColor(memDC, RGB(255, 255, 255));
                char* restartStr = "Press 'R' to Restart";
                TextOutA(memDC, W / 2 - 60, H / 2 + 10, restartStr, lstrlenA(restartStr));
            }

            SelectObject(memDC, hOldFont);
            DeleteObject(hFont);
            
            ModifyWorldTransform(memDC, NULL, MWT_IDENTITY);
            BitBlt(hdc, 0, 0, cw, ch, memDC, 0, 0, SRCCOPY);
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
    SetProcessDPIAware();
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    LoadStats();
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KPongApp";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    RECT winRect = { 0, 0, 950, 750 };
    HDC screenDC = GetDC(NULL);
    int dpi = GetDeviceCaps(screenDC, LOGPIXELSX);
    ReleaseDC(NULL, screenDC);
    float scale = dpi / 96.0f;
    winRect.right = (int)(winRect.right * scale);
    winRect.bottom = (int)(winRect.bottom * scale);
    
    AdjustWindowRect(&winRect, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);
    HWND hwnd = CreateWindowEx(0, "KPongApp", "KPong - Press 'H' for Help", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, winRect.right - winRect.left, winRect.bottom - winRect.top, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
