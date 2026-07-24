#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}

char* __cdecl strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

int __cdecl abs(int x) { return x < 0 ? -x : x; }
double __cdecl fabs(double x) { return x < 0.0 ? -x : x; }

double __cdecl floor(double x) {
    int i = (int)x;
    return (x < 0.0 && x != (double)i) ? (double)(i - 1) : (double)i;
}

double __cdecl sqrt(double x) {
    if (x <= 0.0) return 0.0;
    double val = x;
    for (int i = 0; i < 10; i++) {
        val = 0.5 * (val + x / val);
    }
    return val;
}

#define PI 3.14159265358979323846
double __cdecl sin(double x) {
    while (x < -PI) x += 2 * PI;
    while (x > PI) x -= 2 * PI;
    double x2 = x * x;
    return x * (1.0 - x2 / 6.0 + (x2 * x2) / 120.0 - (x2 * x2 * x2) / 5040.0);
}
double __cdecl cos(double x) {
    return sin(x + PI / 2.0);
}

double __cdecl atan2(double y, double x) {
    if (x == 0.0) return (y > 0.0) ? (PI / 2.0) : ((y < 0.0) ? (-PI / 2.0) : 0.0);
    double atan = y / x;
    if (fabs(atan) < 1.0) {
        atan = atan / (1.0 + 0.28 * atan * atan);
        if (x < 0.0) return (y >= 0.0) ? (atan + PI) : (atan - PI);
        return atan;
    } else {
        atan = (PI / 2.0) - (1.0 / atan) / (1.0 + 0.28 / (atan * atan));
        if (y < 0.0) atan = -atan;
        if (x < 0.0) return (y >= 0.0) ? (atan + PI) : (atan - PI);
        return atan;
    }
}

static unsigned long g_seed = 1;
void __cdecl srand(unsigned int seed) { g_seed = seed; }
int __cdecl rand(void) {
    g_seed = g_seed * 1103515245 + 12345;
    return (int)((g_seed / 65536) % 32768);
}

const int W = 320;
const int H = 240;

const int orig_map1[10][10] = {
    {1,1,1,1,1,1,1,1,1,1},
    {1,0,0,5,0,0,1,5,0,1},
    {1,0,1,1,0,0,1,0,0,1},
    {1,5,0,1,0,0,0,0,0,1},
    {1,0,0,1,1,1,1,0,0,1},
    {1,0,0,0,0,0,0,0,5,1},
    {1,1,0,1,1,1,1,1,0,1},
    {1,0,0,0,5,0,0,1,0,1},
    {1,0,0,0,0,0,0,2,0,1},
    {1,1,1,1,1,1,1,1,1,1}
};

const int orig_map2[12][12] = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,5,0,0,1,0,0,0,5,0,1},
    {1,0,1,1,0,1,0,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,0,1,0,1},
    {1,5,0,0,1,0,0,1,5,1,0,1},
    {1,1,1,0,1,0,1,1,0,1,0,1},
    {1,0,0,0,1,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,5,1},
    {1,0,5,0,0,0,0,0,0,1,0,1},
    {1,1,1,1,1,1,1,1,0,1,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1}
};

const int orig_map3[15][15] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,5,0,0,0,0,0,0,5,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,1,5,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,1,0,1,0,5,0,0,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,1,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,2,1,0,1,0,1,5,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,0,0,1,0,1,0,1,0,1},
    {1,5,1,0,1,1,1,1,1,0,1,0,1,0,1},
    {1,0,1,0,0,5,0,0,0,0,1,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,5,0,0,0,0,1,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

const int orig_map4[10][10] = {
    {1,1,1,1,1,1,1,1,1,1},
    {1,0,0,5,0,0,1,3,0,1},
    {1,1,1,1,1,0,1,1,0,1},
    {1,5,0,0,1,0,0,0,0,1},
    {1,0,1,0,1,1,1,1,1,1},
    {1,0,1,0,0,0,5,0,4,1},
    {1,0,1,1,1,1,1,0,1,1},
    {1,0,5,0,0,0,1,0,2,1},
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

const int orig_map6[12][12] = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,6,0,0,0,1,5,0,0,1},
    {1,0,1,1,1,1,0,1,1,1,0,1},
    {1,0,1,5,0,1,0,6,0,1,0,1},
    {1,0,1,0,0,1,0,1,0,1,0,1},
    {1,0,1,1,4,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,1,0,1},
    {1,1,1,1,1,1,1,1,0,1,0,1},
    {1,3,6,0,5,0,0,1,0,1,0,1},
    {1,0,1,1,1,1,0,1,6,1,0,1},
    {1,0,0,0,0,1,0,0,0,1,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1}
};

const int orig_map7[15][15] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,1,0,0,0,1,0,0,0,1,0,0,8,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,1,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,1,1,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,5,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,1,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,3,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,1,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,1,1,0,1},
    {1,0,0,0,1,0,0,0,1,0,0,0,4,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

const int orig_map8[12][12] = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,1,2,1,0,0,0,5,0,1},
    {1,1,0,1,4,1,0,1,1,1,1,1},
    {1,0,0,1,0,1,0,1,3,0,0,1},
    {1,0,1,1,7,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,1,5,0,0,1},
    {1,1,1,1,1,1,0,1,1,1,1,1},
    {1,5,0,0,0,1,0,1,0,0,0,1},
    {1,1,1,1,0,1,0,1,0,1,0,1},
    {1,8,0,1,0,7,0,7,0,1,0,1},
    {1,1,0,1,1,1,1,1,1,1,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1}
};

const int orig_map9[12][12] = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,6,0,0,1,2,1,0,3,1},
    {1,0,1,1,1,0,1,4,1,0,1,1},
    {1,0,1,8,1,0,1,0,1,0,0,1},
    {1,0,1,1,1,0,1,0,1,1,0,1},
    {1,0,0,0,6,0,7,0,6,0,0,1},
    {1,1,1,1,1,1,1,0,1,1,1,1},
    {1,5,0,0,0,0,1,0,1,5,0,1},
    {1,0,1,1,1,0,1,0,1,0,1,1},
    {1,0,1,6,1,0,1,0,7,0,0,1},
    {1,0,0,0,1,5,1,0,1,1,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1}
};

