#include <windows.h>

#pragma function(memset)
void* memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) {
        *bytes++ = (char)c;
    }
    return dest;
}

#pragma function(memcpy)
void* memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) {
        *d++ = *s++;
    }
    return dest;
}

/* Forward declarations & memory utilities */
static void MyZeroMemory(void* ptr, int size) {
    memset(ptr, 0, size);
}

static void MyCopyMemory(void* dst, const void* src, int size) {
    memcpy(dst, src, size);
}

static unsigned int g_seed = 12345;
static int MyRand() {
    g_seed = (214013 * g_seed + 2531011);
    return (g_seed >> 16) & 0x7FFF;
}

/* Sound Thread Engine */
static DWORD WINAPI SoundThread(LPVOID lpParam) {
    int type = (int)(intptr_t)lpParam;
    if (type == 1) { // Step
        Beep(180, 25);
    } else if (type == 2) { // Epoch shift forward
        Beep(240, 40);
        Beep(360, 40);
        Beep(520, 60);
    } else if (type == 3) { // Epoch shift backward
        Beep(520, 40);
        Beep(360, 40);
        Beep(240, 60);
    } else if (type == 4) { // Causal ripple chime
        Beep(440, 50);
        Beep(554, 50);
        Beep(659, 50);
        Beep(880, 80);
    } else if (type == 5) { // Switch / plate
        Beep(850, 40);
        Beep(600, 40);
    } else if (type == 6) { // Gate open
        Beep(140, 150);
        Beep(110, 200);
    } else if (type == 7) { // Quicksave
        Beep(700, 60);
        Beep(900, 90);
    } else if (type == 8) { // Quickload
        Beep(900, 60);
        Beep(700, 90);
    } else if (type == 9) { // Victory fanfare
        Beep(523, 100);
        Beep(659, 100);
        Beep(784, 120);
        Beep(1046, 250);
    } else if (type == 10) { // Alarm
        Beep(900, 80);
        Sleep(40);
        Beep(720, 80);
    }
    return 0;
}

static void PlaySfx(int type) {
    CreateThread(NULL, 0, SoundThread, (LPVOID)(intptr_t)type, 0, NULL);
}

/* Grid & Game Dimensions */
#define GRID_W 24
#define GRID_H 16
#define TILE_SZ 26

#define TILE_VOID 0
#define TILE_WALL 1
#define TILE_FLOOR 2
#define TILE_RELAY 3
#define TILE_GATE 4
#define TILE_PLATE 5
#define TILE_CORE 6
#define TILE_RIFT 7
#define TILE_VAULT 8
#define TILE_LOCKER 9
#define TILE_PLASMA 10
#define TILE_ICE 11
#define TILE_SAPLING 12
#define TILE_BIO 13
#define TILE_CRYO 14

#define EPOCH_ALPHA 0 // 1984 Past
#define EPOCH_BETA  1 // 2042 Present
#define EPOCH_GAMMA 2 // 2188 Future

#define STATE_SPLASH   0
#define STATE_TUTORIAL 1
#define STATE_PLAYING  2
#define STATE_HELP     3
#define STATE_VICTORY  4

typedef struct {
    int x, y;
    int type; // 1 = crate, 2 = phantom
    int dir;
} Entity;

#define MAX_ENTITIES 16
#define MAX_ECHO_STEPS 64
#define MAX_PARTICLES 64

typedef struct {
    int x, y;
    int epoch;
    int dir;
} EchoStep;

typedef struct {
    int x, y;
    int vx, vy;
    int life;
    COLORREF color;
} Particle;

typedef struct {
    unsigned char grids[3][GRID_H][GRID_W];
    Entity entities[3][MAX_ENTITIES];
    int entityCount[3];
    int playerX, playerY;
    int playerDir; // 0=Down, 1=Up, 2=Left, 3=Right
    int epoch;
    int turn;
    int paradoxStrain;
    int precursorRepaired;
    int seedPlanted;
    int floodgateOpen;
    int riftsClosed;
    int scenario; // 1 to 6
    int won;
    EchoStep echoBuffer[MAX_ECHO_STEPS];
    int echoCount;
} GameState;

static GameState g_game;
static int g_appState = STATE_SPLASH;
static int g_tutorialStep = 0;
static HWND g_hwnd = NULL;
static int g_animTick = 0;
static int g_toastTimer = 0;
static char g_toastMsg[64] = {0};

/* Echo System */
static int g_isRecordingEcho = 0;
static int g_isPlayingEcho = 0;
static EchoStep g_echoBuffer[MAX_ECHO_STEPS];
static int g_echoCount = 0;
static int g_echoStepIndex = 0;
static int g_hasEchoGhost = 0;
static EchoStep g_echoGhost;

/* Particles & Ripples */
static Particle g_particles[MAX_PARTICLES];
static int g_particleCount = 0;
static int g_rippleRadius = 0;
static int g_rippleMax = 0;
static int g_rippleX = 0, g_rippleY = 0;

static void SpawnParticles(int x, int y, COLORREF color, int count) {
    for (int i = 0; i < count && g_particleCount < MAX_PARTICLES; i++) {
        Particle* p = &g_particles[g_particleCount++];
        p->x = x;
        p->y = y;
        p->vx = (MyRand() % 9) - 4;
        p->vy = (MyRand() % 9) - 4;
        p->life = 15 + (MyRand() % 15);
        p->color = color;
    }
}

static void TriggerRipple(int x, int y) {
    g_rippleX = x;
    g_rippleY = y;
    g_rippleRadius = 2;
    g_rippleMax = 120;
    PlaySfx(4); // Ripple chime
}

/* Initialization */
static void InitGrid(int epoch) {
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            if (x == 0 || y == 0 || x == GRID_W - 1 || y == GRID_H - 1) {
                g_game.grids[epoch][y][x] = TILE_WALL;
            } else {
                g_game.grids[epoch][y][x] = TILE_FLOOR;
            }
        }
    }
    // Partition wall with blast doors at (12, 7) and (12, 8)
    for (int y = 1; y < GRID_H - 1; y++) {
        if (y != 7 && y != 8) {
            g_game.grids[epoch][y][12] = TILE_WALL;
        }
    }
    // Quantum vault interior at (17-21, 4-11)
    for (int x = 17; x < 22; x++) {
        g_game.grids[epoch][4][x] = TILE_WALL;
        g_game.grids[epoch][11][x] = TILE_WALL;
    }
    for (int y = 4; y <= 11; y++) {
        if (y != 7) g_game.grids[epoch][y][17] = TILE_WALL;
    }
}

