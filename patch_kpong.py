import sys

# --- PATCH C ---
with open(r'd:\KiloApps\KPong\main.c', 'r') as f:
    c = f.read()

c = c.replace('#include <stdio.h>', '#include <stdio.h>\n#include <stdlib.h>')

c = c.replace('int pad_h = 50;', '''int p1_pad_h = 50;
int p2_pad_h = 50;
int campaign_mode = 0;
int campaign_level = 1;
int win_screen = 0;
int last_hitter = 0;
int powerup_x = -1;
int powerup_y = -1;
int powerup_active = 0;
int p1_buff_timer = 0;
int p2_buff_timer = 0;
int high_rally = 0;
''')

c = c.replace('void ResetBall() {', '''void LoadHighRally() {
    FILE* f = fopen("kpong_stats.dat", "rb");
    if(f) { fread(&high_rally, sizeof(int), 1, f); fclose(f); }
}
void SaveHighRally() {
    FILE* f = fopen("kpong_stats.dat", "wb");
    if(f) { fwrite(&high_rally, sizeof(int), 1, f); fclose(f); }
}

void ResetBall() {''')

c = c.replace('if (GetAsyncKeyState(\'3\') & 0x8000) difficulty = 3;', '''if (GetAsyncKeyState('3') & 0x8000) difficulty = 3;
            if (GetAsyncKeyState('C') & 0x8000 && !campaign_mode) { campaign_mode = 1; campaign_level = 1; p1_score=0; p2_score=0; rally=0; ResetBall(); }''')

c = c.replace('''            // Dynamic paddle size
            pad_h = 50 - (rally * 2);
            if (pad_h < 20) pad_h = 20;''', '''            // Dynamic paddle size
            if(p1_buff_timer > 0) p1_buff_timer--;
            if(p2_buff_timer > 0) p2_buff_timer--;
            
            p1_pad_h = 50 - (rally * 2); if(p1_pad_h < 20) p1_pad_h = 20;
            p2_pad_h = 50 - (rally * 2); if(p2_pad_h < 20) p2_pad_h = 20;
            
            if(campaign_mode && campaign_level >= 3) { p2_pad_h -= 10; if(p2_pad_h < 15) p2_pad_h = 15; }
            if(p1_buff_timer > 0) p1_pad_h += 30;
            if(p2_buff_timer > 0) p2_pad_h += 30;''')

c = c.replace('''if (p1_y > H - pad_h) p1_y = H - pad_h;''', '''if (p1_y > H - p1_pad_h) p1_y = H - p1_pad_h;''')
c = c.replace('''if (p2_y > H - pad_h) p2_y = H - pad_h;''', '''if (p2_y > H - p2_pad_h) p2_y = H - p2_pad_h;''')

c = c.replace('''            int ai_spd = ai_speeds[difficulty];
            if (ball_y > p2_y + pad_h / 2 + 10) p2_y += ai_spd;
            if (ball_y < p2_y + pad_h / 2 - 10) p2_y -= ai_spd;''', '''            int ai_spd = ai_speeds[difficulty];
            if(campaign_mode) {
                if(campaign_level == 1) ai_spd = 2;
                else if(campaign_level == 2) ai_spd = 4;
                else if(campaign_level == 3) ai_spd = 6;
                else if(campaign_level == 4) ai_spd = 8;
                else ai_spd = 10;
            }
            if (ball_y > p2_y + p2_pad_h / 2 + 10) p2_y += ai_spd;
            if (ball_y < p2_y + p2_pad_h / 2 - 10) p2_y -= ai_spd;''')

c = c.replace('''// Ball logic
            ball_x += ball_dx;
            ball_y += ball_dy;''', '''// Powerups
            if(powerup_active > 0) powerup_active--;
            if(powerup_x == -1 && rand() % 200 == 0) {
                powerup_x = W/4 + rand()%(W/2); powerup_y = H/4 + rand()%(H/2); powerup_active = 250;
            }
            if(powerup_active == 0) { powerup_x = -1; powerup_y = -1; }
            
            // Ball logic
            if(campaign_mode && campaign_level == 5) { ball_x += (ball_dx > 0 ? ball_dx + 2 : ball_dx - 2); ball_y += (ball_dy > 0 ? ball_dy + 1 : ball_dy - 1); }
            else { ball_x += ball_dx; ball_y += ball_dy; }
            
            if(powerup_x != -1 && ball_x < powerup_x+15 && ball_x+BALL_SIZE > powerup_x && ball_y < powerup_y+15 && ball_y+BALL_SIZE > powerup_y) {
                if(last_hitter == 1) p1_buff_timer = 200;
                else if(last_hitter == 2) p2_buff_timer = 200;
                powerup_x = -1; powerup_y = -1;
                MessageBeep(MB_ICONASTERISK);
            }
''')

