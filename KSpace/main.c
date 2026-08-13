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
#define MAX_PARTICLES 80
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

int kbUp = VK_UP, kbDown = VK_DOWN, kbLeft = VK_LEFT, kbRight = VK_RIGHT;
int kbFire = VK_SPACE, kbTimeStop = 'T', kbDash = 'D', kbBomb = 'B', kbShield = 'S', kbPause = 'P';

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

typedef struct { float x, y, active, dx, dy, type; int hp, maxHp, timer, cloaked; } Ent;
typedef struct { float x, y, speed; int size, layer; } Star;
typedef struct { float x, y, vx, vy; int life, maxLife; COLORREF color; } Particle;
typedef struct { int score, wave, mode; } LeaderEntry;

typedef struct { float x, y, r, maxR, speed, alpha; COLORREF color; int life; } Shockwave;
typedef struct { float x, y, vx, vy, size, rot, vrot, life, decay; COLORREF color; } Debris;
typedef struct { float x, y, r, maxR, alpha; COLORREF color; int life; } ShieldRipple;
typedef struct { float x, y, size; int life, maxLife; COLORREF color; } MuzzleFlash;
typedef struct { float x, y, r, vx, vy, phase; COLORREF col1; } Nebula;

typedef struct {
    int score, wave, mode;
    int playerHp, shieldActive, hyperShieldTimer, hyperShieldCooldown, bombs, weaponType;
    int spreadTimer, laserTimer, rapidTimer, timeStopTimer, timeStopCooldown, dashCooldown, invincibleTimer;
    float px, py;
    int enemiesKilled;
    int bossActive, bossHp, bossMaxHp, bossLevel, bossIsMothership;
    float bossX, bossY;
    int turretHp[4], turretActive[4];
} SaveState;

// --- GLOBAL GAME DATA ---
int gameState = STATE_MENU;
int menuIndex = 0;
int modeIndex = MODE_CLASSIC;

Ent p = { W/2.0f - 10.0f, H - 60.0f, 1.0f, 0, 0, 0, 3, 3, 0, 0 };
int shieldActive = 0;
int hyperShieldTimer = 0;
int hyperShieldCooldown = 0;
int bombCount = 1;
int maxBombs = 3;
int weaponType = 0; // 0: Normal, 1: Spread, 2: Laser, 3: Plasma

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
    return 0;
}

void PlaySnd(int type) {
    HANDLE hThread = CreateThread(NULL, 0, SndThread, (LPVOID)(intptr_t)type, 0, NULL);
    if (hThread) CloseHandle(hThread);
}

// Loop 2 Visual Effects Generators
void AddShockwave(float x, float y, float maxR, COLORREF col) {
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (shockwaves[i].life <= 0) {
            shockwaves[i].x = x; shockwaves[i].y = y;
            shockwaves[i].r = 4.0f; shockwaves[i].maxR = maxR;
            shockwaves[i].speed = 2.2f; shockwaves[i].alpha = 1.0f;
            shockwaves[i].color = col; shockwaves[i].life = 1;
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
            debris[i].life = 1.0f;
            debris[i].decay = 0.02f + (float)(rnd() % 20) / 1000.0f;
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

// Particle Explosions (Loop 2 Enhanced)
void AddExplosion(float x, float y, int count, COLORREF col) {
    int added = 0;
    for (int i = 0; i < MAX_PARTICLES && added < count; i++) {
        if (particles[i].life <= 0) {
            particles[i].x = x;
            particles[i].y = y;
            particles[i].vx = (float)((int)(rnd() % 11) - 5) * 0.8f;
            particles[i].vy = (float)((int)(rnd() % 11) - 5) * 0.8f;
            particles[i].life = 14 + (rnd() % 16);
            particles[i].maxLife = particles[i].life;
            particles[i].color = col;
            added++;
        }
    }
    if (count >= 10) {
        AddShockwave(x, y, (float)count * 2.2f, col);
        AddDebrisChunk(x, y, (count / 2 > 10 ? 10 : count / 2), col);
    }
}

void UpdateParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0) {
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            particles[i].life--;
        }
    }
}

