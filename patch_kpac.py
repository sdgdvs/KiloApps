import re

def patch_main_c():
    with open('d:/KiloApps/KPac/main.c', 'r', encoding='utf-8') as f:
        content = f.read()

    # 1. maps
    maps_str = """    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,5,2,2,2,2,1,2,1,2,2,2,2,5,1},
        {1,2,1,1,1,2,1,2,1,2,1,1,1,2,1},
        {1,2,1,3,2,2,2,2,2,2,2,3,1,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,2,2,1,2,2,2,2,2,1,2,2,2,1},
        {1,1,1,2,1,2,1,1,1,2,1,2,1,1,1},
        {0,0,0,2,2,2,1,0,1,2,2,2,0,0,0},
        {1,1,1,2,1,2,1,1,1,2,1,2,1,1,1},
        {1,2,2,2,1,2,2,2,2,2,1,2,2,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,1,4,2,2,2,2,2,2,2,4,1,2,1},
        {1,2,1,1,1,2,1,2,1,2,1,1,1,2,1},
        {1,5,2,2,2,2,1,2,1,2,2,2,2,5,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,2,2,1,2,2,2,2,2,3,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,2,2,5,1,2,2,2,1,5,2,2,2,1},
        {1,1,1,1,2,1,1,1,1,1,2,1,1,1,1},
        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
        {1,2,1,1,1,2,1,1,1,2,1,1,1,2,1},
        {1,2,1,0,1,2,1,0,1,2,1,0,1,2,1},
        {1,2,1,1,1,2,1,1,1,2,1,1,1,2,1},
        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
        {1,1,1,1,2,1,1,1,1,1,2,1,1,1,1},
        {1,2,2,2,4,1,2,2,2,1,4,2,2,2,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,3,2,2,2,2,2,1,2,2,2,2,2,3,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,5,2,1,2,2,2,2,2,2,2,1,2,5,1},
        {1,1,2,1,2,1,1,1,1,1,2,1,2,1,1},
        {1,2,2,2,2,1,3,2,3,1,2,2,2,2,1},
        {1,2,1,1,2,1,2,1,2,1,2,1,1,2,1},
        {1,2,1,2,2,2,2,1,2,2,2,2,1,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,2,2,1,0,0,0,0,0,1,2,2,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,1,2,2,2,2,1,2,2,2,2,1,2,1},
        {1,2,1,1,2,1,2,1,2,1,2,1,1,2,1},
        {1,2,2,2,2,1,4,2,4,1,2,2,2,2,1},
        {1,1,2,1,2,1,1,1,1,1,2,1,2,1,1},
        {1,5,2,1,2,2,2,2,2,2,2,1,2,5,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,1,2,5,2,1,2,2,2,3,1},
        {1,2,1,1,2,1,2,1,2,1,2,1,1,2,1},
        {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        {1,1,1,1,1,2,1,1,1,2,1,1,1,1,1},
        {1,2,2,2,1,2,2,2,2,2,1,2,2,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {0,2,1,2,2,2,2,0,2,2,2,2,1,2,0},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,2,2,1,2,2,2,2,2,1,2,2,2,1},
        {1,1,1,1,1,2,1,1,1,2,1,1,1,1,1},
        {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        {1,2,1,1,2,1,2,1,2,1,2,1,1,2,1},
        {1,3,2,2,2,1,2,5,2,1,2,2,2,3,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,5,2,2,2,2,2,2,2,2,2,2,2,5,1},
        {1,2,1,1,1,1,1,2,1,1,1,1,1,2,1},
        {1,2,1,3,2,2,1,2,1,2,2,3,1,2,1},
        {1,2,1,2,1,2,1,2,1,2,1,2,1,2,1},
        {1,2,1,2,1,2,2,2,2,2,1,2,1,2,1},
        {1,2,2,2,1,1,1,1,1,1,1,2,2,2,1},
        {1,1,1,2,2,2,2,0,2,2,2,2,1,1,1},
        {1,2,2,2,1,1,1,1,1,1,1,2,2,2,1},
        {1,2,1,2,1,2,2,2,2,2,1,2,1,2,1},
        {1,2,1,2,1,2,1,2,1,2,1,2,1,2,1},
        {1,2,1,4,2,2,1,2,1,2,2,4,1,2,1},
        {1,2,1,1,1,1,1,2,1,1,1,1,1,2,1},
        {1,5,2,2,2,2,2,2,2,2,2,2,2,5,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    }
};"""

    content = content.replace("char maps[10][ROWS][COLS] = {", "char maps[15][ROWS][COLS] = {")
    content = content.replace("    }\n};", "    },\n" + maps_str)

    content = content.replace("Ghost ghosts[4];", "Ghost ghosts[5];\nint freezeTimer = 0;")
    
    content = content.replace("int mapIndex = (level - 1) % 10;", "int mapIndex = (level - 1) % 15;")
    content = content.replace("if (map[r][c] >= 2 && map[r][c] <= 4) dotCount++;", "if (map[r][c] >= 2 && map[r][c] <= 5) dotCount++;")
    content = content.replace("ghosts[3] = (Ghost){7, 7, RGB(255, 184, 82)};\n    frightTimer = 0;", "ghosts[3] = (Ghost){7, 7, RGB(255, 184, 82)};\n    ghosts[4] = (Ghost){7, 5, RGB(0, 255, 0)};\n    frightTimer = 0;\n    freezeTimer = 0;")

    update_code = """    if (gameOver || paused) return;
    
    if (freezeTimer > 0) freezeTimer--;
    int numGhosts = (level > 5) ? 5 : 4;
    
    // Ghost basic logic (random move)
    if (frightTimer > 0) frightTimer--;
    int ghostSpeed = 4 - (level / 3);
    if (ghostSpeed < 1) ghostSpeed = 1;
    if (frightTimer > 0) ghostSpeed *= 2;

    if (freezeTimer == 0 && frameCount % ghostSpeed == 0) {
        int dirs[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
        for(int i=0; i<numGhosts; i++) {"""
    
    old_update_code = """    if (gameOver || paused) return;
    
    // Ghost basic logic (random move)
    if (frightTimer > 0) frightTimer--;
    int ghostSpeed = 4 - (level / 3);
    if (ghostSpeed < 1) ghostSpeed = 1;
    if (frightTimer > 0) ghostSpeed *= 2;

    if (frameCount % ghostSpeed == 0) {
        int dirs[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
        for(int i=0; i<4; i++) {"""

    content = content.replace(old_update_code, update_code)

    old_ai = """                } else if (i == 3) { // Orange ghost scatters if too close
                    int distToPac = abs(ghosts[i].x - px) + abs(ghosts[i].y - py);
                    if (distToPac > 6) {
                        tx = px;
                        ty = py;
                    } else {
                        tx = 0;
                        ty = ROWS - 1;
                    }
                }
                int best_d = -1;"""
    new_ai = """                } else if (i == 3) { // Orange ghost scatters if too close
                    int distToPac = abs(ghosts[i].x - px) + abs(ghosts[i].y - py);
                    if (distToPac > 6) {
                        tx = px;
                        ty = py;
                    } else {
                        tx = 0;
                        ty = ROWS - 1;
                    }
                } else if (i == 4) { // Green ghost exactly tracks
                    tx = px;
                    ty = py;
                }
                int best_d = -1;"""
    content = content.replace(old_ai, new_ai)

    old_eat = """            if (map[py][px] >= 2 && map[py][px] <= 4) {
                if (map[py][px] == 3) { score += 40; frightTimer = 50; MessageBeep(MB_OK); }
                else if (map[py][px] == 4) { score += 20; speedTimer = 80; MessageBeep(MB_ICONEXCLAMATION); }
                else { score += 10; }"""
    new_eat = """            if (map[py][px] >= 2 && map[py][px] <= 5) {
                if (map[py][px] == 3) { score += 40; frightTimer = 50; MessageBeep(MB_OK); }
                else if (map[py][px] == 4) { score += 20; speedTimer = 80; MessageBeep(MB_ICONEXCLAMATION); }
                else if (map[py][px] == 5) { score += 30; freezeTimer = 100; MessageBeep(MB_ICONINFORMATION); }
                else { score += 10; }"""
    content = content.replace(old_eat, new_eat)

    old_for_col = """    for(int i=0; i<4; i++) {"""
    new_for_col = """    for(int i=0; i<numGhosts; i++) {"""
    content = content.replace(old_for_col, new_for_col)

    old_reset = """                    ghosts[0].x = 7; ghosts[0].y = 6;
                    ghosts[1].x = 6; ghosts[1].y = 7;
                    ghosts[2].x = 8; ghosts[2].y = 7;
                    ghosts[3].x = 7; ghosts[3].y = 7;"""
    new_reset = """                    ghosts[0].x = 7; ghosts[0].y = 6;
                    ghosts[1].x = 6; ghosts[1].y = 7;
                    ghosts[2].x = 8; ghosts[2].y = 7;
                    ghosts[3].x = 7; ghosts[3].y = 7;
                    ghosts[4].x = 7; ghosts[4].y = 5;"""
    content = content.replace(old_reset, new_reset)
    
    old_paint = """                    } else if (map[r][c] == 4) {
                        HBRUSH spBr = CreateSolidBrush(RGB(0, 255, 255));
                        RECT dr = {c * TS + 7, r * TS + 7, c * TS + 13, r * TS + 13};
                        FillRect(memDC, &dr, spBr);
                        DeleteObject(spBr);
                    }
                }"""
    new_paint = """                    } else if (map[r][c] == 4) {
                        HBRUSH spBr = CreateSolidBrush(RGB(0, 255, 255));
                        RECT dr = {c * TS + 7, r * TS + 7, c * TS + 13, r * TS + 13};
                        FillRect(memDC, &dr, spBr);
                        DeleteObject(spBr);
                    } else if (map[r][c] == 5) {
                        HBRUSH frBr = CreateSolidBrush(RGB(255, 255, 255));
                        RECT dr = {c * TS + 6, r * TS + 6, c * TS + 14, r * TS + 14};
                        FillRect(memDC, &dr, frBr);
                        DeleteObject(frBr);
                    }
                }"""
    content = content.replace(old_paint, new_paint)

    with open('d:/KiloApps/KPac/main.c', 'w', encoding='utf-8') as f:
        f.write(content)


