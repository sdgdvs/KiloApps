#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int _fltused = 1;

#define W 400
#define H 450
#define TIMER_ID 1

#define ROWS 6
#define COLS 10
#define BR_W (W / COLS)
#define BR_H 16

#define MAX_TRAIL 6
typedef struct { float x, y; } TrailPoint;

#define MAX_BALLS 16
typedef struct {
    float x, y;
    float dx, dy;
    int active;
    int stuck;
    int stuck_offset;
    TrailPoint trail[MAX_TRAIL];
    int trail_head;
} Ball;
Ball balls[MAX_BALLS];

#define MAX_LASERS 12
typedef struct {
    float x, y;
    int active;
} Laser;
Laser lasers[MAX_LASERS];

#define MAX_PARTICLES 128
typedef struct {
    float x, y;
    float vx, vy;
    int life;
    COLORREF color;
    int type;
} Particle;
Particle particles[MAX_PARTICLES];

#define MAX_SHOCKWAVES 8
typedef struct {
    float x, y;
    float r;
    float max_r;
    float life;
    COLORREF color;
} Shockwave;
Shockwave shockwaves[MAX_SHOCKWAVES];
int screen_shake = 0;

#define MAX_METEORS 4
typedef struct { float x, y; int active; } Meteor;
Meteor meteors[MAX_METEORS];
int meteor_active = 0;

// Game State
int pad_x = W / 2 - 30;
int pad_w = 60;
int pad_h = 10;
int last_pad_x = W / 2 - 30;
int pad_vx = 0;
int pad_squash_timer = 0;

int score = 0;
int high_score = 0;
int lifetime_bricks = 0;
int state = 0; // 0=start, 1=play, 2=gameover, 3=victory
int diff = 0; // 0=Easy, 1=Hard
float speed = 3.5f;
int lives = 3;
int level = 1;

// Powerup Drop
int power_active = 0;
float power_x = 0, power_y = 0;
int power_type = 0;
int paddle_timer = 0;
int sticky_timer = 0;
static int frame_counter = 0;

// Active Skills
int cd_laser = 0, dur_laser = 0;
int cd_multi = 0;
int cd_fire = 0, dur_fire = 0;
int cd_barrier = 0, dur_barrier = 0;

// Hazard Shield
int shield_active = 0;
float shield_x = 100, shield_y = 120.0f, shield_dx = 2;
int shield_w = 70;

// UFO Drone
int ufo_active = 0;
float ufo_x = 0, ufo_dx = 2;
int ufo_y = 25;
int ufo_timer = 200;
int ufo_bullet_active = 0;
float ufo_bullet_x = 0, ufo_bullet_y = 0;

// Boss Fortress (Stage 10, 20 & 30)
int boss_active = 0;
int boss_type = 1;
int boss_dx = 0;
int boss_hp = 50, boss_max_hp = 50;
int boss_x = W / 2 - 50, boss_y = 35, boss_w = 100, boss_h = 40;
float boss_shield_angle = 0.0f;
#define MAX_BOSS_BULLETS 4
typedef struct { float x, y; int active; } BossBullet;
BossBullet boss_bullets[MAX_BOSS_BULLETS];

// Bricks
int bricks[ROWS][COLS];
int brick_hp[ROWS][COLS];
int bricks_left = 0;

static unsigned int rng_seed = 1337;
static int MyRand() {
    rng_seed = rng_seed * 1103515245 + 12345;
    return (int)(rng_seed & 0x7fffffff);
}

static int MyAbs(int x) { return x < 0 ? -x : x; }

static float MySin(float x) {
    const float PI2 = 6.283185f;
    while (x < 0) x += PI2;
    while (x >= PI2) x -= PI2;
    float t = x * 4.0f / PI2;
    if (t < 1.0f) return t;
    if (t < 3.0f) return 2.0f - t;
    return t - 4.0f;
}
static float MyCos(float x) { return MySin(x + 1.570796f); }

void SpawnParticles(float x, float y, COLORREF color, int count) {
    for (int i = 0; i < count; i++) {
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (particles[p].life <= 0) {
                particles[p].x = x;
                particles[p].y = y;
                particles[p].vx = ((float)(MyRand() % 101 - 50)) / 10.0f;
                particles[p].vy = ((float)(MyRand() % 101 - 50)) / 10.0f - 2.0f;
                particles[p].life = 15 + (MyRand() % 15);
                particles[p].color = color;
                particles[p].type = (MyRand() % 3); // 0=debris, 1=spark, 2=glow
                break;
            }
        }
    }
}

void SpawnShockwave(float x, float y, COLORREF color) {
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (shockwaves[i].life <= 0) {
            shockwaves[i].x = x; shockwaves[i].y = y;
            shockwaves[i].r = 0; shockwaves[i].max_r = 30.0f + (MyRand() % 20);
            shockwaves[i].life = 1.0f; shockwaves[i].color = color;
            break;
        }
    }
}

void UpdateParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0) {
            particles[i].vy += 0.25f; // gravity
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            particles[i].life--;
        }
    }
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (shockwaves[i].life > 0) {
            shockwaves[i].r += (shockwaves[i].max_r - shockwaves[i].r) * 0.15f + 1.0f;
            shockwaves[i].life -= 0.05f;
        }
    }
}

void DrawBevelBox(HDC hdc, int bx, int by, int bw, int bh, const char* txt, HPEN lPen, HPEN dPen) {
    HBRUSH bBr = CreateSolidBrush(RGB(15, 15, 25));
    RECT bRc = { bx, by, bx + bw, by + bh };
    FillRect(hdc, &bRc, bBr);
    DeleteObject(bBr);
    SelectObject(hdc, dPen);
    MoveToEx(hdc, bx, by + bh, NULL); LineTo(hdc, bx, by); LineTo(hdc, bx + bw, by);
    SelectObject(hdc, lPen);
    LineTo(hdc, bx + bw, by + bh); LineTo(hdc, bx, by + bh);
    SetTextColor(hdc, RGB(0, 255, 255));
    TextOutA(hdc, bx + 5, by + 3, txt, lstrlenA(txt));
}

COLORREF GetBrickColor(int type, int r, int c) {
    if (type == 9) return RGB(120, 135, 150); // Steel
    if (type == 8) return RGB(80, 80, 80);    // Armored
    if (type == 7) return RGB(200, 255, 255); // Phantom
    if (type == 2) return RGB(255, 65, 65);   // 2-Hit Red
    if (type == 3) return RGB(184, 115, 51);  // 3-Hit Bronze
    if (type == 4) return RGB(255, 100, 0);   // Explosive
    if (type == 5) return RGB(170, 0, 255);   // Portal
    if (type == 6) return RGB(0, 255, 204);   // Mystery
    COLORREF rowColors[6] = {
        RGB(255, 50, 100), RGB(255, 150, 50), RGB(255, 200, 0),
        RGB(50, 200, 100), RGB(50, 200, 255), RGB(200, 100, 255)
    };
    return rowColors[r % 6];
}

