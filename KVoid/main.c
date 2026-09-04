#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

DWORD WINAPI SoundThread(LPVOID lpParam) {
    int type = (int)(intptr_t)lpParam;
    if (type == 1) { // Heartbeat
        Beep(100, 100);
        Sleep(100);
        Beep(100, 100);
    } else if (type == 2) { // Alarm
        Beep(800, 200);
        Beep(600, 200);
    } else if (type == 3) { // Screech
        for (int i=2000; i>100; i-=200) Beep(i, 20);
    }
    return 0;
}
void PlaySoundEffect(int type) {
    CreateThread(NULL, 0, SoundThread, (LPVOID)(intptr_t)type, 0, NULL);
}


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
int deck = 1;
int isDead = 0;
int hasRedKey = 0;
int hasGreenKey = 0;
int hasBlueKey = 0;
int emps = 0;
int totalTime = 0;
int selfDestructActive = 0;
int selfDestructTimer = 0;
int wonGame = 0;
char winEnding[256] = "";
int showHelp = 0;

int playerDir = 0;

typedef struct {
    float x, y;
    float vx, vy;
    int life;
    COLORREF color;
} Particle;
#define MAX_PARTICLES 100
Particle particles[MAX_PARTICLES];

typedef struct {
    float x, y;
    float r, maxR;
    int life;
    COLORREF color;
} Shockwave;
#define MAX_SHOCKWAVES 8
Shockwave shockwaves[MAX_SHOCKWAVES];
int screenShake = 0;

void SpawnParticles(float x, float y, COLORREF color, int count) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < MAX_PARTICLES; j++) {
            if (particles[j].life <= 0) {
                particles[j].x = x;
                particles[j].y = y;
                particles[j].vx = ((rand() % 100) / 25.0f) - 2.0f;
                particles[j].vy = ((rand() % 100) / 25.0f) - 2.0f;
                particles[j].life = 10 + (rand() % 20);
                particles[j].color = color;
                break;
            }
        }
    }
}

