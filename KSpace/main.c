#include <math.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int _fltused = 1;
long _ftol2_sse(float f) { return (long)f; }
long _ftol2(float f) { return (long)f; }

#define W 320
#define H 480
#define MAX_BULLETS 50
#define MAX_ENEMIES 35
#define MAX_EBULLETS 60
#define MAX_POWERUPS 12
#define MAX_STARS 60
#define MAX_PARTICLES 120
#define MAX_LEADERBOARD 5

// --- GAME STATES ---
#define STATE_MENU 0
#define STATE_MODE_SELECT 1
#define STATE_PLAYING 2
#define STATE_PAUSED 3
#define STATE_LEADERBOARD 4
#define STATE_GAMEOVER 5
#define STATE_VICTORY 6
#define STATE_HELP 7
#define STATE_KEYBINDS 8
#define STATE_REPLAY 9

static const signed char s_SinTable[16] = {
    0, 48, 90, 117, 127, 117, 90, 48,
    0, -48, -90, -117, -127, -117, -90, -48
};
static int FastSin(int angle) {
    return (int)s_SinTable[(angle & 15)];
}
static int FastCos(int angle) {
    return (int)s_SinTable[((angle + 4) & 15)];
}

int kbUp = VK_UP, kbDown = VK_DOWN, kbLeft = VK_LEFT, kbRight = VK_RIGHT;
int kbFire = VK_SPACE, kbTimeStop = 'T', kbDash = 'D', kbBomb = 'B', kbShield = 'S', kbPause = 'P', kbOvercharge = 'O';

int shotsFired = 0, shotsHit = 0, timeSurvivedFrames = 0;
int isReplaying = 0;
unsigned int initialSeed = 999;

// --- GAME MODES ---
#define MODE_CLASSIC 0
#define MODE_ENDURANCE 1
#define MODE_BOSS_RUSH 2

#define MAX_SHOCKWAVES 20
#define MAX_DEBRIS 35
#define MAX_RIPPLES 20
#define MAX_FLASHES 20
#define MAX_STRIKES 6

typedef struct { float x, y, active, dx, dy, type; int hp, maxHp, timer, cloaked; int isElite, squadId, shield; } Ent;
typedef struct { float x, y, speed; int size, layer; } Star;
typedef struct { float x, y, vx, vy; int life, maxLife; COLORREF color; int layer; float size; } Particle;
typedef struct { int score, wave, mode; } LeaderEntry;

typedef struct { float x, y, r, maxR, speed, alpha; COLORREF color; int life; int isOuter; } Shockwave;
typedef struct { float x, y, vx, vy, size, rot, vrot, life, decay; COLORREF color; int shape; } Debris;
typedef struct { float x, y, r, maxR, alpha; COLORREF color; int life; } ShieldRipple;
typedef struct { float x, y, size; int life, maxLife; COLORREF color; } MuzzleFlash;
typedef struct { float x, y, r, vx, vy, phase; COLORREF col1; } Nebula;

typedef struct { float x; int timer, delay, active; int width; } OrbitalStrike;

typedef struct {
    int score, wave, mode;
    int playerHp, shieldActive, hyperShieldTimer, hyperShieldCooldown, bombs, weaponType;
    int spreadTimer, laserTimer, rapidTimer, timeStopTimer, timeStopCooldown, dashCooldown, invincibleTimer;
    int overchargeEnergy, overchargeTimer, bombardmentActive;
    float px, py;
    int enemiesKilled;
    int bossActive, bossHp, bossMaxHp, bossLevel, bossIsMothership;
    float bossX, bossY;
    int turretHp[4], turretActive[4];
    int bossIsDreadnought, dreadGenL, dreadGenR, droneCount, hyperJumpEnergy;
} SaveState;

// --- GLOBAL GAME DATA ---
int gameState = STATE_MENU;
int previousState = STATE_MENU;
int menuIndex = 0;
int modeIndex = MODE_CLASSIC;
static HFONT hFontTitle = NULL;
static HFONT hFontMenu = NULL;
static HFONT hFontHUD = NULL;

Ent p = { W/2.0f - 10.0f, H - 60.0f, 1.0f, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0 };
int shieldActive = 0;
int hyperShieldTimer = 0;
int hyperShieldCooldown = 0;
int bombCount = 1;
int maxBombs = 3;
int weaponType = 0; // 0: Normal, 1: Spread, 2: Laser, 3: Plasma

// Loop 10: Weapon Overcharge
int overchargeEnergy = 0; // 0 to 100
int overchargeTimer = 0;  // Frames remaining for Overcharge

// Loop 10: Elite Enemy Squads
int eliteSquadActive = 0;
int eliteSquadTimer = 0;
int eliteSquadType = 0;
char eliteSquadName[32] = "CRIMSON VALKYRIES";

// Loop 10: Planetary Bombardment Missions
int bombardmentActive = 0;
int bombardmentTimer = 0;
int bombardmentBanner = 0;
OrbitalStrike strikes[MAX_STRIKES] = {0};

// Loop 11: Drone Companion Wings
int droneCount = 0; // 0, 1, or 2 support combat drones
typedef struct { float x, y, targetX, targetY; int shootTimer; } DroneWing;
DroneWing drones[2] = {0};

// Loop 11: Hyper-Jump Drive
int hyperJumpEnergy = 0; // 0 to 100%
int hyperJumpTimer = 0;  // Frames for warp distortion
int kbHyperJump = 'J';
int kbDeployDrone = 'W';

// Loop 11: Capital Ship Dreadnought Sieges
int bossIsDreadnought = 0;
int dreadGenL = 80, dreadMaxGenL = 80;
int dreadGenR = 80, dreadMaxGenR = 80;
int dreadIonCharge = 0;    // 0 to 100
int dreadIonBeamTimer = 0; // Active mega-beam frames
int dreadFlakTimer = 0;

Ent b[MAX_BULLETS] = {0};
Ent e[MAX_ENEMIES] = {0};
Ent eb[MAX_EBULLETS] = {0};
Ent pu[MAX_POWERUPS] = {0};
Particle particles[MAX_PARTICLES] = {0};
Star stars[MAX_STARS] = {0};

// Loop 2 Visual Collections
Shockwave shockwaves[MAX_SHOCKWAVES] = {0};
Debris debris[MAX_DEBRIS] = {0};
ShieldRipple ripples[MAX_RIPPLES] = {0};
MuzzleFlash flashes[MAX_FLASHES] = {0};

typedef struct { float x, y, r, rot, vx, vy; int type; COLORREF col; } Planet;
typedef struct { float x, y, vx, vy, life; } Comet;

Nebula nebulae[3] = {
    { W * 0.25f, H * 0.3f, 90.0f, 0.12f, 0.2f, 0.0f, RGB(75, 0, 130) },
    { W * 0.75f, H * 0.7f, 110.0f, -0.09f, 0.15f, 1.5f, RGB(139, 0, 139) },
    { W * 0.45f, H * 0.1f, 80.0f, 0.07f, 0.25f, 3.0f, RGB(0, 80, 180) }
};

Planet planets[2] = {
    { W * 0.8f, H * 0.2f, 40.0f, 0.0f, -0.02f, 0.05f, 0, RGB(0, 77, 64) },
    { W * 0.1f, H * 0.7f, 25.0f, 0.0f, 0.01f, 0.03f, 1, RGB(183, 28, 28) }
};
Comet comets[5] = {0};

int spreadTimer = 0;
int laserTimer = 0;
int rapidTimer = 0;
int timeStopTimer = 0;
int timeStopCooldown = 0;
int dashCooldown = 0;
int invincibleTimer = 0;

int score = 0;
int highScore = 0;
int totalKills = 0;
int enemiesKilled = 0;
int comboMultiplier = 1;
int comboTimer = 0;
int weaponLevel = 0;
int wave = 1;
int frameCount = 0;
int bombFlash = 0;
int bossDeathFlash = 0;
int screenShake = 0;

// Boss & Mothership state
int bossActive = 0;
float bossX = 0, bossY = 0, bossDx = 2.0f;
int bossHp = 0, bossMaxHp = 0, bossLevel = 1, bossAttackTimer = 0, bossPhase = 1;
int bossIsMothership = 0;
int turretHp[4] = {0};
int turretActive[4] = {1, 1, 1, 1};

LeaderEntry leaderboard[MAX_LEADERBOARD] = {
    { 12000, 20, MODE_CLASSIC },
    { 8500, 15, MODE_CLASSIC },
    { 5000, 10, MODE_ENDURANCE },
    { 3500, 7, MODE_BOSS_RUSH },
    { 1500, 4, MODE_CLASSIC }
};

typedef struct { float x, y, dy; int hp, maxHp, active; } EscortShip;
EscortShip escort = {0};
int pathGatesActive = 0;
float pathGatesY = 0.0f;

unsigned int seed = 999;
unsigned int rnd() {
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

// Sound Thread
DWORD WINAPI SndThread(LPVOID param) {
    int type = (int)(intptr_t)param;
    if (type == 0) Beep(900, 25);         // Shoot
    else if (type == 1) Beep(200, 50);    // Explosion
    else if (type == 2) { Beep(520, 40); Beep(780, 40); Beep(1040, 50); } // Powerup
    else if (type == 3) { Beep(350, 60); Beep(550, 60); Beep(750, 80); }  // Wave / Stasis
    else if (type == 4) Beep(150, 120);   // Smart Bomb
    else if (type == 5) Beep(1100, 20);   // Laser
    else if (type == 6) { Beep(400, 80); Beep(600, 80); Beep(800, 100); Beep(1200, 150); } // Victory
    else if (type == 7) { Beep(600, 30); Beep(900, 30); Beep(1300, 40); Beep(1800, 60); } // Overcharge
    else if (type == 8) { Beep(300, 70); Beep(220, 70); Beep(180, 100); } // Tactical siren
    else if (type == 9) { Beep(1200, 35); Beep(800, 35); Beep(400, 45); Beep(950, 60); } // Hyper-Jump Warp
    else if (type == 10) Beep(1400, 18);  // Drone Vulcan / Point Defense
    else if (type == 11) { Beep(260, 40); Beep(190, 70); } // Dreadnought Mega-Ion Beam
    return 0;
}

void PlaySnd(int type) {
    HANDLE hThread = CreateThread(NULL, 0, SndThread, (LPVOID)(intptr_t)type, 0, NULL);
    if (hThread) CloseHandle(hThread);
}

// Loop 8 Visual Effects Generators
void AddShockwave(float x, float y, float maxR, COLORREF col) {
    // Primary compression shockwave ring
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (shockwaves[i].life <= 0) {
            shockwaves[i].x = x; shockwaves[i].y = y;
            shockwaves[i].r = 4.0f; shockwaves[i].maxR = maxR;
            shockwaves[i].speed = 2.4f; shockwaves[i].alpha = 1.0f;
            shockwaves[i].color = col; shockwaves[i].life = 1;
            shockwaves[i].isOuter = 0;
            break;
        }
    }
    // Secondary outer ion dispersion halo
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (shockwaves[i].life <= 0) {
            shockwaves[i].x = x; shockwaves[i].y = y;
            shockwaves[i].r = 2.0f; shockwaves[i].maxR = maxR * 1.35f;
            shockwaves[i].speed = 3.4f; shockwaves[i].alpha = 0.8f;
            shockwaves[i].color = RGB(255, 255, 255); shockwaves[i].life = 1;
            shockwaves[i].isOuter = 1;
            break;
        }
    }
}

void AddDebrisChunk(float x, float y, int count, COLORREF col) {
    int added = 0;
    for (int i = 0; i < MAX_DEBRIS && added < count; i++) {
        if (debris[i].life <= 0.0f) {
            debris[i].x = x; debris[i].y = y;
            debris[i].vx = (float)((int)(rnd() % 11) - 5) * 0.7f;
            debris[i].vy = (float)((int)(rnd() % 11) - 5) * 0.7f;
            debris[i].rot = (float)(rnd() % 628) / 100.0f;
            debris[i].vrot = (float)((int)(rnd() % 10) - 5) * 0.05f;
            debris[i].size = 3.0f + (rnd() % 4);
            debris[i].color = col;
            debris[i].shape = rnd() % 3;
            debris[i].life = 1.0f;
            debris[i].decay = 0.018f + (float)(rnd() % 20) / 1000.0f;
            added++;
        }
    }
}

void AddShieldRipple(float x, float y, COLORREF col) {
    for (int i = 0; i < MAX_RIPPLES; i++) {
        if (ripples[i].life <= 0) {
            ripples[i].x = x; ripples[i].y = y;
            ripples[i].r = 6.0f; ripples[i].maxR = 26.0f;
            ripples[i].alpha = 1.0f; ripples[i].color = col; ripples[i].life = 1;
            break;
        }
    }
}

void AddMuzzleFlash(float x, float y, COLORREF col, float size) {
    for (int i = 0; i < MAX_FLASHES; i++) {
        if (flashes[i].life <= 0) {
            flashes[i].x = x; flashes[i].y = y;
            flashes[i].size = size; flashes[i].life = 3; flashes[i].maxLife = 3;
            flashes[i].color = col;
            break;
        }
    }
}

// Particle Explosions (Loop 8 Multi-Layer Kinematic Engine)
void AddExplosion(float x, float y, int count, COLORREF col) {
    screenShake += count / 3;
    if (screenShake > 25) screenShake = 25;
    
    // Layer 0: Incandescent core needle sparks
    int sparkCount = (count * 3) / 4;
    if (sparkCount < 6) sparkCount = 6;
    int added = 0;
    for (int i = 0; i < MAX_PARTICLES && added < sparkCount; i++) {
        if (particles[i].life <= 0) {
            particles[i].x = x;
            particles[i].y = y;
            float spd = 2.0f + (float)(rnd() % 15) * 0.25f;
            int angleIdx = rnd() % 16;
            particles[i].vx = ((float)FastCos(angleIdx) / 127.0f) * spd;
            particles[i].vy = ((float)FastSin(angleIdx) / 127.0f) * spd;
            particles[i].life = 10 + (rnd() % 12);
            particles[i].maxLife = particles[i].life;
            particles[i].color = (added % 2 == 0) ? RGB(255, 255, 255) : ((rnd() % 2 == 0) ? RGB(0, 229, 255) : RGB(255, 234, 0));
            particles[i].layer = 0;
            particles[i].size = 1.5f;
            added++;
        }
    }

    // Layer 1: Expanding buoyant plasma smoke puffs
    int smokeCount = count / 2;
    if (smokeCount < 4) smokeCount = 4;
    added = 0;
    for (int i = 0; i < MAX_PARTICLES && added < smokeCount; i++) {
        if (particles[i].life <= 0) {
            particles[i].x = x;
            particles[i].y = y;
            float spd = 0.5f + (float)(rnd() % 10) * 0.15f;
            int angleIdx = rnd() % 16;
            particles[i].vx = ((float)FastCos(angleIdx) / 127.0f) * spd;
            particles[i].vy = ((float)FastSin(angleIdx) / 127.0f) * spd - 0.4f;
            particles[i].life = 18 + (rnd() % 16);
            particles[i].maxLife = particles[i].life;
            particles[i].color = (rnd() % 2 == 0) ? RGB(255, 60, 0) : RGB(255, 145, 0);
            particles[i].layer = 1;
            particles[i].size = 3.0f + (float)(rnd() % 3);
            added++;
        }
    }

    // Layer 2: Heavy armor debris chunks
    if (count >= 10) {
        AddDebrisChunk(x, y, (count / 2 > 12 ? 12 : count / 2), col);
    }

    // Layer 3: Radiant celebration energy stars
    if (count >= 12) {
        int starCount = count / 3;
        if (starCount > 6) starCount = 6;
        added = 0;
        for (int i = 0; i < MAX_PARTICLES && added < starCount; i++) {
            if (particles[i].life <= 0) {
                particles[i].x = x;
                particles[i].y = y;
                float spd = 1.0f + (float)(rnd() % 10) * 0.2f;
                int angleIdx = (added * 16 / starCount) & 15;
                particles[i].vx = ((float)FastCos(angleIdx) / 127.0f) * spd;
                particles[i].vy = ((float)FastSin(angleIdx) / 127.0f) * spd;
                particles[i].life = 20 + (rnd() % 10);
                particles[i].maxLife = particles[i].life;
                particles[i].color = (added % 2 == 0) ? RGB(255, 215, 0) : RGB(255, 255, 255);
                particles[i].layer = 3;
                particles[i].size = 4.0f + (float)(rnd() % 3);
                added++;
            }
        }
    }

    if (count >= 10) {
        AddShockwave(x, y, (float)count * 2.4f, col);
    }
}

void AddWeaponHitParticles(float x, float y, int weaponType, COLORREF col) {
    int count = (weaponType == 1) ? 6 : ((weaponType == 2) ? 2 : 3);
    AddExplosion(x, y, count, col);
}

void UpdateParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0) {
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            if (particles[i].layer == 1) {
                particles[i].vx *= 0.92f;
                particles[i].vy *= 0.92f;
                particles[i].vy -= 0.05f; // buoyant drift
            } else if (particles[i].layer == 0) {
                particles[i].vx *= 0.96f;
                particles[i].vy *= 0.96f;
            } else if (particles[i].layer == 3) {
                particles[i].vx *= 0.94f;
                particles[i].vy *= 0.94f;
            }
            particles[i].life--;
        }
    }
}

void DrawParticles(HDC hdc) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0) {
            if (particles[i].layer == 0) {
                // Needle spark along motion vector
                HPEN ppen = CreatePen(PS_SOLID, (int)particles[i].size > 0 ? (int)particles[i].size : 1, particles[i].color);
                HPEN oldP = (HPEN)SelectObject(hdc, ppen);
                MoveToEx(hdc, (int)particles[i].x, (int)particles[i].y, NULL);
                LineTo(hdc, (int)(particles[i].x - particles[i].vx * 2.2f), (int)(particles[i].y - particles[i].vy * 2.2f));
                SelectObject(hdc, oldP);
                DeleteObject(ppen);
            } else if (particles[i].layer == 1) {
                // Expanding plasma smoke puff
                int r = (int)(particles[i].size * (0.8f + (1.0f - (float)particles[i].life / (float)particles[i].maxLife) * 1.4f));
                HBRUSH pbr = CreateSolidBrush(particles[i].color);
                HBRUSH oldBr = (HBRUSH)SelectObject(hdc, pbr);
                HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
                HPEN oldP = (HPEN)SelectObject(hdc, nullPen);
                Ellipse(hdc, (int)particles[i].x - r, (int)particles[i].y - r, (int)particles[i].x + r, (int)particles[i].y + r);
                SelectObject(hdc, oldBr);
                SelectObject(hdc, oldP);
                DeleteObject(pbr);
            } else if (particles[i].layer == 3) {
                // Radiant 4-point energy star
                int sz = (int)particles[i].size;
                HPEN spen = CreatePen(PS_SOLID, 2, particles[i].color);
                HPEN oldP = (HPEN)SelectObject(hdc, spen);
                MoveToEx(hdc, (int)particles[i].x - sz, (int)particles[i].y, NULL);
                LineTo(hdc, (int)particles[i].x + sz, (int)particles[i].y);
                MoveToEx(hdc, (int)particles[i].x, (int)particles[i].y - sz, NULL);
                LineTo(hdc, (int)particles[i].x, (int)particles[i].y + sz);
                SelectObject(hdc, oldP);
                DeleteObject(spen);
            } else {
                HBRUSH pbr = CreateSolidBrush(particles[i].color);
                int sz = (particles[i].life > 8) ? 3 : 2;
                RECT pr = {(int)particles[i].x, (int)particles[i].y, (int)particles[i].x + sz, (int)particles[i].y + sz};
                FillRect(hdc, &pr, pbr);
                DeleteObject(pbr);
            }
        }
    }
}

// Registry Persistence
void LoadLeaderboard() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\KSpace\\Leaderboard", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD type = REG_DWORD;
        DWORD size = sizeof(LeaderEntry) * MAX_LEADERBOARD;
        RegQueryValueExA(hKey, "Entries", NULL, &type, (LPBYTE)leaderboard, &size);
        RegCloseKey(hKey);
    }
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\KSpace", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD type = REG_DWORD;
        DWORD size = sizeof(DWORD);
        RegQueryValueExA(hKey, "HighScore", NULL, &type, (LPBYTE)&highScore, &size);
        size = sizeof(DWORD);
        RegQueryValueExA(hKey, "TotalKills", NULL, &type, (LPBYTE)&totalKills, &size);
        RegCloseKey(hKey);
    }
}

void SaveLeaderboard() {
    HKEY hKey;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\KSpace\\Leaderboard", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "Entries", 0, REG_BINARY, (const BYTE*)leaderboard, sizeof(LeaderEntry) * MAX_LEADERBOARD);
        RegCloseKey(hKey);
    }
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\KSpace", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "HighScore", 0, REG_DWORD, (const BYTE*)&highScore, sizeof(DWORD));
        RegSetValueExA(hKey, "TotalKills", 0, REG_DWORD, (const BYTE*)&totalKills, sizeof(DWORD));
        RegCloseKey(hKey);
    }
}

