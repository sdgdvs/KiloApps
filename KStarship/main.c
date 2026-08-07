#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define MAP_SIZE 2000
#define NUM_STARS 500

typedef struct {
    int x;
    int y;
    int size;
    COLORREF color;
} Star;

Star stars[NUM_STARS];

int ship_x = 0;
int ship_y = 0;
int is_moving = 0;
float res_fuel = 10000.0f;
int res_hull = 100;
int res_crew = 10;

void InitStars() {
    for (int i = 0; i < NUM_STARS; i++) {
        stars[i].x = (rand() % MAP_SIZE) - MAP_SIZE/2;
        stars[i].y = (rand() % MAP_SIZE) - MAP_SIZE/2;
        stars[i].size = (rand() % 3) + 1;
        int colorType = rand() % 10;
        if (colorType > 8) stars[i].color = RGB(255, 221, 170);
        else if (colorType > 6) stars[i].color = RGB(170, 221, 255);
        else stars[i].color = RGB(255, 255, 255);
    }
}

void Update() {
    is_moving = 0;
    int speed = 5;
    int dx = 0, dy = 0;
    if (GetAsyncKeyState('W') & 0x8000) { dy -= speed; }
    if (GetAsyncKeyState('S') & 0x8000) { dy += speed; }
    if (GetAsyncKeyState('A') & 0x8000) { dx -= speed; }
    if (GetAsyncKeyState('D') & 0x8000) { dx += speed; }

    if ((dx != 0 || dy != 0) && res_fuel > 0) {
        ship_x += dx;
        ship_y += dy;
        is_moving = 1;
        res_fuel -= 1.0f;
        if (res_fuel < 0) res_fuel = 0;
    }

    if (ship_x < -MAP_SIZE/2) ship_x = -MAP_SIZE/2;
    if (ship_x > MAP_SIZE/2) ship_x = MAP_SIZE/2;
    if (ship_y < -MAP_SIZE/2) ship_y = -MAP_SIZE/2;
    if (ship_y > MAP_SIZE/2) ship_y = MAP_SIZE/2;
}

