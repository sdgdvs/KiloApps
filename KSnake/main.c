#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define CELL_SIZE 15
#define GRID_WIDTH 20
#define GRID_HEIGHT 20
#define TIMER_ID 1

struct Point { int x; int y; };
struct Point snake[400];
int snake_len = 3;
int dir_x = 1, dir_y = 0;
struct Point food = { 10, 10 };
struct Point special_food = { -1, -1 };
int special_food_timer = 0;
struct Point obstacles[50];
int num_obstacles = 0;
int game_state = 0; // 0 = Menu, 1 = Playing, 2 = Game Over
int difficulty = 1; // 0=Easy, 1=Medium, 2=Hard
int score = 0;
int high_score = 0;
int current_speed = 150;
int base_speed = 150;
int score_mult = 10;
int wrap_mode = 0;


void InitGame() {
    snake_len = 3;
    snake[0].x = 5; snake[0].y = 5;
    snake[1].x = 4; snake[1].y = 5;
    snake[2].x = 3; snake[2].y = 5;
    dir_x = 1; dir_y = 0;
    
    if (difficulty == 0) { base_speed = 200; score_mult = 5; }
    else if (difficulty == 1) { base_speed = 150; score_mult = 10; }
    else { base_speed = 100; score_mult = 20; }
    
    current_speed = base_speed;
    game_state = 1;
    score = 0;
    special_food.x = -1;
    special_food.y = -1;
    special_food_timer = 0;
    
    num_obstacles = difficulty * 10;
    for(int i=0; i<num_obstacles; i++) {
        int ok = 0;
        while(!ok) {
            obstacles[i].x = random_int(GRID_WIDTH);
            obstacles[i].y = random_int(GRID_HEIGHT);
            ok = 1;
            if (obstacles[i].y == 5 && (obstacles[i].x >= 2 && obstacles[i].x <= 6)) ok = 0;
        }
    }
    PlaceFood();
}

unsigned int rng_state = 12345;
int random_int(int max) {
    rng_state = rng_state * 1103515245 + 12345;
    return ((rng_state >> 16) & 0x7FFF) % max;
}

void PlaceFood() {
    int ok = 0;
    while(!ok) {
        food.x = random_int(GRID_WIDTH);
        food.y = random_int(GRID_HEIGHT);
        ok = 1;
        for(int i=0; i<num_obstacles; i++) {
            if (food.x == obstacles[i].x && food.y == obstacles[i].y) ok = 0;
        }
        for(int i=0; i<snake_len; i++) {
            if (food.x == snake[i].x && food.y == snake[i].y) ok = 0;
        }
    }
}

