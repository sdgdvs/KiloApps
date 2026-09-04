#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define BTN_INCUBATE 1
#define BTN_FEED     2
#define BTN_PLAY     3
#define BTN_SLEEP    4
#define TIMER_ID     1

int state = 0; // 0 = egg, 1 = dragon, 2 = adult dragon, 3 = train menu, 4 = str minigame, 5 = spd minigame, 6 = loy minigame, 7 = expedition, 8 = battle, 9 = shop, 10 = event
int element = 0; // 0=none, 2=fire, 3=earth, 4=water
int feed_count = 0;
int play_count = 0;
int sleep_count = 0;
int hunger = 50;
int happiness = 50;
int energy = 100;
int age = 0;

int strength = 0;
int speed = 0;
int loyalty = 0;
int gold = 0;
int prev_state = 1;
int bat_player_hp = 100;
int bat_player_max = 100;
int bat_enemy_hp = 100;
int bat_enemy_max = 100;
int bat_enemy_str = 10;
int bat_enemy_spd = 10;
int minigame_val = 0;
int minigame_dir = 1;
int minigame_state = 0;
DWORD minigame_start_time = 0;

#define MAX_LOG_LINES 4
char log_messages[MAX_LOG_LINES][128] = {0};
int log_count = 0;

#define BTN_TRAIN     5
#define BTN_TR_STR    6
#define BTN_TR_SPD    7
#define BTN_TR_LOY    8
#define BTN_TR_BACK   9
#define BTN_STR_HIT   10
#define BTN_SPD_REACT 11
#define BTN_LOY_1     12
#define BTN_LOY_2     13
#define BTN_LOY_3     14
#define BTN_HOARD     15
#define BTN_BATTLE    16
#define BTN_BAT_ATK   17
#define BTN_BAT_DEF   18
#define BTN_BAT_SPEC  19
#define BTN_BAT_FLEE  20
#define BTN_SHOP      21
#define BTN_SHP_FOOD  22
#define BTN_SHP_TOY   23
#define BTN_SHP_STR   24
#define BTN_SHP_SPD   25
#define BTN_SHP_BACK  26
#define BTN_EVT_OPT1  27
#define BTN_EVT_OPT2  28
#define BTN_HELP      29

HWND btn_incubate, btn_feed, btn_play, btn_sleep, btn_train, btn_hoard, btn_battle, btn_shop, btn_help;
HWND btn_tr_str, btn_tr_spd, btn_tr_loy, btn_tr_back;
HWND btn_str_hit, btn_spd_react, btn_loy_1, btn_loy_2, btn_loy_3;
HWND btn_bat_atk, btn_bat_def, btn_bat_spec, btn_bat_flee;
HWND btn_shp_food, btn_shp_toy, btn_shp_str, btn_shp_spd, btn_shp_back;
HWND btn_evt_opt1, btn_evt_opt2;
HFONT hFontNormal, hFontLarge, hFontTitle, hFontSmall;
HBRUSH bgBrush;

int current_event_id = 0;

// === VISUAL EFFECTS & KINEMATIC PARTICLE ENGINE ===
struct Particle {
    float x, y;
    float vx, vy;
    int life, max_life;
    int type; // 0=spark, 1=smoke/plasma, 2=debris/scale, 3=star
    float size;
    COLORREF color;
};
#define MAX_PARTICLES 120
struct Particle particles[MAX_PARTICLES];

struct Shockwave {
    float x, y;
    float radius;
    float max_radius;
    float speed;
    COLORREF color;
    int active;
};
#define MAX_SHOCKWAVES 8
struct Shockwave shockwaves[MAX_SHOCKWAVES];

struct AmbientMote {
    float x, y;
    float vx, vy;
    float size;
    int alpha;
    int phase;
};
#define MAX_MOTES 25
struct AmbientMote ambient_motes[MAX_MOTES];

float screen_shake_amp = 0.0f;
float screen_shake_angle = 0.0f;
int anim_tick = 0;
int player_attack_offset = 0;
int enemy_attack_offset = 0;
int player_damage_flash = 0;
int enemy_damage_flash = 0;

void init_ambient_motes() {
    for (int i = 0; i < MAX_MOTES; i++) {
        ambient_motes[i].x = (float)(rand() % 580 + 10);
        ambient_motes[i].y = (float)(rand() % 460 + 10);
        ambient_motes[i].vx = ((rand() % 20) - 10) * 0.03f;
        ambient_motes[i].vy = -0.2f - (rand() % 10) * 0.04f;
        ambient_motes[i].size = 2.0f + (rand() % 4);
        ambient_motes[i].phase = rand() % 360;
    }
}

void trigger_screen_shake(float amp) {
    if (amp > screen_shake_amp) {
        screen_shake_amp = amp;
    }
}

void add_shockwave(float x, float y, float max_r, COLORREF color) {
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (!shockwaves[i].active) {
            shockwaves[i].x = x;
            shockwaves[i].y = y;
            shockwaves[i].radius = 4.0f;
            shockwaves[i].max_radius = max_r;
            shockwaves[i].speed = 4.5f;
            shockwaves[i].color = color;
            shockwaves[i].active = 1;
            break;
        }
    }
}

void spawn_particles_ext(int x, int y, COLORREF color, int count, int type) {
    for (int i = 0; i < MAX_PARTICLES && count > 0; i++) {
        if (particles[i].life <= 0) {
            particles[i].x = (float)x + ((rand() % 11) - 5);
            particles[i].y = (float)y + ((rand() % 11) - 5);
            particles[i].type = type;
            particles[i].color = color;

            if (type == 0) { // Needle spark
                float ang = (float)(rand() % 360) * 3.14159f / 180.0f;
                float spd = 3.0f + (rand() % 40) * 0.1f;
                particles[i].vx = cosf(ang) * spd;
                particles[i].vy = sinf(ang) * spd;
                particles[i].life = particles[i].max_life = 12 + (rand() % 10);
                particles[i].size = 2.0f + (rand() % 2);
            } else if (type == 1) { // Expanding smoke / breath puff
                particles[i].vx = ((rand() % 21) - 10) * 0.15f;
                particles[i].vy = -1.0f - (rand() % 15) * 0.1f;
                particles[i].life = particles[i].max_life = 20 + (rand() % 15);
                particles[i].size = 4.0f + (rand() % 4);
            } else if (type == 2) { // Heavy debris / scale shard
                particles[i].vx = ((rand() % 41) - 20) * 0.2f;
                particles[i].vy = -3.0f - (rand() % 25) * 0.2f;
                particles[i].life = particles[i].max_life = 25 + (rand() % 15);
                particles[i].size = 3.0f + (rand() % 3);
            } else if (type == 3) { // Golden celebration star
                float ang = (float)(rand() % 360) * 3.14159f / 180.0f;
                float spd = 2.0f + (rand() % 30) * 0.1f;
                particles[i].vx = cosf(ang) * spd;
                particles[i].vy = sinf(ang) * spd - 1.0f;
                particles[i].life = particles[i].max_life = 30 + (rand() % 15);
                particles[i].size = 3.0f + (rand() % 3);
            }
            count--;
        }
    }
}

void spawn_particles(int x, int y, COLORREF color, int count) {
    spawn_particles_ext(x, y, color, count / 2, 0);
    spawn_particles_ext(x, y, color, count / 2, 1);
}

void add_log(const char* msg) {
    for (int i = MAX_LOG_LINES - 1; i > 0; --i) {
        strcpy(log_messages[i], log_messages[i-1]);
    }
    strcpy(log_messages[0], msg);
    if (log_count < MAX_LOG_LINES) log_count++;
}

COLORREF egg_pixels[16][16] = {
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, RGB(180,150,100), RGB(180,150,100), RGB(180,150,100), RGB(180,150,100), -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(255,255,255), RGB(180,150,100), -1, -1, -1, -1},
    {-1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1},
    {-1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1},
    {-1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1},
    {-1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1},
    {-1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1},
    {-1, -1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, RGB(180,150,100), RGB(180,150,100), RGB(180,150,100), RGB(180,150,100), RGB(180,150,100), RGB(180,150,100), -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};

COLORREF dragon_pixels[16][16] = {
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, RGB(30,100,30), RGB(30,100,30), -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), -1, RGB(30,100,30), -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), RGB(30,100,30), RGB(30,100,30), -1, -1},
    {-1, RGB(30,100,30), -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), -1},
    {-1, RGB(30,100,30), RGB(30,100,30), -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), RGB(60,180,60), RGB(30,100,30), -1, -1},
    {-1, RGB(30,100,30), RGB(60,180,60), RGB(30,100,30), RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), RGB(60,180,60), RGB(30,100,30), -1, -1},
    {-1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), RGB(30,100,30), RGB(30,100,30), -1, -1},
    {-1, -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), -1, -1, -1, -1},
    {-1, -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), -1, -1, -1, -1, -1},
    {-1, -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(30,100,30), -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(30,100,30), -1, -1, -1, -1, -1},
    {-1, -1, -1, RGB(30,100,30), RGB(30,100,30), -1, -1, -1, -1, RGB(30,100,30), RGB(30,100,30), -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};

