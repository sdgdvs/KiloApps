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

char* my_strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
    for (const char* p = haystack; *p; p++) {
        const char* h = p;
        const char* n = needle;
        while (*n && *h == *n) { h++; n++; }
        if (!*n) return (char*)p;
    }
    return NULL;
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
int timeFreezeCharges = 1;
int timeFreezeTimer = 0;
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

typedef struct { int up, down, left, right, pickaxe, pathfinder, speed, stun, freeze; } KeyBinds;
KeyBinds keyBinds = { VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, 'P', 'C', 'S', 'F', 'T' };
int waitingForKey = 0;
int prevState = 0;

#define MAX_REPLAY_FRAMES 10000
typedef struct { float px, py, dx, dy, planex, planey; } ReplayFrame;
ReplayFrame replayFrames[MAX_REPLAY_FRAMES];
int replayFrameCount = 0;
int replayLevel = 0;
int replayMap[45][45];
int replayCurFrame = 0;

// 16x16 Textures buffer: 25 types, 256 DWORD colors (0x00RRGGBB)
DWORD textures[30][256];
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
    for (int t = 0; t < 30; t++) {
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
                } else if (t == 16) { // NPC
                    if (y >= 4 && y <= 12 && x >= 4 && x <= 11) col = 0x0000FF66; else if (y < 4 && x >= 6 && x <= 9) col = 0x00FFCC99; else col = 0x00222222;
                } else if (t == 17) { // Switch
                    float dist = (float)sqrt((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f)); if (dist < 4.0f) col = 0x00FF0000; else col = 0x00555555;
                } else if (t == 18) { // Puzzle Door
                    if (x % 4 == 0) col = 0x00888888; else col = 0x00111111;
                } else if (t == 19) { // Time Freeze
                    float dist = (float)sqrt((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f)); if (dist < 5.0f && dist > 3.0f) col = 0x00CCCCCC; else if (dist <= 3.0f) col = 0x000088FF; else if (x == 7 && y < 3) col = 0x00FFFFFF; else col = 0x00000000;
                } else if (t == 20) { // Tech Wall
                    col = ((x*y) % 7 == 0) ? 0x0000FF00 : 0x00222222;
                } else if (t == 21) { // Ice Wall
                    col = RGB(100 + x*5, 200, 255);
                } else if (t == 22) { // Void Wall
                    int noise = rand() % 20; col = RGB(noise, noise, 50 + noise*2);
                } else if (t == 23) { // Mossy Floor
                    int noise = ((x * 13 + y * 37) % 30) - 15;
                    int r = 40 + noise; if (r < 0) r = 0; if (r > 255) r = 255;
                    int g = 60 + noise; if (g < 0) g = 0; if (g > 255) g = 255;
                    int b = 40 + noise; if (b < 0) b = 0; if (b > 255) b = 255;
                    if ((x+y)%2 == 0) { r = (r>10)?r-10:0; g = (g>10)?g-10:0; b = (b>10)?b-10:0; }
                    col = RGB(r, g, b);
                } else if (t == 24) { // Cave Ceiling
                    int noise = ((x * 7 + y * 23) % 20) - 10;
                    int r = 20 + noise; if (r < 0) r = 0; if (r > 255) r = 255;
                    int g = 20 + noise; if (g < 0) g = 0; if (g > 255) g = 255;
                    int b = 30 + noise; if (b < 0) b = 0; if (b > 255) b = 255;
                    col = RGB(r, g, b);
                } else if (t == 25) { // Spike Trap
                    if ((x+y)%4 == 0) col = 0x00888888; else col = 0x00222222;
                } else if (t == 26) { // Cursed Relic
                    float dist = (float)sqrt((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f));
                    if (dist < 4.0f) col = 0x00800080;
                    else if (dist < 6.0f) col = 0x00000000;
                    else col = 0x00220022;
                } else {
                    col = 0x00AA0000;
                }
                textures[t][y * 16 + x] = col;
            }
        }
    }
}

