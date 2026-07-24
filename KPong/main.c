#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#pragma comment(lib, "msvcrt.lib")

#define W 400
#define H 300
#define PAD_W 10
#define BALL_SIZE 10
#define TIMER_ID 1

#define MAX_PARTICLES 80
#define MAX_TRAIL 10
#define MAX_SHOCKWAVES 10
#define MAX_BALLS 5

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

Particle particles[MAX_PARTICLES];
Shockwave shockwaves[MAX_SHOCKWAVES];
Ball balls[MAX_BALLS];

int p1_pad_h = 50;
int p2_pad_h = 50;
int campaign_mode = 0;
int campaign_level = 1;
int win_screen = 0;
int last_hitter = 0;

int powerup_x = -1;
int powerup_y = -1;
int powerup_active = 0;
int powerup_type = 0;

int p1_buff_timer = 0;
int p2_buff_timer = 0;
int p1_debuff_timer = 0;
int p2_debuff_timer = 0;
int p1_freeze_timer = 0;
int p2_freeze_timer = 0;

// Active Skills
int skill_slow_timer = 0;
int skill_slow_cooldown = 0;
int skill_mega_timer = 0;
int skill_mega_cooldown = 0;
int skill_fireball_ready = 0;
int skill_fireball_cooldown = 0;

// Stats
int high_rally = 0;
int lifetime_wins = 0;

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
    FILE* f = fopen("kpong_stats.dat", "rb");
    if (f) {
        fread(&high_rally, sizeof(int), 1, f);
        fread(&lifetime_wins, sizeof(int), 1, f);
        fclose(f);
    }
}