COLORREF adult_dragon_pixels[16][16] = {
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, RGB(180,30,30), RGB(180,30,30), -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), -1, -1, -1, -1, RGB(180,30,30), -1, -1, -1},
    {-1, -1, -1, RGB(180,30,30), RGB(220,60,60), RGB(255,255,255), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), -1, -1, RGB(180,30,30), RGB(180,30,30), -1, -1, -1},
    {-1, -1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), RGB(180,30,30), RGB(180,30,30), -1, -1, -1, -1},
    {-1, -1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), RGB(180,30,30), -1, -1, -1, -1, -1},
    {-1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), -1, -1, -1, -1, -1, -1},
    {-1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), -1, -1, -1, -1, -1, -1},
    {-1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), RGB(180,30,30), RGB(180,30,30), -1, -1, -1, -1},
    {-1, -1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), -1, -1, -1},
    {-1, -1, -1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), RGB(180,30,30), -1, -1},
    {-1, -1, -1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), -1, -1},
    {-1, -1, -1, RGB(180,30,30), RGB(180,30,30), RGB(180,30,30), RGB(180,30,30), -1, -1, RGB(180,30,30), RGB(180,30,30), RGB(180,30,30), RGB(180,30,30), -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};

void DrawPixelArt(HDC hdc, int x, int y, int scale, COLORREF pixels[16][16], int element_type, int is_flipped, int flash_white) {
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int src_j = is_flipped ? (15 - j) : j;
            if (pixels[i][src_j] != -1) {
                COLORREF c = pixels[i][src_j];
                if (flash_white) {
                    c = RGB(255, 255, 255);
                } else {
                    if (element_type == 4) { // Water
                        if (c == RGB(180,30,30)) c = RGB(30,60,180);
                        else if (c == RGB(220,60,60)) c = RGB(60,120,220);
                    } else if (element_type == 3) { // Earth
                        if (c == RGB(180,30,30)) c = RGB(100,60,30);
                        else if (c == RGB(220,60,60)) c = RGB(150,100,60);
                    } else if (element_type == 5) { // Enemy
                        if (c == RGB(180,30,30)) c = RGB(128,0,128);
                        else if (c == RGB(220,60,60)) c = RGB(180,50,180);
                    }
                }
                HBRUSH b = CreateSolidBrush(c);
                RECT r = {x + j*scale, y + i*scale, x + (j+1)*scale, y + (i+1)*scale};
                FillRect(hdc, &r, b);
                DeleteObject(b);
            }
        }
    }
}

// Draw Ornate Medieval HUD Corner Filigree L-Brackets with Rivets
void DrawFiligreeCorner(HDC hdc, int x, int y, int size, int align_x, int align_y) {
    HPEN goldPen = CreatePen(PS_SOLID, 2, RGB(180, 130, 40));
    HPEN darkPen = CreatePen(PS_SOLID, 1, RGB(80, 50, 20));
    HBRUSH goldBrush = CreateSolidBrush(RGB(220, 180, 70));
    
    HGDIOBJ oldPen = SelectObject(hdc, goldPen);
    
    int sx = (align_x > 0) ? 1 : -1;
    int sy = (align_y > 0) ? 1 : -1;
    
    // Outer bracket
    MoveToEx(hdc, x, y + sy * size, NULL);
    LineTo(hdc, x, y);
    LineTo(hdc, x + sx * size, y);
    
    // Inner bracket notch
    MoveToEx(hdc, x + sx * 4, y + sy * (size - 4), NULL);
    LineTo(hdc, x + sx * 4, y + sy * 4);
    LineTo(hdc, x + sx * (size - 4), y + sy * 4);
    
    // Rivet accent
    SelectObject(hdc, goldBrush);
    SelectObject(hdc, darkPen);
    int rx = x + sx * 6;
    int ry = y + sy * 6;
    Ellipse(hdc, rx - 2, ry - 2, rx + 3, ry + 3);
    
    SelectObject(hdc, oldPen);
    DeleteObject(goldPen);
    DeleteObject(darkPen);
    DeleteObject(goldBrush);
}

void DrawOrnateFrame(HDC hdc, RECT rect) {
    // Outer border
    HPEN borderPen = CreatePen(PS_SOLID, 3, RGB(90, 56, 20));
    HPEN innerPen = CreatePen(PS_SOLID, 1, RGB(180, 140, 60));
    HGDIOBJ oldPen = SelectObject(hdc, borderPen);
    
    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
    
    // Inner shimmer border
    SelectObject(hdc, innerPen);
    Rectangle(hdc, rect.left + 4, rect.top + 4, rect.right - 4, rect.bottom - 4);
    
    // Corner brackets
    int bracket_size = 14;
    DrawFiligreeCorner(hdc, rect.left + 6, rect.top + 6, bracket_size, 1, 1);
    DrawFiligreeCorner(hdc, rect.right - 6, rect.top + 6, bracket_size, -1, 1);
    DrawFiligreeCorner(hdc, rect.left + 6, rect.bottom - 6, bracket_size, 1, -1);
    DrawFiligreeCorner(hdc, rect.right - 6, rect.bottom - 6, bracket_size, -1, -1);
    
    // Animated traveling specular glint along top/bottom border
    int perimeter = (rect.right - rect.left) * 2 + (rect.bottom - rect.top) * 2;
    int glint_pos = (anim_tick * 4) % perimeter;
    int gx = rect.left, gy = rect.top;
    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;
    
    if (glint_pos < w) {
        gx = rect.left + glint_pos;
        gy = rect.top + 2;
    } else if (glint_pos < w + h) {
        gx = rect.right - 3;
        gy = rect.top + (glint_pos - w);
    } else if (glint_pos < w * 2 + h) {
        gx = rect.right - (glint_pos - (w + h));
        gy = rect.bottom - 3;
    } else {
        gx = rect.left + 2;
        gy = rect.bottom - (glint_pos - (w * 2 + h));
    }
    
    HBRUSH glintBrush = CreateSolidBrush(RGB(255, 255, 220));
    RECT gr = {gx - 2, gy - 2, gx + 3, gy + 3};
    FillRect(hdc, &gr, glintBrush);
    DeleteObject(glintBrush);
    
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(borderPen);
    DeleteObject(innerPen);
}

