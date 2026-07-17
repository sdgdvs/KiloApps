import sys

# --- Patch main.c ---
with open(r'd:\KiloApps\KTetris\main.c', 'r') as f:
    code_c = f.read()

# Add states and vars
code_c = code_c.replace('int game_over = 0;\nint is_paused = 0;\nint score = 0;\nint lines = 0;\nint level = 1;\nint combo = 0;\nint timer_speed = 500;', '''int game_over = 0;
int is_paused = 0;
int start_screen = 1;
int win_screen = 0;
int game_mode = 0;
int campaign_level = 1;
int stat_lines[4] = {0, 0, 0, 0};
int score = 0;
int lines = 0;
int level = 1;
int combo = 0;
int timer_speed = 500;''')

# Add garbage color
code_c = code_c.replace('const COLORREF colors[8] = {', 'const COLORREF colors[9] = {')
code_c = code_c.replace('RGB(255,0,0)    // Z\n};', '''RGB(255,0,0),   // Z
    RGB(128,128,128) // Garbage
};''')

# Modify lock_piece
old_lock = '''    if (lines_cleared > 0) {
        combo++;
        Beep(1000 + lines_cleared * 200 + combo * 100, 100);
    } else {
        combo = 0;
        Beep(500, 50);
    }
    lines += lines_cleared;
    level = (lines / 10) + 1;
    score += lines_cleared * 100 * level * (combo > 0 ? combo : 1);
    int new_speed = 500 - (level - 1) * 30;
    if (new_speed < 50) new_speed = 50;
    timer_speed = new_speed;
}'''

new_lock = '''    if (lines_cleared > 0) {
        if (lines_cleared >= 1 && lines_cleared <= 4) stat_lines[lines_cleared - 1]++;
        combo++;
        Beep(1000 + lines_cleared * 200 + combo * 100, 100);
    } else {
        combo = 0;
        Beep(500, 50);
    }
    lines += lines_cleared;
    
    if (game_mode == 1) {
        int goal = campaign_level * 10;
        level = campaign_level;
        score += lines_cleared * 100 * level * (combo > 0 ? combo : 1);
        if (lines >= goal) {
            campaign_level++;
            if (campaign_level > 5) {
                win_screen = 1;
                Beep(800, 150); Beep(1000, 150); Beep(1200, 300);
                return;
            } else {
                InitGame();
                return;
            }
        }
    } else {
        level = (lines / 10) + 1;
        score += lines_cleared * 100 * level * (combo > 0 ? combo : 1);
        int new_speed = 500 - (level - 1) * 30;
        if (new_speed < 50) new_speed = 50;
        timer_speed = new_speed;
    }
}'''
code_c = code_c.replace(old_lock, new_lock)

# Modify InitGame
old_init = '''void InitGame() {
    rng_state = GetTickCount();
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            grid[y][x] = 0;
    score = 0;
    lines = 0;
    level = 1;
    combo = 0;
    timer_speed = 500;
    bag_index = 7;
    game_over = 0;
    is_paused = 0;
    hold_piece = -1;
    hold_used = 0;
    next_piece = get_next_pc();
    next_rot = 0;
    spawn_piece();
}'''

new_init = '''void InitGame() {
    rng_state = GetTickCount();
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            grid[y][x] = 0;
            
    if (game_mode == 1) {
        int garbage = campaign_level * 2;
        for (int r = 0; r < garbage; r++) {
            int hole = random_int(W);
            for (int x = 0; x < W; x++) {
                grid[H - 1 - r][x] = (x == hole) ? 0 : 8;
            }
        }
        lines = 0;
        level = campaign_level;
        timer_speed = 500 - (level - 1) * 30;
    } else {
        lines = 0;
        level = 1;
        timer_speed = 500;
        for (int i=0; i<4; i++) stat_lines[i] = 0;
    }
    combo = 0;
    bag_index = 7;
    game_over = 0;
    is_paused = 0;
    win_screen = 0;
    hold_piece = -1;
    hold_used = 0;
    next_piece = get_next_pc();
    next_rot = 0;
    spawn_piece();
}'''
code_c = code_c.replace(old_init, new_init)

