#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int _fltused = 1;
long _ftol2_sse(float f) { return (long)f; }
long _ftol2(float f) { return (long)f; }

#define W 320
#define H 480
#define MAX_BULLETS 20
#define MAX_ENEMIES 10

typedef struct { float x, y, active; } Ent;

Ent p = { W/2.0f, H - 50.0f, 1.0f };
Ent b[MAX_BULLETS] = {0};
Ent e[MAX_ENEMIES] = {0};
int score = 0;
int gameOver = 0;
int frameCount = 0;

unsigned int seed = 999;
unsigned int rnd() {
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

void SpawnEnemy() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!e[i].active) {
            e[i].active = 1.0f;
            e[i].x = (float)(rnd() % (W - 20));
            e[i].y = -20.0f;
            break;
        }
    }
}

void Shoot() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!b[i].active) {
            b[i].active = 1.0f;
            b[i].x = p.x + 8.0f;
            b[i].y = p.y;
            break;
        }
    }
}

void Update() {
    if (gameOver) return;
    
    float speed = 4.0f;
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) p.x -= speed;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) p.x += speed;
    if (GetAsyncKeyState(VK_UP) & 0x8000) p.y -= speed;
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) p.y += speed;
    
    if (p.x < 0) p.x = 0;
    if (p.x > W - 20) p.x = W - 20;
    if (p.y < 0) p.y = 0;
    if (p.y > H - 20) p.y = H - 20;
    
    if (GetAsyncKeyState(VK_SPACE) & 0x8001) { // simple debounce
        if (frameCount % 5 == 0) Shoot();
    }
    
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (b[i].active) {
            b[i].y -= 8.0f;
            if (b[i].y < 0) b[i].active = 0.0f;
        }
    }
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (e[i].active) {
            e[i].y += 2.0f;
            if (e[i].y > H) e[i].active = 0.0f;
            
            // Player collision
            if (p.x < e[i].x + 20 && p.x + 20 > e[i].x && p.y < e[i].y + 20 && p.y + 20 > e[i].y) {
                gameOver = 1;
            }
            
            // Bullet collision
            for (int j = 0; j < MAX_BULLETS; j++) {
                if (b[j].active && b[j].x < e[i].x + 20 && b[j].x + 4 > e[i].x && b[j].y < e[i].y + 20 && b[j].y + 10 > e[i].y) {
                    b[j].active = 0.0f;
                    e[i].active = 0.0f;
                    score += 10;
                    break;
                }
            }
        }
    }
    
    if (frameCount % 30 == 0) SpawnEnemy();
    frameCount++;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            seed = GetTickCount();
            SetTimer(hwnd, 1, 16, NULL); // ~60 FPS
            break;
        case WM_TIMER:
            Update();
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, W, H);
            SelectObject(memDC, hbm);
            
            HBRUSH bg = CreateSolidBrush(RGB(10, 10, 30));
            RECT rc = {0, 0, W, H};
            FillRect(memDC, &rc, bg);
            DeleteObject(bg);
            
            // Draw player
            HBRUSH pbr = CreateSolidBrush(RGB(100, 200, 255));
            RECT pr = {(int)p.x, (int)p.y, (int)p.x + 20, (int)p.y + 20};
            FillRect(memDC, &pr, pbr);
            DeleteObject(pbr);
            
            // Draw bullets
            HBRUSH bbr = CreateSolidBrush(RGB(255, 255, 100));
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (b[i].active) {
                    RECT br = {(int)b[i].x, (int)b[i].y, (int)b[i].x + 4, (int)b[i].y + 10};
                    FillRect(memDC, &br, bbr);
                }
            }
            DeleteObject(bbr);
            
            // Draw enemies
            HBRUSH ebr = CreateSolidBrush(RGB(255, 50, 50));
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (e[i].active) {
                    RECT er = {(int)e[i].x, (int)e[i].y, (int)e[i].x + 20, (int)e[i].y + 20};
                    FillRect(memDC, &er, ebr);
                }
            }
            DeleteObject(ebr);
            
            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(255, 255, 255));
            char scoreStr[32];
            wsprintfA(scoreStr, "Score: %d", score);
            TextOutA(memDC, 10, 10, scoreStr, lstrlenA(scoreStr));
            
            if (gameOver) {
                SetTextColor(memDC, RGB(255, 0, 0));
                TextOutA(memDC, W/2 - 40, H/2 - 10, "GAME OVER", 9);
            }
            
            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
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
    wc.lpszClassName = "KSpaceApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KSpaceApp", "KSpace", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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
