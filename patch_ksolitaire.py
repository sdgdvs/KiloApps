import os
import re

# --- UPDATE C VERSION ---
with open('d:/KiloApps/KSolitaire/main.c', 'r') as f:
    c_code = f.read()

c_code = c_code.replace('#define ID_EXPERT 1003', '#define ID_EXPERT 1003\n#define ID_CAMPAIGN 1006')
c_code = c_code.replace('AppendMenu(hSubMenu, MF_STRING, ID_EXPERT, "Expert (8x4)");', 'AppendMenu(hSubMenu, MF_STRING, ID_EXPERT, "Expert (8x4)");\n            AppendMenu(hSubMenu, MF_STRING, ID_CAMPAIGN, "Campaign Mode");')
c_code = c_code.replace('} else if (LOWORD(wParam) == ID_EXPERT) {\n                SetDifficulty(hwnd, 2);', '} else if (LOWORD(wParam) == ID_EXPERT) {\n                SetDifficulty(hwnd, 2);\n            } else if (LOWORD(wParam) == ID_CAMPAIGN) {\n                SetDifficulty(hwnd, 3);')

c_code = c_code.replace('int cards[36];', 'int cards[64];')
c_code = c_code.replace('int flipped[36] = {0};', 'int flipped[64] = {0};')
c_code = c_code.replace('int matched[36] = {0};', 'int matched[64] = {0};')

c_code = c_code.replace('''int best_easy = -1;
int best_hard = -1;
int best_expert = -1;''', '''int best_easy = -1;
int best_hard = -1;
int best_expert = -1;
int stats_pairs = 0;
int stats_wins = 0;
int stats_campaign = 0;
int campaign_level = 1;''')

c_code = c_code.replace('''        fread(&best_expert, sizeof(int), 1, f);
        fclose(f);''', '''        fread(&best_expert, sizeof(int), 1, f);
        fread(&stats_pairs, sizeof(int), 1, f);
        fread(&stats_wins, sizeof(int), 1, f);
        fread(&stats_campaign, sizeof(int), 1, f);
        fclose(f);''')

c_code = c_code.replace('''        fwrite(&best_expert, sizeof(int), 1, f);
        fclose(f);''', '''        fwrite(&best_expert, sizeof(int), 1, f);
        fwrite(&stats_pairs, sizeof(int), 1, f);
        fwrite(&stats_wins, sizeof(int), 1, f);
        fwrite(&stats_campaign, sizeof(int), 1, f);
        fclose(f);''')

shuffle_old = '''    int numCards = ROWS * COLS;
    for (int i = 0; i < numCards; i++) {
        cards[i] = i / 2;
        flipped[i] = 0;
        matched[i] = 0;
    }'''
shuffle_new = '''    int numCards = ROWS * COLS;
    for (int i = 0; i < numCards; i++) {
        int val = i / 2;
        // Inject Power-Ups in Campaign Mode or Expert
        if (is_hard >= 2 && numCards >= 24) {
            if (val == (numCards/2 - 1)) val = 100; // Clock
            else if (val == (numCards/2 - 2)) val = 101; // Shuffle Trap
        }
        cards[i] = val;
        flipped[i] = 0;
        matched[i] = 0;
    }'''
c_code = c_code.replace(shuffle_old, shuffle_new)

diff_old = '''void SetDifficulty(HWND hwnd, int hard) {
    is_hard = hard;
    if (hard == 2) {
        ROWS = 4; COLS = 8; W = 800; H = 400;
    } else if (hard == 1) {
        ROWS = 4; COLS = 6; W = 600; H = 400;
    } else {
        ROWS = 4; COLS = 4; W = 400; H = 400;
    }'''
