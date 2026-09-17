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
    int epoch;
    int turn;
    int paradoxStrain;
    int precursorRepaired;
    int riftsClosed;
    int scenario;
    int won;
} GameState;

static GameState g_game;
static int g_appState = STATE_SPLASH;
static int g_tutorialStep = 0;
static HWND g_hwnd = NULL;

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

    // Rule 3: Dual Pressure Plates in Beta (Scenario 2)
    if (g_game.scenario == 2) {
        int plate1 = 0, plate2 = 0;
        // Check player
        if (g_game.epoch == EPOCH_BETA && g_game.playerX == 5 && g_game.playerY == 4) plate1 = 1;
        if (g_game.epoch == EPOCH_BETA && g_game.playerX == 5 && g_game.playerY == 10) plate2 = 1;
        // Check echo ghost
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

    if (notify && ripples > 0) {
        TriggerRipple(g_game.playerX * TILE_SZ + 13, g_game.playerY * TILE_SZ + 13);
    }
}

static void ResetGame(int scenario) {
    MyZeroMemory(&g_game, sizeof(GameState));
    g_game.scenario = scenario;
    g_game.epoch = EPOCH_BETA;
    g_game.playerX = 4;
    g_game.playerY = 7;
    g_game.precursorRepaired = 0;
    g_game.paradoxStrain = 0;
    g_game.turn = 0;
    g_game.won = 0;

    for (int e = 0; e < 3; e++) {
        InitGrid(e);
        g_game.entityCount[e] = 0;
    }

    // Scenario 1: Genesis Core
    if (scenario == 1) {
        g_game.grids[EPOCH_ALPHA][3][6] = TILE_RELAY;
        g_game.grids[EPOCH_BETA][7][12] = TILE_GATE;
        g_game.grids[EPOCH_BETA][8][12] = TILE_GATE;
        g_game.grids[EPOCH_BETA][7][19] = TILE_CORE;

        // Crate in Alpha
        g_game.entities[EPOCH_ALPHA][0].type = 1;
        g_game.entities[EPOCH_ALPHA][0].x = 8;
        g_game.entities[EPOCH_ALPHA][0].y = 7;
        g_game.entityCount[EPOCH_ALPHA] = 1;

        // Corresponding crate in Beta
        g_game.entities[EPOCH_BETA][0].type = 1;
        g_game.entities[EPOCH_BETA][0].x = 8;
        g_game.entities[EPOCH_BETA][0].y = 7;
        g_game.entityCount[EPOCH_BETA] = 1;
    } else if (scenario == 2) {
        g_game.grids[EPOCH_BETA][4][5] = TILE_PLATE;
        g_game.grids[EPOCH_BETA][10][5] = TILE_PLATE;
        g_game.grids[EPOCH_BETA][7][12] = TILE_GATE;
        g_game.grids[EPOCH_BETA][7][19] = TILE_CORE;
    } else { // Scenario 3: Singularity
        g_game.grids[EPOCH_ALPHA][4][8] = TILE_RIFT;
        g_game.grids[EPOCH_BETA][11][8] = TILE_RIFT;
        g_game.grids[EPOCH_GAMMA][7][19] = TILE_RIFT;
    }

    g_isRecordingEcho = 0;
    g_isPlayingEcho = 0;
    g_echoCount = 0;
    g_hasEchoGhost = 0;

    PropagateCausality(0);
}

/* Save & Load */
static void Quicksave() {
    HANDLE hFile = CreateFileA("kchrono_save.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hFile, &g_game, sizeof(GameState), &written, NULL);
        CloseHandle(hFile);
        PlaySfx(7); // Save chime
    }
}

static int Quickload() {
    HANDLE hFile = CreateFileA("kchrono_save.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD read = 0;
        ReadFile(hFile, &g_game, sizeof(GameState), &read, NULL);
        CloseHandle(hFile);
        PlaySfx(8); // Load chime
        return 1;
    }
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

    // Check if player is stuck in wall in new epoch
    if (g_game.grids[g_game.epoch][g_game.playerY][g_game.playerX] == TILE_WALL) {
        if (g_game.playerX > 1) g_game.playerX--;
        else g_game.playerX++;
    }

    SpawnParticles(g_game.playerX * TILE_SZ + 13, g_game.playerY * TILE_SZ + 13, RGB(6, 182, 212), 20);
}

