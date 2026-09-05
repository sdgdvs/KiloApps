#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "msvcrt.lib")

#define CELL_W 70
#define CELL_H 100
#define PAD 20
#define TOP_Y 50
#define TAB_Y 170
#define TAB_PAD_X 10
#define TAB_PAD_Y 25

typedef struct {
    int s; // 0=Spades, 1=Hearts, 2=Clubs, 3=Diamonds
    int r; // 1-13
    int color; // 0=Black, 1=Red
    int frozen; // 0=Normal, 1=Frozen
} Card;

Card deck[52];
#define MAX_FREE_CELLS 6
Card freeCells[MAX_FREE_CELLS];
int freeCellsOccupied[MAX_FREE_CELLS];
int numFreeCells = 4;
int buildRule = 0; // 0=Alt color, 1=Same suit
int emptyKingOnly = 0; // 0=Any card, 1=King only for empty tab
int found[4]; // 0-13

Card tab[8][52];
int tabCount[8];

int selType = -1; // 0=free, 1=tab, -1=none
int selIdx = -1;
int selCardIdx = -1;
int won = 0; // 0=playing, 1=won, -1=lost
int gameInProgress = 0;
int gameMode = 0; // 0=Random, 1=Numbered, 2=Campaign, 3=Time Attack
int currentSeed = 1;

int statsPlayed = 0;
int statsWins = 0;
int statsStreak = 0;
int statsBestStreak = 0;
int statsBestTime = 0;
int campaignStage = 1;
int maxCampaignStage = 1;
int powerupsShuffle = 1;
int powerupsWand = 1;
int powerupsExtraCell = 1;
int settingsCardBack = 0;

int extraCellActive = 0;
int extraCellTimer = 0;

float shakeTrauma = 0.0f;
float shakeMaxTrauma = 0.0f;
int shakeTicks = 0;
int shakeMaxTicks = 0;

void TriggerScreenShake(int intensity) {
    float fIntensity = (float)intensity;
    if (fIntensity > shakeTrauma) {
        shakeTrauma = fIntensity;
        shakeMaxTrauma = fIntensity;
        shakeTicks = 0;
        shakeMaxTicks = (int)(fIntensity * 3.5f + 6.0f);
        if (shakeMaxTicks > 30) shakeMaxTicks = 30;
    }
}

int moves = 0;
int timeElapsed = 0;
int timeRemaining = 180;
int statsTimeAttackWins = 0;
int statsBestTimeAttackTime = 0;
DWORD lastTimeTick = 0;

void LoadSettings() {
    FILE *f = _wfopen(L"kfreecell_settings.dat", L"rb");
    if(f) {
        fread(&settingsCardBack, sizeof(settingsCardBack), 1, f);
        fclose(f);
    }
}

void SaveSettings() {
    FILE *f = _wfopen(L"kfreecell_settings.dat", L"wb");
    if(f) {
        fwrite(&settingsCardBack, sizeof(settingsCardBack), 1, f);
        fclose(f);
    }
}

typedef struct {
    int active;
    Card c;
    int startX, startY;
    int endX, endY;
    DWORD startTime;
    DWORD duration;
} Animation;
#define MAX_ANIMS 52
Animation anims[MAX_ANIMS];

typedef struct {
    int active;
    Card c;
    float x, y;
    float vx, vy;
    DWORD delay;
} CascadeCard;
#define MAX_CASCADE 52
CascadeCard cascadeCards[MAX_CASCADE];
int cascadeActive = 0;

// --- Particle System ---
typedef struct {
    float x, y;
    float vx, vy;
    float alpha;
    float da;
    int size;
} DustMote;
#define MAX_DUST 120
DustMote dustMotes[MAX_DUST];
int dustInit = 0;

#define PARTICLE_SPARK  0
#define PARTICLE_SMOKE  1
#define PARTICLE_DEBRIS 2
#define PARTICLE_STAR   3

typedef struct {
    int active;
    int type; // 0=Spark, 1=Smoke, 2=Debris, 3=Star
    float x, y;
    float vx, vy;
    float life;
    float maxLife;
    float decay;
    COLORREF color;
    int size;
    float spin;
    float spinSpeed;
} Particle;
#define MAX_PARTICLES 500
Particle particles[MAX_PARTICLES];
int fireworksBursts = 0;
DWORD lastBurstTime = 0;

typedef struct {
    float x, y;
    float radius;
    float maxRadius;
    float speed;
    COLORREF color;
    int active;
} Shockwave;
#define MAX_SHOCKWAVES 16
Shockwave shockwaves[MAX_SHOCKWAVES];

void SpawnShockwave(float x, float y, COLORREF color, float maxRadius, float speed) {
    for(int i=0; i<MAX_SHOCKWAVES; i++) {
        if(!shockwaves[i].active) {
            shockwaves[i].active = 1;
            shockwaves[i].x = x;
            shockwaves[i].y = y;
            shockwaves[i].radius = 4.0f;
            shockwaves[i].maxRadius = maxRadius;
            shockwaves[i].speed = speed;
            shockwaves[i].color = color;
            break;
        }
    }
}

void SpawnParticleEx(int type, float x, float y, float vx, float vy, float life, float decay, COLORREF color, int size, float spinSpeed) {
    for(int i=0; i<MAX_PARTICLES; i++) {
        if(!particles[i].active) {
            particles[i].active = 1;
            particles[i].type = type;
            particles[i].x = x; particles[i].y = y;
            particles[i].vx = vx; particles[i].vy = vy;
            particles[i].life = life;
            particles[i].maxLife = life;
            particles[i].decay = decay;
            particles[i].color = color;
            particles[i].size = size;
            particles[i].spin = (float)(rand() % 360);
            particles[i].spinSpeed = spinSpeed;
            break;
        }
    }
}

void SpawnParticle(float x, float y, float vx, float vy, float life, float decay, COLORREF color, int size) {
    SpawnParticleEx(PARTICLE_SPARK, x, y, vx, vy, life, decay, color, size, 0.0f);
}

void Draw4PointStarGDI(HDC hdc, int cx, int cy, int outerR, int innerR, COLORREF col) {
    POINT pts[8];
    for(int i=0; i<8; i++) {
        int r = (i % 2 == 0) ? outerR : innerR;
        float angle = (float)i * 3.14159265f / 4.0f;
        pts[i].x = cx + (int)(cosf(angle) * r);
        pts[i].y = cy + (int)(sinf(angle) * r);
    }
    HBRUSH br = CreateSolidBrush(col);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HBRUSH oldB = (HBRUSH)SelectObject(hdc, br);
    HPEN oldP = (HPEN)SelectObject(hdc, pen);
    Polygon(hdc, pts, 8);
    SelectObject(hdc, oldB);
    SelectObject(hdc, oldP);
    DeleteObject(br);
    DeleteObject(pen);
}

void DrawOrnateHUDCorners(HDC hdc, int winW, int winH) {
    int pad = 20;
    int arm = 30;
    int left = pad;
    int top = pad;
    int right = winW - pad;
    int bottom = winH - pad;

    HPEN goldPen = CreatePen(PS_SOLID, 3, RGB(255, 215, 0));
    HPEN oldPen = (HPEN)SelectObject(hdc, goldPen);

    // Top-Left
    MoveToEx(hdc, left, top + arm, NULL);
    LineTo(hdc, left, top);
    LineTo(hdc, left + arm, top);

    // Top-Right
    MoveToEx(hdc, right - arm, top, NULL);
    LineTo(hdc, right, top);
    LineTo(hdc, right, top + arm);

    // Bottom-Left
    MoveToEx(hdc, left, bottom - arm, NULL);
    LineTo(hdc, left, bottom);
    LineTo(hdc, left + arm, bottom);

    // Bottom-Right
    MoveToEx(hdc, right - arm, bottom, NULL);
    LineTo(hdc, right, bottom);
    LineTo(hdc, right, bottom - arm);

    SelectObject(hdc, oldPen);
    DeleteObject(goldPen);

    // Gold Rivet Studs
    HBRUSH rivetBrush = CreateSolidBrush(RGB(255, 215, 0));
    HPEN rivetPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    SelectObject(hdc, rivetBrush);
    SelectObject(hdc, rivetPen);

    Ellipse(hdc, left + 6, top + 6, left + 14, top + 14);
    Ellipse(hdc, right - 14, top + 6, right - 6, top + 14);
    Ellipse(hdc, left + 6, bottom - 14, left + 14, bottom - 6);
    Ellipse(hdc, right - 14, bottom - 14, right - 6, bottom - 6);

    SelectObject(hdc, oldPen);
    DeleteObject(rivetBrush);
    DeleteObject(rivetPen);
}

void DrawPerimeterInlay(HDC hdc, int winW, int winH, DWORD tick) {
    int pad = 16;
    int x = pad;
    int y = pad;
    int bw = winW - pad * 2;
    int bh = winH - pad * 2;
    if (bw <= 0 || bh <= 0) return;

    float pulse = sinf((float)tick * 0.003f) * 0.25f + 0.75f;
    int rGold = (int)(255 * pulse); if (rGold > 255) rGold = 255;
    int gGold = (int)(215 * pulse); if (gGold > 215) gGold = 215;
    COLORREF inlayCol = RGB(rGold, gGold, 0);

    HPEN pen = CreatePen(PS_DOT, 1, inlayCol);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

    Rectangle(hdc, x, y, x + bw, y + bh);

    // Traveling Specular Glint
    int totalPerim = (bw + bh) * 2;
    if (totalPerim > 0) {
        int glintDist = (int)((tick / 5) % totalPerim);
        int gx = x, gy = y;
        if (glintDist < bw) {
            gx = x + glintDist; gy = y;
        } else if (glintDist < bw + bh) {
            gx = x + bw; gy = y + (glintDist - bw);
        } else if (glintDist < bw * 2 + bh) {
            gx = x + bw - (glintDist - (bw + bh)); gy = y + bh;
        } else {
            gx = x; gy = y + bh - (glintDist - (bw * 2 + bh));
        }

        HBRUSH glintBrush = CreateSolidBrush(RGB(255, 255, 255));
        HPEN glintPen = CreatePen(PS_SOLID, 2, RGB(255, 215, 0));
        SelectObject(hdc, glintBrush);
        SelectObject(hdc, glintPen);
        Ellipse(hdc, gx - 5, gy - 5, gx + 5, gy + 5);
        DeleteObject(glintBrush);
        DeleteObject(glintPen);
    }

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);
}

void DrawSheenSweeps(HDC hdc, int winW, int winH, DWORD tick) {
    float sweepPhase = fmodf((float)tick * 0.00035f, 1.0f);
    int sweepX = (int)((float)winW * (sweepPhase * 1.6f - 0.3f));

    HPEN sheenPen = CreatePen(PS_SOLID, 2, RGB(35, 115, 60));
    HPEN oldPen = (HPEN)SelectObject(hdc, sheenPen);

    MoveToEx(hdc, sweepX - 40, 20, NULL);
    LineTo(hdc, sweepX + 40, winH - 20);

    SelectObject(hdc, oldPen);
    DeleteObject(sheenPen);
}

void DrawShockwavesGDI(HDC hdc) {
    for(int i=0; i<MAX_SHOCKWAVES; i++) {
        if(shockwaves[i].active) {
            int rad = (int)shockwaves[i].radius;
            int cx = (int)shockwaves[i].x;
            int cy = (int)shockwaves[i].y;

            // Outer ring
            HPEN ringPen = CreatePen(PS_SOLID, 3, shockwaves[i].color);
            HPEN oldP = (HPEN)SelectObject(hdc, ringPen);
            HBRUSH oldB = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

            Ellipse(hdc, cx - rad, cy - rad, cx + rad, cy + rad);

            // Inner compression wave
            int innerRad = (int)(rad * 0.65f);
            if(innerRad > 2) {
                HPEN innerPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                SelectObject(hdc, innerPen);
                Ellipse(hdc, cx - innerRad, cy - innerRad, cx + innerRad, cy + innerRad);
                DeleteObject(innerPen);
            }

            SelectObject(hdc, oldP);
            SelectObject(hdc, oldB);
            DeleteObject(ringPen);
        }
    }
}

void SpawnThawSparks(int cx, int cy) {
    TriggerScreenShake(6);
    SpawnShockwave((float)cx, (float)cy, RGB(128, 222, 234), 130.0f, 6.0f);
    COLORREF colors[] = {RGB(224,247,250), RGB(128,222,234), RGB(38,198,218), RGB(255,255,255)};
    for(int i=0; i<28; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 1.5f + (rand() % 55) * 0.1f;
        SpawnParticleEx(PARTICLE_SPARK, (float)cx, (float)cy, cosf(angle)*speed, sinf(angle)*speed, 1.0f, 0.028f, colors[rand()%4], 2 + rand()%3, 0.0f);
    }
    for(int i=0; i<10; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 0.3f + (rand() % 20) * 0.04f;
        SpawnParticleEx(PARTICLE_SMOKE, (float)cx, (float)cy, cosf(angle)*speed, sinf(angle)*speed, 0.9f, 0.02f, RGB(200, 235, 255), 7 + rand()%6, 0.0f);
    }
    for(int i=0; i<16; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 1.0f + (rand() % 45) * 0.1f;
        float spinSpd = (float)((rand() % 40) - 20) * 0.04f;
        SpawnParticleEx(PARTICLE_DEBRIS, (float)cx, (float)cy, cosf(angle)*speed, sinf(angle)*speed - 1.2f, 1.0f, 0.022f, RGB(180, 230, 250), 4 + rand()%4, spinSpd);
    }
    for(int i=0; i<6; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 1.0f + (rand() % 35) * 0.1f;
        float spinSpd = (float)((rand() % 30) - 15) * 0.02f;
        SpawnParticleEx(PARTICLE_STAR, (float)cx, (float)cy, cosf(angle)*speed, sinf(angle)*speed, 1.0f, 0.025f, RGB(128, 222, 234), 8 + rand()%6, spinSpd);
    }
}