diff_new = '''void SetDifficulty(HWND hwnd, int hard) {
    is_hard = hard;
    if (hard == 3) {
        if (campaign_level == 1) { ROWS = 4; COLS = 4; W = 400; H = 400; }
        else if (campaign_level == 2) { ROWS = 4; COLS = 6; W = 600; H = 400; }
        else if (campaign_level == 3) { ROWS = 4; COLS = 8; W = 800; H = 400; }
        else if (campaign_level == 4) { ROWS = 6; COLS = 8; W = 800; H = 600; }
        else { ROWS = 8; COLS = 8; W = 800; H = 800; }
    } else if (hard == 2) {
        ROWS = 4; COLS = 8; W = 800; H = 400;
    } else if (hard == 1) {
        ROWS = 4; COLS = 6; W = 600; H = 400;
    } else {
        ROWS = 4; COLS = 4; W = 400; H = 400;
    }'''
c_code = c_code.replace(diff_old, diff_new)

c_code = c_code.replace('''        char text[8] = {0};
        if (theme == 0) {''', '''        char text[8] = {0};
        if (cards[idx] == 100) { strcpy(text, "+T"); }
        else if (cards[idx] == 101) { strcpy(text, "??"); }
        else if (theme == 0) {''')
c_code = c_code.replace('''            char sym[] = "!@#$%^&*+=?~OX<>";
            text[0] = sym[cards[idx]];''', '''            char sym[] = "!@#$%^&*+=?~OX<>";
            text[0] = sym[cards[idx] % 16];''')

move_old = '''                            matched[firstFlip] = 1;
                            matched[secondFlip] = 1;
                            firstFlip = -1;
                            secondFlip = -1;
                            matches++;
                            MessageBeep(MB_ICONASTERISK);
                            if (matches == (ROWS * COLS) / 2) {'''
move_new = '''                            matched[firstFlip] = 1;
                            matched[secondFlip] = 1;
                            stats_pairs++;
                            if (cards[firstFlip] == 100) {
                                score += 500;
                                elapsed_time = (elapsed_time > 10) ? elapsed_time - 10 : 0;
                            } else if (cards[firstFlip] == 101) {
                                // Shuffle remaining
                                int numCards = ROWS * COLS;
                                for (int i = numCards - 1; i > 0; i--) {
                                    if (!matched[i]) {
                                        int j = rnd() % (i + 1);
                                        while(matched[j]) j = rnd() % (i + 1);
                                        if (!matched[j]) {
                                            int temp = cards[i];
                                            cards[i] = cards[j];
                                            cards[j] = temp;
                                        }
                                    }
                                }
                            }
                            firstFlip = -1;
                            secondFlip = -1;
                            matches++;
                            MessageBeep(MB_ICONASTERISK);
                            if (matches == (ROWS * COLS) / 2) {'''
c_code = c_code.replace(move_old, move_new)

win_old = '''                                if (is_new_best) SaveScores();
                                
                                char msgBuf[256];
                                sprintf(msgBuf, "You won with score %d in %d moves and %d seconds!", score, moves, elapsed_time);
                                MessageBoxA(hwnd, msgBuf, "KMemory", MB_OK);
                                Shuffle(hwnd);'''
win_new = '''                                stats_wins++;
                                if (is_hard == 3) {
                                    stats_campaign++;
                                    campaign_level++;
                                    if (campaign_level > 5) {
                                        MessageBoxA(hwnd, "You beat the Campaign!", "KMemory", MB_OK);
                                        campaign_level = 1;
                                    }
                                }
                                SaveScores();
                                
                                char msgBuf[256];
                                sprintf(msgBuf, "You won with score %d in %d moves and %d seconds!", score, moves, elapsed_time);
                                MessageBoxA(hwnd, msgBuf, "KMemory", MB_OK);
                                SetDifficulty(hwnd, is_hard);'''
c_code = c_code.replace(win_old, win_new)
c_code = c_code.replace('SetDifficulty(hwnd, is_hard); // inside command', 'if(is_hard==3) campaign_level=1; SetDifficulty(hwnd, is_hard);')
c_code = c_code.replace('''            } else if (LOWORD(wParam) == ID_RESTART) {
                SetDifficulty(hwnd, is_hard);''', '''            } else if (LOWORD(wParam) == ID_RESTART) {
                if (is_hard == 3) campaign_level = 1;
                SetDifficulty(hwnd, is_hard);''')