static void PropagateCausality(int notify) {
    int ripples = 0;

    // Rule 1: Precursor Relay in Alpha -> Blast Gates in Beta and Gamma
    if (g_game.precursorRepaired) {
        if (g_game.grids[EPOCH_BETA][7][12] == TILE_GATE) {
            g_game.grids[EPOCH_BETA][7][12] = TILE_FLOOR;
            g_game.grids[EPOCH_BETA][8][12] = TILE_FLOOR;
            ripples++;
        }
        if (g_game.grids[EPOCH_GAMMA][7][12] == TILE_GATE) {
            g_game.grids[EPOCH_GAMMA][7][12] = TILE_FLOOR;
            g_game.grids[EPOCH_GAMMA][8][12] = TILE_FLOOR;
            ripples++;
        }
    }

    // Rule 2: Synchronize Crate Positions from Alpha forward to Beta
    for (int i = 0; i < g_game.entityCount[EPOCH_ALPHA]; i++) {
        if (g_game.entities[EPOCH_ALPHA][i].type == 1) {
            int cx = g_game.entities[EPOCH_ALPHA][i].x;
            int cy = g_game.entities[EPOCH_ALPHA][i].y;
            if (i < g_game.entityCount[EPOCH_BETA]) {
                g_game.entities[EPOCH_BETA][i].x = cx;
                g_game.entities[EPOCH_BETA][i].y = cy;
            }
        }
    }

    // Rule 3: Seed Planted in Alpha -> Bio-Bridge in Beta and Gamma
    if (g_game.seedPlanted) {
        if (g_game.grids[EPOCH_BETA][7][10] == TILE_PLASMA || g_game.grids[EPOCH_BETA][7][10] == TILE_VOID) {
            g_game.grids[EPOCH_BETA][7][10] = TILE_BIO;
            ripples++;
        }
        if (g_game.grids[EPOCH_GAMMA][7][10] == TILE_VOID) {
            g_game.grids[EPOCH_GAMMA][7][10] = TILE_BIO;
            ripples++;
        }
    }

    // Rule 4: Floodgate Open in Alpha -> Ice Bridge in Gamma & Clear Plasma in Beta
    if (g_game.floodgateOpen) {
        for (int y = 3; y <= 11; y++) {
            if (g_game.grids[EPOCH_BETA][y][10] == TILE_PLASMA) {
                g_game.grids[EPOCH_BETA][y][10] = TILE_FLOOR;
            }
            if (g_game.grids[EPOCH_GAMMA][y][10] == TILE_VOID) {
                g_game.grids[EPOCH_GAMMA][y][10] = TILE_ICE;
            }
        }
        ripples++;
    }

    // Rule 5: Dual Pressure Plates in Beta (Scenario 2)
    if (g_game.scenario == 2) {
        int plate1 = 0, plate2 = 0;
        if (g_game.epoch == EPOCH_BETA && g_game.playerX == 5 && g_game.playerY == 4) plate1 = 1;
        if (g_game.epoch == EPOCH_BETA && g_game.playerX == 5 && g_game.playerY == 10) plate2 = 1;
        if (g_hasEchoGhost && g_echoGhost.epoch == EPOCH_BETA && g_echoGhost.x == 5 && g_echoGhost.y == 4) plate1 = 1;
        if (g_hasEchoGhost && g_echoGhost.epoch == EPOCH_BETA && g_echoGhost.x == 5 && g_echoGhost.y == 10) plate2 = 1;

        if (plate1 && plate2) {
            if (g_game.grids[EPOCH_BETA][7][12] == TILE_GATE) {
                g_game.grids[EPOCH_BETA][7][12] = TILE_FLOOR;
                ripples++;
            }
        } else if (!g_game.precursorRepaired) {
            g_game.grids[EPOCH_BETA][7][12] = TILE_GATE;
        }
    }

    // Rule 6: Scenario 6 Dual Pressure & Coolant Cascade
    if (g_game.scenario == 6) {
        int plate = 0;
        if (g_game.epoch == EPOCH_BETA && g_game.playerX == 5 && g_game.playerY == 5) plate = 1;
        if (g_hasEchoGhost && g_echoGhost.epoch == EPOCH_BETA && g_echoGhost.x == 5 && g_echoGhost.y == 5) plate = 1;
        for (int i = 0; i < g_game.entityCount[EPOCH_BETA]; i++) {
            if (g_game.entities[EPOCH_BETA][i].type == 1 && g_game.entities[EPOCH_BETA][i].x == 5 && g_game.entities[EPOCH_BETA][i].y == 5) plate = 1;
        }

        if (plate || g_game.precursorRepaired) {
            if (g_game.grids[EPOCH_BETA][7][12] == TILE_GATE) {
                g_game.grids[EPOCH_BETA][7][12] = TILE_FLOOR;
                g_game.grids[EPOCH_BETA][8][12] = TILE_FLOOR;
                ripples++;
            }
        } else {
            if (g_game.grids[EPOCH_BETA][7][12] == TILE_FLOOR) {
                g_game.grids[EPOCH_BETA][7][12] = TILE_GATE;
                g_game.grids[EPOCH_BETA][8][12] = TILE_GATE;
            }
        }
    }

    if (notify && ripples > 0) {
        TriggerRipple(g_game.playerX * TILE_SZ + 13, g_game.playerY * TILE_SZ + 13);
    }
}

static void ResetGame(int scenario) {
    MyZeroMemory(&g_game, sizeof(GameState));
    g_game.scenario = scenario;
    g_game.epoch = (scenario == 3 || scenario == 4 || scenario == 6) ? EPOCH_ALPHA : EPOCH_BETA;
    g_game.playerX = 4;
    g_game.playerY = 7;
    g_game.playerDir = 0; // Down
    g_game.precursorRepaired = 0;
    g_game.seedPlanted = 0;
    g_game.floodgateOpen = 0;
    g_game.paradoxStrain = 0;
    g_game.turn = 0;
    g_game.won = 0;

    for (int e = 0; e < 3; e++) {
        InitGrid(e);
        g_game.entityCount[e] = 0;
    }

    if (scenario == 1) {
        // Genesis Core
        g_game.grids[EPOCH_ALPHA][3][6] = TILE_RELAY;
        g_game.grids[EPOCH_BETA][7][12] = TILE_GATE;
        g_game.grids[EPOCH_BETA][8][12] = TILE_GATE;
        g_game.grids[EPOCH_BETA][7][19] = TILE_CORE;

        g_game.entities[EPOCH_ALPHA][0].type = 1;
        g_game.entities[EPOCH_ALPHA][0].x = 8;
        g_game.entities[EPOCH_ALPHA][0].y = 7;
        g_game.entityCount[EPOCH_ALPHA] = 1;

        g_game.entities[EPOCH_BETA][0].type = 1;
        g_game.entities[EPOCH_BETA][0].x = 8;
        g_game.entities[EPOCH_BETA][0].y = 7;
        g_game.entityCount[EPOCH_BETA] = 1;

    } else if (scenario == 2) {
        // Echo Protocol
        g_game.grids[EPOCH_BETA][4][5] = TILE_PLATE;
        g_game.grids[EPOCH_BETA][10][5] = TILE_PLATE;
        g_game.grids[EPOCH_BETA][7][12] = TILE_GATE;
        g_game.grids[EPOCH_BETA][7][19] = TILE_CORE;

    } else if (scenario == 3) {
        // Singularity Rupture
        g_game.grids[EPOCH_ALPHA][4][8] = TILE_RIFT;
        g_game.grids[EPOCH_BETA][11][8] = TILE_RIFT;
        g_game.grids[EPOCH_GAMMA][7][19] = TILE_RIFT;

        g_game.entities[EPOCH_GAMMA][0].type = 2; // Phantom
        g_game.entities[EPOCH_GAMMA][0].x = 14;
        g_game.entities[EPOCH_GAMMA][0].y = 7;
        g_game.entities[EPOCH_GAMMA][0].dir = 1;
        g_game.entityCount[EPOCH_GAMMA] = 1;

    } else if (scenario == 4) {
        // Grandfather's Cipher
        for (int y = 3; y <= 11; y++) {
            g_game.grids[EPOCH_BETA][y][10] = TILE_PLASMA;
            g_game.grids[EPOCH_GAMMA][y][10] = TILE_VOID;
        }
        g_game.grids[EPOCH_ALPHA][7][6] = TILE_SAPLING;
        g_game.grids[EPOCH_GAMMA][7][19] = TILE_CORE;
        g_game.grids[EPOCH_ALPHA][11][4] = TILE_LOCKER;
        g_game.grids[EPOCH_BETA][11][4] = TILE_LOCKER;
        g_game.grids[EPOCH_GAMMA][11][4] = TILE_LOCKER;

    } else if (scenario == 6) {
        // Tachyon Cascade (The Grand Paradox)
        g_game.grids[EPOCH_ALPHA][3][6] = TILE_RELAY;
        g_game.grids[EPOCH_ALPHA][11][4] = TILE_CRYO;
        g_game.entities[EPOCH_ALPHA][0].type = 1;
        g_game.entities[EPOCH_ALPHA][0].x = 8;
        g_game.entities[EPOCH_ALPHA][0].y = 7;
        g_game.entityCount[EPOCH_ALPHA] = 1;

        g_game.entities[EPOCH_BETA][0].type = 1;
        g_game.entities[EPOCH_BETA][0].x = 8;
        g_game.entities[EPOCH_BETA][0].y = 7;
        g_game.entityCount[EPOCH_BETA] = 1;

        for (int y = 3; y <= 11; y++) {
            g_game.grids[EPOCH_BETA][y][10] = TILE_PLASMA;
            g_game.grids[EPOCH_GAMMA][y][10] = TILE_VOID;
        }
        g_game.grids[EPOCH_BETA][5][5] = TILE_PLATE;
        g_game.grids[EPOCH_BETA][7][12] = TILE_GATE;
        g_game.grids[EPOCH_BETA][8][12] = TILE_GATE;

        g_game.grids[EPOCH_GAMMA][7][19] = TILE_CORE;
        g_game.grids[EPOCH_GAMMA][11][4] = TILE_RIFT;

        g_game.entities[EPOCH_GAMMA][0].type = 2; // Phantom
        g_game.entities[EPOCH_GAMMA][0].x = 14;
        g_game.entities[EPOCH_GAMMA][0].y = 7;
        g_game.entities[EPOCH_GAMMA][0].dir = 1;
        g_game.entityCount[EPOCH_GAMMA] = 1;

    } else {
        // Scenario 5: Sandbox
        g_game.grids[EPOCH_ALPHA][3][6] = TILE_RELAY;
        g_game.grids[EPOCH_ALPHA][7][6] = TILE_SAPLING;
        g_game.grids[EPOCH_BETA][7][12] = TILE_GATE;
        g_game.grids[EPOCH_BETA][4][5] = TILE_PLATE;
        g_game.grids[EPOCH_GAMMA][7][19] = TILE_CORE;

        g_game.entities[EPOCH_ALPHA][0].type = 1;
        g_game.entities[EPOCH_ALPHA][0].x = 8;
        g_game.entities[EPOCH_ALPHA][0].y = 7;
        g_game.entityCount[EPOCH_ALPHA] = 1;

        g_game.entities[EPOCH_BETA][0].type = 1;
        g_game.entities[EPOCH_BETA][0].x = 8;
        g_game.entities[EPOCH_BETA][0].y = 7;
        g_game.entityCount[EPOCH_BETA] = 1;
    }

    g_isRecordingEcho = 0;
    g_isPlayingEcho = 0;
    g_echoCount = 0;
    g_hasEchoGhost = 0;
    g_game.echoCount = 0;
    memset(g_game.echoBuffer, 0, sizeof(g_game.echoBuffer));

    PropagateCausality(0);
}

