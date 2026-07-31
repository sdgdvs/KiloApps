import re

html_path = r"d:\KiloApps\KiloOS\public\apps\ktetris.html"

with open(html_path, "r", encoding="utf-8") as f:
    html = f.read()

# 1. Replace Math.random with randomFloat for determinism
prng_code = """
  let rngState = 12345;
  function randomSeed(s) { rngState = s; }
  function randomInt(max) {
      if (max <= 0) return 0;
      rngState = (rngState * 1103515245 + 12345) & 0x7fffffff;
      return rngState % max;
  }
  function randomFloat() { return randomInt(1000000) / 1000000; }
"""
html = html.replace("let grid = Array(H)", prng_code + "\n  let grid = Array(H)")
html = html.replace("Math.random()", "randomFloat()")

# 2. Keybinds, Replay and Stats State
state_vars = """
  let keybinds = JSON.parse(localStorage.getItem('ktetris_keys')) || {
      up: 'ArrowUp', down: 'ArrowDown', left: 'ArrowLeft', right: 'ArrowRight',
      drop: ' ', hold: 'c', pause: 'p', nuke: 'b', swap: 's', freeze: 'f'
  };
  let showKeybinds = false;
  let bindIndex = 0;
  let bindNames = Object.keys(keybinds);

  let currentReplay = { seed: 12345, inputs: [] };
  let savedReplay = JSON.parse(localStorage.getItem('ktetris_replay'));
  let isReplaying = false;
  let replayIndex = 0;
  let replayTick = 0;

  function exportStats() {
      let tr = (lines>0) ? (statLines[3]/lines*100).toFixed(1) : 0;
      let stats = { score, lines, piecesPlaced, time: formatTime(modeTimerMs), tetrisRate: tr, singles: statLines[0], doubles: statLines[1], triples: statLines[2], tetrises: statLines[3] };
      let jStr = JSON.stringify(stats, null, 2);
      let cStr = `Score,Lines,Pieces,Time,TetrisRate,Single,Double,Triple,Tetris\\n${score},${lines},${piecesPlaced},${formatTime(modeTimerMs)},${tr},${statLines[0]},${statLines[1]},${statLines[2]},${statLines[3]}`;
      let a1 = document.createElement('a'); a1.href = URL.createObjectURL(new Blob([jStr])); a1.download = 'ktetris_stats.json'; a1.click();
      let a2 = document.createElement('a'); a2.href = URL.createObjectURL(new Blob([cStr])); a2.download = 'ktetris_stats.csv'; a2.click();
  }

  function exportLeaderboard() {
      let a = document.createElement('a'); a.href = URL.createObjectURL(new Blob([JSON.stringify(leaderboard, null, 2)])); a.download = 'ktetris_hiscores.json'; a.click();
  }
  function importLeaderboard() {
      let input = document.createElement('input'); input.type = 'file'; input.accept = '.json';
      input.onchange = e => {
          let f = e.target.files[0];
          if (f) { let r = new FileReader(); r.onload = e2 => { try { leaderboard = JSON.parse(e2.target.result); localStorage.setItem('ktetris_leaderboard', JSON.stringify(leaderboard)); } catch(err) {} }; r.readAsText(f); }
      }; input.click();
  }
"""
html = html.replace("let statLines = [0, 0, 0, 0];", "let statLines = [0, 0, 0, 0];\n" + state_vars)

# 3. Save initial seed on Init
html = html.replace("gameOver = false;", "gameOver = false;\n      if (!isReplaying) { currentReplay = { seed: Date.now(), inputs: [] }; randomSeed(currentReplay.seed); } else { randomSeed(currentReplay.seed); replayTick = 0; replayIndex = 0; }")

# 4. Input handling
input_start = """
  document.addEventListener('keydown', e => {
      let key = e.key === ' ' ? ' ' : (e.key.length === 1 ? e.key.toLowerCase() : e.key);

      if (showKeybinds) {
          e.preventDefault();
          if (e.key === 'Escape') { showKeybinds = false; startScreen = true; return; }
          keybinds[bindNames[bindIndex]] = key;
          localStorage.setItem('ktetris_keys', JSON.stringify(keybinds));
          bindIndex++;
          if (bindIndex >= bindNames.length) { showKeybinds = false; startScreen = true; }
          return;
      }
"""

