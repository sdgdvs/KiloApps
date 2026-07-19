#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define TIMER_ID 1
#define WIDTH 800
#define HEIGHT 600

const char g_szClassName[] = "KAsteroidsClass";

typedef struct {
    float x, y;
    float vx, vy;
    float angle;
    bool active;
    float radius;
} Ship;

typedef struct {
    float x, y;
    float vx, vy;
    float radius;
    int level;
    bool active;
    float points[10];
} Asteroid;

typedef struct {
    float x, y;
    float vx, vy;
    int life;
    bool active;
    bool is_enemy;
} Bullet;

typedef struct {
    float x, y;
    float vx, vy;
    float radius;
    bool active;
    int shoot_timer;
} Ufo;

typedef struct {
    float x, y;
    float vx, vy;
    int life;
    COLORREF color;
    bool active;
} Particle;

Ship ship;
Asteroid asteroids[100];
int num_asteroids = 0;
Bullet bullets[50];
int num_bullets = 0;
Ufo ufos[10];
int num_ufos = 0;
Particle particles[500];
int num_particles = 0;
int ufo_spawn_timer = 0;
int score = 0;
int high_score = 0;
int wave = 1;
bool game_over = true;
bool keys[256] = {0};
int game_mode = 0; // 0=Classic, 1=TimeAttack, 2=Hardcore
int time_left = 0;
long long last_tick_time = 0;

DWORD WINAPI SoundThread(LPVOID param) {
    int type = (int)(intptr_t)param;
    if (type == 1) { // Laser
        Beep(880, 50);
    } else if (type == 2) { // Explosion
        Beep(100, 150);
    } else if (type == 3) { // Thrust
        Beep(50, 50);
    }
    return 0;
}

void PlaySoundEffect(int type) {
    CreateThread(NULL, 0, SoundThread, (LPVOID)(intptr_t)type, 0, NULL);
}

void LoadHighScore() {
    FILE* f = fopen("kasteroids_highscore.txt", "r");
    if (f) {
        fscanf(f, "%d", &high_score);
        fclose(f);
    }
}
void SaveHighScore() {
    FILE* f = fopen("kasteroids_highscore.txt", "w");
    if (f) {
        fprintf(f, "%d", high_score);
        fclose(f);
    }
}

void UpdateHighScore() {
    if (score > high_score) {
        high_score = score;
        SaveHighScore();
    }
}
long long last_shoot_time = 0;

long long GetTimeMs() {
    return GetTickCount64();
}

void SpawnAsteroids(int count) {
    float speedMult = 1.0f + (wave - 1) * 0.15f;
    if (game_mode == 2) speedMult *= 1.5f;
    for (int i = 0; i < count; i++) {
        if (num_asteroids >= 100) break;
        float x, y;
        do {
            x = (rand() % WIDTH);
            y = (rand() % HEIGHT);
        } while (sqrt(pow(x - ship.x, 2) + pow(y - ship.y, 2)) < 100);
        
        Asteroid* a = &asteroids[num_asteroids++];
        a->x = x;
        a->y = y;
        a->radius = 40.0f;
        a->level = 3;
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = ((rand() % 200 + 100) / 100.0f) * speedMult;
        a->vx = cos(angle) * speed;
        a->vy = sin(angle) * speed;
        a->active = true;
        for (int j = 0; j < 10; j++) {
            a->points[j] = a->radius + (rand() % (int)(a->radius * 0.4f)) - a->radius * 0.2f;
        }
    }
}

void InitGame(int mode) {
    game_mode = mode;
    score = 0;
    wave = 1;
    time_left = (game_mode == 1) ? 120 : 0;
    last_tick_time = GetTimeMs();
    ship.x = WIDTH / 2.0f;
    ship.y = HEIGHT / 2.0f;
    ship.vx = 0;
    ship.vy = 0;
    ship.angle = -3.14159f / 2.0f;
    ship.active = true;
    ship.radius = 10.0f;
    
    num_asteroids = 0;
    num_bullets = 0;
    num_ufos = 0;
    num_particles = 0;
    ufo_spawn_timer = 0;
    game_over = false;
    
    SpawnAsteroids(3 + wave);
}