const int orig_map10[15][15] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,1,5,0,6,0,0,0,1,8,0,1},
    {1,0,1,0,1,1,1,1,1,1,0,1,1,0,1},
    {1,0,1,0,7,0,0,5,0,1,0,7,0,0,1},
    {1,0,1,1,1,1,1,1,0,1,1,1,1,1,1},
    {1,0,1,3,0,0,0,1,0,0,0,6,0,0,1},
    {1,0,1,1,1,1,0,1,1,1,1,1,1,0,1},
    {1,0,0,6,0,1,0,4,0,0,0,1,0,0,1},
    {1,1,1,1,0,1,1,1,1,1,0,1,0,1,1},
    {1,5,0,1,0,0,0,0,0,1,0,1,0,0,1},
    {1,0,1,1,1,1,1,1,0,1,0,1,1,0,1},
    {1,0,1,2,1,0,0,1,0,1,0,1,5,0,1},
    {1,0,1,4,1,1,0,1,0,1,0,1,1,1,1},
    {1,0,0,0,6,0,0,1,5,1,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

int map1[10][10];
int map2[12][12];
int map3[15][15];
int map4[10][10];
int map5[12][12];
int map6[12][12];
int map7[15][15];
int map8[12][12];
int map9[12][12];
int map10[15][15];

int mapRandom[45][45];
int isPathTile[45][45];
int curRandW = 15;
int curRandH = 15;

int currentLevel = 0;
int keysHeld = 0;
int hasCompass = 0;
int speedBoost = 0;
int hasPickaxe = 0;

// Power-ups & Active Items
int pathfinderCharges = 1;
int pathfinderTimer = 0;
int speedShoesCharges = 1;
int speedShoesTimer = 0;
int stunSprayCharges = 1;
int stunSprayTimer = 0;
int bossHP = 3;

int totalGames = 0;
int totalEscapes = 0;
int totalScore = 0;

int gameState = 0; // 0=start, 1=play, 2=win
DWORD startTime = 0;
DWORD endTime = 0;
float bestTime = 9999.9f;
int score = 0;

char msgText[64] = "";
int msgTimer = 0;

float pX = 1.5f, pY = 1.5f;
float dX = 1.0f, dY = 0.0f;
float planeX = 0.0f, planeY = 0.66f;

// 16x16 Textures buffer: 16 types, 256 DWORD colors (0x00RRGGBB)
DWORD textures[16][256];
DWORD animFrameCount = 0;

// Particles
typedef struct {
    float x, y;
    float vx, vy;
    int life, maxLife;
    COLORREF color;
} Particle;
Particle particles[64];
int particleCount = 0;

void AddParticles(float x, float y, COLORREF color, int count) {
    for (int i = 0; i < count; i++) {
        if (particleCount < 64) {
            float angle = (float)(rand() % 628) / 100.0f;
            float spd = 0.5f + (float)(rand() % 150) / 100.0f;
            particles[particleCount].x = x + ((rand() % 20) - 10);
            particles[particleCount].y = y + ((rand() % 20) - 10);
            particles[particleCount].vx = (float)cos(angle) * spd;
            particles[particleCount].vy = (float)sin(angle) * spd;
            particles[particleCount].life = 15 + rand() % 15;
            particles[particleCount].maxLife = 30;
            particles[particleCount].color = color;
            particleCount++;
        }
    }
}

void UpdateParticles() {
    int write = 0;
    for (int i = 0; i < particleCount; i++) {
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].life--;
        if (particles[i].life > 0) {
            particles[write++] = particles[i];
        }
    }
    particleCount = write;
}

// Procedural 16x16 Texture Generator
void InitTextures() {
    for (int t = 0; t < 16; t++) {
        for (int y = 0; y < 16; y++) {
            for (int x = 0; x < 16; x++) {
                DWORD col = 0;
                if (t == 1 || t == 7) { // Stone Wall
                    int isMortar = (y == 3 || y == 7 || y == 11 || y == 15);
                    if (!isMortar) {
                        int rowShift = ((y / 4) % 2) * 8;
                        if (((x + rowShift) % 8) == 7) isMortar = 1;
                    }
                    if (isMortar) col = 0x00333333;
                    else {
                        int noise = ((x * 13 + y * 37) % 30) - 15;
                        int r = 160 + noise; if (r < 0) r = 0; if (r > 255) r = 255;
                        int g = 40 + noise / 2; if (g < 0) g = 0;
                        int b = 40 + noise / 2; if (b < 0) b = 0;
                        col = RGB(r, g, b);
                    }
                } else if (t == 2) { // Exit Portal
                    float dist = (float)sqrt((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f));
                    if (dist < 3.0f) col = 0x00FFFFFF;
                    else if (dist < 5.5f) col = 0x0000FF66;
                    else if (dist < 7.5f) col = 0x00009933;
                    else col = 0x00003311;
                } else if (t == 3) { // Key Block
                    int isKey = 0;
                    if ((x >= 6 && x <= 9 && y >= 2 && y <= 5) || (x == 7 && y >= 6 && y <= 12) || (x >= 8 && x <= 10 && y >= 10 && y <= 12)) isKey = 1;
                    if (isKey) col = 0x00FFFF00;
                    else col = 0x00B8860B;
                } else if (t == 4) { // Steel Door
                    if (x == 0 || x == 15 || y == 0 || y == 15) col = 0x00112233;
                    else if (y == 4 || y == 11) col = 0x00778899;
                    else if (x == 7 || x == 8) col = (y >= 7 && y <= 9) ? 0x00000000 : 0x00115599;
                    else col = 0x00004488;
                } else if (t == 5) { // Coin Chest
                    float dist = (float)sqrt((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f));
                    if (dist < 4.5f) col = (dist < 2.0f) ? 0x00FFFFFF : 0x00FFCC00;
                    else col = 0x008B4513;
                } else if (t == 6) { // Trap (Lava/Spike)
                    if (y >= 12 && (x % 4 == 1 || x % 4 == 2)) col = 0x00CCCCCC;
                    else if ((x + y) % 6 < 2) col = 0x000044FF;
                    else col = 0x00001188;
                } else if (t == 8) { // Compass Block
                    if (x == 7 || y == 7 || abs(x - 7) + abs(y - 7) <= 4) col = 0x0000FFFF;
                    else col = 0x00004455;
                } else if (t == 9) { // Speed Boost
                    if ((x >= 6 && x <= 10 && y >= 2 && y <= 6) || (x >= 4 && x <= 8 && y >= 7 && y <= 13)) col = 0x00FFFF00;
                    else col = 0x00708090;
                } else if (t == 10 || t == 11) { // Teleporter Vortex
                    float dist = (float)sqrt((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f));
                    if (dist < 6.0f && ((int)(dist * 2.0f) % 2 == 0)) col = 0x00FF00FF;
                    else col = 0x00300044;
                } else if (t == 12) { // Minotaur Monster
                    if ((x >= 2 && x <= 5 && y <= 4) || (x >= 10 && x <= 13 && y <= 4)) col = 0x00333333;
                    else if ((x >= 4 && x <= 6 && y >= 6 && y <= 7) || (x >= 9 && x <= 11 && y >= 6 && y <= 7)) col = 0x00FFFF00;
                    else if (y >= 10 && y <= 12 && x >= 5 && x <= 10) col = 0x00FFFFFF;
                    else col = 0x00990000;
                } else if (t == 13) { // Pickaxe Block
                    if ((x + y == 15 || x + y == 14) && (x >= 3 && x <= 12)) col = 0x008899AA;
                    else if (x == y && x >= 4 && x <= 11) col = 0x008B4513;
                    else col = 0x005C3A1E;
                } else if (t == 14) { // Stun Spray
                    if (x >= 5 && x <= 10 && y >= 4 && y <= 14) col = 0x00FFCC00;
                    else if (x >= 6 && x <= 9 && y >= 1 && y <= 3) col = 0x0000FFFF;
                    else col = 0x00331100;
                } else if (t == 15) { // Minotaur King Boss
                    if (y <= 3 && x >= 4 && x <= 11) col = 0x0000D7FF;
                    else if ((x >= 1 && x <= 4 && y <= 5) || (x >= 11 && x <= 14 && y <= 5)) col = 0x00EEEEEE;
                    else if ((x >= 4 && x <= 6 && y >= 6 && y <= 7) || (x >= 9 && x <= 11 && y >= 6 && y <= 7)) col = 0x000000FF;
                    else if (y >= 10 && y <= 13 && x >= 4 && x <= 11) col = 0x00FFFFFF;
                    else col = 0x00000099;
                } else {
                    col = 0x00AA0000;
                }
                textures[t][y * 16 + x] = col;
            }
        }
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
    } else if (currentLevel == 4) {
        if (x >= 12 || y >= 12) return 1;
        return map5[x][y];
    } else if (currentLevel == 5) {
        if (x >= 12 || y >= 12) return 1;
        return map6[x][y];
    } else if (currentLevel == 6) {
        if (x >= 15 || y >= 15) return 1;
        return map7[x][y];
    } else if (currentLevel == 7) {
        if (x >= 12 || y >= 12) return 1;
        return map8[x][y];
    } else if (currentLevel == 8) {
        if (x >= 12 || y >= 12) return 1;
        return map9[x][y];
    } else if (currentLevel == 9) {
        if (x >= 15 || y >= 15) return 1;
        return map10[x][y];
    } else {
        if (x >= curRandW || y >= curRandH) return 1;
        return mapRandom[x][y];
    }
}