status_old = '''            char statusText[256];
            int best = (is_hard == 2) ? best_expert : (is_hard == 1 ? best_hard : best_easy);
            if (best == -1)
                sprintf(statusText, "Time: %ds   Moves: %d   Score: %d (x%d)   Best: -", elapsed_time, moves, score, combo);
            else
                sprintf(statusText, "Time: %ds   Moves: %d   Score: %d (x%d)   Best: %d", elapsed_time, moves, score, combo, best);
            TextOutA(memDC, 10, 10, statusText, strlen(statusText));'''
status_new = '''            char statusText[256];
            int best = (is_hard == 2) ? best_expert : (is_hard == 1 ? best_hard : best_easy);
            if (is_hard == 3)
                sprintf(statusText, "Campaign %d/5 | Time: %ds  Moves: %d  Score: %d | Total Wins: %d", campaign_level, elapsed_time, moves, score, stats_wins);
            else if (best == -1)
                sprintf(statusText, "Time: %ds  Moves: %d  Score: %d (x%d)  Best: - | Pairs: %d", elapsed_time, moves, score, combo, stats_pairs);
            else
                sprintf(statusText, "Time: %ds  Moves: %d  Score: %d (x%d)  Best: %d | Pairs: %d", elapsed_time, moves, score, combo, best, stats_pairs);
            TextOutA(memDC, 10, 10, statusText, strlen(statusText));'''
c_code = c_code.replace(status_old, status_new)

with open('d:/KiloApps/KSolitaire/main.c', 'w') as f:
    f.write(c_code)

# --- UPDATE HTML VERSION ---
with open('d:/KiloApps/KiloOS/public/apps/ksolitaire.html', 'r') as f:
    h_code = f.read()

h_code = h_code.replace('''  .grid.easy { grid-template-columns: repeat(4, 1fr); width: 380px; height: 380px; }
  .grid.hard { grid-template-columns: repeat(6, 1fr); width: 570px; height: 380px; }
  .grid.expert { grid-template-columns: repeat(8, 1fr); width: 760px; height: 380px; }''', '''  .grid.easy { grid-template-columns: repeat(4, 1fr); width: 380px; height: 380px; }
  .grid.hard { grid-template-columns: repeat(6, 1fr); width: 570px; height: 380px; }
  .grid.expert { grid-template-columns: repeat(8, 1fr); width: 760px; height: 380px; }
  .grid.camp4 { grid-template-columns: repeat(8, 1fr); width: 760px; height: 570px; }
  .grid.camp5 { grid-template-columns: repeat(8, 1fr); width: 760px; height: 760px; }''')

h_code = h_code.replace('''<div id="ui">
  <div>Time: <span id="time">0</span>s</div>
  <div>Moves: <span id="moves">0</span></div>
  <div>Score: <span id="score">0</span></div>
  <div>Combo: x<span id="combo">1</span></div>
  <div>Best Score: <span id="best">-</span></div>
</div>''', '''<div id="ui">
  <div>Time: <span id="time">0</span>s</div>
  <div>Moves: <span id="moves">0</span></div>
  <div>Score: <span id="score">0</span></div>
  <div>Combo: x<span id="combo">1</span></div>
  <div>Best Score: <span id="best">-</span></div>
  <div>Total Pairs: <span id="stats_pairs">0</span></div>
  <div>Wins: <span id="stats_wins">0</span></div>
</div>''')

h_code = h_code.replace('<button onclick="setDifficulty(\'expert\')">Expert (8x4)</button>', '<button onclick="setDifficulty(\'expert\')">Expert (8x4)</button>\n  <button onclick="setDifficulty(\'campaign\')">Campaign</button>')