html = re.sub(r"document\.addEventListener\('keydown', e => \{.*?(?=if \(showHelp\))", input_start, html, flags=re.DOTALL)

# Replay playback ignore input
html = html.replace("if (['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight', ' '].includes(e.key)) {", "if (isReplaying) return;\n      if (['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight', ' '].includes(e.key)) {")

# Start screen keys
start_screen_repl = """if (startScreen) {
          if (e.key === '1') { gameMode = MODE_MARATHON; startScreen = false; score = 0; isReplaying = false; init(); return; }
          if (e.key === '2') { gameMode = MODE_SPRINT;   startScreen = false; score = 0; isReplaying = false; init(); return; }
          if (e.key === '3') { gameMode = MODE_ULTRA;    startScreen = false; score = 0; isReplaying = false; init(); return; }
          if (e.key === '4') { gameMode = MODE_CAMPAIGN; startScreen = false; campaignLevel = 1; score = 0; isReplaying = false; init(); return; }
          if (e.key === '5' || e.key.toLowerCase() === 'l') { showLeaderboard = true; startScreen = false; return; }
          if (e.key.toLowerCase() === 'v' || e.key.toLowerCase() === 'r') { isReplaying = false; loadGameState(); return; }
          if (e.key.toLowerCase() === 'h') { showHelp = true; return; }
          if (e.key.toLowerCase() === 'k') { showKeybinds = true; bindIndex = 0; startScreen = false; return; }
          if (e.key.toLowerCase() === 'w' && savedReplay) { isReplaying = true; currentReplay = savedReplay; startScreen = false; init(); return; }
          return;
      }"""
html = re.sub(r"if \(startScreen\) \{.*?(?=\n      if \(showLeaderboard\))", start_screen_repl, html, flags=re.DOTALL)

# Leaderboard screen keys (import/export)
lb_screen_repl = """if (showLeaderboard) {
          if (e.key === 'Enter' || e.key === 'Escape' || e.key.toLowerCase() === 'b') { showLeaderboard = false; startScreen = true; }
          if (e.key.toLowerCase() === 'e') { exportLeaderboard(); }
          if (e.key.toLowerCase() === 'i') { importLeaderboard(); }
          return;
      }"""
html = re.sub(r"if \(showLeaderboard\) \{.*?(?=\n      if \(winScreen && e\.key === 'Enter'\))", lb_screen_repl, html, flags=re.DOTALL)

# Win/Game Over screens (Export Stats, Save Replay)
win_gameover_repl = """
      if (winScreen || gameOver) {
          if (e.key === 'Enter') { startScreen = true; winScreen = false; gameOver = false; return; }
          if (e.key.toLowerCase() === 'e') { exportStats(); return; }
          if (e.key.toLowerCase() === 's' && !isReplaying) { localStorage.setItem('ktetris_replay', JSON.stringify(currentReplay)); addPopup(BOARD_W/2-25, H*CELL/2, "REPLAY SAVED!", "#00ff00"); savedReplay = currentReplay; return; }
          return;
      }
"""
html = re.sub(r"if \(winScreen && e\.key === 'Enter'\) \{.*?if \(gameOver \|\| winScreen\) return;", win_gameover_repl.strip(), html, flags=re.DOTALL)


