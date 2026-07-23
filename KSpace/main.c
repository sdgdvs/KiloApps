#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int _fltused = 1;
long _ftol2_sse(float f) { return (long)f; }
long _ftol2(float f) { return (long)f; }

#define W 320
#define H 480
#define MAX_BULLETS 30
#define MAX_ENEMIES 20
#define MAX_EBULLETS 25
#define MAX_POWERUPS 8
#define MAX_STARS 60
#define MAX_PARTICLES 64
#define MAX_LEADERBOARD 5

// --- GAME STATES ---
#define STATE_MENU 0
#define STATE_MODE_SELECT 1
#define STATE_PLAYING 2
#define STATE_PAUSED 3
#define STATE_LEADERBOARD 4
#define STATE_GAMEOVER 5

// --- GAME MODES ---
#define MODE_CLASSIC 0
#define MODE_ENDURANCE 1
#define MODE_BOSS_RUSH 2

typedef struct { float x, y, active, dx, dy, type; int hp, maxHp; } Ent;
typedef struct { float x, y, speed; int size; } Star;
typedef struct { float x, y, vx, vy; int life, maxLife; COLORREF color; } Particle;
typedef struct { int score, wave, mode; } LeaderEntry;

typedef struct {
    int score, wave, mode;
    int playerHp, shieldActive, bombs, weaponType;
    int spreadTimer, laserTimer, rapidTimer, timeStopTimer;
    float px, py;
    int enemiesKilled;
    int bossActive, bossHp, bossMaxHp, bossLevel;
    float bossX, bossY;
} SaveState;

// --- GLOBAL GAME DATA ---
int gameState = STATE_MENU;
int menuIndex = 0;
int modeIndex = MODE_CLASSIC;

Ent p = { W/2.0f - 10.0f, H - 60.0f, 1.0f, 0, 0, 0, 3, 3 }; // x, y, active, dx, dy, type, hp, maxHp
int shieldActive = 0;
int bombCount = 1;
int maxBombs = 3;
int weaponType = 0; // 0: Normal, 1: Spread, 2: Laser, 3: Plasma

Ent b[MAX_BULLETS] = {0};
Ent e[MAX_ENEMIES] = {0};
Ent eb[MAX_EBULLETS] = {0};
Ent pu[MAX_POWERUPS] = {0};
Particle particles[MAX_PARTICLES] = {0};
Star stars[MAX_STARS] = {0};

int spreadTimer = 0;
int laserTimer = 0;
int rapidTimer = 0;
int timeStopTimer = 0;

int score = 0;
int highScore = 0;
int totalKills = 0;
int enemiesKilled = 0;
int wave = 1;
int frameCount = 0;
int bombFlash = 0;

// Boss state
int bossActive = 0;
float bossX = 0, bossY = 0, bossDx = 2.0f;
int bossHp = 0, bossMaxHp = 0, bossLevel = 1, bossAttackTimer = 0;

LeaderEntry leaderboard[MAX_LEADERBOARD] = {
    { 5000, 10, MODE_CLASSIC },
    { 3500, 7, MODE_ENDURANCE },
    { 2500, 5, MODE_BOSS_RUSH },
    { 1500, 3, MODE_CLASSIC },
    { 800, 2, MODE_CLASSIC }
};

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
    return 0;
}

void PlaySnd(int type) {
    HANDLE hThread = CreateThread(NULL, 0, SndThread, (LPVOID)(intptr_t)type, 0, NULL);
    if (hThread) CloseHandle(hThread);
}

