#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int _fltused = 1;

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

#define MAX_ROWS 40
#define MAX_COLS 40
int rows = 9;
int cols = 9;
int mines = 10;
#define CELL_SIZE 22
#define HEADER_HEIGHT 68

// Bitmasks for grid
#define CELL_MINE     0x01
#define CELL_REVEALED 0x02
#define CELL_FLAGGED  0x04
#define CELL_QUESTION 0x08
#define CELL_CHEST    0x10

int grid[MAX_ROWS][MAX_COLS];
DWORD flagTick[MAX_ROWS][MAX_COLS];
int gameOver = 0;
int initialized = 0;
int timeElapsed = 0;
int bestTimes[3] = {999, 999, 999};
int bestRush = 0;
int currentDiff = 0; // 0=Easy, 1=Medium, 2=Hard, 3=Rush
int flagsPlaced = 0;
int campaignMode = 0;
int campaignLevel = 1;

// Power-ups
int shields = 0;   // Blast Shield (S key)
int detectors = 0; // Detector / Flag-Bot (D key)
int sonars = 0;    // Sonar / Radar Scan (R key)

// Rush & Speedrun Mode
int rushMode = 0;
int rushTime = 0;
int rushScore = 0;
int isSpeedrun = 0;
int speedrunTime = 0;

// Rapid-Clear Combo System
int comboCount = 0;
int comboMultiplier = 1;
DWORD lastRevealTick = 0;

// Status Notification Banner
char statusMsg[128] = {0};
DWORD statusMsgTime = 0;

int totalPlayed = 0;
int totalWins = 0;
HWND mainHwnd = NULL;
int mouseCellPressed = 0;

DWORD sonarTick = 0;
DWORD detectorTick = 0;
int detectorR = -1;
int detectorC = -1;

// Screen Shake with quadratic decay
int shakeIntensity = 0;
DWORD shakeTick = 0;
DWORD shakeDuration = 0;

// 16-step trigonometric lookup tables
static const int sin_tab16[16] = { 0, 38, 71, 92, 100, 92, 71, 38, 0, -38, -71, -92, -100, -92, -71, -38 };
static const int cos_tab16[16] = { 100, 92, 71, 38, 0, -38, -71, -92, -100, -92, -71, -38, 0, 38, 71, 92 };

static int FastSin(int idx) {
    idx = ((idx % 16) + 16) % 16;
    return sin_tab16[idx];
}
static int FastCos(int idx) {
    idx = ((idx % 16) + 16) % 16;
    return cos_tab16[idx];
}

// Multi-Layer Kinematic Particle System
// Layer 0: Incandescent core needle sparks
// Layer 1: Expanding buoyant smoke & plasma puffs
// Layer 2: Heavy kinematic rock/debris shards with tumbling rotation & bounce physics
// Layer 3: Radiant golden celebration energy stars
typedef struct {
    float x, y;
    float prev_x, prev_y;
    float vx, vy;
    float life;
    float maxLife;
    float size;
    COLORREF color;
    int layer;
    float rot;
    float vrot;
    float gravity;
    float drag;
} Particle;

#define MAX_PARTICLES 450
Particle particles[MAX_PARTICLES];
int particleCount = 0;

// Dual-Tier Concentric Shockwaves
typedef struct {
    float x, y;
    float radius;
    float maxRadius;
    float speed;
    float life;
    float decay;
    COLORREF color;
    int isInner;
} Shockwave;

#define MAX_SHOCKWAVES 32
Shockwave shockwaves[MAX_SHOCKWAVES];
int shockwaveCount = 0;

// Ambient Cyber Dust Motes
#define MAX_DUST 36
typedef struct {
    float x, y;
    float vx, vy;
    float size;
    COLORREF color;
} DustMote;
DustMote dustMotes[MAX_DUST];
int dustInit = 0;

static unsigned int seed = 0;

int my_rand() {
    seed = seed * 214013 + 2531011;
    return (seed >> 16) & 0x7FFF;
}

static int parse_int(const char** p) {
    while (**p && (**p < '0' || **p > '9')) (*p)++;
    if (!**p) return 0;
    int val = 0;
    while (**p >= '0' && **p <= '9') {
        val = val * 10 + (**p - '0');
        (*p)++;
    }
    return val;
}

void TriggerScreenShake(int intensity, DWORD durationMs) {
    shakeIntensity = intensity;
    shakeTick = GetTickCount();
    shakeDuration = durationMs;
}

void SpawnShockwave(float cx, float cy, COLORREF color, int isInner, float maxR, float speed) {
    if (shockwaveCount < MAX_SHOCKWAVES) {
        Shockwave* sw = &shockwaves[shockwaveCount++];
        sw->x = cx;
        sw->y = cy;
        sw->radius = 1.0f;
        sw->maxRadius = maxR;
        sw->speed = speed;
        sw->life = 1.0f;
        sw->decay = isInner ? 0.035f : 0.02f;
        sw->color = color;
        sw->isInner = isInner;
    }
}

void SpawnExplosion(float cx, float cy) {
    // Dual-tier concentric shockwaves
    SpawnShockwave(cx, cy, RGB(255, 60, 90), 1, 140.0f, 6.5f);
    SpawnShockwave(cx, cy, RGB(255, 140, 40), 0, 200.0f, 4.0f);
    TriggerScreenShake(16, 500);

    COLORREF sparkColors[5] = {
        RGB(255, 255, 255),
        RGB(255, 220, 60),
        RGB(255, 90, 40),
        RGB(255, 160, 80),
        RGB(120, 200, 255)
    };

    COLORREF smokeColors[3] = {
        RGB(40, 42, 50),
        RGB(65, 70, 85),
        RGB(30, 32, 40)
    };

    COLORREF debrisColors[4] = {
        RGB(60, 70, 95),
        RGB(40, 45, 60),
        RGB(247, 118, 142),
        RGB(224, 175, 104)
    };

    // Layer 0: Incandescent core needle sparks
    for (int i = 0; i < 35; i++) {
        if (particleCount < MAX_PARTICLES) {
            Particle* p = &particles[particleCount++];
            p->x = cx; p->y = cy;
            p->prev_x = cx; p->prev_y = cy;
            int angleIdx = my_rand() % 16;
            float speed = 3.0f + (float)(my_rand() % 70) / 10.0f;
            p->vx = ((float)FastCos(angleIdx) / 100.0f) * speed;
            p->vy = ((float)FastSin(angleIdx) / 100.0f) * speed - 1.5f;
            p->maxLife = 20.0f + (float)(my_rand() % 20);
            p->life = p->maxLife;
            p->size = 1.5f + (float)(my_rand() % 2);
            p->color = sparkColors[my_rand() % 5];
            p->layer = 0;
            p->gravity = 0.08f;
            p->drag = 0.94f;
        }
    }

    // Layer 1: Expanding buoyant smoke & plasma puffs
    for (int i = 0; i < 20; i++) {
        if (particleCount < MAX_PARTICLES) {
            Particle* p = &particles[particleCount++];
            p->x = cx; p->y = cy;
            p->prev_x = cx; p->prev_y = cy;
            int angleIdx = my_rand() % 16;
            float speed = 0.5f + (float)(my_rand() % 25) / 10.0f;
            p->vx = ((float)FastCos(angleIdx) / 100.0f) * speed;
            p->vy = ((float)FastSin(angleIdx) / 100.0f) * speed - 1.0f;
            p->maxLife = 30.0f + (float)(my_rand() % 25);
            p->life = p->maxLife;
            p->size = 5.0f + (float)(my_rand() % 8);
            p->color = smokeColors[my_rand() % 3];
            p->layer = 1;
            p->gravity = -0.04f; // buoyant rise
            p->drag = 0.96f;
        }
    }

    // Layer 2: Heavy rock & cyber debris shards
    for (int i = 0; i < 25; i++) {
        if (particleCount < MAX_PARTICLES) {
            Particle* p = &particles[particleCount++];
            p->x = cx; p->y = cy;
            p->prev_x = cx; p->prev_y = cy;
            int angleIdx = my_rand() % 16;
            float speed = 2.0f + (float)(my_rand() % 50) / 10.0f;
            p->vx = ((float)FastCos(angleIdx) / 100.0f) * speed;
            p->vy = ((float)FastSin(angleIdx) / 100.0f) * speed - 2.5f;
            p->maxLife = 35.0f + (float)(my_rand() % 30);
            p->life = p->maxLife;
            p->size = 3.0f + (float)(my_rand() % 4);
            p->color = debrisColors[my_rand() % 4];
            p->layer = 2;
            p->rot = (float)(my_rand() % 16);
            p->vrot = ((float)(my_rand() % 20) - 10.0f) / 10.0f;
            p->gravity = 0.18f;
            p->drag = 0.98f;
        }
    }
}

void SpawnDustFX(float cx, float cy) {
    for (int i = 0; i < 12; i++) {
        if (particleCount < MAX_PARTICLES) {
            Particle* p = &particles[particleCount++];
            p->x = cx; p->y = cy;
            p->prev_x = cx; p->prev_y = cy;
            p->vx = (float)((my_rand() % 60) - 30) / 15.0f;
            p->vy = -(float)(my_rand() % 40) / 15.0f - 0.5f;
            p->maxLife = 12.0f + (float)(my_rand() % 15);
            p->life = p->maxLife;
            p->size = 1.5f + (float)(my_rand() % 2);
            p->color = RGB(160, 170, 195);
            p->layer = 0;
            p->gravity = 0.05f;
            p->drag = 0.95f;
        }
    }
}