c = c.replace('''if (ball_x < 20 + PAD_W && ball_y + BALL_SIZE > p1_y && ball_y < p1_y + pad_h) {
                ball_x = 20 + PAD_W;
                ball_dx = -ball_dx;
                rally++;
                float hit_pos = (float)((ball_y + BALL_SIZE/2.0f) - (p1_y + pad_h/2.0f)) / (pad_h/2.0f);''', '''if (ball_x < 20 + PAD_W && ball_y + BALL_SIZE > p1_y && ball_y < p1_y + p1_pad_h) {
                ball_x = 20 + PAD_W;
                ball_dx = -ball_dx;
                last_hitter = 1;
                rally++;
                if(rally > high_rally) { high_rally = rally; SaveHighRally(); }
                float hit_pos = (float)((ball_y + BALL_SIZE/2.0f) - (p1_y + p1_pad_h/2.0f)) / (p1_pad_h/2.0f);''')

c = c.replace('''if (ball_x + BALL_SIZE > W - 20 - PAD_W && ball_y + BALL_SIZE > p2_y && ball_y < p2_y + pad_h) {
                ball_x = W - 20 - PAD_W - BALL_SIZE;
                ball_dx = -ball_dx;
                rally++;
                float hit_pos = (float)((ball_y + BALL_SIZE/2.0f) - (p2_y + pad_h/2.0f)) / (pad_h/2.0f);''', '''if (ball_x + BALL_SIZE > W - 20 - PAD_W && ball_y + BALL_SIZE > p2_y && ball_y < p2_y + p2_pad_h) {
                ball_x = W - 20 - PAD_W - BALL_SIZE;
                ball_dx = -ball_dx;
                last_hitter = 2;
                rally++;
                if(rally > high_rally) { high_rally = rally; SaveHighRally(); }
                float hit_pos = (float)((ball_y + BALL_SIZE/2.0f) - (p2_y + p2_pad_h/2.0f)) / (p2_pad_h/2.0f);''')

c = c.replace('''if (p1_score >= 11 || p2_score >= 11) {
                game_over = 1;
            }''', '''int target_score = campaign_mode ? 5 : 11;
            if (p1_score >= target_score) {
                if(campaign_mode) {
                    campaign_level++; p1_score = 0; p2_score = 0; rally = 0;
                    if(campaign_level > 5) { win_screen = 1; campaign_mode = 0; game_over = 1; }
                    ResetBall();
                } else {
                    game_over = 1;
                }
            }
            if (p2_score >= target_score) {
                game_over = 1;
                campaign_mode = 0;
            }''')

c = c.replace('''RECT r1 = { 20, p1_y, 20 + PAD_W, p1_y + pad_h };
            RECT r2 = { W - 20 - PAD_W, p2_y, W - 20, p2_y + pad_h };''', '''RECT r1 = { 20, p1_y, 20 + PAD_W, p1_y + p1_pad_h };
            RECT r2 = { W - 20 - PAD_W, p2_y, W - 20, p2_y + p2_pad_h };''')

c = c.replace('''// Draw ball
            RECT rBall = { ball_x, ball_y, ball_x + BALL_SIZE, ball_y + BALL_SIZE };
            FillRect(memDC, &rBall, ball_brush);''', '''// Draw ball
            RECT rBall = { ball_x, ball_y, ball_x + BALL_SIZE, ball_y + BALL_SIZE };
            FillRect(memDC, &rBall, ball_brush);
            if(powerup_x != -1) {
                HBRUSH pu_brush = CreateSolidBrush(RGB(255, 255, 0));
                RECT rPu = { powerup_x, powerup_y, powerup_x+15, powerup_y+15 };
                FillRect(memDC, &rPu, pu_brush);
                DeleteObject(pu_brush);
            }''')

