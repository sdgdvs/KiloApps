#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_PARTICLES 300
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
    for (int i=0; i<count; i++) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = (rand() % 50) / 10.0f;
        float vx = cos(angle) * speed;
        float vy = sin(angle) * speed;
        float decay = 0.02f + (rand() % 50) / 1000.0f;
        COLORREF c = (rand() % 2 == 0) ? color1 : color2;
        SpawnParticle(x, y, vx, vy, decay, c);
    }
}


#define NUM_SYSTEMS 200
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
} StarSystem;

StarSystem systems[NUM_SYSTEMS];

const char* star_names[] = {"Red Dwarf", "Yellow Dwarf", "Blue Giant", "White Dwarf"};
const char* planet_names[] = {"Terrestrial", "Gas Giant", "Ice World", "Lava", "Barren"};

int ship_x = 0;
int ship_y = 0;
int is_moving = 0;
float res_fuel = 10000.0f;
int res_hull = 100;
int res_credits = 1000;

typedef struct {
    char name[16];
    int role; // 0=Unassigned, 1=Pilot, 2=Gunner, 3=Engineer
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
char combat_log[128] = "";

DWORD WINAPI SoundThread(LPVOID lpParam) {
    int type = (int)(intptr_t)lpParam;
    if (type == 1) { // Laser
        Beep(800, 50);
        Beep(400, 50);
    } else if (type == 2) { // Alarm
        Beep(400, 250);
        Beep(600, 250);
        Beep(400, 250);
        Beep(600, 250);
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
        lstrcpyA(combat_log, "Space pirates ambush you!");
    } else if (type == 13) {
        PlaySoundEffect(2); // Alarm
        pirate_hp = 150;
        lstrcpyA(combat_log, "A hostile fleet intercepts you!");
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

void InitEnvironment() {
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
        if (r < 40) {
            systems[i].type_idx = 0; systems[i].color = RGB(255, 170, 170);
            systems[i].size = 2; systems[i].num_planets = 1 + rand() % 3;
        } else if (r < 80) {
            systems[i].type_idx = 1; systems[i].color = RGB(255, 255, 170);
            systems[i].size = 3; systems[i].num_planets = 2 + rand() % 5;
        } else if (r < 95) {
            systems[i].type_idx = 2; systems[i].color = RGB(170, 221, 255);
            systems[i].size = 4; systems[i].num_planets = rand() % 3;
        } else {
            systems[i].type_idx = 3; systems[i].color = RGB(255, 255, 255);
            systems[i].size = 1; systems[i].num_planets = rand() % 2;
        }

        for (int p = 0; p < systems[i].num_planets; p++) {
            systems[i].planets[p] = rand() % 5;
        }

        int enc = rand() % 9;
        if (enc == 0) systems[i].encounter_type = 1;
        else if (enc == 1) systems[i].encounter_type = 2;
        else if (enc == 2) systems[i].encounter_type = 3;
        else if (enc == 3) systems[i].encounter_type = 4;
        else if (enc == 4) systems[i].encounter_type = 7 + (rand() % 3);
        else if (enc == 5) systems[i].encounter_type = 13;
        else if (enc == 6) systems[i].encounter_type = 14;
        else systems[i].encounter_type = 0;
        
        systems[i].visited = 0;
    }
}

void Update() {
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

    // (Particle updates moved to start of Update function to run during modals)
    

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
    
    int mapWidth = width - 200;
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
    Rectangle(memDC, mapWidth + 5, 5, width - 5, height - 5);
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

    POINT shipPts[4] = {
        {centerX, centerY - 10},
        {centerX + 8, centerY + 8},
        {centerX, centerY + 4},
        {centerX - 8, centerY + 8}
    };
    HBRUSH shipBrush = CreateSolidBrush(RGB(0, 255, 255));
    HPEN shipPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
    SelectObject(memDC, shipBrush);
    SelectObject(memDC, shipPen);
    Polygon(memDC, shipPts, 4);
    DeleteObject(shipBrush);
    DeleteObject(shipPen);

    // Frame cycle animation for ship lights
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
                SetPixel(memDC, screenX, screenY, particles[i].color);
                SetPixel(memDC, screenX+1, screenY, particles[i].color);
                SetPixel(memDC, screenX, screenY+1, particles[i].color);
                SetPixel(memDC, screenX+1, screenY+1, particles[i].color);
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
        // Frame cycle for thruster flame
        int flameLength = 16 + (GetTickCount() % 4);
        POINT thrustPts[4] = {
            {centerX, centerY + 6},
            {centerX + 4, centerY + 10},
            {centerX, centerY + flameLength},
            {centerX - 4, centerY + 10}
        };
        HBRUSH thrustBrush = CreateSolidBrush(RGB(255, 136, 0));
        HPEN thrustPen = CreatePen(PS_SOLID, 1, RGB(255, 136, 0));
        SelectObject(memDC, thrustBrush);
        SelectObject(memDC, thrustPen);
        Polygon(memDC, thrustPts, 4);
        DeleteObject(thrustBrush);
        DeleteObject(thrustPen);
    }

    SetBkMode(memDC, TRANSPARENT);
    char buf[128];
    
    HPEN linePen = CreatePen(PS_SOLID, 1, RGB(0, 85, 85));
    
    wsprintfA(buf, "SHIP STATUS");
    SetTextColor(memDC, RGB(0, 255, 255));
    TextOutA(memDC, mapWidth + 15, 20, buf, lstrlenA(buf));
    
    SelectObject(memDC, linePen);
    MoveToEx(memDC, mapWidth + 15, 38, NULL);
    LineTo(memDC, width - 15, 38);

    wsprintfA(buf, "Location: %d, %d", ship_x, ship_y);
    SetTextColor(memDC, RGB(255, 255, 255));
    TextOutA(memDC, mapWidth + 15, 45, buf, lstrlenA(buf));

    wsprintfA(buf, "RESOURCES");
    SetTextColor(memDC, RGB(0, 255, 255));
    TextOutA(memDC, mapWidth + 15, 75, buf, lstrlenA(buf));
    
    MoveToEx(memDC, mapWidth + 15, 93, NULL);
    LineTo(memDC, width - 15, 93);
    
    wsprintfA(buf, "Fuel: %d", (int)res_fuel);
    SetTextColor(memDC, RGB(255, 255, 255));
    TextOutA(memDC, mapWidth + 15, 100, buf, lstrlenA(buf));
    
    wsprintfA(buf, "Hull: %d%%", res_hull);
    TextOutA(memDC, mapWidth + 15, 120, buf, lstrlenA(buf));
    
    wsprintfA(buf, "Crew: %d", roster_count);
    TextOutA(memDC, mapWidth + 15, 140, buf, lstrlenA(buf));

    wsprintfA(buf, "Credits: %d", res_credits);
    TextOutA(memDC, mapWidth + 15, 160, buf, lstrlenA(buf));

    wsprintfA(buf, "CARGO");
    SetTextColor(memDC, RGB(0, 255, 255));
    TextOutA(memDC, mapWidth + 15, 185, buf, lstrlenA(buf));

    MoveToEx(memDC, mapWidth + 15, 203, NULL);
    LineTo(memDC, width - 15, 203);

    wsprintfA(buf, "Minerals: %d", cargo_minerals);
    SetTextColor(memDC, RGB(255, 255, 255));
    TextOutA(memDC, mapWidth + 15, 210, buf, lstrlenA(buf));

    wsprintfA(buf, "Tech: %d", cargo_tech);
    TextOutA(memDC, mapWidth + 15, 230, buf, lstrlenA(buf));

    wsprintfA(buf, "UPGRADES");
    SetTextColor(memDC, RGB(0, 255, 255));
    TextOutA(memDC, mapWidth + 15, 260, buf, lstrlenA(buf));

    MoveToEx(memDC, mapWidth + 15, 278, NULL);
    LineTo(memDC, width - 15, 278);

    wsprintfA(buf, "Wpn: L%d  Shd: L%d", upg_weapons, upg_shields);
    SetTextColor(memDC, RGB(255, 255, 255));
    TextOutA(memDC, mapWidth + 15, 285, buf, lstrlenA(buf));

    wsprintfA(buf, "Eng: L%d  Car: L%d", upg_engines, upg_cargo);
    TextOutA(memDC, mapWidth + 15, 305, buf, lstrlenA(buf));

    wsprintfA(buf, "SCANNER");
    SetTextColor(memDC, RGB(0, 255, 255));
    TextOutA(memDC, mapWidth + 15, 335, buf, lstrlenA(buf));
    
    MoveToEx(memDC, mapWidth + 15, 353, NULL);
    LineTo(memDC, width - 15, 353);
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
        wsprintfA(buf, "Star: %s\nPlanets: %d", star_names[sys->type_idx], sys->num_planets);
        SetTextColor(memDC, RGB(0, 255, 255));
        RECT textRect = {mapWidth + 15, 360, width - 10, 400};
        DrawTextA(memDC, buf, -1, &textRect, DT_WORDBREAK);
        
        char pbuf[256] = "";
        for (int p = 0; p < sys->num_planets; p++) {
            if (p > 0) lstrcatA(pbuf, ", ");
            lstrcatA(pbuf, planet_names[sys->planets[p]]);
        }
        if (sys->num_planets == 0) lstrcatA(pbuf, "None");
        SetTextColor(memDC, RGB(136, 204, 204));
        RECT pRect = {mapWidth + 15, 400, width - 10, 475};
        DrawTextA(memDC, pbuf, -1, &pRect, DT_WORDBREAK);
    } else {
        wsprintfA(buf, "Deep space. Nothing nearby.");
        SetTextColor(memDC, RGB(136, 136, 136));
        RECT textRect = {mapWidth + 15, 360, width - 10, 460};
        DrawTextA(memDC, buf, -1, &textRect, DT_WORDBREAK);
    }
    
    wsprintfA(buf, "[C] Crew  [H] Help");
    SetTextColor(memDC, RGB(255, 255, 0));
    TextOutA(memDC, mapWidth + 15, height - 30, buf, lstrlenA(buf));

    if (modal_open) {
        RECT modalRect = { mapWidth/2 - 170, height/2 - 100, mapWidth/2 + 170, height/2 + 100 };
        HBRUSH mBrush = CreateSolidBrush(RGB(5, 5, 20));
        HPEN mPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 255));
        SelectObject(memDC, mBrush);
        SelectObject(memDC, mPen);
        Rectangle(memDC, modalRect.left, modalRect.top, modalRect.right, modalRect.bottom);
        DeleteObject(mBrush);
        DeleteObject(mPen);
        
        char* title = "";
        char* desc = "";
        char desc_buf[256] = "";
        if (modal_enc_type == 1) { 
            title = "PIRATES ENCOUNTER";
            if (res_hull <= 0) {
                desc = "Your ship has been destroyed!\r\nGame Over.\r\nSPACE: Exit";
            } else if (pirate_hp <= 0) {
                wsprintfA(desc_buf, "%s\r\nSPACE: Claim Bounty (100C)", combat_log);
                desc = desc_buf;
            } else {
                wsprintfA(desc_buf, "%s\r\nPirate HP: %d | Hull: %d%%\r\n1: Fire Weapons  2: Flee", combat_log, pirate_hp, res_hull);
                desc = desc_buf;
            }
        }
        else if (modal_enc_type == 13) { 
            title = "FLEET BATTLE";
            if (res_hull <= 0) {
                desc = "Your ship has been destroyed!\r\nGame Over.\r\nSPACE: Exit";
            } else if (pirate_hp <= 0) {
                wsprintfA(desc_buf, "%s\r\nSPACE: Claim Rewards (300C, 2 Tech)", combat_log);
                desc = desc_buf;
            } else {
                wsprintfA(desc_buf, "%s\r\nFleet HP: %d | Hull: %d%%\r\n1: Fire Weapons  2: Flee", combat_log, pirate_hp, res_hull);
                desc = desc_buf;
            }
        }
        else if (modal_enc_type == 14) { title = "ALIEN DIPLOMACY"; desc = "An alien vessel hails you.\r\n1: Trade (100C for 1 Tech)\r\n2: Insult (Starts Combat)\r\nSPACE: Ignore"; }
        else if (modal_enc_type == 10) { title = "PIRATES ENCOUNTER"; wsprintfA(desc_buf, "%s\r\nHull: %d%%\r\nSPACE: Continue", combat_log, res_hull); desc = desc_buf; }
        else if (modal_enc_type == 11) { title = "EVENT RESULT"; wsprintfA(desc_buf, "%s\r\n\r\nSPACE: Continue", combat_log); desc = desc_buf; }
        else if (modal_enc_type == 7) { title = "DERELICT SHIP"; desc = "You find a derelict starship.\r\n1: Salvage (Risk Hull, Gain Cargo)\r\n2: Ignore"; }
        else if (modal_enc_type == 8) { title = "DISTRESS SIGNAL"; desc = "You receive a distress signal.\r\n1: Help (Cost 50 Fuel, Risk/Reward)\r\n2: Ignore"; }
        else if (modal_enc_type == 9) { title = "ANCIENT RUINS"; desc = "Scanners detect ancient ruins.\r\n1: Explore (Risk Crew, Gain Tech)\r\n2: Leave"; }
        else if (modal_enc_type == 2) { title = "DEEP SPACE ANOMALY"; desc = "A swirling rift in space.\r\n1: Scan (Risk Hull, Gain Tech)\r\n2: Harvest (Risk Crew, Gain Fuel)\r\nSPACE: Leave"; }
        else if (modal_enc_type == 3) { title = "TRADER ENCOUNTER"; desc = "A wandering trader offers help.\r\n1 crew member joins\r\nyour ship."; }
        else if (modal_enc_type == 4) { title = "STATION"; desc = "1: Buy Fuel(50) 2: Rep Hull(100)\r\n3: Buy Min(100) 4: Sell Min(80)\r\n5: Buy Tech(300) 6: Sell Tech(250)\r\n7: Shipyard 8: Tavern(Recruit 100C)\r\nSPACE: Leave"; }
        else if (modal_enc_type == 5) { title = "SHIPYARD"; desc = "1: Upg Wpn 2: Upg Shd (500C/Lvl)\r\n3: Upg Eng 4: Upg Cargo (500C/Lvl)\r\nSPACE: Back to Station"; }
        else if (modal_enc_type == 12) {
            title = "CAPTAIN'S MANUAL";
            desc = "GOAL: Explore, trade, upgrade.\r\n"
                   "CTRLS: W/A/S/D move, C Crew, H Help.\r\n"
                   "RES: Fuel(move), Hull(health), Credits.\r\n"
                   "CREW: Pilot(spd), Gun(dmg), Eng(def).\r\n"
                   "UPG: Improve ship at stations.\r\n";
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
        RECT tRect = { modalRect.left + 10, modalRect.top + 10, modalRect.right - 10, modalRect.top + 30 };
        DrawTextA(memDC, title, -1, &tRect, DT_CENTER);
        
        SetTextColor(memDC, RGB(255, 255, 255));
        RECT dRect = { modalRect.left + 10, modalRect.top + 40, modalRect.right - 10, modalRect.bottom - 40 };
        DrawTextA(memDC, desc, -1, &dRect, DT_CENTER | DT_WORDBREAK);
        
        SetTextColor(memDC, RGB(0, 255, 255));
        RECT bRect = { modalRect.left + 10, modalRect.bottom - 30, modalRect.right - 10, modalRect.bottom - 10 };
        if (modal_enc_type == 4) {
            DrawTextA(memDC, "[ 1-8 OR SPACE ]", -1, &bRect, DT_CENTER);
        } else if (modal_enc_type == 6) {
            DrawTextA(memDC, "[ 1-3 OR SPACE ]", -1, &bRect, DT_CENTER);
        } else if (modal_enc_type == 5) {
            DrawTextA(memDC, "[ 1-4 OR SPACE ]", -1, &bRect, DT_CENTER);
        } else if ((modal_enc_type == 1 && res_hull > 0 && pirate_hp > 0) || (modal_enc_type == 13 && res_hull > 0 && pirate_hp > 0) || modal_enc_type == 7 || modal_enc_type == 8 || modal_enc_type == 9 || modal_enc_type == 14 || modal_enc_type == 2) {
            DrawTextA(memDC, "[ 1-2 OR SPACE ]", -1, &bRect, DT_CENTER);
        } else {
            DrawTextA(memDC, "[ PRESS SPACE ]", -1, &bRect, DT_CENTER);
        }
    }

    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
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
            if (modal_open) {
                if (modal_enc_type == 1) {
                    if (res_hull <= 0) {
                        if (wParam == VK_SPACE) { PostQuitMessage(0); }
                    } else if (pirate_hp <= 0) {
                        if (wParam == VK_SPACE) { res_credits += 100; modal_open = 0; }
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
                            // Weapon impact on pirate
                            SpawnExplosion(ship_x, ship_y - 40, 20, RGB(255, 100, 0), RGB(255, 0, 0));
                            if (pirate_hp > 0) {
                                res_hull -= s_dmg;
                                if (res_hull < 0) res_hull = 0;
                                // Weapon impact on ship
                                SpawnExplosion(ship_x, ship_y, 15, RGB(0, 255, 255), RGB(255, 255, 255));
                                if (res_hull <= 0) {
                                    // Death effect
                                    SpawnExplosion(ship_x, ship_y, 100, RGB(255, 0, 0), RGB(255, 255, 0));
                                }
                                wsprintfA(combat_log, "You hit for %d! Pirate hits for %d!", p_dmg, s_dmg);
                                AddXP(2, 10); AddXP(3, 10);
                            } else {
                                // Pirate death explosion
                                SpawnExplosion(ship_x, ship_y - 40, 50, RGB(255, 50, 0), RGB(200, 200, 200));
                                wsprintfA(combat_log, "You hit for %d! Pirate destroyed!", p_dmg);
                                AddXP(2, 20);
                            }
                        } else if (wParam == '2') {
                            int s_dmg = 15 - upg_shields * 2;
                            if (s_dmg < 0) s_dmg = 0;
                            res_hull -= s_dmg;
                            if (res_hull < 0) res_hull = 0;
                            wsprintfA(combat_log, "You fled! Took %d damage.", s_dmg);
                            modal_enc_type = 10;
                        }
                    }
                } else if (modal_enc_type == 13) {
                    if (res_hull <= 0) {
                        if (wParam == VK_SPACE) { PostQuitMessage(0); }
                    } else if (pirate_hp <= 0) {
                        if (wParam == VK_SPACE) { res_credits += 300; cargo_tech += 2; modal_open = 0; }
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
                            SpawnExplosion(ship_x, ship_y - 40, 30, RGB(255, 100, 0), RGB(255, 0, 0));
                            if (pirate_hp > 0) {
                                res_hull -= s_dmg;
                                if (res_hull < 0) res_hull = 0;
                                SpawnExplosion(ship_x, ship_y, 25, RGB(0, 255, 255), RGB(255, 255, 255));
                                if (res_hull <= 0) {
                                    SpawnExplosion(ship_x, ship_y, 100, RGB(255, 0, 0), RGB(255, 255, 0));
                                }
                                wsprintfA(combat_log, "You hit for %d! Fleet hits for %d!", p_dmg, s_dmg);
                                AddXP(2, 15); AddXP(3, 15);
                            } else {
                                SpawnExplosion(ship_x, ship_y - 40, 80, RGB(255, 50, 0), RGB(200, 200, 200));
                                wsprintfA(combat_log, "You hit for %d! Fleet destroyed!", p_dmg);
                                AddXP(2, 30);
                            }
                        } else if (wParam == '2') {
                            int s_dmg = 25 - upg_shields * 2;
                            if (s_dmg < 0) s_dmg = 0;
                            res_hull -= s_dmg;
                            if (res_hull < 0) res_hull = 0;
                            wsprintfA(combat_log, "You fled! Took %d damage.", s_dmg);
                            modal_enc_type = 10;
                        }
                    }
                } else if (modal_enc_type == 14) {
                    if (wParam == '1') {
                        if (res_credits >= 100) {
                            res_credits -= 100;
                            cargo_tech += 1;
                            lstrcpyA(combat_log, "You successfully traded with the aliens.");
                        } else {
                            lstrcpyA(combat_log, "You don't have enough credits!");
                        }
                        modal_enc_type = 11;
                    } else if (wParam == '2') {
                        PlaySoundEffect(2);
                        pirate_hp = 80;
                        lstrcpyA(combat_log, "The aliens are offended and attack!");
                        modal_enc_type = 1;
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
                } else if (modal_enc_type == 10) {
                    if (wParam == VK_SPACE) { modal_open = 0; }
                } else if (modal_enc_type == 11) {
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
