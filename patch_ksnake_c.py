import sys
with open(r'd:\KiloApps\KSnake\main.c', 'r', encoding='utf-8') as f:
    code = f.read()

# 1. State vars
state_vars = '''int wrap_mode = 0;
int campaign_mode = 0;
int campaign_level = 1;
int apples_eaten = 0;
struct Point ghost_food = {-1, -1};
int ghost_food_timer = 0;
int ghost_active_timer = 0;

void LoadHighScore() {
    FILE* f = fopen("ksnake.dat", "rb");
    if(f) { fread(&high_score, sizeof(int), 1, f); fclose(f); }
}
void SaveHighScore() {
    FILE* f = fopen("ksnake.dat", "wb");
    if(f) { fwrite(&high_score, sizeof(int), 1, f); fclose(f); }
}
'''
code = code.replace('int wrap_mode = 0;', state_vars)

# 2. InitGame updates
code = code.replace('void InitGame() {', '''void InitGame() {
    apples_eaten = 0;
    if(campaign_mode) {
        difficulty = 1;
    }''')
code = code.replace('special_food_timer = 0;', '''special_food_timer = 0;
    ghost_food.x = -1; ghost_food.y = -1;
    ghost_food_timer = 0;
    ghost_active_timer = 0;''')

# Change obstacle logic to support campaign
obs_logic = '''num_obstacles = difficulty * 10;
    if (campaign_mode) {
        num_obstacles = campaign_level * 5;
        if (num_obstacles > 45) num_obstacles = 45;
    }
    for(int i=0; i<num_obstacles; i++) {'''
code = code.replace('''num_obstacles = difficulty * 10;
    for(int i=0; i<num_obstacles; i++) {''', obs_logic)

# Ghost food placement
code = code.replace('void PlaceSpecialFood() {', '''void PlaceGhostFood() {
    int ok = 0;
    while(!ok) {
        ghost_food.x = random_int(GRID_WIDTH); ghost_food.y = random_int(GRID_HEIGHT);
        ok = 1;
        for(int i=0; i<num_obstacles; i++) if(ghost_food.x==obstacles[i].x && ghost_food.y==obstacles[i].y) ok=0;
        for(int i=0; i<snake_len; i++) if(ghost_food.x==snake[i].x && ghost_food.y==snake[i].y) ok=0;
        if(ghost_food.x == food.x && ghost_food.y == food.y) ok=0;
        if(ghost_food.x == special_food.x && ghost_food.y == special_food.y) ok=0;
    }
    ghost_food_timer = 40;
}

void PlaceSpecialFood() {''')

# Timer tick collision handling
code = code.replace('''// Collision with self
            for(int i = 1; i < snake_len; i++) {
                if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
                    game_state = 2;
                }
            }

            // Collision with obstacles
            for(int i = 0; i < num_obstacles; i++) {
                if (snake[0].x == obstacles[i].x && snake[0].y == obstacles[i].y) {
                    game_state = 2;
                }
            }''', '''// Collision with self and obstacles
            if(ghost_active_timer == 0) {
                for(int i = 1; i < snake_len; i++) {
                    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) game_state = 2;
                }
                for(int i = 0; i < num_obstacles; i++) {
                    if (snake[0].x == obstacles[i].x && snake[0].y == obstacles[i].y) game_state = 2;
                }
            }''')

code = code.replace('if (score > high_score) high_score = score;', 'if (score > high_score) { high_score = score; SaveHighScore(); }')

# Add apples_eaten for campaign logic inside Eat food
code = code.replace('''if (special_food.x == -1 && random_int(100) < 20) {
                    PlaceSpecialFood();
                }''', '''if (special_food.x == -1 && random_int(100) < 20) {
                    PlaceSpecialFood();
                }
                if (ghost_food.x == -1 && random_int(100) < 15) {
                    PlaceGhostFood();
                }
                
                apples_eaten++;
                if (campaign_mode && apples_eaten >= 10) {
                    campaign_level++;
                    base_speed -= 10;
                    if(base_speed < 40) base_speed = 40;
                    InitGame();
                }''')