void SpawnWandSparks(int cx, int cy) {
    TriggerScreenShake(5);
    SpawnShockwave((float)cx, (float)cy, RGB(255, 215, 0), 140.0f, 6.5f);
    COLORREF colors[] = {RGB(255,215,0), RGB(255,235,59), RGB(255,193,7), RGB(255,255,255)};
    for(int i=0; i<32; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 2.0f + (rand() % 55) * 0.1f;
        SpawnParticleEx(PARTICLE_SPARK, (float)cx, (float)cy, cosf(angle)*speed, sinf(angle)*speed, 1.0f, 0.02f, colors[rand()%4], 3 + rand()%3, 0.0f);
    }
    for(int i=0; i<8; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 0.5f + (rand() % 20) * 0.03f;
        SpawnParticleEx(PARTICLE_SMOKE, (float)cx, (float)cy, cosf(angle)*speed, sinf(angle)*speed, 0.85f, 0.022f, RGB(255, 235, 150), 6 + rand()%6, 0.0f);
    }
    for(int i=0; i<14; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 1.2f + (rand() % 40) * 0.1f;
        float spinSpd = (float)((rand() % 50) - 25) * 0.04f;
        SpawnParticleEx(PARTICLE_DEBRIS, (float)cx, (float)cy, cosf(angle)*speed, sinf(angle)*speed, 1.0f, 0.02f, RGB(255, 220, 50), 4 + rand()%4, spinSpd);
    }
    for(int i=0; i<8; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 1.2f + (rand() % 40) * 0.1f;
        float spinSpd = (float)((rand() % 30) - 15) * 0.02f;
        SpawnParticleEx(PARTICLE_STAR, (float)cx, (float)cy, cosf(angle)*speed, sinf(angle)*speed, 1.1f, 0.02f, RGB(255, 215, 0), 9 + rand()%6, spinSpd);
    }
}

void SpawnVictoryFireworksBurst(int w, int h) {
    int cx = (w / 5) + rand() % (w * 3 / 5);
    int cy = (h / 5) + rand() % (h * 2 / 5);
    COLORREF color = RGB(rand()%255, rand()%255, rand()%255);
    TriggerScreenShake(8);
    SpawnShockwave((float)cx, (float)cy, RGB(255, 215, 0), 180.0f, 7.5f);
    for(int i=0; i<75; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 2.5f + (rand() % 95) * 0.1f;
        SpawnParticleEx(PARTICLE_SPARK, (float)cx, (float)cy, cosf(angle)*speed, sinf(angle)*speed, 1.0f + (rand()%40)*0.01f, 0.015f, color, 3 + rand()%3, 0.0f);
        SpawnParticleEx(PARTICLE_SPARK, (float)cx, (float)cy, cosf(angle)*speed*0.6f, sinf(angle)*speed*0.6f, 0.85f, 0.02f, RGB(255,255,255), 1 + rand()%2, 0.0f);
    }
    for(int i=0; i<18; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 0.4f + (rand() % 25) * 0.05f;
        SpawnParticleEx(PARTICLE_SMOKE, (float)cx, (float)cy, cosf(angle)*speed, sinf(angle)*speed, 1.0f, 0.016f, RGB(180, 180, 190), 8 + rand()%8, 0.0f);
    }
    COLORREF confettiColors[] = {RGB(255,215,0), RGB(255,69,0), RGB(0,191,255), RGB(50,205,50), RGB(255,105,180), RGB(255,255,255)};
    for(int i=0; i<30; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 1.0f + (rand() % 65) * 0.1f;
        float spinSpd = (float)((rand() % 50) - 25) * 0.03f;
        SpawnParticleEx(PARTICLE_DEBRIS, (float)cx, (float)cy, cosf(angle)*speed, sinf(angle)*speed - 2.2f, 1.25f, 0.014f, confettiColors[rand()%6], 5 + rand()%4, spinSpd);
    }
    for(int i=0; i<15; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 1.5f + (rand() % 50) * 0.1f;
        float spinSpd = (float)((rand() % 30) - 15) * 0.03f;
        SpawnParticleEx(PARTICLE_STAR, (float)cx, (float)cy, cosf(angle)*speed, sinf(angle)*speed, 1.2f, 0.016f, RGB(255, 215, 0), 10 + rand()%8, spinSpd);
    }
}

void SpawnSnapSparks(int cx, int cy) {
    TriggerScreenShake(4);
    SpawnShockwave((float)cx, (float)cy, RGB(255, 215, 0), 120.0f, 5.5f);
    COLORREF sparkColors[] = {RGB(255,255,255), RGB(255,225,50), RGB(100,200,255)};
    for(int i=0; i<22; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 2.5f + (rand() % 45) * 0.1f;
        SpawnParticleEx(PARTICLE_SPARK, (float)cx, (float)cy, cosf(angle)*speed, sinf(angle)*speed, 1.0f, 0.035f, sparkColors[rand()%3], 2 + rand()%3, 0.0f);
    }
    for(int i=0; i<8; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 0.5f + (rand() % 20) * 0.05f;
        SpawnParticleEx(PARTICLE_SMOKE, (float)cx, (float)cy, cosf(angle)*speed, sinf(angle)*speed, 0.85f, 0.022f, RGB(200, 210, 190), 6 + rand()%6, 0.0f);
    }
    COLORREF debrisColors[] = {RGB(255,215,0), RGB(220,20,60), RGB(245,245,245), RGB(40,40,40)};
    for(int i=0; i<14; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 1.5f + (rand() % 40) * 0.1f;
        float spinSpd = (float)((rand() % 40) - 20) * 0.03f;
        SpawnParticleEx(PARTICLE_DEBRIS, (float)cx, (float)cy, cosf(angle)*speed, sinf(angle)*speed - 1.8f, 1.0f, 0.022f, debrisColors[rand()%4], 4 + rand()%4, spinSpd);
    }
    for(int i=0; i<6; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 1.0f + (rand() % 30) * 0.1f;
        float spinSpd = (float)((rand() % 30) - 15) * 0.02f;
        SpawnParticleEx(PARTICLE_STAR, (float)cx, (float)cy, cosf(angle)*speed, sinf(angle)*speed, 1.0f, 0.025f, RGB(255, 215, 0), 8 + rand()%6, spinSpd);
    }
}

DWORD cascadeFrame = 0;

void StartAnim(Card c, int sx, int sy, int ex, int ey) {
    for(int i=0; i<MAX_ANIMS; i++) {
        if(!anims[i].active) {
            anims[i].active = 1;
            anims[i].c = c;
            anims[i].startX = sx;
            anims[i].startY = sy;
            anims[i].endX = ex;
            anims[i].endY = ey;
            anims[i].startTime = GetTickCount();
            anims[i].duration = 150;
            break;
        }
    }
}

void ClearAnims() {
    for(int i=0; i<MAX_ANIMS; i++) anims[i].active = 0;
    cascadeActive = 0;
}

int IsAnimating(Card c, int *px, int *py) {
    for(int i=0; i<MAX_ANIMS; i++) {
        if(anims[i].active && anims[i].c.s == c.s && anims[i].c.r == c.r) {
            DWORD now = GetTickCount();
            if (now >= anims[i].startTime + anims[i].duration) {
                anims[i].active = 0;
                return 0;
            }
            float t = (float)(now - anims[i].startTime) / anims[i].duration;
            *px = anims[i].startX + (int)(t * (anims[i].endX - anims[i].startX));
            *py = anims[i].startY + (int)(t * (anims[i].endY - anims[i].startY));
            return 1;
        }
    }
    return 0;
}

int GetCardX(int type, int idx, int cIdx, RECT clientRect) {
    if (type == 0) return PAD + idx * (CELL_W + TAB_PAD_X);
    if (type == 1) {
        int totalTabW = 8 * CELL_W + 7 * TAB_PAD_X;
        return (clientRect.right - totalTabW) / 2 + idx * (CELL_W + TAB_PAD_X);
    }
    if (type == 2) return clientRect.right - PAD - 4*(CELL_W + TAB_PAD_X) + idx * (CELL_W + TAB_PAD_X);
    return 0;
}

int GetCardY(int type, int idx, int cIdx, RECT clientRect) {
    if (type == 0 || type == 2) return TOP_Y;
    if (type == 1) return TAB_Y + cIdx * TAB_PAD_Y;
    return 0;
}

typedef struct {
    Card freeCells[MAX_FREE_CELLS];
    int freeCellsOccupied[MAX_FREE_CELLS];
    int numFreeCells;
    int found[4];
    Card tab[8][52];
    int tabCount[8];
} GameState;

#define MAX_UNDO 500
GameState undoStack[MAX_UNDO];
int undoMovesStack[MAX_UNDO];
int undoCount = 0;

void PushUndo() {
    if (undoCount == MAX_UNDO) {
        for(int i=1; i<MAX_UNDO; i++) {
            undoStack[i-1] = undoStack[i];
            undoMovesStack[i-1] = undoMovesStack[i];
        }
        undoCount--;
    }
    for(int i=0; i<MAX_FREE_CELLS; i++) {
        undoStack[undoCount].freeCells[i] = freeCells[i];
        undoStack[undoCount].freeCellsOccupied[i] = freeCellsOccupied[i];
    }
    undoStack[undoCount].numFreeCells = numFreeCells;
    for(int i=0; i<4; i++) {
        undoStack[undoCount].found[i] = found[i];
    }
    for(int i=0; i<8; i++) {
        undoStack[undoCount].tabCount[i] = tabCount[i];
        for(int j=0; j<52; j++) {
            undoStack[undoCount].tab[i][j] = tab[i][j];
        }
    }
    undoMovesStack[undoCount] = moves;
    undoCount++;
}

DWORD WINAPI SoundThread(LPVOID lpParam) {
    int type = (int)(intptr_t)lpParam;
    if (type == 0) {
        Beep(600, 30);
    } else if (type == 1) {
        Beep(400, 20);
        Beep(200, 20);
    } else if (type == 2) {
        Beep(440, 200);
        Beep(554, 200);
        Beep(659, 200);
        Beep(880, 400);
    } else if (type == 3) {
        Beep(800, 100);
        Beep(1200, 150);
    } else if (type == 4) {
        Beep(300, 80);
    }
    return 0;
}

void PlaySoundEffect(int type) {
    CreateThread(NULL, 0, SoundThread, (LPVOID)(intptr_t)type, 0, NULL);
}

void ThawCheck() {
    for (int i = 0; i < 8; i++) {
        if (tabCount[i] > 0) {
            if (tab[i][tabCount[i] - 1].frozen) {
                tab[i][tabCount[i] - 1].frozen = 0;
                RECT clientRect; GetClientRect(GetActiveWindow(), &clientRect);
                SpawnThawSparks(GetCardX(1, i, tabCount[i]-1, clientRect) + CELL_W/2, GetCardY(1, i, tabCount[i]-1, clientRect) + CELL_H/2);
            }
        }
    }
}

void ThawAdjacent(int col) {
    for (int c = col - 1; c <= col + 1; c++) {
        if (c >= 0 && c < 8 && tabCount[c] > 0) {
            if (tab[c][tabCount[c] - 1].frozen) {
                tab[c][tabCount[c] - 1].frozen = 0;
                RECT clientRect; GetClientRect(GetActiveWindow(), &clientRect);
                SpawnThawSparks(GetCardX(1, c, tabCount[c]-1, clientRect) + CELL_W/2, GetCardY(1, c, tabCount[c]-1, clientRect) + CELL_H/2);
            }
        }
    }
}

void StartVictoryCascade(HWND hwnd) {
    RECT clientRect; GetClientRect(hwnd, &clientRect);
    cascadeActive = 1;
    cascadeFrame = 0;
    int idx = 0;
    for(int r=13; r>=1; r--) {
        for(int s=0; s<4; s++) {
            cascadeCards[idx].c.s = s;
            cascadeCards[idx].c.r = r;
            cascadeCards[idx].c.color = (s==1||s==3)?1:0;
            cascadeCards[idx].c.frozen = 0;
            cascadeCards[idx].x = (float)GetCardX(2, s, 0, clientRect);
            cascadeCards[idx].y = (float)GetCardY(2, s, 0, clientRect);
            cascadeCards[idx].vx = (float)((rand() % 16) - 8);
            cascadeCards[idx].vy = (float)(-3 - (rand() % 4));
            cascadeCards[idx].active = 0;
            cascadeCards[idx].delay = (13 - r) * 12 + s * 3;
            idx++;
        }
    }
}