void PlaceSpecialFood() {
    int ok = 0;
    while(!ok) {
        special_food.x = random_int(GRID_WIDTH);
        special_food.y = random_int(GRID_HEIGHT);
        ok = 1;
        for(int i=0; i<num_obstacles; i++) {
            if (special_food.x == obstacles[i].x && special_food.y == obstacles[i].y) ok = 0;
        }
        for(int i=0; i<snake_len; i++) {
            if (special_food.x == snake[i].x && special_food.y == snake[i].y) ok = 0;
        }
        if (special_food.x == food.x && special_food.y == food.y) ok = 0;
    }
    special_food_timer = 40;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            // don't start timer yet, wait in menu
            break;
        case WM_TIMER: {
            if (game_state != 1) break;
            
            // Move body
            for(int i = snake_len - 1; i > 0; i--) {
                snake[i] = snake[i-1];
            }
            // Move head
            snake[0].x += dir_x;
            snake[0].y += dir_y;

            // Collision with walls
            if (wrap_mode) {
                if (snake[0].x < 0) snake[0].x = GRID_WIDTH - 1;
                else if (snake[0].x >= GRID_WIDTH) snake[0].x = 0;
                
                if (snake[0].y < 0) snake[0].y = GRID_HEIGHT - 1;
                else if (snake[0].y >= GRID_HEIGHT) snake[0].y = 0;
            } else {
                if (snake[0].x < 0 || snake[0].x >= GRID_WIDTH || 
                    snake[0].y < 0 || snake[0].y >= GRID_HEIGHT) {
                    game_state = 2;
                }
            }

            // Collision with self
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
            }
            
            if (game_state == 2) {
                MessageBeep(MB_ICONHAND);
            }

            // Eat food
            if (snake[0].x == food.x && snake[0].y == food.y) {
                MessageBeep(MB_OK);
                if (snake_len < 400) {
                    snake[snake_len] = snake[snake_len-1]; // grow
                    snake_len++;
                }
                score += score_mult;
                if (score > high_score) high_score = score;
                if (current_speed > 30) current_speed -= 2;
                SetTimer(hwnd, TIMER_ID, current_speed, NULL);
                PlaceFood();
                
                if (special_food.x == -1 && random_int(100) < 20) {
                    PlaceSpecialFood();
                }
            }

            if (special_food_timer > 0) {
                special_food_timer--;
                if (special_food_timer == 0) {
                    special_food.x = -1;
                    special_food.y = -1;
                }
            }

            if (special_food.x != -1 && snake[0].x == special_food.x && snake[0].y == special_food.y) {
                MessageBeep(MB_ICONASTERISK);
                score += score_mult * 5;
                if (score > high_score) high_score = score;
                snake_len -= 3;
                if (snake_len < 3) snake_len = 3;
                special_food.x = -1;
                special_food.y = -1;
            }

            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        case WM_KEYDOWN: {
            if (game_state == 0) {
                if (wParam == '1') { difficulty = 0; InitGame(); SetTimer(hwnd, TIMER_ID, current_speed, NULL); }
                else if (wParam == '2') { difficulty = 1; InitGame(); SetTimer(hwnd, TIMER_ID, current_speed, NULL); }
                else if (wParam == '3') { difficulty = 2; InitGame(); SetTimer(hwnd, TIMER_ID, current_speed, NULL); }
                else if (wParam == 'W') { wrap_mode = !wrap_mode; InvalidateRect(hwnd, NULL, TRUE); }
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (game_state == 2 && wParam == VK_RETURN) {
                game_state = 0;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (game_state == 1) {
                if (wParam == VK_UP && dir_y != 1) { dir_x = 0; dir_y = -1; }
                if (wParam == VK_DOWN && dir_y != -1) { dir_x = 0; dir_y = 1; }
                if (wParam == VK_LEFT && dir_x != 1) { dir_x = -1; dir_y = 0; }
                if (wParam == VK_RIGHT && dir_x != -1) { dir_x = 1; dir_y = 0; }
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HBRUSH bg = CreateSolidBrush(RGB(30, 30, 30));
            FillRect(hdc, &ps.rcPaint, bg);
            DeleteObject(bg);

            if (game_state == 0) {
                SetTextColor(hdc, RGB(255, 255, 255));
                SetBkMode(hdc, TRANSPARENT);
                TextOutA(hdc, 80, 50, "KSNAKE", 6);
                TextOutA(hdc, 40, 100, "Select Difficulty:", 18);
                TextOutA(hdc, 40, 130, "1 - Easy", 8);
                TextOutA(hdc, 40, 150, "2 - Medium", 10);
                TextOutA(hdc, 40, 170, "3 - Hard", 8);
                char wrap_text[32];
                wsprintf(wrap_text, "W - Toggle Wrap: %s", wrap_mode ? "ON" : "OFF");
                TextOutA(hdc, 40, 190, wrap_text, lstrlenA(wrap_text));
            } else if (game_state == 2) {
                SetTextColor(hdc, RGB(255, 0, 0));
                SetBkMode(hdc, TRANSPARENT);
                TextOutA(hdc, 100, 100, "GAME OVER", 9);
                TextOutA(hdc, 60, 130, "Press ENTER to return", 21);
            } else {
                HBRUSH snakeBrush = CreateSolidBrush(RGB(0, 255, 0));
                for(int i = 0; i < snake_len; i++) {
                    RECT r = { snake[i].x * CELL_SIZE, snake[i].y * CELL_SIZE, 
                               (snake[i].x + 1) * CELL_SIZE - 1, (snake[i].y + 1) * CELL_SIZE - 1 };
                    FillRect(hdc, &r, snakeBrush);
                }
                DeleteObject(snakeBrush);

                HBRUSH foodBrush = CreateSolidBrush(RGB(255, 0, 0));
                RECT fr = { food.x * CELL_SIZE, food.y * CELL_SIZE, 
                            (food.x + 1) * CELL_SIZE - 1, (food.y + 1) * CELL_SIZE - 1 };
                FillRect(hdc, &fr, foodBrush);
                DeleteObject(foodBrush);

                if (special_food.x != -1) {
                    HBRUSH sFoodBrush = CreateSolidBrush(RGB(255, 215, 0));
                    RECT sfr = { special_food.x * CELL_SIZE, special_food.y * CELL_SIZE, 
                                 (special_food.x + 1) * CELL_SIZE - 1, (special_food.y + 1) * CELL_SIZE - 1 };
                    FillRect(hdc, &sfr, sFoodBrush);
                    DeleteObject(sFoodBrush);
                }

                HBRUSH obsBrush = CreateSolidBrush(RGB(100, 100, 100));
                for(int i = 0; i < num_obstacles; i++) {
                    RECT or = { obstacles[i].x * CELL_SIZE, obstacles[i].y * CELL_SIZE, 
                                (obstacles[i].x + 1) * CELL_SIZE - 1, (obstacles[i].y + 1) * CELL_SIZE - 1 };
                    FillRect(hdc, &or, obsBrush);
                }
                DeleteObject(obsBrush);
            }

            if (game_state != 0) {
                RECT clientRect;
                GetClientRect(hwnd, &clientRect);
                HBRUSH borderBrush = CreateSolidBrush(RGB(102, 102, 102));
                FrameRect(hdc, &clientRect, borderBrush);
                InflateRect(&clientRect, -1, -1);
                FrameRect(hdc, &clientRect, borderBrush);
                DeleteObject(borderBrush);
                
                char score_text[64];
                wsprintf(score_text, "Score: %d  High: %d", score, high_score);
                SetTextColor(hdc, RGB(255, 255, 255));
                SetBkMode(hdc, TRANSPARENT);
                TextOutA(hdc, 5, 5, score_text, lstrlenA(score_text));
            }

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KSnakeApp";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    
    // Try to load custom icon, ignore if fails
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));

    RegisterClass(&wc);

    int winWidth = GRID_WIDTH * CELL_SIZE + 16;
    int winHeight = GRID_HEIGHT * CELL_SIZE + 39;

    HWND hwnd = CreateWindowEx(0, "KSnakeApp", "KSnake", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, winWidth, winHeight, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
