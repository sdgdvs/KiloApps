#include <windows.h>

#define W 80
#define H 22
#define UI_H 4
#define TOTAL_H (H + UI_H)

#define C_WALL RGB(150,150,150)
#define C_FLOOR RGB(80,80,80)
#define C_FOG RGB(40,40,40)
#define C_PLAYER RGB(255,255,255)
#define C_RAT RGB(180,180,0)
#define C_BAT RGB(200,100,0)
#define C_SLIME RGB(0,255,100)
#define C_GOBLIN RGB(100,200,50)
#define C_ORC RGB(0,255,0)
#define C_SPIDER RGB(150,150,150)
#define C_ZOMBIE RGB(50,150,100)
#define C_KOBOLD RGB(255,150,50)
#define C_TROLL RGB(0,180,0)
#define C_MINOTAUR RGB(200,100,50)
#define C_WRAITH RGB(200,200,255)
#define C_ELF RGB(150,50,255)
#define C_DRAGON RGB(255,50,50)
#define C_VAMPIRE RGB(255,0,0)
#define C_GOLEM RGB(180,180,200)
#define C_LICH RGB(255,255,0)
#define C_SHOPKEEPER RGB(255,215,0)
#define C_SKELETON RGB(220, 220, 220)
#define C_DEMON RGB(200, 0, 50)
#define C_GHOST RGB(150, 200, 255)
#define C_HYDRA RGB(50, 200, 100)
#define C_CUBE  RGB(0, 255, 255)
#define C_BALROG RGB(255, 69, 0)
#define C_TITAN RGB(210, 180, 140)
#define C_BEHOLDER RGB(138, 43, 226)
#define C_MINDFLAYER RGB(148, 0, 211)
#define C_ARCHMAGE RGB(0, 220, 255)
#define C_LICHLORD RGB(255, 255, 100)
#define C_ASSASSIN RGB(120, 50, 180)
#define C_BEHEMOTH RGB(40, 40, 90)
#define C_MIMIC RGB(255, 215, 0)
#define C_ASTAROTH RGB(255, 215, 0)

#define C_POTION RGB(255,50,255)
#define C_WEAPON RGB(50,255,255)
#define C_ARMOR RGB(200,200,200)
#define C_STAIRS RGB(255,255,0)
#define C_DOOR RGB(139,69,19)

#define B_NORMAL 0
#define B_FAST 1
#define B_SLUGGISH 2
#define B_ERRATIC 3

#define RACE_HUMAN 0
#define RACE_ELF 1
#define RACE_DWARF 2

#define CLASS_FIGHTER 0
#define CLASS_ROGUE 1
#define CLASS_WIZARD 2
#define B_COWARD 4
#define B_SMART 5
#define B_SHOPKEEPER 6

typedef struct {
    char ch;
    COLORREF fg;
    int walkable;
    int transparent;
    int explored;
    int visible;
    int has_trap;
} Tile;

#define STATUS_NONE 0
#define STATUS_POISON 1
#define STATUS_ROOTED 2
#define STATUS_INVISIBLE 3

#define ABILITY_NONE 0
#define ABILITY_SPLIT 1
#define ABILITY_BREATHE_FIRE 2
#define ABILITY_SUMMON 3

typedef struct {
    int active;
    int x, y;
    char ch;
    COLORREF fg;
    char name[32];
    int type; // 1=heal, 2=weapon, 3=armor, 4=shield, 7=gold
    int subtype; // 0=sword, 1=hammer, 2=spear, 3=bow
    int val;
} Item;

typedef struct {
    int active;
    int x, y;
    char ch;
    COLORREF fg;
    char name[32];
    int hp, max_hp;
    int mp, max_mp;
    int str, dex, tou, int_stat, wil;
    int atk, def;
    int xp;
    int level;
    int behavior;
    int turn_parity;
    int race;
    int class_id;
    int status_effect;
    int status_duration;
    int special_ability;
    int gold;
    Item shop_inventory[3];
    int hunger;
} Entity;

#define TYPE_HEAL 1
#define TYPE_WEAPON 2
#define TYPE_ARMOR 3
#define TYPE_SHIELD 4
#define TYPE_RING 5
#define TYPE_SPELLBOOK 6
#define TYPE_GOLD 7
#define TYPE_SHRINE 8
#define TYPE_FOOD 9

#define W_SWORD 0
#define W_HAMMER 1
#define W_SPEAR 2
#define W_BOW 3

#define MAX_ENTITIES 200
#define MAX_ITEMS 200
#define MAX_INVENTORY 26
#define MAX_MSGS 50
#define MAX_SPELLS 10

#define S_MISSILE 0
#define S_HEAL 1
#define S_FIREBALL 2
#define S_FREEZE 3
#define S_LIGHTNING 4
#define S_METEOR 5
#define S_INVISIBILITY 6
#define S_BLESSING 7

typedef struct {
    Tile map[H][W];
    Entity entities[MAX_ENTITIES];
    Item items[MAX_ITEMS];
    Item inventory[MAX_INVENTORY];
    Item equip_weapon;
    Item equip_shield;
    Item equip_armor;
    Item equip_ring;
    int target_x, target_y;
    int dlevel;
    char msgs[MAX_MSGS][100];
    int stair_x, stair_y;
    int state; // 0=play, 1=dead, 2=inventory, 3=targeting, 4=create_char, 5=char_sheet, 6=spells, 7=help, 8=msg_log
    int targeting_mode; // 0=bow, 1=spell
    int char_race;
    int char_class;
    int seed;
    int known_spells[MAX_SPELLS];
    int active_spell;
    Entity* active_shopkeeper;
    int difficulty; // 0=Normal, 1=Hard, 2=Nightmare
    int total_kills;
} GameState;

GameState g;
HWND g_hwnd;
HFONT g_font;
int char_w = 12, char_h = 20;

void calc_stats(Entity* e);
void generate_random_item(Item* it);

// Minimal LCG Random
unsigned int g_seed = 12345;
void rand_seed(unsigned int s) { g_seed = s; }
unsigned int rand_next() {
    g_seed = (1103515245 * g_seed + 12345) % 2147483648;
    return g_seed;
}
int rand_range(int min, int max) {
    if (max <= min) return min;
    return min + (rand_next() % (max - min + 1));
}

#pragma function(memset)
void* __cdecl memset(void* dest, int c, unsigned int count) {
    char* p = (char*)dest;
    while(count--) *p++ = (char)c;
    return dest;
}

// Minimal String Functions to avoid CRT
int str_len(const char* s) { int l=0; while(s[l]) l++; return l; }
void str_cpy(char* d, const char* s) { while(*s) *d++ = *s++; *d = 0; }
void str_cat(char* d, const char* s) { while(*d) d++; while(*s) *d++ = *s++; *d = 0; }
int str_cmp(const char* a, const char* b) {
    while(*a && *a == *b) { a++; b++; }
    return *a - *b;
}

void add_msg(const char* msg) {
    for (int i = 0; i < MAX_MSGS - 1; i++) {
        str_cpy(g.msgs[i], g.msgs[i+1]);
    }
    str_cpy(g.msgs[MAX_MSGS - 1], msg);
}

void fmt_msg(const char* fmt, const char* s1, int d1, int d2) {
    char buf[128];
    wsprintfA(buf, fmt, s1, d1, d2);
    add_msg(buf);
}

Entity* get_player() { return &g.entities[0]; }

// Distance and FOV
int dist2(int x1, int y1, int x2, int y2) {
    return (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2);
}