void ShowStats(HWND hwnd) {
    WCHAR msg[512];
    int pct = statsPlayed > 0 ? (statsWins * 100) / statsPlayed : 0;
    wsprintfW(msg, L"Games Played: %d\nWins: %d (%d%%)\nCurrent Streak: %d\nBest Streak: %d\nBest Time: %d sec\nCampaign Progress: Stage %d / 20\nTime Attack Wins: %d\nBest Time Attack Win: %d sec",
              statsPlayed, statsWins, pct, statsStreak, statsBestStreak, statsBestTime, maxCampaignStage, statsTimeAttackWins, statsBestTimeAttackTime);
    MessageBoxW(hwnd, msg, L"Statistics", MB_OK | MB_ICONINFORMATION);
}

void ShowHelp(HWND hwnd) {
    WCHAR msg[2048] = 
        L"How to Play Freecell (Loop 7 Expansion)\n\n"
        L"Rules:\n"
        L"- Build all 4 foundations from Ace to King by suit.\n"
        L"- Move cards between tableau columns. Cards must be placed in descending order (Alt colors or Baker's Suit).\n"
        L"- Free cells store single cards. Extra cells can be unlocked temporarily.\n"
        L"- Moving multiple cards requires enough empty free cells/columns.\n"
        L"- Some campaign stages restrict empty columns to Kings only or contain Frozen cards.\n"
        L"- Frozen cards thaw when exposed at column top or when adjacent plays occur.\n\n"
        L"Active Skills & Controls:\n"
        L"- [W] Magic Wand: Auto-detects and plays optimal safe cards.\n"
        L"- [E] Extra Freecell: Unlocks +1 temporary Free Cell slot for 30 seconds!\n"
        L"- [A] Auto-Solve: Continuously sweeps safe cards to foundations.\n"
        L"- [U] / [Z] Free Undo: Unlimited move undo.\n"
        L"- [P] Shuffle: Shuffles remaining tableau cards in place.\n"
        L"- Key bindings: [N]ew Game, [S]tats, [C]hange Card Back, [M]ode Toggle, [+]Next Seed, [-]Prev Seed, [F5]Save, [F9]Load, [H]elp.\n\n"
        L"Modes:\n"
        L"- Random Deal / Numbered Deal.\n"
        L"- Campaign: 20 stages featuring 4-cell, 3-cell, 2-cell constraints, Baker's rules, King-only spaces, Frozen cards, and Stage 20 Grandmaster Challenge.\n"
        L"- Time Attack: 180s countdown timer! Foundations give +15s bonus time.";
    MessageBoxW(hwnd, msg, L"Help", MB_OK | MB_ICONINFORMATION);
}

