#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define TIMER_ID 1
#define WIDTH 800
#define HEIGHT 600

const char g_szClassName[] = "KAsteroidsClass";

typedef struct {
    float x, y;
    float vx, vy;
    float angle;
    bool active;
    float radius;
} Ship;

typedef struct {
    float x, y;
    float vx, vy;
    float radius;
    int level;
    bool active;
    float points[10];
} Asteroid;

typedef struct {
    float x, y;
    float vx, vy;
    int life;
    bool active;
} Bullet;

Ship ship;
Asteroid asteroids[100];
int num_asteroids = 0;
Bullet bullets[50];
int num_bullets = 0;
int score = 0;
bool game_over = false;
bool keys[256] = {0};
long long last_shoot_time = 0;

long long GetTimeMs() {
    return GetTickCount64();
}

void SpawnAsteroids(int count) {
    for (int i = 0; i < count; i++) {
        if (num_asteroids >= 100) break;
        float x, y;
        do {
            x = (rand() % WIDTH);
            y = (rand() % HEIGHT);
        } while (sqrt(pow(x - ship.x, 2) + pow(y - ship.y, 2)) < 100);
        
        Asteroid* a = &asteroids[num_asteroids++];
        a->x = x;
        a->y = y;
        a->radius = 40.0f;
        a->level = 3;
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = (rand() % 200 + 100) / 100.0f;
        a->vx = cos(angle) * speed;
        a->vy = sin(angle) * speed;
        a->active = true;
        for (int j = 0; j < 10; j++) {
            a->points[j] = a->radius + (rand() % (int)(a->radius * 0.4f)) - a->radius * 0.2f;
        }
    }
}

void InitGame() {
    score = 0;
    ship.x = WIDTH / 2.0f;
    ship.y = HEIGHT / 2.0f;
    ship.vx = 0;
    ship.vy = 0;
    ship.angle = -3.14159f / 2.0f;
    ship.active = true;
    ship.radius = 10.0f;
    
    num_asteroids = 0;
    num_bullets = 0;
    game_over = false;
    
    SpawnAsteroids(4);
}

void CheckCollisions() {
    for (int i = 0; i < num_bullets; i++) {
        if (!bullets[i].active) continue;
        for (int j = 0; j < num_asteroids; j++) {
            if (!asteroids[j].active) continue;
            float dist = sqrt(pow(bullets[i].x - asteroids[j].x, 2) + pow(bullets[i].y - asteroids[j].y, 2));
            if (dist < asteroids[j].radius) {
                bullets[i].active = false;
                asteroids[j].active = false;
                score += (4 - asteroids[j].level) * 10;
                
                if (asteroids[j].level > 1 && num_asteroids + 2 <= 100) {
                    for (int k = 0; k < 2; k++) {
                        Asteroid* a = &asteroids[num_asteroids++];
                        a->x = asteroids[j].x;
                        a->y = asteroids[j].y;
                        a->radius = asteroids[j].radius / 2.0f;
                        a->level = asteroids[j].level - 1;
                        float angle = (rand() % 360) * 3.14159f / 180.0f;
                        float speed = (rand() % 200 + 100) / 100.0f;
                        a->vx = cos(angle) * speed;
                        a->vy = sin(angle) * speed;
                        a->active = true;
                        for (int p = 0; p < 10; p++) {
                            a->points[p] = a->radius + (rand() % (int)(a->radius * 0.4f)) - a->radius * 0.2f;
                        }
                    }
                }
            }
        }
    }
    
    if (ship.active) {
        for (int j = 0; j < num_asteroids; j++) {
            if (!asteroids[j].active) continue;
            float dist = sqrt(pow(ship.x - asteroids[j].x, 2) + pow(ship.y - asteroids[j].y, 2));
            if (dist < asteroids[j].radius + ship.radius) {
                ship.active = false;
                game_over = true;
            }
        }
    }
}

void CompactArrays() {
    int active_bullets = 0;
    for (int i = 0; i < num_bullets; i++) {
        if (bullets[i].active) {
            bullets[active_bullets++] = bullets[i];
        }
    }
    num_bullets = active_bullets;
    
    int active_asteroids = 0;
    for (int i = 0; i < num_asteroids; i++) {
        if (asteroids[i].active) {
            asteroids[active_asteroids++] = asteroids[i];
        }
    }
    num_asteroids = active_asteroids;
}

