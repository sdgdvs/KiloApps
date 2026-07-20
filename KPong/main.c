#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define W 400
#define H 300
#define PAD_W 10
#define BALL_SIZE 10
#define TIMER_ID 1

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

int p1_y = H / 2 - 25;
int p2_y = H / 2 - 25;
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
            
            p1_pad_h = 50 - (rally * 2); if(p1_pad_h < 20) p1_pad_h = 20;
            p2_pad_h = 50 - (rally * 2); if(p2_pad_h < 20) p2_pad_h = 20;
            
            if(campaign_mode && campaign_level >= 3) { p2_pad_h -= 10; }
            if(campaign_mode && campaign_level == 10) { p2_pad_h += 50; } // Boss paddle
            
            if(p1_pad_h < 15) p1_pad_h = 15;
            if(p2_pad_h < 15) p2_pad_h = 15;

            if(p1_buff_timer > 0) p1_pad_h += 30;
            if(p2_buff_timer > 0) p2_pad_h += 30;
            if(p1_debuff_timer > 0) p1_pad_h = 15;
            if(p2_debuff_timer > 0) p2_pad_h = 15;

            // Input
            if (GetAsyncKeyState(VK_UP) & 0x8000) p1_y -= 6;
            if (GetAsyncKeyState(VK_DOWN) & 0x8000) p1_y += 6;
            if (p1_y < 0) p1_y = 0;
            if (p1_y > H - p1_pad_h) p1_y = H - p1_pad_h;

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
                else ai_spd = 15;
            }
            if (ball_y > p2_y + p2_pad_h / 2 + 10) p2_y += ai_spd;
            if (ball_y < p2_y + p2_pad_h / 2 - 10) p2_y -= ai_spd;
            if (p2_y < 0) p2_y = 0;
            if (p2_y > H - p2_pad_h) p2_y = H - p2_pad_h;

            // Powerups
            if(powerup_active > 0) powerup_active--;
            if(powerup_x == -1 && rand() % 200 == 0) {
                powerup_x = W/4 + rand()%(W/2); powerup_y = H/4 + rand()%(H/2); powerup_active = 250;
                powerup_type = rand() % 2;
            }
            if(powerup_active == 0) { powerup_x = -1; powerup_y = -1; }
            
            // Obstacles
            if(campaign_mode && campaign_level >= 6) {
                obs_y += obs_dy;
                if(obs_y < 0) { obs_y = 0; obs_dy = -obs_dy; }
                if(obs_y > H - 40) { obs_y = H - 40; obs_dy = -obs_dy; }
                
                // Obstacle collision
                if(ball_x + BALL_SIZE > W/2 - 10 && ball_x < W/2 + 10 && ball_y + BALL_SIZE > obs_y && ball_y < obs_y + 40) {
                    ball_dx = -ball_dx;
                    MessageBeep(0xFFFFFFFF);
                }
            }

            // Ball logic
            if(campaign_mode && campaign_level >= 5 && campaign_level < 10) { 
                ball_x += (ball_dx > 0 ? ball_dx + 2 : ball_dx - 2); 
                ball_y += (ball_dy > 0 ? ball_dy + 1 : ball_dy - 1); 
            } else if (campaign_mode && campaign_level == 10) {
                ball_x += (ball_dx > 0 ? ball_dx + 3 : ball_dx - 3); 
                ball_y += (ball_dy > 0 ? ball_dy + 2 : ball_dy - 2); 
            } else { 
                ball_x += ball_dx; ball_y += ball_dy; 
            }
            
            if(powerup_x != -1 && ball_x < powerup_x+15 && ball_x+BALL_SIZE > powerup_x && ball_y < powerup_y+15 && ball_y+BALL_SIZE > powerup_y) {
                if (powerup_type == 0) {
                    if(last_hitter == 1) p1_buff_timer = 200;
                    else if(last_hitter == 2) p2_buff_timer = 200;
                } else {
                    if(last_hitter == 1) p2_debuff_timer = 200;
                    else if(last_hitter == 2) p1_debuff_timer = 200;
                }
                powerup_x = -1; powerup_y = -1;
                MessageBeep(MB_ICONASTERISK);
            }


            // Top/bottom collisions
            if (ball_y < 0) { ball_y = 0; ball_dy = -ball_dy; MessageBeep(0xFFFFFFFF); }
            if (ball_y > H - BALL_SIZE) { ball_y = H - BALL_SIZE; ball_dy = -ball_dy; MessageBeep(0xFFFFFFFF); }

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
                MessageBeep(0xFFFFFFFF);
            }

            // Scoring
            if (ball_x < 0) { p2_score++; rally = 0; MessageBeep(MB_ICONEXCLAMATION); ResetBall(); }
            if (ball_x > W) { p1_score++; rally = 0; MessageBeep(MB_ICONEXCLAMATION); ResetBall(); }

            int target_score = campaign_mode ? 5 : 11;
            if (p1_score >= target_score) {
                if(campaign_mode) {
                    campaign_level++; p1_score = 0; p2_score = 0; rally = 0;
                    if(campaign_level > 10) { 
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

            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, W, H);
            HBITMAP hOld = (HBITMAP)SelectObject(memDC, hbm);
            
            HBRUSH bg = CreateSolidBrush(RGB(10, 10, 15));
            RECT fullRc = {0, 0, W, H};
            FillRect(memDC, &fullRc, bg);
            DeleteObject(bg);
            
            HBRUSH p1_brush = CreateSolidBrush(RGB(51, 204, 255));
            HBRUSH p2_brush = CreateSolidBrush(RGB(255, 51, 204));
            HBRUSH ball_brush = CreateSolidBrush(RGB(255, 255, 255));
            
            // Draw paddles
            RECT r1 = { 20, p1_y, 20 + PAD_W, p1_y + p1_pad_h };
            RECT r2 = { W - 20 - PAD_W, p2_y, W - 20, p2_y + p2_pad_h };
            FillRect(memDC, &r1, p1_brush);
            FillRect(memDC, &r2, p2_brush);
            
            // Draw ball
            RECT rBall = { ball_x, ball_y, ball_x + BALL_SIZE, ball_y + BALL_SIZE };
            FillRect(memDC, &rBall, ball_brush);
            if(powerup_x != -1) {
                HBRUSH pu_brush = CreateSolidBrush(powerup_type == 0 ? RGB(255, 255, 0) : RGB(255, 50, 50));
                RECT rPu = { powerup_x, powerup_y, powerup_x+15, powerup_y+15 };
                FillRect(memDC, &rPu, pu_brush);
                DeleteObject(pu_brush);
            }
            if(campaign_mode && campaign_level >= 6) {
                HBRUSH obs_brush = CreateSolidBrush(RGB(150, 150, 150));
                RECT rObs = { W/2 - 10, obs_y, W/2 + 10, obs_y + 40 };
                FillRect(memDC, &rObs, obs_brush);
                DeleteObject(obs_brush);
            }
            
            DeleteObject(p1_brush);
            DeleteObject(p2_brush);
            DeleteObject(ball_brush);
            
            // Draw score
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
                wsprintfA(diffStr, "CAMPAIGN: LVL %d", campaign_level);
            } else {
                char* dName = difficulty == 1 ? "EASY" : (difficulty == 2 ? "NORMAL" : "HARD");
                wsprintfA(diffStr, "DIFF: %s (1-3)", dName);
            }
            TextOutA(memDC, W / 2 - 50, 50, diffStr, lstrlenA(diffStr));
            if(!campaign_mode) {
                char* cInfo = "Press 'C' for Campaign";
                TextOutA(memDC, W / 2 - 65, 70, cInfo, lstrlenA(cInfo));
            }
            
            if (game_over) {
                if(win_screen) {
                    SetTextColor(memDC, RGB(255, 215, 0));
                    char* winStr = "CAMPAIGN COMPLETE!";
                    TextOutA(memDC, W / 2 - 65, H / 2 - 20, winStr, lstrlenA(winStr));
                } else {
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