# Modify timer and keys
code_c = code_c.replace('if (!game_over && !is_paused) {', 'if (!game_over && !is_paused && !start_screen && !win_screen) {', 1)

code_c = code_c.replace('''                } else {
                    lock_piece();
                    spawn_piece();
                    SetTimer(hwnd, TIMER_ID, timer_speed, NULL);
                }''', '''                } else {
                    int old_level = campaign_level;
                    lock_piece();
                    if (!win_screen && (game_mode == 0 || campaign_level == old_level)) {
                        spawn_piece();
                    }
                    SetTimer(hwnd, TIMER_ID, timer_speed, NULL);
                }''')

code_c = code_c.replace('''        case WM_KEYDOWN:
            if (game_over && wParam == VK_RETURN) {
                InitGame();
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (!game_over) {
                if (wParam == 'P') {''', '''        case WM_KEYDOWN:
            if (start_screen) {
                if (wParam == '1') { game_mode = 0; start_screen = 0; score = 0; InitGame(); }
                if (wParam == '2') { game_mode = 1; start_screen = 0; campaign_level = 1; score = 0; InitGame(); }
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (win_screen && wParam == VK_RETURN) {
                start_screen = 1; win_screen = 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (game_over && wParam == VK_RETURN) {
                start_screen = 1; game_over = 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            } else if (!game_over && !win_screen) {
                if (wParam == 'P') {''')

code_c = code_c.replace('''                if (wParam == VK_SPACE) {
                    while (!check_collision(current_piece, current_rot, current_x, current_y + 1)) {
                        current_y++;
                    }
                    lock_piece();
                    spawn_piece();
                    SetTimer(hwnd, TIMER_ID, timer_speed, NULL);
                }''', '''                if (wParam == VK_SPACE) {
                    while (!check_collision(current_piece, current_rot, current_x, current_y + 1)) {
                        current_y++;
                    }
                    int old_level = campaign_level;
                    lock_piece();
                    if (!win_screen && (game_mode == 0 || campaign_level == old_level)) {
                        spawn_piece();
                    }
                    SetTimer(hwnd, TIMER_ID, timer_speed, NULL);
                }''')

# Modify paint logic
code_c = code_c.replace('HBRUSH brushes[8];', 'HBRUSH brushes[9];')
code_c = code_c.replace('for (int i = 0; i < 8; i++) brushes[i] = CreateSolidBrush(colors[i]);', 'for (int i = 0; i < 9; i++) brushes[i] = CreateSolidBrush(colors[i]);')
code_c = code_c.replace('for (int i = 0; i < 8; i++) DeleteObject(brushes[i]);', 'for (int i = 0; i < 9; i++) DeleteObject(brushes[i]);')
code_c = code_c.replace('if (!game_over && !is_paused) {', 'if (!game_over && !is_paused && !start_screen && !win_screen) {')

old_go_txt = '''            } else if (game_over) {
                SetTextColor(memDC, RGB(255, 0, 0));
                SetBkMode(memDC, TRANSPARENT);
                TextOutA(memDC, 20, H * CELL_SIZE / 2, "GAME OVER", 9);
            } else if (is_paused) {'''
new_go_txt = '''            } else if (game_over) {
                SetTextColor(memDC, RGB(255, 0, 0));
                SetBkMode(memDC, TRANSPARENT);
                TextOutA(memDC, 20, H * CELL_SIZE / 2, "GAME OVER", 9);
            } else if (win_screen) {
                SetTextColor(memDC, RGB(0, 255, 0));
                SetBkMode(memDC, TRANSPARENT);
                TextOutA(memDC, 20, H * CELL_SIZE / 2, "YOU WIN!", 8);
                TextOutA(memDC, 20, H * CELL_SIZE / 2 + 20, "ENTER TO MENU", 13);
            } else if (start_screen) {
                SetTextColor(memDC, RGB(255, 255, 255));
                SetBkMode(memDC, TRANSPARENT);
                TextOutA(memDC, 40, H * CELL_SIZE / 2 - 40, "KTETRIS", 7);
                TextOutA(memDC, 40, H * CELL_SIZE / 2, "1. Endless", 10);
                TextOutA(memDC, 40, H * CELL_SIZE / 2 + 20, "2. Campaign", 11);
            } else if (is_paused) {'''
