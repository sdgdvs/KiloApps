#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TIMER_ID 1
#define TIMER_INTERVAL 30 // ms (~33 FPS)

#define ID_BTN_LASER       101
#define ID_BTN_TRACTOR     102
#define ID_BTN_DAMPENER    103
#define ID_BTN_SCAN        104
#define ID_BTN_AUDIO       105
#define ID_BTN_HELP        106
#define ID_BTN_JETTISON    107
#define ID_BTN_SELL        108

#define SFX_NONE      0
#define SFX_COLLECT   1
#define SFX_FRACTURE  2
#define SFX_OVERHEAT  3
#define SFX_LASER_PULSE 4
#define SFX_BEEP      5

#define MAX_STARS 150
#define MAX_ASTEROIDS 24
#define MAX_ORE_CHUNKS 64
#define MAX_PARTICLES 128
#define MAX_FLOATING_TEXTS 16
#define MAX_LOG_ENTRIES 30

typedef struct {
    float x, y;
    float size;
    float brightness;
} Star;

typedef struct {
    float a, r;
} PolyVert;

typedef struct {
    char id[16];
    float x, y;
    float vx, vy;
    float radius;
    PolyVert verts[12];
    int numVerts;
    int oreType; // 0=Ferrum, 1=Silicates, 2=Platinum, 3=Void Quartz, 4=Dark Geode, 5=Derelict Scrap
    int richness;
    float hp, maxHp;
    float rot, rotSpeed;
    int active;
} Asteroid;

typedef struct {
    float x, y;
    float vx, vy;
    int oreType;
    int amount;
    float life;
    float rot, rotSpeed;
    int active;
} OreChunk;

typedef struct {
    float x, y;
    float vx, vy;
    COLORREF color;
    float life, decay;
    float size;
    int active;
} Particle;

typedef struct {
    char text[32];
    float x, y;
    float vy;
    COLORREF color;
    float life;
    int active;
} FloatingText;

typedef struct {
    char text[128];
    int type; // 0=system, 1=mining, 2=cargo, 3=warning, 4=critical, 5=success
} LogEntry;

// Ore Definitions
typedef struct {
    const char* name;
    COLORREF color;
    int value;
} OreInfo;

static const OreInfo ORE_DEFS[6] = {
    { "Ferrum Ore",     RGB(148, 163, 184), 15 },
    { "Silicates",      RGB(96, 165, 250),  25 },
    { "Platinum Vein",  RGB(226, 232, 240), 75 },
    { "Void Quartz",    RGB(192, 132, 252), 140 },
    { "Dark Geode",     RGB(244, 63, 94),   300 },
    { "Derelict Scrap", RGB(251, 191, 36),  50 }
};

// Game State
typedef struct {
    int credits;
    char sector[32];
    float hull, maxHull;
    float shield, maxShield;
    float fuel, maxFuel;
    float heat, maxHeat;
    float reactor;
    float o2;
    int cargoHold[6];
    int totalCargo;
    int maxCargo;
    int dampeners;
    int laserOverheated;
    
    // Ship Navigation
    float shipX, shipY;
    float shipVx, shipVy;
    float shipAngle; // radians
    int miningActive;
    int tractorActive;
    int thrusting;
    int reversing;
    int turningLeft;
    int turningRight;
    
    int selectedAstIndex;
    float radarAngle;
    int soundEnabled;
    int showHelp;
    
    Star stars[MAX_STARS];
    Asteroid asteroids[MAX_ASTEROIDS];
    OreChunk oreChunks[MAX_ORE_CHUNKS];
    Particle particles[MAX_PARTICLES];
    FloatingText texts[MAX_FLOATING_TEXTS];
    LogEntry logs[MAX_LOG_ENTRIES];
    int logCount;
} GameState;

static GameState g_state;
static HWND g_hwnd = NULL;
static HWND g_btnLaser, g_btnTractor, g_btnDampener, g_btnScan, g_btnAudio, g_btnHelp, g_btnJettison, g_btnSell;
static HFONT g_fontMono = NULL;
static HFONT g_fontMonoBold = NULL;
static HFONT g_fontSmall = NULL;
static HFONT g_fontHeader = NULL;

// Audio System
static volatile int g_currentSfx = SFX_NONE;
static HANDLE g_hSoundThread = NULL;

DWORD WINAPI SoundThreadProc(LPVOID lpParam) {
    while (1) {
        if (g_currentSfx != SFX_NONE && g_state.soundEnabled) {
            int sfx = g_currentSfx;
            g_currentSfx = SFX_NONE;
            if (sfx == SFX_COLLECT) {
                Beep(600, 40);
                Beep(880, 60);
            } else if (sfx == SFX_FRACTURE) {
                Beep(180, 80);
                Beep(110, 100);
            } else if (sfx == SFX_OVERHEAT) {
                Beep(320, 100);
                Beep(240, 120);
            } else if (sfx == SFX_BEEP) {
                Beep(700, 50);
            } else if (sfx == SFX_LASER_PULSE) {
                Beep(480, 30);
            }
        }
        Sleep(20);
    }
    return 0;
}

void TriggerSound(int sfx) {
    if (g_state.soundEnabled) {
        g_currentSfx = sfx;
    }
}

// Forward Declarations
void AddLog(const char* text, int type);
void AddFloatingText(const char* text, float x, float y, COLORREF color);
void AddSparks(float x, float y, COLORREF color, int count);
void SpawnOreChunk(float x, float y, int oreType, int amount);
void SpawnAsteroid(int index, int oreType);
void InitGame(void);
void UpdateGame(float dt);
void RenderGame(HDC hdc, RECT* clientRect);
int CalculateCargoValue(void);
void UpdateCargoTotal(void);

void AddLog(const char* text, int type) {
    if (g_state.logCount < MAX_LOG_ENTRIES) {
        strncpy(g_state.logs[g_state.logCount].text, text, 127);
        g_state.logs[g_state.logCount].text[127] = '\0';
        g_state.logs[g_state.logCount].type = type;
        g_state.logCount++;
    } else {
        for (int i = 0; i < MAX_LOG_ENTRIES - 1; i++) {
            g_state.logs[i] = g_state.logs[i + 1];
        }
        strncpy(g_state.logs[MAX_LOG_ENTRIES - 1].text, text, 127);
        g_state.logs[MAX_LOG_ENTRIES - 1].text[127] = '\0';
        g_state.logs[MAX_LOG_ENTRIES - 1].type = type;
    }
}

void AddFloatingText(const char* text, float x, float y, COLORREF color) {
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        if (!g_state.texts[i].active) {
            strncpy(g_state.texts[i].text, text, 31);
            g_state.texts[i].text[31] = '\0';
            g_state.texts[i].x = x;
            g_state.texts[i].y = y;
            g_state.texts[i].vy = -0.7f;
            g_state.texts[i].color = color;
            g_state.texts[i].life = 1.2f;
            g_state.texts[i].active = 1;
            break;
        }
    }
}

void AddSparks(float x, float y, COLORREF color, int count) {
    for (int k = 0; k < count; k++) {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!g_state.particles[i].active) {
                float angle = ((float)rand() / (float)RAND_MAX) * 6.28318f;
                float speed = 0.8f + (((float)rand() / (float)RAND_MAX) * 2.2f);
                g_state.particles[i].x = x;
                g_state.particles[i].y = y;
                g_state.particles[i].vx = (float)cos(angle) * speed;
                g_state.particles[i].vy = (float)sin(angle) * speed;
                g_state.particles[i].color = color;
                g_state.particles[i].life = 1.0f;
                g_state.particles[i].decay = 0.04f + (((float)rand() / (float)RAND_MAX) * 0.05f);
                g_state.particles[i].size = 2.0f;
                g_state.particles[i].active = 1;
                break;
            }
        }
    }
}