void ExportStatsCSV() {
    HANDLE hFile = CreateFileA("kspace_stats.csv", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    float acc = shotsFired > 0 ? ((float)shotsHit / shotsFired) * 100.0f : 0.0f;
    char buf[256];
    wsprintfA(buf, "Score,Wave,Mode,EnemiesKilled,TimeSurvivedSec,ShotsFired,ShotsHit,AccuracyPct\r\n%d,%d,%d,%d,%d,%d,%d,%d\r\n", score, wave, modeIndex, enemiesKilled, timeSurvivedFrames/60, shotsFired, shotsHit, (int)acc);
    DWORD written;
    WriteFile(hFile, buf, lstrlenA(buf), &written, NULL);
    CloseHandle(hFile);
}
void ExportStatsJSON() {
    HANDLE hFile = CreateFileA("kspace_stats.json", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    float acc = shotsFired > 0 ? ((float)shotsHit / shotsFired) * 100.0f : 0.0f;
    char buf[512];
    wsprintfA(buf, "{\r\n\"score\": %d,\r\n\"wave\": %d,\r\n\"enemiesKilled\": %d,\r\n\"timeSurvivedSeconds\": %d,\r\n\"shotsFired\": %d,\r\n\"shotsHit\": %d,\r\n\"accuracyPct\": %d\r\n}\r\n", score, wave, enemiesKilled, timeSurvivedFrames/60, shotsFired, shotsHit, (int)acc);
    DWORD written;
    WriteFile(hFile, buf, lstrlenA(buf), &written, NULL);
    CloseHandle(hFile);
}
void ExportHighScoresJSON() {
    HANDLE hFile = CreateFileA("kspace_highscores.json", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    DWORD written;
    char buf[512];
    lstrcpyA(buf, "[\r\n");
    WriteFile(hFile, buf, lstrlenA(buf), &written, NULL);
    for(int i=0; i<MAX_LEADERBOARD; i++) {
        wsprintfA(buf, "  {\"score\":%d, \"wave\":%d, \"mode\":%d}%s\r\n", leaderboard[i].score, leaderboard[i].wave, leaderboard[i].mode, i==MAX_LEADERBOARD-1 ? "" : ",");
        WriteFile(hFile, buf, lstrlenA(buf), &written, NULL);
    }
    lstrcpyA(buf, "]\r\n");
    WriteFile(hFile, buf, lstrlenA(buf), &written, NULL);
    CloseHandle(hFile);
}

void AddScoreToLeaderboard(int newScore, int newWave, int mode) {
    int insertIdx = -1;
    for (int i = 0; i < MAX_LEADERBOARD; i++) {
        if (newScore > leaderboard[i].score) {
            insertIdx = i;
            break;
        }
    }
    if (insertIdx != -1) {
        for (int i = MAX_LEADERBOARD - 1; i > insertIdx; i--) {
            leaderboard[i] = leaderboard[i - 1];
        }
        leaderboard[insertIdx].score = newScore;
        leaderboard[insertIdx].wave = newWave;
        leaderboard[insertIdx].mode = mode;
        SaveLeaderboard();
    }
}

int HasSavedGame() {
    HKEY hKey;
    int found = 0;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\KSpace\\Save", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        found = 1;
        RegCloseKey(hKey);
    }
    return found;
}

void SaveGameState() {
    HKEY hKey;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\KSpace\\Save", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        SaveState s;
        s.score = score;
        s.wave = wave;
        s.mode = modeIndex;
        s.playerHp = p.hp;
        s.shieldActive = shieldActive;
        s.hyperShieldTimer = hyperShieldTimer;
        s.hyperShieldCooldown = hyperShieldCooldown;
        s.bombs = bombCount;
        s.weaponType = weaponType;
        s.spreadTimer = spreadTimer;
        s.laserTimer = laserTimer;
        s.rapidTimer = rapidTimer;
        s.timeStopTimer = timeStopTimer;
        s.timeStopCooldown = timeStopCooldown;
        s.dashCooldown = dashCooldown;
        s.invincibleTimer = invincibleTimer;
        s.overchargeEnergy = overchargeEnergy;
        s.overchargeTimer = overchargeTimer;
        s.bombardmentActive = bombardmentActive;
        s.px = p.x;
        s.py = p.y;
        s.enemiesKilled = enemiesKilled;
        s.bossActive = bossActive;
        s.bossHp = bossHp;
        s.bossMaxHp = bossMaxHp;
        s.bossLevel = bossLevel;
        s.bossIsMothership = bossIsMothership;
        s.bossX = bossX;
        s.bossY = bossY;
        for (int i = 0; i < 4; i++) {
            s.turretHp[i] = turretHp[i];
            s.turretActive[i] = turretActive[i];
        }
        s.bossIsDreadnought = bossIsDreadnought;
        s.dreadGenL = dreadGenL;
        s.dreadGenR = dreadGenR;
        s.droneCount = droneCount;
        s.hyperJumpEnergy = hyperJumpEnergy;

        RegSetValueExA(hKey, "Data", 0, REG_BINARY, (const BYTE*)&s, sizeof(SaveState));
        RegCloseKey(hKey);
        PlaySnd(2);
    }
}

int LoadGameState() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\KSpace\\Save", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        SaveState s;
        DWORD type = REG_BINARY;
        DWORD size = sizeof(SaveState);
        if (RegQueryValueExA(hKey, "Data", NULL, &type, (LPBYTE)&s, &size) == ERROR_SUCCESS) {
            score = s.score;
            wave = s.wave;
            modeIndex = s.mode;
            p.hp = s.playerHp;
            shieldActive = s.shieldActive;
            hyperShieldTimer = s.hyperShieldTimer;
            hyperShieldCooldown = s.hyperShieldCooldown;
            bombCount = s.bombs;
            weaponType = s.weaponType;
            spreadTimer = s.spreadTimer;
            laserTimer = s.laserTimer;
            rapidTimer = s.rapidTimer;
            timeStopTimer = s.timeStopTimer;
            timeStopCooldown = s.timeStopCooldown;
            dashCooldown = s.dashCooldown;
            invincibleTimer = s.invincibleTimer;
            overchargeEnergy = s.overchargeEnergy;
            overchargeTimer = s.overchargeTimer;
            bombardmentActive = s.bombardmentActive;
            p.x = s.px;
            p.y = s.py;
            enemiesKilled = s.enemiesKilled;
            bossActive = s.bossActive;
            bossHp = s.bossHp;
            bossMaxHp = s.bossMaxHp;
            bossLevel = s.bossLevel;
            bossIsMothership = s.bossIsMothership;
            bossX = s.bossX;
            bossY = s.bossY;
            for (int i = 0; i < 4; i++) {
                turretHp[i] = s.turretHp[i];
                turretActive[i] = s.turretActive[i];
            }
            bossIsDreadnought = s.bossIsDreadnought;
            dreadGenL = s.dreadGenL;
            dreadGenR = s.dreadGenR;
            droneCount = s.droneCount;
            hyperJumpEnergy = s.hyperJumpEnergy;

            for (int i = 0; i < MAX_ENEMIES; i++) e[i].active = 0;
            for (int i = 0; i < MAX_BULLETS; i++) b[i].active = 0;
            for (int i = 0; i < MAX_EBULLETS; i++) eb[i].active = 0;
            for (int i = 0; i < MAX_POWERUPS; i++) pu[i].active = 0;
            for (int i = 0; i < MAX_PARTICLES; i++) particles[i].life = 0;

            gameState = STATE_PLAYING;
            RegCloseKey(hKey);
            PlaySnd(3);
            return 1;
        }
        RegCloseKey(hKey);
    }
    return 0;
}

void ClearSavedGame() {
    RegDeleteKeyA(HKEY_CURRENT_USER, "Software\\KSpace\\Save");
}

int IsMothershipShieldActive() {
    if (!bossIsMothership) return 0;
    for (int i = 0; i < 4; i++) {
        if (turretActive[i]) return 1;
    }
    return 0;
}

void SpawnBoss(int lvl) {
    bossActive = 1;
    bossLevel = lvl;
    bossAttackTimer = 0;
    bossPhase = 1;

    if (wave == 8 || wave == 16 || (modeIndex == MODE_BOSS_RUSH && (lvl == 2 || lvl == 4))) {
        bossIsDreadnought = 1;
        bossIsMothership = 0;
        bossX = W / 2.0f - 60.0f;
        bossY = -75.0f;
        bossDx = 1.4f;
        bossMaxHp = 500 + lvl * 50;
        bossHp = bossMaxHp;
        dreadGenL = 80; dreadMaxGenL = 80;
        dreadGenR = 80; dreadMaxGenR = 80;
        dreadIonCharge = 0;
        dreadIonBeamTimer = 0;
        dreadFlakTimer = 0;
    } else if (wave >= 20 || (modeIndex == MODE_BOSS_RUSH && lvl >= 5) || lvl >= 4) {
        bossIsDreadnought = 0;
        bossIsMothership = 1;
        bossX = W / 2.0f - 45.0f;
        bossY = -70.0f;
        bossDx = 2.0f;
        bossMaxHp = 600;
        bossHp = bossMaxHp;
        for (int i = 0; i < 4; i++) {
            turretHp[i] = 80;
            turretActive[i] = 1;
        }
    } else {
        bossIsDreadnought = 0;
        bossIsMothership = 0;
        bossX = W / 2.0f - 30.0f;
        bossY = -60.0f;
        bossDx = 2.0f;
        bossMaxHp = 140 + lvl * 90;
        bossHp = bossMaxHp;
    }
    PlaySnd(4);
}

void DestroyBoss() {
    comboTimer = 180;
    if (comboMultiplier < 10) comboMultiplier++;
    int baseScore = bossIsDreadnought ? 8000 : (bossIsMothership ? 10000 : (1200 * bossLevel));
    score += baseScore * comboMultiplier;
    enemiesKilled += 10;
    totalKills += 10;
    hyperJumpEnergy += 50; if (hyperJumpEnergy > 100) hyperJumpEnergy = 100;
    overchargeEnergy += 40; if (overchargeEnergy > 100) overchargeEnergy = 100;

    AddExplosion(bossX + (bossIsDreadnought ? 60.0f : 45.0f), bossY + 30.0f, 60, RGB(255, 23, 68));
    bossDeathFlash = 150;
    bossActive = 0;
    bossIsDreadnought = 0;
    PlaySnd(3);

    // Drop Powerups (including guaranteed Drone Wing or Overcharge Core)
    for (int k = 0; k < MAX_POWERUPS; k++) {
        if (!pu[k].active) {
            pu[k].active = 1.0f;
            pu[k].x = bossX + 20.0f + (k % 3) * 30.0f;
            pu[k].y = bossY + 20.0f;
            pu[k].dy = 1.5f;
            pu[k].type = (k == 0 ? 10.0f : (float)(rnd() % 11)); // 10 = Drone Wing Pod
            if (k >= 2) break;
        }
    }

    if (bossIsMothership || wave >= 20) {
        gameState = STATE_VICTORY;
        PlaySnd(6);
        AddScoreToLeaderboard(score, wave, modeIndex);
        ClearSavedGame();
        return;
    }

    wave++;
    if (modeIndex == MODE_BOSS_RUSH) {
        SpawnBoss(wave);
    } else {
        pathGatesActive = 1;
        pathGatesY = -50.0f;
    }
}

void SpawnEnemy() {
    if (bossActive) return;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!e[i].active) {
            e[i].active = 1.0f;
            e[i].x = (float)(rnd() % (W - 28));
            e[i].y = -24.0f;
            e[i].timer = 0;
            e[i].cloaked = 0;

            int t = rnd() % 100;
            if (t < 25) e[i].type = 0.0f;       // Scout Interceptor
            else if (t < 38) e[i].type = 1.0f;  // Chaser Predator
            else if (t < 50) e[i].type = 2.0f;  // Shooter Saucer
            else if (t < 60) e[i].type = 3.0f;  // Armored Heavy
            else if (t < 70) { e[i].type = 4.0f; e[i].dx = (rnd()%2==0?2.2f:-2.2f); } // Zigzag
            else if (t < 78) e[i].type = 5.0f;  // Small Asteroid
            else if (t < 85) { e[i].type = 6.0f; e[i].dx = (rnd()%2==0?1.8f:-1.8f); } // Frigate
            else if (t < 92) e[i].type = 7.0f;  // Kamikaze Interceptor (Fast dive)
            else e[i].type = 8.0f;             // Stealth Cloak Fighter

            if (e[i].type == 9.0f) e[i].hp = 50;
            else if (e[i].type == 8.0f) e[i].hp = 18;
            else if (e[i].type == 7.0f) e[i].hp = 8;
            else if (e[i].type == 6.0f) e[i].hp = 45;
            else if (e[i].type == 5.0f) e[i].hp = 25;
            else if (e[i].type == 3.0f) e[i].hp = 12;
            else if (e[i].type == 2.0f) e[i].hp = 6;
            else if (e[i].type == 4.0f) e[i].hp = 4;
            else e[i].hp = 2;

            e[i].maxHp = e[i].hp;
            break;
        }
    }
}

void SpawnFormation(int type) {
    if (bossActive) return;
    float startX = 40.0f;
    for (int k = 0; k < 5; k++) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!e[i].active) {
                e[i].active = 1.0f;
                if (type == 9) {
                    e[i].x = W / 2.0f - 10.0f + (k - 2) * 35.0f;
                    float kDiff = (k - 2 < 0) ? -(k - 2) : (k - 2);
                    e[i].y = -20.0f - kDiff * 25.0f;
                    e[i].type = 2.0f; // Shooter Saucer
                    e[i].dx = 0.0f;
                } else {
                    e[i].x = startX + k * 48.0f;
                    if (e[i].x > W - 30) e[i].x = W - 30;
                    e[i].y = -20.0f - (k % 3) * 20.0f;
                    e[i].type = (float)type;
                    e[i].dx = (k % 2 == 0) ? 1.5f : -1.5f;
                }
                e[i].hp = (type == 7) ? 8 : ((type == 8) ? 18 : ((type == 9) ? 6 : 5));
                e[i].maxHp = e[i].hp;
                e[i].dy = 2.0f;
                e[i].timer = 0;
                e[i].cloaked = 0;
                break;
            }
        }
    }
}

// Active Skills
void UseTimeStop() {
    if (gameState != STATE_PLAYING) return;
    if (timeStopCooldown <= 0 || timeStopTimer > 0) {
        timeStopTimer = 375; // 6 seconds
        timeStopCooldown = 600; // 10 seconds CD
        PlaySnd(3);
        AddExplosion(W / 2.0f, H / 2.0f, 35, RGB(0, 229, 255));
    }
}

void UseTacticalDash() {
    if (gameState != STATE_PLAYING) return;
    if (dashCooldown <= 0) {
        AddExplosion(p.x + 10.0f, p.y + 10.0f, 25, RGB(0, 176, 255));
        p.x = W / 2.0f - 10.0f;
        p.y = H - 60.0f;
        invincibleTimer = 125; // 2 seconds
        dashCooldown = 300; // 5 seconds CD
        PlaySnd(2);
        AddExplosion(p.x + 10.0f, p.y + 10.0f, 25, RGB(0, 176, 255));
    }
}

void UseSmartBomb() {
    if (bombCount <= 0 || gameState != STATE_PLAYING) return;
    bombCount--;
    bombFlash = 15;
    PlaySnd(4);
    AddExplosion(W / 2.0f, H / 2.0f, 50, RGB(255, 255, 255));

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (e[i].active) {
            e[i].hp -= 100;
            if (e[i].hp <= 0) {
                e[i].active = 0.0f;
                comboTimer = 180;
                if (comboMultiplier < 10) comboMultiplier++;
                score += 30 * comboMultiplier;
                enemiesKilled++;
                totalKills++;
                AddExplosion(e[i].x + 10.0f, e[i].y + 10.0f, 16, RGB(0, 229, 255));
            }
        }
    }
    for (int i = 0; i < MAX_EBULLETS; i++) eb[i].active = 0;

    if (bossActive) {
        if (bossIsDreadnought) {
            if (dreadGenL > 0) { dreadGenL -= 40; if (dreadGenL < 0) dreadGenL = 0; }
            if (dreadGenR > 0) { dreadGenR -= 40; if (dreadGenR < 0) dreadGenR = 0; }
            bossHp -= 60;
        } else if (bossIsMothership) {
            for (int i = 0; i < 4; i++) {
                if (turretActive[i]) {
                    turretHp[i] -= 60;
                    if (turretHp[i] <= 0) turretActive[i] = 0;
                }
            }
            if (!IsMothershipShieldActive()) {
                bossHp -= 80;
            }
        } else {
            bossHp -= 80;
        }
        AddExplosion(bossX + 45.0f, bossY + 30.0f, 35, RGB(255, 23, 68));
        if (bossHp <= 0) {
            if (bossPhase == 1) {
                bossPhase = 2; bossMaxHp = (int)(bossMaxHp * 1.5f); bossHp = bossMaxHp;
                bossDx = (bossDx > 0 ? bossDx + 0.5f : bossDx - 0.5f);
                AddExplosion(bossX + 45.0f, bossY + 30.0f, 40, RGB(255, 234, 0));
            } else {
                DestroyBoss();
            }
        }
    }
}

void UseHyperShield() {
    if (gameState != STATE_PLAYING) return;
    if (hyperShieldCooldown <= 0 || hyperShieldTimer == 0) {
        hyperShieldTimer = 500; // 8 seconds
        hyperShieldCooldown = 750; // 12.5 seconds CD
        PlaySnd(2);
        AddExplosion(p.x + 10.0f, p.y + 10.0f, 25, RGB(255, 234, 0));
    }
}

// Loop 10: Weapon Overcharge Activation
void UseOvercharge() {
    if (gameState != STATE_PLAYING) return;
    if (overchargeTimer > 0) return;
    if (overchargeEnergy >= 100) {
        overchargeTimer = 360; // 6 seconds hyper mode
        overchargeEnergy = 0;
        PlaySnd(7);
        AddShockwave(p.x + 10.0f, p.y + 10.0f, 60.0f, RGB(255, 234, 0));
        AddShockwave(p.x + 10.0f, p.y + 10.0f, 40.0f, RGB(0, 229, 255));
        AddExplosion(p.x + 10.0f, p.y + 10.0f, 30, RGB(255, 234, 0));
    }
}

// Loop 11: Hyper-Jump Warp Activation
void UseHyperJump() {
    if (gameState != STATE_PLAYING) return;
    if (hyperJumpEnergy >= 100 && hyperJumpTimer <= 0) {
        hyperJumpEnergy = 0;
        hyperJumpTimer = 40;
        invincibleTimer = 90; // 1.5s invulnerability
        PlaySnd(9);
        AddShockwave(p.x + 10.0f, p.y + 10.0f, 95.0f, RGB(0, 229, 255));
        AddShockwave(p.x + 10.0f, p.y + 10.0f, 60.0f, RGB(255, 234, 0));
        AddExplosion(p.x + 10.0f, p.y + 10.0f, 40, RGB(0, 229, 255));

        // Clear all enemy bullets on screen
        for (int i = 0; i < MAX_EBULLETS; i++) eb[i].active = 0;

        // Deal 60 warp damage to all active enemies
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (e[i].active) {
                e[i].hp -= 60;
                AddExplosion(e[i].x + 10.0f, e[i].y + 10.0f, 15, RGB(0, 229, 255));
                if (e[i].hp <= 0) {
                    e[i].active = 0.0f;
                    comboTimer = 180;
                    if (comboMultiplier < 10) comboMultiplier++;
                    score += 50 * comboMultiplier;
                    enemiesKilled++;
                    totalKills++;
                }
            }
        }

        // Deal damage to Boss / Dreadnought
        if (bossActive) {
            if (bossIsDreadnought) {
                if (dreadGenL > 0) { dreadGenL -= 40; if (dreadGenL < 0) dreadGenL = 0; }
                if (dreadGenR > 0) { dreadGenR -= 40; if (dreadGenR < 0) dreadGenR = 0; }
                bossHp -= 70;
            } else if (bossIsMothership) {
                for (int tIdx = 0; tIdx < 4; tIdx++) {
                    if (turretActive[tIdx]) {
                        turretHp[tIdx] -= 40;
                        if (turretHp[tIdx] <= 0) turretActive[tIdx] = 0;
                    }
                }
                if (!IsMothershipShieldActive()) bossHp -= 70;
            } else {
                bossHp -= 80;
            }
            AddExplosion(bossX + 45.0f, bossY + 30.0f, 35, RGB(0, 229, 255));
            if (bossHp <= 0) {
                if (bossPhase == 1) {
                    bossPhase = 2; bossMaxHp = (int)(bossMaxHp * 1.5f); bossHp = bossMaxHp;
                    bossDx = (bossDx > 0 ? bossDx + 0.5f : bossDx - 0.5f);
                    AddExplosion(bossX + 45.0f, bossY + 30.0f, 40, RGB(255, 234, 0));
                } else {
                    DestroyBoss();
                }
            }
        }
    }
}

// Loop 11: Deploy or Upgrade Drone Companion Wings
void DeployDroneWing() {
    if (gameState != STATE_PLAYING) return;
    if (droneCount < 2) {
        droneCount++;
        int dIdx = droneCount - 1;
        drones[dIdx].x = p.x + (dIdx == 0 ? -22.0f : 22.0f);
        drones[dIdx].y = p.y + 6.0f;
        drones[dIdx].shootTimer = 0;
        PlaySnd(2);
        AddShockwave(drones[dIdx].x + 6.0f, drones[dIdx].y + 6.0f, 30.0f, RGB(0, 229, 255));
        AddExplosion(drones[dIdx].x + 6.0f, drones[dIdx].y + 6.0f, 15, RGB(0, 229, 255));
    }
}