/* Save & Load */
static void Quicksave() {
    memcpy(g_game.echoBuffer, g_echoBuffer, sizeof(g_echoBuffer));
    g_game.echoCount = g_echoCount;
    HANDLE hFile = CreateFileA("kchrono_save.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hFile, &g_game, sizeof(GameState), &written, NULL);
        CloseHandle(hFile);
        PlaySfx(7); // Save chime
        g_toastTimer = 60;
        lstrcpyA(g_toastMsg, "QUICKSAVE COMPLETE (F5)");
    }
}

static int Quickload() {
    HANDLE hFile = CreateFileA("kchrono_save.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD read = 0;
        ReadFile(hFile, &g_game, sizeof(GameState), &read, NULL);
        CloseHandle(hFile);
        memcpy(g_echoBuffer, g_game.echoBuffer, sizeof(g_echoBuffer));
        g_echoCount = g_game.echoCount;
        g_isRecordingEcho = 0;
        g_isPlayingEcho = 0;
        g_hasEchoGhost = 0;
        PlaySfx(8); // Load chime
        g_toastTimer = 60;
        lstrcpyA(g_toastMsg, "QUICKLOAD COMPLETE (F9)");
        return 1;
    }
    g_toastTimer = 60;
    lstrcpyA(g_toastMsg, "NO SAVE FILE FOUND");
    return 0;
}