void SetMapValue(int x, int y, int v) {
    if (x < 0 || y < 0) return;
    if (currentLevel == 0 && x < 10 && y < 10) map1[x][y] = v;
    else if (currentLevel == 1 && x < 12 && y < 12) map2[x][y] = v;
    else if (currentLevel == 2 && x < 15 && y < 15) map3[x][y] = v;
    else if (currentLevel == 3 && x < 10 && y < 10) map4[x][y] = v;
    else if (currentLevel == 4 && x < 12 && y < 12) map5[x][y] = v;
    else if (currentLevel == 5 && x < 12 && y < 12) map6[x][y] = v;
    else if (currentLevel == 6 && x < 15 && y < 15) map7[x][y] = v;
    else if (currentLevel == 7 && x < 12 && y < 12) map8[x][y] = v;
    else if (currentLevel == 8 && x < 12 && y < 12) map9[x][y] = v;
    else if (currentLevel == 9 && x < 15 && y < 15) map10[x][y] = v;
    else if (currentLevel >= 10 && x < curRandW && y < curRandH) mapRandom[x][y] = v;
}

int TryMove(int x, int y) {
    int val = GetMapValue(x, y);
    if (val == 0 || val == 2 || val == 3 || val == 5 || val == 6 || val == 7 || val == 8 || val == 9 || val == 10 || val == 11 || val == 12 || val == 13 || val == 14 || val == 15) return 1;
    if (val == 4) {
        if (keysHeld > 0) {
            keysHeld--;
            SetMapValue(x, y, 0);
            MessageBeep(MB_ICONEXCLAMATION);
            AddParticles(160.0f, 120.0f, RGB(0, 150, 255), 20);
            return 1;
        }
    }
    if (val == 1) {
        if (hasPickaxe > 0 && x > 0 && y > 0) {
            int mapW = currentLevel >= 10 ? curRandW : 15;
            int mapH = currentLevel >= 10 ? curRandH : 15;
            if (currentLevel == 0 || currentLevel == 3) { mapW = 10; mapH = 10; }
            else if (currentLevel == 1 || currentLevel == 4 || currentLevel == 5 || currentLevel == 7 || currentLevel == 8) { mapW = 12; mapH = 12; }
            if (x < mapW - 1 && y < mapH - 1) {
                hasPickaxe--;
                SetMapValue(x, y, 0);
                MessageBeep(MB_ICONEXCLAMATION);
                AddParticles(160.0f, 120.0f, RGB(180, 100, 50), 25);
                return 1;
            }
        }
    }
    return 0;
}

void ComputePathfinderPath() {
    memset(isPathTile, 0, sizeof(isPathTile));
    int mapW = currentLevel >= 10 ? curRandW : 15;
    int mapH = currentLevel >= 10 ? curRandH : 15;
    if (currentLevel == 0 || currentLevel == 3) { mapW = 10; mapH = 10; }
    else if (currentLevel == 1 || currentLevel == 4 || currentLevel == 5 || currentLevel == 7 || currentLevel == 8) { mapW = 12; mapH = 12; }

    int targetX = -1, targetY = -1;
    int needKey = 0;
    for (int x = 0; x < mapW; x++) {
        for (int y = 0; y < mapH; y++) {
            if (GetMapValue(x, y) == 4) needKey = 1;
        }
    }
    if (needKey && keysHeld == 0) {
        for (int x = 0; x < mapW; x++) {
            for (int y = 0; y < mapH; y++) {
                if (GetMapValue(x, y) == 3) { targetX = x; targetY = y; break; }
            }
        }
    }
    if (targetX == -1) {
        for (int x = 0; x < mapW; x++) {
            for (int y = 0; y < mapH; y++) {
                if (GetMapValue(x, y) == 2) { targetX = x; targetY = y; break; }
            }
        }
    }
    if (targetX == -1) return;

    int qX[2500], qY[2500];
    int qHead = 0, qTail = 0;
    int parentX[45][45], parentY[45][45];
    memset(parentX, -1, sizeof(parentX));
    memset(parentY, -1, sizeof(parentY));

    int startX = (int)pX, startY = (int)pY;
    if (startX < 0 || startX >= mapW || startY < 0 || startY >= mapH) return;

    qX[qTail] = startX; qY[qTail] = startY; qTail++;
    parentX[startX][startY] = startX; parentY[startX][startY] = startY;

    int dirs[4][2] = {{0,1},{0,-1},{1,0},{-1,0}};
    int found = 0;

    while (qHead < qTail) {
        int cx = qX[qHead];
        int cy = qY[qHead];
        qHead++;

        if (cx == targetX && cy == targetY) { found = 1; break; }

        for (int d = 0; d < 4; d++) {
            int nx = cx + dirs[d][0];
            int ny = cy + dirs[d][1];
            if (nx >= 0 && nx < mapW && ny >= 0 && ny < mapH && parentX[nx][ny] == -1) {
                int tile = GetMapValue(nx, ny);
                if (tile == 0 || tile == 2 || tile == 3 || tile == 5 || tile == 6 || tile == 8 || tile == 9 || tile == 10 || tile == 11 || tile == 13 || tile == 14 || (tile == 4 && keysHeld > 0)) {
                    parentX[nx][ny] = cx;
                    parentY[nx][ny] = cy;
                    qX[qTail] = nx; qY[qTail] = ny; qTail++;
                }
            }
        }
    }

    if (found) {
        int currX = targetX, currY = targetY;
        while (currX != startX || currY != startY) {
            isPathTile[currX][currY] = 1;
            int px = parentX[currX][currY];
            int py = parentY[currX][currY];
            if (px == -1 || py == -1 || (px == currX && py == currY)) break;
            currX = px; currY = py;
        }
        isPathTile[startX][startY] = 1;
    }
}