void SpawnTreasureFX(float cx, float cy) {
    SpawnShockwave(cx, cy, RGB(255, 215, 0), 1, 120.0f, 5.0f);
    SpawnShockwave(cx, cy, RGB(56, 189, 248), 0, 160.0f, 3.5f);
    TriggerScreenShake(7, 300);

    COLORREF starColors[4] = {
        RGB(255, 215, 0),
        RGB(255, 245, 140),
        RGB(56, 189, 248),
        RGB(255, 255, 255)
    };

    // Layer 3: Radiant golden celebration energy stars
    for (int i = 0; i < 30; i++) {
        if (particleCount < MAX_PARTICLES) {
            Particle* p = &particles[particleCount++];
            p->x = cx; p->y = cy;
            p->prev_x = cx; p->prev_y = cy;
            int angleIdx = my_rand() % 16;
            float speed = 1.5f + (float)(my_rand() % 40) / 10.0f;
            p->vx = ((float)FastCos(angleIdx) / 100.0f) * speed;
            p->vy = ((float)FastSin(angleIdx) / 100.0f) * speed - 2.0f;
            p->maxLife = 25.0f + (float)(my_rand() % 25);
            p->life = p->maxLife;
            p->size = 3.0f + (float)(my_rand() % 3);
            p->color = starColors[my_rand() % 4];
            p->layer = 3;
            p->rot = (float)(my_rand() % 16);
            p->vrot = ((float)(my_rand() % 20) - 10.0f) / 10.0f;
            p->gravity = 0.10f;
            p->drag = 0.97f;
        }
    }
}

void SpawnVictoryFX() {
    SpawnShockwave((float)(cols * CELL_SIZE / 2), (float)(rows * CELL_SIZE / 2 + HEADER_HEIGHT), RGB(122, 162, 247), 0, 300.0f, 6.0f);
    TriggerScreenShake(12, 600);

    COLORREF victoryColors[6] = {
        RGB(122, 162, 247),
        RGB(158, 206, 106),
        RGB(224, 175, 104),
        RGB(187, 154, 247),
        RGB(125, 207, 255),
        RGB(247, 118, 142)
    };

    for (int i = 0; i < 60; i++) {
        if (particleCount < MAX_PARTICLES) {
            Particle* p = &particles[particleCount++];
            p->x = (float)(my_rand() % (cols * CELL_SIZE));
            p->y = (float)(HEADER_HEIGHT + my_rand() % (rows * CELL_SIZE / 2));
            p->prev_x = p->x; p->prev_y = p->y;
            p->vx = (float)((my_rand() % 60) - 30) / 15.0f;
            p->vy = (float)(my_rand() % 30) / 10.0f + 1.0f;
            p->maxLife = 35.0f + (float)(my_rand() % 35);
            p->life = p->maxLife;
            p->size = 3.5f + (float)(my_rand() % 4);
            p->color = victoryColors[my_rand() % 6];
            p->layer = 3;
            p->rot = (float)(my_rand() % 16);
            p->vrot = ((float)(my_rand() % 20) - 10.0f) / 10.0f;
            p->gravity = 0.08f;
            p->drag = 0.99f;
        }
    }
}

void InitDustMotes() {
    for (int i = 0; i < MAX_DUST; i++) {
        dustMotes[i].x = (float)(my_rand() % (cols * CELL_SIZE));
        dustMotes[i].y = (float)(HEADER_HEIGHT + my_rand() % (rows * CELL_SIZE));
        dustMotes[i].vx = ((float)(my_rand() % 20 - 10)) / 30.0f;
        dustMotes[i].vy = -((float)(my_rand() % 20 + 5)) / 30.0f;
        dustMotes[i].size = 1.0f + (float)(my_rand() % 2);
        dustMotes[i].color = (my_rand() % 2 == 0) ? RGB(122, 162, 247) : RGB(224, 175, 104);
    }
    dustInit = 1;
}

void UpdateDustMotes() {
    if (!dustInit) InitDustMotes();
    for (int i = 0; i < MAX_DUST; i++) {
        dustMotes[i].x += dustMotes[i].vx;
        dustMotes[i].y += dustMotes[i].vy;
        if (dustMotes[i].y < (float)HEADER_HEIGHT) {
            dustMotes[i].y = (float)(rows * CELL_SIZE + HEADER_HEIGHT);
            dustMotes[i].x = (float)(my_rand() % (cols * CELL_SIZE));
        }
        if (dustMotes[i].x < 0) dustMotes[i].x = (float)(cols * CELL_SIZE);
        if (dustMotes[i].x > (float)(cols * CELL_SIZE)) dustMotes[i].x = 0.0f;
    }
}

void UpdateParticles() {
    int activeP = 0;
    for (int i = 0; i < particleCount; i++) {
        Particle* p = &particles[i];
        if (p->life > 0) {
            p->prev_x = p->x;
            p->prev_y = p->y;
            p->x += p->vx;
            p->y += p->vy;
            p->vy += p->gravity;
            p->vx *= p->drag;
            p->rot += p->vrot;

            // Bounce physics for debris
            if (p->layer == 2) {
                if (p->y + p->size > (float)(rows * CELL_SIZE + HEADER_HEIGHT)) {
                    p->y = (float)(rows * CELL_SIZE + HEADER_HEIGHT) - p->size;
                    p->vy *= -0.65f;
                    p->vx *= 0.85f;
                    p->vrot *= 0.7f;
                }
                if (p->x - p->size < 0) { p->x = p->size; p->vx *= -0.65f; }
                if (p->x + p->size > (float)(cols * CELL_SIZE)) { p->x = (float)(cols * CELL_SIZE) - p->size; p->vx *= -0.65f; }
            }

            p->life -= 1.0f;
            particles[activeP++] = *p;
        }
    }
    particleCount = activeP;

    int activeSW = 0;
    for (int i = 0; i < shockwaveCount; i++) {
        Shockwave* sw = &shockwaves[i];
        if (sw->life > 0 && sw->radius < sw->maxRadius) {
            sw->radius += sw->speed;
            sw->life -= sw->decay;
            shockwaves[activeSW++] = *sw;
        }
    }
    shockwaveCount = activeSW;

    UpdateDustMotes();
}

void DrawStarPolygon(HDC hdc, int cx, int cy, int size, COLORREF color) {
    POINT pts[8] = {
        { cx, cy - size },
        { cx + size/3, cy - size/3 },
        { cx + size, cy },
        { cx + size/3, cy + size/3 },
        { cx, cy + size },
        { cx - size/3, cy + size/3 },
        { cx - size, cy },
        { cx - size/3, cy - size/3 }
    };
    HBRUSH hbr = CreateSolidBrush(color);
    HPEN hPen = CreatePen(PS_SOLID, 1, color);
    HGDIOBJ oldBr = SelectObject(hdc, hbr);
    HGDIOBJ oldPen = SelectObject(hdc, hPen);
    Polygon(hdc, pts, 8);
    SelectObject(hdc, oldBr);
    SelectObject(hdc, oldPen);
    DeleteObject(hbr);
    DeleteObject(hPen);
}

void DrawParticles(HDC hdc) {
    // 1. Dust motes
    for (int i = 0; i < MAX_DUST; i++) {
        SetPixel(hdc, (int)dustMotes[i].x, (int)dustMotes[i].y, dustMotes[i].color);
        if (dustMotes[i].size > 1.2f) {
            SetPixel(hdc, (int)dustMotes[i].x + 1, (int)dustMotes[i].y, dustMotes[i].color);
        }
    }

    // 2. Dual-tier Concentric Shockwaves
    for (int i = 0; i < shockwaveCount; i++) {
        Shockwave* sw = &shockwaves[i];
        if (sw->life > 0) {
            int width = sw->isInner ? 3 : 5;
            HPEN hPen = CreatePen(PS_SOLID, width, sw->color);
            HGDIOBJ oldPen = SelectObject(hdc, hPen);
            HGDIOBJ oldBr = SelectObject(hdc, GetStockObject(NULL_BRUSH));
            int r = (int)sw->radius;
            Ellipse(hdc, (int)sw->x - r, (int)sw->y - r, (int)sw->x + r, (int)sw->y + r);
            SelectObject(hdc, oldBr);
            SelectObject(hdc, oldPen);
            DeleteObject(hPen);
        }
    }

    // 3. Multi-layer particles
    for (int i = 0; i < particleCount; i++) {
        Particle* p = &particles[i];
        if (p->life > 0) {
            float ratio = p->life / p->maxLife;
            if (p->layer == 0) {
                // Needle spark trail
                HPEN hPen = CreatePen(PS_SOLID, (int)p->size, p->color);
                HGDIOBJ oldPen = SelectObject(hdc, hPen);
                MoveToEx(hdc, (int)p->prev_x, (int)p->prev_y, NULL);
                LineTo(hdc, (int)p->x, (int)p->y);
                SelectObject(hdc, oldPen);
                DeleteObject(hPen);
            } else if (p->layer == 1) {
                // Expanding buoyant smoke puff
                int r = (int)(p->size * (0.8f + 0.4f * (1.0f - ratio)));
                if (r < 1) r = 1;
                HBRUSH hbr = CreateSolidBrush(p->color);
                HGDIOBJ oldBr = SelectObject(hdc, hbr);
                HGDIOBJ oldPen = SelectObject(hdc, GetStockObject(NULL_PEN));
                Ellipse(hdc, (int)(p->x - r), (int)(p->y - r), (int)(p->x + r), (int)(p->y + r));
                SelectObject(hdc, oldBr);
                SelectObject(hdc, oldPen);
                DeleteObject(hbr);
            } else if (p->layer == 2) {
                // Tumbling debris shard
                int sz = (int)(p->size * (0.5f + 0.5f * ratio));
                if (sz < 1) sz = 1;
                RECT rc = { (int)p->x - sz, (int)p->y - sz, (int)p->x + sz, (int)p->y + sz };
                HBRUSH hbr = CreateSolidBrush(p->color);
                FillRect(hdc, &rc, hbr);
                DeleteObject(hbr);
            } else if (p->layer == 3) {
                // Golden celebration star
                int sz = (int)(p->size * (0.5f + 0.5f * ratio));
                if (sz < 2) sz = 2;
                DrawStarPolygon(hdc, (int)p->x, (int)p->y, sz, p->color);
            }
        }
    }
}