void DrawGDIBrick(HDC hdc, int r, int c, int type, int bx, int by, int hp) {
    COLORREF baseClr = GetBrickColor(type, r, c);
    HBRUSH br = CreateSolidBrush(baseClr);
    RECT rr = { bx + 1, by + 1, bx + BR_W - 1, by + BR_H - 1 };
    FillRect(hdc, &rr, br);
    DeleteObject(br);

    HPEN lightPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HPEN oldPen = (HPEN)SelectObject(hdc, lightPen);
    MoveToEx(hdc, bx + 1, by + BR_H - 2, NULL);
    LineTo(hdc, bx + 1, by + 1);
    LineTo(hdc, bx + BR_W - 2, by + 1);

    HPEN darkPen = CreatePen(PS_SOLID, 1, RGB(30, 30, 30));
    SelectObject(hdc, darkPen);
    LineTo(hdc, bx + BR_W - 2, by + BR_H - 2);
    LineTo(hdc, bx + 1, by + BR_H - 2);

    int min_dist = 999999;
    Ball* closest_ball = NULL;
    for (int i = 0; i < MAX_BALLS; i++) {
        if (balls[i].active) {
            float dx = balls[i].x - (bx + BR_W / 2);
            float dy = balls[i].y - (by + BR_H / 2);
            int dist = (int)(dx*dx + dy*dy);
            if (dist < min_dist) { min_dist = dist; closest_ball = &balls[i]; }
        }
    }
    if (closest_ball && min_dist < 15000) {
        float dist_f = 1.0f - ((float)min_dist / 15000.0f);
        if (dist_f > 0) {
            float dx = closest_ball->x - (bx + BR_W / 2);
            float dy = closest_ball->y - (by + BR_H / 2);
            int spec_x = bx + BR_W / 2 + (int)(dx * 0.15f);
            int spec_y = by + BR_H / 2 + (int)(dy * 0.3f);
            int size = (int)(4.0f * dist_f);
            if (size > 0) {
                HPEN sPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                HBRUSH sBr = CreateSolidBrush(RGB(255, 255, 255));
                HGDIOBJ oldP2 = SelectObject(hdc, sPen);
                HGDIOBJ oldB2 = SelectObject(hdc, sBr);
                Ellipse(hdc, spec_x - size, spec_y - size, spec_x + size, spec_y + size);
                SelectObject(hdc, oldP2);
                SelectObject(hdc, oldB2);
                DeleteObject(sPen);
                DeleteObject(sBr);
            }
        }
    }

    if (type == 9) { // Steel X
        HPEN steelPen = CreatePen(PS_SOLID, 1, RGB(210, 220, 230));
        SelectObject(hdc, steelPen);
        MoveToEx(hdc, bx + 4, by + 3, NULL); LineTo(hdc, bx + BR_W - 4, by + BR_H - 3);
        MoveToEx(hdc, bx + BR_W - 4, by + 3, NULL); LineTo(hdc, bx + 4, by + BR_H - 3);
        DeleteObject(steelPen);
    } else if (type == 3) { // Metal Stripes
        HPEN stripePen = CreatePen(PS_SOLID, 1, RGB(255, 220, 160));
        SelectObject(hdc, stripePen);
        MoveToEx(hdc, bx + 6, by + 2, NULL); LineTo(hdc, bx + 6, by + BR_H - 2);
        MoveToEx(hdc, bx + BR_W - 6, by + 2, NULL); LineTo(hdc, bx + BR_W - 6, by + BR_H - 2);
        DeleteObject(stripePen);
    } else if (type == 4) { // Explosive Yellow Core
        HBRUSH yBr = CreateSolidBrush(RGB(255, 255, 0));
        SelectObject(hdc, yBr);
        Ellipse(hdc, bx + BR_W/2 - 3, by + BR_H/2 - 3, bx + BR_W/2 + 3, by + BR_H/2 + 3);
        DeleteObject(yBr);
    } else if (type == 6) { // Mystery Mark ?
        SetTextColor(hdc, RGB(0, 0, 0));
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, bx + BR_W/2 - 3, by + 1, "?", 1);
    } else if (type == 7) { // Phantom Glass
        HPEN phantomPen = CreatePen(PS_DOT, 1, RGB(255, 255, 255));
        SelectObject(hdc, phantomPen);
        MoveToEx(hdc, bx + 2, by + 2, NULL); LineTo(hdc, bx + BR_W - 2, by + BR_H - 2);
        DeleteObject(phantomPen);
    } else if (type == 8) { // Armored bolts
        HBRUSH boltBr = CreateSolidBrush(RGB(40, 40, 40));
        SelectObject(hdc, boltBr);
        Ellipse(hdc, bx + 2, by + 2, bx + 5, by + 5);
        Ellipse(hdc, bx + BR_W - 5, by + 2, bx + BR_W - 2, by + 5);
        Ellipse(hdc, bx + 2, by + BR_H - 5, bx + 5, by + BR_H - 2);
        Ellipse(hdc, bx + BR_W - 5, by + BR_H - 5, bx + BR_W - 2, by + BR_H - 2);
        DeleteObject(boltBr);
        
        if (hp > 0 && hp < 4) {
            COLORREF cClr = (hp == 3) ? RGB(255, 150, 0) : ((hp == 2) ? RGB(255, 50, 0) : RGB(255, 0, 0));
            HPEN crackPen = CreatePen(PS_SOLID, 1, cClr);
            HGDIOBJ oldP = SelectObject(hdc, crackPen);
            MoveToEx(hdc, bx + (int)(BR_W*0.2f), by, NULL);
            LineTo(hdc, bx + (int)(BR_W*0.4f), by + (int)(BR_H*0.4f));
            LineTo(hdc, bx + (int)(BR_W*0.3f), by + (int)(BR_H*0.7f));
            if (hp < 3) {
                MoveToEx(hdc, bx + BR_W, by + (int)(BR_H*0.2f), NULL);
                LineTo(hdc, bx + (int)(BR_W*0.7f), by + (int)(BR_H*0.5f));
                LineTo(hdc, bx + (int)(BR_W*0.8f), by + BR_H);
            }
            if (hp < 2) {
                MoveToEx(hdc, bx + (int)(BR_W*0.5f), by + (int)(BR_H*0.5f), NULL);
                LineTo(hdc, bx + (int)(BR_W*0.1f), by + (int)(BR_H*0.9f));
            }
            SelectObject(hdc, oldP);
            DeleteObject(crackPen);
        }
    }

    SelectObject(hdc, oldPen);
    DeleteObject(lightPen);
    DeleteObject(darkPen);
}

void TriggerExplosion(int r, int c) {
    MessageBeep(0xFFFFFFFF);
    for (int nr = r - 1; nr <= r + 1; nr++) {
        for (int nc = c - 1; nc <= c + 1; nc++) {
            if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS) {
                if (bricks[nr][nc] != 0 && bricks[nr][nc] != 9) {
                    int bx = nc * BR_W + BR_W / 2;
                    int by = nr * BR_H + 35 + BR_H / 2;
                    SpawnParticles((float)bx, (float)by, RGB(255, 100, 0), 16);
                    SpawnParticles((float)bx, (float)by, RGB(255, 255, 0), 8);
                    SpawnShockwave((float)bx, (float)by, RGB(255, 100, 0));
                    if (screen_shake < 14) screen_shake = 14;
                    int isExp = (bricks[nr][nc] == 4);
                    bricks[nr][nc] = 0;
                    bricks_left--;
                    score += 15;
                    lifetime_bricks++;
                    if (isExp && (nr != r || nc != c)) {
                        TriggerExplosion(nr, nc);
                    }
                }
            }
        }
    }
}

void LoadHighScore() {
    HANDLE hFile = CreateFileA("kbreakout_hi.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD read;
        int buf[2] = {0, 0};
        if (ReadFile(hFile, buf, sizeof(buf), &read, NULL)) {
            high_score = buf[0];
            lifetime_bricks = buf[1];
        }
        CloseHandle(hFile);
    }
}

void SaveHighScore() {
    if (score > high_score) high_score = score;
    HANDLE hFile = CreateFileA("kbreakout_hi.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        int buf[2] = { high_score, lifetime_bricks };
        WriteFile(hFile, buf, sizeof(buf), &written, NULL);
        CloseHandle(hFile);
    }
}

void UseSkill(char skill) {
    if (state != 1) return;
    if ((skill == 'L' || skill == 'l') && cd_laser <= 0) {
        dur_laser = 360; cd_laser = 720;
        MessageBeep(MB_OK);
    } else if ((skill == 'M' || skill == 'm') && cd_multi <= 0) {
        cd_multi = 900;
        int current_count = 0;
        for (int i = 0; i < MAX_BALLS; i++) if (balls[i].active) current_count++;

        for (int i = 0; i < MAX_BALLS; i++) {
            if (balls[i].active) {
                for (int n = 0; n < 2; n++) {
                    for (int k = 0; k < MAX_BALLS; k++) {
                        if (!balls[k].active) {
                            balls[k].active = 1;
                            balls[k].x = balls[i].x;
                            balls[k].y = balls[i].y;
                            balls[k].dx = balls[i].dx + (n == 0 ? 2.0f : -2.0f);
                            balls[k].dy = balls[i].dy;
                            balls[k].stuck = 0;
                            balls[k].trail_head = 0;
                            for (int t = 0; t < MAX_TRAIL; t++) { balls[k].trail[t].x = 0; balls[k].trail[t].y = 0; }
                            break;
                        }
                    }
                }
            }
        }
        MessageBeep(MB_OK);
    } else if ((skill == 'F' || skill == 'f') && cd_fire <= 0) {
        dur_fire = 480; cd_fire = 900;
        MessageBeep(MB_OK);
    } else if ((skill == 'B' || skill == 'b') && cd_barrier <= 0) {
        dur_barrier = 600; cd_barrier = 1080;
        MessageBeep(MB_OK);
    }
}

