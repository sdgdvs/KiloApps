#include <windows.h>
#pragma comment(lib, "msimg32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int screen_shake = 0;
float boss_whiteout = 0.0f;

#define MAX_PARTICLES 400
typedef struct {
    float x, y;
    float vx, vy;
    float life, decay;
    COLORREF color;
} Particle;
Particle particles[MAX_PARTICLES];

void SpawnParticle(float x, float y, float vx, float vy, float decay, COLORREF color) {
    for (int i=0; i<MAX_PARTICLES; i++) {
        if (particles[i].life <= 0) {
            particles[i].x = x; particles[i].y = y;
            particles[i].vx = vx; particles[i].vy = vy;
            particles[i].life = 1.0f; particles[i].decay = decay;
            particles[i].color = color;
            break;
        }
    }
}

void SpawnExplosion(float x, float y, int count, COLORREF color1, COLORREF color2) {
    int intensity = count / 5;
    if (intensity > screen_shake) screen_shake = intensity;

    for (int i=0; i<count; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = (rand() % 50) / 10.0f + (count > 40 ? 2.0f : 0.0f);
        float vx = cos(angle) * speed;
        float vy = sin(angle) * speed;
        float decay = 0.01f + (rand() % 40) / 1000.0f;
        COLORREF c = (rand() % 2 == 0) ? color1 : color2;
        SpawnParticle(x, y, vx, vy, decay, c);
    }
    // Core layer
    for (int i=0; i<count/3; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = (rand() % 20) / 10.0f;
        float vx = cos(angle) * speed;
        float vy = sin(angle) * speed;
        float decay = 0.03f + (rand() % 50) / 1000.0f;
        SpawnParticle(x, y, vx, vy, decay, RGB(255, 255, 255));
    }
}

void SpawnBeamEffect(float x, float y, COLORREF color) {
    for (int i=0; i<60; i++) {
        float vx = (rand() % 200 - 100) / 30.0f;
        float vy = -8.0f - (rand() % 100) / 10.0f;
        float decay = 0.02f + (rand() % 30) / 1000.0f;
        SpawnParticle(x + (rand() % 20 - 10), y, vx, vy, decay, color);
    }
}

#define NUM_SYSTEMS 220
#define MAP_SIZE 2000
typedef struct {
    int x;
    int y;
    int size;
    COLORREF color;
    int type_idx;
    int num_planets;
    int planets[6];
    int encounter_type;
    int visited;
    int faction;
} StarSystem;

const char* faction_names[] = {"Independent", "Federation", "Syndicate", "Xenon"};
int faction_rep[4] = {50, 50, 30, 10}; // 0: Ind, 1: Fed, 2: Syn, 3: Xen

StarSystem systems[NUM_SYSTEMS];

const char* star_names[] = {"Red Dwarf", "Yellow Dwarf", "Blue Giant", "White Dwarf"};
const char* planet_names[] = {"Terrestrial", "Gas Giant", "Ice World", "Lava", "Barren"};

int ship_x = 0;
int ship_y = 0;
int is_moving = 0;
float res_fuel = 10000.0f;
int res_hull = 100;
int res_credits = 1000;
int res_morale = 100;

// Superweapon system
int superweapon_type = 1; // 0=None, 1=Tachyon Beam, 2=Antimatter Torpedo, 3=Chrono Disruptor, 4=Nova Obliterator
int superweapon_charges = 3;
int superweapon_max = 5;
const char* superweapon_names[] = {"None", "Tachyon Beam", "Antimatter Torpedo", "Chrono Disruptor", "Nova Obliterator"};

typedef struct {
    char name[16];
    int role; // 0=Unassigned, 1=Pilot, 2=Gunner, 3=Engineer, 4=Security
    int level;
    int xp;
} CrewMember;
CrewMember roster[50];
int roster_count = 0;
const char* first_names[] = {"Ryker", "Spok", "Chek", "Scot", "Uhura", "Sulu", "Mac", "Data", "Worf", "Geordi", "Ripley", "Dallas", "Hicks", "Hudson", "Vasquez", "Gordy"};

int GetOfficer(int role) {
    for (int i = 0; i < roster_count; i++) {
        if (roster[i].role == role) return i;
    }
    return -1;
}

void CycleOfficer(int role) {
    int current = GetOfficer(role);
    int start = (current == -1) ? 0 : current + 1;
    for (int i = 0; i < roster_count; i++) {
        int idx = (start + i) % roster_count;
        if (roster[idx].role == 0) {
            if (current != -1) roster[current].role = 0;
            roster[idx].role = role;
            return;
        }
    }
}

void AddXP(int role, int amt) {
    int idx = GetOfficer(role);
    if (idx != -1) {
        if (roster[idx].level >= 5) return;
        roster[idx].xp += amt;
        if (roster[idx].xp >= roster[idx].level * 100) {
            roster[idx].xp -= roster[idx].level * 100;
            roster[idx].level++;
        }
    }
}

int cargo_minerals = 0;
int cargo_tech = 0;
int upg_weapons = 1;
int upg_shields = 1;
int upg_engines = 1;
int upg_cargo = 1;

int modal_open = 0;
int modal_enc_type = 0;
int pirate_hp = 50;
int enemy_max_hp = 50;
char combat_log[256] = "";

DWORD WINAPI SoundThread(LPVOID lpParam) {
    int type = (int)(intptr_t)lpParam;
    if (type == 1) { // Laser
        Beep(800, 40);
        Beep(400, 40);
    } else if (type == 2) { // Alarm
        Beep(400, 200);
        Beep(600, 200);
        Beep(400, 200);
        Beep(600, 200);
    } else if (type == 3) { // Superweapon
        Beep(300, 100);
        Beep(500, 100);
        Beep(900, 150);
        Beep(1200, 250);
    } else if (type == 4) { // Boarding Klaxon
        Beep(700, 120);
        Beep(350, 120);
        Beep(700, 120);
    }
    return 0;
}

void PlaySoundEffect(int type) {
    CreateThread(NULL, 0, SoundThread, (LPVOID)(intptr_t)type, 0, NULL);
}

DWORD WINAPI EngineHumThread(LPVOID lpParam) {
    while (1) {
        if (is_moving && !modal_open) {
            Beep(60, 100);
        } else {
            Sleep(50);
        }
    }
    return 0;
}

void TriggerEncounter(int type) {
    modal_open = 1;
    modal_enc_type = type;
    if (type == 1) {
        PlaySoundEffect(2); // Alarm
        pirate_hp = 50;
        enemy_max_hp = 50;
        lstrcpyA(combat_log, "Space pirates ambush you!");
    } else if (type == 13) {
        PlaySoundEffect(2); // Alarm
        pirate_hp = 160;
        enemy_max_hp = 160;
        lstrcpyA(combat_log, "A hostile dreadnought fleet intercepts you!");
    } else if (type == 16) {
        PlaySoundEffect(2);
        lstrcpyA(combat_log, "WARNING: Entered active Faction War combat zone!");
    } else if (type == 17) {
        PlaySoundEffect(4); // Boarding klaxon
        lstrcpyA(combat_log, "INTRUDER ALERT: Alien boarding party has breached deck C!");
    } else if (type == 3) {
        if (roster_count < 50) {
            lstrcpyA(roster[roster_count].name, first_names[rand() % 16]);
            roster[roster_count].role = 0;
            roster[roster_count].level = 1;
            roster[roster_count].xp = 0;
            roster_count++;
        }
    }
}

#define NUM_ASTEROIDS 150
typedef struct {
    int x, y;
    int size;
    POINT pts[4];
} Asteroid;
Asteroid asteroids[NUM_ASTEROIDS];

#define NUM_NEBULAS 15
typedef struct {
    int x, y;
    int radius;
    COLORREF color;
} Nebula;
Nebula nebulas[NUM_NEBULAS];

#define NUM_BGSTARS 200
typedef struct {
    float x, y;
    int size;
    COLORREF color;
} BgStar;
BgStar bgstars[NUM_BGSTARS];

