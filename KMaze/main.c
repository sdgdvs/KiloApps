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
    {1,0,1,1,39,0,1,0,0,1},
    {1,5,0,1,0,29,0,0,0,1},
    {1,0,0,1,1,1,1,0,0,1},
    {1,0,0,0,0,28,0,0,5,1},
    {1,1,0,1,1,1,1,1,0,1},
    {1,0,0,0,5,0,0,1,0,1},
    {1,40,0,0,0,0,0,2,0,1},
    {1,1,1,1,1,1,1,1,1,1}
};

const int orig_map2[12][12] = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,5,0,0,1,0,0,0,5,0,1},
    {1,0,1,1,0,1,0,1,1,1,0,1},
    {1,0,1,0,0,0,29,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,0,1,0,1},
    {1,5,0,0,1,0,0,1,5,1,0,1},
    {1,1,1,0,1,0,1,1,0,1,0,1},
    {1,0,0,0,1,0,28,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,5,1},
    {1,0,5,0,0,0,0,0,0,1,0,1},
    {1,1,1,1,1,1,1,1,0,1,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1}
};

const int orig_map3[15][15] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,5,0,0,0,0,0,0,5,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,1,5,0,0,0,29,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,39,1,0,1},
    {1,0,1,0,1,0,5,0,0,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,1,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,2,1,0,1,0,1,5,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,28,0,1,0,1,0,1,0,1},
    {1,5,1,0,1,1,1,1,1,0,1,0,1,0,1},
    {1,0,1,0,0,5,0,0,0,0,1,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,40,0,0,0,0,0,5,0,0,0,0,1,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

const int orig_map4[10][10] = {
    {1,1,1,1,1,1,1,1,1,1},
    {1,0,0,5,0,0,1,3,0,1},
    {1,1,1,1,1,0,1,1,0,1},
    {1,5,0,0,1,0,0,0,0,1},
    {1,0,1,0,1,1,1,1,1,1},
    {1,0,1,0,28,0,5,0,4,1},
    {1,0,1,1,1,1,1,0,1,1},
    {1,0,5,0,0,0,1,0,2,1},
    {1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1}
};

const int orig_map5[12][12] = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,40,0,0,1,0,0,0,0,0,3,1},
    {1,0,1,0,1,0,1,1,1,1,1,1},
    {1,0,1,0,1,0,0,29,0,0,0,1},
    {1,0,1,0,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,1,0,1},
    {1,0,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,28,0,1,0,1,0,1},
    {1,1,1,1,1,1,0,1,0,1,0,1},
    {1,4,0,0,39,1,0,0,0,1,0,1},
    {1,38,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1}
};

const int orig_map6[12][12] = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,6,0,0,0,1,5,0,0,1},
    {1,0,1,1,1,1,0,1,1,1,0,1},
    {1,0,1,5,0,1,0,6,0,1,0,1},
    {1,0,1,0,0,1,0,1,0,1,0,1},
    {1,0,1,1,4,1,1,1,0,1,0,1},
    {1,0,0,0,0,29,0,0,0,1,0,1},
    {1,1,1,1,1,1,1,1,0,1,0,1},
    {1,3,6,0,5,0,0,1,0,1,0,1},
    {1,0,1,1,1,1,0,1,6,1,0,1},
    {1,0,0,28,0,1,0,0,0,1,2,1},
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
    {1,0,0,0,1,0,28,0,1,0,0,0,4,0,1},
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
    {1,0,0,0,0,29,0,1,5,0,0,1},
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
    {1,5,0,1,0,28,0,0,0,1,0,1,0,0,1},
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

// Power-ups, Active Items, and Dynamic Lighting
int pathfinderCharges = 1;
int pathfinderTimer = 0;
int speedShoesCharges = 1;
int speedShoesTimer = 0;
int stunSprayCharges = 1;
int stunSprayTimer = 0;
int timeFreezeCharges = 1;
int timeFreezeTimer = 0;
int torchTimer = 0;
int bossHP = 3;

// Checkpoint System
int checkpointLevel = -1;
int checkpointScore = 0;
float checkpointPX = 1.5f, checkpointPY = 1.5f;
int checkpointKeys = 0;
int checkpointPickaxe = 0;
int checkpointPathfinder = 0;
int checkpointSpeed = 0;
int checkpointStun = 0;
int checkpointFreeze = 0;
int checkpointMapW = 15, checkpointMapH = 15;
int checkpointMap[45][45];

int totalGames = 0;
int totalEscapes = 0;
int totalScore = 0;

int gameState = 0; // 0=start, 1=play, 2=win, 3=replay, 4=keys
int minotaurFacingDir[45][45] = {0};
int damageFlinchTimer = 0;
DWORD startTime = 0;
DWORD endTime = 0;
float bestTime = 9999.9f;
int score = 0;

char msgText[128] = "";
int msgTimer = 0;
int isCrouching = 0;

int muzzleFlashTimer = 0;
int recoilOffset = 0;
typedef struct { float x, y, radius, outerRadius; int life; } Shockwave;
Shockwave shockwaves[10];
int shockwaveCount = 0;

void AddShockwave(float x, float y) {
    if (shockwaveCount < 10) {
        shockwaves[shockwaveCount].x = x;
        shockwaves[shockwaveCount].y = y;
        shockwaves[shockwaveCount].radius = 0.1f;
        shockwaves[shockwaveCount].outerRadius = 0.25f;
        shockwaves[shockwaveCount].life = 20;
        shockwaveCount++;
    }
}
void UpdateShockwaves() {
    int write = 0;
    for (int i = 0; i < shockwaveCount; i++) {
        shockwaves[i].radius += 0.35f;
        shockwaves[i].outerRadius += 0.55f;
        shockwaves[i].life--;
        if (shockwaves[i].life > 0) {
            shockwaves[write++] = shockwaves[i];
        }
    }
    shockwaveCount = write;
}

int screenShakeTimer = 0;
int screenShakeMaxTimer = 0;
float screenShakeIntensity = 0.0f;
void AddScreenShake(int duration, float intensity) {
    if (screenShakeTimer < duration) {
        screenShakeTimer = duration;
        screenShakeMaxTimer = duration;
    }
    if (screenShakeIntensity < intensity) screenShakeIntensity = intensity;
}

float pX = 1.5f, pY = 1.5f;
float dX = 1.0f, dY = 0.0f;
float planeX = 0.0f, planeY = 0.66f;

typedef struct { int up, down, left, right, pickaxe, pathfinder, speed, stun, freeze, crouch; } KeyBinds;
KeyBinds keyBinds = { VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, 'P', 'C', 'S', 'F', 'T', 'X' };
int waitingForKey = 0;
int prevState = 0;

#define MAX_REPLAY_FRAMES 10000
typedef struct { float px, py, dx, dy, planex, planey; } ReplayFrame;
ReplayFrame replayFrames[MAX_REPLAY_FRAMES];
int replayFrameCount = 0;
int replayLevel = 0;
int replayMap[45][45];
int replayCurFrame = 0;

// 16x16 Textures buffer: 50 types, 256 DWORD colors (0x00RRGGBB)
DWORD textures[50][256];
DWORD animFrameCount = 0;

// Multi-Layered Kinematic Particles (4 Layers)
// Layer 0: Incandescent core needle sparks
// Layer 1: Expanding buoyant plasma/smoke puffs
// Layer 2: Heavy kinematic stone & crystal shards with gravity and bounce
// Layer 3: Radiant celebration energy stars
#define MAX_PARTICLES 384
typedef struct {
    float x, y;
    float vx, vy;
    int life, maxLife;
    COLORREF color;
    int layer;
    float rot, vrot;
    float size;
    int polyType;
} Particle;
Particle particles[MAX_PARTICLES];
int particleCount = 0;

// Ambient atmospheric dungeon motes
#define MAX_MOTES 28
typedef struct {
    float x, y;
    float vx, vy;
    float size;
    float phase;
    int isGold;
} AmbientMote;
AmbientMote ambientMotes[MAX_MOTES];
int motesInitialized = 0;

void InitAmbientMotes() {
    for (int i = 0; i < MAX_MOTES; i++) {
        ambientMotes[i].x = (float)(rand() % 320);
        ambientMotes[i].y = (float)(rand() % 240);
        ambientMotes[i].vx = ((rand() % 100) / 100.0f - 0.5f) * 0.35f;
        ambientMotes[i].vy = -0.15f - ((rand() % 100) / 100.0f) * 0.25f;
        ambientMotes[i].size = 1.0f + ((rand() % 100) / 100.0f) * 1.5f;
        ambientMotes[i].phase = ((rand() % 628) / 100.0f);
        ambientMotes[i].isGold = (rand() % 100 > 45) ? 1 : 0;
    }
    motesInitialized = 1;
}

void AddParticles(float x, float y, COLORREF color, int count) {
    for (int i = 0; i < count; i++) {
        if (particleCount < MAX_PARTICLES) {
            float angle = (float)(rand() % 628) / 100.0f;
            int roll = rand() % 100;
            particles[particleCount].x = x + ((rand() % 16) - 8);
            particles[particleCount].y = y + ((rand() % 16) - 8);
            particles[particleCount].rot = (float)(rand() % 628) / 100.0f;
            particles[particleCount].vrot = ((rand() % 100) / 100.0f - 0.5f) * 0.35f;
            particles[particleCount].polyType = rand() % 3;

            if (roll < 40) {
                // Layer 0: Incandescent needle sparks
                float spd = 3.5f + (float)(rand() % 500) / 100.0f;
                particles[particleCount].layer = 0;
                particles[particleCount].vx = (float)cos(angle) * spd;
                particles[particleCount].vy = (float)sin(angle) * spd;
                particles[particleCount].life = 12 + rand() % 14;
                particles[particleCount].maxLife = particles[particleCount].life;
                particles[particleCount].size = 1.5f;
                particles[particleCount].color = (rand() % 100 > 40) ? RGB(255, 255, 255) : color;
            } else if (roll < 65) {
                // Layer 1: Expanding buoyant plasma/smoke puffs
                float spd = 0.6f + (float)(rand() % 200) / 100.0f;
                particles[particleCount].layer = 1;
                particles[particleCount].vx = (float)cos(angle) * spd;
                particles[particleCount].vy = (float)sin(angle) * spd - 0.8f;
                particles[particleCount].life = 25 + rand() % 20;
                particles[particleCount].maxLife = particles[particleCount].life;
                particles[particleCount].size = 3.5f + (float)(rand() % 200) / 100.0f;
                particles[particleCount].color = color;
            } else if (roll < 85) {
                // Layer 2: Heavy kinematic debris & crystal/stone shards
                float spd = 1.8f + (float)(rand() % 350) / 100.0f;
                particles[particleCount].layer = 2;
                particles[particleCount].vx = (float)cos(angle) * spd;
                particles[particleCount].vy = (float)sin(angle) * spd - 1.2f;
                particles[particleCount].life = 35 + rand() % 25;
                particles[particleCount].maxLife = particles[particleCount].life;
                particles[particleCount].size = 2.5f + (float)(rand() % 250) / 100.0f;
                particles[particleCount].color = color;
            } else {
                // Layer 3: Radiant celebration energy stars
                float spd = 1.0f + (float)(rand() % 250) / 100.0f;
                particles[particleCount].layer = 3;
                particles[particleCount].vx = (float)cos(angle) * spd;
                particles[particleCount].vy = (float)sin(angle) * spd;
                particles[particleCount].life = 28 + rand() % 22;
                particles[particleCount].maxLife = particles[particleCount].life;
                particles[particleCount].size = 4.0f + (float)(rand() % 200) / 100.0f;
                particles[particleCount].color = color;
            }
            particleCount++;
        }
    }
}

