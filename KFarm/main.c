#include <windows.h>
#include <math.h>
#define GRID_COLS 10
#define GRID_ROWS 10
#define CELL_SIZE 40
#define OFFSET_X 10
#define OFFSET_Y 30

int get_scale(int x) {
    static int dpi = 0;
    if (dpi == 0) {
        HDC hdc = GetDC(NULL);
        dpi = GetDeviceCaps(hdc, LOGPIXELSX);
        ReleaseDC(NULL, hdc);
    }
    return MulDiv(x, dpi, 96);
}
#define S(x) get_scale(x)

BOOL CALLBACK SetFontCallback(HWND child, LPARAM font) {
    SendMessage(child, WM_SETFONT, (WPARAM)font, TRUE);
    return TRUE;
}

typedef struct {
    int type; // 0=Grass, 1=Tilled, 2=Planted, 3=Grown
    int watered;
    int growth;
    int cropType;
} Cell;
Cell grid[GRID_COLS * GRID_ROWS] = {0};

typedef struct {
    float x, y;
    float vx, vy;
    int life;
    int maxLife;
    COLORREF color;
    int type; // 0=basic, 1=water, 2=needle/spark, 3=star, 4=smoke, 5=shard
    float targetY;
    float drag;
    float angle, vAngle;
    int size;
} Particle;
#define MAX_PARTICLES 400
Particle particles[MAX_PARTICLES] = {0};

typedef struct {
    float x, y;
    float radius, maxRadius;
    float speed;
    COLORREF color;
    int life;
    int maxLife;
    float width;
} Shockwave;
#define MAX_SHOCKWAVES 16
Shockwave shockwaves[MAX_SHOCKWAVES] = {0};

typedef struct {
    float x, y;
    float vx, vy;
    float rot, vrot;
    int size;
    float sway;
} SeasonParticle;
#define MAX_SEASON_PARTICLES 30
SeasonParticle season_particles[MAX_SEASON_PARTICLES] = {0};

typedef struct {
    float x, y, tx, ty;
    int dir;
    int peck;
    int moveTimer;
} ChickenState;
ChickenState chickens_list[3] = {
    { 40.0f, 45.0f, 70.0f, 55.0f, 1, 0, 30 },
    { 350.0f, 340.0f, 320.0f, 360.0f, -1, 0, 70 },
    { 330.0f, 60.0f, 360.0f, 80.0f, 1, 0, 110 }
};

typedef struct {
    float x, y, tx, ty;
    int dir;
    int chew;
    int moveTimer;
} CowState;
CowState cows_list[2] = {
    { 60.0f, 340.0f, 90.0f, 350.0f, 1, 0, 50 },
    { 340.0f, 200.0f, 310.0f, 220.0f, -1, 0, 120 }
};

float shake_amount = 0.0f;

typedef struct { float x, y, speed; int size; } Cloud;
#define MAX_CLOUDS 5
Cloud clouds[MAX_CLOUDS] = {0};

void SpawnShockwave(float x, float y, COLORREF color, float maxRadius) {
    for (int i = 0; i < MAX_SHOCKWAVES; i++) {
        if (shockwaves[i].life <= 0) {
            shockwaves[i].x = x; shockwaves[i].y = y;
            shockwaves[i].radius = 4.0f;
            shockwaves[i].maxRadius = maxRadius;
            shockwaves[i].speed = 2.5f;
            shockwaves[i].color = color;
            shockwaves[i].life = 30;
            shockwaves[i].maxLife = 30;
            shockwaves[i].width = 3.0f;
            break;
        }
    }
}

void SpawnExplosionParticles(float x, float y, int cropType) {
    COLORREF crop_colors[4] = { RGB(255, 193, 7), RGB(76, 175, 80), RGB(244, 67, 54), RGB(255, 152, 0) };
    COLORREF cColor = (cropType >= 0 && cropType < 4) ? crop_colors[cropType] : RGB(255, 215, 0);

    int spawned_needles = 0;
    int spawned_smoke = 0;
    int spawned_shards = 0;
    int spawned_stars = 0;

    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life <= 0) {
            if (spawned_needles < 24) {
                // Tier 1: Incandescent Core Needle Sparks
                float angle = (float)(rand() % 628) / 100.0f;
                float speed = 3.0f + (float)(rand() % 50) / 10.0f;
                particles[i].x = x; particles[i].y = y;
                particles[i].vx = (float)cos(angle) * speed;
                particles[i].vy = (float)sin(angle) * speed - 1.5f;
                particles[i].life = 70;
                particles[i].maxLife = 70;
                particles[i].color = RGB(255, 250, 200);
                particles[i].type = 2; // needle/spark
                particles[i].drag = 0.94f;
                spawned_needles++;
            } else if (spawned_smoke < 12) {
                // Tier 2: Expanding Buoyant Smoke / Pollen Puffs
                float angle = (float)(rand() % 628) / 100.0f;
                float speed = 0.8f + (float)(rand() % 20) / 10.0f;
                particles[i].x = x + (float)((rand() % 20) - 10);
                particles[i].y = y + (float)((rand() % 20) - 10);
                particles[i].vx = (float)cos(angle) * speed;
                particles[i].vy = (float)sin(angle) * speed - 1.0f;
                particles[i].life = 60;
                particles[i].maxLife = 60;
                particles[i].color = cColor;
                particles[i].type = 4; // smoke
                particles[i].size = 4;
                particles[i].drag = 0.97f;
                spawned_smoke++;
            } else if (spawned_shards < 12) {
                // Tier 3: Heavy Kinematic Shards / Debris
                particles[i].x = x; particles[i].y = y;
                particles[i].vx = (float)((rand() % 100) - 50) / 12.0f;
                particles[i].vy = -(2.5f + (float)(rand() % 40) / 10.0f);
                particles[i].life = 80;
                particles[i].maxLife = 80;
                particles[i].color = (rand() % 2 == 0) ? RGB(93, 64, 55) : cColor;
                particles[i].type = 5; // shard
                particles[i].targetY = y + 18.0f;
                particles[i].size = 3 + rand() % 4;
                particles[i].drag = 0.98f;
                spawned_shards++;
            } else if (spawned_stars < 10) {
                // Tier 4: Radiant Golden Celebration Stars
                particles[i].x = x; particles[i].y = y;
                particles[i].vx = (float)((rand() % 100) - 50) / 20.0f;
                particles[i].vy = -(1.5f + (float)(rand() % 40) / 15.0f);
                particles[i].life = 100;
                particles[i].maxLife = 100;
                particles[i].color = RGB(255, 255, 255);
                particles[i].type = 3; // star
                particles[i].drag = 0.98f;
                spawned_stars++;
            } else {
                break;
            }
        }
    }
    SpawnShockwave(x, y, cColor, 45.0f);
}

void SpawnParticles(float x, float y, COLORREF color, int count) {
    int spawned = 0;
    for (int i = 0; i < MAX_PARTICLES && spawned < count; i++) {
        if (particles[i].life <= 0) {
            particles[i].x = x;
            particles[i].y = y;
            particles[i].vx = (float)((rand() % 100) - 50) / 20.0f;
            particles[i].vy = (float)((rand() % 100) - 100) / 20.0f - 2.0f;
            particles[i].life = 100;
            particles[i].maxLife = 100;
            particles[i].color = color;
            particles[i].type = 0;
            particles[i].drag = 0.98f;
            spawned++;
        }
    }
    if (count >= 10) {
        SpawnShockwave(x, y, color, 30.0f);
    }
}

void SpawnWaterParticles(float targetX, float targetY, int count) {
    int spawned = 0;
    for (int i = 0; i < MAX_PARTICLES && spawned < count; i++) {
        if (particles[i].life <= 0) {
            particles[i].x = targetX + (float)((rand() % 100) - 50) / 5.0f;
            particles[i].y = targetY + 40.0f + (float)((rand() % 100) - 50) / 10.0f;
            particles[i].vx = (float)((rand() % 100) - 50) / 25.0f;
            particles[i].vy = -4.0f - (float)(rand() % 100) / 50.0f;
            particles[i].life = 150;
            particles[i].maxLife = 150;
            particles[i].color = RGB(33, 150, 243);
            particles[i].type = 1;
            particles[i].drag = 0.98f;
            particles[i].targetY = targetY + (float)((rand() % 100) - 50) / 10.0f;
            spawned++;
        }
    }
}