h_code = h_code.replace('''  let playing = false;''', '''  let playing = false;
  let stats_pairs = parseInt(localStorage.getItem('kmemory_pairs')) || 0;
  let stats_wins = parseInt(localStorage.getItem('kmemory_wins')) || 0;
  let campaign_level = 1;
  document.getElementById('stats_pairs').textContent = stats_pairs;
  document.getElementById('stats_wins').textContent = stats_wins;''')

h_code = h_code.replace('''      let numCards = diff === 'easy' ? 16 : (diff === 'hard' ? 24 : 32);''', '''      let numCards = 16;
      if (diff === 'easy') numCards = 16;
      else if (diff === 'hard') numCards = 24;
      else if (diff === 'expert') numCards = 32;
      else if (diff === 'campaign') {
          if (campaign_level === 1) numCards = 16;
          else if (campaign_level === 2) numCards = 24;
          else if (campaign_level === 3) numCards = 32;
          else if (campaign_level === 4) numCards = 48;
          else numCards = 64;
      }''')

h_code = h_code.replace('''      if (d === 'easy') document.getElementById('ui').style.width = '380px';
      else if (d === 'hard') document.getElementById('ui').style.width = '570px';
      else document.getElementById('ui').style.width = '760px';''', '''      if (d === 'campaign') campaign_level = 1;
      updateGridSize();
  }
  function updateGridSize() {
      let d = diff;
      let ui = document.getElementById('ui');
      if (d === 'easy' || (d === 'campaign' && campaign_level === 1)) { grid.className = 'grid easy'; ui.style.width = '380px'; }
      else if (d === 'hard' || (d === 'campaign' && campaign_level === 2)) { grid.className = 'grid hard'; ui.style.width = '570px'; }
      else if (d === 'expert' || (d === 'campaign' && campaign_level === 3)) { grid.className = 'grid expert'; ui.style.width = '760px'; }
      else if (d === 'campaign' && campaign_level === 4) { grid.className = 'grid camp4'; ui.style.width = '760px'; }
      else if (d === 'campaign' && campaign_level === 5) { grid.className = 'grid camp5'; ui.style.width = '760px'; }
  }''')

init_old = '''  function init() {
      stopTimer();
      elapsed = 0;
      timeEl.textContent = elapsed;
      cards = [];
      let numCards = diff === 'easy' ? 16 : (diff === 'hard' ? 24 : 32);
      for (let i = 0; i < numCards; i++) {
          cards.push(Math.floor(i / 2));
          flipped[i] = true;
          matched[i] = false;
      }
      for (let i = numCards - 1; i > 0; i--) {
          const j = Math.floor(Math.random() * (i + 1));
          [cards[i], cards[j]] = [cards[j], cards[i]];
      }'''
init_new = '''  function init() {
      updateGridSize();
      stopTimer();
      elapsed = 0;
      timeEl.textContent = elapsed;
      cards = [];
      let numCards = 16;
      if (diff === 'easy') numCards = 16;
      else if (diff === 'hard') numCards = 24;
      else if (diff === 'expert') numCards = 32;
      else if (diff === 'campaign') {
          if (campaign_level === 1) numCards = 16;
          else if (campaign_level === 2) numCards = 24;
          else if (campaign_level === 3) numCards = 32;
          else if (campaign_level === 4) numCards = 48;
          else numCards = 64;
      }
      for (let i = 0; i < numCards; i++) {
          let val = Math.floor(i / 2);
          if ((diff === 'expert' || diff === 'campaign') && numCards >= 24) {
              if (val === (numCards/2 - 1)) val = 100; // Clock
              else if (val === (numCards/2 - 2)) val = 101; // Shuffle
          }
          cards.push(val);
          flipped[i] = true;
          matched[i] = false;
      }
      for (let i = numCards - 1; i > 0; i--) {
          const j = Math.floor(Math.random() * (i + 1));
          [cards[i], cards[j]] = [cards[j], cards[i]];
      }'''
