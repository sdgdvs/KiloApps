#include <windows.h>
#include <math.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

void* __cdecl memcpy(void* dest, const void* src, size_t sz) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (sz--) *d++ = *s++;
    return dest;
}

void* __cdecl memmove(void* dest, const void* src, size_t sz) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    if (d < s) {
        while (sz--) *d++ = *s++;
    } else {
        d += sz;
        s += sz;
        while (sz--) *--d = *--s;
    }
    return dest;
}

#define MAX_GRID 6
#define HEADER_HEIGHT 65
#define MARGIN 10

int grid_size = 4;
int grid[MAX_GRID][MAX_GRID];
int frozen[MAX_GRID][MAX_GRID];

int score = 0;
int bestScore = 0;
int gameOver = 0;
int timeOut = 0;
int outOfMoves = 0;
int win = 0;
int hasWon = 0;
HWND mainHwnd = NULL;
static unsigned int seed = 0;
int theme = 1; // 0=Dark, 1=Classic, 2=Pastel

int screenShakeTime = 0;
int mergePop[MAX_GRID][MAX_GRID];
int squashTimer[MAX_GRID][MAX_GRID];
int squashDir[MAX_GRID][MAX_GRID];

int timeAttackEnabled = 0;
int timeRemaining = 60;
int gameStarted = 0;
int timerActive = 0;

int ruleset = 0; // 0=Classic, 1=Fibonacci, 2=Threes

int autoPlayEnabled = 0;
int autoPlayTimerActive = 0;

int obstaclesEnabled = 0;
int frozenTilesEnabled = 0;
int bombsEnabled = 0;
int moveCount = 0;
int movesLeft = 0;
int movesMax = 0;

int campaignMode = 0;
int campaignLevel = 1;

int powerups_shuffles = 5;
int powerups_hammers = 5;
int powerups_rotates = 5;
int powerups_upgrades = 5;
int powerups_undos = 10;

typedef struct {
    int level;
    int size;          // 3, 4, 5, 6
    int target;        // 256 up to 8192
    int movesMax;      // e.g. 60, 80, 100, 150...
    int timeLimit;     // 0 or seconds
    int ruleset;       // 0=Classic, 1=Fib, 2=Threes
    int obstacles;     // count
    int frozenEnabled; // 1 = yes
    int bombsEnabled;  // 1 = yes
    char desc[64];
} CampaignStage;

static const CampaignStage campaignStages[30] = {
    {1,  3,  256,  150,  0, 0, 0, 0, 0, "Stage 1: 3x3 Beginner (256)"},
    {2,  4,  512,  300,  0, 0, 0, 0, 0, "Stage 2: 4x4 Standard (512)"},
    {3,  4,  512,  280,  0, 0, 2, 0, 0, "Stage 3: Stone Blockers (512)"},
    {4,  4,  1024, 600, 90, 0, 0, 0, 0, "Stage 4: Sprint Timer (1024)"},
    {5,  5,  1024, 650,  0, 0, 2, 1, 0, "Stage 5: Frozen Expansion (1024)"},
    {6,  5,  2048, 1200, 0, 0, 0, 0, 1, "Stage 6: Bomb Hazards (2048)"},
    {7,  5,  2584, 2500, 0, 1, 0, 1, 0, "Stage 7: Fibonacci Grid (2584)"},
    {8,  4,  1536, 1100, 0, 2, 2, 0, 0, "Stage 8: Threes Challenge (1536)"},
    {9,  6,  2048, 1300, 0, 0, 4, 0, 0, "Stage 9: Mega 6x6 Field (2048)"},
    {10, 5,  2048, 1250,100, 0, 2, 1, 1, "Stage 10: Frozen & Explosive (2048)"},
    {11, 4,  2048, 1400, 0, 0, 2, 0, 0, "Stage 11: Tight Quarters (2048)"},
    {12, 5,  4096, 2600, 0, 0, 0, 1, 0, "Stage 12: Frostbite 5x5 (4096)"},
    {13, 5,  4096, 2500, 0, 0, 3, 0, 1, "Stage 13: Demolition Zone (4096)"},
    {14, 6,  4181, 4200, 0, 1, 2, 1, 0, "Stage 14: Fibonacci Grand 6x6 (4181)"},
    {15, 5,  3072, 2200, 0, 2, 2, 0, 1, "Stage 15: Threes Mayhem (3072)"},
    {16, 4,  2048, 1100, 60, 0, 0, 0, 0, "Stage 16: Speed Blitz (2048)"},
    {17, 5,  4096, 2700, 0, 0, 4, 1, 0, "Stage 17: Ice & Stone (4096)"},
    {18, 6,  4096, 2800, 0, 0, 3, 1, 1, "Stage 18: Frozen Explosions (4096)"},
    {19, 5,  4096, 2600, 90, 0, 2, 0, 1, "Stage 19: Time Crunch 5x5 (4096)"},
    {20, 6,  8192, 5500, 0, 0, 0, 0, 0, "Stage 20: Titan Field (8192)"},
    {21, 4,  2048, 1300, 0, 0, 3, 1, 0, "Stage 21: Narrow Escape (2048)"},
    {22, 5,  4096, 2800, 0, 0, 3, 1, 1, "Stage 22: Hazard Mayhem (4096)"},
    {23, 5,  6765, 7000, 0, 1, 2, 1, 0, "Stage 23: Fibonacci Master (6765)"},
    {24, 6,  6144, 4500, 0, 2, 4, 0, 1, "Stage 24: Threes Titan (6144)"},
    {25, 5,  8192, 6000, 0, 0, 4, 1, 0, "Stage 25: Fortified Grid (8192)"},
    {26, 6,  8192, 5800, 0, 0, 2, 1, 1, "Stage 26: Glacial Blast 6x6 (8192)"},
    {27, 5,  8192, 5500,120, 0, 3, 1, 1, "Stage 27: Chaos Time Attack (8192)"},
    {28, 6,  8192, 6200, 0, 0, 4, 1, 1, "Stage 28: Minefield 6x6 (8192)"},
    {29, 6, 10946, 12000,150, 1, 3, 1, 1, "Stage 29: Ultimate Trial (10946)"},
    {30, 6,  8192, 5000,180, 0, 4, 1, 1, "Stage 30: 8192 Grandmaster Challenge"}
};

#define MAX_HISTORY 50
typedef struct {
    int grid[MAX_GRID][MAX_GRID];
    int frozen[MAX_GRID][MAX_GRID];
    int score;
    int movesLeft;
} HistoryState;

HistoryState history[MAX_HISTORY];
int historyCount = 0;

int stats_gamesPlayed = 0;
int stats_tilesMerged = 0;
int stats_highestTile = 0;
int stats_timePlayed = 0;

int frameAnimCount = 0;

// Particle Spark Explosion FX
#define MAX_PARTICLES 250
typedef struct {
    float x, y;
    float vx, vy;
    COLORREF color;
    int life;
    int maxLife;
    int size;
} Particle;

Particle particles[MAX_PARTICLES];
int particleCount = 0;

#define MAX_DEBRIS 64
typedef struct {
    float x, y, vx, vy, rot, rotV;
    int val, active;
} Debris;
Debris debris[MAX_DEBRIS];

#define MAX_SHOCKWAVES 5
typedef struct {
    float x, y, radius;
    int active;
} Shockwave;
Shockwave shockwaves[MAX_SHOCKWAVES];

int my_rand() {
    seed = seed * 214013 + 2531011;
    return (seed >> 16) & 0x7FFF;
}

void SpawnMergeParticles(int centerX, int centerY, COLORREF color) {
    int count = 30;
    for (int k = 0; k < count; k++) {
        if (particleCount >= MAX_PARTICLES) break;
        Particle* p = &particles[particleCount++];
        p->x = (float)centerX;
        p->y = (float)centerY;
        if (k < 10) {
            p->vx = (float)((my_rand() % 25) - 12);
            p->vy = (float)((my_rand() % 25) - 15);
            p->color = RGB(255, 255, 255);
            p->life = 8 + (my_rand() % 8);
            p->size = 1 + (my_rand() % 3);
        } else if (k < 20) {
            p->vx = (float)((my_rand() % 15) - 7);
            p->vy = (float)((my_rand() % 15) - 10);
            p->color = color;
            p->life = 15 + (my_rand() % 10);
            p->size = 3 + (my_rand() % 5);
        } else {
            p->vx = (float)((my_rand() % 7) - 3);
            p->vy = (float)((my_rand() % 7) - 5);
            int r = GetRValue(color), g = GetGValue(color), b = GetBValue(color);
            p->color = RGB(max(0, r-50), max(0, g-50), max(0, b-50));
            p->life = 20 + (my_rand() % 15);
            p->size = 4 + (my_rand() % 4);
        }
        p->maxLife = p->life;
    }
}