int current_day = 1;
int time_of_day = 0; // 0=Day, 1=Night
int current_season = 0; // 0=Spring, 1=Summer, 2=Fall, 3=Winter
int crop_seasons[4] = {5, 2, 6, 4}; // Bitmasks: Wheat(0,2)->5, Corn(1)->2, Tomato(1,2)->6, Pumpkin(2)->4
const char* season_names[4] = {"Spring", "Summer", "Fall", "Winter"};
int growth_times[4] = {2, 3, 4, 5};
int sell_values[4] = {10, 20, 30, 50};
int seed_costs[4] = {5, 10, 15, 25};
int money = 50;
int fertilizer_bought = 0;
int tools_upgraded = 0;
int selected_seed = 0;
int chickens = 0;
int cows = 0;
int weather = 0; // 0=Clear, 1=Rain, 2=Drought, 3=Crows
const char* weather_names[4] = {"Clear", "Rain", "Drought", "Crows"};
int has_scarecrow = 0;
HWND hNextDayBtn;
HWND hSeedBtns[4];
HWND hUpgradeBtn;
HWND hUpgradeToolsBtn;
HWND hBuyChickenBtn;
HWND hBuyCowBtn;
HWND hBuyScarecrowBtn;
int has_mill = 0;
int has_mayo_maker = 0;
int has_cheese_press = 0;
HWND hMillBtn;
HWND hMayoBtn;
HWND hCheeseBtn;
HWND hHelpBtn;

void PlaySoundEffect(int type) {
    switch(type) {
        case 0: Beep(150, 50); break; // till
        case 1: Beep(400, 50); break; // plant
        case 2: Beep(600, 30); Beep(650, 30); break; // water
        case 3: Beep(300, 40); Beep(400, 40); break; // harvest
        case 4: Beep(500, 50); Beep(600, 50); break; // chicken
        case 5: Beep(100, 100); break; // cow
        case 6: Beep(200, 100); Beep(150, 100); break; // night
        case 7: Beep(400, 50); Beep(500, 50); Beep(600, 100); break; // morning
    }
}