void Update() {
    if (game_over) {
        if (keys[VK_RETURN]) {
            InitGame();
        }
        return;
    }
    
    if (keys[VK_LEFT]) ship.angle -= 0.05f;
    if (keys[VK_RIGHT]) ship.angle += 0.05f;
    if (keys[VK_UP]) {
        ship.vx += cos(ship.angle) * 0.1f;
        ship.vy += sin(ship.angle) * 0.1f;
    }
    
    ship.vx *= 0.99f;
    ship.vy *= 0.99f;
    
    ship.x += ship.vx;
    ship.y += ship.vy;
    
    if (ship.x < 0) ship.x = WIDTH;
    if (ship.x > WIDTH) ship.x = 0;
    if (ship.y < 0) ship.y = HEIGHT;
    if (ship.y > HEIGHT) ship.y = 0;
    
    if (keys[VK_SPACE]) {
        long long now = GetTimeMs();
        if (now - last_shoot_time > 200 && num_bullets < 50) {
            Bullet* b = &bullets[num_bullets++];
            b->x = ship.x + cos(ship.angle) * 15.0f;
            b->y = ship.y + sin(ship.angle) * 15.0f;
            b->vx = cos(ship.angle) * 7.0f;
            b->vy = sin(ship.angle) * 7.0f;
            b->life = 60;
            b->active = true;
            last_shoot_time = now;
        }
    }
    
    for (int i = 0; i < num_bullets; i++) {
        if (!bullets[i].active) continue;
        bullets[i].x += bullets[i].vx;
        bullets[i].y += bullets[i].vy;
        bullets[i].life--;
        if (bullets[i].life <= 0) bullets[i].active = false;
        
        if (bullets[i].x < 0) bullets[i].x = WIDTH;
        if (bullets[i].x > WIDTH) bullets[i].x = 0;
        if (bullets[i].y < 0) bullets[i].y = HEIGHT;
        if (bullets[i].y > HEIGHT) bullets[i].y = 0;
    }
    
    int active_asteroids = 0;
    for (int i = 0; i < num_asteroids; i++) {
        if (!asteroids[i].active) continue;
        active_asteroids++;
        asteroids[i].x += asteroids[i].vx;
        asteroids[i].y += asteroids[i].vy;
        
        if (asteroids[i].x < -asteroids[i].radius) asteroids[i].x = WIDTH + asteroids[i].radius;
        if (asteroids[i].x > WIDTH + asteroids[i].radius) asteroids[i].x = -asteroids[i].radius;
        if (asteroids[i].y < -asteroids[i].radius) asteroids[i].y = HEIGHT + asteroids[i].radius;
        if (asteroids[i].y > HEIGHT + asteroids[i].radius) asteroids[i].y = -asteroids[i].radius;
    }
    
    CheckCollisions();
    CompactArrays();
    
    if (active_asteroids == 0) {
        SpawnAsteroids(4);
    }
}

