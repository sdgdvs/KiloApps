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
    } else if (type == 7) { // Orbital Strike
        Beep(1000, 100); Beep(600, 150); Beep(300, 250); Beep(120, 400);
    } else if (type == 8) { // Trade Chime
        Beep(587, 80); Beep(880, 120); Beep(1174, 160);
    } else if (type == 9) { // Cavern Drill
        Beep(180, 100); Beep(220, 100); Beep(260, 150);
    }
    return 0;
}
void PlayGameSound(int type) {
    CreateThread(NULL, 0, SoundThread, (LPVOID)(LONG_PTR)type, 0, NULL);
}

int animFrame = 0;

#define PARTICLE_SPARK 0
#define PARTICLE_SMOKE 1
#define PARTICLE_DEBRIS 2
#define PARTICLE_STAR 3

typedef struct {
    float x, y;
    float vx, vy;
    int type;
    int life, maxLife;
    float size;
    float rot, vrot;
    COLORREF color;
} Particle;
Particle particles[800];
int particleCount = 0;

typedef struct {
    float x, y;
    float r1, maxR1, speed1;
    float r2, maxR2, speed2;
    int life, maxLife;
    COLORREF color;
} Shockwave;
Shockwave shockwaves[32];
int shockwaveCount = 0;

void SpawnShockwave(float x, float y, COLORREF color) {
    if (shockwaveCount < 32) {
        shockwaves[shockwaveCount].x = x;
        shockwaves[shockwaveCount].y = y;
        shockwaves[shockwaveCount].r1 = 2.0f;
        shockwaves[shockwaveCount].maxR1 = 40.0f;
        shockwaves[shockwaveCount].speed1 = 3.0f;
        shockwaves[shockwaveCount].r2 = 1.0f;
        shockwaves[shockwaveCount].maxR2 = 25.0f;
        shockwaves[shockwaveCount].speed2 = 2.0f;
        shockwaves[shockwaveCount].life = 16;
        shockwaves[shockwaveCount].maxLife = 16;
        shockwaves[shockwaveCount].color = color;
        shockwaveCount++;
    }
}

void SpawnParticles(float x, float y, COLORREF color, int count) {
    for (int i = 0; i < count && particleCount < 800; i++) {
        float angle = ((rand() % 628) / 100.0f);
        float speed = ((rand() % 60) + 20) / 10.0f;
        particles[particleCount].x = x;
        particles[particleCount].y = y;
        particles[particleCount].vx = cosf(angle) * speed;
        particles[particleCount].vy = sinf(angle) * speed;
        particles[particleCount].type = PARTICLE_SPARK;
        particles[particleCount].life = particles[particleCount].maxLife = 12 + (rand() % 10);
        particles[particleCount].size = 2.0f;
        particles[particleCount].rot = 0;
        particles[particleCount].vrot = 0;
        particles[particleCount].color = color;
        particleCount++;
    }
}

int shakeTicks = 0;

void SpawnExplosion(float x, float y) {
    // Layer 1: Incandescent Core Needle Sparks
    for (int i = 0; i < 24 && particleCount < 800; i++) {
        float angle = ((rand() % 628) / 100.0f);
        float speed = ((rand() % 80) + 30) / 10.0f;
        particles[particleCount].x = x;
        particles[particleCount].y = y;
        particles[particleCount].vx = cosf(angle) * speed;
        particles[particleCount].vy = sinf(angle) * speed;
        particles[particleCount].type = PARTICLE_SPARK;
        particles[particleCount].life = particles[particleCount].maxLife = 10 + (rand() % 8);
        particles[particleCount].size = 2.0f;
        particles[particleCount].rot = 0;
        particles[particleCount].vrot = 0;
        particles[particleCount].color = (i % 2 == 0) ? RGB(255, 255, 255) : RGB(0, 255, 255);
        particleCount++;
    }
    // Layer 2: Expanding Buoyant Plasma/Smoke Puffs
    for (int i = 0; i < 16 && particleCount < 800; i++) {
        float angle = ((rand() % 628) / 100.0f);
        float speed = ((rand() % 40) + 10) / 10.0f;
        particles[particleCount].x = x;
        particles[particleCount].y = y;
        particles[particleCount].vx = cosf(angle) * speed;
        particles[particleCount].vy = sinf(angle) * speed - 0.8f;
        particles[particleCount].type = PARTICLE_SMOKE;
        particles[particleCount].life = particles[particleCount].maxLife = 14 + (rand() % 10);
        particles[particleCount].size = 3.0f + (rand() % 3);
        particles[particleCount].rot = 0;
        particles[particleCount].vrot = 0;
        particles[particleCount].color = (i % 3 == 0) ? RGB(255, 85, 0) : ((i % 3 == 1) ? RGB(255, 170, 0) : RGB(80, 80, 80));
        particleCount++;
    }
    // Layer 3: Heavy Kinematic Debris Shards (Gravity & Tumbling)
    for (int i = 0; i < 14 && particleCount < 800; i++) {
        float angle = ((rand() % 628) / 100.0f);
        float speed = ((rand() % 60) + 20) / 10.0f;
        particles[particleCount].x = x;
        particles[particleCount].y = y;
        particles[particleCount].vx = cosf(angle) * speed;
        particles[particleCount].vy = sinf(angle) * speed - 1.5f;
        particles[particleCount].type = PARTICLE_DEBRIS;
        particles[particleCount].life = particles[particleCount].maxLife = 16 + (rand() % 10);
        particles[particleCount].size = 3.0f;
        particles[particleCount].rot = (rand() % 628) / 100.0f;
        particles[particleCount].vrot = ((rand() % 40) - 20) / 100.0f;
        particles[particleCount].color = (i % 2 == 0) ? RGB(255, 0, 255) : RGB(170, 170, 170);
        particleCount++;
    }
    // Layer 4: Radiant Celebration Energy Stars
    for (int i = 0; i < 8 && particleCount < 800; i++) {
        float angle = ((rand() % 628) / 100.0f);
        float speed = ((rand() % 40) + 15) / 10.0f;
        particles[particleCount].x = x;
        particles[particleCount].y = y;
        particles[particleCount].vx = cosf(angle) * speed;
        particles[particleCount].vy = sinf(angle) * speed;
        particles[particleCount].type = PARTICLE_STAR;
        particles[particleCount].life = particles[particleCount].maxLife = 12 + (rand() % 8);
        particles[particleCount].size = 4.0f;
        particles[particleCount].rot = 0;
        particles[particleCount].vrot = 0;
        particles[particleCount].color = RGB(255, 255, 0);
        particleCount++;
    }
    SpawnShockwave(x, y, RGB(0, 255, 255));
    shakeTicks = 18;
}

