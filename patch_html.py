import os

html_path = r"d:\KiloApps\KiloOS\public\apps\kwords.html"
with open(html_path, "r", encoding="utf-8") as f:
    content = f.read()

# 1. Update theme select
old_theme_select = """<option value="Space">Space</option>
      </select>"""
new_theme_select = """<option value="Space">Space</option>
        <option value="Food">Food</option>
        <option value="Sports">Sports</option>
        <option value="Nature">Nature</option>
        <option value="History">History</option>
      </select>"""
content = content.replace(old_theme_select, new_theme_select)

# 2. Update mode select
old_mode_select = """<option value="time_attack">Time Attack</option>
      </select>"""
new_mode_select = """<option value="time_attack">Time Attack</option>
        <option value="campaign">Campaign</option>
      </select>"""
content = content.replace(old_mode_select, new_mode_select)

# 3. Add wand button & Header details
old_header_btns = """<button class="diff-btn" id="btn-hint" onclick="useHint()" style="background-color: #d69e2e; color: #1a202c; font-weight: bold; margin-left: 10px;">Hint</button>"""
new_header_btns = """<button class="diff-btn" id="btn-hint" onclick="useHint()" style="background-color: #d69e2e; color: #1a202c; font-weight: bold; margin-left: 10px;">Hint</button>
      <button class="diff-btn" id="btn-wand" onclick="useWand()" style="background-color: #9f7aea; color: white; font-weight: bold; margin-left: 10px;">Wand</button>"""
content = content.replace(old_header_btns, new_header_btns)

old_header_stats = """<div>Found: <span id="found-count">0</span>/<span id="total">0</span> | Score: <span id="score">0</span> | Timer: <span id="timer">00:00</span></div>"""
new_header_stats = """<div>Found: <span id="found-count">0</span>/<span id="total">0</span> | Score: <span id="score">0</span> | Timer: <span id="timer">00:00</span> | Wands: <span id="wands">3</span></div>"""
content = content.replace(old_header_stats, new_header_stats)

# 4. Update THEMES JSON
old_themes_json = """"Space": ["ASTEROID", "COMET", "GALAXY", "NEBULA", "PLANET", "STAR", "ORBIT", "SATELLITE", "ROCKET", "GRAVITY", "ECLIPSE", "METEOR", "UNIVERSE", "COSMOS", "PULSAR", "QUASAR", "SUPERNOVA", "VACUUM", "EQUATOR", "HORIZON", "ZENITH", "LUNAR", "SOLAR", "TELESCOPE", "ASTRONAUT", "SPACECRAFT", "OBSERVATORY", "CONSTELLATION", "ZODIAC", "APOLLO"]
    };"""