void SpawnOreChunk(float x, float y, int oreType, int amount) {
    for (int i = 0; i < MAX_ORE_CHUNKS; i++) {
        if (!g_state.oreChunks[i].active) {
            float angle = ((float)rand() / (float)RAND_MAX) * 6.28318f;
            float speed = 0.5f + (((float)rand() / (float)RAND_MAX) * 1.5f);
            g_state.oreChunks[i].x = x;
            g_state.oreChunks[i].y = y;
            g_state.oreChunks[i].vx = (float)cos(angle) * speed;
            g_state.oreChunks[i].vy = (float)sin(angle) * speed;
            g_state.oreChunks[i].oreType = oreType;
            g_state.oreChunks[i].amount = amount;
            g_state.oreChunks[i].life = 60.0f;
            g_state.oreChunks[i].rot = 0.0f;
            g_state.oreChunks[i].rotSpeed = (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.1f;
            g_state.oreChunks[i].active = 1;
            break;
        }
    }
}

void SpawnAsteroid(int index, int oreType) {
    Asteroid* ast = &g_state.asteroids[index];
    float angle = ((float)rand() / (float)RAND_MAX) * 6.28318f;
    float dist = 240.0f + (((float)rand() / (float)RAND_MAX) * 850.0f);
    
    sprintf(ast->id, "AST-%03d", 100 + index * 17 + (rand() % 80));
    ast->x = g_state.shipX + (float)cos(angle) * dist;
    ast->y = g_state.shipY + (float)sin(angle) * dist;
    ast->vx = (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.3f;
    ast->vy = (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.3f;
    ast->radius = 18.0f + (((float)rand() / (float)RAND_MAX) * 24.0f);
    ast->maxHp = (float)((int)(ast->radius * 3.5f));
    ast->hp = ast->maxHp;
    ast->oreType = (oreType >= 0 && oreType < 6) ? oreType : (rand() % 6);
    ast->richness = 40 + (rand() % 60);
    ast->rot = ((float)rand() / (float)RAND_MAX) * 6.28318f;
    ast->rotSpeed = (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.02f;
    ast->active = 1;
    
    ast->numVerts = 8 + (rand() % 4);
    for (int v = 0; v < ast->numVerts; v++) {
        ast->verts[v].a = ((float)v / (float)ast->numVerts) * 6.28318f;
        ast->verts[v].r = ast->radius * (0.75f + (((float)rand() / (float)RAND_MAX) * 0.35f));
    }
}

int CalculateCargoValue(void) {
    int total = 0;
    for (int i = 0; i < 6; i++) {
        total += g_state.cargoHold[i] * ORE_DEFS[i].value;
    }
    return total;
}

void UpdateCargoTotal(void) {
    int sum = 0;
    for (int i = 0; i < 6; i++) sum += g_state.cargoHold[i];
    g_state.totalCargo = sum;
}

void InitGame(void) {
    memset(&g_state, 0, sizeof(GameState));
    g_state.credits = 2500;
    strcpy(g_state.sector, "Belt Alpha-09");
    g_state.hull = 100.0f;
    g_state.maxHull = 100.0f;
    g_state.shield = 100.0f;
    g_state.maxShield = 100.0f;
    g_state.fuel = 100.0f;
    g_state.maxFuel = 100.0f;
    g_state.heat = 0.0f;
    g_state.maxHeat = 100.0f;
    g_state.reactor = 88.0f;
    g_state.o2 = 100.0f;
    g_state.maxCargo = 200;
    g_state.dampeners = 1;
    g_state.soundEnabled = 1;
    g_state.shipAngle = -1.57079f; // Facing UP
    g_state.selectedAstIndex = -1;
    
    // Seed Stars
    for (int i = 0; i < MAX_STARS; i++) {
        g_state.stars[i].x = (((float)rand() / (float)RAND_MAX) - 0.5f) * 4000.0f;
        g_state.stars[i].y = (((float)rand() / (float)RAND_MAX) - 0.5f) * 4000.0f;
        g_state.stars[i].size = 1.0f + (((float)rand() / (float)RAND_MAX) * 1.5f);
        g_state.stars[i].brightness = 0.3f + (((float)rand() / (float)RAND_MAX) * 0.7f);
    }
    
    // Seed Asteroids
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        SpawnAsteroid(i, i % 6);
    }
    
    AddLog("[SYSTEM] KStarDredge Mk-IV cockpit operational. Core reactor online.", 0);
    AddLog("[MINING] High-frequency mining laser ready. Aim at asteroids and hold [SPACE].", 1);
    AddLog("[TRACTOR] Tractor emitter active. Hold [T] to gather extracted mineral chunks.", 2);
}

void UpdateGame(float dt) {
    // Steering
    float rotAccel = 0.05f;
    if (g_state.turningLeft) g_state.shipAngle -= rotAccel;
    if (g_state.turningRight) g_state.shipAngle += rotAccel;
    
    // Propulsion
    float thrustPower = 0.12f;
    if (g_state.thrusting && g_state.fuel > 0.0f) {
        g_state.shipVx += (float)cos(g_state.shipAngle) * thrustPower;
        g_state.shipVy += (float)sin(g_state.shipAngle) * thrustPower;
        g_state.fuel = max(0.0f, g_state.fuel - 0.03f);
        
        // Thrust sparks
        float exAngle = g_state.shipAngle + 3.14159f + (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.4f;
        float exX = g_state.shipX - (float)cos(g_state.shipAngle) * 18.0f;
        float exY = g_state.shipY - (float)sin(g_state.shipAngle) * 18.0f;
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!g_state.particles[i].active) {
                g_state.particles[i].x = exX;
                g_state.particles[i].y = exY;
                g_state.particles[i].vx = (float)cos(exAngle) * (2.0f + ((float)rand() / (float)RAND_MAX) * 2.0f);
                g_state.particles[i].vy = (float)sin(exAngle) * (2.0f + ((float)rand() / (float)RAND_MAX) * 2.0f);
                g_state.particles[i].color = RGB(0, 240, 255);
                g_state.particles[i].life = 0.8f;
                g_state.particles[i].decay = 0.06f;
                g_state.particles[i].size = 2.5f;
                g_state.particles[i].active = 1;
                break;
            }
        }
    }
    
    if (g_state.reversing && g_state.fuel > 0.0f) {
        g_state.shipVx -= (float)cos(g_state.shipAngle) * (thrustPower * 0.5f);
        g_state.shipVy -= (float)sin(g_state.shipAngle) * (thrustPower * 0.5f);
        g_state.fuel = max(0.0f, g_state.fuel - 0.015f);
    }
    
    // Inertial Dampeners
    if (g_state.dampeners && !g_state.thrusting && !g_state.reversing) {
        g_state.shipVx *= 0.96f;
        g_state.shipVy *= 0.96f;
    }
    
    // Speed Cap
    float speed = (float)sqrt(g_state.shipVx * g_state.shipVx + g_state.shipVy * g_state.shipVy);
    if (speed > 6.0f) {
        g_state.shipVx = (g_state.shipVx / speed) * 6.0f;
        g_state.shipVy = (g_state.shipVy / speed) * 6.0f;
    }
    
    g_state.shipX += g_state.shipVx;
    g_state.shipY += g_state.shipVy;
    
    // Laser Overheat Logic
    if (g_state.miningActive && !g_state.laserOverheated) {
        g_state.heat += 0.5f;
        if (g_state.heat >= g_state.maxHeat) {
            g_state.laserOverheated = 1;
            g_state.heat = 100.0f;
            TriggerSound(SFX_OVERHEAT);
            AddLog("CRITICAL: Laser optics overheated! Emergency cooling cycle initiated.", 4);
        }
    } else {
        g_state.heat = max(0.0f, g_state.heat - 0.3f);
        if (g_state.laserOverheated && g_state.heat < 25.0f) {
            g_state.laserOverheated = 0;
            AddLog("Laser cooling cycle complete. Optics online.", 5);
        }
    }
    
    // Mining Laser Beam Impact
    if (g_state.miningActive && !g_state.laserOverheated) {
        float laserRange = 250.0f;
        float lx = g_state.shipX + (float)cos(g_state.shipAngle) * 20.0f;
        float ly = g_state.shipY + (float)sin(g_state.shipAngle) * 20.0f;
        float dirX = (float)cos(g_state.shipAngle);
        float dirY = (float)sin(g_state.shipAngle);
        
        for (int i = 0; i < MAX_ASTEROIDS; i++) {
            Asteroid* ast = &g_state.asteroids[i];
            if (!ast->active) continue;
            
            float toAstX = ast->x - lx;
            float toAstY = ast->y - ly;
            float proj = toAstX * dirX + toAstY * dirY;
            
            if (proj > 0.0f && proj < laserRange) {
                float perpX = toAstX - dirX * proj;
                float perpY = toAstY - dirY * proj;
                float distToBeam = (float)sqrt(perpX * perpX + perpY * perpY);
                
                if (distToBeam < ast->radius) {
                    float impactX = lx + dirX * proj;
                    float impactY = ly + dirY * proj;
                    AddSparks(impactX, impactY, RGB(0, 240, 255), 2);
                    ast->hp -= 0.6f;
                    
                    if ((rand() % 100) < 14) {
                        TriggerSound(SFX_FRACTURE);
                        SpawnOreChunk(impactX, impactY, ast->oreType, 1);
                        char buf[32];
                        sprintf(buf, "+1 %s", ORE_DEFS[ast->oreType].name);
                        AddFloatingText(buf, impactX, impactY - 10.0f, ORE_DEFS[ast->oreType].color);
                    }
                    
                    if (ast->hp <= 0.0f) {
                        TriggerSound(SFX_FRACTURE);
                        char buf[64];
                        sprintf(buf, "Asteroid %s shattered into rich mineral fragments!", ast->id);
                        AddLog(buf, 1);
                        AddSparks(ast->x, ast->y, RGB(245, 158, 11), 16);
                        for (int c = 0; c < 4 + (rand() % 3); c++) {
                            SpawnOreChunk(ast->x + (((float)rand() / (float)RAND_MAX) - 0.5f) * 20.0f,
                                          ast->y + (((float)rand() / (float)RAND_MAX) - 0.5f) * 20.0f,
                                          ast->oreType, 1);
                        }
                        ast->active = 0;
                        if (g_state.selectedAstIndex == i) g_state.selectedAstIndex = -1;
                        SpawnAsteroid(i, rand() % 6);
                    }
                    break;
                }
            }
        }
    }
    
    // Tractor Beam Logic
    if (g_state.tractorActive) {
        float tractorRange = 320.0f;
        for (int i = 0; i < MAX_ORE_CHUNKS; i++) {
            OreChunk* chunk = &g_state.oreChunks[i];
            if (!chunk->active) continue;
            
            float dx = g_state.shipX - chunk->x;
            float dy = g_state.shipY - chunk->y;
            float dist = (float)sqrt(dx * dx + dy * dy);
            
            if (dist < tractorRange && dist > 1.0f) {
                float pullForce = 2.2f / max(1.0f, dist * 0.05f);
                chunk->vx += (dx / dist) * pullForce;
                chunk->vy += (dy / dist) * pullForce;
                chunk->vx *= 0.94f;
                chunk->vy *= 0.94f;
            }
        }
    }
    
    // Ore Chunk Physics & Cargo Scooping
    float scoopRadius = 24.0f;
    for (int i = 0; i < MAX_ORE_CHUNKS; i++) {
        OreChunk* chunk = &g_state.oreChunks[i];
        if (!chunk->active) continue;
        
        chunk->x += chunk->vx;
        chunk->y += chunk->vy;
        chunk->rot += chunk->rotSpeed;
        chunk->life -= dt;
        
        float distToShip = (float)sqrt((g_state.shipX - chunk->x) * (g_state.shipX - chunk->x) +
                                      (g_state.shipY - chunk->y) * (g_state.shipY - chunk->y));
        if (distToShip < scoopRadius) {
            UpdateCargoTotal();
            if (g_state.totalCargo < g_state.maxCargo) {
                g_state.cargoHold[chunk->oreType] += chunk->amount;
                UpdateCargoTotal();
                TriggerSound(SFX_COLLECT);
                char buf[32];
                sprintf(buf, "+%dT %s", chunk->amount, ORE_DEFS[chunk->oreType].name);
                AddFloatingText(buf, g_state.shipX, g_state.shipY - 20.0f, ORE_DEFS[chunk->oreType].color);
                
                char logBuf[64];
                sprintf(logBuf, "Scooped %dT of %s into ore hold.", chunk->amount, ORE_DEFS[chunk->oreType].name);
                AddLog(logBuf, 2);
            } else {
                AddLog("WARNING: Cargo hold FULL! Cannot scoop ore.", 3);
            }
            chunk->active = 0;
            continue;
        }
        
        if (chunk->life <= 0.0f) {
            chunk->active = 0;
        }
    }
    
    // Asteroid Physics & Collisions with Ship
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        Asteroid* ast = &g_state.asteroids[i];
        if (!ast->active) continue;
        
        ast->x += ast->vx;
        ast->y += ast->vy;
        ast->rot += ast->rotSpeed;
        
        float dist = (float)sqrt((g_state.shipX - ast->x) * (g_state.shipX - ast->x) +
                                 (g_state.shipY - ast->y) * (g_state.shipY - ast->y));
        float minDist = ast->radius + 14.0f;
        if (dist < minDist && dist > 0.1f) {
            float angle = (float)atan2(g_state.shipY - ast->y, g_state.shipX - ast->x);
            g_state.shipVx += (float)cos(angle) * 1.5f;
            g_state.shipVy += (float)sin(angle) * 1.5f;
            
            if (g_state.shield > 0.0f) {
                g_state.shield = max(0.0f, g_state.shield - 8.0f);
            } else {
                g_state.hull = max(0.0f, g_state.hull - 5.0f);
            }
            AddSparks(g_state.shipX, g_state.shipY, RGB(239, 68, 68), 8);
            TriggerSound(SFX_FRACTURE);
            char buf[64];
            sprintf(buf, "COLLISION ALERT with %s! Shield deflected impact.", ast->id);
            AddLog(buf, 3);
        }
    }
    
    // Shield Regeneration
    if (g_state.shield < g_state.maxShield) {
        g_state.shield = min(g_state.maxShield, g_state.shield + 0.05f);
    }
    
    // Update Particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!g_state.particles[i].active) continue;
        g_state.particles[i].x += g_state.particles[i].vx;
        g_state.particles[i].y += g_state.particles[i].vy;
        g_state.particles[i].life -= g_state.particles[i].decay;
        if (g_state.particles[i].life <= 0.0f) g_state.particles[i].active = 0;
    }
    
    // Update Floating Text
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        if (!g_state.texts[i].active) continue;
        g_state.texts[i].y += g_state.texts[i].vy;
        g_state.texts[i].life -= 0.02f;
        if (g_state.texts[i].life <= 0.0f) g_state.texts[i].active = 0;
    }
    
    // Radar Sweep
    g_state.radarAngle += 0.05f;
    if (g_state.radarAngle > 6.28318f) g_state.radarAngle -= 6.28318f;
    
    // Auto Target Closest Asteroid
    if (g_state.selectedAstIndex < 0 || !g_state.asteroids[g_state.selectedAstIndex].active) {
        float minDist = 600.0f;
        int bestIdx = -1;
        for (int i = 0; i < MAX_ASTEROIDS; i++) {
            if (!g_state.asteroids[i].active) continue;
            float d = (float)sqrt((g_state.asteroids[i].x - g_state.shipX) * (g_state.asteroids[i].x - g_state.shipX) +
                                  (g_state.asteroids[i].y - g_state.shipY) * (g_state.asteroids[i].y - g_state.shipY));
            if (d < minDist) {
                minDist = d;
                bestIdx = i;
            }
        }
        g_state.selectedAstIndex = bestIdx;
    }
}

