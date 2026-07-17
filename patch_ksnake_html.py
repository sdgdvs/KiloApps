import sys

with open(r'd:\KiloApps\KiloOS\public\apps\ksnake.html', 'r', encoding='utf-8') as f:
    code = f.read()

code = code.replace('let specialFood = null;', 'let specialFood = null;\n        let ghostFood = null;\n        let ghostFoodTimer = 0;\n        let ghostActiveTimer = 0;\n        let campaignMode = false;\n        let campaignLevel = 1;\n        let applesEaten = 0;')
code = code.replace('<p style="margin:5px 0">3 - Hard</p>', '<p style="margin:5px 0">3 - Hard</p>\n        <p style="margin:5px 0">4 - Campaign</p>')

# init changes
code = code.replace('score = 0;', 'score = 0;\n            applesEaten = 0;\n            ghostFood = null;\n            ghostFoodTimer = 0;\n            ghostActiveTimer = 0;')

code = code.replace('if (difficulty === 0) { currentSpeed = 200; scoreMult = 5; }', 'if (campaignMode) { difficulty = 1; }\n            if (difficulty === 0) { currentSpeed = 200; scoreMult = 5; }')

# obstacles setup
obs_code = '''let numObs = difficulty * 10;
            if (campaignMode) {
                numObs = campaignLevel * 5;
                if (numObs > 45) numObs = 45;
            }
            for(let i=0; i<numObs; i++) {'''
code = code.replace('''let numObs = difficulty * 10;
            for(let i=0; i<numObs; i++) {''', obs_code)

# food place
ghost_food_code = '''function placeGhostFood() {
            let ok = false;
            while(!ok) {
                ghostFood = {
                    x: Math.floor(Math.random() * WIDTH),
                    y: Math.floor(Math.random() * HEIGHT)
                };
                ok = true;
                if (obstacles.some(o => o.x === ghostFood.x && o.y === ghostFood.y)) ok = false;
                if (snake.some(s => s.x === ghostFood.x && s.y === ghostFood.y)) ok = false;
                if (ghostFood.x === food.x && ghostFood.y === food.y) ok = false;
                if (specialFood && ghostFood.x === specialFood.x && ghostFood.y === specialFood.y) ok = false;
            }
            ghostFoodTimer = 40;
        }

        function update() {'''
code = code.replace('function update() {', ghost_food_code)

# collision handling
code = code.replace('''if (snake.some(s => s.x === head.x && s.y === head.y) || obstacles.some(o => o.x === head.x && o.y === head.y)) {
                    playSound('die');
                    gameState = 2;
                    msgBox.style.display = 'block';
                    return;
                }''', '''if (ghostActiveTimer === 0 && (snake.some(s => s.x === head.x && s.y === head.y) || obstacles.some(o => o.x === head.x && o.y === head.y))) {
                    playSound('die');
                    gameState = 2;
                    msgBox.style.display = 'block';
                    return;
                }''')
code = code.replace('''|| snake.some(s => s.x === head.x && s.y === head.y) || obstacles.some(o => o.x === head.x && o.y === head.y)) {''', '''|| (ghostActiveTimer === 0 && (snake.some(s => s.x === head.x && s.y === head.y) || obstacles.some(o => o.x === head.x && o.y === head.y)))) {''')

# eat food
eat_code = '''if (!specialFood && Math.random() < 0.2) {
                    placeSpecialFood();
                }
                if (!ghostFood && Math.random() < 0.15) {
                    placeGhostFood();
                }
                
                applesEaten++;
                if (campaignMode && applesEaten >= 10) {
                    campaignLevel++;
                    let newSpd = 150 - campaignLevel * 10;
                    if(newSpd < 40) newSpd = 40;
                    currentSpeed = newSpd;
                    init();
                }'''
code = code.replace('''if (!specialFood && Math.random() < 0.2) {
                    placeSpecialFood();
                }''', eat_code)

# ghost logic updates
ghost_logic = '''if (ghostActiveTimer > 0) ghostActiveTimer--;
            if (ghostFoodTimer > 0) {
                ghostFoodTimer--;
                if (ghostFoodTimer === 0) ghostFood = null;
            }
            if (ghostFood && head.x === ghostFood.x && head.y === ghostFood.y) {
                playSound('eat');
                score += scoreMult * 2;
                if (score > highScore) {
                    highScore = score;
                    localStorage.setItem('ksnakeHighScore', highScore);
                }
                ghostActiveTimer = 50;
                ghostFood = null;
            }

            if (specialFoodTimer > 0) {'''
code = code.replace('if (specialFoodTimer > 0) {', ghost_logic)

# drawing
code = code.replace("ctx.fillStyle = '#0f0';", "ctx.fillStyle = ghostActiveTimer > 0 ? '#0ff' : '#0f0';")
code = code.replace('''if (specialFood) {''', '''if (ghostFood) {
                ctx.fillStyle = '#0ff';
                ctx.fillRect(ghostFood.x * CELL, ghostFood.y * CELL, CELL - 1, CELL - 1);
            }
            if (specialFood) {''')

# Text output
code = code.replace("ctx.fillText(`Score: ${score}  High: ${highScore}`, 5, 15);", "ctx.fillText(campaignMode ? `Lvl: ${campaignLevel} Score: ${score} High: ${highScore}` : `Score: ${score} High: ${highScore}`, 5, 15);")

# Input
code = code.replace("if (e.key === '1') { difficulty = 0; init(); }", "if (e.key === '1') { difficulty = 0; campaignMode = false; init(); }")
code = code.replace("if (e.key === '2') { difficulty = 1; init(); }", "if (e.key === '2') { difficulty = 1; campaignMode = false; init(); }")
code = code.replace("if (e.key === '3') { difficulty = 2; init(); }", "if (e.key === '3') { difficulty = 2; campaignMode = false; init(); }\n                if (e.key === '4') { campaignMode = true; campaignLevel = 1; init(); }")

with open(r'd:\KiloApps\KiloOS\public\apps\ksnake.html', 'w', encoding='utf-8') as f:
    f.write(code)
