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
int state = 0; // 0=start, 1=play, 2=gameover
int diff = 0; // 0=Easy, 1=Hard
int speed = 3;
int lives = 3;
int power_x = 0;
int power_y = 0;
int power_active = 0;
int paddle_timer = 0;
int level = 1;
int lifetime_bricks = 0;
int power_type = 0;
int pierce_timer = 0;
int ufo_active = 0;
int ufo_x = 0;
int ufo_y = 25;
int ufo_dx = 2;
int ufo_timer = 500;

#define ROWS 5
#define COLS 10
int bricks[ROWS][COLS];
int bricks_left = 0;

void LoadHighScore() {
    FILE *f = fopen("kbreakout_hi.dat", "r");
    if (f) {
        fscanf(f, "%d %d", &high_score, &lifetime_bricks);
        fclose(f);
    }
}

void SaveHighScore() {
    if (score > high_score) high_score = score;
    FILE *f = fopen("kbreakout_hi.dat", "w");
    if (f) {
        fprintf(f, "%d %d", high_score, lifetime_bricks);
        fclose(f);
    }
}

void InitLevel() {
    bricks_left = 0;
    power_active = 0;
    pierce_timer = 0;
    ufo_active = 0;
    ufo_timer = 300;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            bricks[r][c] = 0;
            if (level % 5 == 1) {
                if (r == 1 && c % 3 == 1) bricks[r][c] = 9;
                else if (r == 2) bricks[r][c] = 2;
                else bricks[r][c] = 1;
            } else if (level % 5 == 2) {
                if ((r+c)%2 == 0) bricks[r][c] = 1;
                else if (r == 0) bricks[r][c] = 9;
            } else if (level % 5 == 3) {
                if (c >= r && c < COLS - r) bricks[r][c] = (r == 0) ? 2 : 1;
            } else if (level % 5 == 4) {
                if ((r==1 && (c==2 || c==7))) bricks[r][c] = 9;
                else if (r==3 && (c>1 && c<8)) bricks[r][c] = 2;
                else if (r==4 && (c==2 || c==7)) bricks[r][c] = 1;
            } else {
                if (c == 2 || c == 7) bricks[r][c] = 9;
                else bricks[r][c] = (r%2==0) ? 2 : 1;
            }
            if (bricks[r][c] != 9 && bricks[r][c] != 0) bricks_left++;
        }
    }
    if (bricks_left == 0) { bricks[0][0] = 1; bricks_left = 1; }
    
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
                    HBRUSH pw;
                    if (power_type == 1) pw = CreateSolidBrush(RGB(255, 255, 0));
                    else if (power_type == 2) pw = CreateSolidBrush(RGB(0, 255, 0));
                    else pw = CreateSolidBrush(RGB(255, 0, 0));
                    RECT rp = { power_x, power_y, power_x + 10, power_y + 10 };
                    FillRect(memDC, &rp, pw);
                    DeleteObject(pw);
                }
            }
            
            char sStr[64];
            wsprintfA(sStr, "L%d Sc:%d Hi:%d Lv:%d Brks:%d", level, score, high_score, lives, lifetime_bricks);
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