void InitEnvironment() {
    for (int i = 0; i < NUM_BGSTARS; i++) {
        bgstars[i].x = (float)(rand() % 1600 - 800);
        bgstars[i].y = (float)(rand() % 1200 - 600);
        bgstars[i].size = 1 + rand() % 2;
        bgstars[i].color = (rand() % 10 > 8) ? RGB(170, 255, 255) : RGB(200, 200, 200);
    }
    for (int i = 0; i < NUM_ASTEROIDS; i++) {
        asteroids[i].x = (rand() % MAP_SIZE) - MAP_SIZE/2;
        asteroids[i].y = (rand() % MAP_SIZE) - MAP_SIZE/2;
        asteroids[i].size = 2 + rand() % 4;
        asteroids[i].pts[0].x = -asteroids[i].size; asteroids[i].pts[0].y = -asteroids[i].size;
        asteroids[i].pts[1].x = asteroids[i].size; asteroids[i].pts[1].y = -asteroids[i].size / 2;
        asteroids[i].pts[2].x = (int)(asteroids[i].size * 0.8f); asteroids[i].pts[2].y = asteroids[i].size;
        asteroids[i].pts[3].x = -asteroids[i].size / 2; asteroids[i].pts[3].y = (int)(asteroids[i].size * 0.8f);
    }
    for (int i = 0; i < NUM_NEBULAS; i++) {
        nebulas[i].x = (rand() % MAP_SIZE) - MAP_SIZE/2;
        nebulas[i].y = (rand() % MAP_SIZE) - MAP_SIZE/2;
        nebulas[i].radius = 200 + rand() % 300;
        int c = rand() % 3;
        if (c == 0) nebulas[i].color = RGB(15, 5, 20);
        else if (c == 1) nebulas[i].color = RGB(5, 15, 30);
        else nebulas[i].color = RGB(20, 5, 30);
    }
}

void InitStars() {
    for (int i = 0; i < NUM_SYSTEMS; i++) {
        systems[i].x = (rand() % MAP_SIZE) - MAP_SIZE/2;
        systems[i].y = (rand() % MAP_SIZE) - MAP_SIZE/2;
        
        int r = rand() % 100;
        if (r < 35) {
            systems[i].type_idx = 0; systems[i].color = RGB(255, 170, 170);
            systems[i].size = 2; systems[i].num_planets = 1 + rand() % 3;
        } else if (r < 70) {
            systems[i].type_idx = 1; systems[i].color = RGB(255, 255, 170);
            systems[i].size = 3; systems[i].num_planets = 2 + rand() % 5;
        } else if (r < 90) {
            systems[i].type_idx = 2; systems[i].color = RGB(170, 221, 255);
            systems[i].size = 4; systems[i].num_planets = rand() % 3;
        } else {
            systems[i].type_idx = 3; systems[i].color = RGB(255, 255, 255);
            systems[i].size = 1; systems[i].num_planets = rand() % 2;
        }

        for (int p = 0; p < systems[i].num_planets; p++) {
            systems[i].planets[p] = rand() % 5;
        }

        int enc = rand() % 12;
        if (enc == 0) systems[i].encounter_type = 1; // Pirates
        else if (enc == 1) systems[i].encounter_type = 2; // Anomaly
        else if (enc == 2) systems[i].encounter_type = 3; // Trader
        else if (enc == 3) systems[i].encounter_type = 4; // Station
        else if (enc == 4) systems[i].encounter_type = 7 + (rand() % 3); // Story
        else if (enc == 5) systems[i].encounter_type = 13; // Fleet Battle
        else if (enc == 6) systems[i].encounter_type = 14; // Diplomacy
        else if (enc == 7) systems[i].encounter_type = 16; // Faction War Zone
        else if (enc == 8) systems[i].encounter_type = 17; // Alien Boarding Party
        else if (enc == 9) systems[i].encounter_type = 18; // Superweapon Forge
        else systems[i].encounter_type = 0;
        
        systems[i].visited = 0;
        systems[i].faction = rand() % 4;
    }
}

void Update() {
    if (boss_whiteout > 0.0f) {
        boss_whiteout -= 0.02f;
    }
    if (screen_shake > 0) {
        screen_shake -= 2;
        if (screen_shake < 0) screen_shake = 0;
    }

    for (int i=0; i<MAX_PARTICLES; i++) {
        if (particles[i].life > 0) {
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            particles[i].life -= particles[i].decay;
        }
    }

    if (modal_open) return;

    is_moving = 0;
    int speed = 5 + (upg_engines - 1) * 2;
    int p_idx = GetOfficer(1);
    if (p_idx != -1) speed += roster[p_idx].level;
    int dx = 0, dy = 0;
    if (GetAsyncKeyState('W') & 0x8000) { dy -= speed; }
    if (GetAsyncKeyState('S') & 0x8000) { dy += speed; }
    if (GetAsyncKeyState('A') & 0x8000) { dx -= speed; }
    if (GetAsyncKeyState('D') & 0x8000) { dx += speed; }

    if ((dx != 0 || dy != 0) && res_fuel > 0) {
        ship_x += dx;
        ship_y += dy;
        is_moving = 1;
        res_fuel -= 1.0f;
        if (res_fuel < 0) res_fuel = 0;
        if (rand() % 10 == 0) AddXP(1, 1);
    }

    if (is_moving) {
        for (int i=0; i<5; i++) {
            float px = ship_x + (rand() % 9 - 4);
            float py = ship_y + 10 + (rand() % 8);
            float vx = (rand() % 100 - 50) / 100.0f;
            float vy = 3.0f + (rand() % 100) / 30.0f;
            float decay = 0.04f + (rand() % 100) / 2000.0f;
            COLORREF c = (rand() % 3 == 0) ? RGB(255, 255, 0) : ((rand() % 2 == 0) ? RGB(255, 136, 0) : RGB(255, 0, 0));
            SpawnParticle(px, py, vx, vy, decay, c);
        }
    }

    if (ship_x < -MAP_SIZE/2) ship_x = -MAP_SIZE/2;
    if (ship_x > MAP_SIZE/2) ship_x = MAP_SIZE/2;
    if (ship_y < -MAP_SIZE/2) ship_y = -MAP_SIZE/2;
    if (ship_y > MAP_SIZE/2) ship_y = MAP_SIZE/2;

    int found_sys_idx = -1;
    for (int i = 0; i < NUM_SYSTEMS; i++) {
        int dX = systems[i].x - ship_x;
        int dY = systems[i].y - ship_y;
        if (dX*dX + dY*dY < 2500) {
            found_sys_idx = i;
            break;
        }
    }
    
    if (found_sys_idx != -1) {
        if (!systems[found_sys_idx].visited && !modal_open) {
            systems[found_sys_idx].visited = 1;
            if (systems[found_sys_idx].encounter_type != 0) {
                TriggerEncounter(systems[found_sys_idx].encounter_type);
            }
        }
    }
}

