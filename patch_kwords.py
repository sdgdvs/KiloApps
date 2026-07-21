import os

c_path = r"d:\KiloApps\KWords\main.c"
with open(c_path, "r", encoding="utf-8") as f:
    content = f.read()

# 1. Update themes definition
old_themes_def = """#define MAX_GRID_SIZE 25
#define CELL_SIZE 30
#define MAX_WORDS 20
#define NUM_THEMES 4"""
new_themes_def = """#define MAX_GRID_SIZE 26
#define CELL_SIZE 30
#define MAX_WORDS 20
#define NUM_THEMES 8"""
content = content.replace(old_themes_def, new_themes_def)

# 2. Update THEMES array
old_themes_arr = """const char* THEMES[NUM_THEMES] = {"Programming", "Animals", "Countries", "Space"};"""
new_themes_arr = """const char* THEMES[NUM_THEMES] = {"Programming", "Animals", "Countries", "Space", "Food", "Sports", "Nature", "History"};"""
content = content.replace(old_themes_arr, new_themes_arr)

# 3. Add new dictionaries
old_space_dict = """        "ZENITH", "LUNAR", "SOLAR", "TELESCOPE", "ASTRONAUT",
        "SPACECRAFT", "OBSERVATORY", "CONSTELLATION", "ZODIAC", "APOLLO"
    }
};"""
new_space_dict = """        "ZENITH", "LUNAR", "SOLAR", "TELESCOPE", "ASTRONAUT",
        "SPACECRAFT", "OBSERVATORY", "CONSTELLATION", "ZODIAC", "APOLLO"
    },
    { // Food
        "PIZZA", "BURGER", "SALAD", "PASTA", "SUSHI",
        "STEAK", "CHEESE", "BREAD", "APPLE", "BANANA",
        "ORANGE", "GRAPE", "CHICKEN", "BACON", "TOMATO",
        "POTATO", "ONION", "GARLIC", "PEPPER", "CARROT",
        "CEREAL", "WAFFLE", "PANCAKE", "MUFFIN", "COOKIE",
        "CHOCOLATE", "VANILLA", "BUTTER", "YOGURT", "HONEY"
    },
    { // Sports
        "SOCCER", "TENNIS", "BASKETBALL", "BASEBALL", "GOLF",
        "RUGBY", "CRICKET", "HOCKEY", "VOLLEYBALL", "SWIMMING",
        "BOXING", "WRESTLING", "CYCLING", "ATHLETICS", "GYMNASTICS",
        "ARCHERY", "FENCING", "BOWLING", "BILLIARDS", "SNOOKER",
        "DARTS", "KARATE", "JUDO", "TAEKWONDO", "SURFING",
        "SKATING", "SKIING", "SNOWBOARD", "ROWING", "SAILING"
    },
    { // Nature
        "FOREST", "RIVER", "MOUNTAIN", "OCEAN", "DESERT",
        "VALLEY", "CANYON", "VOLCANO", "ISLAND", "JUNGLE",
        "GLACIER", "TUNDRA", "PRAIRIE", "SAVANNA", "MARSH",
        "SWAMP", "LAKE", "STREAM", "WATERFALL", "GEYSER",
        "CAVE", "CLIFF", "BEACH", "DUNE", "REEF",
        "FLOWER", "TREE", "BUSH", "GRASS", "FERN"
    },
    { // History
        "EMPIRE", "PHARAOH", "PYRAMID", "CASTLE", "KNIGHT",
        "VIKING", "SAMURAI", "ROMAN", "GREEK", "SPARTAN",
        "AZTEC", "MAYAN", "INCA", "DYNASTY", "REVOLUTION",
        "WARRIOR", "GLADIATOR", "CRUSADE", "RENAISSANCE", "COLONY",
        "TREATY", "ALLIANCE", "MONARCH", "REPUBLIC", "SENATE",
        "CHIEFTAIN", "EMPEROR", "SULTAN", "TSAR", "KAISER"
    }
};"""
content = content.replace(old_space_dict, new_space_dict)

# 4. Update game state variables
old_game_state = """int currentThemeIdx = 0;
int currentGameMode = 0; // 0=Classic, 1=Zen, 2=TimeAttack

char wordsToFind[MAX_WORDS][32];"""
new_game_state = """int currentThemeIdx = 0;
int currentGameMode = 0; // 0=Classic, 1=Zen, 2=TimeAttack, 3=Campaign
int campaignStage = 1;
int magicWands = 3;
int comboMultiplier = 1;
int timeSinceLastFind = 0;

char wordsToFind[MAX_WORDS][32];"""
content = content.replace(old_game_state, new_game_state)