void UpdateAndDrawParticles(HDC hdc) {
    for (int i = 0; i < particleCount; i++) {
        Particle* p = &particles[i];
        p->x += p->vx;
        p->y += p->vy;
        p->vy += 0.25f; // Gravity
        p->life--;
        if (p->life <= 0) {
            particles[i] = particles[particleCount - 1];
            particleCount--;
            i--;
            continue;
        }
        HBRUSH b = CreateSolidBrush(p->color);
        RECT r = { (int)p->x - p->size/2, (int)p->y - p->size/2, (int)p->x + p->size/2 + 1, (int)p->y + p->size/2 + 1 };
        FillRect(hdc, &r, b);
        DeleteObject(b);
    }
    
    // Draw Debris
    for (int k = 0; k < MAX_DEBRIS; k++) {
        if (debris[k].active) {
            debris[k].vy += 1.5f; // gravity
            debris[k].x += debris[k].vx;
            debris[k].y += debris[k].vy;
            debris[k].rot += debris[k].rotV;
            if (debris[k].y > 800) debris[k].active = 0;
            else {
                int s = (320 / grid_size) / 2 - 2;
                float angle = debris[k].rot * 3.14159f / 180.0f;
                float cosA = cosf(angle);
                float sinA = sinf(angle);
                POINT pts[4];
                pts[0].x = (int)(debris[k].x + (-s)*cosA - (-s)*sinA);
                pts[0].y = (int)(debris[k].y + (-s)*sinA + (-s)*cosA);
                pts[1].x = (int)(debris[k].x + (s)*cosA - (-s)*sinA);
                pts[1].y = (int)(debris[k].y + (s)*sinA + (-s)*cosA);
                pts[2].x = (int)(debris[k].x + (s)*cosA - (s)*sinA);
                pts[2].y = (int)(debris[k].y + (s)*sinA + (s)*cosA);
                pts[3].x = (int)(debris[k].x + (-s)*cosA - (s)*sinA);
                pts[3].y = (int)(debris[k].y + (-s)*sinA + (s)*cosA);

                COLORREF c = GetTileColor(debris[k].val);
                HBRUSH b = CreateSolidBrush(c);
                HPEN p = CreatePen(PS_SOLID, 1, RGB(0,0,0));
                HBRUSH oldB = (HBRUSH)SelectObject(hdc, b);
                HPEN oldP = (HPEN)SelectObject(hdc, p);
                Polygon(hdc, pts, 4);
                SelectObject(hdc, oldB);
                SelectObject(hdc, oldP);
                DeleteObject(b);
                DeleteObject(p);
            }
        }
    }
    
    // Draw Shockwaves
    for (int k = 0; k < MAX_SHOCKWAVES; k++) {
        if (shockwaves[k].active) {
            shockwaves[k].radius += 20.0f;
            if (shockwaves[k].radius > 600.0f) shockwaves[k].active = 0;
            else {
                float progress = shockwaves[k].radius / 600.0f;
                int thickness = (int)(15.0f * (1.0f - progress));
                if (thickness < 1) thickness = 1;

                int cx = (int)shockwaves[k].x;
                int cy = (int)shockwaves[k].y;
                int r = (int)shockwaves[k].radius;
                
                HPEN penR = CreatePen(PS_SOLID, thickness, RGB(255, 50, 50));
                HPEN oldP = (HPEN)SelectObject(hdc, penR);
                SelectObject(hdc, GetStockObject(NULL_BRUSH));
                Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
                
                HPEN penG = CreatePen(PS_SOLID, thickness, RGB(50, 255, 50));
                SelectObject(hdc, penG);
                int rG = (int)(shockwaves[k].radius * 0.95f);
                Ellipse(hdc, cx - rG, cy - rG, cx + rG, cy + rG);
                
                HPEN penB = CreatePen(PS_SOLID, thickness, RGB(50, 50, 255));
                SelectObject(hdc, penB);
                int rB = (int)(shockwaves[k].radius * 0.9f);
                Ellipse(hdc, cx - rB, cy - rB, cx + rB, cy + rB);
                
                SelectObject(hdc, oldP);
                DeleteObject(penR);
                DeleteObject(penG);
                DeleteObject(penB);
            }
        }
    }
}

COLORREF GetTileColor(int val) {
    int rCol = 200, gCol = 200, bCol = 200;
    int normalized = val;
    if (ruleset == 1 && val > 0) {
        if (val == 1) normalized = 2;
        else if (val == 3) normalized = 4;
        else if (val == 5) normalized = 8;
        else if (val == 13) normalized = 16;
        else if (val == 21) normalized = 32;
        else if (val == 34) normalized = 64;
        else if (val == 55) normalized = 128;
        else if (val == 89) normalized = 256;
        else if (val == 144) normalized = 512;
        else if (val == 233) normalized = 1024;
        else if (val >= 377) normalized = 2048;
    } else if (ruleset == 2 && val > 0) {
        if (val == 1) normalized = 2;
        else if (val == 2) normalized = 4;
        else if (val == 3) normalized = 8;
        else if (val == 6) normalized = 16;
        else if (val == 12) normalized = 32;
        else if (val == 24) normalized = 64;
        else if (val == 48) normalized = 128;
        else if (val == 96) normalized = 256;
        else if (val == 192) normalized = 512;
        else if (val == 384) normalized = 1024;
        else if (val >= 768) normalized = 2048;
    }

    if (theme == 0) { // Dark / Metallic
        if (normalized == 2) { rCol=75; gCol=70; bCol=85; }
        else if (normalized == 4) { rCol=95; gCol=90; bCol=110; }
        else if (normalized == 8) { rCol=242; gCol=177; bCol=121; }
        else if (normalized == 16) { rCol=245; gCol=149; bCol=99; }
        else if (normalized == 32) { rCol=246; gCol=124; bCol=95; }
        else if (normalized == 64) { rCol=246; gCol=94; bCol=59; }
        else if (normalized == 128) { rCol=237; gCol=207; bCol=114; }
        else if (normalized == 256) { rCol=237; gCol=204; bCol=97; }
        else if (normalized == 512) { rCol=70; gCol=200; bCol=120; }
        else if (normalized == 1024) { rCol=70; gCol=170; bCol=240; }
        else if (normalized == 2048) { rCol=240; gCol=190; bCol=40; }
        else if (normalized > 2048) { rCol=170; gCol=70; bCol=240; }
        if (val == -3) { rCol=220; gCol=30; bCol=30; }
        if (val == -2) { rCol=255; gCol=215; bCol=0; }
        if (val == -1) { rCol=90; gCol=90; bCol=95; }
        if (val == 0) { rCol = 45; gCol = 45; bCol = 50; }
    } else if (theme == 1) { // Classic
        if (normalized == 2) { rCol=238; gCol=228; bCol=218; }
        else if (normalized == 4) { rCol=237; gCol=224; bCol=200; }
        else if (normalized == 8) { rCol=242; gCol=177; bCol=121; }
        else if (normalized == 16) { rCol=245; gCol=149; bCol=99; }
        else if (normalized == 32) { rCol=246; gCol=124; bCol=95; }
        else if (normalized == 64) { rCol=246; gCol=94; bCol=59; }
        else if (normalized == 128) { rCol=237; gCol=207; bCol=114; }
        else if (normalized == 256) { rCol=237; gCol=204; bCol=97; }
        else if (normalized == 512) { rCol=90; gCol=200; bCol=130; }
        else if (normalized == 1024) { rCol=70; gCol=180; bCol=230; }
        else if (normalized == 2048) { rCol=240; gCol=195; bCol=40; }
        else if (normalized > 2048) { rCol=180; gCol=80; bCol=240; }
        if (val == -3) { rCol=220; gCol=30; bCol=30; }
        if (val == -2) { rCol=255; gCol=215; bCol=0; }
        if (val == -1) { rCol=100; gCol=100; bCol=100; }
        if (val == 0) { rCol = 204; gCol = 192; bCol = 179; }
    } else { // Pastel
        if (normalized == 2) { rCol=225; gCol=190; bCol=231; }
        else if (normalized == 4) { rCol=209; gCol=196; bCol=233; }
        else if (normalized == 8) { rCol=197; gCol=202; bCol=233; }
        else if (normalized == 16) { rCol=187; gCol=222; bCol=251; }
        else if (normalized == 32) { rCol=179; gCol=229; bCol=252; }
        else if (normalized == 64) { rCol=178; gCol=235; bCol=242; }
        else if (normalized == 128) { rCol=178; gCol=223; bCol=219; }
        else if (normalized == 256) { rCol=200; gCol=230; bCol=201; }
        else if (normalized == 512) { rCol=160; gCol=230; bCol=175; }
        else if (normalized == 1024) { rCol=150; gCol=210; bCol=250; }
        else if (normalized == 2048) { rCol=255; gCol=225; bCol=130; }
        else if (normalized > 2048) { rCol=210; gCol=160; bCol=250; }
        if (val == -3) { rCol=240; gCol=100; bCol=100; }
        if (val == -2) { rCol=255; gCol=215; bCol=0; }
        if (val == -1) { rCol=150; gCol=150; bCol=160; }
        if (val == 0) { rCol = 245; gCol = 240; bCol = 245; }
    }
    return RGB(rCol, gCol, bCol);
}

