#include <windows.h>

#define GRID_COLS 10
#define GRID_ROWS 10
#define CELL_SIZE 40

typedef struct {
    int type; // 0=Grass, 1=Tilled, 2=Planted, 3=Grown
    int watered;
    int growth;
    int cropType;
} Cell;
Cell grid[GRID_COLS * GRID_ROWS] = {0};
int current_day = 1;
int time_of_day = 0; // 0=Day, 1=Night
int current_season = 0; // 0=Spring, 1=Summer, 2=Fall, 3=Winter
int crop_seasons[4] = {5, 2, 6, 4}; // Bitmasks: Wheat(0,2)->5, Corn(1)->2, Tomato(1,2)->6, Pumpkin(2)->4
const char* season_names[4] = {"Spring", "Summer", "Fall", "Winter"};
int growth_times[4] = {2, 3, 4, 5};
int sell_values[4] = {10, 20, 30, 50};
int seed_costs[4] = {5, 10, 15, 25};
int money = 50;
int fertilizer_bought = 0;
int tools_upgraded = 0;
int selected_seed = 0;
int chickens = 0;
int cows = 0;
HWND hNextDayBtn;
HWND hSeedBtns[4];
HWND hUpgradeBtn;
HWND hUpgradeToolsBtn;
HWND hBuyChickenBtn;
HWND hBuyCowBtn;

