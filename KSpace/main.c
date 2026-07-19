#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int _fltused = 1;
long _ftol2_sse(float f) { return (long)f; }
long _ftol2(float f) { return (long)f; }

#define W 320
#define H 480
#define MAX_BULLETS 20
#define MAX_ENEMIES 10
#define MAX_STARS 50

typedef struct { float x, y, active, dx, dy, type; int hp; } Ent;
typedef struct { float x, y, speed; } Star;

Ent p = { W/2.0f, H - 50.0f, 1.0f, 0, 0, 0 };
Ent b[MAX_BULLETS] = {0};
Ent e[MAX_ENEMIES] = {0};
#define MAX_EBULLETS 20
Ent eb[MAX_EBULLETS] = {0};
#define MAX_POWERUPS 3
Ent pu[MAX_POWERUPS] = {0};
int spreadTimer = 0;
Star stars[MAX_STARS] = {0};
int score = 0;
int highScore = 0;
int gameOver = 0;
int frameCount = 0;
int shieldActive = 0;
int wave = 1;
int enemiesKilled = 0;
int rapidTimer = 0;

unsigned int seed = 999;
unsigned int rnd() {
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

void LoadHighScore() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\KSpace", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD type = REG_DWORD;
        DWORD size = sizeof(DWORD);
        RegQueryValueExA(hKey, "HighScore", NULL, &type, (LPBYTE)&highScore, &size);
        RegCloseKey(hKey);
    }
}

void SaveHighScore() {
    HKEY hKey;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\KSpace", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "HighScore", 0, REG_DWORD, (const BYTE*)&highScore, sizeof(DWORD));
        RegCloseKey(hKey);
    }
}