new_themes_json = """"Space": ["ASTEROID", "COMET", "GALAXY", "NEBULA", "PLANET", "STAR", "ORBIT", "SATELLITE", "ROCKET", "GRAVITY", "ECLIPSE", "METEOR", "UNIVERSE", "COSMOS", "PULSAR", "QUASAR", "SUPERNOVA", "VACUUM", "EQUATOR", "HORIZON", "ZENITH", "LUNAR", "SOLAR", "TELESCOPE", "ASTRONAUT", "SPACECRAFT", "OBSERVATORY", "CONSTELLATION", "ZODIAC", "APOLLO"],
      "Food": ["PIZZA", "BURGER", "SALAD", "PASTA", "SUSHI", "STEAK", "CHEESE", "BREAD", "APPLE", "BANANA", "ORANGE", "GRAPE", "CHICKEN", "BACON", "TOMATO", "POTATO", "ONION", "GARLIC", "PEPPER", "CARROT", "CEREAL", "WAFFLE", "PANCAKE", "MUFFIN", "COOKIE", "CHOCOLATE", "VANILLA", "BUTTER", "YOGURT", "HONEY"],
      "Sports": ["SOCCER", "TENNIS", "BASKETBALL", "BASEBALL", "GOLF", "RUGBY", "CRICKET", "HOCKEY", "VOLLEYBALL", "SWIMMING", "BOXING", "WRESTLING", "CYCLING", "ATHLETICS", "GYMNASTICS", "ARCHERY", "FENCING", "BOWLING", "BILLIARDS", "SNOOKER", "DARTS", "KARATE", "JUDO", "TAEKWONDO", "SURFING", "SKATING", "SKIING", "SNOWBOARD", "ROWING", "SAILING"],
      "Nature": ["FOREST", "RIVER", "MOUNTAIN", "OCEAN", "DESERT", "VALLEY", "CANYON", "VOLCANO", "ISLAND", "JUNGLE", "GLACIER", "TUNDRA", "PRAIRIE", "SAVANNA", "MARSH", "SWAMP", "LAKE", "STREAM", "WATERFALL", "GEYSER", "CAVE", "CLIFF", "BEACH", "DUNE", "REEF", "FLOWER", "TREE", "BUSH", "GRASS", "FERN"],
      "History": ["EMPIRE", "PHARAOH", "PYRAMID", "CASTLE", "KNIGHT", "VIKING", "SAMURAI", "ROMAN", "GREEK", "SPARTAN", "AZTEC", "MAYAN", "INCA", "DYNASTY", "REVOLUTION", "WARRIOR", "GLADIATOR", "CRUSADE", "RENAISSANCE", "COLONY", "TREATY", "ALLIANCE", "MONARCH", "REPUBLIC", "SENATE", "CHIEFTAIN", "EMPEROR", "SULTAN", "TSAR", "KAISER"]
    };"""
content = content.replace(old_themes_json, new_themes_json)

# 5. Add state variables
old_state_vars = """    let timerInterval = null;
    let seconds = 0;
    let score = 0;"""
new_state_vars = """    let timerInterval = null;
    let seconds = 0;
    let score = 0;
    let campaignStage = 1;
    let magicWands = 3;
    let timeSinceLastFind = 0;
    let comboMultiplier = 1;"""
content = content.replace(old_state_vars, new_state_vars)

# 6. Mode select hook
old_setMode = """    function setMode(mode) {
        gameMode = mode;
        initGame();
    }"""
new_setMode = """    function setMode(mode) {
        gameMode = mode;
        if (gameMode === 'campaign') {
            campaignStage = 1;
            magicWands = 3;
            score = 0;
        }
        initGame();
    }"""
content = content.replace(old_setMode, new_setMode)

# 7. initGame
old_initGame_timers = """        if (gameMode === "time_attack") {
            seconds = (gridSize === 10) ? 120 : (gridSize === 15) ? 180 : 300;
        } else {
            seconds = 0;
        }"""
new_initGame_timers = """        if (gameMode === "campaign") {
            if (campaignStage === 1) { gridSize = 10; numWordsToFind = 5; seconds = 120; }
            else if (campaignStage === 2) { gridSize = 12; numWordsToFind = 6; seconds = 130; }
            else if (campaignStage === 3) { gridSize = 12; numWordsToFind = 7; seconds = 140; }
            else if (campaignStage === 4) { gridSize = 15; numWordsToFind = 8; seconds = 150; }
            else if (campaignStage === 5) { gridSize = 15; numWordsToFind = 9; seconds = 160; }
            else if (campaignStage === 6) { gridSize = 15; numWordsToFind = 10; seconds = 170; }
            else if (campaignStage === 7) { gridSize = 18; numWordsToFind = 11; seconds = 180; }
            else if (campaignStage === 8) { gridSize = 20; numWordsToFind = 12; seconds = 190; }
            else if (campaignStage === 9) { gridSize = 22; numWordsToFind = 14; seconds = 200; }
            else { gridSize = 25; numWordsToFind = 16; seconds = 240; }
            document.getElementById('wands').parentElement.style.display = 'inline';
        } else if (gameMode === "time_attack") {
            seconds = (gridSize === 10) ? 120 : (gridSize === 15) ? 180 : 300;
            document.getElementById('wands').parentElement.style.display = 'inline';
        } else {
            seconds = 0;
            document.getElementById('wands').parentElement.style.display = 'inline';
        }
        timeSinceLastFind = 0;
        comboMultiplier = 1;"""
