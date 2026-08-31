#include <windows.h>
#include <stdio.h>
#include <math.h>

#define GRID_W 20
#define GRID_H 20
#define CELL_SIZE 20
#define OFFSET_X 20
#define OFFSET_Y 85

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
    } else if (type == 5) { // Tech unlock
        Beep(523, 60); Beep(659, 60); Beep(784, 80); Beep(1046, 120);
    } else if (type == 6) { // Shield / Drone
        Beep(900, 40); Beep(1100, 40);
    }
    return 0;
}
void PlayGameSound(int type) {
    CreateThread(NULL, 0, SoundThread, (LPVOID)(LONG_PTR)type, 0, NULL);
}

int animFrame = 0;

typedef struct {
    float x, y;
    float vx, vy;
    int life;
    COLORREF color;
} Particle;
Particle particles[600];
int particleCount = 0;

void SpawnParticles(float x, float y, COLORREF color, int count) {
    for (int i = 0; i < count && particleCount < 600; i++) {
        particles[particleCount].x = x;
        particles[particleCount].y = y;
        particles[particleCount].vx = ((rand() % 100) - 50) / 10.0f;
        particles[particleCount].vy = ((rand() % 100) - 50) / 10.0f;
        particles[particleCount].life = 10 + (rand() % 12);
        particles[particleCount].color = color;
        particleCount++;
    }
}

int shakeTicks = 0;

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
Projectile projectiles[120];
int projCount = 0;