# Gameplay Keys mapping and Replay recording
def replace_gameplay_keys():
    s = """
      if (key === keybinds.pause) { isPaused = !isPaused; return; }
      if (key === 'h') { showHelp = true; return; }
      if (key === 'v') { saveGameState(); return; }
      if (key === keybinds.nuke) { useRowNuke(); if(!isReplaying) currentReplay.inputs.push({t:replayTick, k:'nuke'}); return; }
      if (key === keybinds.swap) { usePieceSwap(); if(!isReplaying) currentReplay.inputs.push({t:replayTick, k:'swap'}); return; }
      if (key === keybinds.freeze) { useGravityFreeze(); if(!isReplaying) currentReplay.inputs.push({t:replayTick, k:'freeze'}); return; }
      if (isPaused) return;

      if (key === keybinds.left) { if (!checkCol(pc, rot, cx - 1, cy)) cx--; if(!isReplaying) currentReplay.inputs.push({t:replayTick, k:'left'}); }
      if (key === keybinds.right) { if (!checkCol(pc, rot, cx + 1, cy)) cx++; if(!isReplaying) currentReplay.inputs.push({t:replayTick, k:'right'}); }
      if (key === keybinds.down) { if (!checkCol(pc, rot, cx, cy + 1)) cy++; if(!isReplaying) currentReplay.inputs.push({t:replayTick, k:'down'}); }
      if (key === keybinds.up || key === 'x' || key === 'z') {
          let nr = (rot + 1) % 4;
          if (!checkCol(pc, nr, cx, cy)) rot = nr;
          else if (!checkCol(pc, nr, cx - 1, cy)) { cx--; rot = nr; }
          else if (!checkCol(pc, nr, cx + 1, cy)) { cx++; rot = nr; }
          else if (!checkCol(pc, nr, cx - 2, cy)) { cx -= 2; rot = nr; }
          else if (!checkCol(pc, nr, cx + 2, cy)) { cx += 2; rot = nr; }
          if(!isReplaying) currentReplay.inputs.push({t:replayTick, k:'up'});
      }
      if (key === keybinds.hold || e.key === 'Shift') {
          if (!holdUsed) {
              if (holdPc === -1) { holdPc = pc; holdIsBomb = isBomb; spawn(); } 
              else { let temp = pc; let tempBomb = isBomb; pc = holdPc; isBomb = holdIsBomb; holdPc = temp; holdIsBomb = tempBomb; rot = 0; cx = 3; cy = -2; }
              holdUsed = true; playSound(700, 'sine', 0.08);
              if(!isReplaying) currentReplay.inputs.push({t:replayTick, k:'hold'});
          }
      }
      if (key === keybinds.drop) {
          let startY = cy; let dropDist = 0;
          while (!checkCol(pc, rot, cx, cy + 1)) { cy++; dropDist++; }
          score += dropDist * 2; spawnDropParticles(cx, startY, cy, isBomb ? 15 : (pc + 1));
          let oldLevel = campaignLevel; lock();
          if (!winScreen && !gameOver && (gameMode !== MODE_CAMPAIGN || campaignLevel === oldLevel)) spawn();
          if(!isReplaying) currentReplay.inputs.push({t:replayTick, k:'drop'});
      }
"""
    return s

html = re.sub(r"if \(e\.key\.toLowerCase\(\) === 'p'\) \{.*?if \(e\.key === ' '\) \{.*?\}\n  \}\);", replace_gameplay_keys().strip() + "\n  });", html, flags=re.DOTALL)


# Replay tick playback inside update()
update_start = """function update(time = performance.now()) {
      const deltaTime = time - lastTime;
      lastTime = time;

      if (!isPaused && !gameOver && !startScreen && !winScreen && !showLeaderboard && !showHelp && !showKeybinds) {
          replayTick += deltaTime;
          if (isReplaying) {
              while (replayIndex < currentReplay.inputs.length && currentReplay.inputs[replayIndex].t <= replayTick) {
                  let k = currentReplay.inputs[replayIndex].k;
                  if (k === 'left' && !checkCol(pc, rot, cx - 1, cy)) cx--;
                  if (k === 'right' && !checkCol(pc, rot, cx + 1, cy)) cx++;
                  if (k === 'down' && !checkCol(pc, rot, cx, cy + 1)) cy++;
                  if (k === 'up') {
                      let nr = (rot + 1) % 4;
                      if (!checkCol(pc, nr, cx, cy)) rot = nr;
                      else if (!checkCol(pc, nr, cx - 1, cy)) { cx--; rot = nr; }
                      else if (!checkCol(pc, nr, cx + 1, cy)) { cx++; rot = nr; }
                  }
                  if (k === 'hold' && !holdUsed) {
                      if (holdPc === -1) { holdPc = pc; holdIsBomb = isBomb; spawn(); } 
                      else { let temp = pc; let tempBomb = isBomb; pc = holdPc; isBomb = holdIsBomb; holdPc = temp; holdIsBomb = tempBomb; rot = 0; cx = 3; cy = -2; }
                      holdUsed = true; playSound(700, 'sine', 0.08);
                  }
                  if (k === 'drop') {
                      let startY = cy; let dropDist = 0;
                      while (!checkCol(pc, rot, cx, cy + 1)) { cy++; dropDist++; }
                      score += dropDist * 2; spawnDropParticles(cx, startY, cy, isBomb ? 15 : (pc + 1));
                      let oldLevel = campaignLevel; lock();
                      if (!winScreen && !gameOver && (gameMode !== MODE_CAMPAIGN || campaignLevel === oldLevel)) spawn();
                  }
                  if (k === 'nuke') useRowNuke();
                  if (k === 'swap') usePieceSwap();
                  if (k === 'freeze') useGravityFreeze();
                  replayIndex++;
              }
          }"""