code_c = code_c.replace(old_go_txt, new_go_txt)

old_ui_txt = '''            char level_str[32];
            wsprintfA(level_str, "LEVEL: %d", level);
            TextOutA(memDC, W * CELL_SIZE + 10, 60, level_str, lstrlenA(level_str));
            
            char lines_str[32];
            wsprintfA(lines_str, "LINES: %d", lines);
            TextOutA(memDC, W * CELL_SIZE + 10, 80, lines_str, lstrlenA(lines_str));
            
            if (combo > 1) {'''
new_ui_txt = '''            char level_str[32];
            if (game_mode == 1) wsprintfA(level_str, "STAGE: %d/5", campaign_level);
            else wsprintfA(level_str, "LEVEL: %d", level);
            TextOutA(memDC, W * CELL_SIZE + 10, 60, level_str, lstrlenA(level_str));
            
            char lines_str[32];
            if (game_mode == 1) wsprintfA(lines_str, "LINES: %d/%d", lines, campaign_level * 10);
            else wsprintfA(lines_str, "LINES: %d", lines);
            TextOutA(memDC, W * CELL_SIZE + 10, 80, lines_str, lstrlenA(lines_str));
            
            if (game_mode == 0 && !start_screen) {
                char stat1[32], stat2[32], stat3[32], stat4[32];
                wsprintfA(stat1, "SGL: %d", stat_lines[0]);
                wsprintfA(stat2, "DBL: %d", stat_lines[1]);
                wsprintfA(stat3, "TPL: %d", stat_lines[2]);
                wsprintfA(stat4, "TET: %d", stat_lines[3]);
                SetTextColor(memDC, RGB(150, 150, 255));
                TextOutA(memDC, W * CELL_SIZE + 10, 290, stat1, lstrlenA(stat1));
                TextOutA(memDC, W * CELL_SIZE + 10, 310, stat2, lstrlenA(stat2));
                TextOutA(memDC, W * CELL_SIZE + 10, 330, stat3, lstrlenA(stat3));
                TextOutA(memDC, W * CELL_SIZE + 10, 350, stat4, lstrlenA(stat4));
                SetTextColor(memDC, RGB(255, 255, 255));
            }
            
            if (combo > 1) {'''
code_c = code_c.replace(old_ui_txt, new_ui_txt)

with open(r'd:\KiloApps\KTetris\main.c', 'w') as f:
    f.write(code_c)

# --- Patch ktetris.html ---
with open(r'd:\KiloApps\KiloOS\public\apps\ktetris.html', 'r') as f:
    code_h = f.read()

code_h = code_h.replace("const colors = ['#000', '#0ff', '#00f', '#fa0', '#ff0', '#0f0', '#808', '#f00'];", "const colors = ['#000', '#0ff', '#00f', '#fa0', '#ff0', '#0f0', '#808', '#f00', '#888'];")

code_h = code_h.replace('let pc, rot, cx, cy, gameOver = false, isPaused = false, score = 0, lines = 0, level = 1, combo = 0;', '''let pc, rot, cx, cy, gameOver = false, isPaused = false, score = 0, lines = 0, level = 1, combo = 0;
  let startScreen = true, winScreen = false, gameMode = 0, campaignLevel = 1;
  let statLines = [0, 0, 0, 0];''')

old_h_lock = '''      if (linesCleared > 0) {
          combo++;
          playSound(1000 + linesCleared * 200 + combo * 100, 'square', 0.1);
      } else {
          combo = 0;
          playSound(500, 'square', 0.05);
      }
      
      lines += linesCleared;
      level = Math.floor(lines / 10) + 1;
      score += linesCleared * 100 * level * (combo > 0 ? combo : 1);
      dropInterval = Math.max(50, 500 - (level - 1) * 30);
  }'''