void CheckCollisions() {
    for (int i = 0; i < num_bullets; i++) {
        if (!bullets[i].active) continue;
        
        if (bullets[i].is_enemy && ship.active) {
            float dist = sqrt(pow(bullets[i].x - ship.x, 2) + pow(bullets[i].y - ship.y, 2));
            if (dist < ship.radius) {
                ship.active = false;
                game_over = true;
                bullets[i].active = false;
                PlaySoundEffect(2);
                for(int p=0; p<40; p++) {
                    if(num_particles >= 500) break;
                    Particle* pt = &particles[num_particles++];
                    pt->x = ship.x; pt->y = ship.y;
                    pt->vx = ((rand()%100)-50)/10.0f; pt->vy = ((rand()%100)-50)/10.0f;
                    pt->life = 40 + (rand()%30); pt->color = RGB(226, 232, 240); pt->active = true;
                }
            }
        }
        if (!bullets[i].active) continue;

        for (int j = 0; j < num_asteroids; j++) {
            if (!asteroids[j].active) continue;
            float dist = sqrt(pow(bullets[i].x - asteroids[j].x, 2) + pow(bullets[i].y - asteroids[j].y, 2));
            if (dist < asteroids[j].radius) {
                bullets[i].active = false;
                asteroids[j].active = false;
                PlaySoundEffect(2);
                for(int p=0; p<20; p++) {
                    if(num_particles >= 500) break;
                    Particle* pt = &particles[num_particles++];
                    pt->x = asteroids[j].x; pt->y = asteroids[j].y;
                    pt->vx = ((rand()%100)-50)/12.0f; pt->vy = ((rand()%100)-50)/12.0f;
                    pt->life = 30 + (rand()%20); pt->color = RGB(148, 163, 184); pt->active = true;
                }
                if (!bullets[i].is_enemy) {
                    score += (4 - asteroids[j].level) * 10;
                    UpdateHighScore();
                }
                
                if (asteroids[j].level > 1 && num_asteroids + 2 <= 100) {
                    for (int k = 0; k < 2; k++) {
                        Asteroid* a = &asteroids[num_asteroids++];
                        a->x = asteroids[j].x;
                        a->y = asteroids[j].y;
                        a->radius = asteroids[j].radius / 2.0f;
                        a->level = asteroids[j].level - 1;
                        float angle = (rand() % 360) * 3.14159f / 180.0f;
                        float speedMult = 1.0f + (wave - 1) * 0.15f;
                        float speed = ((rand() % 200 + 100) / 100.0f) * speedMult;
                        a->vx = cos(angle) * speed;
                        a->vy = sin(angle) * speed;
                        a->active = true;
                        for (int p = 0; p < 10; p++) {
                            a->points[p] = a->radius + (rand() % (int)(a->radius * 0.4f)) - a->radius * 0.2f;
                        }
                    }
                }
                break;
            }
        }
        
        if (!bullets[i].active) continue;
        if (!bullets[i].is_enemy) {
            for (int k = 0; k < num_ufos; k++) {
                if (!ufos[k].active) continue;
                float dist = sqrt(pow(bullets[i].x - ufos[k].x, 2) + pow(bullets[i].y - ufos[k].y, 2));
                if (dist < ufos[k].radius) {
                    bullets[i].active = false;
                    ufos[k].active = false;
                    PlaySoundEffect(2);
                    for(int p=0; p<20; p++) {
                        if(num_particles >= 500) break;
                        Particle* pt = &particles[num_particles++];
                        pt->x = ufos[k].x; pt->y = ufos[k].y;
                        pt->vx = ((rand()%100)-50)/12.0f; pt->vy = ((rand()%100)-50)/12.0f;
                        pt->life = 30 + (rand()%20); pt->color = RGB(239, 68, 68); pt->active = true;
                    }
                    score += 200;
                    UpdateHighScore();
                    break;
                }
            }
        }
    }
    
    if (ship.active) {
        for (int j = 0; j < num_asteroids; j++) {
            if (!asteroids[j].active) continue;
            float dist = sqrt(pow(ship.x - asteroids[j].x, 2) + pow(ship.y - asteroids[j].y, 2));
            if (dist < asteroids[j].radius + ship.radius) {
                ship.active = false;
                game_over = true;
                PlaySoundEffect(2);
                for(int p=0; p<40; p++) {
                    if(num_particles >= 500) break;
                    Particle* pt = &particles[num_particles++];
                    pt->x = ship.x; pt->y = ship.y;
                    pt->vx = ((rand()%100)-50)/10.0f; pt->vy = ((rand()%100)-50)/10.0f;
                    pt->life = 40 + (rand()%30); pt->color = RGB(226, 232, 240); pt->active = true;
                }
            }
        }
        for (int k = 0; k < num_ufos; k++) {
            if (!ufos[k].active) continue;
            float dist = sqrt(pow(ship.x - ufos[k].x, 2) + pow(ship.y - ufos[k].y, 2));
            if (dist < ufos[k].radius + ship.radius) {
                ship.active = false;
                game_over = true;
                PlaySoundEffect(2);
                for(int p=0; p<40; p++) {
                    if(num_particles >= 500) break;
                    Particle* pt = &particles[num_particles++];
                    pt->x = ship.x; pt->y = ship.y;
                    pt->vx = ((rand()%100)-50)/10.0f; pt->vy = ((rand()%100)-50)/10.0f;
                    pt->life = 40 + (rand()%30); pt->color = RGB(226, 232, 240); pt->active = true;
                }
            }
        }
    }
}

