#include <windows.h>
#include <stdio.h>

#define GRID_W 20
#define GRID_H 20
#define CELL_SIZE 20
#define OFFSET_X 20
#define OFFSET_Y 60

int food = 50;
int power = 50;
int mat = 50;
int selectedType = 0;
int grid[GRID_W * GRID_H] = {0};

void DrawGrid(HDC hdc) {
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            RECT rc = { OFFSET_X + x * CELL_SIZE, OFFSET_Y + y * CELL_SIZE, OFFSET_X + (x + 1) * CELL_SIZE, OFFSET_Y + (y + 1) * CELL_SIZE };
            HBRUSH brush = CreateSolidBrush(RGB(34, 34, 34));
            FillRect(hdc, &rc, brush);
            DeleteObject(brush);
            
            HPEN pen = CreatePen(PS_SOLID, 1, RGB(85, 85, 85));
            HPEN oldPen = SelectObject(hdc, pen);
            MoveToEx(hdc, rc.left, rc.top, NULL);
            LineTo(hdc, rc.right, rc.top);
            LineTo(hdc, rc.right, rc.bottom);
            LineTo(hdc, rc.left, rc.bottom);
            LineTo(hdc, rc.left, rc.top);
            SelectObject(hdc, oldPen);
            DeleteObject(pen);
            
            int t = grid[y * GRID_W + x];
            if (t > 0) {
                SetBkMode(hdc, TRANSPARENT);
                if (t == 1) { SetTextColor(hdc, RGB(255, 255, 0)); DrawText(hdc, "S", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE); }
                else if (t == 2) { SetTextColor(hdc, RGB(0, 255, 0)); DrawText(hdc, "F", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE); }
                else if (t == 3) { SetTextColor(hdc, RGB(170, 170, 170)); DrawText(hdc, "M", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE); }
            }
        }
    }
}

void DrawUI(HDC hdc) {
    char buf[128];
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));
    sprintf(buf, "Food: %d   Power: %d   Materials: %d", food, power, mat);
    TextOut(hdc, 20, 20, buf, lstrlen(buf));
    
    int sidebarX = OFFSET_X + GRID_W * CELL_SIZE + 20;
    const char* labels[] = { "0: None", "1: Solar (10 Mat)", "2: Farm (10M, 5P)", "3: Mine (10P)" };
    for (int i = 0; i < 4; i++) {
        RECT rcBtn = { sidebarX, OFFSET_Y + i * 40, sidebarX + 150, OFFSET_Y + i * 40 + 30 };
        HBRUSH brush = CreateSolidBrush(i == selectedType ? RGB(42, 90, 42) : RGB(68, 68, 68));
        FillRect(hdc, &rcBtn, brush);
        DeleteObject(brush);
        DrawText(hdc, labels[i], -1, &rcBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            SetTimer(hwnd, 1, 2000, NULL);
            break;
        case WM_TIMER: {
            int pwrProd = 0, foodProd = 0, matProd = 0;
            for(int i=0; i<GRID_W*GRID_H; i++) {
                if(grid[i]==1) pwrProd += 2;
                if(grid[i]==2) foodProd += 1;
                if(grid[i]==3) matProd += 1;
            }
            power += pwrProd;
            food += foodProd;
            mat += matProd;
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            int sidebarX = OFFSET_X + GRID_W * CELL_SIZE + 20;
            if (x >= sidebarX && x <= sidebarX + 150) {
                for (int i = 0; i < 4; i++) {
                    if (y >= OFFSET_Y + i * 40 && y <= OFFSET_Y + i * 40 + 30) {
                        selectedType = i;
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }
            }
            
            if (x >= OFFSET_X && x < OFFSET_X + GRID_W * CELL_SIZE && y >= OFFSET_Y && y < OFFSET_Y + GRID_H * CELL_SIZE) {
                int gx = (x - OFFSET_X) / CELL_SIZE;
                int gy = (y - OFFSET_Y) / CELL_SIZE;
                int idx = gy * GRID_W + gx;
                
                if (selectedType != 0 && grid[idx] == 0) {
                    int costMat = 0, costPwr = 0;
                    if (selectedType == 1) costMat = 10;
                    else if (selectedType == 2) { costMat = 10; costPwr = 5; }
                    else if (selectedType == 3) costPwr = 10;
                    
                    if (mat >= costMat && power >= costPwr) {
                        mat -= costMat;
                        power -= costPwr;
                        grid[idx] = selectedType;
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                }
            }
            break;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HDC hdcMem = CreateCompatibleDC(hdc);
            RECT rc;
            GetClientRect(hwnd, &rc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            HBITMAP hbmOld = SelectObject(hdcMem, hbmMem);
            
            HBRUSH bg = CreateSolidBrush(RGB(26, 26, 26));
            FillRect(hdcMem, &rc, bg);
            DeleteObject(bg);
            
            DrawGrid(hdcMem);
            DrawUI(hdcMem);
            
            BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);
            
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KColonyClass";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KColony", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
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
