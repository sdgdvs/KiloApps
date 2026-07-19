#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#define W 400
#define H 400
#define TIMER_ID 1

int pad_x = W / 2 - 30;
int pad_w = 60;
int pad_h = 10;
int ball_x = W / 2;
int ball_y = H / 2;
int ball_size = 8;
int ball_dx = 0;
int ball_dy = 0;
int score = 0;
int high_score = 0;
int state = 0; // 0=start, 1=play, 2=gameover, 3=win
int diff = 0; // 0=Easy, 1=Hard
int speed = 3;
int lives = 3;
int power_x = 0;
int power_y = 0;
int power_active = 0;
int power_type = 0;
int paddle_timer = 0;
int piercing = 0;
int level = 1;
int max_level = 1;
int total_bricks = 0;

#define ROWS 5
#define COLS 10
int bricks[ROWS][COLS];
int bricks_left = 0;

void LoadHighScore() {
    FILE *f = fopen("kbreakout_hi.dat", "r");
    if (f) {
        fscanf(f, "%d %d %d", &high_score, &max_level, &total_bricks);
        fclose(f);
    }
}

void SaveHighScore() {
    if (score > high_score) high_score = score;
    FILE *f = fopen("kbreakout_hi.dat", "w");
    if (f) {
        fprintf(f, "%d %d %d", high_score, max_level, total_bricks);
        fclose(f);
    }
}

