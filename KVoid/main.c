#include <windows.h>

#define COLS 20
#define ROWS 15
#define TILE_SIZE 32
#define WINDOW_WIDTH (COLS * TILE_SIZE)
#define WINDOW_HEIGHT (ROWS * TILE_SIZE)

// Map (1 = wall, 0 = floor)
int map[ROWS][COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,0,0,1,1,1,1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0,0,0,0,1},
    {1,1,1,0,1,1,1,0,0,1,1,0,1,1,1,1,1,0,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,1},
    {1,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,1},
    {1,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

int playerX = 2;
int playerY = 2;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Create double buffer
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
            
            // Draw map
            HBRUSH hWallBrush = CreateSolidBrush(RGB(0, 255, 0));
            HBRUSH hInnerWallBrush = CreateSolidBrush(RGB(0, 85, 0));
            HBRUSH hFloorBrush = CreateSolidBrush(RGB(17, 17, 17));
            HBRUSH hPlayerBrush = CreateSolidBrush(RGB(255, 255, 255));
            
            // Clear background with black (though floor covers it)
            RECT bgRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
            FillRect(hdcMem, &bgRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

            for (int y = 0; y < ROWS; y++) {
                for (int x = 0; x < COLS; x++) {
                    RECT tileRect = {x * TILE_SIZE, y * TILE_SIZE, (x + 1) * TILE_SIZE, (y + 1) * TILE_SIZE};
                    if (map[y][x] == 1) {
                        FillRect(hdcMem, &tileRect, hWallBrush);
                        RECT innerRect = {x * TILE_SIZE + 2, y * TILE_SIZE + 2, (x + 1) * TILE_SIZE - 2, (y + 1) * TILE_SIZE - 2};
                        FillRect(hdcMem, &innerRect, hInnerWallBrush);
                    } else {
                        FillRect(hdcMem, &tileRect, hFloorBrush);
                    }
                }
            }
            
            // Draw player
            SelectObject(hdcMem, hPlayerBrush);
            SelectObject(hdcMem, GetStockObject(NULL_PEN));
            int r = TILE_SIZE / 3;
            int cx = playerX * TILE_SIZE + TILE_SIZE / 2;
            int cy = playerY * TILE_SIZE + TILE_SIZE / 2;
            Ellipse(hdcMem, cx - r, cy - r, cx + r, cy + r);
            
            // Copy to screen
            BitBlt(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hdcMem, 0, 0, SRCCOPY);
            
            // Cleanup
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);
            DeleteObject(hWallBrush);
            DeleteObject(hInnerWallBrush);
            DeleteObject(hFloorBrush);
            DeleteObject(hPlayerBrush);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_KEYDOWN: {
            int newX = playerX;
            int newY = playerY;
            
            switch (wParam) {
                case VK_UP:
                case 'W':
                    newY--;
                    break;
                case VK_DOWN:
                case 'S':
                    newY++;
                    break;
                case VK_LEFT:
                case 'A':
                    newX--;
                    break;
                case VK_RIGHT:
                case 'D':
                    newX++;
                    break;
            }
            
            if (newX >= 0 && newX < COLS && newY >= 0 && newY < ROWS) {
                if (map[newY][newX] == 0) {
                    playerX = newX;
                    playerY = newY;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KVoid Class";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    RegisterClass(&wc);

    RECT rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
    
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "KVoid",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