html = re.sub(r"function update\(time = performance\.now\(\)\) \{.*?(?=modeTimerMs \+= deltaTime;)", update_start + "\n          ", html, flags=re.DOTALL)

# Add showKeybinds and replaying renders
render_add = """} else if (showKeybinds) {
          ctx.fillStyle = 'rgba(10, 11, 16, 0.94)';
          ctx.fillRect(0, 0, 370, 510);
          ctx.fillStyle = '#00ffff';
          ctx.font = 'bold 20px system-ui, sans-serif';
          ctx.textAlign = 'center';
          ctx.fillText('KEYBINDS CONFIG', 185, 50);
          ctx.fillStyle = '#ffffff';
          ctx.font = '14px system-ui, monospace';
          ctx.textAlign = 'left';
          bindNames.forEach((b, i) => {
              ctx.fillStyle = i === bindIndex ? '#00ff66' : '#aaaaaa';
              ctx.fillText(b.toUpperCase() + ": " + (i === bindIndex ? "[PRESS KEY]" : keybinds[b]), 50, 100 + i * 25);
          });
          ctx.fillStyle = '#666677';
          ctx.textAlign = 'center';
          ctx.fillText('Press ESC to cancel', 185, 480);
          ctx.textAlign = 'left';"""

html = html.replace("} else if (showLeaderboard) {", render_add + "\n      } else if (showLeaderboard) {")

# Start screen keys hint addition
start_hint_add = """ctx.fillStyle = '#ffaa00';
          ctx.fillText('[H]. Help & Controls', 70, yOffset);
          ctx.fillStyle = '#00ccff';
          ctx.fillText('[K]. Configure Keybinds', 70, yOffset + 32);
          if (savedReplay) { ctx.fillStyle = '#ff55aa'; ctx.fillText('[W]. Watch Last Replay', 70, yOffset + 64); }
"""
html = re.sub(r"ctx\.fillStyle = '#ffaa00';\s*ctx\.fillText\('\[H\]\. Help & Controls', 70, yOffset\);", start_hint_add, html)
html = html.replace("yOffset + 32", "yOffset + 96") # For Resume Game

# Replay UI tag
html = html.replace("ctx.fillText(modeNames[gameMode], SIDEBAR_X, 22);", "ctx.fillText(modeNames[gameMode] + (isReplaying ? ' [REPLAY]' : ''), SIDEBAR_X, 22);")

# Game Over / Win hints
go_hints = """ctx.fillText('GAME OVER', 35, 185);
          ctx.fillStyle = '#aaaaaa';
          ctx.font = '12px system-ui, monospace';
          ctx.fillText('[ENTER] MENU', 48, 220);
          ctx.fillText('[E] EXPORT STATS', 48, 240);
          ctx.fillText('[S] SAVE REPLAY', 48, 260);"""
html = re.sub(r"ctx\.fillText\('GAME OVER', 35, 185\);.*?(?=\} else if \(winScreen\))", go_hints + "\n      ", html, flags=re.DOTALL)

win_hints = """ctx.fillText('VICTORY!', 50, 175);
          ctx.fillStyle = '#ffffff';
          ctx.font = '12px system-ui, monospace';
          ctx.fillText(gameMode === MODE_SPRINT ? `TIME: ${formatTime(modeTimerMs)}` : 'STAGE CLEARED!', 30, 205);
          ctx.fillText('[ENTER] MENU', 30, 230);
          ctx.fillText('[E] EXPORT STATS | [S] SAVE REPLAY', 30, 250);"""
html = re.sub(r"ctx\.fillText\('VICTORY!', 50, 175\);.*?(?=\} else if \(startScreen\))", win_hints + "\n      ", html, flags=re.DOTALL)

# Leaderboard hint
html = html.replace("Press ENTER or ESC to return", "ENTER: Menu | E: Export | I: Import")


with open(html_path, "w", encoding="utf-8") as f:
    f.write(html)
print("HTML modified successfully.")