new_h_lock = '''      if (linesCleared > 0) {
          if (linesCleared >= 1 && linesCleared <= 4) statLines[linesCleared - 1]++;
          combo++;
          playSound(1000 + linesCleared * 200 + combo * 100, 'square', 0.1);
      } else {
          combo = 0;
          playSound(500, 'square', 0.05);
      }
      
      lines += linesCleared;
      
      if (gameMode === 1) {
          let goal = campaignLevel * 10;
          level = campaignLevel;
          score += linesCleared * 100 * level * (combo > 0 ? combo : 1);
          if (lines >= goal) {
              campaignLevel++;
              if (campaignLevel > 5) {
                  winScreen = true;
                  playSound(800, 'square', 0.15); playSound(1000, 'square', 0.15); playSound(1200, 'square', 0.3);
                  return;
              } else {
                  init();
                  return;
              }
          }
      } else {
          level = Math.floor(lines / 10) + 1;
          score += linesCleared * 100 * level * (combo > 0 ? combo : 1);
          dropInterval = Math.max(50, 500 - (level - 1) * 30);
      }
  }'''
code_h = code_h.replace(old_h_lock, new_h_lock)

old_h_init = '''  function init() {
      grid = Array(H).fill().map(() => Array(W).fill(0));
      gameOver = false;
      isPaused = false;
      score = 0;
      lines = 0;
      level = 1;
      combo = 0;
      dropInterval = 500;
      bag = [];
      nextPc = getNextPc();
      nextRot = 0;
      holdPc = -1;
      holdUsed = false;
      spawn();
  }'''

new_h_init = '''  function init() {
      grid = Array(H).fill().map(() => Array(W).fill(0));
      if (gameMode === 1) {
          let garbage = campaignLevel * 2;
          for (let r = 0; r < garbage; r++) {
              let hole = Math.floor(Math.random() * W);
              for (let x = 0; x < W; x++) {
                  grid[H - 1 - r][x] = (x === hole) ? 0 : 8;
              }
          }
          lines = 0;
          level = campaignLevel;
          dropInterval = Math.max(50, 500 - (level - 1) * 30);
      } else {
          lines = 0;
          level = 1;
          dropInterval = 500;
          statLines = [0, 0, 0, 0];
      }
      gameOver = false;
      isPaused = false;
      winScreen = false;
      combo = 0;
      bag = [];
      nextPc = getNextPc();
      nextRot = 0;
      holdPc = -1;
      holdUsed = false;
      spawn();
  }'''
code_h = code_h.replace(old_h_init, new_h_init)

old_h_keys = '''  document.addEventListener('keydown', e => {
      if (['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight', ' '].includes(e.key)) {
          e.preventDefault();
      }
      if (gameOver && e.key === 'Enter') init();
      if (gameOver) return;
      if (e.key.toLowerCase() === 'p') { isPaused = !isPaused; return; }
      if (isPaused) return;'''
new_h_keys = '''  document.addEventListener('keydown', e => {
      if (['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight', ' '].includes(e.key)) {
          e.preventDefault();
      }
      if (startScreen) {
          if (e.key === '1') { gameMode = 0; startScreen = false; score = 0; init(); }
          if (e.key === '2') { gameMode = 1; startScreen = false; campaignLevel = 1; score = 0; init(); }
          return;
      }
      if (winScreen && e.key === 'Enter') { startScreen = true; winScreen = false; return; }
      if (gameOver && e.key === 'Enter') { startScreen = true; gameOver = false; return; }
      if (gameOver || winScreen) return;
      if (e.key.toLowerCase() === 'p') { isPaused = !isPaused; return; }
      if (isPaused) return;'''
code_h = code_h.replace(old_h_keys, new_h_keys)

old_h_space = '''      if (e.key === ' ') {
          while (!checkCol(pc, rot, cx, cy + 1)) cy++;
          lock(); spawn();
      }
      draw();'''
new_h_space = '''      if (e.key === ' ') {
          while (!checkCol(pc, rot, cx, cy + 1)) cy++;
          let oldLevel = campaignLevel;
          lock();
          if (!winScreen && (gameMode === 0 || campaignLevel === oldLevel)) spawn();
      }
      draw();'''