void UpdateTitle(HWND hwnd) {
    char title[128];
    wsprintf(title, "KFarm - %s, Day %d | %s | $%d | Ch:%d Co:%d", season_names[current_season], current_day, weather_names[weather], money, chickens, cows);
    SetWindowText(hwnd, title);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            srand(GetTickCount());
            SetTimer(hwnd, 2, 33, NULL);
            for (int i = 0; i < MAX_CLOUDS; i++) {
                clouds[i].x = (float)(rand() % 400);
                clouds[i].y = (float)(rand() % 200);
                clouds[i].speed = 0.5f + (float)(rand() % 20) / 10.0f;
                clouds[i].size = 20 + rand() % 30;
            }
            for (int i = 0; i < MAX_SEASON_PARTICLES; i++) {
                season_particles[i].x = (float)(rand() % 400);
                season_particles[i].y = (float)(rand() % 400);
                season_particles[i].vx = (float)((rand() % 100) - 50) / 100.0f;
                season_particles[i].vy = 0.5f + (float)(rand() % 100) / 100.0f;
                season_particles[i].size = 3 + rand() % 4;
                season_particles[i].sway = (float)(rand() % 628) / 100.0f;
            }
            hUpgradeToolsBtn = CreateWindow("BUTTON", "Tools ($200)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                S(10), S(440), S(110), S(30), hwnd, (HMENU) 9, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hNextDayBtn = CreateWindow("BUTTON", "Sleep [Space]", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                S(130), S(440), S(140), S(30), hwnd, (HMENU) 1, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hUpgradeBtn = CreateWindow("BUTTON", "Fertilizer ($100)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                S(280), S(440), S(120), S(30), hwnd, (HMENU) 6, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            hSeedBtns[0] = CreateWindow("BUTTON", "[1] Wheat (-$5) [Sp/Fa]", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,
                S(10), S(480), S(145), S(20), hwnd, (HMENU) 2, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hSeedBtns[1] = CreateWindow("BUTTON", "[2] Corn (-$10) [Su]", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
                S(160), S(480), S(145), S(20), hwnd, (HMENU) 3, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hSeedBtns[2] = CreateWindow("BUTTON", "[3] Tomato (-$15) [Su/Fa]", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
                S(10), S(505), S(145), S(20), hwnd, (HMENU) 4, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hSeedBtns[3] = CreateWindow("BUTTON", "[4] Pumpkin (-$25) [Fa]", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
                S(160), S(505), S(145), S(20), hwnd, (HMENU) 5, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
                
            hBuyChickenBtn = CreateWindow("BUTTON", "Chicken ($50)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                S(310), S(480), S(100), S(20), hwnd, (HMENU) 7, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hBuyCowBtn = CreateWindow("BUTTON", "Cow ($150)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                S(310), S(505), S(100), S(20), hwnd, (HMENU) 8, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            hBuyScarecrowBtn = CreateWindow("BUTTON", "Scarecrow ($100)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                S(10), S(530), S(140), S(20), hwnd, (HMENU) 10, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hMillBtn = CreateWindow("BUTTON", "Mill ($150)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                S(160), S(530), S(100), S(20), hwnd, (HMENU) 11, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hMayoBtn = CreateWindow("BUTTON", "Mayo ($100)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                S(270), S(530), S(100), S(20), hwnd, (HMENU) 12, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            hCheeseBtn = CreateWindow("BUTTON", "Cheese ($200)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                S(10), S(555), S(140), S(20), hwnd, (HMENU) 13, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hHelpBtn = CreateWindow("BUTTON", "Almanac (H/F1)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                S(160), S(555), S(100), S(20), hwnd, (HMENU) 14, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            SendMessage(hSeedBtns[0], BM_SETCHECK, BST_CHECKED, 0);
            HFONT hFont = CreateFont(S(-13), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            EnumChildWindows(hwnd, SetFontCallback, (LPARAM)hFont);
            return 0;
        case WM_KEYDOWN:
            if (wParam == 'H' || wParam == 'h' || wParam == VK_F1) {
                SendMessage(hwnd, WM_COMMAND, 14, 0);
            } else if (wParam == VK_SPACE) {
                SendMessage(hwnd, WM_COMMAND, 1, 0);
            } else if (wParam >= '1' && wParam <= '4') {
                SendMessage(hwnd, WM_COMMAND, 2 + (wParam - '1'), 0);
            }
            return 0;
        case WM_COMMAND:
            if (LOWORD(wParam) == 1 && time_of_day == 0) {
                time_of_day = 1;
                PlaySoundEffect(6);
                InvalidateRect(hwnd, NULL, TRUE);
                SetTimer(hwnd, 1, 1000, NULL);
            }
            if (LOWORD(wParam) == 6 && time_of_day == 0 && !fertilizer_bought) {
                if (money >= 100) {
                    money -= 100;
                    fertilizer_bought = 1;
                    for(int i=0; i<4; i++) if (growth_times[i] > 1) growth_times[i]--;
                    EnableWindow(hUpgradeBtn, FALSE);
                    SetWindowText(hUpgradeBtn, "Fertilizer (Owned)");
                    SpawnShockwave(210.0f, 230.0f, RGB(129, 199, 132), 80.0f);
                    shake_amount += 8.0f;
                    UpdateTitle(hwnd);
                } else {
                    MessageBeep(MB_ICONERROR);
                }
            }
            if (LOWORD(wParam) == 7 && time_of_day == 0) {
                if (money >= 50) {
                    money -= 50;
                    chickens++;
                    PlaySoundEffect(4);
                    SpawnExplosionParticles(40.0f, 45.0f, 0);
                    shake_amount += 6.0f;
                    UpdateTitle(hwnd);
                } else {
                    MessageBeep(MB_ICONERROR);
                }
            }
            if (LOWORD(wParam) == 8 && time_of_day == 0) {
                if (money >= 150) {
                    money -= 150;
                    cows++;
                    PlaySoundEffect(5);
                    SpawnExplosionParticles(60.0f, 340.0f, 1);
                    shake_amount += 8.0f;
                    UpdateTitle(hwnd);
                } else {
                    MessageBeep(MB_ICONERROR);
                }
            }
            if (LOWORD(wParam) == 9 && time_of_day == 0 && !tools_upgraded) {
                if (money >= 200) {
                    money -= 200;
                    tools_upgraded = 1;
                    EnableWindow(hUpgradeToolsBtn, FALSE);
                    SetWindowText(hUpgradeToolsBtn, "Tools (Owned)");
                    SpawnShockwave(210.0f, 230.0f, RGB(66, 165, 245), 90.0f);
                    shake_amount += 12.0f;
                    UpdateTitle(hwnd);
                } else {
                    MessageBeep(MB_ICONERROR);
                }
            }
            if (LOWORD(wParam) == 10 && time_of_day == 0 && !has_scarecrow) {
                if (money >= 100) {
                    money -= 100;
                    has_scarecrow = 1;
                    EnableWindow(hBuyScarecrowBtn, FALSE);
                    SetWindowText(hBuyScarecrowBtn, "Scarecrow (Owned)");
                    SpawnShockwave(380.0f, 50.0f, RGB(255, 215, 0), 60.0f);
                    shake_amount += 10.0f;
                    UpdateTitle(hwnd);
                } else {
                    MessageBeep(MB_ICONERROR);
                }
            }
            if (LOWORD(wParam) == 11 && time_of_day == 0 && !has_mill) {
                if (money >= 150) {
                    money -= 150;
                    has_mill = 1;
                    sell_values[0] = 25;
                    EnableWindow(hMillBtn, FALSE);
                    SetWindowText(hMillBtn, "Mill (Owned)");
                    SpawnShockwave(30.0f, 50.0f, RGB(255, 224, 130), 60.0f);
                    shake_amount += 8.0f;
                    UpdateTitle(hwnd);
                } else {
                    MessageBeep(MB_ICONERROR);
                }
            }
            if (LOWORD(wParam) == 12 && time_of_day == 0 && !has_mayo_maker) {
                if (money >= 100) {
                    money -= 100;
                    has_mayo_maker = 1;
                    EnableWindow(hMayoBtn, FALSE);
                    SetWindowText(hMayoBtn, "Mayo (Owned)");
                    SpawnShockwave(210.0f, 230.0f, RGB(255, 245, 157), 60.0f);
                    shake_amount += 8.0f;
                    UpdateTitle(hwnd);
                } else {
                    MessageBeep(MB_ICONERROR);
                }
            }
            if (LOWORD(wParam) == 13 && time_of_day == 0 && !has_cheese_press) {
                if (money >= 200) {
                    money -= 200;
                    has_cheese_press = 1;
                    EnableWindow(hCheeseBtn, FALSE);
                    SetWindowText(hCheeseBtn, "Cheese (Owned)");
                    SpawnShockwave(210.0f, 230.0f, RGB(255, 224, 130), 70.0f);
                    shake_amount += 10.0f;
                    UpdateTitle(hwnd);
                } else {
                    MessageBeep(MB_ICONERROR);
                }
            }
            if (LOWORD(wParam) == 14) {
                MessageBox(hwnd, "How to Play:\nClick Grass to Till. Select a seed and click tilled soil to Plant.\nClick planted seed to Water (daily!). Click grown crop to Harvest.\n\nCrops:\nWheat: Grow 2d, Val $10 (Mill $25), Sp/Fa\nCorn: Grow 3d, Val $20, Su\nTomato: Grow 4d, Val $30, Su/Fa\nPumpkin: Grow 5d, Val $50, Fa\n\nAnimals:\nChicken: $5/day (Mayo $15/day)\nCow: $15/day (Cheese $40/day)\n\nWeather:\nClear: Need 1 water\nRain: Auto-waters crops\nDrought: Need 2 water\nCrows: Eats crops (Buy Scarecrow!)\n\nShortcuts:\n[1]-[4]: Select Seed\n[Space]: Sleep (Next Day)\n[H] or [F1]: Open Almanac", "Farmer's Almanac", MB_OK | MB_ICONINFORMATION);
            }
            if (LOWORD(wParam) >= 2 && LOWORD(wParam) <= 5) {
                selected_seed = LOWORD(wParam) - 2;
                CheckRadioButton(hwnd, 2, 5, LOWORD(wParam));
            }
            return 0;
        case WM_TIMER:
            if (wParam == 2) {
                if (shake_amount > 0.1f) {
                    shake_amount *= 0.9f;
                } else {
                    shake_amount = 0.0f;
                }
                for (int i = 0; i < MAX_CLOUDS; i++) {
                    clouds[i].x += clouds[i].speed;
                    if (clouds[i].x > 500) clouds[i].x = (float)(-clouds[i].size * 2);
                }
                for (int i = 0; i < MAX_SEASON_PARTICLES; i++) {
                    season_particles[i].y += season_particles[i].vy;
                    season_particles[i].x += (float)sin(GetTickCount() * 0.002f + season_particles[i].sway) * 0.6f + season_particles[i].vx;
                    if (season_particles[i].y > 420.0f) {
                        season_particles[i].y = -10.0f;
                        season_particles[i].x = (float)(rand() % 400);
                    }
                    if (season_particles[i].x < -10.0f) season_particles[i].x = 410.0f;
                    if (season_particles[i].x > 410.0f) season_particles[i].x = -10.0f;
                }
                for (int i = 0; i < MAX_SHOCKWAVES; i++) {
                    if (shockwaves[i].life > 0) {
                        shockwaves[i].radius += shockwaves[i].speed;
                        shockwaves[i].life--;
                    }
                }
                // Update roaming chickens
                int actCh = (chickens < 3) ? chickens : 3;
                for (int i = 0; i < actCh; i++) {
                    chickens_list[i].moveTimer--;
                    if (chickens_list[i].moveTimer <= 0) {
                        chickens_list[i].moveTimer = 40 + rand() % 80;
                        chickens_list[i].tx = (float)(20 + rand() % 360);
                        chickens_list[i].ty = (float)(20 + rand() % 360);
                        chickens_list[i].peck = (rand() % 100 < 40) ? 20 : 0;
                    }
                    float dx = chickens_list[i].tx - chickens_list[i].x;
                    float dy = chickens_list[i].ty - chickens_list[i].y;
                    if (fabsf(dx) > 1.0f) {
                        chickens_list[i].x += (dx > 0 ? 0.6f : -0.6f);
                        chickens_list[i].dir = (dx > 0 ? 1 : -1);
                    }
                    if (fabsf(dy) > 1.0f) chickens_list[i].y += (dy > 0 ? 0.6f : -0.6f);
                    if (chickens_list[i].peck > 0) chickens_list[i].peck--;
                }
                // Update roaming cows
                int actCw = (cows < 2) ? cows : 2;
                for (int i = 0; i < actCw; i++) {
                    cows_list[i].moveTimer--;
                    if (cows_list[i].moveTimer <= 0) {
                        cows_list[i].moveTimer = 60 + rand() % 120;
                        cows_list[i].tx = (float)(30 + rand() % 340);
                        cows_list[i].ty = (float)(30 + rand() % 340);
                        cows_list[i].chew = (rand() % 100 < 50) ? 40 : 0;
                    }
                    float dx = cows_list[i].tx - cows_list[i].x;
                    float dy = cows_list[i].ty - cows_list[i].y;
                    if (fabsf(dx) > 1.0f) {
                        cows_list[i].x += (dx > 0 ? 0.4f : -0.4f);
                        cows_list[i].dir = (dx > 0 ? 1 : -1);
                    }
                    if (fabsf(dy) > 1.0f) cows_list[i].y += (dy > 0 ? 0.4f : -0.4f);
                    if (cows_list[i].chew > 0) cows_list[i].chew--;
                }
                for (int i = 0; i < MAX_PARTICLES; i++) {
                    if (particles[i].life > 0) {
                        particles[i].x += particles[i].vx;
                        particles[i].y += particles[i].vy;
                        if (particles[i].drag > 0.0f) {
                            particles[i].vx *= particles[i].drag;
                            particles[i].vy *= particles[i].drag;
                        }
                        if (particles[i].type == 2) { // needle
                            particles[i].vy += 0.08f;
                            particles[i].life -= 2;
                        } else if (particles[i].type == 3) { // star
                            particles[i].vy += 0.05f;
                            particles[i].life -= 2;
                        } else if (particles[i].type == 4) { // smoke
                            particles[i].vy -= 0.04f;
                            particles[i].life -= 2;
                        } else if (particles[i].type == 5) { // shard
                            particles[i].vy += 0.4f;
                            if (particles[i].targetY > 0 && particles[i].y > particles[i].targetY) {
                                particles[i].y = particles[i].targetY;
                                particles[i].vy = -particles[i].vy * 0.45f;
                                particles[i].vx *= 0.6f;
                            }
                            particles[i].life -= 2;
                        } else {
                            particles[i].vy += 0.5f; // gravity
                            if (particles[i].type == 1 && particles[i].vy > 0 && particles[i].y > particles[i].targetY) {
                                particles[i].type = 0;
                                particles[i].vy = -1.0f - (float)(rand()%10)/10.0f;
                                particles[i].vx = (float)((rand()%100)-50)/20.0f;
                            }
                            particles[i].life -= 5;
                        }
                    }
                }
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (wParam == 1) {
                KillTimer(hwnd, 1);
                PlaySoundEffect(7);
                current_day++;
                current_season = ((current_day - 1) / 7) % 4;
                time_of_day = 0;
                SpawnShockwave(210.0f, 230.0f, RGB(255, 245, 157), 130.0f);
                shake_amount += 8.0f;
                money += (chickens * (has_mayo_maker ? 15 : 5)) + (cows * (has_cheese_press ? 40 : 15));
                for (int i = 0; i < GRID_COLS * GRID_ROWS; i++) {
                    if (grid[i].type == 2 || grid[i].type == 3) {
                        if ((crop_seasons[grid[i].cropType] & (1 << current_season)) == 0) {
                            grid[i].type = 1; // Dies from season change
                        } else if (grid[i].type == 2) {
                            int req = (weather == 2) ? 2 : ((weather == 1) ? 0 : 1);
                            if (grid[i].watered >= req) {
                                grid[i].growth++;
                                if (grid[i].growth >= growth_times[grid[i].cropType]) grid[i].type = 3;
                            } else {
                                grid[i].type = 1; // Dies without water
                            }
                        }
                    }
                    grid[i].watered = 0;
                }
                
                int r = rand() % 100;
                if (r < 20) weather = 1;
                else if (r < 40) weather = 2;
                else if (r < 60) weather = 3;
                else weather = 0;
                
                if (weather == 3 && !has_scarecrow) {
                    for (int i = 0; i < GRID_COLS * GRID_ROWS; i++) {
                        if ((grid[i].type == 2 || grid[i].type == 3) && (rand() % 100) < 30) {
                            grid[i].type = 1;
                        }
                    }
                }
                
                InvalidateRect(hwnd, NULL, TRUE);
                UpdateTitle(hwnd);
            }
            return 0;
        case WM_LBUTTONDOWN: {
            if (time_of_day == 1) return 0;
            POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
            HDC hdc = GetDC(hwnd);
            SetMapMode(hdc, MM_ISOTROPIC);
            SetWindowExtEx(hdc, 420, 590, NULL);
            SetViewportExtEx(hdc, S(420), S(590), NULL);
            DPtoLP(hdc, &pt, 1);
            ReleaseDC(hwnd, hdc);
            
            if (pt.x < OFFSET_X || pt.y < OFFSET_Y) return 0;
            int cx = (pt.x - OFFSET_X) / CELL_SIZE;
            int cy = (pt.y - OFFSET_Y) / CELL_SIZE;
            if (cx >= 0 && cx < GRID_COLS && cy >= 0 && cy < GRID_ROWS) {
                int c_idx = cy * GRID_COLS + cx;
                int action = -1; // 0=till, 1=plant, 2=water, 3=harvest
                
                if (grid[c_idx].type == 0) action = 0;
                else if (grid[c_idx].type == 1) action = 1;
                else if (grid[c_idx].type == 2) {
                    int req = (weather == 2) ? 2 : 1;
                    if (grid[c_idx].watered < req) action = 2;
                }
                else if (grid[c_idx].type == 3) action = 3;
                
                if (action == -1) return 0;
                
                PlaySoundEffect(action);
                
                int is_aoe = tools_upgraded && (action == 0 || action == 2 || action == 3);
                int min_x = is_aoe ? (cx > 0 ? cx - 1 : 0) : cx;
                int max_x = is_aoe ? (cx < GRID_COLS - 1 ? cx + 1 : GRID_COLS - 1) : cx;
                int min_y = is_aoe ? (cy > 0 ? cy - 1 : 0) : cy;
                int max_y = is_aoe ? (cy < GRID_ROWS - 1 ? cy + 1 : GRID_ROWS - 1) : cy;
                
                for (int y = min_y; y <= max_y; y++) {
                    for (int x = min_x; x <= max_x; x++) {
                        int idx = y * GRID_COLS + x;
                        int px = OFFSET_X + x * CELL_SIZE + CELL_SIZE / 2;
                        int py = OFFSET_Y + y * CELL_SIZE + CELL_SIZE / 2;
                        if (action == 0 && grid[idx].type == 0) {
                            grid[idx].type = 1;
                            SpawnParticles((float)px, (float)py, RGB(93, 64, 55), 10);
                            shake_amount += 5.0f;
                        } else if (action == 1 && x == cx && y == cy && grid[idx].type == 1) {
                            if ((crop_seasons[selected_seed] & (1 << current_season)) != 0 && money >= seed_costs[selected_seed]) {
                                money -= seed_costs[selected_seed];
                                grid[idx].type = 2; grid[idx].growth = 0; grid[idx].cropType = selected_seed;
                                SpawnParticles((float)px, (float)py, RGB(139, 195, 74), 10);
                            } else {
                                MessageBeep(MB_ICONERROR);
                            }
                        } else if (action == 2 && grid[idx].type == 2) {
                            int req = (weather == 2) ? 2 : 1;
                            if (grid[idx].watered < req) {
                                grid[idx].watered++;
                                SpawnWaterParticles((float)px, (float)py, 20);
                            }
                        } else if (action == 3 && grid[idx].type == 3) {
                            money += sell_values[grid[idx].cropType];
                            int cType = grid[idx].cropType;
                            grid[idx].type = 1; grid[idx].watered = 0;
                            SpawnExplosionParticles((float)px, (float)py, cType);
                            shake_amount += 15.0f;
                        }
                    }
                }
                UpdateTitle(hwnd);
            }
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hWindowDC = BeginPaint(hwnd, &ps);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            
            HDC hdc = CreateCompatibleDC(hWindowDC);
            HBITMAP hMemBmp = CreateCompatibleBitmap(hWindowDC, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
            HBITMAP hOldBmp = (HBITMAP)SelectObject(hdc, hMemBmp);

            HBRUSH hSky = CreateSolidBrush(time_of_day ? RGB(26, 35, 126) : RGB(135, 206, 235));
            FillRect(hdc, &clientRect, hSky);
            DeleteObject(hSky);
            
            SetMapMode(hdc, MM_ISOTROPIC);
            SetWindowExtEx(hdc, 420, 590, NULL);
            SetViewportExtEx(hdc, S(420), S(590), NULL);
            
            int shake_dx = 0, shake_dy = 0;
            if (shake_amount > 0.1f) {
                shake_dx = (int)(((float)(rand() % 100) / 100.0f - 0.5f) * shake_amount);
                shake_dy = (int)(((float)(rand() % 100) / 100.0f - 0.5f) * shake_amount);
            }
            SetWindowOrgEx(hdc, -shake_dx, -shake_dy, NULL);
            
            COLORREF grass_colors[4] = { RGB(139, 195, 74), RGB(76, 175, 80), RGB(255, 179, 0), RGB(224, 247, 250) };
            COLORREF night_grass_colors[4] = { RGB(51, 80, 30), RGB(30, 70, 40), RGB(100, 70, 10), RGB(100, 120, 130) };
            COLORREF cGrass = time_of_day ? night_grass_colors[current_season] : grass_colors[current_season];
            COLORREF cSoil = time_of_day ? RGB(40, 20, 15) : RGB(93, 64, 55);
            COLORREF cWetSoil = time_of_day ? RGB(20, 10, 5) : RGB(62, 39, 35);
            COLORREF cDampSoil = time_of_day ? RGB(30, 15, 10) : RGB(78, 52, 45);
            
            HBRUSH hGrass = CreateSolidBrush(cGrass);
            HBRUSH hSoil = CreateSolidBrush(cSoil);
            HBRUSH hWetSoil = CreateSolidBrush(cWetSoil);
            HBRUSH hDampSoil = CreateSolidBrush(cDampSoil);
            
            HPEN hGridPen = CreatePen(PS_SOLID, 1, time_of_day ? RGB(40, 60, 20) : RGB(104, 159, 56));
            HPEN hSproutPen = CreatePen(PS_SOLID, 3, time_of_day ? RGB(80, 120, 40) : RGB(139, 195, 74));
            HPEN hWheatPen = CreatePen(PS_SOLID, 3, time_of_day ? RGB(150, 120, 40) : RGB(255, 213, 79));
            HPEN hCornPen = CreatePen(PS_SOLID, 3, time_of_day ? RGB(150, 150, 0) : RGB(255, 235, 59));
            HPEN hPumpkinPen = CreatePen(PS_SOLID, 3, time_of_day ? RGB(150, 90, 0) : RGB(255, 152, 0));

            HFONT hGuiFont = CreateFont(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            SelectObject(hdc, hGuiFont);
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, time_of_day ? RGB(200,200,200) : RGB(50,50,50));
            const char* inst = "Till->Plant->Water->Harvest | Keys: 1-4, Space, H/F1";
            TextOut(hdc, OFFSET_X, 8, inst, lstrlen(inst));

            for (int y = 0; y < GRID_ROWS; y++) {
                for (int x = 0; x < GRID_COLS; x++) {
                    int idx = y * GRID_COLS + x;
                    RECT r = { OFFSET_X + x * CELL_SIZE, OFFSET_Y + y * CELL_SIZE, OFFSET_X + (x+1) * CELL_SIZE, OFFSET_Y + (y+1) * CELL_SIZE };
                    
                    int req = (weather == 2) ? 2 : ((weather == 1) ? 0 : 1);
                    if (grid[idx].type == 0) {
                        FillRect(hdc, &r, hGrass);
                        
                        int prng = (x * 37 + y * 13) % 100;
                        if (prng < 20) {
                            HBRUSH hFlower = CreateSolidBrush(time_of_day ? RGB(200, 200, 200) : RGB(255, 255, 255));
                            HBRUSH hOld = (HBRUSH)SelectObject(hdc, hFlower);
                            SelectObject(hdc, GetStockObject(NULL_PEN));
                            Ellipse(hdc, r.left + 10, r.top + 15, r.left + 14, r.top + 19);
                            Ellipse(hdc, r.left + 16, r.top + 12, r.left + 20, r.top + 16);
                            SelectObject(hdc, hOld);
                            DeleteObject(hFlower);
                        } else if (prng > 80) {
                            HBRUSH hFlower = CreateSolidBrush(time_of_day ? RGB(100, 150, 255) : RGB(150, 200, 255));
                            HBRUSH hOld = (HBRUSH)SelectObject(hdc, hFlower);
                            SelectObject(hdc, GetStockObject(NULL_PEN));
                            Ellipse(hdc, r.left + 28, r.top + 25, r.left + 32, r.top + 29);
                            SelectObject(hdc, hOld);
                            DeleteObject(hFlower);
                        }
                        
                        HBRUSH hBlade = CreateSolidBrush(time_of_day ? RGB(30, 50, 20) : RGB(100, 150, 50));
                        RECT br1 = { r.left + 5, r.top + 5, r.left + 7, r.top + 10 };
                        RECT br2 = { r.left + 25, r.top + 30, r.left + 27, r.top + 36 };
                        FillRect(hdc, &br1, hBlade);
                        FillRect(hdc, &br2, hBlade);
                        DeleteObject(hBlade);
                    } else {
                        if (weather == 1 || grid[idx].watered >= req) FillRect(hdc, &r, hWetSoil);
                        else if (weather == 2 && grid[idx].watered == 1) FillRect(hdc, &r, hDampSoil);
                        else FillRect(hdc, &r, hSoil);
                        
                        // Procedurally generated 3D dirt furrow patterns
                        HPEN hShadow = CreatePen(PS_SOLID, 1, time_of_day ? RGB(20, 10, 5) : RGB(60, 40, 30));
                        HPEN hHighlight = CreatePen(PS_SOLID, 1, time_of_day ? RGB(60, 30, 20) : RGB(120, 90, 70));
                        for (int f = 0; f < 4; f++) {
                            int fy = r.top + 8 + f * 8;
                            SelectObject(hdc, hShadow);
                            MoveToEx(hdc, r.left, fy, NULL); LineTo(hdc, r.right, fy);
                            SelectObject(hdc, hHighlight);
                            MoveToEx(hdc, r.left, fy + 1, NULL); LineTo(hdc, r.right, fy + 1);
                        }
                        DeleteObject(hShadow);
                        DeleteObject(hHighlight);
                    }
                    
                    SelectObject(hdc, hGridPen);
                    SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    Rectangle(hdc, r.left, r.top, r.right, r.bottom);
                    
                    if (grid[idx].type == 2 || grid[idx].type == 3) {
                        int cx = r.left + CELL_SIZE / 2;
                        int cy = r.top + CELL_SIZE / 2;
                        
                        DWORD tick = GetTickCount();
                        if (!time_of_day) {
                            float cycle = (float)(tick % 60000) / 60000.0f;
                            int shadow_dx = (int)(sin(cycle * 3.14159f * 2) * 15.0f);
                            int shadow_dy = (int)(cos(cycle * 3.14159f * 2) * 5.0f);
                            HBRUSH hShadow = CreateSolidBrush(RGB(60, 40, 20));
                            SelectObject(hdc, hShadow);
                            SelectObject(hdc, GetStockObject(NULL_PEN));
                            int shadowSize = (grid[idx].type == 3) ? 12 : (grid[idx].growth > 0 ? 8 : 5);
                            Ellipse(hdc, cx + shadow_dx - shadowSize, r.bottom - 10 + shadow_dy - shadowSize/2, 
                                         cx + shadow_dx + shadowSize, r.bottom - 10 + shadow_dy + shadowSize/2);
                            DeleteObject(hShadow);
                        }

                        SetGraphicsMode(hdc, GM_ADVANCED);
                        float angle = (float)sin(tick * 0.003f + x + y) * 0.15f;
                        float pivotX = (float)cx;
                        float pivotY = (float)(r.bottom - 8);
                        XFORM xForm;
                        xForm.eM11 = (FLOAT)cos(angle);
                        xForm.eM12 = (FLOAT)sin(angle);
                        xForm.eM21 = (FLOAT)-sin(angle);
                        xForm.eM22 = (FLOAT)cos(angle);
                        xForm.eDx = (FLOAT)(pivotX - cos(angle)*pivotX + sin(angle)*pivotY);
                        xForm.eDy = (FLOAT)(pivotY - sin(angle)*pivotX - cos(angle)*pivotY);
                        SetWorldTransform(hdc, &xForm);

                        if (grid[idx].type == 2) {
                            HBRUSH hLeaf = CreateSolidBrush(time_of_day ? RGB(46, 125, 50) : RGB(76, 175, 80));
                            SelectObject(hdc, hSproutPen);
                            MoveToEx(hdc, cx, r.bottom - 8, NULL); LineTo(hdc, cx, r.top + 12);
                            SelectObject(hdc, hLeaf);
                            SelectObject(hdc, GetStockObject(NULL_PEN));
                            
                            if (grid[idx].growth > 0) {
                                Ellipse(hdc, cx - 14, cy - 6, cx + 2, cy + 6);
                                Ellipse(hdc, cx - 2, cy - 14, cx + 14, cy - 2);
                                HBRUSH hBud = CreateSolidBrush(time_of_day ? RGB(200, 64, 129) : RGB(255, 64, 129));
                                SelectObject(hdc, hBud);
                                Ellipse(hdc, cx - 4, cy - 10, cx + 4, cy - 2);
                                HBRUSH hBudLight = CreateSolidBrush(time_of_day ? RGB(248, 187, 208) : RGB(252, 228, 236));
                                SelectObject(hdc, hBudLight);
                                Ellipse(hdc, cx - 7, cy - 13, cx - 1, cy - 7);
                                Ellipse(hdc, cx + 1, cy - 13, cx + 7, cy - 7);
                                Ellipse(hdc, cx - 7, cy - 7, cx - 1, cy - 1);
                                Ellipse(hdc, cx + 1, cy - 7, cx + 7, cy - 1);
                                DeleteObject(hBud);
                                DeleteObject(hBudLight);
                            } else {
                                Ellipse(hdc, cx - 10, cy - 4, cx, cy + 4);
                                Ellipse(hdc, cx, cy - 10, cx + 10, cy - 2);
                            }
                            DeleteObject(hLeaf);
                        } else {
                            if (grid[idx].cropType == 0) { // Wheat
                                HBRUSH hWheatBrush = CreateSolidBrush(time_of_day ? RGB(180, 140, 0) : RGB(255, 193, 7));
                                SelectObject(hdc, hWheatPen);
                                MoveToEx(hdc, cx, r.bottom - 8, NULL); LineTo(hdc, cx, r.top + 8);
                                SelectObject(hdc, GetStockObject(NULL_PEN));
                                SelectObject(hdc, hWheatBrush);
                                for (int i = 0; i < 3; i++) {
                                    Ellipse(hdc, cx - 6, cy - 8 + (i*6), cx, cy - 4 + (i*6));
                                    Ellipse(hdc, cx, cy - 8 + (i*6), cx + 6, cy - 4 + (i*6));
                                }
                                DeleteObject(hWheatBrush);
                            } else if (grid[idx].cropType == 1) { // Corn
                                SelectObject(hdc, hSproutPen);
                                MoveToEx(hdc, cx, r.bottom - 8, NULL); LineTo(hdc, cx, r.top + 4);
                                HBRUSH hYellow = CreateSolidBrush(time_of_day ? RGB(150, 150, 0) : RGB(255, 235, 59));
                                HBRUSH hLeaf = CreateSolidBrush(time_of_day ? RGB(46, 125, 50) : RGB(76, 175, 80));
                                SelectObject(hdc, hYellow);
                                SelectObject(hdc, GetStockObject(NULL_PEN));
                                Ellipse(hdc, cx - 5, cy - 10, cx + 5, cy + 10);
                                SelectObject(hdc, hLeaf);
                                Ellipse(hdc, cx - 7, cy, cx - 3, cy + 12);
                                Ellipse(hdc, cx + 3, cy, cx + 7, cy + 12);
                                DeleteObject(hYellow);
                                DeleteObject(hLeaf);
                            } else if (grid[idx].cropType == 2) { // Tomato
                                SelectObject(hdc, hSproutPen);
                                MoveToEx(hdc, cx, r.bottom - 8, NULL); LineTo(hdc, cx, r.top + 8);
                                HBRUSH hRed = CreateSolidBrush(time_of_day ? RGB(150, 40, 40) : RGB(244, 67, 54));
                                HBRUSH hDarkRed = CreateSolidBrush(time_of_day ? RGB(100, 20, 20) : RGB(211, 47, 47));
                                SelectObject(hdc, hRed);
                                SelectObject(hdc, GetStockObject(NULL_PEN));
                                Ellipse(hdc, cx - 8, cy, cx + 2, cy + 10);
                                Ellipse(hdc, cx + 2, cy - 4, cx + 12, cy + 6);
                                SelectObject(hdc, hDarkRed);
                                Ellipse(hdc, cx - 4, cy - 10, cx + 6, cy);
                                DeleteObject(hRed);
                                DeleteObject(hDarkRed);
                            } else if (grid[idx].cropType == 3) { // Pumpkin
                                SelectObject(hdc, hSproutPen);
                                MoveToEx(hdc, cx, r.bottom - 8, NULL); LineTo(hdc, cx - 6, r.bottom - 8);
                                HBRUSH hOrange = CreateSolidBrush(time_of_day ? RGB(150, 90, 0) : RGB(255, 152, 0));
                                HBRUSH hDarkOrange = CreateSolidBrush(time_of_day ? RGB(120, 60, 0) : RGB(245, 124, 0));
                                SelectObject(hdc, hOrange);
                                SelectObject(hdc, hPumpkinPen);
                                Ellipse(hdc, cx - 14, r.bottom - 22, cx + 14, r.bottom - 2);
                                SelectObject(hdc, hDarkOrange);
                                SelectObject(hdc, GetStockObject(NULL_PEN));
                                Ellipse(hdc, cx - 8, r.bottom - 20, cx + 8, r.bottom - 4);
                                DeleteObject(hOrange);
                                DeleteObject(hDarkOrange);
                            }

                            // Specular sheen sweep on crops
                            HPEN hSheenPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                            SelectObject(hdc, hSheenPen);
                            MoveToEx(hdc, cx - 6, cy - 4, NULL); LineTo(hdc, cx - 2, cy - 8);
                            DeleteObject(hSheenPen);
                        }
                        ModifyWorldTransform(hdc, NULL, MWT_IDENTITY);
                        SetGraphicsMode(hdc, GM_COMPATIBLE);
                    }
                }
            }
            
            DeleteObject(hGrass);
            DeleteObject(hSoil);
            DeleteObject(hWetSoil);
            DeleteObject(hDampSoil);
            DeleteObject(hWheatPen);
            DeleteObject(hCornPen);
            DeleteObject(hPumpkinPen);
            DeleteObject(hSproutPen);
            DeleteObject(hGridPen);

            // Draw Windmill Sprite if owned
            if (has_mill) {
                int wx = OFFSET_X + 20;
                int wy = OFFSET_Y + 20;
                HBRUSH hMillBase = CreateSolidBrush(RGB(141, 110, 99));
                HBRUSH hMillRoof = CreateSolidBrush(RGB(198, 40, 40));
                HBRUSH hMillDoor = CreateSolidBrush(RGB(62, 39, 35));
                POINT pts[4] = { {wx - 10, wy + 16}, {wx + 10, wy + 16}, {wx + 7, wy - 6}, {wx - 7, wy - 6} };
                HBRUSH hOld = (HBRUSH)SelectObject(hdc, hMillBase);
                SelectObject(hdc, GetStockObject(NULL_PEN));
                Polygon(hdc, pts, 4);
                SelectObject(hdc, hMillRoof);
                POINT rpts[3] = { {wx - 9, wy - 6}, {wx, wy - 16}, {wx + 9, wy - 6} };
                Polygon(hdc, rpts, 3);
                SelectObject(hdc, hMillDoor);
                Rectangle(hdc, wx - 3, wy + 8, wx + 3, wy + 16);
                SelectObject(hdc, hOld);
                DeleteObject(hMillBase);
                DeleteObject(hMillRoof);
                DeleteObject(hMillDoor);

                // Rotating windmill sails
                float rotAngle = (float)(GetTickCount() % 10000) / 10000.0f * 6.28318f;
                HPEN hSailPen = CreatePen(PS_SOLID, 2, RGB(93, 64, 55));
                SelectObject(hdc, hSailPen);
                for (int s = 0; s < 4; s++) {
                    float a = rotAngle + s * 1.57079f;
                    int sx = wx + (int)(cos(a) * 16.0f);
                    int sy = (wy - 6) + (int)(sin(a) * 16.0f);
                    MoveToEx(hdc, wx, wy - 6, NULL); LineTo(hdc, sx, sy);
                }
                DeleteObject(hSailPen);
            }

            // Draw Scarecrow Sprite if owned
            if (has_scarecrow) {
                int scx = OFFSET_X + GRID_COLS * CELL_SIZE - 20;
                int scy = OFFSET_Y + 25;
                // Post
                HBRUSH hPostBrush = CreateSolidBrush(RGB(93, 64, 55));
                RECT postR = { scx - 2, scy - 10, scx + 2, scy + 20 };
                FillRect(hdc, &postR, hPostBrush);
                RECT armR = { scx - 12, scy - 4, scx + 12, scy };
                FillRect(hdc, &armR, hPostBrush);
                DeleteObject(hPostBrush);

                // Coat
                HBRUSH hCoat = CreateSolidBrush(RGB(30, 136, 229));
                POINT cpts[4] = { {scx - 7, scy - 4}, {scx + 7, scy - 4}, {scx + 9, scy + 10}, {scx - 9, scy + 10} };
                HBRUSH hOld = (HBRUSH)SelectObject(hdc, hCoat);
                SelectObject(hdc, GetStockObject(NULL_PEN));
                Polygon(hdc, cpts, 4);
                SelectObject(hdc, hOld);
                DeleteObject(hCoat);

                // Straw Head & Hat
                HBRUSH hHead = CreateSolidBrush(RGB(255, 245, 157));
                SelectObject(hdc, hHead);
                Ellipse(hdc, scx - 5, scy - 14, scx + 5, scy - 4);
                DeleteObject(hHead);

                HBRUSH hHat = CreateSolidBrush(RGB(141, 110, 99));
                SelectObject(hdc, hHat);
                Ellipse(hdc, scx - 9, scy - 15, scx + 9, scy - 11);
                RECT hatTop = { scx - 4, scy - 20, scx + 4, scy - 14 };
                FillRect(hdc, &hatTop, hHat);
                DeleteObject(hHat);

                // Red Scarf
                HPEN hScarfPen = CreatePen(PS_SOLID, 2, RGB(229, 57, 53));
                SelectObject(hdc, hScarfPen);
                int scarfWave = (int)(sin(GetTickCount() * 0.006f) * 3.0f);
                MoveToEx(hdc, scx, scy - 4, NULL); LineTo(hdc, scx + 10, scy - 2 + scarfWave);
                DeleteObject(hScarfPen);
            }

            // Draw Roaming Chickens
            int drawCh = (chickens < 3) ? chickens : 3;
            for (int i = 0; i < drawCh; i++) {
                int cx = OFFSET_X + (int)chickens_list[i].x;
                int cy = OFFSET_Y + (int)chickens_list[i].y;
                int dir = chickens_list[i].dir;
                int bob = (int)(sin(GetTickCount() * 0.01f + chickens_list[i].x) * 2.0f);
                int peckOff = (chickens_list[i].peck > 0) ? 3 : 0;

                HBRUSH hChBody = CreateSolidBrush(RGB(255, 255, 255));
                HBRUSH hOld = (HBRUSH)SelectObject(hdc, hChBody);
                SelectObject(hdc, GetStockObject(NULL_PEN));
                Ellipse(hdc, cx - 6, cy + bob - 4, cx + 6, cy + bob + 4);
                Ellipse(hdc, cx + dir*3 - 3, cy + bob - 7 + peckOff, cx + dir*3 + 3, cy + bob - 1 + peckOff);

                HBRUSH hComb = CreateSolidBrush(RGB(229, 57, 53));
                SelectObject(hdc, hComb);
                Ellipse(hdc, cx + dir*3 - 1, cy + bob - 9 + peckOff, cx + dir*3 + 2, cy + bob - 6 + peckOff);
                DeleteObject(hComb);

                HBRUSH hBeak = CreateSolidBrush(RGB(255, 152, 0));
                SelectObject(hdc, hBeak);
                POINT bpts[3] = { {cx + dir*5, cy + bob - 5 + peckOff}, {cx + dir*8, cy + bob - 4 + peckOff}, {cx + dir*5, cy + bob - 3 + peckOff} };
                Polygon(hdc, bpts, 3);
                DeleteObject(hBeak);

                SelectObject(hdc, hOld);
                DeleteObject(hChBody);
            }

            // Draw Roaming Cows
            int drawCw = (cows < 2) ? cows : 2;
            for (int i = 0; i < drawCw; i++) {
                int cx = OFFSET_X + (int)cows_list[i].x;
                int cy = OFFSET_Y + (int)cows_list[i].y;
                int dir = cows_list[i].dir;
                int chewBob = (cows_list[i].chew > 0) ? (int)(sin(GetTickCount() * 0.02f) * 1.5f) : 0;

                HBRUSH hCowBody = CreateSolidBrush(RGB(250, 250, 250));
                HBRUSH hOld = (HBRUSH)SelectObject(hdc, hCowBody);
                SelectObject(hdc, GetStockObject(NULL_PEN));
                RECT bodyR = { cx - 9, cy - 6, cx + 9, cy + 4 };
                Rectangle(hdc, bodyR.left, bodyR.top, bodyR.right, bodyR.bottom);
                // Legs
                Rectangle(hdc, cx - 7, cy + 4, cx - 4, cy + 9);
                Rectangle(hdc, cx + 4, cy + 4, cx + 7, cy + 9);

                // Black patches
                HBRUSH hPatch = CreateSolidBrush(RGB(33, 33, 33));
                SelectObject(hdc, hPatch);
                Ellipse(hdc, cx - 4, cy - 4, cx + 2, cy + 2);
                DeleteObject(hPatch);

                // Head
                SelectObject(hdc, hCowBody);
                RECT headR = { cx + dir*6 - 4, cy - 9 + chewBob, cx + dir*6 + 5, cy + chewBob };
                Rectangle(hdc, headR.left, headR.top, headR.right, headR.bottom);

                // Pink Snout
                HBRUSH hSnout = CreateSolidBrush(RGB(248, 187, 208));
                SelectObject(hdc, hSnout);
                RECT snoutR = { cx + dir*9 - 2, cy - 6 + chewBob, cx + dir*9 + 4, cy + chewBob };
                Rectangle(hdc, snoutR.left, snoutR.top, snoutR.right, snoutR.bottom);
                DeleteObject(hSnout);

                SelectObject(hdc, hOld);
                DeleteObject(hCowBody);
            }

            // Slow scrolling translucent clouds (hatch brush for stylized look)
            HBRUSH hCloudBrush = CreateHatchBrush(HS_BDIAGONAL, time_of_day ? RGB(80, 80, 100) : RGB(255, 255, 255));
            HPEN hCloudPen = CreatePen(PS_NULL, 0, 0);
            SelectObject(hdc, hCloudBrush);
            SelectObject(hdc, hCloudPen);
            SetBkMode(hdc, TRANSPARENT);
            for (int i = 0; i < MAX_CLOUDS; i++) {
                int cx = (int)clouds[i].x;
                int cy = (int)clouds[i].y;
                int s = clouds[i].size;
                Ellipse(hdc, cx - s, cy - s, cx + s, cy + s);
                Ellipse(hdc, cx - (s*1)/10, cy - s, cx + (s*13)/10, cy + (s*4)/10);
                Ellipse(hdc, cx + (s*4)/10, cy - (s*8)/10, cx + s*2, cy + (s*8)/10);
            }
            DeleteObject(hCloudBrush);
            DeleteObject(hCloudPen);

            if (weather == 1 && time_of_day == 0) {
                HBRUSH hRain = CreateSolidBrush(RGB(180, 200, 255));
                for (int i = 0; i < 60; i++) {
                    int rx = OFFSET_X + rand() % (GRID_COLS * CELL_SIZE);
                    int ry = OFFSET_Y + rand() % (GRID_ROWS * CELL_SIZE);
                    RECT rr = {rx, ry, rx + 2, ry + 10 + (rand() % 10)};
                    FillRect(hdc, &rr, hRain);
                }
                DeleteObject(hRain);
            }

            // Draw Seasonal Atmospheric Weather Particles
            if (time_of_day == 0) {
                for (int i = 0; i < MAX_SEASON_PARTICLES; i++) {
                    int px = (int)season_particles[i].x;
                    int py = (int)season_particles[i].y;
                    int psz = season_particles[i].size;
                    if (current_season == 0) { // Spring: Cherry Blossom
                        HBRUSH hPetal = CreateSolidBrush(RGB(255, 182, 193));
                        HBRUSH hOld = (HBRUSH)SelectObject(hdc, hPetal);
                        SelectObject(hdc, GetStockObject(NULL_PEN));
                        Ellipse(hdc, px - psz, py - psz/2, px + psz, py + psz/2);
                        SelectObject(hdc, hOld);
                        DeleteObject(hPetal);
                    } else if (current_season == 1) { // Summer: Golden Sun Motes
                        HBRUSH hMote = CreateSolidBrush(RGB(255, 235, 59));
                        HBRUSH hOld = (HBRUSH)SelectObject(hdc, hMote);
                        SelectObject(hdc, GetStockObject(NULL_PEN));
                        Ellipse(hdc, px - 2, py - 2, px + 2, py + 2);
                        SelectObject(hdc, hOld);
                        DeleteObject(hMote);
                    } else if (current_season == 2) { // Fall: Autumn Leaves
                        HBRUSH hLeaf = CreateSolidBrush((i % 2 == 0) ? RGB(230, 81, 0) : RGB(251, 140, 0));
                        HBRUSH hOld = (HBRUSH)SelectObject(hdc, hLeaf);
                        SelectObject(hdc, GetStockObject(NULL_PEN));
                        POINT lpts[4] = { {px, py - psz}, {px + psz, py}, {px, py + psz}, {px - psz, py} };
                        Polygon(hdc, lpts, 4);
                        SelectObject(hdc, hOld);
                        DeleteObject(hLeaf);
                    } else if (current_season == 3) { // Winter: Snowflakes
                        HBRUSH hSnow = CreateSolidBrush(RGB(255, 255, 255));
                        HBRUSH hOld = (HBRUSH)SelectObject(hdc, hSnow);
                        SelectObject(hdc, GetStockObject(NULL_PEN));
                        Ellipse(hdc, px - 2, py - 2, px + 2, py + 2);
                        SelectObject(hdc, hOld);
                        DeleteObject(hSnow);
                    }
                }
            }

            // Draw Shockwaves
            for (int i = 0; i < MAX_SHOCKWAVES; i++) {
                if (shockwaves[i].life > 0) {
                    HPEN hSWPen = CreatePen(PS_SOLID, 2, shockwaves[i].color);
                    HPEN hOldP = (HPEN)SelectObject(hdc, hSWPen);
                    SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    int r = (int)shockwaves[i].radius;
                    Ellipse(hdc, (int)shockwaves[i].x - r, (int)shockwaves[i].y - (int)(r * 0.75f),
                                 (int)shockwaves[i].x + r, (int)shockwaves[i].y + (int)(r * 0.75f));
                    SelectObject(hdc, hOldP);
                    DeleteObject(hSWPen);
                }
            }

            // Draw 4-Tier Kinematic Particles
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (particles[i].life > 0) {
                    if (particles[i].type == 3) { // Star
                        HBRUSH hPBrush = CreateSolidBrush(particles[i].color);
                        HBRUSH hOld = (HBRUSH)SelectObject(hdc, hPBrush);
                        SelectObject(hdc, GetStockObject(NULL_PEN));
                        Ellipse(hdc, (int)particles[i].x - 3, (int)particles[i].y - 3, (int)particles[i].x + 4, (int)particles[i].y + 4);
                        HPEN hStarPen = CreatePen(PS_SOLID, 1, RGB(255, 215, 0));
                        SelectObject(hdc, hStarPen);
                        MoveToEx(hdc, (int)particles[i].x - 5, (int)particles[i].y, NULL);
                        LineTo(hdc, (int)particles[i].x + 6, (int)particles[i].y);
                        MoveToEx(hdc, (int)particles[i].x, (int)particles[i].y - 5, NULL);
                        LineTo(hdc, (int)particles[i].x, (int)particles[i].y + 6);
                        DeleteObject(hStarPen);
                        SelectObject(hdc, hOld);
                        DeleteObject(hPBrush);
                    } else if (particles[i].type == 2) { // Needle / Spark
                        HPEN hNeedlePen = CreatePen(PS_SOLID, 2, particles[i].color);
                        HPEN hOld = (HPEN)SelectObject(hdc, hNeedlePen);
                        MoveToEx(hdc, (int)particles[i].x, (int)particles[i].y, NULL);
                        LineTo(hdc, (int)(particles[i].x - particles[i].vx * 2.0f), (int)(particles[i].y - particles[i].vy * 2.0f));
                        SelectObject(hdc, hOld);
                        DeleteObject(hNeedlePen);
                    } else if (particles[i].type == 4) { // Smoke / Pollen
                        HBRUSH hSmokeBrush = CreateSolidBrush(particles[i].color);
                        HBRUSH hOld = (HBRUSH)SelectObject(hdc, hSmokeBrush);
                        SelectObject(hdc, GetStockObject(NULL_PEN));
                        int sz = particles[i].size;
                        Ellipse(hdc, (int)particles[i].x - sz, (int)particles[i].y - sz, (int)particles[i].x + sz, (int)particles[i].y + sz);
                        SelectObject(hdc, hOld);
                        DeleteObject(hSmokeBrush);
                    } else if (particles[i].type == 5) { // Shard
                        HBRUSH hShardBrush = CreateSolidBrush(particles[i].color);
                        RECT pr = { (int)particles[i].x - 2, (int)particles[i].y - 2, (int)particles[i].x + particles[i].size, (int)particles[i].y + particles[i].size };
                        FillRect(hdc, &pr, hShardBrush);
                        DeleteObject(hShardBrush);
                    } else {
                        HBRUSH hPBrush = CreateSolidBrush(particles[i].color);
                        RECT pr = { (int)particles[i].x - 2, (int)particles[i].y - 2, (int)particles[i].x + 3, (int)particles[i].y + 3 };
                        FillRect(hdc, &pr, hPBrush);
                        DeleteObject(hPBrush);
                    }
                }
            }

            // Ornate Rustic Corner Filigree L-Brackets and Pulsating Perimeter Inlay Border
            int gridW = GRID_COLS * CELL_SIZE;
            int gridH = GRID_ROWS * CELL_SIZE;
            int left = OFFSET_X;
            int top = OFFSET_Y;
            int right = OFFSET_X + gridW;
            int bottom = OFFSET_Y + gridH;

            // Pulsating golden border
            DWORD bTick = GetTickCount();
            int bPulse = (int)(sin(bTick * 0.004f) * 40.0f + 200.0f);
            HPEN hBorderPen = CreatePen(PS_SOLID, 2, RGB(255, bPulse, 0));
            HPEN hOldBPen = (HPEN)SelectObject(hdc, hBorderPen);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, left - 2, top - 2, right + 2, bottom + 2);
            SelectObject(hdc, hOldBPen);
            DeleteObject(hBorderPen);

            // 4 Corner Filigree L-Brackets
            HPEN hBracketPen = CreatePen(PS_SOLID, 3, RGB(255, 215, 0));
            SelectObject(hdc, hBracketPen);
            int bLen = 18;
            // Top-Left
            MoveToEx(hdc, left - 2, top - 2 + bLen, NULL); LineTo(hdc, left - 2, top - 2); LineTo(hdc, left - 2 + bLen, top - 2);
            // Top-Right
            MoveToEx(hdc, right + 2 - bLen, top - 2, NULL); LineTo(hdc, right + 2, top - 2); LineTo(hdc, right + 2, top - 2 + bLen);
            // Bottom-Left
            MoveToEx(hdc, left - 2, bottom + 2 - bLen, NULL); LineTo(hdc, left - 2, bottom + 2); LineTo(hdc, left - 2 + bLen, bottom + 2);
            // Bottom-Right
            MoveToEx(hdc, right + 2 - bLen, bottom + 2, NULL); LineTo(hdc, right + 2, bottom + 2); LineTo(hdc, right + 2, bottom + 2 - bLen);
            DeleteObject(hBracketPen);

            // Corner Rivets
            HBRUSH hRivet = CreateSolidBrush(RGB(255, 224, 130));
            HBRUSH hOldR = (HBRUSH)SelectObject(hdc, hRivet);
            SelectObject(hdc, GetStockObject(NULL_PEN));
            Ellipse(hdc, left + 4, top + 4, left + 8, top + 8);
            Ellipse(hdc, right - 8, top + 4, right - 4, top + 8);
            Ellipse(hdc, left + 4, bottom - 8, left + 8, bottom - 4);
            Ellipse(hdc, right - 8, bottom - 8, right - 4, bottom - 4);
            SelectObject(hdc, hOldR);
            DeleteObject(hRivet);

            // Traveling Specular Glint along perimeter
            int perimeter = (gridW + gridH) * 2;
            int glintDist = (int)(bTick * 0.22f) % perimeter;
            int gx = 0, gy = 0;
            if (glintDist < gridW) {
                gx = left + glintDist; gy = top - 2;
            } else if (glintDist < gridW + gridH) {
                gx = right + 2; gy = top + (glintDist - gridW);
            } else if (glintDist < gridW * 2 + gridH) {
                gx = right - (glintDist - (gridW + gridH)); gy = bottom + 2;
            } else {
                gx = left - 2; gy = bottom - (glintDist - (gridW * 2 + gridH));
            }
            HBRUSH hGlintBrush = CreateSolidBrush(RGB(255, 255, 255));
            HBRUSH hOldG = (HBRUSH)SelectObject(hdc, hGlintBrush);
            Ellipse(hdc, gx - 4, gy - 4, gx + 4, gy + 4);
            SelectObject(hdc, hOldG);
            DeleteObject(hGlintBrush);

            // Stylized day/night cycle color overlay that slowly shifts based on internal time
            DWORD tick = GetTickCount();
            float cycle = (float)(tick % 60000) / 60000.0f;
            int rOverlay = (int)(sin(cycle * 3.14159f * 2) * 30 + 30);
            int gOverlay = (int)(cos(cycle * 3.14159f * 2) * 20 + 20);
            int bOverlay = (int)(sin(cycle * 3.14159f * 2 + 3.14159f) * 40 + 40);
            
            static HMODULE hMsimg32 = NULL;
            static BOOL msimg32Loaded = FALSE;
            typedef BOOL(WINAPI *AlphaBlend_t)(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION);
            static AlphaBlend_t pAlphaBlend = NULL;
            if (!msimg32Loaded) {
                hMsimg32 = LoadLibrary("msimg32.dll");
                if (hMsimg32) pAlphaBlend = (AlphaBlend_t)GetProcAddress(hMsimg32, "AlphaBlend");
                msimg32Loaded = TRUE;
            }
            
            if (pAlphaBlend) {
                HDC hOverlayDC = CreateCompatibleDC(hdc);
                HBITMAP hOverlayBmp = CreateCompatibleBitmap(hWindowDC, 1, 1);
                SelectObject(hOverlayDC, hOverlayBmp);
                SetPixel(hOverlayDC, 0, 0, RGB(rOverlay, gOverlay, bOverlay));
                BLENDFUNCTION bf = { AC_SRC_OVER, 0, 40, 0 }; // 40 alpha
                pAlphaBlend(hdc, 0, 0, clientRect.right, clientRect.bottom, hOverlayDC, 0, 0, 1, 1, bf);
                DeleteObject(hOverlayBmp);
                DeleteDC(hOverlayDC);
            } else {
                HBRUSH hOverlay = CreateHatchBrush(HS_DIAGCROSS, RGB(rOverlay, gOverlay, bOverlay));
                HBRUSH hOld = (HBRUSH)SelectObject(hdc, hOverlay);
                SetBkMode(hdc, TRANSPARENT);
                FillRect(hdc, &clientRect, hOverlay);
                SelectObject(hdc, hOld);
                DeleteObject(hOverlay);
            }

            SelectObject(hdc, GetStockObject(SYSTEM_FONT));
            DeleteObject(hGuiFont);

            SetMapMode(hdc, MM_TEXT);
            SetViewportOrgEx(hdc, 0, 0, NULL);
            SetWindowOrgEx(hdc, 0, 0, NULL);
            BitBlt(hWindowDC, 0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, hdc, 0, 0, SRCCOPY);
            
            SelectObject(hdc, hOldBmp);
            DeleteObject(hMemBmp);
            DeleteDC(hdc);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    const char CLASS_NAME[]  = "KFarmClass";
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(135, 206, 235));

    RegisterClass(&wc);

    HMODULE hUser32 = GetModuleHandle("user32.dll");
    if (hUser32) {
        typedef BOOL(WINAPI *SetProcessDPIAwareFunc)();
        SetProcessDPIAwareFunc setDPIAware = (SetProcessDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (setDPIAware) setDPIAware();
    }

    RECT rect = {0, 0, S(420), S(590)};
    AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN, FALSE);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KFarm - Spring, Day 1 | Clear | $50 | Ch:0 Co:0", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_F1 || msg.wParam == 'H' || msg.wParam == 'h') {
                SendMessage(hwnd, WM_COMMAND, 14, 0);
                continue;
            }
            if (msg.wParam == VK_SPACE) {
                SendMessage(hwnd, WM_COMMAND, 1, 0);
                continue;
            }
            if (msg.wParam >= '1' && msg.wParam <= '4') {
                int seedCmd = 2 + (msg.wParam - '1');
                SendMessage(hwnd, WM_COMMAND, seedCmd, 0);
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