void DrawBadgeIcon(HDC hdc, int x, int y, int size, int val) {
    int normalized = val;
    if (ruleset == 1 && val > 0) {
        if (val >= 377) normalized = 2048;
        else if (val >= 233) normalized = 1024;
        else if (val >= 144) normalized = 512;
        else if (val >= 89) normalized = 256;
    } else if (ruleset == 2 && val > 0) {
        if (val >= 768) normalized = 2048;
        else if (val >= 384) normalized = 1024;
        else if (val >= 192) normalized = 512;
        else if (val >= 96) normalized = 256;
    }

    if (normalized >= 2048) {
        POINT points[7];
        int w = size / 4;
        int h = size / 5;
        int px = x + size - w - 6;
        int py = y + 6;

        points[0].x = px;           points[0].y = py + h;
        points[1].x = px;           points[1].y = py;
        points[2].x = px + w / 3;   points[2].y = py + h / 2;
        points[3].x = px + w / 2;   points[3].y = py - 2;
        points[4].x = px + 2*w / 3; points[4].y = py + h / 2;
        points[5].x = px + w;       points[5].y = py;
        points[6].x = px + w;       points[6].y = py + h;

        HBRUSH crownBrush = CreateSolidBrush(RGB(255, 220, 0));
        HPEN crownPen = CreatePen(PS_SOLID, 1, RGB(180, 140, 0));
        HBRUSH oldB = (HBRUSH)SelectObject(hdc, crownBrush);
        HPEN oldP = (HPEN)SelectObject(hdc, crownPen);
        Polygon(hdc, points, 7);
        SelectObject(hdc, oldB);
        SelectObject(hdc, oldP);
        DeleteObject(crownBrush);
        DeleteObject(crownPen);
    } else if (normalized == 1024) {
        POINT points[4];
        int w = size / 5;
        int px = x + size - w - 6;
        int py = y + 6 + w/2;

        points[0].x = px;        points[0].y = py;
        points[1].x = px + w/2;  points[1].y = py - w/2;
        points[2].x = px + w;    points[2].y = py;
        points[3].x = px + w/2;  points[3].y = py + w/2;

        HBRUSH diaBrush = CreateSolidBrush(RGB(180, 240, 255));
        HPEN diaPen = CreatePen(PS_SOLID, 1, RGB(50, 150, 220));
        HBRUSH oldB = (HBRUSH)SelectObject(hdc, diaBrush);
        HPEN oldP = (HPEN)SelectObject(hdc, diaPen);
        Polygon(hdc, points, 4);
        SelectObject(hdc, oldB);
        SelectObject(hdc, oldP);
        DeleteObject(diaBrush);
        DeleteObject(diaPen);
    } else if (val == -3) { // Bomb
        int r = size / 8;
        int cx = x + size / 2;
        int cy = y + size / 3;
        
        int pulse = (frameAnimCount % 10) - 5;
        if (pulse < 0) pulse = -pulse;
        int redVal = 20 + pulse * 15;
        
        HBRUSH bBrush = CreateSolidBrush(RGB(redVal, 20, 20));
        HPEN bPen = CreatePen(PS_SOLID, 1, RGB(255, 60, 0));
        HBRUSH oldB = (HBRUSH)SelectObject(hdc, bBrush);
        HPEN oldP = (HPEN)SelectObject(hdc, bPen);
        Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);

        MoveToEx(hdc, cx, cy - r, NULL);
        int fuseX = cx + r/2;
        int fuseY = cy - r - 4;
        LineTo(hdc, fuseX, fuseY);
        
        if (frameAnimCount % 3 == 0) {
            SetPixel(hdc, fuseX, fuseY - 1, RGB(255, 255, 0));
            SetPixel(hdc, fuseX + 1, fuseY, RGB(255, 200, 0));
            SetPixel(hdc, fuseX - 1, fuseY, RGB(255, 100, 0));
        } else {
            SetPixel(hdc, fuseX, fuseY - 1, RGB(255, 150, 0));
        }

        SelectObject(hdc, oldB);
        SelectObject(hdc, oldP);
        DeleteObject(bBrush);
        DeleteObject(bPen);
    } else if (val == -2) { // Wildcard Star
        int cx = x + size / 2;
        int cy = y + size / 3;
        int s = size / 7;
        
        int pulse = (frameAnimCount % 8) - 4;
        if (pulse < 0) pulse = -pulse;
        s += pulse / 2;
        
        POINT star[10] = {
            {cx, cy - s}, {cx + s/3, cy - s/3}, {cx + s, cy - s/3}, {cx + s/2, cy + s/4},
            {cx + 2*s/3, cy + s}, {cx, cy + s/2}, {cx - 2*s/3, cy + s}, {cx - s/2, cy + s/4},
            {cx - s, cy - s/3}, {cx - s/3, cy - s/3}
        };
        HBRUSH sBrush = CreateSolidBrush(RGB(255, 230, 50));
        HPEN sPen = CreatePen(PS_SOLID, 1, RGB(200, 150, 0));
        HBRUSH oldB = (HBRUSH)SelectObject(hdc, sBrush);
        HPEN oldP = (HPEN)SelectObject(hdc, sPen);
        Polygon(hdc, star, 10);
        SelectObject(hdc, oldB);
        SelectObject(hdc, oldP);
        DeleteObject(sBrush);
        DeleteObject(sPen);
    } else if (val == -1) { // Obstacle
        int cx = x + size / 2;
        int cy = y + size / 2;
        int r = size / 6;
        HPEN p = CreatePen(PS_SOLID, 3, RGB(220, 60, 60));
        HPEN oldP = (HPEN)SelectObject(hdc, p);
        MoveToEx(hdc, cx - r, cy - r, NULL);
        LineTo(hdc, cx + r, cy + r);
        MoveToEx(hdc, cx + r, cy - r, NULL);
        LineTo(hdc, cx - r, cy + r);
        SelectObject(hdc, oldP);
        DeleteObject(p);
    }
}