void UpdateTextures() {
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            DWORD col = 0;
            // Exit Portal (t=2) - pulsing rings
            float dist2 = (float)sqrt((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f));
            float pulse = (float)sin((animFrameCount + dist2 * 10.0f) * 0.1f) * 1.5f;
            if (dist2 < 3.0f + pulse) col = 0x00FFFFFF;
            else if (dist2 < 5.5f + pulse) col = 0x0000FF66;
            else if (dist2 < 7.5f + pulse) col = 0x00009933;
            else col = 0x00003311;
            textures[2][y * 16 + x] = col;

            // Key Block (t=3) - bobbing animation
            int bob = (int)(sin(animFrameCount * 0.1f) * 2.0f);
            int by = y - bob;
            int isKey = 0;
            if (by >= 0 && by < 16) {
                if ((x >= 6 && x <= 9 && by >= 2 && by <= 5) || (x == 7 && by >= 6 && by <= 12) || (x >= 8 && x <= 10 && by >= 10 && by <= 12)) isKey = 1;
            }
            textures[3][y * 16 + x] = isKey ? 0x00FFFF00 : 0x00B8860B;

            // Teleporter Vortex (t=10, 11) - swirling
            float dx = x - 7.5f;
            float dy = y - 7.5f;
            float dist = (float)sqrt(dx*dx + dy*dy);
            float angle = (float)atan2(dy, dx) + animFrameCount * 0.1f;
            if (dist < 6.0f && ((int)(dist * 2.0f + angle * 3.0f) % 2 == 0)) {
                textures[10][y * 16 + x] = 0x00FF00FF;
                textures[11][y * 16 + x] = 0x00FF00FF;
            } else {
                textures[10][y * 16 + x] = 0x00300044;
                textures[11][y * 16 + x] = 0x00300044;
            }
            
            // Boss / Minotaur eyes breathing
            int m_breathe = (int)(sin(animFrameCount * 0.15f) * 1.5f);
            if ((x >= 4 && x <= 6 && y >= 6 && y <= 7) || (x >= 9 && x <= 11 && y >= 6 && y <= 7)) {
                // Minotaur
                if (m_breathe > 0) textures[12][y * 16 + x] = 0x00FF8800;
                else textures[12][y * 16 + x] = 0x00FFFF00;
                // Minotaur King Boss
                if (m_breathe > 0) textures[15][y * 16 + x] = 0x00FF0000;
                else textures[15][y * 16 + x] = 0x00880000;
            }
            // Spike Trap (t=25)
            if ((animFrameCount / 10) % 2 == 0) {
                if ((x+y)%4 == 0) textures[25][y * 16 + x] = 0x00FFFFFF;
                else textures[25][y * 16 + x] = 0x00FF0000;
            } else {
                if ((x+y)%4 == 0) textures[25][y * 16 + x] = 0x00888888;
                else textures[25][y * 16 + x] = 0x00222222;
            }
            // Cursed Relic (t=26)
            int c_bob = (int)(sin(animFrameCount * 0.2f) * 2.0f);
            int cy = y - c_bob;
            float dist3 = (float)sqrt((x - 7.5f) * (x - 7.5f) + (cy - 7.5f) * (cy - 7.5f));
            if (dist3 < 4.0f) textures[26][y * 16 + x] = 0x00FF00FF;
            else if (dist3 < 6.0f) textures[26][y * 16 + x] = 0x00000000;
            else textures[26][y * 16 + x] = 0x00220022;
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
    if (val == 0 || val == 2 || val == 3 || val == 5 || val == 6 || val == 7 || val == 8 || val == 9 || val == 10 || val == 11 || val == 12 || val == 13 || val == 14 || val == 15 || val == 16 || val == 17 || val == 19 || val == 25 || val == 26) return 1;
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
                if (tile == 0 || tile == 2 || tile == 3 || tile == 5 || tile == 6 || tile == 8 || tile == 9 || tile == 10 || tile == 11 || tile == 13 || tile == 14 || tile == 16 || tile == 17 || tile == 18 || tile == 19 || tile == 25 || tile == 26 || (tile == 4 && keysHeld > 0)) {
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
    for(int i=0; i<w*h/25; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) {
            mapRandom[rx][ry] = 25;
        }
    }
    for(int i=0; i<w*h/60; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) {
            mapRandom[rx][ry] = 26;
        }
    }
    if (currentLevel >= 10 && currentLevel % 5 == 0) {
        int placedBoss = 0;
        while (!placedBoss) {
            int rx = 1 + rand()%(w-2);
            int ry = 1 + rand()%(h-2);
            if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY) && abs(rx - 1) + abs(ry - 1) > 8) {
                mapRandom[rx][ry] = 15;
                bossHP = 3;
                placedBoss = 1;
            }
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
    for(int i=0; i<w*h/80; i++) {
        int rx = 1 + rand()%(w-2); int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) mapRandom[rx][ry] = 16;
    }
    for(int i=0; i<w*h/60; i++) {
        int rx = 1 + rand()%(w-2); int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) mapRandom[rx][ry] = 19;
    }
    int numDoors = w*h/50;
    for (int i=0; i<numDoors; i++) {
        int rx = 1 + rand()%(w-2); int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY) && abs(rx - 1) + abs(ry - 1) > 3) mapRandom[rx][ry] = 18;
    }
    for (int i=0; i< (numDoors > 0 ? 2 : 0); i++) {
        int rx = 1 + rand()%(w-2); int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1)) mapRandom[rx][ry] = 17;
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