void UpdateParticles() {
    int write = 0;
    for (int i = 0; i < particleCount; i++) {
        if (particles[i].layer == 0) {
            particles[i].vx *= 0.94f;
            particles[i].vy *= 0.94f;
        } else if (particles[i].layer == 1) {
            particles[i].vy -= 0.06f;
            particles[i].vx *= 0.96f;
            particles[i].size += 0.12f;
        } else if (particles[i].layer == 2) {
            particles[i].vy += 0.16f;
            particles[i].vx *= 0.97f;
            particles[i].rot += particles[i].vrot;
            if (particles[i].y >= 235.0f && particles[i].vy > 0.0f) {
                particles[i].y = 235.0f;
                particles[i].vy = -particles[i].vy * 0.45f;
                particles[i].vrot *= 0.6f;
            }
        } else if (particles[i].layer == 3) {
            particles[i].vx *= 0.95f;
            particles[i].vy *= 0.95f;
        }
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].life--;
        if (particles[i].life > 0) {
            particles[write++] = particles[i];
        }
    }
    particleCount = write;

    if (!motesInitialized) InitAmbientMotes();
    for (int i = 0; i < MAX_MOTES; i++) {
        ambientMotes[i].x += ambientMotes[i].vx + (float)sin(animFrameCount * 0.05f + ambientMotes[i].phase) * 0.2f;
        ambientMotes[i].y += ambientMotes[i].vy;
        if (ambientMotes[i].y < 0.0f) { ambientMotes[i].y = 240.0f; ambientMotes[i].x = (float)(rand() % 320); }
        if (ambientMotes[i].x < 0.0f) ambientMotes[i].x = 320.0f;
        if (ambientMotes[i].x > 320.0f) ambientMotes[i].x = 0.0f;
    }
}

