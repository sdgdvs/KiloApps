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
int game_over = 0;

void InitGame() {
    snake_len = 3;
    snake[0].x = 5; snake[0].y = 5;
    snake[1].x = 4; snake[1].y = 5;
    snake[2].x = 3; snake[2].y = 5;
    dir_x = 1; dir_y = 0;
    game_over = 0;
}

unsigned int rng_state = 12345;
int random_int(int max) {
    rng_state = rng_state * 1103515245 + 12345;
    return ((rng_state >> 16) & 0x7FFF) % max;
}

void PlaceFood() {
    food.x = random_int(GRID_WIDTH);
    food.y = random_int(GRID_HEIGHT);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            InitGame();
            SetTimer(hwnd, TIMER_ID, 150, NULL);
            break;
        case WM_TIMER: {
            if (game_over) break;
            
            // Move body
            for(int i = snake_len - 1; i > 0; i--) {
                snake[i] = snake[i-1];
            }
            // Move head
            snake[0].x += dir_x;
            snake[0].y += dir_y;

            // Collision with walls
            if (snake[0].x < 0 || snake[0].x >= GRID_WIDTH || 
                snake[0].y < 0 || snake[0].y >= GRID_HEIGHT) {
                game_over = 1;
            }

            // Collision with self
            for(int i = 1; i < snake_len; i++) {
                if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
                    game_over = 1;
                }
            }

            // Eat food
            if (snake[0].x == food.x && snake[0].y == food.y) {
                if (snake_len < 400) {
                    snake[snake_len] = snake[snake_len-1]; // grow
                    snake_len++;
                }
                PlaceFood();
            }

            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        case WM_KEYDOWN: {
            if (game_over && wParam == VK_RETURN) {
                InitGame();
            } else if (!game_over) {
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

            if (game_over) {
                SetTextColor(hdc, RGB(255, 0, 0));
                SetBkMode(hdc, TRANSPARENT);
                TextOutA(hdc, 50, 100, "GAME OVER", 9);
                TextOutA(hdc, 30, 130, "Press ENTER to restart", 22);
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
