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

#define MAX_PARTICLES 60
#define MAX_TRAIL 8
#define MAX_SHOCKWAVES 8

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

Particle particles[MAX_PARTICLES];
BallTrailNode ball_trail[MAX_TRAIL];
int ball_trail_count = 0;
Shockwave shockwaves[MAX_SHOCKWAVES];

int p1_pad_h = 50;
int p2_pad_h = 50;
int campaign_mode = 0;
int campaign_level = 1;
int win_screen = 0;
int last_hitter = 0;
int powerup_x = -1;
int powerup_y = -1;
int powerup_active = 0;
int p1_buff_timer = 0;
int p2_buff_timer = 0;
int p1_debuff_timer = 0;
int p2_debuff_timer = 0;
int powerup_type = 0;
int high_rally = 0;
int lifetime_wins = 0;
int obs_y = H / 2 - 20;
int obs_dy = 3;
int obs2_x = W / 2 - 20;
int obs2_dx = 4;
int p1_freeze_timer = 0;
int p2_freeze_timer = 0;

int p1_y = H / 2 - 25;
int p2_y = H / 2 - 25;
int p1_vy = 0;
int p2_vy = 0;

int ball_x = W / 2 - BALL_SIZE / 2;
int ball_y = H / 2 - BALL_SIZE / 2;
int ball_dx = 5;
int ball_dy = 3;
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
    if(f) { 
        fread(&high_rally, sizeof(int), 1, f); 
        fread(&lifetime_wins, sizeof(int), 1, f);
        fclose(f); 
    }
}

void SaveStats() {
    FILE* f = fopen("kpong_stats.dat", "wb");
    if(f) { 
        fwrite(&high_rally, sizeof(int), 1, f); 
        fwrite(&lifetime_wins, sizeof(int), 1, f);
        fclose(f); 
    }
}