void AddShockwave(float x, float y, COLORREF color, float maxR) {
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (shockwaves[i].life <= 0) {
            shockwaves[i].x = x;
            shockwaves[i].y = y;
            shockwaves[i].r = 2.0f;
            shockwaves[i].maxR = maxR;
            shockwaves[i].life = 16;
            shockwaves[i].color = color;
            break;
        }
    }
}

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

    if (deck == 5) {
        int placed = 0;
        while (!placed) {
            int rx = rand() % COLS;
            int ry = rand() % ROWS;
            if (map[ry][rx] == 0 && (abs(rx - playerX) > 10 || abs(ry - playerY) > 10)) {
                map[ry][rx] = 14;
                placed = 1;
            }
        }
        placed = 0;
        while (!placed) {
            int rx = rand() % COLS;
            int ry = rand() % ROWS;
            if (map[ry][rx] == 0 && (abs(rx - playerX) > 10 || abs(ry - playerY) > 10)) {
                map[ry][rx] = 15;
                placed = 1;
            }
        }
    } else {
        int elevatorPlaced = 0;
        for (int attempts = 0; attempts < 1000; attempts++) {
            int rx = rand() % COLS;
            int ry = rand() % ROWS;
            if (map[ry][rx] == 0 && (abs(rx - playerX) > 5 || abs(ry - playerY) > 5)) {
                map[ry][rx] = 13; // Elevator
                elevatorPlaced = 1;
                break;
            }
        }
        if (!elevatorPlaced) {
            for (int y = 1; y < ROWS - 1 && !elevatorPlaced; y++) {
                for (int x = 1; x < COLS - 1 && !elevatorPlaced; x++) {
                    if (map[y][x] == 0 && (x != playerX || y != playerY)) {
                        map[y][x] = 13;
                        elevatorPlaced = 1;
                    }
                }
            }
        }
    }

    alienCount = 0;
    int numAliens = 3 + deck;
    if (numAliens > 15) numAliens = 15;
    for (int i = 0; i < numAliens; i++) {
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
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (particles[i].life > 0) {
                    particles[i].x += particles[i].vx;
                    particles[i].y += particles[i].vy;
                    particles[i].life--;
                }
            }
            for (int i = 0; i < MAX_SHOCKWAVES; i++) {
                if (shockwaves[i].life > 0) {
                    shockwaves[i].r += (shockwaves[i].maxR - 2.0f) / 16.0f;
                    shockwaves[i].life--;
                }
            }
            if (screenShake > 0) screenShake--;

            if (!isDead && !wonGame && !showHelp) {
                static int tickCount = 0;
                tickCount++;
                if (tickCount >= 20) {
                    tickCount = 0;
                    totalTime++;
                    oxygen -= 0.5f;
                    battery -= 0.2f;
                    
                    int chasing = 0;
                    for (int i = 0; i < alienCount; i++) {
                        if (aliens[i].state == 1) chasing = 1;
                    }

                    if (oxygen <= 0) { oxygen = 0; isDead = 1; lstrcpy(sysMsg, "OXYGEN DEPLETED. YOU SUFFOCATED."); msgTimer = 100; screenShake = 8; }
                    if (battery <= 0) { battery = 0; isDead = 1; lstrcpy(sysMsg, "BATTERY DEPLETED. CONSUMED BY THE DARK."); msgTimer = 100; screenShake = 8; }
                    if (selfDestructActive) {
                        selfDestructTimer--;
                        wsprintf(sysMsg, "SELF DESTRUCT IN %ds", selfDestructTimer);
                        msgTimer = 20;
                        screenShake = 3;
                        PlaySoundEffect(2);
                        if (selfDestructTimer <= 0) {
                            isDead = 1;
                            lstrcpy(sysMsg, "STATION DESTROYED. YOU WERE INCINERATED.");
                            msgTimer = 100;
                            screenShake = 15;
                        }
                    } else if (oxygen <= 20 || battery <= 20 || chasing) {
                        PlaySoundEffect(1);
                    }
                }
                
                static int alienTick = 0;
                alienTick++;
                int speedThresh = 12 - deck;
                if (speedThresh < 4) speedThresh = 4;
                if (alienTick >= speedThresh) {
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
                            PlaySoundEffect(3);
                            float dpx = playerX * TILE_SIZE + TILE_SIZE / 2.0f;
                            float dpy = playerY * TILE_SIZE + TILE_SIZE / 2.0f + UI_HEIGHT;
                            SpawnParticles(dpx, dpy, RGB(255, 0, 0), 60);
                            AddShockwave(dpx, dpy, RGB(255, 0, 50), 90.0f);
                            screenShake = 10;
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
            
            // Double buffer
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
            
            // Clear background
            RECT bgRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
            HBRUSH hBlack = (HBRUSH)GetStockObject(BLACK_BRUSH);
            FillRect(hdcMem, &bgRect, hBlack);

            DWORD ticks = GetTickCount();

            // Reusable GDI brushes
            HBRUSH hFloorBrush = CreateSolidBrush(RGB(6, 10, 14));
            HBRUSH hRivetBrush = CreateSolidBrush(RGB(20, 34, 48));
            HPEN hFloorGridPen = CreatePen(PS_SOLID, 1, RGB(14, 22, 31));

            HBRUSH hWallOuter = CreateSolidBrush(RGB(15, 23, 30));
            HBRUSH hWallInner = CreateSolidBrush(RGB(22, 34, 46));
            HPEN hWallHiPen = CreatePen(PS_SOLID, 1, RGB(45, 68, 92));
            HPEN hWallShPen = CreatePen(PS_SOLID, 1, RGB(6, 10, 14));
            HBRUSH hConduitBrush = CreateSolidBrush(RGB(0, 220, 110));

            // Draw map tiles
            for (int y = 0; y < ROWS; y++) {
                for (int x = 0; x < COLS; x++) {
                    int px = x * TILE_SIZE;
                    int py = y * TILE_SIZE + UI_HEIGHT;
                    int tile = map[y][x];

                    // Base floor plating
                    RECT tRect = {px, py, px + TILE_SIZE, py + TILE_SIZE};
                    FillRect(hdcMem, &tRect, hFloorBrush);

                    // Floor grid & rivets
                    HPEN hOldP = (HPEN)SelectObject(hdcMem, hFloorGridPen);
                    MoveToEx(hdcMem, px, py, NULL); LineTo(hdcMem, px + TILE_SIZE, py);
                    MoveToEx(hdcMem, px, py, NULL); LineTo(hdcMem, px, py + TILE_SIZE);
                    SelectObject(hdcMem, hOldP);

                    RECT r1 = {px + 1, py + 1, px + 2, py + 2};
                    RECT r2 = {px + TILE_SIZE - 2, py + 1, px + TILE_SIZE - 1, py + 2};
                    FillRect(hdcMem, &r1, hRivetBrush);
                    FillRect(hdcMem, &r2, hRivetBrush);

                    if (tile == 1) {
                        // Bulkhead Wall
                        FillRect(hdcMem, &tRect, hWallOuter);
                        RECT inW = {px + 2, py + 2, px + TILE_SIZE - 2, py + TILE_SIZE - 2};
                        FillRect(hdcMem, &inW, hWallInner);

                        // 3D Bevel Lines
                        HPEN hP = (HPEN)SelectObject(hdcMem, hWallHiPen);
                        MoveToEx(hdcMem, px + 2, py + TILE_SIZE - 3, NULL);
                        LineTo(hdcMem, px + 2, py + 2);
                        LineTo(hdcMem, px + TILE_SIZE - 2, py + 2);
                        SelectObject(hdcMem, hWallShPen);
                        MoveToEx(hdcMem, px + TILE_SIZE - 3, py + 2, NULL);
                        LineTo(hdcMem, px + TILE_SIZE - 3, py + TILE_SIZE - 3);
                        LineTo(hdcMem, px + 2, py + TILE_SIZE - 3);
                        SelectObject(hdcMem, hP);

                        // Conduit LED
                        if ((x + y + (ticks / 300)) % 7 == 0) {
                            RECT cLed = {px + 7, py + 7, px + 9, py + 9};
                            FillRect(hdcMem, &cLed, hConduitBrush);
                        }
                    } else if (tile == 2) {
                        // Standard Hydraulic Blast Door
                        HBRUSH hDFrame = CreateSolidBrush(RGB(34, 40, 26));
                        HBRUSH hDPlates = CreateSolidBrush(RGB(68, 76, 56));
                        HBRUSH hDYellow = CreateSolidBrush(RGB(212, 160, 23));
                        FillRect(hdcMem, &tRect, hDFrame);
                        RECT dP1 = {px + 3, py + 2, px + 7, py + TILE_SIZE - 2};
                        RECT dP2 = {px + 9, py + 2, px + 13, py + TILE_SIZE - 2};
                        FillRect(hdcMem, &dP1, hDPlates);
                        FillRect(hdcMem, &dP2, hDPlates);
                        RECT dHz1 = {px + 1, py + 1, px + 4, py + 3};
                        RECT dHz2 = {px + TILE_SIZE - 4, py + 1, px + TILE_SIZE - 1, py + 3};
                        FillRect(hdcMem, &dHz1, hDYellow);
                        FillRect(hdcMem, &dHz2, hDYellow);
                        RECT dLed = {px + 7, py + 6, px + 9, py + 10};
                        FillRect(hdcMem, &dLed, hDYellow);
                        DeleteObject(hDFrame);
                        DeleteObject(hDPlates);
                        DeleteObject(hDYellow);
                    } else if (tile >= 3 && tile <= 5) {
                        // Security Keycard Locked Blast Doors
                        COLORREF cMain = (tile == 3) ? RGB(230, 25, 25) : (tile == 4) ? RGB(0, 230, 75) : RGB(40, 125, 255);
                        COLORREF cDark = (tile == 3) ? RGB(50, 10, 10) : (tile == 4) ? RGB(8, 45, 18) : RGB(10, 30, 60);
                        HBRUSH hCFrame = CreateSolidBrush(cDark);
                        HBRUSH hCLed = CreateSolidBrush(cMain);
                        FillRect(hdcMem, &tRect, hCFrame);
                        HPEN hCPen = CreatePen(PS_SOLID, 1, cMain);
                        HPEN hOld = (HPEN)SelectObject(hdcMem, hCPen);
                        Rectangle(hdcMem, px + 2, py + 2, px + TILE_SIZE - 2, py + TILE_SIZE - 2);
                        SelectObject(hdcMem, hOld);
                        DeleteObject(hCPen);
                        RECT cSlot = {px + 6, py + 6, px + 10, py + 10};
                        FillRect(hdcMem, &cSlot, hCLed);
                        DeleteObject(hCFrame);
                        DeleteObject(hCLed);
                    } else if (tile >= 6 && tile <= 8) {
                        // High-Tech Keycard
                        COLORREF cKey = (tile == 6) ? RGB(255, 50, 50) : (tile == 7) ? RGB(50, 255, 85) : RGB(50, 150, 255);
                        HBRUSH hKC = CreateSolidBrush(cKey);
                        HBRUSH hCardBody = CreateSolidBrush(RGB(26, 31, 38));
                        HBRUSH hGold = CreateSolidBrush(RGB(255, 215, 0));
                        HBRUSH hWhite = CreateSolidBrush(RGB(255, 255, 255));
                        RECT cardR = {px + 3, py + 4, px + 13, py + 12};
                        FillRect(hdcMem, &cardR, hCardBody);
                        RECT bandR = {px + 3, py + 7, px + 13, py + 10};
                        FillRect(hdcMem, &bandR, hKC);
                        RECT goldR = {px + 5, py + 5, px + 8, py + 7};
                        FillRect(hdcMem, &goldR, hGold);
                        RECT wR = {px + 9, py + 5, px + 11, py + 6};
                        FillRect(hdcMem, &wR, hWhite);
                        DeleteObject(hKC);
                        DeleteObject(hCardBody);
                        DeleteObject(hGold);
                        DeleteObject(hWhite);
                    } else if (tile == 9 || tile == 10) {
                        // Mainframe Computer Terminal
                        int act = (tile == 9);
                        HBRUSH hDesk = CreateSolidBrush(RGB(21, 28, 36));
                        HBRUSH hMon = CreateSolidBrush(act ? RGB(26, 40, 56) : RGB(20, 24, 31));
                        HBRUSH hScr = CreateSolidBrush(act ? RGB(0, 45, 20) : RGB(8, 12, 16));
                        HBRUSH hLed = CreateSolidBrush(act ? RGB(0, 255, 100) : RGB(40, 55, 70));
                        RECT dR = {px + 2, py + 10, px + 14, py + 14};
                        FillRect(hdcMem, &dR, hDesk);
                        RECT mR = {px + 2, py + 2, px + 14, py + 10};
                        FillRect(hdcMem, &mR, hMon);
                        RECT sR = {px + 4, py + 4, px + 12, py + 8};
                        FillRect(hdcMem, &sR, hScr);
                        if (act) {
                            int scanY = (ticks / 100) % 4;
                            RECT scR = {px + 4, py + 4 + scanY, px + 12, py + 5 + scanY};
                            FillRect(hdcMem, &scR, hLed);
                        }
                        DeleteObject(hDesk);
                        DeleteObject(hMon);
                        DeleteObject(hScr);
                        DeleteObject(hLed);
                    } else if (tile == 11) {
                        // Crew Survival Locker
                        HBRUSH hLBase = CreateSolidBrush(RGB(28, 36, 44));
                        HBRUSH hLDoor = CreateSolidBrush(RGB(45, 58, 71));
                        HBRUSH hLLouv = CreateSolidBrush(RGB(11, 15, 20));
                        HBRUSH hLGold = CreateSolidBrush(RGB(245, 197, 24));
                        RECT lR = {px + 2, py + 1, px + 14, py + 15};
                        FillRect(hdcMem, &lR, hLBase);
                        RECT d1 = {px + 3, py + 2, px + 8, py + 14};
                        RECT d2 = {px + 9, py + 2, px + 14, py + 14};
                        FillRect(hdcMem, &d1, hLDoor);
                        FillRect(hdcMem, &d2, hLDoor);
                        RECT lv1 = {px + 4, py + 4, px + 7, py + 5};
                        RECT lv2 = {px + 4, py + 7, px + 7, py + 8};
                        RECT lv3 = {px + 10, py + 4, px + 13, py + 5};
                        RECT lv4 = {px + 10, py + 7, px + 13, py + 8};
                        FillRect(hdcMem, &lv1, hLLouv);
                        FillRect(hdcMem, &lv2, hLLouv);
                        FillRect(hdcMem, &lv3, hLLouv);
                        FillRect(hdcMem, &lv4, hLLouv);
                        RECT pad = {px + 7, py + 10, px + 10, py + 12};
                        FillRect(hdcMem, &pad, hLGold);
                        DeleteObject(hLBase);
                        DeleteObject(hLDoor);
                        DeleteObject(hLLouv);
                        DeleteObject(hLGold);
                    } else if (tile == 12) {
                        // EMP Canister
                        HBRUSH hEMPBody = CreateSolidBrush(RGB(43, 56, 71));
                        HBRUSH hEMPPlasma = CreateSolidBrush(RGB(0, 220, 255));
                        HBRUSH hEMPRing = CreateSolidBrush(RGB(255, 255, 255));
                        RECT ebR = {px + 4, py + 4, px + 12, py + 12};
                        FillRect(hdcMem, &ebR, hEMPBody);
                        RECT epR = {px + 5, py + 6, px + 11, py + 10};
                        FillRect(hdcMem, &epR, hEMPPlasma);
                        RECT er1 = {px + 6, py + 5, px + 10, py + 6};
                        RECT er2 = {px + 6, py + 10, px + 10, py + 11};
                        FillRect(hdcMem, &er1, hEMPRing);
                        FillRect(hdcMem, &er2, hEMPRing);
                        DeleteObject(hEMPBody);
                        DeleteObject(hEMPPlasma);
                        DeleteObject(hEMPRing);
                    } else if (tile == 13) {
                        // Industrial Cargo Lift / Elevator
                        HBRUSH hElvFrame = CreateSolidBrush(RGB(17, 17, 17));
                        HBRUSH hElvPlat = CreateSolidBrush(RGB(43, 51, 61));
                        HBRUSH hElvGrate = CreateSolidBrush(RGB(24, 30, 36));
                        HBRUSH hElvArrow = CreateSolidBrush(RGB(0, 255, 136));
                        HBRUSH hElvHz = CreateSolidBrush(RGB(255, 215, 0));
                        FillRect(hdcMem, &tRect, hElvFrame);
                        RECT elvIn = {px + 2, py + 2, px + TILE_SIZE - 2, py + TILE_SIZE - 2};
                        FillRect(hdcMem, &elvIn, hElvPlat);
                        RECT elvGr = {px + 4, py + 4, px + TILE_SIZE - 4, py + TILE_SIZE - 4};
                        FillRect(hdcMem, &elvGr, hElvGrate);
                        // Chevron arrow
                        POINT arrPts[3] = {
                            {px + 8, py + 6},
                            {px + 12, py + 10},
                            {px + 4, py + 10}
                        };
                        HBRUSH hOldB = (HBRUSH)SelectObject(hdcMem, hElvArrow);
                        HPEN hOldP = (HPEN)SelectObject(hdcMem, GetStockObject(NULL_PEN));
                        Polygon(hdcMem, arrPts, 3);
                        SelectObject(hdcMem, hOldP);
                        SelectObject(hdcMem, hOldB);
                        // Hazard notches
                        RECT hz1 = {px, py, px + 3, py + 2};
                        RECT hz2 = {px + TILE_SIZE - 3, py + TILE_SIZE - 2, px + TILE_SIZE, py + TILE_SIZE};
                        FillRect(hdcMem, &hz1, hElvHz);
                        FillRect(hdcMem, &hz2, hElvHz);
                        DeleteObject(hElvFrame);
                        DeleteObject(hElvPlat);
                        DeleteObject(hElvGrate);
                        DeleteObject(hElvArrow);
                        DeleteObject(hElvHz);
                    } else if (tile == 14) {
                        // Escape Pod Airlock
                        HBRUSH hEpPlat = CreateSolidBrush(RGB(24, 36, 48));
                        FillRect(hdcMem, &tRect, hEpPlat);
                        HPEN hEpPen = CreatePen(PS_SOLID, 2, RGB(0, 229, 255));
                        HBRUSH hEpNull = (HBRUSH)GetStockObject(NULL_BRUSH);
                        HPEN hOldP = (HPEN)SelectObject(hdcMem, hEpPen);
                        HBRUSH hOldB = (HBRUSH)SelectObject(hdcMem, hEpNull);
                        Ellipse(hdcMem, px + 2, py + 2, px + 14, py + 14);
                        HBRUSH hEpCore = CreateSolidBrush(RGB(0, 229, 255));
                        SelectObject(hdcMem, hEpCore);
                        Ellipse(hdcMem, px + 6, py + 6, px + 10, py + 10);
                        SelectObject(hdcMem, hOldB);
                        SelectObject(hdcMem, hOldP);
                        DeleteObject(hEpPen);
                        DeleteObject(hEpCore);
                        DeleteObject(hEpPlat);
                    } else if (tile == 15) {
                        // Self Destruct Terminal
                        HBRUSH hSdFrame = CreateSolidBrush(RGB(38, 8, 8));
                        HBRUSH hSdConsole = CreateSolidBrush(RGB(74, 18, 18));
                        HBRUSH hSdBeacon = CreateSolidBrush(((ticks / 200) % 2) ? RGB(255, 0, 0) : RGB(255, 100, 0));
                        FillRect(hdcMem, &tRect, hSdFrame);
                        RECT sdC = {px + 2, py + 2, px + 14, py + 14};
                        FillRect(hdcMem, &sdC, hSdConsole);
                        RECT sdB = {px + 6, py + 6, px + 10, py + 10};
                        FillRect(hdcMem, &sdB, hSdBeacon);
                        DeleteObject(hSdFrame);
                        DeleteObject(hSdConsole);
                        DeleteObject(hSdBeacon);
                    }
                }
            }

            // Cleanup wall/floor brushes
            DeleteObject(hFloorBrush);
            DeleteObject(hRivetBrush);
            DeleteObject(hFloorGridPen);
            DeleteObject(hWallOuter);
            DeleteObject(hWallInner);
            DeleteObject(hWallHiPen);
            DeleteObject(hWallShPen);
            DeleteObject(hConduitBrush);

            // Draw Flashlight Beam Polygon
            if (!isDead) {
                int pcx = playerX * TILE_SIZE + TILE_SIZE / 2;
                int pcy = playerY * TILE_SIZE + TILE_SIZE / 2 + UI_HEIGHT;
                HPEN hFlashPen = CreatePen(PS_SOLID, 1, RGB(0, 180, 100));
                HPEN hOldP = (HPEN)SelectObject(hdcMem, hFlashPen);
                if (playerDir == 0) { // Up
                    MoveToEx(hdcMem, pcx, pcy, NULL); LineTo(hdcMem, pcx - 24, pcy - 48);
                    MoveToEx(hdcMem, pcx, pcy, NULL); LineTo(hdcMem, pcx + 24, pcy - 48);
                    MoveToEx(hdcMem, pcx - 24, pcy - 48, NULL); LineTo(hdcMem, pcx + 24, pcy - 48);
                } else if (playerDir == 1) { // Down
                    MoveToEx(hdcMem, pcx, pcy, NULL); LineTo(hdcMem, pcx - 24, pcy + 48);
                    MoveToEx(hdcMem, pcx, pcy, NULL); LineTo(hdcMem, pcx + 24, pcy + 48);
                    MoveToEx(hdcMem, pcx - 24, pcy + 48, NULL); LineTo(hdcMem, pcx + 24, pcy + 48);
                } else if (playerDir == 2) { // Left
                    MoveToEx(hdcMem, pcx, pcy, NULL); LineTo(hdcMem, pcx - 48, pcy - 24);
                    MoveToEx(hdcMem, pcx, pcy, NULL); LineTo(hdcMem, pcx - 48, pcy + 24);
                    MoveToEx(hdcMem, pcx - 48, pcy - 24, NULL); LineTo(hdcMem, pcx - 48, pcy + 24);
                } else { // Right
                    MoveToEx(hdcMem, pcx, pcy, NULL); LineTo(hdcMem, pcx + 48, pcy - 24);
                    MoveToEx(hdcMem, pcx, pcy, NULL); LineTo(hdcMem, pcx + 48, pcy + 24);
                    MoveToEx(hdcMem, pcx + 48, pcy - 24, NULL); LineTo(hdcMem, pcx + 48, pcy + 24);
                }
                SelectObject(hdcMem, hOldP);
                DeleteObject(hFlashPen);
            }

            // Draw Shockwaves
            for (int i = 0; i < MAX_SHOCKWAVES; i++) {
                if (shockwaves[i].life > 0) {
                    HPEN hSwPen = CreatePen(PS_SOLID, 2, shockwaves[i].color);
                    HPEN hOldP = (HPEN)SelectObject(hdcMem, hSwPen);
                    HBRUSH hOldB = (HBRUSH)SelectObject(hdcMem, GetStockObject(NULL_BRUSH));
                    int sr = (int)shockwaves[i].r;
                    Ellipse(hdcMem, (int)shockwaves[i].x - sr, (int)shockwaves[i].y - sr, (int)shockwaves[i].x + sr, (int)shockwaves[i].y + sr);
                    SelectObject(hdcMem, hOldB);
                    SelectObject(hdcMem, hOldP);
                    DeleteObject(hSwPen);
                }
            }

            // Draw Alien Entities
            for (int i = 0; i < alienCount; i++) {
                int acx = aliens[i].x * TILE_SIZE + TILE_SIZE / 2;
                int acy = aliens[i].y * TILE_SIZE + TILE_SIZE / 2 + UI_HEIGHT;
                int isStun = (aliens[i].state == 2);
                int isChase = (aliens[i].state == 1);
                int legAnim = (ticks / 100) % 3;

                // 6 Undulating Tendril Legs
                COLORREF legCol = isStun ? RGB(50, 130, 255) : isChase ? RGB(255, 20, 60) : RGB(170, 0, 170);
                HPEN hLegPen = CreatePen(PS_SOLID, 2, legCol);
                HPEN hOldP = (HPEN)SelectObject(hdcMem, hLegPen);

                MoveToEx(hdcMem, acx - 5, acy - 3, NULL); LineTo(hdcMem, acx - 9 - legAnim, acy - 6);
                MoveToEx(hdcMem, acx - 6, acy, NULL); LineTo(hdcMem, acx - 10, acy + legAnim - 1);
                MoveToEx(hdcMem, acx - 5, acy + 3, NULL); LineTo(hdcMem, acx - 9 - legAnim, acy + 6);

                MoveToEx(hdcMem, acx + 5, acy - 3, NULL); LineTo(hdcMem, acx + 9 + legAnim, acy - 6);
                MoveToEx(hdcMem, acx + 6, acy, NULL); LineTo(hdcMem, acx + 10, acy - legAnim + 1);
                MoveToEx(hdcMem, acx + 5, acy + 3, NULL); LineTo(hdcMem, acx + 9 + legAnim, acy + 6);

                SelectObject(hdcMem, hOldP);
                DeleteObject(hLegPen);

                // Biomechanical Carapace Body
                COLORREF shellCol = isStun ? RGB(16, 28, 60) : isChase ? RGB(59, 8, 20) : RGB(34, 8, 43);
                COLORREF plateCol = isStun ? RGB(31, 61, 122) : isChase ? RGB(102, 20, 38) : RGB(77, 20, 92);
                HBRUSH hShell = CreateSolidBrush(shellCol);
                HBRUSH hPlate = CreateSolidBrush(plateCol);
                HBRUSH hOldB = (HBRUSH)SelectObject(hdcMem, hShell);
                hOldP = (HPEN)SelectObject(hdcMem, GetStockObject(NULL_PEN));

                Ellipse(hdcMem, acx - 6, acy - 6, acx + 6, acy + 6);
                SelectObject(hdcMem, hPlate);
                Ellipse(hdcMem, acx - 4, acy - 5, acx + 4, acy - 1);
                Ellipse(hdcMem, acx - 4, acy + 1, acx + 4, acy + 5);

                // Optic Core Eye
                COLORREF eyeCol = isStun ? RGB(0, 255, 255) : isChase ? RGB(255, 30, 0) : RGB(220, 0, 255);
                HBRUSH hEye = CreateSolidBrush(eyeCol);
                SelectObject(hdcMem, hEye);
                Ellipse(hdcMem, acx - 3, acy - 3, acx + 3, acy + 3);

                if (isChase) {
                    // Slit pupil
                    HBRUSH hPupil = (HBRUSH)GetStockObject(BLACK_BRUSH);
                    RECT pupilR = {acx - 1, acy - 2, acx + 1, acy + 2};
                    FillRect(hdcMem, &pupilR, hPupil);
                    // Snapping red mandibles
                    HPEN hMandPen = CreatePen(PS_SOLID, 1, RGB(255, 50, 80));
                    SelectObject(hdcMem, hMandPen);
                    MoveToEx(hdcMem, acx - 3, acy + 4, NULL); LineTo(hdcMem, acx, acy + 7 + legAnim);
                    MoveToEx(hdcMem, acx + 3, acy + 4, NULL); LineTo(hdcMem, acx, acy + 7 + legAnim);
                    DeleteObject(hMandPen);
                } else if (!isStun) {
                    HBRUSH hPupil = (HBRUSH)GetStockObject(WHITE_BRUSH);
                    RECT pupilR = {acx - 1, acy - 1, acx + 1, acy + 1};
                    FillRect(hdcMem, &pupilR, hPupil);
                }

                SelectObject(hdcMem, hOldB);
                SelectObject(hdcMem, hOldP);
                DeleteObject(hShell);
                DeleteObject(hPlate);
                DeleteObject(hEye);
            }

            // Draw Player Spacesuit Explorer Sprite
            if (!isDead) {
                int pcx = playerX * TILE_SIZE + TILE_SIZE / 2;
                int pcy = playerY * TILE_SIZE + TILE_SIZE / 2 + UI_HEIGHT;
                int step = (ticks / 150) % 2;

                HBRUSH hSuit = CreateSolidBrush(RGB(208, 219, 229));
                HBRUSH hShoulder = CreateSolidBrush(RGB(140, 163, 184));
                HBRUSH hBackpack = CreateSolidBrush(RGB(28, 40, 51));
                HBRUSH hHelmet = CreateSolidBrush(RGB(234, 242, 248));
                HBRUSH hVisor = CreateSolidBrush(RGB(0, 220, 255));
                HBRUSH hBoots = CreateSolidBrush(RGB(72, 90, 106));
                HBRUSH hOxyLed = CreateSolidBrush((oxygen <= 20) ? RGB(255, 50, 50) : RGB(0, 255, 100));
                HBRUSH hBatLed = CreateSolidBrush((battery <= 20) ? RGB(255, 50, 50) : RGB(0, 255, 255));
                HBRUSH hWhite = (HBRUSH)GetStockObject(WHITE_BRUSH);

                HBRUSH hOldB = (HBRUSH)SelectObject(hdcMem, hSuit);
                HPEN hOldP = (HPEN)SelectObject(hdcMem, GetStockObject(NULL_PEN));

                if (playerDir == 0) { // UP (Back View)
                    // Torso
                    RECT torso = {pcx - 5, pcy - 3, pcx + 5, pcy + 5};
                    FillRect(hdcMem, &torso, hSuit);
                    RECT sh1 = {pcx - 6, pcy - 4, pcx - 3, pcy};
                    RECT sh2 = {pcx + 3, pcy - 4, pcx + 6, pcy};
                    FillRect(hdcMem, &sh1, hShoulder);
                    FillRect(hdcMem, &sh2, hShoulder);
                    // Backpack
                    RECT pack = {pcx - 4, pcy - 2, pcx + 4, pcy + 5};
                    FillRect(hdcMem, &pack, hBackpack);
                    RECT oxR = {pcx - 3, pcy, pcx - 1, pcy + 4};
                    RECT btR = {pcx + 1, pcy, pcx + 3, pcy + 4};
                    FillRect(hdcMem, &oxR, hOxyLed);
                    FillRect(hdcMem, &btR, hBatLed);
                    // Helmet
                    SelectObject(hdcMem, hHelmet);
                    Ellipse(hdcMem, pcx - 4, pcy - 8, pcx + 4, pcy);
                    // Boots
                    RECT b1 = {pcx - 4, pcy + 5 + step, pcx - 1, pcy + 8 + step};
                    RECT b2 = {pcx + 1, pcy + 5 - step + 1, pcx + 4, pcy + 8 - step + 1};
                    FillRect(hdcMem, &b1, hBoots);
                    FillRect(hdcMem, &b2, hBoots);

                } else if (playerDir == 1) { // DOWN (Front View)
                    // Torso
                    RECT torso = {pcx - 5, pcy - 2, pcx + 5, pcy + 6};
                    FillRect(hdcMem, &torso, hSuit);
                    RECT sh1 = {pcx - 6, pcy - 3, pcx - 3, pcy + 1};
                    RECT sh2 = {pcx + 3, pcy - 3, pcx + 6, pcy + 1};
                    FillRect(hdcMem, &sh1, hShoulder);
                    FillRect(hdcMem, &sh2, hShoulder);
                    // Chest Rig
                    RECT chest = {pcx - 3, pcy + 1, pcx + 3, pcy + 5};
                    FillRect(hdcMem, &chest, hBackpack);
                    RECT cLed1 = {pcx - 2, pcy + 2, pcx, pcy + 4};
                    FillRect(hdcMem, &cLed1, hOxyLed);
                    // Helmet & Visor
                    SelectObject(hdcMem, hHelmet);
                    Ellipse(hdcMem, pcx - 4, pcy - 8, pcx + 4, pcy);
                    SelectObject(hdcMem, hVisor);
                    Ellipse(hdcMem, pcx - 3, pcy - 6, pcx + 3, pcy - 1);
                    RECT glint = {pcx - 2, pcy - 5, pcx, pcy - 4};
                    FillRect(hdcMem, &glint, hWhite);
                    // Boots
                    RECT b1 = {pcx - 4, pcy + 6 + step, pcx - 1, pcy + 9 + step};
                    RECT b2 = {pcx + 1, pcy + 6 - step + 1, pcx + 4, pcy + 9 - step + 1};
                    FillRect(hdcMem, &b1, hBoots);
                    FillRect(hdcMem, &b2, hBoots);

                } else if (playerDir == 2) { // LEFT (Side View)
                    RECT torso = {pcx - 4, pcy - 2, pcx + 4, pcy + 6};
                    FillRect(hdcMem, &torso, hSuit);
                    RECT pack = {pcx + 2, pcy - 1, pcx + 5, pcy + 5};
                    FillRect(hdcMem, &pack, hBackpack);
                    RECT oxR = {pcx + 3, pcy, pcx + 4, pcy + 3};
                    FillRect(hdcMem, &oxR, hOxyLed);
                    SelectObject(hdcMem, hHelmet);
                    Ellipse(hdcMem, pcx - 5, pcy - 8, pcx + 3, pcy);
                    SelectObject(hdcMem, hVisor);
                    RECT visR = {pcx - 5, pcy - 5, pcx - 2, pcy - 2};
                    FillRect(hdcMem, &visR, hVisor);
                    RECT b1 = {pcx - 4 + step, pcy + 6, pcx - 1 + step, pcy + 9};
                    RECT b2 = {pcx + step, pcy + 6, pcx + 3 + step, pcy + 9};
                    FillRect(hdcMem, &b1, hBoots);
                    FillRect(hdcMem, &b2, hBoots);

                } else { // RIGHT (Side View)
                    RECT torso = {pcx - 4, pcy - 2, pcx + 4, pcy + 6};
                    FillRect(hdcMem, &torso, hSuit);
                    RECT pack = {pcx - 5, pcy - 1, pcx - 2, pcy + 5};
                    FillRect(hdcMem, &pack, hBackpack);
                    RECT oxR = {pcx - 4, pcy, pcx - 3, pcy + 3};
                    FillRect(hdcMem, &oxR, hOxyLed);
                    SelectObject(hdcMem, hHelmet);
                    Ellipse(hdcMem, pcx - 3, pcy - 8, pcx + 5, pcy);
                    SelectObject(hdcMem, hVisor);
                    RECT visR = {pcx + 2, pcy - 5, pcx + 5, pcy - 2};
                    FillRect(hdcMem, &visR, hVisor);
                    RECT b1 = {pcx - 3 + step, pcy + 6, pcx + step, pcy + 9};
                    RECT b2 = {pcx + 1 - step, pcy + 6, pcx + 4 - step, pcy + 9};
                    FillRect(hdcMem, &b1, hBoots);
                    FillRect(hdcMem, &b2, hBoots);
                }

                SelectObject(hdcMem, hOldB);
                SelectObject(hdcMem, hOldP);
                DeleteObject(hSuit);
                DeleteObject(hShoulder);
                DeleteObject(hBackpack);
                DeleteObject(hHelmet);
                DeleteObject(hVisor);
                DeleteObject(hBoots);
                DeleteObject(hOxyLed);
                DeleteObject(hBatLed);
            }
            
            // Draw particles
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (particles[i].life > 0) {
                    HBRUSH pBrush = CreateSolidBrush(particles[i].color);
                    RECT pr = { (int)particles[i].x - 1, (int)particles[i].y - 1, (int)particles[i].x + 2, (int)particles[i].y + 2 };
                    FillRect(hdcMem, &pr, pBrush);
                    DeleteObject(pBrush);
                }
            }
            
            // Draw UI Text
            SetBkMode(hdcMem, TRANSPARENT);
            char uiText[256];
            wsprintf(uiText, "DECK: %d    OXYGEN: %d%%    BATTERY: %d%%", deck, (int)oxygen, (int)battery);
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

            if (wonGame) {
                SetTextColor(hdcMem, RGB(0, 255, 0));
                TextOut(hdcMem, WINDOW_WIDTH / 2 - lstrlen(winEnding) * 4, WINDOW_HEIGHT / 2, winEnding, lstrlen(winEnding));
            } else if (isDead) {
                SetTextColor(hdcMem, RGB(255, 0, 0));
                char* deadMsg = "SIGNAL LOST";
                TextOut(hdcMem, WINDOW_WIDTH / 2 - 40, WINDOW_HEIGHT / 2, deadMsg, lstrlen(deadMsg));
            }

            if (showHelp) {
                HBRUSH hHelpBrush = CreateSolidBrush(RGB(10, 10, 10));
                RECT helpRect = {40, 40, WINDOW_WIDTH - 40, WINDOW_HEIGHT - 40};
                FillRect(hdcMem, &helpRect, hHelpBrush);
                
                HPEN hHelpPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
                HPEN hOldPenHelp = (HPEN)SelectObject(hdcMem, hHelpPen);
                HBRUSH hOldBrushHelp = (HBRUSH)SelectObject(hdcMem, GetStockObject(NULL_BRUSH));
                Rectangle(hdcMem, helpRect.left, helpRect.top, helpRect.right, helpRect.bottom);
                SelectObject(hdcMem, hOldBrushHelp);
                SelectObject(hdcMem, hOldPenHelp);
                DeleteObject(hHelpPen);
                DeleteObject(hHelpBrush);

                SetTextColor(hdcMem, RGB(0, 255, 0));
                SetBkMode(hdcMem, TRANSPARENT);
                int y = 50;
                TextOut(hdcMem, WINDOW_WIDTH / 2 - 110, y, "SURVIVAL GUIDE (Press H to close)", 33);
                y += 40;
                TextOut(hdcMem, 60, y, "Controls & How to Play:", 23); y += 20;
                TextOut(hdcMem, 70, y, "WASD / Arrows: Move", 19); y += 20;
                TextOut(hdcMem, 70, y, "Space: Use EMP (Stuns nearby aliens)", 36); y += 20;
                TextOut(hdcMem, 70, y, "H: Toggle Help", 14); y += 30;
                TextOut(hdcMem, 70, y, "Survive, find elevator. Watch Oxygen & Battery.", 47); y += 40;
                
                TextOut(hdcMem, 60, y, "Lore Index:", 11); y += 20;
                TextOut(hdcMem, 70, y, "Trapped on a derelict station. The crew was", 43); y += 20;
                TextOut(hdcMem, 70, y, "experimenting on aliens... it didn't go well.", 45); y += 40;
                
                TextOut(hdcMem, 60, y, "Enemy Bestiary:", 15); y += 20;
                TextOut(hdcMem, 70, y, "Entities: Sensitive to noise & movement.", 40); y += 20;
                TextOut(hdcMem, 70, y, "They glow magenta. Hide in Lockers to avoid.", 44); y += 20;
                TextOut(hdcMem, 70, y, "Stunned Entities: Glow blue. Safe temporarily.", 46);
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

            // Screen shake offset
            int shakeX = 0, shakeY = 0;
            if (screenShake > 0) {
                shakeX = (rand() % (screenShake * 2 + 1)) - screenShake;
                shakeY = (rand() % (screenShake * 2 + 1)) - screenShake;
            }

            // Copy to screen with screen shake
            BitBlt(hdc, shakeX, shakeY, WINDOW_WIDTH, WINDOW_HEIGHT, hdcMem, 0, 0, SRCCOPY);
            
            // Cleanup
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_KEYDOWN: {
            if (wParam == 'H') {
                showHelp = !showHelp;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (isDead || wonGame || showHelp) return 0;
            int newX = playerX;
            int newY = playerY;
            
            switch (wParam) {
                case VK_UP:
                case 'W':
                    newY--; playerDir = 0;
                    break;
                case VK_DOWN:
                case 'S':
                    newY++; playerDir = 1;
                    break;
                case VK_LEFT:
                case 'A':
                    newX--; playerDir = 2;
                    break;
                case VK_RIGHT:
                case 'D':
                    newX++; playerDir = 3;
                    break;
                case VK_SPACE:
                    if (emps > 0) {
                        emps--;
                        lstrcpy(sysMsg, "EMP DEPLOYED. ALIENS STUNNED.");
                        msgTimer = 60;
                        float epx = playerX * TILE_SIZE + TILE_SIZE / 2.0f;
                        float epy = playerY * TILE_SIZE + TILE_SIZE / 2.0f + UI_HEIGHT;
                        SpawnParticles(epx, epy, RGB(0, 255, 255), 40);
                        AddShockwave(epx, epy, RGB(0, 255, 255), 140.0f);
                        screenShake = 6;
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
                } else if (target == 13) {
                    deck++;
                    hasRedKey = 0; hasGreenKey = 0; hasBlueKey = 0;
                    oxygen = 100.0f;
                    battery = 100.0f;
                    GenerateMap();
                    wsprintf(sysMsg, "ELEVATOR TO DECK %d. STATS RESTORED.", deck);
                    msgTimer = 60;
                    InvalidateRect(hwnd, NULL, FALSE);
                } else if (target == 14) {
                    wonGame = 1;
                    if (selfDestructActive) {
                        if (totalTime < 300) {
                            lstrcpy(winEnding, "S-RANK: STATION DESTROYED, SPEEDRUN ESCAPE!");
                        } else {
                            lstrcpy(winEnding, "HERO: STATION DESTROYED, YOU ESCAPED.");
                        }
                    } else {
                        if (totalTime < 300) {
                            lstrcpy(winEnding, "COWARD (FAST): YOU ESCAPED, BUT THE THREAT REMAINS.");
                        } else {
                            lstrcpy(winEnding, "COWARD: YOU ESCAPED, BUT THE THREAT REMAINS.");
                        }
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                } else if (target == 15) {
                    if (!selfDestructActive) {
                        selfDestructActive = 1;
                        selfDestructTimer = 45;
                        map[newY][newX] = 10;
                        lstrcpy(sysMsg, "SELF DESTRUCT ACTIVATED. 45 SECONDS TO ESCAPE!");
                        msgTimer = 60;
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
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
                            PlaySoundEffect(3);
                            SpawnParticles(playerX * TILE_SIZE + TILE_SIZE / 2.0f, playerY * TILE_SIZE + TILE_SIZE / 2.0f + UI_HEIGHT, RGB(255, 0, 0), 50);
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