// Particle Explosions
void AddExplosion(float x, float y, int count, COLORREF col) {
    int added = 0;
    for (int i = 0; i < MAX_PARTICLES && added < count; i++) {
        if (particles[i].life <= 0) {
            particles[i].x = x;
            particles[i].y = y;
            particles[i].vx = (float)((int)(rnd() % 9) - 4) * 0.7f;
            particles[i].vy = (float)((int)(rnd() % 9) - 4) * 0.7f;
            particles[i].life = 12 + (rnd() % 14);
            particles[i].maxLife = particles[i].life;
            particles[i].color = col;
            added++;
        }
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
            int sz = (particles[i].life > 6) ? 3 : 2;
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
        s.bombs = bombCount;
        s.weaponType = weaponType;
        s.spreadTimer = spreadTimer;
        s.laserTimer = laserTimer;
        s.rapidTimer = rapidTimer;
        s.timeStopTimer = timeStopTimer;
        s.px = p.x;
        s.py = p.y;
        s.enemiesKilled = enemiesKilled;
        s.bossActive = bossActive;
        s.bossHp = bossHp;
        s.bossMaxHp = bossMaxHp;
        s.bossLevel = bossLevel;
        s.bossX = bossX;
        s.bossY = bossY;

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
            bombCount = s.bombs;
            weaponType = s.weaponType;
            spreadTimer = s.spreadTimer;
            laserTimer = s.laserTimer;
            rapidTimer = s.rapidTimer;
            timeStopTimer = s.timeStopTimer;
            p.x = s.px;
            p.y = s.py;
            enemiesKilled = s.enemiesKilled;
            bossActive = s.bossActive;
            bossHp = s.bossHp;
            bossMaxHp = s.bossMaxHp;
            bossLevel = s.bossLevel;
            bossX = s.bossX;
            bossY = s.bossY;

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

void SpawnBoss(int lvl) {
    bossActive = 1;
    bossX = W / 2.0f - 30.0f;
    bossY = -60.0f;
    bossMaxHp = 120 + lvl * 80;
    bossHp = bossMaxHp;
    bossLevel = lvl;
    bossDx = 2.0f;
    bossAttackTimer = 0;
    PlaySnd(4);
}

void DestroyBoss() {
    score += 1000 * bossLevel;
    enemiesKilled += 5;
    totalKills += 5;
    AddExplosion(bossX + 30.0f, bossY + 25.0f, 40, RGB(255, 23, 68));
    bossActive = 0;
    wave++;
    PlaySnd(3);

    // Drop 2 Powerups
    for (int k = 0; k < MAX_POWERUPS; k++) {
        if (!pu[k].active) {
            pu[k].active = 1.0f;
            pu[k].x = bossX + 15.0f;
            pu[k].y = bossY + 20.0f;
            pu[k].dy = 1.5f;
            pu[k].type = (float)(rnd() % 6);
            break;
        }
    }
    for (int k = 0; k < MAX_POWERUPS; k++) {
        if (!pu[k].active) {
            pu[k].active = 1.0f;
            pu[k].x = bossX + 40.0f;
            pu[k].y = bossY + 20.0f;
            pu[k].dy = 1.5f;
            pu[k].type = (float)(rnd() % 6);
            break;
        }
    }

    if (modeIndex == MODE_BOSS_RUSH) {
        SpawnBoss(wave);
    }
}

void SpawnEnemy() {
    if (bossActive) return;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!e[i].active) {
            e[i].active = 1.0f;
            e[i].x = (float)(rnd() % (W - 24));
            e[i].y = -20.0f;
            int t = rnd() % 100;
            if (t < 35) e[i].type = 0.0f;
            else if (t < 50) e[i].type = 1.0f;
            else if (t < 65) e[i].type = 2.0f;
            else if (t < 75) e[i].type = 3.0f;
            else if (t < 85) { e[i].type = 4.0f; e[i].dx = (rnd()%2==0?2.0f:-2.0f); }
            else if (t < 95) e[i].type = 5.0f;
            else { e[i].type = 6.0f; e[i].dx = (rnd()%2==0?2.0f:-2.0f); }

            e[i].hp = (e[i].type == 6.0f) ? 45 : ((e[i].type == 5.0f) ? 25 : ((e[i].type == 3.0f) ? 10 : ((e[i].type == 2.0f) ? 5 : ((e[i].type == 4.0f) ? 3 : 1))));
            e[i].maxHp = e[i].hp;
            break;
        }
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
            e[i].active = 0.0f;
            score += 20;
            enemiesKilled++;
            totalKills++;
            AddExplosion(e[i].x + 10.0f, e[i].y + 10.0f, 16, RGB(0, 229, 255));
        }
    }
    for (int i = 0; i < MAX_EBULLETS; i++) eb[i].active = 0;

    if (bossActive) {
        bossHp -= 35;
        AddExplosion(bossX + 30.0f, bossY + 25.0f, 30, RGB(255, 23, 68));
        if (bossHp <= 0) DestroyBoss();
    }
}