// Loop 10: Planetary Bombardment Mission Trigger
void TriggerBombardment() {
    if (bombardmentActive || bossActive) return;
    bombardmentActive = 1;
    bombardmentTimer = 480; // 8 seconds
    bombardmentBanner = 90;
    for (int i = 0; i < MAX_STRIKES; i++) strikes[i].active = 0;
    PlaySnd(8);
}

// Loop 10: Elite Enemy Squads Spawner
void SpawnEliteSquad(int squadType) {
    if (bossActive) return;
    eliteSquadActive = 1;
    eliteSquadTimer = 90;
    eliteSquadType = squadType;
    PlaySnd(8);

    if (squadType == 0) {
        lstrcpyA(eliteSquadName, "CRIMSON VALKYRIES");
        for (int k = 0; k < 3; k++) {
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (!e[i].active) {
                    e[i].active = 1.0f;
                    e[i].x = W / 2.0f - 60.0f + k * 50.0f;
                    e[i].y = -30.0f - (k == 1 ? 0 : 25.0f);
                    e[i].type = 10.0f; // Elite Crimson Valkyrie
                    e[i].hp = 32; e[i].maxHp = 32;
                    e[i].dx = (k == 0 ? -1.5f : (k == 2 ? 1.5f : 0.0f));
                    e[i].dy = 1.6f;
                    e[i].timer = 0; e[i].cloaked = 0; e[i].isElite = 1; e[i].squadId = 1;
                    e[i].shield = 10;
                    break;
                }
            }
        }
    } else if (squadType == 1) {
        lstrcpyA(eliteSquadName, "VOID PHANTOMS");
        for (int k = 0; k < 2; k++) {
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (!e[i].active) {
                    e[i].active = 1.0f;
                    e[i].x = 60.0f + k * 160.0f;
                    e[i].y = -35.0f;
                    e[i].type = 11.0f; // Elite Void Phantom
                    e[i].hp = 48; e[i].maxHp = 48;
                    e[i].dx = (k == 0 ? 1.2f : -1.2f);
                    e[i].dy = 1.2f;
                    e[i].timer = 0; e[i].cloaked = 0; e[i].isElite = 1; e[i].squadId = 2;
                    e[i].shield = 15;
                    break;
                }
            }
        }
    } else {
        lstrcpyA(eliteSquadName, "DREAD COMMAND");
        for (int k = 0; k < 3; k++) {
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (!e[i].active) {
                    e[i].active = 1.0f;
                    e[i].x = W / 2.0f - 40.0f + k * 40.0f;
                    e[i].y = -35.0f - (k == 1 ? 20.0f : 0.0f);
                    e[i].type = (k == 1) ? 12.0f : 10.0f; // 12 = Command, 10 = Escort
                    e[i].hp = (k == 1) ? 75 : 25; e[i].maxHp = e[i].hp;
                    e[i].dx = (k == 1 ? 0.8f : (k == 0 ? -1.0f : 1.0f));
                    e[i].dy = 1.0f;
                    e[i].timer = 0; e[i].cloaked = 0; e[i].isElite = 1; e[i].squadId = 3;
                    e[i].shield = 20;
                    break;
                }
            }
        }
    }
}

void Shoot() {
    shotsFired++;
    if (overchargeTimer > 0) {
        PlaySnd(5); // Laser pulse sound
        // OVERCHARGE SUPER-SALVO: 4 hyper bolts + 2 plasma orbs + 2 homing micro-missiles!
        float offsets[4] = {-4.0f, 4.0f, 16.0f, 24.0f};
        for (int k = 0; k < 4; k++) {
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!b[i].active) {
                    b[i].active = 1.0f;
                    b[i].x = p.x + offsets[k];
                    b[i].y = p.y - 4.0f;
                    b[i].dx = (k < 2 ? -0.6f : 0.6f);
                    b[i].dy = -10.0f;
                    b[i].type = 0.0f;
                    break;
                }
            }
        }
        for (int k = 0; k < 2; k++) {
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!b[i].active) {
                    b[i].active = 1.0f;
                    b[i].x = p.x + 8.0f;
                    b[i].y = p.y;
                    b[i].dx = (k == 0 ? -3.0f : 3.0f);
                    b[i].dy = -7.5f;
                    b[i].type = 1.0f; // Plasma
                    break;
                }
            }
        }
        for (int k = 0; k < 2; k++) {
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!b[i].active) {
                    b[i].active = 1.0f;
                    b[i].x = p.x + (k == 0 ? 0.0f : 20.0f);
                    b[i].y = p.y + 4.0f;
                    b[i].dx = (k == 0 ? -2.0f : 2.0f);
                    b[i].dy = -4.0f;
                    b[i].type = 2.0f; // Homing Micro-Missile
                    break;
                }
            }
        }
        AddMuzzleFlash(p.x + 10.0f, p.y - 4.0f, RGB(255, 234, 0), 12.0f);
        return;
    }

    PlaySnd(laserTimer > 0 ? 5 : 0);
    if (laserTimer > 0) return;

    if (spreadTimer > 0 || weaponType == 1) {
        int maxB = 3 + weaponLevel * 2;
        float dxs[7] = {0, -2.5f, 2.5f, -1.25f, 1.25f, -3.75f, 3.75f};
        float dys[7] = {-9.0f, -8.0f, -8.0f, -8.5f, -8.5f, -7.0f, -7.0f};
        int spawned = 0;
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!b[i].active) {
                b[i].active = 1.0f;
                b[i].x = p.x + 8.0f;
                b[i].y = p.y;
                b[i].dx = dxs[spawned];
                b[i].dy = dys[spawned];
                b[i].type = 0;
                spawned++;
                if (spawned >= maxB) break;
            }
        }
        AddMuzzleFlash(p.x + 10.0f, p.y - 2.0f, RGB(0, 229, 255), 8.0f);
    } else if (weaponType == 3) { // Plasma Orb
        int maxB = 1 + weaponLevel;
        float dxs[3] = {0.0f, -2.0f, 2.0f};
        int spawned = 0;
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!b[i].active) {
                b[i].active = 1.0f;
                b[i].x = p.x + 6.0f;
                b[i].y = p.y;
                b[i].dx = dxs[spawned];
                b[i].dy = -5.0f;
                b[i].type = 1;
                spawned++;
                if (spawned >= maxB) break;
            }
        }
        AddMuzzleFlash(p.x + 10.0f, p.y - 2.0f, RGB(213, 0, 249), 10.0f);
    } else { // Standard Twin Blaster
        int maxB = 2 + weaponLevel;
        float offsets[4] = {3.0f, 13.0f, 8.0f, 8.0f};
        float dys[4] = {-8.0f, -8.0f, -9.0f, -7.0f};
        float dxs[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        if (weaponLevel == 2) { dxs[2] = -1.5f; dxs[3] = 1.5f; } // at lvl 2, middle ones angle
        int spawned = 0;
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!b[i].active) {
                b[i].active = 1.0f;
                b[i].x = p.x + offsets[spawned];
                b[i].y = p.y;
                b[i].dx = dxs[spawned];
                b[i].dy = dys[spawned];
                b[i].type = 0;
                spawned++;
                if (spawned >= maxB) break;
            }
        }
        AddMuzzleFlash(p.x + 4.0f, p.y, RGB(0, 229, 255), 5.0f);
        AddMuzzleFlash(p.x + 14.0f, p.y, RGB(0, 229, 255), 5.0f);
    }
}

void StartNewGame(int modeIdx) {
    modeIndex = modeIdx;
    p.x = W / 2.0f - 10.0f;
    p.y = H - 60.0f;
    p.hp = 3;
    p.maxHp = 3;
    shieldActive = 0;
    hyperShieldTimer = 0;
    hyperShieldCooldown = 0;
    bombCount = 1;
    weaponType = 0;
    weaponLevel = 0;
    score = 0; shotsFired = 0; shotsHit = 0; timeSurvivedFrames = 0;
    comboMultiplier = 1;
    comboTimer = 0;
    wave = 1;
    enemiesKilled = 0;
    spreadTimer = 0;
    laserTimer = 0;
    rapidTimer = 0;
    timeStopTimer = 0;
    timeStopCooldown = 0;
    dashCooldown = 0;
    invincibleTimer = 0;
    overchargeEnergy = 0;
    overchargeTimer = 0;
    eliteSquadActive = 0;
    eliteSquadTimer = 0;
    bombardmentActive = 0;
    bombardmentTimer = 0;
    bombardmentBanner = 0;
    bossActive = 0;
    bossIsDreadnought = 0;
    droneCount = 0;
    hyperJumpEnergy = 0;
    hyperJumpTimer = 0;

    for (int i = 0; i < MAX_STRIKES; i++) strikes[i].active = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) e[i].active = 0;
    for (int i = 0; i < MAX_BULLETS; i++) b[i].active = 0;
    for (int i = 0; i < MAX_EBULLETS; i++) eb[i].active = 0;
    for (int i = 0; i < MAX_POWERUPS; i++) pu[i].active = 0;
    for (int i = 0; i < MAX_PARTICLES; i++) particles[i].life = 0;
    for (int i = 0; i < MAX_SHOCKWAVES; i++) shockwaves[i].life = 0;
    for (int i = 0; i < MAX_DEBRIS; i++) debris[i].life = 0.0f;
    for (int i = 0; i < MAX_RIPPLES; i++) ripples[i].life = 0;
    for (int i = 0; i < MAX_FLASHES; i++) flashes[i].life = 0;
    
    escort.active = 0;
    pathGatesActive = 0;

    gameState = STATE_PLAYING;
    PlaySnd(3);

    if (modeIndex == MODE_BOSS_RUSH) SpawnBoss(1);
}

void PlayerHit() {
    if (hyperShieldTimer > 0 || invincibleTimer > 0) return;

    if (shieldActive) {
        shieldActive = 0;
        AddShieldRipple(p.x + 10.0f, p.y + 10.0f, RGB(0, 229, 255));
        AddExplosion(p.x + 10.0f, p.y + 10.0f, 16, RGB(0, 229, 255));
        PlaySnd(1);
    } else {
        p.hp--;
        weaponLevel = 0;
        AddExplosion(p.x + 10.0f, p.y + 10.0f, 30, RGB(255, 23, 68));
        PlaySnd(1);
        if (p.hp <= 0) {
            gameState = STATE_GAMEOVER;
            ClearSavedGame();
            AddScoreToLeaderboard(score, wave, modeIndex);
        }
    }
}

void ApplyPowerup(int type) {
    if (type == 0) { spreadTimer = 350; weaponType = 1; }
    else if (type == 1) { laserTimer = 300; weaponType = 2; }
    else if (type == 2) { shieldActive = 1; AddShieldRipple(p.x + 10.0f, p.y + 10.0f, RGB(0, 229, 255)); }
    else if (type == 3) { if (bombCount < maxBombs) bombCount++; }
    else if (type == 4) { rapidTimer = 350; }
    else if (type == 5) { UseTimeStop(); }
    else if (type == 6) { UseHyperShield(); AddShieldRipple(p.x + 10.0f, p.y + 10.0f, RGB(255, 234, 0)); }
    else if (type == 7) { dashCooldown = 0; UseTacticalDash(); }
    else if (type == 8) { if (weaponLevel < 2) weaponLevel++; }
    else if (type == 9) {
        overchargeEnergy += 50;
        if (overchargeEnergy > 100) overchargeEnergy = 100;
        AddShockwave(p.x + 10.0f, p.y + 10.0f, 30.0f, RGB(255, 234, 0));
        PlaySnd(2);
    }
    else if (type == 10) { // Loop 11: Drone Wing Pod
        DeployDroneWing();
        hyperJumpEnergy += 35;
        if (hyperJumpEnergy > 100) hyperJumpEnergy = 100;
    }
}

