#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int _fltused = 1;

void* memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) {
        *bytes++ = (char)c;
    }
    return dest;
}

#define W 400
#define H 450
#define TIMER_ID 1

#define ROWS 6
#define COLS 10
#define BR_W (W / COLS)
#define BR_H 16

#define MAX_TRAIL 8
typedef struct { float x, y; } TrailPoint;

#define MAX_BALLS 32
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

#define MAX_LASERS 32
typedef struct {
    float x, y;
    float vx, vy;
    int active;
    int type; // 0=normal, 1=valkyrie, 2=refracted prism laser
} Laser;
Laser lasers[MAX_LASERS];

#define MAX_PARTICLES 256
typedef struct {
    float x, y;
    float prev_x, prev_y;
    float vx, vy;
    int life;
    int max_life;
    COLORREF color;
    int type; // 0=needle spark, 1=buoyant smoke, 2=debris shard, 3=celebration star
    float rot;
    float vrot;
    int size;
} Particle;
Particle particles[MAX_PARTICLES];

#define MAX_SHOCKWAVES 24
typedef struct {
    float x, y;
    float r;
    float max_r;
    float life;
    COLORREF color;
    int is_inner;
} Shockwave;
Shockwave shockwaves[MAX_SHOCKWAVES];

int screen_shake = 0;
float screen_shake_intensity = 0.0f;
float screen_shake_angle = 0.0f;

static const int sin_tab16[16] = { 0, 38, 71, 92, 100, 92, 71, 38, 0, -38, -71, -92, -100, -92, -71, -38 };
static const int cos_tab16[16] = { 100, 92, 71, 38, 0, -38, -71, -92, -100, -92, -71, -38, 0, 38, 71, 92 };

static int FastSin(int idx) {
    idx = ((idx % 16) + 16) % 16;
    return sin_tab16[idx];
}
static int FastCos(int idx) {
    idx = ((idx % 16) + 16) % 16;
    return cos_tab16[idx];
}

void TriggerScreenShake(float intensity, float rotational_force) {
    if (intensity > screen_shake_intensity) {
        screen_shake_intensity = intensity;
        screen_shake_angle = ((float)(MyRand() % 101 - 50) / 50.0f) * rotational_force * intensity;
    }
    if ((int)intensity > screen_shake) screen_shake = (int)intensity;
}

#define MAX_DUST_MOTES 28
typedef struct {
    float x, y;
    float vx, vy;
    int size;
    COLORREF color;
} DustMote;
DustMote dust_motes[MAX_DUST_MOTES];
int dust_motes_init = 0;

void InitDustMotes() {
    for (int i = 0; i < MAX_DUST_MOTES; i++) {
        dust_motes[i].x = (float)(MyRand() % W);
        dust_motes[i].y = 35.0f + (float)(MyRand() % (H - 40));
        dust_motes[i].vx = ((float)(MyRand() % 11 - 5)) / 10.0f;
        dust_motes[i].vy = 0.3f + ((float)(MyRand() % 10)) / 10.0f;
        dust_motes[i].size = 1 + (MyRand() % 2);
        dust_motes[i].color = (MyRand() % 2 == 0) ? RGB(0, 240, 240) : RGB(255, 215, 0);
    }
    dust_motes_init = 1;
}

void UpdateDustMotes() {
    if (!dust_motes_init) InitDustMotes();
    for (int i = 0; i < MAX_DUST_MOTES; i++) {
        dust_motes[i].x += dust_motes[i].vx;
        dust_motes[i].y += dust_motes[i].vy;
        if (dust_motes[i].y > H - 10) {
            dust_motes[i].y = 35.0f;
            dust_motes[i].x = (float)(MyRand() % W);
        }
        if (dust_motes[i].x < 0) dust_motes[i].x = (float)W;
        if (dust_motes[i].x > W) dust_motes[i].x = 0.0f;
    }
}

#define MAX_METEORS 6
typedef struct { float x, y; int active; } Meteor;
Meteor meteors[MAX_METEORS];
int meteor_active = 0;

// Gravity Wells (Black Holes & Pulsar Repulsors)
#define MAX_GRAVITY_WELLS 3
typedef struct {
    float x, y;
    float mass;
    float radius;
    int active;
    int type;
    float spin;
} GravityWell;
GravityWell gravity_wells[MAX_GRAVITY_WELLS];

// Orbital Satellite Barriers System
#define MAX_SATELLITES 2
typedef struct {
    float angle;
    float dist;
    float x, y;
    int active;
} Satellite;
Satellite satellites[MAX_SATELLITES];
int sat_active = 1;
int dur_satellite = 0, cd_satellite = 0;

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
int state = 0; // 0=start, 1=play, 2=gameover, 3=victory, 4=editor, 5=forge
int diff = 0; // 0=Easy, 1=Hard
float speed = 3.5f;
int lives = 3;
int level = 1;

// Multi-Ball Chaos System
int chaos_mode = 0;

// Materials & Cyber-Forge System
int plasma_shards = 0;
int quantum_cores = 0;
int nano_alloys = 0;

int forge_supernova = 0;
int forge_chronos = 0;
int forge_valkyrie = 0;
int forge_aegis = 0;
int forge_singularity = 0;
int forge_satellite = 0;
int forge_resonance = 0;

int active_supernova = 0;
int active_chronos = 0;
int active_valkyrie = 0;
int aegis_layers = 0;
int active_singularity = 0;
int active_satellite_boost = 0;
int active_resonance_catalyst = 0;
char forge_msg[64] = "Select recipe 1-7 to synthesize.";

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
int cd_gravity = 0, dur_gravity = 0;

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

// Boss Fortress
int boss_active = 0;
int boss_type = 1;
int boss_dx = 0;
int boss_hp = 50, boss_max_hp = 50;
int boss_x = W / 2 - 50, boss_y = 35, boss_w = 100, boss_h = 40;
float boss_shield_angle = 0.0f;
#define MAX_BOSS_BULLETS 6
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
static float MyAbsF(float x) { return x < 0.0f ? -x : x; }

static float MySqrt(float x) {
    if (x <= 0.0f) return 0.0f;
    float r = x > 1.0f ? x * 0.5f : 1.0f;
    for (int i = 0; i < 8; i++) {
        r = 0.5f * (r + x / r);
    }
    return r;
}

static float MySin(float x) {
    const float PI2 = 6.283185f;
    while (x < 0.0f) x += PI2;
    while (x >= PI2) x -= PI2;
    float t = x * 4.0f / PI2;
    if (t < 1.0f) return t;
    if (t < 3.0f) return 2.0f - t;
    return t - 4.0f;
}
static float MyCos(float x) { return MySin(x + 1.570796f); }

void SpawnParticles(float x, float y, COLORREF color, int count) {
    int spawned = 0;
    // Layer 0: Incandescent core needle sparks with velocity trails
    int c0 = count * 35 / 100; if (c0 < 2) c0 = 2;
    for (int i = 0; i < c0 && spawned < MAX_PARTICLES; i++) {
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (particles[p].life <= 0) {
                particles[p].x = x; particles[p].y = y;
                particles[p].prev_x = x; particles[p].prev_y = y;
                particles[p].vx = ((float)(MyRand() % 101 - 50)) / 10.0f;
                particles[p].vy = ((float)(MyRand() % 101 - 50)) / 10.0f - 1.5f;
                particles[p].life = 12 + (MyRand() % 10);
                particles[p].max_life = particles[p].life;
                particles[p].color = RGB(255, 255, 255);
                particles[p].type = 0;
                particles[p].size = 1 + (MyRand() % 2);
                spawned++;
                break;
            }
        }
    }
    // Layer 1: Expanding buoyant plasma smoke puffs with negative gravity and drag
    int c1 = count * 30 / 100; if (c1 < 2) c1 = 2;
    for (int i = 0; i < c1 && spawned < MAX_PARTICLES; i++) {
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (particles[p].life <= 0) {
                particles[p].x = x; particles[p].y = y;
                particles[p].prev_x = x; particles[p].prev_y = y;
                particles[p].vx = ((float)(MyRand() % 61 - 30)) / 15.0f;
                particles[p].vy = ((float)(MyRand() % 61 - 30)) / 15.0f - 1.0f;
                particles[p].life = 20 + (MyRand() % 15);
                particles[p].max_life = particles[p].life;
                particles[p].color = color;
                particles[p].type = 1;
                particles[p].size = 3 + (MyRand() % 3);
                spawned++;
                break;
            }
        }
    }
    // Layer 2: Heavy kinematic debris & glass/brick shards with floor bounce & rotation
    int c2 = count * 25 / 100; if (c2 < 2) c2 = 2;
    for (int i = 0; i < c2 && spawned < MAX_PARTICLES; i++) {
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (particles[p].life <= 0) {
                particles[p].x = x; particles[p].y = y;
                particles[p].prev_x = x; particles[p].prev_y = y;
                particles[p].vx = ((float)(MyRand() % 101 - 50)) / 12.0f;
                particles[p].vy = ((float)(MyRand() % 101 - 50)) / 12.0f - 2.5f;
                particles[p].life = 24 + (MyRand() % 16);
                particles[p].max_life = particles[p].life;
                particles[p].color = color;
                particles[p].type = 2;
                particles[p].size = 3 + (MyRand() % 3);
                particles[p].rot = ((float)(MyRand() % 314)) / 50.0f;
                particles[p].vrot = ((float)(MyRand() % 41 - 20)) / 100.0f;
                spawned++;
                break;
            }
        }
    }
    // Layer 3: Radiant golden/cyan celebration energy stars
    int c3 = count * 15 / 100; if (c3 < 1) c3 = 1;
    for (int i = 0; i < c3 && spawned < MAX_PARTICLES; i++) {
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (particles[p].life <= 0) {
                particles[p].x = x; particles[p].y = y;
                particles[p].prev_x = x; particles[p].prev_y = y;
                particles[p].vx = ((float)(MyRand() % 81 - 40)) / 15.0f;
                particles[p].vy = ((float)(MyRand() % 81 - 40)) / 15.0f;
                particles[p].life = 20 + (MyRand() % 12);
                particles[p].max_life = particles[p].life;
                particles[p].color = (MyRand() % 2 == 0) ? RGB(255, 215, 0) : RGB(0, 255, 255);
                particles[p].type = 3;
                particles[p].size = 4 + (MyRand() % 3);
                particles[p].rot = ((float)(MyRand() % 314)) / 50.0f;
                particles[p].vrot = ((float)(MyRand() % 31 - 15)) / 100.0f;
                spawned++;
                break;
            }
        }
    }
}

