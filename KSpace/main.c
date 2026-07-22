#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int _fltused = 1;
long _ftol2_sse(float f) { return (long)f; }
long _ftol2(float f) { return (long)f; }

#define W 320
#define H 480
#define MAX_BULLETS 20
#define MAX_ENEMIES 15
#define MAX_STARS 50
#define MAX_PARTICLES 64

typedef struct { float x, y, active, dx, dy, type; int hp; } Ent;
typedef struct { float x, y, speed; } Star;
typedef struct { float x, y, vx, vy; int life, maxLife; COLORREF color; } Particle;

Ent p = { W/2.0f, H - 50.0f, 1.0f, 0, 0, 0 };
Ent b[MAX_BULLETS] = {0};
Ent e[MAX_ENEMIES] = {0};
#define MAX_EBULLETS 20
Ent eb[MAX_EBULLETS] = {0};
#define MAX_POWERUPS 5
Ent pu[MAX_POWERUPS] = {0};
Particle particles[MAX_PARTICLES] = {0};

int spreadTimer = 0;
Star stars[MAX_STARS] = {0};
int score = 0;
int highScore = 0;
int totalKills = 0;
int gameOver = 0;
int frameCount = 0;
int shieldActive = 0;
int wave = 1;
int enemiesKilled = 0;
int rapidTimer = 0;
int timeStopTimer = 0;

unsigned int seed = 999;
unsigned int rnd() {
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

void AddExplosion(float x, float y, int count, COLORREF col) {
    int added = 0;
    for (int i = 0; i < MAX_PARTICLES && added < count; i++) {
        if (particles[i].life <= 0) {
            particles[i].x = x;
            particles[i].y = y;
            particles[i].vx = (float)((int)(rnd() % 7) - 3) * 0.8f;
            particles[i].vy = (float)((int)(rnd() % 7) - 3) * 0.8f;
            particles[i].life = 12 + (rnd() % 12);
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

void LoadHighScore() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\KSpace", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD type = REG_DWORD;
        DWORD size = sizeof(DWORD);
        RegQueryValueExA(hKey, "HighScore", NULL, &type, (LPBYTE)&highScore, &size);
        size = sizeof(DWORD);
        RegQueryValueExA(hKey, "TotalKills", NULL, &type, (LPBYTE)&totalKills, &size);
        RegCloseKey(hKey);
    }
}

void SaveHighScore() {
    HKEY hKey;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\KSpace", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "HighScore", 0, REG_DWORD, (const BYTE*)&highScore, sizeof(DWORD));
        RegSetValueExA(hKey, "TotalKills", 0, REG_DWORD, (const BYTE*)&totalKills, sizeof(DWORD));
        RegCloseKey(hKey);
    }
}

void SpawnEnemy() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!e[i].active) {
            e[i].active = 1.0f;
            e[i].x = (float)(rnd() % (W - 20));
            e[i].y = -20.0f;
            int t = rnd() % 100;
            if (t < 35) e[i].type = 0.0f;
            else if (t < 50) e[i].type = 1.0f;
            else if (t < 65) e[i].type = 2.0f;
            else if (t < 75) e[i].type = 3.0f;
            else if (t < 85) { e[i].type = 4.0f; e[i].dx = (rnd()%2==0?2.0f:-2.0f); }
            else if (t < 95) e[i].type = 5.0f;
            else { e[i].type = 6.0f; e[i].dx = (rnd()%2==0?2.0f:-2.0f); }
            if (e[i].type == 3.0f) e[i].hp = 10;
            else if (e[i].type == 2.0f) e[i].hp = 5;
            else if (e[i].type == 4.0f) e[i].hp = 3;
            else if (e[i].type == 5.0f) e[i].hp = 30;
            else if (e[i].type == 6.0f) e[i].hp = 50;
            else e[i].hp = 1;
            break;
        }
    }
}

DWORD WINAPI SndThread(LPVOID param) {
    int type = (int)(intptr_t)param;
    if (type == 0) Beep(1000, 20); // shoot
    else if (type == 1) Beep(200, 40); // explode
    else if (type == 2) { Beep(600, 50); Beep(800, 50); } // powerup
    else if (type == 3) { Beep(400, 100); Beep(600, 100); Beep(800, 150); } // wave
    return 0;
}
void PlaySnd(int type) {
    HANDLE hThread = CreateThread(NULL, 0, SndThread, (LPVOID)(intptr_t)type, 0, NULL);
    if (hThread) CloseHandle(hThread);
}