void LoadBest() {
    HANDLE hFile = CreateFileA("kmines_scores.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[128] = {0};
        DWORD bytesRead;
        if (ReadFile(hFile, buf, sizeof(buf)-1, &bytesRead, NULL)) {
            const char* ptr = buf;
            bestTimes[0] = parse_int(&ptr);
            bestTimes[1] = parse_int(&ptr);
            bestTimes[2] = parse_int(&ptr);
            totalPlayed = parse_int(&ptr);
            totalWins = parse_int(&ptr);
            bestRush = parse_int(&ptr);
        }
        CloseHandle(hFile);
    }
}

void SaveBest() {
    HANDLE hFile = CreateFileA("kmines_scores.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[128];
        wsprintfA(buf, "%d %d %d %d %d %d", bestTimes[0], bestTimes[1], bestTimes[2], totalPlayed, totalWins, bestRush);
        DWORD bytesWritten;
        int len = 0; while(buf[len]) len++;
        WriteFile(hFile, buf, len, &bytesWritten, NULL);
        CloseHandle(hFile);
    }
}

typedef struct {
    int r, c, m, chests, speedrun;
} StageDef;

StageDef campaignStages[20] = {
    {  8,  8,   8, 1,   0 }, // Stage 1
    {  9,  9,  12, 0,  60 }, // Stage 2 (Speedrun 60s)
    { 10, 10,  15, 2,   0 }, // Stage 3
    { 11, 11,  20, 0,  75 }, // Stage 4 (Speedrun 75s)
    { 12, 12,  25, 2,   0 }, // Stage 5
    { 13, 13,  30, 0,  90 }, // Stage 6 (Speedrun 90s)
    { 14, 14,  35, 3,   0 }, // Stage 7
    { 15, 15,  42, 0, 105 }, // Stage 8 (Speedrun 105s)
    { 16, 16,  50, 3,   0 }, // Stage 9
    { 17, 17,  58, 0, 120 }, // Stage 10 (Speedrun 120s)
    { 18, 18,  66, 3,   0 }, // Stage 11
    { 19, 19,  75, 0, 135 }, // Stage 12 (Speedrun 135s)
    { 20, 20,  85, 4,   0 }, // Stage 13
    { 21, 21,  95, 0, 150 }, // Stage 14 (Speedrun 150s)
    { 22, 22, 105, 4,   0 }, // Stage 15
    { 22, 22, 115, 0, 160 }, // Stage 16 (Speedrun 160s)
    { 23, 23, 125, 4,   0 }, // Stage 17
    { 23, 23, 135, 0, 170 }, // Stage 18 (Speedrun 170s)
    { 24, 24, 145, 5,   0 }, // Stage 19
    { 24, 24, 160, 5, 200 }  // Stage 20 Boss Speedrun (200s + Chests)
};

int chestsToPlace = 0;

void InitGame(int firstClickX, int firstClickY) {
    memset(grid, 0, sizeof(grid));
    memset(flagTick, 0, sizeof(flagTick));
    gameOver = 0;
    if (!rushMode && !isSpeedrun) timeElapsed = 0;
    flagsPlaced = 0;
    comboCount = 0;
    comboMultiplier = 1;
    lastRevealTick = 0;
    seed = GetTickCount();

    int placed = 0;
    int attempts = 0;
    while (placed < mines) {
        int r = my_rand() % rows;
        int c = my_rand() % cols;
        attempts++;
        if ((grid[r][c] & CELL_MINE) == 0) {
            int in3x3 = (r >= firstClickY - 1 && r <= firstClickY + 1 && c >= firstClickX - 1 && c <= firstClickX + 1);
            int isExact = (r == firstClickY && c == firstClickX);
            if (!in3x3 || (attempts > 1000 && !isExact) || attempts > 2000) {
                grid[r][c] |= CELL_MINE;
                placed++;
            }
        }
    }

    // Hide Treasure Chests
    int cPlaced = 0;
    attempts = 0;
    while (cPlaced < chestsToPlace) {
        int r = my_rand() % rows;
        int c = my_rand() % cols;
        attempts++;
        if ((grid[r][c] & (CELL_MINE | CELL_CHEST)) == 0) {
            int in3x3 = (r >= firstClickY - 1 && r <= firstClickY + 1 && c >= firstClickX - 1 && c <= firstClickX + 1);
            int isExact = (r == firstClickY && c == firstClickX);
            if (!in3x3 || (attempts > 1000 && !isExact) || attempts > 2000) {
                grid[r][c] |= CELL_CHEST;
                cPlaced++;
            }
        }
    }

    initialized = 1;
    totalPlayed++;
    SaveBest();
    SetTimer(mainHwnd, 1, 1000, NULL);
    SetTimer(mainHwnd, 2, 33, NULL);
}

void InitCampaignLevel(HWND hwnd) {
    if (campaignLevel >= 1 && campaignLevel <= 20) {
        StageDef s = campaignStages[campaignLevel - 1];
        rows = s.r; cols = s.c; mines = s.m; chestsToPlace = s.chests;
        if (s.speedrun > 0) {
            isSpeedrun = 1;
            speedrunTime = s.speedrun;
        } else {
            isSpeedrun = 0;
            speedrunTime = 0;
        }
        shields = campaignLevel >= 5 ? (campaignLevel >= 11 ? 3 : 2) : 1;
        detectors = 2 + (campaignLevel / 4);
        sonars = 1 + (campaignLevel / 3);
    } else {
        campaignMode = 0;
        rows = 9; cols = 9; mines = 10; currentDiff = 0;
        shields = 0; detectors = 0; sonars = 0; isSpeedrun = 0; chestsToPlace = 0;
        MessageBoxA(hwnd, "Campaign Complete! You cleared all 20 Stages!", "VICTORY!", MB_OK);
    }

    initialized = 0; gameOver = 0; timeElapsed = 0; flagsPlaced = 0; memset(grid, 0, sizeof(grid));
    void ResizeWindow(HWND);
    ResizeWindow(hwnd);
    InvalidateRect(hwnd, NULL, FALSE);
}

int CountMines(int r, int c) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int nr = r + i, nc = c + j;
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                if (grid[nr][nc] & CELL_MINE) count++;
            }
        }
    }
    return count;
}

int CheckWin();

void Reveal(int startR, int startC) {
    int q[1600][2];
    int head = 0, tail = 0;
    
    if (startR < 0 || startR >= rows || startC < 0 || startC >= cols) return;
    if (grid[startR][startC] & (CELL_REVEALED | CELL_FLAGGED)) return;

    q[tail][0] = startR;
    q[tail][1] = startC;
    tail++;

    while (head < tail) {
        int r = q[head][0];
        int c = q[head][1];
        head++;

        if (grid[r][c] & (CELL_REVEALED | CELL_FLAGGED)) continue;
        grid[r][c] |= CELL_REVEALED;

        // Rapid-Clear Combo logic
        DWORD now = GetTickCount();
        if (lastRevealTick > 0 && (now - lastRevealTick <= 2000)) {
            comboCount++;
        } else {
            comboCount = 1;
        }
        lastRevealTick = now;
        comboMultiplier = 1 + (comboCount / 3);
        if (comboMultiplier > 8) comboMultiplier = 8;

        if (rushMode) {
            rushScore += 10 * comboMultiplier;
        }

        // Treasure Chest Uncovered!
        if (grid[r][c] & CELL_CHEST) {
            grid[r][c] &= ~CELL_CHEST;
            int px = c * CELL_SIZE + CELL_SIZE / 2;
            int py = r * CELL_SIZE + HEADER_HEIGHT + CELL_SIZE / 2;
            SpawnTreasureFX((float)px, (float)py);

            int reward = my_rand() % 100;
            if (reward < 35) {
                sonars++;
                wsprintfA(statusMsg, "CHEST: +1 SONAR SCAN! (R)");
            } else if (reward < 70) {
                detectors++;
                wsprintfA(statusMsg, "CHEST: +1 DETECTOR! (D)");
            } else if (reward < 90) {
                shields++;
                wsprintfA(statusMsg, "CHEST: +1 BLAST SHIELD! (S)");
            } else {
                if (isSpeedrun) {
                    speedrunTime += 15;
                    wsprintfA(statusMsg, "CHEST: +15 SECONDS!");
                } else {
                    rushScore += 500;
                    wsprintfA(statusMsg, "CHEST: +500 BONUS SCORE!");
                }
            }
            statusMsgTime = GetTickCount() + 2500;
            Beep(1800, 80); Beep(2400, 120);
        }
        
        if (grid[r][c] & CELL_MINE) {
            int px = c * CELL_SIZE + CELL_SIZE / 2;
            int py = r * CELL_SIZE + HEADER_HEIGHT + CELL_SIZE / 2;
            SpawnExplosion((float)px, (float)py);

            if (shields > 0) {
                shields--;
                grid[r][c] &= ~CELL_MINE;
                mines--;
                wsprintfA(statusMsg, "BLAST SHIELD ABSORBED MINE!");
                statusMsgTime = GetTickCount() + 2000;
                Beep(500, 100);
            } else {
                gameOver = 1;
                KillTimer(mainHwnd, 1);
                Beep(200, 500);
                return;
            }
        }
        
        if (!(grid[r][c] & CELL_MINE) && CountMines(r, c) == 0) {
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    int nr = r + i, nc = c + j;
                    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                        if (!(grid[nr][nc] & (CELL_REVEALED | CELL_FLAGGED))) {
                            if (tail < 1600) {
                                q[tail][0] = nr;
                                q[tail][1] = nc;
                                tail++;
                            }
                        }
                    }
                }
            }
        }
    }
}

