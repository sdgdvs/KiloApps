#include <windows.h>

#define GRID_COLS 10
#define GRID_ROWS 10
#define CELL_SIZE 40

typedef struct {
    int type; // 0=Grass, 1=Tilled, 2=Planted, 3=Grown
    int watered;
    int growth;
} Cell;
Cell grid[GRID_COLS * GRID_ROWS] = {0};
int current_day = 1;
int time_of_day = 0; // 0=Day, 1=Night
HWND hNextDayBtn;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            hNextDayBtn = CreateWindow("BUTTON", "Sleep (Next Day)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                130, 410, 140, 30, hwnd, (HMENU) 1, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            return 0;
        case WM_COMMAND:
            if (LOWORD(wParam) == 1 && time_of_day == 0) {
                time_of_day = 1;
                InvalidateRect(hwnd, NULL, TRUE);
                SetTimer(hwnd, 1, 1000, NULL);
            }
            return 0;
        case WM_TIMER:
            if (wParam == 1) {
                KillTimer(hwnd, 1);
                current_day++;
                time_of_day = 0;
                for (int i = 0; i < GRID_COLS * GRID_ROWS; i++) {
                    if (grid[i].type == 2) {
                        if (grid[i].watered) {
                            grid[i].growth++;
                            if (grid[i].growth >= 2) grid[i].type = 3;
                        } else {
                            grid[i].type = 1; // Dies
                        }
                    }
                    grid[i].watered = 0;
                }
                InvalidateRect(hwnd, NULL, TRUE);
                char title[64];
                wsprintf(title, "KFarm - Day %d", current_day);
                SetWindowText(hwnd, title);
            }
            return 0;
        case WM_LBUTTONDOWN: {
            if (time_of_day == 1) return 0;
            int x = LOWORD(lParam) / CELL_SIZE;
            int y = HIWORD(lParam) / CELL_SIZE;
            if (x >= 0 && x < GRID_COLS && y >= 0 && y < GRID_ROWS) {
                int idx = y * GRID_COLS + x;
                if (grid[idx].type == 0) grid[idx].type = 1;
                else if (grid[idx].type == 1) { grid[idx].type = 2; grid[idx].growth = 0; }
                else if (grid[idx].type == 2 && !grid[idx].watered) grid[idx].watered = 1;
                else if (grid[idx].type == 3) { grid[idx].type = 1; grid[idx].watered = 0; }
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
            
            COLORREF cGrass = time_of_day ? RGB(50, 80, 30) : RGB(139, 195, 74);
            COLORREF cSoil = time_of_day ? RGB(40, 20, 15) : RGB(93, 64, 55);
            COLORREF cWetSoil = time_of_day ? RGB(20, 10, 5) : RGB(62, 39, 35);
            
            HBRUSH hGrass = CreateSolidBrush(cGrass);
            HBRUSH hSoil = CreateSolidBrush(cSoil);
            HBRUSH hWetSoil = CreateSolidBrush(cWetSoil);
            
            HPEN hGridPen = CreatePen(PS_SOLID, 1, time_of_day ? RGB(40, 60, 20) : RGB(104, 159, 56));
            HPEN hSproutPen = CreatePen(PS_SOLID, 3, time_of_day ? RGB(80, 120, 40) : RGB(139, 195, 74));
            HPEN hWheatPen = CreatePen(PS_SOLID, 3, time_of_day ? RGB(150, 120, 40) : RGB(255, 213, 79));

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
                        SelectObject(hdc, grid[idx].type == 2 ? hSproutPen : hWheatPen);
                        int cx = r.left + CELL_SIZE / 2;
                        int cy = r.top + CELL_SIZE / 2;
                        
                        MoveToEx(hdc, cx, r.bottom - 8, NULL);
                        LineTo(hdc, cx, r.top + 8);
                        
                        MoveToEx(hdc, cx, cy, NULL);
                        LineTo(hdc, cx - 8, cy - 8);
                        
                        MoveToEx(hdc, cx, cy + 6, NULL);
                        LineTo(hdc, cx + 8, cy - 4);
                        
                        if (grid[idx].type == 3) {
                            MoveToEx(hdc, cx, cy - 6, NULL);
                            LineTo(hdc, cx - 6, cy - 12);
                        }
                    }
                }
            }
            
            DeleteObject(hGrass);
            DeleteObject(hSoil);
            DeleteObject(hWetSoil);
            DeleteObject(hWheatPen);
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

    RECT rect = {0, 0, GRID_COLS * CELL_SIZE, GRID_ROWS * CELL_SIZE + 50};
    AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KFarm - Day 1", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
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
