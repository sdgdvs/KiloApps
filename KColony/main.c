#include <windows.h>
#include <stdio.h>
#include <math.h>

#define GRID_W 20

DWORD WINAPI SoundThread(LPVOID lpParam) {
    int type = (int)(LONG_PTR)lpParam;
    if (type == 1) { // Build
        Beep(300, 50); Beep(400, 50);
    } else if (type == 2) { // Alarm
        Beep(600, 150); Beep(800, 150); Beep(600, 150);
    } else if (type == 3) { // Laser
        for (int i = 800; i >= 200; i -= 150) Beep(i, 20);
    } else if (type == 4) { // Rumble/Wind
        Beep(100, 500);
    }
    return 0;
}
void PlayGameSound(int type) {
    CreateThread(NULL, 0, SoundThread, (LPVOID)(LONG_PTR)type, 0, NULL);
}

#define GRID_H 20
#define CELL_SIZE 20
#define OFFSET_X 20
#define OFFSET_Y 80

int animFrame = 0;

typedef struct {
    float x, y;
    float vx, vy;
    int life;
    COLORREF color;
} Particle;
Particle particles[500];
int particleCount = 0;

void SpawnParticles(float x, float y, COLORREF color, int count) {
    for (int i = 0; i < count && particleCount < 500; i++) {
        particles[particleCount].x = x;
        particles[particleCount].y = y;
        particles[particleCount].vx = ((rand() % 100) - 50) / 10.0f;
        particles[particleCount].vy = ((rand() % 100) - 50) / 10.0f;
        particles[particleCount].life = 10 + (rand() % 10);
        particles[particleCount].color = color;
        particleCount++;
    }
}

extern int shakeTicks;

void SpawnExplosion(float x, float y) {
    SpawnParticles(x, y, RGB(255, 255, 255), 30);
    SpawnParticles(x, y, RGB(255, 136, 0), 40);
    SpawnParticles(x, y, RGB(255, 50, 0), 30);
    SpawnParticles(x, y, RGB(100, 100, 100), 40);
    shakeTicks = 15;
}

typedef struct {
    float x, y;
    float tx, ty;
    float vx, vy;
    COLORREF color;
} Projectile;
Projectile projectiles[100];
int projCount = 0;

void FireProjectile(float x1, float y1, float x2, float y2, COLORREF color) {
    if (projCount < 100) {
        projectiles[projCount].x = x1;
        projectiles[projCount].y = y1;
        projectiles[projCount].tx = x2;
        projectiles[projCount].ty = y2;
        float dx = x2 - x1;
        float dy = y2 - y1;
        float len = sqrt(dx*dx + dy*dy);
        if (len > 0.001f) {
            projectiles[projCount].vx = (dx / len) * 15.0f;
            projectiles[projCount].vy = (dy / len) * 15.0f;
        } else {
            projectiles[projCount].vx = 0;
            projectiles[projCount].vy = 0;
        }
        projectiles[projCount].color = color;
        projCount++;
    }
}

int dustStormTicks = 0;
int msgTicks = 0;
char msgText[128] = "";
int shakeTicks = 0;

int food = 50;
int power = 50;
int maxPower = 50;
int mat = 50;
int advm = 0;
int pop = 0;
int maxPop = 0;
int happiness = 100;
int popWait = 0;
int tick = 0;
int day = 1;
int isDay = 1;
int sci = 0;
int unlockedHydro = 0, unlockedNuke = 0, unlockedLaser = 0, unlockedFactory = 0;
int selectedType = 0;
int grid[GRID_W * GRID_H] = {0};

HBITMAP hbmTerrain = NULL;
void GenerateTerrain(HDC hdc) {
    if (hbmTerrain) DeleteObject(hbmTerrain);
    HDC hdcMem = CreateCompatibleDC(hdc);
    hbmTerrain = CreateCompatibleBitmap(hdc, GRID_W * CELL_SIZE, GRID_H * CELL_SIZE);
    HBITMAP hbmOld = SelectObject(hdcMem, hbmTerrain);
    
    HBRUSH bg = CreateSolidBrush(RGB(10, 17, 26));
    RECT rc = {0, 0, GRID_W * CELL_SIZE, GRID_H * CELL_SIZE};
    FillRect(hdcMem, &rc, bg);
    DeleteObject(bg);
    
    for(int i=0; i<30; i++) {
        int cx = rand() % (GRID_W * CELL_SIZE);
        int cy = rand() % (GRID_H * CELL_SIZE);
        int r = (rand() % 15) + 5;
        
        HBRUSH br = CreateSolidBrush(RGB(5, 8, 13));
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(15, 25, 35));
        HBRUSH oldB = SelectObject(hdcMem, br);
        HPEN oldP = SelectObject(hdcMem, pen);
        Ellipse(hdcMem, cx - r, cy - r, cx + r, cy + r);
        
        SelectObject(hdcMem, oldB); DeleteObject(br);
        br = CreateSolidBrush(RGB(2, 4, 8));
        oldB = SelectObject(hdcMem, br);
        int r2 = r * 0.6;
        Ellipse(hdcMem, cx - (int)(r*0.2) - r2, cy - (int)(r*0.2) - r2, cx - (int)(r*0.2) + r2, cy - (int)(r*0.2) + r2);
        
        SelectObject(hdcMem, oldB); DeleteObject(br);
        SelectObject(hdcMem, oldP); DeleteObject(pen);
    }
    
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
    HPEN oldP = SelectObject(hdcMem, pen);
    for(int i=0; i<10; i++) {
        int x = rand() % (GRID_W * CELL_SIZE);
        int y = rand() % (GRID_H * CELL_SIZE);
        MoveToEx(hdcMem, x, y, NULL);
        int len = (rand() % 50) + 50;
        for(int j=0; j<len; j+=10) {
            x += (rand() % 40) - 20;
            y += (rand() % 40) - 20;
            LineTo(hdcMem, x, y);
        }
    }
    SelectObject(hdcMem, oldP); DeleteObject(pen);
    
    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);
}


int gameState = 0; // 0 = menu, 1 = playing, 2 = help
int prevState = 0;
int gameMode = 0; // 0=endless, 1=sandbox, 2=100-day, 3=resource rush

typedef struct {
    int x, y, hp;
} Alien;
Alien aliens[100];
int alienCount = 0;

typedef struct {
    RECT rc;
    int id; // 0-11 for build, 101-103 for research
    char label[32];
    int isSelected;
} Button;
Button buttons[20];
int btnCount = 0;