void DrawShopItemCard(HDC hdc, int x, int y, int item_idx, int is_hovered) {
    RECT r = {x, y, x + 85, y + 42};
    HBRUSH cardBrush = CreateSolidBrush(is_hovered ? RGB(245, 225, 185) : RGB(230, 200, 155));
    HPEN cardPen = CreatePen(PS_SOLID, 2, RGB(110, 70, 30));
    
    HGDIOBJ oldBrush = SelectObject(hdc, cardBrush);
    HGDIOBJ oldPen = SelectObject(hdc, cardPen);
    RoundRect(hdc, r.left, r.top, r.right, r.bottom, 6, 6);
    
    // Icon rendering inside card
    if (item_idx == 0) { // Premium Roast Meat
        HBRUSH boneB = CreateSolidBrush(RGB(240, 240, 220));
        HBRUSH meatB = CreateSolidBrush(RGB(180, 40, 40));
        HBRUSH meatDk = CreateSolidBrush(RGB(120, 20, 20));
        RECT boneR = {x + 10, y + 18, x + 35, y + 24};
        FillRect(hdc, &boneR, boneB);
        SelectObject(hdc, meatB);
        Ellipse(hdc, x + 18, y + 8, x + 38, y + 34);
        SelectObject(hdc, meatDk);
        Ellipse(hdc, x + 24, y + 14, x + 32, y + 28);
        DeleteObject(boneB); DeleteObject(meatB); DeleteObject(meatDk);
    } else if (item_idx == 1) { // Glowing Orb Toy
        HBRUSH orbB = CreateSolidBrush(RGB(50, 120, 220));
        HBRUSH goldRing = CreateSolidBrush(RGB(240, 200, 50));
        SelectObject(hdc, orbB);
        Ellipse(hdc, x + 14, y + 10, x + 34, y + 30);
        SelectObject(hdc, goldRing);
        Ellipse(hdc, x + 20, y + 16, x + 28, y + 24);
        DeleteObject(orbB); DeleteObject(goldRing);
    } else if (item_idx == 2) { // Power Bracer
        HBRUSH ironB = CreateSolidBrush(RGB(90, 95, 105));
        HBRUSH goldR = CreateSolidBrush(RGB(220, 180, 50));
        RECT br = {x + 14, y + 12, x + 34, y + 30};
        FillRect(hdc, &br, ironB);
        RECT gr1 = {x + 12, y + 15, x + 36, y + 18};
        RECT gr2 = {x + 12, y + 24, x + 36, y + 27};
        FillRect(hdc, &gr1, goldR);
        FillRect(hdc, &gr2, goldR);
        DeleteObject(ironB); DeleteObject(goldR);
    } else if (item_idx == 3) { // Swift Boots
        HBRUSH bootB = CreateSolidBrush(RGB(140, 80, 30));
        HBRUSH wingB = CreateSolidBrush(RGB(230, 230, 250));
        RECT b1 = {x + 14, y + 10, x + 24, y + 28};
        RECT b2 = {x + 14, y + 22, x + 34, y + 30};
        FillRect(hdc, &b1, bootB);
        FillRect(hdc, &b2, bootB);
        SelectObject(hdc, wingB);
        Ellipse(hdc, x + 24, y + 10, x + 35, y + 18);
        DeleteObject(bootB); DeleteObject(wingB);
    }
    
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(cardBrush);
    DeleteObject(cardPen);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: {
            HDC hdcScreen = GetDC(NULL);
            int dpi = GetDeviceCaps(hdcScreen, LOGPIXELSY);
            ReleaseDC(NULL, hdcScreen);
            int fontHeight = -MulDiv(11, dpi, 72);
            srand(GetTickCount());
            
            init_ambient_motes();
            
            hFontNormal = CreateFont(fontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                     DEFAULT_PITCH | FF_DONTCARE, "Georgia");
            hFontSmall = CreateFont(-MulDiv(9, dpi, 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                    DEFAULT_PITCH | FF_DONTCARE, "Courier New");
            hFontTitle = CreateFont(-MulDiv(16, dpi, 72), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                    DEFAULT_PITCH | FF_DONTCARE, "Georgia");
            hFontLarge = CreateFont(80, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                    DEFAULT_PITCH | FF_DONTCARE, "Segoe UI Emoji");
            bgBrush = CreateSolidBrush(RGB(220, 184, 129));
            
            btn_incubate = CreateWindow("BUTTON", "Incubate Egg", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                        230, 260, 120, 40, hwnd, (HMENU)BTN_INCUBATE, NULL, NULL);
            btn_feed = CreateWindow("BUTTON", "Feed", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                    35, 260, 80, 40, hwnd, (HMENU)BTN_FEED, NULL, NULL);
            btn_play = CreateWindow("BUTTON", "Play", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                    125, 260, 80, 40, hwnd, (HMENU)BTN_PLAY, NULL, NULL);
            btn_sleep = CreateWindow("BUTTON", "Sleep", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                     215, 260, 80, 40, hwnd, (HMENU)BTN_SLEEP, NULL, NULL);
            btn_train = CreateWindow("BUTTON", "Train", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                     305, 260, 80, 40, hwnd, (HMENU)BTN_TRAIN, NULL, NULL);
            btn_hoard = CreateWindow("BUTTON", "Hoard", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                     395, 260, 80, 40, hwnd, (HMENU)BTN_HOARD, NULL, NULL);
            btn_battle = CreateWindow("BUTTON", "Battle", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                      485, 260, 80, 40, hwnd, (HMENU)BTN_BATTLE, NULL, NULL);
            btn_shop = CreateWindow("BUTTON", "Shop", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                      485, 215, 80, 40, hwnd, (HMENU)BTN_SHOP, NULL, NULL);
            btn_help = CreateWindow("BUTTON", "Help", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                      395, 215, 80, 40, hwnd, (HMENU)BTN_HELP, NULL, NULL);

            btn_bat_atk = CreateWindow("BUTTON", "Attack", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                       90, 270, 90, 40, hwnd, (HMENU)BTN_BAT_ATK, NULL, NULL);
            btn_bat_def = CreateWindow("BUTTON", "Defend", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                       190, 270, 90, 40, hwnd, (HMENU)BTN_BAT_DEF, NULL, NULL);
            btn_bat_spec = CreateWindow("BUTTON", "Special", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                       290, 270, 90, 40, hwnd, (HMENU)BTN_BAT_SPEC, NULL, NULL);
            btn_bat_flee = CreateWindow("BUTTON", "Flee", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                        390, 270, 90, 40, hwnd, (HMENU)BTN_BAT_FLEE, NULL, NULL);

            btn_shp_food = CreateWindow("BUTTON", "Meat 20g", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 35, 260, 80, 40, hwnd, (HMENU)BTN_SHP_FOOD, NULL, NULL);
            btn_shp_toy = CreateWindow("BUTTON", "Toy 30g", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 125, 260, 80, 40, hwnd, (HMENU)BTN_SHP_TOY, NULL, NULL);
            btn_shp_str = CreateWindow("BUTTON", "Str 50g", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 215, 260, 80, 40, hwnd, (HMENU)BTN_SHP_STR, NULL, NULL);
            btn_shp_spd = CreateWindow("BUTTON", "Spd 50g", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 305, 260, 80, 40, hwnd, (HMENU)BTN_SHP_SPD, NULL, NULL);
            btn_shp_back = CreateWindow("BUTTON", "Back", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 395, 260, 80, 40, hwnd, (HMENU)BTN_SHP_BACK, NULL, NULL);

            btn_evt_opt1 = CreateWindow("BUTTON", "Opt 1", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 150, 270, 130, 40, hwnd, (HMENU)BTN_EVT_OPT1, NULL, NULL);
            btn_evt_opt2 = CreateWindow("BUTTON", "Opt 2", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 300, 270, 130, 40, hwnd, (HMENU)BTN_EVT_OPT2, NULL, NULL);

            btn_tr_str = CreateWindow("BUTTON", "Strength", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                     80, 260, 100, 40, hwnd, (HMENU)BTN_TR_STR, NULL, NULL);
            btn_tr_spd = CreateWindow("BUTTON", "Speed", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                     190, 260, 100, 40, hwnd, (HMENU)BTN_TR_SPD, NULL, NULL);
            btn_tr_loy = CreateWindow("BUTTON", "Loyalty", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                     300, 260, 100, 40, hwnd, (HMENU)BTN_TR_LOY, NULL, NULL);
            btn_tr_back = CreateWindow("BUTTON", "Back", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                     410, 260, 100, 40, hwnd, (HMENU)BTN_TR_BACK, NULL, NULL);

            btn_str_hit = CreateWindow("BUTTON", "Hit!", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                     250, 270, 100, 40, hwnd, (HMENU)BTN_STR_HIT, NULL, NULL);
            btn_spd_react = CreateWindow("BUTTON", "React!", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                     250, 270, 100, 40, hwnd, (HMENU)BTN_SPD_REACT, NULL, NULL);
            btn_loy_1 = CreateWindow("BUTTON", "Box 1", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                     140, 270, 100, 40, hwnd, (HMENU)BTN_LOY_1, NULL, NULL);
            btn_loy_2 = CreateWindow("BUTTON", "Box 2", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                     250, 270, 100, 40, hwnd, (HMENU)BTN_LOY_2, NULL, NULL);
            btn_loy_3 = CreateWindow("BUTTON", "Box 3", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                     360, 270, 100, 40, hwnd, (HMENU)BTN_LOY_3, NULL, NULL);
                                     
            SendMessage(btn_incubate, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_feed, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_play, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_sleep, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_train, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_hoard, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_battle, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_shop, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_help, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_tr_str, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_tr_spd, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_tr_loy, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_tr_back, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_str_hit, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_spd_react, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_loy_1, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_loy_2, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_loy_3, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_bat_atk, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_bat_def, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_bat_spec, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_bat_flee, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_shp_food, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_shp_toy, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_shp_str, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_shp_spd, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_shp_back, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_evt_opt1, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_evt_opt2, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            
            SetTimer(hwnd, 5, 33, NULL); // 30 FPS animation timer
            }
            break;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == BTN_INCUBATE) {
                state = 1;
                add_log("The egg hatched! A baby dragon emerged.");
                Beep(150, 100); Beep(100, 200); Beep(80, 200);
                trigger_screen_shake(6.0f);
                add_shockwave(290, 160, 100.0f, RGB(255, 230, 150));
                spawn_particles_ext(290, 160, RGB(255,255,255), 30, 0);
                spawn_particles_ext(290, 160, RGB(255,215,0), 20, 3);
                ShowWindow(btn_incubate, SW_HIDE);
                ShowWindow(btn_feed, SW_SHOW);
                ShowWindow(btn_play, SW_SHOW);
                ShowWindow(btn_sleep, SW_SHOW);
                ShowWindow(btn_train, SW_SHOW);
                ShowWindow(btn_hoard, SW_SHOW);
                ShowWindow(btn_battle, SW_SHOW);
                ShowWindow(btn_shop, SW_SHOW);
                ShowWindow(btn_help, SW_SHOW);
                SetTimer(hwnd, TIMER_ID, 3000, NULL);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_FEED) {
                if (hunger >= 100) {
                    add_log("Dragon is full and refuses to eat.");
                } else {
                    feed_count++;
                    hunger = hunger + 20;
                    if (hunger > 100) hunger = 100;
                    energy = energy - 5;
                    if (energy < 0) energy = 0;
                    add_log("You fed the dragon. It looks satisfied.");
                    Beep(400, 50); Beep(600, 50);
                    spawn_particles_ext(300, 150, RGB(255,80,80), 16, 0);
                    spawn_particles_ext(300, 150, RGB(255,180,50), 10, 1);
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_PLAY) {
                if (energy < 20) {
                    add_log("Dragon is too tired to play.");
                } else {
                    play_count++;
                    happiness = happiness + 20;
                    if (happiness > 100) happiness = 100;
                    energy = energy - 20;
                    if (energy < 0) energy = 0;
                    hunger = hunger - 10;
                    if (hunger < 0) hunger = 0;
                    add_log("You played with the dragon! It's happy.");
                    trigger_screen_shake(3.0f);
                    spawn_particles_ext(300, 150, RGB(255,255,80), 16, 0);
                    spawn_particles_ext(300, 150, RGB(255,215,0), 12, 3);
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_SLEEP) {
                sleep_count++;
                energy = energy + 40;
                if (energy > 100) energy = 100;
                hunger = hunger - 10;
                if (hunger < 0) hunger = 0;
                add_log("Dragon took a nap and regained energy.");
                spawn_particles_ext(300, 150, RGB(120,160,255), 14, 1);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_TRAIN) {
                if (energy < 15) {
                    add_log("Dragon is too tired to train.");
                } else {
                    ShowWindow(btn_feed, SW_HIDE);
                    ShowWindow(btn_play, SW_HIDE);
                    ShowWindow(btn_sleep, SW_HIDE);
                    ShowWindow(btn_train, SW_HIDE);
                    ShowWindow(btn_hoard, SW_HIDE);
                    ShowWindow(btn_battle, SW_HIDE);
                    ShowWindow(btn_shop, SW_HIDE);
                    ShowWindow(btn_help, SW_HIDE);
                    ShowWindow(btn_tr_str, SW_SHOW);
                    ShowWindow(btn_tr_spd, SW_SHOW);
                    ShowWindow(btn_tr_loy, SW_SHOW);
                    ShowWindow(btn_tr_back, SW_SHOW);
                    prev_state = state;
                    state = 3;
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_TR_BACK) {
                ShowWindow(btn_tr_str, SW_HIDE); ShowWindow(btn_tr_spd, SW_HIDE);
                ShowWindow(btn_tr_loy, SW_HIDE); ShowWindow(btn_tr_back, SW_HIDE);
                ShowWindow(btn_feed, SW_SHOW); ShowWindow(btn_play, SW_SHOW);
                ShowWindow(btn_sleep, SW_SHOW); ShowWindow(btn_train, SW_SHOW);
                ShowWindow(btn_hoard, SW_SHOW); ShowWindow(btn_battle, SW_SHOW);
                ShowWindow(btn_shop, SW_SHOW); ShowWindow(btn_help, SW_SHOW);
                state = prev_state;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_TR_STR) {
                ShowWindow(btn_tr_str, SW_HIDE); ShowWindow(btn_tr_spd, SW_HIDE);
                ShowWindow(btn_tr_loy, SW_HIDE); ShowWindow(btn_tr_back, SW_HIDE);
                energy -= 15; if (energy < 0) energy = 0;
                hunger -= 5; if (hunger < 0) hunger = 0;
                state = 4;
                minigame_val = 0; minigame_dir = 1;
                ShowWindow(btn_str_hit, SW_SHOW);
                SetTimer(hwnd, 2, 40, NULL);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_TR_SPD) {
                ShowWindow(btn_tr_str, SW_HIDE); ShowWindow(btn_tr_spd, SW_HIDE);
                ShowWindow(btn_tr_loy, SW_HIDE); ShowWindow(btn_tr_back, SW_HIDE);
                energy -= 15; if (energy < 0) energy = 0;
                hunger -= 5; if (hunger < 0) hunger = 0;
                state = 5;
                minigame_state = 0;
                minigame_start_time = GetTickCount() + 1000 + (GetTickCount() % 2000);
                ShowWindow(btn_spd_react, SW_SHOW);
                SetTimer(hwnd, 3, 30, NULL);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_TR_LOY) {
                ShowWindow(btn_tr_str, SW_HIDE); ShowWindow(btn_tr_spd, SW_HIDE);
                ShowWindow(btn_tr_loy, SW_HIDE); ShowWindow(btn_tr_back, SW_HIDE);
                energy -= 15; if (energy < 0) energy = 0;
                hunger -= 5; if (hunger < 0) hunger = 0;
                state = 6;
                minigame_val = GetTickCount() % 3;
                ShowWindow(btn_loy_1, SW_SHOW);
                ShowWindow(btn_loy_2, SW_SHOW);
                ShowWindow(btn_loy_3, SW_SHOW);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_STR_HIT) {
                KillTimer(hwnd, 2);
                int gain = minigame_val / 20;
                strength += gain;
                char logMsg[128];
                sprintf(logMsg, "You hit with power %d! Strength +%d", minigame_val, gain);
                add_log(logMsg);
                trigger_screen_shake(minigame_val > 70 ? 7.0f : 3.0f);
                spawn_particles_ext(300, 245, RGB(255,100,50), 20, 0);
                ShowWindow(btn_str_hit, SW_HIDE);
                ShowWindow(btn_feed, SW_SHOW); ShowWindow(btn_play, SW_SHOW);
                ShowWindow(btn_sleep, SW_SHOW); ShowWindow(btn_train, SW_SHOW);
                ShowWindow(btn_hoard, SW_SHOW); ShowWindow(btn_battle, SW_SHOW);
                ShowWindow(btn_shop, SW_SHOW); ShowWindow(btn_help, SW_SHOW);
                state = prev_state;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_SPD_REACT) {
                KillTimer(hwnd, 3);
                if (minigame_state == 0) {
                    add_log("Too early! You missed.");
                } else {
                    DWORD time = GetTickCount() - minigame_start_time;
                    int gain = time < 300 ? 5 : (time < 500 ? 3 : 1);
                    speed += gain;
                    char logMsg[128];
                    sprintf(logMsg, "Reaction time: %dms! Speed +%d", (int)time, gain);
                    add_log(logMsg);
                    trigger_screen_shake(4.0f);
                    spawn_particles_ext(300, 245, RGB(50,220,255), 20, 0);
                }
                ShowWindow(btn_spd_react, SW_HIDE);
                ShowWindow(btn_feed, SW_SHOW); ShowWindow(btn_play, SW_SHOW);
                ShowWindow(btn_sleep, SW_SHOW); ShowWindow(btn_train, SW_SHOW);
                ShowWindow(btn_hoard, SW_SHOW); ShowWindow(btn_battle, SW_SHOW);
                ShowWindow(btn_shop, SW_SHOW); ShowWindow(btn_help, SW_SHOW);
                state = prev_state;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) >= BTN_LOY_1 && LOWORD(wParam) <= BTN_LOY_3) {
                int picked = LOWORD(wParam) - BTN_LOY_1;
                if (picked == minigame_val) {
                    loyalty += 5;
                    add_log("You found the treat! Loyalty +5");
                    trigger_screen_shake(4.0f);
                    spawn_particles_ext(190 + picked * 110, 280, RGB(255,215,0), 20, 3);
                } else {
                    loyalty += 1;
                    add_log("Empty box. Loyalty +1");
                }
                ShowWindow(btn_loy_1, SW_HIDE); ShowWindow(btn_loy_2, SW_HIDE); ShowWindow(btn_loy_3, SW_HIDE);
                ShowWindow(btn_feed, SW_SHOW); ShowWindow(btn_play, SW_SHOW);
                ShowWindow(btn_sleep, SW_SHOW); ShowWindow(btn_train, SW_SHOW);
                ShowWindow(btn_hoard, SW_SHOW); ShowWindow(btn_battle, SW_SHOW);
                ShowWindow(btn_shop, SW_SHOW); ShowWindow(btn_help, SW_SHOW);
                state = prev_state;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_HOARD) {
                if (energy < 30) {
                    add_log("Dragon is too tired for an expedition.");
                } else {
                    energy -= 30;
                    hunger -= 15; if (hunger < 0) hunger = 0;
                    add_log("Dragon departed on an expedition...");
                    ShowWindow(btn_feed, SW_HIDE); ShowWindow(btn_play, SW_HIDE);
                    ShowWindow(btn_sleep, SW_HIDE); ShowWindow(btn_train, SW_HIDE);
                    ShowWindow(btn_hoard, SW_HIDE); ShowWindow(btn_battle, SW_HIDE);
                    ShowWindow(btn_shop, SW_HIDE); ShowWindow(btn_help, SW_HIDE);
                    prev_state = state;
                    state = 7;
                    SetTimer(hwnd, 4, 4000, NULL);
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_BATTLE) {
                if (energy < 20) {
                    add_log("Dragon is too tired to battle.");
                } else {
                    energy -= 20;
                    hunger -= 10; if (hunger < 0) hunger = 0;
                    bat_player_max = 100 + strength * 5;
                    bat_player_hp = bat_player_max;
                    bat_enemy_str = strength - 2 + (rand() % 5); if (bat_enemy_str < 5) bat_enemy_str = 5;
                    bat_enemy_spd = speed - 2 + (rand() % 5); if (bat_enemy_spd < 5) bat_enemy_spd = 5;
                    bat_enemy_max = 100 + bat_enemy_str * 5;
                    bat_enemy_hp = bat_enemy_max;
                    
                    char msg[128];
                    sprintf(msg, "A wild enemy dragon appears! (HP: %d)", bat_enemy_hp);
                    add_log(msg);
                    Beep(150, 100); Beep(100, 200); Beep(80, 200);
                    trigger_screen_shake(5.0f);
                    add_shockwave(410, 150, 80.0f, RGB(220, 50, 220));
                    
                    ShowWindow(btn_feed, SW_HIDE); ShowWindow(btn_play, SW_HIDE);
                    ShowWindow(btn_sleep, SW_HIDE); ShowWindow(btn_train, SW_HIDE);
                    ShowWindow(btn_hoard, SW_HIDE); ShowWindow(btn_battle, SW_HIDE);
                    ShowWindow(btn_shop, SW_HIDE); ShowWindow(btn_help, SW_HIDE);
                    
                    ShowWindow(btn_bat_atk, SW_SHOW);
                    ShowWindow(btn_bat_def, SW_SHOW);
                    if (element != 0) ShowWindow(btn_bat_spec, SW_SHOW);
                    ShowWindow(btn_bat_flee, SW_SHOW);
                    
                    prev_state = state;
                    state = 8;
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) >= BTN_BAT_ATK && LOWORD(wParam) <= BTN_BAT_FLEE) {
                int action = LOWORD(wParam) - BTN_BAT_ATK; // 0=atk, 1=def, 2=spec, 3=flee
                int battle_ended = 0;
                int won = 0;
                
                if (action == 3) {
                    add_log("You fled the battle!");
                    battle_ended = 1;
                    won = 0;
                } else {
                    int p_def = (action == 1);
                    int e_def = ((rand() % 100) < 30);
                    
                    // Player
                    if (action == 0) {
                        int hitChance = 80 + (speed - bat_enemy_spd) * 5;
                        if ((rand() % 100) < hitChance) {
                            int dmg = strength * 2 + (rand() % 5);
                            if (dmg < 1) dmg = 1;
                            if (e_def) dmg /= 2;
                            bat_enemy_hp -= dmg;
                            char m[128]; sprintf(m, "You hit the enemy for %d damage!", dmg); add_log(m);
                            Beep(800, 50); Beep(100, 50);
                            player_attack_offset = 24;
                            enemy_damage_flash = 6;
                            trigger_screen_shake(6.0f);
                            add_shockwave(410, 150, 50.0f, RGB(255, 80, 80));
                            spawn_particles_ext(410, 150, RGB(255,0,0), 16, 0);
                            spawn_particles_ext(410, 150, RGB(180,50,180), 8, 2);
                        } else {
                            add_log("You missed!");
                        }
                    } else if (action == 2) {
                        if (element == 2) { // Fire
                            int dmg = strength * 3 + 10;
                            if (e_def) dmg /= 2;
                            bat_enemy_hp -= dmg;
                            char m[128]; sprintf(m, "You used Fireball! Dealt %d damage.", dmg); add_log(m);
                            player_attack_offset = 24;
                            enemy_damage_flash = 8;
                            trigger_screen_shake(8.0f);
                            add_shockwave(410, 150, 70.0f, RGB(255, 140, 0));
                            spawn_particles_ext(410, 150, RGB(255,140,0), 20, 0);
                            spawn_particles_ext(410, 150, RGB(255,50,0), 12, 1);
                        } else if (element == 4) { // Water
                            int heal = 30 + loyalty;
                            bat_player_hp += heal;
                            if (bat_player_hp > bat_player_max) bat_player_hp = bat_player_max;
                            char m[128]; sprintf(m, "You used Healing Stream! Restored %d HP.", heal); add_log(m);
                            add_shockwave(180, 150, 60.0f, RGB(0, 220, 255));
                            spawn_particles_ext(180, 150, RGB(0,255,255), 20, 0);
                            spawn_particles_ext(180, 150, RGB(150,220,255), 10, 1);
                        } else if (element == 3) { // Earth
                            int dmg = strength * 2;
                            if (dmg < 1) dmg = 1;
                            if (e_def) dmg /= 2;
                            bat_enemy_hp -= dmg;
                            bat_enemy_spd -= 5;
                            if (bat_enemy_spd < 1) bat_enemy_spd = 1;
                            char m[128]; sprintf(m, "You used Earthquake! Dealt %d damage and slowed enemy.", dmg); add_log(m);
                            player_attack_offset = 24;
                            enemy_damage_flash = 8;
                            trigger_screen_shake(9.0f);
                            add_shockwave(410, 150, 80.0f, RGB(140, 90, 40));
                            spawn_particles_ext(410, 150, RGB(140,90,40), 20, 2);
                        }
                    } else if (action == 1) {
                        add_log("You are defending.");
                        add_shockwave(180, 150, 40.0f, RGB(255, 215, 0));
                    }
                    
                    if (bat_enemy_hp <= 0) {
                        add_log("Enemy dragon defeated!");
                        battle_ended = 1;
                        won = 1;
                        trigger_screen_shake(10.0f);
                        add_shockwave(410, 150, 100.0f, RGB(255, 215, 0));
                        spawn_particles_ext(410, 150, RGB(255,215,0), 30, 3);
                    } else {
                        // Enemy
                        if (!e_def) {
                            int hitChance = 80 + (bat_enemy_spd - speed) * 5;
                            if ((rand() % 100) < hitChance) {
                                int dmg = bat_enemy_str * 2 + (rand() % 5);
                                if (dmg < 1) dmg = 1;
                                if (p_def) dmg /= 2;
                                bat_player_hp -= dmg;
                                char m[128]; sprintf(m, "Enemy hit you for %d damage! (HP: %d/%d)", dmg, bat_player_hp, bat_player_max); add_log(m);
                                Beep(800, 50); Beep(100, 50);
                                enemy_attack_offset = 24;
                                player_damage_flash = 6;
                                trigger_screen_shake(6.0f);
                                add_shockwave(180, 150, 50.0f, RGB(255, 50, 50));
                                spawn_particles_ext(180, 150, RGB(255,0,0), 16, 0);
                            } else {
                                add_log("Enemy missed!");
                            }
                        } else {
                            add_log("Enemy is defending.");
                        }
                        
                        if (bat_player_hp <= 0) {
                            add_log("Your dragon was defeated and fled!");
                            battle_ended = 1;
                            won = 0;
                            trigger_screen_shake(8.0f);
                        }
                    }
                }
                
                if (battle_ended) {
                    if (won) {
                        int g = 20 + (rand() % 20);
                        gold += g;
                        happiness += 10; if (happiness > 100) happiness = 100;
                        char m[128]; sprintf(m, "You won the battle and earned %d gold!", g); add_log(m);
                    } else {
                        happiness -= 15; if (happiness < 0) happiness = 0;
                    }
                    ShowWindow(btn_bat_atk, SW_HIDE);
                    ShowWindow(btn_bat_def, SW_HIDE);
                    ShowWindow(btn_bat_spec, SW_HIDE);
                    ShowWindow(btn_bat_flee, SW_HIDE);
                    
                    ShowWindow(btn_feed, SW_SHOW); ShowWindow(btn_play, SW_SHOW);
                    ShowWindow(btn_sleep, SW_SHOW); ShowWindow(btn_train, SW_SHOW);
                    ShowWindow(btn_hoard, SW_SHOW); ShowWindow(btn_battle, SW_SHOW);
                    ShowWindow(btn_shop, SW_SHOW); ShowWindow(btn_help, SW_SHOW);
                    
                    state = prev_state;
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_SHOP) {
                if (state == 0) return 0;
                ShowWindow(btn_feed, SW_HIDE); ShowWindow(btn_play, SW_HIDE);
                ShowWindow(btn_sleep, SW_HIDE); ShowWindow(btn_train, SW_HIDE);
                ShowWindow(btn_hoard, SW_HIDE); ShowWindow(btn_battle, SW_HIDE);
                ShowWindow(btn_shop, SW_HIDE); ShowWindow(btn_help, SW_HIDE);
                
                ShowWindow(btn_shp_food, SW_SHOW);
                ShowWindow(btn_shp_toy, SW_SHOW);
                ShowWindow(btn_shp_str, SW_SHOW);
                ShowWindow(btn_shp_spd, SW_SHOW);
                ShowWindow(btn_shp_back, SW_SHOW);
                
                prev_state = state;
                state = 9;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_SHP_BACK) {
                ShowWindow(btn_shp_food, SW_HIDE); ShowWindow(btn_shp_toy, SW_HIDE);
                ShowWindow(btn_shp_str, SW_HIDE); ShowWindow(btn_shp_spd, SW_HIDE);
                ShowWindow(btn_shp_back, SW_HIDE);
                
                ShowWindow(btn_feed, SW_SHOW); ShowWindow(btn_play, SW_SHOW);
                ShowWindow(btn_sleep, SW_SHOW); ShowWindow(btn_train, SW_SHOW);
                ShowWindow(btn_hoard, SW_SHOW); ShowWindow(btn_battle, SW_SHOW);
                ShowWindow(btn_shop, SW_SHOW); ShowWindow(btn_help, SW_SHOW);
                
                state = prev_state;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_SHP_FOOD) {
                if (gold >= 20) {
                    gold -= 20;
                    hunger += 50; if (hunger > 100) hunger = 100;
                    energy += 20; if (energy > 100) energy = 100;
                    add_log("Bought Premium Meat! Hunger +50, Energy +20.");
                    Beep(400, 50); Beep(600, 50);
                    spawn_particles_ext(75, 230, RGB(255,100,100), 16, 0);
                } else { add_log("Not enough gold."); }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_SHP_TOY) {
                if (gold >= 30) {
                    gold -= 30;
                    happiness += 50; if (happiness > 100) happiness = 100;
                    energy += 10; if (energy > 100) energy = 100;
                    add_log("Bought Mystery Toy! Happiness +50, Energy +10.");
                    spawn_particles_ext(165, 230, RGB(50,150,255), 16, 3);
                } else { add_log("Not enough gold."); }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_SHP_STR) {
                if (gold >= 50) {
                    gold -= 50;
                    strength += 5;
                    add_log("Bought Power Bracer! Strength +5.");
                    spawn_particles_ext(255, 230, RGB(255,215,0), 20, 3);
                } else { add_log("Not enough gold."); }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_SHP_SPD) {
                if (gold >= 50) {
                    gold -= 50;
                    speed += 5;
                    add_log("Bought Swift Boots! Speed +5.");
                    spawn_particles_ext(345, 230, RGB(100,255,200), 20, 3);
                } else { add_log("Not enough gold."); }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_EVT_OPT1 || LOWORD(wParam) == BTN_EVT_OPT2) {
                int opt = LOWORD(wParam) == BTN_EVT_OPT1 ? 1 : 2;
                if (current_event_id == 0) {
                    if (opt == 1) {
                        if (gold >= 20) {
                            gold -= 20; happiness += 10; if (happiness>100) happiness=100;
                            add_log("Medicine worked! Dragon is feeling better.");
                            spawn_particles_ext(300, 150, RGB(100,255,100), 20, 0);
                        } else {
                            happiness -= 20; if (happiness<0) happiness=0;
                            add_log("Not enough gold for medicine... Dragon suffered.");
                        }
                    } else {
                        happiness -= 10; if (happiness<0) happiness=0;
                        strength--; if (strength<0) strength=0;
                        add_log("Dragon rested but lost some strength.");
                    }
                } else if (current_event_id == 1) {
                    if (opt == 1) {
                        if (strength > 10) {
                            happiness += 10; if (happiness>100) happiness=100;
                            add_log("Your dragon roared and scared it away!");
                            Beep(150, 100); Beep(100, 200); Beep(80, 200);
                            trigger_screen_shake(7.0f);
                            add_shockwave(300, 150, 80.0f, RGB(255, 180, 50));
                            spawn_particles_ext(300, 150, RGB(255,180,50), 25, 0);
                        } else {
                            energy -= 20; if (energy<0) energy=0;
                            happiness -= 10; if (happiness<0) happiness=0;
                            add_log("The beast attacked! Dragon lost energy.");
                            trigger_screen_shake(5.0f);
                        }
                    } else {
                        happiness -= 5; if (happiness<0) happiness=0;
                        loyalty -= 2; if (loyalty<0) loyalty=0;
                        add_log("You both hid. The beast left, but your dragon looks disappointed.");
                    }
                } else if (current_event_id == 2) {
                    if (opt == 1) {
                        if (gold >= 10) {
                            gold -= 10; happiness += 5; if (happiness>100) happiness=100;
                            add_log("You bought the eggshell. It's... shiny.");
                            spawn_particles_ext(300, 150, RGB(255,215,0), 15, 3);
                        } else {
                            add_log("Not enough gold.");
                        }
                    } else {
                        add_log("You ignored the merchant.");
                    }
                }
                
                ShowWindow(btn_evt_opt1, SW_HIDE);
                ShowWindow(btn_evt_opt2, SW_HIDE);
                ShowWindow(btn_feed, SW_SHOW); ShowWindow(btn_play, SW_SHOW);
                ShowWindow(btn_sleep, SW_SHOW); ShowWindow(btn_train, SW_SHOW);
                ShowWindow(btn_hoard, SW_SHOW); ShowWindow(btn_battle, SW_SHOW);
                ShowWindow(btn_shop, SW_SHOW); ShowWindow(btn_help, SW_SHOW);
                state = prev_state;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (LOWORD(wParam) == BTN_HELP) {
                MessageBox(hwnd, 
                    "Dragon Master's Guide\n\n"
                    "How to Play:\n"
                    "Raise your dragon from an egg. Keep it fed, happy, and well-rested.\n"
                    "As it grows, it will evolve into a mighty adult with a specific\n"
                    "elemental affinity based on how you raised it.\n\n"
                    "Stats Explanation:\n"
                    "- Hunger/Energy/Happiness: Core needs. Don't let them reach 0!\n"
                    "- Strength (Str): Increases attack power in battles and minigames.\n"
                    "- Speed (Spd): Increases hit chance and dodge rate.\n"
                    "- Loyalty (Loy): Affects special abilities like Healing Stream.\n"
                    "- Age: At age 10, baby dragons evolve.\n\n"
                    "Evolution:\n"
                    "- Mostly Fed: Earth Dragon\n"
                    "- Mostly Played: Fire Dragon\n"
                    "- Mostly Slept: Water Dragon\n\n"
                    "Item Codex:\n"
                    "- Meat: Restores 50 Hunger, 20 Energy.\n"
                    "- Toy: Restores 50 Happiness, 10 Energy.\n"
                    "- Bracer: Permanently grants +5 Strength.\n"
                    "- Boots: Permanently grants +5 Speed.", 
                    "Dragon Master's Guide", MB_OK | MB_ICONINFORMATION);
            }
            break;

        case WM_KEYDOWN:
            if (wParam == VK_F1 || wParam == 'H') {
                SendMessage(hwnd, WM_COMMAND, BTN_HELP, 0);
            }
            break;

        case WM_TIMER:
            if (wParam == TIMER_ID && state != 0) {
                hunger = hunger - 2;
                if (hunger < 0) hunger = 0;
                happiness = happiness - 1;
                if (happiness < 0) happiness = 0;
                age++;
                
                if (hunger < 20) add_log("Dragon is getting hungry...");
                if (happiness < 20) add_log("Dragon is feeling sad...");
                
                if (state == 1 || state == 2) {
                    if ((rand() % 100) < 10) {
                        current_event_id = rand() % 3;
                        ShowWindow(btn_feed, SW_HIDE); ShowWindow(btn_play, SW_HIDE);
                        ShowWindow(btn_sleep, SW_HIDE); ShowWindow(btn_train, SW_HIDE);
                        ShowWindow(btn_hoard, SW_HIDE); ShowWindow(btn_battle, SW_HIDE);
                        ShowWindow(btn_shop, SW_HIDE); ShowWindow(btn_help, SW_HIDE);
                        ShowWindow(btn_evt_opt1, SW_SHOW);
                        ShowWindow(btn_evt_opt2, SW_SHOW);
                        
                        if (current_event_id == 0) {
                            SetWindowText(btn_evt_opt1, "Medicine 20g");
                            SetWindowText(btn_evt_opt2, "Rest");
                        } else if (current_event_id == 1) {
                            SetWindowText(btn_evt_opt1, "Scare Beast");
                            SetWindowText(btn_evt_opt2, "Hide");
                        } else if (current_event_id == 2) {
                            SetWindowText(btn_evt_opt1, "Buy Shell 10g");
                            SetWindowText(btn_evt_opt2, "Ignore");
                        }
                        
                        prev_state = state;
                        state = 10;
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }
                
                if (state == 1 && age >= 10) {
                    state = 2;
                    int max_c = feed_count;
                    if (play_count > max_c) max_c = play_count;
                    if (sleep_count > max_c) max_c = sleep_count;
                    
                    char msg[128];
                    COLORREF evoColor = RGB(255, 215, 0);
                    if (max_c == feed_count) {
                        element = 3;
                        strcpy(msg, "Your baby dragon evolved into an ADULT EARTH DRAGON!");
                        evoColor = RGB(150, 100, 60);
                    } else if (max_c == play_count) {
                        element = 2;
                        strcpy(msg, "Your baby dragon evolved into an ADULT FIRE DRAGON!");
                        evoColor = RGB(255, 100, 30);
                    } else {
                        element = 4;
                        strcpy(msg, "Your baby dragon evolved into an ADULT WATER DRAGON!");
                        evoColor = RGB(50, 150, 255);
                    }
                    add_log(msg);
                    trigger_screen_shake(8.0f);
                    add_shockwave(300, 150, 100.0f, evoColor);
                    spawn_particles_ext(300, 150, evoColor, 35, 0);
                    spawn_particles_ext(300, 150, RGB(255,215,0), 20, 3);
                }
                
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (wParam == 2) { // str minigame
                if (minigame_dir == 1) {
                    minigame_val += 5;
                    if (minigame_val >= 100) minigame_dir = -1;
                } else {
                    minigame_val -= 5;
                    if (minigame_val <= 0) minigame_dir = 1;
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (wParam == 3) { // spd minigame
                if (minigame_state == 0 && GetTickCount() >= minigame_start_time) {
                    minigame_state = 1;
                    minigame_start_time = GetTickCount();
                    add_shockwave(300, 245, 60.0f, RGB(255, 255, 100));
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            else if (wParam == 4) { // expedition timer
                KillTimer(hwnd, 4);
                int found_gold = 10 + (GetTickCount() % 20);
                gold += found_gold;
                happiness += 5; if (happiness > 100) happiness = 100;
                char logMsg[128];
                sprintf(logMsg, "Dragon returned with %d gold!", found_gold);
                add_log(logMsg);
                trigger_screen_shake(4.0f);
                spawn_particles_ext(300, 150, RGB(255,215,0), 25, 3);
                
                if (GetTickCount() % 100 < 30) {
                    const char* items[] = {"Shiny Scale", "Gemstone", "Old Bone", "Mystery Eggshell"};
                    const char* item = items[GetTickCount() % 4];
                    sprintf(logMsg, "Dragon also found a rare item: %s!", item);
                    add_log(logMsg);
                }
                
                state = prev_state;
                ShowWindow(btn_feed, SW_SHOW); ShowWindow(btn_play, SW_SHOW);
                ShowWindow(btn_sleep, SW_SHOW); ShowWindow(btn_train, SW_SHOW);
                ShowWindow(btn_hoard, SW_SHOW); ShowWindow(btn_battle, SW_SHOW);
                ShowWindow(btn_shop, SW_SHOW); ShowWindow(btn_help, SW_SHOW);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else if (wParam == 5) { // 30 FPS continuous animation timer
                anim_tick++;
                
                // Screen shake physics decay
                if (screen_shake_amp > 0.05f) {
                    screen_shake_amp *= 0.88f;
                    screen_shake_angle += 1.3f;
                } else {
                    screen_shake_amp = 0.0f;
                }
                
                if (player_attack_offset > 0) player_attack_offset -= 3;
                if (enemy_attack_offset > 0) enemy_attack_offset -= 3;
                if (player_damage_flash > 0) player_damage_flash--;
                if (enemy_damage_flash > 0) enemy_damage_flash--;
                
                // Update kinematic particles
                for (int i = 0; i < MAX_PARTICLES; i++) {
                    if (particles[i].life > 0) {
                        particles[i].x += particles[i].vx;
                        particles[i].y += particles[i].vy;
                        
                        if (particles[i].type == 0) { // Spark: drag
                            particles[i].vx *= 0.94f;
                            particles[i].vy *= 0.94f;
                        } else if (particles[i].type == 1) { // Smoke: buoyant float & expansion
                            particles[i].vy -= 0.05f;
                            particles[i].size += 0.15f;
                        } else if (particles[i].type == 2) { // Debris: gravity
                            particles[i].vy += 0.25f;
                        } else if (particles[i].type == 3) { // Star: gentle float & sway
                            particles[i].vx += sinf((float)anim_tick * 0.2f + i) * 0.1f;
                            particles[i].vy *= 0.95f;
                        }
                        
                        particles[i].life--;
                    }
                }
                
                // Update shockwaves
                for (int i = 0; i < MAX_SHOCKWAVES; i++) {
                    if (shockwaves[i].active) {
                        shockwaves[i].radius += shockwaves[i].speed;
                        if (shockwaves[i].radius >= shockwaves[i].max_radius) {
                            shockwaves[i].active = 0;
                        }
                    }
                }
                
                // Update ambient motes
                for (int i = 0; i < MAX_MOTES; i++) {
                    ambient_motes[i].x += ambient_motes[i].vx;
                    ambient_motes[i].y += ambient_motes[i].vy;
                    if (ambient_motes[i].y < 10) {
                        ambient_motes[i].y = 470;
                        ambient_motes[i].x = (float)(rand() % 580 + 10);
                    }
                    if (ambient_motes[i].x < 10) ambient_motes[i].x = 590;
                    if (ambient_motes[i].x > 590) ambient_motes[i].x = 10;
                }
                
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdcPaint = BeginPaint(hwnd, &ps);
            RECT rect; GetClientRect(hwnd, &rect);
            HDC hdc = CreateCompatibleDC(hdcPaint);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdcPaint, rect.right, rect.bottom);
            HGDIOBJ hOld = SelectObject(hdc, hbmMem);
            
            // Background fill
            FillRect(hdc, &rect, bgBrush);
            
            // Procedural screen shake calculation
            int shake_dx = 0;
            int shake_dy = 0;
            if (screen_shake_amp > 0.05f) {
                shake_dx = (int)(cosf(screen_shake_angle) * screen_shake_amp);
                shake_dy = (int)(sinf(screen_shake_angle * 1.3f) * screen_shake_amp);
            }
            
            // Draw ambient floating magic motes
            for (int i = 0; i < MAX_MOTES; i++) {
                COLORREF mc = RGB(245, 220, 160);
                if (element == 2) mc = RGB(255, 140, 50); // Fire ember
                else if (element == 4) mc = RGB(100, 200, 255); // Water bubble
                else if (element == 3) mc = RGB(160, 220, 100); // Earth spore
                
                HBRUSH mb = CreateSolidBrush(mc);
                int mx = (int)ambient_motes[i].x;
                int my = (int)ambient_motes[i].y;
                int ms = (int)ambient_motes[i].size;
                RECT mr = {mx - ms/2, my - ms/2, mx + ms/2 + 1, my + ms/2 + 1};
                FillRect(hdc, &mr, mb);
                DeleteObject(mb);
            }
            
            // Ornate playfield frame
            RECT playfieldRect = {12 + shake_dx, 8 + shake_dy, 588 + shake_dx, 485 + shake_dy};
            DrawOrnateFrame(hdc, playfieldRect);
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(42, 23, 4));
            
            // Title Header
            SelectObject(hdc, hFontTitle);
            const char* titleText = "KDragon - Dragon Sanctuary";
            RECT titleR = {20 + shake_dx, 14 + shake_dy, 580 + shake_dx, 38 + shake_dy};
            DrawText(hdc, titleText, strlen(titleText), &titleR, DT_CENTER | DT_TOP);
            
            SelectObject(hdc, hFontNormal);
            
            if (state == 0) {
                const char* msg = "A mysterious mystical dragon egg awaits incubation...";
                RECT msgR = {20 + shake_dx, 45 + shake_dy, 580 + shake_dx, 70 + shake_dy};
                DrawText(hdc, msg, strlen(msg), &msgR, DT_CENTER | DT_TOP);
                
                // Gentle bobbing egg with aura
                int egg_bob = (int)(sinf((float)anim_tick * 0.1f) * 4.0f);
                
                // Golden egg glow
                HBRUSH glowB = CreateSolidBrush(RGB(240, 210, 150));
                Ellipse(hdc, 220 + shake_dx, 90 + egg_bob + shake_dy, 380 + shake_dx, 250 + egg_bob + shake_dy);
                DeleteObject(glowB);
                
                DrawPixelArt(hdc, 236 + shake_dx, 100 + egg_bob + shake_dy, 8, egg_pixels, 0, 0, 0);
            } else {
                const char* helpMsg = "[F1/H: Guide]";
                TextOut(hdc, 24 + shake_dx, 20 + shake_dy, helpMsg, strlen(helpMsg));
                
                char buf1[128];
                char buf2[128];
                const char* type_str = "None";
                if (element == 2) type_str = "Fire";
                else if (element == 3) type_str = "Earth";
                else if (element == 4) type_str = "Water";
                
                sprintf(buf1, "Hunger: %d/100  |  Happiness: %d/100  |  Energy: %d/100  |  Age: %d", 
                        hunger, happiness, energy, age);
                sprintf(buf2, "Type: %s  |  Str: %d  |  Spd: %d  |  Loy: %d  |  Gold: %d g", 
                        type_str, strength, speed, loyalty, gold);
                
                RECT r1 = {20 + shake_dx, 42 + shake_dy, 580 + shake_dx, 60 + shake_dy};
                RECT r2 = {20 + shake_dx, 60 + shake_dy, 580 + shake_dx, 80 + shake_dy};
                DrawText(hdc, buf1, strlen(buf1), &r1, DT_CENTER | DT_TOP);
                DrawText(hdc, buf2, strlen(buf2), &r2, DT_CENTER | DT_TOP);
                
                // Dragon idle breathing sinusoids
                int anim_bob = (int)(sinf((float)anim_tick * 0.15f) * 5.0f);
                
                int player_x = (state == 8) ? 110 : 236;
                int p_dx = shake_dx + player_attack_offset;
                int e_dx = shake_dx - enemy_attack_offset;

                // Player Dragon Sprite
                if (state == 1 || (state > 2 && prev_state == 1)) {
                    DrawPixelArt(hdc, player_x + p_dx, 88 + anim_bob + shake_dy, 8, dragon_pixels, 0, 0, player_damage_flash);
                } else if (state == 2 || (state > 2 && prev_state == 2)) {
                    DrawPixelArt(hdc, player_x + p_dx, 88 + anim_bob + shake_dy, 8, adult_dragon_pixels, element, 0, player_damage_flash);
                }
                
                if (state == 4) { // Strength minigame
                    const char* title = "Power Meter:";
                    TextOut(hdc, 170 + shake_dx, 235 + shake_dy, title, strlen(title));
                    HBRUSH barBrush = CreateSolidBrush(minigame_val > 75 ? RGB(240, 50, 50) : (minigame_val > 40 ? RGB(220, 180, 40) : RGB(80, 180, 60)));
                    HBRUSH bgBarBrush = CreateSolidBrush(RGB(255, 255, 255));
                    HPEN borderP = CreatePen(PS_SOLID, 2, RGB(90, 56, 20));
                    
                    SelectObject(hdc, borderP);
                    RECT barBg = {270 + shake_dx, 232 + shake_dy, 470 + shake_dx, 254 + shake_dy};
                    Rectangle(hdc, barBg.left, barBg.top, barBg.right, barBg.bottom);
                    FillRect(hdc, &barBg, bgBarBrush);
                    RECT barFg = {271 + shake_dx, 233 + shake_dy, 271 + minigame_val * 2 + shake_dx, 253 + shake_dy};
                    FillRect(hdc, &barFg, barBrush);
                    
                    DeleteObject(barBrush);
                    DeleteObject(bgBarBrush);
                    DeleteObject(borderP);
                } else if (state == 5) { // Speed minigame
                    const char* text = minigame_state == 0 ? "Prepare yourself... Wait for signal..." : ">>> STRIKE NOW! <<<";
                    SetTextColor(hdc, minigame_state == 0 ? RGB(110, 80, 50) : RGB(220, 20, 20));
                    SelectObject(hdc, hFontTitle);
                    RECT spdR = {20 + shake_dx, 232 + shake_dy, 580 + shake_dx, 260 + shake_dy};
                    DrawText(hdc, text, strlen(text), &spdR, DT_CENTER | DT_TOP);
                    SetTextColor(hdc, RGB(42, 23, 4));
                    SelectObject(hdc, hFontNormal);
                } else if (state == 6) { // Loyalty minigame
                    const char* text = "Watch the cups closely! Find the hidden golden dragon treat:";
                    RECT loyR = {20 + shake_dx, 235 + shake_dy, 580 + shake_dx, 260 + shake_dy};
                    DrawText(hdc, text, strlen(text), &loyR, DT_CENTER | DT_TOP);
                } else if (state == 8) { // Battle mode
                    SelectObject(hdc, hFontTitle);
                    const char* text = "VS";
                    SetTextColor(hdc, RGB(200, 30, 30));
                    RECT vsR = {200 + shake_dx, 130 + shake_dy, 400 + shake_dx, 160 + shake_dy};
                    DrawText(hdc, text, strlen(text), &vsR, DT_CENTER | DT_TOP);
                    SetTextColor(hdc, RGB(42, 23, 4));
                    SelectObject(hdc, hFontNormal);
                    
                    // HP bars
                    // Player HP
                    HBRUSH phpB = CreateSolidBrush(RGB(40, 180, 40));
                    RECT phpR = {100 + shake_dx, 220 + shake_dy, 100 + (bat_player_hp * 120 / bat_player_max) + shake_dx, 230 + shake_dy};
                    RECT phpBg = {100 + shake_dx, 220 + shake_dy, 220 + shake_dx, 230 + shake_dy};
                    HBRUSH hpBgB = CreateSolidBrush(RGB(80, 80, 80));
                    FillRect(hdc, &phpBg, hpBgB);
                    FillRect(hdc, &phpR, phpB);
                    DeleteObject(phpB);
                    
                    // Enemy HP
                    HBRUSH ehpB = CreateSolidBrush(RGB(200, 40, 40));
                    RECT ehpR = {370 + shake_dx, 220 + shake_dy, 370 + (bat_enemy_hp * 120 / bat_enemy_max) + shake_dx, 230 + shake_dy};
                    RECT ehpBg = {370 + shake_dx, 220 + shake_dy, 490 + shake_dx, 230 + shake_dy};
                    FillRect(hdc, &ehpBg, hpBgB);
                    FillRect(hdc, &ehpR, ehpB);
                    DeleteObject(ehpB);
                    DeleteObject(hpBgB);
                    
                    // Enemy Obsidian Dragon Sprite (Flipped)
                    DrawPixelArt(hdc, 360 + e_dx, 88 + anim_bob + shake_dy, 8, adult_dragon_pixels, 5, 1, enemy_damage_flash);
                } else if (state == 9) { // Shop mode
                    DrawShopItemCard(hdc, 32 + shake_dx, 215 + shake_dy, 0, 0);
                    DrawShopItemCard(hdc, 122 + shake_dx, 215 + shake_dy, 1, 0);
                    DrawShopItemCard(hdc, 212 + shake_dx, 215 + shake_dy, 2, 0);
                    DrawShopItemCard(hdc, 302 + shake_dx, 215 + shake_dy, 3, 0);
                } else if (state == 10) { // Random event
                    const char* text = "";
                    if (current_event_id == 0) text = "Your dragon looks sick and feverish! What will you do?";
                    else if (current_event_id == 1) text = "A wild predator beast is approaching the lair!";
                    else if (current_event_id == 2) text = "A wandering mystic merchant offers a legendary eggshell!";
                    SetTextColor(hdc, RGB(180, 40, 20));
                    RECT evtR = {20 + shake_dx, 235 + shake_dy, 580 + shake_dx, 260 + shake_dy};
                    DrawText(hdc, text, strlen(text), &evtR, DT_CENTER | DT_TOP);
                    SetTextColor(hdc, RGB(42, 23, 4));
                }
                
                // Draw log box with filigree corners
                HBRUSH logBrush = CreateSolidBrush(RGB(235, 210, 170));
                RECT logRect = {24 + shake_dx, 315 + shake_dy, 576 + shake_dx, 415 + shake_dy};
                FillRect(hdc, &logRect, logBrush);
                DeleteObject(logBrush);
                
                DrawOrnateFrame(hdc, logRect);
                
                SelectObject(hdc, hFontSmall);
                SetTextColor(hdc, RGB(42, 23, 4));
                for (int i = 0; i < log_count; i++) {
                    char logStr[150];
                    sprintf(logStr, " %s", log_messages[i]);
                    TextOut(hdc, 32 + shake_dx, 325 + i * 21 + shake_dy, logStr, strlen(logStr));
                }
            }
            
            // Draw active shockwaves
            for (int i = 0; i < MAX_SHOCKWAVES; i++) {
                if (shockwaves[i].active) {
                    HPEN wavePen = CreatePen(PS_SOLID, 2, shockwaves[i].color);
                    HGDIOBJ oldWavePen = SelectObject(hdc, wavePen);
                    HGDIOBJ oldWaveBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    
                    int sx = (int)shockwaves[i].x + shake_dx;
                    int sy = (int)shockwaves[i].y + shake_dy;
                    int sr = (int)shockwaves[i].radius;
                    
                    Ellipse(hdc, sx - sr, sy - sr, sx + sr, sy + sr);
                    
                    // Secondary chromatic ripple
                    if (sr > 10) {
                        Ellipse(hdc, sx - (sr - 6), sy - (sr - 6), sx + (sr - 6), sy + (sr - 6));
                    }
                    
                    SelectObject(hdc, oldWavePen);
                    SelectObject(hdc, oldWaveBrush);
                    DeleteObject(wavePen);
                }
            }
            
            // Draw active kinematic particles
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (particles[i].life > 0) {
                    HBRUSH b = CreateSolidBrush(particles[i].color);
                    int px = (int)particles[i].x + shake_dx;
                    int py = (int)particles[i].y + shake_dy;
                    int psz = (int)particles[i].size;
                    
                    if (particles[i].type == 0) { // Needle spark
                        RECT r = {px, py, px + psz, py + psz};
                        FillRect(hdc, &r, b);
                    } else if (particles[i].type == 1) { // Expanding smoke puff
                        SelectObject(hdc, b);
                        Ellipse(hdc, px - psz, py - psz, px + psz, py + psz);
                    } else if (particles[i].type == 2) { // Shard / Scale
                        POINT pts[3] = { {px, py - psz}, {px + psz, py + psz}, {px - psz, py + psz} };
                        SelectObject(hdc, b);
                        Polygon(hdc, pts, 3);
                    } else if (particles[i].type == 3) { // Golden Star
                        POINT pts[4] = { {px, py - psz - 2}, {px + psz + 2, py}, {px, py + psz + 2}, {px - psz - 2, py} };
                        SelectObject(hdc, b);
                        Polygon(hdc, pts, 4);
                    }
                    DeleteObject(b);
                }
            }
            
            // Double-buffer BitBlt
            BitBlt(hdcPaint, 0, 0, rect.right, rect.bottom, hdc, 0, 0, SRCCOPY);
            SelectObject(hdc, hOld);
            DeleteObject(hbmMem);
            DeleteDC(hdc);
            EndPaint(hwnd, &ps);
            break;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_CTLCOLORBTN: {
            HDC hdcBtn = (HDC)wParam;
            SetBkColor(hdcBtn, RGB(139, 90, 43));
            SetTextColor(hdcBtn, RGB(42, 23, 4));
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        }

        case WM_DESTROY:
            KillTimer(hwnd, TIMER_ID);
            KillTimer(hwnd, 5);
            DeleteObject(hFontNormal);
            DeleteObject(hFontSmall);
            DeleteObject(hFontTitle);
            DeleteObject(hFontLarge);
            DeleteObject(bgBrush);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    SetProcessDPIAware();
    const char CLASS_NAME[]  = "KDragonClass";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClass(&wc);
    
    DWORD style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX | WS_CLIPCHILDREN;
    RECT rect = {0, 0, 600, 500};
    AdjustWindowRect(&rect, style, FALSE);
    
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KDragon",
        style,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, hInstance, NULL
    );
    
    if (hwnd == NULL) {
        return 0;
    }
    
    ShowWindow(hwnd, nCmdShow);
    
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
}
