import re

with open(r'd:\KiloApps\KRogue\main.c', 'r', encoding='utf-8') as f:
    code = f.read()

# Add constants
code = code.replace('#define C_DEMON RGB(200, 0, 50)',
'''#define C_DEMON RGB(200, 0, 50)
#define C_GHOST RGB(150, 200, 255)
#define C_HYDRA RGB(50, 200, 100)
#define C_CUBE  RGB(0, 255, 255)''')

code = code.replace('#define S_FREEZE 3',
'''#define S_FREEZE 3
#define S_LIGHTNING 4''')

# Add to GameState
code = code.replace('int difficulty; // 0=Normal, 1=Hard, 2=Nightmare\n} GameState;',
'''int difficulty; // 0=Normal, 1=Hard, 2=Nightmare
    int total_kills;
} GameState;''')

# Add tracking total kills
code = code.replace('            gain_xp(killer, e->xp);\n        } else if(killer != NULL) {',
'''            gain_xp(killer, e->xp);
            g.total_kills++;
        } else if(killer != NULL) {''')

# Expand dungeon to 15
code = code.replace('if(g.dlevel == 10) {', 'if(g.dlevel == 15) {')
code = code.replace('"ten levels deep."', '"fifteen levels deep."')

# Biome coloring
biome_code = '''
    COLORREF cur_wall = C_WALL;
    COLORREF cur_floor = C_FLOOR;
    if(g.dlevel >= 5 && g.dlevel < 10) { cur_wall = RGB(100, 150, 100); cur_floor = RGB(50, 80, 50); }
    else if(g.dlevel >= 10 && g.dlevel < 15) { cur_wall = RGB(150, 50, 50); cur_floor = RGB(80, 40, 40); }
    else if(g.dlevel >= 15) { cur_wall = RGB(100, 0, 100); cur_floor = RGB(50, 0, 50); }
    for(int y=0; y<H; y++) {
        for(int x=0; x<W; x++) {
            if(g.map[y][x].ch == '#') g.map[y][x].fg = cur_wall;
            if(g.map[y][x].ch == '.') g.map[y][x].fg = cur_floor;
        }
    }
    calc_fov_bresenham();
}
'''
code = code.replace('    calc_fov_bresenham();\n}', biome_code)

# Add enemies
old_tier2 = '''else if(m == 2) { e->ch = 'z'; e->fg = C_ZOMBIE; str_cpy(e->name, "Zombie"); e->hp = e->max_hp = 25 + g.dlevel*2; e->atk = 5; e->def = 1; e->xp = 8; e->behavior = B_SLUGGISH; }
                else if(m == 3) { e->ch = 'k'; e->fg = C_KOBOLD; str_cpy(e->name, "Kobold"); e->hp = e->max_hp = 8 + g.dlevel*2; e->atk = 3; e->def = 1; e->xp = 4; e->behavior = B_COWARD; }
                else { e->ch = 'c'; e->fg = C_GOBLIN; str_cpy(e->name, "Centipede"); e->hp = e->max_hp = 12 + g.dlevel*2; e->atk = 4; e->def = 2; e->xp = 5; e->behavior = B_ERRATIC; }'''

new_tier2 = '''else if(m == 2) { e->ch = 'z'; e->fg = C_ZOMBIE; str_cpy(e->name, "Zombie"); e->hp = e->max_hp = 25 + g.dlevel*2; e->atk = 5; e->def = 1; e->xp = 8; e->behavior = B_SLUGGISH; }
                else if(m == 3) { e->ch = 'k'; e->fg = C_KOBOLD; str_cpy(e->name, "Kobold"); e->hp = e->max_hp = 8 + g.dlevel*2; e->atk = 3; e->def = 1; e->xp = 4; e->behavior = B_COWARD; }
                else if(m == 4) { e->ch = 'c'; e->fg = C_GOBLIN; str_cpy(e->name, "Centipede"); e->hp = e->max_hp = 12 + g.dlevel*2; e->atk = 4; e->def = 2; e->xp = 5; e->behavior = B_ERRATIC; }
                else { e->ch = 'G'; e->fg = C_GHOST; str_cpy(e->name, "Ghost"); e->hp = e->max_hp = 15 + g.dlevel*2; e->atk = 6; e->def = 2; e->xp = 10; e->behavior = B_ERRATIC; }'''