void UpdateTitle(HWND hwnd) {
    char title[128];
    wsprintf(title, "KFarm - %s, Day %d | $%d | Ch:%d | Co:%d", season_names[current_season], current_day, money, chickens, cows);
    SetWindowText(hwnd, title);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            hUpgradeToolsBtn = CreateWindow("BUTTON", "Tools ($200)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                10, 410, 110, 30, hwnd, (HMENU) 9, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hNextDayBtn = CreateWindow("BUTTON", "Sleep (Next Day)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                130, 410, 140, 30, hwnd, (HMENU) 1, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hUpgradeBtn = CreateWindow("BUTTON", "Fertilizer ($100)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                280, 410, 110, 30, hwnd, (HMENU) 6, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hBuyChickenBtn = CreateWindow("BUTTON", "Chicken ($50)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                290, 450, 100, 20, hwnd, (HMENU) 7, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hBuyCowBtn = CreateWindow("BUTTON", "Cow ($150)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                290, 475, 100, 20, hwnd, (HMENU) 8, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hSeedBtns[0] = CreateWindow("BUTTON", "Wheat (-$5) [Sp/Fa]", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,
                10, 450, 130, 20, hwnd, (HMENU) 2, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hSeedBtns[1] = CreateWindow("BUTTON", "Corn (-$10) [Su]", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
                150, 450, 130, 20, hwnd, (HMENU) 3, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hSeedBtns[2] = CreateWindow("BUTTON", "Tomato (-$15) [Su/Fa]", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
                10, 475, 130, 20, hwnd, (HMENU) 4, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hSeedBtns[3] = CreateWindow("BUTTON", "Pumpkin (-$25) [Fa]", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
                150, 475, 140, 20, hwnd, (HMENU) 5, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            SendMessage(hSeedBtns[0], BM_SETCHECK, BST_CHECKED, 0);
            return 0;
        case WM_COMMAND:
            if (LOWORD(wParam) == 1 && time_of_day == 0) {
                time_of_day = 1;
                InvalidateRect(hwnd, NULL, TRUE);
                SetTimer(hwnd, 1, 1000, NULL);
            }
            if (LOWORD(wParam) == 6 && time_of_day == 0 && !fertilizer_bought) {
                if (money >= 100) {
                    money -= 100;
                    fertilizer_bought = 1;
                    for(int i=0; i<4; i++) if (growth_times[i] > 1) growth_times[i]--;
                    EnableWindow(hUpgradeBtn, FALSE);
                    SetWindowText(hUpgradeBtn, "Fertilizer (Owned)");
                    UpdateTitle(hwnd);
                } else {
                    MessageBeep(MB_ICONERROR);
                }
            }
            if (LOWORD(wParam) == 7 && time_of_day == 0) {
                if (money >= 50) {
                    money -= 50;
                    chickens++;
                    UpdateTitle(hwnd);
                } else {
                    MessageBeep(MB_ICONERROR);
                }
            }
            if (LOWORD(wParam) == 8 && time_of_day == 0) {
                if (money >= 150) {
                    money -= 150;
                    cows++;
                    UpdateTitle(hwnd);
                } else {
                    MessageBeep(MB_ICONERROR);
                }
            }
            if (LOWORD(wParam) == 9 && time_of_day == 0 && !tools_upgraded) {
                if (money >= 200) {
                    money -= 200;
                    tools_upgraded = 1;
                    EnableWindow(hUpgradeToolsBtn, FALSE);
                    SetWindowText(hUpgradeToolsBtn, "Tools (Owned)");
                    UpdateTitle(hwnd);
                } else {
                    MessageBeep(MB_ICONERROR);
                }
            }
            if (LOWORD(wParam) >= 2 && LOWORD(wParam) <= 5) {
                selected_seed = LOWORD(wParam) - 2;
            }
            return 0;
        case WM_TIMER:
            if (wParam == 1) {
                KillTimer(hwnd, 1);
                current_day++;
                current_season = ((current_day - 1) / 7) % 4;
                time_of_day = 0;
                money += (chickens * 5) + (cows * 15);
                for (int i = 0; i < GRID_COLS * GRID_ROWS; i++) {
                    if (grid[i].type == 2 || grid[i].type == 3) {
                        if ((crop_seasons[grid[i].cropType] & (1 << current_season)) == 0) {
                            grid[i].type = 1; // Dies from season change
                        } else if (grid[i].type == 2) {
                            if (grid[i].watered) {
                                grid[i].growth++;
                                if (grid[i].growth >= growth_times[grid[i].cropType]) grid[i].type = 3;
                            } else {
                                grid[i].type = 1; // Dies without water
                            }
                        }
                    }
                    grid[i].watered = 0;
                }
                InvalidateRect(hwnd, NULL, TRUE);
                UpdateTitle(hwnd);
            }
            return 0;
        case WM_LBUTTONDOWN: {
            if (time_of_day == 1) return 0;
            int cx = LOWORD(lParam) / CELL_SIZE;
            int cy = HIWORD(lParam) / CELL_SIZE;
            if (cx >= 0 && cx < GRID_COLS && cy >= 0 && cy < GRID_ROWS) {
                int c_idx = cy * GRID_COLS + cx;
                int action = -1; // 0=till, 1=plant, 2=water, 3=harvest
                
                if (grid[c_idx].type == 0) action = 0;
                else if (grid[c_idx].type == 1) action = 1;
                else if (grid[c_idx].type == 2 && !grid[c_idx].watered) action = 2;
                else if (grid[c_idx].type == 3) action = 3;
                
                if (action == -1) return 0;
                
                int is_aoe = tools_upgraded && (action == 0 || action == 2 || action == 3);
                int min_x = is_aoe ? (cx > 0 ? cx - 1 : 0) : cx;
                int max_x = is_aoe ? (cx < GRID_COLS - 1 ? cx + 1 : GRID_COLS - 1) : cx;
                int min_y = is_aoe ? (cy > 0 ? cy - 1 : 0) : cy;
                int max_y = is_aoe ? (cy < GRID_ROWS - 1 ? cy + 1 : GRID_ROWS - 1) : cy;
                
                for (int y = min_y; y <= max_y; y++) {
                    for (int x = min_x; x <= max_x; x++) {
                        int idx = y * GRID_COLS + x;
                        if (action == 0 && grid[idx].type == 0) {
                            grid[idx].type = 1;
                        } else if (action == 1 && x == cx && y == cy && grid[idx].type == 1) {
                            if ((crop_seasons[selected_seed] & (1 << current_season)) != 0 && money >= seed_costs[selected_seed]) {
                                money -= seed_costs[selected_seed];
                                grid[idx].type = 2; grid[idx].growth = 0; grid[idx].cropType = selected_seed;
                            } else {
                                MessageBeep(MB_ICONERROR);
                            }
                        } else if (action == 2 && grid[idx].type == 2 && !grid[idx].watered) {
                            grid[idx].watered = 1;
                        } else if (action == 3 && grid[idx].type == 3) {
                            money += sell_values[grid[idx].cropType];
                            grid[idx].type = 1; grid[idx].watered = 0;
                        }
                    }
                }
                UpdateTitle(hwnd);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            HBRUSH hSky = CreateSolidBrush(time_of_day ? RGB(26, 35, 126) : RGB(135, 206, 235));
            FillRect(hdc, &clientRect, hSky);
            DeleteObject(hSky);
            
            COLORREF grass_colors[4] = { RGB(139, 195, 74), RGB(76, 175, 80), RGB(255, 179, 0), RGB(224, 247, 250) };
            COLORREF night_grass_colors[4] = { RGB(51, 80, 30), RGB(30, 70, 40), RGB(100, 70, 10), RGB(100, 120, 130) };
            COLORREF cGrass = time_of_day ? night_grass_colors[current_season] : grass_colors[current_season];
            COLORREF cSoil = time_of_day ? RGB(40, 20, 15) : RGB(93, 64, 55);
            COLORREF cWetSoil = time_of_day ? RGB(20, 10, 5) : RGB(62, 39, 35);
            
            HBRUSH hGrass = CreateSolidBrush(cGrass);
            HBRUSH hSoil = CreateSolidBrush(cSoil);
            HBRUSH hWetSoil = CreateSolidBrush(cWetSoil);
            
            HPEN hGridPen = CreatePen(PS_SOLID, 1, time_of_day ? RGB(40, 60, 20) : RGB(104, 159, 56));
            HPEN hSproutPen = CreatePen(PS_SOLID, 3, time_of_day ? RGB(80, 120, 40) : RGB(139, 195, 74));
            HPEN hWheatPen = CreatePen(PS_SOLID, 3, time_of_day ? RGB(150, 120, 40) : RGB(255, 213, 79));
            HPEN hCornPen = CreatePen(PS_SOLID, 3, time_of_day ? RGB(150, 150, 0) : RGB(255, 235, 59));
            HPEN hPumpkinPen = CreatePen(PS_SOLID, 3, time_of_day ? RGB(150, 90, 0) : RGB(255, 152, 0));

            for (int y = 0; y < GRID_ROWS; y++) {
                for (int x = 0; x < GRID_COLS; x++) {
                    int idx = y * GRID_COLS + x;
                    RECT r = { x * CELL_SIZE, y * CELL_SIZE, (x+1) * CELL_SIZE, (y+1) * CELL_SIZE };
                    
                    if (grid[idx].type == 0) FillRect(hdc, &r, hGrass);
                    else if (grid[idx].watered) FillRect(hdc, &r, hWetSoil);
                    else FillRect(hdc, &r, hSoil);
                    
                    SelectObject(hdc, hGridPen);
                    SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    Rectangle(hdc, r.left, r.top, r.right, r.bottom);
                    
                    if (grid[idx].type == 2 || grid[idx].type == 3) {
                        int cx = r.left + CELL_SIZE / 2;
                        int cy = r.top + CELL_SIZE / 2;
                        
                        if (grid[idx].type == 2) {
                            SelectObject(hdc, hSproutPen);
                            MoveToEx(hdc, cx, r.bottom - 8, NULL); LineTo(hdc, cx, r.top + 16);
                            MoveToEx(hdc, cx, cy + 6, NULL); LineTo(hdc, cx - 6, cy);
                            MoveToEx(hdc, cx, cy + 10, NULL); LineTo(hdc, cx + 6, cy + 4);
                        } else {
                            if (grid[idx].cropType == 0) { // Wheat
                                SelectObject(hdc, hWheatPen);
                                MoveToEx(hdc, cx, r.bottom - 8, NULL); LineTo(hdc, cx, r.top + 8);
                                MoveToEx(hdc, cx, cy, NULL); LineTo(hdc, cx - 8, cy - 8);
                                MoveToEx(hdc, cx, cy + 6, NULL); LineTo(hdc, cx + 8, cy - 4);
                                MoveToEx(hdc, cx, cy - 6, NULL); LineTo(hdc, cx - 6, cy - 12);
                            } else if (grid[idx].cropType == 1) { // Corn
                                SelectObject(hdc, hCornPen);
                                MoveToEx(hdc, cx, r.bottom - 8, NULL); LineTo(hdc, cx, r.top + 4);
                                HBRUSH hYellow = CreateSolidBrush(time_of_day ? RGB(150, 150, 0) : RGB(255, 235, 59));
                                SelectObject(hdc, hYellow);
                                SelectObject(hdc, GetStockObject(NULL_PEN));
                                Ellipse(hdc, cx - 4, cy - 8, cx + 4, cy + 8);
                                DeleteObject(hYellow);
                            } else if (grid[idx].cropType == 2) { // Tomato
                                SelectObject(hdc, hSproutPen);
                                MoveToEx(hdc, cx, r.bottom - 8, NULL); LineTo(hdc, cx, r.top + 8);
                                HBRUSH hRed = CreateSolidBrush(time_of_day ? RGB(150, 40, 40) : RGB(244, 67, 54));
                                SelectObject(hdc, hRed);
                                SelectObject(hdc, GetStockObject(NULL_PEN));
                                Ellipse(hdc, cx - 6, cy - 6, cx, cy);
                                Ellipse(hdc, cx + 2, cy - 2, cx + 8, cy + 4);
                                Ellipse(hdc, cx - 4, r.top + 8, cx + 4, r.top + 16);
                                DeleteObject(hRed);
                            } else if (grid[idx].cropType == 3) { // Pumpkin
                                SelectObject(hdc, hSproutPen);
                                MoveToEx(hdc, cx, r.bottom - 8, NULL); LineTo(hdc, cx - 8, r.bottom - 8);
                                HBRUSH hOrange = CreateSolidBrush(time_of_day ? RGB(150, 90, 0) : RGB(255, 152, 0));
                                SelectObject(hdc, hOrange);
                                SelectObject(hdc, hPumpkinPen);
                                Ellipse(hdc, cx - 12, r.bottom - 24, cx + 12, r.bottom - 4);
                                DeleteObject(hOrange);
                            }
                        }
                    }
                }
            }
            
            DeleteObject(hGrass);
            DeleteObject(hSoil);
            DeleteObject(hWetSoil);
            DeleteObject(hWheatPen);
            DeleteObject(hCornPen);
            DeleteObject(hPumpkinPen);
            DeleteObject(hSproutPen);
            DeleteObject(hGridPen);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    const char CLASS_NAME[]  = "KFarmClass";
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(135, 206, 235));

    RegisterClass(&wc);

    RECT rect = {0, 0, GRID_COLS * CELL_SIZE, GRID_ROWS * CELL_SIZE + 105};
    AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KFarm - Spring, Day 1 | $50 | Ch:0 | Co:0", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