void UndoMove(HWND hwnd) {
    if(undoCount > 0) {
        undoCount--;
        for(int i=0; i<MAX_FREE_CELLS; i++) {
            freeCells[i] = undoStack[undoCount].freeCells[i];
            freeCellsOccupied[i] = undoStack[undoCount].freeCellsOccupied[i];
        }
        numFreeCells = undoStack[undoCount].numFreeCells;
        for(int i=0; i<4; i++) {
            found[i] = undoStack[undoCount].found[i];
        }
        for(int i=0; i<8; i++) {
            tabCount[i] = undoStack[undoCount].tabCount[i];
            for(int j=0; j<52; j++) {
                tab[i][j] = undoStack[undoCount].tab[i][j];
            }
        }
        moves = undoMovesStack[undoCount];
        selType = -1;
        won = 0;
        ClearAnims();
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void SaveGame(HWND hwnd) {
    FILE *f = _wfopen(L"kfreecell_save.dat", L"wb");
    if(f) {
        fwrite(&freeCells, sizeof(freeCells), 1, f);
        fwrite(&freeCellsOccupied, sizeof(freeCellsOccupied), 1, f);
        fwrite(&numFreeCells, sizeof(numFreeCells), 1, f);
        fwrite(&found, sizeof(found), 1, f);
        fwrite(&tab, sizeof(tab), 1, f);
        fwrite(&tabCount, sizeof(tabCount), 1, f);
        fwrite(&gameInProgress, sizeof(gameInProgress), 1, f);
        fwrite(&statsPlayed, sizeof(statsPlayed), 1, f);
        fwrite(&statsWins, sizeof(statsWins), 1, f);
        fwrite(&statsStreak, sizeof(statsStreak), 1, f);
        fwrite(&statsBestStreak, sizeof(statsBestStreak), 1, f);
        fwrite(&statsBestTime, sizeof(statsBestTime), 1, f);
        fwrite(&gameMode, sizeof(gameMode), 1, f);
        fwrite(&currentSeed, sizeof(currentSeed), 1, f);
        fwrite(&undoCount, sizeof(undoCount), 1, f);
        if(undoCount > 0) fwrite(undoStack, sizeof(GameState), undoCount, f);
        fwrite(&moves, sizeof(moves), 1, f);
        fwrite(&timeElapsed, sizeof(timeElapsed), 1, f);
        if(undoCount > 0) fwrite(undoMovesStack, sizeof(int), undoCount, f);
        fwrite(&campaignStage, sizeof(campaignStage), 1, f);
        fwrite(&maxCampaignStage, sizeof(maxCampaignStage), 1, f);
        fwrite(&powerupsShuffle, sizeof(powerupsShuffle), 1, f);
        fwrite(&powerupsWand, sizeof(powerupsWand), 1, f);
        fwrite(&powerupsExtraCell, sizeof(powerupsExtraCell), 1, f);
        fwrite(&timeRemaining, sizeof(timeRemaining), 1, f);
        fwrite(&statsTimeAttackWins, sizeof(statsTimeAttackWins), 1, f);
        fwrite(&statsBestTimeAttackTime, sizeof(statsBestTimeAttackTime), 1, f);
        fclose(f);
        MessageBoxW(hwnd, L"Game Saved", L"Save", MB_OK | MB_ICONINFORMATION);
    }
}

void LoadGame(HWND hwnd) {
    FILE *f = _wfopen(L"kfreecell_save.dat", L"rb");
    if(f) {
        fread(&freeCells, sizeof(freeCells), 1, f);
        fread(&freeCellsOccupied, sizeof(freeCellsOccupied), 1, f);
        if(fread(&numFreeCells, sizeof(numFreeCells), 1, f) != 1) numFreeCells = 4;
        fread(&found, sizeof(found), 1, f);
        fread(&tab, sizeof(tab), 1, f);
        fread(&tabCount, sizeof(tabCount), 1, f);
        fread(&gameInProgress, sizeof(gameInProgress), 1, f);
        fread(&statsPlayed, sizeof(statsPlayed), 1, f);
        fread(&statsWins, sizeof(statsWins), 1, f);
        fread(&statsStreak, sizeof(statsStreak), 1, f);
        fread(&statsBestStreak, sizeof(statsBestStreak), 1, f);
        if(fread(&statsBestTime, sizeof(statsBestTime), 1, f) != 1) statsBestTime = 0;
        if(fread(&gameMode, sizeof(gameMode), 1, f) != 1) gameMode = 0;
        if(fread(&currentSeed, sizeof(currentSeed), 1, f) != 1) currentSeed = 1;
        fread(&undoCount, sizeof(undoCount), 1, f);
        if(undoCount > 0) fread(undoStack, sizeof(GameState), undoCount, f);
        if(fread(&moves, sizeof(moves), 1, f) != 1) moves = 0;
        if(fread(&timeElapsed, sizeof(timeElapsed), 1, f) != 1) timeElapsed = 0;
        if(undoCount > 0) {
            if(fread(undoMovesStack, sizeof(int), undoCount, f) != undoCount) {
                for(int i=0; i<undoCount; i++) undoMovesStack[i] = 0;
            }
        }
        if(fread(&campaignStage, sizeof(campaignStage), 1, f) != 1) campaignStage = 1;
        if(fread(&maxCampaignStage, sizeof(maxCampaignStage), 1, f) != 1) maxCampaignStage = 1;
        if(fread(&powerupsShuffle, sizeof(powerupsShuffle), 1, f) != 1) powerupsShuffle = 1;
        if(fread(&powerupsWand, sizeof(powerupsWand), 1, f) != 1) powerupsWand = 1;
        if(fread(&powerupsExtraCell, sizeof(powerupsExtraCell), 1, f) != 1) powerupsExtraCell = 1;
        if(fread(&timeRemaining, sizeof(timeRemaining), 1, f) != 1) timeRemaining = 180;
        if(fread(&statsTimeAttackWins, sizeof(statsTimeAttackWins), 1, f) != 1) statsTimeAttackWins = 0;
        if(fread(&statsBestTimeAttackTime, sizeof(statsBestTimeAttackTime), 1, f) != 1) statsBestTimeAttackTime = 0;
        fclose(f);
        lastTimeTick = GetTickCount();
        selType = -1;
        won = 0;
        ClearAnims();
        if (found[0]==13 && found[1]==13 && found[2]==13 && found[3]==13) won = 1;
        InvalidateRect(hwnd, NULL, TRUE);
        MessageBoxW(hwnd, L"Game Loaded", L"Load", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxW(hwnd, L"No Save Found", L"Load", MB_OK | MB_ICONWARNING);
    }
}

typedef struct {
    uint8_t found[4];
    uint8_t free[MAX_FREE_CELLS];
    uint8_t tabCount[8];
    uint8_t tab[8][20];
} SolverState;

#define SET_SIZE 8192
uint32_t visited[SET_SIZE];
int statesExplored = 0;

void ClearVisited() {
    for(int i=0; i<SET_SIZE; i++) visited[i] = 0;
}
void AddVisited(uint32_t h) {
    int idx = h % SET_SIZE;
    while(visited[idx] != 0 && visited[idx] != h) {
        idx = (idx + 1) % SET_SIZE;
    }
    visited[idx] = h;
}
int HasVisited(uint32_t h) {
    int idx = h % SET_SIZE;
    while(visited[idx] != 0) {
        if (visited[idx] == h) return 1;
        idx = (idx + 1) % SET_SIZE;
    }
    return 0;
}

uint32_t HashState(SolverState *s, int numFree) {
    uint32_t h = 0;
    for(int i=0; i<4; i++) h = (h * 31) + s->found[i];
    for(int i=0; i<numFree; i++) h = (h * 31) + s->free[i];
    for(int i=0; i<8; i++) {
        h = (h * 31) + s->tabCount[i];
        if (s->tabCount[i] > 0) h = (h * 31) + s->tab[i][s->tabCount[i]-1];
    }
    return h == 0 ? 1 : h;
}

typedef struct { int type, from, to; } SMove;

int IsSafe_Solver(uint8_t val, uint8_t *found) {
    int r = val & 15;
    if (r <= 2) return 1;
    int s = val >> 4;
    int col = (s==1||s==3)?1:0;
    int minO = 14;
    for(int i=0; i<4; i++) {
        if (((i==1||i==3)?1:0) != col) {
            if (found[i] < minO) minO = found[i];
        }
    }
    return r <= minO + 1;
}

int GetMoves(SolverState *s, SMove *moves, int numFree, int buildRule, int emptyKing) {
    int count = 0;
    for(int i=0; i<8; i++) {
        if (s->tabCount[i] > 0) {
            uint8_t val = s->tab[i][s->tabCount[i]-1];
            if ((val & 15) == s->found[val >> 4] + 1) {
                moves[count].type = 0; moves[count].from = i; moves[count].to = 0;
                if (IsSafe_Solver(val, s->found)) return 1;
                count++;
            }
        }
    }
    if (count == 1 && moves[0].type == 0) {} 
    else {
        for(int i=0; i<numFree; i++) {
            if (s->free[i] != 0) {
                uint8_t val = s->free[i];
                if ((val & 15) == s->found[val >> 4] + 1) {
                    moves[count].type = 1; moves[count].from = i; moves[count].to = 0;
                    if (IsSafe_Solver(val, s->found)) return 1;
                    count++;
                }
            }
        }
    }
    
    if (count > 0 && IsSafe_Solver(moves[count-1].type == 0 ? s->tab[moves[count-1].from][s->tabCount[moves[count-1].from]-1] : s->free[moves[count-1].from], s->found)) {
        // Safe move found
    } else {
        for(int i=0; i<8; i++) {
            if (s->tabCount[i] > 0) {
                uint8_t val = s->tab[i][s->tabCount[i]-1];
                int r = val & 15, su = val >> 4, col = (su==1||su==3)?1:0;
                for(int j=0; j<8; j++) {
                    if (i != j) {
                        if (s->tabCount[j] == 0) {
                            if ((!emptyKing || r == 13) && s->tabCount[i] > 1) moves[count++] = (SMove){2, i, j};
                        } else {
                            uint8_t tval = s->tab[j][s->tabCount[j]-1];
                            int tr = tval & 15, tsu = tval >> 4, tcol = (tsu==1||tsu==3)?1:0;
                            if (buildRule == 1) {
                                if (tsu == su && r == tr - 1) moves[count++] = (SMove){2, i, j};
                            } else {
                                if (tcol != col && r == tr - 1) moves[count++] = (SMove){2, i, j};
                            }
                        }
                    }
                }
            }
        }
        for(int i=0; i<numFree; i++) {
            if (s->free[i] != 0) {
                uint8_t val = s->free[i];
                int r = val & 15, su = val >> 4, col = (su==1||su==3)?1:0;
                for(int j=0; j<8; j++) {
                    if (s->tabCount[j] == 0) {
                        if (!emptyKing || r == 13) moves[count++] = (SMove){3, i, j};
                    } else {
                        uint8_t tval = s->tab[j][s->tabCount[j]-1];
                        int tr = tval & 15, tsu = tval >> 4, tcol = (tsu==1||tsu==3)?1:0;
                        if (buildRule == 1) {
                            if (tsu == su && r == tr - 1) moves[count++] = (SMove){3, i, j};
                        } else {
                            if (tcol != col && r == tr - 1) moves[count++] = (SMove){3, i, j};
                        }
                    }
                }
            }
        }
        for(int i=0; i<8; i++) {
            if (s->tabCount[i] > 1) {
                for(int j=0; j<numFree; j++) {
                    if (s->free[j] == 0) { moves[count++] = (SMove){4, i, j}; break; }
                }
            }
        }
    }
    return count;
}

void ApplyMove(SolverState *s, SMove *m) {
    if (m->type == 0) { uint8_t val = s->tab[m->from][--s->tabCount[m->from]]; s->found[val >> 4]++; }
    else if (m->type == 1) { uint8_t val = s->free[m->from]; s->free[m->from] = 0; s->found[val >> 4]++; }
    else if (m->type == 2) { uint8_t val = s->tab[m->from][--s->tabCount[m->from]]; s->tab[m->to][s->tabCount[m->to]++] = val; }
    else if (m->type == 3) { uint8_t val = s->free[m->from]; s->free[m->from] = 0; s->tab[m->to][s->tabCount[m->to]++] = val; }
    else if (m->type == 4) { uint8_t val = s->tab[m->from][--s->tabCount[m->from]]; s->free[m->to] = val; }
}

int DFS_Solve(SolverState *s, int numFree, int buildRule, int emptyKing, int maxStates) {
    statesExplored++;
    if (statesExplored > maxStates) return 0;
    if (s->found[0]==13 && s->found[1]==13 && s->found[2]==13 && s->found[3]==13) return 1;
    
    uint32_t h = HashState(s, numFree);
    if (HasVisited(h)) return 0;
    AddVisited(h);
    
    SMove moves[100];
    int mCount = GetMoves(s, moves, numFree, buildRule, emptyKing);
    
    for(int i=0; i<mCount-1; i++) {
        for(int j=i+1; j<mCount; j++) {
            int scoreI = (moves[i].type <= 1) ? 100 : ((moves[i].type <= 3) ? 10 : 0);
            int scoreJ = (moves[j].type <= 1) ? 100 : ((moves[j].type <= 3) ? 10 : 0);
            if (scoreJ > scoreI) { SMove tmp = moves[i]; moves[i] = moves[j]; moves[j] = tmp; }
        }
    }
    
    for(int i=0; i<mCount; i++) {
        SolverState ns = *s;
        ApplyMove(&ns, &moves[i]);
        if (DFS_Solve(&ns, numFree, buildRule, emptyKing, maxStates)) return 1;
    }
    return 0;
}

void InitGame() {
    if (gameInProgress && won == 0) {
        statsStreak = 0;
    }
    statsPlayed++;
    gameInProgress = 1;
    moves = 0;
    timeElapsed = 0;
    lastTimeTick = GetTickCount();
    extraCellActive = 0;
    extraCellTimer = 0;

    won = 0;
    selType = -1;
    undoCount = 0;
    ClearAnims();
    
    emptyKingOnly = 0;
    int frozenCount = 0;
    if (gameMode == 2) {
        if (campaignStage == 1) { numFreeCells = 4; buildRule = 0; emptyKingOnly = 0; frozenCount = 0; }
        else if (campaignStage == 2) { numFreeCells = 4; buildRule = 0; emptyKingOnly = 0; frozenCount = 2; }
        else if (campaignStage == 3) { numFreeCells = 4; buildRule = 0; emptyKingOnly = 1; frozenCount = 0; }
        else if (campaignStage == 4) { numFreeCells = 4; buildRule = 1; emptyKingOnly = 0; frozenCount = 0; }
        else if (campaignStage == 5) { numFreeCells = 4; buildRule = 0; emptyKingOnly = 0; frozenCount = 4; }
        else if (campaignStage == 6) { numFreeCells = 3; buildRule = 0; emptyKingOnly = 0; frozenCount = 0; }
        else if (campaignStage == 7) { numFreeCells = 3; buildRule = 0; emptyKingOnly = 0; frozenCount = 2; }
        else if (campaignStage == 8) { numFreeCells = 3; buildRule = 0; emptyKingOnly = 1; frozenCount = 0; }
        else if (campaignStage == 9) { numFreeCells = 3; buildRule = 1; emptyKingOnly = 0; frozenCount = 0; }
        else if (campaignStage == 10) { numFreeCells = 3; buildRule = 0; emptyKingOnly = 0; frozenCount = 4; }
        else if (campaignStage == 11) { numFreeCells = 4; buildRule = 1; emptyKingOnly = 1; frozenCount = 0; }
        else if (campaignStage == 12) { numFreeCells = 4; buildRule = 1; emptyKingOnly = 0; frozenCount = 4; }
        else if (campaignStage == 13) { numFreeCells = 4; buildRule = 0; emptyKingOnly = 1; frozenCount = 4; }
        else if (campaignStage == 14) { numFreeCells = 4; buildRule = 1; emptyKingOnly = 1; frozenCount = 4; }
        else if (campaignStage == 15) { numFreeCells = 3; buildRule = 1; emptyKingOnly = 1; frozenCount = 0; }
        else if (campaignStage == 16) { numFreeCells = 3; buildRule = 1; emptyKingOnly = 0; frozenCount = 4; }
        else if (campaignStage == 17) { numFreeCells = 3; buildRule = 0; emptyKingOnly = 1; frozenCount = 6; }
        else if (campaignStage == 18) { numFreeCells = 2; buildRule = 0; emptyKingOnly = 0; frozenCount = 0; }
        else if (campaignStage == 19) { numFreeCells = 2; buildRule = 0; emptyKingOnly = 0; frozenCount = 4; }
        else if (campaignStage >= 20) { numFreeCells = 2; buildRule = 0; emptyKingOnly = 1; frozenCount = 0; }
    } else {
        numFreeCells = 4;
        buildRule = 0;
        emptyKingOnly = 0;
    }
    powerupsShuffle = 1;
    powerupsWand = 1;
    powerupsExtraCell = 1;
    if (gameMode == 3) {
        timeRemaining = 180;
    }

    for(int i=0; i<MAX_FREE_CELLS; i++) {
        freeCellsOccupied[i] = 0;
    }
    for(int i=0; i<4; i++) {
        found[i] = 0;
    }
    for(int i=0; i<8; i++) {
        tabCount[i] = 0;
    }
    
    Card* deckPtrs[52];
    int c = 0;
    for(int s=0; s<4; s++) {
        for(int r=1; r<=13; r++) {
            deck[c].s = s;
            deck[c].r = r;
            deck[c].color = (s==1 || s==3) ? 1 : 0;
            deck[c].frozen = 0;
            deckPtrs[c] = &deck[c];
            c++;
        }
    }
    
    if (gameMode == 1) {
        int stateRng = currentSeed;
        for(int i=51; i>0; i--) {
            stateRng = (stateRng * 214013 + 2531011) & 0x7FFFFFFF;
            int j = ((stateRng >> 16) & 0x7FFF) % (i + 1);
            Card *t = deckPtrs[i];
            deckPtrs[i] = deckPtrs[j];
            deckPtrs[j] = t;
        }
    } else {
        for(int i=51; i>0; i--) {
            int j = rand() % (i + 1);
            Card *t = deckPtrs[i];
            deckPtrs[i] = deckPtrs[j];
            deckPtrs[j] = t;
        }
    }
    
    c = 0;
    for(int i=0; i<52; i++) {
        tab[c][tabCount[c]++] = *deckPtrs[i];
        c = (c + 1) % 8;
    }

    if (gameMode != 1) {
        int shuffles = 0;
        while (1) {
            SolverState ss;
            memset(&ss, 0, sizeof(ss));
            for(int i=0; i<8; i++) {
                ss.tabCount[i] = tabCount[i];
                for(int j=0; j<tabCount[i]; j++) {
                    ss.tab[i][j] = (tab[i][j].s << 4) | tab[i][j].r;
                }
            }
            ClearVisited();
            statesExplored = 0;
            if (DFS_Solve(&ss, numFreeCells, buildRule, emptyKingOnly, 2000)) break;
            
            shuffles++;
            if (shuffles > 50) break; // Fallback
            
            for(int i=51; i>0; i--) {
                int j = rand() % (i + 1);
                Card *t = deckPtrs[i];
                deckPtrs[i] = deckPtrs[j];
                deckPtrs[j] = t;
            }
            c = 0;
            for(int i=0; i<8; i++) tabCount[i] = 0;
            for(int i=0; i<52; i++) {
                tab[c][tabCount[c]++] = *deckPtrs[i];
                c = (c + 1) % 8;
            }
        }
    }

    if (frozenCount > 0) {
        int count = 0;
        for (int j = 4; j >= 1 && count < frozenCount; j--) {
            for (int i = 0; i < 8 && count < frozenCount; i++) {
                if (j < tabCount[i]) {
                    tab[i][j].frozen = 1;
                    count++;
                }
            }
        }
    }
    ThawCheck();
}

int GetMaxMoveCount() {
    int emptyFree = 0;
    for(int i=0; i<numFreeCells; i++) if(!freeCellsOccupied[i]) emptyFree++;
    int emptyTab = 0;
    for(int i=0; i<8; i++) if(tabCount[i] == 0) emptyTab++;
    return (emptyFree + 1) * (1 << emptyTab);
}

int CanMoveToTab(Card c, int tIdx) {
    if (c.frozen) return 0;
    if(tabCount[tIdx] == 0) {
        if (emptyKingOnly) return c.r == 13;
        return 1;
    }
    Card top = tab[tIdx][tabCount[tIdx]-1];
    if (buildRule == 1) {
        return c.s == top.s && c.r == top.r - 1;
    }
    return c.color != top.color && c.r == top.r - 1;
}

int GetDraggableGroup(int tIdx, int startIdx) {
    if(startIdx >= tabCount[tIdx]) return 0;
    if(tab[tIdx][startIdx].frozen) return 0;
    for(int i=startIdx+1; i<tabCount[tIdx]; i++) {
        Card prev = tab[tIdx][i-1];
        Card curr = tab[tIdx][i];
        if (curr.frozen) return 0;
        if (buildRule == 1) {
            if(curr.s != prev.s || curr.r != prev.r - 1) return 0;
        } else {
            if(curr.color == prev.color || curr.r != prev.r - 1) return 0;
        }
    }
    return tabCount[tIdx] - startIdx;
}

void CheckWin(HWND hwnd) {
    if(found[0]==13 && found[1]==13 && found[2]==13 && found[3]==13) {
        if(gameInProgress && won == 0) {
            PlaySoundEffect(2);
            TriggerScreenShake(20);
            won = 1;
            statsWins++;
            statsStreak++;
            if (statsStreak > statsBestStreak) statsBestStreak = statsStreak;
            if (statsBestTime == 0 || timeElapsed < statsBestTime) statsBestTime = timeElapsed;
            if (gameMode == 2 && campaignStage < 20) {
                campaignStage++;
                if (campaignStage > maxCampaignStage) maxCampaignStage = campaignStage;
            }
            if (gameMode == 3) {
                statsTimeAttackWins++;
                if (statsBestTimeAttackTime == 0 || timeElapsed < statsBestTimeAttackTime) {
                    statsBestTimeAttackTime = timeElapsed;
                }
            }
            gameInProgress = 0;
            StartVictoryCascade(hwnd);
            InvalidateRect(hwnd, NULL, TRUE);
            ShowStats(hwnd);
        }
    }
}

int IsSafeToAutoMove(Card c) {
    if (c.frozen) return 0;
    if (c.r <= 2) return 1;
    int minOppFound = 14;
    for (int i = 0; i < 4; i++) {
        int color = (i == 1 || i == 3) ? 1 : 0;
        if (color != c.color) {
            if (found[i] < minOppFound) {
                minOppFound = found[i];
            }
        }
    }
    return c.r <= minOppFound + 1;
}

int AutoComplete(HWND hwnd) {
    int didMove = 1;
    int anyMoved = 0;
    while(didMove) {
        didMove = 0;
        for (int i = 0; i < numFreeCells; i++) {
            if (freeCellsOccupied[i]) {
                Card c = freeCells[i];
                if (!c.frozen && c.r == found[c.s] + 1 && IsSafeToAutoMove(c)) {
                    RECT clientRect; GetClientRect(hwnd, &clientRect);
                    StartAnim(c, GetCardX(0, i, 0, clientRect), GetCardY(0, i, 0, clientRect), GetCardX(2, c.s, 0, clientRect), GetCardY(2, c.s, 0, clientRect));
                    SpawnSnapSparks(GetCardX(2, c.s, 0, clientRect) + CELL_W/2, GetCardY(2, c.s, 0, clientRect) + CELL_H/2);
                    PushUndo();
                    found[c.s] = c.r;
                    freeCellsOccupied[i] = 0;
                    moves++;
                    if (gameMode == 3) timeRemaining += 15;
                    didMove = 1;
                    anyMoved = 1;
                    TriggerScreenShake(4);
                    break;
                }
            }
        }
        if (didMove) continue;
        
        for (int i = 0; i < 8; i++) {
            if (tabCount[i] > 0) {
                Card c = tab[i][tabCount[i]-1];
                if (!c.frozen && c.r == found[c.s] + 1 && IsSafeToAutoMove(c)) {
                    RECT clientRect; GetClientRect(hwnd, &clientRect);
                    StartAnim(c, GetCardX(1, i, tabCount[i]-1, clientRect), GetCardY(1, i, tabCount[i]-1, clientRect), GetCardX(2, c.s, 0, clientRect), GetCardY(2, c.s, 0, clientRect));
                    SpawnSnapSparks(GetCardX(2, c.s, 0, clientRect) + CELL_W/2, GetCardY(2, c.s, 0, clientRect) + CELL_H/2);
                    PushUndo();
                    found[c.s] = c.r;
                    tabCount[i]--;
                    moves++;
                    if (gameMode == 3) timeRemaining += 15;
                    didMove = 1;
                    anyMoved = 1;
                    TriggerScreenShake(4);
                    break;
                }
            }
        }
    }
    ThawCheck();
    return anyMoved;
}

void UseWandPowerup(HWND hwnd) {
    if (gameInProgress && won == 0 && powerupsWand > 0) {
        powerupsWand--;
        PlaySoundEffect(3);
        int moved = AutoComplete(hwnd);
        if (!moved) {
            for (int i = 0; i < 8; i++) {
                if (tabCount[i] > 0) {
                    Card c = tab[i][tabCount[i]-1];
                    if (!c.frozen && c.r == found[c.s] + 1) {
                        PushUndo();
                        found[c.s] = c.r;
                        tabCount[i]--;
                        moves++;
                        moved = 1;
                        break;
                    }
                }
            }
            if (!moved) {
                for (int i = 0; i < numFreeCells; i++) {
                    if (!freeCellsOccupied[i]) {
                        for (int t = 0; t < 8; t++) {
                            if (tabCount[t] > 0) {
                                Card c = tab[t][tabCount[t]-1];
                                if (!c.frozen) {
                                    PushUndo();
                                    freeCells[i] = c;
                                    freeCellsOccupied[i] = 1;
                                    tabCount[t]--;
                                    moves++;
                                    moved = 1;
                                    break;
                                }
                            }
                        }
                        if (moved) break;
                    }
                }
            }
        }
        ThawCheck();
        CheckWin(hwnd);
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void UseExtraCellPowerup(HWND hwnd) {
    if (gameInProgress && won == 0 && powerupsExtraCell > 0) {
        if (numFreeCells < MAX_FREE_CELLS) {
            powerupsExtraCell--;
            numFreeCells++;
            extraCellActive = 1;
            extraCellTimer = 30;
            PlaySoundEffect(3);
            InvalidateRect(hwnd, NULL, TRUE);
        }
    }
}

void UseAutoSolvePowerup(HWND hwnd) {
    if (gameInProgress && won == 0) {
        int moved = AutoComplete(hwnd);
        if (moved) {
            PlaySoundEffect(3);
            CheckWin(hwnd);
            InvalidateRect(hwnd, NULL, TRUE);
        }
    }
}

void DrawSuitGDI(HDC hdc, int cx, int cy, int size, int suitIdx) {
    COLORREF darkColor = (suitIdx == 1 || suitIdx == 3) ? RGB(160, 0, 0) : RGB(20, 20, 20);
    COLORREF midColor = (suitIdx == 1 || suitIdx == 3) ? RGB(211, 47, 47) : RGB(60, 60, 60);
    COLORREF lightColor = (suitIdx == 1 || suitIdx == 3) ? RGB(255, 120, 120) : RGB(120, 120, 120);
    
    HBRUSH darkBrush = CreateSolidBrush(darkColor);
    HBRUSH midBrush = CreateSolidBrush(midColor);
    HBRUSH lightBrush = CreateSolidBrush(lightColor);
    HPEN nullPen = CreatePen(PS_NULL, 0, 0);
    SelectObject(hdc, nullPen);
    
    int r = size / 2;
    // Draw base shadow
    SelectObject(hdc, darkBrush);
    if (suitIdx == 0) {
        POINT topPt[3] = {{cx, cy - r}, {cx - r + 1, cy + r/3}, {cx + r - 1, cy + r/3}};
        Polygon(hdc, topPt, 3);
        Ellipse(hdc, cx - r, cy - r/4, cx, cy + r/2);
        Ellipse(hdc, cx, cy - r/4, cx + r, cy + r/2);
        POINT stemPt[3] = {{cx, cy}, {cx - r/2, cy + r}, {cx + r/2, cy + r}};
        Polygon(hdc, stemPt, 3);
    } else if (suitIdx == 1) {
        Ellipse(hdc, cx - r, cy - r, cx + 1, cy + r/4);
        Ellipse(hdc, cx - 1, cy - r, cx + r, cy + r/4);
        POINT botPt[3] = {{cx - r + 1, cy - r/4}, {cx + r - 1, cy - r/4}, {cx, cy + r}};
        Polygon(hdc, botPt, 3);
    } else if (suitIdx == 2) {
        Ellipse(hdc, cx - r/2 - 1, cy - r, cx + r/2 + 1, cy);
        Ellipse(hdc, cx - r, cy - r/2, cx, cy + r/2);
        Ellipse(hdc, cx, cy - r/2, cx + r, cy + r/2);
        POINT stemPt[3] = {{cx, cy}, {cx - r/2, cy + r}, {cx + r/2, cy + r}};
        Polygon(hdc, stemPt, 3);
    } else {
        POINT diaPt[4] = {{cx, cy - r}, {cx + r, cy}, {cx, cy + r}, {cx - r, cy}};
        Polygon(hdc, diaPt, 4);
    }
    
    // Draw mid layer for facets
    SelectObject(hdc, midBrush);
    int o1 = r > 8 ? 2 : 1;
    int r1 = r - o1;
    if (r1 > 0) {
        if (suitIdx == 0) {
            POINT topPt[3] = {{cx, cy - r1}, {cx - r1 + 1, cy + r1/3}, {cx + r1 - 1, cy + r1/3}};
            Polygon(hdc, topPt, 3);
            Ellipse(hdc, cx - r1, cy - r1/4, cx, cy + r1/2);
            Ellipse(hdc, cx, cy - r1/4, cx + r1, cy + r1/2);
            POINT stemPt[3] = {{cx, cy}, {cx - r1/2, cy + r1}, {cx + r1/2, cy + r1}};
            Polygon(hdc, stemPt, 3);
        } else if (suitIdx == 1) {
            Ellipse(hdc, cx - r1, cy - r1, cx + 1, cy + r1/4);
            Ellipse(hdc, cx - 1, cy - r1, cx + r1, cy + r1/4);
            POINT botPt[3] = {{cx - r1 + 1, cy - r1/4}, {cx + r1 - 1, cy - r1/4}, {cx, cy + r1}};
            Polygon(hdc, botPt, 3);
        } else if (suitIdx == 2) {
            Ellipse(hdc, cx - r1/2 - 1, cy - r1, cx + r1/2 + 1, cy);
            Ellipse(hdc, cx - r1, cy - r1/2, cx, cy + r1/2);
            Ellipse(hdc, cx, cy - r1/2, cx + r1, cy + r1/2);
            POINT stemPt[3] = {{cx, cy}, {cx - r1/2, cy + r1}, {cx + r1/2, cy + r1}};
            Polygon(hdc, stemPt, 3);
        } else {
            POINT diaPt[4] = {{cx, cy - r1}, {cx + r1, cy}, {cx, cy + r1}, {cx - r1, cy}};
            Polygon(hdc, diaPt, 4);
        }
    }
    
    // Draw light layer highlight
    SelectObject(hdc, lightBrush);
    int o2 = r > 8 ? 4 : 2;
    int r2 = r - o2;
    if (r2 > 0) {
        if (suitIdx == 0) {
            POINT topPt[3] = {{cx - o2/2, cy - r2}, {cx - r2 + 1 - o2/2, cy + r2/3}, {cx + r2 - 1 - o2/2, cy + r2/3}};
            Polygon(hdc, topPt, 3);
        } else if (suitIdx == 1) {
            Ellipse(hdc, cx - r2 - o2/2, cy - r2 - o2/2, cx + 1 - o2/2, cy + r2/4 - o2/2);
        } else if (suitIdx == 2) {
            Ellipse(hdc, cx - r2 - o2/2, cy - r2/2 - o2/2, cx - o2/2, cy + r2/2 - o2/2);
        } else {
            POINT diaPt[4] = {{cx, cy - r2 - o2/2}, {cx + r2, cy - o2/2}, {cx, cy + r2 - o2/2}, {cx - r2, cy - o2/2}};
            Polygon(hdc, diaPt, 4);
        }
    }
    
    DeleteObject(darkBrush);
    DeleteObject(midBrush);
    DeleteObject(lightBrush);
    DeleteObject(nullPen);
}

void DrawCard(HDC hdc, int x, int y, Card c, int selected, int glowing) {
    if (selected) {
        y -= 5;
        HBRUSH shadowBrush = CreateSolidBrush(RGB(10, 30, 15));
        HPEN nullPen = CreatePen(PS_NULL, 0, 0);
        SelectObject(hdc, shadowBrush);
        SelectObject(hdc, nullPen);
        RoundRect(hdc, x + 8, y + 12, x + CELL_W + 8, y + CELL_H + 12, 10, 10);
        DeleteObject(shadowBrush);
        DeleteObject(nullPen);
    } else {
        HBRUSH shadowBrush = CreateSolidBrush(RGB(20, 50, 25));
        HPEN nullPen = CreatePen(PS_NULL, 0, 0);
        SelectObject(hdc, shadowBrush);
        SelectObject(hdc, nullPen);
        RoundRect(hdc, x + 2, y + 4, x + CELL_W + 2, y + CELL_H + 4, 10, 10);
        DeleteObject(shadowBrush);
        DeleteObject(nullPen);
    }
    
    if (glowing) {
        HPEN glowPen = CreatePen(PS_SOLID, 3, RGB(255, 215, 0));
        HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
        SelectObject(hdc, glowPen);
        SelectObject(hdc, nullBrush);
        RoundRect(hdc, x - 4, y - 4, x + CELL_W + 4, y + CELL_H + 4, 14, 14);
        DeleteObject(glowPen);
    }

    HBRUSH bg = CreateSolidBrush(c.frozen ? RGB(224, 247, 250) : RGB(250, 250, 250));
    HPEN pen = CreatePen(PS_SOLID, selected ? 2 : (c.frozen ? 2 : 1), selected ? RGB(255, 215, 0) : (c.frozen ? RGB(0, 188, 212) : RGB(160, 160, 160)));
    
    SelectObject(hdc, bg);
    SelectObject(hdc, pen);
    RoundRect(hdc, x, y, x + CELL_W, y + CELL_H, 10, 10);
    
    // Subtle visual variation (noise texture dots)
    for(int i=0; i<30; i++) {
        int nx = x + 4 + rand() % (CELL_W - 8);
        int ny = y + 4 + rand() % (CELL_H - 8);
        SetPixel(hdc, nx, ny, c.frozen ? RGB(200, 230, 240) : RGB(230, 230, 230));
    }

    DeleteObject(bg);
    DeleteObject(pen);

    HPEN hiPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    SelectObject(hdc, hiPen);
    MoveToEx(hdc, x + 3, y + CELL_H - 6, NULL);
    LineTo(hdc, x + 3, y + 3);
    LineTo(hdc, x + CELL_W - 6, y + 3);
    DeleteObject(hiPen);
    
    HPEN shPen = CreatePen(PS_SOLID, 2, RGB(225, 225, 225));
    SelectObject(hdc, shPen);
    MoveToEx(hdc, x + 4, y + CELL_H - 3, NULL);
    LineTo(hdc, x + CELL_W - 3, y + CELL_H - 3);
    LineTo(hdc, x + CELL_W - 3, y + 4);
    DeleteObject(shPen);
    
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, c.frozen ? RGB(0, 131, 143) : (c.color ? RGB(211, 47, 47) : RGB(30, 30, 30)));
    
    WCHAR *ranks[] = {L"A",L"2",L"3",L"4",L"5",L"6",L"7",L"8",L"9",L"10",L"J",L"Q",L"K"};
    
    HFONT hFont = CreateFontW(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Arial");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
    RECT rT = {x + 4, y + 4, x + 24, y + 20};
    DrawTextW(hdc, ranks[c.r-1], -1, &rT, DT_LEFT | DT_TOP);
    DrawSuitGDI(hdc, x + 12, y + 26, 10, c.s);
    
    RECT rB = {x + CELL_W - 24, y + CELL_H - 20, x + CELL_W - 4, y + CELL_H - 4};
    DrawTextW(hdc, ranks[c.r-1], -1, &rB, DT_RIGHT | DT_BOTTOM);
    DrawSuitGDI(hdc, x + CELL_W - 12, y + CELL_H - 26, 10, c.s);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    
    if (c.frozen) {
        HFONT hIceFont = CreateFontW(12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        HFONT hOldIce = (HFONT)SelectObject(hdc, hIceFont);
        SetTextColor(hdc, RGB(0, 150, 180));
        RECT rIce = {x + 2, y + CELL_H/2 - 8, x + CELL_W - 2, y + CELL_H/2 + 10};
        DrawTextW(hdc, L"FROZEN", -1, &rIce, DT_CENTER | DT_SINGLELINE);
        SelectObject(hdc, hOldIce);
        DeleteObject(hIceFont);
    } else if (c.r == 1) {
        DrawSuitGDI(hdc, x + CELL_W/2, y + CELL_H/2, 24, c.s);
        HPEN ringPen = CreatePen(PS_DOT, 1, RGB(212, 175, 55));
        SelectObject(hdc, ringPen);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Ellipse(hdc, x + CELL_W/2 - 18, y + CELL_H/2 - 18, x + CELL_W/2 + 18, y + CELL_H/2 + 18);
        DeleteObject(ringPen);
    } else if (c.r >= 11) {
        int px = x + 14, py = y + 22, pw = CELL_W - 28, ph = CELL_H - 44;
        HBRUSH portraitBg = CreateSolidBrush(RGB(253, 251, 247));
        HPEN goldPen = CreatePen(PS_SOLID, 1, RGB(212, 175, 55));
        SelectObject(hdc, portraitBg);
        SelectObject(hdc, goldPen);
        Rectangle(hdc, px, py, px + pw, py + ph);
        
        COLORREF suitCol = c.color ? RGB(211, 47, 47) : RGB(30, 30, 30);
        HBRUSH suitBrush = CreateSolidBrush(suitCol);
        HBRUSH faceBrush = CreateSolidBrush(RGB(255, 224, 189));
        HBRUSH crownBrush = CreateSolidBrush(RGB(255, 215, 0));
        
        if (c.r == 11) {
            SelectObject(hdc, suitBrush);
            POINT hat[3] = {{px + pw/2, py + 4}, {px + 4, py + 14}, {px + pw - 4, py + 14}};
            Polygon(hdc, hat, 3);
            SelectObject(hdc, faceBrush);
            Ellipse(hdc, px + pw/2 - 6, py + 14, px + pw/2 + 6, py + 26);
            SelectObject(hdc, suitBrush);
            Rectangle(hdc, px + 6, py + 26, px + pw - 6, py + ph - 4);
        } else if (c.r == 12) {
            SelectObject(hdc, crownBrush);
            POINT crown[5] = {{px + 6, py + 14}, {px + 10, py + 6}, {px + pw/2, py + 10}, {px + pw - 10, py + 6}, {px + pw - 6, py + 14}};
            Polygon(hdc, crown, 5);
            SelectObject(hdc, faceBrush);
            Ellipse(hdc, px + pw/2 - 6, py + 14, px + pw/2 + 6, py + 26);
            SelectObject(hdc, suitBrush);
            Rectangle(hdc, px + 6, py + 26, px + pw - 6, py + ph - 4);
        } else {
            SelectObject(hdc, crownBrush);
            Rectangle(hdc, px + 6, py + 6, px + pw - 6, py + 14);
            SelectObject(hdc, faceBrush);
            Ellipse(hdc, px + pw/2 - 7, py + 14, px + pw/2 + 7, py + 27);
            HBRUSH beardBrush = CreateSolidBrush(RGB(109, 76, 65));
            SelectObject(hdc, beardBrush);
            Ellipse(hdc, px + pw/2 - 5, py + 22, px + pw/2 + 5, py + 30);
            DeleteObject(beardBrush);
            SelectObject(hdc, suitBrush);
            Rectangle(hdc, px + 4, py + 30, px + pw - 4, py + ph - 4);
        }
        
        DeleteObject(portraitBg);
        DeleteObject(goldPen);
        DeleteObject(suitBrush);
        DeleteObject(faceBrush);
        DeleteObject(crownBrush);
    } else {
        int cx = x + CELL_W / 2;
        int cy = y + CELL_H / 2;
        if (c.r == 2) {
            DrawSuitGDI(hdc, cx, cy - 16, 12, c.s);
            DrawSuitGDI(hdc, cx, cy + 16, 12, c.s);
        } else if (c.r == 3) {
            DrawSuitGDI(hdc, cx, cy - 20, 12, c.s);
            DrawSuitGDI(hdc, cx, cy, 12, c.s);
            DrawSuitGDI(hdc, cx, cy + 20, 12, c.s);
        } else {
            DrawSuitGDI(hdc, cx - 12, cy - 16, 12, c.s);
            DrawSuitGDI(hdc, cx + 12, cy - 16, 12, c.s);
            DrawSuitGDI(hdc, cx - 12, cy + 16, 12, c.s);
            DrawSuitGDI(hdc, cx + 12, cy + 16, 12, c.s);
            if (c.r == 5 || c.r == 9) DrawSuitGDI(hdc, cx, cy, 12, c.s);
        }
    }
}

void DrawEmptyCell(HDC hdc, int x, int y, int isFound, int suitIdx) {
    HBRUSH bg = CreateSolidBrush(RGB(10, 40, 20));
    DWORD t = GetTickCount();
    int pulse = (int)(sinf((float)t * 0.004f + (float)(x + y) * 0.01f) * 20.0f);
    int rGold = 212 + pulse; if(rGold > 255) rGold = 255; if(rGold < 180) rGold = 180;
    int gGold = 175 + pulse; if(gGold > 230) gGold = 230; if(gGold < 140) gGold = 140;
    COLORREF goldColor = RGB(rGold, gGold, 55);
    
    HPEN pen = CreatePen(PS_SOLID, 2, goldColor);
    SelectObject(hdc, bg);
    SelectObject(hdc, pen);
    RoundRect(hdc, x, y, x + CELL_W, y + CELL_H, 10, 10);
    DeleteObject(bg);
    DeleteObject(pen);
    
    // Procedural corner filigree brackets
    HPEN filigreePen = CreatePen(PS_SOLID, 1, RGB(rGold, gGold, 80));
    SelectObject(hdc, filigreePen);
    MoveToEx(hdc, x + 4, y + 10, NULL); LineTo(hdc, x + 4, y + 4); LineTo(hdc, x + 10, y + 4);
    MoveToEx(hdc, x + CELL_W - 11, y + 4, NULL); LineTo(hdc, x + CELL_W - 5, y + 4); LineTo(hdc, x + CELL_W - 5, y + 10);
    MoveToEx(hdc, x + 4, y + CELL_H - 11, NULL); LineTo(hdc, x + 4, y + CELL_H - 5); LineTo(hdc, x + 10, y + CELL_H - 5);
    MoveToEx(hdc, x + CELL_W - 11, y + CELL_H - 5, NULL); LineTo(hdc, x + CELL_W - 5, y + CELL_H - 5); LineTo(hdc, x + CELL_W - 5, y + CELL_H - 11);
    DeleteObject(filigreePen);
    
    if(isFound) {
        DrawSuitGDI(hdc, x + CELL_W/2, y + CELL_H/2, 28, suitIdx);
    }
}

void DrawCardBack(HDC hdc, int x, int y, int type) {
    HBRUSH bg;
    if (type == 0) bg = CreateSolidBrush(RGB(30, 60, 114));
    else if (type == 1) bg = CreateSolidBrush(RGB(128, 0, 0));
    else if (type == 2) bg = CreateSolidBrush(RGB(0, 77, 64));
    else bg = CreateSolidBrush(RGB(33, 33, 33));

    HPEN pen = CreatePen(PS_SOLID, 2, RGB(212, 175, 55));
    SelectObject(hdc, bg);
    SelectObject(hdc, pen);
    RoundRect(hdc, x, y, x + CELL_W, y + CELL_H, 10, 10);
    
    HPEN innerPen = CreatePen(PS_SOLID, 1, RGB(255, 215, 0));
    SelectObject(hdc, innerPen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    RoundRect(hdc, x + 5, y + 5, x + CELL_W - 5, y + CELL_H - 5, 6, 6);
    
    HBRUSH goldBrush = CreateSolidBrush(RGB(212, 175, 55));
    SelectObject(hdc, goldBrush);
    POINT shield[4] = {{x + CELL_W/2, y + 25}, {x + CELL_W/2 + 14, y + 35}, {x + CELL_W/2, y + 65}, {x + CELL_W/2 - 14, y + 35}};
    Polygon(hdc, shield, 4);
    
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(20, 20, 20));
    HFONT hFont = CreateFontW(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Arial");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    RECT r = {x, y + 32, x + CELL_W, y + 58};
    DrawTextW(hdc, L"K", -1, &r, DT_CENTER | DT_SINGLELINE);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    DeleteObject(goldBrush);
    DeleteObject(innerPen);
    DeleteObject(bg);
    DeleteObject(pen);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            LoadSettings();
            SetTimer(hwnd, 1, 16, NULL);
            srand((unsigned int)time(NULL));
            InitGame();
            return 0;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
            
            // Draw Mahogany Table Frame Border
            int borderSize = 16;
            HBRUSH mahog1 = CreateSolidBrush(RGB(74, 33, 17));
            HBRUSH mahog2 = CreateSolidBrush(RGB(43, 17, 4));
            RECT borderTop = {0, 0, clientRect.right, borderSize};
            RECT borderBot = {0, clientRect.bottom - borderSize, clientRect.right, clientRect.bottom};
            RECT borderL = {0, 0, borderSize, clientRect.bottom};
            RECT borderR = {clientRect.right - borderSize, 0, clientRect.right, clientRect.bottom};
            FillRect(hdcMem, &borderTop, mahog1);
            FillRect(hdcMem, &borderBot, mahog2);
            FillRect(hdcMem, &borderL, mahog1);
            FillRect(hdcMem, &borderR, mahog2);
            DeleteObject(mahog1);
            DeleteObject(mahog2);
            
            // Draw Rich Casino Felt Background
            RECT feltRect = {borderSize, borderSize, clientRect.right - borderSize, clientRect.bottom - borderSize};
            HBRUSH hbrBg = CreateSolidBrush(RGB(20, 90, 45));
            FillRect(hdcMem, &feltRect, hbrBg);
            DeleteObject(hbrBg);
            
            // Felt pattern grid lines (subtle)
            HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(22, 98, 49));
            SelectObject(hdcMem, gridPen);
            for(int gx = borderSize; gx < clientRect.right - borderSize; gx += 40) {
                MoveToEx(hdcMem, gx, borderSize, NULL); LineTo(hdcMem, gx, clientRect.bottom - borderSize);
            }
            for(int gy = borderSize; gy < clientRect.bottom - borderSize; gy += 40) {
                MoveToEx(hdcMem, borderSize, gy, NULL); LineTo(hdcMem, clientRect.right - borderSize, gy);
            }
            DeleteObject(gridPen);
            
            HPEN feltBorder = CreatePen(PS_SOLID, 4, RGB(10, 56, 26));
            SelectObject(hdcMem, feltBorder);
            SelectObject(hdcMem, GetStockObject(NULL_BRUSH));
            Rectangle(hdcMem, feltRect.left, feltRect.top, feltRect.right, feltRect.bottom);
            DeleteObject(feltBorder);
            
            // Loop 8 Luxury Inlay, Sheen Sweeps and Ornate HUD Corners
            DWORD nowTick = GetTickCount();
            DrawPerimeterInlay(hdcMem, clientRect.right, clientRect.bottom, nowTick);
            DrawSheenSweeps(hdcMem, clientRect.right, clientRect.bottom, nowTick);
            DrawOrnateHUDCorners(hdcMem, clientRect.right, clientRect.bottom);
            
            SetBkMode(hdcMem, TRANSPARENT);
            SetTextColor(hdcMem, RGB(255, 215, 0));
            HFONT hTitleFont = CreateFontW(17, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(hdcMem, hTitleFont);
            RECT titleRect = {0, 10, clientRect.right, 40};
            WCHAR titleMsg[256];
            int m = timeElapsed / 60;
            int s = timeElapsed % 60;
            if (gameMode == 3) {
                int tm = timeRemaining / 60;
                int ts = timeRemaining % 60;
                wsprintfW(titleMsg, L"TIME LEFT: %02d:%02d (+15s)  Moves: %d  |  Time Attack  |  [W]Wand(%d) [E]Cell(%d) [A]Solve [U]Undo [P]Shuf(%d)", tm, ts, moves, powerupsWand, powerupsExtraCell, powerupsShuffle);
            } else if (gameMode == 2) {
                wsprintfW(titleMsg, L"Time: %02d:%02d  Moves: %d  |  Stage %d/20 (%s%s)  |  [W]Wand(%d) [E]Cell(%d) [A]Solve [U]Undo [P]Shuf(%d)", m, s, moves, campaignStage, buildRule==1?L"Suit":L"Color", emptyKingOnly?L"+King":L"", powerupsWand, powerupsExtraCell, powerupsShuffle);
            } else if (gameMode == 1) {
                wsprintfW(titleMsg, L"Time: %02d:%02d  Moves: %d  |  Deal #%d (+/-)  |  [W]Wand(%d) [E]Cell(%d) [A]Solve [U]Undo [P]Shuf(%d)", m, s, moves, currentSeed, powerupsWand, powerupsExtraCell, powerupsShuffle);
            } else {
                wsprintfW(titleMsg, L"Time: %02d:%02d  Moves: %d  |  Random Deal  |  [W]Wand(%d) [E]Cell(%d) [A]Solve [U]Undo [P]Shuf(%d)", m, s, moves, powerupsWand, powerupsExtraCell, powerupsShuffle);
            }
            DrawTextW(hdcMem, titleMsg, -1, &titleRect, DT_CENTER | DT_TOP);
            if(won == 1) {
                RECT winRect = {0, 10, clientRect.right - 20, 40};
                DrawTextW(hdcMem, L"You Win!", -1, &winRect, DT_RIGHT | DT_TOP);
            } else if(won == -1) {
                RECT winRect = {0, 10, clientRect.right - 20, 40};
                DrawTextW(hdcMem, L"Time Expired!", -1, &winRect, DT_RIGHT | DT_TOP);
            }
            SelectObject(hdcMem, hOldFont);
            DeleteObject(hTitleFont);
            
            // Draw Free Cells
            int group1X = PAD;
            for(int i=0; i<numFreeCells; i++) {
                int x = group1X + i*(CELL_W + TAB_PAD_X);
                if(freeCellsOccupied[i]) {
                    int ax, ay;
                    if(!IsAnimating(freeCells[i], &ax, &ay)) {
                        int glowing = (!freeCells[i].frozen && freeCells[i].r == found[freeCells[i].s] + 1 && IsSafeToAutoMove(freeCells[i]));
                        DrawCard(hdcMem, x, TOP_Y, freeCells[i], (selType==0 && selIdx==i), glowing);
                    }
                } else {
                    DrawEmptyCell(hdcMem, x, TOP_Y, 0, 0);
                }
            }
            
            // Draw Decorative Deck
            int deckX = (clientRect.right - CELL_W) / 2;
            DrawCardBack(hdcMem, deckX, TOP_Y, settingsCardBack);
            
            // Draw Foundations
            int group2X = clientRect.right - PAD - 4*(CELL_W + TAB_PAD_X);
            for(int i=0; i<4; i++) {
                int x = group2X + i*(CELL_W + TAB_PAD_X);
                if(found[i] > 0) {
                    Card c = {i, found[i], (i==1||i==3)?1:0, 0};
                    int ax, ay;
                    if(!IsAnimating(c, &ax, &ay)) {
                        DrawCard(hdcMem, x, TOP_Y, c, 0, 0);
                    } else if(found[i] > 1) {
                        Card cPrev = {i, found[i]-1, (i==1||i==3)?1:0, 0};
                        DrawCard(hdcMem, x, TOP_Y, cPrev, 0, 0);
                    } else {
                        DrawEmptyCell(hdcMem, x, TOP_Y, 1, i);
                    }
                } else {
                    DrawEmptyCell(hdcMem, x, TOP_Y, 1, i);
                }
            }
            
            // Draw Tableau
            int totalTabW = 8 * CELL_W + 7 * TAB_PAD_X;
            int tabStartX = (clientRect.right - totalTabW) / 2;
            for(int i=0; i<8; i++) {
                int x = tabStartX + i*(CELL_W + TAB_PAD_X);
                if(tabCount[i] == 0) {
                    DrawEmptyCell(hdcMem, x, TAB_Y, 0, 0);
                } else {
                    for(int j=0; j<tabCount[i]; j++) {
                        int y = TAB_Y + j*TAB_PAD_Y;
                        int selected = (selType==1 && selIdx==i && j>=selCardIdx);
                        int ax, ay;
                        if(!IsAnimating(tab[i][j], &ax, &ay)) {
                            int glowing = (j == tabCount[i]-1 && !tab[i][j].frozen && tab[i][j].r == found[tab[i][j].s] + 1 && IsSafeToAutoMove(tab[i][j]));
                            DrawCard(hdcMem, x, y, tab[i][j], selected, glowing);
                        }
                    }
                }
            }

            for(int i=0; i<MAX_ANIMS; i++) {
                if(anims[i].active) {
                    int cx, cy;
                    if(IsAnimating(anims[i].c, &cx, &cy)) {
                        DWORD now = GetTickCount();
                        float t = (float)(now - anims[i].startTime) / anims[i].duration;
                        if (anims[i].endY == TOP_Y && anims[i].endX >= clientRect.right - PAD - 4*(CELL_W + TAB_PAD_X) - 10) {
                            for(int fold=1; fold<=3; fold++) {
                                float ft = t - (0.08f * fold);
                                if(ft > 0) {
                                    int fcx = anims[i].startX + (int)(ft * (anims[i].endX - anims[i].startX));
                                    int fcy = anims[i].startY + (int)(ft * (anims[i].endY - anims[i].startY)) - (fold * 8);
                                    DrawCard(hdcMem, fcx, fcy, anims[i].c, 0, 0);
                                }
                            }
                        }
                        DrawCard(hdcMem, cx, cy, anims[i].c, 0, 0);
                    }
                }
            }
            
            // Draw Shockwaves
            DrawShockwavesGDI(hdcMem);

            // Draw Multi-Layered Particles (Sparks, Smoke, Debris, Stars)
            for(int pidx=0; pidx<MAX_PARTICLES; pidx++) {
                if(particles[pidx].active) {
                    if (particles[pidx].type == PARTICLE_STAR) {
                        int sz = particles[pidx].size;
                        Draw4PointStarGDI(hdcMem, (int)particles[pidx].x, (int)particles[pidx].y, sz, sz / 3 > 0 ? sz / 3 : 1, particles[pidx].color);
                    } else if (particles[pidx].type == PARTICLE_DEBRIS) {
                        float rad = particles[pidx].spin;
                        float c_rot = cosf(rad);
                        float s_rot = sinf(rad);
                        int sz = particles[pidx].size;
                        POINT pts[4];
                        pts[0].x = (int)(particles[pidx].x + (-sz * c_rot - 0 * s_rot));
                        pts[0].y = (int)(particles[pidx].y + (-sz * s_rot + 0 * c_rot));
                        pts[1].x = (int)(particles[pidx].x + (0 * c_rot - (sz * 1.4f) * s_rot));
                        pts[1].y = (int)(particles[pidx].y + (0 * s_rot + (sz * 1.4f) * c_rot));
                        pts[2].x = (int)(particles[pidx].x + (sz * c_rot - 0 * s_rot));
                        pts[2].y = (int)(particles[pidx].y + (sz * s_rot + 0 * c_rot));
                        pts[3].x = (int)(particles[pidx].x + (0 * c_rot - (-sz * 1.4f) * s_rot));
                        pts[3].y = (int)(particles[pidx].y + (0 * s_rot + (-sz * 1.4f) * c_rot));
                        HBRUSH dbr = CreateSolidBrush(particles[pidx].color);
                        HPEN dpen = CreatePen(PS_SOLID, 1, RGB(255,255,255));
                        SelectObject(hdcMem, dbr);
                        SelectObject(hdcMem, dpen);
                        Polygon(hdcMem, pts, 4);
                        DeleteObject(dbr);
                        DeleteObject(dpen);
                    } else if (particles[pidx].type == PARTICLE_SMOKE) {
                        int sz = particles[pidx].size;
                        HBRUSH smkBr = CreateSolidBrush(particles[pidx].color);
                        HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
                        SelectObject(hdcMem, smkBr);
                        SelectObject(hdcMem, nullPen);
                        Ellipse(hdcMem, (int)particles[pidx].x - sz, (int)particles[pidx].y - sz, (int)particles[pidx].x + sz, (int)particles[pidx].y + sz);
                        DeleteObject(smkBr);
                    } else {
                        HBRUSH pbr = CreateSolidBrush(particles[pidx].color);
                        RECT pr = { (int)particles[pidx].x - particles[pidx].size/2, (int)particles[pidx].y - particles[pidx].size/2, (int)particles[pidx].x + particles[pidx].size/2, (int)particles[pidx].y + particles[pidx].size/2 };
                        FillRect(hdcMem, &pr, pbr);
                        DeleteObject(pbr);
                        if (particles[pidx].size > 2) {
                            HPEN glowP = CreatePen(PS_SOLID, 1, particles[pidx].color);
                            SelectObject(hdcMem, glowP);
                            SelectObject(hdcMem, GetStockObject(NULL_BRUSH));
                            Ellipse(hdcMem, (int)particles[pidx].x - particles[pidx].size, (int)particles[pidx].y - particles[pidx].size, (int)particles[pidx].x + particles[pidx].size, (int)particles[pidx].y + particles[pidx].size);
                            DeleteObject(glowP);
                        }
                    }
                }
            }
            
            // Draw Animated Procedural Fabric Glint Effect
            for(int i=0; i<MAX_DUST; i++) {
                if (dustInit) {
                    float glintAlpha = dustMotes[i].alpha * 1.5f;
                    if (glintAlpha > 1.0f) glintAlpha = 1.0f;
                    int r = (int)(255 * glintAlpha);
                    int g = (int)(215 * glintAlpha);
                    int b = (int)(100 * glintAlpha);
                    HBRUSH dbr = CreateSolidBrush(RGB(r, g, b));
                    int gcx = (int)dustMotes[i].x;
                    int gcy = (int)dustMotes[i].y;
                    RECT dr = { gcx, gcy, gcx + dustMotes[i].size, gcy + dustMotes[i].size };
                    FillRect(hdcMem, &dr, dbr);
                    DeleteObject(dbr);
                }
            }

            if (cascadeActive) {
                for(int i=0; i<MAX_CASCADE; i++) {
                    if (cascadeCards[i].active) {
                        DrawCard(hdcMem, (int)cascadeCards[i].x, (int)cascadeCards[i].y, cascadeCards[i].c, 0, 0);
                    }
                }
            }
            
            int shakeX = 0;
            int shakeY = 0;
            if (shakeTrauma > 0.01f && shakeMaxTicks > 0) {
                float progress = (float)shakeTicks / (float)shakeMaxTicks;
                if (progress > 1.0f) progress = 1.0f;
                float decay = 1.0f - progress;
                float currentAmp = shakeMaxTrauma * (decay * decay);
                int iAmp = (int)currentAmp;
                if (iAmp > 0) {
                    shakeX = (rand() % (iAmp * 2 + 1)) - iAmp;
                    shakeY = (rand() % (iAmp * 2 + 1)) - iAmp;
                }
            }
            BitBlt(hdc, shakeX, shakeY, clientRect.right, clientRect.bottom, hdcMem, 0, 0, SRCCOPY);
            
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_LBUTTONDOWN: {
            if(won != 0) return 0;
            int mx = (short)LOWORD(lParam);
            int my = (short)HIWORD(lParam);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            
            int clickedType = -1;
            int clickedIdx = -1;
            int clickedCardIdx = 0;
            
            int group1X = PAD;
            for(int i=0; i<numFreeCells; i++) {
                int x = group1X + i*(CELL_W + TAB_PAD_X);
                if(mx >= x && mx <= x+CELL_W && my >= TOP_Y && my <= TOP_Y+CELL_H) {
                    clickedType = 0;
                    clickedIdx = i;
                    break;
                }
            }
            
            if(clickedType == -1) {
                int group2X = clientRect.right - PAD - 4*(CELL_W + TAB_PAD_X);
                for(int i=0; i<4; i++) {
                    int x = group2X + i*(CELL_W + TAB_PAD_X);
                    if(mx >= x && mx <= x+CELL_W && my >= TOP_Y && my <= TOP_Y+CELL_H) {
                        clickedType = 2;
                        clickedIdx = i;
                        break;
                    }
                }
            }
            
            if(clickedType == -1) {
                int totalTabW = 8 * CELL_W + 7 * TAB_PAD_X;
                int tabStartX = (clientRect.right - totalTabW) / 2;
                for(int i=0; i<8; i++) {
                    int x = tabStartX + i*(CELL_W + TAB_PAD_X);
                    if(mx >= x && mx <= x+CELL_W) {
                        if(tabCount[i] == 0) {
                            if(my >= TAB_Y && my <= TAB_Y+CELL_H) {
                                clickedType = 1;
                                clickedIdx = i;
                                clickedCardIdx = 0;
                                break;
                            }
                        } else {
                            for(int j=tabCount[i]-1; j>=0; j--) {
                                int y = TAB_Y + j*TAB_PAD_Y;
                                int h = (j == tabCount[i]-1) ? CELL_H : TAB_PAD_Y;
                                if(my >= y && my <= y+h) {
                                    clickedType = 1;
                                    clickedIdx = i;
                                    clickedCardIdx = j;
                                    break;
                                }
                            }
                            if(clickedType != -1) break;
                        }
                    }
                }
            }
            
            if(selType == -1) {
                if(clickedType == 1 && tabCount[clickedIdx] > 0) {
                    if (tab[clickedIdx][clickedCardIdx].frozen) {
                        PlaySoundEffect(4);
                        return 0;
                    }
                    int groupSize = GetDraggableGroup(clickedIdx, clickedCardIdx);
                    if(groupSize > 0 && groupSize <= GetMaxMoveCount()) {
                        selType = clickedType;
                        selIdx = clickedIdx;
                        selCardIdx = clickedCardIdx;
                        PlaySoundEffect(0);
                    }
                } else if(clickedType == 0 && freeCellsOccupied[clickedIdx]) {
                    if (freeCells[clickedIdx].frozen) {
                        PlaySoundEffect(4);
                        return 0;
                    }
                    selType = clickedType;
                    selIdx = clickedIdx;
                    selCardIdx = 0;
                    PlaySoundEffect(0);
                }
            } else {
                int moved = 0;
                
                Card cardsToMove[52];
                int nToMove = 0;
                if(selType == 1) {
                    nToMove = tabCount[selIdx] - selCardIdx;
                    for(int i=0; i<nToMove; i++) {
                        cardsToMove[i] = tab[selIdx][selCardIdx + i];
                    }
                } else if(selType == 0) {
                    nToMove = 1;
                    cardsToMove[0] = freeCells[selIdx];
                }
                
                if(clickedType == 1) {
                    int emptyFree = 0;
                    for(int i=0; i<numFreeCells; i++) if(!freeCellsOccupied[i]) emptyFree++;
                    int emptyTab = 0;
                    for(int i=0; i<8; i++) if(tabCount[i] == 0 && i != clickedIdx) emptyTab++;
                    int maxMove = (emptyFree + 1) * (1 << emptyTab);
                    
                    if(nToMove <= maxMove && CanMoveToTab(cardsToMove[0], clickedIdx)) {
                        for(int i=0; i<nToMove; i++) {
                            StartAnim(cardsToMove[i], GetCardX(selType, selIdx, selCardIdx + i, clientRect), GetCardY(selType, selIdx, selCardIdx + i, clientRect), GetCardX(clickedType, clickedIdx, tabCount[clickedIdx] + i, clientRect), GetCardY(clickedType, clickedIdx, tabCount[clickedIdx] + i, clientRect));
                        }
                        PushUndo();
                        for(int i=0; i<nToMove; i++) {
                            tab[clickedIdx][tabCount[clickedIdx]++] = cardsToMove[i];
                        }
                        moved = 1;
                    }
                } else if(clickedType == 0 && nToMove == 1 && !freeCellsOccupied[clickedIdx]) {
                    StartAnim(cardsToMove[0], GetCardX(selType, selIdx, selCardIdx, clientRect), GetCardY(selType, selIdx, selCardIdx, clientRect), GetCardX(clickedType, clickedIdx, 0, clientRect), GetCardY(clickedType, clickedIdx, 0, clientRect));
                    PushUndo();
                    freeCells[clickedIdx] = cardsToMove[0];
                    freeCellsOccupied[clickedIdx] = 1;
                    moved = 1;
                } else if(clickedType == 2 && nToMove == 1 && cardsToMove[0].s == clickedIdx) {
                    if(cardsToMove[0].r == found[clickedIdx] + 1) {
                        StartAnim(cardsToMove[0], GetCardX(selType, selIdx, selCardIdx, clientRect), GetCardY(selType, selIdx, selCardIdx, clientRect), GetCardX(clickedType, clickedIdx, 0, clientRect), GetCardY(clickedType, clickedIdx, 0, clientRect));
                        SpawnSnapSparks(GetCardX(clickedType, clickedIdx, 0, clientRect) + CELL_W/2, GetCardY(clickedType, clickedIdx, 0, clientRect) + CELL_H/2);
                        PushUndo();
                        found[clickedIdx] = cardsToMove[0].r;
                        if (gameMode == 3) timeRemaining += 15;
                        moved = 1;
                    }
                }
                
                if(moved) {
                    moves++;
                    if(selType == 1) {
                        tabCount[selIdx] = selCardIdx;
                        ThawAdjacent(selIdx);
                    } else if(selType == 0) {
                        freeCellsOccupied[selIdx] = 0;
                        if (extraCellActive && extraCellTimer == 0 && selIdx == numFreeCells - 1) {
                            numFreeCells--;
                            extraCellActive = 0;
                        }
                    }
                    if (clickedType == 1) ThawAdjacent(clickedIdx);
                    TriggerScreenShake(5);
                    PlaySoundEffect(1);
                    AutoComplete(hwnd);
                    CheckWin(hwnd);
                }
                selType = -1;
            }
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        
        case WM_KEYDOWN:
            if(wParam == 'R' || wParam == 'N') {
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if(wParam == 'Z' || wParam == 'U') {
                UndoMove(hwnd);
            } else if(wParam == 'A') {
                UseAutoSolvePowerup(hwnd);
            } else if(wParam == 'S') {
                ShowStats(hwnd);
            } else if(wParam == 'C') {
                settingsCardBack = (settingsCardBack + 1) % 4;
                SaveSettings();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if(wParam == 'M') {
                gameMode = (gameMode + 1) % 4;
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if(wParam == VK_OEM_PLUS || wParam == VK_ADD) {
                if (gameMode == 1) {
                    currentSeed++;
                    InitGame();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if(wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {
                if (gameMode == 1) {
                    if (currentSeed > 1) currentSeed--;
                    InitGame();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if(wParam == VK_F5) {
                SaveGame(hwnd);
            } else if(wParam == VK_F9) {
                LoadGame(hwnd);
            } else if(wParam == 'H' || wParam == VK_F1) {
                ShowHelp(hwnd);
            } else if(wParam == 'P') {
                if (gameInProgress && won == 0 && powerupsShuffle > 0) {
                    powerupsShuffle--;
                    PushUndo();
                    Card flat[52];
                    int flatCount = 0;
                    for(int i=0; i<8; i++) {
                        for(int j=0; j<tabCount[i]; j++) {
                            flat[flatCount++] = tab[i][j];
                        }
                    }
                    for(int i=flatCount-1; i>0; i--) {
                        int j = rand() % (i + 1);
                        Card t = flat[i]; flat[i] = flat[j]; flat[j] = t;
                    }
                    flatCount = 0;
                    for(int i=0; i<8; i++) {
                        for(int j=0; j<tabCount[i]; j++) {
                            tab[i][j] = flat[flatCount++];
                        }
                    }
                    ThawCheck();
                    PlaySoundEffect(1);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if(wParam == 'W') {
                UseWandPowerup(hwnd);
            } else if(wParam == 'E') {
                UseExtraCellPowerup(hwnd);
            } else if(wParam == VK_ESCAPE) {
                selType = -1;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;
            
        case WM_TIMER: {
            int active = 0;
            DWORD now = GetTickCount();
            
            // Atmospheric Dust Motes logic
            RECT crDust; GetClientRect(hwnd, &crDust);
            int dustW = crDust.right > 0 ? crDust.right : 800;
            int dustH = crDust.bottom > 0 ? crDust.bottom : 600;
            if(!dustInit && dustW > 0 && dustH > 0) {
                dustInit = 1;
                for(int i=0; i<MAX_DUST; i++) {
                    dustMotes[i].x = (float)(rand() % dustW);
                    dustMotes[i].y = (float)(rand() % dustH);
                    dustMotes[i].vx = (float)((rand()%100)*0.005f - 0.2f);
                    dustMotes[i].vy = (float)(-((rand()%100)*0.005f) - 0.1f);
                    dustMotes[i].size = 1 + rand()%2;
                    dustMotes[i].alpha = (float)(rand()%100)*0.01f;
                    dustMotes[i].da = (float)((rand()%100)*0.0004f - 0.0002f);
                }
            }
            if(dustInit) {
                for(int i=0; i<MAX_DUST; i++) {
                    dustMotes[i].x += dustMotes[i].vx;
                    dustMotes[i].y += dustMotes[i].vy;
                    dustMotes[i].alpha += dustMotes[i].da;
                    if(dustMotes[i].alpha > 1.0f) { dustMotes[i].alpha = 1.0f; dustMotes[i].da = -dustMotes[i].da; }
                    if(dustMotes[i].alpha < 0.1f) { dustMotes[i].alpha = 0.1f; dustMotes[i].da = -dustMotes[i].da; }
                    if(dustMotes[i].y < 0) dustMotes[i].y = (float)dustH;
                    if(dustMotes[i].x < 0) dustMotes[i].x = (float)dustW;
                    if(dustMotes[i].x > dustW) dustMotes[i].x = 0;
                }
                active = 1;
            }
            
            for(int i=0; i<MAX_ANIMS; i++) {
                if(anims[i].active) {
                    if (now >= anims[i].startTime + anims[i].duration) {
                        anims[i].active = 0;
                    }
                    active = 1;
                }
            }
            
            if (shakeTrauma > 0.01f) {
                shakeTicks++;
                if (shakeTicks >= shakeMaxTicks) {
                    shakeTrauma = 0.0f;
                }
                active = 1;
            }
            
            // Update Shockwaves
            for(int swi=0; swi<MAX_SHOCKWAVES; swi++) {
                if(shockwaves[swi].active) {
                    shockwaves[swi].radius += shockwaves[swi].speed;
                    if(shockwaves[swi].radius >= shockwaves[swi].maxRadius) {
                        shockwaves[swi].active = 0;
                    } else {
                        active = 1;
                    }
                }
            }

            // Update Particles with Multi-Layered Physics
            for(int pidx=0; pidx<MAX_PARTICLES; pidx++) {
                if(particles[pidx].active) {
                    particles[pidx].x += particles[pidx].vx;
                    particles[pidx].y += particles[pidx].vy;
                    
                    if (particles[pidx].type == PARTICLE_STAR) {
                        particles[pidx].vx *= 0.92f;
                        particles[pidx].vy *= 0.92f;
                    } else if (particles[pidx].type == PARTICLE_DEBRIS) {
                        particles[pidx].vx *= 0.94f; // Air friction
                        particles[pidx].vy += 0.35f; // Heavy gravity
                        particles[pidx].spin += particles[pidx].spinSpeed; // Tumbling spin
                    } else if (particles[pidx].type == PARTICLE_SMOKE) {
                        particles[pidx].vx *= 0.88f; // Strong air drag
                        particles[pidx].vy -= 0.08f; // Smoke rises gently
                        particles[pidx].size = (int)(particles[pidx].size * 1.02f); // Expand puff
                    } else { // Spark
                        particles[pidx].vx *= 0.96f;
                        if(won != 0 && cascadeActive) particles[pidx].vy += 0.15f; // Gravity for fireworks
                        else if(particles[pidx].decay == 0.02f) particles[pidx].vy -= 0.1f; // Float up for wand
                    }
                    
                    particles[pidx].life -= particles[pidx].decay;
                    if(particles[pidx].life <= 0) particles[pidx].active = 0;
                    else active = 1;
                }
            }
            if (won == 1 && fireworksBursts < 10) {
                if (now - lastBurstTime > 400) {
                    RECT clientRect; GetClientRect(hwnd, &clientRect);
                    SpawnVictoryFireworksBurst(clientRect.right, clientRect.bottom);
                    fireworksBursts++;
                    lastBurstTime = now;
                    active = 1;
                }
            }

            if (cascadeActive) {
                RECT clientRect; GetClientRect(hwnd, &clientRect);
                cascadeFrame++;
                int remaining = 0;
                for(int i=0; i<MAX_CASCADE; i++) {
                    if (cascadeFrame >= cascadeCards[i].delay) cascadeCards[i].active = 1;
                    if (cascadeCards[i].active) {
                        remaining = 1;
                        cascadeCards[i].x += cascadeCards[i].vx;
                        cascadeCards[i].y += cascadeCards[i].vy;
                        cascadeCards[i].vy += 0.5f;
                        
                        if (cascadeCards[i].y >= clientRect.bottom - CELL_H) {
                            cascadeCards[i].y = (float)(clientRect.bottom - CELL_H);
                            cascadeCards[i].vy = -cascadeCards[i].vy * 0.75f;
                            cascadeCards[i].vx += ((rand() % 20) - 10) * 0.1f;
                        }
                        if (cascadeCards[i].x <= 0 || cascadeCards[i].x >= clientRect.right - CELL_W) {
                            cascadeCards[i].vx = -cascadeCards[i].vx;
                        }
                    }
                }
                if (remaining && cascadeFrame < 600) active = 1;
            }
            
            if(gameInProgress && won == 0) {
                if(now - lastTimeTick >= 1000) {
                    timeElapsed++;
                    if (extraCellActive && extraCellTimer > 0) {
                        extraCellTimer--;
                        if (extraCellTimer == 0) {
                            int extraIdx = numFreeCells - 1;
                            if (!freeCellsOccupied[extraIdx]) {
                                numFreeCells--;
                                extraCellActive = 0;
                            }
                        }
                    }
                    if (gameMode == 3) {
                        timeRemaining--;
                        if (timeRemaining <= 30 && timeRemaining > 0 && timeRemaining % 5 == 0) {
                            PlaySoundEffect(4);
                        }
                        if (timeRemaining <= 0) {
                            won = -1;
                            gameInProgress = 0;
                            PlaySoundEffect(1);
                            MessageBoxW(hwnd, L"Time Expired! Game Over.", L"Time Attack", MB_OK | MB_ICONWARNING);
                        }
                    }
                    lastTimeTick = now;
                    active = 1;
                }
            }
            if(active) {
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"KFreecellClass";
    RegisterClassW(&wc);
    
    HWND hwnd = CreateWindowW(L"KFreecellClass", L"KFreecell",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 700,
        NULL, NULL, hInstance, NULL);
        
    MSG msg;
    while(GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    ExitProcess(0);
}