c = c.replace('''char rallyStr[32];
            wsprintfA(rallyStr, "RALLY: %d", rally);
            SetTextColor(memDC, RGB(180, 180, 180));
            TextOutA(memDC, W / 2 - 35, 30, rallyStr, lstrlenA(rallyStr));''', '''char rallyStr[64];
            wsprintfA(rallyStr, "RALLY: %d  HIGH: %d", rally, high_rally);
            SetTextColor(memDC, RGB(180, 180, 180));
            TextOutA(memDC, W / 2 - 50, 30, rallyStr, lstrlenA(rallyStr));''')

c = c.replace('''char diffStr[32];
            char* dName = difficulty == 1 ? "EASY" : (difficulty == 2 ? "NORMAL" : "HARD");
            wsprintfA(diffStr, "DIFF: %s (1-3)", dName);
            TextOutA(memDC, W / 2 - 50, 50, diffStr, lstrlenA(diffStr));''', '''char diffStr[32];
            if(campaign_mode) {
                wsprintfA(diffStr, "CAMPAIGN: LVL %d", campaign_level);
            } else {
                char* dName = difficulty == 1 ? "EASY" : (difficulty == 2 ? "NORMAL" : "HARD");
                wsprintfA(diffStr, "DIFF: %s (1-3)", dName);
            }
            TextOutA(memDC, W / 2 - 50, 50, diffStr, lstrlenA(diffStr));
            if(!campaign_mode) {
                char* cInfo = "Press 'C' for Campaign";
                TextOutA(memDC, W / 2 - 65, 70, cInfo, lstrlenA(cInfo));
            }''')

c = c.replace('''if (game_over) {
                char* winStr = p1_score >= 11 ? "P1 WINS!" : "P2 WINS!";
                TextOutA(memDC, W / 2 - 30, H / 2 - 20, winStr, lstrlenA(winStr));
                char* restartStr = "Press 'R' to Restart";
                TextOutA(memDC, W / 2 - 60, H / 2 + 10, restartStr, lstrlenA(restartStr));
            }''', '''if (game_over) {
                if(win_screen) {
                    SetTextColor(memDC, RGB(255, 215, 0));
                    char* winStr = "CAMPAIGN COMPLETE!";
                    TextOutA(memDC, W / 2 - 65, H / 2 - 20, winStr, lstrlenA(winStr));
                } else {
                    char* winStr = (p1_score > p2_score) ? "P1 WINS!" : "P2 WINS!";
                    TextOutA(memDC, W / 2 - 30, H / 2 - 20, winStr, lstrlenA(winStr));
                }
                SetTextColor(memDC, RGB(255, 255, 255));
                char* restartStr = "Press 'R' to Restart";
                TextOutA(memDC, W / 2 - 60, H / 2 + 10, restartStr, lstrlenA(restartStr));
            }''')

c = c.replace('wc.lpfnWndProc = WndProc;', 'LoadHighRally();\n    wc.lpfnWndProc = WndProc;')

with open(r'd:\KiloApps\KPong\main.c', 'w') as f:
    f.write(c)

# --- PATCH HTML ---
with open(r'd:\KiloApps\KiloOS\public\apps\kpong.html', 'r') as f:
    h = f.read()

h = h.replace('let padH = 50;', '''let p1PadH = 50;
  let p2PadH = 50;
  let campaignMode = false;
  let campaignLevel = 1;
  let lastHitter = 0;
  let powerupX = -1;
  let powerupY = -1;
  let powerupActive = 0;
  let p1BuffTimer = 0;
  let p2BuffTimer = 0;
  let winScreen = false;
  let highRally = localStorage.getItem('kpong_high_rally') || 0;''')

h = h.replace('''      if (gameOver) {
          if (keys['r'] || keys['R']) {
              s1 = 0; s2 = 0; rally = 0;''', '''      if (gameOver) {
          if (keys['r'] || keys['R']) {
              s1 = 0; s2 = 0; rally = 0; winScreen = false;''')