void GenerateMaze(int w, int h) {
    for (int i=0; i<w; i++) for (int j=0; j<h; j++) mapRandom[i][j] = 1;
    int stackX[2500], stackY[2500];
    int stackPtr = 0;
    
    int cx = 1, cy = 1;
    mapRandom[cx][cy] = 0;
    stackX[stackPtr] = cx; stackY[stackPtr] = cy; stackPtr++;
    
    int farX = cx, farY = cy;
    
    while(stackPtr > 0) {
        cx = stackX[stackPtr-1];
        cy = stackY[stackPtr-1];
        stackPtr--;
        
        int dirs[4][2] = {{0,-2}, {0,2}, {-2,0}, {2,0}};
        for(int i=0; i<4; i++) {
            int r = rand() % 4;
            int tx = dirs[i][0]; int ty = dirs[i][1];
            dirs[i][0] = dirs[r][0]; dirs[i][1] = dirs[r][1];
            dirs[r][0] = tx; dirs[r][1] = ty;
        }
        
        for(int i=0; i<4; i++) {
            int nx = cx + dirs[i][0];
            int ny = cy + dirs[i][1];
            if (nx > 0 && nx < w-1 && ny > 0 && ny < h-1 && mapRandom[nx][ny] == 1) {
                mapRandom[cx + dirs[i][0]/2][cy + dirs[i][1]/2] = 0;
                mapRandom[nx][ny] = 0;
                
                stackX[stackPtr] = cx; stackY[stackPtr] = cy; stackPtr++;
                stackX[stackPtr] = nx; stackY[stackPtr] = ny; stackPtr++;
                
                farX = nx; farY = ny;
                break;
            }
        }
    }
    
    for(int i=0; i<w*h/10; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) {
            mapRandom[rx][ry] = 5;
        }
    }
    int placedCompass = 0;
    while(!placedCompass) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0) {
            mapRandom[rx][ry] = 8;
            placedCompass = 1;
        }
    }
    for(int i=0; i<w*h/20; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0) {
            mapRandom[rx][ry] = 6;
        }
    }
    for(int i=0; i<w*h/20; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 1 && rx > 1 && rx < w-2 && ry > 1 && ry < h-2) {
            mapRandom[rx][ry] = 7;
        }
    }
    for(int i=0; i<w*h/30; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) {
            mapRandom[rx][ry] = 9;
        }
    }
    if (currentLevel >= 5) {
        int placedMino = 0;
        while (!placedMino) {
            int rx = 1 + rand()%(w-2);
            int ry = 1 + rand()%(h-2);
            if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY) && abs(rx - 1) + abs(ry - 1) > 5) {
                mapRandom[rx][ry] = 12;
                placedMino = 1;
            }
        }
    }
    for(int i=0; i<w*h/40; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) {
            mapRandom[rx][ry] = 13;
        }
    }
    for(int i=0; i<w*h/40; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) {
            mapRandom[rx][ry] = 14;
        }
    }
    int t1x = 0, t1y = 0, t2x = 0, t2y = 0;
    for(int i=0; i<100; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) { t1x = rx; t1y = ry; break; }
    }
    for(int i=0; i<100; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != t1x || ry != t1y) && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) { t2x = rx; t2y = ry; break; }
    }
    if (t1x && t2x) {
        mapRandom[t1x][t1y] = 10;
        mapRandom[t2x][t2y] = 11;
    }
    
    // Stage 35 (level 34) Boss Arena Chamber
    if (currentLevel == 34) {
        bossHP = 3;
        for (int i = 15; i <= 25; i++) {
            for (int j = 15; j <= 25; j++) {
                mapRandom[i][j] = 0;
            }
        }
        mapRandom[20][20] = 15; // Minotaur King Boss
        mapRandom[17][17] = 12; // Guard Minotaur 1
        mapRandom[23][23] = 12; // Guard Minotaur 2
        mapRandom[25][25] = 3;  // Boss Key
        mapRandom[farX][farY] = 2;
        if (farX > 1 && farY > 1) mapRandom[farX - 1][farY] = 4;
    } else {
        mapRandom[farX][farY] = 2;
    }

    curRandW = w;
    curRandH = h;
}

void ResetMaps() {
    memcpy(map1, orig_map1, sizeof(map1));
    memcpy(map2, orig_map2, sizeof(map2));
    memcpy(map3, orig_map3, sizeof(map3));
    memcpy(map4, orig_map4, sizeof(map4));
    memcpy(map5, orig_map5, sizeof(map5));
    memcpy(map6, orig_map6, sizeof(map6));
    memcpy(map7, orig_map7, sizeof(map7));
    memcpy(map8, orig_map8, sizeof(map8));
    memcpy(map9, orig_map9, sizeof(map9));
    memcpy(map10, orig_map10, sizeof(map10));
}

void LoadBest() {
    HANDLE hFile = CreateFileA("kmaze_score.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD readBytes = 0;
        ReadFile(hFile, &bestTime, sizeof(float), &readBytes, NULL);
        ReadFile(hFile, &totalGames, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &totalEscapes, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &totalScore, sizeof(int), &readBytes, NULL);
        CloseHandle(hFile);
    }
}

void SaveBest() {
    HANDLE hFile = CreateFileA("kmaze_score.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hFile, &bestTime, sizeof(float), &written, NULL);
        WriteFile(hFile, &totalGames, sizeof(int), &written, NULL);
        WriteFile(hFile, &totalEscapes, sizeof(int), &written, NULL);
        WriteFile(hFile, &totalScore, sizeof(int), &written, NULL);
        CloseHandle(hFile);
    }
}

void InitGame() {
    srand((unsigned int)GetTickCount());
    LoadBest();
    ResetMaps();
    InitTextures();
}

void NextLevel() {
    keysHeld = 0;
    speedBoost = 0;
    hasPickaxe = 1;
    pathfinderCharges = 1;
    speedShoesCharges = 1;
    stunSprayCharges = 1;
    pathfinderTimer = 0;
    speedShoesTimer = 0;
    stunSprayTimer = 0;

    currentLevel++;
    hasCompass = (currentLevel < 6) ? 1 : 0;
    if (currentLevel > 34) {
        gameState = 2;
        endTime = GetTickCount();
        float elapsed = (endTime - startTime) / 1000.0f;
        if (elapsed < bestTime) {
            bestTime = elapsed;
        }
        totalEscapes++;
        totalScore += score;
        SaveBest();
        return;
    }
    
    if (currentLevel >= 10) {
        int s = 11 + 2 * (int)(((currentLevel - 10) * 15) / 24);
        if (s > 41) s = 41;
        if (s % 2 == 0) s++;
        GenerateMaze(s, s);
    }
    
    pX = 1.5f; pY = 1.5f;
    dX = 1.0f; dY = 0.0f;
    planeX = 0.0f; planeY = 0.66f;
}