# 5. Add UseWand function before UseHint
old_usehint = "void UseHint(HWND hwnd) {"
new_usewand = """void UseWand(HWND hwnd) {
    if (gameWon || gameOver) return;
    if (currentGameMode == 3 && magicWands <= 0) return;
    if (currentGameMode != 3 && currentScore < 200) return;
    
    int unfound[MAX_WORDS];
    int uncount = 0;
    for(int w=0; w<wordCount; w++) {
        if(!wordsFoundStatus[w]) {
            unfound[uncount++] = w;
        }
    }
    
    if (uncount == 0) return;
    
    int targetIdx = unfound[rand() % uncount];
    
    if (currentGameMode == 3) magicWands--;
    else currentScore -= 200;
    
    int dirs[8][2] = {{0,1}, {1,0}, {1,1}, {-1,1}, {1,-1}, {-1,-1}, {0,-1}, {-1,0}};
    int len = strlen(wordsToFind[targetIdx]);
    bool foundInGrid = false;
    for (int r = 0; r < gridSize && !foundInGrid; r++) {
        for (int c = 0; c < gridSize && !foundInGrid; c++) {
            if (grid[r][c] == wordsToFind[targetIdx][0]) {
                for (int d = 0; d < 8; d++) {
                    bool match = true;
                    for (int i = 0; i < len; i++) {
                        int nr = r + i * dirs[d][0];
                        int nc = c + i * dirs[d][1];
                        if (nr < 0 || nr >= gridSize || nc < 0 || nc >= gridSize || grid[nr][nc] != wordsToFind[targetIdx][i]) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        for (int i = 0; i < len; i++) {
                            foundGrid[r + i * dirs[d][0]][c + i * dirs[d][1]] = true;
                        }
                        wordsFoundStatus[targetIdx] = true;
                        foundCount++;
                        PlaySoundEffect(1);
                        foundInGrid = true;
                        break;
                    }
                }
            }
        }
    }
    
    if (foundCount == wordCount) {
        if (currentGameMode == 3 && campaignStage < 10) {
            campaignStage++;
            PlaySoundEffect(2);
            InitGame();
        } else {
            gameWon = true;
            PlaySoundEffect(2);
            LoadStats();
            gameStats.completed++;
            int bTime = gameStats.bestTimes[currentThemeIdx][currentDifficulty];
            if (bTime == 0 || timerSeconds < bTime) {
                gameStats.bestTimes[currentThemeIdx][currentDifficulty] = timerSeconds;
            }
            SaveStats();
        }
    }
    
    InvalidateRect(hwnd, NULL, FALSE);
}

"""
content = content.replace(old_usehint, new_usewand + old_usehint)

# 6. Update buttons layout
old_buttons = """RECT btnTheme = {180, 10, 275, 35};
RECT btnMode  = {285, 10, 380, 35};
RECT btnEasy = {390, 10, 440, 35};
RECT btnMed  = {450, 10, 515, 35};
RECT btnHard = {525, 10, 575, 35};
RECT btnHint = {585, 10, 635, 35};
RECT btnSave = {645, 10, 695, 35};
RECT btnLoad = {705, 10, 755, 35};
RECT btnStats = {765, 10, 815, 35};
RECT btnHelp = {825, 10, 875, 35};"""

new_buttons = """RECT btnTheme = {180, 10, 260, 35};
RECT btnMode  = {265, 10, 345, 35};
RECT btnEasy = {350, 10, 400, 35};
RECT btnMed  = {405, 10, 465, 35};
RECT btnHard = {470, 10, 520, 35};
RECT btnHint = {525, 10, 575, 35};
RECT btnWand = {580, 10, 630, 35};
RECT btnSave = {635, 10, 685, 35};
RECT btnLoad = {690, 10, 740, 35};
RECT btnStats = {745, 10, 795, 35};
RECT btnHelp = {800, 10, 850, 35};"""
content = content.replace(old_buttons, new_buttons)

# 7. Update timer setup in InitGame
old_timer_init = """    if (currentGameMode == 2) {
        timerSeconds = (gridSize == 10) ? 120 : ((gridSize == 15) ? 180 : 300);
    } else {
        timerSeconds = 0;
    }"""