code = code.replace(old_tier2, new_tier2)

old_tier3 = '''else if(m == 2) { e->ch = 'W'; e->fg = C_WRAITH; str_cpy(e->name, "Wraith"); e->hp = e->max_hp = 20 + g.dlevel*3; e->atk = 9; e->def = 5; e->xp = 18; e->behavior = B_ERRATIC; }
                else if(m == 3) { e->ch = 'e'; e->fg = C_ELF; str_cpy(e->name, "Dark Elf"); e->hp = e->max_hp = 20 + g.dlevel*3; e->atk = 12; e->def = 3; e->xp = 16; e->behavior = B_SMART; }
                else { e->ch = 'y'; e->fg = C_SKELETON; str_cpy(e->name, "Gargoyle"); e->hp = e->max_hp = 35 + g.dlevel*3; e->atk = 7; e->def = 8; e->xp = 16; e->behavior = B_SLUGGISH; }'''

new_tier3 = '''else if(m == 2) { e->ch = 'W'; e->fg = C_WRAITH; str_cpy(e->name, "Wraith"); e->hp = e->max_hp = 20 + g.dlevel*3; e->atk = 9; e->def = 5; e->xp = 18; e->behavior = B_ERRATIC; }
                else if(m == 3) { e->ch = 'e'; e->fg = C_ELF; str_cpy(e->name, "Dark Elf"); e->hp = e->max_hp = 20 + g.dlevel*3; e->atk = 12; e->def = 3; e->xp = 16; e->behavior = B_SMART; }
                else if(m == 4) { e->ch = 'y'; e->fg = C_SKELETON; str_cpy(e->name, "Gargoyle"); e->hp = e->max_hp = 35 + g.dlevel*3; e->atk = 7; e->def = 8; e->xp = 16; e->behavior = B_SLUGGISH; }
                else { e->ch = 'C'; e->fg = C_CUBE; str_cpy(e->name, "Gelatinous Cube"); e->hp = e->max_hp = 50 + g.dlevel*3; e->atk = 5; e->def = 10; e->xp = 25; e->behavior = B_SLUGGISH; e->special_ability = ABILITY_SPLIT; }'''
code = code.replace(old_tier3, new_tier3)

old_tier4 = '''else if(m == 2) { e->ch = 'G'; e->fg = C_GOLEM; str_cpy(e->name, "Stone Golem"); e->hp = e->max_hp = 80 + g.dlevel*5; e->atk = 10; e->def = 15; e->xp = 60; e->behavior = B_SLUGGISH; }
                else if(m == 3) { e->ch = 'L'; e->fg = C_LICH; str_cpy(e->name, "Lich"); e->hp = e->max_hp = 40 + g.dlevel*5; e->atk = 20; e->def = 5; e->xp = 80; e->behavior = B_SMART; e->special_ability = ABILITY_SUMMON; }
                else { e->ch = 'd'; e->fg = C_DEMON; str_cpy(e->name, "Demon"); e->hp = e->max_hp = 50 + g.dlevel*5; e->atk = 18; e->def = 10; e->xp = 70; e->behavior = B_FAST; e->special_ability = ABILITY_BREATHE_FIRE; }'''

new_tier4 = '''else if(m == 2) { e->ch = 'G'; e->fg = C_GOLEM; str_cpy(e->name, "Stone Golem"); e->hp = e->max_hp = 80 + g.dlevel*5; e->atk = 10; e->def = 15; e->xp = 60; e->behavior = B_SLUGGISH; }
                else if(m == 3) { e->ch = 'L'; e->fg = C_LICH; str_cpy(e->name, "Lich"); e->hp = e->max_hp = 40 + g.dlevel*5; e->atk = 20; e->def = 5; e->xp = 80; e->behavior = B_SMART; e->special_ability = ABILITY_SUMMON; }
                else if(m == 4) { e->ch = 'd'; e->fg = C_DEMON; str_cpy(e->name, "Demon"); e->hp = e->max_hp = 50 + g.dlevel*5; e->atk = 18; e->def = 10; e->xp = 70; e->behavior = B_FAST; e->special_ability = ABILITY_BREATHE_FIRE; }
                else { e->ch = 'H'; e->fg = C_HYDRA; str_cpy(e->name, "Hydra"); e->hp = e->max_hp = 90 + g.dlevel*5; e->atk = 25; e->def = 8; e->xp = 100; e->behavior = B_NORMAL; }'''