void Shoot() {
    PlaySnd(laserTimer > 0 ? 5 : 0);
    if (laserTimer > 0) return;

    if (spreadTimer > 0 || weaponType == 1) {
        float dxs[] = {-2.5f, 0.0f, 2.5f};
        float dys[] = {-8.0f, -9.0f, -8.0f};
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
                if (spawned >= 3) break;
            }
        }
    } else if (weaponType == 3) { // Plasma Orb
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!b[i].active) {
                b[i].active = 1.0f;
                b[i].x = p.x + 6.0f;
                b[i].y = p.y;
                b[i].dx = 0.0f;
                b[i].dy = -5.0f;
                b[i].type = 1;
                break;
            }
        }
    } else { // Standard Twin Blaster
        int spawned = 0;
        float offsets[] = {3.0f, 13.0f};
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!b[i].active) {
                b[i].active = 1.0f;
                b[i].x = p.x + offsets[spawned];
                b[i].y = p.y;
                b[i].dx = 0.0f;
                b[i].dy = -8.0f;
                b[i].type = 0;
                spawned++;
                if (spawned >= 2) break;
            }
        }
    }
}

void StartNewGame(int modeIdx) {
    modeIndex = modeIdx;
    p.x = W / 2.0f - 10.0f;
    p.y = H - 60.0f;
    p.hp = 3;
    p.maxHp = 3;
    shieldActive = 0;
    bombCount = 1;
    weaponType = 0;
    score = 0;
    wave = 1;
    enemiesKilled = 0;
    spreadTimer = 0;
    laserTimer = 0;
    rapidTimer = 0;
    timeStopTimer = 0;
    bossActive = 0;

    for (int i = 0; i < MAX_ENEMIES; i++) e[i].active = 0;
    for (int i = 0; i < MAX_BULLETS; i++) b[i].active = 0;
    for (int i = 0; i < MAX_EBULLETS; i++) eb[i].active = 0;
    for (int i = 0; i < MAX_POWERUPS; i++) pu[i].active = 0;
    for (int i = 0; i < MAX_PARTICLES; i++) particles[i].life = 0;

    gameState = STATE_PLAYING;
    PlaySnd(3);

    if (modeIndex == MODE_BOSS_RUSH) SpawnBoss(1);
}