void DrawBar(HDC hdc, int x, int y, int w, int h, float pct, COLORREF fillColor, COLORREF bgColor, COLORREF borderCol) {
    RECT rcBg = { x, y, x + w, y + h };
    HBRUSH hBrBg = CreateSolidBrush(bgColor);
    FillRect(hdc, &rcBg, hBrBg);
    DeleteObject(hBrBg);
    
    HPEN hPen = CreatePen(PS_SOLID, 1, borderCol);
    HGDIOBJ oldPen = SelectObject(hdc, hPen);
    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, x, y, x + w, y + h);
    
    int fillW = (int)((w - 2) * max(0.0f, min(1.0f, pct)));
    if (fillW > 0) {
        RECT rcFill = { x + 1, y + 1, x + 1 + fillW, y + h - 1 };
        HBRUSH hBrFill = CreateSolidBrush(fillColor);
        FillRect(hdc, &rcFill, hBrFill);
        DeleteObject(hBrFill);
    }
    
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(hPen);
}

void RenderGame(HDC hdc, RECT* clientRect) {
    int totalW = clientRect->right - clientRect->left;
    int totalH = clientRect->bottom - clientRect->top;
    if (totalW <= 0 || totalH <= 0) return;
    
    // Layout geometry
    int topHeaderH = 34;
    int bottomCtrlH = 140;
    int leftPanelW = 220;
    int rightPanelW = 240;
    
    int mainY = topHeaderH;
    int mainH = totalH - topHeaderH - bottomCtrlH;
    int viewportX = leftPanelW;
    int viewportW = totalW - leftPanelW - rightPanelW;
    int viewportY = mainY;
    int viewportH = mainH;
    
    // 1. Top Header Bar
    RECT rcHeader = { 0, 0, totalW, topHeaderH };
    HBRUSH hBrHeader = CreateSolidBrush(RGB(15, 28, 63));
    FillRect(hdc, &rcHeader, hBrHeader);
    DeleteObject(hBrHeader);
    
    HPEN hPenBorder = CreatePen(PS_SOLID, 1, RGB(30, 58, 138));
    HGDIOBJ oldPen = SelectObject(hdc, hPenBorder);
    MoveToEx(hdc, 0, topHeaderH - 1, NULL);
    LineTo(hdc, totalW, topHeaderH - 1);
    
    SelectObject(hdc, g_fontHeader);
    SetTextColor(hdc, RGB(0, 240, 255));
    SetBkMode(hdc, TRANSPARENT);
    TextOutA(hdc, 12, 7, "KStarDredge - Mk-IV Asteroid Mining Barge", 41);
    
    SelectObject(hdc, g_fontMonoBold);
    char statBuf[128];
    UpdateCargoTotal();
    sprintf(statBuf, "SECTOR: %s   CREDITS: %d CR   HOLD: %d/%dT   HULL: %d%%",
            g_state.sector, g_state.credits, g_state.totalCargo, g_state.maxCargo, (int)g_state.hull);
    SetTextColor(hdc, RGB(240, 249, 255));
    RECT rcStats = { totalW - 480, 0, totalW - 12, topHeaderH };
    DrawTextA(hdc, statBuf, -1, &rcStats, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    
    // 2. Left Panel: Ship Systems & Telemetry
    RECT rcLeft = { 0, mainY, leftPanelW, mainY + mainH };
    HBRUSH hBrPanel = CreateSolidBrush(RGB(11, 19, 41));
    FillRect(hdc, &rcLeft, hBrPanel);
    
    MoveToEx(hdc, leftPanelW - 1, mainY, NULL);
    LineTo(hdc, leftPanelW - 1, mainY + mainH);
    
    // Panel Header
    RECT rcLeftHdr = { 0, mainY, leftPanelW, mainY + 22 };
    HBRUSH hBrSubHdr = CreateSolidBrush(RGB(15, 28, 63));
    FillRect(hdc, &rcLeftHdr, hBrSubHdr);
    SelectObject(hdc, g_fontMonoBold);
    SetTextColor(hdc, RGB(56, 189, 248));
    TextOutA(hdc, 8, mainY + 4, "BARGE TELEMETRY", 15);
    
    // System Meters
    int my = mainY + 30;
    int meterW = leftPanelW - 20;
    SelectObject(hdc, g_fontSmall);
    
    // Reactor
    SetTextColor(hdc, RGB(56, 189, 248));
    TextOutA(hdc, 10, my, "REACTOR OUTPUT (MW)", 19);
    char valBuf[32];
    sprintf(valBuf, "%d MW", (int)g_state.reactor);
    TextOutA(hdc, leftPanelW - 55, my, valBuf, (int)strlen(valBuf));
    DrawBar(hdc, 10, my + 14, meterW, 8, g_state.reactor / 100.0f, RGB(0, 240, 255), RGB(2, 6, 23), RGB(30, 58, 138));
    my += 28;
    
    // Shield
    SetTextColor(hdc, RGB(56, 189, 248));
    TextOutA(hdc, 10, my, "SHIELD INTEGRITY", 16);
    sprintf(valBuf, "%d%%", (int)g_state.shield);
    TextOutA(hdc, leftPanelW - 45, my, valBuf, (int)strlen(valBuf));
    DrawBar(hdc, 10, my + 14, meterW, 8, g_state.shield / 100.0f, RGB(16, 185, 129), RGB(2, 6, 23), RGB(30, 58, 138));
    my += 28;
    
    // Hull
    SetTextColor(hdc, RGB(56, 189, 248));
    TextOutA(hdc, 10, my, "HULL PLATING", 12);
    sprintf(valBuf, "%d%%", (int)g_state.hull);
    TextOutA(hdc, leftPanelW - 45, my, valBuf, (int)strlen(valBuf));
    COLORREF hullCol = (g_state.hull < 30.0f) ? RGB(239, 68, 68) : RGB(56, 189, 248);
    DrawBar(hdc, 10, my + 14, meterW, 8, g_state.hull / 100.0f, hullCol, RGB(2, 6, 23), RGB(30, 58, 138));
    my += 28;
    
    // Laser Heat
    SetTextColor(hdc, RGB(56, 189, 248));
    TextOutA(hdc, 10, my, "LASER CORE HEAT", 15);
    sprintf(valBuf, "%d%%", (int)g_state.heat);
    TextOutA(hdc, leftPanelW - 45, my, valBuf, (int)strlen(valBuf));
    COLORREF heatCol = (g_state.heat > 80.0f) ? RGB(239, 68, 68) : (g_state.heat > 50.0f ? RGB(245, 158, 11) : RGB(0, 240, 255));
    DrawBar(hdc, 10, my + 14, meterW, 8, g_state.heat / 100.0f, heatCol, RGB(2, 6, 23), RGB(30, 58, 138));
    my += 28;
    
    // Fuel
    SetTextColor(hdc, RGB(56, 189, 248));
    TextOutA(hdc, 10, my, "FUEL RESERVES", 13);
    sprintf(valBuf, "%d%%", (int)g_state.fuel);
    TextOutA(hdc, leftPanelW - 45, my, valBuf, (int)strlen(valBuf));
    DrawBar(hdc, 10, my + 14, meterW, 8, g_state.fuel / 100.0f, RGB(251, 191, 36), RGB(2, 6, 23), RGB(30, 58, 138));
    my += 28;
    
    // O2 / Life Support
    SetTextColor(hdc, RGB(56, 189, 248));
    TextOutA(hdc, 10, my, "O2 / LIFE SUPPORT", 17);
    sprintf(valBuf, "%d%%", (int)g_state.o2);
    TextOutA(hdc, leftPanelW - 45, my, valBuf, (int)strlen(valBuf));
    DrawBar(hdc, 10, my + 14, meterW, 8, g_state.o2 / 100.0f, RGB(16, 185, 129), RGB(2, 6, 23), RGB(30, 58, 138));
    my += 34;
    
    // System Status Summary Box
    RECT rcSysBox = { 10, my, leftPanelW - 10, my + 70 };
    HBRUSH hBrBox = CreateSolidBrush(RGB(3, 7, 18));
    FillRect(hdc, &rcSysBox, hBrBox);
    DeleteObject(hBrBox);
    FrameRect(hdc, &rcSysBox, (HBRUSH)GetStockObject(WHITE_BRUSH));
    
    SetTextColor(hdc, RGB(148, 163, 184));
    TextOutA(hdc, 14, my + 4, "THRUST DAMPENERS:", 17);
    SetTextColor(hdc, g_state.dampeners ? RGB(16, 185, 129) : RGB(245, 158, 11));
    TextOutA(hdc, leftPanelW - 75, my + 4, g_state.dampeners ? "ACTIVE" : "OFFLINE", g_state.dampeners ? 6 : 7);
    
    SetTextColor(hdc, RGB(148, 163, 184));
    TextOutA(hdc, 14, my + 20, "MINING LASER:", 13);
    SetTextColor(hdc, g_state.laserOverheated ? RGB(239, 68, 68) : (g_state.miningActive ? RGB(0, 240, 255) : RGB(2, 132, 199)));
    TextOutA(hdc, leftPanelW - 85, my + 20, g_state.laserOverheated ? "OVERHEAT" : (g_state.miningActive ? "FIRING" : "STANDBY"), g_state.laserOverheated ? 8 : (g_state.miningActive ? 6 : 7));
    
    SetTextColor(hdc, RGB(148, 163, 184));
    TextOutA(hdc, 14, my + 36, "TRACTOR BEAM:", 13);
    SetTextColor(hdc, g_state.tractorActive ? RGB(0, 240, 255) : RGB(2, 132, 199));
    TextOutA(hdc, leftPanelW - 85, my + 36, g_state.tractorActive ? "ENGAGED" : "STANDBY", g_state.tractorActive ? 7 : 7);
    
    float drain = 12.0f + (g_state.miningActive ? 28.0f : 0.0f) + (g_state.tractorActive ? 14.0f : 0.0f);
    SetTextColor(hdc, RGB(148, 163, 184));
    TextOutA(hdc, 14, my + 52, "POWER GRID DRAIN:", 17);
    char drnBuf[32];
    sprintf(drnBuf, "%.1f MW", drain);
    SetTextColor(hdc, RGB(240, 249, 255));
    TextOutA(hdc, leftPanelW - 65, my + 52, drnBuf, (int)strlen(drnBuf));
    
    // 3. Right Panel: Mineral Cargo Hold
    RECT rcRight = { totalW - rightPanelW, mainY, totalW, mainY + mainH };
    FillRect(hdc, &rcRight, hBrPanel);
    MoveToEx(hdc, totalW - rightPanelW, mainY, NULL);
    LineTo(hdc, totalW - rightPanelW, mainY + mainH);
    
    RECT rcRightHdr = { totalW - rightPanelW, mainY, totalW, mainY + 22 };
    FillRect(hdc, &rcRightHdr, hBrSubHdr);
    SelectObject(hdc, g_fontMonoBold);
    SetTextColor(hdc, RGB(56, 189, 248));
    TextOutA(hdc, totalW - rightPanelW + 8, mainY + 4, "MINERAL ORE HOLD", 16);
    char capBuf[32];
    sprintf(capBuf, "%d/%dT", g_state.totalCargo, g_state.maxCargo);
    TextOutA(hdc, totalW - 65, mainY + 4, capBuf, (int)strlen(capBuf));
    
    int cy = mainY + 28;
    for (int i = 0; i < 6; i++) {
        RECT rcItem = { totalW - rightPanelW + 8, cy, totalW - 8, cy + 34 };
        HBRUSH hBrItem = CreateSolidBrush(RGB(3, 7, 18));
        FillRect(hdc, &rcItem, hBrItem);
        DeleteObject(hBrItem);
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, ORE_DEFS[i].color);
        TextOutA(hdc, totalW - rightPanelW + 12, cy + 3, ORE_DEFS[i].name, (int)strlen(ORE_DEFS[i].name));
        
        SelectObject(hdc, g_fontSmall);
        char rateBuf[32];
        sprintf(rateBuf, "%d CR/T", ORE_DEFS[i].value);
        SetTextColor(hdc, RGB(148, 163, 184));
        TextOutA(hdc, totalW - rightPanelW + 12, cy + 18, rateBuf, (int)strlen(rateBuf));
        
        char holdBuf[32];
        sprintf(holdBuf, "%d T  (%d CR)", g_state.cargoHold[i], g_state.cargoHold[i] * ORE_DEFS[i].value);
        SetTextColor(hdc, RGB(240, 249, 255));
        RECT rcHold = { totalW - rightPanelW + 100, cy + 8, totalW - 14, cy + 26 };
        DrawTextA(hdc, holdBuf, -1, &rcHold, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
        
        cy += 38;
    }
    
    // Cargo Hold Summary
    int estVal = CalculateCargoValue();
    SelectObject(hdc, g_fontMonoBold);
    SetTextColor(hdc, RGB(240, 249, 255));
    TextOutA(hdc, totalW - rightPanelW + 10, cy + 4, "ESTIMATED VALUE:", 16);
    char estBuf[32];
    sprintf(estBuf, "%d CR", estVal);
    SetTextColor(hdc, RGB(245, 158, 11));
    RECT rcEst = { totalW - 100, cy + 4, totalW - 12, cy + 20 };
    DrawTextA(hdc, estBuf, -1, &rcEst, DT_RIGHT | DT_SINGLELINE);
    
    // 4. Center Viewport (Space & Asteroid Field)
    RECT rcViewport = { viewportX, viewportY, viewportX + viewportW, viewportY + viewportH };
    HBRUSH hBrSpace = CreateSolidBrush(RGB(1, 4, 10));
    FillRect(hdc, &rcViewport, hBrSpace);
    DeleteObject(hBrSpace);
    
    // Set viewport clipping
    HRGN hRgnClip = CreateRectRgn(viewportX, viewportY, viewportX + viewportW, viewportY + viewportH);
    SelectClipRgn(hdc, hRgnClip);
    
    int cx = viewportX + (viewportW / 2);
    int cyCenter = viewportY + (viewportH / 2);
    
    // Render Background Stars (Parallax)
    for (int i = 0; i < MAX_STARS; i++) {
        int sx = cx + (int)(g_state.stars[i].x - g_state.shipX * 0.15f);
        int sy = cyCenter + (int)(g_state.stars[i].y - g_state.shipY * 0.15f);
        if (sx >= viewportX && sx < viewportX + viewportW && sy >= viewportY && sy < viewportY + viewportH) {
            COLORREF starCol = RGB((int)(224 * g_state.stars[i].brightness),
                                   (int)(242 * g_state.stars[i].brightness),
                                   (int)(254 * g_state.stars[i].brightness));
            SetPixel(hdc, sx, sy, starCol);
            if (g_state.stars[i].size > 1.2f) {
                SetPixel(hdc, sx + 1, sy, starCol);
                SetPixel(hdc, sx, sy + 1, starCol);
            }
        }
    }
    
    // World Space Relative to Ship
    // Draw Tractor Wave Cone
    if (g_state.tractorActive) {
        float tractorRange = 320.0f;
        float angleL = g_state.shipAngle - 0.785f;
        float angleR = g_state.shipAngle + 0.785f;
        
        HPEN hPenTractor = CreatePen(PS_SOLID, 1, RGB(0, 180, 255));
        HGDIOBJ oldTrPen = SelectObject(hdc, hPenTractor);
        
        MoveToEx(hdc, cx, cyCenter, NULL);
        LineTo(hdc, cx + (int)(cos(angleL) * tractorRange), cyCenter + (int)(sin(angleL) * tractorRange));
        MoveToEx(hdc, cx, cyCenter, NULL);
        LineTo(hdc, cx + (int)(cos(angleR) * tractorRange), cyCenter + (int)(sin(angleR) * tractorRange));
        
        SelectObject(hdc, oldTrPen);
        DeleteObject(hPenTractor);
    }
    
    // Draw Mining Laser Beam
    if (g_state.miningActive && !g_state.laserOverheated) {
        float laserRange = 250.0f;
        int lx = cx + (int)(cos(g_state.shipAngle) * 20.0f);
        int ly = cyCenter + (int)(sin(g_state.shipAngle) * 20.0f);
        int endX = cx + (int)(cos(g_state.shipAngle) * laserRange);
        int endY = cyCenter + (int)(sin(g_state.shipAngle) * laserRange);
        
        HPEN hPenLaser = CreatePen(PS_SOLID, 3, RGB(0, 240, 255));
        HGDIOBJ oldLzrPen = SelectObject(hdc, hPenLaser);
        MoveToEx(hdc, lx, ly, NULL);
        LineTo(hdc, endX, endY);
        
        HPEN hPenLaserCore = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        SelectObject(hdc, hPenLaserCore);
        MoveToEx(hdc, lx, ly, NULL);
        LineTo(hdc, endX, endY);
        
        SelectObject(hdc, oldLzrPen);
        DeleteObject(hPenLaser);
        DeleteObject(hPenLaserCore);
    }
    
    // Draw Asteroids
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        Asteroid* ast = &g_state.asteroids[i];
        if (!ast->active) continue;
        
        int ax = cx + (int)(ast->x - g_state.shipX);
        int ay = cyCenter + (int)(ast->y - g_state.shipY);
        
        if (ax < viewportX - 100 || ax > viewportX + viewportW + 100 ||
            ay < viewportY - 100 || ay > viewportY + viewportH + 100) continue;
        
        POINT pts[12];
        for (int v = 0; v < ast->numVerts; v++) {
            float ca = ast->rot + ast->verts[v].a;
            pts[v].x = ax + (int)(cos(ca) * ast->verts[v].r);
            pts[v].y = ay + (int)(sin(ca) * ast->verts[v].r);
        }
        
        int isTarget = (g_state.selectedAstIndex == i);
        HPEN hPenAst = CreatePen(PS_SOLID, isTarget ? 2 : 1, isTarget ? RGB(0, 240, 255) : RGB(56, 189, 248));
        HBRUSH hBrAst = CreateSolidBrush(RGB(10, 21, 45));
        HGDIOBJ oldAstPen = SelectObject(hdc, hPenAst);
        HGDIOBJ oldAstBr = SelectObject(hdc, hBrAst);
        
        Polygon(hdc, pts, ast->numVerts);
        
        // Ore node center
        HBRUSH hBrOreNode = CreateSolidBrush(ORE_DEFS[ast->oreType].color);
        RECT rcNode = { ax - 3, ay - 3, ax + 4, ay + 4 };
        FillRect(hdc, &rcNode, hBrOreNode);
        DeleteObject(hBrOreNode);
        
        SelectObject(hdc, oldAstPen);
        SelectObject(hdc, oldAstBr);
        DeleteObject(hPenAst);
        DeleteObject(hBrAst);
        
        // Asteroid Label
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(148, 163, 184));
        char lbl[32];
        sprintf(lbl, "%s [%s]", ast->id, ORE_DEFS[ast->oreType].name);
        RECT rcLbl = { ax - 60, ay - (int)ast->radius - 14, ax + 60, ay - (int)ast->radius };
        DrawTextA(hdc, lbl, -1, &rcLbl, DT_CENTER | DT_SINGLELINE);
        
        if (isTarget) {
            HPEN hPenTarget = CreatePen(PS_DOT, 1, RGB(0, 240, 255));
            HGDIOBJ oldTPen = SelectObject(hdc, hPenTarget);
            HGDIOBJ oldTBr = SelectObject(hdc, GetStockObject(NULL_BRUSH));
            int bSize = (int)ast->radius + 8;
            Rectangle(hdc, ax - bSize, ay - bSize, ax + bSize, ay + bSize);
            SelectObject(hdc, oldTPen);
            SelectObject(hdc, oldTBr);
            DeleteObject(hPenTarget);
        }
    }
    
    // Draw Floating Ore Chunks
    for (int i = 0; i < MAX_ORE_CHUNKS; i++) {
        OreChunk* chunk = &g_state.oreChunks[i];
        if (!chunk->active) continue;
        
        int ox = cx + (int)(chunk->x - g_state.shipX);
        int oy = cyCenter + (int)(chunk->y - g_state.shipY);
        if (ox < viewportX || ox > viewportX + viewportW || oy < viewportY || oy > viewportY + viewportH) continue;
        
        POINT ptsChunk[4] = {
            { ox, oy - 4 },
            { ox + 4, oy },
            { ox, oy + 4 },
            { ox - 4, oy }
        };
        HBRUSH hBrChunk = CreateSolidBrush(ORE_DEFS[chunk->oreType].color);
        HPEN hPenChunk = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        HGDIOBJ oldChPen = SelectObject(hdc, hPenChunk);
        HGDIOBJ oldChBr = SelectObject(hdc, hBrChunk);
        
        Polygon(hdc, ptsChunk, 4);
        
        SelectObject(hdc, oldChPen);
        SelectObject(hdc, oldChBr);
        DeleteObject(hPenChunk);
        DeleteObject(hBrChunk);
    }
    
    // Draw Particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!g_state.particles[i].active) continue;
        int px = cx + (int)(g_state.particles[i].x - g_state.shipX);
        int py = cyCenter + (int)(g_state.particles[i].y - g_state.shipY);
        if (px >= viewportX && px < viewportX + viewportW && py >= viewportY && py < viewportY + viewportH) {
            SetPixel(hdc, px, py, g_state.particles[i].color);
            SetPixel(hdc, px + 1, py, g_state.particles[i].color);
        }
    }
    
    // Draw Floating Texts
    SelectObject(hdc, g_fontMonoBold);
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        if (!g_state.texts[i].active) continue;
        int tx = cx + (int)(g_state.texts[i].x - g_state.shipX);
        int ty = cyCenter + (int)(g_state.texts[i].y - g_state.shipY);
        if (tx >= viewportX && tx < viewportX + viewportW && ty >= viewportY && ty < viewportY + viewportH) {
            SetTextColor(hdc, g_state.texts[i].color);
            RECT rcTxt = { tx - 60, ty - 8, tx + 60, ty + 8 };
            DrawTextA(hdc, g_state.texts[i].text, -1, &rcTxt, DT_CENTER | DT_SINGLELINE);
        }
    }
    
    // Draw Dredge Ship (Cockpit Center)
    if (g_state.shield > 0.0f) {
        HPEN hPenShield = CreatePen(PS_SOLID, 1, RGB(56, 189, 248));
        HGDIOBJ oldShPen = SelectObject(hdc, hPenShield);
        HGDIOBJ oldShBr = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Ellipse(hdc, cx - 28, cyCenter - 28, cx + 28, cyCenter + 28);
        SelectObject(hdc, oldShPen);
        SelectObject(hdc, oldShBr);
        DeleteObject(hPenShield);
    }
    
    // Rotate Ship Vertices
    POINT shipLocal[6] = {
        { 22, 0 },
        { -8, -16 },
        { -20, -12 },
        { -16, 0 },
        { -20, 12 },
        { -8, 16 }
    };
    POINT shipWorld[6];
    float cosA = (float)cos(g_state.shipAngle);
    float sinA = (float)sin(g_state.shipAngle);
    for (int i = 0; i < 6; i++) {
        shipWorld[i].x = cx + (int)(shipLocal[i].x * cosA - shipLocal[i].y * sinA);
        shipWorld[i].y = cyCenter + (int)(shipLocal[i].x * sinA + shipLocal[i].y * cosA);
    }
    
    HPEN hPenShip = CreatePen(PS_SOLID, 2, RGB(0, 240, 255));
    HBRUSH hBrShip = CreateSolidBrush(RGB(17, 30, 56));
    HGDIOBJ oldSpPen = SelectObject(hdc, hPenShip);
    HGDIOBJ oldSpBr = SelectObject(hdc, hBrShip);
    Polygon(hdc, shipWorld, 6);
    SelectObject(hdc, oldSpPen);
    SelectObject(hdc, oldSpBr);
    DeleteObject(hPenShip);
    DeleteObject(hBrShip);
    
    // Center Cockpit Glass
    HBRUSH hBrGlass = CreateSolidBrush(RGB(56, 189, 248));
    int gx = cx + (int)(4.0f * cosA);
    int gy = cyCenter + (int)(4.0f * sinA);
    RECT rcGlass = { gx - 3, gy - 3, gx + 4, gy + 4 };
    FillRect(hdc, &rcGlass, hBrGlass);
    DeleteObject(hBrGlass);
    
    // HUD Telemetry Overlays
    SelectObject(hdc, g_fontSmall);
    SetTextColor(hdc, RGB(0, 240, 255));
    TextOutA(hdc, viewportX + 10, viewportY + 10, "RADAR SCANNER: ACTIVE (360)", 27);
    float spd = (float)sqrt(g_state.shipVx * g_state.shipVx + g_state.shipVy * g_state.shipVy);
    char hudBuf[64];
    sprintf(hudBuf, "DRIFT VELOCITY: %.2f km/s", spd);
    TextOutA(hdc, viewportX + 10, viewportY + 24, hudBuf, (int)strlen(hudBuf));
    sprintf(hudBuf, "SECTOR POS: X: %.1f | Y: %.1f", g_state.shipX * 0.1f, g_state.shipY * 0.1f);
    TextOutA(hdc, viewportX + 10, viewportY + 38, hudBuf, (int)strlen(hudBuf));
    
    // Radar Scope in Top Right of Viewport
    int radarR = 55;
    int rcRadarX = viewportX + viewportW - radarR - 15;
    int rcRadarY = viewportY + radarR + 15;
    
    HBRUSH hBrRadar = CreateSolidBrush(RGB(3, 8, 22));
    HPEN hPenRadar = CreatePen(PS_SOLID, 1, RGB(30, 58, 138));
    HGDIOBJ oldRdPen = SelectObject(hdc, hPenRadar);
    HGDIOBJ oldRdBr = SelectObject(hdc, hBrRadar);
    Ellipse(hdc, rcRadarX - radarR, rcRadarY - radarR, rcRadarX + radarR, rcRadarY + radarR);
    
    // Radar Range Rings
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Ellipse(hdc, rcRadarX - radarR / 2, rcRadarY - radarR / 2, rcRadarX + radarR / 2, rcRadarY + radarR / 2);
    
    // Radar Sweep Line
    HPEN hPenSweep = CreatePen(PS_SOLID, 1, RGB(0, 240, 255));
    SelectObject(hdc, hPenSweep);
    MoveToEx(hdc, rcRadarX, rcRadarY, NULL);
    LineTo(hdc, rcRadarX + (int)(cos(g_state.radarAngle) * radarR), rcRadarY + (int)(sin(g_state.radarAngle) * radarR));
    DeleteObject(hPenSweep);
    
    // Radar Blips for Asteroids
    float radarScale = 0.07f;
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!g_state.asteroids[i].active) continue;
        float bdx = (g_state.asteroids[i].x - g_state.shipX) * radarScale;
        float bdy = (g_state.asteroids[i].y - g_state.shipY) * radarScale;
        if (bdx * bdx + bdy * bdy < (radarR - 4) * (radarR - 4)) {
            COLORREF blipCol = (i == g_state.selectedAstIndex) ? RGB(0, 240, 255) : RGB(245, 158, 11);
            SetPixel(hdc, rcRadarX + (int)bdx, rcRadarY + (int)bdy, blipCol);
            SetPixel(hdc, rcRadarX + (int)bdx + 1, rcRadarY + (int)bdy, blipCol);
        }
    }
    
    // Center Ship Blip on Radar
    SetPixel(hdc, rcRadarX, rcRadarY, RGB(56, 189, 248));
    SetPixel(hdc, rcRadarX + 1, rcRadarY, RGB(56, 189, 248));
    
    SelectObject(hdc, oldRdPen);
    SelectObject(hdc, oldRdBr);
    DeleteObject(hPenRadar);
    DeleteObject(hBrRadar);
    
    // Target Lock Reticle Box on Bottom-Right of Viewport
    if (g_state.selectedAstIndex >= 0 && g_state.asteroids[g_state.selectedAstIndex].active) {
        Asteroid* target = &g_state.asteroids[g_state.selectedAstIndex];
        int tBoxW = 200;
        int tBoxH = 48;
        int tBoxX = viewportX + viewportW - tBoxW - 15;
        int tBoxY = viewportY + viewportH - tBoxH - 15;
        
        RECT rcTBox = { tBoxX, tBoxY, tBoxX + tBoxW, tBoxY + tBoxH };
        HBRUSH hBrTBox = CreateSolidBrush(RGB(11, 19, 41));
        FillRect(hdc, &rcTBox, hBrTBox);
        DeleteObject(hBrTBox);
        FrameRect(hdc, &rcTBox, (HBRUSH)GetStockObject(WHITE_BRUSH));
        
        SelectObject(hdc, g_fontMonoBold);
        SetTextColor(hdc, RGB(0, 240, 255));
        TextOutA(hdc, tBoxX + 6, tBoxY + 4, target->id, (int)strlen(target->id));
        
        float dist = (float)sqrt((target->x - g_state.shipX) * (target->x - g_state.shipX) +
                                 (target->y - g_state.shipY) * (target->y - g_state.shipY));
        char distBuf[32];
        sprintf(distBuf, "%d m", (int)dist);
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(148, 163, 184));
        RECT rcDist = { tBoxX + 100, tBoxY + 4, tBoxX + tBoxW - 6, tBoxY + 20 };
        DrawTextA(hdc, distBuf, -1, &rcDist, DT_RIGHT | DT_SINGLELINE);
        
        char oreStr[64];
        sprintf(oreStr, "ORE: %s (%d%%)", ORE_DEFS[target->oreType].name, target->richness);
        SetTextColor(hdc, RGB(240, 249, 255));
        TextOutA(hdc, tBoxX + 6, tBoxY + 18, oreStr, (int)strlen(oreStr));
        
        DrawBar(hdc, tBoxX + 6, tBoxY + 34, tBoxW - 12, 6, target->hp / target->maxHp, RGB(0, 240, 255), RGB(2, 6, 23), RGB(30, 58, 138));
    }
    
    // Restore clipping
    SelectClipRgn(hdc, NULL);
    DeleteObject(hRgnClip);
    
    // 5. Bottom Panel: Cockpit Controls & Event Terminal
    int botY = totalH - bottomCtrlH;
    RECT rcBottom = { 0, botY, totalW, totalH };
    HBRUSH hBrBot = CreateSolidBrush(RGB(11, 19, 41));
    FillRect(hdc, &rcBottom, hBrBot);
    DeleteObject(hBrBot);
    
    MoveToEx(hdc, 0, botY, NULL);
    LineTo(hdc, totalW, botY);
    
    // Control Section Header
    RECT rcCtrlHdr = { 0, botY, 320, botY + 20 };
    FillRect(hdc, &rcCtrlHdr, hBrSubHdr);
    SelectObject(hdc, g_fontMonoBold);
    SetTextColor(hdc, RGB(56, 189, 248));
    TextOutA(hdc, 10, botY + 3, "COCKPIT CONSOLE  [WASD / SPACE / T / Z]", 38);
    
    // Terminal Section Header
    RECT rcTermHdr = { 320, botY, totalW, botY + 20 };
    FillRect(hdc, &rcTermHdr, hBrSubHdr);
    MoveToEx(hdc, 320, botY, NULL);
    LineTo(hdc, 320, totalH);
    TextOutA(hdc, 330, botY + 3, "FLIGHT & DREDGE TERMINAL", 24);
    
    // Draw Terminal Event Logs
    int logBoxX = 330;
    int logBoxY = botY + 25;
    int logBoxW = totalW - logBoxX - 10;
    int logBoxH = bottomCtrlH - 35;
    
    RECT rcLogBox = { logBoxX, logBoxY, logBoxX + logBoxW, logBoxY + logBoxH };
    HBRUSH hBrLog = CreateSolidBrush(RGB(2, 5, 14));
    FillRect(hdc, &rcLogBox, hBrLog);
    DeleteObject(hBrLog);
    FrameRect(hdc, &rcLogBox, (HBRUSH)GetStockObject(WHITE_BRUSH));
    
    SelectObject(hdc, g_fontSmall);
    int visibleLogs = min(6, g_state.logCount);
    int startIdx = max(0, g_state.logCount - 6);
    int ly = logBoxY + 4;
    
    for (int i = startIdx; i < g_state.logCount; i++) {
        COLORREF logCol = RGB(148, 163, 184);
        if (g_state.logs[i].type == 0) logCol = RGB(56, 189, 248);
        else if (g_state.logs[i].type == 1) logCol = RGB(0, 240, 255);
        else if (g_state.logs[i].type == 2) logCol = RGB(192, 132, 252);
        else if (g_state.logs[i].type == 3) logCol = RGB(245, 158, 11);
        else if (g_state.logs[i].type == 4) logCol = RGB(239, 68, 68);
        else if (g_state.logs[i].type == 5) logCol = RGB(16, 185, 129);
        
        SetTextColor(hdc, logCol);
        TextOutA(hdc, logBoxX + 6, ly, g_state.logs[i].text, (int)strlen(g_state.logs[i].text));
        ly += 16;
    }
    
    // Help Overlay Modal
    if (g_state.showHelp) {
        int helpW = 540;
        int helpH = 340;
        int hx = (totalW - helpW) / 2;
        int hy = (totalH - helpH) / 2;
        
        RECT rcHelp = { hx, hy, hx + helpW, hy + helpH };
        HBRUSH hBrHModal = CreateSolidBrush(RGB(11, 19, 41));
        FillRect(hdc, &rcHelp, hBrHModal);
        DeleteObject(hBrHModal);
        FrameRect(hdc, &rcHelp, (HBRUSH)GetStockObject(WHITE_BRUSH));
        
        RECT rcHelpHeader = { hx, hy, hx + helpW, hy + 28 };
        FillRect(hdc, &rcHelpHeader, hBrSubHdr);
        SelectObject(hdc, g_fontHeader);
        SetTextColor(hdc, RGB(0, 240, 255));
        TextOutA(hdc, hx + 12, hy + 6, "KStarDredge - Captain's Flight Manual", 37);
        
        SelectObject(hdc, g_fontMonoBold);
        int myHelp = hy + 40;
        SetTextColor(hdc, RGB(240, 249, 255));
        TextOutA(hdc, hx + 16, myHelp, "FLIGHT CONTROLS & DREDGING TACTICS:", 35);
        myHelp += 24;
        
        SelectObject(hdc, g_fontSmall);
        SetTextColor(hdc, RGB(148, 163, 184));
        TextOutA(hdc, hx + 20, myHelp, "• [W / UP ARROW]: Engage Forward Fusion Thrusters (Consumes Fuel)", 64); myHelp += 18;
        TextOutA(hdc, hx + 20, myHelp, "• [S / DOWN ARROW]: Engage Retro Braking Thrusters", 50); myHelp += 18;
        TextOutA(hdc, hx + 20, myHelp, "• [A / D / LEFT / RIGHT]: Pivot Barge Heading", 45); myHelp += 18;
        TextOutA(hdc, hx + 20, myHelp, "• [SPACEBAR / LASER BTN]: Hold/Toggle Mining Laser (Watch Laser Heat)", 69); myHelp += 18;
        TextOutA(hdc, hx + 20, myHelp, "• [T / TRACTOR BTN]: Toggle Tractor Magnet to draw floating mineral chunks", 73); myHelp += 18;
        TextOutA(hdc, hx + 20, myHelp, "• [Z / DAMPENER BTN]: Toggle Inertial Dampeners for precise stationkeeping", 74); myHelp += 18;
        TextOutA(hdc, hx + 20, myHelp, "• [CLICK VIEWPORT]: Target & Lock asteroid with telemetry computer", 66); myHelp += 18;
        TextOutA(hdc, hx + 20, myHelp, "• [LIQUIDATE]: Sell cargo hold to orbital comm-link for Credits", 62); myHelp += 24;
        
        SetTextColor(hdc, RGB(245, 158, 11));
        TextOutA(hdc, hx + 20, myHelp, "Press [H] or Click 'Pilot Manual' to close this screen.", 54);
    }
    
    SelectObject(hdc, oldPen);
    DeleteObject(hPenBorder);
    DeleteObject(hBrPanel);
    DeleteObject(hBrSubHdr);
}

