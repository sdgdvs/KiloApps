#include <windows.h>
#include <stdio.h>

#define GRID_W 20
#define GRID_H 20
#define CELL_SIZE 20
#define OFFSET_X 20
#define OFFSET_Y 60

int food = 50;
int power = 50;
int maxPower = 50;
int mat = 50;
int pop = 0;
int maxPop = 0;
int happiness = 100;
int popWait = 0;
int tick = 0;
int day = 1;
int isDay = 1;
int sci = 0;
int unlockedHydro = 0, unlockedNuke = 0, unlockedLaser = 0;
int selectedType = 0;
int grid[GRID_W * GRID_H] = {0};

typedef struct {
    RECT rc;
    int id; // 0-9 for build, 101-103 for research
    char label[32];
    int isSelected;
} Button;
Button buttons[20];
int btnCount = 0;

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
            else if (t == 4) { bgCol = RGB(0, 51, 51); borderCol = RGB(0, 255, 255); textCol = RGB(0, 255, 255); }
            else if (t == 5) { bgCol = RGB(51, 17, 0); borderCol = RGB(255, 136, 0); textCol = RGB(255, 136, 0); }
            else if (t == 6) { bgCol = RGB(34, 0, 51); borderCol = RGB(170, 0, 255); textCol = RGB(170, 0, 255); }
            else if (t == 7) { bgCol = RGB(0, 51, 34); borderCol = RGB(0, 255, 170); textCol = RGB(0, 255, 170); }
            else if (t == 8) { bgCol = RGB(34, 51, 0); borderCol = RGB(170, 255, 0); textCol = RGB(170, 255, 0); }
            else if (t == 9) { bgCol = RGB(51, 0, 0); borderCol = RGB(255, 0, 0); textCol = RGB(255, 0, 0); }
            
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
                else if (t == 4) DrawText(hdc, "H", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                else if (t == 5) DrawText(hdc, "B", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                else if (t == 6) DrawText(hdc, "L", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                else if (t == 7) DrawText(hdc, "N", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                else if (t == 8) DrawText(hdc, "Y", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                else if (t == 9) DrawText(hdc, "D", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }
    }
}

void DrawUI(HDC hdc, HFONT hFont) {
    char buf[128];
    SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 255, 255));
    sprintf(buf, "DAY %d (%s) F:%d P:%d/%d M:%d POP:%d/%d HAP:%d%% SCI:%d", day, isDay ? "DAY" : "NIGHT", food, power, maxPower, mat, pop, maxPop, happiness, sci);
    
    RECT rcHeader = { 20, 15, 620, 45 };
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
    const char* labels[] = { "[0] INSPECT", "[1] SOLAR", "[2] FARM", "[3] MINE", "[4] HAB", "[5] BATT", "[6] LAB", "[7] NUKE", "[8] HYDRO", "[9] LASER" };
    
    btnCount = 0;
    int btnY = OFFSET_Y;
    for (int i = 0; i < 10; i++) {
        if (i == 7 && !unlockedNuke) continue;
        if (i == 8 && !unlockedHydro) continue;
        if (i == 9 && !unlockedLaser) continue;
        
        buttons[btnCount].rc = (RECT){ sidebarX, btnY, sidebarX + 180, btnY + 25 };
        buttons[btnCount].id = i;
        strcpy(buttons[btnCount].label, labels[i]);
        buttons[btnCount].isSelected = (i == selectedType);
        btnCount++;
        btnY += 30;
    }
    btnY += 10;
    if (!unlockedHydro) {
        buttons[btnCount].rc = (RECT){ sidebarX, btnY, sidebarX + 180, btnY + 25 };
        buttons[btnCount].id = 101;
        strcpy(buttons[btnCount].label, "RES HYDRO(50S)");
        buttons[btnCount].isSelected = 0;
        btnCount++;
        btnY += 30;
    }
    if (!unlockedNuke) {
        buttons[btnCount].rc = (RECT){ sidebarX, btnY, sidebarX + 180, btnY + 25 };
        buttons[btnCount].id = 102;
        strcpy(buttons[btnCount].label, "RES NUKE(100S)");
        buttons[btnCount].isSelected = 0;
        btnCount++;
        btnY += 30;
    }
    if (!unlockedLaser) {
        buttons[btnCount].rc = (RECT){ sidebarX, btnY, sidebarX + 180, btnY + 25 };
        buttons[btnCount].id = 103;
        strcpy(buttons[btnCount].label, "RES LASER(150S)");
        buttons[btnCount].isSelected = 0;
        btnCount++;
        btnY += 30;
    }

    for (int i = 0; i < btnCount; i++) {
        COLORREF btnBg = buttons[i].isSelected ? RGB(0, 51, 51) : RGB(17, 17, 34);
        if (buttons[i].id >= 100) btnBg = RGB(34, 17, 51); // purple hue for research
        COLORREF btnBorder = buttons[i].isSelected ? RGB(255, 255, 255) : RGB(0, 255, 255);
        if (buttons[i].id >= 100) btnBorder = RGB(170, 0, 255);
        COLORREF btnText = buttons[i].isSelected ? RGB(255, 255, 255) : RGB(0, 255, 255);
        if (buttons[i].id >= 100) btnText = RGB(170, 0, 255);

        HBRUSH brush = CreateSolidBrush(btnBg);
        FillRect(hdc, &buttons[i].rc, brush);
        DeleteObject(brush);
        
        HPEN pen = CreatePen(PS_SOLID, buttons[i].isSelected ? 2 : 1, btnBorder);
        HPEN oldP = SelectObject(hdc, pen);
        MoveToEx(hdc, buttons[i].rc.left, buttons[i].rc.top, NULL);
        LineTo(hdc, buttons[i].rc.right, buttons[i].rc.top);
        LineTo(hdc, buttons[i].rc.right, buttons[i].rc.bottom);
        LineTo(hdc, buttons[i].rc.left, buttons[i].rc.bottom);
        LineTo(hdc, buttons[i].rc.left, buttons[i].rc.top);
        SelectObject(hdc, oldP);
        DeleteObject(pen);
        
        SetTextColor(hdc, btnText);
        DrawText(hdc, buttons[i].label, -1, &buttons[i].rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            SetTimer(hwnd, 1, 2000, NULL);
            break;
        case WM_TIMER: {
            tick++;
            if (tick % 10 == 0) { day++; isDay = 1; }
            else if (tick % 10 == 5) { isDay = 0; }
            
            int pwrProd = 0, farmCount = 0, mineCount = 0, habCount = 0, batCount = 0;
            int labCount = 0, nukeCount = 0, hydroCount = 0, laserCount = 0;
            for(int i=0; i<GRID_W*GRID_H; i++) {
                if(grid[i]==1 && isDay) pwrProd += 4;
                if(grid[i]==2) farmCount += 1;
                if(grid[i]==3) mineCount += 1;
                if(grid[i]==4) habCount += 1;
                if(grid[i]==5) batCount += 1;
                if(grid[i]==6) labCount += 1;
                if(grid[i]==7) nukeCount += 1;
                if(grid[i]==8) hydroCount += 1;
                if(grid[i]==9) laserCount += 1;
            }
            maxPop = habCount * 5;
            maxPower = 50 + batCount * 50;
            pwrProd += nukeCount * 20;
            
            int pwrCons = farmCount + mineCount + habCount + labCount + hydroCount * 2 + laserCount * 5;
            power += pwrProd - pwrCons;
            
            float pwrEff = 1.0f;
            if (power < 0) { power = 0; pwrEff = 0.5f; }
            if (power > maxPower) power = maxPower;

            float eff = (happiness / 100.0f) * pwrEff;
            int foodProd = (int)((farmCount * 5 + hydroCount * 15) * eff);
            int matProd = (int)(mineCount * 2 * eff);
            int sciProd = (int)(labCount * 2 * eff);

            food += foodProd;
            mat += matProd;
            sci += sciProd;

            int foodCons = pop;
            if (food >= foodCons) {
                food -= foodCons;
                happiness += 5;
                if (happiness > 100) happiness = 100;
            } else {
                food = 0;
                happiness -= 10;
                if (happiness < 0) happiness = 0;
            }

            if (pop < maxPop && happiness >= 50) {
                popWait++;
                if (popWait >= 3) {
                    pop++;
                    popWait = 0;
                }
            } else if (pop > maxPop) {
                happiness -= 10;
                if (happiness < 0) happiness = 0;
            }

            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            int sidebarX = OFFSET_X + GRID_W * CELL_SIZE + 20;
            if (x >= sidebarX && x <= sidebarX + 180) {
                for (int i = 0; i < btnCount; i++) {
                    if (y >= buttons[i].rc.top && y <= buttons[i].rc.bottom) {
                        int id = buttons[i].id;
                        if (id < 100) {
                            selectedType = id;
                        } else if (id == 101 && sci >= 50 && !unlockedHydro) {
                            sci -= 50; unlockedHydro = 1;
                        } else if (id == 102 && sci >= 100 && !unlockedNuke) {
                            sci -= 100; unlockedNuke = 1;
                        } else if (id == 103 && sci >= 150 && !unlockedLaser) {
                            sci -= 150; unlockedLaser = 1;
                        }
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
                    else if (selectedType == 4) { costMat = 15; costPwr = 5; }
                    else if (selectedType == 5) costMat = 20;
                    else if (selectedType == 6) { costMat = 20; costPwr = 5; }
                    else if (selectedType == 7) costMat = 50;
                    else if (selectedType == 8) { costMat = 20; costPwr = 10; }
                    else if (selectedType == 9) { costMat = 30; costPwr = 20; }
                    
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