void DrawCell(HDC hdc, int x, int y, int val, int cell_size, int isFrozen, int popVal, int sqTimer, int sqDir) {
    COLORREF baseColor = GetTileColor(val);
    int rCol = GetRValue(baseColor);
    int gCol = GetGValue(baseColor);
    int bCol = GetBValue(baseColor);
    int txtCol = 0x000000;

    int originalVal = val;
    int normalized = val;
    if (ruleset == 1 && val > 0) {
        if (val == 1) normalized = 2;
        else if (val == 3) normalized = 4;
        else if (val == 5) normalized = 8;
        else if (val == 13) normalized = 16;
        else if (val == 21) normalized = 32;
        else if (val == 34) normalized = 64;
        else if (val == 55) normalized = 128;
        else if (val == 89) normalized = 256;
        else if (val == 144) normalized = 512;
        else if (val == 233) normalized = 1024;
        else if (val >= 377) normalized = 2048;
    } else if (ruleset == 2 && val > 0) {
        if (val == 1) normalized = 2;
        else if (val == 2) normalized = 4;
        else if (val == 3) normalized = 8;
        else if (val == 6) normalized = 16;
        else if (val == 12) normalized = 32;
        else if (val == 24) normalized = 64;
        else if (val == 48) normalized = 128;
        else if (val == 96) normalized = 256;
        else if (val == 192) normalized = 512;
        else if (val == 384) normalized = 1024;
        else if (val >= 768) normalized = 2048;
    }

    if (normalized == 2 || normalized == 4) {
        txtCol = (theme == 1) ? RGB(119,110,101) : ((theme == 2) ? RGB(74,20,140) : RGB(220,220,220));
    } else {
        txtCol = (theme == 2) ? RGB(74,20,140) : RGB(255,255,255);
    }

    int tileMargin = 4;
    if (popVal > 0) tileMargin = 1;

    int rx1 = x + tileMargin;
    int ry1 = y + tileMargin;
    int rx2 = x + cell_size - tileMargin;
    int ry2 = y + cell_size - tileMargin;

    if (sqTimer > 0) {
        int squashAmt = (sqTimer <= 3) ? sqTimer * 2 : (6 - sqTimer) * 2;
        if (sqDir == 1) {
            rx1 += squashAmt;
            rx2 -= squashAmt;
            ry1 -= squashAmt / 2;
            ry2 += squashAmt / 2;
        } else if (sqDir == 2) {
            rx1 -= squashAmt / 2;
            rx2 += squashAmt / 2;
            ry1 += squashAmt;
            ry2 -= squashAmt;
        }
    }

    RECT r = { rx1, ry1, rx2, ry2 };

    if (val == 0) {
        HBRUSH bg = CreateSolidBrush(baseColor);
        FillRect(hdc, &r, bg);
        DeleteObject(bg);

        HPEN darkPen = CreatePen(PS_SOLID, 2, RGB(max(0, rCol - 30), max(0, gCol - 30), max(0, bCol - 30)));
        HPEN oldP = (HPEN)SelectObject(hdc, darkPen);
        MoveToEx(hdc, rx1, ry2, NULL);
        LineTo(hdc, rx1, ry1);
        LineTo(hdc, rx2, ry1);
        SelectObject(hdc, oldP);
        DeleteObject(darkPen);
        return;
    }

    // Dynamic Drop Shadow (Pop Animation)
    if (val > 0) {
        int shOff = 2 + popVal * 3;
        RECT sr = { rx1 + shOff, ry1 + shOff, rx2 + shOff, ry2 + shOff };
        HBRUSH shb = CreateSolidBrush(RGB(20, 20, 20));
        FillRect(hdc, &sr, shb);
        DeleteObject(shb);
    }

    // Glowing Aura for Milestone Tiles
    if (normalized >= 2048) {
        int pulse = (frameAnimCount % 12) - 6;
        if (pulse < 0) pulse = -pulse;
        int auraSpread = 3 + pulse + popVal * 2;
        RECT auraRect = { rx1 - auraSpread, ry1 - auraSpread, rx2 + auraSpread, ry2 + auraSpread };
        HBRUSH auraB = CreateSolidBrush(RGB(255, 215, 0));
        FillRect(hdc, &auraRect, auraB);
        DeleteObject(auraB);
    } else if (normalized >= 1024) {
        RECT auraRect = { rx1 - 3, ry1 - 3, rx2 + 3, ry2 + 3 };
        HBRUSH auraB = CreateSolidBrush(RGB(80, 180, 255));
        FillRect(hdc, &auraRect, auraB);
        DeleteObject(auraB);
    }

    // 3D Tile Fill
    HBRUSH bg = CreateSolidBrush(baseColor);
    FillRect(hdc, &r, bg);
    DeleteObject(bg);

    // Textures
    if (val > 0) {
        if (normalized <= 64) {
            // Wood Texture
            HPEN woodPen = CreatePen(PS_SOLID, 1, RGB(max(0, rCol-30), max(0, gCol-30), max(0, bCol-30)));
            HPEN oldPW = (HPEN)SelectObject(hdc, woodPen);
            for (int wy = ry1 + 6; wy < ry2; wy += 8) {
                MoveToEx(hdc, rx1 + 2, wy, NULL);
                LineTo(hdc, rx2 - 2, wy);
            }
            SelectObject(hdc, oldPW);
            DeleteObject(woodPen);
        } else if (normalized <= 1024) {
            // Stone / Marble Texture
            for (int k = 0; k < 20; k++) {
                int px = rx1 + (my_rand() % (rx2 - rx1));
                int py = ry1 + (my_rand() % (ry2 - ry1));
                SetPixel(hdc, px, py, RGB(max(0, rCol-50), max(0, gCol-50), max(0, bCol-50)));
                SetPixel(hdc, px+1, py, RGB(min(255, rCol+50), min(255, gCol+50), min(255, bCol+50)));
            }
        } else {
            // Gem / Neon Texture
            HPEN gemPen = CreatePen(PS_SOLID, 2, RGB(min(255, rCol+80), min(255, gCol+80), min(255, bCol+80)));
            HPEN oldPG = (HPEN)SelectObject(hdc, gemPen);
            int cx = (rx1 + rx2) / 2;
            int cy = (ry1 + ry2) / 2;
            MoveToEx(hdc, rx1, ry1, NULL); LineTo(hdc, cx, cy);
            MoveToEx(hdc, rx2, ry1, NULL); LineTo(hdc, cx, cy);
            MoveToEx(hdc, rx1, ry2, NULL); LineTo(hdc, cx, cy);
            MoveToEx(hdc, rx2, ry2, NULL); LineTo(hdc, cx, cy);
            SelectObject(hdc, oldPG);
            DeleteObject(gemPen);
        }
    }

    // 3D Top/Left Light Highlight Bevel
    int hlR = min(255, rCol + 70);
    int hlG = min(255, gCol + 70);
    int hlB = min(255, bCol + 70);
    HPEN lightPen = CreatePen(PS_SOLID, 3, RGB(hlR, hlG, hlB));
    HPEN oldPen = (HPEN)SelectObject(hdc, lightPen);
    MoveToEx(hdc, rx1 + 1, ry2 - 2, NULL);
    LineTo(hdc, rx1 + 1, ry1 + 1);
    LineTo(hdc, rx2 - 2, ry1 + 1);

    // 3D Bottom/Right Dark Shadow Bevel
    int shR = max(0, rCol - 70);
    int shG = max(0, gCol - 70);
    int shB = max(0, bCol - 70);
    HPEN darkPen = CreatePen(PS_SOLID, 3, RGB(shR, shG, shB));
    SelectObject(hdc, darkPen);
    MoveToEx(hdc, rx2 - 2, ry1 + 2, NULL);
    LineTo(hdc, rx2 - 2, ry2 - 2);
    LineTo(hdc, rx1 + 2, ry2 - 2);

    // Glossy metallic sheen line
    HPEN sheenPen = CreatePen(PS_SOLID, 1, RGB(min(255, rCol + 90), min(255, gCol + 90), min(255, bCol + 90)));
    SelectObject(hdc, sheenPen);
    MoveToEx(hdc, rx1 + 4, ry1 + 4, NULL);
    LineTo(hdc, rx2 - 4, ry1 + 4);

    SelectObject(hdc, oldPen);
    DeleteObject(lightPen);
    DeleteObject(darkPen);
    DeleteObject(sheenPen);

    if (popVal > 0) {
        HRGN hRgn = CreateRectRgn(rx1, ry1, rx2, ry2);
        SelectClipRgn(hdc, hRgn);
        int sweepOffset = (10 - popVal) * ((rx2 - rx1 + 60) / 10);
        int sx = rx1 - 30 + sweepOffset;
        HPEN sweepPen2 = CreatePen(PS_SOLID, 6, RGB(min(255, rCol + 150), min(255, gCol + 150), min(255, bCol + 150)));
        HPEN sweepPen1 = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
        HPEN oldSweep = (HPEN)SelectObject(hdc, sweepPen2);
        MoveToEx(hdc, sx + 10, ry1 - 5, NULL);
        LineTo(hdc, sx - 20, ry2 + 5);
        SelectObject(hdc, sweepPen1);
        MoveToEx(hdc, sx + 25, ry1 - 5, NULL);
        LineTo(hdc, sx - 5, ry2 + 5);
        SelectObject(hdc, oldSweep);
        DeleteObject(sweepPen1);
        DeleteObject(sweepPen2);
        SelectClipRgn(hdc, NULL);
        DeleteObject(hRgn);
    }

    // Draw Frozen Ice Border Overlay
    if (isFrozen && val > 0) {
        HPEN icePen = CreatePen(PS_SOLID, 3, RGB(100, 220, 255));
        HPEN oldP = (HPEN)SelectObject(hdc, icePen);
        MoveToEx(hdc, rx1, ry1, NULL);
        LineTo(hdc, rx2, ry1);
        LineTo(hdc, rx2, ry2);
        LineTo(hdc, rx1, ry2);
        LineTo(hdc, rx1, ry1);
        SelectObject(hdc, oldP);
        DeleteObject(icePen);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(0, 230, 255));
        HFONT iceFont = CreateFontA(10, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
        HFONT oFont = (HFONT)SelectObject(hdc, iceFont);
        RECT iceRect = { rx1 + 3, ry1 + 2, rx1 + 30, ry1 + 14 };
        DrawTextA(hdc, "ICE", -1, &iceRect, DT_LEFT | DT_TOP | DT_SINGLELINE);
        SelectObject(hdc, oFont);
        DeleteObject(iceFont);
    }

    // Draw Badge Icon if applicable
    DrawBadgeIcon(hdc, rx1, ry1, cell_size - tileMargin*2, originalVal);

    // Draw Text Value with Drop Shadow
    if (originalVal != 0) {
        SetBkMode(hdc, TRANSPARENT);
        char buf[16];
        if (originalVal == -3) { buf[0] = 'B'; buf[1] = 0; }
        else if (originalVal == -2) { buf[0] = 'W'; buf[1] = 0; }
        else if (originalVal == -1) { buf[0] = 'X'; buf[1] = 0; }
        else {
            int temp = originalVal;
            int len = 0;
            char rev[16];
            int rlen = 0;
            while(temp > 0) { rev[rlen++] = '0' + (temp % 10); temp /= 10; }
            while(rlen > 0) buf[len++] = rev[--rlen];
            buf[len] = 0;
        }

        int fontSize = cell_size / 3;
        if (fontSize < 12) fontSize = 12;
        HFONT hFont = CreateFontA(fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
        HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

        // Text Drop Shadow
        RECT rShadow = { r.left + 1, r.top + 1, r.right + 1, r.bottom + 1 };
        SetTextColor(hdc, RGB(20, 20, 20));
        DrawTextA(hdc, buf, -1, &rShadow, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // Main Text
        SetTextColor(hdc, txtCol);
        DrawTextA(hdc, buf, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        SelectObject(hdc, oldFont);
        DeleteObject(hFont);
    }
}

void LoadBest() {
    char filename[32];
    if (ruleset == 1) {
        wsprintfA(filename, "k2048_score_%d_fib.dat", grid_size);
    } else {
        wsprintfA(filename, "k2048_score_%d.dat", grid_size);
    }
    HANDLE hFile = CreateFileA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE && grid_size == 4 && ruleset == 0) {
        hFile = CreateFileA("k2048_score.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[64] = {0};
        DWORD bytesRead;
        if (ReadFile(hFile, buf, sizeof(buf)-1, &bytesRead, NULL)) {
            bestScore = 0;
            for (int i = 0; buf[i] >= '0' && buf[i] <= '9'; i++) {
                bestScore = bestScore * 10 + (buf[i] - '0');
            }
        }
        CloseHandle(hFile);
    }
}

void LoadTheme() {
    HANDLE hFile = CreateFileA("k2048_theme.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[16] = {0};
        DWORD bytesRead;
        if (ReadFile(hFile, buf, sizeof(buf)-1, &bytesRead, NULL)) {
            theme = buf[0] - '0';
        }
        CloseHandle(hFile);
    }
    if (theme < 0 || theme > 2) theme = 1;
}

void LoadStats() {
    HANDLE hFile = CreateFileA("k2048_stats.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesRead;
        ReadFile(hFile, &stats_gamesPlayed, sizeof(int), &bytesRead, NULL);
        ReadFile(hFile, &stats_tilesMerged, sizeof(int), &bytesRead, NULL);
        ReadFile(hFile, &stats_highestTile, sizeof(int), &bytesRead, NULL);
        ReadFile(hFile, &stats_timePlayed, sizeof(int), &bytesRead, NULL);
        CloseHandle(hFile);
    }
}

void SaveBest() {
    char filename[32];
    if (ruleset == 1) {
        wsprintfA(filename, "k2048_score_%d_fib.dat", grid_size);
    } else {
        wsprintfA(filename, "k2048_score_%d.dat", grid_size);
    }
    HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[64];
        int temp = bestScore;
        int len = 0;
        if (temp == 0) buf[len++] = '0';
        else {
            char rev[64];
            int rlen = 0;
            while(temp > 0) { rev[rlen++] = '0' + (temp % 10); temp /= 10; }
            while(rlen > 0) buf[len++] = rev[--rlen];
        }
        buf[len] = 0;
        DWORD bytesWritten;
        WriteFile(hFile, buf, len, &bytesWritten, NULL);
        CloseHandle(hFile);
    }
}

void SaveTheme() {
    HANDLE hFile = CreateFileA("k2048_theme.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[2];
        buf[0] = '0' + theme;
        buf[1] = 0;
        DWORD bytesWritten;
        WriteFile(hFile, buf, 1, &bytesWritten, NULL);
        CloseHandle(hFile);
    }
}

void SaveStats() {
    HANDLE hFile = CreateFileA("k2048_stats.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten;
        WriteFile(hFile, &stats_gamesPlayed, sizeof(int), &bytesWritten, NULL);
        WriteFile(hFile, &stats_tilesMerged, sizeof(int), &bytesWritten, NULL);
        WriteFile(hFile, &stats_highestTile, sizeof(int), &bytesWritten, NULL);
        WriteFile(hFile, &stats_timePlayed, sizeof(int), &bytesWritten, NULL);
        CloseHandle(hFile);
    }
}

void AddRandomTile() {
    int emptyCount = 0;
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            if (grid[i][j] == 0) emptyCount++;
        }
    }
    if (emptyCount == 0) return;
    int target = my_rand() % emptyCount;
    emptyCount = 0;
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            if (grid[i][j] == 0) {
                if (emptyCount == target) {
                    if (obstaclesEnabled && moveCount > 0 && moveCount % 5 == 0) {
                        grid[i][j] = -1;
                        frozen[i][j] = 0;
                    } else if (bombsEnabled && my_rand() % 25 == 0) {
                        grid[i][j] = -3; // Bomb
                        frozen[i][j] = 0;
                    } else if (my_rand() % 20 == 0) {
                        grid[i][j] = -2; // Wildcard
                        frozen[i][j] = 0;
                    } else {
                        if (ruleset == 1) grid[i][j] = 1;
                        else if (ruleset == 2) grid[i][j] = (my_rand() % 2 == 0) ? 1 : 2;
                        else grid[i][j] = ((my_rand() % 10 == 0) ? 4 : 2);

                        if (frozenTilesEnabled && (my_rand() % 4 == 0)) {
                            frozen[i][j] = 1;
                        } else {
                            frozen[i][j] = 0;
                        }

                        if (grid[i][j] > stats_highestTile) {
                            stats_highestTile = grid[i][j];
                            SaveStats();
                        }
                    }
                    return;
                }
                emptyCount++;
            }
        }
    }
}