code = code.replace(old_tier4, new_tier4)
code = code.replace('int m = rand_range(0, 4);', 'int m = rand_range(0, 5);')

# Spellbook random gen
old_spell = '''else { str_cpy(it->name, "Spellbook of Ice Storm"); }'''
new_spell = '''else if(it->subtype == S_FREEZE) { str_cpy(it->name, "Spellbook of Ice Storm"); }
        else { str_cpy(it->name, "Spellbook of Lightning"); }'''
code = code.replace('it->subtype = rand_range(0, 3);', 'it->subtype = rand_range(0, 4);')
code = code.replace(old_spell, new_spell)

# Spells menu UI
old_spell_menu = '''if(g.known_spells[S_FREEZE]) {
            TextOutA(memDC, 20, y, "d - Ice Storm (Cost: 6 MP)", 26);
            y += char_h;
        }'''
new_spell_menu = '''if(g.known_spells[S_FREEZE]) {
            TextOutA(memDC, 20, y, "d - Ice Storm (Cost: 6 MP)", 26);
            y += char_h;
        }
        if(g.known_spells[S_LIGHTNING]) {
            TextOutA(memDC, 20, y, "e - Lightning (Cost: 7 MP)", 26);
            y += char_h;
        }'''
code = code.replace(old_spell_menu, new_spell_menu)

# Char sheet UI
code = code.replace('wsprintfA(buf, "Food:  %d / 1000", p->hunger); TextOutA(memDC, 20, 180, buf, str_len(buf));',
'''wsprintfA(buf, "Food:  %d / 1000", p->hunger); TextOutA(memDC, 20, 180, buf, str_len(buf));
        wsprintfA(buf, "Kills: %d", g.total_kills); TextOutA(memDC, 200, 180, buf, str_len(buf));''')

# Firing spell logic
old_spell_logic = '''if(target->hp <= 0) {
                        handle_death(target, p);
                    }
                }
            }
        }
    }
    
    monsters_turn();
}'''
new_spell_logic = '''if(target->hp <= 0) {
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
    }
    
    monsters_turn();
}'''
code = code.replace(old_spell_logic, new_spell_logic)

# Menu inputs
old_input = '''else if(wParam == 'D' && g.known_spells[S_FREEZE]) {
                    g.active_spell = S_FREEZE;
                    g.state = 3; g.targeting_mode = 1;
                    Entity* p = get_player();
                    g.target_x = p->x; g.target_y = p->y;
                    add_msg("Targeting Ice Storm... (Arrows to aim, Enter/F to fire, ESC to cancel)");
                }'''
new_input = '''else if(wParam == 'D' && g.known_spells[S_FREEZE]) {
                    g.active_spell = S_FREEZE;
                    g.state = 3; g.targeting_mode = 1;
                    Entity* p = get_player();
                    g.target_x = p->x; g.target_y = p->y;
                    add_msg("Targeting Ice Storm... (Arrows to aim, Enter/F to fire, ESC to cancel)");
                } else if(wParam == 'E' && g.known_spells[S_LIGHTNING]) {
                    g.active_spell = S_LIGHTNING;
                    g.state = 3; g.targeting_mode = 1;
                    Entity* p = get_player();
                    g.target_x = p->x; g.target_y = p->y;
                    add_msg("Targeting Lightning... (Arrows to aim, Enter/F to fire, ESC to cancel)");
                }'''
code = code.replace(old_input, new_input)

with open(r'd:\KiloApps\KRogue\main.c', 'w', encoding='utf-8') as f:
    f.write(code)