void CompactArrays() {
    int active_bullets = 0;
    for (int i = 0; i < num_bullets; i++) {
        if (bullets[i].active) {
            bullets[active_bullets++] = bullets[i];
        }
    }
    num_bullets = active_bullets;
    
    int active_asteroids = 0;
    for (int i = 0; i < num_asteroids; i++) {
        if (asteroids[i].active) {
            asteroids[active_asteroids++] = asteroids[i];
        }
    }
    num_asteroids = active_asteroids;
    
    int active_ufos = 0;
    for (int i = 0; i < num_ufos; i++) {
        if (ufos[i].active) {
            ufos[active_ufos++] = ufos[i];
        }
    }
    num_ufos = active_ufos;
    
    int active_particles = 0;
    for (int i = 0; i < num_particles; i++) {
        if (particles[i].active) {
            particles[active_particles++] = particles[i];
        }
    }
    num_particles = active_particles;
}

void Update() {
    if (game_over) {
        if (keys['1']) InitGame(0);
        if (keys['2']) InitGame(1);
        if (keys['3']) InitGame(2);
        return;
    }
    
    if (game_mode == 1) {
        long long now = GetTimeMs();
        if (now - last_tick_time >= 1000) {
            time_left--;
            last_tick_time = now;
            if (time_left <= 0) {
                ship.active = false;
                game_over = true;
                PlaySoundEffect(2);
                for(int p=0; p<40; p++) {
                    if(num_particles >= 500) break;
                    Particle* pt = &particles[num_particles++];
                    pt->x = ship.x; pt->y = ship.y;
                    pt->vx = ((rand()%100)-50)/10.0f; pt->vy = ((rand()%100)-50)/10.0f;
                    pt->life = 40 + (rand()%30); pt->color = RGB(226, 232, 240); pt->active = true;
                }
            }
        }
    }

    if (keys[VK_LEFT]) ship.angle -= 0.05f;
    if (keys[VK_RIGHT]) ship.angle += 0.05f;
    if (keys[VK_UP]) {
        ship.vx += cos(ship.angle) * 0.1f;
        ship.vy += sin(ship.angle) * 0.1f;
        static int thrust_timer = 0;
        thrust_timer++;
        if (thrust_timer > 3) {
            PlaySoundEffect(3);
            thrust_timer = 0;
        }
        if (num_particles < 500 && ship.active) {
            Particle* p = &particles[num_particles++];
            p->x = ship.x - cos(ship.angle) * 15.0f;
            p->y = ship.y - sin(ship.angle) * 15.0f;
            p->vx = -cos(ship.angle) * 3.0f + ((rand() % 100) - 50) / 33.0f;
            p->vy = -sin(ship.angle) * 3.0f + ((rand() % 100) - 50) / 33.0f;
            p->life = 15 + (rand() % 10);
            p->color = RGB(249, 115, 22);
            p->active = true;
        }
    }
    
    ship.vx *= 0.99f;
    ship.vy *= 0.99f;
    
    ship.x += ship.vx;
    ship.y += ship.vy;
    
    if (ship.x < 0) ship.x = WIDTH;
    if (ship.x > WIDTH) ship.x = 0;
    if (ship.y < 0) ship.y = HEIGHT;
    if (ship.y > HEIGHT) ship.y = 0;
    
    if (keys[VK_SPACE]) {
        long long now = GetTimeMs();
        if (now - last_shoot_time > 200 && num_bullets < 50) {
            Bullet* b = &bullets[num_bullets++];
            b->x = ship.x + cos(ship.angle) * 15.0f;
            b->y = ship.y + sin(ship.angle) * 15.0f;
            b->vx = cos(ship.angle) * 7.0f;
            b->vy = sin(ship.angle) * 7.0f;
            b->life = 60;
            b->active = true;
            b->is_enemy = false;
            last_shoot_time = now;
            PlaySoundEffect(1);
        }
    }
    
    for (int i = 0; i < num_bullets; i++) {
        if (!bullets[i].active) continue;
        bullets[i].x += bullets[i].vx;
        bullets[i].y += bullets[i].vy;
        bullets[i].life--;
        if (bullets[i].life <= 0) bullets[i].active = false;
        
        if (bullets[i].x < 0) bullets[i].x = WIDTH;
        if (bullets[i].x > WIDTH) bullets[i].x = 0;
        if (bullets[i].y < 0) bullets[i].y = HEIGHT;
        if (bullets[i].y > HEIGHT) bullets[i].y = 0;
    }
    
    int active_asteroids = 0;
    for (int i = 0; i < num_asteroids; i++) {
        if (!asteroids[i].active) continue;
        active_asteroids++;
        asteroids[i].x += asteroids[i].vx;
        asteroids[i].y += asteroids[i].vy;
        
        if (asteroids[i].x < -asteroids[i].radius) asteroids[i].x = WIDTH + asteroids[i].radius;
        if (asteroids[i].x > WIDTH + asteroids[i].radius) asteroids[i].x = -asteroids[i].radius;
        if (asteroids[i].y < -asteroids[i].radius) asteroids[i].y = HEIGHT + asteroids[i].radius;
        if (asteroids[i].y > HEIGHT + asteroids[i].radius) asteroids[i].y = -asteroids[i].radius;
    }
    
    ufo_spawn_timer++;
    int ufo_threshold = (game_mode == 2) ? 300 : 600;
    if (ufo_spawn_timer > ufo_threshold) {
        ufo_spawn_timer = 0;
        if ((rand() % 100) < (int)((0.5f + (wave * 0.05f)) * 100) && num_ufos < 10) {
            Ufo* u = &ufos[num_ufos++];
            u->y = 50.0f + (rand() % (HEIGHT - 100));
            u->x = (rand() % 2 == 0) ? 0.0f : (float)WIDTH;
            u->vx = (u->x == 0.0f) ? 2.0f : -2.0f;
            u->vy = ((rand() % 200) - 100) / 100.0f * 2.0f;
            u->radius = 15.0f;
            u->active = true;
            u->shoot_timer = 0;
        }
    }
    
    for (int i = 0; i < num_ufos; i++) {
        if (!ufos[i].active) continue;
        ufos[i].x += ufos[i].vx;
        ufos[i].y += ufos[i].vy;
        
        if (ufos[i].y < 0) ufos[i].y = HEIGHT;
        if (ufos[i].y > HEIGHT) ufos[i].y = 0;
        
        if ((ufos[i].vx > 0 && ufos[i].x > WIDTH + 50) ||
            (ufos[i].vx < 0 && ufos[i].x < -50)) {
            ufos[i].active = false;
        }
        
        ufos[i].shoot_timer++;
        if (ufos[i].shoot_timer > 100 && ship.active && num_bullets < 50) {
            ufos[i].shoot_timer = 0;
            float angle = atan2(ship.y - ufos[i].y, ship.x - ufos[i].x);
            angle += (((rand() % 100) - 50) / 100.0f) * 0.2f;
            
            Bullet* b = &bullets[num_bullets++];
            b->x = ufos[i].x;
            b->y = ufos[i].y;
            b->vx = cos(angle) * 7.0f;
            b->vy = sin(angle) * 7.0f;
            b->life = 60;
            b->active = true;
            b->is_enemy = true;
            PlaySoundEffect(1);
        }
    }
    
    CheckCollisions();
    
    for (int i = 0; i < num_particles; i++) {
        if (!particles[i].active) continue;
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].life--;
        if (particles[i].life <= 0) particles[i].active = false;
    }
    
    CompactArrays();
    
    if (active_asteroids == 0) {
        wave++;
        SpawnAsteroids(3 + wave);
    }
}

