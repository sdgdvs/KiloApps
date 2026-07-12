#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>
#include <stdio.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

const int W = 320;
const int H = 240;

const int orig_map1[10][10] = {
    {1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,1,0,0,1},
    {1,0,1,1,0,0,1,0,0,1},
    {1,0,0,1,0,0,0,0,0,1},
    {1,0,0,1,1,1,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,1},
    {1,1,0,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,1,0,1},
    {1,0,0,0,0,0,0,2,0,1},
    {1,1,1,1,1,1,1,1,1,1}
};

const int orig_map2[12][12] = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,1,0,0,0,0,0,1},
    {1,0,1,1,0,1,0,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,0,1,0,1},
    {1,0,0,0,1,0,0,1,0,1,0,1},
    {1,1,1,0,1,0,1,1,0,1,0,1},
    {1,0,0,0,1,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,1,0,1},
    {1,1,1,1,1,1,1,1,0,1,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1}
};

const int orig_map3[15][15] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,1,0,1,0,0,0,0,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,1,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,2,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,0,0,1,0,1,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,0,1,0,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,1,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

const int orig_map4[10][10] = {
    {1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,1,3,0,1},
    {1,1,1,1,1,0,1,1,0,1},
    {1,0,0,0,1,0,0,0,0,1},
    {1,0,1,0,1,1,1,1,1,1},
    {1,0,1,0,0,0,0,0,4,1},
    {1,0,1,1,1,1,1,0,1,1},
    {1,0,0,0,0,0,1,0,2,1},
    {1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1}
};

const int orig_map5[12][12] = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,1,0,0,0,0,0,3,1},
    {1,0,1,0,1,0,1,1,1,1,1,1},
    {1,0,1,0,1,0,0,0,0,0,0,1},
    {1,0,1,0,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,1,0,1},
    {1,0,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,1,0,1,0,1},
    {1,1,1,1,1,1,0,1,0,1,0,1},
    {1,4,0,0,0,1,0,0,0,1,0,1},
    {1,2,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1}
};


int map1[10][10];
int map2[12][12];
int map3[15][15];
int map4[10][10];
int map5[12][12];

void ResetMaps() {
    memcpy(map1, orig_map1, sizeof(map1));
    memcpy(map2, orig_map2, sizeof(map2));
    memcpy(map3, orig_map3, sizeof(map3));
    memcpy(map4, orig_map4, sizeof(map4));
    memcpy(map5, orig_map5, sizeof(map5));
}

int currentLevel = 0;
int keysHeld = 0;

int gameState = 0; // 0=start, 1=play, 2=win
DWORD startTime = 0;
DWORD endTime = 0;
float bestTime = 9999.9f;

void LoadBest() {
    FILE* f = fopen("kmaze_score.dat", "rb");
    if (f) {
        fread(&bestTime, sizeof(float), 1, f);
        fclose(f);
    }
}
void SaveBest() {
    FILE* f = fopen("kmaze_score.dat", "wb");
    if (f) {
        fwrite(&bestTime, sizeof(float), 1, f);
        fclose(f);
    }
}


int GetMapValue(int x, int y) {
    if (x < 0 || y < 0) return 1;
    if (currentLevel == 0) {
        if (x >= 10 || y >= 10) return 1;
        return map1[x][y];
    } else if (currentLevel == 1) {
        if (x >= 12 || y >= 12) return 1;
        return map2[x][y];
    } else if (currentLevel == 2) {
        if (x >= 15 || y >= 15) return 1;
        return map3[x][y];
    } else if (currentLevel == 3) {
        if (x >= 10 || y >= 10) return 1;
        return map4[x][y];
    } else {
        if (x >= 12 || y >= 12) return 1;
        return map5[x][y];
    }
}

void SetMapValue(int x, int y, int v) {
    if (x < 0 || y < 0) return;
    if (currentLevel == 0 && x < 10 && y < 10) map1[x][y] = v;
    else if (currentLevel == 1 && x < 12 && y < 12) map2[x][y] = v;
    else if (currentLevel == 2 && x < 15 && y < 15) map3[x][y] = v;
    else if (currentLevel == 3 && x < 10 && y < 10) map4[x][y] = v;
    else if (currentLevel == 4 && x < 12 && y < 12) map5[x][y] = v;
}

float pX = 1.5f, pY = 1.5f;
void InitGame() { LoadBest(); ResetMaps(); }
float dX = 1.0f, dY = 0.0f;
float planeX = 0.0f, planeY = 0.66f;