void SpawnCelebrationStars(float x, float y, COLORREF color, int count) {
    for (int i = 0; i < count && particleCount < 800; i++) {
        float angle = ((rand() % 628) / 100.0f);
        float speed = ((rand() % 50) + 15) / 10.0f;
        particles[particleCount].x = x;
        particles[particleCount].y = y;
        particles[particleCount].vx = cosf(angle) * speed;
        particles[particleCount].vy = sinf(angle) * speed;
        particles[particleCount].type = PARTICLE_STAR;
        particles[particleCount].life = particles[particleCount].maxLife = 15 + (rand() % 10);
        particles[particleCount].size = 4.0f + (rand() % 3);
        particles[particleCount].rot = 0;
        particles[particleCount].vrot = 0;
        particles[particleCount].color = color;
        particleCount++;
    }
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
int unlockedTrade = 0, unlockedCavern = 0, unlockedOrbital = 0;
int freighterDays = 0;

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
Button buttons[60];
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
    DrawText(hdc, "Select Planet Biome [1-7]  |  Press [H] or [F1] for Administrator's Manual", -1, &(RECT){0, 75, rc.right, 100}, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    const char* titles[] = { 
        "[1] MARS PRIME (STANDARD EXPEDITION)", 
        "[2] CRYO TUNDRA (GLIESE 667 - BLIZZARDS)", 
        "[3] VOLCANIC INFERNO (KEPLER 10B - HEATWAVES)", 
        "[4] ACID SWAMP (PROXIMA B - ALIEN SWARMS)", 
        "[5] SANDBOX COLONY (UNLIMITED TECH & MATS)", 
        "[6] 100-DAY SURVIVAL CHALLENGE", 
        "[7] RESOURCE RUSH (1000M, 100A BY D50)", 
        "[8 / H / F1] HELP & TECH SPEC SHEET" 
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
    
    RECT textRc = {25, 12, rc.right - 25, rc.bottom - 50};
    const char* helpText = 
        "KCOLONY ADMINISTRATOR'S MANUAL & EXPANDED SPEC SHEET\n\n"
        "KEYBOARD SHORTCUTS:\n"
        "[1-7] Select Mode (Menu) | [0/Esc] Inspect | [-/R] Repair | [1-6] Core (Solar/Farm/Mine/Hab/Bat/Lab)\n"
        "[7] Nuke | [8] Hydro | [9] Laser | [W] Wall | [T] Turret | [C] Factory | [G] Geo | [V] Bio-Dome\n"
        "[E] Shield | [U] Drone Hub | [P] Trade Port | [K] Cavern Drill | [O] Orbital Beacon | [Space] Orbital Strike\n"
        "[H/F1/?] Toggle Manual | [Esc/Space/Enter] Close Help & Return\n\n"
        "RESOURCES: Food (Colony upkeep) | Power (System operations) | Mat (Basic building)\n"
        "AdvM (Advanced alloy) | Science (Research unlocks) | Happiness (Efficiency factor)\n\n"
        "CORE STRUCTURES:\n"
        "Solar [1]: +4 Pwr (Day) | Farm [2]: +5 Food, -5 Pwr | Mine [3]: +3 Mat, -10 Pwr\n"
        "Hab [4]: +5 Max Pop, -5 Pwr | Battery [5]: +50 Max Pwr | Lab [6]: +2 Sci, -5 Pwr\n"
        "Nuke [7]: +20 Pwr constant | Hydro [8]: +15 Food, -10 Pwr | Laser [9]: Range 5, -20 Pwr\n"
        "Wall [W]: Deflects aliens | Turret [T]: Range 3, -5 Pwr | Factory [C]: 2 Mat -> 1 AdvM\n\n"
        "ADVANCED STRUCTURES:\n"
        "Geothermal [G]: +35 Pwr 24/7 immune to storms | Bio-Dome [V]: +25 Food, +8 Pop, +Happiness\n"
        "Shield Pylon [E]: 3x3 Barrier vs meteors/aliens | Drone Hub [U]: Auto-repairs broken structures\n"
        "Trade Port [P]: Tariff revenue & freighters | Cavern Drill [K]: Deep magma power & geodes\n"
        "Orbital Beacon [O]: Meteor defense & unlocks tactical ORBITAL STRIKE [Space]!\n";
        
    DrawText(hdc, helpText, -1, &textRc, DT_LEFT | DT_TOP);
    
    RECT btnRc = {rc.right / 2 - 80, rc.bottom - 42, rc.right / 2 + 80, rc.bottom - 12};
    HBRUSH br = CreateSolidBrush(RGB(17,17,34));
    FillRect(hdc, &btnRc, br);
    DeleteObject(br);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
    HPEN oldP = SelectObject(hdc, pen);
    MoveToEx(hdc, btnRc.left, btnRc.top, NULL); LineTo(hdc, btnRc.right, btnRc.top); LineTo(hdc, btnRc.right, btnRc.bottom); LineTo(hdc, btnRc.left, btnRc.bottom); LineTo(hdc, btnRc.left, btnRc.top);
    SelectObject(hdc, oldP); DeleteObject(pen);
    DrawText(hdc, "CLOSE GUIDE [ESC]", -1, &btnRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void StartGame(HWND hwnd, int mode) {
    gameState = 1;
    gameMode = mode;
    
    if (mode >= 0 && mode <= 3) planetType = mode;
    else planetType = 0;

    food = 50; power = 50; maxPower = 50; mat = 50; advm = 0;
    pop = 0; maxPop = 0; happiness = 100; sci = 0; popWait = 0;
    day = 1; isDay = 1; tick = 0; freighterDays = 0;
    
    unlockedHydro = 0; unlockedNuke = 0; unlockedLaser = 0; unlockedFactory = 0;
    unlockedSolar4 = 0; unlockedXenoArmor = 0; unlockedGeo = 0; unlockedBio = 0; unlockedShield = 0; unlockedDrone = 0;
    unlockedTrade = 0; unlockedCavern = 0; unlockedOrbital = 0;
    activeAnomaly = 0; weatherType = 0; weatherTicks = 0; dustStormTicks = 0;

    alienCount = 0;
    memset(grid, 0, sizeof(grid));
    
    if (mode == 4) { // Sandbox
        food = 9999; power = 9999; maxPower = 9999; mat = 9999; advm = 9999; sci = 9999;
        unlockedHydro = 1; unlockedNuke = 1; unlockedLaser = 1; unlockedFactory = 1;
        unlockedSolar4 = 1; unlockedXenoArmor = 1; unlockedGeo = 1; unlockedBio = 1; unlockedShield = 1; unlockedDrone = 1;
        unlockedTrade = 1; unlockedCavern = 1; unlockedOrbital = 1;
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
        float shakeFactor = (float)shakeTicks / 18.0f;
        effOffsetX += (int)(sinf(animFrame * 1.5f) * (shakeFactor * 7.0f));
        effOffsetY += (int)(cosf(animFrame * 1.2f) * (shakeFactor * 7.0f));
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

    int glintCell = (animFrame * 2) % (GRID_W + GRID_H);

    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            RECT rc = { effOffsetX + x * CELL_SIZE, effOffsetY + y * CELL_SIZE, effOffsetX + (x + 1) * CELL_SIZE, effOffsetY + (y + 1) * CELL_SIZE };
            
            int t = grid[y * GRID_W + x];
            int seed = y * GRID_W + x;
            
            COLORREF bgCol = RGB(10, 17, 26);
            COLORREF borderCol = RGB(0, 51, 51);
            COLORREF textCol = RGB(0, 255, 255);
            
            if (t > 20) { bgCol = RGB(51, 0, 0); borderCol = RGB(255, 0, 0); textCol = RGB(255, 0, 0); }
            else if (t == 1) { bgCol = RGB(16, 32, 64); borderCol = RGB(0, 255, 255); textCol = RGB(0, 255, 255); }
            else if (t == 2) { bgCol = RGB(0, 38, 14); borderCol = RGB(0, 255, 102); textCol = RGB(0, 255, 102); }
            else if (t == 3) { bgCol = RGB(51, 0, 51); borderCol = RGB(255, 0, 255); textCol = RGB(255, 0, 255); }
            else if (t == 4) { bgCol = RGB(0, 43, 51); borderCol = RGB(0, 255, 255); textCol = RGB(0, 255, 255); }
            else if (t == 5) { bgCol = RGB(51, 21, 0); borderCol = RGB(255, 136, 0); textCol = RGB(255, 136, 0); }
            else if (t == 6) { bgCol = RGB(34, 0, 51); borderCol = RGB(170, 0, 255); textCol = RGB(170, 0, 255); }
            else if (t == 7) { bgCol = RGB(0, 51, 34); borderCol = RGB(0, 255, 170); textCol = RGB(0, 255, 170); }
            else if (t == 8) { bgCol = RGB(34, 51, 0); borderCol = RGB(170, 255, 0); textCol = RGB(170, 255, 0); }
            else if (t == 9) { bgCol = RGB(51, 0, 0); borderCol = RGB(255, 0, 0); textCol = RGB(255, 0, 0); }
            else if (t == 10) { bgCol = RGB(34, 34, 34); borderCol = RGB(136, 136, 136); textCol = RGB(136, 136, 136); }
            else if (t == 11) { bgCol = RGB(51, 34, 0); borderCol = RGB(255, 170, 0); textCol = RGB(255, 170, 0); }
            else if (t == 12) { bgCol = RGB(68, 68, 68); borderCol = RGB(204, 204, 204); textCol = RGB(204, 204, 204); }
            else if (t == 13) { bgCol = RGB(68, 20, 0); borderCol = RGB(255, 102, 0); textCol = RGB(255, 120, 0); } // Geo
            else if (t == 14) { bgCol = RGB(0, 51, 34); borderCol = RGB(0, 255, 180); textCol = RGB(50, 255, 180); } // Bio
            else if (t == 15) { bgCol = RGB(0, 34, 68); borderCol = RGB(0, 204, 255); textCol = RGB(0, 220, 255); } // Shield
            else if (t == 16) { bgCol = RGB(34, 51, 0); borderCol = RGB(204, 255, 0); textCol = RGB(220, 255, 50); } // Drone
            else if (t == 17) { bgCol = RGB(21, 37, 53); borderCol = RGB(255, 170, 0); textCol = RGB(255, 200, 50); } // Trade Port
            else if (t == 18) { bgCol = RGB(53, 32, 16); borderCol = RGB(238, 102, 34); textCol = RGB(255, 140, 50); } // Cavern Drill
            else if (t == 19) { bgCol = RGB(16, 32, 64); borderCol = RGB(0, 204, 255); textCol = RGB(100, 220, 255); } // Orbital Beacon
            
            if (t > 0) {
                if (t == 9 || t == 11 || t == 7 || t == 13 || t == 15 || t == 17 || t == 18 || t == 19) {
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

                // High-Tech Diagonal Specular Sheen Sweep Highlight
                if ((x + y) == glintCell && (t == 1 || t == 7 || t == 9 || t == 13 || t == 14 || t == 15 || t == 19)) {
                    HPEN glintPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                    HPEN oldGp = SelectObject(hdc, glintPen);
                    MoveToEx(hdc, rc.left + 2, rc.bottom - 2, NULL);
                    LineTo(hdc, rc.right - 2, rc.top + 2);
                    SelectObject(hdc, oldGp);
                    DeleteObject(glintPen);
                }
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
                } else if (t == 17) { // Trade Port
                    HPEN p2 = CreatePen(PS_SOLID, 1, RGB(255, 200, 0)); HPEN oldP = SelectObject(hdc, p2);
                    Rectangle(hdc, rc.left+2, rc.top+2, rc.right-2, rc.bottom-2);
                    MoveToEx(hdc, rc.left+5, rc.top+5, NULL); LineTo(hdc, rc.right-5, rc.bottom-5);
                    MoveToEx(hdc, rc.right-5, rc.top+5, NULL); LineTo(hdc, rc.left+5, rc.bottom-5);
                    HBRUSH b2 = CreateSolidBrush(RGB(255, 220, 50)); HBRUSH oldB = SelectObject(hdc, b2);
                    Ellipse(hdc, rc.left+7, rc.top+7, rc.right-7, rc.bottom-7);
                    SelectObject(hdc, oldB); DeleteObject(b2);
                    SelectObject(hdc, oldP); DeleteObject(p2);
                } else if (t == 18) { // Cavern Drill
                    HPEN p2 = CreatePen(PS_SOLID, 1, RGB(220, 110, 40)); HPEN oldP = SelectObject(hdc, p2);
                    POINT pts[3] = {{rc.left+10, rc.top+2}, {rc.left+3, rc.bottom-3}, {rc.right-3, rc.bottom-3}};
                    Polygon(hdc, pts, 3);
                    HBRUSH b2 = CreateSolidBrush(RGB(255, 140, 50)); HBRUSH oldB = SelectObject(hdc, b2);
                    int dy = (animFrame % 6);
                    Rectangle(hdc, rc.left+8, rc.top+6 + dy, rc.right-8, rc.top+10 + dy);
                    SelectObject(hdc, oldB); DeleteObject(b2);
                    SelectObject(hdc, oldP); DeleteObject(p2);
                } else if (t == 19) { // Orbital Beacon
                    HPEN p2 = CreatePen(PS_SOLID, 2, RGB(0, 200, 255)); HPEN oldP = SelectObject(hdc, p2);
                    Arc(hdc, rc.left+3, rc.top+5, rc.right-3, rc.bottom+3, rc.right-3, rc.bottom-2, rc.left+3, rc.bottom-2);
                    MoveToEx(hdc, rc.left+10, rc.top+8, NULL); LineTo(hdc, rc.left+10, rc.top+1);
                    HBRUSH b2 = CreateSolidBrush(RGB(150, 230, 255)); HBRUSH oldB = SelectObject(hdc, b2);
                    Ellipse(hdc, rc.left+8, rc.top+1, rc.left+12, rc.top+5);
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
    
    // 1. Ornate Cybernetic Colony Defense Arcade HUD Reticle L-Brackets around Grid
    int gw = GRID_W * CELL_SIZE, gh = GRID_H * CELL_SIZE;
    HPEN hudPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 255));
    HPEN oldHp = SelectObject(hdc, hudPen);
    // Top-Left
    MoveToEx(hdc, effOffsetX - 4, effOffsetY + 14, NULL); LineTo(hdc, effOffsetX - 4, effOffsetY - 4); LineTo(hdc, effOffsetX + 14, effOffsetY - 4);
    // Top-Right
    MoveToEx(hdc, effOffsetX + gw - 14, effOffsetY - 4, NULL); LineTo(hdc, effOffsetX + gw + 4, effOffsetY - 4); LineTo(hdc, effOffsetX + gw + 4, effOffsetY + 14);
    // Bottom-Left
    MoveToEx(hdc, effOffsetX - 4, effOffsetY + gh - 14, NULL); LineTo(hdc, effOffsetX - 4, effOffsetY + gh + 4); LineTo(hdc, effOffsetX + 14, effOffsetY + gh + 4);
    // Bottom-Right
    MoveToEx(hdc, effOffsetX + gw - 14, effOffsetY + gh + 4, NULL); LineTo(hdc, effOffsetX + gw + 4, effOffsetY + gh + 4); LineTo(hdc, effOffsetX + gw + 4, effOffsetY + gh - 14);
    SelectObject(hdc, oldHp);
    DeleteObject(hudPen);

    // Glowing Status Diodes on Grid Frame
    HBRUSH diodeBr = CreateSolidBrush(((animFrame % 12) < 6) ? RGB(0, 255, 255) : RGB(0, 150, 150));
    HBRUSH oldDb = SelectObject(hdc, diodeBr);
    HPEN nullP = CreatePen(PS_NULL, 0, 0);
    HPEN oldNp = SelectObject(hdc, nullP);
    Ellipse(hdc, effOffsetX - 3, effOffsetY - 3, effOffsetX + 1, effOffsetY + 1);
    Ellipse(hdc, effOffsetX + gw - 1, effOffsetY - 3, effOffsetX + gw + 3, effOffsetY + 1);
    Ellipse(hdc, effOffsetX - 3, effOffsetY + gh - 1, effOffsetX + 1, effOffsetY + gh + 3);
    Ellipse(hdc, effOffsetX + gw - 1, effOffsetY + gh - 1, effOffsetX + gw + 3, effOffsetY + gh + 3);
    SelectObject(hdc, oldNp); DeleteObject(nullP);
    SelectObject(hdc, oldDb); DeleteObject(diodeBr);

    // Traveling Specular Glint along Perimeter Frame
    int borderLen = (gw + gh) * 2;
    int bGlint = (animFrame * 8) % borderLen;
    int glintX = effOffsetX, glintY = effOffsetY;
    if (bGlint < gw) { glintX += bGlint; glintY += 0; }
    else if (bGlint < gw + gh) { glintX += gw; glintY += (bGlint - gw); }
    else if (bGlint < gw * 2 + gh) { glintX += gw - (bGlint - (gw + gh)); glintY += gh; }
    else { glintX += 0; glintY += gh - (bGlint - (gw * 2 + gh)); }
    
    HBRUSH glintB = CreateSolidBrush(RGB(255, 255, 255));
    HBRUSH oldGb = SelectObject(hdc, glintB);
    Ellipse(hdc, glintX - 3, glintY - 3, glintX + 3, glintY + 3);
    SelectObject(hdc, oldGb); DeleteObject(glintB);

    // 2. Atmospheric Planetary Biome Motes
    if (planetType == 0) { // Mars Prime Red Dust
        for (int i = 0; i < 30; i++) {
            int mx = (animFrame * 2 + i * 47) % (GRID_W * CELL_SIZE);
            int my = (i * 37 + (int)(sinf(animFrame * 0.1f + i) * 6)) % (GRID_H * CELL_SIZE);
            HPEN p = CreatePen(PS_SOLID, 1, RGB(255, 120, 60));
            HPEN oldP = SelectObject(hdc, p);
            MoveToEx(hdc, effOffsetX + mx, effOffsetY + my, NULL);
            LineTo(hdc, effOffsetX + mx + 2, effOffsetY + my);
            SelectObject(hdc, oldP); DeleteObject(p);
        }
    } else if (planetType == 1) { // Gliese Cryo Ice Crystals
        for (int i = 0; i < 35; i++) {
            int mx = (animFrame * 2 + i * 31) % (GRID_W * CELL_SIZE);
            int my = (animFrame * 3 + i * 53) % (GRID_H * CELL_SIZE);
            HPEN p = CreatePen(PS_SOLID, 1, RGB(180, 230, 255));
            HPEN oldP = SelectObject(hdc, p);
            MoveToEx(hdc, effOffsetX + mx, effOffsetY + my, NULL);
            LineTo(hdc, effOffsetX + mx + 2, effOffsetY + my + 2);
            SelectObject(hdc, oldP); DeleteObject(p);
        }
    } else if (planetType == 2) { // Kepler Volcanic Fiery Embers
        for (int i = 0; i < 30; i++) {
            int mx = (i * 43 + (int)(sinf(animFrame * 0.15f + i) * 8)) % (GRID_W * CELL_SIZE);
            int my = (GRID_H * CELL_SIZE) - ((animFrame * 3 + i * 41) % (GRID_H * CELL_SIZE));
            COLORREF ec = (i % 2 == 0) ? RGB(255, 100, 0) : RGB(255, 220, 50);
            HPEN p = CreatePen(PS_SOLID, 2, ec);
            HPEN oldP = SelectObject(hdc, p);
            MoveToEx(hdc, effOffsetX + mx, effOffsetY + my, NULL);
            LineTo(hdc, effOffsetX + mx, effOffsetY + my - 2);
            SelectObject(hdc, oldP); DeleteObject(p);
        }
    } else if (planetType == 3) { // Proxima Acid Bioluminescent Spores
        for (int i = 0; i < 30; i++) {
            int mx = (i * 39 + (int)(sinf(animFrame * 0.08f + i) * 10)) % (GRID_W * CELL_SIZE);
            int my = (i * 49 + (int)(cosf(animFrame * 0.08f + i) * 10)) % (GRID_H * CELL_SIZE);
            HPEN p = CreatePen(PS_SOLID, 1, RGB(80, 255, 120));
            HPEN oldP = SelectObject(hdc, p);
            MoveToEx(hdc, effOffsetX + mx, effOffsetY + my, NULL);
            LineTo(hdc, effOffsetX + mx + 1, effOffsetY + my + 1);
            SelectObject(hdc, oldP); DeleteObject(p);
        }
    }

    // 3. Weather Particle Overlays
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
    }
    
    // 4. Aliens
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
    
    // 5. Dual-Tier Concentric Shockwave Ripple Rings
    for (int i = 0; i < shockwaveCount; i++) {
        HPEN sp1 = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
        HPEN oldSp1 = SelectObject(hdc, sp1);
        HBRUSH nullB = (HBRUSH)GetStockObject(NULL_BRUSH);
        HBRUSH oldSb = SelectObject(hdc, nullB);
        int sx = (int)shockwaves[i].x + (effOffsetX - OFFSET_X);
        int sy = (int)shockwaves[i].y + (effOffsetY - OFFSET_Y);
        int r1 = (int)shockwaves[i].r1;
        Ellipse(hdc, sx - r1, sy - r1, sx + r1, sy + r1);
        
        HPEN sp2 = CreatePen(PS_SOLID, 1, shockwaves[i].color);
        SelectObject(hdc, sp2);
        int r2 = (int)shockwaves[i].r2;
        Ellipse(hdc, sx - r2, sy - r2, sx + r2, sy + r2);
        
        SelectObject(hdc, oldSb);
        SelectObject(hdc, oldSp1);
        DeleteObject(sp2);
        DeleteObject(sp1);
    }

    // 6. Multi-Layered Kinematic Particle Simulation Drawing
    for (int i = 0; i < particleCount; i++) {
        int px = (int)particles[i].x + (effOffsetX - OFFSET_X);
        int py = (int)particles[i].y + (effOffsetY - OFFSET_Y);

        if (particles[i].type == PARTICLE_SPARK) {
            HPEN p = CreatePen(PS_SOLID, (int)particles[i].size, particles[i].color);
            HPEN oldP = SelectObject(hdc, p);
            MoveToEx(hdc, px, py, NULL);
            LineTo(hdc, px - (int)(particles[i].vx * 2.0f), py - (int)(particles[i].vy * 2.0f));
            SelectObject(hdc, oldP);
            DeleteObject(p);
        } else if (particles[i].type == PARTICLE_SMOKE) {
            HBRUSH b = CreateSolidBrush(particles[i].color);
            HPEN p = CreatePen(PS_SOLID, 1, particles[i].color);
            HBRUSH oldB = SelectObject(hdc, b);
            HPEN oldP = SelectObject(hdc, p);
            int r = (int)particles[i].size;
            Ellipse(hdc, px - r, py - r, px + r, py + r);
            SelectObject(hdc, oldP); SelectObject(hdc, oldB);
            DeleteObject(p); DeleteObject(b);
        } else if (particles[i].type == PARTICLE_DEBRIS) {
            HBRUSH b = CreateSolidBrush(particles[i].color);
            HPEN p = CreatePen(PS_SOLID, 1, particles[i].color);
            HBRUSH oldB = SelectObject(hdc, b);
            HPEN oldP = SelectObject(hdc, p);
            int s = (int)particles[i].size;
            Rectangle(hdc, px - s, py - s, px + s, py + s);
            SelectObject(hdc, oldP); SelectObject(hdc, oldB);
            DeleteObject(p); DeleteObject(b);
        } else if (particles[i].type == PARTICLE_STAR) {
            HBRUSH b = CreateSolidBrush(particles[i].color);
            HPEN p = CreatePen(PS_SOLID, 1, particles[i].color);
            HBRUSH oldB = SelectObject(hdc, b);
            HPEN oldP = SelectObject(hdc, p);
            int s = (int)particles[i].size;
            POINT starPts[4] = {{px, py - s}, {px + s, py}, {px, py + s}, {px - s, py}};
            Polygon(hdc, starPts, 4);
            SelectObject(hdc, oldP); SelectObject(hdc, oldB);
            DeleteObject(p); DeleteObject(b);
        }
    }

    // 7. Projectiles
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

    // Corner reticles on top bar
    HPEN reticlePen = CreatePen(PS_SOLID, 2, RGB(0, 255, 255));
    SelectObject(hdc, reticlePen);
    MoveToEx(hdc, rcTop.left + 2, rcTop.top + 8, NULL); LineTo(hdc, rcTop.left + 2, rcTop.top + 2); LineTo(hdc, rcTop.left + 8, rcTop.top + 2);
    MoveToEx(hdc, rcTop.right - 8, rcTop.top + 2, NULL); LineTo(hdc, rcTop.right - 2, rcTop.top + 2); LineTo(hdc, rcTop.right - 2, rcTop.top + 8);
    MoveToEx(hdc, rcTop.left + 2, rcTop.bottom - 8, NULL); LineTo(hdc, rcTop.left + 2, rcTop.bottom - 2); LineTo(hdc, rcTop.left + 8, rcTop.bottom - 2);
    MoveToEx(hdc, rcTop.right - 8, rcTop.bottom - 2, NULL); LineTo(hdc, rcTop.right - 2, rcTop.bottom - 2); LineTo(hdc, rcTop.right - 2, rcTop.bottom - 8);
    SelectObject(hdc, hdrPen);
    DeleteObject(reticlePen);

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
        "[-/R] REPAIR", "[0/ESC] NONE", "[1] SOLAR (10M)", "[2] FARM (10M,5P)", "[3] MINE (10P)", "[4] HAB (15M,5P)", 
        "[5] BATT (20M)", "[6] LAB (20M,5P)", "[7] NUKE (20M,10A)", "[8] HYDRO (20M,10P)", "[9] LASER (10M,10A)", "[10/W] WALL (5M)", 
        "[11/T] TURRET (15M,5P)", "[12/C] FACT (30M,10P)", "[13/G] GEO (35M,15A)", "[14/V] BIODOME (40M,20A)", "[15/E] SHIELD (50M,25A)", "[16/U] DRONE (60M,30A)",
        "[17/P] TRADE (70M,35A)", "[18/K] CAVERN (80M,40A)", "[19/O] BEACON (100M,50A)"
    };
    
    btnCount = 0;
    int btnY = OFFSET_Y;
    for (int i = -1; i <= 19; i++) {
        if (i == 7 && !unlockedNuke) continue;
        if (i == 8 && !unlockedHydro) continue;
        if (i == 9 && !unlockedLaser) continue;
        if (i == 12 && !unlockedFactory) continue;
        if (i == 13 && !unlockedGeo) continue;
        if (i == 14 && !unlockedBio) continue;
        if (i == 15 && !unlockedShield) continue;
        if (i == 16 && !unlockedDrone) continue;
        if (i == 17 && !unlockedTrade) continue;
        if (i == 18 && !unlockedCavern) continue;
        if (i == 19 && !unlockedOrbital) continue;
        
        buttons[btnCount].rc = (RECT){ sidebarX, btnY, sidebarX + 185, btnY + 22 };
        buttons[btnCount].id = i;
        strcpy(buttons[btnCount].label, labels[i + 1]);
        buttons[btnCount].isSelected = (i == selectedType);
        btnCount++;
        btnY += 24;
    }
    
    int btnY2 = OFFSET_Y;
    int sidebarX2 = sidebarX + 195;
    
    // Tech Tree Research Buttons
    if (!unlockedHydro) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
        buttons[btnCount].id = 101;
        strcpy(buttons[btnCount].label, "RES: HYDRO (50S)");
        btnCount++; btnY2 += 24;
    }
    if (!unlockedFactory) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
        buttons[btnCount].id = 104;
        strcpy(buttons[btnCount].label, "RES: FACTORY (75S)");
        btnCount++; btnY2 += 24;
    }
    if (!unlockedNuke) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
        buttons[btnCount].id = 102;
        strcpy(buttons[btnCount].label, "RES: NUKE (100S)");
        btnCount++; btnY2 += 24;
    }
    if (!unlockedSolar4) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
        buttons[btnCount].id = 105;
        strcpy(buttons[btnCount].label, "RES: SOLAR IV (120S)");
        btnCount++; btnY2 += 24;
    }
    if (!unlockedLaser) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
        buttons[btnCount].id = 103;
        strcpy(buttons[btnCount].label, "RES: LASER (150S)");
        btnCount++; btnY2 += 24;
    }
    if (!unlockedXenoArmor) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
        buttons[btnCount].id = 106;
        strcpy(buttons[btnCount].label, "RES: XENO-ARM (180S)");
        btnCount++; btnY2 += 24;
    }
    if (!unlockedGeo) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
        buttons[btnCount].id = 107;
        strcpy(buttons[btnCount].label, "RES: GEOTHERM (200S)");
        btnCount++; btnY2 += 24;
    }
    if (!unlockedBio) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
        buttons[btnCount].id = 108;
        strcpy(buttons[btnCount].label, "RES: BIODOME (250S)");
        btnCount++; btnY2 += 24;
    }
    if (!unlockedShield) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
        buttons[btnCount].id = 109;
        strcpy(buttons[btnCount].label, "RES: SHIELD (300S)");
        btnCount++; btnY2 += 24;
    }
    if (!unlockedTrade) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
        buttons[btnCount].id = 111;
        strcpy(buttons[btnCount].label, "RES: TRADE PORT (350S)");
        btnCount++; btnY2 += 24;
    }
    if (!unlockedDrone) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
        buttons[btnCount].id = 110;
        strcpy(buttons[btnCount].label, "RES: DRONES (400S)");
        btnCount++; btnY2 += 24;
    }
    if (!unlockedCavern) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
        buttons[btnCount].id = 112;
        strcpy(buttons[btnCount].label, "RES: CAVERN DRILL (450S)");
        btnCount++; btnY2 += 24;
    }
    if (!unlockedOrbital) {
        buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
        buttons[btnCount].id = 113;
        strcpy(buttons[btnCount].label, "RES: ORBITAL UPLINK (500S)");
        btnCount++; btnY2 += 24;
    }

    btnY2 += 4;
    // Expeditions & Operations
    buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
    buttons[btnCount].id = 201;
    strcpy(buttons[btnCount].label, "EXPED: SCOUT (1P,15M)");
    btnCount++; btnY2 += 24;

    buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
    buttons[btnCount].id = 202;
    strcpy(buttons[btnCount].label, "EXPED: RUINS (2P,30M)");
    btnCount++; btnY2 += 24;

    buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
    buttons[btnCount].id = 203;
    strcpy(buttons[btnCount].label, "EXPED: HIVE (4P,60M)");
    btnCount++; btnY2 += 24;

    buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
    buttons[btnCount].id = 204;
    strcpy(buttons[btnCount].label, "TRADE FREIGHTER");
    btnCount++; btnY2 += 24;

    buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
    buttons[btnCount].id = 205;
    strcpy(buttons[btnCount].label, "CAVERN DIVE (2P,40M)");
    btnCount++; btnY2 += 24;

    buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
    buttons[btnCount].id = 206;
    strcpy(buttons[btnCount].label, "[SPACE] ORBITAL STRIKE");
    btnCount++; btnY2 += 24;

    btnY2 += 4;
    buttons[btnCount].rc = (RECT){ sidebarX2, btnY2, sidebarX2 + 180, btnY2 + 22 };
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
        case WM_KEYDOWN: {
            if (wParam == 'H' || wParam == VK_F1) {
                if (gameState == 2) {
                    gameState = prevState;
                } else {
                    prevState = gameState;
                    gameState = 2;
                }
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (gameState == 0) { // Menu
                if (wParam >= '1' && wParam <= '7') {
                    StartGame(hwnd, (int)(wParam - '1'));
                    return 0;
                } else if (wParam == '8') {
                    prevState = 0;
                    gameState = 2;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            } else if (gameState == 2) { // Help
                if (wParam == VK_ESCAPE || wParam == VK_RETURN || wParam == VK_SPACE) {
                    gameState = prevState;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            } else if (gameState == 1) { // Playing
                if (wParam == VK_ESCAPE || wParam == '0' || wParam == 'I') {
                    selectedType = 0;
                } else if (wParam == 'R' || wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {
                    selectedType = -1;
                } else if (wParam == '1') {
                    selectedType = 1;
                } else if (wParam == '2') {
                    selectedType = 2;
                } else if (wParam == '3') {
                    selectedType = 3;
                } else if (wParam == '4') {
                    selectedType = 4;
                } else if (wParam == '5') {
                    selectedType = 5;
                } else if (wParam == '6') {
                    selectedType = 6;
                } else if (wParam == '7' && unlockedNuke) {
                    selectedType = 7;
                } else if (wParam == '8' && unlockedHydro) {
                    selectedType = 8;
                } else if (wParam == '9' && unlockedLaser) {
                    selectedType = 9;
                } else if (wParam == 'W') {
                    selectedType = 10;
                } else if (wParam == 'T') {
                    selectedType = 11;
                } else if (wParam == 'C' && unlockedFactory) {
                    selectedType = 12;
                } else if (wParam == 'G' && unlockedGeo) {
                    selectedType = 13;
                } else if (wParam == 'V' && unlockedBio) {
                    selectedType = 14;
                } else if (wParam == 'E' && unlockedShield) {
                    selectedType = 15;
                } else if (wParam == 'U' && unlockedDrone) {
                    selectedType = 16;
                } else if (wParam == 'P' && unlockedTrade) {
                    selectedType = 17;
                } else if (wParam == 'K' && unlockedCavern) {
                    selectedType = 18;
                } else if (wParam == 'O' && unlockedOrbital) {
                    selectedType = 19;
                } else if (wParam == VK_SPACE) {
                    int orbFound = 0;
                    for (int gi = 0; gi < GRID_W * GRID_H; gi++) if (grid[gi] == 19) orbFound++;
                    if (power >= 40 && orbFound > 0 && alienCount > 0) {
                        power -= 40; PlayGameSound(7); shakeTicks = 30;
                        for (int ai = 0; ai < alienCount; ai++) {
                            SpawnExplosion(OFFSET_X + aliens[ai].x * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + aliens[ai].y * CELL_SIZE + CELL_SIZE/2);
                        }
                        alienCount = 0;
                        strcpy(msgText, "TACTICAL ORBITAL BOMBARDMENT CONFIRMED!");
                        msgTicks = 6;
                    }
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        case WM_TIMER: {
            if (wParam == 2) {
                animFrame++;
                if (shakeTicks > 0) shakeTicks--;
                for (int i = 0; i < particleCount; i++) {
                    particles[i].x += particles[i].vx;
                    particles[i].y += particles[i].vy;
                    if (particles[i].type == PARTICLE_SPARK) {
                        particles[i].vx *= 0.93f;
                        particles[i].vy *= 0.93f;
                    } else if (particles[i].type == PARTICLE_SMOKE) {
                        particles[i].vx *= 0.91f;
                        particles[i].vy *= 0.91f;
                        particles[i].size += 0.10f;
                    } else if (particles[i].type == PARTICLE_DEBRIS) {
                        particles[i].vy += 0.20f; // gravity
                        particles[i].rot += particles[i].vrot;
                        particles[i].vx *= 0.96f;
                    } else if (particles[i].type == PARTICLE_STAR) {
                        particles[i].vx *= 0.94f;
                        particles[i].vy *= 0.94f;
                    }
                    particles[i].life--;
                    if (particles[i].life <= 0) {
                        particles[i] = particles[--particleCount];
                        i--;
                    }
                }
                for (int i = 0; i < shockwaveCount; i++) {
                    shockwaves[i].r1 += shockwaves[i].speed1;
                    shockwaves[i].r2 += shockwaves[i].speed2;
                    shockwaves[i].life--;
                    if (shockwaves[i].life <= 0) {
                        shockwaves[i] = shockwaves[--shockwaveCount];
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
            if (tick % 10 == 0) { 
                day++; 
                isDay = 1; 
            }
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

            int orbitalCount = 0, tradeCount = 0, cavernCount = 0, droneHubCount = 0;
            for (int i = 0; i < GRID_W * GRID_H; i++) {
                if (grid[i] == 19) orbitalCount++;
                if (grid[i] == 17) tradeCount++;
                if (grid[i] == 18) cavernCount++;
                if (grid[i] == 16) droneHubCount++;
            }

            // Random Disaster Events with Orbital Defense Check
            if (rand() % 100 < 6) {
                int r = rand() % 100;
                int targets[GRID_W * GRID_H];
                int targetCount = 0;
                for (int i = 0; i < GRID_W * GRID_H; i++) {
                    if (grid[i] > 0 && grid[i] <= 20) targets[targetCount++] = i;
                }
                
                if (r < 35 && targetCount > 0) { // Meteor Shower
                    if (orbitalCount > 0 && power >= 20) {
                        power -= 20;
                        strcpy(msgText, "ORBITAL DEFENSE INTERCEPTED METEORS!");
                        PlayGameSound(7);
                        msgTicks = 5;
                    } else {
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
                    }
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

            // Trade Port Periodic Freighter Visits
            if (tradeCount > 0 && power > 0 && tick % 10 == 0) {
                freighterDays++;
                if (freighterDays >= 4) {
                    freighterDays = 0;
                    food += 40; mat += 30; advm += 10;
                    strcpy(msgText, "FREIGHTER DOCKED: +40F, +30M, +10A!");
                    PlayGameSound(8);
                    msgTicks = 6;
                }
            }

            // Automated Drone Hub Repairs (Type 16)
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

            // Orbital Beacon Automated Defense Blasts
            if (orbitalCount > 0 && power >= 25 && alienCount > 0 && tick % 2 == 0) {
                power -= 25;
                PlayGameSound(7);
                aliens[0].hp -= 5;
                SpawnExplosion(OFFSET_X + aliens[0].x * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + aliens[0].y * CELL_SIZE + CELL_SIZE/2);
                FireProjectile(OFFSET_X + aliens[0].x * CELL_SIZE + CELL_SIZE/2, 0, OFFSET_X + aliens[0].x * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + aliens[0].y * CELL_SIZE + CELL_SIZE/2, RGB(0, 255, 255));
                strcpy(msgText, "ORBITAL BEACON VAPORIZED ALIEN!");
                msgTicks = 4;
                if (aliens[0].hp <= 0) {
                    aliens[0] = aliens[--alienCount];
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
            pwrProd += cavernCount * 40; // Cavern Drill geothermal magma power
            
            int pwrCons = farmCount * 5 + mineCount * 10 + habCount * 5 + labCount * 5 + hydroCount * 10 + 
                          laserCount * 20 + turretCount * 5 + factoryCount * 10 + bioCount * 20 + shieldCount * 30 + droneHubCount * 25 +
                          tradeCount * 15 + cavernCount * 25 + orbitalCount * 30;
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
            int matProd = (int)(mBase * eff) + (int)(tradeCount * 3 * eff) + (int)(cavernCount * 5 * eff);

            int sciProd = (int)(labCount * 2 * eff);
            if (activeAnomaly == 1) sciProd = (int)(sciProd * 1.5f); // Crystal Monolith

            food += foodProd;
            mat += matProd;
            sci += sciProd;

            // Subterranean Geode anomaly & Cavern Drill minerals
            if (activeAnomaly == 3 && mineCount > 0 && rand() % 4 == 0) {
                advm += mineCount;
            }
            if (cavernCount > 0) advm += cavernCount * 2;
            if (tradeCount > 0) advm += tradeCount;

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

            // Bio-Dome and Trade Port bonus happiness
            if (bioCount > 0 && power > 0) happiness = min(120, happiness + bioCount * 2);
            if (tradeCount > 0 && power > 0) happiness = min(120, happiness + tradeCount * 2);

            int foodCons = pop;
            if (food >= foodCons) {
                food -= foodCons;
                happiness += 5;
                if (happiness > 100 && bioCount == 0 && tradeCount == 0) happiness = 100;
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
                        if (id <= 19 && id >= -1) {
                            selectedType = id;
                        } 
                        // Research Tree Unlocks
                        else if (id == 101 && sci >= 50 && !unlockedHydro) {
                            sci -= 50; unlockedHydro = 1; PlayGameSound(5);
                            SpawnCelebrationStars(OFFSET_X + 200, OFFSET_Y + 200, RGB(170, 255, 0), 20);
                        } else if (id == 104 && sci >= 75 && !unlockedFactory) {
                            sci -= 75; unlockedFactory = 1; PlayGameSound(5);
                            SpawnCelebrationStars(OFFSET_X + 200, OFFSET_Y + 200, RGB(200, 200, 200), 20);
                        } else if (id == 102 && sci >= 100 && !unlockedNuke) {
                            sci -= 100; unlockedNuke = 1; PlayGameSound(5);
                            SpawnCelebrationStars(OFFSET_X + 200, OFFSET_Y + 200, RGB(0, 255, 170), 25);
                        } else if (id == 105 && sci >= 120 && !unlockedSolar4) {
                            sci -= 120; unlockedSolar4 = 1; PlayGameSound(5);
                            SpawnCelebrationStars(OFFSET_X + 200, OFFSET_Y + 200, RGB(255, 255, 0), 25);
                            MessageBox(hwnd, "Solar Grid IV Unlocked! Solar Panels now generate +50% power.", "Tech Breakthrough", MB_OK | MB_ICONINFORMATION);
                        } else if (id == 103 && sci >= 150 && !unlockedLaser) {
                            sci -= 150; unlockedLaser = 1; PlayGameSound(5);
                            SpawnCelebrationStars(OFFSET_X + 200, OFFSET_Y + 200, RGB(255, 50, 50), 25);
                        } else if (id == 106 && sci >= 180 && !unlockedXenoArmor) {
                            sci -= 180; unlockedXenoArmor = 1; PlayGameSound(5);
                            SpawnCelebrationStars(OFFSET_X + 200, OFFSET_Y + 200, RGB(170, 0, 255), 25);
                            MessageBox(hwnd, "Xenobiology Armor Upgraded! Walls now retaliate against aliens and lasers/turrets deal +1 damage.", "Tech Breakthrough", MB_OK | MB_ICONINFORMATION);
                        } else if (id == 107 && sci >= 200 && !unlockedGeo) {
                            sci -= 200; unlockedGeo = 1; PlayGameSound(5);
                            SpawnCelebrationStars(OFFSET_X + 200, OFFSET_Y + 200, RGB(255, 100, 0), 25);
                        } else if (id == 108 && sci >= 250 && !unlockedBio) {
                            sci -= 250; unlockedBio = 1; PlayGameSound(5);
                            SpawnCelebrationStars(OFFSET_X + 200, OFFSET_Y + 200, RGB(0, 255, 180), 25);
                        } else if (id == 109 && sci >= 300 && !unlockedShield) {
                            sci -= 300; unlockedShield = 1; PlayGameSound(5);
                            SpawnCelebrationStars(OFFSET_X + 200, OFFSET_Y + 200, RGB(0, 200, 255), 30);
                        } else if (id == 111 && sci >= 350 && !unlockedTrade) {
                            sci -= 350; unlockedTrade = 1; PlayGameSound(5);
                            SpawnCelebrationStars(OFFSET_X + 200, OFFSET_Y + 200, RGB(255, 200, 0), 30);
                            MessageBox(hwnd, "Trade Port Tech Unlocked! Build Trade Ports to attract freighters and establish interstellar commerce.", "Tech Breakthrough", MB_OK | MB_ICONINFORMATION);
                        } else if (id == 110 && sci >= 400 && !unlockedDrone) {
                            sci -= 400; unlockedDrone = 1; PlayGameSound(5);
                            SpawnCelebrationStars(OFFSET_X + 200, OFFSET_Y + 200, RGB(200, 255, 0), 30);
                        } else if (id == 112 && sci >= 450 && !unlockedCavern) {
                            sci -= 450; unlockedCavern = 1; PlayGameSound(5);
                            SpawnCelebrationStars(OFFSET_X + 200, OFFSET_Y + 200, RGB(255, 140, 50), 30);
                            MessageBox(hwnd, "Cavern Excavator Unlocked! Build Cavern Drills to tap deep geothermal magma and harvest pristine geodes immune to surface disasters.", "Tech Breakthrough", MB_OK | MB_ICONINFORMATION);
                        } else if (id == 113 && sci >= 500 && !unlockedOrbital) {
                            sci -= 500; unlockedOrbital = 1; PlayGameSound(5);
                            SpawnCelebrationStars(OFFSET_X + 200, OFFSET_Y + 200, RGB(0, 255, 255), 35);
                            MessageBox(hwnd, "Orbital Defense Uplink Unlocked! Establish beacons to connect with the fleet, intercept meteors, and call orbital bombardments.", "Tech Breakthrough", MB_OK | MB_ICONINFORMATION);
                        }
                        // Expeditions & Operations
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
                        } else if (id == 204) { // Trade Freighter
                            if (food >= 50 && mat >= 30) {
                                food -= 50; mat -= 30; advm += 25; sci += 100; PlayGameSound(8);
                                MessageBox(hwnd, "Interstellar Trade Contract Executed!\nExported: 50 Food, 30 Mat.\nImported: +25 AdvM, +100 Science!", "Trade Freighter", MB_OK | MB_ICONINFORMATION);
                            } else if (mat >= 60) {
                                mat -= 60; food += 80; power = min(maxPower, power + 40); PlayGameSound(8);
                                MessageBox(hwnd, "Emergency Supplies Imported!\nExported: 60 Mat.\nImported: +80 Food, +40 Power!", "Trade Freighter", MB_OK | MB_ICONINFORMATION);
                            } else {
                                MessageBox(hwnd, "Need 50 Food + 30 Mat (or 60 Mat) for Freighter Trade.", "Trade Error", MB_OK | MB_ICONWARNING);
                            }
                        } else if (id == 205) { // Cavern Dive
                            if (pop >= 2 && mat >= 40 && power >= 40) {
                                mat -= 40; power -= 40; PlayGameSound(9);
                                int r = rand() % 100;
                                if (r < 45) {
                                    advm += 45; sci += 180;
                                    MessageBox(hwnd, "Deep Cavern team excavated a pristine geode vault!\n(+45 AdvM, +180 Science)", "Cavern Discovery", MB_OK | MB_ICONINFORMATION);
                                } else if (r < 80) {
                                    activeAnomaly = 3; mat += 100;
                                    MessageBox(hwnd, "Subterranean Magma Vent tapped! Subterranean Geode active!\n(+100 Mat, Mines yield AdvM)", "Subterranean Discovery", MB_OK | MB_ICONINFORMATION);
                                } else {
                                    pop -= 1; happiness = max(0, happiness - 10);
                                    MessageBox(hwnd, "Seismic collapse inside deep cavern shaft!\n(-1 Pop, -10% Happiness)", "Cavern Hazard", MB_OK | MB_ICONERROR);
                                }
                            } else {
                                MessageBox(hwnd, "Need 2 Pop, 40 Mat, 40 Power for Cavern Dive.", "Expedition Error", MB_OK | MB_ICONWARNING);
                            }
                        } else if (id == 206) { // Orbital Strike
                            int orbFound = 0;
                            for (int gi = 0; gi < GRID_W * GRID_H; gi++) if (grid[gi] == 19) orbFound++;
                            if (power >= 40) {
                                if (orbFound == 0) {
                                    MessageBox(hwnd, "Requires an operational Orbital Beacon on the grid to uplink targeting coordinates!", "Orbital Strike Offline", MB_OK | MB_ICONWARNING);
                                } else if (alienCount == 0) {
                                    MessageBox(hwnd, "Orbital telemetry scans report no hostiles on the grid.", "No Targets", MB_OK | MB_ICONINFORMATION);
                                } else {
                                    power -= 40; PlayGameSound(7); shakeTicks = 30;
                                    for (int ai = 0; ai < alienCount; ai++) {
                                        SpawnExplosion(OFFSET_X + aliens[ai].x * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + aliens[ai].y * CELL_SIZE + CELL_SIZE/2);
                                    }
                                    alienCount = 0;
                                    strcpy(msgText, "TACTICAL ORBITAL BOMBARDMENT CONFIRMED!");
                                    msgTicks = 6;
                                    MessageBox(hwnd, "ORBITAL STRIKE CONFIRMED!\nKinetic orbital barrage obliterated all surface hostiles!", "Orbital Strike", MB_OK | MB_ICONINFORMATION);
                                }
                            } else {
                                MessageBox(hwnd, "Need at least 40 Power to charge Orbital Beacon capacitors.", "Low Power", MB_OK | MB_ICONWARNING);
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
                    else if (selectedType == 17) { costMat = 70; costAdv = 35; costPwr = 30; } // Trade Port
                    else if (selectedType == 18) { costMat = 80; costAdv = 40; costPwr = 40; } // Cavern Drill
                    else if (selectedType == 19) { costMat = 100; costAdv = 50; costPwr = 50; } // Orbital Beacon
                    
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
        0, CLASS_NAME, "KColony - Planetary Tech Expansion [F1: Help]", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
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