void DrawParticles(HDC hdc) {
    HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0) {
            HBRUSH pbr = CreateSolidBrush(particles[i].color);
            HBRUSH oldBr = (HBRUSH)SelectObject(hdc, pbr);
            int sz = (particles[i].life > 8) ? 3 : 2;
            RECT pr = {(int)particles[i].x, (int)particles[i].y, (int)particles[i].x + sz, (int)particles[i].y + sz};
            FillRect(hdc, &pr, pbr);
            SelectObject(hdc, oldBr);
            DeleteObject(pbr);
        }
    }
    SelectObject(hdc, oldPen);
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
    bossX = W / 2.0f - 45.0f;
    bossY = -70.0f;
    bossLevel = lvl;
    bossDx = 2.0f;
    bossAttackTimer = 0;
    bossPhase = 1;

    if (wave >= 20 || lvl >= 4) {
        bossIsMothership = 1;
        bossMaxHp = 600;
        bossHp = bossMaxHp;
        for (int i = 0; i < 4; i++) {
            turretHp[i] = 80;
            turretActive[i] = 1;
        }
    } else {
        bossIsMothership = 0;
        bossMaxHp = 140 + lvl * 90;
        bossHp = bossMaxHp;
    }
    PlaySnd(4);
}

void DestroyBoss() {
    comboTimer = 180;
    if (comboMultiplier < 10) comboMultiplier++;
    int baseScore = (bossIsMothership ? 10000 : (1200 * bossLevel));
    score += baseScore * comboMultiplier;
    enemiesKilled += 10;
    totalKills += 10;
    AddExplosion(bossX + 45.0f, bossY + 30.0f, 60, RGB(255, 23, 68));
    bossActive = 0;
    PlaySnd(3);

    // Drop Powerups
    for (int k = 0; k < MAX_POWERUPS; k++) {
        if (!pu[k].active) {
            pu[k].active = 1.0f;
            pu[k].x = bossX + 15.0f + (k % 3) * 25.0f;
            pu[k].y = bossY + 20.0f;
            pu[k].dy = 1.5f;
            pu[k].type = (float)(rnd() % 9);
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
        if (bossIsMothership) {
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

void Shoot() {
    shotsFired++;
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
    bossActive = 0;

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
}

void Update() {
    if (gameState != STATE_PLAYING) return;
    timeSurvivedFrames++;

    if (bombFlash > 0) bombFlash--;
    if (comboTimer > 0) {
        comboTimer--;
        if (comboTimer == 0) comboMultiplier = 1;
    }

    // Mode Progression (20 Waves)
    if (modeIndex == MODE_CLASSIC) {
        int targetWave = 1 + (score / 650);
        if (targetWave > 20) targetWave = 20;
        if (targetWave > wave) {
            wave = targetWave;
            PlaySnd(3);
            if (wave == 5 && !bossActive) SpawnBoss(1);
            else if (wave == 10 && !bossActive) SpawnBoss(2);
            else if (wave == 15 && !bossActive) SpawnBoss(3);
            else if (wave == 20 && !bossActive) SpawnBoss(4); // Stage 20 Alien Mothership Boss
            else if (wave % 4 == 0) SpawnFormation(rnd() % 3 == 0 ? 9 : (rnd() % 2 == 0 ? 7 : 8));
        }
    } else if (modeIndex == MODE_ENDURANCE) {
        wave = 1 + (score / 600);
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
        int fireRate = (rapidTimer > 0) ? 3 : 7;
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

    // Bullets Movement
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (b[i].active) {
            b[i].y += b[i].dy;
            b[i].x += b[i].dx;
            if (b[i].y < -10 || b[i].x < -10 || b[i].x > W + 10) b[i].active = 0.0f;
        }
    }

    // Boss & Alien Mothership Logic
    if (bossActive) {
        if (bossY < 45.0f) bossY += 1.0f;
        else {
            bossX += bossDx;
            if (bossX < 10.0f || bossX > W - (bossIsMothership ? 95.0f : 70.0f)) bossDx = -bossDx;
        }

        bossAttackTimer++;
        if (timeStopTimer == 0 && bossAttackTimer % (bossIsMothership ? 35 : 45) == 0) {
            if (bossIsMothership) {
                // Turret Firing Phase
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
                    // Core Burst Ring
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

        // Bullets & Laser vs Boss
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (b[i].active) {
                if (bossIsMothership) {
                    // Turrets check
                    int hitTurret = 0;
                    float tOffsetsX[] = {5, 25, 65, 85};
                    for (int tIdx = 0; tIdx < 4; tIdx++) {
                        if (turretActive[tIdx]) {
                            float tx = bossX + tOffsetsX[tIdx];
                            float ty = bossY + 25.0f;
                            if (b[i].x >= tx - 6 && b[i].x <= tx + 18 && b[i].y >= ty - 6 && b[i].y <= ty + 18) {
                                b[i].active = 0.0f;
                                turretHp[tIdx] -= (b[i].type == 1.0f) ? 12 : 3;
                                AddExplosion(b[i].x, b[i].y, 4, RGB(255, 145, 0));
                                if (turretHp[tIdx] <= 0) {
                                    turretActive[tIdx] = 0;
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
                            AddExplosion(b[i].x, b[i].y, 3, RGB(0, 229, 255)); // Deflector absorbed!
                        } else {
                            shotsHit++; bossHp -= (b[i].type == 1.0f) ? 8 : 2;
                            AddExplosion(b[i].x, b[i].y, 4, RGB(255, 23, 68));
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
                        shotsHit++; bossHp -= (b[i].type == 1.0f) ? 8 : 2;
                        AddExplosion(b[i].x, b[i].y, 3, RGB(0, 229, 255));
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

    // Spawning regular enemies & formations
    int spawnRate = 30 - (score / 200) - (modeIndex == MODE_ENDURANCE ? 8 : 0);
    if (spawnRate < 8) spawnRate = 8;
    if (frameCount % spawnRate == 0) SpawnEnemy();

    // Regular Enemies Update
    float baseEnemySpeed = 1.8f + (score / 300.0f) + (modeIndex == MODE_ENDURANCE ? 1.0f : 0.0f);
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (e[i].active) {
            float ew = (e[i].type == 6.0f || e[i].type == 9.0f) ? 36.0f : 20.0f;
            float eh = (e[i].type == 6.0f || e[i].type == 9.0f) ? 36.0f : 20.0f;

            if (e[i].type == 8.0f) {
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
                    if (!e[i].cloaked) e[i].hp -= 1;
                    AddExplosion(p.x + 10.0f, e[i].y + eh/2.0f, 2, RGB(0, 229, 255));
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
                if (b[j].active && b[j].x < e[i].x + ew && b[j].x + 4 > e[i].x && b[j].y < e[i].y + eh && b[j].y + 10 > e[i].y) {
                    if (e[i].cloaked && (rnd() % 100 < 75)) {
                        // Bullets miss cloaked fighter!
                    } else {
                        b[j].active = 0.0f;
                        shotsHit++; e[i].hp -= (b[j].type == 1.0f) ? 6 : 1;
                        AddExplosion(b[j].x, b[j].y, 3, RGB(0, 229, 255));
                    }
                    break;
                }
            }

            if (e[i].hp <= 0) {
                e[i].active = 0.0f;
                comboTimer = 180;
                if (comboMultiplier < 10) comboMultiplier++;
                int baseScore = (e[i].type == 6.0f ? 300 : (e[i].type == 9.0f ? 200 : (e[i].type == 8.0f ? 150 : (e[i].type == 7.0f ? 120 : 30))));
                score += baseScore * comboMultiplier;
                enemiesKilled++;
                totalKills++;
                PlaySnd(1);
                AddExplosion(e[i].x + 10.0f, e[i].y + 10.0f, 14, RGB(255, 152, 0));

                if (score > highScore) { highScore = score; SaveLeaderboard(); }

                if ((rnd() % 100) < 22) {
                    for (int k = 0; k < MAX_POWERUPS; k++) {
                        if (!pu[k].active) {
                            pu[k].active = 1.0f; pu[k].x = e[i].x; pu[k].y = e[i].y; pu[k].dy = 1.8f;
                            pu[k].type = (float)(rnd() % 9);
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

// GDI Rendering Helpers (Loop 2 Visual Upgrades)
void DrawPlayerShipGDI(HDC hdc, int x, int y, int shield, int frame) {
    HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);

    // Multi-Stage Animated Thruster Flames (Loop 2)
    int flameH = 5 + (frame % 3) * 3;
    // Outer flame plume
    HBRUSH ofbr = CreateSolidBrush((frame % 2 == 0) ? RGB(255, 60, 0) : RGB(255, 145, 0));
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, ofbr);
    POINT outerPts[3] = { {x + 4, y + 20}, {x + 10, y + 23 + flameH}, {x + 16, y + 20} };
    Polygon(hdc, outerPts, 3);
    SelectObject(hdc, oldBr); DeleteObject(ofbr);

    // Core flame
    HBRUSH fbr = CreateSolidBrush(RGB(255, 234, 0));
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
    HBRUSH wtfbr = CreateSolidBrush(RGB(0, 229, 255));
    SelectObject(hdc, wtfbr);
    RECT lwt = {x, y + 16, x + 2, y + 20}; FillRect(hdc, &lwt, wtfbr);
    RECT rwt = {x + 18, y + 16, x + 20, y + 20}; FillRect(hdc, &rwt, wtfbr);
    SelectObject(hdc, oldBr); DeleteObject(wtfbr);

    // Ship Hull
    HBRUSH wbr = CreateSolidBrush(RGB(0, 176, 255));
    SelectObject(hdc, wbr);
    POINT wingPts[6] = { {x + 10, y}, {x + 20, y + 16}, {x + 15, y + 20}, {x + 10, y + 15}, {x + 5, y + 20}, {x + 0, y + 16} };
    Polygon(hdc, wingPts, 6);
    SelectObject(hdc, oldBr); DeleteObject(wbr);

    // Cockpit
    HBRUSH cbr = CreateSolidBrush(RGB(255, 255, 255));
    SelectObject(hdc, cbr);
    Ellipse(hdc, x + 7, y + 4, x + 13, y + 12);
    SelectObject(hdc, oldBr); DeleteObject(cbr);

    if (hyperShieldTimer > 0) {
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
    if (type != 5 && type != 9) {
        int eflame = 3 + (frameCount % 3) * 2;
        COLORREF efcol = (type == 4 ? RGB(0, 229, 255) : (type == 7 ? RGB(255, 235, 59) : RGB(255, 60, 0)));
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
    }

    SelectObject(hdc, oldBr); DeleteObject(br);
    SelectObject(hdc, oldPen);
}

void DrawBossGDI(HDC hdc, float fx, float fy, int frame) {
    int x = (int)fx, y = (int)fy;
    HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);

    int isEnraged = (bossHp < bossMaxHp / 2);

    if (bossIsMothership) {
        // Stage 20 Alien Mothership Hull
        HBRUSH br = CreateSolidBrush(isEnraged ? RGB(183, 28, 28) : RGB(136, 14, 79));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
        POINT pts[8] = { {x + 45, y + 58}, {x + 85, y + 35}, {x + 90, y + 10}, {x + 65, y + 2}, {x + 45, y + 12}, {x + 25, y + 2}, {x + 0, y + 10}, {x + 5, y + 35} };
        Polygon(hdc, pts, 8);

        // Enraged Phase Electric Lightning Arcs
        if (isEnraged && frame % 2 == 0) {
            HPEN lpen = CreatePen(PS_SOLID, 2, RGB(0, 229, 255));
            SelectObject(hdc, lpen);
            MoveToEx(hdc, x + 45, y + 30, NULL);
            LineTo(hdc, x + 45 + (rnd() % 60) - 30, y + 30 + (rnd() % 40) - 20);
            SelectObject(hdc, oldBr); DeleteObject(lpen);
        }

        // Core Eye Overload
        COLORREF coreCol = isEnraged ? ((frame % 4 < 2) ? RGB(255, 234, 0) : RGB(255, 23, 68)) : ((frame % 8 < 4) ? RGB(0, 229, 255) : RGB(255, 23, 68));
        HBRUSH cbr = CreateSolidBrush(coreCol);
        SelectObject(hdc, cbr);
        Ellipse(hdc, x + 35, y + 20, x + 55, y + 40);
        SelectObject(hdc, oldBr); DeleteObject(cbr); DeleteObject(br);

        // Turrets Rendering
        float tOffsetsX[] = {5, 25, 65, 85};
        for (int i = 0; i < 4; i++) {
            if (turretActive[i]) {
                HBRUSH tbr = CreateSolidBrush(RGB(255, 145, 0));
                SelectObject(hdc, tbr);
                Ellipse(hdc, (int)(x + tOffsetsX[i] - 4), (int)(y + 25), (int)(x + tOffsetsX[i] + 12), (int)(y + 41));
                SelectObject(hdc, oldBr); DeleteObject(tbr);
            }
        }

        // Deflector Shield Ring
        if (IsMothershipShieldActive()) {
            HPEN spen = CreatePen(PS_SOLID, 3, RGB(0, 229, 255));
            SelectObject(hdc, spen);
            HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
            SelectObject(hdc, nullBr);
            Ellipse(hdc, x - 10, y - 10, x + 100, y + 68);
            SelectObject(hdc, oldBr); DeleteObject(spen);
        }
    } else { // Dreadnought Boss
        HBRUSH br = CreateSolidBrush(isEnraged ? RGB(213, 0, 249) : RGB(183, 28, 28));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
        POINT pts[6] = { {x + 30, y + 48}, {x + 58, y + 12}, {x + 45, y + 2}, {x + 30, y + 14}, {x + 15, y + 2}, {x + 2, y + 12} };
        Polygon(hdc, pts, 6);

        if (isEnraged && frame % 2 == 0) {
            HPEN lpen = CreatePen(PS_SOLID, 2, RGB(255, 234, 0));
            SelectObject(hdc, lpen);
            MoveToEx(hdc, x + 30, y + 24, NULL);
            LineTo(hdc, x + 30 + (rnd() % 40) - 20, y + 24 + (rnd() % 30) - 15);
            SelectObject(hdc, oldBr); DeleteObject(lpen);
        }

        COLORREF coreCol = isEnraged ? ((frame % 4 < 2) ? RGB(255, 255, 255) : RGB(255, 23, 68)) : ((frame % 10 < 5) ? RGB(255, 234, 0) : RGB(255, 23, 68));
        HBRUSH cbr = CreateSolidBrush(coreCol);
        SelectObject(hdc, cbr);
        Ellipse(hdc, x + 20, y + 14, x + 40, y + 34);

        SelectObject(hdc, oldBr); DeleteObject(br); DeleteObject(cbr);
    }
    SelectObject(hdc, oldPen);
}

void DrawPowerupGDI(HDC hdc, float fx, float fy, float ftype, int frame) {
    int x = (int)fx, y = (int)fy, type = (int)ftype;
    COLORREF cols[9] = { RGB(0, 230, 118), RGB(0, 229, 255), RGB(61, 90, 255), RGB(255, 23, 68), RGB(255, 234, 0), RGB(213, 0, 249), RGB(255, 215, 0), RGB(0, 176, 255), RGB(255, 100, 200) };
    COLORREF c = (type >= 0 && type < 9) ? cols[type] : RGB(255, 255, 255);
    HBRUSH br = CreateSolidBrush(c);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    int pulse = (frame % 20 < 10) ? 1 : 0;
    if (type % 3 == 0) {
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
                if (wParam == 'H') {
                    gameState = STATE_HELP;
                } else {
                    int opts = HasSavedGame() ? 5 : 4;
                    if (wParam == VK_UP || wParam == 'W') menuIndex = (menuIndex - 1 + opts) % opts;
                else if (wParam == VK_DOWN || wParam == 'S') menuIndex = (menuIndex + 1) % opts;
                else if (wParam == VK_RETURN || wParam == VK_SPACE) {
                    int saved = HasSavedGame();
                    if (menuIndex == 0) StartNewGame(MODE_CLASSIC);
                    else if (saved && menuIndex == 1) LoadGameState();
                    else if ((!saved && menuIndex == 1) || (saved && menuIndex == 2)) gameState = STATE_LEADERBOARD;
                    else if ((!saved && menuIndex == 2) || (saved && menuIndex == 3)) gameState = STATE_MODE_SELECT;
                    else if ((!saved && menuIndex == 3) || (saved && menuIndex == 4)) { ExportHighScoresJSON(); MessageBoxA(hwnd, "Exported kspace_highscores.json", "Export", MB_OK); }
                }
                }
            } else if (gameState == STATE_HELP) {
                if (wParam == 'H' || wParam == VK_ESCAPE || wParam == VK_RETURN || wParam == VK_SPACE) gameState = STATE_MENU;
            } else if (gameState == STATE_MODE_SELECT) {
                if (wParam == VK_UP || wParam == 'W') modeIndex = (modeIndex - 1 + 3) % 3;
                else if (wParam == VK_DOWN || wParam == 'S') modeIndex = (modeIndex + 1) % 3;
                else if (wParam == VK_RETURN || wParam == VK_SPACE) StartNewGame(modeIndex);
                else if (wParam == VK_ESCAPE) gameState = STATE_MENU;
            } else if (gameState == STATE_PLAYING) {
                if (wParam == kbPause || wParam == VK_ESCAPE) { gameState = STATE_PAUSED; menuIndex = 0; }
                else if (wParam == kbTimeStop) UseTimeStop();
                else if (wParam == kbDash) UseTacticalDash();
                else if (wParam == kbBomb || wParam == 'X') UseSmartBomb();
                else if (wParam == kbShield) UseHyperShield();
            } else if (gameState == STATE_PAUSED) {
                if (wParam == VK_UP || wParam == 'W') menuIndex = (menuIndex - 1 + 4) % 4;
                else if (wParam == VK_DOWN || wParam == 'S') menuIndex = (menuIndex + 1) % 4;
                else if (wParam == VK_RETURN || wParam == VK_SPACE) {
                    if (menuIndex == 0) gameState = STATE_PLAYING;
                    else if (menuIndex == 1) SaveGameState();
                    else if (menuIndex == 2) LoadGameState();
                    else if (menuIndex == 3) gameState = STATE_MENU;
                } else if (wParam == 'P' || wParam == VK_ESCAPE) gameState = STATE_PLAYING;
            } else if (gameState == STATE_LEADERBOARD || gameState == STATE_GAMEOVER || gameState == STATE_VICTORY || gameState == STATE_HELP) {
                if (wParam == VK_RETURN || wParam == VK_SPACE || wParam == VK_ESCAPE) gameState = STATE_MENU;
                if ((gameState == STATE_GAMEOVER || gameState == STATE_VICTORY) && wParam == 'E') { ExportStatsCSV(); MessageBoxA(hwnd, "Exported CSV", "Export", MB_OK); }
                if ((gameState == STATE_GAMEOVER || gameState == STATE_VICTORY) && wParam == 'J') { ExportStatsJSON(); MessageBoxA(hwnd, "Exported JSON", "Export", MB_OK); }
            }
            break;

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

            if (bombFlash > 0) {
                HBRUSH fbr = CreateSolidBrush(RGB(255, 255, 255));
                RECT rc = {0, 0, W, H};
                FillRect(memDC, &rc, fbr);
                DeleteObject(fbr);
            } else {
                int bgR = 5 + (wave * 4) % 25;
                int bgG = 5 + (wave * 6) % 25;
                int bgB = 18 + (wave * 10) % 40;
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
                        HBRUSH cbr = CreateSolidBrush(RGB(100, 10, 10)); // darker crater color
                        SelectObject(memDC, cbr);
                        Ellipse(memDC, px - (int)(pr*0.3f) - (int)(pr*0.15f), py - (int)(pr*0.2f) - (int)(pr*0.15f), px - (int)(pr*0.3f) + (int)(pr*0.15f), py - (int)(pr*0.2f) + (int)(pr*0.15f));
                        Ellipse(memDC, px + (int)(pr*0.4f) - (int)(pr*0.2f), py + (int)(pr*0.3f) - (int)(pr*0.2f), px + (int)(pr*0.4f) + (int)(pr*0.2f), py + (int)(pr*0.3f) + (int)(pr*0.2f));
                        SelectObject(memDC, oldBr); DeleteObject(cbr);
                    }
                    SelectObject(memDC, oldBr); DeleteObject(pbr);
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
                hFont = CreateFontA(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
                oldFont = (HFONT)SelectObject(memDC, hFont);

                if (gameState == STATE_MENU) {
                    SetTextColor(memDC, RGB(0, 229, 255));
                    TextOutA(memDC, W/2 - 40, 90, "KSPACE", 6);
                    SetTextColor(memDC, RGB(128, 216, 255));
                    TextOutA(memDC, W/2 - 80, 120, "Loop 7 Space Command", 20);

                    int saved = HasSavedGame();
                    char* opts[] = {"START NEW GAME", "RESUME SAVED GAME", "HIGH SCORES", "SELECT GAME MODE", "EXPORT SCORES"};
                    int count = saved ? 5 : 4;
                    int optIdxs[] = {0, 1, 2, 3, 4};
                    if (!saved) { optIdxs[1] = 2; optIdxs[2] = 3; optIdxs[3] = 4; }

                    for (int i = 0; i < count; i++) {
                        int y = 200 + i * 35;
                        if (i == menuIndex) {
                            SetTextColor(memDC, RGB(255, 234, 0));
                            char buf[64]; wsprintfA(buf, "> %s <", opts[optIdxs[i]]);
                            TextOutA(memDC, W/2 - lstrlenA(buf)*4, y, buf, lstrlenA(buf));
                        } else {
                            SetTextColor(memDC, RGB(255, 255, 255));
                            TextOutA(memDC, W/2 - lstrlenA(opts[optIdxs[i]])*4, y, opts[optIdxs[i]], lstrlenA(opts[optIdxs[i]]));
                        }
                    }
                    SetTextColor(memDC, RGB(255, 234, 0));
                    TextOutA(memDC, W/2 - 68, H - 30, "PRESS [H] FOR HELP", 18);
                } else if (gameState == STATE_MODE_SELECT) {
                    SetTextColor(memDC, RGB(255, 234, 0));
                    TextOutA(memDC, W/2 - 65, 80, "SELECT GAME MODE", 16);
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
                    SetTextColor(memDC, RGB(255, 23, 68));
                    TextOutA(memDC, W/2 - 80, H - 40, "Press ENTER to Launch", 21);
                } else if (gameState == STATE_HELP) {
                    SetTextColor(memDC, RGB(0, 229, 255));
                    TextOutA(memDC, W/2 - 45, 60, "HOW TO PLAY", 11);
                    SetTextColor(memDC, RGB(255, 255, 255));
                    char* lines[] = {
                        "ARROWS : Move Ship",
                        "SPACE  : Fire Weapon",
                        "",
                        "--- SKILLS ---",
                        "T : Time Stop (Freeze)",
                        "D : Tactical Dash (Invinc.)",
                        "B : Smart Bomb (Clear)",
                        "S : Hyper Shield (Protect)",
                        "",
                        "--- ITEMS ---",
                        "S:Spread L:Laser H:Shield",
                        "B:Bomb R:Rapid T:Time Stop",
                        "Y:Hyper D:Dash W:Upgrade"
                    };
                    for (int i = 0; i < 13; i++) {
                        TextOutA(memDC, W/2 - 95, 100 + i * 20, lines[i], lstrlenA(lines[i]));
                    }
                    SetTextColor(memDC, RGB(255, 234, 0));
                    TextOutA(memDC, W/2 - 90, H - 40, "Press [H] or ENTER to return", 28);
                } else if (gameState == STATE_LEADERBOARD) {
                    SetTextColor(memDC, RGB(0, 229, 255));
                    TextOutA(memDC, W/2 - 60, 60, "TOP COMMANDERS", 14);
                    for (int i = 0; i < MAX_LEADERBOARD; i++) {
                        char buf[64];
                        wsprintfA(buf, "#%d   Score:%d  Wave:%d", i + 1, leaderboard[i].score, leaderboard[i].wave);
                        SetTextColor(memDC, i == 0 ? RGB(255, 234, 0) : RGB(255, 255, 255));
                        TextOutA(memDC, 40, 120 + i * 35, buf, lstrlenA(buf));
                    }
                    SetTextColor(memDC, RGB(128, 216, 255));
                    TextOutA(memDC, W/2 - 75, H - 40, "Press ENTER to return", 21);
                } else if (gameState == STATE_PLAYING || gameState == STATE_PAUSED || gameState == STATE_GAMEOVER || gameState == STATE_VICTORY) {
                    DrawPlayerShipGDI(memDC, (int)p.x, (int)p.y, shieldActive, frameCount);

                    HBRUSH bbr = CreateSolidBrush(RGB(0, 229, 255));
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (b[i].active) {
                            RECT br = {(int)b[i].x, (int)b[i].y, (int)b[i].x + 4, (int)b[i].y + 10};
                            FillRect(memDC, &br, bbr);
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
                            FillRect(memDC, &dr, dbr);
                            DeleteObject(dbr);
                        }
                    }

                    // Shockwaves & Ripples
                    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
                        if (shockwaves[i].life > 0) {
                            shockwaves[i].r += shockwaves[i].speed;
                            if (shockwaves[i].r >= shockwaves[i].maxR) shockwaves[i].life = 0;
                            else {
                                HPEN swPen = CreatePen(PS_SOLID, 2, shockwaves[i].color);
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

                    // HUD
                    SetTextColor(memDC, RGB(255, 255, 255));
                    char hudStr[64];
                    wsprintfA(hudStr, "SCORE: %d  HIGH: %d  WAVE: %d/20", score, highScore, wave);
                    TextOutA(memDC, 10, 10, hudStr, lstrlenA(hudStr));

                    char statStr[64];
                    wsprintfA(statStr, "HP: %d  B:[B]%d  Shield: %s", p.hp, bombCount, (hyperShieldTimer > 0 || shieldActive) ? "ON" : "OFF");
                    TextOutA(memDC, 10, 26, statStr, lstrlenA(statStr));

                    // Skill Badges Status
                    char skillStr[96];
                    wsprintfA(skillStr, "[T]Stop:%s  [D]Dash:%s  [S]Shield:%s",
                        timeStopTimer > 0 ? "ACT" : (timeStopCooldown <= 0 ? "RDY" : "CD"),
                        invincibleTimer > 0 ? "ACT" : (dashCooldown <= 0 ? "RDY" : "CD"),
                        hyperShieldTimer > 0 ? "ACT" : (hyperShieldCooldown <= 0 ? "RDY" : "CD"));
                    SetTextColor(memDC, RGB(0, 230, 118));
                    TextOutA(memDC, 10, H - 24, skillStr, lstrlenA(skillStr));

                    if (bossActive) {
                        HBRUSH barBg = CreateSolidBrush(RGB(30, 30, 30));
                        RECT rbg = {40, 42, W - 40, 52};
                        FillRect(memDC, &rbg, barBg);
                        DeleteObject(barBg);

                        HBRUSH barFg = CreateSolidBrush(bossIsMothership ? RGB(213, 0, 249) : RGB(255, 23, 68));
                        int wFill = (int)((W - 80) * ((float)bossHp / bossMaxHp));
                        if (wFill < 0) wFill = 0;
                        RECT rfg = {40, 42, 40 + wFill, 52};
                        FillRect(memDC, &rfg, barFg);
                        DeleteObject(barFg);

                        SetTextColor(memDC, RGB(255, 255, 255));
                        TextOutA(memDC, W/2 - 60, 40, bossIsMothership ? "ALIEN MOTHERSHIP" : "DREADNOUGHT BOSS", bossIsMothership ? 16 : 16);
                    }

                    if (gameState == STATE_PAUSED) {
                        SetTextColor(memDC, RGB(255, 234, 0));
                        TextOutA(memDC, W/2 - 50, 120, "SYSTEM PAUSED", 13);
                        char* opts[] = {"RESUME GAME", "SAVE GAME STATE", "LOAD GAME STATE", "QUIT TO MENU"};
                        for (int i = 0; i < 4; i++) {
                            int y = 190 + i * 35;
                            if (i == menuIndex) {
                                SetTextColor(memDC, RGB(0, 229, 255));
                                char buf[64]; wsprintfA(buf, "> %s <", opts[i]);
                                TextOutA(memDC, W/2 - lstrlenA(buf)*4, y, buf, lstrlenA(buf));
                            } else {
                                SetTextColor(memDC, RGB(255, 255, 255));
                                TextOutA(memDC, W/2 - lstrlenA(opts[i])*4, y, opts[i], lstrlenA(opts[i]));
                            }
                        }
                    } else if (gameState == STATE_GAMEOVER) {
                        SetTextColor(memDC, RGB(255, 23, 68));
                        TextOutA(memDC, W/2 - 55, H/2 - 30, "MISSION FAILED", 14);
                        SetTextColor(memDC, RGB(255, 255, 255));
                        char finalStr[64];
                        wsprintfA(finalStr, "FINAL SCORE: %d", score);
                        TextOutA(memDC, W/2 - lstrlenA(finalStr)*4, H/2, finalStr, lstrlenA(finalStr));
                        TextOutA(memDC, W/2 - 80, H/2 + 30, "ENTER:Menu E:CSV J:JSON", 23);
                    } else if (gameState == STATE_VICTORY) {
                        SetTextColor(memDC, RGB(0, 230, 118));
                        TextOutA(memDC, W/2 - 65, H/2 - 40, "CAMPAIGN VICTORY!", 17);
                        SetTextColor(memDC, RGB(255, 234, 0));
                        TextOutA(memDC, W/2 - 80, H/2 - 15, "Alien Mothership Destroyed", 26);
                        SetTextColor(memDC, RGB(255, 255, 255));
                        char finalStr[64];
                        wsprintfA(finalStr, "FINAL SCORE: %d", score);
                        TextOutA(memDC, W/2 - lstrlenA(finalStr)*4, H/2 + 15, finalStr, lstrlenA(finalStr));
                        TextOutA(memDC, W/2 - 80, H/2 + 45, "ENTER:Menu E:CSV J:JSON", 23);
                    }
                }
            }

            if (oldFont) {
                SelectObject(memDC, oldFont);
                DeleteObject(hFont);
            }
            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBm);
            DeleteObject(hbm);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
            break;
        }

        case WM_DESTROY:
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
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);
    HWND hwnd = CreateWindowEx(0, "KSpaceApp", "KSpace", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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
