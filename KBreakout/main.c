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

#define ROWS 5
#define COLS 10
int bricks[ROWS][COLS];
int bricks_left = 0;

void LoadHighScore() {
    FILE *f = fopen("kbreakout_hi.dat", "r");
    if (f) {
        fscanf(f, "%d", &high_score);
        fclose(f);
    }
}

void SaveHighScore() {
    if (score > high_score) {
        high_score = score;
        FILE *f = fopen("kbreakout_hi.dat", "w");
        if (f) {
            fprintf(f, "%d", high_score);
            fclose(f);
        }
    }
}

void InitLevel() {
    bricks_left = 0;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            bricks[r][c] = 1;
            bricks_left++;
        }
    }
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
                    SaveHighScore();
                    state = 2; // Game Over
                    MessageBeep(MB_ICONEXCLAMATION);
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
                                bricks[r][c] = 0;
                                bricks_left--;
                                ball_dy = -ball_dy;
                                score += 10 + (diff * 10);
                                MessageBeep(0xFFFFFFFF);
                                
                                if (bricks_left == 0) {
                                    speed++;
                                    InitLevel();
                                }
                                goto out_loops;
                            }
                        }
                    }
                }
            out_loops:;
            } else if (state == 0 || state == 2) {
                if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
                    diff = 0; speed = 3; score = 0; InitLevel(); state = 1; pad_w = 60;
                }
                if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
                    diff = 1; speed = 5; score = 0; InitLevel(); state = 1; pad_w = 40;
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
                
                RECT rb = { ball_x, ball_y, ball_x + ball_size, ball_y + ball_size };
                FillRect(memDC, &rb, b_brush);
                
                DeleteObject(p_brush);
                DeleteObject(b_brush);
                
                int br_w = W / COLS;
                int br_h = 20;
                for (int r = 0; r < ROWS; r++) {
                    for (int c = 0; c < COLS; c++) {
                        if (bricks[r][c]) {
                            HBRUSH br = CreateSolidBrush(RGB(200 - r * 30, 100 + c * 15, 50 + r * 20));
                            RECT rr = { c * br_w + 1, r * br_h + 40 + 1, (c + 1) * br_w - 1, (r + 1) * br_h + 40 - 1 };
                            FillRect(memDC, &rr, br);
                            DeleteObject(br);
                        }
                    }
                }
            }
            
            char sStr[64];
            wsprintfA(sStr, "Score: %d  High: %d", score, high_score);
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