# Timer for ghost
code = code.replace('''if (special_food_timer > 0) {''', '''
            if (ghost_active_timer > 0) ghost_active_timer--;
            
            if (ghost_food_timer > 0) {
                ghost_food_timer--;
                if (ghost_food_timer == 0) { ghost_food.x = -1; ghost_food.y = -1; }
            }
            if (ghost_food.x != -1 && snake[0].x == ghost_food.x && snake[0].y == ghost_food.y) {
                MessageBeep(MB_ICONASTERISK);
                score += score_mult * 2;
                if (score > high_score) { high_score = score; SaveHighScore(); }
                ghost_active_timer = 50; // ghost for 50 ticks
                ghost_food.x = -1; ghost_food.y = -1;
            }

            if (special_food_timer > 0) {''')

# Input for campaign
code = code.replace('''else if (wParam == '3') { difficulty = 2; InitGame(); SetTimer(hwnd, TIMER_ID, current_speed, NULL); }''', '''else if (wParam == '3') { difficulty = 2; campaign_mode = 0; InitGame(); SetTimer(hwnd, TIMER_ID, current_speed, NULL); }
                else if (wParam == '4') { campaign_mode = 1; campaign_level = 1; InitGame(); SetTimer(hwnd, TIMER_ID, current_speed, NULL); }''')
code = code.replace('''if (wParam == '1') { difficulty = 0; InitGame(); SetTimer(hwnd, TIMER_ID, current_speed, NULL); }''', '''if (wParam == '1') { difficulty = 0; campaign_mode = 0; InitGame(); SetTimer(hwnd, TIMER_ID, current_speed, NULL); }''')
code = code.replace('''else if (wParam == '2') { difficulty = 1; InitGame(); SetTimer(hwnd, TIMER_ID, current_speed, NULL); }''', '''else if (wParam == '2') { difficulty = 1; campaign_mode = 0; InitGame(); SetTimer(hwnd, TIMER_ID, current_speed, NULL); }''')

# Text rendering
code = code.replace('''TextOutA(hdc, 40, 170, "3 - Hard", 8);''', '''TextOutA(hdc, 40, 170, "3 - Hard", 8);
                TextOutA(hdc, 40, 190, "4 - Campaign Mode", 17);''')
code = code.replace('TextOutA(hdc, 40, 190, wrap_text, lstrlenA(wrap_text));', 'TextOutA(hdc, 40, 210, wrap_text, lstrlenA(wrap_text));')

code = code.replace('''HBRUSH snakeBrush = CreateSolidBrush(RGB(0, 255, 0));''', '''HBRUSH snakeBrush;
                if(ghost_active_timer > 0) snakeBrush = CreateSolidBrush(RGB(0, 255, 255));
                else snakeBrush = CreateSolidBrush(RGB(0, 255, 0));''')

# Render Ghost Food
code = code.replace('''if (special_food.x != -1) {''', '''if (ghost_food.x != -1) {
                    HBRUSH gFoodBrush = CreateSolidBrush(RGB(0, 255, 255));
                    RECT gfr = { ghost_food.x * CELL_SIZE, ghost_food.y * CELL_SIZE, 
                                 (ghost_food.x + 1) * CELL_SIZE - 1, (ghost_food.y + 1) * CELL_SIZE - 1 };
                    FillRect(hdc, &gfr, gFoodBrush);
                    DeleteObject(gFoodBrush);
                }
                if (special_food.x != -1) {''')

# Score rendering
code = code.replace('''wsprintf(score_text, "Score: %d  High: %d", score, high_score);''', '''if (campaign_mode) {
                    wsprintf(score_text, "Lvl: %d Score: %d High: %d", campaign_level, score, high_score);
                } else {
                    wsprintf(score_text, "Score: %d  High: %d", score, high_score);
                }''')

code = code.replace('void MainEntry() {', 'void MainEntry() { LoadHighScore();')
code = '#include <stdio.h>\n' + code

with open(r'd:\KiloApps\KSnake\main.c', 'w', encoding='utf-8') as f:
    f.write(code)