void InitGame() {
    // Generate cascading debris
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            if (grid[i][j] != 0) {
                for (int k = 0; k < MAX_DEBRIS; k++) {
                    if (!debris[k].active) {
                        debris[k].active = 1;
                        debris[k].val = grid[i][j];
                        debris[k].x = MARGIN + 8 + j * (320 / grid_size) + (320 / grid_size) / 2.0f;
                        debris[k].y = HEADER_HEIGHT + 8 + i * (320 / grid_size) + (320 / grid_size) / 2.0f;
                        debris[k].vx = (float)((my_rand() % 31) - 15);
                        debris[k].vy = (float)((my_rand() % 15) - 15);
                        debris[k].rot = 0;
                        debris[k].rotV = (float)((my_rand() % 31) - 15);
                        break;
                    }
                }
            }
        }
    }

    memset(grid, 0, sizeof(grid));
    memset(frozen, 0, sizeof(frozen));
    memset(mergePop, 0, sizeof(mergePop));
    memset(squashTimer, 0, sizeof(squashTimer));
    memset(squashDir, 0, sizeof(squashDir));
    score = 0;
    gameOver = 0;
    timeOut = 0;
    outOfMoves = 0;
    win = 0;
    hasWon = 0;
    historyCount = 0;
    gameStarted = 0;
    moveCount = 0;
    timeRemaining = 60;
    movesLeft = movesMax;
    powerups_shuffles = 5;
    powerups_hammers = 5;
    powerups_rotates = 5;
    powerups_upgrades = 5;
    powerups_undos = 10;
    particleCount = 0;
    if (timerActive) {
        KillTimer(mainHwnd, 1);
        timerActive = 0;
    }
    seed = GetTickCount();
    AddRandomTile();
    AddRandomTile();
    LoadBest();
}

void StartCampaignLevel() {
    if (campaignLevel < 1) campaignLevel = 1;
    if (campaignLevel > 30) campaignLevel = 30;

    const CampaignStage* s = &campaignStages[campaignLevel - 1];
    grid_size = s->size;
    timeAttackEnabled = (s->timeLimit > 0) ? 1 : 0;
    timeRemaining = s->timeLimit;
    obstaclesEnabled = (s->obstacles > 0) ? 1 : 0;
    frozenTilesEnabled = s->frozenEnabled;
    bombsEnabled = s->bombsEnabled;
    ruleset = s->ruleset;
    movesMax = s->movesMax;

    InitGame();

    if (s->obstacles > 0) {
        for (int k = 0; k < s->obstacles; k++) {
            int r = my_rand() % grid_size;
            int c = my_rand() % grid_size;
            if (grid[r][c] == 0) {
                grid[r][c] = -1;
            }
        }
    }
}

int GetMergeResult(int a, int b) {
    if (a == -1 || b == -1) return 0;
    if ((a == -3 && b > 0) || (b == -3 && a > 0)) return -100; // Bomb explosion
    if (a == -2 && b > 0) {
        if (ruleset == 1) {
            int fibs[] = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181, 6765, 10946, 17711, 28657, 46368, 75025};
            for (int i = 0; i < 23; i++) if (fibs[i] == b) return fibs[i+1];
            return b;
        } else if (ruleset == 2) {
            if (b == 1 || b == 2) return 3;
            return b * 2;
        }
        return b * 2;
    }
    if (b == -2 && a > 0) return GetMergeResult(b, a);
    if (a == -2 && b == -2) return 2;

    if (ruleset == 1) {
        if (a == 1 && b == 1) return 2;
        int fibs[] = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181, 6765, 10946, 17711, 28657, 46368, 75025};
        int idxA = -1, idxB = -1;
        for (int i = 0; i < 24; i++) {
            if (fibs[i] == a) idxA = i;
            if (fibs[i] == b) idxB = i;
        }
        if (idxA >= 0 && idxB >= 0) {
            int diff = idxA - idxB;
            if (diff == 1 || diff == -1) return a + b;
        }
        return 0;
    } else if (ruleset == 2) {
        if ((a == 1 && b == 2) || (a == 2 && b == 1)) return 3;
        if (a >= 3 && a == b) return a * 2;
        return 0;
    } else {
        if (a == b) return a * 2;
        return 0;
    }
}

int CheckGameOver() {
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            if (grid[i][j] == 0) return 0;
            if (i < grid_size - 1 && GetMergeResult(grid[i][j], grid[i+1][j]) > 0) return 0;
            if (j < grid_size - 1 && GetMergeResult(grid[i][j], grid[i][j+1]) > 0) return 0;
        }
    }
    return 1;
}

void DoTileUpgrade() {
    if (powerups_upgrades <= 0 || gameOver || win) return;
    int minVal = 999999;
    int minI = -1, minJ = -1;
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            if (grid[i][j] > 0 && grid[i][j] < minVal) {
                minVal = grid[i][j];
                minI = i;
                minJ = j;
            }
        }
    }
    if (minI != -1) {
        powerups_upgrades--;
        int nextVal = minVal * 2;
        if (ruleset == 1) {
            int fibs[] = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181, 6765, 10946, 17711, 28657, 46368, 75025};
            for(int f=0; f<23; f++) if(fibs[f] == minVal) { nextVal = fibs[f+1]; break; }
        } else if (ruleset == 2) {
            if (minVal == 1 || minVal == 2) nextVal = 3;
            else nextVal = minVal * 2;
        }
        grid[minI][minJ] = nextVal;
        frozen[minI][minJ] = 0;
        int cell_size = 320 / grid_size;
        int px = MARGIN + 8 + minJ * cell_size + cell_size / 2;
        int py = HEADER_HEIGHT + 8 + minI * cell_size + cell_size / 2;
        SpawnMergeParticles(px, py, RGB(255, 215, 0));
        Beep(880, 40);
        InvalidateRect(mainHwnd, NULL, TRUE);
    }
}