// Simple Bresenham Line for FOV
void do_ray(int x0, int y0, int x1, int y1) {
    int dx = x1 - x0; if (dx < 0) dx = -dx;
    int dy = y1 - y0; if (dy < 0) dy = -dy;
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    int e2;
    
    while(1) {
        if(x0>=0 && x0<W && y0>=0 && y0<H) {
            g.map[y0][x0].visible = 1;
            g.map[y0][x0].explored = 1;
            if(!g.map[y0][x0].transparent) break;
        } else break;
        if (x0 == x1 && y0 == y1) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

void calc_fov_bresenham() {
    for(int y=0; y<H; y++) for(int x=0; x<W; x++) g.map[y][x].visible = 0;
    Entity* p = get_player();
    
    for(int x=0; x<W; x++) { do_ray(p->x, p->y, x, 0); do_ray(p->x, p->y, x, H-1); }
    for(int y=0; y<H; y++) { do_ray(p->x, p->y, 0, y); do_ray(p->x, p->y, W-1, y); }
}

void spawn_monster(int x, int y) {
    for(int i=1; i<MAX_ENTITIES; i++) {
        if(!g.entities[i].active) {
            Entity* e = &g.entities[i];
            e->active = 1;
            e->x = x; e->y = y;
            int max_tier = 1;
            if(g.dlevel >= 3) max_tier = 2;
            if(g.dlevel >= 7) max_tier = 3;
            if(g.dlevel >= 12) max_tier = 4;
            if(g.dlevel >= 18) max_tier = 5;
            if(g.dlevel >= 24) max_tier = 6;
            
            if(rand_range(0, 100) < 5) {
                e->ch = '$'; e->fg = C_SHOPKEEPER; str_cpy(e->name, "Shopkeeper");
                e->hp = e->max_hp = 200; e->atk = 25; e->def = 10; e->xp = 100;
                e->behavior = B_SHOPKEEPER; e->special_ability = ABILITY_NONE;
                for(int j=0; j<3; j++) {
                    generate_random_item(&e->shop_inventory[j]);
                }
                break;
            }
            
            int tier = rand_range(1, max_tier);
            int m = rand_range(0, 5);
            
            if(tier == 1) {
                if(m == 0) { e->ch = 'r'; e->fg = C_RAT; str_cpy(e->name, "Giant Rat"); e->hp = e->max_hp = 5 + g.dlevel; e->atk = 2; e->def = 1; e->xp = 2; e->behavior = B_NORMAL; }
                else if(m == 1) { e->ch = 'b'; e->fg = C_BAT; str_cpy(e->name, "Giant Bat"); e->hp = e->max_hp = 3 + g.dlevel; e->atk = 1; e->def = 1; e->xp = 3; e->behavior = B_FAST; }
                else if(m == 2) { e->ch = 's'; e->fg = C_SLIME; str_cpy(e->name, "Slime"); e->hp = e->max_hp = 10 + g.dlevel; e->atk = 2; e->def = 3; e->xp = 3; e->behavior = B_SLUGGISH; e->special_ability = ABILITY_SPLIT; }
                else if(m == 3) { e->ch = 'g'; e->fg = C_GOBLIN; str_cpy(e->name, "Goblin"); e->hp = e->max_hp = 6 + g.dlevel; e->atk = 3; e->def = 1; e->xp = 2; e->behavior = B_COWARD; }
                else { e->ch = 'x'; e->fg = C_SKELETON; str_cpy(e->name, "Skeleton"); e->hp = e->max_hp = 8 + g.dlevel; e->atk = 3; e->def = 1; e->xp = 3; e->behavior = B_NORMAL; }
            } else if(tier == 2) {
                if(m == 0) { e->ch = 'o'; e->fg = C_ORC; str_cpy(e->name, "Orc"); e->hp = e->max_hp = 12 + g.dlevel*2; e->atk = 4; e->def = 2; e->xp = 5; e->behavior = B_NORMAL; }
                else if(m == 1) { e->ch = 'S'; e->fg = C_SPIDER; str_cpy(e->name, "Giant Spider"); e->hp = e->max_hp = 10 + g.dlevel*2; e->atk = 4; e->def = 1; e->xp = 6; e->behavior = B_FAST; }
                else if(m == 2) { e->ch = 'z'; e->fg = C_ZOMBIE; str_cpy(e->name, "Zombie"); e->hp = e->max_hp = 25 + g.dlevel*2; e->atk = 5; e->def = 1; e->xp = 8; e->behavior = B_SLUGGISH; }
                else if(m == 3) { e->ch = 'k'; e->fg = C_KOBOLD; str_cpy(e->name, "Kobold"); e->hp = e->max_hp = 8 + g.dlevel*2; e->atk = 3; e->def = 1; e->xp = 4; e->behavior = B_COWARD; }
                else if(m == 4) { e->ch = 'c'; e->fg = C_GOBLIN; str_cpy(e->name, "Centipede"); e->hp = e->max_hp = 12 + g.dlevel*2; e->atk = 4; e->def = 2; e->xp = 5; e->behavior = B_ERRATIC; }
                else { e->ch = 'G'; e->fg = C_GHOST; str_cpy(e->name, "Ghost"); e->hp = e->max_hp = 15 + g.dlevel*2; e->atk = 6; e->def = 2; e->xp = 10; e->behavior = B_ERRATIC; }
            } else if(tier == 3) {
                if(m == 0) { e->ch = 'T'; e->fg = C_TROLL; str_cpy(e->name, "Cave Troll"); e->hp = e->max_hp = 30 + g.dlevel*3; e->atk = 8; e->def = 4; e->xp = 15; e->behavior = B_NORMAL; }
                else if(m == 1) { e->ch = 'M'; e->fg = C_MINOTAUR; str_cpy(e->name, "Minotaur"); e->hp = e->max_hp = 25 + g.dlevel*3; e->atk = 10; e->def = 3; e->xp = 20; e->behavior = B_FAST; }
                else if(m == 2) { e->ch = 'W'; e->fg = C_WRAITH; str_cpy(e->name, "Wraith"); e->hp = e->max_hp = 20 + g.dlevel*3; e->atk = 9; e->def = 5; e->xp = 18; e->behavior = B_ERRATIC; }
                else if(m == 3) { e->ch = 'e'; e->fg = C_ELF; str_cpy(e->name, "Dark Elf"); e->hp = e->max_hp = 20 + g.dlevel*3; e->atk = 12; e->def = 3; e->xp = 16; e->behavior = B_SMART; }
                else if(m == 4) { e->ch = 'y'; e->fg = C_SKELETON; str_cpy(e->name, "Gargoyle"); e->hp = e->max_hp = 35 + g.dlevel*3; e->atk = 7; e->def = 8; e->xp = 16; e->behavior = B_SLUGGISH; }
                else { e->ch = 'C'; e->fg = C_CUBE; str_cpy(e->name, "Gelatinous Cube"); e->hp = e->max_hp = 50 + g.dlevel*3; e->atk = 5; e->def = 10; e->xp = 25; e->behavior = B_SLUGGISH; e->special_ability = ABILITY_SPLIT; }
            } else if(tier == 4) {
                int m4 = rand_range(0, 5);
                if(m4 == 0) { e->ch = 'D'; e->fg = C_DRAGON; str_cpy(e->name, "Red Dragon"); e->hp = e->max_hp = 60 + g.dlevel*5; e->atk = 15; e->def = 8; e->xp = 50; e->behavior = B_NORMAL; e->special_ability = ABILITY_BREATHE_FIRE; }
                else if(m4 == 1) { e->ch = 'V'; e->fg = C_VAMPIRE; str_cpy(e->name, "Vampire"); e->hp = e->max_hp = 45 + g.dlevel*5; e->atk = 14; e->def = 6; e->xp = 45; e->behavior = B_FAST; }
                else if(m4 == 2) { e->ch = 'G'; e->fg = C_GOLEM; str_cpy(e->name, "Stone Golem"); e->hp = e->max_hp = 80 + g.dlevel*5; e->atk = 10; e->def = 15; e->xp = 60; e->behavior = B_SLUGGISH; }
                else if(m4 == 3) { e->ch = 'L'; e->fg = C_LICH; str_cpy(e->name, "Lich"); e->hp = e->max_hp = 40 + g.dlevel*5; e->atk = 20; e->def = 5; e->xp = 80; e->behavior = B_SMART; e->special_ability = ABILITY_SUMMON; }
                else if(m4 == 4) { e->ch = 'd'; e->fg = C_DEMON; str_cpy(e->name, "Demon"); e->hp = e->max_hp = 50 + g.dlevel*5; e->atk = 18; e->def = 10; e->xp = 70; e->behavior = B_FAST; e->special_ability = ABILITY_BREATHE_FIRE; }
                else { e->ch = 'H'; e->fg = C_HYDRA; str_cpy(e->name, "Hydra"); e->hp = e->max_hp = 90 + g.dlevel*5; e->atk = 25; e->def = 8; e->xp = 100; e->behavior = B_NORMAL; }
            } else if(tier == 5) {
                int m5 = rand_range(0, 5);
                if(m5 == 0) { e->ch = 'B'; e->fg = C_BALROG; str_cpy(e->name, "Balrog"); e->hp = e->max_hp = 100 + g.dlevel*5; e->atk = 30; e->def = 12; e->xp = 150; e->behavior = B_SMART; e->special_ability = ABILITY_BREATHE_FIRE; }
                else if(m5 == 1) { e->ch = 't'; e->fg = C_TITAN; str_cpy(e->name, "Titan"); e->hp = e->max_hp = 120 + g.dlevel*5; e->atk = 35; e->def = 15; e->xp = 200; e->behavior = B_NORMAL; }
                else if(m5 == 2) { e->ch = 'E'; e->fg = C_BEHOLDER; str_cpy(e->name, "Beholder"); e->hp = e->max_hp = 70 + g.dlevel*5; e->atk = 25; e->def = 8; e->xp = 180; e->behavior = B_ERRATIC; e->special_ability = ABILITY_SUMMON; }
                else if(m5 == 3) { e->ch = 'M'; e->fg = C_MINDFLAYER; str_cpy(e->name, "Mind Flayer"); e->hp = e->max_hp = 60 + g.dlevel*5; e->atk = 20; e->def = 5; e->xp = 160; e->behavior = B_SMART; e->special_ability = ABILITY_SUMMON; }
                else if(m5 == 4) { e->ch = 'a'; e->fg = C_ASSASSIN; str_cpy(e->name, "Shadow Assassin"); e->hp = e->max_hp = 45 + g.dlevel*4; e->atk = 35; e->def = 4; e->xp = 140; e->behavior = B_FAST; }
                else { e->ch = 'U'; e->fg = C_BEHEMOTH; str_cpy(e->name, "Shadow Behemoth"); e->hp = e->max_hp = 140 + g.dlevel*6; e->atk = 40; e->def = 16; e->xp = 300; e->behavior = B_NORMAL; }
            } else {
                int m6 = rand_range(0, 6);
                if(m6 == 0) { e->ch = 'A'; e->fg = C_ARCHMAGE; str_cpy(e->name, "Arch-Mage"); e->hp = e->max_hp = 70 + g.dlevel*5; e->atk = 28; e->def = 8; e->xp = 220; e->behavior = B_SMART; e->special_ability = ABILITY_SUMMON; }
                else if(m6 == 1) { e->ch = 'L'; e->fg = C_LICHLORD; str_cpy(e->name, "Lich Lord"); e->hp = e->max_hp = 110 + g.dlevel*6; e->atk = 32; e->def = 12; e->xp = 350; e->behavior = B_SMART; e->special_ability = ABILITY_SUMMON; }
                else if(m6 == 2) { e->ch = 'm'; e->fg = C_MIMIC; str_cpy(e->name, "Mimic Chest"); e->hp = e->max_hp = 80 + g.dlevel*4; e->atk = 26; e->def = 10; e->xp = 180; e->behavior = B_NORMAL; }
                else if(m6 == 3) { e->ch = 'a'; e->fg = C_ASSASSIN; str_cpy(e->name, "Shadow Assassin"); e->hp = e->max_hp = 50 + g.dlevel*4; e->atk = 36; e->def = 5; e->xp = 160; e->behavior = B_FAST; }
                else if(m6 == 4) { e->ch = 'U'; e->fg = C_BEHEMOTH; str_cpy(e->name, "Shadow Behemoth"); e->hp = e->max_hp = 160 + g.dlevel*6; e->atk = 45; e->def = 18; e->xp = 350; e->behavior = B_NORMAL; }
                else if(m6 == 5) { e->ch = 'B'; e->fg = C_BALROG; str_cpy(e->name, "Balrog"); e->hp = e->max_hp = 120 + g.dlevel*5; e->atk = 34; e->def = 14; e->xp = 250; e->behavior = B_SMART; e->special_ability = ABILITY_BREATHE_FIRE; }
                else { e->ch = 't'; e->fg = C_TITAN; str_cpy(e->name, "Titan"); e->hp = e->max_hp = 150 + g.dlevel*5; e->atk = 40; e->def = 18; e->xp = 300; e->behavior = B_NORMAL; }
            }
            
            if(e->behavior != B_SHOPKEEPER) {
                if(g.difficulty == 1) { e->max_hp = e->max_hp * 3 / 2; e->atk = e->atk * 3 / 2; }
                else if(g.difficulty == 2) { e->max_hp = e->max_hp * 2; e->atk = e->atk * 2; e->def = e->def * 3 / 2; }
                e->hp = e->max_hp;
            }
            break;
        }
    }
}

void generate_random_item(Item* it) {
    int r = rand_range(0, 100);
    if(r < 25) {
        it->ch = '!'; it->fg = C_POTION; it->type = TYPE_HEAL;
        it->val = 15 + g.dlevel * 5;
        str_cpy(it->name, "Healing Potion");
    } else if(r < 40) {
        it->ch = '%'; it->fg = RGB(139, 69, 19); it->type = TYPE_FOOD;
        it->val = 400;
        str_cpy(it->name, "Food Ration");
    } else if(r < 70) {
        it->ch = '('; it->fg = C_WEAPON; it->type = TYPE_WEAPON;
        int w_type = rand_range(0, 3);
        it->subtype = w_type;
        it->val = 2 + g.dlevel + rand_range(0, 3);
        if(w_type == W_SWORD) str_cpy(it->name, "Sword");
        else if(w_type == W_HAMMER) str_cpy(it->name, "Warhammer");
        else if(w_type == W_SPEAR) str_cpy(it->name, "Spear");
        else if(w_type == W_BOW) str_cpy(it->name, "Longbow");
    } else if(r < 85) {
        it->ch = ']'; it->fg = C_ARMOR; it->type = TYPE_ARMOR;
        it->val = 1 + g.dlevel + rand_range(0, 2);
        int a_type = rand_range(0, 2);
        if(a_type == 0) str_cpy(it->name, "Leather Armor");
        else if(a_type == 1) str_cpy(it->name, "Chainmail");
        else str_cpy(it->name, "Platemail");
    } else if(r < 95) {
        it->ch = '['; it->fg = C_ARMOR; it->type = TYPE_SHIELD;
        it->val = 1 + g.dlevel/2 + rand_range(0, 1);
        int s_type = rand_range(0, 1);
        if(s_type == 0) str_cpy(it->name, "Wooden Shield");
        else str_cpy(it->name, "Kiteshield");
    } else if (r < 97) {
        it->ch = '='; it->fg = RGB(255,215,0); it->type = TYPE_RING;
        it->subtype = rand_range(0, 2);
        if(it->subtype == 0) { str_cpy(it->name, "Ring of Strength"); it->val = 2 + g.dlevel/2; }
        else if(it->subtype == 1) { str_cpy(it->name, "Ring of Health"); it->val = 10 + g.dlevel * 5; }
        else { str_cpy(it->name, "Amulet of Life"); it->val = 20 + g.dlevel * 5; it->ch = '"'; }
    } else {
        it->ch = '?'; it->fg = RGB(200, 100, 255); it->type = TYPE_SPELLBOOK;
        it->subtype = rand_range(0, 7);
        if(it->subtype == S_MISSILE) { str_cpy(it->name, "Spellbook of Magic Missile"); }
        else if(it->subtype == S_HEAL) { str_cpy(it->name, "Spellbook of Healing"); }
        else if(it->subtype == S_FIREBALL) { str_cpy(it->name, "Spellbook of Fireball"); }
        else if(it->subtype == S_FREEZE) { str_cpy(it->name, "Spellbook of Ice Storm"); }
        else if(it->subtype == S_LIGHTNING) { str_cpy(it->name, "Spellbook of Lightning"); }
        else if(it->subtype == S_METEOR) { str_cpy(it->name, "Spellbook of Meteor Strike"); }
        else if(it->subtype == S_INVISIBILITY) { str_cpy(it->name, "Spellbook of Invisibility"); }
        else { str_cpy(it->name, "Spellbook of Divine Blessing"); }
    }
}

void spawn_item(int x, int y) {
    for(int i=0; i<MAX_ITEMS; i++) {
        if(!g.items[i].active) {
            Item* it = &g.items[i];
            it->active = 1;
            it->x = x; it->y = y;
            generate_random_item(it);
            break;
        }
    }
}

void carve_room(int x, int y, int w, int h) {
    for(int ry=y; ry<y+h; ry++) {
        for(int rx=x; rx<x+w; rx++) {
            g.map[ry][rx].ch = '.';
            g.map[ry][rx].fg = C_FLOOR;
            g.map[ry][rx].walkable = 1;
            g.map[ry][rx].transparent = 1;
        }
    }
}

void dig_corridor(int x1, int y1, int x2, int y2) {
    int x = x1, y = y1;
    int dx = (x2 > x1) ? 1 : (x2 < x1 ? -1 : 0);
    int dy = (y2 > y1) ? 1 : (y2 < y1 ? -1 : 0);
    
    if(rand_range(0,1)) {
        while(x != x2) { g.map[y][x].ch='.'; g.map[y][x].walkable=1; g.map[y][x].transparent=1; g.map[y][x].fg=C_FLOOR; x+=dx; }
        while(y != y2) { g.map[y][x].ch='.'; g.map[y][x].walkable=1; g.map[y][x].transparent=1; g.map[y][x].fg=C_FLOOR; y+=dy; }
    } else {
        while(y != y2) { g.map[y][x].ch='.'; g.map[y][x].walkable=1; g.map[y][x].transparent=1; g.map[y][x].fg=C_FLOOR; y+=dy; }
        while(x != x2) { g.map[y][x].ch='.'; g.map[y][x].walkable=1; g.map[y][x].transparent=1; g.map[y][x].fg=C_FLOOR; x+=dx; }
    }
}

int bsp_split(int x, int y, int w, int h, int iter, int* cx, int* cy, int* last_cx, int* last_cy) {
    if(iter == 0 || w < 12 || h < 12) {
        int rw = rand_range(4, w - 2);
        int rh = rand_range(4, h - 2);
        int rx = x + rand_range(1, w - rw - 1);
        int ry = y + rand_range(1, h - rh - 1);
        carve_room(rx, ry, rw, rh);
        *cx = rx + rw/2;
        *cy = ry + rh/2;
        *last_cx = *cx;
        *last_cy = *cy;
        
        if(rand_range(0, 100) < 60) spawn_monster(*cx, *cy);
        if(rand_range(0, 100) < 40) spawn_item(rx + rand_range(0, rw-1), ry + rand_range(0, rh-1));
        
        return 1;
    }
    
    int split_horiz = rand_range(0, 1);
    if(w * 2 > h * 3) split_horiz = 0;
    else if(h * 2 > w * 3) split_horiz = 1;
    
    int cx1, cy1, cx2, cy2, lcx1, lcy1, lcx2, lcy2;
    int has1, has2;
    
    if(split_horiz) { // split horizontally (y)
        int split = rand_range(h/3, 2*h/3);
        has1 = bsp_split(x, y, w, split, iter - 1, &cx1, &cy1, &lcx1, &lcy1);
        has2 = bsp_split(x, y + split, w, h - split, iter - 1, &cx2, &cy2, &lcx2, &lcy2);
    } else { // split vertically (x)
        int split = rand_range(w/3, 2*w/3);
        has1 = bsp_split(x, y, split, h, iter - 1, &cx1, &cy1, &lcx1, &lcy1);
        has2 = bsp_split(x + split, y, w - split, h, iter - 1, &cx2, &cy2, &lcx2, &lcy2);
    }
    
    if(has1 && has2) {
        dig_corridor(cx1, cy1, cx2, cy2);
        *cx = cx1; *cy = cy1;
        *last_cx = lcx2; *last_cy = lcy2;
        return 1;
    } else if(has1) {
        *cx = cx1; *cy = cy1; *last_cx = lcx1; *last_cy = lcy1; return 1;
    } else if(has2) {
        *cx = cx2; *cy = cy2; *last_cx = lcx2; *last_cy = lcy2; return 1;
    }
    return 0;
}

void generate_map() {
    for(int y=0; y<H; y++) {
        for(int x=0; x<W; x++) {
            g.map[y][x].ch = '#';
            g.map[y][x].fg = C_WALL;
            g.map[y][x].walkable = 0;
            g.map[y][x].transparent = 0;
            g.map[y][x].explored = 0;
            g.map[y][x].visible = 0;
        }
    }
    for(int i=1; i<MAX_ENTITIES; i++) g.entities[i].active = 0;
    for(int i=0; i<MAX_ITEMS; i++) g.items[i].active = 0;
    
    int cx, cy, lcx, lcy;
    bsp_split(1, 1, W-2, H-2, 4, &cx, &cy, &lcx, &lcy);
    
    get_player()->x = cx;
    get_player()->y = cy;
    
    if(g.dlevel == 30) {
        g.stair_x = -1;
        g.stair_y = -1;
        for(int i=1; i<MAX_ENTITIES; i++) {
            if(!g.entities[i].active) {
                Entity* e = &g.entities[i];
                e->active = 1; e->x = lcx; e->y = lcy;
                e->ch = '&'; e->fg = C_ASTAROTH; str_cpy(e->name, "Astaroth the Fallen");
                e->hp = e->max_hp = 800; e->atk = 50; e->def = 30; e->xp = 10000;
                e->behavior = B_SMART; e->special_ability = ABILITY_SUMMON;
                if(g.difficulty == 1) { e->max_hp = 1200; e->atk = 70; }
                else if(g.difficulty == 2) { e->max_hp = 1600; e->atk = 90; e->def = 45; }
                e->hp = e->max_hp;
                break;
            }
        }
    } else {
        g.stair_x = lcx;
        g.stair_y = lcy;
        g.map[lcy][lcx].ch = '>';
        g.map[lcy][lcx].fg = C_STAIRS;
    }
    
    for(int y=1; y<H-1; y++) {
        for(int x=1; x<W-1; x++) {
            if(g.map[y][x].ch == '.') {
                int walls_x = (g.map[y][x-1].ch == '#') && (g.map[y][x+1].ch == '#');
                int floors_y = (g.map[y-1][x].ch == '.') && (g.map[y+1][x].ch == '.');
                int walls_y = (g.map[y-1][x].ch == '#') && (g.map[y+1][x].ch == '#');
                int floors_x = (g.map[y][x-1].ch == '.') && (g.map[y][x+1].ch == '.');
                
                if((walls_x && floors_y) || (walls_y && floors_x)) {
                    if(rand_range(0, 100) < 60) {
                        g.map[y][x].ch = '+';
                        g.map[y][x].fg = C_DOOR;
                        g.map[y][x].transparent = 0;
                        g.map[y][x].walkable = 0;
                    }
                }
            }
        }
    }
    
    for(int y=1; y<H-1; y++) {
        for(int x=1; x<W-1; x++) {
            if(g.map[y][x].ch == '.' && g.map[y][x].walkable) {
                if(rand_range(0, 100) < 4) {
                    g.map[y][x].has_trap = rand_range(1, 3);
                } else if(rand_range(0, 100) < 1) {
                    for(int i=0; i<MAX_ITEMS; i++) {
                        if(!g.items[i].active) {
                            Item* it = &g.items[i];
                            it->active = 1; it->x = x; it->y = y;
                            it->ch = '^'; it->fg = RGB(0, 255, 255); it->type = TYPE_SHRINE;
                            str_cpy(it->name, "Magic Shrine");
                            break;
                        }
                    }
                }
            }
        }
    }
    

    COLORREF cur_wall = RGB(60, 120, 80); // Sewers L1-5
    COLORREF cur_floor = RGB(30, 60, 40);
    if(g.dlevel >= 6 && g.dlevel <= 10) { cur_wall = RGB(140, 110, 80); cur_floor = RGB(60, 50, 35); } // Caves
    else if(g.dlevel >= 11 && g.dlevel <= 15) { cur_wall = RGB(110, 80, 140); cur_floor = RGB(45, 30, 60); } // Crypt
    else if(g.dlevel >= 16 && g.dlevel <= 20) { cur_wall = RGB(180, 40, 20); cur_floor = RGB(80, 20, 10); } // Inferno
    else if(g.dlevel >= 21 && g.dlevel <= 25) { cur_wall = RGB(70, 20, 120); cur_floor = RGB(30, 10, 50); } // Void
    else if(g.dlevel >= 26) { cur_wall = RGB(220, 180, 60); cur_floor = RGB(40, 80, 100); } // Celestial Sanctuary
    for(int y=0; y<H; y++) {
        for(int x=0; x<W; x++) {
            if(g.map[y][x].ch == '#') g.map[y][x].fg = cur_wall;
            if(g.map[y][x].ch == '.') g.map[y][x].fg = cur_floor;
        }
    }
    calc_fov_bresenham();
}


void init_game() {
    RtlZeroMemory(&g, sizeof(GameState));
    g.state = 4; // Character creation
    g.char_race = RACE_HUMAN;
    g.char_class = CLASS_FIGHTER;
    g.difficulty = 0;
}

void finalize_character() {
    g.state = 0;
    g.dlevel = 1;
    Entity* p = get_player();
    p->active = 1;
    p->ch = '@';
    p->fg = C_PLAYER;
    str_cpy(p->name, "Hero");
    p->race = g.char_race;
    p->class_id = g.char_class;
    
    p->level = 1;
    p->xp = 0;
    p->hunger = 1000;
    
    // Base attributes
    p->str = 10; p->dex = 10; p->tou = 10; p->int_stat = 10; p->wil = 10;
    
    // Race modifiers
    if(p->race == RACE_ELF) { p->dex += 2; p->int_stat += 2; p->tou -= 2; }
    if(p->race == RACE_DWARF) { p->tou += 2; p->str += 1; p->dex -= 1; }
    
    // Class modifiers
    if(p->class_id == CLASS_FIGHTER) { p->str += 3; p->tou += 2; }
    if(p->class_id == CLASS_ROGUE) { p->dex += 4; }
    if(p->class_id == CLASS_WIZARD) { p->int_stat += 4; p->wil += 2; }
    
    
    calc_stats(p);
    p->hp = p->max_hp;
    p->mp = p->max_mp;
    
    // Unlock artifact spells & abilities
    g.known_spells[S_MISSILE] = 1;
    g.known_spells[S_METEOR] = 1;
    g.known_spells[S_INVISIBILITY] = 1;
    g.known_spells[S_BLESSING] = 1;
    
    add_msg("Welcome to KRogue!");
    add_msg("Find the stairs '>'. Defeat evil. F5 to quicksave.");
    
    generate_map();
    
    // Starting items
    if(p->class_id == CLASS_FIGHTER) {
        Item sword = {1, 0, 0, '(', C_WEAPON, "Iron Sword", 2, W_SWORD, 3};
        g.equip_weapon = sword;
    } else if(p->class_id == CLASS_ROGUE) {
        Item bow = {1, 0, 0, '(', C_WEAPON, "Shortbow", 2, W_BOW, 2};
        g.equip_weapon = bow;
    } else if(p->class_id == CLASS_WIZARD) {
        g.known_spells[S_HEAL] = 1;
        g.known_spells[S_FIREBALL] = 1;
        g.known_spells[S_FREEZE] = 1;
        g.known_spells[S_LIGHTNING] = 1;
        Item stick = {1, 0, 0, '(', C_WEAPON, "Stick", 2, W_SWORD, 1};
        g.equip_weapon = stick;
    }
}

Entity* get_entity_at(int x, int y) {
    for(int i=0; i<MAX_ENTITIES; i++) {
        if(g.entities[i].active && g.entities[i].x == x && g.entities[i].y == y)
            return &g.entities[i];
    }
    return NULL;
}

Item* get_item_at(int x, int y) {
    for(int i=0; i<MAX_ITEMS; i++) {
        if(g.items[i].active && g.items[i].x == x && g.items[i].y == y)
            return &g.items[i];
    }
    return NULL;
}

void calc_stats(Entity* e) {
    if(e == get_player()) {
        int eff_str = e->str;
        int bonus_hp = 0;
        
        if(g.equip_ring.active) {
            if(g.equip_ring.subtype == 0) eff_str += g.equip_ring.val;
            if(g.equip_ring.subtype == 1 || g.equip_ring.subtype == 2) bonus_hp += g.equip_ring.val;
        }

        e->max_hp = 10 + (e->tou * 2) + (e->level * 2) + bonus_hp;
        if(e->hp > e->max_hp) e->hp = e->max_hp;
        
        e->max_mp = (e->wil * 2) + (e->level * 2);
        if(e->class_id == CLASS_WIZARD) e->max_mp += 20;
        if(e->mp > e->max_mp) e->mp = e->max_mp;
        
        e->atk = eff_str / 2;
        e->def = e->tou / 3;
    }
}

void gain_xp(Entity* p, int amount) {
    p->xp += amount;
    if(p->xp >= p->level * 20) {
        p->level++;
        p->str += 1;
        p->tou += 1;
        p->dex += 1;
        calc_stats(p);
        p->hp = p->max_hp;
        char buf[100];
        wsprintfA(buf, "Welcome to level %d!", p->level);
        add_msg(buf);
    }
}

void handle_death(Entity* e, Entity* killer) {
    e->active = 0;
    char buf[100];
    wsprintfA(buf, "%s dies!", e->name);
    add_msg(buf);
    
    if(e == get_player()) {
        g.state = 1; // dead
        add_msg("You have perished! Game Over.");
    } else {
        if(killer == get_player()) {
            gain_xp(killer, e->xp);
            g.total_kills++;
        } else if(killer != NULL) {
            killer->xp += e->xp;
        }
        
        if(e->special_ability == ABILITY_SPLIT && e->max_hp >= 4) {
            int count = 0;
            for(int dy=-1; dy<=1; dy++) {
                for(int dx=-1; dx<=1; dx++) {
                    if(dx==0 && dy==0) continue;
                    int nx = e->x + dx, ny = e->y + dy;
                    if(nx>=0 && nx<W && ny>=0 && ny<H && g.map[ny][nx].walkable && !get_entity_at(nx, ny)) {
                        for(int i=1; i<MAX_ENTITIES; i++) {
                            if(!g.entities[i].active) {
                                Entity* n = &g.entities[i];
                                *n = *e;
                                n->active = 1; n->x = nx; n->y = ny;
                                n->max_hp = e->max_hp / 2; n->hp = n->max_hp;
                                n->atk = e->atk > 1 ? e->atk / 2 : 1;
                                n->def = e->def > 1 ? e->def / 2 : 1;
                                n->xp = e->xp > 1 ? e->xp / 2 : 1;
                                count++;
                                break;
                            }
                        }
                    }
                    if(count == 2) break;
                }
                if(count == 2) break;
            }
            if(count > 0 && g.map[e->y][e->x].visible) {
                add_msg("The Slime splits into smaller pieces!");
            }
        }
        
        if(rand_range(0, 100) < 30 && e->special_ability != ABILITY_SPLIT) {
            for(int i=0; i<MAX_ITEMS; i++) {
                if(!g.items[i].active) {
                    Item* it = &g.items[i];
                    it->active = 1; it->x = e->x; it->y = e->y;
                    it->ch = '*'; it->fg = RGB(255, 215, 0); it->type = TYPE_GOLD;
                    it->val = rand_range(5, 20) + g.dlevel * 2;
                    str_cpy(it->name, "Gold Pieces");
                    break;
                }
            }
        }
        
        if(e->ch == '&') {
            g.state = 10; // Victory
        }
    }
}

void combat(Entity* attacker, Entity* defender) {
    int atk = attacker->atk;
    int def = defender->def;
    
    if(attacker == get_player() && g.equip_weapon.active) {
        atk += g.equip_weapon.val;
        if(g.equip_weapon.subtype == W_HAMMER) def = def / 2;
        if(g.equip_weapon.subtype == W_SPEAR && (defender->behavior == B_FAST || defender->behavior == B_ERRATIC)) atk *= 2;
    }
    if(defender == get_player()) {
        if(g.equip_armor.active) def += g.equip_armor.val;
        if(g.equip_shield.active) def += g.equip_shield.val;
    }

    int dmg = rand_range(1, atk) - rand_range(0, def / 2);
    if(dmg < 1) dmg = 1;
    defender->hp -= dmg;
    
    char buf[100];
    wsprintfA(buf, "%s hits %s for %d dmg.", attacker->name, defender->name, dmg);
    add_msg(buf);
    
    if(defender->hp <= 0) {
        handle_death(defender, attacker);
    }
}

int process_status_effects(Entity* e) { // returns 1 if died
    if(e->status_duration > 0) {
        if(e->status_effect == STATUS_POISON) {
            e->hp -= 2;
            char buf[100];
            wsprintfA(buf, "%s takes poison damage.", e->name);
            if(e == get_player() || (e->x >= 0 && g.map[e->y][e->x].visible)) add_msg(buf);
            
            if(e->hp <= 0) {
                handle_death(e, NULL);
                return 1;
            }
        }
        e->status_duration--;
        if(e->status_duration == 0) {
            char buf[100];
            if(e->status_effect == STATUS_POISON) wsprintfA(buf, "%s is no longer poisoned.", e->name);
            else wsprintfA(buf, "%s is no longer rooted.", e->name);
            if(e == get_player() || (e->x >= 0 && g.map[e->y][e->x].visible)) add_msg(buf);
            e->status_effect = STATUS_NONE;
        }
    }
    return 0;
}

void check_trap(Entity* e) {
    if(g.map[e->y][e->x].has_trap) {
        g.map[e->y][e->x].has_trap = 0;
        int t = rand_range(0, 2);
        char buf[100];
        if(t == 0) {
            e->status_effect = STATUS_POISON;
            e->status_duration = 10;
            wsprintfA(buf, "%s triggers a poison trap!", e->name);
        } else if(t == 1) {
            e->status_effect = STATUS_ROOTED;
            e->status_duration = 5;
            wsprintfA(buf, "%s triggers a root trap!", e->name);
        } else {
            wsprintfA(buf, "%s triggers a teleport trap!", e->name);
            int nx, ny;
            do {
                nx = rand_range(1, W-2);
                ny = rand_range(1, H-2);
            } while(!g.map[ny][nx].walkable || get_entity_at(nx, ny));
            e->x = nx; e->y = ny;
            if(e == get_player()) calc_fov_bresenham();
        }
        if(e == get_player() || (e->x >= 0 && g.map[e->y][e->x].visible)) add_msg(buf);
    }
}

void do_monsters_turn(int is_bonus) {
    Entity* p = get_player();
    for(int i=1; i<MAX_ENTITIES; i++) {
        Entity* e = &g.entities[i];
        if(!e->active) continue;
        if(e == p) continue; // skip player in monster turn loop
        if(process_status_effects(e)) continue;
        
        if(is_bonus && e->behavior != B_FAST) continue;
        if(!is_bonus && e->behavior == B_SLUGGISH) {
            e->turn_parity = !e->turn_parity;
            if(e->turn_parity == 0) continue;
        }
        
        if(g.map[e->y][e->x].visible || dist2(e->x, e->y, p->x, p->y) <= 16) {
            if(e->special_ability == ABILITY_BREATHE_FIRE && rand_range(0, 100) < 20 && dist2(e->x, e->y, p->x, p->y) <= 36 && g.map[e->y][e->x].visible) {
                char buf[100];
                wsprintfA(buf, "%s breathes fire!", e->name);
                add_msg(buf);
                for(int dy=-1; dy<=1; dy++) {
                    for(int dx=-1; dx<=1; dx++) {
                        Entity* target = get_entity_at(p->x + dx, p->y + dy);
                        if(target && target != e) {
                            int dmg = 20 + e->level * 3;
                            target->hp -= dmg;
                            wsprintfA(buf, "%s is burned for %d dmg!", target->name, dmg);
                            add_msg(buf);
                            if(target->hp <= 0) handle_death(target, e);
                        }
                    }
                }
                continue;
            }
            
            if(e->special_ability == ABILITY_SUMMON && rand_range(0, 100) < 15 && g.map[e->y][e->x].visible) {
                int spawned = 0;
                for(int dy=-1; dy<=1; dy++) {
                    for(int dx=-1; dx<=1; dx++) {
                        if(dx==0 && dy==0) continue;
                        int nx = e->x + dx, ny = e->y + dy;
                        if(nx>=0 && nx<W && ny>=0 && ny<H && g.map[ny][nx].walkable && !get_entity_at(nx, ny)) {
                            for(int i=1; i<MAX_ENTITIES; i++) {
                                if(!g.entities[i].active) {
                                    Entity* n = &g.entities[i];
                                    RtlZeroMemory(n, sizeof(Entity));
                                    n->active = 1; n->x = nx; n->y = ny;
                                    n->ch = 'z'; n->fg = C_ZOMBIE; str_cpy(n->name, "Zombie");
                                    n->hp = n->max_hp = 25 + g.dlevel*2; n->atk = 5; n->def = 1; n->xp = 8; n->behavior = B_SLUGGISH;
                                    n->level = 1;
                                    spawned = 1;
                                    break;
                                }
                            }
                            if(spawned) break;
                        }
                    }
                    if(spawned) break;
                }
                if(spawned) {
                    char buf[100];
                    wsprintfA(buf, "%s summons a Zombie!", e->name);
                    add_msg(buf);
                    continue;
                }
            }

            int dx = 0, dy = 0;
            
            if(p->status_effect == STATUS_INVISIBLE || (e->behavior == B_ERRATIC && rand_range(0, 100) < 50)) {
                dx = rand_range(-1, 1);
                dy = rand_range(-1, 1);
            } else {
                if(p->x > e->x) dx = 1;
                else if(p->x < e->x) dx = -1;
                if(p->y > e->y) dy = 1;
                else if(p->y < e->y) dy = -1;
                
                if(e->behavior == B_COWARD && e->hp < e->max_hp / 2) {
                    dx = -dx;
                    dy = -dy;
                }
            }
            
            if(dx == 0 && dy == 0) continue;
            
            int nx = e->x + dx;
            int ny = e->y + dy;
            
            Entity* target = get_entity_at(nx, ny);
            if(target == p) {
                combat(e, p);
            } else if (e->status_effect != STATUS_ROOTED) {
                int can_move_direct = (!target && nx >= 0 && nx < W && ny >= 0 && ny < H && g.map[ny][nx].walkable);
                
                if (can_move_direct) {
                    e->x = nx;
                    e->y = ny;
                } else if (e->behavior == B_SMART || e->behavior == B_NORMAL || e->behavior == B_FAST) {
                    // Try alternative paths
                    int alt_x = e->x + dx;
                    int alt_y = e->y;
                    int can_alt_x = dx != 0 && !get_entity_at(alt_x, alt_y) && alt_x >= 0 && alt_x < W && g.map[alt_y][alt_x].walkable;
                    
                    int alt_x2 = e->x;
                    int alt_y2 = e->y + dy;
                    int can_alt_y = dy != 0 && !get_entity_at(alt_x2, alt_y2) && alt_y2 >= 0 && alt_y2 < H && g.map[alt_y2][alt_x2].walkable;
                    
                    if (can_alt_x && (!can_alt_y || rand_range(0, 100) < 50)) {
                        e->x = alt_x;
                        e->y = alt_y;
                    } else if (can_alt_y) {
                        e->x = alt_x2;
                        e->y = alt_y2;
                    } else if (e->behavior == B_SMART) {
                        // SMART enemies try harder to maneuver around obstacles
                        // Try moving perpendicular if direct axes fail
                        if (dx == 0 && dy != 0) {
                            if (!get_entity_at(e->x - 1, e->y) && g.map[e->y][e->x - 1].walkable) { e->x--; }
                            else if (!get_entity_at(e->x + 1, e->y) && g.map[e->y][e->x + 1].walkable) { e->x++; }
                        } else if (dy == 0 && dx != 0) {
                            if (!get_entity_at(e->x, e->y - 1) && g.map[e->y - 1][e->x].walkable) { e->y--; }
                            else if (!get_entity_at(e->x, e->y + 1) && g.map[e->y + 1][e->x].walkable) { e->y++; }
                        }
                    }
                }
                check_trap(e);
            }
        }
    }
}

void monsters_turn() {
    do_monsters_turn(0);
    do_monsters_turn(1);
    process_status_effects(get_player());
    
    Entity* p = get_player();
    if(p->hunger > 0) {
        p->hunger--;
    } else {
        p->hp--;
        if(rand_range(0, 100) < 20) add_msg("You are starving!");
        if(p->hp <= 0) handle_death(p, NULL);
    }
}

void move_player(int dx, int dy) {
    Entity* p = get_player();
    
    if (p->status_effect == STATUS_ROOTED) {
        add_msg("You are rooted to the ground and cannot move!");
        monsters_turn();
        return;
    }
    
    int nx = p->x + dx;
    int ny = p->y + dy;
    
    if(nx < 0 || nx >= W || ny < 0 || ny >= H) return;
    
    Entity* target = get_entity_at(nx, ny);
    if(target) {
        if(target->behavior == B_SHOPKEEPER) {
            g.active_shopkeeper = target;
            g.state = 9; // shop
            return;
        }
        combat(p, target);
        monsters_turn();
    } else if(g.map[ny][nx].ch == '+') {
        g.map[ny][nx].ch = '/';
        g.map[ny][nx].walkable = 1;
        g.map[ny][nx].transparent = 1;
        add_msg("You open the door.");
        calc_fov_bresenham();
        monsters_turn();
    } else if(g.map[ny][nx].walkable) {
        p->x = nx;
        p->y = ny;
        
        if(nx == g.stair_x && ny == g.stair_y) {
            add_msg("You descend deeper into the dungeon...");
            g.dlevel++;
            generate_map();
            return;
        }
        
        check_trap(p);
        calc_fov_bresenham();
        Item* it = get_item_at(p->x, p->y);
        if(it) {
            if(it->type == TYPE_GOLD) {
                p->gold += it->val;
                it->active = 0;
                char buf[100];
                wsprintfA(buf, "You pick up %d gold pieces.", it->val);
                add_msg(buf);
            } else if(it->type == TYPE_SHRINE) {
                it->active = 0;
                int buff = rand_range(0, 2);
                if(buff == 0) {
                    p->max_hp += 10; p->hp = p->max_hp;
                    add_msg("The shrine glows! You feel much healthier!");
                } else if(buff == 1) {
                    gain_xp(p, 50);
                    add_msg("The shrine glows! You feel wiser!");
                } else {
                    p->str += 2; calc_stats(p);
                    add_msg("The shrine glows! You feel stronger!");
                }
            } else {
                char buf[100];
                wsprintfA(buf, "You see a %s here. Press 'g' to pick up.", it->name);
                add_msg(buf);
            }
        }
        monsters_turn();
    }
}

void pickup_item() {
    Entity* p = get_player();
    Item* it = get_item_at(p->x, p->y);
    if(it) {
        // find inventory slot
        for(int i=0; i<MAX_INVENTORY; i++) {
            if(!g.inventory[i].active) {
                g.inventory[i] = *it;
                it->active = 0;
                char buf[100];
                wsprintfA(buf, "You picked up %s.", g.inventory[i].name);
                add_msg(buf);
                monsters_turn();
                return;
            }
        }
        add_msg("Inventory full!");
    } else {
        add_msg("Nothing here to pick up.");
    }
}

void quaff_potion(int inv_idx) {
    if(!g.inventory[inv_idx].active || g.inventory[inv_idx].type != 1) return;
    Entity* p = get_player();
    p->hp += g.inventory[inv_idx].val;
    if(p->hp > p->max_hp) p->hp = p->max_hp;
    char buf[100];
    wsprintfA(buf, "You drink %s. You feel better.", g.inventory[inv_idx].name);
    add_msg(buf);
    g.inventory[inv_idx].active = 0;
    monsters_turn();
}

void eat_food(int inv_idx) {
    if(!g.inventory[inv_idx].active || g.inventory[inv_idx].type != TYPE_FOOD) return;
    Entity* p = get_player();
    p->hunger += g.inventory[inv_idx].val;
    if(p->hunger > 1000) p->hunger = 1000;
    char buf[100];
    wsprintfA(buf, "You eat the %s.", g.inventory[inv_idx].name);
    add_msg(buf);
    g.inventory[inv_idx].active = 0;
    monsters_turn();
}

void read_book(int inv_idx) {
    if(!g.inventory[inv_idx].active || g.inventory[inv_idx].type != TYPE_SPELLBOOK) return;
    int spell_id = g.inventory[inv_idx].subtype;
    if(g.known_spells[spell_id]) {
        add_msg("You already know this spell.");
        return;
    }
    g.known_spells[spell_id] = 1;
    char buf[100];
    wsprintfA(buf, "You read %s. You learned a new spell!", g.inventory[inv_idx].name);
    add_msg(buf);
    g.inventory[inv_idx].active = 0;
    monsters_turn();
}

void unequip_item(int slot) {
    Item* target = NULL;
    if(slot == 1) target = &g.equip_weapon;
    else if(slot == 2) target = &g.equip_armor;
    else if(slot == 3) target = &g.equip_shield;
    else if(slot == 4) target = &g.equip_ring;
    
    if(target && target->active) {
        for(int i=0; i<MAX_INVENTORY; i++) {
            if(!g.inventory[i].active) {
                g.inventory[i] = *target;
                target->active = 0;
                char buf[100];
                wsprintfA(buf, "You unequip %s.", g.inventory[i].name);
                add_msg(buf);
                calc_stats(get_player());
                return;
            }
        }
        add_msg("Inventory full!");
    }
}

void equip_item(int inv_idx) {
    if(!g.inventory[inv_idx].active) return;
    Item* it = &g.inventory[inv_idx];
    char buf[100];
    
    Item temp = *it;
    
    if(it->type == TYPE_WEAPON) {
        if(g.equip_weapon.active) *it = g.equip_weapon;
        else it->active = 0;
        g.equip_weapon = temp;
        wsprintfA(buf, "You equip %s.", g.equip_weapon.name);
    } else if(it->type == TYPE_ARMOR) {
        if(g.equip_armor.active) *it = g.equip_armor;
        else it->active = 0;
        g.equip_armor = temp;
        wsprintfA(buf, "You wear %s.", g.equip_armor.name);
    } else if(it->type == TYPE_SHIELD) {
        if(g.equip_shield.active) *it = g.equip_shield;
        else it->active = 0;
        g.equip_shield = temp;
        wsprintfA(buf, "You equip %s.", g.equip_shield.name);
    } else if(it->type == TYPE_RING) {
        if(g.equip_ring.active) *it = g.equip_ring;
        else it->active = 0;
        g.equip_ring = temp;
        wsprintfA(buf, "You put on %s.", g.equip_ring.name);
    } else {
        return;
    }
    
    calc_stats(get_player());
    add_msg(buf);
    monsters_turn();
}

void save_game() {
    HANDLE hFile = CreateFileA("save.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, &g, sizeof(GameState), &written, NULL);
        CloseHandle(hFile);
        add_msg("Game saved (fake-cheat quicksave).");
    }
}

void load_game() {
    HANDLE hFile = CreateFileA("save.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile != INVALID_HANDLE_VALUE) {
        DWORD read;
        ReadFile(hFile, &g, sizeof(GameState), &read, NULL);
        CloseHandle(hFile);
        add_msg("Game loaded (save scummed!).");
        calc_fov_bresenham();
    } else {
        add_msg("No save file found.");
    }
}

void draw_tile_gdi(HDC memDC, int x, int y, Tile* t, int visible) {
    int px = x * char_w;
    int py = y * char_h;
    
    if (t->ch == '#') {
        COLORREF wallColor = visible ? t->fg : C_FOG;
        HBRUSH brush = CreateSolidBrush(wallColor);
        RECT r = { px, py, px + char_w, py + char_h };
        FillRect(memDC, &r, brush);
        DeleteObject(brush);
        
        HPEN pen = CreatePen(PS_SOLID, 1, visible ? RGB(220, 220, 220) : RGB(70, 70, 70));
        HPEN oldPen = (HPEN)SelectObject(memDC, pen);
        MoveToEx(memDC, px, py + char_h - 1, NULL);
        LineTo(memDC, px, py);
        LineTo(memDC, px + char_w - 1, py);
        SelectObject(memDC, oldPen);
        DeleteObject(pen);
    } else if (t->ch == '.') {
        COLORREF floorColor = visible ? t->fg : RGB(20, 20, 20);
        HBRUSH brush = CreateSolidBrush(floorColor);
        RECT r = { px, py, px + char_w, py + char_h };
        FillRect(memDC, &r, brush);
        DeleteObject(brush);
        
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(15, 15, 15));
        HPEN oldPen = (HPEN)SelectObject(memDC, pen);
        MoveToEx(memDC, px + char_w - 1, py, NULL);
        LineTo(memDC, px + char_w - 1, py + char_h - 1);
        LineTo(memDC, px, py + char_h - 1);
        SelectObject(memDC, oldPen);
        DeleteObject(pen);
    } else if (t->ch == '+') {
        HBRUSH brush = CreateSolidBrush(RGB(139, 69, 19));
        RECT r = { px + 1, py + 1, px + char_w - 1, py + char_h - 1 };
        FillRect(memDC, &r, brush);
        DeleteObject(brush);
        HBRUSH goldBrush = CreateSolidBrush(RGB(255, 215, 0));
        HBRUSH oldB = (HBRUSH)SelectObject(memDC, goldBrush);
        Ellipse(memDC, px + char_w - 4, py + char_h / 2 - 1, px + char_w - 1, py + char_h / 2 + 2);
        SelectObject(memDC, oldB);
        DeleteObject(goldBrush);
    }
}

void draw_item_gdi(HDC memDC, int x, int y, Item* it) {
    int px = x * char_w;
    int py = y * char_h;
    int cx = px + char_w / 2;
    int cy = py + char_h / 2;
    
    if (it->type == TYPE_GOLD || it->ch == '*') {
        HBRUSH b = CreateSolidBrush(RGB(255, 215, 0));
        HBRUSH oldB = (HBRUSH)SelectObject(memDC, b);
        Ellipse(memDC, cx - 3, cy - 3, cx + 4, cy + 4);
        SelectObject(memDC, oldB);
        DeleteObject(b);
    } else if (it->type == TYPE_HEAL || it->ch == '!') {
        HBRUSH b = CreateSolidBrush(it->fg);
        HBRUSH oldB = (HBRUSH)SelectObject(memDC, b);
        Ellipse(memDC, cx - 3, cy, cx + 4, cy + 6);
        RECT r = { cx - 1, cy - 4, cx + 2, cy };
        FillRect(memDC, &r, b);
        SelectObject(memDC, oldB);
        DeleteObject(b);
    } else if (it->type == TYPE_SHRINE || it->ch == '^') {
        HBRUSH b = CreateSolidBrush(RGB(100, 100, 100));
        RECT r = { px + 2, py + char_h - 5, px + char_w - 2, py + char_h - 1 };
        FillRect(memDC, &r, b);
        DeleteObject(b);
        POINT pts[4] = { {cx, py + 2}, {cx + 4, cy - 1}, {cx, cy + 3}, {cx - 4, cy - 1} };
        HBRUSH cb = CreateSolidBrush(RGB(0, 255, 255));
        HBRUSH oldB = (HBRUSH)SelectObject(memDC, cb);
        Polygon(memDC, pts, 4);
        SelectObject(memDC, oldB);
        DeleteObject(cb);
    } else if (it->ch == '>') {
        HBRUSH b = CreateSolidBrush(RGB(255, 215, 0));
        RECT r1 = { px + 1, py + 3, px + char_w - 1, py + 6 };
        RECT r2 = { px + 3, py + 8, px + char_w - 3, py + 11 };
        RECT r3 = { px + 5, py + 13, px + char_w - 5, py + 16 };
        FillRect(memDC, &r1, b);
        FillRect(memDC, &r2, b);
        FillRect(memDC, &r3, b);
        DeleteObject(b);
    } else {
        SetTextColor(memDC, it->fg);
        SetBkMode(memDC, TRANSPARENT);
        TextOutA(memDC, px, py, &it->ch, 1);
        SetBkMode(memDC, OPAQUE);
    }
}

void draw_entity_gdi(HDC memDC, int x, int y, Entity* e) {
    int px = x * char_w;
    int py = y * char_h;
    int cx = px + char_w / 2;
    int cy = py + char_h / 2;
    
    if (e == get_player() || e->ch == '@') {
        HBRUSH capeB = CreateSolidBrush(RGB(0, 120, 255));
        RECT capeR = { cx - 4, cy - 2, cx + 5, cy + 7 };
        FillRect(memDC, &capeR, capeB);
        DeleteObject(capeB);
        
        HBRUSH armorB = CreateSolidBrush(RGB(220, 220, 220));
        RECT armorR = { cx - 3, cy - 4, cx + 4, cy + 5 };
        FillRect(memDC, &armorR, armorB);
        DeleteObject(armorB);
        
        HBRUSH visorB = CreateSolidBrush(RGB(255, 215, 0));
        RECT visorR = { cx - 2, cy - 6, cx + 3, cy - 4 };
        FillRect(memDC, &visorR, visorB);
        DeleteObject(visorB);
        
        HPEN pen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
        HPEN oldP = (HPEN)SelectObject(memDC, pen);
        MoveToEx(memDC, cx + 3, cy + 2, NULL);
        LineTo(memDC, cx + 6, cy - 5);
        SelectObject(memDC, oldP);
        DeleteObject(pen);
    } else if (e->ch == 'r') {
        HBRUSH b = CreateSolidBrush(RGB(180, 180, 0));
        HBRUSH oldB = (HBRUSH)SelectObject(memDC, b);
        Ellipse(memDC, cx - 4, cy - 2, cx + 5, cy + 5);
        SelectObject(memDC, oldB);
        DeleteObject(b);
        SetPixel(memDC, cx + 2, cy - 1, RGB(255, 0, 0));
    } else if (e->ch == 'b') {
        HPEN pen = CreatePen(PS_SOLID, 2, RGB(200, 100, 0));
        HPEN oldP = (HPEN)SelectObject(memDC, pen);
        MoveToEx(memDC, cx - 5, cy - 3, NULL);
        LineTo(memDC, cx, cy + 2);
        LineTo(memDC, cx + 5, cy - 3);
        SelectObject(memDC, oldP);
        DeleteObject(pen);
    } else if (e->ch == 'o' || e->ch == 'g') {
        HBRUSH b = CreateSolidBrush(e->fg);
        RECT r = { cx - 4, cy - 6, cx + 5, cy + 6 };
        FillRect(memDC, &r, b);
        DeleteObject(b);
        SetPixel(memDC, cx - 1, cy - 3, RGB(255, 0, 0));
        SetPixel(memDC, cx + 2, cy - 3, RGB(255, 0, 0));
    } else if (e->ch == 'x' || e->ch == 'z') {
        HBRUSH b = CreateSolidBrush(e->fg);
        HBRUSH oldB = (HBRUSH)SelectObject(memDC, b);
        Ellipse(memDC, cx - 3, cy - 7, cx + 4, cy);
        SelectObject(memDC, oldB);
        DeleteObject(b);
        RECT r = { cx - 2, cy, cx + 3, cy + 6 };
        HBRUSH b2 = CreateSolidBrush(RGB(150, 150, 150));
        FillRect(memDC, &r, b2);
        DeleteObject(b2);
    } else if (e->ch == 'T' || e->ch == 'M') {
        HBRUSH b = CreateSolidBrush(e->fg);
        RECT r = { cx - 5, cy - 8, cx + 6, cy + 8 };
        FillRect(memDC, &r, b);
        DeleteObject(b);
    } else if (e->ch == 'D' || e->ch == 'B') {
        HBRUSH b = CreateSolidBrush(e->fg);
        HBRUSH oldB = (HBRUSH)SelectObject(memDC, b);
        Ellipse(memDC, cx - 6, cy - 6, cx + 7, cy + 7);
        SelectObject(memDC, oldB);
        DeleteObject(b);
        SetPixel(memDC, cx - 2, cy - 2, RGB(255, 255, 0));
        SetPixel(memDC, cx + 2, cy - 2, RGB(255, 255, 0));
    } else {
        SetTextColor(memDC, e->fg);
        SetBkMode(memDC, TRANSPARENT);
        TextOutA(memDC, px, py, &e->ch, 1);
        SetBkMode(memDC, OPAQUE);
    }
}

void draw_game(HDC hdc) {
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBM = CreateCompatibleBitmap(hdc, W * char_w, TOTAL_H * char_h);
    HBITMAP hOldBM = (HBITMAP)SelectObject(memDC, memBM);
    HFONT hOldFont = (HFONT)SelectObject(memDC, g_font);

    RECT bgRect = {0, 0, W * char_w, TOTAL_H * char_h};
    HBRUSH bgBrush = CreateSolidBrush(RGB(0,0,0));
    FillRect(memDC, &bgRect, bgBrush);
    DeleteObject(bgBrush);
    
    SetBkMode(memDC, OPAQUE);
    
    if(g.state == 0 || g.state == 1 || g.state == 10) { // play, dead, or victory
        for(int y=0; y<H; y++) {
            for(int x=0; x<W; x++) {
                Tile* t = &g.map[y][x];
                if(t->visible || t->explored) {
                    draw_tile_gdi(memDC, x, y, t, t->visible);
                    
                    if(t->visible) {
                        Item* it = get_item_at(x, y);
                        if(it) draw_item_gdi(memDC, x, y, it);
                        
                        Entity* e = get_entity_at(x, y);
                        if(e) draw_entity_gdi(memDC, x, y, e);
                    }
                }
            }
        }
        
        // UI
        SetTextColor(memDC, RGB(200,200,200));
        SetBkColor(memDC, RGB(0,0,0));
        Entity* p = get_player();
        char buf[200];
        int eff_atk = p->atk;
        int eff_def = p->def;
        if(g.equip_weapon.active) eff_atk += g.equip_weapon.val;
        if(g.equip_armor.active) eff_def += g.equip_armor.val;
        if(g.equip_shield.active) eff_def += g.equip_shield.val;
        
        wsprintfA(buf, "HP:%d/%d MP:%d/%d ATK:%d DEF:%d LVL:%d XP:%d DLVL:%d GOLD:%d FD:%d", p->hp, p->max_hp, p->mp, p->max_mp, eff_atk, eff_def, p->level, p->xp, g.dlevel, p->gold, p->hunger);
        TextOutA(memDC, 0, H * char_h, buf, str_len(buf));
        
        char status_str[50] = "";
        if(p->status_effect == STATUS_POISON) str_cpy(status_str, "[POISONED]");
        else if(p->status_effect == STATUS_ROOTED) str_cpy(status_str, "[ROOTED]");
        
        if(status_str[0] != '\0') {
            SetTextColor(memDC, RGB(255, 0, 0));
            TextOutA(memDC, W * char_w - 100, H * char_h, status_str, str_len(status_str));
            SetTextColor(memDC, RGB(200,200,200));
        }
        
        for(int i=0; i<5; i++) {
            TextOutA(memDC, 0, (H + 1 + i) * char_h, g.msgs[MAX_MSGS - 1 - i], str_len(g.msgs[MAX_MSGS - 1 - i]));
        }
        
        if(g.state == 3) {
            SetTextColor(memDC, RGB(255,0,0));
            SetBkColor(memDC, RGB(0,0,0));
            TextOutA(memDC, g.target_x * char_w, g.target_y * char_h, "X", 1);
        } else if(g.state == 10) {
            SetTextColor(memDC, RGB(255, 215, 0)); // Gold color
            SetBkColor(memDC, RGB(0,0,0));
            TextOutA(memDC, W/2 * char_w - 150, H/2 * char_h - 20, "V I C T O R Y !", 15);
            SetTextColor(memDC, RGB(255, 255, 255));
            TextOutA(memDC, W/2 * char_w - 200, H/2 * char_h, "You have defeated the Chaos God!", 32);
            TextOutA(memDC, W/2 * char_w - 150, H/2 * char_h + 20, "Press 'R' to restart.", 21);
        }
    } else if(g.state == 9 && g.active_shopkeeper) { // shop
        SetTextColor(memDC, RGB(255,255,255));
        SetBkColor(memDC, RGB(0,0,0));
        char buf[100];
        wsprintfA(buf, "%s's Shop", g.active_shopkeeper->name);
        TextOutA(memDC, 20, 20, buf, str_len(buf));
        TextOutA(memDC, 20, 40, "Press 1-3 to buy, ESC to leave.", 31);
        
        wsprintfA(buf, "Your Gold: %d", get_player()->gold);
        SetTextColor(memDC, RGB(255, 215, 0));
        TextOutA(memDC, 20, 60, buf, str_len(buf));
        
        int y = 100;
        for(int i=0; i<3; i++) {
            Item* it = &g.active_shopkeeper->shop_inventory[i];
            if(it->active) {
                int price = it->val * 5; // basic markup
                wsprintfA(buf, "%d. %s - %d gold", i+1, it->name, price);
                SetTextColor(memDC, it->fg);
                TextOutA(memDC, 20, y, buf, str_len(buf));
            } else {
                SetTextColor(memDC, RGB(100, 100, 100));
                wsprintfA(buf, "%d. (Sold out)", i+1);
                TextOutA(memDC, 20, y, buf, str_len(buf));
            }
            y += char_h * 2;
        }
    } else if(g.state == 2) { // inventory
        SetTextColor(memDC, RGB(255,255,255));
        SetBkColor(memDC, RGB(0,0,0));
        TextOutA(memDC, 20, 20, "INVENTORY", 9);
        TextOutA(memDC, 20, 40, "Press 1-4 to unequip, a-z to use/equip. ESC to return.", 54);
        
        int y = 60;
        char buf[100];
        if(g.equip_weapon.active) { wsprintfA(buf, "1. Weapon: %s (+%d ATK)", g.equip_weapon.name, g.equip_weapon.val); TextOutA(memDC, 20, y, buf, str_len(buf)); y += char_h; }
        if(g.equip_armor.active) { wsprintfA(buf, "2. Armor: %s (+%d DEF)", g.equip_armor.name, g.equip_armor.val); TextOutA(memDC, 20, y, buf, str_len(buf)); y += char_h; }
        if(g.equip_shield.active) { wsprintfA(buf, "3. Shield: %s (+%d DEF)", g.equip_shield.name, g.equip_shield.val); TextOutA(memDC, 20, y, buf, str_len(buf)); y += char_h; }
        if(g.equip_ring.active) { wsprintfA(buf, "4. Ring/Amulet: %s", g.equip_ring.name); TextOutA(memDC, 20, y, buf, str_len(buf)); y += char_h; }
        y += char_h;
        
        for(int i=0; i<MAX_INVENTORY; i++) {
            if(g.inventory[i].active) {
                wsprintfA(buf, "%c - %s (Val:%d)", 'a'+i, g.inventory[i].name, g.inventory[i].val);
                SetTextColor(memDC, g.inventory[i].fg);
                TextOutA(memDC, 20, y, buf, str_len(buf));
                y += char_h;
            }
        }
    } else if(g.state == 4) { // create char
        SetTextColor(memDC, RGB(255,255,255));
        SetBkColor(memDC, RGB(0,0,0));
        TextOutA(memDC, 20, 20, "CHARACTER CREATION", 18);
        
        char buf[100];
        wsprintfA(buf, "Race: %s (Press R to change)", g.char_race == RACE_HUMAN ? "Human" : (g.char_race == RACE_ELF ? "Elf" : "Dwarf"));
        TextOutA(memDC, 20, 60, buf, str_len(buf));
        
        wsprintfA(buf, "Class: %s (Press C to change)", g.char_class == CLASS_FIGHTER ? "Fighter" : (g.char_class == CLASS_ROGUE ? "Rogue" : "Wizard"));
        TextOutA(memDC, 20, 80, buf, str_len(buf));
        
        wsprintfA(buf, "Difficulty: %s (Press D to change)", g.difficulty == 0 ? "Normal" : (g.difficulty == 1 ? "Hard" : "Nightmare"));
        TextOutA(memDC, 20, 100, buf, str_len(buf));
        
        TextOutA(memDC, 20, 140, "Press ENTER to begin your journey...", 36);
    } else if(g.state == 5) { // char sheet
        SetTextColor(memDC, RGB(255,255,255));
        SetBkColor(memDC, RGB(0,0,0));
        TextOutA(memDC, 20, 20, "CHARACTER SHEET", 15);
        TextOutA(memDC, 20, 40, "Press ESC or C to return.", 25);
        
        Entity* p = get_player();
        char buf[100];
        wsprintfA(buf, "Name:  %s", p->name); TextOutA(memDC, 20, 80, buf, str_len(buf));
        wsprintfA(buf, "Race:  %s", p->race == RACE_HUMAN ? "Human" : (p->race == RACE_ELF ? "Elf" : "Dwarf")); TextOutA(memDC, 20, 100, buf, str_len(buf));
        wsprintfA(buf, "Class: %s", p->class_id == CLASS_FIGHTER ? "Fighter" : (p->class_id == CLASS_ROGUE ? "Rogue" : "Wizard")); TextOutA(memDC, 20, 120, buf, str_len(buf));
        wsprintfA(buf, "Level: %d", p->level); TextOutA(memDC, 20, 140, buf, str_len(buf));
        wsprintfA(buf, "XP:    %d / %d", p->xp, p->level * 20); TextOutA(memDC, 20, 160, buf, str_len(buf));
        wsprintfA(buf, "Food:  %d / 1000", p->hunger); TextOutA(memDC, 20, 180, buf, str_len(buf));
        wsprintfA(buf, "Kills: %d", g.total_kills); TextOutA(memDC, 200, 180, buf, str_len(buf));
        
        wsprintfA(buf, "STR: %d", p->str); TextOutA(memDC, 20, 200, buf, str_len(buf));
        wsprintfA(buf, "DEX: %d", p->dex); TextOutA(memDC, 20, 220, buf, str_len(buf));
        wsprintfA(buf, "TOU: %d", p->tou); TextOutA(memDC, 20, 240, buf, str_len(buf));
        wsprintfA(buf, "INT: %d", p->int_stat); TextOutA(memDC, 20, 260, buf, str_len(buf));
        wsprintfA(buf, "WIL: %d", p->wil); TextOutA(memDC, 20, 280, buf, str_len(buf));
    } else if(g.state == 6) { // spells
        SetTextColor(memDC, RGB(255,255,255));
        SetBkColor(memDC, RGB(0,0,0));
        TextOutA(memDC, 20, 20, "SPELLS MENU", 11);
        TextOutA(memDC, 20, 40, "Press a letter to cast. ESC to return.", 38);
        
        Entity* p = get_player();
        char buf[100];
        wsprintfA(buf, "MP: %d / %d", p->mp, p->max_mp);
        TextOutA(memDC, 20, 60, buf, str_len(buf));
        
        int y = 100;
        if(g.known_spells[S_MISSILE]) {
            TextOutA(memDC, 20, y, "a - Magic Missile (Cost: 3 MP)", 30);
            y += char_h;
        }
        if(g.known_spells[S_HEAL]) {
            TextOutA(memDC, 20, y, "b - Heal (Cost: 5 MP)", 21);
            y += char_h;
        }
        if(g.known_spells[S_FIREBALL]) {
            TextOutA(memDC, 20, y, "c - Fireball (Cost: 8 MP)", 25);
            y += char_h;
        }
        if(g.known_spells[S_FREEZE]) {
            TextOutA(memDC, 20, y, "d - Ice Storm (Cost: 6 MP)", 26);
            y += char_h;
        }
        if(g.known_spells[S_LIGHTNING]) {
            TextOutA(memDC, 20, y, "e - Lightning (Cost: 7 MP)", 26);
            y += char_h;
        }
        if(g.known_spells[S_METEOR]) {
            TextOutA(memDC, 20, y, "f - Meteor Strike (Cost: 12 MP)", 31);
            y += char_h;
        }
        if(g.known_spells[S_INVISIBILITY]) {
            TextOutA(memDC, 20, y, "g - Invisibility Cloak (Cost: 10 MP)", 36);
            y += char_h;
        }
        if(g.known_spells[S_BLESSING]) {
            TextOutA(memDC, 20, y, "h - Divine Blessing (Cost: 15 MP)", 33);
            y += char_h;
        }
    } else if(g.state == 8) { // message log
        SetTextColor(memDC, RGB(255,255,255));
        SetBkColor(memDC, RGB(0,0,0));
        TextOutA(memDC, 20, 20, "MESSAGE LOG", 11);
        TextOutA(memDC, 20, 40, "Press ESC or V to return.", 25);
        
        int y = 60;
        int max_lines = 30;
        int start_idx = MAX_MSGS - max_lines;
        if(start_idx < 0) start_idx = 0;
        
        for(int i = start_idx; i < MAX_MSGS; i++) {
            if(g.msgs[i][0] != '\0') {
                TextOutA(memDC, 20, y, g.msgs[i], str_len(g.msgs[i]));
                y += char_h;
            }
        }
    } else if(g.state == 7) { // help
        SetBkColor(memDC, RGB(0,0,0));
        SetTextColor(memDC, RGB(255, 215, 0));
        TextOutA(memDC, 20, 5, "=== KRogue Help ===", 19);
        
        int y = 30;
        SetTextColor(memDC, RGB(100, 200, 255));
        TextOutA(memDC, 20, y, "-- Movement --", 14); y += char_h;
        SetTextColor(memDC, RGB(200, 200, 200));
        TextOutA(memDC, 20, y, "Arrow Keys / Numpad / HJKL  Move cardinal", 43); y += char_h;
        TextOutA(memDC, 20, y, "YUBN / Numpad 7913           Move diagonal", 44); y += char_h;
        TextOutA(memDC, 20, y, "Numpad 5 / .                 Wait a turn", 41); y += char_h;
        y += 5;
        SetTextColor(memDC, RGB(100, 200, 255));
        TextOutA(memDC, 20, y, "-- Actions --", 13); y += char_h;
        SetTextColor(memDC, RGB(200, 200, 200));
        TextOutA(memDC, 20, y, "G or ,    Pick up item", 22); y += char_h;
        TextOutA(memDC, 20, y, "I         Inventory (a-z use, 1-4 unequip)", 43); y += char_h;
        TextOutA(memDC, 20, y, "C         Character sheet", 25); y += char_h;
        TextOutA(memDC, 20, y, "M         Spells menu", 21); y += char_h;
        TextOutA(memDC, 20, y, "V         Message log", 21); y += char_h;
        TextOutA(memDC, 20, y, "F         Fire bow (requires bow equipped)", 42); y += char_h;
        TextOutA(memDC, 20, y, "F5 / F9   Quicksave / Quickload", 31); y += char_h;
        TextOutA(memDC, 20, y, "?         This help screen", 26); y += char_h;
        y += 5;
        SetTextColor(memDC, RGB(100, 200, 255));
        TextOutA(memDC, 20, y, "-- Combat Tips --", 17); y += char_h;
        SetTextColor(memDC, RGB(200, 200, 200));
        TextOutA(memDC, 20, y, "Hammers halve enemy defense.", 28); y += char_h;
        TextOutA(memDC, 20, y, "Spears deal x2 vs Fast/Erratic foes.", 36); y += char_h;
        TextOutA(memDC, 20, y, "Bows let you shoot from safety at Sluggish enemies.", 51); y += char_h;
        TextOutA(memDC, 20, y, "Shields add DEF. Wizards rely on spells + INT/WIL.", 51); y += char_h;
        y += 5;
        SetTextColor(memDC, RGB(100, 200, 255));
        TextOutA(memDC, 20, y, "-- Enemy Behaviors --", 21); y += char_h;
        SetTextColor(memDC, RGB(200, 200, 200));
        TextOutA(memDC, 20, y, "Normal: steady pursuit.  Fast: moves twice.", 43); y += char_h;
        TextOutA(memDC, 20, y, "Sluggish: moves every other turn.", 33); y += char_h;
        TextOutA(memDC, 20, y, "Erratic: random movement.  Coward: flees at low HP.", 52); y += char_h;
        TextOutA(memDC, 20, y, "Smart: finds paths around obstacles.", 36); y += char_h;
        y += 5;
        SetTextColor(memDC, RGB(100, 200, 255));
        TextOutA(memDC, 20, y, "-- Lore --", 10); y += char_h;
        SetTextColor(memDC, RGB(180, 180, 180));
        TextOutA(memDC, 20, y, "The Caverns of Chaos stretch ten levels deep.", 46); y += char_h;
        TextOutA(memDC, 20, y, "At their heart lurks the Chaos God Andor Drakon.", 49); y += char_h;
        TextOutA(memDC, 20, y, "Descend the stairs '>' to reach him. Defeat him to win.", 55); y += char_h;
        y += 10;
        SetTextColor(memDC, RGB(255, 255, 0));
        TextOutA(memDC, 20, y, "Press ESC or ? to return.", 25);
    }
    
    BitBlt(hdc, 0, 0, W * char_w, TOTAL_H * char_h, memDC, 0, 0, SRCCOPY);
    
    SelectObject(memDC, hOldFont);
    SelectObject(memDC, hOldBM);
    DeleteObject(memBM);
    DeleteDC(memDC);
}

void fire_arrow() {
    Entity* p = get_player();
    int x0 = p->x, y0 = p->y;
    int x1 = g.target_x, y1 = g.target_y;
    int dx = (x1 > x0 ? x1 - x0 : x0 - x1);
    int dy = -(y1 > y0 ? y1 - y0 : y0 - y1);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    
    while(1) {
        if(x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if(e2 >= dy) { err += dy; x0 += sx; }
        if(e2 <= dx) { err += dx; y0 += sy; }
        
        Entity* target = get_entity_at(x0, y0);
        if(target && target != p) {
            char buf[100];
            int atk = p->dex / 2;
            if(g.equip_weapon.active) atk += g.equip_weapon.val;
            
            int dmg = rand_range(1, atk) - rand_range(0, target->def / 2);
            if(dmg < 1) dmg = 1;
            target->hp -= dmg;
            wsprintfA(buf, "Arrow hits %s for %d dmg.", target->name, dmg);
            add_msg(buf);
            
            if(target->hp <= 0) {
                handle_death(target, p);
            }
            break;
        }
        
        if(!g.map[y0][x0].walkable && !g.map[y0][x0].transparent) {
            add_msg("The arrow hits a wall.");
            break;
        }
    }
    monsters_turn();
}

void fire_spell() {
    Entity* p = get_player();
    
    if (g.active_spell == S_MISSILE) {
        int x0 = p->x, y0 = p->y;
        int x1 = g.target_x, y1 = g.target_y;
        int dx = (x1 > x0 ? x1 - x0 : x0 - x1);
        int dy = -(y1 > y0 ? y1 - y0 : y0 - y1);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;
        
        p->mp -= 3;
        add_msg("You cast Magic Missile!");
        
        while(1) {
            if(x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if(e2 >= dy) { err += dy; x0 += sx; }
            if(e2 <= dx) { err += dx; y0 += sy; }
            
            Entity* target = get_entity_at(x0, y0);
            if(target && target != p) {
                char buf[100];
                int dmg = 10 + p->level * 2;
                target->hp -= dmg;
                wsprintfA(buf, "Magic Missile hits %s for %d dmg.", target->name, dmg);
                add_msg(buf);
                
                if(target->hp <= 0) {
                    handle_death(target, p);
                }
                break;
            }
            
            if(!g.map[y0][x0].walkable && !g.map[y0][x0].transparent) {
                add_msg("The magic missile dissipates against a wall.");
                break;
            }
        }
    } else if (g.active_spell == S_FIREBALL) {
        p->mp -= 8;
        add_msg("You cast Fireball! Boom!");
        
        for(int dy=-1; dy<=1; dy++) {
            for(int dx=-1; dx<=1; dx++) {
                Entity* target = get_entity_at(g.target_x + dx, g.target_y + dy);
                if(target && target != p) {
                    int dmg = 20 + p->level * 3;
                    target->hp -= dmg;
                    char buf[100];
                    wsprintfA(buf, "%s is burned for %d dmg!", target->name, dmg);
                    add_msg(buf);
                    
                    if(target->hp <= 0) {
                        handle_death(target, p);
                    }
                }
            }
        }
    } else if (g.active_spell == S_FREEZE) {
        p->mp -= 6;
        add_msg("You cast Ice Storm!");
        
        for(int dy=-1; dy<=1; dy++) {
            for(int dx=-1; dx<=1; dx++) {
                Entity* target = get_entity_at(g.target_x + dx, g.target_y + dy);
                if(target && target != p) {
                    int dmg = 10 + p->level * 2;
                    target->hp -= dmg;
                    target->status_effect = STATUS_ROOTED;
                    target->status_duration = 5;
                    char buf[100];
                    wsprintfA(buf, "%s is frozen for %d dmg!", target->name, dmg);
                    add_msg(buf);
                    
                    if(target->hp <= 0) {
                        handle_death(target, p);
                    }
                }
            }
        }
    } else if (g.active_spell == S_LIGHTNING) {
        int x0 = p->x, y0 = p->y;
        int x1 = g.target_x, y1 = g.target_y;
        int dx = (x1 > x0 ? x1 - x0 : x0 - x1);
        int dy = -(y1 > y0 ? y1 - y0 : y0 - y1);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;
        
        p->mp -= 7;
        add_msg("You cast Lightning!");
        
        while(1) {
            if(x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if(e2 >= dy) { err += dy; x0 += sx; }
            if(e2 <= dx) { err += dx; y0 += sy; }
            
            Entity* target = get_entity_at(x0, y0);
            if(target && target != p) {
                char buf[100];
                int dmg = 15 + p->level * 3;
                target->hp -= dmg;
                wsprintfA(buf, "Lightning zaps %s for %d dmg.", target->name, dmg);
                add_msg(buf);
                if(target->hp <= 0) {
                    handle_death(target, p);
                }
            }
            if(!g.map[y0][x0].walkable && !g.map[y0][x0].transparent) {
                break; // stop at wall
            }
        }
    } else if (g.active_spell == S_METEOR) {
        p->mp -= 12;
        add_msg("METEOR STRIKE! Flaming meteors rain down!");
        for(int dy=-2; dy<=2; dy++) {
            for(int dx=-2; dx<=2; dx++) {
                int tx = g.target_x + dx;
                int ty = g.target_y + dy;
                if(tx >= 0 && tx < W && ty >= 0 && ty < H) {
                    Entity* target = get_entity_at(tx, ty);
                    if(target && target != p) {
                        int dmg = 45 + p->level * 5;
                        target->hp -= dmg;
                        char buf[100];
                        wsprintfA(buf, "%s is crushed by meteor for %d dmg!", target->name, dmg);
                        add_msg(buf);
                        if(target->hp <= 0) handle_death(target, p);
                    }
                }
            }
        }
    } else if (g.active_spell == S_INVISIBILITY) {
        p->mp -= 10;
        p->status_effect = STATUS_INVISIBLE;
        p->status_duration = 5;
        add_msg("You don the Invisibility Cloak! Enemies cannot see you for 5 turns.");
    } else if (g.active_spell == S_BLESSING) {
        p->mp -= 15;
        p->hp = p->max_hp;
        p->status_effect = STATUS_NONE;
        p->status_duration = 0;
        p->hunger = 1000;
        add_msg("Divine Miracle! HP restored, status cured, hunger satisfied!");
    }
    
    monsters_turn();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            g_font = CreateFontA(char_h, char_w, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            init_game();
            return 0;
        }
        case WM_DESTROY: {
            if (g_font) DeleteObject(g_font);
            PostQuitMessage(0);
            return 0;
        }
        case WM_KEYDOWN: {
            if(g.state == 1 || g.state == 10) { // dead or victory
                if(wParam == 'R') init_game();
                if(wParam == VK_F9) load_game();
            } else if(g.state == 0) { // play
                switch(wParam) {
                    case VK_UP: case VK_NUMPAD8: case 'K': move_player(0, -1); break;
                    case VK_DOWN: case VK_NUMPAD2: case 'J': move_player(0, 1); break;
                    case VK_LEFT: case VK_NUMPAD4: case 'H': move_player(-1, 0); break;
                    case VK_RIGHT: case VK_NUMPAD6: case 'L': move_player(1, 0); break;
                    case VK_NUMPAD7: case 'Y': move_player(-1, -1); break;
                    case VK_NUMPAD9: case 'U': move_player(1, -1); break;
                    case VK_NUMPAD1: case 'B': move_player(-1, 1); break;
                    case VK_NUMPAD3: case 'N': move_player(1, 1); break;
                    case VK_NUMPAD5: case '.': monsters_turn(); break; // wait
                    case 'G': case ',': pickup_item(); break;
                    case 'I': g.state = 2; break; // open inventory
                    case 'C': g.state = 5; break; // open char sheet
                    case 'M': g.state = 6; break; // open spells menu
                    case 'V': g.state = 8; break; // open message log
                    case VK_OEM_2: g.state = 7; break; // ? key - open help
                    case 'F': 
                        if(g.equip_weapon.active && g.equip_weapon.subtype == W_BOW) {
                            g.state = 3; 
                            g.targeting_mode = 0;
                            Entity* p = get_player();
                            g.target_x = p->x; g.target_y = p->y;
                            add_msg("Targeting... (Arrows to aim, Enter/F to fire, ESC to cancel)");
                        } else {
                            add_msg("You must equip a Bow to fire.");
                        }
                        break;
                    case VK_F5: save_game(); break;
                    case VK_F9: load_game(); break;
                }
            } else if(g.state == 2) { // inventory
                if(wParam == VK_ESCAPE) {
                    g.state = 0;
                } else if(wParam >= '1' && wParam <= '4') {
                    unequip_item(wParam - '0');
                } else if(wParam >= 'A' && wParam <= 'Z') {
                    int idx = wParam - 'A';
                    if(g.inventory[idx].active) {
                        if(g.inventory[idx].type == 1) quaff_potion(idx);
                        else if(g.inventory[idx].type == TYPE_FOOD) eat_food(idx);
                        else if(g.inventory[idx].type == TYPE_SPELLBOOK) read_book(idx);
                        else equip_item(idx);
                    }
                    g.state = 0; // close inventory after use
                }
            } else if(g.state == 3) { // targeting
                if(wParam == VK_ESCAPE) {
                    g.state = 0;
                    add_msg("Targeting cancelled.");
                } else if(wParam == VK_UP || wParam == VK_NUMPAD8) g.target_y--;
                else if(wParam == VK_DOWN || wParam == VK_NUMPAD2) g.target_y++;
                else if(wParam == VK_LEFT || wParam == VK_NUMPAD4) g.target_x--;
                else if(wParam == VK_RIGHT || wParam == VK_NUMPAD6) g.target_x++;
                else if(wParam == VK_RETURN || wParam == 'F') {
                    g.state = 0;
                    if(g.targeting_mode == 0) fire_arrow();
                    else fire_spell();
                }
            } else if(g.state == 4) { // create char
                if(wParam == 'R') g.char_race = (g.char_race + 1) % 3;
                else if(wParam == 'C') g.char_class = (g.char_class + 1) % 3;
                else if(wParam == 'D') g.difficulty = (g.difficulty + 1) % 3;
                else if(wParam == VK_RETURN) finalize_character();
            } else if(g.state == 5) { // char sheet
                if(wParam == VK_ESCAPE || wParam == 'C') g.state = 0;
            } else if(g.state == 6) { // spells
                if(wParam == VK_ESCAPE || wParam == 'M') g.state = 0;
                else if(wParam == 'A' && g.known_spells[S_MISSILE]) {
                    if(get_player()->mp >= 3) {
                        g.state = 3;
                        g.targeting_mode = 1;
                        g.active_spell = S_MISSILE;
                        Entity* p = get_player();
                        g.target_x = p->x; g.target_y = p->y;
                        add_msg("Targeting Magic Missile... (Arrows to aim, Enter/F to fire)");
                    } else {
                        add_msg("Not enough MP!");
                        g.state = 0;
                    }
                }
                else if(wParam == 'B' && g.known_spells[S_HEAL]) {
                    Entity* p = get_player();
                    if(p->mp >= 5) {
                        p->mp -= 5;
                        p->hp += 25 + p->level * 5;
                        if(p->hp > p->max_hp) p->hp = p->max_hp;
                        add_msg("You cast Heal and restore your wounds.");
                        g.state = 0;
                        monsters_turn();
                    } else {
                        add_msg("Not enough MP!");
                        g.state = 0;
                    }
                }
                else if(wParam == 'C' && g.known_spells[S_FIREBALL]) {
                    if(get_player()->mp >= 8) {
                        g.state = 3;
                        g.targeting_mode = 1;
                        g.active_spell = S_FIREBALL;
                        Entity* p = get_player();
                        g.target_x = p->x; g.target_y = p->y;
                        add_msg("Targeting Fireball... (Arrows to aim, Enter/F to fire)");
                    } else {
                        add_msg("Not enough MP!");
                        g.state = 0;
                    }
                }
                else if(wParam == 'D' && g.known_spells[S_FREEZE]) {
                    if(get_player()->mp >= 6) {
                        g.state = 3;
                        g.targeting_mode = 1;
                        g.active_spell = S_FREEZE;
                        Entity* p = get_player();
                        g.target_x = p->x; g.target_y = p->y;
                        add_msg("Targeting Ice Storm... (Arrows to aim, Enter/F to fire)");
                    } else {
                        add_msg("Not enough MP!");
                        g.state = 0;
                    }
                }
                else if(wParam == 'F' && g.known_spells[S_METEOR]) {
                    if(get_player()->mp >= 12) {
                        g.state = 3;
                        g.targeting_mode = 1;
                        g.active_spell = S_METEOR;
                        Entity* p = get_player();
                        g.target_x = p->x; g.target_y = p->y;
                        add_msg("Targeting Meteor Strike 5x5... (Arrows to aim, Enter/F to fire)");
                    } else {
                        add_msg("Not enough MP!");
                        g.state = 0;
                    }
                }
                else if(wParam == 'G' && g.known_spells[S_INVISIBILITY]) {
                    if(get_player()->mp >= 10) {
                        g.state = 0;
                        g.active_spell = S_INVISIBILITY;
                        fire_spell();
                    } else {
                        add_msg("Not enough MP!");
                        g.state = 0;
                    }
                }
                else if(wParam == 'H' && g.known_spells[S_BLESSING]) {
                    if(get_player()->mp >= 15) {
                        g.state = 0;
                        g.active_spell = S_BLESSING;
                        fire_spell();
                    } else {
                        add_msg("Not enough MP!");
                        g.state = 0;
                    }
                }
            } else if(g.state == 9) { // shop
                if(wParam == VK_ESCAPE) {
                    g.state = 0;
                    g.active_shopkeeper = NULL;
                } else if(wParam >= '1' && wParam <= '3') {
                    if(g.active_shopkeeper) {
                        int idx = wParam - '1';
                        Item* it = &g.active_shopkeeper->shop_inventory[idx];
                        if(it->active) {
                            int price = it->val * 5;
                            Entity* p = get_player();
                            if(p->gold >= price) {
                                int slot = -1;
                                for(int i=0; i<MAX_INVENTORY; i++) {
                                    if(!g.inventory[i].active) { slot = i; break; }
                                }
                                if(slot != -1) {
                                    p->gold -= price;
                                    g.inventory[slot] = *it;
                                    it->active = 0;
                                    char buf[100];
                                    wsprintfA(buf, "Bought %s for %d gold.", g.inventory[slot].name, price);
                                    add_msg(buf);
                                } else {
                                    add_msg("Inventory full!");
                                }
                            } else {
                                add_msg("Not enough gold!");
                            }
                        }
                    }
                }
            } else if(g.state == 7) { // help
                if(wParam == VK_ESCAPE || wParam == VK_OEM_2) g.state = 0;
            } else if(g.state == 8) { // message log
                if(wParam == VK_ESCAPE || wParam == 'V') g.state = 0;
            }
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            draw_game(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void __stdcall MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KRogueClass";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(100));
    
    RegisterClassA(&wc);
    
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT r = {0, 0, W * char_w, TOTAL_H * char_h};
    AdjustWindowRect(&r, style, FALSE);
    int win_w = r.right - r.left;
    int win_h = r.bottom - r.top;
    
    g_hwnd = CreateWindowExA(0, "KRogueClass", "KRogue", style, CW_USEDEFAULT, CW_USEDEFAULT, win_w, win_h, NULL, NULL, hInstance, NULL);
    
    ShowWindow(g_hwnd, SW_SHOW);
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    ExitProcess(0);
}