void NextLevel() {
    keysHeld = 0;
    currentLevel++;
    if (currentLevel > 4) currentLevel = 0;
    
    // Copy default maps back if we wanted strict reset, but this is simple version so we won't fully deep copy here for Native unless needed.
    // For simplicity, Native C will just let blocks stay erased if you die or loop around.
    
    pX = 1.5f; pY = 1.5f;
    dX = 1.0f; dY = 0.0f;
    planeX = 0.0f; planeY = 0.66f;
}

HBITMAP hbmCanvas = NULL;
HDC hdcMem = NULL;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            SetTimer(hwnd, 1, 30, NULL);
            break;
        case WM_TIMER: {
            float moveSpeed = 0.1f;
            float rotSpeed = 0.05f;
            
            int TryMove(int x, int y) {
                int val = GetMapValue(x, y);
                if (val == 0 || val == 2 || val == 3) return 1;
                if (val == 4) {
                    if (keysHeld > 0) {
                        keysHeld--;
                        SetMapValue(x, y, 0);
                        return 1;
                    }
                }
                return 0;
            }
            
            if (GetAsyncKeyState(VK_UP) & 0x8000) {
                if (TryMove((int)(pX + dX * moveSpeed), (int)pY)) pX += dX * moveSpeed;
                if (TryMove((int)pX, (int)(pY + dY * moveSpeed))) pY += dY * moveSpeed;
            }
            if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
                if (TryMove((int)(pX - dX * moveSpeed), (int)pY)) pX -= dX * moveSpeed;
                if (TryMove((int)pX, (int)(pY - dY * moveSpeed))) pY -= dY * moveSpeed;
            }
            
            int curVal = GetMapValue((int)pX, (int)pY);
            if (curVal == 3) {
                keysHeld++;
                SetMapValue((int)pX, (int)pY, 0);
            } else if (curVal == 2) {
                NextLevel();
            }

            if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
                float oldDX = dX;
                dX = dX * (float)cos(rotSpeed) - dY * (float)sin(rotSpeed);
                dY = oldDX * (float)sin(rotSpeed) + dY * (float)cos(rotSpeed);
                float oldPlaneX = planeX;
                planeX = planeX * (float)cos(rotSpeed) - planeY * (float)sin(rotSpeed);
                planeY = oldPlaneX * (float)sin(rotSpeed) + planeY * (float)cos(rotSpeed);
            }
            if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
                float oldDX = dX;
                dX = dX * (float)cos(-rotSpeed) - dY * (float)sin(-rotSpeed);
                dY = oldDX * (float)sin(-rotSpeed) + dY * (float)cos(-rotSpeed);
                float oldPlaneX = planeX;
                planeX = planeX * (float)cos(-rotSpeed) - planeY * (float)sin(-rotSpeed);
                planeY = oldPlaneX * (float)sin(-rotSpeed) + planeY * (float)cos(-rotSpeed);
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            if (!hdcMem) {
                hdcMem = CreateCompatibleDC(hdc);
                hbmCanvas = CreateCompatibleBitmap(hdc, W, H);
                SelectObject(hdcMem, hbmCanvas);
            }
            
            HBRUSH ceilB = CreateSolidBrush(RGB(50, 50, 50));
            HBRUSH floorB = CreateSolidBrush(RGB(100, 100, 100));
            RECT ceilRc = {0, 0, W, H/2};
            RECT floorRc = {0, H/2, W, H};
            FillRect(hdcMem, &ceilRc, ceilB);
            FillRect(hdcMem, &floorRc, floorB);
            DeleteObject(ceilB); DeleteObject(floorB);

            HBRUSH w1 = CreateSolidBrush(RGB(200, 0, 0));
            HBRUSH w2 = CreateSolidBrush(RGB(150, 0, 0));
            HBRUSH e1 = CreateSolidBrush(RGB(0, 255, 0));
            HBRUSH e2 = CreateSolidBrush(RGB(0, 204, 0));
            HBRUSH k1 = CreateSolidBrush(RGB(255, 204, 0));
            HBRUSH k2 = CreateSolidBrush(RGB(204, 153, 0));
            HBRUSH d1 = CreateSolidBrush(RGB(0, 153, 255));
            HBRUSH d2 = CreateSolidBrush(RGB(0, 102, 204));
            
            HBRUSH w1s = CreateSolidBrush(RGB(170, 0, 0));
            HBRUSH w2s = CreateSolidBrush(RGB(120, 0, 0));
            HBRUSH e1s = CreateSolidBrush(RGB(0, 204, 0));
            HBRUSH e2s = CreateSolidBrush(RGB(0, 153, 0));
            HBRUSH k1s = CreateSolidBrush(RGB(204, 153, 0));
            HBRUSH k2s = CreateSolidBrush(RGB(153, 102, 0));
            HBRUSH d1s = CreateSolidBrush(RGB(0, 102, 204));
            HBRUSH d2s = CreateSolidBrush(RGB(0, 51, 153));
            
            for (int x = 0; x < W; x++) {
                float cameraX = 2 * x / (float)W - 1;
                float rayDX = dX + planeX * cameraX;
                float rayDY = dY + planeY * cameraX;
                
                int mapX = (int)pX;
                int mapY = (int)pY;
                
                float sideDistX, sideDistY;
                float deltaDistX = (rayDX == 0) ? 1e30f : (float)fabs(1 / rayDX);
                float deltaDistY = (rayDY == 0) ? 1e30f : (float)fabs(1 / rayDY);
                float perpWallDist;
                
                int stepX, stepY, hit = 0, side = 0;
                
                if (rayDX < 0) { stepX = -1; sideDistX = (pX - mapX) * deltaDistX; }
                else           { stepX = 1;  sideDistX = (mapX + 1.0f - pX) * deltaDistX; }
                if (rayDY < 0) { stepY = -1; sideDistY = (pY - mapY) * deltaDistY; }
                else           { stepY = 1;  sideDistY = (mapY + 1.0f - pY) * deltaDistY; }
                
                while (hit == 0) {
                    if (sideDistX < sideDistY) {
                        sideDistX += deltaDistX;
                        mapX += stepX;
                        side = 0;
                    } else {
                        sideDistY += deltaDistY;
                        mapY += stepY;
                        side = 1;
                    }
                    if (GetMapValue(mapX, mapY) > 0) hit = GetMapValue(mapX, mapY);
                }
                
                if (side == 0) perpWallDist = (sideDistX - deltaDistX);
                else           perpWallDist = (sideDistY - deltaDistY);
                
                int lineHeight = (int)(H / perpWallDist);
                int drawStart = -lineHeight / 2 + H / 2;
                if (drawStart < 0) drawStart = 0;
                int drawEnd = lineHeight / 2 + H / 2;
                if (drawEnd >= H) drawEnd = H - 1;
                
                float wallX;
                if (side == 0) wallX = pY + perpWallDist * rayDY;
                else           wallX = pX + perpWallDist * rayDX;
                wallX -= floor(wallX);
                int tex = ((int)(wallX * 8.0f)) % 2;
                
                RECT wallRc = {x, drawStart, x+1, drawEnd};
                if (hit == 2) {
                    FillRect(hdcMem, &wallRc, side == 1 ? (tex ? e2s : e2) : (tex ? e1s : e1));
                } else if (hit == 3) {
                    FillRect(hdcMem, &wallRc, side == 1 ? (tex ? k2s : k2) : (tex ? k1s : k1));
                } else if (hit == 4) {
                    FillRect(hdcMem, &wallRc, side == 1 ? (tex ? d2s : d2) : (tex ? d1s : d1));
                } else {
                    FillRect(hdcMem, &wallRc, side == 1 ? (tex ? w2s : w2) : (tex ? w1s : w1));
                }
            }
            DeleteObject(w1); DeleteObject(w2); DeleteObject(e1); DeleteObject(e2); DeleteObject(k1); DeleteObject(k2); DeleteObject(d1); DeleteObject(d2);
            DeleteObject(w1s); DeleteObject(w2s); DeleteObject(e1s); DeleteObject(e2s); DeleteObject(k1s); DeleteObject(k2s); DeleteObject(d1s); DeleteObject(d2s);
            
            // Draw UI
            char uiText[64];
            sprintf(uiText, "Keys: %d  Level: %d", keysHeld, currentLevel + 1);
            SetBkMode(hdcMem, TRANSPARENT);
            SetTextColor(hdcMem, RGB(255, 255, 255));
            TextOutA(hdcMem, 10, 10, uiText, lstrlenA(uiText));

            StretchBlt(hdc, 0, 0, 640, 480, hdcMem, 0, 0, W, H, SRCCOPY);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            if (hdcMem) DeleteDC(hdcMem);
            if (hbmCanvas) DeleteObject(hbmCanvas);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KMazeClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    
    RegisterClassA(&wc);
    
    RECT wr = {0, 0, 640, 480};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    
    HWND hwnd = CreateWindowExA(0, "KMazeClass", "KMaze", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