HBITMAP hbmCanvas = NULL;
HDC hdcMem = NULL;
DWORD* pBits = NULL;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            InitGame();
            SetTimer(hwnd, 1, 30, NULL);
            break;
        case WM_TIMER: {
            animFrameCount++;
            UpdateParticles();
            static int minotaurTimer = 0;
            static int activeKeyCooldown = 0;
            if (activeKeyCooldown > 0) activeKeyCooldown -= 30;

            if (pathfinderTimer > 0) {
                pathfinderTimer -= 30;
                ComputePathfinderPath();
            }
            if (speedShoesTimer > 0) speedShoesTimer -= 30;
            if (stunSprayTimer > 0) stunSprayTimer -= 30;

            float moveSpeed = 0.1f;
            float rotSpeed = 0.05f;
            if (speedShoesTimer > 0 || speedBoost) moveSpeed *= 2.0f;
            
            if (gameState == 1) {
                minotaurTimer += 30;
                if (minotaurTimer >= 1000) {
                    minotaurTimer = 0;
                    if (stunSprayTimer <= 0) {
                        int mapW = currentLevel >= 10 ? curRandW : 15;
                        int mapH = currentLevel >= 10 ? curRandH : 15;
                        if (currentLevel == 0 || currentLevel == 3) { mapW = 10; mapH = 10; }
                        else if (currentLevel == 1 || currentLevel == 4 || currentLevel == 5 || currentLevel == 7 || currentLevel == 8) { mapW = 12; mapH = 12; }
                        
                        int minotaurs[100][3];
                        int mCount = 0;
                        for (int x = 0; x < mapW; x++) {
                            for (int y = 0; y < mapH; y++) {
                                int val = GetMapValue(x, y);
                                if (val == 12 || val == 15) {
                                    minotaurs[mCount][0] = x;
                                    minotaurs[mCount][1] = y;
                                    minotaurs[mCount][2] = val;
                                    mCount++;
                                }
                            }
                        }
                        for (int i = 0; i < mCount; i++) {
                            int mx = minotaurs[i][0];
                            int my = minotaurs[i][1];
                            int mtype = minotaurs[i][2];
                            int mdx = 0, mdy = 0;
                            if ((int)pX > mx) mdx = 1;
                            else if ((int)pX < mx) mdx = -1;
                            if ((int)pY > my) mdy = 1;
                            else if ((int)pY < my) mdy = -1;
                            
                            if (mdx != 0 && GetMapValue(mx + mdx, my) == 0) {
                                SetMapValue(mx, my, 0);
                                SetMapValue(mx + mdx, my, mtype);
                                mx += mdx;
                            } else if (mdy != 0 && GetMapValue(mx, my + mdy) == 0) {
                                SetMapValue(mx, my, 0);
                                SetMapValue(mx, my + mdy, mtype);
                                my += mdy;
                            }
                            
                            if (mx == (int)pX && my == (int)pY) {
                                MessageBeep(MB_ICONHAND);
                                score = (score >= 100) ? score - 100 : 0;
                                if (mtype == 15) {
                                    pX = 1.5f; pY = 1.5f;
                                    strcpy(msgText, "Trampled by Minotaur King!");
                                    msgTimer = 60;
                                } else {
                                    currentLevel--;
                                    NextLevel();
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            
            if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
                if (gameState == 0 || gameState == 2) {
                    gameState = 1;
                    startTime = GetTickCount();
                    currentLevel = -1;
                    score = 0;
                    totalGames++;
                    SaveBest();
                    ResetMaps();
                    NextLevel();
                }
            }
            
            if (gameState == 1 && activeKeyCooldown <= 0) {
                if (GetAsyncKeyState('P') & 0x8000) {
                    if (hasPickaxe > 0) {
                        int tx = (int)(pX + dX * 0.8f);
                        int ty = (int)(pY + dY * 0.8f);
                        int tVal = GetMapValue(tx, ty);
                        if (tVal == 1 || tVal == 7) {
                            hasPickaxe--;
                            SetMapValue(tx, ty, 0);
                            MessageBeep(MB_OK);
                            AddParticles(160.0f, 120.0f, RGB(180, 100, 50), 25);
                            strcpy(msgText, "Wall Broken!");
                            msgTimer = 60;
                            activeKeyCooldown = 300;
                        } else if (tVal == 12) {
                            hasPickaxe--;
                            SetMapValue(tx, ty, 0);
                            score += 200;
                            MessageBeep(MB_OK);
                            AddParticles(160.0f, 120.0f, RGB(255, 0, 0), 25);
                            strcpy(msgText, "Minotaur Slain!");
                            msgTimer = 60;
                            activeKeyCooldown = 300;
                        } else if (tVal == 15) {
                            hasPickaxe--;
                            bossHP--;
                            AddParticles(160.0f, 120.0f, RGB(255, 215, 0), 30);
                            if (bossHP <= 0) {
                                SetMapValue(tx, ty, 3);
                                score += 1000;
                                strcpy(msgText, "Minotaur King Slain! Key Dropped!");
                            } else {
                                strcpy(msgText, "Minotaur King Hit!");
                            }
                            msgTimer = 60;
                            activeKeyCooldown = 300;
                        }
                    }
                }
                if (GetAsyncKeyState('C') & 0x8000) {
                    if (pathfinderCharges > 0 || hasCompass) {
                        if (pathfinderCharges > 0) pathfinderCharges--;
                        pathfinderTimer = 10000;
                        ComputePathfinderPath();
                        MessageBeep(MB_ICONASTERISK);
                        AddParticles(160.0f, 120.0f, RGB(0, 255, 255), 25);
                        strcpy(msgText, "Pathfinder Active (10s)!");
                        msgTimer = 60;
                        activeKeyCooldown = 300;
                    }
                }
                if (GetAsyncKeyState('S') & 0x8000) {
                    if (speedShoesCharges > 0 || speedBoost) {
                        if (speedShoesCharges > 0) speedShoesCharges--;
                        speedShoesTimer = 8000;
                        MessageBeep(MB_ICONASTERISK);
                        AddParticles(160.0f, 120.0f, RGB(255, 255, 0), 25);
                        strcpy(msgText, "Speed Shoes Active (8s)!");
                        msgTimer = 60;
                        activeKeyCooldown = 300;
                    }
                }
                if (GetAsyncKeyState('F') & 0x8000) {
                    if (stunSprayCharges > 0) {
                        stunSprayCharges--;
                        stunSprayTimer = 10000;
                        MessageBeep(MB_ICONASTERISK);
                        AddParticles(160.0f, 120.0f, RGB(100, 200, 255), 30);
                        strcpy(msgText, "Minotaur Stun Spray Active (10s)!");
                        msgTimer = 60;
                        activeKeyCooldown = 300;
                    }
                }
                if (GetAsyncKeyState('V') & 0x8000) {
                    HANDLE hSave = CreateFileA("kmaze_save.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hSave != INVALID_HANDLE_VALUE) {
                        DWORD written = 0;
                        WriteFile(hSave, &currentLevel, sizeof(int), &written, NULL);
                        WriteFile(hSave, &score, sizeof(int), &written, NULL);
                        WriteFile(hSave, &keysHeld, sizeof(int), &written, NULL);
                        WriteFile(hSave, &hasCompass, sizeof(int), &written, NULL);
                        WriteFile(hSave, &speedBoost, sizeof(int), &written, NULL);
                        WriteFile(hSave, &hasPickaxe, sizeof(int), &written, NULL);
                        WriteFile(hSave, &pathfinderCharges, sizeof(int), &written, NULL);
                        WriteFile(hSave, &speedShoesCharges, sizeof(int), &written, NULL);
                        WriteFile(hSave, &stunSprayCharges, sizeof(int), &written, NULL);
                        WriteFile(hSave, &pX, sizeof(float), &written, NULL);
                        WriteFile(hSave, &pY, sizeof(float), &written, NULL);
                        WriteFile(hSave, &dX, sizeof(float), &written, NULL);
                        WriteFile(hSave, &dY, sizeof(float), &written, NULL);
                        WriteFile(hSave, &planeX, sizeof(float), &written, NULL);
                        WriteFile(hSave, &planeY, sizeof(float), &written, NULL);
                        DWORD elapsed = GetTickCount() - startTime;
                        WriteFile(hSave, &elapsed, sizeof(DWORD), &written, NULL);
                        WriteFile(hSave, &curRandW, sizeof(int), &written, NULL);
                        WriteFile(hSave, &curRandH, sizeof(int), &written, NULL);
                        WriteFile(hSave, mapRandom, sizeof(mapRandom), &written, NULL);
                        CloseHandle(hSave);
                        strcpy(msgText, "Game Saved!");
                        msgTimer = 60;
                        activeKeyCooldown = 1000;
                        MessageBeep(MB_OK);
                    }
                }
                if (GetAsyncKeyState('L') & 0x8000) {
                    HANDLE hLoad = CreateFileA("kmaze_save.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hLoad != INVALID_HANDLE_VALUE) {
                        DWORD readBytes = 0;
                        ReadFile(hLoad, &currentLevel, sizeof(int), &readBytes, NULL);
                        ReadFile(hLoad, &score, sizeof(int), &readBytes, NULL);
                        ReadFile(hLoad, &keysHeld, sizeof(int), &readBytes, NULL);
                        ReadFile(hLoad, &hasCompass, sizeof(int), &readBytes, NULL);
                        ReadFile(hLoad, &speedBoost, sizeof(int), &readBytes, NULL);
                        ReadFile(hLoad, &hasPickaxe, sizeof(int), &readBytes, NULL);
                        ReadFile(hLoad, &pathfinderCharges, sizeof(int), &readBytes, NULL);
                        ReadFile(hLoad, &speedShoesCharges, sizeof(int), &readBytes, NULL);
                        ReadFile(hLoad, &stunSprayCharges, sizeof(int), &readBytes, NULL);
                        ReadFile(hLoad, &pX, sizeof(float), &readBytes, NULL);
                        ReadFile(hLoad, &pY, sizeof(float), &readBytes, NULL);
                        ReadFile(hLoad, &dX, sizeof(float), &readBytes, NULL);
                        ReadFile(hLoad, &dY, sizeof(float), &readBytes, NULL);
                        ReadFile(hLoad, &planeX, sizeof(float), &readBytes, NULL);
                        ReadFile(hLoad, &planeY, sizeof(float), &readBytes, NULL);
                        DWORD elapsed = 0;
                        ReadFile(hLoad, &elapsed, sizeof(DWORD), &readBytes, NULL);
                        startTime = GetTickCount() - elapsed;
                        ReadFile(hLoad, &curRandW, sizeof(int), &readBytes, NULL);
                        ReadFile(hLoad, &curRandH, sizeof(int), &readBytes, NULL);
                        ReadFile(hLoad, mapRandom, sizeof(mapRandom), &readBytes, NULL);
                        CloseHandle(hLoad);
                        strcpy(msgText, "Game Loaded!");
                        msgTimer = 60;
                        activeKeyCooldown = 1000;
                        MessageBeep(MB_OK);
                    }
                }
            }
            if (gameState != 1) {
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }

            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                moveSpeed *= 1.5f;
            }
            if (GetAsyncKeyState(VK_UP) & 0x8000 || GetAsyncKeyState('W') & 0x8000) {
                if (TryMove((int)(pX + dX * moveSpeed), (int)pY)) pX += dX * moveSpeed;
                if (TryMove((int)pX, (int)(pY + dY * moveSpeed))) pY += dY * moveSpeed;
            }
            if (GetAsyncKeyState(VK_DOWN) & 0x8000 || GetAsyncKeyState('S') & 0x8000) {
                if (TryMove((int)(pX - dX * moveSpeed), (int)pY)) pX -= dX * moveSpeed;
                if (TryMove((int)pX, (int)(pY - dY * moveSpeed))) pY -= dY * moveSpeed;
            }
            
            int curVal = GetMapValue((int)pX, (int)pY);
            if (curVal == 3) {
                keysHeld++;
                SetMapValue((int)pX, (int)pY, 0);
                MessageBeep(MB_OK);
                AddParticles(160.0f, 120.0f, RGB(255, 255, 0), 15);
            } else if (curVal == 2) {
                MessageBeep(MB_ICONASTERISK);
                AddParticles(160.0f, 120.0f, RGB(0, 255, 0), 30);
                NextLevel();
            } else if (curVal == 5) {
                score += 100;
                SetMapValue((int)pX, (int)pY, 0);
                MessageBeep(MB_OK);
                AddParticles(160.0f, 120.0f, RGB(255, 200, 0), 15);
            } else if (curVal == 6) {
                MessageBeep(MB_ICONHAND);
                score = (score >= 50) ? score - 50 : 0;
                pX = 1.5f; pY = 1.5f;
                AddParticles(160.0f, 120.0f, RGB(255, 50, 0), 20);
                strcpy(msgText, "Burnt by Lava Trap!"); msgTimer = 60;
            } else if (curVal == 7) {
                SetMapValue((int)pX, (int)pY, 0);
                MessageBeep(MB_OK);
                AddParticles(160.0f, 120.0f, RGB(150, 150, 150), 10);
            } else if (curVal == 8) {
                pathfinderCharges++;
                hasCompass = 1;
                SetMapValue((int)pX, (int)pY, 0);
                MessageBeep(MB_ICONASTERISK);
                AddParticles(160.0f, 120.0f, RGB(0, 255, 255), 15);
                strcpy(msgText, "+1 Pathfinder!"); msgTimer = 60;
            } else if (curVal == 9) {
                speedShoesCharges++;
                speedBoost = 1;
                score += 50;
                SetMapValue((int)pX, (int)pY, 0);
                MessageBeep(MB_ICONASTERISK);
                AddParticles(160.0f, 120.0f, RGB(255, 255, 0), 15);
                strcpy(msgText, "+1 Speed Shoes!"); msgTimer = 60;
            } else if (curVal == 10) {
                for(int i=0; i<45; i++) {
                    for(int j=0; j<45; j++) {
                        if (GetMapValue(i, j) == 11) {
                            pX = i + 0.5f; pY = j + 0.5f;
                            MessageBeep(MB_ICONHAND);
                            AddParticles(160.0f, 120.0f, RGB(255, 0, 255), 25);
                            i=45; break;
                        }
                    }
                }
            } else if (curVal == 11) {
                for(int i=0; i<45; i++) {
                    for(int j=0; j<45; j++) {
                        if (GetMapValue(i, j) == 10) {
                            pX = i + 0.5f; pY = j + 0.5f;
                            MessageBeep(MB_ICONHAND);
                            AddParticles(160.0f, 120.0f, RGB(255, 0, 255), 25);
                            i=45; break;
                        }
                    }
                }
            } else if (curVal == 12) {
                MessageBeep(MB_ICONHAND);
                score = (score >= 100) ? score - 100 : 0;
                currentLevel--;
                NextLevel();
            } else if (curVal == 13) {
                hasPickaxe++;
                SetMapValue((int)pX, (int)pY, 0);
                MessageBeep(MB_ICONASTERISK);
                AddParticles(160.0f, 120.0f, RGB(180, 100, 50), 15);
                strcpy(msgText, "+1 Pickaxe!"); msgTimer = 60;
            } else if (curVal == 14) {
                stunSprayCharges++;
                SetMapValue((int)pX, (int)pY, 0);
                MessageBeep(MB_ICONASTERISK);
                AddParticles(160.0f, 120.0f, RGB(100, 200, 255), 15);
                strcpy(msgText, "+1 Stun Spray!"); msgTimer = 60;
            } else if (curVal == 15) {
                MessageBeep(MB_ICONHAND);
                score = (score >= 150) ? score - 150 : 0;
                pX = 1.5f; pY = 1.5f;
                strcpy(msgText, "Attacked by Minotaur King!"); msgTimer = 60;
            }

            if (GetAsyncKeyState(VK_RIGHT) & 0x8000 || GetAsyncKeyState('D') & 0x8000) {
                float oldDX = dX;
                dX = dX * (float)cos(rotSpeed) - dY * (float)sin(rotSpeed);
                dY = oldDX * (float)sin(rotSpeed) + dY * (float)cos(rotSpeed);
                float oldPlaneX = planeX;
                planeX = planeX * (float)cos(rotSpeed) - planeY * (float)sin(rotSpeed);
                planeY = oldPlaneX * (float)sin(rotSpeed) + planeY * (float)cos(rotSpeed);
            }
            if (GetAsyncKeyState(VK_LEFT) & 0x8000 || GetAsyncKeyState('A') & 0x8000) {
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
                BITMAPINFO bmi = {0};
                bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bmi.bmiHeader.biWidth = W;
                bmi.bmiHeader.biHeight = -H; // top-down
                bmi.bmiHeader.biPlanes = 1;
                bmi.bmiHeader.biBitCount = 32;
                bmi.bmiHeader.biCompression = BI_RGB;
                hbmCanvas = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
                SelectObject(hdcMem, hbmCanvas);
            }
            
            // Software Raycasting into DIBSection pBits
            if (pBits) {
                for (int y = 0; y < H / 2; y++) {
                    float ceilFog = (float)y / (H / 2);
                    BYTE c = (BYTE)(20 + ceilFog * 30);
                    DWORD ceilCol = RGB(c, c, (BYTE)(c * 1.2f));
                    for (int x = 0; x < W; x++) pBits[y * W + x] = ceilCol;
                }
                for (int y = H / 2; y < H; y++) {
                    float floorFog = 1.0f - (float)(y - H / 2) / (H / 2);
                    BYTE f = (BYTE)(20 + (1.0f - floorFog) * 40);
                    DWORD floorCol = RGB(f, (BYTE)(f * 1.1f), f);
                    for (int x = 0; x < W; x++) pBits[y * W + x] = floorCol;
                }

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
                    int drawEnd = lineHeight / 2 + H / 2;
                    int actualStart = (drawStart < 0) ? 0 : drawStart;
                    int actualEnd = (drawEnd >= H) ? H - 1 : drawEnd;
                    
                    float wallX;
                    if (side == 0) wallX = pY + perpWallDist * rayDY;
                    else           wallX = pX + perpWallDist * rayDX;
                    wallX -= (float)floor(wallX);
                    int texX = (int)(wallX * 16.0f) & 15;
                    
                    if (hit == 7) hit = 1;
                    if (hit < 1 || hit > 15) hit = 1;
                    
                    float step = 16.0f / lineHeight;
                    float texPos = (actualStart - H / 2 + lineHeight / 2) * step;
                    
                    float maxDist = (currentLevel >= 20) ? 4.5f : ((currentLevel >= 10) ? 7.0f : 12.0f);
                    if (pathfinderTimer > 0) maxDist = 14.0f;
                    float fog = 1.0f - perpWallDist / maxDist;
                    if (fog < 0.1f) fog = 0.1f; if (fog > 1.0f) fog = 1.0f;
                    if (currentLevel >= 15 && perpWallDist > maxDist && pathfinderTimer <= 0) fog = 0.0f;
                    
                    float sideMult = (side == 1) ? 0.7f : 1.0f;
                    
                    for (int y = actualStart; y <= actualEnd; y++) {
                        int texY = (int)texPos & 15;
                        texPos += step;
                        
                        DWORD srcCol = textures[hit][texY * 16 + texX];
                        BYTE r = (BYTE)((srcCol & 0xFF) * sideMult * fog);
                        BYTE g = (BYTE)(((srcCol >> 8) & 0xFF) * sideMult * fog);
                        BYTE b = (BYTE)(((srcCol >> 16) & 0xFF) * sideMult * fog);
                        
                        pBits[y * W + x] = RGB(r, g, b);
                    }
                }
            }

            // Draw HUD equipment & GDI overlay elements on hdcMem
            for (int i = 0; i < particleCount; i++) {
                int px = (int)particles[i].x;
                int py = (int)particles[i].y;
                if (px >= 0 && px < W && py >= 0 && py < H) {
                    HBRUSH pb = CreateSolidBrush(particles[i].color);
                    RECT pr = {px, py, px + 2, py + 2};
                    FillRect(hdcMem, &pr, pb);
                    DeleteObject(pb);
                }
            }

            // Held Equipment HUD
            if (gameState == 1) {
                if (hasCompass || pathfinderTimer > 0) {
                    HBRUSH brassB = CreateSolidBrush(RGB(200, 150, 50));
                    HBRUSH faceB = CreateSolidBrush(RGB(15, 30, 45));
                    HPEN goldP = CreatePen(PS_SOLID, 1, RGB(255, 215, 0));
                    SelectObject(hdcMem, brassB); SelectObject(hdcMem, goldP);
                    Ellipse(hdcMem, 10, H - 45, 45, H - 10);
                    SelectObject(hdcMem, faceB);
                    Ellipse(hdcMem, 14, H - 41, 41, H - 14);
                    
                    int ex = 8, ey = 8;
                    for (int i = 0; i < 45; i++) {
                        for (int j = 0; j < 45; j++) {
                            if (GetMapValue(i, j) == 2) { ex = i; ey = j; break; }
                        }
                    }
                    float targetAngle = (float)atan2(ey - pY, ex - pX) - (float)atan2(dY, dX);
                    int cx = 27, cy = H - 27;
                    int nx = cx + (int)(cos(targetAngle) * 10);
                    int ny = cy + (int)(sin(targetAngle) * 10);
                    
                    HPEN needleP = CreatePen(PS_SOLID, 2, RGB(255, 50, 50));
                    SelectObject(hdcMem, needleP);
                    MoveToEx(hdcMem, cx, cy, NULL); LineTo(hdcMem, nx, ny);
                    DeleteObject(brassB); DeleteObject(faceB); DeleteObject(goldP); DeleteObject(needleP);
                }

                if (hasPickaxe > 0) {
                    int swing = (int)(sin(animFrameCount * 0.3f) * 4);
                    int bx = W - 45 + swing, by = H - 40 - swing;
                    
                    HPEN handleP = CreatePen(PS_SOLID, 3, RGB(139, 69, 19));
                    SelectObject(hdcMem, handleP);
                    MoveToEx(hdcMem, bx, by + 30, NULL); LineTo(hdcMem, bx + 20, by);
                    
                    HPEN headP = CreatePen(PS_SOLID, 3, RGB(180, 190, 200));
                    SelectObject(hdcMem, headP);
                    MoveToEx(hdcMem, bx + 10, by - 5, NULL); LineTo(hdcMem, bx + 28, by + 12);
                    DeleteObject(handleP); DeleteObject(headP);
                }
            }
            
            // UI Text
            char uiText[128];
            if (gameState == 0) {
                wsprintfA(uiText, "KMAZE - Press ENTER to start  [Played:%d Escaped:%d]", totalGames, totalEscapes);
            } else if (gameState == 2) {
                DWORD elapsedSec = (endTime - startTime) / 1000;
                wsprintfA(uiText, "You Escaped 35 Stages! Score: %d Time: %ds (ENTER restart)", score, elapsedSec);
            } else {
                DWORD elapsedSec = (GetTickCount() - startTime) / 1000;
                wsprintfA(uiText, "Lvl:%d/35 Key:%d P:%d C:%d S:%d F:%d Score:%d %ds", currentLevel + 1, keysHeld, hasPickaxe, pathfinderCharges, speedShoesCharges, stunSprayCharges, score, elapsedSec);
            }
            SetBkMode(hdcMem, TRANSPARENT);
            SetTextColor(hdcMem, RGB(0, 0, 0));
            TextOutA(hdcMem, 11, 11, uiText, lstrlenA(uiText));
            SetTextColor(hdcMem, RGB(255, 255, 255));
            TextOutA(hdcMem, 10, 10, uiText, lstrlenA(uiText));

            if (msgTimer > 0) {
                SetTextColor(hdcMem, RGB(0, 0, 0));
                TextOutA(hdcMem, W/2 - 59, 31, msgText, lstrlenA(msgText));
                SetTextColor(hdcMem, RGB(255, 255, 0));
                TextOutA(hdcMem, W/2 - 60, 30, msgText, lstrlenA(msgText));
            }

            // Active Items Legend HUD
            if (gameState == 1) {
                char itemText[128];
                wsprintfA(itemText, "[P]Break  [C]Path:%ds  [S]Speed:%ds  [F]Stun:%ds", pathfinderTimer/1000, speedShoesTimer/1000, stunSprayTimer/1000);
                SetTextColor(hdcMem, RGB(0, 0, 0));
                TextOutA(hdcMem, 11, H - 19, itemText, lstrlenA(itemText));
                SetTextColor(hdcMem, RGB(0, 255, 255));
                TextOutA(hdcMem, 10, H - 20, itemText, lstrlenA(itemText));
            }

            // Minimap with direction arrow & Pathfinder Path
            if (gameState == 1 && (hasCompass || pathfinderTimer > 0 || currentLevel < 15)) {
                int mmW = 0, mmH = 0;
                if (currentLevel >= 10) { mmW = curRandW; mmH = curRandH; }
                else if (currentLevel == 0 || currentLevel == 3) { mmW = 10; mmH = 10; }
                else if (currentLevel == 1 || currentLevel == 4 || currentLevel == 5 || currentLevel == 7 || currentLevel == 8) { mmW = 12; mmH = 12; }
                else if (currentLevel == 2 || currentLevel == 6 || currentLevel == 9) { mmW = 15; mmH = 15; }
                
                if (mmW > 0) {
                    int mmS = 5;
                    if (mmW > 15) mmS = 4;
                    if (mmW > 23) mmS = 3;
                    if (mmW > 35) mmS = 2;
                    int mmX = W - 10 - mmW * mmS;
                    int mmY = 10;
                    
                    HBRUSH frameB = CreateSolidBrush(RGB(40, 40, 50));
                    RECT frameRc = {mmX - 2, mmY - 2, mmX + mmW * mmS + 2, mmY + mmH * mmS + 2};
                    FillRect(hdcMem, &frameRc, frameB);
                    DeleteObject(frameB);

                    HBRUSH mWall = CreateSolidBrush(RGB(153, 153, 153));
                    HBRUSH mExit = CreateSolidBrush(RGB(0, 255, 0));
                    HBRUSH mKey = CreateSolidBrush(RGB(255, 255, 0));
                    HBRUSH mDoor = CreateSolidBrush(RGB(0, 0, 255));
                    HBRUSH mFloor = CreateSolidBrush(RGB(20, 20, 25));
                    HBRUSH mPlayer = CreateSolidBrush(RGB(255, 0, 0));
                    HBRUSH mCoin = CreateSolidBrush(RGB(255, 128, 0));
                    HBRUSH mTrap = CreateSolidBrush(RGB(255, 0, 0));
                    HBRUSH mComp = CreateSolidBrush(RGB(0, 255, 255));
                    HBRUSH mSpeed = CreateSolidBrush(RGB(255, 255, 0));
                    HBRUSH mTele = CreateSolidBrush(RGB(255, 0, 255));
                    HBRUSH mPath = CreateSolidBrush(RGB(0, 255, 255));
                    HBRUSH mBoss = CreateSolidBrush(RGB(255, 215, 0));
                    
                    for (int i = 0; i < mmW; i++) {
                        for (int j = 0; j < mmH; j++) {
                            if (currentLevel >= 15 && pathfinderTimer <= 0) {
                                float distToPlayer = (float)sqrt((i - pX)*(i - pX) + (j - pY)*(j - pY));
                                if (distToPlayer > 5.5f) continue;
                            }
                            int v = GetMapValue(i, j);
                            HBRUSH b = mFloor;
                            if (isPathTile[i][j] && pathfinderTimer > 0) b = mPath;
                            else if (v == 1 || v == 7) b = mWall;
                            else if (v == 2) b = mExit;
                            else if (v == 3) b = mKey;
                            else if (v == 4) b = mDoor;
                            else if (v == 5) b = mCoin;
                            else if (v == 6) b = mTrap;
                            else if (v == 8) b = mComp;
                            else if (v == 9) b = mSpeed;
                            else if (v == 10 || v == 11) b = mTele;
                            else if (v == 12) b = CreateSolidBrush(RGB(255, 50, 50));
                            else if (v == 13) b = CreateSolidBrush(RGB(150, 75, 0));
                            else if (v == 14) b = CreateSolidBrush(RGB(100, 200, 255));
                            else if (v == 15) b = mBoss;
                            
                            RECT mr = {mmX + i*mmS, mmY + j*mmS, mmX + i*mmS + mmS, mmY + j*mmS + mmS};
                            FillRect(hdcMem, &mr, b);
                        }
                    }
                    RECT mr = {mmX + (int)pX*mmS, mmY + (int)pY*mmS, mmX + (int)pX*mmS + mmS, mmY + (int)pY*mmS + mmS};
                    FillRect(hdcMem, &mr, mPlayer);
                    
                    DeleteObject(mWall); DeleteObject(mExit); DeleteObject(mKey); DeleteObject(mDoor); DeleteObject(mFloor); DeleteObject(mPlayer); DeleteObject(mCoin);
                    DeleteObject(mTrap); DeleteObject(mComp); DeleteObject(mSpeed); DeleteObject(mTele); DeleteObject(mPath); DeleteObject(mBoss);
                }
            }

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
