#include <windows.h>
#include <stdlib.h>
#include <time.h>

#define COLS 40
#define ROWS 30
#define TILE_SIZE 16
#define UI_HEIGHT 40
#define WINDOW_WIDTH (COLS * TILE_SIZE)
#define WINDOW_HEIGHT (ROWS * TILE_SIZE + UI_HEIGHT)

// Map (1 = wall, 0 = floor, 2 = door, 3 = locked door)
int map[ROWS][COLS];

char sysMsg[256] = "";
int msgTimer = 0;

int playerX = 2;
int playerY = 2;
int flickerState = 0;

float oxygen = 100.0f;
float battery = 100.0f;
int isDead = 0;

typedef struct {
    int x, y, w, h;
} Room;

Room rooms[30];
int roomCount = 0;

void GenerateMap() {
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            map[y][x] = 1;
        }
    }
    roomCount = 0;
    
    for (int i = 0; i < 15; i++) {
        int w = (rand() % 5) + 4;
        int h = (rand() % 5) + 4;
        int x = (rand() % (COLS - w - 2)) + 1;
        int y = (rand() % (ROWS - h - 2)) + 1;
        
        int failed = 0;
        for (int j = 0; j < roomCount; j++) {
            if (x < rooms[j].x + rooms[j].w + 1 && x + w + 1 > rooms[j].x &&
                y < rooms[j].y + rooms[j].h + 1 && y + h + 1 > rooms[j].y) {
                failed = 1;
                break;
            }
        }
        
        if (!failed) {
            rooms[roomCount].x = x;
            rooms[roomCount].y = y;
            rooms[roomCount].w = w;
            rooms[roomCount].h = h;
            for (int ry = y; ry < y + h; ry++) {
                for (int rx = x; rx < x + w; rx++) {
                    map[ry][rx] = 0;
                }
            }
            roomCount++;
        }
    }
    
    for (int i = 1; i < roomCount; i++) {
        int cx1 = rooms[i-1].x + rooms[i-1].w / 2;
        int cy1 = rooms[i-1].y + rooms[i-1].h / 2;
        int cx2 = rooms[i].x + rooms[i].w / 2;
        int cy2 = rooms[i].y + rooms[i].h / 2;
        
        int x = cx1;
        int y = cy1;
        while (x != cx2) {
            map[y][x] = 0;
            x += (cx2 > cx1) ? 1 : -1;
        }
        while (y != cy2) {
            map[y][x] = 0;
            y += (cy2 > cy1) ? 1 : -1;
        }
    }
    
    for (int y = 1; y < ROWS - 1; y++) {
        for (int x = 1; x < COLS - 1; x++) {
            if (map[y][x] == 0) {
                int wallsVert = (map[y-1][x] == 1 && map[y+1][x] == 1 && map[y][x-1] == 0 && map[y][x+1] == 0);
                int wallsHoriz = (map[y][x-1] == 1 && map[y][x+1] == 1 && map[y-1][x] == 0 && map[y+1][x] == 0);
                if (wallsVert || wallsHoriz) {
                    if (rand() % 5 == 0) {
                        map[y][x] = (rand() % 3 == 0) ? 3 : 2;
                    }
                }
            }
        }
    }
    
    if (roomCount > 0) {
        playerX = rooms[0].x + rooms[0].w / 2;
        playerY = rooms[0].y + rooms[0].h / 2;
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            SetTimer(hwnd, 1, 50, NULL);
            return 0;
        case WM_TIMER:
            if (!isDead) {
                static int tickCount = 0;
                tickCount++;
                if (tickCount >= 20) {
                    tickCount = 0;
                    oxygen -= 0.5f;
                    battery -= 0.2f;
                    if (oxygen <= 0) { oxygen = 0; isDead = 1; }
                    if (battery <= 0) { battery = 0; isDead = 1; }
                }
            }
            if (msgTimer > 0) {
                msgTimer--;
                if (msgTimer == 0) sysMsg[0] = '\0';
            }
            if (rand() % 20 == 0) {
                flickerState = 1;
            } else {
                flickerState = 0;
            }
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Create double buffer
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
            
            // Draw map
            int wallG = flickerState ? 50 : 170;
            int inWallG = flickerState ? 20 : 51;
            int floorC = flickerState ? 0 : 5;
            int pG = flickerState ? 100 : 255;

            HBRUSH hWallBrush = CreateSolidBrush(RGB(0, wallG, 0));
            HBRUSH hInnerWallBrush = CreateSolidBrush(RGB(0, inWallG, 0));
            
            HBRUSH hDoorBrush = CreateSolidBrush(RGB(wallG, wallG, 0));
            HBRUSH hInnerDoorBrush = CreateSolidBrush(RGB(inWallG, inWallG, 0));
            
            HBRUSH hLockedBrush = CreateSolidBrush(RGB(wallG, 0, 0));
            HBRUSH hInnerLockedBrush = CreateSolidBrush(RGB(inWallG, 0, 0));

            HBRUSH hFloorBrush = CreateSolidBrush(RGB(floorC, floorC, floorC));
            HBRUSH hPlayerBrush = CreateSolidBrush(RGB(0, pG, 0));
            
            // Clear background with black (though floor covers it)
            RECT bgRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
            FillRect(hdcMem, &bgRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

            for (int y = 0; y < ROWS; y++) {
                for (int x = 0; x < COLS; x++) {
                    RECT tileRect = {x * TILE_SIZE, y * TILE_SIZE + UI_HEIGHT, (x + 1) * TILE_SIZE, (y + 1) * TILE_SIZE + UI_HEIGHT};
                    if (map[y][x] == 1) {
                        FillRect(hdcMem, &tileRect, hWallBrush);
                        RECT innerRect = {x * TILE_SIZE + 2, y * TILE_SIZE + 2 + UI_HEIGHT, (x + 1) * TILE_SIZE - 2, (y + 1) * TILE_SIZE - 2 + UI_HEIGHT};
                        FillRect(hdcMem, &innerRect, hInnerWallBrush);
                    } else if (map[y][x] == 2) {
                        FillRect(hdcMem, &tileRect, hDoorBrush);
                        RECT innerRect = {x * TILE_SIZE + 2, y * TILE_SIZE + 2 + UI_HEIGHT, (x + 1) * TILE_SIZE - 2, (y + 1) * TILE_SIZE - 2 + UI_HEIGHT};
                        FillRect(hdcMem, &innerRect, hInnerDoorBrush);
                    } else if (map[y][x] == 3) {
                        FillRect(hdcMem, &tileRect, hLockedBrush);
                        RECT innerRect = {x * TILE_SIZE + 2, y * TILE_SIZE + 2 + UI_HEIGHT, (x + 1) * TILE_SIZE - 2, (y + 1) * TILE_SIZE - 2 + UI_HEIGHT};
                        FillRect(hdcMem, &innerRect, hInnerLockedBrush);
                    } else {
                        FillRect(hdcMem, &tileRect, hFloorBrush);
                    }
                }
            }
            
            // Draw player
            SelectObject(hdcMem, hPlayerBrush);
            SelectObject(hdcMem, GetStockObject(NULL_PEN));
            int r = TILE_SIZE / 3;
            int cx = playerX * TILE_SIZE + TILE_SIZE / 2;
            int cy = playerY * TILE_SIZE + TILE_SIZE / 2 + UI_HEIGHT;
            Ellipse(hdcMem, cx - r, cy - r, cx + r, cy + r);
            
            // Draw UI Text
            SetBkMode(hdcMem, TRANSPARENT);
            char uiText[256];
            wsprintf(uiText, "OXYGEN: %d%%    BATTERY: %d%%", (int)oxygen, (int)battery);
            if (oxygen <= 20 || battery <= 20) {
                SetTextColor(hdcMem, RGB(255, 0, 0));
            } else {
                SetTextColor(hdcMem, RGB(0, 255, 0));
            }
            TextOut(hdcMem, 10, 10, uiText, lstrlen(uiText));

            if (sysMsg[0] != '\0') {
                SetTextColor(hdcMem, RGB(0, 255, 0));
                TextOut(hdcMem, 10, 25, sysMsg, lstrlen(sysMsg));
            }

            if (isDead) {
                SetTextColor(hdcMem, RGB(255, 0, 0));
                char* deadMsg = "SIGNAL LOST";
                TextOut(hdcMem, WINDOW_WIDTH / 2 - 40, WINDOW_HEIGHT / 2, deadMsg, lstrlen(deadMsg));
            }

            // Draw scanlines
            HPEN hScanlinePen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            HPEN hOldPen = (HPEN)SelectObject(hdcMem, hScanlinePen);
            for (int y = UI_HEIGHT; y < WINDOW_HEIGHT; y += 4) {
                MoveToEx(hdcMem, 0, y, NULL);
                LineTo(hdcMem, WINDOW_WIDTH, y);
            }
            SelectObject(hdcMem, hOldPen);
            DeleteObject(hScanlinePen);

            // Copy to screen
            BitBlt(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hdcMem, 0, 0, SRCCOPY);
            
            // Cleanup
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);
            DeleteObject(hWallBrush);
            DeleteObject(hInnerWallBrush);
            DeleteObject(hDoorBrush);
            DeleteObject(hInnerDoorBrush);
            DeleteObject(hLockedBrush);
            DeleteObject(hInnerLockedBrush);
            DeleteObject(hFloorBrush);
            DeleteObject(hPlayerBrush);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_KEYDOWN: {
            if (isDead) return 0;
            int newX = playerX;
            int newY = playerY;
            
            switch (wParam) {
                case VK_UP:
                case 'W':
                    newY--;
                    break;
                case VK_DOWN:
                case 'S':
                    newY++;
                    break;
                case VK_LEFT:
                case 'A':
                    newX--;
                    break;
                case VK_RIGHT:
                case 'D':
                    newX++;
                    break;
            }
            
            if (newX >= 0 && newX < COLS && newY >= 0 && newY < ROWS) {
                if (map[newY][newX] == 2) {
                    map[newY][newX] = 0;
                    lstrcpy(sysMsg, "Door opened.");
                    msgTimer = 40;
                    InvalidateRect(hwnd, NULL, FALSE);
                } else if (map[newY][newX] == 3) {
                    lstrcpy(sysMsg, "DOOR LOCKED. KEYCARD REQUIRED.");
                    msgTimer = 40;
                    InvalidateRect(hwnd, NULL, FALSE);
                } else if (map[newY][newX] == 0) {
                    playerX = newX;
                    playerY = newY;
                    sysMsg[0] = '\0';
                    msgTimer = 0;
                    oxygen -= 0.2f;
                    if (oxygen <= 0) {
                        oxygen = 0;
                        isDead = 1;
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            return 0;
        }
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    srand((unsigned int)time(NULL));
    GenerateMap();
    const char CLASS_NAME[] = "KVoid Class";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    RegisterClass(&wc);

    RECT rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
    
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "KVoid",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