content = content.replace(old_initGame_timers, new_initGame_timers)


old_setInterval = """        timerInterval = setInterval(() => {
            if (gameMode === "time_attack") {
                if (seconds > 0) {
                    seconds--;
                    updateTimerDisplay();
                } else {
                    clearInterval(timerInterval);
                    document.getElementById('final-score').textContent = "Time's up!";
                    document.getElementById('win-title').textContent = "Game Over";
                    document.getElementById('win-message').style.display = 'block';
                }
            } else if (gameMode === "classic") {
                seconds++;
                updateTimerDisplay();
            }
        }, 1000);"""
new_setInterval = """        timerInterval = setInterval(() => {
            if (gameMode === "time_attack" || gameMode === "campaign") {
                if (seconds > 0) {
                    seconds--;
                    updateTimerDisplay();
                } else {
                    clearInterval(timerInterval);
                    document.getElementById('final-score').textContent = "Time's up!";
                    document.getElementById('win-title').textContent = "Game Over";
                    document.getElementById('win-message').style.display = 'block';
                }
            } else if (gameMode === "classic") {
                seconds++;
                updateTimerDisplay();
            }
            timeSinceLastFind++;
        }, 1000);"""
content = content.replace(old_setInterval, new_setInterval)

# 8. updateTimerDisplay
old_updateTimerDisplay = """    function updateTimerDisplay() {
        if (gameMode === "zen") {
            document.getElementById('timer').textContent = "--:--";
            document.getElementById('score').textContent = "-";
        } else {
            let m = Math.floor(seconds / 60).toString().padStart(2, '0');
            let s = (seconds % 60).toString().padStart(2, '0');
            document.getElementById('timer').textContent = `${m}:${s}`;
            document.getElementById('score').textContent = score;
        }
    }"""
new_updateTimerDisplay = """    function updateTimerDisplay() {
        if (gameMode === "zen") {
            document.getElementById('timer').textContent = "--:--";
            document.getElementById('score').textContent = "-";
        } else {
            let m = Math.floor(seconds / 60).toString().padStart(2, '0');
            let s = (seconds % 60).toString().padStart(2, '0');
            document.getElementById('timer').textContent = `${m}:${s}`;
            document.getElementById('score').textContent = score;
        }
        document.getElementById('wands').textContent = magicWands;
    }"""
content = content.replace(old_updateTimerDisplay, new_updateTimerDisplay)

# 9. endSelection score and win
old_endSel_score = """                if (gameMode !== "zen") {
                    let points = 1000 - (seconds * 2);
                    if (gameMode === "time_attack") points = 500; 
                    if (points < 100) points = 100;
                    score += points;
                }"""
new_endSel_score = """                if (gameMode !== "zen") {
                    let points = 1000 - (seconds * 2);
                    if (gameMode === "time_attack" || gameMode === "campaign") points = 500; 
                    if (points < 100) points = 100;
                    if (timeSinceLastFind < 15) comboMultiplier++;
                    else comboMultiplier = 1;
                    score += points * comboMultiplier;
                    timeSinceLastFind = 0;
                }"""
content = content.replace(old_endSel_score, new_endSel_score)


old_endSel_win = """                if (foundWords.length === wordsToFind.length) {
                    playSound('fanfare');
                    clearInterval(timerInterval);
                    updateStatsOnWin();
                    document.getElementById('final-score').textContent = score;
                    document.getElementById('win-message').style.display = 'block';
                } else {
                    playSound('chime');
                }"""
new_endSel_win = """                if (foundWords.length === wordsToFind.length) {
                    if (gameMode === 'campaign' && campaignStage < 10) {
                        playSound('fanfare');
                        clearInterval(timerInterval);
                        campaignStage++;
                        setTimeout(() => initGame(), 1500);
                    } else {
                        playSound('fanfare');
                        clearInterval(timerInterval);
                        updateStatsOnWin();
                        document.getElementById('final-score').textContent = score;
                        document.getElementById('win-message').style.display = 'block';
                    }
                } else {
                    playSound('chime');
                }"""