void Draw(HDC hdc) {
    HBRUSH bgBrush = CreateSolidBrush(RGB(24, 24, 27));
    RECT rect = {0, 0, WIDTH, HEIGHT};
    FillRect(hdc, &rect, bgBrush);
    DeleteObject(bgBrush);
    
    HPEN shipPen = CreatePen(PS_SOLID, 2, RGB(226, 232, 240));
    HPEN astPen = CreatePen(PS_SOLID, 2, RGB(148, 163, 184));
    HPEN bulletPen = CreatePen(PS_SOLID, 2, RGB(253, 224, 71));
    HBRUSH bulletBrush = CreateSolidBrush(RGB(253, 224, 71));
    
    if (ship.active) {
        SelectObject(hdc, shipPen);
        POINT pts[3];
        pts[0].x = (LONG)(ship.x + cos(ship.angle) * 15);
        pts[0].y = (LONG)(ship.y + sin(ship.angle) * 15);
        pts[1].x = (LONG)(ship.x + cos(ship.angle + 2.5f) * 15);
        pts[1].y = (LONG)(ship.y + sin(ship.angle + 2.5f) * 15);
        pts[2].x = (LONG)(ship.x + cos(ship.angle - 2.5f) * 15);
        pts[2].y = (LONG)(ship.y + sin(ship.angle - 2.5f) * 15);
        MoveToEx(hdc, pts[0].x, pts[0].y, NULL);
        LineTo(hdc, pts[1].x, pts[1].y);
        LineTo(hdc, pts[2].x, pts[2].y);
        LineTo(hdc, pts[0].x, pts[0].y);
    }
    
    SelectObject(hdc, astPen);
    for (int i = 0; i < num_asteroids; i++) {
        if (!asteroids[i].active) continue;
        POINT pts[10];
        for (int j = 0; j < 10; j++) {
            float a = (j / 10.0f) * 3.14159f * 2.0f;
            pts[j].x = (LONG)(asteroids[i].x + cos(a) * asteroids[i].points[j]);
            pts[j].y = (LONG)(asteroids[i].y + sin(a) * asteroids[i].points[j]);
        }
        MoveToEx(hdc, pts[0].x, pts[0].y, NULL);
        for (int j = 1; j < 10; j++) LineTo(hdc, pts[j].x, pts[j].y);
        LineTo(hdc, pts[0].x, pts[0].y);
    }
    
    HPEN ufoPen = CreatePen(PS_SOLID, 2, RGB(239, 68, 68));
    HPEN enemyBulletPen = CreatePen(PS_SOLID, 2, RGB(239, 68, 68));
    HBRUSH enemyBulletBrush = CreateSolidBrush(RGB(239, 68, 68));
    
    SelectObject(hdc, ufoPen);
    for (int i = 0; i < num_ufos; i++) {
        if (!ufos[i].active) continue;
        MoveToEx(hdc, (int)(ufos[i].x - 15), (int)ufos[i].y, NULL);
        LineTo(hdc, (int)(ufos[i].x + 15), (int)ufos[i].y);
        LineTo(hdc, (int)(ufos[i].x + 8), (int)(ufos[i].y - 8));
        LineTo(hdc, (int)(ufos[i].x - 8), (int)(ufos[i].y - 8));
        LineTo(hdc, (int)(ufos[i].x - 15), (int)ufos[i].y);
        LineTo(hdc, (int)(ufos[i].x - 8), (int)(ufos[i].y + 8));
        LineTo(hdc, (int)(ufos[i].x + 8), (int)(ufos[i].y + 8));
        LineTo(hdc, (int)(ufos[i].x + 15), (int)ufos[i].y);
    }
    
    for (int i = 0; i < num_bullets; i++) {
        if (!bullets[i].active) continue;
        if (bullets[i].is_enemy) {
            SelectObject(hdc, enemyBulletPen);
            SelectObject(hdc, enemyBulletBrush);
        } else {
            SelectObject(hdc, bulletPen);
            SelectObject(hdc, bulletBrush);
        }
        Ellipse(hdc, (int)(bullets[i].x - 2), (int)(bullets[i].y - 2), (int)(bullets[i].x + 2), (int)(bullets[i].y + 2));
    }
    
    for (int i = 0; i < num_particles; i++) {
        if (!particles[i].active) continue;
        HBRUSH pb = CreateSolidBrush(particles[i].color);
        HPEN pp = CreatePen(PS_SOLID, 1, particles[i].color);
        SelectObject(hdc, pb);
        SelectObject(hdc, pp);
        Ellipse(hdc, (int)(particles[i].x - 1), (int)(particles[i].y - 1), (int)(particles[i].x + 2), (int)(particles[i].y + 2));
        DeleteObject(pb);
        DeleteObject(pp);
    }
    
    SetTextColor(hdc, RGB(56, 189, 248));
    SetBkMode(hdc, TRANSPARENT);
    char scoreStr[128];
    const char* modeStr = (game_mode == 0) ? "Classic" : ((game_mode == 1) ? "Time Attack" : "Hardcore");
    if (game_mode == 1) {
        sprintf(scoreStr, "Score: %d   High: %d   Time: %d   Mode: %s", score, high_score, time_left, modeStr);
    } else {
        sprintf(scoreStr, "Score: %d   High: %d   Wave: %d   Mode: %s", score, high_score, wave, modeStr);
    }
    TextOutA(hdc, 10, 10, scoreStr, strlen(scoreStr));
    
    if (game_over) {
        SetTextColor(hdc, RGB(239, 68, 68));
        if (game_mode == 1 && time_left <= 0) {
            TextOutA(hdc, WIDTH / 2 - 40, HEIGHT / 2 - 60, "TIME UP!", 8);
        } else {
            TextOutA(hdc, WIDTH / 2 - 50, HEIGHT / 2 - 60, "K-ASTEROIDS", 11);
        }
        SetTextColor(hdc, RGB(161, 161, 170));
        TextOutA(hdc, WIDTH / 2 - 70, HEIGHT / 2 - 20, "Select Mode to Play:", 20);
        TextOutA(hdc, WIDTH / 2 - 80, HEIGHT / 2 + 10, "[1] Classic - Standard Waves", 28);
        TextOutA(hdc, WIDTH / 2 - 80, HEIGHT / 2 + 30, "[2] Time Attack - 2 Min Limit", 29);
        TextOutA(hdc, WIDTH / 2 - 80, HEIGHT / 2 + 50, "[3] Hardcore - Fast & Deadly", 28);
    }
    
    DeleteObject(shipPen);
    DeleteObject(astPen);
    DeleteObject(ufoPen);
    DeleteObject(bulletPen);
    DeleteObject(bulletBrush);
    DeleteObject(enemyBulletPen);
    DeleteObject(enemyBulletBrush);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
            srand((unsigned int)time(NULL));
            LoadHighScore();
            SetTimer(hwnd, TIMER_ID, 16, NULL); // ~60fps
            break;
        case WM_TIMER:
            Update();
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case WM_KEYDOWN:
            if (wParam < 256) keys[wParam] = true;
            break;
        case WM_KEYUP:
            if (wParam < 256) keys[wParam] = false;
            break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Double buffering
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT);
            SelectObject(memDC, memBitmap);
            
            Draw(memDC);
            
            BitBlt(hdc, 0, 0, WIDTH, HEIGHT, memDC, 0, 0, SRCCOPY);
            
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
        }
        break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    RECT clientRect = {0, 0, WIDTH, HEIGHT};
    AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "KAsteroids",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 
        clientRect.right - clientRect.left, clientRect.bottom - clientRect.top,
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}