void Draw(HDC hdc, RECT* rect) {
    int width = rect->right - rect->left;
    int height = rect->bottom - rect->top;
    
    int mapWidth = width - 210;
    if (mapWidth < 100) mapWidth = 100;

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
    SelectObject(memDC, memBitmap);

    HBRUSH bgBrush = CreateSolidBrush(RGB(5, 5, 10));
    RECT mapRect = {0, 0, mapWidth, height};
    FillRect(memDC, &mapRect, bgBrush);
    DeleteObject(bgBrush);

    int centerX = mapWidth / 2;
    int centerY = height / 2;

    // Distant background stars (parallax)
    for (int i = 0; i < NUM_BGSTARS; i++) {
        float sx = bgstars[i].x - (ship_x * 0.05f);
        float sy = bgstars[i].y - (ship_y * 0.05f);
        
        while (sx < -mapWidth) sx += mapWidth * 2;
        while (sx > mapWidth) sx -= mapWidth * 2;
        while (sy < -height) sy += height * 2;
        while (sy > height) sy -= height * 2;

        int drawX = centerX + (int)sx;
        int drawY = centerY + (int)sy;

        if (drawX >= 0 && drawX < mapWidth && drawY >= 0 && drawY < height) {
            if ((GetTickCount() + i * 100) % 2000 > 1000) {
                SetPixel(memDC, drawX, drawY, bgstars[i].color);
                if (bgstars[i].size > 1) {
                    SetPixel(memDC, drawX+1, drawY, bgstars[i].color);
                    SetPixel(memDC, drawX, drawY+1, bgstars[i].color);
                    SetPixel(memDC, drawX+1, drawY+1, bgstars[i].color);
                }
            } else {
                SetPixel(memDC, drawX, drawY, RGB(100, 100, 100));
            }
        }
    }

    HBRUSH sunBrush = CreateSolidBrush(RGB(15, 10, 5));
    SelectObject(memDC, sunBrush);
    SelectObject(memDC, GetStockObject(NULL_PEN));
    Ellipse(memDC, (int)(mapWidth*0.8) - 200, (int)(height*0.2) - 200, (int)(mapWidth*0.8) + 200, (int)(height*0.2) + 200);
    DeleteObject(sunBrush);

    SelectObject(memDC, GetStockObject(NULL_PEN));
    for (int i = 0; i < NUM_NEBULAS; i++) {
        int screenX = centerX + (int)((nebulas[i].x - ship_x) * 0.2f);
        int screenY = centerY + (int)((nebulas[i].y - ship_y) * 0.2f);
        int r = nebulas[i].radius;
        if (screenX >= -r && screenX <= mapWidth + r && screenY >= -r && screenY <= height + r) {
            HBRUSH nBrush = CreateSolidBrush(nebulas[i].color);
            SelectObject(memDC, nBrush);
            Ellipse(memDC, screenX - r, screenY - r, screenX + r, screenY + r);
            DeleteObject(nBrush);
        }
    }

    HBRUSH astBrush = CreateSolidBrush(RGB(80, 80, 80));
    SelectObject(memDC, astBrush);
    for (int i = 0; i < NUM_ASTEROIDS; i++) {
        int screenX = centerX + (int)((asteroids[i].x - ship_x) * 0.5f);
        int screenY = centerY + (int)((asteroids[i].y - ship_y) * 0.5f);
        if (screenX >= 0 && screenX <= mapWidth && screenY >= 0 && screenY <= height) {
            POINT pts[4];
            for (int p = 0; p < 4; p++) {
                pts[p].x = screenX + asteroids[i].pts[p].x;
                pts[p].y = screenY + asteroids[i].pts[p].y;
            }
            Polygon(memDC, pts, 4);
        }
    }
    DeleteObject(astBrush);

    HBRUSH uiBrush = CreateSolidBrush(RGB(5, 5, 20));
    RECT uiRect = {mapWidth, 0, width, height};
    FillRect(memDC, &uiRect, uiBrush);
    DeleteObject(uiBrush);

    // Neon borders
    HPEN neonBorderPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
    SelectObject(memDC, neonBorderPen);
    SelectObject(memDC, GetStockObject(NULL_BRUSH));
    Rectangle(memDC, 0, 0, mapWidth, height);
    Rectangle(memDC, mapWidth + 4, 4, width - 4, height - 4);
    DeleteObject(neonBorderPen);

    HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(0, 40, 40));
    SelectObject(memDC, gridPen);
    int gridSize = 100;
    int offsetX = -(ship_x % gridSize);
    int offsetY = -(ship_y % gridSize);
    if (offsetX > 0) offsetX -= gridSize;
    if (offsetY > 0) offsetY -= gridSize;

    for (int x = offsetX; x < mapWidth; x += gridSize) {
        MoveToEx(memDC, x, 0, NULL);
        LineTo(memDC, x, height);
    }
    for (int y = offsetY; y < height; y += gridSize) {
        MoveToEx(memDC, 0, y, NULL);
        LineTo(memDC, mapWidth, y);
    }
    DeleteObject(gridPen);

    for (int i = 0; i < NUM_SYSTEMS; i++) {
        int screenX = centerX + (systems[i].x - ship_x);
        int screenY = centerY + (systems[i].y - ship_y);

        if (screenX >= 0 && screenX <= mapWidth && screenY >= 0 && screenY <= height) {
            HBRUSH starBrush = CreateSolidBrush(systems[i].color);
            HPEN starPen = CreatePen(PS_SOLID, 1, systems[i].color);
            SelectObject(memDC, starBrush);
            SelectObject(memDC, starPen);
            int s = systems[i].size;
            Ellipse(memDC, screenX - s, screenY - s, screenX + s, screenY + s);
            DeleteObject(starBrush);
            DeleteObject(starPen);
        }
    }

    // Dynamic 3D drop-shadow
    int shadowOffX = -ship_x / 100;
    int shadowOffY = -ship_y / 100;
    POINT shadowPts[4] = {
        {centerX + shadowOffX, centerY - 10 + shadowOffY},
        {centerX + 8 + shadowOffX, centerY + 8 + shadowOffY},
        {centerX + shadowOffX, centerY + 4 + shadowOffY},
        {centerX - 8 + shadowOffX, centerY + 8 + shadowOffY}
    };
    HBRUSH shadowBrush = CreateSolidBrush(RGB(2, 2, 8));
    HPEN shadowPen = CreatePen(PS_SOLID, 1, RGB(2, 2, 8));
    SelectObject(memDC, shadowBrush);
    SelectObject(memDC, shadowPen);
    Polygon(memDC, shadowPts, 4);
    DeleteObject(shadowBrush);
    DeleteObject(shadowPen);

    // Ship Hull
    POINT shipPts[4] = {
        {centerX, centerY - 10},
        {centerX + 8, centerY + 8},
        {centerX, centerY + 4},
        {centerX - 8, centerY + 8}
    };
    HBRUSH shipBrush = CreateSolidBrush(RGB(0, 150, 150));
    HPEN shipPen = CreatePen(PS_SOLID, 1, RGB(0, 200, 200));
    SelectObject(memDC, shipBrush);
    SelectObject(memDC, shipPen);
    Polygon(memDC, shipPts, 4);
    DeleteObject(shipBrush);
    DeleteObject(shipPen);
    
    // Specular highlight
    POINT specPts[3] = {
        {centerX, centerY - 8},
        {centerX - 5, centerY + 6},
        {centerX, centerY + 2}
    };
    HBRUSH specBrush = CreateSolidBrush(RGB(200, 255, 255));
    HPEN specPen = CreatePen(PS_SOLID, 1, RGB(200, 255, 255));
    SelectObject(memDC, specBrush);
    SelectObject(memDC, specPen);
    Polygon(memDC, specPts, 3);
    DeleteObject(specBrush);
    DeleteObject(specPen);

    // Ship lights
    if ((GetTickCount() % 1000) < 500) {
        SetPixel(memDC, centerX - 5, centerY + 5, RGB(255, 0, 0));
        SetPixel(memDC, centerX + 5, centerY + 5, RGB(0, 255, 0));
    } else {
        SetPixel(memDC, centerX - 5, centerY + 5, RGB(100, 0, 0));
        SetPixel(memDC, centerX + 5, centerY + 5, RGB(0, 100, 0));
    }

    // Draw particles
    for (int i=0; i<MAX_PARTICLES; i++) {
        if (particles[i].life > 0) {
            int screenX = centerX + (int)(particles[i].x - ship_x);
            int screenY = centerY + (int)(particles[i].y - ship_y);
            if (screenX >= 0 && screenX <= mapWidth && screenY >= 0 && screenY <= height) {
                HPEN partPen = CreatePen(PS_SOLID, 2, particles[i].color);
                HPEN oldPen = SelectObject(memDC, partPen);
                MoveToEx(memDC, screenX - (int)(particles[i].vx * 2), screenY - (int)(particles[i].vy * 2), NULL);
                LineTo(memDC, screenX, screenY);
                SelectObject(memDC, oldPen);
                DeleteObject(partPen);
                SetPixel(memDC, screenX, screenY, RGB(255, 255, 255));
            }
        }
    }

    // Scanner pulse
    HPEN pulsePen = CreatePen(PS_SOLID, 2, RGB(0, 150, 150));
    SelectObject(memDC, pulsePen);
    SelectObject(memDC, GetStockObject(NULL_BRUSH));
    float pulse = (GetTickCount() % 2000) / 2000.0f;
    int r = 20 + (int)(pulse * 10);
    Ellipse(memDC, centerX - r, centerY - r, centerX + r, centerY + r);
    DeleteObject(pulsePen);

    if (is_moving) {
        int baseLength = 16 + (GetTickCount() % 4);
        for (int i = 0; i < 3; i++) {
            int flameLength = baseLength + i * 5;
            POINT thrustPts[4] = {
                {centerX, centerY + 6 + i*2},
                {centerX + 4 - i, centerY + 10 + i*2},
                {centerX, centerY + flameLength},
                {centerX - 4 + i, centerY + 10 + i*2}
            };
            COLORREF fColor = (i == 0) ? RGB(255, 255, 150) : ((i == 1) ? RGB(255, 136, 0) : RGB(200, 50, 0));
            HBRUSH thrustBrush = CreateSolidBrush(fColor);
            HPEN thrustPen = CreatePen(PS_SOLID, 1, fColor);
            SelectObject(memDC, thrustBrush);
            SelectObject(memDC, thrustPen);
            Polygon(memDC, thrustPts, 4);
            DeleteObject(thrustBrush);
            DeleteObject(thrustPen);
        }
    }

    SetBkMode(memDC, TRANSPARENT);
    char buf[128];
    HPEN linePen = CreatePen(PS_SOLID, 1, RGB(0, 85, 85));
    
    // UI Panel Drawing
    wsprintfA(buf, "SHIP STATUS");
    SetTextColor(memDC, RGB(0, 255, 255));
    TextOutA(memDC, mapWidth + 12, 12, buf, lstrlenA(buf));
    
    SelectObject(memDC, linePen);
    MoveToEx(memDC, mapWidth + 12, 28, NULL);
    LineTo(memDC, width - 12, 28);

    wsprintfA(buf, "Loc: %d, %d", ship_x, ship_y);
    SetTextColor(memDC, RGB(255, 255, 255));
    TextOutA(memDC, mapWidth + 12, 32, buf, lstrlenA(buf));

    wsprintfA(buf, "Fuel: %d", (int)res_fuel);
    TextOutA(memDC, mapWidth + 12, 48, buf, lstrlenA(buf));
    
    wsprintfA(buf, "Hull: %d%%  Morale: %d%%", res_hull, res_morale);
    TextOutA(memDC, mapWidth + 12, 64, buf, lstrlenA(buf));
    
    wsprintfA(buf, "Crew: %d  Credits: %d", roster_count, res_credits);
    TextOutA(memDC, mapWidth + 12, 80, buf, lstrlenA(buf));

    wsprintfA(buf, "SUPERWEAPON");
    SetTextColor(memDC, RGB(255, 180, 0));
    TextOutA(memDC, mapWidth + 12, 102, buf, lstrlenA(buf));
    MoveToEx(memDC, mapWidth + 12, 118, NULL);
    LineTo(memDC, width - 12, 118);

    wsprintfA(buf, "%s [%d/%d]", superweapon_names[superweapon_type], superweapon_charges, superweapon_max);
    SetTextColor(memDC, RGB(255, 220, 100));
    TextOutA(memDC, mapWidth + 12, 122, buf, lstrlenA(buf));

    wsprintfA(buf, "FACTIONS");
    SetTextColor(memDC, RGB(0, 255, 255));
    TextOutA(memDC, mapWidth + 12, 142, buf, lstrlenA(buf));
    MoveToEx(memDC, mapWidth + 12, 158, NULL);
    LineTo(memDC, width - 12, 158);

    wsprintfA(buf, "Fed:%d Syn:%d Xen:%d", faction_rep[1], faction_rep[2], faction_rep[3]);
    SetTextColor(memDC, RGB(180, 240, 255));
    TextOutA(memDC, mapWidth + 12, 162, buf, lstrlenA(buf));

    wsprintfA(buf, "CARGO & UPGRADES");
    SetTextColor(memDC, RGB(0, 255, 255));
    TextOutA(memDC, mapWidth + 12, 184, buf, lstrlenA(buf));
    MoveToEx(memDC, mapWidth + 12, 200, NULL);
    LineTo(memDC, width - 12, 200);

    wsprintfA(buf, "Min:%d Tech:%d", cargo_minerals, cargo_tech);
    SetTextColor(memDC, RGB(255, 255, 255));
    TextOutA(memDC, mapWidth + 12, 204, buf, lstrlenA(buf));

    wsprintfA(buf, "W:%d S:%d E:%d C:%d", upg_weapons, upg_shields, upg_engines, upg_cargo);
    TextOutA(memDC, mapWidth + 12, 220, buf, lstrlenA(buf));

    wsprintfA(buf, "SCANNER");
    SetTextColor(memDC, RGB(0, 255, 255));
    TextOutA(memDC, mapWidth + 12, 242, buf, lstrlenA(buf));
    MoveToEx(memDC, mapWidth + 12, 258, NULL);
    LineTo(memDC, width - 12, 258);
    DeleteObject(linePen);

    int found_sys_idx = -1;
    for (int i = 0; i < NUM_SYSTEMS; i++) {
        int dx = systems[i].x - ship_x;
        int dy = systems[i].y - ship_y;
        if (dx*dx + dy*dy < 2500) {
            found_sys_idx = i;
            break;
        }
    }

    if (found_sys_idx != -1) {
        StarSystem* sys = &systems[found_sys_idx];
        wsprintfA(buf, "Star: %s\nFaction: %s\nPlanets: %d", star_names[sys->type_idx], faction_names[sys->faction], sys->num_planets);
        SetTextColor(memDC, RGB(0, 255, 255));
        RECT textRect = {mapWidth + 12, 264, width - 8, 320};
        DrawTextA(memDC, buf, -1, &textRect, DT_WORDBREAK);
        
        char pbuf[256] = "";
        for (int p = 0; p < sys->num_planets; p++) {
            if (p > 0) lstrcatA(pbuf, ", ");
            lstrcatA(pbuf, planet_names[sys->planets[p]]);
        }
        if (sys->num_planets == 0) lstrcatA(pbuf, "None");
        SetTextColor(memDC, RGB(136, 204, 204));
        RECT pRect = {mapWidth + 12, 324, width - 8, 380};
        DrawTextA(memDC, pbuf, -1, &pRect, DT_WORDBREAK);
    } else {
        wsprintfA(buf, "Deep space. Nothing nearby.");
        SetTextColor(memDC, RGB(136, 136, 136));
        RECT textRect = {mapWidth + 12, 264, width - 8, 330};
        DrawTextA(memDC, buf, -1, &textRect, DT_WORDBREAK);
    }
    
    wsprintfA(buf, "[C] Crew  [H] Help");
    SetTextColor(memDC, RGB(255, 255, 0));
    TextOutA(memDC, mapWidth + 12, height - 52, buf, lstrlenA(buf));
    if (found_sys_idx != -1 && systems[found_sys_idx].num_planets > 0) {
        wsprintfA(buf, "[L] Land on Planet");
        TextOutA(memDC, mapWidth + 12, height - 32, buf, lstrlenA(buf));
    }

    if (modal_open) {
        RECT modalRect = { mapWidth/2 - 180, height/2 - 120, mapWidth/2 + 180, height/2 + 120 };
        HBRUSH mBrush = CreateSolidBrush(RGB(5, 5, 20));
        HPEN mPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 255));
        SelectObject(memDC, mBrush);
        SelectObject(memDC, mPen);
        Rectangle(memDC, modalRect.left, modalRect.top, modalRect.right, modalRect.bottom);
        DeleteObject(mBrush);
        DeleteObject(mPen);
        
        char* title = "";
        char* desc = "";
        char desc_buf[350] = "";

        if (modal_enc_type == 1) { 
            title = "PIRATES ENCOUNTER";
            if (res_hull <= 0) {
                desc = "Your ship has been destroyed!\r\nGame Over.\r\nSPACE: Exit";
            } else if (pirate_hp <= 0) {
                wsprintfA(desc_buf, "%s\r\nSPACE: Claim Bounty (120C, +5 Fed Rep)", combat_log);
                desc = desc_buf;
            } else {
                wsprintfA(desc_buf, "%s\r\nPirate HP: %d | Hull: %d%%\r\n1: Fire Lasers  2: Flee\r\n3: Fire Superweapon [%d charges]", combat_log, pirate_hp, res_hull, superweapon_charges);
                desc = desc_buf;
            }
        }
        else if (modal_enc_type == 13) { 
            title = "FLEET BATTLE";
            if (res_hull <= 0) {
                desc = "Your ship has been destroyed!\r\nGame Over.\r\nSPACE: Exit";
            } else if (pirate_hp <= 0) {
                wsprintfA(desc_buf, "%s\r\nSPACE: Claim Rewards (350C, 2 Tech, +15 Rep)", combat_log);
                desc = desc_buf;
            } else {
                wsprintfA(desc_buf, "%s\r\nFleet HP: %d | Hull: %d%%\r\n1: Fire Lasers  2: Flee\r\n3: Fire Superweapon [%d charges]", combat_log, pirate_hp, res_hull, superweapon_charges);
                desc = desc_buf;
            }
        }
        else if (modal_enc_type == 16) {
            title = "FACTION WAR ZONE";
            desc = "Federation and Syndicate dreadnoughts are locked in battle!\r\n"
                   "1: Aid Federation (+25 Fed, 200C)\r\n"
                   "2: Aid Syndicate (+25 Syn, 2 Tech)\r\n"
                   "3: Salvage Under Crossfire (Risk Hull)\r\n"
                   "4: Negotiate Ceasefire (Requires 60 Morale)\r\n"
                   "SPACE: Withdraw";
        }
        else if (modal_enc_type == 17) {
            title = "ALIEN BOARDING PARTY";
            desc = "Alien xenomorphs breached the lower bulkheads!\r\n"
                   "1: Security Tactical Sweep (Gunner Lvl roll)\r\n"
                   "2: Vent Atmosphere (Cost 200 Fuel)\r\n"
                   "3: Overcharge Defense Grid (1 SW Charge or 100C)\r\n"
                   "4: Hand-to-Hand Crew Charge (Risk crew for tech)\r\n"
                   "SPACE: Seal Blast Doors";
        }
        else if (modal_enc_type == 18) {
            title = "SUPERWEAPON FORGE";
            wsprintfA(desc_buf, "Active: %s (Charges: %d/%d)\r\n"
                                "1: Tachyon Beam (200C, 2 Tech)\r\n"
                                "2: Antimatter Torpedo (400C, 4 Tech)\r\n"
                                "3: Nova Obliterator (800C, 6 Tech)\r\n"
                                "4: Synthesize 2 Charges (100C, 1 Tech)\r\n"
                                "SPACE: Exit Forge",
                                superweapon_names[superweapon_type], superweapon_charges, superweapon_max);
            desc = desc_buf;
        }
        else if (modal_enc_type == 14) { title = "ALIEN DIPLOMACY"; desc = "An alien vessel hails you.\r\n1: Trade (100C for 1 Tech)\r\n2: Insult (Starts Combat)\r\nSPACE: Ignore"; }
        else if (modal_enc_type == 10) { title = "PIRATES ENCOUNTER"; wsprintfA(desc_buf, "%s\r\nHull: %d%%\r\nSPACE: Continue", combat_log, res_hull); desc = desc_buf; }
        else if (modal_enc_type == 11) { title = "EVENT RESULT"; wsprintfA(desc_buf, "%s\r\n\r\nSPACE: Continue", combat_log); desc = desc_buf; }
        else if (modal_enc_type == 7) { title = "DERELICT SHIP"; desc = "You find a derelict starship.\r\n1: Salvage (Risk Hull, Gain Cargo)\r\n2: Ignore"; }
        else if (modal_enc_type == 8) { title = "DISTRESS SIGNAL"; desc = "You receive a distress signal.\r\n1: Help (Cost 50 Fuel, Risk/Reward)\r\n2: Ignore"; }
        else if (modal_enc_type == 9) { title = "ANCIENT RUINS"; desc = "Scanners detect ancient ruins.\r\n1: Explore (Risk Crew, Gain Tech)\r\n2: Leave"; }
        else if (modal_enc_type == 2) { title = "DEEP SPACE ANOMALY"; desc = "A swirling rift in space.\r\n1: Scan (Risk Hull, Gain Tech)\r\n2: Harvest (Risk Crew, Gain Fuel)\r\nSPACE: Leave"; }
        else if (modal_enc_type == 3) { title = "TRADER ENCOUNTER"; desc = "A wandering trader offers help.\r\n1 crew member joins\r\nyour ship."; }
        else if (modal_enc_type == 4) { title = "STATION"; desc = "1: Buy Fuel(50) 2: Rep Hull(100)\r\n3: Buy Min(100) 4: Sell Min(80)\r\n5: Buy Tech(300) 6: Sell Tech(250)\r\n7: Shipyard 8: Recruit(100C)\r\n9: Superweapon Bay  SPACE: Leave"; }
        else if (modal_enc_type == 5) { title = "SHIPYARD"; desc = "1: Upg Wpn 2: Upg Shd (500C/Lvl)\r\n3: Upg Eng 4: Upg Cargo (500C/Lvl)\r\nSPACE: Back to Station"; }
        else if (modal_enc_type == 15) { title = "PLANETARY LANDING"; desc = "You landed on a planet.\r\n1: Explore (Risk Morale, Gain Min)\r\n2: Rest (Gain Morale)\r\nSPACE: Leave"; }
        else if (modal_enc_type == 12) {
            title = "CAPTAIN'S MANUAL (LOOP 10)";
            desc = "SUPERWEAPONS: Key [3] in combat to fire!\r\n"
                   "FACTION WARS: Intervene in war zones for rep & bounties.\r\n"
                   "BOARDING: Repel alien intruders with tactical deck sweeps.\r\n"
                   "CTRLS: W/A/S/D move, C Crew, H Help, L Land.\r\n"
                   "RES: Fuel, Hull, Morale, Credits, Tech, Minerals.\r\n"
                   "STATION: Refuel, upgrade, and craft Superweapons.\r\n";
        }
        else if (modal_enc_type == 6) {
            title = "CREW MANAGEMENT";
            int p_i = GetOfficer(1), g_i = GetOfficer(2), e_i = GetOfficer(3);
            char p_b[64]="Pilot: NONE", g_b[64]="Gunner: NONE", e_b[64]="Eng: NONE";
            if(p_i!=-1) wsprintfA(p_b, "Pilot: %s L%d (%d XP)", roster[p_i].name, roster[p_i].level, roster[p_i].xp);
            if(g_i!=-1) wsprintfA(g_b, "Gunner: %s L%d (%d XP)", roster[g_i].name, roster[g_i].level, roster[g_i].xp);
            if(e_i!=-1) wsprintfA(e_b, "Eng: %s L%d (%d XP)", roster[e_i].name, roster[e_i].level, roster[e_i].xp);
            int un = roster_count - (p_i!=-1) - (g_i!=-1) - (e_i!=-1);
            wsprintfA(desc_buf, "%s\r\n%s\r\n%s\r\nUnassigned: %d\r\n\r\n1:Cycle P  2:Cycle G  3:Cycle E", p_b, g_b, e_b, un);
            desc = desc_buf;
        }
        
        SetTextColor(memDC, RGB(255, 136, 0));
        RECT tRect = { modalRect.left + 10, modalRect.top + 8, modalRect.right - 10, modalRect.top + 28 };
        DrawTextA(memDC, title, -1, &tRect, DT_CENTER);
        
        SetTextColor(memDC, RGB(255, 255, 255));
        RECT dRect = { modalRect.left + 10, modalRect.top + 34, modalRect.right - 10, modalRect.bottom - 32 };
        DrawTextA(memDC, desc, -1, &dRect, DT_CENTER | DT_WORDBREAK);
        
        SetTextColor(memDC, RGB(0, 255, 255));
        RECT bRect = { modalRect.left + 10, modalRect.bottom - 26, modalRect.right - 10, modalRect.bottom - 6 };
        if (modal_enc_type == 4) {
            DrawTextA(memDC, "[ 1-9 OR SPACE ]", -1, &bRect, DT_CENTER);
        } else if (modal_enc_type == 6) {
            DrawTextA(memDC, "[ 1-3 OR SPACE ]", -1, &bRect, DT_CENTER);
        } else if (modal_enc_type == 5 || modal_enc_type == 16 || modal_enc_type == 17 || modal_enc_type == 18) {
            DrawTextA(memDC, "[ 1-4 OR SPACE ]", -1, &bRect, DT_CENTER);
        } else if ((modal_enc_type == 1 && res_hull > 0 && pirate_hp > 0) || (modal_enc_type == 13 && res_hull > 0 && pirate_hp > 0)) {
            DrawTextA(memDC, "[ 1: Laser  2: Flee  3: Superweapon ]", -1, &bRect, DT_CENTER);
        } else if (modal_enc_type == 7 || modal_enc_type == 8 || modal_enc_type == 9 || modal_enc_type == 14 || modal_enc_type == 2 || modal_enc_type == 15) {
            DrawTextA(memDC, "[ 1-2 OR SPACE ]", -1, &bRect, DT_CENTER);
        } else {
            DrawTextA(memDC, "[ PRESS SPACE ]", -1, &bRect, DT_CENTER);
        }
    }

    if (modal_open && modal_enc_type == 13) {
        int bX = centerX;
        int bY = centerY - 100;
        int hY = bY + (int)(sin(GetTickCount() / 500.0f) * 10.0f);
        
        POINT bossPts[5] = {
            {bX, hY - 80},
            {bX + 120, hY + 40},
            {bX + 60, hY + 80},
            {bX - 60, hY + 80},
            {bX - 120, hY + 40}
        };
        HBRUSH bBrush = CreateSolidBrush(RGB(34, 34, 34));
        HPEN bPen = CreatePen(PS_SOLID, 3, RGB(255, 68, 68));
        SelectObject(memDC, bBrush);
        SelectObject(memDC, bPen);
        Polygon(memDC, bossPts, 5);
        DeleteObject(bBrush);
        DeleteObject(bPen);
        
        if (pirate_hp > 100) {
            POINT p1Pts[4] = { {bX, hY - 60}, {bX + 80, hY + 20}, {bX, hY + 60}, {bX - 80, hY + 20} };
            HBRUSH p1Brush = CreateSolidBrush(RGB(85, 85, 85));
            SelectObject(memDC, p1Brush);
            SelectObject(memDC, GetStockObject(NULL_PEN));
            Polygon(memDC, p1Pts, 4);
            DeleteObject(p1Brush);
        } else {
            int debY1 = hY + ((GetTickCount() / 30) % 100);
            int debY2 = hY - ((GetTickCount() / 20) % 100);
            HBRUSH p1Brush = CreateSolidBrush(RGB(85, 85, 85));
            SelectObject(memDC, p1Brush);
            Rectangle(memDC, bX + 90, debY1, bX + 100, debY1 + 10);
            Rectangle(memDC, bX - 80, debY2, bX - 65, debY2 + 10);
            DeleteObject(p1Brush);
        }
        
        if (pirate_hp > 50) {
            HBRUSH p2Brush = CreateSolidBrush(RGB(119, 119, 119));
            SelectObject(memDC, p2Brush);
            Rectangle(memDC, bX - 40, hY + 10, bX + 40, hY + 40);
            DeleteObject(p2Brush);
        } else {
            HBRUSH p2Brush = CreateSolidBrush(RGB(255, 0, 0));
            SelectObject(memDC, p2Brush);
            int rad = 20 + (rand() % 10);
            Ellipse(memDC, bX - rad, hY + 25 - rad, bX + rad, hY + 25 + rad);
            DeleteObject(p2Brush);
        }
        
        if (pirate_hp > 0) {
            float charge = (GetTickCount() % 2000) / 2000.0f;
            int cRad = 10 + (int)(charge * 30);
            HPEN cPen = CreatePen(PS_SOLID, 2, RGB(255, 100, 100));
            SelectObject(memDC, GetStockObject(NULL_BRUSH));
            SelectObject(memDC, cPen);
            Ellipse(memDC, bX - cRad, hY + 80 - cRad, bX + cRad, hY + 80 + cRad);
            DeleteObject(cPen);
        }
    }

    int shake_x = 0;
    int shake_y = 0;
    if (screen_shake > 0) {
        shake_x = (rand() % (screen_shake * 2 + 1)) - screen_shake;
        shake_y = (rand() % (screen_shake * 2 + 1)) - screen_shake;
        HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
        RECT r = {0, 0, width, height};
        FillRect(hdc, &r, blackBrush);
        DeleteObject(blackBrush);
    }
    
    if (boss_whiteout > 0.0f) {
        int alpha = (int)(boss_whiteout * 255);
        if (alpha > 255) alpha = 255;
        BLENDFUNCTION bf;
        bf.BlendOp = AC_SRC_OVER;
        bf.BlendFlags = 0;
        bf.SourceConstantAlpha = alpha;
        bf.AlphaFormat = 0;
        
        HDC whiteDC = CreateCompatibleDC(hdc);
        HBITMAP whiteBmp = CreateCompatibleBitmap(hdc, width, height);
        SelectObject(whiteDC, whiteBmp);
        HBRUSH wBrush = CreateSolidBrush(RGB(255, 255, 255));
        RECT wr = {0, 0, width, height};
        FillRect(whiteDC, &wr, wBrush);
        DeleteObject(wBrush);
        
        AlphaBlend(memDC, 0, 0, width, height, whiteDC, 0, 0, width, height, bf);
        
        if (boss_whiteout > 0.5f) {
            bf.SourceConstantAlpha = (int)((boss_whiteout - 0.5f) * 255);
            HBRUSH rBrush = CreateSolidBrush(RGB(255, 0, 0));
            FillRect(whiteDC, &wr, rBrush);
            DeleteObject(rBrush);
            AlphaBlend(memDC, 10, 0, width, height, whiteDC, 0, 0, width, height, bf);
            
            HBRUSH cBrush = CreateSolidBrush(RGB(0, 255, 255));
            FillRect(whiteDC, &wr, cBrush);
            DeleteObject(cBrush);
            AlphaBlend(memDC, -10, 0, width, height, whiteDC, 0, 0, width, height, bf);
        }
        DeleteObject(whiteBmp);
        DeleteDC(whiteDC);
    }

    BitBlt(hdc, shake_x, shake_y, width, height, memDC, 0, 0, SRCCOPY);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