// Dual-Tier Concentric Shockwave Ripple Rings
void SpawnShockwave(float x, float y, COLORREF color) {
    // Inner high-speed wave
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (shockwaves[i].life <= 0.0f) {
            shockwaves[i].x = x; shockwaves[i].y = y;
            shockwaves[i].r = 2.0f; shockwaves[i].max_r = 26.0f + (MyRand() % 8);
            shockwaves[i].life = 1.0f; shockwaves[i].color = RGB(255, 255, 255);
            shockwaves[i].is_inner = 1;
            break;
        }
    }
    // Outer dispersion halo
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (shockwaves[i].life <= 0.0f) {
            shockwaves[i].x = x; shockwaves[i].y = y;
            shockwaves[i].r = 4.0f; shockwaves[i].max_r = 44.0f + (MyRand() % 16);
            shockwaves[i].life = 1.0f; shockwaves[i].color = color;
            shockwaves[i].is_inner = 0;
            break;
        }
    }
}

void SpawnGravityWell(float x, float y, float mass, int type) {
    for (int i = 0; i < MAX_GRAVITY_WELLS; i++) {
        if (!gravity_wells[i].active) {
            gravity_wells[i].active = 1;
            gravity_wells[i].x = x;
            gravity_wells[i].y = y;
            gravity_wells[i].mass = mass;
            gravity_wells[i].radius = 110.0f;
            gravity_wells[i].type = type;
            gravity_wells[i].spin = 0.0f;
            SpawnShockwave(x, y, type == 0 ? RGB(160, 32, 240) : RGB(0, 255, 255));
            break;
        }
    }
}

void SpawnLaser(float x, float y, float vx, float vy, int type) {
    for (int i = 0; i < MAX_LASERS; i++) {
        if (!lasers[i].active) {
            lasers[i].active = 1;
            lasers[i].x = x;
            lasers[i].y = y;
            lasers[i].vx = vx;
            lasers[i].vy = vy;
            lasers[i].type = type;
            break;
        }
    }
}

void UpdateParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0) {
            particles[i].prev_x = particles[i].x;
            particles[i].prev_y = particles[i].y;
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            if (particles[i].type == 0) {
                particles[i].vy += 0.16f;
            } else if (particles[i].type == 1) {
                particles[i].vy -= 0.035f;
                particles[i].vx *= 0.93f;
                particles[i].vy *= 0.93f;
            } else if (particles[i].type == 2) {
                particles[i].vy += 0.24f;
                particles[i].vx *= 0.97f;
                particles[i].rot += particles[i].vrot;
                if (particles[i].y >= H - 12) {
                    particles[i].y = (float)(H - 12);
                    particles[i].vy = -particles[i].vy * 0.45f;
                }
            } else if (particles[i].type == 3) {
                particles[i].vx *= 0.95f;
                particles[i].vy *= 0.95f;
                particles[i].rot += particles[i].vrot;
            }
            particles[i].life--;
        }
    }
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (shockwaves[i].life > 0.0f) {
            shockwaves[i].r += (shockwaves[i].is_inner ? 3.0f : 1.8f);
            shockwaves[i].life -= (shockwaves[i].is_inner ? 0.055f : 0.032f);
        }
    }
    UpdateDustMotes();
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
    if (type == 11) return RGB(160, 40, 240); // Quantum Resonance
    if (type == 10) return RGB(180, 240, 255); // Laser Reflector Prism
    if (type == 9) return RGB(120, 135, 150);
    if (type == 8) return RGB(80, 80, 80);
    if (type == 7) return RGB(200, 255, 255);
    if (type == 2) return RGB(255, 65, 65);
    if (type == 3) return RGB(184, 115, 51);
    if (type == 4) return RGB(255, 100, 0);
    if (type == 5) return RGB(170, 0, 255);
    if (type == 6) return RGB(0, 255, 204);
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

    // Sculpted brick glossy specular sheen highlight traversing block bevels
    int sheen_phase = (int)((frame_counter * 2 + c * 8 + r * 14) % 180);
    if (sheen_phase >= 0 && sheen_phase <= BR_W + 8) {
        HPEN sheenPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        HGDIOBJ oSheenP = SelectObject(hdc, sheenPen);
        int sx1 = bx + sheen_phase - 4;
        int sx2 = bx + sheen_phase + 4;
        if (sx1 >= bx + 1 && sx2 <= bx + BR_W - 2) {
            MoveToEx(hdc, sx1, by + BR_H - 3, NULL);
            LineTo(hdc, sx2, by + 2);
        }
        SelectObject(hdc, oSheenP);
        DeleteObject(sheenPen);
    }

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

    if (type == 11) {
        // Quantum Resonance Brick: pulsing core + concentric harmonic rings
        int pulse = (frame_counter % 20 < 10) ? 1 : 0;
        HPEN qPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
        HGDIOBJ oldQ = SelectObject(hdc, qPen);
        MoveToEx(hdc, bx + 5 - pulse, by + BR_H / 2, NULL);
        LineTo(hdc, bx + BR_W / 2, by + 2 - pulse);
        LineTo(hdc, bx + BR_W - 5 + pulse, by + BR_H / 2);
        LineTo(hdc, bx + BR_W / 2, by + BR_H - 3 + pulse);
        LineTo(hdc, bx + 5 - pulse, by + BR_H / 2);
        
        HBRUSH qCore = CreateSolidBrush(RGB(255, 200, 255));
        RECT qr = { bx + BR_W/2 - 2, by + BR_H/2 - 2, bx + BR_W/2 + 2, by + BR_H/2 + 2 };
        FillRect(hdc, &qr, qCore);
        DeleteObject(qCore);
        SelectObject(hdc, oldQ);
        DeleteObject(qPen);
    } else if (type == 10) {
        // Laser Reflector Prism: Refraction prism facets
        HPEN refPen = CreatePen(PS_SOLID, 1, RGB(0, 200, 255));
        HGDIOBJ oldR = SelectObject(hdc, refPen);
        MoveToEx(hdc, bx + 3, by + BR_H - 3, NULL);
        LineTo(hdc, bx + BR_W / 2, by + 2);
        LineTo(hdc, bx + BR_W - 3, by + BR_H - 3);
        MoveToEx(hdc, bx + BR_W / 2, by + 2, NULL);
        LineTo(hdc, bx + BR_W / 2, by + BR_H - 2);
        SelectObject(hdc, oldR);
        DeleteObject(refPen);
    } else if (type == 9) {
        HPEN steelPen = CreatePen(PS_SOLID, 1, RGB(210, 220, 230));
        SelectObject(hdc, steelPen);
        MoveToEx(hdc, bx + 4, by + 3, NULL); LineTo(hdc, bx + BR_W - 4, by + BR_H - 3);
        MoveToEx(hdc, bx + BR_W - 4, by + 3, NULL); LineTo(hdc, bx + 4, by + BR_H - 3);
        DeleteObject(steelPen);
    } else if (type == 3) {
        HPEN stripePen = CreatePen(PS_SOLID, 1, RGB(255, 220, 160));
        SelectObject(hdc, stripePen);
        MoveToEx(hdc, bx + 6, by + 2, NULL); LineTo(hdc, bx + 6, by + BR_H - 2);
        MoveToEx(hdc, bx + BR_W - 6, by + 2, NULL); LineTo(hdc, bx + BR_W - 6, by + BR_H - 2);
        DeleteObject(stripePen);
    } else if (type == 4) {
        HBRUSH yBr = CreateSolidBrush(RGB(255, 255, 0));
        SelectObject(hdc, yBr);
        Ellipse(hdc, bx + BR_W/2 - 3, by + BR_H/2 - 3, bx + BR_W/2 + 3, by + BR_H/2 + 3);
        DeleteObject(yBr);
    } else if (type == 6) {
        SetTextColor(hdc, RGB(0, 0, 0));
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, bx + BR_W/2 - 3, by + 1, "?", 1);
    } else if (type == 7) {
        HPEN phantomPen = CreatePen(PS_DOT, 1, RGB(255, 255, 255));
        SelectObject(hdc, phantomPen);
        MoveToEx(hdc, bx + 2, by + 2, NULL); LineTo(hdc, bx + BR_W - 2, by + BR_H - 2);
        DeleteObject(phantomPen);
    } else if (type == 8) {
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
                    int isRef = (bricks[nr][nc] == 10);
                    int isRes = (bricks[nr][nc] == 11);
                    bricks[nr][nc] = 0;
                    bricks_left--;
                    score += 15;
                    lifetime_bricks++;
                    plasma_shards += 1;
                    if (isExp && (nr != r || nc != c)) {
                        TriggerExplosion(nr, nc);
                    }
                    if (isRef) {
                        SpawnLaser((float)bx, (float)by, -6.0f, -2.0f, 2);
                        SpawnLaser((float)bx, (float)by, 6.0f, -2.0f, 2);
                    }
                }
            }
        }
    }
}

void DrawHUDCornerReticles(HDC hdc) {
    const int size = 16;
    int diode_pulse = FastSin((frame_counter * 3) % 16);
    COLORREF diode_clr = (diode_pulse > 0) ? RGB(0, 255, 220) : RGB(0, 180, 160);
    if (chaos_mode) diode_clr = (diode_pulse > 0) ? RGB(255, 60, 255) : RGB(180, 40, 180);

    HPEN brPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 255));
    HPEN tickPen = CreatePen(PS_SOLID, 1, RGB(0, 180, 220));
    HBRUSH dBr = CreateSolidBrush(diode_clr);
    HGDIOBJ oP = SelectObject(hdc, brPen);
    HGDIOBJ oB = SelectObject(hdc, dBr);

    int corners[4][4] = {
        { 5, 33, 1, 1 }, { W - 5, 33, -1, 1 },
        { 5, H - 5, 1, -1 }, { W - 5, H - 5, -1, -1 }
    };

    for (int i = 0; i < 4; i++) {
        int cx = corners[i][0], cy = corners[i][1];
        int dx = corners[i][2], dy = corners[i][3];

        // Outer bracket arm
        SelectObject(hdc, brPen);
        MoveToEx(hdc, cx, cy + dy * size, NULL);
        LineTo(hdc, cx, cy);
        LineTo(hdc, cx + dx * size, cy);

        // Tech notch tick
        SelectObject(hdc, tickPen);
        MoveToEx(hdc, cx + dx * 6, cy + dy * 3, NULL);
        LineTo(hdc, cx + dx * 12, cy + dy * 3);

        // Glowing status diode
        SelectObject(hdc, dBr);
        int px = cx + dx * 3, py = cy + dy * 3;
        Ellipse(hdc, px - 2, py - 2, px + 2, py + 2);
        SetPixel(hdc, px, py, RGB(255, 255, 255));
    }

    SelectObject(hdc, oP);
    SelectObject(hdc, oB);
    DeleteObject(brPen);
    DeleteObject(tickPen);
    DeleteObject(dBr);
}