void DrawMenu(HDC hdc, HFONT hFont, RECT rc) {
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 255, 255));
    SelectObject(hdc, hFont);
    
    DrawText(hdc, "KCOLONY SCENARIOS", -1, &(RECT){0, 80, rc.right, 120}, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DrawText(hdc, "Press 'H' at any time for Help", -1, &(RECT){0, 120, rc.right, 150}, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    const char* titles[] = { "1. ENDLESS SURVIVAL", "2. SANDBOX MODE (UNLIMITED)", "3. 100-DAY SURVIVAL", "4. RESOURCE RUSH (1000M, 100A by D50)", "5. [H] HELP & MANUAL" };
    for (int i=0; i<5; i++) {
        RECT bRc = {rc.right/2 - 200, 160 + i*60, rc.right/2 + 200, 200 + i*60};
        HBRUSH br = CreateSolidBrush(RGB(17,17,34));
        FillRect(hdc, &bRc, br);
        DeleteObject(br);
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
        HPEN oldP = SelectObject(hdc, pen);
        MoveToEx(hdc, bRc.left, bRc.top, NULL); LineTo(hdc, bRc.right, bRc.top); LineTo(hdc, bRc.right, bRc.bottom); LineTo(hdc, bRc.left, bRc.bottom); LineTo(hdc, bRc.left, bRc.top);
        SelectObject(hdc, oldP); DeleteObject(pen);
        DrawText(hdc, titles[i], -1, &bRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

void DrawHelp(HDC hdc, HFONT hFont, RECT rc) {
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 255, 255));
    SelectObject(hdc, hFont);
    
    RECT textRc = {20, 20, rc.right - 20, rc.bottom - 60};
    const char* helpText = 
        "KCOLONY ADMINISTRATOR'S MANUAL\n\n"
        "HOW TO PLAY:\n"
        "Manage resources (Food, Power, Mat, AdvM). Keep colonists happy and expand.\n"
        "Research tech to unlock advanced structures.\n\n"
        "STRUCTURES:\n"
        "Solar (S): +4 Pwr (Day) | Farm (F): +5 Food, -5 Pwr | Mine (M): +3 Mat, -10 Pwr\n"
        "Hab (H): +5 Max Pop, -5 Pwr | Battery (B): +50 Max Pwr | Lab (L): +2 Sci, -5 Pwr\n"
        "Nuke (N): +20 Pwr | Hydro (Y): +15 Food, -10 Pwr | Factory (C): 2 Mat -> 1 AdvM\n"
        "Wall (W): Defense | Turret (T): Range 3, -5 Pwr | Laser (D): Range 5, -20 Pwr\n\n"
        "TECH TREE:\n"
        "Hydroponics (50S): Efficient food. | Factory (75S): Advanced Materials.\n"
        "Nuclear (100S): 24/7 Power. | Laser (150S): Long-range defense.\n\n"
        "DISASTERS:\n"
        "Meteors/Breakdowns: Keep Mat to repair. | Dust Storms: Use Batteries/Nukes.\n"
        "Alien Attacks: Build Walls and Turrets.\n";
        
    DrawText(hdc, helpText, -1, &textRc, DT_LEFT | DT_TOP);
    
    RECT btnRc = {rc.right / 2 - 50, rc.bottom - 50, rc.right / 2 + 50, rc.bottom - 20};
    HBRUSH br = CreateSolidBrush(RGB(17,17,34));
    FillRect(hdc, &btnRc, br);
    DeleteObject(br);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
    HPEN oldP = SelectObject(hdc, pen);
    MoveToEx(hdc, btnRc.left, btnRc.top, NULL); LineTo(hdc, btnRc.right, btnRc.top); LineTo(hdc, btnRc.right, btnRc.bottom); LineTo(hdc, btnRc.left, btnRc.bottom); LineTo(hdc, btnRc.left, btnRc.top);
    SelectObject(hdc, oldP); DeleteObject(pen);
    DrawText(hdc, "BACK", -1, &btnRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void StartGame(HWND hwnd, int mode) {
    gameState = 1;
    gameMode = mode;
    food = 50; power = 50; maxPower = 50; mat = 50; advm = 0;
    pop = 0; maxPop = 0; happiness = 100; sci = 0; popWait = 0;
    day = 1; isDay = 1; tick = 0;
    unlockedHydro = 0; unlockedNuke = 0; unlockedLaser = 0; unlockedFactory = 0;
    alienCount = 0;
    memset(grid, 0, sizeof(grid));
    
    if (mode == 1) { 
        food = 9999; power = 9999; maxPower = 9999; mat = 9999; advm = 9999; sci = 9999;
        unlockedHydro = 1; unlockedNuke = 1; unlockedLaser = 1; unlockedFactory = 1;
    }
    InvalidateRect(hwnd, NULL, FALSE);
}

void DrawGrid(HDC hdc, HFONT hFont) {
    SelectObject(hdc, hFont);
    
    int effOffsetX = OFFSET_X;
    int effOffsetY = OFFSET_Y;
    if (shakeTicks > 0) {
        effOffsetX += (rand() % 11) - 5;
        effOffsetY += (rand() % 11) - 5;
    }
    
    if (hbmTerrain) {
        HDC hdcTerrain = CreateCompatibleDC(hdc);
        HBITMAP hbmOld = SelectObject(hdcTerrain, hbmTerrain);
        BitBlt(hdc, effOffsetX, effOffsetY, GRID_W * CELL_SIZE, GRID_H * CELL_SIZE, hdcTerrain, 0, 0, SRCCOPY);
        SelectObject(hdcTerrain, hbmOld);
        DeleteDC(hdcTerrain);
    }
    
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            RECT rc = { effOffsetX + x * CELL_SIZE, effOffsetY + y * CELL_SIZE, effOffsetX + (x + 1) * CELL_SIZE, effOffsetY + (y + 1) * CELL_SIZE };
            
            int t = grid[y * GRID_W + x];
            int seed = y * GRID_W + x;
            
            COLORREF bgCol = RGB(10, 17, 26);
            COLORREF borderCol = RGB(0, 51, 51);
            COLORREF textCol = RGB(0, 255, 255);
            
            if (t > 20) { bgCol = RGB(51, 0, 0); borderCol = RGB(255, 0, 0); textCol = RGB(255, 0, 0); }
            else if (t == 1) { bgCol = RGB(51, 51, 0); borderCol = RGB(255, 255, 0); textCol = RGB(255, 255, 0); }
            else if (t == 2) { bgCol = RGB(0, 51, 0); borderCol = RGB(0, 255, 0); textCol = RGB(0, 255, 0); }
            else if (t == 3) { bgCol = RGB(51, 0, 51); borderCol = RGB(255, 0, 255); textCol = RGB(255, 0, 255); }
            else if (t == 4) { bgCol = RGB(0, 51, 51); borderCol = RGB(0, 255, 255); textCol = RGB(0, 255, 255); }
            else if (t == 5) { bgCol = RGB(51, 17, 0); borderCol = RGB(255, 136, 0); textCol = RGB(255, 136, 0); }
            else if (t == 6) { bgCol = RGB(34, 0, 51); borderCol = RGB(170, 0, 255); textCol = RGB(170, 0, 255); }
            else if (t == 7) { bgCol = RGB(0, 51, 34); borderCol = RGB(0, 255, 170); textCol = RGB(0, 255, 170); }
            else if (t == 8) { bgCol = RGB(34, 51, 0); borderCol = RGB(170, 255, 0); textCol = RGB(170, 255, 0); }
            else if (t == 9) { bgCol = RGB(51, 0, 0); borderCol = RGB(255, 0, 0); textCol = RGB(255, 0, 0); }
            else if (t == 10) { bgCol = RGB(34, 34, 34); borderCol = RGB(136, 136, 136); textCol = RGB(136, 136, 136); }
            else if (t == 11) { bgCol = RGB(51, 34, 0); borderCol = RGB(255, 170, 0); textCol = RGB(255, 170, 0); }
            else if (t == 12) { bgCol = RGB(68, 68, 68); borderCol = RGB(204, 204, 204); textCol = RGB(204, 204, 204); }
            
            if (t > 0) {
                if (t == 9 || t == 11 || t == 7) {
                    int tod = tick % 10;
                    if (tod < 5) {
                        int sx = 0, sy = 0;
                        if (tod == 0) { sx = 12; sy = 2; }
                        else if (tod == 1) { sx = 6; sy = 4; }
                        else if (tod == 2) { sx = 0; sy = 6; }
                        else if (tod == 3) { sx = -6; sy = 4; }
                        else if (tod == 4) { sx = -12; sy = 2; }
                        HBRUSH sb = CreateSolidBrush(RGB(5, 8, 13));
                        HBRUSH oldSb = SelectObject(hdc, sb);
                        HPEN sp = CreatePen(PS_NULL, 0, 0);
                        HPEN oldSp = SelectObject(hdc, sp);
                        POINT shadowPts[4] = {{rc.left+4, rc.bottom-4}, {rc.right-4, rc.bottom-4}, {rc.right-4+sx, rc.top+sy}, {rc.left+4+sx, rc.top+sy}};
                        Polygon(hdc, shadowPts, 4);
                        SelectObject(hdc, oldSp); DeleteObject(sp);
                        SelectObject(hdc, oldSb); DeleteObject(sb);
                    }
                }
                HBRUSH brush = CreateSolidBrush(bgCol);
                FillRect(hdc, &rc, brush);
                DeleteObject(brush);
            }
            
            HPEN pen = CreatePen(PS_SOLID, 1, t > 0 ? borderCol : RGB(0, 51, 51));
            HPEN oldPen = SelectObject(hdc, pen);
            MoveToEx(hdc, rc.left, rc.top, NULL);
            LineTo(hdc, rc.right, rc.top);
            LineTo(hdc, rc.right, rc.bottom);
            LineTo(hdc, rc.left, rc.bottom);
            LineTo(hdc, rc.left, rc.top);
            SelectObject(hdc, oldPen);
            DeleteObject(pen);
            
            if (t > 0) {
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, textCol);
                if (t > 20) {
                    HPEN p2 = CreatePen(PS_SOLID, 2, textCol); HPEN oldP = SelectObject(hdc, p2);
                    MoveToEx(hdc, rc.left+4, rc.top+4, NULL); LineTo(hdc, rc.right-4, rc.bottom-4);
                    MoveToEx(hdc, rc.right-4, rc.top+4, NULL); LineTo(hdc, rc.left+4, rc.bottom-4);
                    SelectObject(hdc, oldP); DeleteObject(p2);
                } else if (t == 1) { // Solar
                    HPEN p2 = CreatePen(PS_SOLID, 1, textCol); HPEN oldP = SelectObject(hdc, p2);
                    Rectangle(hdc, rc.left+2, rc.top+4, rc.right-2, rc.bottom-4);
                    MoveToEx(hdc, rc.left+6, rc.top+4, NULL); LineTo(hdc, rc.left+6, rc.bottom-4);
                    MoveToEx(hdc, rc.left+10, rc.top+4, NULL); LineTo(hdc, rc.left+10, rc.bottom-4);
                    MoveToEx(hdc, rc.left+14, rc.top+4, NULL); LineTo(hdc, rc.left+14, rc.bottom-4);
                    SelectObject(hdc, oldP); DeleteObject(p2);
                } else if (t == 2) { // Farm
                    HPEN p2 = CreatePen(PS_SOLID, 1, textCol); HPEN oldP = SelectObject(hdc, p2);
                    HBRUSH b2 = CreateSolidBrush(textCol); HBRUSH oldB = SelectObject(hdc, b2);
                    Rectangle(hdc, rc.left+2, rc.top+10, rc.right-2, rc.bottom-2);
                    SelectObject(hdc, oldB); DeleteObject(b2);
                    int h = (seed % 3) * 2;
                    MoveToEx(hdc, rc.left+4, rc.top+10, NULL); LineTo(hdc, rc.left+6, rc.top+6 - h);
                    MoveToEx(hdc, rc.left+10, rc.top+10, NULL); LineTo(hdc, rc.left+10, rc.top+4 - h);
                    MoveToEx(hdc, rc.left+16, rc.top+10, NULL); LineTo(hdc, rc.left+14, rc.top+6 - h);
                    SelectObject(hdc, oldP); DeleteObject(p2);
                } else if (t == 3) { // Mine
                    HPEN p2 = CreatePen(PS_SOLID, 2, textCol); HPEN oldP = SelectObject(hdc, p2);
                    MoveToEx(hdc, rc.left+4, rc.bottom-2, NULL); LineTo(hdc, rc.left+10, rc.top+4); LineTo(hdc, rc.right-4, rc.bottom-2);
                    HBRUSH b2 = CreateSolidBrush(textCol); HBRUSH oldB = SelectObject(hdc, b2);
                    Ellipse(hdc, rc.left+8, rc.top+2, rc.left+12, rc.top+6);
                    Rectangle(hdc, rc.left+8, rc.top+10, rc.left+12, rc.bottom-2);
                    if (seed % 2 == 0) Ellipse(hdc, rc.left+2, rc.bottom-6, rc.left+6, rc.bottom-2);
                    if (seed % 3 == 0) Ellipse(hdc, rc.right-6, rc.bottom-5, rc.right-2, rc.bottom-1);
                    SelectObject(hdc, oldB); DeleteObject(b2);
                    SelectObject(hdc, oldP); DeleteObject(p2);
                } else if (t == 4) { // Hab
                    HPEN p2 = CreatePen(PS_SOLID, 1, textCol); HPEN oldP = SelectObject(hdc, p2);
                    HBRUSH b2 = CreateSolidBrush(bgCol); HBRUSH oldB = SelectObject(hdc, b2);
                    Chord(hdc, rc.left+2, rc.top+2, rc.right-2, rc.bottom+8, rc.right-2, rc.bottom-2, rc.left+2, rc.bottom-2);
                    SelectObject(hdc, oldB); DeleteObject(b2);
                    b2 = CreateSolidBrush(textCol); oldB = SelectObject(hdc, b2);
                    Rectangle(hdc, rc.left+8, rc.bottom-6, rc.left+12, rc.bottom-2);
                    SelectObject(hdc, oldB); DeleteObject(b2);
                    SelectObject(hdc, oldP); DeleteObject(p2);
                } else if (t == 5) { // Battery
                    HPEN p2 = CreatePen(PS_SOLID, 1, textCol); HPEN oldP = SelectObject(hdc, p2);
                    Rectangle(hdc, rc.left+4, rc.top+4, rc.right-4, rc.bottom-2);
                    HBRUSH b2 = CreateSolidBrush(textCol); HBRUSH oldB = SelectObject(hdc, b2);
                    Rectangle(hdc, rc.left+8, rc.top+2, rc.left+12, rc.top+4);
                    
                    int pulse = (animFrame % 20) < 10 ? 1 : 0;
                    if (pulse) {
                        Rectangle(hdc, rc.left+6, rc.top+10, rc.right-6, rc.bottom-4);
                    }
                    SelectObject(hdc, oldB); DeleteObject(b2);
                    SelectObject(hdc, oldP); DeleteObject(p2);
                } else if (t == 6) { // Lab
                    HPEN p2 = CreatePen(PS_SOLID, 1, textCol); HPEN oldP = SelectObject(hdc, p2);
                    POINT pts[4] = {{rc.left+8, rc.top+2}, {rc.right-8, rc.top+2}, {rc.right-2, rc.bottom-2}, {rc.left+2, rc.bottom-2}};
                    Polygon(hdc, pts, 4);
                    HBRUSH b2 = CreateSolidBrush(textCol); HBRUSH oldB = SelectObject(hdc, b2);
                    
                    int spin = (animFrame % 10) / 2;
                    int w = 2 + spin;
                    Ellipse(hdc, rc.left+10-w, rc.top+12, rc.left+10+w, rc.top+16);
                    SelectObject(hdc, oldB); DeleteObject(b2);
                    SelectObject(hdc, oldP); DeleteObject(p2);
                } else if (t == 7) { // Nuke
                    HPEN p2 = CreatePen(PS_SOLID, 1, textCol); HPEN oldP = SelectObject(hdc, p2);
                    POINT pts[4] = {{rc.left+6, rc.bottom-2}, {rc.left+8, rc.top+4}, {rc.right-8, rc.top+4}, {rc.right-6, rc.bottom-2}};
                    Polygon(hdc, pts, 4);
                    HBRUSH b2 = CreateSolidBrush(textCol); HBRUSH oldB = SelectObject(hdc, b2);
                    
                    int pulse = (animFrame % 16) < 8 ? 1 : 0;
                    int r = pulse ? 7 : 5;
                    Ellipse(hdc, rc.left+10-r, rc.top+11-r, rc.left+10+r, rc.top+11+r);
                    
                    SelectObject(hdc, oldB); DeleteObject(b2);
                    SelectObject(hdc, oldP); DeleteObject(p2);
                } else if (t == 8) { // Hydro
                    HPEN p2 = CreatePen(PS_SOLID, 1, textCol); HPEN oldP = SelectObject(hdc, p2);
                    Rectangle(hdc, rc.left+2, rc.bottom-6, rc.right-2, rc.bottom-2);
                    Arc(hdc, rc.left+2, rc.top+2, rc.right-2, rc.bottom-2, rc.right-2, rc.bottom-6, rc.left+2, rc.bottom-6);
                    SelectObject(hdc, oldP); DeleteObject(p2);
                } else if (t == 9) { // Laser
                    HPEN p2 = CreatePen(PS_SOLID, 2, textCol); HPEN oldP = SelectObject(hdc, p2);
                    Rectangle(hdc, rc.left+4, rc.bottom-6, rc.right-4, rc.bottom-2);
                    HBRUSH b2 = CreateSolidBrush(textCol); HBRUSH oldB = SelectObject(hdc, b2);
                    Ellipse(hdc, rc.left+6, rc.top+6, rc.right-6, rc.bottom-6);
                    SelectObject(hdc, oldB); DeleteObject(b2);
                    MoveToEx(hdc, rc.left+10, rc.top+8, NULL); LineTo(hdc, rc.left+10, rc.top+2);
                    SelectObject(hdc, oldP); DeleteObject(p2);
                } else if (t == 10) { // Wall
                    HPEN p2 = CreatePen(PS_SOLID, 1, textCol); HPEN oldP = SelectObject(hdc, p2);
                    Rectangle(hdc, rc.left+2, rc.top+2, rc.right-2, rc.bottom-2);
                    MoveToEx(hdc, rc.left+2, rc.top+8, NULL); LineTo(hdc, rc.right-2, rc.top+8);
                    MoveToEx(hdc, rc.left+2, rc.top+14, NULL); LineTo(hdc, rc.right-2, rc.top+14);
                    MoveToEx(hdc, rc.left+8, rc.top+2, NULL); LineTo(hdc, rc.left+8, rc.top+8);
                    MoveToEx(hdc, rc.left+14, rc.top+8, NULL); LineTo(hdc, rc.left+14, rc.top+14);
                    MoveToEx(hdc, rc.left+8, rc.top+14, NULL); LineTo(hdc, rc.left+8, rc.bottom-2);
                    SelectObject(hdc, oldP); DeleteObject(p2);
                } else if (t == 11) { // Turret
                    HPEN p2 = CreatePen(PS_SOLID, 1, textCol); HPEN oldP = SelectObject(hdc, p2);
                    POINT pts[4] = {{rc.left+2, rc.bottom-2}, {rc.right-2, rc.bottom-2}, {rc.right-6, rc.top+8}, {rc.left+6, rc.top+8}};
                    Polygon(hdc, pts, 4);
                    HBRUSH b2 = CreateSolidBrush(textCol); HBRUSH oldB = SelectObject(hdc, b2);
                    Rectangle(hdc, rc.left+8, rc.top+2, rc.left+12, rc.top+8);
                    SelectObject(hdc, oldB); DeleteObject(b2);
                    SelectObject(hdc, oldP); DeleteObject(p2);
                } else if (t == 12) { // Factory
                    HPEN p2 = CreatePen(PS_SOLID, 1, textCol); HPEN oldP = SelectObject(hdc, p2);
                    POINT pts[7] = {{rc.left+2, rc.bottom-2}, {rc.right-2, rc.bottom-2}, {rc.right-2, rc.top+6}, {rc.right-6, rc.top+10}, {rc.right-6, rc.top+4}, {rc.right-10, rc.top+8}, {rc.left+2, rc.top+8}};
                    Polygon(hdc, pts, 7);
                    HBRUSH b2 = CreateSolidBrush(textCol); HBRUSH oldB = SelectObject(hdc, b2);
                    Rectangle(hdc, rc.right-6, rc.top+2, rc.right-4, rc.top+6);
                    SelectObject(hdc, oldB); DeleteObject(b2);
                    SelectObject(hdc, oldP); DeleteObject(p2);
                }
                
                if (power < 15 && t <= 20) {
                    if ((animFrame % 10) < 5) {
                        HBRUSH wb = CreateSolidBrush(RGB(255, 0, 0));
                        HBRUSH oldWb = SelectObject(hdc, wb);
                        Ellipse(hdc, rc.left+2, rc.top+2, rc.left+6, rc.top+6);
                        SelectObject(hdc, oldWb);
                        DeleteObject(wb);
                    }
                }
            }
        }
    }
    
    if (dustStormTicks > 0) {
        for (int i = 0; i < 150; i++) {
            int x = (animFrame * 15 + i * 17) % (GRID_W * CELL_SIZE);
            int y = (i * 29) % (GRID_H * CELL_SIZE);
            HPEN p = CreatePen(PS_SOLID, (i % 3) + 1, RGB(200, 120, 0));
            HPEN oldP = SelectObject(hdc, p);
            MoveToEx(hdc, effOffsetX + x, effOffsetY + y, NULL);
            LineTo(hdc, effOffsetX + x + 20 + (i % 10), effOffsetY + y);
            SelectObject(hdc, oldP); DeleteObject(p);
        }
    } else {
        for (int i = 0; i < 40; i++) {
            int x = (animFrame * 5 + i * 31) % (GRID_W * CELL_SIZE);
            int y = (i * 47) % (GRID_H * CELL_SIZE);
            HPEN p = CreatePen(PS_SOLID, 1, RGB(30, 50, 70));
            HPEN oldP = SelectObject(hdc, p);
            MoveToEx(hdc, effOffsetX + x, effOffsetY + y, NULL);
            LineTo(hdc, effOffsetX + x + 10, effOffsetY + y);
            SelectObject(hdc, oldP); DeleteObject(p);
        }
    }
    
    for (int i = 0; i < alienCount; i++) {
        RECT rc = { effOffsetX + aliens[i].x * CELL_SIZE, effOffsetY + aliens[i].y * CELL_SIZE, effOffsetX + (aliens[i].x + 1) * CELL_SIZE, effOffsetY + (aliens[i].y + 1) * CELL_SIZE };
        COLORREF alienCol = RGB(255, 0, 255);
        HPEN p2 = CreatePen(PS_SOLID, 1, alienCol); HPEN oldP = SelectObject(hdc, p2);
        HBRUSH b2 = CreateSolidBrush(RGB(80, 0, 80)); HBRUSH oldB = SelectObject(hdc, b2);
        Ellipse(hdc, rc.left+2, rc.top+2, rc.right-2, rc.bottom-2);
        SelectObject(hdc, oldB); DeleteObject(b2);
        
        b2 = CreateSolidBrush(RGB(0,0,0)); oldB = SelectObject(hdc, b2);
        Ellipse(hdc, rc.left+5, rc.top+7, rc.left+9, rc.top+11);
        Ellipse(hdc, rc.right-9, rc.top+7, rc.right-5, rc.top+11);
        SelectObject(hdc, oldB); DeleteObject(b2);
        SelectObject(hdc, oldP); DeleteObject(p2);
    }
    
    for (int i = 0; i < particleCount; i++) {
        HBRUSH b = CreateSolidBrush(particles[i].color);
        HPEN p = CreatePen(PS_SOLID, 1, particles[i].color);
        HBRUSH oldB = SelectObject(hdc, b);
        HPEN oldP = SelectObject(hdc, p);
        int px = (int)particles[i].x + (effOffsetX - OFFSET_X);
        int py = (int)particles[i].y + (effOffsetY - OFFSET_Y);
        Ellipse(hdc, px - 2, py - 2, px + 2, py + 2);
        SelectObject(hdc, oldP); SelectObject(hdc, oldB);
        DeleteObject(p); DeleteObject(b);
    }
    for (int i = 0; i < projCount; i++) {
        HBRUSH b = CreateSolidBrush(projectiles[i].color);
        HPEN p = CreatePen(PS_SOLID, 1, projectiles[i].color);
        HBRUSH oldB = SelectObject(hdc, b);
        HPEN oldP = SelectObject(hdc, p);
        int px = (int)projectiles[i].x + (effOffsetX - OFFSET_X);
        int py = (int)projectiles[i].y + (effOffsetY - OFFSET_Y);
        Ellipse(hdc, px - 3, py - 3, px + 3, py + 3);
        SelectObject(hdc, oldP); SelectObject(hdc, oldB);
        DeleteObject(p); DeleteObject(b);
    }
}

void DrawUI(HDC hdc, HFONT hFont) {
    char buf[128];
    SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 255, 255));
    sprintf(buf, "DAY %d (%s) F:%d P:%d/%d M:%d A:%d POP:%d/%d HAP:%d%% SCI:%d", day, isDay ? "DAY" : "NIGHT", food, power, maxPower, mat, advm, pop, maxPop, happiness, sci);
    
    RECT rcHeader = { 20, 15, 620, 45 };
    HBRUSH hdrBrush = CreateSolidBrush(RGB(17, 17, 34));
    FillRect(hdc, &rcHeader, hdrBrush);
    DeleteObject(hdrBrush);
    HPEN hdrPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 255));
    HPEN oldPen = SelectObject(hdc, hdrPen);
    MoveToEx(hdc, rcHeader.left, rcHeader.top, NULL);
    LineTo(hdc, rcHeader.right, rcHeader.top);
    LineTo(hdc, rcHeader.right, rcHeader.bottom);
    LineTo(hdc, rcHeader.left, rcHeader.bottom);
    LineTo(hdc, rcHeader.left, rcHeader.top);
    SelectObject(hdc, oldPen);
    DeleteObject(hdrPen);
    
    DrawText(hdc, buf, -1, &rcHeader, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    if (msgTicks > 0) {
        RECT rcMsg = { 20, 55, 620, 75 };
        SetTextColor(hdc, RGB(255, 0, 0));
        DrawText(hdc, msgText, -1, &rcMsg, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    int sidebarX = OFFSET_X + GRID_W * CELL_SIZE + 20;
    const char* labels[] = { "[-1] REPAIR", "[0] INSPECT", "[1] SOLAR", "[2] FARM", "[3] MINE", "[4] HAB", "[5] BATT", "[6] LAB", "[7] NUKE", "[8] HYDRO", "[9] LASER", "[10] WALL", "[11] TURRET", "[12] FACTORY" };
    
    btnCount = 0;
    int btnY = OFFSET_Y;
    for (int i = -1; i < 13; i++) {
        if (i == 7 && !unlockedNuke) continue;
        if (i == 8 && !unlockedHydro) continue;
        if (i == 9 && !unlockedLaser) continue;
        if (i == 12 && !unlockedFactory) continue;
        
        buttons[btnCount].rc = (RECT){ sidebarX, btnY, sidebarX + 180, btnY + 25 };
        buttons[btnCount].id = i;
        strcpy(buttons[btnCount].label, labels[i + 1]);
        buttons[btnCount].isSelected = (i == selectedType);
        btnCount++;
        btnY += 30;
    }
    int btnY2 = OFFSET_Y;
    int sidebarX2 = sidebarX + 190;
    if (!unlockedHydro) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 150, btnY2 + 25 };
        buttons[btnCount].id = 101;
        strcpy(buttons[btnCount].label, "RES HYDRO(50S)");
        buttons[btnCount].isSelected = 0;
        btnCount++;
        btnY2 += 30;
    }
    if (!unlockedNuke) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 150, btnY2 + 25 };
        buttons[btnCount].id = 102;
        strcpy(buttons[btnCount].label, "RES NUKE(100S)");
        buttons[btnCount].isSelected = 0;
        btnCount++;
        btnY2 += 30;
    }
    if (!unlockedLaser) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 150, btnY2 + 25 };
        buttons[btnCount].id = 103;
        strcpy(buttons[btnCount].label, "RES LASER(150S)");
        buttons[btnCount].isSelected = 0;
        btnCount++;
        btnY2 += 30;
    }
    if (!unlockedFactory) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 150, btnY2 + 25 };
        buttons[btnCount].id = 104;
        strcpy(buttons[btnCount].label, "RES FACTORY(75S)");
        buttons[btnCount].isSelected = 0;
        btnCount++;
        btnY2 += 30;
    }

    btnY2 += 10;
    buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 150, btnY2 + 25 };
    buttons[btnCount].id = 200;
    strcpy(buttons[btnCount].label, "EXPEDITION");
    buttons[btnCount].isSelected = 0;
    btnCount++;

    btnY2 += 30;
    buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 150, btnY2 + 25 };
    buttons[btnCount].id = 300;
    strcpy(buttons[btnCount].label, "[H] HELP / MANUAL");
    buttons[btnCount].isSelected = 0;
    btnCount++;

    for (int i = 0; i < btnCount; i++) {
        COLORREF btnBg = buttons[i].isSelected ? RGB(0, 51, 51) : RGB(17, 17, 34);
        if (buttons[i].id >= 100 && buttons[i].id < 200) btnBg = RGB(34, 17, 51); // purple hue for research
        else if (buttons[i].id == 200) btnBg = RGB(51, 34, 0); // orange hue for expedition
        
        COLORREF btnBorder = buttons[i].isSelected ? RGB(255, 255, 255) : RGB(0, 255, 255);
        if (buttons[i].id >= 100 && buttons[i].id < 200) btnBorder = RGB(170, 0, 255);
        else if (buttons[i].id == 200) btnBorder = RGB(255, 170, 0);
        
        COLORREF btnText = buttons[i].isSelected ? RGB(255, 255, 255) : RGB(0, 255, 255);
        if (buttons[i].id >= 100 && buttons[i].id < 200) btnText = RGB(170, 0, 255);
        else if (buttons[i].id == 200) btnText = RGB(255, 170, 0);

        HBRUSH brush = CreateSolidBrush(btnBg);
        FillRect(hdc, &buttons[i].rc, brush);
        DeleteObject(brush);
        
        HPEN pen = CreatePen(PS_SOLID, buttons[i].isSelected ? 2 : 1, btnBorder);
        HPEN oldP = SelectObject(hdc, pen);
        MoveToEx(hdc, buttons[i].rc.left, buttons[i].rc.top, NULL);
        LineTo(hdc, buttons[i].rc.right, buttons[i].rc.top);
        LineTo(hdc, buttons[i].rc.right, buttons[i].rc.bottom);
        LineTo(hdc, buttons[i].rc.left, buttons[i].rc.bottom);
        LineTo(hdc, buttons[i].rc.left, buttons[i].rc.top);
        SelectObject(hdc, oldP);
        DeleteObject(pen);
        
        SetTextColor(hdc, btnText);
        DrawText(hdc, buttons[i].label, -1, &buttons[i].rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            srand(GetTickCount());
            HDC hdc = GetDC(hwnd);
            GenerateTerrain(hdc);
            ReleaseDC(hwnd, hdc);
            SetTimer(hwnd, 1, 2000, NULL);
            SetTimer(hwnd, 2, 50, NULL);
            break;
        }
        case WM_KEYDOWN:
            if (wParam == 'H') {
                if (gameState == 2) {
                    gameState = prevState;
                } else {
                    prevState = gameState;
                    gameState = 2;
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        case WM_TIMER: {
            if (wParam == 2) {
                animFrame++;
                if (shakeTicks > 0) shakeTicks--;
                for (int i = 0; i < particleCount; i++) {
                    particles[i].x += particles[i].vx;
                    particles[i].y += particles[i].vy;
                    particles[i].life--;
                    if (particles[i].life <= 0) {
                        particles[i] = particles[--particleCount];
                        i--;
                    }
                }
                for (int i = 0; i < GRID_W * GRID_H; i++) {
                    if (grid[i] > 20 && (rand() % 100) < 15) {
                        int tx = i % GRID_W, ty = i / GRID_W;
                        SpawnParticles(OFFSET_X + tx * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + ty * CELL_SIZE + CELL_SIZE/2, RGB(100, 100, 100), 1);
                    }
                }
                for (int i = 0; i < projCount; i++) {
                    projectiles[i].x += projectiles[i].vx;
                    projectiles[i].y += projectiles[i].vy;
                    float dx = projectiles[i].tx - projectiles[i].x;
                    float dy = projectiles[i].ty - projectiles[i].y;
                    if (dx*dx + dy*dy < 200.0f) {
                        SpawnParticles(projectiles[i].tx, projectiles[i].ty, projectiles[i].color, 5);
                        projectiles[i] = projectiles[--projCount];
                        i--;
                    }
                }
                if (gameState == 1) InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (gameState != 1) break;
            tick++;
            if (tick % 10 == 0) { day++; isDay = 1; }
            else if (tick % 10 == 5) { isDay = 0; }
            
            // Random Events
            if (rand() % 100 < 5) { // 5% chance per tick
                int r = rand() % 100;
                int targets[GRID_W * GRID_H];
                int targetCount = 0;
                for (int i = 0; i < GRID_W * GRID_H; i++) {
                    if (grid[i] > 0 && grid[i] <= 11) targets[targetCount++] = i;
                }
                
                if (r < 33 && targetCount > 0) {
                    int hits = targetCount < 3 ? targetCount : 3;
                    for (int i = 0; i < hits; i++) {
                        int idx = targets[rand() % targetCount];
                        if (grid[idx] > 0 && grid[idx] <= 11) {
                            grid[idx] += 20;
                            int tx = idx % GRID_W, ty = idx / GRID_W;
                            SpawnExplosion(OFFSET_X + tx * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + ty * CELL_SIZE + CELL_SIZE/2);
                        }
                    }
                    strcpy(msgText, "METEOR SHOWER! Structures damaged.");
                    msgTicks = 5;
                    PlayGameSound(2);
                } else if (r < 66) {
                    dustStormTicks = 10;
                    strcpy(msgText, "DUST STORM! Solar power reduced.");
                    msgTicks = 10;
                    PlayGameSound(4);
                } else if (targetCount > 0) {
                    int idx = targets[rand() % targetCount];
                    if (grid[idx] > 0 && grid[idx] <= 11) {
                        grid[idx] += 20;
                        int tx = idx % GRID_W, ty = idx / GRID_W;
                        SpawnParticles(OFFSET_X + tx * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + ty * CELL_SIZE + CELL_SIZE/2, RGB(255,136,0), 15);
                    }
                    strcpy(msgText, "EQUIPMENT BREAKDOWN!");
                    msgTicks = 5;
                    PlayGameSound(2);
                }
            }
            
            if (dustStormTicks > 0) dustStormTicks--;
            if (msgTicks > 0) msgTicks--;

            for (int i = 0; i < GRID_W * GRID_H; i++) {
                int type = grid[i] > 20 ? 0 : grid[i];
                if (type == 9 || type == 11) {
                    int range = type == 9 ? 5 : 3;
                    int tx = i % GRID_W, ty = i / GRID_W;
                    for (int a = 0; a < alienCount; a++) {
                        int dist = abs(tx - aliens[a].x) + abs(ty - aliens[a].y);
                        if (dist <= range && power > 0) {
                            PlayGameSound(3);
                            COLORREF col = (type == 9) ? RGB(255, 0, 0) : RGB(255, 170, 0);
                            FireProjectile(OFFSET_X + tx * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + ty * CELL_SIZE + CELL_SIZE/2,
                                           OFFSET_X + aliens[a].x * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + aliens[a].y * CELL_SIZE + CELL_SIZE/2, col);
                            
                            aliens[a].hp--;
                            if (aliens[a].hp <= 0) {
                                SpawnParticles(OFFSET_X + aliens[a].x * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + aliens[a].y * CELL_SIZE + CELL_SIZE/2, RGB(255,0,255), 15);
                                aliens[a] = aliens[--alienCount];
                                a--;
                            }
                            break;
                        }
                    }
                }
            }

            if (tick % 2 == 0) {
                for (int a = 0; a < alienCount; a++) {
                    int targetIdx = -1, minDist = 999;
                    for (int i = 0; i < GRID_W * GRID_H; i++) {
                        if (grid[i] > 0 && grid[i] <= 20) {
                            int tx = i % GRID_W, ty = i / GRID_W;
                            int dist = abs(tx - aliens[a].x) + abs(ty - aliens[a].y);
                            if (dist < minDist) { minDist = dist; targetIdx = i; }
                        }
                    }
                    if (targetIdx != -1) {
                        if (minDist <= 1) {
                            if (grid[targetIdx] <= 20) {
                                grid[targetIdx] += 20;
                                strcpy(msgText, "ALIEN ATTACK! Structure damaged.");
                                msgTicks = 5;
                                PlayGameSound(2);
                                int tx = targetIdx % GRID_W, ty = targetIdx / GRID_W;
                                SpawnExplosion(OFFSET_X + tx * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + ty * CELL_SIZE + CELL_SIZE/2);
                            }
                            aliens[a] = aliens[--alienCount];
                            a--;
                        } else {
                            int tx = targetIdx % GRID_W, ty = targetIdx / GRID_W;
                            if (rand() % 2 == 0 && aliens[a].x != tx) aliens[a].x += (tx > aliens[a].x ? 1 : -1);
                            else if (aliens[a].y != ty) aliens[a].y += (ty > aliens[a].y ? 1 : -1);
                            else aliens[a].x += (tx > aliens[a].x ? 1 : -1);
                        }
                    } else {
                        aliens[a].x += (rand() % 3) - 1;
                        aliens[a].y += (rand() % 3) - 1;
                    }
                    if (aliens[a].x < 0) aliens[a].x = 0;
                    if (aliens[a].x >= GRID_W) aliens[a].x = GRID_W - 1;
                    if (aliens[a].y < 0) aliens[a].y = 0;
                    if (aliens[a].y >= GRID_H) aliens[a].y = GRID_H - 1;
                }
            }

            int spawnChance = 10 + (day / 20);
            if (day > 2 && (rand() % 100) < spawnChance && alienCount < 100) {
                int spawnEdge = rand() % 4;
                int ax = 0, ay = 0;
                if (spawnEdge == 0) { ax = rand() % GRID_W; ay = 0; }
                else if (spawnEdge == 1) { ax = rand() % GRID_W; ay = GRID_H - 1; }
                else if (spawnEdge == 2) { ax = 0; ay = rand() % GRID_H; }
                else { ax = GRID_W - 1; ay = rand() % GRID_H; }
                aliens[alienCount].x = ax;
                aliens[alienCount].y = ay;
                aliens[alienCount].hp = 3 + (day / 10);
                alienCount++;
                strcpy(msgText, "ALIEN SPOTTED!");
                msgTicks = 5;
                PlayGameSound(2);
            }
            
            int pwrProd = 0, farmCount = 0, mineCount = 0, habCount = 0, batCount = 0;
            int labCount = 0, nukeCount = 0, hydroCount = 0, laserCount = 0, turretCount = 0, factoryCount = 0;
            for(int i=0; i<GRID_W*GRID_H; i++) {
                if(grid[i]==1 && isDay && dustStormTicks <= 0) pwrProd += 4;
                if(grid[i]==2) farmCount += 1;
                if(grid[i]==3) mineCount += 1;
                if(grid[i]==4) habCount += 1;
                if(grid[i]==5) batCount += 1;
                if(grid[i]==6) labCount += 1;
                if(grid[i]==7) nukeCount += 1;
                if(grid[i]==8) hydroCount += 1;
                if(grid[i]==9) laserCount += 1;
                if(grid[i]==11) turretCount += 1;
                if(grid[i]==12) factoryCount += 1;
            }
            maxPop = habCount * 5;
            maxPower = 50 + batCount * 50;
            pwrProd += nukeCount * 20;
            
            int pwrCons = farmCount + mineCount + habCount + labCount + hydroCount * 2 + laserCount * 5 + turretCount * 2 + factoryCount * 10;
            power += pwrProd - pwrCons;
            
            float pwrEff = 1.0f;
            if (power < 0) { power = 0; pwrEff = 0.5f; }
            if (power > maxPower) power = maxPower;

            float eff = (happiness / 100.0f) * pwrEff;
            int foodProd = (int)((farmCount * 5 + hydroCount * 15) * eff);
            int matProd = (int)(mineCount * 3 * eff);
            int sciProd = (int)(labCount * 2 * eff);

            food += foodProd;
            mat += matProd;
            sci += sciProd;

            int factoryActive = (int)(factoryCount * eff);
            int matConsForAdv = factoryActive * 2;
            int advmProd = 0;
            if (mat >= matConsForAdv) {
                mat -= matConsForAdv;
                advmProd = factoryActive;
            } else {
                advmProd = mat / 2;
                mat -= advmProd * 2;
            }
            advm += advmProd;

            int foodCons = pop;
            if (food >= foodCons) {
                food -= foodCons;
                happiness += 5;
                if (happiness > 100) happiness = 100;
            } else {
                food = 0;
                happiness -= 10;
                if (happiness < 0) happiness = 0;
            }

            if (pop < maxPop && happiness >= 50) {
                popWait++;
                if (popWait >= 3) {
                    pop++;
                    popWait = 0;
                }
            } else if (pop > maxPop) {
                happiness -= 10;
                if (happiness < 0) happiness = 0;
            }

            if (gameMode == 2 && day >= 100) {
                MessageBox(hwnd, "You survived 100 days! You Win!", "Scenario Complete", MB_OK);
                gameState = 0;
            }
            if (gameMode == 3) {
                if (mat >= 1000 && advm >= 100) {
                    char buf[128];
                    sprintf(buf, "You reached the goal on day %d! You Win!", day);
                    MessageBox(hwnd, buf, "Scenario Complete", MB_OK);
                    gameState = 0;
                } else if (day > 50) {
                    MessageBox(hwnd, "Day 50 reached without meeting quota! Game Over.", "Scenario Failed", MB_OK);
                    gameState = 0;
                }
            }

            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            if (gameState == 0) {
                RECT rc;
                GetClientRect(hwnd, &rc);
                int cx = rc.right / 2;
                if (x >= cx - 200 && x <= cx + 200) {
                    if (y >= 160 && y <= 200) StartGame(hwnd, 0);
                    if (y >= 220 && y <= 260) StartGame(hwnd, 1);
                    if (y >= 280 && y <= 320) StartGame(hwnd, 2);
                    if (y >= 340 && y <= 380) StartGame(hwnd, 3);
                    if (y >= 400 && y <= 440) { prevState = 0; gameState = 2; InvalidateRect(hwnd, NULL, FALSE); }
                }
                return 0;
            }
            
            if (gameState == 2) {
                RECT rc;
                GetClientRect(hwnd, &rc);
                int cx = rc.right / 2;
                if (x >= cx - 50 && x <= cx + 50 && y >= rc.bottom - 50 && y <= rc.bottom - 20) {
                    gameState = prevState;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                return 0;
            }
            
            int sidebarX = OFFSET_X + GRID_W * CELL_SIZE + 20;
            if (x >= sidebarX) {
                for (int i = 0; i < btnCount; i++) {
                    if (x >= buttons[i].rc.left && x <= buttons[i].rc.right && y >= buttons[i].rc.top && y <= buttons[i].rc.bottom) {
                        int id = buttons[i].id;
                        if (id < 100) {
                            selectedType = id;
                        } else if (id == 101 && sci >= 50 && !unlockedHydro) {
                            sci -= 50; unlockedHydro = 1;
                        } else if (id == 102 && sci >= 100 && !unlockedNuke) {
                            sci -= 100; unlockedNuke = 1;
                        } else if (id == 103 && sci >= 150 && !unlockedLaser) {
                            sci -= 150; unlockedLaser = 1;
                        } else if (id == 104 && sci >= 75 && !unlockedFactory) {
                            sci -= 75; unlockedFactory = 1;
                        } else if (id == 200) {
                            if (pop >= 2 && mat >= 30 && power >= 30) {
                                mat -= 30;
                                power -= 30;
                                int r = rand() % 100;
                                if (r < 25) {
                                    mat += 100;
                                    food += 100;
                                    MessageBox(hwnd, "Rover found a massive supply cache!\n(+100 Mat, +100 Food)", "Expedition Report", MB_OK | MB_ICONINFORMATION);
                                } else if (r < 50) {
                                    sci += 150;
                                    MessageBox(hwnd, "Expedition discovered advanced alien data!\n(+150 Sci)", "Expedition Report", MB_OK | MB_ICONINFORMATION);
                                } else if (r < 70) {
                                    advm += 20;
                                    MessageBox(hwnd, "Rover recovered rare components!\n(+20 AdvM)", "Expedition Report", MB_OK | MB_ICONINFORMATION);
                                } else if (r < 85) {
                                    MessageBox(hwnd, "Rover broke down in a dust storm. Expedition returned safely but empty-handed.", "Expedition Report", MB_OK | MB_ICONWARNING);
                                } else {
                                    pop -= 2;
                                    happiness -= 20;
                                    if (happiness < 0) happiness = 0;
                                    MessageBox(hwnd, "Expedition was ambushed by alien mutants!\n(-2 Pop, -20% Happiness)", "Expedition Report", MB_OK | MB_ICONERROR);
                                }
                            } else {
                                MessageBox(hwnd, "Not enough resources! Need 2 Pop, 30 Mat, 30 Pwr.", "Expedition Report", MB_OK | MB_ICONWARNING);
                            }
                        } else if (id == 300) {
                            prevState = 1;
                            gameState = 2;
                        }
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }
            }
            
            if (x >= OFFSET_X && x < OFFSET_X + GRID_W * CELL_SIZE && y >= OFFSET_Y && y < OFFSET_Y + GRID_H * CELL_SIZE) {
                int gx = (x - OFFSET_X) / CELL_SIZE;
                int gy = (y - OFFSET_Y) / CELL_SIZE;
                int idx = gy * GRID_W + gx;
                
                if (selectedType == -1 && grid[idx] > 20) {
                    if (mat >= 5) {
                        mat -= 5;
                        grid[idx] -= 20;
                        PlayGameSound(1);
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                } else if (selectedType > 0 && grid[idx] == 0) {
                    int costMat = 0, costPwr = 0, costAdv = 0;
                    if (selectedType == 1) costMat = 10;
                    else if (selectedType == 2) { costMat = 10; costPwr = 5; }
                    else if (selectedType == 3) costPwr = 10;
                    else if (selectedType == 4) { costMat = 15; costPwr = 5; }
                    else if (selectedType == 5) costMat = 20;
                    else if (selectedType == 6) { costMat = 20; costPwr = 5; }
                    else if (selectedType == 7) { costMat = 20; costAdv = 10; }
                    else if (selectedType == 8) { costMat = 20; costPwr = 10; }
                    else if (selectedType == 9) { costMat = 10; costAdv = 10; costPwr = 20; }
                    else if (selectedType == 10) { costMat = 5; }
                    else if (selectedType == 11) { costMat = 15; costPwr = 5; }
                    else if (selectedType == 12) { costMat = 30; costPwr = 10; }
                    
                    if (mat >= costMat && power >= costPwr && advm >= costAdv) {
                        mat -= costMat;
                        power -= costPwr;
                        advm -= costAdv;
                        grid[idx] = selectedType;
                        PlayGameSound(1);
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                }
            }
            break;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HDC hdcMem = CreateCompatibleDC(hdc);
            RECT rc;
            GetClientRect(hwnd, &rc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            HBITMAP hbmOld = SelectObject(hdcMem, hbmMem);
            
            HBRUSH bg = CreateSolidBrush(RGB(5, 10, 15));
            FillRect(hdcMem, &rc, bg);
            DeleteObject(bg);
            
            HFONT hFont = CreateFont(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH, "Courier New");
            
            if (gameState == 0) {
                DrawMenu(hdcMem, hFont, rc);
            } else if (gameState == 2) {
                DrawHelp(hdcMem, hFont, rc);
            } else {
                DrawGrid(hdcMem, hFont);
                DrawUI(hdcMem, hFont);
            }
            
            DeleteObject(hFont);
            
            BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);
            
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    SetProcessDPIAware();
    const char CLASS_NAME[] = "KColonyClass";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    RECT rcW = {0, 0, 850, 650};
    AdjustWindowRect(&rcW, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KColony", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, rcW.right - rcW.left, rcW.bottom - rcW.top,
        NULL, NULL, hInstance, NULL
    );
    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
