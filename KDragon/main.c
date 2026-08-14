#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BTN_INCUBATE 1
#define BTN_FEED     2
#define BTN_PLAY     3
#define BTN_SLEEP    4
#define TIMER_ID     1

int state = 0; // 0 = egg, 1 = dragon
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

HWND btn_incubate, btn_feed, btn_play, btn_sleep, btn_train, btn_hoard, btn_battle, btn_shop;
HWND btn_tr_str, btn_tr_spd, btn_tr_loy, btn_tr_back;
HWND btn_str_hit, btn_spd_react, btn_loy_1, btn_loy_2, btn_loy_3;
HWND btn_bat_atk, btn_bat_def, btn_bat_spec, btn_bat_flee;
HWND btn_shp_food, btn_shp_toy, btn_shp_str, btn_shp_spd, btn_shp_back;
HWND btn_evt_opt1, btn_evt_opt2;
HFONT hFontNormal, hFontLarge;
HBRUSH bgBrush;

int current_event_id = 0;

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

void DrawPixelArt(HDC hdc, int x, int y, int scale, COLORREF pixels[16][16], int element_type) {
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            if (pixels[i][j] != -1) {
                COLORREF c = pixels[i][j];
                if (element_type == 4) { // Water
                    if (c == RGB(180,30,30)) c = RGB(30,60,180);
                    else if (c == RGB(220,60,60)) c = RGB(60,120,220);
                } else if (element_type == 3) { // Earth
                    if (c == RGB(180,30,30)) c = RGB(100,60,30);
                    else if (c == RGB(220,60,60)) c = RGB(150,100,60);
                }
                HBRUSH b = CreateSolidBrush(c);
                RECT r = {x + j*scale, y + i*scale, x + (j+1)*scale, y + (i+1)*scale};
                FillRect(hdc, &r, b);
                DeleteObject(b);
            }
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            srand(GetTickCount());
            hFontNormal = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                     DEFAULT_PITCH | FF_DONTCARE, "Times New Roman");
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
            break;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == BTN_INCUBATE) {
                state = 1;
                add_log("The egg hatched! A baby dragon emerged.");
                ShowWindow(btn_incubate, SW_HIDE);
                ShowWindow(btn_feed, SW_SHOW);
                ShowWindow(btn_play, SW_SHOW);
                ShowWindow(btn_sleep, SW_SHOW);
                ShowWindow(btn_train, SW_SHOW);
                ShowWindow(btn_hoard, SW_SHOW);
                ShowWindow(btn_battle, SW_SHOW);
                ShowWindow(btn_shop, SW_SHOW);
                SetTimer(hwnd, TIMER_ID, 3000, NULL);
                InvalidateRect(hwnd, NULL, TRUE);
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
                }
                InvalidateRect(hwnd, NULL, TRUE);
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
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (LOWORD(wParam) == BTN_SLEEP) {
                sleep_count++;
                energy = energy + 40;
                if (energy > 100) energy = 100;
                hunger = hunger - 10;
                if (hunger < 0) hunger = 0;
                add_log("Dragon took a nap and regained energy.");
                InvalidateRect(hwnd, NULL, TRUE);
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
                    ShowWindow(btn_tr_str, SW_SHOW);
                    ShowWindow(btn_tr_spd, SW_SHOW);
                    ShowWindow(btn_tr_loy, SW_SHOW);
                    ShowWindow(btn_tr_back, SW_SHOW);
                    prev_state = state;
                    state = 3;
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (LOWORD(wParam) == BTN_TR_BACK) {
                ShowWindow(btn_tr_str, SW_HIDE); ShowWindow(btn_tr_spd, SW_HIDE);
                ShowWindow(btn_tr_loy, SW_HIDE); ShowWindow(btn_tr_back, SW_HIDE);
                ShowWindow(btn_feed, SW_SHOW); ShowWindow(btn_play, SW_SHOW);
                ShowWindow(btn_sleep, SW_SHOW); ShowWindow(btn_train, SW_SHOW);
                ShowWindow(btn_hoard, SW_SHOW); ShowWindow(btn_battle, SW_SHOW);
                state = prev_state;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (LOWORD(wParam) == BTN_TR_STR) {
                ShowWindow(btn_tr_str, SW_HIDE); ShowWindow(btn_tr_spd, SW_HIDE);
                ShowWindow(btn_tr_loy, SW_HIDE); ShowWindow(btn_tr_back, SW_HIDE);
                energy -= 15; if (energy < 0) energy = 0;
                hunger -= 5; if (hunger < 0) hunger = 0;
                state = 4;
                minigame_val = 0; minigame_dir = 1;
                ShowWindow(btn_str_hit, SW_SHOW);
                SetTimer(hwnd, 2, 50, NULL);
                InvalidateRect(hwnd, NULL, TRUE);
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
                SetTimer(hwnd, 3, 50, NULL);
                InvalidateRect(hwnd, NULL, TRUE);
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
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (LOWORD(wParam) == BTN_STR_HIT) {
                KillTimer(hwnd, 2);
                int gain = minigame_val / 20;
                strength += gain;
                char logMsg[128];
                sprintf(logMsg, "You hit with power %d! Strength +%d", minigame_val, gain);
                add_log(logMsg);
                ShowWindow(btn_str_hit, SW_HIDE);
                ShowWindow(btn_feed, SW_SHOW); ShowWindow(btn_play, SW_SHOW);
                ShowWindow(btn_sleep, SW_SHOW); ShowWindow(btn_train, SW_SHOW);
                ShowWindow(btn_hoard, SW_SHOW); ShowWindow(btn_battle, SW_SHOW);
                ShowWindow(btn_shop, SW_SHOW);
                state = prev_state;
                InvalidateRect(hwnd, NULL, TRUE);
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
                }
                ShowWindow(btn_spd_react, SW_HIDE);
                ShowWindow(btn_feed, SW_SHOW); ShowWindow(btn_play, SW_SHOW);
                ShowWindow(btn_sleep, SW_SHOW); ShowWindow(btn_train, SW_SHOW);
                ShowWindow(btn_hoard, SW_SHOW); ShowWindow(btn_battle, SW_SHOW);
                ShowWindow(btn_shop, SW_SHOW);
                state = prev_state;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (LOWORD(wParam) >= BTN_LOY_1 && LOWORD(wParam) <= BTN_LOY_3) {
                int picked = LOWORD(wParam) - BTN_LOY_1;
                if (picked == minigame_val) {
                    loyalty += 5;
                    add_log("You found the treat! Loyalty +5");
                } else {
                    loyalty += 1;
                    add_log("Empty box. Loyalty +1");
                }
                ShowWindow(btn_loy_1, SW_HIDE); ShowWindow(btn_loy_2, SW_HIDE); ShowWindow(btn_loy_3, SW_HIDE);
                ShowWindow(btn_feed, SW_SHOW); ShowWindow(btn_play, SW_SHOW);
                ShowWindow(btn_sleep, SW_SHOW); ShowWindow(btn_train, SW_SHOW);
                ShowWindow(btn_hoard, SW_SHOW); ShowWindow(btn_battle, SW_SHOW);
                ShowWindow(btn_shop, SW_SHOW);
                state = prev_state;
                InvalidateRect(hwnd, NULL, TRUE);
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
                    ShowWindow(btn_shop, SW_HIDE);
                    prev_state = state;
                    state = 7;
                    SetTimer(hwnd, 4, 5000, NULL);
                }
                InvalidateRect(hwnd, NULL, TRUE);
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
                    
                    ShowWindow(btn_feed, SW_HIDE); ShowWindow(btn_play, SW_HIDE);
                    ShowWindow(btn_sleep, SW_HIDE); ShowWindow(btn_train, SW_HIDE);
                    ShowWindow(btn_hoard, SW_HIDE); ShowWindow(btn_battle, SW_HIDE);
                    ShowWindow(btn_shop, SW_HIDE);
                    
                    ShowWindow(btn_bat_atk, SW_SHOW);
                    ShowWindow(btn_bat_def, SW_SHOW);
                    if (element != 0) ShowWindow(btn_bat_spec, SW_SHOW);
                    ShowWindow(btn_bat_flee, SW_SHOW);
                    
                    prev_state = state;
                    state = 8;
                }
                InvalidateRect(hwnd, NULL, TRUE);
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
                        } else {
                            add_log("You missed!");
                        }
                    } else if (action == 2) {
                        if (element == 2) { // Fire
                            int dmg = strength * 3 + 10;
                            if (e_def) dmg /= 2;
                            bat_enemy_hp -= dmg;
                            char m[128]; sprintf(m, "You used Fireball! Dealt %d damage.", dmg); add_log(m);
                        } else if (element == 4) { // Water
                            int heal = 30 + loyalty;
                            bat_player_hp += heal;
                            if (bat_player_hp > bat_player_max) bat_player_hp = bat_player_max;
                            char m[128]; sprintf(m, "You used Healing Stream! Restored %d HP.", heal); add_log(m);
                        } else if (element == 3) { // Earth
                            int dmg = strength * 2;
                            if (dmg < 1) dmg = 1;
                            if (e_def) dmg /= 2;
                            bat_enemy_hp -= dmg;
                            bat_enemy_spd -= 5;
                            if (bat_enemy_spd < 1) bat_enemy_spd = 1;
                            char m[128]; sprintf(m, "You used Earthquake! Dealt %d damage and slowed enemy.", dmg); add_log(m);
                        }
                    } else if (action == 1) {
                        add_log("You are defending.");
                    }
                    
                    if (bat_enemy_hp <= 0) {
                        add_log("Enemy dragon defeated!");
                        battle_ended = 1;
                        won = 1;
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
                    ShowWindow(btn_shop, SW_SHOW);
                    
                    state = prev_state;
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (LOWORD(wParam) == BTN_SHOP) {
                if (state == 0) return 0;
                ShowWindow(btn_feed, SW_HIDE); ShowWindow(btn_play, SW_HIDE);
                ShowWindow(btn_sleep, SW_HIDE); ShowWindow(btn_train, SW_HIDE);
                ShowWindow(btn_hoard, SW_HIDE); ShowWindow(btn_battle, SW_HIDE);
                ShowWindow(btn_shop, SW_HIDE);
                
                ShowWindow(btn_shp_food, SW_SHOW);
                ShowWindow(btn_shp_toy, SW_SHOW);
                ShowWindow(btn_shp_str, SW_SHOW);
                ShowWindow(btn_shp_spd, SW_SHOW);
                ShowWindow(btn_shp_back, SW_SHOW);
                
                prev_state = state;
                state = 9;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (LOWORD(wParam) == BTN_SHP_BACK) {
                ShowWindow(btn_shp_food, SW_HIDE); ShowWindow(btn_shp_toy, SW_HIDE);
                ShowWindow(btn_shp_str, SW_HIDE); ShowWindow(btn_shp_spd, SW_HIDE);
                ShowWindow(btn_shp_back, SW_HIDE);
                
                ShowWindow(btn_feed, SW_SHOW); ShowWindow(btn_play, SW_SHOW);
                ShowWindow(btn_sleep, SW_SHOW); ShowWindow(btn_train, SW_SHOW);
                ShowWindow(btn_hoard, SW_SHOW); ShowWindow(btn_battle, SW_SHOW);
                ShowWindow(btn_shop, SW_SHOW);
                
                state = prev_state;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (LOWORD(wParam) == BTN_SHP_FOOD) {
                if (gold >= 20) {
                    gold -= 20;
                    hunger += 50; if (hunger > 100) hunger = 100;
                    energy += 20; if (energy > 100) energy = 100;
                    add_log("Bought Premium Meat! Hunger +50, Energy +20.");
                } else { add_log("Not enough gold."); }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (LOWORD(wParam) == BTN_SHP_TOY) {
                if (gold >= 30) {
                    gold -= 30;
                    happiness += 50; if (happiness > 100) happiness = 100;
                    energy += 10; if (energy > 100) energy = 100;
                    add_log("Bought Mystery Toy! Happiness +50, Energy +10.");
                } else { add_log("Not enough gold."); }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (LOWORD(wParam) == BTN_SHP_STR) {
                if (gold >= 50) {
                    gold -= 50;
                    strength += 5;
                    add_log("Bought Power Bracer! Strength +5.");
                } else { add_log("Not enough gold."); }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (LOWORD(wParam) == BTN_SHP_SPD) {
                if (gold >= 50) {
                    gold -= 50;
                    speed += 5;
                    add_log("Bought Swift Boots! Speed +5.");
                } else { add_log("Not enough gold."); }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (LOWORD(wParam) == BTN_EVT_OPT1 || LOWORD(wParam) == BTN_EVT_OPT2) {
                int opt = LOWORD(wParam) == BTN_EVT_OPT1 ? 1 : 2;
                if (current_event_id == 0) {
                    if (opt == 1) {
                        if (gold >= 20) {
                            gold -= 20; happiness += 10; if (happiness>100) happiness=100;
                            add_log("Medicine worked! Dragon is feeling better.");
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
                        } else {
                            energy -= 20; if (energy<0) energy=0;
                            happiness -= 10; if (happiness<0) happiness=0;
                            add_log("The beast attacked! Dragon lost energy.");
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
                ShowWindow(btn_shop, SW_SHOW);
                state = prev_state;
                InvalidateRect(hwnd, NULL, TRUE);
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
                        ShowWindow(btn_shop, SW_HIDE);
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
                        InvalidateRect(hwnd, NULL, TRUE);
                        return 0;
                    }
                }
                
                if (state == 1 && age >= 10) {
                    state = 2;
                    int max_c = feed_count;
                    if (play_count > max_c) max_c = play_count;
                    if (sleep_count > max_c) max_c = sleep_count;
                    
                    char msg[128];
                    if (max_c == feed_count) {
                        element = 3;
                        strcpy(msg, "Your baby dragon evolved into an ADULT EARTH DRAGON!");
                    } else if (max_c == play_count) {
                        element = 2;
                        strcpy(msg, "Your baby dragon evolved into an ADULT FIRE DRAGON!");
                    } else {
                        element = 4;
                        strcpy(msg, "Your baby dragon evolved into an ADULT WATER DRAGON!");
                    }
                    add_log(msg);
                }
                
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (wParam == 2) { // str minigame
                if (minigame_dir == 1) {
                    minigame_val += 5;
                    if (minigame_val >= 100) minigame_dir = -1;
                } else {
                    minigame_val -= 5;
                    if (minigame_val <= 0) minigame_dir = 1;
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (wParam == 3) { // spd minigame
                if (minigame_state == 0 && GetTickCount() >= minigame_start_time) {
                    minigame_state = 1;
                    minigame_start_time = GetTickCount();
                    InvalidateRect(hwnd, NULL, TRUE);
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
                ShowWindow(btn_shop, SW_SHOW);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            FillRect(hdc, &ps.rcPaint, bgBrush);
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(42, 23, 4));
            
            SelectObject(hdc, hFontNormal);
            
            if (state == 0) {
                const char* msg = "A mysterious egg awaits...";
                TextOut(hdc, 190, 80, msg, strlen(msg));
                
                DrawPixelArt(hdc, 236, 110, 8, egg_pixels, 0);
            } else {
                char buf[256];
                const char* type_str = "None";
                if (element == 2) type_str = "Fire";
                else if (element == 3) type_str = "Earth";
                else if (element == 4) type_str = "Water";
                sprintf(buf, "Hunger: %d  Happiness: %d  Energy: %d  Age: %d\nType: %s  Str: %d  Spd: %d  Loy: %d  Gold: %d", 
                        hunger, happiness, energy, age, type_str, strength, speed, loyalty, gold);
                
                RECT r = {0, 10, 600, 60};
                DrawText(hdc, buf, strlen(buf), &r, DT_CENTER | DT_TOP);
                
                if (state == 1 || (state > 2 && prev_state == 1)) {
                    DrawPixelArt(hdc, 236, 90, 8, dragon_pixels, 0);
                } else if (state == 2 || (state > 2 && prev_state == 2)) {
                    DrawPixelArt(hdc, 236, 90, 8, adult_dragon_pixels, element);
                }
                
                if (state == 4) {
                    const char* title = "Power:";
                    TextOut(hdc, 190, 235, title, strlen(title));
                    HBRUSH barBrush = CreateSolidBrush(RGB(200, 50, 50));
                    HBRUSH bgBarBrush = CreateSolidBrush(RGB(255, 255, 255));
                    RECT barBg = {260, 235, 460, 255};
                    FillRect(hdc, &barBg, bgBarBrush);
                    RECT barFg = {260, 235, 260 + minigame_val * 2, 255};
                    FillRect(hdc, &barFg, barBrush);
                    DeleteObject(barBrush);
                    DeleteObject(bgBarBrush);
                } else if (state == 5) {
                    const char* text = minigame_state == 0 ? "Wait for it..." : "NOW!";
                    SetTextColor(hdc, minigame_state == 0 ? RGB(100, 100, 100) : RGB(200, 0, 0));
                    TextOut(hdc, 250, 235, text, strlen(text));
                    SetTextColor(hdc, RGB(42, 23, 4));
                } else if (state == 6) {
                    const char* text = "Find the treat!";
                    TextOut(hdc, 240, 235, text, strlen(text));
                } else if (state == 8) {
                    const char* text = "BATTLE!";
                    SetTextColor(hdc, RGB(200, 0, 0));
                    TextOut(hdc, 260, 235, text, strlen(text));
                    SetTextColor(hdc, RGB(42, 23, 4));
                } else if (state == 10) {
                    const char* text = "";
                    if (current_event_id == 0) text = "Your dragon looks sick and feverish.";
                    else if (current_event_id == 1) text = "A wild beast is approaching the lair!";
                    else if (current_event_id == 2) text = "A wandering merchant offers a mystery eggshell.";
                    SetTextColor(hdc, RGB(200, 0, 0));
                    TextOut(hdc, 150, 235, text, strlen(text));
                    SetTextColor(hdc, RGB(42, 23, 4));
                }
                
                SelectObject(hdc, hFontNormal);
                // Draw log box
                HBRUSH logBrush = CreateSolidBrush(RGB(235, 208, 165));
                RECT logRect = {20, 320, 560, 420};
                FillRect(hdc, &logRect, logBrush);
                DeleteObject(logBrush);
                
                SetTextColor(hdc, RGB(42, 23, 4));
                for (int i = 0; i < log_count; i++) {
                    char logStr[150];
                    sprintf(logStr, "> %s", log_messages[i]);
                    TextOut(hdc, 30, 330 + i * 20, logStr, strlen(logStr));
                }
            }
            
            EndPaint(hwnd, &ps);
            break;
        }

        case WM_CTLCOLORBTN: {
            HDC hdcBtn = (HDC)wParam;
            SetBkColor(hdcBtn, RGB(139, 90, 43));
            SetTextColor(hdcBtn, RGB(42, 23, 4));
            return (LRESULT)GetStockObject(WHITE_BRUSH); // simple dark theme button
        }

        case WM_DESTROY:
            KillTimer(hwnd, TIMER_ID);
            DeleteObject(hFontNormal);
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
    const char CLASS_NAME[]  = "KDragonClass";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KDragon",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 500,
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