void DrawPerimeterInlay(HDC hdc) {
    int bx = 3, by = 31, bw = W - 6, bh = H - 34;
    int pulse = FastSin((frame_counter * 2) % 16);
    int clr_val = 140 + (pulse * 60) / 100;
    COLORREF pClr = chaos_mode ? RGB(clr_val, 30, clr_val) : RGB(0, clr_val / 2, clr_val);
    
    HPEN pPen = CreatePen(PS_SOLID, 1, pClr);
    HGDIOBJ oP = SelectObject(hdc, pPen);
    HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
    HGDIOBJ oB = SelectObject(hdc, nullBr);
    Rectangle(hdc, bx, by, bx + bw, by + bh);

    // Traveling specular glint comet packet along perimeter
    int perim = 2 * (bw + bh);
    if (perim > 0) {
        int glint_dist = (frame_counter * 4) % perim;
        int gx = bx, gy = by;
        if (glint_dist < bw) {
            gx = bx + glint_dist; gy = by;
        } else if (glint_dist < bw + bh) {
            gx = bx + bw; gy = by + (glint_dist - bw);
        } else if (glint_dist < 2 * bw + bh) {
            gx = bx + bw - (glint_dist - (bw + bh)); gy = by + bh;
        } else {
            gx = bx; gy = by + bh - (glint_dist - (2 * bw + bh));
        }

        HBRUSH gBr = CreateSolidBrush(RGB(255, 255, 255));
        HGDIOBJ oG = SelectObject(hdc, gBr);
        HPEN nonePen = CreatePen(PS_NULL, 0, 0);
        HGDIOBJ oNP = SelectObject(hdc, nonePen);
        Ellipse(hdc, gx - 3, gy - 3, gx + 3, gy + 3);
        SelectObject(hdc, oG);
        SelectObject(hdc, oNP);
        DeleteObject(nonePen);
        DeleteObject(gBr);
    }

    SelectObject(hdc, oP);
    SelectObject(hdc, oB);
    DeleteObject(pPen);
}

void DrawSpeedDustMotes(HDC hdc) {
    if (!dust_motes_init) InitDustMotes();
    for (int i = 0; i < MAX_DUST_MOTES; i++) {
        int mx = (int)dust_motes[i].x;
        int my = (int)dust_motes[i].y;
        SetPixel(hdc, mx, my, dust_motes[i].color);
        if (dust_motes[i].size > 1) {
            SetPixel(hdc, mx + 1, my, RGB(255, 255, 255));
        }
    }
}

void TriggerQuantumResonance(int orig_r, int orig_c) {
    int chain = 0;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (bricks[r][c] == 11) {
                chain++;
                bricks[r][c] = 0;
                bricks_left--;
                int bx = c * BR_W + BR_W / 2;
                int by = r * BR_H + 35 + BR_H / 2;
                SpawnParticles((float)bx, (float)by, RGB(180, 50, 255), 14);
                SpawnParticles((float)bx, (float)by, RGB(0, 255, 255), 10);
                SpawnShockwave((float)bx, (float)by, RGB(180, 50, 255));
                score += 35 * chain;
                lifetime_bricks++;
                quantum_cores += 2;
            }
        }
    }
    if (chain > 0) {
        if (screen_shake < 12) screen_shake = 12;
        MessageBeep(MB_ICONASTERISK);
    }
}

void LoadHighScore() {
    HANDLE hFile = CreateFileA("kbreakout_hi.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD read;
        int buf[12];
        for (int k = 0; k < 12; k++) buf[k] = 0;
        if (ReadFile(hFile, buf, sizeof(buf), &read, NULL)) {
            high_score = buf[0];
            lifetime_bricks = buf[1];
            plasma_shards = buf[2];
            quantum_cores = buf[3];
            nano_alloys = buf[4];
            forge_supernova = buf[5];
            forge_chronos = buf[6];
            forge_valkyrie = buf[7];
            forge_aegis = buf[8];
            forge_singularity = buf[9];
            forge_satellite = buf[10];
            forge_resonance = buf[11];
        }
        CloseHandle(hFile);
    }
}

void SaveHighScore() {
    if (score > high_score) high_score = score;
    HANDLE hFile = CreateFileA("kbreakout_hi.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        int buf[12] = {
            high_score, lifetime_bricks,
            plasma_shards, quantum_cores, nano_alloys,
            forge_supernova, forge_chronos, forge_valkyrie, forge_aegis, forge_singularity,
            forge_satellite, forge_resonance
        };
        WriteFile(hFile, buf, sizeof(buf), &written, NULL);
        CloseHandle(hFile);
    }
}

void SplitBall(int idx) {
    if (!balls[idx].active) return;
    for (int n = 0; n < 2; n++) {
        for (int k = 0; k < MAX_BALLS; k++) {
            if (!balls[k].active) {
                balls[k].active = 1;
                balls[k].x = balls[idx].x;
                balls[k].y = balls[idx].y;
                balls[k].dx = balls[idx].dx + (n == 0 ? 2.2f : -2.2f);
                balls[k].dy = balls[idx].dy;
                balls[k].stuck = 0;
                balls[k].trail_head = 0;
                for (int t = 0; t < MAX_TRAIL; t++) { balls[k].trail[t].x = 0; balls[k].trail[t].y = 0; }
                break;
            }
        }
    }
}

void UseSkill(char skill) {
    if (state != 1) return;
    if ((skill == 'L' || skill == 'l') && cd_laser <= 0) {
        dur_laser = 360; cd_laser = 720;
        MessageBeep(MB_OK);
    } else if ((skill == 'M' || skill == 'm') && cd_multi <= 0) {
        cd_multi = 900;
        for (int i = 0; i < MAX_BALLS; i++) {
            if (balls[i].active) {
                SplitBall(i);
                break;
            }
        }
        MessageBeep(MB_OK);
    } else if ((skill == 'F' || skill == 'f') && cd_fire <= 0) {
        dur_fire = 480; cd_fire = 900;
        MessageBeep(MB_OK);
    } else if ((skill == 'B' || skill == 'b') && cd_barrier <= 0) {
        dur_barrier = 600; cd_barrier = 1080;
        MessageBeep(MB_OK);
    } else if ((skill == 'G' || skill == 'g') && cd_gravity <= 0) {
        dur_gravity = 400; cd_gravity = 800;
        SpawnGravityWell((float)(W / 2), 160.0f, 35.0f, 0);
        MessageBeep(MB_ICONASTERISK);
    } else if ((skill == 'S' || skill == 's') && cd_satellite <= 0) {
        dur_satellite = 480; cd_satellite = 840;
        SpawnShockwave((float)(pad_x + pad_w / 2), (float)(H - 40), RGB(0, 255, 200));
        MessageBeep(MB_OK);
    }
}

void CraftForge(int recipe) {
    if (recipe == 1) {
        if (plasma_shards >= 15 && nano_alloys >= 10) {
            plasma_shards -= 15; nano_alloys -= 10;
            forge_supernova++;
            active_supernova = 1; dur_fire = 600;
            wsprintfA(forge_msg, "Synthesized NOVA BLAST CORE! (Active)");
            MessageBeep(MB_OK);
        } else {
            wsprintfA(forge_msg, "Need 15 Plasma + 10 Alloy!");
            MessageBeep(0xFFFFFFFF);
        }
    } else if (recipe == 2) {
        if (plasma_shards >= 12 && quantum_cores >= 8) {
            plasma_shards -= 12; quantum_cores -= 8;
            forge_chronos++;
            active_chronos = 1;
            wsprintfA(forge_msg, "Synthesized CHRONOS PADDLE! (Active)");
            MessageBeep(MB_OK);
        } else {
            wsprintfA(forge_msg, "Need 12 Plasma + 8 Quantum!");
            MessageBeep(0xFFFFFFFF);
        }
    } else if (recipe == 3) {
        if (quantum_cores >= 10 && nano_alloys >= 15) {
            quantum_cores -= 10; nano_alloys -= 15;
            forge_valkyrie++;
            active_valkyrie = 1; dur_laser = 600;
            wsprintfA(forge_msg, "Synthesized VALKYRIE TURRETS! (Active)");
            MessageBeep(MB_OK);
        } else {
            wsprintfA(forge_msg, "Need 10 Quantum + 15 Alloy!");
            MessageBeep(0xFFFFFFFF);
        }
    } else if (recipe == 4) {
        if (plasma_shards >= 10 && quantum_cores >= 10 && nano_alloys >= 10) {
            plasma_shards -= 10; quantum_cores -= 10; nano_alloys -= 10;
            forge_aegis++;
            aegis_layers += 3; dur_barrier = 800;
            wsprintfA(forge_msg, "Synthesized QUANTUM AEGIS! (+3 Layers)");
            MessageBeep(MB_OK);
        } else {
            wsprintfA(forge_msg, "Need 10 Plasma + 10 Quantum + 10 Alloy!");
            MessageBeep(0xFFFFFFFF);
        }
    } else if (recipe == 5) {
        if (plasma_shards >= 20 && quantum_cores >= 15 && nano_alloys >= 15) {
            plasma_shards -= 20; quantum_cores -= 15; nano_alloys -= 15;
            forge_singularity++;
            active_singularity = 1;
            SpawnGravityWell((float)(W / 2), 140.0f, 45.0f, 0);
            wsprintfA(forge_msg, "Synthesized SINGULARITY BEACON!");
            MessageBeep(MB_OK);
        } else {
            wsprintfA(forge_msg, "Need 20 Plasma + 15 Quantum + 15 Alloy!");
            MessageBeep(0xFFFFFFFF);
        }
    } else if (recipe == 6) {
        if (plasma_shards >= 12 && quantum_cores >= 12 && nano_alloys >= 12) {
            plasma_shards -= 12; quantum_cores -= 12; nano_alloys -= 12;
            forge_satellite++;
            active_satellite_boost = 1;
            dur_satellite = 700;
            wsprintfA(forge_msg, "Synthesized ORBITAL SATELLITE ARRAY!");
            MessageBeep(MB_OK);
        } else {
            wsprintfA(forge_msg, "Need 12 Pls + 12 Qtm + 12 Aly!");
            MessageBeep(0xFFFFFFFF);
        }
    } else if (recipe == 7) {
        if (quantum_cores >= 15 && nano_alloys >= 10) {
            quantum_cores -= 15; nano_alloys -= 10;
            forge_resonance++;
            active_resonance_catalyst = 1;
            wsprintfA(forge_msg, "Synthesized RESONANCE CATALYST!");
            MessageBeep(MB_OK);
        } else {
            wsprintfA(forge_msg, "Need 15 Quantum + 10 Alloy!");
            MessageBeep(0xFFFFFFFF);
        }
    }
    SaveHighScore();
}