void Draw(HDC hdc) {
    HBRUSH bgBrush = CreateSolidBrush(RGB(17, 17, 17));
    RECT rect = {0, 0, WIDTH, HEIGHT};
    FillRect(hdc, &rect, bgBrush);
    DeleteObject(bgBrush);
    
    HPEN shipPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    HPEN astPen = CreatePen(PS_SOLID, 2, RGB(170, 170, 170));
    HPEN bulletPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 0));
    HBRUSH bulletBrush = CreateSolidBrush(RGB(255, 255, 0));
    
    if (ship.active) {
        SelectObject(hdc, shipPen);
        POINT pts[3];
        pts[0].x = (LONG)(ship.x + cos(ship.angle) * 15);
        pts[0].y = (LONG)(ship.y + sin(ship.angle) * 15);
        pts[1].x = (LONG)(ship.x + cos(ship.angle + 2.5f) * 15);
        pts[1].y = (LONG)(ship.y + sin(ship.angle + 2.5f) * 15);
        pts[2].x = (LONG)(ship.x + cos(ship.angle - 2.5f) * 15);
        pts[2].y = (LONG)(ship.y + sin(ship.angle - 2.5f) * 15);
        MoveToEx(hdc, pts[0].x, pts[0].y, NULL);
        LineTo(hdc, pts[1].x, pts[1].y);
        LineTo(hdc, pts[2].x, pts[2].y);
        LineTo(hdc, pts[0].x, pts[0].y);
        
        if (keys[VK_UP]) {
            POINT flame[3];
            flame[0].x = (LONG)(ship.x - cos(ship.angle) * 10);
            flame[0].y = (LONG)(ship.y - sin(ship.angle) * 10);
            flame[1].x = (LONG)(ship.x - cos(ship.angle) * 20 - sin(ship.angle) * 5);
            flame[1].y = (LONG)(ship.y - sin(ship.angle) * 20 + cos(ship.angle) * 5);
            flame[2].x = (LONG)(ship.x - cos(ship.angle) * 20 + sin(ship.angle) * 5);
            flame[2].y = (LONG)(ship.y - sin(ship.angle) * 20 - cos(ship.angle) * 5);
            MoveToEx(hdc, flame[0].x, flame[0].y, NULL);
            LineTo(hdc, flame[1].x, flame[1].y);
            LineTo(hdc, flame[2].x, flame[2].y);
            LineTo(hdc, flame[0].x, flame[0].y);
        }
    }
    
    SelectObject(hdc, astPen);
    for (int i = 0; i < num_asteroids; i++) {
        if (!asteroids[i].active) continue;
        POINT pts[10];
        for (int j = 0; j < 10; j++) {
            float a = (j / 10.0f) * 3.14159f * 2.0f;
            pts[j].x = (LONG)(asteroids[i].x + cos(a) * asteroids[i].points[j]);
            pts[j].y = (LONG)(asteroids[i].y + sin(a) * asteroids[i].points[j]);
        }
        MoveToEx(hdc, pts[0].x, pts[0].y, NULL);
        for (int j = 1; j < 10; j++) LineTo(hdc, pts[j].x, pts[j].y);
        LineTo(hdc, pts[0].x, pts[0].y);
    }
    
    SelectObject(hdc, bulletPen);
    SelectObject(hdc, bulletBrush);
    for (int i = 0; i < num_bullets; i++) {
        if (!bullets[i].active) continue;
        Ellipse(hdc, (int)(bullets[i].x - 2), (int)(bullets[i].y - 2), (int)(bullets[i].x + 2), (int)(bullets[i].y + 2));
    }
    
    SetTextColor(hdc, RGB(0, 255, 0));
    SetBkMode(hdc, TRANSPARENT);
    char scoreStr[32];
    sprintf(scoreStr, "Score: %d", score);
    TextOutA(hdc, 10, 10, scoreStr, strlen(scoreStr));
    
    if (game_over) {
        SetTextColor(hdc, RGB(255, 0, 0));
        TextOutA(hdc, WIDTH / 2 - 50, HEIGHT / 2 - 20, "GAME OVER", 9);
        SetTextColor(hdc, RGB(255, 255, 255));
        TextOutA(hdc, WIDTH / 2 - 80, HEIGHT / 2 + 10, "Press Enter to Restart", 22);
    }
    
    DeleteObject(shipPen);
    DeleteObject(astPen);
    DeleteObject(bulletPen);
    DeleteObject(bulletBrush);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
            srand((unsigned int)time(NULL));
            InitGame();
            SetTimer(hwnd, TIMER_ID, 16, NULL); // ~60fps
            break;
        case WM_TIMER:
            Update();
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case WM_KEYDOWN:
            if (wParam < 256) keys[wParam] = true;
            break;
        case WM_KEYUP:
            if (wParam < 256) keys[wParam] = false;
            break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Double buffering
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT);
            SelectObject(memDC, memBitmap);
            
            Draw(memDC);
            
            BitBlt(hdc, 0, 0, WIDTH, HEIGHT, memDC, 0, 0, SRCCOPY);
            
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
        }
        break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    RECT clientRect = {0, 0, WIDTH, HEIGHT};
    AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "KAsteroids",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 
        clientRect.right - clientRect.left, clientRect.bottom - clientRect.top,
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}