code_h = code_h.replace(old_h_space, new_h_space)

old_h_draw = '''      if (!gameOver && !isPaused) {
          // Draw Ghost piece'''
new_h_draw = '''      if (!gameOver && !isPaused && !startScreen && !winScreen) {
          // Draw Ghost piece'''
code_h = code_h.replace(old_h_draw, new_h_draw)

old_h_txt = '''      } else if (gameOver) {
          ctx.fillStyle = 'red';
          ctx.font = '20px monospace';
          ctx.fillText('GAME OVER', 50, 200);
      } else if (isPaused) {
          ctx.fillStyle = 'yellow';
          ctx.font = '20px monospace';
          ctx.fillText('PAUSED', 65, 200);
      }'''
new_h_txt = '''      } else if (gameOver) {
          ctx.fillStyle = 'red';
          ctx.font = '20px monospace';
          ctx.fillText('GAME OVER', 50, 200);
      } else if (winScreen) {
          ctx.fillStyle = '#0f0';
          ctx.font = '20px monospace';
          ctx.fillText('YOU WIN!', 60, 190);
          ctx.font = '14px monospace';
          ctx.fillText('ENTER TO MENU', 50, 210);
      } else if (startScreen) {
          ctx.fillStyle = 'white';
          ctx.font = '20px monospace';
          ctx.fillText('KTETRIS', 65, 170);
          ctx.font = '14px monospace';
          ctx.fillText('1. Endless', 65, 200);
          ctx.fillText('2. Campaign', 65, 220);
      } else if (isPaused) {
          ctx.fillStyle = 'yellow';
          ctx.font = '20px monospace';
          ctx.fillText('PAUSED', 65, 200);
      }'''
code_h = code_h.replace(old_h_txt, new_h_txt)

old_h_ui = '''      ctx.fillText('LEVEL: ' + level, 210, 60);
      ctx.fillText('LINES: ' + lines, 210, 80);
      
      if (combo > 1) {'''
new_h_ui = '''      ctx.fillText(gameMode === 1 ? 'STAGE: ' + campaignLevel + '/5' : 'LEVEL: ' + level, 210, 60);
      ctx.fillText(gameMode === 1 ? 'LINES: ' + lines + '/' + (campaignLevel*10) : 'LINES: ' + lines, 210, 80);
      
      if (gameMode === 0 && !startScreen) {
          ctx.fillStyle = '#99f';
          ctx.fillText('SGL: ' + statLines[0], 210, 290);
          ctx.fillText('DBL: ' + statLines[1], 210, 310);
          ctx.fillText('TPL: ' + statLines[2], 210, 330);
          ctx.fillText('TET: ' + statLines[3], 210, 350);
          ctx.fillStyle = 'white';
      }
      
      if (combo > 1) {'''
code_h = code_h.replace(old_h_ui, new_h_ui)

old_h_upd = '''      if (dropCounter > dropInterval) {
          if (!gameOver && !isPaused) {
              if (!checkCol(pc, rot, cx, cy + 1)) cy++;
              else { lock(); spawn(); }
              draw();
          }
          dropCounter = 0;
      }
      if (isPaused) draw();
      requestAnimationFrame(update);'''
new_h_upd = '''      if (dropCounter > dropInterval) {
          if (!gameOver && !isPaused && !startScreen && !winScreen) {
              if (!checkCol(pc, rot, cx, cy + 1)) cy++;
              else { 
                  let oldLevel = campaignLevel;
                  lock(); 
                  if (!winScreen && (gameMode === 0 || campaignLevel === oldLevel)) spawn(); 
              }
              draw();
          }
          dropCounter = 0;
      }
      if (isPaused || startScreen || winScreen) draw();
      requestAnimationFrame(update);'''
code_h = code_h.replace(old_h_upd, new_h_upd)

with open(r'd:\KiloApps\KiloOS\public\apps\ktetris.html', 'w') as f:
    f.write(code_h)