void ExportStats() {
    HANDLE hFile = CreateFileA("kmaze_stats.json", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[256];
        int len = wsprintfA(buf, "{\n  \"bestTime\": %d,\n  \"totalGames\": %d,\n  \"totalEscapes\": %d,\n  \"totalScore\": %d\n}", 
            (int)bestTime, totalGames, totalEscapes, totalScore);
        DWORD w = 0;
        WriteFile(hFile, buf, len, &w, NULL);
        CloseHandle(hFile);
        strcpy(msgText, "Exported kmaze_stats.json!"); msgTimer = 60;
    }
}

void ImportStats() {
    HANDLE hFile = CreateFileA("kmaze_stats.json", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char buf[256] = {0};
        DWORD r = 0;
        ReadFile(hFile, buf, sizeof(buf)-1, &r, NULL);
        CloseHandle(hFile);
        char* p;
        p = my_strstr(buf, "\"bestTime\":"); if (p) { p += 11; int bt = 0; while(*p==' '||*p=='\t') p++; while(*p>='0'&&*p<='9') { bt = bt*10 + (*p-'0'); p++; } bestTime = (float)bt; }
        p = my_strstr(buf, "\"totalGames\":"); if (p) { p += 13; int tg = 0; while(*p==' '||*p=='\t') p++; while(*p>='0'&&*p<='9') { tg = tg*10 + (*p-'0'); p++; } totalGames = tg; }
        p = my_strstr(buf, "\"totalEscapes\":"); if (p) { p += 15; int te = 0; while(*p==' '||*p=='\t') p++; while(*p>='0'&&*p<='9') { te = te*10 + (*p-'0'); p++; } totalEscapes = te; }
        p = my_strstr(buf, "\"totalScore\":"); if (p) { p += 14; int ts = 0; while(*p==' '||*p=='\t') p++; while(*p>='0'&&*p<='9') { ts = ts*10 + (*p-'0'); p++; } totalScore = ts; }
        SaveBest();
        strcpy(msgText, "Imported stats!"); msgTimer = 60;
    } else {
        strcpy(msgText, "kmaze_stats.json not found!"); msgTimer = 60;
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
    timeFreezeCharges = 1;
    pathfinderTimer = 0;
    speedShoesTimer = 0;
    stunSprayTimer = 0;
    timeFreezeTimer = 0;

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

    replayFrameCount = 0;
    replayLevel = currentLevel;
    for(int i=0; i<45; i++) for(int j=0; j<45; j++) replayMap[i][j] = GetMapValue(i, j);
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
            UpdateTextures();
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
            if (timeFreezeTimer > 0) timeFreezeTimer -= 30;

            float moveSpeed = 0.1f;
            float rotSpeed = 0.05f;
            if (speedShoesTimer > 0 || speedBoost) moveSpeed *= 2.0f;
            
            if (gameState == 1) {
                minotaurTimer += 30;
                if (minotaurTimer >= 1000) {
                    minotaurTimer = 0;
                    if (stunSprayTimer <= 0 && timeFreezeTimer <= 0) {
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
            
            if (gameState == 2 && replayFrameCount > 0 && (GetAsyncKeyState('R') & 0x8000)) {
                gameState = 3;
                replayCurFrame = 0;
                activeKeyCooldown = 300;
            } else if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
                if (gameState == 0 || gameState == 2) {
                    gameState = 1;
                    startTime = GetTickCount();
                    currentLevel = -1;
                    score = 0;
                    totalGames++;
                    SaveBest();
                    ResetMaps();
                    NextLevel();
                    activeKeyCooldown = 300;
                }
            }
            
            if (gameState == 1 && activeKeyCooldown <= 0) {
                if (GetAsyncKeyState(keyBinds.pickaxe) & 0x8000) {
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
                if (GetAsyncKeyState(keyBinds.pathfinder) & 0x8000) {
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
                if (GetAsyncKeyState(keyBinds.speed) & 0x8000) {
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
                if (GetAsyncKeyState(keyBinds.stun) & 0x8000) {
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
                if (GetAsyncKeyState(keyBinds.freeze) & 0x8000) {
                    if (timeFreezeCharges > 0) {
                        timeFreezeCharges--;
                        timeFreezeTimer = 10000;
                        MessageBeep(MB_ICONASTERISK);
                        AddParticles(160.0f, 120.0f, RGB(0, 100, 255), 30);
                        strcpy(msgText, "Time Freeze Active (10s)!");
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
                        WriteFile(hSave, &timeFreezeCharges, sizeof(int), &written, NULL);
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
                        ReadFile(hLoad, &timeFreezeCharges, sizeof(int), &readBytes, NULL);
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
                if (gameState == 3) {
                    if (GetAsyncKeyState(VK_RIGHT) & 0x8000 || GetAsyncKeyState('D') & 0x8000) {
                        if (replayCurFrame < replayFrameCount - 1) replayCurFrame++;
                    }
                    if (GetAsyncKeyState(VK_LEFT) & 0x8000 || GetAsyncKeyState('A') & 0x8000) {
                        if (replayCurFrame > 0) replayCurFrame--;
                    }
                }
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }

            if (replayFrameCount < MAX_REPLAY_FRAMES) {
                replayFrames[replayFrameCount].px = pX;
                replayFrames[replayFrameCount].py = pY;
                replayFrames[replayFrameCount].dx = dX;
                replayFrames[replayFrameCount].dy = dY;
                replayFrames[replayFrameCount].planex = planeX;
                replayFrames[replayFrameCount].planey = planeY;
                replayFrameCount++;
            }

            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                moveSpeed *= 1.5f;
            }
            if (GetAsyncKeyState(keyBinds.up) & 0x8000) {
                if (TryMove((int)(pX + dX * moveSpeed), (int)pY)) pX += dX * moveSpeed;
                if (TryMove((int)pX, (int)(pY + dY * moveSpeed))) pY += dY * moveSpeed;
            }
            if (GetAsyncKeyState(keyBinds.down) & 0x8000) {
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
            } else if (curVal == 16) {
                if (score >= 200) {
                    score -= 200; hasPickaxe++; timeFreezeCharges++;
                    SetMapValue((int)pX, (int)pY, 0); MessageBeep(MB_ICONASTERISK); AddParticles(160.0f, 120.0f, RGB(0, 255, 100), 25);
                    strcpy(msgText, "-200 Score: Trade for Pickaxe + Freeze!"); msgTimer = 60;
                } else { strcpy(msgText, "Need 200 Score for Merchant!"); msgTimer = 10; }
            } else if (curVal == 17) {
                for (int i=0; i<45; i++) for(int j=0; j<45; j++) { if (GetMapValue(i, j) == 18) SetMapValue(i, j, 0); }
                SetMapValue((int)pX, (int)pY, 0); MessageBeep(MB_ICONASTERISK); AddParticles(160.0f, 120.0f, RGB(255, 0, 0), 25);
                strcpy(msgText, "Puzzle Switch Pressed! Doors Open!"); msgTimer = 60;
            } else if (curVal == 19) {
                timeFreezeCharges++; SetMapValue((int)pX, (int)pY, 0); MessageBeep(MB_ICONASTERISK); AddParticles(160.0f, 120.0f, RGB(0, 100, 255), 15);
                strcpy(msgText, "+1 Time Freeze!"); msgTimer = 60;
            } else if (curVal == 25) {
                if ((animFrameCount / 10) % 2 == 0) {
                    MessageBeep(MB_ICONHAND);
                    score = (score >= 75) ? score - 75 : 0;
                    pX = 1.5f; pY = 1.5f;
                    AddParticles(160.0f, 120.0f, RGB(255, 0, 0), 20);
                    strcpy(msgText, "Impaled by Spike Trap!"); msgTimer = 60;
                }
            } else if (curVal == 26) {
                score += 500;
                hasCompass = 0;
                pathfinderCharges = 0;
                speedBoost = 0;
                speedShoesCharges = 0;
                SetMapValue((int)pX, (int)pY, 0);
                MessageBeep(MB_ICONHAND);
                AddParticles(160.0f, 120.0f, RGB(128, 0, 128), 30);
                strcpy(msgText, "Cursed Relic: +500 Score, Lost Speed/Nav!"); msgTimer = 60;
            }

            if (GetAsyncKeyState(keyBinds.right) & 0x8000) {
                float oldDX = dX;
                dX = dX * (float)cos(rotSpeed) - dY * (float)sin(rotSpeed);
                dY = oldDX * (float)sin(rotSpeed) + dY * (float)cos(rotSpeed);
                float oldPlaneX = planeX;
                planeX = planeX * (float)cos(rotSpeed) - planeY * (float)sin(rotSpeed);
                planeY = oldPlaneX * (float)sin(rotSpeed) + planeY * (float)cos(rotSpeed);
            }
            if (GetAsyncKeyState(keyBinds.left) & 0x8000) {
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
                float drawPX = pX, drawPY = pY, drawDX = dX, drawDY = dY, drawPlaneX = planeX, drawPlaneY = planeY;
                if (gameState == 3 && replayFrameCount > 0) {
                    drawPX = replayFrames[replayCurFrame].px;
                    drawPY = replayFrames[replayCurFrame].py;
                    drawDX = replayFrames[replayCurFrame].dx;
                    drawDY = replayFrames[replayCurFrame].dy;
                    drawPlaneX = replayFrames[replayCurFrame].planex;
                    drawPlaneY = replayFrames[replayCurFrame].planey;
                }

                // 1. Ceiling & Floor Casting
                for (int y = 0; y < H; y++) {
                    int isFloor = (y >= H / 2);
                    int p = isFloor ? (y - H / 2) : (H / 2 - y);
                    if (p == 0) p = 1;

                    float posZ = 0.5f * H;
                    float rowDistance = posZ / p;

                    float rayDirX0 = drawDX - drawPlaneX;
                    float rayDirY0 = drawDY - drawPlaneY;
                    float rayDirX1 = drawDX + drawPlaneX;
                    float rayDirY1 = drawDY + drawPlaneY;

                    float floorStepX = rowDistance * (rayDirX1 - rayDirX0) / W;
                    float floorStepY = rowDistance * (rayDirY1 - rayDirY0) / W;

                    float floorX = drawPX + rowDistance * rayDirX0;
                    float floorY = drawPY + rowDistance * rayDirY0;

                    float maxDist = (currentLevel >= 20) ? 4.5f : ((currentLevel >= 10) ? 7.0f : 12.0f);
                    if (pathfinderTimer > 0) maxDist = 14.0f;
                    float fog = 1.0f - rowDistance / maxDist;
                    if (fog < 0.1f) fog = 0.1f; if (fog > 1.0f) fog = 1.0f;
                    if (currentLevel >= 15 && rowDistance > maxDist && pathfinderTimer <= 0) fog = 0.0f;

                    for (int x = 0; x < W; ++x) {
                        int cellX = (int)floor(floorX);
                        int cellY = (int)floor(floorY);
                        int tx = (int)(16.0f * (floorX - cellX)) & 15;
                        int ty = (int)(16.0f * (floorY - cellY)) & 15;

                        floorX += floorStepX;
                        floorY += floorStepY;

                        int texIdx = isFloor ? 23 : 24;
                        DWORD srcCol = textures[texIdx][ty * 16 + tx];

                        BYTE r = (BYTE)((srcCol & 0xFF) * fog);
                        BYTE g = (BYTE)(((srcCol >> 8) & 0xFF) * fog);
                        BYTE b = (BYTE)(((srcCol >> 16) & 0xFF) * fog);

                        pBits[y * W + x] = RGB(r, g, b);
                    }
                }
                for (int x = 0; x < W; x++) {
                    float cameraX = 2 * x / (float)W - 1;
                    float rayDX = drawDX + drawPlaneX * cameraX;
                    float rayDY = drawDY + drawPlaneY * cameraX;
                    
                    int mapX = (int)drawPX;
                    int mapY = (int)drawPY;
                    
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
                        int mval = (gameState == 3 && mapX >= 0 && mapY >= 0 && mapX < 45 && mapY < 45) ? replayMap[mapX][mapY] : GetMapValue(mapX, mapY);
                        if (mval > 0) hit = mval;
                    }
                    
                    if (side == 0) perpWallDist = (sideDistX - deltaDistX);
                    else           perpWallDist = (sideDistY - deltaDistY);
                    
                    int lineHeight = (int)(H / perpWallDist);
                    int drawStart = -lineHeight / 2 + H / 2;
                    int drawEnd = lineHeight / 2 + H / 2;
                    int actualStart = (drawStart < 0) ? 0 : drawStart;
                    int actualEnd = (drawEnd >= H) ? H - 1 : drawEnd;
                    
                    float wallX;
                    if (side == 0) wallX = drawPY + perpWallDist * rayDY;
                    else           wallX = drawPX + perpWallDist * rayDX;
                    wallX -= (float)floor(wallX);
                    int texX = (int)(wallX * 16.0f) & 15;
                    
                    if (hit == 7) hit = 1;
                    if (hit < 1 || hit > 22) hit = 1;
                    if (hit == 1) { if (currentLevel >= 30) hit = 22; else if (currentLevel >= 20) hit = 21; else if (currentLevel >= 10) hit = 20; }
                    
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
            
            // UI graphics are drawn above

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
                    HBRUSH mMono = CreateSolidBrush(RGB(255, 50, 50));
                    HBRUSH mPick = CreateSolidBrush(RGB(150, 75, 0));
                    HBRUSH mStun = CreateSolidBrush(RGB(100, 200, 255));
                    
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
                            else if (v == 12) b = mMono;
                            else if (v == 13) b = mPick;
                            else if (v == 14) b = mStun;
                            else if (v == 15) b = mBoss;
                            
                            RECT mr = {mmX + i*mmS, mmY + j*mmS, mmX + i*mmS + mmS, mmY + j*mmS + mmS};
                            FillRect(hdcMem, &mr, b);
                        }
                    }
                    RECT mr = {mmX + (int)pX*mmS, mmY + (int)pY*mmS, mmX + (int)pX*mmS + mmS, mmY + (int)pY*mmS + mmS};
                    FillRect(hdcMem, &mr, mPlayer);
                    
                    DeleteObject(mWall); DeleteObject(mExit); DeleteObject(mKey); DeleteObject(mDoor); DeleteObject(mFloor); DeleteObject(mPlayer); DeleteObject(mCoin);
                    DeleteObject(mTrap); DeleteObject(mComp); DeleteObject(mSpeed); DeleteObject(mTele); DeleteObject(mPath); DeleteObject(mBoss);
                    DeleteObject(mMono); DeleteObject(mPick); DeleteObject(mStun);
                }
            }

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            StretchBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, hdcMem, 0, 0, W, H, SRCCOPY);
            
            HFONT hFont = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5 /*CLEARTYPE_QUALITY*/, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
            HGDIOBJ oldFont = SelectObject(hdc, hFont);
            
            SetBkMode(hdc, TRANSPARENT);
            if (gameState == 0) {
                const char* t1 = "KMAZE";
                const char* t2 = "Press ENTER to start";
                const char* t3 = "Press H for Help / Keys";
                char t4[64]; wsprintfA(t4, "Games: %d Escapes: %d", totalGames, totalEscapes);
                
                SetTextColor(hdc, RGB(0, 0, 0));
                TextOutA(hdc, clientRect.right/2 - 30 + 2, clientRect.bottom/2 - 40 + 2, t1, lstrlenA(t1));
                TextOutA(hdc, clientRect.right/2 - 110 + 2, clientRect.bottom/2 - 10 + 2, t2, lstrlenA(t2));
                TextOutA(hdc, clientRect.right/2 - 130 + 2, clientRect.bottom/2 + 20 + 2, t3, lstrlenA(t3));
                TextOutA(hdc, clientRect.right/2 - 110 + 2, clientRect.bottom/2 + 50 + 2, t4, lstrlenA(t4));
                
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, clientRect.right/2 - 30, clientRect.bottom/2 - 40, t1, lstrlenA(t1));
                TextOutA(hdc, clientRect.right/2 - 110, clientRect.bottom/2 - 10, t2, lstrlenA(t2));
                TextOutA(hdc, clientRect.right/2 - 130, clientRect.bottom/2 + 20, t3, lstrlenA(t3));
                TextOutA(hdc, clientRect.right/2 - 110, clientRect.bottom/2 + 50, t4, lstrlenA(t4));
            } else if (gameState == 2) {
                DWORD elapsedSec = (endTime - startTime) / 1000;
                char t1[64]; wsprintfA(t1, "Escaped 35! Time: %ds", elapsedSec);
                char t2[64]; wsprintfA(t2, "Score: %d", score);
                const char* t3 = "Press R for Replay";
                const char* t4 = "ENTER to Restart";
                
                SetTextColor(hdc, RGB(0, 0, 0));
                TextOutA(hdc, clientRect.right/2 - 110 + 2, clientRect.bottom/2 - 40 + 2, t1, lstrlenA(t1));
                TextOutA(hdc, clientRect.right/2 - 60 + 2, clientRect.bottom/2 - 10 + 2, t2, lstrlenA(t2));
                TextOutA(hdc, clientRect.right/2 - 100 + 2, clientRect.bottom/2 + 20 + 2, t3, lstrlenA(t3));
                TextOutA(hdc, clientRect.right/2 - 90 + 2, clientRect.bottom/2 + 50 + 2, t4, lstrlenA(t4));
                
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, clientRect.right/2 - 110, clientRect.bottom/2 - 40, t1, lstrlenA(t1));
                TextOutA(hdc, clientRect.right/2 - 60, clientRect.bottom/2 - 10, t2, lstrlenA(t2));
                TextOutA(hdc, clientRect.right/2 - 100, clientRect.bottom/2 + 20, t3, lstrlenA(t3));
                TextOutA(hdc, clientRect.right/2 - 90, clientRect.bottom/2 + 50, t4, lstrlenA(t4));
            } else if (gameState == 3) {
                char uiText[128]; wsprintfA(uiText, "REPLAY Lvl %d - Frame %d/%d (A/D: scrub, ESC: exit)", replayLevel+1, replayCurFrame, replayFrameCount);
                SetTextColor(hdc, RGB(0, 0, 0)); TextOutA(hdc, 22, 22, uiText, lstrlenA(uiText));
                SetTextColor(hdc, RGB(255, 255, 255)); TextOutA(hdc, 20, 20, uiText, lstrlenA(uiText));
            } else if (gameState == 4) {
                const char* uiText = "KEYBINDS - Click below to bind, ESC to close";
                SetTextColor(hdc, RGB(0, 0, 0)); TextOutA(hdc, 22, 22, uiText, lstrlenA(uiText));
                SetTextColor(hdc, RGB(255, 255, 255)); TextOutA(hdc, 20, 20, uiText, lstrlenA(uiText));
            } else {
                DWORD elapsedSec = (GetTickCount() - startTime) / 1000;
                char t1[128]; wsprintfA(t1, "Lvl:%d/35 K:%d P:%d C:%d S:%d F:%d T:%d", currentLevel + 1, keysHeld, hasPickaxe, pathfinderCharges, speedShoesCharges, stunSprayCharges, timeFreezeCharges);
                char t2[128]; wsprintfA(t2, "Score:%d Time:%ds", score, elapsedSec);
                const char* t3 = "V:Save L:Load H:Help";
                
                SetTextColor(hdc, RGB(0, 0, 0));
                TextOutA(hdc, 22, 22, t1, lstrlenA(t1)); TextOutA(hdc, 22, 52, t2, lstrlenA(t2));
                TextOutA(hdc, clientRect.right - 248, 22, t3, lstrlenA(t3));
                
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, 20, 20, t1, lstrlenA(t1)); TextOutA(hdc, 20, 50, t2, lstrlenA(t2));
                TextOutA(hdc, clientRect.right - 250, 20, t3, lstrlenA(t3));
            }
            
            if (gameState == 4) {
                char kbText[64];
                int y = 80;
                const char* names[] = {"Up", "Down", "Left", "Right", "Pickaxe", "Pathfinder", "Speed", "Stun Spray", "Freeze"};
                int vals[] = {keyBinds.up, keyBinds.down, keyBinds.left, keyBinds.right, keyBinds.pickaxe, keyBinds.pathfinder, keyBinds.speed, keyBinds.stun, keyBinds.freeze};
                for (int i = 0; i < 9; i++) {
                    if (waitingForKey == i + 1) wsprintfA(kbText, "%s: ...", names[i]);
                    else wsprintfA(kbText, "%s: %c (%d)", names[i], (char)vals[i], vals[i]);
                    TextOutA(hdc, 80, y, kbText, lstrlenA(kbText));
                    y += 30;
                }
            }

            if (msgTimer > 0) {
                SetTextColor(hdc, RGB(0, 0, 0));
                TextOutA(hdc, clientRect.right/2 - 118 + 2, 82, msgText, lstrlenA(msgText));
                SetTextColor(hdc, RGB(255, 255, 0));
                TextOutA(hdc, clientRect.right/2 - 120, 80, msgText, lstrlenA(msgText));
            }

            // Active Items Legend HUD
            if (gameState == 1) {
                char itemText[128];
                wsprintfA(itemText, "[P]Break [C]Path:%ds [S]Speed:%ds [F]Stun:%ds [T]Freeze:%ds [H]Help", pathfinderTimer/1000, speedShoesTimer/1000, stunSprayTimer/1000, timeFreezeTimer/1000);
                SetTextColor(hdc, RGB(0, 0, 0));
                TextOutA(hdc, 22, clientRect.bottom - 28, itemText, lstrlenA(itemText));
                SetTextColor(hdc, RGB(0, 255, 255));
                TextOutA(hdc, 20, clientRect.bottom - 30, itemText, lstrlenA(itemText));
            }
            SelectObject(hdc, oldFont);
            DeleteObject(hFont);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_LBUTTONDOWN: {
            if (gameState == 4) {
                int y = HIWORD(lParam);
                int idx = (y - 80) / 30;
                if (idx >= 0 && idx < 9) waitingForKey = idx + 1;
            }
            break;
        }
        case WM_KEYDOWN: {
            if (gameState == 0 || gameState == 1) {
                if (wParam == 'E') ExportStats();
                if (wParam == 'I') ImportStats();
                if (wParam == 'K' || wParam == 'H') { prevState = gameState; gameState = 4; }
            }
            if (gameState == 4) {
                if (wParam == VK_ESCAPE) { waitingForKey = 0; gameState = prevState; }
                else if (waitingForKey > 0) {
                    if (waitingForKey == 1) keyBinds.up = (int)wParam;
                    if (waitingForKey == 2) keyBinds.down = (int)wParam;
                    if (waitingForKey == 3) keyBinds.left = (int)wParam;
                    if (waitingForKey == 4) keyBinds.right = (int)wParam;
                    if (waitingForKey == 5) keyBinds.pickaxe = (int)wParam;
                    if (waitingForKey == 6) keyBinds.pathfinder = (int)wParam;
                    if (waitingForKey == 7) keyBinds.speed = (int)wParam;
                    if (waitingForKey == 8) keyBinds.stun = (int)wParam;
                    waitingForKey = 0;
                }
            }
            if (gameState == 3) {
                if (wParam == VK_ESCAPE) { gameState = 2; }
            }
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
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        typedef BOOL(WINAPI *SetProcessDPIAwareFunc)(void);
        SetProcessDPIAwareFunc setDpiAware = (SetProcessDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (setDpiAware) setDpiAware();
    }
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KMazeClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    
    RegisterClassA(&wc);
    
    RECT wr = {0, 0, 800, 600};
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