void InitLevel() {
    bricks_left = 0;
    power_active = 0;
    dur_laser = 0; dur_fire = 0; dur_barrier = 0;
    cd_laser = 0; cd_multi = 0; cd_fire = 0; cd_barrier = 0;
    paddle_timer = 0; sticky_timer = 0;
    ufo_bullet_active = 0;

    for (int i = 0; i < MAX_LASERS; i++) lasers[i].active = 0;
    for (int i = 0; i < MAX_PARTICLES; i++) particles[i].life = 0;
    for (int i = 0; i < MAX_SHOCKWAVES; i++) shockwaves[i].life = 0;
    for (int i = 0; i < MAX_BOSS_BULLETS; i++) boss_bullets[i].active = 0;
    for (int i = 0; i < MAX_METEORS; i++) meteors[i].active = 0;

    ufo_active = 0;
    ufo_timer = 200;
    meteor_active = (level >= 12 && level % 4 == 0);

    boss_active = (level % 10 == 0 && level > 0) || (level == 33) || (level == 36) || (level == 39);
    if (level == 10) { boss_hp = 25; boss_max_hp = 25; boss_type = 1; boss_dx = 2; boss_x = W / 2 - 50; }
    else if (level == 20) { boss_hp = 50; boss_max_hp = 50; boss_type = 2; boss_dx = 0; boss_x = W / 2 - 50; }
    else if (level == 30) { boss_hp = 75; boss_max_hp = 75; boss_type = 3; boss_dx = 3; boss_x = W / 2 - 50; }
    else if (level == 33) { boss_hp = 80; boss_max_hp = 80; boss_type = 1; boss_dx = 4; boss_x = W / 2 - 50; }
    else if (level == 36) { boss_hp = 100; boss_max_hp = 100; boss_type = 2; boss_dx = 0; boss_x = W / 2 - 50; }
    else if (level == 39) { boss_hp = 125; boss_max_hp = 125; boss_type = 3; boss_dx = 4; boss_x = W / 2 - 50; }
    else if (level == 40) { boss_hp = 150; boss_max_hp = 150; boss_type = 3; boss_dx = 5; boss_x = W / 2 - 50; }

    shield_active = (level >= 14);
    shield_x = 50; shield_dx = (float)(2 + (level >= 16 ? 1 : 0));

    // Reset single active ball
    for (int i = 0; i < MAX_BALLS; i++) balls[i].active = 0;
    balls[0].active = 1;
    balls[0].x = (float)(W / 2);
    balls[0].y = (float)(H - 50);
    balls[0].dx = speed * ((MyRand() % 2 == 0) ? 1.0f : -1.0f);
    balls[0].dy = -speed;
    balls[0].stuck = 1;
    balls[0].stuck_offset = pad_w / 2;
    balls[0].trail_head = 0;
    for (int t = 0; t < MAX_TRAIL; t++) { balls[0].trail[t].x = 0; balls[0].trail[t].y = 0; }

    pad_w = (diff == 1) ? 45 : 65;
    pad_x = W / 2 - pad_w / 2;

    // Generate Stage Bricks Architecture
    if (level != 99) {
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
            int v = 0;
            if (level == 1) {
                if (r == 0) v = 2; else if (r <= 3) v = 1;
            } else if (level == 2) {
                v = ((r + c) % 2 == 0) ? 1 : 2;
                if ((r == 0 || r == 4) && (c == 0 || c == 9)) v = 9;
            } else if (level == 3) {
                if (c >= r && c < COLS - r) v = (r == 0) ? 6 : (r == 1) ? 3 : 1;
            } else if (level == 4) {
                if ((r + c) % 3 == 0) v = 4; else v = (r % 2 == 0) ? 2 : 1;
            } else if (level == 5) {
                if (c == 0 || c == 9 || r == 0) v = 5; else if (r == 2 && (c == 4 || c == 5)) v = 6; else v = 2;
            } else if (level == 6) {
                if ((r == 1 && (c == 2 || c == 7)) || (r == 2 && c >= 2 && c <= 7) || (r == 3 && (c == 1 || c == 4 || c == 5 || c == 8))) v = 3;
                else if (r == 2 && (c == 4 || c == 5)) v = 4;
            } else if (level == 7) {
                if (r == 2 && (c != 2 && c != 7)) v = 9; else if (r < 2) v = 3; else v = 4;
            } else if (level == 8) {
                if (c <= 2 || c >= 7) v = (r % 2 == 0) ? 3 : 2; else if (r == 1 && (c == 4 || c == 5)) v = 5;
            } else if (level == 9) {
                int pat[6] = {1, 2, 4, 5, 6, 3};
                v = pat[(r * COLS + c) % 6];
            } else if (level == 10) {
                if (r >= 2) v = (r == 2) ? 3 : 2;
            } else if (level == 11) {
                if ((r == 0 && c < 8) || (c == 8 && r < 4) || (r == 4 && c > 1) || (c == 1 && r > 1)) v = (r % 2 == 0) ? 3 : 5; else v = 1;
            } else if (level == 12) {
                if (r == 0 && c % 2 == 0) v = 9; else if (r == 1) v = 3; else v = 4;
            } else if (level == 13) {
                if (r == 1) v = 3; else if (r == 2) v = 4; else if (r == 3) v = 6; else v = 1;
            } else if (level == 14) {
                if (MyAbs(r - 2) + MyAbs(c - 4) <= 3) v = (r == 2) ? 4 : 3;
            } else if (level == 15) {
                if (r >= 1 && r <= 2 && c >= 4 && c <= 5) v = 4; else v = 3;
            } else if (level == 16) {
                if (c == 2 || c == 7) v = 5; else v = (r % 2 == 0) ? 3 : 2;
            } else if (level == 17) {
                if (r == 0) v = 9; else if (r <= 2) v = 3; else v = 4;
            } else if (level == 18) {
                if ((r == 1 || r == 3) && (c == 2 || c == 7)) v = 5; else v = (r % 2 == 0) ? 4 : 3;
            } else if (level == 19) {
                if (r == 1) v = 9; else if (r == 2) v = 3; else v = 4;
            } else if (level == 20) {
                if (r >= 3) v = (r == 3) ? 3 : 2;
            }

            bricks[r][c] = v;
            brick_hp[r][c] = (v == 8) ? 4 : 0;
            if (v != 0 && v != 9) bricks_left++;
            }
        }
    }
    
    // Level Editor Concepts (String Arrays for Stages 21-30)
    if (level >= 21 && level <= 30) {
        bricks_left = 0;
        const char* custom_stages[10][ROWS] = {
            {"8888888888","7777777777","0000000000","1231231231","0000000000","0000000000"}, // 21
            {"9000000009","0800000080","0070000700","0006006000","0000550000","0000000000"}, // 22
            {"8787878787","7878787878","8787878787","7878787878","0000000000","0000000000"}, // 23
            {"5000000005","0888888880","0877777780","0888888880","0000000000","0000000000"}, // 24
            {"9999009999","7777007777","8888008888","7777007777","9999009999","0000000000"}, // 25
            {"8000000008","0800000080","0080000800","0008008000","0000880000","0000000000"}, // 26
            {"6666666666","7777777777","4444444444","8888888888","7777777777","6666666666"}, // 27
            {"9876543210","0123456789","9876543210","0123456789","0000000000","0000000000"}, // 28
            {"8889999888","7779999777","6669999666","5559999555","4449999444","0000000000"}, // 29
            {"0000000000","0000000000","0008888000","0008888000","0000000000","0000000000"}  // 30
        };
        int idx = level - 21;
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                char ch = custom_stages[idx][r][c];
                int v = (ch >= '0' && ch <= '9') ? (ch - '0') : 0;
                bricks[r][c] = v;
                brick_hp[r][c] = (v == 8) ? 4 : 0;
                if (v != 0 && v != 9) bricks_left++;
            }
        }
    }

    if (level >= 31 && level <= 40 && !boss_active && level != 99) {
        bricks_left = 0;
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                int v = 0;
                if (level == 31) v = (r % 2 == 0) ? 8 : 4;
                else if (level == 32) v = (c % 2 == 0) ? 7 : 5;
                else if (level == 34) v = (r == c || r == COLS - 1 - c) ? 9 : 6;
                else if (level == 35) v = (r < 3) ? 8 : 2;
                else if (level == 37) v = ((r + c) % 3 == 0) ? 4 : 8;
                else if (level == 38) v = 7;
                bricks[r][c] = v;
                brick_hp[r][c] = (v == 8) ? 4 : 0;
                if (v != 0 && v != 9) bricks_left++;
            }
        }
    }

    if (level == 99) {
        bricks_left = 0;
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                if (bricks[r][c] != 0 && bricks[r][c] != 9) bricks_left++;
            }
        }
    }

    if (bricks_left == 0 && !boss_active) { bricks[0][0] = 1; bricks_left = 1; }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            LoadHighScore();
            SetTimer(hwnd, TIMER_ID, 16, NULL);
            break;
        case WM_LBUTTONDOWN:
            if (state == 4) {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                if (y >= 35 && y < 35 + ROWS * BR_H && x >= 0 && x < W) {
                    int r = (y - 35) / BR_H;
                    int c = x / BR_W;
                    bricks[r][c] = (bricks[r][c] + 1) % 10;
                    brick_hp[r][c] = (bricks[r][c] == 8) ? 4 : 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            break;
        case WM_KEYDOWN:
            if (state == 1) {
                if (wParam == 'L' || wParam == 'l') UseSkill('L');
                if (wParam == 'M' || wParam == 'm') UseSkill('M');
                if (wParam == 'F' || wParam == 'f') UseSkill('F');
                if (wParam == 'B' || wParam == 'b') UseSkill('B');
            }
            break;
        case WM_TIMER:
            if (state == 1) {
                frame_counter++;
                // Skill Timers
                if (dur_laser > 0) dur_laser--;
                if (dur_fire > 0) dur_fire--;
                if (dur_barrier > 0) dur_barrier--;
                if (cd_laser > 0) cd_laser--;
                if (cd_multi > 0) cd_multi--;
                if (cd_fire > 0) cd_fire--;
                if (cd_barrier > 0) cd_barrier--;
                if (paddle_timer > 0) { paddle_timer--; if (paddle_timer == 0) pad_w = (diff == 1) ? 45 : 65; }

                // Controls
                if ((GetAsyncKeyState(VK_LEFT) & 0x8000) || (GetAsyncKeyState('A') & 0x8000)) pad_x -= 6;
                if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) || (GetAsyncKeyState('D') & 0x8000)) pad_x += 6;
                if (pad_x < 0) pad_x = 0;
                if (pad_x > W - pad_w) pad_x = W - pad_w;

                pad_vx = pad_x - last_pad_x;
                last_pad_x = pad_x;
                if (pad_squash_timer > 0) pad_squash_timer--;

                UpdateParticles();

                // Laser Auto-fire
                if (dur_laser > 0 && dur_laser % 15 == 0) {
                    for (int i = 0; i < MAX_LASERS; i++) {
                        if (!lasers[i].active) {
                            lasers[i].active = 1;
                            lasers[i].x = (float)(pad_x + 5);
                            lasers[i].y = (float)(H - 40);
                            break;
                        }
                    }
                    for (int i = 0; i < MAX_LASERS; i++) {
                        if (!lasers[i].active) {
                            lasers[i].active = 1;
                            lasers[i].x = (float)(pad_x + pad_w - 5);
                            lasers[i].y = (float)(H - 40);
                            break;
                        }
                    }
                }

                // Update Lasers
                for (int i = 0; i < MAX_LASERS; i++) {
                    if (lasers[i].active) {
                        lasers[i].y -= 7.0f;
                        if (lasers[i].y < 0) lasers[i].active = 0;
                        else {
                            // Laser vs Boss
                            if (boss_active && lasers[i].x > boss_x && lasers[i].x < boss_x + boss_w &&
                                lasers[i].y > boss_y && lasers[i].y < boss_y + boss_h) {
                                boss_hp--;
                                lasers[i].active = 0;
                                SpawnParticles(lasers[i].x, lasers[i].y, RGB(255, 0, 85), 4);
                                MessageBeep(0xFFFFFFFF);
                                if (boss_hp <= 0) {
                                    score += 500;
                                    SpawnParticles((float)(boss_x + boss_w/2), (float)(boss_y + boss_h/2), RGB(255, 0, 85), 40);
                                    boss_active = 0;
                                    level++;
                                    if (level > 40) { state = 3; SaveHighScore(); } else InitLevel();
                                }
                                continue;
                            }

                            // Laser vs Bricks
                            int r = ((int)lasers[i].y - 35) / BR_H;
                            int c = (int)lasers[i].x / BR_W;
                            if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
                                if (bricks[r][c] != 0 && bricks[r][c] != 9) {
                                    lasers[i].active = 0;
                                    int type = bricks[r][c];
                                    SpawnParticles(lasers[i].x, lasers[i].y, GetBrickColor(type, r, c), 6);
                                    if (type == 4) TriggerExplosion(r, c);
                                    else if (type == 7) { bricks[r][c] = 0; bricks_left--; score += 30; lifetime_bricks++; }
                                    else if (type == 8) {
                                        brick_hp[r][c]--;
                                        if (brick_hp[r][c] <= 0) { bricks[r][c] = 0; bricks_left--; score += 40; lifetime_bricks++; }
                                        else { score += 5; }
                                    }
                                    else if (type > 1 && type != 6) { bricks[r][c]--; score += 5; }
                                    else { bricks[r][c] = 0; bricks_left--; score += 10; lifetime_bricks++; }
                                }
                            }
                        }
                    }
                }

                // Update Hazard Shield
                if (shield_active) {
                    shield_x += shield_dx;
                    if (shield_x < 10 || shield_x + shield_w > W - 10) shield_dx = -shield_dx;
                }

                // Update UFO Drone
                if (ufo_active) {
                    ufo_x += ufo_dx;
                    if (ufo_x < 10 || ufo_x + 30 > W - 10) ufo_dx = -ufo_dx;

                    if (!ufo_bullet_active && (MyRand() % 100 < 3)) {
                        ufo_bullet_active = 1;
                        ufo_bullet_x = ufo_x + 15;
                        ufo_bullet_y = (float)(ufo_y + 10);
                    }
                } else {
                    if (ufo_timer > 0) ufo_timer--;
                    else if (level >= 3) { ufo_active = 1; ufo_x = 10; ufo_dx = 2; ufo_timer = 300; }
                }

                // UFO Bullet
                if (ufo_bullet_active) {
                    ufo_bullet_y += 3.5f;
                    if (ufo_bullet_y > H - 40 && ufo_bullet_y < H - 30 && ufo_bullet_x > pad_x && ufo_bullet_x < pad_x + pad_w) {
                        ufo_bullet_active = 0;
                        SpawnParticles(ufo_bullet_x, ufo_bullet_y, RGB(255, 0, 255), 10);
                        if (score > 25) score -= 25;
                        MessageBeep(0xFFFFFFFF);
                    } else if (ufo_bullet_y > H) ufo_bullet_active = 0;
                }

                // Boss Attacks & Shields
                if (boss_active) {
                    if (boss_type == 1 || boss_type == 3) {
                        boss_x += boss_dx;
                        if (boss_x < 10 || boss_x + boss_w > W - 10) boss_dx = -boss_dx;
                    }
                    boss_shield_angle += 0.04f;
                    int fire_chance = (boss_type == 1) ? 2 : ((boss_type == 2) ? 3 : 4);
                    if ((MyRand() % 100 < fire_chance)) {
                        for (int i = 0; i < MAX_BOSS_BULLETS; i++) {
                            if (!boss_bullets[i].active) {
                                boss_bullets[i].active = 1;
                                boss_bullets[i].x = (float)(boss_x + 20 + (i % 2 == 0 ? 0 : boss_w - 40));
                                boss_bullets[i].y = (float)(boss_y + boss_h);
                                break;
                            }
                        }
                    }
                }
                for (int i = 0; i < MAX_BOSS_BULLETS; i++) {
                    if (boss_bullets[i].active) {
                        boss_bullets[i].y += 3.0f;
                        if (boss_bullets[i].y > H - 40 && boss_bullets[i].y < H - 30 &&
                            boss_bullets[i].x > pad_x && boss_bullets[i].x < pad_x + pad_w) {
                            boss_bullets[i].active = 0;
                            SpawnParticles(boss_bullets[i].x, boss_bullets[i].y, RGB(255, 0, 85), 8);
                            MessageBeep(0xFFFFFFFF);
                        } else if (boss_bullets[i].y > H) boss_bullets[i].active = 0;
                    }
                }

                // Power-up falling
                if (power_active) {
                    power_y += 2.5f;
                    if (power_y + 10 > H - 40 && power_y < H - 40 + pad_h && power_x + 10 > pad_x && power_x < pad_x + pad_w) {
                        power_active = 0;
                        pad_squash_timer = 15;
                        MessageBeep(MB_OK);
                        SpawnParticles(power_x, power_y, RGB(255, 255, 255), 10);
                        if (power_type == 1) { paddle_timer = 350; pad_w = (diff == 1) ? 75 : 95; }
                        else if (power_type == 2) { lives++; }
                        else if (power_type == 3) { dur_fire = 480; }
                        else if (power_type == 4) { sticky_timer = 300; }
                        else if (power_type == 5) { dur_laser = 360; }
                        else if (power_type == 6) { dur_barrier = 600; }
                        else if (power_type == 7) { UseSkill('M'); }
                    }
                    if (power_y > H) power_active = 0;
                }

                // Meteor Hazard
                if (meteor_active) {
                    if (MyRand() % 100 < 2) {
                        for (int i = 0; i < MAX_METEORS; i++) {
                            if (!meteors[i].active) {
                                meteors[i].active = 1;
                                meteors[i].x = (float)(MyRand() % (W - 12) + 6);
                                meteors[i].y = 0;
                                break;
                            }
                        }
                    }
                    for (int i = 0; i < MAX_METEORS; i++) {
                        if (meteors[i].active) {
                            meteors[i].y += 3.5f;
                            if (meteors[i].y > H - 40 && meteors[i].y < H - 30 && meteors[i].x > pad_x && meteors[i].x < pad_x + pad_w) {
                                meteors[i].active = 0;
                                sticky_timer = 0;
                                pad_w = (diff == 1) ? 30 : 45;
                                paddle_timer = 200;
                                SpawnParticles(meteors[i].x, meteors[i].y, RGB(255, 100, 0), 15);
                                MessageBeep(MB_ICONHAND);
                            } else if (meteors[i].y > H) meteors[i].active = 0;
                        }
                    }
                }

                // Update Active Balls
                int active_balls_count = 0;
                for (int i = 0; i < MAX_BALLS; i++) {
                    if (!balls[i].active) continue;

                    active_balls_count++;

                    if (balls[i].stuck) {
                        balls[i].x = (float)(pad_x + balls[i].stuck_offset);
                        balls[i].y = (float)(H - 40 - 8);
                        if ((GetAsyncKeyState(VK_UP) & 0x8000) || (GetAsyncKeyState(VK_SPACE) & 0x8000)) {
                            balls[i].stuck = 0;
                            balls[i].dy = -speed;
                        }
                        continue;
                    }

                    balls[i].x += balls[i].dx;
                    balls[i].y += balls[i].dy;
                    
                    // Update Trail
                    balls[i].trail[balls[i].trail_head].x = balls[i].x;
                    balls[i].trail[balls[i].trail_head].y = balls[i].y;
                    balls[i].trail_head = (balls[i].trail_head + 1) % MAX_TRAIL;

                    // Wall collision
                    if (balls[i].x < 0) { balls[i].x = 0; balls[i].dx = -balls[i].dx; }
                    if (balls[i].x > W - 8) { balls[i].x = (float)(W - 8); balls[i].dx = -balls[i].dx; }
                    if (balls[i].y < 0) { balls[i].y = 0; balls[i].dy = -balls[i].dy; }

                    // Barrier floor collision
                    if (dur_barrier > 0 && balls[i].y >= H - 20) {
                        balls[i].y = (float)(H - 20);
                        balls[i].dy = -MyAbs((int)balls[i].dy);
                        SpawnParticles(balls[i].x, (float)(H - 10), RGB(0, 255, 255), 6);
                    }

                    // Hazard Shield collision
                    if (shield_active && balls[i].y + 8 > shield_y && balls[i].y < shield_y + 8 &&
                        balls[i].x + 8 > shield_x && balls[i].x < shield_x + shield_w) {
                        balls[i].dy = -balls[i].dy;
                        SpawnParticles(balls[i].x, balls[i].y, RGB(0, 255, 255), 6);
                    }

                    // UFO collision
                    if (ufo_active && balls[i].x + 8 > ufo_x && balls[i].x < ufo_x + 30 &&
                        balls[i].y + 8 > ufo_y && balls[i].y < ufo_y + 12) {
                        ufo_active = 0;
                        balls[i].dy = -balls[i].dy;
                        score += 100;
                        SpawnParticles(ufo_x + 15, (float)(ufo_y + 6), RGB(255, 0, 255), 15);
                        MessageBeep(0xFFFFFFFF);
                        power_active = 1; power_type = 6; power_x = ufo_x + 15; power_y = (float)ufo_y;
                    }

                    // Boss Fortress collision
                    if (boss_active && balls[i].x + 8 > boss_x && balls[i].x < boss_x + boss_w &&
                        balls[i].y + 8 > boss_y && balls[i].y < boss_y + boss_h) {
                        boss_hp -= (dur_fire > 0 ? 2 : 1);
                        if (dur_fire <= 0) balls[i].dy = -balls[i].dy;
                        SpawnParticles(balls[i].x, balls[i].y, RGB(255, 0, 85), 8);
                        MessageBeep(0xFFFFFFFF);
                        if (boss_hp <= 0) {
                            score += 1000;
                            SpawnParticles((float)(boss_x + boss_w/2), (float)(boss_y + boss_h/2), RGB(255, 0, 85), 50);
                            boss_active = 0;
                            level++;
                            if (level > 40) { state = 3; SaveHighScore(); } else InitLevel();
                            break;
                        }
                    }

                    // Paddle collision
                    if (balls[i].y + 8 > H - 40 && balls[i].y < H - 40 + pad_h &&
                        balls[i].x + 8 > pad_x && balls[i].x < pad_x + pad_w) {
                        balls[i].y = (float)(H - 40 - 8);
                        balls[i].dy = -MyAbs((int)balls[i].dy);
                        pad_squash_timer = 12;
                        float hit_pos = (balls[i].x + 4) - (pad_x + pad_w / 2);
                        balls[i].dx = hit_pos * 0.22f;
                        if (MyAbs((int)balls[i].dx) == 0) balls[i].dx = (balls[i].dx > 0) ? 1.5f : -1.5f;
                        MessageBeep(0xFFFFFFFF);
                        SpawnParticles(balls[i].x + 4, (float)(H - 40), RGB(0, 255, 255), 4);
                        SpawnShockwave(balls[i].x + 4, (float)(H - 40), RGB(0, 255, 255));
                        if (screen_shake < 8) screen_shake = 8;
                        if (sticky_timer > 0) {
                            balls[i].stuck = 1;
                            balls[i].stuck_offset = (int)(balls[i].x - pad_x);
                        }
                    }

                    // Bricks collision
                    int r = ((int)balls[i].y - 35) / BR_H;
                    int c = (int)balls[i].x / BR_W;
                    if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
                        int type = bricks[r][c];
                        if (type != 0) {
                            int bx = c * BR_W + BR_W / 2;
                            int by = r * BR_H + 35 + BR_H / 2;

                            if (type == 9) { // Steel Unbreakable
                                if (dur_fire <= 0) balls[i].dy = -balls[i].dy;
                                SpawnParticles(balls[i].x, balls[i].y, RGB(200, 210, 220), 4);
                                SpawnShockwave(balls[i].x, balls[i].y, RGB(200, 210, 220));
                                if (screen_shake < 4) screen_shake = 4;
                                MessageBeep(0xFFFFFFFF);
                            } else if (type == 4) { // Explosive
                                bricks[r][c] = 0; bricks_left--;
                                if (dur_fire <= 0) balls[i].dy = -balls[i].dy;
                                TriggerExplosion(r, c);
                            } else if (type == 5) { // Portal Warp
                                bricks[r][c] = 0; bricks_left--; score += 20; lifetime_bricks++;
                                SpawnParticles((float)bx, (float)by, RGB(170, 0, 255), 12);
                                MessageBeep(0xFFFFFFFF);
                                balls[i].x = (float)(20 + MyRand() % (W - 40));
                                balls[i].y = 150.0f;
                                balls[i].dx = (float)((MyRand() % 2 == 0 ? 1 : -1) * (speed + 1));
                                balls[i].dy = speed;
                            } else if (type == 6) { // Mystery Drop
                                bricks[r][c] = 0; bricks_left--; score += 25; lifetime_bricks++;
                                SpawnParticles((float)bx, (float)by, RGB(0, 255, 204), 10);
                                MessageBeep(0xFFFFFFFF);
                                power_active = 1; power_type = (MyRand() % 7) + 1;
                                power_x = (float)bx; power_y = (float)by;
                            } else if (type == 7) { // Phantom Glass
                                bricks[r][c] = 0; bricks_left--; score += 30; lifetime_bricks++;
                                SpawnParticles((float)bx, (float)by, RGB(200, 255, 255), 10);
                                MessageBeep(0xFFFFFFFF);
                            } else if (type == 8) { // Armored
                                SpawnParticles((float)bx, (float)by, RGB(80, 80, 80), 8);
                                if (dur_fire > 0) { bricks[r][c] = 0; bricks_left--; score += 40; lifetime_bricks++; }
                                else {
                                    brick_hp[r][c]--;
                                    if (brick_hp[r][c] <= 0) { bricks[r][c] = 0; bricks_left--; score += 40; lifetime_bricks++; }
                                    else { score += 5; }
                                    balls[i].dy = -balls[i].dy;
                                }
                                MessageBeep(0xFFFFFFFF);
                            } else { // Normal / Reinforced
                                SpawnParticles((float)bx, (float)by, GetBrickColor(type, r, c), 8);
                                SpawnShockwave((float)bx, (float)by, GetBrickColor(type, r, c));
                                if (screen_shake < 5) screen_shake = 5;
                                if (dur_fire > 0) {
                                    bricks[r][c] = 0; bricks_left--; score += 15; lifetime_bricks++;
                                } else {
                                    bricks[r][c]--;
                                    if (bricks[r][c] == 0) { bricks_left--; score += 10; lifetime_bricks++; }
                                    else score += 5;
                                    balls[i].dy = -balls[i].dy;
                                }
                                MessageBeep(0xFFFFFFFF);

                                if (!power_active && (MyRand() % 5 == 0)) {
                                    power_active = 1;
                                    power_type = (MyRand() % 7) + 1;
                                    power_x = (float)bx; power_y = (float)by;
                                }
                            }

                            if (bricks_left <= 0 && !boss_active) {
                                level++;
                                if (level > 40) { state = 3; SaveHighScore(); } else InitLevel();
                                break;
                            }
                        }
                    }

                    // Ball Out of Bottom
                    if (balls[i].y > H && dur_barrier <= 0) {
                        balls[i].active = 0;
                        active_balls_count--;
                    }
                }

                if (active_balls_count <= 0) {
                    lives--;
                    power_active = 0; sticky_timer = 0; dur_laser = 0; dur_fire = 0;
                    if (lives <= 0) {
                        SaveHighScore();
                        state = 2; // Game Over
                        MessageBeep(MB_ICONEXCLAMATION);
                    } else {
                        InitLevel();
                        MessageBeep(MB_ICONASTERISK);
                    }
                }
            } else if (state == 0 || state == 2 || state == 3) {
                if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
                    diff = 0; speed = 3.5f; score = 0; lives = 3; level = 1; InitLevel(); state = 1; pad_w = 65;
                }
                if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
                    diff = 1; speed = 5.0f; score = 0; lives = 3; level = 1; InitLevel(); state = 1; pad_w = 45;
                }
                if (GetAsyncKeyState('E') & 0x8000) {
                    state = 4;
                    for (int r=0; r<ROWS; r++) for(int c=0; c<COLS; c++) bricks[r][c]=0;
                }
            } else if (state == 4) {
                if (GetAsyncKeyState('P') & 0x8000) {
                    diff = 1; speed = 4.0f; score = 0; lives = 3; level = 99; InitLevel(); state = 1; pad_w = 65;
                }
                if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                    state = 0;
                }
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, W, H);
            HGDIOBJ hOld = SelectObject(memDC, hbm);
            
            int sx = 0, sy = 0;
            if (screen_shake > 0) {
                sx = (MyRand() % screen_shake) - (screen_shake / 2);
                sy = (MyRand() % screen_shake) - (screen_shake / 2);
                screen_shake -= (screen_shake > 6 ? 2 : 1);
                if (screen_shake < 0) screen_shake = 0;
            }
            SetViewportOrgEx(memDC, sx, sy, NULL);
            
            HBRUSH bg = CreateSolidBrush(RGB(16, 16, 26));
            RECT fullRc = {0, 0, W, H};
            FillRect(memDC, &fullRc, bg);
            DeleteObject(bg);
            
            // Environmental Art: Cyber-grid and floating space dust
            HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(20, 40, 60));
            HGDIOBJ oldP2 = SelectObject(memDC, gridPen);
            for (int i = 0; i < W; i += 40) {
                MoveToEx(memDC, i, 0, NULL);
                LineTo(memDC, i, H);
            }
            for (int i = 0; i < H; i += 40) {
                int y = (i + (frame_counter / 2)) % H;
                MoveToEx(memDC, 0, y, NULL);
                LineTo(memDC, W, y);
            }
            SelectObject(memDC, oldP2);
            DeleteObject(gridPen);
            
            // Atmospheric dust
            for(int i = 0; i < 20; i++) {
                int dx = (i * 73 + frame_counter) % W;
                int dy = (i * 37 + frame_counter / 3) % H;
                SetPixel(memDC, dx, dy, RGB(100, 150, 255));
            }
            
            SetTextColor(memDC, RGB(255, 255, 255));
            SetBkMode(memDC, TRANSPARENT);
            
            if (state == 0) {
                char* t1 = "KBREAKOUT - LOOP 9";
                char* t2 = "40 Stages - Advanced Editor";
                char* t3 = "Press ENTER for Easy";
                char* t4 = "Press SPACE for Hard";
                char* t5 = "Press E for Level Editor";
                TextOutA(memDC, W/2 - 70, H/2 - 45, t1, lstrlenA(t1));
                TextOutA(memDC, W/2 - 80, H/2 - 20, t2, lstrlenA(t2));
                TextOutA(memDC, W/2 - 70, H/2 + 10, t3, lstrlenA(t3));
                TextOutA(memDC, W/2 - 70, H/2 + 30, t4, lstrlenA(t4));
                TextOutA(memDC, W/2 - 75, H/2 + 50, t5, lstrlenA(t5));
            } else if (state == 4) {
                char* t1 = "EDITOR MODE";
                char* t2 = "Click grid to change blocks";
                char* t3 = "Press P to Play Custom Level";
                char* t4 = "Press ESC to return";
                TextOutA(memDC, W/2 - 45, H/2 - 20, t1, lstrlenA(t1));
                TextOutA(memDC, W/2 - 85, H/2 + 10, t2, lstrlenA(t2));
                TextOutA(memDC, W/2 - 85, H/2 + 30, t3, lstrlenA(t3));
                TextOutA(memDC, W/2 - 60, H/2 + 50, t4, lstrlenA(t4));

                for (int r = 0; r < ROWS; r++) {
                    for (int c = 0; c < COLS; c++) {
                        if (bricks[r][c]) {
                            DrawGDIBrick(memDC, r, c, bricks[r][c], c * BR_W, r * BR_H + 35, brick_hp[r][c]);
                        } else {
                            HPEN dotPen = CreatePen(PS_DOT, 1, RGB(50, 50, 50));
                            HGDIOBJ oP = SelectObject(memDC, dotPen);
                            HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
                            HGDIOBJ oB = SelectObject(memDC, nullBr);
                            Rectangle(memDC, c * BR_W, r * BR_H + 35, c * BR_W + BR_W, r * BR_H + 35 + BR_H);
                            SelectObject(memDC, oP);
                            SelectObject(memDC, oB);
                            DeleteObject(dotPen);
                        }
                    }
                }
            } else if (state == 2) {
                char* t1 = "GAME OVER";
                char* t2 = "Press ENTER for Easy";
                char* t3 = "Press SPACE for Hard";
                TextOutA(memDC, W/2 - 40, H/2 - 20, t1, lstrlenA(t1));
                TextOutA(memDC, W/2 - 70, H/2 + 10, t2, lstrlenA(t2));
                TextOutA(memDC, W/2 - 70, H/2 + 30, t3, lstrlenA(t3));
            } else if (state == 3) {
                char* t1 = "VICTORY! CAMPAIGN CLEARED";
                char* t2 = "Press ENTER to Play Again";
                TextOutA(memDC, W/2 - 85, H/2 - 20, t1, lstrlenA(t1));
                TextOutA(memDC, W/2 - 75, H/2 + 15, t2, lstrlenA(t2));
            } else {
                // Draw Bricks
                for (int r = 0; r < ROWS; r++) {
                    for (int c = 0; c < COLS; c++) {
                        if (bricks[r][c]) {
                            DrawGDIBrick(memDC, r, c, bricks[r][c], c * BR_W, r * BR_H + 35, brick_hp[r][c]);
                        }
                    }
                }

                // Draw Hazard Shield
                if (shield_active) {
                    HBRUSH sBr = CreateSolidBrush(RGB(0, 255, 255));
                    RECT sRc = { (int)shield_x, (int)120, (int)(shield_x + shield_w), 128 };
                    FillRect(memDC, &sRc, sBr);
                    DeleteObject(sBr);
                }

                // Draw UFO Drone
                if (ufo_active) {
                    HBRUSH uBr = CreateSolidBrush(RGB(255, 0, 255));
                    RECT uRc = { (int)ufo_x, ufo_y, (int)(ufo_x + 30), ufo_y + 10 };
                    FillRect(memDC, &uRc, uBr);
                    DeleteObject(uBr);
                    
                    // Animated UFO lights
                    for (int i = 0; i < 3; i++) {
                        int lx = (int)ufo_x + 5 + i * 10;
                        int blink = ((frame_counter % 30 < 15 && i % 2 == 0) || (frame_counter % 30 >= 15 && i % 2 != 0));
                        HBRUSH lBr = CreateSolidBrush(blink ? RGB(255, 255, 0) : RGB(136, 136, 0));
                        RECT lRc = { lx - 1, ufo_y + 5, lx + 1, ufo_y + 7 };
                        FillRect(memDC, &lRc, lBr);
                        DeleteObject(lBr);
                    }
                }
                if (ufo_bullet_active) {
                    HBRUSH ubBr = CreateSolidBrush(RGB(255, 0, 255));
                    RECT ubRc = { (int)ufo_bullet_x - 2, (int)ufo_bullet_y, (int)ufo_bullet_x + 2, (int)ufo_bullet_y + 8 };
                    FillRect(memDC, &ubRc, ubBr);
                    DeleteObject(ubBr);
                }

                // Draw Boss Fortress
                if (boss_active) {
                    HBRUSH bBr = CreateSolidBrush(RGB(255, 0, 85));
                    RECT bRc = { boss_x, boss_y, boss_x + boss_w, boss_y + boss_h };
                    FillRect(memDC, &bRc, bBr);
                    DeleteObject(bBr);

                    char bStr[32];
                    wsprintfA(bStr, "BOSS HP: %d/%d", boss_hp, boss_max_hp);
                    TextOutA(memDC, boss_x + 10, boss_y + 12, bStr, lstrlenA(bStr));
                    
                    // Orbital Shields for Boss 2 and 3
                    if (boss_type == 2 || boss_type == 3) {
                        int ofs = (frame_counter % 80 < 40) ? (frame_counter % 40) * 2 - 40 : 40 - (frame_counter % 40) * 2;
                        int sx1 = boss_x + boss_w/2 + ofs;
                        int sy1 = boss_y + boss_h + 10;
                        int sx2 = boss_x + boss_w/2 - ofs;
                        int sy2 = boss_y - 10;
                        HBRUSH shBr = CreateSolidBrush(RGB(0, 255, 255));
                        RECT s1 = {sx1-5, sy1-5, sx1+5, sy1+5}; FillRect(memDC, &s1, shBr);
                        RECT s2 = {sx2-5, sy2-5, sx2+5, sy2+5}; FillRect(memDC, &s2, shBr);
                        DeleteObject(shBr);
                    }

                    // Animated Boss Core Ring (Approximated with Ellipse)
                    int pulse = (frame_counter % 20 < 10) ? 2 : -2;
                    HPEN corePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 0));
                    HGDIOBJ oldP = SelectObject(memDC, corePen);
                    HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
                    HGDIOBJ oldB = SelectObject(memDC, nullBr);
                    int cx = boss_x + boss_w / 2;
                    int cy = boss_y + boss_h / 2;
                    int r_boss = 15 + pulse;
                    Ellipse(memDC, cx - r_boss, cy - r_boss, cx + r_boss, cy + r_boss);
                    SelectObject(memDC, oldP);
                    SelectObject(memDC, oldB);
                    DeleteObject(corePen);
                }
                for (int i = 0; i < MAX_BOSS_BULLETS; i++) {
                    if (boss_bullets[i].active) {
                        HBRUSH bbBr = CreateSolidBrush(RGB(255, 50, 100));
                        RECT bbRc = { (int)boss_bullets[i].x - 3, (int)boss_bullets[i].y - 3, (int)boss_bullets[i].x + 3, (int)boss_bullets[i].y + 3 };
                        FillRect(memDC, &bbRc, bbBr);
                        DeleteObject(bbBr);
                    }
                }
                for (int i = 0; i < MAX_METEORS; i++) {
                    if (meteors[i].active) {
                        HBRUSH mBr = CreateSolidBrush(RGB(255, 100, 0));
                        RECT mRc = { (int)meteors[i].x - 5, (int)meteors[i].y - 5, (int)meteors[i].x + 5, (int)meteors[i].y + 5 };
                        FillRect(memDC, &mRc, mBr);
                        DeleteObject(mBr);
                    }
                }

                // Draw Safety Barrier
                if (dur_barrier > 0) {
                    HPEN barPen = CreatePen(PS_SOLID, 3, RGB(0, 255, 255));
                    HGDIOBJ oP = SelectObject(memDC, barPen);
                    MoveToEx(memDC, 0, H - 20, NULL);
                    LineTo(memDC, W, H - 20);
                    SelectObject(memDC, oP);
                    DeleteObject(barPen);
                }

                // Draw Paddle
                float p_squash = 1.0f + (pad_squash_timer > 0 ? MySin(pad_squash_timer * 0.5f) * 0.4f * ((float)pad_squash_timer / 15.0f) : 0.0f);
                int dp_w = (int)(pad_w * p_squash);
                int dp_h = (int)(pad_h * (2.0f - p_squash));
                int dp_x = pad_x + (pad_w - dp_w) / 2;
                int dp_y = (H - 40) + (pad_h - dp_h);

                int r_base = dur_laser > 0 ? 0 : sticky_timer > 0 ? 50 : 70;
                int g_base = dur_laser > 0 ? 204 : sticky_timer > 0 ? 140 : 100;
                int b_base = dur_laser > 0 ? 204 : sticky_timer > 0 ? 255 : 170;

                for (int h_idx = 0; h_idx < dp_h; h_idx++) {
                    float t = (float)h_idx / (float)dp_h;
                    float intensity = 0.6f + 0.4f * MyCos(t * 3.14159f * 2.0f);
                    int br = (int)(r_base * intensity); if (br > 255) br = 255;
                    int bg = (int)(g_base * intensity); if (bg > 255) bg = 255;
                    int bb = (int)(b_base * intensity); if (bb > 255) bb = 255;
                    HPEN pPen = CreatePen(PS_SOLID, 1, RGB(br, bg, bb));
                    HGDIOBJ oldP = SelectObject(memDC, pPen);
                    MoveToEx(memDC, dp_x, dp_y + h_idx, NULL);
                    LineTo(memDC, dp_x + dp_w, dp_y + h_idx);
                    SelectObject(memDC, oldP);
                    DeleteObject(pPen);
                }

                HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                HGDIOBJ oldH = SelectObject(memDC, hPen);
                MoveToEx(memDC, dp_x + 2, dp_y + 1, NULL);
                LineTo(memDC, dp_x + dp_w - 2, dp_y + 1);
                MoveToEx(memDC, dp_x + 2, dp_y + dp_h - 2, NULL);
                LineTo(memDC, dp_x + dp_w - 2, dp_y + dp_h - 2);
                SelectObject(memDC, oldH);
                DeleteObject(hPen);

                // Animated Energy Core on Paddle
                int v_pulse = MyAbs(pad_vx) * 2;
                int pulse_anim = (int)(MySin((float)frame_counter * (0.2f + (float)v_pulse * 0.01f)) * (5 + v_pulse));
                int coreWidth = 10 + pulse_anim;
                if (coreWidth < 2) coreWidth = 2;
                if (coreWidth > dp_w) coreWidth = dp_w;
                int cb = 100 + v_pulse * 5; if (cb > 255) cb = 255;
                HBRUSH coreBr = CreateSolidBrush(RGB(cb, 255, 255));
                RECT coreRc = { dp_x + dp_w / 2 - coreWidth / 2, dp_y + dp_h / 2 - 1, dp_x + dp_w / 2 + coreWidth / 2, dp_y + dp_h / 2 + 1 };
                FillRect(memDC, &coreRc, coreBr);
                DeleteObject(coreBr);

                if (dur_laser > 0) {
                    int blink = (frame_counter % 6 < 3);
                    HBRUSH canBr = CreateSolidBrush(blink ? RGB(100, 255, 255) : RGB(0, 200, 200));
                    HGDIOBJ oB = SelectObject(memDC, canBr);
                    HPEN canPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
                    HGDIOBJ oP = SelectObject(memDC, canPen);
                    
                    POINT leftCan[3] = { {pad_x - 2, H - 36}, {pad_x + 6, H - 46}, {pad_x + 14, H - 36} };
                    Polygon(memDC, leftCan, 3);
                    
                    POINT rightCan[3] = { {pad_x + pad_w - 14, H - 36}, {pad_x + pad_w - 6, H - 46}, {pad_x + pad_w + 2, H - 36} };
                    Polygon(memDC, rightCan, 3);
                    
                    SelectObject(memDC, oB);
                    SelectObject(memDC, oP);
                    DeleteObject(canBr);
                    DeleteObject(canPen);
                }

                // Draw Lasers
                HBRUSH lasBr = CreateSolidBrush(RGB(0, 255, 255));
                for (int i = 0; i < MAX_LASERS; i++) {
                    if (lasers[i].active) {
                        RECT rl = { (int)lasers[i].x - 2, (int)lasers[i].y - 5, (int)lasers[i].x + 2, (int)lasers[i].y + 5 };
                        FillRect(memDC, &rl, lasBr);
                    }
                }
                DeleteObject(lasBr);

                // Render Powerup Drop
                if (power_active) {
                    COLORREF pClr = RGB(255, 255, 255);
                    char pBadge[2] = "?";
                    if (power_type == 1) { pClr = RGB(255, 200, 0); pBadge[0] = 'E'; }
                    else if (power_type == 2) { pClr = RGB(50, 220, 50); pBadge[0] = '+'; }
                    else if (power_type == 3) { pClr = RGB(255, 50, 50); pBadge[0] = 'P'; }
                    else if (power_type == 4) { pClr = RGB(50, 100, 255); pBadge[0] = 'S'; }
                    else if (power_type == 5) { pClr = RGB(0, 255, 255); pBadge[0] = 'L'; }
                    else if (power_type == 6) { pClr = RGB(0, 255, 200); pBadge[0] = 'B'; }
                    else if (power_type == 7) { pClr = RGB(255, 0, 255); pBadge[0] = 'M'; }
                    
                    HBRUSH pwBr = CreateSolidBrush(pClr);
                    HGDIOBJ oldB = SelectObject(memDC, pwBr);
                    HPEN pwPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                    HGDIOBJ oldP = SelectObject(memDC, pwPen);
                    
                    if (power_type == 1 || power_type == 4) {
                        RoundRect(memDC, (int)power_x - 8, (int)power_y - 6, (int)power_x + 8, (int)power_y + 6, 6, 6);
                    } else if (power_type == 2 || power_type == 6) {
                        POINT pts[6] = {
                            {(int)power_x, (int)power_y - 8}, {(int)power_x + 8, (int)power_y - 4},
                            {(int)power_x + 8, (int)power_y + 4}, {(int)power_x, (int)power_y + 8},
                            {(int)power_x - 8, (int)power_y + 4}, {(int)power_x - 8, (int)power_y - 4}
                        };
                        Polygon(memDC, pts, 6);
                    } else if (power_type == 3 || power_type == 5) {
                        POINT pts[4] = {
                            {(int)power_x, (int)power_y - 8}, {(int)power_x + 8, (int)power_y},
                            {(int)power_x, (int)power_y + 8}, {(int)power_x - 8, (int)power_y}
                        };
                        Polygon(memDC, pts, 4);
                    } else {
                        Ellipse(memDC, (int)power_x - 8, (int)power_y - 8, (int)power_x + 8, (int)power_y + 8);
                    }
                    
                    SelectObject(memDC, oldB);
                    SelectObject(memDC, oldP);
                    DeleteObject(pwBr);
                    DeleteObject(pwPen);
                    
                    SetTextColor(memDC, RGB(0, 0, 0));
                    SetBkMode(memDC, TRANSPARENT);
                    TextOutA(memDC, (int)power_x - 3, (int)power_y - 7, pBadge, 1);
                    SetTextColor(memDC, RGB(255, 255, 255));
                }

                // Render Particles
                for (int i = 0; i < MAX_PARTICLES; i++) {
                    if (particles[i].life > 0) {
                        COLORREF c = particles[i].type == 1 ? RGB(255, 255, 255) : particles[i].color;
                        if (particles[i].type == 2) c = RGB(255, 255, 0); // extra glow/flash
                        HBRUSH pBr = CreateSolidBrush(c);
                        int s = particles[i].type == 1 ? 2 : (particles[i].type == 2 ? 6 : 4);
                        if (particles[i].type == 2) {
                            HPEN nonePen = CreatePen(PS_NULL, 0, 0);
                            HGDIOBJ oP = SelectObject(memDC, nonePen);
                            HGDIOBJ oB = SelectObject(memDC, pBr);
                            Ellipse(memDC, (int)particles[i].x - s/2, (int)particles[i].y - s/2, (int)particles[i].x + s/2, (int)particles[i].y + s/2);
                            SelectObject(memDC, oP);
                            SelectObject(memDC, oB);
                            DeleteObject(nonePen);
                        } else {
                            RECT prc = { (int)particles[i].x, (int)particles[i].y, (int)particles[i].x + s, (int)particles[i].y + s };
                            FillRect(memDC, &prc, pBr);
                        }
                        DeleteObject(pBr);
                    }
                }

                // Render Shockwaves
                for (int i = 0; i < MAX_SHOCKWAVES; i++) {
                    if (shockwaves[i].life > 0) {
                        HPEN swPen = CreatePen(PS_SOLID, 2 + (int)(shockwaves[i].life * 4.0f), shockwaves[i].color);
                        HGDIOBJ oldP = SelectObject(memDC, swPen);
                        HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
                        HGDIOBJ oldB = SelectObject(memDC, nullBr);
                        
                        int rx = (int)shockwaves[i].r;
                        int ry = (int)(shockwaves[i].r * 0.6f); // 3D Perspective Squish
                        Ellipse(memDC, (int)shockwaves[i].x - rx, (int)shockwaves[i].y - ry,
                                       (int)shockwaves[i].x + rx, (int)shockwaves[i].y + ry);
                        
                        SelectObject(memDC, oldP);
                        SelectObject(memDC, oldB);
                        DeleteObject(swPen);
                    }
                }

                // Render Balls & Trails
                for (int i = 0; i < MAX_BALLS; i++) {
                    if (balls[i].active) {
                        COLORREF tBaseClr = dur_fire > 0 ? RGB(255, 100, 0) : RGB(0, 200, 255);
                        for (int t = 0; t < MAX_TRAIL; t++) {
                            int idx = (balls[i].trail_head + t) % MAX_TRAIL;
                            float tx = balls[i].trail[idx].x;
                            float ty = balls[i].trail[idx].y;
                            if (tx != 0 || ty != 0) {
                                int intens = (t * 255) / MAX_TRAIL;
                                int r = (GetRValue(tBaseClr) * intens) / 255;
                                int g = (GetGValue(tBaseClr) * intens) / 255;
                                int b = (GetBValue(tBaseClr) * intens) / 255;
                                HBRUSH tBr = CreateSolidBrush(RGB(r, g, b));
                                HPEN nonePen = CreatePen(PS_NULL, 0, 0);
                                HGDIOBJ oP = SelectObject(memDC, nonePen);
                                HGDIOBJ oB = SelectObject(memDC, tBr);
                                int ts = 2 + (t * 6) / MAX_TRAIL;
                                Ellipse(memDC, (int)tx + 4 - ts/2, (int)ty + 4 - ts/2, (int)tx + 4 + ts/2, (int)ty + 4 + ts/2);
                                SelectObject(memDC, oP);
                                SelectObject(memDC, oB);
                                DeleteObject(nonePen);
                                DeleteObject(tBr);
                            }
                        }
                    
                        COLORREF bClr = dur_fire > 0 ? RGB(255, 50, 0) : RGB(0, 255, 255);
                        HBRUSH bBr = CreateSolidBrush(bClr);
                        HGDIOBJ oB = SelectObject(memDC, bBr);
                        HPEN bPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                        HGDIOBJ oP = SelectObject(memDC, bPen);
                        Ellipse(memDC, (int)balls[i].x, (int)balls[i].y, (int)balls[i].x + 8, (int)balls[i].y + 8);
                        SelectObject(memDC, oB);
                        SelectObject(memDC, oP);
                        DeleteObject(bBr);
                        DeleteObject(bPen);
                        
                        // Animated spin core
                        HPEN wPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                        HGDIOBJ oP = SelectObject(memDC, wPen);
                        int bx = (int)balls[i].x + 4;
                        int by = (int)balls[i].y + 4;
                        if ((frame_counter % 8) < 4) {
                            MoveToEx(memDC, bx - 2, by, NULL); LineTo(memDC, bx + 2, by);
                            MoveToEx(memDC, bx, by - 2, NULL); LineTo(memDC, bx, by + 2);
                        } else {
                            MoveToEx(memDC, bx - 2, by - 2, NULL); LineTo(memDC, bx + 2, by + 2);
                            MoveToEx(memDC, bx - 2, by + 2, NULL); LineTo(memDC, bx + 2, by - 2);
                        }
                        SelectObject(memDC, oP);
                        DeleteObject(wPen);
                    }
                }

                // Render Active Skill Status Bar at Bottom
                char skStr[128];
                wsprintfA(skStr, "[L]Laser:%s [M]Split:%s [F]Fire:%s [B]Barrier:%s",
                    dur_laser > 0 ? "ACT" : cd_laser <= 0 ? "RDY" : "CD",
                    cd_multi <= 0 ? "RDY" : "CD",
                    dur_fire > 0 ? "ACT" : cd_fire <= 0 ? "RDY" : "CD",
                    dur_barrier > 0 ? "ACT" : cd_barrier <= 0 ? "RDY" : "CD");
                TextOutA(memDC, 10, H - 16, skStr, lstrlenA(skStr));
            }
            
            // HUD Top Bar 3D Beveled Display
            SetViewportOrgEx(memDC, 0, 0, NULL); // Reset shake for UI

            HBRUSH uiBg = CreateSolidBrush(RGB(30, 30, 45));
            RECT uiRc = { 5, 2, W - 5, 28 };
            FillRect(memDC, &uiRc, uiBg);
            DeleteObject(uiBg);

            HPEN lPen = CreatePen(PS_SOLID, 1, RGB(90, 90, 120));
            HPEN dPen = CreatePen(PS_SOLID, 1, RGB(10, 10, 20));
            SelectObject(memDC, lPen);
            MoveToEx(memDC, 5, 28, NULL); LineTo(memDC, 5, 2); LineTo(memDC, W - 5, 2);
            SelectObject(memDC, dPen);
            LineTo(memDC, W - 5, 28); LineTo(memDC, 5, 28);
            
            char scStr[32], lvStr[32], infoStr[64];
            wsprintfA(scStr, "SCORE: %d", score);
            wsprintfA(lvStr, "LIVES: %d", lives);
            if (level == 99) wsprintfA(infoStr, "STG: CSTM  HI: %d", high_score);
            else wsprintfA(infoStr, "STG: %d  HI: %d", level, high_score);
            
            SetBkMode(memDC, TRANSPARENT);
            DrawBevelBox(memDC, 10, 5, 100, 20, scStr, lPen, dPen);
            DrawBevelBox(memDC, 115, 5, 75, 20, lvStr, lPen, dPen);
            SetTextColor(memDC, RGB(200, 200, 200));
            TextOutA(memDC, 200, 8, infoStr, lstrlenA(infoStr));
            
            DeleteObject(lPen);
            DeleteObject(dPen);
            
            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, hOld);
            DeleteObject(hbm);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            SaveHighScore();
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
    wc.lpszClassName = "KBreakoutApp";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KBreakoutApp", "KBreakout", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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