void InitLevel() {
    bricks_left = 0;
    power_active = 0;
    piercing = 0;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (level == 1) {
                if (r == 1 && c % 3 == 1) bricks[r][c] = 9;
                else if (r == 2) bricks[r][c] = 2;
                else bricks[r][c] = 1;
            } else if (level == 2) {
                if ((r + c) % 2 == 0) bricks[r][c] = 2;
                else bricks[r][c] = 1;
            } else if (level == 3) {
                if (r == 2 && (c < 3 || c > 6)) bricks[r][c] = 9;
                else bricks[r][c] = 2;
            } else if (level == 4) {
                if (c == 2 || c == 7) bricks[r][c] = 9;
                else bricks[r][c] = (r % 2 == 0) ? 2 : 1;
            } else {
                bricks[r][c] = 2;
                if (r == 2 && c > 2 && c < 7) bricks[r][c] = 9;
            }
            
            if (bricks[r][c] != 9) bricks_left++;
        }
    }
    pad_w = (diff == 1) ? 40 : 60;
    paddle_timer = 0;
    pad_x = W / 2 - pad_w / 2;
    ball_x = W / 2;
    ball_y = H - 50;
    ball_dx = speed * (GetTickCount() % 2 == 0 ? 1 : -1);
    ball_dy = -speed;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            LoadHighScore();
            SetTimer(hwnd, TIMER_ID, 16, NULL);
            break;
        case WM_TIMER:
            if (state == 1) {
                if (GetAsyncKeyState(VK_LEFT) & 0x8000) pad_x -= 6;
                if (GetAsyncKeyState(VK_RIGHT) & 0x8000) pad_x += 6;
                if (pad_x < 0) pad_x = 0;
                if (pad_x > W - pad_w) pad_x = W - pad_w;

                ball_x += ball_dx;
                ball_y += ball_dy;

                // Wall collisions
                if (ball_x < 0) { ball_x = 0; ball_dx = -ball_dx; }
                if (ball_x > W - ball_size) { ball_x = W - ball_size; ball_dx = -ball_dx; }
                if (ball_y < 0) { ball_y = 0; ball_dy = -ball_dy; }

                // Paddle collision
                if (ball_y + ball_size > H - 30 && ball_y < H - 30 + pad_h) {
                    if (ball_x + ball_size > pad_x && ball_x < pad_x + pad_w) {
                        ball_y = H - 30 - ball_size;
                        ball_dy = -ball_dy;
                        int hit_pos = (ball_x + ball_size / 2) - (pad_x + pad_w / 2);
                        ball_dx = (hit_pos / 5);
                        if (ball_dx == 0) ball_dx = (ball_dx > 0) ? 1 : -1;
                        MessageBeep(0xFFFFFFFF);
                    }
                }

                // Bottom collision
                if (ball_y > H) {
                    lives--;
                    power_active = 0;
                    if (lives <= 0) {
                        SaveHighScore();
                        state = 2; // Game Over
                        MessageBeep(MB_ICONEXCLAMATION);
                    } else {
                        pad_w = (diff == 1) ? 40 : 60;
                        paddle_timer = 0;
                        pad_x = W / 2 - pad_w / 2;
                        ball_x = W / 2;
                        ball_y = H - 50;
                        ball_dx = speed * (GetTickCount() % 2 == 0 ? 1 : -1);
                        ball_dy = -speed;
                        MessageBeep(MB_ICONASTERISK);
                    }
                }

                if (power_active) {
                    power_y += 3;
                    if (power_y + 10 > H - 30 && power_y < H - 30 + pad_h) {
                        if (power_x + 10 > pad_x && power_x < pad_x + pad_w) {
                            power_active = 0;
                            if (power_type == 1) {
                                paddle_timer = 300;
                                pad_w = (diff == 1) ? 80 : 100;
                            } else if (power_type == 2) {
                                lives++;
                            } else if (power_type == 3) {
                                piercing = 300;
                            }
                            MessageBeep(MB_OK);
                        }
                    }
                    if (power_y > H) power_active = 0;
                }
                
                if (paddle_timer > 0) {
                    paddle_timer--;
                    if (paddle_timer == 0) pad_w = (diff == 1) ? 40 : 60;
                }
                if (piercing > 0) {
                    piercing--;
                }

                // Bricks collision
                int br_w = W / COLS;
                int br_h = 20;
                for (int r = 0; r < ROWS; r++) {
                    for (int c = 0; c < COLS; c++) {
                        if (bricks[r][c]) {
                            int bx = c * br_w;
                            int by = r * br_h + 40;
                            if (ball_x + ball_size > bx && ball_x < bx + br_w &&
                                ball_y + ball_size > by && ball_y < by + br_h) {
                                if (bricks[r][c] == 9) {
                                    ball_dy = -ball_dy;
                                    MessageBeep(0xFFFFFFFF);
                                } else if (bricks[r][c] == 2) {
                                    if (piercing) {
                                        bricks[r][c] = 0;
                                        bricks_left--;
                                        score += 15 + (diff * 5);
                                        total_bricks++;
                                    } else {
                                        bricks[r][c] = 1;
                                        ball_dy = -ball_dy;
                                        score += 5 + (diff * 5);
                                    }
                                    MessageBeep(0xFFFFFFFF);
                                } else {
                                    bricks[r][c] = 0;
                                    bricks_left--;
                                    total_bricks++;
                                    if (!piercing) ball_dy = -ball_dy;
                                    score += 10 + (diff * 10);
                                    MessageBeep(0xFFFFFFFF);
                                    
                                    if (!power_active && (GetTickCount() % 6 == 0)) {
                                        power_active = 1;
                                        power_type = (GetTickCount() % 3) + 1;
                                        power_x = bx + br_w / 2;
                                        power_y = by;
                                    }
                                    
                                    if (bricks_left == 0) {
                                        level++;
                                        if (level > max_level) max_level = level;
                                        if (level > 5) {
                                            SaveHighScore();
                                            state = 3; // Win
                                        } else {
                                            speed++;
                                            InitLevel();
                                        }
                                    }
                                }
                                goto out_loops;
                            }
                        }
                    }
                }
            out_loops:;
            } else if (state == 0 || state == 2 || state == 3) {
                if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
                    diff = 0; speed = 3; score = 0; lives = 3; level = 1; InitLevel(); state = 1; pad_w = 60;
                }
                if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
                    diff = 1; speed = 5; score = 0; lives = 3; level = 1; InitLevel(); state = 1; pad_w = 40;
                }
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, W, H);
            HBITMAP hOld = (HBITMAP)SelectObject(memDC, hbm);
            
            HBRUSH bg = CreateSolidBrush(RGB(20, 20, 30));
            RECT fullRc = {0, 0, W, H};
            FillRect(memDC, &fullRc, bg);
            DeleteObject(bg);
            
            SetTextColor(memDC, RGB(255, 255, 255));
            SetBkMode(memDC, TRANSPARENT);
            
            if (state == 0) {
                char* t1 = "KBreakout";
                char* t2 = "Press ENTER for Easy";
                char* t3 = "Press SPACE for Hard";
                TextOutA(memDC, W/2 - 35, H/2 - 20, t1, lstrlenA(t1));
                TextOutA(memDC, W/2 - 70, H/2 + 10, t2, lstrlenA(t2));
                TextOutA(memDC, W/2 - 70, H/2 + 30, t3, lstrlenA(t3));
            } else if (state == 2) {
                char* t1 = "GAME OVER";
                char* t2 = "Press ENTER for Easy";
                char* t3 = "Press SPACE for Hard";
                TextOutA(memDC, W/2 - 40, H/2 - 20, t1, lstrlenA(t1));
                TextOutA(memDC, W/2 - 70, H/2 + 10, t2, lstrlenA(t2));
                TextOutA(memDC, W/2 - 70, H/2 + 30, t3, lstrlenA(t3));
            } else {
                HBRUSH p_brush = CreateSolidBrush(RGB(51, 204, 255));
                HBRUSH b_brush = CreateSolidBrush(RGB(255, 255, 255));
                
                RECT rp = { pad_x, H - 30, pad_x + pad_w, H - 30 + pad_h };
                FillRect(memDC, &rp, p_brush);
                DeleteObject(p_brush);
                
                if (piercing) {
                    DeleteObject(b_brush);
                    b_brush = CreateSolidBrush(RGB(50, 150, 255));
                }
                RECT rb = { ball_x, ball_y, ball_x + ball_size, ball_y + ball_size };
                FillRect(memDC, &rb, b_brush);
                DeleteObject(b_brush);
                
                int br_w = W / COLS;
                int br_h = 20;
                for (int r = 0; r < ROWS; r++) {
                    for (int c = 0; c < COLS; c++) {
                        if (bricks[r][c]) {
                            HBRUSH br;
                            if (bricks[r][c] == 9) br = CreateSolidBrush(RGB(100, 100, 100));
                            else if (bricks[r][c] == 2) br = CreateSolidBrush(RGB(255, 100, 100));
                            else br = CreateSolidBrush(RGB(200 - r * 30, 100 + c * 15, 50 + r * 20));
                            
                            RECT rr = { c * br_w + 1, r * br_h + 40 + 1, (c + 1) * br_w - 1, (r + 1) * br_h + 40 - 1 };
                            FillRect(memDC, &rr, br);
                            DeleteObject(br);
                        }
                    }
                }
                
                if (power_active) {
                    HBRUSH pw;
                    if (power_type == 1) pw = CreateSolidBrush(RGB(255, 255, 0));
                    else if (power_type == 2) pw = CreateSolidBrush(RGB(255, 50, 50));
                    else pw = CreateSolidBrush(RGB(50, 150, 255));
                    RECT rp = { power_x, power_y, power_x + 10, power_y + 10 };
                    FillRect(memDC, &rp, pw);
                    DeleteObject(pw);
                }
            }
            
            char sStr[128];
            wsprintfA(sStr, "Lvl:%d Sc:%d Hi:%d Lvs:%d TBr:%d", level, score, high_score, lives, total_bricks);
            TextOutA(memDC, 10, 10, sStr, lstrlenA(sStr));
            
            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, hOld);
            DeleteObject(hbm);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            SaveHighScore();
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
    wc.lpszClassName = "KBreakoutApp";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KBreakoutApp", "KBreakout", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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