void Shoot() {
    PlaySnd(0);
    int shots = (spreadTimer > 0) ? 3 : 1;
    float dxs[] = {0.0f, -2.0f, 2.0f};
    int spawned = 0;
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!b[i].active) {
            b[i].active = 1.0f;
            b[i].x = p.x + 8.0f;
            b[i].y = p.y;
            b[i].dx = dxs[spawned];
            b[i].dy = -8.0f;
            spawned++;
            if(spawned >= shots) break;
        }
    }
}

void Update() {
    if (gameOver) {
        if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
            gameOver = 0;
            score = 0;
            for(int i=0; i<MAX_ENEMIES; i++) e[i].active = 0;
            for(int i=0; i<MAX_BULLETS; i++) b[i].active = 0;
            for(int i=0; i<MAX_EBULLETS; i++) eb[i].active = 0;
            for(int i=0; i<MAX_POWERUPS; i++) pu[i].active = 0;
            for(int i=0; i<MAX_PARTICLES; i++) particles[i].life = 0;
            spreadTimer = 0;
            rapidTimer = 0;
            timeStopTimer = 0;
            shieldActive = 0;
            wave = 1;
            enemiesKilled = 0;
            p.x = W/2.0f; p.y = H - 50.0f;
        }
        return;
    }
    
    int currentWave = 1 + (score / 1000);
    if (currentWave > wave) {
        wave = currentWave;
        PlaySnd(3);
    }
    
    for (int i=0; i<MAX_STARS; i++) {
        stars[i].y += stars[i].speed;
        if (stars[i].y > H) {
            stars[i].y = 0;
            stars[i].x = (float)(rnd() % W);
        }
    }

    UpdateParticles();
    
    float baseEnemySpeed = 2.0f + (score / 200.0f);
    int spawnRate = 30 - (score / 100);
    if (spawnRate < 10) spawnRate = 10;
    
    float speed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 8.0f : 4.0f;
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) p.x -= speed;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) p.x += speed;
    if (GetAsyncKeyState(VK_UP) & 0x8000) p.y -= speed;
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) p.y += speed;
    
    if (p.x < 0) p.x = 0;
    if (p.x > W - 20) p.x = W - 20;
    if (p.y < 0) p.y = 0;
    if (p.y > H - 20) p.y = H - 20;
    
    if (GetAsyncKeyState(VK_SPACE) & 0x8001) {
        int fireRate = (rapidTimer > 0) ? 2 : 5;
        if (frameCount % fireRate == 0) Shoot();
    }
    
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (b[i].active) {
            b[i].y += b[i].dy;
            b[i].x += b[i].dx;
            if (b[i].y < 0 || b[i].x < 0 || b[i].x > W) b[i].active = 0.0f;
        }
    }
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (e[i].active) {
            float ew = (e[i].type == 6.0f) ? 40.0f : 20.0f;
            float eh = (e[i].type == 6.0f) ? 40.0f : 20.0f;
            if (timeStopTimer == 0) {
                if (e[i].type == 0.0f) {
                    e[i].y += baseEnemySpeed;
                } else if (e[i].type == 1.0f) {
                    e[i].y += baseEnemySpeed * 0.75f;
                    if (e[i].x < p.x) e[i].x += baseEnemySpeed * 0.5f;
                    if (e[i].x > p.x) e[i].x -= baseEnemySpeed * 0.5f;
                } else if (e[i].type == 2.0f) {
                    e[i].y += baseEnemySpeed * 0.4f;
                } else if (e[i].type == 3.0f) {
                    e[i].y += baseEnemySpeed * 0.3f;
                } else if (e[i].type == 4.0f) {
                    e[i].y += baseEnemySpeed * 0.8f;
                    e[i].x += e[i].dx;
                    if (e[i].x < 0) { e[i].x = 0; e[i].dx = -e[i].dx; }
                    if (e[i].x > W - 20) { e[i].x = W - 20; e[i].dx = -e[i].dx; }
                } else if (e[i].type == 5.0f) {
                    e[i].y += baseEnemySpeed * 1.2f;
                } else if (e[i].type == 6.0f) {
                    if (e[i].y < 50.0f) {
                        e[i].y += baseEnemySpeed * 0.5f;
                    } else {
                        e[i].x += e[i].dx;
                        if (e[i].x < 0) { e[i].x = 0; e[i].dx = -e[i].dx; }
                        if (e[i].x > W - 40) { e[i].x = W - 40; e[i].dx = -e[i].dx; }
                    }
                }
                if (e[i].y > H) e[i].active = 0.0f;
                
                if (e[i].type == 2.0f && (frameCount % 60 == 0) && (rnd() % 2 == 0)) {
                    for (int j = 0; j < MAX_EBULLETS; j++) {
                        if (!eb[j].active) {
                            eb[j].active = 1.0f;
                            eb[j].x = e[i].x + 8.0f;
                            eb[j].y = e[i].y + 20.0f;
                            eb[j].dy = 4.0f;
                            eb[j].dx = 0.0f;
                            break;
                        }
                    }
                }
                
                if (e[i].type == 6.0f && (frameCount % 45 == 0)) {
                    float bdx[] = {-2.0f, 0.0f, 2.0f};
                    int bspawned = 0;
                    for (int j = 0; j < MAX_EBULLETS; j++) {
                        if (!eb[j].active) {
                            eb[j].active = 1.0f;
                            eb[j].x = e[i].x + 20.0f;
                            eb[j].y = e[i].y + 40.0f;
                            eb[j].dy = 4.0f;
                            eb[j].dx = bdx[bspawned];
                            bspawned++;
                            if (bspawned >= 3) break;
                        }
                    }
                }
            }
            
            // Player collision
            if (p.x < e[i].x + ew && p.x + 20 > e[i].x && p.y < e[i].y + eh && p.y + 20 > e[i].y) {
                if (shieldActive) {
                    shieldActive = 0;
                    AddExplosion(e[i].x + ew/2.0f, e[i].y + eh/2.0f, 16, RGB(0, 229, 255));
                    e[i].active = 0.0f;
                    PlaySnd(1);
                } else {
                    AddExplosion(p.x + 10.0f, p.y + 10.0f, 30, RGB(255, 23, 68));
                    gameOver = 1;
                    SaveHighScore();
                    PlaySnd(1);
                }
            }
            
            // Bullet collision
            for (int j = 0; j < MAX_BULLETS; j++) {
                if (b[j].active && b[j].x < e[i].x + ew && b[j].x + 4 > e[i].x && b[j].y < e[i].y + eh && b[j].y + 10 > e[i].y) {
                    b[j].active = 0.0f;
                    e[i].hp--;
                    AddExplosion(b[j].x, b[j].y, 3, RGB(0, 229, 255));
                    if (e[i].hp <= 0) {
                        e[i].active = 0.0f;
                        AddExplosion(e[i].x + ew/2.0f, e[i].y + eh/2.0f, (e[i].type == 6.0f) ? 30 : 12, RGB(255, 152, 0));
                        score += (e[i].type == 2.0f) ? 50 : (e[i].type == 3.0f ? 100 : (e[i].type == 4.0f ? 60 : 10));
                        if (e[i].type == 5.0f) score += 100;
                        if (e[i].type == 6.0f) score += 500;
                        enemiesKilled++;
                        totalKills++;
                        PlaySnd(1);
                        if (score > highScore) {
                            highScore = score;
                            SaveHighScore();
                        }
                        if ((rnd() % 100) < 15) {
                            for(int k=0; k<MAX_POWERUPS; k++) {
                                if(!pu[k].active) {
                                    pu[k].active = 1.0f; pu[k].x = e[i].x; pu[k].y = e[i].y; pu[k].dy = 2.0f;
                                    pu[k].type = (float)(rnd() % 5);
                                    break;
                                }
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
    
    for (int i = 0; i < MAX_EBULLETS; i++) {
        if (eb[i].active) {
            if (timeStopTimer == 0) {
                eb[i].y += eb[i].dy;
                eb[i].x += eb[i].dx;
            }
            if (eb[i].y > H || eb[i].x < 0 || eb[i].x > W) eb[i].active = 0.0f;
            if (p.x < eb[i].x + 4 && p.x + 20 > eb[i].x && p.y < eb[i].y + 10 && p.y + 20 > eb[i].y) {
                eb[i].active = 0.0f;
                if (shieldActive) {
                    shieldActive = 0;
                    AddExplosion(p.x + 10.0f, p.y + 10.0f, 10, RGB(0, 229, 255));
                    PlaySnd(1);
                } else {
                    AddExplosion(p.x + 10.0f, p.y + 10.0f, 30, RGB(255, 23, 68));
                    gameOver = 1;
                    SaveHighScore();
                    PlaySnd(1);
                }
            }
        }
    }
    
    for (int i=0; i<MAX_POWERUPS; i++) {
        if (pu[i].active) {
            pu[i].y += pu[i].dy;
            if (pu[i].y > H) pu[i].active = 0.0f;
            if (p.x < pu[i].x + 15 && p.x + 20 > pu[i].x && p.y < pu[i].y + 15 && p.y + 20 > pu[i].y) {
                pu[i].active = 0.0f;
                if (pu[i].type == 0.0f) spreadTimer = 300;
                else if (pu[i].type == 1.0f) shieldActive = 1;
                else if (pu[i].type == 2.0f) rapidTimer = 300;
                else if (pu[i].type == 3.0f) {
                    for(int j=0; j<MAX_ENEMIES; j++) {
                        if(e[j].active && e[j].type != 5.0f && e[j].type != 6.0f) {
                            e[j].active = 0.0f;
                            AddExplosion(e[j].x + 10.0f, e[j].y + 10.0f, 10, RGB(255, 235, 59));
                            score += 10;
                            enemiesKilled++;
                            totalKills++;
                        }
                    }
                    PlaySnd(1);
                }
                else if (pu[i].type == 4.0f) timeStopTimer = 300;
                AddExplosion(pu[i].x + 7.0f, pu[i].y + 7.0f, 10, RGB(0, 229, 255));
                score += 50;
                PlaySnd(2);
            }
        }
    }
    if (spreadTimer > 0) spreadTimer--;
    if (rapidTimer > 0) rapidTimer--;
    if (timeStopTimer > 0) timeStopTimer--;
    
    if (frameCount % spawnRate == 0) SpawnEnemy();
    frameCount++;
}

void DrawPlayerShipGDI(HDC hdc, int x, int y, int shield, int frame) {
    HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);

    // Thruster flame
    int flameH = 4 + (frame % 3) * 3;
    COLORREF flameCol = (frame % 2 == 0) ? RGB(255, 200, 0) : RGB(255, 60, 0);
    HBRUSH fbr = CreateSolidBrush(flameCol);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, fbr);
    POINT flamePts[3] = { {x + 6, y + 20}, {x + 10, y + 20 + flameH}, {x + 14, y + 20} };
    Polygon(hdc, flamePts, 3);
    SelectObject(hdc, oldBr);
    DeleteObject(fbr);

    // Wings
    HBRUSH wbr = CreateSolidBrush(RGB(30, 136, 229));
    SelectObject(hdc, wbr);
    POINT wingPts[6] = { {x + 10, y}, {x + 20, y + 16}, {x + 16, y + 20}, {x + 10, y + 15}, {x + 4, y + 20}, {x + 0, y + 16} };
    Polygon(hdc, wingPts, 6);
    SelectObject(hdc, oldBr);
    DeleteObject(wbr);

    // Fuselage / body
    HBRUSH bbr = CreateSolidBrush(RGB(100, 181, 246));
    SelectObject(hdc, bbr);
    POINT bodyPts[4] = { {x + 10, y + 1}, {x + 14, y + 12}, {x + 10, y + 18}, {x + 6, y + 12} };
    Polygon(hdc, bodyPts, 4);
    SelectObject(hdc, oldBr);
    DeleteObject(bbr);

    // Cockpit
    HBRUSH cbr = CreateSolidBrush(RGB(0, 229, 255));
    SelectObject(hdc, cbr);
    Ellipse(hdc, x + 7, y + 4, x + 13, y + 12);
    SelectObject(hdc, oldBr);
    DeleteObject(cbr);

    // Shield
    if (shield) {
        HPEN spen = CreatePen(PS_SOLID, 2, RGB(0, 229, 255));
        SelectObject(hdc, spen);
        HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
        SelectObject(hdc, nullBr);
        Ellipse(hdc, x - 4, y - 4, x + 24, y + 24);
        SelectObject(hdc, oldBr);
        DeleteObject(spen);
    }
    SelectObject(hdc, oldPen);
}

void DrawEnemyShipGDI(HDC hdc, float fx, float fy, float ftype, int frame) {
    int x = (int)fx;
    int y = (int)fy;
    int type = (int)ftype;

    HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);

    if (type == 0) { // Scout (Red Interceptor)
        HBRUSH br = CreateSolidBrush(RGB(229, 57, 53));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
        POINT pts[6] = { {x, y + 2}, {x + 10, y + 18}, {x + 20, y + 2}, {x + 13, y + 6}, {x + 10, y}, {x + 7, y + 6} };
        Polygon(hdc, pts, 6);
        HBRUSH cbr = CreateSolidBrush(RGB(255, 235, 59));
        SelectObject(hdc, cbr);
        Ellipse(hdc, x + 7, y + 7, x + 13, y + 13);
        SelectObject(hdc, oldBr);
        DeleteObject(br);
        DeleteObject(cbr);
    } else if (type == 1) { // Chaser (Orange Predator)
        HBRUSH br = CreateSolidBrush(RGB(255, 152, 0));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
        POINT pts[4] = { {x + 10, y + 20}, {x + 20, y + 4}, {x + 10, y}, {x, y + 4} };
        Polygon(hdc, pts, 4);
        HBRUSH coreBr = CreateSolidBrush(RGB(255, 61, 0));
        SelectObject(hdc, coreBr);
        RECT rc = {x + 7, y + 4, x + 13, y + 14};
        FillRect(hdc, &rc, coreBr);
        SelectObject(hdc, oldBr);
        DeleteObject(br);
        DeleteObject(coreBr);
    } else if (type == 2) { // Shooter (Purple Saucer)
        HBRUSH br = CreateSolidBrush(RGB(142, 36, 170));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
        Ellipse(hdc, x, y + 6, x + 20, y + 18);
        HBRUSH cbr = CreateSolidBrush(RGB(224, 64, 251));
        SelectObject(hdc, cbr);
        Ellipse(hdc, x + 5, y + 4, x + 15, y + 12);
        SelectObject(hdc, oldBr);
        DeleteObject(br);
        DeleteObject(cbr);
    } else if (type == 3) { // Armored (Grey Heavy)
        HBRUSH br = CreateSolidBrush(RGB(120, 144, 156));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
        RECT rc = {x + 2, y + 2, x + 18, y + 18};
        FillRect(hdc, &rc, br);
        HBRUSH cbr = CreateSolidBrush(RGB(55, 71, 79));
        RECT rc2 = {x + 5, y + 5, x + 15, y + 15};
        FillRect(hdc, &rc2, cbr);
        HBRUSH pbr = CreateSolidBrush(RGB(66, 165, 245));
        RECT rc3 = {x + 8, y, x + 12, y + 20};
        FillRect(hdc, &rc3, pbr);
        SelectObject(hdc, oldBr);
        DeleteObject(br); DeleteObject(cbr); DeleteObject(pbr);
    } else if (type == 4) { // Zigzag (Cyan Swift)
        HBRUSH br = CreateSolidBrush(RGB(0, 188, 212));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
        POINT pts[4] = { {x + 10, y + 18}, {x + 20, y}, {x + 10, y + 6}, {x, y} };
        Polygon(hdc, pts, 4);
        SelectObject(hdc, oldBr);
        DeleteObject(br);
    } else if (type == 5) { // Asteroid (Brown Rock)
        HBRUSH br = CreateSolidBrush(RGB(109, 76, 65));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
        POINT pts[7] = { {x + 2, y + 6}, {x + 8, y + 1}, {x + 16, y + 3}, {x + 19, y + 11}, {x + 15, y + 18}, {x + 6, y + 19}, {x + 1, y + 13} };
        Polygon(hdc, pts, 7);
        HBRUSH crBr = CreateSolidBrush(RGB(78, 52, 46));
        SelectObject(hdc, crBr);
        Ellipse(hdc, x + 8, y + 9, x + 14, y + 15);
        SelectObject(hdc, oldBr);
        DeleteObject(br); DeleteObject(crBr);
    } else if (type == 6) { // Boss Dreadnought (40x40)
        HBRUSH br = CreateSolidBrush(RGB(183, 28, 28));
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
        POINT pts[6] = { {x + 20, y + 38}, {x + 38, y + 10}, {x + 32, y + 2}, {x + 20, y + 12}, {x + 8, y + 2}, {x + 2, y + 10} };
        Polygon(hdc, pts, 6);
        COLORREF eyeCol = (frame % 10 < 5) ? RGB(255, 235, 59) : RGB(255, 23, 68);
        HBRUSH eyeBr = CreateSolidBrush(eyeCol);
        SelectObject(hdc, eyeBr);
        Ellipse(hdc, x + 13, y + 13, x + 27, y + 27);
        HBRUSH gunBr = CreateSolidBrush(RGB(66, 66, 66));
        RECT g1 = {x + 4, y + 24, x + 8, y + 36};
        RECT g2 = {x + 32, y + 24, x + 36, y + 36};
        FillRect(hdc, &g1, gunBr);
        FillRect(hdc, &g2, gunBr);
        SelectObject(hdc, oldBr);
        DeleteObject(br); DeleteObject(eyeBr); DeleteObject(gunBr);
    }

    SelectObject(hdc, oldPen);
}

void DrawPowerupGDI(HDC hdc, float fx, float fy, float ftype) {
    int x = (int)fx;
    int y = (int)fy;
    int type = (int)ftype;
    COLORREF cols[5] = { RGB(0, 230, 118), RGB(0, 229, 255), RGB(255, 255, 255), RGB(255, 234, 0), RGB(124, 77, 255) };
    COLORREF c = (type >= 0 && type < 5) ? cols[type] : RGB(255, 255, 255);
    HBRUSH br = CreateSolidBrush(c);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    Ellipse(hdc, x, y, x + 15, y + 15);
    SelectObject(hdc, oldBr);
    SelectObject(hdc, oldPen);
    DeleteObject(br);
    DeleteObject(pen);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            seed = GetTickCount();
            LoadHighScore();
            for(int i=0; i<MAX_STARS; i++) {
                stars[i].x = (float)(rnd() % W);
                stars[i].y = (float)(rnd() % H);
                stars[i].speed = 1.0f + (rnd() % 3);
            }
            SetTimer(hwnd, 1, 16, NULL); // ~60 FPS
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
            
            int bgR = 10 + (wave * 5) % 40;
            int bgG = 10 + (wave * 10) % 40;
            int bgB = 30 + (wave * 15) % 60;
            HBRUSH bg = CreateSolidBrush(RGB(bgR, bgG, bgB));
            RECT rc = {0, 0, W, H};
            FillRect(memDC, &rc, bg);
            DeleteObject(bg);
            
            // Draw stars
            HBRUSH sbr = CreateSolidBrush(RGB(200, 200, 200));
            for (int i=0; i<MAX_STARS; i++) {
                RECT sr = {(int)stars[i].x, (int)stars[i].y, (int)stars[i].x + 2, (int)stars[i].y + 2};
                FillRect(memDC, &sr, sbr);
            }
            DeleteObject(sbr);
            
            // Draw player
            DrawPlayerShipGDI(memDC, (int)p.x, (int)p.y, shieldActive, frameCount);
            
            // Draw bullets
            HBRUSH bbr = CreateSolidBrush(RGB(0, 229, 255));
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (b[i].active) {
                    RECT br = {(int)b[i].x, (int)b[i].y, (int)b[i].x + 4, (int)b[i].y + 10};
                    FillRect(memDC, &br, bbr);
                }
            }
            DeleteObject(bbr);
            
            // Draw enemy bullets
            HBRUSH ebbr = CreateSolidBrush(RGB(255, 23, 68));
            for (int i = 0; i < MAX_EBULLETS; i++) {
                if (eb[i].active) {
                    RECT br = {(int)eb[i].x, (int)eb[i].y, (int)eb[i].x + 4, (int)eb[i].y + 10};
                    FillRect(memDC, &br, ebbr);
                }
            }
            DeleteObject(ebbr);
            
            // Draw enemies
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (e[i].active) {
                    DrawEnemyShipGDI(memDC, e[i].x, e[i].y, e[i].type, frameCount);
                }
            }
            
            // Draw powerups
            for (int i = 0; i < MAX_POWERUPS; i++) {
                if (pu[i].active) {
                    DrawPowerupGDI(memDC, pu[i].x, pu[i].y, pu[i].type);
                }
            }

            // Draw particles / explosions
            DrawParticles(memDC);
            
            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(255, 255, 255));
            char scoreStr[64];
            wsprintfA(scoreStr, "Score: %d  High: %d", score, highScore);
            TextOutA(memDC, 10, 10, scoreStr, lstrlenA(scoreStr));
            char waveStr[64];
            wsprintfA(waveStr, "Wave: %d  Kills: %d", wave, enemiesKilled);
            TextOutA(memDC, 10, 28, waveStr, lstrlenA(waveStr));
            char totalStr[64];
            wsprintfA(totalStr, "Life Kills: %d", totalKills);
            TextOutA(memDC, W - 120, 10, totalStr, lstrlenA(totalStr));
            
            if (gameOver) {
                SetTextColor(memDC, RGB(255, 0, 0));
                TextOutA(memDC, W/2 - 40, H/2 - 10, "GAME OVER", 9);
                TextOutA(memDC, W/2 - 75, H/2 + 10, "Press Enter to Restart", 22);
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