void ResetBall() {
    ball_x = W / 2 - BALL_SIZE / 2;
    ball_y = H / 2 - BALL_SIZE / 2;
    ball_dx = (ball_dx > 0) ? -5 : 5;
    ball_dy = (ball_dy > 0) ? 3 : -3;
    ball_trail_count = 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            SetTimer(hwnd, TIMER_ID, 30, NULL);
            break;
        case WM_TIMER:
            if (game_over) {
                if (GetAsyncKeyState('R') & 0x8000) {
                    p1_score = 0;
                    p2_score = 0;
                    rally = 0;
                    game_over = 0;
                    ResetBall();
                }
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }

            if (GetAsyncKeyState('1') & 0x8000) difficulty = 1;
            if (GetAsyncKeyState('2') & 0x8000) difficulty = 2;
            if (GetAsyncKeyState('3') & 0x8000) difficulty = 3;
            if (GetAsyncKeyState('C') & 0x8000 && !campaign_mode) { campaign_mode = 1; campaign_level = 1; p1_score=0; p2_score=0; rally=0; ResetBall(); }

            // Dynamic paddle size
            if(p1_buff_timer > 0) p1_buff_timer--;
            if(p2_buff_timer > 0) p2_buff_timer--;
            if(p1_debuff_timer > 0) p1_debuff_timer--;
            if(p2_debuff_timer > 0) p2_debuff_timer--;
            if(p1_freeze_timer > 0) p1_freeze_timer--;
            if(p2_freeze_timer > 0) p2_freeze_timer--;
            
            p1_pad_h = 50 - (rally * 2); if(p1_pad_h < 20) p1_pad_h = 20;
            p2_pad_h = 50 - (rally * 2); if(p2_pad_h < 20) p2_pad_h = 20;
            
            if(campaign_mode && campaign_level >= 3) { p2_pad_h -= 10; }
            if(campaign_mode && (campaign_level == 10 || campaign_level == 15)) { p2_pad_h += 50; }
            
            if(p1_pad_h < 15) p1_pad_h = 15;
            if(p2_pad_h < 15) p2_pad_h = 15;

            if(p1_buff_timer > 0) p1_pad_h += 30;
            if(p2_buff_timer > 0) p2_pad_h += 30;
            if(p1_debuff_timer > 0) p1_pad_h = 15;
            if(p2_debuff_timer > 0) p2_debuff_timer = 15;

            // Input & velocity
            int p1_prev_y = p1_y;
            if (p1_freeze_timer == 0) {
                if ((GetAsyncKeyState(VK_UP) & 0x8000) || (GetAsyncKeyState('W') & 0x8000)) p1_y -= 6;
                if ((GetAsyncKeyState(VK_DOWN) & 0x8000) || (GetAsyncKeyState('S') & 0x8000)) p1_y += 6;
            }
            if (p1_y < 0) p1_y = 0;
            if (p1_y > H - p1_pad_h) p1_y = H - p1_pad_h;
            p1_vy = p1_y - p1_prev_y;

            int ai_spd = ai_speeds[difficulty];
            if(campaign_mode) {
                if(campaign_level == 1) ai_spd = 2;
                else if(campaign_level == 2) ai_spd = 4;
                else if(campaign_level == 3) ai_spd = 5;
                else if(campaign_level == 4) ai_spd = 6;
                else if(campaign_level == 5) ai_spd = 7;
                else if(campaign_level == 6) ai_spd = 8;
                else if(campaign_level == 7) ai_spd = 9;
                else if(campaign_level == 8) ai_spd = 10;
                else if(campaign_level == 9) ai_spd = 12;
                else if(campaign_level <= 11) ai_spd = 15;
                else if(campaign_level <= 13) ai_spd = 17;
                else ai_spd = 19;
            }
            int p2_prev_y = p2_y;
            if (p2_freeze_timer == 0) {
                if (ball_y > p2_y + p2_pad_h / 2 + 10) p2_y += ai_spd;
                if (ball_y < p2_y + p2_pad_h / 2 - 10) p2_y -= ai_spd;
            }
            if (p2_y < 0) p2_y = 0;
            if (p2_y > H - p2_pad_h) p2_y = H - p2_pad_h;
            p2_vy = p2_y - p2_prev_y;

            // Powerups
            if(powerup_active > 0) powerup_active--;
            if(powerup_x == -1 && rand() % 200 == 0) {
                powerup_x = W/4 + rand()%(W/2); powerup_y = H/4 + rand()%(H/2); powerup_active = 250;
                powerup_type = rand() % 3;
            }
            if(powerup_active == 0) { powerup_x = -1; powerup_y = -1; }
            
            // Obstacles
            if(campaign_mode && campaign_level >= 6) {
                obs_y += obs_dy;
                if(obs_y < 0) { obs_y = 0; obs_dy = -obs_dy; }
                if(obs_y > H - 40) { obs_y = H - 40; obs_dy = -obs_dy; }
                
                if(ball_x + BALL_SIZE > W/2 - 10 && ball_x < W/2 + 10 && ball_y + BALL_SIZE > obs_y && ball_y < obs_y + 40) {
                    ball_dx = -ball_dx;
                    AddParticles((float)(W/2), (float)(ball_y + BALL_SIZE/2), RGB(0, 255, 255), 10, 4.0f);
                    AddShockwave(W/2, ball_y + BALL_SIZE/2, RGB(0, 255, 255));
                    MessageBeep(0xFFFFFFFF);
                }
            }
            if(campaign_mode && campaign_level >= 12) {
                obs2_x += obs2_dx;
                if(obs2_x < 50) { obs2_x = 50; obs2_dx = -obs2_dx; }
                if(obs2_x > W - 90) { obs2_x = W - 90; obs2_dx = -obs2_dx; }
                
                if(ball_x + BALL_SIZE > obs2_x && ball_x < obs2_x + 40 && ball_y + BALL_SIZE > H/2 - 10 && ball_y < H/2 + 10) {
                    ball_dy = -ball_dy;
                    AddParticles((float)(obs2_x + 20), (float)(H/2), RGB(255, 0, 255), 10, 4.0f);
                    AddShockwave(obs2_x + 20, H/2, RGB(255, 0, 255));
                    MessageBeep(0xFFFFFFFF);
                }
            }

            // Ball logic
            if(campaign_mode && campaign_level >= 5 && campaign_level < 10) { 
                ball_x += (ball_dx > 0 ? ball_dx + 2 : ball_dx - 2); 
                ball_y += (ball_dy > 0 ? ball_dy + 1 : ball_dy - 1); 
            } else if (campaign_mode && campaign_level >= 10 && campaign_level < 15) {
                ball_x += (ball_dx > 0 ? ball_dx + 3 : ball_dx - 3); 
                ball_y += (ball_dy > 0 ? ball_dy + 2 : ball_dy - 2); 
            } else if (campaign_mode && campaign_level == 15) {
                ball_x += (ball_dx > 0 ? ball_dx + 4 : ball_dx - 4); 
                ball_y += (ball_dy > 0 ? ball_dy + 3 : ball_dy - 3); 
            } else { 
                ball_x += ball_dx; ball_y += ball_dy; 
            }

            // Ball Trail history update
            COLORREF bTrailColor = (last_hitter == 1) ? RGB(0, 229, 255) : ((last_hitter == 2) ? RGB(255, 0, 204) : RGB(255, 255, 255));
            for (int i = MAX_TRAIL - 1; i > 0; i--) {
                ball_trail[i] = ball_trail[i - 1];
            }
            ball_trail[0].x = ball_x + BALL_SIZE / 2;
            ball_trail[0].y = ball_y + BALL_SIZE / 2;
            ball_trail[0].color = bTrailColor;
            if (ball_trail_count < MAX_TRAIL) ball_trail_count++;

            // Powerup pickup
            if(powerup_x != -1 && ball_x < powerup_x+18 && ball_x+BALL_SIZE > powerup_x && ball_y < powerup_y+18 && ball_y+BALL_SIZE > powerup_y) {
                COLORREF pColor = (powerup_type == 0) ? RGB(255, 215, 0) : ((powerup_type == 1) ? RGB(255, 51, 102) : RGB(0, 229, 255));
                AddParticles((float)(powerup_x + 9), (float)(powerup_y + 9), pColor, 20, 7.0f);
                AddShockwave(powerup_x + 9, powerup_y + 9, pColor);

                if (powerup_type == 0) {
                    if(last_hitter == 1) p1_buff_timer = 200;
                    else if(last_hitter == 2) p2_buff_timer = 200;
                } else if (powerup_type == 1) {
                    if(last_hitter == 1) p2_debuff_timer = 200;
                    else if(last_hitter == 2) p1_debuff_timer = 200;
                } else if (powerup_type == 2) {
                    if(last_hitter == 1) p2_freeze_timer = 100;
                    else if(last_hitter == 2) p1_freeze_timer = 100;
                }
                powerup_x = -1; powerup_y = -1;
                MessageBeep(MB_ICONASTERISK);
            }

            // Top/bottom collisions
            if (ball_y < 0) { 
                ball_y = 0; ball_dy = -ball_dy; 
                AddParticles((float)(ball_x + BALL_SIZE/2), 0, RGB(255, 255, 255), 8, 4.0f);
                AddShockwave(ball_x + BALL_SIZE/2, 0, RGB(0, 229, 255));
                MessageBeep(0xFFFFFFFF); 
            }
            if (ball_y > H - BALL_SIZE) { 
                ball_y = H - BALL_SIZE; ball_dy = -ball_dy; 
                AddParticles((float)(ball_x + BALL_SIZE/2), (float)H, RGB(255, 255, 255), 8, 4.0f);
                AddShockwave(ball_x + BALL_SIZE/2, H, RGB(255, 0, 204));
                MessageBeep(0xFFFFFFFF); 
            }

            // Paddle collisions
            if (ball_x < 20 + PAD_W && ball_y + BALL_SIZE > p1_y && ball_y < p1_y + p1_pad_h) {
                ball_x = 20 + PAD_W;
                ball_dx = -ball_dx;
                last_hitter = 1;
                rally++;
                if(rally > high_rally) { high_rally = rally; SaveStats(); }
                float hit_pos = (float)((ball_y + BALL_SIZE/2.0f) - (p1_y + p1_pad_h/2.0f)) / (p1_pad_h/2.0f);
                ball_dy += (int)(hit_pos * 5.0f);
                if (ball_dy > 10) ball_dy = 10;
                if (ball_dy < -10) ball_dy = -10;
                if (ball_dx < 15 && ball_dx > -15) { ball_dx = ball_dx > 0 ? ball_dx + 1 : ball_dx - 1; }
                
                AddParticles((float)ball_x, (float)(ball_y + BALL_SIZE/2), RGB(0, 229, 255), 18, 9.0f);
                AddShockwave(ball_x, ball_y + BALL_SIZE/2, RGB(0, 229, 255));
                MessageBeep(0xFFFFFFFF);
            }
            if (ball_x + BALL_SIZE > W - 20 - PAD_W && ball_y + BALL_SIZE > p2_y && ball_y < p2_y + p2_pad_h) {
                ball_x = W - 20 - PAD_W - BALL_SIZE;
                ball_dx = -ball_dx;
                last_hitter = 2;
                rally++;
                if(rally > high_rally) { high_rally = rally; SaveStats(); }
                float hit_pos = (float)((ball_y + BALL_SIZE/2.0f) - (p2_y + p2_pad_h/2.0f)) / (p2_pad_h/2.0f);
                ball_dy += (int)(hit_pos * 5.0f);
                if (ball_dy > 10) ball_dy = 10;
                if (ball_dy < -10) ball_dy = -10;
                if (ball_dx < 15 && ball_dx > -15) { ball_dx = ball_dx > 0 ? ball_dx + 1 : ball_dx - 1; }

                AddParticles((float)(ball_x + BALL_SIZE), (float)(ball_y + BALL_SIZE/2), RGB(255, 0, 204), 18, 9.0f);
                AddShockwave(ball_x + BALL_SIZE, ball_y + BALL_SIZE/2, RGB(255, 0, 204));
                MessageBeep(0xFFFFFFFF);
            }

            // Scoring
            if (ball_x < -10) { 
                p2_score++; rally = 0; 
                AddParticles(0, (float)(ball_y + BALL_SIZE/2), RGB(255, 0, 204), 35, 12.0f);
                AddShockwave(0, ball_y + BALL_SIZE/2, RGB(255, 0, 204));
                MessageBeep(MB_ICONEXCLAMATION); ResetBall(); 
            }
            if (ball_x > W + 10) { 
                p1_score++; rally = 0; 
                AddParticles((float)W, (float)(ball_y + BALL_SIZE/2), RGB(0, 229, 255), 35, 12.0f);
                AddShockwave(W, ball_y + BALL_SIZE/2, RGB(0, 229, 255));
                MessageBeep(MB_ICONEXCLAMATION); ResetBall(); 
            }

            int target_score = campaign_mode ? 5 : 11;
            if (p1_score >= target_score) {
                if(campaign_mode) {
                    campaign_level++; p1_score = 0; p2_score = 0; rally = 0;
                    if(campaign_level > 15) { 
                        win_screen = 1; campaign_mode = 0; game_over = 1; 
                        lifetime_wins++; SaveStats();
                    }
                    ResetBall();
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

            // Border Neon Accent Lines
            HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(0, 80, 120));
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

            // Ball Motion Trail
            for (int i = ball_trail_count - 1; i >= 0; i--) {
                int factor = (MAX_TRAIL - i);
                int r = (BALL_SIZE / 2) * factor / MAX_TRAIL;
                if (r < 1) r = 1;

                BYTE red = (BYTE)((GetRValue(ball_trail[i].color) * factor) / MAX_TRAIL);
                BYTE green = (BYTE)((GetGValue(ball_trail[i].color) * factor) / MAX_TRAIL);
                BYTE blue = (BYTE)((GetBValue(ball_trail[i].color) * factor) / MAX_TRAIL);
                HBRUSH tBrush = CreateSolidBrush(RGB(red, green, blue));
                HPEN tPen = CreatePen(PS_SOLID, 1, RGB(red, green, blue));
                
                HPEN pOld = (HPEN)SelectObject(memDC, tPen);
                HBRUSH bOld = (HBRUSH)SelectObject(memDC, tBrush);
                
                Ellipse(memDC, ball_trail[i].x - r, ball_trail[i].y - r, ball_trail[i].x + r, ball_trail[i].y + r);
                
                SelectObject(memDC, pOld);
                SelectObject(memDC, bOld);
                DeleteObject(tBrush);
                DeleteObject(tPen);
            }

            // Draw Paddles
            // P1 Paddle (Cyan Neon Metallic)
            {
                int x = 20, y = p1_y, w = PAD_W, h = p1_pad_h;
                COLORREF mainColor = RGB(0, 229, 255);
                COLORREF darkColor = RGB(0, 50, 80);

                if (p1_buff_timer > 0) {
                    HBRUSH goldB = CreateSolidBrush(RGB(255, 215, 0));
                    RECT rG = { x - 2, y - 2, x + w + 2, y + h + 2 };
                    FrameRect(memDC, &rG, goldB);
                    DeleteObject(goldB);
                } else if (p1_debuff_timer > 0) {
                    HBRUSH redB = CreateSolidBrush(RGB(255, 51, 102));
                    RECT rR = { x - 2, y - 2, x + w + 2, y + h + 2 };
                    FrameRect(memDC, &rR, redB);
                    DeleteObject(redB);
                }

                if (p1_vy != 0 && p1_freeze_timer == 0) {
                    HPEN flamePen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
                    HPEN fOld = (HPEN)SelectObject(memDC, flamePen);
                    int fy = p1_vy > 0 ? y - 4 : y + h + 4;
                    MoveToEx(memDC, x + 2, y + (p1_vy > 0 ? 0 : h), NULL);
                    LineTo(memDC, x + w / 2, fy);
                    LineTo(memDC, x + w - 2, y + (p1_vy > 0 ? 0 : h));
                    SelectObject(memDC, fOld);
                    DeleteObject(flamePen);
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

                if (p1_freeze_timer > 0) {
                    HPEN iceP = CreatePen(PS_SOLID, 1, RGB(180, 240, 255));
                    HPEN iOld = (HPEN)SelectObject(memDC, iceP);
                    MoveToEx(memDC, x - 2, y + h / 3, NULL); LineTo(memDC, x + w + 2, y + h / 3);
                    MoveToEx(memDC, x - 2, y + 2 * h / 3, NULL); LineTo(memDC, x + w + 2, y + 2 * h / 3);
                    SelectObject(memDC, iOld);
                    DeleteObject(iceP);
                }
            }

            // P2 Paddle (Magenta Neon Metallic)
            {
                int x = W - 20 - PAD_W, y = p2_y, w = PAD_W, h = p2_pad_h;
                COLORREF mainColor = RGB(255, 0, 204);
                COLORREF darkColor = RGB(80, 0, 50);

                if (p2_buff_timer > 0) {
                    HBRUSH goldB = CreateSolidBrush(RGB(255, 215, 0));
                    RECT rG = { x - 2, y - 2, x + w + 2, y + h + 2 };
                    FrameRect(memDC, &rG, goldB);
                    DeleteObject(goldB);
                } else if (p2_debuff_timer > 0) {
                    HBRUSH redB = CreateSolidBrush(RGB(255, 51, 102));
                    RECT rR = { x - 2, y - 2, x + w + 2, y + h + 2 };
                    FrameRect(memDC, &rR, redB);
                    DeleteObject(redB);
                }

                if (p2_vy != 0 && p2_freeze_timer == 0) {
                    HPEN flamePen = CreatePen(PS_SOLID, 1, RGB(255, 100, 255));
                    HPEN fOld = (HPEN)SelectObject(memDC, flamePen);
                    int fy = p2_vy > 0 ? y - 4 : y + h + 4;
                    MoveToEx(memDC, x + 2, y + (p2_vy > 0 ? 0 : h), NULL);
                    LineTo(memDC, x + w / 2, fy);
                    LineTo(memDC, x + w - 2, y + (p2_vy > 0 ? 0 : h));
                    SelectObject(memDC, fOld);
                    DeleteObject(flamePen);
                }

                HBRUSH p2B = CreateSolidBrush(darkColor);
                HPEN p2P = CreatePen(PS_SOLID, 1, mainColor);
                HPEN pOld = (HPEN)SelectObject(memDC, p2P);
                HBRUSH bOld = (HBRUSH)SelectObject(memDC, p2B);
                RoundRect(memDC, x, y, x + w, y + h, 6, 6);

                // Core Highlight
                HPEN coreP = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                SelectObject(memDC, coreP);
                MoveToEx(memDC, x + w / 2, y + 4, NULL);
                LineTo(memDC, x + w / 2, y + h - 4);

                SelectObject(memDC, pOld);
                SelectObject(memDC, bOld);
                DeleteObject(p2B);
                DeleteObject(p2P);
                DeleteObject(coreP);

                if (p2_freeze_timer > 0) {
                    HPEN iceP = CreatePen(PS_SOLID, 1, RGB(180, 240, 255));
                    HPEN iOld = (HPEN)SelectObject(memDC, iceP);
                    MoveToEx(memDC, x - 2, y + h / 3, NULL); LineTo(memDC, x + w + 2, y + h / 3);
                    MoveToEx(memDC, x - 2, y + 2 * h / 3, NULL); LineTo(memDC, x + w + 2, y + 2 * h / 3);
                    SelectObject(memDC, iOld);
                    DeleteObject(iceP);
                }
            }
            
            // Draw Ball (Energy sphere with core & glow rim)
            COLORREF ballGlowColor = (last_hitter == 1) ? RGB(0, 229, 255) : ((last_hitter == 2) ? RGB(255, 0, 204) : RGB(255, 255, 255));
            HBRUSH ballB = CreateSolidBrush(RGB(255, 255, 255));
            HPEN ballP = CreatePen(PS_SOLID, 2, ballGlowColor);
            oldPen = (HPEN)SelectObject(memDC, ballP);
            oldBrush = (HBRUSH)SelectObject(memDC, ballB);
            Ellipse(memDC, ball_x - 1, ball_y - 1, ball_x + BALL_SIZE + 1, ball_y + BALL_SIZE + 1);
            SelectObject(memDC, oldPen);
            SelectObject(memDC, oldBrush);
            DeleteObject(ballB);
            DeleteObject(ballP);

            // Powerup Capsule Graphic
            if(powerup_x != -1) {
                COLORREF puColor = (powerup_type == 0) ? RGB(255, 215, 0) : ((powerup_type == 1) ? RGB(255, 51, 102) : RGB(0, 229, 255));
                HBRUSH puB = CreateSolidBrush(RGB(20, 25, 35));
                HPEN puP = CreatePen(PS_SOLID, 2, puColor);
                oldPen = (HPEN)SelectObject(memDC, puP);
                oldBrush = (HBRUSH)SelectObject(memDC, puB);
                RoundRect(memDC, powerup_x, powerup_y, powerup_x + 18, powerup_y + 18, 10, 10);
                
                // Icon symbol
                HPEN symP = CreatePen(PS_SOLID, 1, puColor);
                SelectObject(memDC, symP);
                if (powerup_type == 0) { // Golden Plus / Expand
                    MoveToEx(memDC, powerup_x + 9, powerup_y + 4, NULL); LineTo(memDC, powerup_x + 9, powerup_y + 14);
                    MoveToEx(memDC, powerup_x + 4, powerup_y + 9, NULL); LineTo(memDC, powerup_x + 14, powerup_y + 9);
                } else if (powerup_type == 1) { // Red Exclamation / Shrink
                    MoveToEx(memDC, powerup_x + 9, powerup_y + 4, NULL); LineTo(memDC, powerup_x + 9, powerup_y + 11);
                    SetPixel(memDC, powerup_x + 9, powerup_y + 13, puColor);
                } else { // Ice Asterisk / Freeze
                    MoveToEx(memDC, powerup_x + 9, powerup_y + 4, NULL); LineTo(memDC, powerup_x + 9, powerup_y + 14);
                    MoveToEx(memDC, powerup_x + 4, powerup_y + 9, NULL); LineTo(memDC, powerup_x + 14, powerup_y + 9);
                    MoveToEx(memDC, powerup_x + 5, powerup_y + 5, NULL); LineTo(memDC, powerup_x + 13, powerup_y + 13);
                    MoveToEx(memDC, powerup_x + 13, powerup_y + 5, NULL); LineTo(memDC, powerup_x + 5, powerup_y + 13);
                }
                SelectObject(memDC, oldPen);
                SelectObject(memDC, oldBrush);
                DeleteObject(puB);
                DeleteObject(puP);
                DeleteObject(symP);
            }

            // Obstacles (Industrial Hazard Barriers)
            if(campaign_mode && campaign_level >= 6) {
                HBRUSH obsB = CreateSolidBrush(RGB(25, 35, 50));
                HPEN obsP = CreatePen(PS_SOLID, 1, RGB(0, 229, 255));
                oldPen = (HPEN)SelectObject(memDC, obsP);
                oldBrush = (HBRUSH)SelectObject(memDC, obsB);
                
                Rectangle(memDC, W/2 - 10, obs_y, W/2 + 10, obs_y + 40);

                // Hazard lines inside
                HPEN lineP = CreatePen(PS_SOLID, 1, RGB(0, 120, 160));
                SelectObject(memDC, lineP);
                for(int i = 0; i < 40; i += 8) {
                    MoveToEx(memDC, W/2 - 10, obs_y + i, NULL);
                    LineTo(memDC, W/2 + 10, obs_y + i + 6);
                }
                DeleteObject(lineP);

                if(campaign_level >= 12) {
                    SelectObject(memDC, obsP);
                    SelectObject(memDC, obsB);
                    Rectangle(memDC, obs2_x, H/2 - 10, obs2_x + 40, H/2 + 10);
                }

                SelectObject(memDC, oldPen);
                SelectObject(memDC, oldBrush);
                DeleteObject(obsB);
                DeleteObject(obsP);
            }

            // Draw Shockwaves
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

            // Draw Particles
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

            // Draw score & HUD text
            char scoreStr[32];
            wsprintfA(scoreStr, "%d - %d", p1_score, p2_score);
            SetTextColor(memDC, RGB(255, 255, 255));
            SetBkMode(memDC, TRANSPARENT);
            TextOutA(memDC, W / 2 - 20, 10, scoreStr, lstrlenA(scoreStr));
            
            char rallyStr[64];
            wsprintfA(rallyStr, "RALLY: %d  HIGH: %d  WINS: %d", rally, high_rally, lifetime_wins);
            SetTextColor(memDC, RGB(180, 180, 180));
            TextOutA(memDC, W / 2 - 80, 30, rallyStr, lstrlenA(rallyStr));
            
            char diffStr[32];
            if(campaign_mode) {
                SetTextColor(memDC, RGB(255, 215, 0));
                wsprintfA(diffStr, "CAMPAIGN: LVL %d", campaign_level);
            } else {
                SetTextColor(memDC, RGB(180, 180, 180));
                char* dName = difficulty == 1 ? "EASY" : (difficulty == 2 ? "NORMAL" : "HARD");
                wsprintfA(diffStr, "DIFF: %s (1-3)", dName);
            }
            TextOutA(memDC, W / 2 - 50, 50, diffStr, lstrlenA(diffStr));
            if(!campaign_mode) {
                char* cInfo = "Press 'C' for Campaign";
                SetTextColor(memDC, RGB(120, 140, 160));
                TextOutA(memDC, W / 2 - 65, 70, cInfo, lstrlenA(cInfo));
            }
            
            if (game_over) {
                HBRUSH overlayB = CreateSolidBrush(RGB(5, 10, 20));
                RECT overR = { 0, 0, W, H };
                FillRect(memDC, &overR, overlayB);
                DeleteObject(overlayB);

                if(win_screen) {
                    SetTextColor(memDC, RGB(255, 215, 0));
                    char* winStr = "CAMPAIGN COMPLETE!";
                    TextOutA(memDC, W / 2 - 65, H / 2 - 20, winStr, lstrlenA(winStr));
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
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
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