void FireProjectile(float x1, float y1, float x2, float y2, COLORREF color) {
    if (projCount < 120) {
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

int weatherType = 0; // 0=Clear, 1=Dust Storm, 2=Solar Flare, 3=Blizzard, 4=Acid Rain, 5=Seismic Tremor
int weatherTicks = 0;
int dustStormTicks = 0;
int msgTicks = 0;
char msgText[128] = "";

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

// Tech tree unlocks
int unlockedHydro = 0, unlockedNuke = 0, unlockedLaser = 0, unlockedFactory = 0;
int unlockedSolar4 = 0, unlockedXenoArmor = 0, unlockedGeo = 0, unlockedBio = 0, unlockedShield = 0, unlockedDrone = 0;

int selectedType = 0;
int grid[GRID_W * GRID_H] = {0};

// Planetary Scenarios & Mutators
int planetType = 0; // 0=Mars Prime, 1=Cryo Tundra, 2=Volcanic Inferno, 3=Acid Swamp
const char* planetNames[] = { "MARS PRIME", "CRYO TUNDRA (GLIESE 667)", "VOLCANIC INFERNO (KEPLER 10B)", "ACID SWAMP (PROXIMA B)" };
int activeAnomaly = 0; // 0=None, 1=Crystal Monolith, 2=Nanite Swarm, 3=Subterranean Geode, 4=Ionized Aurora
const char* anomalyNames[] = { "NONE", "CRYSTAL MONOLITH (+50% SCI)", "NANITE SWARM (+EFF)", "SUBTERRANEAN GEODE (ADVM)", "IONIZED AURORA (+50 PWR)" };

HBITMAP hbmTerrain = NULL;
void GenerateTerrain(HDC hdc) {
    if (hbmTerrain) DeleteObject(hbmTerrain);
    HDC hdcMem = CreateCompatibleDC(hdc);
    hbmTerrain = CreateCompatibleBitmap(hdc, GRID_W * CELL_SIZE, GRID_H * CELL_SIZE);
    HBITMAP hbmOld = SelectObject(hdcMem, hbmTerrain);
    
    COLORREF bgColor = RGB(10, 17, 26);
    COLORREF craterColor = RGB(5, 8, 13);
    COLORREF craterRim = RGB(15, 25, 35);
    COLORREF crackColor = RGB(0, 0, 0);

    if (planetType == 1) { // Cryo Tundra
        bgColor = RGB(10, 25, 40);
        craterColor = RGB(15, 35, 55);
        craterRim = RGB(40, 80, 120);
        crackColor = RGB(20, 50, 80);
    } else if (planetType == 2) { // Volcanic Inferno
        bgColor = RGB(28, 12, 8);
        craterColor = RGB(45, 15, 10);
        craterRim = RGB(85, 30, 15);
        crackColor = RGB(120, 40, 0);
    } else if (planetType == 3) { // Acid Swamp
        bgColor = RGB(10, 24, 15);
        craterColor = RGB(5, 15, 8);
        craterRim = RGB(20, 50, 30);
        crackColor = RGB(0, 40, 10);
    }
    
    HBRUSH bg = CreateSolidBrush(bgColor);
    RECT rc = {0, 0, GRID_W * CELL_SIZE, GRID_H * CELL_SIZE};
    FillRect(hdcMem, &rc, bg);
    DeleteObject(bg);
    
    for(int i=0; i<30; i++) {
        int cx = rand() % (GRID_W * CELL_SIZE);
        int cy = rand() % (GRID_H * CELL_SIZE);
        int r = (rand() % 15) + 5;
        
        HBRUSH br = CreateSolidBrush(craterColor);
        HPEN pen = CreatePen(PS_SOLID, 1, craterRim);
        HBRUSH oldB = SelectObject(hdcMem, br);
        HPEN oldP = SelectObject(hdcMem, pen);
        Ellipse(hdcMem, cx - r, cy - r, cx + r, cy + r);
        
        SelectObject(hdcMem, oldB); DeleteObject(br);
        br = CreateSolidBrush(RGB(GetRValue(craterColor)/2, GetGValue(craterColor)/2, GetBValue(craterColor)/2));
        oldB = SelectObject(hdcMem, br);
        int r2 = (int)(r * 0.6);
        Ellipse(hdcMem, cx - (int)(r*0.2) - r2, cy - (int)(r*0.2) - r2, cx - (int)(r*0.2) + r2, cy - (int)(r*0.2) + r2);
        
        SelectObject(hdcMem, oldB); DeleteObject(br);
        SelectObject(hdcMem, oldP); DeleteObject(pen);
    }
    
    HPEN pen = CreatePen(PS_SOLID, 2, crackColor);
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
int gameMode = 0; // 0=Mars, 1=Cryo, 2=Volcanic, 3=Acid, 4=Sandbox, 5=100-day, 6=Resource Rush

typedef struct {
    int x, y, hp;
} Alien;
Alien aliens[100];
int alienCount = 0;

typedef struct {
    RECT rc;
    int id;
    char label[36];
    int isSelected;
} Button;
Button buttons[40];
int btnCount = 0;

// Shield detection helper
int IsShielded(int x, int y) {
    for (int sy = max(0, y - 2); sy <= min(GRID_H - 1, y + 2); sy++) {
        for (int sx = max(0, x - 2); sx <= min(GRID_W - 1, x + 2); sx++) {
            if (grid[sy * GRID_W + sx] == 15) { // Active Shield Pylon
                return 1;
            }
        }
    }
    return 0;
}

void DrawMenu(HDC hdc, HFONT hFont, RECT rc) {
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 255, 255));
    SelectObject(hdc, hFont);
    
    DrawText(hdc, "KCOLONY: PLANETARY EXPEDITIONS & TECH TREE", -1, &(RECT){0, 40, rc.right, 75}, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DrawText(hdc, "Select Planet Biome or Scenario Mode  |  Press 'H' or 'F1' for Manual", -1, &(RECT){0, 75, rc.right, 100}, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    const char* titles[] = { 
        "1. MARS PRIME (STANDARD EXPEDITION)", 
        "2. CRYO TUNDRA (GLIESE 667 - BLIZZARDS)", 
        "3. VOLCANIC INFERNO (KEPLER 10B - HEATWAVES)", 
        "4. ACID SWAMP (PROXIMA B - ALIEN SWARMS)",
        "5. SANDBOX COLONY (UNLIMITED TECH & MATS)", 
        "6. 100-DAY SURVIVAL CHALLENGE", 
        "7. RESOURCE RUSH (1000M, 100A BY D50)", 
        "8. [H] OR [F1] HELP & TECH SPEC SHEET" 
    };
    for (int i=0; i<8; i++) {
        RECT bRc = {rc.right/2 - 240, 110 + i*55, rc.right/2 + 240, 150 + i*55};
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
    
    RECT textRc = {25, 20, rc.right - 25, rc.bottom - 60};
    const char* helpText = 
        "KCOLONY ADMINISTRATOR'S MANUAL & EXPANDED SPEC SHEET\n\n"
        "RESOURCES: Food (Colony upkeep) | Power (System operations) | Mat (Basic building)\n"
        "AdvM (Advanced alloy) | Science (Research unlocks) | Happiness (Efficiency factor)\n\n"
        "CORE STRUCTURES:\n"
        "Solar (S): +4 Pwr (Day) | Farm (F): +5 Food, -5 Pwr | Mine (M): +3 Mat, -10 Pwr\n"
        "Hab (H): +5 Max Pop, -5 Pwr | Battery (B): +50 Max Pwr | Lab (L): +2 Sci, -5 Pwr\n"
        "Nuke (N): +20 Pwr constant | Hydro (Y): +15 Food, -10 Pwr | Factory (C): 2 Mat -> 1 AdvM\n"
        "Wall (W): Deflects aliens | Turret (T): Range 3, -5 Pwr | Laser (D): Range 5, -20 Pwr\n\n"
        "NEW ADVANCED STRUCTURES (LOOP 2):\n"
        "Geothermal (G): +35 Pwr 24/7 immune to storms (+50 Pwr on Cryo). Cost: 35M, 15A.\n"
        "Bio-Dome (V): +25 Food, +8 Pop cap, +2 Happiness/tick up to 120%. Cost: 40M, 20A, 20P.\n"
        "Shield Pylon (E): 3x3 Energy Barrier protecting from meteors & attacks! Cost: 50M, 25A, 30P.\n"
        "Drone Hub (U): Auto-repairs broken grid structures (5M) & buffs mines. Cost: 60M, 30A, 25P.\n\n"
        "PLANETARY BIOMES & EXPEDITIONS:\n"
        "- Mars Prime: Standard balanced climate & dust storms.\n"
        "- Cryo Tundra: Extreme blizzards (-50% Solar, +50% Geothermal).\n"
        "- Volcanic Inferno: Extreme heat & solar flares (+50% Mine output, fast breakdowns).\n"
        "- Acid Swamp: Corrosive acid rain & 2x alien spawns (+50% Food from Hydro/Bio).\n"
        "- Expeditions: Launch Scout Recon, Ruins Excavation, or Hive Incursions for Mutators!\n";
        
    DrawText(hdc, helpText, -1, &textRc, DT_LEFT | DT_TOP);
    
    RECT btnRc = {rc.right / 2 - 60, rc.bottom - 45, rc.right / 2 + 60, rc.bottom - 15};
    HBRUSH br = CreateSolidBrush(RGB(17,17,34));
    FillRect(hdc, &btnRc, br);
    DeleteObject(br);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
    HPEN oldP = SelectObject(hdc, pen);
    MoveToEx(hdc, btnRc.left, btnRc.top, NULL); LineTo(hdc, btnRc.right, btnRc.top); LineTo(hdc, btnRc.right, btnRc.bottom); LineTo(hdc, btnRc.left, btnRc.bottom); LineTo(hdc, btnRc.left, btnRc.top);
    SelectObject(hdc, oldP); DeleteObject(pen);
    DrawText(hdc, "BACK TO GAME", -1, &btnRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void StartGame(HWND hwnd, int mode) {
    gameState = 1;
    gameMode = mode;
    
    if (mode >= 0 && mode <= 3) planetType = mode;
    else planetType = 0;

    food = 50; power = 50; maxPower = 50; mat = 50; advm = 0;
    pop = 0; maxPop = 0; happiness = 100; sci = 0; popWait = 0;
    day = 1; isDay = 1; tick = 0;
    
    unlockedHydro = 0; unlockedNuke = 0; unlockedLaser = 0; unlockedFactory = 0;
    unlockedSolar4 = 0; unlockedXenoArmor = 0; unlockedGeo = 0; unlockedBio = 0; unlockedShield = 0; unlockedDrone = 0;
    activeAnomaly = 0; weatherType = 0; weatherTicks = 0; dustStormTicks = 0;

    alienCount = 0;
    memset(grid, 0, sizeof(grid));
    
    if (mode == 4) { // Sandbox
        food = 9999; power = 9999; maxPower = 9999; mat = 9999; advm = 9999; sci = 9999;
        unlockedHydro = 1; unlockedNuke = 1; unlockedLaser = 1; unlockedFactory = 1;
        unlockedSolar4 = 1; unlockedXenoArmor = 1; unlockedGeo = 1; unlockedBio = 1; unlockedShield = 1; unlockedDrone = 1;
    }
    
    HDC hdc = GetDC(hwnd);
    GenerateTerrain(hdc);
    ReleaseDC(hwnd, hdc);

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
    
    // Draw Shield Fields
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            if (grid[y * GRID_W + x] == 15) { // Shield Pylon
                int pulse = (animFrame % 16);
                HPEN sp = CreatePen(PS_DOT, 1, RGB(0, 180 + pulse * 4, 255));
                HPEN oldSp = SelectObject(hdc, sp);
                HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
                HBRUSH oldSb = SelectObject(hdc, nullBr);
                
                int minGx = max(0, x - 2), maxGx = min(GRID_W - 1, x + 2);
                int minGy = max(0, y - 2), maxGy = min(GRID_H - 1, y + 2);
                Rectangle(hdc, effOffsetX + minGx * CELL_SIZE, effOffsetY + minGy * CELL_SIZE, 
                               effOffsetX + (maxGx + 1) * CELL_SIZE, effOffsetY + (maxGy + 1) * CELL_SIZE);
                
                SelectObject(hdc, oldSb);
                SelectObject(hdc, oldSp);
                DeleteObject(sp);
            }
        }
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
            else if (t == 13) { bgCol = RGB(51, 15, 0); borderCol = RGB(255, 80, 0); textCol = RGB(255, 100, 0); } // Geo
            else if (t == 14) { bgCol = RGB(0, 45, 30); borderCol = RGB(0, 255, 180); textCol = RGB(50, 255, 180); } // Bio
            else if (t == 15) { bgCol = RGB(0, 30, 60); borderCol = RGB(0, 180, 255); textCol = RGB(0, 200, 255); } // Shield
            else if (t == 16) { bgCol = RGB(30, 45, 10); borderCol = RGB(200, 255, 0); textCol = RGB(220, 255, 50); } // Drone
            
            if (t > 0) {
                if (t == 9 || t == 11 || t == 7 || t == 13 || t == 15) {
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
                    if (pulse) Rectangle(hdc, rc.left+6, rc.top+10, rc.right-6, rc.bottom-4);
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
                } else if (t == 13) { // Geothermal
                    HPEN p2 = CreatePen(PS_SOLID, 2, RGB(255, 100, 0)); HPEN oldP = SelectObject(hdc, p2);
                    Rectangle(hdc, rc.left+3, rc.bottom-7, rc.right-3, rc.bottom-2);
                    MoveToEx(hdc, rc.left+7, rc.bottom-7, NULL); LineTo(hdc, rc.left+10, rc.top+3); LineTo(hdc, rc.right-7, rc.bottom-7);
                    HBRUSH b2 = CreateSolidBrush(RGB(255, 140, 0)); HBRUSH oldB = SelectObject(hdc, b2);
                    Ellipse(hdc, rc.left+8, rc.top+1, rc.left+12, rc.top+5);
                    SelectObject(hdc, oldB); DeleteObject(b2);
                    SelectObject(hdc, oldP); DeleteObject(p2);
                } else if (t == 14) { // Bio-Dome
                    HPEN p2 = CreatePen(PS_SOLID, 1, RGB(0, 255, 180)); HPEN oldP = SelectObject(hdc, p2);
                    Arc(hdc, rc.left+2, rc.top+2, rc.right-2, rc.bottom+4, rc.right-2, rc.bottom-2, rc.left+2, rc.bottom-2);
                    HBRUSH b2 = CreateSolidBrush(RGB(0, 200, 100)); HBRUSH oldB = SelectObject(hdc, b2);
                    Ellipse(hdc, rc.left+7, rc.top+8, rc.right-7, rc.bottom-2);
                    SelectObject(hdc, oldB); DeleteObject(b2);
                    SelectObject(hdc, oldP); DeleteObject(p2);
                } else if (t == 15) { // Shield Pylon
                    HPEN p2 = CreatePen(PS_SOLID, 2, RGB(0, 200, 255)); HPEN oldP = SelectObject(hdc, p2);
                    MoveToEx(hdc, rc.left+10, rc.top+2, NULL); LineTo(hdc, rc.left+4, rc.bottom-2);
                    LineTo(hdc, rc.right-4, rc.bottom-2); LineTo(hdc, rc.left+10, rc.top+2);
                    HBRUSH b2 = CreateSolidBrush(RGB(100, 220, 255)); HBRUSH oldB = SelectObject(hdc, b2);
                    Ellipse(hdc, rc.left+8, rc.top+6, rc.left+12, rc.top+10);
                    SelectObject(hdc, oldB); DeleteObject(b2);
                    SelectObject(hdc, oldP); DeleteObject(p2);
                } else if (t == 16) { // Drone Hub
                    HPEN p2 = CreatePen(PS_SOLID, 1, RGB(220, 255, 50)); HPEN oldP = SelectObject(hdc, p2);
                    Rectangle(hdc, rc.left+3, rc.bottom-5, rc.right-3, rc.bottom-2);
                    int dy = (animFrame % 8) < 4 ? 0 : 1;
                    Rectangle(hdc, rc.left+6, rc.top+4+dy, rc.right-6, rc.top+8+dy);
                    MoveToEx(hdc, rc.left+4, rc.top+3+dy, NULL); LineTo(hdc, rc.right-4, rc.top+3+dy);
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
    
    // Weather Particle Overlays
    if (dustStormTicks > 0 || weatherType == 1) { // Dust Storm
        for (int i = 0; i < 150; i++) {
            int x = (animFrame * 15 + i * 17) % (GRID_W * CELL_SIZE);
            int y = (i * 29) % (GRID_H * CELL_SIZE);
            HPEN p = CreatePen(PS_SOLID, (i % 3) + 1, RGB(200, 120, 0));
            HPEN oldP = SelectObject(hdc, p);
            MoveToEx(hdc, effOffsetX + x, effOffsetY + y, NULL);
            LineTo(hdc, effOffsetX + x + 20 + (i % 10), effOffsetY + y);
            SelectObject(hdc, oldP); DeleteObject(p);
        }
    } else if (weatherType == 3) { // Blizzard
        for (int i = 0; i < 150; i++) {
            int x = (animFrame * 12 + i * 23) % (GRID_W * CELL_SIZE);
            int y = (animFrame * 8 + i * 37) % (GRID_H * CELL_SIZE);
            HPEN p = CreatePen(PS_SOLID, (i % 2) + 1, RGB(200, 240, 255));
            HPEN oldP = SelectObject(hdc, p);
            MoveToEx(hdc, effOffsetX + x, effOffsetY + y, NULL);
            LineTo(hdc, effOffsetX + x + 6, effOffsetY + y + 6);
            SelectObject(hdc, oldP); DeleteObject(p);
        }
    } else if (weatherType == 4) { // Acid Rain
        for (int i = 0; i < 120; i++) {
            int x = (animFrame * 6 + i * 19) % (GRID_W * CELL_SIZE);
            int y = (animFrame * 16 + i * 29) % (GRID_H * CELL_SIZE);
            HPEN p = CreatePen(PS_SOLID, 1, RGB(50, 255, 80));
            HPEN oldP = SelectObject(hdc, p);
            MoveToEx(hdc, effOffsetX + x, effOffsetY + y, NULL);
            LineTo(hdc, effOffsetX + x + 2, effOffsetY + y + 10);
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
    char buf[160];
    SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 255, 255));
    
    const char* wName = (weatherType == 1 || dustStormTicks > 0) ? "DUST STORM" : (weatherType == 2 ? "SOLAR FLARE" : (weatherType == 3 ? "BLIZZARD" : (weatherType == 4 ? "ACID RAIN" : (weatherType == 5 ? "SEISMIC" : "CLEAR"))));

    sprintf(buf, "DAY %d (%s) | %s | %s | ANOMALY: %s", day, isDay ? "DAY" : "NIGHT", planetNames[planetType], wName, anomalyNames[activeAnomaly]);
    
    RECT rcTop = { 20, 10, 830, 35 };
    HBRUSH hdrBrush = CreateSolidBrush(RGB(17, 17, 34));
    FillRect(hdc, &rcTop, hdrBrush);
    HPEN hdrPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
    HPEN oldPen = SelectObject(hdc, hdrPen);
    Rectangle(hdc, rcTop.left, rcTop.top, rcTop.right, rcTop.bottom);
    DrawText(hdc, buf, -1, &rcTop, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    sprintf(buf, "FOOD:%d  PWR:%d/%d  MAT:%d  ADVM:%d  POP:%d/%d  HAP:%d%%  SCI:%d", food, power, maxPower, mat, advm, pop, maxPop, happiness, sci);
    RECT rcHeader = { 20, 40, 830, 68 };
    FillRect(hdc, &rcHeader, hdrBrush);
    Rectangle(hdc, rcHeader.left, rcHeader.top, rcHeader.right, rcHeader.bottom);
    SelectObject(hdc, oldPen);
    DeleteObject(hdrPen);
    DeleteObject(hdrBrush);
    DrawText(hdc, buf, -1, &rcHeader, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    if (msgTicks > 0) {
        RECT rcMsg = { 20, 70, 420, 85 };
        SetTextColor(hdc, RGB(255, 80, 80));
        DrawText(hdc, msgText, -1, &rcMsg, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    int sidebarX = OFFSET_X + GRID_W * CELL_SIZE + 15;
    const char* labels[] = { 
        "[-1] REPAIR", "[0] INSPECT", "[1] SOLAR", "[2] FARM", "[3] MINE", "[4] HAB", 
        "[5] BATT", "[6] LAB", "[7] NUKE", "[8] HYDRO", "[9] LASER", "[10] WALL", 
        "[11] TURRET", "[12] FACTORY", "[13] GEOTHERMAL", "[14] BIODOME", "[15] SHIELD", "[16] DRONE HUB" 
    };
    
    btnCount = 0;
    int btnY = OFFSET_Y;
    for (int i = -1; i <= 16; i++) {
        if (i == 7 && !unlockedNuke) continue;
        if (i == 8 && !unlockedHydro) continue;
        if (i == 9 && !unlockedLaser) continue;
        if (i == 12 && !unlockedFactory) continue;
        if (i == 13 && !unlockedGeo) continue;
        if (i == 14 && !unlockedBio) continue;
        if (i == 15 && !unlockedShield) continue;
        if (i == 16 && !unlockedDrone) continue;
        
        buttons[btnCount].rc = (RECT){ sidebarX, btnY, sidebarX + 185, btnY + 23 };
        buttons[btnCount].id = i;
        strcpy(buttons[btnCount].label, labels[i + 1]);
        buttons[btnCount].isSelected = (i == selectedType);
        btnCount++;
        btnY += 26;
    }
    
    int btnY2 = OFFSET_Y;
    int sidebarX2 = sidebarX + 195;
    
    // Tech Tree Research Buttons
    if (!unlockedHydro) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 23 };
        buttons[btnCount].id = 101;
        strcpy(buttons[btnCount].label, "RES: HYDRO (50S)");
        btnCount++; btnY2 += 26;
    }
    if (!unlockedFactory) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 23 };
        buttons[btnCount].id = 104;
        strcpy(buttons[btnCount].label, "RES: FACTORY (75S)");
        btnCount++; btnY2 += 26;
    }
    if (!unlockedNuke) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 23 };
        buttons[btnCount].id = 102;
        strcpy(buttons[btnCount].label, "RES: NUKE (100S)");
        btnCount++; btnY2 += 26;
    }
    if (!unlockedSolar4) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 23 };
        buttons[btnCount].id = 105;
        strcpy(buttons[btnCount].label, "RES: SOLAR IV (120S)");
        btnCount++; btnY2 += 26;
    }
    if (!unlockedLaser) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 23 };
        buttons[btnCount].id = 103;
        strcpy(buttons[btnCount].label, "RES: LASER (150S)");
        btnCount++; btnY2 += 26;
    }
    if (!unlockedXenoArmor) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 23 };
        buttons[btnCount].id = 106;
        strcpy(buttons[btnCount].label, "RES: XENO-ARM (180S)");
        btnCount++; btnY2 += 26;
    }
    if (!unlockedGeo) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 23 };
        buttons[btnCount].id = 107;
        strcpy(buttons[btnCount].label, "RES: GEOTHERM (200S)");
        btnCount++; btnY2 += 26;
    }
    if (!unlockedBio) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 23 };
        buttons[btnCount].id = 108;
        strcpy(buttons[btnCount].label, "RES: BIODOME (250S)");
        btnCount++; btnY2 += 26;
    }
    if (!unlockedShield) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 23 };
        buttons[btnCount].id = 109;
        strcpy(buttons[btnCount].label, "RES: SHIELD (300S)");
        btnCount++; btnY2 += 26;
    }
    if (!unlockedDrone) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 23 };
        buttons[btnCount].id = 110;
        strcpy(buttons[btnCount].label, "RES: DRONES (400S)");
        btnCount++; btnY2 += 26;
    }

    btnY2 += 6;
    // 3-Tier Expeditions
    buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 23 };
    buttons[btnCount].id = 201;
    strcpy(buttons[btnCount].label, "EXPED: SCOUT (1P,15M)");
    btnCount++; btnY2 += 26;

    buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 23 };
    buttons[btnCount].id = 202;
    strcpy(buttons[btnCount].label, "EXPED: RUINS (2P,30M)");
    btnCount++; btnY2 += 26;

    buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 23 };
    buttons[btnCount].id = 203;
    strcpy(buttons[btnCount].label, "EXPED: HIVE (4P,60M)");
    btnCount++; btnY2 += 26;

    btnY2 += 6;
    buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 23 };
    buttons[btnCount].id = 300;
    strcpy(buttons[btnCount].label, "[H]/[F1] HELP/MANUAL");
    btnCount++;

    for (int i = 0; i < btnCount; i++) {
        COLORREF btnBg = buttons[i].isSelected ? RGB(0, 60, 60) : RGB(17, 17, 34);
        if (buttons[i].id >= 100 && buttons[i].id < 200) btnBg = RGB(34, 17, 51); // Tech
        else if (buttons[i].id >= 200 && buttons[i].id < 300) btnBg = RGB(51, 34, 0); // Exped
        
        COLORREF btnBorder = buttons[i].isSelected ? RGB(255, 255, 255) : RGB(0, 255, 255);
        if (buttons[i].id >= 100 && buttons[i].id < 200) btnBorder = RGB(180, 50, 255);
        else if (buttons[i].id >= 200 && buttons[i].id < 300) btnBorder = RGB(255, 170, 0);
        
        COLORREF btnText = buttons[i].isSelected ? RGB(255, 255, 255) : RGB(0, 255, 255);
        if (buttons[i].id >= 100 && buttons[i].id < 200) btnText = RGB(200, 80, 255);
        else if (buttons[i].id >= 200 && buttons[i].id < 300) btnText = RGB(255, 180, 0);

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
            if (wParam == 'H' || wParam == VK_F1) {
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
            
            // Dynamic Planetary Weather Cycles
            if (weatherTicks > 0) {
                weatherTicks--;
                if (weatherTicks == 0) weatherType = 0;
            } else if (rand() % 100 < 8) { // Weather shift
                int w = (rand() % 5) + 1;
                if (planetType == 1 && rand() % 2 == 0) w = 3; // Blizzard prone
                if (planetType == 2 && rand() % 2 == 0) w = 2; // Solar flare prone
                if (planetType == 3 && rand() % 2 == 0) w = 4; // Acid rain prone
                weatherType = w;
                weatherTicks = 8 + (rand() % 8);
                if (w == 1) { strcpy(msgText, "DUST STORM! Solar efficiency down."); PlayGameSound(4); }
                else if (w == 2) { strcpy(msgText, "SOLAR FLARE! High power, electronic hazard."); PlayGameSound(2); }
                else if (w == 3) { strcpy(msgText, "BLIZZARD! Extreme freezing conditions."); PlayGameSound(4); }
                else if (w == 4) { strcpy(msgText, "ACID RAIN! Exterior structures eroding."); PlayGameSound(4); }
                else if (w == 5) { strcpy(msgText, "SEISMIC TREMOR! Ground shifting."); PlayGameSound(4); shakeTicks = 20; }
                msgTicks = 6;
            }

            // Random Disaster Events
            if (rand() % 100 < 6) {
                int r = rand() % 100;
                int targets[GRID_W * GRID_H];
                int targetCount = 0;
                for (int i = 0; i < GRID_W * GRID_H; i++) {
                    if (grid[i] > 0 && grid[i] <= 20) targets[targetCount++] = i;
                }
                
                if (r < 35 && targetCount > 0) { // Meteor Shower
                    int hits = targetCount < 3 ? targetCount : 3;
                    for (int i = 0; i < hits; i++) {
                        int idx = targets[rand() % targetCount];
                        int tx = idx % GRID_W, ty = idx / GRID_W;
                        if (IsShielded(tx, ty) && power >= 10) {
                            power -= 10;
                            SpawnParticles(OFFSET_X + tx * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + ty * CELL_SIZE + CELL_SIZE/2, RGB(0, 200, 255), 20);
                            strcpy(msgText, "METEOR DEFLECTED BY SHIELD PYLON!");
                            PlayGameSound(6);
                        } else if (grid[idx] > 0 && grid[idx] <= 20) {
                            grid[idx] += 20;
                            SpawnExplosion(OFFSET_X + tx * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + ty * CELL_SIZE + CELL_SIZE/2);
                            strcpy(msgText, "METEOR IMPACT! Structure damaged.");
                            PlayGameSound(2);
                        }
                    }
                    msgTicks = 5;
                } else if (r < 65) {
                    dustStormTicks = 8;
                    strcpy(msgText, "SAND WHIRLWIND!");
                    msgTicks = 6;
                    PlayGameSound(4);
                } else if (targetCount > 0) {
                    int idx = targets[rand() % targetCount];
                    int tx = idx % GRID_W, ty = idx / GRID_W;
                    if (!IsShielded(tx, ty) && grid[idx] > 0 && grid[idx] <= 20) {
                        grid[idx] += 20;
                        SpawnParticles(OFFSET_X + tx * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + ty * CELL_SIZE + CELL_SIZE/2, RGB(255,136,0), 15);
                        strcpy(msgText, "EQUIPMENT BREAKDOWN!");
                        msgTicks = 5;
                        PlayGameSound(2);
                    }
                }
            }
            
            if (dustStormTicks > 0) dustStormTicks--;
            if (msgTicks > 0) msgTicks--;

            // Automated Drone Hub Repairs (Type 16)
            int droneHubCount = 0;
            for (int i = 0; i < GRID_W * GRID_H; i++) {
                if (grid[i] == 16) droneHubCount++;
            }
            if (droneHubCount > 0 && power > 0) {
                for (int i = 0; i < GRID_W * GRID_H; i++) {
                    if (grid[i] > 20 && mat >= 5) {
                        mat -= 5;
                        grid[i] -= 20;
                        int tx = i % GRID_W, ty = i / GRID_W;
                        SpawnParticles(OFFSET_X + tx * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + ty * CELL_SIZE + CELL_SIZE/2, RGB(100, 255, 100), 15);
                        PlayGameSound(6);
                        strcpy(msgText, "NANITE DRONE REPAIRED STRUCTURE!");
                        msgTicks = 4;
                        break;
                    }
                }
            }

            // Defenses targeting Aliens
            int dmgBonus = unlockedXenoArmor ? 1 : 0;
            for (int i = 0; i < GRID_W * GRID_H; i++) {
                int type = grid[i] > 20 ? 0 : grid[i];
                if (type == 9 || type == 11) {
                    int range = (type == 9) ? 5 : 3;
                    int tx = i % GRID_W, ty = i / GRID_W;
                    for (int a = 0; a < alienCount; a++) {
                        int dist = abs(tx - aliens[a].x) + abs(ty - aliens[a].y);
                        if (dist <= range && power > 0) {
                            PlayGameSound(3);
                            COLORREF col = (type == 9) ? RGB(255, 0, 0) : RGB(255, 170, 0);
                            FireProjectile(OFFSET_X + tx * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + ty * CELL_SIZE + CELL_SIZE/2,
                                           OFFSET_X + aliens[a].x * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + aliens[a].y * CELL_SIZE + CELL_SIZE/2, col);
                            
                            aliens[a].hp -= (1 + dmgBonus);
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

            // Alien Movement & Assault
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
                            int tx = targetIdx % GRID_W, ty = targetIdx / GRID_W;
                            if (IsShielded(tx, ty) && power >= 15) { // Shield repels
                                power -= 15;
                                aliens[a].hp -= 3;
                                SpawnParticles(OFFSET_X + tx * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + ty * CELL_SIZE + CELL_SIZE/2, RGB(0, 255, 255), 20);
                                strcpy(msgText, "SHIELD REPELLED ALIEN INVASION!");
                                msgTicks = 5;
                                PlayGameSound(6);
                            } else if (grid[targetIdx] == 10 && unlockedXenoArmor) { // Spiked Wall
                                aliens[a].hp -= 2;
                                if (grid[targetIdx] <= 20) grid[targetIdx] += 20;
                                SpawnExplosion(OFFSET_X + tx * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + ty * CELL_SIZE + CELL_SIZE/2);
                                PlayGameSound(2);
                            } else if (grid[targetIdx] <= 20) {
                                grid[targetIdx] += 20;
                                strcpy(msgText, "ALIEN BREACH! Structure damaged.");
                                msgTicks = 5;
                                PlayGameSound(2);
                                SpawnExplosion(OFFSET_X + tx * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + ty * CELL_SIZE + CELL_SIZE/2);
                            }
                            if (aliens[a].hp <= 0) {
                                aliens[a] = aliens[--alienCount];
                                a--;
                            }
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

            // Alien Spawns
            int spawnMult = (planetType == 3) ? 2 : 1; // 2x in Acid Swamp
            int spawnChance = (10 + (day / 15)) * spawnMult;
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
            
            // Resource Production & Maintenance
            int pwrProd = 0, farmCount = 0, mineCount = 0, habCount = 0, batCount = 0;
            int labCount = 0, nukeCount = 0, hydroCount = 0, laserCount = 0, turretCount = 0, factoryCount = 0;
            int geoCount = 0, bioCount = 0, shieldCount = 0;
            
            for(int i=0; i<GRID_W*GRID_H; i++) {
                int b = grid[i];
                if(b == 1 && isDay && dustStormTicks <= 0 && weatherType != 1) {
                    int sPwr = unlockedSolar4 ? 6 : 4;
                    if (planetType == 1) sPwr /= 2; // Cryo
                    if (weatherType == 2) sPwr += 3; // Flare
                    pwrProd += sPwr;
                }
                if(b == 2) farmCount++;
                if(b == 3) mineCount++;
                if(b == 4) habCount++;
                if(b == 5) batCount++;
                if(b == 6) labCount++;
                if(b == 7) nukeCount++;
                if(b == 8) hydroCount++;
                if(b == 9) laserCount++;
                if(b == 11) turretCount++;
                if(b == 12) factoryCount++;
                if(b == 13) geoCount++;
                if(b == 14) bioCount++;
                if(b == 15) shieldCount++;
            }
            
            maxPop = habCount * 5 + bioCount * 8;
            maxPower = 50 + batCount * 50 + (activeAnomaly == 4 ? 50 : 0);
            pwrProd += nukeCount * 20;
            pwrProd += geoCount * (planetType == 1 ? 50 : 35); // Geothermal boosted on Cryo
            
            int pwrCons = farmCount * 5 + mineCount * 10 + habCount * 5 + labCount * 5 + hydroCount * 10 + 
                          laserCount * 20 + turretCount * 5 + factoryCount * 10 + bioCount * 20 + shieldCount * 30 + droneHubCount * 25;
            power += (pwrProd - pwrCons);
            
            float pwrEff = 1.0f;
            if (power < 0) { power = 0; pwrEff = 0.5f; }
            if (power > maxPower) power = maxPower;

            float eff = (happiness / 100.0f) * pwrEff;
            if (activeAnomaly == 2) eff *= 1.2f; // Nanite swarm

            int fBase = farmCount * 5 + hydroCount * 15 + bioCount * 25;
            if (planetType == 3) fBase = (int)(fBase * 1.5f); // Acid Swamp rich biosphere
            int foodProd = (int)(fBase * eff);

            int mBase = mineCount * 3;
            if (planetType == 2) mBase = (int)(mBase * 1.5f); // Volcanic rich minerals
            int matProd = (int)(mBase * eff);

            int sciProd = (int)(labCount * 2 * eff);
            if (activeAnomaly == 1) sciProd = (int)(sciProd * 1.5f); // Crystal Monolith

            food += foodProd;
            mat += matProd;
            sci += sciProd;

            // Subterranean Geode anomaly: Mines extract AdvM
            if (activeAnomaly == 3 && mineCount > 0 && rand() % 4 == 0) {
                advm += mineCount;
            }

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

            // Bio-Dome bonus happiness
            if (bioCount > 0 && power > 0) happiness = min(120, happiness + bioCount * 2);

            int foodCons = pop;
            if (food >= foodCons) {
                food -= foodCons;
                happiness += 5;
                if (happiness > 100 && bioCount == 0) happiness = 100;
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

            // Scenario Win/Loss Checks
            if (gameMode == 5 && day >= 100) {
                MessageBox(hwnd, "100 Days Survived on the Alien Frontier! You Win!", "Scenario Complete", MB_OK | MB_ICONINFORMATION);
                gameState = 0;
            }
            if (gameMode == 6) {
                if (mat >= 1000 && advm >= 100) {
                    char buf[128];
                    sprintf(buf, "Quota fulfilled on Day %d! Colony Secured!", day);
                    MessageBox(hwnd, buf, "Resource Rush Complete", MB_OK | MB_ICONINFORMATION);
                    gameState = 0;
                } else if (day > 50) {
                    MessageBox(hwnd, "Day 50 reached without meeting quota! Mission Failed.", "Quota Failed", MB_OK | MB_ICONERROR);
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
                if (x >= cx - 240 && x <= cx + 240) {
                    for (int i = 0; i < 8; i++) {
                        if (y >= 110 + i*55 && y <= 150 + i*55) {
                            if (i == 7) { prevState = 0; gameState = 2; InvalidateRect(hwnd, NULL, FALSE); }
                            else StartGame(hwnd, i);
                            return 0;
                        }
                    }
                }
                return 0;
            }
            
            if (gameState == 2) {
                RECT rc;
                GetClientRect(hwnd, &rc);
                int cx = rc.right / 2;
                if (x >= cx - 60 && x <= cx + 60 && y >= rc.bottom - 45 && y <= rc.bottom - 15) {
                    gameState = prevState;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                return 0;
            }
            
            int sidebarX = OFFSET_X + GRID_W * CELL_SIZE + 15;
            if (x >= sidebarX) {
                for (int i = 0; i < btnCount; i++) {
                    if (x >= buttons[i].rc.left && x <= buttons[i].rc.right && y >= buttons[i].rc.top && y <= buttons[i].rc.bottom) {
                        int id = buttons[i].id;
                        if (id <= 16 && id >= -1) {
                            selectedType = id;
                        } 
                        // Research Tree Unlocks
                        else if (id == 101 && sci >= 50 && !unlockedHydro) {
                            sci -= 50; unlockedHydro = 1; PlayGameSound(5);
                        } else if (id == 104 && sci >= 75 && !unlockedFactory) {
                            sci -= 75; unlockedFactory = 1; PlayGameSound(5);
                        } else if (id == 102 && sci >= 100 && !unlockedNuke) {
                            sci -= 100; unlockedNuke = 1; PlayGameSound(5);
                        } else if (id == 105 && sci >= 120 && !unlockedSolar4) {
                            sci -= 120; unlockedSolar4 = 1; PlayGameSound(5);
                            MessageBox(hwnd, "Solar Grid IV Unlocked! Solar Panels now generate +50% power.", "Tech Breakthrough", MB_OK | MB_ICONINFORMATION);
                        } else if (id == 103 && sci >= 150 && !unlockedLaser) {
                            sci -= 150; unlockedLaser = 1; PlayGameSound(5);
                        } else if (id == 106 && sci >= 180 && !unlockedXenoArmor) {
                            sci -= 180; unlockedXenoArmor = 1; PlayGameSound(5);
                            MessageBox(hwnd, "Xenobiology Armor Upgraded! Walls now retaliate against aliens and lasers/turrets deal +1 damage.", "Tech Breakthrough", MB_OK | MB_ICONINFORMATION);
                        } else if (id == 107 && sci >= 200 && !unlockedGeo) {
                            sci -= 200; unlockedGeo = 1; PlayGameSound(5);
                        } else if (id == 108 && sci >= 250 && !unlockedBio) {
                            sci -= 250; unlockedBio = 1; PlayGameSound(5);
                        } else if (id == 109 && sci >= 300 && !unlockedShield) {
                            sci -= 300; unlockedShield = 1; PlayGameSound(5);
                        } else if (id == 110 && sci >= 400 && !unlockedDrone) {
                            sci -= 400; unlockedDrone = 1; PlayGameSound(5);
                        }
                        // 3-Tier Expeditions
                        else if (id == 201) { // Scout Recon
                            if (pop >= 1 && mat >= 15 && power >= 15) {
                                mat -= 15; power -= 15;
                                int r = rand() % 100;
                                if (r < 50) {
                                    mat += 50; food += 50;
                                    MessageBox(hwnd, "Scout Recon found surface deposits!\n(+50 Mat, +50 Food)", "Recon Report", MB_OK | MB_ICONINFORMATION);
                                } else if (r < 80) {
                                    sci += 60;
                                    MessageBox(hwnd, "Scout mapped geological anomalies!\n(+60 Sci)", "Recon Report", MB_OK | MB_ICONINFORMATION);
                                } else {
                                    MessageBox(hwnd, "Scout returned safely with standard survey scans.", "Recon Report", MB_OK | MB_ICONINFORMATION);
                                }
                            } else {
                                MessageBox(hwnd, "Need 1 Pop, 15 Mat, 15 Power for Scout Recon.", "Expedition Error", MB_OK | MB_ICONWARNING);
                            }
                        } else if (id == 202) { // Ruins Excavation
                            if (pop >= 2 && mat >= 30 && power >= 30) {
                                mat -= 30; power -= 30;
                                int r = rand() % 100;
                                if (r < 30) {
                                    advm += 25; sci += 120;
                                    MessageBox(hwnd, "Excavation recovered ancient alien tech!\n(+25 AdvM, +120 Sci)", "Ruins Report", MB_OK | MB_ICONINFORMATION);
                                } else if (r < 60) {
                                    activeAnomaly = 1; // Crystal Monolith
                                    MessageBox(hwnd, "DISCOVERY! Ancient Crystal Monolith activated!\n(+50% Science production across colony)", "Planetary Anomaly Discovered", MB_OK | MB_ICONINFORMATION);
                                } else if (r < 85) {
                                    mat += 120;
                                    MessageBox(hwnd, "Excavation yielded reinforced structural alloys!\n(+120 Mat)", "Ruins Report", MB_OK | MB_ICONINFORMATION);
                                } else {
                                    pop -= 1; happiness -= 15;
                                    MessageBox(hwnd, "Excavation team triggered a cave collapse!\n(-1 Pop, -15% Happiness)", "Ruins Hazard", MB_OK | MB_ICONERROR);
                                }
                            } else {
                                MessageBox(hwnd, "Need 2 Pop, 30 Mat, 30 Power for Ruins Excavation.", "Expedition Error", MB_OK | MB_ICONWARNING);
                            }
                        } else if (id == 203) { // Hive Incursion
                            if (pop >= 4 && mat >= 60 && power >= 50) {
                                mat -= 60; power -= 50;
                                int r = rand() % 100;
                                if (r < 40) {
                                    advm += 50; sci += 250;
                                    activeAnomaly = 3; // Subterranean Geode
                                    MessageBox(hwnd, "HIVE CLEANSED! Subterranean Geode uncovered!\n(+50 AdvM, +250 Sci, Mines now yield AdvM)", "Major Incursion Victory", MB_OK | MB_ICONINFORMATION);
                                } else if (r < 75) {
                                    activeAnomaly = 4; // Ionized Aurora
                                    power = maxPower += 50;
                                    MessageBox(hwnd, "Alien Power Core Recovered! Ionized Aurora stabilized.\n(+50 Max Power permanently)", "Incursion Victory", MB_OK | MB_ICONINFORMATION);
                                } else {
                                    pop -= 2; happiness -= 30;
                                    MessageBox(hwnd, "Alien Brood Mother ambushed strike force!\n(-2 Pop, -30% Happiness)", "Heavy Casualties", MB_OK | MB_ICONERROR);
                                }
                            } else {
                                MessageBox(hwnd, "Need 4 Pop, 60 Mat, 50 Power for Hive Incursion.", "Expedition Error", MB_OK | MB_ICONWARNING);
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
            
            // Grid Interaction
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
                    else if (selectedType == 13) { costMat = 35; costAdv = 15; } // Geothermal
                    else if (selectedType == 14) { costMat = 40; costAdv = 20; costPwr = 20; } // Bio-Dome
                    else if (selectedType == 15) { costMat = 50; costAdv = 25; costPwr = 30; } // Shield Pylon
                    else if (selectedType == 16) { costMat = 60; costAdv = 30; costPwr = 25; } // Drone Hub
                    
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
            
            int dpi = GetDpiForWindow(hwnd);
            int fontHeight = -MulDiv(11, dpi, 72);
            HFONT hFont = CreateFont(fontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH, "Courier New");
            
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

    RECT rcW = {0, 0, 870, 660};
    AdjustWindowRect(&rcW, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN, FALSE);
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KColony - Planetary Tech Expansion", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
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