void ExecuteSuperweaponAttack(int is_fleet) {
    if (superweapon_charges <= 0) {
        lstrcpyA(combat_log, "No Superweapon charges remaining!");
        return;
    }
    superweapon_charges--;
    PlaySoundEffect(3); // Superweapon audio
    int base_dmg = 70 + superweapon_type * 25 + (rand() % 20);
    int g_idx = GetOfficer(2); if(g_idx != -1) base_dmg += roster[g_idx].level * 4;
    
    pirate_hp -= base_dmg;
    if (pirate_hp < 0) pirate_hp = 0;
    
    boss_whiteout = 1.0f;
    screen_shake = 35;
    SpawnBeamEffect((float)ship_x, (float)(ship_y - 20), RGB(0, 255, 255));
    SpawnExplosion((float)ship_x, (float)(ship_y - 50), 60, RGB(0, 255, 255), RGB(255, 200, 0));
    
    if (pirate_hp <= 0) {
        SpawnExplosion((float)ship_x, (float)(ship_y - 40), 90, RGB(255, 100, 0), RGB(255, 255, 255));
        wsprintfA(combat_log, "SUPERWEAPON HIT for %d! Target obliterated!", base_dmg);
        AddXP(2, 40);
    } else {
        int s_dmg = (is_fleet ? 15 : 8) - upg_shields * 2;
        if (s_dmg < 0) s_dmg = 0;
        res_hull -= s_dmg;
        if (res_hull < 0) res_hull = 0;
        wsprintfA(combat_log, "SUPERWEAPON HIT for %d! Counter-fire takes %d hull.", base_dmg, s_dmg);
        AddXP(2, 20);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            SetTimer(hwnd, 1, 16, NULL);
            return 0;
        case WM_KEYDOWN:
            if (!modal_open && (wParam == 'C' || wParam == 'c')) {
                modal_open = 1;
                modal_enc_type = 6;
                return 0;
            }
            if (!modal_open && (wParam == 'H' || wParam == 'h')) {
                modal_open = 1;
                modal_enc_type = 12;
                return 0;
            }
            if (!modal_open && (wParam == 'L' || wParam == 'l')) {
                int found_sys_idx = -1;
                for (int i = 0; i < NUM_SYSTEMS; i++) {
                    int dx = systems[i].x - ship_x;
                    int dy = systems[i].y - ship_y;
                    if (dx*dx + dy*dy < 2500) {
                        found_sys_idx = i;
                        break;
                    }
                }
                if (found_sys_idx != -1 && systems[found_sys_idx].num_planets > 0) {
                    modal_open = 1;
                    modal_enc_type = 15;
                }
                return 0;
            }
            if (modal_open) {
                if (modal_enc_type == 1) {
                    if (res_hull <= 0) {
                        if (wParam == VK_SPACE) { PostQuitMessage(0); }
                    } else if (pirate_hp <= 0) {
                        if (wParam == VK_SPACE) { 
                            res_credits += 120; 
                            faction_rep[1] += 5; // Federation bounty
                            modal_open = 0; 
                        }
                    } else {
                        if (wParam == '1') {
                            PlaySoundEffect(1); // Laser
                            int p_dmg = 10 + upg_weapons * 5 + (rand() % 10);
                            int g_idx = GetOfficer(2); if(g_idx != -1) p_dmg += roster[g_idx].level * 2;
                            int s_dmg = 20 - upg_shields * 3 + (rand() % 5);
                            int e_idx = GetOfficer(3); if(e_idx != -1) s_dmg -= roster[e_idx].level * 1;
                            if (s_dmg < 0) s_dmg = 0;
                            pirate_hp -= p_dmg;
                            if (pirate_hp < 0) pirate_hp = 0;
                            SpawnExplosion((float)ship_x, (float)(ship_y - 40), 20, RGB(255, 100, 0), RGB(255, 0, 0));
                            if (pirate_hp > 0) {
                                res_hull -= s_dmg;
                                res_morale -= s_dmg / 2;
                                if (res_morale < 0) res_morale = 0;
                                if (res_hull < 0) res_hull = 0;
                                SpawnExplosion((float)ship_x, (float)ship_y, 15, RGB(0, 255, 255), RGB(255, 255, 255));
                                if (res_hull <= 0) {
                                    SpawnExplosion((float)ship_x, (float)ship_y, 100, RGB(255, 0, 0), RGB(255, 255, 0));
                                }
                                wsprintfA(combat_log, "You hit for %d! Pirate hits for %d!", p_dmg, s_dmg);
                                AddXP(2, 10); AddXP(3, 10);
                            } else {
                                SpawnExplosion((float)ship_x, (float)(ship_y - 40), 50, RGB(255, 50, 0), RGB(200, 200, 200));
                                wsprintfA(combat_log, "You hit for %d! Pirate destroyed!", p_dmg);
                                AddXP(2, 20);
                            }
                        } else if (wParam == '2') {
                            int s_dmg = 15 - upg_shields * 2;
                            if (s_dmg < 0) s_dmg = 0;
                            res_hull -= s_dmg;
                            res_morale -= s_dmg / 2;
                            if (res_morale < 0) res_morale = 0;
                            if (res_hull < 0) res_hull = 0;
                            wsprintfA(combat_log, "You fled! Took %d damage.", s_dmg);
                            modal_enc_type = 10;
                        } else if (wParam == '3') {
                            ExecuteSuperweaponAttack(0);
                        }
                    }
                } else if (modal_enc_type == 13) {
                    if (res_hull <= 0) {
                        if (wParam == VK_SPACE) { PostQuitMessage(0); }
                    } else if (pirate_hp <= 0) {
                        if (wParam == VK_SPACE) { 
                            res_credits += 350; 
                            cargo_tech += 2; 
                            faction_rep[1] += 15;
                            modal_open = 0; 
                        }
                    } else {
                        if (wParam == '1') {
                            PlaySoundEffect(1); // Laser
                            int p_dmg = 10 + upg_weapons * 5 + (rand() % 10);
                            int g_idx = GetOfficer(2); if(g_idx != -1) p_dmg += roster[g_idx].level * 2;
                            int s_dmg = 30 - upg_shields * 3 + (rand() % 10);
                            int e_idx = GetOfficer(3); if(e_idx != -1) s_dmg -= roster[e_idx].level * 1;
                            if (s_dmg < 0) s_dmg = 0;
                            pirate_hp -= p_dmg;
                            if (pirate_hp < 0) pirate_hp = 0;
                            SpawnExplosion((float)ship_x, (float)(ship_y - 40), 30, RGB(255, 100, 0), RGB(255, 0, 0));
                            if (pirate_hp > 0) {
                                res_hull -= s_dmg;
                                res_morale -= s_dmg / 2;
                                if (res_morale < 0) res_morale = 0;
                                if (res_hull < 0) res_hull = 0;
                                SpawnExplosion((float)ship_x, (float)ship_y, 25, RGB(0, 255, 255), RGB(255, 255, 255));
                                if (res_hull <= 0) {
                                    SpawnExplosion((float)ship_x, (float)ship_y, 100, RGB(255, 0, 0), RGB(255, 255, 0));
                                }
                                wsprintfA(combat_log, "You hit for %d! Fleet hits for %d!", p_dmg, s_dmg);
                                AddXP(2, 15); AddXP(3, 15);
                            } else {
                                SpawnExplosion((float)ship_x, (float)(ship_y - 40), 80, RGB(255, 50, 0), RGB(200, 200, 200));
                                wsprintfA(combat_log, "You hit for %d! Fleet destroyed!", p_dmg);
                                AddXP(2, 30);
                                boss_whiteout = 1.0f;
                                screen_shake = 30;
                            }
                        } else if (wParam == '2') {
                            int s_dmg = 25 - upg_shields * 2;
                            if (s_dmg < 0) s_dmg = 0;
                            res_hull -= s_dmg;
                            if (res_hull < 0) res_hull = 0;
                            wsprintfA(combat_log, "You fled! Took %d damage.", s_dmg);
                            modal_enc_type = 10;
                        } else if (wParam == '3') {
                            ExecuteSuperweaponAttack(1);
                        }
                    }
                } else if (modal_enc_type == 16) { // Faction War
                    if (wParam == '1') {
                        faction_rep[1] += 25;
                        faction_rep[2] -= 15;
                        res_credits += 200;
                        lstrcpyA(combat_log, "You supported the Federation! (+25 Fed, -15 Syn, +200C)");
                        modal_enc_type = 11;
                    } else if (wParam == '2') {
                        faction_rep[2] += 25;
                        faction_rep[1] -= 15;
                        cargo_tech += 2;
                        lstrcpyA(combat_log, "You supported the Syndicate! (+25 Syn, -15 Fed, +2 Tech)");
                        modal_enc_type = 11;
                    } else if (wParam == '3') {
                        if (rand() % 2 == 0) {
                            cargo_minerals += 2; cargo_tech += 1;
                            lstrcpyA(combat_log, "Daring salvage under crossfire! Gained 2 Min, 1 Tech.");
                        } else {
                            res_hull -= 20; if (res_hull < 0) res_hull = 0;
                            lstrcpyA(combat_log, "Stray torpedo hit! Lost 20 Hull during salvage.");
                        }
                        modal_enc_type = 11;
                    } else if (wParam == '4') {
                        if (res_morale >= 60) {
                            faction_rep[1] += 15; faction_rep[2] += 15; res_credits += 150;
                            lstrcpyA(combat_log, "Diplomatic ceasefire brokered! (+15 Rep with both, +150C)");
                        } else {
                            lstrcpyA(combat_log, "Crew morale too low to command diplomatic respect!");
                        }
                        modal_enc_type = 11;
                    } else if (wParam == VK_SPACE) {
                        modal_open = 0;
                    }
                } else if (modal_enc_type == 17) { // Alien Boarding Party
                    if (wParam == '1') { // Security sweep
                        int g_idx = GetOfficer(2);
                        int roll = (g_idx != -1 ? roster[g_idx].level * 25 : 10) + (rand() % 40);
                        if (roll >= 45) {
                            cargo_tech += 1;
                            lstrcpyA(combat_log, "Security sweep successful! Alien intruders eliminated. (+1 Tech)");
                            AddXP(2, 25);
                        } else {
                            res_hull -= 15; if (res_hull < 0) res_hull = 0;
                            lstrcpyA(combat_log, "Fierce firefight on deck B. Lost 15 Hull.");
                        }
                        modal_enc_type = 11;
                    } else if (wParam == '2') { // Vent atmosphere
                        if (res_fuel >= 200.0f) {
                            res_fuel -= 200.0f;
                            lstrcpyA(combat_log, "Deck vented! Alien lifeforms flushed into vacuum. (200 Fuel used)");
                        } else {
                            lstrcpyA(combat_log, "Not enough fuel for emergency atmospheric purge!");
                        }
                        modal_enc_type = 11;
                    } else if (wParam == '3') { // Overcharge defense grid
                        if (superweapon_charges > 0) {
                            superweapon_charges--;
                            lstrcpyA(combat_log, "Defense grid overcharged via Superweapon core! Aliens vaporized.");
                        } else if (res_credits >= 100) {
                            res_credits -= 100;
                            lstrcpyA(combat_log, "Defense turrets overloaded with auxiliary power! (100C spent)");
                        } else {
                            lstrcpyA(combat_log, "Insufficient energy or credits to overcharge defenses!");
                        }
                        modal_enc_type = 11;
                    } else if (wParam == '4') { // Crew melee charge
                        if (roster_count > 1 && rand() % 2 == 0) {
                            roster_count--;
                            cargo_tech += 2;
                            lstrcpyA(combat_log, "Aliens repelled, but a valiant crew member fell. (+2 Tech)");
                        } else {
                            cargo_tech += 2;
                            res_credits += 100;
                            lstrcpyA(combat_log, "Heroic crew assault victorious! (+2 Tech, +100C)");
                        }
                        modal_enc_type = 11;
                    } else if (wParam == VK_SPACE) {
                        modal_open = 0;
                    }
                } else if (modal_enc_type == 18) { // Superweapon Forge
                    if (wParam == '1' && res_credits >= 200 && cargo_tech >= 2) {
                        res_credits -= 200; cargo_tech -= 2;
                        superweapon_type = 1; superweapon_charges = superweapon_max;
                        lstrcpyA(combat_log, "Tachyon Beam superweapon forged & fully charged!");
                        modal_enc_type = 11;
                    } else if (wParam == '2' && res_credits >= 400 && cargo_tech >= 4) {
                        res_credits -= 400; cargo_tech -= 4;
                        superweapon_type = 2; superweapon_charges = superweapon_max;
                        lstrcpyA(combat_log, "Antimatter Torpedo launcher forged & fully charged!");
                        modal_enc_type = 11;
                    } else if (wParam == '3' && res_credits >= 800 && cargo_tech >= 6) {
                        res_credits -= 800; cargo_tech -= 6;
                        superweapon_type = 4; superweapon_charges = superweapon_max;
                        lstrcpyA(combat_log, "Nova Obliterator forged & fully charged!");
                        modal_enc_type = 11;
                    } else if (wParam == '4' && res_credits >= 100 && cargo_tech >= 1) {
                        if (superweapon_charges < superweapon_max) {
                            res_credits -= 100; cargo_tech -= 1;
                            superweapon_charges = (superweapon_charges + 2 > superweapon_max) ? superweapon_max : superweapon_charges + 2;
                            lstrcpyA(combat_log, "Superweapon antimatter charges synthesized (+2 charges)!");
                        } else {
                            lstrcpyA(combat_log, "Superweapon charges are already at maximum!");
                        }
                        modal_enc_type = 11;
                    } else if (wParam == VK_SPACE) {
                        modal_open = 0;
                    }
                } else if (modal_enc_type == 14) {
                    if (wParam == '1') {
                        if (res_credits >= 100) {
                            res_credits -= 100;
                            cargo_tech += 1;
                            faction_rep[3] += 10;
                            lstrcpyA(combat_log, "You successfully traded with the aliens. (+10 Xenon Rep)");
                        } else {
                            lstrcpyA(combat_log, "You don't have enough credits!");
                        }
                        modal_enc_type = 11;
                    } else if (wParam == '2') {
                        PlaySoundEffect(2);
                        pirate_hp = 80;
                        enemy_max_hp = 80;
                        faction_rep[3] -= 20;
                        lstrcpyA(combat_log, "The aliens are offended and attack!");
                        modal_enc_type = 1;
                    } else if (wParam == VK_SPACE) {
                        modal_open = 0;
                    }
                } else if (modal_enc_type == 15) {
                    if (wParam == '1') {
                        if (rand() % 2 == 0) {
                            cargo_minerals += 2;
                            lstrcpyA(combat_log, "Found resources! (+2 Minerals)");
                        } else {
                            res_morale -= 20;
                            if (res_morale < 0) res_morale = 0;
                            lstrcpyA(combat_log, "Hostile environment! (-20 Morale)");
                        }
                        modal_enc_type = 11;
                    } else if (wParam == '2') {
                        res_morale += 20;
                        if (res_morale > 100) res_morale = 100;
                        lstrcpyA(combat_log, "Crew is rested. (+20 Morale)");
                        modal_enc_type = 11;
                    } else if (wParam == VK_SPACE) {
                        modal_open = 0;
                    }
                } else if (modal_enc_type == 2) {
                    if (wParam == '1') {
                        if (rand() % 2 == 0) {
                            cargo_tech += 2;
                            lstrcpyA(combat_log, "Scan successful! Gained 2 Tech.");
                        } else {
                            res_hull -= 20; if (res_hull < 0) res_hull = 0;
                            lstrcpyA(combat_log, "The anomaly surged! Lost 20 Hull.");
                        }
                        modal_enc_type = 11;
                    } else if (wParam == '2') {
                        if (rand() % 2 == 0) {
                            res_fuel += 800.0f;
                            lstrcpyA(combat_log, "Harvest successful! Gained 800 Fuel.");
                        } else {
                            if (roster_count > 0) roster_count--;
                            lstrcpyA(combat_log, "Radiation leak! Lost 1 crew member.");
                        }
                        modal_enc_type = 11;
                    } else if (wParam == VK_SPACE) {
                        modal_open = 0;
                    }
                } else if (modal_enc_type == 10 || modal_enc_type == 11) {
                    if (wParam == VK_SPACE) { modal_open = 0; }
                } else if (modal_enc_type == 7) {
                    if (wParam == '1') {
                        if (rand() % 2 == 0) {
                            cargo_tech += 1; cargo_minerals += 1;
                            lstrcpyA(combat_log, "You successfully salvaged tech and minerals!");
                        } else {
                            res_hull -= 15; if (res_hull < 0) res_hull = 0;
                            lstrcpyA(combat_log, "A booby trap exploded! Lost 15 Hull.");
                        }
                        modal_enc_type = 11;
                    } else if (wParam == '2') {
                        modal_open = 0;
                    }
                } else if (modal_enc_type == 8) {
                    if (wParam == '1') {
                        if (res_fuel >= 50) {
                            res_fuel -= 50;
                            if (rand() % 10 < 6) {
                                res_credits += 150;
                                faction_rep[1] += 5;
                                lstrcpyA(combat_log, "You rescued a merchant. Earned 150 Credits.");
                            } else {
                                lstrcpyA(combat_log, "It was a false alarm. You wasted fuel.");
                            }
                            modal_enc_type = 11;
                        }
                    } else if (wParam == '2') {
                        modal_open = 0;
                    }
                } else if (modal_enc_type == 9) {
                    if (wParam == '1') {
                        if (rand() % 2 == 0) {
                            cargo_tech += 2;
                            lstrcpyA(combat_log, "You found valuable ancient technology!");
                        } else {
                            if (roster_count > 0) {
                                roster_count--;
                                lstrcpyA(combat_log, "A cave-in occurred. You lost a crew member.");
                            } else {
                                lstrcpyA(combat_log, "You explored but found nothing.");
                            }
                        }
                        modal_enc_type = 11;
                    } else if (wParam == '2') {
                        modal_open = 0;
                    }
                } else if (modal_enc_type == 4) {
                    if (wParam == '1' && res_credits >= 50) { res_credits -= 50; res_fuel += 500.0f; }
                    if (wParam == '2' && res_credits >= 100 && res_hull < 100) { res_credits -= 100; res_hull += 20; if (res_hull > 100) res_hull = 100; }
                    int max_cargo = upg_cargo * 10;
                    if (wParam == '3' && res_credits >= 100 && (cargo_minerals + cargo_tech) < max_cargo) { res_credits -= 100; cargo_minerals += 1; }
                    if (wParam == '4' && cargo_minerals > 0) { res_credits += 80; cargo_minerals -= 1; }
                    if (wParam == '5' && res_credits >= 300 && (cargo_minerals + cargo_tech) < max_cargo) { res_credits -= 300; cargo_tech += 1; }
                    if (wParam == '6' && cargo_tech > 0) { res_credits += 250; cargo_tech -= 1; }
                    if (wParam == '7') { modal_enc_type = 5; }
                    if (wParam == '8' && res_credits >= 100 && roster_count < 50) {
                        res_credits -= 100;
                        lstrcpyA(roster[roster_count].name, first_names[rand() % 16]);
                        roster[roster_count].role = 0;
                        roster[roster_count].level = 1;
                        roster[roster_count].xp = 0;
                        roster_count++;
                    }
                    if (wParam == '9') { modal_enc_type = 18; } // Superweapon Bay
                    if (wParam == VK_SPACE) { modal_open = 0; }
                } else if (modal_enc_type == 6) {
                    if (wParam == '1') CycleOfficer(1);
                    if (wParam == '2') CycleOfficer(2);
                    if (wParam == '3') CycleOfficer(3);
                    if (wParam == VK_SPACE) modal_open = 0;
                } else if (modal_enc_type == 5) {
                    if (wParam == '1' && res_credits >= upg_weapons*500 && upg_weapons < 5) { res_credits -= upg_weapons*500; upg_weapons++; }
                    if (wParam == '2' && res_credits >= upg_shields*500 && upg_shields < 5) { res_credits -= upg_shields*500; upg_shields++; }
                    if (wParam == '3' && res_credits >= upg_engines*500 && upg_engines < 5) { res_credits -= upg_engines*500; upg_engines++; }
                    if (wParam == '4' && res_credits >= upg_cargo*500 && upg_cargo < 5) { res_credits -= upg_cargo*500; upg_cargo++; }
                    if (wParam == VK_SPACE) { modal_enc_type = 4; }
                } else if (modal_enc_type == 12) {
                    if (wParam == VK_SPACE) { modal_open = 0; }
                } else {
                    if (wParam == VK_SPACE) {
                        modal_open = 0;
                    }
                }
            }
            return 0;
        case WM_TIMER:
            Update();
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            Draw(hdc, &rect);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    CreateThread(NULL, 0, EngineHumThread, NULL, 0, NULL);
    InitEnvironment();
    InitStars();
    for (int i = 0; i < 10; i++) {
        lstrcpyA(roster[i].name, first_names[rand() % 16]);
        roster[i].role = 0;
        roster[i].level = 1;
        roster[i].xp = 0;
    }
    roster_count = 10;

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KStarshipClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(0, wc.lpszClassName, "KStarship Command", WS_OVERLAPPEDWINDOW, 
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