void PlayerHit() {
    if (shieldActive) {
        shieldActive = 0;
        AddExplosion(p.x + 10.0f, p.y + 10.0f, 16, RGB(0, 229, 255));
        PlaySnd(1);
    } else {
        p.hp--;
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
    else if (type == 2) { shieldActive = 1; }
    else if (type == 3) { if (bombCount < maxBombs) bombCount++; }
    else if (type == 4) { rapidTimer = 350; }
    else if (type == 5) { timeStopTimer = 300; }
}

void Update() {
    if (gameState != STATE_PLAYING) return;

    if (bombFlash > 0) bombFlash--;

    // Mode Progression
    if (modeIndex == MODE_CLASSIC) {
        int targetWave = 1 + (score / 1000);
        if (targetWave > wave) {
            wave = targetWave;
            PlaySnd(3);
            if (wave % 5 == 0 && !bossActive) SpawnBoss(wave / 5);
        }
    } else if (modeIndex == MODE_ENDURANCE) {
        wave = 1 + (score / 800);
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

    // Controls
    float speed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 7.0f : 4.5f;
    if ((GetAsyncKeyState(VK_LEFT) & 0x8000) || (GetAsyncKeyState('A') & 0x8000)) p.x -= speed;
    if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) || (GetAsyncKeyState('D') & 0x8000)) p.x += speed;
    if ((GetAsyncKeyState(VK_UP) & 0x8000) || (GetAsyncKeyState('W') & 0x8000)) p.y -= speed;
    if ((GetAsyncKeyState(VK_DOWN) & 0x8000) || (GetAsyncKeyState('S') & 0x8000)) p.y += speed;

    if (p.x < 0) p.x = 0;
    if (p.x > W - 20) p.x = W - 20;
    if (p.y < 0) p.y = 0;
    if (p.y > H - 20) p.y = H - 20;

    // Firing
    if ((GetAsyncKeyState(VK_SPACE) & 0x8001) || (GetAsyncKeyState(VK_RETURN) & 0x8001)) {
        int fireRate = (rapidTimer > 0) ? 3 : 7;
        if (frameCount % fireRate == 0) Shoot();
    }

    // Timers
    if (spreadTimer > 0) spreadTimer--;
    if (laserTimer > 0) laserTimer--;
    if (rapidTimer > 0) rapidTimer--;
    if (timeStopTimer > 0) timeStopTimer--;

    // Bullets Movement
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (b[i].active) {
            b[i].y += b[i].dy;
            b[i].x += b[i].dx;
            if (b[i].y < -10 || b[i].x < -10 || b[i].x > W + 10) b[i].active = 0.0f;
        }
    }

    // Boss Logic
    if (bossActive) {
        if (bossY < 50.0f) bossY += 1.0f;
        else {
            bossX += bossDx;
            if (bossX < 10.0f || bossX > W - 70.0f) bossDx = -bossDx;
        }

        bossAttackTimer++;
        if (timeStopTimer == 0 && bossAttackTimer % 45 == 0) {
            int bspawned = 0;
            float bdx[] = {-1.5f, 0.0f, 1.5f};
            for (int k = 0; k < MAX_EBULLETS; k++) {
                if (!eb[k].active) {
                    eb[k].active = 1.0f;
                    eb[k].x = bossX + 30.0f;
                    eb[k].y = bossY + 45.0f;
                    eb[k].dy = 3.5f;
                    eb[k].dx = bdx[bspawned];
                    bspawned++;
                    if (bspawned >= 3) break;
                }
            }
        }

        if (laserTimer > 0) {
            if (p.x + 10.0f >= bossX && p.x + 10.0f <= bossX + 60.0f) {
                bossHp -= 1;
                AddExplosion(p.x + 10.0f, bossY + 45.0f, 2, RGB(0, 229, 255));
                if (bossHp <= 0) DestroyBoss();
            }
        }

        for (int i = 0; i < MAX_BULLETS; i++) {
            if (b[i].active && b[i].x >= bossX && b[i].x <= bossX + 60.0f && b[i].y >= bossY && b[i].y <= bossY + 50.0f) {
                b[i].active = 0.0f;
                bossHp -= (b[i].type == 1.0f) ? 8 : 2;
                AddExplosion(b[i].x, b[i].y, 3, RGB(0, 229, 255));
                if (bossHp <= 0) { DestroyBoss(); break; }
            }
        }
    }

    // Spawning
    int spawnRate = 32 - (score / 150) - (modeIndex == MODE_ENDURANCE ? 8 : 0);
    if (spawnRate < 8) spawnRate = 8;
    if (frameCount % spawnRate == 0) SpawnEnemy();

    // Regular Enemies Update
    float baseEnemySpeed = 1.8f + (score / 250.0f) + (modeIndex == MODE_ENDURANCE ? 1.0f : 0.0f);
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (e[i].active) {
            float ew = (e[i].type == 6.0f) ? 36.0f : 20.0f;
            float eh = (e[i].type == 6.0f) ? 36.0f : 20.0f;

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
                } else if (e[i].type == 5.0f) e[i].y += baseEnemySpeed * 1.4f;
                else if (e[i].type == 6.0f) {
                    if (e[i].y < 60.0f) e[i].y += 0.8f;
                    else {
                        e[i].x += e[i].dx;
                        if (e[i].x < 0 || e[i].x > W - ew) e[i].dx = -e[i].dx;
                    }
                }

                if (e[i].type == 2.0f && (frameCount % 60 == 0) && (rnd() % 2 == 0)) {
                    for (int j = 0; j < MAX_EBULLETS; j++) {
                        if (!eb[j].active) {
                            eb[j].active = 1.0f;
                            eb[j].x = e[i].x + 10.0f;
                            eb[j].y = e[i].y + 20.0f;
                            eb[j].dy = 3.5f;
                            eb[j].dx = 0.0f;
                            break;
                        }
                    }
                }
            }

            // Laser collision
            if (laserTimer > 0) {
                if (p.x + 10.0f >= e[i].x && p.x + 10.0f <= e[i].x + ew && p.y > e[i].y) {
                    e[i].hp -= 1;
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
                    b[j].active = 0.0f;
                    e[i].hp -= (b[j].type == 1.0f) ? 6 : 1;
                    AddExplosion(b[j].x, b[j].y, 3, RGB(0, 229, 255));
                    break;
                }
            }

            if (e[i].hp <= 0) {
                e[i].active = 0.0f;
                score += (e[i].type == 6.0f ? 300 : (e[i].type == 5.0f ? 100 : (e[i].type == 3.0f ? 80 : 20)));
                enemiesKilled++;
                totalKills++;
                PlaySnd(1);
                AddExplosion(e[i].x + 10.0f, e[i].y + 10.0f, 14, RGB(255, 152, 0));

                if (score > highScore) { highScore = score; SaveLeaderboard(); }

                if ((rnd() % 100) < 18) {
                    for (int k = 0; k < MAX_POWERUPS; k++) {
                        if (!pu[k].active) {
                            pu[k].active = 1.0f; pu[k].x = e[i].x; pu[k].y = e[i].y; pu[k].dy = 1.8f;
                            pu[k].type = (float)(rnd() % 6);
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

// GDI Rendering Helpers
void DrawPlayerShipGDI(HDC hdc, int x, int y, int shield, int frame) {
    HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);

    int flameH = 4 + (frame % 3) * 3;
    COLORREF flameCol = (frame % 2 == 0) ? RGB(255, 200, 0) : RGB(255, 60, 0);
    HBRUSH fbr = CreateSolidBrush(flameCol);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, fbr);
    POINT flamePts[3] = { {x + 6, y + 20}, {x + 10, y + 20 + flameH}, {x + 14, y + 20} };
    Polygon(hdc, flamePts, 3);
    SelectObject(hdc, oldBr);
    DeleteObject(fbr);

    HBRUSH wbr = CreateSolidBrush(RGB(0, 176, 255));
    SelectObject(hdc, wbr);
    POINT wingPts[6] = { {x + 10, y}, {x + 20, y + 16}, {x + 15, y + 20}, {x + 10, y + 15}, {x + 5, y + 20}, {x + 0, y + 16} };
    Polygon(hdc, wingPts, 6);
    SelectObject(hdc, oldBr);
    DeleteObject(wbr);

    HBRUSH cbr = CreateSolidBrush(RGB(255, 255, 255));
    SelectObject(hdc, cbr);
    Ellipse(hdc, x + 7, y + 4, x + 13, y + 12);
    SelectObject(hdc, oldBr);
    DeleteObject(cbr);

    if (shield) {
        HPEN spen = CreatePen(PS_SOLID, 2, RGB(0, 229, 255));
        SelectObject(hdc, spen);
        HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
        SelectObject(hdc, nullBr);
        Ellipse(hdc, x - 4, y - 4, x + 24, y + 24);
        SelectObject(hdc, oldBr);
        DeleteObject(spen);
    }

    if (laserTimer > 0) {
        HPEN lpen = CreatePen(PS_SOLID, 4, RGB(0, 229, 255));
        SelectObject(hdc, lpen);
        MoveToEx(hdc, x + 10, y, NULL);
        LineTo(hdc, x + 10, 0);
        SelectObject(hdc, oldPen);
        DeleteObject(lpen);
    }

    SelectObject(hdc, oldPen);
}

void DrawEnemyShipGDI(HDC hdc, float fx, float fy, float ftype, int frame) {
    int x = (int)fx, y = (int)fy, type = (int)ftype;
    HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);

    if (type == 0) {
        HBRUSH br = CreateSolidBrush(RGB(255, 23, 68));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
        POINT pts[6] = { {x, y + 2}, {x + 10, y + 18}, {x + 20, y + 2}, {x + 13, y + 6}, {x + 10, y}, {x + 7, y + 6} };
        Polygon(hdc, pts, 6);
        SelectObject(hdc, oldBr); DeleteObject(br);
    } else if (type == 1) {
        HBRUSH br = CreateSolidBrush(RGB(255, 145, 0));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
        POINT pts[4] = { {x + 10, y + 20}, {x + 20, y + 4}, {x + 10, y}, {x, y + 4} };
        Polygon(hdc, pts, 4);
        SelectObject(hdc, oldBr); DeleteObject(br);
    } else if (type == 2) {
        HBRUSH br = CreateSolidBrush(RGB(213, 0, 249));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
        Ellipse(hdc, x, y + 6, x + 20, y + 18);
        SelectObject(hdc, oldBr); DeleteObject(br);
    } else if (type == 3) {
        HBRUSH br = CreateSolidBrush(RGB(120, 144, 156));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
        RECT rc = {x + 2, y + 2, x + 18, y + 18};
        FillRect(hdc, &rc, br);
        SelectObject(hdc, oldBr); DeleteObject(br);
    } else if (type == 4) {
        HBRUSH br = CreateSolidBrush(RGB(0, 229, 255));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
        POINT pts[4] = { {x + 10, y + 18}, {x + 20, y}, {x + 10, y + 6}, {x, y} };
        Polygon(hdc, pts, 4);
        SelectObject(hdc, oldBr); DeleteObject(br);
    } else if (type == 5) {
        HBRUSH br = CreateSolidBrush(RGB(141, 110, 99));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
        Ellipse(hdc, x + 1, y + 1, x + 19, y + 19);
        SelectObject(hdc, oldBr); DeleteObject(br);
    } else if (type == 6) {
        HBRUSH br = CreateSolidBrush(RGB(198, 40, 40));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
        POINT pts[4] = { {x + 18, y + 34}, {x + 34, y + 8}, {x + 18, y + 12}, {x + 2, y + 8} };
        Polygon(hdc, pts, 4);
        SelectObject(hdc, oldBr); DeleteObject(br);
    }
    SelectObject(hdc, oldPen);
}

void DrawBossGDI(HDC hdc, float fx, float fy, int frame) {
    int x = (int)fx, y = (int)fy;
    HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);

    HBRUSH br = CreateSolidBrush(RGB(183, 28, 28));
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
    POINT pts[6] = { {x + 30, y + 48}, {x + 58, y + 12}, {x + 45, y + 2}, {x + 30, y + 14}, {x + 15, y + 2}, {x + 2, y + 12} };
    Polygon(hdc, pts, 6);

    COLORREF coreCol = (frame % 10 < 5) ? RGB(255, 234, 0) : RGB(255, 23, 68);
    HBRUSH cbr = CreateSolidBrush(coreCol);
    SelectObject(hdc, cbr);
    Ellipse(hdc, x + 20, y + 14, x + 40, y + 34);

    SelectObject(hdc, oldBr);
    DeleteObject(br); DeleteObject(cbr);
    SelectObject(hdc, oldPen);
}