h_code = h_code.replace(init_old, init_new)

h_code = h_code.replace('''          if (matched[i]) {
              c.classList.add('matched');
          } else if (flipped[i]) {
              c.classList.add('flipped');
              c.textContent = themes[currentTheme](cards[i]);
          }''', '''          if (matched[i]) {
              c.classList.add('matched');
          } else if (flipped[i]) {
              c.classList.add('flipped');
              if (cards[i] === 100) { c.textContent = "+T"; c.style.color = "#00f"; }
              else if (cards[i] === 101) { c.textContent = "??"; c.style.color = "#f00"; }
              else c.textContent = themes[currentTheme](cards[i] % 16);
          }''')

match_old = '''                  if (cards[firstFlip] === cards[secondFlip]) {
                      score += 100 * combo;
                      combo++;
                      scoreEl.textContent = score;
                      comboEl.textContent = combo;
                      matched[firstFlip] = true;
                      matched[secondFlip] = true;
                      firstFlip = -1;
                      secondFlip = -1;
                      matches++;
                      playTone(800, 'square', 0.2);
                      if (matches === numCards / 2) {
                          stopTimer();
                          let b = localStorage.getItem(getBestKey());
                          if (!b || score > parseInt(b)) {
                              localStorage.setItem(getBestKey(), score);
                          }
                          setTimeout(() => { alert("You won with score " + score + " in " + moves + " moves and " + elapsed + " seconds!"); updateBest(); init(); }, 100);
                      }
                      render();
                  } else {'''
match_new = '''                  if (cards[firstFlip] === cards[secondFlip]) {
                      if (cards[firstFlip] === 100) {
                          score += 500;
                          elapsed = Math.max(0, elapsed - 10);
                          timeEl.textContent = elapsed;
                      } else if (cards[firstFlip] === 101) {
                          for (let k = numCards - 1; k > 0; k--) {
                              if (!matched[k]) {
                                  let j = Math.floor(Math.random() * (k + 1));
                                  while(matched[j]) j = Math.floor(Math.random() * (k + 1));
                                  if (!matched[j]) {
                                      [cards[k], cards[j]] = [cards[j], cards[k]];
                                  }
                              }
                          }
                      }
                      
                      score += 100 * combo;
                      combo++;
                      scoreEl.textContent = score;
                      comboEl.textContent = combo;
                      matched[firstFlip] = true;
                      matched[secondFlip] = true;
                      firstFlip = -1;
                      secondFlip = -1;
                      matches++;
                      stats_pairs++;
                      localStorage.setItem('kmemory_pairs', stats_pairs);
                      document.getElementById('stats_pairs').textContent = stats_pairs;
                      
                      playTone(800, 'square', 0.2);
                      if (matches === numCards / 2) {
                          stopTimer();
                          stats_wins++;
                          localStorage.setItem('kmemory_wins', stats_wins);
                          document.getElementById('stats_wins').textContent = stats_wins;
                          
                          let b = localStorage.getItem(getBestKey());
                          if (!b || score > parseInt(b)) {
                              localStorage.setItem(getBestKey(), score);
                          }
                          
                          if (diff === 'campaign') {
                              campaign_level++;
                              if (campaign_level > 5) {
                                  setTimeout(() => { alert("You beat the Campaign! Total score: " + score); updateBest(); diff='campaign'; campaign_level=1; init(); }, 100);
                              } else {
                                  setTimeout(() => { alert("Stage Clear! Next Stage..."); updateBest(); init(); }, 100);
                              }
                          } else {
                              setTimeout(() => { alert("You won with score " + score + " in " + moves + " moves and " + elapsed + " seconds!"); updateBest(); init(); }, 100);
                          }
                      }
                      render();
                  } else {'''
h_code = h_code.replace(match_old, match_new)

with open('d:/KiloApps/KiloOS/public/apps/ksolitaire.html', 'w') as f:
    f.write(h_code)