void RepositionControls(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int totalW = rc.right - rc.left;
    int totalH = rc.bottom - rc.top;
    if (totalW <= 0 || totalH <= 0) return;
    
    int rightPanelW = 240;
    int bottomCtrlH = 140;
    int botY = totalH - bottomCtrlH;
    
    // Cockpit Action Buttons in bottom-left console
    int bx = 12;
    int by = botY + 28;
    int bw = 92;
    int bh = 30;
    
    MoveWindow(g_btnLaser,    bx,          by,      bw, bh, TRUE);
    MoveWindow(g_btnTractor,  bx + bw + 6, by,      bw, bh, TRUE);
    MoveWindow(g_btnDampener, bx + (bw+6)*2, by,    bw, bh, TRUE);
    
    MoveWindow(g_btnScan,     bx,          by + 36, bw, bh, TRUE);
    MoveWindow(g_btnAudio,    bx + bw + 6, by + 36, bw, bh, TRUE);
    MoveWindow(g_btnHelp,     bx + (bw+6)*2, by + 36, bw, bh, TRUE);
    
    // Right panel buttons: Jettison & Liquidate
    int rightX = totalW - rightPanelW + 10;
    int rightY = totalH - bottomCtrlH - 36;
    int rightBtnW = (rightPanelW - 26) / 2;
    
    MoveWindow(g_btnJettison, rightX,                 rightY, rightBtnW, 26, TRUE);
    MoveWindow(g_btnSell,     rightX + rightBtnW + 6, rightY, rightBtnW, 26, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_hwnd = hwnd;
            g_fontMono = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            g_fontMonoBold = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            g_fontSmall = CreateFontA(11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            g_fontHeader = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            
            InitGame();
            
            // Create Control Buttons
            g_btnLaser    = CreateWindowA("BUTTON", "MINING LASER", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_LASER, NULL, NULL);
            g_btnTractor  = CreateWindowA("BUTTON", "TRACTOR BEAM", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_TRACTOR, NULL, NULL);
            g_btnDampener = CreateWindowA("BUTTON", "DAMPENERS",    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_DAMPENER, NULL, NULL);
            g_btnScan     = CreateWindowA("BUTTON", "SECTOR SCAN",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SCAN, NULL, NULL);
            g_btnAudio    = CreateWindowA("BUTTON", "AUDIO SYNTH",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_AUDIO, NULL, NULL);
            g_btnHelp     = CreateWindowA("BUTTON", "PILOT MANUAL", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);
            
            g_btnJettison = CreateWindowA("BUTTON", "JETTISON",     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_JETTISON, NULL, NULL);
            g_btnSell     = CreateWindowA("BUTTON", "LIQUIDATE",    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SELL, NULL, NULL);
            
            SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL, NULL);
            
            // Audio Thread
            g_hSoundThread = CreateThread(NULL, 0, SoundThreadProc, NULL, 0, NULL);
            return 0;
        }
        
        case WM_SIZE: {
            RepositionControls(hwnd);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case ID_BTN_LASER:
                    g_state.miningActive = !g_state.miningActive;
                    AddLog(g_state.miningActive ? "Mining laser firing." : "Mining laser paused.", 1);
                    break;
                case ID_BTN_TRACTOR:
                    g_state.tractorActive = !g_state.tractorActive;
                    AddLog(g_state.tractorActive ? "Tractor beam engaged." : "Tractor beam standby.", 0);
                    break;
                case ID_BTN_DAMPENER:
                    g_state.dampeners = !g_state.dampeners;
                    AddLog(g_state.dampeners ? "Inertia dampeners engaged." : "Inertia dampeners disengaged.", 0);
                    break;
                case ID_BTN_SCAN:
                    TriggerSound(SFX_BEEP);
                    AddLog("Deep sweep complete: 24 mineral asteroids registered in local sector.", 0);
                    break;
                case ID_BTN_AUDIO:
                    g_state.soundEnabled = !g_state.soundEnabled;
                    AddLog(g_state.soundEnabled ? "Audio synthesizer unmuted." : "Audio synthesizer muted.", 0);
                    break;
                case ID_BTN_HELP:
                    g_state.showHelp = !g_state.showHelp;
                    break;
                case ID_BTN_JETTISON:
                    UpdateCargoTotal();
                    if (g_state.totalCargo == 0) {
                        AddLog("Cargo hold is already empty.", 3);
                    } else {
                        for (int i = 0; i < 6; i++) g_state.cargoHold[i] = 0;
                        UpdateCargoTotal();
                        AddLog("Cargo hold purged into void space.", 4);
                    }
                    break;
                case ID_BTN_SELL: {
                    int val = CalculateCargoValue();
                    if (val <= 0) {
                        AddLog("No minerals in hold to liquidate.", 3);
                    } else {
                        g_state.credits += val;
                        char fTxt[32];
                        sprintf(fTxt, "+%d CR", val);
                        AddFloatingText(fTxt, g_state.shipX, g_state.shipY - 30.0f, RGB(245, 158, 11));
                        TriggerSound(SFX_COLLECT);
                        for (int i = 0; i < 6; i++) g_state.cargoHold[i] = 0;
                        UpdateCargoTotal();
                        char logB[64];
                        sprintf(logB, "Transferred minerals to station comm-link for %d CR.", val);
                        AddLog(logB, 5);
                    }
                    break;
                }
            }
            SetFocus(hwnd);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        
        case WM_LBUTTONDOWN: {
            int mx = GET_X_LPARAM(lParam);
            int my = GET_Y_LPARAM(lParam);
            
            RECT rc;
            GetClientRect(hwnd, &rc);
            int topHeaderH = 34;
            int bottomCtrlH = 140;
            int leftPanelW = 220;
            int rightPanelW = 240;
            int totalW = rc.right - rc.left;
            int totalH = rc.bottom - rc.top;
            
            int viewportX = leftPanelW;
            int viewportW = totalW - leftPanelW - rightPanelW;
            int viewportY = topHeaderH;
            int viewportH = totalH - topHeaderH - bottomCtrlH;
            
            if (g_state.showHelp) {
                g_state.showHelp = 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            
            // Check click inside viewport for asteroid targeting
            if (mx >= viewportX && mx < viewportX + viewportW && my >= viewportY && my < viewportY + viewportH) {
                int cx = viewportX + (viewportW / 2);
                int cyCenter = viewportY + (viewportH / 2);
                float clickWorldX = g_state.shipX + (float)(mx - cx);
                float clickWorldY = g_state.shipY + (float)(my - cyCenter);
                
                int bestIdx = -1;
                float minDist = 80.0f;
                for (int i = 0; i < MAX_ASTEROIDS; i++) {
                    if (!g_state.asteroids[i].active) continue;
                    float d = (float)sqrt((g_state.asteroids[i].x - clickWorldX) * (g_state.asteroids[i].x - clickWorldX) +
                                          (g_state.asteroids[i].y - clickWorldY) * (g_state.asteroids[i].y - clickWorldY));
                    if (d < g_state.asteroids[i].radius + 15.0f && d < minDist) {
                        minDist = d;
                        bestIdx = i;
                    }
                }
                
                if (bestIdx >= 0) {
                    g_state.selectedAstIndex = bestIdx;
                    TriggerSound(SFX_BEEP);
                    char tLog[64];
                    sprintf(tLog, "Target locked: %s [%s]", g_state.asteroids[bestIdx].id, ORE_DEFS[g_state.asteroids[bestIdx].oreType].name);
                    AddLog(tLog, 0);
                }
            }
            return 0;
        }
        
        case WM_KEYDOWN: {
            switch (wParam) {
                case 'W': case VK_UP:    g_state.thrusting = 1; break;
                case 'S': case VK_DOWN:  g_state.reversing = 1; break;
                case 'A': case VK_LEFT:  g_state.turningLeft = 1; break;
                case 'D': case VK_RIGHT: g_state.turningRight = 1; break;
                case VK_SPACE:
                    g_state.miningActive = 1;
                    break;
                case 'T':
                    g_state.tractorActive = !g_state.tractorActive;
                    AddLog(g_state.tractorActive ? "Tractor emitter magnetized." : "Tractor beam offline.", 0);
                    break;
                case 'Z':
                    g_state.dampeners = !g_state.dampeners;
                    AddLog(g_state.dampeners ? "Inertia dampeners engaged." : "Inertia dampeners disengaged.", 0);
                    break;
                case 'M':
                    g_state.soundEnabled = !g_state.soundEnabled;
                    AddLog(g_state.soundEnabled ? "Audio synthesizer unmuted." : "Audio synthesizer muted.", 0);
                    break;
                case 'H':
                    g_state.showHelp = !g_state.showHelp;
                    break;
            }
            return 0;
        }
        
        case WM_KEYUP: {
            switch (wParam) {
                case 'W': case VK_UP:    g_state.thrusting = 0; break;
                case 'S': case VK_DOWN:  g_state.reversing = 0; break;
                case 'A': case VK_LEFT:  g_state.turningLeft = 0; break;
                case 'D': case VK_RIGHT: g_state.turningRight = 0; break;
                case VK_SPACE:
                    g_state.miningActive = 0;
                    break;
            }
            return 0;
        }
        
        case WM_TIMER: {
            if (wParam == TIMER_ID) {
                UpdateGame(TIMER_INTERVAL / 1000.0f);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rc;
            GetClientRect(hwnd, &rc);
            int width = rc.right - rc.left;
            int height = rc.bottom - rc.top;
            
            // Double buffering
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBmp = CreateCompatibleBitmap(hdc, width, height);
            HGDIOBJ oldBmp = SelectObject(memDC, memBmp);
            
            RenderGame(memDC, &rc);
            
            BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);
            
            SelectObject(memDC, oldBmp);
            DeleteObject(memBmp);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_ERASEBKGND:
            return 1; // Handled by double buffer
            
        case WM_DESTROY: {
            KillTimer(hwnd, TIMER_ID);
            if (g_fontMono) DeleteObject(g_fontMono);
            if (g_fontMonoBold) DeleteObject(g_fontMonoBold);
            if (g_fontSmall) DeleteObject(g_fontSmall);
            if (g_fontHeader) DeleteObject(g_fontHeader);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KStarDredgeClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(0, "KStarDredgeClass", "KStarDredge - Orbital Salvage & Asteroid Dredger",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 960, 720, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}