void DoGridRotate() {
    if (powerups_rotates <= 0 || gameOver || win) return;
    powerups_rotates--;
    int tempGrid[MAX_GRID][MAX_GRID];
    int tempFrozen[MAX_GRID][MAX_GRID];
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            tempGrid[j][grid_size - 1 - i] = grid[i][j];
            tempFrozen[j][grid_size - 1 - i] = frozen[i][j];
        }
    }
    memcpy(grid, tempGrid, sizeof(grid));
    memcpy(frozen, tempFrozen, sizeof(frozen));
    Beep(600, 35);
    InvalidateRect(mainHwnd, NULL, TRUE);
}

void DoTileHammer() {
    if (powerups_hammers <= 0 || gameOver || win) return;
    int targetI = -1, targetJ = -1;
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            if (grid[i][j] == -1) { targetI = i; targetJ = j; break; }
        }
        if (targetI != -1) break;
    }
    if (targetI == -1) {
        int minVal = 999999;
        for (int i = 0; i < grid_size; i++) {
            for (int j = 0; j < grid_size; j++) {
                if (grid[i][j] > 0 && grid[i][j] < minVal) {
                    minVal = grid[i][j];
                    targetI = i; targetJ = j;
                }
            }
        }
    }
    if (targetI != -1) {
        powerups_hammers--;
        int cell_size = 320 / grid_size;
        int px = MARGIN + 8 + targetJ * cell_size + cell_size / 2;
        int py = HEADER_HEIGHT + 8 + targetI * cell_size + cell_size / 2;
        SpawnMergeParticles(px, py, RGB(255, 60, 60));
        grid[targetI][targetJ] = 0;
        frozen[targetI][targetJ] = 0;
        Beep(300, 45);
        InvalidateRect(mainHwnd, NULL, TRUE);
    }
}

void DoFreeUndo() {
    if (historyCount <= 0 || powerups_undos <= 0) return;
    powerups_undos--;
    historyCount--;
    memcpy(grid, &history[historyCount].grid, sizeof(grid));
    memcpy(frozen, &history[historyCount].frozen, sizeof(frozen));
    score = history[historyCount].score;
    movesLeft = history[historyCount].movesLeft;
    gameOver = 0;
    timeOut = 0;
    outOfMoves = 0;
    win = 0;
    Beep(500, 30);
    InvalidateRect(mainHwnd, NULL, TRUE);
}

