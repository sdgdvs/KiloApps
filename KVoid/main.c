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
int hasRedKey = 0;
int hasGreenKey = 0;
int hasBlueKey = 0;
int emps = 0;

typedef struct {
    int x, y, w, h;
} Room;

Room rooms[30];
int roomCount = 0;

typedef struct {
    int x, y, state;
    int stunTimer;
} Alien;

Alien aliens[5];
int alienCount = 0;

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
                        if (rand() % 3 == 0) {
                            int r = rand() % 3;
                            map[y][x] = 3 + r;
                        } else {
                            map[y][x] = 2;
                        }
                    }
                }
            }
        }
    }
    
    if (roomCount > 0) {
        playerX = rooms[0].x + rooms[0].w / 2;
        playerY = rooms[0].y + rooms[0].h / 2;
    }

    for (int i = 0; i < 3; i++) {
        while (1) {
            int rx = rand() % COLS;
            int ry = rand() % ROWS;
            if (map[ry][rx] == 0 && (rx != playerX || ry != playerY)) {
                map[ry][rx] = 6 + i;
                break;
            }
        }
    }

    for (int i = 0; i < 3; i++) {
        while (1) {
            int rx = rand() % COLS;
            int ry = rand() % ROWS;
            if (map[ry][rx] == 0 && (rx != playerX || ry != playerY)) {
                map[ry][rx] = 9;
                break;
            }
        }
    }

    for (int i = 0; i < 5; i++) {
        while (1) {
            int rx = rand() % COLS;
            int ry = rand() % ROWS;
            if (map[ry][rx] == 0 && (rx != playerX || ry != playerY)) {
                map[ry][rx] = 11; // Locker
                break;
            }
        }
    }

    for (int i = 0; i < 3; i++) {
        while (1) {
            int rx = rand() % COLS;
            int ry = rand() % ROWS;
            if (map[ry][rx] == 0 && (rx != playerX || ry != playerY)) {
                map[ry][rx] = 12; // EMP
                break;
            }
        }
    }

    alienCount = 0;
    for (int i = 0; i < 4; i++) {
        while (1) {
            int rx = rand() % COLS;
            int ry = rand() % ROWS;
            if (map[ry][rx] == 0 && (abs(rx - playerX) > 5 || abs(ry - playerY) > 5)) {
                aliens[alienCount].x = rx;
                aliens[alienCount].y = ry;
                aliens[alienCount].state = 0;
                aliens[alienCount].stunTimer = 0;
                alienCount++;
                break;
            }
        }
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
                
                static int alienTick = 0;
                alienTick++;
                if (alienTick >= 12) { // 600ms
                    alienTick = 0;
                    int hidden = (map[playerY][playerX] == 11);
                    if (hidden && sysMsg[0] == '\0') {
                        lstrcpy(sysMsg, "HIDDEN IN LOCKER.");
                        msgTimer = 20;
                    }
                    for (int i = 0; i < alienCount; i++) {
                        if (aliens[i].state == 2) {
                            aliens[i].stunTimer--;
                            if (aliens[i].stunTimer <= 0) aliens[i].state = 0;
                            continue;
                        }

                        int dist = abs(aliens[i].x - playerX) + abs(aliens[i].y - playerY);
                        if (dist <= 6 && !hidden) aliens[i].state = 1;
                        else aliens[i].state = 0;
                        
                        int dx = 0, dy = 0;
                        if (aliens[i].state == 1) {
                            if (abs(playerX - aliens[i].x) > abs(playerY - aliens[i].y)) {
                                dx = (playerX > aliens[i].x) ? 1 : -1;
                                int target = map[aliens[i].y][aliens[i].x + dx];
                                if (target != 0 && (target < 6 || target > 8)) {
                                    dx = 0; 
                                    dy = (playerY > aliens[i].y) ? 1 : (playerY < aliens[i].y ? -1 : 0);
                                }
                            } else {
                                dy = (playerY > aliens[i].y) ? 1 : -1;
                                int target = map[aliens[i].y + dy][aliens[i].x];
                                if (target != 0 && (target < 6 || target > 8)) {
                                    dy = 0; 
                                    dx = (playerX > aliens[i].x) ? 1 : (playerX < aliens[i].x ? -1 : 0);
                                }
                            }
                        } else {
                            if (rand() % 10 < 4) {
                                int r = rand() % 4;
                                if (r == 0) dx = 1;
                                else if (r == 1) dx = -1;
                                else if (r == 2) dy = 1;
                                else dy = -1;
                            }
                        }
                        
                        if (dx != 0 || dy != 0) {
                            int nx = aliens[i].x + dx;
                            int ny = aliens[i].y + dy;
                            if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS) {
                                int target = map[ny][nx];
                                if (target == 0 || (target >= 6 && target <= 8) || target == 11 || target == 12) {
                                    aliens[i].x = nx;
                                    aliens[i].y = ny;
                                }
                            }
                        }
                        
                        if (aliens[i].x == playerX && aliens[i].y == playerY && !hidden) {
                            isDead = 1;
                            lstrcpy(sysMsg, "CAUGHT BY ALIEN. YOU ARE DEAD.");
                            msgTimer = 100;
                        }
                    }
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
            
            HBRUSH hRedDoorBrush = CreateSolidBrush(RGB(wallG, 0, 0));
            HBRUSH hInnerRedDoorBrush = CreateSolidBrush(RGB(inWallG, 0, 0));

            HBRUSH hGreenDoorBrush = CreateSolidBrush(RGB(0, wallG, 0));
            HBRUSH hInnerGreenDoorBrush = CreateSolidBrush(RGB(0, inWallG, 0));
            
            HBRUSH hBlueDoorBrush = CreateSolidBrush(RGB(0, 0, wallG));
            HBRUSH hInnerBlueDoorBrush = CreateSolidBrush(RGB(0, 0, inWallG));

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
                    } else if (map[y][x] >= 3 && map[y][x] <= 5) {
                        HBRUSH b1 = map[y][x] == 3 ? hRedDoorBrush : map[y][x] == 4 ? hGreenDoorBrush : hBlueDoorBrush;
                        HBRUSH b2 = map[y][x] == 3 ? hInnerRedDoorBrush : map[y][x] == 4 ? hInnerGreenDoorBrush : hInnerBlueDoorBrush;
                        FillRect(hdcMem, &tileRect, b1);
                        RECT innerRect = {x * TILE_SIZE + 2, y * TILE_SIZE + 2 + UI_HEIGHT, (x + 1) * TILE_SIZE - 2, (y + 1) * TILE_SIZE - 2 + UI_HEIGHT};
                        FillRect(hdcMem, &innerRect, b2);
                    } else if (map[y][x] >= 6 && map[y][x] <= 8) {
                        FillRect(hdcMem, &tileRect, hFloorBrush);
                        HBRUSH kc = map[y][x] == 6 ? CreateSolidBrush(RGB(255, 0, 0)) : map[y][x] == 7 ? CreateSolidBrush(RGB(0, 255, 0)) : CreateSolidBrush(RGB(50, 50, 255));
                        RECT kr = {x * TILE_SIZE + 4, y * TILE_SIZE + 6 + UI_HEIGHT, x * TILE_SIZE + 12, y * TILE_SIZE + 11 + UI_HEIGHT};
                        FillRect(hdcMem, &kr, kc);
                        DeleteObject(kc);
                    } else if (map[y][x] == 9 || map[y][x] == 10) {
                        FillRect(hdcMem, &tileRect, hFloorBrush);
                        HBRUSH tc = (map[y][x] == 9) ? CreateSolidBrush(RGB(255, 255, 255)) : CreateSolidBrush(RGB(80, 80, 80));
                        HBRUSH tcInner = (map[y][x] == 9) ? CreateSolidBrush(RGB(0, 255, 0)) : CreateSolidBrush(RGB(30, 30, 30));
                        RECT tr = {x * TILE_SIZE + 4, y * TILE_SIZE + 4 + UI_HEIGHT, x * TILE_SIZE + 12, y * TILE_SIZE + 12 + UI_HEIGHT};
                        FillRect(hdcMem, &tr, tc);
                        RECT trIn = {x * TILE_SIZE + 5, y * TILE_SIZE + 5 + UI_HEIGHT, x * TILE_SIZE + 11, y * TILE_SIZE + 11 + UI_HEIGHT};
                        FillRect(hdcMem, &trIn, tcInner);
                        DeleteObject(tc);
                        DeleteObject(tcInner);
                    } else if (map[y][x] == 11) {
                        FillRect(hdcMem, &tileRect, hFloorBrush);
                        HBRUSH lc = CreateSolidBrush(RGB(100, 100, 100));
                        HBRUSH lcc = CreateSolidBrush(RGB(30, 30, 30));
                        RECT lr = {x * TILE_SIZE + 2, y * TILE_SIZE + 2 + UI_HEIGHT, x * TILE_SIZE + 14, y * TILE_SIZE + 14 + UI_HEIGHT};
                        FillRect(hdcMem, &lr, lc);
                        RECT lrc = {x * TILE_SIZE + 7, y * TILE_SIZE + 2 + UI_HEIGHT, x * TILE_SIZE + 9, y * TILE_SIZE + 14 + UI_HEIGHT};
                        FillRect(hdcMem, &lrc, lcc);
                        DeleteObject(lc);
                        DeleteObject(lcc);
                    } else if (map[y][x] == 12) {
                        FillRect(hdcMem, &tileRect, hFloorBrush);
                        HBRUSH ec = CreateSolidBrush(RGB(0, 255, 255));
                        HBRUSH ecc = CreateSolidBrush(RGB(255, 255, 255));
                        RECT er = {x * TILE_SIZE + 5, y * TILE_SIZE + 6 + UI_HEIGHT, x * TILE_SIZE + 11, y * TILE_SIZE + 10 + UI_HEIGHT};
                        FillRect(hdcMem, &er, ec);
                        RECT erc = {x * TILE_SIZE + 7, y * TILE_SIZE + 6 + UI_HEIGHT, x * TILE_SIZE + 9, y * TILE_SIZE + 10 + UI_HEIGHT};
                        FillRect(hdcMem, &erc, ecc);
                        DeleteObject(ec);
                        DeleteObject(ecc);
                    } else {
                        FillRect(hdcMem, &tileRect, hFloorBrush);
                    }
                }
            }
            
            // Draw aliens
            SelectObject(hdcMem, GetStockObject(NULL_PEN));
            int r = TILE_SIZE / 3;
            for (int i = 0; i < alienCount; i++) {
                HBRUSH hAlienBrush;
                if (aliens[i].state == 2) {
                    hAlienBrush = CreateSolidBrush(RGB(85, 85, 255));
                } else {
                    hAlienBrush = CreateSolidBrush(RGB(255, 0, 255));
                }
                SelectObject(hdcMem, hAlienBrush);
                int acx = aliens[i].x * TILE_SIZE + TILE_SIZE / 2;
                int acy = aliens[i].y * TILE_SIZE + TILE_SIZE / 2 + UI_HEIGHT;
                Ellipse(hdcMem, acx - r, acy - r, acx + r, acy + r);
                DeleteObject(hAlienBrush);
            }

            // Draw player
            SelectObject(hdcMem, hPlayerBrush);
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
            TextOut(hdcMem, 10, 5, uiText, lstrlen(uiText));
            
            char keyText[256] = "KEYS: ";
            if (hasRedKey) lstrcat(keyText, "[RED] ");
            if (hasGreenKey) lstrcat(keyText, "[GREEN] ");
            if (hasBlueKey) lstrcat(keyText, "[BLUE] ");
            char empText[32];
            wsprintf(empText, "  EMP: %d", emps);
            lstrcat(keyText, empText);
            SetTextColor(hdcMem, RGB(170, 170, 170));
            TextOut(hdcMem, 350, 5, keyText, lstrlen(keyText));

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
            DeleteObject(hRedDoorBrush);
            DeleteObject(hInnerRedDoorBrush);
            DeleteObject(hGreenDoorBrush);
            DeleteObject(hInnerGreenDoorBrush);
            DeleteObject(hBlueDoorBrush);
            DeleteObject(hInnerBlueDoorBrush);
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
                case VK_SPACE:
                    if (emps > 0) {
                        emps--;
                        lstrcpy(sysMsg, "EMP DEPLOYED. ALIENS STUNNED.");
                        msgTimer = 60;
                        for (int i = 0; i < alienCount; i++) {
                            int dist = abs(aliens[i].x - playerX) + abs(aliens[i].y - playerY);
                            if (dist <= 8) {
                                aliens[i].state = 2; // stunned
                                aliens[i].stunTimer = 10;
                            }
                        }
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    break;
            }
            
            if (newX >= 0 && newX < COLS && newY >= 0 && newY < ROWS) {
                int target = map[newY][newX];
                if (target == 2) {
                    map[newY][newX] = 0;
                    lstrcpy(sysMsg, "Door opened.");
                    msgTimer = 40;
                    InvalidateRect(hwnd, NULL, FALSE);
                } else if (target >= 3 && target <= 5) {
                    int hasKey = (target == 3 && hasRedKey) || (target == 4 && hasGreenKey) || (target == 5 && hasBlueKey);
                    if (hasKey) {
                        map[newY][newX] = 0;
                        lstrcpy(sysMsg, "LOCKED DOOR OPENED.");
                        msgTimer = 40;
                        InvalidateRect(hwnd, NULL, FALSE);
                    } else {
                        if (target == 3) lstrcpy(sysMsg, "LOCKED. RED KEYCARD REQUIRED.");
                        else if (target == 4) lstrcpy(sysMsg, "LOCKED. GREEN KEYCARD REQUIRED.");
                        else if (target == 5) lstrcpy(sysMsg, "LOCKED. BLUE KEYCARD REQUIRED.");
                        msgTimer = 40;
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                } else if (target == 9) {
                    const char* lores[] = {
                        "LOG: EXPERIMENT FAILED.",
                        "LOG: THEY ARE IN THE VENTS.",
                        "LOG: OXYGEN LEAK DETECTED.",
                        "LOG: DIRECTOR IS DEAD."
                    };
                    int l_idx = rand() % 4;
                    int unlockColor = 3 + (rand() % 3);
                    const char* colorName = (unlockColor == 3) ? "RED" : (unlockColor == 4) ? "GREEN" : "BLUE";
                    wsprintf(sysMsg, "%s %s DOORS OPENED.", lores[l_idx], colorName);
                    
                    for (int y = 1; y < ROWS - 1; y++) {
                        for (int x = 1; x < COLS - 1; x++) {
                            if (map[y][x] == unlockColor) map[y][x] = 0;
                        }
                    }
                    map[newY][newX] = 10;
                    msgTimer = 60;
                    InvalidateRect(hwnd, NULL, FALSE);
                } else if (target == 10) {
                    lstrcpy(sysMsg, "TERMINAL ALREADY ACCESSED.");
                    msgTimer = 40;
                    InvalidateRect(hwnd, NULL, FALSE);
                } else if (target == 0 || (target >= 6 && target <= 8) || target == 11 || target == 12) {
                    if (target >= 6 && target <= 8) {
                        if (target == 6) { hasRedKey = 1; lstrcpy(sysMsg, "PICKED UP RED KEYCARD."); }
                        if (target == 7) { hasGreenKey = 1; lstrcpy(sysMsg, "PICKED UP GREEN KEYCARD."); }
                        if (target == 8) { hasBlueKey = 1; lstrcpy(sysMsg, "PICKED UP BLUE KEYCARD."); }
                        map[newY][newX] = 0;
                        msgTimer = 40;
                    }
                    if (target == 12) {
                        emps++;
                        lstrcpy(sysMsg, "PICKED UP EMP CHARGE.");
                        map[newY][newX] = 0;
                        msgTimer = 40;
                    }
                    playerX = newX;
                    playerY = newY;
                    for (int i = 0; i < alienCount; i++) {
                        if (aliens[i].x == playerX && aliens[i].y == playerY && map[playerY][playerX] != 11) {
                            isDead = 1;
                            lstrcpy(sysMsg, "CAUGHT BY ALIEN. YOU ARE DEAD.");
                            msgTimer = 100;
                            InvalidateRect(hwnd, NULL, FALSE);
                            return 0;
                        }
                    }
                    if (target == 0) {
                        sysMsg[0] = '\0';
                        msgTimer = 0;
                    }
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