void Draw(HDC hdc, RECT* rect) {
    int width = rect->right - rect->left;
    int height = rect->bottom - rect->top;
    
    int mapWidth = width - 200;
    if (mapWidth < 100) mapWidth = 100;

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
    SelectObject(memDC, memBitmap);

    HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0));
    RECT mapRect = {0, 0, mapWidth, height};
    FillRect(memDC, &mapRect, bgBrush);
    DeleteObject(bgBrush);

    HBRUSH uiBrush = CreateSolidBrush(RGB(5, 5, 20));
    RECT uiRect = {mapWidth, 0, width, height};
    FillRect(memDC, &uiRect, uiBrush);
    DeleteObject(uiBrush);

    // Neon borders
    HPEN neonBorderPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
    SelectObject(memDC, neonBorderPen);
    SelectObject(memDC, GetStockObject(NULL_BRUSH));
    Rectangle(memDC, 0, 0, mapWidth, height);
    Rectangle(memDC, mapWidth + 5, 5, width - 5, height - 5);
    DeleteObject(neonBorderPen);

    int centerX = mapWidth / 2;
    int centerY = height / 2;

    HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(0, 40, 40));
    SelectObject(memDC, gridPen);
    int gridSize = 100;
    int offsetX = -(ship_x % gridSize);
    int offsetY = -(ship_y % gridSize);
    if (offsetX > 0) offsetX -= gridSize;
    if (offsetY > 0) offsetY -= gridSize;

    for (int x = offsetX; x < mapWidth; x += gridSize) {
        MoveToEx(memDC, x, 0, NULL);
        LineTo(memDC, x, height);
    }
    for (int y = offsetY; y < height; y += gridSize) {
        MoveToEx(memDC, 0, y, NULL);
        LineTo(memDC, mapWidth, y);
    }
    DeleteObject(gridPen);

    for (int i = 0; i < NUM_STARS; i++) {
        int screenX = centerX + (stars[i].x - ship_x);
        int screenY = centerY + (stars[i].y - ship_y);

        if (screenX >= 0 && screenX <= mapWidth && screenY >= 0 && screenY <= height) {
            HBRUSH starBrush = CreateSolidBrush(stars[i].color);
            HPEN starPen = CreatePen(PS_SOLID, 1, stars[i].color);
            SelectObject(memDC, starBrush);
            SelectObject(memDC, starPen);
            int s = stars[i].size;
            Ellipse(memDC, screenX - s, screenY - s, screenX + s, screenY + s);
            DeleteObject(starBrush);
            DeleteObject(starPen);
        }
    }

    POINT shipPts[4] = {
        {centerX, centerY - 10},
        {centerX + 8, centerY + 8},
        {centerX, centerY + 4},
        {centerX - 8, centerY + 8}
    };
    HBRUSH shipBrush = CreateSolidBrush(RGB(0, 255, 255));
    HPEN shipPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
    SelectObject(memDC, shipBrush);
    SelectObject(memDC, shipPen);
    Polygon(memDC, shipPts, 4);
    DeleteObject(shipBrush);
    DeleteObject(shipPen);

    if (is_moving) {
        POINT thrustPts[4] = {
            {centerX, centerY + 6},
            {centerX + 4, centerY + 10},
            {centerX, centerY + 16},
            {centerX - 4, centerY + 10}
        };
        HBRUSH thrustBrush = CreateSolidBrush(RGB(255, 136, 0));
        HPEN thrustPen = CreatePen(PS_SOLID, 1, RGB(255, 136, 0));
        SelectObject(memDC, thrustBrush);
        SelectObject(memDC, thrustPen);
        Polygon(memDC, thrustPts, 4);
        DeleteObject(thrustBrush);
        DeleteObject(thrustPen);
    }

    SetBkMode(memDC, TRANSPARENT);
    char buf[128];
    
    HPEN linePen = CreatePen(PS_SOLID, 1, RGB(0, 85, 85));
    
    wsprintfA(buf, "SHIP STATUS");
    SetTextColor(memDC, RGB(0, 255, 255));
    TextOutA(memDC, mapWidth + 15, 20, buf, lstrlenA(buf));
    
    SelectObject(memDC, linePen);
    MoveToEx(memDC, mapWidth + 15, 38, NULL);
    LineTo(memDC, width - 15, 38);

    wsprintfA(buf, "Location: %d, %d", ship_x, ship_y);
    SetTextColor(memDC, RGB(255, 255, 255));
    TextOutA(memDC, mapWidth + 15, 45, buf, lstrlenA(buf));

    wsprintfA(buf, "RESOURCES");
    SetTextColor(memDC, RGB(0, 255, 255));
    TextOutA(memDC, mapWidth + 15, 75, buf, lstrlenA(buf));
    
    MoveToEx(memDC, mapWidth + 15, 93, NULL);
    LineTo(memDC, width - 15, 93);
    
    wsprintfA(buf, "Fuel: %d", (int)res_fuel);
    SetTextColor(memDC, RGB(255, 255, 255));
    TextOutA(memDC, mapWidth + 15, 100, buf, lstrlenA(buf));
    
    wsprintfA(buf, "Hull: %d%%", res_hull);
    TextOutA(memDC, mapWidth + 15, 120, buf, lstrlenA(buf));
    
    wsprintfA(buf, "Crew: %d", res_crew);
    TextOutA(memDC, mapWidth + 15, 140, buf, lstrlenA(buf));

    wsprintfA(buf, "SCANNER");
    SetTextColor(memDC, RGB(0, 255, 255));
    TextOutA(memDC, mapWidth + 15, 175, buf, lstrlenA(buf));
    
    MoveToEx(memDC, mapWidth + 15, 193, NULL);
    LineTo(memDC, width - 15, 193);
    DeleteObject(linePen);

    int found_star = 0;
    for (int i = 0; i < NUM_STARS; i++) {
        int dx = stars[i].x - ship_x;
        int dy = stars[i].y - ship_y;
        if (dx*dx + dy*dy < 2500) {
            found_star = 1;
            break;
        }
    }

    if (found_star) {
        wsprintfA(buf, "Star system detected in proximity.");
        SetTextColor(memDC, RGB(0, 255, 255));
    } else {
        wsprintfA(buf, "Deep space. Nothing nearby.");
        SetTextColor(memDC, RGB(136, 136, 136));
    }
    RECT textRect = {mapWidth + 15, 200, width - 10, 300};
    DrawTextA(memDC, buf, -1, &textRect, DT_WORDBREAK);

    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            SetTimer(hwnd, 1, 16, NULL);
            return 0;
        case WM_TIMER:
            Update();
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            Draw(hdc, &rect);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    InitStars();

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KStarshipClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(0, wc.lpszClassName, "KStarship Command", WS_OVERLAPPEDWINDOW, 
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