// Procedural 16x16 Texture Generator
void InitTextures() {
    for (int t = 0; t < 40; t++) {
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
                } else if (t == 6) { // Trap (Lava)
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
                } else if (t == 16) { // NPC Merchant
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
                } else if (t == 27) { // Magma Wall (Inferno Biome)
                    int vein = ((x * 17 + y * 29) % 20);
                    if (vein < 4) col = 0x000088FF; // Molten lava vein (RGB: 255, 136, 0)
                    else if (vein < 7) col = 0x000022AA;
                    else col = 0x00111122; // Obsidian crust
                } else if (t == 28) { // Ancient Save Shrine / Campfire
                    float dist = (float)sqrt((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f));
                    if (dist < 3.0f) col = 0x0000FFFF; // Golden flame core
                    else if (dist < 5.0f) col = 0x000088FF; // Amber aura
                    else if (y >= 12) col = 0x00888888; // Stone shrine base
                    else col = 0x00222233;
                } else if (t == 29) { // Wall Torch Sconce / Crystal
                    float dist = (float)sqrt((x - 7.5f) * (x - 7.5f) + (y - 5.0f) * (y - 5.0f));
                    if (dist < 3.0f) col = 0x0000FFFF; // Bright flame
                    else if (y >= 8 && (x == 7 || x == 8)) col = 0x00444444; // Iron sconce bracket
                    else col = 0x00111111;
                } else if (t == 38) { // Dungeon Descent Stairwell / Shaft
                    float dist = (float)sqrt((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f));
                    if (dist < 3.0f) col = 0x00111111; // Deep shaft pit
                    else if (dist < 5.5f && ((int)(dist * 2.0f + (x+y)) % 2 == 0)) col = 0x00FFFF00; // Cyan rune steps
                    else col = 0x00665544; // Stone rim
                } else if (t == 39) { // Secret Illusionary Fake Wall
                    int isMortar = (y == 3 || y == 7 || y == 11 || y == 15);
                    if (isMortar) col = 0x00443355;
                    else {
                        int rune = ((x + y * 3) % 5 == 0);
                        if (rune) col = 0x00DDAA00;
                        else col = 0x00664488;
                    }
                } else if (t == 40) { // Dungeon Lore Tablet / Ancient Runestone
                    if (x == 0 || x == 15 || y == 0 || y >= 13) col = 0x00222222;
                    else if ((x >= 3 && x <= 12) && (y >= 2 && y <= 11)) {
                        int glyph = ((x * 7 + y * 13) % 4 == 0) || (x == 7) || (y == 4 || y == 8);
                        if (glyph) col = 0x00FFFF00;
                        else col = 0x00111822;
                    } else col = 0x00333344;
                } else {
                    col = 0x00AA0000;
                }
                textures[t][y * 16 + x] = col;
            }
        }
    }
    for (int dir = 0; dir < 8; dir++) {
        for (int y = 0; y < 16; y++) {
            for (int x = 0; x < 16; x++) {
                DWORD col = 0;
                int isHorn = 0, isEye = 0, isSnout = 0, isTail = 0;
                if (dir == 0) {
                    if ((x>=2 && x<=5 && y<=4) || (x>=10 && x<=13 && y<=4)) isHorn = 1;
                    if ((x>=4 && x<=6 && y>=6 && y<=7) || (x>=9 && x<=11 && y>=6 && y<=7)) isEye = 1;
                    if (y>=10 && y<=12 && x>=5 && x<=10) isSnout = 1;
                } else if (dir == 1 || dir == 7) {
                    int shift = (dir == 1) ? -2 : 2;
                    if ((x>=2+shift && x<=5+shift && y<=4) || (x>=10+shift && x<=13+shift && y<=4)) isHorn = 1;
                    if ((x>=4+shift && x<=6+shift && y>=6 && y<=7) || (x>=9+shift && x<=11+shift && y>=6 && y<=7)) isEye = 1;
                    if (y>=10 && y<=12 && x>=5+shift && x<=10+shift) isSnout = 1;
                } else if (dir == 2) {
                    if (x>=8 && x<=12 && y<=4) isHorn = 1;
                    if (x>=10 && x<=12 && y>=6 && y<=7) isEye = 1;
                    if (y>=10 && y<=12 && x>=10 && x<=15) isSnout = 1;
                    if (y>=12 && y<=14 && x>=0 && x<=3) isTail = 1;
                } else if (dir == 6) {
                    if (x>=3 && x<=7 && y<=4) isHorn = 1;
                    if (x>=3 && x<=5 && y>=6 && y<=7) isEye = 1;
                    if (y>=10 && y<=12 && x>=0 && x<=5) isSnout = 1;
                    if (y>=12 && y<=14 && x>=12 && x<=15) isTail = 1;
                } else if (dir == 3 || dir == 5) {
                    int shift = (dir == 3) ? -2 : 2;
                    if ((x>=2+shift && x<=5+shift && y<=4) || (x>=10+shift && x<=13+shift && y<=4)) isHorn = 1;
                    if (y>=12 && y<=14 && x>=6-shift && x<=9-shift) isTail = 1;
                } else if (dir == 4) {
                    if ((x>=2 && x<=5 && y<=4) || (x>=10 && x<=13 && y<=4)) isHorn = 1;
                    if (y>=11 && y<=14 && x>=6 && x<=9) isTail = 1;
                }
                if (isHorn) col = RGB(50, 50, 50);
                else if (isEye) col = RGB(255, 255, 0);
                else if (isSnout) col = RGB(255, 255, 255);
                else if (isTail) col = RGB(80, 0, 0);
                else col = RGB(153, 0, 0);
                textures[30 + dir][y * 16 + x] = col;
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
                if (m_breathe > 0) textures[12][y * 16 + x] = 0x000088FF;
                else textures[12][y * 16 + x] = 0x0000FFFF;
                if (bossHP <= 1) textures[15][y * 16 + x] = (m_breathe > 0) ? 0x0000FFFF : 0x00008888;
                else textures[15][y * 16 + x] = (m_breathe > 0) ? 0x000000FF : 0x00000088;
            } else if (y >= 10 && y <= 13 && x >= 4 && x <= 11) {
                if (bossHP <= 1) textures[15][y * 16 + x] = 0x008888FF;
                else textures[15][y * 16 + x] = 0x00FFFFFF;
            }
            for (int dir = 0; dir < 8; dir++) {
                DWORD c = textures[30 + dir][y * 16 + x];
                if ((c & 0xFFFFFF) == RGB(255, 255, 0) || (c & 0xFFFFFF) == RGB(255, 136, 0)) {
                    textures[30 + dir][y * 16 + x] = (m_breathe > 0) ? RGB(255, 136, 0) : RGB(255, 255, 0);
                }
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

            // Magma Wall (t=27) - Pulsating Heat Veins
            int vein = ((x * 17 + y * 29 + (int)(animFrameCount * 0.5f)) % 20);
            if (vein < 3) textures[27][y * 16 + x] = 0x0000FFFF; // Radiant orange
            else if (vein < 6) textures[27][y * 16 + x] = 0x000066FF; // Deep red
            else textures[27][y * 16 + x] = 0x00111122; // Dark obsidian

            // Save Shrine / Campfire (t=28) - Swirling Flame
            float flameDist = (float)sqrt((x - 7.5f)*(x - 7.5f) + (y - 6.0f + sin(animFrameCount*0.2f)*1.5f)*(y - 6.0f + sin(animFrameCount*0.2f)*1.5f));
            if (flameDist < 2.5f) textures[28][y * 16 + x] = 0x0000FFFF; // Gold
            else if (flameDist < 4.5f) textures[28][y * 16 + x] = 0x000088FF; // Amber aura
            else if (y >= 12) textures[28][y * 16 + x] = 0x00777777; // Pedestal
            else textures[28][y * 16 + x] = 0x00222233;

            // Torch Sconce (t=29) - Flickering Light Core
            float torchDist = (float)sqrt((x - 7.5f)*(x - 7.5f) + (y - 5.0f + sin((animFrameCount + x)*0.3f)*1.0f)*(y - 5.0f + sin((animFrameCount + x)*0.3f)*1.0f));
            if (torchDist < 2.5f) textures[29][y * 16 + x] = 0x0000FFFF; // Flame core
            else if (torchDist < 4.5f) textures[29][y * 16 + x] = 0x000088FF;
            else if (y >= 9 && (x == 7 || x == 8)) textures[29][y * 16 + x] = 0x00555555; // Iron bar
            else textures[29][y * 16 + x] = 0x00111111;

            // Descent Shaft (t=38) - Swirling Runes
            float shaftDist = (float)sqrt((x - 7.5f)*(x - 7.5f) + (y - 7.5f)*(y - 7.5f));
            float sAngle = (float)atan2(y - 7.5f, x - 7.5f) + animFrameCount * 0.08f;
            if (shaftDist < 2.5f) textures[38][y * 16 + x] = 0x00050505; // Dark void center
            else if (shaftDist < 5.5f && ((int)(shaftDist * 2.0f + sAngle * 2.0f) % 2 == 0)) textures[38][y * 16 + x] = 0x00FFFF00; // Cyan stairs
            else textures[38][y * 16 + x] = 0x00444455;

            // Illusionary Wall (t=39) - Ethereal Shimmering Phase Pulses
            float pPhase = (float)sin((animFrameCount * 0.15f) + (x + y * 0.5f));
            int isMortar39 = (y == 3 || y == 7 || y == 11 || y == 15);
            if (isMortar39) textures[39][y * 16 + x] = 0x00443355;
            else {
                int rune39 = ((x + y * 3) % 5 == 0);
                if (rune39 && pPhase > 0.2f) textures[39][y * 16 + x] = 0x00FFFF88;
                else if (pPhase > 0.0f) textures[39][y * 16 + x] = RGB(120 + (int)(pPhase*40), 60, 160 + (int)(pPhase*50));
                else textures[39][y * 16 + x] = RGB(80, 40, 110);
            }

            // Lore Tablet (t=40) - Pulsating Arcane Glyphs
            float tabletPulse = (float)sin(animFrameCount * 0.12f + (x * 0.4f));
            if (x == 0 || x == 15 || y == 0 || y >= 13) textures[40][y * 16 + x] = 0x00222222;
            else if ((x >= 3 && x <= 12) && (y >= 2 && y <= 11)) {
                int glyph = ((x * 7 + y * 13) % 4 == 0) || (x == 7) || (y == 4 || y == 8);
                if (glyph) {
                    if (tabletPulse > 0.2f) textures[40][y * 16 + x] = 0x00FFFF00;
                    else textures[40][y * 16 + x] = 0x0000D7FF;
                } else textures[40][y * 16 + x] = 0x00111822;
            } else textures[40][y * 16 + x] = 0x00333344;

            // Animated Void Wall (t=22)
            int noise = (rand() % 40) - 20;
            DWORD v_old = textures[22][y * 16 + x];
            int rv = (v_old & 0xFF) + noise; if (rv < 0) rv = 0; if (rv > 255) rv = 255;
            int bv = ((v_old >> 16) & 0xFF) + noise; if (bv < 0) bv = 0; if (bv > 255) bv = 255;
            textures[22][y * 16 + x] = RGB(rv, rv, bv);
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
    if (val == 0 || val == 2 || val == 3 || val == 5 || val == 6 || val == 7 || val == 8 || val == 9 || val == 10 || val == 11 || val == 12 || val == 13 || val == 14 || val == 15 || val == 16 || val == 17 || val == 19 || val == 25 || val == 26 || val == 28 || val == 29 || val == 38 || val == 39 || val == 40) return 1;
    if (val == 4) {
        if (keysHeld > 0) {
            keysHeld--;
            SetMapValue(x, y, 0);
            MessageBeep(MB_ICONEXCLAMATION);
            AddParticles(160.0f, 120.0f, RGB(0, 150, 255), 20);
            return 1;
        }
    }
    if (val == 1 || val == 20 || val == 21 || val == 22 || val == 27 || val == 39) {
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
                if (GetMapValue(x, y) == 2 || GetMapValue(x, y) == 38) { targetX = x; targetY = y; break; }
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
                if (tile == 0 || tile == 2 || tile == 3 || tile == 5 || tile == 6 || tile == 8 || tile == 9 || tile == 10 || tile == 11 || tile == 13 || tile == 14 || tile == 16 || tile == 17 || tile == 18 || tile == 19 || tile == 25 || tile == 26 || tile == 28 || tile == 29 || tile == 38 || tile == 39 || tile == 40 || (tile == 4 && keysHeld > 0)) {
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
    // Place Ancient Save Shrine (t=28) - 1 or 2 per floor
    for(int i=0; i < (w*h > 400 ? 2 : 1); i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) {
            mapRandom[rx][ry] = 28;
        }
    }
    // Place Torch Sconces (t=29) for dynamic lighting
    for(int i=0; i < w*h/35; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) {
            mapRandom[rx][ry] = 29;
        }
    }
    // Place Secret Illusionary Fake Walls (t=39) - 1 to 3 per floor
    int numFakeWalls = 1 + (w*h > 400 ? 2 : 1);
    for (int i=0; i < numFakeWalls; i++) {
        int rx = 2 + rand()%(w-4);
        int ry = 2 + rand()%(h-4);
        if (mapRandom[rx][ry] == 1) {
            mapRandom[rx][ry] = 39;
        }
    }
    // Place Ancient Dungeon Lore Tablets (t=40) - 1 to 2 per floor
    int numLoreTablets = 1 + (w*h > 400 ? 1 : 0);
    for (int i=0; i < numLoreTablets; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) {
            mapRandom[rx][ry] = 40;
        }
    }

    if (currentLevel >= 10 && currentLevel % 5 == 0 && currentLevel < 44) {
        int placedBoss = 0;
        while (!placedBoss) {
            int rx = 1 + rand()%(w-2);
            int ry = 1 + rand()%(h-2);
            if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY) && abs(rx - 1) + abs(ry - 1) > 8) {
                mapRandom[rx][ry] = 15;
                bossHP = (currentLevel >= 40) ? 4 : 3;
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

    // Descent Sublevel Shaft (t=38) on sublevel thresholds (every 5 levels)
    if (currentLevel % 5 == 4 && currentLevel < 44) {
        mapRandom[farX][farY] = 38;
    } else if (currentLevel == 44) {
        // Stage 45 Final Boss Lair: Minotaur Overlord
        bossHP = 5;
        for (int i = 14; i <= 26; i++) {
            for (int j = 14; j <= 26; j++) {
                mapRandom[i][j] = 0;
            }
        }
        mapRandom[20][20] = 15; // Minotaur Overlord Boss
        mapRandom[16][16] = 12; // Guard Minotaur 1
        mapRandom[24][24] = 12; // Guard Minotaur 2
        mapRandom[16][24] = 12; // Guard Minotaur 3
        mapRandom[20][16] = 28; // Ancient Save Shrine
        mapRandom[15][20] = 29; // Torch Sconce 1
        mapRandom[25][20] = 29; // Torch Sconce 2
        mapRandom[26][26] = 3;  // Overlord Key
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

void SaveCheckpoint() {
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
    torchTimer = 0;

    currentLevel++;
    hasCompass = (currentLevel < 6) ? 1 : 0;
    if (currentLevel > 44) {
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
        int s = 11 + 2 * (int)(((currentLevel - 10) * 16) / 34);
        if (s > 43) s = 43;
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

void ShowHelpDialog(HWND hwnd) {
    const char* helpMsg =
        "KMAZE - TACTICAL 3D DUNGEON DESCENT GUIDE\n"
        "=========================================\n\n"
        "[CONTROLS & MOVEMENT]\n"
        "  W / UP ARROW    : Move Forward\n"
        "  S / DOWN ARROW  : Move Backward\n"
        "  A / LEFT ARROW  : Turn Left\n"
        "  D / RIGHT ARROW : Turn Right\n"
        "  SHIFT           : Sprint Run\n"
        "  X / CTRL        : Stealth Crouch (Silences footsteps & cushions traps)\n"
        "  ENTER / SPACE   : Start Descent / Play Again\n\n"
        "[ACTIVE RELICS & ABILITIES]\n"
        "  P               : Pickaxe (Cleave walls, fake secrets, slay minotaurs)\n"
        "  C               : Pathfinder (Draws path to key & exit, 10s)\n"
        "  S / B           : Speed Shoes (Double velocity, 8s)\n"
        "  F               : Stun Spray (Freezes hunting minotaurs, 10s)\n"
        "  T               : Time Freeze (Halts dungeon entities, 10s)\n"
        "  V               : Save Checkpoint to file\n"
        "  L               : Load Checkpoint descent\n"
        "  E / I           : Export / Import Statistics\n"
        "  K               : Rebind Keys\n"
        "  F1 / H          : Show this Help Guide\n\n"
        "[LABYRINTH CODEX & TILES]\n"
        "  Green Portal    : Descent exit to next stage (45 total)\n"
        "  Cyan Stairwell  : Deep sublevel descent shaft\n"
        "  Gold Key & Gate : Locked doors require finding matching key\n"
        "  Save Shrine     : Restores item charges and creates checkpoint\n"
        "  Wall Torch      : Illuminates catacombs with radiant glow (15s)\n"
        "  Illusion Wall   : Phantasmal fake wall hide secrets (+150 Score)\n"
        "  Lore Tablet     : Ancient wisdom and relic drops\n"
        "  Lava / Spikes   : Traps (-50% damage when in Stealth Crouch [X]!)\n"
        "  Minotaur Boss   : Guards 45th descent chamber (Pickaxe/Stun to fight)\n\n"
        "Tip: Use Stealth Crouch [X] to sneak past hunting minotaurs undetected!";
    MessageBoxA(hwnd, helpMsg, "KMaze Help & Dungeon Codex", MB_OK | MB_ICONINFORMATION);
}

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
            if (torchTimer > 0) torchTimer -= 30;
            if (damageFlinchTimer > 0) damageFlinchTimer--;
            if (screenShakeTimer > 0) {
                screenShakeTimer--;
                if (screenShakeTimer == 0) {
                    screenShakeIntensity = 0.0f;
                    screenShakeMaxTimer = 0;
                }
            }
            if (msgTimer > 0) msgTimer--;
            
            if (muzzleFlashTimer > 0) muzzleFlashTimer--;
            if (recoilOffset > 0) recoilOffset -= 2;
            UpdateShockwaves();

            float moveSpeed = 0.1f;
            float rotSpeed = 0.05f;
            if (speedShoesTimer > 0 || speedBoost) moveSpeed *= 2.0f;
            if (isCrouching) moveSpeed *= 0.5f;
            
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
                            
                            int distP = abs((int)pX - mx) + abs((int)pY - my);
                            int detectRadius = (mtype == 15) ? (isCrouching ? 4 : 12) : (isCrouching ? 2 : 8);
                            
                            int mdx = 0, mdy = 0;
                            if (distP <= detectRadius) {
                                if ((int)pX > mx) mdx = 1;
                                else if ((int)pX < mx) mdx = -1;
                                if ((int)pY > my) mdy = 1;
                                else if ((int)pY < my) mdy = -1;
                            } else {
                                int rDir = rand() % 4;
                                if (rDir == 0) mdx = 1;
                                else if (rDir == 1) mdx = -1;
                                else if (rDir == 2) mdy = 1;
                                else if (rDir == 3) mdy = -1;
                            }
                            
                            if (mdx != 0 && GetMapValue(mx + mdx, my) == 0) {
                                SetMapValue(mx, my, 0);
                                SetMapValue(mx + mdx, my, mtype);
                                minotaurFacingDir[mx + mdx][my] = (mdx > 0) ? 0 : 4;
                                mx += mdx;
                            } else if (mdy != 0 && GetMapValue(mx, my + mdy) == 0) {
                                SetMapValue(mx, my, 0);
                                SetMapValue(mx, my + mdy, mtype);
                                minotaurFacingDir[mx][my + mdy] = (mdy > 0) ? 2 : 6;
                                my += mdy;
                            }
                            
                            if (mx == (int)pX && my == (int)pY) {
                                MessageBeep(MB_ICONHAND);
                                damageFlinchTimer = 20;
                                AddScreenShake(30, 0.5f);
                                AddShockwave(pX, pY);
                                score = (score >= 100) ? score - 100 : 0;
                                if (mtype == 15) {
                                    pX = 1.5f; pY = 1.5f;
                                    strcpy(msgText, "Trampled by Minotaur Boss!");
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
                if (GetAsyncKeyState(keyBinds.crouch) & 0x8000 || GetAsyncKeyState(VK_CONTROL) & 0x8000) {
                    isCrouching = !isCrouching;
                    if (isCrouching) {
                        strcpy(msgText, "Stealth Crouch Active (Sneaking / Silenced)!");
                        msgTimer = 60;
                        MessageBeep(MB_OK);
                        AddParticles(160.0f, 120.0f, RGB(0, 255, 150), 20);
                    } else {
                        strcpy(msgText, "Standing Stance.");
                        msgTimer = 40;
                    }
                    activeKeyCooldown = 250;
                }
                if (GetAsyncKeyState(keyBinds.pickaxe) & 0x8000) {
                    if (hasPickaxe > 0) {
                        int tx = (int)(pX + dX * 0.8f);
                        int ty = (int)(pY + dY * 0.8f);
                        int tVal = GetMapValue(tx, ty);
                        if (tVal == 1 || tVal == 7 || tVal == 20 || tVal == 21 || tVal == 22 || tVal == 27 || tVal == 39) {
                            hasPickaxe--;
                            SetMapValue(tx, ty, 0);
                            MessageBeep(MB_OK);
                            if (tVal == 39) {
                                score += 150;
                                AddParticles(160.0f, 120.0f, RGB(180, 50, 255), 45);
                                strcpy(msgText, "Illusionary Wall Shattered! (+150 Score)");
                            } else {
                                AddParticles(160.0f, 120.0f, RGB(180, 100, 50), 40);
                                strcpy(msgText, "Wall Broken!");
                            }
                            msgTimer = 60;
                            activeKeyCooldown = 300;
                            recoilOffset = 15;
                            muzzleFlashTimer = 5;
                            AddScreenShake(15, 0.2f);
                            AddShockwave(tx + 0.5f, ty + 0.5f);
                        } else if (tVal == 12) {
                            hasPickaxe--;
                            SetMapValue(tx, ty, 0);
                            score += 200;
                            MessageBeep(MB_OK);
                            AddParticles(160.0f, 120.0f, RGB(255, 0, 0), 40);
                            strcpy(msgText, "Minotaur Slain!");
                            msgTimer = 60;
                            activeKeyCooldown = 300;
                            recoilOffset = 15;
                            muzzleFlashTimer = 5;
                            AddScreenShake(20, 0.3f);
                            AddShockwave(tx + 0.5f, ty + 0.5f);
                        } else if (tVal == 15) {
                            hasPickaxe--;
                            bossHP--;
                            AddParticles(160.0f, 120.0f, RGB(255, 215, 0), 50);
                            recoilOffset = 15;
                            muzzleFlashTimer = 5;
                            AddScreenShake(25, 0.4f);
                            AddShockwave(tx + 0.5f, ty + 0.5f);
                            if (bossHP <= 0) {
                                SetMapValue(tx, ty, 3);
                                score += 1000;
                                strcpy(msgText, "Minotaur Overlord Slain! Key Dropped!");
                            } else {
                                strcpy(msgText, "Minotaur Boss Hit!");
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
                if ((GetAsyncKeyState(keyBinds.speed) & 0x8000) || (GetAsyncKeyState('B') & 0x8000)) {
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
                        recoilOffset = 10;
                        muzzleFlashTimer = 8;
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
                    SaveCheckpoint();
                    strcpy(msgText, "Game Saved to Checkpoint!");
                    msgTimer = 60;
                    activeKeyCooldown = 1000;
                    MessageBeep(MB_OK);
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
                        strcpy(msgText, "Checkpoint / Save Loaded!");
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
            if ((GetAsyncKeyState(keyBinds.up) & 0x8000) || (GetAsyncKeyState('W') & 0x8000)) {
                if (TryMove((int)(pX + dX * moveSpeed), (int)pY)) pX += dX * moveSpeed;
                if (TryMove((int)pX, (int)(pY + dY * moveSpeed))) pY += dY * moveSpeed;
            }
            if ((GetAsyncKeyState(keyBinds.down) & 0x8000) || (GetAsyncKeyState('S') & 0x8000)) {
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
                int trapDmg = isCrouching ? 25 : 50;
                MessageBeep(MB_ICONHAND);
                damageFlinchTimer = 20;
                AddScreenShake(20, 0.3f);
                AddShockwave(pX, pY);
                score = (score >= trapDmg) ? score - trapDmg : 0;
                pX = 1.5f; pY = 1.5f;
                AddParticles(160.0f, 120.0f, RGB(255, 50, 0), 40);
                if (isCrouching) {
                    strcpy(msgText, "Crouch Light Step Cushioned Lava Trap! (-25 Score)");
                } else {
                    strcpy(msgText, "Burnt by Lava Trap! (-50 Score)");
                }
                msgTimer = 60;
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
                damageFlinchTimer = 20;
                AddScreenShake(30, 0.5f);
                AddShockwave(pX, pY);
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
                damageFlinchTimer = 20;
                AddScreenShake(35, 0.6f);
                AddShockwave(pX, pY);
                score = (score >= 150) ? score - 150 : 0;
                pX = 1.5f; pY = 1.5f;
                strcpy(msgText, "Attacked by Minotaur Boss!"); msgTimer = 60;
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
                    int trapDmg = isCrouching ? 35 : 75;
                    MessageBeep(MB_ICONHAND);
                    damageFlinchTimer = 20;
                    AddScreenShake(20, 0.4f);
                    AddShockwave(pX, pY);
                    score = (score >= trapDmg) ? score - trapDmg : 0;
                    pX = 1.5f; pY = 1.5f;
                    AddParticles(160.0f, 120.0f, RGB(255, 0, 0), 40);
                    if (isCrouching) {
                        strcpy(msgText, "Stealth Stance Softened Spike Trap! (-35 Score)");
                    } else {
                        strcpy(msgText, "Impaled by Spike Trap! (-75 Score)");
                    }
                    msgTimer = 60;
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
            } else if (curVal == 28) { // Save Shrine Checkpoint
                checkpointLevel = currentLevel;
                checkpointScore = score;
                checkpointPX = pX;
                checkpointPY = pY;
                checkpointKeys = keysHeld;
                checkpointPickaxe = hasPickaxe + 1;
                checkpointPathfinder = pathfinderCharges + 1;
                checkpointSpeed = speedShoesCharges + 1;
                checkpointStun = stunSprayCharges + 1;
                checkpointFreeze = timeFreezeCharges + 1;
                checkpointMapW = curRandW;
                checkpointMapH = curRandH;
                for (int i = 0; i < 45; i++) for (int j = 0; j < 45; j++) checkpointMap[i][j] = GetMapValue(i, j);
                
                score += 300;
                hasPickaxe = checkpointPickaxe;
                pathfinderCharges = checkpointPathfinder;
                speedShoesCharges = checkpointSpeed;
                stunSprayCharges = checkpointStun;
                timeFreezeCharges = checkpointFreeze;
                SaveCheckpoint();
                
                MessageBeep(MB_ICONASTERISK);
                AddParticles(160.0f, 120.0f, RGB(255, 215, 0), 35);
                strcpy(msgText, "Ancient Save Shrine Activated! Checkpoint Saved!");
                msgTimer = 90;
            } else if (curVal == 29) { // Torch Sconce / Lantern
                torchTimer = 15000;
                score += 50;
                MessageBeep(MB_ICONASTERISK);
                AddParticles(160.0f, 120.0f, RGB(255, 180, 50), 20);
                strcpy(msgText, "Ignited Wall Torch! Radiant Glow (+15s)!");
                msgTimer = 60;
            } else if (curVal == 38) { // Descent Sublevel Stairwell
                score += 250;
                hasPickaxe++;
                pathfinderCharges++;
                speedShoesCharges++;
                stunSprayCharges++;
                timeFreezeCharges++;
                MessageBeep(MB_ICONASTERISK);
                AddParticles(160.0f, 120.0f, RGB(0, 255, 200), 40);
                strcpy(msgText, "Descended into Dungeon Depths! (+250 Score)");
                msgTimer = 90;
                NextLevel();
            } else if (curVal == 39) { // Secret Illusionary Fake Wall
                SetMapValue((int)pX, (int)pY, 0);
                score += 150;
                MessageBeep(MB_ICONASTERISK);
                AddParticles(160.0f, 120.0f, RGB(180, 50, 255), 45);
                AddShockwave(pX, pY);
                AddScreenShake(10, 0.15f);
                strcpy(msgText, "Discovered Secret Illusionary Wall! (+150 Score)");
                msgTimer = 90;
            } else if (curVal == 40) { // Ancient Dungeon Lore Tablet
                SetMapValue((int)pX, (int)pY, 0);
                score += 200;
                int rReward = rand() % 5;
                if (rReward == 0) { hasPickaxe++; }
                else if (rReward == 1) { pathfinderCharges++; }
                else if (rReward == 2) { speedShoesCharges++; }
                else if (rReward == 3) { stunSprayCharges++; }
                else { timeFreezeCharges++; }
                
                static const char* const loreQuotes[] = {
                    "Lore: 'Phantasmal walls shimmer softly to the discerning eye...'",
                    "Lore: 'Crouch [X] to silence footsteps and evade hunting beasts...'",
                    "Lore: 'Ancient Shrines restore item charges and preserve progress...'",
                    "Lore: 'Minotaurs detect running footsteps from 8 paces away...'",
                    "Lore: 'Descent shafts plunge deeper into the labyrinth depths...'",
                    "Lore: 'Pickaxes can cleave through cursed beasts and walls alike...'",
                    "Lore: 'Treading lightly in stealth stance cushions trap damage...'",
                    "Lore: 'Lava flows ignite the infernal depths beyond floor 40...'",
                    "Lore: 'The Time Freeze relic halts all monsters in their tracks...'",
                    "Lore: 'The Minotaur Overlord guards the exit on the 45th descent...'",
                    "Lore: 'Stun sprays blind monsters with concentrated frost mist...'",
                    "Lore: 'Hidden chambers behind phantoms store ancient miner tools...'"
                };
                int qIdx = rand() % 12;
                strcpy(msgText, loreQuotes[qIdx]);
                msgTimer = 120;
                MessageBeep(MB_ICONASTERISK);
                AddParticles(160.0f, 120.0f, RGB(0, 255, 220), 40);
                AddShockwave(pX, pY);
            }

            if ((GetAsyncKeyState(keyBinds.right) & 0x8000) || (GetAsyncKeyState('D') & 0x8000)) {
                float oldDX = dX;
                dX = dX * (float)cos(rotSpeed) - dY * (float)sin(rotSpeed);
                dY = oldDX * (float)sin(rotSpeed) + dY * (float)cos(rotSpeed);
                float oldPlaneX = planeX;
                planeX = planeX * (float)cos(rotSpeed) - planeY * (float)sin(rotSpeed);
                planeY = oldPlaneX * (float)sin(rotSpeed) + planeY * (float)cos(rotSpeed);
            }
            if ((GetAsyncKeyState(keyBinds.left) & 0x8000) || (GetAsyncKeyState('A') & 0x8000)) {
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
                if (damageFlinchTimer > 0) {
                    float skew = sin(damageFlinchTimer * 0.5f) * 0.2f;
                    drawPlaneX += skew * drawDY;
                    drawPlaneY -= skew * drawDX;
                }
                if (screenShakeTimer > 0) {
                    float p = (screenShakeMaxTimer > 0) ? ((float)screenShakeTimer / (float)screenShakeMaxTimer) : ((float)screenShakeTimer / 30.0f);
                    float mag = screenShakeIntensity * (p * p);
                    float oscSin = (float)sin(animFrameCount * 2.2f) * mag;
                    float oscCos = (float)cos(animFrameCount * 2.7f) * mag;
                    float tilt = (float)sin(animFrameCount * 1.5f) * mag * 0.18f;
                    drawPlaneX += tilt * drawDY + oscSin * 0.35f;
                    drawPlaneY += -tilt * drawDX + oscCos * 0.35f;
                    drawDX += oscSin * 0.22f;
                    drawDY += oscCos * 0.22f;
                }

                // 1. Ceiling & Floor Casting with Dynamic Multi-Source Point Lighting
                int horizon = isCrouching ? (H * 3 / 8) : (H / 2);
                for (int y = 0; y < H; y++) {
                    int isFloor = (y >= horizon);
                    int p = isFloor ? (y - horizon) : (horizon - y);
                    if (p == 0) p = 1;

                    float posZ = isFloor ? (isCrouching ? 0.35f * H : 0.5f * H) : (isCrouching ? 0.65f * H : 0.5f * H);
                    float rowDistance = posZ / p;

                    float rayDirX0 = drawDX - drawPlaneX;
                    float rayDirY0 = drawDY - drawPlaneY;
                    float rayDirX1 = drawDX + drawPlaneX;
                    float rayDirY1 = drawDY + drawPlaneY;

                    float floorStepX = rowDistance * (rayDirX1 - rayDirX0) / W;
                    float floorStepY = rowDistance * (rayDirY1 - rayDirY0) / W;

                    float floorX = drawPX + rowDistance * rayDirX0;
                    float floorY = drawPY + rowDistance * rayDirY0;

                    for (int x = 0; x < W; ++x) {
                        int cellX = (int)floor(floorX);
                        int cellY = (int)floor(floorY);
                        int tx = (int)(16.0f * (floorX - cellX)) & 15;
                        int ty = (int)(16.0f * (floorY - cellY)) & 15;

                        // Ambient by biome
                        float lightR = 0.12f, lightG = 0.12f, lightB = 0.12f;
                        if (currentLevel >= 40) { lightR = 0.22f; lightG = 0.10f; lightB = 0.06f; }
                        else if (currentLevel >= 30) { lightR = 0.10f; lightG = 0.05f; lightB = 0.15f; }
                        else if (currentLevel >= 20) { lightR = 0.08f; lightG = 0.14f; lightB = 0.20f; }
                        else if (currentLevel >= 10) { lightR = 0.06f; lightG = 0.18f; lightB = 0.08f; }

                        // Player torch point light
                        float distToPlayer = (float)sqrt((floorX - drawPX)*(floorX - drawPX) + (floorY - drawPY)*(floorY - drawPY));
                        float pRadius = (torchTimer > 0) ? 8.5f : ((pathfinderTimer > 0) ? 14.0f : ((currentLevel >= 20) ? 4.5f : 7.0f));
                        if (distToPlayer < pRadius) {
                            float att = (1.0f - distToPlayer / pRadius);
                            float flicker = 1.0f + 0.12f * (float)sin(animFrameCount * 0.25f);
                            lightR += att * flicker * 1.0f;
                            lightG += att * flicker * 0.85f;
                            lightB += att * flicker * 0.65f;
                        }

                        // Torch sconces (tile 29) & Shrines (tile 28) in local area (scan +/- 3 cells)
                        int minCX = (cellX - 3 < 0) ? 0 : cellX - 3;
                        int maxCX = (cellX + 3 >= 45) ? 44 : cellX + 3;
                        int minCY = (cellY - 3 < 0) ? 0 : cellY - 3;
                        int maxCY = (cellY + 3 >= 45) ? 44 : cellY + 3;
                        for (int scx = minCX; scx <= maxCX; scx++) {
                            for (int scy = minCY; scy <= maxCY; scy++) {
                                int sv = GetMapValue(scx, scy);
                                if (sv == 29 || sv == 28 || sv == 38) {
                                    float sDist = (float)sqrt((floorX - (scx + 0.5f))*(floorX - (scx + 0.5f)) + (floorY - (scy + 0.5f))*(floorY - (scy + 0.5f)));
                                    if (sDist < 4.5f) {
                                        float satt = (1.0f - sDist / 4.5f) * (1.0f + 0.15f * (float)sin((animFrameCount + scx*5 + scy*7)*0.2f));
                                        if (sv == 28) { lightR += satt * 1.0f; lightG += satt * 0.9f; lightB += satt * 0.4f; }
                                        else if (sv == 38) { lightR += satt * 0.2f; lightG += satt * 1.0f; lightB += satt * 0.8f; }
                                        else { lightR += satt * 1.0f; lightG += satt * 0.65f; lightB += satt * 0.3f; }
                                    }
                                }
                            }
                        }

                        if (muzzleFlashTimer > 0 && distToPlayer < 4.5f) {
                            float intensity = (4.5f - distToPlayer) / 4.5f * (muzzleFlashTimer / 5.0f);
                            lightR += intensity * 1.5f; lightG += intensity * 1.2f; lightB += intensity * 0.5f;
                        }

                        float swDist = 0.0f;
                        for (int i = 0; i < shockwaveCount; i++) {
                            float dist = (float)sqrt((floorX - shockwaves[i].x)*(floorX - shockwaves[i].x) + (floorY - shockwaves[i].y)*(floorY - shockwaves[i].y));
                            float dInner = (float)fabs(dist - shockwaves[i].radius);
                            if (dInner < 0.45f) { swDist += (0.45f - dInner) * (shockwaves[i].life / 20.0f) * 1.5f; }
                            float dOuter = (float)fabs(dist - shockwaves[i].outerRadius);
                            if (dOuter < 0.65f) { swDist += (0.65f - dOuter) * (shockwaves[i].life / 20.0f) * 0.75f; }
                        }

                        floorX += floorStepX;
                        floorY += floorStepY;

                        int texIdx = isFloor ? 23 : 24;
                        DWORD srcCol = textures[texIdx][ty * 16 + tx];

                        float fR = (srcCol & 0xFF) * lightR + swDist * 255.0f;
                        float fG = ((srcCol >> 8) & 0xFF) * lightG + swDist * 100.0f;
                        float fB = ((srcCol >> 16) & 0xFF) * lightB + swDist * 100.0f;
                        if (fR > 255.0f) fR = 255.0f; if (fR < 0.0f) fR = 0.0f;
                        if (fG > 255.0f) fG = 255.0f; if (fG < 0.0f) fG = 0.0f;
                        if (fB > 255.0f) fB = 255.0f; if (fB < 0.0f) fB = 0.0f;

                        pBits[y * W + x] = RGB((BYTE)fR, (BYTE)fG, (BYTE)fB);
                    }
                }

                // 2. Wall Casting with Dynamic Multi-Source Point Lighting
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
                    
                    if (hit == 12 || hit == 15) {
                        float dx_m = pX - (mapX + 0.5f);
                        float dy_m = pY - (mapY + 0.5f);
                        float pAngle = (float)atan2(dy_m, dx_m);
                        float mAngle = minotaurFacingDir[mapX][mapY] * (float)(PI / 4.0f);
                        float diff = pAngle - mAngle;
                        while (diff < -PI) diff += 2 * PI;
                        while (diff > PI) diff -= 2 * PI;
                        int dir = (int)(floor(diff / (PI / 4.0f) + 0.5f)) % 8;
                        if (dir < 0) dir += 8;
                        if (hit == 12) hit = 30 + dir;
                    }

                    if (side == 0) perpWallDist = (sideDistX - deltaDistX);
                    else           perpWallDist = (sideDistY - deltaDistY);
                    
                    int lineHeight = (int)(H / perpWallDist);
                    int drawStart = -lineHeight / 2 + horizon;
                    int drawEnd = lineHeight / 2 + horizon;
                    int actualStart = (drawStart < 0) ? 0 : drawStart;
                    int actualEnd = (drawEnd >= H) ? H - 1 : drawEnd;
                    
                    float wallX;
                    if (side == 0) wallX = drawPY + perpWallDist * rayDY;
                    else           wallX = drawPX + perpWallDist * rayDX;
                    wallX -= (float)floor(wallX);
                    int texX = (int)(wallX * 16.0f) & 15;
                    
                    if (hit == 7) hit = 1;
                    if (hit < 1 || hit > 40) hit = 1;
                    if (hit == 1) {
                        if (currentLevel >= 40) hit = 27; // Magma Wall (Inferno)
                        else if (currentLevel >= 30) hit = 22; // Void Wall (Abyss)
                        else if (currentLevel >= 20) hit = 21; // Ice Wall (Frost)
                        else if (currentLevel >= 10) hit = 20; // Tech Wall (Cyber)
                    }
                    
                    float step = 16.0f / lineHeight;
                    float texPos = (actualStart - horizon + lineHeight / 2) * step;
                    
                    float sideMult = (side == 1) ? 0.75f : 1.0f;
                    float actualWX = (side == 0) ? (float)mapX : drawPX + perpWallDist * rayDX;
                    float actualWY = (side == 0) ? drawPY + perpWallDist * rayDY : (float)mapY;

                    // Dynamic multi-source point lighting on wall
                    float lightR = 0.12f, lightG = 0.12f, lightB = 0.12f;
                    if (currentLevel >= 40) { lightR = 0.22f; lightG = 0.10f; lightB = 0.06f; }
                    else if (currentLevel >= 30) { lightR = 0.10f; lightG = 0.05f; lightB = 0.15f; }
                    else if (currentLevel >= 20) { lightR = 0.08f; lightG = 0.14f; lightB = 0.20f; }
                    else if (currentLevel >= 10) { lightR = 0.06f; lightG = 0.18f; lightB = 0.08f; }

                    float distToPlayer = perpWallDist;
                    float pRadius = (torchTimer > 0) ? 8.5f : ((pathfinderTimer > 0) ? 14.0f : ((currentLevel >= 20) ? 4.5f : 7.0f));
                    if (distToPlayer < pRadius) {
                        float att = (1.0f - distToPlayer / pRadius);
                        float flicker = 1.0f + 0.12f * (float)sin(animFrameCount * 0.25f);
                        lightR += att * flicker * 1.0f;
                        lightG += att * flicker * 0.85f;
                        lightB += att * flicker * 0.65f;
                    }

                    int minCX = (mapX - 3 < 0) ? 0 : mapX - 3;
                    int maxCX = (mapX + 3 >= 45) ? 44 : mapX + 3;
                    int minCY = (mapY - 3 < 0) ? 0 : mapY - 3;
                    int maxCY = (mapY + 3 >= 45) ? 44 : mapY + 3;
                    for (int scx = minCX; scx <= maxCX; scx++) {
                        for (int scy = minCY; scy <= maxCY; scy++) {
                            int sv = GetMapValue(scx, scy);
                            if (sv == 29 || sv == 28 || sv == 38) {
                                float sDist = (float)sqrt((actualWX - (scx + 0.5f))*(actualWX - (scx + 0.5f)) + (actualWY - (scy + 0.5f))*(actualWY - (scy + 0.5f)));
                                if (sDist < 4.5f) {
                                    float satt = (1.0f - sDist / 4.5f) * (1.0f + 0.15f * (float)sin((animFrameCount + scx*5 + scy*7)*0.2f));
                                    if (sv == 28) { lightR += satt * 1.0f; lightG += satt * 0.9f; lightB += satt * 0.4f; }
                                    else if (sv == 38) { lightR += satt * 0.2f; lightG += satt * 1.0f; lightB += satt * 0.8f; }
                                    else { lightR += satt * 1.0f; lightG += satt * 0.65f; lightB += satt * 0.3f; }
                                }
                            }
                        }
                    }

                    if (muzzleFlashTimer > 0 && distToPlayer < 4.5f) {
                        float intensity = (4.5f - distToPlayer) / 4.5f * (muzzleFlashTimer / 5.0f);
                        lightR += intensity * 1.5f; lightG += intensity * 1.2f; lightB += intensity * 0.5f;
                    }

                    float swDist = 0.0f;
                    for (int i = 0; i < shockwaveCount; i++) {
                        float dist = (float)sqrt((actualWX - shockwaves[i].x)*(actualWX - shockwaves[i].x) + (actualWY - shockwaves[i].y)*(actualWY - shockwaves[i].y));
                        float dInner = (float)fabs(dist - shockwaves[i].radius);
                        if (dInner < 0.45f) { swDist += (0.45f - dInner) * (shockwaves[i].life / 20.0f) * 1.5f; }
                        float dOuter = (float)fabs(dist - shockwaves[i].outerRadius);
                        if (dOuter < 0.65f) { swDist += (0.65f - dOuter) * (shockwaves[i].life / 20.0f) * 0.75f; }
                    }
                    
                    for (int y = actualStart; y <= actualEnd; y++) {
                        int texY = (int)texPos & 15;
                        texPos += step;
                        
                        DWORD srcCol = textures[hit][texY * 16 + texX];
                        float tintR = 1.0f, tintG = 1.0f, tintB = 1.0f;
                        if (hit == 1 || hit == 20 || hit == 21 || hit == 22 || hit == 27) {
                            int varVal = (mapX * 13 + mapY * 37) % 3;
                            if (varVal == 0) { tintR = 0.85f; tintG = 0.9f; tintB = 1.0f; }
                            else if (varVal == 1) { tintR = 1.0f; tintG = 0.9f; tintB = 0.85f; }
                        }
                        
                        float fR = (srcCol & 0xFF) * sideMult * lightR * tintR + swDist * 255.0f;
                        float fG = ((srcCol >> 8) & 0xFF) * sideMult * lightG * tintG + swDist * 100.0f;
                        float fB = ((srcCol >> 16) & 0xFF) * sideMult * lightB * tintB + swDist * 100.0f;
                        if (fR > 255.0f) fR = 255.0f; if (fR < 0.0f) fR = 0.0f;
                        if (fG > 255.0f) fG = 255.0f; if (fG < 0.0f) fG = 0.0f;
                        if (fB > 255.0f) fB = 255.0f; if (fB < 0.0f) fB = 0.0f;
                        
                        pBits[y * W + x] = RGB((BYTE)fR, (BYTE)fG, (BYTE)fB);
                    }
                }
            }

            // 1. Ambient atmospheric motes on hdcMem
            for (int i = 0; i < MAX_MOTES; i++) {
                int mx = (int)ambientMotes[i].x;
                int my = (int)ambientMotes[i].y;
                if (mx >= 0 && mx < W && my >= 0 && my < H) {
                    COLORREF mCol = ambientMotes[i].isGold ? RGB(255, 204, 68) : RGB(160, 224, 255);
                    SetPixel(hdcMem, mx, my, mCol);
                    if (ambientMotes[i].size > 1.8f && mx + 1 < W && my + 1 < H) {
                        SetPixel(hdcMem, mx + 1, my, RGB(255, 255, 255));
                        SetPixel(hdcMem, mx, my + 1, mCol);
                    }
                }
            }

            // 2. Dual-tier 2D concentric shockwave rings on hdcMem
            for (int i = 0; i < shockwaveCount; i++) {
                if (shockwaves[i].life > 0) {
                    int rInner = (int)(shockwaves[i].radius * 24.0f);
                    int rOuter = (int)(shockwaves[i].outerRadius * 24.0f);
                    HPEN ringP1 = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                    HPEN ringP2 = CreatePen(PS_SOLID, 2, RGB(0, 229, 255));
                    HGDIOBJ oldPen = SelectObject(hdcMem, ringP1);
                    HGDIOBJ oldBrush = SelectObject(hdcMem, GetStockObject(NULL_BRUSH));
                    if (rInner > 0 && rInner < W) {
                        Ellipse(hdcMem, W/2 - rInner, H/2 - rInner, W/2 + rInner, H/2 + rInner);
                    }
                    SelectObject(hdcMem, ringP2);
                    if (rOuter > 0 && rOuter < W) {
                        Ellipse(hdcMem, W/2 - rOuter, H/2 - rOuter, W/2 + rOuter, H/2 + rOuter);
                    }
                    SelectObject(hdcMem, oldPen);
                    SelectObject(hdcMem, oldBrush);
                    DeleteObject(ringP1);
                    DeleteObject(ringP2);
                }
            }

            // 3. Multi-Layered Kinematic Particles (4 Layers)
            for (int i = 0; i < particleCount; i++) {
                int px = (int)particles[i].x;
                int py = (int)particles[i].y;
                if (px >= 0 && px < W && py >= 0 && py < H) {
                    float ratio = (float)particles[i].life / (float)particles[i].maxLife;
                    if (particles[i].layer == 0) {
                        // Layer 0: Incandescent needle sparks with velocity trails
                        HPEN sparkP = CreatePen(PS_SOLID, 1, particles[i].color);
                        HGDIOBJ op = SelectObject(hdcMem, sparkP);
                        MoveToEx(hdcMem, px, py, NULL);
                        LineTo(hdcMem, px - (int)(particles[i].vx * 2.2f), py - (int)(particles[i].vy * 2.2f));
                        SelectObject(hdcMem, op);
                        DeleteObject(sparkP);
                    } else if (particles[i].layer == 1) {
                        // Layer 1: Expanding buoyant plasma/smoke puffs
                        int sz = (int)(particles[i].size * (1.2f - ratio * 0.4f)) + 1;
                        HBRUSH sb = CreateSolidBrush(particles[i].color);
                        HPEN sp = CreatePen(PS_SOLID, 1, particles[i].color);
                        HGDIOBJ ob = SelectObject(hdcMem, sb);
                        HGDIOBJ op = SelectObject(hdcMem, sp);
                        Ellipse(hdcMem, px - sz, py - sz, px + sz, py + sz);
                        SelectObject(hdcMem, ob);
                        SelectObject(hdcMem, op);
                        DeleteObject(sb);
                        DeleteObject(sp);
                    } else if (particles[i].layer == 2) {
                        // Layer 2: Heavy kinematic tumbling debris & stone/crystal shards
                        int s = (int)particles[i].size;
                        if (s < 2) s = 2;
                        POINT pts[4];
                        float a = particles[i].rot;
                        float ca = (float)cos(a), sa = (float)sin(a);
                        if (particles[i].polyType == 0) {
                            pts[0].x = px + (int)(-sa * s); pts[0].y = py + (int)(ca * s);
                            pts[1].x = px + (int)(ca * s * 0.86f + sa * s * 0.5f); pts[1].y = py + (int)(sa * s * 0.86f - ca * s * 0.5f);
                            pts[2].x = px + (int)(-ca * s * 0.86f + sa * s * 0.5f); pts[2].y = py + (int)(-sa * s * 0.86f - ca * s * 0.5f);
                            HBRUSH rb = CreateSolidBrush(particles[i].color);
                            HPEN rp = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                            HGDIOBJ ob = SelectObject(hdcMem, rb);
                            HGDIOBJ op = SelectObject(hdcMem, rp);
                            Polygon(hdcMem, pts, 3);
                            SelectObject(hdcMem, ob); SelectObject(hdcMem, op);
                            DeleteObject(rb); DeleteObject(rp);
                        } else {
                            pts[0].x = px + (int)(-sa * s); pts[0].y = py + (int)(ca * s);
                            pts[1].x = px + (int)(ca * s * 0.7f); pts[1].y = py + (int)(sa * s * 0.7f);
                            pts[2].x = px + (int)(sa * s); pts[2].y = py + (int)(-ca * s);
                            pts[3].x = px + (int)(-ca * s * 0.7f); pts[3].y = py + (int)(-sa * s * 0.7f);
                            HBRUSH rb = CreateSolidBrush(particles[i].color);
                            HPEN rp = CreatePen(PS_SOLID, 1, particles[i].color);
                            HGDIOBJ ob = SelectObject(hdcMem, rb);
                            HGDIOBJ op = SelectObject(hdcMem, rp);
                            Polygon(hdcMem, pts, 4);
                            SelectObject(hdcMem, ob); SelectObject(hdcMem, op);
                            DeleteObject(rb); DeleteObject(rp);
                        }
                    } else {
                        // Layer 3: Radiant celebration energy stars
                        int rad = (int)(particles[i].size * (0.7f + 0.4f * sin(animFrameCount * 0.3f + particles[i].life)));
                        if (rad < 2) rad = 2;
                        POINT pts[8];
                        pts[0].x = px; pts[0].y = py - rad;
                        pts[1].x = px + rad / 3; pts[1].y = py - rad / 3;
                        pts[2].x = px + rad; pts[2].y = py;
                        pts[3].x = px + rad / 3; pts[3].y = py + rad / 3;
                        pts[4].x = px; pts[4].y = py + rad;
                        pts[5].x = px - rad / 3; pts[5].y = py + rad / 3;
                        pts[6].x = px - rad; pts[6].y = py;
                        pts[7].x = px - rad / 3; pts[7].y = py - rad / 3;
                        HBRUSH stb = CreateSolidBrush(particles[i].color);
                        HPEN stp = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                        HGDIOBJ ob = SelectObject(hdcMem, stb);
                        HGDIOBJ op = SelectObject(hdcMem, stp);
                        Polygon(hdcMem, pts, 8);
                        SelectObject(hdcMem, ob); SelectObject(hdcMem, op);
                        DeleteObject(stb); DeleteObject(stp);
                    }
                }
            }

            // 4. Perimeter Inlay Border & Traveling Specular Glint
            HPEN borderP = CreatePen(PS_SOLID, 1, RGB(0, 80, 100));
            HGDIOBJ oldBP = SelectObject(hdcMem, borderP);
            HGDIOBJ oldBB = SelectObject(hdcMem, GetStockObject(NULL_BRUSH));
            Rectangle(hdcMem, 3, 3, W - 3, H - 3);
            SelectObject(hdcMem, oldBP);
            SelectObject(hdcMem, oldBB);
            DeleteObject(borderP);

            // Traveling Specular Glint
            int perim = 2 * (W - 6 + H - 6);
            int glintDist = (animFrameCount * 4) % perim;
            int gx = 3, gy = 3;
            int wSide = W - 6, hSide = H - 6;
            if (glintDist < wSide) { gx = 3 + glintDist; gy = 3; }
            else if (glintDist < wSide + hSide) { gx = 3 + wSide; gy = 3 + (glintDist - wSide); }
            else if (glintDist < 2 * wSide + hSide) { gx = 3 + wSide - (glintDist - wSide - hSide); gy = 3 + hSide; }
            else { gx = 3; gy = 3 + hSide - (glintDist - 2 * wSide - hSide); }
            
            HBRUSH glintB = CreateSolidBrush(RGB(255, 255, 255));
            HPEN glintP = CreatePen(PS_SOLID, 1, RGB(0, 229, 255));
            SelectObject(hdcMem, glintB); SelectObject(hdcMem, glintP);
            Ellipse(hdcMem, gx - 2, gy - 2, gx + 3, gy + 3);
            DeleteObject(glintB); DeleteObject(glintP);

            // Ornate Corner Reticle L-Brackets with Pulsating Runic Diodes
            struct { int x, y, dx, dy; } corners[4] = {
                { 10, 10, 1, 1 },
                { W - 10, 10, -1, 1 },
                { 10, H - 10, 1, -1 },
                { W - 10, H - 10, -1, -1 }
            };
            HPEN bracketP = CreatePen(PS_SOLID, 2, RGB(0, 200, 230));
            HPEN notchP = CreatePen(PS_SOLID, 1, RGB(180, 240, 255));
            int dG = 160 + (int)(90.0f * sin(animFrameCount * 0.1f));
            if (dG > 255) dG = 255; if (dG < 0) dG = 0;
            HBRUSH diodeB = CreateSolidBrush(RGB(0, dG, 180));
            HPEN diodeP = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));

            for (int c = 0; c < 4; c++) {
                SelectObject(hdcMem, bracketP);
                MoveToEx(hdcMem, corners[c].x + corners[c].dx * 14, corners[c].y, NULL);
                LineTo(hdcMem, corners[c].x, corners[c].y);
                LineTo(hdcMem, corners[c].x, corners[c].y + corners[c].dy * 14);

                SelectObject(hdcMem, notchP);
                MoveToEx(hdcMem, corners[c].x + corners[c].dx * 7, corners[c].y - corners[c].dy * 2, NULL);
                LineTo(hdcMem, corners[c].x + corners[c].dx * 7, corners[c].y + corners[c].dy * 3);
                MoveToEx(hdcMem, corners[c].x - corners[c].dx * 2, corners[c].y + corners[c].dy * 7, NULL);
                LineTo(hdcMem, corners[c].x + corners[c].dx * 3, corners[c].y + corners[c].dy * 7);

                SelectObject(hdcMem, diodeB);
                SelectObject(hdcMem, diodeP);
                Ellipse(hdcMem, corners[c].x - 2, corners[c].y - 2, corners[c].x + 3, corners[c].y + 3);
            }
            DeleteObject(bracketP);
            DeleteObject(notchP);
            DeleteObject(diodeB);
            DeleteObject(diodeP);

            static float bobTime = 0.0f;
            static float lastPX = 1.5f, lastPY = 1.5f;
            static float swayX = 0.0f;
            static float lastDX = 1.0f, lastDY = 0.0f;
            float mDist = (float)sqrt((pX - lastPX)*(pX - lastPX) + (pY - lastPY)*(pY - lastPY));
            bobTime += mDist * 15.0f;
            float bobY = (float)sin(bobTime) * 5.0f;
            float bobX = (float)cos(bobTime * 0.5f) * 3.0f;
            float turnAmount = (float)atan2(dY, dX) - (float)atan2(lastDY, lastDX);
            while(turnAmount < -PI) turnAmount += 2*PI;
            while(turnAmount > PI) turnAmount -= 2*PI;
            swayX = swayX * 0.8f + turnAmount * 200.0f;
            lastPX = pX; lastPY = pY; lastDX = dX; lastDY = dY;

            // Held Equipment HUD
            if (gameState == 1) {
                if (hasCompass || pathfinderTimer > 0) {
                    int cx = 27 + (int)(bobX - swayX), cy = H - 27 + (int)bobY;
                    HBRUSH darkRimB = CreateSolidBrush(RGB(17, 17, 17));
                    HPEN darkRimP = CreatePen(PS_SOLID, 1, RGB(17, 17, 17));
                    SelectObject(hdcMem, darkRimB); SelectObject(hdcMem, darkRimP);
                    Ellipse(hdcMem, cx - 22, cy - 22, cx + 22, cy + 22);
                    DeleteObject(darkRimB); DeleteObject(darkRimP);

                    for (int r = 21; r >= 18; r--) {
                        int c = 112 + (21 - r) * 32; if (c > 255) c = 255;
                        HBRUSH rimB = CreateSolidBrush(RGB(c, c, c));
                        HPEN rimP = CreatePen(PS_SOLID, 1, RGB(c, c, c));
                        SelectObject(hdcMem, rimB); SelectObject(hdcMem, rimP);
                        Ellipse(hdcMem, cx - r, cy - r, cx + r, cy + r);
                        DeleteObject(rimB); DeleteObject(rimP);
                    }
                    for (int r = 18; r >= 14; r--) {
                        int g = 100 + (18 - r) * 25; if (g > 255) g = 255;
                        HBRUSH rimB = CreateSolidBrush(RGB(g, (int)(g*0.8f), (int)(g*0.3f)));
                        HPEN rimP = CreatePen(PS_SOLID, 1, RGB(g, (int)(g*0.8f), (int)(g*0.3f)));
                        SelectObject(hdcMem, rimB); SelectObject(hdcMem, rimP);
                        Ellipse(hdcMem, cx - r, cy - r, cx + r, cy + r);
                        DeleteObject(rimB); DeleteObject(rimP);
                    }
                    HBRUSH faceB = CreateSolidBrush(RGB(15, 30, 45));
                    HPEN faceP = CreatePen(PS_SOLID, 1, RGB(10, 20, 30));
                    SelectObject(hdcMem, faceB); SelectObject(hdcMem, faceP);
                    Ellipse(hdcMem, cx - 14, cy - 14, cx + 14, cy + 14);
                    DeleteObject(faceB); DeleteObject(faceP);
                    
                    int ex = 8, ey = 8;
                    for (int i = 0; i < 45; i++) {
                        for (int j = 0; j < 45; j++) {
                            if (GetMapValue(i, j) == 2 || GetMapValue(i, j) == 38) { ex = i; ey = j; break; }
                        }
                    }
                    float targetAngle = (float)atan2(ey - pY, ex - pX) - (float)atan2(dY, dX);
                    int nx = cx + (int)(cos(targetAngle) * 11);
                    int ny = cy + (int)(sin(targetAngle) * 11);
                    int nx2 = cx - (int)(cos(targetAngle) * 5);
                    int ny2 = cy - (int)(sin(targetAngle) * 5);
                    
                    HPEN needleP = CreatePen(PS_SOLID, 2, RGB(255, 50, 50));
                    SelectObject(hdcMem, needleP);
                    MoveToEx(hdcMem, cx, cy, NULL); LineTo(hdcMem, nx, ny);
                    DeleteObject(needleP);
                    
                    HPEN needleP2 = CreatePen(PS_SOLID, 2, RGB(200, 200, 200));
                    SelectObject(hdcMem, needleP2);
                    MoveToEx(hdcMem, cx, cy, NULL); LineTo(hdcMem, nx2, ny2);
                    DeleteObject(needleP2);

                    HBRUSH glassB = CreateSolidBrush(RGB(200, 220, 255));
                    HPEN glassP = CreatePen(PS_SOLID, 1, RGB(200, 220, 255));
                    SelectObject(hdcMem, glassB); SelectObject(hdcMem, glassP);
                    Ellipse(hdcMem, cx - 8, cy - 10, cx + 4, cy - 2);
                    DeleteObject(glassB); DeleteObject(glassP);
                }

                if (hasPickaxe > 0) {
                    int swing = (int)(sin(animFrameCount * 0.3f) * 4);
                    int bx = W - 45 + swing + (int)(bobX - swayX) + recoilOffset, by = H - 40 - swing + (int)bobY + (int)(recoilOffset * 1.5f);
                    
                    HPEN handleP = CreatePen(PS_SOLID, 3, RGB(139, 69, 19));
                    SelectObject(hdcMem, handleP);
                    MoveToEx(hdcMem, bx, by + 30, NULL); LineTo(hdcMem, bx + 20, by);
                    
                    HPEN headP = CreatePen(PS_SOLID, 3, RGB(180, 190, 200));
                    SelectObject(hdcMem, headP);
                    MoveToEx(hdcMem, bx + 10, by - 5, NULL); LineTo(hdcMem, bx + 28, by + 12);
                    DeleteObject(handleP); DeleteObject(headP);

                    // Specular sheen sweep highlight across pickaxe head
                    float sheenT = (float)((animFrameCount * 2) % 20) / 20.0f;
                    int sx = (bx + 10) + (int)(18.0f * sheenT);
                    int sy = (by - 5) + (int)(17.0f * sheenT);
                    HBRUSH sheenB = CreateSolidBrush(RGB(255, 255, 255));
                    HPEN sheenP = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                    SelectObject(hdcMem, sheenB); SelectObject(hdcMem, sheenP);
                    Ellipse(hdcMem, sx - 1, sy - 1, sx + 2, sy + 2);
                    DeleteObject(sheenB); DeleteObject(sheenP);

                    // Trailing sparks when swinging
                    if (abs(swing) > 2 && (rand() % 100 > 50) && particleCount < MAX_PARTICLES) {
                        particles[particleCount].x = (float)(bx + 28 + (rand() % 6 - 3));
                        particles[particleCount].y = (float)(by + 12 + (rand() % 6 - 3));
                        particles[particleCount].vx = ((rand() % 100) / 100.0f - 0.5f) * 2.0f - 1.0f;
                        particles[particleCount].vy = ((rand() % 100) / 100.0f - 0.5f) * 2.0f - 1.0f;
                        particles[particleCount].layer = 0;
                        particles[particleCount].life = 8;
                        particles[particleCount].maxLife = 8;
                        particles[particleCount].size = 1.5f;
                        particles[particleCount].color = RGB(255, 215, 0);
                        particleCount++;
                    }
                }
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
                    HBRUSH mMono = CreateSolidBrush(RGB(255, 50, 50));
                    HBRUSH mPick = CreateSolidBrush(RGB(150, 75, 0));
                    HBRUSH mStun = CreateSolidBrush(RGB(100, 200, 255));
                    HBRUSH mShrine = CreateSolidBrush(RGB(255, 215, 0));
                    HBRUSH mTorch = CreateSolidBrush(RGB(255, 140, 0));
                    HBRUSH mShaft = CreateSolidBrush(RGB(0, 255, 200));
                    HBRUSH mFake = CreateSolidBrush(RGB(120, 70, 150));
                    HBRUSH mLore = CreateSolidBrush(RGB(0, 229, 255));
                    
                    for (int i = 0; i < mmW; i++) {
                        for (int j = 0; j < mmH; j++) {
                            if (currentLevel >= 15 && pathfinderTimer <= 0) {
                                float distToP = (float)sqrt((i - pX)*(i - pX) + (j - pY)*(j - pY));
                                if (distToP > 5.5f) continue;
                            }
                            int v = GetMapValue(i, j);
                            HBRUSH b = mFloor;
                            if (isPathTile[i][j] && pathfinderTimer > 0) b = mPath;
                            else if (v == 1 || v == 7 || v == 20 || v == 21 || v == 22 || v == 27) b = mWall;
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
                            else if (v == 28) b = mShrine;
                            else if (v == 29) b = mTorch;
                            else if (v == 38) b = mShaft;
                            else if (v == 39) b = mFake;
                            else if (v == 40) b = mLore;
                            
                            RECT mr = {mmX + i*mmS, mmY + j*mmS, mmX + i*mmS + mmS, mmY + j*mmS + mmS};
                            FillRect(hdcMem, &mr, b);
                        }
                    }
                    RECT mr = {mmX + (int)pX*mmS, mmY + (int)pY*mmS, mmX + (int)pX*mmS + mmS, mmY + (int)pY*mmS + mmS};
                    FillRect(hdcMem, &mr, mPlayer);
                    
                    DeleteObject(mWall); DeleteObject(mExit); DeleteObject(mKey); DeleteObject(mDoor); DeleteObject(mFloor); DeleteObject(mPlayer); DeleteObject(mCoin);
                    DeleteObject(mTrap); DeleteObject(mComp); DeleteObject(mSpeed); DeleteObject(mTele); DeleteObject(mPath); DeleteObject(mBoss);
                    DeleteObject(mMono); DeleteObject(mPick); DeleteObject(mStun); DeleteObject(mShrine); DeleteObject(mTorch); DeleteObject(mShaft);
                    DeleteObject(mFake); DeleteObject(mLore);
                }
            }

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            StretchBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, hdcMem, 0, 0, W, H, SRCCOPY);
            
            int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
            int fontHeight = -MulDiv(12, dpi, 72);
            HFONT hFont = CreateFontA(fontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5 /*CLEARTYPE_QUALITY*/, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
            HGDIOBJ oldFont = SelectObject(hdc, hFont);
            
            SetBkMode(hdc, TRANSPARENT);
            if (gameState == 0) {
                const char* t1 = "KMAZE - DUNGEON DESCENT";
                const char* t2 = "Press ENTER to start";
                const char* t3 = "Press F1 or H for Help | L to Load Checkpoint";
                char t4[64]; wsprintfA(t4, "Games: %d Escapes: %d Best: %ds", totalGames, totalEscapes, (int)bestTime);
                
                SetTextColor(hdc, RGB(0, 0, 0));
                TextOutA(hdc, clientRect.right/2 - 90 + 2, clientRect.bottom/2 - 40 + 2, t1, lstrlenA(t1));
                TextOutA(hdc, clientRect.right/2 - 110 + 2, clientRect.bottom/2 - 10 + 2, t2, lstrlenA(t2));
                TextOutA(hdc, clientRect.right/2 - 170 + 2, clientRect.bottom/2 + 20 + 2, t3, lstrlenA(t3));
                TextOutA(hdc, clientRect.right/2 - 130 + 2, clientRect.bottom/2 + 50 + 2, t4, lstrlenA(t4));
                
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, clientRect.right/2 - 90, clientRect.bottom/2 - 40, t1, lstrlenA(t1));
                TextOutA(hdc, clientRect.right/2 - 110, clientRect.bottom/2 - 10, t2, lstrlenA(t2));
                TextOutA(hdc, clientRect.right/2 - 170, clientRect.bottom/2 + 20, t3, lstrlenA(t3));
                TextOutA(hdc, clientRect.right/2 - 130, clientRect.bottom/2 + 50, t4, lstrlenA(t4));
            } else if (gameState == 2) {
                DWORD elapsedSec = (endTime - startTime) / 1000;
                char t1[64]; wsprintfA(t1, "Escaped 45 Stages! Time: %ds", elapsedSec);
                char t2[64]; wsprintfA(t2, "Final Score: %d", score);
                const char* t3 = "Press R for Replay";
                const char* t4 = "ENTER to Restart";
                
                SetTextColor(hdc, RGB(0, 0, 0));
                TextOutA(hdc, clientRect.right/2 - 120 + 2, clientRect.bottom/2 - 40 + 2, t1, lstrlenA(t1));
                TextOutA(hdc, clientRect.right/2 - 70 + 2, clientRect.bottom/2 - 10 + 2, t2, lstrlenA(t2));
                TextOutA(hdc, clientRect.right/2 - 100 + 2, clientRect.bottom/2 + 20 + 2, t3, lstrlenA(t3));
                TextOutA(hdc, clientRect.right/2 - 90 + 2, clientRect.bottom/2 + 50 + 2, t4, lstrlenA(t4));
                
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, clientRect.right/2 - 120, clientRect.bottom/2 - 40, t1, lstrlenA(t1));
                TextOutA(hdc, clientRect.right/2 - 70, clientRect.bottom/2 - 10, t2, lstrlenA(t2));
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
                const char* biome = "Catacombs";
                if (currentLevel >= 40) biome = "Inferno Citadel";
                else if (currentLevel >= 30) biome = "Abyssal Depths";
                else if (currentLevel >= 20) biome = "Frost Caverns";
                else if (currentLevel >= 10) biome = "Cyber Labyrinth";

                char t1[128]; wsprintfA(t1, "Lvl:%d/45 [%s] K:%d P:%d C:%d S:%d F:%d T:%d [%s]", currentLevel + 1, biome, keysHeld, hasPickaxe, pathfinderCharges, speedShoesCharges, stunSprayCharges, timeFreezeCharges, isCrouching ? "CROUCH" : "STAND");
                char t2[128]; wsprintfA(t2, "Score:%d Time:%ds Torch:%ds", score, elapsedSec, torchTimer/1000);
                const char* t3 = "V:Save L:Load F1/H:Help";
                
                SetTextColor(hdc, RGB(0, 0, 0));
                TextOutA(hdc, 22, 22, t1, lstrlenA(t1)); TextOutA(hdc, 22, 52, t2, lstrlenA(t2));
                TextOutA(hdc, clientRect.right - 248, 22, t3, lstrlenA(t3));
                
                SetTextColor(hdc, isCrouching ? RGB(0, 255, 180) : RGB(255, 255, 255));
                TextOutA(hdc, 20, 20, t1, lstrlenA(t1)); TextOutA(hdc, 20, 50, t2, lstrlenA(t2));
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, clientRect.right - 250, 20, t3, lstrlenA(t3));
            }
            
            if (gameState == 4) {
                char kbText[64];
                int y = 80;
                const char* names[] = {"Up", "Down", "Left", "Right", "Pickaxe", "Pathfinder", "Speed", "Stun Spray", "Freeze", "Crouch"};
                int vals[] = {keyBinds.up, keyBinds.down, keyBinds.left, keyBinds.right, keyBinds.pickaxe, keyBinds.pathfinder, keyBinds.speed, keyBinds.stun, keyBinds.freeze, keyBinds.crouch};
                for (int i = 0; i < 10; i++) {
                    if (waitingForKey == i + 1) wsprintfA(kbText, "%s: ...", names[i]);
                    else wsprintfA(kbText, "%s: %c (%d)", names[i], (char)vals[i], vals[i]);
                    TextOutA(hdc, 80, y, kbText, lstrlenA(kbText));
                    y += 30;
                }
            }

            if (msgTimer > 0) {
                SetTextColor(hdc, RGB(0, 0, 0));
                TextOutA(hdc, clientRect.right/2 - 148 + 2, 82, msgText, lstrlenA(msgText));
                SetTextColor(hdc, RGB(255, 255, 0));
                TextOutA(hdc, clientRect.right/2 - 150, 80, msgText, lstrlenA(msgText));
            }

            // Active Items Legend HUD
            if (gameState == 1) {
                char itemText[160];
                wsprintfA(itemText, "[P]Break [C]Path:%ds [S]Speed:%ds [F]Stun:%ds [T]Freeze:%ds [X]Crouch:%s [V]Save [L]Load", pathfinderTimer/1000, speedShoesTimer/1000, stunSprayTimer/1000, timeFreezeTimer/1000, isCrouching ? "ON" : "OFF");
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
                if (idx >= 0 && idx < 10) waitingForKey = idx + 1;
            }
            break;
        }
        case WM_KEYDOWN: {
            if (wParam == VK_F1 || wParam == 'H') {
                ShowHelpDialog(hwnd);
                break;
            }
            if (gameState == 0 || gameState == 1) {
                if (wParam == 'E') ExportStats();
                if (wParam == 'I') ImportStats();
                if (wParam == 'K') { prevState = gameState; gameState = 4; }
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
                    if (waitingForKey == 9) keyBinds.freeze = (int)wParam;
                    if (waitingForKey == 10) keyBinds.crouch = (int)wParam;
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
    
    RECT wr = {0, 0, 800, 700};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    
    HWND hwnd = CreateWindowExA(0, "KMazeClass", "KMaze - Tactical 3D Dungeon Descent [Press F1/H for Guide]", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
