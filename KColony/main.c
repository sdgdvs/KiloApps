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

void DrawGrid(HDC hdc, HFONT hFont) {
    SelectObject(hdc, hFont);
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            RECT rc = { OFFSET_X + x * CELL_SIZE, OFFSET_Y + y * CELL_SIZE, OFFSET_X + (x + 1) * CELL_SIZE, OFFSET_Y + (y + 1) * CELL_SIZE };
            
            int t = grid[y * GRID_W + x];
            
            COLORREF bgCol = RGB(10, 17, 26);
            COLORREF borderCol = RGB(0, 51, 51);
            COLORREF textCol = RGB(0, 255, 255);
            
            if (t == 1) { bgCol = RGB(51, 51, 0); borderCol = RGB(255, 255, 0); textCol = RGB(255, 255, 0); }
            else if (t == 2) { bgCol = RGB(0, 51, 0); borderCol = RGB(0, 255, 0); textCol = RGB(0, 255, 0); }
            else if (t == 3) { bgCol = RGB(51, 0, 51); borderCol = RGB(255, 0, 255); textCol = RGB(255, 0, 255); }
            
            HBRUSH brush = CreateSolidBrush(bgCol);
            FillRect(hdc, &rc, brush);
            DeleteObject(brush);
            
            HPEN pen = CreatePen(PS_SOLID, 1, borderCol);
            HPEN oldPen = SelectObject(hdc, pen);
            MoveToEx(hdc, rc.left, rc.top, NULL);
            LineTo(hdc, rc.right, rc.top);
            LineTo(hdc, rc.right, rc.bottom);
            LineTo(hdc, rc.left, rc.bottom);
            LineTo(hdc, rc.left, rc.top);
            SelectObject(hdc, oldPen);
            DeleteObject(pen);
            
            if (t > 0) {
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, textCol);
                if (t == 1) DrawText(hdc, "S", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                else if (t == 2) DrawText(hdc, "F", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                else if (t == 3) DrawText(hdc, "M", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }
    }
}

void DrawUI(HDC hdc, HFONT hFont) {
    char buf[128];
    SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 255, 255));
    sprintf(buf, "FOOD: %d   POWER: %d   MAT: %d", food, power, mat);
    
    RECT rcHeader = { 20, 15, 420, 45 };
    HBRUSH hdrBrush = CreateSolidBrush(RGB(17, 17, 34));
    FillRect(hdc, &rcHeader, hdrBrush);
    DeleteObject(hdrBrush);
    HPEN hdrPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 255));
    HPEN oldPen = SelectObject(hdc, hdrPen);
    MoveToEx(hdc, rcHeader.left, rcHeader.top, NULL);
    LineTo(hdc, rcHeader.right, rcHeader.top);
    LineTo(hdc, rcHeader.right, rcHeader.bottom);
    LineTo(hdc, rcHeader.left, rcHeader.bottom);
    LineTo(hdc, rcHeader.left, rcHeader.top);
    SelectObject(hdc, oldPen);
    DeleteObject(hdrPen);
    
    DrawText(hdc, buf, -1, &rcHeader, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    int sidebarX = OFFSET_X + GRID_W * CELL_SIZE + 20;
    const char* labels[] = { "[0] INSPECT", "[1] SOLAR (10M)", "[2] FARM (10M, 5P)", "[3] MINE (10P)" };
    for (int i = 0; i < 4; i++) {
        RECT rcBtn = { sidebarX, OFFSET_Y + i * 45, sidebarX + 180, OFFSET_Y + i * 45 + 35 };
        
        COLORREF btnBg = (i == selectedType) ? RGB(0, 51, 51) : RGB(17, 17, 34);
        COLORREF btnBorder = (i == selectedType) ? RGB(255, 255, 255) : RGB(0, 255, 255);
        COLORREF btnText = (i == selectedType) ? RGB(255, 255, 255) : RGB(0, 255, 255);
        
        HBRUSH brush = CreateSolidBrush(btnBg);
        FillRect(hdc, &rcBtn, brush);
        DeleteObject(brush);
        
        HPEN pen = CreatePen(PS_SOLID, (i == selectedType) ? 2 : 1, btnBorder);
        HPEN oldP = SelectObject(hdc, pen);
        MoveToEx(hdc, rcBtn.left, rcBtn.top, NULL);
        LineTo(hdc, rcBtn.right, rcBtn.top);
        LineTo(hdc, rcBtn.right, rcBtn.bottom);
        LineTo(hdc, rcBtn.left, rcBtn.bottom);
        LineTo(hdc, rcBtn.left, rcBtn.top);
        SelectObject(hdc, oldP);
        DeleteObject(pen);
        
        SetTextColor(hdc, btnText);
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
            if (x >= sidebarX && x <= sidebarX + 180) {
                for (int i = 0; i < 4; i++) {
                    if (y >= OFFSET_Y + i * 45 && y <= OFFSET_Y + i * 45 + 35) {
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
            
            HBRUSH bg = CreateSolidBrush(RGB(5, 10, 15));
            FillRect(hdcMem, &rc, bg);
            DeleteObject(bg);
            
            HFONT hFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH, "Courier New");
            
            DrawGrid(hdcMem, hFont);
            DrawUI(hdcMem, hFont);
            
            DeleteObject(hFont);
            
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