void Update() {
    if (bombFlash > 0) bombFlash--;
    if (bossDeathFlash > 0) bossDeathFlash -= 5;
    if (screenShake > 0) screenShake = (screenShake * 88) / 100;
    if (hyperJumpTimer > 0) hyperJumpTimer--;
    if (gameState != STATE_PLAYING) return;
    timeSurvivedFrames++;
    if (comboTimer > 0) {
        comboTimer--;
        if (comboTimer == 0) comboMultiplier = 1;
    }

    if (overchargeTimer > 0) {
        overchargeTimer--;
        // Overcharge kinetic deflection aura: destroy incoming enemy bullets within 24px of player
        for (int i = 0; i < MAX_EBULLETS; i++) {
            if (eb[i].active) {
                float dx = eb[i].x - (p.x + 10.0f);
                float dy = eb[i].y - (p.y + 10.0f);
                if (dx * dx + dy * dy < 576.0f) {
                    eb[i].active = 0.0f;
                    AddExplosion(eb[i].x, eb[i].y, 3, RGB(0, 229, 255));
                }
            }
        }
    }

    if (bombardmentBanner > 0) bombardmentBanner--;
    if (eliteSquadTimer > 0) eliteSquadTimer--;

    // Loop 11: Drone Companion Wings Update & Support Systems
    for (int d = 0; d < droneCount; d++) {
        float targetX = p.x + (d == 0 ? -22.0f : 22.0f);
        int phase = (frameCount + (d == 0 ? 0 : 10)) % 20;
        float floatOffset = (phase < 10 ? (float)(phase - 5) : (float)(15 - phase)) * 0.6f;
        float targetY = p.y + 6.0f + floatOffset;
        drones[d].x += (targetX - drones[d].x) * 0.25f;
        drones[d].y += (targetY - drones[d].y) * 0.25f;

        // Drones Auto-Firing support vulcan bolts
        drones[d].shootTimer++;
        if (drones[d].shootTimer % 14 == 0) {
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!b[i].active) {
                    b[i].active = 1.0f;
                    b[i].x = drones[d].x + 4.0f;
                    b[i].y = drones[d].y - 2.0f;
                    b[i].dx = (d == 0 ? -0.4f : 0.4f);
                    b[i].dy = -8.5f;
                    b[i].type = 0.0f;
                    break;
                }
            }
            PlaySnd(10);
            AddMuzzleFlash(drones[d].x + 4.0f, drones[d].y - 2.0f, RGB(0, 229, 255), 4.0f);
        }

        // Drones Point-Defense: Destroy enemy bullets near the drone
        for (int i = 0; i < MAX_EBULLETS; i++) {
            if (eb[i].active) {
                float dx = eb[i].x - (drones[d].x + 5.0f);
                float dy = eb[i].y - (drones[d].y + 5.0f);
                if (dx * dx + dy * dy < 256.0f) {
                    eb[i].active = 0.0f;
                    AddExplosion(eb[i].x, eb[i].y, 3, RGB(0, 229, 255));
                    PlaySnd(10);
                }
            }
        }
    }

    // Loop 10: Planetary Bombardment Mission Simulation
    if (bombardmentActive) {
        bombardmentTimer--;
        if (frameCount % 45 == 0) {
            for (int k = 0; k < MAX_STRIKES; k++) {
                if (!strikes[k].active) {
                    strikes[k].active = 1;
                    strikes[k].x = 25.0f + (float)(rnd() % (W - 50));
                    strikes[k].timer = 55;
                    strikes[k].width = 24;
                    break;
                }
            }
        }
        for (int k = 0; k < MAX_STRIKES; k++) {
            if (strikes[k].active) {
                strikes[k].timer--;
                if (strikes[k].timer == 15) { // Impact moment!
                    PlaySnd(4);
                    AddShockwave(strikes[k].x, H - 30.0f, 35.0f, RGB(255, 60, 0));
                    AddExplosion(strikes[k].x, H - 30.0f, 15, RGB(255, 23, 68));
                    if (p.x + 20.0f > strikes[k].x - 12.0f && p.x < strikes[k].x + 12.0f) {
                        PlayerHit();
                    }
                    for (int i = 0; i < MAX_ENEMIES; i++) {
                        if (e[i].active && e[i].x + 20.0f > strikes[k].x - 14.0f && e[i].x < strikes[k].x + 14.0f) {
                            e[i].hp -= 25;
                            AddExplosion(e[i].x + 10.0f, e[i].y + 10.0f, 10, RGB(255, 60, 0));
                        }
                    }
                }
                if (strikes[k].timer <= 0) strikes[k].active = 0;
            }
        }
        if (frameCount % 80 == 0) {
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (!e[i].active) {
                    e[i].active = 1.0f;
                    e[i].x = (float)(rnd() % (W - 35));
                    e[i].y = -30.0f;
                    e[i].type = 13.0f; // Siege Drop Pod
                    e[i].hp = 24; e[i].maxHp = 24;
                    e[i].dx = 0.0f; e[i].dy = 3.0f;
                    e[i].timer = 0; e[i].cloaked = 0; e[i].isElite = 0; e[i].squadId = 0;
                    e[i].shield = 0;
                    break;
                }
            }
        }
        if (bombardmentTimer <= 0) {
            bombardmentActive = 0;
            score += 3000 * comboMultiplier;
            overchargeEnergy += 25;
            if (overchargeEnergy > 100) overchargeEnergy = 100;
            hyperJumpEnergy += 25;
            if (hyperJumpEnergy > 100) hyperJumpEnergy = 100;
            PlaySnd(6);
        }
    }

    // Loop 10: Elite Squad Tracking
    if (eliteSquadActive) {
        int aliveElites = 0;
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (e[i].active && e[i].isElite) aliveElites++;
        }
        if (aliveElites == 0) {
            eliteSquadActive = 0;
            score += 2500 * comboMultiplier;
            overchargeEnergy += 35;
            if (overchargeEnergy > 100) overchargeEnergy = 100;
            hyperJumpEnergy += 35;
            if (hyperJumpEnergy > 100) hyperJumpEnergy = 100;
            PlaySnd(6);
            for (int k = 0; k < MAX_POWERUPS; k++) {
                if (!pu[k].active) {
                    pu[k].active = 1.0f; pu[k].x = W / 2.0f - 15.0f; pu[k].y = 50.0f; pu[k].dy = 1.5f;
                    pu[k].type = (k == 0 ? 10.0f : 9.0f); // 10 = Drone Pod, 9 = Overcharge Core
                    break;
                }
            }
        }
    }

    // Mode Progression (20 Waves)
    if (modeIndex == MODE_CLASSIC) {
        int targetWave = 1 + (score / 650);
        if (targetWave > 20) targetWave = 20;
        if (targetWave > wave) {
            wave = targetWave;
            PlaySnd(3);
            if (wave == 3 && !eliteSquadActive && !bossActive) SpawnEliteSquad(0);
            else if (wave == 4 && !bombardmentActive && !bossActive) TriggerBombardment();
            else if (wave == 5 && !bossActive) SpawnBoss(1);
            else if (wave == 7 && !eliteSquadActive && !bossActive) SpawnEliteSquad(1);
            else if (wave == 8 && !bossActive) SpawnBoss(2); // Loop 11: Capital Ship Dreadnought Siege!
            else if (wave == 10 && !bossActive) SpawnBoss(3);
            else if (wave == 11 && !eliteSquadActive && !bossActive) SpawnEliteSquad(2);
            else if (wave == 12 && !bombardmentActive && !bossActive) TriggerBombardment();
            else if (wave == 15 && !bossActive) SpawnBoss(4);
            else if (wave == 16 && !bossActive) SpawnBoss(5); // Loop 11: Capital Ship Dreadnought Siege Rematch!
            else if (wave == 17 && !eliteSquadActive && !bossActive) SpawnEliteSquad(rnd() % 3);
            else if (wave == 20 && !bossActive) SpawnBoss(6); // Stage 20 Alien Mothership Boss
            else if (wave % 4 == 0) SpawnFormation(rnd() % 3 == 0 ? 9 : (rnd() % 2 == 0 ? 7 : 8));
        }
    } else if (modeIndex == MODE_ENDURANCE) {
        wave = 1 + (score / 600);
        if (wave % 5 == 0 && frameCount % 300 == 0 && !eliteSquadActive && !bossActive) SpawnEliteSquad(rnd() % 3);
        if (wave % 7 == 0 && frameCount % 400 == 0 && !bombardmentActive && !bossActive) TriggerBombardment();
    }

    // Scroll Starfield
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].y += stars[i].speed * (modeIndex == MODE_ENDURANCE ? 1.5f : 1.0f);
        if (stars[i].y > H) {
            stars[i].y = 0;
            stars[i].x = (float)(rnd() % W);
        }
    }

    UpdateParticles();

    if (pathGatesActive) {
        pathGatesY += 1.0f;
        if (pathGatesY > H + 50.0f) pathGatesActive = 0;
        
        if (p.x < W/4.0f + 20 && p.x + 20 > W/4.0f - 20 && p.y < pathGatesY + 20 && p.y + 20 > pathGatesY - 20) {
            pathGatesActive = 0; wave += 1; score += 2000; PlaySnd(2);
        } else if (p.x < 3*W/4.0f + 20 && p.x + 20 > 3*W/4.0f - 20 && p.y < pathGatesY + 20 && p.y + 20 > pathGatesY - 20) {
            pathGatesActive = 0; p.hp = p.maxHp; PlaySnd(2);
        }
    }

    if (frameCount % 600 == 0 && !escort.active && !bossActive && wave % 2 == 0) {
        escort.active = 1; escort.x = W / 2.0f - 15.0f; escort.y = -30.0f; escort.dy = 0.5f;
        escort.maxHp = 20; escort.hp = 20;
    }
    if (escort.active) {
        escort.y += escort.dy;
        for (int i = 0; i < MAX_EBULLETS; i++) {
            if (eb[i].active && eb[i].x >= escort.x && eb[i].x <= escort.x+30 && eb[i].y >= escort.y && eb[i].y <= escort.y+30) {
                eb[i].active = 0; escort.hp--; AddExplosion(eb[i].x, eb[i].y, 3, RGB(255,234,0));
            }
        }
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (e[i].active && e[i].x+20 >= escort.x && e[i].x <= escort.x+30 && e[i].y+20 >= escort.y && e[i].y <= escort.y+30) {
                e[i].active = 0; escort.hp -= 5; AddExplosion(e[i].x+10, e[i].y+10, 10, RGB(255,152,0));
            }
        }
        if (escort.hp <= 0) {
            escort.active = 0; AddExplosion(escort.x+15, escort.y+15, 30, RGB(255,23,68));
        } else if (escort.y > H) {
            escort.active = 0; score += 5000 * comboMultiplier; PlaySnd(6);
        }
    }

    // Controls
    float speed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 7.0f : 4.5f;
    if ((GetAsyncKeyState(kbLeft) & 0x8000)) p.x -= speed;
    if ((GetAsyncKeyState(kbRight) & 0x8000)) p.x += speed;
    if ((GetAsyncKeyState(kbUp) & 0x8000)) p.y -= speed;
    if ((GetAsyncKeyState(kbDown) & 0x8000)) p.y += speed;

    if (p.x < 0) p.x = 0;
    if (p.x > W - 20) p.x = W - 20;
    if (p.y < 0) p.y = 0;
    if (p.y > H - 20) p.y = H - 20;

    // Firing
    if ((GetAsyncKeyState(kbFire) & 0x8001) || (GetAsyncKeyState(VK_RETURN) & 0x8001)) {
        int fireRate = (overchargeTimer > 0) ? 3 : ((rapidTimer > 0) ? 3 : 7);
        if (frameCount % fireRate == 0) Shoot();
    }

    // Skill Timers & Cooldowns
    if (spreadTimer > 0) spreadTimer--;
    if (laserTimer > 0) laserTimer--;
    if (rapidTimer > 0) rapidTimer--;
    if (timeStopTimer > 0) timeStopTimer--;
    if (timeStopCooldown > 0) timeStopCooldown--;
    if (hyperShieldTimer > 0) hyperShieldTimer--;
    if (hyperShieldCooldown > 0) hyperShieldCooldown--;
    if (dashCooldown > 0) dashCooldown--;
    if (invincibleTimer > 0) invincibleTimer--;

    // Bullets Movement & Homing Micro-Missiles
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (b[i].active) {
            if (b[i].type == 2.0f) { // Homing Micro-Missile
                float targetX = W / 2.0f;
                float closestDist = 999999.0f;
                int hasTarget = 0;
                if (bossActive) {
                    targetX = bossX + (bossIsDreadnought ? 60.0f : 45.0f);
                    hasTarget = 1;
                } else {
                    for (int k = 0; k < MAX_ENEMIES; k++) {
                        if (e[k].active) {
                            float d = (e[k].x - b[i].x)*(e[k].x - b[i].x) + (e[k].y - b[i].y)*(e[k].y - b[i].y);
                            if (d < closestDist) { closestDist = d; targetX = e[k].x + 10.0f; hasTarget = 1; }
                        }
                    }
                }
                if (hasTarget) {
                    if (targetX > b[i].x + 2.0f) b[i].dx += 0.4f;
                    else if (targetX < b[i].x - 2.0f) b[i].dx -= 0.4f;
                    if (b[i].dx > 4.5f) b[i].dx = 4.5f;
                    if (b[i].dx < -4.5f) b[i].dx = -4.5f;
                }
                b[i].dy = -8.0f;
            }
            b[i].y += b[i].dy;
            b[i].x += b[i].dx;
            if (b[i].y < -10 || b[i].x < -10 || b[i].x > W + 10) b[i].active = 0.0f;
        }
    }

    // Loop 11: Dreadnought Boss & Mothership Logic
    if (bossActive) {
        if (bossIsDreadnought) {
            if (bossY < 35.0f) bossY += 0.8f;
            else {
                bossX += bossDx;
                if (bossX < 5.0f || bossX > W - 125.0f) bossDx = -bossDx;
            }

            bossAttackTimer++;
            dreadFlakTimer++;

            // Mega-Ion Cannon Charging & Firing
            if (timeStopTimer == 0) {
                dreadIonCharge += 1;
                if (dreadIonCharge >= 100) {
                    dreadIonCharge = 0;
                    dreadIonBeamTimer = 45; // 0.75 seconds of mega-beam
                    PlaySnd(11);
                    screenShake = 15;
                }
            }

            if (dreadIonBeamTimer > 0) {
                dreadIonBeamTimer--;
                float beamX = bossX + 60.0f;
                if (p.x + 18.0f > beamX - 14.0f && p.x + 2.0f < beamX + 14.0f) {
                    if (frameCount % 6 == 0) PlayerHit();
                }
            }

            // Dreadnought Flak & Homing Salvos
            if (timeStopTimer == 0 && dreadFlakTimer % 38 == 0) {
                for (int k = 0; k < MAX_EBULLETS; k++) {
                    if (!eb[k].active) {
                        eb[k].active = 1.0f;
                        eb[k].x = bossX + 20.0f; eb[k].y = bossY + 40.0f;
                        eb[k].dx = -1.8f; eb[k].dy = 3.2f;
                        break;
                    }
                }
                for (int k = 0; k < MAX_EBULLETS; k++) {
                    if (!eb[k].active) {
                        eb[k].active = 1.0f;
                        eb[k].x = bossX + 100.0f; eb[k].y = bossY + 40.0f;
                        eb[k].dx = 1.8f; eb[k].dy = 3.2f;
                        break;
                    }
                }
                for (int k = 0; k < MAX_EBULLETS; k++) {
                    if (!eb[k].active) {
                        eb[k].active = 1.0f;
                        eb[k].x = bossX + 60.0f; eb[k].y = bossY + 50.0f;
                        eb[k].dx = (frameCount % 2 == 0 ? -0.8f : 0.8f); eb[k].dy = 3.8f;
                        break;
                    }
                }
            }

            // Bullets vs Dreadnought Subsystems & Hull
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (b[i].active) {
                    // Check Left Shield Generator
                    if (dreadGenL > 0 && b[i].x >= bossX + 8.0f && b[i].x <= bossX + 42.0f && b[i].y >= bossY + 12.0f && b[i].y <= bossY + 46.0f) {
                        b[i].active = 0.0f;
                        int dmg = (b[i].type == 1.0f) ? 10 : (b[i].type == 2.0f ? 14 : 3);
                        if (overchargeTimer > 0) dmg *= 3;
                        dreadGenL -= dmg;
                        shotsHit++;
                        AddExplosion(b[i].x, b[i].y, 4, RGB(0, 229, 255));
                        if (dreadGenL <= 0) {
                            dreadGenL = 0;
                            AddExplosion(bossX + 25.0f, bossY + 30.0f, 25, RGB(255, 23, 68));
                            score += 1500 * comboMultiplier;
                            hyperJumpEnergy += 25; if (hyperJumpEnergy > 100) hyperJumpEnergy = 100;
                        }
                        continue;
                    }
                    // Check Right Shield Generator
                    if (dreadGenR > 0 && b[i].x >= bossX + 78.0f && b[i].x <= bossX + 112.0f && b[i].y >= bossY + 12.0f && b[i].y <= bossY + 46.0f) {
                        b[i].active = 0.0f;
                        int dmg = (b[i].type == 1.0f) ? 10 : (b[i].type == 2.0f ? 14 : 3);
                        if (overchargeTimer > 0) dmg *= 3;
                        dreadGenR -= dmg;
                        shotsHit++;
                        AddExplosion(b[i].x, b[i].y, 4, RGB(0, 229, 255));
                        if (dreadGenR <= 0) {
                            dreadGenR = 0;
                            AddExplosion(bossX + 95.0f, bossY + 30.0f, 25, RGB(255, 23, 68));
                            score += 1500 * comboMultiplier;
                            hyperJumpEnergy += 25; if (hyperJumpEnergy > 100) hyperJumpEnergy = 100;
                        }
                        continue;
                    }

                    // Check Central Hull
                    if (b[i].x >= bossX && b[i].x <= bossX + 120.0f && b[i].y >= bossY && b[i].y <= bossY + 60.0f) {
                        b[i].active = 0.0f;
                        int isShielded = (dreadGenL > 0 || dreadGenR > 0);
                        if (isShielded) {
                            AddWeaponHitParticles(b[i].x, b[i].y, weaponType, RGB(0, 229, 255));
                        } else {
                            int dmg = (b[i].type == 1.0f) ? 8 : (b[i].type == 2.0f ? 12 : 2);
                            if (overchargeTimer > 0) dmg *= 3;
                            bossHp -= dmg;
                            shotsHit++;
                            overchargeEnergy++; if (overchargeEnergy > 100) overchargeEnergy = 100;
                            hyperJumpEnergy++; if (hyperJumpEnergy > 100) hyperJumpEnergy = 100;
                            AddWeaponHitParticles(b[i].x, b[i].y, weaponType, RGB(255, 23, 68));
                            if (bossHp <= 0) {
                                DestroyBoss();
                            }
                        }
                    }
                }
            }
        } else {
            // Mothership / Regular Boss
            if (bossY < 45.0f) bossY += 1.0f;
            else {
                bossX += bossDx;
                if (bossX < 10.0f || bossX > W - (bossIsMothership ? 95.0f : 70.0f)) bossDx = -bossDx;
            }

            bossAttackTimer++;
            if (timeStopTimer == 0 && bossAttackTimer % (bossIsMothership ? 35 : 45) == 0) {
                if (bossIsMothership) {
                    for (int tIdx = 0; tIdx < 4; tIdx++) {
                        if (turretActive[tIdx]) {
                            float tx = bossX + (tIdx == 0 ? 5 : (tIdx == 1 ? 25 : (tIdx == 2 ? 65 : 85)));
                            float ty = bossY + 35.0f;
                            for (int k = 0; k < MAX_EBULLETS; k++) {
                                if (!eb[k].active) {
                                    eb[k].active = 1.0f;
                                    eb[k].x = tx; eb[k].y = ty;
                                    eb[k].dy = 3.5f;
                                    eb[k].dx = (tIdx == 0 ? -1.5f : (tIdx == 3 ? 1.5f : 0.0f));
                                    break;
                                }
                            }
                        }
                    }
                    if (!IsMothershipShieldActive() && (bossAttackTimer % 50 == 0)) {
                        for (int a = -2; a <= 2; a++) {
                            for (int k = 0; k < MAX_EBULLETS; k++) {
                                if (!eb[k].active) {
                                    eb[k].active = 1.0f;
                                    eb[k].x = bossX + 45.0f; eb[k].y = bossY + 45.0f;
                                    eb[k].dy = 3.8f; eb[k].dx = (float)a * 1.5f;
                                    break;
                                }
                            }
                        }
                    }
                } else {
                    int bspawned = 0;
                    float bdx[] = {-1.5f, 0.0f, 1.5f};
                    for (int k = 0; k < MAX_EBULLETS; k++) {
                        if (!eb[k].active) {
                            eb[k].active = 1.0f;
                            eb[k].x = bossX + 30.0f; eb[k].y = bossY + 45.0f;
                            eb[k].dy = 3.5f; eb[k].dx = bdx[bspawned];
                            bspawned++;
                            if (bspawned >= 3) break;
                        }
                    }
                }
            }

            // Bullets & Laser vs Mothership/Boss
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (b[i].active) {
                    if (bossIsMothership) {
                        int hitTurret = 0;
                        float tOffsetsX[] = {5, 25, 65, 85};
                        for (int tIdx = 0; tIdx < 4; tIdx++) {
                            if (turretActive[tIdx]) {
                                float tx = bossX + tOffsetsX[tIdx];
                                float ty = bossY + 25.0f;
                                if (b[i].x >= tx - 6 && b[i].x <= tx + 18 && b[i].y >= ty - 6 && b[i].y <= ty + 18) {
                                    b[i].active = 0.0f;
                                    int dmg = (b[i].type == 1.0f) ? 12 : ((b[i].type == 2.0f) ? 15 : 3);
                                    if (overchargeTimer > 0) dmg *= 3;
                                    turretHp[tIdx] -= dmg;
                                    overchargeEnergy++; if (overchargeEnergy > 100) overchargeEnergy = 100;
                                    hyperJumpEnergy++; if (hyperJumpEnergy > 100) hyperJumpEnergy = 100;
                                    AddExplosion(b[i].x, b[i].y, 4, RGB(255, 145, 0));
                                    if (turretHp[tIdx] <= 0) {
                                        turretActive[tIdx] = 0;
                                        overchargeEnergy += 15; if (overchargeEnergy > 100) overchargeEnergy = 100;
                                        hyperJumpEnergy += 15; if (hyperJumpEnergy > 100) hyperJumpEnergy = 100;
                                        AddExplosion(tx, ty, 20, RGB(255, 23, 68));
                                    }
                                    hitTurret = 1;
                                    break;
                                }
                            }
                        }
                        if (!hitTurret && b[i].x >= bossX && b[i].x <= bossX + 90.0f && b[i].y >= bossY && b[i].y <= bossY + 60.0f) {
                            b[i].active = 0.0f;
                            if (IsMothershipShieldActive()) {
                                AddWeaponHitParticles(b[i].x, b[i].y, weaponType, RGB(0, 229, 255));
                            } else {
                                shotsHit++;
                                int dmg = (b[i].type == 1.0f) ? 8 : ((b[i].type == 2.0f) ? 12 : 2);
                                if (overchargeTimer > 0) dmg *= 3;
                                bossHp -= dmg;
                                overchargeEnergy++; if (overchargeEnergy > 100) overchargeEnergy = 100;
                                hyperJumpEnergy++; if (hyperJumpEnergy > 100) hyperJumpEnergy = 100;
                                AddWeaponHitParticles(b[i].x, b[i].y, weaponType, (overchargeTimer > 0 ? RGB(255, 234, 0) : RGB(255, 23, 68)));
                                if (bossHp <= 0) {
                                    if (bossPhase == 1) {
                                        bossPhase = 2; bossMaxHp = (int)(bossMaxHp * 1.5f); bossHp = bossMaxHp;
                                        bossDx = (bossDx > 0 ? bossDx + 0.5f : bossDx - 0.5f);
                                        AddExplosion(bossX + 45.0f, bossY + 30.0f, 40, RGB(255, 234, 0));
                                    } else {
                                        DestroyBoss();
                                    }
                                }
                            }
                        }
                    } else {
                        if (b[i].x >= bossX && b[i].x <= bossX + 60.0f && b[i].y >= bossY && b[i].y <= bossY + 50.0f) {
                            b[i].active = 0.0f;
                            shotsHit++;
                            int dmg = (b[i].type == 1.0f) ? 8 : ((b[i].type == 2.0f) ? 12 : 2);
                            if (overchargeTimer > 0) dmg *= 3;
                            bossHp -= dmg;
                            overchargeEnergy++; if (overchargeEnergy > 100) overchargeEnergy = 100;
                            hyperJumpEnergy++; if (hyperJumpEnergy > 100) hyperJumpEnergy = 100;
                            AddWeaponHitParticles(b[i].x, b[i].y, weaponType, (overchargeTimer > 0 ? RGB(255, 234, 0) : RGB(0, 229, 255)));
                            if (bossHp <= 0) {
                                if (bossPhase == 1) {
                                    bossPhase = 2; bossMaxHp = (int)(bossMaxHp * 1.5f); bossHp = bossMaxHp;
                                    bossDx = (bossDx > 0 ? bossDx + 0.5f : bossDx - 0.5f);
                                    AddExplosion(bossX + 30.0f, bossY + 25.0f, 40, RGB(255, 234, 0));
                                } else {
                                    DestroyBoss();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Spawning regular enemies & formations
    int spawnRate = 30 - (score / 200) - (modeIndex == MODE_ENDURANCE ? 8 : 0);
    if (spawnRate < 8) spawnRate = 8;
    if (frameCount % spawnRate == 0) SpawnEnemy();

    // Regular & Elite Enemies Update
    float baseEnemySpeed = 1.8f + (score / 300.0f) + (modeIndex == MODE_ENDURANCE ? 1.0f : 0.0f);
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (e[i].active) {
            float ew = (e[i].type == 6.0f || e[i].type == 9.0f || e[i].type == 11.0f || e[i].type == 12.0f) ? 36.0f : 20.0f;
            float eh = (e[i].type == 6.0f || e[i].type == 9.0f || e[i].type == 11.0f || e[i].type == 12.0f) ? 36.0f : 20.0f;

            if (e[i].type == 8.0f || e[i].type == 11.0f) {
                e[i].timer++;
                if (e[i].timer % 40 == 0) e[i].cloaked = !e[i].cloaked;
            }

            if (timeStopTimer == 0) {
                if (e[i].type == 0.0f) e[i].y += baseEnemySpeed * 1.2f;
                else if (e[i].type == 1.0f) {
                    e[i].y += baseEnemySpeed * 0.8f;
                    if (e[i].x < p.x) e[i].x += 0.8f;
                    if (e[i].x > p.x) e[i].x -= 0.8f;
                } else if (e[i].type == 2.0f) e[i].y += baseEnemySpeed * 0.5f;
                else if (e[i].type == 3.0f) e[i].y += baseEnemySpeed * 0.4f;
                else if (e[i].type == 4.0f) {
                    e[i].y += baseEnemySpeed * 0.9f;
                    e[i].x += e[i].dx;
                    if (e[i].x < 0 || e[i].x > W - ew) e[i].dx = -e[i].dx;
                } else if (e[i].type == 5.0f || e[i].type == 9.0f) e[i].y += baseEnemySpeed * 1.3f;
                else if (e[i].type == 6.0f) {
                    if (e[i].y < 60.0f) e[i].y += 0.8f;
                    else {
                        e[i].x += e[i].dx;
                        if (e[i].x < 0 || e[i].x > W - ew) e[i].dx = -e[i].dx;
                    }
                } else if (e[i].type == 7.0f) { // Kamikaze dive
                    e[i].y += baseEnemySpeed * 2.0f;
                    if (e[i].x < p.x) e[i].x += 1.4f;
                    if (e[i].x > p.x) e[i].x -= 1.4f;
                } else if (e[i].type == 8.0f) { // Stealth fighter
                    e[i].y += baseEnemySpeed * 1.0f;
                } else if (e[i].type == 10.0f) { // Elite Crimson Valkyrie
                    e[i].y += baseEnemySpeed * 1.0f;
                    e[i].x += e[i].dx;
                    if (e[i].x < 10.0f || e[i].x > W - ew - 10.0f) e[i].dx = -e[i].dx;
                    if (frameCount % 45 == 0) {
                        for (int j = 0; j < MAX_EBULLETS; j++) {
                            if (!eb[j].active) {
                                eb[j].active = 1.0f; eb[j].x = e[i].x + 10.0f; eb[j].y = e[i].y + 20.0f;
                                eb[j].dy = 4.0f; eb[j].dx = (rnd() % 2 == 0 ? -1.0f : 1.0f);
                                break;
                            }
                        }
                    }
                } else if (e[i].type == 11.0f) { // Elite Void Phantom
                    e[i].y += baseEnemySpeed * 0.7f;
                    e[i].x += e[i].dx;
                    if (e[i].x < 15.0f || e[i].x > W - ew - 15.0f) e[i].dx = -e[i].dx;
                    if (frameCount % 55 == 0) {
                        for (int j = 0; j < MAX_EBULLETS; j++) {
                            if (!eb[j].active) {
                                eb[j].active = 1.0f; eb[j].x = e[i].x + 15.0f; eb[j].y = e[i].y + 25.0f;
                                eb[j].dy = 3.2f; eb[j].dx = (p.x > e[i].x ? 1.0f : -1.0f);
                                break;
                            }
                        }
                    }
                } else if (e[i].type == 12.0f) { // Elite Command Cruiser
                    if (e[i].y < 70.0f) e[i].y += 0.6f;
                    e[i].x += e[i].dx;
                    if (e[i].x < 20.0f || e[i].x > W - ew - 20.0f) e[i].dx = -e[i].dx;
                    if (frameCount % 40 == 0) {
                        for (int a = -1; a <= 1; a++) {
                            for (int j = 0; j < MAX_EBULLETS; j++) {
                                if (!eb[j].active) {
                                    eb[j].active = 1.0f; eb[j].x = e[i].x + 18.0f; eb[j].y = e[i].y + 30.0f;
                                    eb[j].dy = 3.6f; eb[j].dx = (float)a * 1.5f;
                                    break;
                                }
                            }
                        }
                    }
                } else if (e[i].type == 13.0f) { // Siege Drop Pod
                    e[i].y += baseEnemySpeed * 2.2f;
                }

                if (e[i].type == 2.0f && (frameCount % 60 == 0) && (rnd() % 2 == 0)) {
                    for (int j = 0; j < MAX_EBULLETS; j++) {
                        if (!eb[j].active) {
                            eb[j].active = 1.0f;
                            eb[j].x = e[i].x + 10.0f; eb[j].y = e[i].y + 20.0f;
                            eb[j].dy = 3.5f; eb[j].dx = 0.0f;
                            break;
                        }
                    }
                }
            }

            // Laser collision
            if (laserTimer > 0) {
                if (p.x + 10.0f >= e[i].x && p.x + 10.0f <= e[i].x + ew && p.y > e[i].y) {
                    if (!e[i].cloaked) {
                        e[i].hp -= (overchargeTimer > 0 ? 3 : 1);
                        overchargeEnergy++; if (overchargeEnergy > 100) overchargeEnergy = 100;
                        hyperJumpEnergy++; if (hyperJumpEnergy > 100) hyperJumpEnergy = 100;
                    }
                    AddWeaponHitParticles(p.x + 10.0f, e[i].y + eh/2.0f, 2, RGB(0, 229, 255));
                }
            }

            // Player Collision
            if (p.x < e[i].x + ew && p.x + 20 > e[i].x && p.y < e[i].y + eh && p.y + 20 > e[i].y) {
                e[i].active = 0.0f;
                AddExplosion(e[i].x + ew/2.0f, e[i].y + eh/2.0f, 16, RGB(255, 152, 0));
                PlayerHit();
            }

            // Bullets Collision
            for (int j = 0; j < MAX_BULLETS; j++) {
                if (b[j].active && b[j].x < e[i].x + ew && b[j].x + 6 > e[i].x && b[j].y < e[i].y + eh && b[j].y + 12 > e[i].y) {
                    if (e[i].cloaked && (rnd() % 100 < 75)) {
                        // Bullets miss cloaked fighter!
                    } else {
                        b[j].active = 0.0f;
                        shotsHit++;
                        int dmg = (b[j].type == 1.0f) ? 6 : ((b[j].type == 2.0f) ? 10 : 1);
                        if (overchargeTimer > 0) dmg *= 3;
                        e[i].hp -= dmg;
                        overchargeEnergy++; if (overchargeEnergy > 100) overchargeEnergy = 100;
                        hyperJumpEnergy++; if (hyperJumpEnergy > 100) hyperJumpEnergy = 100;
                        AddWeaponHitParticles(b[j].x, b[j].y, weaponType, (overchargeTimer > 0 ? RGB(255, 234, 0) : RGB(0, 229, 255)));
                    }
                    break;
                }
            }

            if (e[i].hp <= 0) {
                e[i].active = 0.0f;
                comboTimer = 180;
                if (comboMultiplier < 10) comboMultiplier++;
                int baseScore = (e[i].type == 12.0f ? 800 : (e[i].type == 11.0f ? 500 : (e[i].type == 10.0f ? 400 : (e[i].type == 13.0f ? 250 : (e[i].type == 6.0f ? 300 : (e[i].type == 9.0f ? 200 : (e[i].type == 8.0f ? 150 : (e[i].type == 7.0f ? 120 : 30))))))));
                score += baseScore * comboMultiplier;
                enemiesKilled++;
                totalKills++;
                overchargeEnergy += (e[i].isElite ? 12 : 4);
                if (overchargeEnergy > 100) overchargeEnergy = 100;
                hyperJumpEnergy += (e[i].isElite ? 15 : 3);
                if (hyperJumpEnergy > 100) hyperJumpEnergy = 100;
                PlaySnd(1);
                AddExplosion(e[i].x + ew/2.0f, e[i].y + eh/2.0f, (e[i].isElite ? 25 : 14), (e[i].isElite ? RGB(255, 215, 0) : RGB(255, 152, 0)));

                if (score > highScore) { highScore = score; SaveLeaderboard(); }

                int dropChance = e[i].isElite ? 80 : 22;
                if ((rnd() % 100) < dropChance) {
                    for (int k = 0; k < MAX_POWERUPS; k++) {
                        if (!pu[k].active) {
                            pu[k].active = 1.0f; pu[k].x = e[i].x; pu[k].y = e[i].y; pu[k].dy = 1.8f;
                            pu[k].type = (float)(rnd() % 11); // Powerup 0..10
                            break;
                        }
                    }
                }
            } else if (e[i].y > H + 20) {
                e[i].active = 0.0f;
            }
        }
    }

    // Enemy Bullets Movement
    for (int i = 0; i < MAX_EBULLETS; i++) {
        if (eb[i].active) {
            if (timeStopTimer == 0) {
                eb[i].y += eb[i].dy;
                eb[i].x += eb[i].dx;
            }
            if (eb[i].y > H || eb[i].x < 0 || eb[i].x > W) eb[i].active = 0.0f;
            if (p.x < eb[i].x + 4 && p.x + 20 > eb[i].x && p.y < eb[i].y + 10 && p.y + 20 > eb[i].y) {
                eb[i].active = 0.0f;
                PlayerHit();
            }
        }
    }

    // Powerups Movement
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (pu[i].active) {
            pu[i].y += pu[i].dy;
            if (pu[i].y > H) pu[i].active = 0.0f;
            if (p.x < pu[i].x + 16 && p.x + 20 > pu[i].x && p.y < pu[i].y + 16 && p.y + 20 > pu[i].y) {
                pu[i].active = 0.0f;
                ApplyPowerup((int)pu[i].type);
                AddExplosion(pu[i].x + 8.0f, pu[i].y + 8.0f, 10, RGB(0, 229, 255));
                score += 50;
                PlaySnd(2);
            }
        }
    }

    frameCount++;
}

// GDI Rendering Helpers (Loop 2 & 10 Visual Upgrades)
void DrawPlayerShipGDI(HDC hdc, int x, int y, int shield, int frame) {
    HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);

    // Multi-Stage Animated Thruster Flames
    int flameH = 5 + (frame % 3) * 3;
    if (overchargeTimer > 0) flameH += 4;
    // Outer flame plume
    HBRUSH ofbr = CreateSolidBrush((frame % 2 == 0) ? (overchargeTimer > 0 ? RGB(255, 234, 0) : RGB(255, 60, 0)) : RGB(255, 145, 0));
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, ofbr);
    POINT outerPts[3] = { {x + 4, y + 20}, {x + 10, y + 23 + flameH}, {x + 16, y + 20} };
    Polygon(hdc, outerPts, 3);
    SelectObject(hdc, oldBr); DeleteObject(ofbr);

    // Core flame
    HBRUSH fbr = CreateSolidBrush(overchargeTimer > 0 ? RGB(0, 229, 255) : RGB(255, 234, 0));
    SelectObject(hdc, fbr);
    POINT flamePts[3] = { {x + 6, y + 20}, {x + 10, y + 20 + flameH}, {x + 14, y + 20} };
    Polygon(hdc, flamePts, 3);
    SelectObject(hdc, oldBr); DeleteObject(fbr);

    // White center plasma flame
    HBRUSH wfbr = CreateSolidBrush(RGB(255, 255, 255));
    SelectObject(hdc, wfbr);
    POINT whitePts[3] = { {x + 8, y + 20}, {x + 10, y + 17 + flameH / 2}, {x + 12, y + 20} };
    Polygon(hdc, whitePts, 3);
    SelectObject(hdc, oldBr); DeleteObject(wfbr);

    // Wingtip thrusters
    HBRUSH wtfbr = CreateSolidBrush(overchargeTimer > 0 ? RGB(255, 234, 0) : RGB(0, 229, 255));
    SelectObject(hdc, wtfbr);
    RECT lwt = {x, y + 16, x + 2, y + 20}; FillRect(hdc, &lwt, wtfbr);
    RECT rwt = {x + 18, y + 16, x + 20, y + 20}; FillRect(hdc, &rwt, wtfbr);
    SelectObject(hdc, oldBr); DeleteObject(wtfbr);

    // Ship Hull
    HBRUSH wbr = CreateSolidBrush(overchargeTimer > 0 ? RGB(255, 215, 0) : RGB(0, 176, 255));
    SelectObject(hdc, wbr);
    POINT wingPts[6] = { {x + 10, y}, {x + 20, y + 16}, {x + 15, y + 20}, {x + 10, y + 15}, {x + 5, y + 20}, {x + 0, y + 16} };
    Polygon(hdc, wingPts, 6);
    SelectObject(hdc, oldBr); DeleteObject(wbr);

    // Specular sheen sweep highlight across hull
    int sheenY = ((frame * 2) % 22);
    HPEN sheenPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    HPEN oPen2 = (HPEN)SelectObject(hdc, sheenPen);
    MoveToEx(hdc, x + 10 - sheenY / 3, y + sheenY, NULL);
    LineTo(hdc, x + 10 + sheenY / 3, y + sheenY + 1);
    SelectObject(hdc, oPen2); DeleteObject(sheenPen);

    // Trailing engine ion exhaust motes
    for (int m = 0; m < 3; m++) {
        int my = y + 21 + ((frame * 2 + m * 6) % 15);
        int mx = x + 10 + (FastSin(frame * 2 + m * 4) * 3) / 127;
        HBRUSH mbr = CreateSolidBrush((m % 2 == 0) ? RGB(0, 229, 255) : RGB(255, 234, 0));
        RECT mr = {mx - 1, my, mx + 1, my + 2};
        FillRect(hdc, &mr, mbr);
        DeleteObject(mbr);
    }

    // Cockpit
    HBRUSH cbr = CreateSolidBrush(overchargeTimer > 0 ? RGB(0, 229, 255) : RGB(255, 255, 255));
    SelectObject(hdc, cbr);
    Ellipse(hdc, x + 7, y + 4, x + 13, y + 12);
    SelectObject(hdc, oldBr); DeleteObject(cbr);

    // Overcharge Hyper-Corona Field
    if (overchargeTimer > 0) {
        HPEN ocPen = CreatePen(PS_SOLID, 2, (frame % 2 == 0) ? RGB(255, 234, 0) : RGB(0, 229, 255));
        SelectObject(hdc, ocPen);
        HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
        SelectObject(hdc, nullBr);
        int r = 16 + (frame % 4) * 2;
        Ellipse(hdc, x + 10 - r, y + 10 - r, x + 10 + r, y + 10 + r);
        // Lightning spark arcs
        MoveToEx(hdc, x + 10, y, NULL);
        LineTo(hdc, x + 10 + (rnd() % 20) - 10, y - 8 - (rnd() % 8));
        SelectObject(hdc, oldBr); DeleteObject(ocPen);
    } else if (hyperShieldTimer > 0) {
        HPEN hpen = CreatePen(PS_SOLID, 3, RGB(255, 234, 0));
        SelectObject(hdc, hpen);
        HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
        SelectObject(hdc, nullBr);
        Ellipse(hdc, x - 6, y - 6, x + 26, y + 26);
        SelectObject(hdc, oldBr); DeleteObject(hpen);
    } else if (shield) {
        HPEN spen = CreatePen(PS_SOLID, 2, RGB(0, 229, 255));
        SelectObject(hdc, spen);
        HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
        SelectObject(hdc, nullBr);
        Ellipse(hdc, x - 4, y - 4, x + 24, y + 24);
        SelectObject(hdc, oldBr); DeleteObject(spen);
    }

    if (laserTimer > 0) {
        HPEN lpen = CreatePen(PS_SOLID, 5, RGB(0, 229, 255));
        SelectObject(hdc, lpen);
        MoveToEx(hdc, x + 10, y, NULL);
        LineTo(hdc, x + 10, 0);
        SelectObject(hdc, oldPen); DeleteObject(lpen);
    }

    SelectObject(hdc, oldPen);
}

void DrawEnemyShipGDI(HDC hdc, float fx, float fy, float ftype, int cloaked, int hp, int maxHp, int frame) {
    int x = (int)fx, y = (int)fy, type = (int)ftype;
    HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);

    // Enemy Rear Thruster Flame
    if (type != 5 && type != 9 && type != 13) {
        int eflame = 3 + (frameCount % 3) * 2;
        COLORREF efcol = (type == 4 ? RGB(0, 229, 255) : (type == 7 ? RGB(255, 235, 59) : (type == 10 ? RGB(255, 215, 0) : (type == 11 ? RGB(213, 0, 249) : RGB(255, 60, 0)))));
        HBRUSH efbr = CreateSolidBrush(efcol);
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, efbr);
        POINT efPts[3] = { {x + 7, y}, {x + 10, y - eflame}, {x + 13, y} };
        Polygon(hdc, efPts, 3);
        SelectObject(hdc, oldBr); DeleteObject(efbr);
    }

    COLORREF col = RGB(255, 23, 68);
    if (type == 1) col = RGB(255, 145, 0);
    else if (type == 2) col = RGB(213, 0, 249);
    else if (type == 3) col = RGB(120, 144, 156);
    else if (type == 4) col = RGB(0, 229, 255);
    else if (type == 5 || type == 9) col = RGB(141, 110, 99);
    else if (type == 6) col = RGB(198, 40, 40);
    else if (type == 7) col = RGB(255, 235, 59); // Kamikaze Spike
    else if (type == 8) col = cloaked ? RGB(30, 40, 60) : RGB(103, 58, 183);
    else if (type == 10) col = RGB(255, 40, 40);  // Elite Valkyrie
    else if (type == 11) col = cloaked ? RGB(40, 20, 60) : RGB(170, 0, 255); // Elite Phantom
    else if (type == 12) col = RGB(255, 215, 0);  // Elite Command Cruiser
    else if (type == 13) col = RGB(100, 100, 120); // Siege Drop Pod

    int isDamaged = (hp < maxHp / 2) && (frame % 4 < 2);
    if (isDamaged && type != 5 && type != 9) col = RGB(255, 255, 255);

    HBRUSH br = CreateSolidBrush(col);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);

    if (type == 0 || type == 7) {
        POINT pts[6] = { {x, y + 2}, {x + 10, y + 18}, {x + 20, y + 2}, {x + 13, y + 6}, {x + 10, y}, {x + 7, y + 6} };
        Polygon(hdc, pts, 6);
    } else if (type == 1) {
        POINT pts[4] = { {x + 10, y + 20}, {x + 20, y + 4}, {x + 10, y}, {x, y + 4} };
        Polygon(hdc, pts, 4);
    } else if (type == 2) {
        Ellipse(hdc, x, y + 6, x + 20, y + 18);
    } else if (type == 3) {
        RECT rc = {x + 2, y + 2, x + 18, y + 18};
        FillRect(hdc, &rc, br);
    } else if (type == 4 || type == 8) {
        POINT pts[4] = { {x + 10, y + 18}, {x + 20, y}, {x + 10, y + 6}, {x, y} };
        Polygon(hdc, pts, 4);
    } else if (type == 5 || type == 9) {
        int sz = (type == 9) ? 30 : 18;
        POINT pts[8] = { {x + sz/2, y}, {x + sz, y + sz/4}, {x + sz*7/8, y + sz}, {x + sz/2, y + sz*7/8},
                         {x + sz/8, y + sz}, {x, y + sz*3/4}, {x + sz/4, y + sz/4}, {x + sz/4, y} };
        Polygon(hdc, pts, 8);
    } else if (type == 6) {
        POINT pts[4] = { {x + 18, y + 34}, {x + 34, y + 8}, {x + 18, y + 12}, {x + 2, y + 8} };
        Polygon(hdc, pts, 4);
    } else if (type == 10) { // Elite Valkyrie
        POINT pts[6] = { {x + 10, y + 22}, {x + 20, y + 2}, {x + 14, y + 8}, {x + 10, y}, {x + 6, y + 8}, {x, y + 2} };
        Polygon(hdc, pts, 6);
        HPEN glow = CreatePen(PS_SOLID, 1, RGB(255, 234, 0));
        SelectObject(hdc, glow);
        HBRUSH nullB = (HBRUSH)GetStockObject(NULL_BRUSH);
        SelectObject(hdc, nullB);
        Ellipse(hdc, x - 2, y - 2, x + 22, y + 24);
        SelectObject(hdc, oldBr); DeleteObject(glow);
    } else if (type == 11) { // Elite Void Phantom
        POINT pts[8] = { {x + 18, y + 32}, {x + 34, y + 12}, {x + 28, y + 2}, {x + 18, y + 8}, {x + 8, y + 2}, {x + 2, y + 12}, {x + 10, y + 22}, {x + 18, y + 32} };
        Polygon(hdc, pts, 8);
        HPEN glow = CreatePen(PS_SOLID, 1, RGB(0, 229, 255));
        SelectObject(hdc, glow);
        HBRUSH nullB = (HBRUSH)GetStockObject(NULL_BRUSH);
        SelectObject(hdc, nullB);
        Ellipse(hdc, x, y, x + 36, y + 34);
        SelectObject(hdc, oldBr); DeleteObject(glow);
    } else if (type == 12) { // Elite Command Cruiser
        POINT pts[8] = { {x + 18, y + 34}, {x + 36, y + 18}, {x + 32, y + 2}, {x + 22, y + 6}, {x + 18, y}, {x + 14, y + 6}, {x + 4, y + 2}, {x, y + 18} };
        Polygon(hdc, pts, 8);
        HBRUSH coreB = CreateSolidBrush(RGB(255, 234, 0));
        SelectObject(hdc, coreB);
        Ellipse(hdc, x + 13, y + 12, x + 23, y + 22);
        SelectObject(hdc, oldBr); DeleteObject(coreB);
    } else if (type == 13) { // Siege Drop Pod
        POINT pts[8] = { {x + 6, y}, {x + 14, y}, {x + 20, y + 6}, {x + 20, y + 16}, {x + 14, y + 22}, {x + 6, y + 22}, {x, y + 16}, {x, y + 6} };
        Polygon(hdc, pts, 8);
        HBRUSH vent = CreateSolidBrush(RGB(255, 60, 0));
        SelectObject(hdc, vent);
        RECT vr = {x + 8, y + 2, x + 12, y + 8}; FillRect(hdc, &vr, vent);
        SelectObject(hdc, oldBr); DeleteObject(vent);
    }

    SelectObject(hdc, oldBr); DeleteObject(br);

    // Elite Health Bar
    if (type >= 10 && type <= 12) {
        int barW = (type == 10) ? 20 : 36;
        int fillW = (int)((float)barW * ((float)hp / (float)maxHp));
        if (fillW < 0) fillW = 0;
        HBRUSH bgB = CreateSolidBrush(RGB(40, 40, 40));
        RECT bgR = {x, y - 6, x + barW, y - 3}; FillRect(hdc, &bgR, bgB); DeleteObject(bgB);
        HBRUSH fgB = CreateSolidBrush(RGB(255, 215, 0));
        RECT fgR = {x, y - 6, x + fillW, y - 3}; FillRect(hdc, &fgR, fgB); DeleteObject(fgB);
    }

    SelectObject(hdc, oldPen);
}