static void MovePlayer(int dx, int dy) {
    int nx = g_game.playerX + dx;
    int ny = g_game.playerY + dy;

    if (nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H) return;

    int tile = g_game.grids[g_game.epoch][ny][nx];
    if (tile == TILE_WALL || tile == TILE_GATE) {
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
                if (ctile == TILE_FLOOR || ctile == TILE_PLATE) {
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

    // Check tile actions
    if (tile == TILE_RELAY && !g_game.precursorRepaired) {
        g_game.precursorRepaired = 1;
        g_game.paradoxStrain += 10;
        PlaySfx(5);
        PropagateCausality(1);
    } else if (tile == TILE_RIFT) {
        g_game.grids[g_game.epoch][ny][nx] = TILE_FLOOR;
        g_game.riftsClosed++;
        g_game.paradoxStrain -= 15;
        if (g_game.paradoxStrain < 0) g_game.paradoxStrain = 0;
        PlaySfx(4);
        if (g_game.riftsClosed >= 3) {
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

/* GDI Double Buffered Paint */
static void DrawGame(HDC hdc, RECT* rcClient) {
    int w = rcClient->right - rcClient->left;
    int h = rcClient->bottom - rcClient->top;

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBM = CreateCompatibleBitmap(hdc, w, h);
    HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

    // Background Void
    HBRUSH bgBrush = CreateSolidBrush(RGB(6, 9, 19));
    FillRect(memDC, rcClient, bgBrush);
    DeleteObject(bgBrush);

    HFONT hFont = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Consolas");
    HFONT hTitleFont = CreateFontA(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Consolas");
    HFONT oldFont = (HFONT)SelectObject(memDC, hFont);

    SetBkMode(memDC, TRANSPARENT);

    if (g_appState == STATE_SPLASH) {
        // Render Splash Screen
        SelectObject(memDC, hTitleFont);
        SetTextColor(memDC, RGB(6, 182, 212));
        TextOutA(memDC, w / 2 - 130, 90, "K C H R O N O", 13);

        SelectObject(memDC, hFont);
        SetTextColor(memDC, RGB(148, 163, 184));
        TextOutA(memDC, w / 2 - 170, 130, "Chrono-Spatial Paradox Engine Simulator", 39);

        // Menu Options
        SetTextColor(memDC, RGB(248, 250, 252));
        TextOutA(memDC, w / 2 - 110, 200, "[N] NEW TIMELINE (Scenario 1)", 29);
        TextOutA(memDC, w / 2 - 110, 235, "[2] SCENARIO 2 (Echo Protocol)", 30);
        TextOutA(memDC, w / 2 - 110, 270, "[3] SCENARIO 3 (Singularity)", 28);
        if (HasSaveFile()) {
            SetTextColor(memDC, RGB(16, 185, 129));
            TextOutA(memDC, w / 2 - 110, 305, "[C] RESUME / LOAD QUICKSAVE", 27);
        } else {
            SetTextColor(memDC, RGB(71, 85, 105));
            TextOutA(memDC, w / 2 - 110, 305, "[C] RESUME (No Save Found)", 26);
        }
        SetTextColor(memDC, RGB(245, 158, 11));
        TextOutA(memDC, w / 2 - 110, 340, "[H] TEMPORAL MANUAL / HELP", 26);
        SetTextColor(memDC, RGB(148, 163, 184));
        TextOutA(memDC, w / 2 - 110, 375, "[Q] EXIT TO DESKTOP", 19);

        SetTextColor(memDC, RGB(100, 116, 139));
        TextOutA(memDC, w / 2 - 180, 470, "Quicksave [F5] • Quickload [F9] • Size < 999 KB", 48);

    } else if (g_appState == STATE_TUTORIAL) {
        SelectObject(memDC, hTitleFont);
        SetTextColor(memDC, RGB(245, 158, 11));
        TextOutA(memDC, 60, 40, "TEMPORAL ARCHITECT FIELD BRIEFING", 33);

        SelectObject(memDC, hFont);
        SetTextColor(memDC, RGB(248, 250, 252));
        TextOutA(memDC, 60, 100, "1. TRI-EPOCH SYSTEM: You exist simultaneously across 3 synchronized eras:", 73);
        SetTextColor(memDC, RGB(245, 158, 11));
        TextOutA(memDC, 80, 125, "• Alpha (1984 - Past), Beta (2042 - Present), Gamma (2188 - Future).", 67);
        SetTextColor(memDC, RGB(148, 163, 184));
        TextOutA(memDC, 80, 145, "  Press [Tab] or [1 / 2 / 3] to shift your temporal perspective.", 64);

        SetTextColor(memDC, RGB(248, 250, 252));
        TextOutA(memDC, 60, 185, "2. CAUSAL RIPPLE ENGINE:", 24);
        SetTextColor(memDC, RGB(6, 182, 212));
        TextOutA(memDC, 80, 210, "• Actions in earlier epochs propagate forward into future reality.", 66);
        SetTextColor(memDC, RGB(148, 163, 184));
        TextOutA(memDC, 80, 230, "  Repairing a relay in Alpha powers blast gates in Beta and Gamma!", 65);

        SetTextColor(memDC, RGB(248, 250, 252));
        TextOutA(memDC, 60, 270, "3. CHRONO-ECHO GHOST SYSTEM:", 28);
        SetTextColor(memDC, RGB(192, 132, 252));
        TextOutA(memDC, 80, 295, "• Press [R] to record your footsteps, then [P] to execute an Echo.", 67);
        SetTextColor(memDC, RGB(148, 163, 184));
        TextOutA(memDC, 80, 315, "  A temporal clone will repeat your run to hold switches with you!", 66);

        SetTextColor(memDC, RGB(16, 185, 129));
        TextOutA(memDC, 60, 390, "Press [SPACE] or [ENTER] to acknowledge and begin mission.", 58);

    } else if (g_appState == STATE_PLAYING || g_appState == STATE_VICTORY) {
        // Render Top HUD Bar
        HBRUSH hudBrush = CreateSolidBrush(RGB(12, 18, 34));
        RECT rBar = { 0, 0, w, 44 };
        FillRect(memDC, &rBar, hudBrush);
        DeleteObject(hudBrush);

        SetTextColor(memDC, RGB(248, 250, 252));
        TextOutA(memDC, 14, 12, "KCHRONO", 7);

        // Epoch Badges
        COLORREF epCol = (g_game.epoch == EPOCH_ALPHA) ? RGB(245, 158, 11) : (g_game.epoch == EPOCH_BETA) ? RGB(6, 182, 212) : RGB(192, 132, 252);
        SetTextColor(memDC, epCol);
        if (g_game.epoch == EPOCH_ALPHA) {
            TextOutA(memDC, 110, 12, "[1] EPOCH ALPHA (1984 - Past)", 29);
        } else if (g_game.epoch == EPOCH_BETA) {
            TextOutA(memDC, 110, 12, "[2] EPOCH BETA (2042 - Present)", 31);
        } else {
            TextOutA(memDC, 110, 12, "[3] EPOCH GAMMA (2188 - Future)", 31);
        }

        // Paradox Strain Gauge
        SetTextColor(memDC, RGB(148, 163, 184));
        char strainText[32];
        wsprintfA(strainText, "Strain: %d%%", g_game.paradoxStrain);
        TextOutA(memDC, 450, 12, strainText, lstrlenA(strainText));

        char turnText[32];
        wsprintfA(turnText, "Turn: %d", g_game.turn);
        TextOutA(memDC, 570, 12, turnText, lstrlenA(turnText));

        if (g_isRecordingEcho) {
            SetTextColor(memDC, RGB(239, 68, 68));
            TextOutA(memDC, 670, 12, "[REC ECHO]", 10);
        } else if (g_isPlayingEcho) {
            SetTextColor(memDC, RGB(6, 182, 212));
            TextOutA(memDC, 670, 12, "[PLAY ECHO]", 11);
        }

        // Render Simulation Grid
        int ox = 20;
        int oy = 60;

        for (int y = 0; y < GRID_H; y++) {
            for (int x = 0; x < GRID_W; x++) {
                int tile = g_game.grids[g_game.epoch][y][x];
                RECT rTile = { ox + x * TILE_SZ, oy + y * TILE_SZ, ox + (x + 1) * TILE_SZ, oy + (y + 1) * TILE_SZ };

                COLORREF tileColor = RGB(15, 23, 42);
                if (tile == TILE_WALL) {
                    tileColor = RGB(30, 41, 59);
                } else if (tile == TILE_FLOOR) {
                    tileColor = (g_game.epoch == EPOCH_ALPHA) ? RGB(24, 18, 10) : (g_game.epoch == EPOCH_BETA) ? RGB(10, 17, 32) : RGB(19, 9, 31);
                } else if (tile == TILE_RELAY) {
                    tileColor = g_game.precursorRepaired ? RGB(245, 158, 11) : RGB(120, 53, 15);
                } else if (tile == TILE_GATE) {
                    tileColor = RGB(185, 28, 28);
                } else if (tile == TILE_PLATE) {
                    tileColor = RGB(6, 182, 212);
                } else if (tile == TILE_CORE) {
                    tileColor = RGB(56, 189, 248);
                } else if (tile == TILE_RIFT) {
                    tileColor = RGB(168, 85, 247);
                }

                HBRUSH tBrush = CreateSolidBrush(tileColor);
                FillRect(memDC, &rTile, tBrush);
                DeleteObject(tBrush);

                // Tile Border
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

        // Render Crates
        for (int i = 0; i < g_game.entityCount[g_game.epoch]; i++) {
            Entity* e = &g_game.entities[g_game.epoch][i];
            if (e->type == 1) {
                RECT rCrate = { ox + e->x * TILE_SZ + 2, oy + e->y * TILE_SZ + 2, ox + (e->x + 1) * TILE_SZ - 2, oy + (e->y + 1) * TILE_SZ - 2 };
                HBRUSH cBrush = CreateSolidBrush(RGB(180, 83, 9));
                FillRect(memDC, &rCrate, cBrush);
                DeleteObject(cBrush);
            }
        }

        // Render Echo Ghost
        if (g_hasEchoGhost && g_echoGhost.epoch == g_game.epoch) {
            RECT rGhost = { ox + g_echoGhost.x * TILE_SZ + 4, oy + g_echoGhost.y * TILE_SZ + 4, ox + (g_echoGhost.x + 1) * TILE_SZ - 4, oy + (g_echoGhost.y + 1) * TILE_SZ - 4 };
            HBRUSH gBrush = CreateSolidBrush(RGB(6, 182, 212));
            FillRect(memDC, &rGhost, gBrush);
            DeleteObject(gBrush);
        }

        // Render Player
        int px = ox + g_game.playerX * TILE_SZ + 3;
        int py = oy + g_game.playerY * TILE_SZ + 3;
        HBRUSH pBrush = CreateSolidBrush(RGB(255, 255, 255));
        HBRUSH oldB = (HBRUSH)SelectObject(memDC, pBrush);
        Ellipse(memDC, px, py, px + TILE_SZ - 6, py + TILE_SZ - 6);
        SelectObject(memDC, oldB);
        DeleteObject(pBrush);

        // Render Ripple Circle
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
        // Cleanup dead particles
        int alive = 0;
        for (int i = 0; i < g_particleCount; i++) {
            if (g_particles[i].life > 0) g_particles[alive++] = g_particles[i];
        }
        g_particleCount = alive;

        // Bottom Controls Hint
        SetTextColor(memDC, RGB(148, 163, 184));
        TextOutA(memDC, 20, h - 30, "[WASD/Arrows] Move  [Space] Wait  [1/2/3/Tab] Epoch  [R] Record  [P] Play  [F5/F9] Save/Load  [Esc] Menu", 99);

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
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case WM_KEYDOWN: {
            if (g_appState == STATE_SPLASH) {
                if (wParam == 'N' || wParam == 'n') {
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
                } else if (wParam == 'C' || wParam == 'c') {
                    if (Quickload()) {
                        g_appState = STATE_PLAYING;
                    }
                } else if (wParam == 'H' || wParam == 'h') {
                    g_appState = STATE_TUTORIAL;
                } else if (wParam == 'Q' || wParam == 'q') {
                    PostQuitMessage(0);
                }
            } else if (g_appState == STATE_TUTORIAL) {
                if (wParam == VK_SPACE || wParam == VK_RETURN || wParam == VK_ESCAPE) {
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
                else if (wParam == VK_ESCAPE) g_appState = STATE_SPLASH;
            } else if (g_appState == STATE_VICTORY) {
                if (wParam == VK_RETURN || wParam == VK_ESCAPE || wParam == VK_SPACE) {
                    g_appState = STATE_SPLASH;
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

    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME, "KChrono - Core Paradox Engine v1.0.0",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 840, 560,
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
