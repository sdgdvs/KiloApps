#include <windows.h>

#define GRID_COLS 10
#define GRID_ROWS 10
#define CELL_SIZE 40

int grid[GRID_COLS * GRID_ROWS] = {0}; // 0=Grass, 1=Tilled, 2=Planted

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam) / CELL_SIZE;
            int y = HIWORD(lParam) / CELL_SIZE;
            if (x >= 0 && x < GRID_COLS && y >= 0 && y < GRID_ROWS) {
                int idx = y * GRID_COLS + x;
                if (grid[idx] == 0) grid[idx] = 1;
                else if (grid[idx] == 1) grid[idx] = 2;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HBRUSH hGrass = CreateSolidBrush(RGB(76, 175, 80));
            HBRUSH hSoil = CreateSolidBrush(RGB(121, 85, 72));
            HBRUSH hWheat = CreateSolidBrush(RGB(255, 235, 59));
            HPEN hGridPen = CreatePen(PS_SOLID, 1, RGB(34, 139, 34));

            for (int y = 0; y < GRID_ROWS; y++) {
                for (int x = 0; x < GRID_COLS; x++) {
                    int idx = y * GRID_COLS + x;
                    RECT r = { x * CELL_SIZE, y * CELL_SIZE, (x+1) * CELL_SIZE, (y+1) * CELL_SIZE };
                    
                    if (grid[idx] == 0) FillRect(hdc, &r, hGrass);
                    else FillRect(hdc, &r, hSoil);
                    
                    SelectObject(hdc, hGridPen);
                    SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    Rectangle(hdc, r.left, r.top, r.right, r.bottom);
                    
                    if (grid[idx] == 2) {
                        RECT wheatR = { r.left + 10, r.top + 10, r.right - 10, r.bottom - 10 };
                        FillRect(hdc, &wheatR, hWheat);
                    }
                }
            }
            
            DeleteObject(hGrass);
            DeleteObject(hSoil);
            DeleteObject(hWheat);
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
    wc.hbrBackground = CreateSolidBrush(RGB(34, 34, 34));

    RegisterClass(&wc);

    RECT rect = {0, 0, GRID_COLS * CELL_SIZE, GRID_ROWS * CELL_SIZE};
    AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KFarm", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
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