void DrawDroneWingsGDI(HDC hdc, int frame) {
    if (droneCount <= 0) return;
    HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);

    for (int d = 0; d < droneCount; d++) {
        int dx = (int)drones[d].x, dy = (int)drones[d].y;
        // Drone Thruster flame
        int flame = 3 + (frame % 3) * 2;
        HBRUSH fbr = CreateSolidBrush((frame % 2 == 0) ? RGB(0, 229, 255) : RGB(255, 234, 0));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, fbr);
        POINT fpts[3] = { {dx + 3, dy + 10}, {dx + 5, dy + 10 + flame}, {dx + 7, dy + 10} };
        Polygon(hdc, fpts, 3);
        SelectObject(hdc, oldBr); DeleteObject(fbr);

        // Drone Body
        HBRUSH dbr = CreateSolidBrush(RGB(0, 176, 255));
        oldBr = (HBRUSH)SelectObject(hdc, dbr);
        POINT pts[5] = { {dx + 5, dy}, {dx + 10, dy + 8}, {dx + 8, dy + 10}, {dx + 2, dy + 10}, {dx, dy + 8} };
        Polygon(hdc, pts, 5);
        SelectObject(hdc, oldBr); DeleteObject(dbr);

        // Specular glint on drone wing
        int dSheen = ((frame * 2 + d * 8) % 10);
        HPEN dpen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        HPEN oP = (HPEN)SelectObject(hdc, dpen);
        MoveToEx(hdc, dx + 3, dy + dSheen, NULL);
        LineTo(hdc, dx + 7, dy + dSheen);
        SelectObject(hdc, oP); DeleteObject(dpen);

        // Drone Plasma Cockpit
        HBRUSH cbr = CreateSolidBrush(RGB(255, 234, 0));
        SelectObject(hdc, cbr);
        Ellipse(hdc, dx + 3, dy + 3, dx + 7, dy + 7);
        SelectObject(hdc, oldBr); DeleteObject(cbr);

        // Point Defense Energy Ring
        HPEN spen = CreatePen(PS_SOLID, 1, (frame % 4 < 2) ? RGB(0, 229, 255) : RGB(255, 255, 255));
        SelectObject(hdc, spen);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Ellipse(hdc, dx - 2, dy - 2, dx + 12, dy + 12);
        SelectObject(hdc, oldPen); DeleteObject(spen);
    }
    SelectObject(hdc, oldPen);
}

void DrawHyperJumpEffectGDI(HDC hdc, int frame) {
    if (hyperJumpTimer <= 0) return;
    static const float cosTable[16] = { 1.0f, 0.9238f, 0.7071f, 0.3826f, 0.0f, -0.3826f, -0.7071f, -0.9238f, -1.0f, -0.9238f, -0.7071f, -0.3826f, 0.0f, 0.3826f, 0.7071f, 0.9238f };
    static const float sinTable[16] = { 0.0f, 0.3826f, 0.7071f, 0.9238f, 1.0f, 0.9238f, 0.7071f, 0.3826f, 0.0f, -0.3826f, -0.7071f, -0.9238f, -1.0f, -0.9238f, -0.7071f, -0.3826f };

    HPEN wPen = CreatePen(PS_SOLID, 2, (frame % 2 == 0) ? RGB(0, 229, 255) : RGB(255, 255, 255));
    HPEN oldPen = (HPEN)SelectObject(hdc, wPen);

    for (int i = 0; i < 16; i++) {
        float len1 = 15.0f + (float)(rnd() % 30);
        float len2 = len1 + 60.0f + (float)(rnd() % 60);
        int sx = (int)(p.x + 10.0f + cosTable[i] * len1);
        int sy = (int)(p.y + 10.0f + sinTable[i] * len1);
        int ex = (int)(p.x + 10.0f + cosTable[i] * len2);
        int ey = (int)(p.y + 10.0f + sinTable[i] * len2);
        MoveToEx(hdc, sx, sy, NULL);
        LineTo(hdc, ex, ey);
    }
    SelectObject(hdc, oldPen); DeleteObject(wPen);
}