void DrawBoard(HDC hdc) {
    RECT bgRect = {0, 0, 800, 600};

    int bgR = 250, bgG = 248, bgB = 239;
    int txtR = 119, txtG = 110, txtB = 101;
    int boardR = 187, boardG = 173, boardB = 160;

    if (theme == 0) {
        bgR = 30; bgG = 30; bgB = 30;
        txtR = 255; txtG = 255; txtB = 255;
        boardR = 20; boardG = 20; boardB = 20;
    } else if (theme == 2) {
        bgR = 252; bgG = 228; bgB = 236;
        txtR = 74; txtG = 20; txtB = 140;
        boardR = 248; boardG = 187; boardB = 208;
    }

    HBRUSH bgb = CreateSolidBrush(RGB(bgR, bgG, bgB));
    FillRect(hdc, &bgRect, bgb);
    DeleteObject(bgb);

    // Environmental Art: Subtly moving grid and floating numbers
    HPEN envPen = CreatePen(PS_SOLID, 1, RGB(bgR > 128 ? bgR-15 : bgR+15, bgG > 128 ? bgG-15 : bgG+15, bgB > 128 ? bgB-15 : bgB+15));
    HPEN oldPenEnv = (HPEN)SelectObject(hdc, envPen);
    int offset = (frameAnimCount / 2) % 40;
    for (int x = offset; x < 800; x += 40) {
        MoveToEx(hdc, x, 0, NULL); LineTo(hdc, x, 600);
    }
    for (int y = offset; y < 600; y += 40) {
        MoveToEx(hdc, 0, y, NULL); LineTo(hdc, 800, y);
    }
    SelectObject(hdc, oldPenEnv);
    DeleteObject(envPen);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(bgR > 128 ? bgR-20 : bgR+20, bgG > 128 ? bgG-20 : bgG+20, bgB > 128 ? bgB-20 : bgB+20));
    for (int i=0; i<15; i++) {
        int py = (i*87 - frameAnimCount) % 650;
        if (py < -50) py += 650;
        int wave = (frameAnimCount + i*10) % 100;
        int sinVal = (wave < 50) ? wave : (100 - wave);
        sinVal = sinVal - 25;
        int px = (i*137 + sinVal) % 800;
        char buf[8]; wsprintfA(buf, "%d", 2 << (i%4));
        TextOutA(hdc, px, py, buf, lstrlenA(buf));
    }

    // Header Title
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(txtR, txtG, txtB));
    HFONT hFontTitle = CreateFontA(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
    HFONT oldFont = (HFONT)SelectObject(hdc, hFontTitle);

    RECT titleRect = { MARGIN, MARGIN, 180, HEADER_HEIGHT };
    DrawTextA(hdc, "2048", -1, &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, oldFont);
    DeleteObject(hFontTitle);

    char scoreBuf[128];
    if (campaignMode) {
        const CampaignStage* s = &campaignStages[campaignLevel - 1];
        if (movesMax > 0 && timeAttackEnabled) {
            wsprintfA(scoreBuf, "Lvl %d/30 | Tgt: %d | Mvs: %d/%d | %ds", campaignLevel, s->target, movesLeft, movesMax, timeRemaining);
        } else if (movesMax > 0) {
            wsprintfA(scoreBuf, "Lvl %d/30 | Tgt: %d | Mvs: %d/%d", campaignLevel, s->target, movesLeft, movesMax);
        } else {
            wsprintfA(scoreBuf, "Lvl %d/30 | Tgt: %d | Score: %d", campaignLevel, s->target, score);
        }
    } else if (timeAttackEnabled) {
        wsprintfA(scoreBuf, "Score: %d | Best: %d | Time: %ds", score, bestScore, timeRemaining);
    } else {
        wsprintfA(scoreBuf, "Score: %d | Best: %d", score, bestScore);
    }

    HFONT hFontText = CreateFontA(17, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
    oldFont = (HFONT)SelectObject(hdc, hFontText);

    int cell_size = 320 / grid_size;
    RECT scoreRect = { 160, MARGIN, MARGIN + grid_size*cell_size, MARGIN + 25 };
    DrawTextA(hdc, scoreBuf, -1, &scoreRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

    char helpBuf2[128];
    wsprintfA(helpBuf2, "Pwrs: [U]pg:%d [R]ot:%d [H]am:%d [Z]Undo:%d", powerups_upgrades, powerups_rotates, powerups_hammers, powerups_undos);
    RECT helpRect2 = { MARGIN, HEADER_HEIGHT - 22, MARGIN + grid_size*cell_size, HEADER_HEIGHT };
    DrawTextA(hdc, helpBuf2, -1, &helpRect2, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // 3D Board Outer Frame
    RECT boardBg = { MARGIN, HEADER_HEIGHT, MARGIN + grid_size*cell_size + 8, HEADER_HEIGHT + grid_size*cell_size + 8 };
    HBRUSH boardBrush = CreateSolidBrush(RGB(boardR, boardG, boardB));
    FillRect(hdc, &boardBg, boardBrush);
    DeleteObject(boardBrush);

    // Procedural Wood/Stone Grain
    for (int k = 0; k < 1500; k++) {
        int gx = boardBg.left + (my_rand() % (boardBg.right - boardBg.left));
        int gy = boardBg.top + (my_rand() % (boardBg.bottom - boardBg.top));
        int nR = boardR, nG = boardG, nB = boardB;
        if (my_rand() % 2 == 0) {
            nR = max(0, nR - 15); nG = max(0, nG - 15); nB = max(0, nB - 15);
        } else {
            nR = min(255, nR + 15); nG = min(255, nG + 15); nB = min(255, nB + 15);
        }
        SetPixel(hdc, gx, gy, RGB(nR, nG, nB));
    }

    // Dynamic Ambient Occlusion based on grid occupancy
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            if (grid[i][j] != 0) {
                int cx = MARGIN + 8 + j * cell_size + cell_size / 2;
                int cy = HEADER_HEIGHT + 8 + i * cell_size + cell_size / 2;
                int r = cell_size / 2 + 3;
                for (int d = 1; d <= 4; d++) {
                    HPEN aoPen = CreatePen(PS_SOLID, 2, RGB(max(0, boardR - 25/d), max(0, boardG - 25/d), max(0, boardB - 25/d)));
                    HPEN oldP = (HPEN)SelectObject(hdc, aoPen);
                    SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    Ellipse(hdc, cx - r + d*2, cy - r + d*2, cx + r - d*2, cy + r - d*2);
                    SelectObject(hdc, oldP);
                    DeleteObject(aoPen);
                }
            }
        }
    }

    for (int s = 0; s < 12; s++) {
        int alpha = 12 - s;
        int shadowR = max(0, boardR - alpha * 4);
        int shadowG = max(0, boardG - alpha * 4);
        int shadowB = max(0, boardB - alpha * 4);
        HPEN sPen = CreatePen(PS_SOLID, 1, RGB(shadowR, shadowG, shadowB));
        HPEN oldP2 = (HPEN)SelectObject(hdc, sPen);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, boardBg.left + s, boardBg.top + s, boardBg.right - s, boardBg.bottom - s);
        SelectObject(hdc, oldP2);
        DeleteObject(sPen);
    }

    HPEN fLight = CreatePen(PS_SOLID, 3, RGB(min(255, boardR + 50), min(255, boardG + 50), min(255, boardB + 50)));
    HPEN fDark = CreatePen(PS_SOLID, 3, RGB(max(0, boardR - 50), max(0, boardG - 50), max(0, boardB - 50)));
    HPEN oldP = (HPEN)SelectObject(hdc, fLight);
    MoveToEx(hdc, boardBg.left - 2, boardBg.bottom + 2, NULL);
    LineTo(hdc, boardBg.left - 2, boardBg.top - 2);
    LineTo(hdc, boardBg.right + 2, boardBg.top - 2);
    SelectObject(hdc, fDark);
    LineTo(hdc, boardBg.right + 2, boardBg.bottom + 2);
    LineTo(hdc, boardBg.left - 2, boardBg.bottom + 2);
    SelectObject(hdc, oldP);
    DeleteObject(fLight);
    DeleteObject(fDark);

    // Draw Cells
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            DrawCell(hdc, MARGIN + 8 + j*cell_size, HEADER_HEIGHT + 8 + i*cell_size, grid[i][j], cell_size, frozen[i][j], mergePop[i][j], squashTimer[i][j], squashDir[i][j]);
        }
    }

    UpdateAndDrawParticles(hdc);

    if (gameOver || win) {
        HBRUSH overlay = CreateSolidBrush(gameOver ? RGB(238,228,218) : RGB(237,194,46));
        RECT msgRect = { MARGIN + 20, HEADER_HEIGHT + 80, MARGIN + grid_size*cell_size - 12, HEADER_HEIGHT + 180 };
        FillRect(hdc, &msgRect, overlay);
        DeleteObject(overlay);
        SetTextColor(hdc, gameOver ? RGB(119,110,101) : RGB(255,255,255));

        const char* msg = "You Win! (Press N)";
        if (gameOver) {
            if (timeOut) msg = "Time's Up! (Press N)";
            else if (outOfMoves) msg = "Out of Moves! (Press N)";
            else msg = "Game Over! (Press N)";
        }
        DrawTextA(hdc, msg, -1, &msgRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    SelectObject(hdc, oldFont);
    DeleteObject(hFontText);
}

int Move(int dx, int dy) {
    if (gameOver || win) return 0;
    int moved = 0;

    int tempGrid[MAX_GRID][MAX_GRID];
    int tempFrozen[MAX_GRID][MAX_GRID];
    int tempScore = score;
    int tempMoves = movesLeft;
    memcpy(tempGrid, grid, sizeof(grid));
    memcpy(tempFrozen, frozen, sizeof(frozen));

    int startI = (dy == 1) ? grid_size - 1 : 0;
    int endI = (dy == 1) ? -1 : grid_size;
    int stepI = (dy == 1) ? -1 : 1;

    int startJ = (dx == 1) ? grid_size - 1 : 0;
    int endJ = (dx == 1) ? -1 : grid_size;
    int stepJ = (dx == 1) ? -1 : 1;

    int merged[MAX_GRID][MAX_GRID] = {0};

    for (int i = startI; i != endI; i += stepI) {
        for (int j = startJ; j != endJ; j += stepJ) {
            if (grid[i][j] != 0) {
                int ci = i;
                int cj = j;
                while (1) {
                    int ni = ci + dy;
                    int nj = cj + dx;
                    if (ni < 0 || ni >= grid_size || nj < 0 || nj >= grid_size) {
                        if (ci != i || cj != j) {
                            squashTimer[ci][cj] = 6;
                            squashDir[ci][cj] = (dx != 0) ? 1 : 2;
                        }
                        break;
                    }
                    if (grid[ni][nj] == 0) {
                        grid[ni][nj] = grid[ci][cj];
                        frozen[ni][nj] = frozen[ci][cj];
                        grid[ci][cj] = 0;
                        frozen[ci][cj] = 0;
                        ci = ni;
                        cj = nj;
                        moved = 1;
                    } else if (GetMergeResult(grid[ci][cj], grid[ni][nj]) != 0 && !merged[ni][nj]) {
                        int mergeRes = GetMergeResult(grid[ci][cj], grid[ni][nj]);
                        if (mergeRes == -100) { // Bomb explosion 3x3
                            for (int r = ni - 1; r <= ni + 1; r++) {
                                for (int c = nj - 1; c <= nj + 1; c++) {
                                    if (r >= 0 && r < grid_size && c >= 0 && c < grid_size) {
                                        if (grid[r][c] != 0) {
                                            int cell_size = 320 / grid_size;
                                            int px = MARGIN + 8 + c * cell_size + cell_size / 2;
                                            int py = HEADER_HEIGHT + 8 + r * cell_size + cell_size / 2;
                                            SpawnMergeParticles(px, py, RGB(255, 60, 60));
                                            grid[r][c] = 0;
                                            frozen[r][c] = 0;
                                            score += 10;
                                        }
                                    }
                                }
                            }
                            grid[ci][cj] = 0;
                            frozen[ci][cj] = 0;
                        } else {
                            grid[ni][nj] = mergeRes;
                            // If either tile was frozen, it now thaws!
                            frozen[ni][nj] = 0;
                            grid[ci][cj] = 0;
                            frozen[ci][cj] = 0;
                            score += grid[ni][nj];
                            stats_tilesMerged++;
                            if (grid[ni][nj] > stats_highestTile) {
                                stats_highestTile = grid[ni][nj];
                            }
                            int isWinTile = (ruleset == 1) ? (grid[ni][nj] >= 2584) : ((ruleset == 2) ? (grid[ni][nj] >= 1536) : (grid[ni][nj] >= 2048));
                            if (isWinTile && !hasWon && !campaignMode) {
                                win = 1;
                                hasWon = 1;
                            }
                            if (mergeRes >= 512) {
                                int intensity = (mergeRes >= 2048) ? 30 : (mergeRes >= 1024 ? 20 : 15);
                                if (screenShakeTime < intensity) screenShakeTime = intensity;
                            }
                            if (mergeRes >= 2048) {
                                for (int k=0; k<MAX_SHOCKWAVES; k++) {
                                    if (!shockwaves[k].active) {
                                        shockwaves[k].active = 1;
                                        shockwaves[k].x = MARGIN + 8 + nj * cell_size + cell_size / 2.0f;
                                        shockwaves[k].y = HEADER_HEIGHT + 8 + ni * cell_size + cell_size / 2.0f;
                                        shockwaves[k].radius = 10.0f;
                                        break;
                                    }
                                }
                            }
                            mergePop[ni][nj] = 10;
                            squashTimer[ni][nj] = 0;
                        }

                        int cell_size = 320 / grid_size;
                        int px = MARGIN + 8 + nj * cell_size + cell_size / 2;
                        int py = HEADER_HEIGHT + 8 + ni * cell_size + cell_size / 2;
                        COLORREF pColor = GetTileColor(mergeRes);
                        SpawnMergeParticles(px, py, pColor);

                        if (score > bestScore) {
                            bestScore = score;
                            SaveBest();
                        }
                        merged[ni][nj] = 1;
                        moved = 1;

                        if (campaignMode && mergeRes > 0) {
                            int target = campaignStages[campaignLevel - 1].target;
                            if (grid[ni][nj] >= target) {
                                char msg[64];
                                wsprintfA(msg, "Stage %d Complete!", campaignLevel);
                                MessageBoxA(mainHwnd, msg, "Campaign Victory", MB_OK);
                                campaignLevel++;
                                if (campaignLevel > 30) {
                                    MessageBoxA(mainHwnd, "Congratulations! You completed all 30 Campaign Stages!", "Grandmaster", MB_OK);
                                    campaignLevel = 1;
                                }
                                StartCampaignLevel();
                                return 1;
                            }
                        }
                        break;
                    } else {
                        if (ci != i || cj != j) {
                            squashTimer[ci][cj] = 6;
                            squashDir[ci][cj] = (dx != 0) ? 1 : 2;
                        }
                        break;
                    }
                }
            }
        }
    }

    if (moved) {
        moveCount++;
        if (movesMax > 0) {
            movesLeft--;
        }

        if (!gameStarted) {
            gameStarted = 1;
            if (timeAttackEnabled) {
                SetTimer(mainHwnd, 1, 1000, NULL);
                timerActive = 1;
            }
        }

        if (historyCount < MAX_HISTORY) {
            memcpy(&history[historyCount].grid, tempGrid, sizeof(tempGrid));
            memcpy(&history[historyCount].frozen, tempFrozen, sizeof(tempFrozen));
            history[historyCount].score = tempScore;
            history[historyCount].movesLeft = tempMoves;
            historyCount++;
        } else {
            memmove(&history[0], &history[1], sizeof(HistoryState) * (MAX_HISTORY - 1));
            memcpy(&history[MAX_HISTORY - 1].grid, tempGrid, sizeof(tempGrid));
            memcpy(&history[MAX_HISTORY - 1].frozen, tempFrozen, sizeof(tempFrozen));
            history[MAX_HISTORY - 1].score = tempScore;
            history[MAX_HISTORY - 1].movesLeft = tempMoves;
        }

        int note = 400;
        int max_val = 0;
        for (int i=0;i<grid_size;i++) for(int j=0;j<grid_size;j++) if(grid[i][j] > max_val) max_val = grid[i][j];
        int v = max_val;
        while (v > 1) { note += 50; v >>= 1; }
        Beep(note, 25);
        AddRandomTile();
        SaveStats();

        if (movesMax > 0 && movesLeft <= 0) {
            gameOver = 1;
            outOfMoves = 1;
            stats_gamesPlayed++;
            SaveStats();
            if (timerActive) { KillTimer(mainHwnd, 1); timerActive = 0; }
            SaveBest();
        } else if (CheckGameOver()) {
            gameOver = 1;
            stats_gamesPlayed++;
            SaveStats();
            if (timerActive) { KillTimer(mainHwnd, 1); timerActive = 0; }
            SaveBest();
        }
    }
    return moved;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            mainHwnd = hwnd;
            LoadTheme();
            LoadStats();
            InitGame();
            SetTimer(hwnd, 3, 1000, NULL);
            SetTimer(hwnd, 4, 30, NULL);
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_TIMER:
            if (wParam == 4) {
                frameAnimCount++;
                int hasMilestone = 0;
                for (int i=0; i<grid_size; i++) {
                    for (int j=0; j<grid_size; j++) {
                        if (grid[i][j] >= 1024 || grid[i][j] == -2 || frozen[i][j]) { hasMilestone = 1; }
                        if (mergePop[i][j] > 0) {
                            mergePop[i][j]--;
                            if (mergePop[i][j] > 0) hasMilestone = 1;
                        }
                        if (squashTimer[i][j] > 0) {
                            squashTimer[i][j]--;
                            if (squashTimer[i][j] > 0) hasMilestone = 1;
                        }
                    }
                }
                if (screenShakeTime > 0) hasMilestone = 1;
                for (int k = 0; k < MAX_DEBRIS; k++) if (debris[k].active) hasMilestone = 1;
                for (int k = 0; k < MAX_SHOCKWAVES; k++) if (shockwaves[k].active) hasMilestone = 1;

                if (particleCount > 0 || hasMilestone) {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                return 0;
            } else if (wParam == 3) {
                if (!gameOver && gameStarted && !win) {
                    stats_timePlayed++;
                    if (stats_timePlayed % 10 == 0) SaveStats();
                }
                return 0;
            } else if (wParam == 1) {
                timeRemaining--;
                if (timeRemaining <= 0) {
                    KillTimer(hwnd, 1);
                    timerActive = 0;
                    gameOver = 1;
                    timeOut = 1;
                    stats_gamesPlayed++;
                    SaveStats();
                }
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 2) {
                if (!gameOver && !win) {
                    int dirs[4][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}};
                    int startIdx = my_rand() % 4;
                    int moved = 0;
                    for (int i = 0; i < 4; i++) {
                        int idx = (startIdx + i) % 4;
                        if (Move(dirs[idx][0], dirs[idx][1])) {
                            moved = 1;
                            break;
                        }
                    }
                    if (moved) InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            return 0;
        case WM_KEYDOWN:
            if (wParam >= '3' && wParam <= '6') {
                grid_size = wParam - '0';
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'U' || wParam == 'u') {
                DoTileUpgrade();
            } else if (wParam == 'R' || wParam == 'r') {
                DoGridRotate();
            } else if (wParam == 'H' || wParam == 'h') {
                DoTileHammer();
            } else if (wParam == 'Z' || wParam == 'z') {
                DoFreeUndo();
            } else if (wParam == 'N' || wParam == 'n') {
                if (campaignMode) StartCampaignLevel(); else InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'C' || wParam == 'c') {
                campaignMode = !campaignMode;
                campaignLevel = 1;
                if (campaignMode) StartCampaignLevel(); else InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'M' || wParam == 'm') {
                timeAttackEnabled = !timeAttackEnabled;
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'P' || wParam == 'p') {
                autoPlayEnabled = !autoPlayEnabled;
                if (autoPlayEnabled) {
                    SetTimer(hwnd, 2, 250, NULL);
                    autoPlayTimerActive = 1;
                } else {
                    KillTimer(hwnd, 2);
                    autoPlayTimerActive = 0;
                }
            } else if (wParam == 'F' || wParam == 'f') {
                ruleset = (ruleset + 1) % 3;
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'T' || wParam == 't') {
                theme = (theme + 1) % 3;
                SaveTheme();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'I' || wParam == 'i') {
                char statsBuf[256];
                wsprintfA(statsBuf, "Games Played: %d\nTiles Merged: %d\nHighest Tile: %d\nTime Played: %dm %ds", 
                    stats_gamesPlayed, stats_tilesMerged, stats_highestTile, stats_timePlayed / 60, stats_timePlayed % 60);
                MessageBoxA(hwnd, statsBuf, "Statistics", MB_OK | MB_ICONINFORMATION);
            } else if (wParam == VK_OEM_2) { // ? key
                char helpText[1024];
                wsprintfA(helpText,
                    "K2048 Loop 7 Edition\n\n"
                    "Controls: Arrow Keys or WASD to move tiles.\n\n"
                    "Active Skills / Hotkeys:\n"
                    "U: Tile Upgrade (Doubles lowest tile)\n"
                    "R: Grid Rotate 90 deg clockwise\n"
                    "H: Hammer (Smashes obstacle / lowest tile)\n"
                    "Z: Free Undo (Reverts last move)\n"
                    "N: New Game / Restart Level\n"
                    "C: Toggle Campaign Mode (30 Stages)\n"
                    "M: Time Attack Mode Toggle\n"
                    "P: Auto-Play Toggle\n"
                    "F: Cycle Ruleset (Classic/Fib/Threes)\n"
                    "T: Change Theme (Dark/Classic/Pastel)\n"
                    "I: View Statistics\n"
                    "3, 4, 5, 6: Change Grid Size\n\n"
                    "Special Tiles:\n"
                    "ICE (Frozen): Must be merged twice to thaw!\n"
                    "B (Bomb): Merging explodes 3x3 surrounding tiles!\n"
                    "W (Wildcard): Merges with any matching pair!\n"
                    "X (Stone): Impassable obstacle block."
                );
                MessageBoxA(hwnd, helpText, "How to Play", MB_OK | MB_ICONINFORMATION);
            } else if (!gameOver && !win) {
                if (autoPlayEnabled) {
                    autoPlayEnabled = 0;
                    KillTimer(hwnd, 2);
                    autoPlayTimerActive = 0;
                }
                int moved = 0;
                if (wParam == VK_LEFT || wParam == 'A') moved = Move(-1, 0);
                else if (wParam == VK_RIGHT || wParam == 'D') moved = Move(1, 0);
                else if (wParam == VK_UP || wParam == 'W') moved = Move(0, -1);
                else if (wParam == VK_DOWN || wParam == 'S') moved = Move(0, 1);
                if (moved) InvalidateRect(hwnd, NULL, TRUE);
            } else if (win) {
                if (wParam == VK_SPACE || wParam == VK_RETURN) {
                    win = 0;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rc;
            GetClientRect(hwnd, &rc);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

            int dx = 0, dy = 0;
            if (screenShakeTime > 0) {
                dx = (my_rand() % (screenShakeTime + 2)) - (screenShakeTime / 2 + 1);
                dy = (my_rand() % (screenShakeTime + 2)) - (screenShakeTime / 2 + 1);
                screenShakeTime--;
            }

            SetViewportOrgEx(memDC, dx, dy, NULL);

            DrawBoard(memDC);

            SetViewportOrgEx(memDC, 0, 0, NULL);

            if (screenShakeTime > 12) {
                PatBlt(memDC, 0, 0, rc.right, rc.bottom, DSTINVERT);
            }

            BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            KillTimer(hwnd, 2);
            KillTimer(hwnd, 3);
            KillTimer(hwnd, 4);
            SaveBest();
            SaveStats();
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

#ifndef EXCLUDE_MAIN
int WINAPI MainEntry() {
    HINSTANCE hInstance = GetModuleHandleA(NULL);
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "K2048Class";
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursorA(NULL, (LPCSTR)32512);

    RegisterClassA(&wc);

    int winWidth = MARGIN * 2 + 4 * 80 + 8 + 16;
    int winHeight = HEADER_HEIGHT + MARGIN + 4 * 80 + 8 + 39;

    HWND hwnd = CreateWindowExA(
        0,
        "K2048Class",
        "KiloOS - 2048",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, winWidth, winHeight,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, SW_SHOW);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return 0;
}
#endif