// Power-up Abilities
void UseSonarScan(HWND hwnd) {
    if (sonars <= 0 || !initialized || gameOver) return;
    int targetR = -1, targetC = -1;
    for (int i = 0; i < 1000; i++) {
        int r = my_rand() % rows;
        int c = my_rand() % cols;
        if (!(grid[r][c] & CELL_REVEALED)) {
            targetR = r; targetC = c;
            break;
        }
    }
    if (targetR != -1) {
        sonars--;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                int nr = targetR + i, nc = targetC + j;
                if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                    if (!(grid[nr][nc] & CELL_MINE)) {
                        if (!(grid[nr][nc] & CELL_REVEALED) && !(grid[nr][nc] & CELL_FLAGGED)) {
                            Reveal(nr, nc);
                        }
                    }
                }
            }
        }
        Beep(2000, 120);
        sonarTick = GetTickCount();
        TriggerScreenShake(6, 300);
        wsprintfA(statusMsg, "SONAR SCAN ACTIVATED! (3x3)");
        statusMsgTime = GetTickCount() + 2000;
        InvalidateRect(hwnd, NULL, FALSE);
        if (CheckWin() && !gameOver) {
            gameOver = 1;
            KillTimer(hwnd, 1);
            Beep(1500, 300);
            totalWins++; SaveBest();
            MessageBoxA(hwnd, "Stage Complete! Next Stage...", "Campaign", MB_OK);
            campaignLevel++; InitCampaignLevel(hwnd);
        }
    }
}

void UseDetectorBot(HWND hwnd) {
    if (detectors <= 0 || !initialized || gameOver) return;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if ((grid[r][c] & CELL_MINE) && !(grid[r][c] & CELL_FLAGGED) && !(grid[r][c] & CELL_REVEALED)) {
                grid[r][c] |= CELL_FLAGGED;
                flagTick[r][c] = GetTickCount();
                flagsPlaced++;
                SpawnDustFX((float)(c * CELL_SIZE + CELL_SIZE/2), (float)(r * CELL_SIZE + HEADER_HEIGHT + CELL_SIZE - 2));
                detectors--;
                detectorTick = GetTickCount();
                detectorR = r;
                detectorC = c;
                TriggerScreenShake(5, 250);
                Beep(1400, 100);
                wsprintfA(statusMsg, "FLAG-BOT AUTO-FLAGGED MINE!");
                statusMsgTime = GetTickCount() + 2000;
                InvalidateRect(hwnd, NULL, FALSE);
                return;
            }
        }
    }
}

void UseBlastShield(HWND hwnd) {
    if (shields <= 0 || !initialized || gameOver) return;
    wsprintfA(statusMsg, "BLAST SHIELD ACTIVE (%d CHARGES)", shields);
    statusMsgTime = GetTickCount() + 2000;
    TriggerScreenShake(4, 200);
    Beep(1200, 100);
}

// Graphics Rendering
void DrawSmiley(HDC hdc, int cx, int cy, int size, int state) {
    int r = size / 2;
    HBRUSH hbrFace = CreateSolidBrush(RGB(255, 215, 0));
    HPEN hPenOutline = CreatePen(PS_SOLID, 2, RGB(180, 130, 0));
    HGDIOBJ oldBr = SelectObject(hdc, hbrFace);
    HGDIOBJ oldPen = SelectObject(hdc, hPenOutline);
    
    Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
    
    HPEN hPenShine = CreatePen(PS_SOLID, 1, RGB(255, 255, 200));
    SelectObject(hdc, hPenShine);
    Arc(hdc, cx - r + 3, cy - r + 3, cx + r - 3, cy + r - 3, cx + r, cy - r, cx - r, cy - r);
    DeleteObject(hPenShine);

    if (state == 1) { // Shocked
        HBRUSH hbrEye = CreateSolidBrush(RGB(20, 20, 20));
        SelectObject(hdc, hbrEye);
        SelectObject(hdc, GetStockObject(NULL_PEN));
        Ellipse(hdc, cx - 6, cy - 6, cx - 2, cy - 2);
        Ellipse(hdc, cx + 2, cy - 6, cx + 6, cy - 2);
        Ellipse(hdc, cx - 4, cy + 1, cx + 4, cy + 9);
        DeleteObject(hbrEye);
    } else if (state == 2) { // Dead
        HPEN hPenX = CreatePen(PS_SOLID, 2, RGB(60, 40, 20));
        SelectObject(hdc, hPenX);
        MoveToEx(hdc, cx - 7, cy - 6, NULL); LineTo(hdc, cx - 3, cy - 2);
        MoveToEx(hdc, cx - 3, cy - 6, NULL); LineTo(hdc, cx - 7, cy - 2);
        MoveToEx(hdc, cx + 3, cy - 6, NULL); LineTo(hdc, cx + 7, cy - 2);
        MoveToEx(hdc, cx + 7, cy - 6, NULL); LineTo(hdc, cx + 3, cy - 2);
        Arc(hdc, cx - 6, cy + 2, cx + 6, cy + 12, cx + 6, cy + 6, cx - 6, cy + 6);
        DeleteObject(hPenX);
    } else if (state == 3) { // Cool Win
        HBRUSH hbrGlass = CreateSolidBrush(RGB(30, 30, 35));
        HPEN hPenGlass = CreatePen(PS_SOLID, 1, RGB(10, 10, 10));
        SelectObject(hdc, hbrGlass);
        SelectObject(hdc, hPenGlass);
        POINT ptL[4] = { {cx - 9, cy - 6}, {cx - 1, cy - 6}, {cx - 2, cy + 1}, {cx - 8, cy + 1} };
        POINT ptR[4] = { {cx + 1, cy - 6}, {cx + 9, cy - 6}, {cx + 8, cy + 1}, {cx + 2, cy + 1} };
        Polygon(hdc, ptL, 4); Polygon(hdc, ptR, 4);
        MoveToEx(hdc, cx - 1, cy - 4, NULL); LineTo(hdc, cx + 1, cy - 4);
        DeleteObject(hbrGlass); DeleteObject(hPenGlass);
        HPEN hPenSmirk = CreatePen(PS_SOLID, 2, RGB(40, 30, 0));
        SelectObject(hdc, hPenSmirk);
        Arc(hdc, cx - 5, cy + 1, cx + 6, cy + 8, cx - 5, cy + 4, cx + 5, cy + 6);
        DeleteObject(hPenSmirk);
    } else { // Idle
        HBRUSH hbrEye = CreateSolidBrush(RGB(20, 20, 20));
        SelectObject(hdc, hbrEye); SelectObject(hdc, GetStockObject(NULL_PEN));
        Ellipse(hdc, cx - 6, cy - 6, cx - 2, cy - 2);
        Ellipse(hdc, cx + 2, cy - 6, cx + 6, cy - 2);
        DeleteObject(hbrEye);
        HPEN hPenSmile = CreatePen(PS_SOLID, 2, RGB(40, 30, 0));
        SelectObject(hdc, hPenSmile);
        Arc(hdc, cx - 6, cy - 3, cx + 6, cy + 7, cx + 6, cy + 3, cx - 6, cy + 3);
        DeleteObject(hPenSmile);
    }

    SelectObject(hdc, oldBr); SelectObject(hdc, oldPen);
    DeleteObject(hbrFace); DeleteObject(hPenOutline);
}

