#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int _fltused = 1;
long _ftol2_sse(float f) { return (long)f; }
long _ftol2(float f) { return (long)f; }

#define W 320
#define H 240
#define MAP_SIZE 10

int map[MAP_SIZE][MAP_SIZE] = {
    {1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,1,0,0,1},
    {1,0,1,1,0,0,1,0,0,1},
    {1,0,0,1,0,0,0,0,0,1},
    {1,0,0,1,1,1,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,1},
    {1,1,0,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1}
};

float pX = 1.5f, pY = 1.5f;
float dX = 1.0f, dY = 0.0f;
float planeX = 0.0f, planeY = 0.66f;

void Rotate(float speed) {
    // rotate dX, dY
    float my_sin = speed; // approx sin for small angle
    float my_cos = 1.0f - 0.5f * speed * speed; // approx cos
    if (speed < 0) {
        my_sin = -my_sin;
    }
    float oldDX = dX;
    dX = dX * my_cos - dY * speed;
    dY = oldDX * speed + dY * my_cos;
    
    float oldPlaneX = planeX;
    planeX = planeX * my_cos - planeY * speed;
    planeY = oldPlaneX * speed + planeY * my_cos;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            SetTimer(hwnd, 1, 30, NULL);
            break;
        case WM_TIMER: {
            float moveSpeed = 0.1f;
            float rotSpeed = 0.05f;
            if (GetAsyncKeyState(VK_UP) & 0x8000) {
                if (map[(int)(pX + dX * moveSpeed)][(int)pY] == 0) pX += dX * moveSpeed;
                if (map[(int)pX][(int)(pY + dY * moveSpeed)] == 0) pY += dY * moveSpeed;
            }
            if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
                if (map[(int)(pX - dX * moveSpeed)][(int)pY] == 0) pX -= dX * moveSpeed;
                if (map[(int)pX][(int)(pY - dY * moveSpeed)] == 0) pY -= dY * moveSpeed;
            }
            if (GetAsyncKeyState(VK_RIGHT) & 0x8000) Rotate(-rotSpeed);
            if (GetAsyncKeyState(VK_LEFT) & 0x8000) Rotate(rotSpeed);
            
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, W, H);
            SelectObject(memDC, hbm);
            
            // Draw ceiling and floor
            RECT rCeil = {0, 0, W, H/2};
            RECT rFloor = {0, H/2, W, H};
            HBRUSH bCeil = CreateSolidBrush(RGB(50, 50, 50));
            HBRUSH bFloor = CreateSolidBrush(RGB(100, 100, 100));
            FillRect(memDC, &rCeil, bCeil);
            FillRect(memDC, &rFloor, bFloor);
            DeleteObject(bCeil); DeleteObject(bFloor);
            
            HBRUSH w1 = CreateSolidBrush(RGB(200, 0, 0));
            HBRUSH w2 = CreateSolidBrush(RGB(150, 0, 0));
            
            for (int x = 0; x < W; x++) {
                float cameraX = 2.0f * x / (float)W - 1.0f;
                float rayDX = dX + planeX * cameraX;
                float rayDY = dY + planeY * cameraX;
                
                int mapX = (int)pX;
                int mapY = (int)pY;
                
                float sideDistX, sideDistY;
                float deltaDistX = (rayDX == 0.0f) ? 1e30f : ((rayDX < 0.0f) ? -1.0f/rayDX : 1.0f/rayDX);
                float deltaDistY = (rayDY == 0.0f) ? 1e30f : ((rayDY < 0.0f) ? -1.0f/rayDY : 1.0f/rayDY);
                
                int stepX, stepY;
                int hit = 0, side;
                
                if (rayDX < 0) { stepX = -1; sideDistX = (pX - mapX) * deltaDistX; }
                else           { stepX = 1;  sideDistX = (mapX + 1.0f - pX) * deltaDistX; }
                if (rayDY < 0) { stepY = -1; sideDistY = (pY - mapY) * deltaDistY; }
                else           { stepY = 1;  sideDistY = (mapY + 1.0f - pY) * deltaDistY; }
                
                while (!hit) {
                    if (sideDistX < sideDistY) {
                        sideDistX += deltaDistX;
                        mapX += stepX;
                        side = 0;
                    } else {
                        sideDistY += deltaDistY;
                        mapY += stepY;
                        side = 1;
                    }
                    if (map[mapX][mapY] > 0) hit = 1;
                }
                
                float perpWallDist = (side == 0) ? (sideDistX - deltaDistX) : (sideDistY - deltaDistY);
                int lineHeight = (int)(H / perpWallDist);
                int drawStart = -lineHeight / 2 + H / 2;
                if (drawStart < 0) drawStart = 0;
                int drawEnd = lineHeight / 2 + H / 2;
                if (drawEnd >= H) drawEnd = H - 1;
                
                RECT wallRc = {x, drawStart, x+1, drawEnd};
                FillRect(memDC, &wallRc, side == 1 ? w2 : w1);
            }
            
            DeleteObject(w1); DeleteObject(w2);
            
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
    wc.lpszClassName = "KMazeApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KMazeApp", "KMaze", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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