h = h.replace('''if (keys['1']) { difficulty = 1; }
      if (keys['2']) { difficulty = 2; }
      if (keys['3']) { difficulty = 3; }''', '''if (keys['1']) { difficulty = 1; }
      if (keys['2']) { difficulty = 2; }
      if (keys['3']) { difficulty = 3; }
      if ((keys['c'] || keys['C']) && !campaignMode) {
          campaignMode = true; campaignLevel = 1; s1=0; s2=0; rally=0; reset();
      }''')

h = h.replace('''padH = 50 - (rally * 2);
      if (padH < 20) padH = 20;''', '''if(p1BuffTimer > 0) p1BuffTimer--;
      if(p2BuffTimer > 0) p2BuffTimer--;
      
      p1PadH = 50 - (rally * 2); if(p1PadH < 20) p1PadH = 20;
      p2PadH = 50 - (rally * 2); if(p2PadH < 20) p2PadH = 20;
      
      if(campaignMode && campaignLevel >= 3) { p2PadH -= 10; if(p2PadH < 15) p2PadH = 15; }
      if(p1BuffTimer > 0) p1PadH += 30;
      if(p2BuffTimer > 0) p2PadH += 30;''')

h = h.replace('p1y > H - padH', 'p1y > H - p1PadH').replace('p1y = H - padH', 'p1y = H - p1PadH')
h = h.replace('p2y > H - padH', 'p2y > H - p2PadH').replace('p2y = H - padH', 'p2y = H - p2PadH')

h = h.replace('''let aiSpeed = aiSpeeds[difficulty];
      if (by > p2y + padH/2 + 10) p2y += aiSpeed;
      if (by < p2y + padH/2 - 10) p2y -= aiSpeed;''', '''let aiSpeed = aiSpeeds[difficulty];
      if(campaignMode) {
          if(campaignLevel===1) aiSpeed = 2;
          else if(campaignLevel===2) aiSpeed = 4;
          else if(campaignLevel===3) aiSpeed = 6;
          else if(campaignLevel===4) aiSpeed = 8;
          else aiSpeed = 10;
      }
      if (by > p2y + p2PadH/2 + 10) p2y += aiSpeed;
      if (by < p2y + p2PadH/2 - 10) p2y -= aiSpeed;''')

h = h.replace('bx += bdx; by += bdy;', '''
      if(powerupActive > 0) powerupActive--;
      if(powerupX === -1 && Math.random() < 0.005) {
          powerupX = W/4 + Math.random()*(W/2);
          powerupY = H/4 + Math.random()*(H/2);
          powerupActive = 250;
      }
      if(powerupActive <= 0) { powerupX = -1; powerupY = -1; }
      
      if(campaignMode && campaignLevel === 5) {
          bx += (bdx > 0 ? bdx + 2 : bdx - 2);
          by += (bdy > 0 ? bdy + 1 : bdy - 1);
      } else {
          bx += bdx; by += bdy;
      }
      
      if(powerupX !== -1 && bx < powerupX+15 && bx+BALL_SIZE > powerupX && by < powerupY+15 && by+BALL_SIZE > powerupY) {
          if(lastHitter === 1) p1BuffTimer = 200;
          else if(lastHitter === 2) p2BuffTimer = 200;
          powerupX = -1; powerupY = -1;
          playSound(1000, 'score');
      }''')

h = h.replace('''if (bx < 20 + PAD_W && by + BALL_SIZE > p1y && by < p1y + padH) {
          bx = 20 + PAD_W; bdx = -bdx;
          let hitPos = ((by + BALL_SIZE/2) - (p1y + padH/2)) / (padH/2);''', '''if (bx < 20 + PAD_W && by + BALL_SIZE > p1y && by < p1y + p1PadH) {
          bx = 20 + PAD_W; bdx = -bdx;
          lastHitter = 1;
          let hitPos = ((by + BALL_SIZE/2) - (p1y + p1PadH/2)) / (p1PadH/2);''')