void DrawCyberReticleGDI(HDC hdc, int x, int y, int dirX, int dirY) {
    HPEN cpen = CreatePen(PS_SOLID, 2, RGB(0, 229, 255));
    HPEN oldPen = (HPEN)SelectObject(hdc, cpen);

    // L-bar with 45-degree chamfer
    MoveToEx(hdc, x, y + dirY * 20, NULL);
    LineTo(hdc, x, y + dirY * 5);
    LineTo(hdc, x + dirX * 5, y);
    LineTo(hdc, x + dirX * 20, y);

    // Tech tick marks
    HPEN ypen = CreatePen(PS_SOLID, 1, RGB(255, 234, 0));
    SelectObject(hdc, ypen);
    MoveToEx(hdc, x + dirX * 9, y + dirY * 2, NULL); LineTo(hdc, x + dirX * 9, y + dirY * 5);
    MoveToEx(hdc, x + dirX * 15, y + dirY * 2, NULL); LineTo(hdc, x + dirX * 15, y + dirY * 5);
    MoveToEx(hdc, x + dirX * 2, y + dirY * 9, NULL); LineTo(hdc, x + dirX * 5, y + dirY * 9);
    MoveToEx(hdc, x + dirX * 2, y + dirY * 15, NULL); LineTo(hdc, x + dirX * 5, y + dirY * 15);

    // Corner status diode
    HBRUSH dbr = CreateSolidBrush(RGB(255, 255, 255));
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, dbr);
    RECT dr = {x + dirX * 3 - 1, y + dirY * 3 - 1, x + dirX * 3 + 2, y + dirY * 3 + 2};
    FillRect(hdc, &dr, dbr);

    SelectObject(hdc, oldBr); SelectObject(hdc, oldPen);
    DeleteObject(cpen); DeleteObject(ypen); DeleteObject(dbr);
}

void DrawSciFiHUDFrame(HDC hdc, int frame) {
    int shim = 160 + (FastSin(frame * 2) * 60) / 127;
    COLORREF hudCol = RGB(0, shim, 255);
    HPEN hHudPen = CreatePen(PS_SOLID, 1, hudCol);
    HPEN oldPen = (HPEN)SelectObject(hdc, hHudPen);
    HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, nullBr);

    // Outer & inner perimeter
    Rectangle(hdc, 5, 5, W - 5, H - 5);
    Rectangle(hdc, 8, 8, W - 8, H - 8);

    // Traveling Specular Glint along Perimeter
    int perimW = W - 10;
    int perimH = H - 10;
    int totalLen = 2 * (perimW + perimH);
    int glintPos = (frame * 4) % totalLen;
    int gx = 5, gy = 5;
    if (glintPos < perimW) {
        gx = 5 + glintPos; gy = 5;
    } else if (glintPos < perimW + perimH) {
        gx = 5 + perimW; gy = 5 + (glintPos - perimW);
    } else if (glintPos < 2 * perimW + perimH) {
        gx = 5 + perimW - (glintPos - perimW - perimH); gy = 5 + perimH;
    } else {
        gx = 5; gy = 5 + perimH - (glintPos - 2 * perimW - perimH);
    }
    HBRUSH gbr = CreateSolidBrush(RGB(255, 255, 255));
    RECT gr = {gx - 2, gy - 2, gx + 3, gy + 3};
    FillRect(hdc, &gr, gbr);
    DeleteObject(gbr);

    // 4 Corner Cybernetic Reticles
    DrawCyberReticleGDI(hdc, 5, 5, 1, 1);          // Top-Left
    DrawCyberReticleGDI(hdc, W - 5, 5, -1, 1);     // Top-Right
    DrawCyberReticleGDI(hdc, W - 5, H - 5, -1, -1); // Bottom-Right
    DrawCyberReticleGDI(hdc, 5, H - 5, 1, -1);     // Bottom-Left

    SelectObject(hdc, oldPen); SelectObject(hdc, oldBr);
    DeleteObject(hHudPen);
}

void DrawBossGDI(HDC hdc, float fx, float fy, int frame) {
    int x = (int)fx, y = (int)fy;
    HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);

    int isEnraged = (bossHp < bossMaxHp / 2);
    int isCritical = (bossHp < bossMaxHp / 4);

    if (bossIsDreadnought) {
        // Massive Dreadnought Warship (120 wide, 65 tall)
        HBRUSH hullBr = CreateSolidBrush(isEnraged ? RGB(160, 20, 40) : RGB(60, 70, 90));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, hullBr);
        POINT dpts[10] = {
            {x + 60, y + 62}, {x + 100, y + 45}, {x + 120, y + 25}, {x + 115, y + 5}, {x + 85, y + 2},
            {x + 60, y + 10}, {x + 35, y + 2}, {x + 5, y + 5}, {x + 0, y + 25}, {x + 20, y + 45}
        };
        Polygon(hdc, dpts, 10);
        SelectObject(hdc, oldBr); DeleteObject(hullBr);

        // Armor Plates & Girders
        HBRUSH armorBr = CreateSolidBrush(isCritical ? RGB(90, 20, 20) : RGB(100, 115, 135));
        oldBr = (HBRUSH)SelectObject(hdc, armorBr);
        POINT lPlate[4] = { {x + 15, y + 12}, {x + 45, y + 12}, {x + 40, y + 38}, {x + 10, y + 30} };
        Polygon(hdc, lPlate, 4);
        POINT rPlate[4] = { {x + 75, y + 12}, {x + 105, y + 12}, {x + 110, y + 30}, {x + 80, y + 38} };
        Polygon(hdc, rPlate, 4);
        SelectObject(hdc, oldBr); DeleteObject(armorBr);

        // Left Shield Generator Subsystem
        if (dreadGenL > 0) {
            HBRUSH genBr = CreateSolidBrush(RGB(0, 229, 255));
            oldBr = (HBRUSH)SelectObject(hdc, genBr);
            Ellipse(hdc, x + 18, y + 18, x + 38, y + 38);
            SelectObject(hdc, oldBr); DeleteObject(genBr);
            HPEN sPen = CreatePen(PS_SOLID, 2, (frame % 4 < 2) ? RGB(0, 229, 255) : RGB(128, 216, 255));
            SelectObject(hdc, sPen);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Arc(hdc, x - 10, y - 5, x + 65, y + 65, x + 60, y + 60, x, y);
            SelectObject(hdc, oldPen); DeleteObject(sPen);
        } else {
            HBRUSH charredBr = CreateSolidBrush(RGB(30, 30, 30));
            oldBr = (HBRUSH)SelectObject(hdc, charredBr);
            Ellipse(hdc, x + 18, y + 18, x + 38, y + 38);
            SelectObject(hdc, oldBr); DeleteObject(charredBr);
        }

        // Right Shield Generator Subsystem
        if (dreadGenR > 0) {
            HBRUSH genBr = CreateSolidBrush(RGB(0, 229, 255));
            oldBr = (HBRUSH)SelectObject(hdc, genBr);
            Ellipse(hdc, x + 82, y + 18, x + 102, y + 38);
            SelectObject(hdc, oldBr); DeleteObject(genBr);
            HPEN sPen = CreatePen(PS_SOLID, 2, (frame % 4 < 2) ? RGB(0, 229, 255) : RGB(128, 216, 255));
            SelectObject(hdc, sPen);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Arc(hdc, x + 55, y - 5, x + 130, y + 65, x + 120, y, x + 60, y + 60);
            SelectObject(hdc, oldPen); DeleteObject(sPen);
        } else {
            HBRUSH charredBr = CreateSolidBrush(RGB(30, 30, 30));
            oldBr = (HBRUSH)SelectObject(hdc, charredBr);
            Ellipse(hdc, x + 82, y + 18, x + 102, y + 38);
            SelectObject(hdc, oldBr); DeleteObject(charredBr);
        }

        // Central Mega-Ion Cannon
        HBRUSH ionBr = CreateSolidBrush(dreadIonCharge >= 70 ? RGB(255, 234, 0) : RGB(0, 229, 255));
        oldBr = (HBRUSH)SelectObject(hdc, ionBr);
        RECT ionRc = {x + 52, y + 35, x + 68, y + 58};
        FillRect(hdc, &ionRc, ionBr);
        SelectObject(hdc, oldBr); DeleteObject(ionBr);

        // Telegraph targeting guide laser
        if (dreadIonCharge >= 70 && dreadIonBeamTimer <= 0) {
            HPEN tPen = CreatePen(PS_SOLID, 1, (frame % 4 < 2) ? RGB(255, 23, 68) : RGB(255, 234, 0));
            SelectObject(hdc, tPen);
            MoveToEx(hdc, x + 60, y + 58, NULL);
            LineTo(hdc, x + 60, H);
            SelectObject(hdc, oldPen); DeleteObject(tPen);
        }

        // Active Mega-Ion Cannon Beam
        if (dreadIonBeamTimer > 0) {
            HBRUSH beamBr = CreateSolidBrush((frame % 2 == 0) ? RGB(0, 229, 255) : RGB(0, 176, 255));
            RECT bRc = {x + 48, y + 58, x + 72, H};
            FillRect(hdc, &bRc, beamBr);
            DeleteObject(beamBr);

            HBRUSH coreBr = CreateSolidBrush(RGB(255, 255, 255));
            RECT cRc = {x + 55, y + 58, x + 65, H};
            FillRect(hdc, &cRc, coreBr);
            DeleteObject(coreBr);
        }

        // Flak Turrets on Wings
        HBRUSH flakBr = CreateSolidBrush(RGB(255, 145, 0));
        oldBr = (HBRUSH)SelectObject(hdc, flakBr);
        Ellipse(hdc, x + 10, y + 35, x + 24, y + 49);
        Ellipse(hdc, x + 96, y + 35, x + 110, y + 49);
        SelectObject(hdc, oldBr); DeleteObject(flakBr);
    } else if (bossIsMothership) {
        if (isCritical) {
            HBRUSH ib = CreateSolidBrush(RGB(40, 40, 40));
            HBRUSH oldBr = (HBRUSH)SelectObject(hdc, ib);
            POINT ipts[8] = { {x + 45, y + 58}, {x + 85, y + 35}, {x + 90, y + 10}, {x + 65, y + 2}, {x + 45, y + 12}, {x + 25, y + 2}, {x + 0, y + 10}, {x + 5, y + 35} };
            Polygon(hdc, ipts, 8);
            
            HPEN glowPen = CreatePen(PS_SOLID, 2, RGB(0, 229, 255));
            SelectObject(hdc, glowPen);
            MoveToEx(hdc, x + 20, y + 20, NULL); LineTo(hdc, x + 30, y + 30); LineTo(hdc, x + 40, y + 20);
            MoveToEx(hdc, x + 70, y + 20, NULL); LineTo(hdc, x + 60, y + 30); LineTo(hdc, x + 50, y + 20);
            SelectObject(hdc, oldBr); DeleteObject(ib); DeleteObject(glowPen);

            HBRUSH br = CreateSolidBrush(RGB(100, 14, 30));
            oldBr = (HBRUSH)SelectObject(hdc, br);
            POINT pts1[3] = { {x + 45, y + 58}, {x + 65, y + 35}, {x + 25, y + 35} };
            Polygon(hdc, pts1, 3);
            SelectObject(hdc, oldBr); DeleteObject(br);
        } else {
            HBRUSH br = CreateSolidBrush(isEnraged ? RGB(183, 28, 28) : RGB(136, 14, 79));
            HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
            POINT pts[8] = { {x + 45, y + 58}, {x + 85, y + 35}, {x + 90, y + 10}, {x + 65, y + 2}, {x + 45, y + 12}, {x + 25, y + 2}, {x + 0, y + 10}, {x + 5, y + 35} };
            Polygon(hdc, pts, 8);
            SelectObject(hdc, oldBr); DeleteObject(br);
        }

        int charge = bossAttackTimer % 35;
        if (charge > 15 && !IsMothershipShieldActive()) {
            int r = charge - 15;
            HPEN cPen = CreatePen(PS_SOLID, 2 + r / 4, RGB(0, 229, 255));
            HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
            HBRUSH oldBr = (HBRUSH)SelectObject(hdc, nullBr);
            HPEN oPen = (HPEN)SelectObject(hdc, cPen);
            Ellipse(hdc, x + 45 - r, y + 45 - r, x + 45 + r, y + 45 + r);
            SelectObject(hdc, oldBr); SelectObject(hdc, oPen); DeleteObject(cPen);
        }

        if (isEnraged && frame % 2 == 0) {
            HPEN lpen = CreatePen(PS_SOLID, 2, RGB(0, 229, 255));
            SelectObject(hdc, lpen);
            MoveToEx(hdc, x + 45, y + 30, NULL);
            LineTo(hdc, x + 45 + (rnd() % 60) - 30, y + 30 + (rnd() % 40) - 20);
            SelectObject(hdc, oldPen); DeleteObject(lpen);
        }

        COLORREF coreCol = isEnraged ? ((frame % 4 < 2) ? RGB(255, 234, 0) : RGB(255, 23, 68)) : ((frame % 8 < 4) ? RGB(0, 229, 255) : RGB(255, 23, 68));
        HBRUSH cbr = CreateSolidBrush(coreCol);
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, cbr);
        Ellipse(hdc, x + 35, y + 20, x + 55, y + 40);
        SelectObject(hdc, oldBr); DeleteObject(cbr);

        float tOffsetsX[] = {5, 25, 65, 85};
        for (int i = 0; i < 4; i++) {
            if (turretActive[i]) {
                HBRUSH tbr = CreateSolidBrush(RGB(255, 145, 0));
                HBRUSH oldTBr = (HBRUSH)SelectObject(hdc, tbr);
                Ellipse(hdc, (int)(x + tOffsetsX[i] - 4), (int)(y + 25), (int)(x + tOffsetsX[i] + 12), (int)(y + 41));
                SelectObject(hdc, oldTBr); DeleteObject(tbr);
            }
        }

        if (IsMothershipShieldActive()) {
            HPEN spen = CreatePen(PS_SOLID, 3, RGB(0, 229, 255));
            SelectObject(hdc, spen);
            HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
            HBRUSH oldSBr = (HBRUSH)SelectObject(hdc, nullBr);
            Ellipse(hdc, x - 10, y - 10, x + 100, y + 68);
            SelectObject(hdc, oldSBr); DeleteObject(spen);
        }
    } else { // Normal Dreadnought Boss
        if (isCritical) {
            HBRUSH ib = CreateSolidBrush(RGB(40, 40, 40));
            HBRUSH oldBr = (HBRUSH)SelectObject(hdc, ib);
            POINT ipts[6] = { {x + 30, y + 48}, {x + 58, y + 12}, {x + 45, y + 2}, {x + 30, y + 14}, {x + 15, y + 2}, {x + 2, y + 12} };
            Polygon(hdc, ipts, 6);

            HPEN glowPen = CreatePen(PS_SOLID, 2, RGB(213, 0, 249));
            SelectObject(hdc, glowPen);
            MoveToEx(hdc, x + 15, y + 20, NULL); LineTo(hdc, x + 45, y + 20);
            SelectObject(hdc, oldBr); DeleteObject(ib); DeleteObject(glowPen);

            HBRUSH br = CreateSolidBrush(RGB(100, 14, 30));
            oldBr = (HBRUSH)SelectObject(hdc, br);
            POINT pts1[3] = { {x + 30, y + 48}, {x + 45, y + 24}, {x + 15, y + 24} };
            Polygon(hdc, pts1, 3);
            SelectObject(hdc, oldBr); DeleteObject(br);
        } else {
            HBRUSH br = CreateSolidBrush(isEnraged ? RGB(213, 0, 249) : RGB(183, 28, 28));
            HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
            POINT pts[6] = { {x + 30, y + 48}, {x + 58, y + 12}, {x + 45, y + 2}, {x + 30, y + 14}, {x + 15, y + 2}, {x + 2, y + 12} };
            Polygon(hdc, pts, 6);
            SelectObject(hdc, oldBr); DeleteObject(br);
        }

        int charge = bossAttackTimer % 45;
        if (charge > 20) {
            int r = charge - 20;
            HPEN cPen = CreatePen(PS_SOLID, 2 + r / 4, RGB(255, 234, 0));
            HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
            HBRUSH oldBr = (HBRUSH)SelectObject(hdc, nullBr);
            HPEN oPen = (HPEN)SelectObject(hdc, cPen);
            Ellipse(hdc, x + 30 - r, y + 48 - r, x + 30 + r, y + 48 + r);
            SelectObject(hdc, oldBr); SelectObject(hdc, oPen); DeleteObject(cPen);
        }

        if (isEnraged && frame % 2 == 0) {
            HPEN lpen = CreatePen(PS_SOLID, 2, RGB(255, 234, 0));
            SelectObject(hdc, lpen);
            MoveToEx(hdc, x + 30, y + 24, NULL);
            LineTo(hdc, x + 30 + (rnd() % 40) - 20, y + 24 + (rnd() % 30) - 15);
            SelectObject(hdc, oldPen); DeleteObject(lpen);
        }

        COLORREF coreCol = isEnraged ? ((frame % 4 < 2) ? RGB(255, 255, 255) : RGB(255, 23, 68)) : ((frame % 10 < 5) ? RGB(255, 234, 0) : RGB(255, 23, 68));
        HBRUSH cbr = CreateSolidBrush(coreCol);
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, cbr);
        Ellipse(hdc, x + 20, y + 14, x + 40, y + 34);

        SelectObject(hdc, oldBr); DeleteObject(cbr);
    }
    SelectObject(hdc, oldPen);
}