def patch_html():
    with open('d:/KiloApps/KiloOS/public/apps/kpac.html', 'r', encoding='utf-8') as f:
        content = f.read()

    maps_str_html = """      [
          [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],
          [1,5,2,2,2,2,1,2,1,2,2,2,2,5,1],
          [1,2,1,1,1,2,1,2,1,2,1,1,1,2,1],
          [1,2,1,3,2,2,2,2,2,2,2,3,1,2,1],
          [1,2,1,2,1,1,1,1,1,1,1,2,1,2,1],
          [1,2,2,2,1,2,2,2,2,2,1,2,2,2,1],
          [1,1,1,2,1,2,1,1,1,2,1,2,1,1,1],
          [0,0,0,2,2,2,1,0,1,2,2,2,0,0,0],
          [1,1,1,2,1,2,1,1,1,2,1,2,1,1,1],
          [1,2,2,2,1,2,2,2,2,2,1,2,2,2,1],
          [1,2,1,2,1,1,1,1,1,1,1,2,1,2,1],
          [1,2,1,4,2,2,2,2,2,2,2,4,1,2,1],
          [1,2,1,1,1,2,1,2,1,2,1,1,1,2,1],
          [1,5,2,2,2,2,1,2,1,2,2,2,2,5,1],
          [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
      ],
      [
          [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],
          [1,3,2,2,2,2,2,1,2,2,2,2,2,3,1],
          [1,2,1,1,1,1,2,1,2,1,1,1,1,2,1],
          [1,2,2,2,5,1,2,2,2,1,5,2,2,2,1],
          [1,1,1,1,2,1,1,1,1,1,2,1,1,1,1],
          [1,2,2,2,2,2,2,2,2,2,2,2,2,2,1],
          [1,2,1,1,1,2,1,1,1,2,1,1,1,2,1],
          [1,2,1,0,1,2,1,0,1,2,1,0,1,2,1],
          [1,2,1,1,1,2,1,1,1,2,1,1,1,2,1],
          [1,2,2,2,2,2,2,2,2,2,2,2,2,2,1],
          [1,1,1,1,2,1,1,1,1,1,2,1,1,1,1],
          [1,2,2,2,4,1,2,2,2,1,4,2,2,2,1],
          [1,2,1,1,1,1,2,1,2,1,1,1,1,2,1],
          [1,3,2,2,2,2,2,1,2,2,2,2,2,3,1],
          [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
      ],
      [
          [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],
          [1,5,2,1,2,2,2,2,2,2,2,1,2,5,1],
          [1,1,2,1,2,1,1,1,1,1,2,1,2,1,1],
          [1,2,2,2,2,1,3,2,3,1,2,2,2,2,1],
          [1,2,1,1,2,1,2,1,2,1,2,1,1,2,1],
          [1,2,1,2,2,2,2,1,2,2,2,2,1,2,1],
          [1,2,1,2,1,1,1,1,1,1,1,2,1,2,1],
          [1,2,2,2,1,0,0,0,0,0,1,2,2,2,1],
          [1,2,1,2,1,1,1,1,1,1,1,2,1,2,1],
          [1,2,1,2,2,2,2,1,2,2,2,2,1,2,1],
          [1,2,1,1,2,1,2,1,2,1,2,1,1,2,1],
          [1,2,2,2,2,1,4,2,4,1,2,2,2,2,1],
          [1,1,2,1,2,1,1,1,1,1,2,1,2,1,1],
          [1,5,2,1,2,2,2,2,2,2,2,1,2,5,1],
          [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
      ],
      [
          [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],
          [1,3,2,2,2,1,2,5,2,1,2,2,2,3,1],
          [1,2,1,1,2,1,2,1,2,1,2,1,1,2,1],
          [1,2,2,2,2,2,2,1,2,2,2,2,2,2,1],
          [1,1,1,1,1,2,1,1,1,2,1,1,1,1,1],
          [1,2,2,2,1,2,2,2,2,2,1,2,2,2,1],
          [1,2,1,2,1,1,1,1,1,1,1,2,1,2,1],
          [0,2,1,2,2,2,2,0,2,2,2,2,1,2,0],
          [1,2,1,2,1,1,1,1,1,1,1,2,1,2,1],
          [1,2,2,2,1,2,2,2,2,2,1,2,2,2,1],
          [1,1,1,1,1,2,1,1,1,2,1,1,1,1,1],
          [1,2,2,2,2,2,2,1,2,2,2,2,2,2,1],
          [1,2,1,1,2,1,2,1,2,1,2,1,1,2,1],
          [1,3,2,2,2,1,2,5,2,1,2,2,2,3,1],
          [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
      ],
      [
          [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],
          [1,5,2,2,2,2,2,2,2,2,2,2,2,5,1],
          [1,2,1,1,1,1,1,2,1,1,1,1,1,2,1],
          [1,2,1,3,2,2,1,2,1,2,2,3,1,2,1],
          [1,2,1,2,1,2,1,2,1,2,1,2,1,2,1],
          [1,2,1,2,1,2,2,2,2,2,1,2,1,2,1],
          [1,2,2,2,1,1,1,1,1,1,1,2,2,2,1],
          [1,1,1,2,2,2,2,0,2,2,2,2,1,1,1],
          [1,2,2,2,1,1,1,1,1,1,1,2,2,2,1],
          [1,2,1,2,1,2,2,2,2,2,1,2,1,2,1],
          [1,2,1,2,1,2,1,2,1,2,1,2,1,2,1],
          [1,2,1,4,2,2,1,2,1,2,2,4,1,2,1],
          [1,2,1,1,1,1,1,2,1,1,1,1,1,2,1],
          [1,5,2,2,2,2,2,2,2,2,2,2,2,5,1],
          [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
      ]
  ];"""

    content = content.replace("      ]\n  ];", "      ],\n" + maps_str_html)
    content = content.replace("let speedTimer = 0;", "let speedTimer = 0;\n  let freezeTimer = 0;")
    content = content.replace("const mapIndex = (level - 1) % 10;", "const mapIndex = (level - 1) % 15;")
    content = content.replace("{x: 7, y: 7, color: '#ffb852'}", "{x: 7, y: 7, color: '#ffb852'},\n          {x: 7, y: 5, color: '#00ff00'}")
    content = content.replace("frightTimer = 0;", "frightTimer = 0;\n      freezeTimer = 0;")
    content = content.replace("if (map[r][c] >= 2 && map[r][c] <= 4) dotCount++;", "if (map[r][c] >= 2 && map[r][c] <= 5) dotCount++;")

    old_update_start = """      if (frightTimer > 0) frightTimer--;
      
      let ghostSpeed = Math.max(1, 4 - Math.floor(level/3));
      if (frightTimer > 0) ghostSpeed *= 2;
      
      if (frameCount % ghostSpeed === 0) {"""
    new_update_start = """      if (freezeTimer > 0) freezeTimer--;
      
      if (frightTimer > 0) frightTimer--;
      
      let ghostSpeed = Math.max(1, 4 - Math.floor(level/3));
      if (frightTimer > 0) ghostSpeed *= 2;
      let numGhosts = (level > 5) ? ghosts.length : 4;
      
      if (freezeTimer === 0 && frameCount % ghostSpeed === 0) {"""
    content = content.replace(old_update_start, new_update_start)

    content = content.replace("for (let i = 0; i < ghosts.length; i++) {", "for (let i = 0; i < numGhosts; i++) {")

    old_ghost_ai = """                  } else if (i === 3) { // Orange ghost logic
                      let distToPac = Math.abs(g.x - px) + Math.abs(g.y - py);
                      if (distToPac > 6) {
                          tx = px; ty = py;
                      } else {
                          tx = 0; ty = ROWS - 1;
                      }
                  }"""
    new_ghost_ai = """                  } else if (i === 3) { // Orange ghost logic
                      let distToPac = Math.abs(g.x - px) + Math.abs(g.y - py);
                      if (distToPac > 6) {
                          tx = px; ty = py;
                      } else {
                          tx = 0; ty = ROWS - 1;
                      }
                  } else if (i === 4) { // Green ghost exactly tracks
                      tx = px; ty = py;
                  }"""
    content = content.replace(old_ghost_ai, new_ghost_ai)

    old_eat_html = """              if (map[py][px] >= 2 && map[py][px] <= 4) {
                  if (map[py][px] === 3) { score += 40; frightTimer = 50; playSound(300, 0.3, 'sawtooth'); }
                  else if (map[py][px] === 4) { score += 20; speedTimer = 80; playSound(900, 0.1, 'triangle'); }
                  else { score += 10; playSound(600, 0.05, 'square'); }"""
    new_eat_html = """              if (map[py][px] >= 2 && map[py][px] <= 5) {
                  if (map[py][px] === 3) { score += 40; frightTimer = 50; playSound(300, 0.3, 'sawtooth'); }
                  else if (map[py][px] === 4) { score += 20; speedTimer = 80; playSound(900, 0.1, 'triangle'); }
                  else if (map[py][px] === 5) { score += 30; freezeTimer = 100; playSound(700, 0.2, 'sine'); }
                  else { score += 10; playSound(600, 0.05, 'square'); }"""
    content = content.replace(old_eat_html, new_eat_html)

    # ghost collision - need to use slice of ghosts for collision to only include active ghosts
    content = content.replace("for (let g of ghosts) {", "for (let i = 0; i < numGhosts; i++) {\n          let g = ghosts[i];")
    
    old_reset_html = """                      ghosts[0] = {x: 7, y: 6, color: '#ff0000'};
                      ghosts[1] = {x: 6, y: 7, color: '#ffb8ff'};
                      ghosts[2] = {x: 8, y: 7, color: '#00ffff'};
                      ghosts[3] = {x: 7, y: 7, color: '#ffb852'};"""
    new_reset_html = """                      ghosts[0] = {x: 7, y: 6, color: '#ff0000'};
                      ghosts[1] = {x: 6, y: 7, color: '#ffb8ff'};
                      ghosts[2] = {x: 8, y: 7, color: '#00ffff'};
                      ghosts[3] = {x: 7, y: 7, color: '#ffb852'};
                      ghosts[4] = {x: 7, y: 5, color: '#00ff00'};"""
    content = content.replace(old_reset_html, new_reset_html)

    old_paint_html = """              } else if (map[r][c] === 4) {
                  ctx.fillStyle = '#00ffff';
                  ctx.fillRect(c * TS + 7, r * TS + 7, 6, 6);
              }
          }
      }"""
    new_paint_html = """              } else if (map[r][c] === 4) {
                  ctx.fillStyle = '#00ffff';
                  ctx.fillRect(c * TS + 7, r * TS + 7, 6, 6);
              } else if (map[r][c] === 5) {
                  ctx.fillStyle = '#ffffff';
                  ctx.fillRect(c * TS + 6, r * TS + 6, 8, 8);
              }
          }
      }"""
    content = content.replace(old_paint_html, new_paint_html)

    content = content.replace("for (let g of ghosts) {", "for (let i = 0; i < numGhosts; i++) {\n          let g = ghosts[i];")

    with open('d:/KiloApps/KiloOS/public/apps/kpac.html', 'w', encoding='utf-8') as f:
        f.write(content)

patch_main_c()
patch_html()