new_timer_init = """    if (currentGameMode == 3) {
        if (campaignStage == 1) { gridSize = 10; numWordsToFind = 5; timerSeconds = 120; }
        else if (campaignStage == 2) { gridSize = 12; numWordsToFind = 6; timerSeconds = 130; }
        else if (campaignStage == 3) { gridSize = 12; numWordsToFind = 7; timerSeconds = 140; }
        else if (campaignStage == 4) { gridSize = 15; numWordsToFind = 8; timerSeconds = 150; }
        else if (campaignStage == 5) { gridSize = 15; numWordsToFind = 9; timerSeconds = 160; }
        else if (campaignStage == 6) { gridSize = 15; numWordsToFind = 10; timerSeconds = 170; }
        else if (campaignStage == 7) { gridSize = 18; numWordsToFind = 11; timerSeconds = 180; }
        else if (campaignStage == 8) { gridSize = 20; numWordsToFind = 12; timerSeconds = 190; }
        else if (campaignStage == 9) { gridSize = 22; numWordsToFind = 14; timerSeconds = 200; }
        else { gridSize = 25; numWordsToFind = 16; timerSeconds = 240; }
    } else if (currentGameMode == 2) {
        timerSeconds = (gridSize == 10) ? 120 : ((gridSize == 15) ? 180 : 300);
    } else {
        timerSeconds = 0;
    }
    comboMultiplier = 1;
    timeSinceLastFind = 0;"""
content = content.replace(old_timer_init, new_timer_init)


# 8. Update combo logic in EndSelection
old_score_calc = """                    if (currentGameMode != 1) {
                        int points = 1000 - (timerSeconds * 2);
                        if (currentGameMode == 2) points = 500;
                        if (points < 100) points = 100;
                        currentScore += points;
                    }"""
new_score_calc = """                    if (currentGameMode != 1) {
                        int points = 1000 - (timerSeconds * 2);
                        if (currentGameMode == 2 || currentGameMode == 3) points = 500;
                        if (points < 100) points = 100;
                        if (timeSinceLastFind < 15) comboMultiplier++;
                        else comboMultiplier = 1;
                        currentScore += points * comboMultiplier;
                        timeSinceLastFind = 0;
                    }"""
content = content.replace(old_score_calc, new_score_calc)

# 9. Update campaign win logic in EndSelection
old_game_won = """        if (foundCount == wordCount) {
            gameWon = true;
            PlaySoundEffect(2);
            LoadStats();
            gameStats.completed++;
            int bTime = gameStats.bestTimes[currentThemeIdx][currentDifficulty];
            if (bTime == 0 || timerSeconds < bTime) {
                gameStats.bestTimes[currentThemeIdx][currentDifficulty] = timerSeconds;
            }
            SaveStats();
        } else if (found) {"""
new_game_won = """        if (foundCount == wordCount) {
            if (currentGameMode == 3 && campaignStage < 10) {
                campaignStage++;
                PlaySoundEffect(2);
                InitGame();
                return;
            } else {
                gameWon = true;
                PlaySoundEffect(2);
                LoadStats();
                gameStats.completed++;
                int bTime = gameStats.bestTimes[currentThemeIdx][currentDifficulty];
                if (bTime == 0 || timerSeconds < bTime) {
                    gameStats.bestTimes[currentThemeIdx][currentDifficulty] = timerSeconds;
                }
                SaveStats();
            }
        } else if (found) {"""
content = content.replace(old_game_won, new_game_won)

# 10. Update WM_TIMER timeSinceLastFind
old_timer_update = """                    if (currentGameMode == 2) {
                        if (timerSeconds > 0) {
                            timerSeconds--;
                        } else {
                            gameOver = true;
                            PlaySoundEffect(1); // just a sound
                        }
                    } else if (currentGameMode == 0) {
                        timerSeconds++;
                    }"""
new_timer_update = """                    if (currentGameMode == 2 || currentGameMode == 3) {
                        if (timerSeconds > 0) {
                            timerSeconds--;
                        } else {
                            gameOver = true;
                            PlaySoundEffect(1); // just a sound
                        }
                    } else if (currentGameMode == 0) {
                        timerSeconds++;
                    }
                    timeSinceLastFind++;"""
content = content.replace(old_timer_update, new_timer_update)