void DrawPowerupGDI(HDC hdc, float fx, float fy, float ftype, int frame) {
    int x = (int)fx, y = (int)fy, type = (int)ftype;
    COLORREF cols[11] = { RGB(0, 230, 118), RGB(0, 229, 255), RGB(61, 90, 255), RGB(255, 23, 68), RGB(255, 234, 0), RGB(213, 0, 249), RGB(255, 215, 0), RGB(0, 176, 255), RGB(255, 100, 200), RGB(255, 234, 0), RGB(0, 229, 255) };
    COLORREF c = (type >= 0 && type < 11) ? cols[type] : RGB(255, 255, 255);
    HBRUSH br = CreateSolidBrush(c);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    int pulse = (frame % 20 < 10) ? 1 : 0;
    if (type == 10) { // Drone Wing Pod
        POINT pts[6] = { {x + 8, y - pulse}, {x + 16 + pulse, y + 6}, {x + 12, y + 16 + pulse}, {x + 8, y + 12}, {x + 4, y + 16 + pulse}, {x - pulse, y + 6} };
        Polygon(hdc, pts, 6);
        HBRUSH coreB = CreateSolidBrush(RGB(255, 234, 0));
        SelectObject(hdc, coreB);
        Ellipse(hdc, x + 5, y + 5, x + 11, y + 11);
        SelectObject(hdc, oldBr); DeleteObject(coreB);
    } else if (type == 9) { // Overcharge Energy Core
        POINT pts[4] = { {x + 8, y - pulse}, {x + 16 + pulse, y + 8}, {x + 8, y + 16 + pulse}, {x - pulse, y + 8} };
        Polygon(hdc, pts, 4);
        HBRUSH coreB = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, coreB);
        Ellipse(hdc, x + 4, y + 4, x + 12, y + 12);
        SelectObject(hdc, oldBr); DeleteObject(coreB);
    } else if (type % 3 == 0) {
        POINT pts[3] = { {x+8, y+2-pulse}, {x+14+pulse, y+14+pulse}, {x+2-pulse, y+14+pulse} };
        Polygon(hdc, pts, 3);
    } else if (type % 3 == 1) {
        RECT r = {x+2-pulse, y+2-pulse, x+14+pulse, y+14+pulse};
        FillRect(hdc, &r, br);
    } else {
        Ellipse(hdc, x-pulse, y-pulse, x + 16 + pulse, y + 16 + pulse);
    }
    SelectObject(hdc, oldBr); SelectObject(hdc, oldPen);
    DeleteObject(br); DeleteObject(pen);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            seed = GetTickCount();
            LoadLeaderboard();
            hFontTitle = CreateFontA(-18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            hFontMenu = CreateFontA(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            hFontHUD = CreateFontA(-11, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            for (int i = 0; i < MAX_STARS; i++) {
                stars[i].x = (float)(rnd() % W);
                stars[i].y = (float)(rnd() % H);
                int layer = (i % 2 == 0) ? 0 : ((i % 5 == 0) ? 2 : 1);
                stars[i].layer = layer;
                stars[i].speed = (layer == 0) ? 0.4f : ((layer == 1) ? 1.2f : 2.5f);
                stars[i].size = (layer == 0) ? 1 : ((layer == 1) ? 1 : 2);
            }
            SetTimer(hwnd, 1, 16, NULL);
            break;

        case WM_KEYDOWN:
            if (gameState == STATE_MENU) {
                int opts = HasSavedGame() ? 6 : 5;
                if (wParam == 'H' || wParam == VK_F1) {
                    previousState = STATE_MENU;
                    gameState = STATE_HELP;
                } else {
                    if (wParam == VK_UP || wParam == 'W') menuIndex = (menuIndex - 1 + opts) % opts;
                    else if (wParam == VK_DOWN || wParam == 'S') menuIndex = (menuIndex + 1) % opts;
                    else if (wParam == VK_RETURN || wParam == VK_SPACE) {
                        int saved = HasSavedGame();
                        if (menuIndex == 0) StartNewGame(MODE_CLASSIC);
                        else if (saved && menuIndex == 1) LoadGameState();
                        else if ((!saved && menuIndex == 1) || (saved && menuIndex == 2)) gameState = STATE_LEADERBOARD;
                        else if ((!saved && menuIndex == 2) || (saved && menuIndex == 3)) gameState = STATE_MODE_SELECT;
                        else if ((!saved && menuIndex == 3) || (saved && menuIndex == 4)) { previousState = STATE_MENU; gameState = STATE_HELP; }
                        else if ((!saved && menuIndex == 4) || (saved && menuIndex == 5)) { ExportHighScoresJSON(); }
                    }
                }
            } else if (gameState == STATE_HELP) {
                if (wParam == 'H' || wParam == VK_F1 || wParam == VK_ESCAPE || wParam == VK_RETURN || wParam == VK_SPACE) {
                    gameState = (previousState == STATE_PLAYING || previousState == STATE_PAUSED) ? STATE_PAUSED : STATE_MENU;
                }
            } else if (gameState == STATE_MODE_SELECT) {
                if (wParam == VK_UP || wParam == 'W') modeIndex = (modeIndex - 1 + 3) % 3;
                else if (wParam == VK_DOWN || wParam == 'S') modeIndex = (modeIndex + 1) % 3;
                else if (wParam == VK_RETURN || wParam == VK_SPACE) StartNewGame(modeIndex);
                else if (wParam == VK_ESCAPE) gameState = STATE_MENU;
            } else if (gameState == STATE_PLAYING) {
                if (wParam == kbPause || wParam == VK_ESCAPE) { gameState = STATE_PAUSED; menuIndex = 0; }
                else if (wParam == 'H' || wParam == VK_F1) { previousState = STATE_PLAYING; gameState = STATE_HELP; }
                else if (wParam == kbHyperJump || wParam == 'J') UseHyperJump();
                else if (wParam == kbDeployDrone || wParam == 'W') DeployDroneWing();
                else if (wParam == kbTimeStop) UseTimeStop();
                else if (wParam == kbDash) UseTacticalDash();
                else if (wParam == kbBomb || wParam == 'X') UseSmartBomb();
                else if (wParam == kbShield) UseHyperShield();
                else if (wParam == kbOvercharge || wParam == 'O' || wParam == 'C') UseOvercharge();
            } else if (gameState == STATE_PAUSED) {
                if (wParam == 'H' || wParam == VK_F1) { previousState = STATE_PAUSED; gameState = STATE_HELP; }
                else if (wParam == VK_UP || wParam == 'W') menuIndex = (menuIndex - 1 + 5) % 5;
                else if (wParam == VK_DOWN || wParam == 'S') menuIndex = (menuIndex + 1) % 5;
                else if (wParam == VK_RETURN || wParam == VK_SPACE) {
                    if (menuIndex == 0) gameState = STATE_PLAYING;
                    else if (menuIndex == 1) { previousState = STATE_PAUSED; gameState = STATE_HELP; }
                    else if (menuIndex == 2) SaveGameState();
                    else if (menuIndex == 3) LoadGameState();
                    else if (menuIndex == 4) gameState = STATE_MENU;
                } else if (wParam == 'P' || wParam == VK_ESCAPE) gameState = STATE_PLAYING;
            } else if (gameState == STATE_LEADERBOARD || gameState == STATE_GAMEOVER || gameState == STATE_VICTORY) {
                if (wParam == VK_RETURN || wParam == VK_SPACE || wParam == VK_ESCAPE) gameState = STATE_MENU;
                if ((gameState == STATE_GAMEOVER || gameState == STATE_VICTORY) && wParam == 'E') { ExportStatsCSV(); }
                if ((gameState == STATE_GAMEOVER || gameState == STATE_VICTORY) && wParam == 'J') { ExportStatsJSON(); }
            }
            break;

        case WM_LBUTTONDOWN: {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            if (gameState == STATE_MENU) {
                if (my >= H - 48 && my <= H - 15) {
                    previousState = STATE_MENU;
                    gameState = STATE_HELP;
                } else {
                    int saved = HasSavedGame();
                    int count = saved ? 6 : 5;
                    for (int i = 0; i < count; i++) {
                        int y = 175 + i * 32;
                        if (my >= y - 12 && my <= y + 18) {
                            menuIndex = i;
                            if (menuIndex == 0) StartNewGame(MODE_CLASSIC);
                            else if (saved && menuIndex == 1) LoadGameState();
                            else if ((!saved && menuIndex == 1) || (saved && menuIndex == 2)) gameState = STATE_LEADERBOARD;
                            else if ((!saved && menuIndex == 2) || (saved && menuIndex == 3)) gameState = STATE_MODE_SELECT;
                            else if ((!saved && menuIndex == 3) || (saved && menuIndex == 4)) { previousState = STATE_MENU; gameState = STATE_HELP; }
                            else if ((!saved && menuIndex == 4) || (saved && menuIndex == 5)) { ExportHighScoresJSON(); }
                            break;
                        }
                    }
                }
            } else if (gameState == STATE_MODE_SELECT) {
                for (int i = 0; i < 3; i++) {
                    int y = 170 + i * 50;
                    if (my >= y - 10 && my <= y + 30) {
                        modeIndex = i;
                        StartNewGame(modeIndex);
                        break;
                    }
                }
            } else if (gameState == STATE_PAUSED) {
                for (int i = 0; i < 5; i++) {
                    int y = 175 + i * 34;
                    if (my >= y - 12 && my <= y + 18) {
                        menuIndex = i;
                        if (menuIndex == 0) gameState = STATE_PLAYING;
                        else if (menuIndex == 1) { previousState = STATE_PAUSED; gameState = STATE_HELP; }
                        else if (menuIndex == 2) SaveGameState();
                        else if (menuIndex == 3) LoadGameState();
                        else if (menuIndex == 4) gameState = STATE_MENU;
                        break;
                    }
                }
            } else if (gameState == STATE_PLAYING) {
                if (mx >= W - 52 && mx <= W - 30 && my >= 4 && my <= 26) {
                    previousState = STATE_PLAYING;
                    gameState = STATE_HELP;
                } else if (mx >= W - 28 && mx <= W - 8 && my >= 4 && my <= 26) {
                    gameState = STATE_PAUSED;
                    menuIndex = 0;
                }
            } else if (gameState == STATE_HELP) {
                gameState = (previousState == STATE_PLAYING || previousState == STATE_PAUSED) ? STATE_PAUSED : STATE_MENU;
            } else if (gameState == STATE_LEADERBOARD || gameState == STATE_GAMEOVER || gameState == STATE_VICTORY) {
                gameState = STATE_MENU;
            }
            break;
        }

        case WM_TIMER:
            Update();
            InvalidateRect(hwnd, NULL, FALSE);
            break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, W, H);
            HBITMAP oldBm = (HBITMAP)SelectObject(memDC, hbm);
            
            HFONT hFont = NULL;
            HFONT oldFont = NULL;

            if (bombFlash > 0 || bossDeathFlash > 0) {
                HBRUSH fbr;
                if (bossDeathFlash > 0) {
                    int r = 255, g = 255, b = 255;
                    if (bossDeathFlash < 100) {
                        int f = bossDeathFlash % 15;
                        if (f < 5) { r = 255; g = 0; b = 100; }
                        else if (f < 10) { r = 0; g = 255; b = 100; }
                        else { r = 100; g = 0; b = 255; }
                    }
                    fbr = CreateSolidBrush(RGB(r, g, b));
                } else {
                    fbr = CreateSolidBrush(RGB(255, 255, 255));
                }
                RECT rc = {0, 0, W, H};
                FillRect(memDC, &rc, fbr);
                DeleteObject(fbr);
            } else {
                int bgR = bombardmentActive ? (65 + (frameCount % 20)) : (5 + (wave * 4) % 25);
                int bgG = bombardmentActive ? (25 + (frameCount % 10)) : (5 + (wave * 6) % 25);
                int bgB = bombardmentActive ? 12 : (18 + (wave * 10) % 40);
                HBRUSH bg = CreateSolidBrush(RGB(bgR, bgG, bgB));
                RECT rc = {0, 0, W, H};
                FillRect(memDC, &rc, bg);
                DeleteObject(bg);

                // Deep Space Nebulae Rendering (Loop 2)
                for (int n = 0; n < 3; n++) {
                    nebulae[n].x += nebulae[n].vx;
                    nebulae[n].y += nebulae[n].vy;
                    if (nebulae[n].x < -nebulae[n].r) nebulae[n].x = W + nebulae[n].r;
                    if (nebulae[n].x > W + nebulae[n].r) nebulae[n].x = -nebulae[n].r;
                    if (nebulae[n].y < -nebulae[n].r) nebulae[n].y = H + nebulae[n].r;
                    if (nebulae[n].y > H + nebulae[n].r) nebulae[n].y = -nebulae[n].r;

                    HBRUSH nbr = CreateSolidBrush(nebulae[n].col1);
                    HBRUSH oldBr = (HBRUSH)SelectObject(memDC, nbr);
                    int nr = (int)(nebulae[n].r);
                    Ellipse(memDC, (int)nebulae[n].x - nr, (int)nebulae[n].y - nr, (int)nebulae[n].x + nr, (int)nebulae[n].y + nr);
                    SelectObject(memDC, oldBr); DeleteObject(nbr);
                }

                // Planets (Loop 3)
                for (int p = 0; p < 2; p++) {
                    planets[p].x += planets[p].vx; planets[p].y += planets[p].vy;
                    if (planets[p].x < -planets[p].r*2) planets[p].x = W + planets[p].r*2;
                    if (planets[p].x > W + planets[p].r*2) planets[p].x = -planets[p].r*2;
                    if (planets[p].y < -planets[p].r*2) planets[p].y = H + planets[p].r*2;
                    if (planets[p].y > H + planets[p].r*2) planets[p].y = -planets[p].r*2;

                    HBRUSH pbr = CreateSolidBrush(planets[p].col);
                    HBRUSH oldBr = (HBRUSH)SelectObject(memDC, pbr);
                    int px = (int)planets[p].x, py = (int)planets[p].y, pr = (int)planets[p].r;
                    Ellipse(memDC, px - pr, py - pr, px + pr, py + pr);
                    
                    if (planets[p].type == 0) { // ringed
                        HPEN rpen = CreatePen(PS_SOLID, 4, RGB(0, 255, 128));
                        HPEN oldPen = (HPEN)SelectObject(memDC, rpen);
                        SelectObject(memDC, GetStockObject(NULL_BRUSH));
                        Ellipse(memDC, px - (int)(pr * 1.8f), py - (int)(pr * 0.4f), px + (int)(pr * 1.8f), py + (int)(pr * 0.4f));
                        SelectObject(memDC, oldPen); DeleteObject(rpen);
                    } else { // cratered
                        HBRUSH cbr = CreateSolidBrush(RGB(100, 10, 10));
                        SelectObject(memDC, cbr);
                        Ellipse(memDC, px - (int)(pr*0.3f) - (int)(pr*0.15f), py - (int)(pr*0.2f) - (int)(pr*0.15f), px - (int)(pr*0.3f) + (int)(pr*0.15f), py - (int)(pr*0.2f) + (int)(pr*0.15f));
                        Ellipse(memDC, px + (int)(pr*0.4f) - (int)(pr*0.2f), py + (int)(pr*0.3f) - (int)(pr*0.2f), px + (int)(pr*0.4f) + (int)(pr*0.2f), py + (int)(pr*0.3f) + (int)(pr*0.2f));
                        SelectObject(memDC, oldBr); DeleteObject(cbr);
                    }
                    SelectObject(memDC, oldBr); DeleteObject(pbr);
                }

                // Planetary Surface Terrain during Bombardment
                if (bombardmentActive) {
                    HBRUSH surfBr = CreateSolidBrush(RGB(170, 45, 15));
                    RECT sRc = {0, H - 35, W, H}; FillRect(memDC, &sRc, surfBr); DeleteObject(surfBr);
                    HPEN lavaPen = CreatePen(PS_SOLID, 2, RGB(255, 234, 0));
                    HPEN oldP = (HPEN)SelectObject(memDC, lavaPen);
                    MoveToEx(memDC, 0, H - 35, NULL); LineTo(memDC, W, H - 35);
                    SelectObject(memDC, oldP); DeleteObject(lavaPen);
                }

                // Comets (Loop 3)
                if (rnd() % 200 == 0) {
                    for(int i=0; i<5; i++){
                        if (comets[i].life <= 0) {
                            comets[i].x = (float)(rnd() % W); comets[i].y = -20.0f;
                            comets[i].vx = ((float)(rnd() % 20) - 10.0f) * 0.2f;
                            comets[i].vy = 5.0f + (float)(rnd() % 50) * 0.1f;
                            comets[i].life = 1.0f;
                            break;
                        }
                    }
                }
                for (int i = 0; i < 5; i++) {
                    if (comets[i].life > 0) {
                        comets[i].x += comets[i].vx; comets[i].y += comets[i].vy; comets[i].life -= 0.01f;
                        if (comets[i].y > H + 20) comets[i].life = 0;
                        else {
                            HPEN cpen = CreatePen(PS_SOLID, 2, RGB(0, 229, 255));
                            HPEN oldPen = (HPEN)SelectObject(memDC, cpen);
                            MoveToEx(memDC, (int)comets[i].x, (int)comets[i].y, NULL);
                            LineTo(memDC, (int)(comets[i].x - comets[i].vx * 10), (int)(comets[i].y - comets[i].vy * 10));
                            SelectObject(memDC, oldPen); DeleteObject(cpen);
                            HBRUSH cbr = CreateSolidBrush(RGB(255, 255, 255));
                            HBRUSH oldBr = (HBRUSH)SelectObject(memDC, cbr);
                            Ellipse(memDC, (int)comets[i].x - 1, (int)comets[i].y - 1, (int)comets[i].x + 2, (int)comets[i].y + 2);
                            SelectObject(memDC, oldBr); DeleteObject(cbr);
                        }
                    }
                }

                // Orbital Bombardment Strikes Rendering
                for (int k = 0; k < MAX_STRIKES; k++) {
                    if (strikes[k].active) {
                        int sx = (int)strikes[k].x;
                        if (strikes[k].timer > 15) {
                            HPEN tPen = CreatePen(PS_SOLID, 1, (frameCount % 4 < 2) ? RGB(255, 23, 68) : RGB(255, 234, 0));
                            HPEN oldP = (HPEN)SelectObject(memDC, tPen);
                            MoveToEx(memDC, sx, 0, NULL); LineTo(memDC, sx, H);
                            MoveToEx(memDC, sx - 10, H - 40, NULL); LineTo(memDC, sx + 10, H - 40);
                            MoveToEx(memDC, sx, H - 50, NULL); LineTo(memDC, sx, H - 30);
                            SelectObject(memDC, oldP); DeleteObject(tPen);
                        } else {
                            HBRUSH beamBr = CreateSolidBrush((frameCount % 2 == 0) ? RGB(255, 60, 0) : RGB(255, 234, 0));
                            RECT bRc = {sx - 12, 0, sx + 12, H};
                            FillRect(memDC, &bRc, beamBr);
                            DeleteObject(beamBr);
                            HBRUSH coreBr = CreateSolidBrush(RGB(255, 255, 255));
                            RECT cRc = {sx - 4, 0, sx + 4, H};
                            FillRect(memDC, &cRc, coreBr);
                            DeleteObject(coreBr);
                        }
                    }
                }

                if (escort.active) {
                    HBRUSH br = CreateSolidBrush(RGB(0, 255, 128));
                    HGDIOBJ oldBr = SelectObject(memDC, br);
                    RECT r = {(int)escort.x, (int)escort.y, (int)escort.x + 30, (int)escort.y + 30};
                    FillRect(memDC, &r, br);
                    HBRUSH hpBr = CreateSolidBrush(RGB(0, 255, 0));
                    SelectObject(memDC, hpBr);
                    int hpW = (int)(30 * ((float)escort.hp / escort.maxHp));
                    RECT rHp = {(int)escort.x, (int)escort.y - 6, (int)escort.x + hpW, (int)escort.y - 2};
                    FillRect(memDC, &rHp, hpBr);
                    SelectObject(memDC, oldBr); DeleteObject(br); DeleteObject(hpBr);
                }

                if (pathGatesActive) {
                    HPEN lpen = CreatePen(PS_SOLID, 3, RGB(255, 23, 68));
                    HPEN rpen = CreatePen(PS_SOLID, 3, RGB(0, 229, 255));
                    HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
                    HGDIOBJ oldPen = SelectObject(memDC, lpen);
                    HGDIOBJ oldBr = SelectObject(memDC, nullBr);
                    Ellipse(memDC, W/4 - 20, (int)pathGatesY - 20, W/4 + 20, (int)pathGatesY + 20);
                    SelectObject(memDC, rpen);
                    Ellipse(memDC, 3*W/4 - 20, (int)pathGatesY - 20, 3*W/4 + 20, (int)pathGatesY + 20);
                    SelectObject(memDC, oldPen); SelectObject(memDC, oldBr);
                    DeleteObject(lpen); DeleteObject(rpen);
                }

                // 3-Layer Parallax Starfield Rendering (Loop 2)
                for (int i = 0; i < MAX_STARS; i++) {
                    if (stars[i].layer == 2) {
                        HPEN spen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                        HPEN oldPen = (HPEN)SelectObject(memDC, spen);
                        MoveToEx(memDC, (int)stars[i].x, (int)stars[i].y, NULL);
                        LineTo(memDC, (int)stars[i].x, (int)stars[i].y + 4);
                        SelectObject(memDC, oldPen); DeleteObject(spen);
                    } else if (stars[i].layer == 1) {
                        HBRUSH sbr = CreateSolidBrush(RGB(180, 230, 255));
                        RECT sr = {(int)stars[i].x, (int)stars[i].y, (int)stars[i].x + 1, (int)stars[i].y + 1};
                        FillRect(memDC, &sr, sbr);
                        DeleteObject(sbr);
                    } else {
                        HBRUSH sbr = CreateSolidBrush(timeStopTimer > 0 ? RGB(0, 229, 255) : RGB(100, 130, 180));
                        RECT sr = {(int)stars[i].x, (int)stars[i].y, (int)stars[i].x + 1, (int)stars[i].y + 1};
                        FillRect(memDC, &sr, sbr);
                        DeleteObject(sbr);
                    }
                }

                SetBkMode(memDC, TRANSPARENT);
                oldFont = (HFONT)SelectObject(memDC, hFontMenu);

                if (gameState == STATE_MENU) {
                    SelectObject(memDC, hFontTitle);
                    SetTextColor(memDC, RGB(0, 229, 255));
                    TextOutA(memDC, W/2 - 40, 65, "KSPACE", 6);
                    SelectObject(memDC, hFontHUD);
                    SetTextColor(memDC, RGB(128, 216, 255));
                    TextOutA(memDC, W/2 - 70, 92, "Loop 11 Space Command", 21);

                    int saved = HasSavedGame();
                    char* opts[] = {"START NEW GAME", "RESUME SAVED GAME", "HIGH SCORES", "SELECT GAME MODE", "HOW TO PLAY", "EXPORT SCORES"};
                    int count = saved ? 6 : 5;
                    int optIdxs[] = {0, 1, 2, 3, 4, 5};
                    if (!saved) { optIdxs[1] = 2; optIdxs[2] = 3; optIdxs[3] = 4; optIdxs[4] = 5; }

                    SelectObject(memDC, hFontMenu);
                    for (int i = 0; i < count; i++) {
                        int y = 160 + i * 32;
                        if (i == menuIndex) {
                            SetTextColor(memDC, RGB(255, 234, 0));
                            char buf[64]; wsprintfA(buf, "> %s <", opts[optIdxs[i]]);
                            TextOutA(memDC, W/2 - lstrlenA(buf)*4, y, buf, lstrlenA(buf));
                        } else {
                            SetTextColor(memDC, RGB(255, 255, 255));
                            TextOutA(memDC, W/2 - lstrlenA(opts[optIdxs[i]])*4, y, opts[optIdxs[i]], lstrlenA(opts[optIdxs[i]]));
                        }
                    }

                    // Help button badge at bottom
                    HBRUSH hlpBg = CreateSolidBrush(RGB(10, 30, 60));
                    RECT hlpRc = {W/2 - 115, H - 46, W/2 + 115, H - 20};
                    FillRect(memDC, &hlpRc, hlpBg);
                    DeleteObject(hlpBg);
                    HPEN hlpPen = CreatePen(PS_SOLID, 1, RGB(0, 229, 255));
                    HGDIOBJ oldP = SelectObject(memDC, hlpPen);
                    HGDIOBJ nullB = SelectObject(memDC, GetStockObject(NULL_BRUSH));
                    Rectangle(memDC, hlpRc.left, hlpRc.top, hlpRc.right, hlpRc.bottom);
                    SelectObject(memDC, oldP); SelectObject(memDC, nullB);
                    DeleteObject(hlpPen);

                    SetTextColor(memDC, RGB(255, 234, 0));
                    SelectObject(memDC, hFontHUD);
                    TextOutA(memDC, W/2 - 80, H - 36, "PRESS [H] OR [F1] FOR HELP", 26);
                } else if (gameState == STATE_MODE_SELECT) {
                    SelectObject(memDC, hFontTitle);
                    SetTextColor(memDC, RGB(255, 234, 0));
                    TextOutA(memDC, W/2 - 75, 75, "SELECT GAME MODE", 16);
                    SelectObject(memDC, hFontMenu);
                    char* modes[] = {"Classic 20-Wave Campaign", "Endurance Wave", "Boss Rush"};
                    for (int i = 0; i < 3; i++) {
                        int y = 170 + i * 50;
                        if (i == modeIndex) {
                            SetTextColor(memDC, RGB(0, 229, 255));
                            char buf[64]; wsprintfA(buf, "> %s <", modes[i]);
                            TextOutA(memDC, W/2 - lstrlenA(buf)*4, y, buf, lstrlenA(buf));
                        } else {
                            SetTextColor(memDC, RGB(120, 144, 156));
                            TextOutA(memDC, W/2 - lstrlenA(modes[i])*4, y, modes[i], lstrlenA(modes[i]));
                        }
                    }
                    SelectObject(memDC, hFontHUD);
                    SetTextColor(memDC, RGB(255, 23, 68));
                    TextOutA(memDC, W/2 - 75, H - 40, "Press ENTER to Launch", 21);
                } else if (gameState == STATE_HELP) {
                    SelectObject(memDC, hFontTitle);
                    SetTextColor(memDC, RGB(0, 229, 255));
                    TextOutA(memDC, W/2 - 50, 24, "HOW TO PLAY", 11);
                    SelectObject(memDC, hFontHUD);
                    SetTextColor(memDC, RGB(255, 255, 255));
                    char* lines[] = {
                        "ARROWS : Move Ship",
                        "SPACE  : Fire Weapon",
                        "P      : Pause Game",
                        "",
                        "--- LOOP 11 SYSTEMS ---",
                        "J : Hyper-Jump Warp Drive",
                        "W : Deploy Drone Wing",
                        "O : Overcharge Hyper-Mode",
                        "T : Time Stop Chrono-Field",
                        "D : Tactical Dash Burst",
                        "B : Smart Bomb Subsystems",
                        "S : Hyper Shield Barricade",
                        "",
                        "--- POWERUP PODS ---",
                        "S:Spread L:Laser H:Shield",
                        "B:Bomb R:Rapid T:Time",
                        "W:Drone Pod O:Overcharge Core"
                    };
                    for (int i = 0; i < 17; i++) {
                        TextOutA(memDC, 20, 50 + i * 16, lines[i], lstrlenA(lines[i]));
                    }
                    SetTextColor(memDC, RGB(255, 234, 0));
                    TextOutA(memDC, W/2 - 90, H - 28, "Press [H] or ENTER to return", 28);
                } else if (gameState == STATE_LEADERBOARD) {
                    SelectObject(memDC, hFontTitle);
                    SetTextColor(memDC, RGB(0, 229, 255));
                    TextOutA(memDC, W/2 - 65, 60, "TOP COMMANDERS", 14);
                    SelectObject(memDC, hFontHUD);
                    for (int i = 0; i < MAX_LEADERBOARD; i++) {
                        char buf[64];
                        wsprintfA(buf, "#%d   Score:%d  Wave:%d", i + 1, leaderboard[i].score, leaderboard[i].wave);
                        SetTextColor(memDC, i == 0 ? RGB(255, 234, 0) : RGB(255, 255, 255));
                        TextOutA(memDC, 40, 120 + i * 35, buf, lstrlenA(buf));
                    }
                    SetTextColor(memDC, RGB(128, 216, 255));
                    TextOutA(memDC, W/2 - 75, H - 40, "Press ENTER to return", 21);
                } else if (gameState == STATE_PLAYING || gameState == STATE_PAUSED || gameState == STATE_GAMEOVER || gameState == STATE_VICTORY) {
                    DrawDroneWingsGDI(memDC, frameCount);
                    DrawPlayerShipGDI(memDC, (int)p.x, (int)p.y, shieldActive, frameCount);
                    DrawHyperJumpEffectGDI(memDC, frameCount);

                    HBRUSH bbr = CreateSolidBrush(overchargeTimer > 0 ? RGB(255, 234, 0) : RGB(0, 229, 255));
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (b[i].active) {
                            if (b[i].type == 2.0f) {
                                HBRUSH mbr = CreateSolidBrush(RGB(255, 60, 0));
                                RECT br = {(int)b[i].x, (int)b[i].y, (int)b[i].x + 5, (int)b[i].y + 7};
                                FillRect(memDC, &br, mbr);
                                DeleteObject(mbr);
                            } else {
                                RECT br = {(int)b[i].x, (int)b[i].y, (int)b[i].x + 4, (int)b[i].y + 10};
                                FillRect(memDC, &br, bbr);
                            }
                        }
                    }
                    DeleteObject(bbr);

                    HBRUSH ebbr = CreateSolidBrush(RGB(255, 23, 68));
                    for (int i = 0; i < MAX_EBULLETS; i++) {
                        if (eb[i].active) {
                            RECT br = {(int)eb[i].x, (int)eb[i].y, (int)eb[i].x + 4, (int)eb[i].y + 10};
                            FillRect(memDC, &br, ebbr);
                        }
                    }
                    DeleteObject(ebbr);

                    for (int i = 0; i < MAX_ENEMIES; i++) {
                        if (e[i].active) DrawEnemyShipGDI(memDC, e[i].x, e[i].y, e[i].type, e[i].cloaked, e[i].hp, e[i].maxHp, frameCount);
                    }

                    if (bossActive) DrawBossGDI(memDC, bossX, bossY, frameCount);
                    for (int i = 0; i < MAX_POWERUPS; i++) {
                        if (pu[i].active) DrawPowerupGDI(memDC, pu[i].x, pu[i].y, pu[i].type, frameCount);
                    }

                    DrawParticles(memDC);

                    // Render Visual Effects (Loop 2)
                    // Debris Chunks
                    for (int i = 0; i < MAX_DEBRIS; i++) {
                        if (debris[i].life > 0.0f) {
                            debris[i].x += debris[i].vx; debris[i].y += debris[i].vy;
                            debris[i].life -= debris[i].decay;
                            HBRUSH dbr = CreateSolidBrush(debris[i].color);
                            int sz = (int)debris[i].size;
                            RECT dr = {(int)debris[i].x - sz/2, (int)debris[i].y - sz/2, (int)debris[i].x + sz/2, (int)debris[i].y + sz/2};
                            
                            if (debris[i].shape == 1) {
                                HGDIOBJ oldBr = SelectObject(memDC, dbr);
                                HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
                                HGDIOBJ oldP = SelectObject(memDC, nullPen);
                                POINT pts[3] = { {dr.left, dr.top}, {dr.right, dr.top}, {dr.left + sz/2, dr.bottom} };
                                Polygon(memDC, pts, 3);
                                SelectObject(memDC, oldP);
                                SelectObject(memDC, oldBr);
                            } else if (debris[i].shape == 2) {
                                HGDIOBJ oldBr = SelectObject(memDC, dbr);
                                HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
                                HGDIOBJ oldP = SelectObject(memDC, nullPen);
                                Ellipse(memDC, dr.left, dr.top, dr.right, dr.bottom);
                                SelectObject(memDC, oldP);
                                SelectObject(memDC, oldBr);
                            } else {
                                FillRect(memDC, &dr, dbr);
                            }
                            
                            DeleteObject(dbr);
                        }
                    }

                    // Shockwaves & Ripples
                    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
                        if (shockwaves[i].life > 0) {
                            shockwaves[i].r += shockwaves[i].speed;
                            if (shockwaves[i].r >= shockwaves[i].maxR) shockwaves[i].life = 0;
                            else {
                                HPEN swPen = CreatePen(PS_SOLID, shockwaves[i].isOuter ? 1 : 2, shockwaves[i].color);
                                HPEN oldP = (HPEN)SelectObject(memDC, swPen);
                                HBRUSH oldB = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));
                                int r = (int)shockwaves[i].r;
                                Ellipse(memDC, (int)shockwaves[i].x - r, (int)shockwaves[i].y - r, (int)shockwaves[i].x + r, (int)shockwaves[i].y + r);
                                SelectObject(memDC, oldP); SelectObject(memDC, oldB); DeleteObject(swPen);
                            }
                        }
                    }

                    for (int i = 0; i < MAX_RIPPLES; i++) {
                        if (ripples[i].life > 0) {
                            ripples[i].r += 1.8f;
                            if (ripples[i].r >= ripples[i].maxR) ripples[i].life = 0;
                            else {
                                HPEN rPen = CreatePen(PS_SOLID, 2, ripples[i].color);
                                HPEN oldP = (HPEN)SelectObject(memDC, rPen);
                                HBRUSH oldB = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));
                                int r = (int)ripples[i].r;
                                Ellipse(memDC, (int)ripples[i].x - r, (int)ripples[i].y - r, (int)ripples[i].x + r, (int)ripples[i].y + r);
                                SelectObject(memDC, oldP); SelectObject(memDC, oldB); DeleteObject(rPen);
                            }
                        }
                    }

                    // Muzzle Flashes
                    for (int i = 0; i < MAX_FLASHES; i++) {
                        if (flashes[i].life > 0) {
                            flashes[i].life--;
                            HBRUSH mfBr = CreateSolidBrush(flashes[i].color);
                            int sz = (int)(flashes[i].size * flashes[i].life / flashes[i].maxLife);
                            RECT mfr = {(int)flashes[i].x - sz, (int)flashes[i].y - sz, (int)flashes[i].x + sz, (int)flashes[i].y + sz};
                            FillRect(memDC, &mfr, mfBr);
                            DeleteObject(mfBr);
                        }
                    }

                    // Top Banner Alerts (Elite Squad & Bombardment)
                    if (eliteSquadTimer > 0) {
                        HBRUSH banBr = CreateSolidBrush(RGB(180, 20, 20));
                        RECT bRc = {15, 38, W - 15, 58};
                        FillRect(memDC, &bRc, banBr); DeleteObject(banBr);
                        SetTextColor(memDC, (frameCount % 4 < 2) ? RGB(255, 234, 0) : RGB(255, 255, 255));
                        char eStr[64]; wsprintfA(eStr, "! ELITE SQUAD: %s !", eliteSquadName);
                        TextOutA(memDC, W/2 - lstrlenA(eStr)*4, 41, eStr, lstrlenA(eStr));
                    } else if (bombardmentBanner > 0 || bombardmentActive) {
                        HBRUSH banBr = CreateSolidBrush(RGB(180, 60, 0));
                        RECT bRc = {15, 38, W - 15, 58};
                        FillRect(memDC, &bRc, banBr); DeleteObject(banBr);
                        SetTextColor(memDC, (frameCount % 4 < 2) ? RGB(255, 234, 0) : RGB(255, 255, 255));
                        char bStr[] = "! ORBITAL BOMBARDMENT ACTIVE !";
                        TextOutA(memDC, W/2 - lstrlenA(bStr)*4, 41, bStr, lstrlenA(bStr));
                    }

                    // Pause [||] and Help [?] indicators at top right
                    HBRUSH pBg = CreateSolidBrush(RGB(10, 25, 50));
                    RECT rHlp = {W - 48, 4, W - 28, 20};
                    RECT rPau = {W - 24, 4, W - 4, 20};
                    FillRect(memDC, &rHlp, pBg);
                    FillRect(memDC, &rPau, pBg);
                    DeleteObject(pBg);
                    HPEN boxPen = CreatePen(PS_SOLID, 1, RGB(0, 229, 255));
                    HGDIOBJ oldBoxP = SelectObject(memDC, boxPen);
                    HGDIOBJ nullBr = SelectObject(memDC, GetStockObject(NULL_BRUSH));
                    Rectangle(memDC, rHlp.left, rHlp.top, rHlp.right, rHlp.bottom);
                    Rectangle(memDC, rPau.left, rPau.top, rPau.right, rPau.bottom);
                    SelectObject(memDC, oldBoxP); SelectObject(memDC, nullBr);
                    DeleteObject(boxPen);

                    SelectObject(memDC, hFontHUD);
                    SetTextColor(memDC, RGB(0, 229, 255));
                    TextOutA(memDC, W - 40, 5, "?", 1);
                    TextOutA(memDC, W - 17, 5, "||", 2);

                    // HUD
                    SetTextColor(memDC, RGB(255, 255, 255));
                    char hudStr[64];
                    wsprintfA(hudStr, "SCORE: %d  HIGH: %d  W:%d/20", score, highScore, wave);
                    TextOutA(memDC, 10, 6, hudStr, lstrlenA(hudStr));

                    char statStr[64];
                    wsprintfA(statStr, "HP: %d  B:[B]%d  Drones:[W]%d/2", p.hp, bombCount, droneCount);
                    TextOutA(memDC, 10, 22, statStr, lstrlenA(statStr));

                    // Overcharge & Hyper-Jump Status
                    char ocStr[96];
                    if (hyperJumpTimer > 0) {
                        wsprintfA(ocStr, "[J]HYPER-JUMP WARP ACTIVE!");
                        SetTextColor(memDC, (frameCount % 4 < 2) ? RGB(255, 234, 0) : RGB(0, 229, 255));
                    } else if (overchargeTimer > 0) {
                        wsprintfA(ocStr, "[O]OVERCHARGE: (%ds)  [J]Jump:%d%%", overchargeTimer / 60 + 1, hyperJumpEnergy);
                        SetTextColor(memDC, (frameCount % 4 < 2) ? RGB(255, 234, 0) : RGB(0, 229, 255));
                    } else {
                        wsprintfA(ocStr, "[O]OC:%d%% %s  [J]Jump:%d%%%s",
                            overchargeEnergy, overchargeEnergy >= 100 ? "[RDY]" : "",
                            hyperJumpEnergy, hyperJumpEnergy >= 100 ? "[RDY]" : "");
                        SetTextColor(memDC, (overchargeEnergy >= 100 || hyperJumpEnergy >= 100) ? RGB(255, 234, 0) : RGB(0, 230, 118));
                    }
                    TextOutA(memDC, 10, H - 36, ocStr, lstrlenA(ocStr));

                    // Skill Badges Status
                    char skillStr[96];
                    wsprintfA(skillStr, "[T]Stop:%s  [D]Dash:%s  [S]Shield:%s",
                        timeStopTimer > 0 ? "ACT" : (timeStopCooldown <= 0 ? "RDY" : "CD"),
                        invincibleTimer > 0 ? "ACT" : (dashCooldown <= 0 ? "RDY" : "CD"),
                        hyperShieldTimer > 0 ? "ACT" : (hyperShieldCooldown <= 0 ? "RDY" : "CD"));
                    SetTextColor(memDC, RGB(0, 230, 118));
                    TextOutA(memDC, 10, H - 20, skillStr, lstrlenA(skillStr));

                    if (bossActive) {
                        HBRUSH barBg = CreateSolidBrush(RGB(30, 30, 30));
                        RECT rbg = {40, 42, W - 40, 52};
                        FillRect(memDC, &rbg, barBg);
                        DeleteObject(barBg);

                        HBRUSH barFg = CreateSolidBrush(bossIsDreadnought ? RGB(255, 23, 68) : (bossIsMothership ? RGB(213, 0, 249) : RGB(255, 145, 0)));
                        int wFill = (int)((W - 80) * ((float)bossHp / bossMaxHp));
                        if (wFill < 0) wFill = 0;
                        RECT rfg = {40, 42, 40 + wFill, 52};
                        FillRect(memDC, &rfg, barFg);
                        DeleteObject(barFg);

                        SetTextColor(memDC, RGB(255, 255, 255));
                        if (bossIsDreadnought) {
                            char dStr[64];
                            wsprintfA(dStr, "DREADNOUGHT SIEGE [L-GEN:%s R-GEN:%s]", (dreadGenL > 0 ? "ON" : "OFF"), (dreadGenR > 0 ? "ON" : "OFF"));
                            TextOutA(memDC, W/2 - lstrlenA(dStr)*4, 40, dStr, lstrlenA(dStr));
                        } else {
                            TextOutA(memDC, W/2 - 60, 40, bossIsMothership ? "ALIEN MOTHERSHIP" : "FLAGSHIP COMMANDER", bossIsMothership ? 16 : 18);
                        }
                    }

                    // Cybernetic Sci-Fi HUD Frame & Pulsating Perimeter Inlay
                    DrawSciFiHUDFrame(memDC, frameCount);

                    if (gameState == STATE_PAUSED) {
                        SelectObject(memDC, hFontTitle);
                        SetTextColor(memDC, RGB(255, 234, 0));
                        TextOutA(memDC, W/2 - 60, 100, "SYSTEM PAUSED", 13);
                        SelectObject(memDC, hFontMenu);
                        char* opts[] = {"RESUME GAME", "HOW TO PLAY", "SAVE GAME STATE", "LOAD GAME STATE", "QUIT TO MENU"};
                        for (int i = 0; i < 5; i++) {
                            int y = 160 + i * 34;
                            if (i == menuIndex) {
                                SetTextColor(memDC, RGB(0, 229, 255));
                                char buf[64]; wsprintfA(buf, "> %s <", opts[i]);
                                TextOutA(memDC, W/2 - lstrlenA(buf)*4, y, buf, lstrlenA(buf));
                            } else {
                                SetTextColor(memDC, RGB(255, 255, 255));
                                TextOutA(memDC, W/2 - lstrlenA(opts[i])*4, y, opts[i], lstrlenA(opts[i]));
                            }
                        }
                        SelectObject(memDC, hFontHUD);
                        SetTextColor(memDC, RGB(128, 216, 255));
                        TextOutA(memDC, W/2 - 95, H - 35, "Press [P] to Resume | [H] for Help", 34);
                    } else if (gameState == STATE_GAMEOVER) {
                        SelectObject(memDC, hFontTitle);
                        SetTextColor(memDC, RGB(255, 23, 68));
                        TextOutA(memDC, W/2 - 65, H/2 - 40, "MISSION FAILED", 14);
                        SelectObject(memDC, hFontHUD);
                        SetTextColor(memDC, RGB(255, 255, 255));
                        char finalStr[64];
                        wsprintfA(finalStr, "FINAL SCORE: %d", score);
                        TextOutA(memDC, W/2 - lstrlenA(finalStr)*3, H/2 - 10, finalStr, lstrlenA(finalStr));
                        TextOutA(memDC, W/2 - 80, H/2 + 25, "ENTER:Menu E:CSV J:JSON", 23);
                    } else if (gameState == STATE_VICTORY) {
                        SelectObject(memDC, hFontTitle);
                        SetTextColor(memDC, RGB(0, 230, 118));
                        TextOutA(memDC, W/2 - 75, H/2 - 45, "CAMPAIGN VICTORY!", 17);
                        SelectObject(memDC, hFontHUD);
                        SetTextColor(memDC, RGB(255, 234, 0));
                        TextOutA(memDC, W/2 - 80, H/2 - 20, "Alien Mothership Destroyed", 26);
                        SetTextColor(memDC, RGB(255, 255, 255));
                        char finalStr[64];
                        wsprintfA(finalStr, "FINAL SCORE: %d", score);
                        TextOutA(memDC, W/2 - lstrlenA(finalStr)*3, H/2 + 5, finalStr, lstrlenA(finalStr));
                        TextOutA(memDC, W/2 - 80, H/2 + 35, "ENTER:Menu E:CSV J:JSON", 23);
                    }
                }
            }

            if (oldFont) {
                SelectObject(memDC, oldFont);
            }
            int shakeOffsetX = 0;
            int shakeOffsetY = 0;
            if (screenShake > 0) {
                shakeOffsetX = (screenShake * FastSin(frameCount * 2)) / 250;
                shakeOffsetY = (screenShake * FastCos(frameCount * 2)) / 250;
            }
            BitBlt(hdc, shakeOffsetX, shakeOffsetY, W, H, memDC, 0, 0, SRCCOPY);
            if (shakeOffsetX != 0 || shakeOffsetY != 0) {
                HBRUSH blk = (HBRUSH)GetStockObject(BLACK_BRUSH);
                if (shakeOffsetX > 0) { RECT r = {0, 0, shakeOffsetX, H}; FillRect(hdc, &r, blk); }
                if (shakeOffsetX < 0) { RECT r = {W + shakeOffsetX, 0, W, H}; FillRect(hdc, &r, blk); }
                if (shakeOffsetY > 0) { RECT r = {0, 0, W, shakeOffsetY}; FillRect(hdc, &r, blk); }
                if (shakeOffsetY < 0) { RECT r = {0, H + shakeOffsetY, W, H}; FillRect(hdc, &r, blk); }
            }
            SelectObject(memDC, oldBm);
            DeleteObject(hbm);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
            break;
        }

        case WM_DESTROY:
            if (hFontTitle) DeleteObject(hFontTitle);
            if (hFontMenu) DeleteObject(hFontMenu);
            if (hFontHUD) DeleteObject(hFontHUD);
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    SetProcessDPIAware();
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KSpaceApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    RECT wr = {0, 0, W, H};
    AdjustWindowRect(&wr, (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN, FALSE);
    HWND hwnd = CreateWindowEx(0, "KSpaceApp", "KSpace", (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