void InitLevel() {
    bricks_left = 0;
    power_active = 0;
    dur_laser = 0; dur_fire = 0; dur_barrier = 0; dur_gravity = 0; dur_satellite = 0;
    cd_laser = 0; cd_multi = 0; cd_fire = 0; cd_barrier = 0; cd_gravity = 0; cd_satellite = 0;
    paddle_timer = 0; sticky_timer = 0;
    ufo_bullet_active = 0;

    for (int i = 0; i < MAX_LASERS; i++) lasers[i].active = 0;
    for (int i = 0; i < MAX_PARTICLES; i++) particles[i].life = 0;
    for (int i = 0; i < MAX_SHOCKWAVES; i++) shockwaves[i].life = 0;
    for (int i = 0; i < MAX_BOSS_BULLETS; i++) boss_bullets[i].active = 0;
    for (int i = 0; i < MAX_METEORS; i++) meteors[i].active = 0;
    for (int i = 0; i < MAX_GRAVITY_WELLS; i++) gravity_wells[i].active = 0;

    satellites[0].active = 1; satellites[0].angle = 0.0f; satellites[0].dist = 42.0f;
    satellites[1].active = 1; satellites[1].angle = 3.14159f; satellites[1].dist = 42.0f;

    ufo_active = 0;
    ufo_timer = 200;
    meteor_active = (level >= 12 && level % 4 == 0);

    if (chaos_mode || (level >= 8 && level % 3 == 0)) {
        SpawnGravityWell((float)(W / 3), 140.0f, 25.0f, 0);
        if (level >= 18 || chaos_mode) {
            SpawnGravityWell((float)(W * 2 / 3), 140.0f, -20.0f, 1);
        }
    }

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

    if (chaos_mode) {
        for (int b = 1; b < 3; b++) {
            balls[b].active = 1;
            balls[b].x = (float)(W / 2 + (b == 1 ? -20 : 20));
            balls[b].y = (float)(H - 50);
            balls[b].dx = speed * (b == 1 ? -1.2f : 1.2f);
            balls[b].dy = -speed;
            balls[b].stuck = 1;
            balls[b].stuck_offset = pad_w / 2 + (b == 1 ? -15 : 15);
            balls[b].trail_head = 0;
            for (int t = 0; t < MAX_TRAIL; t++) { balls[b].trail[t].x = 0; balls[b].trail[t].y = 0; }
        }
    }

    pad_w = (diff == 1) ? 45 : 65;
    if (active_chronos) pad_w += 15;
    pad_x = W / 2 - pad_w / 2;

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
                    if (c == 0 || c == 9 || r == 0) v = 10; else if (r == 2 && (c == 4 || c == 5)) v = 11; else v = 2;
                } else if (level == 6) {
                    if ((r == 1 && (c == 2 || c == 7)) || (r == 2 && c >= 2 && c <= 7) || (r == 3 && (c == 1 || c == 4 || c == 5 || c == 8))) v = 3;
                    else if (r == 2 && (c == 4 || c == 5)) v = 4;
                } else if (level == 7) {
                    if (r == 2 && (c != 2 && c != 7)) v = 9; else if (r < 2) v = 3; else v = 4;
                } else if (level == 8) {
                    if (c <= 2 || c >= 7) v = (r % 2 == 0) ? 10 : 2; else if (r == 1 && (c == 4 || c == 5)) v = 11;
                } else if (level == 9) {
                    int pat[6] = {1, 2, 4, 10, 11, 3};
                    v = pat[(r * COLS + c) % 6];
                } else if (level == 10) {
                    if (r >= 2) v = (r == 2) ? 3 : 2;
                } else if (level == 11) {
                    if ((r == 0 && c < 8) || (c == 8 && r < 4) || (r == 4 && c > 1) || (c == 1 && r > 1)) v = (r % 2 == 0) ? 11 : 10; else v = 1;
                } else if (level == 12) {
                    if (r == 0 && c % 2 == 0) v = 9; else if (r == 1) v = 3; else v = 4;
                } else if (level == 13) {
                    if (r == 1) v = 10; else if (r == 2) v = 4; else if (r == 3) v = 11; else v = 1;
                } else if (level == 14) {
                    if (MyAbs(r - 2) + MyAbs(c - 4) <= 3) v = (r == 2) ? 11 : 10;
                } else if (level == 15) {
                    if (r >= 1 && r <= 2 && c >= 4 && c <= 5) v = 4; else v = 3;
                } else if (level == 16) {
                    if (c == 2 || c == 7) v = 11; else v = (r % 2 == 0) ? 10 : 2;
                } else if (level == 17) {
                    if (r == 0) v = 9; else if (r <= 2) v = 10; else v = 11;
                } else if (level == 18) {
                    if ((r == 1 || r == 3) && (c == 2 || c == 7)) v = 5; else v = (r % 2 == 0) ? 4 : 3;
                } else if (level == 19) {
                    if (r == 1) v = 9; else if (r == 2) v = 11; else v = 10;
                } else if (level == 20) {
                    if (r >= 3) v = (r == 3) ? 3 : 2;
                }

                bricks[r][c] = v;
                brick_hp[r][c] = (v == 8) ? 4 : 0;
                if (v != 0 && v != 9) bricks_left++;
            }
        }
    }
    
    if (level >= 21 && level <= 30) {
        bricks_left = 0;
        const char* custom_stages[10][ROWS] = {
            {"8888888888","7777777777","0000000000","1231231231","0000000000","0000000000"},
            {"9000000009","0800000080","0070000700","0006006000","0000550000","0000000000"},
            {"8787878787","7878787878","8787878787","7878787878","0000000000","0000000000"},
            {"5000000005","0888888880","0877777780","0888888880","0000000000","0000000000"},
            {"9999009999","7777007777","8888008888","7777007777","9999009999","0000000000"},
            {"8000000008","0800000080","0080000800","0008008000","0000880000","0000000000"},
            {"6666666666","7777777777","4444444444","8888888888","7777777777","6666666666"},
            {"9876543210","0123456789","9876543210","0123456789","0000000000","0000000000"},
            {"8889999888","7779999777","6669999666","5559999555","4449999444","0000000000"},
            {"0000000000","0000000000","0008888000","0008888000","0000000000","0000000000"}
        };
        int idx = level - 21;
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                char ch = custom_stages[idx][r][c];
                int v = (ch >= '0' && ch <= '9') ? (ch - '0') : 0;
                if (idx % 2 == 0 && (r == 0 || r == 3) && (c == 4 || c == 5) && v != 0) v = 11;
                if (idx % 2 == 1 && (r == 1 || r == 2) && (c == 2 || c == 7) && v != 0) v = 10;
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
                if (level == 31) v = (r % 2 == 0) ? 11 : 10;
                else if (level == 32) v = (c % 2 == 0) ? 10 : 5;
                else if (level == 34) v = (r == c || r == COLS - 1 - c) ? 9 : 11;
                else if (level == 35) v = (r < 3) ? 10 : 11;
                else if (level == 37) v = ((r + c) % 3 == 0) ? 11 : 8;
                else if (level == 38) v = (c % 3 == 0) ? 10 : 7;
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
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            if (state == 4) {
                if (y >= 35 && y < 35 + ROWS * BR_H && x >= 0 && x < W) {
                    int r = (y - 35) / BR_H;
                    int c = x / BR_W;
                    bricks[r][c] = (bricks[r][c] + 1) % 12;
                    brick_hp[r][c] = (bricks[r][c] == 8) ? 4 : 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (state == 5) {
                if (y >= 55 && y <= 75) CraftForge(1);
                else if (y >= 78 && y <= 98) CraftForge(2);
                else if (y >= 101 && y <= 121) CraftForge(3);
                else if (y >= 124 && y <= 144) CraftForge(4);
                else if (y >= 147 && y <= 167) CraftForge(5);
                else if (y >= 170 && y <= 190) CraftForge(6);
                else if (y >= 193 && y <= 213) CraftForge(7);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_KEYDOWN:
            if (state == 1) {
                if (wParam == 'L' || wParam == 'l') UseSkill('L');
                if (wParam == 'M' || wParam == 'm') UseSkill('M');
                if (wParam == 'F' || wParam == 'f') UseSkill('F');
                if (wParam == 'B' || wParam == 'b') UseSkill('B');
                if (wParam == 'G' || wParam == 'g') UseSkill('G');
                if (wParam == 'S' || wParam == 's') UseSkill('S');
                if (wParam == 'O' || wParam == 'o') { state = 5; }
                if (wParam >= '1' && wParam <= '7') CraftForge((int)(wParam - '0'));
            } else if (state == 5) {
                if (wParam >= '1' && wParam <= '7') CraftForge((int)(wParam - '0'));
                if (wParam == VK_ESCAPE || wParam == 'O' || wParam == 'o' || wParam == VK_RETURN) state = 1;
            }
            break;
        case WM_TIMER:
            if (state == 1) {
                frame_counter++;
                if (dur_laser > 0) dur_laser--;
                if (dur_fire > 0) dur_fire--;
                if (dur_barrier > 0) dur_barrier--;
                if (dur_gravity > 0) dur_gravity--;
                if (dur_satellite > 0) dur_satellite--;
                if (cd_laser > 0) cd_laser--;
                if (cd_multi > 0) cd_multi--;
                if (cd_fire > 0) cd_fire--;
                if (cd_barrier > 0) cd_barrier--;
                if (cd_gravity > 0) cd_gravity--;
                if (cd_satellite > 0) cd_satellite--;
                if (paddle_timer > 0) { paddle_timer--; if (paddle_timer == 0) pad_w = (diff == 1) ? 45 : 65; }

                int move_speed = active_chronos ? 8 : 6;
                if ((GetAsyncKeyState(VK_LEFT) & 0x8000) || (GetAsyncKeyState('A') & 0x8000)) pad_x -= move_speed;
                if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) || (GetAsyncKeyState('D') & 0x8000)) pad_x += move_speed;
                if (pad_x < 0) pad_x = 0;
                if (pad_x > W - pad_w) pad_x = W - pad_w;

                pad_vx = pad_x - last_pad_x;
                last_pad_x = pad_x;
                if (pad_squash_timer > 0) pad_squash_timer--;

                UpdateParticles();

                // Update Orbital Satellites
                float sat_spd = (dur_satellite > 0 || active_satellite_boost) ? 0.08f : 0.04f;
                float sat_r = (dur_satellite > 0) ? 55.0f : 42.0f;
                satellites[0].angle += sat_spd;
                satellites[1].angle = satellites[0].angle + 3.14159f;
                int pcx = pad_x + pad_w / 2;
                int pcy = H - 40 + pad_h / 2;
                for (int s = 0; s < MAX_SATELLITES; s++) {
                    satellites[s].x = (float)pcx + MyCos(satellites[s].angle) * sat_r;
                    satellites[s].y = (float)pcy + MySin(satellites[s].angle) * (sat_r * 0.45f);
                }

                // Satellite laser fire during Satellite Surge
                if (dur_satellite > 0 && dur_satellite % 14 == 0) {
                    for (int s = 0; s < MAX_SATELLITES; s++) {
                        SpawnLaser(satellites[s].x, satellites[s].y, 0.0f, -8.0f, 1);
                    }
                }

                int laser_rate = active_valkyrie ? 8 : 15;
                if (dur_laser > 0 && dur_laser % laser_rate == 0) {
                    SpawnLaser((float)(pad_x + 5), (float)(H - 40), 0.0f, active_valkyrie ? -9.0f : -7.0f, active_valkyrie ? 1 : 0);
                    SpawnLaser((float)(pad_x + pad_w - 5), (float)(H - 40), 0.0f, active_valkyrie ? -9.0f : -7.0f, active_valkyrie ? 1 : 0);
                }

                for (int i = 0; i < MAX_LASERS; i++) {
                    if (lasers[i].active) {
                        lasers[i].x += lasers[i].vx;
                        lasers[i].y += lasers[i].vy;
                        if (lasers[i].y < 0 || lasers[i].x < 0 || lasers[i].x > W) lasers[i].active = 0;
                        else {
                            if (boss_active && lasers[i].x > boss_x && lasers[i].x < boss_x + boss_w &&
                                lasers[i].y > boss_y && lasers[i].y < boss_y + boss_h) {
                                boss_hp -= (lasers[i].type == 1 ? 2 : 1);
                                lasers[i].active = 0;
                                SpawnParticles(lasers[i].x, lasers[i].y, RGB(255, 0, 85), 4);
                                MessageBeep(0xFFFFFFFF);
                                if (boss_hp <= 0) {
                                    score += 500;
                                    nano_alloys += 5;
                                    quantum_cores += 3;
                                    SpawnParticles((float)(boss_x + boss_w/2), (float)(boss_y + boss_h/2), RGB(255, 0, 85), 40);
                                    boss_active = 0;
                                    level++;
                                    if (level > 40) { state = 3; SaveHighScore(); } else InitLevel();
                                }
                                continue;
                            }

                            int r = ((int)lasers[i].y - 35) / BR_H;
                            int c = (int)lasers[i].x / BR_W;
                            if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
                                if (bricks[r][c] != 0 && bricks[r][c] != 9) {
                                    int type = bricks[r][c];
                                    SpawnParticles(lasers[i].x, lasers[i].y, GetBrickColor(type, r, c), 6);
                                    plasma_shards += 1;
                                    
                                    if (type == 10) { // Laser Reflector Prism! Splits laser
                                        lasers[i].active = 0;
                                        SpawnLaser((float)(c * BR_W + BR_W/2), (float)(r * BR_H + 35 + BR_H/2), -5.5f, -2.5f, 2);
                                        SpawnLaser((float)(c * BR_W + BR_W/2), (float)(r * BR_H + 35 + BR_H/2), 5.5f, -2.5f, 2);
                                        SpawnParticles(lasers[i].x, lasers[i].y, RGB(0, 255, 255), 10);
                                        score += 20;
                                    } else if (type == 11) { // Quantum Resonance
                                        lasers[i].active = 0;
                                        TriggerQuantumResonance(r, c);
                                    } else if (type == 4) {
                                        lasers[i].active = 0;
                                        TriggerExplosion(r, c);
                                    } else if (type == 7) {
                                        lasers[i].active = 0;
                                        bricks[r][c] = 0; bricks_left--; score += 30; lifetime_bricks++;
                                    } else if (type == 8) {
                                        lasers[i].active = 0;
                                        brick_hp[r][c] -= (lasers[i].type == 1 ? 2 : 1);
                                        if (brick_hp[r][c] <= 0) { bricks[r][c] = 0; bricks_left--; score += 40; lifetime_bricks++; nano_alloys += 2; }
                                        else { score += 5; }
                                    } else if (type > 1 && type != 6) {
                                        lasers[i].active = 0;
                                        bricks[r][c]--; score += 5;
                                    } else {
                                        lasers[i].active = 0;
                                        bricks[r][c] = 0; bricks_left--; score += 10; lifetime_bricks++;
                                    }
                                }
                            }
                        }
                    }
                }

                if (shield_active) {
                    shield_x += shield_dx;
                    if (shield_x < 10 || shield_x + shield_w > W - 10) shield_dx = -shield_dx;
                }

                for (int g = 0; g < MAX_GRAVITY_WELLS; g++) {
                    if (gravity_wells[g].active) {
                        gravity_wells[g].spin += 0.08f;
                    }
                }

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

                if (ufo_bullet_active) {
                    ufo_bullet_y += 3.5f;
                    // Check collision with orbital satellites
                    for (int s = 0; s < MAX_SATELLITES; s++) {
                        float sdx = ufo_bullet_x - satellites[s].x;
                        float sdy = ufo_bullet_y - satellites[s].y;
                        if (sdx * sdx + sdy * sdy < 144.0f) {
                            ufo_bullet_active = 0;
                            SpawnParticles(satellites[s].x, satellites[s].y, RGB(0, 255, 200), 8);
                            SpawnShockwave(satellites[s].x, satellites[s].y, RGB(0, 255, 200));
                            break;
                        }
                    }
                    if (ufo_bullet_active && ufo_bullet_y > H - 40 && ufo_bullet_y < H - 30 && ufo_bullet_x > pad_x && ufo_bullet_x < pad_x + pad_w) {
                        ufo_bullet_active = 0;
                        SpawnParticles(ufo_bullet_x, ufo_bullet_y, RGB(255, 0, 255), 10);
                        if (score > 25) score -= 25;
                        MessageBeep(0xFFFFFFFF);
                    } else if (ufo_bullet_y > H) ufo_bullet_active = 0;
                }

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
                        // Check satellite interception
                        for (int s = 0; s < MAX_SATELLITES; s++) {
                            float sdx = boss_bullets[i].x - satellites[s].x;
                            float sdy = boss_bullets[i].y - satellites[s].y;
                            if (sdx * sdx + sdy * sdy < 144.0f) {
                                boss_bullets[i].active = 0;
                                SpawnParticles(satellites[s].x, satellites[s].y, RGB(0, 255, 200), 8);
                                break;
                            }
                        }
                        if (boss_bullets[i].active && boss_bullets[i].y > H - 40 && boss_bullets[i].y < H - 30 &&
                            boss_bullets[i].x > pad_x && boss_bullets[i].x < pad_x + pad_w) {
                            boss_bullets[i].active = 0;
                            SpawnParticles(boss_bullets[i].x, boss_bullets[i].y, RGB(255, 0, 85), 8);
                            MessageBeep(0xFFFFFFFF);
                        } else if (boss_bullets[i].y > H) boss_bullets[i].active = 0;
                    }
                }

                if (power_active) {
                    power_y += 2.5f;
                    if (active_chronos) {
                        float dx = (pad_x + pad_w / 2) - power_x;
                        power_x += dx * 0.05f;
                    }
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
                        else if (power_type == 8) { UseSkill('G'); }
                        else if (power_type == 9) { UseSkill('S'); }
                    }
                    if (power_y > H) power_active = 0;
                }

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
                            // Check satellite interception
                            for (int s = 0; s < MAX_SATELLITES; s++) {
                                float sdx = meteors[i].x - satellites[s].x;
                                float sdy = meteors[i].y - satellites[s].y;
                                if (sdx * sdx + sdy * sdy < 196.0f) {
                                    meteors[i].active = 0;
                                    SpawnParticles(satellites[s].x, satellites[s].y, RGB(255, 150, 50), 12);
                                    SpawnShockwave(satellites[s].x, satellites[s].y, RGB(255, 150, 50));
                                    break;
                                }
                            }
                            if (meteors[i].active && meteors[i].y > H - 40 && meteors[i].y < H - 30 && meteors[i].x > pad_x && meteors[i].x < pad_x + pad_w) {
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

                    for (int g = 0; g < MAX_GRAVITY_WELLS; g++) {
                        if (gravity_wells[g].active) {
                            float gx = gravity_wells[g].x;
                            float gy = gravity_wells[g].y;
                            float gdx = gx - balls[i].x;
                            float gdy = gy - balls[i].y;
                            float dist_sq = gdx * gdx + gdy * gdy;
                            float r_lim = gravity_wells[g].radius;
                            if (dist_sq < r_lim * r_lim && dist_sq > 36.0f) {
                                float dist = MySqrt(dist_sq);
                                float force = (gravity_wells[g].mass * 0.08f) / (dist * 0.15f + 1.0f);
                                balls[i].dx += (gdx / dist) * force;
                                balls[i].dy += (gdy / dist) * force;
                                if (frame_counter % 3 == 0) {
                                    SpawnParticles(balls[i].x, balls[i].y, gravity_wells[g].type == 0 ? RGB(160, 32, 240) : RGB(0, 255, 255), 1);
                                }
                            }
                        }
                    }

                    // Orbital Satellite Collision Check
                    for (int s = 0; s < MAX_SATELLITES; s++) {
                        float bsx = (balls[i].x + 4) - satellites[s].x;
                        float bsy = (balls[i].y + 4) - satellites[s].y;
                        if (bsx * bsx + bsy * bsy < 144.0f) {
                            balls[i].dy = -MyAbsF(balls[i].dy) * 1.15f;
                            balls[i].dx += bsx * 0.3f;
                            SpawnParticles(satellites[s].x, satellites[s].y, RGB(0, 255, 200), 8);
                            SpawnShockwave(satellites[s].x, satellites[s].y, RGB(0, 255, 200));
                            score += 10;
                            MessageBeep(0xFFFFFFFF);
                        }
                    }

                    float max_spd = 9.0f;
                    if (balls[i].dx > max_spd) balls[i].dx = max_spd;
                    if (balls[i].dx < -max_spd) balls[i].dx = -max_spd;
                    if (balls[i].dy > max_spd) balls[i].dy = max_spd;
                    if (balls[i].dy < -max_spd) balls[i].dy = -max_spd;

                    balls[i].x += balls[i].dx;
                    balls[i].y += balls[i].dy;
                    
                    balls[i].trail[balls[i].trail_head].x = balls[i].x;
                    balls[i].trail[balls[i].trail_head].y = balls[i].y;
                    balls[i].trail_head = (balls[i].trail_head + 1) % MAX_TRAIL;

                    if (balls[i].x < 0) { balls[i].x = 0; balls[i].dx = -balls[i].dx; }
                    if (balls[i].x > W - 8) { balls[i].x = (float)(W - 8); balls[i].dx = -balls[i].dx; }
                    if (balls[i].y < 0) { balls[i].y = 0; balls[i].dy = -balls[i].dy; }

                    if ((dur_barrier > 0 || aegis_layers > 0) && balls[i].y >= H - 20) {
                        balls[i].y = (float)(H - 20);
                        balls[i].dy = -MyAbsF(balls[i].dy);
                        SpawnParticles(balls[i].x, (float)(H - 10), RGB(0, 255, 255), 6);
                    }

                    if (shield_active && balls[i].y + 8 > shield_y && balls[i].y < shield_y + 8 &&
                        balls[i].x + 8 > shield_x && balls[i].x < shield_x + shield_w) {
                        balls[i].dy = -balls[i].dy;
                        SpawnParticles(balls[i].x, balls[i].y, RGB(0, 255, 255), 6);
                    }

                    if (ufo_active && balls[i].x + 8 > ufo_x && balls[i].x < ufo_x + 30 &&
                        balls[i].y + 8 > ufo_y && balls[i].y < ufo_y + 12) {
                        ufo_active = 0;
                        balls[i].dy = -balls[i].dy;
                        score += 100;
                        quantum_cores += 2;
                        plasma_shards += 3;
                        SpawnParticles(ufo_x + 15, (float)(ufo_y + 6), RGB(255, 0, 255), 15);
                        MessageBeep(0xFFFFFFFF);
                        power_active = 1; power_type = (MyRand() % 9) + 1; power_x = ufo_x + 15; power_y = (float)ufo_y;
                    }

                    if (boss_active && balls[i].x + 8 > boss_x && balls[i].x < boss_x + boss_w &&
                        balls[i].y + 8 > boss_y && balls[i].y < boss_y + boss_h) {
                        boss_hp -= (dur_fire > 0 || active_supernova ? 3 : 1);
                        if (dur_fire <= 0 && !active_supernova) balls[i].dy = -balls[i].dy;
                        SpawnParticles(balls[i].x, balls[i].y, RGB(255, 0, 85), 8);
                        MessageBeep(0xFFFFFFFF);
                        if (boss_hp <= 0) {
                            score += 1000;
                            nano_alloys += 8;
                            quantum_cores += 5;
                            SpawnParticles((float)(boss_x + boss_w/2), (float)(boss_y + boss_h/2), RGB(255, 0, 85), 50);
                            boss_active = 0;
                            level++;
                            if (level > 40) { state = 3; SaveHighScore(); } else InitLevel();
                            break;
                        }
                    }

                    if (balls[i].y + 8 > H - 40 && balls[i].y < H - 40 + pad_h &&
                        balls[i].x + 8 > pad_x && balls[i].x < pad_x + pad_w) {
                        balls[i].y = (float)(H - 40 - 8);
                        balls[i].dy = -MyAbsF(balls[i].dy);
                        pad_squash_timer = 12;
                        float hit_pos = (balls[i].x + 4) - (pad_x + pad_w / 2);
                        balls[i].dx = hit_pos * 0.22f;
                        if (MyAbsF(balls[i].dx) < 1.0f) balls[i].dx = (balls[i].dx >= 0) ? 1.5f : -1.5f;
                        MessageBeep(0xFFFFFFFF);
                        SpawnParticles(balls[i].x + 4, (float)(H - 40), RGB(0, 255, 255), 4);
                        SpawnShockwave(balls[i].x + 4, (float)(H - 40), RGB(0, 255, 255));
                        if (screen_shake < 8) screen_shake = 8;
                        if (sticky_timer > 0) {
                            balls[i].stuck = 1;
                            balls[i].stuck_offset = (int)(balls[i].x - pad_x);
                        }
                    }

                    int r = ((int)balls[i].y - 35) / BR_H;
                    int c = (int)balls[i].x / BR_W;
                    if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
                        int type = bricks[r][c];
                        if (type != 0) {
                            int bx = c * BR_W + BR_W / 2;
                            int by = r * BR_H + 35 + BR_H / 2;
                            int chaos_mult = 1 + (active_balls_count / 3);

                            if (type == 11 || active_resonance_catalyst) { // Quantum Resonance
                                TriggerQuantumResonance(r, c);
                                if (dur_fire <= 0 && !active_supernova) balls[i].dy = -balls[i].dy;
                            } else if (type == 10) { // Laser Reflector Prism
                                bricks[r][c] = 0; bricks_left--; score += 30 * chaos_mult; lifetime_bricks++;
                                quantum_cores += 1; plasma_shards += 1;
                                SpawnLaser((float)bx, (float)by, -7.0f, 0.0f, 2);
                                SpawnLaser((float)bx, (float)by, 7.0f, 0.0f, 2);
                                SpawnLaser((float)bx, (float)by, 0.0f, -7.0f, 2);
                                SpawnLaser((float)bx, (float)by, 0.0f, 7.0f, 2);
                                SpawnParticles((float)bx, (float)by, RGB(0, 255, 255), 14);
                                SpawnShockwave((float)bx, (float)by, RGB(0, 255, 255));
                                if (dur_fire <= 0 && !active_supernova) balls[i].dy = -balls[i].dy;
                                MessageBeep(0xFFFFFFFF);
                            } else if (type == 9) {
                                if (dur_fire <= 0 && !active_supernova) balls[i].dy = -balls[i].dy;
                                else { bricks[r][c] = 0; bricks_left--; score += 50 * chaos_mult; nano_alloys += 2; }
                                SpawnParticles(balls[i].x, balls[i].y, RGB(200, 210, 220), 4);
                                SpawnShockwave(balls[i].x, balls[i].y, RGB(200, 210, 220));
                                if (screen_shake < 4) screen_shake = 4;
                                MessageBeep(0xFFFFFFFF);
                            } else if (type == 4) {
                                bricks[r][c] = 0; bricks_left--; plasma_shards += 2;
                                if (dur_fire <= 0 && !active_supernova) balls[i].dy = -balls[i].dy;
                                TriggerExplosion(r, c);
                            } else if (type == 5) {
                                bricks[r][c] = 0; bricks_left--; score += 20 * chaos_mult; lifetime_bricks++; quantum_cores += 1;
                                SpawnParticles((float)bx, (float)by, RGB(170, 0, 255), 12);
                                MessageBeep(0xFFFFFFFF);
                                balls[i].x = (float)(20 + MyRand() % (W - 40));
                                balls[i].y = 150.0f;
                                balls[i].dx = (float)((MyRand() % 2 == 0 ? 1 : -1) * (speed + 1));
                                balls[i].dy = speed;
                            } else if (type == 6) {
                                bricks[r][c] = 0; bricks_left--; score += 25 * chaos_mult; lifetime_bricks++; quantum_cores += 1;
                                SpawnParticles((float)bx, (float)by, RGB(0, 255, 204), 10);
                                MessageBeep(0xFFFFFFFF);
                                power_active = 1; power_type = (MyRand() % 9) + 1;
                                power_x = (float)bx; power_y = (float)by;
                            } else if (type == 7) {
                                bricks[r][c] = 0; bricks_left--; score += 30 * chaos_mult; lifetime_bricks++; plasma_shards += 1;
                                SpawnParticles((float)bx, (float)by, RGB(200, 255, 255), 10);
                                MessageBeep(0xFFFFFFFF);
                            } else if (type == 8) {
                                SpawnParticles((float)bx, (float)by, RGB(80, 80, 80), 8);
                                if (dur_fire > 0 || active_supernova) {
                                    bricks[r][c] = 0; bricks_left--; score += 40 * chaos_mult; lifetime_bricks++; nano_alloys += 2;
                                } else {
                                    brick_hp[r][c]--;
                                    if (brick_hp[r][c] <= 0) { bricks[r][c] = 0; bricks_left--; score += 40 * chaos_mult; lifetime_bricks++; nano_alloys += 2; }
                                    else { score += 5; }
                                    balls[i].dy = -balls[i].dy;
                                }
                                MessageBeep(0xFFFFFFFF);
                            } else {
                                SpawnParticles((float)bx, (float)by, GetBrickColor(type, r, c), 8);
                                SpawnShockwave((float)bx, (float)by, GetBrickColor(type, r, c));
                                if (screen_shake < 5) screen_shake = 5;
                                plasma_shards += 1;
                                if (dur_fire > 0 || active_supernova) {
                                    bricks[r][c] = 0; bricks_left--; score += 15 * chaos_mult; lifetime_bricks++;
                                } else {
                                    bricks[r][c]--;
                                    if (bricks[r][c] == 0) { bricks_left--; score += 10 * chaos_mult; lifetime_bricks++; }
                                    else score += 5;
                                    balls[i].dy = -balls[i].dy;
                                }
                                MessageBeep(0xFFFFFFFF);

                                if (chaos_mode && (MyRand() % 4 == 0) && active_balls_count < MAX_BALLS - 2) {
                                    SplitBall(i);
                                }

                                if (!power_active && (MyRand() % 5 == 0)) {
                                    power_active = 1;
                                    power_type = (MyRand() % 9) + 1;
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

                    if (balls[i].y > H && dur_barrier <= 0 && aegis_layers <= 0) {
                        balls[i].active = 0;
                        active_balls_count--;
                    }
                }

                if (active_balls_count <= 0) {
                    lives--;
                    power_active = 0; sticky_timer = 0; dur_laser = 0; dur_fire = 0; dur_satellite = 0;
                    if (lives <= 0) {
                        SaveHighScore();
                        state = 2;
                        MessageBeep(MB_ICONEXCLAMATION);
                    } else {
                        InitLevel();
                        MessageBeep(MB_ICONASTERISK);
                    }
                }
            } else if (state == 0 || state == 2 || state == 3) {
                if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
                    diff = 0; speed = 3.5f; score = 0; lives = 3; level = 1; chaos_mode = 0; InitLevel(); state = 1; pad_w = 65;
                }
                if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
                    diff = 1; speed = 5.0f; score = 0; lives = 3; level = 1; chaos_mode = 0; InitLevel(); state = 1; pad_w = 45;
                }
                if (GetAsyncKeyState('C') & 0x8000) {
                    diff = 1; speed = 4.5f; score = 0; lives = 5; level = 1; chaos_mode = 1; InitLevel(); state = 1; pad_w = 70;
                }
                if (GetAsyncKeyState('E') & 0x8000) {
                    state = 4;
                    for (int r=0; r<ROWS; r++) for(int c=0; c<COLS; c++) bricks[r][c]=0;
                }
                if (GetAsyncKeyState('O') & 0x8000 || GetAsyncKeyState('F') & 0x8000) {
                    state = 5;
                }
            } else if (state == 4) {
                if (GetAsyncKeyState('P') & 0x8000) {
                    diff = 1; speed = 4.0f; score = 0; lives = 3; level = 99; chaos_mode = 0; InitLevel(); state = 1; pad_w = 65;
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
            if (screen_shake_intensity > 0.05f) {
                int idx = (frame_counter * 3) % 16;
                sx = (int)((FastSin(idx) * screen_shake_intensity) / 80.0f);
                sy = (int)((FastCos(idx + 4) * screen_shake_intensity) / 80.0f);
                screen_shake_intensity *= 0.90f;
                screen_shake_angle *= 0.88f;
                if (screen_shake_intensity < 0.05f) {
                    screen_shake_intensity = 0.0f;
                    screen_shake_angle = 0.0f;
                }
            } else if (screen_shake > 0) {
                sx = (MyRand() % screen_shake) - (screen_shake / 2);
                sy = (MyRand() % screen_shake) - (screen_shake / 2);
                screen_shake -= (screen_shake > 6 ? 2 : 1);
                if (screen_shake < 0) screen_shake = 0;
            }
            SetViewportOrgEx(memDC, sx, sy, NULL);
            
            HBRUSH bg = CreateSolidBrush(chaos_mode ? RGB(22, 10, 30) : RGB(16, 16, 26));
            RECT fullRc = {0, 0, W, H};
            FillRect(memDC, &fullRc, bg);
            DeleteObject(bg);
            
            HPEN gridPen = CreatePen(PS_SOLID, 1, chaos_mode ? RGB(60, 20, 80) : RGB(20, 40, 60));
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
            
            for(int i = 0; i < 20; i++) {
                int dx = (i * 73 + frame_counter) % W;
                int dy = (i * 37 + frame_counter / 3) % H;
                SetPixel(memDC, dx, dy, chaos_mode ? RGB(255, 100, 255) : RGB(100, 150, 255));
            }

            DrawSpeedDustMotes(memDC);
            DrawPerimeterInlay(memDC);
            
            SetTextColor(memDC, RGB(255, 255, 255));
            SetBkMode(memDC, TRANSPARENT);
            
            if (state == 0) {
                char* t1 = "KBREAKOUT - LOOP 11";
                char* t2 = "Satellites, Reflectors & Quantum Resonance";
                char* t3 = "Press ENTER for Classic Campaign";
                char* t4 = "Press SPACE for Hard Campaign";
                char* t5 = "Press C for MULTI-BALL CHAOS MODE";
                char* t6 = "Press O/F for CYBER-FORGE LAB";
                char* t7 = "Press E for Level Editor";
                TextOutA(memDC, W/2 - 70, H/2 - 60, t1, lstrlenA(t1));
                TextOutA(memDC, W/2 - 120, H/2 - 38, t2, lstrlenA(t2));
                TextOutA(memDC, W/2 - 90, H/2 - 5, t3, lstrlenA(t3));
                TextOutA(memDC, W/2 - 80, H/2 + 15, t4, lstrlenA(t4));
                SetTextColor(memDC, RGB(255, 100, 255));
                TextOutA(memDC, W/2 - 105, H/2 + 35, t5, lstrlenA(t5));
                SetTextColor(memDC, RGB(0, 255, 255));
                TextOutA(memDC, W/2 - 85, H/2 + 55, t6, lstrlenA(t6));
                SetTextColor(memDC, RGB(180, 180, 180));
                TextOutA(memDC, W/2 - 75, H/2 + 75, t7, lstrlenA(t7));
            } else if (state == 5) {
                SetTextColor(memDC, RGB(0, 255, 255));
                char* title = "=== CYBER-FORGE POWER LAB ===";
                TextOutA(memDC, W/2 - 100, 10, title, lstrlenA(title));

                char matStr[96];
                wsprintfA(matStr, "Materials: [Plasma:%d]  [Quantum:%d]  [Alloy:%d]", plasma_shards, quantum_cores, nano_alloys);
                SetTextColor(memDC, RGB(255, 220, 0));
                TextOutA(memDC, 20, 32, matStr, lstrlenA(matStr));

                SetTextColor(memDC, RGB(200, 200, 200));
                char* r1 = "[1] Nova Blast Ball (15 Pls + 10 Aly) -> Fire Explosions";
                char* r2 = "[2] Chronos Paddle  (12 Pls + 8 Qtm)  -> Dilate + Magnet";
                char* r3 = "[3] Valkyrie Guns   (10 Qtm + 15 Aly) -> Heavy Cannons";
                char* r4 = "[4] Quantum Aegis   (10/10/10)        -> +3 Shield Layers";
                char* r5 = "[5] Singularity Core(20/15/15)        -> Spawn Black Hole";
                char* r6 = "[6] Satellite Array (12/12/12)        -> Orbital Overdrive";
                char* r7 = "[7] Resonance Catalyst(15 Qtm + 10 Aly)-> Quantum Shockwaves";
                TextOutA(memDC, 10, 58, r1, lstrlenA(r1));
                TextOutA(memDC, 10, 82, r2, lstrlenA(r2));
                TextOutA(memDC, 10, 106, r3, lstrlenA(r3));
                TextOutA(memDC, 10, 130, r4, lstrlenA(r4));
                TextOutA(memDC, 10, 154, r5, lstrlenA(r5));
                TextOutA(memDC, 10, 178, r6, lstrlenA(r6));
                TextOutA(memDC, 10, 202, r7, lstrlenA(r7));

                SetTextColor(memDC, RGB(0, 255, 200));
                TextOutA(memDC, 20, 235, forge_msg, lstrlenA(forge_msg));

                SetTextColor(memDC, RGB(255, 255, 100));
                char* sub = "Press 1-7 to synthesize. Press ESC / O / ENTER to return.";
                TextOutA(memDC, 15, 265, sub, lstrlenA(sub));
            } else if (state == 4) {
                char* t1 = "EDITOR MODE";
                char* t2 = "Click grid to cycle block types (0-11)";
                char* t3 = "Press P to Play Custom Level";
                char* t4 = "Press ESC to return";
                TextOutA(memDC, W/2 - 45, H/2 - 20, t1, lstrlenA(t1));
                TextOutA(memDC, W/2 - 105, H/2 + 10, t2, lstrlenA(t2));
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
                char* t2 = "Press ENTER for Campaign";
                char* t3 = "Press C for Chaos Mode";
                TextOutA(memDC, W/2 - 40, H/2 - 20, t1, lstrlenA(t1));
                TextOutA(memDC, W/2 - 75, H/2 + 10, t2, lstrlenA(t2));
                TextOutA(memDC, W/2 - 75, H/2 + 30, t3, lstrlenA(t3));
            } else if (state == 3) {
                char* t1 = "VICTORY! CAMPAIGN CLEARED";
                char* t2 = "Press ENTER to Play Again";
                TextOutA(memDC, W/2 - 85, H/2 - 20, t1, lstrlenA(t1));
                TextOutA(memDC, W/2 - 75, H/2 + 15, t2, lstrlenA(t2));
            } else {
                for (int r = 0; r < ROWS; r++) {
                    for (int c = 0; c < COLS; c++) {
                        if (bricks[r][c]) {
                            DrawGDIBrick(memDC, r, c, bricks[r][c], c * BR_W, r * BR_H + 35, brick_hp[r][c]);
                        }
                    }
                }

                for (int g = 0; g < MAX_GRAVITY_WELLS; g++) {
                    if (gravity_wells[g].active) {
                        int gx = (int)gravity_wells[g].x;
                        int gy = (int)gravity_wells[g].y;
                        int isPulsar = gravity_wells[g].type == 1;

                        HPEN rPen = CreatePen(PS_SOLID, 2, isPulsar ? RGB(0, 255, 255) : RGB(180, 50, 255));
                        HGDIOBJ oP = SelectObject(memDC, rPen);
                        HBRUSH hBr = CreateSolidBrush(RGB(5, 5, 10));
                        HGDIOBJ oB = SelectObject(memDC, hBr);

                        int r_well = 18;
                        Ellipse(memDC, gx - r_well, gy - r_well, gx + r_well, gy + r_well);

                        for (int k = 0; k < 4; k++) {
                            float ang = gravity_wells[g].spin + ((float)k * 1.57f);
                            int px = gx + (int)(MyCos(ang) * 26.0f);
                            int py = gy + (int)(MySin(ang) * 16.0f);
                            SetPixel(memDC, px, py, isPulsar ? RGB(255, 255, 255) : RGB(255, 100, 255));
                        }

                        SelectObject(memDC, oP);
                        SelectObject(memDC, oB);
                        DeleteObject(rPen);
                        DeleteObject(hBr);
                    }
                }

                if (shield_active) {
                    HBRUSH sBr = CreateSolidBrush(RGB(0, 255, 255));
                    RECT sRc = { (int)shield_x, (int)120, (int)(shield_x + shield_w), 128 };
                    FillRect(memDC, &sRc, sBr);
                    DeleteObject(sBr);
                }

                if (ufo_active) {
                    HBRUSH uBr = CreateSolidBrush(RGB(255, 0, 255));
                    RECT uRc = { (int)ufo_x, ufo_y, (int)(ufo_x + 30), ufo_y + 10 };
                    FillRect(memDC, &uRc, uBr);
                    DeleteObject(uBr);
                    
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

                if (boss_active) {
                    HBRUSH bBr = CreateSolidBrush(RGB(255, 0, 85));
                    RECT bRc = { boss_x, boss_y, boss_x + boss_w, boss_y + boss_h };
                    FillRect(memDC, &bRc, bBr);
                    DeleteObject(bBr);

                    char bStr[32];
                    wsprintfA(bStr, "BOSS HP: %d/%d", boss_hp, boss_max_hp);
                    TextOutA(memDC, boss_x + 10, boss_y + 12, bStr, lstrlenA(bStr));
                    
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

                if (dur_barrier > 0 || aegis_layers > 0) {
                    HPEN barPen = CreatePen(PS_SOLID, aegis_layers > 0 ? 4 : 2, aegis_layers > 0 ? RGB(255, 200, 0) : RGB(0, 255, 255));
                    HGDIOBJ oP = SelectObject(memDC, barPen);
                    MoveToEx(memDC, 0, H - 20, NULL);
                    LineTo(memDC, W, H - 20);
                    SelectObject(memDC, oP);
                    DeleteObject(barPen);
                }

                float p_squash = 1.0f + (pad_squash_timer > 0 ? MySin(pad_squash_timer * 0.5f) * 0.4f * ((float)pad_squash_timer / 15.0f) : 0.0f);
                int dp_w = (int)(pad_w * p_squash);
                int dp_h = (int)(pad_h * (2.0f - p_squash));
                int dp_x = pad_x + (pad_w - dp_w) / 2;
                int dp_y = (H - 40) + (pad_h - dp_h);

                int r_base = dur_laser > 0 ? 0 : sticky_timer > 0 ? 50 : (chaos_mode ? 120 : 70);
                int g_base = dur_laser > 0 ? 204 : sticky_timer > 0 ? 140 : (chaos_mode ? 40 : 100);
                int b_base = dur_laser > 0 ? 204 : sticky_timer > 0 ? 255 : (chaos_mode ? 200 : 170);

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

                // Sculpted paddle specular sheen highlight traversing top bevel
                int sheen_pad_x = (int)((frame_counter * 3) % (dp_w + 30)) - 15;
                if (sheen_pad_x > -8 && sheen_pad_x < dp_w + 8) {
                    HPEN pSheenPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                    HGDIOBJ oldPS = SelectObject(memDC, pSheenPen);
                    int sx1 = dp_x + sheen_pad_x - 6; if (sx1 < dp_x + 2) sx1 = dp_x + 2;
                    int sx2 = dp_x + sheen_pad_x + 6; if (sx2 > dp_x + dp_w - 2) sx2 = dp_x + dp_w - 2;
                    if (sx2 > sx1) {
                        MoveToEx(memDC, sx1, dp_y + 1, NULL);
                        LineTo(memDC, sx2, dp_y + 1);
                    }
                    SelectObject(memDC, oldPS);
                    DeleteObject(pSheenPen);
                }

                // Dual ion thruster nozzles & flickering exhaust flame motes at underside
                int thrust_flicker = (frame_counter % 4 < 2) ? 6 : 4;
                HPEN tPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
                HBRUSH tBr = CreateSolidBrush(RGB(255, 255, 255));
                HGDIOBJ oldTP = SelectObject(memDC, tPen);
                HGDIOBJ oldTB = SelectObject(memDC, tBr);
                POINT lt[3] = { {dp_x + 6, dp_y + dp_h}, {dp_x + 12, dp_y + dp_h}, {dp_x + 9, dp_y + dp_h + thrust_flicker} };
                Polygon(memDC, lt, 3);
                POINT rt[3] = { {dp_x + dp_w - 12, dp_y + dp_h}, {dp_x + dp_w - 6, dp_y + dp_h}, {dp_x + dp_w - 9, dp_y + dp_h + thrust_flicker} };
                Polygon(memDC, rt, 3);
                SelectObject(memDC, oldTP);
                SelectObject(memDC, oldTB);
                DeleteObject(tPen);
                DeleteObject(tBr);

                // Draw Orbital Satellites
                for (int s = 0; s < MAX_SATELLITES; s++) {
                    int sx_sat = (int)satellites[s].x;
                    int sy_sat = (int)satellites[s].y;
                    HBRUSH satBr = CreateSolidBrush(dur_satellite > 0 ? RGB(255, 100, 255) : RGB(0, 255, 220));
                    HGDIOBJ oB = SelectObject(memDC, satBr);
                    HPEN satPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                    HGDIOBJ oP = SelectObject(memDC, satPen);
                    
                    Ellipse(memDC, sx_sat - 5, sy_sat - 5, sx_sat + 5, sy_sat + 5);
                    
                    // Drone antenna wings
                    MoveToEx(memDC, sx_sat - 7, sy_sat, NULL); LineTo(memDC, sx_sat + 7, sy_sat);
                    MoveToEx(memDC, sx_sat, sy_sat - 7, NULL); LineTo(memDC, sx_sat, sy_sat + 7);

                    SelectObject(memDC, oB);
                    SelectObject(memDC, oP);
                    DeleteObject(satBr);
                    DeleteObject(satPen);
                }

                if (dur_laser > 0 || active_valkyrie) {
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

                for (int i = 0; i < MAX_LASERS; i++) {
                    if (lasers[i].active) {
                        COLORREF lClr = lasers[i].type == 1 ? RGB(255, 100, 255) : (lasers[i].type == 2 ? RGB(0, 255, 200) : RGB(0, 255, 255));
                        HBRUSH lasBr = CreateSolidBrush(lClr);
                        RECT rl = { (int)lasers[i].x - 2, (int)lasers[i].y - 5, (int)lasers[i].x + 2, (int)lasers[i].y + 5 };
                        FillRect(memDC, &rl, lasBr);
                        DeleteObject(lasBr);
                    }
                }

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
                    else if (power_type == 8) { pClr = RGB(160, 32, 240); pBadge[0] = 'G'; }
                    else if (power_type == 9) { pClr = RGB(0, 255, 220); pBadge[0] = 'D'; } // Drone Satellite
                    
                    HBRUSH pwBr = CreateSolidBrush(pClr);
                    HGDIOBJ oldB = SelectObject(memDC, pwBr);
                    HPEN pwPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                    HGDIOBJ oldP = SelectObject(memDC, pwPen);
                    
                    if (power_type == 1 || power_type == 4) {
                        RoundRect(memDC, (int)power_x - 8, (int)power_y - 6, (int)power_x + 8, (int)power_y + 6, 6, 6);
                    } else if (power_type == 2 || power_type == 6 || power_type == 9) {
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

                for (int i = 0; i < MAX_PARTICLES; i++) {
                    if (particles[i].life > 0) {
                        if (particles[i].type == 0) {
                            // Layer 0: Needle sparks with velocity streak
                            HPEN spPen = CreatePen(PS_SOLID, particles[i].size, RGB(255, 255, 255));
                            HGDIOBJ oP = SelectObject(memDC, spPen);
                            MoveToEx(memDC, (int)particles[i].prev_x, (int)particles[i].prev_y, NULL);
                            LineTo(memDC, (int)particles[i].x, (int)particles[i].y);
                            SelectObject(memDC, oP);
                            DeleteObject(spPen);
                        } else if (particles[i].type == 1) {
                            // Layer 1: Buoyant smoke puff
                            HBRUSH pBr = CreateSolidBrush(particles[i].color);
                            HPEN nonePen = CreatePen(PS_NULL, 0, 0);
                            HGDIOBJ oP = SelectObject(memDC, nonePen);
                            HGDIOBJ oB = SelectObject(memDC, pBr);
                            int s = particles[i].size;
                            Ellipse(memDC, (int)particles[i].x - s, (int)particles[i].y - s, (int)particles[i].x + s, (int)particles[i].y + s);
                            SelectObject(memDC, oP);
                            SelectObject(memDC, oB);
                            DeleteObject(nonePen);
                            DeleteObject(pBr);
                        } else if (particles[i].type == 2) {
                            // Layer 2: Heavy debris shard with tumbling rotation
                            HBRUSH pBr = CreateSolidBrush(particles[i].color);
                            HGDIOBJ oB = SelectObject(memDC, pBr);
                            HPEN dPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                            HGDIOBJ oP = SelectObject(memDC, dPen);
                            int s = particles[i].size;
                            int px = (int)particles[i].x, py = (int)particles[i].y;
                            POINT tri[3] = {
                                { px, py - s },
                                { px + s, py + s },
                                { px - s, py + s / 2 }
                            };
                            Polygon(memDC, tri, 3);
                            SelectObject(memDC, oP);
                            SelectObject(memDC, oB);
                            DeleteObject(dPen);
                            DeleteObject(pBr);
                        } else if (particles[i].type == 3) {
                            // Layer 3: Radiant celebration star
                            HPEN sPen = CreatePen(PS_SOLID, 1, particles[i].color);
                            HGDIOBJ oP = SelectObject(memDC, sPen);
                            int px = (int)particles[i].x, py = (int)particles[i].y;
                            int s = particles[i].size;
                            MoveToEx(memDC, px - s, py, NULL); LineTo(memDC, px + s, py);
                            MoveToEx(memDC, px, py - s, NULL); LineTo(memDC, px, py + s);
                            SetPixel(memDC, px, py, RGB(255, 255, 255));
                            SelectObject(memDC, oP);
                            DeleteObject(sPen);
                        }
                    }
                }

                for (int i = 0; i < MAX_SHOCKWAVES; i++) {
                    if (shockwaves[i].life > 0.0f) {
                        int penW = (shockwaves[i].is_inner ? 2 : 1) + (int)(shockwaves[i].life * 3.0f);
                        HPEN swPen = CreatePen(PS_SOLID, penW, shockwaves[i].color);
                        HGDIOBJ oldP = SelectObject(memDC, swPen);
                        HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
                        HGDIOBJ oldB = SelectObject(memDC, nullBr);
                        
                        int rx = (int)shockwaves[i].r;
                        int ry = (int)(shockwaves[i].r * 0.65f);
                        Ellipse(memDC, (int)shockwaves[i].x - rx, (int)shockwaves[i].y - ry,
                                       (int)shockwaves[i].x + rx, (int)shockwaves[i].y + ry);
                        
                        SelectObject(memDC, oldP);
                        SelectObject(memDC, oldB);
                        DeleteObject(swPen);
                    }
                }

                for (int i = 0; i < MAX_BALLS; i++) {
                    if (balls[i].active) {
                        COLORREF tBaseClr = (dur_fire > 0 || active_supernova) ? RGB(255, 100, 0) : (chaos_mode ? RGB(255, 50, 255) : RGB(0, 200, 255));
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
                    
                        COLORREF bClr = (dur_fire > 0 || active_supernova) ? RGB(255, 50, 0) : (chaos_mode ? RGB(255, 150, 255) : RGB(0, 255, 255));
                        HBRUSH bBr = CreateSolidBrush(bClr);
                        HGDIOBJ oB_ball = SelectObject(memDC, bBr);
                        HPEN bPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                        HGDIOBJ oP_ball = SelectObject(memDC, bPen);
                        Ellipse(memDC, (int)balls[i].x, (int)balls[i].y, (int)balls[i].x + 8, (int)balls[i].y + 8);
                        SelectObject(memDC, oB_ball);
                        SelectObject(memDC, oP_ball);
                        DeleteObject(bBr);
                        DeleteObject(bPen);
                        
                        HPEN wPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                        HGDIOBJ oP_spin = SelectObject(memDC, wPen);
                        int bx = (int)balls[i].x + 4;
                        int by = (int)balls[i].y + 4;
                        if ((frame_counter % 8) < 4) {
                            MoveToEx(memDC, bx - 2, by, NULL); LineTo(memDC, bx + 2, by);
                            MoveToEx(memDC, bx, by - 2, NULL); LineTo(memDC, bx, by + 2);
                        } else {
                            MoveToEx(memDC, bx - 2, by - 2, NULL); LineTo(memDC, bx + 2, by + 2);
                            MoveToEx(memDC, bx - 2, by + 2, NULL); LineTo(memDC, bx + 2, by - 2);
                        }
                        SelectObject(memDC, oP_spin);
                        DeleteObject(wPen);
                    }
                }

                char skStr[160];
                wsprintfA(skStr, "[L]Las:%s [M]Splt:%s [F]Fir:%s [B]Bar:%s [G]Grv:%s [S]Sat:%s [O]FORGE",
                    dur_laser > 0 ? "ACT" : cd_laser <= 0 ? "RDY" : "CD",
                    cd_multi <= 0 ? "RDY" : "CD",
                    dur_fire > 0 ? "ACT" : cd_fire <= 0 ? "RDY" : "CD",
                    dur_barrier > 0 ? "ACT" : cd_barrier <= 0 ? "RDY" : "CD",
                    dur_gravity > 0 ? "ACT" : cd_gravity <= 0 ? "RDY" : "CD",
                    dur_satellite > 0 ? "ACT" : cd_satellite <= 0 ? "RDY" : "CD");
                SetTextColor(memDC, RGB(180, 220, 255));
                TextOutA(memDC, 2, H - 16, skStr, lstrlenA(skStr));
            }
            
            DrawHUDCornerReticles(memDC);
            SetViewportOrgEx(memDC, 0, 0, NULL);

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
            if (chaos_mode) wsprintfA(infoStr, "CHAOS!  MAT:%d/%d/%d", plasma_shards, quantum_cores, nano_alloys);
            else if (level == 99) wsprintfA(infoStr, "STG: CSTM  HI: %d", high_score);
            else wsprintfA(infoStr, "STG: %d  HI: %d", level, high_score);
            
            SetBkMode(memDC, TRANSPARENT);
            DrawBevelBox(memDC, 10, 5, 100, 20, scStr, lPen, dPen);
            DrawBevelBox(memDC, 115, 5, 75, 20, lvStr, lPen, dPen);
            SetTextColor(memDC, chaos_mode ? RGB(255, 100, 255) : RGB(200, 200, 200));
            TextOutA(memDC, 198, 8, infoStr, lstrlenA(infoStr));
            
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