static int HasSaveFile() {
    DWORD dwAttrib = GetFileAttributesA("kchrono_save.dat");
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

static int HasSeenTutorial() {
    DWORD dwAttrib = GetFileAttributesA("kchrono_tutorial.dat");
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

static void MarkTutorialSeen() {
    HANDLE hFile = CreateFileA("kchrono_tutorial.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }
}

/* Gameplay Actions */
static void SwitchEpoch(int target) {
    if (target == g_game.epoch) return;
    int dir = target > g_game.epoch ? 1 : 0;
    g_game.epoch = target;
    PlaySfx(dir ? 2 : 3);
    g_game.paradoxStrain += 2;
    if (g_game.paradoxStrain > 100) g_game.paradoxStrain = 100;

    if (g_game.grids[g_game.epoch][g_game.playerY][g_game.playerX] == TILE_WALL) {
        if (g_game.playerX > 1) g_game.playerX--;
        else g_game.playerX++;
    }

    SpawnParticles(g_game.playerX * TILE_SZ + 13, g_game.playerY * TILE_SZ + 13, RGB(6, 182, 212), 20);
}

static void MovePlayer(int dx, int dy) {
    if (dx > 0) g_game.playerDir = 3; // Right
    else if (dx < 0) g_game.playerDir = 2; // Left
    else if (dy > 0) g_game.playerDir = 0; // Down
    else if (dy < 0) g_game.playerDir = 1; // Up

    int nx = g_game.playerX + dx;
    int ny = g_game.playerY + dy;

    if (nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H) return;

    int tile = g_game.grids[g_game.epoch][ny][nx];
    if (tile == TILE_WALL || tile == TILE_GATE || tile == TILE_PLASMA || tile == TILE_VOID) {
        PlaySfx(1); // Bump
        return;
    }

    // Check for crate
    for (int i = 0; i < g_game.entityCount[g_game.epoch]; i++) {
        Entity* e = &g_game.entities[g_game.epoch][i];
        if (e->type == 1 && e->x == nx && e->y == ny) {
            int cnx = nx + dx;
            int cny = ny + dy;
            if (cnx >= 0 && cnx < GRID_W && cny >= 0 && cny < GRID_H) {
                int ctile = g_game.grids[g_game.epoch][cny][cnx];
                if (ctile == TILE_FLOOR || ctile == TILE_PLATE || ctile == TILE_ICE || ctile == TILE_BIO) {
                    e->x = cnx;
                    e->y = cny;
                    PlaySfx(5);
                    if (g_game.epoch == EPOCH_ALPHA) {
                        g_game.paradoxStrain += 4;
                    }
                } else {
                    return;
                }
            } else {
                return;
            }
            break;
        }
    }

    g_game.playerX = nx;
    g_game.playerY = ny;
    g_game.turn++;
    PlaySfx(1);

    // Record echo
    if (g_isRecordingEcho && g_echoCount < MAX_ECHO_STEPS) {
        g_echoBuffer[g_echoCount].x = nx;
        g_echoBuffer[g_echoCount].y = ny;
        g_echoBuffer[g_echoCount].epoch = g_game.epoch;
        g_echoBuffer[g_echoCount].dir = g_game.playerDir;
        g_echoCount++;
    }

    // Playback echo ghost
    if (g_isPlayingEcho && g_echoCount > 0) {
        if (g_echoStepIndex < g_echoCount) {
            g_echoGhost = g_echoBuffer[g_echoStepIndex++];
            g_hasEchoGhost = 1;
        } else {
            g_echoStepIndex = 0;
            g_echoGhost = g_echoBuffer[0];
            g_hasEchoGhost = 1;
        }
    }

    // Move phantoms in Gamma
    for (int i = 0; i < g_game.entityCount[g_game.epoch]; i++) {
        Entity* e = &g_game.entities[g_game.epoch][i];
        if (e->type == 2) {
            e->y += e->dir;
            if (e->y <= 4 || e->y >= 11) e->dir *= -1;
            if (e->x == g_game.playerX && e->y == g_game.playerY) {
                g_game.paradoxStrain += 10;
                if (g_game.paradoxStrain > 100) g_game.paradoxStrain = 100;
                PlaySfx(8); // Alarm
                TriggerRipple(g_game.playerX * TILE_SZ + 13, g_game.playerY * TILE_SZ + 13);
            }
        }
    }

    // Passive paradox strain accumulation
    if ((g_game.scenario == 3 || g_game.scenario == 6) && g_game.riftsClosed < 3) {
        if (g_game.turn % 3 == 0) {
            g_game.paradoxStrain++;
            if (g_game.paradoxStrain > 100) g_game.paradoxStrain = 100;
        }
    }

    // Check tile actions
    if (tile == TILE_RELAY && !g_game.precursorRepaired) {
        g_game.precursorRepaired = 1;
        g_game.paradoxStrain += 10;
        PlaySfx(5);
        PropagateCausality(1);
    } else if (tile == TILE_CRYO && !g_game.floodgateOpen) {
        g_game.floodgateOpen = 1;
        g_game.paradoxStrain += 8;
        PlaySfx(5);
        PropagateCausality(1);
    } else if (tile == TILE_SAPLING && !g_game.seedPlanted) {
        g_game.seedPlanted = 1;
        g_game.paradoxStrain += 6;
        PlaySfx(5);
        PropagateCausality(1);
    } else if (tile == TILE_RIFT) {
        g_game.grids[g_game.epoch][ny][nx] = TILE_FLOOR;
        g_game.riftsClosed++;
        g_game.paradoxStrain -= 20;
        if (g_game.paradoxStrain < 0) g_game.paradoxStrain = 0;
        PlaySfx(4);
        if (g_game.scenario == 3 && g_game.riftsClosed >= 3) {
            g_game.won = 1;
            g_appState = STATE_VICTORY;
            PlaySfx(9);
        }
    } else if (tile == TILE_CORE) {
        g_game.won = 1;
        g_appState = STATE_VICTORY;
        PlaySfx(9);
    }

    PropagateCausality(0);
}

static void ToggleRecordEcho() {
    if (g_isRecordingEcho) {
        g_isRecordingEcho = 0;
        PlaySfx(5);
    } else {
        g_isRecordingEcho = 1;
        g_echoCount = 0;
        PlaySfx(5);
    }
}

static void TogglePlayEcho() {
    if (g_isPlayingEcho) {
        g_isPlayingEcho = 0;
        g_hasEchoGhost = 0;
        PlaySfx(5);
    } else if (g_echoCount > 0) {
        g_isPlayingEcho = 1;
        g_echoStepIndex = 0;
        g_echoGhost = g_echoBuffer[0];
        g_hasEchoGhost = 1;
        PlaySfx(4);
    }
}

/* GDI Double Buffered Paint & Sprite Engine */
static void DrawGame(HDC hdc, RECT* rcClient) {
    int w = rcClient->right - rcClient->left;
    int h = rcClient->bottom - rcClient->top;

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBM = CreateCompatibleBitmap(hdc, w, h);
    HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

    // Deep Void Background
    HBRUSH bgBrush = CreateSolidBrush(RGB(6, 9, 19));
    FillRect(memDC, rcClient, bgBrush);
    DeleteObject(bgBrush);

    HFONT hFont = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_MODERN, "Consolas");
    HFONT hTitleFont = CreateFontA(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_MODERN, "Consolas");
    HFONT oldFont = (HFONT)SelectObject(memDC, hFont);

    SetBkMode(memDC, TRANSPARENT);

    if (g_appState == STATE_SPLASH) {
        SelectObject(memDC, hTitleFont);
        SetTextColor(memDC, RGB(6, 182, 212));
        TextOutA(memDC, w / 2 - 130, 60, "K C H R O N O", 13);

        SelectObject(memDC, hFont);
        SetTextColor(memDC, RGB(148, 163, 184));
        TextOutA(memDC, w / 2 - 170, 95, "Chrono-Spatial Paradox Engine Simulator", 39);

        // Menu Options (Scenarios 1-6)
        SetTextColor(memDC, RGB(248, 250, 252));
        TextOutA(memDC, w / 2 - 140, 140, "[1] The Genesis Core (Operation Early Spark)", 43);
        TextOutA(memDC, w / 2 - 140, 170, "[2] The Echo Protocol (Ghost Shift)", 35);
        TextOutA(memDC, w / 2 - 140, 200, "[3] Singularity Rupture (Triple Containment)", 44);
        TextOutA(memDC, w / 2 - 140, 230, "[4] Grandfather's Cipher (Causal Loop)", 38);
        TextOutA(memDC, w / 2 - 140, 260, "[5] Chrono-Architect Sandbox (Free Lab)", 39);
        TextOutA(memDC, w / 2 - 140, 290, "[6] Tachyon Cascade (The Grand Paradox)", 39);

        if (HasSaveFile()) {
            SetTextColor(memDC, RGB(16, 185, 129));
            TextOutA(memDC, w / 2 - 140, 335, "[C] RESUME / LOAD QUICKSAVE", 27);
        } else {
            SetTextColor(memDC, RGB(71, 85, 105));
            TextOutA(memDC, w / 2 - 140, 335, "[C] RESUME (No Save Found)", 26);
        }
        SetTextColor(memDC, RGB(245, 158, 11));
        TextOutA(memDC, w / 2 - 140, 365, "[H] TEMPORAL MANUAL / BRIEFING", 30);
        SetTextColor(memDC, RGB(148, 163, 184));
        TextOutA(memDC, w / 2 - 140, 395, "[Q] EXIT TO DESKTOP", 19);

        SetTextColor(memDC, RGB(100, 116, 139));
        TextOutA(memDC, w / 2 - 180, 480, "Quicksave [F5] • Quickload [F9] • Size < 999 KB", 48);

    } else if (g_appState == STATE_TUTORIAL) {
        SelectObject(memDC, hTitleFont);
        SetTextColor(memDC, RGB(245, 158, 11));
        TextOutA(memDC, 60, 40, "TEMPORAL ARCHITECT FIELD BRIEFING", 33);

        SelectObject(memDC, hFont);
        SetTextColor(memDC, RGB(248, 250, 252));
        TextOutA(memDC, 60, 95, "1. TRI-EPOCH SYSTEM: You exist simultaneously across 3 synchronized eras:", 73);
        SetTextColor(memDC, RGB(245, 158, 11));
        TextOutA(memDC, 80, 120, "• Alpha (1984 - Past), Beta (2042 - Present), Gamma (2188 - Future).", 67);
        SetTextColor(memDC, RGB(148, 163, 184));
        TextOutA(memDC, 80, 140, "  Press [Tab] or [1 / 2 / 3] to shift your temporal perspective.", 64);

        SetTextColor(memDC, RGB(248, 250, 252));
        TextOutA(memDC, 60, 175, "2. FORWARD CAUSAL RIPPLE ENGINE:", 32);
        SetTextColor(memDC, RGB(6, 182, 212));
        TextOutA(memDC, 80, 200, "• Actions in earlier epochs propagate forward into future reality.", 66);
        SetTextColor(memDC, RGB(148, 163, 184));
        TextOutA(memDC, 80, 220, "  Repairing a relay in Alpha powers blast gates in Beta and Gamma!", 65);

        SetTextColor(memDC, RGB(248, 250, 252));
        TextOutA(memDC, 60, 255, "3. CHRONO-ECHO GHOST SYSTEM:", 28);
        SetTextColor(memDC, RGB(192, 132, 252));
        TextOutA(memDC, 80, 280, "• Press [R] to record your footsteps, then [P] to execute an Echo.", 67);
        SetTextColor(memDC, RGB(148, 163, 184));
        TextOutA(memDC, 80, 300, "  A temporal clone repeats your path to hold biometric switches!", 64);

        SetTextColor(memDC, RGB(248, 250, 252));
        TextOutA(memDC, 60, 335, "4. PARADOX STRAIN & STABILIZATION:", 34);
        SetTextColor(memDC, RGB(239, 68, 68));
        TextOutA(memDC, 80, 360, "• Timeline alterations generate Paradox Strain. Close rifts to restore!", 71);

        SetTextColor(memDC, RGB(16, 185, 129));
        TextOutA(memDC, 60, 420, "Press [SPACE] or [ENTER] to acknowledge and begin mission.", 58);

    } else if (g_appState == STATE_PLAYING || g_appState == STATE_VICTORY) {
        // Top HUD Bar
        HBRUSH hudBrush = CreateSolidBrush(RGB(12, 18, 34));
        RECT rBar = { 0, 0, w, 44 };
        FillRect(memDC, &rBar, hudBrush);
        DeleteObject(hudBrush);

        SetTextColor(memDC, RGB(248, 250, 252));
        TextOutA(memDC, 14, 12, "KCHRONO", 7);

        // Active Epoch Badge
        COLORREF epCol = (g_game.epoch == EPOCH_ALPHA) ? RGB(245, 158, 11) : (g_game.epoch == EPOCH_BETA) ? RGB(6, 182, 212) : RGB(192, 132, 252);
        SetTextColor(memDC, epCol);
        if (g_game.epoch == EPOCH_ALPHA) {
            TextOutA(memDC, 95, 12, "[1] EPOCH ALPHA (1984)", 22);
        } else if (g_game.epoch == EPOCH_BETA) {
            TextOutA(memDC, 95, 12, "[2] EPOCH BETA (2042)", 21);
        } else {
            TextOutA(memDC, 95, 12, "[3] EPOCH GAMMA (2188)", 22);
        }

        // Scenario Badge
        SetTextColor(memDC, RGB(203, 213, 225));
        char scText[32];
        wsprintfA(scText, "Op #%d", g_game.scenario);
        TextOutA(memDC, 330, 12, scText, lstrlenA(scText));

        // Paradox Strain Gauge
        COLORREF strainColor = (g_game.paradoxStrain < 50) ? RGB(16, 185, 129) : (g_game.paradoxStrain < 80) ? RGB(245, 158, 11) : RGB(239, 68, 68);
        SetTextColor(memDC, strainColor);
        char strainText[32];
        wsprintfA(strainText, "Strain: %d%%", g_game.paradoxStrain);
        TextOutA(memDC, 410, 12, strainText, lstrlenA(strainText));

        SetTextColor(memDC, RGB(148, 163, 184));
        char turnText[32];
        wsprintfA(turnText, "Turn: %d", g_game.turn);
        TextOutA(memDC, 530, 12, turnText, lstrlenA(turnText));

        if (g_isRecordingEcho) {
            SetTextColor(memDC, RGB(239, 68, 68));
            TextOutA(memDC, 630, 12, "[REC ECHO]", 10);
        } else if (g_isPlayingEcho) {
            SetTextColor(memDC, RGB(6, 182, 212));
            TextOutA(memDC, 630, 12, "[PLAY ECHO]", 11);
        }

        // Render Simulation Grid
        int ox = 20;
        int oy = 56;

        for (int y = 0; y < GRID_H; y++) {
            for (int x = 0; x < GRID_W; x++) {
                int tile = g_game.grids[g_game.epoch][y][x];
                int tx = ox + x * TILE_SZ;
                int ty = oy + y * TILE_SZ;
                RECT rTile = { tx, ty, tx + TILE_SZ, ty + TILE_SZ };

                if (tile == TILE_WALL) {
                    if (g_game.epoch == EPOCH_ALPHA) {
                        // Alpha: Heavy industrial concrete with rivets
                        HBRUSH wBr = CreateSolidBrush(RGB(35, 28, 21));
                        FillRect(memDC, &rTile, wBr);
                        DeleteObject(wBr);

                        HPEN hP1 = CreatePen(PS_SOLID, 1, RGB(65, 52, 38));
                        HPEN oldP = (HPEN)SelectObject(memDC, hP1);
                        MoveToEx(memDC, tx, ty + TILE_SZ - 1, NULL);
                        LineTo(memDC, tx, ty);
                        LineTo(memDC, tx + TILE_SZ - 1, ty);
                        SelectObject(memDC, oldP);
                        DeleteObject(hP1);

                        // Horizontal rebar
                        SetPixel(memDC, tx + 4, ty + 4, RGB(217, 119, 6));
                        SetPixel(memDC, tx + TILE_SZ - 5, ty + 4, RGB(217, 119, 6));
                        SetPixel(memDC, tx + 4, ty + TILE_SZ - 5, RGB(217, 119, 6));
                        SetPixel(memDC, tx + TILE_SZ - 5, ty + TILE_SZ - 5, RGB(217, 119, 6));

                    } else if (g_game.epoch == EPOCH_BETA) {
                        // Beta: High-tech alloy bulkhead with cyan groove
                        HBRUSH wBr = CreateSolidBrush(RGB(12, 19, 34));
                        FillRect(memDC, &rTile, wBr);
                        DeleteObject(wBr);

                        HPEN hP1 = CreatePen(PS_SOLID, 1, RGB(30, 41, 59));
                        HPEN oldP = (HPEN)SelectObject(memDC, hP1);
                        MoveToEx(memDC, tx, ty + TILE_SZ - 1, NULL);
                        LineTo(memDC, tx, ty);
                        LineTo(memDC, tx + TILE_SZ - 1, ty);
                        SelectObject(memDC, oldP);
                        DeleteObject(hP1);

                        // Cyan power groove
                        HPEN hP2 = CreatePen(PS_SOLID, 1, RGB(6, 182, 212));
                        oldP = (HPEN)SelectObject(memDC, hP2);
                        MoveToEx(memDC, tx + 7, ty + 13, NULL);
                        LineTo(memDC, tx + TILE_SZ - 7, ty + 13);
                        SelectObject(memDC, oldP);
                        DeleteObject(hP2);

                    } else {
                        // Gamma: Obsidian ruined wall with purple crystal fracture
                        HBRUSH wBr = CreateSolidBrush(RGB(18, 9, 29));
                        FillRect(memDC, &rTile, wBr);
                        DeleteObject(wBr);

                        HPEN hP1 = CreatePen(PS_SOLID, 1, RGB(192, 132, 252));
                        HPEN oldP = (HPEN)SelectObject(memDC, hP1);
                        MoveToEx(memDC, tx + 4, ty + 8, NULL);
                        LineTo(memDC, tx + 13, ty + 15);
                        LineTo(memDC, tx + TILE_SZ - 4, ty + 20);
                        SelectObject(memDC, oldP);
                        DeleteObject(hP1);
                    }

                } else if (tile == TILE_FLOOR) {
                    COLORREF fCol = (g_game.epoch == EPOCH_ALPHA) ? RGB(22, 16, 9) : (g_game.epoch == EPOCH_BETA) ? RGB(9, 16, 29) : RGB(17, 7, 28);
                    HBRUSH fBr = CreateSolidBrush(fCol);
                    FillRect(memDC, &rTile, fBr);
                    DeleteObject(fBr);

                    // Tile Grid Dot
                    COLORREF dotCol = (g_game.epoch == EPOCH_ALPHA) ? RGB(60, 40, 20) : (g_game.epoch == EPOCH_BETA) ? RGB(16, 35, 60) : RGB(40, 16, 60);
                    SetPixel(memDC, tx + 13, ty + 13, dotCol);

                } else if (tile == TILE_RELAY) {
                    HBRUSH rBr = CreateSolidBrush(RGB(28, 20, 10));
                    FillRect(memDC, &rTile, rBr);
                    DeleteObject(rBr);

                    // Side coils
                    HBRUSH cBr = CreateSolidBrush(RGB(180, 83, 9));
                    RECT rc1 = { tx + 2, ty + 4, tx + 6, ty + TILE_SZ - 4 };
                    RECT rc2 = { tx + TILE_SZ - 6, ty + 4, tx + TILE_SZ - 2, ty + TILE_SZ - 4 };
                    FillRect(memDC, &rc1, cBr);
                    FillRect(memDC, &rc2, cBr);
                    DeleteObject(cBr);

                    // Animated Dynamo Rotor
                    HPEN rPen = CreatePen(PS_SOLID, 2, g_game.precursorRepaired ? RGB(251, 191, 36) : RGB(120, 53, 15));
                    HPEN oldP = (HPEN)SelectObject(memDC, rPen);
                    int rotMod = (g_animTick / 3) % 4;
                    if (rotMod == 0 || rotMod == 2) {
                        MoveToEx(memDC, tx + 13, ty + 6, NULL); LineTo(memDC, tx + 13, ty + 20);
                        MoveToEx(memDC, tx + 6, ty + 13, NULL); LineTo(memDC, tx + 20, ty + 13);
                    } else {
                        MoveToEx(memDC, tx + 8, ty + 8, NULL); LineTo(memDC, tx + 18, ty + 18);
                        MoveToEx(memDC, tx + 18, ty + 8, NULL); LineTo(memDC, tx + 8, ty + 18);
                    }
                    SelectObject(memDC, oldP);
                    DeleteObject(rPen);

                    // Core Node
                    HBRUSH ndBr = CreateSolidBrush(g_game.precursorRepaired ? RGB(254, 240, 138) : RGB(120, 53, 15));
                    HBRUSH oldB = (HBRUSH)SelectObject(memDC, ndBr);
                    Ellipse(memDC, tx + 10, ty + 10, tx + 17, ty + 17);
                    SelectObject(memDC, oldB);
                    DeleteObject(ndBr);

                } else if (tile == TILE_GATE) {
                    HBRUSH gBr = CreateSolidBrush(RGB(30, 41, 59));
                    FillRect(memDC, &rTile, gBr);
                    DeleteObject(gBr);

                    // Diagonal red hazard stripes
                    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(239, 68, 68));
                    HPEN oldP = (HPEN)SelectObject(memDC, hPen);
                    MoveToEx(memDC, tx + 4, ty + 4, NULL); LineTo(memDC, tx + TILE_SZ - 4, ty + TILE_SZ - 4);
                    MoveToEx(memDC, tx + TILE_SZ - 4, ty + 4, NULL); LineTo(memDC, tx + 4, ty + TILE_SZ - 4);
                    SelectObject(memDC, oldP);
                    DeleteObject(hPen);

                    // Lock Deadbolt
                    HBRUSH bBr = CreateSolidBrush(RGB(100, 116, 139));
                    RECT rBolt = { tx + 4, ty + 11, tx + TILE_SZ - 4, ty + 15 };
                    FillRect(memDC, &rBolt, bBr);
                    DeleteObject(bBr);

                } else if (tile == TILE_PLATE) {
                    HBRUSH pBg = CreateSolidBrush(RGB(15, 23, 42));
                    FillRect(memDC, &rTile, pBg);
                    DeleteObject(pBg);

                    // Check if plate active
                    int isPressed = 0;
                    if (g_game.playerX == x && g_game.playerY == y) isPressed = 1;
                    if (g_hasEchoGhost && g_echoGhost.epoch == g_game.epoch && g_echoGhost.x == x && g_echoGhost.y == y) isPressed = 1;
                    for (int i = 0; i < g_game.entityCount[g_game.epoch]; i++) {
                        if (g_game.entities[g_game.epoch][i].type == 1 && g_game.entities[g_game.epoch][i].x == x && g_game.entities[g_game.epoch][i].y == y) isPressed = 1;
                    }

                    HBRUSH plBr = CreateSolidBrush(isPressed ? RGB(6, 182, 212) : RGB(30, 41, 59));
                    RECT rPl = { tx + 4, ty + 4, tx + TILE_SZ - 4, ty + TILE_SZ - 4 };
                    FillRect(memDC, &rPl, plBr);
                    DeleteObject(plBr);

                    HPEN plPen = CreatePen(PS_SOLID, 1, isPressed ? RGB(165, 243, 252) : RGB(8, 145, 178));
                    HPEN oldP = (HPEN)SelectObject(memDC, plPen);
                    SelectObject(memDC, GetStockObject(NULL_BRUSH));
                    Ellipse(memDC, tx + 7, ty + 7, tx + TILE_SZ - 7, ty + TILE_SZ - 7);
                    SelectObject(memDC, oldP);
                    DeleteObject(plPen);

                } else if (tile == TILE_CORE) {
                    HBRUSH cBg = CreateSolidBrush(RGB(3, 7, 18));
                    FillRect(memDC, &rTile, cBg);
                    DeleteObject(cBg);

                    // Gyroscopic rotating rings
                    HPEN cPen = CreatePen(PS_SOLID, 1, RGB(56, 189, 248));
                    HPEN oldP = (HPEN)SelectObject(memDC, cPen);
                    SelectObject(memDC, GetStockObject(NULL_BRUSH));
                    Ellipse(memDC, tx + 4, ty + 4, tx + TILE_SZ - 4, ty + TILE_SZ - 4);
                    SelectObject(memDC, oldP);
                    DeleteObject(cPen);

                    // Pulsing Singularity Core
                    int pR = 4 + (g_animTick % 3);
                    HBRUSH crBr = CreateSolidBrush(RGB(255, 255, 255));
                    HBRUSH oldB = (HBRUSH)SelectObject(memDC, crBr);
                    Ellipse(memDC, tx + 13 - pR, ty + 13 - pR, tx + 13 + pR, ty + 13 + pR);
                    SelectObject(memDC, oldB);
                    DeleteObject(crBr);

                } else if (tile == TILE_RIFT) {
                    HBRUSH rBg = CreateSolidBrush(RGB(9, 2, 20));
                    FillRect(memDC, &rTile, rBg);
                    DeleteObject(rBg);

                    // Swirling vortex rings
                    HPEN rfPen = CreatePen(PS_SOLID, 1, RGB(192, 132, 252));
                    HPEN oldP = (HPEN)SelectObject(memDC, rfPen);
                    SelectObject(memDC, GetStockObject(NULL_BRUSH));
                    Ellipse(memDC, tx + 3, ty + 6, tx + TILE_SZ - 3, ty + TILE_SZ - 6);
                    Ellipse(memDC, tx + 6, ty + 3, tx + TILE_SZ - 6, ty + TILE_SZ - 3);
                    SelectObject(memDC, oldP);
                    DeleteObject(rfPen);

                    // Dark Singularity Center
                    HBRUSH dBr = CreateSolidBrush(RGB(0, 0, 0));
                    HBRUSH oldB = (HBRUSH)SelectObject(memDC, dBr);
                    Ellipse(memDC, tx + 10, ty + 10, tx + 16, ty + 16);
                    SelectObject(memDC, oldB);
                    DeleteObject(dBr);

                } else if (tile == TILE_CRYO) {
                    HBRUSH crBr = CreateSolidBrush(RGB(8, 47, 73));
                    FillRect(memDC, &rTile, crBr);
                    DeleteObject(crBr);

                    HBRUSH vBr = CreateSolidBrush(g_game.floodgateOpen ? RGB(56, 189, 248) : RGB(3, 105, 161));
                    HBRUSH oldB = (HBRUSH)SelectObject(memDC, vBr);
                    Ellipse(memDC, tx + 6, ty + 6, tx + TILE_SZ - 6, ty + TILE_SZ - 6);
                    SelectObject(memDC, oldB);
                    DeleteObject(vBr);

                } else if (tile == TILE_PLASMA) {
                    HBRUSH plBr = CreateSolidBrush(RGB(69, 10, 10));
                    FillRect(memDC, &rTile, plBr);
                    DeleteObject(plBr);

                    HBRUSH bBr = CreateSolidBrush(RGB(220, 38, 38));
                    HBRUSH oldB = (HBRUSH)SelectObject(memDC, bBr);
                    Ellipse(memDC, tx + 6, ty + 8, tx + 16, ty + 18);
                    Ellipse(memDC, tx + 12, ty + 10, tx + 20, ty + 18);
                    SelectObject(memDC, oldB);
                    DeleteObject(bBr);

                } else if (tile == TILE_ICE) {
                    HBRUSH icBr = CreateSolidBrush(RGB(7, 89, 133));
                    FillRect(memDC, &rTile, icBr);
                    DeleteObject(icBr);

                    HPEN icPen = CreatePen(PS_SOLID, 1, RGB(186, 230, 253));
                    HPEN oldP = (HPEN)SelectObject(memDC, icPen);
                    MoveToEx(memDC, tx + 3, ty + 13, NULL); LineTo(memDC, tx + 13, ty + 3);
                    LineTo(memDC, tx + TILE_SZ - 3, ty + 13); LineTo(memDC, tx + 13, ty + TILE_SZ - 3);
                    LineTo(memDC, tx + 3, ty + 13);
                    SelectObject(memDC, oldP);
                    DeleteObject(icPen);

                } else if (tile == TILE_SAPLING) {
                    HBRUSH spBr = CreateSolidBrush(RGB(6, 78, 59));
                    FillRect(memDC, &rTile, spBr);
                    DeleteObject(spBr);

                    HBRUSH sdBr = CreateSolidBrush(g_game.seedPlanted ? RGB(134, 239, 172) : RGB(34, 197, 94));
                    HBRUSH oldB = (HBRUSH)SelectObject(memDC, sdBr);
                    Ellipse(memDC, tx + 9, ty + 9, tx + 17, ty + 17);
                    SelectObject(memDC, oldB);
                    DeleteObject(sdBr);

                } else if (tile == TILE_BIO) {
                    HBRUSH bioBr = CreateSolidBrush(RGB(20, 83, 45));
                    FillRect(memDC, &rTile, bioBr);
                    DeleteObject(bioBr);

                    HPEN bioPen = CreatePen(PS_SOLID, 2, RGB(34, 197, 94));
                    HPEN oldP = (HPEN)SelectObject(memDC, bioPen);
                    MoveToEx(memDC, tx + 2, ty + 6, NULL); LineTo(memDC, tx + TILE_SZ - 2, ty + 16);
                    MoveToEx(memDC, tx + 2, ty + 20, NULL); LineTo(memDC, tx + TILE_SZ - 2, ty + 10);
                    SelectObject(memDC, oldP);
                    DeleteObject(bioPen);

                } else if (tile == TILE_LOCKER) {
                    HBRUSH lkBr = CreateSolidBrush(RGB(30, 41, 59));
                    FillRect(memDC, &rTile, lkBr);
                    DeleteObject(lkBr);

                    HBRUSH ledBr = CreateSolidBrush(RGB(16, 185, 129));
                    RECT rLed = { tx + 10, ty + 8, tx + 16, ty + 12 };
                    FillRect(memDC, &rLed, ledBr);
                    DeleteObject(ledBr);
                }

                // Tile Border Outline
                HPEN hPen = CreatePen(PS_SOLID, 1, RGB(15, 23, 42));
                HPEN oldPen = (HPEN)SelectObject(memDC, hPen);
                MoveToEx(memDC, rTile.left, rTile.top, NULL);
                LineTo(memDC, rTile.right, rTile.top);
                LineTo(memDC, rTile.right, rTile.bottom);
                LineTo(memDC, rTile.left, rTile.bottom);
                LineTo(memDC, rTile.left, rTile.top);
                SelectObject(memDC, oldPen);
                DeleteObject(hPen);
            }
        }

        // Render Entities (Crates & Phantoms)
        for (int i = 0; i < g_game.entityCount[g_game.epoch]; i++) {
            Entity* e = &g_game.entities[g_game.epoch][i];
            int ex = ox + e->x * TILE_SZ;
            int ey = oy + e->y * TILE_SZ;

            if (e->type == 1) { // Crate
                RECT rCrate = { ex + 3, ey + 3, ex + TILE_SZ - 3, ey + TILE_SZ - 3 };
                COLORREF crCol = (g_game.epoch == EPOCH_ALPHA) ? RGB(133, 77, 14) : (g_game.epoch == EPOCH_BETA) ? RGB(30, 58, 95) : RGB(74, 29, 109);
                HBRUSH cBrush = CreateSolidBrush(crCol);
                FillRect(memDC, &rCrate, cBrush);
                DeleteObject(cBrush);

                // Bevel Highlight & X-Brace
                HPEN crPen = CreatePen(PS_SOLID, 1, (g_game.epoch == EPOCH_ALPHA) ? RGB(234, 179, 8) : (g_game.epoch == EPOCH_BETA) ? RGB(56, 189, 248) : RGB(192, 132, 252));
                HPEN oldP = (HPEN)SelectObject(memDC, crPen);
                MoveToEx(memDC, ex + 5, ey + 5, NULL); LineTo(memDC, ex + TILE_SZ - 5, ey + TILE_SZ - 5);
                MoveToEx(memDC, ex + TILE_SZ - 5, ey + 5, NULL); LineTo(memDC, ex + 5, ey + TILE_SZ - 5);
                SelectObject(memDC, oldP);
                DeleteObject(crPen);

                SetPixel(memDC, ex + 13, ey + 13, RGB(255, 255, 255));

            } else if (e->type == 2) { // Phantom
                int floatOff = (g_animTick / 4) % 3;
                HBRUSH phBr = CreateSolidBrush(RGB(88, 28, 135));
                HBRUSH oldB = (HBRUSH)SelectObject(memDC, phBr);
                Ellipse(memDC, ex + 5, ey + 4 + floatOff, ex + TILE_SZ - 5, ey + 20 + floatOff);
                SelectObject(memDC, oldB);
                DeleteObject(phBr);

                // Twin Slit Eyes
                SetPixel(memDC, ex + 9, ey + 10 + floatOff, RGB(255, 255, 255));
                SetPixel(memDC, ex + 10, ey + 10 + floatOff, RGB(255, 255, 255));
                SetPixel(memDC, ex + 15, ey + 10 + floatOff, RGB(255, 255, 255));
                SetPixel(memDC, ex + 16, ey + 10 + floatOff, RGB(255, 255, 255));
            }
        }

        // Render Echo Ghost (Hologram Stippled Silhouette)
        if (g_hasEchoGhost && g_echoGhost.epoch == g_game.epoch) {
            int gx = ox + g_echoGhost.x * TILE_SZ;
            int gy = oy + g_echoGhost.y * TILE_SZ;

            // Scanline Stipple
            HPEN scPen = CreatePen(PS_SOLID, 1, RGB(6, 182, 212));
            HPEN oldP = (HPEN)SelectObject(memDC, scPen);
            for (int ly = gy + 5; ly < gy + TILE_SZ - 5; ly += 2) {
                MoveToEx(memDC, gx + 6, ly, NULL);
                LineTo(memDC, gx + TILE_SZ - 6, ly);
            }
            SelectObject(memDC, oldP);
            DeleteObject(scPen);

            // Cyan Ghost Visor
            HBRUSH gvBr = CreateSolidBrush(RGB(165, 243, 252));
            RECT rGv = { gx + 10, gy + 8, gx + 16, gy + 12 };
            FillRect(memDC, &rGv, gvBr);
            DeleteObject(gvBr);
        }

        // Render Chrononaut Player Sprite
        int px = ox + g_game.playerX * TILE_SZ;
        int py = oy + g_game.playerY * TILE_SZ;

        // Shadow
        HBRUSH shBr = CreateSolidBrush(RGB(0, 0, 0));
        HBRUSH oldB = (HBRUSH)SelectObject(memDC, shBr);
        Ellipse(memDC, px + 7, py + TILE_SZ - 6, px + TILE_SZ - 7, py + TILE_SZ - 1);
        SelectObject(memDC, oldB);
        DeleteObject(shBr);

        // Tachyon Backpack Generator
        HBRUSH bpBr = CreateSolidBrush(RGB(51, 65, 85));
        RECT rBp = { px + 7, py + 8, px + TILE_SZ - 7, py + 18 };
        FillRect(memDC, &rBp, bpBr);
        DeleteObject(bpBr);

        // Suit Body
        HBRUSH suitBr = CreateSolidBrush(RGB(241, 245, 249));
        oldB = (HBRUSH)SelectObject(memDC, suitBr);
        Ellipse(memDC, px + 8, py + 9, px + TILE_SZ - 8, py + TILE_SZ - 4);
        SelectObject(memDC, oldB);
        DeleteObject(suitBr);

        // Helmet
        HBRUSH hlBr = CreateSolidBrush(RGB(248, 250, 252));
        oldB = (HBRUSH)SelectObject(memDC, hlBr);
        Ellipse(memDC, px + 9, py + 5, px + TILE_SZ - 9, py + 15);
        SelectObject(memDC, oldB);
        DeleteObject(hlBr);

        // Directional Visor (Static, Glint-Free)
        COLORREF visorCol = (g_game.epoch == EPOCH_ALPHA) ? RGB(245, 158, 11) : (g_game.epoch == EPOCH_BETA) ? RGB(6, 182, 212) : RGB(192, 132, 252);
        HBRUSH vsBr = CreateSolidBrush(visorCol);
        if (g_game.playerDir == 0) { // Down
            RECT rVisor = { px + 10, py + 9, px + 16, py + 12 };
            FillRect(memDC, &rVisor, vsBr);
        } else if (g_game.playerDir == 2) { // Left
            RECT rVisor = { px + 9, py + 9, px + 13, py + 12 };
            FillRect(memDC, &rVisor, vsBr);
        } else if (g_game.playerDir == 3) { // Right
            RECT rVisor = { px + 13, py + 9, px + 17, py + 12 };
            FillRect(memDC, &rVisor, vsBr);
        } else { // Up
            SetPixel(memDC, px + 13, py + 7, RGB(100, 116, 139));
        }
        DeleteObject(vsBr);

        // Render Causal Ripple Circle
        if (g_rippleRadius > 0 && g_rippleRadius < g_rippleMax) {
            HPEN ripPen = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
            HPEN oldP = (HPEN)SelectObject(memDC, ripPen);
            SelectObject(memDC, GetStockObject(NULL_BRUSH));
            Ellipse(memDC, ox + g_rippleX - g_rippleRadius, oy + g_rippleY - g_rippleRadius, ox + g_rippleX + g_rippleRadius, oy + g_rippleY + g_rippleRadius);
            SelectObject(memDC, oldP);
            DeleteObject(ripPen);
            g_rippleRadius += 6;
        }

        // Render Particles
        for (int i = 0; i < g_particleCount; i++) {
            Particle* p = &g_particles[i];
            SetPixel(memDC, ox + p->x, oy + p->y, p->color);
            p->x += p->vx;
            p->y += p->vy;
            p->life--;
        }
        int alive = 0;
        for (int i = 0; i < g_particleCount; i++) {
            if (g_particles[i].life > 0) g_particles[alive++] = g_particles[i];
        }
        g_particleCount = alive;

        // Toast Notification
        if (g_toastTimer > 0) {
            int tw = 210, th = 26;
            int tx = w - tw - 16, ty = 50;
            HBRUSH bgBrush = CreateSolidBrush(RGB(12, 18, 34));
            HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(56, 189, 248));
            HGDIOBJ oldBrush = SelectObject(memDC, bgBrush);
            HGDIOBJ oldPen = SelectObject(memDC, borderPen);
            RoundRect(memDC, tx, ty, tx + tw, ty + th, 6, 6);
            SelectObject(memDC, oldBrush);
            SelectObject(memDC, oldPen);
            DeleteObject(bgBrush);
            DeleteObject(borderPen);

            SetTextColor(memDC, RGB(248, 250, 252));
            SetBkMode(memDC, TRANSPARENT);
            TextOutA(memDC, tx + 10, ty + 5, g_toastMsg, lstrlenA(g_toastMsg));
        }

        // Bottom Controls Hint
        SetTextColor(memDC, RGB(148, 163, 184));
        TextOutA(memDC, 20, h - 30, "[WASD/Arrows] Move  [Space] Wait  [1/2/3/Tab] Epoch  [R] Rec  [P] Echo  [F1/H] Help  [F5/F9] Save/Load  [Esc] Menu", 108);

        // Victory Overlay
        if (g_appState == STATE_VICTORY) {
            SelectObject(memDC, hTitleFont);
            SetTextColor(memDC, RGB(16, 185, 129));
            TextOutA(memDC, w / 2 - 140, h / 2 - 40, "TIMELINE STABILIZED!", 20);

            SelectObject(memDC, hFont);
            SetTextColor(memDC, RGB(248, 250, 252));
            TextOutA(memDC, w / 2 - 120, h / 2, "Mission Complete. Reality Coherent.", 35);
            TextOutA(memDC, w / 2 - 100, h / 2 + 30, "Press [ENTER] for Menu", 22);
        }
    }

    SelectObject(memDC, oldFont);
    DeleteObject(hFont);
    DeleteObject(hTitleFont);

    BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBM);
    DeleteObject(memBM);
    DeleteDC(memDC);
}