content = content.replace(old_endSel_win, new_endSel_win)

# 10. useWand
wand_fn = """    function useWand() {
        if (foundWords.length === wordsToFind.length || document.getElementById('win-message').style.display === 'block') return;
        if (gameMode === 'campaign' && magicWands <= 0) return;
        if (gameMode !== 'campaign' && score < 200) return;
        
        let unfound = wordsToFind.filter(w => !foundWords.includes(w));
        if (unfound.length === 0) return;
        
        let targetWord = unfound[Math.floor(Math.random() * unfound.length)];
        let foundLoc = findWordInGrid(targetWord);
        
        if (foundLoc) {
            if (gameMode === 'campaign') magicWands--;
            else score = Math.max(0, score - 200);
            
            updateTimerDisplay();
            
            let r = foundLoc.r;
            let c = foundLoc.c;
            const dirs = [[0,1], [1,0], [1,1], [-1,1], [1,-1], [-1,-1], [0,-1], [-1,0]];
            for (let d of dirs) {
                let match = true;
                for (let i = 0; i < targetWord.length; i++) {
                    let nr = r + i * d[0];
                    let nc = c + i * d[1];
                    if (nr < 0 || nr >= gridSize || nc < 0 || nc >= gridSize || grid[nr][nc] !== targetWord[i]) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    foundWords.push(targetWord);
                    document.getElementById('word-' + targetWord).className = 'found';
                    document.getElementById('found-count').textContent = foundWords.length;
                    
                    for (let i = 0; i < targetWord.length; i++) {
                        let nr = r + i * d[0];
                        let nc = c + i * d[1];
                        const el = document.querySelector(`.cell[data-r="${nr}"][data-c="${nc}"]`);
                        if(el) el.classList.add('found');
                    }
                    break;
                }
            }
            
            if (foundWords.length === wordsToFind.length) {
                if (gameMode === 'campaign' && campaignStage < 10) {
                    playSound('fanfare');
                    clearInterval(timerInterval);
                    campaignStage++;
                    setTimeout(() => initGame(), 1500);
                } else {
                    playSound('fanfare');
                    clearInterval(timerInterval);
                    updateStatsOnWin();
                    document.getElementById('final-score').textContent = score;
                    document.getElementById('win-message').style.display = 'block';
                }
            } else {
                playSound('chime');
            }
        }
    }
"""

content = content.replace("function saveGame() {", wand_fn + "\n    function saveGame() {")

# 11. Add mode to help
old_help_modes = """<li style="width: auto;"><strong>Classic:</strong> Find words as fast as possible for a high score. Timer counts up.</li>
        <li style="width: auto;"><strong>Time Attack:</strong> Find all words before the timer runs out!</li>"""
new_help_modes = """<li style="width: auto;"><strong>Classic:</strong> Find words as fast as possible for a high score. Timer counts up.</li>
        <li style="width: auto;"><strong>Campaign:</strong> 10 stages of increasing difficulty and size. Play to the end!</li>
        <li style="width: auto;"><strong>Time Attack:</strong> Find all words before the timer runs out!</li>"""
content = content.replace(old_help_modes, new_help_modes)

old_help_hint = """<p><em>Use the <strong>Hint</strong> button to reveal the first letter of an unfound word (costs 50 points and 30 seconds).</em></p>"""
new_help_hint = """<p><em>Use the <strong>Hint</strong> button to reveal the first letter of an unfound word (costs 50 points and 30 seconds).</em></p>
      <p><em>Use the <strong>Wand</strong> button to reveal an entire word (costs 200 points, or uses 1 Wand in Campaign mode).</em></p>"""
content = content.replace(old_help_hint, new_help_hint)

# 12. Fix bug where start game didn't have cell class in campaign
# Already handled in `renderGrid`.

# Save the file
with open(html_path, "w", encoding="utf-8") as f:
    f.write(content)

print("Patch applied to HTML version.")