void DrawNumberSprite(HDC hdc, int x, int y, int size, int m) {
    char szNum[2] = { (char)(m + '0'), 0 };
    RECT rcCell = { x, y, x + size, y + size };
    HFONT hFont;
    COLORREF c;
    switch(m) {
        case 1: c = RGB(56, 189, 248); hFont = CreateFontA(size-4, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas"); break;
        case 2: c = RGB(74, 222, 128); hFont = CreateFontA(size-2, 0, 0, 0, FW_HEAVY, FALSE, TRUE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Georgia"); break;
        case 3: c = RGB(248, 113, 113); hFont = CreateFontA(size, 0, 0, 0, FW_BLACK, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Impact"); break;
        case 4: c = RGB(192, 132, 252); hFont = CreateFontA(size-4, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Comic Sans MS"); break;
        case 5: c = RGB(250, 204, 21); hFont = CreateFontA(size-2, 0, 0, 0, FW_BOLD, FALSE, FALSE, TRUE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Times New Roman"); break;
        case 6: c = RGB(45, 212, 191); hFont = CreateFontA(size-4, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Courier New"); break;
        case 7: c = RGB(226, 232, 240); hFont = CreateFontA(size-3, 0, 0, 0, FW_MEDIUM, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Tahoma"); break;
        default: c = RGB(148, 163, 184); hFont = CreateFontA(size-4, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial"); break;
    }
    
    HGDIOBJ oldFont = SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);
    
    SetTextColor(hdc, RGB(20, 20, 20));
    RECT rcShadow = { x+2, y+2, x + size+2, y + size+2 };
    DrawTextA(hdc, szNum, 1, &rcShadow, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    SetTextColor(hdc, c);
    DrawTextA(hdc, szNum, 1, &rcCell, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    SelectObject(hdc, oldFont);
    DeleteObject(hFont);
}

void DrawMineSprite(HDC hdc, int x, int y, int size, int isDetonated, DWORD tick) {
    int cx = x + size / 2, cy = y + size / 2, r = size / 3;
    int style = ((x * 13 + y * 7) % 3);

    if (isDetonated) {
        HBRUSH hbrGlow = CreateSolidBrush(RGB(240, 50, 50));
        RECT rcCell = { x, y, x + size, y + size };
        FillRect(hdc, &rcCell, hbrGlow);
        DeleteObject(hbrGlow);
    }

    if (style == 0) {
        // Spiked rusty mine
        HPEN hPenSpike = CreatePen(PS_SOLID, 2, RGB(25, 25, 30));
        HGDIOBJ oldPen = SelectObject(hdc, hPenSpike);
        MoveToEx(hdc, cx - r - 2, cy, NULL); LineTo(hdc, cx + r + 2, cy);
        MoveToEx(hdc, cx, cy - r - 2, NULL); LineTo(hdc, cx, cy + r + 2);
        MoveToEx(hdc, cx - r + 1, cy - r + 1, NULL); LineTo(hdc, cx + r - 1, cy + r - 1);
        MoveToEx(hdc, cx + r - 1, cy - r + 1, NULL); LineTo(hdc, cx - r + 1, cy + r - 1);
        
        HBRUSH hbrBody = CreateSolidBrush(RGB(50, 45, 40));
        HPEN hPenBody = CreatePen(PS_SOLID, 1, RGB(15, 12, 10));
        HGDIOBJ oldBr = SelectObject(hdc, hbrBody);
        SelectObject(hdc, hPenBody);
        Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
        
        HBRUSH hbrRust = CreateSolidBrush(RGB(150, 70, 20));
        SelectObject(hdc, hbrRust);
        SelectObject(hdc, GetStockObject(NULL_PEN));
        Ellipse(hdc, cx - r/2, cy - r/2, cx - r/4, cy - r/4);
        Ellipse(hdc, cx + r/4, cy + r/4, cx + r/2, cy + r/2);
        
        SelectObject(hdc, oldBr);
        SelectObject(hdc, oldPen);
        DeleteObject(hbrBody); DeleteObject(hPenBody); DeleteObject(hPenSpike); DeleteObject(hbrRust);
    } else if (style == 1) {
        // Hi-tech glowing mine
        HBRUSH hbrBody = CreateSolidBrush(RGB(20, 24, 30));
        HPEN hPenBody = CreatePen(PS_SOLID, 1, RGB(10, 15, 20));
        HGDIOBJ oldBr = SelectObject(hdc, hbrBody);
        HGDIOBJ oldPen = SelectObject(hdc, hPenBody);
        
        Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
        
        int glowState = (tick / 200) % 2;
        COLORREF glowColor = glowState ? RGB(56, 189, 248) : RGB(14, 116, 144);
        HBRUSH hbrGlowInner = CreateSolidBrush(glowColor);
        SelectObject(hdc, hbrGlowInner);
        SelectObject(hdc, GetStockObject(NULL_PEN));
        Ellipse(hdc, cx - r/2 + 1, cy - r/2 + 1, cx + r/2 - 1, cy + r/2 - 1);
        
        SelectObject(hdc, oldBr);
        SelectObject(hdc, oldPen);
        DeleteObject(hbrBody); DeleteObject(hPenBody); DeleteObject(hbrGlowInner);
    } else {
        // Biohazard slime mine
        HBRUSH hbrBody = CreateSolidBrush(RGB(30, 45, 20));
        HPEN hPenBody = CreatePen(PS_SOLID, 2, RGB(10, 20, 10));
        HGDIOBJ oldBr = SelectObject(hdc, hbrBody);
        HGDIOBJ oldPen = SelectObject(hdc, hPenBody);
        
        Ellipse(hdc, cx - r - 1, cy - r, cx + r + 1, cy + r);
        
        HBRUSH hbrSlime = CreateSolidBrush(RGB(90, 220, 60));
        SelectObject(hdc, hbrSlime);
        SelectObject(hdc, GetStockObject(NULL_PEN));
        int pulsate = (tick / 150) % 3;
        Ellipse(hdc, cx - r/2 - pulsate, cy - r/2, cx - r/4, cy);
        Ellipse(hdc, cx + r/4, cy, cx + r/2 + pulsate, cy + r/2);
        
        SelectObject(hdc, oldBr);
        SelectObject(hdc, oldPen);
        DeleteObject(hbrBody); DeleteObject(hPenBody); DeleteObject(hbrSlime);
    }
}

void DrawChestSprite(HDC hdc, int x, int y, int size) {
    int cx = x + size / 2, cy = y + size / 2;
    HBRUSH hbrGold = CreateSolidBrush(RGB(245, 180, 30));
    HPEN hPenDark = CreatePen(PS_SOLID, 1, RGB(120, 80, 10));
    HGDIOBJ oldBr = SelectObject(hdc, hbrGold);
    HGDIOBJ oldPen = SelectObject(hdc, hPenDark);
    
    // Chest Box
    Rectangle(hdc, cx - 7, cy - 4, cx + 7, cy + 6);
    // Lid
    HBRUSH hbrLid = CreateSolidBrush(RGB(255, 215, 0));
    SelectObject(hdc, hbrLid);
    RoundRect(hdc, cx - 8, cy - 8, cx + 8, cy - 2, 4, 4);
    SelectObject(hdc, hbrGold);
    DeleteObject(hbrLid);
    // Lock
    HBRUSH hbrLock = CreateSolidBrush(RGB(50, 50, 60));
    SelectObject(hdc, hbrLock);
    Ellipse(hdc, cx - 2, cy - 3, cx + 2, cy + 1);
    SelectObject(hdc, oldBr);
    DeleteObject(hbrLock);

    SelectObject(hdc, oldPen);
    DeleteObject(hbrGold); DeleteObject(hPenDark);
}

void DrawFlagSprite(HDC hdc, int x, int y, int size, DWORD tick, DWORD placedTick) {
    int cx = x + size / 2;
    int elapsed = tick - placedTick;
    
    int dropY = 0;
    if (elapsed < 300) {
        float t = (float)elapsed / 300.0f;
        dropY = (int)(-40.0f * (1.0f - t) * (1.0f - t));
    } else if (elapsed < 400) {
        float t = (float)(elapsed - 300) / 100.0f;
        dropY = (int)(-5.0f * (1.0f - t) * t * 4.0f);
    }
    
    float scale = 1.0f;
    int th = (int)(size * scale);
    int by = y + size - 2 + dropY;
    
    HBRUSH hbrBase = CreateSolidBrush(RGB(60, 60, 70));
    HPEN hPenBase = CreatePen(PS_SOLID, 1, RGB(30, 30, 35));
    HGDIOBJ oldBr = SelectObject(hdc, hbrBase);
    HGDIOBJ oldPen = SelectObject(hdc, hPenBase);
    Ellipse(hdc, cx - (int)(6*scale), by - (int)(4*scale), cx + (int)(6*scale), by);

    HPEN hPenPole = CreatePen(PS_SOLID, 2, RGB(200, 205, 215));
    SelectObject(hdc, hPenPole);
    DeleteObject(hPenBase);
    MoveToEx(hdc, cx - (int)(3*scale), by - th + (int)(4*scale), NULL); LineTo(hdc, cx - (int)(3*scale), by);

    int waveOffset = (int)(((tick / 150) % 2) * 2 * scale);
    POINT pts[3] = { 
        { cx - (int)(2*scale), by - th + (int)(5*scale) }, 
        { cx + (int)((size / 2 + 3)*scale) + waveOffset, by - th + (int)(9*scale) }, 
        { cx - (int)(2*scale), by - th + (int)(15*scale) } 
    };
    HBRUSH hbrCloth = CreateSolidBrush(RGB(240, 45, 65));
    HPEN hPenCloth = CreatePen(PS_SOLID, 1, RGB(160, 20, 35));
    SelectObject(hdc, hbrCloth); 
    SelectObject(hdc, hPenCloth);
    DeleteObject(hbrBase);
    DeleteObject(hPenPole);
    Polygon(hdc, pts, 3);
    
    SelectObject(hdc, oldBr); 
    SelectObject(hdc, oldPen);
    DeleteObject(hbrCloth); 
    DeleteObject(hPenCloth);
}

void DrawQuestionSprite(HDC hdc, int x, int y, int size) {
    RECT rc = { x, y, x + size, y + size };
    HFONT hFont = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    HGDIOBJ oldFont = SelectObject(hdc, hFont);
    SetTextColor(hdc, RGB(56, 189, 248));
    DrawTextA(hdc, "?", 1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont); DeleteObject(hFont);
}

void DrawScorchMarks(HDC hdc, int x, int y, int size, int m) {
    if (m < 3) return;
    int seedVal = (x * 73 + y * 37) % 100;
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(10, 12, 16));
    HGDIOBJ oldPen = SelectObject(hdc, hPen);
    int cx = x + size / 2, cy = y + size / 2;
    for(int i = 0; i < m; i++) {
        int angleIdx = (seedVal + i * 3) % 16;
        int len = 4 + ((seedVal * i) % (size/2 - 2));
        int dx = (FastCos(angleIdx) * len) / 100;
        int dy = (FastSin(angleIdx) * len) / 100;
        MoveToEx(hdc, cx, cy, NULL);
        LineTo(hdc, cx + dx, cy + dy);
    }
    SelectObject(hdc, oldPen);
    DeleteObject(hPen);
}

void Draw3DTile(HDC hdc, int x, int y, int size, int isRevealed, int isPressed) {
    RECT rc = { x, y, x + size, y + size };
    if (isRevealed) {
        HBRUSH hbrRev = CreateSolidBrush(RGB(22, 24, 36));
        FillRect(hdc, &rc, hbrRev); DeleteObject(hbrRev);
        
        // Terrain texture (dirt/rock details)
        int seedVal = (x * 73 + y * 37) % 100;
        for(int i = 0; i < 6; i++) {
            int dx = (seedVal * (i + 1) * 13) % size;
            int dy = (seedVal * (i + 1) * 17) % size;
            int dotColor = ((i % 2) == 0) ? RGB(18, 20, 30) : RGB(28, 32, 46);
            if (i == 5) dotColor = RGB(38, 44, 62); // subtle highlight
            SetPixel(hdc, x + dx, y + dy, dotColor);
            SetPixel(hdc, x + dx + 1, y + dy, dotColor);
        }

        // Specular sheen diagonal tick
        SetPixel(hdc, x + 2, y + 2, RGB(60, 70, 95));
        SetPixel(hdc, x + 3, y + 2, RGB(60, 70, 95));
        SetPixel(hdc, x + 2, y + 3, RGB(60, 70, 95));

        HPEN hDarkInner = CreatePen(PS_SOLID, 1, RGB(12, 14, 22));
        HGDIOBJ oldPen = SelectObject(hdc, hDarkInner);
        MoveToEx(hdc, x, y + size - 1, NULL); LineTo(hdc, x, y); LineTo(hdc, x + size - 1, y);
        SelectObject(hdc, oldPen); DeleteObject(hDarkInner);
    } else {
        HBRUSH hbrUnrev = CreateSolidBrush(RGB(61, 70, 99));
        FillRect(hdc, &rc, hbrUnrev); DeleteObject(hbrUnrev);
        
        // Subtle unrevealed cyber pattern
        int seedVal = (x * 19 + y * 53) % 100;
        for(int i = 0; i < 4; i++) {
            int dx = (seedVal * (i + 1) * 23) % size;
            int dy = (seedVal * (i + 1) * 29) % size;
            SetPixel(hdc, x + dx, y + dy, RGB(75, 88, 120));
            SetPixel(hdc, x + dx + 1, y + dy + 1, RGB(45, 52, 75));
        }

        HPEN hWhiteHi = CreatePen(PS_SOLID, 2, RGB(145, 165, 205));
        HPEN hDarkLo  = CreatePen(PS_SOLID, 2, RGB(25, 30, 45));
        HGDIOBJ oldPen = SelectObject(hdc, hWhiteHi);
        MoveToEx(hdc, x, y + size - 1, NULL); LineTo(hdc, x, y); LineTo(hdc, x + size - 1, y);
        SelectObject(hdc, hDarkLo);
        LineTo(hdc, x + size - 1, y + size - 1); LineTo(hdc, x - 1, y + size - 1);
        SelectObject(hdc, oldPen);
        DeleteObject(hWhiteHi); DeleteObject(hDarkLo);
    }
}

// Ornate Cybernetic Corner Filigree L-Brackets with Tech Rivets
void DrawCornerBracket(HDC hdc, int x, int y, int alignX, int alignY) {
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(122, 162, 247));
    HGDIOBJ oldPen = SelectObject(hdc, hPen);
    
    int len = 10;
    int armX = alignX ? -len : len;
    int armY = alignY ? -len : len;

    MoveToEx(hdc, x + armX, y, NULL);
    LineTo(hdc, x, y);
    LineTo(hdc, x, y + armY);

    SelectObject(hdc, oldPen);
    DeleteObject(hPen);

    // Glowing Gold Tech Rivet Dot
    int rx = alignX ? x - 3 : x + 2;
    int ry = alignY ? y - 3 : y + 2;
    SetPixel(hdc, rx, ry, RGB(245, 215, 0));
    SetPixel(hdc, rx + 1, ry, RGB(245, 215, 0));
    SetPixel(hdc, rx, ry + 1, RGB(245, 215, 0));
    SetPixel(hdc, rx + 1, ry + 1, RGB(245, 215, 0));
}

void DrawBoardToDC(HWND hwnd, HDC hdc) {
    DWORD tick = GetTickCount();
    
    RECT rcFull = { 0, 0, cols * CELL_SIZE, rows * CELL_SIZE + HEADER_HEIGHT };
    HBRUSH hbrBg = CreateSolidBrush(RGB(18, 20, 29));
    FillRect(hdc, &rcFull, hbrBg); DeleteObject(hbrBg);

    // Metallic frame around the entire window
    HPEN hFrameHi = CreatePen(PS_SOLID, 3, RGB(180, 195, 225));
    HPEN hFrameLo = CreatePen(PS_SOLID, 3, RGB(35, 45, 65));
    HGDIOBJ oldPenF = SelectObject(hdc, hFrameHi);
    MoveToEx(hdc, 0, rcFull.bottom, NULL); LineTo(hdc, 0, 0); LineTo(hdc, rcFull.right, 0);
    SelectObject(hdc, hFrameLo);
    LineTo(hdc, rcFull.right, rcFull.bottom); LineTo(hdc, 0, rcFull.bottom);
    SelectObject(hdc, oldPenF);
    DeleteObject(hFrameHi); DeleteObject(hFrameLo);

    RECT rcHeader = { 0, 0, cols * CELL_SIZE, HEADER_HEIGHT };
    HBRUSH hbrHeader = CreateSolidBrush(RGB(38, 44, 58));
    FillRect(hdc, &rcHeader, hbrHeader); DeleteObject(hbrHeader);

    HPEN hHeaderBorder = CreatePen(PS_SOLID, 2, RGB(55, 65, 90));
    HGDIOBJ oldPen = SelectObject(hdc, hHeaderBorder);
    MoveToEx(hdc, 0, HEADER_HEIGHT - 1, NULL); LineTo(hdc, cols * CELL_SIZE, HEADER_HEIGHT - 1);
    SelectObject(hdc, oldPen); DeleteObject(hHeaderBorder);

    HFONT hLcdFont = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Courier New");
    HGDIOBJ oldFont = SelectObject(hdc, hLcdFont);
    SetBkMode(hdc, OPAQUE); SetBkColor(hdc, RGB(9, 10, 15));

    char szMines[16];
    wsprintfA(szMines, "%03d", mines - flagsPlaced);
    RECT rcMineBox = { 10, 10, 60, 38 };
    HBRUSH hbrLcd = CreateSolidBrush(RGB(9, 10, 15));
    FillRect(hdc, &rcMineBox, hbrLcd);
    
    // Bevel around Mine Box
    HPEN hBoxLo = CreatePen(PS_SOLID, 2, RGB(18, 22, 32));
    HPEN hBoxHi = CreatePen(PS_SOLID, 2, RGB(75, 85, 110));
    SelectObject(hdc, hBoxLo);
    MoveToEx(hdc, 10, 38, NULL); LineTo(hdc, 10, 10); LineTo(hdc, 60, 10);
    SelectObject(hdc, hBoxHi);
    LineTo(hdc, 60, 38); LineTo(hdc, 10, 38);

    SetTextColor(hdc, RGB(247, 118, 142));
    DrawTextA(hdc, szMines, -1, &rcMineBox, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    char szTime[16];
    int displayTime = rushMode ? rushTime : (isSpeedrun ? speedrunTime : timeElapsed);
    wsprintfA(szTime, "%03d", displayTime);
    RECT rcTimeBox = { cols * CELL_SIZE - 60, 10, cols * CELL_SIZE - 10, 38 };
    FillRect(hdc, &rcTimeBox, hbrLcd);
    
    // Bevel around Time Box
    SelectObject(hdc, hBoxLo);
    MoveToEx(hdc, rcTimeBox.left, 38, NULL); LineTo(hdc, rcTimeBox.left, 10); LineTo(hdc, rcTimeBox.right, 10);
    SelectObject(hdc, hBoxHi);
    LineTo(hdc, rcTimeBox.right, 38); LineTo(hdc, rcTimeBox.left, 38);

    SetTextColor(hdc, isSpeedrun ? RGB(250, 204, 21) : RGB(122, 162, 247));
    DrawTextA(hdc, szTime, -1, &rcTimeBox, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DeleteObject(hbrLcd);
    DeleteObject(hBoxLo); DeleteObject(hBoxHi);

    int faceCx = cols * CELL_SIZE / 2;
    int faceCy = 24;
    int smileyState = 0;
    if (gameOver) smileyState = 2;
    else if (CheckWin()) smileyState = 3;
    else if (mouseCellPressed) smileyState = 1;
    DrawSmiley(hdc, faceCx, faceCy, 30, smileyState);

    // Mode & Power-up Sub-Header text
    SetBkMode(hdc, TRANSPARENT);
    HFONT hSubFont = CreateFontA(11, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    SelectObject(hdc, hSubFont);
    RECT rcMode = { 0, 42, cols * CELL_SIZE, HEADER_HEIGHT - 2 };
    char szMode[128];
    if (tick < statusMsgTime) {
        SetTextColor(hdc, RGB(255, 235, 120));
        DrawTextA(hdc, statusMsg, -1, &rcMode, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    } else {
        SetTextColor(hdc, RGB(180, 190, 210));
        if (rushMode) {
            wsprintfA(szMode, "RUSH MODE - Score:%d (x%d Combo!) Best:%d", rushScore, comboMultiplier, bestRush);
        } else if (campaignMode) {
            wsprintfA(szMode, "STAGE %d/20 %s| Sh[S]:%d Det[D]:%d Son[R]:%d", campaignLevel, isSpeedrun ? "[TIME] " : "", shields, detectors, sonars);
        } else {
            wsprintfA(szMode, "1-Ez 2-Md 3-Hd C-Camp(20) 4-Rush | Sh[S]:%d Det[D]:%d Son[R]:%d", shields, detectors, sonars);
        }
        DrawTextA(hdc, szMode, -1, &rcMode, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    
    SelectObject(hdc, hLcdFont);
    DeleteObject(hSubFont);

    // Grid Cells
    HFONT hNumFont = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    SelectObject(hdc, hNumFont);

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int x = c * CELL_SIZE;
            int y = r * CELL_SIZE + HEADER_HEIGHT;
            
            int isRev = (grid[r][c] & CELL_REVEALED);
            Draw3DTile(hdc, x, y, CELL_SIZE, isRev, 0);

            if (isRev) {
                if (grid[r][c] & CELL_MINE) {
                    DrawMineSprite(hdc, x, y, CELL_SIZE, 1, tick);
                } else {
                    int m = CountMines(r, c);
                    DrawScorchMarks(hdc, x, y, CELL_SIZE, m);
                    if (m > 0) {
                        DrawNumberSprite(hdc, x, y, CELL_SIZE, m);
                    }
                }
            } else {
                if (grid[r][c] & CELL_FLAGGED) {
                    DrawFlagSprite(hdc, x, y, CELL_SIZE, tick, flagTick[r][c]);
                } else if (grid[r][c] & CELL_QUESTION) {
                    DrawQuestionSprite(hdc, x, y, CELL_SIZE);
                } else if ((grid[r][c] & CELL_CHEST) && gameOver) {
                    DrawChestSprite(hdc, x, y, CELL_SIZE);
                }
            }
        }
    }
    SelectObject(hdc, oldFont);
    DeleteObject(hNumFont);
    DeleteObject(hLcdFont);

    // Ornate Cybernetic Corner Filigree L-Brackets on Board Corners
    DrawCornerBracket(hdc, 2, HEADER_HEIGHT + 2, 0, 0);
    DrawCornerBracket(hdc, cols * CELL_SIZE - 2, HEADER_HEIGHT + 2, 1, 0);
    DrawCornerBracket(hdc, 2, rows * CELL_SIZE + HEADER_HEIGHT - 2, 0, 1);
    DrawCornerBracket(hdc, cols * CELL_SIZE - 2, rows * CELL_SIZE + HEADER_HEIGHT - 2, 1, 1);

    // Pulsating Perimeter Inlay Border with Traveling Specular Glint
    int glintPerim = 2 * (cols * CELL_SIZE + rows * CELL_SIZE);
    int glintPos = (int)((tick / 15) % glintPerim);
    int gx = 0, gy = HEADER_HEIGHT;
    if (glintPos < cols * CELL_SIZE) { gx = glintPos; gy = HEADER_HEIGHT; }
    else if (glintPos < cols * CELL_SIZE + rows * CELL_SIZE) { gx = cols * CELL_SIZE - 1; gy = HEADER_HEIGHT + glintPos - cols * CELL_SIZE; }
    else if (glintPos < 2 * cols * CELL_SIZE + rows * CELL_SIZE) { gx = cols * CELL_SIZE - (glintPos - (cols * CELL_SIZE + rows * CELL_SIZE)); gy = rows * CELL_SIZE + HEADER_HEIGHT - 1; }
    else { gx = 0; gy = rows * CELL_SIZE + HEADER_HEIGHT - (glintPos - (2 * cols * CELL_SIZE + rows * CELL_SIZE)); }
    
    // Draw glint node
    SetPixel(hdc, gx, gy, RGB(255, 255, 255));
    SetPixel(hdc, gx + 1, gy, RGB(122, 162, 247));
    SetPixel(hdc, gx - 1, gy, RGB(122, 162, 247));
    SetPixel(hdc, gx, gy + 1, RGB(122, 162, 247));
    SetPixel(hdc, gx, gy - 1, RGB(122, 162, 247));

    DrawParticles(hdc);

    if (shields > 0) {
        int pulse = (tick / 15) % 100;
        if (pulse > 50) pulse = 100 - pulse;
        HPEN hShieldPen = CreatePen(PS_SOLID, 4, RGB(100 + pulse, 150 + pulse, 255));
        HGDIOBJ oldPenS = SelectObject(hdc, hShieldPen);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, 1, HEADER_HEIGHT + 1, cols * CELL_SIZE - 1, rows * CELL_SIZE + HEADER_HEIGHT - 1);
        SelectObject(hdc, oldPenS);
        DeleteObject(hShieldPen);
    }

    if (tick - sonarTick < 1000) {
        int maxR = cols * CELL_SIZE;
        if (rows * CELL_SIZE > maxR) maxR = rows * CELL_SIZE;
        int radius = (int)((tick - sonarTick) * maxR / 1000);
        HPEN hRadarPen = CreatePen(PS_SOLID, 3, RGB(56, 189, 248));
        HGDIOBJ oldPenR = SelectObject(hdc, hRadarPen);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Ellipse(hdc, cols * CELL_SIZE / 2 - radius, rows * CELL_SIZE / 2 + HEADER_HEIGHT - radius, cols * CELL_SIZE / 2 + radius, rows * CELL_SIZE / 2 + HEADER_HEIGHT + radius);
        SelectObject(hdc, oldPenR);
        DeleteObject(hRadarPen);
    }

    if (tick - detectorTick < 1500 && detectorR != -1 && detectorC != -1) {
        HPEN hLaserPen = CreatePen(PS_SOLID, 2, RGB(74, 222, 128));
        HGDIOBJ oldPenL = SelectObject(hdc, hLaserPen);
        MoveToEx(hdc, cols * CELL_SIZE / 2, 0, NULL);
        LineTo(hdc, detectorC * CELL_SIZE + CELL_SIZE / 2, detectorR * CELL_SIZE + HEADER_HEIGHT + CELL_SIZE / 2);
        SelectObject(hdc, oldPenL);
        DeleteObject(hLaserPen);

        // Sweeping scanner line
        int scanY = HEADER_HEIGHT + (int)((tick - detectorTick) * (rows * CELL_SIZE) / 1500);
        HPEN hScanPen = CreatePen(PS_SOLID, 3, RGB(74, 222, 128));
        oldPenL = SelectObject(hdc, hScanPen);
        MoveToEx(hdc, 0, scanY, NULL);
        LineTo(hdc, cols * CELL_SIZE, scanY);
        SelectObject(hdc, oldPenL);
        DeleteObject(hScanPen);
    }
}

// Double-Buffered Render Loop with Continuous Procedural Screen Shake
void DrawBoard(HWND hwnd, HDC hdc) {
    int totalW = cols * CELL_SIZE;
    int totalH = rows * CELL_SIZE + HEADER_HEIGHT;

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBM = CreateCompatibleBitmap(hdc, totalW, totalH);
    HGDIOBJ oldBM = SelectObject(memDC, memBM);

    DrawBoardToDC(hwnd, memDC);

    DWORD tick = GetTickCount();
    DWORD elapsedShake = tick - shakeTick;
    int sx = 0, sy = 0;

    if (shakeDuration > 0 && elapsedShake < shakeDuration && shakeIntensity > 0) {
        float progress = (float)(shakeDuration - elapsedShake) / (float)shakeDuration;
        float decay = progress * progress; // Quadratic physics decay
        int curInt = (int)((float)shakeIntensity * decay);
        if (curInt > 0) {
            sx = (my_rand() % (curInt * 2 + 1)) - curInt;
            sy = (my_rand() % (curInt * 2 + 1)) - curInt;
        }
    }

    BitBlt(hdc, sx, sy, totalW, totalH, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBM);
    DeleteObject(memBM);
    DeleteDC(memDC);
}

int CheckWin() {
    int revealed = 0;
    for(int r=0; r<rows; r++) {
        for(int c=0; c<cols; c++) {
            if (grid[r][c] & CELL_REVEALED) revealed++;
        }
    }
    return (revealed == (rows * cols - mines));
}

void HandleReveal(HWND hwnd, int x, int y) {
    if (gameOver || !initialized) return;
    if (grid[y][x] & CELL_FLAGGED) return;

    if (grid[y][x] & CELL_REVEALED) {
        int flagged = 0;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                int nr = y + i, nc = x + j;
                if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                    if (grid[nr][nc] & CELL_FLAGGED) flagged++;
                }
            }
        }
        if (flagged == CountMines(y, x)) {
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    int nr = y + i, nc = x + j;
                    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                        if (!(grid[nr][nc] & CELL_FLAGGED) && !(grid[nr][nc] & CELL_REVEALED)) {
                            Reveal(nr, nc);
                        }
                    }
                }
            }
        } else {
            return;
        }
    } else {
        Reveal(y, x);
    }

    if (!gameOver) Beep(1000, 20);
    InvalidateRect(hwnd, NULL, FALSE);
    
    if (gameOver) {
        for(int r=0;r<rows;r++) for(int c=0;c<cols;c++) if(grid[r][c]&CELL_MINE) grid[r][c]|=CELL_REVEALED;
        TriggerScreenShake(14, 500);
        InvalidateRect(hwnd, NULL, FALSE);
        MessageBoxA(hwnd, "Boom! Click Smiley to restart.", "Game Over", MB_OK);
    } else if (CheckWin()) {
        gameOver = 1;
        KillTimer(hwnd, 1);
        Beep(1500, 300);
        SpawnVictoryFX();
        totalWins++;
        SaveBest();
        if (!campaignMode && timeElapsed < bestTimes[currentDiff]) {
            bestTimes[currentDiff] = timeElapsed;
            SaveBest();
        }
        if (campaignMode) {
            MessageBoxA(hwnd, "Stage Complete! Next Stage...", "Campaign Victory", MB_OK);
            campaignLevel++;
            InitCampaignLevel(hwnd);
        } else if (rushMode) {
            rushScore += 100 * comboMultiplier;
            rushTime += 15;
            if (rushTime > 999) rushTime = 999;
            if (rushScore > bestRush) bestRush = rushScore;
            SaveBest();
            Beep(1200, 200);
            initialized = 0;
            memset(grid, 0, sizeof(grid));
            gameOver = 0;
            flagsPlaced = 0;
            InvalidateRect(hwnd, NULL, FALSE);
        } else {
            MessageBoxA(hwnd, "You Win! Click Smiley to restart.", "Congratulations", MB_OK);
        }
    }
}

void ResizeWindow(HWND hwnd) {
    RECT rcClient, rcWindow;
    GetClientRect(hwnd, &rcClient);
    GetWindowRect(hwnd, &rcWindow);
    SetWindowPos(hwnd, NULL, 0, 0,
        (rcWindow.right - rcWindow.left) + (cols * CELL_SIZE - (rcClient.right - rcClient.left)),
        (rcWindow.bottom - rcWindow.top) + (rows * CELL_SIZE + HEADER_HEIGHT - (rcClient.bottom - rcClient.top)),
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    InvalidateRect(hwnd, NULL, FALSE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            mainHwnd = hwnd;
            LoadBest();
            InitDustMotes();
            SetTimer(hwnd, 2, 33, NULL);
            break;
        case WM_ERASEBKGND:
            return 1; // Prevent flicker in double buffering
        case WM_TIMER:
            if (wParam == 1 && !gameOver && initialized) {
                if (rushMode) {
                    rushTime--;
                    if (rushTime <= 0) {
                        rushTime = 0; gameOver = 1; KillTimer(hwnd, 1); Beep(200, 500);
                        for(int r=0;r<rows;r++) for(int c=0;c<cols;c++) if(grid[r][c]&CELL_MINE) grid[r][c]|=CELL_REVEALED;
                        TriggerScreenShake(12, 500);
                        InvalidateRect(hwnd, NULL, FALSE);
                        MessageBoxA(hwnd, "Time's Up! Click Smiley to restart.", "Game Over", MB_OK);
                    }
                } else if (isSpeedrun) {
                    speedrunTime--;
                    if (speedrunTime <= 0) {
                        speedrunTime = 0; gameOver = 1; KillTimer(hwnd, 1); Beep(200, 500);
                        for(int r=0;r<rows;r++) for(int c=0;c<cols;c++) if(grid[r][c]&CELL_MINE) grid[r][c]|=CELL_REVEALED;
                        TriggerScreenShake(12, 500);
                        InvalidateRect(hwnd, NULL, FALSE);
                        MessageBoxA(hwnd, "Speedrun Time Expired! Click Smiley to restart.", "Stage Failed", MB_OK);
                    }
                } else {
                    timeElapsed++;
                    if (timeElapsed > 999) timeElapsed = 999;
                }
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 2) {
                UpdateParticles();
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            DrawBoard(hwnd, hdc);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_LBUTTONDOWN: {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            
            int faceCx = cols * CELL_SIZE / 2;
            int faceCy = 24;
            if ((mx - faceCx)*(mx - faceCx) + (my - faceCy)*(my - faceCy) <= 256) {
                if (campaignMode) { InitCampaignLevel(hwnd); return 0; }
                if (rushMode) { rushTime = 60; rushScore = 0; }
                initialized = 0; memset(grid, 0, sizeof(grid)); gameOver = 0;
                if (!rushMode && !isSpeedrun) timeElapsed = 0;
                flagsPlaced = 0; shields = 0; detectors = 0; sonars = 0; particleCount = 0; shockwaveCount = 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }

            if (gameOver) return 0;

            mouseCellPressed = 1;
            InvalidateRect(hwnd, NULL, FALSE);

            int x = mx / CELL_SIZE;
            int y = (my - HEADER_HEIGHT) / CELL_SIZE;
            if (x >= 0 && x < cols && y >= 0 && y < rows && my >= HEADER_HEIGHT) {
                if (!initialized) InitGame(x, y);
                HandleReveal(hwnd, x, y);
            }
            break;
        }
        case WM_LBUTTONUP: {
            mouseCellPressed = 0;
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_RBUTTONDOWN: {
            if (gameOver || !initialized) return 0;
            int x = LOWORD(lParam) / CELL_SIZE;
            int y = (HIWORD(lParam) - HEADER_HEIGHT) / CELL_SIZE;
            if (x >= 0 && x < cols && y >= 0 && y < rows && HIWORD(lParam) >= HEADER_HEIGHT) {
                if (!(grid[y][x] & CELL_REVEALED)) {
                    if (grid[y][x] & CELL_FLAGGED) {
                        grid[y][x] &= ~CELL_FLAGGED;
                        grid[y][x] |= CELL_QUESTION;
                        flagsPlaced--;
                    } else if (grid[y][x] & CELL_QUESTION) {
                        grid[y][x] &= ~CELL_QUESTION;
                    } else {
                        grid[y][x] |= CELL_FLAGGED;
                        flagTick[y][x] = GetTickCount();
                        flagsPlaced++;
                        SpawnDustFX((float)(x * CELL_SIZE + CELL_SIZE/2), (float)(y * CELL_SIZE + HEADER_HEIGHT + CELL_SIZE - 2));
                    }
                    Beep(800, 20);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            break;
        }
        case WM_KEYDOWN:
            if (wParam == '1') { rushMode=0; campaignMode=0; shields=0; detectors=0; sonars=0; isSpeedrun=0; rows=9; cols=9; mines=10; currentDiff=0; initialized=0; gameOver=0; timeElapsed=0; flagsPlaced=0; memset(grid,0,sizeof(grid)); ResizeWindow(hwnd); }
            if (wParam == '2') { rushMode=0; campaignMode=0; shields=0; detectors=0; sonars=0; isSpeedrun=0; rows=16; cols=16; mines=40; currentDiff=1; initialized=0; gameOver=0; timeElapsed=0; flagsPlaced=0; memset(grid,0,sizeof(grid)); ResizeWindow(hwnd); }
            if (wParam == '3') { rushMode=0; campaignMode=0; shields=0; detectors=0; sonars=0; isSpeedrun=0; rows=16; cols=30; mines=99; currentDiff=2; initialized=0; gameOver=0; timeElapsed=0; flagsPlaced=0; memset(grid,0,sizeof(grid)); ResizeWindow(hwnd); }
            if (wParam == '4') { rushMode=1; rushTime=60; rushScore=0; campaignMode=0; shields=0; detectors=0; sonars=0; isSpeedrun=0; rows=9; cols=9; mines=10; currentDiff=3; initialized=0; gameOver=0; timeElapsed=0; flagsPlaced=0; memset(grid,0,sizeof(grid)); ResizeWindow(hwnd); }
            if (wParam == 'C') { rushMode=0; campaignMode = !campaignMode; if (campaignMode) { campaignLevel = 1; InitCampaignLevel(hwnd); } else { campaignMode=0; shields=0; detectors=0; sonars=0; isSpeedrun=0; rows=9; cols=9; mines=10; currentDiff=0; initialized=0; gameOver=0; timeElapsed=0; flagsPlaced=0; memset(grid,0,sizeof(grid)); ResizeWindow(hwnd); } }
            if (wParam == 'R') { UseSonarScan(hwnd); }
            if (wParam == 'D') { UseDetectorBot(hwnd); }
            if (wParam == 'S') { UseBlastShield(hwnd); }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KMinesClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);
    RECT rc = {0, 0, cols * CELL_SIZE, rows * CELL_SIZE + HEADER_HEIGHT};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, FALSE);
    
    HWND hwnd = CreateWindowExA(0, "KMinesClass", "KMines", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
    
    ResizeWindow(hwnd);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
