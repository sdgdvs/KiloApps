#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#define W 400
#define H 300
#define PAD_W 10
#define PAD_H 50
#define BALL_SIZE 10
#define TIMER_ID 1

int p1_y = H / 2 - PAD_H / 2;
int p2_y = H / 2 - PAD_H / 2;
int ball_dx = 5;
int ball_dy = 3;
int p1_score = 0;
int p2_score = 0;
int game_over = 0;

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
                    game_over = 0;
                    ResetBall();
                }
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }

            // Input
            if (GetAsyncKeyState(VK_UP) & 0x8000) p1_y -= 6;
            if (GetAsyncKeyState(VK_DOWN) & 0x8000) p1_y += 6;
            if (p1_y < 0) p1_y = 0;
            if (p1_y > H - PAD_H) p1_y = H - PAD_H;

            // AI (simple)
            if (ball_y > p2_y + PAD_H / 2 + 10) p2_y += 4;
            if (ball_y < p2_y + PAD_H / 2 - 10) p2_y -= 4;
            if (p2_y < 0) p2_y = 0;
            if (p2_y > H - PAD_H) p2_y = H - PAD_H;

            // Ball logic
            ball_x += ball_dx;
            ball_y += ball_dy;

            // Top/bottom collisions
            if (ball_y < 0) { ball_y = 0; ball_dy = -ball_dy; MessageBeep(0xFFFFFFFF); }
            if (ball_y > H - BALL_SIZE) { ball_y = H - BALL_SIZE; ball_dy = -ball_dy; MessageBeep(0xFFFFFFFF); }

            // Paddle collisions
            if (ball_x < 20 + PAD_W && ball_y + BALL_SIZE > p1_y && ball_y < p1_y + PAD_H) {
                ball_x = 20 + PAD_W;
                ball_dx = -ball_dx;
                if (ball_dx < 15 && ball_dx > -15) { ball_dx = ball_dx > 0 ? ball_dx + 1 : ball_dx - 1; }
                if (ball_dy < 15 && ball_dy > -15) { ball_dy = ball_dy > 0 ? ball_dy + 1 : ball_dy - 1; }
                MessageBeep(0xFFFFFFFF);
            }
            if (ball_x + BALL_SIZE > W - 20 - PAD_W && ball_y + BALL_SIZE > p2_y && ball_y < p2_y + PAD_H) {
                ball_x = W - 20 - PAD_W - BALL_SIZE;
                ball_dx = -ball_dx;
                if (ball_dx < 15 && ball_dx > -15) { ball_dx = ball_dx > 0 ? ball_dx + 1 : ball_dx - 1; }
                if (ball_dy < 15 && ball_dy > -15) { ball_dy = ball_dy > 0 ? ball_dy + 1 : ball_dy - 1; }
                MessageBeep(0xFFFFFFFF);
            }

            // Scoring
            if (ball_x < 0) { p2_score++; MessageBeep(MB_ICONEXCLAMATION); ResetBall(); }
            if (ball_x > W) { p1_score++; MessageBeep(MB_ICONEXCLAMATION); ResetBall(); }

            if (p1_score >= 11 || p2_score >= 11) {
                game_over = 1;
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
            RECT r1 = { 20, p1_y, 20 + PAD_W, p1_y + PAD_H };
            RECT r2 = { W - 20 - PAD_W, p2_y, W - 20, p2_y + PAD_H };
            FillRect(memDC, &r1, p1_brush);
            FillRect(memDC, &r2, p2_brush);
            
            // Draw ball
            RECT rBall = { ball_x, ball_y, ball_x + BALL_SIZE, ball_y + BALL_SIZE };
            FillRect(memDC, &rBall, ball_brush);
            
            DeleteObject(p1_brush);
            DeleteObject(p2_brush);
            DeleteObject(ball_brush);
            
            // Draw score
            char scoreStr[32];
            wsprintfA(scoreStr, "%d - %d", p1_score, p2_score);
            SetTextColor(memDC, RGB(255, 255, 255));
            SetBkMode(memDC, TRANSPARENT);
            TextOutA(memDC, W / 2 - 20, 10, scoreStr, lstrlenA(scoreStr));
            
            if (game_over) {
                char* winStr = p1_score >= 11 ? "P1 WINS!" : "P2 WINS!";
                TextOutA(memDC, W / 2 - 30, H / 2 - 20, winStr, lstrlenA(winStr));
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