void SaveStats() {
    FILE* f = fopen("kpong_stats.dat", "wb");
    if (f) {
        fwrite(&high_rally, sizeof(int), 1, f);
        fwrite(&lifetime_wins, sizeof(int), 1, f);
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
    balls[0].x = W / 2 - BALL_SIZE / 2;
    balls[0].y = H / 2 - BALL_SIZE / 2;
    int spd = campaign_mode ? (4 + (campaign_level / 2)) : start_speeds[difficulty];
    if (spd > 11) spd = 11;
    balls[0].dx = (rand() % 2 == 0) ? spd : -spd;
    balls[0].dy = (rand() % 2 == 0) ? 3 : -3;
    balls[0].active = 1;
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
            AddShockwave(W / 2, H / 2, RGB(200, 50, 255));
            AddParticles(W / 2, H / 2, RGB(200, 50, 255), 15, 6.0f);
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
        case WM_TIMER:
            if (game_over) {
                if (GetAsyncKeyState('R') & 0x8000) {
                    p1_score = 0;
                    p2_score = 0;
                    rally = 0;
                    game_over = 0;
                    win_screen = 0;
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

            if (GetAsyncKeyState('1') & 0x8000) difficulty = 1;
            if (GetAsyncKeyState('2') & 0x8000) difficulty = 2;
            if (GetAsyncKeyState('3') & 0x8000) difficulty = 3;
            if ((GetAsyncKeyState('C') & 0x8000) && !campaign_mode) {
                campaign_mode = 1; campaign_level = 1; p1_score = 0; p2_score = 0; rally = 0;
                boss_shield_hp = 3; boss_shield_timer = 0;
                obs1_active = 1; obs2_active = 1;
                ResetBalls();
            }

            // Skills Activation (F, P, B keys)
            if ((GetAsyncKeyState('F') & 0x8000) && skill_slow_cooldown == 0) {
                skill_slow_timer = 180;    // 6s @ 30fps
                skill_slow_cooldown = 360; // 12s cooldown
                AddShockwave(W / 2, H / 2, RGB(0, 229, 255));
                MessageBeep(MB_OK);
            }
            if ((GetAsyncKeyState('P') & 0x8000) && skill_mega_cooldown == 0) {
                skill_mega_timer = 240;    // 8s @ 30fps
                skill_mega_cooldown = 450; // 15s cooldown
                AddShockwave(20, p1_y + p1_pad_h / 2, RGB(255, 215, 0));
                MessageBeep(MB_OK);
            }
            if ((GetAsyncKeyState('B') & 0x8000) && skill_fireball_cooldown == 0) {
                skill_fireball_ready = 1;
                skill_fireball_cooldown = 300; // 10s cooldown
                AddShockwave(20, p1_y + p1_pad_h / 2, RGB(255, 100, 0));
                MessageBeep(MB_OK);
            }

            // Skill & Buff Timers Update
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

            // Dynamic Paddle Sizes
            p1_pad_h = 50 - (rally * 2); if (p1_pad_h < 20) p1_pad_h = 20;
            p2_pad_h = 50 - (rally * 2); if (p2_pad_h < 20) p2_pad_h = 20;

            if (campaign_mode && campaign_level >= 3 && campaign_level < 20) { p2_pad_h -= 10; }
            if (campaign_mode && (campaign_level == 10 || campaign_level == 15)) { p2_pad_h += 40; }
            if (campaign_mode && campaign_level == 20) { p2_pad_h = 75; } // Boss Paddle

            if (p1_buff_timer > 0) p1_pad_h += 30;
            if (p2_buff_timer > 0) p2_pad_h += 30;
            if (p1_debuff_timer > 0) p1_pad_h = 15;
            if (p2_debuff_timer > 0) p2_pad_h = 15;

            if (skill_mega_timer > 0) {
                p1_pad_h = (int)(p1_pad_h * 2.5f);
            }

            if (p1_pad_h < 15) p1_pad_h = 15;
            if (p2_pad_h < 15) p2_pad_h = 15;

            // Player 1 Input & Velocity
            int p1_prev_y = p1_y;
            if (p1_freeze_timer == 0) {
                if ((GetAsyncKeyState(VK_UP) & 0x8000) || (GetAsyncKeyState('W') & 0x8000)) p1_y -= 6;
                if ((GetAsyncKeyState(VK_DOWN) & 0x8000) || (GetAsyncKeyState('S') & 0x8000)) p1_y += 6;
            }
            if (p1_y < 0) p1_y = 0;
            if (p1_y > H - p1_pad_h) p1_y = H - p1_pad_h;
            p1_vy = p1_y - p1_prev_y;

            // CPU AI Speed Logic across 20 stages
            int ai_spd = ai_speeds[difficulty];
            if (campaign_mode) {
                if (campaign_level == 1) ai_spd = 2;
                else if (campaign_level == 2) ai_spd = 3;
                else if (campaign_level == 3) ai_spd = 4;
                else if (campaign_level == 4) ai_spd = 5;
                else if (campaign_level == 5) ai_spd = 6;
                else if (campaign_level == 6) ai_spd = 7;
                else if (campaign_level == 7) ai_spd = 8;
                else if (campaign_level == 8) ai_spd = 9;
                else if (campaign_level == 9) ai_spd = 10;
                else if (campaign_level <= 12) ai_spd = 12;
                else if (campaign_level <= 15) ai_spd = 15;
                else if (campaign_level <= 19) ai_spd = 18;
                else ai_spd = 22; // Stage 20 Hyper-CPU Boss
            }

            // Target ball selection for AI (closest active ball moving right)
            int target_ball = 0;
            float min_x = -1.0f;
            for (int i = 0; i < MAX_BALLS; i++) {
                if (balls[i].active && balls[i].dx > 0 && balls[i].x > min_x) {
                    min_x = balls[i].x;
                    target_ball = i;
                }
            }

            int p2_prev_y = p2_y;
            if (p2_freeze_timer == 0) {
                int target_y = (int)(balls[target_ball].y);
                if (target_y > p2_y + p2_pad_h / 2 + 8) p2_y += ai_spd;
                if (target_y < p2_y + p2_pad_h / 2 - 8) p2_y -= ai_spd;
            }
            if (p2_y < 0) p2_y = 0;
            if (p2_y > H - p2_pad_h) p2_y = H - p2_pad_h;
            p2_vy = p2_y - p2_prev_y;

            // Powerup Pickup Spawning
            if (powerup_active > 0) powerup_active--;
            if (powerup_x == -1 && rand() % 180 == 0) {
                powerup_x = W / 4 + rand() % (W / 2);
                powerup_y = H / 4 + rand() % (H / 2);
                powerup_active = 250;
                powerup_type = rand() % 4; // 0: Expand, 1: Shrink, 2: Freeze, 3: Multi-Ball
            }
            if (powerup_active == 0) { powerup_x = -1; powerup_y = -1; }

            // Respawn timers for smashed obstacles
            if (!obs1_active) {
                obs1_respawn--;
                if (obs1_respawn <= 0) obs1_active = 1;
            }
            if (!obs2_active) {
                obs2_respawn--;
                if (obs2_respawn <= 0) obs2_active = 1;
            }

            // Obstacle 1 Movement (Vertical Bumper) - Stage >= 5
            if (campaign_mode && campaign_level >= 5 && obs1_active) {
                obs_y += obs_dy;
                if (obs_y < 10) { obs_y = 10; obs_dy = -obs_dy; }
                if (obs_y > H - 50) { obs_y = H - 50; obs_dy = -obs_dy; }
            }

            // Obstacle 2 Movement (Horizontal Bumper) - Stage >= 8
            if (campaign_mode && campaign_level >= 8 && obs2_active) {
                obs2_x += obs2_dx;
                if (obs2_x < 60) { obs2_x = 60; obs2_dx = -obs2_dx; }
                if (obs2_x > W - 100) { obs2_x = W - 100; obs2_dx = -obs2_dx; }
            }

            // Stage 20 Boss Shield Movement & Recharge
            if (campaign_mode && campaign_level == 20) {
                if (boss_shield_hp > 0) {
                    boss_shield_y += boss_shield_dy;
                    if (boss_shield_y < 10) { boss_shield_y = 10; boss_shield_dy = -boss_shield_dy; }
                    if (boss_shield_y > H - boss_shield_h - 10) { boss_shield_y = H - boss_shield_h - 10; boss_shield_dy = -boss_shield_dy; }
                } else {
                    boss_shield_timer--;
                    if (boss_shield_timer <= 0) {
                        boss_shield_hp = 3; // Recharged!
                        AddShockwave(W - 55, H / 2, RGB(0, 255, 200));
                    }
                }
            }

            // Warp Portals Cooldown Update
            if (portal_cooldown > 0) portal_cooldown--;

            // Main Ball Updates Loop
            float spd_scale = (skill_slow_timer > 0) ? 0.5f : 1.0f;

            for (int b = 0; b < MAX_BALLS; b++) {
                if (!balls[b].active) continue;

                // Gravity Well Attraction - Stage >= 9
                if (campaign_mode && campaign_level >= 9) {
                    float dx_gw = gw_x - balls[b].x;
                    float dy_gw = gw_y - balls[b].y;
                    float dist_sq = dx_gw * dx_gw + dy_gw * dy_gw;
                    if (dist_sq < 90.0f * 90.0f && dist_sq > 25.0f) {
                        float dist = sqrtf(dist_sq);
                        balls[b].dx += (dx_gw / dist) * 0.28f * spd_scale;
                        balls[b].dy += (dy_gw / dist) * 0.28f * spd_scale;
                    }
                }

                // Move ball
                balls[b].x += balls[b].dx * spd_scale;
                balls[b].y += balls[b].dy * spd_scale;

                // Warp Portals Collision - Stage >= 13
                if (campaign_mode && campaign_level >= 13 && portal_cooldown == 0) {
                    float d1 = sqrtf((balls[b].x - portal1_x)*(balls[b].x - portal1_x) + (balls[b].y - portal1_y)*(balls[b].y - portal1_y));
                    if (d1 < 18.0f) {
                        balls[b].x = portal2_x + (balls[b].dx > 0 ? 18.0f : -18.0f);
                        balls[b].y = portal2_y;
                        portal_cooldown = 30;
                        AddShockwave(portal1_x, portal1_y, RGB(0, 229, 255));
                        AddShockwave(portal2_x, portal2_y, RGB(255, 0, 204));
                        AddParticles(portal2_x, portal2_y, RGB(255, 0, 204), 15, 6.0f);
                        MessageBeep(MB_OK);
                    } else {
                        float d2 = sqrtf((balls[b].x - portal2_x)*(balls[b].x - portal2_x) + (balls[b].y - portal2_y)*(balls[b].y - portal2_y));
                        if (d2 < 18.0f) {
                            balls[b].x = portal1_x + (balls[b].dx > 0 ? 18.0f : -18.0f);
                            balls[b].y = portal1_y;
                            portal_cooldown = 30;
                            AddShockwave(portal2_x, portal2_y, RGB(255, 0, 204));
                            AddShockwave(portal1_x, portal1_y, RGB(0, 229, 255));
                            AddParticles(portal1_x, portal1_y, RGB(0, 229, 255), 15, 6.0f);
                            MessageBeep(MB_OK);
                        }
                    }
                }

                // Ball Trail Update
                COLORREF bTrailColor = balls[b].is_fireball ? RGB(255, 100, 0) :
                    ((balls[b].last_hitter == 1) ? RGB(0, 229, 255) :
                    ((balls[b].last_hitter == 2) ? RGB(255, 0, 204) : RGB(255, 255, 255)));
                for (int i = MAX_TRAIL - 1; i > 0; i--) {
                    balls[b].trail[i] = balls[b].trail[i - 1];
                }
                balls[b].trail[0].x = (int)balls[b].x + BALL_SIZE / 2;
                balls[b].trail[0].y = (int)balls[b].y + BALL_SIZE / 2;
                balls[b].trail[0].color = bTrailColor;
                if (balls[b].trail_count < MAX_TRAIL) balls[b].trail_count++;

                // Powerup Capsule Collision
                if (powerup_x != -1 && balls[b].x < powerup_x + 18 && balls[b].x + BALL_SIZE > powerup_x &&
                    balls[b].y < powerup_y + 18 && balls[b].y + BALL_SIZE > powerup_y) {
                    COLORREF pColor = (powerup_type == 0) ? RGB(255, 215, 0) :
                        ((powerup_type == 1) ? RGB(255, 51, 102) :
                        ((powerup_type == 2) ? RGB(0, 229, 255) : RGB(200, 50, 255)));
                    AddParticles((float)(powerup_x + 9), (float)(powerup_y + 9), pColor, 20, 7.0f);
                    AddShockwave(powerup_x + 9, powerup_y + 9, pColor);

                    if (powerup_type == 0) {
                        if (balls[b].last_hitter == 1) p1_buff_timer = 200;
                        else if (balls[b].last_hitter == 2) p2_buff_timer = 200;
                    } else if (powerup_type == 1) {
                        if (balls[b].last_hitter == 1) p2_debuff_timer = 200;
                        else if (balls[b].last_hitter == 2) p1_debuff_timer = 200;
                    } else if (powerup_type == 2) {
                        if (balls[b].last_hitter == 1) p2_freeze_timer = 100;
                        else if (balls[b].last_hitter == 2) p1_freeze_timer = 100;
                    } else if (powerup_type == 3) {
                        SpawnExtraBall();
                        SpawnExtraBall();
                    }
                    powerup_x = -1; powerup_y = -1;
                    MessageBeep(MB_ICONASTERISK);
                }

                // Top & Bottom Wall Collisions
                if (balls[b].y < 0) {
                    balls[b].y = 0; balls[b].dy = -balls[b].dy;
                    AddParticles(balls[b].x + BALL_SIZE / 2, 0, RGB(255, 255, 255), 8, 4.0f);
                    AddShockwave(balls[b].x + BALL_SIZE / 2, 0, RGB(0, 229, 255));
                    MessageBeep(0xFFFFFFFF);
                }
                if (balls[b].y > H - BALL_SIZE) {
                    balls[b].y = H - BALL_SIZE; balls[b].dy = -balls[b].dy;
                    AddParticles(balls[b].x + BALL_SIZE / 2, (float)H, RGB(255, 255, 255), 8, 4.0f);
                    AddShockwave(balls[b].x + BALL_SIZE / 2, H, RGB(255, 0, 204));
                    MessageBeep(0xFFFFFFFF);
                }

                // Obstacle 1 Collision (Vertical Bumper)
                if (campaign_mode && campaign_level >= 5 && obs1_active &&
                    balls[b].x + BALL_SIZE > W / 2 - 10 && balls[b].x < W / 2 + 10 &&
                    balls[b].y + BALL_SIZE > obs_y && balls[b].y < obs_y + 40) {
                    if (balls[b].is_fireball) {
                        obs1_active = 0; obs1_respawn = 150; // 5 sec
                        AddParticles(W / 2, obs_y + 20, RGB(255, 100, 0), 25, 8.0f);
                        AddShockwave(W / 2, obs_y + 20, RGB(255, 100, 0));
                        MessageBeep(MB_ICONASTERISK);
                    } else {
                        balls[b].dx = -balls[b].dx;
                        AddParticles(W / 2, balls[b].y + BALL_SIZE / 2, RGB(0, 255, 255), 10, 4.0f);
                        AddShockwave(W / 2, balls[b].y + BALL_SIZE / 2, RGB(0, 255, 255));
                        MessageBeep(0xFFFFFFFF);
                    }
                }

                // Obstacle 2 Collision (Horizontal Bumper)
                if (campaign_mode && campaign_level >= 8 && obs2_active &&
                    balls[b].x + BALL_SIZE > obs2_x && balls[b].x < obs2_x + 40 &&
                    balls[b].y + BALL_SIZE > H / 2 - 10 && balls[b].y < H / 2 + 10) {
                    if (balls[b].is_fireball) {
                        obs2_active = 0; obs2_respawn = 150;
                        AddParticles(obs2_x + 20, H / 2, RGB(255, 100, 0), 25, 8.0f);
                        AddShockwave(obs2_x + 20, H / 2, RGB(255, 100, 0));
                        MessageBeep(MB_ICONASTERISK);
                    } else {
                        balls[b].dy = -balls[b].dy;
                        AddParticles(obs2_x + 20, H / 2, RGB(255, 0, 255), 10, 4.0f);
                        AddShockwave(obs2_x + 20, H / 2, RGB(255, 0, 255));
                        MessageBeep(0xFFFFFFFF);
                    }
                }

                // Stage 20 Boss Shield Barrier Collision
                if (campaign_mode && campaign_level == 20 && boss_shield_hp > 0 &&
                    balls[b].x + BALL_SIZE >= W - 55 && balls[b].x <= W - 45 &&
                    balls[b].y + BALL_SIZE >= boss_shield_y && balls[b].y <= boss_shield_y + boss_shield_h &&
                    balls[b].dx > 0) {
                    if (balls[b].is_fireball) {
                        boss_shield_hp = 0; boss_shield_timer = 360;
                        AddParticles(W - 50, balls[b].y + BALL_SIZE / 2, RGB(255, 50, 0), 30, 10.0f);
                        AddShockwave(W - 50, balls[b].y + BALL_SIZE / 2, RGB(255, 50, 0));
                        MessageBeep(MB_ICONASTERISK);
                    } else {
                        boss_shield_hp--;
                        balls[b].dx = -balls[b].dx;
                        AddParticles(W - 50, balls[b].y + BALL_SIZE / 2, RGB(255, 215, 0), 15, 6.0f);
                        AddShockwave(W - 50, balls[b].y + BALL_SIZE / 2, RGB(255, 215, 0));
                        MessageBeep(0xFFFFFFFF);
                        if (boss_shield_hp <= 0) boss_shield_timer = 360;
                    }
                }

                // P1 Paddle Collision
                if (balls[b].x < 20 + PAD_W && balls[b].y + BALL_SIZE > p1_y && balls[b].y < p1_y + p1_pad_h) {
                    balls[b].x = 20 + PAD_W;
                    balls[b].dx = -balls[b].dx;
                    balls[b].last_hitter = 1;
                    if (skill_fireball_ready) {
                        balls[b].is_fireball = 1;
                        skill_fireball_ready = 0;
                        AddShockwave(balls[b].x, balls[b].y + BALL_SIZE / 2, RGB(255, 100, 0));
                    }
                    rally++;
                    if (rally > high_rally) { high_rally = rally; SaveStats(); }
                    float hit_pos = (float)((balls[b].y + BALL_SIZE / 2.0f) - (p1_y + p1_pad_h / 2.0f)) / (p1_pad_h / 2.0f);
                    balls[b].dy += (int)(hit_pos * 5.0f);
                    if (balls[b].dy > 10) balls[b].dy = 10;
                    if (balls[b].dy < -10) balls[b].dy = -10;
                    if (balls[b].dx < 15 && balls[b].dx > -15) { balls[b].dx = balls[b].dx > 0 ? balls[b].dx + 1 : balls[b].dx - 1; }

                    COLORREF pColor = balls[b].is_fireball ? RGB(255, 100, 0) : RGB(0, 229, 255);
                    AddParticles((float)balls[b].x, (float)(balls[b].y + BALL_SIZE / 2), pColor, 18, 9.0f);
                    AddShockwave(balls[b].x, balls[b].y + BALL_SIZE / 2, pColor);
                    MessageBeep(0xFFFFFFFF);
                }

                // P2 Paddle Collision
                if (balls[b].x + BALL_SIZE > W - 20 - PAD_W && balls[b].y + BALL_SIZE > p2_y && balls[b].y < p2_y + p2_pad_h) {
                    balls[b].x = W - 20 - PAD_W - BALL_SIZE;
                    balls[b].dx = -balls[b].dx;
                    balls[b].last_hitter = 2;
                    balls[b].is_fireball = 0; // Fireball extinguished
                    rally++;
                    if (rally > high_rally) { high_rally = rally; SaveStats(); }
                    float hit_pos = (float)((balls[b].y + BALL_SIZE / 2.0f) - (p2_y + p2_pad_h / 2.0f)) / (p2_pad_h / 2.0f);
                    balls[b].dy += (int)(hit_pos * 5.0f);

                    // Stage 20 Boss Adaptive Spin
                    if (campaign_mode && campaign_level == 20) {
                        balls[b].dy += (int)(p2_vy * 0.8f);
                    }

                    if (balls[b].dy > 12) balls[b].dy = 12;
                    if (balls[b].dy < -12) balls[b].dy = -12;
                    if (balls[b].dx < 15 && balls[b].dx > -15) { balls[b].dx = balls[b].dx > 0 ? balls[b].dx + 1 : balls[b].dx - 1; }

                    AddParticles((float)(balls[b].x + BALL_SIZE), (float)(balls[b].y + BALL_SIZE / 2), RGB(255, 0, 204), 18, 9.0f);
                    AddShockwave(balls[b].x + BALL_SIZE, balls[b].y + BALL_SIZE / 2, RGB(255, 0, 204));
                    MessageBeep(0xFFFFFFFF);
                }

                // Scoring / Out-of-bounds
                if (balls[b].x < -15) {
                    balls[b].active = 0; p2_score++; rally = 0;
                    AddParticles(0, (float)(balls[b].y + BALL_SIZE / 2), RGB(255, 0, 204), 35, 12.0f);
                    AddShockwave(0, (int)(balls[b].y + BALL_SIZE / 2), RGB(255, 0, 204));
                    MessageBeep(MB_ICONEXCLAMATION);
                    if (ActiveBallCount() == 0) ResetBalls();
                }
                if (balls[b].x > W + 15) {
                    balls[b].active = 0; p1_score++; rally = 0;
                    AddParticles((float)W, (float)(balls[b].y + BALL_SIZE / 2), RGB(0, 229, 255), 35, 12.0f);
                    AddShockwave(W, (int)(balls[b].y + BALL_SIZE / 2), RGB(0, 229, 255));
                    MessageBeep(MB_ICONEXCLAMATION);
                    if (ActiveBallCount() == 0) ResetBalls();
                }
            }

            // Win / Stage Transition Checks
            int target_score = (campaign_mode && campaign_level == 20) ? 7 : (campaign_mode ? 5 : 11);
            if (p1_score >= target_score) {
                if (campaign_mode) {
                    campaign_level++; p1_score = 0; p2_score = 0; rally = 0;
                    boss_shield_hp = 3; boss_shield_timer = 0;
                    obs1_active = 1; obs2_active = 1;
                    if (campaign_level > 20) {
                        win_screen = 1; campaign_mode = 0; game_over = 1;
                        lifetime_wins++; SaveStats();
                    } else if (campaign_level >= 17) {
                        ResetBalls();
                        SpawnExtraBall();
                    } else {
                        ResetBalls();
                    }
                } else {
                    game_over = 1;
                    lifetime_wins++; SaveStats();
                }
            }
            if (p2_score >= target_score) {
                game_over = 1;
                campaign_mode = 0;
            }

            UpdateParticles();
            UpdateShockwaves();

            InvalidateRect(hwnd, NULL, FALSE);
            break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, W, H);
            HBITMAP hOld = (HBITMAP)SelectObject(memDC, hbm);

            // Background
            HBRUSH bg = CreateSolidBrush(RGB(8, 12, 20));
            RECT fullRc = {0, 0, W, H};
            FillRect(memDC, &fullRc, bg);
            DeleteObject(bg);

            // CRT Scanlines
            HPEN scanPen = CreatePen(PS_SOLID, 1, RGB(4, 6, 10));
            HPEN oldPen = (HPEN)SelectObject(memDC, scanPen);
            for (int y = 0; y < H; y += 4) {
                MoveToEx(memDC, 0, y, NULL);
                LineTo(memDC, W, y);
            }
            SelectObject(memDC, oldPen);
            DeleteObject(scanPen);

            // Border Accent
            COLORREF borderCol = (skill_slow_timer > 0) ? RGB(0, 229, 255) : RGB(0, 80, 120);
            HPEN borderPen = CreatePen(PS_SOLID, 1, borderCol);
            oldPen = (HPEN)SelectObject(memDC, borderPen);
            MoveToEx(memDC, 2, 2, NULL); LineTo(memDC, W - 2, 2);
            LineTo(memDC, W - 2, H - 2); LineTo(memDC, 2, H - 2); LineTo(memDC, 2, 2);
            SelectObject(memDC, oldPen);
            DeleteObject(borderPen);

            // Center Line & Arena Circle
            HPEN dashPen = CreatePen(PS_DOT, 1, RGB(60, 80, 100));
            oldPen = (HPEN)SelectObject(memDC, dashPen);
            MoveToEx(memDC, W / 2, 0, NULL);
            LineTo(memDC, W / 2, H);
            SelectObject(memDC, oldPen);
            DeleteObject(dashPen);

            HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            HPEN circlePen = CreatePen(PS_SOLID, 1, RGB(30, 45, 60));
            oldPen = (HPEN)SelectObject(memDC, circlePen);
            HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, nullBrush);
            Ellipse(memDC, W / 2 - 45, H / 2 - 45, W / 2 + 45, H / 2 + 45);
            SelectObject(memDC, oldPen);
            SelectObject(memDC, oldBrush);
            DeleteObject(circlePen);

            // Dynamic Arena Feature: Gravity Well (Stage >= 9)
            if (campaign_mode && campaign_level >= 9) {
                HPEN gwPen = CreatePen(PS_SOLID, 1, RGB(180, 50, 255));
                oldPen = (HPEN)SelectObject(memDC, gwPen);
                oldBrush = (HBRUSH)SelectObject(memDC, nullBrush);
                int r_gw = 35 + (rand() % 6);
                Ellipse(memDC, gw_x - r_gw, gw_y - r_gw, gw_x + r_gw, gw_y + r_gw);
                Ellipse(memDC, gw_x - 15, gw_y - 15, gw_x + 15, gw_y + 15);
                SelectObject(memDC, oldPen);
                SelectObject(memDC, oldBrush);
                DeleteObject(gwPen);
            }

            // Dynamic Arena Feature: Warp Portals (Stage >= 13)
            if (campaign_mode && campaign_level >= 13) {
                HPEN p1Pen = CreatePen(PS_SOLID, 2, RGB(0, 229, 255));
                HPEN p2Pen = CreatePen(PS_SOLID, 2, RGB(255, 0, 204));
                oldBrush = (HBRUSH)SelectObject(memDC, nullBrush);

                oldPen = (HPEN)SelectObject(memDC, p1Pen);
                Ellipse(memDC, portal1_x - 14, portal1_y - 14, portal1_x + 14, portal1_y + 14);

                SelectObject(memDC, p2Pen);
                Ellipse(memDC, portal2_x - 14, portal2_y - 14, portal2_x + 14, portal2_y + 14);

                SelectObject(memDC, oldPen);
                SelectObject(memDC, oldBrush);
                DeleteObject(p1Pen);
                DeleteObject(p2Pen);
            }

            // Obstacles 1 & 2 Rendering
            if (campaign_mode && campaign_level >= 5 && obs1_active) {
                HBRUSH obsB = CreateSolidBrush(RGB(25, 35, 50));
                HPEN obsP = CreatePen(PS_SOLID, 1, RGB(0, 229, 255));
                oldPen = (HPEN)SelectObject(memDC, obsP);
                oldBrush = (HBRUSH)SelectObject(memDC, obsB);
                Rectangle(memDC, W / 2 - 10, obs_y, W / 2 + 10, obs_y + 40);
                SelectObject(memDC, oldPen);
                SelectObject(memDC, oldBrush);
                DeleteObject(obsB);
                DeleteObject(obsP);
            }

            if (campaign_mode && campaign_level >= 8 && obs2_active) {
                HBRUSH obsB = CreateSolidBrush(RGB(35, 25, 50));
                HPEN obsP = CreatePen(PS_SOLID, 1, RGB(255, 0, 255));
                oldPen = (HPEN)SelectObject(memDC, obsP);
                oldBrush = (HBRUSH)SelectObject(memDC, obsB);
                Rectangle(memDC, obs2_x, H / 2 - 10, obs2_x + 40, H / 2 + 10);
                SelectObject(memDC, oldPen);
                SelectObject(memDC, oldBrush);
                DeleteObject(obsB);
                DeleteObject(obsP);
            }

            // Stage 20 Boss Shield Barrier Rendering
            if (campaign_mode && campaign_level == 20 && boss_shield_hp > 0) {
                COLORREF shCol = (boss_shield_hp == 3) ? RGB(0, 229, 255) : ((boss_shield_hp == 2) ? RGB(255, 215, 0) : RGB(255, 51, 102));
                HBRUSH shB = CreateSolidBrush(shCol);
                HPEN shP = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                oldPen = (HPEN)SelectObject(memDC, shP);
                oldBrush = (HBRUSH)SelectObject(memDC, shB);
                RoundRect(memDC, W - 55, boss_shield_y, W - 45, boss_shield_y + boss_shield_h, 6, 6);
                SelectObject(memDC, oldPen);
                SelectObject(memDC, oldBrush);
                DeleteObject(shB);
                DeleteObject(shP);
            }

            // Render Ball Trails & Balls
            for (int b = 0; b < MAX_BALLS; b++) {
                if (!balls[b].active) continue;

                // Motion Trail
                for (int i = balls[b].trail_count - 1; i >= 0; i--) {
                    int factor = (MAX_TRAIL - i);
                    int r = (BALL_SIZE / 2) * factor / MAX_TRAIL;
                    if (r < 1) r = 1;

                    BYTE red = (BYTE)((GetRValue(balls[b].trail[i].color) * factor) / MAX_TRAIL);
                    BYTE green = (BYTE)((GetGValue(balls[b].trail[i].color) * factor) / MAX_TRAIL);
                    BYTE blue = (BYTE)((GetBValue(balls[b].trail[i].color) * factor) / MAX_TRAIL);
                    HBRUSH tBrush = CreateSolidBrush(RGB(red, green, blue));
                    HPEN tPen = CreatePen(PS_SOLID, 1, RGB(red, green, blue));

                    HPEN pOld = (HPEN)SelectObject(memDC, tPen);
                    HBRUSH bOld = (HBRUSH)SelectObject(memDC, tBrush);

                    Ellipse(memDC, balls[b].trail[i].x - r, balls[b].trail[i].y - r, balls[b].trail[i].x + r, balls[b].trail[i].y + r);

                    SelectObject(memDC, pOld);
                    SelectObject(memDC, bOld);
                    DeleteObject(tBrush);
                    DeleteObject(tPen);
                }

                // Ball Graphic
                COLORREF ballGlowColor = balls[b].is_fireball ? RGB(255, 100, 0) :
                    ((balls[b].last_hitter == 1) ? RGB(0, 229, 255) :
                    ((balls[b].last_hitter == 2) ? RGB(255, 0, 204) : RGB(255, 255, 255)));

                HBRUSH ballB = CreateSolidBrush(balls[b].is_fireball ? RGB(255, 200, 50) : RGB(255, 255, 255));
                HPEN ballP = CreatePen(PS_SOLID, 2, ballGlowColor);
                oldPen = (HPEN)SelectObject(memDC, ballP);
                oldBrush = (HBRUSH)SelectObject(memDC, ballB);
                Ellipse(memDC, (int)balls[b].x - 1, (int)balls[b].y - 1, (int)balls[b].x + BALL_SIZE + 1, (int)balls[b].y + BALL_SIZE + 1);
                SelectObject(memDC, oldPen);
                SelectObject(memDC, oldBrush);
                DeleteObject(ballB);
                DeleteObject(ballP);
            }

            // Draw P1 Paddle
            {
                int x = 20, y = p1_y, w = PAD_W, h = p1_pad_h;
                COLORREF mainColor = (skill_mega_timer > 0) ? RGB(255, 215, 0) : RGB(0, 229, 255);
                COLORREF darkColor = (skill_mega_timer > 0) ? RGB(100, 80, 0) : RGB(0, 50, 80);

                if (p1_buff_timer > 0 || skill_mega_timer > 0) {
                    HBRUSH goldB = CreateSolidBrush(RGB(255, 215, 0));
                    RECT rG = { x - 2, y - 2, x + w + 2, y + h + 2 };
                    FrameRect(memDC, &rG, goldB);
                    DeleteObject(goldB);
                }

                HBRUSH p1B = CreateSolidBrush(darkColor);
                HPEN p1P = CreatePen(PS_SOLID, 1, mainColor);
                HPEN pOld = (HPEN)SelectObject(memDC, p1P);
                HBRUSH bOld = (HBRUSH)SelectObject(memDC, p1B);
                RoundRect(memDC, x, y, x + w, y + h, 6, 6);

                // Core Highlight
                HPEN coreP = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                SelectObject(memDC, coreP);
                MoveToEx(memDC, x + w / 2, y + 4, NULL);
                LineTo(memDC, x + w / 2, y + h - 4);

                SelectObject(memDC, pOld);
                SelectObject(memDC, bOld);
                DeleteObject(p1B);
                DeleteObject(p1P);
                DeleteObject(coreP);
            }

            // Draw P2 Paddle
            {
                int x = W - 20 - PAD_W, y = p2_y, w = PAD_W, h = p2_pad_h;
                COLORREF mainColor = (campaign_mode && campaign_level == 20) ? RGB(255, 50, 0) : RGB(255, 0, 204);
                COLORREF darkColor = (campaign_mode && campaign_level == 20) ? RGB(100, 10, 0) : RGB(80, 0, 50);

                HBRUSH p2B = CreateSolidBrush(darkColor);
                HPEN p2P = CreatePen(PS_SOLID, 1, mainColor);
                HPEN pOld = (HPEN)SelectObject(memDC, p2P);
                HBRUSH bOld = (HBRUSH)SelectObject(memDC, p2B);
                RoundRect(memDC, x, y, x + w, y + h, 6, 6);

                HPEN coreP = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                SelectObject(memDC, coreP);
                MoveToEx(memDC, x + w / 2, y + 4, NULL);
                LineTo(memDC, x + w / 2, y + h - 4);

                SelectObject(memDC, pOld);
                SelectObject(memDC, bOld);
                DeleteObject(p2B);
                DeleteObject(p2P);
                DeleteObject(coreP);
            }

            // Powerup Capsule Graphic
            if (powerup_x != -1) {
                COLORREF puColor = (powerup_type == 0) ? RGB(255, 215, 0) :
                    ((powerup_type == 1) ? RGB(255, 51, 102) :
                    ((powerup_type == 2) ? RGB(0, 229, 255) : RGB(200, 50, 255)));
                HBRUSH puB = CreateSolidBrush(RGB(20, 25, 35));
                HPEN puP = CreatePen(PS_SOLID, 2, puColor);
                oldPen = (HPEN)SelectObject(memDC, puP);
                oldBrush = (HBRUSH)SelectObject(memDC, puB);
                RoundRect(memDC, powerup_x, powerup_y, powerup_x + 18, powerup_y + 18, 10, 10);

                // Icon symbol
                HPEN symP = CreatePen(PS_SOLID, 1, puColor);
                SelectObject(memDC, symP);
                if (powerup_type == 0) { // Plus
                    MoveToEx(memDC, powerup_x + 9, powerup_y + 4, NULL); LineTo(memDC, powerup_x + 9, powerup_y + 14);
                    MoveToEx(memDC, powerup_x + 4, powerup_y + 9, NULL); LineTo(memDC, powerup_x + 14, powerup_y + 9);
                } else if (powerup_type == 1) { // Minus
                    MoveToEx(memDC, powerup_x + 4, powerup_y + 9, NULL); LineTo(memDC, powerup_x + 14, powerup_y + 9);
                } else if (powerup_type == 2) { // Ice
                    MoveToEx(memDC, powerup_x + 9, powerup_y + 4, NULL); LineTo(memDC, powerup_x + 9, powerup_y + 14);
                    MoveToEx(memDC, powerup_x + 4, powerup_y + 9, NULL); LineTo(memDC, powerup_x + 14, powerup_y + 9);
                } else { // Multi-Ball M
                    MoveToEx(memDC, powerup_x + 4, powerup_y + 14, NULL); LineTo(memDC, powerup_x + 4, powerup_y + 4);
                    LineTo(memDC, powerup_x + 9, powerup_y + 10); LineTo(memDC, powerup_x + 14, powerup_y + 4);
                    LineTo(memDC, powerup_x + 14, powerup_y + 14);
                }
                SelectObject(memDC, oldPen);
                SelectObject(memDC, oldBrush);
                DeleteObject(puB);
                DeleteObject(puP);
                DeleteObject(symP);
            }

            // Draw Shockwaves & Particles
            for (int i = 0; i < MAX_SHOCKWAVES; i++) {
                if (shockwaves[i].radius > 0) {
                    HPEN swP = CreatePen(PS_SOLID, 1, shockwaves[i].color);
                    oldPen = (HPEN)SelectObject(memDC, swP);
                    oldBrush = (HBRUSH)SelectObject(memDC, nullBrush);
                    int r = shockwaves[i].radius;
                    Ellipse(memDC, shockwaves[i].x - r, shockwaves[i].y - r, shockwaves[i].x + r, shockwaves[i].y + r);
                    SelectObject(memDC, oldPen);
                    SelectObject(memDC, oldBrush);
                    DeleteObject(swP);
                }
            }

            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (particles[i].life > 0) {
                    HBRUSH pB = CreateSolidBrush(particles[i].color);
                    RECT pR = {
                        (int)particles[i].x - particles[i].size / 2,
                        (int)particles[i].y - particles[i].size / 2,
                        (int)particles[i].x + particles[i].size / 2 + 1,
                        (int)particles[i].y + particles[i].size / 2 + 1
                    };
                    FillRect(memDC, &pR, pB);
                    DeleteObject(pB);
                }
            }

            // HUD Header
            SetBkMode(memDC, TRANSPARENT);
            char scoreStr[32];
            wsprintfA(scoreStr, "%d - %d", p1_score, p2_score);
            SetTextColor(memDC, RGB(255, 255, 255));
            TextOutA(memDC, W / 2 - 20, 8, scoreStr, lstrlenA(scoreStr));

            char diffStr[64];
            if (campaign_mode) {
                SetTextColor(memDC, RGB(255, 215, 0));
                if (campaign_level == 20) wsprintfA(diffStr, "STAGE 20: HYPER-CPU BOSS");
                else wsprintfA(diffStr, "CAMPAIGN: STAGE %d / 20", campaign_level);
            } else {
                SetTextColor(memDC, RGB(180, 180, 180));
                char* dName = difficulty == 1 ? "EASY" : (difficulty == 2 ? "NORMAL" : "HARD");
                wsprintfA(diffStr, "DIFF: %s (1-3)", dName);
            }
            TextOutA(memDC, W / 2 - 65, 26, diffStr, lstrlenA(diffStr));

            // Skills Cooldown Footer HUD Bar
            char skillsStr[128];
            char sSlow[16], sMega[16], sFire[16];
            if (skill_slow_timer > 0) wsprintfA(sSlow, "ACTIVE");
            else if (skill_slow_cooldown == 0) wsprintfA(sSlow, "READY");
            else wsprintfA(sSlow, "%ds", skill_slow_cooldown / 30);

            if (skill_mega_timer > 0) wsprintfA(sMega, "ACTIVE");
            else if (skill_mega_cooldown == 0) wsprintfA(sMega, "READY");
            else wsprintfA(sMega, "%ds", skill_mega_cooldown / 30);

            if (skill_fireball_ready) wsprintfA(sFire, "READY!");
            else if (skill_fireball_cooldown == 0) wsprintfA(sFire, "READY");
            else wsprintfA(sFire, "%ds", skill_fireball_cooldown / 30);

            wsprintfA(skillsStr, "[F]Slow:%s  [P]Mega:%s  [B]Fire:%s", sSlow, sMega, sFire);
            SetTextColor(memDC, RGB(0, 229, 255));
            TextOutA(memDC, 10, H - 20, skillsStr, lstrlenA(skillsStr));

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
                    SetTextColor(memDC, (p1_score > p2_score) ? RGB(0, 229, 255) : RGB(255, 0, 204));
                    char* winStr = (p1_score > p2_score) ? "P1 WINS!" : "P2 WINS!";
                    TextOutA(memDC, W / 2 - 30, H / 2 - 20, winStr, lstrlenA(winStr));
                }
                SetTextColor(memDC, RGB(255, 255, 255));
                char* restartStr = "Press 'R' to Restart";
                TextOutA(memDC, W / 2 - 60, H / 2 + 10, restartStr, lstrlenA(restartStr));
            }

            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
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
    LoadStats();
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KPongApp";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KPongApp", "KPong", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, W + 16, H + 39, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