void SpawnEnemy() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!e[i].active) {
            e[i].active = 1.0f;
            e[i].x = (float)(rnd() % (W - 20));
            e[i].y = -20.0f;
            int t = rnd() % 4;
            e[i].type = (float)t;
            if (t == 3) e[i].hp = 10;
            else if (t == 2) e[i].hp = 5;
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
void PlaySnd(int type) { CreateThread(NULL, 0, SndThread, (LPVOID)(intptr_t)type, 0, NULL); }

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
            spreadTimer = 0;
            rapidTimer = 0;
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
    
    float baseEnemySpeed = 2.0f + (score / 200.0f);
    int spawnRate = 30 - (score / 100);
    if (spawnRate < 10) spawnRate = 10;
    
    float speed = 4.0f;
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) p.x -= speed;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) p.x += speed;
    if (GetAsyncKeyState(VK_UP) & 0x8000) p.y -= speed;
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) p.y += speed;
    
    if (p.x < 0) p.x = 0;
    if (p.x > W - 20) p.x = W - 20;
    if (p.y < 0) p.y = 0;
    if (p.y > H - 20) p.y = H - 20;
    
    if (GetAsyncKeyState(VK_SPACE) & 0x8001) { // simple debounce
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
            }
            if (e[i].y > H) e[i].active = 0.0f;
            
            if (e[i].type == 2.0f && (frameCount % 60 == 0) && (rnd() % 2 == 0)) {
                for (int j = 0; j < MAX_EBULLETS; j++) {
                    if (!eb[j].active) {
                        eb[j].active = 1.0f;
                        eb[j].x = e[i].x + 8.0f;
                        eb[j].y = e[i].y + 20.0f;
                        eb[j].dy = 4.0f;
                        break;
                    }
                }
            }
            
            // Player collision
            if (p.x < e[i].x + 20 && p.x + 20 > e[i].x && p.y < e[i].y + 20 && p.y + 20 > e[i].y) {
                if (shieldActive) {
                    shieldActive = 0;
                    e[i].active = 0.0f;
                    PlaySnd(1);
                } else {
                    gameOver = 1;
                    PlaySnd(1);
                }
            }
            
            // Bullet collision
            for (int j = 0; j < MAX_BULLETS; j++) {
                if (b[j].active && b[j].x < e[i].x + 20 && b[j].x + 4 > e[i].x && b[j].y < e[i].y + 20 && b[j].y + 10 > e[i].y) {
                    b[j].active = 0.0f;
                    e[i].hp--;
                    if (e[i].hp <= 0) {
                        e[i].active = 0.0f;
                        score += (e[i].type == 2.0f) ? 50 : (e[i].type == 3.0f ? 100 : 10);
                        enemiesKilled++;
                        PlaySnd(1);
                        if (score > highScore) {
                            highScore = score;
                            SaveHighScore();
                        }
                        if ((rnd() % 100) < 15) {
                            for(int k=0; k<MAX_POWERUPS; k++) {
                                if(!pu[k].active) {
                                    pu[k].active = 1.0f; pu[k].x = e[i].x; pu[k].y = e[i].y; pu[k].dy = 2.0f;
                                    pu[k].type = (float)(rnd() % 3);
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
            eb[i].y += eb[i].dy;
            if (eb[i].y > H) eb[i].active = 0.0f;
            if (p.x < eb[i].x + 4 && p.x + 20 > eb[i].x && p.y < eb[i].y + 10 && p.y + 20 > eb[i].y) {
                eb[i].active = 0.0f;
                if (shieldActive) {
                    shieldActive = 0;
                    PlaySnd(1);
                } else {
                    gameOver = 1;
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
                else rapidTimer = 300;
                score += 50;
                PlaySnd(2);
            }
        }
    }
    if (spreadTimer > 0) spreadTimer--;
    if (rapidTimer > 0) rapidTimer--;
    
    if (frameCount % spawnRate == 0) SpawnEnemy();
    frameCount++;
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
            HBRUSH pbr = CreateSolidBrush(RGB(100, 200, 255));
            RECT pr = {(int)p.x, (int)p.y, (int)p.x + 20, (int)p.y + 20};
            FillRect(memDC, &pr, pbr);
            DeleteObject(pbr);
            
            if (shieldActive) {
                HBRUSH sbr2 = CreateSolidBrush(RGB(50, 200, 255));
                RECT sr2 = {(int)p.x - 4, (int)p.y - 4, (int)p.x + 24, (int)p.y + 24};
                FrameRect(memDC, &sr2, sbr2);
                DeleteObject(sbr2);
            }
            
            // Draw bullets
            HBRUSH bbr = CreateSolidBrush(RGB(255, 255, 100));
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (b[i].active) {
                    RECT br = {(int)b[i].x, (int)b[i].y, (int)b[i].x + 4, (int)b[i].y + 10};
                    FillRect(memDC, &br, bbr);
                }
            }
            DeleteObject(bbr);
            
            // Draw enemy bullets
            HBRUSH ebbr = CreateSolidBrush(RGB(255, 100, 255));
            for (int i = 0; i < MAX_EBULLETS; i++) {
                if (eb[i].active) {
                    RECT br = {(int)eb[i].x, (int)eb[i].y, (int)eb[i].x + 4, (int)eb[i].y + 10};
                    FillRect(memDC, &br, ebbr);
                }
            }
            DeleteObject(ebbr);
            
            // Draw enemies
            HBRUSH ebr1 = CreateSolidBrush(RGB(255, 50, 50));
            HBRUSH ebr2 = CreateSolidBrush(RGB(255, 150, 50));
            HBRUSH ebr3 = CreateSolidBrush(RGB(160, 32, 240));
            HBRUSH ebr4 = CreateSolidBrush(RGB(150, 150, 150));
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (e[i].active) {
                    RECT er = {(int)e[i].x, (int)e[i].y, (int)e[i].x + 20, (int)e[i].y + 20};
                    HBRUSH br = e[i].type == 0.0f ? ebr1 : (e[i].type == 1.0f ? ebr2 : (e[i].type == 2.0f ? ebr3 : ebr4));
                    FillRect(memDC, &er, br);
                }
            }
            DeleteObject(ebr1);
            DeleteObject(ebr2);
            DeleteObject(ebr3);
            DeleteObject(ebr4);
            
            // Draw powerups
            HBRUSH pubr1 = CreateSolidBrush(RGB(50, 255, 50));
            HBRUSH pubr2 = CreateSolidBrush(RGB(50, 200, 255));
            HBRUSH pubr3 = CreateSolidBrush(RGB(255, 255, 255));
            for (int i = 0; i < MAX_POWERUPS; i++) {
                if (pu[i].active) {
                    RECT pr = {(int)pu[i].x, (int)pu[i].y, (int)pu[i].x + 15, (int)pu[i].y + 15};
                    HBRUSH br = pu[i].type == 0.0f ? pubr1 : (pu[i].type == 1.0f ? pubr2 : pubr3);
                    FillRect(memDC, &pr, br);
                }
            }
            DeleteObject(pubr1);
            DeleteObject(pubr2);
            DeleteObject(pubr3);
            
            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(255, 255, 255));
            char scoreStr[64];
            wsprintfA(scoreStr, "Score: %d  High: %d", score, highScore);
            TextOutA(memDC, 10, 10, scoreStr, lstrlenA(scoreStr));
            char waveStr[64];
            wsprintfA(waveStr, "Wave: %d  Kills: %d", wave, enemiesKilled);
            TextOutA(memDC, 10, 28, waveStr, lstrlenA(waveStr));
            
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