/* Window Procedure */
static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            SetTimer(hwnd, 1, 33, NULL); // 30 FPS animation timer
            ResetGame(1);
            return 0;
        }
        case WM_TIMER: {
            g_animTick++;
            if (g_toastTimer > 0) g_toastTimer--;
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case WM_KEYDOWN: {
            if (g_appState == STATE_SPLASH) {
                if (wParam == '1' || wParam == 'N' || wParam == 'n') {
                    ResetGame(1);
                    if (!HasSeenTutorial()) {
                        g_appState = STATE_TUTORIAL;
                        MarkTutorialSeen();
                    } else {
                        g_appState = STATE_PLAYING;
                    }
                } else if (wParam == '2') {
                    ResetGame(2);
                    g_appState = STATE_PLAYING;
                } else if (wParam == '3') {
                    ResetGame(3);
                    g_appState = STATE_PLAYING;
                } else if (wParam == '4') {
                    ResetGame(4);
                    g_appState = STATE_PLAYING;
                } else if (wParam == '5') {
                    ResetGame(5);
                    g_appState = STATE_PLAYING;
                } else if (wParam == '6') {
                    ResetGame(6);
                    g_appState = STATE_PLAYING;
                } else if (wParam == 'C' || wParam == 'c' || wParam == VK_F9) {
                    if (Quickload()) {
                        g_appState = STATE_PLAYING;
                    }
                } else if (wParam == 'H' || wParam == 'h') {
                    g_appState = STATE_TUTORIAL;
                } else if (wParam == 'Q' || wParam == 'q') {
                    PostQuitMessage(0);
                }
            } else if (g_appState == STATE_TUTORIAL) {
                if (wParam == VK_SPACE || wParam == VK_RETURN || wParam == VK_ESCAPE || wParam == 'H' || wParam == 'h' || wParam == VK_F1) {
                    g_appState = STATE_PLAYING;
                }
            } else if (g_appState == STATE_PLAYING) {
                if (wParam == VK_UP || wParam == 'W') MovePlayer(0, -1);
                else if (wParam == VK_DOWN || wParam == 'S') MovePlayer(0, 1);
                else if (wParam == VK_LEFT || wParam == 'A') MovePlayer(-1, 0);
                else if (wParam == VK_RIGHT || wParam == 'D') MovePlayer(1, 0);
                else if (wParam == VK_SPACE) MovePlayer(0, 0); // Wait turn
                else if (wParam == '1') SwitchEpoch(EPOCH_ALPHA);
                else if (wParam == '2') SwitchEpoch(EPOCH_BETA);
                else if (wParam == '3') SwitchEpoch(EPOCH_GAMMA);
                else if (wParam == VK_TAB) SwitchEpoch((g_game.epoch + 1) % 3);
                else if (wParam == 'R') ToggleRecordEcho();
                else if (wParam == 'P') TogglePlayEcho();
                else if (wParam == VK_F5) Quicksave();
                else if (wParam == VK_F9) Quickload();
                else if (wParam == 'H' || wParam == 'h' || wParam == VK_F1 || wParam == 0xBF) g_appState = STATE_TUTORIAL;
                else if (wParam == VK_ESCAPE) g_appState = STATE_SPLASH;
            } else if (g_appState == STATE_VICTORY) {
                if (wParam == VK_RETURN || wParam == VK_ESCAPE || wParam == VK_SPACE) {
                    g_appState = STATE_SPLASH;
                }
            }
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            RECT rc;
            GetClientRect(hwnd, &rc);

            if (g_appState == STATE_SPLASH) {
                int midX = rc.right / 2;
                if (mx >= midX - 160 && mx <= midX + 160) {
                    if (my >= 135 && my <= 160) {
                        ResetGame(1);
                        if (!HasSeenTutorial()) {
                            g_appState = STATE_TUTORIAL;
                            MarkTutorialSeen();
                        } else {
                            g_appState = STATE_PLAYING;
                        }
                    } else if (my >= 165 && my <= 190) {
                        ResetGame(2);
                        g_appState = STATE_PLAYING;
                    } else if (my >= 195 && my <= 220) {
                        ResetGame(3);
                        g_appState = STATE_PLAYING;
                    } else if (my >= 225 && my <= 250) {
                        ResetGame(4);
                        g_appState = STATE_PLAYING;
                    } else if (my >= 255 && my <= 280) {
                        ResetGame(5);
                        g_appState = STATE_PLAYING;
                    } else if (my >= 285 && my <= 310) {
                        ResetGame(6);
                        g_appState = STATE_PLAYING;
                    } else if (my >= 330 && my <= 355) {
                        if (Quickload()) g_appState = STATE_PLAYING;
                    } else if (my >= 360 && my <= 385) {
                        g_appState = STATE_TUTORIAL;
                    } else if (my >= 390 && my <= 415) {
                        PostQuitMessage(0);
                    }
                }
            } else if (g_appState == STATE_TUTORIAL) {
                g_appState = STATE_PLAYING;
            } else if (g_appState == STATE_VICTORY) {
                g_appState = STATE_SPLASH;
            } else if (g_appState == STATE_PLAYING) {
                if (my < 44) {
                    if (mx >= 95 && mx <= 180) SwitchEpoch(EPOCH_ALPHA);
                    else if (mx >= 190 && mx <= 280) SwitchEpoch(EPOCH_BETA);
                    else if (mx >= 290 && mx <= 380) SwitchEpoch(EPOCH_GAMMA);
                    else if (mx >= 630 && mx <= 740) {
                        if (g_isRecordingEcho) ToggleRecordEcho();
                        else if (g_isPlayingEcho) TogglePlayEcho();
                        else ToggleRecordEcho();
                    }
                } else if (my >= 56 && my < 56 + GRID_H * TILE_SZ && mx >= 20 && mx < 20 + GRID_W * TILE_SZ) {
                    int tx = (mx - 20) / TILE_SZ;
                    int ty = (my - 56) / TILE_SZ;
                    int dx = tx - g_game.playerX;
                    int dy = ty - g_game.playerY;
                    if ((dx == 1 && dy == 0) || (dx == -1 && dy == 0) || (dx == 0 && dy == 1) || (dx == 0 && dy == -1)) {
                        MovePlayer(dx, dy);
                    } else if (dx == 0 && dy == 0) {
                        MovePlayer(0, 0); // Wait / interact
                    }
                } else if (my >= rc.bottom - 40) {
                    g_appState = STATE_TUTORIAL;
                }
            }
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);
            DrawGame(hdc, &rc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY: {
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandleA(NULL);
    const char CLASS_NAME[] = "KChronoWindow";

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);
    wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(1));
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    RegisterClassA(&wc);

    RECT wr = { 0, 0, 840, 560 };
    AdjustWindowRect(&wr, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME, "KChrono - Core Paradox Engine v1.0.0",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd) {
        g_hwnd = hwnd;
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        MSG msg = {0};
        while (GetMessageA(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    ExitProcess(0);
}