h = h.replace('''if (bx + BALL_SIZE > W - 20 - PAD_W && by + BALL_SIZE > p2y && by < p2y + padH) {
          bx = W - 20 - PAD_W - BALL_SIZE; bdx = -bdx;
          let hitPos = ((by + BALL_SIZE/2) - (p2y + padH/2)) / (padH/2);''', '''if (bx + BALL_SIZE > W - 20 - PAD_W && by + BALL_SIZE > p2y && by < p2y + p2PadH) {
          bx = W - 20 - PAD_W - BALL_SIZE; bdx = -bdx;
          lastHitter = 2;
          let hitPos = ((by + BALL_SIZE/2) - (p2y + p2PadH/2)) / (p2PadH/2);''')

h = h.replace('rally++;', 'rally++; if(rally > highRally) { highRally = rally; localStorage.setItem("kpong_high_rally", highRally); }')

h = h.replace('''if (bx < 0) { s2++; rally = 0; playSound(200, 'score'); reset(); document.getElementById('s2').innerText = s2; if (s2 >= 11) gameOver = true; }
      if (bx > W) { s1++; rally = 0; playSound(200, 'score'); reset(); document.getElementById('s1').innerText = s1; if (s1 >= 11) gameOver = true; }''', '''
      let tScore = campaignMode ? 5 : 11;
      if (bx < 0) { 
          s2++; rally = 0; playSound(200, 'score'); reset(); document.getElementById('s2').innerText = s2; 
          if(s2 >= tScore) { gameOver = true; campaignMode = false; }
      }
      if (bx > W) { 
          s1++; rally = 0; playSound(200, 'score'); reset(); document.getElementById('s1').innerText = s1; 
          if(s1 >= tScore) {
              if(campaignMode) {
                  campaignLevel++; s1=0; s2=0; rally=0; document.getElementById('s1').innerText = s1; document.getElementById('s2').innerText = s2;
                  if(campaignLevel > 5) { winScreen = true; campaignMode = false; gameOver = true; }
              } else { gameOver = true; }
          }
      }''')

h = h.replace('ctx.fillRect(20, p1y, PAD_W, padH);', 'ctx.fillRect(20, p1y, PAD_W, p1PadH);')
h = h.replace('ctx.fillRect(W - 20 - PAD_W, p2y, PAD_W, padH);', 'ctx.fillRect(W - 20 - PAD_W, p2y, PAD_W, p2PadH);')

h = h.replace('''// Draw ball''', '''// Draw powerup
      if (powerupX !== -1) {
          ctx.shadowBlur = 10;
          ctx.shadowColor = 'yellow';
          ctx.fillStyle = 'yellow';
          ctx.fillRect(powerupX, powerupY, 15, 15);
      }
      // Draw ball''')

h = h.replace('ctx.fillText("RALLY: " + rally, W/2, 20);', 'ctx.fillText("RALLY: " + rally + "  HIGH: " + highRally, W/2, 20);')

h = h.replace('''let diffStr = difficulty === 1 ? "EASY" : (difficulty === 2 ? "NORMAL" : "HARD");
      ctx.fillText("DIFF: " + diffStr + " (1-3)", W/2, 40);''', '''if(campaignMode) {
          ctx.fillText("CAMPAIGN: LVL " + campaignLevel, W/2, 40);
      } else {
          let diffStr = difficulty === 1 ? "EASY" : (difficulty === 2 ? "NORMAL" : "HARD");
          ctx.fillText("DIFF: " + diffStr + " (1-3)", W/2, 40);
          ctx.fillText("Press 'C' for Campaign", W/2, 60);
      }''')

h = h.replace('''ctx.fillText(s1 >= 11 ? "P1 WINS!" : "P2 WINS!", W/2, H/2 - 10);
          ctx.font = '20px Inter';
          ctx.fillText("Press 'R' to Restart", W/2, H/2 + 30);''', '''if(winScreen) {
              ctx.fillStyle = 'gold';
              ctx.fillText("CAMPAIGN COMPLETE!", W/2, H/2 - 10);
          } else {
              ctx.fillText(s1 > s2 ? "P1 WINS!" : "P2 WINS!", W/2, H/2 - 10);
          }
          ctx.fillStyle = 'white';
          ctx.font = '20px Inter';
          ctx.fillText("Press 'R' to Restart", W/2, H/2 + 30);''')

with open(r'd:\KiloApps\KiloOS\public\apps\kpong.html', 'w') as f:
    f.write(h)
