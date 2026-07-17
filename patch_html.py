import sys

with open(r'd:\KiloApps\KiloOS\public\apps\krogue.html', 'r', encoding='utf-8') as f:
    code = f.read()

# Add styles for new monsters and biomes
old_style = '''        .food { color: #a52; }'''
new_style = '''        .food { color: #a52; }
        .monster-G { color: #0ff; }
        .monster-C { color: #088; }
        .monster-H { color: #0f0; }
        .wall.moss { color: #696; }
        .floor.moss { color: #353; }
        .wall.volcanic { color: #955; }
        .floor.volcanic { color: #533; }
        .wall.abyss { color: #606; }
        .floor.abyss { color: #303; }'''
code = code.replace(old_style, new_style)

# Add kills state
code = code.replace('let dlevel = 1;', 'let dlevel = 1;\n        let total_kills = 0;')
code = code.replace('player = { x: 1, y: 1, hp: 10, max_hp: 10, xp: 0, level: 1, gold: 0, hunger: 200, max_hunger: 200 };', 'player = { x: 1, y: 1, hp: 10, max_hp: 10, xp: 0, level: 1, gold: 0, hunger: 200, max_hunger: 200 };\n                    total_kills = 0;')
code = code.replace('dlevel = d.dlevel || 1;', 'dlevel = d.dlevel || 1;\n                total_kills = d.total_kills || 0;')
code = code.replace('msgs }));', 'msgs, total_kills }));')

# Spawn new enemies
old_spawn = '''else if (dlevel > 3 && r > 0.6 && r < 0.8) { type = 'z'; hp = 15; name = 'Zombie'; }
                else if (dlevel > 4 && r > 0.8) { type = 'T'; hp = 25; name = 'Cave Troll'; }'''
new_spawn = '''else if (dlevel > 3 && r > 0.4 && r < 0.5) { type = 'z'; hp = 15; name = 'Zombie'; }
                else if (dlevel > 4 && r > 0.5 && r < 0.6) { type = 'T'; hp = 25; name = 'Cave Troll'; }
                else if (dlevel > 5 && r > 0.6 && r < 0.7) { type = 'G'; hp = 18; name = 'Ghost'; }
                else if (dlevel > 8 && r > 0.7 && r < 0.85) { type = 'C'; hp = 35; name = 'Gelatinous Cube'; }
                else if (dlevel > 10 && r > 0.85) { type = 'H'; hp = 50; name = 'Hydra'; }'''
code = code.replace(old_spawn, new_spawn)

# Biomes and kills tracking rendering
old_render = '''} else {
                                    out += map[y][x] === '#' ? "<span class='wall'>#</span>" : "<span class='floor'>.</span>";
                                }'''
new_render = '''} else {
                                    let biome = "";
                                    if(dlevel >= 5 && dlevel < 10) biome = " moss";
                                    else if(dlevel >= 10 && dlevel < 15) biome = " volcanic";
                                    else if(dlevel >= 15) biome = " abyss";
                                    out += map[y][x] === '#' ? `<span class='wall${biome}'>#</span>` : `<span class='floor${biome}'>.</span>`;
                                }'''
code = code.replace(old_render, new_render)

code = code.replace('`\\nHP: ${player.hp}/${player.max_hp}  Level: ${player.level}  XP: ${player.xp}  Gold: ${player.gold}  DLVL: ${dlevel}  Food: ${player.hunger}\\n`;',
'`\\nHP: ${player.hp}/${player.max_hp}  Level: ${player.level}  XP: ${player.xp}  Gold: ${player.gold}  DLVL: ${dlevel}  Food: ${player.hunger}  Kills: ${total_kills}\\n`;')

# Kills logic and Victory
old_kill = '''player.xp += 5;
                                    monsters.splice(mIdx, 1);'''
new_kill = '''player.xp += 5;
                                    total_kills++;
                                    monsters.splice(mIdx, 1);'''
code = code.replace(old_kill, new_kill)

old_stair = '''if (it.type === 'stairs') {
                                        dlevel++;
                                        addMsg(`You descend to level ${dlevel}...`);
                                        initMap();
                                        render();
                                        return;'''
new_stair = '''if (it.type === 'stairs') {
                                        dlevel++;
                                        if (dlevel > 15) {
                                            state = 3; // Victory
                                            render();
                                            return;
                                        }
                                        addMsg(`You descend to level ${dlevel}...`);
                                        initMap();
                                        render();
                                        return;'''
code = code.replace(old_stair, new_stair)

# Victory state
old_victory = '''} else if (state === 2) {
                out += "You died.\\nPress R to restart.";
            }'''
new_victory = '''} else if (state === 2) {
                out += "You died.\\nPress R to restart.";
            } else if (state === 3) {
                out += "VICTORY!\\nYou defeated the dungeon!\\nPress R to restart.";
            }'''
code = code.replace(old_victory, new_victory)

code = code.replace("if (e.key === 'r' || e.key === 'R') { state = 0; render(); }", "if (e.key === 'r' || e.key === 'R') { state = 0; render(); }")
code = code.replace("} else if (state === 2) {", "} else if (state === 2 || state === 3) {")

with open(r'd:\KiloApps\KiloOS\public\apps\krogue.html', 'w', encoding='utf-8') as f:
    f.write(code)