# 11. Update Mouse clicks
old_mode_click = """            if (PtInRect(&btnMode, pt)) {
                currentGameMode = (currentGameMode + 1) % 3; InitGame(); InvalidateRect(hwnd, NULL, TRUE); break;
            }"""
new_mode_click = """            if (PtInRect(&btnMode, pt)) {
                currentGameMode = (currentGameMode + 1) % 4; if (currentGameMode == 3) { campaignStage = 1; magicWands = 3; currentScore = 0; } InitGame(); InvalidateRect(hwnd, NULL, TRUE); break;
            }"""
content = content.replace(old_mode_click, new_mode_click)

old_btn_click = """            if (PtInRect(&btnHint, pt)) {
                UseHint(hwnd); break;
            }"""
new_btn_click = """            if (PtInRect(&btnHint, pt)) {
                UseHint(hwnd); break;
            }
            if (PtInRect(&btnWand, pt)) {
                UseWand(hwnd); break;
            }"""
content = content.replace(old_btn_click, new_btn_click)


# 12. Update Drawing code
old_paint_btns = """            HBRUSH hintBrush = CreateSolidBrush(RGB(214, 158, 46));
            FillRect(hdc, &btnHint, hintBrush);
            DeleteObject(hintBrush);
            
            HBRUSH slBrush = CreateSolidBrush(RGB(100, 100, 100));"""
new_paint_btns = """            HBRUSH hintBrush = CreateSolidBrush(RGB(214, 158, 46));
            FillRect(hdc, &btnHint, hintBrush);
            FillRect(hdc, &btnWand, hintBrush);
            DeleteObject(hintBrush);
            
            HBRUSH slBrush = CreateSolidBrush(RGB(100, 100, 100));"""
content = content.replace(old_paint_btns, new_paint_btns)

old_paint_modes = """            const char* MODES[3] = {"Classic", "Zen", "Time Attack"};
            char modeStr[64];
            sprintf(modeStr, "Mode: %s", MODES[currentGameMode]);"""
new_paint_modes = """            const char* MODES[4] = {"Classic", "Zen", "Time Attack", "Campaign"};
            char modeStr[64];
            if (currentGameMode == 3) sprintf(modeStr, "Stage %d", campaignStage);
            else sprintf(modeStr, "Mode: %s", MODES[currentGameMode]);"""
content = content.replace(old_paint_modes, new_paint_modes)

old_paint_texts = """            TextOut(hdc, btnHint.left + 8, btnHint.top + 5, "Hint", 4);
            SetTextColor(hdc, RGB(226, 232, 240));
            
            TextOut(hdc, btnSave.left + 8, btnSave.top + 5, "Save", 4);"""
new_paint_texts = """            TextOut(hdc, btnHint.left + 8, btnHint.top + 5, "Hint", 4);
            TextOut(hdc, btnWand.left + 6, btnWand.top + 5, "Wand", 4);
            SetTextColor(hdc, RGB(226, 232, 240));
            
            TextOut(hdc, btnSave.left + 8, btnSave.top + 5, "Save", 4);"""
content = content.replace(old_paint_texts, new_paint_texts)

# 13. Update Header Score drawing
old_header = """            char header[128];
            if (currentGameMode == 1) {
                sprintf(header, "Found: %d/%d   Score: -   Timer: --:--", foundCount, wordCount);
            } else {
                sprintf(header, "Found: %d/%d   Score: %d   Timer: %02d:%02d", foundCount, wordCount, currentScore, timerSeconds/60, timerSeconds%60);
            }
            TextOut(hdc, 20, 15, header, strlen(header));"""
new_header = """            char header[128];
            if (currentGameMode == 1) {
                sprintf(header, "Found: %d/%d   Score: -   Timer: --:--", foundCount, wordCount);
            } else if (currentGameMode == 3) {
                sprintf(header, "Found: %d/%d   Score: %d   Timer: %02d:%02d   Wands: %d", foundCount, wordCount, currentScore, timerSeconds/60, timerSeconds%60, magicWands);
            } else {
                sprintf(header, "Found: %d/%d   Score: %d   Timer: %02d:%02d", foundCount, wordCount, currentScore, timerSeconds/60, timerSeconds%60);
            }
            TextOut(hdc, 20, 15, header, strlen(header));"""
content = content.replace(old_header, new_header)

with open(c_path, "w", encoding="utf-8") as f:
    f.write(content)

print("Patch applied to C version.")