void DrawPowerupGDI(HDC hdc, float fx, float fy, float ftype) {
    int x = (int)fx, y = (int)fy, type = (int)ftype;
    COLORREF cols[6] = { RGB(0, 230, 118), RGB(0, 229, 255), RGB(61, 90, 255), RGB(255, 23, 68), RGB(255, 234, 0), RGB(213, 0, 249) };
    COLORREF c = (type >= 0 && type < 6) ? cols[type] : RGB(255, 255, 255);
    HBRUSH br = CreateSolidBrush(c);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    Ellipse(hdc, x, y, x + 16, y + 16);
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
                stars[i].speed = 0.8f + (rnd() % 3);
                stars[i].size = (rnd() % 5 == 0) ? 2 : 1;
            }
            SetTimer(hwnd, 1, 16, NULL);
            break;

        case WM_KEYDOWN:
            if (gameState == STATE_MENU) {
                int opts = HasSavedGame() ? 4 : 3;
                if (wParam == VK_UP || wParam == 'W') menuIndex = (menuIndex - 1 + opts) % opts;
                else if (wParam == VK_DOWN || wParam == 'S') menuIndex = (menuIndex + 1) % opts;
                else if (wParam == VK_RETURN || wParam == VK_SPACE) {
                    int saved = HasSavedGame();
                    if (menuIndex == 0) gameState = STATE_MODE_SELECT;
                    else if (saved && menuIndex == 1) LoadGameState();
                    else if ((!saved && menuIndex == 1) || (saved && menuIndex == 2)) gameState = STATE_LEADERBOARD;
                    else gameState = STATE_MODE_SELECT;
                }
            } else if (gameState == STATE_MODE_SELECT) {
                if (wParam == VK_UP || wParam == 'W') modeIndex = (modeIndex - 1 + 3) % 3;
                else if (wParam == VK_DOWN || wParam == 'S') modeIndex = (modeIndex + 1) % 3;
                else if (wParam == VK_RETURN || wParam == VK_SPACE) StartNewGame(modeIndex);
                else if (wParam == VK_ESCAPE) gameState = STATE_MENU;
            } else if (gameState == STATE_PLAYING) {
                if (wParam == 'P' || wParam == VK_ESCAPE) { gameState = STATE_PAUSED; menuIndex = 0; }
                else if (wParam == 'B' || wParam == 'X') UseSmartBomb();
            } else if (gameState == STATE_PAUSED) {
                if (wParam == VK_UP || wParam == 'W') menuIndex = (menuIndex - 1 + 4) % 4;
                else if (wParam == VK_DOWN || wParam == 'S') menuIndex = (menuIndex + 1) % 4;
                else if (wParam == VK_RETURN || wParam == VK_SPACE) {
                    if (menuIndex == 0) gameState = STATE_PLAYING;
                    else if (menuIndex == 1) SaveGameState();
                    else if (menuIndex == 2) LoadGameState();
                    else if (menuIndex == 3) gameState = STATE_MENU;
                } else if (wParam == 'P' || wParam == VK_ESCAPE) gameState = STATE_PLAYING;
            } else if (gameState == STATE_LEADERBOARD || gameState == STATE_GAMEOVER) {
                if (wParam == VK_RETURN || wParam == VK_SPACE || wParam == VK_ESCAPE) gameState = STATE_MENU;
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

                HBRUSH sbr = CreateSolidBrush(RGB(255, 255, 255));
                for (int i = 0; i < MAX_STARS; i++) {
                    RECT sr = {(int)stars[i].x, (int)stars[i].y, (int)stars[i].x + stars[i].size, (int)stars[i].y + stars[i].size};
                    FillRect(memDC, &sr, sbr);
                }
                DeleteObject(sbr);

                SetBkMode(memDC, TRANSPARENT);

                if (gameState == STATE_MENU) {
                    SetTextColor(memDC, RGB(0, 229, 255));
                    TextOutA(memDC, W/2 - 40, 90, "KSPACE", 6);
                    SetTextColor(memDC, RGB(128, 216, 255));
                    TextOutA(memDC, W/2 - 75, 120, "Deep Space System", 17);

                    int saved = HasSavedGame();
                    char* opts[] = {"START NEW GAME", "RESUME SAVED GAME", "HIGH SCORES", "SELECT GAME MODE"};
                    int count = saved ? 4 : 3;
                    int optIdxs[] = {0, 1, 2, 3};
                    if (!saved) { optIdxs[1] = 2; optIdxs[2] = 3; }

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
                } else if (gameState == STATE_MODE_SELECT) {
                    SetTextColor(memDC, RGB(255, 234, 0));
                    TextOutA(memDC, W/2 - 65, 80, "SELECT GAME MODE", 16);
                    char* modes[] = {"Classic Arcade", "Endurance Wave", "Boss Rush"};
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
                } else if (gameState == STATE_PLAYING || gameState == STATE_PAUSED || gameState == STATE_GAMEOVER) {
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
                        if (e[i].active) DrawEnemyShipGDI(memDC, e[i].x, e[i].y, e[i].type, frameCount);
                    }

                    if (bossActive) DrawBossGDI(memDC, bossX, bossY, frameCount);
                    for (int i = 0; i < MAX_POWERUPS; i++) {
                        if (pu[i].active) DrawPowerupGDI(memDC, pu[i].x, pu[i].y, pu[i].type);
                    }

                    DrawParticles(memDC);

                    // HUD
                    SetTextColor(memDC, RGB(255, 255, 255));
                    char hudStr[64];
                    wsprintfA(hudStr, "SCORE: %d  HIGH: %d  WAVE: %d", score, highScore, wave);
                    TextOutA(memDC, 10, 10, hudStr, lstrlenA(hudStr));

                    char statStr[64];
                    wsprintfA(statStr, "HP: %d  Bombs: %d  Shield: %s", p.hp, bombCount, shieldActive ? "ON" : "OFF");
                    TextOutA(memDC, 10, 28, statStr, lstrlenA(statStr));

                    if (bossActive) {
                        HBRUSH barBg = CreateSolidBrush(RGB(30, 30, 30));
                        RECT rbg = {40, 42, W - 40, 52};
                        FillRect(memDC, &rbg, barBg);
                        DeleteObject(barBg);

                        HBRUSH barFg = CreateSolidBrush(RGB(255, 23, 68));
                        int wFill = (int)((W - 80) * ((float)bossHp / bossMaxHp));
                        if (wFill < 0) wFill = 0;
                        RECT rfg = {40, 42, 40 + wFill, 52};
                        FillRect(memDC, &rfg, barFg);
                        DeleteObject(barFg);

                        SetTextColor(memDC, RGB(255, 255, 255));
                        TextOutA(memDC, W/2 - 45, 40, "BOSS DREADNOUGHT", 16);
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
                        TextOutA(memDC, W/2 - 80, H/2 + 30, "Press ENTER for Main Menu", 25);
                    }
                }
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
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KSpaceApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KSpaceApp", "KSpace", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, W + 16, H + 39, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
